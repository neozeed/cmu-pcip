/*
 *  Zilog 8530 Serial Communications Controller (SCC) register definitions
 *
 **********************************************************************
 * HISTORY
 * 22-Jul-86  Drew D. Perkins (ddp) at Carnegie-Mellon University
 *	Created.
 *
 **********************************************************************
 *
 */

/* WR0 definitions (Command Register) */
#define SCC_RST_RxCRCC	0x40	/* Reset Receive CRC Checker */
#define SCC_RST_TxCRCG	0x80	/* Reset Transmit CRC Generator */
#define SCC_RST_TxUNDER	0xC0	/* Reset Transmit Underrun/EOM Latch */
#define SCC_PNTHIGH	0x08	/* Point High */
#define SCC_RST_EXTINT	0x10	/* Reset External/Status Interrupts */
#define SCC_SENDABORT	0x18	/* Send Abort */
#define SCC_ENA_INTRxC	0x20	/* Enable Interrupt on Next Rx Character */
#define SCC_RST_TxINTP	0x28	/* Reset Tx Interrupt Pending */
#define SCC_ERROR_RST	0x30	/* Error Reset */
#define SCC_RST_HIUS	0x38	/* Reset Highest Interrupt Under Service */
#define SCC_WR0		0x00
#define SCC_WR1		0x01
#define SCC_WR2		0x02
#define SCC_WR3		0x03
#define SCC_WR4		0x04
#define SCC_WR5		0x05
#define SCC_WR6		0x06
#define SCC_WR7		0x07
#define SCC_WR8		(0x00 | SCC_PNTHIGH)
#define SCC_WR9		(0x01 | SCC_PNTHIGH)
#define SCC_WR10	(0x02 | SCC_PNTHIGH)
#define SCC_WR11	(0x03 | SCC_PNTHIGH)
#define SCC_WR12	(0x04 | SCC_PNTHIGH)
#define SCC_WR13	(0x05 | SCC_PNTHIGH)
#define SCC_WR14	(0x06 | SCC_PNTHIGH)
#define SCC_WR15	(0x07 | SCC_PNTHIGH)

/* WR1 definitions (Tx/Rx Interrupt and Data Txfer Mode Definitions) */
#define SCC_WAITDMA_ENA	0x80	/* Wait/DMA Request Enable */
#define SCC_WAITDMA_FUN	0x40	/* Wait/DMA Request Function */
#define SCC_WAITDMA_RTx	0x20	/* Wait/DMA Request Rx/Tx */
#define SCC_ENA_FRSTCHR	0x08	/* Rx Interrupt on First Character */
#define SCC_ENA_ALLCHR	0x10	/* Rx Interrupt on All Character */
#define SCC_ENA_SPECCND	0x18	/* Rx Interrupt on Special Condition */
#define SCC_ENA_PARITY	0x04	/* Parity is Special Condition */
#define SCC_ENA_TxINT	0x02	/* Tx Interrupt Enable */
#define SCC_ENA_EXTINT	0x01	/* External/Status Master Interrupt Enable */

/* WR2 definitions (Interrupt Vector) */

/* WR3 definitions (Receive Parameters and Control) */
#define SCC_Rx_5BITS	0x00	/* 5 bits/character */
#define SCC_Rx_7BITS	0x40	/* 7 bits/character */
#define SCC_Rx_6BITS	0x80	/* 6 bits/character */
#define SCC_Rx_8BITS	0xC0	/* 8 bits/character */
#define SCC_AUTO_ENA	0x20	/* Auto Enables */
#define SCC_ENTER_HUNT	0x10	/* Enter Hunt Mode */
#define SCC_RxCRC_ENA	0x08	/* Receiver CRC Enable */
#define SCC_ADDR_SRCH	0x04	/* Address Search Mode (SDLC) */
#define SCC_SCL_INHIBIT	0x02	/* Sync Character Load Inhibit */
#define SCC_Rx_ENA	0x01	/* Receiver Enable */

