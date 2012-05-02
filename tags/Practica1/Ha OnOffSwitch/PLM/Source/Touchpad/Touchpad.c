/*****************************************************************************
* Touchpad Hardware Abstraction Layer
*
* Copyright (c) 2010, Freescale, Inc. All rights reserved
*
* No part of this document must be reproduced in any form - including copied,
* transcribed, printed or by any electronic means, - without specific written 
* permission from Freescale Semiconductor
* 
*****************************************************************************/

#include "EmbeddedTypes.h"
#include "IrqControlLib.h"
#include "IoConfig.h"
#include "TS_Interface.h"
#include "Touchpad_Interface.h"
#include "Touchpad.h"
#include "IIC_Interface.h"
#include "FunctionLib.h"



#pragma MESSAGE DISABLE C4001 // Warning C4001: Condition always FALSE


/*****************************************************************************
******************************************************************************
* Private macros 
******************************************************************************
*****************************************************************************/
/* Touchpad events */
#define gTP_Event_WriteBlockSuccess_c       (1<<1)
#define gTP_Event_WriteBlockFail_c          (1<<2)
#define gTP_Event_ReadBlockSuccess_c        (1<<3)
#define gTP_Event_ReadBlockFail_c           (1<<4)
#define gTP_Event_ISR_c                     (1<<5)

#define gTP_I2cTransfOperMaxSize_c          (8)
#define gTP_DataTableMaxSize_c              (20) 
#define gTP_RmiAddrByteSize_c               (1)

#define gWriteBlockMaxSize_c                (16)
/* In debug mode some table's elements are automatically cleared for more readability */                                          
#define DEBUG_MODE_ENABLE                   (0)  

/* Table operations sizes for each touchpad process */
#define gTP_DevRestart_TransfOperNo_c       (1)
#define gTP_GetDevInfo_TransfOperNo_c       (4)
#define gTP_GetDevConfig_TransfOperNo_c     (2)
#define gTP_SetDevConfig_TransfOperNo_c     (5)
#define gTP_Isr_TransfOperNo_c              (2)

/*****************************************************************************
******************************************************************************
* Private type definitions 
******************************************************************************
*****************************************************************************/

/* Type of operation */
typedef enum
{
  mTP_OperType_Read_c  = 0,     
  mTP_OperType_Write_c  
} tpOperType_t;
             
/* Structure used to define the entry element in the table of operations */
typedef struct tpI2cTransfOper_tag  
{   
  uint8_t             regAddr;
  uint8_t*            pBuff;
  uint8_t             buffLength;     
  tpOperType_t        tpOperType;
} tpI2cTransfOper_t;
  
             
/* Structure to control the RMI operation over the IIC */
typedef struct 
{   
  bool_t                accessLock;
  tpI2cTransfOper_t     rmiBlockAccess;    
} rmiBlockAccess_t;

/*****************************************************************************
******************************************************************************
* Private memory declaration 
******************************************************************************
*****************************************************************************/
#if(gTouchpadIncluded_d == 1)

/* Id for the Touchpad task */
static tsTaskID_t             gTP_TaskId;

static  uint8_t               gTP_IntStatus;
static  uint8_t               gTP_DevStatus;

/* Variables associated with I2C communication */
static tpI2cTransfOper_t      maTP_I2cTransfOperTable[gTP_I2cTransfOperMaxSize_c];
static volatile index_t       maTP_I2cTransfOperNo;             
static volatile index_t       maTP_I2cOperCurrIndex;

static uint8_t                maDataTable[gTP_DataTableMaxSize_c];
static rmiBlockAccess_t       gRmiBlockAccess; 
    
/* Structure containing all information required by the application's callback function */
tpCallbackInfo_t              gTP_CallbackInfo;

static uint8_t                gWriteBlock[gWriteBlockMaxSize_c]; 

/* Block size used to be read by the touchpad task */
static uint8_t gBlockSize = 0; /* However the global variables are cleared at initialization */

/* Variable counting the number of ISR passing during reset operation */
static volatile uint8_t       gTP_DevResetIdx;
                 
#endif

/*****************************************************************************
******************************************************************************
* Private function prototypes 
******************************************************************************
*****************************************************************************/

/* TP Task; Process the TP events in interrupt-driven context */
void TP_Task(event_t events);
#if(gTouchpadIncluded_d == 1)
  /* TP callback function */
  static void (*pfTpCallback)(tpCallbackInfo_t tpCallbackInfo);

  /*****************************************************************************/
  static void TP_SaveDevInfo(void);
  static void TP_SaveDevConfig(void);
  static void TP_UpdateDevConfig(uint8_t mIntEn0Reg, uint8_t mGestEn1Reg); 
  static statusCode_t TP_GetFingerInfo(uint8_t mFingerNum);
  static void TP_GestureRecognition(void);
  static bool_t TP_ReadBlock(uint8_t startAddr, uint8_t blockLength, uint8_t* pRxBuff);       
  static bool_t TP_WriteBlock(uint8_t startAddr, uint8_t blockLength, uint8_t* pTxBuff);
  static void RmiWriteAddrCallback(bool_t status);
  static void RmiCompleteAccessCallback(bool_t status);
#endif //(gTouchpadIncluded_d == 1)

/*****************************************************************************
******************************************************************************
* Private functions 
******************************************************************************
*****************************************************************************/

/*****************************************************************************
* TP_Init
* 
* Interface assumption:
*
* Return value: generic status code
*
* Initialize the touchpad sensor, IIC communication, global variables
*****************************************************************************/
statusCode_t TP_Init(void)
{
#if(gTouchpadIncluded_d == 1)
  uint8_t retStatus = TP_OK;

  if(FALSE == IIC_SetBaudRate(gTP_Default_IIC_CommRate_c))
  {
    retStatus = TP_ERR_IIC_COMM;     
  }
  else
  {
    /* Create the touchpad task */
    gTP_TaskId = TS_CreateTask(gTP_TaskPriority_c, TP_Task); 
    
    if(gTsInvalidTaskID_c != gTP_TaskId)
    {
      /* Set the pointer callback to NULL */
      pfTpCallback = NULL; 
      
      #if(gTP_Mode_c == gTP_IrqIsrMode_c)
        /* IRQ pin settings */
        m_IRQSC_c =(mIRQSC_IRQPE_c | mIRQSC_IRQIE_c | mIRQ_FallingEdge_Sense_c);
      #endif
      #if(gTP_Mode_c == gTP_KbiIsrMode_c)
        /* KBI2P0 pin setting */
        m_KBI2PE_c |= mKBI2PE_KBIPE0_c;
        
        m_KBI2SC_c = mKBI2SC_KBIE_c;
      #endif
      
      /* Initialize the block access flag */
      gRmiBlockAccess.accessLock = FALSE;
      
      /* Clear all values from the global variable responsible for passing the 
         information to the callback */
      FLib_MemSet(&gTP_CallbackInfo, 0, sizeof(tpCallbackInfo_t));
      
      /* Reset the variable which detect the reset operation */
      gTP_DevResetIdx = 0;      
    }
    else
    {
      retStatus = TP_ERR_INVALID_PARAM;
    }
  } 
         
  return retStatus; 
#else    
  return  TP_OK;
#endif    
}

