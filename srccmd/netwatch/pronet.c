/*  Copyright 1986 by Carnegie Mellon  */
/*  See permission and disclaimer notice in file "cmu-note.h"  */
#include	<cmu-note.h>
#include <stdio.h>
#include <q.h>
#include <attrib.h>
#include <ctype.h>
#include <colors.h>
#include <match.h>
#include "watch.h"

/* display header	*/
static char *hex_header =
"src  dst   type  len  0    1    2    3    4    5    6    7     ";
/*		 ^	       ^	*/

static char *sym_header =
"src  dst   type  len";

char *header;

/* pronet handling stuff for netwatch */

extern struct nameber pronet_prots[];
extern struct layer pronet_layer;

struct pr_hdr {
	char dst;
	char src;
	unsigned type;
	char	pad[2];
	};

unsigned long nbroadcasts = 0;
char manu_switch = FALSE;
extern char local_net_disp;
extern char write_attrib;
extern int color_display;

char *unparse_pr(), *strchr();		/* DDP */
long lswap();

norm_heading() {
	header = hex_header;
	}

sym_heading() {
	header = sym_header;
	}

pr_unprs(data, y, x, len)
	register char *data;
	int y,x;
	unsigned len; {
	char buffer[200];
	register struct pr_hdr *pr = (struct pr_hdr *)data;
	char *addr;

	/* quick check if broadcast
	*/
	if((pr->dst&0xff) == 0xff)
		nbroadcasts++;

	switch(prot_mode) {
	case MD_SYMBOLIC: {
		struct nameber *n;

		n = lookup(pronet_prots, (long)bswap(pr->type));
		if(n) {
			if(unknown_match)
				return FALSE;

			if(color_display)
				write_attrib = n->n_color;

			if(local_net_disp) {
				sprintf(buffer, "(%s ->", unparse_pr(pr->src));
				wr_string(buffer, y, x, NORMAL);
				x = strlen(buffer);
				sprintf(buffer, " %s) ", unparse_pr(pr->dst));
				wr_string(buffer, y, x, NORMAL);
				x += strlen(buffer);
				}

			sprintf(buffer, "%s: ", n->n_name);
			wr_string(buffer, y, x, NORMAL);
			x += strlen(buffer);

			if(n->n_layer && n->n_layer->l_parse) {
				(*n->n_layer->l_parse)(data+sizeof(struct pr_hdr), y, x);
				break;
				}
			}
		else pronet_layer.l_unknown++;

		}

	case MD_NORMAL:

	if(color_display)
		write_attrib = BLACK<<4|WHITE;

	addr = unparse_pr(pr->src);
	wr_string(addr, y, x, NORMAL);

	addr = unparse_pr(pr->dst);
	wr_string(addr, y, x+4, NORMAL);

	sprintf(buffer, "  %08X %4u ", lswap(pr->type), len);
	wr_string(buffer, y, x+8, NORMAL);

	data += sizeof(struct pr_hdr);

	sprintf(buffer, "  %02x  %02x  %02x  %02x  %02x  %02x  %02x  %02x", 
		mkbyte(data[0]), mkbyte(data[1]), mkbyte(data[2]),
		mkbyte(data[3]), mkbyte(data[4]), mkbyte(data[5]),
				 mkbyte(data[6]), mkbyte(data[7]));
	wr_string(buffer, y, x+28, NORMAL);
	break;
	}

	return TRUE;
	}

/* pronet filters - source, destination and type */

/* allow type to be specified as "ip udp nms" or "804" or "chaos"
*/

pr_type(buf, offset)
	register char *buf;	/* the type specification */
	int offset;	{	/* offset into packet for ether header */
	struct nameber *n;
	char *s;
	unsigned type;
	int retcode = F_OK;
	int taboffset = 0;

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

	n = nlookup(pronet_prots, buf);
	if(!n) {
		/* okay, if it didn't match then if it is a question mark,
			print out all the types we know
		*/
		if(strcmp(buf, question) == 0) {
			scroll_start();
			printf("16 bit hexadecimal number\n\tor\n");
			print_namebers(pronet_prots);
			scroll_end();
			return F_PAUSE;
			}

		if(!isdigit(*buf)) {
			clr25();
			pr25(0, "bad packet type");
			return F_ERROR;
			}

		sscanf(buf, "%x", &type);
		}
	else type = n->n_number;

	/* if we did have a table lookup, chain to the next level unless
		there's nothing else to do

	   call before adding the bytes to the patterns so that if the
	   user types "ip tcp ?" we won't end up matching all ip/tcp
	   packets.
	*/
	if(n && n->n_layer && n->n_layer->l_type && s && *(s+1))
		retcode = (*n->n_layer->l_type)(s+1, offset+sizeof(struct pr_hdr));

	if(retcode != F_OK)
		return retcode;

	/* add the type field to both patterns */
	type = bswap(type);
	pt_addbytes(&pat1, &type, offset+2, 2);
	pt_addbytes(&pat2, &type, offset+2, 2);

	if(s)				/* DDP */
		*s = ' ';

	return F_OK;
	}

/* allows address to be specified as "8" or "ip 18.10.0.8"
*/

pr_addr(buf, offset, how)
	register char *buf;	/* the type specification */
	int offset;	{	/* offset into packet for ether header */
	struct nameber *n;
	char *s;
	char addr;
	int retcode = F_OK;
	unsigned long type;

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

	n = nlookup(pronet_prots, buf);
	if(!n) {
		/* okay, if it didn't match then if it is a question mark,
			print out all the types we know
		*/
		if(strcmp(buf, question) == 0) {
			scroll_start();
			printf("8 bit decimal number\n\tor\n");
			print_addr_namebers(pronet_prots);
			scroll_end();
			return F_PAUSE;
			}

		if(!isdigit(*buf)) {
			clr25();
			pr25(0, "bad packet type");
			return F_ERROR;
			}

		addr = atoi(buf);
		switch(how) {
		case 'S':
			pt_addbytes(&pat1, &addr, offset+1, 1);
			pt_addbytes(&pat2, &addr, offset+1, 1);
			break;
		case 'D':
			pt_addbytes(&pat1, &addr, offset+0, 1);
			pt_addbytes(&pat2, &addr, offset+0, 1);

		case 'W':
			pt_addbytes(&pat1, &addr, offset+0, 1);
			pt_addbytes(&pat2, &addr, offset+1, 1);
			}
		return F_OK;
		}

	/* if we did have a table lookup, chain to the next level unless
		there's nothing else to do

	   call before adding the bytes to the patterns so that if the
	   user types "ip tcp ?" we won't end up matching all ip/tcp
	   packets.
	*/
	if(n && n->n_layer && n->n_layer->l_type && s && *(s+1))
		retcode = (*n->n_layer->l_type)(s+1, offset+sizeof(struct pr_hdr), how);

	if(retcode != F_OK)
		return retcode;

	/* since we're matching on a protocol address, we can
		only accept packets of that protocol
	 */
	type = lswap(n->n_number);
	pt_addbytes(&pat1, &type, offset+2, 4);
	pt_addbytes(&pat2, &type, offset+2, 4);

	return F_OK;
	}

char *unparse_pr(a)
	char a; {
	static char buffer[10];

	if((a&0xff) == 0xff)
		strcpy(buffer, "*");
	else
		sprintf(buffer, "%d", a&0xff);

	return buffer;
	}


/* stub routine for internet layer routines that get called
	by the poor unknowing pronet driver
*/
indemux() {
}

in_stats() {
}

in_dump() {
}
