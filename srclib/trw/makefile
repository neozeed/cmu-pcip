# The -DRBD_DEBUG enables a consistency check on the RFD/RBD lists
# it may be removed for "production" code.
# The -DXLOOP turns on 82586 external loopback, this is useful for
# devlopment testing, but must be removed for network use.
# The -DTUNE_DEBUG flag checks that the configuration doesn't exceed
# board memory.  This ought to be used when testing a new configuration.
# The -DDEBUG flag turns on all kinds of print statements.

#CFLAGS = -u -DMSC -Oat -Gs -G0 -Zd -Zl -Zp1 -I$(INC)
CFLAGS = -u -DMSC -DRBD_DEBUG -Oat -Gs -G0 -Zd -Zl -Zp1 -I$(INC)
#CFLAGS = -u -DMSC -DRBD_DEBUG -DTUNE_DEBUG -DXLOOP -Oat -Gs -G0 -Zd -Zl -Zp1 -I$(INC)
#CFLAGS = -u -DMSC -Oat -Gs -G0 -Zd -Zl -Zp1 -I$(INC)
INC = ../../include/
HFILES = trw-note.h trw.h trw_type.h trw_conf.h trw_res.h

AFLAGS = /Mx /Z

t_arp.obj:	t_arp.c $(HFILES)
		cl -c $(CFLAGS) t_arp.c
		lib ..\..\lib\trw-+t_arp;
		lib ..\..\lib\wtrw-+t_arp;

t_close.obj:	t_close.c $(HFILES)
		cl -c $(CFLAGS) t_close.c
		lib ..\..\lib\trw-+t_close;
		lib ..\..\lib\wtrw-+t_close;

t_conf58.obj:	t_conf58.c $(HFILES)
		cl -c $(CFLAGS) t_conf586.c
		lib ..\..\lib\trw-+t_conf58;
		lib ..\..\lib\wtrw-+t_conf58;

t_data.obj:	t_data.c $(HFILES)
		cl -c $(CFLAGS) t_data.c
		lib ..\..\lib\trw-+t_data;
		lib ..\..\lib\wtrw-+t_data;

t_demux.obj:	t_demux.c $(HFILES)
		cl -c $(CFLAGS) t_demux.c
		lib ..\..\lib\trw-+t_demux;
		lib ..\..\lib\wtrw-+t_demux;

t_error.obj:	t_error.c $(HFILES)
		cl -c $(CFLAGS) t_error.c
		lib ..\..\lib\trw-+t_error;
		lib ..\..\lib\wtrw-+t_error;

t_init.obj:	t_init.c $(HFILES)
		cl -c $(CFLAGS) t_init.c
		lib ..\..\lib\trw-+t_init;

wt_init.obj:	t_init.c $(HFILES)
		cl -c $(CFLAGS) -DWATCH -Fowt_init.obj t_init.c
		lib ..\..\lib\wtrw-+wt_init;

t_int.obj:	t_int.asm
		masm t_int,,nul,nul $(AFLAGS)
		lib ..\..\lib\trw-+t_int;
		lib ..\..\lib\wtrw-+t_int;

t_inth.obj:	t_inth.c $(HFILES)
		cl -c $(CFLAGS) t_inth.c
		lib ..\..\lib\trw-+t_inth;

wt_inth.obj:	t_inth.c $(HFILES)
		cl -c $(CFLAGS) -DWATCH -Fowt_inth.obj t_inth.c
		lib ..\..\lib\wtrw-+wt_inth;

t_net.obj:	t_net.c $(HFILES)
		cl -c $(CFLAGS) t_net.c
		lib ..\..\lib\trw-+t_net;

wt_net.obj:	t_net.c $(HFILES)
		cl -c $(CFLAGS) -DWATCH -Fowt_net.obj t_net.c
		lib ..\..\lib\wtrw-+wt_net;

t_res.obj:	t_res.c $(HFILES)
		cl -c $(CFLAGS) t_res.c
		lib ..\..\lib\trw-+t_res;

wt_res.obj:	t_res.c $(HFILES)
		cl -c $(CFLAGS) -DWATCH -Fowt_res.obj t_res.c
		lib ..\..\lib\wtrw-+wt_res;

t_send.obj:	t_send.c $(HFILES)
		cl -c $(CFLAGS) t_send.c
		lib ..\..\lib\trw-+t_send;

wt_send.obj:	t_send.c $(HFILES)
		cl -c $(CFLAGS) -DWATCH -Fowt_send.obj t_send.c
		lib ..\..\lib\wtrw-+wt_send;

t_stat.obj:	t_stat.c $(HFILES)
		cl -c $(CFLAGS) t_stat.c
		lib ..\..\lib\trw-+t_stat;

wt_stat.obj:	t_stat.c $(HFILES)
		cl -c $(CFLAGS) -DWATCH -Fowt_stat.obj t_stat.c
		lib ..\..\lib\wtrw-+wt_stat;

t_util.obj:	t_util.asm
		masm t_util,,nul,nul $(AFLAGS)
		lib ..\..\lib\trw-+t_util;
		lib ..\..\lib\wtrw-+t_util;
