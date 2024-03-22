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

#define	ICMP_TYPE_OFFSET	0

extern struct layer icmp_layer;
extern struct nameber icmp_prots[];

struct icmp {				/* ICMP header */
	char i_type;
	char i_code;
	unsigned i_chksum;
	unsigned i_id;
	unsigned i_seq;
	};

icmp_unprs(pip, y, x)
	register struct icmp *pip;
	int x, y; {
	char buffer[40];
	struct nameber *type;

	type = lookup(icmp_prots, (unsigned long)bswap(pip->i_type));

	if(type)
		sprintf(buffer, " %s", type->n_name);
	else {
		icmp_layer.l_unknown++;
		sprintf(buffer, " %6u", bswap(pip->i_type));
		}

	wr_string(buffer, y, x, NORMAL);
	return x+12;
	}

/* icmp filters - source, destination and type */

/* allow type to be specified as "ip udp nms" or "804" or "chaos"
*/

icmp_type(buf, offset)
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

	n = nlookup(icmp_prots, buf);
	if(!n) {
		/* okay, if it didn't match then if it is a question mark,
			print out all the types we know
		*/
		if(strcmp(buf, question) == 0) {
			scroll_start();
			printf("16 bit decimal number\n\tor\n");
			print_namebers(icmp_prots);
			scroll_end();
			return F_PAUSE;
			}

		if(!isdigit(*buf)) {
			clr25();
			pr25(0, "bad type");
			return F_ERROR;
			}
		sscanf(buf, "%x", &i);
		}
	else i = n->n_number;

	/* if we did have a table lookup, chain to the next level unless
		there's nothing else to do
	*/
	if(n && n->n_layer && n->n_layer->l_type && s && *(s+1))
		retcode = (*n->n_layer->l_type)(s+1,
				offset+sizeof(struct icmp));

	if(retcode != F_OK)
		return retcode;

	/* add the source port to pat1 and the destination port to pat2
	*/
	i = bswap(i);
	pt_addbytes(&pat1, &i, offset + ICMP_TYPE_OFFSET, 2);
	pt_addbytes(&pat2, &i, offset + ICMP_TYPE_OFFSET, 2);

	if(s)			/* DDP */
		*s = ' ';

	return F_OK;
	}
