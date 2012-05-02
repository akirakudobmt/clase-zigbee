/******************************************************************************
* ZclOptions.h
*
* Include this file after BeeOptions.h
*
* (c) Copyright 2008, Freescale, Inc.  All rights reserved.
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
#ifndef _ZCLOPTIONS_H
#define _ZCLOPTIONS_H

/* default 16 scenes per device */
#ifndef gHaMaxScenes_c
#define gHaMaxScenes_c    2
#endif

/* The maximum size in bytes for the storable scene */
/* The Scene data array:
              -> For the OnOff scenes it only needs 1 byte (0x01).
              -> For the Dimmer light it only needs 11 bytes (0x0B).
              -> For the Thermostat it needs the gHaMaxSceneSize_c
                 which is 45 bytes (0x2D).
*/
#ifndef gHaMaxSceneSize_c
#define gHaMaxSceneSize_c    45
#endif

/* Enable optionals to enable all optional clusters/attributes */
#ifndef gZclClusterOptionals_d
#define gZclClusterOptionals_d    FALSE
#endif

/* Enable to include scene name in scene table */
#ifndef gZclIncludeSceneName_d
#define gZclIncludeSceneName_d    FALSE
#endif

/* Foundation commands */


/* enable reporting of attributes (saves about 1.4K ROM) */
#ifndef gZclEnableReporting_c
#define gZclEnableReporting_c     FALSE
#endif

/* enable long attr types Reporting */
#ifndef gZclEnableOver32BitAttrsReporting_c
#define gZclEnableOver32BitAttrsReporting_c     FALSE
#endif

/* enable long string types handling */
#ifndef gZclEnableLongStringTypes_c
#define gZclEnableLongStringTypes_c     FALSE
#endif


/* enable Discover Attributes request */
#ifndef gZclDiscoverAttrReq_d
#define gZclDiscoverAttrReq_d   FALSE
#endif

/* enable reply to Discover Attributes */
#ifndef gZclDiscoverAttrRsp_d
#define gZclDiscoverAttrRsp_d   FALSE
#endif

/* enable for Partition Cluster */
#ifndef gZclEnablePartition_d
#define gZclEnablePartition_d    FALSE
#endif


/* enable for Over the Air  (OTA)  Upgrade Cluster Server */
#ifndef gZclEnableOTAServer_d
#define gZclEnableOTAServer_d    FALSE
#endif

/* enable for Over the Air  (OTA)  Upgrade Cluster Client */
#ifndef gZclEnableOTAClient_d
#define gZclEnableOTAClient_d    FALSE
#endif

/***************************************
  Simple Metering Cluster
****************************************/

#ifndef gZclFastPollMode_d
#define gZclFastPollMode_d FALSE
#endif

#ifndef gZclFastPollUpdatePeriod_d
#define gZclFastPollUpdatePeriod_d 5
#endif

/***************************************
  Key Establishment Cluster
****************************************/

#ifndef gZclAcceptLongCert_d
#define gZclAcceptLongCert_d  TRUE
#endif

#ifndef gZclKeyEstabDebugMode_d
#define gZclKeyEstabDebugMode_d FALSE
#endif

#ifndef gZclKeyEstabDebugTimer_d
#define gZclKeyEstabDebugTimer_d FALSE
#endif

#ifndef gZclKeyEstabDebugTimeInterval_d
#define gZclKeyEstabDebugTimeInterval_d 240000
#endif

#ifndef gZclKeyEstabDebugInhibitInitRsp_d
#define gZclKeyEstabDebugInhibitInitRsp_d FALSE
#endif

#ifndef gZclSeSecurityCheck_d
#define gZclSeSecurityCheck_d FALSE
#endif
/***************************************
  Basic Cluster Defaults
****************************************/

/* Basic Cluster Attribute Defaults, for setting by the OEM */

/* SW version of the application (set by OEM) */
#ifndef gZclAttrBasic_ApplicationVersion_c
#define gZclAttrBasic_ApplicationVersion_c  0x01
#endif

/* HW version of the board (set by OEM) */
#ifndef gZclAttrBasic_HWVersion_c
#define gZclAttrBasic_HWVersion_c           0x03
#endif

/* Power Source - 0x01 = mains, see gZclAttrBasicPowerSource_Mains_c */
#ifndef gZclAttrBasic_PowerSource_c
#define gZclAttrBasic_PowerSource_c         0x01
#endif

