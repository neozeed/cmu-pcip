/*  Copyright 1986 by Carnegie Mellon  */
/*  See permission and disclaimer notice in file "cmu-note.h"  */
#include	<cmu-note.h>

static int (*closers[30])();
static int nclosers = 0;
#ifdef MSC40			/* DDP Use only for MSC > v4.0 */
void pcip_exit ();		/* LKR predeclare our exit func */
#endif

exit_hook(func)
	int (*func)(); {

/*	printf("exit_hook: adding func %04x\n", func);	*/

#ifdef MSC40			/* DDP Use only for MSC > v4.0 */
	if (!nclosers)		/* LKR - use MSC V4.0 facility */
	    onexit(pcip_exit);
#endif

	closers[++nclosers] = func;
	}

#ifdef MSC40			/* DDP Use only for MSC > v4.0 */
void pcip_exit(code) {		/* LKR - changed name from exit()*/
#else
exit(code) {
#endif
	int n;

	for(n=nclosers; n; n--) {
/*		printf("exit: calling func %04x\n", closers[n]);	*/
		(*closers[n])();
		}

#ifndef MSC40			/* DDP Use only for MSC == v3.0 */
	_exit(code);
#endif
}
