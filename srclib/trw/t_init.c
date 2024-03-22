/*  Copyright 1986 by Carnegie Mellon  */
/*  See permission and disclaimer notice in file "cmu-note.h"  */
#include	<cmu-note.h>
/*  Copyright 1984, 1985 by the Massachusetts Institute of Technology  */
/*  See permission and disclaimer notice in file "notice.h"  */
#include	<notice.h>

/* Copyright 1988 TRW Information Networks */
/*  See permission and disclaimer notice in file "trw-note.h"  */
#include	"trw-note.h"

/* $Revision:   1.1  $		$Date:   22 Mar 1988 22:40:08  $	*/

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

/* Setup images of our SCP and ISCP because it is easier than building */
/* them at run time. */
static scp_t	scp = {SCP_BUS_16_BIT, 0, 0, 0, 0, 0,
		       ISCP_PHYS_ADDR_LO_16, ISCP_PHYS_ADDR_HI_8, 0};

static iscp_t	iscp = {0, 0, SCB_OFFSET, SCB_BASE_LO_16, SCB_BASE_HI_8, 0};

void trw_demux(void); 	/* the routine which is the body of the demux task */

/* The following are used by the mechanism that periodically simulates	*/
/* an interrupt.							*/
static void	trw_poke();
static void	trw_keepalive();
static task	*trw_rtk;
static timer	*trw_rtm;

void
trw_init(net, options, dummy)
	NET *net;
	unsigned options;
	unsigned dummy;
{
dataport0 = custom.c_base + BRD_DATAPORT0;
dataport1 = custom.c_base + BRD_DATAPORT1;
addrport0 = custom.c_base + BRD_ADDRPORT0;
addrport1 = custom.c_base + BRD_ADDRPORT1;
ctrlport = custom.c_base + BRD_CTRLPORT;

#ifdef	DEBUG
if(NDEBUG & INFOMSG)
	printf("Forking ETDEMUX.\n");
#endif

EtDemux = tk_fork(tk_cur, trw_demux, net->n_stksiz, "ETDEMUX", net);
if(EtDemux == NULL)
   then {
	brd_error(ERR_SETUPFAIL);
	}

et_net = net;
et_net->n_demux = EtDemux;

trw_switch(1, options);		/* DDP */

tk_yield();	/* Give the per net task a chance to run. */

/* init arp */
etainit();
return;
}

/*
	Routine to switch the board and interrupts on/off.
 */
