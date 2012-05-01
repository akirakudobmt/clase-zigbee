/******************************************************************************
* Source file for LCD driver
*
*
* Copyright (c) 2010, Freescale Inc. All right reserved.
*
* 
* No part of this document must be reproduced in any form - including copied,
* transcribed, printed or by any electronic means - without specific written 
* permission from Freescale Semiconductor.
*
******************************************************************************/

#include "EmbeddedTypes.h"
#include "IrqControlLib.h"
#include "FunctionLib.h"
#include "TS_Interface.h"
#include "SPI_Interface.h"
#include "Display_MC1323x_RCM.h"


#ifndef gMacStandAlone_d
#define gMacStandAlone_d 0
#endif

#if gMacStandAlone_d
  #include "Mac_Globals.h"
#endif

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
typedef enum
{
  mLcd_State_Idle_c,
  mLcd_State_Init_c,
  mLcd_State_Display_c,
  mLcd_State_Control_c,  
} lcdState_t;

typedef enum
{
  mLcd_LineProcess_OnGoing_c,
  mLcd_LineProcess_Finished_c
} lcdLineProcessStatus_t;

typedef struct lcdOperType_tag
{
  lcdState_t      mLcdState;
  uint8_t         mLcdCtrlCmd;
  void            (*pfLcdInitCallback)(bool_t status);
  void            (*pfLcdCtrlCallback)(bool_t status);
} lcdOperType_t;

typedef struct lcdBuffTableRef_tag
{  
  uint8_t         mLcdBuffCmd;
  uint8_t         lcdPos;  
  uint8_t         buffLen;  
  void            (*pfBuffCallback)(bool_t status);
} lcdBuffTableRef_t;

typedef struct lcdInstrCode_tag
{
  uint8_t         mLcdUpperNibble; 
  uint8_t         mLcdLowerNibble;
  index_t         lcdWriteCmdStep;
} lcdInstrCode_t;

/******************************************************************************
*******************************************************************************
* Public memory definitions  
*******************************************************************************
******************************************************************************/
#if(TRUE == gLCDSupported_d)
  /* Id for LCD Task */
  tsTaskID_t      gLcdTaskId;
#endif /* gLCDSupported_d */

/******************************************************************************
*******************************************************************************
* Private memory definitions
*******************************************************************************
******************************************************************************/
#if(TRUE == gLCDSupported_d)
  static lcdOperType_t      lcdOperType;  
  static lcdInstrCode_t     lcdInstrCode;
  static index_t            initStep;
  
  /* SPI configuration parameters */
  static spiConfig_t        mSpiConfig;
  
  /* Buffer table reference structure */
  static lcdBuffTableRef_t  mLcdBuffTableRef[gLCD_MaxLcdLines_c];   
  
  /* LCD line buffers */
  static uint8_t            maLcdBuffTable[gLCD_MaxLcdLines_c][gMAX_LCD_CHARS_c];

  /* List of initialization commands */
  const uint8_t initCmdList[gLCD_MaxInitSteps_c] = 
  {         
    gLCD_FunctionSetCtrl_c,
    gLCD_FunctionSetCtrl_c,
    gLCD_DisplayCtrl_c   | gLCD_DisplayOnBit_c,
    gLCD_ClearCtrl_c,
    gLCD_EntryModeCtrl_c | gLCD_CursorMoveRight_c,
    /* Second init step operations */
    gLCD_FunctionSetCtrl_c,
    gLCD_FunctionSetCtrl_c,
    gLCD_DisplayCtrl_c   | gLCD_DisplayOnBit_c,
    gLCD_ClearCtrl_c,
    gLCD_EntryModeCtrl_c | gLCD_CursorMoveRight_c    
  };
  /* List of waiting time between initialization's commands */
  const uint8_t initWaitTime[gLCD_MaxInitSteps_c] =
  {         
    gLCD_Wait100mSec_c,
    gLCD_Wait150uSec_c,
    gLCD_Wait150uSec_c,
    gLCD_Wait150uSec_c,       
    gLCD_Wait15mSec_c,
    /* Second init step timings */
    gLCD_Wait100mSec_c,
    gLCD_Wait150uSec_c,
    gLCD_Wait150uSec_c,
    gLCD_Wait150uSec_c,       
    gLCD_Wait15mSec_c             
  };

  /* Included only for legacy compatibility with old LCd driver */
  const uint8_t gaHexValue[] = "0123456789ABCDEF";
#endif /* gLCDSupported_d */


/******************************************************************************
*******************************************************************************
* Private function prototypes 
*******************************************************************************
******************************************************************************/
#if(TRUE == gLCDSupported_d)
  static void     LCD_WaitMilliSec(uint8_t timeInMilliSeconds);
  static void     LCD_WaitMicroSec(uint8_t timeInMicroSeconds);
  static void     LCD_Task(event_t events);
  static bool_t   LCD_WriteCmd(bool_t mRegisterSelect, uint8_t dataByte);
  static bool_t   LCD_Write4Bits(uint8_t mData);
  static void     SPI_MasterTxCallback(bool_t status);
  static uint8_t  LCD_GetStrLen(const uint8_t* pString);
  static uint8_t  LCD_LaunchNewCommand(uint8_t eventsPend);
  static lcdLineProcessStatus_t LCD_DisplayTheCurrentLine(uint8_t currLine);
