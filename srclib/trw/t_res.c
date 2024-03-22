/* Copyright 1988 TRW Information Networks */
/*  See permission and disclaimer notice in file "trw-note.h"  */
#include	"trw-note.h"

/* This module manages the allocation of CB blocks and their associated	*/
/* TBDs and transmit buffers. (For convenience each CB has a TBD and	*/
/* buffer permanently associated with it.)				*/

/* $Revision:   1.0  $		$Date:   29 Feb 1988 20:20:28  $	*/

#include <dos.h>
#include <task.h>
#include <q.h>
#include <netq.h>
#include <net.h>
#include <custom.h>
#include <netbuf.h>
#include <stdio.h>
#include <ether.h>
#include <timer.h>
#include "trw.h"

cb_list_hdr_t	free_CB_list;	/* List header for the list of	*/
				/* available CBs, TBDs, and xmit*/
				/* buffers.			*/

cb_list_hdr_t	pending_CB_fifo;	/* List header for the fifo of	*/
					/* CBs/TBDs/Xmit buffers waiting*/
					/* to be executed (Note, this	*/
					/* is not the same list as found*/
					/* in the 82586 itself.)	*/

cb_list_el_t	cb_elements[NUM_CB];	/* The list allocation elements */

cb_list_el_t *
get_CB_from_list(lhp)
	register cb_list_hdr_t	*lhp;	/* Pointer to list header	*/
{
if (lhp->first == CBP_NULL)		/* Any resources available?	*/
   then return CBP_NULL;		/* No.	*/
   else {				/* Yes, give the one at the head*/
	register cb_list_el_t *elp;	/* of the list and update the	*/
	elp = lhp->first;		/* list pointers to make the 2nd*/
	lhp->first = elp->cbp_next;	/* (if any) the first on the	*/
	return elp;			/* list.			*/
	}
/* NOTREACHED */
}

void
append_CB_to_list(lhp, elp)
	register cb_list_hdr_t	*lhp;	/* Pointer to list header	*/
	register cb_list_el_t	*elp;
{
elp->cbp_next = CBP_NULL;
if (lhp->first == CBP_NULL)	/* Is the list empty? */
   then { /* Yes, add the element directly */
	lhp->first = elp;
	lhp->last = elp;
	}
   else { /* No, put the element at the end of the list. */
	(lhp->last)->cbp_next = elp;
	lhp->last = elp;
	}
}

