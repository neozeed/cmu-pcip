ping.obj: ping.c ..\..\include\ip.h
	cl /c /DDEBUG /DMSC /Gs /I..\..\include /Zd ping.c

cmd.obj: cmd.c ..\..\include\ip.h
	cl /c /DDEBUG /DMSC /Gs /I..\..\include /Zd /Zl cmd.c

3ping.exe: ping.obj
	link ping cmd, 3ping/noi/noe, nul, domain udp 3com ip net task pc h19;
	command /c copy 3ping.exe ..\..\exe

pping.exe: ping.obj
	link ping cmd, pping/noi/noe, nul, domain udp pr ip net task pc h19;
	command /c copy pping.exe ..\..\exe

sping.exe: ping.obj
	link ping cmd, sping/noi/noe, nul, domain udp serial ip net task pc h19;
	command /c copy sping.exe ..\..\exe

iping.exe: ping.obj
	link ping cmd, iping/noi/noe, nul, domain udp il ip net task pc h19;
	command /c copy iping.exe ..\..\exe

ping.exe: ping.obj
	link ping cmd, ping/noi/noe, nul, domain udp pkt4d ip net task pc h19;
	command /c copy ping.exe ..\..\exe
