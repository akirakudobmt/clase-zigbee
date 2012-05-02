/*****************************************************************************
* IIC Serial Port implementation.
*
* (c) Copyright 2008, Freescale, Inc. All rights reserved.
*
*
* No part of this document must be reproduced in any form - including copied,
* transcribed, printed or by any electronic means - without specific written
* permission from Freescale Semiconductor.
*
*****************************************************************************/

#include "IIC.h"
#include "IIC_Interface.h"
#include "IoConfig.h"
#include "IrqControlLib.h"

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

/*Set the IIC task Priority  */
#ifndef gTsI2CTaskPriority_c
#define gTsI2CTaskPriority_c             0x10
#endif 

/*****************************************************************************
******************************************************************************
* Private macros
******************************************************************************
*****************************************************************************/

#if gIIC_Slave_TxDataAvailableSignal_Enable_c

/* Verify if there is data to be received from the Slave */
#define IIC_IsTxDataAvailable()  ( (gIIC_TxDataAvailablePortDataReg_c & gIIC_TxDataAvailablePinMask_c) == 0)
#endif
/*****************************************************************************
******************************************************************************
* Private type definitions
******************************************************************************
*****************************************************************************/
/* The transmit code keeps a circular list of buffers to be sent */
typedef struct IIcTxBufRef_tag {
  unsigned char const *pBuf;
  void (*pfCallBack)(unsigned char const *pBuf);
} IIcTxBufRef_t;

typedef enum
  {
    mIIC_OpType_Tx_c,
    mIIC_OpType_Rx_c
  }iicOpType_t;

typedef struct iicMasterOp_tag {
  iicOpType_t iicOpType;
  uint8_t *pBuf;
  uint8_t bufLen;
} iicMasterOp_t;

/*****************************************************************************
******************************************************************************
* Public memory definitions
******************************************************************************
*****************************************************************************/
#if gIIC_Enabled_d
/* ID for IIC task */
         tsTaskID_t   gIIcTaskId;
volatile index_t      mIIcRxBufferByteCount;

#endif


/*****************************************************************************
******************************************************************************
* Private memory definitions
******************************************************************************
*****************************************************************************/
#if gIIC_Enabled_d
/* The leading index is the next position to store a buffer reference.
    The trailing index is the buffer currently being transmitted.
    Using a separate table for the buffer lengths reduces code size */    
static IIcTxBufRef_t  maIIcTxBufRefTable[gIIC_SlaveTransmitBuffersNo_c];
static index_t        maIIcTxBufLenTable[gIIC_SlaveTransmitBuffersNo_c];
static index_t        mIIcTxCurIndex;
static index_t        mIIcTxBufRefLeadingIndex;    /* Post-increment. */
static index_t        mIIcTxBufRefTrailingIndex;   /* Post-increment. */

/* Callback table and index pointers */
static IIcTxBufRef_t  maIIcTxCallbackTable[gIIC_SlaveTransmitBuffersNo_c];
static index_t        maIIcTxCallbackLeadingIndex   = 0; 
static index_t        maIIcTxCallbackTrailingIndex  = 0; 

/* I2C module has a small local circular Rx buffer to store data until the
    application can retrieve it. In addition to the leading and trailing
    indexes, the Rx code keeps a count of the number of bytes in the buffer. */
static uint8_t        maIIcRxBuf[gIIC_SlaveReceiveBufferSize_c];
static index_t        mIIcRxBufLeadingIndex;       /* Post-increment. */
static index_t        mIIcRxBufTrailingIndex;      /* Post-increment. */
/* Local variable to keep the I2C Rx callback provided by the application */
static void           (*pfIIcSlaveRxCallBack)(void);
static void           (*pfIIcMasterRxCallBack)(bool_t status);
static void           (*pfIIcMasterTxCallBack)(bool_t status);
static iicMasterOp_t   mIICMasterOp;

#endif

