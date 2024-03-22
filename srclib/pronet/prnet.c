/*  Copyright 1986 by Carnegie Mellon  */
/*  See permission and disclaimer notice in file "cmu-note.h"  */
#include	<cmu-note.h>
/*  Copyright 1984, 1985 by Proteon, Inc. */
/*  See permission and disclaimer notice in file "proteon-notice.h"  */
#include	"proteon-notice.h"


/* 10/3/84 - removed Init1(); changed to handle the new net structure
						<John Romkey>
   3/24/86 - added pr_swtch routine.  Increased stack to 1400 like ethernet.
						<Drew D. Perkins>
   8/5/86 - Add new elements of net structure for bootp support.
						<Drew D. Perkins>
*/

#include <task.h>
#include <q.h>
#include <netq.h>
#include <net.h>
#include <custom.h>
#include "pronet.h"

#define NULL	0

/* This is the network configuration file for the new IP code. This file
	configures only for a single proNET interface. Applications
	which wish to use multiple net interfaces should have their
	own configuration file.
*/

int Nnet = 1;		/* The number of networks. */
char _net_if_name = 'p';	/* DDP - Default NETCUST = "NETCUSTP" */

/* random fnctns. */
int pr_init(), pr_send(), pr_stat(), pr_close(), pr_switch();
extern char _prme;		/* DDP - my pronet address */

NET nets[1] = { "proNET",
		pr_init,
		pr_send,
		pr_switch,		/* DDP - Switch interrupt vectors */
		pr_close,
		0,			/* ip send */
		pr_stat,
		0,			/* demux */
		0,			/* queue */
		0,			/* initp1 */
		0,			/* initp2 */
		1400,			/* DDP - stack */
		sizeof(struct pr_hdr),	/* lnh */
		0,			/* lnt */
		0L,
		0L,
		0L,			/* network broadcast address */
		0L,			/* 4.2bsd network broadcast address */
		0L,			/* subnetwork broadcast address */
		&custom,
		1,			/* hardware address length */
		0,			/* hardware type */
		&_prme,			/* pointer to hardware address */
		0 };
