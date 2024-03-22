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

#define	IP_PROTOCOL_OFFSET	9
#define	IP_SRC_OFFSET		12
#define	IP_DST_OFFSET		16

extern struct layer ip_layer;
extern struct nameber ip_prots[];

ip_unprs(pip, y, x)
	register struct ip *pip;
	int y, x; {
	char buffer[100];
	struct nameber *n;

	sprintf(buffer, "%a->%a", pip->ip_src, pip->ip_dest);
	wr_string(buffer, y, x, NORMAL);

	x += strlen(buffer);

	n = lookup(ip_prots, (unsigned long)(pip->ip_prot&0xff));
	if(n)
		sprintf(buffer, "%5d %6s ", bswap(pip->ip_len), n->n_name);
	else  {
		ip_layer.l_unknown++;
		sprintf(buffer, "%5d %6u ", bswap(pip->ip_len), pip->ip_prot);
		}

	wr_string(buffer, y, x, NORMAL);
	x += strlen(buffer);

	if(disp_internet) {
		sprintf(buffer, "TTL:%d ", pip->ip_time&0xff);
		wr_string(buffer, y, x, NORMAL);
		x += strlen(buffer);
		}	

	/* check if this is a fragment */
	*(((unsigned *)&pip->ip_id)+1)=bswap(*(((unsigned *)&pip->ip_id)+1));
	if((pip->ip_foff != 0) || (pip->ip_flgs & 1)) {
		wr_string("Fragment", y, x, NORMAL);
		return x+8;	/* got a fragment - depends on 8 chars */
		}

	if(n && n->n_layer && n->n_layer->l_parse)
		 (*n->n_layer->l_parse)(pip+1, y, x);

	return x;
	}

/* ip filters - type and address */

/* allow type to be specified as "ip udp nms" or "804" or "chaos"
*/

ip_type(buf, offset)
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

	n = nlookup(ip_prots, buf);
	if(!n) {
		/* okay, if it didn't match then if it is a question mark,
			print out all the types we know
		*/
		if(strcmp(buf, question) == 0) {
			scroll_start();
			printf("8 bit decimal number\n\tor\n");
			print_namebers(ip_prots);
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
		retcode = (*n->n_layer->l_type)(s+1, offset+sizeof(struct ip));

	if(retcode != F_OK)
		return retcode;

	/* add the type field to both patterns */
	pt_addbytes(&pat1, &i, offset+IP_PROTOCOL_OFFSET, 1);
	pt_addbytes(&pat2, &i, offset+IP_PROTOCOL_OFFSET, 1);

	if(s)			/* DDP */
		*s = ' ';

	return F_OK;
	}

in_name resolve_name();

ip_addr(buf, offset, how)
	register char *buf;	/* the type specification */
	int offset;		/* offset into packet for ether header */
	char how;
{
	struct nameber *n;
	char *s;
	int i;
	int retcode = F_OK;
	in_name addr;

	if(strcmp(buf, question) == 0) {
		scroll_start();
		printf("32 bit internet address\n");
		scroll_end();
		return F_PAUSE;
		}

	addr = resolve_name(buf);

	switch(how) {
	case 'S':
		pt_addbytes(&pat1, &addr, offset+IP_SRC_OFFSET, 4);
		pt_addbytes(&pat2, &addr, offset+IP_SRC_OFFSET, 4);
		break;

	case 'D':
		pt_addbytes(&pat1, &addr, offset+IP_DST_OFFSET, 4);
		pt_addbytes(&pat2, &addr, offset+IP_DST_OFFSET, 4);
		break;

	case 'W':
		pt_addbytes(&pat1, &addr, offset+IP_SRC_OFFSET, 4);
		pt_addbytes(&pat2, &addr, offset+IP_DST_OFFSET, 4);
		break;
		}

	return F_OK;
	}

