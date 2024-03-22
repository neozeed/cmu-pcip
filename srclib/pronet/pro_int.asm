; Copyright 1986 by Carnegie Mellon
; See permission and disclaimer notice in file "cmu-note.mac"
;  Copyright 1984, 1985 by Proteon, Inc.

;  See permission and disclaimer notice in file "proteon-notice.h"

	INCLUDE ..\..\include\dos.mac

; print.asm

; this file contains two pieces of code. One, _pr_patch(), patches the handler
; for the prlni interrupt. The other, print() handles the
; interrupt and calls a (hopefully) C routine, pr_int(), to actually do
; the grunge.


_TEXT	SEGMENT

	PUBLIC	_pr_patch
	PUBLIC	_pr_unpatch
	EXTRN	_pr_ihnd:NEAR

_BSS	SEGMENT
old_off	DW	?	; a place to store the old contents of the interrupt
old_cs	DW	?	; vector used by the ethernet controller
	EXTRN	_pr_int_base:WORD	;DDP
_BSS	ENDS

_pr_patch:
	push	si

	mov	si,_pr_int_base

	push	ds
	xor	ax,ax
	mov	ds,ax
	ASSUME	DS:INTVECT

	mov	bx,[si]		; save old offset
	mov	cx,2[si]	; save old data segment

	mov	WORD PTR [si],OFFSET _TEXT:v2int ; the interrupt handler
	mov	2[si],cs	; our code segment
	pop	ds		; new vector is now patched in
	ASSUME	DS:DGROUP

	mov	old_off,bx	; cobble away the old offset and code segment
	mov	old_cs,cx

	pop	si
	ret

_pr_unpatch:
	push	si

	mov	si,_pr_int_base
	mov	bx,old_off	; patch back in the old interrupt vector
	mov	cx,old_cs

	push	ds
	xor	ax,ax
	mov	ds,ax
	ASSUME	DS:INTVECT

	cli
	mov	[si],bx
	mov	2[si],cx
	pop	ds
	ASSUME	DS:DGROUP

	sti

	pop	si
	ret

; This routine saves the registers and calls a C routine to handle the
; interrupt.

v2int:
	push	ds	; save registers
	push	es
	push	si
	push	di
	push	bp
	push	ax
	push	bx
	push	cx
	push	dx

	; set up our data segment
;	mov	ax,cs
;	add	ax,#_data_seg
	mov	ax,SEG DGROUP		;DDP
	mov	ds,ax
	mov	es,ax

	call	_pr_ihnd

	pop	dx
	pop	cx
	pop	bx
	pop	ax
	pop	bp
	pop	di
	pop	si
	pop	es
	pop	ds
	iret
_TEXT	ENDS
	END
