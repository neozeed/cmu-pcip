/*  Copyright 1986 by Carnegie Mellon  */
/*  See permission and disclaimer notice in file "cmu-note.h"  */
#include	<cmu-note.h>
/*  Copyright 1984 by the Massachusetts Institute of Technology  */
/*  See permission and disclaimer notice in file "notice.h"  */
#include	<notice.h>

/* Copyright (c) 1988 Epilogue Technology Corporation	*/
/*  See permission and disclaimer notice in file "etc-note.h"  */
#include	"etc-note.h"

/* $Revision:   2.0  $		$Date:   29 Oct 1989 16:32:36  $	*/
/*
 * $Log:   C:/KARL/CMUPCIP/SRCLIB/PKT/ET_CLOSE.C_V  $
 * 
 *	Rev 2.0 29 Oct 89 by Joe Doupnik, for Packet Driver and IEEE 802.3
 *
 *    Rev 1.0   04 Mar 1988 16:32:36
 * Initial revision.
*/

#include <task.h>
#include <q.h>
#include <netq.h>
#include <net.h>
#include <custom.h>
#include <netbuf.h>
#include <stdio.h>
#include "pkt.h"
#include "et_pkt.h"

/* Shutdown the packet driver interface */

et_close()
{
pkt_set_rcv_mode(ip_handle,3);	/* set to regular mode jrd */
pkt_release_type(ip_handle);
pkt_release_type(arp_handle);
pkt_release_type(dechandle1);
pkt_release_type(dechandle2);
pkt_release_type(dechandle3);
int_on();
}
