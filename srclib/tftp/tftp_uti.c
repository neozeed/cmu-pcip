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
#include <ip.h>
#include <udp.h>
#include <timer.h>
#include <tftp.h>
#include "tftp.h"


#define MINTICKS 20

/* Utility routines */

/* Modified to eliminate dangerous resends, simplify round-trip
   time estimation, identify old and extra connections and shut
   them down, and use clock-tick timer, 12/83.
   Modified to use standard error code for old TID's and to byte-
   swap the error code.  12/23/83.
   Modified to check foreign port number before considering 
   error packets to be meaningful.  Also arranged to send
   error packets back in the incoming packet buffer, to
   avoid clobbering the current output packet, 1/84.  
   Initialization of new connections improved to set file descriptor
   and udp connection pointers to null, to avoid errors on premature
   cleanup, 1/16/84.  <J.H.Saltzer>

   1/24/84 - added octet mode.
					<John Romkey>

   12/24/84 - added buffer to speed up DOS disk file I/O.
					<J. H. Saltzer>
   1/2/85  - fixed length calculation bug in tfudperr.
					<John Romkey>
   1/2/85 - removed reference to IMAGE mode.
					<J. H. Saltzer>
   4/27/85 - converted netascii gets to use DOS disk file buffer.
   This change fixes the "Disk is full!" bug on getting a binary
   file in netascii mode.  Also changed netascii get to not tack
   DOSEOF (control-Z) on end of file.  5/2/85 - added yields to
   free up packet buffers ahead of calls to udp_alloc. 5/19 - fixed
   tfcleanup() to check for null pointers before all deallocate calls;
   Rearranged code order to make it easier to find things; moved
   responsibility to allocate output packet to caller of tfmkcn(),
   to allow it to happen at downcall time to allow yields to free
   up some buffers.  7/3/85 - made write buffer size an argument to
   allow use as a print spooler.
   					<J. H. Saltzer>
*/

extern int NBUF;

/* Setup a TFTP connection block. */

struct tfconn *tfmkcn(dir, mode, maxbuffs)
unsigned dir;
unsigned mode;
int maxbuffs;	/* # of blocks to buffer between DOS disk writes */
{
	unsigned bufbytes;
	register int i;
	register struct tfconn *cn;

	cn = (struct tfconn *)malloc(sizeof(struct tfconn));
	if(cn == 0) {
		printf("TFTP: Couldn't allocate connection block.\n");
/*		log(tftplog, "Couldn't allocate connection block.");	*/
		return 0;
		}

	cn->tf_fd = NULL;
	cn->tf_udp = 0;
	cn->tf_rcv = 0;
	cn->tf_snt = 0;
	cn->tf_ous = 0;
	cn->tf_tmo = 0;
	cn->tf_rsnd = 0;
	cn->tf_dir = dir;
	cn->tf_mode = mode;
	cn->tf_looks_bad = FALSE;
	cn->tf_size = 0L;
	cn->tf_K = Kinit;
	cn->tf_trt = T0;
	cn->tf_rt = min( cn->tf_trt*TMMULT , MAXTMO);
	cn->tf_NR = 0;
	cn->tf_NR_last = 1;

	cn->tf_tm = tm_alloc();
	if(cn->tf_tm == 0) {
		printf("TFTP: Couldn't allocate timer.\n");
		free(cn);
		return 0;
		}

	cn->tf_outp = 0;	/* defer packet allocation to downcall time */
	cn->tf_task = tk_cur;

	cn->tf_buff_bytes = NORMLEN*maxbuffs;
	while(cn->tf_buff_bytes > 0)
		{
		if((cn->tf_fbufp = (char *)malloc(cn->tf_buff_bytes)) != 0)
				 break;
		cn->tf_buff_bytes -= NORMLEN;
		}
	if(cn->tf_buff_bytes == 0) {
		printf("TFTP:  Couldn't allocate disk buffer.\n");
		tm_free(cn->tf_tm);
		free(cn);
		return 0;
		}
	if(NDEBUG & APTRACE)
		printf("TFTP:  Using %u-byte disk buffer at %x.\n",
					cn->tf_buff_bytes, cn->tf_fbufp);
	cn->tf_bytes_used = 0;
	return cn;
	}


/* Cleanup routine called when done */

