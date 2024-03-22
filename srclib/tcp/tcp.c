/*  Copyright 1986, 1987 by Carnegie Mellon  */
/*  See permission and disclaimer notice in file "cmu-note.h"  */
#include	<cmu-note.h>
/*  Copyright 1984, 1985, 1986 by the Massachusetts Institute of Technology  */
/*  See permission and disclaimer notice in file "notice.h"  */
#include	<notice.h>


/* DCtcp.c */


/* Translation of Dave Clark's Alto TCP into C.
 *
 * This file contains a bare-bones minimal TCP, suitable only for user Telnet
 * and other protocols which do not need to transmit much data.  Its primary
 * weakness is that it does not pay any attention to the window advertised
 * by the receiver; it assumes that it will never be sending much data and
 * hence will never run out of receive window.
 *
 * This TCP requires a tasking package.  In addition to a receive task
 * provided by the network package, TCP creates two tasks:
 * TCPsend, which handles all data transmission, and TCPprocess, which
 * processes the incoming data from the circular buffer. The data is placed
 * in the circular buffer by tcp_rcv() which is upcalled from internet on an
 * arriving packet.
 *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * 12/83 - Tinkered to use different retransmit time on initial request
 * and later  retransmissions; to supply trace info for line 25; to
 * postpone acks only if there has been one recently; and to use tick timer. 
 * 1/84 - Trace info extended to include retry counts.
 *						<J. H. Saltzer>
 *
 * 3/21/84 - corrected several places in the code where last_offset and
 * first_offset were referenced as last_off and first_off.
 *						<John Romkey>
 * 5/29/84 - changed tcp_init() to pass the stksiz argument to netinit().
 * 						<John Romkey>
 * 7/23/84 - made the "unsent data acked" condition dump more information
 *	and attempt to fix up the problem.	<John Romkey>
 * 8/13/84 - removed support for tcp_restore().
 *						<John Romkey>
 * 10/2/84 - changed free[] to be static to avoid a name conflict with
 *	the new i/o library.			<John Romkey>
 *
 * 12/4/84 - added tcp_reset() entry for use by commands that get tired
 * of waiting for results.  3/20/85 - added resend of ACK when other end
 * resends old data (protocol error, was causing broken connections).
 * 5/26/85 - fixed bug in identifying old data so that UNIX keepalives
 * get acknowledged and ACK's get resent when they should.  5/30/85 -
 * removed bug that caused most ACK's to be sent twice.  6/4/85 - fixed
 * close sequence to ignore duplicate FIN's.  (Eliminates complaints
 * about unexpected states and also extraneous resets when the extra FIN's
 * got assigned sequence numbers.)  6/6/85 - Added maxsegsize option.
 * 7/8/85 - changed checksum test to accept 0000 as if it were FFFF, because
 * the TCP spec calls for one's complement but ambiguously doesn't mention
 * which zero to use; both turn up in other implementations.  11/6/85 - FIN
 * acknowledgment in FINSENT and FINACKED states corrected to bump sequence
 * number to account for the FIN.  11/11/85 - fixed to block the sending of
 * data after FIN when other end initiates connection close.  Also added
 * reset of socket numbers after close, to prevent use of dead connection.
 *						<J. H. Saltzer>
 * 11/5/85 - incorporated new function twrite() by Jacob Rekhter
 * 	of IBM/ACIS.
 *		  				<Drew D. Perkins>
 * 11/12/85 - added check for valid sequence numbers by retrofitting code
 *	from new tcp.  Set TCPSEND_STACK back to 2000 from 5500, as in
 *	previous version.  Otherwise tn3270 won't work.
 *		  				<Drew D. Perkins>
 * 11/18/85 - added header dump and option of packet dump on protocol errors.
 * 12/16/85 - removed test for unprocessed receive data in tcp_send that
 * produced lockup when a single incoming packet drives the window all the
 * way from low-window to completely closed.
 * 12/21/85  Added code to handle ESTAB case when receiving FIN (can arise
 * if user asks to close during SYNSENT state.)
 * 3/4/86  reversed flag in SYN receive processing that caused ACK of the
 * SYN to be repeated repeatedly; fixed checksum bug in close-reset sequence
 * that caused our resets to be ignored.
 *						<J. H. Saltzer>
 * 3/27/85 - modify twrite to take a pointer to "far" data as part
 *      of the tn3270 data space reduction.	<Charlie C. Kim>
 * 3/29/85 - Changed name of twrite call to twrite_far, and made new routine
 *	called twrite which just converts it's argument to a far ptr and
 *	call twrite_far.
 *		  				<Drew D. Perkins>
 * 4/22/86  added check and honoring of foreign window.
 *						<J. H. Saltzer>
 * 7/22/86 - Use cticks to generate a pseudo-random Initial Sequence Number.
 *	Don't send a reset in response to a packet with no SYN flag set if
 *	it is a valid ACK while we are waiting for a SYN ACK.  Reversed
 *	order of SYN and ACK number tests.
 *		  				<Drew D. Perkins>
 * 10/30/86 - Add destination unreachable upcall support.  An upcall handler
 *	is install by calling tcp_duinit(handler).
 *		  				<Drew D. Perkins>
 * 11/12/86  retry timeout now set whenever there is unacked data.
 *						<J. H. Saltzer>
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
#include <tcp.h>
#include <timer.h>

#define ACKDALLY	15		/* wait between ACKs (ticks) */
#define	INITRT		8		/* retry time--initial request */
#define	CONNRT		2		/* retry time--after connected*/
#define MAXBUF		500		/* maximum output buffer size */
#define	ICPTMO		5		/* initial conn. timeout */
/*
DDP - These stack sizes are too large, atleast for tn3270. Looking at
	actual stack growth, it appears that the original numbers were
	overly pessimistic.  If mysterious things happen, raising them
	would be a good thing to try
 */
