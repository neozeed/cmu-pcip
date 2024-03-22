#include <task.h>
#include <q.h>
#include <netq.h>
#include <net.h>
#include <custom.h>
#include <netbuf.h>
#include <stdio.h>
#include "pcnet.h"

#define NH 4

extern NET *pc_net;
extern int MaxLnh;
extern struct ncb ncb;
struct ncb ncbr;
static int hp;
static PACKET r, h[NH];

static far
comp()
{
	register char *q;
	extern long cticks;

	estods();
	if(!ncbr.ncb_cmd_cplt) {
		r->nb_len = ncbr.ncb_length - 6;
		r->nb_tstamp = cticks;
		q_addt(pc_net->n_inputq, (q_elt)r);
		r = getfree();
		tk_wake(pc_net->n_demux);
	}
	if(r) {
		q = r->nb_prot - 6;
		ncbr.ncb_buffer = (char far *)q;
		ncbr.ncb_length = LBUF - MaxLnh + 6;
		if(pc(&ncbr)) {
			tk_wake(pc_net->n_demux);
			putfree(r);
			r = 0;
		}
	}
	doiret();
}

pc_switch(state)
{
	int i;

	int_on();
	for(i = 0; i < 100; i++);
	if(state && !ncbr.ncb_post) {
		ncbr.ncb_post = comp;
		if(r && (i = pc(&ncbr))) {
			printf("ncb rcv on failed %d\n", i);
			exit(1);
		}
	}
	else if(!state && ncbr.ncb_post)
		pc_close();
}

pc_demux()
{
	register PACKET p;
	register int i;
	unsigned char *q;

	ncbr.ncb_command = NCB_RECEIVE_DATAGRAM|NCB_NOWAIT;
	ncbr.ncb_num = ncb.ncb_num;
	ncbr.ncb_post = comp;
top:
	while(!ncbr.ncb_post || !(r = a_getfree()))
		tk_yield();
	q = r->nb_prot - 6;
	ncbr.ncb_buffer = (char far *)q;
	ncbr.ncb_length = LBUF - MaxLnh + 6;
	if(i = pc(&ncbr)) {
		printf("ncb rcv failed %d\n", i);
		exit(1);
	}
	while(1) {
		while(!(p = (PACKET)aq_deq(pc_net->n_inputq))) {
			if(!r)
				goto top;
			tk_block();
		}
		switch(p->nb_prot[-6]) {

			case 0:
				indemux(p, p->nb_len, pc_net);
				break;

			case 1:
				for(i = 0; i < NH; i++)
					if(h[i] && *(in_name *)(h[i]->nb_prot-4)
						== *(in_name *)(p->nb_prot-4)) {
						putfree(h[i]);
						h[i] = 0;
					}
				for(i = 0; i < NH; i++)
					if(!h[i]) {
						h[i] = p;
						goto done;
					}
				putfree(h[hp]);
				h[hp++] = p;
				if(hp >= NH)
					hp = 0;
			done:
				break;

			case 2:
				for(i = 0; i < NH; i++)
					if(h[i] && *(in_name *)(h[i]->nb_prot-4)
						== *(in_name *)(p->nb_prot-4))
					if(h[i]->nb_prot[-5] == p->nb_prot[-5]){
						if(h[i]->nb_len + p->nb_len >
							LBUF - MaxLnh) {
printf("double packet too big %d %d\n", h[i]->nb_len, p->nb_len);
							putfree(h[i]);
							h[i] = 0;
							break;
						}
						memcpy(h[i]->nb_prot
						+h[i]->nb_len, p->nb_prot,
						p->nb_len);
						h[i]->nb_len += p->nb_len;
						indemux(h[i], h[i]->nb_len,
							pc_net);
						h[i] = 0;
						break;
					}
					else {
						putfree(h[i]);
						h[i] = 0;
					}
				putfree(p);
				break;
		}
	}
}
