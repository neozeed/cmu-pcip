; Copyright 1986 by Carnegie Mellon
; See permission and disclaimer notice in file "cmu-note.mac"
	TITLE	int

	INCLUDE ..\..\include\dos.mac

;  Copyright 1984 by the Massachusetts Institute of Technology  
;  See permission and disclaimer notice in file "notice.h"  

_TEXT	SEGMENT

	PUBLIC	_int_off
	PUBLIC	_int_on


_int_off:
	cli
	ret

_int_on:
	sti
	ret
_TEXT	ENDS
	END
