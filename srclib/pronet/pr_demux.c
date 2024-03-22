/*  Copyright 1986 by Carnegie Mellon  */
/*  See permission and disclaimer notice in file "cmu-note.h"  */
#include	<cmu-note.h>
/*  Copyright 1984, 1985 by Proteon, Inc. */
/*  See permission and disclaimer notice in file "proteon-notice.h"  */
#include	"proteon-notice.h"

#include <stdio.h>
#include <task.h>
#include <q.h>
#include <netq.h>
#include <net.h>
#include <custom.h>
#include <netbuf.h>
#include "pronet.h"

/***********************************************************************
 * HISTORY
 *
 * 22-Mar-86 Drew D. Perkins (ddp), Carnegie-Mellon University
 *  Merged with newest MIT sources.
 *
 * 13-Jul-85 Drew D. Perkins (ddp), Carnegie-Mellon University
 *  Merged with new MIT sources.
 *
 * 22-Apr-84 Eric R. Crane (erc) Carnegie-Mellon University
 *  Added address resolution code to the V2 proNET driver.
 *
 ***********************************************************************/

/* Process an incoming packet from the ring.
*/
unsigned prdrop = 0;	/* # of packets dropped */
unsigned prmulti = 0;	/* # of times more than one packet on queue */
unsigned prtoosmall = 0;

pr_demux() {
	register PACKET p;
	register struct pr_hdr *ppr;
	unsigned len;
	int i;

#ifdef	DEBUG
	if(NDEBUG & INFOMSG)
		printf("PRDEMUX activated.\n");
#endif

	tk_block();

	while(1) {
	/* get a packet off the queue */
	p = (PACKET)aq_deq(pr_net->n_inputq);

	if(p == NULL) {
		tk_block();
		continue;
		}

	if(p->nb_len < V2MINLEN) {
#ifdef	DEBUG
		if(NDEBUG & (NETERR|PROTERR|INFOMSG))
			printf("PR_DEMUX: p[%u ] too small\n", p->nb_len);
#endif
		prtoosmall++;
		putfree(p);
		tk_block();
		continue;
		}

	/* Check on what protocol this packet is and upcall it. */
	p->nb_prot = p->nb_buff + sizeof(struct pr_hdr);
	ppr = (struct pr_hdr *)p->nb_buff;

	switch(ppr->pr_type) {
	case PRONET_IP:
#ifdef	DEBUG
		if(NDEBUG & (INFOMSG|NETRACE))
			printf("PR_DEMUX: got an IP pkt\n");
#endif
		indemux(p, p->nb_len-sizeof(struct pr_hdr), pr_net);
		break;
/* DDP Start changes... */
#ifdef V2ARP
	case PRONET_ARP:
#ifdef	DEBUG
		if(NDEBUG & (INFOMSG|NETRACE))
			printf("PR_DEMUX: Received ARP packet.\n");
#endif
		pr_arrcv(p, p->nb_len-sizeof(struct pr_hdr));
		break;

	case PRONET_RINGWAY:
		putfree(p);
		prdrop++;
		break;
#endif
/* DDP End changes */
	default:
#ifdef	DEBUG
		if(NDEBUG & (INFOMSG|NETRACE))
			printf("PR_DEMUX: Unknown protocol %08X\n",
							 ppr->pr_type);
		if(NDEBUG & DUMP) in_dump(p);
#endif
		prdrop++;
		putfree(p);
	}

	if(pr_net->n_inputq->q_head) {
		prmulti++;
		tk_wake(tk_cur);
		}

	tk_block();
	}

}
