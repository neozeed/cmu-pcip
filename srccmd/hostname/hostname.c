/*  Copyright 1986 by Carnegie Mellon  */
/*  See permission and disclaimer notice in file "cmu-note.h"  */
#include	<cmu-note.h>
/*  Copyright 1984, 1985 by the Massachusetts Institute of Technology  */
/*  See permission and disclaimer notice in file "notice.h"  */
#include	<notice.h>

/*
    10/10/85 - changed udpname to check for packet allocation errors.
    					<Drew Perkins>
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
#include <ip.h>
#include <udp.h>
#include <timer.h>
#include <sockets.h>

/* This is the header file for the nameserver stuff that sits on UDP. */

#define DELTA		2		/* fiddle for name length */

struct nmitem {
	char nm_type;
	char nm_len;
	char nm_item[1]; };

#define	NI_NAME	1
#define NI_ADDR	2
#define NI_ERR	3

#define	INPKTSIZ	INETLEN

#define TIMEOUT 1
#define MAXTRY 2

/* Hostname finder. */

main(argc, argv)
	int argc;
	char *argv[]; {
	in_name host = 0L;

	if(argc < 2 || argc > 3)
	  {
	      printf("To resolve name using obsolete (IEN-116) protocol:\n");
	      printf("\tonetname name\n");
	      printf("or\n\tonetname name obsolete-name-server\n");
	      exit(1);
	  }
	Netinit(800);
	in_init();
	UdpInit();
	IcmpInit();
	GgpInit();
	nm_init();

	printf("Resolving host name:  %s\n", argv[1]);

	if(argc == 3){
		printf("using obsolete protocol at name server %s\n", argv[2]);
		host = resolve_name(argv[2]);
		if(host == 0L) {
			printf("Name server %s is unknown.\n", argv[2]);
			exit(1);
			}

		if(host == 1L) {
			printf("Can't resolve name server's name--\n");
			printf("Standard name servers not responding.\n");
			exit(1);
			}
	
		}
	else printf("using built-in list of obsolete-protocol nameservers.\n");
	udpsname(argv[1],argv[2],host);
}

static in_name nms[] = {
		0x0500030A,	/* BBN-A */
		0x0500010A,	/* BBN-G */
		0x420201C0,	/* BBN-F */
		0x6f00000A,	/* DCN1 */
		0x0600000A,	/* MIT-Multics */
		0x3300000A,	/* NIC */
		0x01000A80,	/* Purdue */
		0x5E00010A,	/* HI-multics */
		0x2C00000A,	/* XX */
		0x01003A12,     /* MIT-Athena */
		0x80000212	/* alto name server converter */
				};
static	char	*sname[] = {
		"BBN-A",
		"BBN-G",
		"BBN-F",
		"DCN1",
		"MIT-Multics",
		"NIC",
		"Purdue",
		"HI-Multics",
		"MIT-XX",
		"MIT-Athena",
		"MIT-SPOOLER",
		"--unasked!--"
		};

#define	NAMENUM sizeof(nms)/sizeof(long)

static int	reply[NAMENUM+1];
static UDPCONN	udps[NAMENUM];
static in_name  address;
static task	*name_task;
static unsigned nresp = 0;	/* # of responses */
static int	namenum;
static int name_rcv(), name_wake();

extern NET nets[];

#define	COMPILER_MAGIC	2

	udpsname(name, server, host)

	register char *name;
	char *server;
	in_name host; {
	timer *tm;
	PACKET p;
	register struct nmitem *pnm;
	int ntries;
	int waittime;
	int len;
	int i;

	if (host == 0L)	namenum = NAMENUM;
	   else {
		namenum = 1;
		nms[0] = host;
		sname[0] = server;
		sname[1] = "--unasked!--";
		}
	len = strlen(name) + sizeof(struct nmitem) - COMPILER_MAGIC;

	if((p = udp_alloc(len+1, 0)) == NULL) {	/* DDP - Begin */
#ifdef	DEBUG
		if(NDEBUG & INFOMSG || NDEBUG & PROTERR)
			printf("hostname: Couldn't allocate udp packet!\n");
#endif

		return NAMEUNKNOWN;
		}				/* DDP - End */

	pnm = (struct nmitem *)udp_data(udp_head(in_head(p)));

	pnm->nm_type = NI_NAME;
	pnm->nm_len = len - DELTA;
	strcpy(pnm->nm_item, name);

	name_task = tk_cur;
	address = 0L;
	waittime = TIMEOUT;
	nresp = 0;
	tm = tm_alloc();
	for(i=0; i<namenum; i++) reply[i] = 0;

    for(ntries=0; (ntries < MAXTRY) & (nresp < namenum); ntries++) {

	for(i=0; i<namenum; i++) {
	    if(reply[i] == 0) {
		udps[i] = udp_open(nms[i], UDP_NAME, udp_socket(), name_rcv);

		if(NDEBUG & INFOMSG)
			printf("NAME: req to %A\n", nms[i]);

		if((udp_write(udps[i], p, len) < 0) && (NDEBUG & PROTERR))
			printf("UDPNAME: Error while writing UDP packet.\n");

		tm_set(waittime, name_wake, 0, tm);
		tk_block();
	    }

	}
	waittime = waittime * 2;  /*  Let the slowpokes get in, too */

    }

	/* Clean up. */
	tm_clear(tm);
	tm_free(tm);
	udp_free(p);
/*	for(i=0; i<namenum; i++) udp_close(udps[i]);	*/

	for(i=0; i<namenum; i++) 
		if (reply[i] == 0)
			printf("Name server %s (%A) did not respond\n",
					sname[i], nms[i]);

	if(nresp == 0) return NAMETMO;
	else return address;
	}


static name_rcv(p, len, host)
	PACKET p;
	unsigned len;
	in_name host; {
	int	i;
	register struct nmitem *pnm;

	nresp++;

	if(NDEBUG & INFOMSG)
		printf("NAME_RCV: response from %A\n", host);

	for (i=0; i < namenum ; i++) if (host == nms[i]) break;
	reply[i] = 1;
	pnm = (struct nmitem *)udp_data(udp_head(in_head(p)));

	pnm = (struct nmitem *)((char *)pnm + pnm->nm_len + 2);

	if(pnm->nm_type == NI_ADDR) {
		address = *((in_name *)pnm->nm_item);
		printf("Name server %s (%A) responded with address %A\n",
				sname[i], host, address);
		}

	else
		printf("Name server %s (%A) couldn't resolve the name\n",
							sname[i], host);

	if(nresp == namenum) name_wake();
	udp_free(p);
	}


static name_wake() {

	if(NDEBUG & INFOMSG)
		printf("Waking name user task.\n");
	tk_wake(name_task);
	}
