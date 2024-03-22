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

/*
 10/30/86 Added upcalling support on destination unreachable.
					<Drew D. Perkins>
 */

/* Initialize the UDP layer; get an internet connection, initialize the
	demux table */

IPCONN udp;
int udpdemux();
int udpdudemux();			/* DDP */

UdpInit() {
	if((udp = in_open(UDPPROT, udpdemux, udpdudemux)) == 0) {
#ifdef	DEBUG
		if(NDEBUG & TPTRACE)
			printf("UDP: Couldn't open InterNet connection.\n");
#endif
		}
#ifdef	DEBUG
	else if(NDEBUG & TPTRACE)
		printf("UDP: Opened InterNet connection.\n");
#endif
	}
