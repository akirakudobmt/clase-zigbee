/*****************************************************************************
* UART / SCI / USB / Serial Port implementation.
*
* (c) Copyright 2006, Freescale, Inc. All rights reserved.
*
*
* No part of this document must be reproduced in any form - including copied,
* transcribed, printed or by any electronic means - without specific written
* permission from Freescale Semiconductor.
*
*****************************************************************************/

/* Note: This code is size optimized for the case where only one SCI port is
 * enabled. If both ports are enabled, it might be worthwhile to create
 * functions that take a port number as a parameter.
 */

#include "Uart_Interface.h"
#include "Uart.h"
#include "IoConfig.h"
#include "functionlib.h"
#include "TS_Interface.h"
#include "msgsystem.h"

#ifdef PROCESSOR_MC1323X
#include "PWRLib.h"
#endif

#ifndef gMacStandAlone_d
#define gMacStandAlone_d 0
#endif

#ifndef gBeeStackIncluded_d
#define gBeeStackIncluded_d 0
#endif


#if gBeeStackIncluded_d
  #include "BeeStack_Globals.h"
#endif

#if gMacStandAlone_d
  #include "Mac_Globals.h"
#endif

/************************************************************************************
*************************************************************************************
* Private prototypes
*************************************************************************************/


/*****************************************************************************
******************************************************************************
* Public memory definitions
******************************************************************************
*****************************************************************************/

/* if neither UART enabled, then no code or data */
#if gUart1_Enabled_d || gUart2_Enabled_d

/* id for Uart task */
tsTaskID_t gUartTaskId;


/* Each port can store one unget byte. */
#if gUart1_Enabled_d
static bool_t uart1_ungetFlag = FALSE;
static uint8_t uart1_ungetByte;
bool_t Uart1_TxCompleteFlag = FALSE;
#endif

#if gUart2_Enabled_d
static bool_t uart2_ungetFlag = FALSE;
static uint8_t uart2_ungetByte;
bool_t Uart2_TxCompleteFlag = FALSE;
#endif

/* The transmit code keeps a circular list of buffers to be sent. */
/* Each SCI port has it's own list. */
typedef struct SciTxBufRef_tag {
  unsigned char const *pBuf;
  void (*pfCallBack)(unsigned char const *pBuf);
} SciTxBufRef_t;

/* The leading index is the next position to store a buffer reference. */
/* The trailing index is the buffer currently being transmitted. */
/* Using a separate table for the buffer lengths reduces code size. */
#if gUart1_Enabled_d
static SciTxBufRef_t maSci1TxBufRefTable[gUart_TransmitBuffers_c];
static index_t maSci1TxBufLenTable[gUart_TransmitBuffers_c];
static index_t mSci1TxCurIndex;
static index_t mSci1TxBufRefLeadingIndex;   /* Post-increment. */
static index_t mSci1TxBufRefTrailingIndex;  /* Post-increment. */

/*Callback table and index pointers*/
static SciTxBufRef_t maSci1TxCallbackTable[gUart_TransmitBuffers_c];
static index_t maSci1TxCallbackLeadingIndex = 0; 
static index_t maSci1TxCallbackTrailingIndex = 0; 
#endif

#if gUart2_Enabled_d
static SciTxBufRef_t maSci2TxBufRefTable[gUart_TransmitBuffers_c];
static index_t maSci2TxBufLenTable[gUart_TransmitBuffers_c];
static index_t mSci2TxCurIndex;
static index_t mSci2TxBufRefLeadingIndex;   /* Post-increment. */
static index_t mSci2TxBufRefTrailingIndex;  /* Post-increment. */
/*Callback table and index pointers*/
static SciTxBufRef_t maSci2TxCallbackTable[gUart_TransmitBuffers_c];
static index_t maSci2TxCallbackLeadingIndex = 0; 
static index_t maSci2TxCallbackTrailingIndex = 0; 
#endif

