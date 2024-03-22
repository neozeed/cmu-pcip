/*  Copyright 1986 by Carnegie Mellon  */
/*  See permission and disclaimer notice in file "cmu-note.h"  */
#include	<cmu-note.h>
/*  Copyright 1984 by the Massachusetts Institute of Technology  */
/*  See permission and disclaimer notice in file "notice.h"  */
#include	<notice.h>

/* Copyright (c) 1988 Epilogue Technology Corporation	*/
/*  See permission and disclaimer notice in file "etc-note.h"  */
#include	"etc-note.h"

/* $Revision:   1.1  $		$Date:   07 Mar 1988 12:43:34  $	*/
/*
 * $Log:   C:/karl/cmupcip/srclib/pkt/et_stat.c_v  $
 * 
 *    Rev 1.1   07 Mar 1988 12:43:34
 * Added new statistics fields
 * 
 *    Rev 1.0   04 Mar 1988 16:32:44
 * Initial revision.
*/

#include <task.h>
#include <q.h>
#include <netq.h>
#include <net.h>
#include <custom.h>
#include <netbuf.h>
#include <stdio.h>
#include "pkt.h"
#include "et_pkt.h"

et_stat(fd)
	FILE *fd;
{
pkt_driver_statistics_t pkt_stats;

#ifndef NOSTATS
fd = stdout;
fprintf(fd, "Ether Stats:\n");
fprintf(fd, "My ethernet address: %02x.%02x.%02x.%02x.%02x.%02x\n", /* DDP */
	_etme[0]&0xff, _etme[1]&0xff, _etme[2]&0xff,
	_etme[3]&0xff, _etme[4]&0xff, _etme[5]&0xff); /* DDP */

(void) pkt_get_statistics(ip_handle, &pkt_stats, sizeof(pkt_stats));

/* Because C doesn't support structure subtraction we will fake it...	*/
/* We are doing: pkt_stats -= start_pkt_stats;				*/
  {
  int i;
  long *l1, *l2;
  for (l1 = (long *)&pkt_stats, l2 = (long *)&start_pkt_stats, i = 0;
       i < (sizeof(pkt_stats)>>2); l1++, l2++, i++)
        *l1 -= *l2;	/* Subtract original value from the current value */
   }

fprintf(fd, "%8U pkts in\t%8U bytes in\n",
	(long)pkt_stats.pkts_in, (long)pkt_stats.bytes_in);

fprintf(fd, "%8U pkts out\t%8U bytes out\n",
	(long)pkt_stats.pkts_out, (long)pkt_stats.bytes_out);
	
fprintf(fd, "%8U errs in\t%8U errs out\t%8U pkts dropped\n",
	(long)pkt_stats.errors_in, (long)pkt_stats.errors_out,
	(long)pkt_stats.packets_dropped);

if (drvr_info.pdtype == TRW_PC2000)
   {
   fprintf(fd, "%8U ints\t%8U no CD\t%8U no CTS\t%8U XMIT underrun\n",
	   (long)pkt_stats.ints, (long)pkt_stats.cd_lost,
	   (long)pkt_stats.cts_lost, (long)pkt_stats.xmit_dma_under);

   fprintf(fd, "%8U defers\t%8U no SQE\t%8U cols\t%8U ex cols\n",
	   (long)pkt_stats.deferred, (long)pkt_stats.sqe_lost,
	   (long)pkt_stats.collisions, (long)pkt_stats.ex_collisions);

   fprintf(fd, "%8U toobig\t%8U refuse\t\n",
	   (long)pkt_stats.toobig, (long)pkt_stats.refused);

   fprintf(fd, "%8U shorts\t%8U longs\t%8U skips\t%8U missed\n",
	   (long)pkt_stats.shorts, (long)pkt_stats.longs,
	   (long)pkt_stats.skipped, (long)pkt_stats.rscerrs);

   fprintf(fd, "%8U bad crc\t%8U align errs\t%8U RCV overruns\n",
	   (long)pkt_stats.crcerrs, (long)pkt_stats.alnerrs,
	   (long)pkt_stats.ovrnerrs);

   printf("%8U unwanted\t%8U no user buffer\n",
	   (long)pkt_stats.unwanted, (long)pkt_stats.user_drops);
   }

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
