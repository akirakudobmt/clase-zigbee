/******************************************************************************
* ZclZtc.c
*
* This module contains BeeStack specific code for interacting with ZCL/HA
* applications through a serial interface, called ZigBee Test Client (ZTC).
* The PC-side application is called Test Tool.
*
* Copyright (c) 2007, Freescale, Inc.  All rights reserved.
*
*
* No part of this document must be reproduced in any form - including copied,
* transcribed, printed or by any electronic means - without specific written
* permission from Freescale Semiconductor.
*
* Documents used in this specification:
* [R1] - 053520r16ZB_HA_PTG-Home-Automation-Profile.pdf
* [R2] - 075123r00ZB_AFG-ZigBee_Cluster_Library_Specification.pdf
*******************************************************************************/
#include "EmbeddedTypes.h"
#include "zigbee.h"
#include "FunctionLib.h"
#include "BeeStackConfiguration.h"
#include "BeeStack_Globals.h"
#include "AppAfInterface.h"
#include "AfApsInterface.h"
#include "BeeStackInterface.h"
#include "ZtcInterface.h"

#include "ASL_ZclInterface.h"
#include "HaProfile.h"
#include "ZclZtc.h"
#include "SEProfile.h"
#include "zclSE.h"
#include "ZclProtocInterf.h"



/******************************************************************************
*******************************************************************************
* Private macros & prototypes
*******************************************************************************
******************************************************************************/


/******************************************************************************
*******************************************************************************
* Public Funcitons
*******************************************************************************
******************************************************************************/

void InitZtcForZcl(void)
{
  /* register with ZTC */
  ZTC_RegisterAppInterfaceToTestClient(ZclReceiveZtcMessage);
}

