/*****************************************************************************
* BeeApp.c
*
* This application (HaOnOffLight) contains the code necessary for an on/off
* light. It is controled over-the-air by an HaOnOffSwitch, but can also turn
* on it's local light.
*
* Copyright (c) 2008, Freescale, Inc. All rights reserved.
*
*
* No part of this document must be reproduced in any form - including copied,
* transcribed, printed or by any electronic means - without specific written
* permission from Freescale Semiconductor.
*
* USER INTERFACE
* --------------
*
* Like most BeeStack sample applications, this application uses the the common
* ASL user interface. The ASL interface has two "modes" to allow the keys, LEDs
* and (optional on NCB) the LCD display to be used for multiple purposes.
* "Configuration" mode includes commands such as starting and leaving the network,
* whereas "application" mode includes application specific functions and display.
*
* Each key (aka switch) can be pressed for a short duration (short press) or
* long duration of about 1+ seconds (long press). Examples include SW1 or LSW3.
*
* Application specific code can be found here. Code common among applications
* can be found in ASL_UserInterface.c
*
* Config Mode:
* SW1  - (ASL) Form/join network (form if ZC, join if ZR or ZED) with previous configuration
* SW2  - (ASL) Toggle Permit Join (ZC/ZR only)
* SW3  - (ASL) Use end-device-bind to bind to another node (e.g. switch to light)
* SW4  - (ASL) Choose channel (default is 25). Only functional when NOT on network.
* LSW1 - (ASL) Toggle display/keyboard mode (Config and Application)
* LSW2 - (ASL) Leave network
* LSW3 - (ASL) Remove all binding
* LSW4 - (ASL) Form/join network (form if ZC, join if ZR or ZED) with new configuration
*
* OnOffSwitch Application Mode:
* SW1  - Toggle local light
* SW2  -
* SW3  - (ASL) Toggle Identify mode on/off (will stay on for 20 seconds)
* SW4  - (ASL) Recall scene(must store scene  first with LSW4)
* LSW1 - (ASL) Toggle display/keyboard mode (Config and Application)
* LSW2 -
* LSW3 - Add any OnOffLights in identify mode to a group
* LSW4 - Add any OnOffLights in identify mode to scene
*****************************************************************************/
#include "EmbeddedTypes.h"
#include "zigbee.h"
#include "BeeStack_Globals.h"
#include "BeeStackConfiguration.h"
#include "BeeStackParameters.h"
#include "AppZdoInterface.h"
#include "TS_Interface.h"
#include "TMR_Interface.h"
#include "AppAfInterface.h"
#include "FunctionLib.h"
#include "PublicConst.h"
#include "EndPointConfig.h"
#include "BeeApp.h"
#include "ZDOStateMachineHandler.h"
#include "ZdoApsInterface.h"
#include "BeeAppInit.h"
#include "NVM_Interface.h"
#include "ZtcInterface.h"
#include "ASL_ZdpInterface.h"
#include "ASL_UserInterface.h"
#include "HaProfile.h"
#include "SEprofile.h"


/******************************************************************************
*******************************************************************************
* Private type definitions
*******************************************************************************
******************************************************************************/

/******************************************************************************
*******************************************************************************
* Private Prototypes
*******************************************************************************
******************************************************************************/
void BeeAppTask(event_t events);
void BeeAppDataIndication(void);
void BeeAppDataConfirm(void);
#if gInterPanCommunicationEnabled_c
void BeeAppInterPanDataConfirm(void);
void BeeAppInterPanDataIndication(void);
#endif 
zbStatus_t APSME_AddGroupRequest(zbApsmeAddGroupReq_t *pRequest);
void OnOffLight_AppSetLightState(zbEndPoint_t endPoint, zclCmd_t command);
void OnOffLight_AddGroups(void);

/******************************************************************************
*******************************************************************************
* Private Memory Declarations
*******************************************************************************
******************************************************************************/

/******************************************************************************
*******************************************************************************
* Public memory declarations
*******************************************************************************
******************************************************************************/

zbEndPoint_t appEndPoint;
zbEndPoint_t appEndPoint9;

