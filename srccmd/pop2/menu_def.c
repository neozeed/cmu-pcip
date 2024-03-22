/*  Copyright 1987, 1988 by USC Information Sciences Institute (ISI) */
/*  See permission and disclaimer notice in file "isi-note.h"  */
#include	"isi-note.h"

/*  Copyright 1986 by Carnegie Mellon  */
/*  See permission and disclaimer notice in file "cmu-note.h"  */
#include	<cmu-note.h>

/*  Copyright 1984, 1985 by the Massachusetts Institute of Technology  */
/*  See permission and disclaimer notice in file "notice.h"  */
#include	<notice.h>

/*  Dale Chase's mods for set_uname() to query for user's password as
    well as username has been added.  This is so that the pop2 program
    can extract the user's password from 'netdev.sys.'  This method,
    however, is not recommended.  See 'pop2.c' for more details.
							5/28/87  -Koji */


#include	"menu.h"
#include	<stdio.h>
#include	<task.h>
#include	<q.h>
#include	<netq.h>
#include	<net.h>
#include	<custom.h>
#include	<attrib.h>
#include	<em.h>

#define baud_19200	0x0006	/* divisors for the different baud rates */
#define baud_9600	0x000c
#define	baud_4800	0x0018
#define	baud_2400	0x0030
#define	baud_1200	0x0060
#define	baud_600	0x00c0
#define	baud_300	0x0180
#define	baud_110	0x0417

extern struct custom	custom;
struct custom          *p_custom;	/* used in set_uname() - koji */
extern unsigned UNDER_ATTRIB;

extern	long	resolve_name();

long	get_dosl();
char	*baud_string();
char	*debug_string();
int	noecho;		/* no-echo flag */

p_speed()
{
	int	s_attrib;

	s_attrib = attrib;
	attrib = UNDER_ATTRIB;
	printf("Speed is %s", baud_string(custom.c_baud));
	attrib = s_attrib;
	printf("\n\033K\n\033K");
}

/* baud_string(baud) - returns a string that tells the baud rate
 *	the argument is the divisor for some baud rate
 */

char *baud_string(baud)
int	baud;
{
  switch(baud) {
  case baud_110:	return("110 baud");
  case baud_300:	return("300 baud");
  case baud_600:	return("600 baud");
  case baud_1200:	return("1200 baud");
  case baud_2400:	return("2400 baud");
  case baud_4800:	return("4800 baud");
  case baud_9600:	return("9600 baud");
  case baud_19200:	return("19200 baud");
  default:		return("unknown baud rate");
  }
}

p_time()
{
	long	time;
	
	time = custom.c_ctime;
	printf("%2d:%02d:%02d",(int)(time >> 24),
			(int)(time >> 16) & 0xff,
			(int)(time >> 8) & 0xff);
}

char	*month[] = { "FOO","Jan","Feb","Mar","Apr","May","Jun","Jul","Aug",
	"Sep","Oct","Nov","Dec" };

p_date()
{
	long	date;
	
	date = custom.c_cdate;
	printf("%d-%s-%d", (int)(date & 0xff),
			month[(int)(date >> 8) & 0xff],
			(int)(date >> 16));
}


my_addr()
{
	set_addr(&custom.c_me, "my");

	/* may need to change the subnet mask here */
	fixup_subnet_mask();

	return 0;
}

log_addr() {
	set_addr(&custom.c_log, "the log server's");
	return 0;
	}

defgw_addr() {
	set_addr(&custom.c_defgw, "the default gateway's");
	return 0;
	}

printer_addr() {
	set_addr(&custom.c_printer, "the print server's");
	return 0;
	}

/* Set internet addresses */
set_addr(where, name)
long	*where;
char	*name;
{
	char	buf[80];
	long	ret;
	int	s_attrib;
	char	prompt[80];

	printf("\r\033K");
	strcpy(prompt, "Enter ");
	strcat(prompt, name);
	strcat(prompt, " internet address ");
	get_line(prompt, buf, 80);
	if ((ret = resolve_name(buf)) == -1L) {
		printf("\n\033K");
		printf("Not a readable address");
		printf("\n\033K");
		s_attrib = attrib;
		attrib = INVERT;
		printf("Invalid address. Type <sp> to continue.");
		attrib = s_attrib;
		while (h19key() != ' ')
			;
		return(0);
	}
	*where = ret;
	return(0);
}

