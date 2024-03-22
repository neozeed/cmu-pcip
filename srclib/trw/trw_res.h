/* Copyright 1988 TRW Information Networks */
/*  See permission and disclaimer notice in file "trw-note.h"  */
#include	"trw-note.h"

/* $Revision:   1.0  $		$Date:   29 Feb 1988 20:21:12  $	*/

#if (!defined(trw_res))
#define trw_res 1

#pragma pack(1)

#if (!defined(trw_type))
#include "trw_type.h"
#endif

/* Define the structures needed to keep track of the CBs, each of	*/
/* which also has an associated TBD and transmit buffer.		*/
/* CB's may be in either the free pool or in the pending queue, both	*/
/* of which are linked using the cbp_next field.			*/
typedef struct cb_list_el_s
	{
	struct cb_list_el_s *cbp_next;	/* Next CB in the list, CBP_NONE*/
					/* if none.			*/
	uint	cb_link_offset;		/* Link offset to reach this CB.*/
	uint	cb_iomm_addr;		/* I/O port mapped memory addr.	*/
					/* where the CB is to be found.	*/
	uint	tbd_link_offset;	/* Link offset to reach this TBD*/
	uint	tbd_iomm_addr;		/* I/O port mapped memory addr.	*/
					/* where the TBD is to be found.*/
	uint	xbuff_iomm_addr;	/* I/O port mapped memory addr.	*/
					/* where the transmit buffer may*/
					/* be found.			*/
	paddr_t	xbuff_physaddr;		/* Physical address of the xmit	*/
					/* buffer.			*/
	} cb_list_el_t;

#define	CBP_NULL	(cb_list_el_t *)0

typedef struct
	{
	cb_list_el_t	 *first;
	cb_list_el_t	 *last;
	} cb_list_hdr_t;

extern cb_list_hdr_t	free_CB_list;	/* List header for the list of	*/
					/* available CBs, TBDs, and xmit*/
					/* buffers.			*/

extern cb_list_hdr_t	pending_CB_fifo;/* List header for the fifo of	*/
					/* CBs/TBDs/Xmit buffers waiting*/
					/* to be executed (Note, this	*/
					/* is not the same list as found*/
					/* in the 82586 itself.)	*/

extern cb_list_el_t *get_CB_from_list(cb_list_hdr_t *);
extern void append_CB_to_list(cb_list_hdr_t *, cb_list_el_t *);
extern void init_CB_lists(void);

#define get_free_CB()		get_CB_from_list(&free_CB_list)
#define get_next_pending_CB()	get_CB_from_list(&pending_CB_fifo)
#define release_CB(C)		append_CB_to_list(&free_CB_list,(C))
#define queue_CB(C)		append_CB_to_list(&pending_CB_fifo,(C))

#endif
