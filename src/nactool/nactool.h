#ifndef	NAC_TOOL_H
#define	NAC_TOOL_H
/*#ifndef DEBUG
#define DEBUG
#endif*/
//include standard lib
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/time.h>

//include API
#include "nacapi.h"

//software version
#define VERSION	"0.0.2"
//scan cycle
#define USCAN_CYCLE	100000	//0.1s
#define SCAN_CYCLE	2	//2s
//autoneg macro
#define AUTONEG_DISABLE		0x00
#define AUTONEG_ENABLE		0x01
#define AUTONEG_VALUE		0x1300
#define SPEED1000_FULL 		0x0140
#define SPEED100_FULL 		0x2100
#define SPEED100_HALF 		0x2000
#define SPEED10_FULL 		0x0100
#define SPEED10_HALF 		0x0000
#define RESET_PHY			0x8000

#define FULL 0
#define HALF 1
#define SPEED_10 0
#define SPEED_100 1
#define SPEED_1000 2

#define BYPASS_ENABLE	0
#define BYPASS_DISABLE 1
//device name length such as "/dev/nac0"
#define	DEV_LENGTH				15
//type of command mode
static enum {
	MODE_HELP = -1,
	MODE_GSET=0,
	MODE_SSET,
	MODE_GDRV,
	MODE_GREGS,
	MODE_NWAY_RST,
	MODE_GEEPROM,
	MODE_SEEPROM,
	MODE_TEST,
	MODE_ISP,
	MODE_GPAUSE,
	MODE_SPAUSE,
	MODE_GCOALESCE,
	MODE_SCOALESCE,
	MODE_GSN,
	MODE_SRING,
	MODE_GOFFLOAD,
	MODE_SOFFLOAD,
	MODE_GSTATS,
	MODE_DSN,
	MODE_TR,
	MODE_PHY,
	MODE_PCS,
	MODE_MAC,
	MODE_RXBW,
	MODE_BYPASS,
} mode = MODE_GSET;

