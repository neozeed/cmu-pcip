/*  Copyright 1986 by Carnegie Mellon  */
/*  See permission and disclaimer notice in file "cmu-note.h"  */
#include	<cmu-note.h>
/*  Copyright 1985 by the Massachusetts Institute of Technology  */
/*  See permission and disclaimer notice in file "notice.h"  */
#include	<notice.h>

/*  Monitor.c 
 *
 *  Program to monitor and display the status of network services.
 *
 *				J. H. Saltzer  11/24/85 
 */


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
#include <udp.h>
#include <timer.h>
#include <sockets.h>
#include <attrib.h>

/*  Screen format definitions */

#define TITLELINE	0	/*  Screen title */
#define HEADLINE	2	/*  column headings */
#define STARTLINE	4	/*  top of columns */
#define BOTTOMLINE	24	/*  row for special messages */
#define ERRCOL		25	/*  error message display */
#define CURTIMECOL	71	/*  time-of-day display */
#define DATECOL		59	/*  date display */

#define TIMECOL		0	/*  time server column */
#define TIME1COL	12	/*  second column of time servers */
#define TIME2COL	24	/*  lots and lots of time servers */
#define ECHOCOL		36	/*  echo server column */
#define NAMECOL		48	/*  name server column */
#define RVDCOL		60	/*  rvd server column */
#define NUMCOLS		80	/*  number of columns on display */

/*  The services that can be monitored. */

#define NONE		100	/*  Skip this service */
#define NAMESERVICE	0	/*  name resolution service */
#define RVDSERVICE	1	/*  remote virtual disk service */
#define TIMESERVICE	2	/*  time-of-day service */
#define TIME1SERVICE	3	/*  second column of tod services */
#define TIME2SERVICE	4	/*  third column of tod services */
#define ECHOSERVICE	5	/*  echo request service */
#define DOMAINSERVICE	6	/*  domain name service */

/*  The possible results  */

#define NOTTRIED	0	/*  Didn't try. */
#define RESPONDED	1	/*  Got expected response.  */
#define	RESPONDERR	2	/*  Something wrong with the response. */
#define NORESPONSE	3	/*  No response at all.  */
#define TRYTROUBLE	4	/*  Couldn't make request */

/*  Misc.  */

#define TIMEOUT		1	/*  time to wait for response, in secs.  */
#define PAUSETIME	60	/*  time to pause between passes, in secs. */
#define	GETTIME		0x2c	/*  # of DOS get-time entry point */
#define	GETDATE		0x2a	/*  # of DOS get-date entry point */

extern long	get_dosl();	/* DDP function to get date and time from DOS */
static char	s[120];		/*  buffer for setting up screen output */
static task	*montime;	/*  task that updates the 25-th line clock */
static int	tm25wake();	/*  wakeupper for 25-th line update */
static int	mon_time();	/*  function that updates 25-th line clock */
static timer	*tm25;		/*  timer that . . .  */
static task	*monitor_task;	/*  this task */
static int	monitor_wake(); /*  upcalled function that wakes this task */

static char	*month[] = { "FOO","Jan","Feb","Mar","Apr","May","Jun","Jul",
			     "Aug","Sep","Oct","Nov","Dec" };

/*  Make sure the following are #DEFINEd in order of SERVICE #definition. */

static unsigned	column[] = { NAMECOL,
			     RVDCOL,
			     TIMECOL,
			     TIME1COL,
			     TIME2COL,
			     ECHOCOL,
			     NAMECOL
			   };
static    char	working    = NORMAL;	/* display attributes */
static    char	failedonce = INTENSE;
static    char	notworking = (INTENSE|BLINK_ON);
static    char	noticed    = (NORMAL|BLINK_ON);
static	  char	trouble	   = INVERT;
static    char	wronganswer   = UNDER;

static unsigned row[NUMCOLS];	/*  holds index within service column */
static timer	*waittm;	/*  waiting timer */

extern int	pos;		/*  cursor mover */
extern int	_curse(), scr_close();	/*  functions to restore screen */
extern in_name	res_name();	/*  name resolver */

/*  static storage for server tests.  */

#define	MAXSERVICES 120

static struct  serv
{
    in_name	ipadr;		/* internet address */
    char	hostname[20];	/* name for display */
    unsigned	type;		/* kind of service to try */
    char	display;	/* display attribute (blink, etc.) */
    } service[MAXSERVICES];	/* one entry per service to test */

