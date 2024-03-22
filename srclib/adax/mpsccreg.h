/*
 *  NEC 7201 Multiprotocol Serial Communications Controller (MPSCC)
 *	 register definitions
 *
 **********************************************************************
 * HISTORY
 * 06-Jan-86  Drew D. Perkins (ddp) at Carnegie-Mellon University
 *	Created.
 *
 **********************************************************************
 *
 */

/* CR0 definitions (Command Register) */
#define MPSCC_RST_RxCRCC	0x40	/* Reset Receive CRC Checker */
#define MPSCC_RST_TxCRCG	0x80	/* Reset Transmit CRC Generator */
#define MPSCC_RST_TxUNDER	0xC0	/* Reset Transmit Underrun/EOM Latch */
#define MPSCC_NULL		0x00	/* NULL Command */
#define MPSCC_SENDABORT		0x08	/* Send Abort */
#define MPSCC_RST_EXTINT	0x10	/* Reset External/Status Interrupts */
#define MPSCC_RST_CHANNEL	0x18	/* Reset External/Status Interrupts */
#define MPSCC_ENA_INTRxC	0x20	/* Enable Interrupt on Next Rx Character */
#define MPSCC_RST_TxINTP	0x28	/* Reset Tx Interrupt Pending */
#define MPSCC_ERROR_RST		0x30	/* Error Reset (Channel A only) */
#define MPSCC_END_OF_INT	0x38	/* Reset Highest Interrupt Under Service (Channel A only) */
#define MPSCC_CR0		0x00
#define MPSCC_CR1		0x01
#define MPSCC_CR2		0x02
#define MPSCC_CR3		0x03
#define MPSCC_CR4		0x04
#define MPSCC_CR5		0x05
#define MPSCC_CR6		0x06
#define MPSCC_CR7		0x07

/* CR1 definitions (Interrupt Control) */
#define MPSCC_WAITDMA_ENA	0x80	/* Wait/DMA Request Enable */
#define MPSCC_ENA_BCNT_MODE	0x40	/* Enable Byte Count Mode */
#define MPSCC_WAITDMA_RTx	0x20	/* Wait/DMA Request Rx/Tx */
#define MPSCC_ENA_FRSTCHR	0x08	/* Rx Interrupt on First Character */
#define MPSCC_ENA_ALLCHR	0x10	/* Rx Interrupt on All Character */
#define MPSCC_ENA_ALLCHR_NP	0x18	/* Rx Interrupt on All Characters */
#define MPSCC_CAV		0x04	/* Condition Affects Vector (Channel B only) */
#define MPSCC_ENA_TxINT		0x02	/* Tx Interrupt Enable */
#define MPSCC_ENA_EXTINT	0x01	/* External/Status Master Interrupt Enable */

/* CR2 (Channel A) definitions (Processor/Bus Interface Control) */
#define MPSCC_DMA_MODE0		0x00	/* DMA Mode 0 (Non-DMA) */
#define MPSCC_DMA_MODE1		0x01	/* DMA Mode 1 (DMA/Non-DMA) */
#define MPSCC_DMA_MODE2		0x02	/* DMA Mode 2 (DMA/HAI) */
#define MPSCC_DMA_MODE3		0x03	/* DMA Mode 3 (DMA/DTR) */
#define MPSCC_DMA_PRI0		0x00	/* DMA/INT Priority 0 */
#define MPSCC_DMA_PRI1		0x04	/* DMA/INT Priority 1 */
#define MPSCC_IASR_MODE0	0x00	/* INT Ack Sequence Response */
#define MPSCC_IASR_MODE1	0x08	/* INT Ack Sequence Response */
#define MPSCC_IASR_MODE2	0x10	/* INT Ack Sequence Response */
#define MPSCC_IASR_MODE3	0x18	/* INT Ack Sequence Response */
#define MPSCC_IASR_MODE4	0x20	/* INT Ack Sequence Response */
#define MPSCC_IASR_MODE5	0x28	/* INT Ack Sequence Response */
#define MPSCC_IASR_MODE6	0x30	/* INT Ack Sequence Response */
#define MPSCC_IASR_MODE7	0x38	/* INT Ack Sequence Response */
#define MPSCC_RxINT_MASK	0x40	/* Mask Rx Ints but not DMA */
#define MPSCC_ENA_SYNCB		0x80	/* Enable SYNCB vs. RTSB on pin 10 */

/* CR2 (Channel B) definitions (Interrupt Vector) */

/* CR3 definitions (Receiver Control) */
#define MPSCC_Rx_5BITS		0x00	/* 5 bits/character */
#define MPSCC_Rx_7BITS		0x40	/* 7 bits/character */
#define MPSCC_Rx_6BITS		0x80	/* 6 bits/character */
#define MPSCC_Rx_8BITS		0xC0	/* 8 bits/character */
#define MPSCC_AUTO_ENA		0x20	/* Auto Enables */
#define MPSCC_ENTER_HUNT	0x10	/* Enter Hunt Mode */
#define MPSCC_RxCRC_ENA		0x08	/* Receiver CRC Enable */
#define MPSCC_ADDR_SRCH		0x04	/* Address Search Mode (SDLC) */
#define MPSCC_SCL_INHIBIT	0x02	/* Sync Character Load Inhibit */
#define MPSCC_Rx_ENA		0x01	/* Receiver Enable */

