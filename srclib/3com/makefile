enet.obj: enet.c ..\..\include\ether.h ..\..\include\netbuf.h 3com.h
	cl /c /DDEBUG /DMSC /Gs /I..\..\include /Zd /Zl enet.c
	lib ..\..\lib\3com-+enet;

et_arp.obj: et_arp.c ..\..\include\ether.h ..\..\include\netbuf.h 3com.h
	cl /c /DDEBUG /DMSC /Gs /I..\..\include /Zd /Zl /Ze et_arp.c
	lib ..\..\lib\3com-+et_arp;
	lib ..\..\lib\w3com-+et_arp;

et_close.obj: et_close.c ..\..\include\ether.h ..\..\include\netbuf.h 3com.h
	cl /c /DDEBUG /DMSC /Gs /I..\..\include /Zd /Zl et_close.c
	lib ..\..\lib\3com-+et_close;
	lib ..\..\lib\w3com-+et_close;

et_demux.obj: et_demux.c ..\..\include\ether.h ..\..\include\netbuf.h 3com.h
	cl /c /DDEBUG /DMSC /Gs /I..\..\include /Zd /Zl et_demux.c
	lib ..\..\lib\3com-+et_demux;
	lib ..\..\lib\w3com-+et_demux;

et_init.obj: et_init.c ..\..\include\ether.h ..\..\include\netbuf.h 3com.h
	cl /c /DDEBUG /DMSC /Gs /I..\..\include /Zd /Zl et_init.c
	lib ..\..\lib\3com-+et_init;
	lib ..\..\lib\w3com-+et_init;

et_int.obj: et_int.c ..\..\include\ether.h ..\..\include\netbuf.h 3com.h
	cl /c /DDEBUG /DMSC /Gs /I..\..\include /Zd /Zl et_int.c
	lib ..\..\lib\3com-+et_int;

et_send.obj: et_send.c ..\..\include\ether.h ..\..\include\netbuf.h 3com.h
	cl /c /DDEBUG /DMSC /Gs /I..\..\include /Zd /Zl et_send.c
	lib ..\..\lib\3com-+et_send;

et_stat.obj: et_stat.c ..\..\include\ether.h ..\..\include\netbuf.h 3com.h
	cl /c /DDEBUG /DMSC /Gs /I..\..\include /Zd /Zl et_stat.c
	lib ..\..\lib\3com-+et_stat;

ethint.obj: ethint.asm
	masm /ml ethint;
	lib ..\..\lib\3com-+ethint;
	lib ..\..\lib\w3com-+ethint;

wenet.obj: enet.c ..\..\include\ether.h ..\..\include\netbuf.h 3com.h
	cl /c /DDEBUG /DWATCH /DMSC /Gs /I..\..\include /Zd /Zl /Fowenet.obj enet.c
	lib ..\..\lib\w3com-+wenet;

wet_int.obj: et_int.c ..\..\include\ether.h ..\..\include\netbuf.h 3com.h ..\..\include\match.h
	cl /c /DDEBUG /DWATCH /DMSC /Gs /I..\..\include /Zd /Zl /Fowet_int.obj et_int.c
	lib ..\..\lib\w3com-+wet_int;

wet_send.obj: et_send.c ..\..\include\ether.h ..\..\include\netbuf.h 3com.h
	cl /c /DDEBUG /DWATCH /DMSC /Gs /I..\..\include /Zd /Zl /Fowet_send.obj et_send.c
	lib ..\..\lib\w3com-+wet_send;

wet_stat.obj: et_stat.c ..\..\include\ether.h ..\..\include\netbuf.h 3com.h
	cl /c /DDEBUG /DWATCH /DMSC /Gs /I..\..\include /Zd /Zl /Fowet_stat.obj et_stat.c
	lib ..\..\lib\w3com-+wet_stat;
