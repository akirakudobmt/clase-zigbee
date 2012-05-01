/************************************************************************************
* This is a template header file.
*
*
* (c) Copyright 2005, Freescale Semiconductor, Inc. All rights reserved.
*
*
* No part of this document must be reproduced in any form - including copied,
* transcribed, printed or by any electronic means - without specific written
* permission from Freescale Semiconductor.
************************************************************************************/

#ifndef _MC1319XREG_H_
#define _MC1319XREG_H_

/************************************************************************************
*************************************************************************************
* Public macros
*************************************************************************************
************************************************************************************/

#define HW_Reset 0x00


/**
  Register 0x24 definitions aka ABEL_STATUS_REG
*/
#define ABEL_STATUS_REG ABEL_reg24

#define cLO_LOCK_IRQ    (1<<15)
#define cRAM_ADDR_ERR   (1<<14) /* used in packet mode */
#define cTX_DONE        (1<<14) /* used in stream mode */
#define cARB_BUSY_ERR   (1<<13) /* used in packet mode */
#define cRX_DONE        (1<<13) /* used in stream mode */
#define cSTRM_DATA_ERR  (1<<12)
#define cHG_IRQ         (1<<11)
#define cResetATTN_IRQ  (1<<10)
#define cDOZE_IRQ       (1<<9)
#define cTMR1_IRQ       (1<<8)
#define cRX_RCVD_IRQ    (1<<7)
#define cTX_SENT_IRQ    (1<<6)
#define cCCA_IRQ        (1<<5)
#define cTMR3_IRQ       (1<<4)
#define cTMR4_IRQ       (1<<3)
#define cTMR2_IRQ       (1<<2)
#define cCCA            (1<<1)
#define cCRC_VALID      (1<<0)

/**
  Register 0x01 aka RX_DATA
*/
#define RX_DATA ABEL_reg1

/**
  Register 0x02 aka TX_DATA
*/
#define TX_DATA ABEL_reg2


/**
  Register 0x03 aka TX_CONTROL
*/
#define TX_CONTROL     ABEL_reg3

#define cTX_PKT_LENGTH7shift (0)
#define cTX_PKT_LENGTH7mask (0x7F << cTX_PKT_LENGTH7shift)

#define cSTART_ADDR6shift (8)
#define cSTART_ADDR6mask (0x3F << cSTART_ADDR6shift)
#define cBCN_RAM_SEL (1<<15)


/**
  Register 0x06 aka ABEL_CONTROL_REG
*/
#define ABEL_CONTROL_REG ABEL_reg6

#define cACK_FRM_PEND   (1<<15)
#define cRX_RCV_ALL     (1<<14)
#define cTX2RX_REQD     (1<<13)
#define cTX_STRM        (1<<12)
#define cRX_STRM        (1<<11)
#define cCCA_MASK       (1<<10)
#define cTX_SENT_MASK   (1<<9)
#define cRX_RCVD_MASK   (1<<8)
#define cTMR_TRIG_EN    (1<<7)
#define cAUTO_ACK_EN    (1<<6)
#define cCCA_MODE_2MASK (0x03 << 4)
#define cTIMESTAMP_ALL  (1<<3)
#define cXVR_SEQ_3MASK  (0x07 << 0)

#define cCCA_MODE0  (0x01 << 4)
#define cCCA_ED     (0x02 << 4)
#define cRX_AGC     (0x02 << 4)

#define SEQ_NOP (0)
#define SEQ_C   (1)
#define SEQ_RT  (2)
#define SEQ_T   (3)
#define SEQ_TRT (4)
#define SEQ_CT  (5)
#define SEQ_CCT (6)
#define SEQ_CC  (7)


/**
  Register 0x05 aka ABEL_MASK_REG
*/
#define ABEL_MASK_REG ABEL_reg5

