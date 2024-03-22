lpr.obj: lpr.c ..\..\include\custom.h
	cl /c /DDEBUG /DMSC /Gs /I..\..\include /Zd lpr.c

printpc.obj: printpc.c ..\..\include\custom.h
	cl /c /DDEBUG /DMSC /Gs /I..\..\include /Zd /Zl printpc.c

3lpr.exe: lpr.obj lpr.obj
	link lpr printpc, 3lpr/noi/noe, nul, tcp domain udp 3com net ip task h19 pc;
	command /c copy 3lpr.exe ..\..\exe

plpr.exe: lpr.obj lpr.obj
	link lpr printpc, plpr/noi/noe, nul, tcp domain udp pr net ip task h19 pc;
	command /c copy plpr.exe ..\..\exe

slpr.exe: lpr.obj lpr.obj
	link lpr printpc, slpr/noi/noe, nul, tcp domain udp serial net ip task h19 pc;
	command /c copy slpr.exe ..\..\exe

ilpr.exe: lpr.obj lpr.obj
	link lpr printpc, ilpr/noi/noe, nul, tcp domain udp il net ip task h19 pc;
	command /c copy ilpr.exe ..\..\exe

lpr.exe: lpr.obj lpr.obj
	link lpr printpc, lpr/noi/noe, nul, tcp domain udp pkt4d net ip task h19 pc;
	command /c copy lpr.exe ..\..\exe
