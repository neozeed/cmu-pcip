/* Copyright 1988 Bradley N. Davis, Darbick Instructional Software Systems */
/* See permission and disclaimer notice in file "bnd-note.h" */
#include	"bnd-note.h"
/*  Copyright 1986 by Carnegie Mellon  */
/*  See permission and disclaimer notice in file "cmu-note.h"  */
#include	<cmu-note.h>
/*  Copyright 1984, 1985 by the Massachusetts Institute of Technology  */
/*  See permission and disclaimer notice in file "notice.h"  */
#include	<notice.h>

#include <types.h>
#include <task.h>
#include <q.h>
#include <netq.h>
#include <net.h>
#include <custom.h>
#include <netbuf.h>
#include <stdio.h>
#include <ether.h>
#include "i82586.h"
#include <stdio.h>

/* Transmit a packet. If it's an IP packet, calls ARP to figure out the
	ethernet address
*/

#define	iTIMEOUT	5	/* timeout in 1/18 secs for sending pkts */

unsigned idmstart;
extern unsigned idmadone;
unsigned itxdma = 0;
unsigned itmo = 0;
unsigned isend = 0;
unsigned iunder = 0;
unsigned icoll  = 0;
unsigned icollsx = 0;
unsigned irdy = 0;
unsigned itxunknown = 0;

#ifdef	WATCH
unsigned iminlen;
#endif

extern long cticks;

i_send(p, prot, len, fhost)
	PACKET p;
	unsigned prot;
	unsigned len;
	in_name fhost; {
	register struct ethhdr *pe;
	unsigned temp;

	/* Set up the ethernet header. Insert our address and the address of
		the destination and the type field in the ethernet header
		of the packet. */
#ifdef	DEBUG
	if(NDEBUG & (INFOMSG|NETRACE))
		printf("i_SEND: p[%u] -> %a.\n", len, fhost);
#endif

	pe = (struct ethhdr *)p->nb_buff;
	iadcpy(_ime, pe->e_src);

	/* Setup the type field and the addresses in the ethernet header. */
	switch(prot) {
	case IP:
		if((fhost == 0xffffffff) || /* Physical cable broadcast addr*/
					/* All subnet broadcast */
			(fhost == i_net->n_netbr) ||
					/* All subnet bcast (4.2bsd) */
			(fhost == i_net->n_netbr42) ||
					/* Subnet broadcast */
			(fhost == i_net->n_subnetbr)) {
			iadcpy(iBROADCAST, pe->e_dst);
			}
		else if(ip2et(pe->e_dst, fhost) == 0) {
#ifdef	DEBUG
			if(NDEBUG & (INFOMSG|NETERR))
				printf("i_SEND: ether address unknown\n");
#endif
			return 0;
			}
		pe->e_type = ET_IP;
		break;
	case ARP:
		iadcpy((char *)fhost, pe->e_dst);
		pe->e_type = ET_ARP;
		break;
	default:
#ifdef	DEBUG
		if(NDEBUG & (INFOMSG|PROTERR|BUGHALT))
			printf("i_SEND: Unknown prot %u.\n", prot);
#endif
		return 0;
		}

#ifdef	WATCH
	if(len < iminlen - sizeof(struct ethhdr))
		len = iminlen - sizeof(struct ethhdr);
#else
	if(len < ET_MINLEN - sizeof(struct ethhdr))
		len = ET_MINLEN - sizeof(struct ethhdr);
#endif
	Wait_SCB();
	TCBPTR->cmd.status = 0;
	TCBPTR->cmd.command = IEOCL | ITRANSMIT;
	/* e_src gets set by chip */
	iadcpy(pe->e_dst, TCBPTR->destaddr);
	TCBPTR->length = pe->e_type;
	gencpy((char far *)&p->nb_buff[sizeof(struct ethhdr)],
	       (char far *)TBDPTR->buffer, len);
	TBDPTR->count = len | IEOF;
	SCBPTR->cbloff = STRIPOFF(TCBPTR);
	SCBPTR->command = ICSTART;
	doca();
	isend++;

	Wait_SCB();
	temp = TCBPTR->cmd.status;
	if (temp & IUNDERRUN)
		iunder++;
	else if (temp & ICOLL16) {
		icollsx++;
#ifdef DEBUG
		printf("Excessive collisions detected.  Is the ethernet cable plugged in?\n");
#endif
	} else if (!(temp & IOK))
		itxunknown++;
	else
		irdy++;

	return len;
}
