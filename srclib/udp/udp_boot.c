/*  Copyright 1986 by Carnegie Mellon  */
/*  See permission and disclaimer notice in file "cmu-note.h"  */
#include	<cmu-note.h>

/* This file contains the routine that implements a Bootstrap Protocol (bootp)
 * client.  In addition to resolving the machines IP address, it also sets
 * the addresses of the default gateway, the domain and IEN116 name servers,
 * the time servers, and the subnet mask.
 */

/* Author: David C. Kovar (dk1z@andrew.cmu.edu)
     Date: August 1986
*/

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
#include <udp.h>
#include <timer.h>
#include <sockets.h>
#include <memory.h>
#include <attrib.h>
#include <fcntl.h>
#include <ctype.h>
#include <bootp.h>

#define  WAITING	1	/* Possible status values */
#define  ALARM		2
#define  PACKRECV	3
#define  ABORT		4

#define BACKINT		1	/* How long to back off, default = 1 second */
#define MAXREQ		3	/* Maximum number of times to try a request */

#define AMASK	0x80		/* Used for determining subnet mask */
#define AADDR	0x00
#define BMASK	0xC0
#define BADDR	0x80
#define CMASK	0xE0
#define CADDR	0xC0

static int recpack();		/* Handle received packet */
static int alarmed();		/* Handle alarms */
static in_name check_ip();	/* Check our IP address */

static task *bootptask; 	/* BOOTP request task */
static int status;		/* Current status */
static int tid; 		/* Transaction ID */
static PACKET p;		/* Packets */
static char haddr[16];		/* Machine's hardware address */

extern char _net_if_name;	/* Interface name */
extern NET nets[];		/* Array of nets to use */
extern struct custom custom;	/* Customization information */
extern int allow_null_ip_addr;

/* Try to find our IP address using the bootstrap protocol.
 * Build a bootp request packet and broadcast it onto the net.
 * Then wait for a reply packet, or a timeout.
 */

