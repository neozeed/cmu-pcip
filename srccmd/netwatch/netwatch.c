/*  Copyright 1986 by Carnegie Mellon  */
/*  See permission and disclaimer notice in file "cmu-note.h"  */
#include	<cmu-note.h>
/*  Copyright 1984, 1985 by the Massachusetts Institute of Technology  */
/*  See permission and disclaimer notice in file "notice.h"  */
#include	<notice.h>

/*
   7/1/86 - Add packet logging facility.  Packets can be examined with
	John Leong's WATCH program.
					<Drew D. Perkins>
 */

#include <stdio.h>
#include <task.h>
typedef long in_name;
#include <custom.h>
#include <em.h>
#include <attrib.h>
#include <match.h>
#include <q.h>
#include <ctype.h>
#include <colors.h>
#include "watch.h"


/* the display header is provided by the root protocol layer: the ethernet
	protocol handler or the proNET protocol handler.
*/
extern char *header;

char nw_banner[40];
char keybuf[100];

unsigned prot_mode = MD_SYMBOLIC;

char *prtype = "Type to match on [? for help]: ";
char question[] = "?";

/* patterns */
pattern pat1;
pattern pat2;

char unknown_match = FALSE;
char local_net_disp = FALSE;
int color_display = FALSE;
extern char manu_switch;

/* mode strings - human readable forms of filters */
char Watch[30] = "";
char Source[30] = "";
char Destination[30] = "";
char Type[30] = "";

/* protocol layer display flags. Protocol unparser prints more gubbish
	(like seq. numbers, checksums, etc.) if this the right one is
	TRUE
*/
int disp_network = FALSE;	/* hardware layer */
int disp_internet = FALSE;	/* internetwork (IP, Chaos, etc.) layer */
int disp_transport = FALSE;	/* transport (TCP, UDP, Chaos) layer */
int disp_app = FALSE;		/* application (TFTP, etc.) layer */

FILE *logfile = NULL;		/* DDP File to log packets to */
extern char *sys_errlist[];	/* DDP */
extern int errno;		/* DDP */

extern unsigned clear25;
extern unsigned long npackets;
extern unsigned NBUF;
extern unsigned x_pos, y_pos, pos;
extern int disp_y;
extern unsigned version;

extern int pkt_display(), scr_close(), _curse();