static int	servicenum;	/* number of services we are watching */
static int	servindex;	/* runs from 0 to servicenum-1 */
static char	testname[30];	/* name server inquiry */
static in_name	testaddr;	/* name server test answer */
static char	*star = "*";	/* activity flag */
static char	*blank = " ";	/* inactivity flag */
static int	run;		/* flag allowing operation */
static int	wait;		/* flag allowing pause */
static unsigned	timeouttime;	/* DDP time, in seconds, to wait for response */

/*  definitions for name and rvd-control clients */

#define DELTA		2		/* fiddle for name length */
struct nmitem {
	char nm_type;
	char nm_len;
	char nm_item[1]; };
#define	NI_NAME	1
#define NI_ADDR	2
#define	RVDSOCK 531			/*  The RVD Control socket number */

static	PACKET		outp;		/*  ptr to a output packet buffer */
static	PACKET		arrp;		/*  ptr to an arriving packet buffer */
static	unsigned	arrlen;		/*  length of arriving packet */
static	unsigned	reply;		/*  response to test */
static	UDPCONN		cn;		/*  UDP connection pointer  */
static	int		pkt_rcv();	/*  function to upcall on name reply */
static	char		*sendbuff;	/*  pointer to rvdctl data area */


/************************************************************************/
/*          Main part of the monitor program 				*/
/************************************************************************/

