/* Copyright 1988 Bradley N. Davis, Darbick Instructional Software Systems */
/* See permission and disclaimer notice in file "bnd-note.h" */
#include	"bnd-note.h"
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
#include <ether.h>
#include <timer.h>
#include <int.h>
#include "wd8003.h"
#ifdef	WATCH
#include <match.h>
#endif

/* 10/3/84 - removed Init1(); changed to handle the new net structure
						<John Romkey>
   8/9/85 - rewrote to work with new NET structure.
						<John Romkey>
   3/24/86 - (re)added et_swtch routine.  Why did John remove it?
	Decreased demux task stack size back to 1400 like last version.
	Why was it raised to 5400?  There shouldn't be a problem.  Seemed
	like paranoia.
						<Drew D. Perkins>
   8/5/86 - Add new elements of net structure for bootp support.
						<Drew D. Perkins>
*/

/* This is the network configuration file for the new IP code. This file
	sets up the configuration for a single ethernet interface
	machine. Programs which want to use multiple net interfaces
	should use their own configuration file.
*/

struct tabent {
	in_name ip_addr;
	char	i_addr[6];
	int	count;
};

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

PRIVATE unsigned char stop_page;
PRIVATE unsigned wint = 0;
PRIVATE unsigned wfcs = 0;
PRIVATE unsigned wover = 0;
PRIVATE unsigned wdribble = 0;
PRIVATE unsigned wshort = 0;
PRIVATE unsigned wrcv = 0;
PRIVATE unsigned wref = 0;
PRIVATE unsigned wtoobig = 0;
PRIVATE unsigned wmissed = 0;
PRIVATE unsigned wsend = 0;
PRIVATE unsigned wunder = 0;
PRIVATE unsigned wcoll  = 0;
PRIVATE unsigned wcollsx = 0;
PRIVATE unsigned wrdy = 0;
PRIVATE unsigned wtxunknown = 0;
PRIVATE struct tabent table[MAXENT];
PRIVATE task *wneed;
PRIVATE unsigned wwpp = 0;	/* number of times awakened w/o packet to process */
PRIVATE unsigned wdrop = 0;	/* # of packets dropped */
PRIVATE unsigned wmulti = 0;	/* # of times more than one packet on queue */
PRIVATE unsigned wadsnd = 0;	/* # of packets sent */
PRIVATE unsigned wadreq = 0;	/* # of requests received */
PRIVATE unsigned wadrep = 0;	/* # of replies received */
PRIVATE unsigned wadnotme = 0;	/* # of requests not for me */
PRIVATE unsigned wadbad = 0;	/* # of bad ARP packets */
PRIVATE unsigned wadlen = 0;	/* # of bad lengths */
PRIVATE unsigned wunexpect = 0; /* # of unexpected replies */
PRIVATE in_name wadexpect = 0;	/* DDP Address of expected ARP REPLY */
PRIVATE in_name arp_req_record[64];
PRIVATE int arp_recptr = 0;
PRIVATE char w_msgid[] = "Western Digital 8003 adapter";
PRIVATE char _wme[6];		/* my ethernet address */
PRIVATE char BROADCAST[6] = { 0xff, 0xff, 0xff, 0xff, 0xff, 0xff};
PRIVATE task *wDemux;		/* ethernet packet demultiplexing task */
PRIVATE NET *w_net;		/* my net pointer */
PRIVATE char wrcvcmd;		/* receiver command byte */
PRIVATE char save_mask; 	/* receiver command on entry. */
PRIVATE unsigned wrreset = 0;
PRIVATE task *e_rtk;
PRIVATE timer *e_rtm;

#ifdef	WATCH
PUBLIC struct pkt pkts[MAXPKT];
PUBLIC int pproc = 0;
PUBLIC int prcv = 0;
PUBLIC long npackets = 0;
PUBLIC unsigned etminlen;
#endif

/* random fnctns. */
int w_init(), w_send(), w_switch(), w_stat(), w_close(), ip_ether_send();
int w_demux(), w_poke(), w_keepalive();

