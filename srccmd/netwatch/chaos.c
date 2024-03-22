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

#define	CHAOS_OPCODE_OFFSET	1
#define	CHAOS_SRC_OFFSET	8
#define	CHAOS_DST_OFFSET	4

extern struct layer chaos_layer;
extern struct nameber chaos_opcodes[];


struct chaos {
	char	c_rsvd;			/* 0 */
	char	c_opcode;		/* 1 */
	unsigned int c_flen:12;		/* 2 */
	unsigned int c_fcount:4;
	unsigned	c_dest;		/* 4 */
	unsigned	c_dindx;	/* 6 */
	unsigned	c_src;		/* 8 */
	unsigned	c_sindx;	/* 10 */
	unsigned	c_pnumber;	/* 12 */
	unsigned	c_ack;		/* 14 */
	};

chaos_unprs(pcp, y, x)
	register struct chaos *pcp;
	int y;
	int x; {
	char buffer[100];
	struct nameber *type;

	sprintf(buffer, "%06o.%06o -> %06o.%06o ", pcp->c_src, pcp->c_sindx,
						pcp->c_dest, pcp->c_dindx);
	wr_string(buffer, y, x, NORMAL);
	x += 31;

	type = lookup(chaos_opcodes, (unsigned long)pcp->c_opcode);
	if(type)
		sprintf(buffer, "%s  ", type->n_name);
	else {
		chaos_layer.l_unknown++;
		sprintf(buffer, "0%o", pcp->c_opcode&0377);
		}

	wr_string(buffer, y, x, NORMAL);
	x += strlen(buffer);
	if(type && type->n_layer && type->n_layer->l_parse)
		return (*type->n_layer->l_parse)(pcp, y, x);
	return x;
	}

/* also throw in rfc's */

rfc_unprs(pcp, y, x)
	register struct chaos *pcp;
	int y, x; {

	((char *)(pcp+1))[pcp->c_flen] = 0;
	wr_string(pcp+1, y, x, NORMAL);
	return x+strlen(pcp+1);
	}

/* chaos filters - source, destination and type */

/* allow type to be specified as "ip udp nms" or "804" or "chaos"
*/

chaos_type(buf, offset)
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

	n = nlookup(chaos_opcodes, buf);
	if(!n) {
		/* okay, if it didn't match then if it is a question mark,
			print out all the types we know
		*/
		if(strcmp(buf, question) == 0) {
			scroll_start();
			printf("8 bit octal number\n\tor\n");
			print_namebers(chaos_opcodes);
			scroll_end();
			return F_PAUSE;
			}

		if(!isdigit(*buf)) {
			clr25();
			pr25(0, "bad packet type");
			return F_ERROR;
			}
		sscanf(buf, "%o", &i);
		}
	else i = n->n_number;

	/* if we did have a table lookup, chain to the next level unless
		there's nothing else to do
	*/
	if(n && n->n_layer && n->n_layer->l_type && s && *(s+1))
		retcode = (*n->n_layer->l_type)(s+1, offset+14);

	if(retcode != F_OK)
		return retcode;

	/* add the type field to both patterns */
	pt_addbytes(&pat1, &i, offset+CHAOS_OPCODE_OFFSET, 1);
	pt_addbytes(&pat2, &i, offset+CHAOS_OPCODE_OFFSET, 1);

	if(s)			/* DDP */
		*s = ' ';

	return F_OK;
	}

chaos_addr(buf, offset, how)
	register char *buf;
	int offset;
	char how; {
	unsigned addr;
	register char *s;

	/* don't bother to lookup our children */
	if(strcmp(buf, question) == 0) {
		scroll_start();
		printf("16 bit octal number\n");
		scroll_end();
		return F_PAUSE;
		}

	if(!isdigit(*buf)) {
		clr25();
		pr25(0, "bad address");
		}

	sscanf(buf, "%o", &addr);
	switch(how) {
	case 'S':
		pt_addbytes(&pat1, &addr, offset+CHAOS_SRC_OFFSET, 2);
		pt_addbytes(&pat2, &addr, offset+CHAOS_SRC_OFFSET, 2);
		break;
	case 'D':
		pt_addbytes(&pat1, &addr, offset+CHAOS_DST_OFFSET, 2);
		pt_addbytes(&pat2, &addr, offset+CHAOS_DST_OFFSET, 2);
		break;
	case 'W':
		pt_addbytes(&pat1, &addr, offset+CHAOS_SRC_OFFSET, 2);
		pt_addbytes(&pat2, &addr, offset+CHAOS_DST_OFFSET, 2);
		break;
		}

	return F_OK;
	}
