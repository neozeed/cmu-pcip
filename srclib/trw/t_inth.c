/*  Copyright 1986 by Carnegie Mellon  */
/*  See permission and disclaimer notice in file "cmu-note.h"  */
#include	<cmu-note.h>
/*  Copyright 1984, 1985 by the Massachusetts Institute of Technology  */
/*  See permission and disclaimer notice in file "notice.h"  */
#include	<notice.h>

/* Copyright 1988 TRW Information Networks */
/*  See permission and disclaimer notice in file "trw-note.h"  */
#include	"trw-note.h"

/* $Revision:   1.1  $		$Date:   22 Mar 1988 22:40:22  $	*/

#include <task.h>
#include <q.h>
#include <netq.h>
#include <net.h>
#include <custom.h>
#include <netbuf.h>
#include <stdio.h>
#include <ether.h>
#include "trw.h"

#define BACKWARD_SCAN

#if (!defined(WATCH))
#define AT_DRAIN_COUNT		8
#define PC_DRAIN_COUNT		32
#else
#define AT_DRAIN_COUNT		32
#define PC_DRAIN_COUNT		128
#endif

#if defined(WATCH)
#include <match.h>
#endif

static int drain_fd(int);
#define SKIP_PKT	1
#define TAKE_PKT	0

/* This code services the ethernet interrupt. It is called by an assembly */
/* language routines which saves all the registers and sets up the	  */
/* data segment and such.  It also does the interrupt acknowledgement	  */
/* AFTER we return from the C interrupt handler.			  */

/* There are four variables of critical concern, prior_rfd, next_rfd,	  */
/* prior_rbd, and next_rbd.  These are I/O port memory addresses for	  */
/* certain important RBDs and RFDs.  The "prior" variables point to the	  */
/* block that has the end-of-list bit set.  The "next" variables point	  */
/* to its sucessor.  (We can derive "next" from "prior", but it is easier */
/* just to maintain separate variables.)  We use the "next" variables to  */
/* locate the next RFD or RBD in which we expect to find anything.	  */
/* Each time we empty an RFD or RBD, we hand it back to the 82586 by	  */
/* sliding the prior/empty pair one slot.				  */

/* The following are made static to keep the from using too much stack	*/
/* while servicing the interrupt */
static rbd_t	rbd;
static short_rfd_t	srfd;
#if defined(BACKWARD_SCAN)
static uint predrain_rfd, predrain_rbd;
#endif

#if defined(WATCH)
struct pkt pkts[MAXPKT];

int pproc = 0;
int prcv = 0;
long npackets = 0;
#endif

