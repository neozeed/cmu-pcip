/* Copyright 1988 TRW Information Networks */
/*  See permission and disclaimer notice in file "trw-note.h"  */
#include	"trw-note.h"

/* $Revision:   1.0  $		$Date:   29 Feb 1988 20:20:30  $	*/

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

/* This module contains most of the global data */

uint int_cnt			= 0;
uint rcv_cnt			= 0;
uint send_cnt			= 0;
uint cd_lost_cnt		= 0;
uint cts_lost_cnt		= 0;
uint xmit_dma_under_cnt		= 0;
uint deferred_cnt		= 0;
uint sqe_lost_cnt		= 0;
uint collision_cnt		= 0;
uint ex_coll_cnt		= 0;
uint short_cnt			= 0;
uint long_cnt			= 0;
uint skipped_cnt		= 0;
uint eof_missing_cnt		= 0;
uint toobig_cnt			= 0;
uint refused_cnt		= 0;
uint etdrop			= 0;
uint etmulti			= 0;
uint etwpp			= 0;

uchar		driver_state	= DS_UNINITIALIZED;
uchar		int_flag	= 0;
cb_list_el_t	*current_cb	= CBP_NULL;	/* Currently executing CB */

id_prom_t board_id;	/* A copy of the board's ID PROM		*/
uchar	control_image = 0; /* In-core shadow of the board's control reg */
uchar	mask_saved =	0; /* 1: Old 8259 mask saved, 0: not saved	*/
uchar	save_mask; 	/* 8259 interrupt mask to be restored on close	*/
task	*EtDemux;	/* Ethernet packet demultiplexing task		*/
NET	*et_net;	/* My net pointer				*/

char	ETBROADCAST[6] = { 0xff, 0xff, 0xff, 0xff, 0xff, 0xff};
uchar	_etme[6];	/* my ethernet address */

int dataport0, dataport1, addrport0, addrport1, ctrlport;

/* The following 4 variables contain the I/O mapped memory addresses of	*/
/* various special RFDs and RBDs.					*/
uint	next_rfd, prior_rfd, next_rbd, prior_rbd;

/* The following two variables are used by the assembly language interrupt */
/* handler to acknowledge the first and second interrupt controllers.	   */
/* (trw_eoi_2 is left zero if only the first int controller is used.)	   */
/* (trw_eoi_1 == 0: these variables have not yet been initialized)	   */
uchar	trw_eoi_1	= 0;
uchar	trw_eoi_2	= 0;
