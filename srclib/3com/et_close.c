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

#include <task.h>
#include <q.h>
#include <netq.h>
#include <net.h>
#include <custom.h>
#include <netbuf.h>
#include <stdio.h>
#include "3com.h"

extern char save_mask;

/* Shutdown the ethernet interface */

et_close() {
	int vec;	/* crock to avoid compiler bug */

#ifdef WATCH
#define	RCVMULT	ERCVMULTI | EACCDRIBBLE | EACCGOODFRAMES | EDTSHORTFRAMES | EOVERFLOW
	outb(ERCVCMD,RCVMULT);	/* leave interface in start state. */
#endif
	int_off();
	outb(IIMR, inb(IIMR) | save_mask); /* restore original mask */
	outb(EAUXCMD, ERESET);	   /* LKR - turn off Ethernet board */
	outb(EAUXCMD, ESYSBUS);	   /* DDP - clear reset */
	et_unpatch();
	int_on();
}
