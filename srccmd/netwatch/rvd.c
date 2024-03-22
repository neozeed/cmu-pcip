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

#define	RVD_TYPE_OFFSET	0

extern struct layer rvd_layer;
extern struct nameber rvd_types[];

struct rvd {
	char	r_type;		/* request type */
	char	r_mode;		/* mode */
	char	r_pad;		/* padding */
	char	r_ver;		/* version number */
	long	r_drive;	/* drive number */	/* 38 */
	long	r_nonce;	/* packet nonce */
	long	r_index;	/* connection index */
	long	r_xsum;		/* packet checksum */
	long	r_rsvd;		/* reserved */
	long	r_sblock;	/* only for rd or write */
	long	r_nblocks;
	long	r_bindex;				/* 32+38 = 70 */
	};

rvd_unprs(prp, y, x)
	register struct rvd *prp;
	int x, y; {
	char buffer[40];
	struct nameber *type;

	type = lookup(rvd_types, (unsigned long)prp->r_type);

	/* lazy kluge */
#define	RVD_READ	3
#define	RVD_WRITE	4
#define	RVD_ERROR	18
	if(prp->r_type == RVD_ERROR)
		sprintf(buffer, "error %d", prp->r_mode);
	else {
	if(type) {
		if(disp_app)
			sprintf(buffer, " %s d%d n%d i%d", type->n_name,
				prp->r_drive, prp->r_nonce, prp->r_index);
		else
			sprintf(buffer, " %s", type->n_name);
		}
	else {
		rvd_layer.l_unknown++;
		sprintf(buffer, " %3u", prp->r_type);
		}
	}

	wr_string(buffer, y, x, NORMAL);
	return x+strlen(buffer);
	}

/* rvd filters - source, destination and type */

/* allow type to be specified as "ip udp nms" or "804" or "chaos"
*/

rvd_type(buf, offset)
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

	n = nlookup(rvd_types, buf);
	if(!n) {
		/* okay, if it didn't match then if it is a question mark,
			print out all the types we know
		*/
		if(strcmp(buf, question) == 0) {
			scroll_start();
			printf("8 bit decimal number\n\tor\n");
			print_namebers(rvd_types);
			scroll_end();
			return F_PAUSE;
			}

		if(!isdigit(*buf)) {
			clr25();
			pr25(0, "bad type");
			return F_ERROR;
			}
		sscanf(buf, "%d", &i);
		}
	else i = n->n_number;

	/* if we did have a table lookup, chain to the next level unless
		there's nothing else to do
	*/
	if(n && n->n_layer && n->n_layer->l_type && s && *(s+1))
		retcode = (*n->n_layer->l_type)(s+1, offset+sizeof(struct rvd));

	if(retcode != F_OK)
		return retcode;

	/* add the type field to both patterns */
	pt_addbytes(&pat1, &i, offset + RVD_TYPE_OFFSET, 1);
	pt_addbytes(&pat2, &i, offset + RVD_TYPE_OFFSET, 1);

	if(s)			/* DDP */
		*s = ' ';

	return F_OK;
	}
