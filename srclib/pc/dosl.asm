; Copyright 1986 by Carnegie Mellon
; See permission and disclaimer notice in file "cmu-note.mac"
	TITLE	dosl

	INCLUDE ..\..\include\dos.mac

;  Copyright 1984 by the Massachusetts Institute of Technology  
;  See permission and disclaimer notice in file "notice.h"  


;***** file: dos.a86		operating system entry


;; IBM DOS interface (1/82)

;; int set_dosl(index,arg1,arg2,arg3,arg4) -- make DOS call specified by index
;;		puts	CL <- arg1
;;			CH <- arg2
;;			DL <- arg3
;;			DH <- arg4

_TEXT	SEGMENT

	PUBLIC	_setdosl

_setdosl:
	push	bp		; entry sequence
	mov	bp,sp
	push	di
	push	si

	mov	ah,4[bp]	; get index into AH
	mov	cx,6[bp]	; arg1, arg2 into CX
	mov	dx,8[bp]	; arg2, arg3 into DX
	int	021H		; let operating system do what it will
	mov	ah,0		; zero extend answer to make an int result

	lea	sp,-4[bp]
	pop	si
	pop	di
	pop	bp
	ret

;***** file: dosl.a86		operating system entry


;; IBM DOS interface (1/82)

;; long get_dosl(index) -- make DOS call specified by index

	PUBLIC	_get_dosl

_get_dosl:
	push	bp		; entry sequence
	mov	bp,sp
	push	di
	push	si

	mov	ah,4[bp]	; get index into AH
	int	021H		; let operating system do what it will
	mov	ax,dx
	mov	dx,cx		; longs are returned in ax and dx

	lea	sp,-4[bp]
	pop	si
	pop	di
	pop	bp
	ret
_TEXT	ENDS
	END
