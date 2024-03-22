/*  Copyright 1986 by Carnegie Mellon  */
/*  See permission and disclaimer notice in file "cmu-note.h"  */
#include	<cmu-note.h>
/*
 * getopt - get option letter from argv
 */

#include <stdio.h>

char	*optarg;	/* Global argument pointer. */
int	optind = 0;	/* Global argv index. */

static char	*scan = NULL;	/* Private scan pointer. */

#ifndef MSC			/* DDP */
extern char	*index();
#else				/* DDP */
extern char	*strchr();	/* DDP */
#endif				/* DDP */

int
getopt(argc, argv, optstring)
int argc;
char *argv[];
char *optstring;
{
	register char c;
	register char *place;

	optarg = NULL;

	if (scan == NULL || *scan == '\0') {
		if (optind == 0)
			optind++;
	
		if (optind >= argc || argv[optind][0] != '-' || argv[optind][1] == '\0')
			return(EOF);
		if (strcmp(argv[optind], "--")==0) {
			optind++;
			return(EOF);
		}
	
		scan = argv[optind]+1;
		optind++;
	}

	c = *scan++;
#ifndef MSC				/* DDP */
	place = index(optstring, c);
#else					/* DDP */
	place = strchr(optstring, c);	/* DDP */
#endif					/* DDP */

	if (place == NULL || c == ':') {
		fprintf(stderr, "%s: unknown option -%c\n", argv[0], c);
		return('?');
	}

	place++;
	if (*place == ':') {
		if (*scan != '\0') {
			optarg = scan;
			scan = NULL;
		} else {
			optarg = argv[optind];
			optind++;
		}
	}

	return(c);
}
