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
#include <timer.h>
#include <dma.h>
#include <int.h>
#include <lapb.h>
#include "ax.h"
#include "mpsccreg.h"

/* Transmit a packet.
*/

extern long cticks;

unsigned axsent = 0;		/* Number of packets sent on link */
unsigned axresend = 0;		/* Number of resends */
unsigned axtmo = 0;		/* Number of timeouts on send */
unsigned long axschr = 0;	/* Number of characters sent */

extern int ax_insync;		/* Receiver sync'ed flag */
extern int ax_cts;		/* Receiver CTS flag */

ax_send(p, prot, len, fhost)
PACKET p;
unsigned prot;
unsigned len;
in_name fhost;
{
	int i, length;
	long time;
	register char *cp;
	struct lapb_hhdr *plapb;
	int status;

	/* Set up the lapb header. Insert the address of
		the destination and the control field in the lapb header
		of the packet. */
#ifdef	DEBUG
	if(NDEBUG & (INFOMSG|NETRACE))
		printf("AX_SEND: p[%u] -> %a.\n", len, fhost);
#endif

	plapb = (struct lapb_hhdr *)p->nb_buff;
	plapb->lapb_address = LAPB_BROADCAST; /* Broadcast for now */

	/* Setup the type field and the addresses in the lapb header. */
	switch(prot) {
	case IP:
		plapb->lapb_type = LAPB_IP;
		break;

	default:
#ifdef	DEBUG
		if(NDEBUG & (INFOMSG|PROTERR|BUGHALT))
			printf("MB_SEND: Unknown prot %u.\n", prot);
#endif
		return 0;
	}

restart:
	length = len + sizeof(struct lapb_hhdr);

	if(!ax_insync) {
		printf("ax_send: Receiver not synced.  Is the cable plugged in?\n");

		/* Get receiver back in sync */
		ax_rx_setup();
	}
	if(!ax_cts) {
		printf("ax_send: No CTS from receiver.  Is the cable plugged in?\n");
		return(0);
	}

	/* We should now be ready to transmit the packet */
	time = cticks;

	cp = p->nb_buff;	/* Set up pointer to data */

	int_off();		/* Need exclusive access to SCC */

	/* Set up for DMA of packet, minus first byte */
	dma_setup(custom.c_tx_dma, cp+1, length-1, DMA_OUTPUT);

	/* Reset any pending tx ints */
	outb(MKAX(AX_CHANA_CTL), MPSCC_RST_TxINTP);

	/* Reset CRC generator */
	outb(MKAX(AX_CHANA_CTL), MPSCC_RST_TxCRCG);

	/* Output first byte */
	outb(MKAX(AX_CHANA_DATA), *cp);

	/* Reset Tx underrun/EOM latch */
	outb(MKAX(AX_CHANA_CTL), MPSCC_RST_TxUNDER);

	int_on();		/* Done */

	/* Wait for Tx underrun/EOM latch to be set */
	while(!(inb(MKAX(AX_CHANA_CTL)) & MPSCC_TxUNDER)) {
/*		axuloop++;		/* Waited again... */
		if(cticks - time > AX_TIMEOUT) { /* Too long? */
			axtmo++;	/* Yep, abort transfer */

			/* Reset Tx underrun/EOM latch */
			outb(MKAX(AX_CHANA_CTL), MPSCC_SENDABORT);

			outb(MKAX(AX_CHANA_CTL), MPSCC_RST_TxUNDER);

			if(NDEBUG & (INFOMSG|PROTERR|NETERR))
				printf("ax_send: EOM tmo\n");

			outb(MKAX(AX_CHANA_CTL), MPSCC_RST_TxINTP);
			outb(MKAX(AX_CHANA_CTL), MPSCC_RST_EXTINT);
			outb(MKAX(AX_CHANA_CTL), MPSCC_END_OF_INT);
			return(0);
		}
	}

	/* Reset TBE int */
	outb(MKAX(AX_CHANA_CTL), MPSCC_RST_TxINTP);
	outb(MKAX(AX_CHANA_CTL), MPSCC_RST_EXTINT);
	outb(MKAX(AX_CHANA_CTL), MPSCC_END_OF_INT);

	dma_reset(custom.c_tx_dma);

	axsent++;

	return (length);
}
