/************************************************************************************
* Sound.c
*
* Sound driver. Includes only buzzer support for now.
*
* Copyright (c) 2006, Freescale, Inc.  All rights reserved.
*
*
* No part of this document must be reproduced in any form - including copied,
* transcribed, printed or by any electronic means - without specific written
* permission from Freescale Semiconductor.
*
************************************************************************************/
#include "TMR_Interface.h"
#include "IoConfig.h"
#include "AppToPlatformConfig.h"
#include "Sound.h"

#if (gHaveBuzzer_c)
tmrTimerID_t mSoundTimerID;


/*****************************************************************************
* BuzzerTimerCallBack
*
* Handle buzzer timer. Turn off when done.
*****************************************************************************/
void BuzzerTimerCallBack
  (
  tmrTimerID_t timerID
  )
{
  (void)timerID;
  BuzzerOff();
}

/*****************************************************************************
* BuzzerBeep
*
* Create short beep (1/4 second). Requies a timer
*
*****************************************************************************/
void BuzzerBeep
  (
  void
  )
{
  /* allocate a timer if not one already */
  if(!mSoundTimerID)
    mSoundTimerID = TMR_AllocateTimer();

  /* start clicking buzzer */
  TMR_StartSingleShotTimer(mSoundTimerID, 250, BuzzerTimerCallBack);
  BuzzerOn();
}

#endif /* gHaveBuzzer_c */

