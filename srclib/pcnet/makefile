pcnet.obj: pcnet.c ..\..\include\netbuf.h
	cl /c /DDEBUG /DMSC /Gs /I..\..\include /Zd /Zl pcnet.c
	lib ..\..\lib\pcnet-+pcnet;

pc_close.obj: pc_close.c ..\..\include\netbuf.h
	cl /c /DDEBUG /DMSC /Gs /I..\..\include /Zd /Zl pc_close.c
	lib ..\..\lib\pcnet-+pc_close;

pc_demux.obj: pc_demux.c ..\..\include\netbuf.h
	cl /c /DDEBUG /DMSC /Gs /I..\..\include /Zd /Zl pc_demux.c
	lib ..\..\lib\pcnet-+pc_demux;

pc_init.obj: pc_init.c ..\..\include\netbuf.h
	cl /c /DDEBUG /DMSC /Gs /I..\..\include /Zd /Zl pc_init.c
	lib ..\..\lib\pcnet-+pc_init;

pc_send.obj: pc_send.c ..\..\include\netbuf.h
	cl /c /DDEBUG /DMSC /Gs /I..\..\include /Zd /Zl pc_send.c
	lib ..\..\lib\pcnet-+pc_send;

pcnetio.obj: pcnetio.asm
	masm /ml /t pcnetio;
	lib ..\..\lib\pcnet-+pcnetio;