long tfcleanup(cn)
register struct tfconn *cn;
{
	long size;

	/* Give us one last chance to flush out a packet that hasn't been
		processed yet. */
	tk_yield();

#ifdef	DEBUG
	if(NDEBUG & INFOMSG)
		printf("TFCLEANUP called\n");
#endif
	if(cn->tf_fd != NULL) fclose(cn->tf_fd);
	if(cn->tf_udp != 0) udp_close(cn->tf_udp);
	if(cn->tf_tm != 0) 
	  {
	      tm_clear(cn->tf_tm);
	      tm_free(cn->tf_tm);
	  }
	if(cn->tf_outp != 0) udp_free(cn->tf_outp);
	if(cn->tf_fbufp != 0) free(cn->tf_fbufp);

	size = cn->tf_size;	/*  Save only item of value  */
	free(cn);
	return size;
	}


tftptmo(cn)
	register struct tfconn *cn; {

	if(NDEBUG & TMO) printf("TFTP: Timeout.\n");

	cn->tf_tmo++;
	if(--cn->tf_tries) {
		if(NDEBUG & APTRACE) printf("TFTP:  resending\n");
		cn->tf_rsnd++;
		cn->tf_NR++;
		udp_write(cn->tf_udp, cn->tf_outp, cn->tf_lastlen);
		tm_clear(cn->tf_tm);
		tm_tset((int)cn->tf_rt, tftptmo, cn, cn->tf_tm);
		}
	else {
		cn->tf_state = TIMEOUT;
		tk_wake(cn->tf_task);
		}
	}

tf_good(cn)
	register struct tfconn *cn; {
	long trtM;

	trtM = cticks - cn->tf_sent;  /*  Measured round trip time  */
	if(cn->tf_NR_last == 1) cn->tf_K = Kinit;

	if(cn->tf_NR == 1)
		cn->tf_trt = (trtM+cn->tf_trt)/2;
	else {
		if((cn->tf_NR_last > 1) && (cn->tf_K >1) )
			cn->tf_K -= Kinc;

		cn->tf_trt += cn->tf_trt/cn->tf_K;
		}
	cn->tf_rt = max(min( cn->tf_trt*TMMULT , MAXTMO), MINTICKS);
	cn->tf_NR_last = cn->tf_NR;
	}


/* Format up and send out an initial request for a tftp connection. */

tfsndreq(cn, fname)
	register struct tfconn *cn;
	char *fname; {
	PACKET p;
	register struct tfreq *ptreq;
	unsigned reqlen;

	p = cn->tf_outp;
	ptreq = (struct tfreq *)tftp_head(p);
	if(cn->tf_dir == GET) ptreq->tf_op = RRQ;
	else if(cn->tf_dir == PUT) ptreq->tf_op = WRQ;
	else {
		printf("TFSNDREQ: Bad direction %u.\n", cn->tf_dir);
		tfcndump(cn);
#ifdef	DEBUG
		if(NDEBUG & BUGHALT) exit(1);
#endif
		return -1;
		}

	strcpy(ptreq->tf_name, fname);
	if(cn->tf_mode == OCTET || cn->tf_mode == TEST)
		strcpy(ptreq->tf_name+strlen(fname)+1, "octet");
	else if(cn->tf_mode == ASCII)
		strcpy(ptreq->tf_name+strlen(fname)+1, "netascii");
	else {
 		printf("TFSNDREQ: Bad mode %u.\n", cn->tf_mode);
		tfcndump(cn);
#ifdef	DEBUG
		if(NDEBUG & BUGHALT) exit(1);
#endif
		return -1;
		}

	if(NDEBUG & APTRACE) printf("TFTP:  sending initial request\n");
	return tf_write(cn, sizeof(struct tfreq) + strlen(fname));
	}


/* Process a data packet received for TFTP connection cn, according to the
	type (ASCII, OCTET, TEST) specified in the connection block. Also
	handle out of secquence blocks and check on block length. If a block
	is way to short (len < tftp header size) send back an error message
	and abort the transfer; we have CSR disease. If the block is less
	than 512 bytes long, shut down the transfer; we're done.
	Otherwise, just write it to disk (if necessary).
*/

