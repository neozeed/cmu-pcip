; Copyright 1986 by Carnegie Mellon
; See permission and disclaimer notice in file "cmu-note.mac"
	TITLE	gencpy

	INCLUDE ..\..\include\dos.mac

;  Copyright 1984 by the Massachusetts Institute of Technology  
;  See permission and disclaimer notice in file "notice.h"  

_TEXT	SEGMENT

; gencpy(srcoff, srcseg, dstoff, dstseg, nbytes) copies.

	PUBLIC	_gencpy
_gencpy:
	push	bp
	mov	bp,sp

	push	si
	push	di
	push	es
	push	ds

	mov	cx,12[bp]	; get the length of the req block
	mov	si,4[bp]	; get the address of the source
	mov	di,8[bp]	; now all set
	mov	es,10[bp]	; destination segment
	mov	ds,6[bp]	; get the source segment

;lp:
;	movb	al,[si]
;	seg	es
;	movb	[di],al
;	inc	si
;	inc	di
;	loop	lp

	repnz	movsb

	pop	ds
	pop	es
	pop	di
	pop	si
	pop	bp
	ret
_TEXT	ENDS
	END
