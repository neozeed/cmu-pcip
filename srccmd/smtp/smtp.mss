@device[imprint10]
@Make(manual)
@Section[Synopsis]
@begin(verbatim)
include <nsmtp.h>		/* must include TCP, Timers before this */
@end(verbatim)

@Section[@i{smtp@ux( )open()}]
@begin(verbatim)
Smtp smtp_ open(uc_ procs, mode [, for_ host])
	int (*uc_ procs[6])();
	int mode;		/* USER, SERVER, or REJECTION */
	in_ name *for_ host;	
@end(verbatim)

This routine creates an SMTP with six upcall procedures
(explained later).  The type of SMTP is controlled by the mode variable,
which is set to the USER, SERVER, or REJECTION constant.  

A user SMTP
sends mail, and requires a third argument which specifies the 
foreign host for the TCP connection.  SMTP automatically picks a random
local socket.

Servers, which wait for connection from other hosts, receive mail.
To create a server, only the upcall procedures and the server type
are needed.

A Rejection SMTP is a special kind
of server that immediately closes the communication channel when a foreign
hosts calls.
This type of SMTP can be used once the number of connected
servers reaches a defined limit (like when memory runs out).  Most
foreign hosts will try to resend the mail later.

@Section[User and Server Upcalls]
@SubSection[The Message Destination]
@begin(verbatim)
int us_ mail_ hop(smtp,from,to,host)		/* server mode only */
	Smtp smtp;
	char *from;  /* e.g. "<@@comet,@@mit-multics:DClark@@mit-multics>" */
	char *to;    /* e.g. "<gillies@@mit-comet>"			 */
	char *host;  /* e.g. "mit-comet"				*/
@end(verbatim)

This procedure communicates a mailbox path to the client.  The path
contains an RFC822 "From:" and "To:" argument, and the next host
to send the mail to, each terminated by a '\0'.  NOTE: The From: and
To: lines are preprocessed: if the message is source-routed,
one source host has already been extracted from the "To:" line and 
prepended to the "From:" line.

The strings live in temporary storage so they should be immediately copied.
If the SMTP server cannot accept the mail (i.e. if
a @i(local) recipient does not exist), SMTP will
refuse the mail if us@ux( )mail returns FALSE.
To accept the mail, it returns TRUE.
The text of the mail will be upcalled later.

@subsection[Major Smtp Events]
@begin(verbatim)
int us_ event(smtp,type)     /* server mode except open, close: both modes */
	Smtp smtp;
	int type;	    /* OPEN, CLOSE, TURN, or RESET */
@end(verbatim)
The OPEN
event is upcalled when SMTP is open and ready to receive or send
commands to the user.  When a connection turns around, the OPEN
event will be upcalled once SMTP is ready for more input or output.
It is illegal to make downcalls before each OPEN upcall.

The CLOSE event is upcalled when the communication channel is completely
closed, and the connection block nears deletion.  All transactions
are acknowleged before the CLOSE event is upcalled.

The TURN event upcall signals a request from SMTP to switch
user and server roles.  A
mail repository's SERVER might receive a TURN request,
from other hosts polling the repository for mail.
If the upcalled program can handle a TURN command, it should return
TRUE, otherwise it should return FALSE.  

The RESET upcall invalidates
any mail@ux( )hop and us@ux( )data upcalls that 
have already occured since the last us@ux( )confirm upcall.

@Subsection[Receiving the Mail Text]
@begin(verbatim)
us_ data(smtp,datablk)		 /* server mode only */
	Smtp smtp;
	char *datablk;  	/* A maximum of 1000 characters at once */
@end(verbatim)

This upcall communicates a chunk of mail text to the client.
When the mail text ends, the us@ux( )confirm() upcall confirms that the
piece of mail has been reliably stored.

@Subsection[Verifying the Mail is Stored]
@begin(verbatim)
int us_ confirm(smtp)		 /* server mode only */
	Smtp smtp;
@end(verbatim)
This upcall terminates a mail text (communicated by us@ux( )data() upcalls)
and ensures the text is reliably stored, before SMTP takes responsibility 
for the mail.  If for some reason the mail is 
not stored, the upcalled function should return FALSE.

@Subsection[The Host's Name]
@begin(verbatim)
char *us_ get_ myname(smtp)	  /* user and server smtp */
	Smtp smtp;
@end(verbatim)
This procedure must return the name of this host.  In some cases,
one host may want to dynamically masquerade as another (e.g. special
mail forwarders).
In these cases, the returned name can be computed
depending on lower-level information.  Normally this
procedure will return a constant pointer, like "MIT-ZUD".

@Subsection[Result of a Mailing]
@begin(verbatim)		
us_ sent(trans,status)		 /* user smtp only */
	SmtpTrans trans;	/* mail transaction */
	int status		/* result of the transaction */
@end(verbatim)
This procedure notifies the user about whether a mailing was successful.
There are at least 5 reply codes:  NO@ux( )FILE, ERROR, TRY@ux( )LATER, 
NO@ux( )SUCH@ux( )USER, HOST@ux( )PROBLEM, and SUCCESSFUL.
NO@ux( )FILE is returned when the file to be mailed is not on the disk.
ERROR indicates that there was an error in the from: or to: lines.
TRY@ux( )LATER indicates that a temporary problem stopped the mail from 
being sent.  NO@ux( )SUCH@ux( )USER indicates that there was no such user 
on the foreign machine.  HOST@ux( )PROBLEM indicates a permanent
processing problem with the foreign SMTP which
prevents it from receiving the mail.  Finally, SUCCESSFUL indicates that
the mail has been reliably sent, and may be deleted from this machine.

@Section(User Mode SMTP)

@Subsection[@i{smtp@ux( )mail()}]
@begin(verbatim)
SmtpTrans smtp_mail(smtp,from,to,message);
	Smtp smtp;
	char *from;		/* e.g. "<gillies@@mit-comet>"    */
	char *to;		/* e.g. "<romkey@@borax>"         */
	char *message;		/* the name of a message file	  */
@end(verbatim)
This procedure is called to package a mail transaction.  Mail transactions
are passed down to SMTP for mailing, and acknowleged by upcalls.  Mail
transactions may be combined by SMTP for efficient transmission, especially
if one file is being sent to many recipients on one host.  SMTP will
transmit these efficiently if the all the transactions fit into memory
before they are flushed.

These macros allow the user to decode a transaction
@begin(verbatim)
char *Trans_ get_ from(x);	/* returns from: argument	  */
char *Trans_ get_ to(x);	/* returns to: argument		  */
char *Trans_ get_ filename(x);	/* returns the message file name  */
@end(verbatim)


@Subsection[@i{smtp@ux( )flush()}]
@begin(verbatim)
smtp_ flush(smtp)
	Smtp smtp;
@end(verbatim)
This call forces Smtp to send any outstanding mail transactions.  It
is especially useful if the user program runs out of memory and must
flush memory to free up space for new transactions.

@Subsection[@i{smtp@ux( )close()}]
@begin(verbatim)
smtp_ close(smtp)
	Smtp smtp;
@end(verbatim)
This procedure flushes all the mail transactions and closes the smtp
channel.  All outstanding mail transactions will soon be upcalled,
and the the us@ux( )event function will signal an SMTP close.

@Subsection[@i{smtp@ux( )turn()}]
@begin(verbatim)
int smtp_ turn(smtp)
	Smtp smtp;
@end(verbatim)
This procedure flushes all outstanding mail transactions, and asks
the foreign host to reverse server and user roles.  The procedure
returns TRUE if the channel was turned around, or FALSE if the
foreign host refused.  This command allows one host to poll another
for its mail.
