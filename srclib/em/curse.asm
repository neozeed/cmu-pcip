; Copyright 1986 by Carnegie Mellon
; See permission and disclaimer notice in file "cmu-note.mac"
	TITLE	curse

	INCLUDE	..\..\include\dos.mac

;  Copyright 1984 by the Massachusetts Institute of Technology  
;  See permission and disclaimer notice in file "notice.h"  



; This function fixes it so that dos's cursor will be on the same line as
; the h19 emulator's was when we're all through.

_DATA	SEGMENT
	EXTRN	_y_pos:BYTE	; vertical cursor coordinate
_DATA	ENDS

_TEXT	SEGMENT
	PUBLIC	__curse
__curse:

	mov	dh,_y_pos	; frob it good
	mov	dl,0

	mov	bh,0		; set the active page number
	mov	ah,2		; now, actually set the position
	int	010H

	ret
_TEXT	ENDS
	END


