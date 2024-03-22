/*  Copyright 1986 by the  Massachusetts Institute of Technology */
#include <notice.h>

/*  When you have 125 PC's all of which need the same change to their
custom structure, wire the required change into a version of this
program.
 					<J. H. Saltzer, 4/25/86>
*/

#include	<stdio.h>
#include	<task.h>
#include	<q.h>
#include	<netq.h>
#include	<net.h>
#include	<custom.h>
#include	<colors.h>
#include	<attrib.h>
#define		DATE	0x2a
#define		TIME	0x2c

extern int	errno;
extern char	*sys_errlist[];
       long	get_dosl();

main(argc, argv)
int argc;
char *argv[];
{
    struct custom	custom;
    int			newfd;
    char		temp[40];

    if(argc != 2)  /*  check arguments */
      {
	  printf("usage:\n\tcustfix devicefile\n");
	  exit(1);
      }

    /*  fill out file name if necessary */
#ifndef MSC		/* DDP */
    if(index(argv[1], '.'))	strcpy(temp, argv[1]);
#else			/* DDP */
    if(strchr(argv[1], '.'))	strcpy(temp, argv[1]);
#endif			/* DDP */
    else 			sprintf(temp, "%s.sys", argv[1]);

    /*  find and read the custom file */
    newfd = open(temp, 2);
    if(newfd < 0)
      {
	  printf("can't open %s: %s\n", temp, sys_errlist[errno]);
	  exit(1);
      }

    mkraw(newfd);
    read(newfd, &custom, sizeof(struct custom));

    if(custom.c_iver != CVERSION)  /*  Make sure it's the kind we understand */
      {
	  printf("wrong version of custom structure: got %d, expected %d\n",
		 custom.c_iver, CVERSION);
	  close(newfd);
	  exit(1);
      }

    /* looks good,  fix it up as needed */

    /*  This particular fixup provides Athena domain name servers. */

    /*  First, set the old-fashioned name servers */
    custom.c_numname = 2;
    custom.c_names[0] = (in_name)0x10003A12;   /*  18.58.0.16  (Gaea) */  
    custom.c_names[1] = (in_name)0xA0004612;   /*  18.70.0.160 (W20NS) */

    /*  Now, the new ones and the default domain */
    custom.c_dm_numname = 3;
    custom.c_dm_servers[0] = (in_name)0x10003A12;   /*  18.58.0.16  (Gaea) */  
    custom.c_dm_servers[1] = (in_name)0xA0004612;   /*  18.70.0.160 (W20NS) */
    custom.c_dm_servers[2] = (in_name)0x03004812;   /*  18.72.0.3   (Bitsy) */
    strcpy(custom.c_domain, "MIT.EDU");

    /*  Leave a time and date stamp */
    custom.c_ctime = get_dosl(TIME);
    custom.c_cdate = get_dosl(DATE);

    /*  All done, now write the file back and close up shop. */

    lseek(newfd, 0L, 0);
    write(newfd, &custom, sizeof(struct custom));
    close(newfd);
    exit(0);

}


