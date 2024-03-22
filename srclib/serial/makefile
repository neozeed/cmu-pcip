snet.obj: snet.c
	cl /c /DDEBUG /DMSC /Gs /I..\..\include /Zd /Zl snet.c
	lib ..\..\lib\serial-+snet;

sl_close.obj: sl_close.c
	cl /c /DDEBUG /DMSC /Gs /I..\..\include /Zd /Zl sl_close.c
	lib ..\..\lib\serial-+sl_close;

sl_stat.obj: sl_stat.c
	cl /c /DDEBUG /DMSC /Gs /I..\..\include /Zd /Zl sl_stat.c
	lib ..\..\lib\serial-+sl_stat;

sl_demux.obj: sl_demux.c
	cl /c /DDEBUG /DMSC /Gs /I..\..\include /Zd /Zl sl_demux.c
	lib ..\..\lib\serial-+sl_demux;

sl_init.obj: sl_init.c
	cl /c /DDEBUG /DMSC /Gs /I..\..\include /Zd /Zl sl_init.c
	lib ..\..\lib\serial-+sl_init;

sl_rcv.obj: sl_rcv.c
	cl /c /DDEBUG /DMSC /Gs /I..\..\include /Zd /Zl sl_rcv.c
	lib ..\..\lib\serial-+sl_rcv;

sl_snd.obj: sl_snd.c
	cl /c /DDEBUG /DMSC /Gs /I..\..\include /Zd /Zl sl_snd.c
	lib ..\..\lib\serial-+sl_snd;

slah.obj: slah.asm
	masm /ml slah;
	lib ..\..\lib\serial-+slah;