/* CR4 definitions (Mode Control) */
#define MPSCC_CLK_x1		0x00	/* x1 Clock Mode */
#define MPSCC_CLK_x16		0x40	/* x16 Clock Mode */
#define MPSCC_CLK_x32		0x80	/* x32 Clock Mode */
#define MPSCC_CLK_x64		0xC0	/* x64 Clock Mode */
#define MPSCC_SYNC_8		0x00	/* 8 Bit Sync Character (Monosync) */
#define MPSCC_SYNC_16		0x10	/* 16 Bit Sync Character (Bisync) */
#define MPSCC_SYNC_SDLC		0x20	/* SDLC Mode */
#define MPSCC_SYNC_EXT		0x30	/* External Sync Mode */
#define MPSCC_SYNC_ENA		0x00	/* Sync Modes Enable */
#define MPSCC_1STOP		0x04	/* 1 Stop Bit/Character */
#define MPSCC_1PNT5STOP		0x08	/* 1.5 Stop Bit/Character */
#define MPSCC_2STOP		0x0C	/* 2 Stop Bit/Character */
#define MPSCC_PARITY_EVEN	0x02	/* Even Parity */
#define MPSCC_PARITY_ODD	0x00	/* Odd Parity */
#define MPSCC_PARITY_ENA	0x01	/* Parity Enable */

/* CR5 definitions (Transmitter Control) */
#define MPSCC_DTR		0x80	/* DTR */
#define MPSCC_Tx_5BITS		0x00	/* 5 bits/character */
#define MPSCC_Tx_7BITS		0x20	/* 7 bits/character */
#define MPSCC_Tx_6BITS		0x40	/* 6 bits/character */
#define MPSCC_Tx_8BITS		0x60	/* 8 bits/character */
#define MPSCC_SEND_BREAK	0x10	/* Send Break */
#define MPSCC_Tx_ENA		0x08	/* Transmit Enable */
#define MPSCC_POLY_SDLC		0x00	/* SDLC Polynomial */
#define MPSCC_POLY_CRC16	0x04	/* CRC-16 Polynomial */
#define MPSCC_RTS		0x02	/* RTS */
#define MPSCC_TxCRC_ENA		0x01	/* Transmit CRC Enable */

/* CR6 definitions (Sync Character 1 or SDLC Address Field) */

/* CR7 definitions (Sync Character 2 or SDLC Flag) */

/* SR0 definitions (Transmit/Receive Buffer Status and External Status) */
#define MPSCC_BREAK		0x80	/* Break/Abort */
#define MPSCC_TxUNDER		0x40	/* Tx Underrun/EOM */
#define MPSCC_CTS		0x20	/* CTS */
#define MPSCC_SYNC		0x10	/* Sync/Hunt */
#define MPSCC_DCD		0x08	/* DCD */
#define MPSCC_TxBUF_EMPTY	0x04	/* Tx Buffer Empty */
#define MPSCC_INT_PENDING	0x02	/* Interrupt Pending (Channel A only) */
#define MPSCC_RxCHR_AVAIL	0x01	/* Rx Character Available */

/* SR1 definitions (Special Receive Status Bits) */
#define MPSCC_EOF		0x80	/* End of Frame (SDLC) */
#define MPSCC_CRC_FRAMING	0x40	/* CRC/Framing Error */
#define MPSCC_Rx_OVERRUN	0x20	/* Rx Overrun Error */
#define MPSCC_PARITY		0x10	/* Parity Error */
#define MPSCC_RES_CODE0		0x08	/* Residue Code 0 */
#define MPSCC_RES_CODE1		0x04	/* Residue Code 1 */
#define MPSCC_RES_CODE2		0x02	/* Residue Code 2 */
#define MPSCC_ALL_SENT		0x01	/* All Sent */

/* SR2 (Channel B) definitions (Interrupt Vector) */
#define MPSCC_CHNB_TxIP		0x00	/* Channel B Tx IP */
#define MPSCC_CHNB_ESIP		0x01	/* Channel B Ext/Stat IP */
#define MPSCC_CHNB_RxIP		0x02	/* Channel B Rx IP */
#define MPSCC_CHNB_SRxIP	0x03	/* Channel B Special Rx IP */
#define MPSCC_CHNA_TxIP		0x04	/* Channel A Tx IP */
#define MPSCC_CHNA_ESIP		0x05	/* Channel A Ext/Stat IP */
#define MPSCC_CHNA_RxIP		0x06	/* Channel A Rx IP */
#define MPSCC_CHNA_SRxIP	0x07	/* Channel A Special Rx IP */
