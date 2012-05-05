/*****************************************************************************
* BeeApp.c
*
* This application (HaOnOffSwitch) contains the code necessary for an on/off
* light switch. It can control one or more on/off lights.
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
* Configuration mode includes commands such as starting and leaving the network,
* whereas application mode includes application specific functions and display.
*
* Each key (aka switch) can be pressed for a short duration (short press) or
* long duration of about 2+ seconds (long press). Examples include SW1 or LSW3.
*
* Application specific code can be found here. Code common among applications
* can be found in ASL_UserInterface.c
*
* Config Mode:
* SW1  - Form/join network (form if ZC, join if ZR or ZED) with previous configuration
* SW2  - Toggle Permit Join (ZC/ZR only)
* SW3  - Use end-device-bind to bind to another node (e.g. switch to light)
* SW4  - Choose channel (default is 25). Only functional when NOT on network.
* LSW1 - Toggle display/keyboard mode (Config and Application)
* LSW2 - Leave network
* LSW3 - Remove all binding
* LSW4 - Form/join network (form if ZC, join if ZR or ZED) with new configuration
*
* OnOffSwitch Application Mode:
* SW1  - Toggle remote light
* SW2  - Based on the SwitchActions attribute, moves the switch from state 1 to state 2
* SW3  - Toggle Identify mode on/off (will stay on for 20 seconds)
* SW4  - Recall scene(must store scene  first with LSW4)
* LSW1 - Toggle display/keyboard mode (Config and Application)
* LSW2 - Based on the SwitchActions attribute, moves the switch from state 2 to state 1
* LSW3 - Add any OnOffLights in identify mode to a group
* LSW4 - Add any OnOffLights in identify mode to scene
*
*****************************************************************************/
#include "BeeStack_Globals.h"
#include "BeeStackConfiguration.h"
#include "BeeStackParameters.h"
#include "AppZdoInterface.h"
#include "TS_Interface.h"
#include "TMR_Interface.h"
#include "AppAfInterface.h"
#include "FunctionLib.h"
#include "PublicConst.h"
#include "keyboard.h"
#include "Display.h"
#include "led.h"
#include "EndPointConfig.h"
#include "BeeApp.h"
#include "ZdoApsInterface.h"
#include "BeeAppInit.h"
#include "NVM_Interface.h"
#include "BeeStackInterface.h"
#include "HaProfile.h"
#include "ZdpManager.h"
#include "ASL_ZdpInterface.h"
#include "ASL_UserInterface.h"
#include "SEprofile.h"

/******************************************************************************
*******************************************************************************
* Private type definitions
*******************************************************************************
******************************************************************************/
#define ticksof10Sec 10000

/* Events for the user application, to be use for the BeeAppUpdateDevice */
enum {
	gOffEvent_c,
	gOnEvent_c,
	gToggleEvent_c
};

/******************************************************************************
*******************************************************************************
* Private Prototypes
*******************************************************************************
******************************************************************************/
void BeeAppTask(event_t events);

void OnOffSwitch_StoreSceneHandler(void);

/******************************************************************************
*******************************************************************************
* Private Memory Declarations
*******************************************************************************
******************************************************************************/

zbEndPoint_t appEndPoint;
zbEndPoint_t appEndPoint10;

/******************************************************************************
*******************************************************************************
* Public memory declarations
*******************************************************************************
******************************************************************************/

/* This data set contains app layer variables to be preserved across resets */
NvDataItemDescription_t const gaNvAppDataSet[] = {
  gAPS_DATA_SET_FOR_NVM,    /* APS layer NVM data */
  {&gZclCommonAttr,         sizeof(zclCommonAttr_t)},       /* scenes, location, etc... */
  {&gAslData,               sizeof(ASL_Data_t)},            /* state of ASL */
  /* insert any user data for NVM here.... */
  {NULL, 0}       /* Required end-of-table marker. */
};

uint8_t lastCmd; /* Last sent On/Off/Toggle command */

/******************************************************************************
*******************************************************************************
* Private Functions
*******************************************************************************
******************************************************************************/
void BeeAppDataIndication(void);
void BeeAppDataConfirm(void);
#if gInterPanCommunicationEnabled_c
void BeeAppInterPanDataConfirm(void);
void BeeAppInterPanDataIndication(void);
#endif 
void OnOffSwitch_SetLightState(zbAddrMode_t addrMode, zbNwkAddr_t aNwkAddr, zbEndPoint_t dstEndPoint, uint8_t command, zbApsTxOption_t txOptions);

