/*  Copyright 1986 by Carnegie Mellon  */
/*  See permission and disclaimer notice in file "cmu-note.h"  */
#include	<cmu-note.h>
/*  Copyright 1985 by the Massachusetts Institute of Technology  */
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
#include <tcp.h>

/*  Symbolic Dump of a TCP packet.  Completely revised 6/1/85.
 *    				<J. H. Saltzer>
 *
 *  tcp_disp_hdr(p):  Display essentials of TCP packet header.
 *  tcp_dump(p):  Display remainder of header and start of data.
*/

tcp_disp_hdr(p)
PACKET	p;			/* Packet containing header */
{
	register struct tcp *itp; /* pointer to TCP header */

        itp = (struct tcp *)in_data(in_head(p));
	tcp_swab(itp);
	printf("Seq/Ack %U/%U win %u", itp->tc_seq, itp->tc_ack,
		itp->tc_win);
	if(itp->tc_fin)  printf(" FIN");
	if(itp->tc_syn)  printf(" SYN");
	if(itp->tc_rst)  printf(" RST");
	if(itp->tc_psh)  printf(" PSH");
	if(itp->tc_fack) printf(" ACK");
	if(itp->tc_furg) printf(" URG");
	printf("\n");
	tcp_swab(itp);
}

tcp_dump(p,len)
PACKET 	p;	 		/*  The packet to be dumped */
int	len; 			/*  length */
{
	register struct tcp *itp; /* pointer to TCP header */
	char 	*idp;		/* input pkt data ptr */
	int	idlen;          /* length of arriving data in bytes */
	int	i;		/* temporary */

	itp = (struct tcp *)in_data(in_head(p));
	idp = (char *)itp + (itp->tc_thl << 2);
	idlen = len - (itp->tc_thl << 2);
	tcp_swab(itp);
	printf("ports: %04x/%04x; window: %u, chksum: %4x; urgent ptr: %u\n",
	        itp->tc_srcp, itp->tc_dstp, itp->tc_win, itp->tc_cksum,
	        itp->tc_urg);
	printf("data[%u]:  ", idlen);
	for(i=0; i < min(20, idlen); i++) printf ("%02x ", *idp++);
	if (idlen > 20) printf(". . .");
 	printf("\n");
	tcp_swab(itp);
}


lprint(l)
	long l; {

	printf("%02x%02x", (unsigned)((l & 0xffff0000L) >> 16),
			   (unsigned)( l & 0x0000ffffL));

	return; }


unsigned tcpdrop = 0;

tcp_stats() {
	printf("TCP has dropped %u packets.\n", tcpdrop);
}

