/* Dan Lanciani - 1987 */
/* PC/IP driver for AppleTalk/TOPS */
/* Please send changes to ddl@harvard.harvard.edu or sob@harvard.harvard.edu */

#include <task.h>
#include <q.h>
#include <netq.h>
#include <net.h>
#include <custom.h>
#include <netbuf.h>
#include <stdio.h>
#include "at.h"

extern int MaxLnh;
extern NET *at_net;
extern DDPParams DDPPr;
extern long cticks;
PACKET p;


static far
comp()
{
	tk_wake(at_net->n_demux);
}

at_switch(state)
{
	DDPParams DDPP;

	if(state && !DDPPr.atd_compfun) {
		DDPPr.atd_compfun = comp;
		if(p)
			at(&DDPPr);
	}
	else if(!state && DDPPr.atd_compfun) {
		int_off();
		DDPPr.ddp_compfun = 0;
		int_on();
		if(DDPPr.atd_status > 0) {
			DDPP.atd_command = DDPCancel;
			DDPP.atd_compfun = 0;
			DDPP.ddp_buffptr = (BuffPtr)&DDPPr;
			at(&DDPP);
			if(DDPP.atd_status)
				printf("DDPCancel failed %d\n",DDPP.atd_status);
		}
		DDPPr.atd_status = 1;
	}
}

at_demux()
{
	DDPPr.atd_compfun = comp;
	while(1) {
		while(!DDPPr.atd_compfun || !(p = a_getfree()))
			tk_yield();
		DDPPr.ddp_buffptr = (BuffPtr)p->nb_prot;
		DDPPr.ddp_buffsize = LBUF - MaxLnh;
		do {
			at(&DDPPr);
			while(DDPPr.atd_status > 0)
				tk_block();
		} while(DDPPr.atd_status);
		p->nb_tstamp = cticks;
		indemux(p, p->nb_len = DDPPr.ddp_buffsize, at_net);
	}
}
