/*****************************************************************************
* UART / SCI / USB / Serial Port driver declarations.
* 
* This driver supports both SCI1 and SCI2 on the HCS08, each of which can be
* enabled or disabled independantly. On the EVB, SRB and NCB boards, the USB
* (SCI2) port is used. On the SARD, SCI1 (9-pin serial) port is used.
*
* Copyright (c) 2006, Freescale, Inc. All rights reserved.
*
*
* No part of this document must be reproduced in any form - including copied,
* transcribed, printed or by any electronic means - without specific written
* permission from Freescale Semiconductor.
*
*****************************************************************************/

/* On the receive side, this module keeps a small circular buffer, and calls
 * a callback function whenever a byte is received. The application can
 * retrieve bytes from the buffer at it's convenience, as long as it does so
 * before the driver's buffer fills up.
 *
 * On the transmit side, this module keeps a list of buffers to be
 * transmitted, and calls an application callback function when the entire
 * buffer has been sent. The application is responsible for ensuring that the
 * buffer is available until the callback function is called.
 *
 * If both SCI ports are enabled, each has it's own Rx circular buffer and
 * list of Tx buffers.
 */

#ifndef _Uart_Interface_h_
#define _Uart_Interface_h_

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

/* Default setting for debugging code (includes high-water mark checking). */
#ifndef gUart_Debug_d
#define gUart_Debug_d       FALSE
#endif

/*****************************************************************************/

/* The HCS08 has two SCI/UART hardware interfaces. Define the one(s) in use. */
/* The MC1323X has one SCI/UART hardware interfaces */
#ifdef PROCESSOR_MC1323X

 #ifndef gUart1_Enabled_d
  #define gUart1_Enabled_d    FALSE
 #endif

 #ifndef gUart2_Enabled_d
  #define gUart2_Enabled_d    TRUE
 #endif

 #if(gUart2_Enabled_d == TRUE )
  #error "UART2 does not exist"
 #endif

#else //PROCESSOR_HCS08

 #ifndef gUart1_Enabled_d
  #define gUart1_Enabled_d    FALSE
 #endif

 #ifndef gUart2_Enabled_d
  #define gUart2_Enabled_d    TRUE
 #endif

#endif 


/* events for uart task */
#define gRxSci1Event_c  (1<<0)
#define gRxSci2Event_c  (1<<1)
#define gTxSci1Event_c  (1<<2)
#define gTxSci2Event_c  (1<<3)


/* set up ISR vectors for UARTs */

#if gUart1_Enabled_d
#define gUart1_ErrorIsr_c   Uart1_ErrorIsr
#define gUart1_RxIsr_c      Uart1_RxIsr
#define gUart1_TxIsr_c      Uart1_TxIsr
#else
#define gUart1_ErrorIsr_c   Default_Dummy_ISR
#define gUart1_RxIsr_c      Default_Dummy_ISR
#define gUart1_TxIsr_c      Default_Dummy_ISR
#endif

#if gUart2_Enabled_d
#define gUart2_ErrorIsr_c   Uart2_ErrorIsr
#define gUart2_RxIsr_c      Uart2_RxIsr
#define gUart2_TxIsr_c      Uart2_TxIsr
#else
#define gUart2_ErrorIsr_c   Default_Dummy_ISR
#define gUart2_RxIsr_c      Default_Dummy_ISR
#define gUart2_TxIsr_c      Default_Dummy_ISR
#endif


/* Which port is used by default? */
/* If a default port is not defined */
#ifndef gUart_PortDefault_d
#if gUart2_Enabled_d
#define gUart_PortDefault_d     2
#else
#define gUart_PortDefault_d     1
#endif
#endif

/*****************************************************************************/

/* Use hardware flow control? */

#ifndef gUart1_EnableHWFlowControl_d
#define gUart1_EnableHWFlowControl_d     TRUE
#endif

#ifndef gUart2_EnableHWFlowControl_d
#define gUart2_EnableHWFlowControl_d     TRUE
#endif

/*****************************************************************************/

/* Tuning definitions. */

/* Number of entries in the transmit-buffers-in-waiting list. */
#ifndef gUart_TransmitBuffers_c
#define gUart_TransmitBuffers_c     3
#endif

/* Size of the driver's Rx circular buffer. These buffers are used to */
/* hold received bytes until the application can retrieve them via the */
/* UartX_GetBytesFromRxBuffer() functions, and are not otherwise accessable */
/* from outside the driver. The size does not need to be a power of two. */
#ifndef gUart_ReceiveBufferSize_c
#define gUart_ReceiveBufferSize_c   32
#endif

