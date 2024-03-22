/* Copyright 1988 Bradley N. Davis, Darbick Instructional Software Systems */
/* See permission and disclaimer notice in file "bnd-note.h" */
#include	"bnd-note.h"
/*  Copyright 1986 by Carnegie Mellon  */
/*  See permission and disclaimer notice in file "cmu-note.h"  */
#include	<cmu-note.h>
/*  Copyright 1984 by the Massachusetts Institute of Technology  */
/*  See permission and disclaimer notice in file "notice.h"  */
#include	<notice.h>

/* This header file includes register descriptions of the Ethernet
   boards for the IBM PC based on the Intel 82586 chip (hence known as
   "the ethernet"). */

/* Ports on the ethernet board */
#ifdef LCS8833
#define	ICA		(custom.c_base+0)	/* Attention Address */
#define	IENABLE		(custom.c_base+1)	/* Enable transmitter */
#define	IDISABLE	(custom.c_base+2)	/* Disable transmitter */
#define	IADDR		(custom.c_base+3)	/* 1 byte Ethernet Address */
#endif

#ifdef MI5210
#define	RESET		(custom.c_base+0)
#define	ICA		(custom.c_base+1)	/* Attention Address */
#define	IENABLE		(custom.c_base+3)	/* Enable transmitter */
#define	IDISABLE	(custom.c_base+2)	/* Disable transmitter */
#define IINTENA		(custom.c_base+4)
#define IINTDIS		(custom.c_base+5)
#define	IADDR		(custom.c_base+0)	/* Ethernet Address */
#endif

#define MAKEADDR(off, type)	((type far *)(custom.c_basemem + off))
#define STRIPOFF(ptr)		((unsigned int)ptr & 0xffff)

#define ICX		0x8000
#define IFR		0x4000
#define ICNR		0x2000
#define IRNR		0X1000
#define ICUSMASK	0x700
#define ICIDLE		0x000
#define ICSUSPENDED	0x100
#define ICACTIVE	0x200
#define IRUSMASK	0x70
#define IRIDLE		0x00
#define IRSUSPENDED	0x10
#define IRNORESOURCES	0x20
#define IRREADY		0x40

#define ICUCMASK	0x700
#define ICNOP		0x000
#define ICSTART		0x100
#define ICRESUME	0x200
#define ICSUSPEND	0x300
#define ICABORT		0x400
#define IRUCMASK	0x70
#define IRNCP		0x00
#define IRSTART		0x10
#define IRRESUME	0x20
#define IRSUSPEND	0x30
#define IRABORT		0x40
#define ISRESET		0x80

typedef struct {
	unsigned int status;
	unsigned int command;
	unsigned int cbloff;
	unsigned int rfaoff;
	unsigned int crcerrs;
	unsigned int alnerrs;
	unsigned int rscerrs;
	unsigned int ovrnerrs;
} SCB;

#define	ICOMPLETE	0x8000	/* Command completed */
#define	IBUSY		0x4000	/* Busy block */
#define	IOK		0x2000	/* Error free completion */
#define	IABORT		0x1000	/* Command aborted */
#define ICRCERR		0x800	/* CRC error in received frame */
#define IFAIL		0x800	/* Diagnostics failed */
#define INOCARRIER	0x400	/* No carrier sense signal */
#define IBADALIGN	0x400	/* Alignment error */
#define INOCTS		0x200	/* Lost clear to send */
#define INOSPACE	0x200	/* Ran out of room */
#define IUNDERRUN	0x100	/* Dma underrun */
#define IOVERRUN	0x100	/* Dma overrun */
#define ITRAFFIC	0x80	/* Defer due to traffic */
#define ISHORTFRAME	0x80	/* Fewer bits than minimum */
#define IHEARTBEAT	0x40	/* Heart beat detected */
#define INOEOF		0x40	/* EOF not detected */
#define ICOLL16		0x20	/* 16 collisions experienced */
#define ICOLLISIONS	0xf	/* collision mask */

#define IEOCL		0x8000	/* End of command list */
#define ISUSPEND	0x4000	/* Suspend after completion */
#define IINTERRUPT	0x2000	/* Interrupt after completion */

#define INOP		0x0	/* NOP command */
#define IIASETUP	0x1	/* IA-Setup command */
#define ICONFIGURE	0x2	/* Configure command */
#define IMCSETUP	0x3	/* MC-Setup command */
#define ITRANSMIT	0x4	/* Transmit command */
#define ITDR		0x5	/* TDR command */
#define IDUMP		0x6	/* Dump command */
#define IDIAGNOSE	0x7	/* Diagnose command */

typedef struct {
	unsigned int status;
	unsigned int command;
	unsigned int link;
} COMMAND;

typedef struct {
	COMMAND cmd;
} NOP_COMMAND;

typedef struct {
	COMMAND cmd;
	char myaddr[6];
} IASETUP_COMMAND;

typedef struct {
	COMMAND cmd;
	unsigned int param1;
	unsigned int param2;
	unsigned int param3;
	unsigned int param4;
	unsigned int param5;
	unsigned int param6;
} CONFIGURE_COMMAND;

#define MAXMC	1

typedef struct {
	COMMAND cmd;
	int count;
	char mcaddr[MAXMC][6];
} MCSETUP_COMMAND;

typedef struct {
	COMMAND cmd;
	unsigned int tbdoff;
	char destaddr[6];
	unsigned int length;
} TRANSMIT_COMMAND, TCB;

typedef struct {
	COMMAND cmd;
	unsigned int time;
} TDR_COMMAND;

typedef struct {
	COMMAND cmd;
	unsigned int bufoff;
} DUMP_COMMAND;

typedef struct {
	COMMAND cmd;
} DIAGNOSE_COMMAND;

typedef struct {
	char data[170];
} DUMP_BUFFER;

typedef struct {
	unsigned int count;
	unsigned int tbdoff;
	char far *buffer;
} TBD;

typedef struct {
	COMMAND cmd;
	unsigned int rbdoff;
	char dstaddr[6];
	char srcaddr[6];
	unsigned int length;
} RFD;

typedef struct {
	unsigned int count;
	unsigned int rbdoff;
	char far *buffer;
	unsigned int size;
} RBD;

#define IEOF		0x8000
#define ICOUNTVALID	0x4000
#define ICOUNTMASK	0x3fff
#define ILASTBLK	0x8000
#define ISIZEMASK	0x3fff

#define RBUFSIZE	512
#define TBUFSIZE	1518

#define NIL	0xffff

#define doca()		outb(ICA, 0)
#define Wait_SCB()	while ((SCBPTR->status & ICUSMASK) == ICACTIVE)

#include <int.h>

/* The ethernet address copying function */
#define	iadcpy(a, b)	{ int _i;	\
	for(_i=0; _i<6; _i++) (b)[_i] = (a)[_i];	}

extern NET *i_net;	/* I's net */
extern task *iDemux;	/* I's demultiplexing task */
extern char _ime[6];	/* my ethernet address */
extern char iBROADCAST[6];	/* ethernet broadcast address */

extern IASETUP_COMMAND far *ADDRPTR;
extern MCSETUP_COMMAND far *BROADPTR;
extern TDR_COMMAND far *TDRPTR;
extern DIAGNOSE_COMMAND far *DIAGPTR;
extern DUMP_COMMAND far *DMPPTR;
extern CONFIGURE_COMMAND far *CONFPTR;

extern SCB far *SCBPTR;
extern TBD far *TBDPTR;
extern TCB far *TCBPTR;
extern RFD far *BOTRFD;
extern RBD far *BOTRBD;
