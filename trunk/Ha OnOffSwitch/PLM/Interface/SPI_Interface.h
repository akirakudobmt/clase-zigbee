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

#ifndef _SPI_Interface_h_
#define _SPI_Interface_h_   

#include "IrqControlLib.h"

/******************************************************************************
*******************************************************************************
* Public macros
*******************************************************************************
******************************************************************************/
/* Enable or disable the SPI module */
/* QE128 has two SPI modules */
#ifndef gSPI_Enabled_d
  #define gSPI_Enabled_d                     FALSE
#endif

                                                                                            ;
/* SPI Task priority: should have a medium level */
/* Note: The priority of the SPI task is configured here unless it not */
/*       defined in other location, like "Mac_Globals.h" */
#ifndef gTsSpiTaskPriority_c
  #define gTsSpiTaskPriority_c                0x12
#endif

/* Number of entries in the circular transmit buffer */
#ifndef gSPI_SlaveTransmitBuffersNo_c
  #define gSPI_SlaveTransmitBuffersNo_c       3
#endif

/* Size of the driver'sRx circular buffer */
#ifndef gSPI_SlaveReceiveBufferSize_c
  #define gSPI_SlaveReceiveBufferSize_c       32
#endif

/* SPI Baud Rate settings */
/* SPI_Baud_Rate = BUSCLK / ((SPPR[3:0] + 1) * 2^(SPR[3:0] + 1)) */
#ifndef gSystemClock_d 
  #define gSystemClock_d                      16    /* 16 MHz */
#endif

/* List of possible SPI baud rates */
#if(gSystemClock_d == 8)
  #define gSPI_BaudRate_100000_c                (uint8_t)((4<<4) | 0x03)      /* SPPR[3:0] = 4; SPPR[3:0] = 3 */
  #define gSPI_BaudRate_200000_c                (uint8_t)((4<<4) | 0x02)      /* SPPR[3:0] = 4; SPPR[3:0] = 2 */
  #define gSPI_BaudRate_400000_c                (uint8_t)((4<<4) | 0x01)      /* SPPR[3:0] = 4; SPPR[3:0] = 1 */
  #define gSPI_BaudRate_800000_c                (uint8_t)((4<<4) | 0x00)      /* SPPR[3:0] = 4; SPPR[3:0] = 0 */
  #define gSPI_BaudRate_1000000_c               (uint8_t)((1<<4) | 0x01)      /* SPPR[3:0] = 1; SPPR[3:0] = 1 */
  #define gSPI_BaudRate_2000000_c               (uint8_t)((1<<4) | 0x00)      /* SPPR[3:0] = 1; SPPR[3:0] = 0 */
  #define gSPI_BaudRate_4000000_c               (uint8_t)(0x00)               /* SPPR[3:0] = 0; SPPR[3:0] = 0 */
#endif

#if(gSystemClock_d == 12)
  #define gSPI_BaudRate_100000_c                (uint8_t)((3<<4) | 0x04)      /* SPPR[3:0] = 3; SPPR[3:0] = 4 */
  #define gSPI_BaudRate_200000_c                (uint8_t)((3<<4) | 0x03)      /* SPPR[3:0] = 3; SPPR[3:0] = 3 */
  #define gSPI_BaudRate_400000_c                (uint8_t)((3<<4) | 0x02)      /* SPPR[3:0] = 3; SPPR[3:0] = 2 */
  #define gSPI_BaudRate_800000_c                (uint8_t)((3<<4) | 0x01)      /* SPPR[3:0] = 3; SPPR[3:0] = 1 */
  #define gSPI_BaudRate_1000000_c               (uint8_t)((2<<4) | 0x01)      /* SPPR[3:0] = 2; SPPR[3:0] = 1 */
  #define gSPI_BaudRate_2000000_c               (uint8_t)((2<<4) | 0x00)      /* SPPR[3:0] = 2; SPPR[3:0] = 0 */
#endif

#if(gSystemClock_d == 16)
  #define gSPI_BaudRate_100000_c                (uint8_t)((4<<4) | 0x04)      /* SPPR[3:0] = 4; SPPR[3:0] = 4 */
  #define gSPI_BaudRate_200000_c                (uint8_t)((4<<4) | 0x03)      /* SPPR[3:0] = 4; SPPR[3:0] = 3 */
  #define gSPI_BaudRate_400000_c                (uint8_t)((4<<4) | 0x02)      /* SPPR[3:0] = 4; SPPR[3:0] = 2 */
  #define gSPI_BaudRate_800000_c                (uint8_t)((4<<4) | 0x01)      /* SPPR[3:0] = 4; SPPR[3:0] = 1 */
  #define gSPI_BaudRate_1000000_c               (uint8_t)((1<<4) | 0x02)      /* SPPR[3:0] = 1; SPPR[3:0] = 2 */
  #define gSPI_BaudRate_2000000_c               (uint8_t)((1<<4) | 0x01)      /* SPPR[3:0] = 1; SPPR[3:0] = 1 */
  #define gSPI_BaudRate_4000000_c               (uint8_t)((1<<4) | 0x00)      /* SPPR[3:0] = 1; SPPR[3:0] = 0 */
  #define gSPI_BaudRate_8000000_c               (uint8_t)(0x00)               /* SPPR[3:0] = 0; SPPR[3:0] = 0 */
