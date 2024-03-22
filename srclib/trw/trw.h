/* Copyright 1988 TRW Information Networks */
/*  See permission and disclaimer notice in file "trw-note.h"  */
#include	"trw-note.h"

/* $Revision:   1.2  $		$Date:   22 Mar 1988 22:39:12  $	*/

#ifndef NO_EXT_KEYS /* extensions enabled */
    #define _CDECL  cdecl
    #define _NEAR   near
#else /* extensions not enabled */
    #define _CDECL
    #define _NEAR
#endif /* NO_EXT_KEYS */

#if (!defined(trw_inc))
#define trw_inc 1

#pragma loop_opt(off)
#pragma pack(1)

#include <conio.h>
#pragma	 intrinsic(outp, outpw, inp, inpw)
#include <string.h>
#pragma  intrinsic(memcmp, memcpy, memset)


/* The following declarations are here because there is a significant	*/
/* collision between ../../include/dos.h and /include/dos.h		*/
void _CDECL _disable(void);
void _CDECL _enable(void);
#pragma  intrinsic(_enable, _disable)

#if (!defined(trw_type))
#include "trw_type.h"
#endif

#if (!defined(trw_conf))
#include "trw_config.h"
#endif

#if (!defined(trw_res))
#include "trw_res.h"
#endif

/*	TRW PC-2000 Ethernet board
 *
 *  Jumper at E6 blocks or unblocks heartbeat (SQE)
 *
 *  For AUI interface (15 pin connector) jumper group E3/E4/E5/E7 should
 *  have all seven jumpers across E4-E5.
 *  
 *  For Cheapernet interface (BNC connector) jumper group E3/E4/E5/E7 should
 *  have all seven jumpers across E3-E4.
 *  
 *  Jumpers on E2 determine the base address of the IO ports:
 *
 *  PIN 1-2 3-4 5-6	BASE ADDRESS
 *	 0   0	 0	0x220
 *	 0   0	 1	0x228
 *	 0   1	 0	0x230
 *	 0   1	 1	0x238
 *	 1   0	 0	0x240
 *	 1   0	 1	0x248
 *	 1   1	 0	0x250
 *	 1   1	 1	illegal
 *
 *  BASE+0  R/W	  Data Register 0, D0 - D7 memory array access
 *  BASE+1  R/W	  Data Register 1, D8 - D15 memory array access
 *		  NOTE: ID Prom may be read only by INB instructions
 *		  through BASE+1.
 *		  NOTE: BASE+0 and BASE+1 may be read or written
 *		  together by single word INB or OUTW instruction.
 *  BASE+2  R/W	  Address Register 0, D0 - D7 determine A1 - A8
 *		  address of memory array	
 *  BASE+3  R/W	  Address Register 1, D8 - D15 determine A9 - A16
 *		  address of memory array
 *		  NOTE: BASE+2 and BASE+3 may be read or written
 *		  together by single word INB or OUTW instruction.
 *		  NOTE: Access to BASE+1 causes address register pair
 *		  to be incremented by two.
 *  BASE+4  R/W	  Control Register (all bits set to 0 after reset)
 *		  D0,D1 PCBS -	Memory Bank Select
 *				D1    D0    MA		IOA
 *				--    --   -----    ----------
 *				0     0	   Bank 0    Bank 0 , 1
 *				0     1	   Bank 1    Bank 0 , 1
 *				1     0	   Bank 2    Bank 2 , 3
 *				1     1	   Bank 3    Bank 2 , 3
 *				MA - Memory Cycle Access
 *				IOA -  IO Cycle Access
 *				Bank 0 and Bank 1 invalid selection if
 *				only 128k memory installed.
 *		  D2	IRQRST* Interrupt Request Reset.
 *				Resets interrupt request register when
 *				low, must be reset by next instruction.
 *		  D3	RDID*	Read I D
 *				Enables Ethernet ID Prom, to be 
 *				read through IO port read mechanism.
 *				High setting enables normal I/O port
 *				reads.
 *		  D4	CHATTN	Channel Attention
 *				Activate 80186 INT1 input if 80186 present.
 *				If no 81086, activates 82586 Channel
 *				Attention input. Must be reset at next
 *				instruction.
 *		  D5	CHRST*	Channel Reset.
 *				Resets both 82586 and 80186 when low.
 *		  D6	MEMEN	Memory Mapping Enable.
 *				Board Memory may be accessed via memory
 *				reads/writes from the PC Bus when set
 *				high.
 *		  D7	LOOP	Ethernet Loopback Enable
 *				Set 82501 to Loopback mode.
 *  BASE+4   R	Status Register 
 *		  D0,D1		Read Value of Control Register bit 0 and 1
 *				indicating memory bank selection.
 *		  D2	CPUP	80186 Present when true.
 *		  D3-D5		Reserved
 *		  D6	MEMEN	Read value of Control Register
 *				bit D6 Memory Map Enable.
 *		  D7	LOOP	Read Value of Control Register
 *				bit D7 Ethernet Loopback Enable.
 *
 *  D0 is the low order bit of an octet, D7 is the high order bit
 *  D8 and D15 are the low and high order bits of a high order byte.
 */

