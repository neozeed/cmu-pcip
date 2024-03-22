anet.obj: anet.c ..\..\include\netbuf.h
	cl /c /DDEBUG /DMSC /Gs /I..\..\include /Zd /Zl anet.c
	lib ..\..\lib\at-+anet;

at_close.obj: at_close.c ..\..\include\netbuf.h
	cl /c /DDEBUG /DMSC /Gs /I..\..\include /Zd /Zl at_close.c
	lib ..\..\lib\at-+at_close;

at_demux.obj: at_demux.c ..\..\include\netbuf.h
	cl /c /DDEBUG /DMSC /Gs /I..\..\include /Zd /Zl at_demux.c
	lib ..\..\lib\at-+at_demux;

at_init.obj: at_init.c ..\..\include\netbuf.h
	cl /c /DDEBUG /DMSC /Gs /I..\..\include /Zd /Zl at_init.c
	lib ..\..\lib\at-+at_init;

at_send.obj: at_send.c ..\..\include\netbuf.h
	cl /c /DDEBUG /DMSC /Gs /I..\..\include /Zd /Zl at_send.c
	lib ..\..\lib\at-+at_send;

atio.obj: atio.asm
	masm /ml /t atio;
	lib ..\..\lib\at-+atio;
