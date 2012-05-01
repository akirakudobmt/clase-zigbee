/************************************************************************************
* HaOnOffLightEndPoint.c
*
* This module contains the OnOffLight endpoint data, and pointers to the proper
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


#define gHaNumOnOffLights_c     1

/******************************************************************************
*******************************************************************************
* Public memory definitions
*******************************************************************************
******************************************************************************/
/* Scene data for the thermostat device */
zclOnOffLightSceneData_t gHaOnOffLightSceneData =
{
  {
    0,    /* # of active elements in scene table */
    0, 0, /* current group */
    0,    /* current scene */
    0,    /* is this currently a valid scene? */
#if gZclClusterOptionals_d
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, /* IEEE address of the device that last configured the scenes */
#endif
    sizeof(zclOnOffLightSceneTableEntry_t) /* size of an entry in the scenes table */
  }
  
};

/* make sure to have once copy of this structure per instance of this device */
haOnOffLightAttrRAM_t gHaOnOffLightData = {0x00,{0x00,0x00,0x00}};

/* only need 1 copy of this structure for all instances of this device type */
afClusterDef_t const gaHaOnOffLightClusterList[] =
{
  { { gaZclClusterBasic_c    }, (pfnIndication_t)ZCL_BasicClusterServer, NULL,     (void *)(&gZclBasicClusterAttrDefList), 0 mNoValidationFuncPtr_c },
  { { gaZclClusterIdentify_c }, (pfnIndication_t)ZCL_IdentifyClusterServer, (pfnIndication_t)ZCL_IdentifyClusterClient,  (void *)(&gZclIdentifyClusterAttrDefList), 0 mNoValidationFuncPtr_c },
  { { gaZclClusterGroups_c   }, (pfnIndication_t)ZCL_GroupClusterServer, (pfnIndication_t)ZCL_GroupClusterClient,     (void *)(&gaZclGroupClusterAttrDefList), 0 mNoValidationFuncPtr_c },
  { { gaZclClusterScenes_c   }, (pfnIndication_t)ZCL_SceneClusterServer, (pfnIndication_t)ZCL_SceneClusterClient,     (void *)(&gZclSceneClusterAttrDefList), 0 mNoValidationFuncPtr_c },
  { { gaZclClusterOnOff_c    }, (pfnIndication_t)ZCL_OnOffClusterServer, NULL,     (void *)(&gZclOnOffClusterAttrDefList), MbrOfs(haOnOffLightAttrRAM_t,onOffAttrs) mNoValidationFuncPtr_c }
#if gZclEnableOTAServer_d && gZclEnableOTAClient_d
  , { { gaZclClusterOTA_c      }, (pfnIndication_t)ZCL_OTAClusterServer, (pfnIndication_t)ZCL_OTAClusterClient, (void *)(&gaZclOTAClusterAttrDef), 0 mNoValidationFuncPtr_c }
#elif gZclEnableOTAServer_d  
  , { { gaZclClusterOTA_c      }, (pfnIndication_t)ZCL_OTAClusterServer, NULL, (void *)(&gaZclOTAClusterAttrDef), 0 mNoValidationFuncPtr_c }
#elif gZclEnableOTAClient_d  
  , { { gaZclClusterOTA_c      }, NULL, (pfnIndication_t)ZCL_OTAClusterClient, (void *)(&gaZclOTAClusterAttrDef), 0 mNoValidationFuncPtr_c }
#endif   
};

/* a list of attributes (with their cluster) that can be reported */
/* only need 1 copy of this structure for all instances of this device type */
zclReportAttr_t const gaHaOnOffLightReportList[] = 
{
  { { gaZclClusterOnOff_c    }, gZclAttrOnOff_OnOffId_c }
};


/* 
  have one copy of this structure per instance of this device. For example, if 
  there are 3 OnOffLights, make sure this array is set to 3 and initialized to 
  3 devices
*/
afDeviceDef_t const gHaOnOffLightDeviceDef =
{
 (pfnIndication_t)ZCL_InterpretFoundationFrame,
  NumberOfElements(gaHaOnOffLightClusterList),
  (afClusterDef_t *)gaHaOnOffLightClusterList,  /* pointer to Cluster List */
  NumberOfElements(gaHaOnOffLightReportList),
  (zclReportAttr_t *)gaHaOnOffLightReportList,  /* pointer to report attribute list */
  &gHaOnOffLightData,                           /* pointer to endpoint data */
  &gHaOnOffLightSceneData                    /* pointer to scene data */
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

/*****************************************************************************
* HA_StoreScene
*
* Stores the attributes in the scene
*
*****************************************************************************/
void HA_StoreScene(zbEndPoint_t endPoint, zclSceneTableEntry_t *pScene)
{
  zclOnOffLightSceneTableEntry_t *data;
  zbClusterId_t aClusterId;
  uint8_t attrLen;
  
  data = (zclOnOffLightSceneTableEntry_t*) pScene;
  
  Set2Bytes(aClusterId, gZclClusterOnOff_c);  
  (void) ZCL_GetAttribute(endPoint, aClusterId, gZclAttrOnOff_OnOffId_c, &(data->onOff), &attrLen);  
}

/*****************************************************************************
* HA_RecallScene
*
* Restores OnOff light to this state.
*
*****************************************************************************/
void HA_RecallScene(zbEndPoint_t endPoint, void *pData, zclSceneTableEntry_t *pScene)
{
  zclUIEvent_t event;
  zclOnOffLightSceneTableEntry_t *data;
  zbClusterId_t aClusterId;
  
  (void)pData;
  
  /* set the current scene */
  data = (zclOnOffLightSceneTableEntry_t*) pScene;

  Set2Bytes(aClusterId, gZclClusterOnOff_c);
  (void)ZCL_SetAttribute(endPoint, aClusterId, gZclAttrOnOff_OnOffId_c, &data->onOff);    

  /* update the device */
  event = data->onOff ? gZclUI_On_c : gZclUI_Off_c;
  BeeAppUpdateDevice(endPoint, event,0,0,NULL);
}

/*****************************************************************************
* HA_ViewSceneData
*
* Creates a payload of scene data
*
* Returns the length of the scene data payload
*****************************************************************************/
uint8_t HA_ViewSceneData(zclSceneOtaData_t *pClusterData, zclSceneTableEntry_t *pScene)
{
  zclOnOffLightSceneTableEntry_t *levelData;
  
  levelData = (zclOnOffLightSceneTableEntry_t*) pScene;
  
  /* build data for the OnOff Light */
  pClusterData->clusterId = gZclClusterOnOff_c;
  pClusterData->length = sizeof(levelData->onOff);
  pClusterData->aData[0] = levelData->onOff;
  
  return sizeof(zclSceneOtaData_t);
}

/*****************************************************************************
* HA_AddScene
*
* Internal add scene, when this device recieves and add-scene over the air.
*
* Returns status (worked or failed).
*****************************************************************************/
zbStatus_t HA_AddScene(zclSceneOtaData_t *pClusterData, zclSceneTableEntry_t *pScene)
{
  zclOnOffLightSceneTableEntry_t *levelData;    
  zbStatus_t ret = gZbSuccess_c;
  
  levelData = (zclOnOffLightSceneTableEntry_t*) pScene;  
  
  /* 
    NOTE: If device is extended with scenes across multiple clusters, 
    there must be an If(pClusterData->clusterId == xx) check for each cluster
  */
  
  if (pClusterData->clusterId == gZclClusterOnOff_c)
  {
    levelData->onOff = pClusterData->aData[0];    
  }
  else
    ret = gZclMalformedCommand_c;
        
  return ret;
}


