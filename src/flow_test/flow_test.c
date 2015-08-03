/********************************
 * flow_test.c
 * Only used with forward card
 * version: 1.0
 * 2010.7.5
 *********************************/

#include "flow_test.h"


//#define WRITEFILE 
//#define MD5_CAL

#ifdef DEBUG
	#define DEBUG_INFO(MSG)   printf MSG
#else 
	#define DEBUG_INFO
#endif

#ifdef MD5_CAL
#include <openssl/md5.h>
#endif

//#define CONFIG_TX  1


//#define DEBUG
#define Kbps    1000
#define Mbps    1000000
static char nacname[20] = "/dev/";
int nacfd = -1;
char shutdownInProgress = 0;
static uint64_t totalBytes[NUM_STREAM];
static uint64_t totalPackets[NUM_STREAM];

int refresh_num = 0;
unsigned char report_stop;
unsigned char an_stop[NUM_STREAM];

time_t pro_start, pro_end;
uint64_t rx_right_cnt ;
uint64_t rx_err_cnt;

#ifdef MD5_CAL
MD5_CTX c;
#endif
uint8_t md5[16];
uint8_t md5_result[16];


uint8_t log_name[64];


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
node_t list[10];

an_arg_t anThreadArg[NUM_STREAM];
pthread_t anRxThread[NUM_STREAM];
pthread_t anTxThread[NUM_STREAM];

pthread_t reportThread;



void print_md5(char *buf)
{
	int i;
	DEBUG_INFO(("MD5: "));

	for (i = 0; i < 16; i++)
		{ DEBUG_INFO(("%02x", (unsigned char)buf[i])); }

	DEBUG_INFO(("\n"));
}

int compare_md5(char *old, char *new)
{
	int i;

	for (i = 0; i < 16; i++)
		if (old[i] != new[i])
		{
			DEBUG_INFO(("Error, md5 not correct!\n old:"));
			print_md5(old);
			DEBUG_INFO(("new: "));
			print_md5(new);
			rx_err_cnt++;
			return 1;
		}

	printf("md5sum is OK\n");
	return 0;
}

static int init_listspace()
{
	uint8_t loop;

	for (loop = 0; loop < 10; loop++)
	{
		list[loop].pos = malloc(1600);

		if (NULL == list[loop].pos)
		{
			printf("malloc list space is failed\n");
			goto failed;
		}
	}

	return 0;
failed:

	while (loop)
	{
		free(list[loop].pos);
		list[loop].pos = NULL;
		loop--;
	}

	exit(-1);
}

static void clean_listspace()
{
	uint8_t loop;

	for (loop = 0; loop < 10; loop++)
	{
		if (list[loop].pos == NULL)
			{ continue; }

		free(list[loop].pos);
		list[loop].pos = NULL;
	}
}
/**********************************************************/
void shutdown_flow(void)
{
	int i;
	capture_stop(nacfd);
	shutdownInProgress = 1;
	printf("Waiting for thread to stop...\n");

	while (!report_stop) { sleep(1); }

	printf("Report thread stopped.\n");
#ifndef CONFIG_TX

	for (i = 0; i < NUM_STREAM; i++)
	{
		while (!an_stop[i]) { sleep(1); }

		printf("Analyze thread for stream %d stopped.\n", i);
	}

#endif

	for (i = 0; i < NUM_STREAM; i++)
	{
		if (nac_stop_stream(nacfd, i) < 0)
		{
			printf("Unable to stop stream %d\n", i);
			exit(-1);
		}

		if (nac_detach_stream(nacfd, i) < 0)
		{
			printf("Unable to detach stream %d\n", i);
			exit(-1);
		}

		if (nac_tx_stop_stream(nacfd, (i + NUM_STREAM), COPY_FWD) < 0)
		{
			printf("Unable to stop stream %d\n", (i + NUM_STREAM));
			exit(-1);
		}

		if (nac_tx_detach_stream(nacfd, (i + NUM_STREAM), COPY_FWD) < 0)
		{
			printf("Unable to detach stream %d\n", (i + NUM_STREAM));
			exit(-1);
		}
	}

	nac_close(nacfd);
	nacfd = -1;
	clean_listspace();
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
	int i = 0;

	for (i = 0; i < len; i++)
	{
		if (i % 16 == 0)
		{
			printf("\n[0x%04x]  ", i);
		}

		//	printf("%02x ",s[i]);
		printf("[%c]", s[i]);
	}

	printf("\n");
}