/*****************************************************************************
******************************************************************************
* Private prototypes
******************************************************************************
*****************************************************************************/
#if(gIIC_Enabled_d)
  static void IIC_TaskInit(void);
  static void IIC_SendNextByte(void);
  static void IIC_ModuleAddressedByMaster(void);
#endif

/*****************************************************************************
******************************************************************************
* Public functions
******************************************************************************
*****************************************************************************/

/*****************************************************************************
*   IIC_ModuleInit 
*
*   Initializes the I2C module 
******************************************************************************/
void IIC_ModuleInit(void)
{
#if gIIC_Enabled_d  
  /* Configure the I2C hardware peripheral */
  mIIC_C_c = mIICC_Reset_c;
  /* Clear the I2C Rx software buffer */
  mIIcRxBufLeadingIndex = mIIcRxBufTrailingIndex = mIIcRxBufferByteCount = 0;
  pfIIcSlaveRxCallBack = NULL;
  pfIIcMasterTxCallBack = pfIIcMasterRxCallBack = NULL;
  #if  gIIC_Slave_TxDataAvailableSignal_Enable_c
/* Configure as output the GPIO that will be used to signal to the host that
     the blackBox I2C slave device has data to be transmitted */
/* Signal to the host that there are no data available to be read */     
  gIIC_TxDataAvailablePortDataReg_c |= gIIC_TxDataAvailablePinMask_c; 
  gIIC_TxDataAvailablePortDDirReg_c |= gIIC_TxDataAvailablePinMask_c;
#endif

  mIIC_S_c = mIICS_Init_c;   
  mIIC_A_c = (gIIC_DefaultSlaveAddress_c << 1);
  mIIC_F_c = gIIC_DefaultBaudRate_c;
#if defined(PROCESSOR_MC1323X)  
  IIC1C2 = mIICxC2_Init_c; 
#endif
 /* Create I2C module main task */
  IIC_TaskInit();
  mIIC_C_c = mIICC_Init_c;

#endif  
}




/*****************************************************************************
*   IIC_ModuleUninit 
*
*   Resets the I2C module.
******************************************************************************/
void IIC_ModuleUninit(void)
{
#if gIIC_Enabled_d
  mIIC_C_c = mIICC_Reset_c;
  
  /* Destroy the IIC task */
  if(gIIcTaskId != gTsInvalidTaskID_c)
  {
    TS_DestroyTask(gIIcTaskId);  
  }
#endif
}
/*****************************************************************************
*   IIC_SetBaudRate 
*
*   Resets the I2C module.
******************************************************************************/
bool_t IIC_SetBaudRate(uint8_t baudRate)
{
#if gIIC_Enabled_d
   if(mIIC_S_c & mIICS_BUSY_c)
    {
     return FALSE; 
    }
   mIIC_F_c = baudRate;
   return TRUE;
#else
  {
   (void)baudRate;
   return FALSE; 
  }
#endif
}


/*****************************************************************************
*   IIC_SetSlaveAddress 
*
*   Sets the slave address of the I2C module.
******************************************************************************/
bool_t IIC_SetSlaveAddress(uint8_t slaveAddress)
{
  /* Check if the I2C address is valid */
  if((slaveAddress > 0x7f) || 
     (((slaveAddress & 0x78) == 0) && ((slaveAddress & 0x07) != 0)) || 
     ((slaveAddress & 0x78) == 0x78))
  {
    return FALSE;
  }
  {
    mIIC_A_c = (slaveAddress << 1);
    return TRUE;
  }
}


/*****************************************************************************
*   IIC_BusRecovery 
*
*   Resets the I2C module.
******************************************************************************/
void IIC_BusRecovery(void)
{
#if gIIC_Enabled_d
   uint8_t iicControlReg;
   iicControlReg = mIIC_C_c;
   mIIC_C_c = mIICC_Reset_c;
   mIIC_C_c = mIICC_MST_c;
   mIIC_C_c |= mIICC_IICEN_c;
   mIIC_S_c = mIICS_Init_c;
   mIIC_D_c;
   while((mIIC_S_c & mIICS_IICIF_c) == 0);
   mIIC_C_c &= ~mIICC_MST_c;
   mIIC_S_c = mIICS_Init_c;
   mIIC_C_c = iicControlReg;
   
#endif
}


