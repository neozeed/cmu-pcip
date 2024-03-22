/*  Copyright 1986 by Carnegie Mellon  */
/*  See permission and disclaimer notice in file "cmu-note.h"  */
#include	<cmu-note.h>
/*  Copyright 1984 by the Massachusetts Institute of Technology  */
/*  See permission and disclaimer notice in file "notice.h"  */
#include	<notice.h>

/* Copyright 1988 TRW Information Networks */
/*  See permission and disclaimer notice in file "trw-note.h"  */
#include	"trw-note.h"

/* $Revision:   1.1  $		$Date:   27 Mar 1988 18:20:46  $	*/

#include <task.h>
#include <q.h>
#include <netq.h>
#include <net.h>
#include <custom.h>
#include <netbuf.h>
#include <stdio.h>
#include <int.h>
#include "trw.h"

/* Shutdown the ethernet interface */

void
trw_close()
{
_disable();
HALT_BOARD();		/* Make sure it is stopped and mapped out.	*/
if (trw_eoi_1 != 0)
   then trw_eoi_int();	/* Make sure there are no unacknowledged ints	*/
if (mask_saved == 1)
   then {
	if (trw_eoi_2 == 0)	/* Is there a second 8259 involved?	*/
	   then { /* Only one 8259 */
		outp(IIMR, inp(IIMR) | save_mask); /* restore original mask */
		}
	   else { /* There is a second 8259 */
		outp(IIMR2, inp(IIMR2) | save_mask); /*restore original mask*/
		}
	mask_saved = 0;
	}
trw_unpatch();		/* unpatch is self protecting against attempts	*/
			/*    to remove a non-existant patch.		*/
driver_state = DS_UNINITIALIZED;
_enable();
}