#ifndef MSC				/* DDP */
#define TCPSEND_STACK	5500		/* stack size for send process */
#define TCPPROC_STACK	5800		/* stack size for tcp process */
#else					/* DDP */
#define TCPSEND_STACK	2000		/* DDP - stack size for send process*/
#define TCPPROC_STACK	2000		/* DDP - stack size for tcp process */
#endif					/* DDP */
#define	TCPWINDOW	1000		/* normal advertised window */
#define	TCPLOWIND	500		/* low water mark on window */
#define TOTALCHUNKS	20		/* max # of unassembled segments */
#define DISPOSELIMIT	120		/* upcall only this between yields */
#define BUFSIZE		0x0800		/* 2048 byte buffer  */
#define BUFMASK		0x07FF		/* Modulo BUFSIZE */
#define TCP_SEGSIZE_OPT	511		/* Limit other end to PC buffer size*/

int	conn_state;		/* connection state */
int	blk_inpt;		/* prevents new data after close req */
PACKET	opbi;			/* ptr to output packet buffer */
timer	*tcptm;			/* The tcp timer */
timer	*tmack;			/* ack timer */

struct	tcp	*otp;		/* ptr to output pkt tcp hdr */
char	*odp;			/* ptr to start of output pkt data */
struct	tcp_option *optp;	/* ptr to option position in header */
struct	tcpph	*ophp;		/* tcp psuedo hdr for cksum calc (output) */
struct	tcpph	*iphp;		/* tcp psuedo hdr for cksum calc (input) */

unsigned	tcp_src_port;	/* our port number */
unsigned	tcp_dst_port;	/* their port number */
in_name		tcp_fhost;	/* the other end's ip address */
unsigned	tcp_window;	/* our maximum window value */
unsigned	tcp_low_window;	/* open window when this much is left */
seq_t		tcp_init_seq;	/* DDP Our initial sequence number */

unsigned	tcppsnt;		/* number of packets sent */
unsigned	tcpprcv;		/* number of packets received */
unsigned	tcpbsnt;		/* number of bytes sent */
unsigned	tcpbrcv;		/* number of bytes received */
unsigned	tcprack;		/* # of bytes received and acked */
unsigned	tcpsock;		/* # of packets not for my socket */
unsigned	tcpresend;		/* # of resent packets */
unsigned	tcpnodata;		/* # empty packets */
unsigned	tcprercv;		/* # of old packets received */
unsigned	tcpbadck;		/* # of pkts rcvd w/ bad checksums.*/
unsigned	ign_win;		/* # pkts rcvd over our window. */
unsigned	frn_win;		/* Size of foreign window. */
long	ack_time;			/* When next ACK should be sent.  */
int	odlen;				/* bytes of data in output pkt */
int	sndlen = 0;			/* bytes actually sent */
int	dally_time;			/* time to delay ack (ticks) */
int	retry_time;			/* retransmission timeout (ticks)*/
int	resend;				/* retries since last we were happy */
int	taken;				/* pointer to last cbuf byte taken. */

/*  Because upcall may be delayed in order to empty out incoming packet
 *  buffers, incoming data is not considered to be added to the stream,
 *  and thus not ACKed, until after it is upcalled.  */
int	avail;	    /*  amount of data available for upcall, less one octet */
int	start_offset; /*  first byte in packet is this far from stream end */
int	end_offset;   /*  last byte in packet is this far from stream end */
int	fin_offset;   /*  fin is this far from stream end, if present */
int	fin_rcvd;	/*  TRUE if the fin bit has ever been ON.  */
int	numchunks;	/*  Years of study with Dave Clark are required */
int	maxchunks;	/*  to understand the meaning of these variables */
int	lastchunk;			/*  ..  */
int	ceilchunk;			/*  ..  */
int	first[TOTALCHUNKS];		/*  ..  */
int	last[TOTALCHUNKS];		/*  ..  */
static int	free[TOTALCHUNKS];	/*  ..  */
char	circbuf [BUFSIZE];		/*  ..  */

IPCONN	tcpfd;			/* connection ID used when calling Internet */
/*in_name	lochost;*/			/* local host address */
task	*TCPsend;		/* tcp sending task */
task	*TCPprocess;		/* TCP data processing task */

int	(*tc_ofcn)();		/* user function called on open */
int	(*tc_dispose)();	/* user function to receive data */
int	(*tc_yield)();		/* Predicate set when user must run.*/
int	(*tc_cfcn)();		/* user function called on close */
int	(*tc_tfcn)();		/* user function called on icp tmo */
int	(*tc_rfcn)();		/* user function called on icp resend */
int	(*tc_buff)();		/* user function to upcall when output buffer
					space is available */
int	(*tc_dufcn)() = NULL;	/* user function called on dest. unreachable*/

int	tmhnd(), tcp_send(), tcp_rcv(), tcp_process(), tcp_ack();

extern unsigned cticks;			/*  our clock  */


/* tcp_init():  This routine is called to initialize the TCP.
 * It starts up the tasking system, initiates the timer, TCPsend,
 * and TCPrcv tasks, and sets up the pointers to the up-callable
 * user routines for open, received data, close and timeout.
 * It does not attempt to open the connection; that function
 * is performed by tcp_open().   When it returns, the caller
 * is running as the first task, on the primary process stack.
 */

tcp_init(stksiz, ofcn, infcn, yldfcn, cfcn, tmofcn, rsdfcn, buff)
	int	stksiz;			/* user task stack size (bytes) */
	int	(*ofcn)();		/* user fcn to call on open done */
	int	(*infcn)();		/* user fcn to process input data */
	int	(*yldfcn)();		/* predicate set if user needs to run*/
	int	(*cfcn)();		/* user fcn to call on close done */
	int	(*tmofcn)();		/* user fcn to call on icp timeout */
	int	(*rsdfcn)();		/* user icp resend function */
	int	(*buff)(); {		/* user output buffer room upcall */

	tc_ofcn = ofcn;			/* save user fcn addresses */
	tc_dispose = infcn;
	tc_yield = yldfcn;
	tc_cfcn = cfcn;
	tc_tfcn = tmofcn;
	tc_rfcn = rsdfcn;
	tc_buff = buff;

	conn_state = CLOSED;		/* initially, conn. closed */
	resend = 0;			/* no timeouts yet. */
	retry_time = INITRT*TPS;

	Netinit(stksiz);
	in_init();				/* initialize internet*/
	IcmpInit();
	GgpInit();
	UdpInit();
	nm_init();

	tcptm = tm_alloc();
	if(tcptm == 0) {
		printf("TCP: Couldn't allocate timer.\n");
		exit(1);
		}

	tmack = tm_alloc();
	if(tmack == 0) {
		printf("TCP: Couldn't allocate ack timer.\n");
		exit(1);
		}

	TCPsend = tk_fork(tk_cur, tcp_send, TCPSEND_STACK, "TCPsend");
	if(TCPsend == NULL) {
		fprintf(stderr, "TCP send fork failed\n");
		exit(1);
		}

	TCPprocess = tk_fork(tk_cur, tcp_process, TCPPROC_STACK,
		"TCPprocess");
	if(TCPprocess == NULL) {
		fprintf(stderr, "TCP process fork failed\n");
		exit(1);
		}

	/* let the other tasks get started */
	tk_yield();
#ifdef DEBUG
	if(NDEBUG & TPTRACE) printf("TCP initialized\n");
#endif
	}


