/************************************************************************************
* This header file is for Ha OnOffLight application Interface
*
* (c) Copyright 2005, Freescale, Inc.  All rights reserved.
*
*
* No part of this document must be reproduced in any form - including copied,
* transcribed, printed or by any electronic means - without specific written
* permission from Freescale Semiconductor.
*
************************************************************************************/
#ifndef _BAPP_H_
#define _BAPP_H_

#include "BeeStack_Globals.h"
#include "EmbeddedTypes.h"
#include "zigbee.h"
#include "ZCL.h"
#include "ZCLGeneral.h"
#include "keyboard.h"
#include "AppToPlatformConfig.h"
/******************************************************************************
*******************************************************************************
* Public Macros
*******************************************************************************
******************************************************************************/
#define gHaOnOffLight_d 1

#define gIdentifyTimeSecs_d 10

#define BeeKitGroupDefault_d 0x01,0x00

#define BeeKitSceneDefault_d 1

/* events for the application */

/* The top 5 events are reserved by BeeStack. Apps may use bits 0-10 */
#define gAppEvtDataConfirm_c      (1 << 15)
#define gAppEvtDataIndication_c   (1 << 14)
#define gAppEvtStoreScene_c       (1 << 12) /*Store Scene event */
#define gAppEvtAddGroup_c         (1 << 13) /*Add group event */
#define gAppEvtSyncReq_c          (1 << 11)
#define gInterPanAppEvtDataConfirm_c      (1 << 10)
#define gInterPanAppEvtDataIndication_c   (1 << 9) 

#define gAppRadiusCounter_c      gNwkMaximumDepth_c * 2

/* Default Aps security option - Home Automation uses only Nwk security */
#if gApsLinkKeySecurity_d
  #if gZDPNwkSec_d
    #define zdoTxOptionsDefault_c (gApsTxOptionUseNwkKey_c)
  #else
    #define zdoTxOptionsDefault_c (gApsTxOptionSecEnabled_c)
  #endif
  #define afTxOptionsDefault_c (gApsTxOptionSecEnabled_c)
#else
/* default to no APS layer security (both ZigBee and ZigBee Pro) */
  #define afTxOptionsDefault_c gApsTxOptionNone_c
  #define zdoTxOptionsDefault_c gApsTxOptionNone_c
#endif



/************************Control macros for an external light *****************/
/*example on how the macro's can be configured to control an GPIO pin */
#if(gTargetMC13213SRB_d)
#define ExternalDeviceInit()    PTBDD |= 0x40;
#define ExternalDeviceOn()      PTBD|=0x40
#define ExternalDeviceOff()     PTBD&=~0x40
#define ExternalDeviceToggle()  PTBD^=0x40
#else
#define ExternalDeviceInit()    
#define ExternalDeviceOn()
#define ExternalDeviceOff()
#define ExternalDeviceToggle()
#endif

/******************************************************************************
*******************************************************************************
* Public type definitions
*******************************************************************************
******************************************************************************/

/******************************************************************************
*******************************************************************************
* Public Memory Declarations
*******************************************************************************
******************************************************************************/

/******************************************************************************
*******************************************************************************
* Public Prototypes
*******************************************************************************
******************************************************************************/
void InitZtcForZcl(void);

void BeeAppInit(void);
void BeeAppHandleKeys(key_event_t keyCode);

/* tell applicaton about change in HA status */
void BeeAppUpdateDevice(zbEndPoint_t endPoint, zclUIEvent_t event, zclAttrId_t attrId, zbClusterId_t clusterId, void *pData);
/**********************************************************************************/
#endif /* _BAPP_H_ */
