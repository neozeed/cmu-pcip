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

/* close a udp connection - remove the connection from udp's list of
	connections and deallocate it.
   But only if the connection is not null.  1/16/84 <J. H. Saltzer>
*/

udp_close(con)
	register UDPCONN con; {
	register UDPCONN pcon;

	if (con == 0) return;
	if(firstudp == con) firstudp = 0;
	else {
		for(pcon=firstudp; pcon && pcon->u_next != con;
						pcon=pcon->u_next)

		if(!pcon)
			return 0;

		pcon->u_next = con->u_next;
		}

	cfree(con);
	}
