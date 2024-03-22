/********  POP  **********
* Client process for     *
* Post Office Protocol 2 *
* for the IBM PC         *
*************************/

/*  Copyright 1987, 1988 by USC Information Sciences Institute (ISI) */
/*  See permission and disclaimer notice in file "isi-note.h"  */
#include	"isi-note.h"

/*  Copyright 1984 by the Massachusetts Institute of Technology  */
/*  See permission and disclaimer notice in file "notice.h"  */
#include	<notice.h>

/* written in 1984 by Dale M. Chase at ISI
 *
 *#2 4/16/84 Change do_data to do netascii newline xformations.  <DMC>
 *
 *#12 Fixup for loop logic in expect.  5/29/84  dmc 
 *
 *#14 Increase timeout and improve handling.  5/31/84  dmc
 *
 *#16 In tgets, don't allocate resptime every call, do it in main instead;
 *	 increase RESPTIME if dbg is on.		6/5/84  dmc
 *
 *#19 Replace wakes of TCPsend with calls to tcp_ex(), which does PUSH.
 *							6/11/84  dmc
 *#85029,85030 Check for overflow in cmd_rcv + general cleanup
 *
 * Added password switch, rewrote switch reader, and slapped on an #ifdef
 *       so that this will compile under Microsoft C.  6/19/87 Koji Okazaki
 *
 * Note: The default method of reading in the user's password from
 *	 custom.c_user in 'netdev.sys' requires a modified custom program
 *	 (which is built from a modified menu_def.c) so the user can enter
 *	 both his/her username and password in this custom structure member.
 *	 The altered menu_def.c can be had from koji@venera.isi.edu.  However,
 *	 as recording the user's password to his/her 'netdev.sys' file is
 *	 not safe and secure (anyone can find out someone else's password
 *	 by using a text editor and examining their 'netdev.sys' file),
 *	 it is recommended that the password switch be used to read in the
 *	 user's password.  - Koji Okazaki
 * 	 
 */

/* pop2.c for the IBM PC
 * 
 * User mode pop2 
 *
 * Calling sequence:
 * 	pop2 [-dfFOLDER pPASSWD] <local-file> <host> [<dbg>]
 */

typedef	long	time_t;			/* ugly! */

#ifdef MSC	/* koji - begin */
#include	<stdio.h>
#include	<types.h>
#include	<netq.h>
#include	<task.h>
#endif		/* koji - end */
#include	<ip.h>
#include	<timer.h>
#include	<custom.h>

#define	WINDOW		536		/*#85029 seems reasonable */
#define	POPBUF		540		/*#85029 */
#define	POPSOCK		109		/* pop server socket */
#define	RESPTIME	120		/*#85029 maximum response timeout */

char	cmdbuf[POPBUF];			/* command buffer */
int	buflen;				/* size of string in cmd buffer */
char	cirbuf[POPBUF];			/* circular buffer for cmd input */
char	*cirput = cirbuf;		/* put ptr for circular buf */
char	*cirget = cirbuf;		/* get ptr for circular buf */
char	termbuf[POPBUF];		/* termination line */
int	termcode;			/* program termination code */
int	dbg;				/* debugging flag */
int	success;			/* successful transfer flag */
int	quitin = 0;			/* Sending QUIT command */
task	*tk_pop2;			/* the pop2 user task */
timer	*resptime;			/* current response timer */
event	conn_open;			/* connection open event */
event	in_avail;			/* input available event */
event	spc_avail;			/* output buffer space available */
event	rsptmo;				/* response timeout event */
long	datalen;			/* length of mail file from server */
char	*mailfile;			/* global ptr to local file */
int	fileflag = 0;			/* Flag: non default folder */
int	delflag = 0;			/* Flag: delete msgs from folder */
int	passflag = 0;			/* Flag: password switch flag -koji */
char	plus = '+';
char	minus = '-';
char	pound = '#';
char	equals = '=';

extern	int	cmd_rcv(), sm_tmo();
extern	int	us_opna(), us_cls(), us_tmo(), bfr(), us_yld(), pr_dot();
extern	long	atol();
extern	int	optind;
extern	char	*optarg;


