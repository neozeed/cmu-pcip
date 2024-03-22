; Copyright 1986 by Carnegie Mellon
; See permission and disclaimer notice in file "cmu-note.mac"
	TITLE	cursor_type

	INCLUDE ..\..\include\dos.mac

_TEXT	SEGMENT

	PUBLIC	__set_cursor_type
__set_cursor_type:
	push	bp
	mov	bp,sp
	push	si
	push	di	

	mov	ch,4[bp]
	mov	cl,6[bp]
	mov	ah,1

	int	010h

	pop	di
	pop	si
	pop	bp
	ret
_TEXT	ENDS
	END
