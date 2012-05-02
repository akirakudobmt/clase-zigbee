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

#include "EmbeddedTypes.h"
#include "IrqControlLib.h"
#include "IoConfig.h"
#include "TS_Interface.h"
#include "Cmt_Interface.h"
#include "Cmt.h"

/*****************************************************************************
******************************************************************************
* Private macros
******************************************************************************
*****************************************************************************/


/*****************************************************************************
******************************************************************************
* Private type definitions  
******************************************************************************
*****************************************************************************/


/*****************************************************************************
******************************************************************************
* Private memory declaration
******************************************************************************
*****************************************************************************/  
#if(gCmtIncluded_d == 1)    

/* Defines the Tx status activity flag */
static volatile bool_t mCmt_TxActiveFlag;

/* Id for the CMT task */
static tsTaskID_t gCmtTaskId;

/* CMT Task event; only one event is needed */
#define mCMT_Event_c  (1<<0)

/* Indicates the number of bits left to be send */
static uint8_t mCmt_BitsLeft;

/* Byte to be send onto the IRO pin */
static uint8_t mCmt_DataByte;

/* Current bit to be processed */
static uint8_t mCmt_CurrentBit;
      
/* Variables defining the mark/space width for logic 0/1 */
static uint16_t mCmt_Log0MarkWidth;
static uint16_t mCmt_Log0SpaceWidth;
static uint16_t mCmt_Log1MarkWidth;
static uint16_t mCmt_Log1SpaceWidth;

/* Defines the CMT module mode of operation: either 'Time' or 'Baseband' */
static bool_t mCmt_TimeOperMode;

/* Defines the biwise shifting mode during bits transmisssion */
static bool_t mCmt_LsbFirst;

/* CMT callback function */
static void (*pfCmtTxCallBack)(void);

#endif

/*****************************************************************************
******************************************************************************
* Private function prototypes
******************************************************************************
*****************************************************************************/ 
#if(gCmtIncluded_d == 1)
  /* CMT Task; Process the CMT events in interrupt-driven context */
  static void CMT_Task(event_t events);
#endif             
              
/*****************************************************************************
******************************************************************************
* Private functions
******************************************************************************
*****************************************************************************/

/* Initialize the CMT module */
bool_t CMT_Initialize(void)
{   
#if(gCmtIncluded_d == 1)
  uint8_t mCmt_OutputPolarity = 0;
  bool_t retStatus = FALSE;
       
  /* Initialize the CMT task */
  gCmtTaskId = TS_CreateTask(gTsCmtTaskPriority_c, CMT_Task);
  
  if(gTsInvalidTaskID_c != gCmtTaskId)
  {
    /* Reloads CMT configuration registers to their reset state values together */
    /* with the clock input configuration */
    CMTOC   = mCMT_CMTOC_Reset_c | mCmtDividerDefault_c; 
    CMTMSC  = mCMT_CMTMSC_Reset_c | mCmtPrescaleDefault_c;
      
    /* Configure the carrier generation */
    CMTCGH1 = ComputeCarrierHigh(mCmtDefaultClockInput_c, gCmtDefaultCarrierFrequency_c, gCmtDefaultCarrierDutyCycle_c);
    CMTCGL1 = ComputeCarrierLow(mCmtDefaultClockInput_c, gCmtDefaultCarrierFrequency_c, gCmtDefaultCarrierDutyCycle_c); 
      
    /* Set mark/space period width for logic 0/1 bit with default values */
    /* Logic 0 */
    mCmt_Log0MarkWidth  = CmtModMarkTicksInMicroSeconds(gCmtDefaultLog0MarkInMicros_c, mCmtDefaultClockInput_c);
    mCmt_Log0SpaceWidth = CmtModSpaceTicksInMicroSeconds(gCmtDefaultLog0SpaceInMicros_c, mCmtDefaultClockInput_c);
    /* Logic 1 */
    mCmt_Log1MarkWidth  = CmtModMarkTicksInMicroSeconds(gCmtDefaultLog1MarkInMicros_c, mCmtDefaultClockInput_c);
    mCmt_Log1SpaceWidth = CmtModSpaceTicksInMicroSeconds(gCmtDefaultLog1SpaceInMicros_c, mCmtDefaultClockInput_c);
                  
    /* CMT Mode of operation: default */
    mCmt_TimeOperMode = gCmtTimeOperModeDefault_c;
      
    /* Bitwise shifting: default */
    mCmt_LsbFirst = gCmtLsbFirstDefault_c; 
      
    /* Deassert the Tx activity flag */
    mCmt_TxActiveFlag = FALSE;   
      
    /* Set the pointer callback to NULL */
    pfCmtTxCallBack = NULL; 
      
    #if(TRUE == gCmtOutputPolarityDefault_c)
      mCmt_OutputPolarity = mCMT_CMTOC_CMTPOL_c;
    #endif
    
    /* Enable the IRO output pin and set the polarity type */
    CMTOC |= (mCMT_CMTOC_IROPEN_c | mCmt_OutputPolarity); 
    
    /* Update the status to be return */
    retStatus = TRUE;     
  }

  return retStatus;
#else
  return FALSE;
#endif  
}

