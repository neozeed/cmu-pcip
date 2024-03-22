/*  Copyright 1986 by Carnegie Mellon  */
/*  See permission and disclaimer notice in file "cmu-note.h"  */
#include	<cmu-note.h>
/*  Copyright 1984 by the Massachusetts Institute of Technology  */
/*  See permission and disclaimer notice in file "notice.h"  */
#include	<notice.h>

/*
24-Mar-86 Drew D. Perkins (ddp) at Carnegie-Mellon Univsersity,
	I just made a fantastic discovery.  Since we're now using bios
	to deal with the screen, and since scr_rest() is now a null
	routine, we no longer rst_screen to do anything but move the
	cursor.  That means we don't need the huge array at all!
 */

/* This is a routine that restores the state of the screen (i.e. it
 * sets the start register in the 6845 back to zero) but it leaves the
 * screen looking the same.
 */

extern int pos, x_pos, y_pos;

scr_close()
{
	y_pos = pos/80;
	x_pos = pos-y_pos*80;

	rst_screen();
}

rst_screen()
{
#ifndef MSC
  static int screen[25][80];	/* DDP - place to save the old screen */
  int	count;

  /* save the screen */
  for (count = 0; count <= 24; count++)
	read_line(&screen[count][0],count);

  scr_rest();

  /* restore the screen */

  for (count = 0; count <= 24; count++)
	write_line(&screen[count][0],count);
#endif

  rset_cursor();
}
