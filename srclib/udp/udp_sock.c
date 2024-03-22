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

static unsigned socket = 0;
extern long cticks;

/* Select a port number at random, but
 * avoid reserved ports from 0 thru 999.
 * Also leave range from 1000 thru 1199
 * available for explicit application use.
 */

unshort udp_socket() {

	if(socket) return socket++;

	socket = cticks;
	if(socket < 1200) socket +=1200;
	return socket++;
	}