main(argc, argv)
int argc;
char *argv[];
{

    FILE	*fid;		/* descriptor of initialization file */
    char	*infile;	/* input file name */
    char	*rbuf;		/* token scan pointer */
    int		temp;		/* result of line/file scan */
    char	stype[50];	/* service type from token */
    char	sname[50];	/* service name from token */
    char	saddr[50];	/* service ip address from token */
    unsigned	result;		/* coded result of service test */
    unsigned	pausetime;	/* time, in seconds, between test passes */
    int		d_row;		/* row currently being displayed */
    int		d_column;	/* column currently being displayed */
    int		i;		/* miscellaneous loop index */


/*
 *  Here we get down to business.  Initialize . . .
 */

    scr_init();			/*  Using BIOS bypass to screen.  */
    exit_hook(_curse);		/*    "    "     "     "   "      */
    exit_hook(scr_close);	/*    "    "     "     "   "      */
    if(argc != 2)
      {
	  printf("Usage:\n\tmonitor filename\n");
	  exit(1);
      }
    infile = argv[1];
    pausetime = PAUSETIME;
    timeouttime = TIMEOUT;	/* DDP */
    strcpy(testname,"athena.athena.mit.edu");
    testaddr = 0;

/*  Set up list of services from input file suggested by user.  */


    fid = fopen(infile, "rb");
    if (fid == 0)
      {
	  printf("PC/monitor:  Can't open file %s, giving up.\n", infile);
	  exit(1);
      }
    servicenum = 0;
    while(fscanf(fid,"%s", s) != -1)
      {
	  if(servicenum >= MAXSERVICES)
	    {
		printf("too many services--truncating\n");
		break;
	    }
	  if (s[0] == 26) break;	/* stop on old DOS eof */
#ifndef MSC		/* DDP */
	  if((rbuf = (char *) index(s, '=')) != 0) rbuf[0] = ' ';
#else			/* DDP */
	  if((rbuf = (char *) strchr(s, '=')) != 0) rbuf[0] = ' '; /* DDP */
#endif			/* DDP */
	  else 
	    {
		printf("missing equal sign in this entry:  %s\n", s);
		continue;
	    }
	  rbuf++;
	  for(i=0; rbuf[i] != 0; i++) if(rbuf[i] == ';') rbuf[i] = ' ';
#ifdef notdef				/* DDP - This is BOGUS!  There is no
					   return character with fscanf()! */
	  rbuf[strlen(rbuf)-1] = 0;	/* strip off return character */
#endif

	  switch(s[0])
	    {
	    case 's':
		temp = sscanf(rbuf, "%s%s%s", stype, sname, saddr);
		if (strlen(sname)>11) 
		  {
		      printf("hostname more than 11 chars: %s\n", rbuf);
		      continue;
		  }
		strcpy(service[servicenum].hostname, sname);
		service[servicenum].type = NONE;
		if (stype[0] ==  'r') service[servicenum].type = RVDSERVICE;
		if (stype[0] ==  't')
		  {
		      if(stype[4] == '1')
			service[servicenum].type = TIME1SERVICE;
		      else if(stype[4] == '2')
			service[servicenum].type = TIME2SERVICE;
		      else  service[servicenum].type = TIMESERVICE;
		  }
		if (stype[0] ==  'e') service[servicenum].type = ECHOSERVICE;
		if (stype[0] ==  'n') service[servicenum].type = NAMESERVICE;
		if (stype[0] ==  'd') service[servicenum].type = DOMAINSERVICE;
		if (service[servicenum].type == NONE)
		  {
		      printf("service type not known: %s\n", rbuf);
		      continue;
		  }
		if (temp == 3) service[servicenum].ipadr = res_name(saddr);
		++servicenum;
		continue;
	    case 'n':
		temp = sscanf(rbuf, "%s%s", sname, saddr);
		if(temp != 2) printf("syntax problem: %s\n", rbuf);
		if (strlen(sname)>30) 
		  {
		      printf("name more than 30 chars: %s\n", rbuf);
		      continue;
		  }
		strcpy(testname, sname);
		testaddr = res_name(saddr);
		continue;
	    case 'p':
		temp = sscanf(rbuf, "%u", &pausetime);
		if(temp != 1) printf("syntax problem: %s\n", rbuf);
		continue;
/* DDP Begin */
	    case 't':
		temp = sscanf(rbuf, "%u", &timeouttime);
		if(temp != 1) printf("syntax problem: %s\n", rbuf);
		continue;
/* DDP End */
	    default:
		rbuf--;
		rbuf[0] = '=';
		printf("not recognized:  %s\n", s);
	    }
      }
    fclose(fid);

/*  Initialize the network packages and our pause timer. */

    Netinit(2500);
    in_init();
    UdpInit();
    IcmpInit();
    GgpInit();
    nm_init();

/*  set up buffers and connections for the tests */

    waittm = tm_alloc();
    if(waittm == 0)
      {
	  printf("PC/Monitor: Couldn't allocate pause timer!\n");
	  exit(1);
      }
    outp = udp_alloc(512, 0);	/*  get one packet buffer for testing */
    if(outp == 0)
      {
	  printf("PC/monitor: couldn't allocate udp packet buffer!\n");
	  exit(1);
      }
    monitor_task = tk_cur;

/*  Get the ip addresses of the servers to be tested, and initialize static. */
    for (servindex = 0; servindex < servicenum; servindex++)
      {
	  service[servindex].display = working;
	  if(service[servindex].ipadr == NAMEUNKNOWN)
	    service[servindex].ipadr =
	      resolve_name(service[servindex].hostname);
	  if(service[servindex].ipadr == NAMEUNKNOWN)
	    {
		printf("server %s is unknown.\n", service[servindex].hostname);
		service[servindex].ipadr= NONE;
	    }

	  if(service[servindex].ipadr == NAMETMO)
	    {
		printf("Can't resolve name server names--");
		printf("name servers not responding.\n");
		exit(1);
	    }

      }

/*  Pause to allow contemplation of error messages, before clearing screen. */ 

    tm_set(5, monitor_wake, 0, waittm);
    tk_block();

/*  Set up the screen. */

    tm25 = tm_alloc();	/*  set up line-25 clock updater */
    if(tm25 == 0)
      {
	  printf("PC/Monitor: Couldn't allocate clock timer!\n");
	  return;
      }
    montime = tk_fork(tk_cur, mon_time, 1000, "MonTime");
    tm_tset(TPS, tm25wake, 0, tm25);

    NDEBUG = 0;			/* shut off screen-damaging messages */
    setup_display();
    for (i=0; i < NUMCOLS; i++) row[i] = STARTLINE;

/*  Enter server test loop.  */

    run = TRUE;
    servindex = 0;
    while (run)
      {	  /*  check next service */
	  d_column = column[service[servindex].type];
	  d_row =  row[d_column];
	  write_string(star, d_row, d_column, NORMAL);
			     
	  switch(service[servindex].type)
	    {
	    case NAMESERVICE:
 		result = try_ns();
		break;
	    case DOMAINSERVICE:
		result = try_ds();
		break;
	    case RVDSERVICE:
		result = try_rs();
		break;
	    case TIMESERVICE:
		result = try_ts();
		break;
	    case TIME1SERVICE:
		result = try_ts();
		break;
	    case TIME2SERVICE:
		result = try_ts();
		break;
	    case ECHOSERVICE:
		result = try_es();
		break;
	    default:
		sprintf(s, "Service %u unknown", servindex);
		write_string(s, BOTTOMLINE, ERRCOL, NORMAL);
		result = NOTTRIED;
	    }
	  write_string(blank, d_row, d_column, NORMAL);
	  switch(result)
	    {
	    case NORESPONSE:
		if(service[servindex].display != noticed)
		  {
		      if(service[servindex].display == failedonce)
			{
			    ring();
			    service[servindex].display = notworking;
			    break;
			}
		      if (service[servindex].display == notworking) break;
		      service[servindex].display = failedonce;
		  }
		break;
	    case RESPONDED:
		service[servindex].display = working;
		break;
	    case RESPONDERR:
		service[servindex].display = wronganswer;
		break;
	    case TRYTROUBLE:
	    default:
		service[servindex].display = trouble;
	    }
	  write_string(service[servindex].hostname, d_row, d_column+1,
		       service[servindex].display);
	  row[d_column]++;
	  servindex++;
	  if(servindex >= servicenum) 
	    {   /*  Pause before doing another pass.  */
		wait = TRUE;
		for (i=0; (i<pausetime) && wait; i++)
		  {
		      tm_set(1, monitor_wake, 0, waittm);
		      tk_block();
		      check_key();
		  }
		servindex = 0;
		for (i=0; i < NUMCOLS; i++) row[i] = STARTLINE;
	    }
	  else check_key();
      }
    clear_lines(0,25);
    udp_free(outp);
    tm_clear(waittm);
    tm_free(waittm);
    tm_clear(tm25);
    tm_free(tm25);
    exit(0);

}

