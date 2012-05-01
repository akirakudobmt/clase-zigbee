/************************************************************************************
* This file contains the Freescale HCS08 MCU register map for:
* - MC908HCS08GB60
* - MC908HCS08GT60
* - MC908HCS08GB32
* - MC908HCS08GT32
*
*
* (c) Copyright 2005, Freescale Semiconductor, Inc. All rights reserved.
*
*
* No part of this document must be reproduced in any form - including copied,
* transcribed, printed or by any electronic means - without specific written
* permission from Freescale Semiconductor.
************************************************************************************/
#ifndef _IOCONFIG_H_
#define _IOCONFIG_H_

#if defined PROCESSOR_QE128
   #include "IoConfigS08QE128.h"
#elif defined PROCESSOR_MC1323X
   #include "IoConfigMC1323x.h"          
#else   
   #include "IoConfigS08.h"
#endif

#endif /* _IOCONFIG_H_ */