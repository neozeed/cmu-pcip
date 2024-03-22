#include <task.h>
#include <q.h>
#include <netq.h>
#include <net.h>
#include <custom.h>
#include <netbuf.h>
#include <stdio.h>
#include "pcnet.h"

NET *pc_net;

pc(param)
char *param;
{
	return(pcnet((char far *)param));
}

int pc_demux();
struct ncb ncb;

pc_init(net, options, dummy)
NET *net;
unsigned options;
unsigned dummy;
{
	pc_net = net;
	ncb.ncb_command = NCB_ADD_NAME;
	memcpy(ncb.ncb_name, "PCIPPCIPPCIP", 12);
	*(in_name *)&ncb.ncb_name[12] = pc_net->ip_addr;
	pc(&ncb);
	if(ncb.ncb_retcode && ncb.ncb_retcode != 13) {
		printf("add_name failed %d\n", ncb.ncb_retcode);
		exit(1);
	}
	ncb.ncb_command = NCB_SEND_DATAGRAM;
	memcpy(ncb.ncb_callname, ncb.ncb_name, 12);
	pc_net->n_demux =tk_fork(tk_cur,pc_demux,pc_net->n_stksiz,"PCD",pc_net);
	if(pc_net->n_demux == NULL) {
		printf("PCD setup failed\n");
		exit(1);
	}
}
