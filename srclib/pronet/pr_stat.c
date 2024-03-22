/*  Copyright 1986 by Carnegie Mellon  */
/*  See permission and disclaimer notice in file "cmu-note.h"  */
#include	<cmu-note.h>
/*  Copyright 1984, 1985 by Proteon, Inc. */
/*  See permission and disclaimer notice in file "proteon-notice.h"  */

/* 20-Sep-85  Drew D. Perkins (ddp), at Carnegie-Mellon University
	Added Arp statistics upcall
 */

#include	"proteon-notice.h"

#include <stdio.h>
#include <task.h>
#include <q.h>
#include <netq.h>
#include <net.h>
#include <custom.h>
#include <netbuf.h>
#include "pronet.h"

extern unsigned prbadfmt, print, prparity, proverrun, prtoobig, prpunted;
extern unsigned prtoosmall, prtx, prrcv, prref;
extern unsigned prmulti, prdrop, prpref; /* DDP */

pr_stat(fd)
	FILE *fd; {

	fprintf(fd, "Pronet ring address: %d (0%o) 0x%x\n", /* DDP */
		_prme & 0xff, _prme & 0xff, _prme & 0xff); /* DDP */
	fprintf(fd, "Pronet interface: %u interrupts\n", print);
	fprintf(fd, "txed packets %u\trcvd packets %u\n", prtx, prrcv);
	fprintf(fd, "badfmt %u\tparity %u\toverrun %u\n", prbadfmt, prparity,
							proverrun);
	fprintf(fd, "too big %u\ttoo small %u\tpunted %u\n", prtoobig,
							prtoosmall, prpunted);
	fprintf(fd, "refused %u\tdropped %u\tmulti %u\n", prref, /* DDP */
						prdrop, prmulti); /* DDP */
	fprintf(fd, "icsr = %02x\tocsr = %02x\n", inb(mkv2(V2ICSR)),
						inb(mkv2(V2OCSR)));
#ifdef V2ARP				/* DDP */
        pradstat(fd);		/* upcall to address resolution protocol */
#endif					/* DDP */
	in_stats(fd);
	}
