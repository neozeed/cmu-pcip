/*  Copyright 1986 by Carnegie Mellon  */
/*  See permission and disclaimer notice in file "cmu-note.h"  */
#include	<cmu-note.h>

/*  COPYRIGHT 1985 by the Massachusetts Institute of Technology  */
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
#include <timer.h>
#include <sockets.h>
#include <dos/time.h>

/* send a file to a 4.2 lpr daemon to be spooled for the default
	printer. For this to work, the PC's internet address must
	be listed in /etc/hosts.lpd on the 4.2 machine doing the
	spooling.

  written 1/17/85 thru	3/13/85, John Romkey, MIT-LCS using Shawn
	Routhier's derived lpr protocol.

  modified 5/22/85 by Nancy Crowther, IBM, to allow -Pprinter parameter
	and change contents of control file

  modified 5/30/85 by Nancy Crowther, IBM, to direct files to
	local or remote PC printers if "printer" is "local",
	"prn","com1","com2","lpt1", or "lpt2"

  modified 10/7/85 by Michael Johnson, IBM, to change the default
	printer to 'default' from 'lp'.

  modified 10/7/85 by Michael Johnson, IBM, do not produce an error
	message if a server was not specified but printer is "local"

  modified 10/11/85 by Michael Johnson, IBM, enable binary files to be printed
	with the addition of the Binary flag (-o).

  modified 10/15/85 by Michael Johnson, IBM, enable the user to give up
	by typing 'q' if the the UNIX lpd doesn't answer. This is desirable
	because the user waits for 8 retries of 8 seconds each by default.

  modified 11/13/85 by Michael Johnson, IBM, enable user to specify the server
	by using an environmet variable SERVER. The default remains the
	print server internet address specified in the customization file.

  modified 10/7/85 by John Romkey, MIT-LCS, to change the default
	printer to 'default' from 'lp'.

  modified 2/25/86 to fix bad tm_reset call and clear timer before
  	tm_set.  Also fixed overlong connection	message, made
	waiting for connection close happen only if you ask for it
	(with -w), fixed server filename to change every second,
	changed "spooler" to "server" in user messages.
	  				<J. H. Saltzer>

*/

#define DOS_EOF 	26	/* control Z */
#define MAXTIME 	30	/*  Death timer, in seconds, pushed
				forward on every acknowledged packet.  */

/* this is all a little gross, but it gets the job done. */

extern int wr_usr(), mst_run(), opn_usr(), cls_usr(), tmo_usr(), pr_dot();
extern int bfr();

int con_open = FALSE;	/* the output connection is open */
int con_ready = TRUE;	/* the output connection is ready for more data */

int writec();

int errflag = FALSE;
int quietflag = FALSE;
int waitflag = FALSE;
int pflag = FALSE;
int vflag = FALSE;
int gflag = FALSE;
int oflag = FALSE;
int Sflag = FALSE;

/* communication variables between TCP send and receive sides */
int got_response;
char response;
char	response_string[256];
int	rsi;

int	exitval;
timer *tm;

extern int optind;
extern NET nets[];
extern int errno;
extern char *sys_errlist[];

