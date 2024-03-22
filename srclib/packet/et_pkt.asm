.286c
	TITLE	Packet Driver Interface

; /* Copyright (c) 1988 by Epilogue Technology Corporation
;
; Permission to use, copy, modify, and distribute this program for any
; purpose and without fee is hereby granted, provided that this copyright
; and permission notice appear on all copies and supporting documentation,
; the name of Epilogue Technology Corporation not be used in advertising or
; publicity pertaining to distribution of the program without specific
; prior permission, and notice be given in supporting documentation that
; copying and distribution is by permission of Epilogue Technology
; Corporation.  Epilogue Technology Corporation makes no representations
; about the suitability of this software for any purpose.
; This software is provided "AS IS" without express or implied warranty.  */

; $Revision:   1.3  $		$Date:   02 Aug 1988 18:34:54  $	*/
; $Log:   E:/cmupcip/srclib/pkt/et_pkt.asv  $
;  
;     Rev 1.3   02 Aug 1988 18:34:54
;  Corrected bug in which the flags register was being improperly loaded
;  as the result of a failure to clear arguments off the stack after a
;  procedure call.
;  Bug reported by, and correction supplied by:
;     Brad Clements, Clarkson University, bkc@omnigate.clarkson.edu
;  
;     Rev 1.2   17 Jun 1988 11:33:26
;  Fixed error in which SS was being saved rather than DS.
;  
;     Rev 1.1   08 Mar 1988 12:17:06
;  Simply replaced a two instruction sequence with a one instruction
;  equivalent.
;  
;     Rev 1.0   04 Mar 1988 16:33:14
;  Initial revision.

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

DOSINT		equ	021H		; How to reach DOS
DOSGETVECTOR	equ	035H		; DOS GET Interrupt vector value

; Range in which the packet driver interrupt may lie
PKTDRVR_MIN_INT		equ	060H
PKTDRVR_MAX_INT		equ	07FH

PF_DRIVER_INFO		equ	1
PF_ACCESS_TYPE		equ	2
PF_RELEASE_TYPE		equ	3
PF_SEND_PKT		equ	4
PF_TERMINATE		equ	5
PF_GET_ADDRESS		equ	6
PF_RESET_INTERFACE	equ	7
PF_SET_RCV_MODE		equ	20
PF_GET_RCV_MODE		equ	21
PF_SET_MULTICAST_LIST	equ	22
PF_GET_MULTICAST_LIST	equ	23
PF_GET_STATISTICS	equ	24

PDE_NO_MULTICAST	equ	6
PDE_NO_PKT_DRVR		equ	128

; pkt_driver_info_t
PD_INFO		STRUC
version		dw	(?)
pdtype		dw	(?)
class		db	(?)
number		db	(?)
name_offset	dw	(?)
name_segment	dw	(?)
basic_flag	db	(?)
PD_INFO		ENDS

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

	ASSUME	CS:_TEXT, DS:DGROUP, SS:DGROUP, ES:NOTHING

_DATA	SEGMENT
	PUBLIC	_pkt_errno

_pkt_errno		DB	0
pkt_initialized		DB	0
pstring			DB	"PKT DRVR",0

STACK_FILL	EQU	0CC33H
ISTACK_SIZE	EQU	512		; Size in bytes
		DW	256 DUP (STACK_FILL)
END_ISTACK	EQU	THIS BYTE

_DATA	ENDS

_TEXT	SEGMENT
	PUBLIC	_pkt_access_type
	PUBLIC	_pkt_driver_info
	PUBLIC	_pkt_release_type
	PUBLIC	_pkt_send
	PUBLIC	_pkt_terminate
	PUBLIC	_pkt_get_address
	PUBLIC	_pkt_reset_interface
	PUBLIC	_pkt_set_rcv_mode
	PUBLIC	_pkt_get_rcv_mode
	PUBLIC	_pkt_get_statistics
	PUBLIC	_pkt_set_multicast_list
	PUBLIC	_pkt_get_multicast_list
	PUBLIC	_pkt_receive_helper

;*************************
; int test_vector(unsigned short) -- checks whether a given interrupt
; vector is the one used by an installed packet driver.
; Returns 1 if the vector is the one we want, 0 otherwise.
;*************************
test_vector	PROC	NEAR
	prologue
	push	si
	push	di
	mov	ah,DOSGETVECTOR
	mov	al,[bp+4]
	int	DOSINT
;	ES:BX now has the interrupt vector

	; Check whether the vector begins with a 3 byte jump instruction
	mov	al,BYTE PTR es:[bx]
	cmp	al,0E9H
	je	short is_possible
	; Also accept 2 byte form since assembler often optimizes to this
	cmp	al,0EBH
	jne	short isnt_possible
