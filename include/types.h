/* Copyright 1986 by Carnegie Mellon */
/*  Copyright 1983 by the Massachusetts Institute of Technology  */

/* This file includes useful typedefs for various kinds of data which should
	be portable between machines. This is, then, the machine dependent
	file. */

#ifndef TYPES_H				/* DDP */
#define TYPES_H	1			/* DDP */

typedef	char	byte;	/* 8 bits */
typedef int	word;	/* 16 bits */
typedef	long	lword;	/* 32 bits */
typedef	unsigned unshort;
typedef char *caddr_t;

#endif					/* DDP */
