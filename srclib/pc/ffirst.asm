; Copyright 1986 by Carnegie Mellon
; See permission and disclaimer notice in file "cmu-note.mac"
	TITLE	ffirst

	INCLUDE ..\..\include\dos.mac

; Copyright 1984 by the Massachusetts Institute of Technology
; See permission and disclaimer notice in file "notice.h"

_TEXT	SEGMENT

; wildcarding routines
	PUBLIC	__setdta
	PUBLIC	__ffirst
	PUBLIC	__fnext
	PUBLIC	__fnext2

__setdta:
	push	bp
	mov	bp,sp
	push	si
	push	di

	mov	dx,4[bp]
	mov	ah,01aH
	int	021H

	pop	di
	pop	si
	pop	bp
	ret

__ffirst:
	push	bp
	mov	bp,sp
	push	si
	push	di

	mov	dx,4[bp]
	mov	cx,6[bp]

	mov	ah,4eH
	int	021H

	pop	di
	pop	si
	pop	bp
	ret

__fnext:
	push	si
	push	di

	mov	ah,4fH
	int	021H

	pop	di
	pop	si
	ret
__fnext2:
	push	bp
	mov	bp,sp
	push	si
	push	di

	mov	dx,4[bp]
	mov	ah,04fh
	int	021h

	pop	di
	pop	si
	pop	bp
	ret
_TEXT	ENDS
	END
