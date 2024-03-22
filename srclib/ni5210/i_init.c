/* Copyright 1988 Bradley N. Davis, Darbick Instructional Software Systems */
/* See permission and disclaimer notice in file "bnd-note.h" */
#include	"bnd-note.h"
/*  Copyright 1986 by Carnegie Mellon  */
/*  See permission and disclaimer notice in file "cmu-note.h"  */
#include	<cmu-note.h>
/*  Copyright 1984, 1985 by the Massachusetts Institute of Technology  */
/*  See permission and disclaimer notice in file "notice.h"  */
#include	<notice.h>

#include <types.h>
#include <task.h>
#include <q.h>
#include <netq.h>
#include <net.h>
#include <custom.h>
#include <netbuf.h>
#include <stdio.h>
#include <ether.h>
#include <timer.h>
#include "i82586.h"

/* This C routine does as much of the initialization at a high level as
	it can.
*/

#ifdef LCS8833
#define MAXMEM	(16*1024)
#define RFDCOUNT	10
static char i_msgid[] = "Longshine LCS-8833n adapter";
#endif

#ifdef MI5210
#define MAXMEM	(8*1024)
#define RFDCOUNT	4
static char i_msgid[] = "Micom/Interlan 5210a adapter";
#endif

/* define some convenient constants */
/* storage for lots of things like my ethernet address, the ethernet broadcast
	address and my task and net pointers
*/
char iBROADCAST[6] = { 0xff, 0xff, 0xff, 0xff, 0xff, 0xff};
char _default[6] = { 0x70, 0x00, 0x70, 0x01, 0x02, 0x00};
char _ime[6];		/* my ethernet address */
task *iDemux;		/* ethernet packet demultiplexing task */
NET *i_net;		/* my net pointer */
char ircvcmd;		/* receiver command byte */
char save_mask; 	/* receiver command on entry. */
unsigned i_eoi;
unsigned irreset = 0;

int i_demux(); 	/* the routine which is the body of the demux task */
static int i_poke();
static int i_keepalive();
static task *e_rtk;
static timer *e_rtm;
extern unsigned iint;

i_init(net, options, dummy)
	NET *net;
	unsigned options;
	unsigned dummy; {

#ifdef	DEBUG
	if(NDEBUG & INFOMSG)
		printf("Forking iDEMUX.\n");
#endif

	iDemux = tk_fork(tk_cur, i_demux, net->n_stksiz, "iDEMUX", net);
	if(iDemux == NULL) {
		printf("Error: %s setup failed\n", i_msgid);
		exit(1);
		}

	i_net = net;
	i_net->n_demux = iDemux;

	i_switch(1, options);		/* DDP */

	/* start up a task which periodically kicks the receiver to
		keep it alive
	*/
	e_rtm = tm_alloc();
	if(e_rtm == NULL) {
		printf("Error: %s timer setup failed\n", i_msgid);
		exit(1);
		}

	e_rtk = tk_fork(tk_cur, i_keepalive, 400, "Keepalive", 0);
	if(e_rtk == NULL) {
		printf("Error: %s keepalive setup failed\n", i_msgid);
		exit(1);
		}

	/* Now everything is initialized. The DMA channel should only be
		initialized on demand, so it's not necessary to touch it
		now.
	*/
	tk_yield();	/* Give the per net task a chance to run. */

	/* init arp */
	iainit();
	}

static i_poke() {
	tk_wake(e_rtk);
	}

static i_keepalive() {
	unsigned int_cnt;
	while(1) {
		tm_set(3, i_poke, NULL, e_rtm);
		tk_block();
		int_cnt = iint;
		i_ihnd(1);
		if (int_cnt != iint) ++irreset;
		}
	}


#define iALLOC(ptr, type)	ptr = MAKEADDR((TOPMEM -= sizeof(type)), type)
#define sALLOC(ptr, size)	ptr = MAKEADDR((TOPMEM -= size), char)
#define tALLOC(ptr, size)	ptr = (TOPMEM -= size)
static unsigned int TOPMEM;

typedef struct {
	unsigned int status;
	unsigned int dummy1;
	unsigned int dummy2;
	unsigned int iscpoff;
	unsigned int iscpseg;
} SCP;

typedef struct {
	unsigned int status;
	unsigned int scboff;
	unsigned int base1;
	unsigned int base2;
} ISCP;

/*
	Routine to switch the board and interrupts on/off.
 */
