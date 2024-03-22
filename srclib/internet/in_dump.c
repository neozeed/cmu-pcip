/*  Copyright 1986 by Carnegie Mellon  */
/*  See permission and disclaimer notice in file "cmu-note.h"  */
#include	<cmu-note.h>
/*  Copyright 1984 by the Massachusetts Institute of Technology  */
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
#include <ip.h>

/* This function dumps an internet packet to the screen. It uses some of the
	screen handling functions to help in this tedious chore. */

in_dump(p)
	PACKET p; {
#ifdef DEBUG
	int i, j;
	register struct ip *pip;
	register char *data;
	unsigned xsum, osum;

	pip = in_head(p);

	data = p->nb_buff;

	printf("IP Dump of Packet in buffer = %04x\tFrom = %a\tTo = %a\n",
		     p, pip->ip_src, pip->ip_dest);
	printf("\tHdr len = %u\tIP Len = %u\tTotal Len =%u\tID = %u\n",
	       pip->ip_ihl, bswap(pip->ip_len), p->nb_len, pip->ip_id);
	printf("\tVers = %u\tType of Ser = %u\tProt = %u\tTTL = %u\n",
		pip->ip_ver, pip->ip_tsrv, pip->ip_prot, pip->ip_time&0xff);
	printf("Frag Offset = %u\tFlags = %u\tXsum = %04x\n",
		pip->ip_foff,pip->ip_flgs,pip->ip_chksum);

/*	osum = pip->ip_chksum;
	pip->ip_chksum = 0;
	xsum = ~cksum(pip, pip->ip_ihl << 1);
	printf("Computed xsum = %04x\t", xsum);
	pip->ip_chksum = osum;
	if(xsum == osum) prints("Xsum is CORRECT.\n\r");
	else prints("Xsum is NOT CORRECT\n\r");
*/
	printf("Contents of first 120 bytes:\n");
	for(i=0; i < 6; i++) {
		printf("\t");
		for(j=0; j<20; j++) printf("%02x ",(char)*data++ & 0xff);
		printf("\n"); }
#endif
	return;
	}
