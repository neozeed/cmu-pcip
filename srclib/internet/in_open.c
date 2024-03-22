/*  Copyright 1986 by Carnegie Mellon  */
/*  See permission and disclaimer notice in file "cmu-note.h"  */
#include	<cmu-note.h>
/*  Copyright 1984 by the Massachusetts Institute of Technology  */
/*  See permission and disclaimer notice in file "notice.h"  */
#include	<notice.h>

/*
 10/30/86 Added upcalling support on destination unreachable.
					<Drew D. Perkins>
 */

#include <stdio.h>
#include <types.h>
#include <task.h>
#include <q.h>
#include <netq.h>
#include <net.h>
#include <custom.h>
#include <netbuf.h>
#include <ip.h>
#include "ipconn.h"

/*
 10/30/86 Added upcalling support on destination unreachable.
					<Drew D. Perkins>
 */

/* Open a protocol connection on top of internet. Protocol information
	necessary for packet demultiplexing; handler is upcalled
	upon receipt of packet. handler is
		int handler(p, len, fhost)
			PACKET p
			int len
			in_name fhost
*/

IPCONN ipconn[10];		/* demux table */
int nipconns = 0;			/* current posn in demux table */

IPCONN in_open(prot, handler, du)
	unshort prot;
	int (*handler)();
	int (*du)(); {		/* DDP */
	int i;
	IPCONN conn;
#ifdef DEBUG
	if(NDEBUG & IPTRACE) printf("IP: setting handler for protocol %u\n",
				    prot);
#endif
	for(i=0; i<nipconns; i++)
		if(ipconn[i]->c_prot == prot) return 0;

	conn = (IPCONN)calloc(1, sizeof(struct ip_iob));
	if(conn == 0) return 0;
	conn->c_prot = prot&0xff;
	conn->c_handle = handler;
	conn->c_net = 0;
	conn->du_handle = du;		/* DDP */
	ipconn[nipconns++] = conn;
	return conn;
	}
