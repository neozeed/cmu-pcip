; Copyright 1986 by Carnegie Mellon
; See permission and disclaimer notice in file "cmu-note.mac"
;  Copyright 1984 by the Massachusetts Institute of Technology  
;  See permission and disclaimer notice in file "notice.h"  

	INCLUDE ..\..\include\dos.mac

; copy512(source, destination) copy rapidly 512 bytes.
; 1/5/84				<J. H. Saltzer>

_TEXT	SEGMENT

	PUBLIC	_copy512

_copy512:
	push	bp
	mov	bp,sp

	push	si
	push	di
	push	es

	mov	cx,ds		; set up destination segment
	mov	es,cx
	mov	cx,256		; 512 bytes is 256 words
	mov	si,4[bp]	; get the address of the source
	mov	di,6[bp]	; and the destination

	repnz movsw		; now do move

	pop	es
	pop	di
	pop	si
	pop	bp
	ret
_TEXT	ENDS
	END

