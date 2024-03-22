#ifndef	ATALK
#define ATALK
#endif

#ifndef	CENTRAM
#define CENTRAM
#endif

#define nbp_status atd_status
#define nbp_compfun atd_compfun

#define atp_status atd_status
#define atp_compfun atd_compfun

#define lap_status atd_status
#define lap_compfun atd_compfun

#define ddp_status atd_status
#define ddp_compfun atd_compfun

#define tmr_status atd_status
#define tmr_compfun atd_compfun

#define inf_status atd_status
#define inf_compfun atd_compfun

#define zip_status atd_status
#define zip_compfun atd_compfun

#define pap_status atd_status
#define pap_compfun atd_compfun

#define asp_status atd_status
#define asp_compfun atd_compfun

#define asps_status atd_status
#define asps_compfun atd_compfun

#define aspw_status atd_status
#define aspw_compfun atd_compfun

typedef unsigned char ubyte;
typedef unsigned int uword;
typedef int word;
typedef unsigned long udword;
typedef long dword;
typedef int (far *FuncPtr)();
typedef char far *BuffPtr;

#define AsyncMask 0x8000

#define PapQuantum 0x200

#define EOMbit 0x10
#define XObit 0x20
#define STSbit 0x8
#define ChkSum 0x1
#define InfiniteRetry 0

#define ATInit 0x1
#define ATKill 0x2
#define ATGetNetInfo 0x3
#define ATGetClockTicks 0x4
#define ATStartTimer 0x5
#define ATResetTimer 0x6
#define ATCancelTimer 0x7

#define LAPInstall 0x10
#define LAPRemove 0x11
#define LAPWrite 0x12
#define LAPRead 0x13
#define LAPCancel 0x14

#define DDPOpenSocket 0x20
#define DDPCloseSocket 0x21
#define DDPWrite 0x22
#define DDPRead 0x23
#define DDPCancel 0x24

#define NBPRegister 0x30
#define NBPRemove 0x31
#define NBPLookup 0x32
#define NBPConfirm 0x33
#define NBPCancel 0x34

#define ZIPGetZoneList 0x35
#define ZIPGetMyZOne 0x36
#define ZIPTakedown 0x37
#define ZIPBringup 0x38

#define ATPOpenSocket 0x40
#define ATPCloseSocket 0x41
#define ATPSendRequest 0x42
#define ATPGetRequest 0x43
#define ATPSendResponse 0x44
#define ATPAddResponse 0x45
#define ATPCancelTrans 0x46
#define ATPCancelResponse 0x47
#define ATPCancelRequest 0x48

#define ASPGetParms 0x50
#define ASPCloseSession 0x51
#define ASPCancel 0x52
#define ASPInit 0x53
#define ASPKill 0x54
#define ASPGetSession 0x55
#define ASPGetRequest 0x56
#define ASPCmdReply 0x57
#define ASPWrtContinue 0x58
#define ASPWrtReply 0x59
#define ASPCloseReply 0x5a
#define ASPNewStatus 0x5b
#define ASPAttention 0x5c
#define ASPGetStatus 0x5d
#define ASPOpenSession 0x5e
#define ASPCommand 0x5f
#define ASPWrite 0x60
#define ASPGetAttention 0x61

#define PAPOpen 0x70
#define PAPClose 0x71
#define PAPRead 0x72
#define PAPWrite 0x73
#define PAPStatus 0x74
#define PAPRegName 0x75
#define PAPRemName 0x76
#define PAPInit 0x77
#define PAPNewStatus 0x78
#define PAPGetNextJob 0x79
#define PAPKill 0x7a
#define PAPCancel 0x7b

#define NoErr 0

#define MAXCOLISERR -1
#define MAXDEFERERR -2

#define LAP_LENERR -30
#define LAP_TYPERR -31
#define TABFULLERR -32
#define LAP_NOTFND -33
#define LAP_CANCELLED -34

#define DDP_SKTERR -40
#define DDP_LENERR -41
#define NOBRDGERR -42
#define DDP_CANCELLED -43

#define ATP_REQFAILED -100
#define ATP_NOSENDRESP -101
#define ATP_BADSOCKET -102
#define ATP_NORELEASE -103
#define ATP_OVERFLOW -104
#define ATP_CANCELLED -105

#define NO_MEM_ERROR -120
#define BAD_PARAMETER -121
#define STACKERROR -122
#define ATNOTINITIALIZED -123
#define CALLNOTSUPPORTED -124
#define HARDWARE_ERROR -125

#define NBP_NEWSOCKET -200
#define NBP_NOCONFIRM -201
#define NAME_IN_USE -202
#define NBP_NO_ROOM -203
#define BAD_NAME -204
#define NBP_NOTFOUND -205
#define NBP_CANCELLED -206
#define TMR_NOTFOUND -215
#define TMR_CANCELLED -216

#define PAP_BADCONNID -300
#define PAP_NOCONNIDS -301
#define PAP_DIED -302
#define PAP_LENERR -303
#define PAP_CANCELLED -304
#define PAP_WRITE_ACTIVE -310
#define PAP_READ_ACTIVE -311

typedef struct InfoParams {
	word atd_command;
	word atd_status;
	FuncPtr atd_compfun;
	uword inf_network;
	ubyte inf_nodeid;
	ubyte inf_abridge;
	uword inf_config;
	BuffPtr inf_buffptr;
	uword inf_buffsize;
} InfoParams;

