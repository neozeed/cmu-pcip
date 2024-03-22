/*  Copyright 1986 by Carnegie Mellon  */
/*  See permission and disclaimer notice in file "cmu-note.h"  */
#include	<cmu-note.h>
/*  Copyright 1984 by the Massachusetts Institute of Technology  */
/*  See permission and disclaimer notice in file "notice.h"  */
#include	<notice.h>

#include <stdio.h>
#include <types.h>
#include <task.h>
#include <q.h>
#include <netq.h>
#include <net.h>
#include <custom.h>
#include <netbuf.h>
#include <icmp.h>
#include <ip.h>

/*  Procedure to convert the seconds from a network time server, and
    set the PC clock accordingly.  <J. H. Saltzer > */

/*  Removed bug in days-per-year calculation, 12/31/83. <J.H. Saltzer>
    Readjusted calculation to base on 1/1/81 rather than 1/1/80,
    to simplify leap-year processing and remove a bug that caused
    12/31 to be shown as 1/1 once every four years.  12/31/84.
    Added Daylight Savings Time calculation, and incidentally,
    display of day of week.  4/28/85.
						 <J. H. Saltzer>  */
/*  Microsoft C V3.00 didn't like a long value that was unsigned, so
    the most-significant bit must be stripped off in a different manner.
    						<Drew Perkins> */

#ifndef MSC			/* DDP */
#define START	2556144000L	/* seconds between 1/1/1900 and 1/1/1981 */
#else				/* DDP - Begin */
#define START	408660352L	/* seconds between 1/1/1900 and 1/1/1981 */
#endif				/* DDP - End */

#define	QUADY	126230400L	/* seconds in four years, including
							one leap day */
#define	YEAR	31536000L	/* seconds in one non-leap year */
#define	DAY	86400L		/* seconds in one day */
#define	HOUR	3600		/* seconds in one hour */
#define	MINUTE	60		/* you guessed it */
#define WKDAY81	3		/* 1 January 1981 was 3 days before Sunday */
#define DSTBGN	120		/* day number, 30 April, non-leap-year */
#define DSTEND	305		/* day number, 31 October, non-leap-year */

int	setdosl();

set_pc_clk(arg)
	long	arg; {	/*  arg is number of seconds since
					 midnight GMT January 1, 1900  */
	int i;
	int nmons;
	int ndate;
	int nquads, dnquads;		/* Full 4-yr blocks since START */
	int nyrs, dnyrs, xnyrs;		/* Full years in this block */
	int ndays, dndays, xndays;	/* Number of this day in this year  */
	int nhrs;			/* Full hours in this day  */
	int nmins;			/* Full minutes in this hour  */
	int nsecs;			/* Residue after removing minutes */
	int wkday, dwkday, xwkday;	/* Day of week, Sunday = 0 */
	int next_sunday;		/* Next Sunday's day number */
	int dst;			/* Flag, TRUE if DST is in effect  */
	long offset;
	char *label;
	long time, dtime, xtime;
	static int days[12] = {31,28,31,30,31,30,31,31,30,31,30,31};
	static char *dayname[] = {"Sunday","Monday","Tuesday","Wednesday",
				  	   "Thursday","Friday","Saturday"};

	label = custom.c_tmlabel;
	offset = custom.c_tmoffset*60L;
#ifdef MSC			/* DDP */
	arg &= 0x7fffffff;	/* DDP */
#endif				/* DDP */
	time = arg - START;	/*  move to 1981, to eliminate sign bit.*/
	time = time - offset;	/*  move to local time before figuring date.*/
	dtime = time + 3600;	/*  daylight savings time, if needed */
	xtime = time - 7200;	/*  start DST at midnight, 2 zones west */
	wkday = ((time/86400 - WKDAY81) % 7); /*  calculate day of week */
	dwkday = ((dtime/86400 - WKDAY81) % 7); /* repeat everything in dst */
	xwkday = ((xtime/86400 - WKDAY81) % 7); /* and in 2-zone-west time */
	dnquads = nquads = 0;
	xnyrs = dnyrs = nyrs = 0;
	xndays = dndays = ndays = 1;	/*  Note that dates are one-origin */
	nmins = 0;		/*  But minutes and hours are zero-origin */
	nhrs = 0;
	dst = FALSE;
	while( time  >= QUADY )
		{ time = time - QUADY;
		  ++nquads; }
	while( dtime  >= QUADY )
		{ dtime = dtime - QUADY;
		  ++dnquads; }
	while( xtime  >= QUADY )
		{ xtime = xtime - QUADY;
	      		}

	/*  Can't use while() here because fourth year is longer.  */
	if( time >= YEAR ) { time = time - YEAR; ++nyrs; }
	if( time >= YEAR ) { time = time - YEAR; ++nyrs; }
	if( time >= YEAR ) { time = time - YEAR; ++nyrs; }
	if( dtime >= YEAR ) { dtime = dtime - YEAR; ++dnyrs; }
	if( dtime >= YEAR ) { dtime = dtime - YEAR; ++dnyrs; }
	if( dtime >= YEAR ) { dtime = dtime - YEAR; ++dnyrs; }
	if( xtime >= YEAR ) { xtime = xtime - YEAR; ++xnyrs; }
	if( xtime >= YEAR ) { xtime = xtime - YEAR; ++xnyrs; }
	if( xtime >= YEAR ) { xtime = xtime - YEAR; ++xnyrs; }

	while( time >= DAY )
		{ time = time - DAY;
		  ++ndays; }
	while( dtime >= DAY )
		{ dtime = dtime - DAY;
		  ++dndays; }
	while( xtime >= DAY )
		{ xtime = xtime - DAY;
		  ++xndays; }

	if(label[1] == 's' || label[1] == 'S')
	    {
		/*  Assume user wants DST adjustment; calculate next Sunday's
		    day number, to see if adjustment is needed.  DST starts
		    and ends one day late in leap years. */

		next_sunday = xndays - xwkday + 7 - (xnyrs == 3);
		if((DSTBGN < next_sunday) && (next_sunday < DSTEND))
		   {   /*  OK, use the DST stuff for real  */
		       dst = TRUE;
		       nquads = dnquads;
		       nyrs = dnyrs;
		       ndays = dndays;
		       wkday = dwkday;
		       time = dtime;
		       if(label[1] == 's') label[1] = 'd';
		       if(label[1] == 'S') label[1] = 'D';
		   }
	     }

	while( time >= HOUR )
		{ time = time - HOUR;
		  ++nhrs; }

	while( time >= MINUTE )
		{ time = time - MINUTE;
		  ++nmins; }


	if(NDEBUG & APTRACE)
	printf("4yrs %u, Yrs %u, Days %u, Wkday %u Hrs %u, Mins %u, DST %u\n",
				nquads, nyrs, ndays, wkday, nhrs, nmins, dst);
	if (nyrs == 3) days[1]++;
	for( i = 0; ndays > days[i]; i++) ndays = ndays - days[i];
	nmons = i+1;
	ndate = ndays;
	nyrs = nyrs + 4*nquads + 1981;
	nsecs = time;

	if ( setdosl( 0x2B, nyrs, (nmons*256 + ndate )) == 0 )
		printf("setting ");
		else printf("unable to set ");
	printf("date to %s, %d/%d/%02d\n",
	       		dayname[wkday], nmons, ndate, nyrs-1900);
	if ( setdosl( 0x2D, (nhrs*256 + nmins), nsecs*256) == 0 )  
		printf("setting ");
		else printf ("unable to set");  
	printf("time to %02u:%02u:%02u %s\n", nhrs, nmins, nsecs,
							label);
	return;
	}
