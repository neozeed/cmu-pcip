il_net.obj: il_net.c ..\..\include\ether.h ..\..\include\netbuf.h interlan.h
	cl /c /DDEBUG /DMSC /Gs /I..\..\include /Zd /Zl il_net.c
	lib ..\..\lib\il-+il_net;

et_arp.obj: et_arp.c ..\..\include\ether.h ..\..\include\netbuf.h interlan.h
	cl /c /DDEBUG /DMSC /Gs /I..\..\include /Zd /Zl et_arp.c
	lib ..\..\lib\il-+et_arp;
	lib ..\..\lib\wil-+et_arp;

il_close.obj: il_close.c ..\..\include\ether.h ..\..\include\netbuf.h interlan.h
	cl /c /DDEBUG /DMSC /Gs /I..\..\include /Zd /Zl il_close.c
	lib ..\..\lib\il-+il_close;
	lib ..\..\lib\wil-+il_close;

il_demux.obj: il_demux.c ..\..\include\ether.h ..\..\include\netbuf.h interlan.h
	cl /c /DDEBUG /DMSC /Gs /I..\..\include /Zd /Zl il_demux.c
	lib ..\..\lib\il-+il_demux;
	lib ..\..\lib\wil-+il_demux;

il_init.obj: il_init.c ..\..\include\ether.h ..\..\include\netbuf.h interlan.h
	cl /c /DDEBUG /DMSC /Gs /I..\..\include /Zd /Zl il_init.c
	lib ..\..\lib\il-+il_init;
	lib ..\..\lib\wil-+il_init;

il_int.obj: il_int.c ..\..\include\ether.h ..\..\include\netbuf.h interlan.h
	cl /c /DDEBUG /DMSC /Gs /I..\..\include /Zd /Zl il_int.c
	lib ..\..\lib\il-+il_int;

il_send.obj: il_send.c ..\..\include\ether.h ..\..\include\netbuf.h interlan.h
	cl /c /DDEBUG /DMSC /Gs /I..\..\include /Zd /Zl il_send.c
	lib ..\..\lib\il-+il_send;
	lib ..\..\lib\wil-+il_send;

ether.obj: ether.c
	cl /c /DDEBUG /DMSC /Gs /I..\..\include /Zd /Zl ether.c
	lib ..\..\lib\il-+ether;
	lib ..\..\lib\wil-+ether;

ilint.obj: ilint.asm
	masm /ml ilint;
	lib ..\..\lib\il-+ilint;
	lib ..\..\lib\wil-+ilint;

il_wnet.obj: il_net.c ..\..\include\ether.h ..\..\include\netbuf.h interlan.h
	cl /c /DDEBUG /DWATCH /DMSC /Gs /I..\..\include /Zd /Zl /Foil_wnet.obj il_net.c
	lib ..\..\lib\wil-+il_wnet;

il_wint.obj: il_wint.c ..\..\include\ether.h ..\..\include\netbuf.h interlan.h ..\..\include\match.h
	cl /c /DDEBUG /DWATCH /DMSC /Gs /I..\..\include /Zd /Zl il_wint.c
	lib ..\..\lib\wil-+il_wint;