/* NOTE: trw_ihnd() IS ENTERED WITH INTERRUPTS *DISABLED*. */
void
trw_ihnd()
{
uint scb_status;
uint drained = 0;

int_flag = 1;
int_cnt++;

spin_on_scb_command();	/* Make sure the 82586 isn't doing a command */

scb_status = get_scb_status();		/* Get the status word from the SCB */
IRQ_RESET();
if (driver_state == DS_UNINITIALIZED)
   then {
	/* Assume that someone else [like i586_config()] acks the 82586 */
	return;
	}

/* Acknowledge the status if necessary */
if (scb_status & (ACK_CX | ACK_CNA | ACK_FR | ACK_RNR))
   then {
	command_scb(scb_status & (ACK_CX | ACK_CNA | ACK_FR | ACK_RNR));
	spin_on_scb_command();
	}

/* Look for some kind of CU interrupt condition */
if (scb_status & (CX | CNA))
   then { /* The CU has something to say... */
	if (current_cb != CBP_NULL)
	   then {  /* A CB has completed.  Since we only give the 82586	*/
		   /* one at a time, we know which one it was.		*/
		register uint xmit_status;
		uint xmit_cmd;

		/* Read in the status and command of the CB */
		outpw(ADDRPORT0, current_cb->cb_iomm_addr);
		xmit_status = inpw(DATAPORT0);
		xmit_cmd = inpw(DATAPORT0);

		/* Validate that the CB is, in fact, an xmit one */
		/* And that it did, in fact, complete.		 */
		if (((xmit_cmd & CB_COMMAND_MASK) != CB_TRANSMIT) ||
		     !(xmit_status & CMD_COMPLETED))
		   then { /* Oh boy, a problem! */
			brd_error(ERR_SOFTWARE);
			}

		if (xmit_status & 0x0FFF)  /* Anything in the error field? */
		   then {
		        if (xmit_status & XSTAT_LOST_CD)  then cd_lost_cnt++;
			if (xmit_status & XSTAT_LOST_CTS) then cts_lost_cnt++;
			if (xmit_status & XSTAT_UNDERRUN)
						    then xmit_dma_under_cnt++;
			if (xmit_status & XSTAT_DEFERRED) then deferred_cnt++;
			if (!(xmit_status & XSTAT_SQE))   then sqe_lost_cnt++;
			if (xmit_status & XSTAT_MAX_COLLISION)
							  then ex_coll_cnt++;
			collision_cnt += xmit_status & XSTAT_COLLISION_MASK;
			}

		release_CB(current_cb);
		current_cb = CBP_NULL;

		/* Start up next xmit, if any */
		kick_586();
	        }
	}

/* Here we will check for a received packet.  We don't wait for the	*/
/* presence of an FR flag.  This allows a timer-based routine to	*/
/* stimulate the draining of incoming packets in case this driver locks.*/
/* Such lock-up can occur if all the RFDs or RBDs are full and it was	*/
/* not possible to obtain a PACKET buffer.  In that case, there is no	*/
/* hardware interrupt to indicate that PACKET buffers have become	*/
/* available.  Steps are taken in the RNR code, below, to discard the	*/
/* oldest frames when the potential of lock-up occurs.  But ultimately	*/
/* the only way out is by a periodic poke to this interrupt handler.	*/

#if defined(BACKWARD_SCAN)
/* Remember the following pointers in order to get a head start on a	*/
/* reverse scan, in case we need to restart the 82596 RU.		*/
predrain_rfd = prior_rfd;
predrain_rbd = prior_rbd;
#endif

/* Drain as many frames as we can */
while (drain_fd(TAKE_PKT) == 1) drained++;




/* Check whether the RU has dropped out of ready mode.  It is not	*/
/* important whether it has just happened, i.e. RNR is set, or at some	*/
/* time in the past.							*/
if ((scb_status & RUS_MASK) != RUS_READY)
   then { /* It's not ready */
	register uint test_rfd, test_rbd;

#if defined(DEBUG)
	printf("RU Not Ready!\n");
#endif

	/* If we get here and if the code that handles rcv'd frames	*/
	/* could not release sufficient resources, we must take drastic */
	/* action and discard some received frames.  In this way we	*/
	/* achieve a polling situation in which every couple of incoming*/
	/* frames we will get control to see whether we can get a	*/
	/* PACKET buffer.  If no further frames come in, the pending	*/
	/* frames will quietly sit, and sit, and sit, ...		*/

	/* Now make some space.  This will free RFDs and RBDs starting	*/
	/* at the "next" locations, and moving the end-of-list markers	*/
	/* forward.							*/
	{
	register int i;
	i = cpu_is_286 ? AT_DRAIN_COUNT : PC_DRAIN_COUNT;
	if (i > drained)
	   then {
		for (i -= drained; i != 0; --i)
	           {
	           if (drain_fd(SKIP_PKT) == 0) then break;
		   }
	        }
	}

	/* Starting with the current "next_rfd", see if we can find an	*/
	/* RFD with no frame attached.  Then we will do the same for the*/
	/* RBD chain.							*/
	/* Because we "know" that drain_fd cleared some space we assume	*/
	/* that we will find it without getting into an infinite loop.	*/
	/* Once we find this RFD/RBD pair, we use them to restart the	*/
	/* 586.  Note, we know that the 586 is not running through the	*/
	/* RFD/RBD chains right now because its RU is stopped.		*/

#if (!defined(BACKWARD_SCAN))
	/* The following code scans FORWARD through the chains.	*/
	/* For long chains of RFDs and RBDs it would probably	*/
	/* be faster to do a backwards scan.			*/

	for (test_rfd = next_rfd;;)
	   {
	   outpw(ADDRPORT0, test_rfd);
	   if (!((inpw(DATAPORT0)) & RFD_C))
	      then { /* We found a free RFD */
		   for (test_rbd = next_rbd;;)
		      {
		      outpw(ADDRPORT0, test_rbd);
		      if (inpw(DATAPORT0) == 0)
		         then { /* We found a free RBD */
			      /* Construct the proper RFD->RBD linkage	*/
			      /* and start the 82586's RU.		*/
			      outpw(ADDRPORT0, test_rfd + 3);
			      outpw(DATAPORT0, test_rbd << 1);
			      start_rfd(test_rfd << 1);
			      return;
			      }
		      test_rbd = inpw(DATAPORT0) >> 1; /* Get next RBD */
		      } /* End of for loop on RBDs */
		   }
	   (void) inpw(DATAPORT0);	    /* Ignore the EL/S field */
	   test_rfd = inpw(DATAPORT0) >> 1; /* Grab the link offset field */
	   } /* End of for on RFDs */

#else 	/* BACKWARD_SCAN */
	/* The following code scans BACKWARDS through the chains.	*/
	/* It is possible that the drain_fd()s were so effective that	*/
	/* all the RFDs and all the RBDs are free!			*/

	for (test_rfd = predrain_rfd;;)
	   {
	   outpw(ADDRPORT0, test_rfd);
	   if (inpw(DATAPORT0) & RFD_C)
	      then { /* We found the predecessor to a free RFD */
		   /* Locate the sucessor, the real first free RFD */
		   (void) inpw(DATAPORT0);    /* Ignore the EL/S field */
		   test_rfd = inpw(DATAPORT0) >> 1; /* Link offset field */
		   break;
		   }
	   /* Check whether we come to our starting point (unmodified	*/
	   /* by the predrain variables).  If so, we assume that every	*/
	   /* RFD and RBD is available.					*/
	   if (test_rfd == next_rfd)	/* In case the list is empty */
              then {
		   test_rbd = next_rbd;
		   goto found_bs_place;
		   }
	   if (test_rfd != (RFD0_LINK_OFFSET >> 1))
	      then test_rfd -= (sizeof(rfd_t) >> 1);
	      else test_rfd = RFDX_LINK_OFFSET >> 1;
	   } /* End of for on RFDs */

	for (test_rbd = predrain_rbd;;)
	   {
	   outpw(ADDRPORT0, test_rbd);
	   if (inpw(DATAPORT0) != 0)
              then { /* We found a predecessor to free RBD */
		   /* Find the real first RBD */
		   test_rbd = inpw(DATAPORT0) >> 1;
		   break;
		   }
	   /* If we come across our starting point (unmodified by the	*/
	   /* predrain variables) then something odd is happening.	*/
	   /* Because we tested previously for the case in which the	*/
	   /* RFD list is empty, if we are here, there must be RFDs, but*/
	   /* no RBDs in use.  This *might* be possible, but I'm not	*/
	   /* very sure.						*/
	   if (test_rbd == next_rbd)	/* In case the list is empty */
	      then {
#if defined(RBD_DEBUG)
	           printf("RFD BUT NO RBD!\n"); trw_close(); exit(1);
#else
		   break;
#endif
		   }
	   if (test_rbd != (RBD0_LINK_OFFSET >> 1))
	      then test_rbd -= (sizeof(rbd_t) >> 1);
              else test_rbd = RBDX_LINK_OFFSET >> 1;
	   } /* End of for loop on RBDs */

found_bs_place:
	/* Construct the proper RFD->RBD linkage and start the 82586's RU. */
	outpw(ADDRPORT0, test_rfd + 3);
	outpw(DATAPORT0, test_rbd << 1);
	start_rfd(test_rfd << 1);
	return;

#endif /* BACKWARD_SCAN */
	}
}