/* DDP - dest. unreachable upcall */
tcp_duinit(dufcn)
int	(*dufcn)();
{
	tc_dufcn = dufcn;
}


/* tcp_rcv():  This routine forms the body of the TCP data receiver task.
 * It is upcalled once, by IP, on the arrival of each TCP packet, using the
 * stack and task of the network driver.
 * The processing of each packet is divided into three phases:
 *	1) Processing acknowledgments.  This involves "shifting" the data
 *	   in the output packet to account for acknowledged data.
 *	2) Processing state information: syn's, fin's, urgents, etc.
 *	3) Processing the received data.  This is done by moving it
 *	   into a circular reassembly buffer and looking to see how
 *	   much contiguous data has been appended to the stream.
 *	   Upcall to the user is done later, by the TCPprocess task.
 */

tcp_rcv(inpkt, len, fhost)
	PACKET	inpkt;		/* the packet itself */
	int	len;		/* length of packet */
	in_name fhost; {	/* source of packet */
	register struct tcp *itp;	/* input pkt tcp hdr */
	register char *idp;		/* input pkt data ptr */
	int	needed_acking;	/* number of outstanding bytes */
	int	acked;		/* # outstanding bytes acked by this packet */
	int	data_acked;	/* # outstanding bytes acked by this packet */
	int	prev_rcvd;	/* number of previously ACKed octets
				   (including. fin) in current packet*/
	int	idlen;		/* length of arriving data in bytes */
	int	i;		/* temporary, for counting */
	int	limit;		/* temporary, for loop limit */
	unsigned tempsum;	/* temp variable for a checksum */
	long unsigned inseq, outack, mywin, winend, inend; /* DDP */
	int	reject;			/* DDP */

	tcpprcv++;			/* another packet received */

#ifdef DEBUG
	if(NDEBUG & TPTRACE)
	    {
	      printf("\nTCP: pkt[%u] from %a; ", len, fhost);
	      tcp_disp_hdr(inpkt);
	      if(NDEBUG & DUMP) tcp_dump(inpkt,len);
	    }
#endif

	/*  find tcp header and start and length of data */
	itp = (struct tcp *)in_data(in_head(inpkt));
	idp = (char *)itp + (itp->tc_thl << 2);
	idlen = len - (itp->tc_thl << 2);

	/* If the user wishes to send data, give up the processor. */
	if((*tc_yield)()) tk_yield();

	/* compute incoming tcp checksum */
	if(idlen >= 0) *(idp +idlen)= 0;
/*	iphp->tp_len = idlen + (itp->tc_thl << 2); DDP - Why recompute len? */
	iphp->tp_len = len;			/* DDP */
	iphp->tp_len = swab(iphp->tp_len);
	iphp->tp_dst = in_mymach(fhost);
	iphp->tp_src = fhost;
	tempsum = itp->tc_cksum;
	itp->tc_cksum = cksum(iphp, sizeof(struct tcpph) >> 1, 0);
	itp->tc_cksum = ~cksum(itp,((idlen + 1) >> 1) + (itp->tc_thl << 1),0);
	if(itp->tc_cksum != tempsum)
	  {   /*  allow for the case where TCP spec is ambiguous */
	      if(!((itp->tc_cksum == 0) && (tempsum == 0xFFFF)))
		{  /*  it's really wrong, so discard packet  */
#ifdef DEBUG
		if(NDEBUG & (NETERR|PROTERR))
		  {
		  printf("TCP: Bad xsum %4x. xsum of data is %4x\n",
						 itp->tc_cksum, tempsum);
	          }
#endif
		tcpbadck ++;
		in_free(inpkt);
		return;
		}
	  }

	tcp_swab(itp);

	if(itp->tc_dstp != tcp_src_port || itp->tc_srcp != tcp_dst_port) {
#ifdef DEBUG
		if(NDEBUG & PROTERR)
		  {
		  printf("\nTCP: pkt for/from prt %u/%u",
					itp->tc_dstp, itp->tc_srcp);
		  if(NDEBUG & TPTRACE) printf("\n");
		  else 
		    {
		    printf(" from %a; ", fhost);
		    tcp_disp_hdr(inpkt);
		    if(NDEBUG & DUMP) tcp_dump(inpkt,len);
		    printf("last pkt[%u] was to %a; ",
		       sndlen + sizeof(struct tcp), tcp_fhost);
		    tcp_swab(otp);
		    tcp_disp_hdr(opbi);
		    tcp_swab(otp);
		    }
		  }
#endif
		if(!itp->tc_rst) {	/* DDP - Is it a reset? */
			tc_clrs(inpkt, fhost);
			tcpsock++;
		}			/* DDP */
		in_free(inpkt);
		return;
		}

	if(itp->tc_rst) {	/* other guy's resetting me */
		cleanup("foreign reset");
		tcp_src_port = 0;
		tcp_dst_port = 0;
		conn_state = CLOSED;
		(*tc_cfcn)();
		in_free(inpkt);
		return;
		}

/* incoming packet processing is dependent on the state of the connection */

	if(NOT_YET(conn_state, ESTAB)) {
/* Ack must be for our initial sequence number - namely, 1 */
/*		if(itp->tc_ack != 1) {			   DDP */
		if(itp->tc_ack != tcp_init_seq + 1) {	/* DDP */
			printf("\nThe foreign host is trying to open ");
			printf("a connection\nto us at the same time as we ");
			printf("are to it. Perhaps you should quit.\n\n");
			tc_clrs(inpkt, fhost);	/* DDP */
			in_free(inpkt);
			return;
			}

/* opening the connection; incoming packet must have syn */
		if(!itp->tc_syn) {
#ifdef DEBUG
			if(NDEBUG & (PROTERR|TPTRACE)) {
			  printf("\nThe foreign host knows of a previous ");
			  printf("connection.\nWe will ignore it.\n");
			}
#endif
/*			tc_clrs(inpkt, fhost);	/* DDP */
			in_free(inpkt);
			return;
			}

/* Connection successfully opened.  Tell the user. */

		otp->tc_syn = 0;
		otp->tc_thl  = sizeof(struct tcp) >> 2;	/* clear option */
		otp->tc_rst = 0;
		otp->tc_fack = 1;
		otp->tc_psh = 1;
/*		otp->tc_seq = 1;		   DDP */
		otp->tc_seq = tcp_init_seq + 1;	/* DDP */
		itp->tc_seq ++;	/* syn's take sequence no. space */
		otp->tc_ack = itp->tc_seq;
		tcpbrcv = 1;
		frn_win = itp->tc_win;
		conn_state = ESTAB;
		retry_time = CONNRT*TPS;
		(*tc_ofcn)();
		tcp_ack();
		}

/* DDP - Begin changes */
/* Check if the incoming sequence number is valid */
    /* SECOND TEST IF THE PACKET IS OUT OF SEQUENCE */
    /* this follows TCP spec closely except for the variable names: */
    /* inseq  = seg.seq */
    /* outack = rcv.nxt */
    /* mywin  = rcv.win */

    /* this TCP does not take responsibility for data until the user sees   */
    /* it.  Thus, we shouldn't send ack/update packets if the packet is in  */
    /* the tcp storage range:  0---old data---1---tcp storage---2---new---3 */
    /* because doing so would frustrate the foreign TCP (the ack value is   */
    /* 1).  The next few lines consider the storage window as old data, and */
    /* avoid sending extra ACKs at the last minute (read below)    */
    inseq  = itp->tc_seq;
    outack = otp->tc_ack + avail + 1;     /* compute storage window (2)*/
    mywin  = otp->tc_win;
    reject = FALSE;
    winend = mywin + outack;
    inend = inseq;
    if (idlen + itp->tc_fin == 0) {    /* text-less packet */
	if(mywin > 0) {
	    if (inseq < outack || inseq >= winend) {
		tcpnodata++;
		reject = 1;
	    }
	}
	else {
	    if (inseq != outack) {
		tcpnodata++;
		reject = 2;
	    }
	}
    }
    else {    /* packet has a text */
	/* compute byte no of last byte */
	inend = inseq - 1L + (long) idlen + (long) itp->tc_fin;
	if (mywin > 0) {
	    if (inend < outack) {	/* all below window? */
		tcprercv++;
		reject = 3;
	    }
	    if(inseq >= winend) {	/* all above window? */
		ign_win++;
		reject = 3;
	    }
	}
	else reject = 4;
    }

	if(reject) {
#ifdef DEBUG
		if(NDEBUG & (INFOMSG)) {
			printf("tcp: Packet rejected for reason %d\n",reject);
			printf("tcp: My window = %U-%U.  Packet = %U-%U\n",
				outack, winend, inseq, inend);
		}
#endif
		tcp_ack();	/* other end may have lost our ack, resend. */
		in_free(inpkt);
		return;
	}

    /* FOURTH CHECK the SYN BIT */
    if (itp->tc_syn && itp->tc_ack != tcp_init_seq + 1) {
#ifdef DEBUG
	printf("TCP: closed: SYN in packet\n");
#endif
	cleanup("aborted\n");
	tcp_src_port = 0;
	tcp_dst_port = 0;
	conn_state = CLOSED;
	(*tc_cfcn)();		/* call the user close routine */
	tc_clrs(inpkt, fhost);
	in_free(inpkt);
	return;
    }
/* DDP - End changes */

/* Update things based on incoming ack value */

	if(itp->tc_fack) {
		acked = (int)(itp->tc_ack - otp->tc_seq);
		needed_acking = odlen + (int)otp->tc_fin;
		data_acked = ((acked == needed_acking) ? (acked - (int)otp->tc_fin) : acked);

/* Ack for unsent data prompts us to resynchronize. */

		if(acked > needed_acking) {
#ifdef DEBUG
			printf("TCP_RCV: unsent data acked;");
			printf("our Seq = %U, their Ack = %U\n",
						otp->tc_seq, itp->tc_ack);
			printf("\ttrying to fix\n");
#endif
			otp->tc_seq = itp->tc_ack;
			tk_wake(TCPsend);
			in_free(inpkt);
			return;
			}

		if(acked > 0) {
/* So now we have some free output buffer space. Upcall	the client. */
			if(odlen >= MAXBUF) (*tc_buff)();

/* Account for ack of our urgent data by updating the urgent pointer */
			if(otp->tc_furg) {
				otp->tc_urg = otp->tc_urg - acked;
				if(otp->tc_urg <= 0) {
					otp->tc_urg = 0;
					otp->tc_furg = 0;
					}
				}

/* See if all our outgoing data is now acked.  If so, turn off resend timer */

			if(acked == needed_acking) {
				resend = 0;
				tm_clear(tcptm);
				if(otp->tc_fin) {
					otp->tc_fin = 0;
					switch(conn_state) {
					case ESTAB:
					case FINSENT:
						conn_state = FINACKED;
						break;
					case SIMUL:
						conn_state = TIMEWAIT;
						break;
					case R_AND_S:
						in_free(inpkt);
						tcp_src_port = 0;
						tcp_dst_port = 0;
						conn_state = CLOSED;
						(*tc_cfcn)();
						return;
					default:
						printf(
					       "tcp_rcv: strange state: %5d",
						       conn_state);
					}
				}
			}

/* Otherwise, shift the output data in the packet to account for ack */
			odlen -= data_acked;
			shift(odp+data_acked, odp, odlen);
			odp[odlen] = 0;

/* update the sequence number */
			otp->tc_seq += acked;
		}
	}

/* Update the window.  */

	frn_win = itp->tc_win;

/* Now process the received data */

/* Check if the incoming data is over the top of our window */

	if((itp->tc_seq + idlen) > (otp->tc_ack + otp->tc_win)) {
		ign_win++;
		/* ignore excess data */
		idlen -= (itp->tc_seq + idlen - (otp->tc_ack + otp->tc_win));
	}

/* Calculate the number of previously received sequence numbers in the current packet */

	start_offset = -(prev_rcvd = otp->tc_ack - itp->tc_seq);
	end_offset = start_offset + idlen - 1;
	if(end_offset >= sizeof(circbuf))
		end_offset = sizeof(circbuf) - 1;
	if(itp->tc_fin == 1) {
		fin_rcvd = TRUE;
		fin_offset = end_offset;
	}

	if((end_offset <= avail) && (idlen > 0))
	    {
		tcprercv++;	/* If all incoming octets have been seen, */
#ifdef DEBUG
		if(NDEBUG & TPTRACE)
			printf("\nTCP: discarding pkt, no new data\n");
#endif
		tcp_ack();	/* other end may have lost our ack, resend. */
		if (fin_rcvd == TRUE) tk_wake(TCPprocess);
		in_free(inpkt);
		return;
	    }
	if(idlen == 0) tcpnodata++;

/*  Move the data into the circular reassembly buffer and, where possible,
    reassemble.  (Warning--hairy section.) */

	for(i = max(0, prev_rcvd); i < idlen; i++)
		circbuf[(taken + 1 + start_offset + i) & BUFMASK] = *(idp+i);

	if(numchunks == 0) {
		if(start_offset <= avail+1)	/* DDC CHANGE */
			avail = end_offset;
		else {
			first[0] = start_offset;
			last[0]  = end_offset;
			free[0]  = FALSE;
			numchunks = 1;
			maxchunks = 0;
			lastchunk = 0;
			}
		}
	else {
		if((lastchunk > -1) && (last[lastchunk] + 1 == start_offset))
			last[lastchunk] = end_offset;
		else {
			for(i = 0; i <= maxchunks; i++) {
				if(free[i]) continue;
				if(last[i]+1 < start_offset) continue;
				if(first[i] > end_offset + 1) continue;
				end_offset = max(end_offset, last[i]);
				start_offset = min(start_offset,first[i]);
				free[i] = TRUE;
				numchunks--;
				if(numchunks == 0) {
					maxchunks = -1;
					break;
					}
				}

			if(free[lastchunk]) lastchunk = -1;
			if(start_offset <= 0) avail = end_offset;
			else {
				limit = min(maxchunks+1, TOTALCHUNKS-1);
				for(i=0; i<=limit; i++) {
					if(!free[i]) continue;
					free [i] = FALSE;
					first[i] = start_offset;
					last [i] = end_offset;
					numchunks++;
					if(i == maxchunks + 1) maxchunks = i;
					if(i > ceilchunk) ceilchunk = i;

					if(lastchunk == -1)
						lastchunk = i;
					break;
					}
				}
			}
		}

/* If there's data to process, wake up the data processing task. */

	if(avail >= 0 || fin_offset == -1) tk_wake(TCPprocess);
	in_free(inpkt);
	return;
	}

	
