/*  Copyright 1986 by Carnegie Mellon  */
/*  See permission and disclaimer notice in file "cmu-note.h"  */
#include	<cmu-note.h>
/*  Copyright 1984, 1985 by the Massachusetts Institute of Technology  */
/*  See permission and disclaimer notice in file "notice.h"  */
#include	<notice.h>

/* Copyright (c) 1988 Epilogue Technology Corporation	*/
/*  See permission and disclaimer notice in file "etc-note.h"  */
#include	"etc-note.h"

/* $Revision:   1.0  $		$Date:   04 Mar 1988 16:32:42  $	*/
/*
 * $Log:   C:/KARL/CMUPCIP/SRCLIB/PKT/ET_SEND.C_V  $
 * 
 *    Rev 1.0   04 Mar 1988 16:32:42
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
#include <stdio.h>
#include "pkt.h"
#include "et_pkt.h"

extern int ip_handle, arp_handle;
extern unsigned etminlen;   /* jrd */

/* Transmit a packet. If it's an IP packet, calls ARP to figure out the
	ethernet address.
*/


/* 19-Aug-84 - changed et_send to send packets sent to x.x.x.0 to the
	ethernet broadcast address.
						<John Romkey>
*/
et_send(p, prot, len, fhost)
	PACKET p;
	unsigned prot;
	unsigned len;
	in_name fhost;
{
register struct ethhdr *pe;

/* Set up the ethernet header. Insert our address and the address of
  the destination and the type field in the ethernet header
  of the packet. */

#ifdef	DEBUG
if(NDEBUG & (INFOMSG|NETRACE))
   printf("ET_SEND: p[%u] -> %a.\n", len, fhost);
#endif

pe = (struct ethhdr *)p->nb_buff;
etadcpy(_etme, pe->e_src);

/* Setup the type field and the addresses in the ethernet header. */
switch(prot)
   {
   case IP:
	if((fhost == 0xffffffff) ||	/* Physical cable broadcast addr*/
					/* All subnet broadcast */
	   (fhost == et_net->n_netbr) ||
					/* All subnet bcast (4.2bsd) */
	   (fhost == et_net->n_netbr42) ||
					/* Subnet broadcast */
	   (fhost == et_net->n_subnetbr)) etadcpy(ETBROADCAST, pe->e_dst);
	   else if(ip2et(pe->e_dst, (in_name)fhost) == 0)
	     {
#ifdef	DEBUG
	     if(NDEBUG & (INFOMSG|NETERR))
		printf("ET_SEND: ether address unknown\n");
#endif
	     return 0;
	     }
	pe->e_type = ET_IP;
	break;
   case ARP:
	etadcpy((char *)fhost, pe->e_dst);
	pe->e_type = ET_ARP;
	break;
   default:
#ifdef	DEBUG
	if(NDEBUG & (INFOMSG|PROTERR|BUGHALT))
	   printf("ET_SEND: Unknown prot %u.\n", prot);
#endif
	return 0;
   }

len += sizeof(struct ethhdr);
#ifdef	WATCH
if(len < etminlen) len = etminlen;
#endif
#ifndef	WATCH
if(len < ET_MINLEN) len = ET_MINLEN;
#endif

(void) pkt_send((char *)pe, len);

return len;
}
