/* Copyright 1986 by Carnegie Mellon */
/*  Copyright 1985 by the Massachusetts Institute of Technology  */

/* to get the time from MS-DOS, call get_dosl(GETTIME) and store the
	result in a long variable. Then have a pointer to a struct __time
	and assign the address of the long to it. Then you can access
	fields of the time using the structure.
*/

struct __time {
	char	_t_cents;	/* 1/100ths of seconds */
	char	_t_sec;		/* seconds */
	char	_t_min;		/* minutes */
	char	_t_hrs;		/* hours */
	};

#define	GETTIME	0x2c

extern long get_dosl();
