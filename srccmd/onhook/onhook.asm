; Copyright 1986 by Carnegie Mellon
; See permission and disclaimer notice in file "cmu-note.mac"
comment %  Copyright 1984 by the Massachusetts Institute of Technology  
	   See permission and disclaimer notice in file "notice.h"  

	Program to "hang up" the telephone.  Sets Asynchronous Communication
	adapter modem control register to zero, thus resetting the DTR line
	in the RS-232 modem interface.  Most modems interpret a zero on DTR
	to mean that they should go "on-hook".

	Usage:  (meant to be executed as a DOS command)

		hangup [port]

	where "port" if present is one of the strings "COM1:" or "COM2:".
	If "port" is absent, "COM1:" is used as a default.

	Program written 5/1/83 by J. H. Saltzer
	%

cseg	segment	para public 'code'
	org	0100h
hangup	proc	far		; Called from DOS command interpreter.
	assume	cs:cseg
	jmp	go		; Allows constants before reference.

onhook	db	00h		; Control byte to put modem onhook.
base	dw	03F0h		; Address of serial line adapter.
mcr	db	0Ch		; Offset of modem control register.

go:	push	ds		; Prepare for return to DOS
	sub	ax,ax		; by clearing accumulator
	push	ax		; and pushing the zeros on the stack.

	mov	dx,base		; Get base address of port
	or	dl,mcr		; Calculate address of modem control reg.
	mov	al,onhook	; Setup control byte for mcr.
	out	dx,al		; Stuff control byte into mcr.

	ret			; Return to DOS.

hangup	endp
cseg	ends
	end	hangup		; Identifies entry point for this program. 