/*
	ZclReceiveZtcMessage

	Receives a message from ZTC (Test Tool). Used to control the ZCL and HA
	application through the serial port.
*/
void ZclReceiveZtcMessage
(
	ZTCMessage_t* pMsg    /* IN: message from ZTC/UART (Test Tool) */
)
{
	uint8_t opCode;
	zbStatus_t status;
	zclAnyReq_t *pAnyReq;

	/* ignore invalid opcodes */
	if(pMsg->opCode != gHaZtcOpCodeGroup_c)
		return;

	/* opcodes */
	opCode = pMsg->opCodeId;
	status = gZclUnsupportedGeneralCommand_c;
	pAnyReq = (void *)pMsg->data;
	switch(opCode)
	{
		/* read one or more attributes */
		case gHaZtcOpCode_ReadAttr_c:
			status = ZCL_ReadAttrReq(&(pAnyReq->readAttrReq));
		  break;

		/* write one or more attributes */
		case gHaZtcOpCode_WriteAttr_c:
			status = ZCL_WriteAttrReq(&(pAnyReq->writeAttrReq));
		  break;

		/* Configure reporting */
		case gHaZtcOpCode_CfgReporting_c:
			status = ZCL_ConfigureReportingReq(&(pAnyReq->cfgReporting));
		  break;

#ifdef gHcGenericApp_d          
#if gHcGenericApp_d
    case gGt_AdvertiseProtocolAddr_c:
      status = GenericTunnel_AdvertiseProtocolAddress(&(pAnyReq->Gt_AdvertiseProtoAddr));
      break;
    case gGt_MatchProtocolAddr_c:
      status = GenericTunnel_MatchProtocolAddress(&(pAnyReq->Gt_MatchProtoAddr));
      break;
    case gGt_MatchProtocolAddrRes_c:
      status = GenericTunnel_MatchProtocolAddressResponse(&(pAnyReq->Gt_MatchProtoAddrRes));
      break;
    case g11073_TransferApdu_c:
      status = IEEE11073ProtocolTunnel_TransferApdu(&(pAnyReq->Ieee11073_TrsApdu));
      break;      
    case g11073_ConnectRequest_c:
      status = IEEE11073ProtocolTunnel_ConnectRequest(&(pAnyReq->Ieee11073_ConnectRequest));
      break;      
    case g11073_DisconnectRequest_c:
      status = IEEE11073ProtocolTunnel_DisconnectRequest(&(pAnyReq->Ieee11073_DisconnectRequest));
      break;
    case g11073_ConnectStatusNotif_c: 
      {
        zclCmd11073_ConnectStatusNotif_t* statusNotif = &(pAnyReq->Ieee11073_ConnectStatusNotif);
        if (statusNotif->connectStatus == zcl11073Status_Disconnected)
          status = IEEE11073ProtocolTunnel_InternalDisconnectReq(&(pAnyReq->Ieee11073_ConnectStatusNotif));
        else
          status = IEEE11073ProtocolTunnel_ConnectStatusNotif(&(pAnyReq->Ieee11073_ConnectStatusNotif));
      }
      break;    
    case g11073_GetIEEE11073MessageProcStatus_c:
      status = IEEE11073ProtocolTunnel_GetIEEE11073MessageProcStatus();
      break;        
#if gZclEnablePartition_d        
	  case g11073_SetPartitionThreshold_c:
	    status =  IEEE11073ProtocolTunnel_SetPartitionThreshold(pAnyReq->Ieee11073_SetPartitionThreshold);
	    break;
#endif	    
#endif
#endif

#ifdef gZclEnablePartition_d    
#if (gZclEnablePartition_d == TRUE)
		case gPartitionZtcOpCode_ReadHandshakeParamReq_c:
			status = ZCL_PartitionReadHandshakeParamReq(&(pAnyReq->partitionReadHandshakeParamReq));
		  break;    
		case gPartitionZtcOpCode_WriteHandshakeParamReq_c:
			status = ZCL_PartitionWriteHandshakeParamReq(&(pAnyReq->partitionWriteHandshakeParamReq));
		  break;    		  
        case gPartitionZtcOpCode_RegisterOutgoingFrame_c:
			status = ZCL_PartitionRegisterTxFrame(&(pAnyReq->partitionClusterFrameInfo));
		  break;   
		case gPartitionZtcOpCode_RegisterIncomingFrame_c:
			status = ZCL_PartitionRegisterRxFrame(&(pAnyReq->partitionClusterFrameInfo));
		  break;   		  
	  case gPartitionZtcOpCode_SetDefaultAttrs_c:
	    status =  ZCL_PartitionSetDefaultAttrs(&(pAnyReq->partitionSetDefaultAttrsReq));
	    break;		  
#endif
#endif

#if gASL_ZclGroupAddGroupReq_d                  
    /* add group request (same for add group and add group if identifying) */
    /* use command 0 in request for add, 5 for add if identifying */
    case gHaZtcOpCode_GroupCmd_AddGroup_c:
      status = ASL_ZclGroupAddGroupReq(&(pAnyReq->addGroupReq));
      break;

    /* Sends a AddGroupIfIdentifying command*/
    case gHaZtcOpCode_GroupCmd_AddGroupIfIdentifying_c:
			status = ASL_ZclGroupAddGroupIfIdentifyReq(&(pAnyReq->addGroupIfIdentifyReq));
      break;
#endif       

#if gASL_ZclGroupViewGroupReq_d      
    /* Sends a ViewGroup command*/
    case gHaZtcOpCode_GroupCmd_ViewGroup_c:
      status = ASL_ZclGroupViewGroupReq(&(pAnyReq->viewGroupReq));
      break;
#endif      


#if gASL_ZclGetGroupMembershipReq_d      
    /* Sends a GetGroupMembership command*/
    case gHaZtcOpCode_GroupCmd_GetGroupMembership_c:
      status = ASL_ZclGetGroupMembershipReq(&(pAnyReq->getGroupMembershipReq));
      break;
#endif      

#if gASL_ZclRemoveGroupReq_d      
    /* Sends a RemoveGroup command*/
    case gHaZtcOpCode_GroupCmd_RemoveGroup_c:
      status = ASL_ZclRemoveGroupReq(&(pAnyReq->removeGroupReq));
      break;
#endif      

#if gASL_ZclRemoveAllGroupsReq_d      
    /* remove all groups */
    case gHaZtcOpCode_GroupCmd_RemoveAllGroups_c:
      status = ASL_ZclRemoveAllGroupsReq(&(pAnyReq->removeAllGroupsReq));
      break;
#endif      

#if gASL_ZclSceneAddSceneReq_d      
    /* Sends a AddScene command */
    case gHaZtcOpCode_SceneCmd_AddScene_c:
			status = ASL_ZclSceneAddSceneReq(&(pAnyReq->addSceneReq), pMsg->length - sizeof(pAnyReq->addSceneReq.addrInfo));
			break;
#endif                        

#if gASL_ZclViewSceneReq_d                        
    /* Sends a ViewScene command */
		case gHaZtcOpCode_SceneCmd_ViewScene_c:
			status = ASL_ZclViewSceneReq(&(pAnyReq->viewSceneReq));
			break;
#endif
                       

#if gASL_ZclRemoveSceneReq_d                        
    /* Sends a RemoveScene command */
    case gHaZtcOpCode_SceneCmd_RemoveScene_c:
			status = ASL_ZclRemoveSceneReq(&(pAnyReq->removeSceneReq));
			break;
#endif                        

                        
#if gASL_ZclRemoveAllScenesReq_d                        
    /* Sends a AddScene command */
    case gHaZtcOpCode_SceneCmd_RemoveAllScenes_c:
			status = ASL_ZclRemoveAllScenesReq(&(pAnyReq->removeAllScenesReq));
			break;
#endif
                       
#if gASL_ZclStoreSceneReq_d                        
    /* Sends a StoreScene command */
    case gHaZtcOpCode_SceneCmd_StoreScene_c:
			status = ASL_ZclStoreSceneReq(&(pAnyReq->storeSceneReq));
			break;
#endif                        

#if gASL_ZclRecallSceneReq_d                        
    /* Sends a RecallScene command */
		case gHaZtcOpCode_SceneCmd_RecallScene_c:
			status = ASL_ZclRecallSceneReq(&(pAnyReq->recallSceneReq));
			break;
#endif
                        
#if gASL_ZclGetSceneMembershipReq_d                        
    /* Sends a GetScene Membership command */
    case gHaZtcOpCode_SceneCmd_GetSceneMembership_c:
			status = ASL_ZclGetSceneMembershipReq(&(pAnyReq->getSceneMembershipreq));
			break;
#endif                        

#if gASL_ZclOnOffReq_d                        
    /* Send the on/off command */
    case gHaZtcOpCode_OnOffCmd_Off_c:
      status = ASL_ZclOnOffReq(&(pAnyReq->onOffReq));
      break;
#endif
      
#if gASL_ZclSetWeeklyScheduleReq                        
    /* Send the SetWeeklySchedule command */
    case gHaZtcOpCode_Thermostat_SetWeeklyScheduleReq:
      status = ZCL_ThermostatSetWeeklyScheduleReq(&(pAnyReq->SetWeeklyScheduleReq));
      break;
#endif
      
#if gASL_ZclGetWeeklyScheduleReq                        
    /* Send the GetWeeklySchedule command */
    case gHaZtcOpCode_Thermostat_GetWeeklyScheduleReq:
      status = ZCL_ThermostatGetWeeklyScheduleReq(&(pAnyReq->GetWeeklyScheduleReq));
      break;
#endif
      
#if gASL_ZclClearWeeklyScheduleReq                        
    /* Send the ClearWeeklySchedule command */
    case gHaZtcOpCode_Thermostat_ClearWeeklyScheduleReq:
      status = ZCL_ThermostatClearWeeklyScheduleReq(&(pAnyReq->ClearWeeklyScheduleReq));
      break;
#endif
      
#if gASL_ZclSetpointRaiseLowerReq                        
    /* Send the ClearWeeklySchedule command */
    case gHaZtcOpCode_Thermostat_SetpointRaiseLowerReq:
      status = ZCL_ThermostatSetpointRaiseLowerReq(&(pAnyReq->SetpointRaiseLowerReq));
      break;
#endif

#if gASL_ZclIdentifyReq_d      
    /* Send the identify command */
	  case gHaZtcOpCode_IdentifyCmd_Identify_c: 
	    status = ASL_ZclIdentifyReq(&(pAnyReq->identifyReq));
	    break;	    
#endif   
            
#if gASL_ZclAlarms_ResetAlarm_d
    case gZclZtcOpCode_Alarms_ResetAlarm_c:
      status = Alarms_ResetAlarm(&(pAnyReq->resetAlarmReq));
      break;        
#endif
	    
#if gASL_ZclAlarms_ResetAllAlarms_d      
    case gZclZtcOpCode_Alarms_ResetAllAlarms_c:
      status = Alarms_ResetAllAlarms(&(pAnyReq->resetAllAlarmsReq));
      break;        
#endif
	    
#if gASL_ZclAlarms_Alarm_d      
    case gZclZtcOpCode_Alarms_Alarm_c:
      status = Alarms_Alarm(&(pAnyReq->alarmReq));
      break;        
#endif
	    
#if gASL_ZclIdentifyQueryReq_d            
	  /* Send the identify query command */
	  case gHaZtcOpCode_IdentifyCmd_IdentifyQuery_c:
	    status = ASL_ZclIdentifyQueryReq(&(pAnyReq->identifyQueryReq));
	    break;		  
#endif            

#if gZclDiscoverAttrReq_d            
	  /* Send the discover attributes command */
	  case gHaZtcOpCode_DiscoverAttr_c:
	    status = ZCL_DiscoverAttrReq(&(pAnyReq->discoverAttrReq));
	    break;		  	    
#endif            
/*******************************************************************/	  
#if gASL_ZclCommissioningRestartDeviceRequest_d
	  case gHaZtcOpCode_CommissioningRestartDeviceRequest_c:
	    status = ZCL_Commisioning_RestartDeviceReq( (zclCommissioningRestartDeviceReq_t*) &(pAnyReq->RestartDeviceReq));
	    break;		  	    
#endif	  
#if gASL_ZclCommissioningSaveStartupParametersRequest_d            
	  case gHaZtcOpCode_CommissioningSaveStartupParametersRequest_c:
	    status = ZCL_Commisioning_SaveStartupParametersReq((zclCommissioningSaveStartupParametersReq_t *) &(pAnyReq->SaveStartupParameterReq));
	    break;		  	    
#endif
#if gASL_ZclCommissioningRestoreStartupParametersRequest_d
	  case gHaZtcOpCode_CommissioningRestoreStartupParametersRequest_c:
	    status = ZCL_Commisioning_RestoreStartupParametersReq((zclCommissioningRestoreStartupParametersReq_t*) &(pAnyReq->RestoreStartupParameterReq));
	    break;		  	    
#endif
#if gASL_ZclCommissioningResetStartupParametersRequest_d            
	  case gHaZtcOpCode_CommissioningResetStartupParametersRequest_c:
	    status = ZCL_Commisioning_ResetStartupParametersReq((zclCommissioningResetStartupParametersReq_t *) &(pAnyReq->ResetStartupParameterReq));
	    break;		 
#endif	    
/*******************************************************************/	             
#if gASL_ZclDisplayMsgReq_d
        case gAmiZtcOpCode_Msg_DisplayMsgReq_c:
	    status = ZclMsg_DisplayMsgReq((zclDisplayMsgReq_t *) &(pAnyReq->DisplayMsgReq));
	    break;	
#endif
#if gASL_ZclCancelMsgReq_d            
	  case gAmiZtcOpCode_Msg_CancelMsgReq_c:
	    status = ZclMsg_CancelMsgReq((zclCancelMsgReq_t *) &(pAnyReq->CancelMsgReq));
	    break;
#endif 
#if gASL_ZclGetLastMsgReq_d            
	  case gAmiZtcOpCode_Msg_GetLastMsgRequest_c:
	    status = ZclMsg_GetLastMsgReq((zclGetLastMsgReq_t *) &(pAnyReq->GetLastMsgReq));
	    break;
#endif
#if gASL_ZclMsgConfReq_d            
	  case gAmiZtcOpCode_Msg_MsgConfReq:
	    status = ZclMsg_MsgConf((zclMsgConfReq_t *) &(pAnyReq->MsgConfReq));
	    break;
#endif
            
#if gInterPanCommunicationEnabled_c    
#if gASL_ZclInterPanDisplayMsgReq_d
        case gAmiZtcOpCode_Msg_InterPanDisplayMsgReq_c:
	    status = ZclMsg_InterPanDisplayMsgReq((zclInterPanDisplayMsgReq_t *) &(pAnyReq->InterPanDisplayMsgReq));
	    break;	
#endif
#if gASL_ZclInterPanCancelMsgReq_d            
	  case gAmiZtcOpCode_Msg_InterPanCancelMsgReq_c:
	    status = ZclMsg_InterPanCancelMsgReq((zclInterPanCancelMsgReq_t *) &(pAnyReq->InterPanCancelMsgReq));
	    break;
#endif 
#if gASL_ZclInterPanGetLastMsgReq_d            
	  case gAmiZtcOpCode_Msg_InterPanGetLastMsgRequest_c:
	    status = ZclMsg_InterPanGetLastMsgReq((zclInterPanGetLastMsgReq_t *) &(pAnyReq->InterPanGetLastMsgReq));
	    break;
#endif
#if gASL_ZclMsgConfReq_d            
	  case gAmiZtcOpCode_Msg_InterPanMsgConfReq:
	    status = ZclMsg_InterPanMsgConf((zclInterPanMsgConfReq_t *) &(pAnyReq->InterPanMsgConfReq));
	    break;
#endif
#endif /*#if gInterPanCommunicationEnabled_c    */            
            
#if gASL_ZclSmplMet_GetProfReq_d            
	  case gAmiZtcOpCode_SmplMet_GetProfReq_c:
	    status = ZclSmplMet_GetProfReq((zclSmplMet_GetProfReq_t *) &(pAnyReq->SmplMet_GetProfReq));
	    break;
#endif            
#if gASL_ZclSmplMet_GetProfRsp_d
	  case gAmiZtcOpCode_SmplMet_GetProfRsp_c:
	    status = ZclSmplMet_GetProfRsp((zclSmplMet_GetProfRsp_t *) &(pAnyReq->SmplMet_GetProfRsp));
	    break;	              
#endif
#if gZclFastPollMode_d
#if gASL_ZclSmplMet_FastPollModeReq_d
	  case gAmiZtcOpCode_SmplMet_FastPollModeReq_c:
	    status = ZclSmplMet_ReqFastPollModeReq((zclSmplMet_ReqFastPollModeReq_t *) &(pAnyReq->SmplMet_GetProfRsp));
	    break;	              
#endif
#if gASL_ZclSmplMet_FastPollModeRsp_d
	  case gAmiZtcOpCode_SmplMet_FastPollModeRsp_c:
	    status = ZclSmplMet_ReqFastPollModeRsp((zclSmplMet_ReqFastPollModeRsp_t *) &(pAnyReq->SmplMet_GetProfRsp));
	    break;	              
#endif
#endif
#if gASL_ZclSmplMet_UpdateMetStatus_d
	  case gAmiZtcOpCode_SmplMet_UpdateMetStatus_c:
	    status = ZclSmplMet_UpdateMetStatus((zclSmplMet_UpdateMetStatus_t *)pAnyReq);
	    break;	              
#endif
#if gASL_ZclDmndRspLdCtrl_ReportEvtStatus_d            
	  case gAmiZtcOpCode_DmndRspLdCtrl_ReportEvtStatusReq_c:
	    status = ZclDmndRspLdCtrl_ReportEvtStatus((zclDmndRspLdCtrl_ReportEvtStatus_t *) &(pAnyReq->DmndRspLdCtrl_ReportEvtStatus));
	    break;
#endif
#if gASL_ZclDmndRspLdCtrl_GetScheduledEvtsReq_d            
	  case gAmiZtcOpCode_DmndRspLdCtrl_GetScheduledEvtsReq_c:
	    status = ZclDmndRspLdCtrl_GetScheduledEvtsReq((zclDmndRspLdCtrl_GetScheduledEvts_t *) &(pAnyReq->DmndRspLdCtrl_GetScheduledEvts));
	    break;
#endif 
#if gASL_ZclDmndRspLdCtrl_LdCtrlEvtReq_d            
	  case gAmiZtcOpCode_DmndRspLdCtrl_LdCtrlEvtReq_c:
	    status = ZclDmndRspLdCtrl_LdCtrlEvtReq((zclDmndRspLdCtrl_LdCtrlEvtReq_t *) &(pAnyReq->DmndRspLdCtrl_LdCtrlEvtReq));
	    break;
#endif
#if gASL_ZclDmndRspLdCtrl_CancelLdCtrlEvtReq_d            
	  case gAmiZtcOpCode_DmndRspLdCtrl_CancelLdCtrlEvtReq_c:
	    status = ZclDmndRspLdCtrl_CancelLdCtrlEvtReq((zclDmndRspLdCtrl_CancelLdCtrlEvtReq_t *) &(pAnyReq->DmndRspLdCtrl_CancelLdCtrlEvtReq));
	    break;
#endif
#if gASL_ZclDmndRspLdCtrl_CancelAllLdCtrlEvtsReq_d            
	  case gAmiZtcOpCode_DmndRspLdCtrl_CancelAllLdCtrlEvtReq_c:
	    status = ZclDmndRspLdCtrl_CancelAllLdCtrlEvtReq((zclDmndRspLdCtrl_CancelAllLdCtrlEvtsReq_t *) &(pAnyReq->DmndRspLdCtrl_CancelAllLdCtrlEvtsReq));
	    break;
#endif
#if gASL_ZclDmndRspLdCtrl_ScheduledServerEvts_d            
	  case gAmiZtcOpCode_DmndRspLdCtrl_ScheduleServerEvt_c:
	    status = ZCL_ScheduleServerLdCtrlEvents((zclCmdDmndRspLdCtrl_LdCtrlEvtReq_t *)pAnyReq);
	    break;
#endif              
#if gASL_ZclPrice_GetCurrPriceReq_d            
	  case gAmiZtcOpCode_Price_GetCurrPriceReq_c:
	    status = zclPrice_GetCurrPriceReq((zclPrice_GetCurrPriceReq_t *) &(pAnyReq->Price_GetCurrPriceReq));
	    break;
#endif
#if gASL_ZclPrice_GetSheduledPricesReq_d            
	  case gAmiZtcOpCode_Price_GetSheduledPricesReq_c:
	    status = zclPrice_GetScheduledPricesReq((zclPrice_GetScheduledPricesReq_t *) &(pAnyReq->Price_GetSheduledPricesReq));
	    break;
#endif 
#if gASL_ZclPrice_PublishPriceRsp_d            
	  case gAmiZtcOpCode_Price_PublishPriceRsp_c:
	    status = zclPrice_PublishPriceRsp((zclPrice_PublishPriceRsp_t *) &(pAnyReq->Price_PublishPriceRsp));
	    break;
#endif    
            
#if gASL_ZclPrice_PublishPriceRsp_d            
	  case gAmiZtcOpCode_Price_ScheduleServerPrice_c:
	    status = ZCL_ScheduleServerPriceEvents((zclCmdPrice_PublishPriceRsp_t *)pAnyReq);
	    break;
#endif   

#if gASL_ZclPrice_PublishPriceRsp_d            
	  case gAmiZtcOpCode_Price_UpdateServerPrice_c:
	    status = ZCL_UpdateServerPriceEvents((zclCmdPrice_PublishPriceRsp_t *)pAnyReq);
	    break;
#endif   

#if gASL_ZclPrice_PublishPriceRsp_d            
	  case gAmiZtcOpCode_Price_DeleteServerScheduledPrices_c:
	       ZCL_DeleteServerScheduledPrices();
               status = gZclSuccess_c;
	    break;
#endif               
#if gInterPanCommunicationEnabled_c    
#if gASL_ZclPrice_InterPanGetCurrPriceReq_d            
          case gAmiZtcOpCode_Price_InterPanGetCurrPriceReq_c:
                status = zclPrice_InterPanGetCurrPriceReq((zclPrice_InterPanGetCurrPriceReq_t *) &(pAnyReq->InterPanGetCurrPriceReq));
            break;
#endif
#if gASL_ZclPrice_InterPanGetSheduledPricesReq_d    
          case gAmiZtcOpCode_Price_InterPanGetSheduledPricesReq_c:
                status = zclPrice_InterPanGetScheduledPricesReq((zclPrice_InterPanGetScheduledPricesReq_t *) &(pAnyReq->InterPanGetScheduledPricesReq));
           break;
#endif 
#if gASL_ZclPrice_InterPanPublishPriceRsp_d    
          case gAmiZtcOpCode_Price_InterPanPublishPriceRsp_c:
                status = zclPrice_InterPanPublishPriceRsp((zclPrice_InterPanPublishPriceRsp_t *) &(pAnyReq->InterPanPublishPriceRsp));
          break;    
#endif    
#endif
#if gASL_ZclPrepayment_SelAvailEmergCreditReq_d
	  case gAmiZtcOpCode_Prepayment_SelAvailEmergCreditReq_c:
	    status = zclPrepayment_Client_SelAvailEmergCreditReq((zclPrepayment_SelAvailEmergCreditReq_t *) &(pAnyReq->SmplMet_GetProfRsp));
	    break;	              
#endif
#if gASL_ZclPrepayment_ChangeSupplyReq_d
	  case gAmiZtcOpCode_Prepayment_ChangeSupplyReq_c:
	    status = zclPrepayment_Client_ChangeSupplyReq((zclPrepayment_ChangeSupplyReq_t *) &(pAnyReq->SmplMet_GetProfRsp));
	    break;
#endif
#if gASL_ZclKeyEstab_InitKeyEstabReq_d    
	  case gAmiZtcOpCode_KeyEstab_InitKeyEstabReq_c:
	    status = zclKeyEstab_InitKeyEstabReq((ZclKeyEstab_InitKeyEstabReq_t *) &(pAnyReq->KeyEstab_InitKeyEstabReq));
	    break;	  
#endif
#if gASL_ZclKeyEstab_EphemeralDataReq_d            
	  case gAmiZtcOpCode_KeyEstab_EphemeralDataReq_c:
	    status = zclKeyEstab_EphemeralDataReq((ZclKeyEstab_EphemeralDataReq_t *) &(pAnyReq->KeyEstab_EphemeralDataReq));
	    break;	  	  
#endif
#if gASL_ZclKeyEstab_ConfirmKeyDataReq_d            
	  case gAmiZtcOpCode_KeyEstab_ConfirmKeyDataReq_c:
	    status = zclKeyEstab_ConfirmKeyDataReq((ZclKeyEstab_ConfirmKeyDataReq_t *) &(pAnyReq->KeyEstab_ConfirmKeyDataReq));
	    break;	   
#endif            
#if gASL_ZclKeyEstab_TerminateKeyEstabServer_d            
	  case gAmiZtcOpCode_KeyEstab_TerminateKeyEstabServer_c:
	    status = zclKeyEstab_TerminateKeyEstabServer((ZclKeyEstab_TerminateKeyEstabServer_t *) &(pAnyReq->KeyEstab_TerminateKeyEstabServer));
	    break;	  
#endif
#if gASL_ZclKeyEstab_InitKeyEstabRsp_d            
	  case gAmiZtcOpCode_KeyEstab_InitKeyEstabRsp_c:
	    status = zclKeyEstab_InitKeyEstabRsp((ZclKeyEstab_InitKeyEstabRsp_t *) &(pAnyReq->KeyEstab_InitKeyEstabRsp));
	    break;	  	  
#endif
#if gASL_ZclKeyEstab_EphemeralDataRsp_d            
	  case gAmiZtcOpCode_KeyEstab_EphemeralDataRsp_c:
	    status = zclKeyEstab_EphemeralDataRsp((ZclKeyEstab_EphemeralDataRsp_t *) &(pAnyReq->KeyEstab_EphemeralDataRsp));
	    break;
#endif
#if gASL_ZclKeyEstab_ConfirmKeyDataRsp_d            
	  case gAmiZtcOpCode_KeyEstab_ConfirmKeyDataRsp_c:
	    status = zclKeyEstab_ConfirmKeyDataRsp((ZclKeyEstab_ConfirmKeyDataRsp_t *) &(pAnyReq->KeyEstab_ConfirmKeyDataRsp));
	    break;	  	  
#endif 
#if gASL_ZclKeyEstab_TerminateKeyEstabClient_d            
	  case gAmiZtcOpCode_KeyEstab_TerminateKeyEstabClient_c:
	    status = zclKeyEstab_TerminateKeyEstabClient((ZclKeyEstab_TerminateKeyEstabClient_t *) &(pAnyReq->KeyEstab_TerminateKeyEstabClient));
	    break;	  
#endif	  
#if gASL_ZclSE_RegisterDevice_d
        case gAmiZtcOpCode_SE_RegisterDeviceReq_c:
            if (pSE_ESPRegFunc != NULL)
              status = (*pSE_ESPRegFunc)((EspRegisterDevice_t*)pAnyReq);
          break;
        case gAmiZtcOpCode_SE_DeRegisterDeviceReq_c:
            if (pSE_ESPDeRegFunc != NULL)
              status = (*pSE_ESPDeRegFunc)((EspDeRegisterDevice_t*)pAnyReq);
          break;
#endif            
#if gASL_ZclSE_InitTime_d
        case gGeneralZtcOpCode_IniTimeReq_c:
          ZCL_TimeInit((ZCLTimeConf_t *) pAnyReq);
        break;
#endif          

#if gZclEnableOTAServer_d:
    case gOTAImageNotify_c:
      ZCL_OTAImageNotify((zclZtcOTAImageNotify_t *)pAnyReq);
    break;
    case gOTAQueryNextImageResponse_c:
      ZCL_OTANextImageResponse((zclZtcOTANextImageResponse_t *)pAnyReq);
      break;
    case gOTABlockResponse_c:
      ZCL_OTABlockResponse((zclZtcOTABlockResponse_t *)pAnyReq);
      break;
  case gOTAUpgradeEndResponse_c:
      ZCL_OTAUpgradeEndResponse((ztcZclOTAUpgdareEndResponse_t *)pAnyReq);
      break;
#endif	

#if gZclEnableOTAClient_d:	
    case gOTASetClientParams_c:
      ZCL_OTASetClientParams((zclOTAClientParams_t *)pAnyReq);
    break;
    case gOTABlockRequest_c:
      ZCL_OTABlockRequest((zclZtcOTABlockRequest_t *)pAnyReq);
      break;
#endif
        
#if gASL_ZclLevelControlReq_d          
    default:
      /* Send level cluster commands; they have a count of 8 and are similar, so 
       * they are processed based on opcode difference from MoveToLevel */
      if (opCode >= gHaZtcOpCode_LevelControlCmd_MoveToLevel_c && opCode <= gHaZtcOpCode_LevelControlCmd_StopOnOff_c)
        status = ASL_ZclLevelControlReq(&(pAnyReq->levelControlReq), gZclCmdLevelControl_MoveToLevel_c + (opCode - gHaZtcOpCode_LevelControlCmd_MoveToLevel_c));
      break;
#endif      
      	    
  }
  

  /* done, send confirm */
  /* ZTC data req */
#ifndef gHostApp_d  
  ZTCQueue_QueueToTestClient(&status, gHaZtcOpCodeGroup_c, opCode, sizeof(status));
#else
  ZTCQueue_QueueToTestClient(gpHostAppUart, &status, gHaZtcOpCodeGroup_c, opCode, sizeof(status));
#endif
}

