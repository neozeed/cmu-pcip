/*  Copyright 1986 by Carnegie Mellon  */
/*  See permission and disclaimer notice in file "cmu-note.h"  */
#include	<cmu-note.h>
/*  Copyright 1984,1985 by the Massachusetts Institute of Technology  */
/*  See permission and disclaimer notice in file "notice.h"  */
#include	<notice.h>

/* 10-16-85 Use O_BINARY mode in open() if MSC defined.
					<Drew D. Perkins>
 */

#include	<stdio.h>
#include	<task.h>
#include	<q.h>
#include	<netq.h>
#include	<net.h>
#include	<custom.h>
#include	<colors.h>
#include	<attrib.h>
#include	"menu.h"
#include	<fcntl.h>			/* DDP */

#define	BYTESTODO	sizeof(struct custom)
struct custom	custom;
struct custom   custom2;

unsigned UNDER_ATTRIB = UNDER;

extern	struct menu_ent top[];
extern  int errno;
extern  char *sys_errlist[];

long	get_dosl();

main(argc,argv)
int	argc;
char	*argv[];
{
	int	infd;
	int	tfd;
	char	temp[80];
	int	ret;
	int	count;
	int	myaddr_switch = 0;
	int	defgw_switch = 0;
	int	subnet_switch = 0, subnet_bits;
	long	myaddr, defgw;

	scr_init();		/* initialize the screen package */

	for(; argc >= 3; argc -= 2, argv +=2) {
		if(argv[1][0] == '-') {
			switch (argv[1][1]) {
			case 'a':
				myaddr = resolve_name(argv[2]);
				myaddr_switch = 1;
				break;

			case 'g':
				defgw = resolve_name(argv[2]);
				defgw_switch = 1;
				break;

			case 's':
				if((subnet_bits = atoi(argv[2])) == 0 &&
					argv[2][0] != '0')
					subnet_bits = -1;
				subnet_switch = 1;
				break;

			default:
				printf("Usage: customize [-ags #] filename [sample file]\n");
				quit(1);
			}
		} else break;
	}

	if (argc < 2 || argc > 3) {
		printf("Usage: customize [-ags #] filename [sample file]\n");
		quit(1);
	}

	if(_display_type() == 3)
		UNDER_ATTRIB = LGRAY<<4|RED;

	strcpy(temp, argv[1]);
#ifndef MSC						/* DDP */
	strcat(temp, ".sys");
	infd = open(temp, 2);
	if (infd < 0) {
		printf("Error opening %s: %s\n", temp, sys_errlist[errno]);
		quit(1);
	}
#else							/* DDP */
	infd = pcip_open(temp, O_RDWR|O_BINARY);	/* DDP */
	if (infd < 0) {
		strcat(temp, ".sys");
		infd = pcip_open(temp, O_RDWR|O_BINARY);	/* DDP */
		if (infd < 0) {
			printf("Error opening %s: %s\n", temp,
				sys_errlist[errno]);
			quit(1);
		}
	}
#endif							/* DDP */
	mkraw(infd);
	{ char *p, *strchr(); if((p = strchr(temp, '.')) &&
	(!strcmp(p, ".exe") || !strcmp(p, ".EXE"))) lseek(infd, 512L, 0); }
	read(infd, &custom, BYTESTODO);

	if(custom.c_iver != CVERSION) {
		close(infd);
		printf("\tYou attempted to customize a program that has an\n");
		printf("old version number or one that is not customizeable.\n");
		quit(1);
	}

	if (argc == 3) {
		strcpy(temp, argv[2]);
#ifndef MSC						/* DDP */
		strcat(temp, ".sys");
 		tfd = open(temp, 2);
		if (tfd == -1) {
			printf("Couldn't open %s: %s\n", temp,
							sys_errlist[errno]);
			close(infd);
			quit(1);
		}
#else							/* DDP */
		tfd = pcip_open(temp, O_RDWR|O_BINARY);	/* DDP */
		if (tfd == -1) {
			strcat(temp, ".sys");
			tfd = pcip_open(temp, O_RDWR|O_BINARY);	/* DDP */
			if (tfd == -1) {
				printf("Couldn't open %s: %s\n", temp,
							sys_errlist[errno]);
				close(infd);
				quit(1);
			}
		}
#endif							/* DDP */
		else {
			mkraw(tfd);
			read(tfd, &custom2, BYTESTODO);
			close(tfd);
			if (custom2.c_iver != CVERSION) {
				printf("Error: The template file does not have a current\n");
				printf("version of the custom structure. The copy has been\n");
				printf("aborted\n");
				close(infd);
				quit(1);
			}
			cust_copy(&custom2, &custom);
		}
	}

	if(myaddr_switch || defgw_switch || subnet_switch) {
		if(myaddr_switch)
			if((custom.c_me = myaddr) == -1L) {
				printf("Bad ip address (%a) in -a option\n",
					myaddr);
				printf("Usage: customize [-ags #] filename [sample file]\n");
				close(infd);
				quit(0);
			}
		if(defgw_switch)
			if((custom.c_defgw = defgw) == -1L) {
				printf("Bad ip address (%a) in -g option\n",
					defgw);
				printf("Usage: customize [-ags #] filename [sample file]\n");
				close(infd);
				quit(0);
			}
		if(subnet_switch) {
			if(subnet_bits < 0 || subnet_bits > 32) {
				printf("Bad number of subnet bits (%d) in -s option\n",
					subnet_bits);
				printf("Usage: customize [-ags #] filename [sample file]\n");
				close(infd);
				quit(0);
			} else custom.c_subnet_bits = subnet_bits;
		}
		fixup_subnet_mask();
	} else {
		clear_lines(0, 25);

		ret = menu(top);
		printf("\n");
		if (ret != 42) {
			close(infd);
			quit(0);
		}
	}

	/* Now save the data back. */

#define	DATE	0x2a
#define	TIME	0x2c

	custom.c_ctime = get_dosl(TIME);	/* Set the time and date of */
	custom.c_cdate = get_dosl(DATE);	/* the last modification */

#ifndef MSC						/* DDP */
	strcpy(temp, argv[1]);
	strcat(temp, ".sys");
	infd = open(temp, 1);
#else							/* DDP */
	infd = pcip_open(temp, O_WRONLY|O_BINARY);	/* DDP */
#endif							/* DDP */
	mkraw(infd);
	{ char *p, *strchr(); if((p = strchr(temp, '.')) &&
	(!strcmp(p, ".exe") || !strcmp(p, ".EXE"))) lseek(infd, 512L, 0); }
	write(infd, &custom, BYTESTODO);	/* write block */
	close(infd);
	quit(0);
}

