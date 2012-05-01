/******************************************************************************
* SPI Serial Port driver implementation. 
*
* Copyright (c) 2010, Freescale, Inc. All right reserved
*
*
* No part of this document must be reproduced in any form - including copied, 
* transcribed, printed or by any electronic means - vithout specific written
* permission from Freescale Semicondictor.
*  
******************************************************************************/

#include "EmbeddedTypes.h"
#include "IrqControlLib.h"
#include "IoConfig.h"
#include "TS_Interface.h"
#include "SPI.h"
#include "SPI_Interface.h"


/******************************************************************************
*******************************************************************************
* Private macros
*******************************************************************************
******************************************************************************/


/******************************************************************************
*******************************************************************************
* Private type definitions
*******************************************************************************
******************************************************************************/
/* The transmit code keeps a circular list of buffers to be sent */
typedef struct SpiTxBufRef_tag
{
  uint8_t       *pTxBuf;
  uint8_t       mTxBufLen;
  void          (*pfTxBufCallBack)(uint8_t *pTxBuf);
} SpiTxBufRef_t;

typedef enum 
{
  mSpi_OpType_Tx_c,
  mSpi_OpType_Rx_c
} spiOpType_t;

typedef struct spiMasterOp_tag
{
  spiOpType_t   spiOpType;
  uint8_t       *pBuf;
  uint8_t       bufLen;
} spiMasterOp_t;

typedef struct SpiTxCallbackTable_tag
{
  void          (*pfTxCallBack)(uint8_t *pTxBuf);
  uint8_t       *pTxBuf;
} SpiTxCallbackTable_t;

/******************************************************************************
*******************************************************************************
* Public memory definitions
*******************************************************************************
******************************************************************************/
#if (gSPI_Enabled_d == TRUE)
  /* Global variable keeping the current SPI module configuration */
  spiConfig_t gSpiConfig;

  /* Id for SPI Task */
  tsTaskID_t  gSpiTaskId;
   
#endif


/******************************************************************************
*******************************************************************************
* Private memory definitions
*******************************************************************************
******************************************************************************/
#if (gSPI_Enabled_d == TRUE)
  /* Slave Tx circular buffers */
  static SpiTxBufRef_t          maSpiTxBufRefTable[gSPI_SlaveTransmitBuffersNo_c];
  /* Leading and trailing indexes for Tx circular buffers */
  /* The leading index is the next position to store a buffer reference.
     The trailing index is the buffer currently being transmitted. */
  static index_t                mSpiTxBufRefLeadingIndex;
  static index_t                mSpiTxBufRefTrailingIndex;
  /* Current index in the processed buffer specified by 'mSpiTxBufRefTrailingIndex' */
  static index_t                mSpiTxBufRefCurIndex;
  
  /* Callback table and index pointers */
  static SpiTxCallbackTable_t   maSpiTxCallbackTable[gSPI_SlaveTransmitBuffersNo_c];
  static index_t                maSpiTxCallbackLeadingIndex   = 0;
  static index_t                maSpiTxCallbackTrailingIndex  = 0;
  
  /* Slave Rx circular buffer */
  static uint8_t                maSpiRxBuf[gSPI_SlaveReceiveBufferSize_c];
  static index_t                mSpiRxBufLeadingIndex;
  static index_t                mSpiRxBufTrailingIndex;
  volatile index_t              mSpiRxBufferByteCount;

  /* Local variable to keep the SPI Master Tx and Rx callbacks provided by application */
  static void                   (*pfSpiMasterTxCallBack)(bool_t status);
  static void                   (*pfSpiMasterRxCallBack)(bool_t status); 
  static void                   (*pfSpiSlaveRxCallBack)(void);
  static spiMasterOp_t          mSpiMasterOp;
#endif


/******************************************************************************
*******************************************************************************
* Private function prototypes
*******************************************************************************
******************************************************************************/
#if(gSPI_Enabled_d == TRUE)
  static void SPI_Task(event_t events);
  
  #if defined(PROCESSOR_MC1323X)
    static void SPI1_SlaveSendNextByte(void);  
  #elif defined(PROCESSOR_QE128)
    static void SPI2_SlaveSendNextByte(void);  
  #endif 
#endif


/******************************************************************************
*******************************************************************************
* Public functions
*******************************************************************************
******************************************************************************/

/******************************************************************************
*  SPI1_Init
* 
*  Initialize the SPI module 
******************************************************************************/
#if defined(PROCESSOR_MC1323X)
  bool_t SPI1_Init(void)
  {           
    #if (gSPI_Enabled_d == TRUE)
      bool_t retStatus = FALSE;
      
      /* Create the SPI Task */
      gSpiTaskId = TS_CreateTask(gTsSpiTaskPriority_c, SPI_Task);
      
      if(gTsInvalidTaskID_c != gSpiTaskId)
      {
        /* Initialize the global variable structure */
        gSpiConfig.devMode      = gSPI_DefaultMode_c;
        gSpiConfig.clockPol     = gSPI_DefaultClockPol_c;
        gSpiConfig.clockPhase   = gSPI_DefaultClockPhase_c;
        gSpiConfig.baudRate     = gSPI_DefaultBaudRate_c;
        gSpiConfig.bitwiseShift = gSPI_DefaultBitwiseShfting_c;
        gSpiConfig.s3Wire       = gSPI_DefaultOperMode_c;
        
        /* Clear the SPI Rx software buffer */
        mSpiRxBufLeadingIndex = mSpiRxBufTrailingIndex = mSpiRxBufferByteCount = 0;
        /* Initialize all SPI callback pointers */
        pfSpiSlaveRxCallBack  = NULL;
        pfSpiMasterTxCallBack = pfSpiMasterRxCallBack = NULL;
        
        /* Initialize the SPI module registers */
        /* Note:  both Tx and Rx interrupts are disabled */
        SPIC1 = mSPIxC1_Init_c;
        SPIC2 = mSPIxC2_Init_c;
        SPIBR = gSPI_DefaultBaudRate_c;  
        
        /* Update the retStatus variable */
        retStatus = TRUE;     
      }

      return retStatus;        
    #else
      return FALSE;
    #endif      
  }
#endif

/******************************************************************************
*  SPI2_Init
* 
*  Initialize the SPI module 
******************************************************************************/
#if defined(PROCESSOR_QE128)  
  bool_t SPI2_Init(void)
  {    
    #if (gSPI_Enabled_d == TRUE) 
      bool_t retStatus = FALSE;
      
      /* Create the SPI Task */
      gSpiTaskId = TS_CreateTask(gTsSpiTaskPriority_c, SPI_Task);
      
      if(gTsInvalidTaskID_c != gSpiTaskId)
      {
        /* Initialize the global variable structure */
        gSpiConfig.devMode      = gSPI_DefaultMode_c;
        gSpiConfig.clockPol     = gSPI_DefaultClockPol_c;
        gSpiConfig.clockPhase   = gSPI_DefaultClockPhase_c;
        gSpiConfig.baudRate     = gSPI_DefaultBaudRate_c;
        gSpiConfig.bitwiseShift = gSPI_DefaultBitwiseShfting_c;
        gSpiConfig.s3Wire       = gSPI_DefaultOperMode_c;
        
        /* Clear the SPI Rx software buffer */
        mSpiRxBufLeadingIndex = mSpiRxBufTrailingIndex = mSpiRxBufferByteCount = 0;
        /* Initialize all SPI callback pointers */
        pfSpiSlaveRxCallBack  = NULL;
        pfSpiMasterTxCallBack = pfSpiMasterRxCallBack = NULL;
        
        /* Initialize the SPI module registers */
        /* Note:  both Tx and Rx interrupts are disabled */
        SPI2C1 = mSPIxC1_Init_c;
        SPI2C2 = mSPIxC2_Init_c;
        SPI2BR = gSPI_DefaultBaudRate_c;  
        
        /* Update the retStatus variable */
        retStatus = TRUE;     
      }

      return retStatus; 
    #else
      return FALSE;
    #endif    
  }
