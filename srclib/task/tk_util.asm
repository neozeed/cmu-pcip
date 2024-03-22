; Copyright 1986 by Carnegie Mellon
; See permission and disclaimer notice in file "cmu-note.mac"
;  Copyright 1984 by the Massachusetts Institute of Technology  
;  See permission and disclaimer notice in file "notice.h"  

	INCLUDE ..\..\include\dos.mac

; This file contains assembly code for the machine dependent features of
;	Larry Allen's tasking package as now implemented on the IBM PCs.

; 2-11-86 Set up ES register in case someone has modified it.  The Microsoft C
;	manual isn't too clear on whether or not ES needs to be preserved
;	across subroutine calls.

; 1-16-86 Add code to preserve the SI and DI registers across task switches
;	Since Micrsoft C uses them for register variables and they must be
;	preserved across a subroutine call.
;					Drew D. Perkins

; 3-21-86 Changed __stk_sys_fill to allocate the entire DOS stack beginning
;	from _end to the current SP.  It now returns the size of the stack
;	allocated instead of the beginning since we know where that.
;					Drew D. Perkins

tk	=	4
base	=	6
entry	=	8
arg	=	10
tk_sp	=	0
tk_guard =	10			;DDP
guard	=	1234H

_TEXT	SEGMENT

	EXTRN	__cdump:NEAR
	PUBLIC	_tk_frame

_tk_frame:
	push	bp
	mov	bp,sp

	mov	bx,base[bp]	; get the stack base

	dec	bx		; 	what I'd give for an autodecrement
	dec	bx		;	addressing mode...

	mov	WORD PTR [bx],0		; a foney fp, make better later
	dec	bx
	dec	bx

	mov	cx,arg[bp]
	mov	[bx],cx
	dec	bx
	dec	bx

	mov	cx,OFFSET __cdump
	mov	[bx],cx		; an error trapping function.

	mov	dx,bx		; save addr of previous fp
	dec	bx		; set entry point
	dec	bx
	mov	cx,entry[bp]
	mov	[bx],cx

;DDP Begin - Must push si and di since they will be popped off in tk_swtch()
	dec  bx
	dec  bx
	mov  cx, si
	mov  [bx],cx
 
	dec  bx
	dec  bx
	mov  cx, di
	mov  [bx],cx
;DDP End
 
	dec	bx		; set previous fp
	dec	bx
	mov	[bx],dx

	mov	dx,bx
	mov	bx,tk[bp]	; install knowledge of stack in task thingie
	mov	tk_sp[bx],dx	

	pop	bp
	ret


; And, here we have the routine which actually switches from one task to
;	another... ho hum.

_DATA	SEGMENT
	EXTRN	_tk_cur:WORD
	EXTRN	_end:WORD	; DDP
_DATA	ENDS

	PUBLIC	_tk_swtch

_tk_swtch:
	push	si		;DDP Preserve si register of task
	push	di		;DDP Preserve di too
	push	bp
	mov	bp,sp		; now for a bit of stack perversity...

	mov	bx,_tk_cur	; find the current task
	mov	tk_sp[bx],bp
	mov	bx,tk+4[bp]	;DDP add 4 -get new task context thingie stuff
	mov	_tk_cur,bx	; make it well known
	mov	bp,tk_sp[bx]	; restore saved fp

	mov	sp,bp		; restore sp
	pop	bp		; return in new context (???)
	pop  	di		;DDP Pop di back off stack
	pop	si		;DDP Pop si back off stack
	ret

	PUBLIC	__stk_sys_fill
__stk_sys_fill:
	push	bp
	mov	bp,sp
	push	di

;DDP - Begin
COMMENT *
MIT code returned address of guard word.  Due to MSC stack mechanism
we'd really like the size of the stack actually available.

	mov	cx,4[bp]	; get the number of bytes to reserve

	mov	ax,sp
	sub	ax,2		; reserve space which we'll use in a moment

	sub	ax,cx		; ax now has address to start fill at
*
	mov	cx,sp		; get current top of stack
	sub	cx,2		; reserve space which we'll use in a moment
	mov	ax,offset DGROUP:_end
				; get address of end of data area
				; i.e. desired end of stack
	sub	cx,ax		; cx now has number of bytes to reserve
				; ax has beginning of reserved stack
;DDP - End

do_fill:
;DDP	push	ax		; use of reserved space
;DDP				; saving pointer to beginning of guard words
	push	cx		; DDP use of reserved space
				; saving number of bytes (i.e. size of stack)
	sar	cx,1		; adjust cx to be a word count
	mov	di,ax
	mov	ax,SEG DGROUP	; DDP get current data segment
	mov	es,ax		; DDP set up ES just in case
	mov	ax,guard
	cld			; set direction flag for increment
l:
	stosw			; damned stos doesn't decrement cx so
	dec	cx		; I can't use repnz
	jnz	l

	pop	ax		; return the beginning of the guard
	pop	di
	pop	bp
	ret

	PUBLIC	__stk_fill
__stk_fill:
	push	bp
	mov	bp,sp
	push	di

	mov	cx,4[bp]

	mov	ax,6[bp]
	jmp	do_fill
_TEXT	ENDS
	END