/* If flow control is used, there is a delay before telling the far side */
/* to stop and the far side actually stopping. When there are SKEW bytes */
/* remaining in the driver's Rx buffer, tell the far side to stop */
/* transmitting. */
#ifndef gUart_RxFlowControlSkew_d
#define gUart_RxFlowControlSkew_d   8
#endif

/* Number of bytes left in the Rx buffer when hardware flow control is */
/* deasserted. */
#ifndef gUart_RxFlowControlResume_d
#define gUart_RxFlowControlResume_d 8
#endif

#if gUart_RxFlowControlResume_d >= (gUart_ReceiveBufferSize_c - gUart_RxFlowControlSkew_d)
#error Deassert flow control before it is asserted?
#endif

/*****************************************************************************/

/* SCI baud rate = BUSCLK/(16?Baud rate)
 * Note that not all baud rates work for all clock rates. The actual baud rate
 * may deviate too much from the RS232 specification.
 */
#ifndef gSystemClock_d
#define gSystemClock_d     16           /* 16 MHz. */
#endif

typedef uint16_t UartBaudRate_t;
#ifdef PROCESSOR_MC1323X
  typedef uint8_t UartBaudRateDiv_t; 
#endif
#if gSystemClock_d == 8
#define gUARTBaudRate1200_c     ((UartBaudRate_t) 0x01A1)
#define gUARTBaudRate2400_c     ((UartBaudRate_t) 0x00D0)
#define gUARTBaudRate4800_c     ((UartBaudRate_t) 0x0068)
#define gUARTBaudRate9600_c     ((UartBaudRate_t) 0x0034)
#define gUARTBaudRate19200_c    ((UartBaudRate_t) 0x001A)
#define gUARTBaudRate38400_c    ((UartBaudRate_t) 0x0011)
#define gUARTBaudRate57600_c    ((UartBaudRate_t) 0x0009)
#define gUARTBaudRate115200_c   ((UartBaudRate_t) 0x0004)   /* Might not work for all clients */
#endif

#if gSystemClock_d == 12
#define gUARTBaudRate1200_c     ((UartBaudRate_t) 0x0271)
#define gUARTBaudRate2400_c     ((UartBaudRate_t) 0x0139)
#define gUARTBaudRate4800_c     ((UartBaudRate_t) 0x009C)
#define gUARTBaudRate9600_c     ((UartBaudRate_t) 0x004E)
#define gUARTBaudRate19200_c    ((UartBaudRate_t) 0x0027)
#define gUARTBaudRate38400_c    ((UartBaudRate_t) 0x0014)
#define gUARTBaudRate57600_c    ((UartBaudRate_t) 0x000D)
#define gUARTBaudRate115200_c   ((UartBaudRate_t) 0x0007)
#endif

#if gSystemClock_d == 16
  #ifdef PROCESSOR_MC1323X
    #define gUARTBaudRate1200_c     ((UartBaudRate_t) 0x0341)
    #define gUARTBaudRate2400_c     ((UartBaudRate_t) 0x01A0)     
    #define gUARTBaudRate4800_c     ((UartBaudRate_t) 0x00D0)
    #define gUARTBaudRate9600_c     ((UartBaudRate_t) 0x0068)
    #define gUARTBaudRate19200_c    ((UartBaudRate_t) 0x0034)
    #define gUARTBaudRate38400_c    ((UartBaudRate_t) 0x001A)
    #define gUARTBaudRate57600_c    ((UartBaudRate_t) 0x0011)
    #define gUARTBaudRate115200_c   ((UartBaudRate_t) 0x0008)   /* Might not work for all clients */

    #define gUARTBaudRate1200Div_c    ((UartBaudRateDiv_t) 0x0B)
    #define gUARTBaudRate2400Div_c    ((UartBaudRateDiv_t) 0x15)   
    #define gUARTBaudRate4800Div_c    ((UartBaudRateDiv_t) 0x0B)
    #define gUARTBaudRate9600Div_c    ((UartBaudRateDiv_t) 0x05)  
    #define gUARTBaudRate19200Div_c   ((UartBaudRateDiv_t) 0x03)
    #define gUARTBaudRate38400Div_c   ((UartBaudRateDiv_t) 0x01)    
    #define gUARTBaudRate57600Div_c   ((UartBaudRateDiv_t) 0x0C)    
    #define gUARTBaudRate115200Div_c  ((UartBaudRateDiv_t) 0x16)     
  #else
    #define gUARTBaudRate1200_c     ((UartBaudRate_t) 0x0341)
    #define gUARTBaudRate2400_c     ((UartBaudRate_t) 0x01A1)     
    #define gUARTBaudRate4800_c     ((UartBaudRate_t) 0x00D0)
    #define gUARTBaudRate9600_c     ((UartBaudRate_t) 0x0068)
    #define gUARTBaudRate19200_c    ((UartBaudRate_t) 0x0034)
    #define gUARTBaudRate38400_c    ((UartBaudRate_t) 0x001A)
    #define gUARTBaudRate57600_c    ((UartBaudRate_t) 0x0011)
    #define gUARTBaudRate115200_c   ((UartBaudRate_t) 0x0009)   /* Might not work for all clients */
  #endif
