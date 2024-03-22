#include <task.h>
#include <q.h>
#include <netq.h>
#include <net.h>
#include <custom.h>
#include <netbuf.h>
#include <stdio.h>
#include "pcnet.h"

extern struct ncb ncbr;

pc_close()
{
	struct ncb ncb;
	int i;

	int_off();
	ncbr.ncb_post = 0;
	int_on();
	ncb.ncb_lana_num = 0;
	ncb.ncb_command = NCB_CANCEL;
	ncb.ncb_buffer = (char far *)&ncbr;
	if(i = pc(&ncb))
		printf("pc_close: cancel failed %d\n", i);
}
