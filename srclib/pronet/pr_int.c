/*  Copyright 1986 by Carnegie Mellon  */
/*  See permission and disclaimer notice in file "cmu-note.h"  */
#include	<cmu-note.h>
/*  Copyright 1984, 1985 by Proteon, Inc. */
/*  See permission and disclaimer notice in file "proteon-notice.h"  */
#include	"proteon-notice.h"

#include <task.h>
#include <q.h>
#include <netq.h>
#include <net.h>
#include <custom.h>
#include <netbuf.h>
#include "pronet.h"

#define	FALSE	0
#define	TRUE	!FALSE

/* This code services the ethernet interrupt. It is called by an assembly
	language routines which saves all the registers and sets up the
	data segment. */

unsigned prbadfmt = 0;
unsigned print = 0;
unsigned prparity = 0;
unsigned proverrun = 0;
unsigned prtoobig = 0;
unsigned prpunted = 0;
unsigned prrcv = 0;
unsigned prref = 0;

extern long cticks;

pr_ihnd() {
	unsigned icsr;
	unsigned ocsr;
	unsigned len;
	int i;
	PACKET p;
	register char *data;
	unsigned change = TRUE;

	print++;

	while(change) {
	change = FALSE;

	icsr = inb(mkv2(V2ICSR));
	if(icsr & ININTSTAT) {
		change = TRUE;
		if(icsr & BADFMT) prbadfmt++;
		else if(icsr & OVERRUN) proverrun++;
		else if(icsr & PARITY) prparity++;
		else {
		if(!(icsr & COPYEN)) {
			len = (inb(mkv2(V2ILCNT)) & 0xff) +
			    ((unsigned short)(inb(mkv2(V2IHCNT))&0x07) << 8);

			if(len > LBUF || len == 0) prtoobig++;
			else if((p = getfree()) != 0) {
				prrcv++;

				outb(mkv2(V2ILCNT), 0);
				outb(mkv2(V2IHCNT), 0);

				if(custom.c_rcv_dma == 0) {
					fastin(mkv2(V2IBUF), p->nb_buff, len);
					p->nb_tstamp = cticks;
					p->nb_len = len;
					q_addt(pr_net->n_inputq, (q_elt)p);
					tk_wake(prDemux);
					}
				else {
					dma_setup(custom.c_rcv_dma, p->nb_buff, len, DMA_INPUT);

					/* do some housekeeping before we wait
						for the dma to complete.
					 */
					p->nb_tstamp = cticks;
					p->nb_len = len;
					q_addt(pr_net->n_inputq, (q_elt)p);
					tk_wake(prDemux);

					while(dma_done(custom.c_rcv_dma) != -1) ;

					dma_reset(custom.c_rcv_dma);
					}
				}
			else prpunted++;
	
			outb(mkv2(V2ILCNT), 0);
			outb(mkv2(V2IHCNT), 0);
			}
		}

		/* reenable interrupts */
		outb(pr_ocwr, pr_eoi);
		outb(mkv2(V2ICSR), MODE2|MODE1|ININTRES);
		outb(mkv2(V2ICSR), MODE2|MODE1|COPYEN|ININTEN);
		}

		ocsr = inb(mkv2(V2OCSR));
		if(ocsr & OUTINTSTAT) {
			change = TRUE;
			if(ocsr & REFUSED) prref++;

			outb(pr_ocwr, pr_eoi);
			outb(mkv2(V2OCSR), OUTINTRES);
			outb(mkv2(V2OCSR), OUTINTEN);
			}
		}
	}
