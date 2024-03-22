struct ncb {
	unsigned char ncb_command;
	unsigned char ncb_retcode;
	unsigned char ncb_lsn;
	unsigned char ncb_num;
	char far *ncb_buffer;
	unsigned short ncb_length;
	char ncb_callname[16];
	char ncb_name[16];
	unsigned char ncb_rto;
	unsigned char ncb_sto;
	int (far *ncb_post)();
	unsigned char ncb_lana_num;
	unsigned char ncb_cmd_cplt;
	char ncb_reserve[14];
};

#define NCB_NOWAIT 0x80
#define NCB_RESET 0x32
#define NCB_CANCEL 0x35
#define NCB_ADAPTER_STATUS 0x33
#define NCB_UNLINK 0x70
#define NCB_ADD_NAME 0x30
#define NCB_ADD_GROUP_NAME 0x36
#define NCB_DELETE_NAME 0x31
#define NCB_CALL 0x10
#define NCB_LISTEN 0x11
#define NCB_HANG_UP 0x12
#define NCB_SEND 0x14
#define NCB_CHAIN_SEND 0x17
#define NCB_RECEIVE 0x15
#define NCB_RECEIVE_ANY 0x16
#define NCB_SESSION_STATUS 0x34
#define NCB_SEND_DATAGRAM 0x20
#define NCB_SEND_BROADCAST_DATAGRAM 0x22
#define NCB_RECEIVE_DATAGRAM 0x21
#define NCB_RECEIVE_BROADCAST_DATAGRAM 0x23

struct name {
	char nm_name[16];
	unsigned char nm_num;
	unsigned char nm_status;
};

struct astatus {
	unsigned char as_id[6];
	unsigned char as_jumpers;
	unsigned char as_post;
	unsigned char as_major;
	unsigned char as_minor;
	unsigned short as_interval;
	unsigned short as_crcerr;
	unsigned short as_algerr;
	unsigned short as_colerr;
	unsigned short as_abterr;
	unsigned long as_tcount;
	unsigned long as_rcount;
	unsigned short as_retran;
	unsigned short as_xresrc;
	char as_res0[8];
	unsigned short as_ncbfree;
	unsigned short as_ncbmax;
	unsigned short as_ncbx;
	char as_res1[4];
	unsigned short as_sespend;
	unsigned short as_msp;
	unsigned short as_sesmax;
	unsigned short as_bufsize;
	unsigned short as_names;
	struct name as_name[16];
};
