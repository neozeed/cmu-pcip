/*  Copyright 1986, 1987 by Carnegie Mellon  */
/*  See permission and disclaimer notice in file "cmu-note.h"  */
#include	<cmu-note.h>

/*
 * History:
 *	May 1987, David C. Kovar (dk1z@andrew.cmu.edu)
 *		Created.
 *	July 1987, Drew D. Perkins (ddp@andrew.cmu.edu)
 *		Fixed up for distribution.
 */

#include <stdio.h>
#include <types.h>
#include <task.h>
#include <q.h>
#include <netq.h>
#include <net.h>
#include <custom.h>
#include <ctype.h>

extern NET nets[];		/* Array of nets to use */
extern int allow_null_ip_addr;


main()
{
	int i;
	unsigned char *cp;

	allow_null_ip_addr = 1;
	Netinit(800);
	printf("Your hardware address is: ");
	for (i = nets[0].n_hal, cp = nets[0].n_haddr; i; i--, cp++) {
		printf("%02x", *cp);
		printf(i > 1 ? ":" : "\n");
	}
}