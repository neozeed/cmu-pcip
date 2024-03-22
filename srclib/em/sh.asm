; Copyright 1986 by Carnegie Mellon
; See permission and disclaimer notice in file "cmu-note.mac"
	TITLE	sh

	INCLUDE	..\..\include\dos.mac

;  Copyright 1984 by the Massachusetts Institute of Technology  
;  See permission and disclaimer notice in file "notice.h"  

; written by David Bridgham
; 9/5/84 - fixed bug in scrolldn which messed up blanking the new line.
;	bug fix first reported by Rob Hagens.	<John Romkey>
; 10/19/84 - added calls to exit_hook.		<John Romkey>
; 11/12/84 - Fixed bug in cursor position       <J. H. Saltzer>
; 11/18/84 - Changed init to not reset graphics card as long as it's in
;		80x25 mode.			<John Romkey>
; 11/19/84 - rearranged copy loop in scrollup to eliminate garbage
;	character in lower left corner.		<John Romkey>
; ~8/10/85 - Use BIOS calls to manipulate the display instead of playing
;	with the hardware ourselves.  Now works with any display that
;	BIOS can handle, which is almost all of them.
;						<Drew D. Perkins>
;  2/11/86 - Make sure ES is set correctly in _read_line().  Microsoft
;	C isn't too clear on the use of ES.
;						<Drew D. Perkins>
; 11/18/86 - LOTS of work to suppress SNOW on color displays.  Very little
;	of the original code is left.  All "long" operations now use BIOS.
;	Short opertaions use BIOS only if color card.
;						<Drew D. Perkins>
;  2/23/87 - Treat EGA cards like monochrome cards instead of color cards.
;	Only CGA's really need to use BIOS since EGA's have hardware
;	support for snow suppression.  Slight speed up using new binary
;	flag, bios_mode.
;						<Charlie Kim (CCK)>

	PUBLIC	_scr_init	;initialize variables and controller
	PUBLIC	_move_lines	;move lines
	PUBLIC	_scrollup	;scroll screen up
	PUBLIC	_scrolldn	;scroll screen down
	PUBLIC	_clear_lines	;clear lines
	PUBLIC	_clear_chars	;DDP clear characters
	PUBLIC	_write_line	;write line
	PUBLIC	_read_line	;read line
	PUBLIC	_write_char	;write character
	PUBLIC	_nwrite_char	;write character with no cursor change
	PUBLIC	_set_cursor	;set cursor
	PUBLIC	_rset_cursor	;reset cursor to previous position
	PUBLIC	_scr_rest	;returns the 6845 to a known state

_DATA	SEGMENT
	EXTRN	_attrib:BYTE	;the current attribute byte
	EXTRN	_x_pos:WORD	;the x cursor position
	EXTRN	_y_pos:WORD	;the y cursor position
	EXTRN	_pos:WORD	;cursor position = _x_pos + 80*_y_pos
_DATA	ENDS
	
space = 020H		;ASCII space character
start_h = 12		;Start address (high)
start_l = 13		;Start address (low)
highpos = 14		;high cursor position register
lowpos = 15		;low cursor position register
line_len = 80		;Line length
screen_size = 80*24	;Characters on the whole screen
video_bios = 010H	; bios video interrupt
vid_set_mode = 2	; ax value to set 8025H mode, B&W
vid_get_mode = 0f00H	; ax value to get current mode
curs_pos = 00300H	; ax value to get current cursor position

_TEXT	SEGMENT

; scr_init() - Initializes various variables and the 8245

_scr_init:
	push	bp		;DDP BIOS trashes these registers
	push	si		;DDP
	push	di		;DDP
	mov	ax,vid_get_mode	
	int	video_bios
	mov	crt_mode,al	;DDP Store crt mode for later reference
	mov	display_page,bh	;DDP Store crt mode for later reference
	cmp	al,7		; B&W card?
	je	bw_screen
	cmp	al,2		; 80x25 B&W mode, color card?
	je	no_set
	cmp	al,3		; 80x25 color mode, color card?
	je	no_set

	mov	ax,vid_set_mode	; set 80x25 B&W mode
	mov	crt_mode,al	;DDP Store crt mode for later reference
	int	video_bios

no_set:
	call	is_ega		;CCK is the card an ega?
	mov	bios_mode,al	;CCK is_ega returns 1 if not ega
	mov	ax,0b800H	; set color parameters for color card
	mov	screen,ax
	mov	ax,3fffH
	mov	mem_mask,ax
	mov	ax,03d4H
	mov	index,ax
	mov	ax,03d5H
	mov	datareg,ax
	
