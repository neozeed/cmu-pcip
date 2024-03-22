; Copyright 1986 by Carnegie Mellon
; See permission and disclaimer notice in file "cmu-note.mac"
;  Copyright 1984 by the Massachusetts Institute of Technology  
;  See permission and disclaimer notice in file "notice.h"  

	INCLUDE	..\..\include\dos.mac


	PUBLIC	_bell_init	;initialize bell
	PUBLIC	_ring		;ring the bell
	PUBLIC	_bell_off	;turn the bell off

pitch_port = 042H	;port that controls the pitch of the bell
timctl	= 043H		; PC address for timer load control register
timcw	= 0B6H		; Control byte for: timer 2, load lsb and msb,
			; output square wave, count in binary.
pitch = 525		;the divisor for the desired pitch
bell_port = 061H	;port to turn on bell
bell = 003H		;mask to turn on bell
count = 2		;how long to wait before turning off the bell

;time_int = 01cH * 4	;interupt vector for the system clock
INTVECT	SEGMENT
	ORG	01CH * 4
time_int DW	?
INTVECT	ENDS

_TEXT	SEGMENT
;bell_init() - initialize interupts for bell

_bell_init:
	push	ds

	;save timer interupt vector
	cli
	mov	ax,0			;set up data segment
	mov	ds,ax
	ASSUME	DS:INTVECT

	mov	ax,time_int
	mov	bx,time_int+2

	;load new interupt vector
	mov	time_int,OFFSET _TEXT:int_ring
	mov	time_int+2,cs
	pop	ds
	ASSUME	DS:DGROUP

	mov	int_off,ax		;now is when the old int vec gets saved
	mov	int_seg,bx

	mov	ax,timcw		;set up timer
	out	timctl,al
	mov	ax,pitch		;set pitch value
	out	pitch_port,al
	mov	al,ah
	out	pitch_port,al
	in	al,bell_port		;save value at bell port
	mov	bell_save,al
	sti
	ret
	
;ring() - rings the bell

_ring:
	in	al,bell_port
	or	al,bell		;turn on bell
	out	bell_port,al
	add	bell_count,count	;load count to ring bell
	ret

;int_ring - interupt level for ringing the bell
;		is enterered at every tic, bell_count is decremented each time
;		when it is zero the bell is turned off and the interupt vector
;		is reset

int_ring:
	push	ds
	push	ax

	;set up data segment
;	mov	ax,cs
;	add	ax,#_data_seg
	mov	ax,SEG DGROUP
	mov	ds,ax

	cmp	bell_count,0
	jz	off_bell
	dec	bell_count
	jmp	not_zero

off_bell:
	mov	al,bell_save	;turn off bell
	out	bell_port,al

not_zero:
	pop	ax
	pop	ds
	iret

;bell_off() - replaces the interupt vector for the bell

_bell_off:
	push	ds

	;reset interupt vector
	mov	ax,int_off
	mov	bx,int_seg
	mov	cx,0		;set up data segment
	mov	ds,cx
	ASSUME	DS:INTVECT

	mov	time_int,ax
	mov	time_int+2,bx
	pop	ds
	ret
_TEXT	ENDS

_BSS	SEGMENT
int_off	DW	?	;space to save the timer interupt vector
int_seg	DW	?

bell_count DW	?	;count for the bell, stops ringing when count
				;is 0
bell_save DB	?	;saved value from bell port
_BSS	ENDS
	END
