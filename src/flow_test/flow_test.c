/********************************
* flow_test.c
* Only used with forward card
* version: 1.0
* 2010.7.5
*********************************/

#include "flow_test.h"


//#define DEBUG
#define Kbps    1000
#define Mbps    1000000
static char nacname[20] = "/dev/";
int nacfd = -1;
char shutdownInProgress = 0;
static uint64_t totalBytes[NUM_STREAM];
static uint64_t totalPackets[NUM_STREAM];
char in_file[8][10]= {"test0.erf","test1.erf","test2.erf","test3.erf","test4.erf","test5.erf","test6.erf","test7.erf"};
char out_file[8][12]= {"testo0.erf\0","testo1.erf\0","testo2.erf\0","testo3.erf\0","testo4.erf\0","testo5.erf\0","testo6.erf\0","testo7.erf\0"};

int refresh_num=0;
unsigned char report_stop;
unsigned char an_stop[NUM_STREAM];


typedef struct an_arg
{
    int stream_num;
    struct timeval maxwait;
    struct timeval poll;
    unsigned int mindata;
} an_arg_t;

an_arg_t anThreadArg[NUM_STREAM];
pthread_t anRxThread[NUM_STREAM];
pthread_t anTxThread[NUM_STREAM];

pthread_t reportThread;

/**********************************************************/
void shutdown_flow(void)
{

    int i;
    capture_stop(nacfd);
    shutdownInProgress = 1;

    printf("Waiting for thread to stop...\n");
    while (!report_stop) sleep(1);
    printf("Report thread stopped.\n");

    // while (!pulse_stop) sleep(1);
    // printf("Test_rft thread stopped.\n");

    for (i=0; i<NUM_STREAM; i++)
    {
        while(!an_stop[i]) sleep(1);
        printf("Analyze thread for stream %d stopped.\n",i);
    }

    for (i=0; i < NUM_STREAM; i++)
    {

        // stop nac card
        if(nac_stop_stream(nacfd, i) < 0)
        {
            printf("Unable to stop stream %d\n", i);
            exit(-1);
        }

        if (nac_detach_stream(nacfd, i) < 0)
        {
            printf("Unable to detach stream %d\n", i);
            exit(-1);
        }
        if(nac_tx_stop_stream(nacfd, (i + NUM_STREAM),COPY_FWD) < 0)
        {
            printf("Unable to stop stream %d\n", (i + NUM_STREAM));
            exit(-1);
        }

        if (nac_tx_detach_stream(nacfd, (i + NUM_STREAM),COPY_FWD) < 0)
        {
            printf("Unable to detach stream %d\n", (i + NUM_STREAM));
            exit(-1);
        }
    }

    nac_close(nacfd);
    nacfd = -1;
    return;
}

/**********************************************************/
void brokenPipe(int signo)
{

    signal(SIGPIPE, brokenPipe);
}

/**********************************************************/
void cleanup(int signo)
{
    shutdown_flow();
}

/**********************************************************/
/**********************************************************/







void print_buf(unsigned char *s, unsigned int len)
{
    int i=0;
    for (i=0; i<len; i++)
    {
        if (i%16 == 0)
        {
            printf("\n[0x%04x]  ",i);
        }
        printf("%02x ",s[i]);

    }
    printf("\n");

}

