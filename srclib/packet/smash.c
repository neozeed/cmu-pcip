#include <stdio.h>
#include <stdlib.h>

/* $Header:   E:/cmupcip/srclib/pkt/smash.c_v   1.2   19 Jun 1988 18:44:56  $	*/
/*
 * $Log:   E:/cmupcip/srclib/pkt/smash.c_v  $
 * 
 *    Rev 1.2   19 Jun 1988 18:44:56
 * Added parameter to specify packet length
 * 
 *    Rev 1.1   03 Jun 1988 16:45:30
 * Retry count made a command line parameter.
 * 
 *    Rev 1.0   03 Jun 1988 16:24:12
 * Initial revision.
*/

#include "pkt.h"

#define then

char pkt_buf[1256] = {	0, 0, 0x2A, 00, 12, 36,	/* Destination	*/
			0, 4, 3, 2, 1, 0,	/* Source	*/
			9, 7,			/* Type/length	*/
			1,2,3,4,5,6,7,8,9,10};
main(argc, argv)
	int argc;
	char **argv;
{
unsigned int retries;
register unsigned int tries;
register unsigned int sucess, failure;
int handle;
int rcode;
int plen;

static char weirdtype[] = { 0x88, 0x67 };

if (argc != 3)
   then {
	printf("Usage: smash repeat-count length\n");
	exit(1);
	}

retries = atoi(argv[1]);
plen = atoi(argv[2]);

if ((handle = pkt_access_type(IC_ETHERNET, IT_ANY, 0,
     weirdtype, sizeof(weirdtype), pkt_receive_helper))
     == -1)
      then {
	   printf("Can't access the packet driver\n");
	   exit(1);
	   }

for (sucess = failure = 0, tries = retries; tries != 0; tries--)
   {
   if (pkt_send(pkt_buf, plen) == 0)
      then sucess++;
      else failure++;
   }

pkt_release_type(handle);

printf("Of %u retries, %u suceeded, %u failed\n", retries, sucess, failure);
}

char far *
pkt_rcv_call1(handle, len)
	unsigned int handle, len;
{
return (char far *)0;
}

void
pkt_rcv_call2(handle, len, buff)
	unsigned int handle, len;
	char far *buff;
{
}
