/******** SMTP  **********
* Client process for     *
* Simple Mail Transfer   *
* Protocol               *
* for the IBM PC         *
*************************/

/*  Copyright 1987, 1988 by USC Information Sciences Institute (ISI) */
/*  See permission and disclaimer notice in file "isi-note.h"  */
#include	"isi-note.h"

/*  Copyright 1984 by the Massachusetts Institute of Technology  */
/*  See permission and disclaimer notice in file "notice.h"  */
#include	<notice.h>

/*#2 4/16/84 Change do_data to do netascii newline xformations.  <DMC>
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
 *
 *#84362 Get mail-from field out of mail file instead of cmd line
 *
 *#85028 Init cirput,cirget to cirbuf!!!  Also replace "short timeout" msg
 *#85035 Init termcode early in main
 *
 *Added some #includes so it'll compile under Microsoft C 3.0. - koji
 *
 */

/* smtp.c
 * 
 * User mode smtp.  Takes a source and a recipient and tries to
 * deliver the mail to each.  Writes the returned error codes out to the
 * standard output for the use of its caller.  Note that this routine uses
 * the small tcp and hence must run under tasking.
 *
 * Calling sequence:
 * 	smtp <mail file> <dest-host> <dbg>
 */

typedef	long	time_t;			/* ugly! */

#ifdef MSC	/* koji -begin */
#include	<stdio.h>
#include	<types.h>
#include	<netq.h>
#include	<task.h>
#endif		/* koji -end */
#include	<ip.h>
#include	<timer.h>
#include	<custom.h>
#include	"cmds.h"

#define	SMTPSOCK	25		/* smtp server socket */
#define	WINDOW		1000		/* seems reasonable */
#define	RESPTIME	30		/*#14 maximum response timeout */
#define	BUFSIZ		512

char	cmdbuf[BUFSIZ];			/* command buffer */
char	mpath[BUFSIZ];			/*#84362 recipient buffer */
int	buflen;				/* size of string in cmd buffer */
char	cirbuf[BUFSIZ];			/* circular buffer for cmd input */
char	*cirput;			/* put ptr for circular buf */
char	*cirget;			/* get ptr for circular buf */
char	termbuf[BUFSIZ];		/* termination line */
char	*smtpaddr;			/* Global holder for rmt host argv */
int	termcode;			/* program termination code */
int	dbg;				/* debugging flag */
int	success;			/* successful transfer flag */
task	*tk_smtp;			/* the smtp user task */
timer	*resptime;			/* current response timer */
event	conn_open;			/* connection open event */
event	in_avail;			/* input available event */
event	spc_avail;			/* output buffer space available */
event	rsptmo;				/* response timeout event */

extern	int	cmd_rcv(), sm_tmo();
extern	int	us_opna(), us_cls(), us_tmo(), bfr(), us_yld(), pr_dot();


/* Insure that the mail file is readable, then open a tcp connection to
 * the server smtp at the destination site.  If successful, send the
 * mail by calling the routine do_cmds().
 */

main(argc, argv)
	int	argc;
	char	**argv; {
	FILE	*mlfd;			/* mail file desc */
	in_name	fhost;			/* foreign host address */
	int	flag;			/* debug flags */

	if(argc < 3 || argc > 4) {
		printf("501 usage: smtp <file> <dest-host> [dbg]\n");
		exit(1);
		}
	
	if(argc == 4) {
		flag = atoi(argv[3]);
		dbg = flag & 1;
		printf("Debug mode: %d\n",dbg);
		}
	
	if((mlfd = fopen(argv[1], "ra")) == NULL) {
		printf("554 mail file unreadable\n");
		exit(1);
		}

	termcode = 1;			/*#85035 Assume failure */
	tcp_init(512, us_opna, cmd_rcv, us_yld, us_cls, us_tmo, pr_dot, bfr);
	
	smtpaddr = argv[2];
	if((fhost = resolve_name(smtpaddr)) == 0L) {
		printf("550 Never heard of host %s\n", smtpaddr);
		exit(1);
		}
	
	
	resptime = tm_alloc();		/*#16 Do this once only, here */
	cirput = cirget = cirbuf;	/*#85028 Init these, too */
	tk_smtp = tk_cur;
	
	tcp_open(&fhost, SMTPSOCK, tcp_sock(), WINDOW, WINDOW/2);
	
	do_cmds(mlfd);
	
	fclose(mlfd);
	strcpy(termbuf, "250 OK\n");	/* successful termination */
	termcode = 0;
	
	tcp_close();
	close_wait(RESPTIME);
	}