/*****************************************************************************
*   IIC_SetRxCallBack 
*
*   Sets a pointer to the function to be called whenever a byte is received.
*   If the pointer is set to NULL clear the Rx buffer.
******************************************************************************/
void IIC_SetSlaveRxCallBack(void (*pfCallBack)(void))
{
#if !gIIC_Enabled_d
  (void) pfCallBack;
#else
  pfIIcSlaveRxCallBack = pfCallBack;
#endif
}


/*****************************************************************************
*   IIC_Transmit_Slave 
*
*   Begin transmitting size bytes of data from *pBuffer.
*   Returns FALSE if there are no more slots in the buffer reference table.
******************************************************************************/
bool_t IIC_Transmit_Slave(uint8_t const *pBuf, index_t bufLen, void (*pfCallBack)(uint8_t const *pBuf)) 
{
#if !gIIC_Enabled_d
  (void) pBuf;
  (void) bufLen;
  (void) pfCallBack;
#else
  /* Handle empty buffers. */
  if (!bufLen) 
   {
    if(pfCallBack)
     {
      (*pfCallBack)(pBuf);  
     }
    return TRUE;
   }

  /* Room for one more? */
  if (maIIcTxBufLenTable[mIIcTxBufRefLeadingIndex]) 
  {
    return FALSE;
  }

  maIIcTxBufRefTable[mIIcTxBufRefLeadingIndex].pBuf = pBuf;
  maIIcTxBufRefTable[mIIcTxBufRefLeadingIndex].pfCallBack = pfCallBack;
  /* This has to be last, in case the Tx ISR finishes with the previous */
  /* buffer while this function is executing. */
  maIIcTxBufLenTable[mIIcTxBufRefLeadingIndex] = bufLen;

  if (++mIIcTxBufRefLeadingIndex >= NumberOfElements(maIIcTxBufRefTable)) 
  {
    mIIcTxBufRefLeadingIndex = 0;
  }
  
#if gIIC_Slave_TxDataAvailableSignal_Enable_c 
  /* Signal to host that there are data to receive */
  IIC_TxDataAvailable(TRUE);
#endif
  
#endif  
  
  return TRUE;
} 
/*****************************************************************************
*   IIC_Transmit_Master 
*
*   Begin transmitting size bytes of data from *pBuffer.
*   Returns FALSE if there are no more slots in the buffer reference table.
******************************************************************************/
bool_t IIC_Transmit_Master(uint8_t const *pBuf, index_t bufLen, uint8_t destAddress, void (*pfCallBack)(bool_t status)) 
{
#if !gIIC_Enabled_d
  (void) pBuf;
  (void) bufLen;
  (void) destAddress;
  (void) pfCallBack;
#else
  /* Handle empty buffers. */
  if (!bufLen) 
   {
    if(pfCallBack)
     {
      (*pfCallBack)(TRUE);  
     }
    return TRUE;
   }
   destAddress <<= 1;
   if(destAddress == mIIC_A_c)
    {
    return FALSE;
    }
   // pfIIcMasterTxCallBack is reset by IIC task after the callback is called
   //if pfIIcMasterTxCallBack is != 0 it means that the previous callback didn't run yet  
   if(pfIIcMasterTxCallBack) 
    {
      return FALSE;
    }
   if(mIIC_S_c & mIICS_BUSY_c)
    {
     return FALSE; 
    }
    mIICMasterOp.iicOpType = mIIC_OpType_Tx_c;
    mIICMasterOp.pBuf = (uint8_t*)pBuf;
    mIICMasterOp.bufLen = bufLen;
    pfIIcMasterTxCallBack = pfCallBack;
    
    mIIC_C_c |= (mIICC_MST_c | mIICC_TX_c);// start condition
    mIIC_D_c = destAddress; // address the slave for writting
  
  
#endif  
  
  return TRUE;
} 

