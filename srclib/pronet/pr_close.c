/*  Copyright 1986 by Carnegie Mellon  */
/*  See permission and disclaimer notice in file "cmu-note.h"  */
#include	<cmu-note.h>
/*  Copyright 1984, 1985 by Proteon, Inc. */
/*  See permission and disclaimer notice in file "proteon-notice.h"  */
#include	"proteon-notice.h"

/*
30-Mar-86 Rearranged pr_close to allow it to do what pr_switch() wants.
					<Drew D. Perkins>
 */

#include <task.h>
#include <q.h>
#include <netq.h>
#include <net.h>
#include <custom.h>
#include "pronet.h"

/* Shutdown the proNET ring interface */

pr_close() {
	int i;

#ifdef DEBUG				/* DDP */
	if(NDEBUG & INFOMSG)
		printf("pr_close() called\n");
#endif					/* DDP */

	int_off();
	outb(mkv2(V2OCSR), 0);
	outb(mkv2(V2ICSR), MODE1|MODE2);
	i = 1 << custom.c_intvec;
	outb(IIMR, inb(IIMR) | i);

	pr_unpatch();
	int_on();
	}
