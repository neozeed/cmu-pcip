#include <stdio.h>
#include <stdlib.h>

#pragma pack(1)

typedef unsigned char	uchar;
typedef unsigned short	uint;

#define ADDRLEN		6

/* Structure to hold a 24 bit physical address */
typedef struct
	{
	uint	low_16;
	uchar	hi_8;
	uchar	unused;
	} paddr_t;

#define then

/* SYSTEM CONTROL BLOCK (SCB) */
typedef struct
	{
	uint	scb_status;
	uint	scb_command;
	uint	scb_cbl_offset; /* Offset address - first command in	*/
				/*   the command list to be executed	*/
				/*   following a CU-START		*/
	uint	scb_rfa_offset; /* Offset address - first receive frame */
				/*   descriptor to be accesssed after	*/
				/*   an RU_START			*/
	uint	scb_crcerrs;	/* Count of aligned packets received	*/
				/*   with CRC errors			*/
	uint	scb_alnerrs;	/* Count of misaligned packets received */
	uint	scb_rscerrs;	/* Count of the number of packets	*/
				/*   discarded due to lack of resources */
	uint	scb_ovrnerrs;	/* Count of the number of packets lost	*/
				/*   because the memory bus was not	*/
				/*   available to 82586			*/
	} scb_t;

/* CU COMMAND BLOCKS */
/* Header found on all command blocks */
typedef struct
	{
	uint	cb_stat;	/* Status of command		*/
	uint	cb_command;	/* Command			*/
	uint	cb_link;	/* Link to next command		*/
	} cb_hdr_t;

/* DUMP CB */
typedef struct
	{
	cb_hdr_t cb_dump_hdr;		/* Standard CB header		*/
	uint	cb_dump_offset;		/* Offset to 170 (decimal byte	*/
					/*   dump area)			*/
	} dump_cb_t;

/* IA-SETUP CB */
typedef struct
	{
	cb_hdr_t cb_ias_hdr;		/* Standard CB header		*/
	uchar	 cb_ias_ia[ADDRLEN];	/* MAC/Ethernet address		*/
	} ias_cb_t;

/* TDR CB */
typedef struct
	{
	cb_hdr_t cb_tdr_hdr;		/* Standard CB header		*/
	uint	 cb_tdr_result;		/* TDR results			*/
	} tdr_cb_t;

typedef struct
	{
	cb_hdr_t	cb_tran_hdr;	/* Standard CB header		*/
	uint	 	cb_tran_tbd;	/* Offset - 1st transmit buffer */
					/*   descriptor			*/
	uchar	 cb_tran_mac[ADDRLEN];	/* Destination MAC address	*/
	uint	 	cb_tran_length;	/* Frame length			*/
	} xmit_cb_t;

/* MC-SETUP CB */
typedef struct
	{
	cb_hdr_t cb_mcs_hdr;		/* Standard CB header		*/
	uint	 cb_mcs_cnt;		/* # of m-cast addresss, zero to*/
					/*   disable m-cast reception.	*/
	uchar	 cb_mcs_addr[ADDRLEN];	/* List of m-cast addresses,	*/
					/*   additional entries, each of*/
					/*   size ADDRLEN may be defined*/
	} mcs_cb_t;

/* CONFIGURE CB */
/* First define an internal structure in which the various config	*/
/* parameters will be placed.						*/
typedef struct
	{
	uchar   cb_config_6;	/* Byte 6 -- byte count			*/
	uchar   cb_config_7;	/* Byte 7 -- fifo limit			*/
	uchar   cb_config_8;	/* Byte 8				*/
	uchar   cb_config_9;	/* Byte 9				*/
	uchar   cb_config_10;	/* Byte 10				*/
	uchar   cb_config_11;	/* Byte 11 -- interframe spacing	*/
	uchar   cb_config_12;	/* Byte 12 -- slot time	(low)		*/
	uchar   cb_config_13;	/* Byte 13 -- retries and slot time (hi)*/
	uchar   cb_config_14;	/* Byte 14				*/
	uchar   cb_config_15;	/* Byte 15				*/
	uchar	cb_config_16;	/* Byte 16 -- min frame length		*/
	uchar	cb_config_pad;	/* unused				*/
	} config_block_t;

/* Now for the actual configure CB */
typedef struct
	{
	cb_hdr_t	cb_config_hdr;	/* Standard CB header		*/
	config_block_t	cb_config_blk;	/* The actual config parms.	*/
	} config_cb_t;

/* General form of command block. */
typedef struct
	{
	union {
	      cb_hdr_t 		cb_hdr;
	      dump_cb_t		dump_cb;
	      ias_cb_t		ias_cb;
	      tdr_cb_t		tdr_cb;
	      xmit_cb_t		xmit_cb;
	      mcs_cb_t		mcs_cb;
	      config_cb_t	config_cb;
	      } cb_forms;
	} cb_t;

