netname.obj: netname.c
	cl /c /DDEBUG /DMSC /Gs /I..\..\include /Zd netname.c

3netname.exe: netname.obj
	link netname, 3netname/noi/noe, nul, domain udp ip net 3com task pc;
	command /c copy 3netname.exe ..\..\exe

pnetname.exe: netname.obj
	link netname, pnetname/noi/noe, nul, domain udp ip net pr task pc;
	command /c copy pnetname.exe ..\..\exe

snetname.exe: netname.obj
	link netname, snetname/noi/noe, nul, domain udp ip net serial task pc;
	command /c copy snetname.exe ..\..\exe

inetname.exe: netname.obj
	link netname, inetname/noi/noe, nul, domain udp ip net il task pc;
	command /c copy inetname.exe ..\..\exe

netname.exe: netname.obj
	link netname, netname/noi/noe, nul, domain udp ip net pkt4d task pc;
	command /c copy netname.exe ..\..\exe
