#ifndef FLOW_TEST_H
#define FLOW_TEST_H

#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <time.h>
#include <pthread.h> 
#include <signal.h>
#include <fcntl.h>
#include <ctype.h>
#include <sys/types.h>

#include <netinet/in.h>

#include "nacapi.h"

//#define LOG_FILE

#define MEM_SWAP

#define BUCKET_SIZE		10000000
// max data to process per loop
#define MAX_PROCESS_LEN  (256<<10) //16MB

#define RECORD_HDR_SIZE 16
#ifdef MEM_SWAP
	#define PAYLOAD_SIZE	64
#else
	#define PAYLOAD_SIZE    1
#endif

#define MAX_SEARCH_TIME	5

#define SCAN_CYCLE		4

typedef unsigned long long ErfTime;

typedef struct ipHeader {
 unsigned char         ver_len;
 unsigned char         tos;
 unsigned short        total_len;
 unsigned short        id;
 unsigned short        flag_offset;
 unsigned char         ttl;
 unsigned char         protocol;
 unsigned short        checksum;
 unsigned int        	  src_ip;
 unsigned int        	  dst_ip;
 unsigned short	  src_port;
 unsigned short	  dst_port;
} IpHeader;


typedef struct hashBucket {
 unsigned char         protocol;
 unsigned int        	  src_ip;
 unsigned int        	  dst_ip;
 unsigned short	  src_port;
 unsigned short	  dst_port;
 unsigned char	  payload[PAYLOAD_SIZE];
 struct hashBucket *next;
} HashBucket;

#endif /* FLOW_TEST_H */