is_possible:
	mov	ax,1		; Set return, assuming we will find match

	; Look for string "PKT DRVR" (with null terminator)
	lea	di,[bx+3]	; Generate address where string would begin
	mov	si,OFFSET DGROUP: pstring
	mov	cx,9		; sizeof string, including null
	repe	cmpsb		; compare the strings
	je	short ret_result; jump if equal

isnt_possible:
	xor	ax,ax
ret_result:
	pop	di
	pop	si
	epilogue
test_vector	ENDP

;*************************
; do_int_xx -- invoke the packet driver interrupt
; The code for this routine is partially constructed by pkt_scan()
;*************************
do_int_xx	PROC	NEAR
	DB	0CDH			; INT op-code
int_num	DB	080H			; INT number
		ret
do_int_xx	ENDP

;*************************
; int pkt_scan() -- Scans vectors 0x60 through 0x7F looking for the 
; packet driver.
; Returns 1 if found, -0 if not.
; Records the vector number by constructing the appropriate INT instruction
; in do_int_xx and also marks that the
; scan has been done by setting pkt_initialized to 1 (if sucessful)
; or -1 if not.
;*************************
pkt_scan	PROC	NEAR
	prologue
	mov	bx,PKTDRVR_MIN_INT - 1
scan_next:
	inc	bx
	cmp	bx,PKTDRVR_MAX_INT
	jg	SHORT scan_fail
	push	bx		; save BX from destruction
	push	bx
	call	test_vector
	add	sp,2
	pop	bx
	or	ax,ax
	je	SHORT scan_next
	mov	int_num,bl
	mov	pkt_initialized,1
	mov	ax,1
	jmp	SHORT scan_done
scan_fail:
	mov	pkt_initialized,-1
	mov	_pkt_errno,PDE_NO_PKT_DRVR
	xor	ax,ax
scan_done:
	epilogue
pkt_scan	ENDP

;*************************
;*************************
_pkt_access_type	PROC NEAR
	prologue
	push	si
	push	di
	cmp	pkt_initialized,1
	je	SHORT is_init_1
	call	pkt_scan
	or	ax,ax
	jne	SHORT is_init_1
ret_error_1:
	mov	ax,-1
	pop	di
	pop	si
	epilogue

is_init_1:
	mov	ah,PF_ACCESS_TYPE
	mov	al,BYTE PTR [bp+4]	;if_class
	mov	bx,WORD PTR [bp+6]	;if_type
	mov	dl,BYTE PTR [bp+8]	;if_number
	mov	si,WORD PTR [bp+10]	;type
	mov	cx,WORD PTR [bp+12]	;typelen
	mov	di,cs
	mov	es,di
	mov	di,WORD PTR [bp+14]	;receiver
	call	do_int_xx
	jc	SHORT had_error_1
	pop	di
	pop	si
	epilogue

had_error_1:
	mov	_pkt_errno,dh
	jmp	SHORT ret_error_1
_pkt_access_type	ENDP

;*************************
;*************************
_pkt_driver_info	PROC	NEAR
	prologue
	push	si
	cmp	pkt_initialized,1
	je	SHORT is_init_2
	call	pkt_scan
	or	ax,ax
	jne	SHORT is_init_2
ret_error_2:
	mov	ax,-1
	pop	si
	epilogue

is_init_2:
	mov	ah,PF_DRIVER_INFO
	mov	bx,WORD PTR [bp+4]	;handle
	call	do_int_xx
	jc	SHORT had_error_2
	ASSUME	DS:nothing
	push	bx
	push	ds
	mov	bx,ss
	mov	ds,bx
	ASSUME	DS:DGROUP
	mov	bx,WORD PTR [bp+6]	;pkt_driver_info_t *
	mov	BYTE PTR [bx+class],ch
	mov	BYTE PTR [bx+number],cl
	mov	WORD PTR [bx+pdtype],dx
	mov	WORD PTR [bx+name_offset],si
	pop	cx			; Get DS as it was after the call
	mov	WORD PTR [bx+name_segment],cx
	mov	BYTE PTR [bx+basic_flag],al
	pop	cx			; Get BX as it was after the call
	mov	WORD PTR [bx+version],cx
	xor	ax,ax
	pop	si
	epilogue

had_error_2:
	ASSUME	DS:DGROUP
	mov	_pkt_errno,dh
	jmp	SHORT ret_error_2
_pkt_driver_info	ENDP


