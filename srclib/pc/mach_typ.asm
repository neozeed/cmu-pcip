; Copyright 1986 by Carnegie Mellon
; See permission and disclaimer notice in file "cmu-note.mac"
	TITLE	mach_type

	INCLUDE ..\..\include\dos.mac

_TEXT	SEGMENT

; int _mach_type()
	PUBLIC	__mach_type

__mach_type:
	push	ds

	mov	ax,0f000H
	mov	ds,ax
	mov	bx,0fffeH
	mov	al,[bx]
	mov	ah,0H
	pop	ds
	ret

; int _display_type()
video_bios = 010h	; bios video interrupt
vid_set_mode = 2	; ax value to set 80x25 mode, color
vid_get_mode = 0f00h	; ax value to get current mode

	PUBLIC	__display_type
__display_type:
	push	si
	push	di

	mov	ax,vid_get_mode	
	int	video_bios

	mov	ah,0

	pop	si
	pop	di
	ret
_TEXT	ENDS
	END
