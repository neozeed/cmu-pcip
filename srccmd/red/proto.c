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
#include <timer.h>

#define MAGIC 123

static task *mtask;
static int state;

int us_open(), us_cls(), us_tmo(), us_yld();
int us_data(), no_op();

tcpopen(host, port)
char *host;
{
	in_name fhost;
	static int first = 1;

	state = 0;
	tcp_init(512, us_open, us_data, us_yld, us_cls, us_tmo, no_op, no_op);

	if((fhost = resolve_name(host)) == 0L) {
		fprintf(stderr, "Host %s unknown.\n", host);
		return(-1);
	}

	if(fhost == 1L) {
		fprintf(stderr, "Name servers not responding.\n");
		return(-1);
	}
	mtask = tk_cur;
	tcp_open(&fhost, port, 0, custom.c_telwin, custom.c_tellowwin);
	while(!state)
		tk_block();
	if(first) {
		int tcpclose();
		first = 0;
		onexit(tcpclose);
	}
	return(MAGIC);
}

tcpclose(fd)
{
	if(state) {
		tcp_close();
		tk_yield();
	}
}

tcpwrite(fd, p, cnt)
{
	return(twrite(p, cnt));
}

#define BSZ (8*1024)
char buf[BSZ], *head = buf, *tail = buf;
int bufcnt;

tcpread(fd, p, cnt)
register char *p;
{
	register int c;

	while(!bufcnt) {
		if(!state)
			return(-1);
		tk_block();
	}
	if(cnt > bufcnt)
		cnt = bufcnt;
	c = cnt;
	bufcnt -= c;
	while(c--) {
		*p++ = *tail++;
		if(tail >= &buf[BSZ])
			tail = buf;
	}
	return(cnt);
}

us_data(p, cnt, urg)
register char *p;
register int cnt;
{
	while(cnt--) {
		while(bufcnt >= BSZ) {
			tk_wake(mtask);
			tk_yield();
		}
		*head++ = *p++;
		if(head >= &buf[BSZ])
			head = buf;
		bufcnt++;
	}
	tk_wake(mtask);
}

us_tmo()
{
	fprintf(stderr, "Host not responding\n");
	tcp_close();
	tcp_reset();
	tk_yield();
	tk_wake(mtask);
}

no_op()
{
}

us_open()
{
	state = 1;
	tk_wake(mtask);
}

us_cls()
{
	state = 0;
	tk_wake(mtask);
}

us_yld()
{
	return(1);
}

ngetche()
{
	while(!kbhit())
		tk_yield();
	return(getche());
}

char *
ngets(buf)
char *buf;
{
	register char *p = buf;
	register int c;

	while((c = ngetche()) != '\n' && c != '\r')
		*p++ = c;
	*p = 0;
	write(1, "\n", 1);
	return(buf);
}
