/* Copyright 1988 TRW Information Networks */
/*  See permission and disclaimer notice in file "trw-note.h"  */
#include	"trw-note.h"

/* $Revision:   1.0  $		$Date:   29 Feb 1988 20:21:14  $	*/

#if (!defined(trw_type))
#define trw_type 1

#pragma pack(1)

typedef unsigned char	uchar;
typedef unsigned short	uint;

/* Structure to hold a 24 bit physical address */
typedef struct
	{
	uint	low_16;
	uchar	hi_8;
	uchar	unused;
	} paddr_t;

#define then

#endif