/* Each port has a small local circular Rx buffer to store data until the */
/* application can retrieve it. In addition to the leading and trailing */
/* indexes, the Rx code keeps a count of the number of bytes in the buffer. */
#if gUart1_Enabled_d
static   uint8_t maSci1RxBuf[gUart_ReceiveBufferSize_c];
static   index_t mSci1RxBufLeadingIndex;      /* Post-increment. */
static   index_t mSci1RxBufTrailingIndex;     /* Post-increment. */
volatile index_t mSci1RxBufferByteCount;      /* # of bytes in buffer. */
static void (*pfSci1RxCallBack)(void);
#endif

#if gUart2_Enabled_d
static   uint8_t maSci2RxBuf[gUart_ReceiveBufferSize_c];
static   index_t mSci2RxBufLeadingIndex;      /* Post-increment. */
static   index_t mSci2RxBufTrailingIndex;     /* Post-increment. */
volatile index_t mSci2RxBufferByteCount;      /* # of bytes in buffer. */
static void (*pfSci2RxCallBack)(void);
#endif

/*****************************************************************************
******************************************************************************
* Private memory definitions
******************************************************************************
*****************************************************************************/

/* Keep track of the high water mark in the Rx circular buffer. */
#if gUart_Debug_d
#if gUart1_Enabled_d
static index_t uart1RxBufHighWaterMark = 0;
#endif
#if gUart2_Enabled_d
static index_t uart2RxBufHighWaterMark = 0;
#endif
#endif

/*****************************************************************************
******************************************************************************
* Public functions
******************************************************************************
*****************************************************************************/

/*****************************************************************************/


void Uart_ClearErrors(void) {
  uint8_t dummy;

#if gUart1_Enabled_d
  while (SCI1S1 & (mSCIxS1_OR_c | mSCIxS1_NF_c | mSCIxS1_FE_c | mSCIxS1_PF_c)) {
    dummy = SCI1D;
  }
#endif

#if gUart2_Enabled_d
  while (SCI2S1 & (mSCIxS1_OR_c | mSCIxS1_NF_c | mSCIxS1_FE_c | mSCIxS1_PF_c)) {
    dummy = SCI2D;
  }
#endif
}                                       /* Uart_ClearErrors() */

/*****************************************************************************/


/* Retrieve a byte from the driver's Rx buffer and store it at *pDst. */
/* Return TRUE if a byte was retrieved; FALSE if the Rx buffer is empty. */
bool_t Uart1_GetByteFromRxBuffer(unsigned char *pDst) {
#if !gUart1_Enabled_d
  (void) pDst;
  return 0;
#else
  unsigned char scic2, var;

  if (uart1_ungetFlag) {
    uart1_ungetFlag = FALSE;
    *pDst = uart1_ungetByte;
    return TRUE;
  }

  if (!mSci1RxBufferByteCount) {
    return FALSE;
  }

#ifdef PROCESSOR_MC1323X
  IrqControlLib_ProtectFromMC1323xIrq(var);
#endif
  /* Disable the Rx interrupt while the counter is updated. */
  scic2 = SCI1C2;
  SCI1C2 &= ~mSCIxC2_RIE_c;

  *pDst = maSci1RxBuf[mSci1RxBufTrailingIndex];
  if (++mSci1RxBufTrailingIndex >= sizeof(maSci1RxBuf)) 
  {
    mSci1RxBufTrailingIndex = 0;
  }

  --mSci1RxBufferByteCount;
#if gUart1_EnableHWFlowControl_d
  if (mSci1RxBufferByteCount <= gUart_RxFlowControlSkew_d) 
  {
    Uart1_EnableFlow();
  }
#endif
  SCI1C2 = scic2;
#ifdef PROCESSOR_MC1323X
  IrqControlLib_UnprotectFromMC1323xIrq(var);
#endif

  return TRUE;
#endif
}                                       /* Uart1_GetBytesFromRxBuffer() */

/**************************************/

