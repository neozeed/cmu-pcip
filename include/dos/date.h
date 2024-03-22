/* Copyright 1986 by Carnegie Mellon */
/*  Copyright 1985 by the Massachusetts Institute of Technology  */

/* to get the date from MS-DOS, call get_dosl(GETDATE) and store the
	result in a long variable. Then have a pointer to a struct __date
	and assign the address of the long to it. Then you can access
	fields of the date using the structure.
*/

struct __date {
	char	 _d_day;		/* day of month */
	char	 _d_month;	/* month of year - 1 based */
	unsigned _d_year;	/* year in binary */
	};

#define	GETDATE	0x2a

extern long get_dosl();
