/*  Copyright 1986 by Carnegie Mellon  */
/*  See permission and disclaimer notice in file "cmu-note.h"  */
#include	<cmu-note.h>
/*  Copyright 1984, 1985 by Proteon, Inc. */
/*  See permission and disclaimer notice in file "proteon-notice.h"  */
#include	"proteon-notice.h"

/*
 ***********************************************************************
 * HISTORY
 * 16-Jan-86, Drew D. Perkins (ddp), Carnegie-Mellon University
 *  Merged in changes from Jacob Rekhter (IBM-ACIS) for retry on
 *  packet refused.
 *
 * 13-Jul-85, Drew D. Perkins (ddp), Carnegie-Mellon University
 *  Merged with new MIT sources.
 *
 * 23-Apr-84, Eric R. Crane (erc), Carnegie-Mellon University
 *  Changed v2_send so that it will do address resolution as per
 * Plummer's algorythm, RFC 826.
 *
 ***********************************************************************
 */

#include <task.h>
#include <q.h>
#include <netq.h>
#include <net.h>
#include <custom.h>
#include <netbuf.h>
#include <stdio.h>
#include "pronet.h"

/* this file contains the function which sends packets out over the v2 lni.
	For now it has a timeout function so that if the packet isn't sent
	within a reasonable length of time we'll notice and punt.
*/

#define	V2TIMEOUT	5
#define V2MAXRETRY	7		/* DDP */

unsigned prtmo = 0;
unsigned prtx = 0;
unsigned prpref = 0;			/* DDP */
unsigned prretry = 0;			/* DDP */

extern long cticks;
extern unsigned prref;			/* DDP */

pr_send(p, prot, len, fhost)
	PACKET p;
	unsigned prot;
	unsigned len;
	in_name fhost; {
	register struct pr_hdr *ppr;
	union {
		long _l;
		char _c[4];
		} foo;
	unsigned i;
	long time;
	unsigned stat;

	/* Set up the ethernet header. Insert our address and the address of
		the destination and the type field in the ethernet header
		of the packet. */
#ifndef	V2ARP		/* DDP */
#ifdef	DEBUG
	if(NDEBUG & (INFOMSG|NETRACE))
		printf("PR_SEND: p[%u] -> %a\n", len, fhost);
#endif

	ppr = (struct pr_hdr *)p->nb_buff;

	/* Setup the type field and the addresses in the prlni header. */
	if(prot != IP) {
		printf("PR_SEND: Bad packet type %u\n", prot);
		exit(1);
		}

	ppr->pr_type = PRONET_IP;
	foo._l = fhost;
	ppr->pr_dst = foo._c[3]; 
#else				/* DDP Start changes... */
	ppr = (struct pr_hdr *)p->nb_buff;

	switch (prot) {

	    case IP:
#ifdef	DEBUG
		if(NDEBUG & (INFOMSG|NETRACE))
		    printf("PR_SEND: p[%u] -> %a\n", len, fhost);
#endif
		if(ip2pr(&ppr->pr_dst, (in_name)fhost) == 0) {
#ifdef DEBUG
		    if (NDEBUG & (INFOMSG | NETERR))
			printf("PR_SEND: Couldn't send packet\n");
#endif
		    return 0;
		}
		ppr->pr_type = PRONET_IP;
		break;

	    case ARP:
#ifdef DEBUG
		if (NDEBUG & (INFOMSG|NETRACE))
		    printf("PR_SEND: Sending ARP Packet\n");
#endif
		ppr->pr_dst = fhost;
		ppr->pr_type = PRONET_ARP;
		break;

	    default:
#ifdef DEBUG
		if(NDEBUG & (INFOMSG|PROTERR|BUGHALT))
		    printf("PR_SEND: Unknown protocol %u.\n",prot);
#endif
		return 0;
	}
#endif
				/* DDP End changes... */
	if(NDEBUG & INFOMSG)
		printf("PR_SEND: foreign host addr %u\n", ppr->pr_dst);

	/* Now to send the packet. Copy the packet into the packet buffer,
		starting at the address -len. Then set the count to -len
		again. Then write 0x11 to the output CSR to send then
		packet, and check the status bits afterwards.
	*/
	prtx++;
	len += sizeof(struct pr_hdr);

	/* only send even length packets */
	if(len&1) len++;

	/* if tx dma channel is 0, use fast i/o routine. otherwise
		we dma to the appropriate channel.
	*/
	outb(mkv2(V2OLCNT), (-len)&0xff);
	outb(mkv2(V2OHCNT), ((-len) >> 8) & 0x07);

	if(custom.c_tx_dma == 0) {
		fastout(mkv2(V2OBUF), ppr, len);
		}
	else {
		/* disable input interrupts for the dma - don't want
		   the receiver trying to dma at the same time if
		   we both use the same channel
		 */
		if(custom.c_tx_dma == custom.c_rcv_dma)
			outb(mkv2(V2ICSR), MODE1|MODE2|COPYEN);

		dma_setup(custom.c_tx_dma, ppr, len, DMA_OUTPUT);

		time = cticks;

		while(dma_done(custom.c_tx_dma) != -1)
			if(cticks - time > V2TIMEOUT) {
				if(NDEBUG & (PROTERR|NETERR))
					printf("pr_send: dma tmo\n");

				/* reset the DMA controller & net interface */
				dma_reset(custom.c_tx_dma);
				return 0;
				}

		dma_reset(custom.c_tx_dma);
		}


	/* reenable receiver interrupts */
	outb(mkv2(V2ICSR), MODE2|MODE1|COPYEN|ININTEN);

    for (prretry = 0; prretry < V2MAXRETRY ;prretry++) { /* DDP - Retry code */
    	prtx++;						/* DDP */

	outb(mkv2(V2OLCNT), (-len)&0xff);
	outb(mkv2(V2OHCNT), ((-len) >> 8) & 0x07);
	outb(mkv2(V2OCSR), ORIGEN|INITRING);

	if(NDEBUG & INFOMSG)
		printf("PR_SEND: packet send started\n");

	while(inb(mkv2(V2OCSR)) & ORIGEN) ;

	stat = inb(mkv2(V2OCSR));
#ifdef notdef					/* DDP - Retry code */
	if((stat & REFUSED) && (NDEBUG & INFOMSG)) {
		printf("PR_SEND: refused\n");
		return 0;
		}

	if(NDEBUG & INFOMSG)
		printf("PR_SEND: output csr was %04x\n", stat);

	return len;
	}
#else
	if(stat & REFUSED) {		/* DDP Begin */
                if(NDEBUG & INFOMSG)
			printf("PR_SEND: refused\n");
		++prref;
		if(prretry==0)
			++prpref;
	}
	else {
		if(NDEBUG & INFOMSG)
			printf("PR_SEND: output csr was %04x\n", stat);
		return len;
	}
    }
    return 0;
}
#endif						/* DDP End */
