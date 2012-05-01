/************************************************************************************
* This header file is for Switch Driver Interface.
*
*
* (c) Copyright 2006, Freescale, Inc.  All rights reserved.
*
*
* No part of this document must be reproduced in any form - including copied,
* transcribed, printed or by any electronic means - without specific written
* permission from Freescale Semiconductor.
*
************************************************************************************/
#ifndef _SWITCH_H_
#define _SWITCH_H_
#include "AppToPlatformConfig.h"
#include "PortConfig.h"
/******************************************************************************
*******************************************************************************
* Public Macros
*******************************************************************************
******************************************************************************/
#define gSWITCHSupported_d 1

#if (gSWITCHSupported_d == 1)     
 #if (gTarget_UserDefined_d == 1)
    #define mSWITCH1_PORT_TARGET_c   SWITCH1_PORT
    #define mSWITCH2_PORT_TARGET_c   SWITCH2_PORT
    #define mSWITCH3_PORT_TARGET_c   SWITCH3_PORT
    #define mSWITCH4_PORT_TARGET_c   SWITCH4_PORT
 #else
    #define mSWITCH1_PORT_TARGET_c   SWITCH_PORT
    #define mSWITCH2_PORT_TARGET_c   SWITCH_PORT
    #define mSWITCH3_PORT_TARGET_c   SWITCH_PORT
    #define mSWITCH4_PORT_TARGET_c   SWITCH_PORT
 #endif
 
    #define mSWITCH1_TARGET_c		    mSWITCH1_MASK
    #define mSWITCH2_TARGET_c		    mSWITCH2_MASK
    #define mSWITCH3_TARGET_c		    mSWITCH3_MASK
    #define mSWITCH4_TARGET_c		    mSWITCH4_MASK
    
    /* Define PCB switch mapping */
    #define mSWITCH1_PORT_c		mSWITCH1_PORT_TARGET_c
    #define mSWITCH2_PORT_c		mSWITCH2_PORT_TARGET_c
    #define mSWITCH3_PORT_c		mSWITCH3_PORT_TARGET_c
    #define mSWITCH4_PORT_c		mSWITCH4_PORT_TARGET_c

    /* gSWITCHx_MASK_c is used for detecting which key is pressed */
    #define gSWITCH1_MASK_c		mSWITCH1_TARGET_c
    #define gSWITCH2_MASK_c		mSWITCH2_TARGET_c
    #define gSWITCH3_MASK_c		mSWITCH3_TARGET_c
    #define gSWITCH4_MASK_c		mSWITCH4_TARGET_c

    /* SwitchPortGet is used for reading the switch port */
    #define SwitchPortGet  (((mSWITCH1_PORT_c & gSWITCH1_MASK_c) ^ gSWITCH1_MASK_c) |\
                                  ((mSWITCH2_PORT_c & gSWITCH2_MASK_c) ^ gSWITCH2_MASK_c) |\
                    							((mSWITCH3_PORT_c & gSWITCH3_MASK_c) ^ gSWITCH3_MASK_c) |\
                							((mSWITCH4_PORT_c & gSWITCH4_MASK_c) ^ gSWITCH4_MASK_c))
#else
    #define SwitchPortGet    0
    #define gSWITCH1_MASK_c  0
    #define gSWITCH2_MASK_c  0
    #define gSWITCH3_MASK_c  0
    #define gSWITCH4_MASK_c  0
#endif /* gSWITCHSupported_d */

/******************************************************************************
*******************************************************************************
* Public Macros
*******************************************************************************
******************************************************************************/


/******************************************************************************
*******************************************************************************
* Public type definitions
*******************************************************************************
******************************************************************************/
/* None */

/******************************************************************************
*******************************************************************************
* Public prototypes
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

#endif /* _SWITCH_H_ */
