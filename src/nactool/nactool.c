#include "nactool.h"
//identify input arguments
static void parse_cmdline(int argc, char **argp)
{
	int i, k;

	for (i = 1; i < argc; i++) {
		switch (i) {
		case 1:
			for (k = 0; args[k].srt; k++)
				if (!strcmp(argp[i], args[k].srt) ||
				    !strcmp(argp[i], args[k].lng)) {
					mode = args[k].Mode;
					break;
				}
			if (mode == MODE_HELP ||
			    (!args[k].srt && argp[i][0] == '-'))
				show_usage(0);
			else	//no command match it's device name
				devname = argp[i];
			break;
		case 2:
			if (
			(mode == MODE_SSET) ||
			    (mode == MODE_GDRV) ||
			    (mode == MODE_GREGS)||
			    (mode == MODE_NWAY_RST) ||
			    (mode == MODE_TEST) ||
			    (mode == MODE_GEEPROM) ||
			    (mode == MODE_SEEPROM) ||
			    (mode == MODE_GPAUSE) ||
			    (mode == MODE_SPAUSE) ||
			    (mode == MODE_GCOALESCE) ||
			    (mode == MODE_SCOALESCE) ||
			    (mode == MODE_GSN) ||
			    (mode == MODE_SRING) ||
			    (mode == MODE_GOFFLOAD) ||
			    (mode == MODE_SOFFLOAD) ||
			    (mode == MODE_GSTATS) ||
			    (mode == MODE_DSN) ||
			    (mode == MODE_TR)	||
			    (mode == MODE_PHY)	||
			    (mode == MODE_PCS)	||
			    (mode == MODE_MAC)	||
			    (mode == MODE_ISP)	||
			    (mode == MODE_RXBW)	||
			    (mode == MODE_BYPASS) ) {
				devname = argp[i];
				break;
			}
			/* fallthrough */
		case 3:
			if (mode == MODE_TEST) {
				if (!strcmp(argp[i], "online")) {
				//	test_type = ONLINE;
				} else if (!strcmp(argp[i], "offline")) {
				//	test_type = OFFLINE;
				} else {
					show_usage(1);
				}
				break;
			} else if (mode == MODE_ISP) {
					sn_file = argp[i];
				if (sn_file == NULL)
					show_usage(1);
				break;
			}
			else if (mode == MODE_GSN) {
					bin_file = argp[i];
				if (bin_file == NULL)
					show_usage(1);
				break;
			}
			/* fallthrough */
		default:
		/*	if (mode == MODE_GREGS) {
				
				i = argc;
				break;
			}
			if (mode == MODE_GEEPROM) {
				
				i = argc;
				break;
			}
			if (mode == MODE_SEEPROM) {
				
				i = argc;
				break;
			}
			if (mode == MODE_SPAUSE) {
				
				i = argc;
				break;
			}
			if (mode == MODE_SRING) {
				
				i = argc;
				break;
			}
			if (mode == MODE_SCOALESCE) {
				
				i = argc;
				break;
			}
			if (mode == MODE_SOFFLOAD)	{
				
				i = argc;
				break;
			}	*/
			if (mode == MODE_SSET)	
			{
				 if (!strcmp(argp[i], "port"))
				 {
					i += 1;
					if (i >= argc)
						show_usage(1);
					if (!strcmp(argp[i], "1"))
						port_wanted = 0;
					else if (!strcmp(argp[i], "2"))
						port_wanted = 1;
					else if (!strcmp(argp[i], "3"))
						port_wanted = 2;
					else if (!strcmp(argp[i], "4"))
						port_wanted = 3;
					else
						show_usage(1);
					i += 1; //increase to match next parameter	
					if (i >= argc)
						show_usage(1);
					if (!strcmp(argp[i], "autoneg")) 
					{
						neg_changed = 1;
						i += 1;
						if (i != (argc - 1))
							show_usage(1);
						if (!strcmp(argp[i], "on")) 
						{
							autoneg_wanted = AUTONEG_ENABLE;
							
						}
						else if (!strcmp(argp[i], "off")) 
						{
							autoneg_wanted = AUTONEG_DISABLE;
						}
						else
						{
							show_usage(1);
						}
						break;
					} 
					
					 else if (!strcmp(argp[i], "speed")) {
						 mode_changed = 1;
						i += 1;
						if (i >= argc)
							show_usage(1);
						if (!strcmp(argp[i], "10"))
							speed_wanted = SPEED_10;
						else if (!strcmp(argp[i], "100"))
							speed_wanted = SPEED_100;
						else if (!strcmp(argp[i], "1000"))
							speed_wanted = SPEED_1000;
						else
							show_usage(1);
					 }
					 i = i+1;//increase to match next parameter	
					 if (i >= argc)
						show_usage(1);
					 if (!strcmp(argp[i], "duplex")) {
						i += 1;
						if (i >= argc)
							show_usage(1);
						if (!strcmp(argp[i], "half"))
							duplex_wanted = HALF;
						else if (!strcmp(argp[i], "full"))
							duplex_wanted = FULL;
						else
							show_usage(1);
						break;
					}  
					
					
				}
				
				
				
				/* else if (!strcmp(argp[i], "phyad")) {
				//	gset_changed = 1;
					i += 1;
					if (i >= argc)
						show_usage(1);
				//	phyad_wanted = strtol(argp[i], NULL, 0);
					//if (phyad_wanted < 0)
						//show_usage(1);
					break;
				} else if (!strcmp(argp[i], "xcvr")) {
					//gset_changed = 1;
					i += 1;
					if (i >= argc)
						show_usage(1);
					if (!strcmp(argp[i], "internal"))
						;//xcvr_wanted = XCVR_INTERNAL;
					else if (!strcmp(argp[i], "external"))
						;//xcvr_wanted = XCVR_EXTERNAL;
					else
						show_usage(1);
					break;
				} else if (!strcmp(argp[i], "wol")) {
					//gwol_changed = 1;
					i++;
					if (i >= argc)
						show_usage(1);
					//if (parse_wolopts(argp[i], &wol_wanted) < 0)
						//show_usage(1);
					//wol_change = 1;
					break;
				} else if (!strcmp(argp[i], "sopass")) {
					//gwol_changed = 1;
					i++;
					if (i >= argc)
						show_usage(1);
					//if (parse_sopass(argp[i], sopass_wanted) < 0)
						//show_usage(1);
					//sopass_change = 1;
					break;
				} else if (!strcmp(argp[i], "msglvl")) {
					i++;
					if (i >= argc)
						show_usage(1);
					//msglvl_wanted = strtol(argp[i], NULL, 0);
					//if (msglvl_wanted < 0)
						//show_usage(1);
					break;
				}*/
		}
		if (mode == MODE_BYPASS){
			if (!strcmp(argp[i], "bypass")) {
				i += 1;
				if (i >= argc)
					show_usage(1);
				if (!strcmp(argp[i], "on")) {
					bypass_wanted = BYPASS_ENABLE;
					
				} else if (!strcmp(argp[i], "off")) {
					bypass_wanted = BYPASS_DISABLE;
				} else {
					show_usage(1);
				}
				break;
			}
		}
			show_usage(1);
		}
	}

/*	if ((autoneg_wanted == AUTONEG_ENABLE) && (advertising_wanted < 0)) {
		if (speed_wanted == SPEED_10 && duplex_wanted == DUPLEX_HALF)
			advertising_wanted = ADVERTISED_10baseT_Half;
		else if (speed_wanted == SPEED_10 &&
			 duplex_wanted == DUPLEX_FULL)
			advertising_wanted = ADVERTISED_10baseT_Full;
		else if (speed_wanted == SPEED_100 &&
			 duplex_wanted == DUPLEX_HALF)
			advertising_wanted = ADVERTISED_100baseT_Half;
		else if (speed_wanted == SPEED_100 &&
			 duplex_wanted == DUPLEX_FULL)
			advertising_wanted = ADVERTISED_100baseT_Full;
		else if (speed_wanted == SPEED_1000 &&
			 duplex_wanted == DUPLEX_HALF)
			advertising_wanted = ADVERTISED_1000baseT_Half;
		else if (speed_wanted == SPEED_1000 &&
			 duplex_wanted == DUPLEX_FULL)
			advertising_wanted = ADVERTISED_1000baseT_Full;
		else if (speed_wanted == SPEED_2500 &&
			 duplex_wanted == DUPLEX_FULL)
			advertising_wanted = ADVERTISED_2500baseX_Full;
		else if (speed_wanted == SPEED_10000 &&
			 duplex_wanted == DUPLEX_FULL)
			advertising_wanted = ADVERTISED_10000baseT_Full;
		else
			// auto negotiate without forcing,
			 * all supported speed will be assigned in do_sset()
			 
			advertising_wanted = 0;

	}	
*/

	if (devname == NULL)
		show_usage(1);
	if (strcmp(devname,"nac0")&&strcmp(devname,"nac1")&&strcmp(devname,"nac2")&&strcmp(devname,"nac3"))	//需要进一步判断设备名字的健全性
		show_usage(1);
}
//show user help
static void show_usage(int badarg)
{
	int i;
	fprintf(stderr, " version " VERSION "\n");
	fprintf(stderr,
		"Usage:\n"
		"nactool DEVNAME\tDisplay standard information about device\n");
	for (i = 0; args[i].srt; i++) {
		if ( (args[i].srt=="-s")||(args[i].srt=="-S")||(args[i].srt=="-h")||(args[i].srt=="-b")||(args[i].srt=="-B") )
		fprintf(stderr, "        nactool %s|%s DEVNAME\t%s\n%s",
			args[i].srt, args[i].lng,
			args[i].help,
			args[i].opthelp ? args[i].opthelp : "");
	}
	exit(badarg);
}
//execute command
static int doit(void)
{
	int fd;
	char dev[DEV_LENGTH]= "/dev/";
	/* Open device. */
	if (NULL == strcat(dev,devname))
	{
		perror("strcat");
	}
	fd = nac_open(dev);	//设备名字需要进一步处理和判断
	if (fd < 0) {
		perror("nac_open");
		exit(EXIT_FAILURE);
	}

	/* all of these are expected to populate ifr->ifr_data as needed */
	if (mode == MODE_GDRV) {
		return 0;//do_gdrv(fd, &ifr);
	} else if (mode == MODE_GSET) {
		return do_gset(fd);
	} else if (mode == MODE_SSET) {
		return do_sset(fd);
	} else if (mode == MODE_GREGS) {
		return 0;//do_gregs(fd, &ifr);
	} else if (mode == MODE_NWAY_RST) {
		return 0;//do_nway_rst(fd, &ifr);
	} else if (mode == MODE_GEEPROM) {
		return 0;//do_geeprom(fd, &ifr);
	} else if (mode == MODE_SEEPROM) {
		return 0;//do_seeprom(fd, &ifr);
	} else if (mode == MODE_TEST) {
		return 0;//do_test(fd, &ifr);
	} else if (mode == MODE_ISP) {
		return do_isp(fd,sn_file);
	} else if (mode == MODE_GPAUSE) {
		return 0;//do_gpause(fd, &ifr);
	} else if (mode == MODE_DSN) {
		return do_dsn(fd);
	} else if	(mode == MODE_TR)	{
		return	do_testreg(fd);
	} else if (mode == MODE_SPAUSE) {
		return 0;//do_spause(fd, &ifr);
	} else if (mode == MODE_GCOALESCE) {
		return 0;//do_gcoalesce(fd, &ifr);
	} else if (mode == MODE_SCOALESCE) {
		return 0;//do_scoalesce(fd, &ifr);
	} else if (mode == MODE_GSN) {
		return do_gsn(fd,bin_file);
	} else if (mode == MODE_SRING) {
		return 0;//do_sring(fd, &ifr);
	} else if (mode == MODE_GOFFLOAD) {
		return 0;//do_goffload(fd, &ifr);
	} else if (mode == MODE_SOFFLOAD) {
		return 0;//do_soffload(fd, &ifr);
	} else if (mode == MODE_GSTATS) {
		return do_gstats(fd);
	} else if	(mode == MODE_PHY)	{
		return	do_phyreg(fd);
	} else if	(mode == MODE_PCS)	{
		return	do_pcsreg(fd);
	} else if	(mode == MODE_MAC)	{
		return	do_macreg(fd);
	} else if	(mode == MODE_RXBW){
		return	do_rxbw(fd);
	} else if	(mode == MODE_BYPASS){
		return	do_bypass(fd);
	}
	return 1;
}
static int do_gset(int fd)
{
	int nacfd = fd;
	int i;
	uint16_t result16 = 0;
	//uint32_t result32 = 0;
	int link_status[4] = {0};	
	printf("*****************************************************************************************************\n");
	printf("*								    				    *\n");
	printf("* 					PORT STATUS DIAGNOSE			                    *\n");
	printf("*								    				    *\n");
	printf("*****************************************************************************************************\n");
	printf("			Port1			Port2			Port3			Port4\n");

	printf("\n\n");
	printf("Autoneg Enable Status	");
	for(i=0;i<4;i++)
	{
#ifndef COPPER
		result16 = nac_pcs_read(nacfd, i, 0);
		if (result16 == 0xffff){
			printf("	*	");
		} else if (result16 & 0x1000) {
			printf("On	");
		} else{
			printf("Off	");
		}
#else
	if (get_phy_autonegEN(nacfd,i))
		printf("On	");
	 else
		printf("Off	");
#endif
		printf("  		");
		
	}
	printf("\n\n");

	printf("Autoneg Complete Status	");
	for(i=0;i<4;i++)
	{
#ifndef COPPER
		result16 = nac_pcs_read(nacfd, i, 1);
		if (result16 == 0xffff){
			printf("*	");
		} else if (result16 & 0x0020) {
			printf("Completed");
		} else{
			printf("Not completed");
		}
#else
		if (get_phy_autonegCMPL(nacfd,i))
			printf("Completed");
		else
			printf("Not completed");
#endif
		printf("  		");
		
	}
	printf("\n\n");
	printf("Link_Status		");
	for(i=0;i < 4;i++)
	{
#ifdef COPPER
		result16 = nac_pcs_read(nacfd, i, 5);
#else
		result16 = nac_pcs_read(nacfd, i, 1);
#endif
		if (result16 == 0xffff)
		{
			printf("*	");
			link_status[i] = 0;
		}
#ifdef COPPER
		else if (result16 & 0x8000)
		{
#else
		else if (result16 & 0x4)
		{
#endif
			printf("Up	");
			link_status[i] = 1;
		}
		else
		{
			printf("Down	");
			link_status[i] = 0;
		}

		printf("		");
		
	}

	printf("\n\n");
#ifdef COPPER  //COPPER card port status disorder fixed ,but fiber card unkown now 
	printf("Speed			");
	for(i=0;i < 4;i++)
	{
		result16 = nac_pcs_read(nacfd, i, 5);
		if (result16 == 0xffff)
		{	printf("*	");
		}
		else
		{ 
			result16 = (result16 & 0x0C00) >> 10;
			if (link_status[i] == 0) { // link down
				printf("*	");
			}
			else if (result16 == 0) {	 
				printf("10Mbps	");
			}else if (result16 == 1){
				printf("100Mbps	");
			}
			else if (result16 == 2){
				printf("1000Mbps");
			}else
				printf("Reserved");
		}
		printf("		");
		
	}
	printf("\n\n");

	printf("Duplex_Mode		");
	for(i=0;i < 4;i++)
	{
		result16 = nac_pcs_read(nacfd, i, 5);
		if (link_status[i] == 0) { // link down
			printf("*	");
		}
		else if (result16 == 0xffff)
		{	printf("*	");
		}
		else if (result16 & 0x1000)
		{
			printf("Full	");
		}else
		{
			printf("Half	");
		}

		printf("		");
		
	}
	printf("\n\n");

	printf("Bypass_Status		");
	result16 = reg_read(nacfd,BYPASS_CRL);
	for(i=0;i<4;i++)
	{
		if (result16 & 0x0001)
		{ 
			printf("Off	");
		}
		else 
		{	printf("On	");
		}
		printf("		");
		
	}
	printf("\n\n");
#else
	printf("Speed			");
	for(i=0;i<4;i++)
	{
		if (link_status[i] == 0) { // link down
			printf("*	");
		}
		else{
			printf("1000Mbps");
		}
		printf("		");
		
	}
	printf("\n\n");

	printf("Duplex_Mode		");
	for(i=0;i<4;i++)
	{
		if (link_status[i] == 0) { // link down
			printf("*	");
		}
		else
		{
			printf("Full	");
		}

		printf("		");
		
	}
	printf("\n\n");
#endif	
	nac_close(nacfd); 
  return 0;
}

static int do_sset(int fd)
{
//	int i = 0;
	int nacfd = fd;
	if(neg_changed)
	{
		/*for (i = 0; i < 4; i++)
		{
			nac_mdio_init( nacfd, i);
		
		}*/
		//fiber card  autoneg haven't complete
		
	/*	if (autoneg_wanted != -1)
		{
#ifdef	DEBUG
			fprintf(stdout,"we are setting MAC auto negotiate\n");
#endif
			nac_set_autoneg_enable(nacfd,port_wanted ,autoneg_wanted);
		}*/
		

#ifdef COPPER
			if (autoneg_wanted != -1)
			{
#ifdef	DEBUG
				fprintf(stdout,"we are setting PHY auto negotiate\n");
#endif
				if (autoneg_wanted == AUTONEG_ENABLE)
					ge_mdio_random_write(nacfd,port_wanted,0,AUTONEG_VALUE);
				else if (autoneg_wanted == AUTONEG_DISABLE)
					ge_mdio_random_write(nacfd,port_wanted,0,SPEED1000_FULL ); //defalut value is speed 1000 duplex FULL
				else
					show_usage(1);
			}

#endif
	}
#ifdef COPPER
	if (mode_changed)
	{
		ge_mdio_random_write(nacfd,port_wanted,0,RESET_PHY );//reset phy
		usleep(10);
		if ((speed_wanted == SPEED_1000) && (duplex_wanted == FULL))
			//ge_mdio_random_write(nacfd,port_wanted,0,SPEED1000_FULL );
			printf("force speed 1000M and duplex full is not supported\n");
		else if ((speed_wanted == SPEED_1000) && (duplex_wanted == HALF))
			printf("force speed 1000M and duplex half is not supported\n");
		else if ((speed_wanted == SPEED_100) && (duplex_wanted == FULL))
			ge_mdio_random_write(nacfd,port_wanted,0,SPEED100_FULL );
		else if ((speed_wanted == SPEED_100) && (duplex_wanted == HALF))
			ge_mdio_random_write(nacfd,port_wanted,0,SPEED100_HALF );
		else if ((speed_wanted == SPEED_10) && (duplex_wanted == FULL))
			ge_mdio_random_write(nacfd,port_wanted,0,SPEED10_FULL );
		else if ((speed_wanted == SPEED_10) && (duplex_wanted == HALF))
			ge_mdio_random_write(nacfd,port_wanted,0,SPEED10_HALF );
		else
			show_usage(1);
	}
#endif
	nac_close(nacfd); 
  return 0;
}
uint64_t read_tx_bytes(int nacfd, uint8_t port)
{
	uint32_t tmp;
	uint64_t result;
	tmp = reg_read(nacfd, PORT0_TX_BYTE_HIGH + port*PORT_OFFSET_60);
	result = (uint64_t)tmp;
	result = result <<32;
	tmp = reg_read(nacfd, PORT0_TX_BYTE_LOW + port*PORT_OFFSET_60);
	result +=(uint64_t)tmp;
	return result;
}

uint64_t read_rx_bytes(int nacfd, uint8_t port)
{
	uint32_t tmp;
	uint64_t result;
	tmp = reg_read(nacfd, PORT0_RX_BYTE_HIGH + port*PORT_OFFSET_60);
	result = (uint64_t)tmp;
	result = result <<32;
	tmp = reg_read(nacfd, PORT0_RX_BYTE_LOW + port*PORT_OFFSET_60);
	result +=(uint64_t)tmp;
	return result;
}
static int do_gstats(int fd)
{
	int nacfd = fd;
	struct timeval start, end;
	int i = 0;
	//int times = 0;
	uint64_t rx_bytes[4] = {0};
	uint64_t tx_bytes[4]= {0};
	uint64_t pre_rx_bytes[4] = {0};
	uint64_t pre_tx_bytes[4] = {0};
	uint64_t timeuse = 0;
	printf("==========================================================================\n");
	printf("               Port1         Port2         Port3         Port4\n");
	printf("==========================================================================\n");
		gettimeofday(&start,NULL);
		for(i = 0; i < 4; i++){
			pre_rx_bytes[i] = read_rx_bytes(nacfd, i);
			pre_tx_bytes[i] = read_tx_bytes(nacfd, i);		
		}

		sleep(SCAN_CYCLE);
		
		gettimeofday(&end,NULL);
		for(i = 0; i < 4; i++){
			rx_bytes[i] = read_rx_bytes(nacfd, i);
			tx_bytes[i] = read_tx_bytes(nacfd, i);		
		}
		
		timeuse = 1000000 * (end.tv_sec - start.tv_sec) + end.tv_usec - start.tv_usec;
	
		// print tx bytes
		printf("Tx Bytes  ");

		for(i=0;i<4;i++)
		{
			printf("    %10llu  ", (long long unsigned int)(tx_bytes[i] - pre_tx_bytes[i]));
		}
		printf("\n");

		// print rx bytes
		printf("Rx Bytes  ");
		for(i=0;i<4;i++)
		{
			printf("    %10llu  ", (long long unsigned int)(rx_bytes[i] - pre_rx_bytes[i]));
		}
		printf("\n");
	
		printf("--------------------------------------------------------------------------\n");
		
		// print tx rate
		printf("Tx Rate   ");

		for(i=0;i<4;i++)
		{
			printf("%10.3fMbps  ", (double)(tx_bytes[i] - pre_tx_bytes[i]) * 8 / (double)(timeuse));
		}
		printf("\n");

		// print rx rate
		printf("Rx Rate   ");
		for(i=0;i<4;i++)
		{
			printf("%10.3fMbps  ", (double)(rx_bytes[i] - pre_rx_bytes[i]) * 8 / (double)(timeuse));
		}
		printf("\n");
		printf("==========================================================================\n");
		close(nacfd);
    return 0;
}
static int do_isp(int fd,char *file)
{
	int nacfd = -1;
	int binfd = -1;
	int i, j;
	struct stat statbuf;
	uint32_t file_size = 0;
	uint32_t file_size_w = 0;
	uint8_t need_bank = 0;
	uint8_t last_bank = 0;
	uint32_t last_words = 0;
	struct timeval tv_pre, tv_now;
	struct timezone tz;
	struct timeval tv_start, tv_end;
	long sec = 0;
	uint64_t sn_logic = 0;
	/*uint64_t sn_hw = 0;
	uint64_t sn1 = 0;
	uint16_t temp = 0;*/
	nacfd = fd;
	gettimeofday(&tv_start, &tz);	
	printf("********************************************\n");
	printf("*                                          *\n");
	printf("*    in-system-program tool for nac card   *\n");
	printf("*                version 1.3               *\n");
	printf("*             date: 11/02/2009             *\n");
	printf("*                                          *\n");
	printf("********************************************\n");
	/* bin file process */
	if ((binfd = open(file, O_RDONLY)) < 0)
	{
		printf("[%s]: open %s failed!\n", __FUNCTION__, file);
		return -1;
	}
	
	if(fstat(binfd, &statbuf) < 0)
   	{
        	perror("fstat source");
        	exit(EXIT_FAILURE);
    }
	
	file_size = (uint32_t)statbuf.st_size;
	if( (file_size % 2) != 0 )
	{
		printf("ERROR! The file %s size is even number\n", file);	
		return -1;
	}
		
//	printf("The file %s is %u bytes\n",file, file_size);
	file_size_w = (file_size -8) / 2;

	if ( (file_size_w % BANK_SIZE_W) == 0 )
	{
		need_bank = file_size_w / BANK_SIZE_W;
		last_words = BANK_SIZE_W;
	}
	else
	{
		need_bank = file_size_w / BANK_SIZE_W + 1;
		last_words = file_size_w % BANK_SIZE_W;
	}
	last_bank = need_bank - 1;
//	printf("The file need %d bank\n", need_bank);
//	printf("The last bank is %d\n", last_bank);
//	printf("The last bank has words %d\n", last_words);

	void *sm = mmap(0, (size_t)file_size, PROT_READ,
               MAP_PRIVATE  |  MAP_NORESERVE, binfd, 0);
    	if( sm == MAP_FAILED)
    	{
      		 perror("mmap source");
       	 exit(EXIT_FAILURE);
   	}

	uint16_t *pw = NULL;
	pw = (uint16_t *)sm;
	
	/* set configuration register command */
	set_config_register(nacfd, 0xBDDF);
	
	/* clear status register */
	clear_stats_regs(nacfd);
	
	/* unlock */
	printf("-> step1: unlock %d bank...\n", need_bank);
	for(i = 0; i < need_bank; i++) 
		bank_unlock(nacfd, i);
	printf("unlock complete!\n");

	/* erase */
	printf("********************************************\n");
	printf("-> step2: earse %d bank...\n", need_bank);
	gettimeofday(&tv_pre, &tz);
	for(i = 0; i < need_bank; i++)
		bank_erase(nacfd, i);
	gettimeofday(&tv_now, &tz);
	sec = (tv_now.tv_sec - tv_pre.tv_sec);
	printf("erase complete, time used %ld s\n", sec);
	
	/* program */
	printf("********************************************\n");
	printf("-> step3: program ...\n");
	gettimeofday(&tv_pre, &tz);
	for(i = 0; i < need_bank -1; i++)
	{
		bank_program(nacfd, i, BANK_SIZE_W, pw);
		pw += BANK_SIZE_W;
	}
	bank_program(nacfd, last_bank, last_words, pw);
	gettimeofday(&tv_now, &tz);
	sec = (tv_now.tv_sec - tv_pre.tv_sec);
	printf("program complete, time used %ld s\n", sec);

	pw = (uint16_t *)sm + file_size_w;
	sn_logic = *(uint64_t *)pw;
	set_logic_num(nacfd, sn_logic);
	
	sn_logic = read_logic_num(nacfd);
	printf("logic serial number is %016llx\n", (long long unsigned int)sn_logic);
	close(binfd); // not use pw any more


	/* verify */
	printf("********************************************\n");
	printf("-> step4: verify ...\n");
	FILE* fp;
	fp = fopen("read_flash", "wb+");	
	uint16_t result16;
	/* read */
	for(j = 0; j < need_bank - 1; j++) // read 3 bank
	{
		flash_write(nacfd, j*BANK_SIZE_W, 0xff);
		for (i = j*BANK_SIZE_W; i < j*BANK_SIZE_W + BANK_SIZE_W; i++)
		{
			result16 = flash_read(nacfd, i);
			fwrite(&result16, 1, 2, fp);
		}
		printf("....");
		fflush(stdout);
	}
	flash_write(nacfd, last_bank*BANK_SIZE_W, 0xff);
	for(i = last_bank*BANK_SIZE_W; i < j*BANK_SIZE_W + last_words; i++)
	{
		result16 = flash_read(nacfd, i);
		fwrite(&result16, 1, 2, fp);
	}
	sn_logic = read_logic_num(nacfd);
	fwrite(&sn_logic, 1, 8, fp);	
	fclose(fp);
	printf("................end\n");

	if ( compare_file_order(file, "read_flash") == 0 )
		printf("Verify successful!\n");
	else
	{
		printf("Verify Error, please reconfig!\n");
		goto END;
	}


	/* reconfigure */
	printf("********************************************\n");
	printf("-> step5: reconfigure ...\n");
	
	/* return to sync read mode */
	set_config_register(nacfd, 0x3DDF);
	/*reconfigure*/
	reconfig(nacfd);
	
	printf("Reconfigure successful, please reboot!\n");
	
	/* calc the total time used */
	gettimeofday(&tv_end, &tz);	
	sec = tv_end.tv_sec - tv_start.tv_sec;	
	printf("********************************************\n");
	printf("*                                          *\n");
	printf("*        Download successful!              *\n");
	printf("*       Total time used %ld s!              *\n",sec);
	printf("*                                          *\n");
	printf("****************** end *********************\n");


END:
	close(nacfd);
  	return 0;
}
static int do_gsn(int fd,char *file)
{
	int binfd = -1;
	int nacfd = -1;
	uint64_t sn = -1;
	uint64_t sn1 = -1;
	/* bin file process */
	nacfd = fd;
	if ((binfd = open(file, O_WRONLY | O_APPEND)) < 0){
		printf("open %s failed!\n", file);
		return -1;
	}
	do{
		printf("please input the hardware serial number:\n");
		printf("0x");
		fflush(stdout);
		scanf("%llx", (long long unsigned int*)&sn);
		printf("please input it again:\n");
		printf("0x");
		fflush(stdout);
		scanf("%llx", (long long unsigned int*)&sn1);
		if (sn != sn1)
			printf("sn is error, please re-input\n");
	}
	while(sn != sn1);

	if (write(binfd, &sn, 8) != 8)
	{
		printf("write error\n");
		return -1;
	}	
	close(binfd);
	nac_close(nacfd);
	return 0;	
}
static int do_dsn(int fd)
{
	int nacfd = -1;
	uint64_t sn_logic = 0;
	uint64_t sn_hw = 0;
	//uint64_t logo = 0;
	//char select = '\0';
	//uint64_t sn1 = 0;
	nacfd = fd;
	/* hardware number */
	printf("*******************************************\n");
	sn_hw = read_hardware_num(nacfd);
	printf("Hardware number is %llx\n", (long long unsigned int)sn_hw);

	/* logic number */	
	printf("*******************************************\n");
	sn_logic = read_logic_num(nacfd);
	printf("Logic serial number is %016llx\n", (long long unsigned int)sn_logic);	
	nac_close(nacfd);
	return 0;
}
static int do_testreg(int fd)
{
	int nacfd = -1;
	unsigned int menu;
	unsigned int addr;
	unsigned int value;
	nacfd = fd;
	while(1){
		fprintf(stdout,"*******************FPGA register*******************\n"
									 "*                 1: write                        *\n"
									 "*                 2: read                         *\n"
									 "*                 3: exit                         *\n"
									 "***************************************************\n");
		scanf("%d",&menu);	
		switch (menu)
		{ 
			case 1: 
			    printf("Input reg address:\n");
			    printf("0x");
			    fflush(stdout);
			    scanf("%x",&addr);
			    printf("input value:\n");
			    printf("0x");
			    fflush(stdout);
			    scanf("%x",&value);
			    reg_write(nacfd,addr,value);
			    value = reg_read(nacfd,addr);
			    printf("reg 0x%x value is 0x%x\n",addr,value);
			    break;
		  case 2:
			    printf("Input reg address:\n");
			    printf("0x");
			    fflush(stdout);
			    scanf("%x",&addr);
	 		    value = reg_read(nacfd,addr);
			    printf("reg 0x%x value is 0x%x\n",addr,value);
			    break;
	    case 3: goto END;
	    default : break;
	   }
 	menu=0;
    }
END:
  close(nacfd);
  return 0;
}
static int do_phyreg(int fd)
{
	int nacfd = -1;
	unsigned int menu;
	unsigned int addr;
	unsigned int value;
	unsigned int port;
	nacfd = fd;
	while(1){
	fprintf(stdout,"*******************PHY register*******************\n"
								 "*                 1: write                       *\n"
								 "*                 2: read                        *\n"
								 "*                 3: exit                        *\n"
								 "**************************************************\n");
		scanf("%d",&menu);	
		switch (menu)
		{ 
			case 1: 
			    printf("Input reg address:\n");
			    printf("0x");
			    fflush(stdout);
			    scanf("%x",&addr);
			    printf("input value:\n");
			    printf("0x");
			    fflush(stdout);
			    scanf("%x",&value);
			    printf("input port:\n");
			    printf("0x");
			    fflush(stdout);
			    scanf("%x",&port);
			    ge_mdio_random_write(nacfd,port,addr,value);
			    value = ge_mdio_random_read(nacfd,port,addr);
			    printf("port %x reg 0x%x value is 0x%x\n",port,addr,value);
			    break;
		  case 2:
			    printf("Input reg address:\n");
			    printf("0x");
			    fflush(stdout);
			    scanf("%x",&addr);
			    printf("input port:\n");
			    printf("0x");
			    fflush(stdout);
			    scanf("%x",&port);
	 		    value = ge_mdio_random_read(nacfd,port,addr);
			    printf("port %x reg 0x%x value is 0x%x\n",port,addr,value);
			    break;
	    case 3: goto END;
	    default : break;
	   }
 			menu=0;
    }
END:
  close(nacfd);
  return 0;
}
static int do_pcsreg(int fd)
{
	int nacfd = -1;
	unsigned int menu;
	unsigned int addr;
	unsigned int value;
	unsigned int port;
	nacfd = fd;
	while(1){
		fprintf(stdout,"*******************PCS register*******************\n"
									 "*                 1: write                        *\n"
									 "*                 2: read                         *\n"
									 "*                 3: exit                         *\n"
									 "***************************************************\n");
		scanf("%d",&menu);
		switch (menu)
		{ 
			case 1: 
			    printf("Input reg address:\n");
			    printf("0x");
			    fflush(stdout);
			    scanf("%x",&addr);
			    printf("input value:\n");
			    printf("0x");
			    fflush(stdout);
			    scanf("%x",&value);
			    printf("input port:\n");
			    printf("0x");
			    fflush(stdout);
			    scanf("%x",&port);
			    nac_pcs_write(nacfd,port,addr,value);
			    value = nac_pcs_read(nacfd,port,addr);
			    printf("port %x reg 0x%x value is 0x%x\n",port,addr,value);
			    break;
		  case 2:
			    printf("Input reg address:\n");
			    printf("0x");
			    fflush(stdout);
			    scanf("%x",&addr);
			    printf("input port:\n");
			    printf("0x");
			    fflush(stdout);
			    scanf("%x",&port);
	 		    value = nac_pcs_read(nacfd,port,addr);
			    printf("port %x reg 0x%x value is 0x%x\n",port,addr,value);
			    break;
	    case 3: goto END;
	    default : break;
	   }
 			menu=0;
    }
END:
  close(nacfd);
  return 0;
}
static int do_macreg(int fd)
{
	int nacfd = -1;
	unsigned int menu;
	unsigned int addr;
	unsigned int value;
	unsigned int port;
	nacfd = fd;
	while(1){
	fprintf(stdout,"*******************MAC register*******************\n"
								 "*                 1: write                       *\n"
								 "*                 2: read                        *\n"
								 "*                 3: exit                        *\n"
								 "**************************************************\n");
		scanf("%d",&menu);	
		switch (menu)
		{ 
			case 1: 
			    printf("Input reg address:\n");
			    printf("0x");
			    fflush(stdout);
			    scanf("%x",&addr);
			    printf("input value:\n");
			    printf("0x");
			    fflush(stdout);
			    scanf("%x",&value);
			    printf("input port:\n");
			    printf("0x");
			    fflush(stdout);
			    scanf("%x",&port);
			    nac_mac_write(nacfd,port,addr,value);
			    value = nac_mac_read(nacfd,port,addr);
			    printf("port %x reg 0x%x value is 0x%x\n",port,addr,value);
			    break;
		  case 2:
			    printf("Input reg address:\n");
			    printf("0x");
			    fflush(stdout);
			    scanf("%x",&addr);
			    printf("input port:\n");
			    printf("0x");
			    fflush(stdout);
			    scanf("%x",&port);
	 		    value = nac_mac_read(nacfd,port,addr);
			    printf("port %x reg 0x%x value is 0x%x\n",port,addr,value);
			    break;
	    case 3: goto END;
	    default : break;
	   }
 			menu=0;
    }
END:
  close(nacfd);
  return 0;
}
/**********************************************************/

static int rxbw_Report(int fd) {

	struct timeval start, end;
	uint64_t 	rcd_s[NUM_STREAM];
	uint64_t 	rcd_e[NUM_STREAM];
	uint32_t    size;
	uint32_t	buf_size;
	uint64_t	timeuse=0;
	int i;
	int nacfd = -1;
	nacinf_t inf;
	nacfd = fd;
	if(ioctl(nacfd, NACIOCINFO, &inf) < 0)
	{
		/* ioctl() sets errno. */
		perror("ioctl");
		return -1;
	}
	buf_size = inf.buf_size;
#ifdef DEBUG
	printf("buf_size:%uMB\n",buf_size>>20);
#endif
	size = (buf_size- RFT_SIZE)/NUM_STREAM;
	while(1) {
		printf("************************************************\n");
		gettimeofday(&start,NULL);
		for (i = 0;i < NUM_STREAM;i++)
		{
			rcd_s[i] = reg_read(nacfd,RECORD_PTR_LO_REG + i * STREAM_REG_DIFF_0x40);
#ifdef DEBUG
			printf("rcd_s[%d] %x\n",i,rcd_s[i]);
#endif
		}
		usleep(USCAN_CYCLE);	
		gettimeofday(&end,NULL);
		for (i = 0;i < NUM_STREAM;i++)
		{
			rcd_e[i] = reg_read(nacfd,RECORD_PTR_LO_REG + i * STREAM_REG_DIFF_0x40);
#ifdef DEBUG
			printf("rcd_e[%d] %x\n",i,rcd_e[i]);
#endif
		}
		timeuse = 1000000 * (end.tv_sec - start.tv_sec) + end.tv_usec - start.tv_usec;
#ifdef DEBUG
		printf("timeuse: %lu\n",timeuse);
#endif
		for (i = 0;i < NUM_STREAM;i++)
		{
			if (rcd_e[i] < rcd_s[i])
			{
				rcd_e[i] += size;
			}
		}
		for (i=0;i<NUM_STREAM;i++)
		{
			printf("Stream %d BW: %f Mbps\n",i,(double)(8*(rcd_e[i] - rcd_s[i]))/(double)timeuse);
		}
		printf("************************************************\n");	
		sleep(2);
		}
	return 0;
}
/**********************************************************/
static int do_rxbw(int fd)
{
	int nacfd = -1;
	nacfd = fd;
	//nactool_config(nacfd);
	return rxbw_Report(nacfd);
	
}

static int do_bypass(int fd)
{
	int nacfd = -1;
	nacfd = fd;
	if (bypass_wanted == BYPASS_ENABLE)
	{
			bypass_switch_on(nacfd);
	}
	else if	(bypass_wanted == BYPASS_DISABLE)
	{
			bypass_switch_off(nacfd);
	}
	return 0;
}

static int get_phy_autonegEN(int fd,int port)
{
	uint16_t value = 0;
	value = ge_mdio_random_read(fd,port,0);
	if (value & 0x1000 )
		return 1;
	else
		return 0;
}
static int get_phy_autonegCMPL(int fd,int port)
{
	uint16_t value = 0;
	value = ge_mdio_random_read(fd,port,1);
	if (value & 0x0020 )
		return 1;
	else
		return 0;
}


int main(int argc, char **argp)
{
	parse_cmdline(argc, argp);
	return doit();
}