#endif


/******************************************************************************
*******************************************************************************
* Public functions 
*******************************************************************************
******************************************************************************/

/*****************************************************************************
* Function name:          MC1323xRCM_LCD_Init
*
* Parameters passed in:   pfLcdCallBack - callback function for notification 
*                                         when operation has been finished
* Returned value:         boolean indicating the function status
*
* Description:            Function to initalize both the LCD driver and device 
*****************************************************************************/
bool_t MC1323xRCM_LCD_Init(void (*pfLcdInitCallBack)(bool_t status))
{
  #if(TRUE == gLCDSupported_d)
    bool_t retStatus = FALSE;
             
    /* Create the LCD Task */
    gLcdTaskId = TS_CreateTask(gTsLcdTaskPriority_c, LCD_Task);    
    
    if(gTsInvalidTaskID_c != gLcdTaskId)
    {
      /* Prepare the SPI module in master mode */
      mSpiConfig.devMode      = gSPI_MasterMode_c;
      mSpiConfig.baudRate     = gSPI_BaudRate_1000000_c;
      mSpiConfig.clockPol     = gSPI_ActiveHighPolarity_c;
      mSpiConfig.clockPhase   = gSPI_EvenEdgeShifting_c;
      mSpiConfig.bitwiseShift = gSPI_MsbFirst_c;
      mSpiConfig.s3Wire       = gSPI_FullDuplex_c;  
                            
      /* Restart the initialization step counter */
      initStep = 0;
          
      /* Prepare the E pin to handle the LCD device */
      LCD_DefineEnablePin();  
      
      /* Keep the E pin as low */
      LCD_Disable();

      /* First initialization delay (see the ST7920 datasheet) */
      LCD_WaitMilliSec(initWaitTime[initStep]); 
          
      /* Start the LCD initialization */    
      lcdOperType.mLcdState         = mLcd_State_Init_c;
      lcdOperType.pfLcdInitCallback = pfLcdInitCallBack;
      lcdOperType.pfLcdCtrlCallback = NULL;
                 
      /* Add a new event for the LCD task */
      TS_SendEvent(gLcdTaskId, gLCD_Event_Init_Start_c);  
      
      retStatus = TRUE;      
    }               
    
    return retStatus;    
  #else
    (void) pfLcdInitCallBack;
    return FALSE;        
  #endif     
}


/*****************************************************************************
* Function name:          MC1323xRCM_LCD_WriteChar
*
* Parameters passed in:   chrLine - Display line
*                         chrPos  - Character position
                          pfCharCallBack - callback function to indicate the  
                                           end of the current character writting  
* Returned value:         boolean representing the function status
*
* Description:            Function to display a char to the LCD
*****************************************************************************/
bool_t MC1323xRCM_LCD_WriteChar(uint8_t chrLine, uint8_t chrPos, uint8_t* pChr, void (*pfCharCallBack)(bool_t status))
{
  #if(TRUE == gLCDSupported_d)
    bool_t  retStatus = TRUE;
    uint8_t mLcdNewCmd;
               
    if((!chrLine) || (chrLine > gLCD_MaxLcdLines_c) || (chrPos > gMAX_LCD_CHARS_c)) 
    {
      retStatus = FALSE;  
    }
    else
    {         
      /* Adjust the 'chrLine' to be used as table index */
      chrLine--;      
      
      /* Store the character to display */
      maLcdBuffTable[chrLine][chrPos] = *pChr;
                  
      /* Set DDRAM address into the next pending command */
      mLcdNewCmd = gLCD_Set_DDRAM_Addr_c; 
      /* Select the line */
      if(gLCD_SecondLine_c == chrLine)
      {
        mLcdNewCmd |= gLCD_SecondLineAC_c;
      }
      /* Select the position */
      mLcdNewCmd |= chrPos;          
              
      /* Update the LCD buffer table reference */
      mLcdBuffTableRef[chrLine].mLcdBuffCmd     = mLcdNewCmd; 
      mLcdBuffTableRef[chrLine].lcdPos          = chrPos;
      mLcdBuffTableRef[chrLine].buffLen         = gLCD_CharSize_c;
      mLcdBuffTableRef[chrLine].pfBuffCallback  = pfCharCallBack;
                          
      /* Update the LCD state and callback */
      lcdOperType.mLcdState         = mLcd_State_Display_c;
      lcdOperType.pfLcdCtrlCallback = NULL;
      
      /* Add the new event for the LCD task */
      TS_SendEvent(gLcdTaskId, gLCD_Event_Display_Start_c);       
    }      
         
    return retStatus;
  #else
    (void) chrLine;
    (void) chrPos;
    (void) pChr;
    (void) pfCharCallBack;
    
    return TRUE;
  #endif    
}