/************************************************************************/
/*        End of the main part of the monitor program. 		        */
/*        Lots of internal subroutines follow.			        */
/************************************************************************/

setup_display()		/*  used at start and if the display gets muddled */
{
    clear_lines(0, 25);
    sprintf(s,"PC/IP Network Service Monitor, version %u.%u",
	    				version/10, version%10);
    write_string(s, TITLELINE, 19, NORMAL);
    sprintf(s,"  Name");
    write_string(s, HEADLINE, NAMECOL, NORMAL);
    sprintf(s,"  RVD");
    write_string(s, HEADLINE, RVDCOL, NORMAL);
    sprintf(s,"  Echo");
    write_string(s, HEADLINE, ECHOCOL, NORMAL);
    sprintf(s,"  Time");
    write_string(s, HEADLINE, TIMECOL, NORMAL);
    write_string(" quit: q  clear: c  go: g  ack: space",
		 BOTTOMLINE,0, NORMAL);
    set_cursor(pos = (BOTTOMLINE*80));
}

/*  See if the user wants to stop or change the display */

check_key()
{
    char	key;		/* user's typed input */
    int		i;		/* loop index */

    key = h19key();
    if (key == 'q') wait = (run = FALSE);
    if (key == 'g') wait = FALSE;
    if (key == ' ') 
      {			/* if user hit space, reduce display intensity. */
	  for(i = 0; i < servicenum; i++)
	    {
		if (service[i].display == notworking)
		  service[i].display = noticed;
	    }
      }
    if (key == 'c') setup_display();
}


/*  Here are the tests for presence of servers  */

/*  Echo service test.  Must use ICMP subroutine, because ICMP will snag all
    ICMP-looking replies. */

try_es()
{
    unsigned	res;

    res = IcEchoRequest(service[servindex].ipadr,20);
    if (res == PGTMO) return NORESPONSE;
    if (res == PGSUCCESS) return RESPONDED;
    return RESPONDERR;
}

/*  Time service */

try_ts()
{
    long	rcvdtime;	/* time received in the test */
    long	*ptm;		/* ptr to byte-swapped incoming result */

    /* zero-length packet is time request (!) */

    reply = NORESPONSE;
    cn = udp_open(service[servindex].ipadr,UDP_TIME,udp_socket(),pkt_rcv,0);
    if(cn == 0) return TRYTROUBLE;
    if((udp_write(cn, outp, 0) < 0)) return TRYTROUBLE;
    tm_set(timeouttime, monitor_wake, 0, waittm);	/* DDP */
    tk_block();		/* Wait for response to come back */

/*  reply received or timeout happened; either way, clean up and exit. */

    tm_clear(waittm);
    udp_close(cn);
    if (reply == RESPONDED) 
      {
	  ptm = (long *)udp_data(udp_head(in_head(arrp)));
	  rcvdtime = lswap(*ptm) ;
	  /* should check answer for plausibility */
	  udp_free(arrp);
      }
    return reply;
}