/* bits in inf_config */
#define TimerMask 0x1
#define LAPMask 0x2
#define DDPMask 0x4
#define NBPMask 0x8
#define ATPMask 0x10
#define ZIPMask 0x20
#define PAPWksMask 0x40
#define PAPSrvrMask 0x80
#define ASPWksMask 0x100
#define ASPSrvrMask 0x200

typedef struct ATErrorStatus {
	uword bufflen;
	udword packets;
	udword crcerrs;
	udword overruns;
	udword deferrals;
	udword collisions;
#ifdef	CENTRAM
	udword timeouts;
	udword totalsent;
	udword goodrecptns;
	uword pktlenerrs;
	uword badlappkt;
	uword buffoverflow;
#else
	ubyte etcbuff[];
#endif
} ATErrorStatus;

typedef struct AddrBlk {
	uword network;
	ubyte nodeid;
	ubyte socket;
} AddrBlk;

typedef struct NBPTuple {
	AddrBlk ent_address;
	char ent_enum;
	char ent_name[99];
} NBPTuple;

typedef struct NBPTabEntry {
	BuffPtr tab_next;
	NBPTuple tab_tuple;
} NBPTabEntry;

typedef struct NBPParams {
	word atd_command;
	word atd_status;
	FuncPtr atd_compfun;
	AddrBlk nbp_addr;
	word nbp_toget;
	BuffPtr nbp_buffptr;
	uword nbp_buffsize;
	ubyte nbp_interval;
	ubyte nbp_retry;
	BuffPtr nbp_entptr;
} NBPParams;

typedef struct BDSElement {
	BuffPtr bds_buffptr;
	word bds_buffsize;
	word bds_datasize;
	char bds_usrbytes[4];
} BDSElement;

typedef struct ATPParams {
	word atd_command;
	word	atd_status;
	FuncPtr atd_compfun;
	AddrBlk atp_addrblk;
	ubyte atp_socket;
	ubyte atp_fill;
	BuffPtr atp_buffptr;
	word atp_buffsize;
	ubyte atp_interval;
	ubyte atp_retry;
	ubyte atp_flags;
	ubyte atp_seqbit;
	word atp_tranid;
	char atp_userbytes[4];
	ubyte atp_bdsbuffs;
	ubyte atp_bdsresps;
	BuffPtr atp_bdsptr;
} ATPParams;

typedef struct LAPParams {
	word atd_command;
	word atd_status;
	FuncPtr atd_compfun;
	word lap_fill1;
	ubyte lap_destnode;
	word lap_fill2;
	ubyte lap_type;
	BuffPtr lap_buffptr;
	word lap_buffsize;
} LAPParams;

typedef struct DDPParams {
	word atd_command;
	word atd_status;
	FuncPtr atd_compfun;
	AddrBlk ddp_addr;
	ubyte ddp_socket;
	ubyte ddp_type;
	BuffPtr ddp_buffptr;
	word ddp_buffsize;
	ubyte ddp_chksum;
} DDPParams;

typedef struct ZIPParams {
	word atd_command;
	word atd_status;
	FuncPtr atd_compfun;
	dword zip_fill;
	word zip_zones;
	BuffPtr zip_buffptr;
	word zip_buffsize;
} ZIPParams;

typedef struct ASPParams {
	word atd_command;
	word atd_status;
	FuncPtr atd_compfun;
	word asp_maxcmdsize;
	word asp_quantum;
} ASPParams;

typedef union ASPetc {
	AddrBlk asp_addr;
	uword asp_cmdresult;
	ubyte aspa_reqtype;
} ASPetc;

typedef struct ASPSrvrParams {
	word atd_command;
	word atd_status;
	FuncPtr atd_compfun;
	ASPetc asps_u;
	uword asps_sesrefnum;
	BuffPtr asps_buffptr;
	word asps_buffsize;
	uword asps_reqrefnum;
} ASPSrvrParams;

typedef struct ASPWksParams {
	word atd_command;
	word atd_status;
	FuncPtr atd_compfun;
	ASPetc aspw_u;
	uword aspw_sesrefnum;
	BuffPtr aspw_cmdblock;
	word aspw_cmdblocksize;
	BuffPtr aspw_replybuff;
	word aspw_replysize;
	word aspw_actreply;
	BuffPtr aspw_writebuff;
	word aspw_writesize;
	word aspw_actwritten;
} ASPWksParams;

typedef struct TimerParams {
	word atd_command;
	word atd_status;
	FuncPtr atd_compfun;
	dword tmr_ticks;
	uword tmr_time;
	BuffPtr tmr_params;
} TimerParams;

typedef struct PAPStatusRec {
	dword pap_system;
	char pap_status[256];
} PAPStatusRec;

typedef struct PAPParams {
	word atd_command;
	word atd_status;
	FuncPtr atd_compfun;
	AddrBlk pap_addr;
	word pap_refnum;
	BuffPtr pap_buffptr;
	word pap_buffsize;
	ubyte pap_eof;
	ubyte pap_srefnum;
	BuffPtr pap_entptr;
} PAPParams;

#define ipgpAssign (1L << 24)
#define ipgpName (2L << 24)
#define ipgpServer (3L << 24)
#define ipgpError -1L
struct IPGP {
	long opcode;
	long ipaddress;		/* me */
	long ipname;		/* name server */
	long ipbroad;		/* broadcast address */
	long ipfile;		/* file server */
	long ipother[4];
	char string[128];	/* error message */
};
