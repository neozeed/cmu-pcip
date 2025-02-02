/* Copyright 1986 by Carnegie Mellon */
/*  Copyright 1983 by the Massachusetts Institute of Technology  */

/* DCtcp.h */

#ifndef TCP_H				/* DDP */
#define TCP_H	1			/* DDP */

#include <ip.h>

/* Definitions for Alto TCP translation */

typedef	unsigned long	seq_t;		/* DDP - sequence numbers */

typedef	union	seq_no	{
		seq_t	a;
		struct	{
			unsigned   l;
			unsigned   h;
		}	p;
};

struct	tcp	{			/* a tcp header */
	unshort	tc_srcp;		/* source port */
	unshort	tc_dstp;		/* dest port */
	seq_t	tc_seq;			/* sequence number */
	seq_t	tc_ack;			/* acknowledgement number */
/* DDP - Make these unsigned */
	unsigned int tc_uu1 : 4;	/* unused */
	unsigned int tc_thl : 4;	/* tcp header length */
	unsigned int tc_fin : 1;	/* fin bit */
	unsigned int tc_syn : 1;	/* syn bit */
	unsigned int tc_rst : 1;	/* reset bit */
	unsigned int tc_psh : 1;	/* push bit */
	unsigned int tc_fack : 1;	/* ack valid */
	unsigned int tc_furg : 1;	/* urgent ptr. valid */
	unsigned int tc_uu2 : 2;	/* unused */
	unshort	tc_win;			/* window */
	unshort	tc_cksum;		/* checksum */
	unshort	tc_urg;			/* urgent pointer */
   };

struct	tcp_option	{		/* max-seg-size option */
	unsigned int tc_opt : 8;	/* option type */
        unsigned int tc_opt_len : 8;	/* option length */
	unsigned int tc_opt_val;	/* option value */
    };

/* TCP psuedo-header structure, used for checksumming */
struct	tcpph	{			/* psuedo-header */
	in_name	tp_src;			/* source addr */
	in_name	tp_dst;			/* dest addr */
	char	tp_zero;		/* always 0 */
	char	tp_pro;			/* protocol */
	int	tp_len;			/* length */
	};


#define	SEQ_GT(a, b)	(((a) - (b)) > 0)
#define	SEQ_GE(a, b)	(((a) - (b) >= 0)
#define	SEQ_EQ(a, b)	((a) == (b))
#define	SEQ_NE(a, b)	((a) != (b))
#define	SEQ_LE(a, b)	(((a) - (b)) <= 0)
#define	SEQ_LT(a, b)	(((a) - (b)) < 0)


#define	TCPPROT		6		/* tcp protocol number */
#define TCPMAXWIND	4096		/* maximum tcp window that this 
					   implemnetation supports */
#define	TCPSIZE		sizeof(struct tcp)

/* tcp connection states */

#define CLOSED		0		/* nothing has happenned             */
			/* CLOSED,SYNSENT,ESTAB,TIMEWAIT same as in tcp spec */
#define SYNSENT		1		/* connection requested              */
#define SYNRCVD		2
#define ESTAB		3		/* connection established            */
#define FINSENT		4		/* local user wishes to close        */
					/* same as FIN-WAIT-1 in tcp spec    */
#define FINRCVD		5 		/* foreign host wishes to close      */
					/* same as CLOSE-WAIT in tcp spec    */
#define SIMUL		6	/* local user and foreign host wish to close */
					/* same as CLOSING in tcp spec       */
#define FINACKED	7		/* local user's close request acked  */
					/* same as FIN-WAIT-2 in tcp spec    */
#define R_AND_S		8 	/* frgn close req rcvd, local close req sent */
					/* same as LAST-ACK in tcp spec      */
#define TIMEWAIT	9		/* last ack is being sent            */

#define NOT_YET(a, b)	((a) < (b))

#endif				/* DDP */
