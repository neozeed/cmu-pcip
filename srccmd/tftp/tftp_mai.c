/*  Copyright 1986 by Carnegie Mellon  */
/*  See permission and disclaimer notice in file "cmu-note.h"  */
#include	<cmu-note.h>
/*  Copyright 1984 by the Massachusetts Institute of Technology  */
/*  See permission and disclaimer notice in file "notice.h"  */
#include	<notice.h>

#include <stdio.h>
#include <types.h>
#include <task.h>
#include <q.h>
#include <netq.h>
#include <net.h>
#include <custom.h>
#include <netbuf.h>
#include <icmp.h>
#include <ip.h>
#include <udp.h>
#include <tftp.h>
#include <timer.h>
#include <em.h>

/* This is the new TFTP for the PC's that runs under tasking with the new
	internet.
	It accepts the following forms of arguments:

	tftp get localfile host fornfile [mode] [debug]
	tftp put localfile host fornfile [mode] [debug]
	tftp serve [spool]
	tftp get localfile host fornfile test [debug]
	tftp put filesize  host fornfile test [debug]
*/

/*  Modified 12/23/83 to make server tftp message direction printout test
    compatible with the corresponding test in telnet, and to add an upcall
    for server tftp on file transfer complete.  <J. H. Saltzer>

    1/23/84 - added simple (awful) user interface to tftp server to allow
	quitting and calling netstat. <John Romkey>
    1/24/84 - added octet mode. <John Romkey>
    1/2/85 - removed reference to IMAGE mode value.  <J. H. Saltzer>
    4/26/85 - message at startup mentions mode of transfer. <J. H. Saltzer>
    7/3/85 - added spool option (to suppress write buffering) <J. H. Saltzer>
*/

extern NET nets[];
extern long cticks;

int tfprint(), tfdone();
long tftp_use();

main(argc, argv)
	int argc;
	char *argv[]; {
	in_name fhost;
	long size=0;
	long bps;
	long deltat;
	unsigned dir, mode;
	char *modename;
	char *fname;
	int c;

	if(argc == 2 || argc == 3)
		if(strcmp(argv[1], "serve") == 0) {
			printf("serving\n");
			NBUF = 8;

			Netinit(800);
			in_init();
			IcmpInit();
			GgpInit();
			UdpInit();
			tfsinit(tfprint, tfdone,
				strcmp(argv[2], "spool") == 0);
			tfs_on();
			while(1) {
				while((c = h19key()) == NONE) tk_yield();
				if(c == 'q') exit(0);
				else if(c == '+') {
					printf("tftp server turned on\n");
					tfs_on();
					}
				else if(c == '-') {
					printf("tftp server turned off\n");
					tfs_off();
					}
				else if(c == 's') net_stats(stdout);
				else {
				printf("tftp server commands\n");
				printf("+	turn server on\n");
				printf("-	turn server off\n");
				printf("q	exit server\n");
				printf("s	network statistics\n");
				}
				}
			}
		else {
			printf("TFTP: Invalid argument %s.\n", argv[1]);
			tfusage();
			}

	if(argc < 5) {
		printf("TFTP: Too few arguments.\n");
		tfusage();
		}

	mode = ASCII;
	modename = "NETASCII";
	if(argc > 5)
		if(strcmp(argv[5], "netascii") == 0) {mode = ASCII;
						      modename = "NETASCII";}
		else if(strcmp(argv[5], "image") == 0) {mode = OCTET;
							modename = "OCTET";}
		else if(strcmp(argv[5], "octet") == 0) {mode = OCTET;
							modename = "OCTET";}
		else if(strcmp(argv[5], "test") == 0) {mode = TEST;
						       modename = "TEST";}
		else {
			printf("TFTP: Bad mode: %s\n", argv[5]);
			tfusage();
			}

	if(argc == 7) {
		NDEBUG = atoi(argv[6]);
		printf("NDEBUG = %04x\n", NDEBUG);
		}


	if(strcmp(argv[1], "get") == 0) dir = GET;
	else if(strcmp(argv[1], "put") == 0) dir = PUT;
	else if(strcmp(argv[1], "-g") == 0) dir = GET;
	else if(strcmp(argv[1], "-p") == 0) dir = PUT;
	else {
		printf("TFTP: Bad direction: %s\n", argv[1]);
		tfusage();
		}

	if(dir == PUT && mode == TEST) {
		size = atol(argv[2]);
		if(size == 0) {
			printf("TFTP: Bad test size.\n");
			exit(1);
			}
		}


	/* Initialize the network */
	NBUF = 12;		/* Number of packet buffers */

	Netinit(800);
	in_init();
	IcmpInit();
	GgpInit();
	UdpInit();
	nm_init();
/*	LogInit();	*/

	fhost = resolve_name(argv[3]);
	if(fhost == 0) {
		printf("Couldn't resolve hostname %s.\n", argv[3]);
		exit(1);	}

	if(fhost == NAMETMO) {
		printf("name servers not responding.\n");
		exit(1);
		}

	fname = argv[4];
	if(*fname == '"' && fname[strlen(fname)-1] == '"') {
		fname[strlen(fname)-1] = 0;
		fname++;
		}

	/* start up a user TFTP */
	printf("IBM PC User UDP/TFTP Ver %u.%u", version/10,
						 version%10);
	printf(" - bugs to pc-ip-request@mit-xx\n");
	printf("%s mode TFTP started with %s via %s.\n",
	       			modename, argv[3], nets[0].n_name);
/*	log(tftplog, "User TFTP started with %s via %s.",
						argv[3], nets[0].n_name);
*/

	size = tftpuse(fhost, argv[2], fname, dir, mode, size, &deltat);

	if(size == 0) {	
		printf("Transfer not successful.\n");
/*		log(tftplog, "Transfer not successful.");	*/
		exit(1);	}

	if (deltat == 0) bps = 0;
	   else bps = size*8L*TPS/deltat;
	deltat = deltat/TPS;
	printf("Transfer successful:\n");
	printf("%U bytes in %U seconds, %U bits/second\n", size, deltat, bps);

/*	log(tftplog, "done: %U bytes %U secs %U bits/second",size,deltat,bps);
*/
	}

/* convert a number represented in ascii to a long. */

long atol() {
	return 0;	}

/* yes, "usage" is the right way to spell it. */

tfusage() {

	printf("TFTP usage is:\n");
	printf("tftp serve [debug]\n");
	printf("\tor\n");
	printf("tftp direction local_file foreign_host foreign_file [mode]\n\n");
	printf("where:\n");
	printf("\tdirection is: get, put, -g or -p\n");
	printf("\tmode is: netascii, image, octet, or test - defaults to netascii\n");
	printf("\t   and if mode is test and direction is put, local_file is a number\n");
	printf("\t   specifying how many bytes to put.\n\n\n");
	exit(1);
	}

/* function which prints messages about tftps server transactions
*/

static unsigned tftpnum = 1;

tfprint(host, file, dir)
	in_name host;
	char *file;
	int dir; {

	printf("tftp #%u\n", tftpnum++);
	printf("transferring file %s %s %a\n", file,
					dir == PUT ? "to" : "from" , host);

	return 1;
	}

tfdone(success)
	int success; {
	if (success) printf("Successful transfer.\n");
	else printf("Transfer failed.\n");
	return;
	}
