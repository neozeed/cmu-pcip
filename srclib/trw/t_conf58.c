/*  Copyright 1986 by Carnegie Mellon  */
/*  See permission and disclaimer notice in file "cmu-note.h"  */
#include	<cmu-note.h>
/*  Copyright 1984, 1985 by the Massachusetts Institute of Technology  */
/*  See permission and disclaimer notice in file "notice.h"  */
#include	<notice.h>

/* Copyright 1988 TRW Information Networks */
/*  See permission and disclaimer notice in file "trw-note.h"  */
#include	"trw-note.h"

/* $Revision:   1.2  $		$Date:   22 Mar 1988 22:40:32  $	*/

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
#include <int.h>
#include "trw.h"

/* Define the eight blocks of configuration parameters corresponding to	*/
/* the eight possible configurations found in the board's configuration	*/
/* byte.								*/
static	config_block_t	conf_blks[8] = {
	/* Block 0 -- Ethernet/802.3: Baseband, 10Mbits			*/
	11,		/* B6:  # of config bytes (including this one)  */
	8,		/* B7:  FIFO-LIM				*/
	0x40,		/* B8:  SRDY					*/
	0x20|0x06,	/* B9:  PREAM_LEN=8, ADDR_LEN=6			*/
	0,		/* B10: linear priority				*/
	96,		/* B11: interframe spacing			*/
	0,		/* B12: slot time of 512 (low part)		*/
	(15 << 4) | 2,	/* B13: 15 retries | slot time of 512 (hi part)	*/
	0,		/* B14: PRM=0, TONO_CRS=0			*/
	0,		/* B15: 					*/
	64,		/* B16: Min frame leng				*/
	0,		/* unused					*/

	/* Block 1 -- Broadband, 10Mbits				*/
	11,		/* B6:  # of config bytes (including this one)  */
	8,		/* B7:  FIFO-LIM				*/
	0x40,		/* B8:  SRDY					*/
	0x20|0x06,	/* B9:  PREAM_LEN=8, ADDR_LEN=6			*/
	0,		/* B10: linear priority				*/
	96,		/* B11: interframe spacing			*/
	0,		/* B12: slot time of 512 (low part)		*/
	(15 << 4) | 2,	/* B13: 15 retries | slot time of 512 (hi part)	*/
	0,		/* B14: PRM=0, TONO_CRS=0			*/
	0,		/* B15: 					*/
	64,		/* B16: Min frame leng				*/
	0,		/* unused					*/

	/* Block 2 -- Baseband, 5Mbits					*/
	11,		/* B6:  # of config bytes (including this one)  */
	8,		/* B7:  FIFO-LIM				*/
	0x40,		/* B8:  SRDY					*/
	0x20|0x06,	/* B9:  PREAM_LEN=8, ADDR_LEN=6			*/
	0,		/* B10: linear priority				*/
	96,		/* B11: interframe spacing			*/
	0,		/* B12: slot time of 512 (low part)		*/
	(15 << 4) | 2,	/* B13: 15 retries | slot time of 512 (hi part)	*/
	0,		/* B14: PRM=0, TONO_CRS=0			*/
	0,		/* B15: 					*/
	64,		/* B16: Min frame leng				*/
	0,		/* unused					*/

	/* Block 3 -- Broadband, 5Mbits					*/
	11,		/* B6:  # of config bytes (including this one)  */
	8,		/* B7:  FIFO-LIM				*/
	0x40,		/* B8:  SRDY					*/
	0x20|0x06,	/* B9:  PREAM_LEN=8, ADDR_LEN=6			*/
	0,		/* B10: linear priority				*/
	96,		/* B11: interframe spacing			*/
	0,		/* B12: slot time of 512 (low part)		*/
	(15 << 4) | 2,	/* B13: 15 retries | slot time of 512 (hi part)	*/
	0,		/* B14: PRM=0, TONO_CRS=0			*/
	0,		/* B15: 					*/
	64,		/* B16: Min frame leng				*/
	0,		/* unused					*/

	/* Block 4 -- Baseband, 2Mbits					*/
	11,		/* B6:  # of config bytes (including this one)  */
	8,		/* B7:  FIFO-LIM				*/
	0x40,		/* B8:  SRDY					*/
	0x20|0x06,	/* B9:  PREAM_LEN=8, ADDR_LEN=6			*/
	0,		/* B10: linear priority				*/
	96,		/* B11: interframe spacing			*/
	0,		/* B12: slot time of 512 (low part)		*/
	(15 << 4) | 2,	/* B13: 15 retries | slot time of 512 (hi part)	*/
	0,		/* B14: PRM=0, TONO_CRS=0			*/
	0,		/* B15: 					*/
	64,		/* B16: Min frame leng				*/
	0,		/* unused					*/

	/* Block 5 -- Broadband, 2Mbits					*/
	11,		/* B6:  # of config bytes (including this one)  */
	8,		/* B7:  FIFO-LIM				*/
	0x40,		/* B8:  SRDY					*/
	0x20|0x06,	/* B9:  PREAM_LEN=8, ADDR_LEN=6			*/
	0,		/* B10: linear priority				*/
	96,		/* B11: interframe spacing			*/
	0,		/* B12: slot time of 512 (low part)		*/
	(15 << 4) | 2,	/* B13: 15 retries | slot time of 512 (hi part)	*/
	0,		/* B14: PRM=0, TONO_CRS=0			*/
	0,		/* B15: 					*/
	64,		/* B16: Min frame leng				*/
	0,		/* unused					*/

	/* Block 6 -- Baseband, 1Mbit					*/
	11,		/* B6:  # of config bytes (including this one)  */
	8,		/* B7:  FIFO-LIM				*/
	0x40,		/* B8:  SRDY					*/
	0x20|0x06,	/* B9:  PREAM_LEN=8, ADDR_LEN=6			*/
	0,		/* B10: linear priority				*/
	96,		/* B11: interframe spacing			*/
	0,		/* B12: slot time of 512 (low part)		*/
	(15 << 4) | 2,	/* B13: 15 retries | slot time of 512 (hi part)	*/
	0,		/* B14: PRM=0, TONO_CRS=0			*/
	0,		/* B15: 					*/
	64,		/* B16: Min frame leng				*/
	0,		/* unused					*/

	/* Block 7 -- Broadband, 1Mbit					*/
	11,		/* B6:  # of config bytes (including this one)  */
	8,		/* B7:  FIFO-LIM				*/
	0x40,		/* B8:  SRDY					*/
	0x20|0x06,	/* B9:  PREAM_LEN=8, ADDR_LEN=6			*/
	0,		/* B10: linear priority				*/
	96,		/* B11: interframe spacing			*/
	0,		/* B12: slot time of 512 (low part)		*/
	(15 << 4) | 2,	/* B13: 15 retries | slot time of 512 (hi part)	*/
	0,		/* B14: PRM=0, TONO_CRS=0			*/
	0,		/* B15: 					*/
	64,		/* B16: Min frame leng				*/
	0		/* unused					*/
					};

