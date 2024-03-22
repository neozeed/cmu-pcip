mbnet.obj: mbnet.c
	cl /c /DDEBUG /DMSC /Gs /I..\..\include /Zd /Zl mbnet.c
	lib ..\..\lib\mb-+mbnet;

mb_close.obj: mb_close.c
	cl /c /DDEBUG /DMSC /Gs /I..\..\include /Zd /Zl mb_close.c
	lib ..\..\lib\mb-+mb_close;

mb_stat.obj: mb_stat.c
	cl /c /DDEBUG /DMSC /Gs /I..\..\include /Zd /Zl mb_stat.c
	lib ..\..\lib\mb-+mb_stat;

mb_demux.obj: mb_demux.c
	cl /c /DDEBUG /DMSC /Gs /I..\..\include /Zd /Zl mb_demux.c
	lib ..\..\lib\mb-+mb_demux;

mb_init.obj: mb_init.c
	cl /c /DDEBUG /DMSC /Gs /I..\..\include /Zd /Zl mb_init.c
	lib ..\..\lib\mb-+mb_init;

mb_int.obj: mb_int.c
	cl /c /DDEBUG /DMSC /Gs /I..\..\include /Zd /Zl mb_int.c
	lib ..\..\lib\mb-+mb_int;

mb_send.obj: mb_send.c
	cl /c /DDEBUG /DMSC /Gs /I..\..\include /Zd /Zl mb_send.c
	lib ..\..\lib\mb-+mb_send;

mbsnd.obj: mbsnd.asm
	masm /ml mbsnd;
	lib ..\..\lib\mb-+mbsnd;

mbint.obj: mbint.asm
	masm /ml mbint;
	lib ..\..\lib\mb-+mbint;