#endif

#if gSystemClock_d == 16780
#define gUARTBaudRate1200_c     ((UartBaudRate_t) 0x036A)
#define gUARTBaudRate2400_c     ((UartBaudRate_t) 0x01B5)
#define gUARTBaudRate4800_c     ((UartBaudRate_t) 0x00DA)
#define gUARTBaudRate9600_c     ((UartBaudRate_t) 0x006D)
#define gUARTBaudRate19200_c    ((UartBaudRate_t) 0x0037)
#define gUARTBaudRate38400_c    ((UartBaudRate_t) 0x001B)
#define gUARTBaudRate57600_c    ((UartBaudRate_t) 0x0012)
#define gUARTBaudRate115200_c   ((UartBaudRate_t) 0x0009)
#endif


/*list of possible baudrates */
#define  Baudrate_1200   gUARTBaudRate1200_c
#define  Baudrate_2400   gUARTBaudRate2400_c
#define  Baudrate_4800   gUARTBaudRate4800_c
#define  Baudrate_9600   gUARTBaudRate9600_c
#define  Baudrate_19200  gUARTBaudRate19200_c
#define  Baudrate_38400  gUARTBaudRate38400_c
#define  Baudrate_57600  gUARTBaudRate57600_c
#define  Baudrate_115200 gUARTBaudRate115200_c

/* Default baud rate. */

#ifndef gUartDefaultBaud_c
#define gUartDefaultBaud_c  Baudrate_38400
#endif

/*****************************************************************************
******************************************************************************
* Public prototypes
******************************************************************************
*****************************************************************************/

#if !gUart1_Enabled_d && !gUart2_Enabled_d

/* stub functions if disabled */

#define Uart_ModuleInit()
#define Uart_ModuleUninit()
#define Uart_ClearErrors()
#define Uart1_SetBaud(baudRate)
#define Uart2_SetBaud(baudRate)
#define Uart1_Transmit(pBuf, bufLen, pfCallBack) FALSE
#define Uart2_Transmit(pBuf, bufLen, pfCallBack) FALSE
#define Uart1_IsTxActive() FALSE
#define Uart2_IsTxActive() FALSE
#define Uart1_SetRxCallBack(pfCallBack)
#define Uart2_SetRxCallBack(pfCallBack)
#define Uart1_GetByteFromRxBuffer(pDst) FALSE
#define Uart2_GetByteFromRxBuffer(pDst) FALSE
#define Uart1_UngetByte(byte)
#define Uart2_UngetByte(byte)

#else

#if gUart1_Enabled_d
/* Place it in NON_BANKED memory */
#ifdef MEMORY_MODEL_BANKED
#pragma CODE_SEG __NEAR_SEG NON_BANKED
#else
#pragma CODE_SEG DEFAULT
#endif /* MEMORY_MODEL_BANKED */
extern INTERRUPT_KEYWORD void Uart1_ErrorIsr(void);
extern INTERRUPT_KEYWORD void Uart1_RxIsr(void);
extern INTERRUPT_KEYWORD void Uart1_TxIsr(void);
#pragma CODE_SEG DEFAULT
extern bool_t Uart1_TxCompleteFlag;
extern volatile index_t mSci1RxBufferByteCount;      /* # of bytes in buffer. */
#endif

#if gUart2_Enabled_d
/* Place it in NON_BANKED memory */
#ifdef MEMORY_MODEL_BANKED
#pragma CODE_SEG __NEAR_SEG NON_BANKED
#else
#pragma CODE_SEG DEFAULT
#endif /* MEMORY_MODEL_BANKED */
extern INTERRUPT_KEYWORD void Uart2_ErrorIsr(void);
extern INTERRUPT_KEYWORD void Uart2_RxIsr(void);
extern INTERRUPT_KEYWORD void Uart2_TxIsr(void);
#pragma CODE_SEG DEFAULT
extern bool_t Uart2_TxCompleteFlag;
extern volatile index_t mSci2RxBufferByteCount;      /* # of bytes in buffer. */
#endif

/*Folowing functions are placed in paged memory for QE128 if gUART_PagedMem_d is TRUE*/


/* Initialize the UART module, including whichever ports are enabled. */
extern void Uart_ModuleInit(void);

