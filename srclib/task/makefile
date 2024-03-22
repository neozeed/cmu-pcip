timer.obj: timer.c
	cl /c /DDEBUG /DMSC /Gs /I..\..\include /Zd /Zl timer.c
	lib ..\..\lib\task-+timer;

task.obj: task.c
	cl /c /DDEBUG /DMSC /Gs /I..\..\include /Zd /Zl task.c
	lib ..\..\lib\task-+task;

stack.obj: stack.c
	cl /c /DDEBUG /DMSC /Gs /I..\..\include /Zd /Zl stack.c
	lib ..\..\lib\task-+stack;

tk_util.obj: tk_util.asm
	masm /ml tk_util;
	lib ..\..\lib\task-+tk_util;