/*****************************************************************************
* TP_Uninit
*
* Interface assumption:
*
* Return value: None
* 
* Uninit the touchpad sensor, IIC communication, disallocates the memory for 
* global variable
*****************************************************************************/
void TP_Uninit(void)
{
#if(gTouchpadIncluded_d == 1)   
  /* Destroy the touchpad task if there is a valid ID */
  if(gTsInvalidTaskID_c != gTP_TaskId)
  {
    TS_DestroyTask(gTP_TaskId);  
  }
  
  #if(gTP_Mode_c == gTP_IrqIsrMode_c)
    /* IRQ pin disable */
    m_IRQSC_c = mIRQSC_Reset_c;
  #endif
  #if(gTP_Mode_c == gTP_KbiIsrMode_c)
    /* KBI2P0 pin disable */
    m_KBI2SC_c = mKBI2SC_Reset_c;
  #endif
#endif
}

    
/*****************************************************************************
* TP_DevRestart
* 
* Interface assumption:
*
* Return value: generic status code
*
* Reset the touchpad sensor
* 
*****************************************************************************/
statusCode_t TP_DevRestart(void)            
{    
#if(gTouchpadIncluded_d == 1)  
  uint8_t retStatus = TP_OK;
  tpI2cTransfOper_t* pTransfOper = maTP_I2cTransfOperTable;
             
  /* Store the function id */
  gTP_CallbackInfo.funcId = gTP_DevRestartId_c;
  
  /* Reset the variable counting the External ISR occurence; during reset the 
     external pin interrupt is going twice */
  gTP_DevResetIdx = 0;     
  
  /* Reset the current transfer count */
  maTP_I2cTransfOperNo = gTP_DevRestart_TransfOperNo_c;
 
  /* Prepare the first entry into the action table: Reset sensor */
  *maDataTable = gTP_DevReset_c;   
  pTransfOper->regAddr    = PDT_F01_RMI_COMMAND_BASE + mRMI_DeviceCommand_Offset_c;
  pTransfOper->pBuff      = maDataTable;
  pTransfOper->buffLength = 1;
  pTransfOper->tpOperType = mTP_OperType_Write_c;
    
  /* Prepare the second entry to be read: Interrupt status: this is necessary to deassert the ATTN line again */
  (++pTransfOper)->regAddr  = PDT_F01_RMI_DATA_BASE + mRMI_InterruptStatus_Offset_c; 
  pTransfOper->pBuff        = maDataTable + 1;
  pTransfOper->buffLength   = 1;
  pTransfOper->tpOperType   = mTP_OperType_Read_c;        
  
  #if(DEBUG_MODE_ENABLE == 1)
    /* Clear the remaining entries in the action table after filling with necessary actions */
    FLib_MemSet((maTP_I2cTransfOperTable + maTP_I2cTransfOperNo + 1), 0, (gTP_I2cTransfOperMaxSize_c - maTP_I2cTransfOperNo - 1) * sizeof(tpI2cTransfOper_t));     

    /* Keep clear the data table */
    FLib_MemSet((maDataTable + maTP_I2cTransfOperNo + 1), 0, (gTP_DataTableMaxSize_c - maTP_I2cTransfOperNo - 1));
  #endif  
  
  /* Prepare one IIC Write operation */
  maTP_I2cOperCurrIndex = 0;
  
  pTransfOper = maTP_I2cTransfOperTable;     
  if(FALSE == TP_WriteBlock(pTransfOper->regAddr, pTransfOper->buffLength, pTransfOper->pBuff))      
  {
    retStatus = TP_ERR_IIC_COMM;
  }
   
  return retStatus;
#else
  return  TP_OK;
#endif   
}


/*****************************************************************************
* TP_GetDevInfo
* 
* Interface assumption:   This function should be called after calling the 
*                         TP_DevRestart() in order to get the correct information 
*
* Return value: generic status code
*
* Collects the information about the device configuration
*****************************************************************************/
statusCode_t TP_GetDevInfo(void)
{                    
#if(gTouchpadIncluded_d == 1)  
  uint8_t retStatus = TP_OK;
  tpI2cTransfOper_t* pTransfOper = maTP_I2cTransfOperTable;
 
  if(gTP_DevResetIdx != 2)
  {
    retStatus = TP_ERR_DEVICE_NOT_READY;
  }
  else
  {
    /* Store the function id */
    gTP_CallbackInfo.funcId = gTP_GetDevInfoId_c;
    
    /* Fill the array with further operations to be accomplished in the TP_Task */
    maTP_I2cTransfOperNo = gTP_GetDevInfo_TransfOperNo_c;    
    
    /* Prepare the first entry to be read: No of fingers, finger's absolute values, 
    finger's relative values, gesture, sensor adjustement capabilities */
    pTransfOper->regAddr      = PDT_F11_2D_QUERY_BASE + m2D_ReportingMode_Offset_c;  
    pTransfOper->pBuff        = maDataTable;      
    pTransfOper->buffLength   = 1;
    pTransfOper->tpOperType   = mTP_OperType_Read_c;
    
    /* Prepare the second entry to be read: gesture types */
    (++pTransfOper)->regAddr  = PDT_F11_2D_QUERY_BASE + m2D_GestureQuery_Offset_c; 
    pTransfOper->pBuff        = maDataTable + 1;      
    pTransfOper->buffLength   = 2;
    pTransfOper->tpOperType   = mTP_OperType_Read_c;    
     
    /* Prepare the third entry to be read: reporting mode */
    (++pTransfOper)->regAddr  = PDT_F11_2D_CONTROL_BASE + m2D_ReportMode_Offset_c; 
    pTransfOper->pBuff        = maDataTable + 3;      
    pTransfOper->buffLength   = 1;
    pTransfOper->tpOperType   = mTP_OperType_Read_c;               

    /* Prepare the fourth entry to be read: product id string */   
    (++pTransfOper)->regAddr  = PDT_F01_RMI_QUERY_BASE + mRMI_ProductIdQuery0_Offset_c;
    pTransfOper->pBuff        = maDataTable + 4;       
    pTransfOper->buffLength   = gTP_ProdIdLengthMax_c;
    pTransfOper->tpOperType   = mTP_OperType_Read_c;      
    
    /* Prepare the fifth entry to be read: maximum X and Y position */               
    (++pTransfOper)->regAddr  = PDT_F11_2D_CONTROL_BASE + m2D_MaxXPositionLow_Offset_c;
    pTransfOper->pBuff        = maDataTable + 4 + gTP_ProdIdLengthMax_c;
    pTransfOper->buffLength   = 4;
    pTransfOper->tpOperType   = mTP_OperType_Read_c;             
    
    #if(DEBUG_MODE_ENABLE == 1)
      /* Clear the left entries in the action table */
      FLib_MemSet((maTP_I2cTransfOperTable + maTP_I2cTransfOperNo + 1), 0, (gTP_I2cTransfOperMaxSize_c - maTP_I2cTransfOperNo - 1) * sizeof(tpI2cTransfOper_t));
               
      /* Keep clear the data table */
      FLib_MemSet(maDataTable, 0, gTP_DataTableMaxSize_c);       
    #endif  
                     
    /* Prepare the first IIC read operation */
    maTP_I2cOperCurrIndex = 0;
     
    pTransfOper = maTP_I2cTransfOperTable;     
    if(FALSE == TP_ReadBlock(pTransfOper->regAddr, pTransfOper->buffLength, pTransfOper->pBuff))     
    {
      retStatus = TP_ERR_IIC_COMM;
    }    
  }  
   
  return retStatus;
#else
  return  TP_OK;
#endif   
}


