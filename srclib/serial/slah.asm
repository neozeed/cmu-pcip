; Copyright 1986 by Carnegie Mellon
; See permission and disclaimer notice in file "cmu-note.mac"
;  Copyright 1984 by the Massachusetts Institute of Technology
;  See permission and disclaimer notice in file "notice.h"  

	INCLUDE	..\..\include\dos.mac

; aux_hndlr - a package of routines to manage the auxiliary port

_TEXT	SEGMENT

	PUBLIC	_init_aux	;initializes port and interrupt vector
				;limited to 8 characters since c is limited
	PUBLIC	_close_aux	;turns off interrupts from the aux port
	PUBLIC	_getint
	PUBLIC	_int_hnd	;interrupt handler
	PUBLIC	__wake_serial	;restarts transmitter by turning on interrupts
	EXTRN	_sl_bin:NEAR	;DDP
	EXTRN	_sl_bout:NEAR	;DDP

_DATA	SEGMENT
	EXTRN	_serint:WORD	;number of spurious serial io interrupts
	EXTRN	_badtx:WORD	;bad transmitter interrupt count

	PUBLIC	_slrint
	PUBLIC	_slint
	PUBLIC	_sltint
	PUBLIC	_sllstat
	PUBLIC	_slmstat
	PUBLIC	_intcomp
	PUBLIC	_sliir

	EXTRN	_PSEND:BYTE	; sending a packet flag
	EXTRN	_DOSEND:BYTE	; please, do send that character...
_DATA	ENDS


lcr = 03fbH	;line control register
dla = 080H	;divisor latch access
dll = 03f8H	;low divisor latch
dlh = 03f9H	;high divisor latch
datreg = 03f8H	;data register
;mode = 01bH	;8-bits, even parity
mode = 003H	; 8 bits, no parity
intr = 00cH	;interrupt number for aux port

_INTVEC	SEGMENT
	ORG	intr*4	;offset of interrupt vector
int_off	DW	?
_INTVEC	ENDS

mcr = 03fcH	;modem control register
lsr = 03fdH	;line status register
msr = 03feH	;modem status register

dtr = 00bH	;bits to set dtr line
ier = 03f9H	;interrupt enable register
rxint = 001H	;enable data available interrupt
txint = 002H	;enable tx holding register empty interrupt
tcheck = 020H	; mask for checking on tx reg status on interrupt
rcheck = 001H	; mask for checking rx reg status on interrupt
iir = 03faH	;interrupt identification register
imr = 021H	;interuprt mask register
int_mask = 0efH	;mask to clear bit 4
int_pend = 001H	; there is an interrupt pending
lstat = 006H	; line status interrupt
rd = 004H	; received data interrupt
wr = 002H	; ready to xmit data
mstat = 000H	; modem status interrupt
ocw2 = 020H	;operational control word on 8259
eoi = 064H	;specific end of interrupt 4
endc = 245	;end of packet indicator
req = 243	; request indicator


; init_aux(divisor) - initializes 8250 and set up interrupt vector to int_hndlr
;			divisor is the divisor for the baud rate generator

_init_aux:
	cli
	push	bp
;***** ***** ***** *****  EXTRANEOUS STUFF TO FIND OUT HOW 8250 WORKS  *****
;	mov	dx,ier		;let's find out what we start with.
;	in
;	mov	_slmstat,ax	;statistic mstat is enable register at entry.
;
;	mov	dx,iir		;check iir on chance enable is non-zero.
;	in
;	mov	ah,0		;clear high byte
;	mov	_sliir,ax	;statistic iir is iir at entry.
;
;	mov	dx,ier		;prepare to enable
;	mov	al,0		;first, disable
;	out
;	mov	al,txint	;now enable for xmit condx.
;	out
;	mov	dx,iir		;look to see what condx are there now
;	in
;	mov	ah,0		;clear high byte
;	mov	_sllstat,ax	;statistic lstat is iir after xmit enable.
;	mov	dx,ier		;now cycle the xmit enable bit.
;	mov	al,0
;
;	out
;	mov	al,txint
;	out
;	mov	dx,iir		;look at condx again.
;	in
;	mov	ah,0
;	mov	_slint,ax	;statistic slint is condx after xmit cycle.
;***** ***** ***** *****  END OF EXTRANEOUS STUFF  ***** *****
	mov	bp,sp
;					reset the UART

	mov	dx,mcr
	in	al,dx
	mov	oldmcr,ax		; save the old mcr
	mov	al,0
	out	dx,al
;clear_int:				clear out any lurking conditions
	mov	dx,lsr			;reset line status condition
	in	al,dx			
	mov	dx,datreg		;reset receive data condition
	in	al,dx
	mov	dx,msr			;reset modem deltas and condition
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
	ASSUME	DS:_INTVEC

	mov	bx,int_off
	mov	cx,int_off+2
	mov	int_off,OFFSET _TEXT:_int_hnd
	mov	int_off+2,cs
	pop	ds
	ASSUME	DS:DGROUP

	mov	int_addr,bx		;save the original interrupt vector
	mov	int_segment,cx

	;enable interrupts on 8259 and 8250
	in	al,imr			;set enable bit on 8259
	and	al,int_mask
	out	imr,al
	mov	dx,ier			;enable interrupts on 8250
	mov	al,txint		;set a lurking xmit interrupt first
	out	dx,al			;ZAP!
	mov	al,rxint		;now enable for receive only
	out	dx,al
	mov	dx,mcr			;set dtr and enable int driver
	mov	al,dtr
	out	dx,al