/* up to 32 characters, manufacturers name */
#ifndef gszZclAttrBasic_MfgName_c           
#define gszZclAttrBasic_MfgName_c           "Freescale"
#endif

/* up to 32 characters, model ID (any set of characters) */
#ifndef gszZclAttrBasic_Model_c             
#define gszZclAttrBasic_Model_c             "0001"
#endif

/* up to 16 characters (date code, must be in yyyymmdd format */
#ifndef gszZclAttrBasic_DateCode_c          
#define gszZclAttrBasic_DateCode_c          "20070128"
#endif

/*--------------------------------------- ZCL Definitions ----------------------------------------*/

/* Identify Cluster */
#ifndef gASL_ZclIdentifyQueryReq_d
#define gASL_ZclIdentifyQueryReq_d    TRUE
#endif

#ifndef gASL_ZclIdentifyReq_d
#define gASL_ZclIdentifyReq_d   TRUE 
#endif

/* Groups Cluster */
#ifndef gASL_ZclGroupAddGroupReq_d
#define gASL_ZclGroupAddGroupReq_d    TRUE
#endif

#ifndef gASL_ZclGroupViewGroupReq_d
#define gASL_ZclGroupViewGroupReq_d    TRUE
#endif

#ifndef gASL_ZclGetGroupMembershipReq_d
#define gASL_ZclGetGroupMembershipReq_d    TRUE
#endif

#ifndef gASL_ZclRemoveGroupReq_d
#define gASL_ZclRemoveGroupReq_d    TRUE
#endif

#ifndef gASL_ZclRemoveAllGroupsReq_d
#define gASL_ZclRemoveAllGroupsReq_d    TRUE
#endif

#ifndef gASL_ZclStoreSceneReq_d
#define gASL_ZclStoreSceneReq_d    TRUE
#endif

#ifndef gASL_ZclRecallSceneReq_d
#define gASL_ZclRecallSceneReq_d    TRUE
#endif

#ifndef gASL_ZclGetSceneMembershipReq_d
#define gASL_ZclGetSceneMembershipReq_d    TRUE
#endif

#ifndef gASL_ZclOnOffReq_d
#define gASL_ZclOnOffReq_d    TRUE
#endif

/* Thermostat cluster commands */

#ifndef gASL_ZclSetWeeklyScheduleReq
#define gASL_ZclSetWeeklyScheduleReq    TRUE
#endif

#ifndef gASL_ZclGetWeeklyScheduleReq
#define gASL_ZclGetWeeklyScheduleReq    TRUE
#endif

#ifndef gASL_ZclClearWeeklyScheduleReq
#define gASL_ZclClearWeeklyScheduleReq    TRUE
#endif

#ifndef gASL_ZclSetpointRaiseLowerReq
#define gASL_ZclSetpointRaiseLowerReq    TRUE
#endif

#ifndef gASL_ZclLevelControlReq_d
#define gASL_ZclLevelControlReq_d    TRUE
#endif

#ifndef gASL_ZclSceneAddSceneReq_d
#define gASL_ZclSceneAddSceneReq_d    TRUE
#endif

#ifndef gASL_ZclViewSceneReq_d
#define gASL_ZclViewSceneReq_d    TRUE
#endif

#ifndef gASL_ZclRemoveSceneReq_d
#define gASL_ZclRemoveSceneReq_d    TRUE
#endif

#ifndef gASL_ZclRemoveAllScenesReq_d
#define gASL_ZclRemoveAllScenesReq_d    TRUE
#endif

#ifndef gASL_ZclStoreSceneHandler_d
#define gASL_ZclStoreSceneHandler_d    TRUE
#endif

#ifndef gASL_ZclAddGroupHandler_d
#define gASL_ZclAddGroupHandler_d    TRUE
#endif

#ifndef gASL_ZclCommissioningRestartDeviceRequest_d
#define gASL_ZclCommissioningRestartDeviceRequest_d FALSE
#endif

#ifndef gASL_ZclCommissioningSaveStartupParametersRequest_d
#define gASL_ZclCommissioningSaveStartupParametersRequest_d FALSE
#endif

#ifndef gASL_ZclCommissioningRestoreStartupParametersRequest_d
#define gASL_ZclCommissioningRestoreStartupParametersRequest_d FALSE
#endif

