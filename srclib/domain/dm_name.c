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
#include <udp.h>
#include <timer.h>
#include <sockets.h>
#include <ctype.h>
#include "domain.h"

/* PC/IP domain name resolver - written summer 1985 by John Romkey, MIT-LCS-DCS
*/

#define	ST_QSENT	1	/* query sent */
#define	ST_TMO		2	/* timeout */
#define	ST_RESPONSE	3	/* got a response */
#define	ST_ERROR	4	/* error response from servers */
#define	ST_NETERR	5	/* network error - couldn't send request */

char req_cname[100];	/* canonical name of host we're resolving */
static in_name primary;		/* primary name server */
static current_id = 0;
static in_name address;
static in_name curr_server;
static int state;
static task *waiter;

int dm_timeout = 10;	/*  Time, in seconds, to wait for a response.  */
int dm_rcv();
#ifndef MSC			/* DDP */
char	*index();
#else				/* DDP */
char	*strchr();		/* DDP */
#endif				/* DDP */

static dm_tmo() {
	tk_wake(waiter);
	state = ST_TMO;
	}

static dm_durcv(pip, host, port)
struct ip *pip;
in_name host;
unsigned port;
{
	if(curr_server != host || port != UDP_DOMAIN) {
		return;
	}
	tk_wake(waiter);
	state = ST_NETERR;
	}

in_name dm_resolve(name, server)
	register char *name;
	in_name server; {
	register timer *tm;
	UDPCONN udp;
	unsigned local_socket;

	local_socket = udp_socket();

	udp = udp_open(server, UDP_DOMAIN, local_socket, dm_rcv, 0);
	if(udp == NULL)
		return NAMETROUBLE;
	udp_duopen(udp, dm_durcv);	/* DDP */

	tm = tm_alloc();
	if(tm == NULL) {
		return NAMETROUBLE;
		}

	strcpy(req_cname, name);
	curr_server = server;

	/* send the request */
	if(!send_req(req_cname, server, udp, local_socket)) {
		state = ST_NETERR;
		goto done;
		}

	state = ST_QSENT;

	/* set up timer. if it expires before we get back a response
		then we have to poll some more name servers if we know
		any more
	*/
	waiter = tk_cur;

	tm_set(dm_timeout, dm_tmo, 0, tm);

	while(state == ST_QSENT)
		tk_block();

done:
	tm_clear(tm);
	tm_free(tm);

	udp_close(udp);

	if(state == ST_RESPONSE)
		return address;

	if(state == ST_TMO)
		return NAMETMO;

	if(state == ST_NETERR)
		return NAMETROUBLE;

	return NAMEUNKNOWN;
	}

send_req(name, server, udp, local_socket)
	char *name;
	in_name server;
	UDPCONN udp;
	unsigned local_socket; {
	PACKET p;
	register struct dm_hdr *dmp;
	register char *data;
	int length;
	char buffer[400];

	p = udp_alloc(512, 0);
	if(p == NULL) {
		state = ST_TMO;
		tk_wake(waiter);
		udp_close(udp);
		return FALSE;
		}

	dmp = (struct dm_hdr *)udp_data(udp_head(in_head(p)));

	/* fill in header, byte swapping as needed */
	dmp->dm_id = ++current_id;
	dmp->dm_flags = DM_DO_RECURSE;
	put_opcode(dmp, OP_QUERY);
	dmp->dm_qd_count = 1;
	dmp->dm_an_count = 0;
	dmp->dm_ns_count = 0;
	dmp->dm_ar_count = 0;
	put_rc(dmp, RC_NO_ERR);

	dmp->dm_id = bswap(dmp->dm_id);
	dmp->dm_flags = bswap(dmp->dm_flags);
	dmp->dm_qd_count = bswap(dmp->dm_qd_count);

	data = (char *)(dmp+1);		/* right after the domain header */

	/* format the query:
		1. query name: compressed domain name
		2. query type: always A
		3. query class: IN
	*/
	length = compress_name(name, data);

	data += length;
	*(unsigned *)data = bswap(A);
	data++; data++;
	*(unsigned *)data = bswap(IN);

/*	if((NDEBUG & (APTRACE|DUMP)) == (APTRACE|DUMP))
		dm_dump(dmp);
*/
	/* compute true length */
	length += 4 + sizeof(struct dm_hdr);	/* 4 bytes in query header */
	udp_send_to(server, UDP_DOMAIN, local_socket, p, length);
{int i; for(i = 0; i < 10000; i++);}

	udp_free(p);

	return TRUE;
	}

compress_name(name, obuf)
	register char *name;
	char *obuf; {
	register char *buf = obuf;
	char *ptr;
	int len;

	while(1) {
#ifndef MSC					/* DDP */
		ptr = index(name, '.');
#else						/* DDP */
		ptr = strchr(name, '.');	/* DDP */
#endif						/* DDP */

		/* is it the last one? */
		if(ptr == NULL) {
			/* yes - copy it and terminate the name */
			*buf++ = len = strlen(name);
			strcpy(buf, name);
			buf[len] = '\0';
			return buf - obuf + len + 1;
			}

		*ptr = '\0';

		*buf++ = len = strlen(name);
		strcpy(buf, name);
		buf += len;
		name = ptr + 1;
		*ptr = '.';
		}
	}

