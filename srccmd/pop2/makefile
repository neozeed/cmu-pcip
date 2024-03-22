pop2.obj: pop2.c ..\..\include\ip.h ..\..\include\timer.h \
..\..\include\custom.h
	cl /c /DDEBUG /DMSC /Gs /I..\..\include /Zd pop2.c

3pop2.exe: pop2.obj
	link pop2,3pop2/noi/noe,nul, domain tcp udp 3com ip net task pc;
	command /c copy 3pop2.exe ..\..\exe

ppop2.exe: pop2.obj
	link pop2,ppop2/noi/noe,nul, domain tcp udp pr ip net task pc;
	command /c copy ppop2.exe ..\..\exe

spop2.exe: pop2.obj
	link pop2,spop2/noi/noe,nul, domain tcp udp serial ip net task pc;
	command /c copy spop2.exe ..\..\exe

ipop2.exe: pop2.obj
	link pop2,ipop2/noi/noe,nul, domain tcp udp il ip net task pc;
	command /c copy ipop2.exe ..\..\exe

pop2.exe: pop2.obj
	link pop2,pop2/noi/noe,nul, domain tcp udp pkt4d ip net task pc;
	command /c copy pop2.exe ..\..\exe