#ifndef gASL_ZclCommissioningResetStartupParametersRequest_d
#define gASL_ZclCommissioningResetStartupParametersRequest_d FALSE
#endif

/* Alarms cluster commands */

#ifndef gASL_ZclAlarms_ResetAlarm_d
#define gASL_ZclAlarms_ResetAlarm_d    TRUE
#endif

#ifndef gASL_ZclAlarms_ResetAllAlarms_d
#define gASL_ZclAlarms_ResetAllAlarms_d    TRUE
#endif

#ifndef gASL_ZclAlarms_Alarm_d
#define gASL_ZclAlarms_Alarm_d    TRUE
#endif

/* SE commands and responses */

#ifndef gASL_ZclDisplayMsgReq_d
#define gASL_ZclDisplayMsgReq_d FALSE
#endif

#ifndef gASL_ZclCancelMsgReq_d
#define gASL_ZclCancelMsgReq_d FALSE
#endif

#ifndef gASL_ZclGetLastMsgReq_d
#define gASL_ZclGetLastMsgReq_d FALSE
#endif

#ifndef gASL_ZclMsgConfReq_d
#define gASL_ZclMsgConfReq_d FALSE
#endif

#ifndef gASL_ZclInterPanDisplayMsgReq_d
#define gASL_ZclInterPanDisplayMsgReq_d FALSE
#endif

#ifndef gASL_ZclInterPanCancelMsgReq_d
#define gASL_ZclInterPanCancelMsgReq_d FALSE
#endif

#ifndef gASL_ZclInterPanGetLastMsgReq_d
#define gASL_ZclInterPanGetLastMsgReq_d FALSE
#endif

#ifndef gASL_ZclInterPanMsgConfReq_d
#define gASL_ZclInterPanMsgConfReq_d FALSE
#endif

#ifndef gASL_ZclSmplMet_Optionals_d
#define gASL_ZclSmplMet_Optionals_d TRUE
#endif

#ifndef gASL_ZclSmplMet_GetProfReq_d
#define gASL_ZclSmplMet_GetProfReq_d FALSE
#endif

#ifndef gASL_ZclSmplMet_GetProfRsp_d
#define gASL_ZclSmplMet_GetProfRsp_d FALSE
#endif

#ifndef gASL_ZclSmplMet_FastPollModeReq_d
#define gASL_ZclSmplMet_FastPollModeReq_d FALSE
#endif

#ifndef gASL_ZclSmplMet_FastPollModeRsp_d
#define gASL_ZclSmplMet_FastPollModeRsp_d FALSE
#endif

#ifndef gASL_ZclSmplMet_UpdateMetStatus_d
#define gASL_ZclSmplMet_UpdateMetStatus_d FALSE
#endif

#ifndef gASL_ZclDmndRspLdCtrl_ReportEvtStatus_d
#define gASL_ZclDmndRspLdCtrl_ReportEvtStatus_d FALSE
#endif

#ifndef gASL_ZclDmndRspLdCtrl_GetScheduledEvtsReq_d
#define gASL_ZclDmndRspLdCtrl_GetScheduledEvtsReq_d FALSE
#endif

#ifndef gASL_ZclDmndRspLdCtrl_ScheduledServerEvts_d
#define gASL_ZclDmndRspLdCtrl_ScheduledServerEvts_d FALSE
#endif

#ifndef gASL_ZclDmndRspLdCtrl_LdCtrlEvtReq_d
#define gASL_ZclDmndRspLdCtrl_LdCtrlEvtReq_d FALSE
#endif

#ifndef gASL_ZclDmndRspLdCtrl_CancelLdCtrlEvtReq_d
#define gASL_ZclDmndRspLdCtrl_CancelLdCtrlEvtReq_d FALSE
#endif

#ifndef gASL_ZclDmndRspLdCtrl_CancelAllLdCtrlEvtsReq_d
#define gASL_ZclDmndRspLdCtrl_CancelAllLdCtrlEvtsReq_d FALSE
#endif

#ifndef gASL_ZclPrice_GetCurrPriceReq_d
#define gASL_ZclPrice_GetCurrPriceReq_d FALSE
#endif

#ifndef gASL_ZclPrice_TiersNumber_d
#define gASL_ZclPrice_TiersNumber_d 7
#endif

