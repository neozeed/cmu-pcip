
_TEXT	segment byte public 'CODE'
_TEXT	ends
_DATA	segment word public 'DATA'
_DATA	ends
CONST	segment word public 'CONST'
CONST	ends
_BSS	segment word public 'BSS'
_BSS	ends
DGROUP	group CONST, _BSS, _DATA
	assume cs:_TEXT, ds:DGROUP

PATCH	struc
pretn	dw	?
oldbp	dw	?
int_no	db	?
pad	db	?
PATCH	ends

CALLAT	struc
cretn	dw	?
coldbp	dw	?
parms	dd	?
CALLAT	ends

AT_INT	equ	5ch

_TEXT	segment
	assume cs:_TEXT, ds:DGROUP

	public	doATint
	public	_ATPatch, _ATGetInt
	public	_CallATDriver
	public	_isATLoaded

driverstring db	'AppleTalk', 0

doATint	proc near
	int	AT_INT
	ret
doATint	endp

_ATPatch proc near
	push	bp
	mov	bp, sp
	push	es
	mov	al, [bp].int_no
	push	cs
	pop	es
	lea	bx, doATint
	inc	bx
	mov	es:[bx], al
	pop	es
	pop	bp
	ret
_ATPatch endp

_ATGetInt proc near
	push	es
	push	cs
	pop	es
	lea	bx, doATint
	inc	bx
	mov	al, es:[bx]
	xor	ah, ah
	pop	es
	ret
_ATGetInt endp

_CallATDriver proc near
	push	bp
	mov	bp, sp
	push	ds
	push	es
	lds	bx, [bp].parms
	call	doATint
	pop	es
	pop	ds
	pop	bp
	ret
_CallATDriver endp

_isATLoaded proc near
	push	si
	push	di
	push	es
	push	ds
	cld
	call	_ATGetInt
	mov	dx, ax
chkloop:cmp	dx, 70h
	jne	checkstring
	xor	ax, ax
	jmp	chksplit
checkstring:mov	bx, dx
	shl	bx, 1
	shl	bx, 1
	xor	ax, ax
	mov	es, ax
	lds	si, es:[bx]
	mov	ax, ds
	or	ax, si
	jz	keepchecking
	sub	si, 16
	mov	di, offset driverstring
	mov	cx, 9
	push	cs
	pop	es
	repe	cmpsb
	jne	keepchecking
	call	_ATGetInt
	cmp	ax, dx
	jz	chksplit
	push	dx
	call	_ATPatch
	add	sp, 2
	call	_ATGetInt
	jmp	chksplit
keepchecking:inc dx
	jmp	chkloop
chksplit:pop	ds
	pop	es
	pop	di
	pop	si
	ret
_isATLoaded endp

_TEXT	ends
	end
