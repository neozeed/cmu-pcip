; Copyright 1988 Bradley N. Davis, Darbick Instructional Software Systems
; See permission and disclaimer notice in file "bnd-note.mac"
	INCLUDE bnd-note.mac
; Copyright 1986 by Carnegie Mellon
; See permission and disclaimer notice in file "cmu-note.mac"
	INCLUDE ..\..\include\cmu-note.mac
;  Copyright 1984 by the Massachusetts Institute of Technology  
;  See permission and disclaimer notice in file "notice.mac"  
	INCLUDE ..\..\include\notice.mac

	INCLUDE ..\..\include\dos.mac

; wint.asm

; this file contains two pieces of code. One, _w_patch(), patches the handler
; for the ethernet controller interrupt. The other, iint() handles the
; interrupt and calls a (hopefully) C routine, w_int(), to actually do
; the grunge.

_TEXT	SEGMENT

	PUBLIC	_w_patch
	PUBLIC	_w_unpatch
	EXTRN	_w_ihnd:NEAR

intr	=	020H		; base address of interrupt vector
ocwr	=	020H		; 8259A interrupt controller control register

_BSS	SEGMENT
old_off	DW	?	; a place to store the old contents of the interrupt
old_cs	DW	?	; vector used by the ethernet controller
intvec	DW	?	; the vector address
_BSS	ENDS

_DATA	SEGMENT
	EXTRN	_w_eoi:WORD
_DATA	ENDS

_w_patch:
	push	bp
	mov	bp,sp
	push	si

	mov	ax,4[bp]
	add	ax,intr
	mov	intvec,ax
	mov	si,ax

	push	ds
	xor	ax,ax
	mov	ds,ax
	ASSUME	DS:INTVECT
	mov	bx,[si]		; save old offset
	mov	cx,2[si]	; save old data segment

	mov	WORD PTR [si],OFFSET _TEXT:wint ; the interrupt handler
	mov	2[si],cs	; our code segment
	pop	ds		; new vector is now patched in
	ASSUME	DS:DGROUP

	mov	old_off,bx	; cobble away the old offset and code segment
	mov	old_cs,cx

	pop	si
	pop	bp
	ret

_w_unpatch:
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
wint:
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
;	mov	ax,cs
;	add	ax,#_data_seg
	mov	ax,SEG DGROUP
	mov	ds,ax
	mov	es,ax

	call	_w_ihnd

	mov	dx,ocwr
;	mov	al,_w_eoi
	mov	ax,_w_eoi	;DDP
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
