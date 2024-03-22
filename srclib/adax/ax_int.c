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
#include <lapb.h>
#include "ax.h"
#include "mpsccreg.h"

/* This code services the PC-SDMA interrupt. It is called by an assembly
	language routines which saves all the registers and sets up the
	data segment. */

unsigned axint = 0;		/* Number of interrupts processed */
unsigned axspurint = 0;		/* Number of spurious ints */
unsigned axspecrx = 0;		/* Number of special rx cond. ints */
unsigned axextstat = 0;		/* Number of External/Status ints */
unsigned axrca = 0;		/* Number of receive character avail ints */
unsigned axtbe = 0;		/* Number of Tx Buffer Empty ints */
unsigned axbadint = 0;		/* Number of who knows what ints */
unsigned axrcv = 0;		/* Number of packets received correctly */
unsigned axtoobig = 0;		/* Number of too large packets */
unsigned axcrc = 0;		/* Number of CRC errors */
unsigned axrxovr = 0;		/* Number of rx overruns */

unsigned axabort = 0;		/* Number of started abort sequences */
unsigned axunabort = 0;		/* Number of finished abort sequences */
int ax_aborting = 0;		/* Not currently aborting a frame */

unsigned axsync = 0;		/* Number of (re)gained sync ints */
unsigned axunsync = 0;		/* Number of lost sync ints */
int ax_insync = 0;		/* Not currently sync'ed */

unsigned axctson = 0;		/* Number of CTS on ints */
unsigned axctsoff = 0;		/* Number of CTS off ints */
int ax_cts = 0;			/* CTS currently off */

unsigned axdcdon = 0;		/* Number of DCD on ints */
unsigned axdcdoff = 0;		/* Number of DCD off ints */
int ax_dcd = 0;			/* DCD currently off */

int ax_lastrr0 = 0;
int ax_lastrr1 = 0;
int ax_lastrr2 = 0;
long axlastpkttime = 0;

extern 	long	cticks;

extern PACKET ax_inp;

ax_ihnd() {
	int len;

	axint++;			/* One more interrupt */
	outb(MKAX(AX_CHANB_CTL), MPSCC_CR2); /* Set ptr to reg 2 (chan B only) */
	ax_lastrr2 = inb(MKAX(AX_CHANB_CTL)); /* Get MODIFIED vector */
	ax_lastrr0 = inb(MKAX(AX_CHANA_CTL)); /* Get External Status Status */
	outb(MKAX(AX_CHANA_CTL), MPSCC_CR1); /* Set ptr to reg 1 */
	ax_lastrr1 = inb(MKAX(AX_CHANA_CTL)); /* Get Special Receive Status */

	if(!(ax_lastrr0 & MPSCC_INT_PENDING)) {
		axspurint++;		/* Spurious interrupt? */
		return;
	}

	switch (ax_lastrr2 & 0x07) {	/* Mask unwanted bits just in case */
		case MPSCC_CHNA_SRxIP:	/* Special receive condition chan A */
			axspecrx++;

			if(ax_lastrr1 & MPSCC_EOF) {
				axlastpkttime = cticks;
				len = LBUF;
				len -= dma_done(custom.c_rcv_dma);
				if(len < LAPB_MINLEN || len > LBUF) {
					axtoobig++;
					goto abort;
				}

				axrcv++;

				if(ax_inp) {
					ax_inp->nb_len = len;
					ax_inp->nb_tstamp = cticks;
					q_addt(ax_net->n_inputq,
						(q_elt)ax_inp);
					ax_inp = NULL;
					tk_wake(AxDemux);
				}
			}
			if(ax_lastrr1 & MPSCC_CRC_FRAMING) {
				/* CRC error, abort pkt */
				axcrc++;
			}
			if(ax_lastrr1 & MPSCC_Rx_OVERRUN) {
				/* Overrun, abort pkt */
				axrxovr++;
			}
abort:
			ax_rx_setup();
			break;	/* Start new packet */

		case MPSCC_CHNA_ESIP:	/* External/status change */
			axextstat++;

			/* Check for change in Break/Abort status */
			if(ax_lastrr0 & MPSCC_BREAK) {
				if(!ax_aborting) {
					axabort++;
					ax_aborting = 1;
				}
			} else {
				if(ax_aborting) {
					axunabort++;
					ax_aborting = 0;
				}
			}

			/* Check for change in Sync status */
			if(ax_lastrr0 & MPSCC_SYNC) {
				if(ax_insync) {
					axunsync++;
					ax_insync = 0;
				}
			} else {
				if(!ax_insync) {
					axsync++;
					ax_insync = 1;
				}
			}

			/* Check for change in CTS status */
			if(ax_lastrr0 & MPSCC_CTS) {
				if(!ax_cts) {
					axctson++;
					ax_cts = 1;
				}
			} else {
				if(ax_cts) {
					axctsoff++;
					ax_cts = 0;
				}
			}

			/* Check for change in DCD status */
			if(ax_lastrr0 & MPSCC_DCD) {
				if(!ax_dcd) {
					axdcdon++;
					ax_dcd = 1;
				}
			} else {
				if(ax_dcd) {
					axdcdoff++;
					ax_dcd = 0;
				}
			}

			/* Check for Tx Buffer Empty Condition */
			if(ax_lastrr0 & MPSCC_TxBUF_EMPTY) {
				axtbe++;
				outb(MKAX(AX_CHANA_CTL), MPSCC_RST_TxINTP);
			}

			/* Check for All Sent Condition */
			if(ax_lastrr1 & MPSCC_ALL_SENT) {
				axtbe++;
				outb(MKAX(AX_CHANA_CTL), MPSCC_RST_TxINTP);
			}

			outb(MKAX(AX_CHANA_CTL), MPSCC_RST_EXTINT);
			break;

		case MPSCC_CHNA_RxIP:	/* Receive character available */
			axrca++;
			outb(MKAX(AX_CHANA_CTL), MPSCC_RST_EXTINT);
			break;

		case MPSCC_CHNA_TxIP:	/* Transmit buffer empty */
			axtbe++;
			outb(MKAX(AX_CHANA_CTL), MPSCC_RST_TxINTP);
			outb(MKAX(AX_CHANA_CTL), MPSCC_RST_EXTINT);
			break;

		default:
			axbadint++;
			break;
	}

	outb(MKAX(AX_CHANA_CTL), MPSCC_END_OF_INT);
	return;	/* Don't restart packet */
}
