/*  Copyright 1986 by Carnegie Mellon  */
/*  See permission and disclaimer notice in file "cmu-note.h"  */
#include	<cmu-note.h>
/*  Copyright 1984 by the Massachusetts Institute of Technology  */
/*  See permission and disclaimer notice in file "notice.h"  */
#include	<notice.h>


/*  Modified to send octal internet address on F10/control-I, to put
a blank after octal and decimal internet address, to leave the line-25
message containing "My internet address" displayed indefinitely, to
leave the message "file transfer in progress" on line 25 for the
duration of the transfer, to accept an upcall from the tftp server
when the transfer is complete, to display a message on line 25
indicating success or failure of that transfer, and to leave the
line-25 message "closing connection" displayed indefinitely, 12/23/83.
F10/A toggles TCP tracing, 12/28/83.
Line 25 message for tftp now gives name of file and host, 1/2/84.
					<J. H. Saltzer>
2/3/84 - changed to allow the status line (25th line) display to be
	turned off. <John Romkey>
2/8/84 - commented out call to scr_close() and _curse() because the
	emulator's stdne() already does it.	<John Romkey>
2/24/84 - changed code to use new printf internet address fields
	instead of having duplicated code all over the place.
						<John Romkey>
8/12/84 - added a space in the new printf() calls after the IP address.
						<John Romkey>
8/30/84 - added F10A to turn off TFTP server asking.
						<John Romkey>
6/2/85 - added F10/control menu and commands to toggle all debugging
        switches.
					      	<J. H. Saltzer>
10/14/85 - changed putchar("$") to putchar('$').  Made conditional changes
	for use with Microsoft C V3.00.
						<Drew D. Perkins>
11/14/85 - merged in changes for IBM 3278 emulation from Jacob Rekhter
	IBM/ACIS.
						<Drew D. Perkins>
1/9/86 - Send IAC DO ECHO upon opening of the connection.
						<Drew D. Perkins>
3/21/86	- Merge in color support for IBM 3279 emulation from Jacob Rekhter
	IBM/ACIS.
						<Drew D. Perkins>
3/32/86 - Break up gt_usr routine so MSC can optimize it.  It got too big.
						<Drew D. Perkins>
*/

#include <stdio.h>
#include <types.h>
#include <task.h>
#include <q.h>
#include <netq.h>
#include <net.h>
#include <custom.h>
#include <netbuf.h>
#include <icmp.h>
#include	<ip.h>
#include	"telnet.h"
#include	<em.h>
#include	<tftp.h>
#ifdef MSC			/* DDP */
#include	<process.h>	/* DDP */
#endif				/* DDP */
#ifdef E3270			/* DDP */
#include	<e3278.h>	/* DDP */
#endif				/* DDP */

#define	C_L	0x0c	/* Control-L */

extern long cticks;	/* number of clock ticks since program start */
extern int wrap_around;	/* h19 emulator wrap-around flag */
extern int ba_bs;	/* h19 emulator backarrow key flag */
extern unsigned clear25;
extern in_name tnhost;
extern int TIMERDEBUG;
extern int NBUF;

/* DDP - TA Fellela begin.  Public vars for color support. */
int display_buf;
char protn, proth, unprotn, unproth, backg, nondisp;
/* DDP - TA Fellela end.  Public vars for color support. */

int chartest;
int scrntest;
int ct1time;
int ct2time;
int ctcount;
int st1time;
int st2time;

char buf1[80];
char numchar[10];
/*
char flashbuf[600] = "No flash messages\n";
*/
 
/* DDP - Begin */
#ifdef E3270
extern unsigned char work[];	/* work area */
extern struct packet far xmt;	/* CCK - xmt packet */
extern struct packet far rcv;	/* CCK - rcv packet */
#endif

static char *term_types[] = {
	"ZENITH-H19",
#ifdef E3270
	"IBM-3278-2",
#endif
	NULL
};
/* DDP - End */