#endif

/******************************************************************************
*  SPI1_Uninit
* 
*  Uninitialize the SPI module 
******************************************************************************/
#if defined(PROCESSOR_MC1323X)
  void SPI1_Uninit(void)
  { 
    #if (gSPI_Enabled_d == TRUE)    
      /* Destroy the SPI task if there is a valid ID */
      if(gTsInvalidTaskID_c != gSpiTaskId)
      {
        TS_DestroyTask(gSpiTaskId); 
      }
      
      SPIC1 = mSPIxC1_Reset_c;
      SPIC2 = mSPIxC2_Reset_c;   
    #endif      
  }
#endif

/******************************************************************************
*  SPI2_Uninit
* 
*  Uninitialize the SPI module 
******************************************************************************/
#if defined(PROCESSOR_QE128) 
  void SPI2_Uninit(void)  
  {          
    #if (gSPI_Enabled_d == TRUE) 
      /* Destroy the SPI task if there is a valid ID */
      if(gTsInvalidTaskID_c != gSpiTaskId)
      {
        TS_DestroyTask(gSpiTaskId); 
      }
      
      SPI2C1 = mSPIxC1_Reset_c;
      SPI2C2 = mSPIxC2_Reset_c;          
    #endif       
  }
#endif

/******************************************************************************
*  SPI1_SetConfig
* 
*  Set the configuration of the SPI module 
******************************************************************************/
#if defined(PROCESSOR_MC1323X)
  void SPI1_SetConfig(spiConfig_t mSpiConfig)
  {      
    #if (gSPI_Enabled_d == TRUE)
   
      gSpiConfig = mSpiConfig;
      
      /* Disable the SPI module */
      SPIC1 &= ~mSPIxC1_SPE_c;
       
      /* SPI mode of operation */
      if(gSpiConfig.devMode == gSPI_MasterMode_c)
      {
        SPIC1 |= mSPIxC1_MSTR_c;  
        SPIBR  = gSpiConfig.baudRate;
        
        /* Disable all SPI interrupts */
        SPIC1 &= ~(mSPIxC1_SPIE_c | mSPIxC1_SPTIE_c); 
        
        #if (TRUE == gSPI_AutomaticSsPinAssertion_c)
          /* Assert the nSS output pin to select the slave */       
          SPIC1 |= mSPIxC1_SSOE_c;
          SPIC2 |= mSPIxC2_MODFEN_c;
        #endif
        
        #if gSPI_Slave_TxDataAvailableSignal_Enable_c 
          /* Configure as output the GPIO that will be used to signal 
  	            slave device has data to be transmitted */
          SPI_ConfigRxDataAvailablePin();
        #endif    
      }
      else if(gSpiConfig.devMode == gSPI_SlaveMode_c)
      {
        SPIC1 &= ~mSPIxC1_MSTR_c;
        
        /* Assure that the SPI receive flag is cleared before enabling the receive 
           interrput in order to avoid false alarms */
        if(SPIS & mSPIxS_SPRF_c)
        {
          SPID;   
        }
                   
        /* Enable the Receive Interrupt */
        SPIC1 |= mSPIxC1_SPIE_c;
        
        #if (TRUE == gSPI_AutomaticSsPinAssertion_c)
          /* Keep the nSS pin deasserted */
          SPIC2 &= ~mSPIxC2_MODFEN_c;
          SPIC1 &= ~mSPIxC1_SSOE_c;                
        #endif
        
        #if gSPI_Slave_TxDataAvailableSignal_Enable_c 
          /* Configure as output the GPIO that will be used to signal 
  	            slave device has data to be transmitted */
          SPI_ConfigTxDataAvailablePin();
        #endif           
      }
      
      /* Bitwise shifting mode */
      if(gSpiConfig.bitwiseShift == gSPI_LsbFirst_c)
      {
        SPIC1 |= mSPIxC1_LSBFE_c;  
      }
      else if(gSpiConfig.bitwiseShift == gSPI_MsbFirst_c)
      {
        SPIC1 &= ~mSPIxC1_LSBFE_c;      
      }
      
      /* Clock polarity */
      if(gSpiConfig.clockPol == gSPI_ActiveLowPolarity_c)
      {
        SPIC1 |= mSPIxC1_CPOL_c;  
      }
      else if(gSpiConfig.clockPol == gSPI_ActiveHighPolarity_c)
      {
        SPIC1 &= ~mSPIxC1_CPOL_c;
      }
      
      /* Clock phase */
      if(gSpiConfig.clockPhase == gSPI_OddEdgeShifting_c)
      {
        SPIC1 |= mSPIxC1_CPHA_c;  
      }
      else if(gSpiConfig.clockPhase == gSPI_EvenEdgeShifting_c)
      {
        SPIC1 &= ~mSPIxC1_CPHA_c; 
      }
      
      /* SPI bus wires number */
      if(gSpiConfig.s3Wire == gSPI_SingleWire_c)
      {
        SPIC2 |= mSPIxC2_SPC0_c;
      }
      else if(gSpiConfig.s3Wire == gSPI_FullDuplex_c)
      {
        SPIC2 &= ~mSPIxC2_SPC0_c; 
      }
      
      /* Enable the SPI module */
      SPIC1 |= mSPIxC1_SPE_c;    
    #else
      (void) mSpiConfig;
    #endif 
  }
#endif