/* Do the necessary commands for a smtp transfer.  Start by waiting for the
 * connection to open, then send HELO, MAIL, RCPT, and DATA.  Check the
 * reply codes and give up if needed.
 */

do_cmds(mlfd)
	FILE	*mlfd; {			/* mail file descriptor */

	while(!conn_open)
		tk_block();
	
	conn_open = 0;

	expect(220);			/* expect a service ready msg */
	tputs("HELO ");
	tputs(custom.c_user);		/* Use name from here */
	tputs("\n");
	expect(250);			/* expect an OK */
	tputs("MAIL FROM:<");
	mparse_path(mlfd);		/*#84362 parse single path  */
	tputs(">\n");
	expect(250);			/* expect OK */
	tputs("RCPT TO:<");
	mparse_path(mlfd);		/*#20 Try for multi rcpts */
	tputs(">\n");
	expect(250);			/* expect OK */
	tputs("DATA\n");
	expect(354);
	do_data(mlfd);
	expect(250);
	success = TRUE;
	tputs("QUIT\n");
	expect(221);
	}


/*#20 Break the string pointed to by mpath into individual paths, as
 *	seperated by commas.  Generate a seperate RCPT command for each
 *	path, and call parse_path to parse each one.
 *#21 Replace mpath string with the mail FILE ptr.  The recipients will
 *	now be in the mail file, seperated from the message by a ^L.
 *#84362 Allow rcpts to be seperated by line breaks as well as commas
 *#85008 Previous allowed either/or, now allow both
 */

mparse_path(mfile)
	register FILE	*mfile; {	/*#20 ptr to mail file */
	register char	*p, *q;		/*#20 temp ptrs */
	int pendrcpt = FALSE;		/*#84362 flag to keep rcpts distinct */
	char c;				/*#85008 tmp char */

	while( (p = fgets(mpath, 511, mfile)) && *p != '\f') {
	  if (*p == '\n') continue;		/*#84362 ignore blank lines */
	  if (pendrcpt) {		/*#84362 if have pending rcpt,  */
		tputs(">\n");		/*#84362 Finish it */
		expect(250);		/*#84362 Get reply */
		tputs("RCPT TO:<");	/*#84362 Start next one */
	  }				/*#84362 end if(pendrcpt)  */
	  for (q = mpath; *p != '\0'; p++) {	/*#20 Scan whole string */
		if (*p == ',') {	/*#20 If we hit a comma */
			*p = '\0';	/*#20 Change it to null */
			for (c = *++p; c == ' ' || c == '\t';)
				c = *++p; /*#85008 skip any spaces after , */
			if (c == '\n') break;	/*#85008 let eol code do it */
			if (!(parse_path(q)))	/*#20 Find path */
				return (FALSE);	/*#20 No path here */
			q = p;		/*#20 Pick up after comma */
			tputs(">\n");	/*#20 Finish this RCPT */
			expect(250);	/*#20 Get reply */
			tputs("RCPT TO:<");	/*#20 Start next one */
		}				/*#20 end if(*p==',')  */
		if (*p == '\n')
			*p = '\0';
	  }
	  if (!(parse_path(q)))
	  	return (FALSE);
	  else
	  	pendrcpt = TRUE;
	}
	return (TRUE);
	}


/* Send the data from the specified mail file out on the current smtp
 * connection.  Do the appropriate netascii conversion and starting '.'
 * padding.  Send the <CRLF>.<CRLF> at completion.
 */

