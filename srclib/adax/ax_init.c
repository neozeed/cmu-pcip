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

/* This file contains the initialization code for the ADAX Inc.
PC-SDMA card for the PC.  AX_INIT() has to initialize the serial port,
perhaps fill in some of its net struct and fork the serial line packet to
protocol demultiplexor.
 */

NET *ax_net;		/* Serial line net descriptor */
task *AxDemux;		/* The packet-to-protocol demultiplexor. */
char save_mask;		/* receiver command on entry. */
unsigned ax_eoi;
PACKET ax_inp;		/* Current input packet */
lapb_cb ax_lc;		/* LAPB Connection Block */

extern int ax_demux();	/* the routine which is the body of the demux task */
extern int ax_aborting;	/* Aborting state */
extern int ax_insync;	/* Sync state */
extern int ax_cts;	/* CTS state */
extern int ax_dcd;	/* DCD state */


ax_init(net, options, dummy)
	NET *net;
	unsigned options;
	unsigned dummy; {

#ifdef	DEBUG
	if(NDEBUG & INFOMSG)
		printf("Initializing PC-SDMA.\n");
#endif

	/* patch in a bogus gateway address so the internet router
		doesn't complain.
	*/
	net->n_defgw = 0x01020304;

#ifdef	DEBUG
	if(NDEBUG & INFOMSG)
		printf("Forking AXDEMUX.\n");
#endif

	AxDemux = tk_fork(tk_cur, ax_demux, net->n_stksiz, "AXDEMUX", net);
	if(AxDemux == NULL) {
		printf("Can't fork AxDemux task.\n");
		exit(1);
	}
	
	ax_net = net;
	ax_net->n_demux = AxDemux;

	ax_inp = getfree();	/* Preallocate the first packet */
	if(ax_inp == NULL) {
		printf("AX_INIT: Can't pre-allocate input packet.\n");
		exit(1);
	}

	ax_switch(1, options);

	tk_yield();	/* Give the per net task a chance to run. */

	return;
}


char ax_on_tbl[] = {
	/* DMA mode/Nonvectored Interrupts/Rx INT mask */
	MPSCC_CR2,
	    MPSCC_DMA_MODE3|MPSCC_DMA_PRI1|MPSCC_IASR_MODE2|MPSCC_RxINT_MASK|MPSCC_ENA_SYNCB,
	/* sdlc mode/x1 clk/sync mode/no parity (must be done first) */
	MPSCC_CR4,
	    MPSCC_CLK_x1|MPSCC_SYNC_SDLC|MPSCC_SYNC_ENA,
#define LINK_ADDRESS	0	/* Doesn't matter at this point */
	/* SDLC link address */
	MPSCC_CR6,
	    LINK_ADDRESS,
	/* SDLC flag (is this already fixed?) */
	MPSCC_CR7,
	    0x7e,
	/* enable dma and interrupts */
	MPSCC_CR1,
	    MPSCC_ENA_FRSTCHR|MPSCC_ENA_TxINT|MPSCC_ENA_EXTINT,
	/* DTR/Tx 8 bits/SDLC poly/Tx enable/RTS/Tx CRC enable (initialize transmitter) */
	MPSCC_CR5,
	    MPSCC_DTR|MPSCC_Tx_8BITS|MPSCC_POLY_SDLC|MPSCC_Tx_ENA|MPSCC_RTS|MPSCC_TxCRC_ENA,
	/* Rx 8 bits/Enter hunt mode/Enable CRC Gen (initialize receiver) */
	MPSCC_CR3,
	    MPSCC_Rx_8BITS|MPSCC_ENTER_HUNT|MPSCC_RxCRC_ENA|MPSCC_Rx_ENA,
};

/*
 * Routine to switch the board and interrupts on/off.
 */
ax_switch(state, options)
int state;
unsigned options;
{
	int vec, i;
	char *m;

    if(state) {			/* Turn them on? */
	int_off();		/* Disable interrupts. */

	/* patch in the new interrupt handler - rather, call the routine to
		do this. This routine saves the old contents of the vector.
	*/
	ax_eoi = 0x60 + custom.c_intvec;
	ax_patch(custom.c_intvec<<2);

	/* setup interrupts for the specified line */
	vec = (1 << custom.c_intvec);
	save_mask = inb(IIMR) & vec;
	outb(IIMR, inb(IIMR) & ~vec);

	/* Reset the PC-SDMA MPSCC chip */
	inb(MKAX(AX_CHANA_CTL)); /* Make sure register pointer = 0 */
	outb(MKAX(AX_CHANA_CTL), MPSCC_RST_CHANNEL);

	/* Waste a little time */
	for(i = 100; i; i++)
		;

	/* Set up interrupt vector and Condition Affects Vector */
	outb(MKAX(AX_CHANB_CTL), MPSCC_CR2);
	outb(MKAX(AX_CHANB_CTL), 0);	/* Interrupt vector = 0 */
	outb(MKAX(AX_CHANB_CTL), MPSCC_CR1);
	outb(MKAX(AX_CHANB_CTL), MPSCC_CAV);

	/* Set up MPSCC Channel A parameters */
	for(i = 0; i < sizeof(ax_on_tbl); i++)
		outb(MKAX(AX_CHANA_CTL), ax_on_tbl[i]);

	/* If RTS is looped back to CTS an interrupt for CTS change will
		show up immediately.  So wait for CTS to settle and then
		clear pending interrupts. */
	for(i = 100; i; i++)
		;

	outb(MKAX(AX_CHANB_CTL), MPSCC_RST_TxINTP);
	outb(MKAX(AX_CHANA_CTL), MPSCC_RST_EXTINT);
	outb(MKAX(AX_CHANA_CTL), MPSCC_RST_TxINTP);

	/* enable DMA/interrupts */
	outb(MKAX(AX_CHANB_CTL), MPSCC_CR5);
	outb(MKAX(AX_CHANB_CTL), MPSCC_DTR);

	ax_rx_setup();		/* Get setup to input a packet */

	/* Get current state of Abort/CTS/Sync/DCD */
	i = inb(MKAX(AX_CHANA_CTL)); /* Get External Status Status */
	if(i & MPSCC_BREAK)
		ax_aborting = 1;
	if(!(i & MPSCC_SYNC))
		ax_insync = 1;
	if(i & MPSCC_CTS)
		ax_cts = 1;
	if(i & MPSCC_DCD)
		ax_dcd = 1;

	/* turn interrupts on */
	int_on();
    }
    else ax_close();		/* Let ax_close do the work */
}


/* Setup the MPSCC chip and the DMA logic so that when the next packet
	arrives, it will automatically be dma'd directly into a preallocated
	packet buffer.  We will only get an interrupt after the entire packet
	is in memory, or on error.  This routine must execute atomically, i.e.
	at interrupt level or with interrupts masked off.
 */
ax_rx_setup()
{
	register int i;

	dma_reset(custom.c_rcv_dma);

	/* Reset rx error */
	outb(MKAX(AX_CHANA_CTL), MPSCC_ERROR_RST);

	/* Reset CRC checker */
	outb(MKAX(AX_CHANA_CTL), MPSCC_RST_RxCRCC);

	if (ax_inp == NULL) {
		ax_inp = getfree();	/* Preallocate the next packet */
		if(ax_inp == NULL)
			return;
	}

	/* Set interrupt on next rx character */
	outb(MKAX(AX_CHANA_CTL), MPSCC_ENA_INTRxC);

	dma_setup(custom.c_rcv_dma, ax_inp->nb_buff, LBUF, DMA_INPUT);
}
