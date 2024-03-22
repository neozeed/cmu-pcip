; Copyright 1986 by Carnegie Mellon
; See permission and disclaimer notice in file "cmu-note.mac"
;  Copyright 1984, 1985 by the Massachusetts Institute of Technology  
;  See permission and disclaimer notice in file "notice.h"  


; netdev.a86
;	12/30/83	John Romkey
; device net: which allows the custom structure to be shared by all programs

; device handler equates
command	=	2
status	=	3
lastaddr =	14
buffer	=	14
count	=	18
bpb	=	18

; custom structure equates
ncbytes	=	310	; number of bytes in the customization structure
cversion =	8	; custom structure version number
defint	=	5	; default interrupt vector
defchan	=	1	; default dma channels
defwin	=	512	; default telnet window size
defbase = 	0300H	; default base i/o address
deflowwin	=	200	; default telnet low window
defbits	=	8	; default length of subnet bit field
defmask	=	0FFFFH	; don't ask, it's really 0xFFFF0000
defipradix	=	10	; default ip address output radix

_TEXT	SEGMENT  BYTE PUBLIC 'CODE'
NETDEV	PROC	FAR
	ASSUME  CS: _TEXT, DS: _TEXT, SS: _TEXT

; first the dos special device header
	DD	-1
	DW	08000H
	DW	OFFSET _TEXT:netstrat
	DW	OFFSET _TEXT:netint
name0	DB	"NETCUST"
name7	DB	" "
; and next our copy of the custom structure
custom:
	DD	0	; padding so that old programs won't get confused
	DW	cversion	; custom structure version
	DD	0	; time of last customization
	DD	0	; date of last customization
	DW	0	; baud rate
	DW	0	; type of line driver
	DW	0	; debugging messages
	DW	300	; time zone offset
	DB	"EST",0	; and a label for it
	DW	0	; telnetish custom bits
	DW	0	; routing option
	DW	0	; ethernet address selection option
	DW	3 dup(0) ; and room for the user specified ethernet address
	DD	0	; my address
	DD	0	; my log server
	DD	0	; my default gateway
	DD	0	; my cookie server
	DD	0	; my print server
	DD	0	; my *fill in your favorite formatter* server
	DW	0	; number of time servers
	DW	10 dup(0) ; my time servers
	DW	0	; number of name servers
	DW	10 dup(0) ; name server addresses
	DW	25 dup(0) ; user name
	DW	25 dup(0) ; office name
	DW	10 dup(0) ; phone number
	DW	9 dup(0) ; 3 ethernet addresses
	DW	6 dup(0) ; and 3 ip addresses for the translation cache
	DW	defint	; default interrupt vector
	DW	defchan	; default transmit dma channel
	DW	defbase ; default CSR base
	DW	defwin	; default telnet window
	DW	deflowwin	; default telnet low window
	DW	defbits	; default length of subnet bit field
	DD	defmask	; subnet mask
	DW	defchan	; default receive dma channel
	DW	defipradix	; default ip address output radix
	DW	5 dup(0)	; variables
	DW	0	; other net interface user active flag
	DW	1	; total number of net interfaces on the machine
	DB	67	; first RVD drive ('c')
	DB	0	; padding

; functions which handle device calls
funs	DW	OFFSET _TEXT:netinit
	DW	OFFSET _TEXT:foul
	DW	OFFSET _TEXT:foul
	DW	OFFSET _TEXT:foul
	DW	OFFSET _TEXT:netrd
	DW	OFFSET _TEXT:foul
	DW	OFFSET _TEXT:foul	; okay
	DW	OFFSET _TEXT:foul
	DW	OFFSET _TEXT:netwr
	DW	OFFSET _TEXT:netwr
	DW	OFFSET _TEXT:foul
	DW	OFFSET _TEXT:foul	; okay
	DW	OFFSET _TEXT:foul
; the count used by read and write because FLAMING MSDOS only asks for a
; single character from the driver each time!!!
current	DW	0
; lastdone variable lets us check for transitions to reset current
lastdone DW	0	; 0 means read, 1 means write

wrcalled DW 0
written	DW 0
fouled	DW 0

; and that's it for the custom structure. Now here's the actual driver.
; First handling the strategy call
netstrat:
	ret
; and then handling the interrupt call
netint:
	push	ax
	push	cx
	push	dx
	push	di
	push	si
	push	ds
	push	es

	mov	al,es:command[bx]
	xor	ah,ah
	add	ax,ax
	mov	di,OFFSET _TEXT:funs
	add	di,ax
	jmp	[di+0]

; handle the read call
netrd:
	cmp	lastdone,0
	jz	rdstart
	mov	lastdone,0
	mov	current,0

rdstart:
	mov	cx,es:count[bx]
	les	di,es:buffer[bx]
	mov	ax,cs
	mov	ds,ax
	mov	si,cs:current
	cld
	repnz	movsb

	mov	cs:current,si
	mov	ax,si
	cmp	ax,OFFSET _TEXT:funs
	jl	nordchange
	xor	ax,ax
	mov	current,ax	

nordchange:
	pop	es
	push	es

okay:
	mov	word ptr es:status[bx],00100H

done:	pop	es
	pop	ds
	pop	si
	pop	di
	pop	dx
	pop	cx
	pop	ax
	ret

; handle a write call
netwr:
	inc	wrcalled
	cmp	lastdone,1
	jz	wrstart
	mov	lastdone,1
	mov	current,0
wrstart:
	mov	cx,es:count[bx]
	add	written,cx
	lds	si,es:buffer[bx]
	mov	ax,cs
	mov	es,ax
	mov	di,cs:current
	cld
	repnz	movsb

	mov	cs:current,di
	mov	ax,di
	cmp	ax,OFFSET _TEXT:funs
	jl	nowrchange
	xor	ax,ax
	mov	current,ax	

nowrchange:
	pop	es
	push	es
	jmp	okay

foul:
	inc	fouled
	mov	es:status[bx],08002H
	jmp	done

netinit:
	mov	WORD PTR es:lastaddr[bx],OFFSET _TEXT:_etext
	mov	es:lastaddr+2[bx],cs
	mov	word ptr es:status[bx],00100H

	lds	si,es:bpb[bx]

l:
	mov	al,[si]
	inc	si
	cmp	al,0aH
	jz	done
	cmp	al,0dH
	jz	done
	cmp	al,020H
	jnz	l

again:	mov	al,[si]
	inc	si
	cmp	al,020H
	jz	again
	cmp	al,0aH
	jz	xdone
	cmp	al,0dH
	jz	xdone

	mov	cs:name7,al

xdone:	jmp	done
_etext:
NETDEV	ENDP
_TEXT	ENDS
	END
