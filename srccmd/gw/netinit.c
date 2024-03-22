/* Get NET structs, PACKET structs, tasking and Queue stuff. */

#include <stdio.h>
#include <types.h>
#include <task.h>
#include <q.h>
#include <netq.h>
#include <net.h>
#include <custom.h>
#include <netbuf.h>
#ifdef MSC			/* DDP */
#include <fcntl.h>		/* DDP */
#endif				/* DDP */

/* 8/12/84 - modified to work with version 8 of the custom structure.
						<John Romkey>
   10/1/84 - added a declaration of calloc to make work with the new i/o
	library.				<John Romkey>
   10/3/84 - changed code to work with new net structure.
						<John Romkey>
   11/30/84 - made netinit check if forward compatiblity with subnet
	masks is necessary. If mask is 0 then generate it.
						<John Romkey>
   10/10/85 - Fix generation of subnet masks when 0.
						<Drew Perkins>
   10/15/85 - Open netcust: in binary mode to prevent CRLF translation.
						<Drew Perkins>
   3/24/84 - Provide support for multiple NETCUST devices so you can have
	more than one type of network interface.  The order of search
	is first the device specified by the "NETCUST" environment
	variable.  Second, a device dependent name "NETCUST%c", where
	'%c' is replaced by the variable __net_if_name.  This is expected
	to be something like '3' for 3com boards, 'p' for proNET, 's' for
	serial, 'i' for interlan, etc.  Lastly, the old "NETCUST:" is
	tried for compatibility.
						<Drew Perkins>
   8/7/86 - Make changes for bootp and broadcast addresses.
						<Drew Perkins>
   9/16/86 - Call pcip_open() instead of open() to avoid "FAT bad" bug.
						<Drew Perkins>
*/

/* The netinit() routine does the basic initialization at the lowest level
	of the PC networking environment. It does (in order):
	0. Custom initialization
	1. Packet buffer initialization, creation of the free queue.
	2. Tasking initialization.
	3. Per net initialization.

	Per protocol initialization is not done by this routine; it has to
	be done explicitly by the user. Thus if the applications program
	requires InterNet, it has to do in_init().
*/

#define AMASK	0x80
#define AADDR	0x00
#define BMASK	0xC0
#define BADDR	0x80
#define CMASK	0xE0
#define CADDR	0xC0

queue freeq;			/* the queue of unused packet buffers */

int MaxLnh;			/* Length of the biggest local net header in
					the whole system */

extern int Nnet;		/* Number of network drivers */
extern NET nets[];		/* The actual network structs */
extern unsigned _printf_ip_radix;
extern char _net_if_name;
extern unsigned chirpf, chirpd, chirpl, chirps; /* DDP */

PACKET buffers[20];

char netcust[14];		/* DDP - now global for BOOTP */
struct custom custom;		/* got to be defined somewhere */

int NBUF=18;		/* # of packet buffers */
int LBUF=1500;		/* size of packet buffers */

unsigned NDEBUG = 0;		/* Debugging...*/

int allow_null_ip_addr = 0;	/* DDP - Hack... */

/* give a hoot - don't pollute */
static char nomem[] = "netinit: out of memory";

char *calloc();
int crock_c(), brk_c(), netclose();

