/*  Copyright 1986 by Carnegie Mellon  */
/*  See permission and disclaimer notice in file "cmu-note.h"  */
#include	<cmu-note.h>
/*  Copyright 1984,1985 by the Massachusetts Institute of Technology  */
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
#include <h19.h>

/* User Ping. */
in_name ping_host;		/* for the command interpreter */

main(argc, argv)
	int argc;
	char *argv[]; {
	in_name host;
	unsigned ntimes = 1;
	char *arg;
	char test=0;
	char serve=0;
	char key;
	long nsuccess=0L;
	long i;
	unsigned res = PGWAITING;
	unsigned length = 256;
	int c;

	if(argc < 2 || argc > 5) {
		printf("Usage:\n\tping host\n");
		printf("\tping -n number_of_times host\n");
		printf("\tping -t host\t(ping forever)\n");
		printf("\tping -s\t\t(ping server)\n");
		exit(1);
		}

	arg = argv[1];
	if(arg[0] == '-') {
		arg++;
		while(*arg != '\0') {
			switch(*arg) {
			case 't':
				test = 1;
				break;
			case 'n':
				ntimes = atoi(argv[2]);
				argv++;
				argc--;
				break;
			case 'l':
				length = atoi(argv[2]);
				argv++;
				argc--;
				break;
			case 's':
				serve = 1;
				break;
			default:
				printf("PING: Illegal option %c.\n", *arg);
				exit(1);
			}
			arg++;	}
		argv++;
		argc--;	}

	Netinit(800);
	in_init();
	UdpInit();
	IcmpInit();
	GgpInit();
	nm_init();

	if(serve) {
		printf("\nYes, O Master.\n\n");
		while(1) {
			tk_yield();
			c = h19key();
			if(c != NONE)
				ping_cmd(c);
			}
		}

	host = resolve_name(argv[1]);
	if(host == 0L) {
		printf("Host %s is unknown.\n", argv[1]);
		exit(1);
		}

	if(host == 1L) {
		printf("Name servers not responding.\n");
		exit(1);
		}

	ping_host = host;

	if(test) {
		printf("Pinging host %s (%A) repeatedly\n", argv[1], host);
		printf("for net statistics, type 'n',\n");
		printf("to exit, type 'q'.\n");
		for(i=1;1;i++) {
			res = IcEchoRequest(host, length);
			c = h19key();

			if(res == PGNOSND)
		 printf("Couldn't send (net address unknown?) on try %U\n",i);
			else if(res == PGTMO)
			 printf("ping: timed out on try %U            \n",i);
			else if(res == PGBADDATA)
			 printf("ping: reply with bad data on try %U  \n",i);
			else if(res == PGSUCCESS) nsuccess++;
			else printf("ping: bad return value (try %U) %u\n",
							i, res);

			fprintf(stderr,"# of tries = %U, successes = %U\r",
								i, nsuccess);

			if(c != NONE)
				ping_cmd(c);
			}
		}

	for(i=0; i<ntimes; i++) {
		c = h19key();
		if(c != NONE)
			ping_cmd(c);

		res = IcEchoRequest(host, length);
		switch(res) {
		case PGSUCCESS:
			nsuccess++;
			printf("Host %A responding\n", host);
			break;
		case PGNOSND:
		printf("Couldn't send (net address unknown?) on try %U\n",
								i);
			break;
		case PGTMO:
			printf("Host %A timed out\n", host);
			break;
		case PGBADDATA:
			printf("ping: reply with bad data\n");
			break;
		default:
			printf("ping: bad return value %u\n", res);
		}
	}

	if(ntimes != 1)
		printf("\nPinged host %A %u times and got %U responses.\n",
						host, ntimes, nsuccess);

	net_stats(stdout);
	}	
