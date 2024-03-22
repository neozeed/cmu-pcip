/*  Copyright 1984 by the Massachusetts Institute of Technology  */

/* definitions for ioctls */
#define	DISKSIZE	1
#define	LASTERR		2
#define	SPINUP		3
#define	SPINDOWN	4
#define	RVDRIVES	5
#define	STATS		6
#define	VERSION		7
#define	ISTATS		8	/* internal statistics */
#define	SET_DEBUG	9
#define	RVDRECORD	10
#define	DOSRECORD	11
#define	ERRRECORD	12
#define ARPRECORD	12	/* allows old code to compile */
#define RVDCOPY		13	/* fast disk-to-disk copy */
#define RVDCOMP		14	/* compare two RVD packs.  */

#define ERRBUFFSIZE	1000	/* size of area set aside for error msgs */

/* disk modes
*/
#define	READONLY	1
#define	SHARED		2
#define	EXCLUSIVE	4

/* Version Number */
#define	RVD_IMP_VERSION	32

#define	NUMRVDERRS	(022 + RVD_ERR_OFFSET)
#define	RVD_ERR_OFFSET	3
#define	NUM_DRIVES	10	/* maximum number of drives */

#define	MAXRECORD	512
#define	RECORDMASK	511

/* structures used in communication between rvd and the user
*/

struct sizereq {
	long	*s_len;
	unsigned s_drive;
	};

struct copyreq {
    	unsigned c_drive1;
	unsigned c_drive2;
	long *c_error;
        };

struct upreq {
	unsigned u_drive;
	char	u_pack[32];
	char	u_cap[32];
	long	u_host;
	unsigned u_mode;
	};

struct rvdrive {
	char	s_pack[33];
	long	s_index;
	long	s_host;
	long	s_size;
	unsigned s_mode;
	};

struct stats {
	unsigned s_version;
	unsigned s_errors;
	unsigned s_chks;
	unsigned s_snd;
	unsigned s_rcv;
	unsigned s_tmo;
	unsigned s_badnonce;
	unsigned s_maxrreq;
	unsigned s_maxwreq;
	unsigned s_top;
	unsigned s_end;
	};

struct istats {
	unsigned s_nosend;
	unsigned s_freep;
	unsigned s_maxfreep;
	unsigned s_minfreep;
	unsigned s_cs;
	unsigned s_ds;
	};

struct dos_record {
	unsigned dr_index;
	unsigned dr_block[MAXRECORD];
	unsigned dr_count[MAXRECORD];
	};

struct rvd_record {
	unsigned rr_index;
	long	 rr_block[MAXRECORD];
	};