tel_init() {
	register struct ucb *pucb;

	pucb = &ucb;

	pucb->u_state = ESTAB;
	pucb->u_tftp = TFUNKNOWN;
	pucb->u_tcpfull = 0;
	pucb->u_rstate = NORMALMODE;
	pucb->u_rspecial = NORMALMODE;
	pucb->u_wstate = NORMALMODE;
	pucb->u_wspecial = NORMALMODE;
	pucb->u_sendm = custom.c_1custom & NLSET ? NEWLINE : EVERYC;
	pucb->u_echom = LOCAL;
	pucb->u_echongo = NORMALMODE;
	pucb->u_mode = TRUE;
	pucb->u_ask = custom.c_1custom & TN_TFTP_ASK ? FALSE : TRUE;
#ifdef E3270				/* DDP - Begin */
	pucb->u_terminal = ASCII_TERM;	/* Default to ASCII */

	/* sets the correct buffer for monochrome vs color */
	/* returns mode of monitor			   */

	if(monitor_type()==3)
		term_types[1] = "IBM-3279-2";
#endif					/* DDP - End */
	chartest = 0;
	scrntest = 0;
	ct1time = 0;
	ct2time = 0;
	st1time = 0;
	st2time = 0;

	if(custom.c_1custom & BSDEL) ba_bs = !ba_bs;
	if(custom.c_1custom & WRAP) wrap_around = !wrap_around;

#ifdef E3270			/* DDP */
	e3278i(work);		/* DDP - initialize 3278 stuff just in case */
#endif				/* DDP */
	}

tel_exit() {
	extern int x_pos, y_pos, pos;

	norm25();
	move_lines(1, 0, 24);
	set_cursor(pos = 24*80);
	x_pos = 0;
	y_pos = 24;
	scr_close();
	_curse();
	}

/* Return true if telnet must run; false otherwise. */

mst_run() {
	return 1; /*fready(stdin);*/
	}

bfr() {
	ucb.u_tcpfull = 0;
	clr25();
	}

/* gt_usr  Read nch chars from user's terminal
* When prompt char is encountered, go into
*  SPECIAL read mode and handle char following
*  prompt specially.
* Tc_put puts telnet chars into an output
*  to net packet.
*/

