/******************************************************************************
* ZclZtc.h
*
* Definitions for interacting with the ZigBee Test Client (ZTC), allowing
* Freescale Test Tool to control the application.
*
* (c) Copyright 2007, Freescale, Inc.  All rights reserved.
*
*
* No part of this document must be reproduced in any form - including copied,
* transcribed, printed or by any electronic means - without specific written
* permission from  Freescale Semiconductor.
*
* Documents used in this specification:
* [R1] - 053520r16ZB_HA_PTG-Home-Automation-Profile.pdf
* [R2] - 075123r00ZB_AFG-ZigBee_Cluster_Library_Specification.pdf
******************************************************************************/
#ifndef _ZCLZTC_H_
#define _ZCLZTC_H_

#include "ZCL.h"
#include "ZCLGeneral.h"
#include "SEProfile.h"
#include "ZclProtocInterf.h"
#ifdef gZclEnablePartition_d    
#if (gZclEnablePartition_d == TRUE)
#include "ZclPartition.h"
#endif
#endif
#include "ZclOTA.h"

/******************************************************************************
*******************************************************************************
* Public macros & types
*******************************************************************************
******************************************************************************/
void ZclReceiveZtcMessage(ZTCMessage_t* pMsg);
void InitZtcForZcl(void);

/******************************************
	ZTC Interface to HA
*******************************************/

/* only used with ZTC interface (optional) */
#define gHaZtcOpCodeGroup_c                 0x70 /* opcode group used for HA commands/events */

/* general ZCL commands only used with ZTC interface (optional) */
#define gHaZtcOpCode_ReadAttr_c             0x00 /* Read attributes 2.4.1 */
#define gHaZtcOpCode_ReadAttrRsp_c          0x01 /* Read attributes response 2.4.2 (event only) */
#define gHaZtcOpCode_WriteAttr_c            0x02 /* Write attributes 2.4.3 */
#define gHaZtcOpCode_WriteAttrUndivided_c   0x03 /* Write attributes undivided 2.4.4 */
#define gHaZtcOpCode_WriteAttrRsp_c         0x04 /* Write attributes response 2.4.5 (event only) */
#define gHaZtcOpCode_WriteAttrNoRsp_c       0x05 /* Write attributes no response 2.4.6 */
#define gHaZtcOpCode_CfgReporting_c         0x06 /* Configure reporting 2.4.7 */
#define gHaZtcOpCode_CfgReportingRsp_c      0x07 /* Configure reporting response 2.4.8 (event only) */
#define gHaZtcOpCode_ReadReportingCfg_c     0x08 /* Read reporting configuration 2.4.9 */
#define gHaZtcOpCode_ReadReportingCfgRsp_c  0x09 /* Read reporting configuration response 2.4.10 (event only) */
#define gHaZtcOpCode_ReportAttr_c           0x0a /* Report attributes 2.4.11 */
#define gHaZtcOpCode_DefaultRsp_c           0x0b /* Default response 2.4.12 (event only) */
#define gHaZtcOpCode_DiscoverAttr_c         0x0c /* Discover attributes 2.4.13 */
#define gHaZtcOpCode_DiscoverAttrRsp_c      0x0d /* Discover attributes response (event only) 2.4.14 */

/* cluster specific only used with ZTC interface (optional) */
#define gHaZtcOpCode_BasicCmd_Reset_c                   0x10 /* basic cluster */
#define gHaZtcOpCode_IdentifyCmd_Identify_c             0x20 /* go into identify mode for time */
#define gHaZtcOpCode_IdentifyCmd_IdentifyQuery_c        0x21 /* identify query command */
#define gHaZtcOpCode_GroupCmd_AddGroup_c                0x30 /* group cluster commands */
#define gHaZtcOpCode_GroupCmd_ViewGroup_c               0x31
#define gHaZtcOpCode_GroupCmd_GetGroupMembership_c      0x32
#define gHaZtcOpCode_GroupCmd_RemoveGroup_c             0x33
#define gHaZtcOpCode_GroupCmd_RemoveAllGroups_c         0x34
#define gHaZtcOpCode_GroupCmd_AddGroupIfIdentifying_c   0x35