/******************************************************************************
*  SPI2_SetConfig
* 
*  Set the configuration of the SPI module 
******************************************************************************/
#if defined(PROCESSOR_QE128) 
  void SPI2_SetConfig(spiConfig_t mSpiConfig)
  {
    #if (gSPI_Enabled_d == TRUE)
      gSpiConfig = mSpiConfig;
      
      /* Disable the SPI module */
      SPI2C1 &= ~mSPIxC1_SPE_c;
       
      /* SPI mode of operation */
      if(gSpiConfig.devMode == gSPI_MasterMode_c)
      {
        SPI2C1 |= mSPIxC1_MSTR_c;  
        SPI2BR  = gSpiConfig.baudRate;
        
        /* Disable all SPI interrupts */
        SPI2C1 &= ~(mSPIxC1_SPIE_c | mSPIxC1_SPTIE_c); 
        
        #if (TRUE == gSPI_AutomaticSsPinAssertion_c)
          /* Assert the nSS output pin to select the slave */       
          SPI2C1 |= mSPIxC1_SSOE_c;
          SPI2C2 |= mSPIxC2_MODFEN_c;
        #endif
        
        #if gSPI_Slave_TxDataAvailableSignal_Enable_c 
          /* Configure as output the GPIO that will be used to signal 
  	            slave device has data to be transmitted */
          SPI_ConfigRxDataAvailablePin();
        #endif    
      }
      else if(gSpiConfig.devMode == gSPI_SlaveMode_c)
      {
        SPI2C1 &= ~mSPIxC1_MSTR_c;
        
        /* Assure that the SPI receive flag is cleared before enabling the receive 
           interrput in order to avoid false alarms */
        if(SPI2S & mSPIxS_SPRF_c)
        {
          SPI2D;   
        }
                   
        /* Enable the Receive Interrupt */
        SPI2C1 |= mSPIxC1_SPIE_c;
        
        #if (TRUE == gSPI_AutomaticSsPinAssertion_c)
          /* Keep the nSS pin deasserted */
          SPI2C2 &= ~mSPIxC2_MODFEN_c;
          SPI2C1 &= ~mSPIxC1_SSOE_c;                
        #endif
        
        #if gSPI_Slave_TxDataAvailableSignal_Enable_c 
          /* Configure as output the GPIO that will be used to signal 
  	            slave device has data to be transmitted */
          SPI_ConfigTxDataAvailablePin();
        #endif           
      }
      
      /* Bitwise shifting mode */
      if(gSpiConfig.bitwiseShift == gSPI_LsbFirst_c)
      {
        SPI2C1 |= mSPIxC1_LSBFE_c;  
      }
      else if(gSpiConfig.bitwiseShift == gSPI_MsbFirst_c)
      {
        SPI2C1 &= ~mSPIxC1_LSBFE_c;      
      }
      
      /* Clock polarity */
      if(gSpiConfig.clockPol == gSPI_ActiveLowPolarity_c)
      {
        SPI2C1 |= mSPIxC1_CPOL_c;  
      }
      else if(gSpiConfig.clockPol == gSPI_ActiveHighPolarity_c)
      {
        SPI2C1 &= ~mSPIxC1_CPOL_c;
      }
      
      /* Clock phase */
      if(gSpiConfig.clockPhase == gSPI_OddEdgeShifting_c)
      {
        SPI2C1 |= mSPIxC1_CPHA_c;  
      }
      else if(gSpiConfig.clockPhase == gSPI_EvenEdgeShifting_c)
      {
        SPI2C1 &= ~mSPIxC1_CPHA_c; 
      }
      
      /* SPI bus wires number */
      if(gSpiConfig.s3Wire == gSPI_SingleWire_c)
      {
        SPI2C2 |= mSPIxC2_SPC0_c;
      }
      else if(gSpiConfig.s3Wire == gSPI_FullDuplex_c)
      {
        SPI2C2 &= ~mSPIxC2_SPC0_c; 
      }
      
      /* Enable the SPI module */
      SPI2C1 |= mSPIxC1_SPE_c;
    #else
      (void) mSpiConfig;
    #endif   
  }
#endif


/******************************************************************************
*  SPI_GetConfig
* 
*  Get the configuration of the SPI module 
******************************************************************************/
bool_t SPI_GetConfig(spiConfig_t* pSpiConfig)
{
  #if (gSPI_Enabled_d == TRUE)
    bool_t retStatus = TRUE;
    
    if(NULL == pSpiConfig)
    {
      retStatus = FALSE;
    }
    else
    {
      /* Save all memebers of the spiConfig_t struct. */
      pSpiConfig->devMode     = gSpiConfig.devMode;
      pSpiConfig->baudRate    = gSpiConfig.baudRate;
      pSpiConfig->clockPol    = gSpiConfig.clockPol;
      pSpiConfig->clockPhase  = gSpiConfig.clockPhase;
      pSpiConfig->bitwiseShift= gSpiConfig.bitwiseShift;
      pSpiConfig->s3Wire      = gSpiConfig.s3Wire;
    }
    
    return retStatus;    
  #else
    (void) pSpiConfig;
    return TRUE;
  #endif   
}


/******************************************************************************
*  SPI_SetSlaveRxCallBack
*
*  Set the callback function for SPI slave mode receive operation   
******************************************************************************/
void SPI_SetSlaveRxCallBack(void (*pfCallBack)(void))
{
  #if (gSPI_Enabled_d == TRUE)
    pfSpiSlaveRxCallBack = pfCallBack; 
  #else
    (void) pfCallBack;
  #endif
}

/*****************************************************************************
*   SPI1_IsSlaveTxActive 
*
*   Returns TRUE if there is still data to be transmitted to the master.
*   Returns FALSE if nothing left to transmit.
******************************************************************************/
#if defined(PROCESSOR_MC1323X)
  bool_t SPI1_IsSlaveTxActive(void) 
  {
    #if !gSPI_Enabled_d
      return FALSE;
    #else      
      return (SPIC1 & mSPIxC1_SPTIE_c);     
    #endif  
  }
#endif


/*****************************************************************************
*   SPI2_IsSlaveTxActive 
*
*   Returns TRUE if there is still data to be transmitted to the master.
*   Returns FALSE if nothing left to transmit.
******************************************************************************/
#if defined(PROCESSOR_QE128)
  bool_t SPI2_IsSlaveTxActive(void) 
  {
    #if !gSPI_Enabled_d
      return FALSE;
    #else
        return (SPI2C1 & mSPIxC1_SPTIE_c);
    #endif  
  }
#endif


/******************************************************************************
*  SPI1_MasterTransmit
*
*  Begin transmission of a 'bufLen' size bytes starting from the *pBuf   
******************************************************************************/
#if defined(PROCESSOR_MC1323X)
  bool_t SPI1_MasterTransmit(uint8_t *pBuf, index_t bufLen, void (*pfCallBack)(bool_t status))
  {
    #if (gSPI_Enabled_d == TRUE)   
      bool_t retStatus = TRUE;
      
      if(gSpiConfig.devMode == gSPI_MasterMode_c)
      {
        /* Handle empty buffers. */
        if(!bufLen)
        {
          /* Call the callback function (with the status FALSE), if the pointer to it is valid */
          if(pfCallBack)
          {
            (*pfCallBack)(FALSE);
          }
        }
        else
        {
          /* 'pfSpiMasterTxCallBack' is reset by Spi Task after the callback is called */  
          /* If 'pfSpiMasterTxCallBack' is != 0 it means that the previous callback 
             didn't run yet */
          if(pfSpiMasterTxCallBack)
          {
            retStatus = FALSE;
          }
          else
          {
            /* Update the variable storing the master operation and also the callback */
            mSpiMasterOp.spiOpType  = mSpi_OpType_Tx_c;     
            mSpiMasterOp.pBuf       = (uint8_t*)pBuf;
            mSpiMasterOp.bufLen     = bufLen;
            pfSpiMasterTxCallBack   = pfCallBack;
            
            /* If the 3 wire operation is selected, then select the pin as output */
            if(gSpiConfig.s3Wire == gSPI_SingleWire_c)
            {
              SPIC2 |= mSPIxC2_BIDIROE_c;
            }                                                      
            /* Enable the Transmit Interrupt */
            /* Note: Because the Transmit Buffer is empty it will generate immediatelly an interrupt */
            SPIC1 |= mSPIxC1_SPTIE_c;
          }
        }      
      }
      else
      {
        retStatus = FALSE;
      }
        
      return retStatus;
    #else
      (void) pBuf;
      (void) bufLen;
      (void) pfCallBack;
      
      return TRUE;
    #endif
  }