main() {
	char c;
	int i;

	NBUF = 6; /* use 10 if LPKT is undefined in ~/include/netbuf.h  jrd*/

	scr_init();
	Netinit(1000);

	i = _display_type();
	if(i == 3)
		color_display = TRUE;

	exit_hook(_curse);
	exit_hook(scr_close);

	if(tk_fork(tk_cur, pkt_display, 1200, "Pkt display") == NULL) {
		printf("couldn't fork packet display task\n");
		exit(1);
		}

	/* init the arrays of bytes for the patterns */

#ifdef	LENGTH_HISTO
	/* init the histogram */
	for(i=0; i<HIST_NUM_INCS; i++)
		hist_counts[i] = 0;
#endif

	clear_lines(0, 25);

	sprintf(nw_banner, "MIT PC/IP Netwatch ver %d.%d ",
						version/10, version%10);

	sym_heading();

	scroll_end();

	tminit();
	inv25();

	while(1) {
		c = h19key();
		if(c == NONE) {
			tk_yield();
			continue;
			}

		switch(c) {
		case 'E':
			scroll_start();
			pt_dump(&pat1);
			pt_dump(&pat2);
			scroll_end();
			goto pause;
		case 'n':
			clr25();
			if(prot_mode == MD_SYMBOLIC) {
				norm_heading();
				write_string("Entering normal mode", 24, 0, INVERT);
				prot_mode = MD_NORMAL;
				}
			else {
				sym_heading();
				write_string("Entering symbolic mode", 24, 0, INVERT);
				prot_mode = MD_SYMBOLIC;
				}

			/* redraw top line */
			scroll_end();

			clear25 = 3;
			break;
		case 'q':
			clr25();
			write_string("Exiting", 24, 0, INVERT);
			norm25();
			move_lines(1, 0, 24);
			set_cursor(pos = 24*80);
			x_pos = 0;
			y_pos = 24;
			scr_close();
			_curse();
			exit(0);
		case 'S':
			scroll_start();
			net_stats(stdout);
			scroll_end();
			goto pause;
		case '?':
			scroll_start();
			printf("\nCommand Summary:\n");
			printf("a\tmatch all packets\n");
			printf("c\tdisplay packet type counts\n");
			printf("d\tmatch on destination\n");
			printf("f\topen packet log file\n");
#ifdef	LENGTH_HISTO
			printf("h\tdisplay packet length histogram\n");
#endif
			printf("l\tclear screen\n");
			printf("m\ttoggle using manufacturer info in hardware addresses\n");
			printf("n\ttoggle normal and symbolic modes\n");
			printf("p\tpause\n");
			printf("q\tquit\n");
			printf("r\treset packet count\n");
			printf("s\tmatch on source\n");
			printf("t\tmatch on packet type\n");
			printf("u\tmatch only on unknown packets\n");
			printf("w\tmatch all packets coming to or from an address\n");
			printf("N,I,T,A\tshow more network, internetwork, transport or application layer info\n");
			printf("F\tclose packet log file\n");
#ifdef	LENGTH_HISTO
			printf("H\tdisplay histogram of packet lengths\n");
#endif
			printf("L\tToggle displaying local net addresses\n");
			printf("S\tprint statistics\n");
			printf("?\tprint command summary\n");
			scroll_end();
		/* drop into pause */
		case 'p':
pause:			clr25();
			pr25(0, "---paused [type any character to continue] ---");
			while(h19key() == NONE) ;
			clr25();
			break;
		case 'c':
			scroll_start();
			nameber_stats(root_layer, 0, 1);
			scroll_end();
			goto pause;
		case 'u':
			unknown_match = TRUE;
			break;
		case 'L':
			local_net_disp = !local_net_disp;
			break;
		case 'm':
			manu_switch = !manu_switch;
			break;
		case 'A':
			disp_app = !disp_app;
			break;
		case 'I':
			disp_internet = !disp_internet;
			break;
		case 'N':
			disp_network = !disp_network;
			break;
		case 'T':
			disp_transport = !disp_transport;
			break;
		case 'a': {
			clr25();

			unknown_match = FALSE;

			pt_clear(&pat1);
			pt_clear(&pat2);

			pr25(0, "Match all packets");
			clear25 = 3;
			break;
			}
		case '+':
			clr25();
			tk_stats(stdout);
			pr25(0, "Stack usage");
			clear25 = 3;
			break;
		case 't': {
			clr25();
			if(!root_layer->l_type) {
				pr25(0, "no type matching available");
				clear25 = 3;
				break;
				}

			pr25(0, prtype);
			if(rdchrs(40, strlen(prtype))) {
				clr25();
				break;
				}

			i = (*root_layer->l_type)(keybuf, 0);

			if(i == F_PAUSE)
				goto pause;

			if(i != F_ERROR)
				clr25();
			break;
			}
/* DDP - Begin additions */
		case 'f': {		
			if (logfile != NULL)
				fclose(logfile);
			clr25();
			pr25(0, "File to log packets to: ");
			if(rdchrs(40, strlen("File to log packets to: "))) {
				clr25();
				break;
				}

			logfile = fopen(keybuf, "wb");
			clr25();
			if(logfile == NULL)
				pr25(0, sys_errlist[errno]);
			else
				pr25(0, "Log file opened");
			clear25 = 3;
			break;
			}
		case 'F': {
			if (logfile == NULL)
				pr25(0, "Error: Log file not open");
			else {
				fclose(logfile);
				logfile = NULL;
				pr25(0, "Log file closed");
			}
			clear25 = 3;
			break;
			}
/* DDP - End additions */
		case 's':
		case 'd':
		case 'w':
			clr25();
			if(!root_layer->l_addr) {
				pr25(0, "no address matching available");
				clear25 = 3;
				break;
				}

			pr25(0, "Address to match on [? for help]: ");
			if(rdchrs(40, sizeof("Address to match on [? for help]: "))) {
				clr25();
				break;
				}

			i = (*root_layer->l_addr)(keybuf, 0, c - 32);

			if(i == F_PAUSE)
				goto pause;

			if(i != F_ERROR)
				clr25();
			break;
		case 'r':
			npackets = 0;
		case 'l':
			clear_lines(0, 25);
			putchar(27); putchar('H');
			printf("\n");
			scroll_end();
			inv25();
			pr25(0, "Clear screen");
			clear25 = 3;
			break;

#ifdef	LENGTH_HISTO
		case 'h':
			scroll_start();

			printf("packet length histogram\n");
			printf("length: count\n");
			for(i=0; i<HIST_NUM_INCS/2; i++)
				printf("%4d: %8U\t\t\t%4d: %8U\n", i*HIST_INCR, hist_counts[i],
					(i+HIST_NUM_INCS/2)*HIST_INCR,
					hist_counts[i+HIST_NUM_INCS/2]);

			scroll_end();
			goto pause;
		/* fancy histogram display */
		case 'H':
			fancy_histo();
			break;
#endif
		default:
			clr25();
			write_string("unknown command", 24, 0, INVERT);
			}
		}
	}

