/*  Copyright 1986 by Carnegie Mellon  */
/*  See permission and disclaimer notice in file "cmu-note.h"  */
#include	<cmu-note.h>
/*  Copyright 1984 by the Massachusetts Institute of Technology  */
/*  See permission and disclaimer notice in file "notice.h"  */
#include	<notice.h>


#define	TRUE	1
#define	FALSE	0

#define baud_19200	0x0006	/* divisors for the different baud rates */
#define baud_9600	0x000c
#define	baud_4800	0x0018
#define	baud_2400	0x0030
#define	baud_1200	0x0060
#define	baud_600	0x00c0
#define	baud_300	0x0180
#define	baud_110	0x0417

int	baud_default = baud_9600;	/* the default baud rate for port 1 */
int	baud2_default = baud_9600;	/* the default baud rate for port 2 */
int	port = 1;		/* which serial port is currently in use */
int	hdx = FALSE;		/* half duplex flag */

/* Variables of the emulator that need to be set by the menu */
extern	char	NORM_VIDEO;
extern	char	REV_VIDEO;
extern	char	attrib;
extern	int	wrap_around;
extern	int	x_pos,y_pos;
extern	int	ba_bs;

char	*baud_string();

#include	<h19.h>

main() /* Terminal Emulator */
{
 char	ch;

  if(port == 1)
	  init_aux(baud_default);	/* initialize aux ports */
  else    init2_aux(baud2_default);

  bell_init();			/* initialize the bell routines */

  scr_init();			/* initialize screen */
  set_cursor(pos = 0);
  x_pos = y_pos = 0;
  clear_lines(0,25);

  while (TRUE) {
	switch (ch = get_kbd()) {
	case NONE:
		break;
	case C_BREAK:
		done();
		exit();
		break;
	case F9:		/* break */
		if (port == 1)
			make_break();
		if (port == 2)
			make2_break();
		break;
	case F10:
		menu();
		break;
	default:
		send_char((char)ch);
		break;
	}
	if (port == 1) {		/* DDP */
		if (crcnt()) {
			char c;
			c = cread();
			em(c);
			}
	}
	else if (port == 2) {		/* DDP */
		if (crcnt2())
			em(cread2());
	}				/* DDP */
  }
}


/* send_char(data) - causes the data character to be sent to the appropriate
 *			places according to the state of the flags
 */

send_char(data)
char	data;		/* data to be sent */
{
  if (port == 1) {
	cwrit(data);
	if (hdx)
		local(data);
  }
  else if (port == 2) {
	cwrit2(data);
	if (hdx)
		local2(data);
  }
}

/* menu() - allows entering of various terminal modes */