;	mov	al,00aH		;read the irr
;	out	ocw2
;	in	ocw2
;	mov	al,00bH		;read the isr
;	out	ocw2
;	in	ocw2

	pop	bp
	sti
	ret

; close_aux - turns off interrupts from the auxiliary port

_close_aux:
	;turn off 825
	mov	dx,ier
	mov	al,0
	out	dx,al

	;turn off 8259
	cli
	mov	dx,imr
	in	al,dx
	or	al,NOT int_mask
	out	dx,al

	;reset interrupt vector
	push	ds
	mov	ax,0
	mov	bx,int_addr
	mov	cx,int_segment
	mov	ds,ax
	ASSUME	DS:_INTVEC

	mov	int_off,bx
	mov	int_off+2,cx
	pop	ds
	ASSUME	DS:DGROUP

	mov	dx,mcr		; restore old mcr
	mov	ax,oldmcr
	out	dx,al

	sti
	ret

_getint:
	push	ds
	xor	ax,ax
	mov	ds,ax
	ASSUME	DS:_INTVEC

	mov	ax,int_off
	pop	ds
	ASSUME	DS:DGROUP

	ret

; int_hndlr - handles interrupts generated by the aux. port

_int_hnd:
	sti
	push	bp
	push	ds
	push	es		;DDP
	push	di
	push	ax
	push	bx
	push	cx
	push	dx

	;set up data segment
;	mov	ax,cs
;	add	ax,_data_seg
	mov	ax,SEG DGROUP
	mov	ds,ax
	mov	es,ax		;DDP

	inc	_slint

;	mov	dx,ocw2	;tell the 8259 that we want more.
;	mov	al,eoi
;	out	dx,al

	;find out where interrupt came from and jump to routine to handle it

	mov	dx,iir
	in	al,dx
	cmp	al,rd
	jz	rx_int		;if it's from the receiver
	cmp	al,wr
	jz	tx_int		;if it's from the transmitter
	cmp	al,lstat
	jz	lstat_int	;interrupt because of line status
	cmp	al,mstat
	jz	mstat_int	;interrupt because of modem status
	inc	_sliir		;we got an interrupt but none was pending
	jmp	int_end		;go finish this interrupt.

repoll:
	mov	dx,lsr		;we always expect received data, so
	in	al,dx		;check status to see if any is ready.
	and	al,rcheck	;get received data bit
	jnz	good_rx		;yes, go accept the byte.

	mov	dx,ier		;look at transmit condition
	in	al,dx		;to see if we are enabled to send data.
	and	al,txint	;
	jz	int_end		;not enabled, so go finish this interrupt.
	mov	dx,lsr		;we are enabled, so look for tx condition.
	in	al,dx
	and	al,tcheck
	jnz	goodtx		;transmitter is finished, go get more data.
	jmp 	int_end		;tx busy, nothing else to do but exit.

tx_int:
	mov	dx,lsr
	in	al,dx
	and	al,tcheck
	jnz	goodtx		;good interrupt

	inc	_badtx		; bump counter
	jmp	repoll		;see if any more interrupts

goodtx:
	inc	_sltint		; count number of valid tx conditions
	call	_sl_bout	; actual output routine can be in C
	cmp	_DOSEND,0	; should this byte be sent?
	jz	nosend

	mov	dx,datreg	; now output the byte
	out	dx,al		; it was in al

nosend:
	cmp	_PSEND,0	; check if still active
	jnz	repoll		;see if any more interrupts

Ldie:	mov	dx,ier		; if it was, turn off interrupts
	mov	al,rxint
	out	dx,al
	jmp	repoll		;go look for other conditions

rx_int:
	mov	dx,lsr		;check to see if read is real
	in	al,dx
	and	al,rcheck	;look at receive data bit
	jnz	good_rx		;real, go get byte
	inc	_intcomp	;must be a left-over signal.
	jmp	repoll		;go look for other conditions

good_rx:
	inc	_slrint
	mov	dx,datreg	; get the byte
	in	al,dx
	push	ax		; the argument to _sl_bin
	call	_sl_bin		; call a possible C routine
	pop	ax		
	jmp	repoll		;see if any more interrupts

lstat_int:
	inc	_sllstat
	mov	dx,lsr		;clear interrupt
	in	al,dx
	jmp	repoll		;go look for more interrupts

mstat_int:
	inc	_slmstat
	mov	dx,msr		;clear interrupt
	in	al,dx
	jmp	repoll		;go look for more interrupts

;repoll:
int_end:
	mov	dx,ocw2	;tell the 8259 that we want more.
	mov	al,eoi
	out	dx,al
	pop	dx
	pop	cx
	pop	bx
	pop	ax
	pop	di
	pop	es		;DDP
	pop	ds
	pop	bp
	iret


__wake_serial:
	cli
	mov	dx,ier		; turn on serial interrupts
	in	al,dx
	or	al,txint
	out	dx,al
	sti
	ret

_TEXT	ENDS

_DATA	SEGMENT
int_addr	DW	?	;the initial contents of the serial line's
int_segment	DW	?	;interrupt vector

		DW	0
_sltint		DW	0
_slrint		DW	0
_sllstat	DW	0
_slmstat	DW	0
_intcomp	DW	0
_slint		DW	0
_sliir		DW	0
_badrx		DW	0
oldmcr		DW	0
_DATA	ENDS
	END
