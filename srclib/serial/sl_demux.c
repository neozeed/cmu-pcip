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
#include "sl.h"

/* Statistics counters */
extern unsigned sldrop;		/* number of dropped packets */
unsigned slprot = 0;		/* number of packets with bad protocol */
unsigned slslp = 0;		/* number of slp packets received */
unsigned slmulti = 0;		/* # of times more than 1 packet on queue */
unsigned slwpp = 0;		/* task awakened w/o packet to process */

/* This file contains the code for the net-level packet demultiplexor for the
	serial line code. It demultiplexes the packet according to its protocol
	(ie: IP, AddrReply, Chaos, PUP, ...). It also does all the validation 
	on the packet that can be done from this level.	*/

sl_demux() {
	PACKET p;
	char *data;

#ifdef	DEBUG
	if(NDEBUG & INFOMSG)
		printf("SL_DEMUX activated.\n");

	if(NDEBUG & INFOMSG)
		printf("SL_DEMUX running.\n");
#endif

	tk_block();

	while(1) {
	/* There should be a packet waiting on our queue for us. Get it. */
	p = (PACKET)aq_deq(sl_net->n_inputq);

	if(p == 0) {
#ifdef	DEBUG
		if(NDEBUG & (NETERR|INFOMSG))
			printf("SLDEMUX: no pkt to process\n");
#endif
		slwpp++;
		tk_block();
		continue;
		}

	if(p->nb_len < 4) {
#ifdef	DEBUG
		if(NDEBUG & (NETERR|PROTERR|INFOMSG))
			printf("SL: pkt too short %u\n", p->nb_len);
#endif
		putfree(p);
		tk_block();
		continue;
		}

	data = p->nb_buff;

	if(data[0] != 2) {
#ifdef	DEBUG
		if(NDEBUG & (NETERR|PROTERR|INFOMSG))
		  {
			printf("SL: pkt hdr bad: %02x\n", data[0]);
			if(NDEBUG & DUMP) sl_dump(p);
		  }
#endif
		slprot++;
		sldrop++;
		putfree(p);
		tk_block();
		continue;
		}

	p->nb_prot = p->nb_buff+4;

	switch(data[1]) {
	case 1:
#ifdef	DEBUG
		if(NDEBUG & NETRACE)
			printf("SL: IP pkt\n");
#endif
		indemux(p, p->nb_len-4, sl_net);
		break;
	case 3:
#ifdef	DEBUG
		if(NDEBUG & NETRACE)
			printf("SL: SLP pkt\n");
#endif
		slslp++;
		sl_slp(p, p->nb_len-4, sl_net);
		break;

	default:
#ifdef	DEBUG
		if(NDEBUG & (NETERR|PROTERR|INFOMSG))
		  {
		      printf("SL: bad protocol %u\n", data[1]);
		      if(NDEBUG & DUMP) sl_dump(p);
		  }
#endif
		sldrop++;
		putfree(p);
		}

/* NOTE: if we continue after having an error, we may not process
	other packets sitting on our queue!
*/
	if(sl_net->n_inputq->q_head) {
#ifdef	DEBUG
		if(NDEBUG & INFOMSG)
			printf("SLDEMUX: More packets; waking self.\n");
#endif
		slmulti++;
		tk_wake(tk_cur);
		}
	tk_block();
	}
}


sl_dump(p)
	PACKET p; {
	char *data;
	int i;

	data = p->nb_buff;
	for(i=1;i<121; i++) {
		printf("%02x ", *data++);
		if(i%20 == 0) printf("\n");
		}
	printf("\n");
	}