menu()
{
  static int	screen[25][80];	/* DDP place to save the old screen */
  int	count;
  int	baud;
  unsigned old_pos;
  int ox, oy;
  char	data = 1;

  /* save the screen */
  for (count = 0; count <= 24; count++)
	read_line(&screen[count][0],count);

  old_pos = pos;
  ox = x_pos; oy = y_pos;

  scr_rest();		/* reset the beginning of the screen to zero */
  while(data) {
	clear_lines(0,25);
	write_string("Commands:",0,0,NORM_VIDEO);
	write_string("f - set full duplex",2,5,
					((!hdx) ? REV_VIDEO : NORM_VIDEO));
	write_string("h - set half duplex",3,5,
					((hdx) ? REV_VIDEO : NORM_VIDEO));
	write_string("b - black background",4,5,0x70);
	write_string("w - white background",5,5,0x07);
	write_string("r - wrap around mode",6,5,
				((wrap_around) ? REV_VIDEO : NORM_VIDEO));
	write_string("s - no wrap around",7,5,
				((!wrap_around) ? REV_VIDEO : NORM_VIDEO));
	write_string("1 - port #1",8,5,((port == 1) ? REV_VIDEO : NORM_VIDEO));
	write_string("2 - port #2",9,5,((port == 2) ? REV_VIDEO : NORM_VIDEO));
	write_string("d - change default baud rate on current port",10,5,
								NORM_VIDEO);
	write_string(" - swap meaning of backarrow key", 11, 5, NORM_VIDEO);
	write_string("backspace", 11, 39, (ba_bs ? REV_VIDEO : NORM_VIDEO));
	write_string("delete", 11, 48, (ba_bs ? NORM_VIDEO : REV_VIDEO));
	write_string("c - exit the emulator turning off DTR", 12, 5, NORM_VIDEO);
	write_string("q - quit the emulator leaving DTR alone", 13, 5, NORM_VIDEO);
	write_string("F10 - return to terminal",14,5,NORM_VIDEO);

	write_string("Baud rates:",3,50,NORM_VIDEO);
	write_string("Port 1 =",5,55,NORM_VIDEO);
	write_string(baud_string(baud_default),5,65,NORM_VIDEO);
	write_string("Port 2 =",6,55,NORM_VIDEO);
	write_string(baud_string(baud2_default),6,65,NORM_VIDEO);

	pos = 11;
	set_cursor();
	x_pos = 11; y_pos = 0;

	while(kbd_stat() == 0)
		;

	data = (char)kbd_in();
	switch(data) {
	case 'f':
		hdx = FALSE;
		break;
	case 'h':
		hdx = TRUE;
		break;
	case 'b':
		NORM_VIDEO = 0x07;
		REV_VIDEO = 0x70;
		attrib = NORM_VIDEO;
		break;
	case 'w':
		NORM_VIDEO = 0x70;
		REV_VIDEO = 0x07;
		attrib = NORM_VIDEO;	
		break;
	case 'r':
		wrap_around = TRUE;
		break;
	case 's':
		wrap_around = FALSE;
		break;
	case '1':
		if(port == 2) {
			close2_aux();
			init_aux(baud_default);
			}
		port = 1;
		break;
	case '2':
		if(port == 1) {
			close_aux();
			init2_aux(baud2_default);
			}
		port = 2;
		break;
	case 'd':
		baud = ((port == 1) ? baud_default : baud2_default);
		clear_lines(0,25);
		write_string(((port == 1) ? "Baud for Port 1" :
					"Baud for Port 2"),0,0,NORM_VIDEO);
		write_string("1 - 110 baud",2,5,((baud == baud_110) ?
						REV_VIDEO : NORM_VIDEO));
		write_string("2 - 300 baud",3,5,((baud == baud_300) ?
						REV_VIDEO : NORM_VIDEO));
		write_string("3 - 600 baud",4,5,((baud == baud_600) ?
						REV_VIDEO : NORM_VIDEO));
		write_string("4 - 1200 baud",5,5,((baud == baud_1200) ?
						REV_VIDEO : NORM_VIDEO));
		write_string("5 - 2400 baud",6,5,((baud == baud_2400) ?
						REV_VIDEO : NORM_VIDEO));
		write_string("6 - 4800 baud",7,5,((baud == baud_4800) ?
						REV_VIDEO : NORM_VIDEO));
		write_string("7 - 9600 baud",8,5,((baud == baud_9600) ?
						REV_VIDEO : NORM_VIDEO));
		write_string("8 - 19200 baud",9,5,((baud == baud_19200) ?
						REV_VIDEO : NORM_VIDEO));
		while(kbd_stat() == 0)
			;
	
		data = (char)kbd_in();
		if (data == '1')
			baud = baud_110;
		else if (data == '2')
			baud = baud_300;
		else if (data == '3')
			baud = baud_600;
		else if (data == '4')
			baud = baud_1200;
		else if (data == '5')
			baud = baud_2400;
		else if (data == '6')
			baud = baud_4800;
		else if (data == '7')
			baud = baud_9600;
		else if (data == '8')
			baud = baud_19200;

		if (port == 1) {
			baud_default = baud;
		  	close_aux();
			init_aux(baud_default);
		}
		if (port == 2) {
			baud2_default = baud;
			close2_aux();
			init2_aux(baud2_default);
		}
		break;
	case 'c':
		done();
		if (port == 1)
			dtr_off();
		if (port == 2)
			dtr2_off();
		exit();
		break;
	case 'q':
		done();
		exit();
		break;
	case B_SPACE:
	case DELETE:
		if (ba_bs == TRUE)
			ba_bs = FALSE;
		else
			ba_bs = TRUE;
		break;
	}
  }
  /* restore the screen */
  for (count = 0; count <= 24; count++)
	write_line(&screen[count][0],count);

  pos = old_pos;
  x_pos = ox; y_pos = oy;
  set_cursor();
}

/* baud_string(baud) - returns a string that tells the baud rate
 *	the argument is the divisor for some baud rate
 */

char *baud_string(baud)
int	baud;
{
  if (baud == baud_110)
	return("110 baud");
  else if (baud == baud_300)
	return("300 baud");
  else if (baud == baud_600)
	return("600 baud");
  else if (baud == baud_1200)
	return("1200 baud");
  else if (baud == baud_2400)
	return("2400 baud");
  else if (baud == baud_4800)
	return("4800 baud");
  else if (baud == baud_9600)
	return("9600 baud");
  else if (baud == baud_19200)
	return("19200 baud");
  else
	return("unknown baud rate");
}

done()
{
  scr_rest();
  pos = 0;
  set_cursor();
  clear_lines(0,25);
  bell_off();
  if(port == 1)
	close_aux();
  else if(port == 2)
	close2_aux();
}

/* define the following routines to prevent the standard i/o
library from being linked in.
*/

dos_ini() { }

_cleanup() { }

prints() { }

_dos_cl() { }

_wbyte() { } 
