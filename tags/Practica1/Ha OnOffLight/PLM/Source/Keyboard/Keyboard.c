/************************************************************************************
* Source file for keyboard driver.
*
* This keyboard driver has the concept of short keys and long keys. A long key is
* press and hold on a key. 
*
* The keyboard handling logic can only understand one key at a time (pressing
* two keys at once will indicate only the first key).
*
* Copyright (c) 2006, Freescale, Inc. All rights reserved.
*
*
* No part of this document must be reproduced in any form - including copied,
* transcribed, printed or by any electronic means - without specific written
* permission from Freescale Semiconductor.
*
******************************************************************************/
#include "EmbeddedTypes.h"
#include "TMR_Interface.h"
#include "Keyboard.h"
#include "Led.h"
#include "switch.h"
#include "PWRLib.h"


#if (gKeyBoardSupported_d == 1)

/******************************************************************************
*******************************************************************************
* Private macros
*******************************************************************************
******************************************************************************/

/* Keyboard Interrupt Status and Control register
 bit7-4:  Edge select for kb pins 7-4 (PTA 7-4)
 bit3:    KBIF Keyboard interrupt flag - read only
 bit2:    KBACK Keyboard interrupt acknowledge - write 1 to clear KBIF
 bit1:    KBIE Keyboard interrupt enable
 bit0:    Keyboard detection mode - 0= edge only , 1= edge and level */
#define mKBIE_BIT_c (1<<1)
#define mKBACK_BIT_c (1<<2)
#define mKBIF_BIT_c (1<<3)

#if defined(PROCESSOR_QE128)
// PROCESSOR_QE128
#define mKBI_ENABLE_c KBI2SC |= mKBIE_BIT_c
#define mKBI_DISABLE_c KBI2SC &= ~mKBIE_BIT_c
#define mKBI_ACKNOWLEDGE_c KBI2SC |= mKBACK_BIT_c
#define mSWITCH_MASK_c (gSWITCH1_MASK_c | gSWITCH2_MASK_c | gSWITCH3_MASK_c | gSWITCH4_MASK_c)
#elif defined(PROCESSOR_MC1323X)
// PROCESSOR_MC1323X
  #define mKBI_ENABLE_c       KBIxSC |= mKBIE_BIT_c
  #define mKBI_DISABLE_c      KBIxSC &= ~mKBIE_BIT_c
  #define mKBI_ACKNOWLEDGE_c  KBIxSC |= mKBACK_BIT_c
  #else
  // PROCESSOR_HCS08
    #define mKBI_ENABLE_c KBISC |= mKBIE_BIT_c
    #define mKBI_DISABLE_c KBISC &= ~mKBIE_BIT_c
    #define mKBI_ACKNOWLEDGE_c KBISC |= mKBACK_BIT_c
    #define mSWITCH_MASK_c (gSWITCH1_MASK_c | gSWITCH2_MASK_c | gSWITCH3_MASK_c | gSWITCH4_MASK_c)
#endif

/* Keyboard State */
#if (gKeyEventNotificationMode_c == gKbdEventShortLongPressMode_c)
enum {
  mStateKeyIdle,        /* coming in for first time */
  mStateKeyDetected,    /* got a key, waiting to see if it's a long key */
  mStateKeyWaitRelease  /* got the long key, waiting for the release to go back to idle */
};
#elif (gKeyEventNotificationMode_c == gKbdEventPressHoldReleaseMode_c)
enum {
  mStateKeyDebounce,
  mStateKeyHoldDetection,
  mStateKeyHoldGen,
};
#endif //gKeyEventNotificationMode_c

#define mNoKey_c 0xff

/******************************************************************************
*******************************************************************************
* Private type definitions
*******************************************************************************
******************************************************************************/
#if (gMC1323xMatrixKBD_d == 1)
typedef uint16_t   switchScan_t;
typedef struct  kbdRowPort_tag 
{
 volatile uint8_t *portData;
 volatile uint8_t *portDir;
 uint8_t pin;
}kbdRowPort_t;
#else
typedef uint8_t    switchScan_t;
#endif /*  gMC1323xMatrixKBD_d */

