/*  Copyright 1986 by Carnegie Mellon  */
/*  See permission and disclaimer notice in file "cmu-note.h"  */
#include	<cmu-note.h>
/*  Copyright 1984 by the Massachusetts Institute of Technology  */
/*  See permission and disclaimer notice in file "notice.h"  */
#include	<notice.h>

/*  12/3/84  Changed to use telnet window size from custom structure,
             and to push death timer forward on each received packet,
             and to listen for a quit request from the user.
                                   <J. H. Saltzer>

    10/14/85 Fixed arguments to tcp_init() to be in proper order.
    	     Allocate timer before call to tcp_init().
    						<Drew D. Perkins>
 */

typedef	long	time_t;			/* ugly! */

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
#include	<em.h>

#define	MAXTIME		20	/*  Death timer, in seconds, pushed forward
					on every received packet.  */
#define	GETTIME		0x2c
#define BREAKC		C_BREAK

int	dbg;
task	*ftask;
event	open_done;
event	spc_avail;

int	us_opna(), us_cls(), us_tmo(), us_space(), us_yld(), us_space();
int	displa(), no_op(); quit();
int	exitval;
timer *tm;
extern	task	*TCPsend;
extern	NET	nets[];

main(argc, argv)
	int	argc;
	char	**argv; {
	char	key;
	in_name fhost = 0x3300000a;
	
	exitval = 0;
	if(argc != 2) {
		printf("usage:\tnicname user\n");
		printf("\tor\n");
		printf("\t\tnicname -help\n");
		exit(1);
		}

	if(strcmp(argv[1], "-help") == 0) {
		printf("Help for TCP/Nicname Version %u.%u\n",
						version/10, version%10);
		printf("This program sends queries to the NIC name database.\n");
		printf("One type of query is using a person's name:\n");
		printf("For instance:\n");
		printf("\tnicname reagan\n");
		printf("would list all people in the database with \"reagan\" as one\n");
		printf("of their names.\n");
		printf("To find out who maintains a host, do\n");
		printf("\tnicname full-hostname\n");
		printf("So, to find out who maintains MIT-BORAX, this would work:\n");
		printf("\tnicname mit-borax\n");
		printf("\nFinally, for more help, just do:\n");
		printf("\tnicname help\n");
		printf("and the NIC will tell you what it thinks you can do.\n");
		exit(1);
		}

	tm = tm_alloc();		/* DDP - Begin */
	if(tm == 0) {
		exitval = 1;
		quit("Couldn't allocate timer.");
		}

	tcp_init(512, us_opna, displa, us_yld, us_cls, us_tmo,
							 no_op, us_space);
					/* DDP -End */

	if(NDEBUG & INFOMSG)
		printf("TCP inited.\n");

	printf("IBM PC %s User TCP/Nicname - bugs to pc-ip-request@mit-xx\n",
							nets[0].n_name);

	ftask = tk_cur;

	tcp_open(&fhost, TCP_WHOIS, tcp_sock(), 
                                      custom.c_telwin, custom.c_tellowwin);

	while(!open_done) tk_block();

	open_done = 0;

	tputs(argv[1]);
	tputs("\n");
	tk_yield();			/* make sure it gets out */
	tcp_close();
#ifdef notdef				/* DDP */
	tm = tm_alloc();
	if(tm == 0) {
		quit("Couldn't allocate timer.");
		}
#endif					/* DDP */

	tm_set(MAXTIME, us_tmo, 0, tm);
	for(;;)
		{
		key = h19key();
		if(key==C_BREAK | key=='q')
			quit("PC/nicname aborted at user request");
		tk_yield();
		}
	}

displa(buf, len, urg)
	register char	*buf;
	int	len;
	int	urg; {

	fwrite(buf, 1, len, stdout);
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
	char key;
	key = h19key();
	if(key==BREAKC | key=='q') quit("PC/nicname aborted at user request");
	tm_set(MAXTIME, us_tmo, 0, tm);
	return 0;
	}


us_space() {
	tk_wake(ftask);
	}
