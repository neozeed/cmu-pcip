/* Copyright 1988 Bradley N. Davis, Darbick Instructional Software Systems */
/* See permission and disclaimer notice in file "bnd-note.h" */
#include	"bnd-note.h"
/*  Copyright 1986 by Carnegie Mellon  */
/*  See permission and disclaimer notice in file "cmu-note.h"  */
#include	<cmu-note.h>
/*  Copyright 1984 by the Massachusetts Institute of Technology  */
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
#include "i82586.h"

extern unsigned iint, ifcs, iover, idribble, ishort, ircv;
extern unsigned iref, itoobig, imulti, idrop, irreset;
extern unsigned itmo, isend, iunder, icoll, icollsx, irdy;
extern unsigned itxunknown;
extern unsigned inoresources;

i_stat(fd)
	FILE *fd; {
#ifndef NOSTATS
	fd = stdout;
	fprintf(fd, "Ether Stats:\n");
	fprintf(fd, "My ethernet address: %02x.%02x.%02x.%02x.%02x.%02x\n", /* DDP */
		_ime[0]&0xff, _ime[1]&0xff, _ime[2]&0xff,
		_ime[3]&0xff, _ime[4]&0xff, _ime[5]&0xff); /* DDP */
	fprintf(fd, "%4u ints\t%4u pkts rcvd\t%4u pkts sent\t%4u ints lost\n",
			 iint, ircv, isend, irreset);
	fprintf(fd, "%4u underflows\t%4u colls\t%4u 16 colls\t%4u rdys\n",
			iunder, icoll, icollsx, irdy);
	fprintf(fd, "%4u FCS errs\t%4u overflows\t%4u dribbles\t%4u shorts\n",
			ifcs, iover, idribble, ishort);
	fprintf(fd, "%4u tx timeouts\t%4u resc\n", itmo, inoresources);
	fprintf(fd, "%4u refused\t%4u too big\t%4u dropped\t%4u multi\n",
			iref, itoobig, idrop, imulti);

	fprintf(fd, "max q depth %u\n", i_net->n_inputq->q_max);
#endif
	iadstat(fd);		/* upcall to address resolution protocol */
#ifndef WATCH
	in_stats(fd);		/* upcall to internet */
#endif
	}

#ifdef	DEBUG
i_dump(p)
	register PACKET p; {
	register char *data;
	int i;

	data = p->nb_buff;
	for(i=1; i<121; i++) {
		printf("%02x ", (*data++)&0xff);
		if(i%20 == 0) printf("\n");
		}
	puts("");
	}

#endif
