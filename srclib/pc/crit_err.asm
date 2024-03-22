; Copyright 1986 by Carnegie Mellon
; See permission and disclaimer notice in file "cmu-note.mac"
	TITLE	crit_err

	INCLUDE ..\..\include\dos.mac

; critical error handler for the spooler
; Written 5/18/84 - John Romkey

	PUBLIC	_crit_install
	PUBLIC	_crit_destall

_BSS	SEGMENT
oldcs	DW	?
oldip	DW	?
_BSS	ENDS

INTVEC	SEGMENT	AT 0
	ORG	24H
crerr_off	DW	?
crerr_seg	DW	?
INTVEC	ENDS

_TEXT	SEGMENT

; theoretically we don't have to save the old critical error handler
; since the DOS manual says that it will be restored automatically when
; we return control to COMMAND.COM.

_crit_install:
	push	ds
	xor	ax,ax
	mov	ds,ax
	ASSUME	DS:INTVEC

	mov	bx,crerr_off
	mov	cx,crerr_seg
	mov	crerr_off,OFFSET _TEXT:criterrhnd
	mov	crerr_seg,cs

	pop	ds
	ASSUME	DS:DGROUP

	mov	oldcs,cx
	mov	oldip,bx

	ret

; return control to whoever had it before we usurped them
_crit_destall:
	mov	bx,oldip
	mov	cx,oldcs

	push	ds
	xor	ax,ax
	mov	ds,ax
	ASSUME	DS:INTVEC

	mov	crerr_off,bx
	mov	crerr_seg,cx
	pop	ds
	ASSUME	DS:DGROUP

	ret

_REBOOT	SEGMENT	AT 0FFFFH
reboot	LABEL	FAR
_REBOOT	ENDS

; handle a critical error: reboot
criterrhnd:
	sti
	jmp	reboot
_TEXT	ENDS
	END
