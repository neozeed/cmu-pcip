/*  Copyright 1986 by Carnegie Mellon  */
/*  See permission and disclaimer notice in file "cmu-note.h"  */
#include	<cmu-note.h>
/*  Copyright 1984 by the Massachusetts Institute of Technology  */
/*  See permission and disclaimer notice in file "notice.h"  */
#include	"proteon-notice.h"

#include <task.h>
#include <q.h>
#include <netq.h>
#include <net.h>
#include <custom.h>
#include <netbuf.h>
#include <stdio.h>
#include "pronet.h"


/* This code handles the address resolution protocol as described by
	David Plummer in NIC RFC-826. It's split into two parts:
	etarsnd which sends a request packet and
	etarrcv which handles an incoming packet. If the packet is a request
		and is for us, reply to it. If it is a reply, update our
		table.

	The code is currently crocked to work only with internet and proNET.
*/

/* define the table format of internet -> proNET address mappings */
#define	MAXENT	16

struct tabent {
	in_name		ip_addr;
	char		pr_addr;
	int		count;
	};

static struct tabent table[MAXENT];

static task *prneed;

/* Plummer's internals. All constants are already byte-swapped. */
#define	ARV2	0xFE00		/* proNET hardware type */
#define	ARIP	0x100		/* internet protocol type */
#define	ARREQ	0x100		/* byte swapped request opcode */
#define	ARREP	0x200		/* byte swapped reply opcode */

struct adr {
	unsigned	ar_hd;		/* hardware type */
	unsigned	ar_pro;		/* protcol type */
	char		ar_hln;		/* hardware addr length */
	char		ar_pln;		/* protocol header length */
	unsigned	ar_op;		/* opcode */
	char		ar_sha; 	/* sender hardware address */
	in_name		ar_spa;		/* sender protocol address */
	char		ar_tha; 	/* target hardware address */
	in_name		ar_tpa;		/* target protocol address */
	};

static unsigned pradsnd = 0;	/* # of packets sent */
static unsigned pradreq = 0;	/* # of requests received */
static unsigned pradrep = 0;	/* # of replies received */
static unsigned pradnotme = 0;	/* # of requests not for me */
static unsigned pradbad = 0;	/* # of bad ARP packets */
static unsigned pradlen = 0;	/* # of bad lengths */
static unsigned prunexpect = 0;	/* # of unexpected replies */
static in_name pradexpect = 0;	/* Address of expected ARP REPLY */

prainit() {
	int i;

	/* zero out the cache */
	for(i=0; i<MAXENT; i++)
		table[i].ip_addr = 0;

	}

/* Send an address request packet for internet address ipaddr. This routine
	doesn't wait to receive a reply. */

prarsnd(ipaddr)
	in_name ipaddr; {
	struct adr *padr;
	PACKET p;

	pradsnd++;
	pradexpect = ipaddr;

	p = a_getfree();
	if(p == NULL)
		return FALSE;

	padr = (struct adr *)(p->nb_buff+sizeof(struct pr_hdr));

	padr->ar_hd = ARV2;
	padr->ar_pro = ARIP;
	padr->ar_hln = 1;
	padr->ar_pln = sizeof(in_name);
	padr->ar_op  = ARREQ;
	padr->ar_sha =_prme;
	padr->ar_spa = pr_net->ip_addr;
	padr->ar_tpa = ipaddr;

	if(pr_send(p, ARP, sizeof(struct adr), prBROADCAST) == 0) {
#ifdef	DEBUG
		if(NDEBUG & (INFOMSG|NETERR))
			printf("PRARSND: Couldn't xmit request packet.\n");
		return FALSE;
#endif
		}

	putfree(p);
	return TRUE;
	}

/* Handle an incoming ARP packet. If it is a request and is for us, answer it.
	Otherwise, if it is a reply, log it. If not, discard it */

