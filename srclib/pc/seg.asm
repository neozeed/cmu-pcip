; Copyright 1986 by Carnegie Mellon
; See permission and disclaimer notice in file "cmu-note.mac"
	TITLE	seg

	INCLUDE ..\..\include\dos.mac

;  Copyright 1984 by the Massachusetts Institute of Technology  
;  See permission and disclaimer notice in file "notice.h"  

_TEXT	SEGMENT

	PUBLIC	_get_ds
	PUBLIC	_get_cs

_get_ds:
	mov	ax,ds
	ret

_get_cs:
	mov	ax,cs
	ret
_TEXT	ENDS
	END
