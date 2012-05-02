/************************************************************************************
* Led.h
*
* This header file is for LED Driver Interface.
*
* Copyright (c) 2006, Freescale, Inc.  All rights reserved.
*
*
* No part of this document must be reproduced in any form - including copied,
* transcribed, printed or by any electronic means - without specific written
* permission from Freescale Semiconductor.
*
************************************************************************************/
#ifndef _LED_H_
#define _LED_H_

#include "EmbeddedTypes.h"
#include "AppToPlatformConfig.h"
#include "PortConfig.h"
#include "TMR_Interface.h"
/******************************************************************************
*******************************************************************************
* Public Macros
*******************************************************************************
******************************************************************************/

/* are LEDs supported */

#ifndef gLEDSupported_d
  #define gLEDSupported_d     TRUE
#endif

/* is blip support in LED */
#ifndef gLEDBlipEnabled_d
 #define gLEDBlipEnabled_d   FALSE
#endif

/* possible LED values for LED_SetLed() */
typedef uint8_t LED_t;
#define LED1          0x01
#define LED2          0x02
#define LED3          0x04
#define LED4          0x08
#define LED_ALL       0x0F  /* turn on/off all LEDs */
#define LED1_FLASH    0x10
#define LED2_FLASH    0x20
#define LED3_FLASH    0x40
#define LED4_FLASH    0x80
#define LED_FLASH_ALL 0xF0  /* flash all LEDs */

typedef uint8_t LedState_t;
/* possible LED states for LED_SetLed() */
/* note: all LEDs can operate independantly */
typedef uint8_t ledState_t;
enum {
	gLedFlashing_c,       /* flash at a fixed rate */
	gLedStopFlashing_c,   /* same as gLedOff_c */
	gLedBlip_c,           /* just like flashing, but blinks only once */
	gLedOn_c,             /* on solid */
	gLedOff_c,            /* off solid */
	gLedToggle_c          /* toggle state */
};


#if (gLEDSupported_d)
/*
  Include LED hardware definitions 
*/

#if ((gTargetMC1323xREM_d == 1) || (gTargetMC1323xRCM_d == 1) || (gTarget_UserDefined_d == 1))
  #define mLED_PORT1_TARGET_c  LED1_PORT
  #define mLED_PORT2_TARGET_c  LED2_PORT
  #define mLED_PORT3_TARGET_c  LED3_PORT
  #define mLED_PORT4_TARGET_c  LED4_PORT
  #define mLED_PORT5_TARGET_c  LED5_PORT
#else
  #define mLED_PORT1_TARGET_c  LED_PORT
  #define mLED_PORT2_TARGET_c  LED_PORT
  #define mLED_PORT3_TARGET_c  LED_PORT
  #define mLED_PORT4_TARGET_c  LED_PORT
  #define mLED_PORT5_TARGET_c  LED5_PORT
#endif
  
  #define mLED1_PIN_TARGET_c   LED1_PIN
  #define mLED2_PIN_TARGET_c   LED2_PIN
  #define mLED3_PIN_TARGET_c   LED3_PIN
  #define mLED4_PIN_TARGET_c   LED4_PIN
  #define mLED5_PIN_TARGET_c   LED5_PIN

/* Interval time for toggle LED which is used for flashing LED (0 - 262140ms) */
#ifndef mLEDInterval_c
#define mLEDInterval_c  100
#endif

/* Define LED port mapping */
#define mLED_PORT1_c   mLED_PORT1_TARGET_c
#define mLED_PORT2_c   mLED_PORT2_TARGET_c
#define mLED_PORT3_c   mLED_PORT3_TARGET_c
#define mLED_PORT4_c   mLED_PORT4_TARGET_c
#define mLED_PORT5_c   mLED_PORT5_TARGET_c
#define mLED1_PIN_c    mLED1_PIN_TARGET_c
#define mLED2_PIN_c    mLED2_PIN_TARGET_c
#define mLED3_PIN_c    mLED3_PIN_TARGET_c
#define mLED4_PIN_c    mLED4_PIN_TARGET_c
#define mLED5_PIN_c    mLED5_PIN_TARGET_c

/*   Macros with functionality to LED   */
#define Led1On()            (mLED_PORT1_c &= ~mLED1_PIN_c)
#define Led1Off()           (mLED_PORT1_c |= mLED1_PIN_c)
#define Led1Toggle()        (mLED_PORT1_c ^= mLED1_PIN_c)
#define GetLed1()           (~(mLED_PORT1_c & mLED1_PIN_c))

#define Led2On()            (mLED_PORT2_c &= ~mLED2_PIN_c)
#define Led2Off()           (mLED_PORT2_c |= mLED2_PIN_c)
#define Led2Toggle()        (mLED_PORT2_c ^= mLED2_PIN_c)
#define GetLed2()           (~(mLED_PORT2_c & mLED2_PIN_c))

