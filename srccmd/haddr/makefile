haddr.obj: haddr.c 
	cl /c /DMSC /Gs /I..\..\include /Zd haddr.c

3haddr.exe: haddr.obj
	link haddr, 3haddr/noi/noe, nul, ip net 3com task pc;
	command /c copy 3haddr.exe ..\..\exe

ihaddr.exe: haddr.obj
	link haddr, ihaddr/noi/noe, nul, ip net il task pc;
	command /c copy ihaddr.exe ..\..\exe

haddr.exe: haddr.obj
	link haddr, haddr/noi/noe, nul, ip net pkt4d task pc;
	command /c copy haddr.exe ..\..\exe
