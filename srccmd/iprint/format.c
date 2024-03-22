/*  Copyright 1986 by Carnegie Mellon  */
/*  See permission and disclaimer notice in file "cmu-note.h"  */
#include	<cmu-note.h>
/*  Copyright 1984 by the Massachusetts Institute of Technology  */
/*  See permission and disclaimer notice in file "notice.h"  */
#include	<notice.h>

/* format.c
	simple page formatter; adds page headers & page numbers to files

Oct 12, 1985 - Changed call to _fdate() into calls to fstat() and ctime()
	if MSC is defined, for Microsoft compiler.
					<Drew Perkins>
*/

#include <stdio.h>
#ifdef MSC			/* DDP - Begin */
#include <sys\types.h>
#include <sys\stat.h>
#include <time.h>
#endif				/* DDP - End */


/* page header layout:

filename			date				Page #
*/

#define	put_string(s)	{ ptr = (s); while(*ptr) (*dispose)(*ptr++, disp_arg);}
#define	put_char(c)	(*dispose)((c), disp_arg)

#define	BACKSPACE	'H'-'@'
#define	FORMFEED	'L'-'@'

char *malloc();

char *months[] = { "???", "January", "February", "March", "April",
	"May", "June", "July", "August", "September", "October",
	"November", "December"};

pg_format(name, dispose, disp_arg, page_len, line_len)
	char *name;
	int (*dispose)();
	unsigned disp_arg;
	int page_len, line_len; {
	register FILE *fin;
	int lines = 0;
	int pageno = 1;
	char *underbar;
	register char *ptr;
	int i;
	char pagenobuf[20];
	char datebuf[30];
	int c;
	short dtime[2];

	page_len -= 2;

	underbar = malloc(line_len+1);
	if(underbar == NULL) underbar = "";
	else {
		for(i=0; i<line_len; i++)
			underbar[i] = '_';

		underbar[line_len] = '\0';
		}

	fin = fopen(name, "ra");
	if(fin == NULL) {
		fprintf(stderr, "can't open file %s\n", name);
		return 1;
		}

#ifndef MSC				/* DDP */
	_fdate(fileno(fin), dtime, 0);
	sprintf(datebuf, "%2d %s %4d %02d:%02d:%02d", dtime[0] & 0x1f,
		months[(dtime[0] >> 5)&0xf], 1980+((dtime[0]>>9)&0x7f),
		(dtime[1]>>11)&0x1f, (dtime[1]>>5)&0x3f, (dtime[1] & 0x1f)/2);
#else					/* DDP - Begin */
	{
		struct stat statbuf;

		fstat(fileno(fin), &statbuf);
		strcpy(datebuf, ctime(statbuf.st_atime));
		datebuf[24] = '\0';
	}
#endif					/* DDP - End */

top_of_page:
	put_string(underbar);
	put_char('\r');

	put_string(name);

	/* 13 = strlen("Page xxxxxxxx") 	*/
	i = (line_len - strlen(name) - 14 - strlen(datebuf))/2;
	while(i--) (*dispose)(' ', disp_arg);

	put_string(datebuf);

	i = (line_len - strlen(name) - 14 - strlen(datebuf))/2;
	while(i--) (*dispose)(' ', disp_arg);

	sprintf(pagenobuf, "Page %u\r\n\r\n", pageno);
	put_string(pagenobuf);

	/* okay, now send some text */
	while(1) {
		c = getc(fin);
		switch(c) {
		case EOF:
			fclose(fin);
			return 0;
		case '\n':
			if(++lines == page_len) {
				lines = 0;
				pageno++;
				put_char(FORMFEED);
				goto top_of_page;
				}
			put_char('\r');
			put_char(c);
			break;
		case FORMFEED:
			lines = 0;
			pageno++;
			put_char(c);
			goto top_of_page;
		default:
			put_char(c);
			}
		}
	}