#endif


/******************************************************************************
*  SPI1_MasterTransmit
*
*  Begin transmission of a 'bufLen' size bytes starting from the *pBuf   
******************************************************************************/
#if defined(PROCESSOR_QE128)
  bool_t SPI2_MasterTransmit(uint8_t *pBuf, index_t bufLen, void (*pfCallBack)(bool_t status))
  {  
    #if (gSPI_Enabled_d == TRUE)    
      bool_t retStatus = TRUE;
      
      if(gSpiConfig.devMode == gSPI_MasterMode_c)
      {
        /* Handle empty buffers. */
        if(!bufLen)
        {
          /* Call the callback function (with the status FALSE), if the pointer to it is valid */
          if(pfCallBack)
          {
            (*pfCallBack)(FALSE);
          }
        }
        else
        {
          /* 'pfSpiMasterTxCallBack' is reset by Spi Task after the callback is called */  
          /* If 'pfSpiMasterTxCallBack' is != 0 it means that the previous callback 
             didn't run yet */
          if(pfSpiMasterTxCallBack)
          {
            retStatus = FALSE;
          }
          else
          {
            /* Update the variable storing the master operation and also the callback */
            mSpiMasterOp.spiOpType  = mSpi_OpType_Tx_c;     
            mSpiMasterOp.pBuf       = (uint8_t*)pBuf;
            mSpiMasterOp.bufLen     = bufLen;
            pfSpiMasterTxCallBack   = pfCallBack;
            
            /* If the 3 wire operation is selected, then select the pin as output */
            if(gSpiConfig.s3Wire == gSPI_SingleWire_c)
            {
              SPI2C2 |= mSPIxC2_BIDIROE_c;
            }                                                      
            /* Enable the Transmit Interrupt */
            /* Note: Because the Transmit Buffer is empty it will generate immediatelly an interrupt */
            SPI2C1 |= mSPIxC1_SPTIE_c;
          }
        }      
      }
      else
      {
        retStatus = FALSE;
      }
        
      return retStatus;
    #else
      (void) pBuf;
      (void) bufLen;
      (void) pfCallBack;
      
      return TRUE;    
    #endif    
  }
#endif


/******************************************************************************
*  SPI1_MasterReceive
*
*  Begin reception of a 'bufLen' size bytes starting from the *pBuf   
******************************************************************************/
#if defined(PROCESSOR_MC1323X)
  bool_t SPI1_MasterReceive(uint8_t *pBuf, index_t bufLen, void (*pfCallBack)(bool_t status))
  {
    #if (gSPI_Enabled_d == TRUE)       
      bool_t retStatus = TRUE;
      
      if(gSpiConfig.devMode == gSPI_MasterMode_c)
      {
        /* Handle empty buffers. */
        if((!bufLen) || (pBuf == NULL)) 
        {
          /* Call the callback function (with the status FALSE), if it is a valid pointer to it */
          if(pfCallBack)
          {
            (*pfCallBack)(FALSE);
          }      
        }
        else
        {
          /* 'pfSpiMasterRxCallBack' is reset by Spi Task after the callback is called */
          /* If 'pfSpiMasterRxCallBack' is != 0 it means that the previous callback didn't run yet */
          if(pfSpiMasterRxCallBack)
          {
            retStatus = FALSE;
          }
          else
          {
            /* Update the variable storing the master operation and also the callback */
            mSpiMasterOp.spiOpType  = mSpi_OpType_Rx_c;
            mSpiMasterOp.pBuf       = (uint8_t*)pBuf;  
            mSpiMasterOp.bufLen     = bufLen;
            pfSpiMasterRxCallBack   = pfCallBack;   
              
            /* If the 3 wire operation is selected, then select the pin as input */
            if(gSpiConfig.s3Wire == gSPI_SingleWire_c)
            {
              SPIC2 &= ~mSPIxC2_BIDIROE_c;
            } 
                     
            /* Assure that the SPI receive flag is cleared before enabling the receive 
               interrupt in order to avoid false alarms */
            if(SPIS & mSPIxS_SPRF_c)
            {
              SPID;
            }          
            
            /* Enable the Receive Interrupt */
            SPIC1 |= mSPIxC1_SPIE_c;          
                      
            /* Initiate a dummy transmission; this will also initiate the reception over the SPI bus */
            SPID = 0x00;                         
          }
        }      
      }
      else
      {
        retStatus = FALSE;
      }
          
      return retStatus;     
    #else
      (void) pBuf;
      (void) bufLen;
      (void) pfCallBack;
      
      return TRUE;
    #endif
  }
#endif


/******************************************************************************
*  SPI2_MasterReceive
*
*  Begin reception of a 'bufLen' size bytes starting from the *pBuf   
******************************************************************************/
#if defined(PROCESSOR_QE128)
  bool_t SPI2_MasterReceive(uint8_t *pBuf, index_t bufLen, void (*pfCallBack)(bool_t status))
  {   
    #if (gSPI_Enabled_d == TRUE) 
      bool_t retStatus = TRUE;
      
      if(gSpiConfig.devMode == gSPI_MasterMode_c)
      {
        /* Handle empty buffers. */
        if((!bufLen) || (pBuf == NULL)) 
        {
          /* Call the callback function (with the status FALSE), if it is a valid pointer to it */
          if(pfCallBack)
          {
            (*pfCallBack)(FALSE);
          }      
        }
        else
        {
          /* 'pfSpiMasterRxCallBack' is reset by Spi Task after the callback is called */
          /* If 'pfSpiMasterRxCallBack' is != 0 it means that the previous callback didn't run yet */
          if(pfSpiMasterRxCallBack)
          {
            retStatus = FALSE;
          }
          else
          {
            /* Update the variable storing the master operation and also the callback */
            mSpiMasterOp.spiOpType  = mSpi_OpType_Rx_c;
            mSpiMasterOp.pBuf       = (uint8_t*)pBuf;  
            mSpiMasterOp.bufLen     = bufLen;
            pfSpiMasterRxCallBack   = pfCallBack;   
            
            /* If the 3 wire operation is selected, then select the pin as input */
            if(gSpiConfig.s3Wire == gSPI_SingleWire_c)
            {
              SPI2C2 &= ~mSPIxC2_BIDIROE_c;
            } 
                     
            /* Assure that the SPI receive flag is cleared before enabling the receive 
               interrupt in order to avoid false alarms */
            if(SPI2S & mSPIxS_SPRF_c)
            {
              SPI2D;
            }          
            
            /* Enable the Receive Interrupt */
            SPI2C1 |= mSPIxC1_SPIE_c;          
                      
            /* Initiate a dummy transmission; this will also initiate the reception over the SPI bus */
            SPI2D = 0x00;                         
          }
        }      
      }
      else
      {
        retStatus = FALSE;
      }
          
      return retStatus;  
    #else
      (void) pBuf;
      (void) bufLen;
      (void) pfCallBack;
      
      return TRUE;      
    #endif
  }
