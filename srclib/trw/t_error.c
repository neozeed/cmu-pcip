/*  Copyright 1986 by Carnegie Mellon  */
/*  See permission and disclaimer notice in file "cmu-note.h"  */
#include	<cmu-note.h>
/*  Copyright 1984, 1985 by the Massachusetts Institute of Technology  */
/*  See permission and disclaimer notice in file "notice.h"  */
#include	<notice.h>

/* Copyright 1988 TRW Information Networks */
/*  See permission and disclaimer notice in file "trw-note.h"  */
#include	"trw-note.h"

/* $Revision:   1.1  $		$Date:   22 Mar 1988 22:40:00  $	*/

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

static char trw_msgid[] = "TRW PC-2000 adapter";

typedef struct
	{
	uchar	*errtext;
	uchar	errcode;
	} errmsg_t;

static errmsg_t errmsg[] = {
	"can't spawn demultiplexor",			ERR_SETUPFAIL,
	"bad board type (base address may be incorrect)",
							ERR_BAD_BRD_TYPE,
	"board newer than software can handle",		ERR_NEW_BOARD,
	"can't work with intelligent board",		ERR_INTELLIGENT,
	"memory error",					ERR_BAD_MEMORY,
	"failed to initiate (interrupt level may be incorrect)",
							ERR_INITIATE_FAIL,
	"incorrect initiation",				ERR_BAD_INITIATE,
	"failed to configure",			      	ERR_CONFIG_FAIL,
	"transceiver cable problem",		      	ERR_CABLE_PROB,
	"link cable open",			      	ERR_OPEN_CABLE,
	"link cable shorted",			      	ERR_CABLE_SHORT,
	"indeterminate net problem",		      	ERR_INDETERMINATE,
	"software error",			     	ERR_SOFTWARE,
	"configuration failure or cable not connected",	ERR_CONFIG_OR_CABLE,
	"keepalive timer setup failed",			ERR_KEEPALIVE_FAIL,
	"IRQ level may not exceed 15",			ERR_IRQ_TOO_HI,
	"invalid address selection option",	      	ERR_BAD_ADDRTYPE,
	"undefined error",				0
	};

void
brd_error(code)
	int code;
{
register errmsg_t *ep;

trw_close();
for (ep = errmsg;; ep++)
   {
   if ((ep->errcode == code) || (ep->errcode == 0))
      then {
	   printf("Error: %s - %s\n", trw_msgid, ep->errtext);
	   exit(1);
	   }
   }
/* NOTREACHED */
}