static int find_nextpacketinlist(unsigned int *verify, node_t *list, file_header_t *file)
{
	unsigned int verify_seqnum;
	int loop;
	uint8_t tmp_filename[16];
	verify_seqnum = *verify;
FIND:

	for (loop = 0; loop < 10; loop++)
	{
		if (list[loop].seqnum == verify_seqnum)
			{ break; }
	}

	if (10 == loop)
	{
		*verify = verify_seqnum;
		return 1;	/*not found */
	}
	else
	{
		erf_record_t *tmp_erfhdr;
		unsigned short tmp_rlen, tmp_wlen;
		tmp_erfhdr      = (erf_record_t *)list[loop].pos;
		tmp_rlen        = ntohs(tmp_erfhdr->rlen);
		tmp_wlen        = ntohs(tmp_erfhdr->wlen);

		if (1 == verify_seqnum)
		{
#ifdef MD5_CAL
			MD5_Init(&c);
#endif
			verify_seqnum = 1;
			memcpy(file->name, list[loop].pos + 16, 16);
			memcpy(file->md5, list[loop].pos + 32, 16);
			DEBUG_INFO(("[%s]:file.name:%s\n",__func__, file->name));
			sprintf(tmp_filename, "/dev/shm/%s", file->name);

			if ((file->fp = fopen(tmp_filename, "wb")) == NULL)
			{
				printf("fail to open file %s\n", tmp_filename);
				exit(-2);
			}
#ifdef WRITEFILE
			fwrite(list[loop].pos + 48, 1600 - 48, 1, file->fp);
#endif
#ifdef MD5_CAL
			MD5_Update(&c, list[loop].pos + 48, 1600 - 48);
#endif
			list[loop].seqnum = 0;
			verify_seqnum++;
		}
		else if (tmp_wlen != tmp_rlen - 16)
		{
#ifdef WRITEFILE
			fwrite(list[loop].pos + 48, tmp_wlen - 32, 1, file->fp);
#endif
#ifdef MD5_CAL
			MD5_Update(&c, list[loop].pos + 48, tmp_wlen - 32);
#endif
			list[loop].seqnum = 0;
			DEBUG_INFO(("{%s]:seqnum:%d file get the last packet \n", __func__,verify_seqnum));
			fclose(file->fp);
#ifdef MD5_CAL
			MD5_Final(md5_result, &c);
			print_md5(md5_result);
			compare_md5(file->md5, md5_result);
#endif
			verify_seqnum = 1;
      rx_right_cnt ++;
		}
		else
		{
			DEBUG_INFO (("[%s]:write seqnum:%d into file\n", __func__, verify_seqnum));
#ifdef WRITEFILE
			fwrite(list[loop].pos + 48, tmp_wlen - 32, 1, file->fp);
#endif
#ifdef MD5_CAL
			MD5_Update(&c, list[loop].pos + 48, tmp_wlen - 32);
#endif
			list[loop].seqnum = 0;
			verify_seqnum++;
		}

		goto FIND;
	}
}




