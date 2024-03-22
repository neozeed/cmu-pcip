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
#include "../../srclib/internet/ipconn.h"

/* internet statistics */
unsigned ipdrop = 0;		/* ip packets dropped */
unsigned ipxsum = 0;		/* ip packets with bad checksums */
unsigned iplen = 0;		/* ip packets with bad lengths */
unsigned ipdest = 0;		/* ip packets with bad destinations */
unsigned ipttl = 0;		/* ip packets with time to live = 0 */
unsigned ipprot = 0;		/* no server for protocol */
unsigned ipver = 0;		/* bad ip version number */
unsigned iprcv = 0;		/* number of ip packets received */
unsigned ipfrag = 0;		/* number of fragments received */
unsigned ipbadadr = 0;		/* number of IP packets not for me */
unsigned ipdurcv = 0;		/* number of ip dest. unreachables received */

extern int Nnet;
extern NET nets[];
#ifdef	PCSTAT
int pcacti = 0;
char pcact[256];
extern NET *pc_net;
#endif

/* This is the internet demultiplexing routine. It handles packets received by
	the per-net task, verifies their headers and does the upcall to
	the whoever should receive the packet. All the guts of demultiplexing
	is in this piece of code. If the packet doesn't belong to anyone,
	this gets logged and the packet dropped.	*/

indemux(p, len, nt)
PACKET p;
int len;
NET *nt;
{
	register struct ip *pip;	/* the internet header */
	register IPCONN conn;		/* an internet connection */
	unsigned prot;			/* packet protocol */
	int i;
	unsigned csum;			/* packet checksum */
	char *pdata;
	in_name firsthop;
	NET *nnt;

	iprcv++;

	pip = in_head(p);

	if(pip->ip_ihl < IP_IHL) {
		printf("short ip_ihl %d ", pip->ip_ihl);
		printf("%a -> %a\n", pip->ip_src, pip->ip_dest);
		in_free(p);
		return;
	}
	if(pip->ip_ihl > IP_IHL * 2) {
		printf("questionable ip_ihl %d len %d", pip->ip_ihl, len);
		printf("%a -> %a\n", pip->ip_src, pip->ip_dest);
	}
	if(pip->ip_ihl << 2 > len) {
		printf("ip_ihl %d len %d ", pip->ip_ihl, len);
		printf("%a -> %a\n", pip->ip_src, pip->ip_dest);
		in_free(p);
		return;
	}

	if(len < bswap(pip->ip_len)) {
#ifdef	DEBUG
		if(NDEBUG & PROTERR) {
			printf("indemux: bad pkt len\n");
			if(NDEBUG & DUMP) in_dump(p);
		}
#endif
		iplen++;
		ipdrop++;
		in_free(p);
		return;
	}

	len = bswap(pip->ip_len);

	if(pip->ip_ver != IP_VER) {
#ifdef	DEBUG
		if(NDEBUG & PROTERR) {
			printf("indemux: bad version number\n");
			if(NDEBUG & DUMP) in_dump(p);
		}
#endif
		ipver++;
		ipdrop++;
		in_free(p);
		return;
	}

	csum = pip->ip_chksum;
	pip->ip_chksum = 0;
	if(csum != ~cksum(pip, pip->ip_ihl << 1)) {
		pip->ip_chksum = csum;
#ifdef	DEBUG				/* DDP - DEBUG or no identifier? */
		if(NDEBUG & PROTERR) {
			printf("indemux: bad xsum\n");
			if(NDEBUG & DUMP) in_dump(p);
		}
#endif
		ipxsum++;
		ipdrop++;
		in_free(p);
		return;
	}

	pip->ip_chksum = csum;
#ifdef	PCSTAT
	if(nt == pc_net)
		pcact[((unsigned char *)&pip->ip_src)[3]] = 1;
#endif

	for(i = 0; i < Nnet; i++)
		if(pip->ip_dest == nets[i].ip_addr)
			goto forus;
	if((pip->ip_dest != 0xffffffff) && /* Physical cable broadcast addr*/
	   (pip->ip_dest != nt->n_netbr) && /* All subnet broadcast */
	   (pip->ip_dest != nt->n_netbr42) && /* All subnet bcast (4.2bsd) */
	   (pip->ip_dest != nt->n_subnetbr) && /* Our subnet broadcast */
	   (nt->ip_addr & ~nt->n_custom->c_net_mask)) { /* Know our own host address? */
		if(nnt = inroute(pip->ip_dest, &firsthop)) {
			if(nnt == nt) {
				extern char sbuf[];
				printf("should redirect %a -> %a\n",
					pip->ip_src, pip->ip_dest);
				sprintf(sbuf, "should redirect %a -> %a\n",
					pip->ip_src, pip->ip_dest);
				syslog(sbuf);
			}
#ifdef	PCSTAT
			if(nnt == pc_net)
				pcact[((unsigned char *)&pip->ip_dest)[3]] = 1;
#endif
			(*nnt->n_send)(p, IP, len, firsthop);
			in_free(p);
			return;
		}
#ifdef	DEBUG
		if(NDEBUG & INFOMSG)
			printf("indemux: got pkt not for me (%a)\n",
				pip->ip_dest);
#endif
		ipbadadr++;
		ipdrop++;
		in_free(p);
		return;
	}

forus:
	/* Woe, oh silly crock */
	*(((unsigned *)&pip->ip_id)+1)=bswap(*(((unsigned *)&pip->ip_id)+1));

	if((pip->ip_foff != 0) || (pip->ip_flgs & 1)) {
#ifdef	DEBUG
		if(NDEBUG & PROTERR) {
			printf("indemux: fragment from %a\n", pip->ip_src);
			if(NDEBUG & DUMP) in_dump(p);
		}
#endif
		ipfrag++;
		ipdrop++;
		in_free(p);
		return;
	}

	/* The packet is now verified; the header is correct. Now we have
		to demultiplex it among our internet connections. */

	prot = pip->ip_prot&0xff;	/* defeat sign extension */
#ifdef	DEBUG
	if(NDEBUG & IPTRACE) {
		printf("ipdemux: got pkt[%u] prot %u from %a\n",
			len-IPLEN, prot, pip->ip_src);
		if(NDEBUG & DUMP) in_dump(p);
	}
#endif

	for(i=0; i<nipconns; i++) {
		conn = ipconn[i];
		if(conn->c_prot == prot)
			if(conn->c_handle == 0)
				break;
			else {
/* DDP				conn->c_handle(p, len-IPLEN, pip->ip_src); */
				(*conn->c_handle)(p, len-IPLEN, pip->ip_src);
				return;
			}
	}

	/* nobody's listening for this packet. Unless it was broadcast, send
		a destination unreachable.
	*/
	if((pip->ip_dest != 0xffffffff) && /* Physical cable broadcast addr*/
	   (pip->ip_dest != nt->n_netbr) && /* All subnet broadcast */
	   (pip->ip_dest != nt->n_netbr42) && /* All subnet bcast (4.2bsd) */
	   (pip->ip_dest != nt->n_subnetbr) && /* Our subnet broadcast */
	   (nt->ip_addr ^ nt->n_subnetbr)) { /* Know our own host address? */

#ifdef	DEBUG
		if(NDEBUG & PROTERR) {
			printf("ipdemux: unhandled prot %u\n", prot);
			if(NDEBUG & DUMP) in_dump(p);
		}
#endif

		icmp_destun(pip->ip_src, pip, DSTPROT);
	}

	ipprot++;
	ipdrop++;
	in_free(p);
	return;
}


