; Copyright 1986 by Carnegie Mellon
; See permission and disclaimer notice in file "cmu-note.mac"

	INCLUDE ..\..\include\dos.mac

; mbint.asm

; this file contains two pieces of code. One, _mb_patch(), patches the handler
; for the MacBridge SCC interrupt. The other, mbint() handles the
; interrupt and calls a (hopefully) C routine, mb_int(), two actually do
; the grunge.

_TEXT	SEGMENT

	PUBLIC	_mb_patch
	PUBLIC	_mb_unpatch
	EXTRN	_mb_ihnd:NEAR

int	=	020H		; base address of interrupt vector
ocwr	=	020H		; 8259A interrupt controller control register

_BSS	SEGMENT
old_off	DW	?	; a place to store the old contents of the interrupt
old_cs	DW	?	; vector used by the ethernet controller
intvec	DW	?	; the vector address
_BSS	ENDS

_DATA	SEGMENT
	EXTRN	_mb_eoi:WORD
_DATA	ENDS

_mb_patch:
	push	bp
	mov	bp,sp
	push	si

	mov	ax,4[bp]
	add	ax,int
	mov	intvec,ax
	mov	si,ax

	push	ds
	xor	ax,ax
	mov	ds,ax
	ASSUME	DS:INTVECT
	mov	bx,[si]		; save old offset
	mov	cx,2[si]	; save old data segment

	mov	WORD PTR [si],OFFSET _TEXT:mbint ; the interrupt handler
	mov	2[si],cs	; our code segment
	pop	ds		; new vector is now patched in
	ASSUME	DS:DGROUP

	mov	old_off,bx	; cobble away the old offset and code segment
	mov	old_cs,cx

	pop	si
	pop	bp
	ret

_mb_unpatch:
	push	si

	mov	si,intvec
	mov	bx,old_off	; patch back in the old interrupt vector
	mov	cx,old_cs

	push	ds
	xor	ax,ax
	mov	ds,ax
	ASSUME	DS:INTVECT

	mov	[si],bx
	mov	2[si],cx
	pop	ds
	ASSUME	DS:DGROUP

	pop	si
	ret

; This routine saves the registers and calls a C routine to handle the
; interrupt.

mbint:
	push	ds	; save registers
	push	es
	push	si
	push	di
	push	bp
	push	ax
	push	bx
	push	cx
	push	dx


	; turn on interrupts
	sti

	; set up our data segment
	mov	ax,SEG DGROUP
	mov	ds,ax
	mov	es,ax

	call	_mb_ihnd

	mov	dx,ocwr
	mov	ax,_mb_eoi
	out	dx,al

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
