/*  Copyright 1986 by Carnegie Mellon  */
/*  See permission and disclaimer notice in file "cmu-note.h"  */
#include	<cmu-note.h>
/*  Copyright 1984 by the Massachusetts Institute of Technology  */
/*  See permission and disclaimer notice in file "notice.h"  */
#include	<notice.h>

/*  12/3/84  Changed to push death timer forward on each received
	     packet, and to listen for quit request from the user.
    12/5/84  Added call to tcp_reset when command aborts.
    6/7/85   Tampered to work with out-of-spec BSD UNIX systems.
    	     (Delayed close, extra CR on every LF.)
						<J. H. Saltzer>
    10/14/85 Fixed arguments to tcp_init() to be in proper order.
    	     Allocate timer before call to tcp_init().  Recode
	     '@' parsing to allow indirect fingers, ie. foo@bar@baz.
    						<Drew D. Perkins>
 */

/*typedef	long	time_t;	*/		/* ugly! */

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
#include	<timer.h>
#include	<sockets.h>

#define	MAXTIME		20	/*  Death timer, in seconds, pushed
				forward on every received packet.  */
int	dbg;
task	*ftask;
event	open_done;
event	spc_avail;

int	us_opna(), us_cls(), us_tmo(), us_space(), us_yld(), us_space();
int	displa(), quit(), no_op();
int	exitval;
timer *tm;
extern	task	*TCPsend;
extern	NET	nets[];

main(argc, argv)
int	argc;
char	**argv;
{
	in_name	fhost;
	char	name[128];
	register char	*p, *q;
	unsigned fsock;
	
	exitval = 0;
	if(argc < 2) {
		printf("usage: finger [user]@host\n");
		exit(1);
		}
	
/* DDP - Begin */
	tm = tm_alloc();
	if(tm == 0) {
		exitval = 1;
		quit("Couldn't allocate timer.");
		}

	tcp_init(512, us_opna, displa, us_yld, us_cls, us_tmo,
							no_op, us_space);

	for(p = argv[1], q = name; *p != NULL;)
		*q++ = *p++;

	for(; *p != '@' && p != argv[1];) {
		*--q = NULL;
		 --p;
	}

	if(*p != '@') {
/* DDP -End */
		printf("usage: finger [user]@host\n");
		exit(1);
	}

	*q++ = '\n';
	*q = NULL;
	
	p++;				/* skip over '@' */

	if((fhost = resolve_name(p)) == 0L) {
		printf("PC/finger:  Host %s not known\n", p);
		exit(1);
		}

	if(fhost == 1L) {
		printf("PC/finger:  Name servers not responding.\n");
		exit(1);
		}
	
	printf("[%s]\n", p);

	ftask = tk_cur;

	fsock = tcp_sock();

	tcp_open(&fhost, TCP_FINGER, fsock, custom.c_telwin,
						custom.c_tellowwin);

	while(!open_done) tk_block();

	open_done = 0;

	tputs(name);
	tk_yield();			/* make sure it gets out */
/*	tcp_close();*/		/* BSD UNIX can't cope with early close */
#ifdef notdef			/* DDP - allocated above */
	tm = tm_alloc();
	if(tm == 0) {
		exitval = 1;
		quit("Couldn't allocate timer.");
		}
#endif				/* DDP */

	tm_set(MAXTIME, us_tmo, 0, tm);
	for(;;)
		{
		if(h19key() =='q') quit("PC/finger aborted at user request");
		tk_yield();
		}
	}

displa(buf, len, urg)	/* Display result, adding CR's for BSD UNIX */
register char	*buf;
int	len;
int	urg;
{
    int	i;
    char	cr = '\r';
    
    for(i = 0; i < len; i++)
    {
	fwrite(&buf[i], 1, 1, stdout);
	if (buf[i] == 10) fwrite(&cr, 1, 1, stdout);
    }

}


us_tmo() {

	quit("Host not responding");
	}

no_op() {
	return;
	}

quit(msg) 
char *msg[];
{
	tcp_close();	/*  Better late than never for BSD UNIX. */
	exitval = 1;
	tcp_reset();	/*  Prepare to reset connection and stop output.  */
	printf("\n%s\n", msg);	/*  Explain why.  */
	tk_yield();	/*  Give the reset a chance to happen now.  */
	exit(1);	/*  Just in case--you can't get here if reset
			    worked right.  (should exit via us_cls()) */
}


/* Send the specified string out to the net.  Do the appropriate netascii
 * conversion as it goes.
 */

tputs(str)
	register char	*str; {
 	extern	task	*TCPsend;
	event	sendef;
	
	if(dbg)
		puts(str);
	for(; *str != '\0'; str++) {
		if(*str == '\n')
			tputc('\r');
		tputc(*str);
		if(*str == '\r')
			tputc('\0');
		}
		
	tk_setef(TCPsend, &sendef);
	}


/* Put the specified character out to tcp.  If the output buffer is full,
 * block until reawakened by the tcp(via the us_space routine).
 */

tputc(c)
	char	c; {			/* character to output */

	while(tc_put(c))
		tk_block();
	}


us_opna() {

	if(dbg)
		printf("Open\n");
	tk_setef(ftask, &open_done);
	}


us_cls() {

	if(dbg)
		printf("Closed\n");
	exit(exitval);
	}


us_yld() {
	if (h19key()=='q') quit("PC/finger aborted at user request");
	tm_set(MAXTIME, us_tmo, 0, tm);
	return 0;
	}


us_space() {
	tk_wake(ftask);
	}
