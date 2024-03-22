/*  Copyright 1986 by Carnegie Mellon  */
/*  See permission and disclaimer notice in file "cmu-note.h"  */
#include	<cmu-note.h>
/*  Copyright 1984, 1985 by the Massachusetts Institute of Technology  */
/*  See permission and disclaimer notice in file "notice.h"  */
#include	<notice.h>

#include <attrib.h>

/* 25th line (and generic) special printing routines.
*/

unsigned clear25 = 0;		/* when to clear line 25 */
unsigned att25 = INVERT;	/* 25th line attribute byte */
char write_attrib = NORMAL;

/* write a string st with attribute attrib at (x,y) on the screen. If
	it would overflow the current line, change the last character
	that would fit on the line to an exclamation mark and truncate
	the string there.
*/

wr_string(st, y, x, attrib)
	char *st;
	int y,x;
	unsigned attrib; {

	if(strlen(st) + x > 80) {
		st[79-x] = '!';
		st[80-x] = '\0';
		}

	write_string(st, y, x, write_attrib);
	}

/* Print an error message on the 25th line, setting a timer which will cause
	it to be wiped out in 3 seconds. */

prerr25(s)
	char *s; {

	write_string(s, 24, 0, att25|BLINK_ON);
	clear25 = 3;
	}

prbl25(s)
	char *s; {

	write_string(s, 24, 0, att25|BLINK_ON);
	}


prat25(s, x)
	char *s;
	int x; {

	write_string(s, 24, x, NORMAL);
	}

inv25() {
	char line[160];
	int i;

	att25 = INVERT;
	read_line(line, 24);
	for(i=1; i<160; i=i+2) line[i] = INVERT;
	write_line(line, 24);
	}

norm25() {
	char line[160];
	int i;

	att25 = NORMAL;
	read_line(line, 24);
	for(i=1; i<160; i=i+2) line[i] = NORMAL;
	write_line(line, 24);
	}


pr25(x, s)
	int x;
	char *s; {

	write_string(s, 24, x, att25);
	}
