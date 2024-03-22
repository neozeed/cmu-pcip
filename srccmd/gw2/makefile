nets.obj: nets.c ..\..\include\netbuf.h
	cl /c /DDEBUG /DMSC /Gs /I..\..\include nets.c

arp.obj: arp.c ..\..\include\ether.h ..\..\include\netbuf.h
	cl /c /DDEBUG /DMSC /Gs /I..\..\include arp.c

netinit.obj: netinit.c ..\..\include\custom.h ..\..\include\netbuf.h
	cl /c /DDEBUG /DMSC /Gs /I..\..\include netinit.c

indemux.obj: indemux.c ..\..\include\ip.h
	cl /c /DDEBUG /DMSC /Gs /I..\..\include indemux.c

gw.obj: gw.c ..\..\include\ip.h
	cl /c /DDEBUG /DMSC /Gs /I..\..\include gw.c

gwc.obj: gwc.c ..\..\include\ip.h
	cl /c /DDEBUG /DMSC /Gs /I..\..\include gwc.c

gw.exe: gw.obj gwc.obj netinit.obj nets.obj arp.obj indemux.obj
	link gw gwc netinit nets arp indemux,gw/noi/noe,nul,domain udp 3com slip ip net task pc;
