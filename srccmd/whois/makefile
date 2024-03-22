whois.obj: whois.c
	cl /c /DDEBUG /DMSC /Gs /I..\..\include /Zd whois.c

3whois.exe: whois.obj
	link whois, 3whois/noi/noe, nul, tcp domain udp 3com ip net task h19 pc;
	command /c copy 3whois.exe ..\..\exe

pwhois.exe: whois.obj
	link whois, pwhois/noi/noe, nul, tcp domain udp pr ip net task h19 pc;
	command /c copy pwhois.exe ..\..\exe

swhois.exe: whois.obj
	link whois, swhois/noi/noe, nul, tcp domain udp serial ip net task h19 pc;
	command /c copy swhois.exe ..\..\exe

iwhois.exe: whois.obj
	link whois, iwhois/noi/noe, nul, tcp domain udp il ip net task h19 pc;
	command /c copy iwhois.exe ..\..\exe

whois.exe: whois.obj
	link whois, whois/noi/noe, nul, tcp domain udp pkt4d ip net task h19 pc;
	command /c copy whois.exe ..\..\exe