do_data(fd)
	register FILE	*fd; {		/* mail file descriptor */
	register char	c;		/* current character */
 	extern	task	*TCPsend;
	event	sendef;

	while((c = getc(fd)) != EOF) {	/*#2 Do netascii xformations */
		if(c == '\n')
			tputc('\r');
		tputc(c);
		if(c == '\r')
			tputc('\0');
	}

	tputc('\r');
	tputc('\n');
	tputc('.');
	tputc('\r');
	tputc('\n');
	
	tcp_ex();			/*#19 make sure it gets sent */
	}


expect(code)

/* Expect a reply message with the specified code.  If the specified code
 * is received return TRUE; otherwise print the error message out on the
 * standard output and give up.  Note that the reply can be a multiline
 * message.
 *
 * Arguments:
 */

int	code;
{
	int	retcd;
	char *data = cmdbuf;
	int i,errtmo;			/*#16 Add errtmo to vary timeout */
	
	for(;;) {		/* get whole reply */
		errtmo = (dbg) ? 4*RESPTIME : RESPTIME;	/*#16 longer if dbg */
		if(tgets(cmdbuf, errtmo) > 0) { /*#16 get input line */
			if(cmdbuf[3] == '-') /* continuation line? */
				continue;

			for(i=0; i<30; i++)
				if(*data >= '0' && *data <= '9') break;
				else data++;

			retcd = atoi(data);
/*			sscanf(cmdbuf, "%d", &retcd);  no, last line */
			if(retcd == code)	/* If equal, */
				return;		/*  just return */
			i = code - code % 100;	/*  else check if */
						/*  in right range */
			if ( retcd >= i && retcd <= i+99)
				return;		/* It is */
			
			if(dbg)
			 printf("Expected: %u, got %u.\n",code, retcd);
			strcpy(termbuf, cmdbuf); /* return the error line */
			tputs("QUIT\n");
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
	tcp_close();
	close_wait(RESPTIME);
}


us_opna()

/* Called by the tcp receiver task when a connection is successfully opened.
 * Just wakes up the smtp task and returns
 */
{
	tk_setef(tk_smtp, &conn_open);
}

		
cmd_rcv(buf, len, urg)

/* Called by the tcp receive task on receipt of a command from the net
 * Just copy the data into the circular buffer and wake up the smtp task
 * to process the data.
 *
 * Arguments:
 */

char	*buf;				/* data buffer */
int	len;				/* data length */
int	urg;				/* urgent ptr(unused) */
{
	register char	*p, *q;
	register int	i;
	
	for(p = cirput, q = buf, i = len; i > 0; i--) { /* copy the data */
		if(p == &cirbuf[BUFSIZ])
			p = cirbuf;
		*p++ = *q++;
	}

	cirput = p;
	
	tk_setef(tk_smtp, &in_avail);
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
		for(p = cirget; p != cirput; p++) {
			if(p == &cirbuf[BUFSIZ])
				p = cirbuf;
			if(crseen) {	/* need to de-netascify */
				crseen = FALSE;
				if(*p == '\n') {
					*q++ = '\n'; /* end of line */
					*q = '\0'; /* null terminate */
					i++;
					cirget = ++p; /* clean up */
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
				*q++ = *p;
				i++;
			}
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
	tk_setef(tk_smtp, &rsptmo);
	}

/* Called by the tcp layer when space becomes available in the tcp output
 * buffer.  Just wakes up the smtp task if it's asleep.
 */

bfr() {

	tk_setef(tk_smtp, &spc_avail);
	}

pr_dot() {
	printf(".");
	}


us_cls() {
	int	opening=0, ourclosing=0, hisclosing=0;
	
	if(termbuf[0] == '\0')
		printf("450 termbuf null.\n success = %d, opening = %d,\
ourclosing = %d, hisclosing = %d\n", success, opening, ourclosing, hisclosing);
	else
		printf("%s", termbuf);
	exit(termcode);
	}

us_yld() {
	return 0;
	}

close_wait(tmo)
	int tmo; {

	rsptmo = 0;
	tm_set(tmo, sm_tmo, 0, resptime);
	while(!rsptmo)
		tk_block();		/* don't come back */
	printf("Timed out waiting for close\n");
	exit(termcode);
	}
