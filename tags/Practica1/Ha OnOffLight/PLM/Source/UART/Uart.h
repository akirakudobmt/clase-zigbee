/*****************************************************************************
* UART / SCI / USB / Serial Port declarations.
*
* (c) Copyright 2006, Freescale, Inc. All rights reserved.
*
*
* No part of this document must be reproduced in any form - including copied,
* transcribed, printed or by any electronic means - without specific written
* permission from Freescale Semiconductor.
*
*****************************************************************************/

#ifndef _Uart_h
#define _Uart_h

/*****************************************************************************
******************************************************************************
* Private macros
******************************************************************************
*****************************************************************************/

/* Bits in SCI Control Register 1. Read/Write. */
/* SCI1C1 at 0x001A, SCI2C1 at 0x0022. */
#define mSCIxC1_LOOPS_c     0x80        /* Loop Mode Select. */
#define mSCIxC1_SCISWAI_c   0x40        /* SCI Stops in Wait Mode. */
#define mSCIxC1_RSRC_c      0x20        /* Receiver Source Select. */
#define mSCIxC1_M_c         0x10        /* Mode Select (8 bit or 9 bit). */
#define mSCIxC1_WAKE_c      0x08        /* Receiver Wakeup Method Select. */
#define mSCIxC1_ILT_c       0x04        /* Idle Line Type Select. */
#define mSCIxC1_PE_c        0x02        /* Parity Enable. */
#define mSCIxC1_PT_c        0x01        /* Parity Type. */

/* Bits in SCI Control Register 2. Read/Write. */
/* SCI1C2 at 0x001B, SCI2C2 at 0x0023. */
#define mSCIxC2_TIE_c       0x80        /* Transmit Interrupt Enable. */
#define mSCIxC2_TCIE_c      0x40        /* Transmission Complete Interrupt Enable. */
#define mSCIxC2_RIE_c       0x20        /* Receiver Interrupt Enable. */
#define mSCIxC2_ILIE_c      0x10        /* Idle Line Interrupt Enable. */
#define mSCIxC2_TE_c        0x08        /* Transmitter Enable. */
#define mSCIxC2_RE_c        0x04        /* Receiver Enable. */
#define mSCIxC2_RWU_c       0x02        /* Receiver Wakeup Control. */
#define mSCIxC2_SBK_c       0x01        /* Send Break. */

/* Bits in SCI Control Register 3. Read/Write. */
/* SCI1C3 at 0x001E, SCI2C3 at 0x0026. */
#define mSCIxC3_R8_c        0x80        /* 9th Data Bit for Receiver. */
#define mSCIxC3_T8_c        0x40        /* 9th Data Bit for Transmitter. */
#define mSCIxC3_TXDIR_c     0x20        /* TxD1 Pin Direction. */
#define mSCIxC3_Bit4_c      0x10        /* Not used. */
#define mSCIxC3_ORIE_c      0x08        /* Overrun Interrupt Enable. */
#define mSCIxC3_NEIE_c      0x04        /* Noise Error Interrupt Enable. */
#define mSCIxC3_FEIE_c      0x02        /* Framing Error Interrupt Enable. */
#define mSCIxC3_PEIE_c      0x01        /* Parity Error Interrupt Enable. */

/* Bits in SCI Status Register 1. Read only. */
/* SCI1S1 at 0x001C, SCI2S1 at 0x0024. */
#define mSCIxS1_TDRE_c      0x80        /* Transmit Data Register Empty Flag. */
#define mSCIxS1_TC_c        0x40        /* Transmit Complete Flag. */
#define mSCIxS1_RDRF_c      0x20        /* Receive Data Register Full Flag. */
#define mSCIxS1_IDLE_c      0x10        /* Idle Line Flag. */
#define mSCIxS1_OR_c        0x08        /* Receiver Overrun Flag. */
#define mSCIxS1_NF_c        0x04        /* Noise Flag. */
#define mSCIxS1_FE_c        0x02        /* Framing Error Flag. */
#define mSCIxS1_PF_c        0x01        /* Parity Error Flag. */

/* Bits in SCI Status Register 2. Read only. */
/* SCI1S2 at 0x001D, SCI2S2 at 0x0025. Bits 7-1 are not defined. */
#define mSCIxS2_RAF_c       0x01        /* Receiver Active Flag. */

/*****************************************************************************/

/* Loop Mode off, SCI Clocks continue to run in wait mode, start bit + 8 */
/* data bits, Idle line wakeup, no hardware parity checking,. */
#define mSCI1C1_Init_c      0x00
#define mSCI2C1_Init_c      0x00

/* Transmitter enable, Receiver enable. */
/* The transmitter remains idle until data is written to the SCI Data Register. */
#define mSCI1C2_Init_c      (mSCIxC2_TE_c | mSCIxC2_RE_c)
#define mSCI2C2_Init_c      (mSCIxC2_TE_c | mSCIxC2_RE_c)

/* Enable error interrupts. */
#define mSCI1C3_Init_c      (  mSCIxC3_ORIE_c | mSCIxC3_NEIE_c \
                             | mSCIxC3_FEIE_c | mSCIxC3_PEIE_c)
#define mSCI2C3_Init_c      (  mSCIxC3_ORIE_c | mSCIxC3_NEIE_c \
                             | mSCIxC3_FEIE_c | mSCIxC3_PEIE_c)

#ifdef PROCESSOR_MC1323X
  #define mSCI1C4_Init_c    0x00
#endif

/* Turn everything off. */
#define mSCIxC1_Reset_c     0x00
#define mSCIxC2_Reset_c     0x00
#define mSCIxC3_Reset_c     0x00

#ifdef PROCESSOR_MC1323X
  #define mSCIxC4_Reset_c    0x00
#endif

/*****************************************************************************/

/* Flow control. */
#if gUart1_EnableHWFlowControl_d
 #if gTarget_UserDefined_d == 1
  #define Uart1_EnableFlow()  { gSCI1RTSPort_c &= ~gSCI1RTSPin_c; }
  #define Uart1_DisableFlow() { gSCI1RTSPort_c |= gSCI1RTSPin_c; }
 #else
  #ifdef PROCESSOR_MC1323X
    #define Uart1_EnableFlow()  { PTDD &= ~0x04; }
    #define Uart1_DisableFlow() { PTDD |= 0x04; }    
  #else
    #define Uart1_EnableFlow()  { PTAD &= 0xFD; }
    #define Uart1_DisableFlow() { PTAD |= 0x02; }
  #endif
 #endif 
#else
 #define Uart1_EnableFlow()
 #define Uart1_DisableFlow()
#endif

#if gUart2_EnableHWFlowControl_d
 #if gTarget_UserDefined_d == 1
  #define Uart2_EnableFlow()  { gSCI2RTSPort_c &= ~gSCI2RTSPin_c; }
  #define Uart2_DisableFlow() { gSCI2RTSPort_c |= gSCI2RTSPin_c; }
 #else
  #define Uart2_EnableFlow()  { PTAD &= 0xBF; }
  #define Uart2_DisableFlow() { PTAD |= 0x40; }
 #endif 
#else
 #define Uart2_EnableFlow()
 #define Uart2_DisableFlow()
#endif

/*****************************************************************************/

/* if neither UART enabled, then no code or data */
#if gUart1_Enabled_d || gUart2_Enabled_d
  /* id for Uart task */
extern tsTaskID_t gUartTaskId;

#endif

/* Number of elements in an array. */
#ifndef NumberOfElements
#define NumberOfElements(array)     ((sizeof(array) / (sizeof(array[0]))))
#endif

#endif
