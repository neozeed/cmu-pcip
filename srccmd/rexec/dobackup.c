#include <sys/types.h>
#include <time.h>
#include <stdio.h>

#define DAY (24L*60L*60L)
#define TARDATES "/tardates"

char pass[11], days[10], *getenv();
time_t tardates[10], now, f;

main(argc, argv)
char **argv;
{
	char *host = getenv("TARHOST"), *user = getenv("USER");
	struct tm *tm;
	register int i, c;
	int level;

	if(argc > 2) {
		fprintf(stderr, "Usage: %s [level]\n", argv[0]);
		exit(1);
	}
	if(!user) {
		fprintf(stderr, "You must set your USER variable.\n");
		exit(1);
	}
	if((i = open(TARDATES, 0x8000)) >= 0) {
		read(i, tardates, sizeof(tardates));
		close(i);
	}
	if(!host)
		host = "128.103.1.36";
	time(&now);
	if(argc > 1) {
		switch(argv[1][0]) {

			case 'f':
			case 'F':
				level = 0;
				break;

			case 'y':
			case 'Y':
				level = 5;
				break;

			case 's':
			case 'S':
				level = 6;
				break;

			case 'm':
			case 'M':
				level = 7;
				break;

			case 'w':
			case 'W':
				level = 8;
				break;

			case 'd':
			case 'D':
				level = 9;
				break;

			case '0':
			case '1':
			case '2':
			case '3':
			case '4':
			case '5':
			case '6':
			case '7':
			case '8':
			case '9':
				level = argv[1][0] - '0';
				break;

			default:
				fprintf(stderr, "Level must be 0-9 or:\n");
				fprintf(stderr, "F(ull) (all files)\n");
				fprintf(stderr, "Y(early)\n");
				fprintf(stderr, "S(emesterly)\n");
				fprintf(stderr, "M(onthly)\n");
				fprintf(stderr, "W(eekly)\n");
				fprintf(stderr, "D(aily)\n");
				exit(1);
		}
	}
	else {
		tm = localtime(&now);
		if(tm->tm_mday == 1)
			level = 7;
		else if(tm->tm_wday == 1)
			level = 8;
		else
			level = 9;
	}
	printf("Password:");
	fflush(stdout);
	*pass = '"';
	i = 1;
	while((c = getch()) != '\r' && c != '\n' && c != -1)
		if(i < sizeof(pass) - 2)
			pass[i++] = c;
	pass[i++] = '"';
	pass[i] = 0;
	printf("\n");
	switch(level) {

		case 0:
			printf("Full backup\n");
			break;

		case 5:
			printf("Yearly backup\n");
			break;

		case 6:
			printf("Semesterly backup\n");
			break;

		case 7:
			printf("Monthly backup\n");
			break;

		case 8:
			printf("Weekly backup\n");
			break;

		case 9:
			printf("Daily backup\n");
			break;
	}
	if(!(f = tardates[(level && level != 5) ? level - 1 : 0]))
		f = now - 3 * DAY;
	tardates[level] = now;
	sprintf(days, "%ld", (now - f + DAY - 1) / DAY);
	printf("The date of this backup will be %s", ctime(&now));
	if(level) {
		printf("Last backup at this level was %s", ctime(&tardates[level]));
		printf("Files modified within the last %s days will be saved\n",
			days);
	}
	chdir("/");
	if(spawnlp(0, "rexec", "rexec", host, "-l", user, "-p", pass,
		"/usr/local/bin/pctarset", user, 0)) {
		fprintf(stderr, "Setup failed\n");
		exit(1);
	}
	if(level)
	spawnlp(0, "tar", "tar", "Xcvabf", days, "30", "X.TAR", "*.*", 0);
	else
	spawnlp(0, "tar", "tar", "Xcvbf", "30", "X.TAR", "*.*", 0);
	if(spawnlp(0, "rexec", "rexec", host, "-l", user, "-p", pass,
		"/bin/rm -f $HOME/pcbackup.tar;",
		"/usr/local/bin/pctarget", user, "> $HOME/pcbackup.tar", 0)) {
		fprintf(stderr, "Completion failed\n");
		exit(1);
	}
	if((i = creat(TARDATES, 0644)) >= 0) {
		setmode(i, 0x8000);
		write(i, tardates, sizeof(tardates));
		close(i);
	}
	exit(0);
}