main(argc, argv)
	int	argc;
	char	**argv; {
	FILE	*mlfd;			/* mail file desc */
	in_name	fhost;			/* foreign host address */
	int	flag;			/* debug flags */
	char	*fname;			/* ptr to remote filespec */
	char	*pass;			/* ptr to pw */
	int	i;
	int	skip_argv = 1;		/* number of argv's to skip later
					   (the 1st one is argv[0] - koji */

	if(argc < 3 || argc > 6)
		xusage();

	while ((i = getopt(argc, argv, "df:p:")) != EOF)
			switch(i) {
			case 'd':
				if(delflag) xusage();
				delflag = 1;
				skip_argv++;
				break;
			case 'f':
				fileflag = 1;
				fname = optarg;
				if (!delflag) skip_argv++;
				break;
			case 'p':
				passflag = 1;
				pass = optarg;
				skip_argv++;
				break;
			case '?':
			default:
				xusage();
			}

	/* No more switches */

	for (i = 0; i < skip_argv; i++) argc--;

	if(argc == 3) {
		flag = atoi(argv[skip_argv+2]);
		dbg = flag & 1;
		printf("Debug mode: %d\n",dbg);
		}
	
	if(*fname == '"' && fname[strlen(fname)-1] == '"') {
		fname[strlen(fname)-1] = 0;
		fname++;
		}

	tcp_init(512, us_opna, cmd_rcv, us_yld, us_cls, us_tmo, pr_dot, bfr);
	
	if((fhost = resolve_name(argv[skip_argv+1])) <= 1L) {
		if (fhost == 0L)
			printf("550 Never heard of host %s\n", argv[1]);
		else if (fhost == 1L)
			printf("550 Name servers not responding\n");
		else printf("550 Bad address from resolve_name\n");
		exit(1);
		}
	
	
	resptime = tm_alloc();		/*#16 Do this once only, here */

	tk_pop2 = tk_cur;
	
	tcp_open(&fhost, POPSOCK, tcp_sock(), WINDOW, WINDOW/2);

	if (!passflag)
	{
		for (pass = custom.c_user; *pass != '\0'; pass++)
			;
		if (*++pass == '\0') {
			printf("No password\n");
			exit(1);
		}
	}

	mailfile = argv[skip_argv];	/* Point to file string */

	if((mlfd = fopen(mailfile, "wb")) == NULL) {
		printf("554 Can't open local file\n");
		exit(1);
		}

	do_cmds(fname, custom.c_user, pass, mlfd);
	
	fclose(mlfd);
	strcpy(termbuf, "250 OK\n");	/* successful termination */
	termcode = 0;

	tcp_close();
	for(;;)
		tk_block();
	}


/* Do the necessary commands for POP2 transfer.  Start by waiting for the
 * connection to open, then send commands.  Check the
 * reply codes and act appropriately
 */

do_cmds(remfile, user, pass, mlfd)
	char	*remfile;		/* remote file */
	char	*user;			/* User name */
	char	*pass;			/* Password get from netcust later */
	FILE	*mlfd; {		/* mail file descriptor */

	int	nmsgs;			/* Number messages in folder */
	int	tdbg;			/* temp */

	while(!conn_open)
		tk_block();
	
	conn_open = 0;
	success = FALSE;		/* Init to false */

	expect(plus);			/* expect a service ready msg */
	tputs("HELO ");
	tputs(user);
	tputs(" ");

	tdbg = dbg;
	dbg = 0;			/* Make sure no echo */
	tputs(pass);
	dbg = tdbg;
	tputs("\n");
	datalen = 0L;
	expect(pound);			/* expect OK */

	if(fileflag) {
		tputcmd("FOLD",remfile);
		datalen = 0L;
		expect(pound);
	}

	nmsgs=datalen;
	if(nmsgs) {
		tputs("READ\n");	/* Want next msg */
		datalen = 0L;
		expect(equals);		/* get number chars in msg */
	}

	for(; nmsgs && datalen; nmsgs--) {
		tputs("RETR\n");	/* Ask for the msg */
		do_data(mlfd);		/* Read the msg */
		if(delflag)
			tputs("ACKD\n");	/* Delete the msg */
		else
			tputs("ACKS\n");	/* Save that msg */
		datalen = 0L;
		expect(equals);
	}
	success = TRUE;
	quitin = TRUE;
	tputs("QUIT\n");
	expect(plus);
	}

tputcmd(cmd, arg)
char	*cmd;
char	*arg;
{
	tputs(cmd);
	tputs(" ");
	tputs(arg);
	tputs("\n");
}


/* Put the data in the specified mail file from the current pop2
 * connection.  Do the appropriate netascii conversion
 */

