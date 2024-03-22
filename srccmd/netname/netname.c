/*  Copyright 1986 by Carnegie Mellon  */
/*  See permission and disclaimer notice in file "cmu-note.h"  */
#include	<cmu-note.h>
/*  Copyright 1986 by the Massachusetts Institute of Technology  */
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
#include <udp.h>
#include <timer.h>
#include <sockets.h>

/* PC/netname
 * A name lookup command, using domain name system.  January, 1986.
 * 					<J. H. Saltzer>
 */

extern	int	dm_timeout;		/* time to wait for response */
extern	char	req_cname[100];		/* returned primary name */
 					/* (belongs to dm_name) */
main(argc, argv)
int	argc;
char	*argv[];
{
    int		i;			/*  name server loop index */
    int		argcnt;			/*  previous value of argument count */
    int		trace;			/*  switch TRUE if trace wanted */
    char	sname[100];		/*  place to put name of name server */
    char	temp[100];		/*  place to put name being resolved */
    in_name	resolve_name();		/*  used to resolve name server name */
    in_name	dm_resolve();		/*  used to invoke domain name server*/
    in_name	host;			/*  the answer */

    if(argc < 2 || argc > 6)
      {
usage:
	  printf("To resolve domain names:\n");
	  printf("\tnetname [-t timeout] [-a] name [domain-name-server]\n");
	  printf("(optional timeout is in seconds)\n");
	  exit(1);
      }

    Netinit(800);
    in_init();
    UdpInit();
    IcmpInit();
    GgpInit();
    nm_init();

    			/* set up defaults, then look for overrides. */
    trace = FALSE;	/* don' trace unless asked. */
    dm_timeout = 20;	/* long wait, we explore obscure namespaces  */

    argcnt = 0;		/* pick off optional arguments */
    while (argcnt != argc)
      {
	  argcnt = argc;		/* stops loop if nothing is found */
	  if(strcmp(argv[1], "-a") == 0) /* user wants more output */
	    {
		trace = TRUE;		/* N.B. this feature doesn't do any */
		argv++; argc--;		/* good unless dm_name.c is compiled*/
	    }				/* using the DEBUG option */

	  if(strcmp(argv[1], "-t") == 0)
	    {
		if (argc < 4) goto usage;
		dm_timeout = atoi(argv[2]);
		argv++; argv++;
		argc--; argc--;
	    }
      }
    sname[0] = 0;
    if(argc == 3)
      {
	  strcpy(sname, " ");
	  strcpy(&sname[1], argv[2]);
	  host = resolve_name(argv[2]);
	  if(host == 0L)
	    {
		printf("Domain name server%s is unknown.\n", sname);
		exit(1);
	    }

	  if(host == 1L)
	    {
		printf("Can't find address of domain name server%s\n", sname);
		printf("Customized name servers not responding.\n");
		exit(1);
	    }

	  /* Look the other way for a moment.  */
	  custom.c_dm_servers[0] = host;
	  custom.c_dm_numname = 1;
      }
    else printf("Using customized list of domain name servers.\n");

#ifndef MSC				/* DDP - Begin */
    if(!index(argv[1], '.') && strlen(custom.c_domain))
#else
    if(!strchr(argv[1], '.') && strlen(custom.c_domain))
#endif					/* DDP - End */
      {
	  sprintf(temp, "%s.%s", argv[1], custom.c_domain);
	  printf("Name extended with default domain:  %s\n", temp);
      }
    else strcpy(temp, argv[1]);
    if(trace) NDEBUG = NDEBUG|APTRACE;

    for (i = 0; i < custom.c_dm_numname; i++)
      {
	  host = NAMETMO;
	  host = dm_resolve(temp, custom.c_dm_servers[i]);

	  if (host == NAMETROUBLE)
	    {
		printf("Couldn't send to domain name server%s (%a)\n",
		       sname, custom.c_dm_servers[i]);
	    }
	  else if (host == NAMETMO)
	    {
		printf("Domain name server%s (%a) did not respond\n",
		       sname, custom.c_dm_servers[i]);
	    }
	  else if (host == NAMEUNKNOWN)
	    {
		printf("Domain name server%s (%a)\n didn't know name %s\n",
		       sname, custom.c_dm_servers[i], temp);
	    }
	  else
	    {
		printf("Domain name server%s (%a)\n",
		       sname, custom.c_dm_servers[i]);
		printf("reports address of %s as: %a\n",
		       	temp, host);
		if (strcmp(req_cname, temp))
		  printf("and primary name as %s\n", req_cname);
	    }

      }

    exit(0);

}
