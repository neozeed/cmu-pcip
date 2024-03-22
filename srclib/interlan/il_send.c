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
#include <stdio.h>

/* Send an ethernet packet. The packet type is passed down by the caller;
	it can be IP or ARP.

	Observation about the NI5010 ethernet interface: the original driver
	waited for DMA and transmit to complete by polling the interface
	and waiting for the appropiate bit to flip. The bit seems to flip
	before DMA or transmit really does complete, though, and the code
	proceeds to reset the DMA controller or transmitter, screwing
	things up. So the driver has to use completion interrupts on DMA
	and transmit.
*/

#define	ETTIMEOUT	5	/* timeout in 1/18 secs for sending pkts */

unsigned ettmo = 0;
unsigned il_tx_coll = 0;

extern long cticks;
extern int il_dma_done;
extern int il_tx_done;
extern NET *et_net;

il_send(p, prot, len, fhost)
	PACKET p;
	unsigned prot;
	unsigned len;
	in_name fhost; {
	register struct ethhdr *pe;
	unsigned temp;
	unsigned i;
	char txstat;
	char txmode;
	long time;
	unsigned pkt_start;

#ifdef DEBUG						/* Added line - LKR */
	if(NDEBUG & NETRACE)
		printf("il_send: p[%d] [addr %04x] type %d\n", len, p, prot);
#endif							/* Added line - LKR */

	/* Set up the ethernet header. Insert our address and the address of
		the destination and the type field in the ethernet header
		of the packet. */

	pe = (struct ethhdr *)p->nb_buff;
	etadcpy(_etme, pe->e_src);

	/* Setup the type field and the addresses in the ethernet header.
	*/
	switch(prot) {
	case IP:
		if((fhost == 0xffffffff) || /* Physical cable broadcast addr*/
					/* All subnet broadcast */
			(fhost == et_net->n_netbr) ||
					/* All subnet bcast (4.2bsd) */
			(fhost == et_net->n_netbr42) ||
					/* Subnet broadcast */
			(fhost == et_net->n_subnetbr)) {
			etadcpy(ETBROADCAST, pe->e_dst);
			}
		else if(ip2et(pe->e_dst, (in_name)fhost) == 0) {
#ifdef	DEBUG
			if(NDEBUG & (INFOMSG|NETERR))
				printf("IL_SEND: ether address unknown\n");
#endif
			return 0;
			}
		pe->e_type = ET_IP;
		break;
	case ARP:
		etadcpy((char *)fhost, pe->e_dst);
		pe->e_type = ET_ARP;
		break;
	default:
#ifdef	DEBUG
		if(NDEBUG & (INFOMSG|PROTERR|BUGHALT))
			printf("IL_SEND: Unknown prot %u.\n", prot);
#endif
		return 0;
		}

	len += sizeof(struct ethhdr);

	/* ethernet packets must be at least 60 bytes long
	*/
	if(len < ET_MINLEN)
		len = ET_MINLEN;

	/* disable the receiver */
	outb(RCV_MASK, 0);
	outb(M_MODE, 0);
	outb(CLR_RCV_INT, 0xff);

	/* set the starting address in the output packet buffer */
	pkt_start = 2048-len;
	outw(M_START_LO, pkt_start);

	/* copy the packet. If the Receive DMA channel is 0, then do fastio.
	   otherwise DMA it.
	*/
	if(custom.c_tx_dma == 0) {
		fastout(XMIT_BUF, pe, len);
		}
	else {
		dma_setup(custom.c_tx_dma, pe, len, DMA_OUTPUT);

		il_dma_done = FALSE;

		/* disable the receiver and start the dma */
		outb(M_MODE, MM_EN_DMA);

		time = cticks;

		/* wait for dma int. Tried looping on ENDMA bit but that
			seemed indicate DMAs were done prematurely and
			we'd lose some data.
		*/
		while(!il_dma_done)
			if(cticks - time > ETTIMEOUT) {
				ettmo++;
#ifdef DEBUG						/* Added line - LKR */
				if(NDEBUG & (INFOMSG|PROTERR|NETERR))
					printf("il_send: dma tmo\n");
#endif							/* Added line - LKR */

				/* reset the DMA controller & net interface */
				dma_reset(custom.c_tx_dma);

				il_rcvr_reset();
				return 0;
				}

#ifdef DEBUG						/* Added line - LKR */
		if(NDEBUG & NETRACE)
			printf("dma done, took %D ticks\n", cticks-time);
#endif							/* Added line - LKR */

		dma_reset(custom.c_tx_dma);
		}

	/* come back up here if we get a collision
	*/
retransmit:
	time = cticks;

	/* rewrite where the packet starts */
	outw(M_START_LO, pkt_start);

	/* transmit it, one bit at a time */
	il_tx_done = FALSE;

	outb(M_MODE, MM_MUX);
	outb(M_MODE, (MM_EN_XMT|MM_MUX));
	outb(XMIT_MASK, 0xfe);

	/* have to get things ready for the receiver here */
	while(!il_tx_done)
		if(cticks - time > ETTIMEOUT) {
			ettmo++;
#ifdef DEBUG						/* Added line - LKR */
			if(NDEBUG & (INFOMSG|PROTERR|NETERR))
				printf("il_send: net tmo\n");
#endif							/* Added line - LKR */

			outb(XMIT_MASK, 0);
			outb(CLR_XMIT_INT, 0xff);
			il_rcvr_reset();
			return 0;
			}

	txstat = inb(XMIT_STAT);
	txmode = inb(XMIT_MODE);
#ifdef DEBUG						/* Added line - LKR */
	if(NDEBUG & NETRACE)
		printf("INT_STAT = %02x [writing 0]\n", inb(INT_STAT)&0xff);
#endif							/* Added line - LKR */

	outb(XMIT_MASK, 0);
	outb(CLR_XMIT_INT, 0xff);

	if(txstat & XS_COLL) {
#ifdef DEBUG						/* Added line - LKR */
		if(NDEBUG & NETRACE)
			printf("il_send: collision!\n");
#endif							/* Added line - LKR */
		il_tx_coll++;
		goto retransmit;
		}

	/* reenable the receiver */
	il_rcvr_reset();

	return len;
	}