/* TRANSMIT BUFFER DESCRIPTOR */
typedef struct
	{
	uint		 tbd_act_cnt;	/* Number of bytes in buffer	   */
	uint		 tbd_link;	/* Link to next tbd		   */
	paddr_t		 tbd_buff;	/* Physical address of the buffer  */
	} tbd_t;

/* RECEIVE FRAME DESCRIPTOR (RFD) -- FULL FORM */
typedef struct
	{
	uint		rfd_stat;	/* Status word of fd		   */
	uint		rfd_el_s;	/* EL and S bits		   */
	uint		rfd_link;	/* Link to next fd		   */
	uint		rfd_rbd_offset;	/* Receive buffer descriptor	   */
					/*   offset			   */
	uchar		rfd_dest_addr[ADDRLEN]; /* Destination address	   */
	uchar		rfd_src_addr[ADDRLEN];	/* Source address	   */
	uint		rfd_length;	/* Length field			   */
	} rfd_t;

/* RECEIVE BUFFER DESCRIPTOR (RBD) */
typedef struct
	{
	uint		rbd_act_cnt;	/* Number of bytes in buffer	   */
	uint		rbd_link;	/* Link to next RBD		   */
	paddr_t 	rbd_buff;	/* Physical address of buffer	   */
	uint		rbd_size;	/* Bit 15 set indicates last RBD in*/
					/*   list, bits 0-13 indicate	   */
					/*   number of bytes in buffer	   */
	uint		rbd_buff_ioaddr;/* I/O port address to reach the   */
					/*   buffer.  This is a software-  */
					/*   field, not used by the 82586. */
    } rbd_t;

main(argc, argv)
	int argc;
	char **argv;
{
uint num_cb, num_tbd, num_rfd, num_rbd;
uint xmit_size, rcv_size, num_xmit, num_rcv;
int i;
unsigned long buf_paddr;
uint cb0_link_offset, tbd0_link_offset, rfd0_link_offset, rbd0_link_offset;
uint xmit_buffer_offset;

if (argc != 5)
   then {
	printf("Usage: tune #-CBs #RFD XMIT-SIZE RCV-SIZE\n");
	exit(0);
	}
num_cb = atoi(argv[1]);
num_rfd = atoi(argv[2]);
xmit_size = atoi(argv[3]);
rcv_size = atoi(argv[4]);
num_tbd = num_cb;
num_rbd = num_rfd;
num_xmit = num_tbd;
num_rcv = num_rbd;

printf("# CB: %u \t# TBD: %u\t# RFD: %u \t# RBD: %u\n",
	num_cb, num_tbd, num_rfd, num_rbd);
printf("# XMIT: %u \t# RCV: %u \tXMIT BUFF: %u \tRCV BUFF: %u\n",
	num_xmit, num_rcv, xmit_size, rcv_size);

cb0_link_offset = sizeof(scb_t);
tbd0_link_offset = cb0_link_offset + (sizeof(cb_t) * num_cb);
rfd0_link_offset = tbd0_link_offset + (sizeof(tbd_t) * num_tbd);
rbd0_link_offset = rfd0_link_offset + (sizeof(rfd_t) * num_rfd);
xmit_buffer_offset = rbd0_link_offset + (sizeof(rbd_t) * num_rbd);
printf("CB0_LINK_OFFSET: %u (%x) \tport: %x\n",
	cb0_link_offset, cb0_link_offset, cb0_link_offset >> 1);
printf("TBD0_LINK_OFFSET: %u (%x) \tport: %x\n",
	tbd0_link_offset, tbd0_link_offset, tbd0_link_offset >> 1);
printf("RFD0_LINK_OFFSET: %u (%x) \tport: %x\n",
	rfd0_link_offset, rfd0_link_offset, rfd0_link_offset >> 1);
printf("RBD0_LINK_OFFSET: %u (%x) \tport: %x\n",
	rbd0_link_offset, rbd0_link_offset, rbd0_link_offset >> 1);

buf_paddr = xmit_buffer_offset;
printf("XMT_BUFF: hi_8: %x, low_16: %x, port: %x\n",
        (uint)(buf_paddr > 0x0000FFFFL ? 0xFF : 0xFE),
	(uint)buf_paddr,
        (uint)(buf_paddr >> 1));

for (i=0; i<num_tbd; i++) buf_paddr += xmit_size;
printf("RCV_BUFF: hi_8: %x, low_16: %x, port: %x\n",
        (uint)(buf_paddr > 0x0000FFFFL ? 0xFF : 0xFE),
	(uint)buf_paddr,
        (uint)(buf_paddr >> 1));

for (i=0; i<num_rbd; i++) buf_paddr += rcv_size;
printf("END-BUFF: hi_8: %x, low_16: %x, port: %x\n",
        (uint)(buf_paddr > 0x0000FFFFL ? 0xFF : 0xFE),
	(uint)buf_paddr,
        (uint)(buf_paddr >> 1));

if (buf_paddr > 0x0001FFE0L)
   then printf("BUFFERS OVERLAY ISCP\n");

if (buf_paddr > 0x0001FFFFL)
   then printf("TOO MANY BUFFERS\n");
}