#define Led3On()            (mLED_PORT3_c &= ~mLED3_PIN_c)
#define Led3Off()           (mLED_PORT3_c |= mLED3_PIN_c)
#define Led3Toggle()        (mLED_PORT3_c ^= mLED3_PIN_c)
#define GetLed3()           (~(mLED_PORT3_c & mLED3_PIN_c))

#define Led4On()            (mLED_PORT4_c &= ~mLED4_PIN_c)
#define Led4Off()           (mLED_PORT4_c |= mLED4_PIN_c)
#define Led4Toggle()        (mLED_PORT4_c ^= mLED4_PIN_c)
#define GetLed4()           (~(mLED_PORT4_c & mLED4_PIN_c))

#if (gTargetMC13213SRB_d == 1) || (gTargetMC13213NCB_d == 1) || ((gTarget_UserDefined_d == 1) && (!defined(PROCESSOR_MC1323X)) && (!defined(PROCESSOR_QE128)))
  #define Led5On()          (mLED_PORT5_c &= ~mLED5_PIN_c)
  #define Led5Off()         (mLED_PORT5_c |= mLED5_PIN_c)
  #define Led5Toggle()      (mLED_PORT5_c ^= mLED5_PIN_c)
  #define GetLed5()         (~(mLED_PORT5_c & mLED5_PIN_c))
#endif

/* Turn off LEDs */
#define TurnOffLeds()       LED_TurnOffAllLeds()

/* Turn on LEDs */
#define TurnOnLeds()        LED_TurnOnAllLeds()

/* serial flashing */
#define SerialFlashing()    LED_StartSerialFlash()

/* Flashing LED */
#define Led1Flashing()      LED_StartFlash(LED1)
#define Led2Flashing()      LED_StartFlash(LED2)
#define Led3Flashing()      LED_StartFlash(LED3)
#define Led4Flashing()      LED_StartFlash(LED4)
#define StopLed1Flashing()  LED_StopFlash(LED1)
#define StopLed2Flashing()  LED_StopFlash(LED2)
#define StopLed3Flashing()  LED_StopFlash(LED3)
#define StopLed4Flashing()  LED_StopFlash(LED4)

/******************************************************************************
*******************************************************************************
* Public prototypes
*******************************************************************************
******************************************************************************/

void LED_Init(void);
void LED_TurnOffLed(LED_t LEDNr);
void LED_TurnOnLed(LED_t LEDNr);
void LED_ToggleLed(LED_t LEDNr);
void LED_StartFlash(LED_t LEDNr);
void LED_StopFlash(LED_t LEDNr);
void LED_StartSerialFlash(void);
void LED_TurnOffAllLeds(void);
void LED_TurnOnAllLeds(void);
void LED_StopFlashingAllLeds(void);
void LED_SetLed(LED_t LEDNr,LedState_t state);
void LED_SetHex(uint8_t hexValue);

/* LEDs not supported */
#else

/* define empty prototypes */
#define LED_Init()
#define LED_TurnOffLed(LEDNr)
#define LED_TurnOnLed(LEDNr)
#define LED_ToggleLed(LEDNr)
#define LED_StartFlash(LEDNr)
#define LED_StopFlash(LEDNr)
#define LED_StartSerialFlash()
#define LED_TurnOffAllLeds()
#define LED_TurnOnAllLeds()
#define LED_StopFlashingAllLeds()
#define LED_SetLed(LEDNr,state)
#define LED_SetHex(hexValue)

  #define Led1On()
  #define Led1Off()
  #define Led1Toggle()

#define Led2On()
#define Led2Off()
#define Led2Toggle()

#define Led3On()
#define Led3Off()
#define Led3Toggle()

#define Led4On()
#define Led4Off()
#define Led4Toggle()

#if (gTargetMC13213SRB_d == 1) || (gTargetMC13213NCB_d == 1)
  #define Led5On()
  #define Led5Off()
  #define Led5Toggle()
#endif
  #define Led1Flashing()
  #define Led2Flashing()
  #define Led3Flashing()
  #define Led4Flashing()
  #define StopLed1Flashing()
  #define StopLed2Flashing()
  #define StopLed3Flashing()
  #define StopLed4Flashing()
  #define TurnOffLeds()
  #define TurnOnLeds()
  #define SerialFlashing()
#endif /* gLEDSupported_d */

/******************************************************************************
*******************************************************************************
* Public type definitions
*******************************************************************************
******************************************************************************/
/* None */

/******************************************************************************
*******************************************************************************
* Public Memory Declarations
*******************************************************************************
******************************************************************************/
/* None */

/******************************************************************************
*******************************************************************************
* Public functions
*******************************************************************************
******************************************************************************/
/* None */

#endif /* _LED_H_ */
