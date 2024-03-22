/* Copyright 1986 by Carnegie Mellon */
/* screen size and buffer positions */
#define BUFFSIZE  1920
#define MINPOS       0
#define MAXPOS    1919
#define ATTRSTOP  1920
#define ATTRCTRL  1921
#define FIRSTINFO 1920
#define KEYINFO   1929
#define PRGINFO   1931
#define INPOS     1980
#define LASTINFO  1999
 
#define wrap(baddr) (baddr == MAXPOS) ? MINPOS : baddr + 1
#define unwrap(baddr) (baddr == MINPOS) ? MAXPOS : baddr - 1
 
#define LSIZE  80
/* WCC codes */
#define SND_ALARM 04
#define KYBDRESET 02
#define RESETMDT  01
 
 
/* Field Attribute Character Bit Assignment
 
	Bit             Field Descriptor
 
	0,1             Value determined by contents of bits 2-7
 
	2               0 = Unprotected
			1 = Protected
 
	3               0 = Alphanumeric
			1 = Numeric
 
			Note: Bits 2 and 3 equal to 11 causes an
			automatic skip.
 
	4,5             00 = Display/not selector-light-pen
			     detectable.
			01 = DIsplay/selector-light-pen detectable.
			10 = Intensified display/selector-light-pen
			     detectable.
			11 = Nondisplay, nonprint, nondetectable.
 
	6               Reserved. Must always be 0.
	7               Modified Data Tag (MDT); identifies
			modified fields during Read Modified
			command operations.
 
			0 = Field has not been modified.
			1 = Field has been modified by the
			    operator. Can also be set by pro-
			    gram in data stream.
*/
/* attribute byte flags */
#define PROTECT  0x20
#define ALPHANUM 0x10
#define AUTOSKIP 0x30
#define DISP0    0x08
#define DISP1    0x04
#define ARESERVE 0x02
#define MDT      0x01
 
 
/* 327x orders  in EBC , so do not translate !!! */
#define SF       0x1d           /* Start Field                  */
#define SBA      0x11           /* Set Buffer Address           */
#define IC       0x13           /* Insert Cursor                */
#define PT       0x05           /* Program Tab                  */
#define RA       0x3c           /* Repeat to Address            */
#define EUA      0x12           /* Erase Unprotected to Address */
 
 
/* AID characters  in EBC, so do not translate !!!*/
#define NO_AID   0x60
#define ENTER    0x7d
#define PF1      0xf1
#define PF2      0xf2
#define PF3      0xf3
#define PF4      0xf4
#define PF5      0xf5
#define PF6      0xf6
#define PF7      0xf7
#define PF8      0xf8
#define PF9      0xf9
#define PF10     0x7a
#define PF11     0x7b
#define PF12     0x7c
#define PF13     0xc1
#define PF14     0xc2
#define PF15     0xc3
#define PF16     0xc4
#define PF17     0xc5
#define PF18     0xc6
#define PF19     0xc7
#define PF20     0xc8
#define PF21     0xc9
#define PF22     0x4a
#define PF23     0x4b
#define PF24     0x4c
#define SELPEN   0x7e
#define PA1      0x6c
#define PA2      0x6e
#define PA3      0x6b
#define CLEAR    0x6d
#define TESTREQ  0xf0
 
 
/* this is for 3272 terminal  in EBC, so do not translate !!!  */
#define EAU             0x0f            /* Erase All Unprotected        */
#define EW              0x05            /* Erase/Write                  */
#define EWA             0x0d            /* Erase/Write Alternate        */
#define RDBUFFER        0x02            /* Read Buffer                  */
#define RDMOD           0x06            /* Read Modified                */
#define WRT             0x01            /* Write                        */
 
 
/* host data stream info */
#define MAXDATA 4096
/* colors */
#define GREEN   0x00
#define RED     0x01
#define BLUE    0x02
#define WHITE   0x03
#define HIDDEN  0x04
#define NOCOLOR 0x7f
 
 
/* useful ASCII characters */
#define NULL    0x00
#define BELL    0x07
#define ESC     0x1b
 
 
/* emulator workarea */
struct workarea {
   int bufpos;         /* buffer position for writes  */
   int attrpos;        /* current attribute postition */
   int curpos;         /* cursor position             */
   char ctlflag;       /* flags, see below            */
   char aid;
   char wcc;
   unsigned char buffer[BUFFSIZE+1+LSIZE];      /* Where actual data goes */
   unsigned char attr_buf[BUFFSIZE+1+LSIZE];     /* Where attribute goes   */
   };
 
 
/* flags for workarea.ctlflag */
#define KEYLOCKD 0x80     /* keyboard unlocked */
#define INSMODE  0x40     /* insert mode       */
#define REFRESH  0x20     /* must refresh screen */
#define REFRINFO 0x10     /* must refresh info line */
#define WRTALARM 0x08     /* need to sound alarm */
#define ORDER    0x01     /* last thing done was an order */
 
 
/* return codes from SCREENIO */
#define NEWAID   1        /* user generated attention */
#define PACKETIN 2        /* incoming packet queued */
#define ESCAPED  3        /* user hit ESCAPE     */
 
 
/* identification codes for packets */
#define USERINT   '1'     /* user generated interrupt */
#define RESPONSE  '2'     /* command response         */
#define CMD327X   '5'     /* 327x command & data      */
 
 
/* packet format */
struct packet {
   char ptype;     /* identification code (see above) */
   char cmd;       /* 327x command code (if 327XCMD)  */
   char data[4090];/* 327x command stream data        */
   };
 
#define PHDRL 1    /* length of ptype in packet       */
