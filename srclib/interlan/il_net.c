/*  Copyright 1986 by Carnegie Mellon  */
/*  See permission and disclaimer notice in file "cmu-note.h"  */
#include	<cmu-note.h>
/*  Copyright 1985 by Micom-Interlan, Inc. */
/*  See permission and disclaimer notice in file "interlan-notice.h"  */
#include	"il-notice.h"

/* 8/10/85 - wrote for interlan ethernet interface.
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
  ***** OF COURSE, PC/IP DOES NOT YET SUPPORT MULTIPLE NET INTERFACES...

*/

int Nnet = 1;		/* The number of networks. */
char _net_if_name = 'i';	/* DDP - Default NETCUST = "NETCUSTI" */

/* random fnctns. */
int il_init(), il_send(), il_switch(), il_stat(), il_close(), ip_ether_send();
extern char _etme[6];		/* DDP - my ethernet address */

/* DDP - Plummer's internals. All constants are already byte-swapped. */
#define	ARETH	0x1		/* DDP - ethernet hardware type */

NET nets[1] = { "Interlan Ethernet",	/* interface name */
		il_init,		/* initialization routine */
		il_send,		/* raw packet send routine */
		il_switch,		/* interrupt vector swap routine */
		il_close,		/* shutdown routine */
		ip_ether_send,		/* ip packet send routine */
		il_stat,		/* statistics routine */
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
