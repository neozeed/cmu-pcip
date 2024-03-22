/*  Copyright 1986 by Carnegie Mellon  */
/*  See permission and disclaimer notice in file "cmu-note.h"  */
#include	<cmu-note.h>

/* Register offset definitions */
#define AX_CHANA_DATA	0x0	/* SCC channel A data */
#define AX_CHANB_DATA	0x1	/* SCC channel B data */
#define AX_CHANA_CTL	0x2	/* SCC channel A control */
#define AX_CHANB_CTL	0x3	/* SCC channel B control */
#define AX_CTM_CNTR0	0x4	/* Counter/Timer Counter 0 */
#define AX_CTM_CNTR1	0x5	/* Counter/Timer Counter 1 */
#define AX_CTM_CNTR2	0x6	/* Counter/Timer Counter 2 */
#define AX_CTM_CTL	0x7	/* Counter/Timer control */

#define	MKAX(x)	((x)+custom.c_base)

#define AX_FREQ		3686400L /* xtal frequency in MHz */
#define AX_TIMEOUT	5	/* timeout in 1/18 secs for sending pkts */

/* Some externals for the ADAX PC-SDMA package */
extern NET *ax_net;
extern Task AxDemux;
extern PACKET ax_inp;		/* Current input packet */
extern lapb_cb ax_lcb;		/* LAPB Connection Block */
extern int ax_aborting;		/* Aborting state */
extern int ax_insync;		/* Receiver sync'ed flag */
extern int ax_cts;		/* Receiver CTS flag */
extern int ax_dcd;		/* Receiver DCD flag */
extern long cticks;