/* This is the internet destination unreachable demultiplexing routine.
	It handles destination unreachable packets received by
	the icmp protocol and does the upcall to the whoever should
	receive notification. All the guts of demultiplexing
	is in this piece of code. If the packet doesn't belong to anyone,
	this gets logged and the packet dropped.	*/

indudemux(pip)
struct ip *pip;				/* the internet header */
{
	register IPCONN conn;		/* an internet connection */
	unsigned prot;			/* packet protocol */
	int i;
	unsigned csum;			/* packet checksum */
	char *pdata;

	ipdurcv++;

	prot = pip->ip_prot&0xff;	/* defeat sign extension */

	for(i=0; i<nipconns; i++) {
		conn = ipconn[i];
		if(conn->c_prot == prot)
			if(conn->du_handle == 0)
				break;
			else {
				(*conn->du_handle)(pip, pip->ip_dest);
				return;
			}
	}

	ipprot++;
	ipdrop++;
	return;
}


/* pretty print the statistics */
extern unsigned ipsnd;		/* # packets sent */

in_stats(fd)
FILE *fd;
{
#ifndef NOSTATS
	fprintf(fd, "IP Stats ");
	fprintf(fd, "%4u pkts rcvd\t%4u pkts sent\n", iprcv, ipsnd);
	fprintf(fd, "%4u pkts dropped because of:\t", ipdrop);
	fprintf(fd, "%4u bad xsums\t%4u bad protocols\n", ipxsum, ipprot);
	fprintf(fd, "\t%4u bad vers\t%4u bad lens\t", ipver, iplen);
	fprintf(fd, "%4u not for me\n", ipbadadr);
	fprintf(fd, "\t%4u ttl expired\t%4u frags\n", ipttl, ipfrag);
	fprintf(fd, "%4u destination unreachables rcvd\n", ipdurcv);
#endif
}