tfdodata(cn, p, len)
	register struct tfconn *cn;
	PACKET p;
	unsigned len; {
	register char *data;
	struct tfdata *ptfdat;
	unsigned count;

	if(len < 4) {
/*		log(tftplog, "TFDODATA: CSR disease; len = %u", len);	*/
		tfrpyerr(cn->tf_udp, p, ERRTXT, "You have CSR disease.");
		printf("TFDODATA: Died of CSR disease.\n");
		tfkill(cn);
		return;
		}

	if(cn->tf_state != DATAWAIT) {
#ifdef	DEBUG
		if(NDEBUG & PROTERR)
			printf("TFTP:  Received unexpected data block.\n");
#endif

		/*tfrpyerr(cn->tf_udp, p, ERRTXT, "Rcvd unexpected data block");*/
		return;
		}

	ptfdat = (struct tfdata *)tftp_head(p);
	len -= 4;				/* BAD. */

	if(ptfdat->tf_block != cn->tf_expected) {
		if(NDEBUG & (INFOMSG|PROTERR|APTRACE))
			printf("TFTP:     Got block %u, expecting %u.\n",
					ptfdat->tf_block, cn->tf_expected);

/*	We got a retransmission of a packet we have already tried to
ACK.  If we retransmit the ACK, and the old ACK finally gets through also,
our correspondent will resend the next data block, and we will do the same
thing for it, on through to the end of the file.  So we shouldn't retransmit
the ACK until we are convinced that the first one was actually lost.  We will
become convinced if our own timeout waiting for the next data packet
expires.  */

/*  Here is what you shouldn't do. . .
		if(ptfdat->tf_block == cn->tf_expected-1)
			tfsndack(cn, ptfdat->tf_block);
    And now we return to correct procedures. . .  */
		cn->tf_ous++;	
		return;
		}

	/* Send the ack before writing the data */
	tm_clear(cn->tf_tm);
	tf_good(cn);

	tfsndack(cn, ptfdat->tf_block);

	cn->tf_size += len;
	data = ptfdat->tf_data;

	if( cn->tf_mode == OCTET) 
		{ 
		copy512(data, cn->tf_fbufp+cn->tf_bytes_used);
		cn->tf_bytes_used += len;
		if(len < NORMLEN || cn->tf_bytes_used == cn->tf_buff_bytes)
			{
			if(fwrite(cn->tf_fbufp,1,cn->tf_bytes_used, cn->tf_fd)
			      != cn->tf_bytes_used) {tf_full(cn,p); return;}
			cn->tf_bytes_used = 0;
			}
		}
	else if(cn->tf_mode == ASCII) for(count = 0; count<len; count++)
	        {
		if(data[count])
		    {
		    if(data[count]==dos_eof) cn->tf_looks_bad = TRUE;
 		    cn->tf_fbufp[cn->tf_bytes_used] = data[count];
		    if (++(cn->tf_bytes_used) == cn->tf_buff_bytes)
		        {
			if(fwrite(cn->tf_fbufp,1,cn->tf_bytes_used, cn->tf_fd)
			      != cn->tf_bytes_used) {tf_full(cn,p);return;}
			cn->tf_bytes_used = 0;
		        }
		    }
	        }
	else if(cn->tf_mode == TEST) 
		{}
	else {
		tfrpyerr(cn->tf_udp, p, ERRTXT, "Internal Error.");
		tfkill(cn);
		return;
		}

	if(len == NORMLEN) cn->tf_state = RCVDATA;
	else {
		cn->tf_state = RCVLASTDATA;
		if(cn->tf_mode == ASCII)
			if(fwrite(cn->tf_fbufp,1,cn->tf_bytes_used, cn->tf_fd)
			      != cn->tf_bytes_used) {tf_full(cn,p);return;}
		}

	cn->tf_expected++;
	tk_wake(cn->tf_task);

	}

/*  Handle disk full condition by sending error packet and killing
    this connection.  */

tf_full(cn,p)

register struct tfconn *cn;
PACKET p;

{
	tfrpyerr(cn->tf_udp, p, DISKFULL, " ");
	printf("TFTP: disk is full!\n");
	tfkill(cn);	
}

/* Send an error packet. If the code is nonzero, put it in the packet.
	Otherwise, copy the text into the packet. */

static char *errors[] = {
	"The JNC Memorial BUGHALT",
	"File not found",
	"Access violation",
	"Disk full",
	"Illegal TFTP operation",
	"Unknown transfer ID",
	"File already exists",
	"No such user"
	};
