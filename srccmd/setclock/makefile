setclock.obj: setclock.c
	cl /c /DDEBUG /DMSC /Gs /I..\..\include /Zd setclock.c

set_pc_c.obj: set_pc_c.c
	cl /c /DDEBUG /DMSC /Gs /I..\..\include /Zd /Zl set_pc_c.c

3setcloc.exe: setclock.obj set_pc_c.obj
	link setclock set_pc_c, 3setcloc/noi/noe, nul, domain udp 3com ip net task pc;
	command /c copy 3setcloc.exe ..\..\exe

psetcloc.exe: setclock.obj set_pc_c.obj
	link setclock set_pc_c, psetcloc/noi/noe, nul, domain udp pr ip net task pc;
	command /c copy psetcloc.exe ..\..\exe

ssetcloc.exe: setclock.obj set_pc_c.obj
	link setclock set_pc_c, ssetcloc/noi/noe, nul, domain udp serial ip net task pc;
	command /c copy ssetcloc.exe ..\..\exe

isetcloc.exe: setclock.obj set_pc_c.obj
	link setclock set_pc_c, isetcloc/noi/noe, nul, domain udp il ip net task pc;
	command /c copy isetcloc.exe ..\..\exe

setcloc.exe: setclock.obj set_pc_c.obj
	link setclock set_pc_c, setclock/noi/noe, nul, domain udp pkt4d ip net task pc;
	command /c copy setclock.exe ..\..\exe

