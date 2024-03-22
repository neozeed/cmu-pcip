/*  Copyright 1986 by Carnegie Mellon  */
/*  See permission and disclaimer notice in file "cmu-note.h"  */
#include	<cmu-note.h>

#include <stdio.h>
#include <task.h>
#include <q.h>
#include <netq.h>
#include <net.h>
#include <custom.h>
#include <netbuf.h>
#include <int.h>
#include <sdlc.h>
#include "mb.h"
#include "sccreg.h"

extern char save_mask;

/* Shutdown the MacBridge */

mb_close()
{
	int_off();
	dma_reset();
	outb(IIMR, inb(IIMR) | save_mask); /* restore original mask */
	mb_unpatch();

	/* Reset the MacBridge SCC chip. */
	outb(MB_CHANA_CTL, SCC_WR9);
	outb(MB_CHANA_CTL, SCC_RST_HARD);
	int_on();
}