#define gZclZtcOpCode_Alarms_ResetAlarm_c               0x37 /* alarms cluster commands */
#define gZclZtcOpCode_Alarms_ResetAllAlarms_c           0x38
#define gZclZtcOpCode_Alarms_Alarm_c                    0x39

#define gHaZtcOpCode_SceneCmd_AddScene_c                0x40 /* scene cluster commands */
#define gHaZtcOpCode_SceneCmd_ViewScene_c               0x41
#define gHaZtcOpCode_SceneCmd_RemoveScene_c             0x42
#define gHaZtcOpCode_SceneCmd_RemoveAllScenes_c         0x43
#define gHaZtcOpCode_SceneCmd_StoreScene_c              0x44
#define gHaZtcOpCode_SceneCmd_RecallScene_c             0x45
#define gHaZtcOpCode_SceneCmd_GetSceneMembership_c      0x46
#define gHaZtcOpCode_OnOffCmd_Off_c                     0x50 /* on/off/toggle cluster commands */
#define gHaZtcOpCode_Thermostat_SetWeeklyScheduleReq      0x55 /* Thermostat cluster commands */
#define gHaZtcOpCode_Thermostat_GetWeeklyScheduleReq      0x56
#define gHaZtcOpCode_Thermostat_ClearWeeklyScheduleReq    0x57
#define gHaZtcOpCode_Thermostat_SetpointRaiseLowerReq     0x58
#define gHaZtcOpCode_LevelControlCmd_MoveToLevel_c      0x60 /* level control cluster commands */
#define gHaZtcOpCode_LevelControlCmd_Move_c             0x61
#define gHaZtcOpCode_LevelControlCmd_Step_c             0x62
#define gHaZtcOpCode_LevelControlCmd_Stop_c             0x63
#define gHaZtcOpCode_LevelControlCmd_MoveToLevelOnOff_c 0x64 
#define gHaZtcOpCode_LevelControlCmd_MoveOnOff_c        0x65
#define gHaZtcOpCode_LevelControlCmd_StepOnOff_c        0x66
#define gHaZtcOpCode_LevelControlCmd_StopOnOff_c        0x67

#define gHaZtcOpCode_CommissioningRestartDeviceRequest_c 0x70
#define gHaZtcOpCode_CommissioningSaveStartupParametersRequest_c    0x71
#define gHaZtcOpCode_CommissioningRestoreStartupParametersRequest_c 0x72
#define gHaZtcOpCode_CommissioningResetStartupParametersRequest_c   0x73


#define gAmiZtcOpCode_Msg_DisplayMsgReq_c       0x81
#define gAmiZtcOpCode_Msg_CancelMsgReq_c        0x82
#define gAmiZtcOpCode_Msg_GetLastMsgRequest_c   0x83
#define gAmiZtcOpCode_Msg_MsgConfReq            0x84

#define gAmiZtcOpCode_SmplMet_GetProfReq_c      0x85
#define gAmiZtcOpCode_SmplMet_GetProfRsp_c      0x86

#define gAmiZtcOpCode_DmndRspLdCtrl_ReportEvtStatusReq_c    0x87
#define gAmiZtcOpCode_DmndRspLdCtrl_LdCtrlEvtReq_c          0x88
#define gAmiZtcOpCode_DmndRspLdCtrl_CancelLdCtrlEvtReq_c    0x89
#define gAmiZtcOpCode_DmndRspLdCtrl_CancelAllLdCtrlEvtReq_c 0x8a

#define gAmiZtcOpCode_Price_GetCurrPriceReq_c         0x8b
#define gAmiZtcOpCode_Price_GetSheduledPricesReq_c    0x8c
#define gAmiZtcOpCode_Price_PublishPriceRsp_c         0x8d

#define gAmiZtcOpCode_KeyEstab_InitKeyEstabReq_c   0x8e
#define gAmiZtcOpCode_KeyEstab_EphemeralDataReq_c  0x8f
#define gAmiZtcOpCode_KeyEstab_ConfirmKeyDataReq_c 0x90
#define gAmiZtcOpCode_KeyEstab_TerminateKeyEstabServer_c 0x91

