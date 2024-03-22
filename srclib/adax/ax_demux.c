/*  Copyright 1986 by Carnegie Mellon  */
/*  See permission and disclaimer notice in file "cmu-note.h"  */
#include	<cmu-note.h>

#include <stdio.h>
#include <task.h>
#include <q.h>
#include <netq.h>
#include <net.h>
#include <custom.h>
#include <netbuf.h>
#include <lapb.h>
#include "ax.h"
#include "mpsccreg.h"

/* Process an incoming ethernet packet. Upcall the appropriate protocol
	(Internet, Chaos, PUP, NS, ARP, ...). */

unsigned axwpp, axdrop, axmulti;

ax_demux()
{
	register PACKET p;
	unsigned type;
	register struct lapb_hhdr *plapb;

	while(1) {
	tk_block();
	p = (PACKET)aq_deq(ax_net->n_inputq);

	if(p == 0) {
#ifdef	DEBUG
		if((NDEBUG & NETRACE) && (NDEBUG & NETERR))
		    printf("AXDEMUX: no pkt\n");
#endif
		axwpp++;
		continue;
		}

	if(p->nb_len < LAPB_MINLEN) {
#ifdef	DEBUG
		if((NDEBUG & NETRACE) && (NDEBUG & NETERR))
			printf("AXDEMUX: pkt[%u] too small\n", p->nb_len);
#endif
		putfree(p);
		continue;
		}

#ifdef	DEBUG
	if(NDEBUG & NETRACE)
		printf("AX_DEMUX: got pkt[%u]", p->nb_len);
#endif

	/* Check on what protocol this packet is and upcall it. */
	p->nb_prot = p->nb_buff+sizeof(struct lapb_hhdr);
	plapb = (struct lapb_hhdr *)p->nb_buff;
	type = plapb->lapb_type;
	switch(type) {
	case LAPB_IP:
#ifdef	DEBUG
		if(NDEBUG & NETRACE)
			printf(" type IP\n");
#endif
		indemux(p, p->nb_len-sizeof(struct lapb_hhdr), ax_net);
		break;

	default:
#ifdef	DEBUG
		if(NDEBUG & NETRACE)
			printf(" type unknown: %04x, dropping\n",
			       			bswap(type));
		if(NDEBUG & DUMP) {
			int i;

			printf("Packet contents:");
			for(i = p->nb_len > 64 ? 64 : p->nb_len; i; i--)
				printf(" %02x", p->nb_buff[i]);
			printf("\n");
		}
#endif
		axdrop++;
		putfree(p);
	}

	if(ax_net->n_inputq->q_head) {
#ifdef	DEBUG
		if(NDEBUG & INFOMSG)
			printf("AXDEMUX: More pkts; yielding\n");
#endif
		axmulti++;
		tk_wake(tk_cur);
		}
	}
}