/*****************************************************************************
* BeeAppInit
*
* Initializes the application
*
* Initialization function for the App Task. This is called during
* initialization and should contain any application specific initialization
* (ie. hardware initialization/setup, table initialization, power up
* notification.
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
  appEndPoint10 = endPointList[1].pEndpointDesc->pSimpleDesc->endPoint;
  
  /* start with all LEDs off */
  ASL_InitUserInterface("HaOnOffSwitch_EP");

}

/*****************************************************************************
  BeeAppTask

  Application task. Responds to events for the application.
*****************************************************************************/
void BeeAppTask
  (
  event_t events    /*IN: events for the application task */
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
 
  if(events & gAppEvtAddGroup_c)
    ASL_ZclAddGroupHandler();

  if(events & gAppEvtStoreScene_c)
    ASL_ZclStoreSceneHandler();

  if(events & gAppEvtSyncReq_c)
    ASL_Nlme_Sync_req(FALSE);
}



/*****************************************************************************
  BeeAppGetSwitchCommand
  Return ON/OFF/Toggle command based on the SwitchType attribute

  stateTransition :   0 - transition from state 1 to state 2
                      1 - transition from state 2 to state 1
  Refer to Table 3.44 from ZCL specification.
*****************************************************************************/
zclOnOffCmd_t BeeAppGetSwitchCommand(uint8_t stateTransition)
{
  
  uint8_t len, switchAction;
  uint8_t cmd;
  zbClusterId_t clusterId;
  
  Set2Bytes(clusterId, gZclClusterOnOffCfg_c);

  (void) ZCL_GetAttribute(appEndPoint, clusterId, gZclAttrOnOffSwitchCfg_SwitchAction_c, &switchAction, &len);

  if (switchAction < 2)
  {
    if (0 == stateTransition)
      cmd = 1 - switchAction;
    else if (1 == stateTransition)
      cmd = switchAction;
  }
  else
    cmd = gZclCmdOnOff_Toggle_c;

  return cmd;
}


/*****************************************************************************
  BeeAppHandleKeys

  Handles all key events for this device. Keyboard can be in 2 modes (:

  config-mode: this mode is for setting up the network, joining, forming, etc...
  run-mode: this is for controlling the light.

*****************************************************************************/
void BeeAppHandleKeys
  (
  key_event_t events  /*IN: Events from keyboard modul */
  )
{
	zbNwkAddr_t  aDestAddress = {0x00,0x00};
  zbEndPoint_t EndPoint = 0;

  /* Application-mode keys */
  if( gmUserInterfaceMode == gApplicationMode_c ) {
    /* Getting the communication mode and Address of intrest */
    switch (gSendingNwkData.gAddressMode){
      case gZbAddrModeGroup_c:
     /* Group addressing */
       aDestAddress[0] = gSendingNwkData.aGroupId[0];
       aDestAddress[1] = gSendingNwkData.aGroupId[1];
       break;
      case gZbAddrModeIndirect_c:
    /* Indirect addressing */
       aDestAddress[0] = 0x00;
       aDestAddress[1] = 0x00;
       break;
    case gZbAddrMode16Bit_c:
    /* Direct addressing */
       aDestAddress[0] = gSendingNwkData.NwkAddrOfIntrest[0];
       aDestAddress[1] = gSendingNwkData.NwkAddrOfIntrest[1];
       EndPoint = gSendingNwkData.endPoint;
       break;
    }
    
    
    switch ( events ) {      
    case gKBD_EventSW1_c: /* Sends a Toggle command to the light */
      lastCmd = gZclCmdOnOff_Toggle_c;
      OnOffSwitch_SetLightState(gSendingNwkData.gAddressMode,aDestAddress,EndPoint,lastCmd, 0);          
      break;
    case gKBD_EventSW2_c: /* Sends a On command to the light with acknoledge */       
      if ((gSendingNwkData.gAddressMode == gZbAddrModeIndirect_c) || (gSendingNwkData.gAddressMode == gZbAddrMode16Bit_c))
      {
        lastCmd = BeeAppGetSwitchCommand(0);
        OnOffSwitch_SetLightState(gSendingNwkData.gAddressMode,aDestAddress,EndPoint, lastCmd, gApsTxOptionAckTx_c);
      }
      break;
    /* SW3-> Toggle Identify Mode, SW4-> Recall Scene */
    /*LongSW1-> Sets the Keys and Display in Configuration mode */
    /* Sends an Off command to the light  with acknoledge*/
    case gKBD_EventLongSW2_c:        
      if ((gSendingNwkData.gAddressMode == gZbAddrModeIndirect_c) || (gSendingNwkData.gAddressMode == gZbAddrMode16Bit_c))
      {
        lastCmd = BeeAppGetSwitchCommand(1);
        OnOffSwitch_SetLightState(gSendingNwkData.gAddressMode,aDestAddress,EndPoint, lastCmd, gApsTxOptionAckTx_c);          
      }
      break;
    case gKBD_EventSW4_c: /* Sends a Toggle command to the light */
        lastCmd = gZclCmdOnOff_Toggle_c;
        OnOffSwitch_SetLightState(gSendingNwkData.gAddressMode,aDestAddress,appEndPoint10,lastCmd, 0);          
      break;
    /* LongSW3-> Toggle Identify Mode, Long SW4-> Recall Scene */
    default: /* Sw3, Sw4, LongSw1, LongS3 and LongS4 cases */
      ASL_HandleKeys(events);
      break;
    }     
  }
  else 
  {
    /* If you are not on Appication Mode, then call ASL_HandleKeys, to be eable to use the Configure Mode */
    ASL_HandleKeys(events);
  }
}

