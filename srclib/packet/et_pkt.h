/* Copyright (c) 1988 Epilogue Technology Corporation	*/
/*  See permission and disclaimer notice in file "etc-note.h"  */
#include	"etc-note.h"

/* $Revision:   1.0  $		$Date:   04 Mar 1988 16:33:00  $	*/
/*
 * $Log:   C:/KARL/CMUPCIP/SRCLIB/PKT/ET_PKT.H_V  $
 * 
 *    Rev 1.0   04 Mar 1988 16:33:00
 * Initial revision.
*/

#if (!defined(pkt_include))
#include "pkt.h"
#endif

extern pkt_driver_info_t drvr_info;
extern pkt_driver_statistics_t start_pkt_stats;

extern int ip_handle, arp_handle;
extern int dechandle1, dechandle2, dechandle3;

extern NET *et_net;	/* ET's net */
extern task *EtDemux;	/* ET's demultiplexing task */
extern char _etme[6];	/* my ethernet address */
extern char ETBROADCAST[6];	/* ethernet broadcast address */

/* The ethernet address copying function */
#define etadcpy(S,D)	(((int *)(D))[0]=((int *)(S))[0], \
			 ((int *)(D))[1]=((int *)(S))[1], \
			 ((int *)(D))[2]=((int *)(S))[2])