/* DDP - Handle a destination unreachable upcall. */
tcp_durcv(pip, host)
struct ip *pip;
in_name host;
{
	register struct tcp *itp;

	itp = (struct tcp *)in_data(pip);

/* Swap first 8 bytes of header */
	itp->tc_srcp = swab(itp->tc_srcp);
	itp->tc_dstp = swab(itp->tc_dstp);
	itp->tc_seq = lswap(itp->tc_seq);

#ifdef	DEBUG
	if(NDEBUG & TPTRACE)
		printf("TCP: destination unreachable on %d to %a:%d\n",
		       itp->tc_srcp, host, itp->tc_dstp);
#endif

	if(itp->tc_dstp != tcp_dst_port ||
	   itp->tc_srcp != tcp_src_port ||
	   host != tcp_fhost) {
		return;
	}

	cleanup("destination unreachable");
	tcp_src_port = 0;
	tcp_dst_port = 0;
	conn_state = CLOSED;
	if(tc_dufcn)
		(*tc_dufcn)();
	return;
}


/* Process some received data for tcp. This is the task counterpart of
	tcp_rcv. */

tcp_process() {
	register int i;		/*  Temporary for counting  */
	register int nmoved;	/*  Number of bytes actually disposed on this call */

	/* forget about the initial wakeup */
	tk_block();

	while(1) {

	if((avail >= 0) || (fin_offset == -1)) {
		if(in_more(tcpfd)) {
			tk_yield();
			continue;
			}
		nmoved = min(avail, DISPOSELIMIT) + 1;
/*quick speedup*/
if (taken + nmoved  > BUFSIZ)
  {
		for(i=0; i < nmoved; i++)
			(*tc_dispose)(circbuf+((taken+1+i)&BUFMASK), 1, 0);
 }
else			(*tc_dispose)(circbuf+((taken+1)&BUFMASK), nmoved, 0);

		for(i = maxchunks; i >=0; i--) {
			if(free[i]) {
				if(i == maxchunks) maxchunks--;
				continue;
				}
			first[i] -= nmoved;
			last[i]  -= nmoved;
			}

		if(fin_rcvd) fin_offset -= nmoved;
		taken = (taken + nmoved) & BUFMASK;
		avail -= nmoved;
		if(avail != -1) tk_wake(tk_cur);

		if(fin_rcvd) {
			if(fin_offset == -1) {
		/* foreign close request prompts connection state change */
				switch(conn_state) {
				case ESTAB:
					conn_state = FINRCVD;
					otp->tc_fin = 1;
					blk_inpt = 1;
					otp->tc_ack++;
					conn_state = R_AND_S;
					break;
				case R_AND_S:
					break;
				case FINSENT:
					otp->tc_ack++;
					conn_state = SIMUL;
					break;
				case FINACKED:
					otp->tc_ack++;
					conn_state = TIMEWAIT;
					break;
				default:
					printf("TCP: rcvd FIN in state %5d\n",
								conn_state);
				}

				tcpbrcv ++;
				tk_wake(TCPsend);
				}
			}
		otp->tc_ack += nmoved;
		tcpbrcv     += nmoved;
		otp->tc_win -= nmoved;

		/*  Decide whether to do ACK now or later. . . */
		tm_clear(tmack);
		dally_time = ack_time - cticks;
		if(
		   (otp->tc_win < tcp_low_window) ||
		   (avail == -1 && !in_more(tcpfd)) ||
		   (dally_time <= 0)
		    )
		  {
		      tk_wake(TCPsend);
		  }
		else
		  {
		      tm_tset(dally_time, tcp_ack, 0, tmack);
		  }
		}
		tk_block();
		}
    }