/*
tfsnderr(cn, code, text)
	register struct tfconn *cn;
	unsigned code;
	char *text; { tfudperr(cn->tf_udp, cn->tf_outp, code, text); }
*/
tfudperr(udpc, p, code, text)
	UDPCONN udpc;
	PACKET p;
	unsigned code;
	char *text; {
	unsigned len;
	register struct tferr *perr;
	len = sizeof(struct tferr)-2;

	perr = (struct tferr *)tftp_head(p);
	perr->tf_op = bswap(ERROR);
	perr->tf_code = bswap(code);

	if(code == ERRTXT) {
		strcpy(perr->tf_err, text);
		len += strlen(text)+1;
		}
	else {
		strcpy(perr->tf_err, errors[code]);
		len += strlen(errors[code]) + 1;
		}
	if(NDEBUG & APTRACE)
		 printf("TFTP:  Sending error packet, code %u, <%s>\n", 
				code, perr->tf_err);
	return udp_write(udpc, p, len);
	}

/* Process an incoming error packet */

tfdoerr(cn, p, len)
	register struct tfconn *cn;
	PACKET p;
	unsigned len; {
	register struct tferr *perr;

	perr = (struct tferr *)tftp_head(p);
	printf("TFTP: Error from foreign host:\n");
	printf("\t%s\n", perr->tf_err);

	}

/* ack a certain block number */

tfsndack(cn, number)
	register struct tfconn *cn;
	unsigned number; {
	register struct tfack *pack;

	pack = (struct tfack *)tftp_head(cn->tf_outp);

	cn->tf_lastlen = sizeof(struct tfack);
	pack->tf_op = ACK;
	pack->tf_block = number;
	if(NDEBUG & APTRACE) printf("TFTP:  ACKing block %u\n", number);
	return tf_write(cn, sizeof(struct tfack));
	}

/* write a tftp packet */

tf_write(cn, len)
	register struct tfconn *cn;
	unsigned len; {
	register struct tfack *packet;
	unsigned res;

	packet = (struct tfack *)tftp_head(cn->tf_outp);
	if(packet->tf_op != RRQ && packet->tf_op != WRQ) {
		packet->tf_block = bswap(packet->tf_block);
		cn->tf_tries = TFTPTRIES;
		}		
		else cn->tf_tries = REQTRIES;
	packet->tf_op = bswap(packet->tf_op);

	cn->tf_lastlen = len;
	cn->tf_snt++;
	res =  ((udp_write(cn->tf_udp, cn->tf_outp, len) == 0) ? -1 : 0);


	tm_tset((int)cn->tf_rt, tftptmo, cn, cn->tf_tm);
	cn->tf_sent = cticks;
	cn->tf_NR = 1;
	}


/* Dump a connection block for debugging purposes. */

tfcndump(cn)
	register struct tfconn *cn; {

	printf("Connection addr = %04x\n", cn);
	printf("lastlen = %u\texpected = %u\n",
				cn->tf_lastlen, cn->tf_expected);
	printf("state = %u\tdir = %u\tmode = %u\n",
				cn->tf_state, cn->tf_dir, cn->tf_mode);
	printf("sent = %u\trcvd = %u\tous = %u\tmo = %u\trsnd = %u\n\n",
		cn->tf_snt, cn->tf_rcv, cn->tf_ous,cn->tf_tmo, cn->tf_rsnd);
	printf("round trip delay = %U\tK = %u\tcurnt tmo = %U\n",
			cn->tf_trt, cn->tf_K, cn->tf_rt);
	printf("size = %U\n", cn->tf_size);
	}

/* Log a connection block.

tfcnlog(cn)
	register struct tfconn *cn; {

	log(tftplog, "snt=%u rcvd=%u ous=%u tmo=%u rsnd=%u trt=%U",
		cn->tf_snt, cn->tf_rcv, cn->tf_ous, cn->tf_tmo, cn->tf_rsnd,
							cn->tf_trt);
	}
*/


/* Send a TFTP data block. */

tfsndata(cn, len)
	register struct tfconn *cn;
	unsigned len; {
	register struct tfdata *tfdata;

	tfdata = (struct tfdata *)tftp_head(cn->tf_outp);

	tfdata->tf_op = DATA;
	tfdata->tf_block = cn->tf_expected;
	if(NDEBUG & APTRACE) printf("TFTP:  sending block %u\n",
				tfdata->tf_block);
	return tf_write(cn, sizeof(struct tfdata)-512+len);
	}

/* Handle an incoming ack. */

