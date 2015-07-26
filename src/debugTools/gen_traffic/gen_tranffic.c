#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <arpa/inet.h>

typedef struct erf_record {
        unsigned int          ts[2];
        unsigned char           type;
        unsigned char           flags;
        unsigned short          rlen; 
        unsigned short          lctr;
        unsigned short          wlen;
} erf_record_t;

int main(int argc,char **argv)
{  //unsigned int i=0;
   erf_record_t head;
   FILE *fp;
   unsigned short rlen,lctl,wlen;
   unsigned int j,i;
   unsigned char a=0;

   fp = fopen("test.erf","wb");
   if (fp == NULL) return(-1);

   for (i=0;i<100;i++){
 //  for (i=0;i<30;i++){
      memset(&head,0,sizeof(erf_record_t)); 
      head.ts[0]=htonl(i);
      head.ts[1]=htonl(i+1);
      lctl=atoi(argv[1]);
     // lctl=i%4;
      printf("lctl = %d\n",lctl);
      head.lctr=htons(lctl);
      wlen = (unsigned short)(i+64);
      head.wlen = htons(wlen);
                
    // head.wlen = htons(111);           
      if (i%16 == 0)
        rlen = (unsigned short)i+64+16;
      else
        rlen = (((unsigned short)i+64)/16 + 1)*16+16;
    
    // fix rlen to 128
   //  rlen = 128;    

     head.rlen = htons(rlen);
     printf("rlen = 0x%x, wlen=0x%x\n",head.rlen,head.wlen);
     fwrite(&head,1,sizeof(erf_record_t),fp);
     a= (unsigned char )(i+1);
    for (j=0;j<rlen-16;j++){
      fwrite(&a,1,1,fp);
     }
   }
   fclose(fp);
   return 0;
}   

