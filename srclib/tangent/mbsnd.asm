; Copyright 1986 by Carnegie Mellon
; See permission and disclaimer notice in file "cmu-note.mac"

	INCLUDE ..\..\include\dos.mac

; mbsnd.asm

; this file contains a time critical section of code the the MacBridge
; send routine.

_TEXT	SEGMENT

	PUBLIC	_mbsnd

; int mbsnd(port, c)
; int port;
; char c;
; returns:
;	0 - failure
;	1 - success
; assumes that port is the control port, and port+2 is the data port
	
_mbsnd:
	push	bp
	mov	bp,sp

	mov	ah,0			; assume failure
	cli				; turn off interrupts

	mov	dx,4[bp]		; get port number
	in	al,dx			; get rr0 status
	and	ax,040h			; check SCC_TxUNDER bit
	jne	abort			; uh oh, we lost it...

ok:	mov	al,6[bp]
	add	dx,2
	out	dx,al
	mov	ah,1			; success

abort:	sti				; turn on interrupts
	mov	al,ah			; get status

	pop	bp
	ret
_TEXT	ENDS
	END
