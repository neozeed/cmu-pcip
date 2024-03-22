/*  Copyright 1986 by Carnegie Mellon  */
/*  See permission and disclaimer notice in file "cmu-note.h"  */
#include	<cmu-note.h>

#include <stdio.h>
#include <task.h>
#include <q.h>
#include <netq.h>
#include <net.h>
#include <custom.h>
#include <netbuf.h>
#include "ax.h"

/* print some pretty statistics */

extern unsigned axint, axspurint, axspecrx, axextstat, axrca, axtbe, axbadint;
extern unsigned axrcv, axtoobig, axcrc, axrxovr;
extern unsigned axabort, axunabort, axsync, axunsync;
extern unsigned axctson, axctsoff, axdcdon, axdcdoff;
extern int ax_aborting, ax_insync, ax_cts, ax_dcd;
extern unsigned axsent, axtmo;
extern unsigned long axschr;
extern unsigned axwpp, axdrop, axmulti;
extern int ax_lastrr0, ax_lastrr1, ax_lastrr2;

ax_stat(fd)
FILE *fd;
{
	fprintf(fd, "PC-SDMA Statistics:\nStatus:");
	if(ax_aborting)
		fprintf(fd, " ABORTING");
	if(ax_insync)
		fprintf(fd, " SYNC'D");
	if(ax_cts)
		fprintf(fd, " CTS");
	if(ax_dcd)
		fprintf(fd, " DCD");
	fprintf(fd, "\nInt stats: %4u ints\n", axint);
	fprintf(fd, "\t%4u specrx %4u extstat %4u rca %4u tbe %4u spur %4u unknown\n",
		axspecrx, axextstat, axrca, axtbe, axspurint, axbadint);
	fprintf(fd, "\t%4u aborts %4u unaborts %4u syncs %4u unsyncs\n",
		axabort, axunabort, axsync, axunsync);
	fprintf(fd, "\t%4u ctson %4u ctsoff %4u dcdon %4u dcdoff\n",
		axctson, axctsoff, axdcdon, axdcdoff);
	fprintf(fd, "\t%2x lastrr0 %2x lastrr1 %2x lastrr2\n",
		ax_lastrr0, ax_lastrr1, ax_lastrr2);
	fprintf(fd, "Tx stats: %4u pkts %4u tmos\n",
		axsent, axtmo);
	fprintf(fd, "Rx stats: %4u rcv\n",
		axrcv);
	fprintf(fd, "\t%4u crc %4u rxovr\n",
		axcrc, axrxovr);
	fprintf(fd, "\t%4u toobig\n",
		axtoobig);
/*	fprintf(fd, "%4u packets rcvd %4u sent %4u dropped %4u refused\n",
					axrcv, axsnd, axdrop, axref);
	fprintf(fd, "%4u PCGW timeouts %4u protocol, %4u req in packet\n",
							axtmo, axprot, axrip);
	fprintf(fd, "%4u ESCs sent\t%4u ESCs received\n", axsesc, axresc);
	fprintf(fd, "%4u SLP packets rcvd %4u reqs sent %4u acks sent\n",
						axslp, axreq, axack);
	fprintf(fd, "ints %4u, condx:  line %4u, rcv %4u, tx %4u, mdm %4u\n",
				axint, axlstat, axrint, axtint, axmstat);
	fprintf(fd, "ints w/out condx:  null %4u, rcv %4u, tx %4u\n",
						axiir, intcomp, badtx);
	fprintf(fd, "%4u timeouts during packet send %4u extra wakeups\n",
							 axptmo, axwpp);
	fprintf(fd, "tbusy is %4u\tnot tx is %4u\n",
							axbusy, axnotx);
*/
	fprintf(fd, "\t%4u timeouts during packet send %4u extra wakeups\n",
		 axtmo, axwpp);
	fprintf(fd, "Multiple packets on net queue %4u\tMax depth = %4u\n",
		axmulti, ax_net->n_inputq->q_max);
	in_stats(fd);
}