pr_arrcv(p, len)
	register PACKET p;
	unsigned len; {
	register struct adr *padr;
	int i;

	padr = (struct adr *)p->nb_prot;
	if(len < sizeof(struct adr)) {
#ifdef	DEBUG
		if(NDEBUG & (INFOMSG | PROTERR))
			printf("PR_ARRCV: bad pkt len %u.\n", len);
#endif
		pradlen++;
		putfree(p);
		return;
		}

	switch(padr->ar_op) {
	case ARREQ:
#ifdef DEBUG
		if(NDEBUG & INFOMSG) {
		    printf("PR_ARRCV: Received a ARP request packet from %d\n",
		    	padr->ar_sha & 0xff);
		    printf("packet contains - \n");
		    printf(" ar_hd = 0x%04x\tar_pro = 0x%04x\n",padr->ar_hd,
		    	padr->ar_pro);
		    printf(" ar_hln = %d\tar_pln = %d\tar_op = 0x%04x\n",
		    	padr->ar_hln & 0xff, padr->ar_pln & 0xff, padr->ar_op);
		    printf(" ar_sha = %d\tar_spa = %a\n",padr->ar_sha & 0xff,
		    	padr->ar_spa);
		    printf(" ar_tha = %d\tar_tpa = %a\n",padr->ar_tha & 0xff,
		    	padr->ar_tpa);
		    }
#endif
		pradreq++;
		if(padr->ar_hd != ARV2) {
			pradnotme++;
			putfree(p);
			return;
			}

		if(padr->ar_pro != ARIP) {
			pradnotme++;
			putfree(p);
			return;
			}

		if(padr->ar_tpa != pr_net->ip_addr) {
			pradnotme++;
			putfree(p);
			return;
			}

		/* Make an entry in our translation table for this host since
			he's obviously going to send me a packet. */
		for(i=0; i<MAXENT; i++) {
			/* update the table even if we already know the host
			   - makes it easier to change gateways
			*/
			if(table[i].ip_addr == padr->ar_spa) {
				table[i].pr_addr = padr->ar_sha;
				break;
				}
			if(i == MAXENT-1) {
				table[i].ip_addr = padr->ar_spa;
				table[i].pr_addr = padr->ar_sha;
				break;
				}
			if(table[i].ip_addr) continue;
			table[i].ip_addr = padr->ar_spa;
			table[i].pr_addr = padr->ar_sha;
			break;
			}

		padr->ar_op = ARREP;
		padr->ar_tha = padr->ar_sha;
		padr->ar_sha = _prme;
		padr->ar_tpa = padr->ar_spa;
		padr->ar_spa = pr_net->ip_addr;
		pr_send(p, ARP, sizeof(struct adr), padr->ar_tha);
		putfree(p);
		break;

	case ARREP:
#ifdef DEBUG
		if(NDEBUG & INFOMSG) {
		    printf("PR_ARRCV: Received an ARP reply from %d.\n",
		    	padr->ar_sha & 0xff);
		    printf("packet contains - \n");
		    printf(" ar_hd = 0x%04x\tar_pro = 0x%04x\n",padr->ar_hd,
		    	padr->ar_pro);
		    printf(" ar_hln = %d\tar_pln = %d\tar_op = 0x%04x\n",
		    	padr->ar_hln & 0xff, padr->ar_pln & 0xff,padr->ar_op);
		    printf(" ar_sha = %d\tar_spa = %a\n",padr->ar_sha & 0xff,
		    	padr->ar_spa);
		    printf(" ar_tha = %d\tar_tpa = %a\n",padr->ar_tha & 0xff,
		    	padr->ar_tpa);
		    }
#endif
		pradrep++;
		if(padr->ar_tpa != pr_net->ip_addr) {
			putfree(p);
			return;
			}

		for(i=0; (i < MAXENT) && (table[i].ip_addr); i++)
		       if(table[i].ip_addr == padr->ar_spa) {
			        table[i].pr_addr = padr->ar_sha;
				putfree(p);
				if(prneed) {
					tk_wake(prneed);
					prneed=0;
					}
				return;
				}

		if(i==MAXENT)
			i=MAXENT-1;

		table[i].ip_addr = padr->ar_spa;
		table[i].pr_addr = padr->ar_sha;
		putfree(p);
		if(pradexpect != padr->ar_spa)
		    prunexpect++;

		if(prneed) {
			tk_wake(prneed);
			prneed = 0;
			}
		break;

	default:
		pradbad++;
#ifdef	DEBUG
		printf("PR_ARRCV: Unknown opcode %u.\n", padr->ar_op);
#endif
		putfree(p);
		}
	}

/* Convert an internet address into an proNET address. Assumes that the
	internet address is for someone on our net; does no checking.
	First tries to look the internet address up in a table; if
	this fails, then it sends out a Plummer packet and waits for
	a reply with a timeout. Fills in the proNET address of the
	foreign host. If the proNET address couldn't be determined,
	returns a FALSE, otehrwise TRUE. */

extern long cticks;

ip2pr(pr, ip)
	register char *pr;
	in_name ip; {
	int i;
	long time;

	if(ip == 0L) {
#ifdef	DEBUG
		printf("IP2PR: Passed null IP address!\n");
#endif
		return FALSE;
		}

	for(i=0; (i<MAXENT)&&(table[i].ip_addr != ip)&&(table[i].ip_addr); i++)
		;

	if((i != MAXENT) && (table[i].ip_addr)) {
		*pr = table[i].pr_addr;
		return TRUE;
		}

	if(!prarsnd(ip))
		return FALSE;

	prneed = tk_cur;

	time = cticks;
	while(cticks - time < 3*18) {
		tk_yield();
		if(!prneed) {
			for(i=0; i<MAXENT; i++) {
				if(table[i].ip_addr == ip) {
					*pr = table[i].pr_addr;
					return TRUE;
					}
				}
			}
		}

#ifdef	DEBUG
	if(NDEBUG & (INFOMSG | PROTERR))
		printf("IP2PR: Exiting without having resolved address.\n");
#endif
	prneed = NULL;
	return FALSE;
	}


pradstat(fd)
	FILE *fd; {

	fprintf(fd, "%u ARPs sent\t%u ARP reqs rcvd\t%u ARP reps rcved\n",
						pradsnd, pradreq, pradrep);
	fprintf(fd, "%u ARP reqs not for me\t%u bad ARPs\n",
						pradnotme, pradbad);
	fprintf(fd, "%u unexpected replies\t%u bad lengths\n",
							prunexpect, pradlen);
	}

