onhook.obj: onhook.asm
	masm onhook;

onhook.com: onhook.obj
	link onhook;
	exe2bin onhook.exe onhook.com
	command /c copy onhook.com ..\..\exe

offhook.obj: offhook.asm
	masm offhook;

offhook.com: offhook.obj
	link offhook;
	exe2bin offhook.exe offhook.com
	command /c copy offhook.com ..\..\exe