dn_uncompress(name, buffer, msg)
	register char *name;
	register char *buffer;
	struct dm_hdr *msg; {
	char *msg_start = (char *)msg;
	int ret_val = 0;
	int pointered = FALSE;
	int ins_dot;
	unsigned len = 0;

	strcpy(buffer, "");

	ins_dot = FALSE;

	while(1) {
		if(!pointered)
			ret_val += len+1;

		len = (*name++) & 0xff;		/* defeat sign extension */

		if(len == 0) {
			return ret_val;
			}

		if((len & 0xc0) == 0xc0) {
			if(!pointered) {
				if(ret_val == 1)
					ins_dot = FALSE; /* DDP */
				ret_val++;
				}

			pointered = TRUE;
			len = (*name++) & 0xff;
			name = msg_start + len;
			continue;
			}

		if(ins_dot) {
			strcat(buffer, ".");
			}

		ins_dot = TRUE;

		strncat(buffer, name, len);

		name += len;
		}
	}

static char *opcodes[] = {
	"QUERY",
	"IQUERY",
	"CQUERYM",
	"CQUERYU"
	};

static char *responses[] = {
	"No error",
	"Format error",
	"Server error",
	"Name error",
	"Not implemented",
	"Refused"
	};

static char *types[] = {
	"illegal (0)",
	"A",
	"NS",
	"MD",
	"MF",
	"CNAME",
	"SOA",
	"MB",
	"MG",
	"MR",
	"NULL",
	"WKS",
	"PTR",
	"HINFO",
	"MINFO"
	};

static char *classes[] = {
	"illegal (0)",
	"IN",
	"CS"
	};


dm_dump(dmp)
	register struct dm_hdr *dmp; {
	register char *data;
	int i;

#ifdef	DEBUG
	if(NDEBUG & APTRACE) {
		printf("id %u\tqds %u\tans %d\tnss %d\tars %d\n",
				bswap(dmp->dm_id), bswap(dmp->dm_qd_count),
						bswap(dmp->dm_an_count),
						bswap(dmp->dm_ns_count),
						bswap(dmp->dm_ar_count));

		printf("flags: ");

		if(dmp->dm_flags & DM_RESPONSE)
			printf("<RESPONSE>");
		else printf("<QUERY>");

		if(dmp->dm_flags & DM_AUTHORITY)
			printf("<AUTHORITY>");

		if(dmp->dm_flags & DM_TRUNCATED)
			printf("<TRUNCATED>");

		if(dmp->dm_flags & DM_CAN_RECURSE)
			printf("<CAN_RECURSE>");

		if(dmp->dm_flags & DM_DO_RECURSE)
			printf("<DO_RECURSE>");

		printf("\nopcode %s (%u)\tresponse code %s (%u)\n",
				opcodes[get_opcode(dmp)], get_opcode(dmp),
				responses[get_rc(dmp)], get_rc(dmp));

		}
#endif

	data = (char *)(dmp+1);

	for(i = bswap(dmp->dm_qd_count); i; i--)
		data += dsp_query(data, dmp);

	for(i = bswap(dmp->dm_an_count); i; i--)
		data += dsp_rr(data, dmp);

	for(i = bswap(dmp->dm_ns_count); i; i--)
		data += dsp_rr(data, dmp);

	for(i = bswap(dmp->dm_ar_count); i; i--)
		data += dsp_rr(data, dmp);

	}

#ifdef	DEBUG
static char *qtypes[] = {
	"*",
	"MAILA",
	"MAILB",
	"AXFR"
	};

static char *qclasses[] = {
	"*"
	};
#endif

dsp_query(odata, msg)
	char *odata;
	struct dm_hdr *msg; {
	register char *data = odata;
	char rsp_name[100];
	char rsp_cname[100];
	char buffer[200];
	unsigned w;

	data += dn_uncompress(data, rsp_name, msg);

#ifdef	DEBUG
	if(NDEBUG & APTRACE)
		printf("query: %s ", rsp_name);
#endif

	w = bswap(*(unsigned *)data);
	data += sizeof(unsigned);

#ifdef	DEBUG
	if(NDEBUG & APTRACE) {
		printf("type: ");
		if(w < sizeof(types)/sizeof(char *))
			printf("%s (%04x)", types[w], w);
		else if(255 - w < sizeof(qtypes)/sizeof(char *))
			printf("%s (%04x)", qtypes[255-w], w);
		else printf("<unknown type %d> ", w);
		}
#endif

	w = bswap(*(unsigned *)data);
	data += sizeof(unsigned);

#ifdef	DEBUG
	if(NDEBUG & APTRACE) {
		printf(" class: ");
		if(w < sizeof(classes)/sizeof(char *))
			printf("%s (%04x)", classes[w], w);
		else if(255 - w < sizeof(qclasses)/sizeof(char *))
			printf("%s (%04x)", qclasses[255-w], w);
		else printf("<unknown type %d> ", w);

		printf("\n");
		}
#endif
	return data-odata;
	}

