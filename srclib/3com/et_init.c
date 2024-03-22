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
#include <timer.h>
#include "3com.h"

/* This C routine does as much of the initialization at a high level as
	it can. It uses the following routines from the -lpc library:

		char inb(port)	to input a byte;
		unsigned inw(port)	to input a word. Gets the low byte
					from port, high byte from port+1;
		outb(port, byte)	to output a byte to a port;
		outw(port, word)	outputs low byte to port, hi byte to
					port+1;

	These routines work well with the ethernet controller but not with
	the 8237A DMA chip which wants a different method of handling words.
	The routines outw2() and inw2() are for use with it.
*/

/* Jan-85 Added task to keep the ethernet controller alive. See
	comments in et_send.c.		<Saltzer, Romkey>
30-Mar-86 Added et_switch routine and rearrange et_init to call it.
					<Drew D. Perkins>
10-Dec-86 Fixed bugs to allow BOOTP/UDP broadcasts to work.  A very
	strange problem with the board made sends of broadcast
	packets > 114 bytes fail mysteriously.
*/

/* define some convenient constants */
/* receive all packets */
#define RCVALL	ERCVALLADDR | EACCDRIBBLE | EACCGOODFRAMES | EDTFCSERR | EDTSHORTFRAMES | EDTOVRFLOW | EDTNOOVERFLOW
/* Receive multicast packets */
#define RCVMULT ERCVMULTI | EACCDRIBBLE | EACCGOODFRAMES | EDTSHORTFRAMES | EDTOVRFLOW
/* Receive broadcast, detect all errors */
#define RCVNORM ERCVBROAD | EACCDRIBBLE | EACCGOODFRAMES | EDTSHORTFRAMES | EDTOVRFLOW
/* loopback and DMA/interrupts */
#define TXLOOP	EINTDMAENABLE | ELOOPBACK

/* storage for lots of things like my ethernet address, the ethernet broadcast
	address and my task and net pointers
*/
char ETBROADCAST[6] = { 0xff, 0xff, 0xff, 0xff, 0xff, 0xff};
char _etme[6];		/* my ethernet address */
task *EtDemux;		/* ethernet packet demultiplexing task */
NET *et_net;		/* my net pointer */
char etrcvcmd;		/* receiver command byte */
char save_mask; 	/* receiver command on entry. */
unsigned et_eoi;
static char et_msgid[] = "3COM adapter";
unsigned etrreset = 0;

int et_demux(); 	/* the routine which is the body of the demux task */
static int et_poke();
static int et_keepalive();
static task *e_rtk;
static timer *e_rtm;
extern unsigned etint;

et_init(net, options, dummy)
	NET *net;
	unsigned options;
	unsigned dummy; {

#ifdef	DEBUG
	if(NDEBUG & INFOMSG)
		printf("Forking ETDEMUX.\n");
#endif

	EtDemux = tk_fork(tk_cur, et_demux, net->n_stksiz, "ETDEMUX", net);
	if(EtDemux == NULL) {
		printf("Error: %s setup failed\n", et_msgid);
		exit(1);
		}

	et_net = net;
	et_net->n_demux = EtDemux;

	et_switch(1, options);		/* DDP */

	/* start up a task which periodically kicks the receiver to
		keep it alive
	*/
	e_rtm = tm_alloc();
	if(e_rtm == NULL) {
		printf("Error: %s timer setup failed\n", et_msgid);
		exit(1);
		}

	e_rtk = tk_fork(tk_cur, et_keepalive, 400, "Keepalive", 0);
	if(e_rtk == NULL) {
		printf("Error: %s keepalive setup failed\n", et_msgid);
		exit(1);
		}

	/* Now everything is initialized. The DMA channel should only be
		initialized on demand, so it's not necessary to touch it
		now.
	*/
	tk_yield();	/* Give the per net task a chance to run. */

	/* init arp */
	etainit();
	return;
	}

static et_poke() {
	tk_wake(e_rtk);
	}

static et_keepalive() {
	unsigned int_cnt;
	while(1) {
		tm_set(3, et_poke, NULL, e_rtm);
		tk_block();
		int_cnt = etint;
		et_ihnd();
		if (int_cnt != etint) ++etrreset;
		}
	}


/*
	Routine to switch the board and interrupts on/off.
 */
et_switch(state, options)
int state;
unsigned options;
{
	union {
		long ulong;
		char uc[4];
		} myaddr;
	register int i;
	int vec;
	char temp;

    if(state) { 	/* Turn them on? */
	int_off();		/* Disable interrupts. */

	/* Reset the ethernet controller. */
	outb(EAUXCMD, ERESET);
	outb(EAUXCMD, ESYSBUS);	/* DDP - Clear reset */

	/* patch in the new interrupt handler - rather, call the routine to
		do this. This routine saves the old contents of the vector.
	*/
	et_eoi = 0x60 + custom.c_intvec;
	et_patch(custom.c_intvec<<2);

	/* setup interrupts for the specified line */
	vec = (1 << custom.c_intvec);
	save_mask = inb(IIMR) & vec;
	outb(IIMR, inb(IIMR) & ~vec);

	/* We need to setup our ethernet address. Do this by reading the
		address from the PROM and writing it back to the controller.
	*/

	switch(custom.c_seletaddr) {
	case HARDWARE:
		for(i=0; i<6; i++) {
			outw(EGPPLOW, i);
			temp = inb(EADDRWIN);
			_etme[i] = temp;
			outb(EADDR+i, temp);
			}
		break;
	case ETINTERNET:
		myaddr.ulong =	et_net->ip_addr;
		for(i=3; i != -1; i--) {
			_etme[i+2] = myaddr.uc[i];
			outb(EADDR+i+2, myaddr.uc[i]);
			}
		_etme[0] = 0;
		outb(EADDR, 0);
		_etme[1] = 0;
		outb(EADDR+1, 0);
		break;
	case ETUSER:
		for(i=0; i<6; i++) {
			_etme[i] = custom.c_myetaddr.e_ether[i];
			outb(EADDR+i, _etme[i]);
			}
		break;
	default:
#ifdef DEBUG
		printf("Invalid %s address selection option\n", et_msgid);
#endif
		;
	}

	/* turn interrupts on to try to eliminate an insidious race
	   condition. */
	int_on();

	outb(EAUXCMD, EINTDMAENABLE|ESYSBUS);

	/* Initialize the transmitter to not interrupt us at all */
	outb(ETXCMD, 0);

	/* Initialize the receiver to detect no errors and accept
		good packets and dribble errors. */

	if(options & ALLPACK) etrcvcmd = RCVALL;
	else if(options & MULTI) etrcvcmd = RCVMULT;
	else etrcvcmd = RCVNORM;

	/* set up the receiver once */
	outb(ERCVCMD, etrcvcmd);
	inb(ERCVCMD);		/* staleify the receiver status */
	outb(ECLRRP, 0);

	outb(EAUXCMD, EINTDMAENABLE|ERECEIVE);

#ifdef	DEBUG
    if(NDEBUG & INFOMSG) {
	printf("PC Ethernet address = ");
	for (i=0; i<6; i++)
	    printf("%02x", _etme[i]&0xff);
	printf ("\n");
	}
#endif
    }
    else et_close();	/* Let et_close do the work */
}