bw_screen:
	; set the cursor position to the current cursor position
	mov	bh,0
	mov	ax,curs_pos
	int	video_bios
	mov	bl,dh
	mov	bh,0

	mov	_y_pos,bx
	mov	dh,0
	mov	_x_pos,dx
	; compute _pos
	mov	ax,bx
	mov	cl,80
	mul	cl
	add	ax,dx
	mov	_pos,ax
	mov	savpos,ax

	;Now if we are on row 25 things will really get screwed up.
	; So the idea is to scroll the screen up one line and put the
	; cursor on the 24th line.
	cmp	bx,24
	jnz	not_25
	mov	ax,24		;the things you have to put up with
	push	ax		;with a crufty architecture
	mov	ax,0
	push	ax
	mov	ax,1
	push	ax
	call	_move_lines

	pop	ax
	pop	ax
	pop	ax

	mov	_y_pos,23	; set the cursor to the 24th line
	mov	ax,_pos
	sub	ax,80
	mov	_pos,ax
	call	_set_cursor

not_25:	pop	di		;DDP
	pop	si		;DDP
	pop	bp		;DDP
	ret


;is_ega - Is the card an EGA?
; returns 0 in ax if ega attached, else 1.
; Figures this out by making a call which is only present in the ega bios.
; from IBM Personal Computer Seminar Proceedings, Volume 2, Number 11-1

is_ega	proc	near
	mov	ax,1200h	; Set ax for BIOS call (Alternate Select)
	mov	bx,0ff10h	; Set BL for Return EGA
				; Load BH with invalid info for test
	mov	cl,0fh		; Load CL with reserved switch settings
	int	video_bios	; Okay, go
; bl - color/mono bit
; bh - mem size bits
; cl -ega switch settings
	mov	al,0		; assume ega present
	cmp	cl,0ch		; test reserved switch settings
	jl	is_ega1		; < 0, ega in use
	mov	al,1		; no ega present
is_ega1:
	ret
is_ega	endp


;move_lines(Sy,Dy,n) - Move line Sy to line Dy. Do for n lines. If Sy > Dy
; then the screen is scrolled up. Else the screen is scrolled down.
; Unfortunatly, there is a slight crock in how I wrote this which I didn't
; discover until long after the terminal emulator was working (I'm still
; not sure how I ever made it work). If you are scrolling the screen down
; (i.e. Sy < Dy), then the screen is numbered starting from line 1 instead
; from line 0 as the rest of the commands are.

_move_lines:
	push	bp
	mov	bp,sp
	push	di
	push	si

	mov	cx,4[bp]	;get Sy
	mov	dx,6[bp]	;get Dy
	mov	bx,8[bp]	;get n
	cmp	cx,dx
	jg	TtB		;if cx greater then scroll up
	jl	BtT		;if dx greater then scroll down
	pop	bx		;get rid of attribute byte
	jmp	done		;if equal then pop and return

;scroll up
TtB:	mov	ax,cx		;compute number of lines
	sub	ax,dx
	add	cx,bx		;compute last row
	dec	cx
	jge	low_enough	;is the last row > 24?
	mov	cx,24		;well make it 24 then
low_enough:
	xchg	cx,dx
	mov	ch,cl
	mov	cl,0
	mov	dh,dl
	mov	dl,4FH
	mov	ah,6
	mov	bh,_attrib
	int	video_bios
	jmp	done

;scroll down
BtT:	mov	ax,dx		;compute number of lines
	sub	ax,cx
	sub	cx,bx		;compute first row
	inc	cx		;account for crock explained above
	jge	hi_enough	;is the first row below zero?
	xor	cx,cx		;well make it zero then
hi_enough:
	mov	ch,cl
	mov	cl,0
	mov	dh,dl
	mov	dl,4FH
	mov	ah,7
	mov	bh,_attrib
	int	video_bios

done:	pop	si
	pop	di
	pop	bp
	ret


; scrollup() - scrolls the screen one line up
_scrollup:
	push	bp		;BIOS in the PC doesn't preserve BP
	push	si
	push	di
	mov	ax,0601H
	xor	cx,cx
	mov	dx,174FH	;only to 24th line
	mov	bh,_attrib
	int	video_bios
	pop	di
	pop	si
	pop	bp
	ret


; scrolldn() - move screen one line down
_scrolldn:
	push	bp		;BIOS in the PC doesn't preserve BP
	push	si
	push	di
	mov	ax,0701H
	xor	cx,cx
	mov	dx,174FH
	mov	bh,_attrib
	int	video_bios
	pop	di
	pop	si
	pop	bp
	ret


