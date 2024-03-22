/*  Copyright 1986 by Carnegie Mellon  */
/*  See permission and disclaimer notice in file "cmu-note.h"  */
#include	<cmu-note.h>
/*  Copyright 1984, 1985 by the Massachusetts Institute of Technology  */
/*  See permission and disclaimer notice in file "notice.h"  */
#include	<notice.h>

#include <task.h>
#include <q.h>
#include <netq.h>
#include <net.h>
#include <custom.h>
#include <netbuf.h>
#include <stdio.h>
#include <ether.h>
#include "3com.h"
#include <stdio.h>

/* Transmit a packet. If it's an IP packet, calls ARP to figure out the
	ethernet address. Disables receiver, DMAs packet from memory to
	card, then transmits packet. Performs timeout checking on the
	DMA transfer and the packet send. Send timeout probably means a
	collision.
   There seems to be a problem losing interrupts on the PC/AT, so the
	transmit routine no longer depends on interrupts at all. Even
	so, we still lose receive interrupts, and we theorize that
	there's a small window at the beginning of the transmit routine
	where when we turn off the receiver we've got a race condition
	between turning it off and it giving us an interrupt because it
	received a packet. We couldn't fix the lost interrupt problem,
	so we finally had to add the task spawned by et_init which pokes
	the controller every 3 seconds.
   When we did do transmits without fixing up the receiver, the
	ethernet board got horribly confused and stopped giving us any
	interrupts at all. A general reset to the board seemed necessary
	to get it unconfused.
*/


/* 19-Aug-84 - changed et_send to send packets sent to x.x.x.0 to the
	ethernet broadcast address.
						<John Romkey>
   Jan-85 Fixed lost interrupts, changed send routine to never turn off
	DMA enable bit.				<Saltzer, Romkey>
   Dec-86 Fixed to be able to send long broadcast packets.  As far as
	I could tell, the silly hardware was generating receive overflow
	interrupts upon recognizing the broadcast address as it's own
	and then realizing that the buffer wasn't available.  This
	then caused an interrupt before completion of the send.  Upon
	entering the interrupt handler and reading the receive status
	register, the transmission would be somehow aborted.
						<Drew D. Perkins>
*/

#define	ETTIMEOUT	5	/* timeout in 1/18 secs for sending pkts */

unsigned etdmstart;
extern unsigned etdmadone;
unsigned ettxdma = 0;
unsigned ettmo = 0;
unsigned etsend = 0;
unsigned etunder = 0;
unsigned etcoll  = 0;
unsigned etcollsx = 0;
unsigned etrdy = 0;
unsigned ettxunknown = 0;

#ifdef	WATCH
unsigned etminlen;
#endif

extern long cticks;

et_send(p, prot, len, fhost)
	PACKET p;
	unsigned prot;
	unsigned len;
	in_name fhost; {
	register struct ethhdr *pe;
	unsigned temp;
	long time;
	int vec;

	/* Set up the ethernet header. Insert our address and the address of
		the destination and the type field in the ethernet header
		of the packet. */
#ifdef	DEBUG
	if(NDEBUG & (INFOMSG|NETRACE))
		printf("ET_SEND: p[%u] -> %a.\n", len, fhost);
#endif

	pe = (struct ethhdr *)p->nb_buff;
	etadcpy(_etme, pe->e_src);

	/* Setup the type field and the addresses in the ethernet header. */
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
				printf("ET_SEND: ether address unknown\n");
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
			printf("ET_SEND: Unknown prot %u.\n", prot);
#endif
		return 0;
		}

	len += sizeof(struct ethhdr);
#ifdef	WATCH
	if(len < etminlen)
		len = etminlen;
#endif
#ifndef	WATCH
	if(len < ET_MINLEN)
		len = ET_MINLEN;