/*****************************************************************************/
cmtErr_t CMT_SetCarrierWaveform(uint8_t highCount, uint8_t lowCount)
{
#if(gCmtIncluded_d == 1)        
  cmtErr_t mStatusRet = gCmtErrNoErr_c;
    
  if((!highCount) || (!lowCount))
  {
    /*Invalid parameters: either 'highCount' or 'lowCount' was provided as 0 */
    mStatusRet = gCmtErrInvalidParameter_c; 
  }
  else
  {
    if(TRUE == mCmt_TxActiveFlag)
    {
      /* CMT module is busy */
      mStatusRet = gCmtErrBusy_c;   
    }
    else
    {
      /* Write CMT Carrier Generator Registers with appropriate values */
      CMTCGH1 = highCount;
      CMTCGL1 = lowCount;   
    }
  }

  /* Return the status code */
  return mStatusRet; 
#else
  (void) highCount;
  (void) lowCount;
  return gCmtErrNoErr_c;
#endif  
}

/*****************************************************************************/
void CMT_SetTxCallback(cmtCallback_t callback)
{  
#if(gCmtIncluded_d == 1)  
  /* CMT callback function will be "loaded" with provided argument pointer */
  pfCmtTxCallBack = callback; 
#else
  (void) callback;
#endif          
}

/*****************************************************************************/
cmtErr_t CMT_SetMarkSpaceLog0(uint16_t markPeriod, uint16_t spacePeriod)
{   
#if(gCmtIncluded_d == 1)  
  cmtErr_t mStatusRet = gCmtErrNoErr_c;
  
  /* Check input parameters */
  if((!markPeriod) && (!spacePeriod))
  {
    /* Invalid parameters: 'markPeriod' and 'spacePeriod' are both 0 */
    mStatusRet = gCmtErrInvalidParameter_c;
  }
  else
  {
    if(TRUE == mCmt_TxActiveFlag)
    {
      /* CMT is busy transmitting either a modulation cycle or a string of bits */
      mStatusRet = gCmtErrBusy_c;
    }
    else
    {
      /* Load the generic mark and space variables for logic 0 bit value */
      mCmt_Log0MarkWidth  = markPeriod;
      mCmt_Log0SpaceWidth = spacePeriod;
    }
  }
  
  /* Return the status code */
  return mStatusRet;
#else
  (void) markPeriod;
  (void) spacePeriod;
  return gCmtErrNoErr_c;
#endif
}

