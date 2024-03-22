; Copyright 1986 by Carnegie Mellon
; See permission and disclaimer notice in file "cmu-note.mac"
	TITLE	crock

	INCLUDE ..\..\include\dos.mac

;  Copyright 1984 by the Massachusetts Institute of Technology  
;  See permission and disclaimer notice in file "notice.h"  


; crock.a86 contains the necessary strangeness for the alarm functions et al.
; It has an interrupt handler and a slight interface to the real world
; (ie: the high level schtuff)

INTVECT	SEGMENT
	ORG	070H
int_clk	DW	?		; address of secondary clock interrupt handler
int_clk2 DW	?		; address of secondary clock interrupt handler
INTVECT	ENDS

SIGALRM	  = 14		; alarm clock signal

_DATA	SEGMENT
	EXTRN	__tcount:WORD	; crock count
	EXTRN	_cticks:DWORD	; crock ticks
	EXTRN	__alrm:WORD	; alarm function to call
_DATA	ENDS

_TEXT	SEGMENT

	PUBLIC	_crock_init	; crock initialization
	PUBLIC	_crock_c	; crock cleanup
_crock_init:
	cli
	push	ds
	mov	ax,0			; get interrupt vectors' data seg
	mov	ds,ax
	ASSUME	DS:INTVECT
	mov	bx,int_clk2		; get interrupt's cs
	mov	cx,int_clk

	mov	int_clk, OFFSET _TEXT:crock ; initialize appropriate vector
	mov	int_clk2,cs

	pop	ds
	ASSUME	DS:DGROUP
	mov	save_cs,bx
	mov	save_ip,cx

	mov	__tcount,0		; initialize count
	sti
	ret

_crock_c:
	mov	bx,save_cs
	cmp	bx,0
	jz	rtrn

	mov	cx,save_ip

	cli
	push	ds

	mov	ax,0
	mov	ds,ax
	ASSUME	DS:INTVECT

	mov	int_clk2,bx
	mov	int_clk,cx
	pop	ds
	ASSUME	DS:DGROUP

	sti
rtrn:	ret

_BSS	SEGMENT
save_ip	DW	?
save_cs	DW	?
_BSS	ENDS

crock:
	push	ds
	push	es
	push	ax

;	mov	ax,cs
;	add	ax,#_data_seg
	mov	ax,SEG DGROUP
	mov	ds,ax
	mov	es,ax
	pushf
	call	dword ptr save_ip

;	inc	_cticks
;	jae	Lcont1
;	inc	_cticks+2
	add	WORD PTR _cticks,1
	adc	WORD PTR _cticks+2,0

Lcont1:
	cmp	__tcount,0
	jz	done2

	dec	__tcount
	jnz	done2

	push	bp
	push	bx
	push	cx
	push	dx
	push	si
	push	di

	call	[__alrm]

	pop	di
	pop	si
	pop	dx
	pop	cx
	pop	bx
	pop	bp

done2:	pop	ax
	pop	es
	pop	ds
	iret
_TEXT	ENDS
	END
