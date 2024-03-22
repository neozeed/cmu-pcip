/*  Copyright 1986 by Carnegie Mellon  */
/*  See permission and disclaimer notice in file "cmu-note.h"  */
#include	<cmu-note.h>
/*
22-Mar-86 Drew D. Perkins (ddp) at Carnegie-Mellon University,
	Made bit-fields be unsigned to please MSC V3.0.
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
#include <attrib.h>
#include <ctype.h>
#include <match.h>
#include "watch.h"

/* ethernet handling stuff for netwatch */

#define	TCP_SRC_SOCKET_OFFSET	0
#define	TCP_DST_SOCKET_OFFSET	2

extern struct layer tcp_layer;
extern struct nameber tcp_prots[];


struct	tcp	{			/* a tcp header */
	unsigned tc_srcp;		/* source port */
	unsigned tc_dstp;		/* dest port */
	long	tc_seq;			/* sequence number */
	long	tc_ack;			/* acknowledgement number */
	unsigned int tc_uu1 : 4;	/* unused */
	unsigned int tc_thl : 4;	/* tcp header length */
	unsigned int tc_fin : 1;	/* fin bit */
	unsigned int tc_syn : 1;	/* syn bit */
	unsigned int tc_rst : 1;	/* reset bit */
	unsigned int tc_psh : 1;	/* push bit */
	unsigned int tc_fack : 1;	/* ack valid */
	unsigned int tc_furg : 1;	/* urgent ptr. valid */
	unsigned int tc_uu2 : 2;	/* unused */
	unsigned	tc_win;			/* window */
	unsigned	tc_cksum;		/* checksum */
	unsigned	tc_urg;			/* urgent pointer */
	};

tcp_unprs(ptp, y, x)
	register struct tcp *ptp;
	int x, y; {
	char buffer[40];
	struct nameber *src, *dst;

	src = lookup(tcp_prots, (unsigned long)bswap(ptp->tc_srcp));
	dst = lookup(tcp_prots, (unsigned long)bswap(ptp->tc_dstp));

	if(src && dst)
		sprintf(buffer, " %s->%s ", src->n_name, dst->n_name);
	else if(src && !dst) {
		tcp_layer.l_unknown++;
		sprintf(buffer, " %s->%u ", src->n_name,
						bswap(ptp->tc_dstp));
		}
	else if(!src && dst) {
		tcp_layer.l_unknown++;
		sprintf(buffer, " %u->%s ", bswap(ptp->tc_srcp),
								dst->n_name);
		}
	else {
		tcp_layer.l_unknown++;
		tcp_layer.l_unknown++;
		sprintf(buffer, " %u->%u ", bswap(ptp->tc_srcp),
							bswap(ptp->tc_dstp));
		}

	wr_string(buffer, y, x, NORMAL);
	x += strlen(buffer);

	/* if not displaying transport protocol info, then skip
		the rest
	*/
	if(!disp_transport)
		return x;

	sprintf(buffer, "a:%08X s:%08X w:%u", lswap(ptp->tc_ack),
		lswap(ptp->tc_seq), bswap(ptp->tc_win));
	wr_string(buffer, y, x, NORMAL);
	x += strlen(buffer);
	return x;
	}

/* tcp filters - source, destination and type */

/* allow type to be specified as "ip udp nms" or "804" or "chaos"
*/

tcp_type(buf, offset)
	register char *buf;	/* the type specification */
	int offset;	{	/* offset into packet for ether header */
	struct nameber *n;
	char *s, *strchr();	/* DDP */
	int i;
	int retcode = F_OK;

	/* uggh this is ugly - will have to do this in every filter...
		so much for distributed parsing...
	*/

	/* look for a space */
#ifndef MSC			/* DDP */
	s = index(buf, ' ');
#else				/* DDP */
	s = strchr(buf, ' ');	/* DDP */
#endif				/* DDP */

	if(s)
		*s = '\0';

	n = nlookup(tcp_prots, buf);
	if(!n) {
		/* okay, if it didn't match then if it is a question mark,
			print out all the types we know
		*/
		if(strcmp(buf, question) == 0) {
			scroll_start();
			printf("16 bit decimal number\n\tor\n");
			print_namebers(tcp_prots);
			scroll_end();
			return F_PAUSE;
			}

		if(!isdigit(*buf)) {
			clr25();
			pr25(0, "bad packet type");
			return F_ERROR;
			}
		sscanf(buf, "%d", &i);
		}
	else i = n->n_number;

	/* if we did have a table lookup, chain to the next level unless
		there's nothing else to do
	*/
	if(n && n->n_layer && n->n_layer->l_type && s && *(s+1))
		retcode = (*n->n_layer->l_type)(s+1, offset+sizeof(struct tcp));

	if(retcode != F_OK)
		return retcode;

	/* add the source port to pat1 and the destination port to pat2
	*/
	i = bswap(i);
	pt_addbytes(&pat1, &i, offset+TCP_SRC_SOCKET_OFFSET, 2);
	pt_addbytes(&pat2, &i, offset+TCP_DST_SOCKET_OFFSET, 2);

	if(s)			/* DDP */
		*s = ' ';

	return F_OK;
	}

