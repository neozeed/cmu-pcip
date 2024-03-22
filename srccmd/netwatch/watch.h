/* Copyright 1986 by Carnegie Mellon */
/*  Copyright 1986 by Carnegie Mellon  */
/*  See permission and disclaimer notice in file "cmu-note.h"  */
#include	<cmu-note.h>
/*  Copyright 1984, 1985 by the Massachusetts Institute of Technology  */
/*  See permission and disclaimer notice in file "notice.h"  */

/* structure used for describing protocols & numbers */
struct nameber {
	unsigned long n_number;
	char *n_name;
	long n_count;		/* statistics keeping */
	struct layer *n_layer;	/* pointer to this nameber's layer */
	char	n_color;	/* color for special effects */
	};

/* lookup by number */
extern struct nameber *lookup();
/* lookup by name */
extern struct nameber *nlookup();

/* root of the lists */
extern struct layer *root_layer;

/* and this describes a layer and is intimately interconnected with a
	the nameber structure
   the parsing routines are all in the namebers.c file. The filters are
	all in the command interpreter and are called from there. They
	link through the protocol layers and understand where in the
	packet address fields and type fields occur. Most layers won't
	have addresses. Although you might want to talk about "all
	incoming telnet packets" I don't think I'll let you.

   namebers and layers are separates structures because I can't put
	struct nameber foo[];
	as a field in the layer structure without the C compiler bitching.
*/

struct layer {
	struct layer   *l_parent;	/* nameber which points to me */
	struct nameber *l_children;	/* nameber which I point to */
	long l_unknown;			/* packets of this layer of ? type */
	int (*l_parse)();		/* this layer's unparsing routine */
	int (*l_addr)();		/* address filter */
	int (*l_type)();		/* type filter */
	};


/* pattern and byte stuff. A pattern is a queue of bytesses. A "bytes"
	is a set of bytes and an offset where they're supposed to be
	found in a packet. A packet matches a pattern if all the bytes
	specified in the pattern are found at the appropriate offsets
	in the packet.
*/

/* Have to change pt_match.a86 if this structure or MATCH_DATA_LEN
	changes.
   High byte of each pattern entry is a mask: 0xff if this byte matters,
	0x00 if it doesn't. The low byte is the value to match against.
*/
typedef struct _pattern {
	unsigned	bytes[MATCH_DATA_LEN];
	unsigned	count;
	} pattern;

/* a few random constants */
#define	MD_NORMAL	0
#define	MD_SYMBOLIC	1

/* return codes from filters */
#define	F_OK		0
#define	F_ERROR		1
#define	F_PAUSE		2

#define	mkbyte(x)	((x)&0377)

extern unsigned prot_mode;
extern pattern pat1, pat2;
extern char question[];
extern char unknown_match;		/* match only unknown packets */

/* protocol layer display flags
*/
extern int disp_network, disp_internet, disp_transport, disp_app;

extern char *index();

#ifdef	LENGTH_HISTO
/* histogram stuff */
#define	HIST_INCR	64	/* 64 byte size increments */
#define	HIST_SHIFT	6
#define	HIST_MAX_LEN	2048
#define	HIST_NUM_INCS	HIST_MAX_LEN/HIST_INCR

extern unsigned long hist_counts[];
extern unsigned long hist_too_big;
#endif