#endif

/* Default baud rate for SPI in master mode */
#define gSPI_DefaultBaudRate_c                  gSPI_BaudRate_100000_c

/* SPI Mode of operation */
#define gSPI_MasterMode_c                       0x01
#define gSPI_SlaveMode_c                        0x00

/* Default mode of operation: slave */
#define gSPI_DefaultMode_c                      gSPI_SlaveMode_c

/* Bitswise shifting mode */
#define gSPI_LsbFirst_c                         0x01
#define gSPI_MsbFirst_c                         0x00

/* Default bitwise shifting: LSB first */
#define gSPI_DefaultBitwiseShfting_c            gSPI_MsbFirst_c

/* SPI bus directional option */
#define gSPI_FullDuplex_c                       0x01
#define gSPI_SingleWire_c                       0x00

/* Default SPI bus: full duplex */
#define gSPI_DefaultOperMode_c                  gSPI_FullDuplex_c

/* SPI clock polarity */
#define gSPI_ActiveLowPolarity_c                0x01
#define gSPI_ActiveHighPolarity_c               0x00

#define gSPI_DefaultClockPol_c                 gSPI_ActiveHighPolarity_c
                                                    
/* SPI clock phase */
#define gSPI_OddEdgeShifting_c                  0x01
#define gSPI_EvenEdgeShifting_c                 0x00

#define gSPI_DefaultClockPhase_c                gSPI_EvenEdgeShifting_c

/* Define the automatic SS output assertion */
#define gSPI_AutomaticSsPinAssertion_c          TRUE

/* Slave transmitter can signal to the master if there's data available */
#ifndef gSPI_Slave_TxDataAvailableSignal_Enable_c
  #define gSPI_Slave_TxDataAvailableSignal_Enable_c   FALSE
#endif

#if gSPI_Slave_TxDataAvailableSignal_Enable_c
  
  #ifndef gSPI_DataAvailablePortDataReg_c
    #define gSPI_DataAvailablePortDataReg_c     PTDD  
  #endif
  #ifndef gSPI_DataAvailablePortDDirReg_c
    #define gSPI_DataAvailablePortDDirReg_c     PTDDD
  #endif
  #ifndef gSPI_DataAvailablePinMask_c
    #define gSPI_DataAvailablePinMask_c         0x10
  #endif
 
  #define SPI_SignalTxDataAvailable(bTrue) (bTrue) ? (gSPI_DataAvailablePortDataReg_c |= gSPI_DataAvailablePinMask_c)\
                                                   : (gSPI_DataAvailablePortDataReg_c &= ~gSPI_DataAvailablePinMask_c)
  /* For slave on slave SPI transmit */  
  #define SPI_IsTxDataAvailable()           ((gSPI_DataAvailablePortDataReg_c & gSPI_DataAvailablePinMask_c))
  
  /* For master on slave SPI transmit */
  #define SPI_IsRxDataAvailable()           ((gSPI_DataAvailablePortDataReg_c & gSPI_DataAvailablePinMask_c))
  
  /* For slave on slave SPI transmit */ 
  /* Set GPIO as output */
  #define SPI_ConfigTxDataAvailablePin() gSPI_DataAvailablePortDDirReg_c |= gSPI_DataAvailablePinMask_c; \
                                         gSPI_DataAvailablePortDataReg_c &= ~gSPI_DataAvailablePinMask_c; 
                                          
  
  /* For master on slave SPI transmit */
  /* Set GPIO as input */
  #define SPI_ConfigRxDataAvailablePin() gSPI_DataAvailablePortDDirReg_c &= ~gSPI_DataAvailablePinMask_c;
                                               
#endif
/******************************************************************************
*******************************************************************************
* Public type definitions
*******************************************************************************
******************************************************************************/
/* Structure keeping the current module configuration */

typedef uint8_t SPIBaudRate_t;

typedef struct spiConfig_tag
{
  uint8_t devMode;
  uint8_t baudRate;
  uint8_t clockPol;
  uint8_t clockPhase;
  uint8_t bitwiseShift;
  bool_t  s3Wire;
} spiConfig_t;

/******************************************************************************
*******************************************************************************
* Public memory declarations
*******************************************************************************
******************************************************************************/