main(argc, argv)
	int	argc;
	char	*argv[];
{
	char buffer[100];
	char filename[20];	/* spooler filename */
	char cmds[300];
	int  c, i;
	long time;
	FILE *fin;
	long count;
	in_name server;
	char	user[20], printer[20];
	char	*arg,	*saveserve, *new, *getenv();
	char	*defprtr = "lp", *defuser = "PC";
#ifdef	PASSWD
	char pass[9];

	printf("Password:");
	fflush(stdout);
	i = 0;
	while((c = getch()) != '\r' && c != '\n' && c != -1)
		if(i < sizeof(pass) - 1)
			pass[i++] = c;
	pass[i] = 0;
	printf("\n");
#endif

	/* have to init tcp here so we can use name servers */
	tcp_init(1200, opn_usr, wr_usr, mst_run, cls_usr, tmo_usr, pr_dot,
									bfr);

	exitval = 0;
	argv[0] = "lpr";

	/* figure out something to use for printer name */
	if ((new = getenv("PRT")) != NULL)
		strcpy(printer, new);
	else
		strcpy(printer, defprtr);

	/* figure out something to use for user name */
	if((new = getenv("USER")) != NULL)
		strcpy(user, new);
	else if (*custom.c_user)
		strcpy(user, custom.c_user);
	else strcpy(user, defuser);

	while (argc > 1 && argv[1][0] == '-')
	{
		i = 1;	/* start on argument just past - sign */
		argc--;
		arg = *++argv;

again:
		switch(arg[i]) {
		case 'q':
			quietflag = TRUE;
			break;
		case 'p':
			pflag = TRUE;
			break;
		case 'v':
			vflag = TRUE;
			break;
		case 'o':
			oflag = TRUE;
			break;
		case 'g':
			gflag = TRUE;
			break;
                case 'w':	/* user wants to wait for connection close */
			waitflag = TRUE;
			break;

		case 'S':
			Sflag = TRUE;
			if (arg[2])	/* get print server name */
			{
				saveserve = &arg[2];
				i = strlen(saveserve) + 1;
			}
			else if (argc > 1)	{
				argc--;
				saveserve = *++argv;
			}
			server = resolve_name(saveserve);

			if(server == NAMEUNKNOWN) {
				printf("host %s is unknown\n", saveserve);
				exit(1);
				}
			if(server == NAMETMO) {
				printf("name server timeout\n");
				exit(1);
				}
			break;

		case 'P':       /* specify printer name */
			if (arg[2])
			{
				strcpy(printer, &arg[2]);
				i = strlen(printer)+ 1;
			}
			else if (argc > 1)	{
				argc--;
				strcpy(printer, *++argv);
			}
			break;

		case '?':
		default:
			errflag = TRUE;
			}
		if (arg[++i]) goto again;
	}

	if(errflag || argc == 1) {
		fprintf(stderr, "Usage: lpr [-S server] [-P printer] -pgvqow file\n");
		exit(1);
		}

	arg = *++argv;
	if (strcmp(printer,"local") == 0)
	{
		if (!quietflag)
			printf("Trying local printer.\n");
		printPC(arg, quietflag, pflag, gflag, oflag, printer, server);
		exit(0);
	}


	/* if -S not specified, figure out something to use for print server name */
	if (!Sflag) {
	   if ((saveserve = getenv("SERVER")) != NULL)
		 { server = resolve_name(saveserve);
		   if(server == NAMEUNKNOWN) {
			   printf("host %s is unknown\n", saveserve);
			   exit(1);
			   }
		   if(server == NAMETMO) {
			   printf("name server timeout\n");
			   exit(1);
			   }
		 }
	   else
		   server = custom.c_printer;
	   }

	if (server == 0)
	{
		fprintf(stderr, "No print server has been specified. Add to customization or specify in command.\n");
		fprintf(stderr, "Usage: lpr [-S server] [-P printer] -pgvqo file\n");
		exit(1);
	}
	else if ( (strcmp(printer,"prn") == 0)  |
		  (strcmp(printer,"com1") == 0)  |
		  (strcmp(printer,"com2") == 0)  |
		  (strcmp(printer,"lpt1") == 0)  |
		  (strcmp(printer,"lpt2") == 0) )
	{
		if (!quietflag)
			printf("Trying DOS print server %a, device %s.\n", server, printer);
		printPC(arg, quietflag, pflag, gflag, oflag, printer, server);
		exit(0);
	}

	if (gflag)
		fprintf(stderr, "Graphics printing (-g flag) ignored on 4.3 bsd print server.\n");
	if (oflag)
		fprintf(stderr, "Binary printing (-o flag) ignored on 4.3 bsd print server.\n");
	fin = fopen(arg, "rb");
	if(fin == NULL) {
		fprintf(stderr, "can't open %s: %s\n", arg,
						sys_errlist[errno]);
		exit(1);
		}

	if(!quietflag)
		printf("Trying 4.3 bsd print server %a, device %s...", server, printer);

	tcp_open(&server, TCP_LPR, tcp_sock()%1023, custom.c_telwin,
						custom.c_tellowwin);

	while(!con_open) tk_yield();

	tm = tm_alloc();
	if(tm == 0) {
		exitval = 1;
		quit("Couldn't allocate timer.");
		}

	tm_set(MAXTIME, tmo_usr, 0, tm);


	/* connection is open, start the lprd protocol */
	sprintf(buffer, "\2%s\n", printer);

	got_response = FALSE;
	rsi = 0;	/* index into response string */

	writes(buffer);
	tcp_ex();

	/* wait for response */
	while(!got_response)
		tk_yield();

	if(response == '\1') {
		printf("error from print server, cannot accept file\n");
		goto done;
		}
	if(response != '\0') {
		exitval = 100;
		goto waitexit;
		}

	/* figure out the remote filename to use, sans first character.
	   The number field increases once every four seconds, and
	   recycles every 61 minutes. The host name is our internet
	   address in dotted decimal notation.
	*/
	time = get_dosl(GETTIME);

	sprintf(filename, "fA%03D%a",
		(((time >> 24) & 0xff) + ((time >> 16) & 0xff))*15 +
		((time >> 10) & 0x3f),	nets[0].ip_addr);

	if(NDEBUG & APTRACE) printf("server file name: %s\n", filename);

	/* send the file - requires some hackery to compute length
		because of goddamned MSDOS optional end-of-file
		characters. If there's a control-Z on the end we
		don't want to send it.
	*/
	got_response = FALSE;
	fseek(fin, 1L, 2);

	c = getc(fin);
	if(!vflag && c == DOS_EOF || c == EOF)
		count = ftell(fin)-1;
	else count = ftell(fin);

	fseek(fin, 0L, 2);

	sprintf(buffer, "\3%D d%s\n", count, filename);
	fseek(fin, 0L, 0);
	writes(buffer);
	tcp_ex();

	while(!got_response)
		tk_yield();

	if(response == '\1') {
		printf("error from server: connection messed up, try again\n");
		goto done;
		}

	if(response == '\2') {
		printf("error from server: out of storage space\n");
		goto done;
		}

	if(response != '\0') {
		printf("unknown response %d\n", response);
		goto done;
		}

	got_response = FALSE;
	while(count--) {
		c = getc(fin);
		writec(c);
		}

	writec('\0');
	tcp_ex();

	fclose(fin);

	while(!got_response)
		tk_yield();

	if(response != '\0') {
		printf("data file not properly transferred, aborting\n");
		goto done;
		}

	/* build the control file */
	sprintf(cmds, "H%a\n", nets[0].ip_addr);

	sprintf(cmds+strlen(cmds), "P%s\n", user);
	sprintf(cmds+strlen(cmds), "G%s\n", getenv("GROUP") ? getenv("GROUP") : "nobody");
#ifdef PASSWD
	sprintf(cmds+strlen(cmds), "V%s\n", pass);
#endif

	sprintf(cmds+strlen(cmds), "J%s\n", arg);

	sprintf(cmds+strlen(cmds), "C%a\n", nets[0].ip_addr);

	sprintf(cmds+strlen(cmds), "L%s\n", user);

	if(vflag)
		sprintf(cmds+strlen(cmds), "vd%s\n", filename);
	else if(pflag)
		sprintf(cmds+strlen(cmds), "T%s\npd%s\n", arg,  filename);
	else
		sprintf(cmds+strlen(cmds), "fd%s\n", filename);

	sprintf(cmds+strlen(cmds), "Ud%s\n", filename);
	sprintf(cmds+strlen(cmds), "N%s\n", arg);

	got_response = FALSE;
	sprintf(buffer, "\2%d c%s\n", strlen(cmds), filename);
	writes(buffer);
	tcp_ex();

	while(!got_response)
		tk_yield();

	if(response == '\1') {
		printf("error from server: connection messed up, try again\n");
		goto done;
		}

	if(response == '\2') {
		printf("error from server: out of storage space\n");
		goto done;
		}

	if(response != '\0') {
		printf("unknown response %d\n", response);
		goto done;
		}

	got_response = FALSE;
	writes(cmds);
	writec('\0');
	tcp_ex();

	while(!got_response)
		tk_yield();

	if(response != '\0') {
		printf("control file not properly transferred, aborting\n");
		goto done;
		}

	if(!quietflag)
		printf("entire document sent\n");

done:
	writec('\0');   /* for posterity */
	tcp_close();
	tk_yield();	/* get the connection close out in case we vanish. */
	tm_clear(tm);
	if(waitflag)
	  {
	      printf("Wait option was specified.  This program will exit\n");
	      printf("  when the server closes the connection, indicating\n");
	      printf("  that it has finished processing your file.\n");
	      printf("  Type 'q' to exit immediately.\n");
waitexit:      while(h19key() != 'q')
		tk_yield();  /*  Exit via cls_usr() */
	  }
	exit(0);
    }	/* end of main() */

