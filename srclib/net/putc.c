/*  Copyright 1986 by Carnegie Mellon  */
/*  See permission and disclaimer notice in file "cmu-note.h"  */
#include	<cmu-note.h>
#include <stdio.h>

putc(c, s)
	char c;
	FILE *s; {

/*	if(!(s->_flag & _IOASCII)) return(_wbyte(c, s));	DDP */

/*	if(c == '\n') _wbyte('\r', s);				DDP */
	return _wbyte(c, s);
	}
