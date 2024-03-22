INC = ../../include/
HFILES = pkt.h et_pkt.h etc-note.h

CFLAGS = -u -DMSC -Oat -Gs -G0 -Zd -Zl -Zp1 -I$(INC)

WFLAG = -DWATCH -DLPKT

AFLAGS = /Mx /Z

enet.obj:	enet.c $(HFILES)
	cl -c $(CFLAGS) enet.c
	lib ..\..\lib\pkt4d-+enet;
	lib ..\..\lib\wpkt4d-+enet;

et_arp.obj:	et_arp.c $(HFILES)
	cl -c $(CFLAGS) et_arp.c
	lib ..\..\lib\pkt4d-+et_arp;
	lib ..\..\lib\wpkt4d-+et_arp;

et_close.obj:	et_close.c $(HFILES)
	cl -c $(CFLAGS) et_close.c
	lib ..\..\lib\pkt4d-+et_close;
	lib ..\..\lib\wpkt4d-+et_close;

et_demux.obj:	et_demux.c $(HFILES)
	cl -c $(CFLAGS) et_demux.c
	lib ..\..\lib\pkt4d-+et_demux;
	cl -c -u -DMSC -DWATCH -Oat -Gs -G0 -Zd -Zl -Zp1 -I$(INC) et_demux.c
	lib ..\..\lib\wpkt4d-+et_demux;

et_init.obj:	et_init.c $(HFILES)
	cl -c $(CFLAGS) et_init.c
	lib ..\..\lib\pkt4d-+et_init;
	cl -c -u -DMSC -DWATCH -Oat -Gs -G0 -Zd -Zl -Zp1 -I$(INC) et_init.c
	lib ..\..\lib\wpkt4d-+et_init;

et_int.obj:	et_int.c $(HFILES)
	cl -c $(CFLAGS) et_int.c
	lib ..\..\lib\pkt4d-+et_int;
	cl -c -u -DMSC -DWATCH -Oat -Gs -G0 -Zd -Zl -Zp1 -I$(INC) et_int.c
	lib ..\..\lib\wpkt4d-+et_int;

et_pkt.obj:	et_pkt.asm
	masm et_pkt,,nul,nul $(AFLAGS)
	lib ..\..\lib\pkt4d-+et_pkt;
	lib ..\..\lib\wpkt4d-+et_pkt;

et_send.obj:	et_send.c $(HFILES)
	cl -c $(CFLAGS) et_send.c
	lib ..\..\lib\pkt4d-+et_send;
	lib ..\..\lib\wpkt4d-+et_send;

et_stat.obj:	et_stat.c $(HFILES)
	cl -c $(CFLAGS) et_stat.c
	lib ..\..\lib\pkt4d-+et_stat;
	cl -c -u -DMSC -DWATCH -Oat -Gs -G0 -Zd -Zl -Zp1 -I$(INC) et_stat.c
	lib ..\..\lib\wpkt4d-+et_stat;

#pdkill.exe:	pdkill.c pkt.h et_pkt.obj
#		cl pdkill.c et_pkt.obj

#pdstat.exe:	pdstat.c pkt.h et_pkt.obj
#		cl pdstat.c et_pkt.obj

#smash.exe:	smash.c pkt.h et_pkt.obj
#		cl smash.c et_pkt.obj
