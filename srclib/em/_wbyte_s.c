/*  Copyright 1986 by Carnegie Mellon  */
/*  See permission and disclaimer notice in file "cmu-note.h"  */
#include	<cmu-note.h>
/*  Copyright 1984 by the Massachusetts Institute of Technology  */
/*  See permission and disclaimer notice in file "notice.h"  */
#include	<notice.h>

#include <stdio.h>

_wbyte(x, p)
	char x;
	FILE *p; {

	if(p->_file == 1) {
		if (x == '\n') x = '\207';	/* %TDCRL */
		if (x == '\r') x = '\207';	/* %TDCRL */
		supem(x);
		return (unsigned)x;
		}

/*	return(--(p)->_cnt>=0? ((int)(*(p)->_ptr++=(unsigned)(x))):_flsbuf((unsigned)(x),p));	DDP */
	return(--(p)->_cnt>=0? 0xff & (*(p)->_ptr++= (x)):_flsbuf((x),p)); /* DDP */
	}