/* one of these for each light endpoint */
extern haOnOffLightAttrRAM_t gHaOnOffLightData;
extern zclOnOffLightSceneData_t gHaOnOffLightSceneData;

/* This data set contains app layer variables to be preserved across resets */
NvDataItemDescription_t const gaNvAppDataSet[] = {
  gAPS_DATA_SET_FOR_NVM,    /* APS layer NVM data */
  {&gZclCommonAttr,         sizeof(zclCommonAttr_t)},       /* scenes, location, etc... */
  {&gAslData,               sizeof(ASL_Data_t)},            /* state of ASL */
  {&gHaOnOffLightData,      sizeof(haOnOffLightAttrRAM_t)}, /* state of light */
#if gZclEnableReporting_c
  {&gZclReportingSetup,      sizeof(zclReportingSetup_t)}, /* for attribute reporting */
#endif
  {&gHaOnOffLightSceneData,  sizeof(zclOnOffLightSceneData_t)},
  /* insert any user data for NVM here.... */
  {NULL, 0}       /* Required end-of-table marker. */
};


/******************************************************************************
*******************************************************************************
* Private Functions
*******************************************************************************
******************************************************************************/

/*****************************************************************************
* BeeAppInit
*
* Initialization function for the App Task. This is called during
* initialization and should contain any application specific initialization
* (ie. hardware initialization/setup, table initialization, power up
* notification.
*
*****************************************************************************/
void BeeAppInit
  (
  void
  )
{
  index_t i;

  /* register the application endpoint(s), so we receive callbacks */
  for(i=0; i<gNum_EndPoints_c; ++i) {
    (void)AF_RegisterEndPoint(endPointList[i].pEndpointDesc);
  }

  /* where to send switch commands from */
  appEndPoint = endPointList[0].pEndpointDesc->pSimpleDesc->endPoint;
  appEndPoint9 = endPointList[1].pEndpointDesc->pSimpleDesc->endPoint;
  
  /* initialize common user interface */
  ASL_InitUserInterface("HaOnOffLight_EP");

  /* control of external light (if available) */
  ExternalDeviceInit();

}

/*****************************************************************************
* BeeAppTask
*
* ZigBee Application Task event processor.  This function is called to
* process all events for the task. Events include timers, messages and any
* other user defined events
*
* Note: Event bits 11-15 are reserved by BeeStack. Event bits 0-10 are
* available for the application specific events.
******************************************************************************/
void BeeAppTask
  (
  event_t events    /*IN: event bitmask for the application task */
  )
{
  /* received one or more data confirms */
  if(events & gAppEvtDataConfirm_c)
    BeeAppDataConfirm();

  /* received one or more data indications */
  if(events & gAppEvtDataIndication_c)
    BeeAppDataIndication();
  
#if gInterPanCommunicationEnabled_c
    /* received one or more data confirms */
  if(events & gInterPanAppEvtDataConfirm_c)
    BeeAppInterPanDataConfirm();

  /* received one or more data indications */
  if(events & gInterPanAppEvtDataIndication_c)
    BeeAppInterPanDataIndication();
#endif 

  /* ZCL specific */
  if(events & gAppEvtAddGroup_c)
    ASL_ZclAddGroupHandler();

  if(events & gAppEvtStoreScene_c)
    ASL_ZclStoreSceneHandler();

  if(events & gAppEvtSyncReq_c)
    ASL_Nlme_Sync_req(FALSE);
}

