/* Copyright 1986 by Carnegie Mellon */
/*  Copyright 1983,1984,1985 by the Massachusetts Institute of Technology  */

/*
3-29-86	added c_chirp{f,d,l,s} fields to allow control over the telnet chirp.
						<Drew D. Perkins>
 */

#ifndef CUSTOM_H		/* DDP */
#define CUSTOM_H	1	/* DDP */

/* Still 8 after all these years...
*/

/* This file contains lots of customization info necessary to work with the
	system. */

/* some random constants */
#define	MAXNAMES	5	/* maximum number of name servers */
#define	MAXTIMES	5	/* maximum number of time servers */
#define	MNAMELEN	50	/* maximum user name length */
#define	MPROGLEN	20	/* maximum program name length */

/* Custom structure version number */
#define	CVERSION	8


/* some constants */
#define	SUBNET		0
#define	GENERIC		1

/* how to figure out my local net address */
#define	HARDWARE	0	/* use hardware address */
#define	ETINTERNET	1	/* use my ip address */
#define	ETUSER		2	/* use the user specified address */

/* ethernet address structure */
struct etha {
	char	e_ether[6];
	};

/* The custom structure itself. */

struct custom {
	char		c_dummy[18];	/* 18 bytes of padding */
#define c_basemem c_pad
	long		c_pad;		/* padding for old programs */
	unsigned	c_iver;		/* internal version number */
	long		c_ctime;	/* time of last customization */
	long		c_cdate;	/* date of last customization */
	unsigned	c_baud;		/* serial line speed */
	unsigned	c_driver;	/* interface type */
	unsigned	c_debug;	/* default debugging options */
	int		c_tmoffset;	/* time zone offset in minutes */
	char		c_tmlabel[4];	/* time zone label */
	unsigned	c_1custom;	/* bits mean things for each program */
	unsigned 	c_route;	/* routing option */
	unsigned	c_seletaddr;	/* how to choose my et addr */
	struct	etha	c_myetaddr;	/* my ethernet address */
	in_name		c_me;		/* skeletal my-internet-address */
	in_name		c_log;		/* address of log server */
	in_name		c_defgw;	/* address of default gateway */
	in_name		c_cookie;	/* address of cookie server */
	in_name		c_printer;	/* address of print server */
	in_name		c_scribe;	/* address of scribe server */
	unsigned	c_numtime;	/* number of time servers */
	in_name		c_time[MAXTIMES];	/* local time server */
	unsigned	c_numname;	/* # of name servers */
	in_name		c_names[2];	/* and the name servers */
	in_name		c_dm_servers[3];
	char		c_user[MNAMELEN];	/* user name */
	char		c_domain[30];	/* domain name: LCS.MIT.EDU */
	unsigned	c_dm_numname;	/* number of domain name servers */
	char		c_spare[38];
	struct	etha	c_ether[3];	/* ip - ether address xlation cache */
	in_name		c_ipname[3];
	unsigned	c_intvec;	/* net interface interrupt number */
	unsigned	c_tx_dma;	/* net interface xmit dma channel */
	unsigned	c_base;		/* net interface CSR base address */
	unsigned	c_telwin;	/* telnet window size */
	unsigned	c_tellowwin;	/* telnet low window size */
	unsigned	c_subnet_bits;	/* # of bits of subnet */
	in_name		c_net_mask;	/* net & subnet address mask */
	unsigned	c_rcv_dma;	/* net interface rcv dma channel */
	unsigned	c_ip_radix;	/* print ip addrs in base 8 or 10 */
	unsigned	c_chirpf;	/* DDP start count for chirp */
	int		c_chirpd;	/* DDP delta for each chirp segment */
	unsigned	c_chirpl;	/* DDP length of a chirp segment */
	unsigned	c_chirps;	/* DDP number of chirp segments */
	unsigned	c_vars[1];
	unsigned	c_otheruser;	/* 1 if rvd active */
	unsigned	c_num_nets;	/* number of interfaces */
	char		c_rvd_base;	/* first rvd drive */
	};

/* TELNET custom bits in c_1custom */
#define	BSDEL	0x01	/* BS/DEL key setting */
#define	WRAP	0x02	/* wrap-around */
#define	NLSET	0x04	/* newline mode default */
#define	FINGER_ON	0x08	/* finger server default */
#define	SUPDUP_WRAP	0x10	/* supdup wrap default */
#define	TN_TFTP_ASK	0x20	/* telnet tftp server asking default */

/* in multiple interface configurations, this will be the first custom
    structure
*/
extern struct custom custom;
#endif				/* DDP */
