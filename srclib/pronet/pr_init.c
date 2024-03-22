/*  Copyright 1986 by Carnegie Mellon  */
/*  See permission and disclaimer notice in file "cmu-note.h"  */
#include	<cmu-note.h>
/*  Copyright 1984, 1985 by Proteon, Inc. */
/*  See permission and disclaimer notice in file "proteon-notice.h"  */
#include	"proteon-notice.h"

/***********************************************************************
 * HISTORY
 *
 * 24-Mar-86, Drew D. Perkins (ddp), Carnegie-Mellon University
 *  Add pr_switch routine to switch interrupts in/out.
 *
 * 16-Jan-86, Drew D. Perkins (ddp), Carnegie-Mellon University
 *  Removed code that tested for _IBM_AT and changed interrupts.
 *
 * 20-Sep-85, Drew D. Perkins (ddp), Carnegie-Mellon University
 *  Merged with latest MIT code, and made MSC 3.0 compatible.
 *
 * 23-Apr-84, Eric R. Crane (erc), Carnegie-Mellon University
 *  Added and rearanged code so that the board will now find its local
 * ring address and set its IP address acordingly
 ***********************************************************************
 */

#include <stdio.h>
#include <task.h>
#include <q.h>
#include <netq.h>
#include <net.h>
#include <custom.h>
#include <ip.h>			/* DDP */
#include <mach_type.h>
#include "pronet.h"

/* initialize the proNET interface. Basically, enable interrupts at the
	interrupt controller and the interface, and enable receive.
	Should also figure out our host address based on the hardware
	address, but to do that we have to send a packet, which is a bit
	more complicated than I want to do right now.
*/

char prBROADCAST = 0xff; /* DDP */
char _prme;		/* my proNET address */
task *prDemux;		/* proNET packet demultiplexing task */
NET *pr_net;		/* my net pointer */
unsigned pr_eoi;

int retry_count = 0;	/* DDP */

unsigned pr_int_base_default = INT_BASE1;	/* default for non-AT */
unsigned pr_int_base;
unsigned pr_ocwr = IOCWR;

int pr_demux();		/* the routine which is the body of the demux task */

pr_init(net, options, dummy)
	NET *net;
	unsigned options;
	unsigned dummy; {
	char temp;
	char initb;
	int i;
	int iimr = IIMR;
	int waitcount;			/* DDP */
	union _ipname temp_addr;	/* DDP */

#ifdef	DEBUG
	if(NDEBUG & INFOMSG)
		printf("Forking prDEMUX.\n");
#endif

	prDemux = tk_fork(tk_cur, pr_demux, net->n_stksiz, "PRDEMUX");
	pr_net = net;
	pr_net->n_demux = prDemux;

	/* if it's int 2 and the machine is an AT, hack, hack.

	if((custom.c_intvec == 2) && (_mach_type() == _IBM_AT)) { 
		custom.c_intvec = 1;
		pr_int_base_default = INT_BASE2;
		pr_ocwr = IOCWR2;
		iimr = IIMR2;
		}
*/

/* DDP Start changes... */
	/*
	If the host part of our ip address is set to a zero, then set
	it from our hardware address.  To do this, broadcast some packets
	in digital loopback mode to find out what our source hardware
	address is.  Set our ip address according to our hardware address.
	 */
	_prme = ((union _ipname *)&(pr_net->ip_addr))->in_lst.in_host;

	while( _prme == 0 && retry_count++ < 10) {
#ifdef DEBUG
	    if(NDEBUG & INFOMSG)
		printf("PR_INIT: Trying to find local LNI address (%d)\n",retry_count);
#endif

	    outb(mkv2(V2ICSR), INRST);          /* Reset the INPUT side */
	    outb(mkv2(V2OCSR), OUTRST);         /* Reset the OUTPUT side */

	    outb(mkv2(V2ILCNT), 0);             /* Put a Zero here */
	    outb(mkv2(V2IHCNT), 0);             /* Put a Zero here */
	    outb(mkv2(V2ICSR), MODE1|COPYEN);   /* now we wait for a packet */

	    /* Now set up the transmit side */

	    outb(mkv2(V2OLCNT), (-2)&0xff);     /* Set up the word count */
	    outb(mkv2(V2OHCNT), ((-2)>>8)&0x07);
	    outb(mkv2(V2OBUF), prBROADCAST);	/* We are doing a broadcast */
	    outb(mkv2(V2OLCNT), (-2)&0xff);     /* Reset the word count */
	    outb(mkv2(V2OHCNT), ((-2)>>8)&0x07); 
	    outb(mkv2(V2OCSR), ORIGEN|INITRING); /* Send the packet off */

	    /* Now wait for the transmit and receive to finish */

	    for(waitcount=0;waitcount<10000;waitcount++) {
		if (!(inb(mkv2(V2ICSR)) & COPYEN)) {
		    outb(mkv2(V2ILCNT), 0);	/* Clear the low byte */
		    outb(mkv2(V2IHCNT), 0);	/* Clear the high byte */
		    inb(mkv2(V2IBUF));		/* Throw this away */
		    _prme = inb(mkv2(V2IBUF));	/* Our address */
		    break;
		}
	    }
	}

/*
 * If we were able to find out hardware address, then set the last
 * byte of the internet address from this value
 */

	if ( _prme != 0)
	    ((union _ipname *)&(pr_net->ip_addr))->in_lst.in_host = _prme;

#ifdef DEBUG
	if (NDEBUG & INFOMSG)
	    if ( _prme != 0)
		printf("LNI address %d\n",_prme);
	    else printf("Could not get LNI address\n");
#endif
/* -------------------- */
/* DDP End changes... */

	/* Setup the interrupt system and the LNI board */
	pr_switch(TRUE);

	/* We should also figure out our local ring address and use this to
		calculate our ring address. This is not trivial: we have to
		put ourself in loopback mode and broadcast a packet to
		ourself and look at the source address.
	*/
	/* DDP - Actually it wasn't that hard! */

	tk_yield();	/* Give the per net task a chance to run. */

#ifdef V2ARP		/* DDP */
	/* init arp */
	prainit();
#endif
	return;
	}


/*
	Routine to switch the board and interrupts on/off.
 */
pr_switch(state)
int state;
{
    int iimr = IIMR;

    if(state) {		/* Turn them on? */
	/* here we should patch into the interrupt vector and initialize
		the v2lni
	*/
	int_off();
	pr_eoi = IEOI + custom.c_intvec; /* calculate the eoi command code */
	pr_int_base = (custom.c_intvec<<2) + pr_int_base_default;
	pr_patch();

	outb(iimr, inb(iimr) & ~(1 << custom.c_intvec));
	int_on();

	/* reset the lni */
	outb(mkv2(V2ICSR), INRST|MODE1|MODE2);
	outb(mkv2(V2OCSR), OUTRST);
	outb(mkv2(V2ILCNT), 0);
	outb(mkv2(V2IHCNT), 0);
	outb(mkv2(V2ICSR), MODE2|MODE1|COPYEN|ININTEN);
    }
    else pr_close();		/* Let pr_close do the work */
}
