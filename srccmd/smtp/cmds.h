/*  Copyright 1984 by the Massachusetts Institute of Technology  */
/*  See permission and disclaimer notice in file "notice.h"  */
#include	<notice.h>

/* EMACS_MODES: c !fill */

/* cmds.h 
 *
 * smtp command strings and associated codes.  Note that the command code
 * MUST be equal to the index of the command in the table.
 */

#define	NONE		0		/* no such command */
#define	HELO		1
#define	MAIL		2
#define	RCPT		3
#define	DATA		4
#define	QUIT		5
#define	RSET		6
#define	NOOP		7

struct	cmdtab	{
	char	*c_name;		/* command name */
	int	c_len;			/* command length */
} cmdtab[] = {
	{ "", 0, },
	{ "HELO", 4, },
	{ "MAIL FROM:", 10 },
	{ "RCPT TO:", 8, },
	{ "DATA", 4, },
	{ "QUIT", 4, },
	{ "RSET", 4, },
	{ "NOOP", 4, },
	{ 0, 0, }			/* end of table marker */
};
 
#define islower(c)	('a' <= (c) && (c) <= 'z')
#define toupper(c)	(islower(c) ? ((c) - ('a' - 'A')) : (c))
