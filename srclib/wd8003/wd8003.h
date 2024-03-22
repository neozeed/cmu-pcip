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
   boards for the IBM PC based on the National Semiconductor dp8930
   chip (hence known as "the ethernet"). */

/* Ports on the ethernet board */
#define	WD8base		(custom.c_base)
#define MEMADDR		(custom.c_basemem)

#define MAKEADDR(off)	((char far *)(MEMADDR + off))
#define STRIPOFF(ptr)	((unsigned int)ptr & 0xffff)

/* #define IO_BASE_OF_WD8003	0x0280	*/	/* default base */
#define MAX_WD8003_DATA	   	1518	/* should be other value,chip depnendent value */

#define W83CREG		0x00	/* I/O port definition */

#define MSK_RESET	0x80	/* W83CREG masks */
#define MSK_ENASH	0x40
#define MSK_DECOD      	0x3F	/* ???? memory decode bits, corresponding */
				/* to SA 18-13. SA 19 assumed to be 1 */


#define	ADDROM		0x08

#define	CMDR		0x10	
#define CLDA0  	0x11		/* current local dma addr 0 for read */
#define CLDA1  	0x12		/* current local dma addr 1 for read */
#define BNRY   	0x13		/* boundary reg for rd and wr */
#define TSR    	0x14		/* tx status reg for rd */
#define NCR    	0x15		/* number of collision reg for rd */
#define FIFO   	0x16		/* FIFO for rd */
#define ISR    	0x17		/* interrupt status reg for rd and wr */
#define CRDA0  	0x18		/* current remote dma address 0 for rd */
#define CRDA1  	0x19		/* current remote dma address 1 for rd */
#define RSR    	0x1C		/* rx status reg for rd */
#define CNTR0  	0x1D		/* tally cnt 0 for frm alg err for rd */
#define CNTR1  	0x1E		/* tally cnt 1 for crc err for rd */
#define CNTR2  	0x1F		/* tally cnt 2 for missed pkt for rd */

#define PAR   	0x11		/* physical addr reg base for rd and wr */
#define CURR   	0x17		/* current page reg for rd and wr */
#define MAR   	0x18		/* multicast addr reg base fro rd and WR */
#define MARsize	8		/* size of multicast addr space */

#define PSTART 	0x11		/* page start register for write */
#define PSTOP  	0x12		/* page stop register for write */
#define TPSR   	0x14		/* tx start page start reg for wr */
#define RCR    	0x1C		/* rx configuration reg for wr */
#define TCR    	0x1D		/* tx configuration reg for wr */
#define DCR    	0x1E		/* data configuration reg for wr */
#define IMR    	0x1F		/* interrupt mask reg for wr */

#define TBCR0  	0x15		/* tx byte count 0 reg for wr */
#define TBCR1  	0x16		/* tx byte count 1 reg for wr */
#define RSAR0  	0x18		/* remote start address reg 0  for wr */
#define RSAR1  	0x19		/* remote start address reg 1 for wr */
#define RBCR0  	0x1A		/* remote byte count reg 0 for wr */
#define RBCR1  	0x1B		/* remote byte count reg 1 for wr */

#define MSK_STP		0x01	/* 8390 CMDR masks */
#define MSK_STA		0x02	  
#define MSK_TXP	       	0x04	/* initial txing of a frm */
#define MSK_RD2		0x20
#define MSK_PG0	       	0x00	/* select register page 0 */
#define MSK_PG1	       	0x40	/* select register page 1 */
#define MSK_PG2	       	0x80	/* select register page 2 */

#define MSK_PRX 	0x01	/* rx with no error */
#define MSK_PTX 	0x02	/* tx with no error */
#define MSK_RXE 	0x04	/* rx with error */
#define MSK_TXE 	0x08	/* tx with error */
#define MSK_OVW 	0x10	/* overwrite warning */
#define MSK_CNT 	0x20	/* MSB of one of the tally counters is set */
#define MSK_RDC 	0x40	/* remote dma completed */
#define MSK_RST		0x80	/* reset state indicator */

#define MSK_WTS		0x01	/* word transfer mode selection */
#define MSK_BOS		0x02	/* byte order selection */
#define MSK_LAS		0x04	/* long addr selection */
#define MSK_BMS		0x08	/* burst mode selection */
#define MSK_ARM		0x10	/* autoinitialize remote */
#define MSK_FT00	0x00	/* burst lrngth selection */
#define MSK_FT01	0x20	/* burst lrngth selection */
#define MSK_FT10	0x40	/* burst lrngth selection */
#define MSK_FT11	0x60	/* burst lrngth selection */

#define MSK_SEP		0x01	/* save error pkts */
#define MSK_AR	 	0x02	/* accept runt pkt */
#define MSK_AB		0x04	/* 8390 RCR */
#define MSK_AM	 	0x08	/* accept multicast  */
#define MSK_PRO		0x10	/* promiscuous physical */
				/* accept all pkt with physical adr */
#define MSK_MON	0x20		/* monitor mode */

#define MSK_CRC		0x01	/* inhibit CRC, do not append crc */
#define MSK_LB01	0x06	/* encoded loopback control */
#define MSK_ATD		0x08	/* auto tx disable */
#define MSK_OFST	0x10	/* collision offset enable  */

#define SMK_PRX 	0x01	/* rx without error */
#define SMK_CRC 	0x02	/* CRC error */
#define SMK_FAE 	0x04	/* frame alignment error */
#define SMK_FO  	0x08	/* FIFO overrun */
#define SMK_MPA 	0x10	/* missed pkt */
#define SMK_PHY 	0x20	/* physical/multicase address */
#define SMK_DIS 	0x40	/* receiver disable. set in monitor mode */
#define SMK_DEF		0x80	/* deferring */

#define SMK_PTX 	0x01	/* tx without error */
#define SMK_DFR 	0x02	/* non deferred tx */
#define SMK_COL 	0x04	/* tx collided */
#define SMK_ABT 	0x08	/* tx aboort because of excessive collisions */
#define SMK_CRS 	0x10	/* carrier sense lost */
#define SMK_FU  	0x20	/* FIFO underrun */
#define SMK_CDH 	0x40	/* collision detect heartbeat */
#define SMK_OWC		0x80	/* out of window collision */

#define TB_SIZE		0x02	/* number of tx buff in shard memory */

/* The ethernet address copying function */
#define	wadcpy(a, b)	{ int _i;	\
	for(_i=0; _i<6; _i++) (b)[_i] = (a)[_i];	}

#define PRIVATE static
#define PUBLIC

/* DDP - Plummer's internals. All constants are already byte-swapped. */
#define	ARETHx	0x1		/* DDP - ethernet hardware type */
/* Plummer's internals. All constants are already byte-swapped. */
#define ARETH	0x100		/* ethernet hardware type */
#define ARIP	ET_IP		/* internet protocol type */
#define ARREQ	0x100		/* byte swapped request opcode */
#define ARREP	0x200		/* byte swapped reply opcode */

#define WADDR	(WD8base+ADDROM)

#define MAXENT	16
#define RECORD_LENGTH	64
#define MAXMEM		(8*1024)
#define START_PAGE	0
#define PAGE_LEN	256