Netinit(stack_size)
int stack_size;
{
	int i;			/* general counter variable */
	PACKET packet;
	unsigned physaddr;
	char *env, *getenv();		/* DDP */
	unsigned tchirpf, tchirpd, tchirpl, tchirps; /* DDP */
	long templ;			/* DDP */

	crock_init();
	exit_hook(crock_c);

	brk_init();
	exit_hook(brk_c);

	/* this whole business with the custom structure will have to
		change soon to support multiple net interfaces. There
		will be one structure associated with each interface.
		Each net structure will have a pointer to its custom
		structure. The device names will be "netcust", "netcust2",
		"netcust3", etc., though no program will probably use
		more than two interfaces. The global "struct custom"
		will still remain, though, and will be the custom
		structure of the first interface, from "netcust".
	*/

	/* read in the custom structure */
#ifndef MSC					/* DDP */
	sprintf(netcust, "netcust%c", _net_if_name);

	i = open(netcust, 2);
	if(i < 0) {
		printf("Error: open failed for %s\n", netcust);
		printf("Check for improper NETDEV.SYS installation\n");
		exit(1);
	}
#else						/* DDP - Begin changes */
	i = -1; 				/* Nothing found yet */
	if(env = getenv("NETCUST")) {		/* Try environment */
		strcpy(netcust, env);		/* Remember file name */
		i = pcip_open(netcust, O_RDWR|O_BINARY);
	}

	if(i < 0) {				/* Try per device type */
		sprintf(netcust, "netcust%c", _net_if_name);
		i = pcip_open(netcust, O_RDWR|O_BINARY);

		if(i < 0) {			/* Try default */
			strcpy(netcust, "netcust"); /* Remember file name */
			i = pcip_open(netcust, O_RDWR|O_BINARY);
		}
	}
	if(i < 0) {
		printf("Error: open failed for %s\n", netcust);
		printf("Check for improper NETDEV.SYS installation\n");
		exit(1);
	}
#endif						/* DDP - End changes */

	mkraw(i);
	read(i, &custom, sizeof(struct custom));
	close(i);

	if(custom.c_cdate == 0L) {
		printf("Error: customization of NETDEV.SYS required\n");
		exit(1);
	}

	if(custom.c_iver != CVERSION) {
		printf("Error: obsolete level of NETDEV.SYS\n");
		exit(1);
	}

	NDEBUG = custom.c_debug;

	/* select the user's preferred radix */
	_printf_ip_radix = custom.c_ip_radix;

/* DDP begin */
	/* select the user's chirp parameters if they seem reasonable */
	/* reasonable is less than 2 seconds but not zero */
	tchirpf = custom.c_chirpf;
	tchirpd = custom.c_chirpd;
	tchirpl = custom.c_chirpl;
	tchirps = custom.c_chirps;
	templ = (long)custom.c_chirpl * (long)custom.c_chirps;
	if (templ && templ <= 200000L) {
		chirpf = tchirpf;
		chirpd = tchirpd;
		chirpl = tchirpl;
		chirps = tchirps;
	}
/* DDP end */

	MaxLnh = nets[0].n_lnh;

	/* initialize freeq */
	freeq.q_head = freeq.q_tail = NULL;

	/* Create the queue of free packets. Format each packet and add it to
		the tail of the queue */

	for(i=0; i<NBUF; i++) {
		packet = (PACKET)calloc(1, sizeof(struct net_buf));
		if(packet == 0) {
			puts(nomem);
			exit(1);
		}

		buffers[i] = packet;
		packet->nb_tstamp = 0;
		packet->nb_len = 0;
		packet->nb_buff = calloc(1, LBUF);

	/* Check if the packet buffer crosses a physical 64K boundary. If
		it does, throw it away and allocate another packet buffer.
		Because of the way the core allocator works and because
		of us only having 64K of data area, we should never get
		two of such packets. */

		physaddr = (unsigned)packet->nb_buff;
		physaddr = (physaddr >> 4) + get_ds();

		if(((physaddr + (LBUF>>4)) & 0xF000) > (physaddr & 0xF000)) {
#ifdef	DEBUG
			if(NDEBUG & INFOMSG) printf("found bad pkt buffer\n");
#endif
			packet->nb_buff = calloc(1, LBUF);
		}

		if(packet->nb_buff == NULL) {
			puts(nomem);
			exit(1);
		}

		putfree(packet);
	}

	freeq.q_max = freeq.q_len;
	freeq.q_min = freeq.q_len;

	/* The packet buffer system should now be initialized. It's time to
		go on to initializing tasking. */
#ifdef	DEBUG
	if(NDEBUG & INFOMSG)
		printf("Initializing Tasking.\n");
#endif

	tk_init(stack_size);		/* Do it! */
	tm_init();

	exit_hook(netclose);
	fixup_subnet_mask();		/* DDP */
	for(i = 0; i < Nnet; i++) {
		if(!nets[i].ip_addr)
			nets[i].ip_addr = custom.c_me;
		if(!nets[i].n_defgw)
			nets[i].n_defgw = custom.c_defgw;
		nets[i].n_inputq = q_create();
		if(!nets[i].n_inputq) {
			puts(nomem);
			exit(1);
		}
		if(nets[i].n_init == NULL) {
			printf("BUGHALT: no interface initializer %d\n", i);
			exit(1);
		}
		(*nets[i].n_init)(&nets[i], nets[i].n_initp1, nets[i].n_initp2);
	if((nets[i].ip_addr & AMASK) == AADDR) {
		nets[i].n_netbr = (nets[i].ip_addr & 0xff) | 0xffffff00;
		nets[i].n_netbr42 = nets[i].ip_addr & 0xff;
	}
	else if((nets[i].ip_addr & BMASK) == BADDR) {
		nets[i].n_netbr = (nets[i].ip_addr & 0xffff) | 0xffff0000;
		nets[i].n_netbr42 = nets[i].ip_addr & 0xffff;
	}
	else if((nets[i].ip_addr & CMASK) == CADDR) {
		nets[i].n_netbr = (nets[i].ip_addr & 0xffffff) | 0xff000000;
		nets[i].n_netbr42 = nets[i].ip_addr & 0xffffff;
	}
	nets[i].n_subnetbr = nets[i].ip_addr /*| ~*/ & custom.c_net_mask;
	if(((nets[i].ip_addr & ~(nets[i].n_custom->c_net_mask)) == 0) &&
	   allow_null_ip_addr == 0) {
		printf("Error: PC IP address not known.\n");
		printf("Use BOOTP or CUSTOM to set your IP address,\n");
		printf("or contact your network administrator for help.\n");
		exit(1);
	}
	}

	/* Per net initialization is now finished. Can return to the user
		all happy and initialized in a warm tasking environment. */

	return;
	}


/* Fix the subnet mask given the IP address and the number of subnet bits.
*/
fixup_subnet_mask() {
	unsigned long smask;
	unsigned subnet_bits = custom.c_subnet_bits;
	extern long lswap();

	/* initialize the bit field */
	if((custom.c_me & AMASK) == AADDR)
		smask = 0xFF000000;
	else if((custom.c_me & BMASK) == BADDR)
		smask = 0xFFFF0000;
	else if((custom.c_me & CMASK) == CADDR)
		smask = 0xFFFFFF00;

	/* generate the bit field for the subnet part. This code is
		incredibly dependent on byte ordering.
	 */
	while(subnet_bits--)
		smask = (smask >> 1) | 0x80000000;

	custom.c_net_mask = lswap(smask);
}
