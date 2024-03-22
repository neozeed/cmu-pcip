; Copyright 1986 by Carnegie Mellon
; See permission and disclaimer notice in file "cmu-note.mac"
	TITLE	alt

	INCLUDE	..\..\include\dos.mac

	PUBLIC	_alt_key

alt_shift =	08H

KEYSEG	SEGMENT AT 40H
	ORG	017H
kb_flag	DB	?
KEYSEG	ENDS

_TEXT	SEGMENT

; alt_key() - Returns 1 if the alt key is depressed else returns 0.

_alt_key:
	push	ds
	mov	ax,SEG KEYSEG
	mov	ds,ax
	ASSUME DS:KEYSEG

	xor	ax,ax
	test	kb_flag,alt_shift
	jz	done
	inc	ax
done:	pop	ds
	ret
_TEXT	ENDS
	END
