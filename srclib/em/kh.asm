; Copyright 1986 by Carnegie Mellon
; See permission and disclaimer notice in file "cmu-note.mac"
	TITLE	kh

	INCLUDE	..\..\include\dos.mac

;  Copyright 1984 by the Massachusetts Institute of Technology  
;  See permission and disclaimer notice in file "notice.h"  


	PUBLIC	_kbd_stat
	PUBLIC	_kbd_in

bios_kbd = 016H	;interupt for bios keyboard call
read = 0	;bios argument - get keyboard character
stat = 1	;bios argument - get keyboard status
break = 046H	;break scan code
true = -1
false = 0

_TEXT	SEGMENT

; kbd_stat - checks to see if there is a character ready from the keyboard
;	returns true if there is and false if there ain't

_kbd_stat:
	mov	ah,stat		;bios argument - get keyboard status
	int	bios_kbd	;call bios
	jz	none		;bios returns z flag set if there is
				;a character ready
	mov	ax,true
	ret

none:	mov	ax,false
	ret


; kbd_in - returns the next character from the keyboard
;	it waits until a character is ready

_kbd_in:
	mov	ah,read
	int	bios_kbd
	ret
_TEXT	ENDS
	END
