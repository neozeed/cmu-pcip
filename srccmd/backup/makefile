DEFS=-DNONAMES -DDUMPTIME -DXBUF
DEFBLOCKING=20

CFLAGS= $(DEFS) -DDEFBLOCKING=$(DEFBLOCKING) -Dindex=strchr

OBJS=tar.obj create.obj extract.obj buffer.obj getoldopt.obj list.obj names.obj port.obj

tar.exe: $(OBJS)
	cc $(OBJS) \lib\ssetargv.obj -o tar -link compat

clean:
	del *.obj
	del *.exe

$(OBJS): tar.h port.h
