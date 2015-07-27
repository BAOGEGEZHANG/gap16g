/********************************
 * flow_test.c
 * Only used with forward card
 * version: 1.0
 * 2010.7.5
 *********************************/

#include "flow_test.h"
#include <openssl/md5.h>


#define WRITEFILE 1

//#define CONFIG_TX  1


//#define DEBUG
#define Kbps    1000
#define Mbps    1000000
static char nacname[20] = "/dev/";
int nacfd = -1;
char shutdownInProgress = 0;
static uint64_t totalBytes[NUM_STREAM];
static uint64_t totalPackets[NUM_STREAM];

int refresh_num=0;
unsigned char report_stop;
unsigned char an_stop[NUM_STREAM];




typedef struct an_arg
{
				int stream_num;
				struct timeval maxwait;
				struct timeval poll;
				unsigned int mindata;
				char file_name[16];
} an_arg_t;

typedef struct file_header
{
				uint32_t    size;
				uint32_t    chip;
				uint32_t     tx_count;   
				uint8_t     name[16];
				uint8_t     md5[16];
				FILE        *fp;
} file_header_t;

typedef struct node
{
	uint64_t seqnum;
	unsigned char *pos;
} node_t;


uint64_t block_seqnum[10];
uint64_t block_seqnum_count;
node_t list[10];
int flag_fp_status = 1;

an_arg_t anThreadArg[NUM_STREAM];
pthread_t anRxThread[NUM_STREAM];
pthread_t anTxThread[NUM_STREAM];

pthread_t reportThread;



void print_md5(char *buf)
{
				int i;
				printf("MD5: ");
				for(i=0;i<16;i++)
								printf("%02x",(unsigned char)buf[i]);
				printf("\n");
}    

