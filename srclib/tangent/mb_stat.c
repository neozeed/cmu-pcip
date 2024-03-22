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
#include "mb.h"

/* print some pretty statistics */

extern unsigned mbsent, mbresend, mbtmo, mbswait, mbint, mbspecrx, mbcrc, mbrxovr;
extern unsigned mbrcv, mbpitch, mbabort, mbunabort, mbsync, mbunsync, mbtoobig;
extern unsigned long mbschr, mbsloop, mbuloop;
extern unsigned mbwpp, mbdrop, mbmulti, mbextstat, mbrca, mbtbe, mbbadint;
extern int mb_lastrr0, mb_lastrr1, mb_lastrr2, mbunkext;

/*extern unsigned mbsnd, mbsesc, mbtmo, mbrcv, mbdrop, mbresc, mbrip, mbprot;
extern unsigned mbmbp, mbmulti, badtx, mbref, mbwpp;
extern unsigned mbreq, mback, mbint, mbtint, mbrint;
extern unsigned	mblstat,mbmstat,intcomp,mbiir;
extern unsigned mbnotx, mbbusy, mbptmo;
*/

mb_stat(fd)
	FILE *fd; {

	fprintf(fd, "MacBridge Statistics:\n");
	fprintf(fd, "Tx stats: %4u pkts\n", mbsent);
	fprintf(fd, "\t%4u tmos %4u resends %8D chars sent\n",
		mbtmo, mbresend, mbschr);
	fprintf(fd, "\t%4u waits %8D sloops %8D uloops\n",
		mbswait, mbsloop, mbuloop);
	fprintf(fd, "Rx stats: %4u rcv\n",
		mbrcv);
	fprintf(fd, "\t%4u crc %4u rxovr %4u pitched\n",
		mbcrc, mbrxovr, mbpitch);
	fprintf(fd, "\t%4u toobigs %4u dropped\n",
		mbtoobig, mbdrop);
	fprintf(fd, "Int stats: %4u ints\n", mbint);
	fprintf(fd, "\t%4u specrx %4u extstat %4u rca %4u tbe %4u unknown\n",
		mbspecrx, mbextstat, mbrca, mbtbe, mbbadint);
	fprintf(fd, "\t%4u aborts %4u unaborts %4u syncs %4u unsyncs\n",
		mbabort, mbunabort, mbsync, mbunsync);
	fprintf(fd, "\t%2x lastrr0 %2x lastrr1 %2x lastrr2 %2x unkext\n",
		mb_lastrr0, mb_lastrr1, mb_lastrr2, mbunkext);
/*	fprintf(fd, "%4u packets rcvd %4u sent %4u dropped %4u refused\n",
					mbrcv, mbsnd, mbdrop, mbref);
	fprintf(fd, "%4u PCGW timeouts %4u protocol, %4u req in packet\n",
							mbtmo, mbprot, mbrip);
	fprintf(fd, "%4u ESCs sent\t%4u ESCs received\n", mbsesc, mbresc);
	fprintf(fd, "%4u SLP packets rcvd %4u reqs sent %4u acks sent\n",
						mbslp, mbreq, mback);
	fprintf(fd, "ints %4u, condx:  line %4u, rcv %4u, tx %4u, mdm %4u\n",
				mbint, mblstat, mbrint, mbtint, mbmstat);
	fprintf(fd, "ints w/out condx:  null %4u, rcv %4u, tx %4u\n",
						mbiir, intcomp, badtx);
	fprintf(fd, "%4u timeouts during packet send %4u extra wakeups\n",
							 mbptmo, mbwpp);
	fprintf(fd, "tbusy is %4u\tnot tx is %4u\n",
							mbbusy, mbnotx);
*/
	fprintf(fd, "\t%4u timeouts during packet send %4u extra wakeups\n",
							 mbtmo, mbwpp);
	fprintf(fd, "Multiple packets on net queue %4u\tMax depth = %4u\n",
					mbmulti, mb_net->n_inputq->q_max);
	in_stats(fd);
}