/*****************************************************************************
* Function name:          MC1323xRCM_LCD_WriteString
*
* Parameters passed in:   lcdLine - Display line
*                         pStr    - pointer to the first character of the string 
*                         
* Returned value:         boolean indicating the status of the function 
*
* Description:            This function display a string to the LCD
*****************************************************************************/
bool_t MC1323xRCM_LCD_WriteString(uint8_t lcdLine, uint8_t* pStr, void (*pfBuffCallBack)(bool_t status))
{  
  #if(TRUE == gLCDSupported_d)  
    bool_t             retStatus = TRUE; 
    uint8_t            mLcdNewCmd;  
    lcdBuffTableRef_t* pLcdBuffTableRef; 
         
    if((NULL == pStr) || (!lcdLine) || (lcdLine > gLCD_MaxLcdLines_c))
    {
      retStatus = FALSE;  
    }
    else
    {                    
      /* Adjust the 'lcdLine' to be used as table index */
      lcdLine--;
            
      /* Clear the current LCD buffer */
      FLib_MemSet(maLcdBuffTable + lcdLine, 0x20, gMAX_LCD_CHARS_c);
      /* Update the current LCD buffer */
      FLib_MemCpy(maLcdBuffTable + lcdLine, pStr, LCD_GetStrLen(pStr));
                            
      /* Set DDRAM address into the next pending command */
      mLcdNewCmd = gLCD_Set_DDRAM_Addr_c; 
      /* Select the line */
      if(gLCD_SecondLine_c == lcdLine)
      {
        mLcdNewCmd |= gLCD_SecondLineAC_c;
      }              
      
      /* Update the information for buffer display operation */     
      pLcdBuffTableRef = mLcdBuffTableRef + lcdLine;
      pLcdBuffTableRef->mLcdBuffCmd     = mLcdNewCmd; 
      pLcdBuffTableRef->lcdPos          = 0;     
      pLcdBuffTableRef->buffLen         = gMAX_LCD_CHARS_c;
      pLcdBuffTableRef->pfBuffCallback  = pfBuffCallBack;
                     
      /* Update the LCD state and callback */
      lcdOperType.mLcdState         = mLcd_State_Display_c;
      lcdOperType.pfLcdCtrlCallback = NULL;
      
      /* Add a new event for the LCD task */
      TS_SendEvent(gLcdTaskId, gLCD_Event_Display_Start_c);                                       
    }    
        
    return retStatus;  
  #else
    (void) lcdLine;
    (void) pStr;
    (void) pfBuffCallBack;
    
    return TRUE;
  #endif    
}

/*****************************************************************************
* Function name:          MC1323xRCM_LCD_ClearDisplay
*
* Parameters passed in:   pfCallBack - callback function for notification 
*                                      when operation has been finished
* Returned value:         None
*
* Description:            Function to clear the display
*****************************************************************************/
void MC1323xRCM_LCD_ClearDisplay(void (*pfLcdCtrlCallBack)(bool_t status))
{
  #if(TRUE == gLCDSupported_d)  
    
    /* Update the LCD operation structure */     
    lcdOperType.mLcdState         = mLcd_State_Control_c;
    lcdOperType.mLcdCtrlCmd       = gLCD_ClearCtrl_c;
    lcdOperType.pfLcdCtrlCallback = pfLcdCtrlCallBack;
               
    /* Record the new event for the LCD task */
    TS_SendEvent(gLcdTaskId, gLCD_Event_Control_Start_c);    
    
  #else
    (void) pfLcdCtrlCallBack;        
  #endif    
}

/*****************************************************************************
* Function name:          MC1323xRCM_LCD_DisplayOn
*
* Parameters passed in:   cursorOn - cursor is On/Off
*                         chaBlink - character is blinking or not
*                         pfLcdCtrlCallBack - pointer to the function to indicate
*                         the end of the clear operation 
* Returned value:         None
*
* Description:            Function to put the display on
*****************************************************************************/
void MC1323xRCM_LCD_DisplayOn(bool_t cursorOn, bool_t charBlink, void (*pfLcdCtrlCallBack)(bool_t status))
{
  #if(TRUE == gLCDSupported_d)
    uint8_t mLcdNewCmd;
       
    /* Prepare the next pending command */         
    mLcdNewCmd = (gLCD_DisplayCtrl_c | gLCD_DisplayOnBit_c);
    if(gLCD_CursorOn_c == cursorOn)
    {
      mLcdNewCmd |= gLCD_CursorOnBit_c;
    }
    
    if(gLCD_CharBlinkOn_c == charBlink) 
    {
      mLcdNewCmd |= gLCD_CharBlinkBit_c;
    }
        
    /* Update the LCD operation structure */      
    lcdOperType.mLcdState         = mLcd_State_Control_c;
    lcdOperType.mLcdCtrlCmd       = mLcdNewCmd;
    lcdOperType.pfLcdCtrlCallback = pfLcdCtrlCallBack;
              
    /* Add the new event for the LCD task */
    TS_SendEvent(gLcdTaskId, gLCD_Event_Control_Start_c);       

  #else
    (void) cursorOn;
    (void) charBlink;
    (void) pfLcdCtrlCallBack;        
  #endif    
}

