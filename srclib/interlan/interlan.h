/* Copyright 1986 by Carnegie Mellon */
/*  Copyright 1986 by Carnegie Mellon  */
/*  See permission and disclaimer notice in file "cmu-note.h"  */
#include	<cmu-note.h>
/* register definitions for the Micom-Interlan NI5010 ethernet card.
*/

/* I/O addresses for registers */
#define	XMIT_STAT	(custom.c_base + 0x00)
#define	CLR_XMIT_INT	(custom.c_base + 0x00)
#define	XMIT_MASK	(custom.c_base + 0x01)
#define	RCV_STAT	(custom.c_base + 0x02)
#define	CLR_RCV_INT	(custom.c_base + 0x02)
#define	RCV_MASK	(custom.c_base + 0x03)
#define	XMIT_MODE	(custom.c_base + 0x04)
#define	RCV_MODE	(custom.c_base + 0x05)
#define	RESET		(custom.c_base + 0x06)
#define	TDR1		(custom.c_base + 0x07)
#define	NODE_ID_0	(custom.c_base + 0x08)
#define	NODE_ID_1	(custom.c_base + 0x09)
#define	NODE_ID_2	(custom.c_base + 0x0a)
#define	NODE_ID_3	(custom.c_base + 0x0b)
#define	NODE_ID_4	(custom.c_base + 0x0c)
#define	NODE_ID_5	(custom.c_base + 0x0d)
#define	TDR2		(custom.c_base + 0x0f)
#define	RCV_CNT_LO	(custom.c_base + 0x10)
#define	M_START_LO	(custom.c_base + 0x10)
#define	RCV_CNT_HI	(custom.c_base + 0x11)
#define	M_START_HI	(custom.c_base + 0x11)
#define	M_MODE		(custom.c_base + 0x12)
#define	INT_STAT	(custom.c_base + 0x13)
#define	RESET_DMA	(custom.c_base + 0x13)
#define	RCV_BUF		(custom.c_base + 0x14)
#define	XMIT_BUF	(custom.c_base + 0x15)
#define	ETHER_ADDR	(custom.c_base + 0x16)
#define	RESET_ALL	(custom.c_base + 0x17)

/* transmit status bits */
/* read */
#define	XS_TPOK		0x80		/* transmit packet successful */
#define	XS_CS		0x40		/* carrier sense */
#define	XS_RCVD		0x20		/* transmitted packet received */
#define	XS_SHORT	0x10		/* ether is shorted */
#define	XS_UFLW		0x08		/* transmitter failure */
#define	XS_COLL		0x04		/* collision occurred */
#define	XS_16COLL	0x02		/* sixteenth collision occurred */
#define	XS_PERR		0x01		/* parity error */
/* write */
#define	XS_CLR_UFLW	0x08		/* clear underflow */
#define	XS_CLR_COLL	0x04		/* clear collision */
#define	XS_CLR_16COLL	0x02		/* clear sixteenth collision */
#define	XS_CLR_PERR	0x01		/* clear parity error */

/* transmit mask register */
#define	XM_TPOK		0x80		/* int on successful xmit */
#define	XM_RCVD		0x20		/* int on xmitted packet rcvd */
#define	XM_UFLW		0x08		/* int on underflow */
#define	XM_COLL		0x04		/* int on collision */
#define	XM_16COLL	0x02		/* int on 16th collision */
#define	XM_PERR		0x01		/* int on parity error */

/* receive status register */
/* read */
#define	RS_PKT_OK	0x80		/* received good packet */
#define	RS_RST_PKT	0x10		/* received reset packet (type 0x0900) */
#define	RS_RUNT		0x08		/* received short packet */
#define	RS_ALIGN	0x04		/* received extra bits */
#define	RS_CRC		0x02		/* received packet with CRC error */
#define	RS_OVERFLOW	0x01		/* packet overflowed buffer */
/* write */
#define	RS_CLR_PKT_OK	0x80
#define	RS_CLR_RST_PKT	0x10
#define	RS_CLR_RUNT	0x08
#define	RS_CLR_ALIGN	0x04
#define	RS_CLR_CRC	0x02
#define	RS_CLR_OVERFLOW	0x01

/* receive mask register */
#define	RM_PKT_OK	0x80
#define	RM_RST_PKT	0x10
#define	RM_RUNT		0x08
#define	RM_ALIGN	0x04
#define	RM_CRC		0x02
#define	RM_OVERFLOW	0x01

/* transmit mode register */
#define	XMD_IG_PAR	0x08		/* ignore parity */
#define	XMD_T_MODE	0x04		/* transceiver power */
#define	XMD_LBC		0x02		/* loopback */
#define	XMD_DIS_C	0x01		/* disable contention */

/* receive mode register */
#define	RMD_TEST	0x80		/* chip test */
#define	RMD_ADD_SIZE	0x10		/* address = 5 or 6 bytes */
#define	RMD_EN_RUNT	0x08		/* enable runt reception */
#define	RMD_EN_RST	0x04		/* enable reset reception */
/* address matching */
#define	RMD_NO_PACKETS	0x00		/* disable receiver */
#define	RMD_BROADCAST	0x01		/* accept some multicast */
#define	RMD_MULTICAST	0x02		/* accept all multicast */
#define	RMD_ALL_PACKETS	0x03		/* accept all packets */

/* reset register */
#define	RS_RESET	0x80

/* memory mode register */
#define	MM_EN_DMA	0x80		/* enable DMA */
#define	MM_EN_RCV	0x40		/* enable receiving */
#define	MM_EN_XMT	0x20		/* begin transmitting */
#define	MM_BUS_PAGE	0x00		/* buffer page for bus */
#define	MM_NET_PAGE	0x00		/* net page for bus */
#define	MM_MUX		0x01		/* 0 = xmit buffer on bus;	*/
					/* 1 = rcv  buffer on bus	*/

/* interrupt status register */
#define	IN_TDIAG	0x80		/* diagnostic bit */
#define	IN_ENRCV	0x20		/* =0 -> rcv complete */
#define	IN_ENXMIT	0x10		/* =0 -> tx complete */
#define	IN_ENDMA	0x08		/* =0 -> dma complete */
#define	IN_DMAINT	0x04		/* dma interrupt */
#define	IN_RINT		0x02		/* receiver interrupt */
#define	IN_TINT		0x01		/* transmitter interrupt */

/* The ethernet address copying function */
#define	etadcpy(a, b)	{ int _i;	\
	for(_i=0; _i<6; _i++) (b)[_i] = (a)[_i];	}

/* some shared variables */
extern char ETBROADCAST[6];
extern char _etme[6];

extern unsigned il_transmitting;
extern unsigned il_received;
