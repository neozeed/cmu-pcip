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
#include <icmp.h>
#include <ip.h>
#include <udp.h>

udpswap(pup)
	struct udp *pup; {

	pup->ud_srcp = bswap(pup->ud_srcp);
	pup->ud_dstp = bswap(pup->ud_dstp);
	pup->ud_len = bswap(pup->ud_len);
	pup->ud_cksum = bswap(pup->ud_cksum);
	}