/*****************************************************************************
* Function name:          MC1323xRCM_LCD_DisplayOff
*
* Parameters passed in:   pfLcdCtrlCallBack - callback function for notification 
*                                      when operation has been finished
* 
* Returned value:         None
*
* Description:            Function puts the display off; the content of lines are
*                         kept  
*****************************************************************************/
void MC1323xRCM_LCD_DisplayOff(void (*pfLcdCtrlCallBack)(bool_t status))
{
  #if(TRUE == gLCDSupported_d)        
  
    /* Update the LCD operation structure */     
    lcdOperType.mLcdState         = mLcd_State_Control_c;
    lcdOperType.mLcdCtrlCmd       = gLCD_DisplayCtrl_c;    
    lcdOperType.pfLcdCtrlCallback = pfLcdCtrlCallBack;
             
    /* Send event to the LCD Task */
    TS_SendEvent(gLcdTaskId, gLCD_Event_Control_Start_c);        
    
  #else
    (void) pfLcdCtrlCallBack;        
  #endif    
}


/*****************************************************************************
* Function name:          MC1323xRCM_LCD_ReturnHome
*
* Parameters passed in:   pfLcdCtrlCallBack - callback function for notification 
*                                      when operation has been finished
* Returned value:         None
*
* Description:            This function will move the cursor to the first 
*                         position of the display
*****************************************************************************/
void MC1323xRCM_LCD_ReturnHome(void (*pfLcdCtrlCallBack)(bool_t status))
{
  #if(TRUE == gLCDSupported_d)

    /* Update the LCD operation structure */      
    lcdOperType.mLcdState         = mLcd_State_Control_c;
    lcdOperType.mLcdCtrlCmd       = gLCD_HomeCtrl_C;
    lcdOperType.pfLcdCtrlCallback = pfLcdCtrlCallBack;
             
    /* Send event to the LCD Task */
    TS_SendEvent(gLcdTaskId, gLCD_Event_Control_Start_c);  

  #else
    (void) pfLcdCtrlCallBack;          
  #endif     
}

