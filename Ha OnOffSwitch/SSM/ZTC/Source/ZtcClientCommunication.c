/******************************************************************************
* ZTC routines to handle the ZTC <--> external client protocol.
*
* (c) Copyright 2011, Freescale, Inc. All rights reserved.
*
*
* No part of this document must be reproduced in any form - including copied,
* transcribed, printed or by any electronic means - without specific written
* permission from Freescale Semiconductor.
*
******************************************************************************/
/* Generic headers */
#include "EmbeddedTypes.h"
#include "FunctionLib.h"

/* BeeStack headers */
#include "ZigBee.h"
#include "BeeStackFunctionality.h"
#include "BeeStackConfiguration.h"
#include "BeeCommon.h"

/* ZTC headers */
#include "ZtcInterface.h"
#include "ZtcPrivate.h"
#include "ZtcClientCommunication.h"

/* Support for SCI/UART ZTC Interface */
#include "Uart_Interface.h"

/* Support for I2C ZTC Interface */ 
#if gI2CInterface_d
  #include "IIC_Interface.h"
#endif

/* Support for SPI ZTC Interface */ 
#if gSPIInterface_d
  #include "SPI_Interface.h"
  #include "SPI.h"
#endif

/* File contents compilation/inclusion conditioned by ZTC being included */
#if gZtcIncluded_d

/******************************************************************************
*******************************************************************************
* Private macros
*******************************************************************************
******************************************************************************/
#define mNumCyclesToAgeRxBuffer_c 100
#define mMaxTxShotSize_c          255

/* Minimum length of a ZTC packet: 
*       clientPacketHdr_t = OpcodeGroup + OpCode + Length:  3 bytes;          */
/*      clientPacketChecksum_t:  1 byte;                                      */
/*  Total min len for Beestack: 4 bytes                                       */
#define mMinValidPacketLen_c      (sizeof(clientPacketHdr_t) + sizeof(clientPacketChecksum_t))

/* Definitions of generic communication APIs for all 3 supported ZTC protocols */
#if gSCIInterface_d

  #define ZtcComm_GetByteFromRxBuffer   UartX_GetByteFromRxBuffer
  #define ZtcComm_Transmit              UartX_Transmit
  #define ZtcComm_IsTxActive            UartX_IsTxActive
  #define ZtcComm_SetRxCallBack         UartX_SetRxCallBack

#elif gSPIInterface_d 

  #define ZtcComm_GetByteFromRxBuffer   SPI_GetByteFromBuffer
  #define ZtcComm_Transmit              SPI_SlaveTransmit
  #define ZtcComm_SetRxCallBack         SPI_SetSlaveRxCallBack
  #define ZtcComm_IsTxActive            SPI_IsSlaveTxActive


#elif gI2CInterface_d                    

  #define ZtcComm_GetByteFromRxBuffer   IIC_GetByteFromRxBuffer
  #define ZtcComm_Transmit              IIC_Transmit_Slave
  #define ZtcComm_SetRxCallBack         IIC_SetSlaveRxCallBack
  #define ZtcComm_IsTxActive            IIC_IsSlaveTxActive

#endif

/* Expected value of the start of frame delimiter/synchronization byte  - currently 0x02 */
#define mStxValue_c     ((unsigned char) 0x02)

/******************************************************************************
*******************************************************************************
* Private type definitions
*******************************************************************************
******************************************************************************/
typedef uint8_t clientPacketChecksum_t;

/******************************************************************************
*******************************************************************************
* Private memory declarations
*******************************************************************************
******************************************************************************/

/******************************************************************************
*******************************************************************************
* Public memory declarations
*******************************************************************************
******************************************************************************/

/* Flag storing if the ZTC to client transmit buffer is busy */
volatile bool_t bZtcPacketToClientIsBusy;

/* UART packet received from the external client, minus the interface envelope. */
clientPacket_t gZtcPacketFromClient;

/* UART packet to be sent to external client, minus the interface envelope. */
clientPacket_t  gZtcPacketToClient;

/* Flag storing if the start of a frame was received on the serial interface */
bool_t          bStartOfFrameSeen = FALSE;

/* Number of received bytes over the serial interface. Does not count the STX */
index_t         bytesReceived     = 0;       

/* If true, ZtcWritePacketToClient() will block after enabling the interface Tx */
/* interrupt, until the interface driver's output buffer is empty again. */
#if (gZtcDebug_d == TRUE)
  bool_t gZtcCommTxBlocking = TRUE;