get_line(prompt, buf, size)
char	*prompt;
char	buf[];
int	size;
{
	char	c;
	int	pos = 0;
	int	save_attrib;
	
    save_attrib = attrib;
    attrib = UNDER_ATTRIB;
    printf("%s", prompt);
    attrib = save_attrib;
    while(1) {
	while ((c = h19key()) == NONE)
		;
	switch(c) {
	case 0177:	if (pos == 0)
	/* delete */		continue;
			pos--;
			if (!noecho) printf("\010 \010");
			break;
	case 21:	pos = 0;
	/* ^U */	printf("\r%cK",27);
			save_attrib = attrib;
			attrib = UNDER_ATTRIB;
			printf("%s",prompt);
			attrib = save_attrib;
			break;
	case '\n':	buf[pos] = '\0';
	/* newline */	return;
	case '\r':	break;
	/* cariage return; just ignore it */
	default:	if (c < ' ')
				continue;
			buf[pos++] = c;
			if (!noecho) putchar(c);
			if (pos >= size)
				pos = size - 1;
			break;
	}
    }
}

set_speed(speed)
int	speed;
{
	custom.c_baud = speed;
	return(1);
}

/* toggle bit in debug field */
debug(bit)
unsigned	bit;
{
	custom.c_debug ^= bit;
	return(0);
}

print_debug()
{
	int	s_attrib;

	s_attrib = attrib;
	attrib = UNDER_ATTRIB;
	printf("Debug options are:\n\033K%s", debug_string());
	attrib = s_attrib;
	printf("\n\033K\n\033K");
}

char	d_str[80];
char *debug_string()
{
	if (custom.c_debug == 0)
		return("all off");
	d_str[0] = ' ';
	d_str[1] = '\0';
	if (custom.c_debug & DUMP)	strcat(d_str, "dump ");
	if (custom.c_debug & BUGHALT)	strcat(d_str, "bughalt ");
	if (custom.c_debug & INFOMSG)	strcat(d_str, "info ");
	if (custom.c_debug & NETERR)	strcat(d_str, "neterrs ");
	if (custom.c_debug & PROTERR)	strcat(d_str, "proterrs ");
	if (custom.c_debug & TMO)	strcat(d_str, "timeout ");
	if (custom.c_debug & NETRACE)	strcat(d_str, "nettrace ");
	if (custom.c_debug & IPTRACE)	strcat(d_str, "IPtrace ");
	if (custom.c_debug & TPTRACE)	strcat(d_str, "TPtrace ");
	if (custom.c_debug & APTRACE)	strcat(d_str, "APtrace ");
	return(d_str);
}

/* add another name server to the end of the list */
add_name()
{
	add_server(&custom.c_numname, 2, custom.c_names);
	return(0);
}

/* add another name server to the end of the list */
add_dm_name()
{
	add_server(&custom.c_dm_numname, 3, custom.c_dm_servers);
	return(0);
}

add_time()
{
	add_server(&custom.c_numtime, MAXTIMES, custom.c_time);
}

add_server(num, max_num, addrs)
int	*num;
int	max_num;
in_name	addrs[];
{
	char	buf[80];
	long	addr;
	
	printf("\r\033K");
	if (*num >= max_num) {
		printf("The list is full\n\033K");
		attrib = INVERT;
		printf("Type <sp> to continue ");
		attrib = NORMAL;
		while (h19key() != ' ')
			;
		return(0);
	}
	get_line("Enter the address ", buf, 80);
	if ((addr = resolve_name(buf)) == -1L) {
		printf("\033K\n\033KNot a valid name\n\033K");
		attrib = INVERT;
		printf("Type <sp> to continue ");
		attrib = NORMAL;
		while (h19key() != ' ')
			;
		return(0);
	}
	addrs[(*num)++] = addr;
	return(0);
}