; clear_lines(y,n) - clear n lines starting from line y

_clear_lines:
	push	bp		;BIOS in the PC doesn't preserve BP
	mov	bp,sp
	push	si
	push	di
	mov	ax,0600H	;clear lines
	mov	cx,4[bp]	;get y
	mov	dx,6[bp]	;get n
	add	dx,cx		;compute last row
	dec	dx
	mov	ch,cl		;put in ch (row)
	xor	cl,cl		;column = 0
	mov	dh,dl		;put in ch (row)
	mov	dl,4FH		;column = 79
	mov	bh,_attrib
	int	video_bios
	pop	di
	pop	si
	pop	bp
	ret


; clear_chars(y, xs, xe, a) - clear chars on line y from xs to xe with
; attribute a

_clear_chars:
	push	bp		;BIOS in the PC doesn't preserve BP
	mov	bp,sp
	push	si
	push	di
	mov	ax,0600H	;clear lines
	mov	bx,4[bp]	;get y
	mov	cx,6[bp]	;get cs
	mov	dx,8[bp]	;get ce
	mov	ch,bl		;stick in row
	mov	dh,bl		;stick in row
	mov	bx,10[bp]	;get a
	mov	bh,bl
	int	video_bios
	pop	di
	pop	si
	pop	bp
	ret


;write_line(buf,y) -- writes chars. in buf to line y of screen memory
;			the chars are assumed to include their attribute bytes

_write_line:
	push	bp
	mov	bp,sp
	push	di
	push	si
	push	es

	mov	ax,6[bp]	;get y
	mov	bx,line_len	;get ax pointing to first char. of line y
	mul	bx
	add	ax,start_screen	;add in start of screen offset
	and	ax,mem_mask	;wrap around at end of screen memory
	shl	ax,1		;two bytes per character
	mov	di,ax		;screen memory is destination
	mov	si,4[bp]	;buf is source
	cld			;set auto increment
	mov	ax,screen	;set es to screen memory
	mov	es,ax
	mov	cx,line_len	;load count
;loop2:	movsw			;move word to screen memory
;	and	di,mem_mask	;wrap around at end of screen memory
;	loop	loop2		;do it for all the chars.
	rep	movsw		;DDP move word to screen memory

	pop	es
	pop	si
	pop	di
	pop	bp
	ret


;read_line(buf,y) -- copies line y from screen buffer to buf.

_read_line:
	push	bp
	mov	bp,sp
	push	di
	push	si
	push	ds

	mov	ax,6[bp]	;get y
	mov	bx,line_len	;get ax pointing to first char. of line y
	mul	bx
	add	ax,start_screen	;add in start of screen offset
	and	ax,mem_mask	;wrap around at end of screen memory
	shl	ax,1		;two bytes per character
	mov	si,ax
	mov	di,4[bp]	;get buf
	cld			;set auto increment
	mov	bx,mem_mask	;get value before changing data segment
	mov	ax,screen	;set ds to screen memory
	mov	ds,ax
	mov	ax,SEG DGROUP	;DDP Microsoft C isn't too clear on this...
	mov	es,ax		;make sure ES is correct
	mov	cx,line_len	;load count
;loop3:	movsw			;move char to buf.
;	and	si,bx		;wrap around at end of screen memory
;	loop	loop3		;do it for all 80 chars.
	rep movsw		;DDP move char to buf.

	pop	ds
	pop	si
	pop	di
	pop	bp
	ret


;nwrite_character(char,pos) - write character char to position pos
; doesn't change the cursor position

_nwrite_char:
	push	bp
	mov	bp,sp

	cmp	bios_mode,1	;CCK use bios?
	je	_cnwrite_char	;CCK yes (for CGA or PGC)

	push	ds

	;figure out where to put the character
	mov	bx,6[bp]	;get position

	inc 	bx
	add	bx,start_screen	;add in start of screen offset

	dec	bx
	shl	bx,1		;two bytes per char
	and	bx,mem_mask	;wrap around at end of screen memory

	mov	ax,screen	;set ds to screen memory
	mov	ds,ax
	mov	ax,4[bp]	;get char, the attrib should have been passed
	mov	[bx],ax		;put the character in its place

	pop	ds
	pop	bp
	ret
;cnwrite_character(char,pos) - BIOS version of nwrite_char()
; doesn't change the cursor position

