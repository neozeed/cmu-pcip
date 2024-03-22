/*  Copyright 1986 by Carnegie Mellon  */
/*  See permission and disclaimer notice in file "cmu-note.h"  */
/*  Local coloring changes and 802.3 by Joe Doupnik, 29 Oct 89 */
/*  Banyan-Vines 31 March 90 jrd */
#include	<cmu-note.h>
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
#include <attrib.h>
#include <colors.h>
#include <h19.h>
#include <ctype.h>
#include <match.h>
#include "watch.h"

/* have to declare all this stuff up top because of forward references */

/* packet header unparsing functions */
int ip_unprs(), arp_unprs(), chaos_unprs();
int udp_unprs(), tcp_unprs(), icmp_unprs(), rvd_unprs();
int rfc_unprs(), et_unprs(), netblt_unprs();


/* protocol layer structures */
extern struct layer et_layer, ip_layer, udp_layer, tcp_layer, icmp_layer;
extern struct layer rvd_layer, arp_layer, chaos_layer, rfc_layer, netblt_layer;


/* arrays of protocol layer structures */
extern struct nameber et_prots[], ip_prots[], udp_prots[], tcp_prots[];
extern struct nameber icmp_prots[], rvd_types[], chaos_opcodes[];


/* filter setting functions */
extern int et_addr(), et_type();
extern int ip_addr(), ip_type();
extern int chaos_addr(), chaos_type();
extern int udp_type(), tcp_type(), icmp_type(), rvd_type(), rfc_type();


/* the primordial (hardware) layer */
struct layer *root_layer = &et_layer;

struct layer et_layer = {
	NULL,		
	et_prots,	
	0,
	et_unprs,
	et_addr,
	et_type		
};

