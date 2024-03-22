custom.obj: custom.c menu.h ..\..\include\custom.h ..\..\include\attrib.h
	cl /c /DDEBUG /DMSC /Gs /I..\..\include /Zd custom.c

menu.obj: menu.c menu.h ..\..\include\custom.h ..\..\include\attrib.h
	cl /c /DDEBUG /DMSC /Gs /I..\..\include /Zd /Zl menu.c

menu_def.obj: menu_def.c menu.h ..\..\include\custom.h ..\..\include\attrib.h
	cl /c /DDEBUG /DMSC /Gs /I..\..\include /Zd /Zl menu_def.c

res.obj: res.c menu.h ..\..\include\custom.h ..\..\include\attrib.h
	cl /c /DDEBUG /DMSC /Gs /I..\..\include /Zd /Zl res.c

custom.exe: custom.obj menu.obj menu_def.obj res.obj
	link custom menu menu_def res,custom/noi/noe,nul,h19 net pc;
	command /c copy custom.exe ..\..\exe
