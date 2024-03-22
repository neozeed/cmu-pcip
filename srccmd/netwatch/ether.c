/*  Copyright 1986 by Carnegie Mellon  */
/*  See permission and disclaimer notice in file "cmu-note.h"  */
/*  Mfr increases, format changes by Joe Doupnik, 29 Oct 1989, 31 March 90 */
#include <cmu-note.h>
#include <stdio.h>
#include <q.h>
#include <attrib.h>
#include <colors.h>
#include <ctype.h>
#include <match.h>
#include "watch.h"

/* display header */
static char *hex_header =
" source     destination     type  len  0  1  2  3  4  5  6  7  8  9  10";
/*		 ^	       ^	*/

static char *sym_header =
" kind  source       destination    type  len";
/*			 ^	       ^	*/

char *header;

/* ethernet handling stuff for netwatch */

extern struct nameber et_prots[];
extern struct layer et_layer;

struct ethdr {
	char et_src[6];
	char et_dst[6];
	unsigned et_type;
	};

unsigned long nbroadcasts = 0;
extern char local_net_disp;
extern char write_attrib;
extern int color_display;

unsigned bswap();
char *unparse_ether(), *strchr();		/* DDP */

norm_heading() {
	header = hex_header;
	}

sym_heading() {
	header = sym_header;
	}

et_unprs(data, y, x, len)
	register char *data;
	int y,x;
	unsigned len; {
	char buffer[200];
	register struct ethdr *eth = (struct ethdr *)data;
	char *addr;

	/* quick check if broadcast
	*/
	if((*(long *)data == 0xFFFFFFFF) &&
	   (*(unsigned *)(data+4) == 0xFFFF))
		nbroadcasts++;

	switch(prot_mode) {
	case MD_SYMBOLIC: {
		struct nameber *n;

		n = lookup(et_prots, (unsigned long) 0 + bswap(eth->et_type));

		if(n) {
			if(unknown_match)
				return FALSE;

			if(color_display)
				write_attrib = n->n_color;

			if(local_net_disp) {
				sprintf(buffer, "(%s ", unparse_ether(data+6));
				wr_string(buffer, y, 0, NORMAL);
				x = strlen(buffer);
				sprintf(buffer, "-> %s) ", unparse_ether(data));
				wr_string(buffer, y, x, NORMAL);
				x += strlen(buffer);
				}

			sprintf(buffer, "%s:    ", n->n_name);
			buffer[7] = '\0';	 /* jrd */
			wr_string(buffer, y, x, NORMAL);
			x += strlen(buffer);

			if(n->n_layer && n->n_layer->l_parse) {
				(*n->n_layer->l_parse)(data+14, y, x);
				break;
				}
			}
		else et_layer.l_unknown++;
		goto more;
		}

	case MD_NORMAL:

	if(color_display)
		write_attrib = BLUE<<4|WHITE;
more:
	/* source address */
	addr = unparse_ether(data+6);
	wr_string(addr, y, x, NORMAL);

	/* destination address */
	addr = unparse_ether(data);
	wr_string(addr, y, x+13, NORMAL);

	data += 12;

	sprintf(buffer, "  %04x %4u ", bswap(eth->et_type), len);
	wr_string(buffer, y, x+26, NORMAL);
	data += 2;

	sprintf(buffer," %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x", 
		mkbyte(data[0]), mkbyte(data[1]), mkbyte(data[2]),
		mkbyte(data[3]), mkbyte(data[4]), mkbyte(data[5]),
		mkbyte(data[6]), mkbyte(data[7]),
		mkbyte(data[8]), mkbyte(data[9]), mkbyte(data[10]));
	wr_string(buffer, y, x+38, NORMAL);
	break;
	}

	return TRUE;
	}

/* ethernet filters - source, destination and type */

/* allow type to be specified as "ip udp nms" or "804" or "chaos"
*/

et_type(buf, offset)
	register char *buf;	/* the type specification */
	int offset;	{	/* offset into packet for ether header */
	struct nameber *n;
	char *s;
	int i;
	int retcode = F_OK;

	/* uggh this is ugly - will have to do this in every filter...
		so much for distributed parsing...
	*/

	/* look for a space */
#ifndef MSC			/* DDP */
	s = index(buf, ' ');
#else				/* DDP */
	s = strchr(buf, ' ');	/* DDP */
#endif				/* DDP */

	if(s)
		*s = '\0';

	n = nlookup(et_prots, buf);
	if(!n) {
		/* okay, if it didn't match then if it is a question mark,
			print out all the types we know
		*/
		if(strcmp(buf, question) == 0) {
			scroll_start();
			printf("16 bit hexadecimal number\n\tor\n");
			print_namebers(et_prots);
			scroll_end();
			return F_PAUSE;
			}

		if(!isdigit(*buf)) {
			clr25();
			pr25(0, "bad packet type");
			return F_ERROR;
			}
		sscanf(buf, "%x", &i);
		}
	else i = n->n_number;

	/* if we did have a table lookup, chain to the next level unless
		there's nothing else to do

	   call before adding the bytes to the patterns so that if the
	   user types "ip tcp ?" we won't end up matching all ip/tcp
	   packets.
	*/
	if(n && n->n_layer && n->n_layer->l_type && s && *(s+1))
		retcode = (*n->n_layer->l_type)(s+1, offset+sizeof(struct ethdr));

	if(retcode != F_OK)
		return retcode;

	/* add the type field to both patterns */
	i = bswap(i);
	pt_addbytes(&pat1, &i, offset+12, 2);
	pt_addbytes(&pat2, &i, offset+12, 2);

	if(s)			/* DDP */
		*s = ' ';

	return F_OK;
	}