/****************************************************************************/
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

  switch (event){
    case gOnEvent_c: /* On Event, */      
      ASL_SetLed(LED2,gLedOn_c);
      break;
    case gOffEvent_c: /* Off Event */      
      ASL_SetLed(LED2,gLedOff_c);  
      break;
    case gToggleEvent_c: /* Toggle Event */
      ASL_LCDWriteString("Confirm Received" );
      if (gAppModeDisplay.Leds & LED2)
        ASL_SetLed(LED2,gLedOff_c);
      else
        ASL_SetLed(LED2,gLedOn_c);
    break;

    default:
      /* If the recived event is not any of the others then is a Configure mode event, so it is treated on the
      ASL_UpdateDevice, like: StartNetwork, JoinNetwork, Identify, PermitJoin, etc */
    	ASL_UpdateDevice(endPoint,event);
	}

}

/*****************************************************************************
  BeeAppDataIndication

  Process incoming ZigBee over-the-air messages.
*****************************************************************************/
void BeeAppDataIndication
  (
  void
  )
{
  apsdeToAfMessage_t *pMsg;
  zbApsdeDataIndication_t *pIndication;
  zbStatus_t status;

  while(MSG_Pending(&gAppDataIndicationQueue))
  {
    /* Get a message from a queue */
    pMsg = MSG_DeQueue( &gAppDataIndicationQueue );

    /* ask ZCL to handle the frame */
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
void BeeAppDataConfirm
  (
  void
  )
{
  apsdeToAfMessage_t *pMsg;
  zbApsdeDataConfirm_t *pConfirm;

  while(MSG_Pending(&gAppDataConfirmQueue))
  {
    /* Get a message from a queue */
    pMsg = MSG_DeQueue( &gAppDataConfirmQueue );
    pConfirm = &(pMsg->msgData.dataConfirm);

    /* Action taken when confirmation is received. */
    if( pConfirm->status == gZbSuccess_c )
    {     /* if confirm is successfull */
       BeeAppUpdateDevice(appEndPoint, lastCmd,0,0,NULL);
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

/*****************************************************************************
* OnOffSwitch_SetLightState
*
* Sets the remote light state (on/off/toggle). Use address mode to control
* which light(s) to control:
*
* addrMode                 Meaning
* gZbAddrModeIndirect_c    unicast command to a set of bound lights
* gZbAddrModeGroup_c       broadcast command to a group of lights
* gZbAddrMode16Bit_c       unicast command to a single light
*****************************************************************************/
void OnOffSwitch_SetLightState
  (
  zbAddrMode_t  addrMode,     /* IN: address mode (indirect, group, 16bit */
  zbNwkAddr_t   aNwkAddr,     /* IN: nwk address or group */
  zbEndPoint_t  dstEndPoint,  /* IN: APP endpoint */
  uint8_t command,            /* IN: command (on/off/toggle) */
  zbApsTxOption_t txOptions   /* IN: send with (or without) end-to-end acknowledement */
  )
{
  zclOnOffReq_t req;

  /* set up address info for a broadcast */
  req.addrInfo.dstAddrMode = addrMode;
  Copy2Bytes(req.addrInfo.dstAddr.aNwkAddr, aNwkAddr);
  req.addrInfo.dstEndPoint = dstEndPoint;
  Set2Bytes(req.addrInfo.aClusterId, gZclClusterOnOff_c);
  req.addrInfo.srcEndPoint = appEndPoint;
  req.addrInfo.txOptions = txOptions;
  req.addrInfo.radiusCounter = afDefaultRadius_c;

  /* send the command */
  req.command = command;

  (void)ASL_ZclOnOffReq(&req);
}