extern long lswap();

dsp_rr(odata, msg)
	char *odata;
	struct dm_hdr *msg; {
	register char *data = odata;
	char name[200];
	char buffer[200];
	unsigned type, class;
	unsigned len;
	unsigned long ttl;
	int i;
	in_name addr;

#ifdef	DEBUG
	if((NDEBUG & (DUMP|APTRACE)) == (DUMP|APTRACE)) {
		for(i=0; i<80; i++) {
			printf("%02x ", data[i] & 0xff);
			if((i+1)%20 == 0)
				printf("\n");
			}
		}
#endif

	data += dn_uncompress(data, name, msg);

#ifdef	DEBUG
	if(NDEBUG & APTRACE)
		printf("rr: %s ", name);
#endif

	type = bswap(*(unsigned *)data);
	data += sizeof(unsigned);

#ifdef	DEBUG
	if(NDEBUG & APTRACE) {
		printf("type: ");
		if(type < sizeof(types)/sizeof(char *))
			printf("%s (%04x)", types[type], type);
		else if(255 - type < sizeof(qtypes)/sizeof(char *))
			printf("%s (%04x)", qtypes[255-type], type);
		else printf("<unknown type %04x>", type);
		}
#endif

	class = bswap(*(unsigned *)data);
	data += sizeof(unsigned);

#ifdef	DEBUG
	if(NDEBUG & APTRACE) {
		printf("class: ");
		if(class < sizeof(classes)/sizeof(char *))
			printf("%s (%04x)", classes[class], class);
		else if(255 - class < sizeof(qclasses)/sizeof(char *))
			printf("%s (%04x)", qclasses[255-class], class);
		else printf("<unknown type %04x>", class);
	}
#endif

	ttl = lswap(*(long *)data);
	data += sizeof(long);

#ifdef	DEBUG
	if(NDEBUG & APTRACE)
		printf(" TTL=%D\n", ttl);
#endif

	len = bswap(*(unsigned *)data);
	data += sizeof(unsigned);

	if(type == CNAME) {
		dn_uncompress(data, buffer, msg);
#ifdef	DEBUG
		if(NDEBUG & APTRACE)
			printf("  CNAME: %s", buffer);
#endif

		data += len;

		/* if this is the cname of the host we're looking for,
			replace what we think is its cname with the
			correct cname.
		*/
		if(cslesscmp(name, req_cname))
			strcpy(req_cname, buffer);

		}
	else
	if(type == NS) {
		dn_uncompress(data, buffer, msg);
		data += len;
		}
	else
	if(type == A) {
		addr = *((in_name *)data);
#ifdef	DEBUG
		if(NDEBUG & APTRACE)
			printf("  address: %a ", addr);
#endif

		data += len;

		if(cslesscmp(name, req_cname)) {
#ifdef	DEBUG
			if(NDEBUG & APTRACE)
				printf("--and for the requested name!\n");
#endif
			address = addr;
			state = ST_RESPONSE;
			tk_wake(waiter);
			}
		}
	else {
#ifdef	DEBUG
		if(NDEBUG & APTRACE) {
			printf("data length = %d\n", len);
			for(i=0; i<len; i++) {
				printf("%02x ", data[i] & 0xff);
				if(i%20 == 0)
					printf("\n");
				}
			}
#endif
		data += len;
		}

#ifdef	DEBUG
	if(NDEBUG & APTRACE)
		printf("\n");
#endif
	return data-odata;
	}

dm_rcv(p, len, host)
	PACKET p;
	unsigned len;
	in_name host; {
	register struct dm_hdr *dmp;
	register char *buf;
	int i;

#ifdef	DEBUG
	if(NDEBUG & APTRACE)
		printf("received response of length %d from %a\n", len, host);
#endif
	dmp = (struct dm_hdr *)udp_data(udp_head(in_head(p)));
	dm_dump(dmp);

	dmp->dm_id = bswap(dmp->dm_id);
	dmp->dm_flags = bswap(dmp->dm_flags);
	dmp->dm_qd_count = bswap(dmp->dm_qd_count);
	dmp->dm_an_count = bswap(dmp->dm_an_count);
	dmp->dm_ns_count = bswap(dmp->dm_ns_count);
	dmp->dm_ar_count = bswap(dmp->dm_ar_count);

	if(dmp->dm_id != current_id) {
		udp_free(p);
		return;
		}

	if(get_rc(dmp) != RC_NO_ERR) {
#ifdef	DEBUG
		if(NDEBUG & APTRACE)
			printf("error from server: %s\n", responses[get_rc(dmp)]);
#endif
		state = ST_ERROR;
		tk_wake(waiter);
		}

	udp_free(p);
	}

cslesscmp(s1, s2)
	register char *s1, *s2; {

	while(1) {
		if((islower(*s1) ? (*s1) - 'a' + 'A' : (*s1)) != 
		   (islower(*s2) ? (*s2) - 'a' + 'A' : (*s2)))
			return FALSE;
		if(!*s1) return TRUE;
		s1++; s2++;
		}
	}