unsigned w_eoi;
char _net_if_name = 'W';	/* DDP - Default NETCUST = "NETCUSTW" */
int Nnet = 1;		/* The number of networks. */
NET nets[1] = { "Western Digital 8003 Ethernet",	/* interface name */
		w_init,		/* initialization routine */
		w_send,		/* raw packet send routine */
		w_switch,	/* DDP interrupt vector swap routine */
		w_close,	/* shutdown routine */
/*		ip_ether_send,	/* ip packet send routine */
		NULL,
		w_stat,		/* statistics routine */
		NULL,		/* demultiplexing task */
		NULL,		/* packet queue */
				/* first parameter...*/
#ifdef	WATCH
		ALLPACK,	/* ...promiscuous mode for netwatch */
#else
		0,		/* ...normal mode for others */
#endif
		0,		/* second parameter unused */
		1400,		/* DDP demux task stack size */
		14,		/* local net header size */
		0,		/* local net trailer size */
		0L,		/* ip address */
		0L,		/* default gateway */
		0L,		/* network broadcast address */
		0L,		/* 4.2bsd network broadcast address */
		0L,		/* subnetwork broadcast address */
		&custom,	/* our custom structure! */
		6,		/* hardware address length */
		ARETHx,		/* hardware type */
		_wme,		/* pointer to hardware address */
		NULL		/* per interface info */
	};

extern 	long	cticks;

/*  8-Aug-85 Drew D. Perkins (ddp) at Carnegie-Mellon University
	Added support for counting number of replies received, both
	expected and unexpected.
 */
/* This code handles the address resolution protocol as described by
	David Plummer in NIC RFC-826. It's split into two parts:
	etarsnd which sends a request packet and
	etarrcv which handles an incoming packet. If the packet is a request
		and is for us, reply to it. If it is a reply, update our
		table.

	The code is currently crocked to work only with internet and ethernet.
*/

/* define the table format of internet -> ethernet address mappings */
PRIVATE
wainit()
{
	int i;

	for(i=0; i<3; i++) {
		table[i].ip_addr = custom.c_ipname[i];
		wadcpy(custom.c_ether[i].e_ether, table[i].i_addr);
	}

	/* zero out the remaining cache */
	for(i=3; i<MAXENT; i++)
		table[i].ip_addr = 0;
}

/* Send an address request packet for internet address ipaddr. This routine
	doesn't wait to receive a reply. */

