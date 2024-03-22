#include <ether.h>
#include <task.h>
#include <q.h>
#include <netq.h>
#include <net.h>
#include <custom.h>
#include <netbuf.h>

#define NULL	0

int Nnet = 2;
char _net_if_name = 'z';

int et_init(), et_send(), et_switch(), et_stat(), et_close(), ip_ether_send();
extern char _etme[6];		/* DDP - my ethernet address */

int pc_init(), pc_send(), pc_close();

static
nulldev()
{
}

/* DDP - Plummer's internals. All constants are already byte-swapped. */
#define	ARETH	0x1		/* DDP - ethernet hardware type */

NET nets[2] = {
		{ "3COM Ethernet",	/* interface name */
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
		3000,			/* DDP demux task stack size */
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
	},

		{ "PCNET",		/* interface name */
		pc_init,		/* initialization routine */
		pc_send,		/* raw packet send routine */
		NULL,			/* DDP interrupt vector swap routine */
		pc_close,		/* shutdown routine */
		NULL,
		nulldev,			/* statistics routine */
		NULL,			/* demultiplexing task */
		NULL,			/* packet queue */
					/* first parameter...*/
		0,			/* ...normal mode for others */
		0,			/* second parameter unused */
		3000,			/* DDP demux task stack size */
		6,			/* local net header size */
		0,			/* local net trailer size */
		0x011f6780,		/* ip address */
		0L,			/* default gateway */
		0L,			/* network broadcast address */
		0L,			/* 4.2bsd network broadcast address */
		0L,			/* subnetwork broadcast address */
		&custom,		/* our custom structure! */
		6,			/* hardware address length */
		0,			/* hardware type */
		NULL,			/* pointer to hardware address */
		NULL			/* per interface info */
	}
};
