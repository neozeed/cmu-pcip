/* Copyright 1988 Bradley N. Davis, Darbick Instructional Software Systems */
/* See permission and disclaimer notice in file "bnd-note.h" */
#include	"bnd-note.h"
/*  Copyright 1986 by Carnegie Mellon  */
/*  See permission and disclaimer notice in file "cmu-note.h"  */
#include	<cmu-note.h>
/*  Copyright 1984 by the Massachusetts Institute of Technology  */
/*  See permission and disclaimer notice in file "notice.h"  */
#include	<notice.h>

/*
30-Mar-86 Added another outb() to et_close to reset the board completely.
					<Larry K. Raper>
 */

#include <types.h>
#include <task.h>
#include <q.h>
#include <netq.h>
#include <net.h>
#include <custom.h>
#include <netbuf.h>
#include <stdio.h>
#include "i82586.h"

extern char save_mask;

/* Shutdown the ethernet interface */

i_close() {
	int vec;	/* crock to avoid compiler bug */

	int_off();
	outb(IIMR, inb(IIMR) | save_mask); /* restore original mask */
	outb(IDISABLE, 0);	   /* LKR - turn off Ethernet transceiver */
#ifdef MI5210
	outb(IINTDIS, 0);
#endif
	SCBPTR->command = ISRESET;
	doca();
	i_unpatch();
	int_on();
}
