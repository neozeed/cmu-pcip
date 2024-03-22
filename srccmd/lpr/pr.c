/*  Copyright 1986 by Carnegie Mellon  */
/*  See permission and disclaimer notice in file "cmu-note.h"  */
#include	<cmu-note.h>
/*
 *      Program:        pr.c
 *      Purpose:        file paginator  filter -- splits files into
 *                      pages with header at top or bottom.  Number of
 *                      lines per page may be specified, and style of
 *                      header.
 */
 
#include <stdio.h>
 
#define FALSE           0
#define TRUE            1
#define MAXSZ           8
#define PAGE_LNTH       66
#define PAGE_WDTH       80
#define TOP_MARGIN      5
#define BOT_MARGIN      5
#define INBUFSZ         100
#define NEWLINE         '\n'
#define FORMFEED        0x0c
#define BACKUP          0x08
#define END_OF_FILE     0x1a
char    *months[12] = {
  "Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"};
 
 
main( argc, argv )
int argc;
char *argv[];
{
  FILE *fip;
  char *sp, *tp, *vp, *arg, *monthstr;
  int line_cnt, i, stop, length;
  char inbuf[INBUFSZ];
  char hdr[INBUFSZ];
  int   errflag = FALSE, top = TRUE;
  int pagenum, end, numlines, pagelength, numblanks;
  extern getdt();
  extern unsigned atyear;
  extern char atmonth;
  extern char atday;
  extern char athour;
  extern char atmin;
 
  pagelength = PAGE_LNTH;
  while (argc > 1 && argv[1][0] == '-')
  {
 
        argc--;
        arg = *++argv;
        switch(arg[1])  {
        case    'b':    /* page numbers at bottom */
                top = FALSE;
                break;
        case    'l':
                if (arg[2])
                {
                        sscanf (arg, "-l%d", &pagelength);
                        if (pagelength < (TOP_MARGIN + BOT_MARGIN +1))
                        {
                                printf("Invalid page length %d\n",pagelength);
                                errflag = TRUE;
                        }
                }
                else
                        errflag = TRUE;
                break;
        default:
                errflag = TRUE;
        }  /* end of while loop on arguments */
  }
 
  if( errflag )
  {
    printf( "usage: pr [-b] [-l#] [filename]\n" );
    exit(1);
  }
 
  if (argv[1] == NULL)
        fip = stdin;
  else
  {
        if( !(fip = fopen( argv[1], "r" )) )
        {
                printf( "File %s cannot be found: terminating.\n", argv[1] );
                exit(1);
        }
  }
  sp = argv[1];
  pagenum = 1;
  numblanks = PAGE_WDTH/2 - 2;
  numlines = pagelength - TOP_MARGIN - BOT_MARGIN;
  if (top)
  {
 
        getdt();
        monthstr = months[atmonth-1];
        sprintf(hdr, "%s %u %u:%u %u %s   Page ", monthstr, atday, athour, atmin, atyear, sp);
        for (end = 0; end < INBUFSZ; end++)     /* find end of header info */
        {
                if (hdr[end] == '\0')
                        break;
        }
  }
 
  if( !fgets( inbuf, INBUFSZ-2, fip ) )
        goto EXIT;
 
  for (;;)
  {
    line_cnt = 1;
 
    if (top)
    {
        for( i = 0;  i < TOP_MARGIN/2;  i++ )
                 putchar( NEWLINE );
        sprintf( &hdr[end], "%d\n", pagenum);
        for ( i = 0; i < strlen(hdr); i++)
                 putchar( hdr[i] );
        for( i = 0;  i < TOP_MARGIN/2;  i++ )
                 putchar( NEWLINE );
    }
    else        /* page number at bottom */
    {
        for( i = 0;  i < TOP_MARGIN;  i++ )
                 putchar( NEWLINE );
    }
    while( line_cnt <= numlines )
    {
        line_cnt++;
        sp = inbuf;
        for( ;  *sp;  sp++ )
             putchar( *sp);
        if( !fgets( inbuf, INBUFSZ-2, fip ) )
          goto EXIT;
    }   /* end of while */
 
    /* bottom margin  */
    if (top)
    {
        for( i = 0;  i < BOT_MARGIN;  i++ )
                  putchar( NEWLINE );
    }
    else
    {
        /* bottom margin -- page number in middle of trailing blank lines */
        for( i = 0;  i < BOT_MARGIN/2;  i++ )
                  putchar( NEWLINE );
        for ( i = 0; i < numblanks; i++)
                  putchar( ' ');
        sprintf( hdr, "- %d -\n", pagenum);
        for ( i = 0; i < strlen(hdr); i++)
                  putchar( hdr[i] );
        for( i = 0;  i < BOT_MARGIN/2;  i++ )
                  putchar( NEWLINE );
    }
 
    pagenum++;
  }     /* end of for */
 
EXIT:   /* finish last page */
  if (!top)
  {
        while( line_cnt <= (numlines + BOT_MARGIN/2) )
        {
                line_cnt++;
                putchar( NEWLINE );
        }       /* end of while */
 
        /* bottom margin -- page number in middle of trailing blank lines */
        for ( i = 0; i < numblanks; i++)
                  putchar( ' ');
        sprintf( hdr, "- %d -\n", pagenum);
        for ( i = 0; i < strlen(hdr); i++)
                 putchar( hdr[i] );
  }
  putchar( FORMFEED );
  putchar( END_OF_FILE );
  fclose( fip );
}
