netinit.obj: netinit.c ..\..\include\custom.h ..\..\include\netbuf.h
	cl /c /DDEBUG /DMSC /Gs /I..\..\include /Zd /Zl netinit.c
	lib ..\..\lib\net-+netinit;

netclose.obj: netclose.c ..\..\include\net.h
	cl /c /DDEBUG /DMSC /Gs /I..\..\include /Zd /Zl netclose.c
	lib ..\..\lib\net-+netclose;

netstats.obj: netstats.c ..\..\include\netbuf.h
	cl /c /DDEBUG /DMSC /Gs /I..\..\include /Zd /Zl netstats.c
	lib ..\..\lib\net-+netstats;

net_util.obj: net_util.asm
	masm /ml net_util;
	lib ..\..\lib\net-+net_util;

printf.obj: printf.c
	cl /c /DDEBUG /DMSC /Gs /I..\..\include /Zd /Zl printf.c
	lib ..\..\lib\net-+printf;

putc.obj: putc.c
	cl /c /DDEBUG /DMSC /Gs /I..\..\include /Zd /Zl putc.c
	lib ..\..\lib\net-+putc;

_wbyte.obj: _wbyte.c
	cl /c /DDEBUG /DMSC /Gs /I..\..\include /Zd /Zl _wbyte.c
	lib ..\..\lib\net-+_wbyte;