gt_usr() {
	register struct ucb	*pucb;
	int	c;
	int	i;
	int	ich;
	static 	int	inc;
#ifdef E3270				/* DDP */
	int pause = 0;			/* DDP */
#endif					/* DDP */

	pucb = &ucb;

	for(;;) {
	tk_yield();

#ifdef E3270				/* DDP - Begin changes */
	if (pucb->u_terminal == E3278_TERM &&
	    pucb->u_rspecial == NORMALMODE) {
		e3278input();
		continue;
	}
#endif					/* DDP - End changes */

	while((c = h19key()) != NONE) {
		switch(pucb->u_rspecial) {
		case HOLD:
			continue;
		case NORMALMODE:
			normal_input(c); /* DDP - Broke off so compiler can optimize */
			break;

		case SPECIAL:
			pucb->u_rspecial = NORMALMODE;
/*			if(c == pucb->u_prompt) {
				if(pucb->u_sendm == EVERYC) {
					if(tc_fput(c)) tcpfull();
					}
				else
					if(tc_put(c)) tcpfull();
				break;
				}
*/
			switch(c) {
			case 'q':
				pucb->u_rspecial = CONFIRM;
				clr25();
				prbl25("Dangerous, confirm with Y: ");
				break;

/*			case 'C': {
				int fd;

				pr25(9, "Dumping core");
				fd = creat("core", 1);
				if(fd < 0) {
					perror("couldn't open core file\n");
					break;
					}
				write(fd, 0, 65535);
				close(fd);
				printf("done\n");
				tel_exit();
				exit(1);
				}
				break;
*/

			case '!': {
				extern char *sys_errlist[];
				extern int errno;
				int retcode;

				norm25();
#ifndef E3270				/* DDP */
				clear_lines(24, 1); /* DDP */
#else					/* DDP */
				clear_lines(0, 25);
#endif					/* DDP */
				printf("\nEntering inferior command interpreter. Do not execute any network programs.\n");
				printf("Use the EXIT command to return to telnet\n");
				_curse();
				scr_close();
#ifdef MSC		/* DDP */
				retcode = spawnlp(P_WAIT, getenv("COMSPEC"), getenv("COMSPEC"), NULL); /* DDP */
#else			/* DDP */
				retcode = system(0);
#endif			/* DDP */
				scr_init();
				if(retcode < 0)
					printf("Can't exec command interpreter: %s\n", sys_errlist[errno]);
				else puts("");
				break;
				}

			case 'A':
				pucb->u_ask = !pucb->u_ask;
				pr25(9, "Toggle tftp server asking.");
				norm25();
				clear25 = 4;
				break;

			case 'u':
				pucb->u_mode = FALSE;
				norm25();
				clear_lines(24, 1);
				break;

			case 'U':
				pucb->u_mode = TRUE;
				pr25(9, "Turn on status line.");
				norm25();
				clear25 = 4;
				break;


			case 'T':
				tfs_on();
				pr25(9, "File transfer service turned on.");
				norm25();
				clear25 = 4;
				break;

			case 't':
				tfs_off();
				pucb->u_tftp = TFNO;
				pr25(9, "File transfer service turned off.");
				norm25();
				clear25 = 4;
				break;

			case 'I':
				{
				in_name me;
				char buffer[20];

				me = in_mymach(tnhost);
				sprintf(buffer, "%A = %a",me, me);
				pr25(0, "My internet address is ");
				pr25(23, buffer);
				norm25();
				clear25 = 0;
				}
#ifdef E3270				/* DDP */
				pause++; /* DDP */
#endif					/* DDP */
				break;

			case 'i':
				{
				in_name me;
				char buffer[20];

				me = in_mymach(tnhost);
				sprintf(buffer, "%a ", me);

				ich = 0;
				for(ich = 0; buffer[ich]; ich++) {
					if(tc_put(buffer[ich])){
						tcpfull();
						break;
						}
					if(pucb->u_echom == LOCAL)
						putchar(buffer[ich]);
					}

				if(pucb->u_sendm == EVERYC) tcp_ex();

			       pr25(0,"Send my internet address in decimal.");
				norm25();
				clear25 = 2;
				}
				break;

			case 'o':
				{
				in_name me;
				char buffer[20];

				me = in_mymach(tnhost);
				sprintf(buffer, "%A ", me);

				for(ich = 0; buffer[ich]; ich++) {
					if(tc_put(buffer[ich])){
						tcpfull();
						break;
						}
					if(pucb->u_echom == LOCAL)
						putchar(buffer[ich]);
					}

				if(pucb->u_sendm == EVERYC) tcp_ex();

				pr25(0,"Send Internet address in octal");
				norm25();
				clear25 = 2;
				}
				break;

			case 'c':
				tcp_close();
				pucb->u_state = CLOSING;
				pucb->u_rstate = BLOCK;
				pr25(9, "Closing connection.");
				norm25();
				clear25 = 0;
				break;
/*
			case 'S':
				if(!tcp_save()) {
					prbl25("Suspend failed!");
					clear25 = 0;
					break;
					}
				pr25(9, "Suspending telnet.");
				norm25();
				clear25 = 0;
				tel_exit();
				exit();
*/
			case 'n':
				pucb->u_tftp = TFNO;
				pr25(9, "File transfer refused.");
				norm25();
				clear25 = 4;
				break;


			case 'y':
				pucb->u_tftp = TFYES;
				clr25();
				pr25(0, buf1);
				norm25();
				clear25 = 0;
				break;

			case 'D':
				ba_bs = 0;
				pr25(9, "<-- key is delete.");
				norm25();
				clear25 = 4;
				break;

			case 'B':
				ba_bs = 1;
				pr25(9, "<-- key is backspace.");
				norm25();
				clear25 = 4;
				break;

			case 'w':
				wrap_around = 1;
				pr25(9, "Turn end-of-line wrap-around on.");
				norm25();
				clear25 = 4;
				break;

			case 'd':
				wrap_around = 0;
				pr25(9, "Turn end-of-line discard on.");
				norm25();
				clear25 = 4;
				break;

			case 'E':
				pucb->u_sendm = NEWLINE;
				pr25(9, "Send on end-of-line.");
				norm25();
				clear25 = 4;
				break;

			case 'e':
				pucb->u_sendm = EVERYC;
				pr25(9, "Send on every character.");
				norm25();
				clear25 = 4;
				break;

			case 'l':
				pr25(9, "Local echo mode.");
				norm25();
				clear25 = 4;
				if(pucb->u_echom == LOCAL)
					break;
				echolocal(pucb);
				pucb->u_echongo = LECHOREQ;
				break;

			case 'r':
				pr25(9, "Remote echo mode.");
				norm25();
				clear25 = 4;
				if(pucb->u_echom == REMOTE)
					break;
				echoremote(pucb);
				pucb->u_echongo = RECHOREQ;
				break;

			case 'a':
				pr25(9, "Sending Are-You-There.");
				norm25();
				clear25 = 4;
				if(tc_put(IAC)) {
					tcpfull();
					break;
					}
				if(tc_fput(AYT)) tcpfull();
				break;

			case 'z':
				tc_put(IAC);
				tc_fput(AO);
				pr25(9, "Sending abort output.");
				norm25();
				clear25 = 4;
				break;

			case 'b':
			case C_BREAK:
				tc_put(IAC);
				tc_put(INTP);
				tc_put(IAC);
				tc_put(DM);
				tcpurgent();
				pr25(9, "Sending break.");
				norm25();
				clear25 = 4;
				break;

			case 'x':
				tcp_ex();
				pr25(9, "Expediting data.");
				norm25();
				clear25 = 4;
				break;

			case '?':
				showcmds();
				pr25(9, "Help.");
				norm25();
				clear25 = 4;
#ifdef E3270				/* DDP */
				pause++; /* DDP */
#endif					/* DDP */
				break;

/* works fine for here... */
#define	CTL(x)	((x) - '@')

			case CTL('H'):
				showctls();
				norm25();
				clr25();
#ifdef E3270				/* DDP */
				pause++; /* DDP */
#endif					/* DDP */
				break;
	
			case CTL('A'):
				NDEBUG = (NDEBUG ^ APTRACE);
				norm25();
				clr25();
				break;

			case CTL('D'):
				NDEBUG = (NDEBUG ^ DUMP);
				norm25();
				clr25();
				break;

			case CTL('E'):
				NDEBUG = (NDEBUG ^ NETERR);
				norm25();
				clr25();
				break;

			case CTL('I'):
				NDEBUG = (NDEBUG ^ IPTRACE);
				norm25();
				clr25();
				break;

			case CTL('N'):
				NDEBUG = (NDEBUG ^ NETRACE);
				norm25();
				clr25();
				break;

			case CTL('O'):
				NDEBUG = (NDEBUG ^ TMO);
				norm25();
				clr25();
				break;

			case CTL('P'):
				NDEBUG = (NDEBUG ^ PROTERR);
				norm25();
				clr25();
				break;

			case CTL('T'):
				NDEBUG = (NDEBUG ^ TPTRACE);
				norm25();
				clr25();
				break;

			case CTL('Y'):
				TDEBUG = !TDEBUG;
				norm25();
				clr25();
				break;

			case '\r':
				TIMERDEBUG = !TIMERDEBUG;
				norm25();
				clr25();
				break;

			case CTL('S'):
				net_stats(stdout);
				printf("NBUF = %d\n", NBUF);
				norm25();
				clr25();
#ifdef E3270				/* DDP */
				pause++; /* DDP */
#endif					/* DDP */
				break;

			case CTL('Q'):
				tk_stats();
				norm25();
				clr25();
#ifdef E3270				/* DDP */
				pause++; /* DDP */
#endif					/* DDP */
				break;

			case CTL('U'):
				udp_table();
/*				space_display();	*/
				norm25();
				clr25();
#ifdef E3270				/* DDP */
				pause++; /* DDP */
#endif					/* DDP */
				break;

			case CTL('W'):
				showstats();
				norm25();
				clr25();
#ifdef E3270				/* DDP */
				pause++; /* DDP */
#endif					/* DDP */
				break;

			default:
				clr25();
				norm25();
				if (c != pucb->u_prompt){
					prerr25("Unknown command.");
					clear25 = 4;}
				}
#ifdef E3270				/* DDP - Begin changes */
			if (pause && (pucb->u_terminal == E3278_TERM)) {
				pr25(40, "Press any key to continue");
				while (h19key() == NONE)
					;
				norm25();
				--pause;
			}
#endif					/* DDP - End changes */
			rst_screen();
			break;

		case CONFIRM:
			pucb->u_rspecial = NORMALMODE;
			if(c == 'y' || c == 'Y') {
				clr25();
				pr25(0, "Confirmed, quitting.");
/*				tel_exit();	*/
				exit();
				}
			clr25();
			pr25(0, "Quit aborted.");
			norm25();
			clear25 = 4;
			break;

		default:
			printf("\nTelnet BUG %u\n", pucb->u_rspecial);
			pucb->u_rspecial = NORMALMODE;
			break;
			}
		}
	}
}