static void *run_rx(an_arg_t* arg)
{
    unsigned char* top;
    unsigned char* bottom;
    int j;
    uint8_t * cp_org;
    unsigned long  accumulated = 0;
    uint32_t tsc_before,tsc_after;
    uint32_t tsc_one_ms;
    uint32_t uNacApiTxCopyCnt = 0;
    uint32_t drop_pkt_cnt = 0;
    erf_record_t * erfhdr;
    uint8_t *p;
    uint8_t card_fwd;
    IpHeader *      ip_hdr;
    unsigned short	rlen;
    unsigned short last_rlen=0;
    unsigned short	align_rlen;
    unsigned short	wlen;
    pkt_info_arg_t pkt_info;
    unsigned int	src_ip;
    unsigned int	dst_ip;
    unsigned char	protocol;
    unsigned short	src_port;
    unsigned short	dst_port;
    int err,cnt,ts;
    int out_port;
    uint8_t * ret;
    int stream_num  		= arg->stream_num;
    unsigned int mindata 	= arg->mindata;
    struct timeval maxwait	= arg->maxwait;
    struct timeval poll		= arg->poll;
    FILE *fpx;
    // init bottom pointer to NULL
    bottom = NULL;

    cnt=ts=0;
    if ((fpx=fopen(out_file[stream_num],"wb")) == NULL)
    {
        printf("fail to open file %s\n",in_file);
        exit(-2);
    }
    // map stream to buffers
    if(nac_attach_stream(nacfd, stream_num, 0, 0) < 0)
    {
        printf("nac configure failed, not attached\n");
        return(NULL);
    }
    // set polling parameters
    if(nac_set_stream_poll(nacfd, stream_num, mindata, &maxwait, &poll) < 0)
    {
        printf("set stream poll parameter failed\n");
        return(NULL);
    }
    // start capturing data
    if(nac_start_stream(nacfd, stream_num) < 0)
    {
        printf("nac_start_stream failed\n");
        return(NULL);
    }
    err=0;
    // get data
    while(!shutdownInProgress)
    {
        //printf("cycle out\n");
        if(( top = nac_advance_stream(nacfd, stream_num, &bottom)) == NULL)
        {
            printf("Error, nac_advance_stream %d return NULL pointer.\n",stream_num);
            goto PEND;
        }
        //if (top==bottom)
        //continue;
        //	printf("Set bottom = %p\n",bottom);
        while(((top-bottom) > RECORD_HDR_SIZE))
        {
            // 	at least a record header exists
            erfhdr = (erf_record_t *)bottom;
            p = (uint8_t *)bottom;
            rlen = ntohs(erfhdr->rlen);
            wlen = ntohs(erfhdr->wlen);
            ts = ntohl(erfhdr->ts[0]);


            if((top-bottom) < rlen)
                break;

            /*          if (ts != cnt)
                      {
                       printf("[stream %d] Bad packet ts 0x%x != cnt 0x%x\n", stream_num, ts, cnt);
                       err=1;
                       }
                       cnt++;
                       if (cnt == 0x5ae)cnt=0;
            */
            if (rlen == 0)
            {
                printf("stream [%d] rlen is zero\n",stream_num);
                err = 1;
            }

//              printf("rx buffer\n\n");
//              for (j=0;j<512;j++){
            //               if (j%16==0) printf("\n");
            //              printf("%x  ",*(unsigned char *)(bottom+j));

            //          }
//		printf("\n");

//	if (totalPackets[stream_num]%100000==0)
//	printf("[Stream %d]One packet received, size = %d bytes, rlen = %d, packet %d\n",stream_num,wlen,rlen,totalPackets[stream_num]);
            if (rlen < 60 || rlen > 1521)
            {
                printf("[stream %d] wlen is error.\n",stream_num);
                printf("[stream %d] rlen %d, wlen %d.\n", stream_num,rlen, wlen);
                err=1;
            }


            if ((wlen%16) == 0)
            {
                if (rlen != (wlen + 16))
                {
                    printf("rlen is error1.\n");
                    printf("rlen %d, wlen %d.\n", rlen, wlen);
                    err=1;
                }
            }
            else
            {

                if (rlen != (wlen/16 + 2) * 16)
                {
                    printf("rlen is error2.\n");
                    printf("rlen %d, wlen %d.\n", rlen, wlen);
                    err=1;
                }
            }

//	print_buf(bottom,rlen);
            //      printf("\n\n");

            if (err==1)
            {

                printf("\n[steam %d] last rlen = 0x%x,bottom = %p, top = %p, pkt: %d, bytes: %d\n",stream_num,last_rlen,bottom,top,totalPackets[stream_num],totalBytes[stream_num]);
                //  print_buf(bottom-(last_rlen+16),last_rlen+last_rlen);
                print_buf(bottom-256,512);
                fwrite(bottom,1,top-bottom,fpx);
                fclose(fpx);
                fflush(stdout);
                sleep(5);
                exit(1);
            };

            /********add user process pakcet function*******/
            totalBytes[stream_num] +=rlen;// (wlen+4);
            totalPackets[stream_num] ++;
            last_rlen = rlen;

            // increment bottom pointer to next packet
            bottom += rlen;
        }
//	printf("top  %p  bottom  %p\n\n",top,bottom);

    }
PEND:
    an_stop[stream_num] = 1;

    return(NULL);


}

