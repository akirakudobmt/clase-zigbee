/************************************************************************************
* Sound.h
*
* Header file for interface to the sound driver. Includes only buzzer support for
* now.
*
* Copyright (c) 2006, Freescale, Inc.  All rights reserved.
*
*
* No part of this document must be reproduced in any form - including copied,
* transcribed, printed or by any electronic means - without specific written
* permission from Freescale Semiconductor.
*
************************************************************************************/
#ifndef _SOUND_H_
#define _SOUND_H_

/* enable/disable buzzer support */
#ifndef gBuzzerSupported_d
#define gBuzzerSupported_d  FALSE
#endif

/*
  Pseudo prototypes for the macros
  void BuzzerInit(void);
  void BuzzerOn(void);      // clicks buzzer once
  void BuzzerOff(void);     // toggle between on/off to make sound
*/
#if gBuzzerSupported_d
#if (gTargetMC13213SRB_d || gTargetMC13213NCB_d)
#define BuzzerPin     0x04
#define BuzzerInit()  PTDDD |=  BuzzerPin; PTDD  &= ~BuzzerPin
#define BuzzerOn()    (PTDD  |=  BuzzerPin)
#define BuzzerOff()   (PTDD  &= ~BuzzerPin)
#define BuzzerIsOn()  (PTDD & BuzzerPin)
#define gHaveBuzzer_c  TRUE

#elif gTargetAxiomGB60_d
#define BuzzerPin     0x01
#define BuzzerInit()  PTDDD |=  BuzzerPin; PTDD  &= ~BuzzerPin
#define BuzzerOn()    PTDD  |=  BuzzerPin
#define BuzzerOff()   PTDD  &= ~BuzzerPin
#define BuzzerIsOn()  (PTDD & BuzzerPin)
#define gHaveBuzzer_c  TRUE

#elif gTargetQE128EVB_d
#define BuzzerPin     0x04
#define BuzzerInit()  PTEDD |=  BuzzerPin; PTED  &= ~BuzzerPin
#define BuzzerOn()    PTED  |=  BuzzerPin
#define BuzzerOff()   PTED  &= ~BuzzerPin
#define BuzzerIsOn()  (PTED & BuzzerPin)
#define gHaveBuzzer_c  TRUE

#elif gTargetMC1323xRCM_d
#define BuzzerPin     0x02
#define BuzzerInit()  PTADD |=  BuzzerPin; PTAD  &= ~BuzzerPin
#define BuzzerOn()    PTAD  |=  BuzzerPin
#define BuzzerOff()   PTAD  &= ~BuzzerPin
#define BuzzerIsOn()  (PTAD & BuzzerPin)
#define gHaveBuzzer_c  TRUE


/* not a supported board, turn buzzer off */
#else
#define gHaveBuzzer_c  FALSE
#endif

/* user selected to turn buzzer off */
#else
#define gHaveBuzzer_c  FALSE
#endif

/* buzzer supported this board (and turned on) */
#if gHaveBuzzer_c
void BuzzerBeep(void);

/* buzzer not supported this board (or turned off) */
#else
#define BuzzerInit()
#define BuzzerOn()
#define BuzzerOff()
#define BuzzerIsOn()
#define BuzzerBeep()
#endif

#endif /* _SOUND_H_ */