bool_t Uart2_GetByteFromRxBuffer(unsigned char *pDst) {
#if !gUart2_Enabled_d
  (void) pDst;
  return 0;
#else
  unsigned char scic2;

  if (uart2_ungetFlag) {
    uart2_ungetFlag = FALSE;
    *pDst = uart2_ungetByte;
    return TRUE;
  }

  if (!mSci2RxBufferByteCount) {
    return FALSE;
  }
  
  /* Disable the Rx interrupt while the counter is updated. */
  scic2 = SCI2C2;
  SCI2C2 &= ~mSCIxC2_RIE_c;
  
  *pDst = maSci2RxBuf[mSci2RxBufTrailingIndex];
  if (++mSci2RxBufTrailingIndex >= sizeof(maSci2RxBuf)) 
  {
    mSci2RxBufTrailingIndex = 0;
  }

  --mSci2RxBufferByteCount;
#if gUart2_EnableHWFlowControl_d
  if (mSci2RxBufferByteCount <= gUart_RxFlowControlSkew_d) 
  {
    Uart2_EnableFlow();
  }
#endif
  SCI2C2 = scic2;

  return TRUE;
#endif
}                                       /* Uart2_GetBytesFromRxBuffer() */

/*****************************************************************************/

/* Return TRUE if there is data still not sent. */
bool_t Uart1_IsTxActive(void) {
#if !gUart1_Enabled_d
  return FALSE;
#else
  return (maSci1TxBufLenTable[mSci1TxBufRefTrailingIndex] != 0) || (!(SCI1S1 & 0x40));
#endif
}                                       /* Uart1_IsTxActive() */

/*****************************************************************************/

bool_t Uart2_IsTxActive(void) {
#if !gUart2_Enabled_d
  return FALSE;
#else
  return (maSci2TxBufLenTable[mSci2TxBufRefTrailingIndex] != 0) || (!(SCI2S1 & 0x40));
#endif
}                                       /* Uart2_IsTxActive() */

/*****************************************************************************/

/* Initialize the serial port(s) and this module's local data. */
void Uart_ModuleInit(void) {
#if gUart1_Enabled_d
  
  Uart1_SetBaud(gUartDefaultBaud_c);

  SCI1C1 = mSCI1C1_Init_c;
  SCI1C3 = mSCI1C3_Init_c;
  SCI1C2 = mSCI1C2_Init_c;
  
  #ifdef PROCESSOR_MC1323X
    SCI1C4 = mSCI1C4_Init_c;
  #endif

  #if gUart1_EnableHWFlowControl_d
    Uart1_EnableFlow();
  #endif
#endif

#if gUart2_Enabled_d
  SCI2BDH = (gUartDefaultBaud_c >> 8);
  SCI2BDL = (gUartDefaultBaud_c & 0xFF);

  SCI2C1 = mSCI2C1_Init_c;
  SCI2C3 = mSCI2C3_Init_c;
  SCI2C2 = mSCI2C2_Init_c;
  
  #if gUart2_EnableHWFlowControl_d
    Uart2_EnableFlow();
  #endif
#endif

  Uart_ClearErrors();
  Uart_TaskInit();
}                                       /* Uart_ModuleInit() */


/*****************************************************************************/

/* Set a pointer to the function to be called whenever a byte is received. */
/* If the pointer is set to NULL, turn off the Rx interrupt and clear the Rx */
/* buffer. */
void Uart1_SetRxCallBack(void (*pfCallBack)(void)) {
#if !gUart1_Enabled_d
  (void) pfCallBack;
#else
  pfSci1RxCallBack = pfCallBack;

  if (pfSci1RxCallBack) {
    SCI1C2 |= mSCIxC2_RIE_c;
  } else {
    SCI1C2 &= ~mSCIxC2_RIE_c;
    mSci1RxBufLeadingIndex = mSci1RxBufTrailingIndex = mSci1RxBufferByteCount = 0;
  }
#endif
}                                       /* Uart1_SetRxCallback() */

/*****************************************************************************/

void Uart2_SetRxCallBack(void (*pfCallBack)(void)) {
#if !gUart2_Enabled_d
  (void) pfCallBack;
#else
  pfSci2RxCallBack = pfCallBack;

  if (pfSci2RxCallBack) {
    SCI2C2 |= mSCIxC2_RIE_c;
  } else {
    SCI2C2 &= ~mSCIxC2_RIE_c;
    mSci2RxBufLeadingIndex = mSci2RxBufTrailingIndex = mSci2RxBufferByteCount = 0;
  }
#endif
}                                       /* Uart2_SetRxCallback() */

/*****************************************************************************/

