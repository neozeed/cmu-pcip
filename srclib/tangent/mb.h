/*  Copyright 1986 by Carnegie Mellon  */
/*  See permission and disclaimer notice in file "cmu-note.h"  */
#include	<cmu-note.h>

/* Some externals for the MacBridge package */
extern NET *mb_net;
extern Task MbDemux;

/* Register definitions */
#define MB_CHANA_CTL	0x341	/* SCC channel A control */
#define MB_CHANB_CTL	0x340	/* SCC channel B control */
#define MB_CHANA_DATA	0x343	/* SCC channel A data */
#define MB_CHANB_DATA	0x342	/* SCC channel B data */
#define MB_DMA_RESET	0x344	/* clear DMA request after last */
#define MB_FREQ		3686400L /* xtal frequency in MHz */
#define MB_TIMEOUT	5	/* timeout in 1/18 secs for sending pkts */