struct nameber et_prots[] = {
	{ 0x1,   "IEEE", 0, NULL, (RED<<4|WHITE)},	/* IEEE 802.3  jrd */
	{ 0x200, "PUP", 0, NULL, (BLUE<<4|WHITE)},
	{ 0x600, "XNS", 0, NULL, (BROWN<<4|WHITE)},
	{ 0x800, "IP", 0, &ip_layer, (MAGENTA<<4|WHITE)},
	{ 0x801, "X.75_IP", 0, NULL, (LGRAY<<4|WHITE)},
	{ 0x802, "NBS_IP", 0, NULL, (LGRAY<<4|WHITE)},
	{ 0x803, "ECMA_IP", 0, NULL, (LGRAY<<4|WHITE)},
	{ 0x804, "Chaos", 0, &chaos_layer, (GREEN<<4|WHITE)},
	{ 0x805, "X.25", 0, NULL, (LGRAY<<4|WHITE)},
	{ 0x806, "ARP", 0, &arp_layer, (BROWN<<4|WHITE)},
	{ 0x807, "XNS_Compatibility", 0, NULL, (BROWN<<4|WHITE)},
	{ 0x81c, "Symbolics", 0, NULL, (LGRAY<<4|WHITE)},
	{ 0x900, "UB-debug", 0, NULL, (LGRAY<<4|WHITE)},
	{ 0xa00, "PUP-802.3", 0, NULL, (BLUE<<4|WHITE)},
	{ 0xa01, "PUP-adr-xlt", 0, NULL, (BLUE<<4|WHITE)},
	{ 0xbad, "Banyan",0, NULL, (GREEN<<4|WHITE)},
	{ 0x1001, "IP_trailer_1_block", 0, NULL, (MAGENTA<<4|WHITE)},
	{ 0x1002, "IP_trailer_2_blocks", 0, NULL, (MAGENTA<<4|WHITE)},
	{ 0x1003, "IP_trailer_3_blocks", 0, NULL, (MAGENTA<<4|WHITE)},
	{ 0x1004, "IP_trailer_4_blocks", 0, NULL, (MAGENTA<<4|WHITE)},
	{ 0x1005, "IP_trailer_5_blocks", 0, NULL, (MAGENTA<<4|WHITE)},
	{ 0x1006, "IP_trailer_6_blocks", 0, NULL, (MAGENTA<<4|WHITE)},
	{ 0x1007, "IP_trailer_7_blocks", 0, NULL, (MAGENTA<<4|WHITE)},
	{ 0x1008, "IP_trailer_8_blocks", 0, NULL, (MAGENTA<<4|WHITE)},
	{ 0x1009, "IP_trailer_9_blocks", 0, NULL, (MAGENTA<<4|WHITE)},
	{ 0x100a, "IP_trailer_10_blocks", 0, NULL, (MAGENTA<<4|WHITE)},
	{ 0x100b, "IP_trailer_11_blocks", 0, NULL, (MAGENTA<<4|WHITE)},
	{ 0x100c, "IP_trailer_12_blocks", 0, NULL, (MAGENTA<<4|WHITE)},
	{ 0x100d, "IP_trailer_13_blocks", 0, NULL, (MAGENTA<<4|WHITE)},
	{ 0x100e, "IP_trailer_14_blocks", 0, NULL, (MAGENTA<<4|WHITE)},
	{ 0x100f, "IP_trailer_15_blocks", 0, NULL, (MAGENTA<<4|WHITE)},
	{ 0x1600, "VALID", 0, NULL, (LGRAY<<4|WHITE)},
	{ 0x1600, "BBN_Simnet", 0, NULL, (LGRAY<<4|WHITE)},
	{ 0x6001, "DEC_MOP_DLA", 0, NULL, (CYAN<<4|WHITE)},
		/* Dump/Load assistance */
	{ 0x6002, "DEC_MOP_RC", 0, NULL, (CYAN<<4|WHITE)},
		/* Remote console facility */
	{ 0x6003, "DECnet", 0, NULL, (BLUE<<4|WHITE)},
		/* DECnet routing */
	{ 0x6004, "DECLAT", 0, NULL, (BROWN<<4|WHITE)},
	{ 0x6005, "DECInit", 0, NULL, (BROWN<<4|WHITE)},
	{ 0x6006, "C_USE", 0, NULL, (BLUE<<4|WHITE)},
	{ 0x6007, "LAVC", 0, NULL, (BROWN<<4|WHITE)},
	{ 0x6008, "DEC-amber", 0, NULL, (BROWN<<4|WHITE)},
	{ 0x6009, "DEC-MUMPS", 0, NULL, (BROWN<<4|WHITE)},
	{ 0x7000, "UB-dnload", 0, NULL, (LGRAY<<4|WHITE)},
	{ 0x7002, "UB-loopbk", 0, NULL, (LGRAY<<4|WHITE)},
	{ 0x7030, "Proteon", 0, NULL, (BROWN<<4|WHITE)},
	{ 0x8003, "Cronus_VLN", 0, NULL, (LGRAY<<4|WHITE)},
	{ 0x8004, "Cronus_Direct", 0, NULL, (LGRAY<<4|WHITE)},
	{ 0x8005, "HP_Probe", 0, NULL, (LGRAY<<4|WHITE)},
	{ 0x8006, "Nextar", 0, NULL, (LGRAY<<4|WHITE)},
	{ 0x8006, "AT&T", 0, NULL, (LGRAY<<4|BLACK)},
	{ 0x8010, "Excelan", 0, NULL, (LGRAY<<4|WHITE)},
	{ 0x8013, "Silicon_Graphics-diagnostic", 0, NULL, (BROWN<<4|WHITE)},
	{ 0x8014, "Silicon_Graphics-games", 0, NULL, (BROWN<<4|WHITE)},
	{ 0x8015, "Silicon_Graphics-resvd", 0, NULL, (BROWN<<4|WHITE)},
	{ 0x8016, "Silicon_Graphics-XNS", 0, NULL, (BROWN<<4|WHITE)},
	{ 0x8019, "Apollo_DOMAIN", 0, NULL, (BROWN<<4|WHITE)},
	{ 0x802e, "Tymshare", 0, NULL, (BROWN<<4|WHITE)},
	{ 0x802f, "Tigan", 0, NULL, (BROWN<<4|WHITE)},
	{ 0x8035, "RARP", 0, NULL, (DGRAY<<4|WHITE)},
	{ 0x8036, "Aeonic", 0, NULL, (BROWN<<4|WHITE)},
	{ 0x8038, "DEC_LanBridge", 0, NULL, (BLUE<<4|WHITE)},
	{ 0x8039, "DEC-DSM/DTP", 0, NULL, (BLUE<<4|WHITE)},
	{ 0x803a, "DEC-Argonaut", 0, NULL, (BLUE<<4|WHITE)},
	{ 0x803b, "DEC-VAXELN", 0, NULL, (BLUE<<4|WHITE)},
	{ 0x803c, "DEC-NMSV", 0, NULL, (BLUE<<4|WHITE)},
	{ 0x803d, "DEC-Encrypt", 0, NULL, (BLUE<<4|WHITE)},
	{ 0x803e, "DEC-Time-service", 0, NULL, (BLUE<<4|WHITE)},
	{ 0x803f, "DEC-LTM", 0, NULL, (BLUE<<4|WHITE)},
	{ 0x8040, "DEC-NetBios", 0, NULL, (BLUE<<4|WHITE)},
	{ 0x8041, "DEC-LAST", 0, NULL, (BLUE<<4|WHITE)},
	{ 0x8046, "AT&T", 0, NULL, (LGRAY<<4|BLACK)},
	{ 0x8047, "AT&T", 0, NULL, (LGRAY<<4|BLACK)},
	{ 0x805B, "SU_V_exp", 0, NULL, (LGRAY<<4|WHITE)},
	{ 0x805C, "SU_V", 0, NULL, (LGRAY<<4|WHITE)},
	{ 0x8069, "AT&T", 0, NULL, (LGRAY<<4|BLACK)},
	{ 0x809b, "AppleTalk", 0, NULL, (LGRAY<<4|WHITE)},
	{ 0x80d5, "DECLAD", 0, NULL, (BLUE<<4|YELLOW)},
	{ 0x8137, "Novell", 0, NULL, (RED<<4|WHITE)},
	{ 0x80f2, "Retix", 0, NULL, (LGRAY<<4|YELLOW)},
	{ 0x9000, "Loopbk", 0, NULL, (GREEN<<4|WHITE)},
	{ 0x9001, "Bridge_XNS", 0, NULL, (LGRAY<<4|BLACK)},
	{ 0x9002, "Bridge_TCP/IP", 0, NULL, (LGRAY<<4|BLACK)},
	{ 0x9003, "Bridge_comms", 0, NULL, (LGRAY<<4|BLACK)},
	{ 0xff00, "BBN_VITAL", 0, NULL, (LGRAY<<4|WHITE)},
	{ 0, 0, 0, NULL}
};

