/*  Copyright 1986 by Carnegie Mellon  */
/*  See permission and disclaimer notice in file "cmu-note.h"  */
#include	<cmu-note.h>
/*  Copyright 1984, 1985 by the Massachusetts Institute of Technology  */
/*  See permission and disclaimer notice in file "notice.h"  */
#include	<notice.h>

/* 8/23/84 - changed debugging messages slightly.
						<John Romkey>
*/

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

/* Fill in the udp header on a packet, checksum it and pass it to
	Internet. */

udp_send_to(host, fport, lport, p, len)
	in_name host;
	unsigned fport;
	unsigned lport;
	PACKET p;
	int len; {
	register struct udp *pup;
	/* DDP - register */ struct ph php;
	int udplen;

#ifdef	DEBUG
	if(NDEBUG & (INFOMSG|TPTRACE))
	    printf("UDP: pkt [%u] %04x -> %a:%04x\n", len, lport,
						host, fport);
#endif

	pup = udp_head(in_head(p));
	udplen = len + sizeof(struct udp);
	if(udplen & 1) ((char *)pup)[udplen] = 0;

	pup->ud_len = udplen;
	pup->ud_srcp = lport;
	pup->ud_dstp = fport;
	udpswap(pup);

	php.ph_src = in_mymach(host);
	php.ph_dest = host;
	php.ph_zero = 0;
	php.ph_prot = UDPPROT;
	php.ph_len = pup->ud_len;
	pup->ud_cksum = cksum(&php, sizeof(struct ph)>>1);
	pup->ud_cksum = ~cksum(pup, (udplen+1)>>1);

	return in_write(udp, p, udplen, host);
	}