tcp_ack() {
	tm_clear(tmack);
	tk_wake(TCPsend);
	}

/* Just shift the data in a buffer, moving len bytes from from to to. */

shift(from, to, len)
	register char *to;		/* destination */
	register char *from;		/* source */
	int len; {			/* length in bytes */

	if(len < 0) {
		printf("tcp: shift: bad arg--len < 0");
		return;
		}

	while(len--)
		*to++ = *from++;
	}



/* Swap the bytes in a tcp packet. */

tcp_swab(pkt)
	register struct tcp *pkt; {	/* ptr to tcp packet to be swapped */

	pkt->tc_srcp = swab(pkt->tc_srcp);
	pkt->tc_dstp = swab(pkt->tc_dstp);
	pkt->tc_seq  = lswap(pkt->tc_seq);
	pkt->tc_ack  = lswap(pkt->tc_ack);
	pkt->tc_win  = swab(pkt->tc_win);
	pkt->tc_urg  = swab(pkt->tc_urg);
	}



/* This routine forms the main body of the TCP data sending task.  This task
 * is awakened for one of two reasons: someone has data to send, or a resend
 * timeout has occurred and a retransmission is called for.
 * This routine in either case finishes filling in the header of the
 * output packet, and calls in_write() to send it to the net.
 */