extern volatile index_t   mSpiRxBufferByteCount;

/******************************************************************************
*******************************************************************************
* Public function prototypes
*******************************************************************************
******************************************************************************/
/* Initialize the SPI module */
extern bool_t SPI1_Init(void);
extern bool_t SPI2_Init(void);

/* Uninitialize the SPI module */
extern void SPI1_Uninit(void);
extern void SPI2_Uninit(void);

/* This function set a specific configuration for the SPI module */
extern void SPI1_SetConfig(spiConfig_t mSpiConfig);  
extern void SPI2_SetConfig(spiConfig_t mSpiConfig);  

/* This function gets the SPI module configuration */
extern bool_t SPI_GetConfig(spiConfig_t* pSpiConfig); 

/* Set the callback function for the SPI slave mode receive operation */
extern void SPI_SetSlaveRxCallBack(void (*pfCallBack)(void));
 
/* Function performs a data transmission in master mode */
extern bool_t SPI1_MasterTransmit(uint8_t *pBuf, index_t bufLen, void (*pfCallBack)(bool_t status));
extern bool_t SPI2_MasterTransmit(uint8_t *pBuf, index_t bufLen, void (*pfCallBack)(bool_t status));

/* This function will perform a data receive operation in master mode */
extern bool_t SPI1_MasterReceive(uint8_t *pBuf, index_t bufLen, void (*pfCallBack)(bool_t status)); 
extern bool_t SPI2_MasterReceive(uint8_t *pBuf, index_t bufLen, void (*pfCallBack)(bool_t status));

/* Function to schedule a transmission over the SPI bus in slave mode */
extern bool_t SPI1_SlaveTransmit(uint8_t *pBuf, index_t bufLen, void (*pfCallBack)(uint8_t *pBuf)); 
extern bool_t SPI2_SlaveTransmit(uint8_t *pBuf, index_t bufLen, void (*pfCallBack)(uint8_t *pBuf)); 

/* This function is called to retrieve a character from the circular receive 
   buffer in slave mode */
extern bool_t SPI1_GetByteFromBuffer(uint8_t *pDst);
extern bool_t SPI2_GetByteFromBuffer(uint8_t *pDst);
      
/* Checks if Slave  Tx process is still running */
bool_t SPI1_IsSlaveTxActive(void);
bool_t SPI2_IsSlaveTxActive(void);

#if defined(PROCESSOR_MC1323X)
  #define SPI_Init()                (void)SPI1_Init()
  #define SPI_Uninit                SPI1_Uninit
  #define SPI_SetConfig             SPI1_SetConfig
  #define SPI_MasterTransmit        SPI1_MasterTransmit 
  #define SPI_MasterReceive         SPI1_MasterReceive
  #define SPI_SlaveTransmit         SPI1_SlaveTransmit
  #define SPI_GetByteFromBuffer     SPI1_GetByteFromBuffer
  #define SPI_IsSlaveTxActive       SPI1_IsSlaveTxActive   
#elif defined(PROCESSOR_QE128)
  #define SPI_Init()                (void)SPI2_Init()
  #define SPI_Uninit                SPI2_Uninit
  #define SPI_SetConfig             SPI2_SetConfig
  #define SPI_MasterTransmit        SPI2_MasterTransmit 
  #define SPI_MasterReceive         SPI2_MasterReceive
  #define SPI_SlaveTransmit         SPI2_SlaveTransmit
  #define SPI_GetByteFromBuffer     SPI2_GetByteFromBuffer
  #define SPI_IsSlaveTxActive       SPI2_IsSlaveTxActive     
#else
  #define SPI_Init()                  
  #define SPI_Uninit()                
  #define SPI_SetConfig(mSpiConfig)             
  #define SPI_MasterTransmit(pBuf, bufLen, pfCallBack)         
  #define SPI_MasterReceive(pBuf, bufLen, pfCallBack)         
  #define SPI_SlaveTransmit(pBuf, bufLen, pfCallBack)         
  #define SPI_GetByteFromBuffer(pDst)     
  #define SPI_IsSlaveTxActive()          
#endif

/* Place it in NON_BANKED memory */
#ifdef MEMORY_MODEL_BANKED
#pragma CODE_SEG __NEAR_SEG NON_BANKED
#else
#pragma CODE_SEG DEFAULT
#endif  /* MEMORY_MODEL_BANKED */
/* SPI Interrupt Service Routine */
#if defined(PROCESSOR_MC1323X)
  extern INTERRUPT_KEYWORD void SPI_Isr(void);
#elif defined(PROCESSOR_QE128)
  extern INTERRUPT_KEYWORD void SPI2_Isr(void);  
#endif
#pragma CODE_SEG DEFAULT


#endif  /* _SPI_Interface_h_ */