/*****************************************************************************/
cmtErr_t CMT_SetMarkSpaceLog1(uint16_t markPeriod, uint16_t spacePeriod)
{
#if(gCmtIncluded_d == 1)  
  cmtErr_t mStatusRet = gCmtErrNoErr_c;
  
  /* Check input parameters */
  if((!markPeriod) && (!spacePeriod))
  {
    /* Invalid parameters: 'markPeriod' and 'spacePeriod' are both 0 */
    mStatusRet = gCmtErrInvalidParameter_c;
  }
  else
  {
    if(TRUE == mCmt_TxActiveFlag)
    {
      /* CMT is busy transmitting either a modulation cycle or a string of bits */
      mStatusRet = gCmtErrBusy_c;
    }
    else
    {
      /* Load generic mark and space variables for logic 1 bit value */
      mCmt_Log1MarkWidth  = markPeriod;
      mCmt_Log1SpaceWidth = spacePeriod;   
    }
  }
  
  /* Return the status code */
  return mStatusRet;
#else
  (void) markPeriod;
  (void) spacePeriod;
  return gCmtErrNoErr_c;    
#endif
}

/*****************************************************************************/
cmtErr_t  CMT_TxBits(uint8_t data, uint8_t bitsCount)
{
#if(gCmtIncluded_d == 1)  
  cmtErr_t mStatusRet = gCmtErrNoErr_c;
  
  /* Check if parameters delivered are valid */
  if((bitsCount < 1) || (bitsCount > 8))
  {
    /* Invalid parameters: bitsCount is outside of interval [1..8] */
    mStatusRet = gCmtErrInvalidParameter_c;    
  }
  else
  {
    /* Check if another Tx operation is ongoing */
    if(TRUE == mCmt_TxActiveFlag)
    {
      /* CMT is busy by performing another transmission */
      mStatusRet = gCmtErrBusy_c;  
    }
    else
    {
      /* No other Tx operation ongoing */
      /* Select the CMT mode of operation */
      if(TRUE == mCmt_TimeOperMode)
      {
        /* Time mode */
        CMTMSC &= ~mCMT_CMTMSC_BASE_c;  
      }
      else
      {
        /* BaseBand mode */
        CMTMSC |= mCMT_CMTMSC_BASE_c; 
      }
             
      /* Stores the data in a module variable in order to be accessed also by CMT task */
      mCmt_DataByte = data;
      
      /* Depending on the bits shifting different bit is currently processing */
      if(TRUE == mCmt_LsbFirst)
      {
        /* LSB first */
        mCmt_CurrentBit = mCmt_DataByte & 0x01; 
      }
      else
      {
        /* MSB first */
        /* Aligns the data byte in order to have the MSB on the 8th byte position */
        mCmt_DataByte <<= (8 - bitsCount);
        /* Get the current bit position */
        mCmt_CurrentBit = mCmt_DataByte & 0x80;  
      }
      
      /* Determine the current bit value; depending on the logical bit value different mark/space group */ 
      /* is reloaded into modulation mark/space registers */
      if(!mCmt_CurrentBit) 
      {
        /* Bit value: Logic 0 */
        mCMT_Modulation_MarkRegs  = mCmt_Log0MarkWidth; 
        mCMT_Modulation_SpaceRegs = mCmt_Log0SpaceWidth;  
      }
      else
      {
        /* Bit value: Logic 1 */ 
        mCMT_Modulation_MarkRegs  = mCmt_Log1MarkWidth; 
        mCMT_Modulation_SpaceRegs = mCmt_Log1SpaceWidth;  
      }
      
      /* Configure the number of bits left to be send, as an indication for the CMT_ISR */
      mCmt_BitsLeft = bitsCount;
          
      /* Assert the Tx activity flag */
      mCmt_TxActiveFlag = TRUE;  
    
      /* Enable the CMT interrupt and the CMT module */
      CMTMSC |= (mCMT_CMTMSC_EOCIE_c | mCMT_CMTMSC_MCGEN_c);                    
    } 
  }
  
  /* Return the status code */
  return mStatusRet;
#else
  (void) data;
  (void) bitsCount;
  return gCmtErrNoErr_c;
#endif
}