tcp_send() {

	for(;;) {
	tk_block();			/* wait to be awakened */

	tm_clear(tcptm);
	if(conn_state == CLOSED)	/* if no connection open, punt */
		continue;

	/* If this is not a timeout, clear the resend timer */


	if(resend) {			/* if the resend timer has gone off */
		tcpresend++;
		if(resend >= 8) {
			if(NOT_YET(conn_state, ESTAB))
			  (*tc_tfcn)(); /* tell client, then resend anyway */
			else {		/* put timeout code here */
				}
			}
		else if(NOT_YET(conn_state, ESTAB))
				(*tc_rfcn)();
		}

	/* make sure to avoid silly window syndrome */

	if(otp->tc_win < tcp_low_window) otp->tc_win = tcp_window;

	tcp_swab(otp);			/* swap the bytes */

	/* Calculate the number of bytes to send, keeping in mind the
		foreign host's window. If the connection isn't yet open,
		send just the max segsize option. */

	sndlen = NOT_YET(conn_state, ESTAB) ?
					  4 : ((odlen > frn_win) ?
					       		 frn_win : odlen);

	/* Finish filling in the TCP header */
	ophp->tp_len = swab(sndlen + sizeof(struct tcp));
	otp->tc_cksum = cksum(ophp, sizeof(struct tcpph) >> 1, 0);
	otp->tc_cksum = ~cksum(otp, (sndlen + sizeof(struct tcp) + 1) >> 1,0);

	/* Send it */
#ifdef DEBUG
	if(NDEBUG & TPTRACE)
	    {
		printf("\nTCP: pkt[%u] to   %a; ",
		       sndlen + sizeof(struct tcp), tcp_fhost);
		tcp_disp_hdr(opbi);
		printf("frnwin %u\n", frn_win);
	    }
#endif
	in_write(tcpfd, opbi, sndlen + sizeof(struct tcp), tcp_fhost);

	/*  If there is anything acknowledgeable in this packet, set timer.*/
	if((sndlen + (int)otp->tc_fin) > 0)
		tm_tset(retry_time, tmhnd, 0, tcptm);

	ack_time = cticks + ACKDALLY;

	tcp_swab(otp);			/* swap it back so we can use it */
	tcppsnt ++;
	tcpbsnt = (unsigned)(otp->tc_seq - tcp_init_seq) + sndlen;
	tcprack = tcpbrcv;


	if(otp->tc_rst) {		/* were we resetting? */
		cleanup("aborted\n");
		tcp_src_port = 0;
		tcp_dst_port = 0;
		conn_state = CLOSED;
		(*tc_cfcn)();		/* call the user close routine */
		}

	if(conn_state ==  TIMEWAIT)	/* if we sent the last ack */
	    {
		tcp_src_port = 0;
		tcp_dst_port = 0;
		conn_state = CLOSED;
		(*tc_cfcn)();
	    }
	}
	}


/* Stuff a character into the send buffer for transmission, but do not
 * wake up the TCP sending task.  This assumes that more data will
 * immediately follow.
 */

tc_put(c)
	register char c;	 {		/* character to send */

	if(!blk_inpt) {
		odp[odlen] = c;
		odlen++;
		odp[odlen] = 0;
		if(odlen >= MAXBUF) return 1;
		else return 0;
		}
	else return 1;
	}

/* Stuff a character into the send buffer for transmission, and wake
 * up the TCP sender task to send it.
 */

tc_fput(c)
	register char c;	 {		/* character to send */

	if(!blk_inpt) {
		odp[odlen] = c;
		odlen++;
		odp[odlen] = 0;
		if(odlen > MAXBUF) return 1;
		}
	else return 1;

	tk_wake(TCPsend);
	return 0;
	}

/* Indicate the presence of urgent data.  Just sets the urgent pointer to
 * the current data length and wakes up the sender.
 */

tcpurgent() {
	otp->tc_urg = odlen;
	otp->tc_furg = 1;
	tk_wake(TCPsend);
	}

