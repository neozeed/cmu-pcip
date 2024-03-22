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
#ifdef	PCNET
#include "../../srclib/pcnet/pcnet.h"

extern struct ncb ncb;
static struct ncb gwpcncb;

static
gwpcnet(p, len, host)
PACKET p;
int len;
in_name host;
{
	register char *q = (char *)udp_data(udp_head(in_head(p)));

	gwpcncb = ncb;
	memcpy(gwpcncb.ncb_callname, q, 16);
	q += 16;
	gwpcncb.ncb_buffer = (char far *)q;
	gwpcncb.ncb_length = len - 16;
	pc(&gwpcncb);
	udp_free(p);
}
#endif

UDPCONN logf;
static task *gwbtk;
#ifdef	PCSTAT
extern char pcact[];
extern int pcacti;
#endif
extern NET *et_net, nets[];
extern int Nnet;

static
gwbwake()
{
	tk_wake(gwbtk);
}

static
gwsink(p, len, host)
PACKET p;
int len;
in_name host;
{
	udp_free(p);
}

struct ripinfo {
	short rip_af;
	short rip_port;
	in_name rip_addr;
	char rip_r[8];
	long rip_metric;
};

static
gwbackground()
{
	timer *gwbtm;
	char *q;
	int cnt = 0, len;
	UDPCONN rip;
	PACKET p;
	register struct ripinfo *rp;
	register int i;

	if(!(gwbtm = tm_alloc())) {
		printf("Gw background timer setup failed\n");
		exit(1);
	}
	if(!(rip = udp_open(et_net->n_subnetbr, 520, 520, gwsink))) {
		printf("Rip open failed\n");
		exit(1);
	}
	if(!(p = udp_alloc(len = 4 + (Nnet - 1) * sizeof(struct ripinfo), 0))) {
		printf("Rip packet failed\n");
		exit(1);
	}
	q = (char *)udp_data(udp_head(in_head(p)));
	q[0] = 2;
	q[1] = 1;
	q[2] = q[3] = 0;
	rp = (struct ripinfo *)&q[4];
	for(i = 0; i < Nnet; i++) {
		if(&nets[i] == et_net)
			continue;
		rp->rip_af = 512;
		rp->rip_port = 0;
		rp->rip_addr = nets[i].n_subnetbr;
		rp++->rip_metric = 1L << 24;
	}
	while(1) {
		udp_write(rip, p, len);
		tm_set(30, gwbwake, NULL, gwbtm);
		tk_block();
		if(!(++cnt%4)) {
#ifdef	PCSTAT
			for(i = pcacti + 1; i < 256; i++)
				if(pcact[i]) {
					pcacti = i;
					pcact[i] = 0;
					goto found;
				}
			for(i = 0; i <= pcacti; i++)
				if(pcact[i]) {
					pcacti = i;
					pcact[i] = 0;
					goto found;
				}
			pcacti = 1;
		found:
			if(pcacti != 1 || !(cnt%60))
				pcstat(pcacti);
#endif
		}
	}
}

char sbuf[200];
syslog(s)
char *s;
{
	register int len = strlen(s) + 3;
	register char *q;
	PACKET p;

	if(!logf)
		return;
	if(!(p = udp_alloc(len + 1, 0))) {
		printf("Log packet failed\n");
		return;
	}
	q = (char *)udp_data(udp_head(in_head(p)));
	strcpy(q, "<3>");
	strcat(q, s);
	udp_write(logf, p, len);
	udp_free(p);
}

in_name ping_host;		/* for the command interpreter */