#if (!defined(WATCH))
/* Extract the frame from an FD */
static int
drain_fd(skip)
	int	skip;	/* 1: send incoming data to /dev/null.	*/
			/* 0: try to accept the incoming data.	*/
{
PACKET		inpack;
uchar		*bp;

/* Read in the top part of the RFD of interest. */
/* We use direct I/O for speed.			*/
outpw(ADDRPORT0, next_rfd);
if (!((srfd.rfd_stat = inpw(DATAPORT0)) & RFD_C))
   then return 0;	/* Return if no frame in the RFD */

srfd.rfd_el_s = inpw(DATAPORT0);
srfd.rfd_link = inpw(DATAPORT0);
srfd.rfd_rbd_offset = inpw(DATAPORT0);

if (skip == TAKE_PKT)
   then {
        if ((inpack = getfree()) == NULL)	/* Can we get a buffer */
           then {		/* No, we'll get it next time -- */
				/* the hard part is arranging for*/
		return 0;	/* a next time.			 */
		}
	block_in(next_rfd+4, inpack->nb_buff, ADDRLEN + ADDRLEN + 2);
	bp = inpack->nb_buff + sizeof(struct ethhdr);
	}

/* Update the error counters from the frame */
if (srfd.rfd_stat & RFD_SHORT) then short_cnt++;

/* The rest of the error counters are maintained by the 586 in the SCB	*/
rcv_cnt++;

/* We've got everything we need to know from this RFD.		*/
/* So we can give the real, on-board one back to the 82586.	*/
/* We do this by making this RFD the end-of-list and then	*/
/* removing the end-of-list flag from the "prior" RFD.		*/
outpw(ADDRPORT0, next_rfd);
outpw(DATAPORT0, 0);		/* Clear the status so we don't trip */
				/* ourselves later. */
outpw(DATAPORT0, RFD_EL);	/* Turn on the EL flag */
outpw(ADDRPORT0, next_rfd+3);	/* Address the RBD offset word of the RFD */
outpw(DATAPORT0, 0xFFFF);	/* Insert a null RBD linkage */
outpw(ADDRPORT0, prior_rfd+1);	/* Address the EL & S word of the RFD */
outpw(DATAPORT0, 0);		/* Turn off the EL flag */

prior_rfd = next_rfd;
next_rfd = srfd.rfd_link >> 1;	/* link is in address form, we need it	*/
				/* i/o port address form. */

/* Now deal with the RBD's, if any, that have the frame's data */
  {
  uint  rbd_offset;
  uint	len = sizeof(struct ethhdr);
  uchar drop_pkt = 0;	/* Will be set to 1 if for some reason we must */
			/* discard a received packet buffer */

  if ((rbd_offset = srfd.rfd_rbd_offset) != 0xFFFF)
     then {
#if defined(RBD_DEBUG)
          /* Sanity check: rbd_offset should equal next_rbd */
	  if ((rbd_offset>>1) != next_rbd)
	     then {
	          printf("RBD_OFFSET %x != NEXT_RBD %x\n",
			  rbd_offset>>1, next_rbd);
		  trw_close();
		  exit(1);
		  }
#endif
	  for (;;)
	      {
	      block_in(next_rbd, &rbd, sizeof(rbd_t));

#if defined(RBD_DEBUG)
	      if (!(rbd.rbd_act_cnt & RBD_F))	/* Make sure RBD is valid */
	         then {
		      printf("Incomplete RBD\n");
		      trw_close();
		      exit(1);
		      }
#endif
		if (skip == TAKE_PKT)
		   then {
			uint chunk_sz;
			chunk_sz = rbd.rbd_act_cnt & RBD_COUNT_MASK;
			/* Prevent buffer overruns */
			if ((chunk_sz + len) > LBUF)
			   then {
				chunk_sz = LBUF - len;
				drop_pkt = 1;
				}
			if (chunk_sz != 0)
			   then {
				/* Use chunk_sz+1 to ensure we	*/
				/* get the last byte because	*/
				/* block_in moves words.	*/
				block_in(rbd.rbd_buff_ioaddr, bp,
					chunk_sz+1);
				bp += chunk_sz;
				len += chunk_sz;
				}
			}

	      /* Rejuvinate the just processed RBD, make it the end-of-list*/
	      outpw(ADDRPORT0, next_rbd);
	      outpw(DATAPORT0, 0);	/* Clear the status so we don't*/
					/* trip ourselves later. */
	      outpw(ADDRPORT0, next_rbd+4);
	      outpw(DATAPORT0, RCV_BUFF_SIZE | RBD_EL); /*Turn on EL flag */

	      /* Remove the end-of-list marker from the prececessor RBD */
	      outpw(ADDRPORT0, prior_rbd+4);
	      outpw(DATAPORT0, RCV_BUFF_SIZE);	/* Turn off the EL flag */

	      prior_rbd = next_rbd;
	      next_rbd = rbd.rbd_link >> 1;

	      if (rbd.rbd_act_cnt & RBD_EOF) then break;
	      } /* End of the for(;;) */

	  /* Send the packet (if any) to the demultiplexor */
	  if (skip == TAKE_PKT)
	     then {
		  if ((len == 0) || drop_pkt)
		     then { /* Abandon wrong size packets */
			  putfree(inpack);
			  long_cnt++;
			  }
		     else {
			  inpack->nb_tstamp = cticks;
			  inpack->nb_len = len;
			  q_addt(et_net->n_inputq, (q_elt)inpack);
			  tk_wake(EtDemux);
			  }
	          }
	     else skipped_cnt++;
	  }
  }

return 1;
}
#else	/* (!defined(WATCH)) */
/* Extract the frame from an FD for netwatch */
static int
drain_fd(skip)
	int	skip;	/* 1: send incoming data to /dev/null.		*/
			/*    In this mode we assume the 586 is stopped	*/
			/* 0: try to accept the incoming data.		*/
{
uchar		*bp;

/* Check whether there is space in the circular queue */
if ((skip != SKIP_PKT) && (((pproc - prcv) & PKTMASK) == 1)) return 0; 

/* Read in the top part of the RFD of interest. */
/* We use direct I/O for speed.			*/
outpw(ADDRPORT0, next_rfd);
if (!((srfd.rfd_stat = inpw(DATAPORT0)) & RFD_C))
   then return 0;	/* Return if no frame in the RFD */

srfd.rfd_el_s = inpw(DATAPORT0);
srfd.rfd_link = inpw(DATAPORT0);
srfd.rfd_rbd_offset = inpw(DATAPORT0);

if (skip == TAKE_PKT)
   then {
	bp = pkts[prcv].p_data;
	block_in(next_rfd+4, bp, ADDRLEN + ADDRLEN + 2);
	bp += sizeof(struct ethhdr);
	}

/* Update the error counters from the frame */
if (srfd.rfd_stat & RFD_SHORT) then short_cnt++;

/* The rest of the error counters are maintained by the 586 in the SCB	*/
rcv_cnt++;

/* We've got everything we need to know from this RFD.		*/
/* So we can give the real, on-board one back to the 82586.	*/
/* We do this by making this RFD the end-of-list and then	*/
/* removing the end-of-list flag from the "prior" RFD.		*/
outpw(ADDRPORT0, next_rfd);
outpw(DATAPORT0, 0);		/* Clear the status so we don't trip */
				/* ourselves later. */
outpw(DATAPORT0, RFD_EL);	/* Turn on the EL flag */
outpw(ADDRPORT0, next_rfd+3);	/* Address the RBD offset word of the RFD */
outpw(DATAPORT0, 0xFFFF);	/* Insert a null RBD linkage */

outpw(ADDRPORT0, prior_rfd+1);	/* Address the EL & S word of the RFD */
outpw(DATAPORT0, 0);		/* Turn off the EL flag */

prior_rfd = next_rfd;
next_rfd = srfd.rfd_link >> 1;	/* link is in address form, we need it	*/
				/* i/o port address form. */

/* Now deal with the RBD's, if any, that have the frame's data */
  {
  uint  rbd_offset;
  uint	len;		/* #octets copied into the buffer	*/
  uint	rlen;		/* Actual packet length			*/

  if ((rbd_offset = srfd.rfd_rbd_offset) != 0xFFFF)
     then {
#if defined(RBD_DEBUG)
          /* Sanity check: rbd_offset should equal next_rbd */
	  if ((rbd_offset>>1) != next_rbd)
	     then {
	          printf("RBD_OFFSET %x != NEXT_RBD %x\n",
			  rbd_offset>>1, next_rbd);
		  trw_close();
		  exit(1);
		  }
#endif
	  len = rlen = sizeof(struct ethhdr);

	  for (;;)
	      {
	      block_in(next_rbd, &rbd, sizeof(rbd_t));

#if defined(RBD_DEBUG)
	      if (!(rbd.rbd_act_cnt & RBD_F))	/* Make sure RBD is valid */
	         then {
		      printf("Incomplete RBD\n");
		      trw_close();
		      exit(1);
		      }
#endif
		if (skip == TAKE_PKT)
		   then {
			uint chunk_sz;
			chunk_sz = rbd.rbd_act_cnt & RBD_COUNT_MASK;
			rlen += chunk_sz;
			/* Prevent buffer overruns */
			if ((chunk_sz + len) > MATCH_DATA_LEN)
			   then chunk_sz = MATCH_DATA_LEN - len;
			if (chunk_sz != 0)
			   then {
				/* Use chunk_sz+1 to ensure we	*/
				/* get the last byte because	*/
				/* block_in moves words.	*/
				block_in(rbd.rbd_buff_ioaddr, bp,
					chunk_sz+1);
				bp += chunk_sz;
				len += chunk_sz;
				}
			}

	      /* Rejuvinate the just processed RBD, make it the end-of-list*/
	      outpw(ADDRPORT0, next_rbd);
	      outpw(DATAPORT0, 0);	/* Clear the status so we don't*/
					/* trip ourselves later. */
	      outpw(ADDRPORT0, next_rbd+4);
	      outpw(DATAPORT0, RCV_BUFF_SIZE | RBD_EL); /* Turn on EL flag*/

	      /* Remove the end-of-list marker from the prececessor RBD */
	      outpw(ADDRPORT0, prior_rbd+4);
	      outpw(DATAPORT0, RCV_BUFF_SIZE);	/* Turn off the EL flag */

	      prior_rbd = next_rbd;
	      next_rbd = rbd.rbd_link >> 1;

	      if (rbd.rbd_act_cnt & RBD_EOF) then break;
	      } /* End of the for(;;) */

	  /* Send the packet (if any) to netwatch */
	  if (skip == TAKE_PKT)
	     then {
		  pkts[prcv].p_len = rlen;
		  prcv = (prcv + 1) & PKTMASK;
		  }
	     else skipped_cnt++;
	  }
  }

return 1;
}
#endif