i_switch(state, options)
int state;
unsigned options;
{
	union {
		long ulong;
		char uc[4];
		} myaddr;
	register int i;
	int vec;
	unsigned int temp;
	RFD far *RFDPTR;
	RBD far *RBDPTR;
	SCP far *scp;
	ISCP far *iscp;

	if (!state) { 	/* Turn them on? */
		i_close();	/* Let i_close do the work */
		return;
	}
	/* We need to setup our ethernet address. Do this by reading the
		address from the switches.
	*/

	switch(custom.c_seletaddr) {
	case HARDWARE:
#ifdef LCS8833
		for(i=0; i<5; i++)
			_ime[i] = _default[i];
		_ime[5] = inb(IADDR);
#endif
#ifdef MI5210
		for(i=0; i<6; i++)
			_ime[i] = inb(IADDR+i);
#endif
		break;
	case ETINTERNET:
		myaddr.ulong =	i_net->ip_addr;
		for(i=3; i != -1; i--)
			_ime[i+2] = myaddr.uc[i];
		_ime[0] = 0;
		_ime[1] = 0;
		break;
	case ETUSER:
		for(i=0; i<6; i++)
			_ime[i] = custom.c_myetaddr.e_ether[i];
		break;
	default:
#ifdef DEBUG
		printf("Invalid %s address selection option\n", i_msgid);
#endif
		;
	}
	TOPMEM = MAXMEM;

	/* allocate an SCP */
	iALLOC(scp, SCP);
	scp->status = 1;
	scp->dummy1 = 0;
	scp->dummy2 = 0;
	scp->iscpseg = 0;
	/* allocate an ISCP */
	iALLOC(iscp, ISCP);
	scp->iscpoff = STRIPOFF(iscp);
	iscp->status = 1;
	iscp->scboff = 0;
	iscp->base1 = iscp->base2 = 0;

	/* allocate an SCB */
	iALLOC(SCBPTR, SCB);
	iscp->scboff = STRIPOFF(SCBPTR);
	SCBPTR->status = 0;
	SCBPTR->command = ISRESET;
	SCBPTR->crcerrs = 0;
	SCBPTR->alnerrs = 0;
	SCBPTR->rscerrs = 0;
	SCBPTR->ovrnerrs = 0;
	doca();

	/* allocate a CONFIGURE */
	iALLOC(CONFPTR, CONFIGURE_COMMAND);
	CONFPTR->cmd.status = 0;
	CONFPTR->cmd.command = ICONFIGURE | IEOCL;
	CONFPTR->cmd.link = NIL;
	CONFPTR->param1 = 0x080C;	/* fifo=8, byte count=C */
	CONFPTR->param2 = 0x2E00;	/* important! Addr (AL) not inserted on the fly! */
	CONFPTR->param3 = 0x6000;	/* IFS = 60h */
	CONFPTR->param4 = 0xF200;	/* retry=F, slot time=200h */
	CONFPTR->param5 = 0x0000;	/* flags, set to 1 for promiscuous */
	CONFPTR->param6 = 0x0040;	/* min frame length=40h */

	/* allocate an IA-SETUP */
	iALLOC(ADDRPTR, IASETUP_COMMAND);
	ADDRPTR->cmd.status = 0;
	ADDRPTR->cmd.command = IIASETUP | IEOCL;
	ADDRPTR->cmd.link = NIL;
	iadcpy(_ime, ADDRPTR->myaddr);

	/* allocate an MC-SETUP */
	iALLOC(BROADPTR, MCSETUP_COMMAND);
	BROADPTR->cmd.status = 0;
	BROADPTR->cmd.command = IMCSETUP | IEOCL;
	BROADPTR->cmd.link = NIL;
	BROADPTR->count = 1;
	iadcpy(iBROADCAST, BROADPTR->mcaddr[0]);

	/* allocate a TCB */
	iALLOC(TCBPTR, TCB);
	TCBPTR->cmd.status = 0;
	TCBPTR->cmd.command = ITRANSMIT | IEOCL;
	TCBPTR->cmd.link = NIL;

	/* allocate a TBD and a TRANSMIT BUFFER */
	iALLOC(TBDPTR, TBD);
	TBDPTR->count = IEOF;
	TBDPTR->tbdoff = NIL;
	sALLOC(TBDPTR->buffer, TBUFSIZE);
	TCBPTR->tbdoff = STRIPOFF(TBDPTR);

	/* allocate a TDR */
	iALLOC(TDRPTR, TDR_COMMAND);
	TDRPTR->cmd.status = 0;
	TDRPTR->cmd.command = ITDR | IEOCL;
	TDRPTR->cmd.link = NIL;
	TDRPTR->time = 0;

	/* allocate a DUMP (with dump space) */
	iALLOC(DMPPTR, DUMP_COMMAND);
	DMPPTR->cmd.status = 0;
	DMPPTR->cmd.command = IDUMP | IEOCL;
	DMPPTR->cmd.link = NIL;
	tALLOC(DMPPTR->bufoff, sizeof(DUMP_BUFFER));

	/* allocate a DIAGNOSE */
	iALLOC(DIAGPTR, DIAGNOSE_COMMAND);
	DIAGPTR->cmd.status = 0;
	DIAGPTR->cmd.command = IDIAGNOSE | IEOCL;
	DIAGPTR->cmd.link = NIL;

	/* allocate 10 RFD's */
	temp = NIL;
	for (i=0; i < RFDCOUNT; i++) {
		iALLOC(RFDPTR, RFD);
		if (temp == NIL) BOTRFD = RFDPTR;
		RFDPTR->cmd.status = 0;
		RFDPTR->cmd.command = 0;
		RFDPTR->cmd.link = temp;
		temp = STRIPOFF(RFDPTR);
	}
	SCBPTR->rfaoff = temp;
	BOTRFD->cmd.command |= ILASTBLK;

	/* allocate RBD's and RECEIVE BUFFER's to fill rest of memory */
	temp = NIL;
	while (TOPMEM >= sizeof(RBD) + RBUFSIZE) {
		iALLOC(RBDPTR, RBD);
		if (temp == NIL) BOTRBD = RBDPTR;
		RBDPTR->count = 0;
		RBDPTR->rbdoff = temp;
		sALLOC(RBDPTR->buffer, RBUFSIZE);
		RBDPTR->size = RBUFSIZE;
		temp = STRIPOFF(RBDPTR);
	}
	RFDPTR->rbdoff = temp;
	BOTRBD->size |= ILASTBLK;

	for (temp=0; temp < 0xff00; temp++)
		if (SCBPTR->command == 0) break;
	if (temp >= 0xff00) {
		printf("Error: board not responding\n");
		exit(1);
	}

	/* execute CONFIGURE */
	Wait_SCB();
	SCBPTR->cbloff = STRIPOFF(CONFPTR);
	SCBPTR->command = ICSTART;
	doca();

	/* execute IASETUP */
	Wait_SCB();
	SCBPTR->cbloff = STRIPOFF(ADDRPTR);
	SCBPTR->command = ICSTART;
	doca();

	/* execute MCSETUP */
	Wait_SCB();
	SCBPTR->cbloff = STRIPOFF(BROADPTR);
	SCBPTR->command = ICSTART;
	doca();

	/* enable transceiver */
	outb(IENABLE, 0);

#ifdef MI5210
	outb(IINTDIS, 0);
#endif
	int_off();		/* Disable interrupts. */

	/* patch in the new interrupt handler - rather, call the routine to
		do this. This routine saves the old contents of the vector.
	*/
	i_eoi = 0x60 + custom.c_intvec;
	i_patch(custom.c_intvec<<2);

	/* setup interrupts for the specified line */
	vec = (1 << custom.c_intvec);
	save_mask = inb(IIMR) & vec;
	outb(IIMR, inb(IIMR) & ~vec);

	/* turn interrupts on to try to eliminate an insidious race
	   condition. */
	int_on();
#ifdef MI5210
	outb(IINTENA, 0);
#endif

	/* start receiver */
	Wait_SCB();
	SCBPTR->cbloff = NIL;
	SCBPTR->command = IRSTART | IFR | ICX | ICNR | IRNR;
	doca();

#ifdef	DEBUG
	if(NDEBUG & INFOMSG) {
		printf("PC Ethernet address = ");
		for (i=0; i<6; i++)
			printf("%02x", _ime[i]&0xff);
		printf ("\n");
	}
#endif
}

