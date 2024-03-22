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

task *ftask;
event open_done;

int us_open(), us_cls(), us_tmo(), us_yld();
int show(), no_op();
int exitval;
char cmd[256];

main(argc, argv)
char **argv;
{
	char *getenv(), *user = getenv("USER");
	char *host, *pass = "";
	in_name fhost;

	if(argc < 3) {
		fprintf(stderr,
		"Usage: %s host [-l user] [-p password] command\n", argv[0]);
		exit(1);
	}
	host = argv[1];
	argv += 2;
	argc -= 2;
loop:
	if(argc > 1 && !strcmp(argv[0], "-l")) {
		user = argv[1];
		argv += 2;
		argc -= 2;
		goto loop;
	}
	if(argc > 1 && !strcmp(argv[0], "-p")) {
		pass = argv[1];
		argv += 2;
		argc -= 2;
		goto loop;
	}
	if(!user) {
		fprintf(stderr, "You must set your USER variable.\n");
		exit(1);
	}
	*cmd = 0;
	while(argc > 0) {
		strcat(cmd, *argv);
		argv++;
		if(--argc)
			strcat(cmd, " ");
	}

	tcp_init(512, us_open, show, us_yld, us_cls, us_tmo, no_op, no_op);

	if((fhost = resolve_name(host)) == 0L) {
		fprintf(stderr, "Host %s unknown.\n", host);
		exit(1);
	}

	if(fhost == 1L) {
		fprintf(stderr, "Name servers not responding.\n");
		exit(1);
	}
	ftask = tk_cur;
	tcp_open(&fhost, 512, 0, custom.c_telwin, custom.c_tellowwin);
	while(!open_done)
		tk_block();
	twrite("0", 2);
	twrite(user, strlen(user) + 1);
	twrite(pass, strlen(pass) + 1);
	twrite(cmd, strlen(cmd) + 1);
	while(1) {
		if(kbhit()) {
			char c = getche();
			twrite(&c, 1);
		}
		tk_yield();
	}
}

show(buf, len, urg)
register char *buf;
{
	register int i;
    
	for(i = 0; i < len; i++) {
		if(buf[i] == '\n')
			putchar('\r');
		putchar(buf[i]);
	}

}

us_tmo()
{
	fprintf(stderr, "Host not responding\n");
	quit();
}

no_op()
{
}

quit() 
{
	tcp_close();
	exitval = 1;
	tcp_reset();
	tk_yield();
	exit(1);
}

us_open()
{
	tk_setef(ftask, &open_done);
}

us_cls()
{
	exit(exitval);
}

us_yld()
{
	return(1);
}
