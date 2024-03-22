/*  Copyright 1986 by Carnegie Mellon  */
/*  See permission and disclaimer notice in file "cmu-note.h"  */
#include	<cmu-note.h>
/*  Copyright 1984 by the Massachusetts Institute of Technology  */
/*  See permission and disclaimer notice in file "notice.h"  */
#include	<notice.h>

/* 10/3/84 - removed Init1(); changed to handle the new net structure
						<John Romkey>
   8/9/85 - rewrote to work with new NET structure.
						<John Romkey>
   3/24/86 - (re)added et_swtch routine.  Why did John remove it?
	Decreased demux task stack size back to 1400 like last version.
	Why was it raised to 5400?  There shouldn't be a problem.  Seemed
	like paranoia.
						<Drew D. Perkins>
   8/5/86 - Add new elements of net structure for bootp support.
						<Drew D. Perkins>
*/

#include <ether.h>
#include <task.h>
#include <q.h>
#include <netq.h>
#include <net.h>
#include <custom.h>
#include <netbuf.h>

#define NULL	0

/* This is the network configuration file for the new IP code. This file
	sets up the configuration for a single ethernet interface
	machine. Programs which want to use multiple net interfaces
	should use their own configuration file.
*/

int Nnet = 1;		/* The number of networks. */
char _net_if_name = '3';	/* DDP - Default NETCUST = "NETCUST3" */

/* random fnctns. */
int et_init(), et_send(), et_switch(), et_stat(), et_close(), ip_ether_send();
extern char _etme[6];		/* DDP - my ethernet address */

/* DDP - Plummer's internals. All constants are already byte-swapped. */
#define	ARETH	0x1		/* DDP - ethernet hardware type */

NET nets[1] = { "3COM Ethernet",	/* interface name */
		et_init,		/* initialization routine */
		et_send,		/* raw packet send routine */
		et_switch,		/* DDP interrupt vector swap routine */
		et_close,		/* shutdown routine */
/*		ip_ether_send,		/* ip packet send routine */
		NULL,
		et_stat,		/* statistics routine */
		NULL,			/* demultiplexing task */
		NULL,			/* packet queue */
					/* first parameter...*/
#ifdef	WATCH
		ALLPACK,		/* ...promiscuous mode for netwatch */
#endif
#ifndef	WATCH
		0,			/* ...normal mode for others */
#endif
		0,			/* second parameter unused */
		1400,			/* DDP demux task stack size */
		14,			/* local net header size */
		0,			/* local net trailer size */
		0L,			/* ip address */
		0L,			/* default gateway */
		0L,			/* network broadcast address */
		0L,			/* 4.2bsd network broadcast address */
		0L,			/* subnetwork broadcast address */
		&custom,		/* our custom structure! */
		6,			/* hardware address length */
		ARETH,			/* hardware type */
		_etme,			/* pointer to hardware address */
		NULL			/* per interface info */
	};