/* Initiate the TCP closing sequence.  This routine will return immediately;
 * when the close is complete the user close function will be called.
 */

tcp_close() {
	switch(conn_state) {
	case CLOSED:
		(*tc_cfcn)();
		return;
	case ESTAB:
		conn_state = FINSENT;	/* then fall through to next case */
	case SYNSENT:
		otp->tc_fin = 1;
		blk_inpt = 1;
		tk_wake(TCPsend);
		}
	}

/*  Initiate close/reset.  This routine just sets flags and wakes
 *  the TCP send task, which does the actual dirty work.
 */

tcp_reset() {
	otp->tc_rst = 1;	/*  Notify send that we want a reset.*/
	avail = 0;		/*  Reset processing of arriving data.*/
	blk_inpt = 1;		/*  Don't accept more departing data. */
	tk_wake(TCPsend);	/*  Caller should yield when ready for
				    the reset to go out.  */
	}

/* Actively open a tcp connection to foreign host fh on foreign socket
 * fs.  Get a unique local socket to open the connection on.  Returns
 * FALSE if unable to open an internet connection with the specified
 * hosts and sockets, or TRUE otherwise.
 * Note that this routine does not wait until the connection is
 * actually opened before returning.  Instead, the user open function
 * specified as ofcn in the call to tcp_init() (which must precede
 * this call) will be called when the connection is successfully opened.
 * This routine also sets a timer to call the user timeout routine
 * on ICP timeout.
 */

tcp_open(fh, fs, ls, win, lowwin)
	register in_name *fh;	/* foreign host address */
	unsigned fs;		/* foreign socket */
	unsigned ls;		/* local socket */
	int	win;		/* window to advertise */
	int	lowwin;	 {	/* low water mark for window */
	register int i;

	tcp_fhost = *fh;
	tcp_dst_port = fs;

	if((win<1) || (win>TCPMAXWIND) || (lowwin<1)||(lowwin>TCPMAXWIND)){
		tcp_window = TCPWINDOW;
		tcp_low_window = TCPLOWIND;
		}
	else {
		tcp_window = win;
		tcp_low_window = lowwin;
		}

	if((tcp_src_port = ls) == 0) {
		tcp_src_port = cticks;
		if(tcp_src_port < 1024) tcp_src_port += 1024;
		}

	for(i = 0; i < TOTALCHUNKS; i++) free[i] = TRUE;
	blk_inpt = 0;
	conn_state = SYNSENT;		/* syn-sent */
	odlen = 0;			/* no output data yet */
	fin_rcvd = 0;			/* DDP - no FIN received */

	if((tcpfd = in_open(TCPPROT, tcp_rcv, tcp_durcv)) == 0) {
#ifdef	DEBUG
		printf("TCP_OPEN: can't open ip con\n");
#endif
		(*tc_cfcn)();
		return;
		}

	/* alloc and set up output pkt */
	if((opbi = in_alloc(INETLEN, 0)) == NULL) {
#ifdef	DEBUG
		printf("TCP_OPEN: can't alloc pkt\n");
#endif
		(*tc_tfcn)();
		return;
		}

	otp = (struct tcp *)in_data(in_head(opbi));
	odp = (char *)otp + sizeof(struct tcp);
	optp = (struct tcp_option *)odp;

	/* allocate and set up psuedoheader for outgoing packet */
	ophp = (struct tcpph *)calloc(1, sizeof(struct tcpph));
	if(ophp == NULL) {
		printf("tcp_open: can't alloc output pseudohdr\n");
		return;
		}

	ophp->tp_src = in_mymach(tcp_fhost);
	ophp->tp_dst = *fh;
	ophp->tp_zero = 0;
	ophp->tp_pro = TCPPROT;

	/* allocate and set up psudoheader for incoming packets */
	iphp = (struct tcpph *)calloc(1, sizeof(struct tcpph));
	if(iphp == NULL) {
		printf("tcp_open: can't allocate input pseudohdr\n");
		return;
		}

	iphp->tp_zero = 0;
	iphp->tp_pro = TCPPROT;
	/* fill in output tcp hdr */
	otp->tc_thl  = (sizeof(struct tcp)+4) >> 2;	/* include option */
	otp->tc_srcp = tcp_src_port;
	otp->tc_dstp = tcp_dst_port;
/*	otp->tc_seq = 0;				   DDP */
	otp->tc_seq = tcp_init_seq = ~cticks << 1;	/* DDP */
	otp->tc_ack = 0;
	otp->tc_uu1 = 0;
	otp->tc_fin = 0;
	otp->tc_syn = 1;
	otp->tc_rst = 0;
	otp->tc_psh = 0;
	otp->tc_fack = 0;
	otp->tc_furg = 0;
	otp->tc_uu2 = 0;
	otp->tc_win = tcp_window;
	otp->tc_urg = 0;
	optp->tc_opt = 2;		/* set maximum segment size option */
	optp->tc_opt_len = 4;		/* option is 4 bytes long */
	optp->tc_opt_val = swab(TCP_SEGSIZE_OPT);
	tcppsnt = 0;			/* initially, no pkts sent */
	tcpprcv = 0;			/* or received */
	tcpbsnt = 0;			/* no bytes sent */
	tcpbrcv = 0;			/* foreign host sent no bytes */
	tcprack =0;			/* we acked no bytes of foreign host*/
	tcprercv = 0;
	tcpnodata = 0;
	tcpresend = 0;
	tcpbadck = 0;			/* no bad checksums yet */
	ign_win = 0;
	taken = -1;
	avail = -1;
	tk_wake(TCPsend);
	}

cleanup(why)
	char *why; {

	printf("Closed: %s\n", why);
	}

/*  Supply TCP data for line 25 display  */

tcp_state(s)
	char *s; {sprintf(s,"Sent: %u/%u/%u Rcvd: %u/%u/%u Wind: %u      ",
			tcpbsnt, tcpbsnt-(unsigned)(otp->tc_seq - tcp_init_seq), tcpresend,
			tcpbrcv, tcpbrcv-tcprack, tcprercv,
			otp->tc_win);}

