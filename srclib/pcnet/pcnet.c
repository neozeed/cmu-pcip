#include <task.h>
#include <q.h>
#include <netq.h>
#include <net.h>
#include <custom.h>
#include <netbuf.h>

int Nnet = 1;
char _net_if_name = 'c';

int pc_init(), pc_send(), pc_switch(), pc_close();

static
nulldev()
{
}

NET nets[1] = { "PCNET",		/* interface name */
		pc_init,		/* initialization routine */
		pc_send,		/* raw packet send routine */
		pc_switch,		/* DDP interrupt vector swap routine */
		pc_close,		/* shutdown routine */
		0,
		nulldev,		/* statistics routine */
		0,			/* demultiplexing task */
		0,			/* packet queue */
					/* first parameter...*/
		0,			/* ...normal mode for others */
		0,			/* second parameter unused */
		1400,			/* DDP demux task stack size */
		6,			/* local net header size */
		0,			/* local net trailer size */
		0L,			/* ip address */
		0L,			/* default gateway */
		0L,			/* network broadcast address */
		0L,			/* 4.2bsd network broadcast address */
		0L,			/* subnetwork broadcast address */
		&custom,		/* our custom structure! */
		6,			/* hardware address length */
		0,			/* hardware type */
		0,			/* pointer to hardware address */
		0			/* per interface info */
};
