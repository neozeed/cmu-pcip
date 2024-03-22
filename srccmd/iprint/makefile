iprint.obj: iprint.c ..\..\include\custom.h
	cl /c /DDEBUG /DMSC /Gs /I..\..\include /Zd iprint.c

format.obj: format.c
	cl /c /DDEBUG /DMSC /Gs /I..\..\include /Zd /Zl format.c

3iprint.exe: iprint.obj format.obj
	link iprint format, 3iprint/noi/noe, nul, tcp domain udp 3com net ip task h19 pc;
	command /c copy 3iprint.exe ..\..\exe

piprint.exe: iprint.obj format.obj
	link iprint format, piprint/noi/noe, nul, tcp domain udp pr net ip task h19 pc;
	command /c copy piprint.exe ..\..\exe

siprint.exe: iprint.obj format.obj
	link iprint format, siprint/noi/noe, nul, tcp domain udp serial net ip task h19 pc;
	command /c copy siprint.exe ..\..\exe

iiprint.exe: iprint.obj format.obj
	link iprint format, iiprint/noi/noe, nul, tcp domain udp il net ip task h19 pc;
	command /c copy iiprint.exe ..\..\exe

iprint.exe: iprint.obj format.obj
	link iprint format, iprint/noi/noe, nul, tcp domain udp pkt4d net ip task h19 pc;
	command /c copy iprint.exe ..\..\exe