/*****************************************************************************
*   IIC_Receive_Master 
*
*   Begin transmitting size bytes of data from *pBuffer.
*   Returns FALSE if there are no more slots in the buffer reference table.
******************************************************************************/
bool_t IIC_Receive_Master(uint8_t *pBuf, index_t bufLen, uint8_t destAddress, void (*pfCallBack)(bool_t status)) 
{
#if !gIIC_Enabled_d
  (void) pBuf;
  (void) bufLen;
  (void) destAddress;
  (void) pfCallBack;
#else
  /* Handle empty buffers. */
  if (!bufLen) 
   {
    if(pfCallBack)
     {
      (*pfCallBack)(TRUE);  
     }
    return TRUE;
   }
   destAddress <<= 1;
   if(destAddress == mIIC_A_c)
    {
    return FALSE;
    }
   // pfIIcMasterTxCallBack is reset by IIC task after the callback is called
   //if pfIIcMasterTxCallBack is != 0 it means that the previous callback didn't run yet  
   if(pfIIcMasterRxCallBack) 
    {
      return FALSE;
    }
   if(mIIC_S_c & mIICS_BUSY_c)
    {
     return FALSE; 
    }
    mIICMasterOp.iicOpType = mIIC_OpType_Rx_c;
    mIICMasterOp.pBuf = pBuf;
    mIICMasterOp.bufLen = bufLen;
    pfIIcMasterRxCallBack = pfCallBack;
    
    mIIC_C_c |= (mIICC_MST_c | mIICC_TX_c);// start condition
    mIIC_D_c = destAddress  | 0x1; // address the slave for reading
#endif  
  
  return TRUE;
} 

/*****************************************************************************
*   IIC_IsSlaveTxActive 
*
*   Returns TRUE if there is still data to be transmitted to the master.
*   Returns FALSE if nothing left to transmit.
******************************************************************************/
bool_t IIC_IsSlaveTxActive(void) 
{
#if !gIIC_Enabled_d
  return FALSE;
#else  
  return (maIIcTxBufLenTable[mIIcTxBufRefTrailingIndex] != 0) || ((mIIC_S_c & mIICS_BUSY_c));
#endif  
}

/*****************************************************************************
*   IIC_TxDataAvailable 
*
*   Depending on bool parameter master will be signaled that need to read data from slave
******************************************************************************/
void IIC_TxDataAvailable(bool_t bIsAvailable)
{
#if gIIC_Slave_TxDataAvailableSignal_Enable_c
  if(bIsAvailable) 
    {
     gIIC_TxDataAvailablePortDataReg_c &= ~(gIIC_TxDataAvailablePinMask_c); 
    }
  else
    {
     gIIC_TxDataAvailablePortDataReg_c |= gIIC_TxDataAvailablePinMask_c; 
    }
#else
(void)bIsAvailable;
#endif    
}

/*****************************************************************************
*   IIC_GetByteFromRxBuffer 
*
*   Retrieves a byte from the driver's Rx buffer and store it at *pDst.
*   Return TRUE if a byte was retrieved; FALSE if the Rx buffer is empty.
******************************************************************************/
bool_t IIC_GetByteFromRxBuffer(unsigned char *pDst)
{
#if !gIIC_Enabled_d
  (void) pDst;
#else
  /* Temp storage for I2C control register */
  uint8_t iicControlReg;
  
  if (!mIIcRxBufferByteCount) 
    return FALSE;
  IrqControlLib_PushIrqStatus();
  IrqControlLib_DisableAllIrqs();
  iicControlReg = mIIC_C_c;
  mIIC_C_c &= ~mIICC_IICIE_c;
  IrqControlLib_PullIrqStatus();
  *pDst = maIIcRxBuf[mIIcRxBufTrailingIndex];
  if (++mIIcRxBufTrailingIndex >= sizeof(maIIcRxBuf)) {
    mIIcRxBufTrailingIndex = 0;
  }
  --mIIcRxBufferByteCount;
  mIIC_C_c = iicControlReg;
 #endif  

  return TRUE;
}




