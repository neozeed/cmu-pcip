/*  Copyright 1986 by Carnegie Mellon  */
/*  See permission and disclaimer notice in file "cmu-note.h"  */
#include	<cmu-note.h>

/* Command to get the IP address, subnet mask, default gateway, server
   addresses, etc. from a bootp server.  */

#include <stdio.h>
#include <types.h>
#include <task.h>
#include <q.h>
#include <netq.h>
#include <net.h>
#include <ctype.h>

extern int allow_null_ip_addr;

main(argc, argv)
int argc;
char   *argv[];
{
    register char  *ap;
    int     nback = 0;			/* No backoff flag */
    int	    force = 0;			/* Get new IP address anyhow */
    int     toggle = 0;			/* Toggle broadcast address 4.2 */
    int     retries = 0;		/* Number of retries */
    in_name myaddr, bootp();		/* My IP address */

    allow_null_ip_addr = 1;
    Netinit(800);
    in_init();
    UdpInit();
    IcmpInit();
    GgpInit();
    
    argc--, argv++;			/* Parse command line */
    if (argc > 0) {
	ap = argv[0];
	while (*ap) {
	    while (*ap == ' ' || *ap == '-') {
		*ap++;
	    }
	    switch (*ap++) {
		case 'n': 	/* No backoff */
		    nback++;
		    break;
		case 't':	/* Twiddle broadcast address */
		    toggle++;
		    break;
		case 'f':	/* Get new IP address anyhow */
		    force++;
		    break;
		case 'h':	/* Help */
		    printf("Bootp options:\n");
		    printf("\t-n\tDo not increase backoff time between attempts.\n");
		    printf("\t-t\tToggle between official and 4.2 broadcast addresses.\n");
		    printf("\t-f\tForce bootp to set a new IP address.\n");
		    printf("\t-<num>\tDo <num> retries. Default is three.\n");
		    printf("If a valid IP address is already in place, bootp will not overwrite it\n");
		    printf("unless -f option is used.\n");
		    exit(1);
		    break;
		default: 
		    if (isdigit(ap[-1])) {/* Retries ... */
			retries = atoi(--ap);
			*ap++;
			break;
		    }
		    else {
			printf("Usage: boot [-ntf][1-9]. -h for help.\n");
			exit(1);
		    }
	    }
	}
    }

    myaddr = bootp(nback, force, retries, toggle);
    if(!myaddr)
	printf("Error: Bootp servers not responding!\nPlease contact your network administrator.\n");
    else if(myaddr == -1)
	printf("Bootp internal error!\nContact your network administrator.");
    else
	printf("IP address set to %a.\n", myaddr);

#ifdef DEBUG
    if(NDEBUG & NETRACE)
	net_stats(stdout);
#endif
}
