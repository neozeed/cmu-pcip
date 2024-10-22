; Copyright 1986 by Carnegie Mellon
; See permission and disclaimer notice in file "cmu-note.mac"
;  Copyright 1984 by the Massachusetts Institute of Technology  
;  See permission and disclaimer notice in file "notice.h"  


; aux2_hndlr - a package of routines to manage the second auxiliary port

	INCLUDE	..\..\include\dos.mac

	PUBLIC	_init2_aux	;initializes port and interrupt vector
	PUBLIC	_close2_aux	;turns off interrupts from the aux port
	PUBLIC	_int2_hndlr	;interrupt handler
	PUBLIC	_dtr2_off	;turns off dtr etc
	PUBLIC	_dtr2_on	;turns on dtr etc
	PUBLIC	_crcnt2		;returns number of characters in input buffer
	PUBLIC	_cread2		;read next character in input buffer
	PUBLIC	_cwcnt2		;returns number of free bytes in output buffer
	PUBLIC	_cwrit2		;writes character to output buffer
	PUBLIC	_local2		;writes a character to the input queue
	PUBLIC	_make2_break	;causes a break to be sent
;	.globl	_etext		;end of text

base = 02f0H	;base of address of aux. port registers
xint = 00bH	;interrupt number for aux port

INTVECT	SEGMENT
	ORG	xint*4		;offset of interrupt vector
int_off DW	?
INTVECT	ENDS

rsize = 2048	;size of receive queue
tsize = 256	;size of transmit queue
datreg = base + 08H	;data register
dll = base + 08H	;low divisor latch
dlh = base + 09H	;high divisor latch

ier = base + 09H	;interrupt enable register
iir = base + 0aH	;interrupt identification register
lcr = base + 0bH	;line control register
mcr = base + 0cH	;modem control register
lsr = base + 0dH	;line status register
msr = base + 0eH	;modem status register
dla = 080H	;divisor latch access
mode = 003H	;8-bits, no parity
dtr = 00bH	;bits to set dtr line
dtr_off = 000H	;turn off dtr, rts, and the interupt driver
thre = 020H	;mask to find status of x-mit holding register
rxint = 001H	;enable data available interrupt
txint = 002H	;enable tx holding register empty interrupt
tcheck = 020H	; mask for checking tx reg status on interrupt
rcheck = 001H	; mask for checking rx reg status on interrupt
imr = 021H	;interuprt mask register
int_mask = 0f7H	;mask to clear bit 3
int_pend = 001H	;there is an interrupt pending
mstat = 000H	;modem status interrupt
wr = 002H	;ready to xmit data
rd = 004H	;received data interrupt
lstat = 006H	;line status interrupt
ack = 244	;acknowledge symbol
parity = 07fH	;bits to mask off parity
ocw2 = 020H	;operational control word on 8259
eoi = 063H	;specific end of interrupt 3
break = 040H	;bits to cause break
true = 1	;truth
false = 0	;falsehood

_TEXT	SEGMENT

; init2aux(divisor) - initializes 8250 and set up int. vector to int2_hndlr
;			divisor is the divisor for the baud rate generator

_init2_aux:
	cli
	push	bp
	mov	bp,sp

	;reset the UART
	mov	al,0
	mov	dx,mcr
	out	dx,al

	mov	dx,lsr		;reset line status condition
	in	al,dx

	mov	dx,datreg	;reset recsive data condition
	in	al,dx
	mov	dx,msr		;reset modem deltas and conditions
	in	al,dx

	;set baud rate with the passed argument
	mov	dx,lcr
	mov	al,dla+mode
	out	dx,al
	mov	dx,dll
	mov	al,4[bp]	;low byte of passed argument
	out	dx,al
	mov	dx,dlh
	mov	al,5[bp]	;high byte of passed argument
	out	dx,al

	;set 8250 to 8 bits, no parity
	mov	dx,lcr
	mov	al,mode
	out	dx,al

	;set interrupt vector
	push	ds
	mov	ax,0
	mov	ds,ax
	ASSUME	DS:INTVECT

	mov	bx,int_off
	mov	cx,int_off+2
	mov	int_off,OFFSET _TEXT:_int2_hndlr
	mov	int_off+2,cs
	pop	ds
	ASSUME	DS:DGROUP

	mov	int_offset,bx
	mov	int_segment,cx

	;enable interrupts on 8259 and 8250
	in	al,imr			;set enable bit on 8259
	and	al,int_mask
	out	imr,al
	mov	dx,ier			;enable interrupts on 8250
	mov	al,rxint
	out	dx,al
	mov	dx,mcr			;set dtr and enable int driver
	mov	al,dtr
	out	dx,al
	
	pop	bp
	sti
	ret

