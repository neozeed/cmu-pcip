.286c
; Copyright 1986 by Carnegie Mellon
; See permission and disclaimer notice in file "cmu-note.mac"
;  Copyright 1984 by the Massachusetts Institute of Technology  
;  See permission and disclaimer notice in file "notice.h"  

; /* Copyright 1988 by TRW Information Networks
;
; Permission to use, copy, modify, and distribute this program for any
; purpose and without fee is hereby granted, provided that this copyright
; and permission notice appear on all copies and supporting documentation,
; the name of TRW Information Networks not be used in advertising or
; publicity pertaining to distribution of the program without specific
; prior permission, and notice be given in supporting documentation that
; copying and distribution is by permission of TRW Information Networks.
; TRW Information Networks makes no representations about the suitability
; of this software for any purpose.  It is provided "as is" without
; express or implied warranty.  */

; trw_int.asm

; $Revision:   1.0  $		$Date:   29 Feb 1988 20:20:58  $

_TEXT	SEGMENT  BYTE   PUBLIC   'CODE'
_TEXT	ENDS
_DATA	SEGMENT  WORD   PUBLIC   'DATA'
_DATA	ENDS
CONST	SEGMENT  WORD   PUBLIC   'CONST'
CONST	ENDS
_BSS	SEGMENT  WORD   PUBLIC   'BSS'
_BSS	ENDS
STACK   SEGMENT  PARA   STACK   'STACK'
STACK	ENDS

INTVECT	SEGMENT AT 0
INTVECT	ENDS

DGROUP	GROUP	CONST, _BSS, _DATA, STACK

;eoi_int	MACRO
;	mov	dx,ocwr_1
;	mov	al,_trw_eoi_1
;	out	dx,al
;	ENDM

; The following macro produces the sequence necessary to end an interrupt
eoi_int	MACRO
	LOCAL	L_EOI
	mov	al,_trw_eoi_1
	out	ocwr_1,al
	xor	al,al
	add	al,_trw_eoi_2
	jz	short L_EOI
	out	ocwr_2,al
L_EOI:
	ENDM

DOSINT		equ	021H		; How to reach DOS
DOSGETVECTOR	equ	035H		; Get Interrupt vector
DOSSETVECTOR	equ	025H		; Set Interrupt vector

_BSS	SEGMENT
old_off	DW	?	; a place to store the old contents of the interrupt
old_cs	DW	?	; vector used by the ethernet controller
intvec	DW	?	; the vector address
_BSS	ENDS

_DATA	SEGMENT
	PUBLIC	_cpu_is_286
	EXTRN	_trw_eoi_1:BYTE
	EXTRN	_trw_eoi_2:BYTE

_cpu_is_286	DB	0	; Will be set to 1 if CPU is a 286
patched		DB	0	; Will be set to 1 if int vector is
				; patched in, zero otherwise
_DATA	ENDS

_TEXT	SEGMENT
	PUBLIC	_trw_patch
	PUBLIC	_trw_unpatch
	PUBLIC	_trw_eoi_int

	EXTRN	_trw_ihnd:NEAR

	ASSUME	CS:_TEXT, DS:DGROUP, SS:DGROUP, ES:NOTHING

ocwr_1	=	020H		; First 8259A interrupt controller
				; control register (OCW2)
ocwr_2	=	0A0H		; Second 8259A interrupt controller
				; control register (OCW2)

_trw_patch	PROC	NEAR
	cmp	patched,0		; Is vector patched-in?
	je	short make_patch	; If not, go patch it in.
	ret				; Otherwise just return

make_patch:
	push	bp
	mov	bp,sp

	; Lets see whether we're on a 286 or 386 processor...
	; Load dx with the offset of the int handler we want for the
	; particular type of processor.
	mov	dx,OFFSET _TEXT:ethint_8088 ; Assume 8088 handler
	push	sp	; 286 and 8088 don't push the same value
	pop	ax
	cmp	ax,sp
	jne	short L286	; Jump if we're not a 286 or 386
	mov	_cpu_is_286,01H
	mov	dx,OFFSET _TEXT:ethint_286 ; Use interrupt handler for 286
L286:
	push	si

	mov	ax,4[bp]
	mov	intvec,ax
	mov	si,ax

	push	ds
	ASSUME	DS:NOTHING
	xor	ax,ax
	mov	ds,ax
	ASSUME	DS:INTVECT
	mov	bx,[si]		; save old offset
	mov	cx,2[si]	; save old data segment

	mov	WORD PTR [si],dx ; The handler offset, as determined above
	mov	2[si],cs	; our code segment
	pop	ds		; new vector is now patched in
	ASSUME	DS:DGROUP

	mov	old_off,bx	; cobble away the old offset and code segment
	mov	old_cs,cx

	pop	si
	mov	patched,1
	pop	bp
	ret
_trw_patch	ENDP

_trw_unpatch	PROC	NEAR
	cmp	patched,1	; Is vector patched-in?
	je	short rm_patch	; If yes, go remove it
	ret			; Otherwise just return

rm_patch:
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
	mov	patched,0
	ret
_trw_unpatch	ENDP

; This routine saves the registers and calls a C routine to handle the
; interrupt.
; Two versions are provided, one for the 8088 and one for the 80286/80386

ethint_8088	PROC	NEAR
	ASSUME	CS:_TEXT, DS:NOTHING, SS:NOTHING, ES:NOTHING
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
;***	sti

	; set up our data segment
	mov	ax,SEG DGROUP
	mov	ds,ax
	mov	es,ax

; Where's the stack???

	ASSUME	CS:_TEXT, DS:DGROUP, SS:DGROUP, ES:NOTHING
	call	_trw_ihnd

	eoi_int

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
ethint_8088	ENDP

ethint_286	PROC	NEAR
	ASSUME	CS:_TEXT, DS:NOTHING, SS:NOTHING, ES:NOTHING
	push	ds	; save registers
	push	es
	pusha

	; turn on interrupts
;***	sti

	; set up our data segment
	mov	ax,SEG DGROUP
	mov	ds,ax
	mov	es,ax

; Where's the stack???

	ASSUME	CS:_TEXT, DS:DGROUP, SS:DGROUP, ES:NOTHING
	call	_trw_ihnd

	eoi_int

	popa
	pop	es
	pop	ds
	iret
ethint_286	ENDP

_trw_eoi_int	PROC	NEAR
		eoi_int
		ret
_trw_eoi_int	ENDP

_TEXT	ENDS
	END