/*****************************************************************************
*   IIC_Task 
*
*   Main task of the I2C module 
******************************************************************************/
void IIC_Task(event_t events)
{
#if !gIIC_Enabled_d
  (void) events;
#else

  void (*pfCallBack)(uint8_t const *pBuf);

  if (events & gIIC_Event_SlaveRx_c)
  {
    pfIIcSlaveRxCallBack();
  }

  if (events & gIIC_Event_SlaveTx_c)
  {
    /* Tx call back event received - run through the callback table and execute any
         pending Callbacks */
    while (maIIcTxCallbackLeadingIndex != maIIcTxCallbackTrailingIndex)
    {
       pfCallBack = maIIcTxCallbackTable[maIIcTxCallbackTrailingIndex].pfCallBack;
      /* Call calback with buffer info as parameter */
      (*pfCallBack)(maIIcTxCallbackTable[maIIcTxCallbackTrailingIndex].pBuf);
      /* Increment and wrap around trailing index */
      if (++maIIcTxCallbackTrailingIndex >= NumberOfElements(maIIcTxCallbackTable))
      {
        maIIcTxCallbackTrailingIndex = 0;
      }
    }
  }
  
  if (events & gIIC_Event_MasterRxFail_c)
    {
     (*pfIIcMasterRxCallBack)(FALSE); 
     pfIIcMasterRxCallBack = NULL;
    }
  if (events & gIIC_Event_MasterRxSuccess_c)
    {
     (*pfIIcMasterRxCallBack)(TRUE); 
     pfIIcMasterRxCallBack = NULL;
    }    
  if (events & gIIC_Event_MasterTxFail_c)  
    {
      (*pfIIcMasterTxCallBack)(FALSE);
      pfIIcMasterTxCallBack = NULL;
    }
  if (events & gIIC_Event_MasterTxSuccess_c)  
    {
      (*pfIIcMasterTxCallBack)(TRUE);
      pfIIcMasterTxCallBack = NULL;
    }    
#endif  
} 


/*****************************************************************************
*  IIC_Isr
*
*  I2C Interrupt Service Routine.
******************************************************************************/
/* Place it in NON_BANKED memory */
#ifdef MEMORY_MODEL_BANKED
#pragma CODE_SEG __NEAR_SEG NON_BANKED
#else
#pragma CODE_SEG DEFAULT
#endif /* MEMORY_MODEL_BANKED */

