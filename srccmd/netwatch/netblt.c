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
typedef unsigned long u_long;
#include "netblt.h"

netblt_unprs(pnh, y, x)
	NB_HDR *pnh;
	int y, x; {
	char buffer[100];

	sprintf(buffer, "ver %d type %d len %d lp %u fp %u xsum %04x",
			pnh->hd_version, pnh->hd_type, bswap(pnh->hd_data_len),
			bswap(pnh->hd_lport), bswap(pnh->hd_fport),
			bswap(pnh->hd_cksum));
	wr_string(buffer, y, x, NORMAL);
	x += strlen(buffer);
	return x;
	}
