/*  Copyright 1986 by Carnegie Mellon  */
/*  See permission and disclaimer notice in file "cmu-note.h"  */
#include	<cmu-note.h>
/*  Copyright 1984 by the Massachusetts Institute of Technology  */
/*  See permission and disclaimer notice in file "notice.h"  */
#include	<notice.h>


#include <stdio.h>

/* This function repairs some of the damage that the standard I/O library
	did during initialization, like cleaning up interrupt vectors. */

stdne() {

/*	fclose(stdin);
	fclose(stdout);
	fclose(stderr);		DDP - Don't need these */
	_curse();
	scr_close();
	}
