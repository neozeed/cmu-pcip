; Copyright 1986 by Carnegie Mellon
; See permission and disclaimer notice in file "cmu-note.mac"
	INCLUDE ..\..\include\dos.mac

; pt_match(pattern, data)
;	returns FALSE if data doesn't match pattern.

match_data_len	=	60	; also in <watch.h>; must stay in sync

_TEXT	SEGMENT

	PUBLIC	_pt_match
_pt_match:
	push	bp
	mov	bp,sp
	push	si
	push	di

	mov	si,4[bp]	; si is pointer into pattern
	mov	di,6[bp]	; di is pointer into data
	mov	cx,match_data_len

lp:
	lodsw			; fetch a word into ax
	mov	bl,[di]		; get some data into bl
	inc	di

	and	bl,ah		; mask the data
	cmp	bl,al		; if bl&mask != al then
	jz	okay

	mov	ax,0		;	report failure
	jmp	return

okay:
	loop	lp		; go back and do it again

	mov	ax,-1		; report success

return:
	pop	di
	pop	si
	pop	bp
	ret
_TEXT	ENDS
	END
