/************************************************************************************
* This file is owned by the platform/application specific software and basically 
* defines how the Freescale 802.15.4 MAC/PHY is configured.
*
*
* (c) Copyright 2005, Freescale Semiconductor, Inc. All rights reserved.
*
*
* No part of this document must be reproduced in any form - including copied,
* transcribed, printed or by any electronic means - without specific written
* permission from Freescale Semiconductor.
************************************************************************************/

#ifndef _PLATFORMTOMACPHYCONFIG_H_
#define _PLATFORMTOMACPHYCONFIG_H_

#include "AppToPlatformConfig.h"

  // This is a list of the possible Transceivers. Do not alter!
#define gWINMC1319x_c 1 // Windows simulator
#define gMC1319x_c    2 // Abel
#define gMC1321x_c    3 // Toroweap

  // Select one of the possible Transceivers from the list above.
#if defined(TARGET_USER_DEFINED)
  #include "PortConfig.h" 
  #if (gUserTransceiverType_d == MC1319x)
      #define gTransceiverType_d gMC1319x_c
  #else    
      #define gTransceiverType_d gMC1321x_c
  #endif    

#elif defined (TARGET_TOROWEAP)
  #define gTransceiverType_d gMC1321x_c
#else
  #define gTransceiverType_d gMC1319x_c
#endif // TARGET_TOROWEAP

  // Specify if placing of execution speed demanding variables in fast memory is 
  // supported. Currently the variables for the AES (sequrity engine) are placed
  // in fast RAM if it is supported.
#define gFastRamSupported_d 1  // Set to 1 to notify the MAC/PHY that fast memory is supported.
#define gFastRamBegin_d DATA_SEG __SHORT_SEG MY_ZEROPAGE // Used as an argument to a #pragma to mark the beginning of the fast RAM placement.
#define gFastRamEnd_d DATA_SEG DEFAULT // Used as an argument to a #pragma to mark the end of the fast RAM placement.

#define gVersionStringCapability_d 1
#define gVersionStringMacBegin_d CONST_SEG BOOTLOADER_MAC_NV_DATA0
#define gVersionStringMacEnd_d CONST_SEG DEFAULT
#define gVersionStringPhyBegin_d CONST_SEG BOOTLOADER_PHY_NV_DATA0
#define gVersionStringPhyEnd_d CONST_SEG DEFAULT



#endif /* _PLATFORMTOMACPHYCONFIG_H_ */
