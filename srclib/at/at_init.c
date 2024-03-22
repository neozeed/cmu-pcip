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

NET *at_net;
char inetbuf[30];
int mysock = 1;
static struct IPGP IPGP = { ipgpAssign };
static InfoParams INP = { ATGetNetInfo };
DDPParams DDPP, DDPPr;
NBPParams NBPP;
NBPTuple NBPT;
NBPTabEntry NBPTE;
static ATPParams ATPP;
static BDSElement BDSE;

at(param)
char *param;
{
	return(CallATDriver((char far *)param));
}

int at_demux();

at_init(net, options, dummy)
NET *net;
unsigned options;
unsigned dummy;
{
	register unsigned char *p;

	at_net = net;
	if(!isATLoaded()) {
		printf("ATALK.SYS not loaded\n");
		exit(1);
	}
	at(&INP);
	if(INP.atd_status) {
		INP.atd_command = ATInit;
		at(&INP);
	}
	DDPP.atd_command = DDPOpenSocket;
	DDPP.ddp_socket = 72;
	DDPP.ddp_type = 22;
	at(&DDPP);
	if(DDPP.atd_status) {
		if(DDPP.atd_status == DDP_SKTERR)
			mysock = 0;
		else {
			printf("DDP open failed %d\n", DDPP.atd_status);
			exit(1);
		}
	}
	else if(DDPP.ddp_socket != 72) {
		mysock = 0;
		DDPP.atd_command = DDPCloseSocket;
		at(&DDPP);
		DDPP.ddp_socket = 72;
	}
	DDPPr = DDPP;
	DDPP.atd_command = DDPWrite;
	DDPPr.atd_command = DDPRead|AsyncMask;
#ifdef	SADDR
	if(!at_net->ip_addr) {
#endif
	NBPP.atd_command = NBPLookup;
	NBPP.nbp_toget = 1;
	NBPP.nbp_buffptr = (BuffPtr)&NBPT;
	NBPP.nbp_buffsize = sizeof(NBPT);
	NBPP.nbp_interval = 5 * 3;
	NBPP.nbp_retry = 5;
	NBPP.nbp_entptr = (BuffPtr)"\1=\11IPGATEWAY\1*";
	at(&NBPP);
	if(NBPP.nbp_toget != 1 || NBPP.atd_status) {
		printf("NB: No IPGATEWAY %d\n", NBPP.atd_status);
		exit(1);
	}
	BDSE.bds_buffptr = (BuffPtr)&IPGP;
	BDSE.bds_buffsize = sizeof(IPGP);
	ATPP.atd_command = ATPSendRequest;
	ATPP.atp_addrblk = NBPT.ent_address;
	ATPP.atp_buffptr = (BuffPtr)&IPGP;
	ATPP.atp_buffsize = sizeof(IPGP);
	ATPP.atp_interval = 5;
	ATPP.atp_retry = 5;
	ATPP.atp_flags = XObit;
	ATPP.atp_bdsbuffs = 1;
	ATPP.atp_bdsptr = (BuffPtr)&BDSE;
	at(&ATPP);
	if(ATPP.atp_bdsbuffs != 1 || ATPP.atd_status) {
		printf("AT: No IPGATEWAY %d\n", ATPP.atd_status);
		exit(1);
	}
	if(IPGP.opcode == ipgpError) {
		printf("Gateway error: %s\n", IPGP.string);
		exit(1);
	}
	at_net->n_custom->c_me = at_net->ip_addr = IPGP.ipaddress;
	if(at_net->n_custom->c_dm_numname < 3)
		at_net->n_custom->c_dm_numname++;
	at_net->n_custom->c_dm_servers[at_net->n_custom->c_dm_numname - 1]
		= IPGP.ipname;
	at_net->n_custom->c_net_mask = 0L;
#ifdef	SADDR
	}
#endif
	NBPTE.tab_tuple.ent_address.nodeid = INP.inf_nodeid;
	NBPTE.tab_tuple.ent_address.socket = DDPP.ddp_socket;
	p = (unsigned char *)&at_net->ip_addr;
	sprintf(inetbuf, "%d.%d.%d.%d", *p, p[1], p[2], p[3]);
/*printf("My address = %s\n", inetbuf);*/
	sprintf(NBPTE.tab_tuple.ent_name, "%c%s\11IPADDRESS\1*",
		strlen(inetbuf), inetbuf);
	NBPP.atd_command = NBPRegister;
	NBPP.nbp_buffptr = (BuffPtr)&NBPTE;
	NBPP.nbp_interval = 1;
	NBPP.nbp_retry = 3;
	if(mysock) {
		at(&NBPP);
		if(NBPP.atd_status) {
			printf("NBP register failed %d\n", NBPP.atd_status);
			exit(1);
		}
	}
	at_net->n_demux =tk_fork(tk_cur,at_demux,at_net->n_stksiz,"ATD",at_net);
	if(at_net->n_demux == NULL) {
		printf("ATD setup failed\n");
		exit(1);
	}
}
