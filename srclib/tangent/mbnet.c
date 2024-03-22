/*  Copyright 1986 by Carnegie Mellon  */
/*  See permission and disclaimer notice in file "cmu-note.h"  */
#include	<cmu-note.h>

#include <task.h>
#include <q.h>
#include <netq.h>
#include <net.h>
#include <custom.h>
#include <ah.h>
#include <sdlc.h>

#define NULL	0

/* This is the network configuration file for the new IP code. This file
	configures only for the Tangent Technologies PC MacBridge (appletalk)
	serial line net.	*/

int Nnet = 1;		/* The number of networks. */
char _net_if_name = 'm';	/* Default NETCUST = "NETCUSTM" */

/* random fnctns. */
int mb_init(), mb_send(), mb_switch(), mb_close(), mb_stat();

NET nets[1] = { "Tangent MacBridge",
		mb_init,		/* initialization routine */
		mb_send,		/* raw packet send routine */
		mb_switch,		/* switch */
		mb_close,		/* shutdown routine */
		NULL,			/* ip send */
		mb_stat,
		NULL,			/* task */
		NULL,			/* queue */
		0,			/* initp1 */
		0,			/* initp2 */
		1400,			/* stack size */
		sizeof(struct sdlc_hhdr),/* local net header */
		0,			/*   "    "  trailer */
		0L,			/* ip address */
		0L,			/* default gateway */
		0L,			/* network broadcast address */
		0L,			/* 4.2bsd network broadcast address */
		0L,			/* subnetwork broadcast address */
		&custom,		/* custom structure */
		0,			/* hardware address length */
		0,			/* hardware type */
		0,			/* pointer to hardware address */
		NULL			/* per-net info */
		};
