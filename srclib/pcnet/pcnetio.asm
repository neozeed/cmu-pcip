_TEXT	segment byte public 'CODE'
	assume cs:_TEXT

	public	_pcnet
_pcnet:	push	bp
	mov	bp, sp
	push	si
	push	di
	push	ds
	push	es
	mov	bx, 4[bp]
	mov	es, 6[bp]
	int	5ch
	xor	ah, ah
	pop	es
	pop	ds
	pop	di
	pop	si
	pop	bp
	ret

	public	_estods
_estods:mov	ax, es
	mov	ds, ax
	ret

	public	_doiret
_doiret:mov	sp, bp
	pop	bp
	iret

_TEXT	ends
	end