/* Delete a name server from the list. Move the rest of the list up so that
 * the list is contiguous. */
del_names()
{
	int	number;
	int	c;
	
	printf("\r\033KEnter the number of the entry to delete. ");
	while ((c = h19key()) == -1)
		;
	number = c - '0';
	if (number > custom.c_numname || number <= 0) {
		printf("\n\033KThere is no name server with that number\n\033K");
		attrib = INVERT;
		printf("Type <sp> to continue");
		attrib = NORMAL;
		while (h19key() != ' ')
			;
		return(0);
	}
	custom.c_numname--;
	for (number--; number < custom.c_numname; number++)
		custom.c_names[number] = custom.c_names[number + 1];
	return(0);
}

/* Delete a name server from the list. Move the rest of the list up so that
 * the list is contiguous. */
del_dm_names()
{
	int	number;
	int	c;
	
	printf("\r\033KEnter the number of the entry to delete. ");
	while ((c = h19key()) == NONE) ;

	number = c - '0';
	if (number > custom.c_dm_numname || number <= 0) {
		printf("\n\033KThere is no name server with that number\n\033K");
		attrib = INVERT;
		printf("Type <sp> to continue");
		attrib = NORMAL;
		while (h19key() != ' ')
			;
		return(0);
	}
	custom.c_dm_numname--;
	for (number--; number < custom.c_dm_numname; number++)
		custom.c_dm_servers[number] = custom.c_dm_servers[number + 1];
	return(0);
}

del_times()
{
	int	number;
	int	c;
	
	printf("\r\033KEnter the number of the entry to delete. ");
	while ((c = h19key()) == -1)
		;
	number = c - '0';
	if (number > custom.c_numtime || number <= 0) {
		printf("\n\033KThere is no time server with that number\n\033K");
		attrib = INVERT;
		printf("Type <sp> to continue");
		attrib = NORMAL;
		while (h19key() != ' ')
			;
		return(0);
	}
	custom.c_numtime--;
	for (number--; number < custom.c_numtime; number++)
		custom.c_time[number] = custom.c_time[number + 1];
	return(0);
}

p_names()
{
	attrib = UNDER_ATTRIB;
	printf("The Name Servers are");
	attrib = NORMAL;
	printf("\n\033K");
	p_servers(custom.c_numname, custom.c_names);
	printf("\n\033K");
}

p_dm_names()
{
	attrib = UNDER_ATTRIB;
	printf("The Domain Name Servers are");
	attrib = NORMAL;
	printf("\n\033K");
	p_servers(custom.c_dm_numname, custom.c_dm_servers);
	printf("\n\033K");
}

p_times()
{
	int	i;
	
	attrib = UNDER_ATTRIB;
	printf("The Time Servers are");
	attrib = NORMAL;
	printf("\n\033K");
	p_servers(custom.c_numtime, custom.c_time);
	printf("\n\033K");
}

/* set the user's name and password -
   (password is used by the pop2 program only) - koji */
set_uname()
{
	char	name[80];
	char	*p, *q;
	int	namesiz;
	
	printf("\r\033K");
	p_custom = &custom;
	get_line("Enter user name ", name, 80);
	strcpy(custom.c_user, name);
	q = p_custom->c_user;
	for (namesiz = 80, p = name; *p != '\0'; p++, namesiz--)
		q++;		/* get to end of user's name */
	noecho = 1;		/* echo off */
	get_line(" Now enter password ", ++p, --namesiz);	/* get pw */
	noecho = 0;		/* echo back on */
	strcpy(++q, p);		/* copy pw */
	return(0);
}

p_servers(numb,addrs)
int	numb;
in_name	addrs[];
{
	int	i;

	if (numb == 0)
		printf("\tNone");
	for (i=0; i < numb; i++) {
		printf("\t %d) %A", i+1, addrs[i]);
		if ((i+1) % 3 == 0)
			printf("\n\033K");
		else	printf("\t");
	}
	printf("\n\033K");
}