#define BRD_PCBS_MASK		0x03
#define BRD_PCBS_BANK0		0x00
#define BRD_PCBS_BANK1		0x01
#define BRD_PCBS_BANK2		0x02
#define BRD_PCBS_BANK3		0x03

#define BRD_DATAPORT0		0x00
#define BRD_DATAPORT1		0x01
#define BRD_ADDRPORT0		0x02
#define BRD_ADDRPORT1		0x03
#define BRD_CTRLPORT		0x04
#define BRD_STATUSPORT		0x04
#define BRD_SEGBASE	0xD000	/* Segment to view board memory, if board */
				/*   memory is made visible.		  */

/* Define things that go on in the control and status port(s)...	  */
#define BRD_IRQRST	0x04	/* When LOW - blocks/clears interrupts.	  */
				/*   i.e. flag should normally be set.	  */
#define BRD_RDID	0x08	/* When LOW - enables ID PROM to be read  */
				/*   via the dataports, when HIGH, the	  */
				/*   board memory may be read via	  */
				/*   the dataports.			  */
#define BRD_CHATTN	0x10	/* INT1 to board's 80186 or Channel Attn  */
				/*   to board's 82586 if no 80186 present.*/
				/*   Signal is presented to the 186 or 586*/
				/*   for as long as this bet is set.	  */
#define BRD_CHRST	0x20	/* When LOW - Reinitializes the 186 or 586*/
				/*   The reset signal is presented to the */
				/*   186 or 586 for as long as this bet is*/
				/*   set.				  */
#define BRD_MEMEN	0x40	/* Turn on memory mapping -- board memory */
				/*   is visible at segment 0xD000	  */
#define BRD_LOOP	0x80	/* Turns on ethernet loopback		  */
#define BRD_CPU_FLAG	0x04	/* On read: if set indicates 80186 present*/

/* Define the 32 bit PROM ID */
typedef struct
	{
	uchar	space1;		/* A blank character			*/
	uchar	trw[3];		/* Should be 'TRW' in ASCII		*/
	uchar	space2;		/* A blank character			*/
	uchar	ind[3];		/* Should be 'IND' in ASCII		*/
	uchar	space3;		/* A blank character			*/
	uchar	brd_addr[6];	/* 1st 3 octets should be 0x00 0x00 0x2A*/
	uchar	space4;		/* Just another blank			*/
	uchar	assembly_num[8];/* Assembly # in ASCII '03-00269'	*/
	uchar	space5;		/* Wow, another blank!			*/
	uchar	reserved[3];	/* Reservations for three.		*/
	uchar	compatability;	/* Compatability level of board.	*/
				/* Software should be >= to this.	*/
	uchar	configuration;	/* Configuration table selector, only	*/
				/* low 3 bits are presently in use, rest*/
				/* ought to be zero.			*/
	uchar	chksum_lo;	/* The checksum is a 16 bit 2-s comp.	*/
	uchar	chksum_hi;	/* of this structure excepting these two*/
				/* checksum octets.			*/
	} id_prom_t;
/* Decoding for bits in the configuration byte */
#define BOARD_CONFIG_MASK		0x07
#define	BOARD_RATE_1_MBIT		0x06
#define	BOARD_RATE_2_MBIT		0x04
#define	BOARD_RATE_5_MBIT		0x02
#define	BOARD_RATE_10_MBIT		0x00
#define BOARD_BROADBAND			0x01
#define BOARD_BASEBAND			0x00