in_name bootp(nback, force, retries, toggle)
int nback,			/* No backoff flag */
    force,			/* Force new address flag */
    retries,			/* Number of retries flag */
    toggle;			/* Toggle 4.2 broadcast addr flag */
{
    in_name fhost;		/* Where to send request to */
    long    time ();
    int     nreq = 0;		/* Number of requests sent */
    int     backoff = BACKINT;	/* How long to back off */
    long    starttime;		/* Time session started */
    struct bootp *bp;		/* BOOTP packet pointer */
    timer *tm;			/* A timer */
    UDPCONN udps;		/* UDP connection */
#ifdef COMPAT42 		/* Broadcast address to use */
    int broad42 = 1;
#else
    int broad42 = 0;
#endif


#ifdef	DEBUG
    if(NDEBUG & INFOMSG)
	printf("Entering BOOTP.\n");
#endif

    if (toggle)
	broad42 = !broad42;		/* Toggle 4.2 broadcast addr */

    srand((unsigned) time(NULL));	/* Seed random timer for tid */

    /* Check IP address and get broadcast address back based on IP addr */
    fhost = check_ip(force, broad42);
    if (status == ABORT)
	    return(custom.c_me);

    if (retries == 0) { 	/* If number of retries not defined, use default */
	retries = MAXREQ;
    }

    tm = tm_alloc();		/* Get a timer */
    if (tm == 0) {
#ifdef	DEBUG
	if(NDEBUG & INFOMSG)
	    printf("BOOTP: Couldn't allocate timer.\n");
#endif
	return (-1);
    }

    /* Open connection */
    udps = udp_open(fhost, UDP_BOOTPS, UDP_BOOTPC, recpack);
    if (udps == 0) {
#ifdef	DEBUG
	if(NDEBUG & INFOMSG)
	    printf("BOOTP: Couldn't open UDP conn.\n");
#endif
	return (-1);
    }

    p = udp_alloc(sizeof(struct bootp) , 0);	/* Get a packet */
    if (p == NULL) {
#ifdef	DEBUG
	if(NDEBUG & INFOMSG)
	    printf("BOOTP: Couldn't allocate packet!\n");
#endif
	return (-1);
    }

    /* Make bp point at UDP data section */
    bp = (struct bootp *) udp_data (udp_head (in_head (p)));

    /* Fill in BOOTP data */
    bp->bp_op = BOOTREQUEST;		/* Boot request */
    bp->bp_htype = nets[0].n_htype;	/* Hardware type */
    bp->bp_hlen = nets[0].n_hal;	/* Length of hardware address */
    bp->bp_hops = 0;			/* Number of hops */
    tid = rand();			/* Transaction ID */
    bp->bp_xid = tid;
    bp->bp_secs = 0;			/* Seconds since start up */
    bp->bp_ciaddr = 0;			/* Client address (us) */
    bp->bp_yiaddr = 0;			/* Your address (us, via server) */
    bp->bp_siaddr = 0;			/* Server address */
    bp->bp_giaddr = 0;			/* Gateway address */

    /* Copy hardware address into bootp packet */
    memcpy(bp->bp_chaddr, nets[0].n_haddr, nets[0].n_hal);
    memcpy(haddr, bp->bp_chaddr, nets[0].n_hal);
    bp->bp_sname[0] = 0;		/* Server name */
    bp->bp_file[0] = 0; 		/* Boot file */
    bp->bp_vend[0] = 0; 		/* Vendor information */

    starttime = time(); 		/* Set start time for session */
    bootptask = tk_cur;

    for (;;) {				/* Loop 'til done */
	/* Send packet */
	if (udp_write(udps, p, sizeof(struct bootp)) <= 0) {
#ifdef DEBUG
	    if (NDEBUG & INFOMSG)
		printf ("BOOTP: udp write failed.\n");
#endif
	}
	status = WAITING;		/* We're waiting for reply */
	tm_set(backoff, alarmed, 0, tm); /* Fire off timer */

	tk_block();			/* Block 'til alarm or packet */
	if (status == PACKRECV) 	/* If response was received ... */
		break;
	else if (status == ALARM) {		/* If it was alarm ... */
	    /* If the backoff interval is not defined and the last interval
	       was less than 60, double last interval and add random between
	       0 and 3 to help avoid congestion */
	    if (nback == 0) {
		if (backoff < 60) {
		    backoff = (backoff << 1) + (rand() % 3);
		}
		else
		    backoff = 60;
	    }

	    /* Point at data part again */
	    bp = (struct bootp *) udp_data (udp_head (in_head (p)));

	    /* Update time we've been waiting */
	    bp->bp_secs = time() - starttime;

	    /* Check to see if we've sent out too many packets */
	    if (++nreq > retries) {
#ifdef DEBUG
		if (NDEBUG & INFOMSG)
			printf("BOOTP: Server not responding.\n");
#endif
		status = ABORT;
		break;
	    }
#ifdef DEBUG
	    if (NDEBUG & INFOMSG)
		    printf("BOOTP: Sending request: %d\n", nreq);
#endif
	} else {
#ifdef DEBUG
		printf("BOOTP: invalid state!\n");
#endif
		break;
	}
    }
    tm_clear(tm);
    tm_free(tm);
    udp_free(p);
    udp_close(udps);
    if (status == ABORT)
	return(0);
    else
	return(custom.c_me);
}


/* Called when alarm goes off */
static alarmed() {
    if(status == WAITING) {		/* Avoid possible race condition */
	status = ALARM; 		/* Set status */
#ifdef DEBUG
	if (NDEBUG & (INFOMSG | TMO))
	    printf("BOOTP: Alarmed\n");
#endif
	tk_wake(bootptask);		/* And wake up main routine */
    }
}


