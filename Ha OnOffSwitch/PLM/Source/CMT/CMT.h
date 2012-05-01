/*****************************************************************************
* Carrier Modulation Timer implementation 
*
* Copyrigtht (c) 2009, Freescale, Inc. All rights reserved
*
*
* No part of this document must be reproduced in any form - including copied,
* transcribed, printed or by any electronic means - without specific written
* permission from Freescale Semiconductor
*
*****************************************************************************/

#ifndef _CMT_H
#define _CMT_H


/*****************************************************************************
******************************************************************************
* Private macros
******************************************************************************
*****************************************************************************/

/* Bits in CMT Output Control Register. Read/Write */
#define mCMT_CMTOC_IROL_c           0x80        /* IRO Latch Control */
#define mCMT_CMTOC_CMTPOL_c         0x40        /* CMT Output Polarity */
#define mCMT_CMTOC_IROPEN_c         0x20        /* IRO Pin Enable */
#define mCMT_CMTOC_CMTDIV2_c        0x10        /* CMT Divide by 2 */

/* Bits in CMT Modulator Status and Control Register. Read/Write and Read Only */
#define mCMT_CMTMSC_EOCF_c          0x80        /* End of Cycle Status Flag */
#define mCMT_CMTMSC_CMTDIV1_c       0x40        /* CMT Clock Divide Prescaler 1 */
#define mCMT_CMTMSC_CMTDIV0_c       0x20        /* CMT Clock Divide Prescaler 0 */
#define mCMT_CMTMSC_EXSPC_c         0x10        /* Extended Space Enable */
#define mCMT_CMTMSC_BASE_c          0x08        /* Baseband Enable */
#define mCMT_CMTMSC_FSK_c           0x04        /* FSK Mode Select */
#define mCMT_CMTMSC_EOCIE_c         0x02        /* End of Cycle Interrupt Enable*/
#define mCMT_CMTMSC_MCGEN_c         0x01        /* Modulation and Carrier Generator Enable */

/* 16-bit Modulation Mark and Space Registers () */
#define mCMT_Modulation_MarkRegs    (*((volatile uint16_t *) (0x0066)))
#define mCMT_Modulation_SpaceRegs   (*((volatile uint16_t *) (0x0068)))

/* CMT Divider */
#define mCmtDivider_c               0x10

/* CMT Prescale possible values */
#define mCmtPrescale1_c             0x00
#define mCmtPrescale2_c             0x20
#define mCmtPrescale4_c             0x40
#define mCmtPrescale8_c             0x60

/* CMT input clock possible values */
#define mCmtClockInput16MHz_c       16000000
#define mCmtClockInput8MHz_c        8000000
#define mCmtClockInput4MHz_c        4000000
#define mCmtClockInput2MHz_c        2000000
#define mCmtClockInput1MHz_c        1000000

#ifndef mCmtDefaultClockInput_c 
  #define mCmtDefaultClockInput_c  mCmtClockInput8MHz_c
#endif

/* Configure the CMT clock input fCMTCLK, based on system frequency (bus clock) equals to 16 MHz */
#if(mCmtDefaultClockInput_c == mCmtClockInput16MHz_c)
  #define mCmtPrescaleDefault_c    mCmtPrescale1_c 
  #define mCmtDividerDefault_c     0                                  
#elif(mCmtDefaultClockInput_c == mCmtClockInput8MHz_c)
  #define mCmtPrescaleDefault_c    mCmtPrescale2_c 
  #define mCmtDividerDefault_c     0                                                                       
#elif(mCmtDefaultClockInput_c == mCmtClockInput4MHz_c)
  #define mCmtPrescaleDefault_c    mCmtPrescale4_c 
  #define mCmtDividerDefault_c     0                                                                                                            
#elif(mCmtDefaultClockInput_c == mCmtClockInput2MHz_c)
  #define mCmtPrescaleDefault_c    mCmtPrescale8_c 
  #define mCmtDividerDefault_c     0                                                                                                             
#elif(mCmtDefaultClockInput_c == mCmtClockInput1MHz_c)                         
  #define mCmtPrescaleDefault_c    mCmtPrescale1_c 
  #define mCmtDividerDefault_c     mCmtDivider_c                                                                      
#endif

/*****************************************************************************/
/* Turn CMT Modulation  and Carrier Generation off */
#define mCMT_CMTMSC_Reset_c   0
#define mCMT_CMTOC_Reset_c    0

#endif