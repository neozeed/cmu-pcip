/*  Copyright 1986 by Carnegie Mellon  */
/*  See permission and disclaimer notice in file "cmu-note.h"  */
#include	<cmu-note.h>
/*  Copyright 1985 by Micom-Interlan Corp. */
/*  See permission and disclaimer notice in file "interlan-notice.h"  */
#include	"il-notice.h"

#include <task.h>
#include <q.h>
#include <netq.h>
#include <net.h>
#include <custom.h>
#include <netbuf.h>
#include <stdio.h>
#include <ether.h>
#include <int.h>
#include <dma.h>
#include "interlan.h"
#include <match.h>

/* NI5010 interrupt handler for netwatch. Copies incoming packets into
	a circular buffer instead of into a PACKET.
*/

extern 	long	cticks;

struct pkt pkts[MAXPKT];

int pproc = 0;
int prcv = 0;
long npackets = 0;

/* This code services the ethernet interrupt. It is called by an assembly
	language routines which saves all the registers and sets up the
	data segment.
*/

unsigned il_rcv_int = 0;
unsigned il_spur_int = 0;
unsigned il_xmit_int = 0;
unsigned il_dma_int = 0;
unsigned il_bad_rcv = 0;
unsigned il_rst_pkt = 0;
unsigned il_runt = 0;
unsigned il_align = 0;
unsigned il_crc = 0;
unsigned il_overflow = 0;
unsigned il_delayed = 0;
unsigned il_too_big = 0;
unsigned etdrop = 0;
extern unsigned ettmo, il_tx_coll;

long etlastpkttime = 0;

/* transmitter and receiver semaphores */
/*unsigned il_transmitting= FALSE;	*/
unsigned il_received = FALSE;
unsigned il_dma_done = FALSE;
unsigned il_tx_done = FALSE;

extern unsigned etrcvcmd;

#define	DMMRST1		0x05
#define	DMMSET1		0x01
#define	DMWRITE1	0x45

/* interrupt handler; only has to deal with receiver interrupts */
extern int il_eoi;
extern NET *et_net;
extern task *EtDemux;

il_ihnd() {
	char intstat;
	char rcvstat;

	/* check if it was a receiver interrupt. whatever could it have
		been if it wasn't?
	*/
	intstat = inb(INT_STAT);

	/* no xmitter interrupts but check */
	if(!(intstat & IN_TINT)) {
		outb(M_MODE, 0);
		outb(XMIT_MASK, 0);
		outb(CLR_XMIT_INT, 0xff);
		il_tx_done = TRUE;
		il_xmit_int++;
		}

	if(!(intstat & IN_DMAINT)) {
		il_dma_done = TRUE;
		outb(RESET_DMA, 0);
		il_dma_int++;
		}

	if(!(intstat & IN_RINT)) {

	/* if it was a good packet, copy it in
	*/
	rcvstat = inb(RCV_STAT);
	if((rcvstat & 0x9f) == RS_PKT_OK) {
		il_rcv_int++;

		/* clear the interrupt */
		outb(CLR_RCV_INT, 0xff);

		il_rcv_pkt();
		}
	else {
		if(rcvstat & RS_RST_PKT)
			il_rst_pkt++;
		if(rcvstat & RS_RUNT)
			il_runt++;
		if(rcvstat & RS_ALIGN)
			il_align++;
		if(rcvstat & RS_CRC)
			il_crc++;
		if(rcvstat & RS_OVERFLOW)
			il_overflow++;

		il_bad_rcv++;
		}

	il_rcvr_reset();
	}
	}

il_stat(fd)
	FILE *fd; {
	fprintf(fd, "interlan stats\n");
	fprintf(fd, "My ethernet address: %02x.%02x.%02x.%02x.%02x.%02x\n", /* DDP */
		_etme[0]&0xff, _etme[1]&0xff, _etme[2]&0xff,
		_etme[3]&0xff, _etme[4]&0xff, _etme[5]&0xff); /* DDP */
	fprintf(fd, "%d rcv ints, %d spurious ints\n", il_rcv_int, il_spur_int);
	fprintf(fd, "%d xmit ints, %d dma ints\n", il_xmit_int, il_dma_int);
	fprintf(fd, "%d rsts, %d runts, %d aligns, %d crcs, %d overflows\n",
			il_rst_pkt, il_runt, il_align, il_crc,
			il_overflow);
	fprintf(fd, "%d bad rcvs, %u delayed rcvs, %u too bigs\n",
					il_bad_rcv, il_delayed, il_too_big);
	fprintf(fd, "%d transmit timeouts, %d transmit collisions\n",
					ettmo, il_tx_coll);
	fprintf(fd, "rcv stat %02x\n", inb(RCV_STAT)&0xff);
	etadstat(fd);
	in_stats(fd);
	}

ip_ether_send() {
	printf("ip ether stub called\n");
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

il_rcv_pkt() {
	PACKET p;
	unsigned len;
	register char *data = pkts[prcv].p_data;
	register int i;

	len = inw(RCV_CNT_LO);

	etlastpkttime = cticks;
	outw(M_MODE, MM_MUX);
	outw(M_START_LO, 0);

	if(((pproc - prcv) & PKTMASK) != 1) {
		pkts[prcv].p_len = len;

		/* ought to write a fast assembly hack to do this */
		fastin(RCV_BUF, data, MATCH_DATA_LEN);

		prcv = (prcv+1)&PKTMASK;
		tk_wake(EtDemux);
		}
	}

/* reset the receiver */
il_rcvr_reset() {
	outw(M_START_LO, 0);
	outb(CLR_RCV_INT, 0xff);	/* clear all conditions */
	outb(M_MODE, 0);
	outb(M_MODE, MM_EN_RCV);
	outb(RCV_MASK, 0xff);
	}
