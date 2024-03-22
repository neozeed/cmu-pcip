/*  Copyright 1986 by Carnegie Mellon  */
/*  See permission and disclaimer notice in file "cmu-note.h"  */
#include	<cmu-note.h>
/*  Copyright 1984, 1985 by the Massachusetts Institute of Technology  */
/*  See permission and disclaimer notice in file "notice.h"  */
#include	<notice.h>

/* 11/17/84 - changed to use subnet masks.
					<John Romkey>
   11/17/85 - changed to return error when unable to route.
					<John Romkey>
    7/31/86 - changed to always route broadcast address to local host.
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
#include "redirtab.h"

/* Route a packet.
	Takes the internet address that we want to send a packet to and
	tries to figure out which net interface to send it through.

	Returns NULL when unable to route.
*/

extern NET nets[];
extern int Nnet;

NET *inroute(host, hop1)
	register in_name host;
	in_name *hop1; {
	register int i;

	/* first check through the redirect table for this host */
	for(i=0; i<REDIRTABLEN && redtab[i].rd_dest; i++) {
		if(redtab[i].rd_dest == host) {
			*hop1 = redtab[i].rd_to;
			return &nets[0];	/* DDP - Why nets[0]? */
			}
		}

	for(i=0; i<Nnet; i++) {
		/* Check if it is on my net */
		if((nets[i].ip_addr & nets[i].n_custom->c_net_mask) ==
			      (host & nets[i].n_custom->c_net_mask)) {
			*hop1 = host;
			return &nets[i];
			}

		if((host == nets[i].n_netbr) || /* Sending to net broadcast?*/
#ifdef notdef				/* should get taken care of above */
		   (host == nets[i].n_subnetbr) ||
#endif
		   (host == nets[i].n_netbr42) || /* To 4.2bsd broadcast? */
		   (host == 0xffffffff)) {	/* To physical broadcast? */
			*hop1 = host;
			return &nets[i];
			}
		}

	/* The host isn't on a net I'm on, so send it to the default gateway
		on my first net   --- this is a really bad idea.
	*/

	*hop1 = nets[0].n_defgw;

	/* if no gateway is set, then change the first hop address to
		the host we're trying to route to. this is just a kluge
		to make this work with arp routing. otherwise, we would
		try to return some sort of error indication.
	*/
	if(*hop1 == 0)
		return NULL;

	return &nets[0];
	}
