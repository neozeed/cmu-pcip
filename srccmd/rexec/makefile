rexec.obj: rexec.c
	cl /c /DDEBUG /DMSC /Gs /I..\..\include /Zd rexec.c

3rexec.exe: rexec.obj
	link rexec, 3rexec/noi/noe, nul, tcp domain udp 3com ip net task pc;
	command /c copy 3rexec.exe ..\..\exe

prexec.exe: rexec.obj
	link rexec, prexec/noi/noe, nul, tcp domain udp pr ip net task pc;
	command /c copy prexec.exe ..\..\exe

srexec.exe: rexec.obj
	link rexec, srexec/noi/noe, nul, tcp domain udp serial ip net task pc;
	command /c copy srexec.exe ..\..\exe

irexec.exe: rexec.obj
	link rexec, irexec/noi/noe, nul, tcp domain udp il ip net task pc;
	command /c copy irexec.exe ..\..\exe

rexec.exe: rexec.obj
	link rexec, rexec/noi/noe, nul, tcp domain udp pkt4d ip net task pc;
	command /c copy rexec.exe ..\..\exe
