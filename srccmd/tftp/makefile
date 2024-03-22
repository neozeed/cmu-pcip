tftp_mai.obj: tftp_mai.c
	cl /c /DDEBUG /DMSC /Gs /I..\..\include /Zd tftp_mai.c

3tftp.exe: tftp_mai.obj
	link tftp_mai, 3tftp/noi/noe, nul, tftp domain udp 3com ip net task h19 pc;
	command /c copy 3tftp.exe ..\..\exe

ptftp.exe: tftp_mai.obj
	link tftp_mai, ptftp/noi/noe, nul, tftp domain udp pr ip net task h19 pc;
	command /c copy ptftp.exe ..\..\exe

stftp.exe: tftp_mai.obj
	link tftp_mai, stftp/noi/noe, nul, tftp domain udp serial ip net task h19 pc;
	command /c copy stftp.exe ..\..\exe

itftp.exe: tftp_mai.obj
	link tftp_mai, itftp/noi/noe, nul, tftp domain udp il ip net task h19 pc;
	command /c copy itftp.exe ..\..\exe

tftp.exe: tftp_mai.obj
	link tftp_mai, tftp/noi/noe, nul, tftp domain udp pkt4d ip net task h19 pc;
	command /c copy tftp.exe ..\..\exe
