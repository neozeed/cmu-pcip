res.obj: res.c
	cl /c /DDEBUG /DMSC /Gs /I..\..\include /Zd /Zl res.c

monitor.obj: monitor.c
	cl /c /DDEBUG /DMSC /Gs /I..\..\include /Zd monitor.c

3monitor.exe: monitor.obj res.obj
	link monitor res, 3monitor/noi/noe, nul, domain udp ip net 3com task h19 pc;
	exemod 3monitor.exe /stack 1000
	command /c copy 3monitor.exe ..\..\exe

pmonitor.exe: monitor.obj res.obj
	link monitor res, pmonitor/noi/noe, nul, domain udp ip net pr task h19 pc;
	exemod pmonitor.exe /stack 1000
	command /c copy pmonitor.exe ..\..\exe

smonitor.exe: monitor.obj res.obj
	link monitor res, smonitor/noi/noe, nul, domain udp ip net serial task h19 pc;
	exemod smonitor.exe /stack 1000
	command /c copy smonitor.exe ..\..\exe

imonitor.exe: monitor.obj res.obj
	link monitor res, imonitor/noi/noe, nul, domain udp ip net il task h19 pc;
	exemod imonitor.exe /stack 1000
	command /c copy imonitor.exe ..\..\exe

monitor.exe: monitor.obj res.obj
	link monitor res, monitor/noi/noe, nul, domain udp ip net pkt4d task h19 pc;
	exemod monitor.exe /stack 1000
	command /c copy monitor.exe ..\..\exe