main(argc, argv)
char **argv;
{
	in_name host;
	unsigned ntimes = 1;
	char *arg;
	char test=0;
	char serve=0;
	char key;
	long nsuccess=0L;
	long i;
	unsigned res = PGWAITING;
	unsigned length = 256;
	int c;

	if(argc < 2 || argc > 5)
		serve = 1;
	else {
	arg = argv[1];
	if(arg[0] == '-') {
		arg++;
		while(*arg != '\0') {
			switch(*arg) {
			case 't':
				test = 1;
				break;
			case 'n':
				ntimes = atoi(argv[2]);
				argv++;
				argc--;
				break;
			case 'l':
				length = atoi(argv[2]);
				argv++;
				argc--;
				break;
			case 's':
				serve = 1;
				break;
			default:
				printf("PING: Illegal option %c.\n", *arg);
				exit(1);
			}
			arg++;
		}
		argv++;
		argc--;
	}
	}

	Netinit(800);
	in_init();
	UdpInit();
	IcmpInit();
	GgpInit();
	nm_init();
	if(et_net->n_custom->c_log &&
		!(logf = udp_open(et_net->n_custom->c_log, 514, 514, gwsink))) {
		printf("Log open failed\n");
		exit(1);
	}
	if(!(gwbtk = tk_fork(tk_cur, gwbackground, 1500, "GwBackground", 0))) {
		printf("Gw background process setup failed\n");
		exit(1);
	}
#ifdef	PCNET
	if(!udp_open(0L, 0, 20000, gwpcnet, 0)) {
		printf("Gwpcnet open failed\n");
		exit(1);
	}
#endif

	if(serve) {
		printf("Gateway ready (? for help)\n");
		while(1) {
			tk_yield();
			if(kbhit())
				ping_cmd(getch());
		}
	}

	host = resolve_name(argv[1]);
	if(host == 0L) {
		printf("Host %s is unknown.\n", argv[1]);
		exit(1);
	}

	if(host == 1L) {
		printf("Name servers not responding.\n");
		exit(1);
	}

	ping_host = host;

	if(test) {
		printf("Pinging host %s (%A) repeatedly\n", argv[1], host);
		printf("for net statistics, type 'n',\n");
		printf("to exit, type 'q'.\n");
		for(i=1;1;i++) {
			res = IcEchoRequest(host, length);

			if(res == PGNOSND)
		 printf("Couldn't send (net address unknown?) on try %U\n",i);
			else if(res == PGTMO)
			 printf("ping: timed out on try %U            \n",i);
			else if(res == PGBADDATA)
			 printf("ping: reply with bad data on try %U  \n",i);
			else if(res == PGSUCCESS) nsuccess++;
			else printf("ping: bad return value (try %U) %u\n",
							i, res);

			printf("# of tries = %U, successes = %U\r",
								i, nsuccess);

			if(kbhit())
				ping_cmd(getch());
		}
	}

	for(i=0; i<ntimes; i++) {
		if(kbhit())
			ping_cmd(getch());

		res = IcEchoRequest(host, length);
		switch(res) {
		case PGSUCCESS:
			nsuccess++;
			printf("Host %A responding\n", host);
			break;
		case PGNOSND:
		printf("Couldn't send (net address unknown?) on try %U\n",
								i);
			break;
		case PGTMO:
			printf("Host %A timed out\n", host);
			break;
		case PGBADDATA:
			printf("ping: reply with bad data\n");
			break;
		default:
			printf("ping: bad return value %u\n", res);
		}
	}

	if(ntimes != 1)
		printf("\nPinged host %A %u times and got %U responses.\n",
						host, ntimes, nsuccess);

	net_stats(stdout);
}	

#ifdef	PCSTAT
static struct ncb stncb;
static struct astatus astatus;
int dopcstat = 1;

pcstat(n)
{
	register unsigned char *p;
	register int i;
	extern NET *pc_net;

	if(!dopcstat)
		return;
	stncb.ncb_command = NCB_ADAPTER_STATUS;
	stncb.ncb_buffer = (char far *)&astatus;
	stncb.ncb_length = sizeof(astatus);
	strcpy(stncb.ncb_callname, "PCIPPCIPPCIP");
	p = &stncb.ncb_callname[12];
	*(in_name *)p = pc_net->ip_addr;
	p[3] = n;
	pc(&stncb);
	if(stncb.ncb_retcode)
		return;
	printf("\033[;H\033[2J");
	p = astatus.as_id;
	printf("Address: %x:%x:%x:%x:%x:%x\n", p[0], p[1], p[2],p[3],p[4],p[5]);
	printf("Jumpers: %x, POST result: %x, Version: %d.%d\n",
		astatus.as_jumpers, astatus.as_post,
		astatus.as_major, astatus.as_minor);
	printf("Time: %u, CRC: %u, Alignment: %u, Collision: %u, Abort: %u\n",
		astatus.as_interval, astatus.as_crcerr, astatus.as_algerr,
		astatus.as_colerr, astatus.as_abterr);
	printf("Transmitted: %ld, Received: %ld, Retran: %u, Exhausted: %u\n",
		astatus.as_tcount, astatus.as_rcount,
		astatus.as_retran, astatus.as_xresrc);
	printf("NCBS - Free: %u, Max configured: %u, Max possible: %u\n",
		astatus.as_ncbfree, astatus.as_ncbmax, astatus.as_ncbx);
	printf("Sessions - Pending: %u, MaxPending: %u, Max: %u\n",
		astatus.as_sespend, astatus.as_msp, astatus.as_sesmax);
	printf("Max Packet size: %u\n", astatus.as_bufsize);
	printf("Local names: %u\n", astatus.as_names);
	for(i = 0; i < astatus.as_names; i++) {
		char buf[17];
		strncpy(buf, astatus.as_name[i].nm_name, 16);
		buf[16] = 0;
		printf("%d(%x) - %s", astatus.as_name[i].nm_num,
			astatus.as_name[i].nm_status, buf);
		if(!strncmp(astatus.as_name[i].nm_name, "PCIPPCIPPCIP", 12)) {
			p = &astatus.as_name[i].nm_name[12];
			printf(" [ip: %d.%d.%d.%d]", p[0], p[1], p[2], p[3]);
			goto done;
		}
		for(p = astatus.as_name[i].nm_name;
			p < &astatus.as_name[i].nm_name[16]; p++)
			if((*p)&0x80 || *p < ' ' || *p == 0x7f) {
				printf(" [hex: ");
				for(p = astatus.as_name[i].nm_name;
					p < &astatus.as_name[i].nm_name[16];p++)
					printf("%02x", *p);
				printf("]");
				break;
			}
	done:
		printf("\n");
	}
}
#endif