/* Define the Intel 82586 LAN Controller */
/* The 82586 requires all structures to be on even byte boundaries */

/* NOTE: ALL "OFFSETS" ARE RELATIVE TO THE CONTROL BLOCK BASE DEFINED	*/
/*	 IN iscp_scb_base.						*/


/* SYSTEM CONFIGURATION POINTER (SCP) */
/* Must live at address 0xFFFF6 in the 82586 address space */
typedef struct
	{
	uchar	scp_bussize;	/* used to indicate bus size	*/
	uchar	scp_unused[5];	
	paddr_t scp_iscpptr;	/* Absolute address of to iscp	*/
	} scp_t;

/* Bits in scp_bussize */
#define SCP_BUS_16_BIT	0	/* value to indicate 16 bit bus */
#define SCP_BUS_8_BIT	1	/* value to indicate 8 bit bus	*/


/* INTERMEDIATE SYSTEM CONFIGURATION POINTER (ISCP) */
typedef struct
	{
	uchar	iscp_busy;	 /* 0x01 while 82586 is initializing	 */
	uchar	iscp_unused;	 
	uint	iscp_scb_offset; /* offset to scb from base at iscp_base */
	paddr_t iscp_scb_base;	 /* address 64k segment containing scb	 */
	} iscp_t;


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

/* Bits in scb_status */
#define CX	0x8000		/* CU finished executing an action	*/
				/*  command with 'I' interupt bit on	*/
#define FR	0x4000		/* RU finished receivng a frame		*/
#define CNA	0x2000		/* CU left the active state		*/
#define RNR	0x1000		/* RU left the ready state		*/
#define CUS_MASK	 0x0700	  /* Status of Command Unit...		*/
#define CUS_IDLE	 0x0000	    /* idle		*/
#define CUS_SUSPENDED	 0x0100	    /* suspended	*/
#define CUS_ACTIVE	 0x0200	    /* active		*/
#define RUS_MASK	 0x0070	  /* Status of Receive Unit...	*/
#define RUS_IDLE	 0x0000	    /* idle		    */
#define RUS_SUSPEND	 0x0010	    /* suspended	    */
#define RUS_NO_RESOURCES 0x0020	    /* ran out of resources */
#define RUS_READY	 0x0040	    /* ready		    */

/* Bits in scb_command */
#define ACK_MASK	0xF000	      /* Interupt ack mask		  */
#define ACK_CX		0x8000		/* Ack completion of an action	  */
					/* command by the CU		  */
#define ACK_FR		0x4000		/* Ack reception of a frame by RU */
#define ACK_CNA		0x2000		/* Ack that CU has left active	  */
					/*   state.			  */
#define ACK_RNR		0x1000		/* Acks that the receiver left	  */
					/*   ready state		  */
#define CUC_MASK	0x0700	      /* CU command field		  */
#define CUC_NOP		0x0000		/* No operation			  */
#define CUC_START	0x0100		/* Start execution of the first	  */
					/*   command on the command list  */
#define CUC_RESUME	0x0200		/* Resume the operation of the cu */
					/*   by executing next command.	  */
#define CUC_SUSPEND	0x0300		/* Suspend operation of the cu	  */
					/*   after current command is done*/
#define CUC_ABORT	0x0400		/* Abort current command	  */
#define RUC_MASK	0x0070	      /* RU command field		  */
#define RUC_NOP		0x0000		/* No operation			  */
#define RUC_START	0x0010		/* Start reception of frames	  */
#define RUC_RESUME	0x0020		/* Resume frame reception	  */
#define RUC_SUSPEND	0x0030		/* Suspend frame reception	  */
#define RUC_ABORT	0x0040		/* Abort frame reception	  */
#define RESET_586	0x0008		/* reset the 82586		  */

/* CU COMMAND BLOCKS */
/* Header found on all command blocks */
typedef struct
	{
	uint	cb_stat;	/* Status of command		*/
	uint	cb_command;	/* Command			*/
	uint	cb_link;	/* Link to next command		*/
	} cb_hdr_t;

/* Bits in cb_stat (some CBs have additional bits) */
#define CMD_COMPLETED	0x8000	/* Command completed	  */
#define CMD_EXECUTING	0x4000	/* Busy executing command */
#define CMD_COMPLETE_OK 0x2000	/* Command completed ok	  */
#define CMD_ABORT	0x1000	/* Command was aborted	  */

