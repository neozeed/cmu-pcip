/*  Copyright 1986 by Carnegie Mellon  */
/*  See permission and disclaimer notice in file "cmu-note.h"  */
#include	<cmu-note.h>
/*  Copyright 1984 by the Massachusetts Institute of Technology  */
/*  See permission and disclaimer notice in file "notice.h"  */
#include	<notice.h>

#include <task.h>
#include <q.h>
#include <netq.h>
#include <net.h>
#include <custom.h>
#include <netbuf.h>
#include <stdio.h>
#include "3com.h"

extern unsigned etint, etfcs, etover, etdribble, etshort, etrcv, etdmadone;
extern unsigned etrcvdma, etref, ettoobig, etmulti, etdrop, etrreset;
extern unsigned ettxdma, ettmo, etsend, etunder, etcoll, etcollsx, etrdy;
extern unsigned ettxunknown;

et_stat(fd)
	FILE *fd; {
#ifndef NOSTATS
	fd = stdout;
	fprintf(fd, "Ether Stats:\n");
	fprintf(fd, "My ethernet address: %02x.%02x.%02x.%02x.%02x.%02x\n", /* DDP */
		_etme[0]&0xff, _etme[1]&0xff, _etme[2]&0xff,
		_etme[3]&0xff, _etme[4]&0xff, _etme[5]&0xff); /* DDP */
	fprintf(fd, "%4u ints\t%4u pkts rcvd\t%4u pkts sent\t%4u ints lost\n",
			 etint, etrcv, etsend, etrreset);
	fprintf(fd, "%4u underflows\t%4u colls\t%4u 16 colls\t%4u rdys\n",
			etunder, etcoll, etcollsx, etrdy);
	fprintf(fd, "%4u FCS errs\t%4u overflows\t%4u dribbles\t%4u shorts\n",
			etfcs, etover, etdribble, etshort);
	fprintf(fd, "%4u DMAs\t%4u rcv DMAs\t%4u tx DMAs\t%4u tx timeouts\n",
			etdmadone, etrcvdma, ettxdma, ettmo);
	fprintf(fd, "%4u refused\t%4u too big\t%4u dropped\t%4u multi\n",
			etref, ettoobig, etdrop, etmulti);

	fprintf(fd, "max q depth %u\n", et_net->n_inputq->q_max);
#endif
	etadstat(fd);		/* upcall to address resolution protocol */
#ifndef WATCH
	in_stats(fd);		/* upcall to internet */
#endif
	}

#ifdef	DEBUG
et_dump(p)
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