tfdoack(cn, p, len)
	register struct tfconn *cn;
	PACKET p;
	unsigned len; {
	register struct tfack *ack;

	ack = (struct tfack *)tftp_head(p);

	if(ack->tf_block != cn->tf_expected) {/*  We have received an ACK,
			but not for the data block we sent.  It must be for
			a duplicate, since we wouldn't have sent
			the current data block if we hadn't gotten an ACK for
			the previous one.  This duplicate ACK means either
			that the network resent a packet that it wasn't sure
			got through, or else the other end resent the ACK
			because our current data block is lost or late.
			In either case, we can safely ignore this extra ACK,
			and if the ACK we want doesn't come our own timer will
			get us started again.  It isn't safe
			to resend the current data block now unless we are
			absolutely certain that the other end won't reack
			it if the earlier send was just delayed.  */

		cn->tf_ous++;
		if(NDEBUG & APTRACE)
			 printf("TFTP: ACK for block %u received again.\n",
						 ack->tf_block);
		}
	else {
		tf_good(cn);
		tm_clear(cn->tf_tm);
		cn->tf_state = RCVACK;
		tk_wake(cn->tf_task);
		}

	}


/* Handle an incoming packet. */

tftprcv(p, len, fhost, cn)
	PACKET p;
	unsigned len;
	in_name fhost;
	register struct tfconn *cn; {
	register struct tfdata *pdata;
	unsigned op;

	cn->tf_rcv++;
	pdata = (struct tfdata *)tftp_head(p);
	op = bswap(pdata->tf_op);
	pdata->tf_op = op;

	switch(op) {
	case DATA:
		if (tfckport(cn,p)) tfdodata(cn, p, len);
		break;
	case ACK:
		if (tfckport(cn,p)) tfdoack(cn, p, len);
		break;
	case ERROR:
		if ((cn->tf_fport == 0) ||
		    (cn->tf_udp->u_fport == udp_head(in_head(p))->ud_srcp))
				{
				tfdoerr(cn, p, len);
				tfkill(cn);}
		else if (NDEBUG & APTRACE) {
				printf("TFTP:  ignoring error packet.\n");
				tfdoerr(cn, p, len);}
		break;
	default:
		printf("TFTPRCV: Got bad opcode %u.\n", op);
		tfrpyerr(cn->tf_udp, p, ILLTFTP, " ");
		break;
		}
	udp_free(p);	/*  So much for that packet!  */
	return;
	
	}	/*  end tftprcv() */


/*  Check over the port in the incoming packet.  */

tfckport(cn, p)
	register struct tfconn *cn;
	PACKET p; {
	register struct tfdata *pdata;
	PACKET svoutp;
        unsigned pfport, svport;

	pdata = (struct tfdata *)tftp_head(p);
	pdata->tf_block = bswap(pdata->tf_block);
	pfport = udp_head(in_head(p))->ud_srcp;

	if(cn->tf_fport == 0) /*  Foreign port not yet identified, save it. */
	    if (cn->tf_expected == pdata->tf_block) {/* but only if this is */
		cn->tf_fport = 1;		     /*  a response to our  */
		cn->tf_udp->u_fport = pfport;	     /*  request.  */
		}
	    else  {
#ifdef	DEBUG
		if(NDEBUG & (PROTERR|NETERR))
		    printf("TFTP:  Received packet from old connection.\n");
		    printf("       Expected block %u, got block %u\n",
					cn->tf_expected, pdata->tf_block);
#endif
		tfrpyerr(cn->tf_udp, p, ERRTXT, "old connection");
		return FALSE;
		}

	else if( cn->tf_udp->u_fport != pfport) {
#ifdef	DEBUG
		if(NDEBUG & (PROTERR|NETERR)){
		printf("TFTP: Rcvd pkt from port %04x, expect port %04x,\n",
					pfport, cn->tf_udp->u_fport);
			}
#endif
		tfrpyerr(cn->tf_udp, p, BADTID, " ");
		return FALSE;
		}
	return TRUE;
	}	/*  end tfckport()  */


/*  Send error packet back where this packet came from.  */

tfrpyerr(udpc, p, code, text)
	UDPCONN udpc;
	PACKET	p;
	unsigned code;
	char	*text; {
	unsigned svport;
	in_name svhost;
	PACKET svoutp;
		svport = udpc->u_fport;  /* Save correct port no.  */
		svhost = udpc->u_fhost;  /*  and host id  */
		udpc->u_fport = udp_head(in_head(p))->ud_srcp;  /* improper */
		udpc->u_fhost = in_head(p)->ip_src;  /*  layer violation  */
		tfudperr(udpc, p, code, text);

		udpc->u_fport = svport; /*N.B. udperr must not yield */
		udpc->u_fhost = svhost; /* or these saves will fail  */
	}	/*  end tfrpyerr()  */
