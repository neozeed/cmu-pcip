/*  Copyright 1986 by Carnegie Mellon  */
/*  See permission and disclaimer notice in file "cmu-note.h"  */
#include	<cmu-note.h>
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

#define	UDP_SRC_SOCKET_OFFSET	0
#define	UDP_DST_SOCKET_OFFSET	2

extern struct layer udp_layer;
extern struct nameber udp_prots[];


struct udp {
	unsigned ud_srcp;	/* source port */
	unsigned ud_dstp;	/* dest port */
	unsigned ud_len;		/* length of UDP packet */
	unsigned ud_cksum;	/* UDP checksum */
	};

udp_unprs(pup, y, x)
	register struct udp *pup;
	int x, y; {
	char buffer[40];
	struct nameber *src, *dst;

	src = lookup(udp_prots, (unsigned long)bswap(pup->ud_srcp));
	dst = lookup(udp_prots, (unsigned long)bswap(pup->ud_dstp));

	if(src && dst)
		sprintf(buffer, " %6s -> %6s  %4u", src->n_name, dst->n_name,
							bswap(pup->ud_len));
	else if(src && !dst) {
		udp_layer.l_unknown++;
		sprintf(buffer, " %6s->%6u  %4u", src->n_name,
				bswap(pup->ud_dstp), bswap(pup->ud_len));
		}
	else if(!src && dst) {
		udp_layer.l_unknown++;
		sprintf(buffer, " %6u->%6s  %4u", bswap(pup->ud_srcp),
						dst->n_name, bswap(pup->ud_len));
		}
	else {
		udp_layer.l_unknown++;
		udp_layer.l_unknown++;
		sprintf(buffer, " %6u->%6u  %4u", bswap(pup->ud_srcp),
				bswap(pup->ud_dstp), bswap(pup->ud_len));
		}

	wr_string(buffer, y, x, NORMAL);
	return x+22;
	}

/* udp filters - source, destination and type */

/* allow type to be specified as "ip udp nms" or "804" or "chaos"
*/

udp_type(buf, offset)
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

	n = nlookup(udp_prots, buf);
	if(!n) {
		/* okay, if it didn't match then if it is a question mark,
			print out all the types we know
		*/
		if(strcmp(buf, question) == 0) {
			scroll_start();
			printf("16 bit decimal number\n\tor\n");
			print_namebers(udp_prots);
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
		retcode = (*n->n_layer->l_type)(s+1, offset+sizeof(struct udp));

	if(retcode != F_OK)
		return retcode;

	/* add the source port to pat1 and the destination port to pat2
	*/
	i = bswap(i);
	pt_addbytes(&pat1, &i, offset+UDP_SRC_SOCKET_OFFSET, 2);
	pt_addbytes(&pat2, &i, offset+UDP_DST_SOCKET_OFFSET, 2);

	if(s)			/* DDP */
		*s = ' ';

	return F_OK;
	}

