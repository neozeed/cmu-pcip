axnet.obj: axnet.c
	cl /c /DDEBUG /DMSC /Gs /I..\..\include /Zi /Zl axnet.c
	lib ..\..\lib\adax-+axnet;

ax_close.obj: ax_close.c
	cl /c /DDEBUG /DMSC /Gs /I..\..\include /Zi /Zl ax_close.c
	lib ..\..\lib\adax-+ax_close;

ax_init.obj: ax_init.c
	cl /c /DDEBUG /DMSC /Gs /I..\..\include /Zi /Zl ax_init.c
	lib ..\..\lib\adax-+ax_init;

ax_stat.obj: ax_stat.c
	cl /c /DDEBUG /DMSC /Gs /I..\..\include /Zi /Zl ax_stat.c
	lib ..\..\lib\adax-+ax_stat;

ax_demux.obj: ax_demux.c
	cl /c /DDEBUG /DMSC /Gs /I..\..\include /Zi /Zl ax_demux.c
	lib ..\..\lib\adax-+ax_demux;

axint.obj: axint.asm
	masm /ml axint;
	lib ..\..\lib\adax-+axint;

ax_int.obj: ax_int.c
	cl /c /DDEBUG /DMSC /Gs /I..\..\include /Zi /Zl ax_int.c
	lib ..\..\lib\adax-+ax_int;

ax_send.obj: ax_send.c
	cl /c /DDEBUG /DMSC /Gs /I..\..\include /Zi /Zl ax_send.c
	lib ..\..\lib\adax-+ax_send;
