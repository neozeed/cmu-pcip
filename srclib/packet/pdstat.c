#include <stdio.h>
#include <stdlib.h>

/* $Revision:   1.1  $		$Date:   07 Mar 1988 12:43:34  $	*/
/*
 * $Log:   C:/karl/cmupcip/srclib/pkt/pdstat.c_v  $
 * 
 *    Rev 1.1   07 Mar 1988 12:43:34
 * Added new statistics fields
 * 
 *    Rev 1.0   04 Mar 1988 16:32:46
 * Initial revision.
*/

#include "pkt.h"

#define then

main(argc, argv)
	int argc;
	char **argv;
{
pkt_driver_statistics_t pkt_stats;
int handle;
pkt_driver_info_t drvr_info;
char mymac[6];
static char weirdtype[] = { 0x88, 0x66 };

if ((handle = pkt_access_type(IC_ETHERNET, IT_ANY, 0,
     weirdtype, sizeof(weirdtype), pkt_receive_helper))
     == -1)
      then {
	   printf("Can't access the packet driver\n");
	   exit(1);
	   }

(void) pkt_driver_info(handle, &drvr_info);
(void) pkt_get_address(handle, mymac, sizeof(mymac));

printf("Ethernet/MAC address: %2.2X.%2.2X.%2.2X.%2.2X.%2.2X.%2.2X\n",
	mymac[0]&0xff, mymac[1]&0xff, mymac[2]&0xff,
	mymac[3]&0xff, mymac[4]&0xff, mymac[5]&0xff); /* DDP */

(void) pkt_get_statistics(handle, &pkt_stats, sizeof(pkt_stats));

printf("%8lu pkts in\t%8lu bytes in\n",
	(long)pkt_stats.pkts_in, (long)pkt_stats.bytes_in);

printf("%8lu pkts out\t%8lu bytes out\n",
	(long)pkt_stats.pkts_out, (long)pkt_stats.bytes_out);
	
printf("%8lu errs in\t%8lu errs out\t%8lu pkts dropped\n",
	(long)pkt_stats.errors_in, (long)pkt_stats.errors_out,
	(long)pkt_stats.packets_dropped);

if (drvr_info.pdtype == TRW_PC2000)
   {
   printf("%8lu ints\t%8lu no CD\t%8lu no CTS\t%8lu XMIT underrun\n",
	   (long)pkt_stats.ints, (long)pkt_stats.cd_lost,
	   (long)pkt_stats.cts_lost, (long)pkt_stats.xmit_dma_under);

   printf("%8lu defers\t%8lu no SQE\t%8lu cols\t%8lu ex cols\n",
	   (long)pkt_stats.deferred, (long)pkt_stats.sqe_lost,
	   (long)pkt_stats.collisions, (long)pkt_stats.ex_collisions);

   printf("%8lu toobig\t%8lu refuse\t   %5d Max send Q\n",
	   (long)pkt_stats.toobig, (long)pkt_stats.refused,
	   pkt_stats.max_send_pend);

   printf("%8lu shorts\t%8lu longs\t%8lu skips\t%8lu missed\n",
	   (long)pkt_stats.shorts, (long)pkt_stats.longs,
	   (long)pkt_stats.skipped, (long)pkt_stats.rscerrs);

   printf("%8lu bad crc\t%8lu align errs\t%8lu RCV overruns\n",
	   (long)pkt_stats.crcerrs, (long)pkt_stats.alnerrs,
	   (long)pkt_stats.ovrnerrs);

   printf("%8lu unwanted\t%8lu no user buffer\n",
	   (long)pkt_stats.unwanted, (long)pkt_stats.user_drops);
   }
pkt_release_type(handle);
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