do_data(fd)
	register FILE	*fd; {		/* mail file descriptor */
	register char	*p;		/* temp pointer */
	long i;

	/* for now, kludge a t20 mail hdr line: date,nchars;flags */
	fprintf(fd," mm-dd-yy,%d;000000000000\n",datalen);
	
	for(i = 0L, p = cirget; i < datalen; i++, p++) {
		if(p == &cirbuf[POPBUF])	/*#85029 check wraparound */
			p = cirbuf;
		if(p == cirput) { /* Have to wait for data */
			cirget = p;	/* Update ptr before sleeping */
			tm_set(RESPTIME, sm_tmo, 0, resptime);
			for(;;) {
				if(in_avail) {
					in_avail = 0;
					tm_clear(resptime);
					rsptmo = 0;
					break;
				}
				if(rsptmo)
					break;
				tk_block();
			}
			if(rsptmo) {
				printf("Data wait timeout\n");
				fclose(fd);
				xclose();
			}
		}
		if(p == cirput) {
			printf("do_data: internal error\n");
			fclose(fd);
			xclose();
		}
		/* Don't have to do netascii-fy, just store as is */
		if(putc(*p, fd) == EOF) {
			printf("do_data: file write error\n");
			fclose(fd);
			xclose();
		}
	}

	if(p == &cirbuf[POPBUF])	/*#85030 check wrap for last p++ */
		p = cirbuf;
	cirget = p;			/* Update ptr */
	if(p != cirput) {		/*#85030 Leftover data!! */
		printf("do_data: Leftover data, put = %u, get = %u\n",
				cirput,cirget);
		fclose(fd);
		xclose();
	}

	}				/* End of do_data */


/* Close connection after some error has occurred. */

xclose()
{
	unlink(mailfile);		/* delete (partial file) */
	tcp_close();
	termcode = 1;
	tm_set(RESPTIME, sm_tmo, 0, resptime);
	rsptmo = 0;
	for(;;)
		if(rsptmo) exit(1);
		else tk_block();
}

expect(code)

/* Expect a reply message with the specified code.  If the specified code
 * is received return TRUE; otherwise print the error message out on the
 * standard output and give up.  Note that the reply can be a multiline
 * message.
 *
 * Arguments:
 */

char	code;
{
	int	retcd;			/* Code from server */
	int	negresp = FALSE;	/* '-' response, assume not */
	int	lenresp = FALSE;	/* '#' response, assume not */
	char *data = cmdbuf;
	int i,errtmo;			/*#16 Add errtmo to vary timeout */
	
	for(;;) {		/* get whole reply */
		errtmo = RESPTIME;	/*#85029 long always */

		if(tgets(cmdbuf, errtmo) > 0) { /*#16 get input line */
			switch(*data) {
			case '+':
				if(code == plus) return;
				break;
			case '-':
				negresp = TRUE;
				break;
			case '=':
			case '#':
				lenresp = TRUE;
				break;
			default:
				if(cmdbuf[3] == '-') /* continuation line? */
					continue;
			}	/* End of switch */ 

			if (!negresp && *data == code) {
				for(i=0; i<30; i++)
				    if(*data >= '0' && *data <= '9') break;
				    else data++;

				if (lenresp)
					datalen = atol(data);
				return;
			}
			/* Here if negresp || retcd != code */
			if(quitin) return;	/* No big deal */
			if(dbg)
				if (negresp)
					printf("Expected: %c, got \"-\"\n",
						code);
				else
					printf("Expected: %c, got %c.\n",
						code, *data);
			strcpy(termbuf, cmdbuf); /* return the error line */
			quitin = TRUE;	/* Prevent loopin */
			tputs("QUIT\n");
			expect(plus);
			break;	/*#12 Get out of for loop */
		} else if(success) {	/* Here if not tgets(... */
				if(dbg) printf("Expect: Couldnt get input line,	but transfer successful\n");	/*#12*/
				strcpy(termbuf, "250 OK\n");
				return;	/*#12 Get out of for loop */
			}
			else {			/* Here if not success */
				if (dbg) printf("Expect: TGets failed\n");/*#14*/
				strcpy(termbuf, "451 Receive timeout\n");
				break;
			}
	}					/* End for(;;) */
	termcode = !success;			/* error return */
	xclose();
}


us_opna()

/* Called by the tcp receiver task when a connection is successfully opened.
 * Just wakes up the pop2 task and returns
 */
{
	if (dbg) printf("Open\n");
	tk_setef(tk_pop2, &conn_open);
}

		
cmd_rcv(buf, len, urg)

/* Called by the tcp receive task on receipt of a command from the net
 * Just copy the data into the circular buffer and wake up the pop2 task
 * to process the data.
 *
 * Arguments:
 */

