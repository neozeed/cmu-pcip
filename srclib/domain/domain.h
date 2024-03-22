/* Copyright 1986 by Carnegie Mellon */
/*  Copyright 1986 by Carnegie Mellon  */
/*  See permission and disclaimer notice in file "cmu-note.h"  */
#include	<cmu-note.h>
/* domain name server protocol */

struct dm_hdr {
	unsigned	dm_id;
	unsigned	dm_flags;
	unsigned	dm_qd_count;
	unsigned	dm_an_count;
	unsigned	dm_ns_count;
	unsigned	dm_ar_count;
	};

/* bits in flags */
#define	DM_RESPONSE	0x8000
#define	DM_AUTHORITY	0x0400
#define	DM_TRUNCATED	0x0200
#define	DM_DO_RECURSE	0x0100
#define	DM_CAN_RECURSE	0x0080

/* field masks */
#define	OPCODE_MASK	0x7800
#define	RESPONSE_MASK	0x000F

/* opcodes in flags */
#define	OP_QUERY	0
#define	OP_IQUERY	1
#define	OP_CQUERYM	2
#define	OP_CQUERYU	3

/* response codes in flags */
#define	RC_NO_ERR	0
#define	RC_FORMAT	1
#define	RC_SERVER	2
#define	RC_NAME		3
#define	RC_NOT_IMP	4
#define	RC_REFUSED	5

#define	get_opcode(p)	(((p)->dm_flags & OPCODE_MASK) >> 11)
#define	put_opcode(p, c)	{ (p)->dm_flags &= ~OPCODE_MASK; \
				  (p)->dm_flags |= (c << 11); }

#define	get_rc(p)	((p)->dm_flags & RESPONSE_MASK)
#define	put_rc(p, c)	{ (p)->dm_flags &= ~RESPONSE_MASK; \
			  (p)->dm_flags |= c; }

/* resource record types */
#define	A	1
#define	NS	2
#define	MD	3
#define	MF	4
#define	CNAME	5
#define	SOA	6
#define	MB	7
#define	MG	8
#define	MR	9
#define	RT_NULL	10
#define	WKS	11
#define	PTR	12
#define	HINFO	13
#define	MINFO	14

/* query types */
#define	AXFR	252
#define	MAILB	253
#define	MAILA	254
#define	QT_STAR	255

/* class types */
#define	IN	1
#define	CS	2

/* qclass types */
#define	QC_STAR	255
