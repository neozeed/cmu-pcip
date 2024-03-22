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
#include <match.h>
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

/* netwatch variables */
struct pkt pkts[MAXPKT];

int pproc = 0;
int prcv = 0;
long npackets = 0;

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
		else {
		if(icsr & OVERRUN) proverrun++;
		if(icsr & PARITY) prparity++;
		if(!(icsr & COPYEN)) {
			register char *data = pkts[prcv].p_data;
			unsigned long type = 0;
			int i;

			if(((pproc - prcv) & PKTMASK) == 1)
				goto punt;

			len = (inb(mkv2(V2ILCNT)) & 0xff) +
			    ((unsigned short)(inb(mkv2(V2IHCNT))&0x07) << 8);

			pkts[prcv].p_len = len;

			outb(mkv2(V2ILCNT), 0);
			outb(mkv2(V2IHCNT), 0);

			/* use fast i/o routines to copy in data.
				in theory, there ought to be less
				overhead than DMA.
			*/
			fastin(mkv2(V2IBUF), data, MATCH_DATA_LEN);

			prcv = (prcv+1) & PKTMASK;
	
	punt:		outb(mkv2(V2ILCNT), 0);
			outb(mkv2(V2IHCNT), 0);
			}
		}

		outb(pr_ocwr, pr_eoi);
		outb(mkv2(V2ICSR), MODE2|MODE1|ININTRES);
		outb(mkv2(V2ICSR), MODE2|MODE1|COPYEN|ININTEN);
		}

		ocsr = inb(mkv2(V2OCSR));
		if(ocsr & OUTINTSTAT) {
			change = TRUE;
			if(ocsr & REFUSED) prref++;

			/* reenable interrupts */
			outb(pr_ocwr, pr_eoi);
			outb(mkv2(V2OCSR), OUTINTRES);
			outb(mkv2(V2OCSR), OUTINTEN);
			}
		}
	}
