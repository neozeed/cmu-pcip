#include <dos.h>

pcip_open(pathname, oflag)
char *pathname;
char oflag;
{
	union REGS inregs, outregs;

	inregs.h.ah = 0x3d;		/* Open a file */
	inregs.h.al = oflag & 3;	/* Open in this mode */
	inregs.x.dx = (unsigned int) pathname;
					/* DS(already set):DX -> file name */
	intdos(&inregs, &outregs);	/* Make dos call */
	return outregs.x.cflag ? -1 : outregs.x.ax;
					/* Error if carry set */
}
