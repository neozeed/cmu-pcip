#include <task.h>
#include <q.h>
#include <netq.h>
#include <net.h>
#include <custom.h>
#include <netbuf.h>
#include <stdio.h>
#include "pcnet.h"

extern NET *pc_net;
extern long cticks;
extern struct ncb ncb;

#define TH 500
static int id;

pc_send(p, prot, len, fhost)
PACKET p;
unsigned prot;
unsigned len;
in_name fhost;
{
	register unsigned char *q;
	char sav[6];

	switch(prot) {
		case IP:
			if(fhost == 0xffffffffL || fhost == pc_net->n_netbr ||
				fhost == pc_net->n_netbr42 ||
				fhost == pc_net->n_subnetbr) {
printf("PC broadcast request ignored %a\n, fhost");
				return(0);
			}
			if(len < TH) {
				q = p->nb_prot - 6;
				*q = 0;
				ncb.ncb_buffer = (char far *)q;
				ncb.ncb_length = len + 6;
				*(in_name *)&ncb.ncb_callname[12] = fhost;
				pc(&ncb);
				if(ncb.ncb_retcode) {
printf("PC send failed %d\n", ncb.ncb_retcode);
					return(0);
				}
				return(len);
			}
{int i; for(i = 0; i < 2000; i++); }
			if(!id)
				id = cticks;
			q = p->nb_prot - 6;
			q[1] = id;
			*(in_name *)&q[2] = pc_net->ip_addr;
			*q = 1;
			ncb.ncb_buffer = (char far *)q;
			ncb.ncb_length = TH + 6;
			*(in_name *)&ncb.ncb_callname[12] = fhost;
			pc(&ncb);
			if(ncb.ncb_retcode) {
printf("PC send 1 failed %d\n", ncb.ncb_retcode);
				return(0);
			}
{int i; for(i = 0; i < 20000; i++); }
			q = p->nb_prot + TH - 6;
			memcpy(sav, q, sizeof(sav));
			q[1] = id++;
			*(in_name *)&q[2] = pc_net->ip_addr;
			*q = 2;
			ncb.ncb_buffer = (char far *)q;
			ncb.ncb_length = len + 6 - TH;
			pc(&ncb);
			memcpy(q, sav, sizeof(sav));
			if(ncb.ncb_retcode) {
printf("PC send 2 failed %d\n", ncb.ncb_retcode);
				return(0);
			}
			return(len);

		default:
			printf("pc_send type %d\n", prot);
			return(0);
	}
}
