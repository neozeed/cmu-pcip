/*  Copyright 1986 by Carnegie Mellon  */
/*  See permission and disclaimer notice in file "cmu-note.h"  */
#include	<cmu-note.h>
/*  Copyright 1984 by the Massachusetts Institute of Technology  */
/*  See permission and disclaimer notice in file "notice.h"  */
#include	<notice.h>

#include <stdio.h>

/* Intialize the internet layer. Essentially, let the net drivers know
	we're around. */

int indemux(), in_stats();

in_init() {
	/* so don't do anything, anymore */
	}
