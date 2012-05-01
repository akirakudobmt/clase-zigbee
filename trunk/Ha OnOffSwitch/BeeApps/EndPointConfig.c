/******************************************************************************
* This is the Source file for device End point registration
*
* (c) Copyright 2006, Freescale, Inc. All rights reserved.
*
*
* No part of this document must be reproduced in any form - including copied,
* transcribed, printed or by any electronic means - without specific written
* permission from Freescale Semiconductor.
*
*
******************************************************************************/
#include "EmbeddedTypes.h"
#include "BeeStack_Globals.h"
#include "BeeStackConfiguration.h"
#include "AppZdoInterface.h"
#include "AppAfInterface.h"
#include "AppZdoInterface.h"
#include "ZdpManager.h"
#include "EndPointConfig.h"


#include "HaProfile.h"
#include "HcProfile.h"
#include "SeProfile.h"

#ifndef __IAR_SYSTEMS_ICC__
#ifdef MC13226Included_d
    #undef MC13226Included_d
#endif    
    #define MC13226Included_d 0
#endif
/******************************************************************************
*******************************************************************************
* Private Macros
*******************************************************************************
******************************************************************************/
/* None */

/******************************************************************************
*******************************************************************************
* Private Prototypes
*******************************************************************************
******************************************************************************/


/******************************************************************************
*******************************************************************************
* Private type definitions
*******************************************************************************
******************************************************************************/
/* None */

/******************************************************************************
*******************************************************************************
* Private Memory Declarations
*******************************************************************************
******************************************************************************/


/* Simple Discriptor for 0xFF(Broadcast)*/
const zbSimpleDescriptor_t BroadcastEp =
{
  /*End Point(1Byte), Device Description(2Bytes), Profile ID (2 Bytes),
  AppDeviceVersionAndFlag(1Byte), NumOfInputClusters(1Byte),
  PointerToInputClusterList(1Byte), NumOfOutputClusters(1Byte),
  PointerToOutputClusterList(1Byte) */
  0xFF,
  0x00,0x00,    /* profile ID */
  0x00,0x00,    /* device ID */
  0x00,         /* flags - no user or complex descriptor */
  0x00,         /* # of input clusters */
  NULL,         /* ptr to input clusters */
  0x00,         /* # of output clusters */
  NULL          /* ptr to output clusters */
};


/* End point 0xFF(Broadcast) description */
#if MC13226Included_d
extern endPointDesc_t broadcastEndPointDesc;
#else
const endPointDesc_t broadcastEndPointDesc =
{
  (zbSimpleDescriptor_t *)&BroadcastEp,   /* simple descriptor */
  AppBroadcastMsgCallBack,                /* broadcast MSG callback */
  NULL,                                   /* no broadcast confirm callback */
  0x00                                    /* Values 1 - 8, 0 if fragmentation is not supported. */
};
#endif


/******************************************************************************
*******************************************************************************
* Public memory declarations
*******************************************************************************
******************************************************************************/
#if gNum_EndPoints_c != 0  

/* BEGIN Endpoint Descriptor Section */

/*-- Endpoint Start --
Application name: HA OnOffSwitch application
Description: HA OnOffSwitch endpoint description.
*/

const uint8_t Endpoint8_InputClusterList[8] = {
  0x0, 0x0,  /* Basic */
  0x3, 0x0,  /* Identify */
  0x4, 0x0,  /* Groups */
  0x7, 0x0  /* OnOffConfiguration */
};

const uint8_t Endpoint8_OutputClusterList[6] = {
  0x4, 0x0,  /* Groups */
  0x5, 0x0,  /* Scenes */
  0x6, 0x0  /* OnOff */
};

const zbSimpleDescriptor_t Endpoint8_simpleDescriptor = {
  8, /* Endpoint number */
  0x4, 0x1, /* Application profile ID */
  0x3, 0x1, /* Application device ID */
  0, /* Application version ID */
  4, /* Number of input clusters */
  (uint8_t *) Endpoint8_InputClusterList, /* Input cluster list */
  3, /* Number of output clusters */
  (uint8_t *) Endpoint8_OutputClusterList, /* Output cluster list */
};

const endPointDesc_t Endpoint8_EndPointDesc = {
  (zbSimpleDescriptor_t *)&Endpoint8_simpleDescriptor,  /* pointer to the simple descriptor stored for the endpoint */
  AppMsgCallBack,        /* Callback function for MSG data */
  AppCnfCallBack,        /* Callback function for data confirmation */
gApsWindowSizeDefault_c        /* Values 1 - 8, 0 if fragmentation is not supported. */
};
/*-- Endpoint End --*/

/*- Endpoint list Start-*/
const endPointList_t endPointList[1] = {
  {&Endpoint8_EndPointDesc, &gHaOnOffSwitchDeviceDef}
};
/*- Endpoint list End-*/
/* END Endpoint Descriptor Section */
#endif

/******************************************************************************
*******************************************************************************
* Public Functions
*******************************************************************************
******************************************************************************/
/* None */

/*****************************************************************************/