/*****************************************************************************
* TP_GetDevConfig
* 
* Interface assumption:
*
* Return value: generic status code
*
* Configure the list of gestures to be identified together with their 
* associated parameters
*****************************************************************************/
statusCode_t TP_GetDevConfig(void)     
{
#if(gTouchpadIncluded_d == 1)
  uint8_t retStatus         = TP_OK;
  uint8_t maDataTableIndex  = 0;
  tpI2cTransfOper_t* pTransfOper = maTP_I2cTransfOperTable;
  
  if(FALSE == gTP_CallbackInfo.devInfo.mHasGesture)
  {
    retStatus = TP_ERR_INVALID_PARAM;  
  }
  else
  {    
    /* Store the function id */
    gTP_CallbackInfo.funcId = gTP_GetDevConfigId_c; 
    
    /* Fill the array with further operations to be accomplished in the TP_Task */       
    maTP_I2cTransfOperNo = gTP_GetDevConfig_TransfOperNo_c;
    if((TRUE == gTP_CallbackInfo.devInfo.mGestList.mHasSingleTap) || 
       (TRUE == gTP_CallbackInfo.devInfo.mGestList.mHasDoubleTap) || 
       (TRUE == gTP_CallbackInfo.devInfo.mGestList.mHasTapAndHold))
    {        
      /* Prepare the current entry into the action table: maximum tap time */
      pTransfOper->regAddr      = PDT_F11_2D_CONTROL_BASE + m2D_MaxTapTime_Offset_c;  
      pTransfOper->pBuff        = maDataTable;
      pTransfOper->buffLength   = 1;
      pTransfOper->tpOperType   = mTP_OperType_Read_c;

      /* Prepare the current entry into the action table: maximum tap distance */      
      (++pTransfOper)->regAddr  = PDT_F11_2D_CONTROL_BASE + m2D_MaxTapDistance_Offset_c; 
      pTransfOper->pBuff        = maDataTable + 1;
      pTransfOper->buffLength   = 1;
      pTransfOper->tpOperType   = mTP_OperType_Read_c;      
      
      /* Update the table index and transfer number */
      maDataTableIndex +=2;          
      maTP_I2cTransfOperNo +=2;
      pTransfOper++;
    }

    if(TRUE == gTP_CallbackInfo.devInfo.mGestList.mHasFlick)
    {
      /* Prepare two entries into the action table: minimum flick distance and speed */      
      pTransfOper->regAddr      = PDT_F11_2D_CONTROL_BASE + m2D_MinFlickDistance_Offset_c;  
      pTransfOper->pBuff        = maDataTable + maDataTableIndex;
      pTransfOper->buffLength   = 2;
      pTransfOper->tpOperType   = mTP_OperType_Read_c;      
      
      /* Update the table index */
      maDataTableIndex +=2; 
      maTP_I2cTransfOperNo++;
      pTransfOper++;         
    }
    
    if(TRUE == gTP_CallbackInfo.devInfo.mGestList.mHasPress)
    {
      /* Prepare the current entry into the action table: minimum press time */      
      pTransfOper->regAddr      = PDT_F11_2D_CONTROL_BASE + m2D_MinPressTime_Offset_c;  
      pTransfOper->pBuff        = maDataTable + maDataTableIndex;
      pTransfOper->buffLength   = 1;
      pTransfOper->tpOperType   = mTP_OperType_Read_c;      
      
      /* Update the table index and number of transfers */
      maDataTableIndex++;  
      maTP_I2cTransfOperNo++;
      pTransfOper++;            
    }
    
    if(TRUE == gTP_CallbackInfo.devInfo.mGestList.mHasPalm)
    {     
      pTransfOper->regAddr      = PDT_F11_2D_CONTROL_BASE + m2D_PalmReject_Offset_c;  
      pTransfOper->pBuff        = maDataTable + maDataTableIndex;
      pTransfOper->buffLength   = 1;
      pTransfOper->tpOperType   = mTP_OperType_Read_c;  

      /* Update the table index and number of transfers */
      maDataTableIndex++;  
      maTP_I2cTransfOperNo++;
      pTransfOper++;      
    }       
    
    /* Prepare the read of functions supported by the device */  
    pTransfOper->regAddr      = PDT_F01_RMI_CONTROL_BASE + mRMI_InterruptEnable0_Offset_c;  
    pTransfOper->pBuff        = maDataTable + maDataTableIndex;
    pTransfOper->buffLength   = 1;
    pTransfOper->tpOperType   = mTP_OperType_Read_c;      
    
    /* Update the table index */    
    maDataTableIndex++;
   
    /* Prepare the read of gesture enabled by the device */
    (++pTransfOper)->regAddr  = PDT_F11_2D_CONTROL_BASE + m2D_GestureEnable1_Offset_c;  
    pTransfOper->pBuff        = maDataTable + maDataTableIndex;
    pTransfOper->buffLength   = 1;
    pTransfOper->tpOperType   = mTP_OperType_Read_c;
    
    #if(DEBUG_MODE_ENABLE == 1)
      /* Clear the remaining entries in the action table */
      FLib_MemSet((maTP_I2cTransfOperTable + maTP_I2cTransfOperNo + 1), 0, (gTP_I2cTransfOperMaxSize_c - maTP_I2cTransfOperNo - 1) * sizeof(tpI2cTransfOper_t));
      
      /* Clear the data table before reading from it */      
      FLib_MemSet(maDataTable, 0, gTP_DataTableMaxSize_c);     
    #endif    
      
    /* Prepare the first IIC read operation */
    maTP_I2cOperCurrIndex = 0;
    
    pTransfOper = maTP_I2cTransfOperTable;     
    if(FALSE == TP_ReadBlock(pTransfOper->regAddr, pTransfOper->buffLength, pTransfOper->pBuff))      
    {
      retStatus = TP_ERR_IIC_COMM;
    }                              
  }
  
  return retStatus; 
#else
  return  TP_OK;
#endif    
}