/* Set the UART baud rate. */
void Uart1_SetBaud(UartBaudRate_t baudRate) {
#if !gUart1_Enabled_d
  (void) baudRate;
#else
#ifdef PROCESSOR_MC1323X 
  SCI1BDH = (baudRate >> 8) | (1 << 6);
#else
  SCI1BDH = baudRate >> 8;
#endif 
 SCI1BDL = baudRate & 0xFF;
  
  #ifdef PROCESSOR_MC1323X    
    switch(baudRate)
    {
      case Baudrate_1200:
              SCI1C4 = gUARTBaudRate1200Div_c;
              break;
              
      case Baudrate_2400:              
              SCI1C4 = gUARTBaudRate2400Div_c;
              break;
              
      case Baudrate_4800:
              SCI1C4 = gUARTBaudRate4800Div_c;
              break;
              
      case Baudrate_9600:              
              SCI1C4 = gUARTBaudRate9600Div_c;
              break;   
              
      case Baudrate_19200:
              SCI1C4 = gUARTBaudRate19200Div_c;
              break;
              
      case Baudrate_38400:              
              SCI1C4 = gUARTBaudRate38400Div_c;
              break;
              
      case Baudrate_57600:
              SCI1C4 = gUARTBaudRate57600Div_c;
              break;
              
      case Baudrate_115200:              
              SCI1C4 = gUARTBaudRate115200Div_c;
              break;                                      
    }    
  #endif
#endif
}                                       /* Uart1_SetBaud() */

/*****************************************************************************/

void Uart2_SetBaud(UartBaudRate_t baudRate) {
#if !gUart2_Enabled_d
  (void) baudRate;
#else
  SCI2BDH = baudRate >> 8;
  SCI2BDL = baudRate & 0xFF;   
#endif
}                                       /* Uart2_SetBaud() */

/*****************************************************************************/

/* Begin transmitting size bytes of data from *pBuffer. */
/* Returns FALSE if there are no more slots in the buffer reference table. */
bool_t Uart1_Transmit(unsigned char const *pBuf, index_t bufLen, void (*pfCallBack)(unsigned char const *pBuf)) {
#if !gUart1_Enabled_d
  (void) pBuf;
  (void) bufLen;
  (void) pfCallBack;
  return FALSE;
#else
  bool_t status = FALSE;
  /* Handle empty buffers. */
  if (!bufLen) 
  {
    if(pfCallBack)
      (*pfCallBack)(pBuf);
    return TRUE;
  }


  /* Room for one more? */
  if (maSci1TxBufLenTable[mSci1TxBufRefLeadingIndex] == 0) 
  {
       
   /* Disable the Tx ISR before updating the shared variables */
   SCI1C2 &= ~mSCIxC2_TIE_c;
   
   maSci1TxBufRefTable[mSci1TxBufRefLeadingIndex].pBuf = pBuf;
   maSci1TxBufRefTable[mSci1TxBufRefLeadingIndex].pfCallBack = pfCallBack;
   /* This has to be last, in case the Tx ISR finishes with the previous */
   /* buffer while this function is executing. */
   maSci1TxBufLenTable[mSci1TxBufRefLeadingIndex] = bufLen;
   
   if (++mSci1TxBufRefLeadingIndex >= NumberOfElements(maSci1TxBufRefTable)) {
     mSci1TxBufRefLeadingIndex = 0;
   }
  
   Uart1_TxCompleteFlag = FALSE;
   /* Enable the transmit interrupt. Harmless if the interrupt is already */
   /* enabled. */
   SCI1C2 |= mSCIxC2_TIE_c;
   
   status = TRUE;
  }

  return status; 
#endif
}                                       /* Uart1_Transmit() */