i_debug()
{
	int i;
	RFD far *RFDPTR;
	RBD far *RBDPTR;

	/* dump SCP */
	/* dump iscp */
	/* dump SCBPTR */
	printf("SCB %x %x %x %x %x %d %d %d %d\n", STRIPOFF(SCBPTR),
		SCBPTR->status, SCBPTR->command, SCBPTR->cbloff,
		SCBPTR->rfaoff, SCBPTR->crcerrs, SCBPTR->alnerrs,
		SCBPTR->rscerrs, SCBPTR->ovrnerrs);
	/* dump ADDRPTR */
	printf("ADR %x %x %x %x\n\t", STRIPOFF(ADDRPTR), ADDRPTR->cmd.status,
		ADDRPTR->cmd.command, ADDRPTR->cmd.link);
	for (i=0; i < 6; i++)
		printf("%x:", ADDRPTR->myaddr[i] & 0xff);
	printf("\n");
	/* dump CONFPTR */
	printf("CNF %x %x %x %x %x %x %x %x %x %x\n", STRIPOFF(CONFPTR),
		CONFPTR->cmd.status, CONFPTR->cmd.command, CONFPTR->cmd.link,
		CONFPTR->param1, CONFPTR->param2, CONFPTR->param3,
		CONFPTR->param4, CONFPTR->param5, CONFPTR->param6);
	/* dump BROADPTR */
	printf("BRD %x %x %x %x %d\n\t", STRIPOFF(BROADPTR),
		BROADPTR->cmd.status, BROADPTR->cmd.command,
		BROADPTR->cmd.link, BROADPTR->count);
	for (i=0; i < 6; i++)
		printf("%x:", BROADPTR->mcaddr[0][i]& 0xff);
	printf("\n");
	/* dump TCB */
	printf("TCB %x %x %x %x %x\n\t", STRIPOFF(TCBPTR), TCBPTR->cmd.status,
		TCBPTR->cmd.command, TCBPTR->cmd.link, TCBPTR->tbdoff); 
	for (i=0; i < 6; i++)
		printf("%x:", TCBPTR->destaddr[i]& 0xff);
	printf("\n\t%d\n", TCBPTR->length);
	/* dump TBD */
	printf("TBD %x %x %x %x\n", STRIPOFF(TBDPTR), TBDPTR->count,
		TBDPTR->tbdoff, STRIPOFF(TBDPTR->buffer));
	/* dump TDR */
	printf("TDR %x %x %x %x %d\n", STRIPOFF(TDRPTR), TDRPTR->cmd.status,
		TDRPTR->cmd.command, TDRPTR->cmd.link, TDRPTR->time);
	/* dump DMPPTR */
	printf("DMP %x %x %x %x %x\n", STRIPOFF(DMPPTR), DMPPTR->cmd.status,
		DMPPTR->cmd.command, DMPPTR->cmd.link, DMPPTR->bufoff);
	/* dump DIAGPTR */
	printf("DIG %x %x %x %x\n", STRIPOFF(DIAGPTR), DIAGPTR->cmd.status,
		DIAGPTR->cmd.command, DIAGPTR->cmd.link);
	/* dump RFDPTR list */
	RFDPTR = MAKEADDR(SCBPTR->rfaoff, RFD);
	RBDPTR = MAKEADDR(RFDPTR->rbdoff, RBD);
	while (STRIPOFF(RFDPTR) != NIL) {
		printf("RFD %x %x %x %x %x\n\t", STRIPOFF(RFDPTR),
			RFDPTR->cmd.status, RFDPTR->cmd.command,
			RFDPTR->cmd.link, RFDPTR->rbdoff);
		for (i=0; i < 6; i++)
			printf("%x:", RFDPTR->dstaddr[i] & 0xff);
		printf("\n\t");
		for (i=0; i < 6; i++)
			printf("%x:", RFDPTR->srcaddr[i] & 0xff);
		printf("\n\t%d\n", RFDPTR->length);
		RFDPTR = MAKEADDR(RFDPTR->cmd.link, RFD);
	}
	printf("BOTRFD %x\n", STRIPOFF(BOTRFD));
	/* dump RBDPTR list */
	while (STRIPOFF(RBDPTR) != NIL) {
		printf("RBD %x %x %x %x %x\n", STRIPOFF(RBDPTR),
			RBDPTR->count, RBDPTR->rbdoff,
			STRIPOFF(RBDPTR->buffer), RBDPTR->size);
		RBDPTR = MAKEADDR(RBDPTR->rbdoff, RBD);
	}
	printf("BOTRBD %x\n", STRIPOFF(BOTRBD));
}