p_site()
{
	int	i;
	

	printf("My Internet Address\tLog Server\tDefault Gateway\n\033K");
	printf("%A\t\t", custom.c_me);
	if(custom.c_me == 0) printf("\t"); /* this is stupid. */
	printf("%A\t", custom.c_log);
	if(custom.c_log == 0) printf("\t");
	printf("%A\n\033K", custom.c_defgw);

	printf("Print server: %A\tDomain: %s\n\033K", custom.c_printer, custom.c_domain);
	if (custom.c_numtime)
		printf("There are %d time servers, ", custom.c_numtime);
	else
		printf("There are no time servers, ");

	if (custom.c_numname)
		printf("%d old name servers and ", custom.c_numname);
	else
		printf("no old name servers and ");

	if(custom.c_dm_numname)
		printf("%d domain servers\n\033K", custom.c_dm_numname);
	else
		printf("no domain servers.\n\033K");

	printf("There are %d bits of subnet; the subnet mask is %A\n\033K",
				custom.c_subnet_bits, custom.c_net_mask);
	p_tzone();
	printf("The first RVD drive is %c:.\n\033K", custom.c_rvd_base);
	printf("\n\033K");
}

p_hardware() {
	_p_ethadd();
	printf(". CSR base I/O address 0x%04x\n\033K", custom.c_base);
	printf("Interrupt vector %u\n\033K", custom.c_intvec);
	printf("Transmit DMA channel %u, Receive DMA channel %u\n\033K",
				custom.c_tx_dma, custom.c_rcv_dma);
	printf("Serial line speed is %s\n\033K", baud_string(custom.c_baud));
	printf("This machine has %d net interfaces\n\033K", custom.c_num_nets);
	printf("\n\033K");
	}

p_top()
{
	printf("Last Customized at ");
	p_time();
	printf(" on ");
	p_date();
	printf("\n\033K");

	printf("User's name is %s\n\033K", custom.c_user);

	printf("Debug:\n\033K%s\n\033K", debug_string());
	printf("Radix for displaying internet addresses is ");
	switch(custom.c_ip_radix) {
	case 010:
		printf("octal");
		break;
	case 10:
		printf("decimal");
		break;
	default:
		printf("illegally set");
		}

	printf("\n\033KThis machine has %d net interfaces\n\033K",
							custom.c_num_nets);

	printf("\n\033K\n\033K");
}

p_eic()
{
	int	i;

	printf("This is only a temporary fix. Eventually all will use\n\033K");
	printf("Plummer's address resolution protocol and this will go away.\n\033K");
	printf("Read the manual before using this menu.\n\033K");
	printf("\n\033K");
	attrib = UNDER_ATTRIB;
	printf("Internet");
	attrib = NORMAL;
	printf("\t");
	attrib = UNDER_ATTRIB;
	printf("EtherNet");
	attrib = NORMAL;
	printf("\n\033K");
	for (i=0; i < 3; i++) {
		printf("%A\t-----\t", custom.c_ipname[i]);
		out_ethaddr(&custom.c_ether[i]);
		printf("\n\033K");
	}
	printf("\n\033K");
}

out_ethaddr(ead)
struct etha	*ead;
{
	printf("%03o,%03o,%03o,%03o,%03o,%03o",
			ead->e_ether[0] & 0377, ead->e_ether[1] & 0377,
			ead->e_ether[2] & 0377, ead->e_ether[3] & 0377,
			ead->e_ether[4] & 0377, ead->e_ether[5] & 0377);
}

set_eic(which)
int	which;
{
	char	buf[80];
	in_name	iad;
	struct etha	ead;
	int	i,count;
	
	for (i = 0; i < 6; i++)
		ead.e_ether[i] = 0;
	printf("\r\033K");
	get_line("Enter Internet address", buf, 80);
	if ((iad = resolve_name(buf)) == -1L) {
		printf("\n\033KNot a valid address, clearing entry");
		iad = 0L;
	}
	printf("\n\033K");
	get_line("Enter EtherNet address", buf, 80);
	count = 0;
	for (i = 0; i < 80; i++) {
		if (buf[i] == '\0') break;
		if ((buf[i] <= '7') && (buf[i] >= '0')) {
			ead.e_ether[count] = (ead.e_ether[count] << 3) + buf[i] - '0';
			continue;
		}
		else {
			count++;
			if (count > 6) break;
		}
	}
	custom.c_ipname[which] = iad;
	for (i = 0; i < 6; i++)
		custom.c_ether[which].e_ether[i] = ead.e_ether[i];
	return(0);
}

