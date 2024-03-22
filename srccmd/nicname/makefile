nicname.obj: nicname.c
	cl /c /DDEBUG /DMSC /Gs /I..\..\include /Zd nicname.c

3nicname.exe: nicname.obj
	link nicname.obj, 3nicname/noi/noe, nul, tcp domain udp 3com ip net task h19 pc;
	command /c copy 3nicname.exe ..\..\exe

pnicname.exe: nicname.obj
	link nicname.obj, pnicname/noi/noe, nul, tcp domain udp pr ip net task h19 pc;
	command /c copy pnicname.exe ..\..\exe

snicname.exe: nicname.obj
	link nicname.obj, snicname/noi/noe, nul, tcp domain udp serial ip net task h19 pc;
	command /c copy snicname.exe ..\..\exe

inicname.exe: nicname.obj
	link nicname.obj, inicname/noi/noe, nul, tcp domain udp il ip net task h19 pc;
	command /c copy inicname.exe ..\..\exe

nicname.exe: nicname.obj
	link nicname.obj, nicname/noi/noe, nul, tcp domain udp pkt4d ip net task h19 pc;
	command /c copy nicname.exe ..\..\exe