PRIVATE
warsnd(ipaddr)
in_name ipaddr;
{
	struct adr *padr;
	PACKET p;

	wadsnd++;
	wadexpect = ipaddr;		/* DDP */
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
	wadcpy(_wme, padr->ar_sha);
	padr->ar_spa = w_net->ip_addr;
	padr->ar_tpa = ipaddr;

	if(w_send(p, ARP, sizeof(struct adr), BROADCAST) == 0) {
#ifdef	DEBUG
		if(NDEBUG & (INFOMSG|NETERR))
			printf("WARSND: Couldn't xmit request packet.\n");
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
warrcv(p, len)
register PACKET p;
unsigned len;
{
	register struct adr *padr;
	int i;

	padr = (struct adr *)p->nb_prot;
	if(len < sizeof(struct adr)) {
#ifdef	DEBUG
		if(NDEBUG & (INFOMSG|PROTERR))
			printf("WARRCV: bad pkt len %u\n", len);
#endif
		wadlen++;
		putfree(p);
		return;
	}

	switch(padr->ar_op) {
	case ARREQ:
		wadreq++;
		if(padr->ar_hd != ARETH) {
			wadnotme++;
			putfree(p);
			return;
		}
		if(padr->ar_pro != ARIP) {
			wadnotme++;
			putfree(p);
			return;
		}
		if(padr->ar_tpa != w_net->ip_addr) {
			wadnotme++;
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
				wadcpy(padr->ar_sha, table[i].i_addr);
				break;
			}
			if(i == MAXENT-1) {
				table[i].ip_addr = padr->ar_spa;
				wadcpy(padr->ar_sha, table[i].i_addr);
				break;
			}
			if(table[i].ip_addr) continue;
			table[i].ip_addr = padr->ar_spa;
			wadcpy(padr->ar_sha, table[i].i_addr);
			break;
		}

		padr->ar_op = ARREP;
		wadcpy(padr->ar_sha, padr->ar_tha);
		wadcpy(_wme, padr->ar_sha);
		padr->ar_tpa = padr->ar_spa;
		padr->ar_spa = w_net->ip_addr;
		w_send(p, ARP, sizeof(struct adr), padr->ar_tha);
		putfree(p);
		break;

	case ARREP:
		if(padr->ar_tpa != w_net->ip_addr) {
			wadrep++;	/* DDP */
			putfree(p);
			return;
		}

		for(i=0; (i < MAXENT) && table[i].ip_addr; i++)
			if(table[i].ip_addr == padr->ar_spa) {
				wadcpy(padr->ar_sha, table[i].i_addr);
				wadrep++;	/* DDP */
				putfree(p);
				if(wneed) {
					tk_wake(wneed);
					wneed=0;
				}
				return;
			}

		if(i==MAXENT)
			i=MAXENT-1;

		table[i].ip_addr = padr->ar_spa;
		wadcpy(padr->ar_sha, table[i].i_addr);
		putfree(p);
		wadrep++;			/* DDP */
		if(wadexpect != padr->ar_spa)	/* DDP */
		    wunexpect++;		/* DDP */

		if(wneed) {
			tk_wake(wneed);
			wneed = 0;
		}
		break;

	default:
		wadbad++;
#ifdef	DEBUG
		printf("WARRCV: Unknown opcode %u.\n", padr->ar_op);
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
ip2et(ether, ip)
register char *ether;
in_name ip;
{
	int i;
	long time;

	if(ip == 0L) {
#ifdef	DEBUG
		printf("IP2ET: Passed null IP address!\n");
#endif
		return FALSE;
	}

	for(i=0; i<MAXENT; i++) {
		if(table[i].ip_addr == ip) {
			wadcpy(table[i].i_addr, ether);
#ifdef	DEBUG
			if(NDEBUG & INFOMSG)
				printf("IP2ET: Found %a table.\n", ip);
#endif
			return TRUE;
		}
	}

	if(!warsnd(ip))
		return FALSE;

	wneed = tk_cur;

	time = cticks;
	while(cticks - time < 3*18) {
		tk_yield();
		if(!wneed) {
			for(i=0; i<MAXENT; i++) {
				if(table[i].ip_addr == ip) {
					wadcpy(table[i].i_addr, ether);
#ifdef	DEBUG
					if(NDEBUG & INFOMSG)
						printf("IP2ET: ARP'd %a\n",ip);
#endif
					return TRUE;
				}
			}
		}
	}

#ifdef	DEBUG
	if(NDEBUG & (INFOMSG|PROTERR))
		printf("IP2ET: Exiting without having resolved address.\n");
#endif
	wneed = NULL;
	return FALSE;
}


wadstat(fd)
FILE *fd;
{
#ifndef NOSTATS
	fprintf(fd, "%4u ARPs sent\t%4u ARP reqs rcvd\t%4u ARP reps rcved\n",
		wadsnd, wadreq, wadrep);
	fprintf(fd, "%4u ARP reqs not for me\t%4u bad ARPs\n",
		wadnotme, wadbad);
	fprintf(fd, "%4u unexpected replies\t%4u bad lengths\n",
		wunexpect, wadlen);
#endif
}

/*
30-Mar-86 Added another outb() to et_close to reset the board completely.
					<Larry K. Raper>
 */

/* Shutdown the ethernet interface */
w_close() {
	int vec;	/* crock to avoid compiler bug */

	int_off();
	outb(IIMR, inb(IIMR) | save_mask); /* restore original mask */
	outb(WD8base + CMDR, MSK_STP + MSK_RD2); /* take 8390 off line */
	w_unpatch();
	int_on();
}


/* 7/10/84 - moved some variables definitions into et_int.c.
					<John Romkey>
   7/16/84 - changed debugging level on short packet message to only
	INFOMSG.			<John Romkey>
   6/5/85  - changed debugging levels to support separate tracing of
        network level; consolidated calls to tk_block.
					<J. H. Saltzer>
*/

/* Process an incoming ethernet packet. Upcall the appropriate protocol
	(Internet, Chaos, PUP, NS, ARP, ...). This does not check on
	my address and does not support multicast. It may in the future
	attempt to do more of the right thing with broadcast. */
w_demux()
{
	register PACKET p;
	unsigned type;
	register struct ethhdr *pet;

	while(1) {
		tk_block();
		p = (PACKET)aq_deq(w_net->n_inputq);

		if(p == 0) {
#ifdef	DEBUG
			if(NDEBUG & (NETRACE|NETERR))
				printf("wDEMUX: no pkt\n");
#endif
			wwpp++;
			continue;
		}

		if(p->nb_len < ET_MINLEN) {
#ifdef	DEBUG
			if(NDEBUG & (NETRACE|NETERR)) {
				printf("WDEMUX: pkt[%u] too small\n", p->nb_len);
				w_dump(p);
			}
#endif
			putfree(p);
			continue;
		}

#ifdef	DEBUG
		if(NDEBUG & NETRACE)
			printf("W_DEMUX: got pkt[%u]", p->nb_len);
#endif

		/* Check on what protocol this packet is and upcall it. */
		p->nb_prot = p->nb_buff+sizeof(struct ethhdr);
		pet = (struct ethhdr *)p->nb_buff;
		type = pet->e_type;
		switch(type) {
		case ET_IP:
#ifdef	DEBUG
			if(NDEBUG & NETRACE)
				printf(" type IP\n");
#endif
			indemux(p, p->nb_len-sizeof(struct ethhdr), w_net);
			break;
		case ET_ARP:
#ifdef	DEBUG
			if(NDEBUG & NETRACE)
				printf(" type ARP\n");
#endif
			warrcv(p, p->nb_len-sizeof(struct ethhdr));
			break;
		default:
#ifdef	DEBUG
			if(NDEBUG & NETRACE)
				printf(" type unknown: %04x, dropping\n",
				       bswap(type));
#endif
			wdrop++;
#ifdef	DEBUG
			if(NDEBUG & (DUMP|NETRACE)) w_dump(p);
#endif
			putfree(p);
		}

		if(w_net->n_inputq->q_head) {
#ifdef	DEBUG
			if(NDEBUG & INFOMSG)
				printf("WDEMUX: More pkts; yielding\n");
#endif
			wmulti++;
			tk_wake(tk_cur);
		}
	}
}

w_stat(fd)
FILE *fd;
{
#ifndef NOSTATS
	fd = stdout;
	fprintf(fd, "Ether Stats:\n");
	fprintf(fd, "My ethernet address: %02x.%02x.%02x.%02x.%02x.%02x\n", /* DDP */
		_wme[0]&0xff, _wme[1]&0xff, _wme[2]&0xff,
		_wme[3]&0xff, _wme[4]&0xff, _wme[5]&0xff); /* DDP */
	fprintf(fd, "%4u ints\t%4u pkts rcvd\t%4u pkts sent\t%4u ints lost\n",
		wint, wrcv, wsend, wrreset);
	fprintf(fd, "%4u underflows\t%4u colls\t%4u 16 colls\t%4u rdys\n",
		wunder, wcoll, wcollsx, wrdy);
	fprintf(fd, "%4u FCS errs\t%4u overflows\t%4u dribbles\t%4u shorts\n",
		wfcs, wover, wdribble, wshort);
	fprintf(fd, "%4u missed\t%4u unknown\n", wmissed, wtxunknown);
	fprintf(fd, "%4u refused\t%4u too big\t%4u dropped\t%4u multi\n",
		wref, wtoobig, wdrop, wmulti);

	fprintf(fd, "max q depth %u\n", w_net->n_inputq->q_max);
#endif
	wadstat(fd);		/* upcall to address resolution protocol */
#ifndef WATCH
	in_stats(fd);		/* upcall to internet */
#endif
}

#ifdef	DEBUG
w_dump(p)
register PACKET p;
{
	register char *data;
	int i;

	data = p->nb_buff;
	for(i=1; i<121; i++) {
		printf("%02x ", (*data++)&0xff);
		if(i%20 == 0) printf("\n");
		}
	puts("");
}
#endif

/* This C routine does as much of the initialization at a high level as
	it can.
*/
w_init(net, options, dummy)
NET *net;
unsigned options;
unsigned dummy;
{
#ifdef	DEBUG
	if(NDEBUG & INFOMSG)
		printf("Forking WDEMUX.\n");
#endif

	wDemux = tk_fork(tk_cur, w_demux, net->n_stksiz, "wDEMUX", net);
	if(wDemux == NULL) {
		printf("Error: %s setup failed\n", w_msgid);
		exit(1);
	}

	w_net = net;
	w_net->n_demux = wDemux;

	w_switch(1, options);		/* DDP */

	/* start up a task which periodically kicks the receiver to
		keep it alive
	*/
	e_rtm = tm_alloc();
	if(e_rtm == NULL) {
		printf("Error: %s timer setup failed\n", w_msgid);
		exit(1);
	}

	e_rtk = tk_fork(tk_cur, w_keepalive, 400, "Keepalive", 0);
	if(e_rtk == NULL) {
		printf("Error: %s keepalive setup failed\n", w_msgid);
		exit(1);
	}

	/* Now everything is initialized. The DMA channel should only be
		initialized on demand, so it's not necessary to touch it
		now.
	*/
	tk_yield();	/* Give the per net task a chance to run. */

	/* init arp */
	wainit();
}

PRIVATE w_poke()
{
	tk_wake(e_rtk);
}

PRIVATE w_keepalive()
{
	unsigned int_cnt;

	while(1) {
		tm_set(3, w_poke, NULL, e_rtm);
		tk_block();
		int_cnt = wint;
		w_ihnd();
		if (int_cnt != wint) ++wrreset;
	}
}


/*
	Routine to switch the board and interrupts on/off.
 */
PRIVATE
w_switch(state, options)
int state;
unsigned options;
{
	union {
		long ulong;
		char uc[4];
	} myaddr;
	register int i;
	int vec;
	unsigned int temp;
	unsigned char adrsum;

	if (!state) { 	/* Turn them on? */
		w_close();	/* Let i_close do the work */
		return;
	}
	/* We need to setup our ethernet address. Do this by reading the
		address from the switches.
	*/

	/* sumcheck the addr ROM */
	adrsum = 0;
	for (i= 0; i < 8; i++)
		adrsum = adrsum + inb(ADDROM + WD8base + i);
	if (adrsum != 0xff) {
		printf("Error: Checksum error in LAN address ROM\n");
		exit(1);
	}

	switch(custom.c_seletaddr) {
	case HARDWARE:
		for(i=0; i<6; i++)
			_wme[i] = inb(WADDR+i);
		break;
	case ETINTERNET:
		myaddr.ulong =	w_net->ip_addr;
		for(i=3; i != -1; i--)
			_wme[i+2] = myaddr.uc[i];
		_wme[0] = 0;
		_wme[1] = 0;
		break;
	case ETUSER:
		for(i=0; i<6; i++)
			_wme[i] = custom.c_myetaddr.e_ether[i];
		break;
#ifdef DEBUG
	default:
		printf("Invalid %s address selection option\n", w_msgid);
#endif
	}
	/* reset WD8003 board, and then enable the shared memory */
	/* use SM_base to calculate the MSK_DECOD */
	outb(WD8base+W83CREG, MSK_RESET);
	outb(WD8base+W83CREG, 0);
	outb(WD8base+W83CREG, MSK_ENASH +
		(((long)MEMADDR >> 16) >> 9 & 0x3f));

	/* initial the LAN Controller register program for page 0 */
	outb(WD8base+CMDR, MSK_PG0 + MSK_RD2);
	outb(WD8base+DCR, MSK_BMS + MSK_FT10);
	outb(WD8base+RBCR0, 0);
	outb(WD8base+RBCR1, 0);
	outb(WD8base+RCR, MSK_MON); /* disable the rxer */
	outb(WD8base+TCR, 0);	/* normal operation */
	stop_page = MAXMEM / PAGE_LEN -
		(MAX_WD8003_DATA+PAGE_LEN-1) / PAGE_LEN;
	outb(WD8base+PSTOP, stop_page);	/* init PSTOP */
	outb(WD8base+PSTART, START_PAGE); /*init PSTART to 1st page of ring*/
	outb(WD8base+BNRY, START_PAGE-1); /* init BNRY */
	outb(WD8base+ISR, -1);	/* write FF */
	outb(WD8base+IMR, 0x15); /* enable interrupt */

	/* program for page 1 */
	outb(WD8base+CMDR, MSK_PG1 + MSK_RD2);
	for (i=0; i < sizeof(_wme); i++)	/* inital physical addr */
		outb(WD8base+PAR+i, _wme[i]);
	for (i=0; i < MARsize; i++)		/* clear multicast */
		outb(WD8base+MAR+i, 0);
	outb(WD8base+CURR, START_PAGE);

	/* program for page 0 */
	outb(WD8base+CMDR, MSK_PG0 + MSK_RD2);
	outb(WD8base+CMDR, MSK_STA + MSK_RD2);	/* put 8390 on line */

#ifdef WATCH
	outb(WD8base+RCR, MSK_PRO|MSK_AB|MSK_AR);	/* accept all */
#else
	outb(WD8base+RCR, MSK_AB);	/* accept broadcast no multicast */
#endif

	int_off();		/* Disable interrupts. */
	/* patch in the new interrupt handler - rather, call the routine to
	   do this. This routine saves the old contents of the vector. */
	w_eoi = 0x60 + custom.c_intvec;
	w_patch(custom.c_intvec<<2);
	/* setup interrupts for the specified line */
	vec = (1 << custom.c_intvec);
	save_mask = inb(IIMR) & vec;
	outb(IIMR, inb(IIMR) & ~vec);
	/*turn interrupts on to try to eliminate an insidious race condition*/
	int_on();

#ifdef	DEBUG
	if(NDEBUG & INFOMSG) {
		printf("PC Ethernet address = ");
		for (i=0; i<6; i++)
			printf("%02x", _wme[i]&0xff);
		printf ("\n");
	}
#endif
}

/* This code services the ethernet interrupt. It is called by an assembly
	language routines which saves all the registers and sets up the
	data segment. */
PUBLIC
w_ihnd()
{
	unsigned int rcv, next, status;
	unsigned char start;
	char far *bufptr;
#ifdef WATCH
	register char *data = pkts[prcv].p_data;
	int i;
#else
	unsigned int tmplen, len;
	PACKET i_inp;
#endif

	if (inb(WD8base+ISR) & (MSK_PRX|MSK_RXE)) {
		rcv = inb(WD8base+RSR);
		wint++;
		if (rcv & SMK_PRX) {
			wrcv++;
			start = inb(WD8base+BNRY) + 1;
			bufptr = MAKEADDR(PAGE_LEN * start);
#ifdef WATCH
			status = *bufptr++;
			next = *bufptr++;
			if (((pproc - prcv) & PKTMASK) != 1) {
				pkts[prcv].p_len = *((int far *)bufptr)++;
				for(i=0; i<MATCH_DATA_LEN; i++)
					*data++ = *bufptr++;
				prcv = (prcv+1)&PKTMASK;
				tk_wake(wDemux);
			}
			outb(WD8base+BNRY, next-1);
#else
#ifdef DEBUG
			if (NDEBUG & DUMP) {
				printf("Receive buffer: %d\n", start);
				for(next=0; next<120; next++) {
					printf("%02x ", bufptr[next]&0xff);
					if((next+1)%20 == 0) printf("\n");
				}
			}
#endif
			status = *bufptr++;
			next = *bufptr++;
			if ((i_inp = getfree()) == NULL) {
				wref++;
				outb(WD8base+BNRY, next-1);
				return;
			}
			len = *((int far *)bufptr)++;
			/* We wrapped around, do both buffer groups */
			if (next != 0 && next < start) {
				/* -4 for status, next, and length */
				tmplen = (stop_page - start) * PAGE_LEN - 4;
				gencpy(bufptr, (char far *)i_inp->nb_buff,
					tmplen);
				bufptr = MAKEADDR(PAGE_LEN * START_PAGE);
				gencpy(bufptr,
					(char far *)&i_inp->nb_buff[tmplen],
					len - tmplen);
			} else
				gencpy(bufptr, (char far *)i_inp->nb_buff,
					len);
			outb(WD8base+BNRY, next-1);
			i_inp->nb_len = len;
			i_inp->nb_tstamp = cticks;
			q_addt(w_net->n_inputq, (q_elt)i_inp);
			tk_wake(wDemux);
#endif
		} else {
			if (rcv & SMK_CRC) wfcs++;
			if (rcv & SMK_FAE) wdribble++;
			if (rcv & SMK_FO) wover++;
			if (rcv & SMK_MPA) wmissed++;
		}
		outb(WD8base+ISR, -1);	/* clear ISR */
	}
}


/* Transmit a packet. If it's an IP packet, calls ARP to figure out the
	ethernet address
*/

PRIVATE
w_send(p, prot, len, fhost)
PACKET p;
unsigned prot;
unsigned len;
in_name fhost;
{
	register struct ethhdr *pe;
	unsigned temp;
	char far *bufptr;

	/* Set up the ethernet header. Insert our address and the address of
	   the destination and the type field in the ethernet header
	   of the packet. */
#ifdef	DEBUG
	if(NDEBUG & (INFOMSG|NETRACE))
		printf("w_SEND: %u p[%u] -> %a.\n", prot, len, fhost);
#endif
	pe = (struct ethhdr *)p->nb_buff;
	wadcpy(_wme, pe->e_src);

	/* Setup the type field and the addresses in the ethernet header. */
	switch(prot) {
	case IP:
		if((fhost == 0xffffffff) || /* Physical cable broadcast addr*/
					/* All subnet broadcast */
			(fhost == w_net->n_netbr) ||
					/* All subnet bcast (4.2bsd) */
			(fhost == w_net->n_netbr42) ||
					/* Subnet broadcast */
			(fhost == w_net->n_subnetbr)) {
			wadcpy(BROADCAST, pe->e_dst);
		} else if(ip2et(pe->e_dst, fhost) == 0) {
#ifdef	DEBUG
			if(NDEBUG & (INFOMSG|NETERR))
				printf("w_SEND: ether address unknown\n");
#endif
			return 0;
		}
		pe->e_type = ET_IP;
		break;
	case ARP:
		wadcpy((char *)fhost, pe->e_dst);
		pe->e_type = ET_ARP;
		break;
	default:
#ifdef	DEBUG
		if(NDEBUG & (INFOMSG|PROTERR|BUGHALT))
			printf("w_SEND: Unknown prot %u.\n", prot);
#endif
		return 0;
	}
	len += sizeof(struct ethhdr);
#ifdef	WATCH
	if(len < etminlen) len = etminlen;
#else
	if(len < ET_MINLEN) len = ET_MINLEN;
#endif
	/* shared memory pointer in stop_page */
	bufptr = MAKEADDR(PAGE_LEN * stop_page);
	gencpy((char far *)pe, bufptr, len);
#ifdef DEBUG
	if (NDEBUG & DUMP) {
		printf("Sending buffer:\n");
		for(temp=0; temp<120; temp++) {
			printf("%02x ", bufptr[temp]&0xff);
			if((temp+1)%20 == 0) printf("\n");
		}
	}
#endif
	outb(WD8base+TBCR0, len);
	outb(WD8base+TBCR1, len >> 8);
	outb(WD8base+TPSR, stop_page);
	wsend++;
	/* issue tx command */
	outb(WD8base+CMDR, MSK_TXP + MSK_RD2);
	/* wait till done */
	while (inb(WD8base+CMDR) & MSK_TXP) ;

	temp = inb(WD8base+TSR);
	if (temp & SMK_FU)
		wunder++;
	else if (temp & SMK_COL)
		wcoll++;
	else if (temp & SMK_ABT) {
		wcollsx++;
#ifdef DEBUG
		printf("Excessive collisions detected.  Is the ethernet cable plugged in?\n");
#endif
	} else if (!(temp & SMK_PTX))
		wtxunknown++;
	else
		wrdy++;

	return len;
}