_cnwrite_char:
	push	si
	push	di
	;figure out where to put the character
	mov	dx,8[bp]	;get position
	mov	bh,display_page	;get current cursor position
	mov	ah,2		;set new cursor position
	int	video_bios

	mov	ax,4[bp]	;get char, the attrib should have been passed
	mov	bl,ah		;move attribute into bl
	mov	cx,1		;do only 1 character
	mov	ah,9
	int	video_bios

	mov	dx,savpos	;hopefully this WAS saved
	mov	ah,2		;set old cursor postion
	int	video_bios

	pop	di
	pop	si
	pop	bp
	ret

;write_character(char,pos) - write character char to position pos

_write_char:
	push	bp
	mov	bp,sp

	cmp	bios_mode,1	;CCK use bios?
	je	_cwrite_char	;CCK yes (for CGA or PGC)

	push	ds

	;figure out where to put the character
	mov	bx,6[bp]	;get position
	inc 	bx		;
	mov	savpos,bx       ;save cursor position so screen can move
	add	bx,start_screen	;add in start of screen offset
; excerpted from setcursor()
	mov	dx,index	;set 6845 to accept low cursor address
	mov	al,lowpos
	out	dx,al
	mov	dx,datareg	;write low cursor address
	mov	al,bl
	out	dx,al
	mov	dx,index	;set 6845 to accept high cursor address
	mov	al,highpos
	out	dx,al
	mov	al,bh		;write high cursor address
	mov	dx,datareg
	out	dx,al
; end of excerpt

	dec	bx
	shl	bx,1		;two bytes per char
	and	bx,mem_mask	;wrap around at end of screen memory

	mov	ax,screen	;set ds to screen memory
	mov	ds,ax
	mov	ax,4[bp]	;get char, the attrib should have been passed
	mov	[bx],ax		;put the character in its place

	pop	ds
	pop	bp
	ret


;cwrite_character(char,pos) - BIOS version of write_char()

_cwrite_char:
	push	si
	push	di
	;figure out where to put the character
	mov	dx,8[bp]	;get position
	mov	bh,display_page
	mov	ah,2		;set new cursor position
	int	video_bios

	mov	ax,4[bp]	;get char, the attrib should have been passed
	mov	bl,ah		;move attribute into bl
	mov	cx,1		;do only 1 character
	mov	ah,9
	int	video_bios

	;figure out where to put the character
	mov	dx,8[bp]	;get position
	inc	dx		;advance cursor to next column
	mov	savpos,dx	;save position
	mov	ah,2		;set new cursor position
	int	video_bios

	pop	di
	pop	si
	pop	bp
	ret

;set_cursor() - put cursor at the position given by the global variable _pos
_set_cursor:
	cmp	bios_mode,1	;CCK use bios?
	je	_cset_cursor	;CCK yes (for CGA or PGC)

	mov	ax,_pos		;get pos
	mov	savpos,ax	;save it

rst:
	add	ax,start_screen	;add in start of screen offset
	mov	bx,ax		;save cursor position

;send cursor address to 6845
	mov	dx,index	;set 6845 to accept low cursor address
	mov	al,lowpos
	out	dx,al
	mov	dx,datareg		;write low cursor address
	mov	al,bl
	out	dx,al
	mov	dx,index	;set 6845 to accept high cursor address
	mov	al,highpos
	out	dx,al
	mov	al,bh		;write high cursor address
	mov	dx,datareg
	out	dx,al

	ret

;cset_cursor - BIOS version of set_cursor()

_cset_cursor:
	push	bp
	push	si
	push	di
	mov	bx,_y_pos	;get position
	mov	dx,_x_pos
	mov	dh,bl
	mov	savpos,dx	;save it
crst:	mov	bh,display_page
	mov	ah,2		;set new cursor position
	int	video_bios

	pop	di
	pop	si
	pop	bp
	ret

_rset_cursor:
	cmp	bios_mode,1	;CCK use bios?
	je	_crset_cursor	;CCK yes (for CGA or PGC)

	mov	ax,savpos
	jmp	rst		;go put the cursor out again

_crset_cursor:
	push	bp
	push	si
	push	di
	mov	dx,savpos
	jmp	crst
	
; scr_restore() - restores the 6845
_scr_rest:
	ret
_TEXT	ENDS

_DATA	SEGMENT
savpos		DW	0	;cursor coordinate
screen		DW	0b000H	;screen refresh memory
mem_mask	DW	00fffH	;mask for 2k buffer of words
index		DW	03b4H	;6845 index register
datareg		DW	03b5H	;6845 data register
start_screen	DW	0	;where the start of the screen is in the
				;buffer memory
crt_mode	DB	0	;DDP current crt mode (color, bw, etc.)
bios_mode	DB	0	;CCK use bios
display_page	DB	0	;DDP current display page
_DATA	ENDS
	END
