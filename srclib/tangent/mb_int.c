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
#include <sdlc.h>
#include "mb.h"
#include "sccreg.h"

/* This code services the MacBridge interrupt. It is called by an assembly
	language routines which saves all the registers and sets up the
	data segment. */

unsigned mbint = 0;		/* Number of interrupts processed */
unsigned mbspecrx = 0;		/* Number of special rx cond. ints */
unsigned mbextstat = 0;		/* Number of External/Status ints */
unsigned mbrca = 0;		/* Number of receive character avail ints */
unsigned mbtbe = 0;		/* Number of Tx Buffer Empty ints */
unsigned mbbadint = 0;		/* Number of who knows what ints */
unsigned mbcrc = 0;		/* Number of CRC errors */
unsigned mbrxovr = 0;		/* Number of rx overruns */
unsigned mbrcv = 0;		/* Number of packets received correctly */
unsigned mbabort = 0;		/* Number of started abort sequences */
unsigned mbunabort = 0;		/* Number of finished abort sequences */
int mb_aborting = 0;		/* Not currently aborting a frame */
int mb_abortpkt = 0;		/* Flag to abort next packet */
unsigned mbpitch = 0;		/* Number of pkts thrown away due to abort */
unsigned mbsync = 0;		/* Number of (re)gained sync ints */
unsigned mbunsync = 0;		/* Number of lost sync ints */
int mb_insync = 0;		/* Not currently sync'ed */
unsigned mbtoobig = 0;		/* Number of too large packets */
int mb_lastrr0 = 0;
int mb_lastrr1 = 0;
int mb_lastrr2 = 0;
int mbunkext = 0;		/* Last unknown ext int status */
long mblastpkttime = 0;

extern 	long	cticks;

extern PACKET mb_inp;

mb_ihnd() {
	char rcv;
	int len;
	int status, vector;

	mbint++;			/* One more interrupt */
	outb(MB_CHANB_CTL, SCC_WR2);	/* Set ptr to reg 2 (chan B only) */
	vector = inb(MB_CHANB_CTL);	/* Get MODIFIED vector */
	mb_lastrr2 = vector;

	switch (vector & 0x0e) {	/* Mask unwanted bits just in case */
		case 0xe:		/* Special receive condition chan A */
			mbspecrx++;
			outb(MB_CHANA_CTL, SCC_WR1);
			rcv = inb(MB_CHANA_CTL);
			mb_lastrr1 = rcv;

			if(rcv & SCC_CRC_FRAMING) { /* CRC error, abort pkt */
				mbcrc++;
				break;	/* Start new rx */
			}
			if(rcv & SCC_Rx_OVERRUN) { /* Overrun, abort pkt */
				mbrxovr++;
				if(mb_inp == NULL) /* No wonder... */
					mb_inp = getfree();
				break;	/* Start new rx */
			}
			if(rcv & SCC_EOF) {
#ifdef notdef
			    if(mb_abortpkt) {
				mb_abortpkt = 0; /* Don't throw next away */
				mbpitch++;
			    } else { /* Valid pkt!  Add to queue */
#endif
				mblastpkttime = cticks;
				len = LBUF;
				len -= dma_done(custom.c_rcv_dma);
				if(len < SDLC_MINLEN || len > LBUF) {
					mbtoobig++;
					break;
				}

				mbrcv++;

/*					mbweirdness++;*/


				mb_inp->nb_len = len;
				mb_inp->nb_tstamp = cticks;
				q_addt(mb_net->n_inputq, (q_elt)mb_inp);
				tk_wake(MbDemux);
					/* Pre-allocate a new packet */
				mb_inp = getfree();
#ifdef notdef
			    }
#endif
			}
			break;	/* Start new packet */
		case 0xa:	/* External/status change */
			mbextstat++;
			status = inb(MB_CHANA_CTL);
			mb_lastrr0 = status;
			mbunkext = mb_lastrr0;

			if(status & SCC_BREAK) {
				if(!mb_aborting) {
					mbabort++;
					mb_aborting = 1;
					mbunkext = 0;
/*					mb_abortpkt = 1; /* Abort this pkt */
					break;	/* Restart this packet */
				}
			} else {
				if(mb_aborting) {
					mbunabort++;
					mb_aborting = 0;
					mbunkext = 0;
					inb(MB_CHANA_DATA);
					break;	/* Restart this packet */
				}
			}
			if(status & SCC_SYNC) {
				if(mb_insync) {
					mbunsync++;
					mb_insync = 0;
					mbunkext = 0;
				}
			} else {
				if(!mb_insync) {
					mbsync++;
					mb_insync = 1;
					mbunkext = 0;
				}
			}
			/* Fall through */
		case 0xc:	/* Receive character available */
			if(vector == 0xc)
				mbrca++;
		case 0x8:	/* Transmit buffer empty */
			if(vector == 0x8)
				mbtbe++;
		default:
			if(vector != 0xc && vector != 0x8 && vector != 0xa)
				mbbadint++;
			outb(MB_CHANA_CTL, SCC_RST_EXTINT);
			outb(MB_CHANA_CTL, SCC_ERROR_RST);
			outb(MB_CHANA_CTL, SCC_RST_HIUS);
			return;	/* Don't restart packet */
	}

abort:
	mb_rx_setup();
}
