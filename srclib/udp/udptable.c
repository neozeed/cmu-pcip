/*  Copyright 1986 by Carnegie Mellon  */
/*  See permission and disclaimer notice in file "cmu-note.h"  */
#include	<cmu-note.h>
/*  Copyright 1984 by the Massachusetts Institute of Technology  */
/*  See permission and disclaimer notice in file "notice.h"  */
#include	<notice.h>

#include <stdio.h>
#include <types.h>
#include <task.h>
#include <q.h>
#include <netq.h>
#include <net.h>
#include <custom.h>
#include <netbuf.h>
#include <icmp.h>
#include <ip.h>
#include <udp.h>
#include "internal.h"

/* Dump the internal table of UDP connections. */

udp_table() {
#ifdef DEBUG
	register UDPCONN con;

	printf("\nUDP demux table:\n");
	printf("Local Port\tForn Port\tFhost\t\troutine\tcn\n");

	for(con=firstudp; con; con = con->u_next) {
		printf("%04x\t\t%04x\t %a ",
				con->u_lport, con->u_fport, con->u_fhost);
		printf("\t\t%04x\t%04x\n",
				con->u_rcv, con->u_data);
		}

	printf("\n");
#endif
	}