/* Called when packet comes in */
static recpack(p, len, host)
PACKET p;
unsigned len;
in_name host;
{
    struct vend *vendp; 		/* Pointer for vendor information */
    struct bootp *rbp;

    if (len == 0) {			/* Check for bad packet */
#ifdef DEBUG
	if (NDEBUG & INFOMSG)
	    printf("BOOTP: bad length packet received\n");
#endif
	udp_free (p);			/* Free packet */
	return;
    }
#ifdef DEBUG
    if (NDEBUG & INFOMSG)
	printf("BOOTP: Packet received.\n");
#endif

    /* Point at data part */
    rbp = (struct bootp *) udp_data (udp_head (in_head (p)));

    /* Check hardware address and transaction ID to make sure it's for us */
    if ((memcmp(haddr, rbp->bp_chaddr, 6) == 0)
	&& (tid == rbp->bp_xid)
	&& (rbp->bp_op == BOOTREPLY)) {
#ifdef DEBUG
	if (NDEBUG & INFOMSG)
	    printf("BOOTP: IP address: %a\n", rbp->bp_yiaddr);
#endif

	/* Update custom file */
	write_ip(rbp->bp_yiaddr, (struct vend *)(rbp->bp_vend));
	status = PACKRECV;		/* We've received a packet */
	tk_wake(bootptask);		/* Wake up main routine */
    }
#ifdef DEBUG
    else if (NDEBUG & INFOMSG)
	printf("BOOTP: Packet received but it was not for me.\n");
#endif

    udp_free (p);			/* Free packet */
}

/* Checks to see if IP address already defined. Returns value of broadcast
   address based on bogus IP address in custom structure. Not elegant, but
   it works. Force flag set if we want new IP address no matter what. */
static in_name check_ip(force, broad42)
int force, broad42;
{
    /* Check to see if we're forcing a new IP address. */
    if (force) {
#ifdef COMPAT42
	if (broad42)
	    return(nets[0].n_netbr42);
	else
	    return(nets[0].n_netbr);
#else
	return(0xffffffff);
#endif
    }

    /* If my IP address is 0, broadcast is 255.255.255.255 */
    if (custom.c_me == 0)
	    return(0xffffffff);

    /* If my address is the 4.2 broadcast address ... */
    if (custom.c_me == nets[0].n_netbr42) {
	    /* and if we *want* to use 4.2 broadcast (0.0), use it */
	    if (broad42)
		    return(nets[0].n_netbr42);
	    /* Otherwise, use the normal broadcast address (.255.255) */
	    else
		    return(nets[0].n_netbr);
    }
    /* If the subnet is 0, use subnet broadcast address */
    if ((custom.c_me & ~custom.c_net_mask) == 0)
	    return(nets[0].n_subnetbr);

    /* Otherwise, leave it alone and exit */
#ifdef DEBUG
    if (NDEBUG & INFOMSG)
	printf("BOOTP: Valid IP address already in place.\n");
#endif
    status = ABORT;
}


/* Updates custom structure. Changes IP address, subnet mask, number of
   subnet bits, and name and time server addresses. Everything 'cept the
   IP address comes from the vendor section of the bootp packet */
static write_ip(addr, vendp)
in_name addr;				/* Our IP address */
struct vend *vendp;			/* Everything else */
{
    int     outfd;			/* File descriptor */
    register i;
    long    mask;			/* Temp subnet mask */
    long    get_dosl ();
    extern  char netcust[];

    /* Set address */
    if(!(custom.c_me = addr)) {
#ifdef DEBUG
	printf("BOOTP: NULL IP address returned by server!\n");
#endif
	}

    /* check on subnet routing - if no mask then make one using number of
       subnet bits recorded in custom structure */
    fixup_subnet_mask();