int compare_md5(char *old, char *new)
{
				int i;
				for(i=0;i<16;i++)
								if (old[i] != new[i])
								{
												printf("Error, md5 not correct!\n old:");
												print_md5(old);
												printf("new: ");
												print_md5(new);
												return 1;
								}
				printf("md5sum is OK\n");
				return 0;
}   
/**********************************************************/
void shutdown_flow(void)
{

				int i;
				capture_stop(nacfd);
				shutdownInProgress = 1;

				printf("Waiting for thread to stop...\n");
				while (!report_stop) sleep(1);
				printf("Report thread stopped.\n");
#ifndef CONFIG_TX
				for (i=0;i<NUM_STREAM;i++)
				{
								while(!an_stop[i]) sleep(1);
								printf("Analyze thread for stream %d stopped.\n",i);
				}
#endif

				for (i=0;i < NUM_STREAM;i++)
				{

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
				for (i=0;i<len;i++){
								if (i%16 == 0) {
												printf("\n[0x%04x]  ",i);
								}
								//	printf("%02x ",s[i]);
								printf ("[%c]",s[i]);
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

				uint8_t md5[16];
				uint8_t md5_result[16];
				unsigned int verify_seq=1;
				MD5_CTX c;
				// init bottom pointer to NULL
				bottom = NULL;


				file_header_t file;


				cnt=ts=0;
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
				uint32_t seqnum;
				uint8_t tmp_filename[24];
				uint8_t *buffer_content;
				uint8_t *buffer_block;
				uint32_t buffer_block_count;

				buffer_block_count = 0;
				buffer_block = malloc (1600 * 10);
				if (NULL == buffer_block)
				{
								printf ("run_rx:malloc buffer_block space failed\n");
								return NULL;
				}
				memset(buffer_block, 0x00, 1600 * 10);

				// get data
				while(!shutdownInProgress)
				{
								if(( top = nac_advance_stream(nacfd, stream_num, &bottom)) == NULL) 
								{
												printf("Error, nac_advance_stream %d return NULL pointer.\n",stream_num);
												goto PEND;
								}

								while(((top-bottom) > RECORD_HDR_SIZE)) 
								{
												erfhdr      = (erf_record_t *)bottom;
												rlen        = ntohs(erfhdr->rlen);
												wlen        = ntohs(erfhdr->wlen);
												file.size   = ntohl(erfhdr->ts[0]);
												seqnum      = ntohl(erfhdr->ts[1]);


												if((top-bottom) < rlen)
																break;
#if 1
												if ( 1 == seqnum )
												{
																printf ("run_rx: flag_fp_status:%d\n",flag_fp_status);
															
																if (!flag_fp_status)
																{
																				printf ("run_rx:last file not close\n");	
																				uint8_t loop;
																				for (loop = 0; loop < 10; loop++)
																				{
																								if (list[loop].seqnum == 0)
																												break;
																				}	
																				if (loop == 10)
																				{
																								printf ("run_rx:lost too much chip\n");
																								return NULL;
																				}
																				else{
																								list[loop].seqnum = seqnum;
																								list[loop].pos = malloc (1600);
																								if ( list[loop].pos == NULL)
																								{
																												printf ("run_rx:malloc new node error");
																												return NULL;
																								}
																								memcpy ( list[loop].pos ,bottom , 1600);	
																								block_seqnum_count++;
																								goto NEXT;
																				}
																}
																else{
																				flag_fp_status = 0;
																				MD5_Init(&c);
																				//	printf ("run_rx:file.size:%d\n",file.size);

																				verify_seq=1;
																				memcpy (file.name, (char *)bottom + 16, 16); 
																				memcpy (file.md5, (char *)bottom + 32, 16);
																				printf ("run_rx:file.name:%s\n",file.name);
																				sprintf (tmp_filename, "/dev/shm/%s",file.name);
																				if ((file.fp = fopen( tmp_filename, "wb")) == NULL)
																				{
																								printf("fail to open file %s\n",tmp_filename);
																								exit(-2);
																				}
																}

												}
#endif
												if (seqnum != verify_seq){
																printf("Bad seq %u, expect %u\n",seqnum, verify_seq);
													//			verify_seq = seqnum;
#if 1
															uint8_t loop;
															for (loop = 0; loop < 10; loop++)
															{
																if (list[loop].seqnum == 0)
																	break;
															}	
															if (loop == 10)
															{
																printf ("run_rx:lost too much chip\n");
																return NULL;
															}
															else{
																list[loop].seqnum = seqnum;
																list[loop].pos = malloc (1600);
																if ( list[loop].pos == NULL)
																{
																	printf ("run_rx:malloc new node error");
																	return NULL;
																}
																memcpy ( list[loop].pos ,bottom , 1600);	
															}

															block_seqnum_count++;
															printf ("run_rx:add seqnum block_seqnum_count:%d\n",block_seqnum_count);

															goto NEXT;
#endif
												}
												else if ( block_seqnum_count )
												{
														fwrite ( bottom + 48, wlen - 32, 1, file.fp);
														MD5_Update(&c, bottom + 48, wlen-32);

														if (wlen != rlen - 16)
														{
																flag_fp_status = 1;
																printf ("run_rx:[bloc_seqnum_count:%d]seqnum:%d file get the last packet \n",block_seqnum_count,seqnum);
																fclose(file.fp);
																MD5_Final(md5_result,&c);
																print_md5(md5_result);
																compare_md5(file.md5,md5_result);

																verify_seq  = 1;
										
														}
														else
															verify_seq ++;

														uint8_t loop;
FIND:
														for (loop = 0; loop < 10 ; loop++)
														{
															if (list[loop].seqnum == verify_seq)
																		break;

														}
		
														if (loop == 10)	
														{
															goto NEXT;
														}

														if (list[loop].seqnum == 1)
														{
																				flag_fp_status = 0;
																				MD5_Init(&c);
																				//	printf ("run_rx:file.size:%d\n",file.size);

																				verify_seq=1;
																				memcpy (file.name, (char *)bottom + 16, 16); 
																				memcpy (file.md5, (char *)bottom + 32, 16);
																				printf ("run_rx:file.name:%s\n",file.name);
																				sprintf (tmp_filename, "/dev/shm/%s",file.name);
																				if ((file.fp = fopen( tmp_filename, "wb")) == NULL)
																				{
																								printf("fail to open file %s\n",tmp_filename);
																								exit(-2);
																				}

														}



														fwrite ( list[loop].pos + 48, 1600 - 48, 1, file.fp);
														MD5_Update(&c, list[loop].pos + 48, 1600 - 48);

														list[loop].seqnum = 0;
														free(list[loop].pos);
														list[loop].pos = NULL;
														block_seqnum_count--;
														printf ("run_rx:sub seqnum block_seqnum_count:%d\n",block_seqnum_count);

														verify_seq++;
														goto FIND;
														
											}	

											  verify_seq++;

												fwrite ( bottom + 48, wlen - 32, 1, file.fp);
												MD5_Update(&c, bottom + 48, wlen-32);


												//						fseek(file.fp,(1600 - 48)*(seqnum - 1),SEEK_SET);
												//						fwrite ( bottom + 48, wlen - 32, 1, file.fp);
												//						fflush(file.fp);

												totalBytes[0] += wlen - 32;
												totalPackets[0] ++;

												if (wlen != rlen - 16)
												{

																flag_fp_status = 1;
																printf ("run_rx:seqnum:%d file get the last packet\n",seqnum);
																fclose(file.fp);
																MD5_Final(md5_result,&c);
																print_md5(md5_result);
																compare_md5(file.md5,md5_result);
																//		goto PEND;

												}
NEXT:
												bottom += rlen;
								}
								
				}

PEND:
				an_stop[stream_num] = 1;
				int loop;

				if (file.size != totalBytes[0])
				{
								printf ("run_rx:file.size:%d totalBytes[0]:%d\n",file.size,totalBytes[0]);
								printf ("run_rx:get file broken;\n");
								return NULL;
				}



				printf ("++++++++++++++Translate the file OK++++++++++++++\n");
				totalBytes[0] =0;
				totalPackets[0] = 0;
				//			goto NEXT;

				return(NULL);



}






static void *run_tx(an_arg_t* arg)
{
				uint32_t fsize=0;
				uint32_t seqnum ;
				file_header_t file;

				int stream_num			= arg->stream_num;
				unsigned int mindata	= arg->mindata;
				struct timeval maxwait	= arg->maxwait;
				struct timeval poll 	= arg->poll;
				strcpy (file.name,arg->file_name);

				uint8_t * ret;
				int i=0,j=0,tx_times=0;

				uint8_t * cp_org;
				uint8_t * send_buf;

				unsigned int trans_time=0;

				printf("start tx %d\n",stream_num);

				uint64_t len;
				uint8_t md5[16];
				MD5_CTX c;   

				file.fp = fopen( file.name,"rb");
				printf("run_tx:file.fp open\n");	

				if (file.fp == NULL) return(-3);

				fseek(file.fp,0,SEEK_END);

				file.size= ftell(file.fp);
				printf ("run_tx:Get file.size:%d\n",file.size);

				fseek(file.fp,0,SEEK_SET);

				send_buf = malloc(file.size);
				if (NULL == send_buf)
				{
								printf ("run_tx:malloc send buff failed\n");
								exit(-2);
				}
				memset(send_buf,0,file.size);
				printf ("run_tx:malloc send_buff\n");
				MD5_Init(&c);
				printf ("run_tx:MD5 init\n");

				while (0 !=  (len = fread(send_buf,1,file.size,file.fp)))
				{

								printf ("run_tx:md5 update is running\n");
								MD5_Update(&c, send_buf, len);
				}

				printf ("run_tx:md5 update is ok\n");
				MD5_Final(file.md5,&c);
				printf ("run_tx: md5 final\n");

				fclose(file.fp);
				print_md5(file.md5);
				printf ("run_tx:Get file into buffer\n");


				file.chip = 1600 - 48; 
				file.tx_count = (file.size / file.chip) + 1;
				printf ("run_tx:file.size:%d file.chip:%d file.tx_count:%d\n",file.size, file.chip,file.tx_count);

				uint8_t *buffer_packet;
				buffer_packet = malloc (1600);
				if (NULL == buffer_packet)
				{
								perror("run_tx: malloc buffer_packet failed\n");
								return NULL;
				}

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


				while (!shutdownInProgress){
								printf("Trans times: %d\n",++trans_time);
								seqnum = 1;

								while (1){

												erf_record_t pkt_header;
												pkt_header.ts[0] = htonl(file.size);
												pkt_header.ts[1] = htonl(seqnum);
												pkt_header.rlen = htons(1600);

												memset (buffer_packet, 0, pkt_header.rlen);

												memcpy (buffer_packet + 16, file.name, 16);
												//print_buf(buffer_packet, 1600);

												memcpy (buffer_packet + 16 + 16, file.md5, sizeof(file.md5));
												//print_buf(buffer_packet, 1600);


												if (seqnum < file.tx_count) 
												{
																pkt_header.wlen = htons(file.chip + 32 );          //file.chip - erf_header(16byte)
																memcpy (buffer_packet, &pkt_header, 16);
																memcpy (buffer_packet + 48,send_buf + (seqnum - 1) * file.chip, file.chip);
												}
												else
												{
																pkt_header.wlen = htons( 32 + file.size % file.chip);

																memcpy (buffer_packet, &pkt_header, 16);

																memcpy (buffer_packet + 48, send_buf + (seqnum - 1) * file.chip, file.size % file.chip); 
												}



												//		printf ("run_tx:seqnum:%d,wlen:%d\n",seqnum,ntohs(pkt_header.wlen) );
												//			print_buf(buffer_packet, 1600);
												cp_org = nac_tx_get_stream_space(nacfd,(stream_num + NUM_STREAM),1600);
												if (NULL == cp_org)
												{
																perror("nac_tx_get_stream_space,return");
																return NULL;
												}

												memcpy(cp_org,buffer_packet, 1600);
												tx_times++;

												ret = nac_tx_stream_commit_bytes(nacfd,(stream_num + NUM_STREAM), 1600);

												if (NULL == ret)
												{
																perror("nac_tx_stream_commit_bytes,return");
																return NULL;
												}
												if (seqnum >= file.tx_count)
												{
																printf ("file.name:%s send over...\n",file.name);
																seqnum = 0;
																break;
												}
												seqnum++;
								}
								sleep(5);
				}
TX_END:


				if(stream_num == 0){
								printf("stream 0 stopped\n");
				}
				printf ("run_tx:end\n");
}


void* anReport(void * unUsed) {

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

				while(!shutdownInProgress) {
								sleep(SCAN_CYCLE);	
								gettimeofday(&end,NULL);
								timeuse = 1000000 * (end.tv_sec - start.tv_sec) + end.tv_usec - start.tv_usec;
								t=time(NULL);
								strftime(tmp,sizeof(tmp),"%F %T",localtime(&t));
								printf("%s\n",tmp);
								for (i=0;i<NUM_STREAM;i++){

#if 1
												printf("Stream %d: %f Mbps, %f Mpps, total: %d \n",i,(double)(8*(totalBytes[i] - bytesCnt[i]))/(double)timeuse, 
																				(double)(totalPackets[i]-packetCnt[i])/(double)timeuse,totalPackets[i]-packetCnt[i]);

#endif
								}

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
				uint8_t tx_file[16];
				unsigned int cnum=0;
				int i;
				int tmp;
				int wrong = 0;
				int wrong1 = 0, wrong2 = 0;
				unsigned int linkstatus = 0;

				// parser input option
				if (argc != 3){
								printf("Wrong args,exiting\n");
								exit(0);
				}
				strncpy(tmpname,argv[1],3);
				if(strncmp(tmpname,"nac",3)){
								printf("Wrong card name.\n");
								exit(0);
				}
				if (!isdigit(argv[1][3])){
								printf("Wrong card number.\n");
								exit(0);
				}
				strncpy(tmpname,argv[1]+3,5);
				sscanf(tmpname,"%u",&cnum);
				if (cnum > 9){
								printf("Unsupported card number %u, must be in 0-9.\n",cnum);
								exit(0);
				}
				strncat(nacname,argv[1],20);
				strcpy (tx_file, argv[2]);

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

				for(i=0; i<NUM_STREAM; i++){
								anThreadArg[i].stream_num = i;
								anThreadArg[i].maxwait.tv_usec = 1* 1000; 	// 1 ms max wait time
								anThreadArg[i].mindata = 64 * 1024;         // 64KB min
								anThreadArg[i].poll.tv_usec = 50;           // 50 us poll time
								strcpy(anThreadArg[i].file_name,tx_file);
				}

				for(i=0; i<NUM_STREAM; i++){
#ifndef CONFIG_TX
								pthread_create(&anRxThread[i], NULL,(void *)run_rx, (void*)&anThreadArg[i]);
#else
								sleep(2);
								pthread_create(&anTxThread[i], NULL,(void *)run_tx, (void*)&anThreadArg[i]);
#endif
				}
				capture_start(nacfd);

				for(i=0; i<NUM_STREAM; i++){ 
#ifndef CONFIG_TX    		
								pthread_join(anRxThread[i], NULL); 
#else         
								pthread_join(anTxThread[i], NULL);   
#endif     
				} 
				pthread_join(reportThread, NULL);

				return 0;  
}