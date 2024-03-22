/*  Copyright 1986 by Carnegie Mellon  */
/*  See permission and disclaimer notice in file "cmu-note.h"  */
#include	<cmu-note.h>
/*  Copyright 1984, 1985 by the Massachusetts Institute of Technology  */
/*  See permission and disclaimer notice in file "notice.h"  */
#include	<notice.h>


/*  Modifed 1/16/84 to timestamp incoming packets.  <J. H. Saltzer>
	1/25/84 - added code to support netwatch. <John Romkey>
	7/6/84 -  changed code to handle runt packets and not go deaf
		to the net; moved some variable declarations from et_demux
		to here.			<John Romkey>
	7/9/84 - changed driver to handle runt packet condition and
		reset the controller properly. Involved switching the packet
		buffer to the bus and then back to receive to get the
		receiver going again. Driver should no longer go deaf
		to the net.			<John Romkey>
*/

#include <task.h>
#include <q.h>
#include <netq.h>
#include <net.h>
#include <custom.h>
#include <netbuf.h>
#include <stdio.h>
#include <ether.h>
#include "3com.h"

#ifdef	WATCH
#include <match.h>
#endif

extern 	long	cticks;

/* This code services the ethernet interrupt. It is called by an assembly
	language routines which saves all the registers and sets up the
	data segment. */

unsigned etint = 0;
unsigned etfcs = 0;
unsigned etover = 0;
unsigned etdribble = 0;
unsigned etshort = 0;
unsigned etrcv = 0;
unsigned etdmadone = 0;
unsigned etrcvdma = 0;
unsigned etref = 0;
unsigned ettoobig = 0;

#ifdef	WATCH
struct pkt pkts[MAXPKT];

int pproc = 0;
int prcv = 0;
long npackets = 0;

#endif

extern unsigned etrcvcmd;

et_ihnd() {
	char rcv;
	char orcv;
	unsigned len;


	while(!((rcv = inb(ERCVSTAT)) & ESTALESTAT)) {
		etint++;
		if(rcv & EOVERFLOW) {
			etover++;
			goto rcv_fixup;
			}
		else if(rcv & EDRIBBLEERR) etdribble++;
		else if(rcv & EFCSERR) etfcs++;
		else if(rcv & ESHORTFRAME) {
			etshort++;
	rcv_fixup:	outb(EAUXCMD, EINTDMAENABLE|ESYSBUS);
			outb(EAUXCMD, EINTDMAENABLE|ERECEIVE);
			outb(ECLRRP, 0);
			}
		else if(rcv & EGOODPACKET) {
#ifdef	WATCH
			register char *data = pkts[prcv].p_data;
			int i;

			etrcv++;
			outw(EGPPLOW, 0);
			outb(EAUXCMD, EINTDMAENABLE|ESYSBUS);
			if(((pproc - prcv) & PKTMASK) != 1) {
				pkts[prcv].p_len = inw(ERBPLOW);

				for(i=0; i<MATCH_DATA_LEN; i++)
					*data++ = inb(EBUFWIN);

				prcv = (prcv+1)&PKTMASK;
				tk_wake(EtDemux);
				}

			outb(EAUXCMD, EINTDMAENABLE|ERECEIVE);
			outb(ERCVCMD, etrcvcmd);
			outb(ECLRRP, 0);
#endif

#ifndef	WATCH
			PACKET et_inp;

			etrcv++;

			outb(EAUXCMD, EINTDMAENABLE|ESYSBUS);

			len = inw(ERBPLOW);
			if(len > LBUF || len == 0) {
				ettoobig++;
		punt_rcv:	outb(EAUXCMD, EINTDMAENABLE|ERECEIVE);
				outb(ERCVCMD, etrcvcmd);
				outb(ECLRRP, 0);
				continue;
				}

			et_inp = getfree();
			if(et_inp == NULL) {
				etref++;
				goto punt_rcv;
				}

			outw(EGPPLOW, 0);

			/* if receive dma channel is 0, don't dma
			 */
			if(custom.c_rcv_dma == 0) {
				fastin(EBUFWIN, et_inp->nb_buff, len);
				et_inp->nb_len = len;
				et_inp->nb_tstamp = cticks;
				q_addt(et_net->n_inputq, (q_elt)et_inp);
				tk_wake(EtDemux);
				goto complete_rcv;
				}

			dma_setup(custom.c_rcv_dma, et_inp->nb_buff, len, DMA_INPUT);

			outw(EGPPLOW, 0);

			outb(EAUXCMD, EINTDMAENABLE|EDMAREQ|ESYSBUS);

			/* keep house while we're waiting for dma to
				complete
			*/
			etdmadone++;
			etrcvdma++;

			et_inp->nb_len = len;
			et_inp->nb_tstamp = cticks;
			q_addt(et_net->n_inputq, (q_elt)et_inp);
			tk_wake(EtDemux);

			while(!(inb(EAUXSTAT) & EDMADONE)) ;

			/* DDP - Clear DMA request. */
			outb(EAUXCMD, EINTDMAENABLE|ESYSBUS);

			dma_reset(custom.c_rcv_dma);

complete_rcv:
			outb(ECLRRP, 0);
			outb(EAUXCMD, EINTDMAENABLE|ERECEIVE);
#endif
			}
		}
	}
