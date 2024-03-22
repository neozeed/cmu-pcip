/*  Copyright 1984 by the Massachusetts Institute of Technology  */
/*  See permission and disclaimer notice in file "notice.h"  */
#include	<notice.h>

#
/* parse_path.c */

/* Includes those routines needed to parse smtp-style path names */

#include	<stdio.h>

extern	int	dbg;
extern	char	*smtpaddr;


parse_path (pathp)

/* Parse the path pointed to by pathp into the standard form for smtp
 * paths.  The path supplied is in the mail daemon's standard format
 * (the old arpa multiple-at-sign format).  Reverse the order of the
 * forwarding hosts and escape special characters.  Send the result
 * to the net.
 *
 * Arguments:
 */

register char	*pathp;			/* ptr. to path */
{
	register char	*p;		/* temp pointer */
	register int	atseen = FALSE;
	
	for (p = pathp; *p != '\0'; p++) /* find at signs */
		if (*p == '@')
			if (atseen)
				if (parse_hosts(p))
					break;
				else
					return (FALSE);
			else
				atseen = TRUE;


	while (pathp < p) {		/* output the mailbox */
		if (special (*pathp)) {
			if (dbg)
				putchar ('\\');
			tputc ('\\');
		}
		if (dbg)
			putchar (*pathp);
		tputc (*pathp++);
	}
	if (!atseen) {
		tputc ('@');
		tputs (smtpaddr);
	}
	return (TRUE);
}


parse_hosts (ptr)

/* Reverse the host string (in multiple-at-sign format) into the
 * standard smtp path format.  Output the result to the net.  Calls
 * itself recursively to do the reversal.  Returns TRUE if successful
 * and FALSE otherwise.
 *
 * Arguments:
 */

register char	*ptr;			/* ptr to start of '@host' */
{
	register char	*p;		/* temp */
	
	for (p = ptr + 1; *p != '\0'; p++) /* any more hosts in string? */
		if (*p == '@')
			if (parse_hosts (p))
				break;
			else
				return (FALSE);
		
	while (ptr < p) {		/* put out this host */
		if (special (*ptr)) {
			if (dbg)
				putchar ('\\');
			tputc ('\\');
		}
		if (dbg)
			putchar (*ptr);
		tputc (*ptr++);
	}
	if (dbg)
		putchar (',');
	tputc (',');
	return (TRUE);
}


special (c)

/* Returns TRUE if the character is one of the smtp special characters,
 * or FALSE otherwise.
 *
 * Arguments:
 */

register char	c;			/* the character */
{
	if (c < '\040' || c == '\177' || c == '<' || c == '>' ||
	    c == '(' || c == ')' || c == '\\' || c == ',' ||
	    c == ';' || c == ':' || c == '"')
		return (TRUE);
	return (FALSE);
}
