/*  Copyright 1986 by Carnegie Mellon  */
/*  See permission and disclaimer notice in file "cmu-note.h"  */
#include	<cmu-note.h>
/*  Copyright 1984, 1985 by the Massachusetts Institute of Technology  */
/*  See permission and disclaimer notice in file "notice.h"  */
#include	<notice.h>

/* Copyright (c) 1988 Epilogue Technology Corporation	*/
/*  See permission and disclaimer notice in file "etc-note.h"  */
#include	"etc-note.h"

/* $Revision:   2.0  $		$Date:   29 Oct 1989 16:32:38  $	*/
/*
 * $Log:   C:/KARL/CMUPCIP/SRCLIB/PKT/ET_INIT.C_V  $
 * 
 *	Rev 2.0 29 Oct 89 by Joe Doupnik, for Packet Driver and IEEE 802.3
 *
 *    Rev 1.0   04 Mar 1988 16:32:38
 * Initial revision.
*/

#include <task.h>
#include <q.h>
#include <netq.h>
#include <net.h>
#include <custom.h>
#include <netbuf.h>
#include <stdio.h>
#include <ether.h>
#include <timer.h>
#include "pkt.h"
#include "et_pkt.h"

#define then

/* 30-Mar-86 Added et_switch routine and rearrange et_init to call it.
					<Drew D. Perkins>
*/

/* storage for lots of things like my ethernet address, the ethernet broadcast
	address and my task and net pointers
*/
char ETBROADCAST[6] = { 0xff, 0xff, 0xff, 0xff, 0xff, 0xff};
char _etme[6];		/* my ethernet address */
task *EtDemux;		/* ethernet packet demultiplexing task */
NET *et_net;		/* my net pointer */
char etrcvcmd;		/* receiver command byte */
char save_mask; 	/* receiver command on entry. */
unsigned et_eoi;
static char et_msgid[] = "PKT DRVR";
unsigned etrreset = 0;
int ip_handle, arp_handle;
int dechandle1, dechandle2, dechandle3;

pkt_driver_info_t drvr_info;
pkt_driver_statistics_t start_pkt_stats;	/* Stats at startup */

int et_demux(); 	/* the routine which is the body of the demux task */
extern unsigned etint;

et_init(net, options, dummy)
	NET *net;
	unsigned options;
	unsigned dummy; {

#ifdef	DEBUG
	if(NDEBUG & INFOMSG)
		printf("Forking ETDEMUX.\n");
#endif

	EtDemux = tk_fork(tk_cur, et_demux, net->n_stksiz, "ETDEMUX", net);
	if(EtDemux == NULL) {
		printf("Error: %s setup failed\n", et_msgid);
		exit(1);
		}

	et_net = net;
	et_net->n_demux = EtDemux;

	et_switch(1, options);		/* DDP */

	/* Now everything is initialized. The DMA channel should only be
		initialized on demand, so it's not necessary to touch it
		now.
	*/
	tk_yield();	/* Give the per net task a chance to run. */

	/* init arp */
	etainit();
	return;
	}

/*
	Routine to switch the board and interrupts on/off.
 */
et_switch(state, options)
int state;
unsigned options;
{
static char cantaccess[] = "Can't access %s packet type\n";

static char iptype[] = { 0x08, 0x00 };
static char arptype[] = { 0x08, 0x06 };
static char wtype[] = {0xff, 0xff};	/* jrd */

if(state)
   { 	/* Turn them on? */

#ifdef WATCH			/* setup for Packet Driver jrd */
   if ((ip_handle = pkt_access_type(IC_ETHERNET, IT_ANY, 0,
	   wtype, 0, pkt_receive_helper))
	== -1)
      then {
	   printf(cantaccess, "ALL");
	   printf("Packet driver probably not loaded\n");
	   exit(1);
	   }
	pkt_set_rcv_mode(ip_handle,6); /* set promiscuous mode */
#else

   if ((ip_handle = pkt_access_type(IC_ETHERNET, IT_ANY, 0,
	   iptype, sizeof(iptype), pkt_receive_helper))
	== -1)
      then {
	   printf(cantaccess, "IP");
	   printf("Packet driver probably not loaded\n");
	   exit(1);
	   }
/* #endif */

   if ((arp_handle = pkt_access_type(IC_ETHERNET, IT_ANY, 0,
	   arptype, sizeof(arptype), pkt_receive_helper))
	== -1)
      then {
	   printf(cantaccess, "ARP");
	   (void) pkt_release_type(ip_handle);
	   exit(1);
	   }
#endif
     (void) pkt_driver_info(ip_handle, &drvr_info);

     (void) pkt_driver_info(dechandle1, &drvr_info);

     if (custom.c_seletaddr != HARDWARE)
        then {
	     printf("Using hardware Ethernet/MAC address\n");
	     }

    /* Learn our Ethernet address ... we can not set it */
    (void) pkt_get_address(ip_handle, _etme, sizeof(_etme));

    /* Snapshot the statistics before we do anything.	*/
    /* That way we can give the user a better picture of*/
    /* what happened during a particular command.	*/
    (void) pkt_get_statistics(ip_handle, &start_pkt_stats,
			      sizeof(start_pkt_stats));
    }
    else et_close();	/* Let et_close do the work */
}
