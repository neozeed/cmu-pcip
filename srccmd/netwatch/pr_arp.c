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

/* pronet handling stuff for netwatch */

extern struct layer arp_layer;
extern struct nameber pronet_prots[];

struct arp {
	unsigned	ar_hd;		/* hardware type */
	unsigned	ar_pro;		/* protcol type */
	char		ar_hln;		/* hardware addr length */
	char		ar_pln;		/* protocol header length */
	unsigned	ar_op;		/* opcode */
	union	{
		struct {
		char		ar_sha;		/* sender hardware address */
		long		ar_spa;		/* sender protocol address */
		char		ar_tha;		/* target hardware address */
		long		ar_tpa;		/* target protocol address */
		} arp_ip;
		struct {
		char		ar_sha[6];	/* sender hardware address */
		unsigned	ar_spa;		/* sender protocol address */
		char		ar_tha[6];	/* target hardware address */
		unsigned	ar_tpa;		/* target protocol address */
		} arp_ch;
		}	ar_addrs;
	};

arp_unprs(pap, y, x)
	register struct arp *pap;
	int y;
	int x; {
	char buffer[100];
	struct nameber *type;

	if(pap->ar_op == 0x100) {
		wr_string("REQ", y, x, NORMAL);
		x += 3;
		}
	else if(pap->ar_op == 0x200) {
		wr_string("REP", y, x, NORMAL);
		x += 3;
		}
	else {
		sprintf(buffer, "???%04x", bswap(pap->ar_op));
		wr_string(buffer, y, x, NORMAL);
		x += 7;
		}

/*	type = lookup(pronet_prots, (unsigned long)bswap(pap->ar_pro));
	if(type) {
		sprintf(buffer, " prot: %s ", type->n_name);
		wr_string(buffer, y, x, NORMAL);
		x += strlen(buffer);
		}
	else {
		arp_layer.l_unknown++;
		sprintf(buffer, " unknown prot %04x", bswap(pap->ar_pro));
		wr_string(buffer, y, x, NORMAL);
		x += strlen(buffer);
		}
*/
	sprintf(buffer, " prot: %s ", "IP");	/* DDP Yes, this is totally bogus */
	wr_string(buffer, y, x, NORMAL);
	x += strlen(buffer);
	sprintf(buffer, "%a -> %a", pap->ar_addrs.arp_ip.ar_spa,
					pap->ar_addrs.arp_ip.ar_tpa);
	wr_string(buffer, y, x, NORMAL);
	x += strlen(buffer);
	return x;
	}
