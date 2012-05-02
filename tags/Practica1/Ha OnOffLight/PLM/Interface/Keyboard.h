/************************************************************************************
* This header file is for Keyboard Driver Interface.
*
* Copyright (c) 2006, Freescale, Inc.  All rights reserved.
*
*
* No part of this document must be reproduced in any form - including copied,
* transcribed, printed or by any electronic means - without specific written
* permission from Freescale Semiconductor.
*
*****************************************************************************/

#ifndef _KEYBOARD_H_
#define _KEYBOARD_H_

#include "IrqControlLib.h"
#include "PortConfig.h"

#define gKbdEventShortLongPressMode_c   1
#define gKbdEventPressHoldReleaseMode_c 2

#ifndef gKeyEventNotificationMode_c
#define gKeyEventNotificationMode_c gKbdEventShortLongPressMode_c
#endif //gKeyEventNotificationMode_c

/* enabled keyboard support (or not) */
#ifndef gKeyBoardSupported_d
  #define gKeyBoardSupported_d       TRUE
#endif


/* Processor MC1323x has 2 KBI modules.
 * KBI1 - 8 input pins - optionally enabled in the KBD software component
 * KBI2 - 4 input pins - always enabled in the KBD software component
 */
#ifdef PROCESSOR_MC1323X  
 #define gMC1323xMatrixKBD_d        KBI_ENABLE_MATRIX
 #define gMC1323xKBI1Enable_d       TRUE
 #define gMC1323xKBI2Enable_d       FALSE
 
 #if gMC1323xMatrixKBD_d 
  #define gMC1323xKBIPinEnableMask_d KBI_MATRIX_COL_MASK
 #else   
  #define gMC1323xKBIPinEnableMask_d mSWITCH_MASK
 #endif
  
 #define gMC1323xKBD_ROWS_d         KBI_MATRIX_NUM_ROWS
 #define gMC1323xKBD_COLS_d         KBI_MATRIX_NUM_COLS
#else
 #define gMC1323xMatrixKBD_d        FALSE
 #define gMC1323xKBI1Enable_d       FALSE
 #define gMC1323xKBI2Enable_d       FALSE
 #define gMC1323xKBIPinEnableMask_d 0x00
 #define gMC1323xKBD_ROWS_d         0
 #define gMC1323xKBD_COLS_d         0 
#endif

#if ((gKeyBoardSupported_d == TRUE) && defined(PROCESSOR_MC1323X))
 #if ((gMC1323xKBI1Enable_d == TRUE) && (gMC1323xKBI2Enable_d == TRUE))
  #error "KBI Configuration not supported on MC1323x"
 #endif /* (gMC1323xKBI1Enable_d == TRUE) && (gMC1323xKBI2Enable_d == TRUE) */ 
 #if (gMC1323xKBI1Enable_d == TRUE)
   #define KBIxSC   KBISC
   #define KBIxPE   KBIPE
   #define KBIxES   KBIES
  #elif(gMC1323xKBI2Enable_d == TRUE)  
     #define KBIxSC   KBI2SC
     #define KBIxPE   KBI2PE
     #define KBIxES   KBI2ES
   #else
     #error "KBI Configuration not supported on MC1323x" 
 #endif /*gMC1323xKBI1Enable_d */
 #if ((gMC1323xMatrixKBD_d == TRUE) && ((gMC1323xKBD_ROWS_d == 0) || \
                                        (gMC1323xKBD_COLS_d == 0) || \
                                        (gMC1323xKBD_ROWS_d > 8 ) || \
                                        (gMC1323xKBD_COLS_d > 8) \
                                       ) \
     )     
  #error "KBI Configuration not supported on MC1323x"  
 #endif /* (gMC1323xMatrixKBD_d == TRUE) && ... */ 
#endif

/* Constant for a keypress. A short key will be returned after this # of millisecond if pressed */
/* make sure this constant is long enough for debounce time (default 50 milliseconds) */
#ifndef gKeyScanInterval_c
#define gKeyScanInterval_c 50
#endif  //gKeyScanInterval_c

#if (gKeyEventNotificationMode_c == gKbdEventShortLongPressMode_c)
/* A long key is if the key is held down for at least this many of the keyScanInterval */
/* default 20*50 = 1000 milliseconds or 1 second for long key */

