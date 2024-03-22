#include <stdio.h>
#include <stdlib.h>

/* $Revision:   1.0  $		$Date:   04 Mar 1988 16:32:44  $	*/
/*
 * $Log:   C:/KARL/CMUPCIP/SRCLIB/PKT/PDKILL.C_V  $
 * 
 *    Rev 1.0   04 Mar 1988 16:32:44
 * Initial revision.
*/

#include "pkt.h"

#define then

main(argc, argv)
	int argc;
	char **argv;
{
int handle;
int rcode;
static char weirdtype[] = { 0x88, 0x66 };

if ((handle = pkt_access_type(IC_ETHERNET, IT_ANY, 0,
     weirdtype, sizeof(weirdtype), pkt_receive_helper))
     == -1)
      then {
	   printf("Can't access the packet driver\n");
	   exit(1);
	   }

if ((rcode = pkt_terminate(handle)) == 0)
   then printf("Packet driver terminated\n");
   else {
	printf("Can not terminate packet driver\n");
	pkt_release_type(handle);
	}
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