static void *run_tx(an_arg_t* arg)
{
    uint8_t * cp_org;
    uint8_t * send_buf;

    unsigned int fsize=0;

    int stream_num			= arg->stream_num;
    unsigned int mindata	= arg->mindata;
    struct timeval maxwait	= arg->maxwait;
    struct timeval poll 	= arg->poll;
    FILE *fpi;
    uint8_t * ret;
    int i=0,j=0,tx_times=0;

    printf("start tx %d\n",stream_num);
    fpi = fopen(in_file[stream_num],"rb");
//fp = fopen("test.erf","rb");
//fpo = fopen(out_file[stream_num],"wb");

    if (fpi==NULL) return(-3);

    fseek(fpi,0,SEEK_END);

    fsize = ftell(fpi);
    printf("file to send size is %d\n",fsize);

    fseek(fpi,0,SEEK_SET);

    send_buf = malloc(fsize+1);
    memset(send_buf,0,fsize+1);

    fread(send_buf,1,fsize,fpi);

    fclose(fpi);


    /*transmit stream map buffers*/

// set polling parameters

// start capturing data


//initial pointer to NULL





//if (j==0) memset(cp_org,0,64*1024*1024);

//printf("cp_org = %p\n",cp_org);

//printf("tx %d\n",tx_times);
//memcpy(cp_org+16,&tx_times,4);

//print_buf(cp_org,fsize);

//while(cp_org[fsize-1]!= send_buf[fsize-1]) printf("stream[%d] no equal\n",stream_num);


//sleep(30);
//printf("sleep done\n");

//sleep(50);
//printf("xxxxxxxxxxxxxx\n");

//fwrite(cp_org,1,fsize,fpo);
//fflush(fpo);

//sleep(30);
//printf("start send again\n");
//if ((j==100)&&(stream_num==0)) goto TX_END;
//j++;
//nac_tx_stop_stream(nacfd, (stream_num + NUM_STREAM),COPY_FWD);

//fclose(fpo);



    /**********************************************************/
    // init bottom pointer to NULL
    // map stream to buffers
    // set polling parameters
    // start capturing data

    /*transmit stream map buffers*/
    if(nac_tx_attach_stream(nacfd, (stream_num + NUM_STREAM),COPY_FWD, 0) < 0)
    {
        printf("nac configure failed, not attached\n");
        return(NULL);
    }
    // set polling parameters
    if(nac_tx_set_stream_poll(nacfd, (stream_num + NUM_STREAM), mindata, &maxwait, &poll,COPY_FWD) < 0)
    {
        printf("set stream poll parameter failed\n");
        return(NULL);
    }
    // start capturing data
    if(nac_tx_start_stream(nacfd, (stream_num + NUM_STREAM),COPY_FWD) < 0)
    {
        printf("nac_start_stream failed\n");
        return(NULL);
    }
    /*************************/

    //set timeout
    //initial pointer to NULL
    // get data
    while (!shutdownInProgress)
    {
        //printf("cycle out\n");
        //if (top==bottom)
        //continue;
        // 	at least a record header exists
        //printf("rlen is %d, wlen is %d.\n", rlen, wlen);
        //	pkt_info.value = ntohl(erfhdr->pkt_info.value);
        //printf("forward is %d, vld is %d dst_mac: %02x:%02x:%02x:%02x:%02x:%02x src_mac: %02x:%02x:%02x:%02x:%02x:%02x\n",pkt_info.data.forward, pkt_info.data.vld,
        //	erfhdr->dst_mac[0], erfhdr->dst_mac[1], erfhdr->dst_mac[2], erfhdr->dst_mac[3], erfhdr->dst_mac[4], erfhdr->dst_mac[5],
        //	erfhdr->src_mac[0], erfhdr->src_mac[1], erfhdr->src_mac[2], erfhdr->src_mac[3], erfhdr->src_mac[4], erfhdr->src_mac[5]);
        //card_fwd = fwd_info.data.card_fwd;
        //		printf("rlen is %d ,wlen is %d top %p bottom %p\n",rlen,wlen,top,bottom);

        //		printf("hash1 is 0x%x hash2 is 0x%x\n",hash1,hash2);
        //	printf("wlen is %d host rlen is %d ,host align rlen is %d net rlen is %d\n",wlen,rlen,align_rlen,ntohs(erfhdr->rlen));

        /********add user process pakcet function*******/
        //erfhdr->pkt_info.data.card_id = pkt_info.data.port_id;//
        //erfhdr->pkt_info.data.port_id = 1;// host_forward
        /********add user process pakcet function*******/
        /*filt_packet((uint8_t*)bottom);
        if(packet should be filter)
        {// filter drop, we don't want to forward these
        	bottom+=rlen;
        	continue;
        }*/
        /***if bytes to transmit is too large ,commit  them to transmit.***/
        //		printf("0 nac_tx_commit %d\n",uNacApiTxCopyCnt);
        //clear timeout
        //clear accumulated bytes
        //get transmit free space,update original copy pointer
        cp_org = nac_tx_get_stream_space(nacfd,(stream_num + NUM_STREAM),fsize);
        if (NULL == cp_org)
        {
            perror("nac_tx_get_stream_space,return");
            return NULL;
        }
        memset(cp_org,0xEE,fsize);
        /***if bytes to transmit is too large ,commit  them to transmit.***/
        //if come here indicate you want to tansmit it ,so copy them to transmit buffer.
        /*	out_port = stream_num;
        	nac_tx_memcpy(&cp_org,bottom,out_port);
        */
        memcpy(cp_org,send_buf,fsize);
        // increment bottom pointer to next packet
        tx_times++;
        // increment count of data accumulated since last nac_advance_stream() call
        // increment bottom pointer to next packet
        /* commit uncommited data, if 1ms has passed since last commit*/

        /***if timeout ,commit  them to transmit.*/
        ret = nac_tx_stream_commit_bytes(nacfd,(stream_num + NUM_STREAM),fsize);

        //sleep(30);
        //printf("sleep done\n");
        //	printf("1 nac_tx_commit %d\n",uNacApiTxCopyCnt);
        if (NULL == ret)
        {
            perror("nac_tx_stream_commit_bytes,return");
            return NULL;
            //clear timeout
            //clear accumulated bytes
            //get transmit free space,update original copy pointer
        }
    }
    /***if timeout ,commit  them to transmit.*/

TX_END:
    free(send_buf);


    /**********************************************************/
//fwrite(p,1,RFT_SIZE,fd);
    if(stream_num == 0)
    {
        //nac_tx_stop_stream(nacfd, (stream_num + NUM_STREAM),COPY_FWD);
        printf("stream 0 stopped\n");
    }
}