/*****************************************************************************
* BeeAppHandleKeys
*
* Handles all key events for this node. See also KBD_Init().
*
*
* The default keyboard handling uses a model system: a network configuration-mode
* and an application run-mode. Combined with the concepts of short and
* long-press, this gives the application a total of 16 keys on a 4 button system
* (4 buttons * 2 modes * short and long).
*
* Config-mode covers joining and leaving a network, binding and other
* non-application specific keys, and are common across all Freescale applications.
*
* Run-mode covers application specific keys.
*
*****************************************************************************/
void BeeAppHandleKeys
  (
  key_event_t keyCode  /*IN: Events from keyboard module */
  )
{
  /* Application-mode keys */
  if( gmUserInterfaceMode == gApplicationMode_c ) {
    switch ( keyCode ) {
      /* Toggle local light */
      case gKBD_EventSW1_c:
        OnOffLight_AppSetLightState(appEndPoint, gZclCmdOnOff_Toggle_c);
        break;

      /* not used by application */
      case gKBD_EventSW2_c:
    	  OnOffLight_AppSetLightState(appEndPoint9, gZclCmdOnOff_Toggle_c);
        break;

      /* not used by application */
      case gKBD_EventLongSW2_c:
        break;

      /* all other keys handled by ASL */
      default:
        ASL_HandleKeys(keyCode);
        break;
      }
    }
  
  /* if not in application mode, then use default configuration mode key handling */
  else {
  	ASL_HandleKeys(keyCode);
  }
}

/*****************************************************************************
* BeeAppUpdateDevice
*
* Contains application specific
*
*
* The default keyboard handling uses a model system: a network configuration-mode
* and an application run-mode. Combined with the concepts of short and
* long-press, this gives the application a total of 16 keys on a 4 button system
* (4 buttons * 2 modes * short and long).
*
* Config-mode covers joining and leaving a network, binding and other
* non-application specific keys, and are common across all Freescale applications.
*
* Run-mode covers application specific keys.
*
*****************************************************************************/
void BeeAppUpdateDevice
  (
  zbEndPoint_t endPoint,    /* IN: endpoint update happend on */
  zclUIEvent_t event,        /* IN: state to update */
  zclAttrId_t attrId, 
  zbClusterId_t clusterId, 
  void *pData
  )
{
 (void) attrId;
 (void) clusterId;
 (void) pData;
 
 switch (event) {
    case gZclUI_Off_c:
    	if(9 == endPoint)    
    	{
    		ASL_SetLed(LED3,gLedOff_c);
    	}
    	else
		{
    		ASL_SetLed(LED2,gLedOff_c);
		}
    	
      ASL_LCDWriteString("Light Off" );
      ExternalDeviceOff();
      break;

    case gZclUI_On_c:
    	if(9 == endPoint)    
    	{
    		ASL_SetLed(LED3,gLedOn_c);
    	}
    	else
    	{
    		 ASL_SetLed(LED2,gLedOn_c);
    	}
      ASL_LCDWriteString("Light On" );
      ExternalDeviceOn();
      break;
#if gZclEnableReporting_c
    /* Formed/joined network */
    case gZDOToAppMgmtZCRunning_c:
    case gZDOToAppMgmtZRRunning_c:
    case gZDOToAppMgmtZEDRunning_c:

  if(gZclReportingSetup.fReporting == TRUE)
  {
    /* Start reporting timer with the interval period */
    if ((!TMR_IsTimerActive(gZclReportingTimerID)) && (gZclReportingSetup.reportTimeout != 0))
    {
       ZLC_StartReportingTimer();
    }
  }      
#endif      
    /* If the recived event is not any of the others then is a Configure mode event, so it is treated on the
       ASL_UpdateDevice, like: StartNetwork, JoinNetwork, Identify, PermitJoin, etc */
    default:
        ASL_UpdateDevice(endPoint,event);
  }
}

/*****************************************************************************
* OnOffLight_AppSetLightState
*
* Turn on/off the local light. Note: this will end up ultimately calling
* BeeAppUpdateDevice(), but goes through the ZCL engine.
*****************************************************************************/

void OnOffLight_AppSetLightState
  (
  zbEndPoint_t endPoint,    /* IN: APP endpoint */
  zclCmd_t command          /* IN: */
  )
{
  zclOnOffReq_t req;

  /* set up address info for a unicast to ourselves */
  req.addrInfo.dstAddrMode = gZbAddrMode16Bit_c;
  Copy2Bytes(req.addrInfo.dstAddr.aNwkAddr, NlmeGetRequest(gNwkShortAddress_c));
  req.addrInfo.dstEndPoint = endPoint;
  Set2Bytes(req.addrInfo.aClusterId, gZclClusterOnOff_c);
  req.addrInfo.srcEndPoint = endPoint;
  req.addrInfo.txOptions = gZclTxOptions;
  req.addrInfo.radiusCounter = afDefaultRadius_c;

  /* send the command */
  req.command = command;

  (void)ASL_ZclOnOffReq(&req);
}