/* Shut down the UART module. */
extern void Uart_ModuleUninit(void);

/* Clear any error flags seen by the UART driver. In rare conditions, an */
/* error flag can be set without triggering an error interrupt, and will */
/* prevent Rx and/or Tx interrupts from occuring. The most likely cause */
/* is a breakpoint set during debugging, when a UART port is active. */
/* Calling this function occasionaly allows the UART code to recover */
/* from these errors. */
extern void Uart_ClearErrors(void);

/* Set UART1 baud rate. */
extern void Uart1_SetBaud(UartBaudRate_t baudRate);

/* Set UART2 baud rate. */
extern void Uart2_SetBaud(UartBaudRate_t baudRate);

/* Transmit bufLen bytes of data from pBuffer over a port. Call *pfCallBack() */
/* when the entire buffer has been sent. Returns FALSE if there are no */
/* more available Tx buffer slots, TRUE otherwise. The caller must ensure */
/* that the buffer remains available until the call back function is called. */
/* pfCallBack must not be NULL. */
/* The callback function will be called in interrupt context, so it should */
/* be kept very short. */
extern bool_t Uart1_Transmit(unsigned char const *pBuf, index_t bufLen, void (*pfCallBack)(unsigned char const *pBuf));

extern bool_t Uart2_Transmit(unsigned char const *pBuf, index_t bufLen, void (*pfCallBack)(unsigned char const *pBuf));

/* Return TRUE if there are any bytes in transmit buffers that have not */
/* been sent yet. Return FALSE otherwise. */
extern bool_t Uart1_IsTxActive(void);

extern bool_t Uart2_IsTxActive(void);

/* Set the receive side callback function. This function will be called */
/* whenever a byte is received by the driver. The callback's bytesWaiting */
/* parameter is the number of bytes available in the driver's Rx buffer. */
/* The callback function will be called in interrupt context, so it should */
/* be kept very short. */
/* If the callback pointer is set to NULL, the Rx interrupt will be turned */
/* off, and all data in the driver's Rx buffer will be discarded. */
extern void Uart1_SetRxCallBack(void (*pfCallBack)(void));

extern void Uart2_SetRxCallBack(void (*pfCallBack)(void));

/* Retrieve one byte from the driver's Rx buffer and store it at *pDst. */
/* Return TRUE if a byte was retrieved; FALSE if the Rx buffer is empty. */
extern bool_t Uart1_GetByteFromRxBuffer(unsigned char *pDst);

extern bool_t Uart2_GetByteFromRxBuffer(unsigned char *pDst);

/* If a caller reads one too many characters, it can unget it. Calling */
/* the unget routine will store the byte to be read by the next call to */
/* UartX_GetByteFromRxBuffer(), and will also call the Rx call back. */

extern void Uart1_UngetByte(unsigned char byte);

extern void Uart2_UngetByte(unsigned char byte);

#endif /* !gUart1_Enabled_d && !gUart2_Enabled_d */

/* Declare the default API. If gUart_PortDefault_d is not defined, there is */
/* no default API. */
#if gUart_PortDefault_d == 1
#define UartX_Transmit              Uart1_Transmit
#define UartX_TransmitAndBlock      Uart1_TransmitAndBlock
#define UartX_IsTxActive            Uart1_IsTxActive
#define UartX_SetRxCallBack         Uart1_SetRxCallBack
#define UartX_GetRxBytesWaiting     Uart1_GetRxBytesWaiting
#define UartX_GetByteFromRxBuffer   Uart1_GetByteFromRxBuffer
#define UartX_SetBaud               Uart1_SetBaud
#define UartX_UngetByte             Uart1_UngetByte
#define UartX_TxCompleteFlag        Uart1_TxCompleteFlag
#define UartX_RxBufferByteCount     mSci1RxBufferByteCount
#endif

#if gUart_PortDefault_d == 2
#define UartX_Transmit              Uart2_Transmit
#define UartX_TransmitAndBlock      Uart2_TransmitAndBlock
#define UartX_IsTxActive            Uart2_IsTxActive
#define UartX_SetRxCallBack         Uart2_SetRxCallBack
#define UartX_GetRxBytesWaiting     Uart2_GetRxBytesWaiting
#define UartX_GetByteFromRxBuffer   Uart2_GetByteFromRxBuffer
#define UartX_SetBaud               Uart2_SetBaud
#define UartX_UngetByte             Uart2_UngetByte
#define UartX_TxCompleteFlag        Uart2_TxCompleteFlag
#define UartX_RxBufferByteCount     mSci2RxBufferByteCount
#endif

void Uart_Task(event_t events);
void Uart_TaskInit(void);

#endif _Uart_Interface_h_