/* Bits in cb_command */
#define CB_END_LIST	0x8000	/* End of command list			*/
#define CB_SUSPEND	0x4000	/* Suspend after completion		*/
#define CB_INTERRUPT	0x2000	/* Interupt atfer completion		*/
#define CB_COMMAND_MASK 0x0007	/* Command field			*/
#define CB_NOP		0x0000	/* Bo operation				*/
#define CB_IA_SETUP	0x0001	/* Setup individual address		*/
#define CB_CONFIGURE	0x0002	/* Configure 586			*/
#define CB_MC_SETUP	0x0003	/* Setup multicast hash table		*/
#define CB_TRANSMIT	0x0004	/* Transmit frame, retransmit on colls	*/
#define CB_TDR		0x0005	/* Time domain reflectometry test	*/
#define CB_DUMP		0x0006	/* Dump internal registers to memory	*/
#define CB_DIAGNOSE	0x0007	/* Internal test			*/

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
/* Definitions for cb_tdr_result */
#define TDR_TIME_MASK		0x03FF
#define TDR_PROBLEM_MASK	0x7000
#define TDR_LINK_OK		0x8000
#define TDR_XCVR_PROB		0x4000
#define TDR_CABLE_OPEN		0x2000
#define TDR_CABLE_SHORT		0x1000

/* TRANSMIT CB */
/* Note: this CB has a resource allocation element appended after the	*/
/* part that is recognized by the 82586 hardware.			*/
typedef struct
	{
	cb_hdr_t	cb_tran_hdr;	/* Standard CB header		*/
	uint	 	cb_tran_tbd;	/* Offset - 1st transmit buffer */
					/*   descriptor			*/
	uchar	 cb_tran_mac[ADDRLEN];	/* Destination MAC address	*/
	uint	 	cb_tran_length;	/* Frame length			*/
	} xmit_cb_t;
/* Status bits found in cb_tran_hdr.cb_stat	*/
#define XSTAT_LOST_CD		0x0400	/* Lost carrier detect		*/
#define XSTAT_LOST_CTS		0x0200	/* Lost clear-to-send		*/
#define XSTAT_UNDERRUN		0x0100	/* DMA underrun			*/
#define XSTAT_DEFERRED		0x0080	/* Deferred to other traffic	*/
#define XSTAT_SQE		0x0040	/* SQE test present		*/
#define XSTAT_MAX_COLLISION     0x0020	/* Too many collisions		*/
#define XSTAT_COLLISION_MASK	0x000F	/* # collisions experienced	*/

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

/* Bits in tbc_act_cnt */
#define TBD_END_OF_LIST 0x8000

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
/* Bits in rfd_stat */
#define RFD_C		0x8000
#define RFD_B		0x4000
#define RFD_OK		0x2000
#define RFD_CRC_ERR	0x0800
#define RFD_ALIGN_ERR	0x0400
#define RFD_RES_ERR	0x0200
#define RFD_OVERRUN	0x0100
#define RFD_SHORT	0x0080
#define RFD_NO_EOF	0x0040

/* Bits in rfd_el_s */
#define RFD_EL		0x8000
#define RFD_S		0x4000

/* RECEIVE FRAME DESCRIPTOR (RFD) -- SHORT FORM */
typedef struct
	{
	uint		rfd_stat;	/* Status word of fd		   */
	uint		rfd_el_s;	/* EL and S bits		   */
	uint		rfd_link;	/* Link to next fd		   */
	uint		rfd_rbd_offset;	/* Receive buffer descriptor	   */
					/*   offset			   */
	} short_rfd_t;

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
/* Bits and fields in rbd_act_cnt */
#define RBD_EOF			0x8000	/* Last buffer of a frame	   */
#define RBD_F			0x4000	/* rbd_act_cnt is valid		   */
#define RBD_COUNT_MASK		0x3FFF  /* Length part of rbd_act_cnt and  */
					/* rbd_buff			   */

/* Bits in rbd_size */
#define RBD_EL			0x8000	/* Last RBD in list		   */

