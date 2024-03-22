#include <stdio.h>

char buf[100];

main(argc, argv)
char **argv;
{
	char *getenv(), *user = getenv("USER"), *host = getenv("TARHOST");

	if(!user) {
		fprintf(stderr, "You must set your USER variable.\n");
		exit(1);
	}
	if(!host)
		host = "128.103.1.36";
	sprintf(buf, "/tmp/%s/%s", user, argv[2]);
	exit(spawnlp(0, "tftp", "tftp", "-p", argv[1], host, buf, "image", 0));
}
