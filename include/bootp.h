/*  Copyright 1986 by Carnegie Mellon  */
/*  See permission and disclaimer notice in file "cmu-note.h"  */

/*
 * Bootstrap Protocol (BOOTP).  RFC 951.
 */

#define BOOTREPLY	2
#define BOOTREQUEST	1
struct bootp {
	unsigned char	bp_op;		/* packet opcode type */
	unsigned char	bp_htype;	/* hardware addr type */
	unsigned char	bp_hlen;	/* hardware addr length */
	unsigned char	bp_hops;	/* gateway hops */
	unsigned long	bp_xid;		/* transaction ID */
	unsigned short	bp_secs;	/* seconds since boot began */	
	unsigned short	bp_unused;
	in_name	bp_ciaddr;	/* client IP address */
	in_name	bp_yiaddr;	/* 'your' IP address */
	in_name	bp_siaddr;	/* server IP address */
	in_name	bp_giaddr;	/* gateway IP address */
	unsigned char	bp_chaddr[16];	/* client hardware address */
	unsigned char	bp_sname[64];	/* server host name */
	unsigned char	bp_file[128];	/* boot file name */
	unsigned char	bp_vend[64];	/* vendor-specific area */
};

/*
 * UDP port numbers, server and client.
 */
#define	IPPORT_BOOTPS		67
#define	IPPORT_BOOTPC		68

/*
 * "vendor" data permitted for CMU boot clients.
 */
struct vend {
	unsigned char	v_magic[4];	/* magic number */
	unsigned long	v_flags;	/* flags/opcodes, etc. */
	unsigned long 	v_smask;	/* Subnet mask */
	unsigned long 	v_dgate;	/* Default gateway */
	unsigned long	v_dns1, v_dns2; /* Domain name servers */
	unsigned long	v_ins1, v_ins2; /* IEN-116 name servers */
	unsigned long	v_ts1, v_ts2;	/* Time servers */
	unsigned char	v_unused[25];	/* currently unused */
};

#define	VM_CMU		"CMU"	/* v_magic for CMU */

/* v_flags values */
#define VF_SMASK	1	/* Subnet mask field contains valid data */