/* set the ip address output radix
*/
set_radix() {
	char buf[80];
	extern unsigned _printf_ip_radix;

	get_line("\r\033KPrint Internet addresses in octal or decimal? ",
								buf, 80);

	if(strcmp(buf, "o") == 0 || strcmp(buf, "octal") == 0 ||
	   strcmp(buf, "O") == 0 || strcmp(buf, "Octal") == 0 ||
	   strcmp(buf, "8") == 0 || strcmp(buf, "OCTAL") == 0) {
		custom.c_ip_radix = _printf_ip_radix = 010;
		return 0;
		}
	if(strcmp(buf, "d") == 0 || strcmp(buf, "decimal") == 0 ||
	   strcmp(buf, "D") == 0 || strcmp(buf, "Decimal") == 0 ||
	   strcmp(buf, "10") == 0 || strcmp(buf, "DECIMAL") == 0) {
		custom.c_ip_radix = _printf_ip_radix = 10;
		return 0;
		}
	printf("illegal radix %s\n\033K", buf);
	return 0;
	}

p_telnet()
{
	printf((custom.c_1custom & NLSET) ? "Transmit on newline\n\033K" :
					"Transmit on each character\n\033K");
	printf("Telnet window is %u\n\033K", custom.c_telwin);
	printf("Telnet low window is %u\n\033K", custom.c_tellowwin);
	printf("\n\033K");
}

/* adjust custom bits */
togl_custom(bit)
int	bit;	/* which bit to toggle */
{
	custom.c_1custom ^= bit;
	return 0;
}

p_term()
{
	printf((custom.c_1custom & BSDEL) ? "<-- is Backspace\n\033K" :
					"<-- is Delete\n\033K");
	printf((custom.c_1custom & WRAP) ? "Wrap at end of line\n\033K" :
					"Discard at end of line\n\033K");
/* DDP Begin */
	printf("Chirp initial frequency divisor is %u\n\033K", custom.c_chirpf);
	printf("Chirp frequency divisor delta is %d\n\033K", custom.c_chirpd);
	printf("Number of chirp segments is %u\n\033K", custom.c_chirps);
	printf("Length of a chirp segment is %u\n\033K", custom.c_chirpl);
/* DDP End */
	printf("\n\033K");

}

_p_ethadd()
{
	switch(custom.c_seletaddr) {
	case HARDWARE:	printf("Uses hardware EtherNet address");
			break;
	case ETINTERNET: printf("Derives EtherNet address from internet address");
			break;
	case ETUSER:	printf("EtherNet address is ");
			out_ethaddr(&custom.c_myetaddr);
			break;
	default:	printf("Don't know EtherNet address");
	}
}

p_ethadd() {
	_p_ethadd();
	printf(".\n\033K");
}

set_eadd(value)
int	value;
{
    char	buf[80];
    int		i,count;
    struct etha	ead;

    custom.c_seletaddr = value;
    if (value == ETUSER) {
	for (i = 0; i < 6; i++)
		ead.e_ether[i] = 0;
	printf("\r\033K");
	get_line("Enter EtherNet address", buf, 80);
	count = 0;
	for (i = 0; i < 80; i++) {
		if (buf[i] == '\0') break;
		if ((buf[i] <= '7') && (buf[i] >= '0')) {
			ead.e_ether[count] = (ead.e_ether[count] << 3) + buf[i] - '0';
			continue;
		}
		else {
			count++;
			if (count > 6) break;
		}
	}
	for (i = 0; i < 6; i++)
		custom.c_myetaddr.e_ether[i] = ead.e_ether[i];
    }
    return(0);
}

