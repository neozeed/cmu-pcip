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

extern DDPParams DDPP;
extern NBPParams NBPP;
extern NBPTuple NBPT;
extern NET *at_net;

static in_name incache;
static AddrBlk abcache;
static char ibuf[30], nbuf[50];

at_send(p, prot, len, fhost)
PACKET p;
unsigned prot;
unsigned len;
in_name fhost;
{
	register unsigned char *q;

	switch(prot) {
		case IP:
			if(fhost == 0xffffffffL || fhost == at_net->n_netbr ||
				fhost == at_net->n_netbr42 ||
				fhost == at_net->n_subnetbr) {
printf("AT broadcast request ignored\n");
				return(0);
			}
			if(fhost != incache) {
				NBPP.atd_command = NBPLookup;
				NBPP.nbp_toget = 1;
				NBPP.nbp_buffptr = (BuffPtr)&NBPT;
				NBPP.nbp_buffsize = sizeof(NBPT);
				NBPP.nbp_interval = 5 * 3;
				NBPP.nbp_retry = 5;
				q = (unsigned char *)&fhost;
				sprintf(ibuf, "%d.%d.%d.%d", *q,q[1],q[2],q[3]);
				sprintf(nbuf, "%c%s\11IPADDRESS\1*",
					strlen(ibuf), ibuf);
				NBPP.nbp_entptr = (BuffPtr)nbuf;
				at(&NBPP);
				if(NBPP.nbp_toget != 1 || NBPP.atd_status) {
printf("AT ARP for %s failed %d\n", ibuf, NBPP.atd_status);
					return(0);
				}
				incache = fhost;
				abcache = NBPT.ent_address;
			}
			DDPP.ddp_addr = abcache;
			DDPP.ddp_buffptr = (BuffPtr)p->nb_prot;
			DDPP.ddp_buffsize = len;
			at(&DDPP);
			if(DDPP.atd_status) {
/*printf("AT send failed %d\n", DDPP.atd_status);*/
				return(0);
			}
			return(len);

		default:
			printf("at_send type %d\n", prot);
			return(0);
	}
}
