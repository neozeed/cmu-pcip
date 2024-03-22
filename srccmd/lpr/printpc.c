/*  Copyright 1986 by Carnegie Mellon  */
/*  See permission and disclaimer notice in file "cmu-note.h"  */
#include	<cmu-note.h>
/*  Copyright 1985 by the Massachusetts Institute of Technology  */
/*  See permission and disclaimer notice in file "notice.h"  */
#include	<notice.h>

#include	<stdio.h>
#include	<task.h>
#include	<q.h>
#include	<netq.h>
#include	<net.h>
#include	<custom.h>

/* issue the DOS PRINT or tftp commands to print a file on a PC printer

  written 5/23/85 by Nancy Crowther, IBM

  modified 10/1/85 by Michael Johnson, IBM, to enable graphics printing
	on the IBM PC Graphics Printer

  modified 10/8/85 by Michael Johnson, IBM, to include the quiet flag (-q)

  modified 10/11/85 by MIcahel Johnson, IBM, to include the binary flag
	or octet flag (-o) to enable binary files to be sent to the
	IBM PC Graphics Printer.

  modified 10/11/85 by Michael Johnson, IBM, to fix bug with -p flag
	when -Plocal is set. If the temporary file is large, it will
	be deleted before the DOS PRINT command has finished printing it.
	Thus, the file is truncated to whatever part of the file made it
	to the print buffer.
*/

printPC(filename, quietflag, pflag, gflag, oflag, printer, server)
char	*filename;
int	quietflag;
int	pflag;
int	gflag;
int	oflag;
char	*printer;
in_name server;
{
	FILE	*f;
	char	cmd[300];
	char	tempname[13];
	char	*fn;

	sprintf(tempname, "$$$$zzzz.spl");
	fn = filename;
	if (gflag)
	{
		fn = tempname;
		sprintf(cmd, "prntmeta -R %s > %s", filename, tempname);
		if (!quietflag) printf("%s\n", cmd);
		system (cmd);
	}
	if (pflag)
	  if ((gflag) || (oflag))
	     {
		pflag = FALSE;
		printf("the -p was ignored, the -g and -o flags have priority\n");
	     }
	  else
	     {
		fn = tempname;
		sprintf(cmd, "pr %s > %s", filename, tempname);
		if (!quietflag) printf("%s\n", cmd);
		system (cmd);
	     }

	if (strcmp(printer,"local") == 0)
	{
		if ((!gflag) && (!pflag))
		     sprintf(cmd, "PRINT %s", fn);
		else sprintf(cmd, "COPY %s PRN", fn);
		if (!quietflag) printf("%s\n", cmd);
		system(cmd);
	}
	else  /* remote PC */
	{
		if ((gflag) || (oflag))
		     sprintf(cmd, "tftp put %s %a %s octet", fn, server, printer);
		else sprintf(cmd, "tftp put %s %a %s netascii", fn, server, printer);
		if (!quietflag) printf("%s\n", cmd);
		system(cmd);
		/* send formfeed to printer if not -p */
		if (!pflag)
		{
			if ( (strcmp(printer,"prn") == 0)  |
			(strcmp(printer,"lpt1") == 0)  |
			(strcmp(printer,"lpt2") == 0) )
			{
				if ((f = fopen(tempname, "w")) != NULL)
				{
					fputc('\f', f);
					fclose(f);
					sprintf(cmd, "tftp put %s %a %s octet", tempname, server, printer);
					system(cmd);
					unlink(tempname);

				}
			}
		}
	}

	if (pflag || gflag)  /* erase temporary file */
	{
		unlink(tempname);
	}
	return;
}
