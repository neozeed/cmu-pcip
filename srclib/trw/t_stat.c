/*	Copyright 1986 by Carnegie Mellon  */
/*	See permission and disclaimer notice in file "cmu-note.h"  */
#include	<cmu-note.h>
/*	Copyright 1984 by the Massachusetts Institute of Technology	 */
/*	See permission and disclaimer notice in file "notice.h"	 */
#include	<notice.h>

/* Copyright 1988 TRW Information Networks */
/*	See permission and disclaimer notice in file "trw-note.h"  */
#include	"trw-note.h"

/* $Revision:   1.0  $		$Date:   29 Feb 1988 20:20:40  $	*/

#include <task.h>
#include <q.h>
#include <netq.h>
#include <net.h>
#include <custom.h>
#include <netbuf.h>
#include <stdio.h>
#include "trw.h"

void
trw_stat(fd)
	FILE *fd;
{
#if (!defined(NOSTATS))
scb_t cur_scb;

get_scb(&cur_scb);
fd = stdout;
fprintf(fd, "Ether Stats:\t");

fprintf(fd, "     MAC/Ethernet address: %02x.%02x.%02x.%02x.%02x.%02x\n",
	_etme[0]&0xff, _etme[1]&0xff, _etme[2]&0xff,
	_etme[3]&0xff, _etme[4]&0xff, _etme[5]&0xff);

fprintf(fd, "%4u ints\t%4u pkts rcvd\t%4u pkts sent\n",
		 int_cnt, rcv_cnt, send_cnt);

/* Transmission error statistics */
fprintf(fd, "%4u CD lost\t%4u CTS lost\t%4u underrun\n",
	cd_lost_cnt, cts_lost_cnt, xmit_dma_under_cnt);
fprintf(fd, "%4u deferred\t%4u no SQE\t%4u coll\t%4u >16 coll\n",
	deferred_cnt, sqe_lost_cnt, collision_cnt, ex_coll_cnt);

/* Receive error statistics */
/* Note: Many of the receive counters are maintained by the 82586 */
fprintf(fd, "%4u CRC err\t%4u align err\t%4u missed\t%4u overrun\n",
		cur_scb.scb_crcerrs, cur_scb.scb_alnerrs,
		cur_scb.scb_rscerrs, cur_scb.scb_ovrnerrs);

fprintf(fd, "%4u shorts\t%4u longs\t%4u no EOF\n",
		short_cnt, long_cnt, eof_missing_cnt);

/* Driver internal statistics */
fprintf(fd, "%4u toobig\t%4u refused\t%4u dropped\t%4u skipped\t%4u multi\n",
		toobig_cnt, refused_cnt, etdrop, skipped_cnt, etmulti);

fprintf(fd, "max q depth %u\n", et_net->n_inputq->q_max);
#endif
etadstat(fd);		/* upcall to address resolution protocol */
#if (!defined(WATCH))
in_stats(fd);		/* upcall to internet */
#endif
}

#if defined(DEBUG)
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