p_tzone()
{
	int	i;

	printf("The time zone is %s. It is %d minutes from UT.\n\033K",
		custom.c_tmlabel, custom.c_tmoffset);
}

set_tzone()
{
	char	buf[80];
	int	i;

	printf("\r\033K");
	get_line("Enter string for time zone ", buf, 80);
	for (i = 0; i < 3; i++)
		custom.c_tmlabel[i] = buf[i];
	custom.c_tmlabel[4] = '\0';
	return(0);
}

set_twin() {
	char buf[80];

	printf("\r\033K");
	get_line("Enter the telnet window size ", buf, 80);
	custom.c_telwin = atoi(buf);
	return 0;
	}

set_tlwin() {
	char buf[80];

	printf("\r\033K");
	get_line("Enter the telnet low window point ", buf, 80);
	custom.c_tellowwin = atoi(buf);
	return 0;
	}

set_int() {
	char buf[80];

	printf("\r\033K");
	get_line("Enter the interrupt number for the net interface ",
							buf, 80);
	custom.c_intvec = atoi(buf);
	return 0;
	}

set_tx_dma() {
	char buf[80];

	printf("\r\033K");
	get_line("Enter the transmit dma channel (0 for no dma) for the net interface ",
							buf, 80);
	custom.c_tx_dma = atoi(buf);
	return 0;
	}

set_rcv_dma() {
	char buf[80];

	printf("\r\033K");
	get_line("Enter the receive dma channel (0 for no dma) for the net interface ",
							buf, 80);
	custom.c_rcv_dma = atoi(buf);
	return 0;
	}

set_base() {
	char buf[80];

	printf("\r\033K");
	get_line("Enter the base I/O address for the net interface in hex ",
							buf, 80);
	sscanf(buf, "%x", &custom.c_base);
	return 0;
	}

set_num_net() {
	char buf[80];

	printf("\r\033K");
	get_line("Enter the number of net interfaces on this machine ",
							buf, 80);
	custom.c_num_nets = atoi(buf);
	return 0;
	}

set_subnet() {
	char buf[80];

	printf("\r\033K");
	get_line("Enter the number of bits in the subnet field ",
							buf, 80);
	custom.c_subnet_bits = atoi(buf);

	fixup_subnet_mask();

	return 0;
	}

/* DDP Begin */
set_chirpf() {
	char buf[80];
	int count;

	printf("\r\033K");
	get_line("Enter the initial chirp count [800] ", buf, 80);
	count = atoi(buf);
	if (count)
		custom.c_chirpf = count;
	else
		custom.c_chirpf = 800;
	return 0;
	}

set_chirpd() {
	char buf[80];
	int count;

	printf("\r\033K");
	get_line("Enter the chirp count delta [-24]", buf, 80);
	custom.c_chirpd = atoi(buf);
	if (buf[0] == '\0')
		custom.c_chirpd = -24;
	return 0;
	}

set_chirpl() {
	char buf[80];
	int count;

	printf("\r\033K");
	get_line("Enter the length of a chirp segment [1000] ", buf, 80);
	count = atoi(buf);
	if (count)
		custom.c_chirpl = count;
	else
		custom.c_chirpl = 1000;
	return 0;
	}

set_chirps() {
	char buf[80];
	int count;

	printf("\r\033K");
	get_line("Enter the number of chirp segments [25] ", buf, 80);
	count = atoi(buf);
	if (count)
		custom.c_chirps = count;
	else
		custom.c_chirps = 25;
	return 0;
	}
/* DDP End */

#define	AMASK	0x80
#define	AADDR	0x00
#define	BMASK	0xC0
#define	BADDR	0x80
#define	CMASK	0xE0
#define	CADDR	0xC0

/* also called by my_addr()
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
	else {
		printf("invalid ip address %08X\n", custom.c_me);
		return 0;
		}

	/* generate the bit field for the subnet part. This code is
		incredibly dependent on byte ordering.
	 */
	while(subnet_bits--)
		smask = (smask >> 1) | 0x80000000;

	custom.c_net_mask = lswap(smask);
	}

