/*  Copyright 1986 by Carnegie Mellon  */
/*  See permission and disclaimer notice in file "cmu-note.h"  */
#include	<cmu-note.h>
/*  Copyright 1985 by Micom-Interlan Corp. */
/*  See permission and disclaimer notice in file "interlan-notice.h"  */
#include	"il-notice.h"

#include <task.h>
#include <q.h>
#include <netq.h>
#include <net.h>
#include <custom.h>
#include <netbuf.h>
#include <stdio.h>
#include <int.h>
#include "interlan.h"

extern char save_mask;

/* Shutdown the ethernet interface */

il_close() {
	int vec;	/* crock to avoid compiler bug */

#ifdef WATCH
#define	RCVMULT	ERCVMULTI | EACCDRIBBLE | EACCGOODFRAMES | EDTSHORTFRAMES | EOVERFLOW
	/* disable the interface */
	outb(M_MODE, 0);
	outb(RESET, RS_RESET);
#endif
	int_off();
	outb(IIMR, inb(IIMR) | save_mask); /* restore original mask */
	il_unpatch();
	int_on();

	}
