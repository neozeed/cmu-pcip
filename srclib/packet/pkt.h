/* Copyright (c) 1988 by Epilogue Technology Corporation

Permission to use, copy, modify, and distribute this program for any
purpose and without fee is hereby granted, provided that this
copyright and permission notice appear on all copies and supporting
documentation, the name of Epilogue Technology Corporation not be
used in advertising or publicity pertaining to distribution of the
program without specific prior permission, and notice be given in
supporting documentation that copying and distribution is by
permission of Epilogue Technology Corporation.  Epilogue Technology
Corporation makes no representations about the suitability of this
software for any purpose.
This software is provided "AS IS" without express or implied warranty.  */

/* $Revision:   1.1  $		$Date:   07 Mar 1988 12:43:32  $	*/
/*
 * $Log:   C:/karl/cmupcip/srclib/pkt/pkt.h_v  $
 * 
 *    Rev 1.1   07 Mar 1988 12:43:32
 * Added new statistics fields
 * 
 *    Rev 1.0   04 Mar 1988 16:32:56
 * Initial revision.
*/

#if (!defined(pkt_include))
#define pkt_include 1

/* The following definitions are from the FTP Software Packet Driver	*/
/* specification.							*/

/* Range in which the packet driver interrupt may lie	*/
#define	PKTDRVR_MIN_INT		0x60
#define	PKTDRVR_MAX_INT		0x7F

/* The interface classes */
#define IC_ETHERNET		1
#define	IC_PRONET10		2
#define	IC_TOKENRING		3
#define IC_OMNINET		4
#define IC_APPLETALK		5
#define IC_SERIAL_LINE		6
#define IC_STARLAN		7

/* Packet driver error codes */
#define PDE_BAD_HANDLE		1
#define PDE_NO_CLASS		2
#define PDE_NO_TYPE		3
#define PDE_NO_NUMBER		4
#define PDE_BAD_TYPE		5
#define PDE_NO_MULTICAST	6
#define PDE_CANT_TERMINATE	7
#define PDE_BAD_MODE		8
#define PDE_NO_SPACE		9		/* Added */
#define PDE_TYPE_INUSE  	10		/* Added */
#define PDE_BAD_COMMAND		11		/* Added */
#define PDE_CANT_SEND		12		/* Added */
#define PDE_NO_PKT_DRVR		128		/* Added */

/* Interface types (within each class) */
#define IT_ANY			0xFFFF

/* Devices in the Ethernet class */
#define _3COM_3C501		1
#define _3COM_3C505		2
#define INTERLAN_NI5010		3
#define	BICC_4110		4
#define	BICC_4117		5
#define INTERLAN_NP600		6
#define UB_NIC			8
#define UNIVATION		9
#define TRW_PC2000		10
#define	INTERLAN_NI5210		11
#define	_3COM_3C503		12
#define	_3COM_3C523		13
#define	WESTERN_DIGITAL_WD8003	14

/* ProNET-10 */
#define PROTEON_P1300		1

/* IEEE 802.5 & IBM Token Ring */
#define IBM_TOKEN_RING		1
#define PROTEON_P1340		2
#define PROTEON_P1344		2

/* Extended driver receive modes */
#define PKT_RCVR_OFF		1
#define PKT_RCVR_UNICAST	2	/* unicast only */
#define PKT_RCVR_BCAST		3	/* unicast + b'cast */
#define PKT_RCVR_LIM_MCAST	3	/* unicast + b'cast + some m-cast */
#define PKT_RCVR_ALL_MCAST	4	/* unicast + b'cast + all m-cast  */
#define PKT_RCVR_PROMISCUOUS	5	/* everything */

/* Define the pkt driver function codes */
#define PF_DRIVER_INFO		1
#define PF_ACCESS_TYPE		2
#define PF_RELEASE_TYPE		3
#define PF_SEND_PKT		4
#define PF_TERMINATE		5
#define PF_GET_ADDRESS		6
#define PF_RESET_INTERFACE	7
#define PF_SET_RCV_MODE		20
#define PF_GET_RCV_MODE		21
#define PF_SET_MULTICAST_LIST	22
#define PF_GET_MULTICAST_LIST	23
#define PF_GET_STATISTICS	24

typedef struct
	{
	int		version;
	int		pdtype;
	unsigned char	class;
	unsigned char	number;
	char	far 	*name;
	char		basic_flag;
	} pkt_driver_info_t;

typedef struct
	{
	long int  pkts_in;
	long int  pkts_out;
	long int  bytes_in;
	long int  bytes_out;
	long int  errors_in;
	long int  errors_out;
	long int  packets_dropped;
	long int  pad[4];		/* Space for FTP Software to	*/
					/* add new items.		*/
	/* The remaining fields are TRW PC-2000 extensions		*/
	long int  ints;			/* Interrupts			*/
	/* XMIT errors... */
	long int  cd_lost;		/* Lost Carrier Detect on XMIT	*/
	long int  cts_lost;		/* Lost Clear-to-send on XMIT	*/
	long int  xmit_dma_under;	/* XMIT DMA underrun		*/
	long int  deferred;		/* XMITs deferred		*/
	long int  sqe_lost;		/* SQE failed to follow XMIT	*/
	long int  collisions;		/* Collisions during XMIT	*/
	long int  ex_collisions;	/* XMITs terminated due to	*/
					/*   excess collisions		*/
	long int  toobig;		/* User packet too big to XMIT	*/
	long int  refused;		/* User packet refused because	*/
					/*   no XMIT resources available*/
	int	  max_send_pend;	/* High water mark of pending	*/
					/*   transmissions.		*/
	int	  place_holder_0;	/* Place holder for future use	*/
	/* RCV errors ... */
	long int  shorts;		/* RCVD packets < min length	*/
	long int  longs;		/* RCVD packets > max length	*/
	long int  skipped;		/* RCVD packets discarded by	*/
					/*   software.			*/
	long int  crcerrs;		/* RCVD packets with CRC errors	*/
	long int  alnerrs;		/* RCVD pkts w/ alignment errors*/
	long int  rscerrs;		/* RCVD pkts but no 82586 rcv	*/
					/*   resources			*/
	long int  ovrnerrs;		/* RCVD pkts lost due to rcv	*/
					/*   DMA overruns (occurs often	*/
					/*   when using 82586 in	*/
					/*   loopback mode.)		*/
	long int  unwanted;		/* RCVD packets skipped by	*/
					/*   software because no user	*/
					/*   wants that type.		*/
	long int  user_drops;		/* RCVD packets skipped by	*/
					/*   software because user would*/
					/*   not provide a buffer.	*/
	} pkt_driver_statistics_t;

extern int  pkt_access_type(int, int, int, char *, unsigned int, int (*)());
extern int  pkt_driver_info(int, pkt_driver_info_t *);
extern int  pkt_release_type(int);
extern int  pkt_send(char *, unsigned int);
extern int  pkt_terminate(int);
extern int  pkt_get_address(int, char *, int);
extern int  pkt_reset_interface(int);
extern int  pkt_set_rcv_mode(int, int);
extern int  pkt_get_rcv_mode(int);
extern int  pkt_get_statistics(int, pkt_driver_statistics_t *, int);
extern int  pkt_set_multicast_list();
extern int  pkt_get_multicast_list();
extern pkt_receive_helper();

extern unsigned char pkt_errno;
#endif