et_addr(buf, offset, how)
	register char *buf;	/* the type specification */
	int offset;		/* offset into packet for ether header */
	char how; {
	struct nameber *n;
	char *s;
	int i;
	int retcode = F_OK;
	unsigned type;
	unsigned addr[3];		/* the address */
	char c;

	/* uggh this is ugly - will have to do this in every filter...
		so much for distributed parsing...
	*/

	/* look for a space */
#ifndef MSC			/* DDP */
	s = index(buf, ' ');
#else				/* DDP */
	s = strchr(buf, ' ');	/* DDP */
#endif				/* DDP */

	if(s)
		*s = '\0';

	n = nlookup(et_prots, buf);
	if(!n) {
		/* okay, if it didn't match then if it is a question mark,
			print out all the types we know
		*/
		if(strcmp(buf, question) == 0) {
			scroll_start();
			printf("48 bit hexadecimal number\n\tor\n");
			print_addr_namebers(et_prots);
			scroll_end();
			return F_PAUSE;
			}

		/* if we make it here, then we should parse the ethernet
			address and add it to the appropriate patterns.
		 */
		if(strcmp(buf, "*") == 0) {
			for(i=0; i<3; i++)
				addr[i] = 0xffff;
			}
		else {
			/* THIS IS REALLY GROSS. */
			for(i=0; i<3; i++) {
				c = buf[i*4 + 4];
				buf[i*4 + 4] = '\0';
				sscanf(&buf[i*4], "%x", &addr[i]);
				addr[i] = bswap(addr[i]);
				buf[i*4+4] = c;
				}
			}

		/* insert in patterns */
		switch(how) {
		case 'S':
			pt_addbytes(&pat1, addr, offset+6, 6);
			pt_addbytes(&pat2, addr, offset+6, 6);
			break;
		case 'D':
			pt_addbytes(&pat1, addr, offset+0, 6);
			pt_addbytes(&pat2, addr, offset+0, 6);
			break;
		case 'W':
			pt_addbytes(&pat1, addr, offset+0, 6);
			pt_addbytes(&pat2, addr, offset+6, 6);
			break;
			}

		return F_OK;
		}

	/* if we did have a table lookup, chain to the next level unless
		there's nothing else to do

	   call before adding the bytes to the patterns so that if the
	   user types "ip tcp ?" we won't end up matching all ip/tcp
	   packets.
	*/
	if(!(n && s && *(s+1)))
		return F_OK;

	retcode = (*n->n_layer->l_addr)(s+1, offset+sizeof(struct ethdr), how);

	if(retcode != F_OK)
		return retcode;

	/* since we're matching on a protocol address, we can
		only accept packets of that protocol
	 */
	type = bswap(n->n_number);
	pt_addbytes(&pat1, &type, offset+12, 2);
	pt_addbytes(&pat2, &type, offset+12, 2);
	return F_OK;
	}