/*****************************************************************************/
cmtErr_t CMT_TxModCycle(uint16_t markPeriod, uint16_t spacePeriod)
{
#if(gCmtIncluded_d == 1)  
  cmtErr_t mStatusRet = gCmtErrNoErr_c;
  
  /* Check if parameters delivered are valid */
  if((!markPeriod) && (!spacePeriod))
  {
    /* Invalid parameters: both 'markPeriod' and 'spacePeriod' provided are 0 */
    mStatusRet = gCmtErrInvalidParameter_c; 
  }
  else
  {       
    /* Check if another Tx operation is ongoing */
    if(TRUE == mCmt_TxActiveFlag)
    {      
      /* CMT is busy by performing another transmission */
      mStatusRet = gCmtErrBusy_c;
    }
    else
    {       
      /* No other Tx operation ongoing */
      /* Select the CMT mode of operation */
      if(TRUE == mCmt_TimeOperMode)
      {
        /* Time mode */
        CMTMSC &= ~mCMT_CMTMSC_BASE_c;
      }
      else
      {
        /* BaseBand mode */
        CMTMSC |= mCMT_CMTMSC_BASE_c;
      }
      
      /* Configures the registers for modulation cycle mark and space */
      /* mark: CMTCMD1: CMTCMD2  space: CMTCMD3: CMTCMD4   */   
            
      /* Note: the following two expressions are valid taking into account that the default */
      /*       endianism mode is 'BigEndian' */
      mCMT_Modulation_MarkRegs  = markPeriod;
      mCMT_Modulation_SpaceRegs = spacePeriod;
      
      /* Number of bits left to be send */
      mCmt_BitsLeft = 1;
                 
      /* Assert the Tx activity flag */
      mCmt_TxActiveFlag = TRUE;  
      
      /* Enable the CMT interrupt and the CMT module */
      CMTMSC |= (mCMT_CMTMSC_EOCIE_c | mCMT_CMTMSC_MCGEN_c);  
    }
  }
   
  /* Return the status code */
  return mStatusRet; 
#else
  (void) markPeriod; 
  (void) spacePeriod;
  return gCmtErrNoErr_c;
#endif
}

/*****************************************************************************/
void CMT_Abort(void)
{
#if(gCmtIncluded_d == 1)
  /* Check if there is any ongoing Tx activity */
  if(TRUE == mCmt_TxActiveFlag)
  {   
    /* Disable the module, the output pin and the CMT interrupt */
    CMTMSC &= ~(mCMT_CMTMSC_MCGEN_c | mCMT_CMTOC_IROPEN_c | mCMT_CMTMSC_EOCIE_c);
    
    /* Deassert the Tx activity flag */
    mCmt_TxActiveFlag = FALSE;
  }
#endif
}

/*****************************************************************************/
bool_t CMT_IsTxActive(void)
{
#if(gCmtIncluded_d == 1) 
  return mCmt_TxActiveFlag;  
#else
  return FALSE;
#endif
}

/****************************************************************************/
cmtErr_t CMT_SetModOperation(bool_t timeOperMode)
{
#if(gCmtIncluded_d == 1)  
  cmtErr_t mStatusRet = gCmtErrNoErr_c;
  
  /* Check if another Tx operation is ongoing */
  if(TRUE == mCmt_TxActiveFlag)
  {      
    /* CMT is busy by performing another transmission */
    mStatusRet = gCmtErrBusy_c;
  }
  else
  {       
    /* No other Tx operation ongoing */
    /* Set the CMT mode of operation */ 
    mCmt_TimeOperMode = (TRUE == timeOperMode) ? TRUE : FALSE;    
  }
  
  /* Return the status code */
  return mStatusRet; 
#else
  (void) timeOperMode;
  return gCmtErrNoErr_c;
#endif   
}

