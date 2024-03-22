/*  Copyright 1986 by Carnegie Mellon  */
/*  See permission and disclaimer notice in file "cmu-note.h"  */
#include	<cmu-note.h>
/*  Copyright 1984, 1985 by the Massachusetts Institute of Technology  */
/*  See permission and disclaimer notice in file "notice.h"  */
#include	<notice.h>

#include <task.h>
#include <q.h>
#include <netq.h>
#include <net.h>
#include <custom.h>
#include <netbuf.h>
#include <stdio.h>
#include <ether.h>

/* Process an incoming ethernet packet. Upcall the appropriate protocol
	(Internet, Chaos, PUP, NS, ARP, ...).
	Does nothing special with multicast packets (could try to maintain
	a list of multicast addresses we're interested in and discard other
	packets but we don't).
*/

extern unsigned etdrop;

/* we're passed the net interface structure so that this task can be used
	by several interfaces.
*/

il_demux(et_net)
	NET *et_net; {
	register PACKET p;
	unsigned type;
	register struct ethhdr *pet;

	/* sit in a loop processing packets. when none available to
		process, block.
	*/

	while(1) {

	/* try to pull a packet off the net interface queue */
	p = (PACKET)aq_deq(et_net->n_inputq);

	if(p == NULL) {
		/* block when there's not a packet to process */
		tk_block();
		continue;
		}

	if(p->nb_len < ET_MINLEN) {
#ifdef	DEBUG
		if((NDEBUG & (NETRACE|NETERR)) == (NETRACE|NETERR))
			printf("ETDEMUX: pkt too small: %u\n", p->nb_len);
#endif
		putfree(p);
		continue;
		}

#ifdef	DEBUG
	if(NDEBUG & NETRACE)
		printf("ET_DEMUX: got pkt len %u\n", p->nb_len);
#endif

	/* Check on what protocol this packet is and upcall the
		appropriate protocol handler.
	*/
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
		if((NDEBUG & DUMP)&&(NDEBUG & NETRACE))
			et_dump(p);
#endif
		etdrop++;
		putfree(p);
	}

	if(et_net->n_inputq->q_head) {
#ifdef	DEBUG
		if(NDEBUG & INFOMSG)
			printf("ETDEMUX: More pkts; yielding\n");
#endif
		tk_yield();
		}
	}
}