/**********************************************************/

void* anReport(void * unUsed)
{

    struct timeval start, end;
    uint64_t 	bytesCnt[NUM_STREAM],packetCnt[NUM_STREAM];
    double	 	byteSpeed, packetSpeed;
    uint64_t	timeuse=0;
    int i;
    uint64_t rx_bytes0=0;
    uint64_t rx_bytes1=0;
    uint64_t rx_bytes2=0;
    uint64_t rx_bytes3=0;
    time_t t;
    char tmp[100];
    statistics_cnt_t port0_cnt,port1_cnt,port2_cnt,port3_cnt;
    memset(bytesCnt,0,sizeof(uint64_t) * NUM_STREAM);
    memset(packetCnt,0,sizeof(uint64_t) * NUM_STREAM);
    memset(&port0_cnt,0,sizeof(statistics_cnt_t));
    memset(&port1_cnt,0,sizeof(statistics_cnt_t));
    memset(&port2_cnt,0,sizeof(statistics_cnt_t));
    memset(&port3_cnt,0,sizeof(statistics_cnt_t));
    gettimeofday(&start,NULL);
    printf("************************************************\n");

    while(!shutdownInProgress)
    {
        sleep(SCAN_CYCLE);
        gettimeofday(&end,NULL);
        timeuse = 1000000 * (end.tv_sec - start.tv_sec) + end.tv_usec - start.tv_usec;
        t=time(NULL);
        strftime(tmp,sizeof(tmp),"%F %T",localtime(&t));
        printf("%s\n",tmp);
        for (i=0; i<NUM_STREAM; i++)
        {

          //  printf("Stream %d: %f Mbps, %f Mpps, total: %d \n",i,(double)(8*(totalBytes[i] - bytesCnt[i]))/(double)timeuse,
          //         (double)(totalPackets[i]-packetCnt[i])/(double)timeuse,totalPackets[i]-packetCnt[i]);

            //	printf("total : 0x%x\n",totalBytes[i]);
        }
        /*
        		nac_statistics_cnt(nacfd, 0, &port0_cnt);
        		nac_statistics_cnt(nacfd, 1, &port1_cnt);
        		nac_statistics_cnt(nacfd, 2, &port2_cnt);
        		nac_statistics_cnt(nacfd, 3, &port3_cnt);

        		printf("Port[0] Rate:%lu  RX bytes: %lu RX frames :%lu; TX bytes %lu, TX frames %lu\n",(port0_cnt.rx_bytes-rx_bytes0)/SCAN_CYCLE,port0_cnt.rx_bytes,port0_cnt.rx_frames,port0_cnt.tx_bytes,port0_cnt.tx_frames);
        		printf("Port[1] Rate:%lu  RX bytes: %lu RX frames :%lu; TX bytes %lu, TX frames %lu\n",(port1_cnt.rx_bytes-rx_bytes1)/SCAN_CYCLE,port1_cnt.rx_bytes,port1_cnt.rx_frames,port1_cnt.tx_bytes,port1_cnt.tx_frames);
        		printf("Port[2] Rate:%lu  RX bytes: %lu RX frames :%lu; TX bytes %lu, TX frames %lu\n",(port2_cnt.rx_bytes-rx_bytes2)/SCAN_CYCLE,port2_cnt.rx_bytes,port2_cnt.rx_frames,port2_cnt.tx_bytes,port2_cnt.tx_frames);
        		printf("Port[3] Rate:%lu  RX bytes: %lu RX frames :%lu; TX bytes %lu, TX frames %lu\n",(port3_cnt.rx_bytes-rx_bytes3)/SCAN_CYCLE,port3_cnt.rx_bytes,port3_cnt.rx_frames,port3_cnt.tx_bytes,port3_cnt.tx_frames);
        		rx_bytes0 =port0_cnt.rx_bytes;
        		rx_bytes1 =port1_cnt.rx_bytes;
        		rx_bytes2 =port2_cnt.rx_bytes;
        		rx_bytes3 =port3_cnt.rx_bytes;
        		memset(&port0_cnt,0,sizeof(statistics_cnt_t));
        		memset(&port1_cnt,0,sizeof(statistics_cnt_t));
        		memset(&port2_cnt,0,sizeof(statistics_cnt_t));
        		memset(&port3_cnt,0,sizeof(statistics_cnt_t));

        */
        memcpy(&start,&end,sizeof(struct timeval));
        memcpy(bytesCnt, totalBytes, sizeof(uint64_t) * NUM_STREAM);
        memcpy(packetCnt, totalPackets, sizeof(uint64_t) * NUM_STREAM);
        printf("************************************************\n");
    }
    report_stop = 1;
    return(NULL);
}