static struct option {
    char *srt, *lng;
    int Mode;
    char *help;
    char *opthelp;
} args[] = {   
    	{ 
    	"-s", "--change", MODE_SSET, "Change generic options(auto negotiation ,speed and duplex)",
#ifdef COPPER
		"		[ port 1|2|3|4 ] [ speed 10|100|1000 ] [ duplex half|full ]\n"
#endif
	//	"		[ advertise %%x ]\n"
	//	"		[ phyad %%d ]\n"
	//	"		[ xcvr internal|external ]\n"
	//	"		[ wol p|u|m|b|a|g|s|d... ]\n"
	//	"		[ sopass %%x:%%x:%%x:%%x:%%x:%%x ]\n"
	//	"		[ msglvl %%d ] \n" 
		"		[ port 1|2|3|4 ] [ autoneg on|off ]\n"

		},


#ifdef	COPPER 
				{ 
				"-b", "--bypass", MODE_BYPASS, "bypass switch on or off",
				"		[ bypass on|off ]\n"
				},
#endif

		{ "-g", "--generate", MODE_GSN, "generate bit stream file with serial number for isp(.bin to .bin.sn)", 
								"               [ config file(*.bin) ]\n"},
		{ "-p", "--program", MODE_ISP, "in system program",
                "               [ config file(*.bin.sn) ]\n" },
    {	"-n",	"--serial--number",MODE_DSN,"show and set device serial number"},
    {	"-T","--test--register",MODE_TR,"write or read FPGA registers with a given address"},	//nactool -T avoid to be same as ethtool -t
    { "-c", "--pcs", MODE_PCS, "write or read PCS registers with a given address" },
#ifdef COPPER
    { "-y", "--phy", MODE_PHY, "write or read PHY registers with a given address" },
#endif
    { "-m", "--mac", MODE_MAC, "write or read MAC registers with a given address" },
	{ "-B", "--band", MODE_RXBW, "evaluate receiced band width "},
    /*{ "-a", "--show-pause", MODE_GPAUSE, "Show pause options" },
    { "-A", "--pause", MODE_SPAUSE, "Set pause options",
      "		[ autoneg on|off ]\n"
      "		[ rx on|off ]\n"
      "		[ tx on|off ]\n" },
    { "-c", "--show-coalesce", MODE_GCOALESCE, "Show coalesce options" },
    { "-C", "--coalesce", MODE_SCOALESCE, "Set coalesce options",
		"		[adaptive-rx on|off]\n"
		"		[adaptive-tx on|off]\n"
		"		[rx-usecs N]\n"
		"		[rx-frames N]\n"
		"		[rx-usecs-irq N]\n"
		"		[rx-frames-irq N]\n"
		"		[tx-usecs N]\n"
		"		[tx-frames N]\n"
		"		[tx-usecs-irq N]\n"
		"		[tx-frames-irq N]\n"
		"		[stats-block-usecs N]\n"
		"		[pkt-rate-low N]\n"
		"		[rx-usecs-low N]\n"
		"		[rx-frames-low N]\n"
		"		[tx-usecs-low N]\n"
		"		[tx-frames-low N]\n"
		"		[pkt-rate-high N]\n"
		"		[rx-usecs-high N]\n"
		"		[rx-frames-high N]\n"
		"		[tx-usecs-high N]\n"
		"		[tx-frames-high N]\n"
	        "		[sample-interval N]\n" },
    { "-G", "--set-ring", MODE_SRING, "Set RX/TX ring parameters",
		"		[ rx N ]\n"
		"		[ rx-mini N ]\n"
		"		[ rx-jumbo N ]\n"
	        "		[ tx N ]\n" },
    { "-k", "--show-offload", MODE_GOFFLOAD, "Get protocol offload information" },
    { "-K", "--offload", MODE_SOFFLOAD, "Set protocol offload",
		"		[ rx on|off ]\n"
		"		[ tx on|off ]\n"
		"		[ sg on|off ]\n"
	        "		[ tso on|off ]\n"
	        "		[ ufo on|off ]\n"
	        "		[ gso on|off ]\n" },
    { "-i", "--driver", MODE_GDRV, "Show driver information" },
    { "-d", "--register-dump", MODE_GREGS, "Do a register dump",
		"		[ raw on|off ]\n"
		"		[ file FILENAME ]\n" },
    { "-e", "--eeprom-dump", MODE_GEEPROM, "Do a EEPROM dump",
		"		[ raw on|off ]\n"
		"		[ offset N ]\n"
		"		[ length N ]\n" },
    { "-E", "--change-eeprom", MODE_SEEPROM, "Change bytes in device EEPROM",
		"		[ magic N ]\n"
		"		[ offset N ]\n"
		"		[ value N ]\n" },
    { "-r", "--negotiate", MODE_NWAY_RST, "Restart N-WAY negotation" },
    { "-t", "--test", MODE_TEST, "Execute adapter self test",
                "               [ online | offline ]\n" },*/
    { "-S", "--statistics", MODE_GSTATS, "Show adapter statistics" },
    { "-h", "--help", MODE_HELP, "Show this help" },
    {}
};

static char *devname = NULL;
int gset_changed = 0;
int autoneg_wanted = -1;
int bypass_wanted = -1;
char * sn_file = NULL;
char * bin_file = NULL;
//use dor do_rxbw
unsigned char report_stop;
//char shutdownInProgress = 0;
pthread_t reportThread;
//static uint64_t totalBytes[NUM_STREAM];
int speed_wanted = -1;
int port_wanted = -1;
int duplex_wanted = -1;
int neg_changed = 0;
int mode_changed = 0;

/*******************function************************************/
//show user help
static void show_usage(int badarg);
//identity input arguments
static void parse_cmdline(int argc, char **argp);
//execute command
static int doit(void);
//get nac board sgmii state
static int do_gset(int fd);
//set nac board sgmii state
static int do_sset(int fd);
//get nac board statistics
static int do_gstats(int fd);
//get transmited bytes
uint64_t read_tx_bytes(int nacfd, uint8_t port);
//get received bytes
uint64_t read_rx_bytes(int nacfd, uint8_t port);
//in system program function
static int do_isp(int fd,char *file);
//generate serial number from *.bin file to *.bin.sn file
static int do_gsn(int fd,char *file);
//show and set device serial number
static int do_dsn(int fd);
//write or read FPGA registers with a given address
static int do_testreg(int fd);
//write or read PHY registers with a given address
static int do_phyreg(int fd);
//write or read PCS registers with a given address
static int do_pcsreg(int fd);
//write or read MAC registers with a given address
static int do_macreg(int fd);
//get receiced band width by recorder pointer
static int do_rxbw(int fd);
//use for do_rxbw configure board;
//static void nactool_config(int fd);
static int rxbw_Report(int fd);
static int do_bypass(int fd);
//get PHY status
static int get_phy_autonegEN(int fd,int port);
static int get_phy_autonegCMPL(int fd,int port);

/*******************function************************************/

#endif
