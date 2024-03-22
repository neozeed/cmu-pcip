/*  Copyright 1986 by Carnegie Mellon  */
/*  See permission and disclaimer notice in file "cmu-note.h"  */
#include	<cmu-note.h>
/*  Copyright 1984 by the Massachusetts Institute of Technology  */
/*  See permission and disclaimer notice in file "notice.h"  */
#include	<notice.h>


#include <stdio.h>

/* FILE *stdin, *stdout, *stderr;	DDP - don't need this */

/* this is a silly kluge to make sure that the new _wbyte will be linked in
*/
int _wbyte();
static int (*foo)() = _wbyte;

stinit() {
/*	space_initialize(0);			   DDP */

	scr_init();

/*	stdin  = fdopen(0, "ra");
	stdout = fdopen(1, "wa");
	stderr = fdopen(2, "wa");		   DDP */
	}
