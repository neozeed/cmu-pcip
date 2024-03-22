#/DDEBUG
CFLAGS	=	/Gs /I../../include /Zd /Zl /Od /Ze /DDEBUG /DMI5210
#3d 4n 
TYPE	=	

i82586.obj: i82586.c ../../include/ether.h ../../include/netbuf.h i82586.h
	cl /c $(CFLAGS) i82586.c
	lib ..\..\lib\i82586$(TYPE)-+i82586;

i_arp.obj: i_arp.c ../../include/ether.h ../../include/netbuf.h i82586.h
	cl /c $(CFLAGS) i_arp.c
	lib ..\..\lib\i82586$(TYPE)-+i_arp;
	lib ..\..\lib\w82586$(TYPE)-+i_arp;

i_close.obj: i_close.c ../../include/ether.h ../../include/netbuf.h i82586.h
	cl /c $(CFLAGS) i_close.c
	lib ..\..\lib\i82586$(TYPE)-+i_close;
	lib ..\..\lib\w82586$(TYPE)-+i_close;

i_demux.obj: i_demux.c ../../include/ether.h ../../include/netbuf.h i82586.h
	cl /c $(CFLAGS) i_demux.c
	lib ..\..\lib\i82586$(TYPE)-+i_demux;
	lib ..\..\lib\w82586$(TYPE)-+i_demux;

i_init.obj: i_init.c ../../include/ether.h ../../include/netbuf.h i82586.h
	cl /c $(CFLAGS) i_init.c
	lib ..\..\lib\i82586$(TYPE)-+i_init;
	lib ..\..\lib\w82586$(TYPE)-+i_init;

i_int.obj: i_int.c ../../include/ether.h ../../include/netbuf.h i82586.h
	cl /c $(CFLAGS) i_int.c
	lib ..\..\lib\i82586$(TYPE)-+i_int;

i_send.obj: i_send.c ../../include/ether.h ../../include/netbuf.h i82586.h
	cl /c $(CFLAGS) i_send.c
	lib ..\..\lib\i82586$(TYPE)-+i_send;

i_stat.obj: i_stat.c ../../include/ether.h ../../include/netbuf.h i82586.h
	cl /c $(CFLAGS) i_stat.c
	lib ..\..\lib\i82586$(TYPE)-+i_stat;

iint.obj: iint.asm
	masm /ml iint;
	lib ..\..\lib\i82586$(TYPE)-+iint;
	lib ..\..\lib\w82586$(TYPE)-+iint;

w82586.obj: i82586.c ../../include/ether.h ../../include/netbuf.h i82586.h
	cl /c $(CFLAGS) /DWATCH /Fow82586.obj i82586.c
	lib ..\..\lib\w82586$(TYPE)-+w82586;

wi_int.obj: i_int.c ../../include/ether.h ../../include/netbuf.h i82586.h ../../include/match.h
	cl /c $(CFLAGS) /DWATCH /Fowi_int.obj i_int.c
	lib ..\..\lib\w82586$(TYPE)-+wi_int;

wi_send.obj: i_send.c ../../include/ether.h ../../include/netbuf.h i82586.h
	cl /c $(CFLAGS) /DWATCH /Fowi_send.obj i_send.c
	lib ..\..\lib\w82586$(TYPE)-+wi_send;

wi_stat.obj: i_stat.c ../../include/ether.h ../../include/netbuf.h i82586.h
	cl /c $(CFLAGS) /DWATCH /Fowi_stat.obj i_stat.c
	lib ..\..\lib\w82586$(TYPE)-+wi_stat;
