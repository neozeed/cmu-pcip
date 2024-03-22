res_name.obj: res_name.c ..\..\include\udp.h domain.h
	cl /c /DDEBUG /DMSC /Gs /I..\..\include /Zd /Zl res_name.c
	lib ..\..\lib\domain-+res_name;

dm_name.obj: dm_name.c ..\..\include\udp.h domain.h
	cl /c /DMSC /Gs /I..\..\include /Zd /Zl dm_name.c
	lib ..\..\lib\domain-+dm_name;

###########################################################################
#	DO NOT compile dm_name.c with DEBUG defined unless you
#	want some programs (telnet) to be too big for DOS to load.
#	if you need to debug dm_name.c, you'll have to change this
#	and use a small stub program to debug it.
###########################################################################