char	*buf;				/* data buffer */
int	len;				/* data length */
int	urg;				/* urgent ptr(unused) */
{
	register char	*p, *q, *o;	/*#85029 */
	register int	i;
	
	for(p = cirput, q = buf, i = len; i > 0; i--) { /* copy the data */
		o = p++;		/*#85029 */
		if(p == &cirbuf[POPBUF])
			p = cirbuf;
		if(p == cirget) {	/*#85029 About to overflow? */
			cirput = o;	/*#85029 Yes, update for consumer */
			tk_setef(tk_pop2, &in_avail);	/*#85029 Wake him */
			tk_yield();	/*#85029 and yield cpu */
		}
		*o = *q++;
	}

	cirput = p;
	
	tk_setef(tk_pop2, &in_avail);
}


tgets(buf, tmo)

/* Wait for a full command line to be input.  As the characters come in,
 * they are copied into the circular buffer.  This routine takes them
 * out and copies them into the specified buffer for processing.  It
 * performs the netascii conversion on the fly.  Returns the number of
 * characters input, or -1 on response timeout.
 *
 * Arguments:
 */

char	*buf;				/* buffer address */
int	tmo;				/* response timeout */
{
	register char	*p, *q;		/* temp pointers */
	register int	i;		/* length counter */
	static	int	crseen = FALSE;	/* for netascii conversion */
	
	for(q = buf, i = 0;;) {
		for(p = cirget; p != cirput;) {	/*#85030 Move p++ to end */
			if(crseen) {	/* need to de-netascify */
				crseen = FALSE;
				if(*p == '\n') {
					*q++ = '\n'; /* end of line */
					*q = '\0'; /* null terminate */
					i++;
					cirget = ++p; /* clean up */
					if(p == &cirbuf[POPBUF])
						cirget = cirbuf;
					if(dbg)
						printf("%s", buf);
					return(i);
				} else {
					*q++ = '\r';
					i++;
					if(*p != '\0') {
						*q++ = *p;
						i++;
					}
				}
			} else if(*p == '\r') {
				crseen = TRUE;
			} else {
				*q++ = *p;	/* Copy the char */
				i++;
			}
			p++;		/*#85030 Do this here */
			if(p == &cirbuf[POPBUF]) /*#85030 So we can do */
				p = cirbuf;	/*#85030 this too */
		}
		
		cirget = p;		/* fix get ptr. back up */

/* Full line not in yet; wait for character input */

		tm_set(tmo, sm_tmo, 0, resptime);

		for(;;) {
			if(in_avail) {
				in_avail = 0;
				rsptmo = 0;	/*#14 In case both */
				tm_clear(resptime);
				break;
			}
			if(rsptmo) {
				rsptmo = 0;
				return(-1);	}
			tk_block();
		}
	}
}


tputs(str)

/* Send the specified string out to the net.  Do the appropriate netascii
 * conversion as it goes.
 *
 * Arguments:
 */

register char	*str;
{
 	extern	task	*TCPsend;
	event	sendef;
	
	if(dbg)
		printf("%s", str);
		
	for(; *str != '\0'; str++) {
		if(*str == '\n')
			tputc('\r');
		tputc(*str);
		if(*str == '\r')
			tputc('\0');
	}
		
	tcp_ex();			/*#19 make sure it gets sent */
	}


/* Put the specified character out to tcp.  If the output buffer is full,
 * block until reawakened by the tcp (via the us_space routine).
 */

tputc(c)
	char	c; {			/* character to output */

	while(tc_put(c)) tk_block();
	}


us_tmo() {
	printf("451 Host not responding\n");
	exit(1);
	}


sm_tmo() {
	tk_setef(tk_pop2, &rsptmo);
	}

/* Called by the tcp layer when space becomes available in the tcp output
 * buffer.  Just wakes up the pop2 task if it's asleep.
 */

bfr() {

	tk_setef(tk_pop2, &spc_avail);
	}

pr_dot() {
	if (dbg) printf(".");
	}


us_cls() {
	int	opening=0, ourclosing=0, hisclosing=0;
	
	if(success) exit(0);
	if(termbuf[0] == '\0')
		printf("450 termbuf null.\n success = %d, opening = %d,\
ourclosing = %d, hisclosing = %d\n", success, opening, ourclosing, hisclosing);
	else
		printf("%s", termbuf);
	exit(termcode ? termcode : 1);
	}

us_yld() {
	return 0;
	}

xusage() {
	printf("usage: pop2 [-dfFOLDER pPASSWD] <loc_file> <host> [dbg]\n");
	printf("\tSwitches: -d delete msgs from folder\n");
	printf("\t\t  -f use <folder> instead of default mailbox\n");
	printf("\t\t  -p read in password from here instead of netdev.sys");
	exit(1);
	}
