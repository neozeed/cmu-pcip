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

/* 1/24/84 - changed to understand octet mode (which is the same as image).
						<John Romkey>
   3/21/84 - corrected several places in the code where cn->tf_expected
	was referenced as cn->tf_expect.
						<John Romkey>
   1/2/85 - removed references to IMAGE mode.
						<J. H. Saltzer>
   5/2/85 - added calls to yield ahead of packet buffer allocation,
        to flush out broadcast packets that have piled up and may be
        clogging the buffers.  5/19/85 - moved output buffer allocation
        here from tfmkcn().
						<J. H. Saltzer>
*/

/* This is the source code for TFTP. I've tried to write it so that it
	may be called from inside a program with minimal hassle... */

/* User TFTP: attempt to transmit or receive a file in the specified mode. */

extern int NBUF;
extern long cticks;

long tftpuse(fhost, fname, rmfile, dir, mode, size, deltat)
	in_name fhost;
	char *fname;
	char *rmfile;
	unsigned dir;
	unsigned mode;
	long size;
	long *deltat; {
	register struct tfconn *cn;
	unsigned mysock;
	PACKET pfill, psnd;
	int i;
	long len;
	unsigned flen;
	unsigned done;
	char *data;
	int temp;
	unsigned CRSEEN = 0;
	unsigned HOLDING = 0;
	char HELD;

#ifdef	DEBUG
	if(NDEBUG & INFOMSG)
		printf("tftp_use(%a, %s, %s, %u, %u.\n", fhost, fname,
							rmfile, dir, mode);
#endif
	cn = tfmkcn(dir, mode, MAXBUFSIZE);
	if(cn == 0) {
		printf("TFTP: Couldn't make connection.\n");
		return 0;
		}
	i = 0;
	while((cn->tf_outp = udp_alloc(NORMLEN, 0)) == 0) /*  grab a buffer */
	  {
	      if (++i < 100)		/*  none free, try to free up some */
		{
		    tk_yield();
		}
	      else			/*  won't free up, give up.  */
	        {
	        printf("TFTP: Couldn't allocate output packet.\n");
		return 0;
	        }
	  }

	if(dir == GET) {
		if(mode == ASCII || mode == OCTET)
					cn->tf_fd = fopen(fname, "wb");
		else if(mode == TEST) cn->tf_fd = (FILE *)1;
		else {
			printf("TFTP_USER: Bad mode %u.\n", mode);
			return 0;
			}

		if(cn->tf_fd == 0) {
		   printf("TFTP_USER: Couldn't open local file %s.\n", fname);
			return 0;
			}
		*deltat = cticks;

		mysock = udp_socket();
		cn->tf_udp = udp_open(fhost, TFTPSOCK, mysock, tftprcv, cn);

		if(cn->tf_udp == 0) {
			printf("TFTP_USER: Couldn't open UDP connection.\n");
			return 0;
			}

		tfsndreq(cn, rmfile);

		cn->tf_expected = 1;
		cn->tf_fport = 0;

		while(1) {
		cn->tf_state = DATAWAIT;
		while(cn->tf_state == DATAWAIT) tk_block();

		if(cn->tf_state == TIMEOUT) {
			printf("TFTP:  Host not responding, giving up.\n");
			tfudperr(cn->tf_udp, cn->tf_outp, ERRTXT,
					"Retry limit exceeded, giving up");
			tfcleanup(cn);
			return 0;
			}

		if(cn->tf_state == RCVLASTDATA) {
			len = tfcleanup(cn);
			*deltat = cticks - *deltat;
			if(len == 0)
			printf("TFTP:  Tried to transfer zero-length file\n");
			return len;
			}

		if(cn->tf_state != RCVDATA) {
			tfcleanup(cn);
			return 0;
			}
		}
	}

	else if(dir == PUT) {
		if(mode == ASCII || mode == OCTET)
			cn->tf_fd = fopen(fname, "rb");
		else {
			printf("Invalid mode for put.\n");
			return 0;
			}

		if(cn->tf_fd == 0) {
			printf("TFTP: Couldn't open file %s for read.\n",
									fname);
			return 0;
			}

		*deltat = cticks;

		mysock = udp_socket();
		cn->tf_udp = udp_open(fhost, TFTPSOCK, mysock, tftprcv, cn);
		if(cn->tf_udp == 0) {
			printf("TFTP: Couldn't open UDP connection.\n");
			fclose(cn->tf_fd);
			return 0;
			}

		i = 0;
		while((pfill = udp_alloc(NORMLEN, 0)) == 0) /*  grab buffer */
		  {
		      if (++i < 100)	/*  none free, try to free up some */
			{
			    tk_yield();
			}
		      else		/*  won't free up, give up.  */
			{
			    printf("TFTP: Couldn't allocate fill packet.\n");
			    return 0;
			}
		  }
		cn->tf_fport = 0;
		tfsndreq(cn, rmfile);
		cn->tf_state = ACKWAIT;
		cn->tf_expected = 0;
		flen = NORMLEN;
		done = 0;

		/* Here's how the main loop for putting data blocks works:
			We make the request packet and then we enter the main
			loop.

			SEND current pkt	<-----------------------+
			If we haven't filled the data pkt, fill it	|
			Wait for the ACK				|
			If timeout, ------------------------------------+
			Swap the pkt buffers				|
			GOTO -------------------------------------------+
		*/

	while(1) {
		data = (char *)tftp_data(pfill);
		for(flen=0; flen<NORMLEN; flen++) {
			if(HOLDING) {
				HOLDING = 0;
				data[flen] = HELD;
				continue;
				}
			temp = getc(cn->tf_fd);
			if(CRSEEN) {
				CRSEEN = 0;
				if(temp == 0x0a) {
					data[flen] = temp;
					continue;
					}
				else {
					HOLDING = 1;
					HELD = temp;
					data[flen] = 0;
					continue;
					}
				}

			if(cn->tf_mode == ASCII && temp == 0x0d)
				CRSEEN = 1;
			if(temp == EOF) break;
			data[flen] = temp;
			if(cn->tf_mode == ASCII && temp == dos_eof)
				break;
			}

		cn->tf_size += flen;

		while(cn->tf_state == ACKWAIT) tk_block();

		if(cn->tf_state == TIMEOUT) {
			printf("TFTP:  Host not responding, giving up.\n");
			tfudperr(cn->tf_udp, cn->tf_outp,
				 ERRTXT, "Retry limit exceeded, giving up");
			cn->tf_state = DEAD;
			tfcleanup(cn);
			udp_free(pfill);
			return 0;
			}

		if(cn->tf_state == RCVACK) {
			cn->tf_expected++;

			/* Check if we're done */
			if(done){
				len = tfcleanup(cn);
				udp_free(pfill);
				*deltat = cticks - *deltat;
				if(len==0)
			printf("TFTP:  tried to transfer zero-length file\n");
				return len; 
				}
			/* If not, resume the loop */
			psnd = pfill;
			pfill = cn->tf_outp;
			cn->tf_outp = psnd;
			tfsndata(cn, flen);
			cn->tf_state = ACKWAIT;
			if(flen < NORMLEN) done = 1;
			continue;
			}

		tfcleanup(cn);
		udp_free(pfill);
		return 0;
		}
		}

	printf("Falling off the edge!\n");
	return 0;
	}