/******************************************************************************
*******************************************************************************
* Private prototypes
*******************************************************************************
******************************************************************************/
static void         KBD_KeyScan(uint8_t timerId);
static switchScan_t KBD_KeySwitchPortGet(void);
static uint8_t      KBD_PortGet(void);
static void         KBD_FinishScan(uint8_t timerId);
#if (gMC1323xMatrixKBD_d == 1)
static switchScan_t KBD_KeyMatrixGet(void);
static void         KBD_MatrixSetRows(bool_t activate);
static void         KBD_MatrixActivateRow(uint8_t row);
static void         KBD_MatrixDeactivateRow(uint8_t row);
#endif /*  gMC1323xMatrixKBD_d */
/******************************************************************************
*******************************************************************************
* Private memory declarations
*******************************************************************************
******************************************************************************/
#if (gKeyEventNotificationMode_c == gKbdEventShortLongPressMode_c)
static uint8_t       mKeyState = mStateKeyIdle;
static uint8_t       mLongKeyCount;
#elif (gKeyEventNotificationMode_c == gKbdEventPressHoldReleaseMode_c)
static uint8_t       mKeyState = mStateKeyDebounce;
static uint8_t       mHoldDectionKeyCount;
static uint8_t       mHoldGenKeyCount;
#endif //gKeyEventNotificationMode_c
static switchScan_t  mSwitch_SCAN;
static KBDFunction_t mpfKeyFunction;

#if (gMC1323xMatrixKBD_d == 1)
static kbdRowPort_t mKeyRowPorts[] = 
{
  #if (KBI_MATRIX_ROW1_PIN != KBI_PIN_NOT_IMPLEMENTED) 
   {&KBI_MATRIX_ROW1_PORT, &KBI_MATRIX_ROW1_PORT_DIR, KBI_MATRIX_ROW1_PIN},
  #endif
  
  #if (KBI_MATRIX_ROW2_PIN != KBI_PIN_NOT_IMPLEMENTED) 
   {&KBI_MATRIX_ROW2_PORT, &KBI_MATRIX_ROW2_PORT_DIR, KBI_MATRIX_ROW2_PIN},
  #endif
  
  #if (KBI_MATRIX_ROW3_PIN != KBI_PIN_NOT_IMPLEMENTED) 
   {&KBI_MATRIX_ROW3_PORT, &KBI_MATRIX_ROW3_PORT_DIR, KBI_MATRIX_ROW3_PIN},
  #endif

  #if (KBI_MATRIX_ROW4_PIN != KBI_PIN_NOT_IMPLEMENTED) 
   {&KBI_MATRIX_ROW4_PORT, &KBI_MATRIX_ROW4_PORT_DIR, KBI_MATRIX_ROW4_PIN},
  #endif

  #if (KBI_MATRIX_ROW5_PIN != KBI_PIN_NOT_IMPLEMENTED) 
   {&KBI_MATRIX_ROW5_PORT, &KBI_MATRIX_ROW5_PORT_DIR, KBI_MATRIX_ROW5_PIN},
  #endif

  #if (KBI_MATRIX_ROW6_PIN != KBI_PIN_NOT_IMPLEMENTED) 
   {&KBI_MATRIX_ROW6_PORT, &KBI_MATRIX_ROW6_PORT_DIR, KBI_MATRIX_ROW6_PIN},
  #endif
  
  #if (KBI_MATRIX_ROW7_PIN != KBI_PIN_NOT_IMPLEMENTED) 
   {&KBI_MATRIX_ROW7_PORT, &KBI_MATRIX_ROW7_PORT_DIR, KBI_MATRIX_ROW7_PIN},
  #endif

  #if (KBI_MATRIX_ROW8_PIN != KBI_PIN_NOT_IMPLEMENTED) 
   {&KBI_MATRIX_ROW8_PORT, &KBI_MATRIX_ROW8_PORT_DIR, KBI_MATRIX_ROW8_PIN},
  #endif    
};
#endif

uint8_t pressedKey;
tmrTimerID_t mKeyScanTimerID = gTmrInvalidTimerID_c;
/******************************************************************************
*******************************************************************************
* Public functions
*******************************************************************************/