#ifndef gLongKeyIterations_c
#define gLongKeyIterations_c 20
#endif  //gLongKeyIterations_c

#elif (gKeyEventNotificationMode_c == gKbdEventPressHoldReleaseMode_c)

#ifndef gFirstHoldDetectIterations_c
#define gFirstHoldDetectIterations_c 6
#endif  //gFirstHoldDetectIterations_c

#ifndef gHoldDetectIterations_c
#define gHoldDetectIterations_c      6
#endif  //gHoldDetectIterations_c

#endif //gKeyEventNotificationMode_c



/*****************************************************************************
******************************************************************************
* Public macros
******************************************************************************
*****************************************************************************/

/* Each key delivered to the callback function is of this type (see the following enum) */
typedef uint8_t key_event_t;

/* which key code is given to the callback function */
#if (gKeyEventNotificationMode_c == gKbdEventShortLongPressMode_c)
enum {
  gKBD_EventSW1_c = 1,
  gKBD_EventSW2_c,
  gKBD_EventSW3_c,
  gKBD_EventSW4_c,
  gKBD_EventLongSW1_c,
  gKBD_EventLongSW2_c,
  gKBD_EventLongSW3_c,
  gKBD_EventLongSW4_c
};
#if (gMC1323xMatrixKBD_d == 1)
/* event types for MC1323x Matrix KBD */
enum {
  gKBD_Event_c = 1,
  gKBD_EventLong_c = 2
};
#endif /* gMC1323xMatrixKBD_d */
#elif (gKeyEventNotificationMode_c == gKbdEventPressHoldReleaseMode_c)
enum {
  gKBD_EventPressSW1_c = 1,
  gKBD_EventPressSW2_c,
  gKBD_EventPressSW3_c,
  gKBD_EventPressSW4_c,
  gKBD_EventHoldSW1_c,
  gKBD_EventHoldSW2_c,
  gKBD_EventHoldSW3_c,
  gKBD_EventHoldSW4_c,
  gKBD_EventReleaseSW1_c,
  gKBD_EventReleaseSW2_c,
  gKBD_EventReleaseSW3_c,
  gKBD_EventReleaseSW4_c, 
}; 
#if (gMC1323xMatrixKBD_d == 1)
/* event types for MC1323x Matrix KBD */
enum {
  gKBD_EventPress_c = 1,
  gKBD_EventHold_c = 2,
  gKBD_EventRelease_c = 3,
};
#endif /* gMC1323xMatrixKBD_d */
#endif //gKeyEventNotificationMode_c



/* Callback function prototype */
#if (gMC1323xMatrixKBD_d == 1)
    typedef void (*KBDFunction_t) ( uint8_t events, uint8_t pressedKey );
#else    
    typedef void (*KBDFunction_t) ( uint8_t events );
#endif

/*****************************************************************************
******************************************************************************
* Public type definitions
******************************************************************************
*****************************************************************************/
/*None*/

/*****************************************************************************
******************************************************************************
* Public prototypes
******************************************************************************
*****************************************************************************/
#if (gKeyBoardSupported_d == 1)

/******************************************************************************
* This function generates a callback function
*
* Interface assumptions:
*
* Return value:
* None
*
******************************************************************************/
extern void KBD_Init
  (
  KBDFunction_t pfCallBackAdr /* IN: Pointer to callback function */
  );

/******************************************************************************
* This interrupt function is the ISR keyboard function
*
* Interface assumptions:
*
*
* Return value:
* None
*
*
******************************************************************************/

/* Place it in NON_BANKED memory */
#ifdef MEMORY_MODEL_BANKED
#pragma CODE_SEG __NEAR_SEG NON_BANKED
#else
#pragma CODE_SEG DEFAULT
#endif /* MEMORY_MODEL_BANKED */
INTERRUPT_KEYWORD void Switch_Press_ISR(void);

#pragma CODE_SEG DEFAULT  

#else
  #define KBD_Init(pfCallBackAdr)
  #define Switch_Press_ISR       Default_Dummy_ISR
#endif /* gKeyBoardSupported_d */

 
#endif /* _KEYBOARD_H_ */