/* WR4 definitions (Tx/Rx Miscellaneous Parameters and Modes) */
#define SCC_CLK_x1	0x00	/* x1 Clock Mode */
#define SCC_CLK_x16	0x40	/* x16 Clock Mode */
#define SCC_CLK_x32	0x80	/* x32 Clock Mode */
#define SCC_CLK_x64	0xC0	/* x64 Clock Mode */
#define SCC_SYNC_8	0x00	/* 8 Bit Sync Character (Monosync) */
#define SCC_SYNC_16	0x10	/* 16 Bit Sync Character (Bisync) */
#define SCC_SYNC_SDLC	0x20	/* SDLC Mode */
#define SCC_SYNC_EXT	0x30	/* External Sync Mode */
#define SCC_SYNC_ENA	0x00	/* Sync Modes Enable */
#define SCC_1STOP	0x04	/* 1 Stop Bit/Character */
#define SCC_1PNT5STOP	0x08	/* 1.5 Stop Bit/Character */
#define SCC_2STOP	0x0C	/* 2 Stop Bit/Character */
#define SCC_PARITY_EVEN	0x02	/* Even Parity */
#define SCC_PARITY_ODD	0x00	/* Odd Parity */
#define SCC_PARITY_ENA	0x01	/* Parity Enable */

/* WR5 definitions (Tx Parameters and Controls) */
#define SCC_DTR		0x80	/* DTR */
#define SCC_Tx_5BITS	0x00	/* 5 bits/character */
#define SCC_Tx_7BITS	0x20	/* 7 bits/character */
#define SCC_Tx_6BITS	0x40	/* 6 bits/character */
#define SCC_Tx_8BITS	0x60	/* 8 bits/character */
#define SCC_SEND_BREAK	0x10	/* Send Break */
#define SCC_Tx_ENA	0x08	/* Transmit Enable */
#define SCC_POLY_SDLC	0x00	/* SDLC Polynomial */
#define SCC_POLY_CRC16	0x04	/* CRC-16 Polynomial */
#define SCC_RTS		0x02	/* RTS */
#define SCC_TxCRC_ENA	0x01	/* Transmit CRC Enable */

/* WR6 definitions (Sync Characters or SDLC Address Field) */

/* WR7 definitions (Sync Characters or SDLC Flag) */

/* WR8 definitions (Trasmit Buffer) */

/* WR9 definitions (Master Interrupt Control) */
#define SCC_RST_CHB	0x40	/* Channel Reset B */
#define SCC_RST_CHA	0x80	/* Channel Reset A */
#define SCC_RST_HARD	0xC0	/* Force Hardware Reset */
#define SCC_STATUS_HL	0x10	/* Status High/Low */
#define SCC_MIE		0x08	/* Master interrupt Enable */
#define SCC_DLC		0x04	/* Disable Lower Chain */
#define SCC_NOVECTOR	0x02	/* No Vector */
#define SCC_VIS		0x01	/* Vector Include Status */

/* WR10 definitions (Miscellaneous Trasmit/Receive Control Bits) */
#define SCC_CRCPRESET	0x80	/* CRC Preset I/O */
#define SCC_NRZ		0x00	/* NRZ */
#define SCC_NRZI	0x40	/* NRZI */
#define SCC_FM1		0x80	/* FM1 */
#define SCC_FM0		0xC0	/* FM0 */
#define SCC_GOACTIVE	0x10	/* Go Active On Poll */
#define SCC_MARK	0x08	/* Mark Idle */
#define SCC_FLAG	0x00	/* Flag Idle */
#define SCC_ABORT	0x04	/* Abort/Flag on Underrun */
#define SCC_LOOP	0x02	/* Loop Mode */
#define SCC_6BIT_SYNC	0x01	/* 6/8 Bit Sync */

/* WR11 definitions (Clock Mode Control) */
#define SCC_RTxC_XTAL	0x80	/* RTxC Xtal/No Xtal */
#define SCC_RxC_RTxC	0x00	/* Receive Clock = RTxC pin */
#define SCC_RxC_TRxC	0x20	/* Receive Clock = TRxC pin */
#define SCC_RxC_BRGO	0x40	/* Receive Clock = BR Generator Output */
#define SCC_RxC_DPLL	0x60	/* Receive Clock = DPLL Output */
#define SCC_TxC_RTxC	0x00	/* Transmit Clock = RTxC pin */
#define SCC_TxC_TRxC	0x08	/* Transmit Clock = TRxC pin */
#define SCC_TxC_BRGO	0x10	/* Transmit Clock = BR Generator Output */
#define SCC_TxC_DPLL	0x18	/* Transmit Clock = DPLL Output */
#define SCC_TRxC_OUTPUT	0x04	/* TRxC Output/Input */
#define SCC_TRxC_XTAL	0x00	/* Transmit Clock = RTxC pin */
#define SCC_TRxC_TxC	0x01	/* Transmit Clock = TRxC pin */
#define SCC_TRxC_BRGO	0x02	/* Transmit Clock = BR Generator Output */
#define SCC_TRxC_DPLL	0x03	/* Transmit Clock = DPLL Output */

