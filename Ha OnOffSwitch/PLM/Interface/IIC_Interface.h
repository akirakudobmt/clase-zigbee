/*****************************************************************************
* IIC Serial Port driver declarations.
* 
* This driver supports the IIC on the HCS08.
*
* Copyright (c) 2008, Freescale, Inc. All rights reserved.
*
*
* No part of this document must be reproduced in any form - including copied,
* transcribed, printed or by any electronic means - without specific written
* permission from Freescale Semiconductor.
*
*****************************************************************************/

#ifndef _IIC_Interface_h_
#define _IIC_Interface_h_

#include "EmbeddedTypes.h"
#include "AppToPlatformConfig.h"
#include "IoConfig.h"
#include "TS_Interface.h"
#include "IrqControlLib.h"

/*****************************************************************************
******************************************************************************
* Public macros
******************************************************************************
*****************************************************************************/

/* Enable or disable the IIC module */
#ifndef gIIC_Enabled_d
#define gIIC_Enabled_d    FALSE
#endif

/* Number of entries in the transmit-buffers-in-waiting list */
#ifndef gIIC_SlaveTransmitBuffersNo_c
#define gIIC_SlaveTransmitBuffersNo_c      3
#endif

/* Size of the driver's Rx circular buffer. This buffers is used to
    hold received bytes until the application can retrieve them via the
    IIcX_GetBytesFromRxBuffer() functions, and are not otherwise accessable
    from outside the driver. The size does not need to be a power of two. */
#ifndef gIIC_SlaveReceiveBufferSize_c
#define gIIC_SlaveReceiveBufferSize_c      32
#endif

/* IIC  Baud Rate settings */
/* IIC baud rate = BUSCLK / (mul * SCLdivider ) ; */
/*        |    MULT   |                ICR                |*/
/* IICF    IICF7 IICF6 IICF5 IICF4 IICF3 IICF2 IICF1 IICF0 */ 
/*   mul = f(MULT) ; SCLdivider = f(ICR)    */

#ifndef gSystemClock_d
#define gSystemClock_d     16           /* 16 MHz. */
#endif

#if gSystemClock_d == 8
#define gIIC_FrequencyDivider_50000_c     (uint8_t)((0<<6)| 0x1D)
#define gIIC_FrequencyDivider_100000_c    (uint8_t)((0<<6)| 0x14)
#define gIIC_FrequencyDivider_200000_c    (uint8_t)((0<<6)| 0x0B)
#define gIIC_FrequencyDivider_400000_c    (uint8_t)((0<<6)| 0x00)
#endif

#if gSystemClock_d == 12
#define gIIC_FrequencyDivider_50000_c     (uint8_t)((0<<6)| 0x1F)
#define gIIC_FrequencyDivider_100000_c    (uint8_t)((2<<6)| 0x05)
#define gIIC_FrequencyDivider_200000_c    (uint8_t)((1<<6)| 0x05)
#define gIIC_FrequencyDivider_400000_c    (uint8_t)((0<<6)| 0x05)
#endif

#if gSystemClock_d == 16
#define gIIC_FrequencyDivider_50000_c     (uint8_t)((0<<6)| 0x25)
#define gIIC_FrequencyDivider_100000_c    (uint8_t)((0<<6)| 0x1D)
#define gIIC_FrequencyDivider_200000_c    (uint8_t)((0<<6)| 0x14)
#define gIIC_FrequencyDivider_400000_c    (uint8_t)((0<<6)| 0x0B)
#endif

#if gSystemClock_d == 16780
#define gIIC_FrequencyDivider_50000_c     (uint8_t)((0<<6)| 0x25) //  52437.5
#define gIIC_FrequencyDivider_100000_c    (uint8_t)((0<<6)| 0x1D) // 104875
#define gIIC_FrequencyDivider_200000_c    (uint8_t)((0<<6)| 0x15) // 190681.81
#define gIIC_FrequencyDivider_400000_c    (uint8_t)((0<<6)| 0x0C) // 381363.63
#endif

/*list of possible baudrates */
#define  gIIC_BaudRate_50000_c    gIIC_FrequencyDivider_50000_c
#define  gIIC_BaudRate_100000_c   gIIC_FrequencyDivider_100000_c
#define  gIIC_BaudRate_200000_c   gIIC_FrequencyDivider_200000_c
#define  gIIC_BaudRate_400000_c   gIIC_FrequencyDivider_400000_c