opn_usr() {
	con_open = TRUE;

	if(!quietflag)
		printf("\n\tserver connection opened.\n");
	}

bfr() {
        tm_clear(tm);
	tm_set(MAXTIME, tmo_usr, 0, tm);
	if (h19key()=='q') quit("PC/lpr aborted by user request");
	con_ready = TRUE;
	}

mst_run() {
	return 0;
	}

wr_usr(s, len, x)
	char *s;
	int len;
	int x; {
	int i;

	response = *s;

	for(i=0; i<len; i++)
		response_string[rsi++] = *s++;

	got_response = TRUE;
	}

cls_usr() {
	con_open = FALSE;

	if(!quietflag)
		printf("\tserver connection closed.\n");

	if(exitval == 100) {
		response_string[rsi] = '\0';
		printf("server error: %s\n", response_string);
		}

	exit(exitval);
	}

tmo_usr() {
	quit("server not responding");
	}

pr_dot() {
	if(!quietflag)
		putchar('.');
	if (h19key()=='q') quit("PC/lpr aborted by user request");
	}

writec(c, arg)
	char c;
	unsigned arg; {

	while(!con_ready) {
		if (h19key()=='q') quit("PC/lpr aborted at user request");
		tk_yield();
		}
	if(tc_put(c)) {
		con_ready = FALSE;
		tcp_ex();
		}
	}

writes(s)
	register char *s; {

/*	printf("sending: %s\n", s);     */
	while(*s)
		writec(*s++);
	}

quit(msg)
	char *msg[];
	{
	exitval = 1;
	tcp_reset();	/*  Prepare to reset connection and stop output.  */
	printf("\n%s\n", msg);  /*  Explain why.  */
	tk_yield();	/*  Give the reset a chance to happen now.  */
	exit(1);	/*  Just in case--you can't get here if reset
			    worked right.  (should exit via cls_usr()) */
	}