#endif


	/* before touching anything, check & see if we've encountered
		an obscure condition where we seem to have lost a
		receive interrupt. If it looks like this has happened,
		reset the board.
	   ***Update. According to what I now know, the buffer status
		does not automatically switch from XMTRCV to RCV when the
		transmit completes. This means that this reset will
		occur whenever we send two packets without receiving one
		between the two sends. So this test is bogus, but if
		I remove it the code seems to fail sometimes.

	 Turn off the receiver
		Have to leave the Interrupt & DMA Enable bit set when
		doing this because of an obscure problem with systems
		with the expansion chassis. On these systems, if you
		just write a 0 to the AUXCMD register and later enable
		DMA at the same time as you do a DMA request, you lose
		and no DMA occurs. It's fine to write a 0 to the AUXCMD
		register otherwise. The whole reason we do this is to
		guarantee that the receiver will go off.
	*/

/*~*/	/* mask out interrupts for the specified line */
	vec = (1 << custom.c_intvec);
	outb(IIMR, inb(IIMR) | vec);

	outb(EAUXCMD, EINTDMAENABLE|ESYSBUS);
/*~*/	et_ihnd();

	etdmstart = 2048-len;
	outw(EGPPLOW, etdmstart);

	if(custom.c_tx_dma == 0) {
		fastout(EBUFWIN, pe, len);
		}
	else {
		dma_setup(custom.c_tx_dma, pe, len, DMA_OUTPUT);

		outb(EAUXCMD, EINTDMAENABLE|EDMAREQ|ESYSBUS);

		/* We should now be dma'ing the packet */
		time = cticks;

		while(!(inb(EAUXSTAT) & EDMADONE))
			if(cticks - time > ETTIMEOUT) {
				ettmo++;

				/* reset the DMA controller & net interface */
				outb(EAUXCMD, EINTDMAENABLE|ESYSBUS);
				dma_reset(custom.c_tx_dma);
#ifdef DEBUG
				if(NDEBUG & (INFOMSG|PROTERR|NETERR))
					printf("et_send: dma tmo\n");
#endif
/*				outb(ECLRRP, 0);
				outb(EAUXCMD, EINTDMAENABLE|ERECEIVE);

				return 0;
*/
				goto punt;
				}
		dma_reset(custom.c_tx_dma);
		etdmadone++;
		ettxdma++;
		}

	etsend++;

/*	int_off();	/* DDP - This is necessary believe it or not */
rexmit:
	time = cticks;
	outb(EAUXCMD, EINTDMAENABLE|ESYSBUS);
	outw(EGPPLOW, etdmstart);

	/* have to get things ready for the receiver here */
	outb(ECLRRP, 0);
	outb(EAUXCMD, EINTDMAENABLE|EXMTRCV);

	/* We should now be transmitting the packet */
	while((temp = inb(EAUXSTAT)) & EXMITIDLE) {
		if(cticks - time > ETTIMEOUT) {
			ettmo++;

			/* DDP - reset the net interface one bit at a time */
			outb(EAUXCMD, EINTDMAENABLE|ESYSBUS);
#ifdef DEBUG
			if(NDEBUG & (INFOMSG|PROTERR|NETERR))
				printf("et_send: net tmo\n");
#endif

/*			int_on();	/* DDP */
punt:
/*~*/			/* enable interrupts for the specified line */
			outb(IIMR, inb(IIMR) & ~vec);

			outb(ECLRRP, 0);
			outb(EAUXCMD, EINTDMAENABLE|ERECEIVE);

			return 0;
			}
		}
/*	int_on();	/* DDP */

	temp = inb(ETXCMD);
	if(temp & EUNDERFLOW)
		etunder++;
	else if(temp & ECOLLISION) {
		etcoll++;
		goto rexmit;
	}
	else if(temp & ECOLLSIXTEEN) {
		etcollsx++;
#ifdef DEBUG
		printf("Excessive collisions detected.  Is the ethernet cable plugged in?\n");
#endif
	}
	else if(temp & ERDYFORNEW)
		etrdy++;
	else
		ettxunknown++;

/*~*/	/* enable interrupts for the specified line */
	outb(IIMR, inb(IIMR) & ~vec);

	return len;
	}