static void *run_rx(an_arg_t *arg)
{
	unsigned char *top;
	unsigned char *bottom;
	int j;
	uint8_t *cp_org;
	unsigned long  accumulated = 0;
	uint32_t tsc_before, tsc_after;
	uint32_t tsc_one_ms;
	uint32_t uNacApiTxCopyCnt = 0;
	uint32_t drop_pkt_cnt = 0;
	erf_record_t *erfhdr;
	uint8_t *p;
	uint8_t card_fwd;
	IpHeader       *ip_hdr;
	unsigned short	rlen;
	unsigned short last_rlen = 0;
	unsigned short	align_rlen;
	unsigned short	wlen;
	pkt_info_arg_t pkt_info;
	unsigned int	src_ip;
	unsigned int	dst_ip;
	unsigned char	protocol;
	unsigned short	src_port;
	unsigned short	dst_port;
	int err, cnt, ts;
	int out_port;
	uint8_t *ret;
	int stream_num  		= arg->stream_num;
	unsigned int mindata 	= arg->mindata;
	struct timeval maxwait	= arg->maxwait;
	struct timeval poll		= arg->poll;
	FILE *fpx;
	unsigned int verify_seq = 1;
	// init bottom pointer to NULL
	bottom = NULL;
	file_header_t file;
	cnt = ts = 0;

	// map stream to buffers
	if (nac_attach_stream(nacfd, stream_num, 0, 0) < 0)
	{
		printf("nac configure failed, not attached\n");
		return(NULL);
	}

	// set polling parameters
	if (nac_set_stream_poll(nacfd, stream_num, mindata, &maxwait, &poll) < 0)
	{
		printf("set stream poll parameter failed\n");
		return(NULL);
	}

	// start capturing data
	if (nac_start_stream(nacfd, stream_num) < 0)
	{
		printf("nac_start_stream failed\n");
		return(NULL);
	}

	err = 0;
	uint32_t seqnum;
	uint8_t tmp_filename[24];
	init_listspace();

	// get data
	while (!shutdownInProgress)
	{
		if ((top = nac_advance_stream(nacfd, stream_num, &bottom)) == NULL)
		{
			printf("Error, nac_advance_stream %d return NULL pointer.\n", stream_num);
			goto PEND;
		}

		while (((top - bottom) > RECORD_HDR_SIZE))
		{
			erfhdr      = (erf_record_t *)bottom;
			rlen        = ntohs(erfhdr->rlen);
			wlen        = ntohs(erfhdr->wlen);
			file.size   = ntohl(erfhdr->ts[0]);
			seqnum      = ntohl(erfhdr->ts[1]);

			if ((top - bottom) < rlen)
				{ break; }

			if (seqnum == verify_seq)
			{
				if (1 == seqnum)	/*first packet of file*/
				{
#ifdef MD5_CAL
					MD5_Init(&c);
#endif
					memcpy(file.name, (char *)bottom + 16, 16);
					memcpy(file.md5, (char *)bottom + 32, 16);
					DEBUG_INFO(("run_rx:seqnum:%d file.name:%s\n", seqnum, file.name));
					sprintf(tmp_filename, "/dev/shm/%s", file.name);

					if ((file.fp = fopen(tmp_filename, "wb")) == NULL)
					{
						printf("fail to open file %s\n", tmp_filename);
						exit(-2);
					}
#ifdef WRITEFILE
					fwrite(bottom + 48, wlen - 32, 1, file.fp);
#endif
#ifdef MD5_CAL
					MD5_Update(&c, bottom + 48, wlen - 32);
#endif
					verify_seq++;
				}
				else if (wlen != rlen - 16) /*last packet of file*/
				{
#ifdef WRITEFILE
					fwrite(bottom + 48, wlen - 32, 1, file.fp);
#endif
#ifdef MD5_CAL
					MD5_Update(&c, bottom + 48, wlen - 32);
#endif
					fclose(file.fp);
#ifdef MD5_CAL
					MD5_Final(md5_result, &c);
					print_md5(md5_result);
					compare_md5(file.md5, md5_result);
#endif
					verify_seq = 1;
          rx_right_cnt ++;
				}
				else
				{
#ifdef WRITEFILE
					fwrite(bottom + 48, wlen - 32, 1, file.fp);
#endif
#ifdef MD5_CAL
					MD5_Update(&c, bottom + 48, wlen - 32);
#endif
					verify_seq++;
				}

				totalBytes[0] += wlen - 32;
				totalPackets[0] ++;
				find_nextpacketinlist(&verify_seq, list, &file);
			}
			else
			{
				uint8_t loop;

				for (loop = 0; loop < 10; loop++)
				{
					if (list[loop].seqnum == 0)
						{ break; }
				}

				if (loop == 10)
				{
					printf("run_rx:lost too much chip\n");
					return NULL;
				}
				else
				{
					list[loop].seqnum = seqnum;
					DEBUG_INFO (("run_rx:bad seqnum:%d ,verify_num:%d\n",seqnum, verify_seq));
					memcpy(list[loop].pos , bottom , 1600);
				}
			}

NEXT:
			bottom += rlen;
		}
	}

PEND:
	an_stop[stream_num] = 1;

  pro_end = time(NULL);
  /****************************************************************************/
  printf ("*****************************************************************\n");
  printf ("*\tTranslate file.name:%s\n", file.name);
  printf ("*\tTranslate Use_Time:%llu Min\n", (pro_end - pro_start) / 60);
  printf ("*\trx_file OK count:%llu\n", rx_right_cnt);
  printf ("*\trx_file ERROR count:%llu\n", rx_err_cnt);
  printf ("*\tTranslate aver-BandWith:%f Mbps\n",(double)( 8 *totalBytes[0] / 1000000)/(double)(pro_end - pro_start));
  printf ("*\tTranslate totalBytes:%llu totalPackets:%llu\n", totalBytes[0], totalPackets[0]);
  printf ("*****************************************************************\n");

	if (file.size != totalBytes[0])
	{
		printf("run_rx:file.size:%d totalBytes[0]:%d\n", file.size, totalBytes[0]);
		printf("run_rx:get file broken;\n");
		return NULL;
	}

	//			goto NEXT;
	return(NULL);
}



