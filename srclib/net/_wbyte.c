/*  Copyright 1986 by Carnegie Mellon  */
/*  See permission and disclaimer notice in file "cmu-note.h"  */
#include	<cmu-note.h>
#include <stdio.h>

_wbyte(x, p)
	unsigned x;
	FILE *p; {

/*	return(--(p)->_cnt>=0?((int)(*(p)->_ptr++=(unsigned)(x))):_flsbuf((unsigned)(x),p));	DDP */
	return(--(p)->_cnt>=0? 0xff & (*(p)->_ptr++= (x)):_flsbuf((x),p)); /* DDP */
	}
