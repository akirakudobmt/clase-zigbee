/******************************************************************************
* Header file for LCD driver
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

#ifndef _DISPLAY_MC1323X_RCM_H_
#define _DISPLAY_MC1323X_RCM_H_

/******************************************************************************
*******************************************************************************
* Private macros 
*******************************************************************************
******************************************************************************/
/* Enable of disable the LCD module */
#ifndef gLCDSupported_d
  #define gLCDSupported_d                     (TRUE)
#endif

/* LCD Task priority */
#ifndef gTsLcdTaskPriority_c        
  #define gTsLcdTaskPriority_c                (0x11) 
#endif
                                                                  
/* System clock */
#ifndef gSystemClock_d
  #define gSystemClock_d                      (16)    /* 16 MHz */
#endif

/* LCD line length */
#ifndef gMAX_LCD_CHARS_c 
  #define gMAX_LCD_CHARS_c                    (16)
#endif

#define LCD_DefineEnablePin()                 (PTADD  |= (1<<2))
#define LCD_Enable()                          (PTAD   |= (1<<2))
#define LCD_Disable()                         (PTAD   &=~(1<<2))

/* Display characteristics */
#define gLCD_MaxLcdLines_c                    (2)
#define gLCD_LineOne_c                        (1)
#define gLCD_LineTwo_c                        (2)

#define gMC1323xRCM_LCD_DisplayOn_c           (TRUE)
#define gMC1323xRCM_LCD_DisplayOff_c          (FALSE)

#define gLCD_CursorOn_c                       (TRUE)
#define gLCD_CursorOff_c                      (FALSE)

#define gLCD_CharBlinkOn_c                    (TRUE) 
#define gLCD_CharBlinkOff_c                   (FALSE) 

 
/* LCD Task events */
#define gLCD_Event_Init_Start_c               (1<<0)
#define gLCD_Event_Control_Start_c            (1<<1)
#define gLCD_Event_Display_Start_c            (1<<2)
#define gLCD_Event_Wr4bits_Fail_c             (1<<3)
#define gLCD_Event_Wr4bits_Success_c          (1<<4)
/* New command available */
#define gLCD_Event_WrCmd_Start_c              (gLCD_Event_Control_Start_c | gLCD_Event_Display_Start_c | gLCD_Event_Init_Start_c)

/* Initialization */ 
#define gLCD_MaxInitSteps_c                   (10)

/* Timing definitions for initialization steps */
#define gLCD_Wait40mSec_c                     (40)
#define gLCD_Wait100uSec_c                    (100)
#define gLCD_Wait10mSec_c                     (10)
#define gLCD_Wait37uSec_c                     (37)
/* Time definitions (all times are 50% greater than required) */
#define gLCD_Wait100mSec_c                    (100)
#define gLCD_Wait150uSec_c                    (150)
#define gLCD_Wait15mSec_c                     (15)
#define gLCD_Wait5mSec_c                      (5)
   
#define gLCD_OneMilliSecondInMicroSeconds_c   (1000)

/* Address Counter for the line position */
#define gLCD_FirstLineAC_c                    (0)
#define gLCD_SecondLineAC_c                   (1<<4)

#define gLCD_CmdRsWrite_c                     (TRUE)
#define gLCD_CmdRsRead_c                      (FALSE)

/* Nibble manipulation */
#define gLCD_LowNibbleMask_c                  (0x0F)
#define gLCD_HighNibbleMask_c                 (0xF0)

#define gLCD_GetLowerNibble(data)             (data & gLCD_LowNibbleMask_c)
#define gLCD_GetUpperNibble(data)             ((data & gLCD_HighNibbleMask_c)>>4)

#define gLCD_CharSize_c                       (1)

/* Steps to execute any LCD instruction */
#define gLCD_InstrExec_Partially_c            (1)
#define gLCD_InstrExec_Complete_c             (2)

/* LCD Lines */
#define gLCD_FirstLine_c                      (0)
#define gLCD_SecondLine_c                     (1)

/* Several commands */
#define gLCD_DisplayOnBit_c                   (1<<2)
#define gLCD_CursorOnBit_c                    (1<<1)
#define gLCD_CharBlinkBit_c                   (1<<0) 
 
#define gLCD_ClearCtrl_c                      (1<<0)
#define gLCD_HomeCtrl_C                       (1<<1)
#define gLCD_EntryModeCtrl_c                  (1<<2)
#define gLCD_DisplayCtrl_c                    (1<<3)
#define gLCD_CursorDisplayCtrl_c              (1<<4)
#define gLCD_FunctionSetCtrl_c                (1<<5)
#define gLCD_Set_CGRAM_Addr_c                 (1<<6)
#define gLCD_Set_DDRAM_Addr_c                 (1<<7)

#define gLCD_8BitInterface_c                  (1<<4)
#define gLCD_ExtendedInstruction_c            (1<<2)
#define gLCD_CursorMoveRight_c                (1<<1)
#define gLCD_DisplayShift_c                   (1<<0)


/******************************************************************************
*******************************************************************************
* Private function prototypes 
*******************************************************************************
******************************************************************************/
/* Initialize the LCD driver */
bool_t MC1323xRCM_LCD_Init(void (*pfLcdInitCallBack)(bool_t status));

/* Function to write a character to the LCD */
bool_t MC1323xRCM_LCD_WriteChar(uint8_t chrLine, uint8_t chrPos, uint8_t* pChr, void (*pfCharCallBack)(bool_t status));

/* Function to write a string to the LCD */
bool_t MC1323xRCM_LCD_WriteString(uint8_t lcdLine, uint8_t* pStr, void (*pfBuffCallBack)(bool_t status));

/* Function to clear the display */
void MC1323xRCM_LCD_ClearDisplay(void (*pfLcdCtrlCallBack)(bool_t status));

/* Function to put the display on */
void MC1323xRCM_LCD_DisplayOn(bool_t cursorOn, bool_t charBlink, void (*pfLcdCtrlCallBack)(bool_t status));

/* Function to put the display off */
void MC1323xRCM_LCD_DisplayOff(void (*pfLcdCtrlCallBack)(bool_t status));

/* Function to put the cursor to origin */
void MC1323xRCM_LCD_ReturnHome(void (*pfLcdCtrlCallBack)(bool_t status));

#endif /* _DISPLAY_MC1323X_RCM_H_ */