/*
DDP - This routine was broken out of gt_user() in because it was too big for
the optimizer.
 */
normal_input(c)
register char c;
{
	register struct ucb	*pucb;

	pucb = &ucb;
	if(c == pucb->u_prompt)  {
		pucb->u_rspecial = SPECIAL;
		clear25 = 0;
		inv25();
		clr25();
		pr25(0, "Command: ");
		return;
		}

	if(pucb->u_tcpfull) {
		tcpfull();
		return;
		}

	if(pucb->u_echom == LOCAL) {
		if(c == '\r') putchar('\n');

		if(c == '\n' || c == '\r' || c >= ' ' ||
		   c == 9    || c == 5    || c == 8)
				putchar(c);
		else if(c == 27) putchar('$');
		else if(c == F9) {
			putchar('F');
			putchar('9');
			}
		else if(c == F10) {
			putchar('F');
			putchar('1');
			putchar('0');
			}
		else if(c == C_BREAK || c == C_L) ;
		else {
			putchar('^');
			putchar(c + '@');
			}
		}


	if(c == '\r') c = '\n';
	else if(c == '\n') tc_put('\r');
	else if(c == IAC) tc_put(IAC);
	else if(c == C_BREAK) {
		tc_put(IAC);
		tc_put(INTP);
		tc_put(IAC);
		tc_put(DM);
		tcpurgent();
		return;
		}

	if(pucb->u_sendm == EVERYC) {
		if(tc_fput(c)) tcpfull();
		}
	else {
		if(tc_put(c)) {
			tcpfull();
			return;
			}
		if(c == '\n') tcp_ex();
		}
}