static void *run_tx(an_arg_t *arg)
{
	uint32_t fsize = 0;
	uint32_t seqnum ;
	file_header_t file;
	int stream_num			= arg->stream_num;
	unsigned int mindata	= arg->mindata;
	struct timeval maxwait	= arg->maxwait;
	struct timeval poll 	= arg->poll;
	strcpy(file.name, arg->file_name);
	uint8_t *ret;
	int i = 0, j = 0, tx_times = 0;
	uint8_t *cp_org;
	uint8_t *send_buf;
	unsigned int trans_time = 0;
	printf("start tx %d\n", stream_num);
	uint64_t len;
	uint8_t md5[16];
#ifdef MD5_CAL
	MD5_CTX c;
#endif
	file.fp = fopen(file.name, "rb");
	printf("run_tx:file.fp open\n");

	if (file.fp == NULL) { return(-3); }

	fseek(file.fp, 0, SEEK_END);
	file.size = ftell(file.fp);
	printf("run_tx:Get file.size:%d\n", file.size);
	fseek(file.fp, 0, SEEK_SET);
	send_buf = malloc(file.size);

	if (NULL == send_buf)
	{
		printf("run_tx:malloc send buff failed\n");
		exit(-2);
	}

	memset(send_buf, 0, file.size);
	printf("run_tx:malloc send_buff\n");
#ifdef MD5_CAL
	MD5_Init(&c);
#endif
	printf("run_tx:MD5 init\n");

	while (0 != (len = fread(send_buf, 1, file.size, file.fp)))
	{
		printf("run_tx:md5 update is running\n");
#ifdef MD5_CAL
		MD5_Update(&c, send_buf, len);
#endif
	}

	printf("run_tx:md5 update is ok\n");
#ifdef MD5_CAL
	MD5_Final(file.md5, &c);
#endif
	printf("run_tx: md5 final\n");
	fclose(file.fp);
	file.chip = 1600 - 48;
	file.tx_count = (file.size / file.chip) + 1;
	printf("run_tx:file.size:%d file.chip:%d file.tx_count:%d\n", file.size, file.chip, file.tx_count);
	uint8_t *buffer_packet;
	buffer_packet = malloc(1600);

	if (NULL == buffer_packet)
	{
		perror("run_tx: malloc buffer_packet failed\n");
		return NULL;
	}

	/*transmit stream map buffers*/
	if (nac_tx_attach_stream(nacfd, (stream_num + NUM_STREAM), COPY_FWD, 0) < 0)
	{
		printf("nac configure failed, not attached\n");
		return(NULL);
	}

	// set polling parameters
	if (nac_tx_set_stream_poll(nacfd, (stream_num + NUM_STREAM), mindata, &maxwait, &poll, COPY_FWD) < 0)
	{
		printf("set stream poll parameter failed\n");
		return(NULL);
	}

	// start capturing data
	if (nac_tx_start_stream(nacfd, (stream_num + NUM_STREAM), COPY_FWD) < 0)
	{
		printf("nac_start_stream failed\n");
		return(NULL);
	}

	/*************************/

	while (!shutdownInProgress)
	{
		printf("Trans times: %d\n", ++trans_time);
		seqnum = 1;

		while (1)
		{
			erf_record_t pkt_header;
			pkt_header.ts[0] = htonl(file.size);
			pkt_header.ts[1] = htonl(seqnum);
			pkt_header.rlen = htons(1600);
			memset(buffer_packet, 0, pkt_header.rlen);
			memcpy(buffer_packet + 16, file.name, 16);
			//print_buf(buffer_packet, 1600);
#ifdef MD5_CAL
			memcpy(buffer_packet + 16 + 16, file.md5, sizeof(file.md5));
#endif
			//print_buf(buffer_packet, 1600);

			if (seqnum < file.tx_count)
			{
				pkt_header.wlen = htons(file.chip + 32);           //file.chip - erf_header(16byte)
				memcpy(buffer_packet, &pkt_header, 16);
				memcpy(buffer_packet + 48, send_buf + (seqnum - 1) * file.chip, file.chip);
			}
			else
			{
				pkt_header.wlen = htons(32 + file.size % file.chip);
				memcpy(buffer_packet, &pkt_header, 16);
				memcpy(buffer_packet + 48, send_buf + (seqnum - 1) * file.chip, file.size % file.chip);
			}

			//		printf ("run_tx:seqnum:%d,wlen:%d\n",seqnum,ntohs(pkt_header.wlen) );
			//			print_buf(buffer_packet, 1600);
			cp_org = nac_tx_get_stream_space(nacfd, (stream_num + NUM_STREAM), 1600);

			if (NULL == cp_org)
			{
				perror("nac_tx_get_stream_space,return");
				return NULL;
			}

			memcpy(cp_org, buffer_packet, 1600);
			tx_times++;
			ret = nac_tx_stream_commit_bytes(nacfd, (stream_num + NUM_STREAM), 1600);

			if (NULL == ret)
			{
				perror("nac_tx_stream_commit_bytes,return");
				return NULL;
			}

			if (seqnum >= file.tx_count)
			{
				printf("file.name:%s send over...\n", file.name);
				seqnum = 0;
				break;
			}

			seqnum++;
		}

		sleep(5);
	}

TX_END:

	if (stream_num == 0)
	{
		printf("stream 0 stopped\n");
	}

	printf("run_tx:end\n");
}