struct layer arp_layer = {
	&et_layer,
	NULL,
	0,
	arp_unprs,
	NULL,
	NULL
};

struct layer ip_layer = {
	&et_layer,
	ip_prots,
	0,
	ip_unprs,
	ip_addr,
	ip_type
	};

/* temp debugging kluge */
struct layer netblt_layer = {
	&ip_layer,
	NULL,
	0,
	netblt_unprs,
	NULL,
	NULL
	};

struct nameber ip_prots[] = {
	{ 1, "ICMP", 0, &icmp_layer},
	{ 3, "GGP", 0, NULL},
	{ 6, "TCP", 0, &tcp_layer},
	{ 8, "EGP", 0, NULL},
	{ 9, "IGP", 0, NULL},
	{ 17, "UDP", 0, &udp_layer},
	{ 66, "RVD", 0, &rvd_layer},
	{ 77, "Sun_FS", 0, NULL},
	{ 255, "Netblt", 0, &netblt_layer},
	{ 0, 0, 0, NULL}
};

struct layer udp_layer = {
	&ip_layer,
	udp_prots,
	0,
	udp_unprs,
	NULL,
	udp_type
	};

struct nameber udp_prots[] = {
	{ 7, "echo", 0, NULL},
	{ 9, "discard", 0, NULL},
	{ 19, "chargen", 0, NULL},
	{ 22, "netlog", 0, NULL},
	{ 35, "printer", 0, NULL},
	{ 37, "time", 0, NULL},
	{ 39, "rlp", 0, NULL},
	{ 42, "name", 0, NULL},
	{ 53, "domain", 0, NULL},
	{ 67, "bootps", 0, NULL},
	{ 68, "bootpc", 0, NULL},
	{ 69, "tftp", 0, NULL},
	{ 111, "sunrpc", 0, NULL},
	{ 300, "nms", 0, NULL},
	{ 513, "rwho", 0, NULL},
	{ 514, "syslog", 0, NULL},
	{ 517, "talk", 0, NULL},
	{ 520, "route", 0, NULL},
	{ 0, 0, 0, NULL}
};

struct layer tcp_layer = {
	&ip_layer,
	tcp_prots,
	0,
	tcp_unprs,
	NULL,
	tcp_type
	};