#define LOOPBIT		0x80		/* Bit to turn on in B9 to do	*/
					/* 82586 external loopback.	*/
#define PRM_BIT		0x01		/* Bit to turn on in B14 to do	*/
					/* promiscuous reception.	*/

/* Configure the 82586 and give it the node's individual address */
/* Returns 0 on sucess, -1 on failure.				 */
int
i586_config(mode)
	unsigned int mode;	/* 0, LOOPBACK, ALLPACK, or MULTI	*/
{
config_cb_t	conf_cb;
ias_cb_t	ias_cb;
cb_list_el_t	*conf_cbp;
cb_list_el_t	*ias_cbp;

tdr_cb_t	tdr_cb;
cb_list_el_t	*tdr_cbp;

_disable();
init_CB_lists();	/* Build the CB, RFD, and RBD lists */

/* We assume that the following get_free_CB()s can't fail */
conf_cbp = get_free_CB();
ias_cbp = get_free_CB();

/* We use the custom structure's transmit DMA value to indicate whether	*/
/* to do the TDR check.  If tx_dma == 0 we do the TDR.			*/
if (custom.c_tx_dma == 0) tdr_cbp = get_free_CB();

/*conf_cb.cb_config_hdr.cb_stat = 0; -- The 586 writes the stat field */

conf_cb.cb_config_hdr.cb_command = CB_CONFIGURE;
conf_cb.cb_config_hdr.cb_link = ias_cbp->cb_link_offset;
memcpy(&conf_cb.cb_config_blk,
       &conf_blks[board_id.configuration & BOARD_CONFIG_MASK],
       sizeof(config_block_t));

#if defined(XLOOP)
conf_cb.cb_config_blk.cb_config_9 |= LOOPBIT;
#else
if (mode == LOOPBACK)
   then conf_cb.cb_config_blk.cb_config_9 |= LOOPBIT;
   else if (mode == ALLPACK)
      then conf_cb.cb_config_blk.cb_config_14 |= PRM_BIT;
#endif

block_out(conf_cbp->cb_iomm_addr, &conf_cb, sizeof(config_cb_t));

/*ias_cb.cb_ias_hdr.cb_stat = 0; -- The 586 writes the stat field */

if (custom.c_tx_dma == 0)
   then { /* Do the TDR check */
	ias_cb.cb_ias_hdr.cb_command = CB_IA_SETUP;
	ias_cb.cb_ias_hdr.cb_link = tdr_cbp->cb_link_offset;
	}
   else { /* Skip the TDR check */
	ias_cb.cb_ias_hdr.cb_command = CB_IA_SETUP | CB_END_LIST;
	}
etadcpy(_etme, ias_cb.cb_ias_ia);

block_out(ias_cbp->cb_iomm_addr, &ias_cb, sizeof(ias_cb_t));

if (custom.c_tx_dma == 0)
   then { /* Do the TDR check */
	/* tdr_cb.cb_tdr_hdr.cb_stat = 0; -- The 586 writes the stat field */
	tdr_cb.cb_tdr_hdr.cb_command = CB_TDR | CB_END_LIST;
	block_out(tdr_cbp->cb_iomm_addr, &tdr_cb, sizeof(tdr_cb_t));
	}

int_flag = 0;
start_cb(conf_cbp->cb_link_offset);
_enable();

   {
   unsigned long expires;
   expires = cticks + 18;  /* We will wait as long as 1 second on this one */
			   /* Again, this is probably much longer than is  */
			   /* really needed.				   */
   while (cticks < expires)
      {
      if (int_flag != 0) goto got_config_int;
      }
   brd_error(ERR_CONFIG_OR_CABLE);	/* This does not return */
   }

got_config_int:

_disable();
if (get_scb_status() != CNA)
   then { /* The 82586 didn't start correctly */
        brd_error(ERR_CONFIG_FAIL);
	}

command_scb(ACK_CNA);
spin_on_scb_command();		/* Could this hang forever? */

if (custom.c_tx_dma == 0)
   then { /* Check the TDR results */
	block_in(tdr_cbp->cb_iomm_addr, &tdr_cb, sizeof(tdr_cb_t));
	if (!(tdr_cb.cb_tdr_result & TDR_LINK_OK))
	   then switch (tdr_cb.cb_tdr_result & TDR_PROBLEM_MASK)
		   {
		   case TDR_XCVR_PROB:
			brd_error(ERR_CABLE_PROB);
			break;
		   case TDR_CABLE_OPEN:
			brd_error(ERR_OPEN_CABLE);
			break;
	           case TDR_CABLE_SHORT:
			brd_error(ERR_CABLE_SHORT);
			break;
		   default:
			brd_error(ERR_INDETERMINATE);
		   }
	   }

release_CB(conf_cbp);
release_CB(ias_cbp);

if (custom.c_tx_dma == 0) release_CB(tdr_cbp);

_enable();
return 0;
}