/*****************************************************************************
* TP_SetDevConfig
* 
* Interface assumption:
*
* Return value: generic status code
*
* Configure the list of gestures to be identified together with their 
* associated parameters
*****************************************************************************/
statusCode_t TP_SetDevConfig (devConfig_t mDevConfig)
{      
#if(gTouchpadIncluded_d == 1)  
  uint8_t retStatus = TP_OK;
  bool_t  nextStep  = TRUE;
  uint8_t dummy;
  tpI2cTransfOper_t*  pTransfOper = maTP_I2cTransfOperTable;  
       
  if(FALSE == gTP_CallbackInfo.devInfo.mHasGesture)
  {
    retStatus =  TP_ERR_INVALID_PARAM;
  }
  else
  {     
    /* Store the function id */
    gTP_CallbackInfo.funcId = gTP_SetDevConfigId_c;
                    
    /* Clear the data table */
    FLib_MemSet(maDataTable, 0, gTP_DataTableMaxSize_c);
       
    /* Prepare the function to be available */ 
    dummy = 0;               
    if(TRUE == mDevConfig.mFuncIntEn.mGpioEn)
    {
      dummy |= gTP_GpioIntMask_c;    
    } 

    if(TRUE == mDevConfig.mFuncIntEn.mDevStatusEn)
    {
      dummy |= gTP_StatusIntMask_c;       
    }
    #pragma MESSAGE DISABLE C5917               
    if(TRUE == mDevConfig.mFuncIntEn.mSelfTestEn)
    {          
      dummy |= gTP_BistIntMask_c;                        
    }
         
    if(TRUE == mDevConfig.mFuncIntEn.mFlashEn)
    {
      dummy |= gTP_FlashIntMask_c;       
    }                      
        
    if(TRUE == mDevConfig.mFuncIntEn.mSensorEn)
    {
      dummy |= gTP_Abs0IntMask_c;                  
    }
      
    /* Update the maDataTable[1] */
    maDataTable[1] = dummy;
             
    /* Prepare the gestures available */
    dummy = 0;
    /* Single tap */    
    if(TRUE == mDevConfig.mGestIntEn.mSingleTapIntEn)
    {
      if(TRUE == gTP_CallbackInfo.devInfo.mGestList.mHasSingleTap)
      {
        /* Add the 'single tap' to the gesture list */         
        dummy |= gTP_SingleTapMask_c;        
      }
      else
      {
        retStatus = TP_ERR_GESTURE_NOT_SUPPORTED;
        nextStep = FALSE;        
      }
    }     
    
    /* Double tap */
    if(TRUE == nextStep)
    {       
      if(TRUE == mDevConfig.mGestIntEn.mDoubleTapIntEn)
      {
        if(TRUE == gTP_CallbackInfo.devInfo.mGestList.mHasDoubleTap)
        {
          /* Add the 'double tap' to the gesture list */            
          dummy |= gTP_DoubleTapMask_c;               
        }
        else
        {
          retStatus = TP_ERR_GESTURE_NOT_SUPPORTED;
          nextStep = FALSE;          
        }
      }                  
    }    
         
    /* Tap and hold */
    if(TRUE == nextStep)
    {         
      if(TRUE == mDevConfig.mGestIntEn.mTapAndHoldIntEn)
      {
        if(TRUE == gTP_CallbackInfo.devInfo.mGestList.mHasTapAndHold)
        {
          /* Add the 'tap and hold' to the gesture list */            
          dummy |= gTP_TapAndHoldMask_c;           
        }
        else
        {
          retStatus = TP_ERR_GESTURE_NOT_SUPPORTED;
          nextStep = FALSE;           
        }
      }                            
    }        
          
    /* Flick */
    if(TRUE == nextStep)
    {
      if(TRUE == mDevConfig.mGestIntEn.mFlickIntEn)
      {
        if(TRUE == gTP_CallbackInfo.devInfo.mGestList.mHasFlick)
        {
          /* Add the 'flick' to the gesture list */            
          dummy |= gTP_FlickMask_c;              
        }
        else
        {
          retStatus = TP_ERR_GESTURE_NOT_SUPPORTED;
          nextStep = FALSE;         
        }
      }      
    }
              
    /* Press */
    if(TRUE == nextStep)
    {
      if(TRUE == mDevConfig.mGestIntEn.mPressIntEn)
      {
        if(TRUE == gTP_CallbackInfo.devInfo.mGestList.mHasPress)
        {
          /* Add the 'press' to the gesture list */               
          dummy |= gTP_PressMask_c;          
        }
        else
        {
          retStatus = TP_ERR_GESTURE_NOT_SUPPORTED;
          nextStep = FALSE;          
        }
      }            
    }
    
    /* Pinch */
    if(TRUE == nextStep)
    {
      if(TRUE == mDevConfig.mGestIntEn.mPinchIntEn)
      {
        if(TRUE == gTP_CallbackInfo.devInfo.mGestList.mHasPinch)
        {
          /* Add the 'pinch' to the gesture list */           
          dummy |= gTP_PinchMask_c;            
        }
        else
        {
          retStatus = TP_ERR_GESTURE_NOT_SUPPORTED;
          nextStep = FALSE;          
        }
      }      
    }       
    
    /* Store the configuration to the data table entry 2 */
    maDataTable[2] = dummy; 
    
    /* Prepare for the next config update */
    dummy = 0;    
    /* Palm */
    if(TRUE == nextStep)
    {
      if(TRUE == mDevConfig.mGestIntEn.mPalmIntEn)
      {
        if(TRUE == gTP_CallbackInfo.devInfo.mGestList.mHasPalm)
        {
          /* Add the 'palm' to the gesture list */            
          dummy |= gTP_PalmMask_c;             
        }
        else
        {
          retStatus = TP_ERR_GESTURE_NOT_SUPPORTED;
          nextStep = FALSE;           
        }
      }               
    }
  
    /* Rotate */
    if(TRUE == nextStep)
    {
      if(TRUE == mDevConfig.mGestIntEn.mRotateIntEn)
      {
        if(TRUE == gTP_CallbackInfo.devInfo.mGestList.mHasRotate)
        {
          /* Add the 'rotate' to the gesture list */            
          dummy |= gTP_RotateMask_c;          
        }
        else
        {
          retStatus = TP_ERR_GESTURE_NOT_SUPPORTED;
          nextStep = FALSE;           
        }
      }                  
    }    
    
    /* Store the remaining of configuration to the data table */
    maDataTable[3] = dummy; 
    
    if(TRUE == nextStep)     
    {             
      /* Fill the table with appropriate operations */
      /* Store the number of operations */
      maTP_I2cTransfOperNo = gTP_SetDevConfig_TransfOperNo_c;
      
      *maDataTable = gTP_Configured_c;
      /* Prepare the first entry into the action table: write the Configure bit in the Device Control Register */
      pTransfOper->regAddr      = PDT_F01_RMI_CONTROL_BASE + mRMI_DeviceControl_Offset_c;  
      pTransfOper->pBuff        = maDataTable;
      pTransfOper->buffLength   = 1;
      pTransfOper->tpOperType   = mTP_OperType_Write_c;
             
      /* Prepare the second entry into the action table: write the list of function to be available */   
      (++pTransfOper)->regAddr  = PDT_F01_RMI_CONTROL_BASE + mRMI_InterruptEnable0_Offset_c; 
      pTransfOper->pBuff        = maDataTable + 1;
      pTransfOper->buffLength   = 1;
      pTransfOper->tpOperType   = mTP_OperType_Write_c;              
      
      /* Prepare the third entry into the action table: write the list of gestures to be enabled */
      (++pTransfOper)->regAddr  = PDT_F11_2D_CONTROL_BASE + m2D_GestureEnable1_Offset_c;
      pTransfOper->pBuff        = maDataTable + 2;        
      if(!(*(maDataTable + 3)))           
      {
        pTransfOper->buffLength   = 1;        
      }
      else
      {
        pTransfOper->buffLength   = 2;             
      }
      pTransfOper->tpOperType   = mTP_OperType_Write_c;
                                         
      /* Prepare the fourth entry into the action table: read the Device status in order to check if configuration has done */
      (++pTransfOper)->regAddr  = PDT_F01_RMI_DATA_BASE + mRMI_DeviceStatus_Offset_c; 
      pTransfOper->pBuff        = maDataTable + 4;
      pTransfOper->buffLength   = 1;
      pTransfOper->tpOperType   = mTP_OperType_Read_c;             
      
      /* Prepare the fifth entry into the action table: read the Interrupt enable register to check if configuration has done */
      (++pTransfOper)->regAddr  = PDT_F01_RMI_CONTROL_BASE + mRMI_InterruptEnable0_Offset_c;
      pTransfOper->pBuff        = maDataTable + 5;  
      pTransfOper->buffLength   = 1;
      pTransfOper->tpOperType   = mTP_OperType_Read_c;
      
      /* Prepare the fifth entry into the action table:  read the Gesture Interrupt enable register to check if configuration has done */
      (++pTransfOper)->regAddr  = PDT_F11_2D_CONTROL_BASE + m2D_GestureEnable1_Offset_c;
      pTransfOper->pBuff        = maDataTable + 6;  
      pTransfOper->buffLength   = 1;
      pTransfOper->tpOperType   = mTP_OperType_Read_c;      
              
      #if(DEBUG_MODE_ENABLE == 1) 
        /* Clear the rest of the entries in the action table */
        FLib_MemSet((maTP_I2cTransfOperTable + maTP_I2cTransfOperNo + 1), 0, (gTP_I2cTransfOperMaxSize_c - maTP_I2cTransfOperNo - 1) * sizeof(tpI2cTransfOper_t));                                                 
      #endif
      
      /* Prepare the first IIC Write operation */
      maTP_I2cOperCurrIndex = 0;
      
      pTransfOper = maTP_I2cTransfOperTable; 
      if(FALSE == TP_WriteBlock(pTransfOper->regAddr, pTransfOper->buffLength, pTransfOper->pBuff))    
      {
        retStatus = TP_ERR_IIC_COMM;     
      }    
    }
  }
   
  return retStatus; 
#else
  (void)  mDevConfig;
  return  TP_OK;
#endif  
}


/*****************************************************************************
* TP_SetCallback
* 
* Interface assumption:
*
* Return value: generic status code
*
* Provides a pointer to the callback function for any touchpad event
*****************************************************************************/
statusCode_t TP_SetCallback(tpCallback_t callback)
{  
#if(gTouchpadIncluded_d == 1)  
  uint8_t retStatus = TP_OK;

  if(NULL == callback)
  {
    retStatus = TP_ERR_INVALID_PARAM;
  }
  else
  {
    pfTpCallback = callback;    
  }
  
  return retStatus;
#else
  (void) callback;
  return TP_OK;     
#endif    
}


/****************************************************************************/
/* Place it in NON_BANKED memory */
#ifdef MEMORY_MODEL_BANKED
#pragma CODE_SEG __NEAR_SEG NON_BANKED
#else
#pragma CODE_SEG DEFAULT
#endif  /* MEMORY_MODEL_BANKED */
/* TP Interrupt Service Routine */

#if(gTP_Mode_c == gTP_PollingMode_c)
/* When touchpad is used in polling mode, TP_InterruptHandler
should be called periodically by the application and is not 
an interrupt handler */
void TP_InterruptHandler(void)  
#else
INTERRUPT_KEYWORD void TP_InterruptHandler(void)
#endif
{
#if(gTouchpadIncluded_d == 1)  
  #if(gTP_Mode_c == gTP_IrqIsrMode_c)
    /* Acknowledge the external IRQ interrupt event */
    m_IRQSC_c |= mIRQSC_IRQACK_c;
  #endif
  #if(gTP_Mode_c == gTP_KbiIsrMode_c)
    /* Acknowledge the KBI2P0 interrupt event */
    m_KBI2SC_c |= mKBI2SC_KBACK_c;
  #endif
  
  if(gTP_CallbackInfo.funcId)
  {
    if(gTP_CallbackInfo.funcId != gTP_DevRestartId_c) 
    {
      /* Store the function identifier */ 
      gTP_CallbackInfo.funcId = gTP_IsrId_c;                                
    }
    else
    {
      gTP_DevResetIdx++;
    }
    
    /* Send event to the TP_Task */
    TS_SendEvent(gTP_TaskId, gTP_Event_ISR_c);                            
  }
        
#endif    
}
#pragma CODE_SEG DEFAULT

