hostname.obj: hostname.c
	cl /c /DDEBUG /DMSC /Gs /I..\..\include /Zd hostname.c

3hostnam.exe: hostname.obj
	link hostname, 3hostnam/noi/noe, nul, domain udp ip net 3com task pc;
	command /c copy 3hostnam.exe ..\..\exe

phostnam.exe: hostname.obj
	link hostname, phostnam/noi/noe, nul, domain udp ip net pr task pc;
	command /c copy phostnam.exe ..\..\exe

shostnam.exe: hostname.obj
	link hostname, shostnam/noi/noe, nul, domain udp ip net serial task pc;
	command /c copy shostnam.exe ..\..\exe

ihostnam.exe: hostname.obj
	link hostname, ihostnam/noi/noe, nul, domain udp ip net il task pc;
	command /c copy ihostnam.exe ..\..\exe

hostnam.exe: hostname.obj
	link hostname, hostnam/noi/noe, nul, domain udp ip net pkt4d task pc;
	command /c copy hostnam.exe ..\..\exe
