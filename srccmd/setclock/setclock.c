/*  Copyright 1986 by Carnegie Mellon  */
/*  See permission and disclaimer notice in file "cmu-note.h"  */
#include	<cmu-note.h>
/*  Copyright 1984,1985 by the Massachusetts Institute of Technology  */
/*  See permission and disclaimer notice in file "notice.h"  */
#include	<notice.h>


/*  Command to get the date and time from a network time server, and
    set the PC clock accordingly.  */
/*  12/3/84--Debugging message updated to use unsigned integers.  
					<J. H. Saltzer>     */

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

long	udptime();

main(argc, argv)
	int argc;
	char *argv[]; {
	in_name time_server;
	long time;
	int i;

	if(argc > 2) {
		printf("Usage:\n\n     setclock\nor\n     setclock host\n");
		exit(1);
		}

#ifdef	DEBUG
	NDEBUG = BUGHALT|NETERR|PROTERR;
	
	if(NDEBUG & INFOMSG) printf("Initializing system.\n");
#endif

	Netinit(1000);
	in_init();
	IcmpInit();
	GgpInit();
	UdpInit();
	nm_init();

	time_server = 0L;
	if(argc == 2) {
		time_server = resolve_name(argv[1]);
		if(time_server == 0L) {
			printf("Host %s not known\n", argv[1]);
			exit(1);
			}
		if(time_server == 1L) {
			printf("Name servers not responding.\n");
			exit(1);
			}
		}		

	for ( i=1; i <= 5; i++) {
#ifdef	DEBUG
		if(NDEBUG & INFOMSG) printf("Calling udptime\n");
#endif
		time = udptime(time_server,i);
#ifdef	DEBUG
		if(NDEBUG & INFOMSG) printf("udptime returned %U\n", time);
#endif
		if(time != 0L) {
			set_pc_clk(time);
			exit(0);
			}
		}

	printf("SETCLOCK:  Time service not responding.  ");
	printf("Clock not set.\n");
	exit(1); 

	}
