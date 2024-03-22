proto.obj: proto.c
	cl /c /DDEBUG /DMSC /Gs /I..\..\include /Zd proto.c

red.obj: red.c
	cl /c /DMSC /Gs /I..\..\include /Zd red.c

3red.exe: red.obj proto.obj
	link red proto, 3red/noi/noe, nul, tcp domain udp 3com ip net task pc;
	command /c copy 3red.exe ..\..\exe

pred.exe: red.obj proto.obj
	link red proto, pred/noi/noe, nul, tcp domain udp pr ip net task pc;
	command /c copy pred.exe ..\..\exe

sred.exe: red.obj proto.obj
	link red proto, sred/noi/noe, nul, tcp domain udp serial ip net task pc;
	command /c copy sred.exe ..\..\exe

ired.exe: red.obj proto.obj
	link red proto, ired/noi/noe, nul, tcp domain udp il ip net task pc;
	command /c copy ired.exe ..\..\exe

red.exe: red.obj proto.obj
	link red proto, red/noi/noe, nul, tcp domain udp pkt4d ip net task pc;
	command /c copy red.exe ..\..\exe
