
#/DDEBUG
CFLAGS	=	/Gs /I../../include /Zd /Zl /Od /Ze
#3d 4n 
TYPE	=	

wd8003.obj: wd8003.c ../../include/ether.h ../../include/netbuf.h wd8003.h
	cl /c $(CFLAGS) wd8003.c
	lib ..\..\lib\wd8003$(TYPE)-+wd8003;

wint.obj: wint.asm
	masm /ml wint;
	lib ..\..\lib\wd8003$(TYPE)-+wint;
	lib ..\..\lib\ww8003$(TYPE)-+wint;

wwd8003.obj: wd8003.c ../../include/ether.h ../../include/netbuf.h wd8003.h
	cl /c $(CFLAGS) /DWATCH /Fowwd8003.obj wd8003.c
	lib ..\..\lib\ww8003$(TYPE)-+wwd8003;
