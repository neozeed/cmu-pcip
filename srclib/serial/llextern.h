/* Copyright 1986 by Carnegie Mellon */
/*  Copyright 1986 by Carnegie Mellon  */
/*  See permission and disclaimer notice in file "cmu-note.h"  */
#include	<cmu-note.h>
/*  Copyright 1984 by the Massachusetts Institute of Technology  */
/*  See permission and disclaimer notice in file "notice.h"  */
#include	<notice.h>

/* This header defines the various externals used by the interrupt handling
	routines. */

extern PACKET RBUFF;
extern char RACK;
extern char INBUSY;
extern char OUTBUSY;
extern char TACK;
extern PACKET TBUFF;
extern char NEEDBUF;