/*****************************************************************************/
cmtErr_t CMT_SetBitsShifting(bool_t bitsLsbShifting) 
{
#if(gCmtIncluded_d == 1)  
  cmtErr_t mStatusRet = gCmtErrNoErr_c;
  
  /* Check if another Tx operation is ongoing */
  if(TRUE == mCmt_TxActiveFlag)
  {      
    /* CMT is busy by performing another transmission */
    mStatusRet = gCmtErrBusy_c;
  }
  else
  {       
    /* No other Tx operation ongoing */
    /* Set the CMT bits shifting */ 
    mCmt_LsbFirst = (TRUE == bitsLsbShifting) ? TRUE : FALSE;    
  }
    
  /* Return the status code */
  return mStatusRet;   
#else
  (void) bitsLsbShifting;
  return gCmtErrNoErr_c;
#endif
} 


/*****************************************************************************/
/* Place it in NON_BANKED memory */
#ifdef MEMORY_MODEL_BANKED
#pragma CODE_SEG __NEAR_SEG NON_BANKED
#else
#pragma CODE_SEG DEFAULT
#endif /* MEMORY_MODEL_BANKED */ 
/* CMT Interrupt Service Routine */
INTERRUPT_KEYWORD void CMT_InterruptHandler(void)
{
#if(gCmtIncluded_d == 1)  
  volatile uint8_t dummyRead;
    
  /* Clear the EOCF bit by reading the CMTMSC and accessing the CMTCMD2 register */
  dummyRead = CMTMSC;
    
  /* Touch the CMTCMD2 register */
  dummyRead = CMTCMD2;
 
  /* Check the number of bits left for Tx */
  if((--mCmt_BitsLeft) != 0)
  {
    /* There still available bits for transmission */
    /* Depending on the bits shifting, different bit is loaded to be send */
    if(TRUE == mCmt_LsbFirst)
    {
      /* LSB first */
      /* Shift the data byte by one bit to the right */
      mCmt_DataByte >>= 1;
      /* Get the current bit value */
      mCmt_CurrentBit = (mCmt_DataByte & 0x01);   
    }
    else
    {
      /* MSB first */
      /* Shift the data byte by one bit to the left */ 
      mCmt_DataByte <<= 1;
      /* Get the current bit value */        
      mCmt_CurrentBit = (mCmt_DataByte & 0x80);    
    }
            
    /* Determine the current bit value; depending on the logical bit value different mark/space group */ 
    /* is reloaded into modulation mark/space registers */
    if(!mCmt_CurrentBit) 
    {
      /* Bit value: Logic 0 */
      mCMT_Modulation_MarkRegs  = mCmt_Log0MarkWidth; 
      mCMT_Modulation_SpaceRegs = mCmt_Log0SpaceWidth;  
    }
    else
    {
      /* Bit value: Logic 1 */ 
      mCMT_Modulation_MarkRegs  = mCmt_Log1MarkWidth; 
      mCMT_Modulation_SpaceRegs = mCmt_Log1SpaceWidth;  
    }         
  }
  else
  {
    /* The number of bits to be sent has been finished */              
    /* Disable the module and the CMT interrupt */
    CMTMSC &= ~(mCMT_CMTMSC_MCGEN_c | mCMT_CMTMSC_EOCIE_c);  
  
    /* Deasserts the Tx active flag */
    mCmt_TxActiveFlag = FALSE;       
              
    /* Send event to the task scheduler */
    TS_SendEvent(gCmtTaskId, mCMT_Event_c); 
  }
#endif
}
#pragma CODE_SEG DEFAULT

/*****************************************************************************/
#if(gCmtIncluded_d == 1)
  static void CMT_Task(event_t events)
  {
    /* Touch the 'events' argument to avoid compiler warnings */
    (void)events;    
      
    /* Call the function callback; if it is a valid pointer to it */    
    if(pfCmtTxCallBack)
    {
      pfCmtTxCallBack();
    }  
  }
#endif 