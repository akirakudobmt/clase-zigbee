/************************************************************************************
* HaOnOffSwitchEndPoint.c
*
* This module contains the OnOffSwitch endpoint data, and pointers to the proper
* handling code from ZCL.
*
* Copyright (c) 2008, Freescale, Inc. All rights reserved.
*
*
* No part of this document must be reproduced in any form - including copied,
* transcribed, printed or by any electronic means - without specific written
* permission from Freescale Semiconductor.
*
************************************************************************************/
#include "EmbeddedTypes.h"
#include "zigbee.h"
#include "FunctionLib.h"
#include "BeeStackConfiguration.h"
#include "BeeStack_Globals.h"
#include "AppAfInterface.h"
#include "AfApsInterface.h"
#include "BeeStackInterface.h"
#include "HaProfile.h"
#include "ZCLGeneral.h"
#if gZclEnableOTAClient_d ||  gZclEnableOTAServer_d 
#include "ZclOta.h"
#endif

#if gAddValidationFuncPtrToClusterDef_c
#define mNoValidationFuncPtr_c ,NULL
#else
#define mNoValidationFuncPtr_c
#endif

/******************************************************************************
*******************************************************************************
* Public memory definitions
*******************************************************************************
******************************************************************************/

haOnOffSwitchAttrRAM_t gHaOnOffSwitchData;
haOnOffSwitchAttrRAM_t gHaOnOffSwitchData10;

/* list of clusters and function handlers */
afClusterDef_t const gaHaOnOffSwitchClusterList[] =
{
  { { gaZclClusterBasic_c    }, (pfnIndication_t)ZCL_BasicClusterServer, NULL,     (void *)(&gZclBasicClusterAttrDefList), 0 mNoValidationFuncPtr_c },
  { { gaZclClusterIdentify_c }, (pfnIndication_t)ZCL_IdentifyClusterServer, (pfnIndication_t)ZCL_IdentifyClusterClient,  (void *)(&gZclIdentifyClusterAttrDefList), 0 mNoValidationFuncPtr_c },
  { { gaZclClusterGroups_c   }, (pfnIndication_t)ZCL_GroupClusterServer, (pfnIndication_t)ZCL_GroupClusterClient,     (void *)(&gaZclGroupClusterAttrDefList), 0 mNoValidationFuncPtr_c },
  { { gaZclClusterScenes_c   }, NULL, (pfnIndication_t)ZCL_SceneClusterClient,     NULL, 0 mNoValidationFuncPtr_c},
  { { gaZclClusterOnOffCfg_c   }, NULL, NULL, (void *)(&gZclOnOffSwitchConfigureClusterAttrDefList), MbrOfs(haOnOffSwitchAttrRAM_t,SwitchCfg) mNoValidationFuncPtr_c }
#if gZclEnableOTAServer_d && gZclEnableOTAClient_d
  , { { gaZclClusterOTA_c      }, (pfnIndication_t)ZCL_OTAClusterServer, (pfnIndication_t)ZCL_OTAClusterClient, (void *)(&gaZclOTAClusterAttrDef), 0 mNoValidationFuncPtr_c }
#elif gZclEnableOTAServer_d  
  , { { gaZclClusterOTA_c      }, (pfnIndication_t)ZCL_OTAClusterServer, NULL, (void *)(&gaZclOTAClusterAttrDef), 0 mNoValidationFuncPtr_c }
#elif gZclEnableOTAClient_d  
  , { { gaZclClusterOTA_c      }, NULL, (pfnIndication_t)ZCL_OTAClusterClient, (void *)(&gaZclOTAClusterAttrDef), 0 mNoValidationFuncPtr_c }
#endif   
};


afDeviceDef_t const gHaOnOffSwitchDeviceDef =
{
  (pfnIndication_t)ZCL_InterpretFoundationFrame,
  NumberOfElements(gaHaOnOffSwitchClusterList),
  (afClusterDef_t *)gaHaOnOffSwitchClusterList,  /* pointer to AF Cluster List */
  0,            /* no reportable attributes for an OnOffSwitch */
  NULL,
  &gHaOnOffSwitchData
};

afDeviceDef_t const gHaOnOffSwitchDeviceDef10 =
{
  (pfnIndication_t)ZCL_InterpretFoundationFrame,
  NumberOfElements(gaHaOnOffSwitchClusterList),
  (afClusterDef_t *)gaHaOnOffSwitchClusterList,  /* pointer to AF Cluster List */
  0,            /* no reportable attributes for an OnOffSwitch */
  NULL,
  &gHaOnOffSwitchData10
};

/******************************************************************************
*******************************************************************************
* Private memory declarations
*******************************************************************************
******************************************************************************/


/******************************************************************************
*******************************************************************************
* Public functions
*******************************************************************************
******************************************************************************/