#define gAmiZtcOpCode_KeyEstab_InitKeyEstabRsp_c   0x92
#define gAmiZtcOpCode_KeyEstab_EphemeralDataRsp_c  0x93
#define gAmiZtcOpCode_KeyEstab_ConfirmKeyDataRsp_c 0x94
#define gAmiZtcOpCode_KeyEstab_TerminateKeyEstabClient_c 0x95

#define gAmiZtcOpCode_DmndRspLdCtrl_GetScheduledEvtsReq_c 0x96
#define gAmiZtcOpCode_DmndRspLdCtrl_ScheduleServerEvt_c 0x97

#define gAmiZtcOpCode_SmplMet_FastPollModeReq_c         0x98
#define gAmiZtcOpCode_SmplMet_FastPollModeRsp_c         0x99

#define gAmiZtcOpCode_SmplMet_UpdateMetStatus_c         0xa0

#define gAmiZtcOpCode_Msg_InterPanDisplayMsgReq_c       0xa1
#define gAmiZtcOpCode_Msg_InterPanCancelMsgReq_c        0xa2
#define gAmiZtcOpCode_Msg_InterPanGetLastMsgRequest_c   0xa3
#define gAmiZtcOpCode_Msg_InterPanMsgConfReq            0xa4

#define gAmiZtcOpCode_Price_ScheduleServerPrice_c            0xa8
#define gAmiZtcOpCode_Price_UpdateServerPrice_c              0xa9
#define gAmiZtcOpCode_Price_DeleteServerScheduledPrices_c    0xaa

#define gAmiZtcOpCode_Price_InterPanGetCurrPriceReq_c         0xab
#define gAmiZtcOpCode_Price_InterPanGetSheduledPricesReq_c    0xac
#define gAmiZtcOpCode_Price_InterPanPublishPriceRsp_c         0xad
#define gAmiZtcOpCode_SE_RegisterDeviceReq_c                  0xae
#define gAmiZtcOpCode_SE_DeRegisterDeviceReq_c                0xaf
#define gGeneralZtcOpCode_IniTimeReq_c                        0xb0


#define gGt_AdvertiseProtocolAddr_c         0xb1
#define gGt_MatchProtocolAddr_c             0xb2
#define gGt_MatchProtocolAddrRes_c          0xb3
#define g11073_TransferApdu_c               0xb4
#define g11073_ConnectRequest_c             0xb5
#define g11073_DisconnectRequest_c          0xb6
#define g11073_ConnectStatusNotif_c         0xb7
#define g11073_SetPartitionThreshold_c      0xb8
#define g11073_GetIEEE11073MessageProcStatus_c      0xb9



#define gPartitionZtcOpCode_RegisterClusterWithTestDataGenerator_c   0xC8
#define gPartitionZtcOpCode_ConfigureTestDataGenerator_c             0xC9
#define gPartitionZtcOpCode_RegisterClusterWithZtcData_c             0xCA
#define gPartitionZtcOpCode_UnRegisterCluster_c                      0xCB

#define gPartitionZtcOpCode_RegisterOutgoingFrame_c                  0xCC
#define gPartitionZtcOpCode_CancelOutgoingFrame_c                    0xCD
#define gPartitionZtcOpCode_RegisterIncomingFrame_c                  0xCE
#define gPartitionZtcOpCode_CancelIncomingFrame_c                    0xCF

#define gPartitionZtcOpCode_ReadHandshakeParamReq_c                  0xC0
#define gPartitionZtcOpCode_WriteHandshakeParamReq_c                 0xC1

#define gPartitionZtcOpCode_ZtcDataEvent_c                           0xC2
#define gPartitionZtcOpCode_ZtcDataTransfer_c                        0xC3

#define gPartitionZtcOpCode_SetDefaultAttrs_c                        0xC4


