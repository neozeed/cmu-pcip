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

/* This routine handles incoming UDP packets. They're handed to it by the
	internet layer. It demultiplexes the incoming packet based on the
	local port and upcalls the appropriate routine. */

/* 7/3/84 - fixed changed checksum computation to use length from UDP
	header instead of length passed up from internet.
						<John Romkey>
   7/12/84 - "fixed" the demultiplexor to not send destination
	unreachables in response to packets sent to the 4.2 ip
	broadcast address.			<John Romkey>
   12/19/85 - changed trace message to include length of incoming
	packet. 				<J. H. Saltzer>
   30/10/86 - handle ICMP destination unreachables.
						<Drew D. Perkins>
*/

extern UDPCONN firstudp;
extern NET nets[];			/* DDP */

udpdemux(p, len, host)
	PACKET p;
	int len;
	in_name host;	{
	struct ip *pip;
	register struct udp *pup;
	struct ph php;
	register UDPCONN con;
	unsigned osum, xsum;
	char *data;
	unsigned plen;

	/* First let's verify that it's a valid UDP packet. */
	pip = in_head(p);
	pup = udp_head(pip);
	plen = bswap(pup->ud_len);

	if(plen > len) {
#ifdef	DEBUG
		if(NDEBUG & PROTERR)
			printf("UDP: bad len pkt: rcvd: %u, hdr: %u.\n",
					len, bswap(pup->ud_len) + UDPLEN);
#endif

		in_free(p);
		return;
		}

	osum = pup->ud_cksum;
	if(osum) {
		if(len & 1) ((char *)pup)[plen] = 0;
		php.ph_src = host;
		php.ph_dest = pip->ip_dest;
		php.ph_zero = 0;
		php.ph_prot = UDPPROT;
		php.ph_len  = pup->ud_len;

		pup->ud_cksum = cksum(&php, sizeof(struct ph)>>1);
		xsum = ~cksum(pup, (plen+1)>>1);
		pup->ud_cksum = osum;
		if(xsum != osum) {
#ifdef	DEBUG
			if(NDEBUG & PROTERR)
			  {
			      printf(
			       "UDPDEMUX: bad xsum %04x right %04x from %a\n",
							osum, xsum, host);
			      if(NDEBUG & DUMP) in_dump(p);
			  }
#endif
			in_free(p);
			return;
			}
		}

	udpswap(pup);

#ifdef	DEBUG
	if(NDEBUG & TPTRACE)
		printf("UDP: pkt[%u] from %a:%d to %d\n",
		       plen, host, pup->ud_srcp, pup->ud_dstp);
#endif

	/* ok, accept it. run through the demux table and try to upcall it */

	for(con = firstudp; con; con = con->u_next) {
		if(con->u_lport && (con->u_lport != pup->ud_dstp))
			continue;

		if(con->u_rcv)
			(*con->u_rcv)(p, plen-UDPLEN, host, con->u_data);
		return;
		}

	/* what a crock. check if the packet was sent to an ip
		broadcast address. If it was, don't send a destination
		unreachable.
	*/

	if((pip->ip_dest == 0xffffffff) || /* Physical cable broadcast addr*/
	   (pip->ip_dest == nets[0].n_netbr) || /* All subnet broadcast */
	   (pip->ip_dest == nets[0].n_netbr42) || /* All subnet bcast (4.2bsd) */
	   (pip->ip_dest == nets[0].n_subnetbr)) { /* Our subnet broadcast */
#ifdef	DEBUG
		if(NDEBUG & INFOMSG)
			printf("UDP: ignoring ip broadcast\n");
#endif

		udp_free(p);
		return;
		}

#ifdef	DEBUG
	if(NDEBUG & (PROTERR))
	  {
		printf("UDP: unexpected port %04x\n", pup->ud_dstp);
		if(NDEBUG & DUMP) in_dump(p);
	  }
#endif

	/* send destination unreachable */
	icmp_destun(host, in_head(p), DSTPORT);

	udp_free(p);
	return;
	}


/* This routine handles incoming UDP destination unreachable packets.
	They're handed to it by the internet layer. It demultiplexes
	the incoming packet based on the local port and upcalls the
	appropriate routine. */

udpdudemux(pip, host)
	struct ip *pip;
	in_name host;
{
	register struct udp *pup;
	struct ph php;
	register UDPCONN con;
	char *data;
	unsigned plen;

	pup = udp_head(pip);
	udpswap(pup);

#ifdef	DEBUG
	if(NDEBUG & TPTRACE)
		printf("UDP: destination unreachable on %d to %a:%d\n",
		       pup->ud_srcp, host, pup->ud_dstp);
#endif

	/* ok, accept it. run through the demux table and try to upcall it */

	for(con = firstudp; con; con = con->u_next) {
		if(con->u_lport && (con->u_lport != pup->ud_srcp))
			continue;

		if(con->u_durcv)
			(*con->u_durcv)(pip, host, pup->ud_dstp, con->u_data);
		return;
	}
}
