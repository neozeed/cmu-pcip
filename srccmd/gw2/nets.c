#include <ether.h>
#include <task.h>
#include <q.h>
#include <netq.h>
#include <net.h>
#include <custom.h>
#include <netbuf.h>
#include <ah.h>

#define NULL	0

int et_init(), et_send(), et_switch(), et_stat(), et_close(), ip_ether_send();
extern char _etme[6];		/* DDP - my ethernet address */

int pc_init(), pc_send(), pc_close();

int sl_init(), sl_write(), sl_switch(), sl_open(), sl_close(), sl_stat();

static
nulldev()
{
}

/* DDP - Plummer's internals. All constants are already byte-swapped. */
#define	ARETH	0x1		/* DDP - ethernet hardware type */

NET nets[] = {
		{ "3COM Ethernet",	/* interface name */
		et_init,		/* initialization routine */
		et_send,		/* raw packet send routine */
		et_switch,		/* DDP interrupt vector swap routine */
		et_close,		/* shutdown routine */
		NULL,
		et_stat,		/* statistics routine */
		NULL,			/* demultiplexing task */
		NULL,			/* packet queue */
					/* first parameter...*/
		0,			/* ...normal mode for others */
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

		{ "SLIP",
		sl_init,
		sl_write,
		sl_switch,		/* switch	*/
		sl_close,
		0,			/* ip send */
		sl_stat,
		0,			/* task */
		0,			/* queue */
		BAUD_9600,		/* initp1 */
		0,			/* initp2 */
		3000,			/* DDP stack size */
		0,			/* local net header */
		0,			/*   "    "  trailer */
		0x01206780,		/* ip address */
		0L,			/* default gateway */
		0L,			/* network broadcast address */
		0L,			/* 4.2bsd network broadcast address */
		0L,			/* subnetwork broadcast address */
		&custom,		/* custom structure */
		0,			/* hardware address length */
		0,			/* hardware type */
		0,			/* pointer to hardware address */
		0			/* per-net info */
		}
};

int Nnet = sizeof(nets) / sizeof(nets[0]);
char _net_if_name = 'z';