  /* Check to see if vendor specific information is ours */
  if (memcmp(vendp->v_magic, VM_CMU, sizeof(vendp->v_magic)) == 0) {

#ifdef DEBUG
    if (NDEBUG & INFOMSG) {
	printf("BOOTP: CMU vendor specific data received\n");
	printf("BOOTP: received subnet mask %a\n", vendp->v_smask);
    }
#endif

    if (vendp->v_smask != 0) {
	custom.c_net_mask = vendp->v_smask;

	/* Figure out subnet mask and number of subnet bits */
	mask = custom.c_net_mask;
	custom.c_subnet_bits = 0;

	for (i = 0; i < 32; i++) {	/* Count number of '1's in mask */
	    if (mask & 1) {
		custom.c_subnet_bits++;
	    }
	    mask >>= 1;
	}

	/* Number of bits based on type of IP address */
	if ((custom.c_me & AMASK) == AADDR)
	    custom.c_subnet_bits -= 8;
	else if ((custom.c_me & BMASK) == BADDR)
	    custom.c_subnet_bits -= 16;
	else if ((custom.c_me & CMASK) == CADDR)
	    custom.c_subnet_bits -= 24;
    }

 /* Get default gateway address */
#ifdef DEBUG
    if (NDEBUG & INFOMSG)
	printf("BOOTP: Default gateway address %a\n", vendp->v_dgate);
#endif
    if (vendp->v_dgate != 0) {
	custom.c_defgw = vendp->v_dgate;
    }

    /* Now fill in servers. Leave servers in custom intact, only update */
    /* Time servers */
    for (i = 0; i < custom.c_numtime; i++) {
	if (custom.c_time[i] == vendp -> v_ts1) {
	    vendp -> v_ts1 = 0;
	}
	else if (custom.c_time[i] == vendp -> v_ts2) {
	    vendp -> v_ts2 = 0;
	}
    }

    if (vendp -> v_ts1 != 0 && i < MAXTIMES) {
	custom.c_time[i++] = vendp -> v_ts1;
	custom.c_numtime++;
    }

    if (vendp -> v_ts2 != 0 && i < MAXTIMES) {
	custom.c_time[i++] = vendp -> v_ts2;
	custom.c_numtime++;
    }

    /* IEN-116 name servers */
    for (i = 0; i < custom.c_numname; i++) {
	if (custom.c_names[i] == vendp -> v_ins1) {
	    vendp -> v_ins1 = 0;
	}
	else if (custom.c_names[i] == vendp -> v_ins2) {
	    vendp -> v_ins2 = 0;
	}
    }

    if (vendp -> v_ins1 != 0 && i < 2) {
	custom.c_names[i++] = vendp -> v_ins1;
	custom.c_numname++;
    }

    if (vendp -> v_ins2 != 0 && i < 2) {
	custom.c_names[i++] = vendp -> v_ins2;
	custom.c_numname++;
    }

    /* Domain name servers */
    for (i = 0; i < custom.c_dm_numname; i++) {
	if (custom.c_dm_servers[i] == vendp -> v_dns1) {
	    vendp -> v_dns1 = 0;
	}
	else if (custom.c_dm_servers[i] == vendp -> v_dns2) {
	    vendp -> v_dns2 = 0;
	}
    }

    if (vendp -> v_dns1 != 0 && i < 3) {
	custom.c_dm_servers[i++] = vendp -> v_dns1;
	custom.c_dm_numname++;
    }

    if (vendp -> v_dns2 != 0 && i < 3) {
	custom.c_dm_servers[i++] = vendp -> v_dns2;
	custom.c_dm_numname++;
    }

  }
 /* Now save the data back. */

#define DATE	0x2a
#define TIME	0x2c

    custom.c_ctime = get_dosl(TIME);/* Set the time and date of */
    custom.c_cdate = get_dosl(DATE);/* the last modification */

    /* netcust is a global from netinit */
    outfd = pcip_open(netcust, O_WRONLY | O_BINARY);
    if(outfd < 0) {
	printf("Error: unable to open the custom file (%s)\n", netcust);
	printf("Check for proper installation of NETDEV.SYS\n");
	exit(1);
    }
    mkraw(outfd);
    write(outfd, &custom, sizeof(struct custom));/* write block */
    close(outfd);
}
