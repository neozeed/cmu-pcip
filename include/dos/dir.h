/* Copyright 1986 by Carnegie Mellon */
struct dosdir {
	char	dd_rsvd[21];
	char	dd_attrib;
	unsigned dd_time;
	unsigned dd_date;
	long	dd_size;
	char	dd_file[13];
	};

/* file attributes */
#define	READONLY	0x01
#define	HIDDEN		0x02
#define	SYSTEM		0x04
#define	VOLUME		0x08
#define	DIRECTORY	0x10
#define	ARCHIVE		0x20

#define	NOTFOUND	2
#define	NOMOREFILES	18