/*  RVD-control service */

try_rs()
{

    sendbuff = udp_data(udp_head(in_head(outp)));
    sprintf(sendbuff,"operation=shutdown\npassword=x\n");

    reply = NORESPONSE;
    cn = udp_open(service[servindex].ipadr,RVDSOCK,udp_socket(),pkt_rcv,0);
    if(cn == 0) return TRYTROUBLE;
    if((udp_write(cn, outp, strlen(sendbuff)) < 0)) return TRYTROUBLE;
    tm_set(timeouttime, monitor_wake, 0, waittm);	/* DDP */
    tk_block();		/* Wait for response to come back */

/*  reply received or timeout happened; either way, clean up and exit. */

    tm_clear(waittm);
    udp_close(cn);
    if (reply == RESPONDED) udp_free(arrp);
    return reply;
   }

/*  Domain Name service */

try_ds()
{
    in_name	res, dm_resolve();

    res = dm_resolve(testname, service[servindex].ipadr);
    if (res == NAMETMO)		return NORESPONSE;
    if (res == NAMETROUBLE)	return TRYTROUBLE;
    if (res == NAMEUNKNOWN)	return RESPONDERR;
    if (res == testaddr)	return RESPONDED;
    return RESPONDERR;
}

/*  Old-fashioned IEN-116 name service */

try_ns()
{
    register struct nmitem *pnm;
    int len;

    len = strlen(testname) + sizeof(struct nmitem) - DELTA;
    pnm = (struct nmitem *)udp_data(udp_head(in_head(outp)));
    pnm->nm_type = NI_NAME;
    pnm->nm_len = len - DELTA;
    strcpy(pnm->nm_item, testname);

    reply = NORESPONSE;
    cn = udp_open(service[servindex].ipadr,UDP_NAME,udp_socket(),pkt_rcv);
    if (cn == 0) return TRYTROUBLE;
    if((udp_write(cn, outp, len) < 0)) return TRYTROUBLE;
    tm_set(timeouttime, monitor_wake, 0, waittm);	/* DDP */
    tk_block();

/*  reply received or timeout happened; either way, clean up and exit. */

    tm_clear(waittm);
    udp_close(cn);
    if (reply == RESPONDED)
      {
	  pnm = (struct nmitem *)udp_data(udp_head(in_head(arrp)));
	  pnm = (struct nmitem *)((char *)pnm + pnm->nm_len + 2);
	  if(pnm->nm_type != NI_ADDR) reply = RESPONDERR;
	  udp_free(arrp);
      }
    return reply;
}


/*  Upcall for arriving packet; just save packet info for later processing */

static		pkt_rcv(inp, len, host)
PACKET		inp;
unsigned	len;
in_name		host;
{
/*    if (host != service[servindex].ipadr)*/	/*  Make sure it is ours.  */
/*    {				*/		/*  Not ours, ignore.  */
/*	  udp_free(inp);*/
/*	  return;*/
/*    }*/
    arrp = inp;
    arrlen = len;
    reply = RESPONDED; 
    monitor_wake();
}

static		monitor_wake()
{
    tk_wake(monitor_task);
}

static		tm25wake()
{
    tk_wake(montime);
}

static		mon_time()
{
    union
      {long l; char c[4]; int i[2];}
		time;		/* to decode DOS date & time format */
    char s1[60];
    char s[40];
    unsigned secs;

    while(1)
      {
	  time.l = get_dosl(GETDATE);
	  sprintf(s,"%02d %s %04d",
		  time.c[0],month[time.c[1]], time.i[1]);
	  write_string(s, BOTTOMLINE, DATECOL, NORMAL);
	  time.l = get_dosl(GETTIME);
	  sprintf(s, "%02u:%02u:%02u", time.c[3], time.c[2], time.c[1]);
	  write_string(s, BOTTOMLINE, CURTIMECOL, NORMAL);
	  tm_clear(tm25);
	  tm_tset(TPS, tm25wake, 0, tm25);
	  tk_block();
      }
}