#endif


/******************************************************************************
*  SPI1_SlaveTransmit
*
*  Begin transmission of a 'bufLen' size bytes starting from the *pBuf   
******************************************************************************/
#if defined(PROCESSOR_MC1323X)
  bool_t SPI1_SlaveTransmit(uint8_t *pBuf, index_t bufLen, void (*pfCallBack)(uint8_t *pBuf))
  {
    #if (gSPI_Enabled_d == TRUE)
      bool_t retStatus = TRUE;
      
      if(gSpiConfig.devMode == gSPI_SlaveMode_c)
      {
        /* Handle empty buffers. */
        if(!bufLen)
        {
          /* Call the callback if the pointer to it, is valid */
          if(pfCallBack)
          {
            (*pfCallBack)(pBuf);               
          }
        }
        else
        {
          /* Room for one more? */
          if(maSpiTxBufRefTable[mSpiTxBufRefLeadingIndex].mTxBufLen)  
          {
            retStatus = FALSE;
          }
          else
          {
            /* Update the table ref. with pointer of the Tx buffer and the callback */
            maSpiTxBufRefTable[mSpiTxBufRefLeadingIndex].pTxBuf           = pBuf;
            maSpiTxBufRefTable[mSpiTxBufRefLeadingIndex].pfTxBufCallBack  = pfCallBack;
            /* Update the table with bufer lengths with the new element */
            maSpiTxBufRefTable[mSpiTxBufRefLeadingIndex].mTxBufLen        = bufLen;
            
            /* Increment the leading index and reset if exceed the number of Tx 
               buffers in slave mode */
            if(++mSpiTxBufRefLeadingIndex >= gSPI_SlaveTransmitBuffersNo_c)
            {
              mSpiTxBufRefLeadingIndex = 0; 
            }
            
            /* Enable the Transmit Interrupt */ 
            SPIC1 |= mSPIxC1_SPTIE_c;
          }
        }      
      }
      else
      {
        retStatus = FALSE;
      }
                
      return retStatus; 
    #else
      (void) pBuf;
      (void) bufLen;
      (void) pfCallBack;
      
      return TRUE;
    #endif
  }
#endif

 
/******************************************************************************
*  SPI2_SlaveTransmit
*
*  Begin transmission of a 'bufLen' size bytes starting from the *pBuf   
******************************************************************************/
#if defined(PROCESSOR_QE128) 
  bool_t SPI2_SlaveTransmit(uint8_t *pBuf, index_t bufLen, void (*pfCallBack)(uint8_t *pBuf))
  {    
    #if (gSPI_Enabled_d == TRUE) 
      bool_t retStatus = TRUE;
        
      if(gSpiConfig.devMode == gSPI_SlaveMode_c)
      {
        /* Handle empty buffers. */
        if(!bufLen)
        {
          /* Call the callback if the pointer to it, is valid */
          if(pfCallBack)
          {
            (*pfCallBack)(pBuf);               
          }
        }
        else
        {
          /* Room for one more? */
          if(maSpiTxBufRefTable[mSpiTxBufRefLeadingIndex].mTxBufLen)  
          {
            retStatus = FALSE;
          }
          else
          {
            /* Update the table ref. with pointer of the Tx buffer and the callback */
            maSpiTxBufRefTable[mSpiTxBufRefLeadingIndex].pTxBuf           = pBuf;
            maSpiTxBufRefTable[mSpiTxBufRefLeadingIndex].pfTxBufCallBack  = pfCallBack;
            /* Update the table with bufer lengths with the new element */
            maSpiTxBufRefTable[mSpiTxBufRefLeadingIndex].mTxBufLen        = bufLen;
            
            /* Increment the leading index and reset if exceed the number of Tx 
               buffers in slave mode */
            if(++mSpiTxBufRefLeadingIndex >= gSPI_SlaveTransmitBuffersNo_c)
            {
              mSpiTxBufRefLeadingIndex = 0; 
            }
            
            /* Enable the Transmit Interrupt */ 
            SPI2C1 |= mSPIxC1_SPTIE_c;
          }
        }      
      }
      else
      {
        retStatus = FALSE;
      }
                
      return retStatus;  
    #else
      (void) pBuf;
      (void) bufLen;
      (void) pfCallBack;
      
      return TRUE;    
    #endif
  }
#endif

 
/******************************************************************************
*  SPI1_GetByteFromBuffer
*
*  Retrieve a byte from the driver's Rx circular buffer and store it at *pDest.
*  Return TRUE if a byte was retrieved; FALSE if the Rx buffer is empty.   
******************************************************************************/
#if defined(PROCESSOR_MC1323X)
  bool_t SPI1_GetByteFromBuffer(uint8_t *pDst)
  {
    #if (gSPI_Enabled_d == TRUE)
      bool_t  retStatus = TRUE;
      uint8_t spiControlReg, var;
      
      if(!mSpiRxBufferByteCount)
      {
        retStatus = FALSE;
      }
      else
      {
        IrqControlLib_ProtectFromMC1323xIrq(var);      
        IrqControlLib_PushIrqStatus();
        /* Disable all interrupts */
        IrqControlLib_DisableAllIrqs();
        
        /* Store the Control Register */
        spiControlReg = SPIC1;
        /* Disable the SPI Rx interrupt while the counter is updated */
        SPIC1 &= ~mSPIxC1_SPIE_c;
        
        IrqControlLib_PullIrqStatus();
        
        /* Pick-up the byte from the Rx buffer and store it in the location pointed by 'pDst' */
        *pDst = maSpiRxBuf[mSpiRxBufTrailingIndex];
        /* Update the Rx buffer trailing index */
        if(++mSpiRxBufTrailingIndex >= gSPI_SlaveReceiveBufferSize_c)
        {
          mSpiRxBufTrailingIndex = 0; 
        }
        
        /* Decrement the number of bytes left */
        --mSpiRxBufferByteCount;
        
        /* Restore the SPI Control Register */
        SPIC1 = spiControlReg;
        IrqControlLib_UnprotectFromMC1323xIrq(var);
      }
      
      return retStatus; 
    #else
      (void) pDst;
      
      return TRUE;
    #endif
  }
#endif


