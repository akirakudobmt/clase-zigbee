/******************************************************************************
* SPI Serial Port driver declaration. 
*
* Copyright (c) 2010, Freescale, Inc. All right reserved
*
*
* No part of this document must be reproduced in any form - including copied, 
* transcribed, printed or by any electronic means - vithout specific written
* permission from Freescale Semicondictor.
*  
******************************************************************************/

#ifndef _SPI_h_
#define _SPI_h_

/******************************************************************************
*******************************************************************************
* Private macros
*******************************************************************************
******************************************************************************/

/* Events for SPI task */
#define gSPI_Event_MasterTxSuccess_c  (1<<1)
#define gSPI_Event_MasterTxFail_c     (1<<2)
#define gSPI_Event_MasterRxSuccess_c  (1<<3)
#define gSPI_Event_MasterRxFail_c     (1<<4)
#define gSPI_Event_SlaveTx_c          (1<<5)
#define gSPI_Event_SlaveRx_c          (1<<6)

/* Bits in SPI Control Register 1. Read/Write. */
#define mSPIxC1_LSBFE_c               (1<<0)      /* LSB First (Shift Direction) */
#define mSPIxC1_SSOE_c                (1<<1)      /* Slave Select Output Enable */
#define mSPIxC1_CPHA_c                (1<<2)      /* Clock Phase */
#define mSPIxC1_CPOL_c                (1<<3)      /* Clock Polarity */
#define mSPIxC1_MSTR_c                (1<<4)      /* Master/Slave Mode Select */
#define mSPIxC1_SPTIE_c               (1<<5)      /* SPI Transmit Enable */
#define mSPIxC1_SPE_c                 (1<<6)      /* SPI System Enable */
#define mSPIxC1_SPIE_c                (1<<7)      /* SPI Interrupt Enable (for SPRF and MODF) */

/* Bits in SPI Control Register 2. Read/Write. */
#define mSPIxC2_SPC0_c                (1<<0)      /* SPI Pin Control 0 */
#define mSPIxC2_SPISWAI_c             (1<<1)      /* SPI Stop in Wait Mode */
#define mSPIxC2_BIDIROE_c             (1<<3)      /* Bidirectional Mode Output Enable */
#define mSPIxC2_MODFEN_c              (1<<4)      /* Master Mode-Fault Function Enable */

/* Bits in SPI Status Register. Read only. */
#define mSPIxS_MODF_c                 (1<<4)      /* Master Mode Fault Flag */
#define mSPIxS_SPTEF_c                (1<<5)      /* SPI Transmit Buffer Empty Flag */
#define mSPIxS_SPRF_c                 (1<<7)      /* SPI Read Buffer Full Flag */

/* Bits in SPI Baud Rate Register. Read/Write. */
#define mSPIxBR_SPR0_c                (1<<0)      /* SPI Baud Rate Divisor, bit 0 */
#define mSPIxBR_SPR1_c                (1<<1)      /* SPI Baud Rate Divisor, bit 1 */
#define mSPIxBR_SPR2_c                (1<<2)      /* SPI Baud Rate Divisor, bit 2 */
#define mSPIxBR_SPPR0_c               (1<<4)      /* SPI Baud Rate Prescaler Divisor, bit 0 */
#define mSPIxBR_SPPR1_c               (1<<5)      /* SPI Baud Rate Prescaler Divisor, bit 1 */
#define mSPIxBR_SPPR2_c               (1<<6)      /* SPI Baud Rate Prescaler Divisor, bit 2 */

/* Bits in SPI Data Register. Read/Write. */
#define mSPIxD_DATA0_c                (1<<0)      /* SPI Data 0 */
#define mSPIxD_DATA1_c                (1<<1)      /* SPI Data 1 */
#define mSPIxD_DATA2_c                (1<<2)      /* SPI Data 2 */
#define mSPIxD_DATA3_c                (1<<3)      /* SPI Data 3 */
#define mSPIxD_DATA4_c                (1<<4)      /* SPI Data 4 */
#define mSPIxD_DATA5_c                (1<<5)      /* SPI Data 5 */
#define mSPIxD_DATA6_c                (1<<6)      /* SPI Data 6 */
#define mSPIxD_DATA7_c                (1<<7)      /* SPI Data 7 */

/* Initialize the SPI Control Register 1                        */
/*  - SPI Enabled, slave mode, MSB first bitwise shifting       */
/*  - Enable interrupts: both for Rx and Tx                     */
/*  - Clock polarity and phase: active high, odd edge shifting  */
#define mSPIxC1_Init_c                (mSPIxC1_SPE_c | mSPIxC1_CPHA_c)

/* Initialize the SPI Control Register 2                */
/* - Full-duplex mode, SPI clock continue in wait mode  */
/* - Master Mode Fault disabled                         */
#define mSPIxC2_Init_c                (0)

/* Turn everything off */
#define mSPIxC1_Reset_c               (0x04)
#define mSPIxC2_Reset_c               (0)

#endif   /* _SPI_h_ */