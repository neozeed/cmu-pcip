; Copyright 1986 by Carnegie Mellon
; See permission and disclaimer notice in file "cmu-note.mac"
	TITLE	mkraw

	INCLUDE ..\..\include\dos.mac

;  Copyright 1984 by the Massachusetts Institute of Technology  
;  See permission and disclaimer notice in file "notice.h"  

_TEXT	SEGMENT

; Stupid dos 2.0...this is the last crock I'll write to sidestep its many
; and splendid "features."

	PUBLIC	_mkraw
_mkraw:
	push	bp
	mov	bp,sp
	push	si
	push	di

	mov	bx,4[bp]
	mov	ax,4400H
	int	21H

	mov	dh,0
	or	dl,020H
	mov	ax,4401H
	mov	bx,4[bp]
	int	21H

	pop	di
	pop	si
	pop	bp
	ret
_TEXT	ENDS
	END