/*****************************************************************************
  BeeAppDataIndication

  Process incoming ZigBee over-the-air messages.
*****************************************************************************/
void BeeAppDataIndication(void)
{
  apsdeToAfMessage_t *pMsg;
  zbApsdeDataIndication_t *pIndication;
  zbStatus_t status = gZclMfgSpecific_c;

  while(MSG_Pending(&gAppDataIndicationQueue))
  {
    /* Get a message from a queue */
    pMsg = MSG_DeQueue( &gAppDataIndicationQueue );

    /* give ZCL first crack at the frame */
    pIndication = &(pMsg->msgData.dataIndication);
    status = ZCL_InterpretFrame(pIndication);

    /* not handled by ZCL interface, handle cluster here... */
    if(status == gZclMfgSpecific_c)
    {
      /* insert manufacturer specific code here... */	  
	  ZCL_SendDefaultMfgResponse(pIndication);
    }

    /* Free memory allocated by data indication */
    AF_FreeDataIndicationMsg(pMsg);
  }
}

/*****************************************************************************
  BeeAppDataConfirm

  Process incoming ZigBee over-the-air data confirms.
*****************************************************************************/
void BeeAppDataConfirm(void)
{
  apsdeToAfMessage_t *pMsg;
  zbApsdeDataConfirm_t *pConfirm;

  while(MSG_Pending(&gAppDataConfirmQueue))
  {
    /* Get a message from a queue */
    pMsg = MSG_DeQueue( &gAppDataConfirmQueue );
    pConfirm = &(pMsg->msgData.dataConfirm);

    /* Action taken when confirmation is received. */
    if( pConfirm->status != gZbSuccess_c )
    {
      /* The data wasn't delivered -- Handle error code here */
    }
    /* Free memory allocated in Call Back function */
    MSG_Free(pMsg);
  }
}

#if gInterPanCommunicationEnabled_c

/*****************************************************************************
  BeeAppInterPanDataIndication

  Process InterPan incoming ZigBee over-the-air messages.
*****************************************************************************/
void BeeAppInterPanDataIndication(void)
{
  InterPanMessage_t *pMsg;
  zbInterPanDataIndication_t *pIndication;
  zbStatus_t status = gZclMfgSpecific_c;

  while(MSG_Pending(&gInterPanAppDataIndicationQueue))
  {
    /* Get a message from a queue */
    pMsg = MSG_DeQueue( &gInterPanAppDataIndicationQueue );

    /* ask ZCL to handle the frame */
    pIndication = &(pMsg->msgData.InterPandataIndication );
    status = ZCL_InterpretInterPanFrame(pIndication);

    /* not handled by ZCL interface, handle cluster here... */
    if(status == gZclMfgSpecific_c)
    {
      /* insert manufacturer specific code here... */	  
    }

    /* Free memory allocated by data indication */
    MSG_Free(pMsg);
  }
}


/*****************************************************************************
  BeeAppDataConfirm

  Process InterPan incoming ZigBee over-the-air data confirms.
*****************************************************************************/
void BeeAppInterPanDataConfirm
(
void
)
{
  InterPanMessage_t *pMsg;
  zbInterPanDataConfirm_t *pConfirm;
  
  while(MSG_Pending(&gInterPanAppDataConfirmQueue))
  {
    /* Get a message from a queue */
    pMsg = MSG_DeQueue( &gInterPanAppDataConfirmQueue );
    pConfirm = &(pMsg->msgData.InterPandataConf);
    
    /* Action taken when confirmation is received. */
    if( pConfirm->status != gZbSuccess_c )
    {
      /* The data wasn't delivered -- Handle error code here */
    }
    
    /* Free memory allocated in Call Back function */
    MSG_Free(pMsg);
  }
}
#endif 