#ifdef E3270				/* DDP - Begin */
/*
This routine was broken out of gt_user() in because it was too big for the
optimizer.
 */

e3278input()
{
	register struct ucb	*pucb;
	int count;
	int rc;

	pucb = &ucb;

	while ((rc = screenio(work)) != PACKETIN) {
		if (rc == NEWAID) {
			xmt.cmd = RDMOD;
			count = e3278(work, xmt.cmd, 0, &xmt.data[0]);
			xmt.ptype = USERINT;
			netwrite(&xmt.data[0], count);
		}
		else if (rc == ESCAPED) {
			pucb->u_rspecial = SPECIAL;
			clear25 = 0;
			inv25();
			clr25();
			pr25(0, "Command: ");
			break;	/* get out of while loop */
		}
		else
			printf("unknown rc = %d\n", rc);
	}
}
#endif					/* DDP - End */


/* wr_usr  manage chars coming from net and
*   going to user
* Process received telnet special chars and
*  option negotiation.  When wstate is URGENTM,
*  only process special chars.
* All interrupts should be turned off
*  when in this routine - write to terminal
*  may block; an interrupt would cause
*  an error return from write with resulting
*  loss of chars to terminal
*/
wr_usr(buf, len, urg)
	char *buf;
	int len;
	int urg; {
	register struct ucb	*pucb;
	register char	*p;
	int c;
/* DDP - Begin changes */
	int i;
	static char **terminal = term_types;
	char *cp;
#ifdef E3270
	static int count = 0;
	int rc;
	static int sentibm = 0;	/* Sent terminal type as IBM-xxx */
#endif				/* DDP - End changes */

	pucb = &ucb;

	for(p = buf; p < buf+len; p++) {
		c = (*p & 0377);
		switch(pucb->u_wspecial) {
		case NORMALMODE:
			switch(c) {
			case IAC:
				pucb->u_wspecial = IAC;
				break;
			default:
#ifdef E3270			/* DDP - Begin changes */
				if (pucb->u_terminal == E3278_TERM) {
					i = bsctrcp(p,
					((char far *)&(rcv.cmd)) + count,
					(buf+len)-p);

					count += i;	/* bump count */
					p += (i-1);	/* take into account
							    p++ at for( )   */
				}
				else
#endif				/* DDP - End changes */
				/* Don't print ^L because Multics sends them
					quite often */
				if(pucb->u_wstate != URGENTM && c != C_L)
								em(c & 0177);

			}
			break;

		case IAC:
			switch(c) {
			case IAC:
#ifdef DEBUG
				if(NDEBUG & APTRACE)
					printf("Telnet: RCVD IAC IAC\n");
#endif

				if(pucb->u_wstate != URGENTM)
					putchar(c);
				pucb->u_wspecial = NORMALMODE;
				break;

			case AO:
#ifdef DEBUG
				if(NDEBUG & APTRACE)
					printf("Telnet: RCVD IAC AO\nTelnet: SENT IAC DM\n");
#endif

				tc_put(IAC);
				tc_put(DM);
				tcpurgent();
				pucb->u_wspecial = NORMALMODE;
				break;

			case WILL:
			case WONT:
			case DO:
			case DONT:
				pucb->u_wspecial = c;
				break;

/*			case GA:
				pucb->u_wspecial = NORMALMODE;
				putchar('G');
				putchar('A');
				break;		*/

/* DDP - Begin changes */
			case SBNVT:
				pucb->u_wspecial = c;
				break;

			case SENVT:
				tc_put(IAC);
				tc_put(SBNVT);
				tc_put(TERMTYPE);
				tc_put(IS);

#ifdef DEBUG
				if(NDEBUG & APTRACE)
					printf("Telnet: RCVD IAC SENVT\nTelnet: SENT IAC SBNVT TERMTYPE IS %s IAC SENVT\n", *terminal);
#endif

				for (cp = *terminal; *cp; cp++)
					tc_put(*cp);
#ifdef E3270
				if(!strncmp("IBM", *terminal, 3))
					sentibm++;
#endif

				if(terminal[1] != NULL)
					terminal++;
				tc_put(IAC);
				tc_fput(SENVT);
				pucb->u_wspecial = NORMALMODE;
				break;
#ifdef E3270
			case EOR:
#ifdef DEBUG
				if(NDEBUG & APTRACE)
					printf("Telnet: RCVD IAC EOR\n");
#endif

				if (pucb->u_terminal != E3278_TERM)
					break;
				rcv.ptype = CMD327X;

				/* process incoming screen */
				rc = e3278(work, rcv.cmd, count - 1,
				&rcv.data[0]);

				if (rc > 0) {
					rcv.ptype = RESPONSE;
					netwrite(&rcv.data[0], rc);
				}

				pucb->u_wspecial = NORMALMODE;
				count = 0;	/* reset count  */
				break;
#endif				/* DDP - End changes */

		/* Ignore IAC x */
			default:
#ifdef DEBUG
				if(NDEBUG & APTRACE)
					printf("Telnet: RCVD IAC %d\n", c);
#endif

				pucb->u_wspecial = NORMALMODE;
				break;
				}
			break;

		case WILL:
			switch(c) {
			case OPTECHO:
#ifdef DEBUG
				if(NDEBUG & APTRACE)
					printf("Telnet: RCVD IAC WILL ECHO\n");
#endif

				switch(pucb->u_echongo) {
		/* This host did not initiate echo negot - so respond */
				case NORMALMODE:
					if(pucb->u_echom != REMOTE)
						echoremote(pucb);
					break;
		/* Rejecting my IAC DONT ECHO  (illegit) */
				case LECHOREQ:
					ttechoremote(pucb);
					break;
					}

				pucb->u_echongo = NORMALMODE;
				break;

			case OPTSPGA:	/* suppress GA's */
#ifdef DEBUG
				if(NDEBUG & APTRACE)
					printf("Telnet: RCVD IAC WILL SPGA\nTelnet: SENT IAC DO SPGA\n");
#endif

				tc_put(IAC);
				tc_put(DO);
				tc_fput(c);
				break;

#ifdef E3270			/* DDP - Begin changes */
			case TRANS_BIN:
			    /* Don't go into 3270 mode unless we've sent
				the terminal type as an IBM-xxx! */
			    if (sentibm) {
#ifdef DEBUG
				if(NDEBUG & APTRACE)
					printf("Telnet: RCVD IAC WILL BIN\nTelnet: SENT IAC DO BIN\n");
#endif

				tc_put(IAC);
				tc_put(DO);
				tc_fput(c);

				/* now we know that we will run 3278
						   terminal emulator */
				pucb->u_terminal = E3278_TERM;
				break;
			    } else
				;
				/* Fall through */
#endif				/* DDP - End changes */

			default:
#ifdef DEBUG
				if(NDEBUG & APTRACE)
					printf("Telnet: RCVD IAC WILL %d\nTelnet: SENT IAC DON'T %d\n", c, c);
#endif

				tc_put(IAC);
				tc_put(DONT);
				tc_fput(c);
				break;
				}

			pucb->u_wspecial = NORMALMODE;
			break;

		case WONT:
			switch(c) {
			case OPTECHO:
#ifdef DEBUG
				if(NDEBUG & APTRACE)
					printf("Telnet: RCVD IAC WON'T ECHO\n");
#endif

				switch(pucb->u_echongo) {
		/* This host did not initiate echo negot - so respond */
				case NORMALMODE:
					if(pucb->u_echom != LOCAL)
						echolocal(pucb);
					break;
		/* Rejecting my IAC DO ECHO */
				case RECHOREQ:
					ttecholocal(pucb);
					break;
					}

				pucb->u_echongo = NORMALMODE;
				break;
				}

			pucb->u_wspecial = NORMALMODE;
			break;

		case DO:
/* DDP - Begin changes */
			switch (c) {
			case TERMTYPE:
#ifdef DEBUG
				if(NDEBUG & APTRACE)
					printf("Telnet: RCVD IAC DO TERMTYPE\nTelnet: SENT IAC WILL TERMTYPE\n");
#endif

				tc_put(IAC);
				tc_put(WILL);
				tc_fput(c);
				break;
#ifdef E3270
			case TRANS_BIN:
#ifdef DEBUG
				if(NDEBUG & APTRACE)
					printf("Telnet: RCVD IAC DO BIN\nTelnet: SENT IAC WILL BIN\n");
#endif

				tc_put(IAC);
				tc_put(WILL);
				tc_fput(c);
				break;
#endif
			default:
#ifdef DEBUG
				if(NDEBUG & APTRACE)
					printf("Telnet: RCVD IAC DO %d\nTelnet: SENT IAC WON'T %d\n", c, c);
#endif
				tc_put(IAC);
				tc_put(WONT);
				tc_fput(c);
				pucb->u_wspecial = NORMALMODE;
				break;
			}
/* DDP - End changes */

		case DONT:
			pucb->u_wspecial = NORMALMODE;
			break;

/* DDP - Begin changes */
		case SBNVT:
			if ( c != TERMTYPE)
				printf(" SBNVT c = %x\n", c & 0xff);
			pucb->u_wspecial = c;
			break;

		case TERMTYPE:
			if (c != SEND)
				printf(" TERMTYPE c = %x\n", c& 0xff);
			pucb->u_wspecial = c;
			break;
		case SEND:
			if (c != IAC)
				printf(" SEND c = %x\n", c & 0xff);
			pucb->u_wspecial = c;
			break;
/* DDP - End changes */
			}
		}
}


