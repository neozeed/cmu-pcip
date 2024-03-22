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
#include <lapb.h>
#include "ax.h"
#include "mpsccreg.h"

extern char save_mask;

/* Shutdown the PC-SDMA */

ax_close()
{
	int_off();
	dma_reset();
	outb(IIMR, inb(IIMR) | save_mask); /* restore original mask */
	ax_unpatch();

	/* Reset the PC-SDMA MPSCC chip */
	inb(MKAX(AX_CHANA_CTL)); /* Make sure register pointer = 0 */
	outb(MKAX(AX_CHANA_CTL), MPSCC_RST_CHANNEL);

	inb(MKAX(AX_CHANB_CTL)); /* Make sure register pointer = 0 */
	outb(MKAX(AX_CHANB_CTL), MPSCC_RST_CHANNEL);

	int_on();
}