set_domain() {
	char buf[80];

	printf("\r\033K");
	get_line("Enter the domain that this host is in (like MIT.EDU): ",
							buf, 80);
	strncpy(custom.c_domain, buf, 29);

	return 0;
	}

set_rvd_base() {
	char buf[10];
	printf("\r\033K");
	get_line("Enter the first drive used by RVD on this machine ",
							buf, 10);
	if(*buf >= 'a') custom.c_rvd_base = *buf-'a'+'A';
	else if(*buf >= 'A') custom.c_rvd_base = *buf;

	return 0;
	}

set_toffset()
{
	char	buf[80];

	printf("\r\033K");
	get_line("Enter offset in minutes of timezone from UT ", buf, 80);
	custom.c_tmoffset = atoi(buf);
	return(0);
}

struct menu_ent	menu_speed[] = {
	'\0',"Set serial line speed",p_speed,0,
	'1'," -1\t110",set_speed,baud_110,
	'2'," -2\t300",set_speed,baud_300,
	'3'," -3\t600",set_speed,baud_600,
	'4'," -4\t1200",set_speed,baud_1200,
	'5'," -5\t2400",set_speed,baud_2400,
	'6'," -6\t4800",set_speed,baud_4800,
	'7'," -7\t9600",set_speed,baud_9600,
	'8'," -8\t19200",set_speed,baud_19200,
	'\033'," -ESC\treturn",pop,-1,
	'\0',0,0,0,
};

struct menu_ent	menu_debug[] = {
	'\0',"Toggle Debug Options",print_debug,0,
	'd'," -d\tDump packets or headers",debug,DUMP,
	'b'," -b\tBughalt",debug,BUGHALT,
	'm'," -m\tInformational messages",debug,INFOMSG,
	'e'," -e\tNetwork errors",debug,NETERR,
	'p'," -p\tProtocol errors",debug,PROTERR,
	'o'," -o\tTimeout",debug,TMO,
	'n'," -n\tNet driver trace",debug,NETRACE,
	'i'," -i\tInternet protocol level trace",debug,IPTRACE,
	't'," -t\tTransport protocol level trace",debug,TPTRACE,
	'a'," -a\tApplication level trace",debug,APTRACE,
	'\033'," -ESC\treturn",pop,-1,
	'\0',0,0,0,
};

struct menu_ent	menu_name[] = {
	'\0',"Edit List of Name Servers",p_names,0,
	'a'," -a\tAdd a name server",add_name,0,
	'd'," -d\tDelete a name server",del_names,0,
	'\033'," -ESC\treturn",pop,-1,
	'\0',0,0,0,
};

struct menu_ent	menu_dm[] = {
	'\0',"Edit List of Domain Name Servers",p_dm_names,0,
	'a'," -a\tAdd a name server",add_dm_name,0,
	'd'," -d\tDelete a name server",del_dm_names,0,
	'\033'," -ESC\treturn",pop,-1,
	'\0',0,0,0,
};

struct menu_ent	menu_time [] = {
	'\0',"Set Time Servers",p_times,0,
	'a'," -a\tAdd a time server",add_time,0,
	'd'," -d\tDelete a time server",del_times,0,
	'\033'," -ESC\treturn",pop,-1,
	'\0',0,0,0,
};

struct menu_ent	menu_eic[] = {
	'\0',"Initialize EtherNet to Internet translation cache",p_eic,0,
	'1'," -1\tSet entry #1",set_eic,0,
	'2'," -2\tSet entry #2",set_eic,1,
	'3'," -3\tSet entry #3",set_eic,2,
	'\033'," -ESC\treturn",pop,-1,
	'\0',0,0,0,
};

struct menu_ent	menu_ethadd[] = {
	'\0',"EtherNet address",p_ethadd,0,
	'h'," -h\tUse hardware address",set_eadd,HARDWARE,
	'i'," -i\tDerive EtherNet address from internet address",set_eadd,ETINTERNET,
	'u'," -u\tSet EtherNet address",set_eadd,ETUSER,
	'\033'," -ESC\treturn",pop,-1,
	'\0',0,0,0,
};

