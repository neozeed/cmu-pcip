/*  Copyright 1986 by Carnegie Mellon  */
/*  See permission and disclaimer notice in file "cmu-note.h"  */
#include	<cmu-note.h>
/*  Copyright 1984 by the Massachusetts Institute of Technology  */
/*  See permission and disclaimer notice in file "notice.h"  */
#include	<notice.h>

#include <task.h>
#include <q.h>
#include <netq.h>
#include <net.h>

/* This routine is called from netcrt to shut down the network. It calls
	the net_close routine associated with each network interface which
	should disable the interface and patch back any interrupt vectors
	which it might have been using.
*/

extern int Nnet;
extern NET nets[];

netclose() {
	int i;
#ifdef DEBUG
	if(NDEBUG & INFOMSG)
		printf("netclose() called\n");
#endif
	for(i=0; i<Nnet; i++)
		if(nets[i].n_close)
			(*(nets[i].n_close))();
		else {
#ifdef DEBUG
		     if(NDEBUG & INFOMSG)
			printf("no close routine!\n");
#endif
		}
	crock_c();
	brk_c();
	}
