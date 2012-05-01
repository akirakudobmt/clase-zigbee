/*****************************************************************************
* IIC Serial Port declarations.
*
* (c) Copyright 2007, Freescale, Inc. All rights reserved.
*
*
* No part of this document must be reproduced in any form - including copied,
* transcribed, printed or by any electronic means - without specific written
* permission from Freescale Semiconductor.
*
*****************************************************************************/

#ifndef _IIC_h
#define _IIC_h





/*****************************************************************************
******************************************************************************
* Private macros
******************************************************************************
****************************************************************************/

/* Events for IIC task */
#define gIIC_Event_SlaveRx_c          (1<<2)
#define gIIC_Event_SlaveTx_c          (1<<3)
#define gIIC_Event_MasterRxFail_c     (1<<4)
#define gIIC_Event_MasterRxSuccess_c  (1<<5)
#define gIIC_Event_MasterTxFail_c     (1<<6)
#define gIIC_Event_MasterTxSuccess_c  (1<<7)


/* Define the IIC ports */
#if defined(PROCESSOR_QE128)
/* QE128 */
#define mIIC_A_c IIC2A
#define mIIC_F_c IIC2F
#define mIIC_C_c IIC2C1
#define mIIC_S_c IIC2S
#define mIIC_D_c IIC2D

#elif defined(PROCESSOR_MC1323X)

#define mIIC_A_c IIC1A
#define mIIC_F_c IIC1F
#define mIIC_C_c IIC1C1
#define mIIC_S_c IIC1S
#define mIIC_D_c IIC1D


#else
/* HCS08 */
#define mIIC_A_c IICA
#define mIIC_F_c IICF
#define mIIC_C_c IICC
#define mIIC_S_c IICS
#define mIIC_D_c IICD

#endif


/* Bits in IIC Control Register. Read/Write. */
/* IICC at 0x005A. */
#define mIICC_IICEN_c       0x80        /* IIC Enable */
#define mIICC_IICIE_c       0x40        /* IIC Interrupt Enable */
#define mIICC_MST_c         0x20        /* Master Mode Select */
#define mIICC_TX_c          0x10        /* Transmit Mode Select */
#define mIICC_TXAK_c        0x08        /* Transmit Acknowledge Enable */
#define mIICC_RSTA_c        0x04        /* Repeat START */

/* Bits in IIC Status Register. Read/Write. */
/* IIC1S at 0x005B. */
#define mIICS_TCF_c         0x80        /* Transfer Complete Flag */
#define mIICS_IAAS_c        0x40        /* Addressed as a Slave */
#define mIICS_BUSY_c        0x20        /* Bus Busy Flag */
#define mIICS_ARBL_c        0x10        /* Arbitration Lost */
#define mIICS_SRW_c         0x04        /* Slave Read/Write Flag */
#define mIICS_IICIF_c       0x02        /* IIC Interrupt Flag */
#define mIICS_RXAK_c        0x01        /* Receive Acknowledge */




/* Initialize IIC Control Register so that:
*   - IIC is enabled
*   - configured for Slave Mode */
#define mIICC_Init_c        (mIICC_IICEN_c | mIICC_IICIE_c)

#if defined(PROCESSOR_MC1323X)
/* Initialize IICxC2 Control Register so that:
*   - General call address is disabled
*   - 7 bit address scheme  */
#define mIICxC2_Init_c        0
#endif

/* Initialize IIC Status Register so that:
*   - Busy flag is cleared
*   - R/W bit = 0
*   - Interrupt flag is cleared
*   - Arbitration Lost flag is cleared */
#define mIICS_Init_c        (mIICS_ARBL_c | mIICS_IICIF_c)

/* Turn everything off. */
#define mIICC_Reset_c     0x00

/* Number of elements in an array. */
#ifndef NumberOfElements
#define NumberOfElements(array)     ((sizeof(array) / (sizeof(array[0]))))
#endif

#endif    /* _IIC_h */