echolocal(pucb)
register struct ucb	*pucb;
{
#ifdef DEBUG
	if(NDEBUG & APTRACE)
		printf("Telnet: SENT IAC DON'T ECHO\n");
#endif

	tc_put(IAC);
	tc_put(DONT);
	tc_fput(OPTECHO);
	pucb->u_echom = LOCAL;
/*	foptions(STECHO);	*/
}

ttecholocal(pucb)
	register struct ucb	*pucb; {

	pucb->u_echom = LOCAL;
/*	foptions(STECHO);	*/
	}

echoremote(pucb)
	register struct ucb	*pucb; {

#ifdef DEBUG
	if(NDEBUG & APTRACE)
		printf("Telnet: SENT IAC DO ECHO\n");
#endif

	tc_put(IAC);
	tc_put(DO);
	tc_fput(OPTECHO);
	pucb->u_echom = REMOTE;
/*	foptions(STNECHO);		*/
	}

ttechoremote(pucb)
	register struct ucb	*pucb; {

	pucb->u_echom = REMOTE;
/*	foptions(STNECHO);		*/
	}

opn_usr() {
	echoremote();		/* DDP - Attempt to change to REMOTE echo */
	printf("Open\n"); }

cls_usr() {
	printf("Closed\n");
/*	tel_exit();	*/
	exit(); }

