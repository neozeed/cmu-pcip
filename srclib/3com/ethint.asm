; Copyright 1986 by Carnegie Mellon
; See permission and disclaimer notice in file "cmu-note.mac"
;  Copyright 1984 by the Massachusetts Institute of Technology  
;  See permission and disclaimer notice in file "notice.h"  

	INCLUDE ..\..\include\dos.mac

; ethint.asm

; this file contains two pieces of code. One, _et_patch(), patches the handler
; for the ethernet controller interrupt. The other, ethint() handles the
; interrupt and calls a (hopefully) C routine, et_int(), two actually do
; the grunge.

_TEXT	SEGMENT

	PUBLIC	_et_patch
	PUBLIC	_et_unpatch
	PUBLIC	_etdma
	EXTRN	_et_ihnd:NEAR

intr	=	020H		; base address of interrupt vector
ocwr	=	020H		; 8259A interrupt controller control register

_BSS	SEGMENT
old_off	DW	?	; a place to store the old contents of the interrupt
old_cs	DW	?	; vector used by the ethernet controller
intvec	DW	?	; the vector address
_BSS	ENDS

_DATA	SEGMENT
	EXTRN	_et_eoi:WORD
_DATA	ENDS

_et_patch:
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

	mov	WORD PTR [si],OFFSET _TEXT:ethint ; the interrupt handler
	mov	2[si],cs	; our code segment
	pop	ds		; new vector is now patched in
	ASSUME	DS:DGROUP

	mov	old_off,bx	; cobble away the old offset and code segment
	mov	old_cs,cx

	pop	si
	pop	bp
	ret

_et_unpatch:
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

ethint:
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

	call	_et_ihnd

	mov	dx,ocwr
;	mov	al,_et_eoi
	mov	ax,_et_eoi	;DDP
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


; This routine takes an address of a packet buffer as an argument. It sets up
; the DMA controller and page register to point at the beginning of this
; packet. It does not set the length register in the DMA controller or start
; the DMA. We use channel one.

_etdma:
	push	bp
	mov	bp,sp		; set up the frame pointer

	mov	bx,4[bp]	; get the address of the packet

	mov	ax,ds		; get the segment
	mov	cl,4
	rol	ax,cl

	mov	ch,al		; save the lower four bits = page number
	and	al,0f0H
	add	ax,bx
	adc	ch,0

;	jae	skip
;	inc	ch
;skip:

	mov	dx,2
	out	dx,al
	mov	al,ah
	out	dx,al

	mov	al,ch
	and	al,00fH
	mov	dx,083H
	out	dx,al		; output to the page register

	mov	ah,0

	pop	bp
	ret
_TEXT	ENDS
	END
