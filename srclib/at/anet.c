#include <task.h>
#include <q.h>
#include <netq.h>
#include <net.h>
#include <custom.h>
#include <netbuf.h>

int Nnet = 1;
char _net_if_name = 't';

int at_init(), at_send(), at_switch(), at_close();

static
nulldev()
{
}

NET nets[1] = { "AppleTalk",
		at_init,		/* initialization routine */
		at_send,		/* raw packet send routine */
		at_switch,		/* DDP interrupt vector swap routine */
		at_close,		/* shutdown routine */
		0,
		nulldev,		/* statistics routine */
		0,			/* demultiplexing task */
		0,			/* packet queue */
					/* first parameter...*/
		0,			/* ...normal mode for others */
		0,			/* second parameter unused */
		1400,			/* DDP demux task stack size */
		0,			/* local net header size */
		0,			/* local net trailer size */
		0L,			/* ip address */
		0L,			/* default gateway */
		0L,			/* network broadcast address */
		0L,			/* 4.2bsd network broadcast address */
		0L,			/* subnetwork broadcast address */
		&custom,		/* our custom structure! */
		1,			/* hardware address length */
		0,			/* hardware type */
		0,			/* pointer to hardware address */
		0			/* per interface info */
};