/**************************************/
bool_t Uart2_Transmit(unsigned char const *pBuf, index_t bufLen, void (*pfCallBack)(unsigned char const *pBuf)) {
#if !gUart2_Enabled_d
  (void) pBuf;
  (void) bufLen;
  (void) pfCallBack;
  return FALSE;
#else
  bool_t status = FALSE;
  /* Handle empty buffers. */
  if (!bufLen) 
  {
    if(pfCallBack)
      (*pfCallBack)(pBuf);
    return TRUE;
  }
  
  /* Room for one more? */
  if (maSci2TxBufLenTable[mSci2TxBufRefLeadingIndex] == 0) 
  {
    /* Disable the Tx ISR before reading or updating the shared variables */
   SCI2C2 &= ~mSCIxC2_TIE_c;   
   
   maSci2TxBufRefTable[mSci2TxBufRefLeadingIndex].pBuf = pBuf;
   maSci2TxBufRefTable[mSci2TxBufRefLeadingIndex].pfCallBack = pfCallBack;
   /* This has to be last, in case the Tx ISR finishes with the previous */
   /* buffer while this function is executing. */
   maSci2TxBufLenTable[mSci2TxBufRefLeadingIndex] = bufLen;

   if (++mSci2TxBufRefLeadingIndex >= NumberOfElements(maSci2TxBufRefTable)) {
     mSci2TxBufRefLeadingIndex = 0;
   }
   Uart2_TxCompleteFlag = FALSE;
   /* Enable the transmit interrupt. Harmless if the interrupt is already */
   /* enabled. */
   
   SCI2C2 |= mSCIxC2_TIE_c;
   status = TRUE;
  } 
  
  return status;
#endif
}                                       /* Uart2_Transmit() */

/*****************************************************************************/

/* Shut down the serial port(s). */
void Uart_ModuleUninit(void) {
#if gUart1_Enabled_d
  SCI1C1 = mSCIxC1_Reset_c;
  SCI1C3 = mSCIxC2_Reset_c;
  SCI1C2 = mSCIxC3_Reset_c;
  
  #ifdef PROCESSOR_MC1323X
    SCI1C4 = mSCIxC4_Reset_c;    
  #endif 
#endif

#if gUart2_Enabled_d
  SCI2C1 = mSCIxC1_Reset_c;
  SCI2C3 = mSCIxC2_Reset_c;
  SCI2C2 = mSCIxC3_Reset_c;
#endif
}                                       /* Uart_ModuleUninit() */

/*****************************************************************************/

/* Unget a byte: store the byte so that it will be returned by the next call */
/* to the get byte function. Also call the Rx call back. */
void Uart1_UngetByte(unsigned char byte) {
#if !gUart1_Enabled_d
  (void) byte;
#else
  /* If the un get flag is already true, there isn't anything we can */
  /* do about it. Not much point in checking. Its up to the called to */
  /* use this function correctly. */
  uart1_ungetFlag = TRUE;
  uart1_ungetByte = byte;

  /* Likewise, it is the caller's responsibility to not call this function */
  /* if the call back pointer is not valid. */
  pfSci1RxCallBack();

#endif
}                                       /* Uart1_UngetByte() */

/*****************************************************************************/

void Uart2_UngetByte(unsigned char byte) {
#if !gUart2_Enabled_d
  (void) byte;
#else
  uart2_ungetFlag = TRUE;
  uart2_ungetByte = byte;
  pfSci2RxCallBack();
#endif
}                                       /* Uart2_UngetByte() */

/*****************************************************************************/

void Uart_Task(event_t events)
{
  void (*pfCallBack)(unsigned char const *pBuf);

#if gUart1_Enabled_d

  if( events & gRxSci1Event_c ) {
    pfSci1RxCallBack();
  }

  if( events & gTxSci1Event_c ) {
    /*Tx call back event received - run through the callback table and execute any pending
    Callbacks.
    */
    while (maSci1TxCallbackLeadingIndex != maSci1TxCallbackTrailingIndex) {    
       pfCallBack = maSci1TxCallbackTable[maSci1TxCallbackTrailingIndex].pfCallBack;
      /*Call calback with buffer info as parameter*/
       pfCallBack(maSci1TxCallbackTable[maSci1TxCallbackTrailingIndex].pBuf);
      /*increment and wrap around trailing index*/      
      if (++maSci1TxCallbackTrailingIndex >= NumberOfElements(maSci1TxCallbackTable)) {
        maSci1TxCallbackTrailingIndex = 0;    
      }
    } /* while */
  } /*if events ...*/


#endif gUart1_Enabled_d
  
#if gUart2_Enabled_d

  if( events & gRxSci2Event_c ) {
    pfSci2RxCallBack();
  } 

  if( events & gTxSci2Event_c ) {
    /*Tx call back event received - run through the callback table and execute any pending
    Callbacks.
    */
    while (maSci2TxCallbackLeadingIndex != maSci2TxCallbackTrailingIndex) {    
       pfCallBack = maSci2TxCallbackTable[maSci2TxCallbackTrailingIndex].pfCallBack;
      /*Call calback with buffer info as parameter*/
       pfCallBack(maSci2TxCallbackTable[maSci2TxCallbackTrailingIndex].pBuf);
      /*increment and wrap around trailing index*/
      if (++maSci2TxCallbackTrailingIndex >= NumberOfElements(maSci2TxCallbackTable)) {
        maSci2TxCallbackTrailingIndex = 0;    
      }
    } /* while */
  } /*if events ...*/

#endif gUart2_Enabled_d
}


