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
#include <sdlc.h>
#include "mb.h"
#include "sccreg.h"

/* This file contains the initialization code for the Tangent Technologies
MacBridge card for the PC's.  MB_INIT() has to initialize the serial port,
perhaps fill in some of its net struct and fork the serial line packet to
protocol demultiplexor.
 */

NET *mb_net;		/* Serial line net descriptor */
task *MbDemux;		/* The packet-to-protocol demultiplexor. */
char save_mask;		/* receiver command on entry. */
unsigned mb_eoi;
PACKET mb_inp;		/* Current input packet */

extern int mb_demux();	/* the routine which is the body of the demux task */
extern int mb_insync;	/* Sync'd flag */


mb_init(net, options, dummy)
	NET *net;
	unsigned options;
	unsigned dummy; {

#ifdef	DEBUG
	if(NDEBUG & INFOMSG)
		printf("Initializing MacBridge.\n");
#endif

	/* patch in a bogus gateway address so the internet router
		doesn't complain.
	*/
	net->n_defgw = 0x01020304;

#ifdef	DEBUG
	if(NDEBUG & INFOMSG)
		printf("Forking MBDEMUX.\n");
#endif

	MbDemux = tk_fork(tk_cur, mb_demux, net->n_stksiz, "MBDEMUX", net);
	if(MbDemux == NULL) {
		printf("Can't fork MbDemux task.\n");
		exit(1);
	}
	
	mb_net = net;
	mb_net->n_demux = MbDemux;

	mb_inp = getfree();	/* Preallocate the first packet */
	if(mb_inp == NULL) {
		printf("MB_INIT: Can't pre-allocate input packet.\n");
		exit(1);
	}

	mb_switch(1, options);

	tk_yield();	/* Give the per net task a chance to run. */

	return;
	}

char mb_on_tbl[] = {
/* sdlc mode/x1 clk/sync mode/no parity (must be done first) */
	SCC_WR4, SCC_CLK_x1|SCC_SYNC_SDLC|SCC_SYNC_ENA,
/* Tx 8 bits/SDLC poly/Tx disable/Tx CRC enable (initialize transmitter) */
	SCC_WR5, SCC_Tx_8BITS|SCC_POLY_SDLC|SCC_TxCRC_ENA,
/* Rx 8 bits/Enter hunt mode/Enable CRC Gen (initialize receiver) */
	SCC_WR3, SCC_Rx_8BITS|SCC_ENTER_HUNT|SCC_RxCRC_ENA,
/* crc preset to 0, nrz, flag idle */
	SCC_WR10, SCC_NRZ|SCC_FLAG,
#define LINK_ADDRESS	0	/* Doesn't matter at this point */
/* SDLC link address */
	SCC_WR6, LINK_ADDRESS,
/* SDLC flag (is this already fixed?) */
	SCC_WR7, 0x7e,
/* interrupt vector */
	SCC_WR2, 0x0,
/* no xtal, tx=RTxC pin, rx=RTxC pin (initialize clocking) */
	SCC_WR11, SCC_RxC_RTxC|SCC_TxC_RTxC|SCC_TRxC_OUTPUT|SCC_TRxC_TxC,
#ifdef DCE			/* Only if we supply clocking */
/* Load BRG constants (done before BRG enabled in WR14) */
/* br gen lsb divide */
	SCC_WR12, BRG_LSB,	/* Divisor for desired clock rate */
/* br gen msb divide */
	SCC_WR13, BRG_MSB,
/* Loopback mode for debugging/BRG enable */
	SCC_WR14, SCC_LOOPBACK|SCC_BRG_ENA,
#endif
/* only break/abort and sync external/status ints */
	SCC_WR15, SCC_ENA_BREAK|SCC_ENA_SYNC,
/* Tx 8 bits/SDLC poly/Tx enable/Tx CRC enable (enable transmitter) */
	SCC_WR5, SCC_DTR|SCC_Tx_8BITS|SCC_POLY_SDLC|SCC_Tx_ENA|SCC_TxCRC_ENA,
/* Initialize CRC generator (after transmitter enabled) */
	/* SCC_WR0, */ SCC_RST_TxCRCG,
/* Reset Tx underrun/EOM latch (after transmitter enabled) */
	/* SCC_WR0, */ SCC_RST_TxUNDER,
/* no wait/dma/interrupts yet (enable in mb_start_rx()) */
	SCC_WR1, 0x00,
/* allow interrupts */
	SCC_WR9, SCC_MIE|SCC_NOVECTOR|SCC_VIS
};