void *anReport(void *unUsed)
{
	struct timeval start, end;
	uint64_t 	bytesCnt[NUM_STREAM], packetCnt[NUM_STREAM];
	double	 	byteSpeed, packetSpeed;
	uint64_t	timeuse = 0;
	int i;
	uint64_t rx_bytes0 = 0;
	uint64_t rx_bytes1 = 0;
	uint64_t rx_bytes2 = 0;
	uint64_t rx_bytes3 = 0;
	time_t t;
	char tmp[100];
	statistics_cnt_t port0_cnt, port1_cnt, port2_cnt, port3_cnt;
	memset(bytesCnt, 0, sizeof(uint64_t) * NUM_STREAM);
	memset(packetCnt, 0, sizeof(uint64_t) * NUM_STREAM);
	memset(&port0_cnt, 0, sizeof(statistics_cnt_t));
	memset(&port1_cnt, 0, sizeof(statistics_cnt_t));
	memset(&port2_cnt, 0, sizeof(statistics_cnt_t));
	memset(&port3_cnt, 0, sizeof(statistics_cnt_t));
	gettimeofday(&start, NULL);
	t = time(NULL);
	strftime(log_name, sizeof(log_name), "%F:%T", localtime(&t));
	strcat(log_name, ".csv");
	FILE *log_file ;
	log_file = fopen(log_name, "wb");

	if (NULL == log_file)
	{
		printf("[%s]:fopen %s failed\n", __func__, log_name);
		return NULL;
	}

	uint8_t tmp_buffer_log[128];
	memset(tmp_buffer_log, 0x00, sizeof(tmp_buffer_log));
	uint64_t count = 0;
	printf("************************************************\n");

	while (!shutdownInProgress)
	{
		sleep(SCAN_CYCLE);
		gettimeofday(&end, NULL);
		timeuse = 1000000 * (end.tv_sec - start.tv_sec) + end.tv_usec - start.tv_usec;
		t = time(NULL);
		strftime(tmp, sizeof(tmp), "%F %T", localtime(&t));
		printf("%s\n", tmp);

		for (i = 0; i < NUM_STREAM; i++)
		{
			printf("Stream %d: %f Mbps, %f Mpps, total: %d \n", i, (double)(8 * (totalBytes[i] - bytesCnt[i])) / (double)timeuse,
			       (double)(totalPackets[i] - packetCnt[i]) / (double)timeuse, totalPackets[i] - packetCnt[i]);
			memset(tmp_buffer_log, 0x00, sizeof(tmp_buffer_log));
			sprintf(tmp_buffer_log, "[%s],%f Mbps, %f Mpps, totalBytes:%llu, totalPackets:%llu \n", tmp, (double)(8 * (totalBytes[i] - bytesCnt[i])) / (double)timeuse,
			        (double)(totalPackets[i] - packetCnt[i]) / (double)timeuse,  totalBytes[i], totalPackets[i]);
			fputs(tmp_buffer_log, log_file);
		}

		memcpy(&start, &end, sizeof(struct timeval));
		memcpy(bytesCnt, totalBytes, sizeof(uint64_t) * NUM_STREAM);
		memcpy(packetCnt, totalPackets, sizeof(uint64_t) * NUM_STREAM);
		printf("************************************************\n");
		count++;
	}

	fclose(log_file);
	report_stop = 1;
	return(NULL);
}

