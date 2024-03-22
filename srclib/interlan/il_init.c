/*  Copyright 1986 by Carnegie Mellon  */
/*  See permission and disclaimer notice in file "cmu-note.h"  */
#include	<cmu-note.h>
/*  Copyright 1985 by Micom-Interlan, Inc. */
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
#include <timer.h>
#include <int.h>
#include <dma.h>
#include "interlan.h"

/* interlan NI5010 IBM PC ethernet card initialization.
	written 8/10/85
				<John Romkey>
*/

/* storage for lots of things like my ethernet address, the ethernet broadcast
	address and my task and net pointers
*/
char _etme[6];		/* my ethernet address */
task *EtDemux;		/* ethernet packet demultiplexing task */
NET *et_net;		/* my net pointer */
unsigned il_eoi;

int il_demux();		/* the routine which is the body of the demux task */
unsigned save_mask;
unsigned il_rcv_mode;

extern unsigned etint;

il_init(net, options, dummy)
	NET *net;
	unsigned options;
	unsigned dummy; {

	/* start up the demultiplexing task.
	*/
	EtDemux = tk_fork(tk_cur, il_demux, net->n_stksiz, "ETDEMUX", net);
	if(EtDemux == NULL) {
		printf("can't fork interlan task\n");
		exit(1);
		}

	et_net = net;
	et_net->n_demux = EtDemux;

	il_switch(1, options);		/* DDP */

	tk_yield();	/* Give the per net task a chance to run. */

	/* init arp - nets to be on a per-net basis */
	etainit();
	return;
	}


/*
	Routine to switch the board and interrupts on/off.
 */
il_switch(state, options)
int state;
unsigned options;
{
	char temp;
	int i;
	union {
		long ulong;
		char uc[4];
		} myaddr;
	int vec;	/* crock to circumvent compiler bug */

    if(state) { 	/* Turn them on? */
	/* reset the card and patch in our interrupt handler
	*/
	int_off();

	outb(RESET, RS_RESET);
	outb(RESET_ALL, 0);

	/* patch in the new interrupt handler - rather, call the routine to
		do this, saving the old interrupt mask and handler.
	*/
	il_eoi = 0x60 + custom.c_intvec;
	il_patch(custom.c_intvec<<2);

	/* setup interrupts for the specified line */
	vec = (1 << custom.c_intvec);
	save_mask = inb(IIMR) & vec;
	outb(IIMR, inb(IIMR) & ~vec);

	/* should be safe to turn interrupts back on now.
	*/
	int_on();

	/* set up the card's hardware ethernet address. several choices.
	   first have to set loopback.	
	*/
	outb(XMIT_MODE, XMD_LBC);

#ifdef	DEBUG
	if(NDEBUG & INFOMSG)
		printf("ethernet address is ");
#endif

	switch(custom.c_seletaddr) {
	/* choose the address programmed into the card
	*/
	case HARDWARE:
		for(i=0; i<6; i++) {
			outw(M_START_LO, i);
			temp = inb(ETHER_ADDR);
#ifdef	DEBUG
			if(NDEBUG & INFOMSG)
				printf("%02x", temp&0xff);
#endif
			_etme[i] = temp;
			outb(NODE_ID_0+i, temp);
			}
		break;

	/* use our internet address. sets the two most significant
		bytes of the ether address to zero. Used to be used
		instead of ARP for figuring out addresses.
	*/
	case ETINTERNET:
		myaddr.ulong = 	et_net->ip_addr;
		for(i=3; i != -1; i--) {
			_etme[i+2] = myaddr.uc[i];
			outb(NODE_ID_2+i, myaddr.uc[i]);
			}

		/* zero out the two most significant bytes */
		_etme[0] = 0;
		outb(NODE_ID_0, 0);
		_etme[1] = 0;
		outb(NODE_ID_1, 0);

#ifdef	DEBUG
		if(NDEBUG & INFOMSG) {
			for(i=0; i<6; i++)
				printf("%02x", _etme[i]);
			printf("\n");
			}
#endif
		break;

	/* use an ethernet address entirely specified by the user. may
		be useful for debugging.
	*/
	case ETUSER:
		for(i=0; i<6; i++) {
			_etme[i] = custom.c_myetaddr.e_ether[i];
			outb(NODE_ID_0+i, _etme[i]);
#ifdef	DEBUG
			if(NDEBUG & INFOMSG)
				printf("%02x",_etme[i]);
#endif
			}
		break;
	default:
		printf("invalid ethernet address selection option\n");
	}

#ifdef	DEBUG
	if(NDEBUG & INFOMSG) printf("\n");
#endif

	/* start out with transmitter interrupts off */
	outb(XMIT_MASK, 0x0);

	/* set up the transmitter mode */
	outb(XMIT_MODE, (XMD_IG_PAR|XMD_T_MODE|XMD_LBC));

	/* reset any pending transmitter interrupts */
	outb(CLR_XMIT_INT, 0xff);

	/* set up the receiver mode. store it in a variable so that
		we can use it when enabling the receiver after
		transmitting or recieving a packet.
	*/
	if(options & ALLPACK)
		il_rcv_mode = RMD_ALL_PACKETS;
	else if(options & MULTI)
		il_rcv_mode = RMD_MULTICAST;
	else
		il_rcv_mode = RMD_BROADCAST;

	outb(RCV_MODE, il_rcv_mode);

	il_rcvr_reset();

	/* now un-reset the card */
	outb(RESET, 0);

	/* Now everything is initialized. The DMA channel should only be
		initialized on demand, so it's not necessary to touch it
		now.
	*/
    }
    else il_close();	/* Let il_close do the work */
}