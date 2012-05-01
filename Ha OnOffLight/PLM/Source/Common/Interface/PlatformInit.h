/************************************************************************************
* Init platform
*
*
* (c) Copyright 2006, Freescale Semiconductor, Inc. All rights reserved.
*
*
* No part of this document must be reproduced in any form - including copied,
* transcribed, printed or by any electronic means - without specific written
* permission from Freescale Semiconductor.
************************************************************************************/

#ifndef _PLATFORMINIT_H_
#define _PLATFORMINIT_H_


#ifdef PROCESSOR_MC1323X

  //PPD MODE
#define gPpdModeOFF_d 0x00
#define gPpdModeON_LowThrs_m80dBm_d   0xF3
#define gPpdModeON_MidThrs_m70dBm_d   0xF1
#define gPpdModeON_HighThres_m60dBm_d 0xF9

  //PPD MODE
#define gPpdMode_d  gPpdModeOFF_d

#endif //PROCESSOR_MC1323X

#ifdef PROCESSOR_MC1323X
void PPD_InitPpdMode(void);
#endif //PROCESSOR_MC1323X

/************************************************************************************
* Perform the complete GPIO port initialization
*
* Interface assumptions:
*
* Return value:
*   NONE
*
* Revision history:
*
************************************************************************************/
void PlatformPortInit(void);

/************************************************************************************
* Initialize platform specific stuff
* NVRAM, Bootloader, MC1319x driver etc.
*
* Interface assumptions:
*
*   NOTE!!! This First section of this function code is added to ensure that the symbols 
*   are included in the build. The linker will remove these symbols if not referenced.
*   Hope to find a better way 21.10.03 MVC 
*
* Return value:
*   NONE
*
************************************************************************************/
void Platform_Init(void);


/************************************************************************************
* Initialize platform and stack. 
*
* This function is the main initialization procedure, which will be called from the 
* startup code of the project. 
* If the code is build as a stand-alone application, this function is
* called from main() below. 
*
* Interface assumptions:
*
* Return value:
*   NONE
*
************************************************************************************/
void Init_802_15_4(void);


#endif /* _PLATFORMINIT_H_  */ 