#define g11073_TransferMeta_c               0xb6
/* OTA cluster specific */
#define gOTAImageNotify_c                   0xD0
#define gOTAQueryNextImageRequest_c        0xD1
#define gOTAQueryNextImageResponse_c       0xD2
#define gOTAUpgradeEndRequest_c             0xD3
#define gOTAUpgradeEndResponse_c            0xD4
#define gOTASetClientParams_c               0xD5
#define gOTABlockRequest_c                  0xD6
#define gOTABlockResponse_c                 0xD7
/* Prepayment cluster specific */
#define gAmiZtcOpCode_Prepayment_SelAvailEmergCreditReq_c       0xE0
#define gAmiZtcOpCode_Prepayment_ChangeSupplyReq_c              0xE1


#ifdef __IAR_SYSTEMS_ICC__
#pragma pack(1)
#endif
/* union of all requests */
typedef union zclAnyReq_tag
{
  zclReadAttrReq_t                    readAttrReq;
  zclWriteAttrReq_t                   writeAttrReq;
  zclDiscoverAttrReq_t                discoverAttrReq;
  zclOnOffReq_t                       onOffReq;
  zclConfigureReportingReq_t          cfgReporting;
  zclLevelControlReq_t                levelControlReq;
  zclBasicResetReq_t                  basicResetReq;
  zclIdentifyReq_t                    identifyReq;	
  zclIdentifyQueryReq_t               identifyQueryReq;
  zclGroupAddGroupReq_t               addGroupReq;
  zclGroupAddGroupIfIdentifyingReq_t  addGroupIfIdentifyReq;
  zclGroupRemoveGroupReq_t            removeGroupReq;
  zclGroupRemoveAllGroupsReq_t        removeAllGroupsReq;
  zclGroupViewGroupReq_t              viewGroupReq;
  zclGroupGetGroupMembershipReq_t     getGroupMembershipReq;
  zclSceneAddSceneReq_t               addSceneReq;
  zclSceneViewSceneReq_t              viewSceneReq;
  zclSceneRemoveSceneReq_t            removeSceneReq;
  zclSceneRemoveAllScenesReq_t        removeAllScenesReq;
  zclSceneStoreSceneReq_t             storeSceneReq;
  zclSceneRecallSceneReq_t            recallSceneReq;
  zclSceneGetSceneMembershipReq_t     getSceneMembershipreq;
  
  zclAlarmInformation_ResetAlarm_t    resetAlarmReq;
  zclAlarmInformation_Alarm_t         alarmReq;
  zclAlarmInformation_ResetAllAlarms_t resetAllAlarmsReq;
    
  zclCmdCommissiong_RestartDeviceRequest_t RestartDeviceReq;
  zclCmdCommissiong_SaveStartupParametersRequest_t    SaveStartupParameterReq;
  zclCmdCommissiong_RestoreStartupParametersRequest_t RestoreStartupParameterReq;
  zclCmdCommissiong_ResetStartupParametersRequest_t   ResetStartupParameterReq;
  
  zclThermostat_SetWeeklySchedule_t     SetWeeklyScheduleReq;
  zclThermostat_GetWeeklySchedule_t     GetWeeklyScheduleReq;
  zclThermostat_ClearWeeklySchedule_t   ClearWeeklyScheduleReq;
  zclThermostat_SetpointRaiseLower_t    SetpointRaiseLowerReq;
  
  zclDisplayMsgReq_t              DisplayMsgReq;
  zclCancelMsgReq_t               CancelMsgReq;
  zclGetLastMsgReq_t              GetLastMsgReq;
  zclMsgConfReq_t                 MsgConfReq;
  zclInterPanDisplayMsgReq_t      InterPanDisplayMsgReq;
  zclInterPanCancelMsgReq_t       InterPanCancelMsgReq;
  zclInterPanGetLastMsgReq_t      InterPanGetLastMsgReq;
  zclInterPanMsgConfReq_t         InterPanMsgConfReq;  
  zclSmplMet_GetProfReq_t         SmplMet_GetProfReq;
  zclSmplMet_GetProfRsp_t         SmplMet_GetProfRsp;
  zclSmplMet_FastPollModeReq_t    SmplMet_FastPollModeReq;
  zclSmplMet_ReqFastPollModeRsp_t SmplMet_FastPollModeRsp;
  zclDmndRspLdCtrl_ReportEvtStatus_t DmndRspLdCtrl_ReportEvtStatus;
  zclDmndRspLdCtrl_GetScheduledEvts_t DmndRspLdCtrl_GetScheduledEvts;
  zclDmndRspLdCtrl_LdCtrlEvtReq_t DmndRspLdCtrl_LdCtrlEvtReq;
  zclDmndRspLdCtrl_CancelLdCtrlEvtReq_t DmndRspLdCtrl_CancelLdCtrlEvtReq;
  zclDmndRspLdCtrl_CancelAllLdCtrlEvtsReq_t DmndRspLdCtrl_CancelAllLdCtrlEvtsReq;
  zclPrice_GetCurrPriceReq_t       Price_GetCurrPriceReq;
  zclPrice_GetScheduledPricesReq_t Price_GetSheduledPricesReq;
  zclPrice_PublishPriceRsp_t       Price_PublishPriceRsp;
  zclPrice_InterPanPublishPriceRsp_t InterPanPublishPriceRsp;
  zclPrice_InterPanGetScheduledPricesReq_t InterPanGetScheduledPricesReq;
  zclPrice_InterPanGetCurrPriceReq_t InterPanGetCurrPriceReq;
  ZclKeyEstab_InitKeyEstabReq_t    KeyEstab_InitKeyEstabReq;
  ZclKeyEstab_EphemeralDataReq_t   KeyEstab_EphemeralDataReq;
  ZclKeyEstab_ConfirmKeyDataReq_t  KeyEstab_ConfirmKeyDataReq;
  ZclKeyEstab_TerminateKeyEstabServer_t KeyEstab_TerminateKeyEstabServer;
  ZclKeyEstab_InitKeyEstabRsp_t         KeyEstab_InitKeyEstabRsp;
  ZclKeyEstab_EphemeralDataRsp_t        KeyEstab_EphemeralDataRsp;
  ZclKeyEstab_ConfirmKeyDataRsp_t       KeyEstab_ConfirmKeyDataRsp;
  ZclKeyEstab_TerminateKeyEstabClient_t KeyEstab_TerminateKeyEstabClient;
  
  zclCmdGtAdvertiseProtoAddr_t        Gt_AdvertiseProtoAddr;
  zclCmdGtMpa_t                       Gt_MatchProtoAddr;
  zclCmdGtMpaRes_t                    Gt_MatchProtoAddrRes;
  zclCmd11073_TransferApdu_t          Ieee11073_TrsApdu;
  zclCmd11073_ConnectRequest_t        Ieee11073_ConnectRequest;
  zclCmd11073_DisconnectRequest_t     Ieee11073_DisconnectRequest;
  zclCmd11073_ConnectStatusNotif_t    Ieee11073_ConnectStatusNotif;
  uint8_t Ieee11073_SetPartitionThreshold;
  
#ifdef gZclEnablePartition_d    
#if (gZclEnablePartition_d == TRUE)
  zclPartitionCmdReadHandshakeParamReq_t partitionReadHandshakeParamReq;
  zbClusterId_t partitionClusterId;
  zclPartitionedClusterFrameInfo_t partitionClusterFrameInfo;
  zclPartitionWriteHandshakeParamReq_t partitionWriteHandshakeParamReq;
  zclPartitionAttrs_t partitionSetDefaultAttrsReq;
#endif
#endif

#ifdef gZclEnableOTAServer_d
#if (gZclEnableOTAServer_d == TRUE)
  zclZtcOTAImageNotify_t  otaImageNotifyParam;
  zclZtcOTANextImageResponse_t  otaNextImageResponseParams;
#endif
#endif

#ifdef gZclEnableOTAClient_d
#if (gZclEnableOTAClient_d == TRUE)
  zclOTAClientParams_t    otaClientParams;
#endif
#endif

  
} zclAnyReq_t;
#ifdef __IAR_SYSTEMS_ICC__
#pragma pack()
#endif
/******************************************************************************
*******************************************************************************
* Public Prototypes
*******************************************************************************
******************************************************************************/
void InitZtcForZcl(void);

#endif /* _ZCLZTC_H_ */