/******************************************************************************
*  SPI2_GetByteFromBuffer
*
*  Retrieve a byte from the driver's Rx circular buffer and store it at *pDest.
*  Return TRUE if a byte was retrieved; FALSE if the Rx buffer is empty.   
******************************************************************************/
#if defined(PROCESSOR_QE128)
  bool_t SPI2_GetByteFromBuffer(uint8_t *pDst)
  {          
    #if (gSPI_Enabled_d == TRUE)
      bool_t  retStatus = TRUE;
      uint8_t spiControlReg;
      
      if(!mSpiRxBufferByteCount)
      {
        retStatus = FALSE;
      }
      else
      {      
        IrqControlLib_PushIrqStatus();
        /* Disable all interrupts */
        IrqControlLib_DisableAllIrqs();
        
        /* Store the Control Register */
        spiControlReg = SPI2C1;
        /* Disable the SPI Rx interrupt while the counter is updated */
        SPI2C1 &= ~mSPIxC1_SPIE_c;
        
        IrqControlLib_PullIrqStatus();
        
        /* Pick-up the byte from the Rx buffer and store it in the location pointed by 'pDst' */
        *pDst = maSpiRxBuf[mSpiRxBufTrailingIndex];
        /* Update the Rx buffer trailing index */
        if(++mSpiRxBufTrailingIndex >= gSPI_SlaveReceiveBufferSize_c)
        {
          mSpiRxBufTrailingIndex = 0; 
        }
        
        /* Decrement the number of bytes left */
        --mSpiRxBufferByteCount;
        
        /* Restore the SPI Control Register */
        SPI2C1 = spiControlReg;
      }
      
      return retStatus;  
    #else
      (void) pDst;
      
      return TRUE;    
    #endif
  }
#endif

#if(gSPI_Enabled_d == TRUE)
  /******************************************************************************
  *  SPI_Task
  *
  *  Main task of the SPI module 
  ******************************************************************************/
  static void SPI_Task(event_t events)
  { 
    void (*pfTxCallBack)(uint8_t *pTxBuf);
    
    /* Master Tx success event */
    if(events & gSPI_Event_MasterTxSuccess_c)
    {
      /* Run the master Tx callback with TRUE as status argument then invalidate it */
      (*pfSpiMasterTxCallBack)(TRUE);
      pfSpiMasterTxCallBack = NULL;
    }
    
    /* Master Tx fail event */
    if(events & gSPI_Event_MasterTxFail_c)
    {
      /* Run the master Tx callback with FALSE as status argument then invalidate it */
      (*pfSpiMasterTxCallBack)(FALSE);
      pfSpiMasterTxCallBack = NULL;      
    }
    
    /* Master Rx success event */
    if(events & gSPI_Event_MasterRxSuccess_c)
    {
      /* Run the master Rx callback with TRUE as status argument then invalidate it */
      (*pfSpiMasterRxCallBack)(TRUE);
      pfSpiMasterRxCallBack = NULL;      
    }
    
    /* Master Rx fail event */
    if(events & gSPI_Event_MasterRxFail_c)
    {
      /* Run the master Rx callback with FALSE as status argument then invalidate it */
      (*pfSpiMasterRxCallBack)(FALSE);
      pfSpiMasterRxCallBack = NULL;      
    }    
    
    /* Slave Tx event */
    if(events & gSPI_Event_SlaveTx_c)
    {
      /* Run through the callback table and execute any pending callbacks */
      while(maSpiTxCallbackTrailingIndex != maSpiTxCallbackLeadingIndex)
      {
        pfTxCallBack = maSpiTxCallbackTable[maSpiTxCallbackTrailingIndex].pfTxCallBack;
        /* Call callback with buffer info as parameter */
        (*pfTxCallBack)(maSpiTxCallbackTable[maSpiTxCallbackTrailingIndex].pTxBuf);
        
        /* Increment and wrap around the callback trailing index */
        if(++maSpiTxCallbackTrailingIndex >= gSPI_SlaveTransmitBuffersNo_c)
        {
          maSpiTxCallbackTrailingIndex = 0;
        }
      }
    }
    
    /* Slave Rx event */
    if(events & gSPI_Event_SlaveRx_c)
    {
      pfSpiSlaveRxCallBack();  
    }        
  }
#endif

/******************************************************************************
*  SPI_Isr
*
*  SPI Interrupt service Routine
******************************************************************************/
/* Place it in NON_BANKED memory */
#ifdef MEMORY_MODEL_BANKED
#pragma CODE_SEG __NEAR_SEG NON_BANKED
#else
#pragma CODE_SEG DEFAULT
#endif  /* MEMORY_MODEL_BANKED */

#if defined(PROCESSOR_MC1323X)
  INTERRUPT_KEYWORD void SPI_Isr(void)
  {
    #if (gSPI_Enabled_d == TRUE)
      volatile uint8_t spiStatusReg;
      //volatile uint8_t iCnt;
    
      /* Read the SPI Status Register */
      spiStatusReg = SPIS; 
        
      /* Check if the interrupt is caused by a transmission buffer empty */
      if((spiStatusReg & mSPIxS_SPTEF_c) && (SPIC1 & mSPIxC1_SPTIE_c))
      {  /* Tx */
        /* Check the mode of operation: master or slave */
        if(SPIC1 & mSPIxC1_MSTR_c)
        { /* Master mode */
          if(mSpiMasterOp.spiOpType == mSpi_OpType_Tx_c)     
          {
            if(mSpiMasterOp.bufLen)
            {
              /* Initiate an SPI Transmission; Writting the SPI Data Register 
                 will also clear the SPTEF flag */
              SPID = *mSpiMasterOp.pBuf++;
              
              /* Decrement the buffer length */
              mSpiMasterOp.bufLen--;                          
            }
            else
            {
              /* Disable the Transmit Interrupt */
              SPIC1 &= ~mSPIxC1_SPTIE_c;
                                     
              if(pfSpiMasterTxCallBack)
              {
                /* Send event to the SPI Task with successfull Tx status (only if 
                   there is a valid master Tx callback) */
                TS_SendEvent(gSpiTaskId, gSPI_Event_MasterTxSuccess_c);              
              }
            }
          }
          else
          {
            if(pfSpiMasterTxCallBack)
            {
              /* Send an event to the SPI Task with fail Tx status (only if there 
                 is a valid master Tx callback) */
              TS_SendEvent(gSpiTaskId, gSPI_Event_MasterTxFail_c);              
            }          
          }
        }
        else
        {       
          /* Disable the Transmit Interrupt if and only if there is no data to be send */
          if((mSpiTxBufRefTrailingIndex == mSpiTxBufRefLeadingIndex) && !(maSpiTxBufRefTable[mSpiTxBufRefTrailingIndex].mTxBufLen))
          {

            SPIC1 &= ~mSPIxC1_SPTIE_c;
            #if gSPI_Slave_TxDataAvailableSignal_Enable_c
              SPI_SignalTxDataAvailable(FALSE);
            #endif 
          }
          else
          {        
            /* Slave mode */
            SPI1_SlaveSendNextByte();
          }
        }      
      }
      
      /* Check if the interrupt is caused by a reception buffer full */
      if((spiStatusReg & mSPIxS_SPRF_c) && (SPIC1 & mSPIxC1_SPIE_c))
      { /* Rx */
        /* Check the mode of operation: master or slave */
        if(SPIC1 & mSPIxC1_MSTR_c)
        { /* Master mode */
          if(mSpiMasterOp.spiOpType == mSpi_OpType_Rx_c)
          {
            /* Read the received data (from SPI Data Register); this will also 
               clear the SPRF flag */
            *mSpiMasterOp.pBuf++ = SPID;
              
            /* Decrement the buffer length */
            mSpiMasterOp.bufLen--;          
            
            if(mSpiMasterOp.bufLen)
            {                          
              /* Initiate the next reception */
              SPID = 0x00;                      
            }
            else
            {
              /* Disable the Receive Interrupt */
              SPIC1 &= ~mSPIxC1_SPIE_c;                              
              
              if(pfSpiMasterRxCallBack)
              {
                /* Send an event to the SPI Task with success Rx status (only if 
                   there is a valid master Rx callback) */
                TS_SendEvent(gSpiTaskId, gSPI_Event_MasterRxSuccess_c);
              }
            }
          }
          else
          {
            if(pfSpiMasterRxCallBack)
            {
              /* Send an event to the SPI Task with fail Rx status (only if there
                 is a valid master Rx callback) */
              TS_SendEvent(gSpiTaskId, gSPI_Event_MasterRxFail_c);            
            }            
          }        
        }
        else
        { /* Slave mode */
          /* Put the received byte in the Rx buffer */
          if(pfSpiSlaveRxCallBack)  
          {
            maSpiRxBuf[mSpiRxBufLeadingIndex] = SPID;
            
            /* Update the Rx buffer leading index */
            if(++mSpiRxBufLeadingIndex >= gSPI_SlaveReceiveBufferSize_c)
            {
              mSpiRxBufLeadingIndex = 0; 
            }
            
            /* Update the Rx buffer byte count */
            if(mSpiRxBufferByteCount < gSPI_SlaveReceiveBufferSize_c)
            {
              mSpiRxBufferByteCount++;
            }
            else
            {
              /* If the number of bytes to be read exceed the Rx buffer size then
                 update also the trailing index */
              if(++mSpiRxBufTrailingIndex >= gSPI_SlaveReceiveBufferSize_c)
              {
                mSpiRxBufTrailingIndex = 0;
              }
            }
            
            /* Let the application know a byte has been receive. */
            TS_SendEvent(gSpiTaskId, gSPI_Event_SlaveRx_c);  
          }
          else
          {
            /* Dummy read of the SPI Data Register in order to clear the SPRF flag */
            SPID;
          }
        }      
      }
      
      /* Check if the interrupt is caused by a mode fault event */
      if((spiStatusReg & mSPIxS_MODF_c) && (SPIC1 & mSPIxC1_SPIE_c))
      { /* MODF */
        /* Check the mode of operation: master or slave */
        if(SPIC1 & mSPIxC1_MSTR_c)
        { /* Master mode */
          /* Turn the SPI module in slave mode. Writting to the SPIxC1 will also 
             clear the MODF flag */
          SPIC1 &= ~mSPIxC1_MSTR_c;
        }    
      }
      
    #endif
  }