/* Default baud rate. */
#ifndef gIIC_DefaultBaudRate_c
  #define gIIC_DefaultBaudRate_c  gIIC_BaudRate_100000_c  
#endif

/* The I2C slave address */
#ifndef gIIC_DefaultSlaveAddress_c
  #define gIIC_DefaultSlaveAddress_c   0x76
#endif

#if((gIIC_DefaultSlaveAddress_c > 0x7f) || (((gIIC_DefaultSlaveAddress_c & 0x78) == 0) && ((gIIC_DefaultSlaveAddress_c & 0x07) != 0)) || ((gIIC_DefaultSlaveAddress_c & 0x78) == 0x78))
  #error Illegal Slave Address!!!
#endif

/* Slave transmitter can signal to the master if there's data available */
#ifndef gIIC_Slave_TxDataAvailableSignal_Enable_c
#define gIIC_Slave_TxDataAvailableSignal_Enable_c   FALSE
#endif

#if gIIC_Slave_TxDataAvailableSignal_Enable_c
  #ifndef gIIC_TxDataAvailablePortDataReg_c
    #define gIIC_TxDataAvailablePortDataReg_c     PTAD  
  #endif
  #ifndef gIIC_TxDataAvailablePortDDirReg_c
    #define gIIC_TxDataAvailablePortDDirReg_c     PTADD
  #endif
  #ifndef gIIC_TxDataAvailablePinMask_c
    #define gIIC_TxDataAvailablePinMask_c         0x02
  #endif
#endif


/*****************************************************************************
******************************************************************************
* Public type definitions
******************************************************************************
*****************************************************************************/
typedef uint8_t IICBaudRate_t;
typedef uint8_t IICSlaveAddress_t;

/*****************************************************************************
******************************************************************************
* Public memory declarations
******************************************************************************
*****************************************************************************/
 extern volatile index_t        mIIcRxBufferByteCount;

/*****************************************************************************
******************************************************************************
* Public prototypes
******************************************************************************
*****************************************************************************/

/* Initialize the IIC module */
extern void     IIC_ModuleInit(void);
/* Shut down the IIC module */
extern void     IIC_ModuleUninit(void);
/* Set the IIC module Baud Rate  */
bool_t IIC_SetBaudRate(uint8_t baudRate);
/* Set the IIC module slave address */
bool_t   IIC_SetSlaveAddress(uint8_t slaveAddress);
/* Tries to set free the SDA line if another IIC device keeps it low    */
void IIC_BusRecovery(void);
/* Set the slave receive side callback function */
extern void     IIC_SetSlaveRxCallBack(void (*pfCallBack)(void));
/* Retrieve one byte from the driver's Rx buffer and store it at *pDst.
    Return TRUE if a byte was retrieved; FALSE if the Rx buffer is empty */
extern bool_t   IIC_GetByteFromRxBuffer(unsigned char *pDst);
/* Transmit bufLen bytes of data from pBuffer over a port. Call *pfCallBack() */
/* when the entire buffer has been sent. Returns FALSE if there are no */
/* more available Tx buffer slots, TRUE otherwise. The caller must ensure */
/* that the buffer remains available until the call back function is called. */
/* pfCallBack must not be NULL. */
/* The callback function will be called in interrupt context, so it should */
/* be kept very short. */
extern bool_t IIC_Transmit_Slave(uint8_t const *pBuf, index_t bufLen, void (*pfCallBack)(uint8_t const *pBuf));
bool_t IIC_Transmit_Master(uint8_t const *pBuf, index_t bufLen, uint8_t destAddress, void (*pfCallBack)(bool_t status)) ;
bool_t IIC_Receive_Master(uint8_t *pBuf, index_t bufLen, uint8_t destAddress, void (*pfCallBack)(bool_t status)) ;

/* Checks if Slave  Tx process is still running */
bool_t IIC_IsSlaveTxActive(void);
extern void IIC_TxDataAvailable(bool_t bIsAvailable);
/* Place it in NON_BANKED memory */
#ifdef MEMORY_MODEL_BANKED
#pragma CODE_SEG __NEAR_SEG NON_BANKED
#else
#pragma CODE_SEG DEFAULT
#endif /* MEMORY_MODEL_BANKED */
extern INTERRUPT_KEYWORD void IIC_Isr(void);
#pragma CODE_SEG DEFAULT           



#endif    /* _IIC_Interface_h_ */
