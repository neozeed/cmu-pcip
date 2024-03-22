.286c
	TITLE	TRW Board Utilities

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

; $Revision:   1.0  $		$Date:   29 Feb 1988 20:20:56  $

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

DGROUP	GROUP	CONST, _BSS, _DATA, STACK

; The "prologue" macro builds an entry stack frame.
; The "enter" instruction is not used because the push/mov sequence
; is faster when no local space is being reserved

prologue	MACRO
		PUSH	BP
		MOV	BP,SP
		ENDM

; The "epilogue" macro destroys a stack frame built by "prologue".
; The "leave" instruction is not used because it's not found on 808x
; processors.

epilogue	MACRO
		MOV	SP,BP
		POP	BP
		RET
		ENDM

BRD_SEGBASE	equ	0D000H	; Segment where board memory is visible

_DATA	SEGMENT
	EXTRN	_cpu_is_286:BYTE	; Zero if cpu is 808x/8018x/NEC Vx0
					; One if cpu is 80286/80386
_DATA	ENDS

_TEXT	SEGMENT
	PUBLIC	_portin_b
	PUBLIC	_portin_w
	PUBLIC	_portout_w

	ASSUME	CS:_TEXT, DS:DGROUP, SS:DGROUP, ES:NOTHING

; portin_b(port, buffer, len) -- Blast data in from a byte port.
_portin_b	PROC	NEAR
	mov	bx,sp
	mov	ax,ds
	mov	es,ax
	push	di
	mov	cx,ss:6[bx]	; length
	mov	di,ss:4[bx]	; pointer
	mov	dx,ss:2[bx]	; port
	cld
	cmp	_cpu_is_286,0	; Are we on a 286 processor?
	je	short lpin	; Jump if not
	rep	insb
	pop	di
	ret

lpin:	in	al,dx
	stosb
	loop	lpin
	pop	di
	ret
_portin_b	ENDP

; portin_w(port, buffer, len) -- Blast data in from a word port.
_portin_w	PROC	NEAR
	mov	bx,sp
	mov	ax,ds
	mov	es,ax
	push	di
	mov	cx,ss:6[bx]	; length in words
	mov	di,ss:4[bx]	; pointer
	mov	dx,ss:2[bx]	; port
	cld
	cmp	_cpu_is_286,0	; Are we on a 286 processor?
	je	short lpinw	; Jump if not
	rep	insw
	pop	di
	ret

lpinw:	in	ax,dx
	stosw
	loop	lpinw
	pop	di
	ret
_portin_w	ENDP

; portout_w(port, buffer, len) - Blast data out to a word port
_portout_w	PROC	NEAR
	mov	bx,sp
	push	si
	mov	cx,ss:6[bx]	; length in words
	mov	si,ss:4[bx]	; pointer
	mov	dx,ss:2[bx]	; port
	cld
	cmp	_cpu_is_286,0	; Are we on a 286 processor?
	je	short lpoutw	; Jump if not
	rep	outsw
	pop	si
	ret

lpoutw:	lodsw
	out	dx,ax
	loop	lpoutw
	pop	si
	ret
_portout_w	ENDP

_TEXT	ENDS
	END
