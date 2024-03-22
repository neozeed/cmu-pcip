/*  Copyright 1986 by Carnegie Mellon  */
/*  See permission and disclaimer notice in file "cmu-note.h"  */
#include	<cmu-note.h>
/*  Copyright 1984, 1985 by the Massachusetts Institute of Technology  */
/*  See permission and disclaimer notice in file "notice.h"  */
#include	<notice.h>

/* Copyright (c) 1988 Epilogue Technology Corporation	*/
/*  See permission and disclaimer notice in file "etc-note.h"  */
#include	"etc-note.h"

/* $Revision:   2.0  $		$Date:   29 Oct 1989  $	*/
/*
 * $Log:   C:/KARL/CMUPCIP/SRCLIB/PKT/ET_INT.C_V  $
 *
 *	Rev 2.0 29 Oct 89 by Joe Doupnik, for Packet Driver and IEEE 802.3
 *
 *    Rev 1.0   04 Mar 1988 16:32:40
 * Initial revision.
*/

#include <task.h>
#include <q.h>
#include <netq.h>
#include <net.h>
#include <custom.h>
#include <netbuf.h>
#include <stdio.h>
#include <ether.h>
#include "pkt.h"
#include "et_pkt.h"

#define then

#ifdef	WATCH
#include <match.h>
struct pkt pkts[MAXPKT];
int pproc = 0;
int prcv = 0;
long npackets = 0;
unsigned etminlen = 0;
#endif

extern 	long	cticks;

static PACKET et_inp;

char far *
pkt_rcv_call1(handle, len)
unsigned int handle, len;
{
	et_inp = getfree();
	if(et_inp == NULL) then return (char far *) 0;

	if ((len > LBUF) || (len == 0)) then
		{
		putfree(et_inp);	/* free the buffer now jrd */
		return (char far *) 0;
		}
	return (char far *)(et_inp->nb_buff);
}

/* For the moment, it is assumed that the pkt_rcv_call2 comes very, very */
/* soon after the return of pkt_rcv_call1, in particular, before any	 */
/* other call is make to pkt_rcv_call1.					 */
void
pkt_rcv_call2(handle, len, buff)
	unsigned int handle, len;
	char far *buff;
{
#ifdef WATCH
	register char *data = pkts[prcv].p_data;   /* jrd */
	int i;

	if(((pproc - prcv) & PKTMASK) != 1) {
		pkts[prcv].p_len = len;
		if ((buff[12] & 0xfc) == 0 )  /* leading byte, 60 in 0x6003 */
			{
			buff[13] = 1;	/* assign TYPE of 0x0001 */
			buff[12] = 0;	/* for IEEE 802.3 */
			}
		if (len > MATCH_DATA_LEN) len = MATCH_DATA_LEN;
		for (i = 0; i < len; i++) data[i] = buff[i];
		prcv = (prcv+1) & PKTMASK;
		}
	putfree(et_inp); 		/* free the packet structure */
	return;
#endif

	et_inp->nb_len = len;
	et_inp->nb_tstamp = cticks;
	q_addt(et_net->n_inputq, (q_elt)et_inp);
	tk_wake(EtDemux);
}