#if(gTouchpadIncluded_d == 1) 
  /*****************************************************************************
  * TP_Task
  * 
  * Interface assumption:
  *
  * Return value: generic status code
  *
  * 
  *****************************************************************************/
  void TP_Task(event_t events)  
  { 
    volatile uint8_t iFingerNum;
    uint8_t operType; 
    tpI2cTransfOper_t* pTransfOper = maTP_I2cTransfOperTable;
                  
    if(events & gTP_Event_ISR_c)
    { /* Events from the TP_ISR */
      if(gTP_CallbackInfo.funcId == gTP_IsrId_c) 
      {                   
        maTP_I2cTransfOperNo = gTP_Isr_TransfOperNo_c;     
        
        /* Prepare the first entry in the table of actions: Device Status */
        pTransfOper->regAddr      = PDT_F01_RMI_DATA_BASE + mRMI_DeviceStatus_Offset_c;
        pTransfOper->pBuff        = maDataTable;
        pTransfOper->buffLength   = 1;
        pTransfOper->tpOperType   = mTP_OperType_Read_c;         
        
        /* Prepare the second entry in the table of actions: Interrupt status: this should deassert the ATTN line */
        (++pTransfOper)->regAddr  = PDT_F01_RMI_DATA_BASE + mRMI_InterruptStatus_Offset_c;
        pTransfOper->pBuff        = maDataTable + 1;
        pTransfOper->buffLength   = 1;
        pTransfOper->tpOperType   = mTP_OperType_Read_c;        
        
        /* Prepare the third entry into the table of operation: finger info and gestures status */
        (++pTransfOper)->regAddr  = PDT_F11_2D_DATA_BASE + m2D_FingerState_Offset_c;
        pTransfOper->pBuff        = maDataTable + 2;
        pTransfOper->buffLength   = gBlockSize;
        pTransfOper->tpOperType   = mTP_OperType_Read_c;      

        #if(DEBUG_MODE_ENABLE == 1)
          /* Clear all entries in the action table before filling with necessary actions */
          FLib_MemSet((maTP_I2cTransfOperTable + maTP_I2cTransfOperNo + 1), 0, (gTP_I2cTransfOperMaxSize_c - maTP_I2cTransfOperNo - 1) * sizeof(tpI2cTransfOper_t));

          /* Keep clear the data table */
          FLib_MemSet(maDataTable, 0, gTP_DataTableMaxSize_c);     
        #endif      
        
        /* Prepare the first IIC read operation */
        maTP_I2cOperCurrIndex = 0;
        
        pTransfOper = maTP_I2cTransfOperTable;
        if(FALSE == TP_ReadBlock(pTransfOper->regAddr, pTransfOper->buffLength, pTransfOper->pBuff))    
        {
          /* Call the application callback with the error status of the initial function */
          gTP_CallbackInfo.status = FALSE;
          
          pfTpCallback(gTP_CallbackInfo);  
        }           
      }
      else
      {
        if((gTP_CallbackInfo.funcId == gTP_DevRestartId_c) && (gTP_DevResetIdx == 2))      
        {
          /* Send event to the TP_Task */
          TS_SendEvent(gTP_TaskId, gTP_Event_WriteBlockSuccess_c);         
        }
      }
    }
    else if((events & gTP_Event_ReadBlockSuccess_c) || (events & gTP_Event_WriteBlockSuccess_c))
    {
      uint8_t functId = gTP_CallbackInfo.funcId; 
      
      /* The following operations are valid only for 'GetDevInfo()', 'GetGesturesConfig()', 'TP_ISR()', 'SetGesturesList()', 'TP_RestartDevice()' functions */
      if(functId && (functId <= gTP_DevRestartId_c))
      {
        /* Check if the operation is the last one in the list */
        if(++maTP_I2cOperCurrIndex <= maTP_I2cTransfOperNo)
        {
          /* For the device restart function if the reset didn't finished, then take no action */
          if((maTP_I2cOperCurrIndex == 1) && (functId == gTP_DevRestartId_c))
          {
            if(gTP_DevResetIdx < 2) 
            {
              /* Decrement the current transfer (previously incremented in the condition) because the action should not be taken, then leaves the function */
              maTP_I2cOperCurrIndex--;
              return; 
            }
          }
                  
          /* Check the type of the next transfer over the I2C: read or write */
          pTransfOper = maTP_I2cTransfOperTable + maTP_I2cOperCurrIndex;
          operType = pTransfOper->tpOperType;        
          
          if(mTP_OperType_Read_c == operType)
          {                 
            /* Prepare the next transfer over the I2C */
            if(FALSE == TP_ReadBlock(pTransfOper->regAddr, pTransfOper->buffLength, pTransfOper->pBuff))    
            {
              /* Call the application callback with the error status of the initial function */
              gTP_CallbackInfo.status = FALSE;
            
              pfTpCallback(gTP_CallbackInfo);      
            }               
          }
          else if(mTP_OperType_Write_c == operType)
          {                                                       
            /* Prepare the next transfer over the I2C */
            if(FALSE == TP_WriteBlock(pTransfOper->regAddr, pTransfOper->buffLength, pTransfOper->pBuff))    
            {
              /* Call the application callback with the error status of the initial function */        
              gTP_CallbackInfo.status = FALSE;
            
              pfTpCallback(gTP_CallbackInfo);     
            }          
          }               
        }
        else
        {     
          switch(functId)
          {
            case gTP_GetDevInfoId_c:
                    TP_SaveDevInfo();                      
                                              
                    /* Update the status of the callback */  
                    gTP_CallbackInfo.status = TRUE;                                                              
                    break;
              
            case gTP_GetDevConfigId_c:
                    TP_SaveDevConfig();
                                                         
                    /* Update the status of the callback */ 
                    gTP_CallbackInfo.status = TRUE;                                                                         
                    break;
            
            case gTP_SetDevConfigId_c:                              
                    /* Call the application callback with the success status of the initial function */                     
                    gTP_DevStatus = *(maDataTable + 4);
                    
                    /* Set the returned status of the function */
                    if((!(gTP_DevStatus & gTP_Unconfigured_c)) && (*(maDataTable + 1) == *(maDataTable + 5)) && (*(maDataTable + 2) == *(maDataTable + 6)))
                    {
                      gTP_CallbackInfo.status = TRUE; 
                    }
                    else
                    {
                      gTP_CallbackInfo.status = FALSE;
                    }
                                     
                    /* Updates the available function and gesture list */
                    TP_UpdateDevConfig(*(maDataTable + 5), *(maDataTable + 6)); 
                    break;
              
            case gTP_IsrId_c:
                    /* Get the Interrupt and Device Status registers information */                                     
                    gTP_DevStatus = *maDataTable;
                    gTP_IntStatus = *(maDataTable + 1);
                    
                    /* Check if interrupt has occured due to finger presence on the sensor surface */
                    if(gTP_IntStatus & gTP_Abs0IntMask_c)
                    {
                      /* Process information about fingers position */
                      for(iFingerNum = 0; iFingerNum < gTP_MaxNoOfFingers; iFingerNum++)
                      {                       
                        
                        if(TP_OK != TP_GetFingerInfo(iFingerNum))              
                        {
                          /* Call the application callback with the error status of the initial function */   
                          gTP_CallbackInfo.status = FALSE;
                        
                          pfTpCallback(gTP_CallbackInfo);
                          return;                                        
                        }                                           
                      }
                        
                      /* Process information about gesture status */
                      TP_GestureRecognition();                          
                                                                                         
                      /* Prepare the status of the callback */             
                      gTP_CallbackInfo.status = TRUE;
                    }
                    else
                    {
                      /* Prepare the status of the callback */
                      gTP_CallbackInfo.status = FALSE;                    
                    }                                                                                       
                    break;
            
            case gTP_DevRestartId_c:                                    
                    /* Update the status of the callback */                
                    gTP_CallbackInfo.status = TRUE;                                                                                                               
                    break;
          }
          
          /* Call the application callback with the parameters updated */
          pfTpCallback(gTP_CallbackInfo);
        }      
      }   
    }
    else if(events & (gTP_Event_ReadBlockFail_c | gTP_Event_WriteBlockFail_c))
    {
      uint8_t functId = gTP_CallbackInfo.funcId;
      
      /* The following operations are valid only for 'GetDeviceInfo()', 'GetGesturesConfig()', 'TP_ISR()', 'SetGesturesList()', 'TP_RestartDevice()' functions */
      if(functId && (functId <= gTP_DevRestartId_c))
      {
        /* Call the application callback with the error status of the initial function */   
        gTP_CallbackInfo.status = FALSE;
            
        pfTpCallback(gTP_CallbackInfo);       
      }                        
    }         
  }


  /*****************************************************************************
  * TP_SaveDevInfo
  * 
  * Interface assumption:
  *
  * Return value: None
  *
  * 
  *****************************************************************************/
  static void TP_SaveDevInfo(void)
  {    
    uint8_t dummy;
         
    /* Fill the appropriate structure of type 'devInfo' with the default values */
    FLib_MemSet(&(gTP_CallbackInfo.devInfo), 0, sizeof(devInfo_t));
         
    /* Save the number of finger and other parameters (abs, rel values, gesture and sensitivity) */
    gTP_CallbackInfo.devInfo.mNoOfFingers  = (*maDataTable & gTP_NoOfFingersMask_c) + 1;  
    
    dummy = *maDataTable;  
    if(dummy & gTP_AbsModeMask_c)
    {
      gTP_CallbackInfo.devInfo.mHasAbs = TRUE;     
    }  
    if(dummy & gTP_RelModeMask_c)
    {
      gTP_CallbackInfo.devInfo.mHasRel = TRUE;
    }
    if(dummy & gTP_GestureMask_c)
    {
      gTP_CallbackInfo.devInfo.mHasGesture = TRUE;
    }
    if(dummy & gTP_SensAdjMask_c)
    {
      gTP_CallbackInfo.devInfo.mHasSensAdj = TRUE;
    }
             
    /* Save the gesture suite supported by the sensor */
    if(TRUE == gTP_CallbackInfo.devInfo.mHasGesture)
    {
      dummy = *(maDataTable + 1);
      if(dummy & gTP_SingleTapMask_c)
      {
        gTP_CallbackInfo.devInfo.mGestList.mHasSingleTap = TRUE;
      }
      if(dummy & gTP_DoubleTapMask_c)
      {
        gTP_CallbackInfo.devInfo.mGestList.mHasDoubleTap = TRUE; 
      }
      if(dummy & gTP_TapAndHoldMask_c)
      {
        gTP_CallbackInfo.devInfo.mGestList.mHasTapAndHold  = TRUE;
      }
      if(dummy & gTP_FlickMask_c)
      {
        gTP_CallbackInfo.devInfo.mGestList.mHasFlick = TRUE;
      }
      if(dummy & gTP_PressMask_c)
      {
        gTP_CallbackInfo.devInfo.mGestList.mHasPress = TRUE; 
      }
      if(dummy & gTP_PinchMask_c)
      {
        gTP_CallbackInfo.devInfo.mGestList.mHasPinch = TRUE; 
      }
      
      dummy = *(maDataTable + 2);
      if(dummy & gTP_RotateMask_c)
      {
        gTP_CallbackInfo.devInfo.mGestList.mHasRotate = TRUE; 
      }
      if(dummy & gTP_PalmMask_c)
      {
        gTP_CallbackInfo.devInfo.mGestList.mHasPalm = TRUE; 
      }                              
    }
    
    /* Save the reported mode */
    gTP_CallbackInfo.devInfo.mReportedMode  = *(maDataTable + 3)  & gTP_ReportingModeMask_c;
          
    /* Save the product id string */
    FLib_MemCpy(gTP_CallbackInfo.devInfo.mProdId, (maDataTable + 4), gTP_ProdIdLengthMax_c);
         
    /* Save the maximum X and Y positions */
    gTP_CallbackInfo.devInfo.mMaxXPos = (uint16_t)(*(maDataTable + gTP_ProdIdLengthMax_c + 4) + (*(maDataTable + gTP_ProdIdLengthMax_c + 5) << 8)); 
    gTP_CallbackInfo.devInfo.mMaxYPos = (uint16_t)(*(maDataTable + gTP_ProdIdLengthMax_c + 6) + (*(maDataTable + gTP_ProdIdLengthMax_c + 7) << 8)); 
          
    /* Update the 'gBlockSize' global variable further used in the TP_Task() function */
    /* Compute the block size to be read */
    if(TRUE == gTP_CallbackInfo.devInfo.mHasAbs)
    {
      gBlockSize += gTP_FingerAbsValSize_c;        
    }
    if(TRUE == gTP_CallbackInfo.devInfo.mHasRel)
    {
      gBlockSize += gTP_FingerRelValSize_c;        
    }
    /* Increase the total block size with size for each finger info block */
    gBlockSize *= gTP_CallbackInfo.devInfo.mNoOfFingers;
    
    /* Add to the total block size, the size of gesture status list and gestures parameters */
    gBlockSize += (gTP_GestureStatusSize_c + gTP_GestureInfoSize_c + TP_FingerStateSize(gTP_CallbackInfo.devInfo.mNoOfFingers));              
  }


  /*****************************************************************************
  * TP_SaveDevConfig
  * 
  * Interface assumption:
  *
  * Return value: None
  *
  * 
  *****************************************************************************/
  static void TP_SaveDevConfig(void)   
  {    
    uint8_t dummy;
    uint8_t* pDataTable = maDataTable;
    
           
    /* Fill the appropriate structure of type 'devConfig' with default values */
    FLib_MemSet(&(gTP_CallbackInfo.devConfig), 0, sizeof(devConfig_t));
    
    if((TRUE == gTP_CallbackInfo.devInfo.mGestList.mHasSingleTap) || 
       (TRUE == gTP_CallbackInfo.devInfo.mGestList.mHasDoubleTap) || 
       (TRUE == gTP_CallbackInfo.devInfo.mGestList.mHasTapAndHold))
    {
        gTP_CallbackInfo.devConfig.mGestConfigParam.mTapConfigParam.mMaximumTapTime      = *(pDataTable++);
        gTP_CallbackInfo.devConfig.mGestConfigParam.mTapConfigParam.mMaximumTapDistance  = *(pDataTable++);    
    }
     
    if(TRUE == gTP_CallbackInfo.devInfo.mGestList.mHasFlick)
    {
        gTP_CallbackInfo.devConfig.mGestConfigParam.mFlickConfigParam.mMinimumFlickDistance  = *(pDataTable++);
        gTP_CallbackInfo.devConfig.mGestConfigParam.mFlickConfigParam.mMinimumFlickSpeed     = *(pDataTable++); 
    }
    
    if(TRUE == gTP_CallbackInfo.devInfo.mGestList.mHasPress)
    {
        gTP_CallbackInfo.devConfig.mGestConfigParam.mPressConfigParam = *(pDataTable++);
    }
    
    if(TRUE == gTP_CallbackInfo.devInfo.mGestList.mHasPalm)
    {
      gTP_CallbackInfo.devConfig.mGestConfigParam.mPalmConfigParam = *(pDataTable++);
    }

    /* Save the information about the function and gestures enabled */
    dummy = *(maDataTable + 6);
    
    if(dummy & gTP_FlashIntMask_c)
    {
      gTP_CallbackInfo.devConfig.mFuncIntEn.mFlashEn = TRUE;    
    }
    if(dummy & gTP_BistIntMask_c)
    {
      gTP_CallbackInfo.devConfig.mFuncIntEn.mSelfTestEn = TRUE;
    }
    if(dummy & gTP_StatusIntMask_c)
    {
      gTP_CallbackInfo.devConfig.mFuncIntEn.mDevStatusEn = TRUE;
    }
    if(dummy & gTP_Abs0IntMask_c)
    {
      gTP_CallbackInfo.devConfig.mFuncIntEn.mSensorEn = TRUE;
    }  
    if(dummy & gTP_GpioIntMask_c)
    {
      gTP_CallbackInfo.devConfig.mFuncIntEn.mGpioEn = TRUE;
    }    
      
    /* Gestures enabled */   
    dummy = *(maDataTable + 7);
     
    if(dummy & gTP_PinchMask_c)
    {
      gTP_CallbackInfo.devConfig.mGestIntEn.mPinchIntEn = TRUE;    
    }
    if(dummy & gTP_PressMask_c)
    {
      gTP_CallbackInfo.devConfig.mGestIntEn.mPressIntEn = TRUE;    
    }
    if(dummy & gTP_FlickMask_c)
    {
      gTP_CallbackInfo.devConfig.mGestIntEn.mFlickIntEn = TRUE;     
    }
    if(dummy & gTP_DoubleTapMask_c)
    {
      gTP_CallbackInfo.devConfig.mGestIntEn.mDoubleTapIntEn = TRUE;     
    }
    if(dummy & gTP_TapAndHoldMask_c)
    {
      gTP_CallbackInfo.devConfig.mGestIntEn.mTapAndHoldIntEn = TRUE;     
    }
    if(dummy & gTP_SingleTapMask_c)
    {
      gTP_CallbackInfo.devConfig.mGestIntEn.mSingleTapIntEn = TRUE;     
    }                      
  }


  /*****************************************************************************
  * TP_UpdateDevConfig
  * 
  * Interface assumption:
  *
  * Return value: generic status code
  *
  * 
  *****************************************************************************/
  static void TP_UpdateDevConfig(uint8_t mIntEn0Reg, uint8_t mGestEn1Reg)   
  {
    uint8_t dummy;
    
    /* Fill the appropriate structures of types 'funcIntEn_t' and 'gestIntEn_t' with default values */
    FLib_MemSet(&(gTP_CallbackInfo.devConfig), 0, sizeof(funcIntEn_t) + sizeof(gestIntEn_t));
      
    /* Update the function enable interrupt list */
    dummy = mIntEn0Reg;    
    if(dummy & gTP_FlashIntMask_c)
    {
      gTP_CallbackInfo.devConfig.mFuncIntEn.mFlashEn = TRUE;   
    }
    if(dummy & gTP_BistIntMask_c)
    {
      gTP_CallbackInfo.devConfig.mFuncIntEn.mSelfTestEn = TRUE;       
    }
    if(dummy & gTP_StatusIntMask_c)
    {
      gTP_CallbackInfo.devConfig.mFuncIntEn.mDevStatusEn = TRUE;       
    }
    if(dummy & gTP_Abs0IntMask_c)
    {
      gTP_CallbackInfo.devConfig.mFuncIntEn.mSensorEn = TRUE;       
    }
    if(dummy & gTP_GpioIntMask_c)
    {
      gTP_CallbackInfo.devConfig.mFuncIntEn.mGpioEn = TRUE;      
    } 
    
    /* Update the gesture enable interrupt list */
    dummy = mGestEn1Reg; 
    if(dummy & gTP_PinchMask_c)
    {
      gTP_CallbackInfo.devConfig.mGestIntEn.mPinchIntEn = TRUE;    
    }
    if(dummy & gTP_PressMask_c)
    {
      gTP_CallbackInfo.devConfig.mGestIntEn.mPressIntEn = TRUE;        
    }
    if(dummy & gTP_FlickMask_c)
    {
      gTP_CallbackInfo.devConfig.mGestIntEn.mFlickIntEn = TRUE;        
    }
    if(dummy & gTP_DoubleTapMask_c)
    {
      gTP_CallbackInfo.devConfig.mGestIntEn.mDoubleTapIntEn = TRUE;        
    }
    if(dummy & gTP_TapAndHoldMask_c)
    {
      gTP_CallbackInfo.devConfig.mGestIntEn.mTapAndHoldIntEn = TRUE;        
    } 
    if(dummy & gTP_SingleTapMask_c)
    {
      gTP_CallbackInfo.devConfig.mGestIntEn.mSingleTapIntEn = TRUE;        
    }                    
  }


  /*****************************************************************************
  * TP_GetFingerInfo
  * 
  * Interface assumption:
  *
  * Return value: generic status code
  *
  * 
  *****************************************************************************/
  static statusCode_t TP_GetFingerInfo(uint8_t mFingerNum)
  {  
    statusCode_t  retStatus = TP_OK;
    uint8_t       mFingerShift, mFingerStateSize;
    uint8_t       maDataTableIndex;
    uint8_t*      pDataTable; 
    fingerInfo_t* pFingerInfo = &gTP_CallbackInfo.fingerInfo[mFingerNum];
    
    /* Input parameters checking */  
    if(mFingerNum > gTP_CallbackInfo.devInfo.mNoOfFingers)
    {
      retStatus = TP_ERR_INVALID_PARAM;   
    }
    else
    {
      mFingerStateSize =(uint8_t) TP_FingerStateSize(gTP_CallbackInfo.devInfo.mNoOfFingers);
      maDataTableIndex = mFingerStateSize - 1;
      mFingerShift = (uint8_t)((2*mFingerNum) % 8);    
         
      /* Get the status finger */    
      pFingerInfo->mFingerStatus = (*(maDataTable + maDataTableIndex + 2) >> mFingerShift) & gTP_FingerStateMask_c; 
      
      if(TRUE == gTP_CallbackInfo.devInfo.mHasAbs)
      {
        if(TRUE == gTP_CallbackInfo.devInfo.mHasRel)
        {
          /* Compute the current index in the data table where info about finger are */
          maDataTableIndex = (mFingerNum * gTP_MaxFingerInfoSize_c) + mFingerStateSize;
          /* First two table entries are allocated for Interrupt and Device Status Registers so the index should be incremented */
          maDataTableIndex += 2;         
          
          /* Update the pointer to the data table */
          pDataTable = maDataTable + maDataTableIndex + gTP_FingerAbsValSize_c;
          
          /* Get the relative values */
          pFingerInfo->mDeltaX = *(pDataTable);
          pFingerInfo->mDeltaY = *(pDataTable + 1);
        }
        else
        {
          /* Compute the current index in the data table where info about finger are */
          maDataTableIndex = (mFingerNum * gTP_FingerAbsValSize_c) + mFingerStateSize;         
          /* First two table entries are allocated for Interrupt and Device Status Registers so the index should be incremented */
          maDataTableIndex += 2; 
          
          /* Put the relative values to zero */
          pFingerInfo->mDeltaX = 0;        
          pFingerInfo->mDeltaY = 0;          
        }
        
        /* Update the pointer to the data table */
        pDataTable = maDataTable + maDataTableIndex;
        
        /* Get the absolute values */
        /* Get the X, Y position */
        pFingerInfo->mPosX = ((uint16_t)(*(pDataTable) << 4) + (*(pDataTable + 2) & gTP_LowNibbleMask_c));
        pFingerInfo->mPosY = ((uint16_t)(*(pDataTable + 1) << 4) + ((*(pDataTable + 2) & gTP_HighNibbleMask_c) >> 4));          
                       
        /* Get the finger width for X and Y */
        pFingerInfo->mWidthX = *(pDataTable + 3) & gTP_LowNibbleMask_c;
        pFingerInfo->mWidthY = *(pDataTable + 3) >> 4;       
           
        /* Get the finger strength */
        pFingerInfo->mFingerStrength = *(pDataTable + 4);           
      }
      else
      {
        maDataTableIndex = (mFingerNum * gTP_FingerRelValSize_c) + mFingerStateSize;          
        /* First two table entries are allocated for Interrupt and Device Status Registers so the index should be incremented */
        maDataTableIndex += 2; 
        
        /* Put absolute values to zero */ 
        pFingerInfo->mPosX = 0;
        pFingerInfo->mPosY = 0;
        pFingerInfo->mWidthX = 0;
        pFingerInfo->mFingerStrength = 0;       
              
        /* Get the relative values */ 
        pFingerInfo->mDeltaX = *(maDataTable + maDataTableIndex);
        pFingerInfo->mDeltaY = *(maDataTable + maDataTableIndex + 1);             
      }  
    }
                           
    return retStatus;    
  }

  /*****************************************************************************
  * TP_GestureRecognition
  * 
  * Interface assumption:
  *
  * Return value: generic status code
  *
  * 
  *****************************************************************************/
  static void TP_GestureRecognition(void)
  {    
    uint8_t maDataTableIndex = 0;
    uint8_t dummy;
    uint8_t *pDataTable;
    
    /* Fill the appropriate structures of type 'gestStatusList_t' and 'gestInfo_t' with default values */
    FLib_MemSet(&(gTP_CallbackInfo.gestStatusList), 0, sizeof(gestStatusList_t) + sizeof(gestInfo_t));
      
    /* Compute the current index in the data table where info about gesture are */
    if(TRUE == gTP_CallbackInfo.devInfo.mHasAbs)
    {
      maDataTableIndex = gTP_FingerAbsValSize_c;
    }
    if(TRUE == gTP_CallbackInfo.devInfo.mHasRel)
    {
      maDataTableIndex += gTP_FingerRelValSize_c; 
    }
    /* Update the position with the number of fingers */
    maDataTableIndex *= gTP_CallbackInfo.devInfo.mNoOfFingers;
    maDataTableIndex += TP_FingerStateSize(gTP_CallbackInfo.devInfo.mNoOfFingers); 
    /* First two table entries are allocated for Interrupt and Device Status Registers so the index should be incremented */
    maDataTableIndex += 2;
        
    /* Update the pointer to the data table then the gesture status list */
    pDataTable = maDataTable + maDataTableIndex;   
    dummy = *pDataTable; 
    /* Update again the pointer to the data table in orde to point to the gestures parameters */
    pDataTable += 2;
    
    if(dummy & gTP_PinchMask_c)
    {
      gTP_CallbackInfo.gestStatusList.mPinch      = TRUE;        
      gTP_CallbackInfo.gestInfo.mPinchGestureInfo = *(pDataTable);     
    }
    if(dummy & gTP_PressMask_c)
    {
      gTP_CallbackInfo.gestStatusList.mPress = TRUE;     
    }
    if(dummy & gTP_FlickMask_c)
    { 
      gTP_CallbackInfo.gestStatusList.mFlick = TRUE;
      gTP_CallbackInfo.gestInfo.mFlickGestureInfo.mFlickDistanceX = *(pDataTable); 
      gTP_CallbackInfo.gestInfo.mFlickGestureInfo.mFlickDistanceY = *(pDataTable + 1); 
      gTP_CallbackInfo.gestInfo.mFlickGestureInfo.mFlickTime      = *(pDataTable + 2);                
    }
    if(dummy & gTP_SingleTapMask_c)
    {
      gTP_CallbackInfo.gestStatusList.mSingleTap  = TRUE;     
    }
    if(dummy & gTP_DoubleTapMask_c)
    {
      gTP_CallbackInfo.gestStatusList.mDoubleTap  = TRUE;     
    }
    if(dummy & gTP_TapAndHoldMask_c)
    {
      gTP_CallbackInfo.gestStatusList.mTapAndHold = TRUE;     
    }
    
    /* Get palm and rotate status */
    dummy = *(maDataTable + maDataTableIndex + 1); 
    if(dummy & gTP_RotateMask_c)
    {
      gTP_CallbackInfo.gestStatusList.mRotate  = TRUE; 
      gTP_CallbackInfo.gestInfo.mRotateGestureInfo.mRotateMotion    = *(pDataTable + 1);
      gTP_CallbackInfo.gestInfo.mRotateGestureInfo.mFingersDistance = *(pDataTable + 2);    
    }
    if(dummy & gTP_PalmMask_c)
    {
      gTP_CallbackInfo.gestStatusList.mPalm = TRUE;        
    }     
  }
   

  /*****************************************************************************
  * TP_ReadBlock
  * 
  * Interface assumption:
  *
  * Return value: generic status code
  *
  * 
  *****************************************************************************/
  static bool_t TP_ReadBlock(uint8_t startAddr, uint8_t blockLength, uint8_t* pRxBuff)
  {     
    bool_t retStatus = FALSE;
         
    if(gRmiBlockAccess.accessLock == FALSE)
    {
      /* Prepare the structure to be used by the RmiWriteAddrCallback */
      gRmiBlockAccess.rmiBlockAccess.regAddr      = startAddr;
      gRmiBlockAccess.rmiBlockAccess.buffLength   = blockLength;
      gRmiBlockAccess.rmiBlockAccess.tpOperType   = mTP_OperType_Read_c;
      gRmiBlockAccess.rmiBlockAccess.pBuff        = pRxBuff;
      gRmiBlockAccess.accessLock                  = TRUE; 
      
      /* Transmit the start address of the RMI registers block */
      retStatus = IIC_Transmit_Master(&gRmiBlockAccess.rmiBlockAccess.regAddr, 
                                      gTP_RmiAddrByteSize_c, 
                                      gTP_SlaveAddress_c, 
                                      RmiWriteAddrCallback); 
      if(retStatus == FALSE)
      {
        gRmiBlockAccess.accessLock = FALSE; 
      }
    }
         
    return retStatus;  
  }

  /*****************************************************************************
  * TP_WriteBlock
  * 
  * Interface assumption:
  *
  * Return value: generic status code
  *
  * 
  *****************************************************************************/
  static bool_t TP_WriteBlock(uint8_t startAddr, uint8_t blockLength, uint8_t* pTxBuff)
  {    
    rmiBlockAccess_t* pRmiBlockAccess = &gRmiBlockAccess;
    bool_t            retStatus       = FALSE;
       
    if((pRmiBlockAccess->accessLock == FALSE) && (blockLength < gWriteBlockMaxSize_c))
    {  
      /* Prepare the structure to be used by the RmiCompleteAccessCallback */
      pRmiBlockAccess->rmiBlockAccess.regAddr      = startAddr; 
      pRmiBlockAccess->rmiBlockAccess.buffLength   = blockLength;
      pRmiBlockAccess->rmiBlockAccess.tpOperType   = mTP_OperType_Write_c;
      pRmiBlockAccess->rmiBlockAccess.pBuff        = pTxBuff;
      pRmiBlockAccess->accessLock                  = TRUE;   
                  
      *gWriteBlock = pRmiBlockAccess->rmiBlockAccess.regAddr; 
      FLib_MemCpy((gWriteBlock + 1), pTxBuff, blockLength);
                                                                    
      /* Start transmission over the IIC */
      retStatus = IIC_Transmit_Master(gWriteBlock, 
                                      blockLength + 1, 
                                      gTP_SlaveAddress_c, 
                                      RmiCompleteAccessCallback); 
      
      if(retStatus == FALSE)
      {
        pRmiBlockAccess->accessLock = FALSE; 
      }                                                                       
    }
    
    return retStatus;  
  } 
    

  /*****************************************************************************
  * RmiWriteAddrCallback
  * 
  * Interface assumption:
  *
  * Return value: none
  *
  * 
  *****************************************************************************/
  static void RmiWriteAddrCallback(bool_t status)
  {         
    if(gRmiBlockAccess.rmiBlockAccess.tpOperType  == mTP_OperType_Read_c)
    {
      if(FALSE == status)
      {
        /* Release the access to the 'gRmiBlockAccess' variable */
        gRmiBlockAccess.accessLock = FALSE;
              
        /* Send event to the TP_Task */
        TS_SendEvent(gTP_TaskId, gTP_Event_ReadBlockFail_c);  
      }
      else
      {
        if(FALSE == IIC_Receive_Master((uint8_t*)gRmiBlockAccess.rmiBlockAccess.pBuff, 
                                       gRmiBlockAccess.rmiBlockAccess.buffLength, 
                                       gTP_SlaveAddress_c, 
                                       RmiCompleteAccessCallback))        
        {
          /* Release the access to the 'gRmiBlockAccess' variable */
          gRmiBlockAccess.accessLock = FALSE;
                     
          /* Send event to the TP_Task */
          TS_SendEvent(gTP_TaskId, gTP_Event_ReadBlockFail_c);    
        }
      }
    }
  }


  /*****************************************************************************
  * RmiCompleteAccessCallback
  * 
  * Interface assumption:
  *
  * Return value: none
  *
  * 
  *****************************************************************************/
  static void RmiCompleteAccessCallback(bool_t status)
  {       
    event_t dummyEvent;
        
    if(gRmiBlockAccess.rmiBlockAccess.tpOperType == mTP_OperType_Read_c)
    {
      dummyEvent = (FALSE == status) ?  gTP_Event_ReadBlockFail_c : gTP_Event_ReadBlockSuccess_c; 
    }
    else
    { /* Write RMI block */            
      if(gRmiBlockAccess.rmiBlockAccess.tpOperType == mTP_OperType_Write_c)
      {
        dummyEvent = (FALSE == status) ?  gTP_Event_WriteBlockFail_c : gTP_Event_WriteBlockSuccess_c;     
      }
    }
    
    /* Release the access to the 'gRmiBlockAccess' variable */
    gRmiBlockAccess.accessLock = FALSE;
    
    /* Send event to the TP_Task */
    TS_SendEvent(gTP_TaskId, dummyEvent);  
  }
#endif
 