struct menu_ent	menu_tzone[] = {
	'\0',"Time Zone Customization",p_tzone,0,
	'o'," -o\tSet time zone offset",set_toffset,0,
	'n'," -n\tSet time zone name",set_tzone,0,
	'\033'," -ESC\treturn",pop,-1,
	'\0',0,0,0,
};

struct menu_ent	menu_site[] = {
	'\0',"Site Customization",p_site,0,
	'a'," -a\tSet my internet address",my_addr,0,
	'd'," -d\tSet my domain",set_domain,0,
	'g'," -g\tSet default gateway's address",defgw_addr,0,
	'l'," -l\tSet log server's address",log_addr,0,
	'n'," -n\tSet old-style name servers",NULL,(int)menu_name,
	'p'," -p\tSet the print server's address",printer_addr,0,
	'r'," -r\tSet domain name servers",NULL,(int)menu_dm,
	's'," -s\tSet number of subnet bits",set_subnet,0,
	't'," -t\tSet time server's address",NULL,(int)menu_time,
	'v'," -v\tSet the first RVD drive",set_rvd_base,0,
	'z'," -z\tSet time zone",NULL,(int)menu_tzone,

	'\033'," -ESC\treturn",pop,-1,
	'\0',0,0,0,
};

struct menu_ent menu_hardware[] = {
	'\0', "Hardware Customization", p_hardware, 0,
	'b'," -b\tSet net interface CSR base I/O address", set_base, 0,
	'c'," -c\tSet EtherNet to Internet translation cache",NULL,(int)menu_eic,
	'e'," -e\tSet Ethernet address",NULL,(int)menu_ethadd,
	'n'," -n\tSet number of net interfaces on this machine", set_num_net, 0,
	'r'," -r\tSet net interface receive DMA channel", set_rcv_dma, 0,
	's'," -s\tSet serial line speed",NULL,(int)menu_speed,
	't'," -t\tSet net interface transmit DMA channel", set_tx_dma, 0,
	'v'," -v\tSet net interface interrupt vector",set_int,0,
	'\033'," -ESC\treturn", pop, -1,
	'\0', 0, 0, 0
	};

struct menu_ent	menu_telnet[] = {
	'\0',"Telnet Customization",p_telnet,0,
	'l'," -l\tSet telnet low window", set_tlwin, 0,
	'n'," -n\tTransmit on every character/Transmit on newline",togl_custom,NLSET,
	'w'," -w\tSet telnet window", set_twin, 0,
	'\033'," -ESC\treturn",pop,-1,
	'\0',0,0,0,
};

struct menu_ent	menu_em[] = {
	'\0',"Terminal Emulation Customization",p_term,0,
	'\177'," - <--\tSwap meaning of <-- key",togl_custom,BSDEL,
/* DDP Begin */
	'f'," -f\tSet chirp initial segment count", set_chirpf, 0,
	'd'," -d\tSet chirp segment count delta", set_chirpd, 0,
	'l'," -l\tSet length of a single chirp segment", set_chirpl, 0,
	's'," -s\tSet number of chirp segments", set_chirps, 0,
/* DDP End */
	'w'," -w\tDiscard at end of line/Wrap around at end of line",togl_custom,WRAP,
	'\033'," -ESC\treturn",pop,-1,
	'\0',0,0,0,
};

struct menu_ent	top[] = {
	'\0',"PC Network Customizer  - Ver 3.0",p_top,0,
	'd'," -d\tSet debug options",NULL,(int)menu_debug,
	'e'," -e\tExit and save",pop,42,
	'h'," -h\tDo hardware customizations",NULL,(int)menu_hardware,
	'm'," -m\tSet terminal options",NULL,(int)menu_em,
	'q'," -q\tQuit without saving",pop,-1,
	'r'," -r\tSet radix for displaying internet addresses",set_radix,0,
	's'," -s\tDo Site Customizations",NULL,(int)menu_site,
	't'," -t\tTelnet specific customization",NULL,(int)menu_telnet,
	'u'," -u\tSet user's name",set_uname,0,
	'\0',0,0,0,
};
