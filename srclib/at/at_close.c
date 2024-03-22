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
extern NBPTabEntry NBPTE;
extern int mysock;

at_close()
{
	at_switch(0);
	if(mysock) {
		DDPP.atd_command = DDPCloseSocket;
		at(&DDPP);
		NBPP.atd_command = NBPRemove;
		NBPP.nbp_entptr = (BuffPtr)NBPTE.tab_tuple.ent_name;
		at(&NBPP);
	}
}
