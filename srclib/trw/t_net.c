/*  Copyright 1986 by Carnegie Mellon  */
/*  See permission and disclaimer notice in file "cmu-note.h"  */
#include	<cmu-note.h>
/*  Copyright 1984 by the Massachusetts Institute of Technology  */
/*  See permission and disclaimer notice in file "notice.h"  */
#include	<notice.h>

/* Copyright 1988 TRW Information Networks */
/*  See permission and disclaimer notice in file "trw-note.h"  */
#include	"trw-note.h"

/* $Revision:   1.0  $		$Date:   29 Feb 1988 20:20:22  $	*/

#include <stdio.h>
#include <ether.h>
#include <task.h>
#include <q.h>
#include <netq.h>
#include <net.h>
#include <custom.h>
#include <netbuf.h>
#include "trw.h"

#define NULL	0

/* This is the network configuration file for the TRW PC-2000 Ethernet	*/
/* board.								*/

int Nnet = 1;		/* The number of networks. */
char _net_if_name = 'T';	/* Default NETCUST = "NETCUSTT" */

/* DDP - Plummer's internals. All constants are already byte-swapped. */
#define	ARETH	0x1		/* DDP - ethernet hardware type */

NET nets[1] = { "TRW Ethernet/802.3",		/* interface name */
		trw_init,		/* initialization routine */
		trw_send,		/* raw packet send routine */
		trw_switch,		/* DDP interrupt vector swap routine */
		trw_close,		/* shutdown routine */
		NULL,			/* ip packet send routine */
		trw_stat,		/* statistics routine */
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
