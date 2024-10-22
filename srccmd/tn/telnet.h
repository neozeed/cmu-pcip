/* Copyright 1986 by Carnegie Mellon */
/*  Copyright 1986 by Carnegie Mellon  */
/*  See permission and disclaimer notice in file "cmu-note.h"  */
#include	<cmu-note.h>
/*  Copyright 1984 by the Massachusetts Institute of Technology  */
/*  See permission and disclaimer notice in file "notice.h"  */
#include	<notice.h>

/* 11-5-85	Added terminal type information, and option negotiation code
		from Jacob Rekhter, IBM Corp.
					<Drew D. Perkins>
 */

/* Definitions for telnet */

struct ucb {
	int	u_state;	/* ESTAB or CLOSING */
	int	u_tcpfull;	/* is tcp's output buffer full? */
	int	u_tftp;		/* tftp acceptance */
	int	u_rstate;	/* Read terminal state */
				/* NORMALMODE */
				/* BLOCK - don't read terminal */
	int	u_rspecial;	/* Read terminal character handling */
				/* NORMALMODE */
				/* SPECIAL - processing char after prompt */
				/* CONFIRM - quit confirmation */
				/* TCPFULL - tcp output buffer full */
	int	u_wstate;	/* Write terminal state */
				/* NORMALMODE */
				/* URGENTM - urgent mode
				   ignore nonspecial chars */
	int	u_wspecial;	/* Write terminal character handling */
				/* NORMALMODE */
				/* '\r' (13), IAC, WILL, WONT, DO, DONT -
				   processing special chars */
	int	u_prompt;	/* Prompt char */
	int	u_sendm;	/* Send mode */
				/* EVERYC - send to net on every char */
				/* NEWLINE - send to net on newline */
	int	u_echom;	/* Echo mode */
				/* LOCAL - local echo */
				/* REMOTE - remote echo */
	int	u_echongo;	/* Echo negotiation request outstanding */
				/* NORMALMODE */
				/* LECHOREQ - IAC DONT ECHO was sent */
				/* RECHOREQ - IAC DO ECHO was sent */
	int	u_mode;		/* status line on or off flag */
	int	u_ask;		/* ask about tftp transfers */
	int     u_terminal;	/* DDP - ASCII or 3278 */
	};

#define NORMALMODE		0
#define SPECIAL		1
/*
#define TEST		2
*/
#define	CONFIRM		3
#define HOLD		4

#define	TFUNKNOWN	1
#define TFWAITING	2
#define	TFYES		3
#define	TFNO		4

#define ASCII_TERM      1       /* DDP - ASCII terminal emulator */
#define E3278_TERM      2       /* DDP - 3278 emulator */

#define	BLOCK		1
#define NOBLOCK		2
#define URGENTM		1
#define EVERYC		1
#define NEWLINE		2
#define LOCAL		1
#define REMOTE		2
#define LECHOREQ	1
#define RECHOREQ	2

#define IAC	255
#define WILL	251
#define WONT	252
#define DO	253
#define DONT	254
#define DM	242
#define INTP	244
#define AO	245
#define AYT	246
#define GA	249
#define OPTECHO	1
#define	OPTSPGA	3
#define TRANS_BIN	0	/* DDP - Transmit Binary */
#define TERMTYPE	24	/* DDP - TERMINALtype */
#define OPTEOR	25		/* DDP - End of Record */
#define SBNVT	250             /* DDP - SBnvt*/
#define SENVT	240             /* DDP - SEnvt */
#define IS	0               /* DDP - ISchar */
#define SEND	1               /* DDP - SEnvt */
#define EOR	239             /* DDP - End-of-Record */

#define	DFESC	'\036'		/* default escape char */

#define ESTAB	1
#define CLOSING	2
#define CLOSED	3

#define TELNETSOCK	23	/* Telnet well known socket no. */

extern struct ucb	ucb;
extern struct	task		*TNsend;	/* Telnet send task. */
extern int	speed;
extern char tnshost[];		/* foreign host's string name */
