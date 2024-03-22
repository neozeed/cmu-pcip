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
#include <em.h>
#include "telnet.h"

#define	GETDATE	0x2a
#define	GETTIME	0x2c
long get_dosl();

extern NET nets[];
extern in_name tnhost;
char tnshost[40];

pr_banner(shost)
	char *shost; {
	struct ucb	*pucb;

	pucb = &ucb;
	pucb->u_prompt = F10;

	strcpy(tnshost, shost);

	pr_tn();
	printf("Last customized at ");
	p_time(custom.c_ctime);
	printf(" on ");
	p_date(custom.c_cdate);
	printf(".\n");
	printf("Telnet escape character is ");
	showesc(pucb->u_prompt);
	printf("\n");
	}

showcmds() {
	register int	prompt;

	prompt = ucb.u_prompt;

	printf("\nTelnet Commands\n");
	printf("Escape character is ");
	showesc(prompt);
	printf("\n");
	printf("?	Display this list\n");
	printf("a	Send Are-You-There?\n");
	printf("b	Send telnet interrupt (break) in urgent mode\n");
	printf("c	Close connection and exit telnet\n");
	printf("d	Discard characters at end of the line\n");
	printf("e/E	Send on every character/end-of-line\n");
	printf("i/I	Send/show my internet address in decimal format\n");
	printf("l	Local echo\n");
	printf("n	Refuse a file transfer request\n");
	printf("o	Send my internet address in octal format\n");
	printf("q	exit telnet immediately. [Dangerous, see manual]\n");
	printf("r	Remote echo\n");
	printf("t/T	Turn file transfer service off/on\n");
	printf("u/U	Turn status line off/on\n");
	printf("w	Wrap around characters at end of line\n");
	printf("x	Expedite outstanding data\n");
	printf("y	Accept a file transfer  request\n");
	printf("A	Toggle TFTP server asking\n");
	printf("B/D	Have <-- key be backspace/delete\n");
	printf("!	Enter inferior command interpreter\n");
	}

showctls() {

	printf("\nTelnet commands for debugging and maintenance\n");
	printf("\nF10/Control-\n");
	printf("h	Display this list\n");
	printf("a	Toggle APTRACE (application level trace)\n");
	printf("t	Toggle TPTRACE (transport level (TCP/UDP) trace)\n");
	printf("i	Toggle IPTRACE (IP/ICMP level trace)\n");
	printf("n	Toggle NETRACE (network driver trace)\n");
	printf("d	Toggle DUMP debugging switch\n");
	printf("e	Toggle NETERR (network error trace)\n");
	printf("p	Toggle PROTERR (protocol error trace)\n");
	printf("o	Toggle TMO (timeout trace)\n");
	printf("y	Toggle task trace\n");
	printf("j	Toggle timer trace\n");
	printf("s	Display network and IP statistics\n");
	printf("q	Display task table\n");
	printf("u	Display UDP connection table\n");
	printf("w	Display TCP/Telnet operation statistics\n");	
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

/* Print the escape character in a nice format */

showesc(c)
	char c; {

	if (c == F10) printf ("F10");
	else if (c < '\040') printf ("^%c", c|0100);
	else if (c == '\177') printf ("^?");
	else printf ("%c", c);

	}
	
/* Print telnet statistics */

showstats() {
	register struct ucb	*pucb;
	char 	*msg;

	pucb = &ucb;
	pr_tn();
	switch (pucb->u_echom) {
		case LOCAL: 	msg = "local";
				break;
		case REMOTE:	msg = "remote";
				break;
		default:   	msg = "invalid state";
		}

	printf("\nEcho Mode: %15s\t", msg);

	switch (pucb->u_sendm) {
		case EVERYC:	msg = "every character";
				break;
		case NEWLINE:	msg = "newline";
				break;
		default:	msg = "invalid state";
		}

	printf("Send Mode: %15s\n", msg);
	tc_status ();
	}

/* Print the telnet version number & foreign host message */
pr_tn() {

	printf("IBM PC User Telnet Version %u.%u", version/10,
						   version%10);
	printf(" - bugs to pc-ip-request@mit-xx\n");
	printf("To host %s via %s. ", tnshost, nets[0].n_name);
	printf("Time is ");
	p_time(get_dosl(GETTIME));
	printf(" on ");
	p_date(get_dosl(GETDATE));
	printf("\n");
	}

#ifdef notdef		/* DDP - This is the same as tcp_sock() */
/* figure out a neat telnet socket */

tn_sock() {
	long temp;

	temp = get_dosl(GETTIME);
	temp &= 0xffff;
	if(temp < 1000) temp += 1000;
	return (unsigned)temp;
	}
#endif
