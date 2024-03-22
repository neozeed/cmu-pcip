parse.obj: parse.c
	cl /c /DDEBUG /DMSC /Gs /I..\..\include /Zd parse.c

smtp.obj: smtp.c ..\..\include\ip.h ..\..\include\timer.h \
..\..\include\custom.h cmds.h
	cl /c /DDEBUG /DMSC /Gs /I..\..\include /Zd smtp.c

3smtp.exe: smtp.obj smtp.obj
	link smtp parse,3smtp/noi/noe,nul, domain tcp udp 3com ip net task pc;
	command /c copy 3smtp.exe ..\..\exe

psmtp.exe: smtp.obj smtp.obj
	link smtp parse,psmtp/noi/noe,nul, domain tcp udp pr ip net task pc;
	command /c copy psmtp.exe ..\..\exe

ssmtp.exe: smtp.obj smtp.obj
	link smtp parse,ssmtp/noi/noe,nul, domain tcp udp serial ip net task pc;
	command /c copy ssmtp.exe ..\..\exe

ismtp.exe: smtp.obj smtp.obj
	link smtp parse,ismtp/noi/noe,nul, domain tcp udp il ip net task pc;
	command /c copy ismtp.exe ..\..\exe

smtp.exe: smtp.obj smtp.obj
	link smtp parse,smtp/noi/noe,nul, domain tcp udp pkt4d ip net task pc;
	command /c copy smtp.exe ..\..\exe