void
trw_switch(state, options)
	int state;
	unsigned options;
{
uint vec;	/* Address (not the vector number) of the interrupt	*/
		/* vector we will be using.				*/

if (state)
   then { /* Install the driver */
	_disable();		/* Disable interrupts. */

	/* Reset the PC-2000 board in case it was running before. */
	HALT_BOARD();

	/* Now read in the ID PROM.  Should we test its checksum? */
	outpw(ADDRPORT0, 0);
	/* ID prom is visible only through DATAPORT1 */
	portin_b(DATAPORT1, (uchar *)&board_id, sizeof(board_id));

	/* Validate that we have a TRW board */
	if (memcmp(&board_id.space1, " TRW IND ", 9) != 0)
	   then {
		brd_error(ERR_BAD_BRD_TYPE);
		}

	/* Check that we are compatable with the board version */
	if (board_id.compatability > DRIVER_COMPATABILITY_LEVEL)
	   then {
		brd_error(ERR_NEW_BOARD);
		}

	/* While still masking interrupts and holding the reset line to */
	/* the 586, check whether we've got a smart board.		*/
	if (inp(STATUSPORT) & BRD_CPU_FLAG)
	   then { /* We've got a smart board, let's quit now */
		brd_error(ERR_INTELLIGENT);
		}

	/* Load _etme with our correct ethernet address. */
	switch(custom.c_seletaddr)
	   {
	   case HARDWARE:
		etadcpy(board_id.brd_addr, _etme);
		break;
	   case ETINTERNET:
		*((int *)&(_etme[0])) = 0; /* All that to do it in one word */
					   /* mov rather than two byte movs!*/
		(void) memcpy(&(_etme[2]), &(et_net->ip_addr), 4);
		break;
	   case ETUSER:
		etadcpy(custom.c_myetaddr.e_ether, _etme);
		break;
	   default:
		brd_error(ERR_BAD_ADDRTYPE);
	   } /* end of switch(custom.c_seletaddr) */

	/* Check that the configured interrupt level is within range.	*/
        /* (We don't check whether we are on a PC/AT, so levels 8-15 may*/
        /* be invalid in practice.)					*/
	if (custom.c_intvec > 15)
	   then brd_error(ERR_IRQ_TOO_HI);	/* Does not return */

	/* OK, it looks like we are going to go with this board, so we	*/
	/* need to set up some of the PC hardware.			*/

	/* patch in the new interrupt handler and save the old vector.	*/
	/* The interrupt vectors for IRQ0-7 start at address 0x20, for	*/
	/* IRQ8-15 start at address 0x1C0.				*/
	if (custom.c_intvec < 8)
	   then {
		trw_eoi_1 = 0x60 + custom.c_intvec;
		trw_eoi_2 = 0;
		vec = (custom.c_intvec << 2) + INT_BASE1;
		}
	   else {
		trw_eoi_1 = 0x62;
		trw_eoi_2 = 0x60 + custom.c_intvec - 8;
		vec = ((custom.c_intvec - 8) << 2) + INT_BASE2;
		}

	trw_eoi_int();	/* Make sure any prior interrupts are ack'ed */
	trw_patch(vec);

	/* setup 8259 interrupts for the specified line */
	if (trw_eoi_2 == 0)	/* Is there a second 8259 involved?	*/
	   then { /* Only one 8259 */
	        save_mask = inp(IIMR) & (1 << custom.c_intvec);
		outp(IIMR, inp(IIMR) & ~(1 << custom.c_intvec));
		}
	   else { /* There is a second 8259  -- we don't worry about	*/
		  /* the mask of the 1st 8259 because we assume that	*/
		  /* DOS has opened the level corresponding to the	*/
		  /* slave 8259.  Is this a safe assumption?		*/
	        save_mask = inp(IIMR2) & (1 << (custom.c_intvec - 8));
		outp(IIMR2, inp(IIMR2) & ~(1 << (custom.c_intvec - 8)));
		}
	mask_saved = 1;

	/* Write the memory on the board to set the parity bits etc.	*/
        /* Since we will only be using the last two banks, we will only	*/
	/* with those, potentially leaving the others initialized.	*/
	/* NOTE: From this point on, we will be using control_image as	*/
	/* a shadow of what the board's control register should hold.	*/

	/* Select memory bank two because we will be using banks 2 & 3	*/
	/* via the I/O register method of access.  We also need to	*/
	/* deselect the ID Prom by turning on BRD_RDID*			*/

	/* Since we will be dealing with the data and address registers */
	/* we must continue to run disabled for a while.		*/

	/* NOTE: IF THE MEMORY IS NOT INITIALIZED, OR INITIALIZED TO	*/
	/* SOMETHING OTHER THAN 0X0000, THEN THE SCB SETUP BELOW MUST	*/
	/* BE REINSTATED.						*/
	control_image = BRD_PCBS_BANK2 | BRD_RDID;
	outp(CTRLPORT, BRD_PCBS_BANK2 | BRD_RDID);
	   {
	   register unsigned int i = 0;
	   outpw(ADDRPORT0, 0);
	   do {
	      outpw(DATAPORT0, 0x0000);
	      } while (--i != 0);
	   }
	
        /* Set up the SCP, ISCP, and SCB for the 586 so that it has	*/
        /* something to live on when we start by raising CHRST*.	*/
	   {
	   block_out(SCP_PORT_ADDR, &scp, sizeof(scb_t));
	   block_out(ISCP_PORT_ADDR, &iscp, sizeof(iscp_t));

	   /* SINCE WE INITIALIZED MEMORY TO ALL ZEROS JUST A	  */
	   /* MOMENT EARLIER, WE CAN DISPENSE WITH THE FOLLOWING: */
	   /* (void) memset(&scb, 0, sizeof(scb_t));		*/
	   /* block_out(SCB_PORT_ADDR, &scb, sizeof(scb_t));	*/
	   }

	/* Open the board up for interrupts and release the 82586 */
	control_image |= BRD_IRQRST | BRD_CHRST;
	outp(CTRLPORT, control_image);

	/* Start up the 586 (This will cause an interrupt) */
	/* Remember that we are still running disabled.	   */
	int_flag = 0;
	   {
	   unsigned long expires;
	   uint scbstatus;

	   expires = cticks + 6;   /* We will wait at most 1/3 of a second */
				   /* (1/3 sec is probably lots more than  */
				   /* is really necessary.)		   */
	   CH_ATTENTION();
	   _enable();

	   while (cticks < expires)
	      {
	      if (int_flag != 0) goto got_an_int;
	      }
           brd_error(ERR_INITIATE_FAIL);	/* This does not return */

got_an_int:
           _disable();
	   scbstatus = get_scb_status();
	   _enable();
	   if (scbstatus != (CX | CNA))
	      then { /* The 82586 didn't start correctly */
		   brd_error(ERR_BAD_INITIATE);
		   }
	   }

	_disable();
	command_scb(ACK_CX|ACK_CNA);
	spin_on_scb_command();		/* Could this hang forever? */
	_enable();
			
	/* Configure the 82586 */
	if (i586_config(et_net->n_initp1) != 0)
	   then {
		brd_error(ERR_CONFIG_FAIL);	/* Does not return */
		}
	
	trw_rtm = tm_alloc();
	if(trw_rtm == NULL)
	   then brd_error(ERR_KEEPALIVE_FAIL);	/* Does not return */

	trw_rtk = tk_fork(tk_cur, trw_keepalive, 400, "Keepalive", 0);
	if (trw_rtk == NULL)
	   then brd_error(ERR_KEEPALIVE_FAIL);	/* Does not return */

	/* Start receiving packets */
	_disable();
	start_rfd(RFD0_LINK_OFFSET);
	_enable();
	
#ifdef	DEBUG
    if (NDEBUG & INFOMSG)
        {
	register int i;
	printf("PC Ethernet address = ");
	for (i=0; i<6; i++)
	    printf("%02x", _etme[i]&0xff);
	printf ("\n");
	}
#endif
    driver_state = DS_INITIALIZED;
    tk_yield();
    } /* End of if (state) then ... */
    else trw_close();	/* Let trw_close do the work */
}

/* The following two routines cooperate to periodically simulate an	*/
/* interrupt.  This is necessary in case the 82586 reaches a state	*/
/* in which it has consumed all its resources, we are unable to consume	*/
/* any, and no further net traffic is happening.  This is probably a	*/
/* very, very rare situation.						*/
static void
trw_poke()
{
tk_wake(trw_rtk);
}

static void
trw_keepalive()
{
while(1)
   {
   tm_set(INT_POKE_TIME, trw_poke, NULL, trw_rtm);
   tk_block();
   _disable();
   trw_ihnd();
   _enable();
   }
}