/* Initialize the CB, RFD, and RBD lists */
/* This routine should be called with interrupts disabled */
void
init_CB_lists()
{
int i;
unsigned long buf_paddr;	/* Physical address of a buffer in full	*/
				/* 24 bit representation.		*/

/* Build the CB list */
free_CB_list.first = free_CB_list.last =
	pending_CB_fifo.first = pending_CB_fifo.last = CBP_NULL;

   {
   register cb_list_el_t	*elp;
   uint	cb_link, tbd_link;
   
   for (i = NUM_CB,
	elp = cb_elements, 
	cb_link = CB0_LINK_OFFSET,
	tbd_link = TBD0_LINK_OFFSET,
	buf_paddr = XMIT_BUFFER_OFFSET;
	i != 0;
	elp++,
	cb_link += sizeof(cb_t),
	tbd_link += sizeof(tbd_t),
	buf_paddr += XMIT_BUFF_SIZE,
	--i)
      {
      elp->cb_link_offset = cb_link;
      elp->cb_iomm_addr = cb_link >> 1;
      elp->tbd_link_offset = tbd_link;
      elp->tbd_iomm_addr = tbd_link >> 1;
      elp->xbuff_iomm_addr = (uint)(buf_paddr >> 1);
      elp->xbuff_physaddr.low_16 = (uint)buf_paddr;
      elp->xbuff_physaddr.hi_8 = buf_paddr > 0x0000FFFFL ? 0xFF : 0xFE;

#if defined(DEBUG)
printf("CB: off=%x iom=%x \tTBD: off=%x iom=%x \tXB: low16=%x iom=%x\n",
	elp->cb_link_offset, elp->cb_iomm_addr,
	elp->tbd_link_offset, elp->tbd_iomm_addr,
	elp->xbuff_physaddr.low_16, elp->xbuff_iomm_addr);
#endif
      release_CB(elp);
      }
   }


/* At this point buf_paddr should be pointing to the first of the	*/
/* receive buffers.							*/

/* Build the RFD list -- a circular list in which the first element	*/
/* points to the first RBD and the last element has the EL bit set.	*/
   {
   rfd_t rfd;
   uint  this_fd_iomm;	/* I/O port address to reach this rfd.	*/

   memset(&rfd, 0, sizeof(rfd_t));

   next_rfd = RFD0_LINK_OFFSET >> 1;
   for (rfd.rfd_link = RFD0_LINK_OFFSET, i = 0; i < NUM_RFD; i++)
      {
      this_fd_iomm = rfd.rfd_link >> 1;	/* Remember where this RFD lives */

      if (i == 0)  /* Is this the first RFD? */
         then rfd.rfd_rbd_offset = RBD0_LINK_OFFSET; /*Yes, point to 1st RBD*/
	 else rfd.rfd_rbd_offset = 0xFFFF; /* No, give null pointer */     

      if (i == (NUM_RFD - 1))  /* Is this the last RFD? */
         then { /* Yes */
              rfd.rfd_el_s = RFD_EL;
	      rfd.rfd_link = RFD0_LINK_OFFSET;
	      prior_rfd = this_fd_iomm;
	      }
         else { /* Not the last RFD */
              rfd.rfd_el_s = 0;
	      rfd.rfd_link += sizeof(rfd_t);
	      }

      block_out(this_fd_iomm, &rfd, sizeof(rfd_t));

#if defined(DEBUG)
      printf("#=%d  @%x  S: %x  ELS: %x  LINK: %x  RBD: %x\n",
	     i, this_fd_iomm,
	     rfd.rfd_stat, rfd.rfd_el_s, rfd.rfd_link, rfd.rfd_rbd_offset);
#endif
      }
#if defined(DEBUG)
   printf("NXT_RFD=%x  PRIOR_RFD=%x\n", next_rfd, prior_rfd);
#endif
   }

/* Build the RBD list -- a circular list.       */
/* Each element points to its buffer.		*/
/* The last element has the EL bit set.		*/
   {
   rbd_t rbd;
   uint  this_rbd_iomm;	/* I/O port address to reach this rbd.	*/

   memset(&rbd, 0, sizeof(rbd_t));

   next_rbd = RBD0_LINK_OFFSET >> 1;
   for (rbd.rbd_link = RBD0_LINK_OFFSET, i = 0;
        i < NUM_RBD;
	i++, buf_paddr += RCV_BUFF_SIZE)
      {

#if defined(TUNE_DEBUG)
      if (buf_paddr > 0x0001FFFFL)
         then {
	      printf("TOO MANY BUFFERS %i\n", i);
	      exit(1);
	      }
#endif

      this_rbd_iomm = rbd.rbd_link >> 1; /* Remember where this RBD lives */

      rbd.rbd_buff.low_16 = (uint)buf_paddr;
      rbd.rbd_buff.hi_8 = buf_paddr > 0x0000FFFFL ? 0xFF : 0xFE;
      rbd.rbd_buff_ioaddr = (uint)(buf_paddr >> 1);

      if (i == (NUM_RBD - 1))  /* Is this the last rbd? */
         then { /* Yes */
              rbd.rbd_size = RCV_BUFF_SIZE | RBD_EL;
	      rbd.rbd_link = RBD0_LINK_OFFSET;
	      prior_rbd = this_rbd_iomm;
	      }
         else { /* Not the last rbd */
              rbd.rbd_size = RCV_BUFF_SIZE;
	      rbd.rbd_link += sizeof(rbd_t);
	      }

      block_out(this_rbd_iomm, &rbd, sizeof(rbd_t));

#if defined(DEBUG)
     printf("#=%d @%x  ACT: %x  LINK: %x  Bhi: %x  Blo %x  SZ: %x  IOM: %x\n",
	     i, this_rbd_iomm,
	     rbd.rbd_act_cnt, rbd.rbd_link, rbd.rbd_buff.hi_8,
	     rbd.rbd_buff.low_16, rbd.rbd_size, rbd.rbd_buff_ioaddr);
#endif
      }
#if defined(DEBUG)
   printf("FREEMEM @ %X\n", (unsigned long)buf_paddr);
   printf("NXT_RBD=%x  PRIOR_RBD=%x\n", next_rbd, prior_rbd);
#endif
   }
}