INTERRUPT_KEYWORD void IIC_Isr(void)
{

#if gIIC_Enabled_d

  /* Clear the interrupt request */
  mIIC_S_c |= mIICS_IICIF_c;
  /* Check arbitration  and slave addressing*/
  if (mIIC_S_c & (mIICS_ARBL_c | mIICS_IAAS_c ))
  {
    if (mIIC_S_c & mIICS_IAAS_c)
      {
      /* Check if I2C module was addressed for read or for write */
      if(mIIC_S_c & mIICS_SRW_c)
        {
         /* Configure I2C module for Tx operation.Writing to IICxC register also clears IAAS bit */
         mIIC_C_c |= mIICC_TX_c;
         /* Send next byte from the current Tx buffer */
         IIC_SendNextByte();
        }
      else
        {
         /* Configure I2C module for Rx operation.Writing to IICxC register also clears IAAS bit */
         mIIC_C_c &= ~(mIICC_TX_c | mIICC_TXAK_c);
         mIIC_D_c;
        }

      }
    if(mIIC_S_c & mIICS_ARBL_c)
      {
      /* Arbitration lost */
       mIIC_S_c |= mIICS_ARBL_c;    
       
       if(mIICMasterOp.iicOpType == mIIC_OpType_Tx_c)
        {
         if(pfIIcMasterTxCallBack)
          {
           TS_SendEvent(gIIcTaskId, gIIC_Event_MasterTxFail_c); 
          }
          
        }
       else
        {
        if(pfIIcMasterRxCallBack)
          {
          TS_SendEvent(gIIcTaskId, gIIC_Event_MasterRxFail_c);    
          }
        
        }
      }
  }
  else
  {
    /* Arbitration okay
       Check addressing */
  if(mIIC_C_c & mIICC_MST_c)/* Master mode */
    {
     if(mIIC_C_c & mIICC_TX_c)
      {
        if(mIIC_S_c & mIICS_RXAK_c)// No ack received
          {
            mIIC_C_c &= ~(mIICC_MST_c | mIICC_TX_c);
            if(mIICMasterOp.iicOpType == mIIC_OpType_Tx_c)	// Tx
              {
              if(pfIIcMasterTxCallBack)
               {
                TS_SendEvent(gIIcTaskId, gIIC_Event_MasterTxFail_c); 
               }
              }
            else	// Rx
              {
              if(pfIIcMasterRxCallBack)
               {
                TS_SendEvent(gIIcTaskId, gIIC_Event_MasterRxFail_c);    
               }
              }
          }
        else	// Ack received
          {
            if(mIICMasterOp.iicOpType == mIIC_OpType_Rx_c)	// Rx
              {
              mIIC_C_c &= ~(mIICC_TX_c | mIICC_TXAK_c);
              if(mIICMasterOp.bufLen-- == 1)  
                {
                 mIIC_C_c |= mIICC_TXAK_c;
                }
              mIIC_D_c;  
              }
            else	// Tx
              {
               if(mIICMasterOp.bufLen)
                {
                 mIIC_D_c =  *mIICMasterOp.pBuf++ ;
                 mIICMasterOp.bufLen--;
                }
               else
                {
                 mIIC_C_c &= ~(mIICC_MST_c | mIICC_TX_c); 
                 if(pfIIcMasterTxCallBack)
                  {
                   TS_SendEvent(gIIcTaskId, gIIC_Event_MasterTxSuccess_c); 
                  }
                 
                }
              }
          }
      }
     else// (mIIC_C_c & mIICC_TX_c) == 0       
      {
       if(mIICMasterOp.bufLen == 0)   
        {
        mIIC_C_c &= ~mIICC_MST_c;   
        if(pfIIcMasterRxCallBack)
          {
           TS_SendEvent(gIIcTaskId, gIIC_Event_MasterRxSuccess_c); 
          }
        }
       else if(mIICMasterOp.bufLen-- == 1)  
        {
         mIIC_C_c |= mIICC_TXAK_c;
        }
       *mIICMasterOp.pBuf++ = mIIC_D_c;
        
      }
     
    }
    
  else   /* slave mode*/
    {
    if (mIIC_C_c & mIICC_TX_c)
      {
        /* IIC has Tx a byte to master. Check if ack was received */
        if (mIIC_S_c & mIICS_RXAK_c)
        {
          /* No ack received. Switch back to receive mode */
          mIIC_C_c &= ~mIICC_TX_c;
          mIIC_D_c;
        }
        else
        {
          /* Ack received. Send next byte */
          IIC_SendNextByte();
        }
      }
    else
      {
        /* Put the received byte in the buffer */
        if(pfIIcSlaveRxCallBack)
          {
          maIIcRxBuf[mIIcRxBufLeadingIndex] = mIIC_D_c;

          if (++mIIcRxBufLeadingIndex >= sizeof(maIIcRxBuf)) 
            {
            mIIcRxBufLeadingIndex = 0;
            }

          if (mIIcRxBufferByteCount < sizeof(maIIcRxBuf)) 
            {
            ++mIIcRxBufferByteCount;
            }
          else
            {
             if (++mIIcRxBufTrailingIndex >= sizeof(maIIcRxBuf)) 
               {
                mIIcRxBufTrailingIndex = 0;
               }
            }
            
        /* Let the application know a byte has been received. */
          TS_SendEvent(gIIcTaskId, gIIC_Event_SlaveRx_c);
            
          }
        else
          {
           mIIC_D_c; 
          }
      }  
    }/* Data transfer.Check if it is a Tx or Rx operation */
      
    
  }
  
#endif  
}
#pragma CODE_SEG DEFAULT           


