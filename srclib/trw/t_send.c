/*  Copyright 1986 by Carnegie Mellon  */
/*  See permission and disclaimer notice in file "cmu-note.h"  */
#include	<cmu-note.h>
/*  Copyright 1984, 1985 by the Massachusetts Institute of Technology  */
/*  See permission and disclaimer notice in file "notice.h"  */
#include	<notice.h>

/* Copyright 1988 TRW Information Networks */
/*  See permission and disclaimer notice in file "trw-note.h"  */
#include	"trw-note.h"

/* $Revision:   1.0  $		$Date:   29 Feb 1988 20:20:38  $	*/

#include <task.h>
#include <q.h>
#include <netq.h>
#include <net.h>
#include <custom.h>
#include <netbuf.h>
#include <stdio.h>
#include <ether.h>
#include <stdio.h>
#include "trw.h"

/* Transmit a packet.							*/
/* If it's an IP packet, calls ARP to figure out the ethernet address.	*/
/* Unlike the original 3COM 3C501 driver, this one accepts the packet	*/
/* and may return before actual transmission is performed.  This may	*/
/* require an interlock with closing to allow the send queue to be	*/
/* drained.								*/

/* Each send packet is packaged into an 82586 Transmit CB, a TBD, and	*/
/* an actual buffer.  These are kept out in board memory.  Only the	*/
/* first in the queue is actually handed to the 82586 -- we don't try	*/
/* to build chains here, especially as Intel sort-of recommends against	*/
/* them in their handbook.  The interrupt handler will take the		*/
/* sucessive requests, attach them to the 82586 and start the transmit	*/
/* side of the 82586.							*/

#if defined(WATCH)
unsigned etminlen;
#endif

unsigned int
trw_send(p, prot, len, fhost)
	PACKET p;
	unsigned prot;
	unsigned len;
	in_name fhost;
{
cb_list_el_t	*xmit_cbp;
uchar		dstaddr[ADDRLEN];
uint		typefield;

if (len > XMIT_BUFF_SIZE)
   then {
	toobig_cnt++;
	return 0;
	}

   {
   uchar  *pe;

   pe = p->nb_buff + sizeof(struct ethhdr);

   /* We let the 82586 generate the ethernet/802.3 header.  This lets	*/
   /* us get away with smaller buffers on the board and avoid an 82586	*/
   /* bug in which it can't receive minimum spaced frames unless the	*/
   /* dst/src/len fields go into the RBD.				*/

#ifdef	DEBUG
   if(NDEBUG & (INFOMSG|NETRACE))
      then printf("ET_SEND: p[%u] -> %a.\n", len, fhost);
#endif

   /* Setup the type field and the addresses in the ethernet header. */
   switch(prot)
      {
      case IP:
	   if(
	      (fhost == 0xffffffff) 	||     /* Physical cable b'cast addr*/
	      (fhost == et_net->n_netbr)   ||  /* All subnet broadcast	    */
  	      (fhost == et_net->n_netbr42) ||  /* All subnet bcast (4.2bsd) */
	      (fhost == et_net->n_subnetbr))   /* Subnet broadcast	    */
	         then {
		      etadcpy(ETBROADCAST, dstaddr);
		      }
	         else {
		      if (ip2et(dstaddr, (in_name)fhost) == 0)
		         then {
#ifdef	DEBUG
			      if (NDEBUG & (INFOMSG|NETERR))
			        printf("ET_SEND: MAC address unknown\n");
#endif
			      return 0;
			      }
		      }
	   typefield = ET_IP;
	   break;
      case ARP:
	   etadcpy((char *)fhost, dstaddr);
	   typefield = ET_ARP;
	   break;
      default:
#ifdef	DEBUG
	   if(NDEBUG & (INFOMSG|PROTERR|BUGHALT))
	      then printf("ET_SEND: Unknown prot %u.\n", prot);
#endif
	   return 0;
      }

#if defined(WATCH)
   if(len < (etminlen - sizeof(struct ethhdr)))
      then len = etminlen - sizeof(struct ethhdr);
#else
   if(len < (ET_MINLEN - sizeof(struct ethhdr)))
      then len = ET_MINLEN - sizeof(struct ethhdr);
#endif

   /* In the following code, interrupts are disabled and enabled farily	*/
   /* often.  The purpose is to protect shared device registers while	*/
   /* leaving a few interrupt windows in case a packet is received.	*/
   /* The code could run completely disabled, but on 8088/8086 class	*/
   /* machines the time to move the packet buffer out to the board could*/
   /* be fairly long.							*/

   /* Get the CB, TBD, and buffer for this packet */
   _disable();
   xmit_cbp = get_free_CB();
   if (xmit_cbp == CBP_NULL)
      then {
	   _enable();
	   refused_cnt++;
	   return 0;	/* Tell' em we couldn't send it */
	   }

   /* Move the packet image out to the board */
   /* We use len + 1 because we know that block_out only transfers words */
   block_out(xmit_cbp->xbuff_iomm_addr, pe, len+1);
   _enable();
   }

/* Build the CB */
   {
   xmit_cb_t	xmit_cb;

   /* xmit_cb.cb_tran_hdr.cb_stat = 0;  -- The 586 writes the stat field */

   xmit_cb.cb_tran_hdr.cb_command = CB_TRANSMIT | CB_END_LIST;
   /* We don't bother to set the link field, because we don't */
   /* link transmit CBs. */
   
   xmit_cb.cb_tran_tbd = xmit_cbp->tbd_link_offset;
   etadcpy(dstaddr, xmit_cb.cb_tran_mac);
   xmit_cb.cb_tran_length = typefield;

   _disable();
   block_out(xmit_cbp->cb_iomm_addr, &xmit_cb, sizeof(xmit_cb_t));
   _enable();
   }

/* Build the TBD */
   {
   tbd_t	tbd;

   tbd.tbd_act_cnt = len | TBD_END_OF_LIST;
   /* tbd.tbd_link = ??; -- Not used when end-of-list bit is set */
   tbd.tbd_buff = xmit_cbp->xbuff_physaddr;	/* Structure assignment */
   _disable();
   block_out(xmit_cbp->tbd_iomm_addr, &tbd, sizeof(tbd_t));
   }

/* Interrupts are disabled now */

/* Place the CB in the pending send queue */
queue_CB(xmit_cbp);

/* Possibly initiate transmission */
kick_586();
_enable();

return len;
}

/* Start the 82586 transmitting the first CB in the pending queue */
/* Must be called with interrupts masked.			  */
void
kick_586()
{
cb_list_el_t	*xmit_cbp;

if (current_cb != CBP_NULL)
   then {
	return;
	}

if ((xmit_cbp = get_next_pending_CB()) == CBP_NULL)
   then {
        return;
	}

current_cb = xmit_cbp;
start_cb(xmit_cbp->cb_link_offset);

send_cnt++;
}