/* ethernet address hacking - define a table of the first three bytes
	of ethernet addresses versus manufacturer
	five characters max for manufacturer's name because of display
	formatting
*/
struct nameber ether_makers[] = {
	{ 0x00000c, "Cisco",0,NULL},	/* jrd */
	{ 0x00000f, "NeXT ",0,NULL },	/* jrd */
	{ 0x000022, "VTech",0,NULL },	/* jrd */
	{ 0x00002a, "TRW  ",0,NULL },
	{ 0x00005a, "S&Koc",0,NULL },
	{ 0x000065, "NetGe",0,NULL },	/* jrd */
	{ 0x000089, "Caymn",0,NULL },	/* jrd */
	{ 0x000093, "Prote",0,NULL },
	{ 0x00009f, "Ameri",0,NULL },	/* jrd */
	{ 0x0000a9, "NetSy",0,NULL },	/* jrd */
	{ 0x0000aa, "Xerox",0, NULL },
	{ 0x0000b3, "CIMLi",0,NULL },	/* jrd */
	{ 0x0000c0, "WsDig",0, NULL },	/* jrd */
	{ 0x0000dd, "Gould",0,NULL },
	{ 0x0000e2, "Acer ",0,NULL },	/* jrd */
	{ 0x000102, "BBN  ",0,NULL },
	{ 0x001700, "Kabel",0,NULL },
	{ 0x00aa00, "Intel",0,NULL },	/* jrd */
	{ 0x00dd00, "UBass",0, NULL },
	{ 0x00dd01, "UBass",0,NULL },	/* jrd */
	{ 0x020701, "Intrl",0, NULL },
	{ 0x02608c, "3Com ",0, NULL },	/* jrd */
	{ 0x02cf1f, "CMC  " ,0,NULL},
	{ 0x080002, "Bridg",0, NULL },
	{ 0x080002, "ACC  ",0,NULL },	/* jrd */
	{ 0x080005, "Symbl",0, NULL },
	{ 0x080009, "HP   ",0, NULL },
	{ 0x08000a, "Nesta",0,NULL },	/* jrd */
	{ 0x080010, "AT&T ",0, NULL },
	{ 0x080014, "Excln",0, NULL },
	{ 0x080017, "NSC  ",0,NULL },
	{ 0x08001a, "DG   ",0,NULL },
	{ 0x08001b, "DG   ",0,NULL },	/* jrd */
	{ 0x08001e, "Apollo",0,NULL },	/* jrd */
	{ 0x080020, "Sun  ",0, NULL },
	{ 0x080022, "NBI  ",0, NULL },
	{ 0x080028, "TI   ",0, NULL },
	{ 0x08002b, "dec  ",0, NULL },
		/* Unibus, Qbus, LANBridges (DEUNA, DEQNA, DELUA) */
	{ 0x08002f, "Prime",0, NULL },
	{ 0x080036, "Intgr",0, NULL },
	{ 0x080045, "Spidr",0, NULL },
	{ 0x080047, "Seque",0,NULL },
	{ 0x080049, "Univa",0,NULL },
	{ 0x08004c, "Encor",0,NULL },
	{ 0x08004e, "BICC ",0,NULL },
	{ 0x08005a, "IBM  ",0, NULL },
	{ 0x080067, "Comde",0, NULL },
	{ 0x080068, "Ridge",0,NULL },
	{ 0x080069, "SiGra",0, NULL },
	{ 0x08006e, "Excln",0,NULL },
	{ 0x080075, "DDE  ",0, NULL },
	{ 0x08007c, "Vital",0, NULL },
	{ 0x080086, "Imagn",0, NULL },
	{ 0x080087, "Xpylx",0, NULL },
	{ 0x080089, "Kinet",0, NULL },
	{ 0x08008b, "Pyram",0,NULL },
	{ 0x08008d, "XyVis",0,NULL },
	{ 0x090002, "Vital",0,NULL },	/* jrd */
	{ 0x09002b, "DECLB",0, NULL },
	{ 0x09004e, "Novel",0, NULL },
	/* DEC LanBridge Hello packet multicast address 09002b010001 */
	{ 0x0000b5, "Vista",0, NULL },	/* jrd */
	{ 0x0de000, "Dlink",0, NULL },	/* jrd */
	{ 0x00de01, "Novel",0, NULL },	/* Novell NE1000? jrd*/
	{ 0xaa0003, "DEC  ",0, NULL },
		/* Physical address for some DEC machines */
	{ 0xaa0004, "Dec  ",0, NULL },
		/* Logical address for DECnet */
	{ 0xab0000, "DecMC",0, NULL },
	/* DEC Multicast addresses:
		AB0000010000 DEC_MOP_DLA
		AB0000020000 DEC_MOP_RC
		AB0000030000 DECnet Phase IV end node Hello packets
		AB0000040000 DECnet Phase IV router Hello packets
	*/
	{ 0xab0003, "LAT* ",0,NULL },
	{ 0xab0004, "LaVax",0,NULL },
	{ 0xcf0000, "LoopB",0, NULL }
		/* Ethernet Configuration Protocol MCast */
	};

char manu_switch = FALSE;

long lswap();

char *unparse_ether(addr)
	char *addr; {
	static char buffer[80];
	struct nameber *n;
	unsigned long a;

	if((*(long *)addr == 0xFFFFFFFF) &&
	   (*(unsigned *)(addr+4) == 0xFFFF))
		return "      *      ";

	if(manu_switch) {
		a = *(unsigned long *)addr;
		a = (lswap(a) >> 8) & 0xFFFFFF;
		n = lookup(ether_makers, a);

		if(n) {
			sprintf(buffer, "%s %02x%02x%02x", n->n_name,
					mkbyte(addr[3]), mkbyte(addr[4]),
					mkbyte(addr[5]));
			return buffer;
			}
		}

	sprintf(buffer, "%02x%02x%02x%02x%02x%02x ", mkbyte(addr[0]),
			mkbyte(addr[1]), mkbyte(addr[2]), mkbyte(addr[3]),
					mkbyte(addr[4]), mkbyte(addr[5]));
	return buffer;
	}

/* stub the internet demultiplexer
*/
indemux() {
}

in_stats() {
}
