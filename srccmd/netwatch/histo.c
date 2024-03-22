/*  Copyright 1986 by Carnegie Mellon  */
/*  See permission and disclaimer notice in file "cmu-note.h"  */
#include	<cmu-note.h>
#include <stdio.h>
#include <q.h>
#include <em.h>
#include <attrib.h>
#include <match.h>
#include "watch.h"

/* histogram routines */

fancy_histo() {
	unsigned long biggest = 0;
	char buf[10];
	int i;
	int y;

	/* clear the screen */
	clear_lines(0, 25);

	/* find biggest value */
	for(i=0; i<HIST_NUM_INCS; i++)
		if(biggest < hist_counts[i])
			biggest = hist_counts[i];

	if(biggest < hist_too_big)
		biggest = hist_too_big;

	/* label the Y axis */
	for(i = 0; i < 24; i++) {
		sprintf(buf, "%8D ", biggest*(i+1)/24);
		write_string(buf, 23-i, 0, NORMAL);
		}

	/* now draw a line for each value, starting at line 24 going up
		to line 0 for the maximum.
	*/
	for(i=0; i<HIST_NUM_INCS; i++)
		for(y = 0; y < (hist_counts[i]*24)/biggest; y++)
			nwrite_char(' '|(INVERT<<8), (23-y)*80+i+9);

	clr25();
	pr25(0, "---paused [type any character to continue] ---");
	while(h19key() == NONE) ;
	clr25();
	clear_lines(0, 25);
	inv25();
	scroll_end();
	}