/*****************************************************************************/


/* Interrupts service routines (for QE128 these routines are placed in non-paged memory) */
/*****************************************************************************/
/* Place it in NON_BANKED memory */
#ifdef MEMORY_MODEL_BANKED
#pragma CODE_SEG __NEAR_SEG NON_BANKED
#else
#pragma CODE_SEG DEFAULT
#endif /* MEMORY_MODEL_BANKED */
#if gUart1_Enabled_d
INTERRUPT_KEYWORD void Uart1_ErrorIsr(void) {
  uint8_t dummy;
  while (SCI1S1 & (mSCIxS1_OR_c | mSCIxS1_NF_c | mSCIxS1_FE_c | mSCIxS1_PF_c)) {
    dummy = SCI1D;
  }
}                                       /* Uart1_ErrorIsr() */
#endif

/*****************************************************************************/

#if gUart2_Enabled_d
INTERRUPT_KEYWORD void Uart2_ErrorIsr(void) {
  uint8_t dummy;
  while (SCI2S1 & (mSCIxS1_OR_c | mSCIxS1_NF_c | mSCIxS1_FE_c | mSCIxS1_PF_c)) {
    dummy = SCI2D;
  }
}                                       /* Uart2_ErrorIsr() */
#endif

/*****************************************************************************/
#ifdef PROCESSOR_MC1323X 
extern uint8_t mLPMFlag;
extern volatile uint8_t enetredStop3;
volatile uint8_t uartWakeUp;
#endif

/* Rx ISR. If there is room, store the byte in the Rx circular buffer. */
#if gUart1_Enabled_d
INTERRUPT_KEYWORD void Uart1_RxIsr(void) {
  uint8_t dummy;

  /* The HCS08 does not save H on interrupt. */
  __asm {
    PSHH
  }

#ifdef PROCESSOR_MC1323X 
if (SCI1S2 & (1 << 6))
{
	SCI1S2 = SCI1S2;
	PWRLib_MCU_WakeupReason.Bits.FromSCI = TRUE;
}
#endif 
  
#if gUart_Debug_d
  if (mSci1RxBufferByteCount > uart1RxBufHighWaterMark) {
    uart1RxBufHighWaterMark = mSci1RxBufferByteCount;
  }

/*   if (mSci1RxBufferByteCount >= sizeof(maSci1RxBuf)) { */
/*     /\* Out of room in the Rx buffer. *\/ */
/*     while (mSci1RxBufferByteCount);    /\* assert(). *\/ */
/*     __asm { */
/*       PULH */
/*     } */
/*     return; */
/*   } */
#endif

  /* Receive register full and no errors */
  if ((SCI1S1 & (mSCIxS1_RDRF_c | mSCIxS1_OR_c | mSCIxS1_NF_c | mSCIxS1_FE_c | mSCIxS1_PF_c)) == mSCIxS1_RDRF_c) 
  {
    maSci1RxBuf[mSci1RxBufLeadingIndex] = SCI1D;

    if (++mSci1RxBufLeadingIndex >= sizeof(maSci1RxBuf)) 
    {
      mSci1RxBufLeadingIndex = 0;
    }

    if (mSci1RxBufferByteCount < sizeof(maSci1RxBuf)) 
    {
      ++mSci1RxBufferByteCount;
    }
    else 
    {
      /* A new character was received while the buffer is already full. 
       * The oldest character in the buffer has been overwritten. 
       * The trailing index must be also incremented to compensate the overwritten characters
       */
       if(++mSci1RxBufTrailingIndex >= sizeof(maSci1RxBuf)) 
       {
        mSci1RxBufTrailingIndex = 0;
       }
    }
    

#if gUart1_EnableHWFlowControl_d
    if (sizeof(maSci1RxBuf) - mSci1RxBufferByteCount <= gUart_RxFlowControlSkew_d) 
    {
      Uart1_DisableFlow();
    }
#endif

    /* Let the application know a byte has been received. */
    TS_SendEvent(gUartTaskId, gRxSci1Event_c);
  }
  else 
  {
    dummy = SCI1D;                      /* Clear the error. */
  }

  __asm {
    PULH
  }
}                                       /* Uart1_RxIsr() */
#endif

