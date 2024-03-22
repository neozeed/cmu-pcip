/* Copyright 1988 Bradley N. Davis, Darbick Instructional Software Systems */
/* See permission and disclaimer notice in file "bnd-note.h" */
#include	"bnd-note.h"
/*  Copyright 1986 by Carnegie Mellon  */
/*  See permission and disclaimer notice in file "cmu-note.h"  */
#include	<cmu-note.h>
/*  Copyright 1984 by the Massachusetts Institute of Technology  */
/*  See permission and disclaimer notice in file "notice.h"  */

/*  8-Aug-85 Drew D. Perkins (ddp) at Carnegie-Mellon University
	Added support for counting number of replies received, both
	expected and unexpected.
 */

#include	<notice.h>

#include <types.h>
#include <task.h>
#include <q.h>
#include <netq.h>
#include <net.h>
#include <custom.h>
#include <netbuf.h>
#include <stdio.h>
#include <ether.h>
#include "i82586.h"

/* This code handles the address resolution protocol as described by
	David Plummer in NIC RFC-826. It's split into two parts:
	etarsnd which sends a request packet and
	etarrcv which handles an incoming packet. If the packet is a request
		and is for us, reply to it. If it is a reply, update our
		table.

	The code is currently crocked to work only with internet and ethernet.
*/

/* define the table format of internet -> ethernet address mappings */
#define MAXENT	16

struct tabent {
	in_name ip_addr;
	char	i_addr[6];
	int	count;
	};

static struct tabent table[MAXENT];

static task *ineed;

/* Plummer's internals. All constants are already byte-swapped. */
#define ARETH	0x100		/* ethernet hardware type */
#define ARIP	ET_IP		/* internet protocol type */
#define ARREQ	0x100		/* byte swapped request opcode */
#define ARREP	0x200		/* byte swapped reply opcode */

struct adr {
	unsigned	ar_hd;		/* hardware type */
	unsigned	ar_pro; 	/* protcol type */
	char		ar_hln; 	/* hardware addr length */
	char		ar_pln; 	/* protocol header length */
	unsigned	ar_op;		/* opcode */
	char		ar_sha[6];	/* sender hardware address */
	in_name 	ar_spa; 	/* sender protocol address */
	char		ar_tha[6];	/* target hardware address */
	in_name 	ar_tpa; 	/* target protocol address */
	};

static unsigned iadsnd = 0;	/* # of packets sent */
static unsigned iadreq = 0;	/* # of requests received */
static unsigned iadrep = 0;	/* # of replies received */
static unsigned iadnotme = 0;	/* # of requests not for me */
static unsigned iadbad = 0;	/* # of bad ARP packets */
static unsigned iadlen = 0;	/* # of bad lengths */
static unsigned iunexpect = 0; /* # of unexpected replies */
static in_name iadexpect = 0;	/* DDP Address of expected ARP REPLY */

iainit() {
	int i;

	for(i=0; i<3; i++) {
		table[i].ip_addr = custom.c_ipname[i];
		iadcpy(custom.c_ether[i].e_ether, table[i].i_addr);
		}

	/* zero out the remaining cache */
	for(i=3; i<MAXENT; i++)
		table[i].ip_addr = 0;

	}

/* Send an address request packet for internet address ipaddr. This routine
	doesn't wait to receive a reply. */

#define RECORD_LENGTH	64
in_name arp_req_record[64];
int arp_recptr = 0;

iarsnd(ipaddr)
	in_name ipaddr; {
	struct adr *padr;
	PACKET p;

	iadsnd++;
	iadexpect = ipaddr;		/* DDP */
	arp_req_record[arp_recptr] = ipaddr;
	arp_recptr = (arp_recptr + 1) & (RECORD_LENGTH-1);

	p = a_getfree();
	if(p == NULL)
		return FALSE;

	padr = (struct adr *)(p->nb_buff+sizeof(struct ethhdr));

	padr->ar_hd = ARETH;
	padr->ar_pro = ARIP;
	padr->ar_hln = 6;
	padr->ar_pln = sizeof(in_name);
	padr->ar_op  = ARREQ;
	iadcpy(_ime, padr->ar_sha);
	padr->ar_spa = i_net->ip_addr;
	padr->ar_tpa = ipaddr;

	if(i_send(p, ARP, sizeof(struct adr), iBROADCAST) == 0) {
#ifdef	DEBUG
		if(NDEBUG & (INFOMSG|NETERR))
			printf("IARSND: Couldn't xmit request packet.\n");
#endif					/* DDP */
		return FALSE;
		}

	putfree(p);
	return TRUE;
	}

/* Handle an incoming ADR packet. If it is a request and is for us, answer it.
	Otherwise, if it is a reply, log it. If not, discard it.

	***Ugh. The cache management code is *so* gross.
*/