#define cATTN_MASK      (1<<15)
#define cRAM_ADDR_MASK  (1<<12)
#define cARB_BUSY_MASK  (1<<11)
#define cSTRM_DATA_MASK (1<<10)
#define cLO1_LOCK_MASK  (1<<9)
#define cACOMA_EN_MASK  (1<<8)
#define cHG_MASK        (1<<5)
#define cDOZE_MASK      (1<<4)
#define cTMR4_MASK      (1<<3)
#define cTMR3_MASK      (1<<2)
#define cTMR2_MASK      (1<<1)
#define cTMR1_MASK      (1<<0)


/**
  Register 0x09
*/
#define ABEL_PRESCALE_REG ABEL_reg9

#define cTMR_PRESCALEshift (0)
#define cTMR_PRESCALEmask (0x07)

#define gCLKO_EnableBit_c (1<<5)

/**
  Register 0x07 aka ABEL_CONTROL2_REG
*/
#define ABEL_CONTROL2_REG ABEL_reg7

#define cTMR_LOAD       (1<<15)
#define cCLKO_DOZE_EN   (1<<9)
#define cABEL2SPI_MASK  (1<<8)
#define cTX_DONE_MASK   (1<<7)
#define cRX_DONE_MASK   (1<<6)
#define cUSE_STRM_MODE  (1<<5)
#define cHG_BIAS_EN     (1<<4)
#define cHG_EN          (1<<2)
#define cHIB_EN         (1<<1)
#define cDOZE_EN        (1<<0)


/**
  Register 0x25 aka rx_status
*/
#define ABEL_RX_STATUS_REG ABEL_reg25

/**
  Register 0x26 aka ABEL_CURR_TIME_MSB_REG
*/
#define ABEL_CURR_TIME_MSB_REG ABEL_reg26

/**
  Register 0x27 aka ABEL_CURR_TIME_LSB_REG
*/
#define ABEL_CURR_TIME_LSB_REG ABEL_reg27

/**
  Register 0x2E aka ABEL_TIMESTAMP_MSB_REG
*/
#define ABEL_TIMESTAMP_MSB_REG ABEL_reg2E

/**
  Register 0x2F aka ABEL_TIMESTAMP_LSB_REG
*/
#define ABEL_TIMESTAMP_LSB_REG ABEL_reg2F



/**
  Register 0x2D aka ABEL_ENERGYLVL_REG
*/
#define ABEL_ENERGYLVL_REG ABEL_reg2D

#define cRX_LEVEL8mask  (0xFF)
#define cRX_LEVEL8shift (8)

#define cRX_DSN8shift   (8)
#define cRX_DSN8mask    (0xFF << cRX_DSN8shift)
#define cRX_PKT_LENGTH7shift   (0)
#define cRX_PKT_LENGTH7mask    (0x7F)
#define cRX_LEV_LENmask (((unsigned long)cRX_LEVEL8mask << cRX_LEVEL8shift) | ((unsigned long)cRX_PKT_LENGTH7mask << cRX_PKT_LENGTH7shift)) // 0xFF7F

/**
  Register 0x0F aka ABEL_LO1DIV_REG
*/
#define ABEL_LO1DIV_REG ABEL_regF
#define cIDIV8mask  (0xFF)
#define cIDIV8shift (0x0)

/**
  Register 0x10 aka ABEL_LO1NUM_REG
*/
#define ABEL_LO1NUM_REG ABEL_reg10

#define ABEL_TMR1_HI_REG ABEL_reg1B
#define ABEL_TMR1_LO_REG ABEL_reg1C
#define ABEL_TMR2_HI_REG ABEL_reg1D
#define ABEL_TMR2_LO_REG ABEL_reg1E
#define ABEL_TMR3_HI_REG ABEL_reg1F
#define ABEL_TMR3_LO_REG ABEL_reg20
#define ABEL_TMR4_HI_REG ABEL_reg21
#define ABEL_TMR4_LO_REG ABEL_reg22
#define ABEL_TMR2_PRIME_REG ABEL_reg23

