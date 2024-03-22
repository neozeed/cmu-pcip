alt.obj: alt.asm
	masm /ml alt;
	lib ..\..\lib\h19-+alt;

curse.obj: curse.asm
	masm /ml curse;
	lib ..\..\lib\h19-+curse;
	lib ..\..\lib\sup-+curse;

em.obj: em.asm
	masm /ml em;
	lib ..\..\lib\h19-+em;

kh.obj: kh.asm
	masm /ml kh;
	lib ..\..\lib\h19-+kh;
	lib ..\..\lib\sup-+kh;

sh.obj: sh.asm
	masm /ml sh;
	lib ..\..\lib\h19-+sh;
	lib ..\..\lib\sup-+sh;

h19.obj: h19.c
	cl /c /DDEBUG /DMSC /Gs /I..\..\include /Zd /Zl h19.c
	lib ..\..\lib\h19-+h19;

h19key.obj: h19key.c
	cl /c /DDEBUG /DMSC /Gs /I..\..\include /Zd /Zl h19key.c
	lib ..\..\lib\h19-+h19key;

h19line.obj: h19line.c
	cl /c /DDEBUG /DMSC /Gs /I..\..\include /Zd /Zl h19line.c
	lib ..\..\lib\h19-+h19line;

_wbyte.obj: _wbyte.c
	cl /c /DDEBUG /DMSC /Gs /I..\..\include /Zd /Zl _wbyte.c
	lib ..\..\lib\h19-+_wbyte;

_wbyte_s.obj: _wbyte_s.c
	cl /c /DDEBUG /DMSC /Gs /I..\..\include /Zd /Zl _wbyte_s.c
	lib ..\..\lib\sup-+_wbyte_s;

scr_clos.obj: scr_clos.c
	cl /c /DDEBUG /DMSC /Gs /I..\..\include /Zd /Zl scr_clos.c
	lib ..\..\lib\h19-+scr_clos;
	lib ..\..\lib\sup-+scr_clos;

stdne.obj: stdne.c
	cl /c /DDEBUG /DMSC /Gs /I..\..\include /Zd /Zl stdne.c
	lib ..\..\lib\h19-+stdne;

stinit.obj: stinit.c
	cl /c /DDEBUG /DMSC /Gs /I..\..\include /Zd /Zl stinit.c
	lib ..\..\lib\h19-+stinit;

supdup.obj: supdup.c
	cl /c /DDEBUG /DMSC /Gs /I..\..\include /Zd /Zl supdup.c
	lib ..\..\lib\sup-+supdup;
