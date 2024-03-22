/*  Copyright 1986 by Carnegie Mellon  */
/*  See permission and disclaimer notice in file "cmu-note.h"  */
#include	<cmu-note.h>
/*  Copyright 1985 by the Massachusetts Institute of Technology  */
/*  See permission and disclaimer notice in file "notice.h"  */
#include	<notice.h>

#include <types.h>
#include <task.h>
#include <q.h>
#include <netq.h>
#include <net.h>
#include <netbuf.h>
#include <timer.h>
#include <stdio.h>
#ifdef MSC			/* DDP */
#include <process.h>		/* DDP */
#endif				/* DDP */

/* Ping's command interpreter
*/

/* works fine for here... */
#define	CTL(x)	((x) - '@')

extern in_name ping_host;

ping_cmd(c)
	char c; {

	switch(c) {
	case 'q':
		exit(0);
		break;

	case '!': {
		extern char *sys_errlist[];
		extern int errno;
		int retcode;

		printf("\nEntering inferior command interpreter. Do not execute any network programs.\n");
		printf("Use the EXIT command to return to ping.\n");
#ifdef MSC			/* DDP */
		retcode = spawnlp(P_WAIT, getenv("COMSPEC"), getenv("COMSPEC"), NULL); /* DDP */
#else				/* DDP */
		retcode = system(0);
#endif			/* DDP */
		if(retcode < 0)
			printf("Can't exec command interpreter: %s\n",
							sys_errlist[errno]);
		else puts("");
		break;
		}

	case 'I':
		printf("My internet address is %a\n", in_mymach(ping_host));
		break;

	case '?':
		showhelp();
		break;
	
	case CTL('A'):
		NDEBUG = (NDEBUG ^ APTRACE);
		break;

	case CTL('D'):
		NDEBUG = (NDEBUG ^ DUMP);
		break;

	case CTL('E'):
		NDEBUG = (NDEBUG ^ NETERR);
		break;

	case CTL('I'):
		NDEBUG = (NDEBUG ^ IPTRACE);
		break;

	case CTL('N'):
		NDEBUG = (NDEBUG ^ NETRACE);
		break;

	case CTL('O'):
		NDEBUG = (NDEBUG ^ TMO);
		break;

	case CTL('P'):
		NDEBUG = (NDEBUG ^ PROTERR);
		break;

	case CTL('T'):
		NDEBUG = (NDEBUG ^ TPTRACE);
		break;

	case CTL('Y'):
		TDEBUG = !TDEBUG;
		break;

	case '\r':
		TIMERDEBUG = !TIMERDEBUG;
		break;

	case CTL('S'):
	case 'n':
		net_stats(stdout);
		printf("NBUF = %d\n", NBUF);
		break;

	case CTL('Q'):
		tk_stats();
		break;

	default:
		printf("unknown command ");
		if(c >= ' ')
			printf("%c. Type ? for help\n", c);
		else
			printf("^%c. Type ? for help\n", c+'@');
		break;
	}

	return;
	}

showhelp() {
	printf("!	Push to an inferior command interpreter\n");
	printf("?	Help\n");
	printf("q	Quit\n");
	printf("Debugging commands:\nControl-\n");
	printf("h	Display this list\n");
	printf("a	Toggle APTRACE (application level trace)\n");
	printf("t	Toggle TPTRACE (transport level (TCP/UDP) trace)\n");
	printf("i	Toggle IPTRACE (IP/ICMP level trace)\n");
	printf("n	Toggle NETRACE (network driver trace\n");
	printf("d	Toggle DUMP debugging switch\n");
	printf("e	Toggle NETERR (network error trace)\n");
	printf("p	Toggle PROTERR (protocol error trace)\n");
	printf("o	Toggle TMO (timeout trace)\n");
	printf("y	Toggle task trace\n");
	printf("j	Toggle timer trace\n");
	printf("s	Display network and IP statistics [also 'n']\n");
	printf("q	Display task table\n");
	printf("\nCurrent debugging flags:\n");
	if(!NDEBUG) printf(" all off");
	if(NDEBUG&APTRACE)printf(" APtrace");
	if(NDEBUG&TPTRACE)printf(" TPtrace");
	if(NDEBUG&IPTRACE)printf(" IPtrace");
	if(NDEBUG&NETRACE)printf(" NETtrace");
	if(NDEBUG&PROTERR)printf(" Proterr");
	if(NDEBUG&NETERR )printf(" Neterr");
	if(NDEBUG&DUMP   )printf(" Dump");
	if(NDEBUG&TMO )printf(" Timeout");
	printf("\n");
	}