#if(TRUE == gLCDSupported_d)
  /*****************************************************************************
  * Function name:          LCD_WaitMilliSec
  *
  * Parameters pasqed in:   timeInMilliSec - time duration in milliseconds
  * Returned value:         None
  *
  * Description:            Function used to wait a number of miliseconds
  *****************************************************************************/
  static void LCD_WaitMilliSec(uint8_t timeInMilliSeconds)
  {   
    uint8_t dec;
    uint8_t dec1;
   
    while(timeInMilliSeconds--)
    {
      dec1 = 100;
      while(dec1--)
      {
        dec = gSystemClock_d*10;
        while(dec--);
      }  
    }  
  }


  /*****************************************************************************
  * Function name:          LCD_WaitMicroSec
  *
  * Parameters passed in:   timeInMicroSec - time duration in microseconds
  * Returned value:         None
  *
  * Description:            Function used to wait a number of microseconds
  ****************************************************************************/
  static void LCD_WaitMicroSec(uint8_t timeInMicroSeconds)
  {   
    uint8_t dec;
       
    while(timeInMicroSeconds--)
    {
      dec = gSystemClock_d;
      while(dec--);
    }  
  }


  /*****************************************************************************
  * Function name:          LCD_GetStrLen
  *
  * Parameters passed in:   pString - pointer to the start of the string
  * Returned value:         lenght of the string
  *
  * Description:            This function gets the length of a string and return it 
  *****************************************************************************/
  static uint8_t LCD_GetStrLen(const uint8_t* pString)
  {  
    uint8_t length = 0; 
    
    while((*pString != '\0') && (length < gMAX_LCD_CHARS_c))
    {
      length++;
      pString++;
    }
    
    return length;  
  }


  /*****************************************************************************
  * Function name:          LCD_WriteCmd
  *
  * Parameters passed in:   mRegisterSelect - instruction/data selection operation
  *                         dataByte - byte to be send
  * Returned value:         boolean indicating the function status
  *
  * Description:            Function write a command via the SPI to the LCD 
  *****************************************************************************/
  static bool_t LCD_WriteCmd(bool_t mRegisterSelect, uint8_t dataByte)
  {      
    bool_t retStatus = TRUE;
       
    /* Prepare the upper 4-bits and lower 4-bits */
    lcdInstrCode.mLcdUpperNibble = gLCD_GetUpperNibble(dataByte); 
    lcdInstrCode.mLcdLowerNibble = gLCD_GetLowerNibble(dataByte);
    
    if(gLCD_CmdRsWrite_c == mRegisterSelect)
    {
      lcdInstrCode.mLcdUpperNibble |= (1<<4);
      lcdInstrCode.mLcdLowerNibble |= (1<<4); 
    }
    lcdInstrCode.lcdWriteCmdStep = 0;
    
    /* Initiate the upper 4-bits transmission */
    retStatus = LCD_Write4Bits(lcdInstrCode.mLcdUpperNibble);
      
    return retStatus; 
  }



  /*****************************************************************************
  * Function name:          LCD_Write4Bits
  *
  * Parameters passed in:   mData - data to be sent via SPI
  * Returned value:         boolean indicating the status of the SPI operation
  *
  * Description:            Function to send the the half byte form the command
  *****************************************************************************/
  static bool_t LCD_Write4Bits(uint8_t mData)
  {    
    bool_t retStatus;
    static uint8_t spiData;
    
    /* Apply the specific configuration to the SPI module */
    /* The configuration is done here in order to be sure that no other 
       application routine affect the SPI communication */
    SPI_SetConfig(mSpiConfig);
    
    spiData = mData;
    retStatus = SPI_MasterTransmit(&spiData, gLCD_CharSize_c, SPI_MasterTxCallback);
    
    return retStatus; 
  }
   

  /******************************************************************************
  * Function name:          SPI_MasterTxCallback
  *
  * Parameters passed in:   boolean indicating the status of the SPI operation
  * Returned value:         None 
  *
  * Description:            SPI Master Transmit callback function, indicating the 
  *                         end of the SPI operation      
  ******************************************************************************/
  static void SPI_MasterTxCallback(bool_t status)
  {     
    uint8_t dummyEvent; 
            
    /* Prepare the event to the LCD_Task */
    if(TRUE == status)
    {
      /* LCD pin E asserted */
      LCD_Enable();
      
      /* Store the event */
      dummyEvent = gLCD_Event_Wr4bits_Success_c;
                               
      /* LCD pin E deasserted */
      LCD_Disable();
    }
    else
    {
      if(FALSE == status)
      {
        dummyEvent = gLCD_Event_Wr4bits_Fail_c; 
      }
    }
    
    /* Send event to the LCD Task */
    TS_SendEvent(gLcdTaskId, dummyEvent);    
  }

  /*****************************************************************************
  * Function name:          LCD_Task
  *
  * Parameters passed in:   events - LCD task's event
  * Returned value:         None
  *
  * Description:            LCD task function
  *****************************************************************************/
  static void LCD_Task(event_t events)
  {           
    static uint8_t eventsPending, eventCleared; 
    uint8_t timeWait;
        
    /* Store the current events into the pending list */
    eventsPending |= events;         
                                 
    if(eventsPending & gLCD_Event_Wr4bits_Fail_c)
    {
      /* Clear the current processing event */
      eventsPending &= ~gLCD_Event_Wr4bits_Fail_c; 
      
      /* The initialization step is most prioritar; any fail on this, should be 
         signaled to the application and further initiative should be invalidated */
      if(initStep < gLCD_MaxInitSteps_c)
      {
        if(lcdOperType.pfLcdInitCallback)
        {
          (*(lcdOperType.pfLcdInitCallback))(FALSE); 
          lcdOperType.pfLcdInitCallback = NULL;          
        }
          
        /* New LCD status: IDLE */
        lcdOperType.mLcdState = mLcd_State_Idle_c;          
      }
      else
      { /* Initialization already done */        
        /* Check if there is any pending command */
        if(eventsPending & gLCD_Event_WrCmd_Start_c)
        {
          /* Initiate the new command */
          eventCleared = LCD_LaunchNewCommand(eventsPending);          
          
          /* Clear the current processing event */
          eventsPending &= ~eventCleared;                                                  
        }
        else
        { /* No pending command */
          /* Call the callback function with status false */
          if(lcdOperType.pfLcdCtrlCallback)
          {
            (*(lcdOperType.pfLcdCtrlCallback))(FALSE); 
            /* Invalidate the callback */
            lcdOperType.pfLcdCtrlCallback = NULL;             
          }
            
          /* New LCD status: IDLE */
          lcdOperType.mLcdState = mLcd_State_Idle_c;          
        }        
      } /* End if(initStep < gLCD_MaxInitSteps_c) */     
    }
    else
    { 
      /* Check if the previous SPI has been successfully done */
      if(eventsPending & gLCD_Event_Wr4bits_Success_c)
      {
        /* Clear the current procesing event */
        eventsPending &= ~gLCD_Event_Wr4bits_Success_c;
        
        /* Upper 4-bit nibble sent successfully */
        if(++(lcdInstrCode.lcdWriteCmdStep) == gLCD_InstrExec_Partially_c)
        {                            
          /* Initiate the lower 4 bits transmission */
          if(FALSE == LCD_Write4Bits(lcdInstrCode.mLcdLowerNibble))
          {
            /* Second lower 4 bits SPI operation: FAILED */
            /* If the Init has failed then signals this to the 
               application and invalidates any further initatives */
            if(initStep < gLCD_MaxInitSteps_c)
            {                
              if(lcdOperType.pfLcdInitCallback)
              {
                (*(lcdOperType.pfLcdInitCallback))(FALSE);     
                /* Invalidate the callback */
                lcdOperType.pfLcdInitCallback = NULL;             
              }
                
              /* New LCD status: IDLE */
              lcdOperType.mLcdState = mLcd_State_Idle_c;                        
            }
            else
            {
              /* Check for the new pending command */
              if(eventsPending & gLCD_Event_WrCmd_Start_c)
              {
                /* Initiate the new command */
                eventCleared = LCD_LaunchNewCommand(eventsPending);                
                
                /* Clear the current procesing event */
                eventsPending &= ~eventCleared;                                
              }
              else
              { /* No pending command */
                /* Call the control callback function with status false */
                if(lcdOperType.pfLcdCtrlCallback)
                {
                  (*(lcdOperType.pfLcdCtrlCallback))(FALSE);
                  /* Invalidate the callback */
                  lcdOperType.pfLcdCtrlCallback = NULL;                   
                }
                  
                /* New LCD status: IDLE */
                lcdOperType.mLcdState = mLcd_State_Idle_c;                   
              }              
            } /* End: if(initStep < gLCD_MaxInitSteps_c) */                        
          }
        }
        else
        {
          if(lcdInstrCode.lcdWriteCmdStep == gLCD_InstrExec_Complete_c)
          { /* A 10 bit complete instruction has been sent */
            /* The init operation is most prioritar and any further commands 
               should still pending until the init is finished */
            if(initStep < gLCD_MaxInitSteps_c)
            {                                       
              /* Still process the initialization step */               
              if(++initStep < gLCD_MaxInitSteps_c)
              {                                
                timeWait = initWaitTime[initStep];
                if((initStep == (gLCD_MaxInitSteps_c - 1)) || (initStep == (gLCD_MaxInitSteps_c >> 1)))                   
                {
                  LCD_WaitMilliSec(timeWait); 
                }
                else
                {
                  LCD_WaitMicroSec(timeWait);                  
                }
                                
                if(FALSE == LCD_WriteCmd(gLCD_CmdRsRead_c, initCmdList[initStep]))
                {
                  if(lcdOperType.pfLcdInitCallback)
                  {
                    /* Call the init callback function with status FALSE */
                    (*(lcdOperType.pfLcdInitCallback))(FALSE);
                    /* Invalidates the callback pointer */
                    lcdOperType.pfLcdInitCallback = NULL;
                  }
                
                  lcdOperType.mLcdState = mLcd_State_Idle_c;
                }                              
              }
              else
              {
                /* Initialization end */
                /* Wait for the last instruction */
                LCD_WaitMicroSec(gLCD_Wait150uSec_c);

                /* Call the init callback */
                if(lcdOperType.pfLcdInitCallback)
                {
                  /* Call the init callback function with status TRUE */
                  (*(lcdOperType.pfLcdInitCallback))(TRUE);
                  /* Invalidate the callback pointer */ 
                  lcdOperType.pfLcdInitCallback = NULL;                              
                }                                               
                                
                if(lcdOperType.mLcdState == mLcd_State_Init_c)
                {
                  lcdOperType.mLcdState = mLcd_State_Idle_c;   
                }
                else
                {
                  /* It's possible that during initialization, a new command may occur */ 
                  /* Check if there is a new pending command in order to don't lose it */
                  if(eventsPending & gLCD_Event_WrCmd_Start_c)
                  {
                    /* Initiate the new command */
                    eventCleared = LCD_LaunchNewCommand(eventsPending);
                    
                    /* Clear the current procesing event */
                    eventsPending &= ~eventCleared;                                                   
                  }
                }                
              }
            }
            else
            { /* Initialization has done */                            
              /* Check if there is a new pending command */              
              if(eventsPending & gLCD_Event_WrCmd_Start_c)
              {
                /* A new command has been required; if the command is from init or from control, then launch the pending command */
                /* If the command is a display requirement, then check if there is any line display in progress; if there is one  
                   and the display command is for this line, then restart displaying by launching that command pending, otherwise 
                   continue with the current display operation */
                             
                /* The LCD device (CFAG12832AYGHN) requires 1 msec. between consecutive commands */
                LCD_WaitMilliSec(1);
                
                /* Either the initialization has been restarted or a new control command is required */                               
                if((!initStep) || lcdOperType.mLcdCtrlCmd)
                {
                  /* Initiate the new command */
                  eventCleared = LCD_LaunchNewCommand(eventsPending);                 
                  
                  /* Clear the current procesing event */
                  eventsPending &= ~eventCleared;                  
                }
                else
                {
                  if(!(mLcdBuffTableRef[gLCD_FirstLine_c].mLcdBuffCmd) && mLcdBuffTableRef[gLCD_FirstLine_c].buffLen) 
                  {
                    (void)LCD_DisplayTheCurrentLine(gLCD_FirstLine_c); 
                  }
                  else
                  {
                    if(!(mLcdBuffTableRef[gLCD_SecondLine_c].mLcdBuffCmd) && mLcdBuffTableRef[gLCD_SecondLine_c].buffLen)
                    {
                      (void)LCD_DisplayTheCurrentLine(gLCD_SecondLine_c);
                    }
                    else
                    {
                      /* Initiate the new command */
                      eventCleared = LCD_LaunchNewCommand(gLCD_Event_Display_Start_c);                 
                
                      /* Clear the current procesing event */
                      eventsPending &= ~eventCleared;                         
                    }
                  }                  
                }                                                           
              }
              else
              { /* No command pending */
                /* Work in the current LCD operation state */
                if(lcdOperType.mLcdState == mLcd_State_Control_c)
                {
                  /* The control operation */
                  if(lcdOperType.pfLcdCtrlCallback)
                  {
                    (*(lcdOperType.pfLcdCtrlCallback))(TRUE);
                    lcdOperType.pfLcdCtrlCallback = NULL;                    
                  }
                  
                  /* New LCD status: IDLE */
                  lcdOperType.mLcdState = mLcd_State_Idle_c;                          
                }
                else
                {
                  if(lcdOperType.mLcdState == mLcd_State_Display_c)
                  {
                    /* The display operation */
                    /* Process the lcd strings to display */
                    /* Process first line buffer string */                    
                    
                    
                    if(!(mLcdBuffTableRef[gLCD_FirstLine_c].mLcdBuffCmd)) 
                    { 
                      /* Check if the first line buffer processing has finished */
                      if(mLcd_LineProcess_Finished_c ==  LCD_DisplayTheCurrentLine(gLCD_FirstLine_c))
                      {                                             
                        /* The first line buffer display process has finished, consequently check the status 
                           of the second line buffer display operation; if there is a start to display then 
                           launch the display command otherwise continue with processing the string  */
                        if(mLcdBuffTableRef[gLCD_SecondLine_c].mLcdBuffCmd)
                        {
                          /* Start to process the second line buffer string */ 
                          (void)LCD_LaunchNewCommand(gLCD_Event_Display_Start_c);                                                  
                        } 
                        else
                        {
                          /* Continue tranmsitting buffer string on the second line */
                          /* If there is nothing to display, the function will return a status to notify this case */
                          if(mLcd_LineProcess_Finished_c == LCD_DisplayTheCurrentLine(gLCD_SecondLine_c))
                          {
                            /* Check if there is any pending command */
                            if(eventsPending & gLCD_Event_WrCmd_Start_c)
                            {
                              /* Initiate the new command */
                              eventCleared = LCD_LaunchNewCommand(eventsPending);                                  
                            
                              /* Clear the current processing event */
                              eventsPending &= ~eventCleared;                                                                          
                            }
                            else
                            {
                              /* No request pending; switch the LCD status to IDLE */
                              /* New LCD status: IDLE */
                              lcdOperType.mLcdState = mLcd_State_Idle_c;                         
                            }                                               
                          }
                        }                        
                      }
                    }
                    else
                    {
                      /* Check if the second line has finished to send data */
                      if(!(mLcdBuffTableRef[gLCD_SecondLine_c].mLcdBuffCmd))
                      {
                        if(mLcd_LineProcess_Finished_c == LCD_DisplayTheCurrentLine(gLCD_SecondLine_c))
                        {
                          /* The second line has finished sending data, consequently it is allowed to start displaying the first line */
                          (void)LCD_LaunchNewCommand(gLCD_Event_Display_Start_c);                             
                        }
                      }
                    }                                                                                                                                                                      
                  }
                }
              }              
            }                                    
          }
        }
      }
      else
      {
        /* Process a new freshed command */
        if(eventsPending & gLCD_Event_WrCmd_Start_c)
        {
          /* Initiate the new command */
          eventCleared = LCD_LaunchNewCommand(eventsPending);             
          
          /* Clear the current procesing event */
          eventsPending &= ~eventCleared;                            
        }        
      }
    }          
  } 


  /*****************************************************************************
  * Function name:          LCD_LaunchNewCommand
  *
  * Parameters passed in:   list of events pending
  * Returned value:         None
  *
  * Description:            Launch a new command to the LCD driver
  *****************************************************************************/
  static uint8_t LCD_LaunchNewCommand(uint8_t eventsPend)
  {          
    uint8_t mLcdNewCommand, callbackType;
    void (*pfNewCallback)(bool_t status);
    uint8_t retEvent = 0;
    lcdBuffTableRef_t* pLcdBuffTableRef;
         
    if(!initStep)
    {    
      mLcdNewCommand  = initCmdList[initStep];
      pfNewCallback   = lcdOperType.pfLcdInitCallback;
      callbackType    = 0;
      
      retEvent = gLCD_Event_Init_Start_c;
    }
    else
    {
      if(eventsPend & gLCD_Event_Control_Start_c)
      {
        mLcdNewCommand  = lcdOperType.mLcdCtrlCmd; 
        pfNewCallback   = lcdOperType.pfLcdCtrlCallback;
        callbackType    = 1;
        
        /* Clear the current control command */
        lcdOperType.mLcdCtrlCmd = 0; 
        
        retEvent = gLCD_Event_Control_Start_c;
      }
      else
      {
        if(eventsPend & gLCD_Event_Display_Start_c)
        {
          /* Decide which line should be started to display */
          pLcdBuffTableRef = mLcdBuffTableRef + gLCD_FirstLine_c; 
          if(pLcdBuffTableRef->mLcdBuffCmd)
          { /* Line 0 */
            /* Store the display command then clear it from the pending list */
            mLcdNewCommand  = pLcdBuffTableRef->mLcdBuffCmd;
            pLcdBuffTableRef->mLcdBuffCmd = 0;
            
            pfNewCallback = pLcdBuffTableRef->pfBuffCallback;
            callbackType  = 2;
          }
          else
          {
            pLcdBuffTableRef = mLcdBuffTableRef + gLCD_SecondLine_c; 
            if(pLcdBuffTableRef->mLcdBuffCmd)
            { /* Line 1 */
              /* Store the display command then clear it from the pending list */
              mLcdNewCommand  = pLcdBuffTableRef->mLcdBuffCmd;
              pLcdBuffTableRef->mLcdBuffCmd = 0;
              
              pfNewCallback = pLcdBuffTableRef->pfBuffCallback; 
              callbackType  = 3;           
            }
          }
          
          retEvent = gLCD_Event_Display_Start_c; 
        }        
      }    
    }

    if(FALSE == LCD_WriteCmd(gLCD_CmdRsRead_c, mLcdNewCommand))
    { /* New command has failed */
      /* Call the callback function with status false */
      if(pfNewCallback)
      {
        (*(pfNewCallback))(FALSE);
        
        /* Invalidate the callback pointer */
        switch(callbackType)
        {
          case 0:
                  lcdOperType.pfLcdInitCallback = NULL;
                  break;
          case 1:
                  lcdOperType.pfLcdCtrlCallback = NULL;
                  break;
          case 2:
                  pLcdBuffTableRef->pfBuffCallback = NULL;
                  break;
          case 3:
                  pLcdBuffTableRef->pfBuffCallback = NULL;
                  break;
        }                                     
      }
            
      /* New LCD status: IDLE */
      lcdOperType.mLcdState = mLcd_State_Idle_c;  
    }
    
    return retEvent; 
  }


  /*****************************************************************************
  * Function name:          LCD_DisplayTheCurrentLine
  *
  * Parameters passed in:   currLine - the current line to be displayed
  * Returned value:         mLcd_LineProcess_OnGoing_c - there are character left 
  *                           to be displayed on the specified line
  *                         mLcd_LineProcess_Finished_c - no character left to be 
  *                           displayed and the callback associated to that buffer 
  *                           is serviced (if there is defined)
  *
  * Description:            Display the information on the current line 
  *****************************************************************************/
  static lcdLineProcessStatus_t LCD_DisplayTheCurrentLine(uint8_t currLine)
  {                                  
    lcdLineProcessStatus_t procStatus;
    lcdBuffTableRef_t* pLcdBuffTableRef;
    
    /* Assure that you are touching the correct struct element */
    if(currLine > gLCD_SecondLine_c)
    {
      currLine = gLCD_SecondLine_c; 
    }
    
    pLcdBuffTableRef = mLcdBuffTableRef + currLine;  
    /* Process the current LCD line strings to display */
    if(pLcdBuffTableRef->buffLen)                                 
    {                               
      if(FALSE == LCD_WriteCmd(gLCD_CmdRsWrite_c, maLcdBuffTable[currLine][pLcdBuffTableRef->lcdPos])) 
      {
        /* Call the current line buffer callback */
        if(pLcdBuffTableRef->pfBuffCallback)
        {                              
          (*(pLcdBuffTableRef->pfBuffCallback))(FALSE);
          /* Invalidates the current line buffer callback pointer */
          pLcdBuffTableRef->pfBuffCallback = NULL;
        }                        
      }
                 
      /* Decrement the buffer length */
      pLcdBuffTableRef->buffLen--;
      
      /* Increment the current byte position into string */
      pLcdBuffTableRef->lcdPos++;
      
      /* Prepare the return status */
      procStatus = mLcd_LineProcess_OnGoing_c;
    }
    else
    {
      /* Call the current line buffer callback with status TRUE */
      if(pLcdBuffTableRef->pfBuffCallback)                       
      {
        (*(pLcdBuffTableRef->pfBuffCallback))(TRUE);
        /* Invalidates the current line buffer callback pointer */
        pLcdBuffTableRef->pfBuffCallback = NULL;                         
      }
      
      /* Prepare the return status */
      procStatus = mLcd_LineProcess_Finished_c; 
    }
    
    return procStatus;             
  }
#endif  

  