iarrcv(p, len)
	register PACKET p;
	unsigned len; {
	register struct adr *padr;
	int i;

	padr = (struct adr *)p->nb_prot;
	if(len < sizeof(struct adr)) {
#ifdef	DEBUG
		if(NDEBUG & (INFOMSG|PROTERR))
			printf("IARRCV: bad pkt len %u\n", len);
#endif
		iadlen++;
		putfree(p);
		return;
		}

	switch(padr->ar_op) {
	case ARREQ:
		iadreq++;
		if(padr->ar_hd != ARETH) {
			iadnotme++;
			putfree(p);
			return;
			}

		if(padr->ar_pro != ARIP) {
			iadnotme++;
			putfree(p);
			return;
			}

		if(padr->ar_tpa != i_net->ip_addr) {
			iadnotme++;
			putfree(p);
			return;
			}

		/* Make an entry in our translation table for this host since
			he's obviously going to send me a packet. */
		for(i=0; i<MAXENT; i++) {
			/* update the table even if we already know the host
			   - makes it possible to change gateway interfaces
			*/
			if(table[i].ip_addr == padr->ar_spa) {
				iadcpy(padr->ar_sha, table[i].i_addr);
				break;
				}
			if(i == MAXENT-1) {
				table[i].ip_addr = padr->ar_spa;
				iadcpy(padr->ar_sha, table[i].i_addr);
				break;
				}
			if(table[i].ip_addr) continue;
			table[i].ip_addr = padr->ar_spa;
			iadcpy(padr->ar_sha, table[i].i_addr);
			break;
			}

		padr->ar_op = ARREP;
		iadcpy(padr->ar_sha, padr->ar_tha);
		iadcpy(_ime, padr->ar_sha);
		padr->ar_tpa = padr->ar_spa;
		padr->ar_spa = i_net->ip_addr;
		i_send(p, ARP, sizeof(struct adr), padr->ar_tha);
		putfree(p);
		break;

	case ARREP:
		if(padr->ar_tpa != i_net->ip_addr) {
			iadrep++;	/* DDP */
			putfree(p);
			return;
			}

		for(i=0; (i < MAXENT) && table[i].ip_addr; i++)
			if(table[i].ip_addr == padr->ar_spa) {
				iadcpy(padr->ar_sha, table[i].i_addr);
				iadrep++;	/* DDP */
				putfree(p);
				if(ineed) {
					tk_wake(ineed);
					ineed=0;
					}
				return;
				}

		if(i==MAXENT)
			i=MAXENT-1;

		table[i].ip_addr = padr->ar_spa;
		iadcpy(padr->ar_sha, table[i].i_addr);
		putfree(p);
		iadrep++;			/* DDP */
		if(iadexpect != padr->ar_spa)	/* DDP */
		    iunexpect++;		/* DDP */

		if(ineed) {
			tk_wake(ineed);
			ineed = 0;
			}
		break;

	default:
		iadbad++;
#ifdef	DEBUG
		printf("IARRCV: Unknown opcode %u.\n", padr->ar_op);
#endif
		putfree(p);
		}
	}

/* Convert an internet address into an ethernet address. Assumes that the
	internet address is for someone on our net; does no checking.
	First tries to look the internet address up in a table; if
	this fails, then it sends out a Plummer packet and waits for
	a reply with a timeout. Fills in the ethernet address of the
	foreign host. If the ethernet address couldn't be determined,
	returns a FALSE, otherwise TRUE. */

extern long cticks;

ip2et(ether, ip)
	register char *ether;
	in_name ip; {
	int i;
	long time;

	if(ip == 0L) {
#ifdef	DEBUG
		printf("IP2ET: Passed null IP address!\n");
#endif
		return FALSE;
		}

	for(i=0; (i<MAXENT)&&(table[i].ip_addr != ip)&&(table[i].ip_addr); i++)
		;

	if(i != MAXENT && table[i].ip_addr) {
		iadcpy(table[i].i_addr, ether);
		return TRUE;
		}

	if(!iarsnd(ip))
		return FALSE;

	ineed = tk_cur;

	time = cticks;
	while(cticks - time < 3*18) {
		tk_yield();
		if(!ineed) {
			for(i=0; i<MAXENT; i++) {
				if(table[i].ip_addr == ip) {
					iadcpy(table[i].i_addr, ether);
					return TRUE;
					}
				}
			}
		}

#ifdef	DEBUG
	if(NDEBUG & (INFOMSG|PROTERR))
		printf("IP2ET: Exiting without having resolved address.\n");
#endif
	ineed = NULL;
	return FALSE;
	}


iadstat(fd)
	FILE *fd; {
#ifndef NOSTATS
	fprintf(fd, "%4u ARPs sent\t%4u ARP reqs rcvd\t%4u ARP reps rcved\n",
		iadsnd, iadreq, iadrep);
	fprintf(fd, "%4u ARP reqs not for me\t%4u bad ARPs\n",
		iadnotme, iadbad);
	fprintf(fd, "%4u unexpected replies\t%4u bad lengths\n",
		iunexpect, iadlen);
#endif
	}