;*************************
;*************************
_pkt_release_type	PROC	NEAR
	prologue
	cmp	pkt_initialized,1
	je	SHORT is_init_3
	call	pkt_scan
	or	ax,ax
	jne	SHORT is_init_3
ret_error_3:
	mov	ax,-1
	epilogue

is_init_3:
	mov	ah,PF_RELEASE_TYPE
	mov	bx,WORD PTR [bp+4]	;handle
	call	do_int_xx
	jc	SHORT had_error_3
	xor	ax,ax
	epilogue

had_error_3:
	mov	_pkt_errno,dh
	jmp	SHORT ret_error_3
_pkt_release_type	ENDP

;*************************
;*************************
_pkt_send	PROC	NEAR
	prologue
	push	si
	cmp	pkt_initialized,1
	je	SHORT is_init_4
	call	pkt_scan
	or	ax,ax
	jne	SHORT is_init_4
ret_error_4:
	mov	ax,-1
	pop	si
	epilogue

is_init_4:
	mov	ah,PF_SEND_PKT
	mov	si,WORD PTR [bp+4]	;buffer (offset)
	mov	cx,WORD PTR [bp+6]	;length
	call	do_int_xx
	jc	SHORT had_error_4
	xor	ax,ax
	pop	si
	epilogue

had_error_4:
	mov	_pkt_errno,dh
	jmp	SHORT ret_error_4
_pkt_send	ENDP

;*************************
;*************************
_pkt_terminate	PROC	NEAR
	prologue
	cmp	pkt_initialized,1
	je	SHORT is_init_5
	call	pkt_scan
	or	ax,ax
	jne	SHORT is_init_5
	mov	_pkt_errno,PDE_NO_PKT_DRVR
ret_error_5:
	mov	ax,-1
	epilogue

is_init_5:
	mov	ah,PF_TERMINATE
	mov	bx,WORD PTR [bp+4]	;handle
	call	do_int_xx
	jc	SHORT had_error_5
	xor	ax,ax
	mov	pkt_initialized,al
	epilogue

had_error_5:
	mov	_pkt_errno,dh
	jmp	SHORT ret_error_5

_pkt_terminate	ENDP

;*************************
;*************************
_pkt_get_address	PROC	NEAR
	prologue
	push	di
	cmp	pkt_initialized,1
	je	SHORT is_init_6
	call	pkt_scan
	or	ax,ax
	jne	SHORT is_init_6
ret_error_6:
	mov	ax,-1
	pop	di
	epilogue

is_init_6:
	mov	ah,PF_GET_ADDRESS
	mov	bx,WORD PTR [bp+4]	;handle
	mov	di,ds
	mov	es,di			;buffer (segment)
	mov	di,WORD PTR [bp+6]	;buffer (offset)
	mov	cx,WORD PTR [bp+8]	;length
	call	do_int_xx
	jc	SHORT had_error_6
	mov	ax,cx
	pop	di
	epilogue

had_error_6:
	mov	_pkt_errno,dh
	jmp	SHORT ret_error_6

_pkt_get_address	ENDP

;*************************
;*************************
_pkt_reset_interface	PROC	NEAR
	prologue
	cmp	pkt_initialized,1
	je	SHORT is_init_7
	call	pkt_scan
	or	ax,ax
	jne	SHORT is_init_7
ret_error_7:
	mov	ax,-1
	epilogue

is_init_7:
	mov	ah,PF_RESET_INTERFACE
	mov	bx,WORD PTR [bp+4]	;handle
	call	do_int_xx
	jc	SHORT had_error_7
	xor	ax,ax
	epilogue

had_error_7:
	mov	_pkt_errno,dh
	jmp	SHORT ret_error_7

_pkt_reset_interface	ENDP

;*************************
;*************************
_pkt_set_rcv_mode	PROC	NEAR
	prologue
	cmp	pkt_initialized,1
	je	SHORT is_init_8
	call	pkt_scan
	or	ax,ax
	jne	SHORT is_init_8
ret_error_8:
	mov	ax,-1
	epilogue

is_init_8:
	mov	ah,PF_SET_RCV_MODE
	mov	bx,WORD PTR [bp+4]	;handle
	mov	cx,WORD PTR [bp+6]	;mode
	call	do_int_xx
	jc	SHORT had_error_8
	xor	ax,ax
	epilogue

had_error_8:
	mov	_pkt_errno,dh
	jmp	SHORT ret_error_8

_pkt_set_rcv_mode	ENDP

;*************************
;*************************
_pkt_get_rcv_mode	PROC	NEAR
	prologue
	cmp	pkt_initialized,1
	je	SHORT is_init_9
	call	pkt_scan
	or	ax,ax
	jne	SHORT is_init_9
