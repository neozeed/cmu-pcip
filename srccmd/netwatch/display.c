/*  Copyright 1986 by Carnegie Mellon  */
/*  See permission and disclaimer notice in file "cmu-note.h"  */
#include	<cmu-note.h>
/*  Copyright 1984 by the Massachusetts Institute of Technology  */
/*  See permission and disclaimer notice in file "notice.h"  */
#include	<notice.h>

/* 7/10/84 - moved some variables definitions into et_int.c.
					<John Romkey>
   7/16/84 - changed debugging level on short packet message to only
	INFOMSG.			<John Romkey>
   10/11/84 - moved display code from et_demux.t.c to display.c.
					<John Romkey>
   7/1/86 - Add packet logging facility.  Packets can be examined with
	John Leong's WATCH program.
					<Drew D. Perkins>
*/

#include <stdio.h>
#include <types.h>
#include <task.h>
#include <q.h>
#include <netq.h>
#include <net.h>
#include <custom.h>
#include <netbuf.h>
#include <icmp.h>
#include <ip.h>
#include <ether.h>

#include <match.h>
#include <attrib.h>
#include "watch.h"

extern int pproc;
extern int prcv;
extern unsigned long npackets;

extern char *header;
extern unsigned etwpp, etdrop, etmulti;

extern FILE *logfile;		/* DDP File to log packets to */

int disp_y = 1;

#ifdef LENGTH_HISTO
/* histogram stuff */
unsigned long hist_counts[HIST_NUM_INCS];
unsigned long hist_too_big = 0;
#endif

pkt_display() {
	unsigned type;
	register char *data;
	int i;
	char buffer[200];
	int len;

	while(1) {
	tk_yield();
	if(prcv == pproc) continue;
	
	data = pkts[pproc].p_data;

	if(!pt_match(&pat1, data) && !pt_match(&pat2, data))
		goto check;

	/* accepted, add to histogram */
	len = pkts[pproc].p_len;

#ifdef LENGTH_HISTO
	if(len > HIST_MAX_LEN)
		hist_too_big++;
	else hist_counts[len>>HIST_SHIFT]++;
#endif

/* DDP/CCK - Begin: Write packet to log file */
	if(logfile != NULL) {
		if ((fwrite(data, 62, 1, logfile) != 1) ||
		    (fwrite(&len, 2, 1, logfile) != 1)) {
		    /* some error */
		    fclose(logfile);
		    logfile = NULL;
		    clr25();
		    pr25(0, "Error writing to logfile, logging terminated");
		}
	}
/* DDP/CCK - End */

	if(!(*root_layer->l_parse)(data, disp_y, 0, len))
		goto check;

	if(disp_y++ == 23) disp_y = 1;

	clear_lines(disp_y, 1);

	npackets++;

/*	sprintf(buffer, "%8U packets", npackets);
	wr_string(buffer, 24, 50, INVERT);
*/

check:
	pproc = (pproc+1)&PKTMASK;
	}
}