#ifndef gASL_ZclPrice_BlockThresholdNumber_d
#define gASL_ZclPrice_BlockThresholdNumber_d 15//max 15
#endif

#ifndef gASL_ZclPrice_BlockPeriodNumber_d
#define gASL_ZclPrice_BlockPeriodNumber_d 4 //max 4
#endif

#ifndef gASL_ZclPrice_CommodityTypeNumber_d
#define gASL_ZclPrice_CommodityTypeNumber_d 2 //max 2
#endif

#ifndef gASL_ZclPrice_BlockPriceInfoNumber_d
#define gASL_ZclPrice_BlockPriceInfoNumber_d 20
#endif

#ifndef gASL_ZclPrice_GetSheduledPricesReq_d
#define gASL_ZclPrice_GetSheduledPricesReq_d FALSE
#endif

#ifndef gASL_ZclPrice_PublishPriceRsp_d
#define gASL_ZclPrice_PublishPriceRsp_d FALSE
#endif

#ifndef gASL_ZclPrice_InterPanGetCurrPriceReq_d
#define gASL_ZclPrice_InterPanGetCurrPriceReq_d FALSE
#endif

#ifndef gASL_ZclPrice_InterPanGetSheduledPricesReq_d
#define gASL_ZclPrice_InterPanGetSheduledPricesReq_d FALSE
#endif

#ifndef gASL_ZclPrice_InterPanPublishPriceRsp_d
#define gASL_ZclPrice_InterPanPublishPriceRsp_d FALSE
#endif

#ifndef gASL_ZclSETunneling_d
#define gASL_ZclSETunneling_d FALSE
#endif

#ifndef gASL_ZclSETunnelingOptionals_d
#define gASL_ZclSETunnelingOptionals_d FALSE
#endif

#ifndef gASL_ZclPrepayment_d
#define gASL_ZclPrepayment_d FALSE
#endif

#ifndef gASL_ZclPrepayment_SelAvailEmergCreditReq_d
#define gASL_ZclPrepayment_SelAvailEmergCreditReq_d FALSE
#endif

#ifndef gASL_ZclPrepayment_ChangeSupplyReq_d
#define gASL_ZclPrepayment_ChangeSupplyReq_d FALSE
#endif

#ifndef gASL_ZclKeyEstab_InitKeyEstabReq_d
#define gASL_ZclKeyEstab_InitKeyEstabReq_d FALSE
#endif

#ifndef gASL_ZclKeyEstab_EphemeralDataReq_d
#define gASL_ZclKeyEstab_EphemeralDataReq_d FALSE
#endif

#ifndef gASL_ZclKeyEstab_ConfirmKeyDataReq_d
#define gASL_ZclKeyEstab_ConfirmKeyDataReq_d FALSE
#endif

#ifndef gASL_ZclKeyEstab_TerminateKeyEstabServer_d
#define gASL_ZclKeyEstab_TerminateKeyEstabServer_d FALSE
#endif

#ifndef gASL_ZclKeyEstab_InitKeyEstabRsp_d
#define gASL_ZclKeyEstab_InitKeyEstabRsp_d FALSE
#endif

#ifndef gASL_ZclKeyEstab_EphemeralDataRsp_d
#define gASL_ZclKeyEstab_EphemeralDataRsp_d FALSE
#endif

#ifndef gASL_ZclKeyEstab_ConfirmKeyDataRsp_d
#define gASL_ZclKeyEstab_ConfirmKeyDataRsp_d FALSE
#endif

#ifndef gASL_ZclKeyEstab_TerminateKeyEstabClient_d
#define gASL_ZclKeyEstab_TerminateKeyEstabClient_d FALSE
#endif

#ifndef gASL_ZclSE_RegisterDevice_d
#define gASL_ZclSE_RegisterDevice_d FALSE
#endif

#ifndef gASL_ZclSE_DeRegisterDevice_d
#define gASL_ZclSE_DeRegisterDevice_d FALSE
#endif

#ifndef gASL_ZclSE_InitTime_d
#define gASL_ZclSE_InitTime_d FALSE
#endif


/***************************************
  Define the set of clusters used by each device
****************************************/

extern zbApsTxOption_t const gZclTxOptions;

#endif /* _ZCLOPTIONS_H */