; close2_aux - turns off interrupts from the auxiliary port

_close2_aux:
	;turn off 8250
	mov	dx,ier
	mov	al,0
	out	dx,al

	;turn off the 8259
	mov	dx,imr
	in	al,dx
	or	al,NOT int_mask
	out	dx,al


	;reset interrupt vector
	mov	bx,int_offset
	mov	cx,int_segment
	push	ds
	mov	ax,0
	mov	ds,ax
	ASSUME	DS:INTVECT
	cli

	mov	int_off,bx
	mov	int_off+2,cx
	pop	ds
	ASSUME	DS:DGROUP

	sti
	ret

; int2_hndlr - handles interrupts generated by the aux. port

_int2_hndlr:
	push	bp
	push	ds
	push	di
	push	ax
	push	bx
	push	cx
	push	dx

	;set up data segment
;	mov	ax,cs
;	add	ax,#_data_seg
	mov	ax,SEG DGROUP
	mov	ds,ax

	;find out where interrupt came from and jump to routine to handle it
	mov	dx,iir
	in	al,dx
	cmp	al,rd
	jz	rx_int		;if it's from the receiver
	cmp	al,wr
	jz	tx_int		;if it's from the transmitter
	cmp	al,lstat
	jz	lstat_int	;interrupt becuase of line status
	cmp	al,mstat
	jz	mstat_int	;interrupt because of modem status
	jmp	int_end		;we got an interrupt when no interrupt
				;was pending, just go away

lstat_int:
	mov	dx,lsr		;clear interrupt
	in	al,dx
	jmp	repoll		;see if any more interrupts

mstat_int:
	mov	dx,msr		;clear interrupt
	in	al,dx
	jmp	repoll		;see if any more interrupts

tx_int:
	mov	dx,lsr
	in	al,dx
	and	al,tcheck
	jnz	goodtx		;good interrupt
	jmp	repoll		;see if any more interrupts

goodtx:	cmp	size_tdata,0	;see if any more data to send
	jne	have_data	;if not equal then there is data to send

	;if no data to send then reset tx interrupt and return
	mov	dx,ier
	mov	al,rxint
	out	dx,al
	jmp	repoll

have_data:
	mov	bx,start_tdata	;bx points to next char. to be sent
	mov	dx,datreg	;dx equals port to send data to
	mov	al,tdata[bx]	;get data from buffer
	out	dx,al			;send data
	inc	bx		;increment start_tdata
	cmp	bx,tsize	;see if gone past end
	jl	ntadj		;if not then skip
	sub	bx,tsize	;reset to beginning
ntadj:	mov	start_tdata,bx	;save start_tdata
	dec	size_tdata	;one less character in x-mit queue
	jmp	repoll

rx_int:
	mov	dx,lsr		;check and see if read is real
	in	al,dx
	and	al,rcheck	;look at receive data bit
	jnz	good_rx		;real, go get byte
	jmp	repoll		;go look for other interrupts

good_rx:
	mov	dx,datreg
	in	al,dx		;get data
	cmp	size_rdata,rsize	;see if any room
	jg	repoll		;if no room then look for more interrupts
	mov	bx,end_rdata	;bx points to free space
	mov	rdata[bx],al	;send data to buffer
	inc	size_rdata	;got one more character
	inc	bx		;increment end_rdata pointer
	cmp	bx,rsize	;see if gone past end
	jl	nradj		;if not then skip
	sub	bx,rsize	;else adjust to beginning
nradj:	mov	end_rdata,bx	;save value
	jmp	repoll

repoll:
	mov	dx,lsr		;we always expect receive data, so
	in	al,dx			;check status to see if any is ready.
	and	al,rcheck	;get received data bit
	jnz	good_rx		;yes, go accept the byte


	mov	dx,ier		;look at transmit condition
	in	al,dx		;to see if we are enabled to send data
	and	al,txint
	jz	int_end		;not enabled, so go away

	mov	dx,lsr		;we are enabled, so look for tx condition
	in	al,dx
	and	al,tcheck
	jnz	goodtx		;transmitter is finished, go get more data
	jmp	int_end		;tx busy, nothing else to do but wait