/**************************************/

#if gUart2_Enabled_d
INTERRUPT_KEYWORD void Uart2_RxIsr(void) {
  uint8_t dummy;

  /* The HCS08 does not save H on interrupt. */
  __asm {
    PSHH
  }

#if gUart_Debug_d
  if (mSci2RxBufferByteCount > uart2RxBufHighWaterMark) {
    uart2RxBufHighWaterMark = mSci2RxBufferByteCount;
  }

/*   if (mSci2RxBufferByteCount >= sizeof(maSci2RxBuf)) { */
/*     /\* Out of room in the Rx buffer. *\/ */
/*     while (mSci2RxBufferByteCount);    /\* assert(). *\/ */
/*     __asm { */
/*       PULH */
/*     } */
/*     return; */
/*   } */
#endif

  /* Receive register full and no errors */
  if ((SCI2S1 & (mSCIxS1_RDRF_c | mSCIxS1_OR_c | mSCIxS1_NF_c | mSCIxS1_FE_c | mSCIxS1_PF_c)) == mSCIxS1_RDRF_c) 
  {
    maSci2RxBuf[mSci2RxBufLeadingIndex] = SCI2D;

    if (++mSci2RxBufLeadingIndex >= sizeof(maSci2RxBuf)) 
    {
      mSci2RxBufLeadingIndex = 0;
    }

    if(mSci2RxBufferByteCount < sizeof(maSci2RxBuf)) 
    {
      ++mSci2RxBufferByteCount;
    }
    else 
    {
      /* A new character was received while the buffer is already full. 
       * The oldest character in the buffer has been overwritten. 
       * The trailing index must be also incremented to compensate the overwritten characters
       */
       if (++mSci2RxBufTrailingIndex >= sizeof(maSci2RxBuf)) 
       {
         mSci2RxBufTrailingIndex = 0;
       }
    }
    
#if gUart2_EnableHWFlowControl_d
    if (sizeof(maSci2RxBuf) - mSci2RxBufferByteCount <= gUart_RxFlowControlSkew_d) {
      Uart2_DisableFlow();
    }
#endif

    /* Let the application know a byte has been received. */
    TS_SendEvent(gUartTaskId, gRxSci2Event_c);
  }
  else {
    dummy = SCI2D;                      /* Clear the error. */
  }

  __asm {
    PULH
  }
}                                       /* Uart2_RxIsr() */
#endif

