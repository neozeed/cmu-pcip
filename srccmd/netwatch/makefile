netwatch.obj: netwatch.c
	cl /c /DMSC /DLENGTH_HISTO /Gs /I..\..\include /Zd netwatch.c

ip.obj: ip.c
	cl /c /DDEBUG /DMSC /Gs /I..\..\include /Zd /Zl ip.c
	lib netwatch-+ip;

res.obj: res.c
	cl /c /DDEBUG /DMSC /Gs /I..\..\include /Zd /Zl res.c
	lib netwatch-+res;

netblt.obj: netblt.c
	cl /c /DDEBUG /DMSC /Gs /I..\..\include /Zd /Zl netblt.c
	lib netwatch-+netblt;

icmp.obj: icmp.c
	cl /c /DDEBUG /DMSC /Gs /I..\..\include /Zd /Zl icmp.c
	lib netwatch-+icmp;

udp.obj: udp.c
	cl /c /DDEBUG /DMSC /Gs /I..\..\include /Zd /Zl udp.c
	lib netwatch-+udp;

tcp.obj: tcp.c
	cl /c /DDEBUG /DMSC /Gs /I..\..\include /Zd /Zl tcp.c
	lib netwatch-+tcp;

rvd.obj: rvd.c
	cl /c /DDEBUG /DMSC /Gs /I..\..\include /Zd /Zl rvd.c
	lib netwatch-+rvd;

namebers.obj: namebers.c
	cl /c /DDEBUG /DMSC /Gs /I..\..\include /Zd /Zl namebers.c
	lib netwatch-+namebers;

chaos.obj: chaos.c
	cl /c /DDEBUG /DMSC /Gs /I..\..\include /Zd /Zl chaos.c
	lib netwatch-+chaos;

ether.obj: ether.c
	cl /c /DDEBUG /DMSC /Gs /I..\..\include /Zd /Zl ether.c
	lib enetwatc-+ether;

et_nameb.obj: et_nameb.c
	cl /c /DDEBUG /DMSC /Gs /I..\..\include /Zd /Zl et_nameb.c
	lib enetwatc-+et_nameb;

arp.obj: arp.c
	cl /c /DDEBUG /DMSC /Gs /I..\..\include /Zd /Zl arp.c
	lib enetwatc-+arp;

pronet.obj: pronet.c
	cl /c /DDEBUG /DMSC /Gs /I..\..\include /Zd /Zl pronet.c
	lib pnetwatc-+pronet;

pr_nameb.obj: pr_nameb.c
	cl /c /DDEBUG /DMSC /Gs /I..\..\include /Zd /Zl pr_nameb.c
	lib pnetwatc-+pr_nameb;

pr_arp.obj: pr_arp.c
	cl /c /DDEBUG /DMSC /Gs /I..\..\include /Zd /Zl /Zp pr_arp.c
	lib pnetwatc-+pr_arp;

display.obj: display.c
	cl /c /DMSC /DLENGTH_HISTO /Gs /I..\..\include /Zd /Zl display.c
	lib netwatch-+display;

pattern.obj: pattern.c
	cl /c /DDEBUG /DMSC /Gs /I..\..\include /Zd /Zl pattern.c
	lib netwatch-+pattern;

pt_match.obj: pt_match.asm
	masm /ml pt_match;
	lib netwatch-+pt_match;

screen.obj: screen.c
	cl /c /DDEBUG /DMSC /Gs /I..\..\include /Zd /Zl screen.c
	lib netwatch-+screen;

time.obj: time.c
	cl /c /DDEBUG /DMSC /Gs /I..\..\include /Zd /Zl time.c
	lib netwatch-+time;

histo.obj: histo.c
	cl /c /DMSC /DLENGTH_HISTO /Gs /I..\..\include /Zd /Zl histo.c
	lib netwatch-+histo;

3netwatc.exe: netwatch.obj netwatch.lib enetwatc.lib 
	link netwatch,3netwatc/noi/noe,nul,h19 netwatch enetwatc net w3com task pc;
	command /c copy 3netwatc.exe ..\..\exe

pnetwatc.exe: netwatch.obj netwatch.lib pnetwatc.lib 
	link netwatch/line,pnetwatc/noi/noe,nul,h19 netwatch pnetwatc net wpr task pc;
	command /c copy pnetwatc.exe ..\..\exe

inetwatc.exe: netwatch.obj netwatch.lib enetwatc.lib 
	link netwatch,inetwatc/noi/noe,nul,h19 netwatch enetwatc net wil task pc;
	command /c copy inetwatc.exe ..\..\exe

netwatch.exe: netwatch.obj netwatch.lib enetwatc.lib 
	link netwatch,netwatch/noi/noe,nul,h19 netwatch enetwatc net wpkt4d task pc;
	command /c copy netwatch.exe ..\..\exe
