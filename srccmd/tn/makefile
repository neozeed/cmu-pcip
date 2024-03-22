telnet.obj: telnet.c ..\..\include\tftp.h ..\..\include\em.h \
..\..\include\custom.h ..\..\include\ip.h
	cl /c /DDEBUG /DMSC /Gs /I..\..\include /Zd /Zl telnet.c

telnet2.obj: telnet2.c ..\..\include\em.h ..\..\include\custom.h \
..\..\include\ip.h
	cl /c /DDEBUG /DMSC /Gs /I..\..\include /Zd /Zl telnet2.c

time.obj: time.c ..\..\include\net.h ..\..\include\timer.h \
..\..\include\attrib.h
	cl /c /DDEBUG /DMSC /Gs /I..\..\include /Zd /Zl time.c

tn.obj: tn.c ..\..\include\custom.h ..\..\include\ip.h
	cl /c /DDEBUG /DMSC /Gs /I..\..\include /Zd /Gs tn.c

tnscreen.obj: tnscreen.c ..\..\include\ip.h ..\..\include\attrib.h
	cl /c /DDEBUG /DMSC /Gs /I..\..\include /Zd /Zl tnscreen.c

3tn.exe: telnet.obj telnet2.obj time.obj tn.obj tnscreen.obj
	link tn telnet telnet2 tnscreen time,3tn/noi/noe,nul,h19 tftp tcp domain udp ip net 3com task pc;
	exemod 3tn.exe /stack 0a00
	command /c copy 3tn.exe ..\..\exe

ptn.exe: telnet.obj telnet2.obj time.obj tn.obj tnscreen.obj
	link tn telnet telnet2 tnscreen time,ptn/noi/noe,nul,h19 tftp tcp domain udp ip net pr task pc;
	exemod ptn.exe /stack 0a00
	command /c copy ptn.exe ..\..\exe

stn.exe: telnet.obj telnet2.obj time.obj tn.obj tnscreen.obj
	link tn telnet telnet2 tnscreen time,stn/noi/noe,nul,h19 tftp tcp domain udp ip net serial task pc;
	exemod stn.exe /stack 0a00
	command /c copy stn.exe ..\..\exe

itn.exe: telnet.obj telnet2.obj time.obj tn.obj tnscreen.obj
	link tn telnet telnet2 tnscreen time,itn/noi/noe,nul,h19 tftp tcp domain udp ip net il task pc;
	exemod itn.exe /stack 0a00
	command /c copy itn.exe ..\..\exe

tn.exe: telnet.obj telnet2.obj time.obj tn.obj tnscreen.obj
	link tn telnet telnet2 tnscreen time,tn/noi/noe,nul,h19 tftp tcp domain udp ip net pkt4d task pc;
	exemod tn.exe /stack 0a00
	command /c copy tn.exe ..\..\exe
