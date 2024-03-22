cookie.obj: cookie.c
	cl /c /DDEBUG /DMSC /Gs /I..\..\include /Zd cookie.c

3cookie.exe: cookie.obj
	link cookie,3cookie/noi/noe,nul,domain udp 3com ip net task pc;
	command /c copy 3cookie.exe ..\..\exe

pcookie.exe: cookie.obj
	link cookie,pcookie/noi/noe,nul,domain udp pr ip net task pc;
	command /c copy pcookie.exe ..\..\exe

scookie.exe: cookie.obj
	link cookie,scookie/noi/noe,nul,domain udp serial ip net task pc;
	command /c copy scookie.exe ..\..\exe

icookie.exe: cookie.obj
	link cookie,icookie/noi/noe,nul,domain udp il ip net task pc;
	command /c copy icookie.exe ..\..\exe

cookie.exe: cookie.obj
	link cookie,cookie/noi/noe,nul,domain udp pkt4d ip net task pc;
	command /c copy cookie.exe ..\..\exe