#else
  bool_t gZtcCommTxBlocking = FALSE;
#endif

/******************************************************************************
*******************************************************************************
* Private function prototypes
*******************************************************************************
******************************************************************************/
static void ZtcComm_RxCallback(void);
static void ZtcComm_WritePacketCallback(unsigned char const *pBuf);
static uint8_t ZtcComm_ZtcCheckPacketFromClient(void);
static void ZtcComm_SyncOnFirstFoundStxByte(void);
static void ZtcComm_TransmitShortBuffer(
                                  uint8_t const*  pTxLocation,
                                  uint8_t msgLength, 
                                  void (*pfCallBack)(unsigned char const *pBuf) 
                                 );

/******************************************************************************
*******************************************************************************
* Public functions
*******************************************************************************
******************************************************************************/

/*****************************************************************************
* ZtcComm_Init()
*
* Initializes the ZTC to work on either SCI, SPI, or I2C
*
*****************************************************************************/
void ZtcComm_Init()
{


#if gSCIInterface_d
  #if defined(gBlackBox_d)
	/* Set UART baudrate */
	UartX_SetBaud(BlackBoxConfig.BaudRate);
  #endif
#endif

#if gSPIInterface_d
  /* Configure SPI interface */
  spiConfig_t mSpiConfig;
   
  /* Prepare the SPI module in slave mode */
  mSpiConfig.devMode      = gSPI_SlaveMode_c;
  #if defined(gBlackBox_d)
	mSpiConfig.baudRate     = BlackBoxConfig.SPIBaudRate;
  #else
	mSpiConfig.baudRate     = gSPI_BaudRate_100000_c;
  #endif
  
  mSpiConfig.clockPol     = gSPI_ActiveHighPolarity_c;
  mSpiConfig.clockPhase   = gSPI_EvenEdgeShifting_c;
  mSpiConfig.bitwiseShift = gSPI_MsbFirst_c;
  mSpiConfig.s3Wire       = gSPI_FullDuplex_c;
    
  /* Apply configuration to the SPI module */
  SPI_SetConfig(mSpiConfig);
#endif


#if gI2CInterface_d
  #if defined(gBlackBox_d)
    /* Set slave address of the I2C module */
	(void)IIC_SetSlaveAddress(BlackBoxConfig.I2CSlaveId);
  #endif
#endif

  /* Set UART Rx callback function */
  ZtcComm_SetRxCallBack(ZtcComm_RxCallback);
}


/*****************************************************************************
* ZtcComm_BufferIsEmpty
*
* Checks the buffer status
******************************************************************************/  
bool_t ZtcComm_BufferIsEmpty(void)
{
#if gSCIInterface_d
  return (0 == UartX_RxBufferByteCount);
#elif gI2CInterface_d
  return (0 == mIIcRxBufferByteCount);
#elif gSPIInterface_d
  return (0 == mSpiRxBufferByteCount);  
#endif  
}

/*****************************************************************************
* ZtcComm_WritePacketToClient
*
* Writes a packet over the serial interface
******************************************************************************/
void ZtcComm_WritePacketToClient(uint8_t const length) 
{
  bZtcPacketToClientIsBusy = TRUE;  
  
  /* Send packet to client */
  ZtcComm_ZtcTransmitBuffer(gZtcPacketToClient.raw, length, ZtcComm_WritePacketCallback);
}


/*****************************************************************************
* ZtcComm_TransmitBufferBlock
*
* Send an arbitrary buffer to the external client. Block until the entire 
* buffer has been sent, regardless of the state of the gZtcCommTxBlocking 
* flag.
******************************************************************************/
void ZtcComm_TransmitBufferBlock(uint8_t const *pSrc, uint8_t const length) 
{
  ZtcComm_ZtcTransmitBuffer(pSrc, length, NULL);
  
  /* Wait until previous transmissions complete */
  while (ZtcComm_IsTxActive()); 
}


