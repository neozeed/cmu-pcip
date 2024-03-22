/*  Copyright 1986 by Carnegie Mellon  */
/*  See permission and disclaimer notice in file "cmu-note.h"  */
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
#include <h19.h>
#include <ctype.h>
#include <colors.h>
#include <match.h>
#include "watch.h"

/* have to declare all this stuff up top because of forward references */

/* packet header unparsing functions */
int pr_unprs(), ip_unprs(), arp_unprs(), chaos_unprs();
int udp_unprs(), tcp_unprs(), icmp_unprs(), rvd_unprs();
int rfc_unprs(), et_unprs(), netblt_unprs();


/* protocol layer structures */
extern struct layer pronet_layer, ip_layer, udp_layer, tcp_layer, icmp_layer;
extern struct layer rvd_layer, arp_layer, netblt_layer;


/* arrays of protocol layer structures */
extern struct nameber pronet_prots[], ip_prots[], udp_prots[], tcp_prots[];
extern struct nameber icmp_prots[], rvd_types[];


/* filter setting functions */
extern int pr_addr(), pr_type();
extern int ip_addr(), ip_type();
extern int udp_type(), tcp_type(), icmp_type(), rvd_type();


/* the primordial (hardware) layer */
struct layer *root_layer = &pronet_layer;

struct layer pronet_layer = {
	NULL,		
	pronet_prots,
	0,
	pr_unprs,
	pr_addr,
	pr_type
};

struct nameber pronet_prots[] = {
	{ 0x0201, "IP", 0, &ip_layer, (RED<<4|WHITE)},
	{ 0x0203, "ARP", 0, &arp_layer, (BROWN<<4|WHITE)},
	{ 0x0204, "HDLC", 0, NULL, (BLUE<<4|WHITE)},
	{ 0x0206, "Decnet", 0, NULL, (CYAN<<4|WHITE)},
	{ 0x0208, "DEC MOP", 0, NULL, (LGRAY<<4|WHITE)},
	{ 0x020a, "Novell", 0, NULL, (YELLOW<<4|WHITE)},
	{ 0x020b, "Pheonix", 0, NULL, (BLUE<<4|WHITE)},
	{ 0, 0, 0, NULL}
};

struct layer arp_layer = {
	&pronet_layer,
	NULL,
	0,
	arp_unprs,
	NULL,
	NULL
};

struct layer ip_layer = {
	&pronet_layer,
	ip_prots,
	0,
	ip_unprs,
	NULL,
	ip_type
	};

/* temp debugging kluge */
struct layer netblt_layer = {
	&ip_layer,
	NULL,
	0,
	netblt_unprs,
	NULL,
	NULL,
	};

struct nameber ip_prots[] = {
	{ 1, "ICMP", 0, &icmp_layer},
	{ 3, "GGP", 0, NULL},
	{ 6, "TCP", 0, &tcp_layer},
	{ 8, "EGP", 0, NULL},
	{ 9, "IGP", 0, NULL},
	{ 17, "UDP", 0, &udp_layer},
	{ 66, "RVD", 0, &rvd_layer},
	{ 77, "Sun FS", 0, NULL},
	{ 255, "netblt", 0, &netblt_layer},
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
	{ 42, "name", 0, NULL},
	{ 53, "domain", 0, NULL},
	{ 69, "tftp", 0, NULL},
	{ 300, "nms", 0, NULL},
	{ 513, "rwho", 0, NULL},
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
	{ 0x000, "echo reply", 0, NULL},
	{ 0x300, "net unreachable", 0, NULL},
	{ 0x301, "host unreachable", 0, NULL},
	{ 0x302, "protocol unreachable", 0, NULL},
	{ 0x303, "port unreachable", 0, NULL},
	{ 0x304, "fragmentation needed", 0, NULL},
	{ 0x305, "source route failed", 0, NULL},
	{ 0x400, "source quench", 0, NULL},
	{ 0x500, "net redirect", 0, NULL},
	{ 0x501, "host redirect", 0, NULL},
	{ 0x502, "tos & net redirect", 0, NULL},	
	{ 0x503, "tos & host redirect", 0, NULL},	
	{ 0x800, "echo request", 0, NULL},
	{ 0xb00, "ttl exceeded", 0, NULL},
	{ 0xb01, "frag reassembly time exceeded", 0, NULL},
	{ 0xc00, "parameter problem", 0, NULL},
	{ 0xd00, "timestamp request", 0, NULL},
	{ 0xe00, "timestamp reply", 0, NULL},
	{ 0xf00, "info request", 0, NULL},
	{ 0x1000, "info reply", 0, NULL},
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
