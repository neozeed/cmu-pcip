; Copyright 1986 by Carnegie Mellon
; See permission and disclaimer notice in file "cmu-note.mac"
	TITLE	dma

	INCLUDE ..\..\include\dos.mac

; routines to setup and perform dma
;
;	dma_setup(channel, buffer, length, dir)
;		get ready to do a transfer
;			dir = 0 for write (copy into memory)
;			      1 for read (copy out of memory)
;
;	dma_reset(channel)
;		reset the channel
;
;	dma_done(channel)
;		returns -1 if the dma has completed

byte_ptr	EQU	00ch	; byte pointer flip-flop
mode		EQU	00bh	; dma controller mode register
dma_mask	EQU	00ah	; dma controller mask register

addr		EQU	000h	; per-channel base address
count		EQU	001h	; per-channel byte count

read_cmd	EQU	048h	; read mode
write_cmd	EQU	044h	; write mode
set_cmd		EQU	000h	; mask set
reset_cmd	EQU	004h	; mask reset


; dma controller page register table
; this table maps from channel number to the i/o port number of the
; page register for that channel
_DATA	SEGMENT
page_table	DW	00087h	; channel 0
		DW	00083h	; channel 1
		DW	00081h	; channel 2
		DW	00082h	; channel 3
_DATA	ENDS

_TEXT	SEGMENT

	PUBLIC	_dma_setup

; argument offsets
CHANNEL		EQU	4
BUFFER		EQU	6
LEN		EQU	8
DIRECTION	EQU	10

_dma_setup:
	push	bp
	mov	bp,sp
	pushf
	cli

	mov	dx,byte_ptr	; reset the byte pointer to be safe
	out	dx,al

	mov	bx,BUFFER[bp]	; get the address of the data

	mov	ax,ds		; get the segment
	mov	cl,4
	rol	ax,cl

	mov	ch,al		; save the lower four bits EQU page number
	and	al,0f0h
	add	ax,bx
	adc	ch,0

	mov	bx,CHANNEL[bp]
	shl	bx,1		; double the channel number for use as base

	mov	dx,bx		; figure out the port for the address register
				; it's offset 0, don't add anything to dx
	out	dx,al		; output it
	mov	al,ah
	out	dx,al

	mov	al,ch		; output top 4 bits of address
	and	al,00fh
	mov	dx,offset DGROUP:page_table
				; couldn't just do "add bx,page_table"
	add	bx,dx
	mov	dx,[bx]		; get the page register address
	out	dx,al

	; restore the channel number to bx
	mov	bx,CHANNEL[bp]
	mov	cx,bx			; and save it in cx
	shl	bx,1

	; now we've set up the base address. output the length
	mov	ax,LEN[bp]	; get the length

	mov	dx,bx		; get the dma channel base address
	add	dx,count	; and add in the offset of the count register

	out	dx,al		; and output the length
	mov	al,ah
	out	dx,al

	; now output the mode
	cmp	WORD PTR DIRECTION[bp],0
	jnz	do_read

	mov	al,write_cmd
	jmp	do_mode
do_read:
	mov	al,read_cmd
do_mode:
	add	al,cl		; add in the channel number
	mov	dx,mode
	out	dx,al

	mov	dx,dma_mask
	mov	ax,set_cmd
	add	al,cl
	out	dx,al

	popf
	pop	bp
	ret

	PUBLIC	_dma_reset
_dma_reset:
	push	bp
	mov	bp,sp

	mov	dx,dma_mask
	mov	ax,reset_cmd
	add	ax,CHANNEL[bp]
	out	dx,al

	pop	bp
	ret

	PUBLIC	_dma_done
_dma_done:
	push	bp
	mov	bp,sp
	pushf
	cli

	mov	dx,CHANNEL[bp]
	shl	dx,1
	add	dx,count
	in	al,dx

	mov	ah,al

	in	al,dx

	xchg	al,ah
	popf
	pop	bp
	ret
_TEXT	ENDS
	END
