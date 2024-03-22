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
#include <timer.h>
#include <sockets.h>

/* Pull cookies out of the network. */

#define	COOKIEJAR	0x2c00000a
#define	COOKIETIME	6

#define WAITING		0
#define	TIMEOUT		1
#define	GOTONE		2

int gimme(), ohboy();
task *cookietask;
unsigned well = WAITING;

main(argc, argv)
	int argc;
	char *argv[]; {
	in_name fhost;
	timer *tm;
	UDPCONN udp;
	PACKET p;

	if(argc > 2) {
		printf("cookie: usage:  cookie [host]\n");
		exit(1);
		}

	/* Initialize the network */
	Netinit(800);
	in_init();
	IcmpInit();
	GgpInit();
	UdpInit();
	nm_init();

	if(argc == 2) {
		fhost = resolve_name(argv[1]);
		if(fhost == 0) {
		   printf("%s isn't listed in any of my friends'", argv[1]);
		   printf(" phone books; perhaps you got a wrong number.\n");
			exit(1);
			}

		if(fhost == NAMETMO) {
			printf("No one will tell me who %s is!", argv[1]);
			exit(1);
			}
	}
	else fhost = COOKIEJAR;

	tm = tm_alloc();
	if(tm == 0) {
		printf("The hour is unkown, and I have better things that I ");
		printf("can do [meaning that this isn't one of them].\n");
		exit(1);
		}

	udp = udp_open(fhost, UDP_COOKIE, udp_socket(), gimme, 0);
	if(udp == 0) {
		printf("Sorry, I just can't make the connection.\n");
		exit(1);
		}

	p = udp_alloc(0, 0);
	if(p == NULL) {
		printf("Boss! The packet, the packet!!\n");
		exit(1);
		}

	if(udp_write(udp, p, 0) <= 0) {
		printf("Not only can't I give you cookies - I can't even");
		printf(" reach the jar!\n");
		exit(1);
		}

	cookietask = tk_cur;

	tm_set(COOKIETIME, ohboy, 0, tm);
	while(well == WAITING) tk_block();

	if(well == TIMEOUT) {
	    printf("One cannot always produce a cookie out of one's hat.\n");
		exit(1);
		}

	}

ohboy() {
	well = TIMEOUT;
	tk_wake(cookietask);
	}

gimme(p, len, host)
	PACKET p;
	unsigned len;
	in_name host; {
	char *cookie;

	if(len == 0) {
		printf("Gee, that was a dietetic cookie!\n");
		well = GOTONE;
		tk_wake(cookietask);
		}

	cookie = udp_data(udp_head(in_head(p)));
	cookie[len] = 0;
	puts(cookie);

	well = GOTONE;
	tk_wake(cookietask);
	}
		