#endif  

#if defined(PROCESSOR_QE128)
  INTERRUPT_KEYWORD void SPI2_Isr(void)
  {        
    #if (gSPI_Enabled_d == TRUE)
      volatile uint8_t spiStatusReg;
      //volatile uint8_t iCnt;
    
      /* Read the SPI Status Register */
      spiStatusReg = SPI2S; 
        
      /* Check if the interrupt is caused by a transmission buffer empty */
      if((spiStatusReg & mSPIxS_SPTEF_c) && (SPI2C1 & mSPIxC1_SPTIE_c))
      {  /* Tx */
        /* Check the mode of operation: master or slave */
        if(SPI2C1 & mSPIxC1_MSTR_c)
        { /* Master mode */
          if(mSpiMasterOp.spiOpType == mSpi_OpType_Tx_c)     
          {
            if(mSpiMasterOp.bufLen)
            {
              /* Initiate an SPI Transmission; Writting the SPI Data Register 
                 will also clear the SPTEF flag */
              SPI2D = *mSpiMasterOp.pBuf++;
              
              /* Decrement the buffer length */
              mSpiMasterOp.bufLen--;                          
            }
            else
            {
              /* Disable the Transmit Interrupt */
              SPI2C1 &= ~mSPIxC1_SPTIE_c;
                                     
              if(pfSpiMasterTxCallBack)
              {
                /* Send event to the SPI Task with successfull Tx status (only if 
                   there is a valid master Tx callback) */
                TS_SendEvent(gSpiTaskId, gSPI_Event_MasterTxSuccess_c);              
              }
            }
          }
          else
          {
            if(pfSpiMasterTxCallBack)
            {
              /* Send an event to the SPI Task with fail Tx status (only if there 
                 is a valid master Tx callback) */
              TS_SendEvent(gSpiTaskId, gSPI_Event_MasterTxFail_c);              
            }          
          }
        }
        else
        {       
          /* Disable the Transmit Interrupt if and only if there is no data to be send */
          if((mSpiTxBufRefTrailingIndex == mSpiTxBufRefLeadingIndex) && !(maSpiTxBufRefTable[mSpiTxBufRefTrailingIndex].mTxBufLen))
          {

            SPI2C1 &= ~mSPIxC1_SPTIE_c;
            #if gSPI_Slave_TxDataAvailableSignal_Enable_c
              SPI_SignalTxDataAvailable(FALSE);
            #endif 
          }
          else
          {        
            /* Slave mode */
            SPI2_SlaveSendNextByte();
          }
        }      
      }
      
      /* Check if the interrupt is caused by a reception buffer full */
      if((spiStatusReg & mSPIxS_SPRF_c) && (SPI2C1 & mSPIxC1_SPIE_c))
      { /* Rx */
        /* Check the mode of operation: master or slave */
        if(SPI2C1 & mSPIxC1_MSTR_c)
        { /* Master mode */
          if(mSpiMasterOp.spiOpType == mSpi_OpType_Rx_c)
          {
            /* Read the received data (from SPI Data Register); this will also 
               clear the SPRF flag */
            *mSpiMasterOp.pBuf++ = SPI2D;
              
            /* Decrement the buffer length */
            mSpiMasterOp.bufLen--;          
            
            if(mSpiMasterOp.bufLen)
            {                          
              /* Initiate the next reception */
              SPI2D = 0x00;                      
            }
            else
            {
              /* Disable the Receive Interrupt */
              SPI2C1 &= ~mSPIxC1_SPIE_c;                              
              
              if(pfSpiMasterRxCallBack)
              {
                /* Send an event to the SPI Task with success Rx status (only if 
                   there is a valid master Rx callback) */
                TS_SendEvent(gSpiTaskId, gSPI_Event_MasterRxSuccess_c);
              }
            }
          }
          else
          {
            if(pfSpiMasterRxCallBack)
            {
              /* Send an event to the SPI Task with fail Rx status (only if there
                 is a valid master Rx callback) */
              TS_SendEvent(gSpiTaskId, gSPI_Event_MasterRxFail_c);            
            }            
          }        
        }
        else
        { /* Slave mode */
          /* Put the received byte in the Rx buffer */
          if(pfSpiSlaveRxCallBack)  
          {
            maSpiRxBuf[mSpiRxBufLeadingIndex] = SPI2D;
            
            /* Update the Rx buffer leading index */
            if(++mSpiRxBufLeadingIndex >= gSPI_SlaveReceiveBufferSize_c)
            {
              mSpiRxBufLeadingIndex = 0; 
            }
            
            /* Update the Rx buffer byte count */
            if(mSpiRxBufferByteCount < gSPI_SlaveReceiveBufferSize_c)
            {
              mSpiRxBufferByteCount++;
            }
            else
            {
              /* If the number of bytes to be read exceed the Rx buffer size then
                 update also the trailing index */
              if(++mSpiRxBufTrailingIndex >= gSPI_SlaveReceiveBufferSize_c)
              {
                mSpiRxBufTrailingIndex = 0;
              }
            }
            
            /* Let the application know a byte has been receive. */
            TS_SendEvent(gSpiTaskId, gSPI_Event_SlaveRx_c);  
          }
          else
          {
            /* Dummy read of the SPI Data Register in order to clear the SPRF flag */
            SPI2D;
          }
        }      
      }
    
      /* Check if the interrupt is caused by a mode fault event */
      if((spiStatusReg & mSPIxS_MODF_c) && (SPI2C1 & mSPIxC1_SPIE_c))
      { /* MODF */
        /* Check the mode of operation: master or slave */
        if(SPI2C1 & mSPIxC1_MSTR_c)
        { /* Master mode */
          /* Turn the SPI module in slave mode. Writting to the SPIxC1 will also 
             clear the MODF flag */
          SPI2C1 &= ~mSPIxC1_MSTR_c;
        }    
      } 
    #endif      
  }