/* Transmit ISR. */
#if gUart1_Enabled_d
INTERRUPT_KEYWORD void Uart1_TxIsr(void) {
  uint8_t dummy;
  void (*pfCallBack)(unsigned char const *pBuf);

  /* The HCS08 does not save H on interrupt. */
  __asm {
    PSHH
  }

  /* Write a byte. */
  dummy = SCI1S1;                       /* Required before write to data register. */
  SCI1D = maSci1TxBufRefTable[mSci1TxBufRefTrailingIndex].pBuf[mSci1TxCurIndex];

  /* Finished with this buffer? */
  if (++mSci1TxCurIndex >= maSci1TxBufLenTable[mSci1TxBufRefTrailingIndex]) {

    /* Mark this one as done, and call the callback. */
    maSci1TxBufLenTable[mSci1TxBufRefTrailingIndex] = 0;
    pfCallBack = maSci1TxBufRefTable[mSci1TxBufRefTrailingIndex].pfCallBack;
    if (pfCallBack) {
      /*Signal uart task that we got a callback to be executed*/
      TS_SendEvent(gUartTaskId, gTxSci1Event_c);
      /*add callback information to callback table*/
      maSci1TxCallbackTable[maSci1TxCallbackLeadingIndex].pfCallBack = pfCallBack;
      maSci1TxCallbackTable[maSci1TxCallbackLeadingIndex].pBuf = maSci1TxBufRefTable[mSci1TxBufRefTrailingIndex].pBuf;
      /*increment and wrap around leading index*/        
      if (++maSci1TxCallbackLeadingIndex >= NumberOfElements(maSci1TxBufRefTable)) {
        maSci1TxCallbackLeadingIndex = 0;
      }
    
    }
    /*Set Tx Complete flag*/
    Uart1_TxCompleteFlag = TRUE;

    /* Increment to the next buffer. */
    mSci1TxCurIndex = 0;
    if (++mSci1TxBufRefTrailingIndex >= NumberOfElements(maSci1TxBufRefTable)) {
      mSci1TxBufRefTrailingIndex = 0;
    }

    /* If there is no more data to send, turn off the transmit interrupt. */
    if (!maSci1TxBufLenTable[mSci1TxBufRefTrailingIndex]) {
      SCI1C2 &= ~mSCIxC2_TIE_c;
    }
  }

  __asm {
    PULH
  }
}                                       /* Uart1_TxIsr() */
#endif

/**************************************/

#if gUart2_Enabled_d
INTERRUPT_KEYWORD void Uart2_TxIsr(void) {
  uint8_t dummy;
  void (*pfCallBack)(unsigned char const *pBuf);
 
  /* The HCS08 does not save H on interrupt. */
  __asm {
    PSHH
  }

  /* Write a byte. */
  dummy = SCI2S1;                       /* Required before write to data register. */
  SCI2D = maSci2TxBufRefTable[mSci2TxBufRefTrailingIndex].pBuf[mSci2TxCurIndex];

  /* Finished with this buffer? */
  if (++mSci2TxCurIndex >= maSci2TxBufLenTable[mSci2TxBufRefTrailingIndex]) {

    /* Mark this one as done, and call the callback. */
    maSci2TxBufLenTable[mSci2TxBufRefTrailingIndex] = 0;
    pfCallBack = maSci2TxBufRefTable[mSci2TxBufRefTrailingIndex].pfCallBack;

    if (pfCallBack) {
      /*Signal uart task that we got a callback to be executed*/
      TS_SendEvent(gUartTaskId, gTxSci2Event_c);
      /*add callback information to callback table*/
      maSci2TxCallbackTable[maSci2TxCallbackLeadingIndex].pfCallBack = pfCallBack;
      maSci2TxCallbackTable[maSci2TxCallbackLeadingIndex].pBuf = maSci2TxBufRefTable[mSci2TxBufRefTrailingIndex].pBuf;
      /*increment and wrap around leading index*/     
      if (++maSci2TxCallbackLeadingIndex >= NumberOfElements(maSci2TxBufRefTable)) {
        maSci2TxCallbackLeadingIndex = 0;
      }
    
    }
    /*Set Tx Complete flag*/
    Uart2_TxCompleteFlag = TRUE;
    
    /* Increment to the next buffer. */
    mSci2TxCurIndex = 0;
    if (++mSci2TxBufRefTrailingIndex >= NumberOfElements(maSci2TxBufRefTable)) {
      mSci2TxBufRefTrailingIndex = 0;
    }

    /* If there is no more data to send, turn off the transmit interrupt. */
    if (!maSci2TxBufLenTable[mSci2TxBufRefTrailingIndex]) {
      SCI2C2 &= ~mSCIxC2_TIE_c;
    }
  }  

  __asm {
    PULH
  }
}                                       /* Uart2_TxIsr() */
#endif

/*****************************************************************************/

#endif /* gUart1_Enabled_d || gUart2_Enabled_d */
#pragma CODE_SEG DEFAULT


/* Initialize the uart task.*/
void Uart_TaskInit(void)
{
#if gUart1_Enabled_d || gUart2_Enabled_d
  gUartTaskId = TS_CreateTask(gTsUartTaskPriority_c, Uart_Task);
#endif
}

/*****************************************************************************/
