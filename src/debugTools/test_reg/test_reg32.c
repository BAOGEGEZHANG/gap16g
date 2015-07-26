#include "nacapi.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
int main(void)
{
	int nacfd = -1;
	unsigned int menu;
	unsigned int addr;
	unsigned int value;
	unsigned char c;
	unsigned char mac=0;
	unsigned short addr16=0;

	if((nacfd = nac_open("/dev/nac0")) < 0)
	{
		printf("open pcie card failed\n");
		return 0;
	}
	while(1){

	printf("1: write fpga reg\n");
	printf("2: read fpga reg\n");
        printf("3: write mac config reg\n");
	printf("4: read mac config reg\n");
  	printf("5: read mac statistics\n");
  	printf("6: stop dma channel\n");
	printf("7: exit \n");
	scanf("%d",&menu);

	switch (menu)
	{ case 1: 
		    printf("Input reg address\n");
		    scanf("%x",&addr);
		    printf("input value:\n");
		    scanf("%x",&value);
		    reg_write(nacfd,addr,value);
		    break;
	  case 2:
		    printf("Input reg address\n");
		    scanf("%x",&addr);
 		    value = reg_read(nacfd,addr);
		    printf("reg 0x%x value is 0x%x\n",addr,value);
		    break;
            case 3:
 		  printf("Select MAC: 0: MAC0, 1: MAC0 mdio, 2: MAC1, 3: MAC1 mdio\n ");
                  scanf("%d",&mac);
                  printf("Input MAC write address: \n");
                  scanf("%x", &addr16);
                  printf("input value:\n");
                  scanf("%x", &value);
                  nac_mac_write(nacfd, mac, addr16, value);
                  break;
            case 4:
                  printf("Select MAC: 0: MAC0, 1: MAC0 mdio, 2: MAC1, 3: MAC1 mdio\n ");
                  scanf("%d",&mac);
                  printf("Input MAC read address: \n");
                  scanf("%x", &addr16);
                  value = nac_mac_read(nacfd, mac,addr16);
                  printf("MAC Cfg reg: 0x%x value is 0x%x\n", addr16, value);
                  break;
            case 5:
                  printf("Select MAC: 0: MAC0, 1: MAC0 mdio, 2: MAC1, 3: MAC1 mdio\n ");
                  scanf("%d",&mac);
                  printf("Input MAC Stats address: \n");
                  scanf("%x", &addr16);
                  value = nac_mac_read(nacfd,mac,addr16);
                  printf("MAC Stats reg: 0x%x lo value is 0x%x\n", addr16, value);
                  break;
           case 6:
                reg_write(nacfd,0x80000,0);
                reg_write(nacfd,0x80040,0);
                reg_write(nacfd,0x80080,0);
                reg_write(nacfd,0x800c0,0);
                reg_write(nacfd,0x80100,0);
                reg_write(nacfd,0x80140,0);
                reg_write(nacfd,0x80180,0);
                reg_write(nacfd,0x801c0,0);
                reg_write(nacfd,0x80200,0);
                reg_write(nacfd,0x80240,0);
                reg_write(nacfd,0x80280,0);
                reg_write(nacfd,0x802c0,0);
                reg_write(nacfd,0x80300,0);
                reg_write(nacfd,0x80340,0);
                reg_write(nacfd,0x80380,0);
                reg_write(nacfd,0x803c0,0);
                break;

         case 7: goto end;
          default : break;
        }
 	menu=0;
      }
end:
    close(nacfd);
    return 0;
}