cust_copy(from, to)
struct custom	*from;
struct custom	*to;
{
	int	i;
	
	to->c_baud = from->c_baud;
	to->c_debug = from->c_debug;
	to->c_1custom = from->c_1custom;
	to->c_me = from->c_me;
	to->c_log = from->c_log;
	to->c_defgw = from->c_defgw;
	to->c_cookie = from->c_cookie;
	to->c_printer = from->c_printer;
	to->c_scribe = from->c_scribe;
	to->c_subnet_bits = from->c_subnet_bits;
	to->c_net_mask = from->c_net_mask;
	to->c_route = from->c_route;

	to->c_numtime = from->c_numtime;
	for (i = 0; i < MAXTIMES; i++)
		to->c_time[i] = from->c_time[i];
	to->c_numname = from->c_numname;
	for (i = 0; i < 2; i++)
		to->c_names[i] = from->c_names[i];

	to->c_dm_numname = from->c_dm_numname;
	for(i = 0; i < 3; i++)
		to->c_dm_servers[i] = from->c_dm_servers[i];

	strcpy(to->c_user, from->c_user);
	strcpy(to->c_domain, from->c_domain);

	to->c_tmoffset = from->c_tmoffset;
	strcpy(to->c_tmlabel, from->c_tmlabel);

	to->c_route = from->c_route;

	to->c_seletaddr = from->c_seletaddr;
	for (i = 0; i < sizeof(struct etha); i++)
		((char *)(&to->c_myetaddr))[i] =
			((char *)(&from->c_myetaddr))[i];

	for (i = 0; i < (3 * (sizeof(struct etha) + sizeof(in_name))); i++)
		((char *)(to->c_ether))[i] = ((char *)(from->c_ether))[i];

	for(i=0; i < 3; i++)
		to->c_ipname[i] = from->c_ipname[i];

	to->c_ip_radix = from->c_ip_radix;
	to->c_chirpf = from->c_chirpf;		/* DDP */
	to->c_chirpd = from->c_chirpd;		/* DDP */
	to->c_chirps = from->c_chirps;		/* DDP */
	to->c_chirpl = from->c_chirpl;		/* DDP */
	to->c_intvec = from->c_intvec;
	to->c_rcv_dma = from->c_rcv_dma;
	to->c_tx_dma = from->c_tx_dma;
	to->c_base = from->c_base;
	to->c_rvd_base = from->c_rvd_base;
	to->c_basemem = from->c_basemem;
}

quit(status)
	int status; {

	scr_close();
	_curse();
	exit(status);
	}