/* Display some tcp statistics and a few lines of unacked data. Should be
	revised and integrated in with the normal logging system. */


tc_status() {
	int	i;	 /* loop index */
	int crseen = 0;

	printf("Connection State: ");

	switch(conn_state) {
	case CLOSED: 	printf("Closed\n");
			break;
	case SYNSENT:	printf("Trying to Open\n");
			break;
	case SYNRCVD:	printf("SYNRCVD\n");
			break;
	case ESTAB:	printf("Established\n");
			break;
	default:	printf("(%d) Closing\n",conn_state);
	}

	printf("Packets sent: %5u\tResends: %5u\n", tcppsnt, tcpresend);
	printf("Packets rcvd: %5u\tRercvds: %5u\tNot for me: %5u\n",
					 tcpprcv, tcprercv, tcpsock);
	printf("Bad TCP xsums: %5u\tNo data: %5u\tOver window: %5u\n",
	       tcpbadck, tcpnodata, ign_win);
	printf("Bytes Sent: %5u\tAcked: %u\n",
					tcpbsnt, (unsigned)(otp->tc_seq - tcp_init_seq));
	printf("Bytes Received: %5u\tAcked: %u\n", tcpbrcv, tcprack);
	printf("Local Win: %5u\tLocal Low Win: %5d\tForeign Win: %8u\n",
					otp->tc_win,tcp_low_window,frn_win);

	printf("Ack #: %08X, Seq #: %08X\n", otp->tc_ack, otp->tc_seq);
	printf("Output Flags: ");
	if(otp->tc_syn)  printf(" SYN");
	if(otp->tc_fack) printf(" ACK");
	if(otp->tc_psh)  printf(" PSH");
	if(otp->tc_furg) printf(" URG");
	if(otp->tc_fin)  printf(" FIN");
	if(otp->tc_rst)  printf(" RST");
	if(odlen) printf("\noutput data:\n");
	else {
		putchar('\n');
		return;
		}

	i=0;
	while(1) {
		if(odp[i] == '\n') crseen++;
		putchar(odp[i]);
		if(crseen > 3 || ++i > 100 || i > odlen) break;
		}

	printf("\n");
	if(i <= odlen) printf("***MORE DATA***\n\n");

	}


/* expedite (resend and push) a packet */
tcp_ex() {
	tk_wake(TCPsend);
	otp->tc_psh = 1;
	return;
	}

/* Handle a timer upcall. */
tmhnd() {
	resend++;
#ifdef DEBUG
	if(NDEBUG & TMO) printf("\nTCP:  timeout\n");
#endif
	tk_wake(TCPsend);
	}

tc_q() {
	}

/* Close and reset a tcp connection. */

tc_clrs(p, fhost)
	PACKET p;
	in_name fhost; {
	long ltemp;
	unsigned myport;
	register struct tcp *ptp;
	struct tcpph php;

	ptp = (struct tcp *)in_data(in_head(p));

	/* swap port numbers */
	myport = ptp->tc_dstp;
	ptp->tc_dstp = ptp->tc_srcp;
	ptp->tc_srcp = myport;

	/* fill in the rest of the header */
	ltemp = ptp->tc_seq;
	ptp->tc_seq = ptp->tc_ack;
	ptp->tc_ack = ltemp;
	ptp->tc_thl = sizeof(struct tcp) >> 2;
	ptp->tc_uu1 = 0;
	ptp->tc_fin = 0;
	ptp->tc_syn = 0;
	ptp->tc_rst = 1;	/* The RESET bit */
	ptp->tc_psh = 0;
	ptp->tc_fack = 0;
	ptp->tc_furg = 0;
	ptp->tc_uu2 = 0;
	ptp->tc_win = 0;
	tcp_swab(ptp);

	/* Set up the tcp pseudo header */
	php.tp_src = in_mymach(fhost);
	php.tp_dst = fhost;
	php.tp_zero = 0;
	php.tp_pro = TCPPROT;
	php.tp_len = swab(sizeof(struct tcp));

	/* checksum the packet */
	ptp->tc_cksum = cksum(&php, sizeof(struct tcpph) >> 1, 0);
	ptp->tc_cksum = ~cksum(ptp, (sizeof(struct tcp) + 1) >> 1, 0);

#ifdef DEBUG
	if(NDEBUG & TPTRACE)
	    {
	  	printf("\nTCP(reset): pkt[%u] to   %a; ",
		       sizeof(struct tcp), fhost);
		tcp_disp_hdr(p);
	    }
#endif
	in_write(tcpfd, p, sizeof(struct tcp), fhost);
	}


#define	GETTIME	0x2c

long get_dosl();

tcp_sock() {
	long temp;

	temp = get_dosl(GETTIME);
	temp &= 0xffff;
	if(temp < 1000) temp += 1000;
	return (unsigned)temp;
	}


/* DDP - Begin addition */
/* Send count number of bytes starting at buf over tcp connection.
   If count is greater then MAXBUF, then do on a piece by piece basis.
   Note, that task which issues this call will be blocked until all
   data are sent
 */

twrite_far(buf,count)
register char far *buf;	/* CCK far pointer for tn3270 data space reduction */
int count;
{
	register char *to;
	char *end;
	int i;
 
	if(!blk_inpt) {				/* Connection open? */
		while (count) {         	/* Have data to send? */
			i = (count > (MAXBUF - odlen)) ? /* Enough buffer? */
				(MAXBUF - odlen) : count;
			to = odp + odlen;	/* Copy starting here */
			end = to + i;		/* End here */
			while(to != end)	/* Copy it */
				*to++ = *buf++;
			odlen += i;		/* Update odlen */
			odp[odlen] = 0;
			count -= i;             /* Update count */
			while (odlen >= MAXBUF && count) {
				tk_wake(TCPsend);
				tk_yield();	/* Wait for buffer available*/
			}
		}
	}
	else return 1;

	tk_wake(TCPsend);			/* for final buffer */
	return 0;
}


/* DDP - Allow use of old twrite() routine, but convert it into a call to
   the new twrite_far routine.
 */

twrite(buf,count)
register char *buf;
int count;
{
	twrite_far((char far *)buf, count);
}
/* DDP - End addition */
