; Copyright 1986 by Carnegie Mellon
; See permission and disclaimer notice in file "cmu-note.mac"
	TITLE	em

	INCLUDE	..\..\include\dos.mac

; Copyright 1984 by the Massachusetts Institute of Technology
; See permission and disclaimer notice in file "notice.h"

; em.a86
; assembly language stub routine which calls the terminal emulator
; efficiently

_DATA	SEGMENT
	EXTRN	_em_routine:WORD ; vertical cursor coordinate
_DATA	ENDS

_TEXT	SEGMENT
	PUBLIC	_em
_em:
	jmp	[_em_routine]
_TEXT	ENDS
	END
