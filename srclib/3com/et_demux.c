/*  Copyright 1986 by Carnegie Mellon  */
/*  See permission and disclaimer notice in file "cmu-note.h"  */
#include	<cmu-note.h>
/*  Copyright 1984 by the Massachusetts Institute of Technology  */
/*  See permission and disclaimer notice in file "notice.h"  */
#include	<notice.h>

/* 7/10/84 - moved some variables definitions into et_int.c.
					<John Romkey>
   7/16/84 - changed debugging level on short packet message to only
	INFOMSG.			<John Romkey>
   6/5/85  - changed debugging levels to support separate tracing of
        network level; consolidated calls to tk_block.
					<J. H. Saltzer>
*/

#include <task.h>
#include <q.h>
#include <netq.h>
#include <net.h>
#include <custom.h>
#include <netbuf.h>
#include <stdio.h>
#include <ether.h>
#include "3com.h"

/* Process an incoming ethernet packet. Upcall the appropriate protocol
	(Internet, Chaos, PUP, NS, ARP, ...). This does not check on
	my address and does not support multicast. It may in the future
	attempt to do more of the right thing with broadcast. */

unsigned etwpp = 0;	/* number of times awakened w/o packet to process */
unsigned etdrop = 0;	/* # of packets dropped */
unsigned etmulti = 0;	/* # of times more than one packet on queue */

et_demux() {
	register PACKET p;
	unsigned type;
	register struct ethhdr *pet;

	while(1) {
	tk_block();
	p = (PACKET)aq_deq(et_net->n_inputq);

	if(p == 0) {
#ifdef	DEBUG
		if((NDEBUG & NETRACE) && (NDEBUG & NETERR))
		    printf("ETDEMUX: no pkt\n");
#endif
		etwpp++;
		continue;
		}

	if(p->nb_len < ET_MINLEN) {
#ifdef	DEBUG
		if((NDEBUG & NETRACE) && (NDEBUG & NETERR))
			printf("ETDEMUX: pkt[%u] too small\n", p->nb_len);
#endif
		putfree(p);
		continue;
		}

#ifdef	DEBUG
	if(NDEBUG & NETRACE)
		printf("ET_DEMUX: got pkt[%u]", p->nb_len);
#endif

	/* Check on what protocol this packet is and upcall it. */
	p->nb_prot = p->nb_buff+sizeof(struct ethhdr);
	pet = (struct ethhdr *)p->nb_buff;
	type = pet->e_type;
	switch(type) {
	case ET_IP:
#ifdef	DEBUG
		if(NDEBUG & NETRACE)
			printf(" type IP\n");
#endif
		indemux(p, p->nb_len-sizeof(struct ethhdr), et_net);
		break;
	case ET_ARP:
#ifdef	DEBUG
		if(NDEBUG & NETRACE)
			printf(" type ARP\n");
#endif
		etarrcv(p, p->nb_len-sizeof(struct ethhdr));
		break;
	default:
#ifdef	DEBUG
		if(NDEBUG & NETRACE)
			printf(" type unknown: %04x, dropping\n",
			       			bswap(type));
#endif
		etdrop++;
#ifdef	DEBUG
		if((NDEBUG & DUMP)&&(NDEBUG & NETRACE)) et_dump(p);
#endif
		putfree(p);
	}

	if(et_net->n_inputq->q_head) {
#ifdef	DEBUG
		if(NDEBUG & INFOMSG)
			printf("ETDEMUX: More pkts; yielding\n");
#endif
		etmulti++;
		tk_wake(tk_cur);
		}
	}
}
