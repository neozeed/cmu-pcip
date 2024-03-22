/* Copyright 1986 by Carnegie Mellon */
/*  Copyright 1986 by Carnegie Mellon  */
/*  See permission and disclaimer notice in file "cmu-note.h"  */
#include	<cmu-note.h>
/* netblt packet header definitions */

typedef struct {
    unsigned		hd_cksum;	/* checksum */
    byte		hd_version;	/* protocol version */
    byte		hd_type;	/* packet type */
    unsigned		hd_data_len;	/* length of packet data */
    unsigned		hd_lport;	/* local port number */
    unsigned		hd_fport;	/* foreign port number */
} NB_HDR;

#define NB_HDR_SIZE	sizeof(NB_HDR)


#define m_nb_hdr(ip_pkt) ((NB_HDR *) in_data(in_head(ip_pkt)))

/* OPEN packet definitions */

typedef struct {
    u_long		opn_uid;	    /* connection UID */
    u_long		opn_buf_size;	    /* buffer size in bytes */
    u_long		opn_data_len;	    /* transfer size */
    unsigned		opn_pkt_size;	    /* DATA packet size */
    unsigned		opn_burst_size;	    /* yes */
    unsigned		opn_burst_interval; /* yes */
    byte                opn_transfer_mode;  /* send or receive modes */
    byte		opn_max_nbufs;	    /* max number of buffers */
    byte		opn_cksum;	    /* checksum flag */
    byte		opn_pad_1;	    /* padding to get word align */
} OPN_HDR;

#define MAX_OPN_RETRY   5		    /* number of OPEN retries */

#define m_opn_hdr(ip_pkt) ((OPN_HDR *) ((in_data(in_head(ip_pkt))) + \
					NB_HDR_SIZE))
#define m_opn_data(ip_pkt) ((byte *) (((byte *) m_opn_hdr(ip_pkt)) + \
				      sizeof(OPN_HDR)))

/* RESPONSE packet definitions */

typedef OPN_HDR RSP_HDR;

#define MAX_RSP_RETRY   5

#define m_rsp_hdr(ip_pkt) ((RSP_HDR *) ((in_data(in_head(ip_pkt))) + \
			   NB_HDR_SIZE))
#define m_rsp_data(ip_pkt) ((byte *) ((byte *) (m_rsp_hdr(ip_pkt)) + \
				      sizeof(RSP_HDR)))

/* OK packet definition */

typedef struct {
    u_long		ok_bnum;
    unsigned		ok_burst_size;
    unsigned		ok_burst_interval;
} OK_HDR;

#define m_ok_hdr(ip_pkt) ((OK_HDR *) ((in_data(in_head(ip_pkt))) + \
				      NB_HDR_SIZE))

/*  RESEND packet definition */

typedef struct {
    u_long		rs_bnum;
    unsigned		rs_nmissing;
    unsigned		rs_padding;
} RS_HDR;

#define m_rs_hdr(ip_pkt) ((RS_HDR *) ((in_data(in_head(ip_pkt))) + \
				      NB_HDR_SIZE))
#define m_rs_dat(ip_pkt) ((byte *) ((byte *) (m_rs_hdr(ip_pkt)) + \
				    sizeof(RS_HDR)))

/* QUIT packet definition */

#define m_qt_dat(ip_pkt) ((byte *) (((byte *) m_nb_hdr(ip_pkt)) + NB_HDR_SIZE))

/* ABORT packet definition */

#define m_ab_dat(ip_pkt) ((byte *) (((byte *) m_nb_hdr(ip_pkt)) + NB_HDR_SIZE))

/* QUITACK packet definition */

#define MAX_CTL_RETRY   5

/* DATA packet definitions */

typedef struct {
    u_long		dat_buf_num;
    unsigned		dat_pkt_num;
    unsigned		dat_padding;
} DAT_HDR;

#define m_dat_hdr(ip_pkt) ((DAT_HDR *) ((in_data(in_head(ip_pkt))) + \
					NB_HDR_SIZE))
#define m_dat_data(ip_pkt) ((byte *) (((byte *) m_dat_hdr(ip_pkt) + \
				       sizeof(DAT_HDR))))

/* LDATA packet definitions */

typedef DAT_HDR LDAT_HDR;

#define m_ldat_hdr(ip_pkt) ((LDAT_HDR *) ((in_data(in_head(ip_pkt))) + \
					  NB_HDR_SIZE))
#define m_ldat_data(ip_pkt) ((byte *) (((byte *) m_ldat_hdr(ip_pkt) + \
					sizeof(LDAT_HDR))))

extern unsigned HDR_SIZES[];
