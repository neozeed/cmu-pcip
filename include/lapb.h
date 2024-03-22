/* Copyright 1986 by Carnegie Mellon */

/*
 * Information common to all LAPB drivers.
 */

#define	LOOPBACK	0x01	/* send packets in loopback mode */

/* HDLC/LAPB packet header */

struct lapb_hhdr {
	unsigned char lapb_address;
	unsigned char lapb_control;
	unsigned lapb_type;
};

/*
 * LAPB control field encoding.
 */
#define LAPB_I_S_MASK	0x80	/* Information/Supervisory Frame Mask */
#define LAPB_I_FRAME	0x00	/* Information Transfer Frame */
#define LAPB_S_U_MASK	0xc0	/* Supervisory Frame Mask */
#define LAPB_S_FRAME	0x80	/* Supervisory Frame */
#define LAPB_U_FRAME	0xc0	/* Unnumbered Frame */
#define LAPB_P_F_BIT	0x08	/* Poll/Final Bit */
#define LAPB_N_S_FIELD	0x70	/* N(S) Field Mask */
#define LAPB_N_R_FIELD	0x07	/* N(R) Field Mask */

/*
 * Supervisory command encoding.
 */
#define LAPB_RR		0x80	/* Receive Ready */
#define LAPB_REJ	0x90	/* Reject */
#define LAPB_RNR	0xa0	/* Receive Not Ready */

/*
 * Unnumbered frame encoding.
 */
#define LAPB_DISC	0xc2	/* Disconnect */
#define LAPB_UA		0xc6	/* Unnumbered Acknowledge */
#define LAPB_DM		0xf0	/* Disconnect Mode */
#define LAPB_FRMR	0xe1	/* Frame Reject */
#define LAPB_SABM	0xf4	/* Set Async Balanced Mode */

/*
 * LAPB States.
 */
typedef enum lapb_state {
	ls_init,		/* Initial unconnected state */
	ls_con_active,		/* Connecting active */
	ls_con_passive,		/* Connecting passive */
	ls_connected,		/* Connection established */
	ls_prel_disc,		/* Preliminary disconnecting */
	ls_frmr,		/* Frame reject */
	ls_disc_active,		/* Disconnecting active */
	ls_disc_passive		/* Disconnecting passive */
} lapb_state;

/*
 * LAPB Connection Block (State Variables).
 */
typedef struct lapb_cb {
	lapb_state lc_state;	/* Connection State */
	int lc_NS;		/* Next Send State Variable */
	int lc_NR;		/* Next Receive State Variable */
	Task lc_waiter;		/* Task waiting on event */
	char lc_hdlc_addr;	/* HDLC cmd. address - B => DTE, A => DCE */
#define LAPB_ADDR_A	3
#define LAPB_ADDR_B	1
	char lc_tx_flags;	/* Transmit flags */
#define LAPB_REM_BUSY	1	/* Remote end sent RNR */
#define LAPB_OACTIVE	2	/* Output active */
	char lc_rx_flags;	/* Receive flags */
#define LAPB_LOC_BUSY	1	/* Local end sent RNR */
#define LAPB_LOC_REJ	1	/* Local end sent FRMR */

	/*
	 * These variables indicate type of frame that needs to be sent.
	 */
	int s_cmd;		/* Supervisory commands */
	int s_resp;		/* Supervisory responses */
	int u_cmd;		/* Unnumbered commands */
	int u_resp;		/* Unnumbered responses */
} lapb_cb;

/*
 * LAPB System Parameters.
 */
extern int lapb_K;		/* Maximum Number of I-Frames Outstanding */
extern int lapb_N1;		/* Maximum Number of Bits in an I Frame */
extern int lapb_N2;		/* Maximum Number of Retransmissions */
extern int lapb_T1;		/* HDLC Packet Timeout */
extern int lapb_T2;		/* Inactivity Timeout */
extern int lapb_T3;		/* Disconnection Timeout */


/*
 * LAPB packet types: not all of these are really used.
 *	These packet types are ALREADY BYTESWAPPED.
 */
#define	LAPB_IP		0x0100	/* really 0x0001 */

#define LAPB_BROADCAST	0xff

#define	LAPB_MINLEN	sizeof(struct lapb_hhdr)