/* WR12 definitions (Lower Byte of Baud Rate Generator Time Constant) */

/* WR13 definitions (Upper Byte of Baud Rate Generator Time Constant) */

/* WR14 definitions (Miscellaneous Control Bits) */
#define SCC_NRZI_MODE	0xE0	/* Set NRZI Mode */
#define SCC_FM_MODE	0xC0	/* Set FM Mode */
#define SCC_SOURCE_RTxC	0xA0	/* Set Source = RTxC */
#define SCC_SOURCE_BRG	0x80	/* Set Source = BR Generator */
#define SCC_DIS_DPLL	0x60	/* Disable DPLL */
#define SCC_RST_MISSCLK	0x40	/* Reset Missing Clock */
#define SCC_SEARCH_MODE	0x20	/* Enter Search Mode */
#define SCC_LOOPBACK	0x10	/* Local Loopback */
#define SCC_AUTO_ECHO	0x08	/* Auto Echo */
#define SCC_DTR_REQ	0x04	/* DTR/Request Function */
#define SCC_BRG_SOURCE	0x02	/* BR Generator Source */
#define SCC_BRG_ENA	0x01	/* BR Generator Enable */

/* WR15 definitions (External/Status Interrupt Control) */
#define SCC_ENA_BREAK	0x80	/* Break/Abort IE */
#define SCC_ENA_TxUNDER	0x40	/* Tx Underrun/EOM IE */
#define SCC_ENA_CTS	0x20	/* CTS IE */
#define SCC_ENA_SYNC	0x10	/* Sync/Hunt IE */
#define SCC_ENA_DCD	0x08	/* DCD IE */
#define SCC_ENA_ZERO	0x02	/* Zero Count IE */

/* RR0 definitions (Transmit/Receive Buffer Status and External Status) */
#define SCC_BREAK	0x80	/* Break/Abort */
#define SCC_TxUNDER	0x40	/* Tx Underrun/EOM */
#define SCC_CTS		0x20	/* CTS */
#define SCC_SYNC	0x10	/* Sync/Hunt */
#define SCC_DCD		0x08	/* DCD */
#define SCC_TxBUF_EMPTY	0x04	/* Tx Buffer Empty */
#define SCC_ZERO	0x02	/* Zero Count */
#define SCC_RxCHR_AVAIL	0x01	/* Rx Character Available */

/* RR1 definitions (Special Receive Status Bits) */
#define SCC_EOF		0x80	/* End of Frame (SDLC) */
#define SCC_CRC_FRAMING	0x40	/* CRC/Framing Error */
#define SCC_Rx_OVERRUN	0x20	/* Rx Overrun Error */
#define SCC_PARITY	0x10	/* Parity Error */
#define SCC_RES_CODE0	0x08	/* Residue COde 0 */
#define SCC_RES_CODE1	0x04	/* Residue COde 1 */
#define SCC_RES_CODE2	0x02	/* Residue COde 2 */
#define SCC_ALL_SENT	0x01	/* All Sent */

/* RR2 definitions (Interrupt Vector) */

/* RR3 definitions (Interrupt Pending Register) */
#define SCC_CHNA_RxIP	0x20	/* Channel A Rx IP */
#define SCC_CHNA_TxIP	0x10	/* Channel A Tx IP */
#define SCC_CHNA_ESIP	0x08	/* Channel A Ext/Stat IP */
#define SCC_CHNB_RxIP	0x04	/* Channel B Rx IP */
#define SCC_CHNB_TxIP	0x02	/* Channel B Tx IP */
#define SCC_CHNB_ESIP	0x01	/* Channel B Ext/Stat IP */

/* RR8 definitions (Receive Data Register) */

/* RR10 definitions (Miscellaneous Status Bits) */
#define SCC_ONE_CLK_MIS	0x80	/* One Clock Missing */
#define SCC_TWO_CLK_MIS	0x40	/* Two Clock Missing */
#define SCC_LOOP_SEND	0x10	/* Loop Sending */
#define SCC_ON_LOOP	0x02	/* On Loop */

/* RR12 definitions (Lower Byte of Baud Rate Generator Time Constant) */

/* RR13 definitions (Upper Byte of Baud Rate Generator Time Constant) */

/* WR15 definitions (External/Status Interrupt Control) */