#define READ_GPIO ABEL_sim_gpio
#define cABEL_GPIO1 (1)
#define cABEL_GPIO2 (2)

#define ABEL_reg0 (0x0)
#define ABEL_reg1 (0x1)
#define ABEL_reg2 (0x2)
#define ABEL_reg3 (0x3)
#define ABEL_reg4 (0x4)
#define ABEL_reg5 (0x5)
#define ABEL_reg6 (0x6)
#define ABEL_reg7 (0x7)
#define ABEL_reg8 (0x8)
#define ABEL_reg9 (0x9)
#define ABEL_regA (0xA)
#define ABEL_regB (0xB)
#define ABEL_regC (0xC)
#define ABEL_regD (0xD)
#define ABEL_regE (0xE)
#define ABEL_regF (0xF)
#define ABEL_reg10 (0x10)
#define ABEL_reg11 (0x11)
#define ABEL_reg12 (0x12)
#define ABEL_reg13 (0x13)
#define ABEL_reg14 (0x14)
#define ABEL_reg15 (0x15)
#define ABEL_reg16 (0x16)
#define ABEL_reg17 (0x17)
#define ABEL_reg18 (0x18)
#define ABEL_reg19 (0x19)
#define ABEL_reg1A (0x1A)
#define ABEL_reg1B (0x1B)
#define ABEL_reg1C (0x1C)
#define ABEL_reg1D (0x1D)
#define ABEL_reg1E (0x1E)
#define ABEL_reg1F (0x1F)
#define ABEL_reg20 (0x20)
#define ABEL_reg21 (0x21)
#define ABEL_reg22 (0x22)
#define ABEL_reg23 (0x23)
#define ABEL_reg24 (0x24)
#define ABEL_reg25 (0x25)
#define ABEL_reg26 (0x26)
#define ABEL_reg27 (0x27)
#define ABEL_reg28 (0x28)
#define ABEL_reg29 (0x29)
#define ABEL_reg2A (0x2A)
#define ABEL_reg2B (0x2B)
#define ABEL_reg2C (0x2C)
#define ABEL_reg2D (0x2D)
#define ABEL_reg2E (0x2E)
#define ABEL_reg2F (0x2F)
#define ABEL_reg30 (0x30)
#define ABEL_reg31 (0x31)
#define ABEL_reg32 (0x32)
#define ABEL_reg33 (0x33)
#define ABEL_reg34 (0x34)
#define ABEL_reg35 (0x35)
#define ABEL_reg36 (0x36)
#define ABEL_reg37 (0x37)
#define ABEL_reg38 (0x38)
#define ABEL_reg39 (0x39)
#define ABEL_reg3A (0x3A)
#define ABEL_reg3B (0x3B)
#define ABEL_reg3C (0x3C)
#define ABEL_reg3D (0x3D)
#define ABEL_reg3E (0x3E)
#define ABEL_reg3F (0x3F)
#define ABEL_sim_gpio (0x3F)

/************************************************************************************
*************************************************************************************
* Public prototypes
*************************************************************************************
************************************************************************************/

/************************************************************************************
*************************************************************************************
* Public type definitions
*************************************************************************************
************************************************************************************/

/************************************************************************************
*************************************************************************************
* Public memory declarations
*************************************************************************************
************************************************************************************/

/************************************************************************************
*************************************************************************************
* Public functions
*************************************************************************************
************************************************************************************/

/************************************************************************************
*************************************************************************************
* Level 1 block comment
*************************************************************************************
************************************************************************************/

//-----------------------------------------------------------------------------------
//-----------------------------------------------------------------------------------
// Level 2 block comment
//-----------------------------------------------------------------------------------
//-----------------------------------------------------------------------------------

/* Level 3 block comment */




// Delimiters

/***********************************************************************************/

//-----------------------------------------------------------------------------------

#endif /* _MC1319XREG_H_ */