ret_error_9:
	mov	ax,-1
	epilogue

is_init_9:
	mov	ah,PF_GET_RCV_MODE
	mov	bx,WORD PTR [bp+4]	;handle
	call	do_int_xx
	jc	SHORT had_error_9
	epilogue

had_error_9:
	mov	_pkt_errno,dh
	jmp	SHORT ret_error_9

_pkt_get_rcv_mode	ENDP

;*************************
;*************************
_pkt_get_statistics	PROC	NEAR
	prologue
	push	di
	push	si
	cmp	pkt_initialized,1
	je	SHORT is_init_10
	call	pkt_scan
	or	ax,ax
	jne	SHORT is_init_10
ret_error_10:
	mov	ax,-1
	pop	si
	pop	di
	epilogue

is_init_10:
	mov	ah,PF_GET_STATISTICS
	mov	bx,WORD PTR [bp+4]	;handle
	call	do_int_xx
	jc	SHORT had_error_10
	ASSUME	DS:nothing
	mov	di,ss
	mov	es,di
	mov	di,WORD PTR [bp+6]	; address of buffer
	mov	cx,WORD PTR[bp+8]	; length of buffer in bytes
	shr	cx,1			; Convert to words (we "know"
					; that the buffer is all "longs")
	rep	movsw
	mov	ax,ss
	mov	ds,ax
	ASSUME	DS:DGROUP

	xor	ax,ax
	pop	si
	pop	di
	epilogue

had_error_10:
	ASSUME	DS:DGROUP
	mov	_pkt_errno,dh
	jmp	SHORT ret_error_10

_pkt_get_statistics	ENDP

;*************************
;*************************
_pkt_set_multicast_list	PROC	NEAR
	mov	_pkt_errno,PDE_NO_MULTICAST
	mov	ax,-1
	ret
_pkt_set_multicast_list	ENDP

;*************************
;*************************
_pkt_get_multicast_list	PROC	NEAR
	mov	_pkt_errno,PDE_NO_MULTICAST
	mov	ax,-1
	ret
_pkt_get_multicast_list	ENDP

;*************************
; The following routine may be used as a receiver function
;*************************
	EXTRN	_pkt_rcv_call1:NEAR
	EXTRN	_pkt_rcv_call2:NEAR

helper_ds	DW	?
helper_ss	DW	?
helper_sp	DW	?

	ASSUME	CS:_TEXT, DS:NOTHING, SS:NOTHING, ES:NOTHING
_pkt_receive_helper	PROC	FAR
	; Squirrel away the packet driver's critical registers...
	; This kills any hope of being re-entrant
	mov	helper_ds,ds
	mov	helper_ss,ss
	mov	helper_sp,sp

	or	ax,ax		; First or second call?
				; DON'T DO ANYTHING TO CHANGE FLAGS
				; UNTIL THE JNZ SECOND_CALL, BELOW

	; Get a stack and a way to reach our data segment...
	; MOV instructions don't affect the flags
	mov	ax,DGROUP
	mov	ss,ax
	ASSUME	SS:DGROUP
	mov	ds,ax
	ASSUME	DS:DGROUP
	mov	sp,OFFSET DGROUP: END_ISTACK

	; Save some of the packet driver's other registers...
	pushf			; Preserve direction-flag and int-flag

	cld			; Microsoft C needs this, doesn't
				; harm the flags set by the or ax,ax
				; above

	jnz	short second_call	; Check result of or ax,ax
					; way above
	push	cx		; len
	push	bx		; handle
	call	_pkt_rcv_call1	; First call returns a far pointer
				; in dx (segment) & ax (offset)
	add	sp,4		; Remove the parameters from the stack
	mov	es,dx		; segment of far ptr returned
	mov	di,ax		; offset of far ptr returned
help_ret:
	popf			; Restore int status and direction flag
	mov	ds,helper_ds
	ASSUME	DS:NOTHING
	mov	ss,helper_ss
	ASSUME	SS:NOTHING
	mov	sp,helper_sp
	ret

	ASSUME	DS:DGROUP, SS:DGROUP
second_call:
	push	ds		; Segment part of buffer
	push	si		; Offset part of buffer
	push	cx		; len
	push	bx		; handle
	call	_pkt_rcv_call2	; Nothing returned
	add	sp,8		; Remove the parameters from the stack
	jmp	short help_ret
_pkt_receive_helper	ENDP

	ASSUME	CS:_TEXT, DS:DGROUP, SS:DGROUP, ES:NOTHING

_TEXT	ENDS
	END