int_end:
	mov	dx,ocw2	;tell the 8259 that I'm done
	mov	al,eoi
	out	dx,al

	pop	dx
	pop	cx
	pop	bx
	pop	ax
	pop	di
	pop	ds
	pop	bp
	iret

; dtr2_off - turns off dtr to tell modems that the terminal has gone away
;	and to hang up the phone
_dtr2_off:
	mov	dx,mcr
	mov	al,dtr_off
	out	dx,al
	ret

;dtr2_on - turns dtr on
_dtr2_on:
	mov	dx,mcr
	mov	al,dtr
	out	dx,al
	ret


; crcnt2 - returns number of bytes in the receive buffer
_crcnt2:
	mov	ax,size_rdata	;get number of bytes used
	ret

; cread2 - returns the next character from the receive buffer and
;	removes it from the buffer
_cread2:
	mov	bx,start_rdata
	mov	al,rdata[bx]
	and	al,parity	;remove parity@
	mov	ah,0
	inc	bx		;bump start_rdata so it points at next char
	cmp	bx,rsize	;see if past end
	jl	L12		;if not then skip
	sub	bx,rsize	;adjust to beginning
L12:	mov	start_rdata,bx	;save the new start_rdata value
	dec	size_rdata	;one less character
	ret

; cwcnt2 - returns the amount of free space remaining in the transmit buffer
_cwcnt2:
	mov	ax,tsize	;get the size of the x-mit buffer
	sub	ax,size_tdata	;subtract the number of bytes used
	ret

; cwrit2 - the passed character is put in the transmit buffer
_cwrit2:
	push	bp
	mov	bp,sp

	cmp	first_send,0
	jz	not_first

	mov	dx,datreg
	mov	al,4[bp]
	out	dx,al

	xor	al,al
	mov	al,first_send
	jmp	L44

not_first:
	mov	bx,end_tdata	;bx points to free space
	mov	al,4[bp]	;move data from stack to x-mit buffer
	mov	tdata[bx],al
	inc	bx		;increment end_tdata to point to free space
	cmp	bx,tsize	;see if past end
	jl	L4		;if not then skip
	sub	bx,tsize	;adjust to beginning
L4:	mov	end_tdata,bx	;save new end_tdata
	inc	size_tdata	;one more character in x-mit queue

	mov	dx,ier		;see if tx interrupts are enabled
	in	al,dx
	and	al,txint
	or	al,al
	jnz	L44
	mov	al,rxint+txint	;if not then set them
	out	dx,al
L44:	pop	bp

	ret

; local2() - writes a character to the input queue

_local2:
	cli
	push	bp
	mov	bp,sp

	cmp	size_rdata,rsize	;see if any room
	jg	L14		;if no room then quit
	mov	bx,end_rdata	;bx points to free space
	mov	al,4[bp]	;get data
	mov	rdata[bx],al	;send data to buffer
	inc	size_rdata	;got one more character
	inc	bx		;increment end_rdata pointer
	cmp	bx,rsize	;see if gone past end
	jl	L13		;if not then skip
	sub	bx,rsize	;else adjust to beginning
L13:	mov	end_rdata,bx	;save value

L14:	pop	bp
	sti
	ret

; make2_break() - causes a break to be sent out on the line

_make2_break:
	mov	dx,lcr		;save the line control register
	in	al,dx
	mov	bl,al
		
	mov	al,break	;set break condition
	out	dx,al

	mov	cx,0		;wait a while
waitl:	loop	waitl

	mov	al,bl		;restore the line control register
	out	dx,al
	ret
_TEXT	ENDS

_DATA	SEGMENT
int_offset	DW	0	;the original interrupt offset
int_segment	DW	0	;the original interrupt segment
start_tdata	DW	0	;index to first character in x-mit buffer
end_tdata	DW	0	;index to first free space in x-mit buffer
size_tdata	DW	0	;number of characters in x-mit buffer
start_rdata	DW	0	;index to first character in rec. buffer
end_rdata	DW	0	;index to first free space in rec. buffer
size_rdata	DW	0	;number of characters in rec. buffer
first_send	DB	1	; first data send flag
_DATA	ENDS

_BSS	SEGMENT
tdata	DB	tsize DUP (?)
rdata	DB	rsize DUP (?)
_BSS	ENDS
	END