int main(int argc, char *argv[])
{
	char tmpname[10];
	uint8_t tx_file[16];
	unsigned int cnum = 0;
	int i;
	int tmp;
	int wrong = 0;
	int wrong1 = 0, wrong2 = 0;
	unsigned int linkstatus = 0;

	// parser input option
	if (argc != 3)
	{
		printf("Wrong args,exiting\n");
		exit(0);
	}

	strncpy(tmpname, argv[1], 3);

	if (strncmp(tmpname, "nac", 3))
	{
		printf("Wrong card name.\n");
		exit(0);
	}

	if (!isdigit(argv[1][3]))
	{
		printf("Wrong card number.\n");
		exit(0);
	}

	strncpy(tmpname, argv[1] + 3, 5);
	sscanf(tmpname, "%u", &cnum);

	if (cnum > 9)
	{
		printf("Unsupported card number %u, must be in 0-9.\n", cnum);
		exit(0);
	}

	strncat(nacname, argv[1], 20);
	strcpy(tx_file, argv[2]);
	signal(SIGTERM, cleanup);
	signal(SIGINT,  cleanup);
	signal(SIGPIPE, brokenPipe);
	// init thread signals
	report_stop = 0;
	memset(an_stop, 0, sizeof(char)*NUM_STREAM);
	// init counters
	memset(totalBytes, 0, sizeof(uint64_t)*NUM_STREAM);
	memset(totalPackets, 0, sizeof(uint64_t)*NUM_STREAM);

  pro_start = time(NULL);

	// Use API, open nac card
	if ((nacfd = nac_open(nacname)) < 0)
	{
		printf("Fail to open device %s.\n", nacname);
		exit(0);
	}

	// config number of stream
	if (nac_inline_msu_configure(nacfd, NUM_STREAM, COPY_FWD) < 0)
	{
		printf("nac_inlien_msu_configure failed\n");
		return 0;
	}

	linkstatus = reg_read(nacfd, 0x900dc);
	printf("physical link status is: 0x%08X\n", linkstatus);
	//start report thread
	pthread_create(&reportThread, NULL, anReport, NULL);

	for (i = 0; i < NUM_STREAM; i++)
	{
		anThreadArg[i].stream_num = i;
		anThreadArg[i].maxwait.tv_usec = 1 * 1000; 	// 1 ms max wait time
		anThreadArg[i].mindata = 64 * 1024;         // 64KB min
		anThreadArg[i].poll.tv_usec = 50;           // 50 us poll time
		strcpy(anThreadArg[i].file_name, tx_file);
	}

	for (i = 0; i < NUM_STREAM; i++)
	{
#ifndef CONFIG_TX
		pthread_create(&anRxThread[i], NULL, (void *)run_rx, (void *)&anThreadArg[i]);
#else
		sleep(2);
		pthread_create(&anTxThread[i], NULL, (void *)run_tx, (void *)&anThreadArg[i]);
#endif
	}

	capture_start(nacfd);

	for (i = 0; i < NUM_STREAM; i++)
	{
#ifndef CONFIG_TX
		pthread_join(anRxThread[i], NULL);
#else
		pthread_join(anTxThread[i], NULL);
#endif
	}

	pthread_join(reportThread, NULL);
	return 0;
}
