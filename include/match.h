/* Copyright 1986 by Carnegie Mellon */
/*  Copyright 1984, 1985 by the Massachusetts Institute of Technology  */
/*  See permission and disclaimer notice in file "notice.h"  */
#include	<notice.h>

#define	MATCH_DATA_LEN	60

/* pkt data structure */
struct pkt {
	unsigned p_len;
	char	p_data[MATCH_DATA_LEN];
	};

#define	MAXPKT		512
#define	PKTMASK		511

extern struct pkt pkts[];
extern int prcv, pproc;

extern unsigned start;
