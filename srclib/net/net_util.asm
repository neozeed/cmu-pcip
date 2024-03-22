; Copyright 1986 by Carnegie Mellon
; See permission and disclaimer notice in file "cmu-note.mac"
;  Copyright 1984 by the Massachusetts Institute of Technology  
;  See permission and disclaimer notice in file "notice.h"  

	INCLUDE	..\..\include\dos.mac

; handle breaks in network programs.
; This is necessary so that any network devices can shut themselves down
; and restore interrupt vectors.

_BSS	SEGMENT
brkip	DW	?
brkcs	DW	?
_BSS	ENDS

INTVECT	SEGMENT
	ORG	023H*4
brkoff	DW	?
brkseg	DW	?
INTVECT	ENDS

_TEXT	SEGMENT

	EXTRN	_exit:NEAR

	PUBLIC	_brk_init
_brk_init:
	push	ds
	xor	ax,ax
	mov	ds,ax

	ASSUME DS:INTVECT
	mov	bx,brkoff
	mov	cx,brkseg

	pop	ds
	ASSUME DS:DGROUP
	push	ds

	mov	brkip,bx
	mov	brkcs,cx

	mov	ds,ax
	ASSUME DS:INTVECT

	mov	ax,cs
	mov	bx,OFFSET _TEXT:brkhnd

	mov	brkoff,bx
	mov	brkseg,ax

	pop	ds
	ASSUME DS:DGROUP
	ret

	PUBLIC	_brk_c
_brk_c:
	push	ds

	mov	bx,brkip
	mov	cx,brkcs

	xor	ax,ax
	mov	ds,ax
	ASSUME DS:INTVECT

	mov	brkoff,bx
	mov	brkseg,cx

	pop	ds
	ASSUME DS:DGROUP
	ret

brkhnd:
	mov	bx,brkip
	mov	cx,brkcs

	xor	ax,ax
	mov	ds,ax
	ASSUME DS:INTVECT

	mov	brkoff,bx
	mov	brkseg,cx

;	mov	ax,cs
;	add	ax,#_data_seg
	mov	ax,SEG DGROUP
	mov	ds,ax
	ASSUME DS:DGROUP
	mov	es,ax

	jmp	_exit

;; bswap(int) swaps the bytes in an int...boo hiss...

	PUBLIC	_swab
	PUBLIC	_bswap

_swab:
_bswap:
	push	bp
	mov	bp,sp

	mov	ah,4[bp]	; Purposely reversing order of bytes
	mov	al,5[bp]

	pop	bp
	ret


;SRI-UNIX:~billw/cksum.a86 16-nov-84 0400 edit by William Westfield
; improved checksum algorithm (untested)
;  Copyright 1984 by the Massachusetts Institute of Technology  
;  See permission and disclaimer notice in file "notice.h"  


; _cksum(buf, len, 0) performs an Internet type checksum. buf points to where
; 	the checksumming should begin, len is the number of 16 bit words
;	to be summed. Returns the checksum. This is the Unix compatible
;       version

	PUBLIC	_cksum

_cksum:
	push	bp
	mov	bp,sp
	push	si

;	mov	bx,4[bp]		; get buf
;	mov	cx,6[bp]		; get len
;	xor	ax,ax			; initial value for xsum, clear carry
;
;lpchk:	add	ax,[bx]		; add value and carry	2 bytes, 18 cycles
;	adc	ax,0		;			3 bytes,  8 c
;	inc	bx		; bump pointer		1 byte,   2 c
;	inc	bx		;			1 byte,   2 c
;	loop	lpchk		; do it again		2 bytes, 17 c

;new (faster?) checksum algorithm by Bill Westfield
; The idea is to overlap the carry wrap around with the addition of the
; next word of data.  Thus we must be careful that pointer increments
; and looping instructions do not affect the carry flag...

	mov	si,4[bp]
	mov	cx,6[bp]
	xor	bx,bx		; initial value for xsum, clear carry

lpchk:	lodsw			;get next word		1 byte, 16 cycles
	adc	bx,ax		;overlap adding last CY	2 bytes, 3 c
	loop	lpchk		;next word		2 bytes, 17 c

	mov	ax,bx		; put where results have to go
	adc	ax,0		; add final carry bit.

contck:
	pop	si
	pop	bp
	ret				; go home

; byte swap a long

	PUBLIC	_lswap
	PUBLIC	_wswap
_lswap:
_wswap:
	push	bp
	mov	bp,sp

	mov	dh,4[bp]
	mov	dl,5[bp]
	mov	ah,6[bp]
	mov	al,7[bp]

	pop	bp
	ret
_TEXT	ENDS
	END
