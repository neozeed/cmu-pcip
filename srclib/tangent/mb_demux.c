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
#include <sdlc.h>
#include "mb.h"
#include "sccreg.h"

/* Process an incoming ethernet packet. Upcall the appropriate protocol
	(Internet, Chaos, PUP, NS, ARP, ...). */

unsigned mbwpp, mbdrop, mbmulti;

mb_demux()
{
	register PACKET p;
	unsigned type;
	register struct sdlc_hhdr *psdlc;

	while(1) {
	tk_block();
	p = (PACKET)aq_deq(mb_net->n_inputq);

	if(p == 0) {
#ifdef	DEBUG
		if((NDEBUG & NETRACE) && (NDEBUG & NETERR))
		    printf("MBDEMUX: no pkt\n");
#endif
		mbwpp++;
		continue;
		}

	if(p->nb_len < SDLC_MINLEN) {
#ifdef	DEBUG
		if((NDEBUG & NETRACE) && (NDEBUG & NETERR))
			printf("MBDEMUX: pkt[%u] too small\n", p->nb_len);
#endif
		putfree(p);
		continue;
		}

#ifdef	DEBUG
	if(NDEBUG & NETRACE)
		printf("MB_DEMUX: got pkt[%u]", p->nb_len);
#endif

	/* Check on what protocol this packet is and upcall it. */
	p->nb_prot = p->nb_buff+sizeof(struct sdlc_hhdr);
	psdlc = (struct sdlc_hhdr *)p->nb_buff;
	type = psdlc->sdlc_type;
	switch(type) {
	case SDLC_IP:
#ifdef	DEBUG
		if(NDEBUG & NETRACE)
			printf(" type IP\n");
#endif
		indemux(p, p->nb_len-sizeof(struct sdlc_hhdr), mb_net);
		break;

	default:
#ifdef	DEBUG
		if(NDEBUG & NETRACE)
			printf(" type unknown: %04x, dropping\n",
			       			bswap(type));
#endif
		mbdrop++;
		putfree(p);
	}

	if(mb_net->n_inputq->q_head) {
#ifdef	DEBUG
		if(NDEBUG & INFOMSG)
			printf("MBDEMUX: More pkts; yielding\n");
#endif
		mbmulti++;
		tk_wake(tk_cur);
		}
	}
}
