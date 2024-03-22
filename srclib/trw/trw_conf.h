/* Copyright 1988 TRW Information Networks */
/*  See permission and disclaimer notice in file "trw-note.h"  */
#include	"trw-note.h"

/* $Revision:   1.0  $		$Date:   29 Feb 1988 20:21:08  $	*/

#if (!defined(trw_conf))
#define trw_conf 1

/* Establish our compatability level.  This is checked against that on	 */
/* the board to be sure that we can work together.  (Our value must be	 */
/* >= to that of the board.						 */
#define DRIVER_COMPATABILITY_LEVEL	0

/* Define the fixed configuration values we use.			 */
/* These may NOT be changed.						 */
#define ADDRLEN		6

/* Define the configurable configuration (!) values we use.		 */
/* These may be adjusted for tuning.					 */
/* These parameters should not be made so large that they consume more   */
/* than the 128K available on the dumb version of the board.  (And	 */
/* enought space should be left to hold the SCP and ISCP at the very top */
/* of the memory space.							 */
#if (!defined(WATCH))
#define	NUM_CB		12	/* Number of Command Blocks, MIN=3	 */
#define NUM_RFD		73	/* Number of Receive Frame Descriptors	 */
#else
#define	NUM_CB		4	/* Number of Command Blocks, MIN=3	 */
#define NUM_RFD		1148	/* Number of Receive Frame Descriptors	 */
#endif

#define NUM_RBD		NUM_RFD	/* Number of Receive Buffer Descriptors	 */
				/*   (Should be >= NUM_RFD)		 */
#if (!defined(WATCH))
#define XMIT_BUFF_SIZE	1500	/* Should be big enough to hold largest	 */
				/*   transmitted packet, not counting	 */
				/*   the 14 byte ether header.		 */
				/* Must be a multiple of 2		 */
#define RCV_BUFF_SIZE	1500	/* Must be > 64 to prevent 586 overruns  */
				/* Must be a multiple of 2		 */
#else
#define XMIT_BUFF_SIZE	0	/* May be zero				 */
				/* Must be a multiple of 2		 */
#define RCV_BUFF_SIZE	80	/* Should be >= MATCH_DATA_LEN		 */
				/* Must be > 64 to prevent 586 overruns	 */
				/* Must be a multiple of 2		 */
#endif

/* Define some parameters that may be, but ought not be, changed.	 */
#define INT_POKE_TIME	3	/* How many seconds between sucessive	 */
				/* interrupt simulations.		 */

/* The following definitions should not be changed.			 */
#define	NUM_TBD			NUM_CB	/* # Transmit Buffer Descriptors */
#define NUM_XMIT_BUFFERS	NUM_CB	/* Number of transmit buffers	 */
#define NUM_RECV_BUFFERS	NUM_RBD	/* Number of receive buffers	 */
#endif