/******************************************************************************
* Keyboard ISR. Starts a timer for itself.
******************************************************************************/
/* Place it in NON_BANKED memory */
#ifdef MEMORY_MODEL_BANKED
#pragma CODE_SEG __NEAR_SEG NON_BANKED
#else
#pragma CODE_SEG DEFAULT
#endif /* MEMORY_MODEL_BANKED */
INTERRUPT_KEYWORD void Switch_Press_ISR
  (
  )
{
  mKBI_DISABLE_c;
  mKBI_ACKNOWLEDGE_c;
  TMR_StartIntervalTimer(mKeyScanTimerID, gKeyScanInterval_c, KBD_KeyScan);
  PWRLib_MCU_WakeupReason.Bits.FromKBI = TRUE;
  #if (gMC1323xMatrixKBD_d == 1) 
   /* Switch all ROW ports to input to avoid current consumption */
   KBD_MatrixSetRows(FALSE);
  #endif 
}
#pragma CODE_SEG DEFAULT
/******************************************************************************
* Initialize the keyboard handling. Works on on Freescale demo boards.
*******************************************************************************/


void KBD_Init
  (
  KBDFunction_t pfCallBackAdr /* IN: Pointer to callback function */
  )
{
  /* timer is used to determine short or long key press */
  mKeyScanTimerID = TMR_AllocateTimer();
  
  /* where to send the data */
  mpfKeyFunction = pfCallBackAdr;
  
  #if (gTargetAxiomGB60_d == 1)
    KBISC |= 0x02;
    KBIPE |= 0xF0;
  #else
    #if defined(PROCESSOR_QE128)
      /* write kbd status and control register
      - bits 7-4 set to 0: set falling edge detect for kbd pins (7-4)
      - bit  1   set to 1: kbd interrupt enable
      - bit  0   set to 0: kbd detection mode set to edge-only */
      KBI2SC |= 0x02; 
      /* write kbd pin enable register. Set to 1 all bits corresponding
      to the pins to be used as KBD pins and not as GPIOs */
      KBI2PE |= mSWITCH_MASK; 
      /* set kbd interrupt edge select*/
      KBI2ES &= ~mSWITCH_MASK; 
    #elif defined(PROCESSOR_MC1323X)
      #if (gMC1323xMatrixKBD_d == 1)
      /* Activate rows -> ROW Pins = 0. 
       * When a switch is pressed, the corresponding column will generate an interrupt (1 -> 0)
       */
       KBD_MatrixSetRows(TRUE);
      #endif 
      KBIxSC  = 0x02; /* KBI status and control register initialized*/
      KBIxPE  =  gMC1323xKBIPinEnableMask_d; /* KBI pin enable controls initialized*/
      KBIxES  = ~gMC1323xKBIPinEnableMask_d; /* KBI interrupt edge select*/   
    #else 
      /* write kbd status and control register
      - bits 7-4 set to 0: set falling edge detect for kbd pins (7-4)
      - bit  1   set to 1: kbd interrupt enable
      - bit  0   set to 0: kbd detection mode set to edge-only */
      KBISC |= 0x02;
      /* write kbd pin enable register. Set to 1 all bits corresponding
      to the pins to be used as KBD pins and not as GPIOs */
      KBIPE |= mSWITCH_MASK;
    #endif
  #endif

}

/******************************************************************************
*******************************************************************************
* Private functions
*******************************************************************************
******************************************************************************/

/******************************************************************************
* Called to get state of keyboard
******************************************************************************/
static switchScan_t KBD_KeySwitchPortGet
  (
  void
  )
{
 #if (gMC1323xMatrixKBD_d == 1)
  #if ((KBI_MATRIX_ROW_MASK == 0) || (KBI_MATRIX_COL_MASK == 0))
   #error "Wrong configuration for ROW or COL masks when using Matrix scan"
  #endif 
  return KBD_KeyMatrixGet(); /* Returns the first pressed row and column */
 #else
   uint8_t portScan = SwitchPortGet;
   pressedKey = mNoKey_c;
   
   if(portScan & gSWITCH1_MASK_c)
      pressedKey = 0;
   else if(portScan & gSWITCH2_MASK_c)
      pressedKey = 1;
   else if(portScan & gSWITCH3_MASK_c)
      pressedKey = 2;
   else if(portScan & gSWITCH4_MASK_c)
      pressedKey = 3;
  return portScan;
 #endif   
}


