bootp.obj: bootp.c 
	cl /c /DDEBUG /DMSC /Gs /I..\..\include /Zd bootp.c

3bootp.exe: bootp.obj
	link bootp, 3bootp/noi/noe, nul, udp ip net 3com task pc;
	command /c copy 3bootp.exe ..\..\exe

ibootp.exe: bootp.obj
	link bootp, ibootp/noi/noe, nul, udp ip net il task pc;
	command /c copy ibootp.exe ..\..\exe

bootp.exe: bootp.obj
	link bootp, bootp/noi/noe, nul, udp ip net pkt4d task pc;
	command /c copy bootp.exe ..\..\exe

