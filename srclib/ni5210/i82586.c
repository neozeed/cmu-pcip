/* Copyright 1988 Bradley N. Davis, Darbick Instructional Software Systems */
/* See permission and disclaimer notice in file "bnd-note.h" */
#include	"bnd-note.h"
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

#include <types.h>
#include <ether.h>
#include <task.h>
#include <q.h>
#include <netq.h>
#include <net.h>
#include <custom.h>
#include <netbuf.h>
#include "i82586.h"

#define NULL	0


/* This is the network configuration file for the new IP code. This file
	sets up the configuration for a single ethernet interface
	machine. Programs which want to use multiple net interfaces
	should use their own configuration file.
*/

int Nnet = 1;		/* The number of networks. */

/* random fnctns. */
int i_init(), i_send(), i_switch(), i_stat(), i_close(), ip_ether_send();
extern char _ime[6];		/* DDP - my ethernet address */

/* DDP - Plummer's internals. All constants are already byte-swapped. */
#define	ARETH	0x1		/* DDP - ethernet hardware type */

#ifdef LCS8833
char _net_if_name = 'L';	/* DDP - Default NETCUST = "NETCUSTL" */
NET nets[1] = { "Longshine LCS-8833N Ethernet",	/* interface name */
#endif
#ifdef MI5210
char _net_if_name = 'M';	/* DDP - Default NETCUST = "NETCUSTL" */
NET nets[1] = { "Micom/Interlan 5210a Ethernet",	/* interface name */
#endif
		i_init,		/* initialization routine */
		i_send,		/* raw packet send routine */
		i_switch,	/* DDP interrupt vector swap routine */
		i_close,	/* shutdown routine */
/*		ip_ether_send,	/* ip packet send routine */
		NULL,
		i_stat,		/* statistics routine */
		NULL,		/* demultiplexing task */
		NULL,		/* packet queue */
				/* first parameter...*/
#ifdef	WATCH
		ALLPACK,	/* ...promiscuous mode for netwatch */
#endif
#ifndef	WATCH
		0,		/* ...normal mode for others */
#endif
		0,		/* second parameter unused */
		1400,		/* DDP demux task stack size */
		14,		/* local net header size */
		0,		/* local net trailer size */
		0L,		/* ip address */
		0L,		/* default gateway */
		0L,		/* network broadcast address */
		0L,		/* 4.2bsd network broadcast address */
		0L,		/* subnetwork broadcast address */
		&custom,	/* our custom structure! */
		6,		/* hardware address length */
		ARETH,		/* hardware type */
		_ime,		/* pointer to hardware address */
		NULL		/* per interface info */
	};

IASETUP_COMMAND far *ADDRPTR;
MCSETUP_COMMAND far *BROADPTR;
TDR_COMMAND far *TDRPTR;
DIAGNOSE_COMMAND far *DIAGPTR;
DUMP_COMMAND far *DMPPTR;
CONFIGURE_COMMAND far *CONFPTR;

SCB far *SCBPTR;
TBD far *TBDPTR;
TCB far *TCBPTR;
RFD far *BOTRFD;
RBD far *BOTRBD;
