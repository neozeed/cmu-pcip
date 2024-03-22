/*  Copyright 1986 by Carnegie Mellon  */
/*  See permission and disclaimer notice in file "cmu-note.h"  */
#include	<cmu-note.h>
/*  Copyright 1984,1985 by the Massachusetts Institute of Technology  */
/*  See permission and disclaimer notice in file "notice.h"  */
#include	<notice.h>

/* 10/3/84 - removed Init1(); changed to handle the new net structure
						<John Romkey>
   3/27/86 - Increased demux task stack size to 1400 like ethernets.
						<Drew D. Perkins>
   8/5/86 - Add new elements of net structure for bootp support.
						<Drew D. Perkins>
*/

#include <task.h>
#include <q.h>
#include <netq.h>
#include <net.h>
#include <custom.h>
#include <ah.h>

#define NULL	0

/* This is the network configuration file for the new IP code. This file
	configures only for the serial line net.	*/

int Nnet = 1;		/* The number of networks. */
char _net_if_name = 's';	/* DDP - Default NETCUST = "NETCUSTS" */

/* random fnctns. */
int sl_init(), sl_write(), sl_open(), sl_close(), sl_stat();

NET nets[1] = { "Serial Line",
		sl_init,
		sl_write,
		0,			/* switch	*/
		sl_close,
		0,			/* ip send */
		sl_stat,
		0,			/* task */
		0,			/* queue */
		BAUD_9600,		/* initp1 */
		0,			/* initp2 */
		1400,			/* DDP stack size */
		4,			/* local net header */
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
		0			/* per-net info */
		};
