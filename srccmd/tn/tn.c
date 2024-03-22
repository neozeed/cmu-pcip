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
#include	<ip.h>
#include	"telnet.h"

/*  Modified 12/23/83 to include upcalled "done" function in initialization
    of tftp server.  <J. H. Saltzer>
    Modified 1/2/84 to get window and low window sizes from the custom
    structure. <John Romkey>
    11/15/84 - fixed screen condition on error exit.
					<John Romkey>
    3/21/85 - Decreased initial stack size in tcp_init.  We no longer need as
    	much stack becuase the large array in rst_screen was made static
	instead of auto, saving 2000 bytes.
    					<Drew D. Perkins>
   10/30/86 - Added destination unreachable upcall.
    					<Drew D. Perkins>
*/

#define	WAITCLS		5		/* close wait time */


struct ucb	ucb;
struct	task		*TNsend;		/* telnet data sending task */
int	speed;
in_name tnhost;

extern int wr_usr(), mst_run(), opn_usr(), cls_usr(), tmo_usr(), pr_dot();
extern int du_usr();			/* DDP */
extern int bfr();
extern int tn_flash();

extern int tntftp();
extern int tntfdn();

char usage[] = "usage: telnet [-p port] host\n";

main(argc, argv)
	int	argc;
	char	*argv[]; {

	if(!main_init(argc, argv))
		exit(1);

	tel_init();
	gt_usr();
	}

int tel_exit();

main_init(argc, argv)
	int	argc;
	char	*argv[]; {
	in_name	fhost;
	unsigned sock;
	unsigned fsock = TELNETSOCK;
	char *cp;		/* DDP */
	int debsw = 0;		/* DDP */

	scr_init();
	exit_hook(tel_exit);

/* DDP - Begin changes */
	cp = argv[1];
	if((argc > 2) && (*cp++ == '-') && (*cp++ == 'd')) {
		argc--, argv++;
		for(; *cp; cp++)
			switch (*cp) {
				case 'd': debsw ^= DUMP; break;
				case 'b': debsw ^= BUGHALT; break;
				case 'm': debsw ^= INFOMSG; break;
				case 'e': debsw ^= NETERR; break;
				case 'p': debsw ^= PROTERR; break;
				case 'o': debsw ^= TMO; break;
				case 'n': debsw ^= NETRACE; break;
				case 'i': debsw ^= IPTRACE; break;
				case 't': debsw ^= TPTRACE; break;
				case 'a': debsw ^= APTRACE; break;
				default:
					printf("telnet: improper debug switch ignored.\n");
					break;
			}
	}
/* DDP - End changes */
		
	if(argc < 2 || argc > 4) {
		printf("telnet: improper arguments.\n");
		printf(usage);
		return FALSE;
		}

	if(argc == 4) {
		if(strcmp(argv[1], "-p") == 0) {
			fsock = atoi(argv[2]);
			argv++; argv++;
			argc--; argc--;
			}
		else {
			printf(usage);
			return FALSE;
			}
		}

	if(argc != 2) {
		printf(usage);
		return FALSE;
		}

	/* Initialize tasking and save our task's name. */
	NBUF = 14;

	/* need lots of stack because of terminal emulator */
#ifndef MSC				/* DDP - No longer true for MSC. */
	tcp_init(5500, opn_usr, wr_usr, mst_run, cls_usr, tmo_usr, pr_dot, bfr);
#else					/* DDP */
	tcp_init(2000, opn_usr, wr_usr, mst_run, cls_usr, tmo_usr, pr_dot, bfr); /* DDP */
#endif					/* DDP */
	tcp_duinit(du_usr);		/* DDP */

/* DDP	if(argc == 3) NDEBUG = atoi(argv[2]);
 */
	NDEBUG ^= debsw;		/* DDP */

	fhost = resolve_name(argv[1]);
	if(fhost == 0) {
		printf("Foreign host %s not known.\n", argv[1]);
		return FALSE;
		}

	if(fhost == 1) {
		printf("Name servers not responding.\n");
		return FALSE;
		}

	tnhost = fhost;

	/* setup the initial telnet screen */
	putchar(27);
	putchar('E');

	pr_banner(argv[1]);
	tnscrninit();
	tn_tminit();

	/* start the tftp server and turn it on with full buffering*/
	tfsinit(tntftp, tntfdn, FALSE);
	tfs_on();

	printf("Trying...");

	sock = tcp_sock();	/* DDP */
	tcp_open(&fhost, fsock, sock, custom.c_telwin, custom.c_tellowwin);

	tk_yield();	/* let it start */

	return(TRUE);
	}