du_usr() {
	printf("Destination Unreachable\n");
/*	tel_exit();	*/
	exit(); }

tmo_usr() {
	printf("Host not responding\n");
/*	tel_exit();	let exit_hook do the work */
	exit(); }

pr_dot() {
	putchar('.');
	}

tcpfull() {
	ucb.u_tcpfull = 1;
	clr25();
	norm25();
	prbl25("Output buffer full");
	clear25 = 0;
/*	fullbell();	*/
	}


tntftp(host, file, dir)
	in_name host;
	char *file;
	unsigned dir; {
	char buffer[80];
	char c;

	ring();

	if(host == tnhost){
	sprintf(buffer,"Host %s wants to %s file %s; OK? [F10/y or F10/n]",
			tnshost,  dir == PUT ? "read" : "write",file);
	sprintf(buf1,"Host %s is %s file %s",
			tnshost, dir == PUT ? "reading" : "writing", file);
			}
	else		{
	sprintf(buffer, "Host %a wants to %s file %s; OK? [F10/y or F10/n]",
				host, dir == PUT ? "read" : "write",file);
	sprintf(buf1, "Host %a is %s file %s",
			host, dir == PUT ? "reading" : "writing", file);
			}

	if(ucb.u_ask) {
		inv25();
		pr25(0, buffer);
		clear25 = 0;
		}
	else {
		pr25(0, buf1);
		clear25 = 10;
		}

/*if(FALSE)return true;*/	/*TEMPORARY DEBUGGING AID*/

	if(ucb.u_ask) {
		ucb.u_tftp = TFWAITING;

		while(ucb.u_tftp == TFWAITING) tk_yield();

		if(ucb.u_tftp == TFYES)
			return TRUE;
		else
			return FALSE;
		}
	else return TRUE;

	}  /*  end of tntftp()  */

/*  function called when file transfer is done.  */

tntfdn(success)
	int	success;
	{
	clr25();
	if (success) pr25(0, "File transfer successful.");
	    else     pr25(0, "File transfer failed.");
	ring();
	*buf1 = '\0';
	clear25 = 6;
	}
#ifdef E3270				/* DDP - Begin changes */

/* Send count number of bytes beginnig at buf via tcp connection.
   Mark end of data stream for WISCNET package by IAC EOR.
   When tcp connection is full yield current task, so TCP_SEND will
   be able to run
 */



/* Send count number of characters via tcp connection */
netwrite(buf,count)
register char far *buf;
register int count;
{
	struct ucb *pucb = &ucb;
	int i;

	twrite_far(buf, count);
	if (tc_put(IAC)) {
		tcpfull();
		tcp_ex();	/* expedite outstanding data */
		while (pucb->u_tcpfull)
			tk_yield();
	}

	if (tc_fput(EOR)) {
		tcpfull();
		tcp_ex();	/* expedite outstanding data  */
		while (pucb->u_tcpfull)
			tk_yield();
	}
}
#endif				/* DDP - End changes */
