term.obj: term.c
	cl /c /DDEBUG /DMSC /Gs /I..\..\include /Zd term.c

ah.obj: ah.asm
	masm /ml ah;

ah2.obj: ah2.asm
	masm /ml ah2;

bell.obj: bell.asm
	masm /ml bell;

term.exe: term.obj ah.obj ah2.obj bell.obj
	link term ah ah2 bell,term/noi/noe,nul, h19 pc net;
	command /c copy term.exe ..\..\exe