/*****************************************************************************
******************************************************************************
* Private functions
******************************************************************************
*****************************************************************************/

#if gIIC_Enabled_d

/*****************************************************************************
*   IIC_TaskInit 
*
*   Creates the I2C module task
******************************************************************************/
static void IIC_TaskInit(void)
{
  gIIcTaskId = TS_CreateTask(gTsI2CTaskPriority_c, IIC_Task);
}                                       

/*****************************************************************************
*   IIC_SendNextByte 
*
*   Send next byte of information from the Tx buffer
******************************************************************************/
static void IIC_SendNextByte (void)
{
  void (*pfCallBack)(unsigned char const *pBuf);
  
  /* The HCS08 does not save H on interrupt. */
  __asm 
  {
    PSHH
  }

  
#if gIIC_Slave_TxDataAvailableSignal_Enable_c  
  if(IIC_IsTxDataAvailable())
#else
  if(maIIcTxBufLenTable[mIIcTxBufRefTrailingIndex])  
#endif  
  {
    /* Write a byte. */
    mIIC_D_c = maIIcTxBufRefTable[mIIcTxBufRefTrailingIndex].pBuf[mIIcTxCurIndex];

    /* Finished with this buffer? */
    if (++mIIcTxCurIndex >= maIIcTxBufLenTable[mIIcTxBufRefTrailingIndex]) 
    {

      /* Mark this one as done, and call the callback. */
      maIIcTxBufLenTable[mIIcTxBufRefTrailingIndex] = 0;
      pfCallBack = maIIcTxBufRefTable[mIIcTxBufRefTrailingIndex].pfCallBack;
      if (pfCallBack)
      {
        /* Signal the IIC task that we got a callback to be executed */
        TS_SendEvent(gIIcTaskId, gIIC_Event_SlaveTx_c);
        /* Add callback information to the callback table */
        maIIcTxCallbackTable[maIIcTxCallbackLeadingIndex].pfCallBack = pfCallBack;
        maIIcTxCallbackTable[maIIcTxCallbackLeadingIndex].pBuf = maIIcTxBufRefTable[mIIcTxBufRefTrailingIndex].pBuf;
        /* Increment and wrap around the leading index */
        if (++maIIcTxCallbackLeadingIndex >= NumberOfElements(maIIcTxCallbackTable)) {
          maIIcTxCallbackLeadingIndex = 0;
        }
      
      }
      /* Increment to the next buffer. */
      mIIcTxCurIndex = 0;
      if (++mIIcTxBufRefTrailingIndex >= NumberOfElements(maIIcTxBufRefTable)) 
      {
        mIIcTxBufRefTrailingIndex = 0;
      }

#if gIIC_Slave_TxDataAvailableSignal_Enable_c        
      /* If there is no more data to send, turn off the transmit interrupt. */
      if (!maIIcTxBufLenTable[mIIcTxBufRefTrailingIndex]) 
      {
        /* Signal to host that there are no more data to receive */
        IIC_TxDataAvailable(FALSE);
      }
#endif      
    }
  }
  else
  {
    /* Write a byte. */
    mIIC_D_c = 0;
  }
  __asm 
  {
    PULH
  }
}



#endif