/*
	Routine to switch the board and interrupts on/off.
 */
mb_switch(state, options)
int state;
unsigned options;
{
	int vec, i;
	char *m;

    if(state) {			/* Turn them on? */
	int_off();		/* Disable interrupts. */

	/* Reset the MacBridge SCC chip. */
	outb(MB_CHANA_CTL, SCC_WR9);
	outb(MB_CHANA_CTL, SCC_RST_HARD);

	for(i = 0; i < sizeof(mb_on_tbl); i++)
		outb(MB_CHANA_CTL, mb_on_tbl[i]);

	/* patch in the new interrupt handler - rather, call the routine to
		do this. This routine saves the old contents of the vector.
	*/
	mb_eoi = 0x60 + custom.c_intvec;
	mb_patch(custom.c_intvec<<2);

	/* setup interrupts for the specified line */
	vec = (1 << custom.c_intvec);
	save_mask = inb(IIMR) & vec;
	outb(IIMR, inb(IIMR) & ~vec);

	mb_rx_setup();		/* Get setup to input a packet */

/* Tx 8 bits/SDLC poly/Tx enable/Tx CRC enable (enable interrupts) */
	outb(MB_CHANA_CTL, SCC_WR5);
	outb(MB_CHANA_CTL, SCC_DTR|SCC_Tx_8BITS|SCC_POLY_SDLC|SCC_Tx_ENA|SCC_RTS|SCC_TxCRC_ENA);

	/* turn interrupts on */
	int_on();
    }
    else mb_close();	/* Let mb_close do the work */
}


char mb_rx_tbl[] = {
/* Reset external/status int */
	/* SCC_WR0, */ SCC_RST_EXTINT,
/* Error Reset */
	/* SCC_WR0,*/ SCC_ERROR_RST,
/* Reset highest ius */
	/* SCC_WR0, */ SCC_RST_HIUS,
/* enable dma (vs. wait) on receive (vs. transmit) */
	SCC_WR1, SCC_WAITDMA_FUN|SCC_WAITDMA_RTx,
/* now enable dma and interrupts */
	SCC_WR1, SCC_WAITDMA_ENA|SCC_WAITDMA_FUN|SCC_WAITDMA_RTx|SCC_ENA_SPECCND|SCC_ENA_EXTINT,
};


/* Setup the SCC chip and the DMA logic so that when the next packet arrives,
	it will automatically be dma'd directly into a preallocated packet
	buffer.  We will only get an interrupt after the entire packet is
	in memory, or on error.  This routine must execute atomically, i.e.
	at interrupt level or with interrupts masked off.
 */
mb_rx_setup()
{
	register int i;

	inb(MB_CHANA_CTL);	/* Make sure register pointer = 0 */

	dma_reset(custom.c_rcv_dma);

	for(i = 0; i < sizeof(mb_rx_tbl); i++)
		outb(MB_CHANA_CTL, mb_rx_tbl[i]);

/* Rx 8 bits/Enter hunt mode/Enable CRC Gen (enable receiver) */
	outb(MB_CHANA_CTL, SCC_WR3);
	outb(MB_CHANA_CTL, mb_insync ? SCC_Rx_8BITS|SCC_RxCRC_ENA|SCC_Rx_ENA :
		SCC_Rx_8BITS|SCC_RxCRC_ENA|SCC_Rx_ENA|SCC_ENTER_HUNT);

	if (mb_inp == NULL)
		return;
	
	outb(MB_CHANB_CTL, 5);
	outb(MB_CHANB_CTL, SCC_DTR);	/* Enable Channel A dma */

	dma_setup(custom.c_rcv_dma, mb_inp->nb_buff, LBUF, DMA_INPUT);
}
