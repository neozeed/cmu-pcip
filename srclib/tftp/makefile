tftp_use.obj: tftp_use.c
	cl /c /DDEBUG /DMSC /Gs /I..\..\include /Zd /Zl tftp_use.c
	lib ..\..\lib\tftp-+tftp_use;

tftp_srv.obj: tftp_srv.c
	cl /c /DDEBUG /DMSC /Gs /I..\..\include /Zd /Zl tftp_srv.c
	lib ..\..\lib\tftp-+tftp_srv;

tftp_uti.obj: tftp_uti.c
	cl /c /DDEBUG /DMSC /Gs /I..\..\include /Zd /Zl tftp_uti.c
	lib ..\..\lib\tftp-+tftp_uti;

copy512.obj: copy512.asm
	masm /ml copy512;
	lib ..\..\lib\tftp-+copy512;