#endif

#pragma CODE_SEG DEFAULT

/******************************************************************************
*******************************************************************************
* Private functions 
*******************************************************************************
******************************************************************************/
#if(gSPI_Enabled_d == TRUE)
  /******************************************************************************
  *  SPI1_SlaveSendNextByte
  *
  *  Send the next byte when the SPI is in slave transmit; it also manages the 
  *  Tx circular buffers 
  ******************************************************************************/
  #if defined(PROCESSOR_MC1323X)
    static void SPI1_SlaveSendNextByte(void)
    {
      void (*pfSpiCallBack)(uint8_t *pBuf);
      
      if(maSpiTxBufRefTable[mSpiTxBufRefTrailingIndex].mTxBufLen)
      {
        /* Write a byte */
        SPID = maSpiTxBufRefTable[mSpiTxBufRefTrailingIndex].pTxBuf[mSpiTxBufRefCurIndex]; 
        
        #if gSPI_Slave_TxDataAvailableSignal_Enable_c
          SPI_SignalTxDataAvailable(TRUE);
        #endif
         
        /* Finished with this buffer? */
        if(++mSpiTxBufRefCurIndex >= maSpiTxBufRefTable[mSpiTxBufRefTrailingIndex].mTxBufLen)
        {
          /* Mark this one as done and call the callback */
          maSpiTxBufRefTable[mSpiTxBufRefTrailingIndex].mTxBufLen = 0;
          pfSpiCallBack = maSpiTxBufRefTable[mSpiTxBufRefTrailingIndex].pfTxBufCallBack;
          
          if(pfSpiCallBack)
          {        
            /* Add the callback information to the callback table */
            maSpiTxCallbackTable[maSpiTxCallbackLeadingIndex].pfTxCallBack  = pfSpiCallBack;
            maSpiTxCallbackTable[maSpiTxCallbackLeadingIndex].pTxBuf        = maSpiTxBufRefTable[mSpiTxBufRefTrailingIndex].pTxBuf;
            
            /* Increment and wrap around the Tx callback leading index */
            if(++maSpiTxCallbackLeadingIndex >= gSPI_SlaveTransmitBuffersNo_c)
            {
              maSpiTxCallbackLeadingIndex = 0;
            }
            
            /* Signal the SPI Task that we got a callback to be executed */
            TS_SendEvent(gSpiTaskId, gSPI_Event_SlaveTx_c);         
          }
          
          /* Reset the character count */
          mSpiTxBufRefCurIndex = 0;
          
          /* Increment and wrap around the trailing index */
          if(++mSpiTxBufRefTrailingIndex >= gSPI_SlaveTransmitBuffersNo_c)
          {
            mSpiTxBufRefTrailingIndex = 0;  
          }    
        }       
      }
      else
      {
        /* Write a dummy byte in order to clear the SPTEF flag */
        SPID = 0x00;     
      }
               
    }
  #endif
  
 
  /******************************************************************************
  *  SPI2_SlaveSendNextByte
  *
  *  Send the next byte when the SPI is in slave transmit; it also manages the 
  *  Tx circular buffers 
  ******************************************************************************/ 
  #if defined(PROCESSOR_QE128)
    static void SPI2_SlaveSendNextByte(void)
    {
      void (*pfSpiCallBack)(uint8_t *pBuf);
      
      if(maSpiTxBufRefTable[mSpiTxBufRefTrailingIndex].mTxBufLen)
      {
        /* Write a byte */
        SPI2D = maSpiTxBufRefTable[mSpiTxBufRefTrailingIndex].pTxBuf[mSpiTxBufRefCurIndex]; 
        
        #if gSPI_Slave_TxDataAvailableSignal_Enable_c
          SPI_SignalTxDataAvailable(TRUE);
        #endif
         
        /* Finished with this buffer? */
        if(++mSpiTxBufRefCurIndex >= maSpiTxBufRefTable[mSpiTxBufRefTrailingIndex].mTxBufLen)
        {
          /* Mark this one as done and call the callback */
          maSpiTxBufRefTable[mSpiTxBufRefTrailingIndex].mTxBufLen = 0;
          pfSpiCallBack = maSpiTxBufRefTable[mSpiTxBufRefTrailingIndex].pfTxBufCallBack;
          
          if(pfSpiCallBack)
          {        
            /* Add the callback information to the callback table */
            maSpiTxCallbackTable[maSpiTxCallbackLeadingIndex].pfTxCallBack  = pfSpiCallBack;
            maSpiTxCallbackTable[maSpiTxCallbackLeadingIndex].pTxBuf        = maSpiTxBufRefTable[mSpiTxBufRefTrailingIndex].pTxBuf;
            
            /* Increment and wrap around the Tx callback leading index */
            if(++maSpiTxCallbackLeadingIndex >= gSPI_SlaveTransmitBuffersNo_c)
            {
              maSpiTxCallbackLeadingIndex = 0;
            }
            
            /* Signal the SPI Task that we got a callback to be executed */
            TS_SendEvent(gSpiTaskId, gSPI_Event_SlaveTx_c);         
          }
          
          /* Reset the character count */
          mSpiTxBufRefCurIndex = 0;
          
          /* Increment and wrap around the trailing index */
          if(++mSpiTxBufRefTrailingIndex >= gSPI_SlaveTransmitBuffersNo_c)
          {
            mSpiTxBufRefTrailingIndex = 0;  
          }    
        }       
      }
      else
      {
        /* Write a dummy byte in order to clear the SPTEF flag */
        SPI2D = 0x00;     
      }
               
    }
  #endif  
#endif /*(gSPI_Enabled_d == TRUE)*/
