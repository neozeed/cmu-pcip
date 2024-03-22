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

/* Transmit a packet.
*/

extern long cticks;

unsigned mbsent = 0;		/* Number of packets sent on link */
unsigned mbresend = 0;		/* Number of resends */
unsigned mbtmo = 0;		/* Number of timeouts on send */
unsigned mbswait = 0;		/* Number of waits on send */
unsigned long mbschr = 0;	/* Number of characters sent */
unsigned long mbsloop = 0;	/* Number of wait loops in send */
unsigned long mbuloop = 0;	/* Number of wait loops in underrun wait */

extern int mb_insync;		/* Receiver sync'ed flag */

mb_send(p, prot, len, fhost)
	PACKET p;
	unsigned prot;
	unsigned len;
	in_name fhost; {
	int i, length;
	long time;
	register char *cp;
	struct sdlc_hhdr *psdlc;
	int status;

	/* Set up the sdlc header. Insert the address of
		the destination and the control field in the sdlc header
		of the packet. */
#ifdef	DEBUG
	if(NDEBUG & (INFOMSG|NETRACE))
		printf("MB_SEND: p[%u] -> %a.\n", len, fhost);
#endif

	psdlc = (struct sdlc_hhdr *)p->nb_buff;
	psdlc->sdlc_address = SDLC_BROADCAST; /* Broadcast for now */

	/* Setup the type field and the addresses in the sdlc header. */
	switch(prot) {
	case IP:
		psdlc->sdlc_type = SDLC_IP;
		break;

	default:
#ifdef	DEBUG
		if(NDEBUG & (INFOMSG|PROTERR|BUGHALT))
			printf("MB_SEND: Unknown prot %u.\n", prot);
#endif
		return 0;
	}

restart:
	length = len + sizeof(struct sdlc_hhdr);

	if(!mb_insync) {
		printf("mb_send: Receiver not synced.  Is the cable plugged in?\n");
		mb_rx_setup();
	}

	/* We should now be ready to transmit the packet */
	time = cticks;

	while(!(inb(MB_CHANA_CTL) & SCC_TxBUF_EMPTY)) {
		mbswait++;		/* Had to wait for old packet again */
		if(cticks - time > MB_TIMEOUT) { /* Too long? */
			mbtmo++;
			printf("mb_send: send timeout, probably no tx clock.\n");
			return 0;
		}
	}

	outb(MB_CHANA_CTL, SCC_RST_TxCRCG); /* Reset CRC generator */

	cp = p->nb_buff;	/* Set up pointer to data */

	int_off();		/* Need exclusive access to SCC */
	outb(MB_CHANA_DATA, *cp++); /* Output first character */
	outb(MB_CHANA_CTL, SCC_RST_TxUNDER); /* Reset Tx underrun/EOM latch */
	outb(MB_CHANA_CTL, SCC_WR10); /* Setup for write to register 10 */
				/* Send Abort on Tx underrun */
	outb(MB_CHANA_CTL, SCC_NRZ|SCC_FLAG|SCC_ABORT);
	int_on();		/* Done */

	for(i = length - 1; i; i--) {
		/* Wait for Tx buffer to be empty */
		while(!(inb(MB_CHANA_CTL) & SCC_TxBUF_EMPTY)) {
			mbsloop++;		/* Waited again... */
			if(cticks - time > MB_TIMEOUT) { /* Too long? */
				mbtmo++;	/* Yep, abort transfer */
				outb(MB_CHANA_CTL, SCC_SENDABORT);
#ifdef DEBUG
				if(NDEBUG & (INFOMSG|PROTERR|NETERR))
					printf("mb_send: send timeout.\n");
#endif
				return 0;
			}
		}

 		if(!mbsnd(MB_CHANA_CTL, *cp++)) {
#ifdef DEBUG
			printf("mb_send: lost a packet, resending!\n");
#endif
			mbresend++;
			int_off();	/* Need exclusive access to SCC */
					/* Setup for write to register 10 */
			outb(MB_CHANA_CTL, SCC_WR10);
					/* Send CRC on Tx underrun */
			outb(MB_CHANA_CTL, SCC_NRZ|SCC_FLAG);
			int_on();	/* Done */
			goto restart;
		}
			
		mbschr++;
	}

	int_off();			/* Need exclusive access to SCC */
	outb(MB_CHANA_CTL, SCC_WR10);	/* Setup for write to register 10 */
	outb(MB_CHANA_CTL, SCC_NRZ|SCC_FLAG); /* Send CRC on Tx underrun */
	int_on();			/* Done */

	/* Wait for Tx underrun/EOM latch to be set */
	while(!(inb(MB_CHANA_CTL) & SCC_TxUNDER)) {
		mbuloop++;		/* Waited again... */
		if(cticks - time > MB_TIMEOUT) { /* Too long? */
			mbtmo++;	/* Yep, abort transfer */
			outb(MB_CHANA_CTL, SCC_SENDABORT);
					/* Reset Tx underrun/EOM latch */
			outb(MB_CHANA_CTL, SCC_RST_TxUNDER);
#ifdef DEBUG
			if(NDEBUG & (INFOMSG|PROTERR|NETERR))
				printf("mb_send: EOM tmo\n");
#endif
			return 0;
		}
	}
	outb(MB_CHANA_CTL, SCC_RST_TxUNDER); /* Reset Tx underrun/EOM latch */
	mbsent++;

	return (length);
}
