/*  Copyright 1986 by Carnegie Mellon  */
/*  See permission and disclaimer notice in file "cmu-note.h"  */
#include	<cmu-note.h>
/*  Copyright 1984, 1985 by the Massachusetts Institute of Technology  */
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
#include <timer.h>
#include <sockets.h>

/* iprint.c
	Imagen file printing program; sends a file to an Imagen printer.

  written 10/19/84 John Romkey, MIT-LCS

  12/3/84     Modified to use telnet window size from custom structure.
                                                     <J. H. Saltzer>
  10/14/85    Modified to allocate timer before it is used!
  						     <Drew D. Perkins>
*/

#define	DOS_EOF		26	/* control Z */
#define	MAXTIME		20	/*  Death timer, in seconds, pushed
				forward on every acknowledged packet.  */


extern int wr_usr(), mst_run(), opn_usr(), cls_usr(), tmo_usr(), pr_dot();
extern int bfr();

int con_open = FALSE;	/* the output connection is open */
int con_ready = TRUE;	/* the output connection is ready for more data */

char doc_buffer[400];

int writec();

int errflag = FALSE;
int quietflag = FALSE;
int nohdrflag = FALSE;

int	exitval;
timer *tm;
extern int optind;

main(argc, argv)
	int	argc;
	char	*argv[]; {
	register char *s = doc_buffer;
	int c;

	exitval = 0;
	argv[0] = "iprint";

	while((c = getopt(argc, argv, "qn")) != EOF)
		switch(c) {
		case 'q':
			quietflag = TRUE;
			break;
		case 'n':
			nohdrflag = TRUE;
			break;
		case '?':
		default:
			errflag = TRUE;
			}

	if(errflag || optind != argc-1) {
		fprintf(stderr, "Usage: iprint [-nq] file\n");
		exit(1);
		}

	tm = tm_alloc();		/* DDP - Begin */
	if(tm == 0) {
		exitval = 1;
		quit("Couldn't allocate timer.");
		}			/* DDP -End */

	tcp_init(1200, opn_usr, wr_usr, mst_run, cls_usr, tmo_usr, pr_dot, bfr);

	if(!quietflag)
		printf("Trying...");

	tcp_open(&custom.c_printer, TCP_PRINTER, tcp_sock(),
                               custom.c_telwin, custom.c_tellowwin);

	while(!con_open) tk_yield();

#ifdef notdef			/* DDP */
	tm = tm_alloc();
	if(tm == 0) {
		exitval = 1;
		quit("Couldn't allocate timer.");
		}
#endif				/* DDP */

	tm_set(MAXTIME, tmo_usr, 0, tm);


	/* connection is open, send job header */
	sprintf(doc_buffer, "@document(language printer, file \"%s\", owner \"%s\")",
						argv[optind], custom.c_user);

	while(*s)
		writec(*s++);

	/* now send the document header */
	tcp_ex();

	/* now send the document */
	if(nohdrflag) {
		FILE *fin;

		fin = fopen(argv[optind], "ra");
		if(fin == NULL) {
			fprintf(stderr, "can't open file %s\n", argv[optind]);
			tcp_close();
			while(con_open) tk_yield();
			exit(1);
			}

		while((c = getc(fin)) != EOF)
			writec(c);

		fclose(fin);
		}
	else if(pg_format(argv[optind], writec, 0, 60, 80)) {
			tcp_close();
			while(con_open) tk_yield();
			exit(1);
			}

	if(!quietflag)
		printf("entire document sent\n");

	tcp_close();

	while(con_open) tk_yield();
	}

opn_usr() {
	con_open = TRUE;

	if(!quietflag)
		printf("Print service connected\n");
	}

bfr() {
	tm_set(MAXTIME, tmo_usr, 0, tm);
	if (h19key()=='q') quit("PC/iprint aborted by user request");
	con_ready = TRUE;
	}

mst_run() {
	return 0;
	}

wr_usr() {
	printf("write user called!\n");
	}

cls_usr() {
	con_open = FALSE;

	if(!quietflag)
		printf("print service disconnected\n");
	exit(exitval);
	}

tmo_usr() {
	quit("printer not responding");
	}

pr_dot() {
	if(!quietflag)
		putchar('.');
	}

writec(c, arg)
	char c;
	unsigned arg; {

	while(!con_ready) {
		if (h19key()=='q') quit("PC/iprint aborted at user request");
		tk_yield();
		}
	if(tc_put(c)) {
		con_ready = FALSE;
		tcp_ex();
		}
	}

quit(msg) 
        char *msg[];
	{
	exitval = 1;
	tcp_reset();	/*  Prepare to reset connection and stop output.  */
	printf("\n%s\n", msg);	/*  Explain why.  */
	tk_yield();	/*  Give the reset a chance to happen now.  */
	exit(1);	/*  Just in case--you can't get here if reset
			    worked right.  (should exit via us_cls()) */
	}