/******************************************************************************
* Called to get state of the port
******************************************************************************/
static uint8_t KBD_PortGet
  (
  void
  )
{
 #if (gMC1323xMatrixKBD_d == 1)
  uint8_t      colVal;
  
  /* Check if at least one row is activated */
  
  /* Activate all rows (logical 0) */
  KBD_MatrixSetRows(TRUE);
  
  colVal = KBI_MATRIX_COL_GET;
  
  /* Deactivate all rows to avoid current consumption */
  KBD_MatrixSetRows(FALSE);
  
  return colVal;
 #else
  return SwitchPortGet;
 #endif    
}

/******************************************************************************
* Called to check if a key is still pressed
******************************************************************************/
bool_t KBD_KeyCheck(switchScan_t previousPressed)
{
  bool_t pressed = FALSE;
  
 #if (gMC1323xMatrixKBD_d == 1)
  /* Check if the matrix key is still pressed */
  uint8_t r = (uint8_t)(previousPressed >> 8);
  uint8_t c_mask = (uint8_t)previousPressed;

  
  /* Activate this row */
  KBD_MatrixActivateRow(r);
  
  if(KBI_MATRIX_COL_GET & c_mask)
  {
   /* The key is still pressed */
   pressed = TRUE;
  }
  
  /* Deactivate the row */
  KBD_MatrixActivateRow(r);
 #else
   uint8_t portScan = SwitchPortGet;
   uint8_t key = mNoKey_c;
   
   (void)previousPressed; /* avoid compiler warnings */
  
   if(portScan & gSWITCH1_MASK_c)
      key = 0;
   else if(portScan & gSWITCH2_MASK_c)
      key = 1;
   else if(portScan & gSWITCH3_MASK_c)
      key = 2;
   else if(portScan & gSWITCH4_MASK_c)
      key = 3;
  /* Check if the switch is still pressed */
  if(pressedKey == key)
  {
   pressed = TRUE;
  }
 #endif 
  return pressed;
}

/******************************************************************************
* Called to activate / deactivate all rows
******************************************************************************/
#if (gMC1323xMatrixKBD_d == 1)
static void KBD_MatrixSetRows(bool_t activate) 
{
  uint8_t i;
  
  for (i = 0; i < (sizeof(mKeyRowPorts) / sizeof(mKeyRowPorts[0])); i++) 
  {                                   
    if(activate) 
    {
      KBD_MatrixActivateRow(i);
    }
    else 
    {
      KBD_MatrixDeactivateRow(i);
    }
  }
}
#endif /* gMC1323xMatrixKBD_d */


/******************************************************************************
* Called to activate one row
******************************************************************************/
#if (gMC1323xMatrixKBD_d == 1)
static void KBD_MatrixActivateRow(uint8_t row)
{
   /* Direction to output */
    *(mKeyRowPorts[row].portDir)  |=  mKeyRowPorts[row].pin;
   /* Data = 0 */
    *(mKeyRowPorts[row].portData) &= ~mKeyRowPorts[row].pin;
}
#endif /* gMC1323xMatrixKBD_d */

/******************************************************************************
* Called to de-activate one row
******************************************************************************/
#if (gMC1323xMatrixKBD_d == 1)
static void KBD_MatrixDeactivateRow(uint8_t row)
{
     /* Direction to input */
      *(mKeyRowPorts[row].portDir)  &= ~mKeyRowPorts[row].pin;
}
#endif /* gMC1323xMatrixKBD_d */



