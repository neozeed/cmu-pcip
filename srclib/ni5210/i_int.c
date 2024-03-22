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

#ifdef	WATCH
#include <match.h>
#endif

extern 	long	cticks;

/* This code services the ethernet interrupt. It is called by an assembly
	language routines which saves all the registers and sets up the
	data segment. */

unsigned iint = 0;
unsigned ifcs = 0;
unsigned iover = 0;
unsigned idribble = 0;
unsigned ishort = 0;
unsigned ircv = 0;
unsigned iref = 0;
unsigned itoobig = 0;
unsigned inoresources = 0;

#ifdef	WATCH
struct pkt pkts[MAXPKT];

int pproc = 0;
int prcv = 0;
long npackets = 0;

#endif

extern unsigned ircvcmd;

i_ihnd(keepalive) int keepalive; {
	unsigned int rcv;
	unsigned len, buflen;
	PACKET i_inp;
	RFD far *RFDPTR;
	RBD far *RBDPTR, far *RBDEND;

	rcv = SCBPTR->status;
	SCBPTR->status = 0;
printf("STATUS = %x %d\n", rcv, keepalive);
	if (!keepalive) {
		SCBPTR->command = rcv & (ICX|IFR|ICNR|IRNR);
		doca();
		if ((rcv & IRUSMASK) == IRNORESOURCES) inoresources++;
	}
	if (rcv & IFR) {
		iint++;
		RFDPTR = MAKEADDR(SCBPTR->rfaoff, RFD);
		while (RFDPTR->cmd.status & ICOMPLETE) {
			SCBPTR->rfaoff = RFDPTR->cmd.link;
			rcv = RFDPTR->cmd.status;
			RFDPTR->cmd.status = 0;
			RBDEND = RBDPTR = MAKEADDR(RFDPTR->rbdoff, RBD);
			RFDPTR->rbdoff = NIL;
			RFDPTR->cmd.command |= IEOF;
			RFDPTR->cmd.link = NIL;
			BOTRFD->cmd.link = STRIPOFF(RFDPTR);
			if (rcv & IOK) {
				while (!(RBDEND->count & IEOF))
					RBDEND = MAKEADDR(RBDEND->rbdoff,RBD);
				RBDEND = MAKEADDR(RBDEND->rbdoff, RBD);
				MAKEADDR(SCBPTR->rfaoff, RFD)->rbdoff =
					STRIPOFF(RBDEND);
				ircv++;
				len = RFDPTR->length;
#ifdef DOIEEE
				if (len > LBUF || len == 0) {
					itoobig++;
					continue;
				}
#endif
				i_inp = getfree();
				if (i_inp != NULL) {
					len = sizeof(RFDPTR->dstaddr) +
						sizeof(RFDPTR->srcaddr) +
						sizeof(RFDPTR->length);
					gencpy((char far *)RFDPTR->dstaddr,
						(char far *)i_inp->nb_buff,
						len);
				} else
					iref++;
				BOTRBD->rbdoff = STRIPOFF(RBDPTR);
				for (;;) {
					rcv = RBDPTR->count;
					RBDPTR->count = 0;
					if (i_inp != NULL) {
						if (rcv & ICOUNTVALID)
							buflen = rcv & ICOUNTMASK;
						else
							buflen = RBDPTR->size & ISIZEMASK;
						gencpy((char far *)RBDPTR->buffer,
							(char far *)&i_inp->nb_buff[len],
							buflen);
						len += buflen;
					}
				if (rcv & IEOF) break;
					RBDPTR = MAKEADDR(RBDPTR->rbdoff, RBD);
				}
				RBDPTR->rbdoff = NIL;
				RBDPTR->size |= ILASTBLK;
				BOTRBD->size &= ~ILASTBLK;
				BOTRBD = RBDPTR;
				if (i_inp != NULL) {
					i_inp->nb_len = len;
					i_inp->nb_tstamp = cticks;
					q_addt(i_net->n_inputq, (q_elt)i_inp);
					tk_wake(iDemux);
				}
			} else {
				if (rcv & IOVERRUN) iover++;
				else if(rcv & IBADALIGN) idribble++;
				else if(rcv & ICRCERR) ifcs++;
				else if(rcv & ISHORTFRAME) ishort++;
				/* if save bad frames then recover them here */
			}
			BOTRFD->cmd.command &= ~IEOF;
			BOTRFD = RFDPTR;
			RFDPTR = MAKEADDR(SCBPTR->rfaoff, RFD);
		}
	}
	SCBPTR->command = IRSTART;
	doca();
}