rdchrs(n, scr_x)
	int n;
	int scr_x; {
	int pos=0;
	char c;
	char foo[2];

	foo[1] = 0;

	while(1) {
		c = h19key();
		switch(c) {
		case NONE:
			continue;
		case 0177:
			if(pos) {
				write_string(" ", 24, scr_x+pos, INVERT);
				keybuf[pos--]=0;
				}
			break;
		case 033:
			return 1;
		default:
			if(c == '\n' || c == '\r') {
				keybuf[pos]=0;
				return 0;
				}
			if(pos == n-1) ring();
			else {
				keybuf[pos++] = c;
				foo[0] = c;
				write_string(foo, 24, scr_x+pos, INVERT);
				continue;
				}
			}
		}
	}

htoi(s)
	register char *s; {
	unsigned a=0; 

	while(1) {
		if(*s >= '0' && *s <= '9') a = *s++ - '0' + 16*a;
		else if(*s >= 'a' && *s <= 'f') a = *s++ - 'a' + 10 + 16*a;
		else if(*s >= 'A' && *s <= 'F') a = *s++ - 'A' + 10 + 16*a;
		else return a;
		}
	}

etparse() {
	char *next;
	char old;
	int i;

	next = keybuf;

	for(i=0; i<6; i++) {
		if(*next == 0 || *(next+1) == 0) return;
		old = *(next+2);
		*(next+2) = 0;
/*		maddr[i] = htoi(next);	*/
		*(next+2) = old;
		next++; next++;
		}
	}

nomem() {
	clr25();
	pr25(0, "Out of memory");
	clear25 = 3;
	}

scroll_start() {
	/* move cursor to line 24 and clear the line */
	pos = 23*80; y_pos = 23; x_pos = 0;
	set_cursor();
	clear_lines(23, 1);
	clr25();
	}

scroll_end() {
	int old_attrib;

	old_attrib = attrib;
	if(color_display)
		attrib = LGRAY<<4|RED;
	else
		attrib = UNDER;

	clear_lines(0, 1);

	attrib = old_attrib;

	if(color_display) {
		write_string(header, 0, 0, LGRAY<<4|RED);
		if(strlen(header) < 80 - strlen(nw_banner))
			write_string(nw_banner, 0, 80 - strlen(nw_banner),
						LGRAY<<4|BLUE);
		}
	else {
		write_string(header, 0, 0, UNDER);
		if(strlen(header) < 80 - strlen(nw_banner))
			write_string(nw_banner, 0, 80 - strlen(nw_banner),
							UNDER);
		}

	disp_y = 1;
	}