struct nameber tcp_prots[] = {
	{ 7, "echo", 0, NULL},
	{ 9, "discard", 0, NULL},
	{ 11, "systat", 0, NULL},
	{ 13, "daytime", 0, NULL},
	{ 15, "netstat", 0, NULL},
	{ 17, "quotd", 0, NULL},
	{ 19, "chargen", 0, NULL},
	{ 21, "ftp", 0, NULL},
	{ 23, "telnet", 0, NULL},
	{ 25, "smtp", 0, NULL},
	{ 35, "printer", 0, NULL},
	{ 37, "time", 0, NULL},
	{ 42, "name", 0, NULL},
	{ 43, "whois", 0, NULL},
	{ 53, "domain", 0, NULL},
	{ 57, "mtp", 0, NULL},
	{ 77, "rje", 0, NULL},
	{ 79, "finger", 0, NULL},
	{ 87, "ttylink", 0, NULL},
	{ 95, "supdup", 0, NULL},
	{ 101, "hostnames", 0, NULL},
	{ 115, "write", 0, NULL},
	{ 512, "rexec", 0, NULL},
	{ 513, "rlogin", 0, NULL},
	{ 514, "rsh", 0, NULL},
	{ 515, "lpd", 0, NULL},
	{ 0, 0, 0, NULL}
};

struct layer icmp_layer = {
	&ip_layer,
	icmp_prots,
	0,
	icmp_unprs,
	NULL,
	icmp_type
	};

struct nameber icmp_prots[] = {
	{ 0x000, "echo_reply", 0, NULL},
	{ 0x300, "net_unreachable", 0, NULL},
	{ 0x301, "host_unreachable", 0, NULL},
	{ 0x302, "protocol_unreachable", 0, NULL},
	{ 0x303, "port_unreachable", 0, NULL},
	{ 0x304, "fragmentation_needed", 0, NULL},
	{ 0x305, "source_route_failed", 0, NULL},
	{ 0x400, "source_quench", 0, NULL},
	{ 0x500, "net_redirect", 0, NULL},
	{ 0x501, "host_redirect", 0, NULL},
	{ 0x502, "tos_&_net_redirect", 0, NULL},	
	{ 0x503, "tos_&_host_redirect", 0, NULL},	
	{ 0x800, "echo_request", 0, NULL},
	{ 0xb00, "ttl_exceeded", 0, NULL},
	{ 0xb01, "frag_reassembly_time_exceeded", 0, NULL},
	{ 0xc00, "parameter_problem", 0, NULL},
	{ 0xd00, "timestamp_request", 0, NULL},
	{ 0xe00, "timestamp_reply", 0, NULL},
	{ 0xf00, "info_request", 0, NULL},
	{ 0x1000, "info_reply", 0, NULL},
	{ 0, 0, 0, NULL}
};

struct layer rvd_layer = {
	&ip_layer,
	rvd_types,
	0,
	rvd_unprs,
	NULL,
	rvd_type
	};

struct nameber rvd_types[] = {
	{ 1, "spinup", 0, NULL},
	{ 2, "spindown", 0, NULL},
	{ 3, "read", 0, NULL},
	{ 4, "write-block", 0, NULL},
	{ 17, "spinup-ack", 0, NULL},
	{ 18, "error", 0, NULL},
	{ 19, "spindown-ack", 0, NULL},
	{ 20, "read-block", 0, NULL},
	{ 21, "write-ack", 0, NULL},
	{ 0, 0, 0, NULL}
};

struct layer chaos_layer = {
	&et_layer,
	chaos_opcodes,
	0,
	chaos_unprs,
	chaos_addr,
	chaos_type
};

struct nameber chaos_opcodes[] = {
	{ 1, "RFC", 0, &rfc_layer},
	{ 2, "OPN", 0, NULL},
	{ 3, "CLS", 0, NULL},
	{ 4, "FWD", 0, NULL},
	{ 5, "ANS", 0, NULL},
	{ 6, "SNS", 0, NULL},
	{ 7, "STS", 0, NULL},
	{ 010, "RUT", 0, NULL},
	{ 011, "LOS", 0, NULL},
	{ 012, "LSN", 0, NULL},
	{ 013, "MNT", 0, NULL},
	{ 014, "EOF", 0, NULL},
	{ 015, "UNC", 0, NULL},
	{ 016, "BRD", 0, NULL},
	{ 0, 0, 0, NULL}
};

struct layer rfc_layer = {
	&chaos_layer,
	NULL,
	0,
	rfc_unprs,
	NULL,
	NULL
	};