/******************************************************************************
* Called to get state of the matrix keyboard.
******************************************************************************/
#if (gMC1323xMatrixKBD_d == 1)
static switchScan_t KBD_KeyMatrixGet(void)
{
  /* Get the row and column values */
  uint8_t      colVal;
  switchScan_t portReading = 0;
  uint8_t      r = 0, r_mask = 1, c = 0, c_mask = 1;
  uint8_t      mask;
  
  pressedKey = mNoKey_c;
  
  /* Check if at least one column is activated */
  
  /* Activate the rows */
  KBD_MatrixSetRows(TRUE);
  
  colVal = KBI_MATRIX_COL_GET;
  
  /* De-activate the rows */
  KBD_MatrixSetRows(FALSE);
  
  if(colVal)
  {
    /* Find the first pressed column */
   mask = KBI_MATRIX_COL_MASK;  /*  KBI_MATRIX_COL_MASK cannot be 0 (forced compiler error) */
  
   while((!(colVal & 0x01)) && (c < gMC1323xKBD_COLS_d) && (mask != 0))
   {
    /* This pin was not asserted */
    if(mask&0x01)
    {
     /* And it belongs to a column */
     c++;
    } 
    mask = mask >>1;
    colVal = colVal>>1;
    c_mask = c_mask<<1;
   }
   
   if(c < gMC1323xKBD_COLS_d)
   {
    /* valid column found to be pressed
     * Activate the rows in ascending order to find the first pressed column
     */
          
     do
     {
      KBD_MatrixActivateRow(r);            
      /* Check again the column to see if the current row activates it */
     }while( (!(KBI_MATRIX_COL_GET & c_mask)) && (r++ < gMC1323xKBD_ROWS_d) );
    
   }
   
   if(r < gMC1323xKBD_ROWS_d)
   {
    /* A column was determined to be pressed */    
    portReading = ((switchScan_t)r << 8) | c_mask;
    pressedKey = (r * gMC1323xKBD_COLS_d) + c + 1;
   }
  }  
  
  /* De-activate the rows */
  KBD_MatrixSetRows(FALSE);
  
  return portReading;
}
#endif /* gMC1323xMatrixKBD_d */
/******************************************************************************
* Called when a key is pressed. Determines when the key is up (lifted).
******************************************************************************/
#if (gKeyEventNotificationMode_c == gKbdEventShortLongPressMode_c)
static void KBD_KeyScan
  (
  uint8_t timerId
  )
{
  uint8_t keyBase;

  switch(mKeyState) {

    /* got a fresh key */
    case mStateKeyIdle:      
      mSwitch_SCAN = KBD_KeySwitchPortGet();
      if(mSwitch_SCAN != 0)
      {        
       mKeyState = mStateKeyDetected;
       mLongKeyCount = 0;
      }
      else
      {
       KBD_FinishScan(timerId); 
      }
      break;

    /* a key was detected. Has it been released or still being pressed? */
    case mStateKeyDetected:
       keyBase = 0;  /* assume no key */
       
       /* Check to see if the key is still pressed. Ignore other pressed keys */
       if( KBD_KeyCheck(mSwitch_SCAN) ) 
       {
         mLongKeyCount++;

         if(mLongKeyCount >= gLongKeyIterations_c) {
           keyBase = gKBD_EventLongSW1_c;
         }

       }
       else {     
         /* short keypress */
         keyBase = gKBD_EventSW1_c;
       }
      
       if(keyBase) {       

        /* if a key was pressed, send it */
        if(pressedKey != mNoKey_c)
        {
          #if (gMC1323xMatrixKBD_d == 1)
           if(keyBase == gKBD_EventSW1_c)
           {
             mpfKeyFunction(gKBD_Event_c, pressedKey);
           }
           else
           {
             mpfKeyFunction(gKBD_EventLong_c, pressedKey); 
           }
          #else
           mpfKeyFunction(keyBase + pressedKey);
          #endif /* gMC1323xMatrixKBD_d */
        }

        /* whether we sent a key or not, wait to go back to keybaord  */
        mKeyState = mStateKeyWaitRelease;
      }
      break;

    /* got the long key, waiting for the release to go back to idle */
    case mStateKeyWaitRelease:      
      /* wait for the release before going back to idle */  
      if((mSwitch_SCAN == 0) || (KBD_PortGet() == 0)) 
      {
        mKeyState = mStateKeyIdle;
        
        KBD_FinishScan(timerId);                
      }
      break;
  }
}
#elif (gKeyEventNotificationMode_c == gKbdEventPressHoldReleaseMode_c)
static void KBD_KeyScan
  (
  uint8_t timerId
  )
{
  switch(mKeyState) {    
    case mStateKeyDebounce:
      mSwitch_SCAN = KBD_KeySwitchPortGet();
      if(mSwitch_SCAN != 0) {
        mKeyState =  mStateKeyHoldDetection;
        mHoldDectionKeyCount = 0; 
        //Generate press event indication     
        if(pressedKey != mNoKey_c) {
          #if (gMC1323xMatrixKBD_d == 1)
            mpfKeyFunction(gKBD_EventPress_c, pressedKey);
          #else
            mpfKeyFunction(gKBD_EventPressSW1_c + pressedKey);
          #endif /* gMC1323xMatrixKBD_d */           
        }
      } 
      else {
        KBD_FinishScan(timerId);
      }      
    break;
    case mStateKeyHoldDetection:
      if( KBD_KeyCheck(mSwitch_SCAN) ) {
        mHoldDectionKeyCount++;        
        if (mHoldDectionKeyCount >= gFirstHoldDetectIterations_c) {
          //first hold event detected - generate hold event
          if(pressedKey != mNoKey_c) {
            #if (gMC1323xMatrixKBD_d == 1)
              mpfKeyFunction(gKBD_EventHold_c, pressedKey);
            #else
              mpfKeyFunction(gKBD_EventHoldSW1_c + pressedKey);
            #endif /* gMC1323xMatrixKBD_d */                       
          }                    
          mHoldGenKeyCount = 0;
          mKeyState = mStateKeyHoldGen;
        }
      } 
      else {        
        if((mSwitch_SCAN == 0) || (KBD_PortGet() == 0)) {
          if(pressedKey != mNoKey_c) {
            #if (gMC1323xMatrixKBD_d == 1)
              mpfKeyFunction(gKBD_EventRelease_c, pressedKey);
            #else
              mpfKeyFunction(gKBD_EventReleaseSW1_c + pressedKey);
            #endif /* gMC1323xMatrixKBD_d */                       
            mKeyState = mStateKeyDebounce;
            KBD_FinishScan(timerId);            
          }                                       
        }                
      }
    break;
    case mStateKeyHoldGen:
      if( KBD_KeyCheck(mSwitch_SCAN) ) {
        mHoldGenKeyCount++;
        if(mHoldGenKeyCount >= gHoldDetectIterations_c) {
          mHoldGenKeyCount = 0;
          if(pressedKey != mNoKey_c) {
            #if (gMC1323xMatrixKBD_d == 1)
              mpfKeyFunction(gKBD_EventHold_c, pressedKey);
            #else
              mpfKeyFunction(gKBD_EventHoldSW1_c + pressedKey);
            #endif /* gMC1323xMatrixKBD_d */                       
          }           
        }
      } 
      else {
        if((mSwitch_SCAN == 0) || (KBD_PortGet() == 0)) {
          if(pressedKey != mNoKey_c) {
            #if (gMC1323xMatrixKBD_d == 1)
              mpfKeyFunction(gKBD_EventRelease_c, pressedKey);
            #else
              mpfKeyFunction(gKBD_EventReleaseSW1_c + pressedKey);
            #endif /* gMC1323xMatrixKBD_d */                       
            mKeyState = mStateKeyDebounce;
            KBD_FinishScan(timerId);            
          }                                       
        }                     
      }
    break;
    default:
    break;
  }
}
#endif//gKeyEventNotificationMode_c

/******************************************************************************
* Called when a scan is finished. Reactivates the KBI interrupt.
******************************************************************************/
static void KBD_FinishScan(uint8_t timerId)
{
  TMR_StopTimer(timerId);
        
  #if (gMC1323xMatrixKBD_d == 1)
   /* Reactivate the rows */
   KBD_MatrixSetRows(TRUE);
  #endif

  /* enable keyboard interrupts */
   mKBI_ACKNOWLEDGE_c;
   mKBI_ENABLE_c;
}

/******************************************************************************
*******************************************************************************
* Private Debug stuff
*******************************************************************************
******************************************************************************/


#endif /* gKeyBoardSupported_d */

