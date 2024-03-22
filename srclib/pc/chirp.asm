; Copyright 1986 by Carnegie Mellon
; See permission and disclaimer notice in file "cmu-note.mac"
	TITLE	chirp

	INCLUDE ..\..\include\dos.mac

;  Copyright 1984 by the Massachusetts Institute of Technology  
;  See permission and disclaimer notice in file "notice.h"  


_TEXT	SEGMENT

	PUBLIC	_ring		; program to make sound for ASCII BEL code

portb	=	061H		; PC address for port that gates speaker
timer2	=	042H		; PC address for timer that drives speaker
timctl	=	043H		; PC address for timer load control register
timcw	=	0B6H		; Control byte for: timer 2, load lsb and msb,
				; output square wave, count in binary.
;DDP Begin
_DATA	SEGMENT
	PUBLIC	_chirpf, _chirps, _chirpl, _chirpd

_chirpf	DW	800		; Start chirp at 1487 Hz (F'')
_chirps	DW	25		; Use 25 different pitches
_chirpl	DW	1000		; Length of any one pitch, 10 ms
_chirpd	DW	-24		; Decrement for each pitch
_DATA	ENDS
;DDP end

_ring:	push	bp
	push	ax
	push	bx
	push	cx

	mov	ax,_chirpf	;DDP Start at F'' (1487 Hz).
	mov	divisor,ax	; initialize clock stuffer

	mov	al,timcw	; Get timer control byte and
	out	timctl,al	; set timer control register.
	mov	ax,divisor	; Make sure timer running when we
	out	timer2,al	; first start the loudspeaker
	mov	al,ah		; ..
	out	timer2,al	; ..
	in	al,portb	; Get old port control byte
	push	ax		; and save it.
	or	al,03		; Make up a new one, and turn
	out	portb,al	; bell on

	mov	bx,_chirps	;DDP Number of segments in chirp
	cmp	bx,0		;DDP Test for zero
	jz	g9		;DDP Skip loop
g6:	mov	cx,_chirpl	;DDP length of segment
	jcxz	g8		;DDP Just in case it's zero
g7:	loop	g7		; dally
g8:	mov	ax,divisor	; Get latest divisor
	add	ax,_chirpd	;DDP Make it bigger/smaller
	mov	divisor,ax	; Put it back
	out	timer2,al	; and also in the timer
	mov	al,ah		; ..
	out	timer2,al	; ..
	dec	bx		; Are there any more segments?
	jnz	g6		; Yes, go do them too.

g9:	pop	ax		; No, restore original port control byte.
	and	al,0FCH		; Make sure speaker control bits off.
	out	portb,al	; ..

	pop	cx
	pop	bx
	pop	ax
	pop	bp
	ret			; Return to DOS.
_TEXT	ENDS

_BSS	SEGMENT
divisor	DW	?		; Current divisor
portcb	DB	?		; Place to hold old port control byte.
_BSS	ENDS
	END