/* Externally visible entry points */
extern void trw_init(NET *, unsigned int, unsigned int);
extern unsigned int trw_send(PACKET, unsigned int, unsigned int, ...);
extern void kick_586(void);
extern void trw_switch(int, unsigned int);
extern void trw_stat(FILE *);
extern void trw_close(void);
extern void trw_demux(void);
extern void trw_ihnd(void);
extern void et_patch(unsigned int);
extern void et_unpatch(void);
extern void trw_eoi_int(void);
extern void portin_b(unsigned int, unsigned char *, unsigned int);
extern void portin_w(unsigned int, unsigned char *, unsigned int);
extern void portout_w(unsigned int, unsigned char *, unsigned int);
extern void init_board_memory(void);
extern int  i586_config(unsigned int);
extern void brd_error(int);

extern long	cticks;		/* Timer ticks in 1/18 second units */
extern NET	*et_net;	/* ET's net */
extern task	*EtDemux;	/* ET's demultiplexing task */
extern uchar	_etme[6];	/* Our Ethernet/MAC address */
extern uchar	mask_saved;
extern uchar	save_mask;	/* 8259 mask to be restored on close */
extern uchar	cpu_is_286;	/* 0 if 8088, 1 if 80286	     */
extern uchar	control_image;	/* Our in-core version of the board control */
				/*   register */
extern id_prom_t board_id;	/* A copy of the board's ID PROM */
extern uchar	driver_state;	/* State of this driver */
#define	DS_UNINITIALIZED	0
#define	DS_INITIALIZED		1
extern uchar	int_flag;	/* Used to indicate whether an int happened */

extern cb_list_el_t	*current_cb;	/* Currently executing CB */
					/* CBP_NULL, if none	  */

extern char	ETBROADCAST[6];

/* Define the driver statistics counters */
extern uint int_cnt;
extern uint rcv_cnt;
extern uint send_cnt;
extern uint cd_lost_cnt;
extern uint cts_lost_cnt;
extern uint xmit_dma_under_cnt;
extern uint deferred_cnt;
extern uint sqe_lost_cnt;
extern uint collision_cnt;
extern uint ex_coll_cnt;
extern uint short_cnt;
extern uint long_cnt;
extern uint skipped_cnt;
extern uint eof_missing_cnt;
extern uint toobig_cnt;
extern uint refused_cnt;
extern uint etdrop;
extern uint etmulti;
extern uint etwpp;

/* The ethernet address copying function */
#define etadcpy(S,D)	(((int *)(D))[0]=((int *)(S))[0], \
			 ((int *)(D))[1]=((int *)(S))[1], \
			 ((int *)(D))[2]=((int *)(S))[2])

/* Miscellaneous definitions */
extern int dataport0, dataport1, addrport0, addrport1, ctrlport;
#define REG_BASE	custom.c_base
#define DATAPORT0	dataport0
#define DATAPORT1	dataport1
#define ADDRPORT0	addrport0
#define ADDRPORT1	addrport1
#define CTRLPORT	ctrlport
#define STATUSPORT	ctrlport

/* The next/prior variables are used to keep track of what	*/
/* blocks are pending or in use on the RFD and RBD lists.	*/
extern uint	next_rfd, prior_rfd, next_rbd, prior_rbd;

/* The following two variables are used by the assembly language interrupt */
/* handler to acknowledge the first and second interrupt controllers.	   */
/* (If "trw_eo1_2" is set to zero if only the first int controller is used.*/
extern uchar	trw_eoi_1, trw_eoi_2;

/* Layout the memory of the board.					*/
/* We have 128K to use, the top two banks of board memory.		*/
/* The various control blocks (except the SCP and ISCP) are placed in	*/
/* the lower bank.  The buffers take up most of the remaining memory,	*/
/* leaving room for the SCP/ISCP at the top of the upper bank.		*/
/* From the perspective of the 82586, the two banks of memory are	*/
/* replicated at base address 0xA0000 and 0xE0000.  We use 0xE0000.	*/
/* Thus the segment part of addresses used by the 82586 is 0xE000.	*/

/* The following definitions give the value to be used in the board's	*/
/* ADDRPORTs to reach a block (or collection of blocks).		*/

/* The SCP goes at a fixed location in the upper bank. We place the ISCP*/
/* at an arbitrary location a bit below the SCP.			*/
#define SCP_PORT_ADDR		(0xFFFF6 >> 1)
#define ISCP_PORT_ADDR		(0xFFFE0 >> 1)
#define ISCP_PHYS_ADDR_LO_16	0xFFE0
#define ISCP_PHYS_ADDR_HI_8	0xFF
#define SCB_BASE_LO_16		0x0000
#define SCB_BASE_HI_8		0xFE

