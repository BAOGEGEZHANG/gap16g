#include "nacapi.h"
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
int main(void)
{
	int nacfd = -1;
	unsigned int menu;
	unsigned int addr;
	uint16_t value;
	unsigned char c;

	if((nacfd = nac_open("/dev/nac0")) < 0)
	{
		printf("open pcie card failed\n");
		return 0;
	}
	while(1){

	printf("1: write \n");
	printf("2: read \n");
	printf("3: exit \n");
	scanf("%d",&menu);

	switch (menu)
	{ case 1: 
		    printf("Input reg address\n");
		    scanf("%x",&addr);
		    printf("input value:\n");
		    scanf("%x",&value);
		    framer_write(nacfd,addr,value);
		    break;
	  case 2:
		    printf("Input reg address\n");
		    scanf("%x",&addr);
 		    value = framer_read(nacfd,addr);
		    printf("reg 0x%x value is 0x%x\n",addr,value);
		    break;
          case 3: goto end;
          default : break;
        }
 	menu=0;
      }
end:
    close(nacfd);
    return 0;
}