/*****************************************************************************
* ZtcComm_CheckPacket
*
* Checks if the received packet is valid. 
*****************************************************************************/
ztcPacketStatus_t ZtcComm_CheckPacket(void) 
{
  index_t                 i;
  uint8_t                payloadLength;
  clientPacketChecksum_t  checksum = 0;

  if (bytesReceived < mMinValidPacketLen_c) 
  {
    /* Too short to be valid. */
    return packetIsTooShort;
  }

  if (bytesReceived >= sizeof(gZtcPacketFromClient)) 
  {
    return framingError;
  }

  /* 
   * The packet's payloadLength field does not count:
   *  - STX               1 byte
   *  - opcodeGroup       1 byte
   *  - msgType           1 byte
   *  - payloadLength     2 byte
   *  - checksum          1 byte
   */
  payloadLength = gZtcPacketFromClient.structured.header.len;
  if (bytesReceived < payloadLength + sizeof(clientPacketHdr_t) + sizeof(checksum)) 
  {
    return packetIsTooShort;
  }

  /* 
   * If the length appears to be too long, it might be because the external
   * client is sending a packet that is too long, or the sync is lost with the external client. 
   * Assuming that the sync is lost.
   */
  if (payloadLength > sizeof(gZtcPacketFromClient.structured.payload)) 
  {
    return framingError;
  }

  /* If the length looks right, make sure that the checksum is correct. */
  if (bytesReceived == payloadLength + sizeof(clientPacketHdr_t) + sizeof(checksum)) 
  {
    for (checksum = 0, i = 0; i < payloadLength + sizeof(clientPacketHdr_t); ++i) 
    {
      checksum ^= gZtcPacketFromClient.raw[i];
    }
    if (checksum == gZtcPacketFromClient.structured.payload[payloadLength]) 
    {
      return packetIsValid;
    }
  }
  return framingError;
}  


/*****************************************************************************
* ZtcComm_ZtcReadPacketFromClient
*
* Read data from the interface. If the ISR has received a complete packet
* (STX, packet header, and FCS), copy it to the ZTC global
* gZtcPacketFromClient buffer and return true. Otherwise return false.
* This routine is called each time a byte is received from the interface.
* Client packets consist of an interface envelope enclosing a variable length
* clientPacket_t.
* The envelope is a leading sync (STX) byte and a trailing FCS.
******************************************************************************/
bool_t ZtcComm_ZtcReadPacketFromClient(void) 
{
  uint8_t           byte;
  static uint8_t    cyclesToAgeRxBuffer = mNumCyclesToAgeRxBuffer_c;
  
  /* Try to process first the bytes in the interface buffer, if any */
  if (!ZtcComm_BufferIsEmpty()) 
  {
    /* There are bytes to received, so init the cycles for ageing the RxBuffer */
    cyclesToAgeRxBuffer = mNumCyclesToAgeRxBuffer_c;
    
    while (ZtcComm_GetByteFromRxBuffer(&byte)) 
    {
      if (!bStartOfFrameSeen) 
    	{
    	  /* Don't store the STX in the buffer. */
        bytesReceived     = 0;
        bStartOfFrameSeen = (byte == mStxValue_c);
        break;                            
      }
      gZtcPacketFromClient.raw[bytesReceived++] = byte;

      if (ZtcComm_ZtcCheckPacketFromClient() == packetIsValid) 
      {
        return TRUE;
      }
    } /* end while */                   
  } 
  else 
  {
    /* No bytes in the buffer. Check if the gZtcPacketFromClient buffer has bytes that are 
    still not processed */
    if (bytesReceived)
    {
      /* The code below implements a kind of aging timer. 
         The arrival of bytes from the client will trigger the call to this function, based on an event.
         If at some point in time the communication with the client get desynchronized, the ZTC
      application will try to resync on next STX byte found in the client buffer. The ZTC interprets 
      the bytes following the STX as being the length of the frame to be received. If the byte ZTC
      has synchronized on is not a real STX, but just a payload byte having same value as STX, the 
      length of the frame can be missinterpretted. The worse case scenario is when the length of the
      frame is bigger than the number of bytes already received. In this case the ZTC will block waiting
      for more information from the client, altough the already received bytes contain a valid ZTC
      frame that starts after the STX ZTC is currently synchronized on.
         The mecanism below will age the bytes already received. If no other bytes are received
      from the client in a certain amount of time, the ZTC will try to resynchronize again, on the next
      STX byte. The amount of time used for the aging is obtained by running the code below for a defined
      number of times, instead of using a timer. */
      cyclesToAgeRxBuffer --;
      
      if(cyclesToAgeRxBuffer == 0) 
      {
        cyclesToAgeRxBuffer = mNumCyclesToAgeRxBuffer_c;
        /* Try to synchronize ZTC on next STX byte in the client buffer */
        ZtcComm_SyncOnFirstFoundStxByte();
        /* Check if what we have in the Rx buffer is a valid ZTC frame */
        if (ZtcComm_ZtcCheckPacketFromClient() == packetIsValid) 
        {
          return TRUE;
        }
      }
    }
    else
    {
      cyclesToAgeRxBuffer = mNumCyclesToAgeRxBuffer_c; 
    }
  }
  return FALSE;        
} 