/* Control Blocks in the lower bank...	*/
/* The SCB is at the bottom, followed by the pool of CBs,	*/
/* the pool of TBDs, the pool of RFDs, the pool of RFDs, then	*/
/* the transmit buffers followed by the receive buffers.	*/
#define SCB_OFFSET	      0
#define SCB_PORT_ADDR	      0
#define CB0_LINK_OFFSET	      (sizeof(scb_t))
#define TBD0_LINK_OFFSET      (CB0_LINK_OFFSET + (sizeof(cb_t)*NUM_CB))
#define RFD0_LINK_OFFSET      (TBD0_LINK_OFFSET + (sizeof(tbd_t)*NUM_TBD))
#define RFDX_LINK_OFFSET      (RFD0_LINK_OFFSET + (sizeof(rfd_t)*(NUM_RFD-1)))
#define RBD0_LINK_OFFSET      (RFD0_LINK_OFFSET + (sizeof(rfd_t)*NUM_RFD))
#define RBDX_LINK_OFFSET      (RBD0_LINK_OFFSET + (sizeof(rbd_t)*(NUM_RBD-1)))
#define XMIT_BUFFER_OFFSET    (RBD0_LINK_OFFSET + (sizeof(rbd_t)*NUM_RBD))

/* Board and 586 manipulation macros */
#define block_in(A,B,S)  {outpw(ADDRPORT0,(A));	\
			  portin_w(DATAPORT0,(unsigned char *)(B),(S)>>1);}
#define block_out(A,B,S) {outpw(ADDRPORT0,(A));	\
			  portout_w(DATAPORT0,(unsigned char *)(B),(S)>>1);}

#define CH_ATTENTION()	{outp(CTRLPORT, control_image|BRD_CHATTN);	\
			 outp(CTRLPORT, control_image);}

#define HALT_BOARD()	{outp(CTRLPORT,0); control_image=0;}

/* The following macro assumes that BRD_IRQRST is on in "control_image" */
#define IRQ_RESET()	{outp(CTRLPORT, control_image & ~BRD_IRQRST);	\
			 outp(CTRLPORT, control_image);}

#define get_scb(S)	block_in(SCB_PORT_ADDR, S, sizeof(scb_t))

#define get_scb_status() (outpw(ADDRPORT0,SCB_PORT_ADDR),inpw(DATAPORT0))

#define command_scb(C)	{outpw(ADDRPORT0,SCB_PORT_ADDR+1);	\
			 outpw(DATAPORT0,(C)); CH_ATTENTION();}

#define start_cb(C)	{outpw(ADDRPORT0,SCB_PORT_ADDR+1);	\
			 outpw(DATAPORT0, CUC_START);		\
			 outpw(DATAPORT0,(C)); CH_ATTENTION();}

#define start_rfd(F)	{outpw(ADDRPORT0, SCB_PORT_ADDR+1);	\
			 outpw(DATAPORT0, RUC_START);		\
			 outpw(ADDRPORT0, SCB_PORT_ADDR+3);	\
			 outpw(DATAPORT0, (F)); CH_ATTENTION();}

#define spin_on_scb_command() while(outpw(ADDRPORT0,SCB_PORT_ADDR+1),	\
				    inpw(DATAPORT0));

/* Define the brd_error() codes	*/
#define ERR_SETUPFAIL			1
#define ERR_BAD_BRD_TYPE		2
#define ERR_NEW_BOARD			3
#define ERR_INTELLIGENT			4
#define ERR_BAD_MEMORY			5
#define ERR_INITIATE_FAIL		6
#define ERR_BAD_INITIATE		7
#define ERR_CONFIG_FAIL			8
#define ERR_CABLE_PROB			9
#define ERR_OPEN_CABLE			10
#define ERR_CABLE_SHORT			11
#define ERR_INDETERMINATE		12
#define ERR_SOFTWARE			13
#define ERR_CONFIG_OR_CABLE		14
#define ERR_KEEPALIVE_FAIL		15
#define ERR_IRQ_TOO_HI			16
#define ERR_BAD_ADDRTYPE		17

#endif
