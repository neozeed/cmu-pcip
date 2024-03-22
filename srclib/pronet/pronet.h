/* Copyright 1986 by Carnegie Mellon */
/*  Copyright 1986 by Carnegie Mellon  */
/*  See permission and disclaimer notice in file "cmu-note.h"  */
#include	<cmu-note.h>
/*  Copyright 1984, 1985 by Proteon, Inc. */
/*  See permission and disclaimer notice in file "proteon-notice.h"  */
#include	"proteon-notice.h"

/* proNET ring driver header file - defines packet header format
	and some constants
 *
 ***********************************************************************
 *HISTORY
 * 23-Mar-86, Drew D. Perkins (ddp), Carnegie-Mellon University
 *  Merged in newer MIT sources.
 *
 * 13-Jun-85, Drew D. Perkins (ddp), Carnegie-Mellon University
 *  Merged in new MIT sources.
 *
 * 23-Apr-84, Eric R. Crane (erc), Carnegie-Mellon University
 *  Added V2BROARCAST for the address resolution code, and for the
 * initialization code.
 *
 * 22-Apr-84, Eric R. Crane (erc), Carnegie-Mellon University
 *  Added a protocol type of V2ADR for the INTERNET Address resolution
 * protocol
 *
 ***********************************************************************
*/

#include <dma.h>
#include <int.h>

/* register addresses (offsets from the base) */
#define	V2ICSR	0	/* input CSR */
#define	V2IBUF	1	/* input buffer */
#define	V2ILCNT	2	/* low byte of input buffer byte count */
#define	V2IHCNT	3	/* high byte of input buffer byte count */
#define	V2OCSR	4	/* output CSR */
#define	V2OBUF	5	/* output buffer */
#define	V2OLCNT	6	/* low byte of output buffer byte count */
#define	V2OHCNT	7	/* high byte of output buffer byte count */

/* should actually be a variable set from the net struct or the
	custom structure
*/
#define	v2_base	custom.c_base	/* for now */
#define	mkv2(x)	((x)+custom.c_base)

/* Bits. */
/* input CSR */
#define	COPYEN	0x01
#define	MODE1	0x02
#define	MODE2	0x04
#define	BADFMT	0x08
#define	PARITY	0x10
#define	INRST	0x20
#define	OVERRUN	0x20
#define	ININTEN	0x40
#define	ININTRES	0x80
#define	ININTSTAT	0x80

/* output CSR */
#define	ORIGEN	0x01
#define	REFUSED	0x02
#define	OBADFMT	0x04
#define	OUTTMO	0x08
#define	INITRING	0x10
#define	OUTRST	0x20
#define	RGNOK	0x20
#define	OUTINTEN	0x40
#define	OUTINTRES	0x80
#define	OUTINTSTAT	0x80


/* header includes hardware header and link-level (set by convention) header
*/
struct pr_hdr {
	char	pr_dst;
	char	pr_src;
	long	pr_type;
	};

#define	V2MINLEN	sizeof(struct pr_hdr)

/* proNET ring packet types */
#define	PRONET_IP	0x00000102L
#define PRONET_ARP	0x00000302L /* DDP Address Resolution Protocol */
#define PRONET_RINGWAY	0x00000602L /* DDP For ringway (DECnet) */

extern char prBROADCAST;    	/* DDP Broadcast address */

extern NET *pr_net;
extern task *prDemux;
extern char _prme;
extern unsigned pr_eoi;		/* end-of-interrupt command for 8259A */
extern unsigned pr_ocwr;
