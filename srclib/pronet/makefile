prnet.obj: prnet.c pronet.h
	cl /c /DDEBUG /DMSC /Gs /I..\..\include /Zd /Zl prnet.c
	lib ..\..\lib\pr-+prnet;
	lib ..\..\lib\wpr-+prnet;

pr_close.obj: pr_close.c pronet.h
	cl /c /DDEBUG /DMSC /Gs /I..\..\include /Zd /Zl pr_close.c
	lib ..\..\lib\pr-+pr_close;
	lib ..\..\lib\wpr-+pr_close;

pr_demux.obj: pr_demux.c pronet.h
	cl /c /DDEBUG /DMSC /Gs /I..\..\include /Zd /Zl pr_demux.c
	lib ..\..\lib\pr-+pr_demux;
	lib ..\..\lib\wpr-+pr_demux;

pr_init.obj: pr_init.c pronet.h
	cl /c /DDEBUG /DMSC /Gs /I..\..\include /Zd /Zl pr_init.c
	lib ..\..\lib\pr-+pr_init;
	lib ..\..\lib\wpr-+pr_init;

pr_int.obj: pr_int.c pronet.h
	cl /c /DDEBUG /DMSC /Gs /I..\..\include /Zd /Zl pr_int.c
	lib ..\..\lib\pr-+pr_int;

pr_send.obj: pr_send.c pronet.h
	cl /c /DDEBUG /DMSC /Gs /I..\..\include /Zd /Zl pr_send.c
	lib ..\..\lib\pr-+pr_send;
	lib ..\..\lib\wpr-+pr_send;

pr_stat.obj: pr_stat.c pronet.h
	cl /c /DDEBUG /DMSC /Gs /I..\..\include /Zd /Zl pr_stat.c
	lib ..\..\lib\pr-+pr_stat;
	lib ..\..\lib\wpr-+pr_stat;

pro_int.obj: pro_int.asm
	masm /ml pro_int;
	lib ..\..\lib\pr-+pro_int;
	lib ..\..\lib\wpr-+pro_int;

pr_wint.obj: pr_wint.c pronet.h
	cl /c /DDEBUG /DMSC /Gs /I..\..\include /Zd /Zl pr_wint.c
	lib ..\..\lib\wpr-+pr_wint;