/*****************************************************************************
* ZtcComm_ZtcTransmitBuffer
*
* Send an arbitrary buffer to the external client
******************************************************************************/
void ZtcComm_ZtcTransmitBuffer(
                        uint8_t const*  pSrc,
                        uint8_t const  length,
                        void (*pfCallBack)(unsigned char const *pBuf)
                       )
{
  uint8_t stxValue[4];
  uint8_t checksum = 0;
  uint8_t i;
  
  /* Create first buffer */
  stxValue[0] = mStxValue_c;    /* Stx */
  stxValue[1] = pSrc[0];        /* OpCode Group */
  stxValue[2] = pSrc[1];        /* OpCode */
  stxValue[3] = pSrc[2];        /* Length */
  
  /* Send the start of the frame (for I2C) */  
  ZtcComm_TransmitShortBuffer(stxValue, 4, NULL);  

  /* Stay here until the message is placed in the UART/SPI/I2C queue */
  if(length-3)
    ZtcComm_TransmitShortBuffer(&pSrc[3], length-3, NULL);

  /* Compute checkSum */
  for (i = 0; i < length; ++i) 
  {
    checksum ^= pSrc[i];
  }

  /* Send the checkSum of the frame */
  ZtcComm_TransmitShortBuffer(&checksum, sizeof(uint8_t), NULL);
  
  /* Execute callback manually */
  if (pfCallBack) 
    pfCallBack(NULL);  
  
}

/******************************************************************************
*******************************************************************************
* Private functions
*******************************************************************************
******************************************************************************/


/*****************************************************************************
* ZtcComm_RxCallBack
*
* Function to be called each time information is received over the interface
* with the host
*****************************************************************************/
void ZtcComm_RxCallback(void) 
{
  TS_SendEvent(gZTCTaskID, gDataFromTestClientToZTCEvent_c);
}


/*****************************************************************************
* ZtcComm_WritePacketCallBack
*
* Communication interface write operation callback
******************************************************************************/
void ZtcComm_WritePacketCallback(unsigned char const *pBuf) 
{
  /* Avoid compiler warning */
  (void) pBuf;
  bZtcPacketToClientIsBusy = FALSE;
}

/*****************************************************************************/
static uint8_t ZtcComm_ZtcCheckPacketFromClient(void)
{
  uint8_t status = framingError;
  
  while (bytesReceived) 
  {
    status = ZtcComm_CheckPacket();
    if (status == packetIsValid) 
  	{
      bStartOfFrameSeen = FALSE;
      bytesReceived     = 0;
      return status;
    }

    if (status == packetIsTooShort) 
      return status;
    
    ZtcComm_SyncOnFirstFoundStxByte();
  } 
  return status;
}

/*****************************************************************************/
static void ZtcComm_SyncOnFirstFoundStxByte(void)
{
  uint8_t i;
  
  bStartOfFrameSeen = FALSE;
  
  for (i = 0; i < bytesReceived; ++i) 
	{
    if (gZtcPacketFromClient.raw[i] == mStxValue_c) 
		{
		  /* STX byte found. Bring all bytes following the STX in the beginning of the 
		  client buffer */
      bytesReceived     -= (i + 1);
      FLib_MemCpy(gZtcPacketFromClient.raw, gZtcPacketFromClient.raw + i + 1, bytesReceived);
      bStartOfFrameSeen  = TRUE;
      break;
    }
  }
  /* If the STX byte was not found, drop all bytes we have in the client buffer */
  if(bStartOfFrameSeen == FALSE) 
  {
    bytesReceived     = 0;
  }
}


/*****************************************************************************
* ZtcComm_TransmitShortBuffer - does actual data transmission to interface
*
******************************************************************************/
static void ZtcComm_TransmitShortBuffer(uint8_t const*  pBuffer,
                                        uint8_t msgLength, 
                                        void (*pfCallBack)(unsigned char const *pBuf) 
                                     )
{
  while (!ZtcComm_Transmit(pBuffer, msgLength, pfCallBack));   
#if gI2CInterface_d
   /* Signal to the master that there is data to be sent. */
  IIC_TxDataAvailable(TRUE);
#endif
 
  /* Wait for the completion of the tx operation */
  if(gZtcCommTxBlocking) 
    while (ZtcComm_IsTxActive()); 
}

#endif /* gZtcIncluded_d */