int main(int argc, char *argv[])
{

    char tmpname[10];
    unsigned int cnum=0;
    int i;
    int tmp;
    int wrong = 0;
    int wrong1 = 0, wrong2 = 0;
    unsigned int linkstatus = 0;

    // parser input option
    if (argc != 2)
    {
        printf("Wrong args,exiting\n");
        exit(0);
    }
    strncpy(tmpname,argv[1],3);
    if(strncmp(tmpname,"nac",3))
    {
        printf("Wrong card name.\n");
        exit(0);
    }
    if (!isdigit(argv[1][3]))
    {
        printf("Wrong card number.\n");
        exit(0);
    }
    strncpy(tmpname,argv[1]+3,5);
    sscanf(tmpname,"%u",&cnum);
    if (cnum > 9)
    {
        printf("Unsupported card number %u, must be in 0-9.\n",cnum);
        exit(0);
    }
    strncat(nacname,argv[1],20);


    signal(SIGTERM, cleanup);
    signal(SIGINT,  cleanup);
    signal(SIGPIPE, brokenPipe);

    // init thread signals
    report_stop = 0;
    memset(an_stop, 0, sizeof(char)*NUM_STREAM);

    // init counters
    memset(totalBytes,0,sizeof(uint64_t)*NUM_STREAM);
    memset(totalPackets,0,sizeof(uint64_t)*NUM_STREAM);

    // Use API, open nac card
    if ((nacfd = nac_open(nacname)) < 0)
    {
        printf("Fail to open device %s.\n",nacname);
        exit(0);
    }

    // config number of stream
    if(nac_inline_msu_configure(nacfd,NUM_STREAM,COPY_FWD) < 0)
    {
        printf("nac_inlien_msu_configure failed\n");
        return 0;
    }

    linkstatus = reg_read(nacfd, 0x900dc);
    printf("physical link status is: 0x%08X\n", linkstatus);

    //start report thread
    pthread_create(&reportThread, NULL, anReport, NULL);
//	pthread_create(&pulseThread, NULL, pulse, NULL);
    //start rft thread
    //pthread_create(&rftThread, NULL, test_rft, NULL);
    // initialize thread args
    for(i=0; i<NUM_STREAM; i++)
    {
        anThreadArg[i].stream_num = i;
        anThreadArg[i].maxwait.tv_usec = 1* 1000; 	// 1 ms max wait time
        anThreadArg[i].mindata = 64 * 1024; //64KB min
        anThreadArg[i].poll.tv_usec = 50; // 50 us poll time
    }



//		pthread_create(&anTxThread[0], NULL,(void *)run_tx, (void*)&anThreadArg[0]);
    for(i=0; i<NUM_STREAM; i++)
    {
        pthread_create(&anRxThread[i], NULL,(void *)run_rx, (void*)&anThreadArg[i]);
        sleep(2);
        pthread_create(&anTxThread[i], NULL,(void *)run_tx, (void*)&anThreadArg[i]);
        //	sleep(10);
        //	pthread_create(&anRxThread[i], NULL,(void *)run_rx, (void*)&anThreadArg[i]);
    }
    //pthread_create(&anTxThread[0], NULL,(void *)run_tx, (void*)&anThreadArg[0]);
    capture_start(nacfd);
    //		pthread_create(&anTxThread[0], NULL,(void *)run_tx, (void*)&anThreadArg[0]);

    //	pthread_create(&anTxThread[1], NULL,(void *)run_tx, (void*)&anThreadArg[1]);
    //	pthread_create(&anTxThread[i], NULL,(void *)run_tx, (void*)&anThreadArg[i]);
    //	pthread_create(&anTxThread[i], NULL,(void *)run_tx, (void*)&anThreadArg[i]);

    //join rft threaad
    //pthread_join(rftThread, NULL);

    for(i=0; i<NUM_STREAM; i++)
    {
        pthread_join(anRxThread[i], NULL);
        pthread_join(anTxThread[i], NULL);
    }

//	pthread_join(anTxThread[2], NULL);
    pthread_join(reportThread, NULL);
//	pthread_join(pulseThread, NULL);


    return 0;

}



