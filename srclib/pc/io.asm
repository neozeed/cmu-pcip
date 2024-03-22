; Copyright 1986 by Carnegie Mellon
; See permission and disclaimer notice in file "cmu-note.mac"
	TITLE	io

	INCLUDE ..\..\include\dos.mac

;  Copyright 1984 by the Massachusetts Institute of Technology  
;  See permission and disclaimer notice in file "notice.h"  


; file: io.a86			INPUT and OUTPUT functions for C

_TEXT	SEGMENT

; Input/Output instruction interface to 8086. (26Feb82)	(Wayne)
; 	added inw, outw, inw2, outw2	      ( 8Jan83) (Romkey)

; int outb(port, value) -- Output value to specific port.
	PUBLIC	_outb

_outb:
	push	bp		; entry sequence
	mov	bp,sp

	mov	dx,4[bp]	; get port into DX
	mov	ax,6[bp]	; argument pointer into AX
	out	dx,al		; perform output instruction

	pop	bp
	ret

; int outw(port, value) -- Output value to specific port.
	PUBLIC	_outw

_outw:
	push	bp		; entry sequence
	mov	bp,sp

	mov	dx,4[bp]	; get port into DX
	mov	ax,6[bp]	; argument pointer into AX
	out	dx,al		; perform output instruction

	inc	dx
	jmp	next1
next1:
	mov	al,ah
	out	dx,al

	pop	bp
	ret

; int outw2(port, value) -- Output value to specific port.
	PUBLIC	_outw2

_outw2:
	push	bp		; entry sequence
	mov	bp,sp

	mov	dx,4[bp]	; get port into DX
	mov	ax,6[bp]	; argument pointer into AX
	out	dx,al		; perform output instruction

	jmp	next2
next2:
	mov	al,ah
	out	dx,al

	pop	bp
	ret

; char inb(port) -- input one byte from port.

	PUBLIC	_inb

_inb:	
	push	bp		; entry sequence
	mov	bp,sp

	mov	dx,4[bp]	; get port into DX
	in	al,dx		; perform input instruction
	mov	ah,0		; Clear high order byte

	pop	bp
	ret

; unsigned inw(port) - input one word from a port

	PUBLIC	_inw

_inw:	
	push	bp		; entry sequence
	mov	bp,sp

	mov	dx,4[bp]	; get port into DX
	inc	dx
	in	al,dx		; perform input instruction
	mov	ah,al		; Set high order byte

	dec	dx
	in	al,dx		; get low order byte

	pop	bp
	ret

	PUBLIC	_inw2
_inw2:	push	bp
	mov	bp,sp

	xor	ax,ax

	mov	dx,4[bp]
	in	al,dx

	mov	cl,8
	sal	ax,cl

	in	al,dx

	pop	bp
	ret

; fastin(port, buffer, len) - quick multibyte input routine
	PUBLIC	_fastin
_fastin:
	push	bp
	mov	bp,sp
	push	di

	mov	cx,8[bp]	; length
	mov	di,6[bp]	; pointer
	mov	dx,4[bp]	; port

	cld

lpin:	in	al,dx
	stosb
	loop	lpin

	pop	di
	pop	bp
	ret

; fastout(port, buffer, len) - quick multibyte output routine
	PUBLIC	_fastout
_fastout:
	push	bp
	mov	bp,sp
	push	si

	mov	cx,8[bp]	; length
	mov	si,6[bp]	; pointer
	mov	dx,4[bp]	; port

	cld

lpout:	lodsb
	out	dx,al
	loop	lpout

	pop	si
	pop	bp
	ret
_TEXT	ENDS
	END
