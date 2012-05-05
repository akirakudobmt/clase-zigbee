/******************************************************************************
* ZclSE.c
*
* This source file describes Clusters defined in the ZigBee Cluster Library,
* General document. Some or all of these
*
* Copyright (c) 2007, Freescale, Inc. All rights reserved.
*
*
* No part of this document must be reproduced in any form - including copied,
* transcribed, printed or by any electronic means - without specific written
* permission from Freescale Semiconductor.
*
******************************************************************************/
#include "EmbeddedTypes.h"
#include "zigbee.h"
#include "FunctionLib.h"
#include "BeeStackConfiguration.h"
#include "BeeStack_Globals.h"
#include "AppAfInterface.h"
#include "AfApsInterface.h"
#include "BeeStackInterface.h"
#include "ZdoApsInterface.h"
#include "HaProfile.h"
#include "ZCLGeneral.h"
#include "zcl.h"
#include "SEProfile.h"
#include "zclSE.h"
#include "display.h"
#include "eccapi.h"
#include "Led.h"
#include "ZdoApsInterface.h"
#include "ASL_ZdpInterface.h"
#include "ApsMgmtInterface.h"
#include "beeapp.h"
/******************************************************************************
*******************************************************************************
* Private macros
*******************************************************************************
******************************************************************************/

#ifndef gASL_ZclSmplMet_MaxFastPollInterval_d
#define gASL_ZclSmplMet_MaxFastPollInterval_d 900//15min
#endif

/******************************************************************************
*******************************************************************************
* Private prototypes
*******************************************************************************
******************************************************************************/
int ECC_GetRandomDataFunc(unsigned char *buffer, unsigned long sz);
int ECC_HashFunc(unsigned char *digest, unsigned long sz, unsigned char *data);
void ZCL_BuildGetProfResponse(uint8_t IntrvChannel, ZCLTime_t EndTime, uint8_t NumberOfPeriods, zclCmdSmplMet_GetProfRsp_t *pProfRsp);
static Consmp *CalculateConsumptionFrom6ByteArray(uint8_t *pNew, uint8_t *pOld);
static void Add8ByteTo6ByteArray(uint8_t value, uint8_t *pSumm_t);
static void UpdatePowerConsumptionTimerCallBack(tmrTimerID_t tmrID);
static uint8_t FindEmptyEntryInEventsTable(void);
static void AddNewEntry(zclLdCtrl_EventsTableEntry_t *pDst, zclCmdDmndRspLdCtrl_LdCtrlEvtReq_t *pSrc, afAddrInfo_t *pAddrInfo);
static void CheckForSuccessiveEvents(uint8_t msgIndex);
static uint8_t ZCL_ScheduleEvents(afAddrInfo_t *pAddrInfo, zclCmdDmndRspLdCtrl_LdCtrlEvtReq_t *pMsg);
static zbStatus_t ZCL_ProcessCancelLdCtrlEvt(zbApsdeDataIndication_t *pIndication);
static zbStatus_t ZCL_ProcessCancelAllLdCtrlEvtst(zbApsdeDataIndication_t * pIndication);
static zbStatus_t ZCL_ProcessLdCtrlEvt(zbApsdeDataIndication_t *pIndication);
static zbStatus_t ZCL_ProcessGetScheduledEvtsReq(addrInfoType_t *pAddrInfo, zclCmdDmndRspLdCtrl_GetScheduledEvts_t * pGetScheduledEvts);
static void LdCtrlEvtTimerCallback(tmrTimerID_t timerID);
static void LdCtrlJitterTimerCallBack(tmrTimerID_t timerID);

void GetProfileTestTimerCallBack(tmrTimerID_t tmrID);
static Consmp consumptionValue;
static Summ_t LastRISCurrSummDlvrd;
static uint8_t ProfIntrvHead;
static uint8_t ProfIntrvTail;
static zclGetProfEntry_t ProfileIntervalTable[gMaxNumberOfPeriodsDelivered_c];
tmrTimerID_t gUpdatePowerConsumptionTimerID;

/* simple metering cluster*/
static zbStatus_t ZCL_ProcessReqFastPollEvt(zbApsdeDataIndication_t *pIndication);
static ZCLTime_t mcFastPollEndTime = 0;
static uint16_t mcFastPollRemainingTime = 0;
static uint16_t mcTimeInFastPollMode = 0;
static void FastPollTimerCallBack(tmrTimerID_t tmrID);
tmrTimerID_t gFastPollTimerID;

/* struct for message cluster response */
static void MsgDisplayTimerCallBack(tmrTimerID_t tmrID);
static msgAddrInfo_t MsgResponseSourceAddr;
tmrTimerID_t gMsgDisplayTimerID;

/* prepayment cluster */
static zbStatus_t zclPrepayment_Server_SupplyStatusRsp(zclPrepayment_SupplyStatRsp_t *pReq);
static zbStatus_t ZCL_SendSupplyStatusRsp(zbApsdeDataIndication_t *pIndication);
static zbStatus_t ZCL_ProcessChangeSupplyReq(zbApsdeDataIndication_t *pIndication);

/* price cluster */
static zbStatus_t ZCL_ProcessClientPublishPrice(zclCmdPrice_PublishPriceRsp_t *pMsg);
static zbStatus_t ZCL_ProcessGetCurrPriceReq
(
addrInfoType_t *pAddrInfo,
zclCmdPrice_GetCurrPriceReq_t * pGetCurrPrice,
bool_t IsInterPanFlag
);
static zbStatus_t ZCL_ProcessGetScheduledPricesReq
(
addrInfoType_t *pAddrInfo, 
zclCmdPrice_GetScheduledPricesReq_t * pGetScheduledPrice, 
bool_t IsInterPanFlag
);
static zbStatus_t ZCL_SendPublishPrice(addrInfoType_t *pAddrInfo, publishPriceEntry_t *pMsg, bool_t IsInterPanFlag);
static zbStatus_t ZCL_PriceAck(zclPrice_PriceAck_t *pReq);
static zbStatus_t ZCL_SendPriceAck(zbApsdeDataIndication_t *pIndication);
static uint8_t AddPriceInTable(publishPriceEntry_t *pTable, uint8_t len, zclCmdPrice_PublishPriceRsp_t *pMsg);
static uint8_t CheckForPriceUpdate(zclCmdPrice_PublishPriceRsp_t *pMsg, publishPriceEntry_t *pTable, uint8_t len);
static void PrepareInterPanForReply(InterPanAddrInfo_t *pAdrrDest, zbInterPanDataIndication_t *pIndication);
static void RegisterDevForPrices(afAddrInfo_t *pAddrInfo);
#if gInterPanCommunicationEnabled_c
static zbStatus_t ZCL_SendInterPriceAck(zbInterPanDataIndication_t *pIndication);
static zbStatus_t ZCL_InterPriceAck(zclPrice_InterPriceAck_t *pReq);
static void StoreInterPanAddr(InterPanAddrInfo_t *pAddrInfo);
static void InterPanJitterTimerCallBack(tmrTimerID_t timerID);
#endif /* #if gInterPanCommunicationEnabled_c */
static void TimerClientPriceCallBack(tmrTimerID_t timerID);
static zbStatus_t ZCL_ProcessGetCurrPriceReq
(
addrInfoType_t *pAddrInfo,
zclCmdPrice_GetCurrPriceReq_t * pGetCurrPrice,
bool_t IsInterPanFlag
);
/******************************************************************************
*******************************************************************************
* Private type definitions
*******************************************************************************
******************************************************************************/

#define gUpdateConsumption_c  10

/******************************************************************************
*******************************************************************************
* Public memory definitions
*******************************************************************************
******************************************************************************/
// esp table
ESPRegTable_t RegistrationTable[RegistrationTableEntries_c] = {0};
/* Timers ID for Simple Metering */
tmrTimerID_t gGetProfileTestTimerID;

extern uint8_t appEndPoint;
extern uint8_t appEndPoint10;
/************** KEY ESTABLISHMENT CLUSTER ***************************************/
/*****************************************************************************/
/*Variables used by KeyEstab cluster                                         */
/*****************************************************************************/
uint8_t KeyEstabState = KeyEstab_InitState_c;
IdentifyCert_t *pOppositeImplicitCert = NULL;
KeyEstab_KeyData_t *KeyData = NULL;
tmrTimerID_t KeyEstabTimerId = gTmrInvalidTimerID_c;
uint8_t EphemeralDataGenerateTime;
uint8_t ConfirmKeyGenerateTime;

#if gZclKeyEstabDebugTimer_d
tmrTimerID_t KeyEstabDebugTimerId = gTmrInvalidTimerID_c;
zbApsdeDataIndication_t *pKeyEstabDataIndication  = NULL;
#endif

zclAttrKeyEstabServerAttrsRAM_t gZclAttrKeyEstabServerAttrs = { 
{0x01,0x00}
};


const zclAttrDef_t gZclKeyEstabServerAttrDef[] = {
/* Server attributes */
  { gZclAttrKeyEstabSecSuite_c,gZclDataTypeEnum16_c, gZclAttrFlagsInRAM_c | gZclAttrFlagsRdOnly_c, sizeof(uint16_t),(void *) &gZclAttrKeyEstabServerAttrs.SecuritySuite },
};

const zclAttrDefList_t gZclKeyEstabServerAttrDefList = {
  NumberOfElements(gZclKeyEstabServerAttrDef),
  gZclKeyEstabServerAttrDef
};

/**************SIMPLE METERING CLUSTER  ******************************************/
const zclAttrDef_t gaZclSmplMetServerAttrDef[] = {

/* Mandatory SummIntformationSet */ 
  { gZclAttrSmplMetRISCurrSummDlvrd_c, gZclDataTypeUint48_c, gZclAttrFlagsInRAM_c | gZclAttrFlagsRdOnly_c | gZclAttrFlagsReportable_c | gZclAttrFlagsAsynchronous_c, sizeof(Summ_t), (void *)MbrOfs(ZCLSmplMetServerAttrsRAM_t, RISCurrSummDlvrd) },
#if gZclClusterOptionals_d && gASL_ZclSmplMet_Optionals_d
  { gZclAttrSmplMetRISCurrSummRcvd_c,                          gZclDataTypeUint48_c,gZclAttrFlagsInRAM_c|gZclAttrFlagsRdOnly_c,sizeof(Summ_t),(void *) MbrOfs(ZCLSmplMetServerAttrsRAM_t,RISCurrSummRcvd)},
  { gZclAttrSmplMetRISCurrMaxDmndDlvrd_c,                      gZclDataTypeUint48_c,gZclAttrFlagsInRAM_c|gZclAttrFlagsRdOnly_c,sizeof(Summ_t),(void *) MbrOfs(ZCLSmplMetServerAttrsRAM_t,RISCurrSummDlvrd)},
  { gZclAttrSmplMetRISCurrMaxDmndRcvd_c,                       gZclDataTypeUint48_c,gZclAttrFlagsInRAM_c|gZclAttrFlagsRdOnly_c,sizeof(Summ_t),(void *) MbrOfs(ZCLSmplMetServerAttrsRAM_t,RISCurrMaxDmndRcvd)},
  { gZclAttrSmplMetRISDFTSumm_c,                               gZclDataTypeUint48_c,gZclAttrFlagsInRAM_c|gZclAttrFlagsRdOnly_c,sizeof(Summ_t),(void *) MbrOfs(ZCLSmplMetServerAttrsRAM_t,RISDFTSumm)},
  { gZclAttrSmplMetRISDailyFreezeTime_c,                       gZclDataTypeUint16_c,gZclAttrFlagsInRAM_c|gZclAttrFlagsRdOnly_c,sizeof(uint16_t),(void *) MbrOfs(ZCLSmplMetServerAttrsRAM_t,RISDailyFreezeTime)},
  { gZclAttrSmplMetRISPowerFactor_c,                           gZclDataTypeInt8_c,  gZclAttrFlagsInRAM_c|gZclAttrFlagsRdOnly_c,sizeof(uint8_t),(void *) MbrOfs(ZCLSmplMetServerAttrsRAM_t,RISPowerFactor)},
  { gZclAttrSmplMetRISReadingSnapShotTime_c,                   gZclDataTypeUTCTime_c, gZclAttrFlagsInRAM_c|gZclAttrFlagsRdOnly_c,sizeof(ZCLTime_t),(void *) MbrOfs(ZCLSmplMetServerAttrsRAM_t,RISReadingSnapShotTime)},
  { gZclAttrSmplMetRISCurrMaxDmndDlvrdTimer_c,                 gZclDataTypeUTCTime_c, gZclAttrFlagsInRAM_c|gZclAttrFlagsRdOnly_c,sizeof(ZCLTime_t),(void *) MbrOfs(ZCLSmplMetServerAttrsRAM_t,RISCurrMaxDmndDlvrdTimer)},
  { gZclAttrSmplMetRISCurrMaxDmndRcvdTime_c,                   gZclDataTypeUTCTime_c, gZclAttrFlagsInRAM_c|gZclAttrFlagsRdOnly_c,sizeof(ZCLTime_t),(void *) MbrOfs(ZCLSmplMetServerAttrsRAM_t,RISCurrMaxDmndRcvdTime)},
  { gZclAttrSmplMetRISDefaultUpdatePeriod_c,                   gZclDataTypeUint8_c, gZclAttrFlagsInRAM_c|gZclAttrFlagsRdOnly_c,sizeof(uint8_t),(void *) MbrOfs(ZCLSmplMetServerAttrsRAM_t,RISDefaultUpdatePeriod)},
  { gZclAttrSmplMetRISFastPollUpdatePeriod_c,                  gZclDataTypeUint8_c, gZclAttrFlagsInRAM_c|gZclAttrFlagsRdOnly_c,sizeof(uint8_t),(void *) MbrOfs(ZCLSmplMetServerAttrsRAM_t,RISFastPollUpdatePeriod)},
  { gZclAttrSmplMetRISCurrBlockPeriodConsumptionDelivered_c,   gZclDataTypeUint48_c, gZclAttrFlagsInRAM_c|gZclAttrFlagsRdOnly_c,sizeof(Summ_t),(void *) MbrOfs(ZCLSmplMetServerAttrsRAM_t,RISCurrBlockPeriodConsumptionDelivered)},
  { gZclAttrSmplMetRISDailyConsumptionTarget_c,                gZclDataTypeUint24_c, gZclAttrFlagsInRAM_c|gZclAttrFlagsRdOnly_c,sizeof(Consmp),(void *) MbrOfs(ZCLSmplMetServerAttrsRAM_t,RISDailyConsumptionTarget)},
  { gZclAttrSmplMetRISCurrBlock_c,                             gZclDataTypeEnum8_c, gZclAttrFlagsInRAM_c|gZclAttrFlagsRdOnly_c,sizeof(uint8_t),(void *) MbrOfs(ZCLSmplMetServerAttrsRAM_t,RISCurrBlock)},
  { gZclAttrSmplMetRISProfileIntPeriod_c,                      gZclDataTypeEnum8_c, gZclAttrFlagsInRAM_c|gZclAttrFlagsRdOnly_c,sizeof(uint8_t),(void *) MbrOfs(ZCLSmplMetServerAttrsRAM_t,RISProfileIntPeriod)},
  { gZclAttrSmplMetRISIntReadReportingPeriod_c,                gZclDataTypeUint16_c, gZclAttrFlagsInRAM_c|gZclAttrFlagsRdOnly_c,sizeof(uint16_t),(void *) MbrOfs(ZCLSmplMetServerAttrsRAM_t,RISIntReadReportingPeriod)},
  { gZclAttrSmplMetRISPresetReadingTime_c,                     gZclDataTypeUint16_c, gZclAttrFlagsInRAM_c|gZclAttrFlagsRdOnly_c,sizeof(uint16_t),(void *) MbrOfs(ZCLSmplMetServerAttrsRAM_t,RISPresetReadingTime)},
  { gZclAttrSmplMetRISVolumePerReport_c,                       gZclDataTypeUint16_c, gZclAttrFlagsInRAM_c|gZclAttrFlagsRdOnly_c,sizeof(uint16_t),(void *) MbrOfs(ZCLSmplMetServerAttrsRAM_t,RISVolumePerReport)},
  { gZclAttrSmplMetRISFlowRestriction_c,                       gZclDataTypeUint8_c, gZclAttrFlagsInRAM_c|gZclAttrFlagsRdOnly_c,sizeof(uint8_t),(void *) MbrOfs(ZCLSmplMetServerAttrsRAM_t,RISFlowRestrictio)},
  { gZclAttrSmplMetRISSupplyStatus_c,                          gZclDataTypeBitmap8_c, gZclAttrFlagsInRAM_c|gZclAttrFlagsRdOnly_c,sizeof(uint8_t),(void *) MbrOfs(ZCLSmplMetServerAttrsRAM_t,RISSupplyStatus)},
  { gZclAttrSmplMetRISCurrInletEnergyCarrierSummation_c,       gZclDataTypeUint48_c, gZclAttrFlagsInRAM_c|gZclAttrFlagsRdOnly_c,sizeof(Summ_t),(void *) MbrOfs(ZCLSmplMetServerAttrsRAM_t,RISCurrInletEnergyCarrierSummation)},
  { gZclAttrSmplMetRISCurrOutletEnergyCarrierSummation_c,      gZclDataTypeUint48_c, gZclAttrFlagsInRAM_c|gZclAttrFlagsRdOnly_c,sizeof(Summ_t),(void *) MbrOfs(ZCLSmplMetServerAttrsRAM_t,RISCurrOutletEnergyCarrierSummation)},
  { gZclAttrSmplMetRISInletTemp_c,   gZclDataTypeInt16_c,      gZclAttrFlagsInRAM_c|gZclAttrFlagsRdOnly_c,sizeof(int16_t),(void *) MbrOfs(ZCLSmplMetServerAttrsRAM_t,RISInletTemp)},
  { gZclAttrSmplMetRISOutletTemp_c,   gZclDataTypeInt16_c,     gZclAttrFlagsInRAM_c|gZclAttrFlagsRdOnly_c,sizeof(int16_t),(void *) MbrOfs(ZCLSmplMetServerAttrsRAM_t,RISOutletTemp)},
  { gZclAttrSmplMetRISCtrlTemp_c,   gZclDataTypeInt16_c,       gZclAttrFlagsInRAM_c|gZclAttrFlagsRdOnly_c,sizeof(int16_t),(void *) MbrOfs(ZCLSmplMetServerAttrsRAM_t,RISCtrlTemp)},
  { gZclAttrSmplMetRISCurrInletEnergyCarrierDemand_c,          gZclDataTypeInt24_c, gZclAttrFlagsInRAM_c|gZclAttrFlagsRdOnly_c,sizeof(Consmp),(void *) MbrOfs(ZCLSmplMetServerAttrsRAM_t,RISCurrInletEnergyCarrierDemand)},
  { gZclAttrSmplMetRISCurrOutletEnergyCarrierDemand_c,         gZclDataTypeInt24_c, gZclAttrFlagsInRAM_c|gZclAttrFlagsRdOnly_c,sizeof(Consmp),(void *) MbrOfs(ZCLSmplMetServerAttrsRAM_t,RISCurrOutletEnergyCarrierDemand)},
#endif
   /*Attributes of the Simple Metering TOU Information attribute set */
#if gZclClusterOptionals_d && gASL_ZclSmplMet_Optionals_d
  {gZclAttrSmplMetTOUCurrTier1SummDlvrd_c,    gZclDataTypeUint48_c,gZclAttrFlagsInRAM_c|gZclAttrFlagsRdOnly_c,sizeof(Summ_t),(void *) MbrOfs(ZCLSmplMetServerAttrsRAM_t,TOUCurrTier1SummDlvrd)},
  {gZclAttrSmplMetTOUCurrTier1SummRcvd_c,     gZclDataTypeUint48_c,gZclAttrFlagsInRAM_c|gZclAttrFlagsRdOnly_c,sizeof(Summ_t),(void *) MbrOfs(ZCLSmplMetServerAttrsRAM_t,TOUCurrTier1SummRcvd)},
  {gZclAttrSmplMetTOUCurrTier2SummDlvrd_c,    gZclDataTypeUint48_c,gZclAttrFlagsInRAM_c|gZclAttrFlagsRdOnly_c,sizeof(Summ_t),(void *) MbrOfs(ZCLSmplMetServerAttrsRAM_t,TOUCurrTier2SummDlvrd)},
  {gZclAttrSmplMetTOUCurrTier2SummRcvd_c,     gZclDataTypeUint48_c,gZclAttrFlagsInRAM_c|gZclAttrFlagsRdOnly_c,sizeof(Summ_t),(void *) MbrOfs(ZCLSmplMetServerAttrsRAM_t,TOUCurrTier2SummRcvd)},
  {gZclAttrSmplMetTOUCurrTier3SummDlvrd_c,    gZclDataTypeUint48_c,gZclAttrFlagsInRAM_c|gZclAttrFlagsRdOnly_c,sizeof(Summ_t),(void *) MbrOfs(ZCLSmplMetServerAttrsRAM_t,TOUCurrTier3SummDlvrd)},
  {gZclAttrSmplMetTOUCurrTier3SummRcvd_c,     gZclDataTypeUint48_c,gZclAttrFlagsInRAM_c|gZclAttrFlagsRdOnly_c,sizeof(Summ_t),(void *) MbrOfs(ZCLSmplMetServerAttrsRAM_t,TOUCurrTier3SummRcvd)},
  {gZclAttrSmplMetTOUCurrTier4SummDlvrd_c,    gZclDataTypeUint48_c,gZclAttrFlagsInRAM_c|gZclAttrFlagsRdOnly_c,sizeof(Summ_t),(void *) MbrOfs(ZCLSmplMetServerAttrsRAM_t,TOUCurrTier4SummDlvrd)},
  {gZclAttrSmplMetTOUCurrTier4SummRcvd_c,     gZclDataTypeUint48_c,gZclAttrFlagsInRAM_c|gZclAttrFlagsRdOnly_c,sizeof(Summ_t),(void *) MbrOfs(ZCLSmplMetServerAttrsRAM_t,TOUCurrTier4SummRcvd)},
  {gZclAttrSmplMetTOUCurrTier5SummDlvrd_c,    gZclDataTypeUint48_c,gZclAttrFlagsInRAM_c|gZclAttrFlagsRdOnly_c,sizeof(Summ_t),(void *) MbrOfs(ZCLSmplMetServerAttrsRAM_t,TOUCurrTier5SummDlvrd)},
  {gZclAttrSmplMetTOUCurrTier5SummRcvd_c,     gZclDataTypeUint48_c,gZclAttrFlagsInRAM_c|gZclAttrFlagsRdOnly_c,sizeof(Summ_t),(void *) MbrOfs(ZCLSmplMetServerAttrsRAM_t,TOUCurrTier5SummRcvd)},
  {gZclAttrSmplMetTOUCurrTier6SummDlvrd_c,    gZclDataTypeUint48_c,gZclAttrFlagsInRAM_c|gZclAttrFlagsRdOnly_c,sizeof(Summ_t),(void *) MbrOfs(ZCLSmplMetServerAttrsRAM_t,TOUCurrTier6SummDlvrd)},
  {gZclAttrSmplMetTOUCurrTier6SummRcvd_c,     gZclDataTypeUint48_c,gZclAttrFlagsInRAM_c|gZclAttrFlagsRdOnly_c,sizeof(Summ_t),(void *) MbrOfs(ZCLSmplMetServerAttrsRAM_t,TOUCurrTier6SummRcvd)},
  {gZclAttrSmplMetTOUCurrTier7SummDlvrd_c,    gZclDataTypeUint48_c,gZclAttrFlagsInRAM_c|gZclAttrFlagsRdOnly_c,sizeof(Summ_t),(void *) MbrOfs(ZCLSmplMetServerAttrsRAM_t,TOUCurrTier7SummDlvrd)},
  {gZclAttrSmplMetTOUCurrTier7SummRcvd_c,     gZclDataTypeUint48_c,gZclAttrFlagsInRAM_c|gZclAttrFlagsRdOnly_c,sizeof(Summ_t),(void *) MbrOfs(ZCLSmplMetServerAttrsRAM_t,TOUCurrTier7SummRcvd)},
  {gZclAttrSmplMetTOUCurrTier8SummDlvrd_c,    gZclDataTypeUint48_c,gZclAttrFlagsInRAM_c|gZclAttrFlagsRdOnly_c,sizeof(Summ_t),(void *) MbrOfs(ZCLSmplMetServerAttrsRAM_t,TOUCurrTier8SummDlvrd)},
  {gZclAttrSmplMetTOUCurrTier8SummRcvd_c,     gZclDataTypeUint48_c,gZclAttrFlagsInRAM_c|gZclAttrFlagsRdOnly_c,sizeof(Summ_t),(void *) MbrOfs(ZCLSmplMetServerAttrsRAM_t,TOUCurrTier8SummRcvd)},
  {gZclAttrSmplMetTOUCurrTier9SummDlvrd_c,    gZclDataTypeUint48_c,gZclAttrFlagsInRAM_c|gZclAttrFlagsRdOnly_c,sizeof(Summ_t),(void *) MbrOfs(ZCLSmplMetServerAttrsRAM_t,TOUCurrTier9SummDlvrd)},
  {gZclAttrSmplMetTOUCurrTier9SummRcvd_c,     gZclDataTypeUint48_c,gZclAttrFlagsInRAM_c|gZclAttrFlagsRdOnly_c,sizeof(Summ_t),(void *) MbrOfs(ZCLSmplMetServerAttrsRAM_t,TOUCurrTier9SummRcvd)},
  {gZclAttrSmplMetTOUCurrTier10SummDlvrd_c,   gZclDataTypeUint48_c,gZclAttrFlagsInRAM_c|gZclAttrFlagsRdOnly_c,sizeof(Summ_t),(void *) MbrOfs(ZCLSmplMetServerAttrsRAM_t,TOUCurrTier10SummDlvrd)},
  {gZclAttrSmplMetTOUCurrTier10SummRcvd_c,    gZclDataTypeUint48_c,gZclAttrFlagsInRAM_c|gZclAttrFlagsRdOnly_c,sizeof(Summ_t),(void *) MbrOfs(ZCLSmplMetServerAttrsRAM_t,TOUCurrTier10SummRcvd)},
  {gZclAttrSmplMetTOUCurrTier11SummDlvrd_c,   gZclDataTypeUint48_c,gZclAttrFlagsInRAM_c|gZclAttrFlagsRdOnly_c,sizeof(Summ_t),(void *) MbrOfs(ZCLSmplMetServerAttrsRAM_t,TOUCurrTier11SummDlvrd)},
  {gZclAttrSmplMetTOUCurrTier11SummRcvd_c,    gZclDataTypeUint48_c,gZclAttrFlagsInRAM_c|gZclAttrFlagsRdOnly_c,sizeof(Summ_t),(void *) MbrOfs(ZCLSmplMetServerAttrsRAM_t,TOUCurrTier11SummRcvd)},
  {gZclAttrSmplMetTOUCurrTier12SummDlvrd_c,   gZclDataTypeUint48_c,gZclAttrFlagsInRAM_c|gZclAttrFlagsRdOnly_c,sizeof(Summ_t),(void *) MbrOfs(ZCLSmplMetServerAttrsRAM_t,TOUCurrTier12SummDlvrd)},
  {gZclAttrSmplMetTOUCurrTier12SummRcvd_c,    gZclDataTypeUint48_c,gZclAttrFlagsInRAM_c|gZclAttrFlagsRdOnly_c,sizeof(Summ_t),(void *) MbrOfs(ZCLSmplMetServerAttrsRAM_t,TOUCurrTier12SummRcvd)},
  {gZclAttrSmplMetTOUCurrTier13SummDlvrd_c,   gZclDataTypeUint48_c,gZclAttrFlagsInRAM_c|gZclAttrFlagsRdOnly_c,sizeof(Summ_t),(void *) MbrOfs(ZCLSmplMetServerAttrsRAM_t,TOUCurrTier13SummDlvrd)},
  {gZclAttrSmplMetTOUCurrTier13SummRcvd_c,    gZclDataTypeUint48_c,gZclAttrFlagsInRAM_c|gZclAttrFlagsRdOnly_c,sizeof(Summ_t),(void *) MbrOfs(ZCLSmplMetServerAttrsRAM_t,TOUCurrTier13SummRcvd)},
  {gZclAttrSmplMetTOUCurrTier14SummDlvrd_c,   gZclDataTypeUint48_c,gZclAttrFlagsInRAM_c|gZclAttrFlagsRdOnly_c,sizeof(Summ_t),(void *) MbrOfs(ZCLSmplMetServerAttrsRAM_t,TOUCurrTier14SummDlvrd)},
  {gZclAttrSmplMetTOUCurrTier14SummRcvd_c,    gZclDataTypeUint48_c,gZclAttrFlagsInRAM_c|gZclAttrFlagsRdOnly_c,sizeof(Summ_t),(void *) MbrOfs(ZCLSmplMetServerAttrsRAM_t,TOUCurrTier14SummRcvd)},
  {gZclAttrSmplMetTOUCurrTier15SummDlvrd_c,   gZclDataTypeUint48_c,gZclAttrFlagsInRAM_c|gZclAttrFlagsRdOnly_c,sizeof(Summ_t),(void *) MbrOfs(ZCLSmplMetServerAttrsRAM_t,TOUCurrTier15SummDlvrd)},
  {gZclAttrSmplMetTOUCurrTier15SummRcvd_c,    gZclDataTypeUint48_c,gZclAttrFlagsInRAM_c|gZclAttrFlagsRdOnly_c,sizeof(Summ_t),(void *) MbrOfs(ZCLSmplMetServerAttrsRAM_t,TOUCurrTier15SummRcvd)},
#endif

/* Mandatory Meter Status Set */
  { gZclAttrSmplMetMSStatus_c,                 gZclDataTypeBitmap8_c, gZclAttrFlagsInRAM_c | gZclAttrFlagsRdOnly_c, sizeof(uint8_t), (void *)MbrOfs(ZCLSmplMetServerAttrsRAM_t, MSStatus)  },
#if gZclClusterOptionals_d && gASL_ZclSmplMet_Optionals_d 
  { gZclAttrSmplMetMSSRemainingBatteryLife_c,  gZclDataTypeUint8_c, gZclAttrFlagsInRAM_c | gZclAttrFlagsRdOnly_c, sizeof(uint8_t), (void *)MbrOfs(ZCLSmplMetServerAttrsRAM_t, RemainingBatteryLife)  },
  { gZclAttrSmplMetMSSHoursInOperation_c,      gZclDataTypeUint24_c, gZclAttrFlagsInRAM_c | gZclAttrFlagsRdOnly_c, sizeof(Consmp), (void *)MbrOfs(ZCLSmplMetServerAttrsRAM_t, HoursInOperation)  },
  { gZclAttrSmplMetMSSHoursInFault_c,          gZclDataTypeUint24_c, gZclAttrFlagsInRAM_c | gZclAttrFlagsRdOnly_c, sizeof(Consmp), (void *)MbrOfs(ZCLSmplMetServerAttrsRAM_t, HoursInFault)  },
#endif

/* Mandatory Consmp Formating Set */
  { gZclAttrSmplMetCFSUnitofMeasure_c,          gZclDataTypeEnum8_c, gZclAttrFlagsInRAM_c | gZclAttrFlagsRdOnly_c, sizeof(uint8_t), (void *)MbrOfs(ZCLSmplMetServerAttrsRAM_t, SmplMetCFSUnitofMeasure) },
#if gZclClusterOptionals_d && gASL_ZclSmplMet_Optionals_d  
  {gZclAttrSmplMetCFSMultiplier_c,              gZclDataTypeUint24_c,gZclAttrFlagsInRAM_c|gZclAttrFlagsRdOnly_c, sizeof(Consmp),(void *) MbrOfs(ZCLSmplMetServerAttrsRAM_t,Mult)},
  {gZclAttrSmplMetCFSDivisor_c,                 gZclDataTypeUint24_c,gZclAttrFlagsInRAM_c|gZclAttrFlagsRdOnly_c, sizeof(Consmp),(void *) MbrOfs(ZCLSmplMetServerAttrsRAM_t,Div)},
#endif  
  {gZclAttrSmplMetCFSSummFormat_c,              gZclDataTypeBitmap8_c,gZclAttrFlagsInRAM_c | gZclAttrFlagsRdOnly_c, sizeof(uint8_t), (void *)MbrOfs(ZCLSmplMetServerAttrsRAM_t, CFSSummFormat) },    
#if gZclClusterOptionals_d && gASL_ZclSmplMet_Optionals_d    
  {gZclAttrSmplMetCFSDmndFormat_c,              gZclDataTypeBitmap8_c,gZclAttrFlagsInRAM_c|gZclAttrFlagsRdOnly_c, sizeof(uint8_t),(void *) MbrOfs(ZCLSmplMetServerAttrsRAM_t,DmndFormat)},
  {gZclAttrSmplMetCFSHistoricalConsmpFormat_c,  gZclDataTypeBitmap8_c,gZclAttrFlagsInRAM_c|gZclAttrFlagsRdOnly_c, sizeof(uint8_t),(void *) MbrOfs(ZCLSmplMetServerAttrsRAM_t,HistoricalConsmpFormat)},
#endif  
  {gZclAttrSmplMetCFSMetDevType_c,              gZclDataTypeBitmap8_c,gZclAttrFlagsInRAM_c | gZclAttrFlagsRdOnly_c, sizeof(uint8_t), (void *)MbrOfs(ZCLSmplMetServerAttrsRAM_t, CFSMetDevType) },   
#if gZclClusterOptionals_d && gASL_ZclSmplMet_Optionals_d
  {gZclAttrSmplMetCFSSiteID_c,                            gZclDataTypeOctetStr_c,gZclAttrFlagsInRAM_c | gZclAttrFlagsRdOnly_c, sizeof(zclStr32Oct_t), (void *)MbrOfs(ZCLSmplMetServerAttrsRAM_t, CFSSiteID) },
  {gZclAttrSmplMetCFSMeterSerialNumber_c,                 gZclDataTypeOctetStr_c,gZclAttrFlagsInRAM_c | gZclAttrFlagsRdOnly_c, sizeof(zclStr24Oct_t), (void *)MbrOfs(ZCLSmplMetServerAttrsRAM_t, CFSMeterSerialNumber) },
  {gZclAttrSmplMetCFSEnergyCarrierUnitOfMeasure_c,        gZclDataTypeEnum8_c,gZclAttrFlagsInRAM_c | gZclAttrFlagsRdOnly_c, sizeof(uint8_t), (void *)MbrOfs(ZCLSmplMetServerAttrsRAM_t, CFSEnergyCarrierUnitOfMeasure) },   
  {gZclAttrSmplMetCFSEnergyCarrierSummationFormatting_c,  gZclDataTypeBitmap8_c,gZclAttrFlagsInRAM_c | gZclAttrFlagsRdOnly_c, sizeof(uint8_t), (void *)MbrOfs(ZCLSmplMetServerAttrsRAM_t, CFSEnergyCarrierSummationFormatting) },   
  {gZclAttrSmplMetCFSEnergyCarrierDemandFormatting_c,     gZclDataTypeBitmap8_c,gZclAttrFlagsInRAM_c | gZclAttrFlagsRdOnly_c, sizeof(uint8_t), (void *)MbrOfs(ZCLSmplMetServerAttrsRAM_t, CFSEnergyCarrierDemandFormatting) },   
  {gZclAttrSmplMetCFSTempUnitOfMeasure_c,                 gZclDataTypeEnum8_c,gZclAttrFlagsInRAM_c | gZclAttrFlagsRdOnly_c, sizeof(uint8_t), (void *)MbrOfs(ZCLSmplMetServerAttrsRAM_t, CFSTempUnitOfMeasure) },   
  {gZclAttrSmplMetCFSTempFormatting_c,                    gZclDataTypeBitmap8_c,gZclAttrFlagsInRAM_c | gZclAttrFlagsRdOnly_c, sizeof(uint8_t), (void *)MbrOfs(ZCLSmplMetServerAttrsRAM_t, CFSTempFormatting) },   
   /*Attributes of the Simple Metering Meter ESP Historical attribute set */  
  {gZclAttrSmplMetEHCInstantaneousDmnd_c,               gZclDataTypeInt24_c, gZclAttrFlagsInRAM_c|gZclAttrFlagsRdOnly_c, sizeof(Consmp),(void *) MbrOfs(ZCLSmplMetServerAttrsRAM_t,InstantaneousDmnd)},
  {gZclAttrSmplMetEHCCurrDayConsmpDlvrd_c,              gZclDataTypeUint24_c,gZclAttrFlagsInRAM_c|gZclAttrFlagsRdOnly_c, sizeof(Consmp),(void *) MbrOfs(ZCLSmplMetServerAttrsRAM_t,CurrDayConsmpDlvrd)},
  {gZclAttrSmplMetEHCCurrDayConsmpRcvd_c,               gZclDataTypeUint24_c,gZclAttrFlagsInRAM_c|gZclAttrFlagsRdOnly_c, sizeof(Consmp),(void *) MbrOfs(ZCLSmplMetServerAttrsRAM_t,CurrDayConsmpRcvd)},
  {gZclAttrSmplMetEHCPreviousDayConsmpDlvrd_c,          gZclDataTypeUint24_c,gZclAttrFlagsInRAM_c|gZclAttrFlagsRdOnly_c, sizeof(Consmp),(void *) MbrOfs(ZCLSmplMetServerAttrsRAM_t,PreviousDayConsmpDlvrd)},
  {gZclAttrSmplMetEHCPreviousDayConsmpRcvd_c,           gZclDataTypeUint24_c,gZclAttrFlagsInRAM_c|gZclAttrFlagsRdOnly_c, sizeof(Consmp),(void *) MbrOfs(ZCLSmplMetServerAttrsRAM_t,PreviousDayConsmpRcvd)},
  {gZclAttrSmplMetEHCCurrPrtlProfIntrvStartTimeDlvrd_c, gZclDataTypeUTCTime_c, gZclAttrFlagsInRAM_c|gZclAttrFlagsRdOnly_c, sizeof(ZCLTime_t),(void *) MbrOfs(ZCLSmplMetServerAttrsRAM_t,CurrPartialProfileIntrvStartTimeDlvrd)},
  {gZclAttrSmplMetEHCCurrPrtlProfIntrvStartTimeRcvd_c,  gZclDataTypeUTCTime_c, gZclAttrFlagsInRAM_c|gZclAttrFlagsRdOnly_c, sizeof(ZCLTime_t),(void *) MbrOfs(ZCLSmplMetServerAttrsRAM_t,CurrPartialProfileIntrvStartTimeRcvd)},
  {gZclAttrSmplMetEHCCurrPrtlProfIntrvValueDlvrd_c,     gZclDataTypeUint24_c,gZclAttrFlagsInRAM_c|gZclAttrFlagsRdOnly_c, sizeof(IntrvForProfiling_t),(void *) MbrOfs(ZCLSmplMetServerAttrsRAM_t,CurrPartialProfileIntrvValueDlvrd)},
  {gZclAttrSmplMetEHCCurrPrtlProfIntrvValueRcvd_c,      gZclDataTypeUint24_c,gZclAttrFlagsInRAM_c|gZclAttrFlagsRdOnly_c, sizeof(IntrvForProfiling_t),(void *) MbrOfs(ZCLSmplMetServerAttrsRAM_t,CurrPartialProfileIntrvValueRcvd)},

  {gZclAttrSmplMetEHCCurrDayMaxPressure_c,              gZclDataTypeUint48_c,gZclAttrFlagsInRAM_c|gZclAttrFlagsRdOnly_c, sizeof(Pressure_t),(void *) MbrOfs(ZCLSmplMetServerAttrsRAM_t,CurrDayMaxPressure)},
  {gZclAttrSmplMetEHCCurrDayMinPressure_c,              gZclDataTypeUint48_c,gZclAttrFlagsInRAM_c|gZclAttrFlagsRdOnly_c, sizeof(Pressure_t),(void *) MbrOfs(ZCLSmplMetServerAttrsRAM_t,CurrDayMinPressure)},
  {gZclAttrSmplMetEHCPrevDayMaxPressure_c,              gZclDataTypeUint48_c,gZclAttrFlagsInRAM_c|gZclAttrFlagsRdOnly_c, sizeof(Pressure_t),(void *) MbrOfs(ZCLSmplMetServerAttrsRAM_t,PrevDayMaxPressure)},
  {gZclAttrSmplMetEHCPrevDayMinPressure_c,              gZclDataTypeUint48_c,gZclAttrFlagsInRAM_c|gZclAttrFlagsRdOnly_c, sizeof(Pressure_t),(void *) MbrOfs(ZCLSmplMetServerAttrsRAM_t,PrevDayMinPressure)},
  {gZclAttrSmplMetEHCCurrDayMaxDemand_c,                gZclDataTypeInt24_c,gZclAttrFlagsInRAM_c|gZclAttrFlagsRdOnly_c, sizeof(Demmand_t),(void *) MbrOfs(ZCLSmplMetServerAttrsRAM_t,CurrDayMaxDemand)},
  {gZclAttrSmplMetEHCPrevDayMaxDemand_c,                gZclDataTypeInt24_c,gZclAttrFlagsInRAM_c|gZclAttrFlagsRdOnly_c, sizeof(Demmand_t),(void *) MbrOfs(ZCLSmplMetServerAttrsRAM_t,PrevDayMaxDemand)},
  {gZclAttrSmplMetEHCCurrMonthMaxDemand_c,              gZclDataTypeInt24_c,gZclAttrFlagsInRAM_c|gZclAttrFlagsRdOnly_c, sizeof(Demmand_t),(void *) MbrOfs(ZCLSmplMetServerAttrsRAM_t,CurrMonthMaxDemand)},
  {gZclAttrSmplMetEHCCurrYearMaxDemand_c,               gZclDataTypeInt24_c,gZclAttrFlagsInRAM_c|gZclAttrFlagsRdOnly_c, sizeof(Demmand_t),(void *) MbrOfs(ZCLSmplMetServerAttrsRAM_t,CurrYearMaxDemand)},
  {gZclAttrSmplMetEHCCurrDayMaxEnergyCarrierDemand_c,   gZclDataTypeInt24_c,gZclAttrFlagsInRAM_c|gZclAttrFlagsRdOnly_c, sizeof(Demmand_t),(void *) MbrOfs(ZCLSmplMetServerAttrsRAM_t,CurrDayMaxEnergyCarrierDemand)},
  {gZclAttrSmplMetEHCPrevDayMaxEnergyCarrierDemand_c,   gZclDataTypeInt24_c,gZclAttrFlagsInRAM_c|gZclAttrFlagsRdOnly_c, sizeof(Demmand_t),(void *) MbrOfs(ZCLSmplMetServerAttrsRAM_t,PrevDayMaxEnergyCarrierDemand)},
  {gZclAttrSmplMetEHCCurrMonthMaxEnergyCarrierDemand_c, gZclDataTypeInt24_c,gZclAttrFlagsInRAM_c|gZclAttrFlagsRdOnly_c, sizeof(Demmand_t),(void *) MbrOfs(ZCLSmplMetServerAttrsRAM_t,CurrMonthMaxEnergyCarrierDemand)},
  {gZclAttrSmplMetEHCCurrMonthMinEnergyCarrierDemand_c, gZclDataTypeInt24_c,gZclAttrFlagsInRAM_c|gZclAttrFlagsRdOnly_c, sizeof(Demmand_t),(void *) MbrOfs(ZCLSmplMetServerAttrsRAM_t,CurrMonthMinEnergyCarrierDemand)},
  {gZclAttrSmplMetEHCCurrYearMaxEnergyCarrierDemand_c,  gZclDataTypeInt24_c,gZclAttrFlagsInRAM_c|gZclAttrFlagsRdOnly_c, sizeof(Demmand_t),(void *) MbrOfs(ZCLSmplMetServerAttrsRAM_t,CurrYearMaxEnergyCarrierDemand)},
  {gZclAttrSmplMetEHCCurrYearMinEnergyCarrierDemand_c,  gZclDataTypeInt24_c,gZclAttrFlagsInRAM_c|gZclAttrFlagsRdOnly_c, sizeof(Demmand_t),(void *) MbrOfs(ZCLSmplMetServerAttrsRAM_t,CurrYearMinEnergyCarrierDemand)},
  /*Attributes of the Simple Metering Meter Load Profile Configuration attribute set */  
  {gZclAttrSmplMetLPCMaxNumOfPeriodsDlvrd_c,      gZclDataTypeUint8_c,gZclAttrFlagsInRAM_c|gZclAttrFlagsRdOnly_c, sizeof(uint8_t),(void *) MbrOfs(ZCLSmplMetServerAttrsRAM_t,MaxNumberOfPeriodsDlvrd)},
  /*Attributes of the Simple Metering Alarm attribute set */  
  {gZclAttrSmplMetASGenericAlarmMask_c,                 gZclDataTypeBitmap16_c,gZclAttrFlagsInRAM_c | gZclAttrFlagsRdOnly_c, sizeof(uint16_t), (void *)MbrOfs(ZCLSmplMetServerAttrsRAM_t, ASGenericAlarmMask) },   
  {gZclAttrSmplMetASElectricityAlarmMask_c,             gZclDataTypeBitmap16_c,gZclAttrFlagsInRAM_c | gZclAttrFlagsRdOnly_c, sizeof(uint16_t), (void *)MbrOfs(ZCLSmplMetServerAttrsRAM_t, ASElectricityAlarmMask) },     
  {gZclAttrSmplMetASGenericFlowPressureAlarmMask_c,     gZclDataTypeBitmap16_c,gZclAttrFlagsInRAM_c | gZclAttrFlagsRdOnly_c, sizeof(uint16_t), (void *)MbrOfs(ZCLSmplMetServerAttrsRAM_t, ASGenericFlowPressureAlarmMask) },     
  {gZclAttrSmplMetASWaterSpecificAlarmMask_c,           gZclDataTypeBitmap16_c,gZclAttrFlagsInRAM_c | gZclAttrFlagsRdOnly_c, sizeof(uint16_t), (void *)MbrOfs(ZCLSmplMetServerAttrsRAM_t, ASWaterSpecificAlarmMask) },     
  {gZclAttrSmplMetASHeatAndCoolingSpecificAlarmMask_c,  gZclDataTypeBitmap16_c,gZclAttrFlagsInRAM_c | gZclAttrFlagsRdOnly_c, sizeof(uint16_t), (void *)MbrOfs(ZCLSmplMetServerAttrsRAM_t, ASHeatAndCoolingSpecificAlarmMask) },     
  {gZclAttrSmplMetASGasSpecificAlarmMask_c,             gZclDataTypeBitmap16_c,gZclAttrFlagsInRAM_c | gZclAttrFlagsRdOnly_c, sizeof(uint16_t), (void *)MbrOfs(ZCLSmplMetServerAttrsRAM_t, ASGasSpecificAlarmMask) },      
  /*Attributes of the Simple Metering Block Information Attribute Set */  
  /*Attributes of the Simple Metering Supply Limit Attribute Set */ 
  {gZclAttrSmplMetSLSCurrDmndDlvrd_c,                 gZclDataTypeUint24_c,gZclAttrFlagsInRAM_c | gZclAttrFlagsRdOnly_c, sizeof(Consmp), (void *)MbrOfs(ZCLSmplMetServerAttrsRAM_t, CurrDmndDlvrd) },   
  {gZclAttrSmplMetSLSDmndLimit_c,                     gZclDataTypeUint24_c,gZclAttrFlagsInRAM_c | gZclAttrFlagsRdOnly_c, sizeof(Consmp), (void *)MbrOfs(ZCLSmplMetServerAttrsRAM_t, DmndLimit) },     
  {gZclAttrSmplMetSLSDmndIntegrationPeriod_c,         gZclDataTypeUint8_c,gZclAttrFlagsInRAM_c | gZclAttrFlagsRdOnly_c, sizeof(uint8_t), (void *)MbrOfs(ZCLSmplMetServerAttrsRAM_t, DmndIntegrationPeriod) },     
  {gZclAttrSmplMetSLSNumOfDmndSubIntrvs_c,            gZclDataTypeUint8_c,gZclAttrFlagsInRAM_c | gZclAttrFlagsRdOnly_c, sizeof(uint8_t), (void *)MbrOfs(ZCLSmplMetServerAttrsRAM_t, NumOfDmndSubIntrvs) }
#endif
                                                                                                            
};

const zclAttrDefList_t gZclSmplMetAttrDefList = {
  NumberOfElements(gaZclSmplMetServerAttrDef),
  gaZclSmplMetServerAttrDef
};
/************** MESSAGE CLUSTER  ******************************************/


/*
There is NO attributes defined for the Messaging cluster.

*/
/************** PRICE CLUSTER  ******************************************/

const zclAttrDef_t gaZclPriceClientAttrDef[] = {
  {gZclAttrClientPrice_StartRandomizeMinutes_c, gZclDataTypeUint8_c, gZclAttrFlagsInRAM_c | gZclAttrFlagsRdOnly_c, sizeof(uint8_t), (void*)MbrOfs(sePriceClientAttrRAM_t, PriceIncreaseRandomizeMinutes) }
  ,{gZclAttrClientPrice_StopRandomizeMinutes_c, gZclDataTypeUint8_c, gZclAttrFlagsInRAM_c | gZclAttrFlagsRdOnly_c, sizeof(uint8_t), (void*)MbrOfs(sePriceClientAttrRAM_t, PriceDecreaseRandomizeMinutes) }
  ,{gZclAttrClientPrice_CommodityType_c, gZclDataTypeEnum8_c, gZclAttrFlagsInRAM_c | gZclAttrFlagsRdOnly_c, sizeof(uint8_t), (void*)MbrOfs(sePriceClientAttrRAM_t, CommodityType) }
};

const zclAttrDefList_t gZclPriceClientAttrDefList = {
  NumberOfElements(gaZclPriceClientAttrDef),
  gaZclPriceClientAttrDef
};

#if gZclClusterOptionals_d && (gASL_ZclPrice_TiersNumber_d > 0 || gASL_ZclPrice_BlockThresholdNumber_d > 0 || gASL_ZclPrice_BlockPeriodNumber_d  > 0 || gASL_ZclPrice_CommodityTypeNumber_d > 0 || gASL_ZclPrice_BlockPriceInfoNumber_d > 0)
const zclAttrDef_t gaZclPriceServerAttrDef[] = {

 
 //// 8 ---------- ---------- ---------- ---------- ---------- ---------- ---------- ---------- ---------- ---------- 
#if gASL_ZclPrice_TiersNumber_d >= 1
	{gZclAttrPrice_Tier1PriceLabel_c, gZclDataTypeOctetStr_c, gZclAttrFlagsInRAM_c | gZclAttrFlagsRdOnly_c, sizeof(zclStr12_t), (void*)MbrOfs(sePriceAttrRAM_t, TierPriceLabel[0]) }
#endif			
#if gASL_ZclPrice_TiersNumber_d >= 2
	,{gZclAttrPrice_Tier2PriceLabel_c, gZclDataTypeOctetStr_c, gZclAttrFlagsInRAM_c | gZclAttrFlagsRdOnly_c, sizeof(zclStr12_t), (void*)MbrOfs(sePriceAttrRAM_t, TierPriceLabel[1]) }
#endif			
#if gASL_ZclPrice_TiersNumber_d >= 3
	,{gZclAttrPrice_Tier3PriceLabel_c, gZclDataTypeOctetStr_c, gZclAttrFlagsInRAM_c | gZclAttrFlagsRdOnly_c, sizeof(zclStr12_t), (void*)MbrOfs(sePriceAttrRAM_t, TierPriceLabel[2]) }
#endif			
#if gASL_ZclPrice_TiersNumber_d >= 4
	,{gZclAttrPrice_Tier4PriceLabel_c, gZclDataTypeOctetStr_c, gZclAttrFlagsInRAM_c | gZclAttrFlagsRdOnly_c, sizeof(zclStr12_t), (void*)MbrOfs(sePriceAttrRAM_t, TierPriceLabel[3]) }
#endif			
#if gASL_ZclPrice_TiersNumber_d >= 5
	,{gZclAttrPrice_Tier5PriceLabel_c, gZclDataTypeOctetStr_c, gZclAttrFlagsInRAM_c | gZclAttrFlagsRdOnly_c, sizeof(zclStr12_t), (void*)MbrOfs(sePriceAttrRAM_t, TierPriceLabel[4]) }
#endif			
#if gASL_ZclPrice_TiersNumber_d >= 6
	,{gZclAttrPrice_Tier6PriceLabel_c, gZclDataTypeOctetStr_c, gZclAttrFlagsInRAM_c | gZclAttrFlagsRdOnly_c, sizeof(zclStr12_t), (void*)MbrOfs(sePriceAttrRAM_t, TierPriceLabel[5]) }
#endif			
#if gASL_ZclPrice_TiersNumber_d >= 7
	,{gZclAttrPrice_Tier7PriceLabel_c, gZclDataTypeOctetStr_c, gZclAttrFlagsInRAM_c | gZclAttrFlagsRdOnly_c, sizeof(zclStr12_t), (void*)MbrOfs(sePriceAttrRAM_t, TierPriceLabel[6]) }
#endif			
#if gASL_ZclPrice_TiersNumber_d >= 8
	,{gZclAttrPrice_Tier8PriceLabel_c, gZclDataTypeOctetStr_c, gZclAttrFlagsInRAM_c | gZclAttrFlagsRdOnly_c, sizeof(zclStr12_t), (void*)MbrOfs(sePriceAttrRAM_t, TierPriceLabel[7]) }
#endif			
#if gASL_ZclPrice_TiersNumber_d >= 9
	,{gZclAttrPrice_Tier9PriceLabel_c, gZclDataTypeOctetStr_c, gZclAttrFlagsInRAM_c | gZclAttrFlagsRdOnly_c, sizeof(zclStr12_t), (void*)MbrOfs(sePriceAttrRAM_t, TierPriceLabel[8]) }
#endif			
#if gASL_ZclPrice_TiersNumber_d >= 10
	,{gZclAttrPrice_Tier10PriceLabel_c, gZclDataTypeOctetStr_c, gZclAttrFlagsInRAM_c | gZclAttrFlagsRdOnly_c, sizeof(zclStr12_t), (void*)MbrOfs(sePriceAttrRAM_t, TierPriceLabel[9]) }
#endif			
#if gASL_ZclPrice_TiersNumber_d >= 11
	,{gZclAttrPrice_Tier11PriceLabel_c, gZclDataTypeOctetStr_c, gZclAttrFlagsInRAM_c | gZclAttrFlagsRdOnly_c, sizeof(zclStr12_t), (void*)MbrOfs(sePriceAttrRAM_t, TierPriceLabel[10]) }
#endif			
#if gASL_ZclPrice_TiersNumber_d >= 12
	,{gZclAttrPrice_Tier12PriceLabel_c, gZclDataTypeOctetStr_c, gZclAttrFlagsInRAM_c | gZclAttrFlagsRdOnly_c, sizeof(zclStr12_t), (void*)MbrOfs(sePriceAttrRAM_t, TierPriceLabel[11]) }
#endif			
#if gASL_ZclPrice_TiersNumber_d >= 13
	,{gZclAttrPrice_Tier13PriceLabel_c, gZclDataTypeOctetStr_c, gZclAttrFlagsInRAM_c | gZclAttrFlagsRdOnly_c, sizeof(zclStr12_t), (void*)MbrOfs(sePriceAttrRAM_t, TierPriceLabel[12]) }
#endif			
#if gASL_ZclPrice_TiersNumber_d >= 14
	,{gZclAttrPrice_Tier14PriceLabel_c, gZclDataTypeOctetStr_c, gZclAttrFlagsInRAM_c | gZclAttrFlagsRdOnly_c, sizeof(zclStr12_t), (void*)MbrOfs(sePriceAttrRAM_t, TierPriceLabel[13]) }
#endif			
#if gASL_ZclPrice_TiersNumber_d >= 15
	,{gZclAttrPrice_Tier15PriceLabel_c, gZclDataTypeOctetStr_c, gZclAttrFlagsInRAM_c | gZclAttrFlagsRdOnly_c, sizeof(zclStr12_t), (void*)MbrOfs(sePriceAttrRAM_t, TierPriceLabel[14]) }
#endif			


#if gASL_ZclPrice_BlockThresholdNumber_d >= 1
	,{gZclAttrPrice_Block1Threshold_c, gZclDataTypeInt48_c, gZclAttrFlagsInRAM_c | gZclAttrFlagsRdOnly_c, sizeof(BlockThreshold_t), (void*)MbrOfs(sePriceAttrRAM_t, BlockThresholdPrice[0]) }
#endif			
#if gASL_ZclPrice_BlockThresholdNumber_d >= 2
	,{gZclAttrPrice_Block2Threshold_c, gZclDataTypeInt48_c, gZclAttrFlagsInRAM_c | gZclAttrFlagsRdOnly_c, sizeof(BlockThreshold_t), (void*)MbrOfs(sePriceAttrRAM_t, BlockThresholdPrice[1]) }
#endif			
#if gASL_ZclPrice_BlockThresholdNumber_d >= 3
	,{gZclAttrPrice_Block3Threshold_c, gZclDataTypeInt48_c, gZclAttrFlagsInRAM_c | gZclAttrFlagsRdOnly_c, sizeof(BlockThreshold_t), (void*)MbrOfs(sePriceAttrRAM_t, BlockThresholdPrice[2]) }
#endif			
#if gASL_ZclPrice_BlockThresholdNumber_d >= 4
	,{gZclAttrPrice_Block4Threshold_c, gZclDataTypeInt48_c, gZclAttrFlagsInRAM_c | gZclAttrFlagsRdOnly_c, sizeof(BlockThreshold_t), (void*)MbrOfs(sePriceAttrRAM_t, BlockThresholdPrice[3]) }
#endif			
#if gASL_ZclPrice_BlockThresholdNumber_d >= 5
	,{gZclAttrPrice_Block5Threshold_c, gZclDataTypeInt48_c, gZclAttrFlagsInRAM_c | gZclAttrFlagsRdOnly_c, sizeof(BlockThreshold_t), (void*)MbrOfs(sePriceAttrRAM_t, BlockThresholdPrice[4]) }
#endif			
#if gASL_ZclPrice_BlockThresholdNumber_d >= 6
	,{gZclAttrPrice_Block6Threshold_c, gZclDataTypeInt48_c, gZclAttrFlagsInRAM_c | gZclAttrFlagsRdOnly_c, sizeof(BlockThreshold_t), (void*)MbrOfs(sePriceAttrRAM_t, BlockThresholdPrice[5]) }
#endif			
#if gASL_ZclPrice_BlockThresholdNumber_d >= 7
	,{gZclAttrPrice_Block7Threshold_c, gZclDataTypeInt48_c, gZclAttrFlagsInRAM_c | gZclAttrFlagsRdOnly_c, sizeof(BlockThreshold_t), (void*)MbrOfs(sePriceAttrRAM_t, BlockThresholdPrice[6]) }
#endif			
#if gASL_ZclPrice_BlockThresholdNumber_d >= 8
	,{gZclAttrPrice_Block8Threshold_c, gZclDataTypeInt48_c, gZclAttrFlagsInRAM_c | gZclAttrFlagsRdOnly_c, sizeof(BlockThreshold_t), (void*)MbrOfs(sePriceAttrRAM_t, BlockThresholdPrice[7]) }
#endif			
#if gASL_ZclPrice_BlockThresholdNumber_d >= 9
	,{gZclAttrPrice_Block9Threshold_c, gZclDataTypeInt48_c, gZclAttrFlagsInRAM_c | gZclAttrFlagsRdOnly_c, sizeof(BlockThreshold_t), (void*)MbrOfs(sePriceAttrRAM_t, BlockThresholdPrice[8]) }
#endif			
#if gASL_ZclPrice_BlockThresholdNumber_d >= 10
	,{gZclAttrPrice_Block10Threshold_c, gZclDataTypeInt48_c, gZclAttrFlagsInRAM_c | gZclAttrFlagsRdOnly_c, sizeof(BlockThreshold_t), (void*)MbrOfs(sePriceAttrRAM_t, BlockThresholdPrice[9]) }
#endif			
#if gASL_ZclPrice_BlockThresholdNumber_d >= 11
	,{gZclAttrPrice_Block11Threshold_c, gZclDataTypeInt48_c, gZclAttrFlagsInRAM_c | gZclAttrFlagsRdOnly_c, sizeof(BlockThreshold_t), (void*)MbrOfs(sePriceAttrRAM_t, BlockThresholdPrice[10]) }
#endif			
#if gASL_ZclPrice_BlockThresholdNumber_d >= 12
	,{gZclAttrPrice_Block12Threshold_c, gZclDataTypeInt48_c, gZclAttrFlagsInRAM_c | gZclAttrFlagsRdOnly_c, sizeof(BlockThreshold_t), (void*)MbrOfs(sePriceAttrRAM_t, BlockThresholdPrice[11]) }
#endif			
#if gASL_ZclPrice_BlockThresholdNumber_d >= 13
	,{gZclAttrPrice_Block13Threshold_c, gZclDataTypeInt48_c, gZclAttrFlagsInRAM_c | gZclAttrFlagsRdOnly_c, sizeof(BlockThreshold_t), (void*)MbrOfs(sePriceAttrRAM_t, BlockThresholdPrice[12]) }
#endif			
#if gASL_ZclPrice_BlockThresholdNumber_d >= 14
	,{gZclAttrPrice_Block14Threshold_c, gZclDataTypeInt48_c, gZclAttrFlagsInRAM_c | gZclAttrFlagsRdOnly_c, sizeof(BlockThreshold_t), (void*)MbrOfs(sePriceAttrRAM_t, BlockThresholdPrice[13]) }
#endif			
#if gASL_ZclPrice_BlockThresholdNumber_d >= 15
	,{gZclAttrPrice_Block15Threshold_c, gZclDataTypeInt48_c, gZclAttrFlagsInRAM_c | gZclAttrFlagsRdOnly_c, sizeof(BlockThreshold_t), (void*)MbrOfs(sePriceAttrRAM_t, BlockThresholdPrice[14]) }
#endif			


#if gASL_ZclPrice_BlockPeriodNumber_d >= 1
	,{gZclAttrPrice_StartofBlockPeriod_c, gZclDataTypeUTCTime_c, gZclAttrFlagsInRAM_c | gZclAttrFlagsRdOnly_c, sizeof(ZCLTime_t), (void*)(MbrOfs(sePriceAttrRAM_t, BlockPeriodPrice) + MbrOfs(BlockPeriodPrice_t, StartofBlockPeriod)) }
#endif			
#if gASL_ZclPrice_BlockPeriodNumber_d >= 2
	,{gZclAttrPrice_StartofBlockPeriodDuration_c, gZclDataTypeUint24_c, gZclAttrFlagsInRAM_c | gZclAttrFlagsRdOnly_c, sizeof(Duration24_t), (void*)(MbrOfs(sePriceAttrRAM_t, BlockPeriodPrice) + MbrOfs(BlockPeriodPrice_t, BlockPeriodDuration)) }
#endif
#if gASL_ZclPrice_BlockPeriodNumber_d >= 3
	,{gZclAttrPrice_ThresholdMultiplier_c, gZclDataTypeUint24_c, gZclAttrFlagsInRAM_c | gZclAttrFlagsRdOnly_c, sizeof(Multiplier24_t), (void*)(MbrOfs(sePriceAttrRAM_t, BlockPeriodPrice) + MbrOfs(BlockPeriodPrice_t, ThresholdMultiplier)) }
#endif
#if gASL_ZclPrice_BlockPeriodNumber_d >= 4
	,{gZclAttrPrice_ThresholdDivisor_c, gZclDataTypeUint24_c, gZclAttrFlagsInRAM_c | gZclAttrFlagsRdOnly_c, sizeof(Divisor24_t), (void*)(MbrOfs(sePriceAttrRAM_t, BlockPeriodPrice) + MbrOfs(BlockPeriodPrice_t, ThresholdDivisor)) }
#endif

#if gASL_ZclPrice_CommodityTypeNumber_d >= 1
	,{gZclAttrPrice_CommodityType_c, gZclDataTypeEnum8_c, gZclAttrFlagsInRAM_c | gZclAttrFlagsRdOnly_c, sizeof(uint8_t), (void*)(MbrOfs(sePriceAttrRAM_t, CommodityTypePrice) + MbrOfs(CommodityTypePrice_t, CommodityType)) }
#endif
#if gASL_ZclPrice_CommodityTypeNumber_d >= 2
	,{gZclAttrPrice_StandingCharge_c, gZclDataTypeUint32_c, gZclAttrFlagsInRAM_c | gZclAttrFlagsRdOnly_c, sizeof(CommodityCharge_t), (void*)(MbrOfs(sePriceAttrRAM_t, CommodityTypePrice) + MbrOfs(CommodityTypePrice_t, StandingCharge)) }
#endif

#if gASL_ZclPrice_BlockPriceInfoNumber_d >= 1
	,{gZclAttrPrice_NoTierBlock1Price_c, gZclDataTypeUint32_c, gZclAttrFlagsInRAM_c | gZclAttrFlagsRdOnly_c, sizeof(uint32_t), (void*)MbrOfs(sePriceAttrRAM_t, BlockPriceInfoPrice[0]) }
#endif			
#if gASL_ZclPrice_BlockPriceInfoNumber_d >= 2
	,{gZclAttrPrice_NoTierBlock2Price_c, gZclDataTypeUint32_c, gZclAttrFlagsInRAM_c | gZclAttrFlagsRdOnly_c, sizeof(uint32_t), (void*)MbrOfs(sePriceAttrRAM_t, BlockPriceInfoPrice[1]) }
#endif			
#if gASL_ZclPrice_BlockPriceInfoNumber_d >= 3
	,{gZclAttrPrice_NoTierBlock3Price_c, gZclDataTypeUint32_c, gZclAttrFlagsInRAM_c | gZclAttrFlagsRdOnly_c, sizeof(uint32_t), (void*)MbrOfs(sePriceAttrRAM_t, BlockPriceInfoPrice[2]) }
#endif			
#if gASL_ZclPrice_BlockPriceInfoNumber_d >= 4
	,{gZclAttrPrice_NoTierBlock4Price_c, gZclDataTypeUint32_c, gZclAttrFlagsInRAM_c | gZclAttrFlagsRdOnly_c, sizeof(uint32_t), (void*)MbrOfs(sePriceAttrRAM_t, BlockPriceInfoPrice[3]) }
#endif			
#if gASL_ZclPrice_BlockPriceInfoNumber_d >= 5
	,{gZclAttrPrice_NoTierBlock5Price_c, gZclDataTypeUint32_c, gZclAttrFlagsInRAM_c | gZclAttrFlagsRdOnly_c, sizeof(uint32_t), (void*)MbrOfs(sePriceAttrRAM_t, BlockPriceInfoPrice[4]) }
#endif			
#if gASL_ZclPrice_BlockPriceInfoNumber_d >= 6
	,{gZclAttrPrice_NoTierBlock6Price_c, gZclDataTypeUint32_c, gZclAttrFlagsInRAM_c | gZclAttrFlagsRdOnly_c, sizeof(uint32_t), (void*)MbrOfs(sePriceAttrRAM_t, BlockPriceInfoPrice[5]) }
#endif			
#if gASL_ZclPrice_BlockPriceInfoNumber_d >= 7
	,{gZclAttrPrice_NoTierBlock7Price_c, gZclDataTypeUint32_c, gZclAttrFlagsInRAM_c | gZclAttrFlagsRdOnly_c, sizeof(uint32_t), (void*)MbrOfs(sePriceAttrRAM_t, BlockPriceInfoPrice[6]) }
#endif			
#if gASL_ZclPrice_BlockPriceInfoNumber_d >= 8
	,{gZclAttrPrice_NoTierBlock8Price_c, gZclDataTypeUint32_c, gZclAttrFlagsInRAM_c | gZclAttrFlagsRdOnly_c, sizeof(uint32_t), (void*)MbrOfs(sePriceAttrRAM_t, BlockPriceInfoPrice[7]) }
#endif			
#if gASL_ZclPrice_BlockPriceInfoNumber_d >= 9
	,{gZclAttrPrice_NoTierBlock9Price_c, gZclDataTypeUint32_c, gZclAttrFlagsInRAM_c | gZclAttrFlagsRdOnly_c, sizeof(uint32_t), (void*)MbrOfs(sePriceAttrRAM_t, BlockPriceInfoPrice[8]) }
#endif			
#if gASL_ZclPrice_BlockPriceInfoNumber_d >= 10
	,{gZclAttrPrice_NoTierBlock10Price_c, gZclDataTypeUint32_c, gZclAttrFlagsInRAM_c | gZclAttrFlagsRdOnly_c, sizeof(uint32_t), (void*)MbrOfs(sePriceAttrRAM_t, BlockPriceInfoPrice[9]) }
#endif			
#if gASL_ZclPrice_BlockPriceInfoNumber_d >= 11
	,{gZclAttrPrice_NoTierBlock11Price_c, gZclDataTypeUint32_c, gZclAttrFlagsInRAM_c | gZclAttrFlagsRdOnly_c, sizeof(uint32_t), (void*)MbrOfs(sePriceAttrRAM_t, BlockPriceInfoPrice[10]) }
#endif			
#if gASL_ZclPrice_BlockPriceInfoNumber_d >= 12
	,{gZclAttrPrice_NoTierBlock12Price_c, gZclDataTypeUint32_c, gZclAttrFlagsInRAM_c | gZclAttrFlagsRdOnly_c, sizeof(uint32_t), (void*)MbrOfs(sePriceAttrRAM_t, BlockPriceInfoPrice[11]) }
#endif			
#if gASL_ZclPrice_BlockPriceInfoNumber_d >= 13
	,{gZclAttrPrice_NoTierBlock13Price_c, gZclDataTypeUint32_c, gZclAttrFlagsInRAM_c | gZclAttrFlagsRdOnly_c, sizeof(uint32_t), (void*)MbrOfs(sePriceAttrRAM_t, BlockPriceInfoPrice[12]) }
#endif			
#if gASL_ZclPrice_BlockPriceInfoNumber_d >= 14
	,{gZclAttrPrice_NoTierBlock14Price_c, gZclDataTypeUint32_c, gZclAttrFlagsInRAM_c | gZclAttrFlagsRdOnly_c, sizeof(uint32_t), (void*)MbrOfs(sePriceAttrRAM_t, BlockPriceInfoPrice[13]) }
#endif			
#if gASL_ZclPrice_BlockPriceInfoNumber_d >= 15
	,{gZclAttrPrice_NoTierBlock15Price_c, gZclDataTypeUint32_c, gZclAttrFlagsInRAM_c | gZclAttrFlagsRdOnly_c, sizeof(uint32_t), (void*)MbrOfs(sePriceAttrRAM_t, BlockPriceInfoPrice[14]) }
#endif			
#if gASL_ZclPrice_BlockPriceInfoNumber_d >= 16
	,{gZclAttrPrice_NoTierBlock16Price_c, gZclDataTypeUint32_c, gZclAttrFlagsInRAM_c | gZclAttrFlagsRdOnly_c, sizeof(uint32_t), (void*)MbrOfs(sePriceAttrRAM_t, BlockPriceInfoPrice[15]) }
#endif			

#if gASL_ZclPrice_BlockPriceInfoNumber_d >= 17
	,{gZclAttrPrice_Tier1Block1Price_c, gZclDataTypeUint32_c, gZclAttrFlagsInRAM_c | gZclAttrFlagsRdOnly_c, sizeof(uint32_t), (void*)MbrOfs(sePriceAttrRAM_t, BlockPriceInfoPrice[16]) }
#endif			
#if gASL_ZclPrice_BlockPriceInfoNumber_d >= 18
	,{gZclAttrPrice_Tier1Block2Price_c, gZclDataTypeUint32_c, gZclAttrFlagsInRAM_c | gZclAttrFlagsRdOnly_c, sizeof(uint32_t), (void*)MbrOfs(sePriceAttrRAM_t, BlockPriceInfoPrice[17]) }
#endif			
#if gASL_ZclPrice_BlockPriceInfoNumber_d >= 19
	,{gZclAttrPrice_Tier1Block3Price_c, gZclDataTypeUint32_c, gZclAttrFlagsInRAM_c | gZclAttrFlagsRdOnly_c, sizeof(uint32_t), (void*)MbrOfs(sePriceAttrRAM_t, BlockPriceInfoPrice[18]) }
#endif			
#if gASL_ZclPrice_BlockPriceInfoNumber_d >= 20
	,{gZclAttrPrice_Tier1Block4Price_c, gZclDataTypeUint32_c, gZclAttrFlagsInRAM_c | gZclAttrFlagsRdOnly_c, sizeof(uint32_t), (void*)MbrOfs(sePriceAttrRAM_t, BlockPriceInfoPrice[19]) }
#endif			
#if gASL_ZclPrice_BlockPriceInfoNumber_d >= 21
	,{gZclAttrPrice_Tier1Block5Price_c, gZclDataTypeUint32_c, gZclAttrFlagsInRAM_c | gZclAttrFlagsRdOnly_c, sizeof(uint32_t), (void*)MbrOfs(sePriceAttrRAM_t, BlockPriceInfoPrice[20]) }
#endif			
#if gASL_ZclPrice_BlockPriceInfoNumber_d >= 22
	,{gZclAttrPrice_Tier1Block6Price_c, gZclDataTypeUint32_c, gZclAttrFlagsInRAM_c | gZclAttrFlagsRdOnly_c, sizeof(uint32_t), (void*)MbrOfs(sePriceAttrRAM_t, BlockPriceInfoPrice[21]) }
#endif			
#if gASL_ZclPrice_BlockPriceInfoNumber_d >= 23
	,{gZclAttrPrice_Tier1Block7Price_c, gZclDataTypeUint32_c, gZclAttrFlagsInRAM_c | gZclAttrFlagsRdOnly_c, sizeof(uint32_t), (void*)MbrOfs(sePriceAttrRAM_t, BlockPriceInfoPrice[22]) }
#endif			
#if gASL_ZclPrice_BlockPriceInfoNumber_d >= 4
	,{gZclAttrPrice_Tier1Block8Price_c, gZclDataTypeUint32_c, gZclAttrFlagsInRAM_c | gZclAttrFlagsRdOnly_c, sizeof(uint32_t), (void*)MbrOfs(sePriceAttrRAM_t, BlockPriceInfoPrice[23]) }
#endif			
#if gASL_ZclPrice_BlockPriceInfoNumber_d >= 25
	,{gZclAttrPrice_Tier1Block9Price_c, gZclDataTypeUint32_c, gZclAttrFlagsInRAM_c | gZclAttrFlagsRdOnly_c, sizeof(uint32_t), (void*)MbrOfs(sePriceAttrRAM_t, BlockPriceInfoPrice[24]) }
#endif			
#if gASL_ZclPrice_BlockPriceInfoNumber_d >= 26
	,{gZclAttrPrice_Tier1Block10Price_c, gZclDataTypeUint32_c, gZclAttrFlagsInRAM_c | gZclAttrFlagsRdOnly_c, sizeof(uint32_t), (void*)MbrOfs(sePriceAttrRAM_t, BlockPriceInfoPrice[25]) }
#endif			
#if gASL_ZclPrice_BlockPriceInfoNumber_d >= 27
	,{gZclAttrPrice_Tier1Block11Price_c, gZclDataTypeUint32_c, gZclAttrFlagsInRAM_c | gZclAttrFlagsRdOnly_c, sizeof(uint32_t), (void*)MbrOfs(sePriceAttrRAM_t, BlockPriceInfoPrice[26]) }
#endif			
#if gASL_ZclPrice_BlockPriceInfoNumber_d >= 28
	,{gZclAttrPrice_Tier1Block12Price_c, gZclDataTypeUint32_c, gZclAttrFlagsInRAM_c | gZclAttrFlagsRdOnly_c, sizeof(uint32_t), (void*)MbrOfs(sePriceAttrRAM_t, BlockPriceInfoPrice[27]) }
#endif			
#if gASL_ZclPrice_BlockPriceInfoNumber_d >= 29
	,{gZclAttrPrice_Tier1Block13Price_c, gZclDataTypeUint32_c, gZclAttrFlagsInRAM_c | gZclAttrFlagsRdOnly_c, sizeof(uint32_t), (void*)MbrOfs(sePriceAttrRAM_t, BlockPriceInfoPrice[28]) }
#endif			
#if gASL_ZclPrice_BlockPriceInfoNumber_d >= 30
	,{gZclAttrPrice_Tier1Block14Price_c, gZclDataTypeUint32_c, gZclAttrFlagsInRAM_c | gZclAttrFlagsRdOnly_c, sizeof(uint32_t), (void*)MbrOfs(sePriceAttrRAM_t, BlockPriceInfoPrice[29]) }
#endif			
#if gASL_ZclPrice_BlockPriceInfoNumber_d >= 31
	,{gZclAttrPrice_Tier1Block15Price_c, gZclDataTypeUint32_c, gZclAttrFlagsInRAM_c | gZclAttrFlagsRdOnly_c, sizeof(uint32_t), (void*)MbrOfs(sePriceAttrRAM_t, BlockPriceInfoPrice[30]) }
#endif			
#if gASL_ZclPrice_BlockPriceInfoNumber_d >= 32
	,{gZclAttrPrice_Tier1Block16Price_c, gZclDataTypeUint32_c, gZclAttrFlagsInRAM_c | gZclAttrFlagsRdOnly_c, sizeof(uint32_t), (void*)MbrOfs(sePriceAttrRAM_t, BlockPriceInfoPrice[31]) }
#endif			

#if gASL_ZclPrice_BlockPriceInfoNumber_d >= 33
	,{gZclAttrPrice_Tier2Block1Price_c, gZclDataTypeUint32_c, gZclAttrFlagsInRAM_c | gZclAttrFlagsRdOnly_c, sizeof(uint32_t), (void*)MbrOfs(sePriceAttrRAM_t, BlockPriceInfoPrice[32]) }
#endif			
#if gASL_ZclPrice_BlockPriceInfoNumber_d >= 34
	,{gZclAttrPrice_Tier2Block2Price_c, gZclDataTypeUint32_c, gZclAttrFlagsInRAM_c | gZclAttrFlagsRdOnly_c, sizeof(uint32_t), (void*)MbrOfs(sePriceAttrRAM_t, BlockPriceInfoPrice[33]) }
#endif			
#if gASL_ZclPrice_BlockPriceInfoNumber_d >= 35
	,{gZclAttrPrice_Tier2Block3Price_c, gZclDataTypeUint32_c, gZclAttrFlagsInRAM_c | gZclAttrFlagsRdOnly_c, sizeof(uint32_t), (void*)MbrOfs(sePriceAttrRAM_t, BlockPriceInfoPrice[34]) }
#endif			
#if gASL_ZclPrice_BlockPriceInfoNumber_d >= 36
	,{gZclAttrPrice_Tier2Block4Price_c, gZclDataTypeUint32_c, gZclAttrFlagsInRAM_c | gZclAttrFlagsRdOnly_c, sizeof(uint32_t), (void*)MbrOfs(sePriceAttrRAM_t, BlockPriceInfoPrice[35]) }
#endif			
#if gASL_ZclPrice_BlockPriceInfoNumber_d >= 37
	,{gZclAttrPrice_Tier2Block5Price_c, gZclDataTypeUint32_c, gZclAttrFlagsInRAM_c | gZclAttrFlagsRdOnly_c, sizeof(uint32_t), (void*)MbrOfs(sePriceAttrRAM_t, BlockPriceInfoPrice[36]) }
#endif			
#if gASL_ZclPrice_BlockPriceInfoNumber_d >= 38
	,{gZclAttrPrice_Tier2Block6Price_c, gZclDataTypeUint32_c, gZclAttrFlagsInRAM_c | gZclAttrFlagsRdOnly_c, sizeof(uint32_t), (void*)MbrOfs(sePriceAttrRAM_t, BlockPriceInfoPrice[37]) }
#endif			
#if gASL_ZclPrice_BlockPriceInfoNumber_d >= 39
	,{gZclAttrPrice_Tier2Block7Price_c, gZclDataTypeUint32_c, gZclAttrFlagsInRAM_c | gZclAttrFlagsRdOnly_c, sizeof(uint32_t), (void*)MbrOfs(sePriceAttrRAM_t, BlockPriceInfoPrice[38]) }
#endif			
#if gASL_ZclPrice_BlockPriceInfoNumber_d >= 40
	,{gZclAttrPrice_Tier2Block8Price_c, gZclDataTypeUint32_c, gZclAttrFlagsInRAM_c | gZclAttrFlagsRdOnly_c, sizeof(uint32_t), (void*)MbrOfs(sePriceAttrRAM_t, BlockPriceInfoPrice[39]) }
#endif			
#if gASL_ZclPrice_BlockPriceInfoNumber_d >= 41
	,{gZclAttrPrice_Tier2Block9Price_c, gZclDataTypeUint32_c, gZclAttrFlagsInRAM_c | gZclAttrFlagsRdOnly_c, sizeof(uint32_t), (void*)MbrOfs(sePriceAttrRAM_t, BlockPriceInfoPrice[40]) }
#endif			
#if gASL_ZclPrice_BlockPriceInfoNumber_d >= 42
	,{gZclAttrPrice_Tier2Block10Price_c, gZclDataTypeUint32_c, gZclAttrFlagsInRAM_c | gZclAttrFlagsRdOnly_c, sizeof(uint32_t), (void*)MbrOfs(sePriceAttrRAM_t, BlockPriceInfoPrice[41]) }
#endif			
#if gASL_ZclPrice_BlockPriceInfoNumber_d >= 43
	,{gZclAttrPrice_Tier2Block11Price_c, gZclDataTypeUint32_c, gZclAttrFlagsInRAM_c | gZclAttrFlagsRdOnly_c, sizeof(uint32_t), (void*)MbrOfs(sePriceAttrRAM_t, BlockPriceInfoPrice[42]) }
#endif			
#if gASL_ZclPrice_BlockPriceInfoNumber_d >= 44
	,{gZclAttrPrice_Tier2Block12Price_c, gZclDataTypeUint32_c, gZclAttrFlagsInRAM_c | gZclAttrFlagsRdOnly_c, sizeof(uint32_t), (void*)MbrOfs(sePriceAttrRAM_t, BlockPriceInfoPrice[43]) }
#endif			
#if gASL_ZclPrice_BlockPriceInfoNumber_d >= 45
	,{gZclAttrPrice_Tier2Block13Price_c, gZclDataTypeUint32_c, gZclAttrFlagsInRAM_c | gZclAttrFlagsRdOnly_c, sizeof(uint32_t), (void*)MbrOfs(sePriceAttrRAM_t, BlockPriceInfoPrice[44]) }
#endif			
#if gASL_ZclPrice_BlockPriceInfoNumber_d >= 46
	,{gZclAttrPrice_Tier2Block14Price_c, gZclDataTypeUint32_c, gZclAttrFlagsInRAM_c | gZclAttrFlagsRdOnly_c, sizeof(uint32_t), (void*)MbrOfs(sePriceAttrRAM_t, BlockPriceInfoPrice[45]) }
#endif			
#if gASL_ZclPrice_BlockPriceInfoNumber_d >= 47
	,{gZclAttrPrice_Tier2Block15Price_c, gZclDataTypeUint32_c, gZclAttrFlagsInRAM_c | gZclAttrFlagsRdOnly_c, sizeof(uint32_t), (void*)MbrOfs(sePriceAttrRAM_t, BlockPriceInfoPrice[46]) }
#endif			
#if gASL_ZclPrice_BlockPriceInfoNumber_d >= 48
	,{gZclAttrPrice_Tier2Block16Price_c, gZclDataTypeUint32_c, gZclAttrFlagsInRAM_c | gZclAttrFlagsRdOnly_c, sizeof(uint32_t), (void*)MbrOfs(sePriceAttrRAM_t, BlockPriceInfoPrice[47]) }
#endif			

#if gASL_ZclPrice_BlockPriceInfoNumber_d >= 49
	,{gZclAttrPrice_Tier3Block1Price_c, gZclDataTypeUint32_c, gZclAttrFlagsInRAM_c | gZclAttrFlagsRdOnly_c, sizeof(uint32_t), (void*)MbrOfs(sePriceAttrRAM_t, BlockPriceInfoPrice[48]) }
#endif			
#if gASL_ZclPrice_BlockPriceInfoNumber_d >= 50
	,{gZclAttrPrice_Tier3Block2Price_c, gZclDataTypeUint32_c, gZclAttrFlagsInRAM_c | gZclAttrFlagsRdOnly_c, sizeof(uint32_t), (void*)MbrOfs(sePriceAttrRAM_t, BlockPriceInfoPrice[49]) }
#endif			
#if gASL_ZclPrice_BlockPriceInfoNumber_d >= 51
	,{gZclAttrPrice_Tier3Block3Price_c, gZclDataTypeUint32_c, gZclAttrFlagsInRAM_c | gZclAttrFlagsRdOnly_c, sizeof(uint32_t), (void*)MbrOfs(sePriceAttrRAM_t, BlockPriceInfoPrice[50]) }
#endif			
#if gASL_ZclPrice_BlockPriceInfoNumber_d >= 52
	,{gZclAttrPrice_Tier3Block4Price_c, gZclDataTypeUint32_c, gZclAttrFlagsInRAM_c | gZclAttrFlagsRdOnly_c, sizeof(uint32_t), (void*)MbrOfs(sePriceAttrRAM_t, BlockPriceInfoPrice[51]) }
#endif			
#if gASL_ZclPrice_BlockPriceInfoNumber_d >= 53
	,{gZclAttrPrice_Tier3Block5Price_c, gZclDataTypeUint32_c, gZclAttrFlagsInRAM_c | gZclAttrFlagsRdOnly_c, sizeof(uint32_t), (void*)MbrOfs(sePriceAttrRAM_t, BlockPriceInfoPrice[52]) }
#endif			
#if gASL_ZclPrice_BlockPriceInfoNumber_d >= 54
	,{gZclAttrPrice_Tier3Block6Price_c, gZclDataTypeUint32_c, gZclAttrFlagsInRAM_c | gZclAttrFlagsRdOnly_c, sizeof(uint32_t), (void*)MbrOfs(sePriceAttrRAM_t, BlockPriceInfoPrice[53]) }
#endif			
#if gASL_ZclPrice_BlockPriceInfoNumber_d >= 55
	,{gZclAttrPrice_Tier3Block7Price_c, gZclDataTypeUint32_c, gZclAttrFlagsInRAM_c | gZclAttrFlagsRdOnly_c, sizeof(uint32_t), (void*)MbrOfs(sePriceAttrRAM_t, BlockPriceInfoPrice[54]) }
#endif			
#if gASL_ZclPrice_BlockPriceInfoNumber_d >= 56
	,{gZclAttrPrice_Tier3Block8Price_c, gZclDataTypeUint32_c, gZclAttrFlagsInRAM_c | gZclAttrFlagsRdOnly_c, sizeof(uint32_t), (void*)MbrOfs(sePriceAttrRAM_t, BlockPriceInfoPrice[55]) }
#endif			
#if gASL_ZclPrice_BlockPriceInfoNumber_d >= 57
	,{gZclAttrPrice_Tier3Block9Price_c, gZclDataTypeUint32_c, gZclAttrFlagsInRAM_c | gZclAttrFlagsRdOnly_c, sizeof(uint32_t), (void*)MbrOfs(sePriceAttrRAM_t, BlockPriceInfoPrice[56]) }
#endif			
#if gASL_ZclPrice_BlockPriceInfoNumber_d >= 58
	,{gZclAttrPrice_Tier3Block10Price_c, gZclDataTypeUint32_c, gZclAttrFlagsInRAM_c | gZclAttrFlagsRdOnly_c, sizeof(uint32_t), (void*)MbrOfs(sePriceAttrRAM_t, BlockPriceInfoPrice[57]) }
#endif			
#if gASL_ZclPrice_BlockPriceInfoNumber_d >= 59
	,{gZclAttrPrice_Tier3Block11Price_c, gZclDataTypeUint32_c, gZclAttrFlagsInRAM_c | gZclAttrFlagsRdOnly_c, sizeof(uint32_t), (void*)MbrOfs(sePriceAttrRAM_t, BlockPriceInfoPrice[58]) }
#endif			
#if gASL_ZclPrice_BlockPriceInfoNumber_d >= 60
	,{gZclAttrPrice_Tier3Block12Price_c, gZclDataTypeUint32_c, gZclAttrFlagsInRAM_c | gZclAttrFlagsRdOnly_c, sizeof(uint32_t), (void*)MbrOfs(sePriceAttrRAM_t, BlockPriceInfoPrice[59]) }
#endif			
#if gASL_ZclPrice_BlockPriceInfoNumber_d >= 61
	,{gZclAttrPrice_Tier3Block13Price_c, gZclDataTypeUint32_c, gZclAttrFlagsInRAM_c | gZclAttrFlagsRdOnly_c, sizeof(uint32_t), (void*)MbrOfs(sePriceAttrRAM_t, BlockPriceInfoPrice[60]) }
#endif			
#if gASL_ZclPrice_BlockPriceInfoNumber_d >= 62
	,{gZclAttrPrice_Tier3Block14Price_c, gZclDataTypeUint32_c, gZclAttrFlagsInRAM_c | gZclAttrFlagsRdOnly_c, sizeof(uint32_t), (void*)MbrOfs(sePriceAttrRAM_t, BlockPriceInfoPrice[61]) }
#endif			
#if gASL_ZclPrice_BlockPriceInfoNumber_d >= 63
	,{gZclAttrPrice_Tier3Block15Price_c, gZclDataTypeUint32_c, gZclAttrFlagsInRAM_c | gZclAttrFlagsRdOnly_c, sizeof(uint32_t), (void*)MbrOfs(sePriceAttrRAM_t, BlockPriceInfoPrice[62]) }
#endif			
#if gASL_ZclPrice_BlockPriceInfoNumber_d >= 64
	,{gZclAttrPrice_Tier3Block16Price_c, gZclDataTypeUint32_c, gZclAttrFlagsInRAM_c | gZclAttrFlagsRdOnly_c, sizeof(uint32_t), (void*)MbrOfs(sePriceAttrRAM_t, BlockPriceInfoPrice[63]) }
#endif			

#if gASL_ZclPrice_BlockPriceInfoNumber_d >= 65
	,{gZclAttrPrice_Tier4Block1Price_c, gZclDataTypeUint32_c, gZclAttrFlagsInRAM_c | gZclAttrFlagsRdOnly_c, sizeof(uint32_t), (void*)MbrOfs(sePriceAttrRAM_t, BlockPriceInfoPrice[64]) }
#endif			
#if gASL_ZclPrice_BlockPriceInfoNumber_d >= 66
	,{gZclAttrPrice_Tier4Block2Price_c, gZclDataTypeUint32_c, gZclAttrFlagsInRAM_c | gZclAttrFlagsRdOnly_c, sizeof(uint32_t), (void*)MbrOfs(sePriceAttrRAM_t, BlockPriceInfoPrice[65]) }
#endif			
#if gASL_ZclPrice_BlockPriceInfoNumber_d >= 67
	,{gZclAttrPrice_Tier4Block3Price_c, gZclDataTypeUint32_c, gZclAttrFlagsInRAM_c | gZclAttrFlagsRdOnly_c, sizeof(uint32_t), (void*)MbrOfs(sePriceAttrRAM_t, BlockPriceInfoPrice[66]) }
#endif			
#if gASL_ZclPrice_BlockPriceInfoNumber_d >= 68
	,{gZclAttrPrice_Tier4Block4Price_c, gZclDataTypeUint32_c, gZclAttrFlagsInRAM_c | gZclAttrFlagsRdOnly_c, sizeof(uint32_t), (void*)MbrOfs(sePriceAttrRAM_t, BlockPriceInfoPrice[67]) }
#endif			
#if gASL_ZclPrice_BlockPriceInfoNumber_d >= 69
	,{gZclAttrPrice_Tier4Block5Price_c, gZclDataTypeUint32_c, gZclAttrFlagsInRAM_c | gZclAttrFlagsRdOnly_c, sizeof(uint32_t), (void*)MbrOfs(sePriceAttrRAM_t, BlockPriceInfoPrice[68]) }
#endif			
#if gASL_ZclPrice_BlockPriceInfoNumber_d >= 70
	,{gZclAttrPrice_Tier4Block6Price_c, gZclDataTypeUint32_c, gZclAttrFlagsInRAM_c | gZclAttrFlagsRdOnly_c, sizeof(uint32_t), (void*)MbrOfs(sePriceAttrRAM_t, BlockPriceInfoPrice[69]) }
#endif			
#if gASL_ZclPrice_BlockPriceInfoNumber_d >= 71
	,{gZclAttrPrice_Tier4Block7Price_c, gZclDataTypeUint32_c, gZclAttrFlagsInRAM_c | gZclAttrFlagsRdOnly_c, sizeof(uint32_t), (void*)MbrOfs(sePriceAttrRAM_t, BlockPriceInfoPrice[70]) }
#endif			
#if gASL_ZclPrice_BlockPriceInfoNumber_d >= 72
	,{gZclAttrPrice_Tier4Block8Price_c, gZclDataTypeUint32_c, gZclAttrFlagsInRAM_c | gZclAttrFlagsRdOnly_c, sizeof(uint32_t), (void*)MbrOfs(sePriceAttrRAM_t, BlockPriceInfoPrice[71]) }
#endif			
#if gASL_ZclPrice_BlockPriceInfoNumber_d >= 73
	,{gZclAttrPrice_Tier4Block9Price_c, gZclDataTypeUint32_c, gZclAttrFlagsInRAM_c | gZclAttrFlagsRdOnly_c, sizeof(uint32_t), (void*)MbrOfs(sePriceAttrRAM_t, BlockPriceInfoPrice[72]) }
#endif			
#if gASL_ZclPrice_BlockPriceInfoNumber_d >= 74
	,{gZclAttrPrice_Tier4Block10Price_c, gZclDataTypeUint32_c, gZclAttrFlagsInRAM_c | gZclAttrFlagsRdOnly_c, sizeof(uint32_t), (void*)MbrOfs(sePriceAttrRAM_t, BlockPriceInfoPrice[73]) }
#endif			
#if gASL_ZclPrice_BlockPriceInfoNumber_d >= 75
	,{gZclAttrPrice_Tier4Block11Price_c, gZclDataTypeUint32_c, gZclAttrFlagsInRAM_c | gZclAttrFlagsRdOnly_c, sizeof(uint32_t), (void*)MbrOfs(sePriceAttrRAM_t, BlockPriceInfoPrice[74]) }
#endif			
#if gASL_ZclPrice_BlockPriceInfoNumber_d >= 76
	,{gZclAttrPrice_Tier4Block12Price_c, gZclDataTypeUint32_c, gZclAttrFlagsInRAM_c | gZclAttrFlagsRdOnly_c, sizeof(uint32_t), (void*)MbrOfs(sePriceAttrRAM_t, BlockPriceInfoPrice[75]) }
#endif			
#if gASL_ZclPrice_BlockPriceInfoNumber_d >= 77
	,{gZclAttrPrice_Tier4Block13Price_c, gZclDataTypeUint32_c, gZclAttrFlagsInRAM_c | gZclAttrFlagsRdOnly_c, sizeof(uint32_t), (void*)MbrOfs(sePriceAttrRAM_t, BlockPriceInfoPrice[76]) }
#endif			
#if gASL_ZclPrice_BlockPriceInfoNumber_d >= 78
	,{gZclAttrPrice_Tier4Block14Price_c, gZclDataTypeUint32_c, gZclAttrFlagsInRAM_c | gZclAttrFlagsRdOnly_c, sizeof(uint32_t), (void*)MbrOfs(sePriceAttrRAM_t, BlockPriceInfoPrice[77]) }
#endif			
#if gASL_ZclPrice_BlockPriceInfoNumber_d >= 79
	,{gZclAttrPrice_Tier4Block15Price_c, gZclDataTypeUint32_c, gZclAttrFlagsInRAM_c | gZclAttrFlagsRdOnly_c, sizeof(uint32_t), (void*)MbrOfs(sePriceAttrRAM_t, BlockPriceInfoPrice[78]) }
#endif			
#if gASL_ZclPrice_BlockPriceInfoNumber_d >= 80
	,{gZclAttrPrice_Tier4Block16Price_c, gZclDataTypeUint32_c, gZclAttrFlagsInRAM_c | gZclAttrFlagsRdOnly_c, sizeof(uint32_t), (void*)MbrOfs(sePriceAttrRAM_t, BlockPriceInfoPrice[79]) }
#endif			

#if gASL_ZclPrice_BlockPriceInfoNumber_d >= 81
	,{gZclAttrPrice_Tier5Block1Price_c, gZclDataTypeUint32_c, gZclAttrFlagsInRAM_c | gZclAttrFlagsRdOnly_c, sizeof(uint32_t), (void*)MbrOfs(sePriceAttrRAM_t, BlockPriceInfoPrice[80]) }
#endif			
#if gASL_ZclPrice_BlockPriceInfoNumber_d >= 82
	,{gZclAttrPrice_Tier5Block2Price_c, gZclDataTypeUint32_c, gZclAttrFlagsInRAM_c | gZclAttrFlagsRdOnly_c, sizeof(uint32_t), (void*)MbrOfs(sePriceAttrRAM_t, BlockPriceInfoPrice[81]) }
#endif			
#if gASL_ZclPrice_BlockPriceInfoNumber_d >= 83
	,{gZclAttrPrice_Tier5Block3Price_c, gZclDataTypeUint32_c, gZclAttrFlagsInRAM_c | gZclAttrFlagsRdOnly_c, sizeof(uint32_t), (void*)MbrOfs(sePriceAttrRAM_t, BlockPriceInfoPrice[82]) }
#endif			
#if gASL_ZclPrice_BlockPriceInfoNumber_d >= 84
	,{gZclAttrPrice_Tier5Block4Price_c, gZclDataTypeUint32_c, gZclAttrFlagsInRAM_c | gZclAttrFlagsRdOnly_c, sizeof(uint32_t), (void*)MbrOfs(sePriceAttrRAM_t, BlockPriceInfoPrice[83]) }
#endif			
#if gASL_ZclPrice_BlockPriceInfoNumber_d >= 85
	,{gZclAttrPrice_Tier5Block5Price_c, gZclDataTypeUint32_c, gZclAttrFlagsInRAM_c | gZclAttrFlagsRdOnly_c, sizeof(uint32_t), (void*)MbrOfs(sePriceAttrRAM_t, BlockPriceInfoPrice[84]) }
#endif			
#if gASL_ZclPrice_BlockPriceInfoNumber_d >= 86
	,{gZclAttrPrice_Tier5Block6Price_c, gZclDataTypeUint32_c, gZclAttrFlagsInRAM_c | gZclAttrFlagsRdOnly_c, sizeof(uint32_t), (void*)MbrOfs(sePriceAttrRAM_t, BlockPriceInfoPrice[85]) }
#endif			
#if gASL_ZclPrice_BlockPriceInfoNumber_d >= 87
	,{gZclAttrPrice_Tier5Block7Price_c, gZclDataTypeUint32_c, gZclAttrFlagsInRAM_c | gZclAttrFlagsRdOnly_c, sizeof(uint32_t), (void*)MbrOfs(sePriceAttrRAM_t, BlockPriceInfoPrice[86]) }
#endif			
#if gASL_ZclPrice_BlockPriceInfoNumber_d >= 88
	,{gZclAttrPrice_Tier5Block8Price_c, gZclDataTypeUint32_c, gZclAttrFlagsInRAM_c | gZclAttrFlagsRdOnly_c, sizeof(uint32_t), (void*)MbrOfs(sePriceAttrRAM_t, BlockPriceInfoPrice[87]) }
#endif			
#if gASL_ZclPrice_BlockPriceInfoNumber_d >= 89
	,{gZclAttrPrice_Tier5Block9Price_c, gZclDataTypeUint32_c, gZclAttrFlagsInRAM_c | gZclAttrFlagsRdOnly_c, sizeof(uint32_t), (void*)MbrOfs(sePriceAttrRAM_t, BlockPriceInfoPrice[88]) }
#endif			
#if gASL_ZclPrice_BlockPriceInfoNumber_d >= 90
	,{gZclAttrPrice_Tier5Block10Price_c, gZclDataTypeUint32_c, gZclAttrFlagsInRAM_c | gZclAttrFlagsRdOnly_c, sizeof(uint32_t), (void*)MbrOfs(sePriceAttrRAM_t, BlockPriceInfoPrice[89]) }
#endif			
#if gASL_ZclPrice_BlockPriceInfoNumber_d >= 91
	,{gZclAttrPrice_Tier5Block11Price_c, gZclDataTypeUint32_c, gZclAttrFlagsInRAM_c | gZclAttrFlagsRdOnly_c, sizeof(uint32_t), (void*)MbrOfs(sePriceAttrRAM_t, BlockPriceInfoPrice[90]) }
#endif			
#if gASL_ZclPrice_BlockPriceInfoNumber_d >= 92
	,{gZclAttrPrice_Tier5Block12Price_c, gZclDataTypeUint32_c, gZclAttrFlagsInRAM_c | gZclAttrFlagsRdOnly_c, sizeof(uint32_t), (void*)MbrOfs(sePriceAttrRAM_t, BlockPriceInfoPrice[91]) }
#endif			
#if gASL_ZclPrice_BlockPriceInfoNumber_d >= 93
	,{gZclAttrPrice_Tier5Block13Price_c, gZclDataTypeUint32_c, gZclAttrFlagsInRAM_c | gZclAttrFlagsRdOnly_c, sizeof(uint32_t), (void*)MbrOfs(sePriceAttrRAM_t, BlockPriceInfoPrice[92]) }
#endif			
#if gASL_ZclPrice_BlockPriceInfoNumber_d >= 94
	,{gZclAttrPrice_Tier5Block14Price_c, gZclDataTypeUint32_c, gZclAttrFlagsInRAM_c | gZclAttrFlagsRdOnly_c, sizeof(uint32_t), (void*)MbrOfs(sePriceAttrRAM_t, BlockPriceInfoPrice[93]) }
#endif			
#if gASL_ZclPrice_BlockPriceInfoNumber_d >= 95
	,{gZclAttrPrice_Tier5Block15Price_c, gZclDataTypeUint32_c, gZclAttrFlagsInRAM_c | gZclAttrFlagsRdOnly_c, sizeof(uint32_t), (void*)MbrOfs(sePriceAttrRAM_t, BlockPriceInfoPrice[94]) }
#endif			
#if gASL_ZclPrice_BlockPriceInfoNumber_d >= 96
	,{gZclAttrPrice_Tier5Block16Price_c, gZclDataTypeUint32_c, gZclAttrFlagsInRAM_c | gZclAttrFlagsRdOnly_c, sizeof(uint32_t), (void*)MbrOfs(sePriceAttrRAM_t, BlockPriceInfoPrice[95]) }
#endif			

#if gASL_ZclPrice_BlockPriceInfoNumber_d >= 97
	,{gZclAttrPrice_Tier6Block1Price_c, gZclDataTypeUint32_c, gZclAttrFlagsInRAM_c | gZclAttrFlagsRdOnly_c, sizeof(uint32_t), (void*)MbrOfs(sePriceAttrRAM_t, BlockPriceInfoPrice[96]) }
#endif			
#if gASL_ZclPrice_BlockPriceInfoNumber_d >= 98
	,{gZclAttrPrice_Tier6Block2Price_c, gZclDataTypeUint32_c, gZclAttrFlagsInRAM_c | gZclAttrFlagsRdOnly_c, sizeof(uint32_t), (void*)MbrOfs(sePriceAttrRAM_t, BlockPriceInfoPrice[97]) }
#endif			
#if gASL_ZclPrice_BlockPriceInfoNumber_d >= 99
	,{gZclAttrPrice_Tier6Block3Price_c, gZclDataTypeUint32_c, gZclAttrFlagsInRAM_c | gZclAttrFlagsRdOnly_c, sizeof(uint32_t), (void*)MbrOfs(sePriceAttrRAM_t, BlockPriceInfoPrice[98]) }
#endif			
#if gASL_ZclPrice_BlockPriceInfoNumber_d >= 100
	,{gZclAttrPrice_Tier6Block4Price_c, gZclDataTypeUint32_c, gZclAttrFlagsInRAM_c | gZclAttrFlagsRdOnly_c, sizeof(uint32_t), (void*)MbrOfs(sePriceAttrRAM_t, BlockPriceInfoPrice[99]) }
#endif			
#if gASL_ZclPrice_BlockPriceInfoNumber_d >= 101
	,{gZclAttrPrice_Tier6Block5Price_c, gZclDataTypeUint32_c, gZclAttrFlagsInRAM_c | gZclAttrFlagsRdOnly_c, sizeof(uint32_t), (void*)MbrOfs(sePriceAttrRAM_t, BlockPriceInfoPrice[100]) }
#endif			
#if gASL_ZclPrice_BlockPriceInfoNumber_d >= 102
	,{gZclAttrPrice_Tier6Block6Price_c, gZclDataTypeUint32_c, gZclAttrFlagsInRAM_c | gZclAttrFlagsRdOnly_c, sizeof(uint32_t), (void*)MbrOfs(sePriceAttrRAM_t, BlockPriceInfoPrice[101]) }
#endif			
#if gASL_ZclPrice_BlockPriceInfoNumber_d >= 103
	,{gZclAttrPrice_Tier6Block7Price_c, gZclDataTypeUint32_c, gZclAttrFlagsInRAM_c | gZclAttrFlagsRdOnly_c, sizeof(uint32_t), (void*)MbrOfs(sePriceAttrRAM_t, BlockPriceInfoPrice[102]) }
#endif			
#if gASL_ZclPrice_BlockPriceInfoNumber_d >= 104
	,{gZclAttrPrice_Tier6Block8Price_c, gZclDataTypeUint32_c, gZclAttrFlagsInRAM_c | gZclAttrFlagsRdOnly_c, sizeof(uint32_t), (void*)MbrOfs(sePriceAttrRAM_t, BlockPriceInfoPrice[103]) }
#endif			
#if gASL_ZclPrice_BlockPriceInfoNumber_d >= 105
	,{gZclAttrPrice_Tier6Block9Price_c, gZclDataTypeUint32_c, gZclAttrFlagsInRAM_c | gZclAttrFlagsRdOnly_c, sizeof(uint32_t), (void*)MbrOfs(sePriceAttrRAM_t, BlockPriceInfoPrice[104]) }
#endif			
#if gASL_ZclPrice_BlockPriceInfoNumber_d >= 106
	,{gZclAttrPrice_Tier6Block10Price_c, gZclDataTypeUint32_c, gZclAttrFlagsInRAM_c | gZclAttrFlagsRdOnly_c, sizeof(uint32_t), (void*)MbrOfs(sePriceAttrRAM_t, BlockPriceInfoPrice[105]) }
#endif			
#if gASL_ZclPrice_BlockPriceInfoNumber_d >= 107
	,{gZclAttrPrice_Tier6Block11Price_c, gZclDataTypeUint32_c, gZclAttrFlagsInRAM_c | gZclAttrFlagsRdOnly_c, sizeof(uint32_t), (void*)MbrOfs(sePriceAttrRAM_t, BlockPriceInfoPrice[106]) }
#endif			
#if gASL_ZclPrice_BlockPriceInfoNumber_d >= 108
	,{gZclAttrPrice_Tier6Block12Price_c, gZclDataTypeUint32_c, gZclAttrFlagsInRAM_c | gZclAttrFlagsRdOnly_c, sizeof(uint32_t), (void*)MbrOfs(sePriceAttrRAM_t, BlockPriceInfoPrice[107]) }
#endif			
#if gASL_ZclPrice_BlockPriceInfoNumber_d >= 109
	,{gZclAttrPrice_Tier6Block13Price_c, gZclDataTypeUint32_c, gZclAttrFlagsInRAM_c | gZclAttrFlagsRdOnly_c, sizeof(uint32_t), (void*)MbrOfs(sePriceAttrRAM_t, BlockPriceInfoPrice[108]) }
#endif			
#if gASL_ZclPrice_BlockPriceInfoNumber_d >= 110
	,{gZclAttrPrice_Tier6Block14Price_c, gZclDataTypeUint32_c, gZclAttrFlagsInRAM_c | gZclAttrFlagsRdOnly_c, sizeof(uint32_t), (void*)MbrOfs(sePriceAttrRAM_t, BlockPriceInfoPrice[109]) }
#endif			
#if gASL_ZclPrice_BlockPriceInfoNumber_d >= 111
	,{gZclAttrPrice_Tier6Block15Price_c, gZclDataTypeUint32_c, gZclAttrFlagsInRAM_c | gZclAttrFlagsRdOnly_c, sizeof(uint32_t), (void*)MbrOfs(sePriceAttrRAM_t, BlockPriceInfoPrice[110]) }
#endif			
#if gASL_ZclPrice_BlockPriceInfoNumber_d >= 112
	,{gZclAttrPrice_Tier6Block16Price_c, gZclDataTypeUint32_c, gZclAttrFlagsInRAM_c | gZclAttrFlagsRdOnly_c, sizeof(uint32_t), (void*)MbrOfs(sePriceAttrRAM_t, BlockPriceInfoPrice[111]) }
#endif			

#if gASL_ZclPrice_BlockPriceInfoNumber_d >= 113
	,{gZclAttrPrice_Tier7Block1Price_c, gZclDataTypeUint32_c, gZclAttrFlagsInRAM_c | gZclAttrFlagsRdOnly_c, sizeof(uint32_t), (void*)MbrOfs(sePriceAttrRAM_t, BlockPriceInfoPrice[112]) }
#endif			
#if gASL_ZclPrice_BlockPriceInfoNumber_d >= 114
	,{gZclAttrPrice_Tier7Block2Price_c, gZclDataTypeUint32_c, gZclAttrFlagsInRAM_c | gZclAttrFlagsRdOnly_c, sizeof(uint32_t), (void*)MbrOfs(sePriceAttrRAM_t, BlockPriceInfoPrice[113]) }
#endif			
#if gASL_ZclPrice_BlockPriceInfoNumber_d >= 115
	,{gZclAttrPrice_Tier7Block3Price_c, gZclDataTypeUint32_c, gZclAttrFlagsInRAM_c | gZclAttrFlagsRdOnly_c, sizeof(uint32_t), (void*)MbrOfs(sePriceAttrRAM_t, BlockPriceInfoPrice[114]) }
#endif			
#if gASL_ZclPrice_BlockPriceInfoNumber_d >= 116
	,{gZclAttrPrice_Tier7Block4Price_c, gZclDataTypeUint32_c, gZclAttrFlagsInRAM_c | gZclAttrFlagsRdOnly_c, sizeof(uint32_t), (void*)MbrOfs(sePriceAttrRAM_t, BlockPriceInfoPrice[115]) }
#endif			
#if gASL_ZclPrice_BlockPriceInfoNumber_d >= 117
	,{gZclAttrPrice_Tier7Block5Price_c, gZclDataTypeUint32_c, gZclAttrFlagsInRAM_c | gZclAttrFlagsRdOnly_c, sizeof(uint32_t), (void*)MbrOfs(sePriceAttrRAM_t, BlockPriceInfoPrice[116]) }
#endif			
#if gASL_ZclPrice_BlockPriceInfoNumber_d >= 118
	,{gZclAttrPrice_Tier7Block6Price_c, gZclDataTypeUint32_c, gZclAttrFlagsInRAM_c | gZclAttrFlagsRdOnly_c, sizeof(uint32_t), (void*)MbrOfs(sePriceAttrRAM_t, BlockPriceInfoPrice[117]) }
#endif			
#if gASL_ZclPrice_BlockPriceInfoNumber_d >= 119
	,{gZclAttrPrice_Tier7Block7Price_c, gZclDataTypeUint32_c, gZclAttrFlagsInRAM_c | gZclAttrFlagsRdOnly_c, sizeof(uint32_t), (void*)MbrOfs(sePriceAttrRAM_t, BlockPriceInfoPrice[118]) }
#endif			
#if gASL_ZclPrice_BlockPriceInfoNumber_d >= 120
	,{gZclAttrPrice_Tier7Block8Price_c, gZclDataTypeUint32_c, gZclAttrFlagsInRAM_c | gZclAttrFlagsRdOnly_c, sizeof(uint32_t), (void*)MbrOfs(sePriceAttrRAM_t, BlockPriceInfoPrice[119]) }
#endif			
#if gASL_ZclPrice_BlockPriceInfoNumber_d >= 121
	,{gZclAttrPrice_Tier7Block9Price_c, gZclDataTypeUint32_c, gZclAttrFlagsInRAM_c | gZclAttrFlagsRdOnly_c, sizeof(uint32_t), (void*)MbrOfs(sePriceAttrRAM_t, BlockPriceInfoPrice[120]) }
#endif			
#if gASL_ZclPrice_BlockPriceInfoNumber_d >= 122
	,{gZclAttrPrice_Tier7Block10Price_c, gZclDataTypeUint32_c, gZclAttrFlagsInRAM_c | gZclAttrFlagsRdOnly_c, sizeof(uint32_t), (void*)MbrOfs(sePriceAttrRAM_t, BlockPriceInfoPrice[121]) }
#endif			
#if gASL_ZclPrice_BlockPriceInfoNumber_d >= 123
	,{gZclAttrPrice_Tier7Block11Price_c, gZclDataTypeUint32_c, gZclAttrFlagsInRAM_c | gZclAttrFlagsRdOnly_c, sizeof(uint32_t), (void*)MbrOfs(sePriceAttrRAM_t, BlockPriceInfoPrice[122]) }
#endif			
#if gASL_ZclPrice_BlockPriceInfoNumber_d >= 124
	,{gZclAttrPrice_Tier7Block12Price_c, gZclDataTypeUint32_c, gZclAttrFlagsInRAM_c | gZclAttrFlagsRdOnly_c, sizeof(uint32_t), (void*)MbrOfs(sePriceAttrRAM_t, BlockPriceInfoPrice[123]) }
#endif			
#if gASL_ZclPrice_BlockPriceInfoNumber_d >= 125
	,{gZclAttrPrice_Tier7Block13Price_c, gZclDataTypeUint32_c, gZclAttrFlagsInRAM_c | gZclAttrFlagsRdOnly_c, sizeof(uint32_t), (void*)MbrOfs(sePriceAttrRAM_t, BlockPriceInfoPrice[124]) }
#endif			
#if gASL_ZclPrice_BlockPriceInfoNumber_d >= 126
	,{gZclAttrPrice_Tier7Block14Price_c, gZclDataTypeUint32_c, gZclAttrFlagsInRAM_c | gZclAttrFlagsRdOnly_c, sizeof(uint32_t), (void*)MbrOfs(sePriceAttrRAM_t, BlockPriceInfoPrice[125]) }
#endif			
#if gASL_ZclPrice_BlockPriceInfoNumber_d >= 127
	,{gZclAttrPrice_Tier7Block15Price_c, gZclDataTypeUint32_c, gZclAttrFlagsInRAM_c | gZclAttrFlagsRdOnly_c, sizeof(uint32_t), (void*)MbrOfs(sePriceAttrRAM_t, BlockPriceInfoPrice[126]) }
#endif			
#if gASL_ZclPrice_BlockPriceInfoNumber_d >= 128
	,{gZclAttrPrice_Tier7Block16Price_c, gZclDataTypeUint32_c, gZclAttrFlagsInRAM_c | gZclAttrFlagsRdOnly_c, sizeof(uint32_t), (void*)MbrOfs(sePriceAttrRAM_t, BlockPriceInfoPrice[127]) }
#endif			

#if gASL_ZclPrice_BlockPriceInfoNumber_d >= 129
	,{gZclAttrPrice_Tier8Block1Price_c, gZclDataTypeUint32_c, gZclAttrFlagsInRAM_c | gZclAttrFlagsRdOnly_c, sizeof(uint32_t), (void*)MbrOfs(sePriceAttrRAM_t, BlockPriceInfoPrice[128]) }
#endif			
#if gASL_ZclPrice_BlockPriceInfoNumber_d >= 130
	,{gZclAttrPrice_Tier8Block2Price_c, gZclDataTypeUint32_c, gZclAttrFlagsInRAM_c | gZclAttrFlagsRdOnly_c, sizeof(uint32_t), (void*)MbrOfs(sePriceAttrRAM_t, BlockPriceInfoPrice[129]) }
#endif			
#if gASL_ZclPrice_BlockPriceInfoNumber_d >= 131
	,{gZclAttrPrice_Tier8Block3Price_c, gZclDataTypeUint32_c, gZclAttrFlagsInRAM_c | gZclAttrFlagsRdOnly_c, sizeof(uint32_t), (void*)MbrOfs(sePriceAttrRAM_t, BlockPriceInfoPrice[130]) }
#endif			
#if gASL_ZclPrice_BlockPriceInfoNumber_d >= 132
	,{gZclAttrPrice_Tier8Block4Price_c, gZclDataTypeUint32_c, gZclAttrFlagsInRAM_c | gZclAttrFlagsRdOnly_c, sizeof(uint32_t), (void*)MbrOfs(sePriceAttrRAM_t, BlockPriceInfoPrice[131]) }
#endif			
#if gASL_ZclPrice_BlockPriceInfoNumber_d >= 133
	,{gZclAttrPrice_Tier8Block5Price_c, gZclDataTypeUint32_c, gZclAttrFlagsInRAM_c | gZclAttrFlagsRdOnly_c, sizeof(uint32_t), (void*)MbrOfs(sePriceAttrRAM_t, BlockPriceInfoPrice[132]) }
#endif			
#if gASL_ZclPrice_BlockPriceInfoNumber_d >= 134
	,{gZclAttrPrice_Tier8Block6Price_c, gZclDataTypeUint32_c, gZclAttrFlagsInRAM_c | gZclAttrFlagsRdOnly_c, sizeof(uint32_t), (void*)MbrOfs(sePriceAttrRAM_t, BlockPriceInfoPrice[133]) }
#endif			
#if gASL_ZclPrice_BlockPriceInfoNumber_d >= 135
	,{gZclAttrPrice_Tier8Block7Price_c, gZclDataTypeUint32_c, gZclAttrFlagsInRAM_c | gZclAttrFlagsRdOnly_c, sizeof(uint32_t), (void*)MbrOfs(sePriceAttrRAM_t, BlockPriceInfoPrice[134]) }
#endif			
#if gASL_ZclPrice_BlockPriceInfoNumber_d >= 136
	,{gZclAttrPrice_Tier8Block8Price_c, gZclDataTypeUint32_c, gZclAttrFlagsInRAM_c | gZclAttrFlagsRdOnly_c, sizeof(uint32_t), (void*)MbrOfs(sePriceAttrRAM_t, BlockPriceInfoPrice[135]) }
#endif			
#if gASL_ZclPrice_BlockPriceInfoNumber_d >= 137
	,{gZclAttrPrice_Tier8Block9Price_c, gZclDataTypeUint32_c, gZclAttrFlagsInRAM_c | gZclAttrFlagsRdOnly_c, sizeof(uint32_t), (void*)MbrOfs(sePriceAttrRAM_t, BlockPriceInfoPrice[136]) }
#endif			
#if gASL_ZclPrice_BlockPriceInfoNumber_d >= 138
	,{gZclAttrPrice_Tier8Block10Price_c, gZclDataTypeUint32_c, gZclAttrFlagsInRAM_c | gZclAttrFlagsRdOnly_c, sizeof(uint32_t), (void*)MbrOfs(sePriceAttrRAM_t, BlockPriceInfoPrice[137]) }
#endif			
#if gASL_ZclPrice_BlockPriceInfoNumber_d >= 139
	,{gZclAttrPrice_Tier8Block11Price_c, gZclDataTypeUint32_c, gZclAttrFlagsInRAM_c | gZclAttrFlagsRdOnly_c, sizeof(uint32_t), (void*)MbrOfs(sePriceAttrRAM_t, BlockPriceInfoPrice[138]) }
#endif			
#if gASL_ZclPrice_BlockPriceInfoNumber_d >= 140
	,{gZclAttrPrice_Tier8Block12Price_c, gZclDataTypeUint32_c, gZclAttrFlagsInRAM_c | gZclAttrFlagsRdOnly_c, sizeof(uint32_t), (void*)MbrOfs(sePriceAttrRAM_t, BlockPriceInfoPrice[139]) }
#endif			
#if gASL_ZclPrice_BlockPriceInfoNumber_d >= 141
	,{gZclAttrPrice_Tier8Block13Price_c, gZclDataTypeUint32_c, gZclAttrFlagsInRAM_c | gZclAttrFlagsRdOnly_c, sizeof(uint32_t), (void*)MbrOfs(sePriceAttrRAM_t, BlockPriceInfoPrice[140]) }
#endif			
#if gASL_ZclPrice_BlockPriceInfoNumber_d >= 142
	,{gZclAttrPrice_Tier8Block14Price_c, gZclDataTypeUint32_c, gZclAttrFlagsInRAM_c | gZclAttrFlagsRdOnly_c, sizeof(uint32_t), (void*)MbrOfs(sePriceAttrRAM_t, BlockPriceInfoPrice[141]) }
#endif			
#if gASL_ZclPrice_BlockPriceInfoNumber_d >= 143
	,{gZclAttrPrice_Tier8Block15Price_c, gZclDataTypeUint32_c, gZclAttrFlagsInRAM_c | gZclAttrFlagsRdOnly_c, sizeof(uint32_t), (void*)MbrOfs(sePriceAttrRAM_t, BlockPriceInfoPrice[142]) }
#endif			
#if gASL_ZclPrice_BlockPriceInfoNumber_d >= 144
	,{gZclAttrPrice_Tier8Block16Price_c, gZclDataTypeUint32_c, gZclAttrFlagsInRAM_c | gZclAttrFlagsRdOnly_c, sizeof(uint32_t), (void*)MbrOfs(sePriceAttrRAM_t, BlockPriceInfoPrice[143]) }
#endif			

#if gASL_ZclPrice_BlockPriceInfoNumber_d >= 145
	,{gZclAttrPrice_Tier9Block1Price_c, gZclDataTypeUint32_c, gZclAttrFlagsInRAM_c | gZclAttrFlagsRdOnly_c, sizeof(uint32_t), (void*)MbrOfs(sePriceAttrRAM_t, BlockPriceInfoPrice[144]) }
#endif			
#if gASL_ZclPrice_BlockPriceInfoNumber_d >= 146
	,{gZclAttrPrice_Tier9Block2Price_c, gZclDataTypeUint32_c, gZclAttrFlagsInRAM_c | gZclAttrFlagsRdOnly_c, sizeof(uint32_t), (void*)MbrOfs(sePriceAttrRAM_t, BlockPriceInfoPrice[145]) }
#endif			
#if gASL_ZclPrice_BlockPriceInfoNumber_d >= 147
	,{gZclAttrPrice_Tier9Block3Price_c, gZclDataTypeUint32_c, gZclAttrFlagsInRAM_c | gZclAttrFlagsRdOnly_c, sizeof(uint32_t), (void*)MbrOfs(sePriceAttrRAM_t, BlockPriceInfoPrice[146]) }
#endif			
#if gASL_ZclPrice_BlockPriceInfoNumber_d >= 148
	,{gZclAttrPrice_Tier9Block4Price_c, gZclDataTypeUint32_c, gZclAttrFlagsInRAM_c | gZclAttrFlagsRdOnly_c, sizeof(uint32_t), (void*)MbrOfs(sePriceAttrRAM_t, BlockPriceInfoPrice[147]) }
#endif			
#if gASL_ZclPrice_BlockPriceInfoNumber_d >= 149
	,{gZclAttrPrice_Tier9Block5Price_c, gZclDataTypeUint32_c, gZclAttrFlagsInRAM_c | gZclAttrFlagsRdOnly_c, sizeof(uint32_t), (void*)MbrOfs(sePriceAttrRAM_t, BlockPriceInfoPrice[148]) }
#endif			
#if gASL_ZclPrice_BlockPriceInfoNumber_d >= 150
	,{gZclAttrPrice_Tier9Block6Price_c, gZclDataTypeUint32_c, gZclAttrFlagsInRAM_c | gZclAttrFlagsRdOnly_c, sizeof(uint32_t), (void*)MbrOfs(sePriceAttrRAM_t, BlockPriceInfoPrice[149]) }
#endif			
#if gASL_ZclPrice_BlockPriceInfoNumber_d >= 151
	,{gZclAttrPrice_Tier9Block7Price_c, gZclDataTypeUint32_c, gZclAttrFlagsInRAM_c | gZclAttrFlagsRdOnly_c, sizeof(uint32_t), (void*)MbrOfs(sePriceAttrRAM_t, BlockPriceInfoPrice[150]) }
#endif			
#if gASL_ZclPrice_BlockPriceInfoNumber_d >= 152
	,{gZclAttrPrice_Tier9Block8Price_c, gZclDataTypeUint32_c, gZclAttrFlagsInRAM_c | gZclAttrFlagsRdOnly_c, sizeof(uint32_t), (void*)MbrOfs(sePriceAttrRAM_t, BlockPriceInfoPrice[151]) }
#endif			
#if gASL_ZclPrice_BlockPriceInfoNumber_d >= 153
	,{gZclAttrPrice_Tier9Block9Price_c, gZclDataTypeUint32_c, gZclAttrFlagsInRAM_c | gZclAttrFlagsRdOnly_c, sizeof(uint32_t), (void*)MbrOfs(sePriceAttrRAM_t, BlockPriceInfoPrice[152]) }
#endif			
#if gASL_ZclPrice_BlockPriceInfoNumber_d >= 154
	,{gZclAttrPrice_Tier9Block10Price_c, gZclDataTypeUint32_c, gZclAttrFlagsInRAM_c | gZclAttrFlagsRdOnly_c, sizeof(uint32_t), (void*)MbrOfs(sePriceAttrRAM_t, BlockPriceInfoPrice[153]) }
#endif			
#if gASL_ZclPrice_BlockPriceInfoNumber_d >= 155
	,{gZclAttrPrice_Tier9Block11Price_c, gZclDataTypeUint32_c, gZclAttrFlagsInRAM_c | gZclAttrFlagsRdOnly_c, sizeof(uint32_t), (void*)MbrOfs(sePriceAttrRAM_t, BlockPriceInfoPrice[154]) }
#endif			
#if gASL_ZclPrice_BlockPriceInfoNumber_d >= 156
	,{gZclAttrPrice_Tier9Block12Price_c, gZclDataTypeUint32_c, gZclAttrFlagsInRAM_c | gZclAttrFlagsRdOnly_c, sizeof(uint32_t), (void*)MbrOfs(sePriceAttrRAM_t, BlockPriceInfoPrice[155]) }
#endif			
#if gASL_ZclPrice_BlockPriceInfoNumber_d >= 157
	,{gZclAttrPrice_Tier9Block13Price_c, gZclDataTypeUint32_c, gZclAttrFlagsInRAM_c | gZclAttrFlagsRdOnly_c, sizeof(uint32_t), (void*)MbrOfs(sePriceAttrRAM_t, BlockPriceInfoPrice[156]) }
#endif			
#if gASL_ZclPrice_BlockPriceInfoNumber_d >= 158
	,{gZclAttrPrice_Tier9Block14Price_c, gZclDataTypeUint32_c, gZclAttrFlagsInRAM_c | gZclAttrFlagsRdOnly_c, sizeof(uint32_t), (void*)MbrOfs(sePriceAttrRAM_t, BlockPriceInfoPrice[157]) }
#endif			
#if gASL_ZclPrice_BlockPriceInfoNumber_d >= 159
	,{gZclAttrPrice_Tier9Block15Price_c, gZclDataTypeUint32_c, gZclAttrFlagsInRAM_c | gZclAttrFlagsRdOnly_c, sizeof(uint32_t), (void*)MbrOfs(sePriceAttrRAM_t, BlockPriceInfoPrice[158]) }
#endif			
#if gASL_ZclPrice_BlockPriceInfoNumber_d >= 160
	,{gZclAttrPrice_Tier9Block16Price_c, gZclDataTypeUint32_c, gZclAttrFlagsInRAM_c | gZclAttrFlagsRdOnly_c, sizeof(uint32_t), (void*)MbrOfs(sePriceAttrRAM_t, BlockPriceInfoPrice[159]) }
#endif			

#if gASL_ZclPrice_BlockPriceInfoNumber_d >= 161
	,{gZclAttrPrice_Tier10Block1Price_c, gZclDataTypeUint32_c, gZclAttrFlagsInRAM_c | gZclAttrFlagsRdOnly_c, sizeof(uint32_t), (void*)MbrOfs(sePriceAttrRAM_t, BlockPriceInfoPrice[160]) }
#endif			
#if gASL_ZclPrice_BlockPriceInfoNumber_d >= 162
	,{gZclAttrPrice_Tier10Block2Price_c, gZclDataTypeUint32_c, gZclAttrFlagsInRAM_c | gZclAttrFlagsRdOnly_c, sizeof(uint32_t), (void*)MbrOfs(sePriceAttrRAM_t, BlockPriceInfoPrice[161]) }
#endif			
#if gASL_ZclPrice_BlockPriceInfoNumber_d >= 163
	,{gZclAttrPrice_Tier10Block3Price_c, gZclDataTypeUint32_c, gZclAttrFlagsInRAM_c | gZclAttrFlagsRdOnly_c, sizeof(uint32_t), (void*)MbrOfs(sePriceAttrRAM_t, BlockPriceInfoPrice[162]) }
#endif			
#if gASL_ZclPrice_BlockPriceInfoNumber_d >= 164
	,{gZclAttrPrice_Tier10Block4Price_c, gZclDataTypeUint32_c, gZclAttrFlagsInRAM_c | gZclAttrFlagsRdOnly_c, sizeof(uint32_t), (void*)MbrOfs(sePriceAttrRAM_t, BlockPriceInfoPrice[163]) }
#endif			
#if gASL_ZclPrice_BlockPriceInfoNumber_d >= 165
	,{gZclAttrPrice_Tier10Block5Price_c, gZclDataTypeUint32_c, gZclAttrFlagsInRAM_c | gZclAttrFlagsRdOnly_c, sizeof(uint32_t), (void*)MbrOfs(sePriceAttrRAM_t, BlockPriceInfoPrice[164]) }
#endif			
#if gASL_ZclPrice_BlockPriceInfoNumber_d >= 166
	,{gZclAttrPrice_Tier10Block6Price_c, gZclDataTypeUint32_c, gZclAttrFlagsInRAM_c | gZclAttrFlagsRdOnly_c, sizeof(uint32_t), (void*)MbrOfs(sePriceAttrRAM_t, BlockPriceInfoPrice[165]) }
#endif			
#if gASL_ZclPrice_BlockPriceInfoNumber_d >= 167
	,{gZclAttrPrice_Tier10Block7Price_c, gZclDataTypeUint32_c, gZclAttrFlagsInRAM_c | gZclAttrFlagsRdOnly_c, sizeof(uint32_t), (void*)MbrOfs(sePriceAttrRAM_t, BlockPriceInfoPrice[166]) }
#endif			
#if gASL_ZclPrice_BlockPriceInfoNumber_d >= 168
	,{gZclAttrPrice_Tier10Block8Price_c, gZclDataTypeUint32_c, gZclAttrFlagsInRAM_c | gZclAttrFlagsRdOnly_c, sizeof(uint32_t), (void*)MbrOfs(sePriceAttrRAM_t, BlockPriceInfoPrice[167]) }
#endif			
#if gASL_ZclPrice_BlockPriceInfoNumber_d >= 169
	,{gZclAttrPrice_Tier10Block9Price_c, gZclDataTypeUint32_c, gZclAttrFlagsInRAM_c | gZclAttrFlagsRdOnly_c, sizeof(uint32_t), (void*)MbrOfs(sePriceAttrRAM_t, BlockPriceInfoPrice[168]) }
#endif			
#if gASL_ZclPrice_BlockPriceInfoNumber_d >= 170
	,{gZclAttrPrice_Tier10Block10Price_c, gZclDataTypeUint32_c, gZclAttrFlagsInRAM_c | gZclAttrFlagsRdOnly_c, sizeof(uint32_t), (void*)MbrOfs(sePriceAttrRAM_t, BlockPriceInfoPrice[169]) }
#endif			
#if gASL_ZclPrice_BlockPriceInfoNumber_d >= 171
	,{gZclAttrPrice_Tier10Block11Price_c, gZclDataTypeUint32_c, gZclAttrFlagsInRAM_c | gZclAttrFlagsRdOnly_c, sizeof(uint32_t), (void*)MbrOfs(sePriceAttrRAM_t, BlockPriceInfoPrice[170]) }
#endif			
#if gASL_ZclPrice_BlockPriceInfoNumber_d >= 172
	,{gZclAttrPrice_Tier10Block12Price_c, gZclDataTypeUint32_c, gZclAttrFlagsInRAM_c | gZclAttrFlagsRdOnly_c, sizeof(uint32_t), (void*)MbrOfs(sePriceAttrRAM_t, BlockPriceInfoPrice[171]) }
#endif			
#if gASL_ZclPrice_BlockPriceInfoNumber_d >= 173
	,{gZclAttrPrice_Tier10Block13Price_c, gZclDataTypeUint32_c, gZclAttrFlagsInRAM_c | gZclAttrFlagsRdOnly_c, sizeof(uint32_t), (void*)MbrOfs(sePriceAttrRAM_t, BlockPriceInfoPrice[172]) }
#endif			
#if gASL_ZclPrice_BlockPriceInfoNumber_d >= 174
	,{gZclAttrPrice_Tier10Block14Price_c, gZclDataTypeUint32_c, gZclAttrFlagsInRAM_c | gZclAttrFlagsRdOnly_c, sizeof(uint32_t), (void*)MbrOfs(sePriceAttrRAM_t, BlockPriceInfoPrice[173]) }
#endif			
#if gASL_ZclPrice_BlockPriceInfoNumber_d >= 175
	,{gZclAttrPrice_Tier10Block15Price_c, gZclDataTypeUint32_c, gZclAttrFlagsInRAM_c | gZclAttrFlagsRdOnly_c, sizeof(uint32_t), (void*)MbrOfs(sePriceAttrRAM_t, BlockPriceInfoPrice[174]) }
#endif			
#if gASL_ZclPrice_BlockPriceInfoNumber_d >= 176
	,{gZclAttrPrice_Tier10Block16Price_c, gZclDataTypeUint32_c, gZclAttrFlagsInRAM_c | gZclAttrFlagsRdOnly_c, sizeof(uint32_t), (void*)MbrOfs(sePriceAttrRAM_t, BlockPriceInfoPrice[175]) }
#endif			

#if gASL_ZclPrice_BlockPriceInfoNumber_d >= 177
	,{gZclAttrPrice_Tier11Block1Price_c, gZclDataTypeUint32_c, gZclAttrFlagsInRAM_c | gZclAttrFlagsRdOnly_c, sizeof(uint32_t), (void*)MbrOfs(sePriceAttrRAM_t, BlockPriceInfoPrice[176]) }
#endif			
#if gASL_ZclPrice_BlockPriceInfoNumber_d >= 178
	,{gZclAttrPrice_Tier11Block2Price_c, gZclDataTypeUint32_c, gZclAttrFlagsInRAM_c | gZclAttrFlagsRdOnly_c, sizeof(uint32_t), (void*)MbrOfs(sePriceAttrRAM_t, BlockPriceInfoPrice[177]) }
#endif			
#if gASL_ZclPrice_BlockPriceInfoNumber_d >= 179
	,{gZclAttrPrice_Tier11Block3Price_c, gZclDataTypeUint32_c, gZclAttrFlagsInRAM_c | gZclAttrFlagsRdOnly_c, sizeof(uint32_t), (void*)MbrOfs(sePriceAttrRAM_t, BlockPriceInfoPrice[178]) }
#endif			
#if gASL_ZclPrice_BlockPriceInfoNumber_d >= 180
	,{gZclAttrPrice_Tier11Block4Price_c, gZclDataTypeUint32_c, gZclAttrFlagsInRAM_c | gZclAttrFlagsRdOnly_c, sizeof(uint32_t), (void*)MbrOfs(sePriceAttrRAM_t, BlockPriceInfoPrice[179]) }
#endif			
#if gASL_ZclPrice_BlockPriceInfoNumber_d >= 181
	,{gZclAttrPrice_Tier11Block5Price_c, gZclDataTypeUint32_c, gZclAttrFlagsInRAM_c | gZclAttrFlagsRdOnly_c, sizeof(uint32_t), (void*)MbrOfs(sePriceAttrRAM_t, BlockPriceInfoPrice[180]) }
#endif			
#if gASL_ZclPrice_BlockPriceInfoNumber_d >= 182
	,{gZclAttrPrice_Tier11Block6Price_c, gZclDataTypeUint32_c, gZclAttrFlagsInRAM_c | gZclAttrFlagsRdOnly_c, sizeof(uint32_t), (void*)MbrOfs(sePriceAttrRAM_t, BlockPriceInfoPrice[181]) }
#endif			
#if gASL_ZclPrice_BlockPriceInfoNumber_d >= 183
	,{gZclAttrPrice_Tier11Block7Price_c, gZclDataTypeUint32_c, gZclAttrFlagsInRAM_c | gZclAttrFlagsRdOnly_c, sizeof(uint32_t), (void*)MbrOfs(sePriceAttrRAM_t, BlockPriceInfoPrice[182]) }
#endif			
#if gASL_ZclPrice_BlockPriceInfoNumber_d >= 184
	,{gZclAttrPrice_Tier11Block8Price_c, gZclDataTypeUint32_c, gZclAttrFlagsInRAM_c | gZclAttrFlagsRdOnly_c, sizeof(uint32_t), (void*)MbrOfs(sePriceAttrRAM_t, BlockPriceInfoPrice[183]) }
#endif			
#if gASL_ZclPrice_BlockPriceInfoNumber_d >= 185
	,{gZclAttrPrice_Tier11Block9Price_c, gZclDataTypeUint32_c, gZclAttrFlagsInRAM_c | gZclAttrFlagsRdOnly_c, sizeof(uint32_t), (void*)MbrOfs(sePriceAttrRAM_t, BlockPriceInfoPrice[184]) }
#endif			
#if gASL_ZclPrice_BlockPriceInfoNumber_d >= 186
	,{gZclAttrPrice_Tier11Block10Price_c, gZclDataTypeUint32_c, gZclAttrFlagsInRAM_c | gZclAttrFlagsRdOnly_c, sizeof(uint32_t), (void*)MbrOfs(sePriceAttrRAM_t, BlockPriceInfoPrice[185]) }
#endif			
#if gASL_ZclPrice_BlockPriceInfoNumber_d >= 187
	,{gZclAttrPrice_Tier11Block11Price_c, gZclDataTypeUint32_c, gZclAttrFlagsInRAM_c | gZclAttrFlagsRdOnly_c, sizeof(uint32_t), (void*)MbrOfs(sePriceAttrRAM_t, BlockPriceInfoPrice[186]) }
#endif			
#if gASL_ZclPrice_BlockPriceInfoNumber_d >= 188
	,{gZclAttrPrice_Tier11Block12Price_c, gZclDataTypeUint32_c, gZclAttrFlagsInRAM_c | gZclAttrFlagsRdOnly_c, sizeof(uint32_t), (void*)MbrOfs(sePriceAttrRAM_t, BlockPriceInfoPrice[187]) }
#endif			
#if gASL_ZclPrice_BlockPriceInfoNumber_d >= 189
	,{gZclAttrPrice_Tier11Block13Price_c, gZclDataTypeUint32_c, gZclAttrFlagsInRAM_c | gZclAttrFlagsRdOnly_c, sizeof(uint32_t), (void*)MbrOfs(sePriceAttrRAM_t, BlockPriceInfoPrice[188]) }
#endif			
#if gASL_ZclPrice_BlockPriceInfoNumber_d >= 190
	,{gZclAttrPrice_Tier11Block14Price_c, gZclDataTypeUint32_c, gZclAttrFlagsInRAM_c | gZclAttrFlagsRdOnly_c, sizeof(uint32_t), (void*)MbrOfs(sePriceAttrRAM_t, BlockPriceInfoPrice[189]) }
#endif			
#if gASL_ZclPrice_BlockPriceInfoNumber_d >= 191
	,{gZclAttrPrice_Tier11Block15Price_c, gZclDataTypeUint32_c, gZclAttrFlagsInRAM_c | gZclAttrFlagsRdOnly_c, sizeof(uint32_t), (void*)MbrOfs(sePriceAttrRAM_t, BlockPriceInfoPrice[190]) }
#endif			
#if gASL_ZclPrice_BlockPriceInfoNumber_d >= 192
	,{gZclAttrPrice_Tier11Block16Price_c, gZclDataTypeUint32_c, gZclAttrFlagsInRAM_c | gZclAttrFlagsRdOnly_c, sizeof(uint32_t), (void*)MbrOfs(sePriceAttrRAM_t, BlockPriceInfoPrice[191]) }
#endif			

#if gASL_ZclPrice_BlockPriceInfoNumber_d >= 193
	,{gZclAttrPrice_Tier12Block1Price_c, gZclDataTypeUint32_c, gZclAttrFlagsInRAM_c | gZclAttrFlagsRdOnly_c, sizeof(uint32_t), (void*)MbrOfs(sePriceAttrRAM_t, BlockPriceInfoPrice[192]) }
#endif			
#if gASL_ZclPrice_BlockPriceInfoNumber_d >= 194
	,{gZclAttrPrice_Tier12Block2Price_c, gZclDataTypeUint32_c, gZclAttrFlagsInRAM_c | gZclAttrFlagsRdOnly_c, sizeof(uint32_t), (void*)MbrOfs(sePriceAttrRAM_t, BlockPriceInfoPrice[193]) }
#endif			
#if gASL_ZclPrice_BlockPriceInfoNumber_d >= 195
	,{gZclAttrPrice_Tier12Block3Price_c, gZclDataTypeUint32_c, gZclAttrFlagsInRAM_c | gZclAttrFlagsRdOnly_c, sizeof(uint32_t), (void*)MbrOfs(sePriceAttrRAM_t, BlockPriceInfoPrice[194]) }
#endif			
#if gASL_ZclPrice_BlockPriceInfoNumber_d >= 196
	,{gZclAttrPrice_Tier12Block4Price_c, gZclDataTypeUint32_c, gZclAttrFlagsInRAM_c | gZclAttrFlagsRdOnly_c, sizeof(uint32_t), (void*)MbrOfs(sePriceAttrRAM_t, BlockPriceInfoPrice[195]) }
#endif			
#if gASL_ZclPrice_BlockPriceInfoNumber_d >= 197
	,{gZclAttrPrice_Tier12Block5Price_c, gZclDataTypeUint32_c, gZclAttrFlagsInRAM_c | gZclAttrFlagsRdOnly_c, sizeof(uint32_t), (void*)MbrOfs(sePriceAttrRAM_t, BlockPriceInfoPrice[196]) }
#endif			
#if gASL_ZclPrice_BlockPriceInfoNumber_d >= 198
	,{gZclAttrPrice_Tier12Block6Price_c, gZclDataTypeUint32_c, gZclAttrFlagsInRAM_c | gZclAttrFlagsRdOnly_c, sizeof(uint32_t), (void*)MbrOfs(sePriceAttrRAM_t, BlockPriceInfoPrice[197]) }
#endif			
#if gASL_ZclPrice_BlockPriceInfoNumber_d >= 199
	,{gZclAttrPrice_Tier12Block7Price_c, gZclDataTypeUint32_c, gZclAttrFlagsInRAM_c | gZclAttrFlagsRdOnly_c, sizeof(uint32_t), (void*)MbrOfs(sePriceAttrRAM_t, BlockPriceInfoPrice[198]) }
#endif			
#if gASL_ZclPrice_BlockPriceInfoNumber_d >= 200
	,{gZclAttrPrice_Tier12Block8Price_c, gZclDataTypeUint32_c, gZclAttrFlagsInRAM_c | gZclAttrFlagsRdOnly_c, sizeof(uint32_t), (void*)MbrOfs(sePriceAttrRAM_t, BlockPriceInfoPrice[199]) }
#endif			
#if gASL_ZclPrice_BlockPriceInfoNumber_d >= 201
	,{gZclAttrPrice_Tier12Block9Price_c, gZclDataTypeUint32_c, gZclAttrFlagsInRAM_c | gZclAttrFlagsRdOnly_c, sizeof(uint32_t), (void*)MbrOfs(sePriceAttrRAM_t, BlockPriceInfoPrice[200]) }
#endif			
#if gASL_ZclPrice_BlockPriceInfoNumber_d >= 202
	,{gZclAttrPrice_Tier12Block10Price_c, gZclDataTypeUint32_c, gZclAttrFlagsInRAM_c | gZclAttrFlagsRdOnly_c, sizeof(uint32_t), (void*)MbrOfs(sePriceAttrRAM_t, BlockPriceInfoPrice[201]) }
#endif			
#if gASL_ZclPrice_BlockPriceInfoNumber_d >= 203
	,{gZclAttrPrice_Tier12Block11Price_c, gZclDataTypeUint32_c, gZclAttrFlagsInRAM_c | gZclAttrFlagsRdOnly_c, sizeof(uint32_t), (void*)MbrOfs(sePriceAttrRAM_t, BlockPriceInfoPrice[202]) }
#endif			
#if gASL_ZclPrice_BlockPriceInfoNumber_d >= 204
	,{gZclAttrPrice_Tier12Block12Price_c, gZclDataTypeUint32_c, gZclAttrFlagsInRAM_c | gZclAttrFlagsRdOnly_c, sizeof(uint32_t), (void*)MbrOfs(sePriceAttrRAM_t, BlockPriceInfoPrice[203]) }
#endif			
#if gASL_ZclPrice_BlockPriceInfoNumber_d >= 205
	,{gZclAttrPrice_Tier12Block13Price_c, gZclDataTypeUint32_c, gZclAttrFlagsInRAM_c | gZclAttrFlagsRdOnly_c, sizeof(uint32_t), (void*)MbrOfs(sePriceAttrRAM_t, BlockPriceInfoPrice[204]) }
#endif			
#if gASL_ZclPrice_BlockPriceInfoNumber_d >= 206
	,{gZclAttrPrice_Tier12Block14Price_c, gZclDataTypeUint32_c, gZclAttrFlagsInRAM_c | gZclAttrFlagsRdOnly_c, sizeof(uint32_t), (void*)MbrOfs(sePriceAttrRAM_t, BlockPriceInfoPrice[205]) }
#endif			
#if gASL_ZclPrice_BlockPriceInfoNumber_d >= 207
	,{gZclAttrPrice_Tier12Block15Price_c, gZclDataTypeUint32_c, gZclAttrFlagsInRAM_c | gZclAttrFlagsRdOnly_c, sizeof(uint32_t), (void*)MbrOfs(sePriceAttrRAM_t, BlockPriceInfoPrice[206]) }
#endif			
#if gASL_ZclPrice_BlockPriceInfoNumber_d >= 208
	,{gZclAttrPrice_Tier12Block16Price_c, gZclDataTypeUint32_c, gZclAttrFlagsInRAM_c | gZclAttrFlagsRdOnly_c, sizeof(uint32_t), (void*)MbrOfs(sePriceAttrRAM_t, BlockPriceInfoPrice[207]) }
#endif			
#if gASL_ZclPrice_BlockPriceInfoNumber_d >= 209
	,{gZclAttrPrice_Tier13Block1Price_c, gZclDataTypeUint32_c, gZclAttrFlagsInRAM_c | gZclAttrFlagsRdOnly_c, sizeof(uint32_t), (void*)MbrOfs(sePriceAttrRAM_t, BlockPriceInfoPrice[208]) }
#endif			
#if gASL_ZclPrice_BlockPriceInfoNumber_d >= 210
	,{gZclAttrPrice_Tier13Block2Price_c, gZclDataTypeUint32_c, gZclAttrFlagsInRAM_c | gZclAttrFlagsRdOnly_c, sizeof(uint32_t), (void*)MbrOfs(sePriceAttrRAM_t, BlockPriceInfoPrice[209]) }
#endif			
#if gASL_ZclPrice_BlockPriceInfoNumber_d >= 211
	,{gZclAttrPrice_Tier13Block3Price_c, gZclDataTypeUint32_c, gZclAttrFlagsInRAM_c | gZclAttrFlagsRdOnly_c, sizeof(uint32_t), (void*)MbrOfs(sePriceAttrRAM_t, BlockPriceInfoPrice[210]) }
#endif			
#if gASL_ZclPrice_BlockPriceInfoNumber_d >= 212
	,{gZclAttrPrice_Tier13Block4Price_c, gZclDataTypeUint32_c, gZclAttrFlagsInRAM_c | gZclAttrFlagsRdOnly_c, sizeof(uint32_t), (void*)MbrOfs(sePriceAttrRAM_t, BlockPriceInfoPrice[211]) }
#endif			
#if gASL_ZclPrice_BlockPriceInfoNumber_d >= 213
	,{gZclAttrPrice_Tier13Block5Price_c, gZclDataTypeUint32_c, gZclAttrFlagsInRAM_c | gZclAttrFlagsRdOnly_c, sizeof(uint32_t), (void*)MbrOfs(sePriceAttrRAM_t, BlockPriceInfoPrice[212]) }
#endif			
#if gASL_ZclPrice_BlockPriceInfoNumber_d >= 214
	,{gZclAttrPrice_Tier13Block6Price_c, gZclDataTypeUint32_c, gZclAttrFlagsInRAM_c | gZclAttrFlagsRdOnly_c, sizeof(uint32_t), (void*)MbrOfs(sePriceAttrRAM_t, BlockPriceInfoPrice[213]) }
#endif			
#if gASL_ZclPrice_BlockPriceInfoNumber_d >= 215
	,{gZclAttrPrice_Tier13Block7Price_c, gZclDataTypeUint32_c, gZclAttrFlagsInRAM_c | gZclAttrFlagsRdOnly_c, sizeof(uint32_t), (void*)MbrOfs(sePriceAttrRAM_t, BlockPriceInfoPrice[214]) }
#endif			
#if gASL_ZclPrice_BlockPriceInfoNumber_d >= 216
	,{gZclAttrPrice_Tier13Block8Price_c, gZclDataTypeUint32_c, gZclAttrFlagsInRAM_c | gZclAttrFlagsRdOnly_c, sizeof(uint32_t), (void*)MbrOfs(sePriceAttrRAM_t, BlockPriceInfoPrice[215]) }
#endif			
#if gASL_ZclPrice_BlockPriceInfoNumber_d >= 217
	,{gZclAttrPrice_Tier13Block9Price_c, gZclDataTypeUint32_c, gZclAttrFlagsInRAM_c | gZclAttrFlagsRdOnly_c, sizeof(uint32_t), (void*)MbrOfs(sePriceAttrRAM_t, BlockPriceInfoPrice[216]) }
#endif			
#if gASL_ZclPrice_BlockPriceInfoNumber_d >= 218
	,{gZclAttrPrice_Tier13Block10Price_c, gZclDataTypeUint32_c, gZclAttrFlagsInRAM_c | gZclAttrFlagsRdOnly_c, sizeof(uint32_t), (void*)MbrOfs(sePriceAttrRAM_t, BlockPriceInfoPrice[217]) }
#endif			
#if gASL_ZclPrice_BlockPriceInfoNumber_d >= 219
	,{gZclAttrPrice_Tier13Block11Price_c, gZclDataTypeUint32_c, gZclAttrFlagsInRAM_c | gZclAttrFlagsRdOnly_c, sizeof(uint32_t), (void*)MbrOfs(sePriceAttrRAM_t, BlockPriceInfoPrice[218]) }
#endif			
#if gASL_ZclPrice_BlockPriceInfoNumber_d >= 220
	,{gZclAttrPrice_Tier13Block12Price_c, gZclDataTypeUint32_c, gZclAttrFlagsInRAM_c | gZclAttrFlagsRdOnly_c, sizeof(uint32_t), (void*)MbrOfs(sePriceAttrRAM_t, BlockPriceInfoPrice[219]) }
#endif			
#if gASL_ZclPrice_BlockPriceInfoNumber_d >= 221
	,{gZclAttrPrice_Tier13Block13Price_c, gZclDataTypeUint32_c, gZclAttrFlagsInRAM_c | gZclAttrFlagsRdOnly_c, sizeof(uint32_t), (void*)MbrOfs(sePriceAttrRAM_t, BlockPriceInfoPrice[220]) }
#endif			
#if gASL_ZclPrice_BlockPriceInfoNumber_d >= 222
	,{gZclAttrPrice_Tier13Block14Price_c, gZclDataTypeUint32_c, gZclAttrFlagsInRAM_c | gZclAttrFlagsRdOnly_c, sizeof(uint32_t), (void*)MbrOfs(sePriceAttrRAM_t, BlockPriceInfoPrice[221]) }
#endif			
#if gASL_ZclPrice_BlockPriceInfoNumber_d >= 223
	,{gZclAttrPrice_Tier13Block15Price_c, gZclDataTypeUint32_c, gZclAttrFlagsInRAM_c | gZclAttrFlagsRdOnly_c, sizeof(uint32_t), (void*)MbrOfs(sePriceAttrRAM_t, BlockPriceInfoPrice[222]) }
#endif			
#if gASL_ZclPrice_BlockPriceInfoNumber_d >= 224
	,{gZclAttrPrice_Tier13Block16Price_c, gZclDataTypeUint32_c, gZclAttrFlagsInRAM_c | gZclAttrFlagsRdOnly_c, sizeof(uint32_t), (void*)MbrOfs(sePriceAttrRAM_t, BlockPriceInfoPrice[223]) }
#endif			
#if gASL_ZclPrice_BlockPriceInfoNumber_d >= 225
	,{gZclAttrPrice_Tier14Block1Price_c, gZclDataTypeUint32_c, gZclAttrFlagsInRAM_c | gZclAttrFlagsRdOnly_c, sizeof(uint32_t), (void*)MbrOfs(sePriceAttrRAM_t, BlockPriceInfoPrice[224]) }
#endif			
#if gASL_ZclPrice_BlockPriceInfoNumber_d >= 226
	,{gZclAttrPrice_Tier14Block2Price_c, gZclDataTypeUint32_c, gZclAttrFlagsInRAM_c | gZclAttrFlagsRdOnly_c, sizeof(uint32_t), (void*)MbrOfs(sePriceAttrRAM_t, BlockPriceInfoPrice[225]) }
#endif			
#if gASL_ZclPrice_BlockPriceInfoNumber_d >= 227
	,{gZclAttrPrice_Tier14Block3Price_c, gZclDataTypeUint32_c, gZclAttrFlagsInRAM_c | gZclAttrFlagsRdOnly_c, sizeof(uint32_t), (void*)MbrOfs(sePriceAttrRAM_t, BlockPriceInfoPrice[226]) }
#endif			
#if gASL_ZclPrice_BlockPriceInfoNumber_d >= 228
	,{gZclAttrPrice_Tier14Block4Price_c, gZclDataTypeUint32_c, gZclAttrFlagsInRAM_c | gZclAttrFlagsRdOnly_c, sizeof(uint32_t), (void*)MbrOfs(sePriceAttrRAM_t, BlockPriceInfoPrice[227]) }
#endif			
#if gASL_ZclPrice_BlockPriceInfoNumber_d >= 229
	,{gZclAttrPrice_Tier14Block5Price_c, gZclDataTypeUint32_c, gZclAttrFlagsInRAM_c | gZclAttrFlagsRdOnly_c, sizeof(uint32_t), (void*)MbrOfs(sePriceAttrRAM_t, BlockPriceInfoPrice[228]) }
#endif			
#if gASL_ZclPrice_BlockPriceInfoNumber_d >= 230
	,{gZclAttrPrice_Tier14Block6Price_c, gZclDataTypeUint32_c, gZclAttrFlagsInRAM_c | gZclAttrFlagsRdOnly_c, sizeof(uint32_t), (void*)MbrOfs(sePriceAttrRAM_t, BlockPriceInfoPrice[229]) }
#endif			
#if gASL_ZclPrice_BlockPriceInfoNumber_d >= 231
	,{gZclAttrPrice_Tier14Block7Price_c, gZclDataTypeUint32_c, gZclAttrFlagsInRAM_c | gZclAttrFlagsRdOnly_c, sizeof(uint32_t), (void*)MbrOfs(sePriceAttrRAM_t, BlockPriceInfoPrice[230]) }
#endif			
#if gASL_ZclPrice_BlockPriceInfoNumber_d >= 232
	,{gZclAttrPrice_Tier14Block8Price_c, gZclDataTypeUint32_c, gZclAttrFlagsInRAM_c | gZclAttrFlagsRdOnly_c, sizeof(uint32_t), (void*)MbrOfs(sePriceAttrRAM_t, BlockPriceInfoPrice[231]) }
#endif			
#if gASL_ZclPrice_BlockPriceInfoNumber_d >= 233
	,{gZclAttrPrice_Tier14Block9Price_c, gZclDataTypeUint32_c, gZclAttrFlagsInRAM_c | gZclAttrFlagsRdOnly_c, sizeof(uint32_t), (void*)MbrOfs(sePriceAttrRAM_t, BlockPriceInfoPrice[232]) }
#endif			
#if gASL_ZclPrice_BlockPriceInfoNumber_d >= 234
	,{gZclAttrPrice_Tier14Block10Price_c, gZclDataTypeUint32_c, gZclAttrFlagsInRAM_c | gZclAttrFlagsRdOnly_c, sizeof(uint32_t), (void*)MbrOfs(sePriceAttrRAM_t, BlockPriceInfoPrice[233]) }
#endif			
#if gASL_ZclPrice_BlockPriceInfoNumber_d >= 235
	,{gZclAttrPrice_Tier14Block11Price_c, gZclDataTypeUint32_c, gZclAttrFlagsInRAM_c | gZclAttrFlagsRdOnly_c, sizeof(uint32_t), (void*)MbrOfs(sePriceAttrRAM_t, BlockPriceInfoPrice[234]) }
#endif			
#if gASL_ZclPrice_BlockPriceInfoNumber_d >= 236
	,{gZclAttrPrice_Tier14Block12Price_c, gZclDataTypeUint32_c, gZclAttrFlagsInRAM_c | gZclAttrFlagsRdOnly_c, sizeof(uint32_t), (void*)MbrOfs(sePriceAttrRAM_t, BlockPriceInfoPrice[235]) }
#endif			
#if gASL_ZclPrice_BlockPriceInfoNumber_d >= 237
	,{gZclAttrPrice_Tier14Block13Price_c, gZclDataTypeUint32_c, gZclAttrFlagsInRAM_c | gZclAttrFlagsRdOnly_c, sizeof(uint32_t), (void*)MbrOfs(sePriceAttrRAM_t, BlockPriceInfoPrice[236]) }
#endif			
#if gASL_ZclPrice_BlockPriceInfoNumber_d >= 238
	,{gZclAttrPrice_Tier14Block14Price_c, gZclDataTypeUint32_c, gZclAttrFlagsInRAM_c | gZclAttrFlagsRdOnly_c, sizeof(uint32_t), (void*)MbrOfs(sePriceAttrRAM_t, BlockPriceInfoPrice[237]) }
#endif			
#if gASL_ZclPrice_BlockPriceInfoNumber_d >= 239
	,{gZclAttrPrice_Tier14Block15Price_c, gZclDataTypeUint32_c, gZclAttrFlagsInRAM_c | gZclAttrFlagsRdOnly_c, sizeof(uint32_t), (void*)MbrOfs(sePriceAttrRAM_t, BlockPriceInfoPrice[238]) }
#endif			
#if gASL_ZclPrice_BlockPriceInfoNumber_d >= 240
	,{gZclAttrPrice_Tier14Block16Price_c, gZclDataTypeUint32_c, gZclAttrFlagsInRAM_c | gZclAttrFlagsRdOnly_c, sizeof(uint32_t), (void*)MbrOfs(sePriceAttrRAM_t, BlockPriceInfoPrice[239]) }
#endif			
#if gASL_ZclPrice_BlockPriceInfoNumber_d >= 241
	,{gZclAttrPrice_Tier15Block1Price_c, gZclDataTypeUint32_c, gZclAttrFlagsInRAM_c | gZclAttrFlagsRdOnly_c, sizeof(uint32_t), (void*)MbrOfs(sePriceAttrRAM_t, BlockPriceInfoPrice[240]) }
#endif			
#if gASL_ZclPrice_BlockPriceInfoNumber_d >= 242
	,{gZclAttrPrice_Tier15Block2Price_c, gZclDataTypeUint32_c, gZclAttrFlagsInRAM_c | gZclAttrFlagsRdOnly_c, sizeof(uint32_t), (void*)MbrOfs(sePriceAttrRAM_t, BlockPriceInfoPrice[241]) }
#endif			
#if gASL_ZclPrice_BlockPriceInfoNumber_d >= 243
	,{gZclAttrPrice_Tier15Block3Price_c, gZclDataTypeUint32_c, gZclAttrFlagsInRAM_c | gZclAttrFlagsRdOnly_c, sizeof(uint32_t), (void*)MbrOfs(sePriceAttrRAM_t, BlockPriceInfoPrice[242]) }
#endif			
#if gASL_ZclPrice_BlockPriceInfoNumber_d >= 244
	,{gZclAttrPrice_Tier15Block4Price_c, gZclDataTypeUint32_c, gZclAttrFlagsInRAM_c | gZclAttrFlagsRdOnly_c, sizeof(uint32_t), (void*)MbrOfs(sePriceAttrRAM_t, BlockPriceInfoPrice[243]) }
#endif			
#if gASL_ZclPrice_BlockPriceInfoNumber_d >= 245
	,{gZclAttrPrice_Tier15Block5Price_c, gZclDataTypeUint32_c, gZclAttrFlagsInRAM_c | gZclAttrFlagsRdOnly_c, sizeof(uint32_t), (void*)MbrOfs(sePriceAttrRAM_t, BlockPriceInfoPrice[244]) }
#endif			
#if gASL_ZclPrice_BlockPriceInfoNumber_d >= 246
	,{gZclAttrPrice_Tier15Block6Price_c, gZclDataTypeUint32_c, gZclAttrFlagsInRAM_c | gZclAttrFlagsRdOnly_c, sizeof(uint32_t), (void*)MbrOfs(sePriceAttrRAM_t, BlockPriceInfoPrice[245]) }
#endif			
#if gASL_ZclPrice_BlockPriceInfoNumber_d >= 247
	,{gZclAttrPrice_Tier15Block7Price_c, gZclDataTypeUint32_c, gZclAttrFlagsInRAM_c | gZclAttrFlagsRdOnly_c, sizeof(uint32_t), (void*)MbrOfs(sePriceAttrRAM_t, BlockPriceInfoPrice[246]) }
#endif			
#if gASL_ZclPrice_BlockPriceInfoNumber_d >= 248
	,{gZclAttrPrice_Tier15Block8Price_c, gZclDataTypeUint32_c, gZclAttrFlagsInRAM_c | gZclAttrFlagsRdOnly_c, sizeof(uint32_t), (void*)MbrOfs(sePriceAttrRAM_t, BlockPriceInfoPrice[247]) }
#endif			
#if gASL_ZclPrice_BlockPriceInfoNumber_d >= 249
	,{gZclAttrPrice_Tier15Block9Price_c, gZclDataTypeUint32_c, gZclAttrFlagsInRAM_c | gZclAttrFlagsRdOnly_c, sizeof(uint32_t), (void*)MbrOfs(sePriceAttrRAM_t, BlockPriceInfoPrice[248]) }
#endif			
#if gASL_ZclPrice_BlockPriceInfoNumber_d >= 250
	,{gZclAttrPrice_Tier15Block10Price_c, gZclDataTypeUint32_c, gZclAttrFlagsInRAM_c | gZclAttrFlagsRdOnly_c, sizeof(uint32_t), (void*)MbrOfs(sePriceAttrRAM_t, BlockPriceInfoPrice[249]) }
#endif			
#if gASL_ZclPrice_BlockPriceInfoNumber_d >= 251
	,{gZclAttrPrice_Tier15Block11Price_c, gZclDataTypeUint32_c, gZclAttrFlagsInRAM_c | gZclAttrFlagsRdOnly_c, sizeof(uint32_t), (void*)MbrOfs(sePriceAttrRAM_t, BlockPriceInfoPrice[250]) }
#endif			
#if gASL_ZclPrice_BlockPriceInfoNumber_d >= 252
	,{gZclAttrPrice_Tier15Block12Price_c, gZclDataTypeUint32_c, gZclAttrFlagsInRAM_c | gZclAttrFlagsRdOnly_c, sizeof(uint32_t), (void*)MbrOfs(sePriceAttrRAM_t, BlockPriceInfoPrice[251]) }
#endif			
#if gASL_ZclPrice_BlockPriceInfoNumber_d >= 253
	,{gZclAttrPrice_Tier15Block13Price_c, gZclDataTypeUint32_c, gZclAttrFlagsInRAM_c | gZclAttrFlagsRdOnly_c, sizeof(uint32_t), (void*)MbrOfs(sePriceAttrRAM_t, BlockPriceInfoPrice[252]) }
#endif			
#if gASL_ZclPrice_BlockPriceInfoNumber_d >= 254
	,{gZclAttrPrice_Tier15Block14Price_c, gZclDataTypeUint32_c, gZclAttrFlagsInRAM_c | gZclAttrFlagsRdOnly_c, sizeof(uint32_t), (void*)MbrOfs(sePriceAttrRAM_t, BlockPriceInfoPrice[253]) }
#endif			
#if gASL_ZclPrice_BlockPriceInfoNumber_d >= 255
	,{gZclAttrPrice_Tier15Block15Price_c, gZclDataTypeUint32_c, gZclAttrFlagsInRAM_c | gZclAttrFlagsRdOnly_c, sizeof(uint32_t), (void*)MbrOfs(sePriceAttrRAM_t, BlockPriceInfoPrice[254]) }
#endif			
#if gASL_ZclPrice_BlockPriceInfoNumber_d >= 256
	,{gZclAttrPrice_Tier15Block16Price_c, gZclDataTypeUint32_c, gZclAttrFlagsInRAM_c | gZclAttrFlagsRdOnly_c, sizeof(uint32_t), (void*)MbrOfs(sePriceAttrRAM_t, BlockPriceInfoPrice[255]) }
#endif
};

const zclAttrDefList_t gZclPriceServerAttrDefList = {
  NumberOfElements(gaZclPriceServerAttrDef),
  gaZclPriceServerAttrDef
};
#endif

/************** SE TUNNELING CLUSTER  ******************************************/
#if gZclClusterOptionals_d && gASL_ZclSETunneling_d
const zclAttrDef_t gaZclSETunnelingServerAttrDef[] = {
  { gZclAttrSETunnelingCloseTunnelTimeout_c, gZclDataTypeUint16_c, gZclAttrFlagsInRAM_c | gZclAttrFlagsRdOnly_c, sizeof(uint16_t), (void *)MbrOfs(seTunnelingAttrRAM_t, closeTunnelTimeout)}
};

const zclAttrDefList_t gZclSETunnelingServerAttrDefList = {
  NumberOfElements(gaZclSETunnelingServerAttrDef),
  gaZclSETunnelingServerAttrDef
};
#endif

/************** PREPAYMENT CLUSTER  ******************************************/
#if gZclClusterOptionals_d && gASL_ZclPrepayment_d
const zclAttrDef_t gaZclPrepaymentServerAttrDef[] = {
  { gZclAttrPrepaymentPaymentCtrl_c, gZclDataTypeBitmap8_c, gZclAttrFlagsInRAM_c | gZclAttrFlagsRdOnly_c, sizeof(uint8_t), (void *)MbrOfs(sePrepaymentAttrRAM_t, paymentCtrl)}
};

const zclAttrDefList_t gZclPrepaymentServerAttrDefList = {
  NumberOfElements(gaZclPrepaymentServerAttrDef),
  gaZclPrepaymentServerAttrDef
};
#endif
/************** DEMAND RESPONSE LOAD CONTROL CLUSTER  ******************************************/
/*Note there is no Server attributes ???*/

const zclAttrDef_t gaZclDRLCClientServerAttrDef[] = {
/* Server attributes */
  
  /* Client attributes */
  { gZclAttrUtilityGroup_c,   gZclDataTypeUint8_c, gZclAttrFlagsInRAM_c, sizeof(uint8_t), (void *)MbrOfs(ZCLDRLCClientServerAttrsRAM_t, UtilityGroup) },
  { gZclAttrStartRandomizeMin_c,     gZclDataTypeUint8_c, gZclAttrFlagsInRAM_c, sizeof(uint8_t), (void *)MbrOfs(ZCLDRLCClientServerAttrsRAM_t, StartRandomizeMin)  },
  { gZclAttrStopRandomizeMin_c, gZclDataTypeUint8_c, gZclAttrFlagsInRAM_c, sizeof(uint8_t), (void *)MbrOfs(ZCLDRLCClientServerAttrsRAM_t, StopRandomizeMin) },
  { gZclAttrDevCls_c, gZclDataTypeUint16_c, gZclAttrFlagsInRAM_c | gZclAttrFlagsRdOnly_c, sizeof(uint16_t), (void *)MbrOfs(ZCLDRLCClientServerAttrsRAM_t, DevCls) }

};

const zclAttrDefList_t gZclDRLCClientServerClusterAttrDefList = {
  NumberOfElements(gaZclDRLCClientServerAttrDef),
  gaZclDRLCClientServerAttrDef
};


#if gInterPanCommunicationEnabled_c
/*  Handle the InterPan messages  */
pfnInterPanIndication_t pfnInterPanServerInd = NULL;
pfnInterPanIndication_t pfnInterPanClientInd = NULL;
#endif

/* Timers ID for DmndRspLdCtrl  */
tmrTimerID_t gLdLdCtrlTimerID, jitterTimerID;
/*Events Table Information*/
zclLdCtrl_EventsTableEntry_t gaEventsTable[gNumOfEventsTableEntry_c];
zclLdCtrl_EventsTableEntry_t gaServerEventsTable[gNumOfServerEventsTableEntry_c];
uint32_t mGetLdCtlEvtStartTime;
uint8_t mGetNumOfLdCtlEvts;
uint8_t mIndexLdCtl;

/* TimerId for Price Client */
tmrTimerID_t gPriceClientTimerID;
publishPriceEntry_t gaClientPriceTable[gNumofClientPrices_c];
publishPriceEntry_t gaServerPriceTable[gNumofServerPrices_c];
index_t mRegIndex , mInterPanIndex , mUpdatePriceIndex;
addrInfoType_t mAddrInfo; 
uint32_t mGetPriceStartTime;
uint8_t mGetNumOfPriceEvts;
bool_t mIsInterPanFlag;
uint8_t mIndex;

storedInterPanAddrInfo_t gaInterPanAddrTable[gNumOfInterPanAddr_c];
storedPriceAddrInfo_t gaPriceAddrTable[gNumOfPriceAddr_c];

/********************************************************************************/

/******************************************************************************
*******************************************************************************
* Private memory declarations
*******************************************************************************
******************************************************************************/

/*Used to keep the new message(event)*/
static zclCmdDmndRspLdCtrl_LdCtrlEvtReq_t mNewLDCtrlEvt;
static afAddrInfo_t mNewAddrInfo;
static uint8_t mNewEntryIndex = 0xff, mReportStatusEntryIndex, mGetRandomFlag = TRUE;
static uint8_t mAcceptVoluntaryLdCrtlEvt = TRUE; /* Default Voluntary Load Control events are accepted */

/******************************************************************************
*******************************************************************************
* Public functions
*******************************************************************************
******************************************************************************/

/*****************************************************************************/

/******************************************************************************/

/*WARNING -remove this function*/
void SE_Init(void)
{
  
  
  /* ZCL Time Cluster starts at 01 Jan 2000 00:00:00 GMT, hence -946702800 offset to unix Epox 01 Jan 1970 00:00:00 GMT */
#define mTimeClusterTimeEpoxOffset_c 946702800
  
  /* 28 August 2008 12:00:00 GMT */
#define mDefaultValueOfTimeClusterTime_c 1219924800 - mTimeClusterTimeEpoxOffset_c
  
  /* Time Zone GMT+1 - Aarhus, Denmark */
#define mDefaultValueOfTimeClusterTimeZone_c 3600
  /* Start daylight savings 2008 for Aarhus, Denmark 30 March 2008 */
#define mDefaultValueOfTimeClusterDstStart_c 1206849600 - mTimeClusterTimeEpoxOffset_c
  /* End daylight savings 2008 for Aarhus, Denmark 26 October 2008 */
#define mDefaultValueOfTimeClusterDstEnd_c 1224993600 - mTimeClusterTimeEpoxOffset_c
  /* Daylight Saving + 1h */
#define mDefaultValueOfClusterDstShift_c 3600
  /* Valid Until Time - mDefaultValueOfTimeClusterTime_c + aprox 5 years */
#define mDefaultValueOfTimeClusterValidUntilTime_c mDefaultValueOfTimeClusterTime_c + 31536000
  
  {
    ZCLTimeConf_t defaultTimeConf;
    defaultTimeConf.Time = mDefaultValueOfTimeClusterTime_c;
    defaultTimeConf.TimeZone = mDefaultValueOfTimeClusterTimeZone_c;
    defaultTimeConf.DstStart = mDefaultValueOfTimeClusterDstStart_c;
    defaultTimeConf.DstEnd = mDefaultValueOfTimeClusterDstEnd_c;
    defaultTimeConf.DstShift = mDefaultValueOfClusterDstShift_c;
    defaultTimeConf.ValidUntilTime = mDefaultValueOfTimeClusterValidUntilTime_c;
    
    ZCL_TimeInit(&defaultTimeConf);
  }
  
#if gFragmentationCapability_d 
  {
    uint8_t aIncomingTransferSize[2] ={ gSEMaxIncomingTransferSize };
    FLib_MemCpy(&gBeeStackConfig.maxTransferSize[0], &aIncomingTransferSize[0], 2);
    ApsmeSetRequest(gApsMaxWindowSize_c, gSEMaxWindowSize_c);        // window can be 1 - 8
    ApsmeSetRequest(gApsInterframeDelay_c, gSEInterframeDelay_c);    // interframe delay can be 1 - 255ms
    ApsmeSetRequest(gApsMaxFragmentLength_c, gSEMaxFragmentLength_c);  // max len can be 1 - ApsmeGetMaxAsduLength()
  }
#endif /* #if gFragmentationCapability_d  */  
  
}

#if gInterPanCommunicationEnabled_c
/******************************************************************************/
void ZCL_RegisterInterPanClient(pfnInterPanIndication_t pFunc)
{
  pfnInterPanClientInd = pFunc;
}

/******************************************************************************/

void ZCL_RegisterInterPanServer(pfnInterPanIndication_t pFunc)
{
  pfnInterPanServerInd = pFunc;
}
#endif /* #if gInterPanCommunicationEnabled_c */

/******************************************************************************/
void ZCL_MeterInit(void)
{
  /*A timer is used for demo purpose to generate consumption data.*/
  gGetProfileTestTimerID = TMR_AllocateTimer();
  gUpdatePowerConsumptionTimerID = TMR_AllocateTimer();
  TMR_StartSecondTimer(gUpdatePowerConsumptionTimerID, gUpdateConsumption_c, UpdatePowerConsumptionTimerCallBack);
#if(gProfIntrvPeriod_c < 5)
  TMR_StartMinuteTimer(gGetProfileTestTimerID, gTimerValue_c, GetProfileTestTimerCallBack);
#else
  TMR_StartSecondTimer(gGetProfileTestTimerID, gTimerValue_c, GetProfileTestTimerCallBack);
#endif
}

/******************************************************************************/
void ZCL_MsgInit(void)
{

  zbNwkAddr_t ESPAddr = {0x00,0x00};
  /*Set default msg cluster response addr.*/
  MsgResponseSourceAddr.msgAddrInfo.addrInfo.dstAddrMode = 0x02;
  Copy2Bytes(MsgResponseSourceAddr.msgAddrInfo.addrInfo.dstAddr.aNwkAddr, ESPAddr);
//  MsgResponseSourceAddr.dstEndPoint=gSendingNwkData.endPoint;
  Set2Bytes(MsgResponseSourceAddr.msgAddrInfo.addrInfo.aClusterId, gZclClusterMsg_c);
  MsgResponseSourceAddr.msgAddrInfo.addrInfo.srcEndPoint=appEndPoint;
  MsgResponseSourceAddr.msgAddrInfo.addrInfo.txOptions=afTxOptionsDefault_c;
  MsgResponseSourceAddr.msgAddrInfo.addrInfo.radiusCounter=afDefaultRadius_c;
  MsgResponseSourceAddr.msgControl = 0x00;
  gMsgDisplayTimerID = TMR_AllocateTimer();
  TMR_StartSecondTimer(gMsgDisplayTimerID, 1, MsgDisplayTimerCallBack);
  
}

/******************************************************************************/

void ZCL_LdCtrlClientInit(void)
{
  
  gLdLdCtrlTimerID = TMR_AllocateTimer();
  jitterTimerID = TMR_AllocateTimer();
} 


/******************************************************************************/
void ZCL_PriceClientInit(void)
{
  gPriceClientTimerID =  TMR_AllocateTimer();
} 	



#if gInterPanCommunicationEnabled_c
/******************************************************************************/
/* Handle ALL Inter Pan Server\Client Indications (filter after cluster ID)   */
/******************************************************************************/
zbStatus_t ZCL_InterPanClusterServer
(
	zbInterPanDataIndication_t *pIndication, /* IN: */
	afDeviceDef_t *pDev                /* IN: */
) 
{
(void) pIndication;
(void) pDev;
  
#if(gASL_ZclPrice_InterPanPublishPriceRsp_d )    
if(IsEqual2BytesInt(pIndication->aClusterId, gZclClusterPrice_c))
 return ZCL_InterPanPriceClusterServer(pIndication, pDev); 
#endif

if(IsEqual2BytesInt(pIndication->aClusterId, gZclClusterMsg_c))
 return ZCL_InterPanMsgClusterServer(pIndication, pDev); 
   
/* if user uses Inter Pan with other cluster, add the Cluster ID filter here*/
   
 return gZclUnsupportedClusterCommand_c;
}

/******************************************************************************/
zbStatus_t ZCL_InterPanClusterClient
(
	zbInterPanDataIndication_t *pIndication, /* IN: */
	afDeviceDef_t *pDev                /* IN: */
) 
{
(void) pIndication;
(void) pDev;

#if(gASL_ZclPrice_InterPanGetCurrPriceReq_d )  
if(IsEqual2BytesInt(pIndication->aClusterId, gZclClusterPrice_c))
 return ZCL_InterPanPriceClusterClient(pIndication, pDev); 
#endif

 
if(IsEqual2BytesInt(pIndication->aClusterId, gZclClusterMsg_c))
 return ZCL_InterPanMsgClusterClient(pIndication, pDev); 

   
/* if user uses Inter Pan with other cluster, add the Cluster ID filter here*/
   
 return gZclUnsupportedClusterCommand_c;
}
#endif /* #if gInterPanCommunicationEnabled_c */


/******************************************************************************/
/******************************************************************************/
/* The Publish Price command is generated in response to receiving a Get Current
Price command , a Get Scheduled Prices command , or when an update to the pricing information is available
from the commodity provider. */
static zbStatus_t ZCL_SendPublishPrice(addrInfoType_t *pAddrInfo, publishPriceEntry_t * pMsg, bool_t IsInterPanFlag) 
{
  zclPrice_PublishPriceRsp_t req;
  #if gInterPanCommunicationEnabled_c
  zclPrice_InterPanPublishPriceRsp_t interPanReq;
  #endif   
  uint32_t currentTime;
  /* Get the current time */
  currentTime = ZCL_GetUTCTime();
  currentTime = Native2OTA32(currentTime);
  
  if(!IsInterPanFlag)
  {
    FLib_MemCpy(&req.addrInfo, pAddrInfo, sizeof(afAddrInfo_t));	
    req.zclTransactionId =  gZclTransactionId++;
    FLib_MemCpy(&req.cmdFrame, pMsg, sizeof(zclCmdPrice_PublishPriceRsp_t));
    req.cmdFrame.CurrTime = currentTime;
    return zclPrice_PublishPriceRsp(&req);
  }  
#if gInterPanCommunicationEnabled_c
  else	
  {
    FLib_MemCpy(&interPanReq.addrInfo, pAddrInfo, sizeof(InterPanAddrInfo_t));	
    interPanReq.zclTransactionId =  gZclTransactionId++;
    FLib_MemCpy(&interPanReq.cmdFrame, pMsg, sizeof(zclCmdPrice_PublishPriceRsp_t));
    interPanReq.cmdFrame.CurrTime = currentTime;
    return zclPrice_InterPanPublishPriceRsp(&interPanReq);		
  } 
#else
return FALSE;
#endif     
}

/******************************************************************************/
static zbStatus_t ZCL_PriceAck(zclPrice_PriceAck_t *pReq)
{
  return ZCL_SendClientRspSeqPassed(gZclCmdPrice_PriceAck_c, sizeof(zclCmdPrice_PriceAck_t),(zclGenericReq_t *)pReq);
}

static zbStatus_t ZCL_InterPriceAck(zclPrice_InterPriceAck_t *pReq)
{
  return ZCL_SendInterPanClientRspSeqPassed(gZclCmdPrice_PriceAck_c, sizeof(zclCmdPrice_PriceAck_t),(zclGenericReq_t *)pReq);
}

/******************************************************************************/

static zbStatus_t ZCL_SendPriceAck(zbApsdeDataIndication_t *pIndication)
{
  uint32_t currentTime;
  zclPrice_PriceAck_t priceAck;
  zclCmdPrice_PublishPriceRsp_t * pMsg;
  
  pMsg = (zclCmdPrice_PublishPriceRsp_t*)((uint8_t*)pIndication->pAsdu + sizeof(zclFrame_t));
  AF_PrepareForReply(&priceAck.addrInfo, pIndication);
  priceAck.zclTransactionId = ((zclFrame_t *)pIndication->pAsdu)->transactionId;
  FLib_MemCpy(&priceAck.cmdFrame.ProviderID, pMsg->ProviderID, sizeof(ProviderID_t));
  FLib_MemCpy(&priceAck.cmdFrame.IssuerEvt, pMsg->IssuerEvt, sizeof(SEEvtId_t));
  currentTime = ZCL_GetUTCTime();
  currentTime = Native2OTA32(currentTime);
  priceAck.cmdFrame.PriceAckTime = currentTime;
  priceAck.cmdFrame.PriceControl = pMsg->PriceControl;
  
  return ZCL_PriceAck(&priceAck);
}

/******************************************************************************/

static zbStatus_t ZCL_SendInterPriceAck(zbInterPanDataIndication_t *pIndication)
{
  uint32_t currentTime;
  zclPrice_InterPriceAck_t priceAck;
  zclCmdPrice_PublishPriceRsp_t * pMsg;
  
  pMsg = (zclCmdPrice_PublishPriceRsp_t*)((uint8_t*)pIndication->pAsdu + sizeof(zclFrame_t));
  PrepareInterPanForReply((InterPanAddrInfo_t *)&priceAck.addrInfo, pIndication);
  priceAck.zclTransactionId = ((zclFrame_t *)pIndication->pAsdu)->transactionId;
  FLib_MemCpy(&priceAck.cmdFrame.ProviderID, pMsg->ProviderID, sizeof(ProviderID_t));
  FLib_MemCpy(&priceAck.cmdFrame.IssuerEvt, pMsg->IssuerEvt, sizeof(SEEvtId_t));
  currentTime = ZCL_GetUTCTime();
  currentTime = Native2OTA32(currentTime);
  priceAck.cmdFrame.PriceAckTime = currentTime;
  priceAck.cmdFrame.PriceControl = pMsg->PriceControl;
  
  return ZCL_InterPriceAck(&priceAck);
}

/******************************************************************************/
/* Add Price in Table... so that to have the scheduled price in asccendent order */
static uint8_t AddPriceInTable(publishPriceEntry_t *pTable, uint8_t len, zclCmdPrice_PublishPriceRsp_t *pMsg)
{
  uint32_t startTime, msgStartTime, currentTime;
  uint8_t i, poz = 0xff;
  /* get message time */
  msgStartTime = OTA2Native32(pMsg->StartTime);
  
  /* keep the price in asccendent order; find the index in table where the message will be added */
  for(i = 0; i < len; i++)
  {
    if((pTable+i)->EntryStatus != 0x00)
    {
      startTime = OTA2Native32((pTable+i)->Price.StartTime);
      if(((startTime > msgStartTime) && (startTime != 0xffffffff)) ||
         (msgStartTime == 0xffffffff)) 
      {
        poz = i;
        break;
      }
    }
    else
    {
      poz = i;
      break;
    }
    
  }
  /* check if the table is full; return oxff */
  if(poz == 0xff)
    return poz;
  /*move the content to the left*/
  if((pTable+poz)->EntryStatus !=0x00)
    FLib_MemInPlaceCpy((pTable+poz+1),(pTable+poz),(len-poz-1)* sizeof(publishPriceEntry_t));
  
  FLib_MemCpy((pTable+poz), pMsg, sizeof(zclCmdPrice_PublishPriceRsp_t));
  
  if((pMsg->StartTime == 0x00000000)||
     (pMsg->StartTime == 0xffffffff))
  
  {
    /*  here get the currentTime  */
    currentTime = ZCL_GetUTCTime();
    currentTime = Native2OTA32(currentTime);
    (pTable+poz)->EffectiveStartTime = currentTime;
  }
  else
    (pTable+poz)->EffectiveStartTime = pMsg->StartTime ;
  /* new price was received */	
  (pTable+poz)->EntryStatus = gPriceReceivedStatus_c;
  /* Call the App to signal that a Price was received; User should check EntryStatus */
  BeeAppUpdateDevice (0, gZclUI_PriceEvt_c, 0, 0, (pTable+poz));
  return poz;
}

/******************************************************************************/
/*Check and Updated a Price.
When new pricing information is provided that replaces older pricing
information for the same time period, IssuerEvt field allows devices to determine which
information is newer. It is expected that the value contained in this field is a
unique number managed by upstream servers.
Thus, newer pricing information will have a value in the Issuer Event ID field that
is larger than older pricing information.
*/
static uint8_t CheckForPriceUpdate(zclCmdPrice_PublishPriceRsp_t *pMsg, publishPriceEntry_t *pTable, uint8_t len)
{
  uint8_t i;
  uint32_t msgIssuerEvt, entryIssuerEvt;
  
  msgIssuerEvt = FourBytesToUint32(pMsg->IssuerEvt);
  msgIssuerEvt = OTA2Native32(msgIssuerEvt);
  
  for(i = 0; i < len; i++)
  {
    if((pTable+i)->EntryStatus != 0x00)
    {
      entryIssuerEvt = FourBytesToUint32(&(pTable+i)->Price.IssuerEvt);
      entryIssuerEvt = OTA2Native32(entryIssuerEvt);
      if ((pTable+i)->Price.StartTime == pMsg->StartTime &&
          FLib_MemCmp(&(pTable+i)->Price.DurationInMinutes, &pMsg->DurationInMinutes, sizeof(Duration_t)))
        if(entryIssuerEvt < msgIssuerEvt)
        {
          FLib_MemCpy((pTable+i), pMsg, sizeof(zclCmdPrice_PublishPriceRsp_t));
          return i;
        }
        else 
          return 0xFE; /* reject it */
    }
  }
  return 0xff;
  
}

/******************************************************************************/
/* The Price timer callback that keep track of current active price */
static void TimerClientPriceCallBack(tmrTimerID_t timerID)
{
  (void) timerID;
  TS_SendEvent(gZclTaskId, gzclEvtHandleClientPrices_c);
}

/******************************************************************************/
/* Handle the Client Prices signalling when the current price starts, was updated or is completed */
void ZCL_HandleClientPrices(void)
{
  uint32_t currentTime, startTime, nextTime=0x00000000, stopTime;
  uint16_t duration;
  publishPriceEntry_t *pEntry = &gaClientPriceTable[0];
  
  
  /* the Price table is kept in ascendent order; check if any price is scheduled*/
  if(pEntry->EntryStatus == 0x00)
  {
    TMR_StopSecondTimer(gPriceClientTimerID);
    return;
  }
  
  /* Get the timing */
  currentTime = ZCL_GetUTCTime();
  startTime = OTA2Native32(pEntry->EffectiveStartTime);
  duration = OTA2Native16(pEntry->Price.DurationInMinutes);
  stopTime = startTime + (60*(uint32_t)duration);
  /* Check if the Price Event is completed */
  if(stopTime <= currentTime)
  {
    pEntry->EntryStatus = gPriceCompletedStatus_c; /* entry is not used anymore */
    /* Call the App to signal that a Price was completed; User should check EntryStatus */
    BeeAppUpdateDevice (0, gZclUI_PriceEvt_c, 0, 0, pEntry);
    pEntry->EntryStatus = 0x00;
    FLib_MemInPlaceCpy(pEntry, (pEntry+1), (gNumofClientPrices_c-1) * sizeof(publishPriceEntry_t));
    gaClientPriceTable[gNumofClientPrices_c-1].EntryStatus = 0x00;

  }
  else	
    if(startTime <= currentTime) /* check if the Price event have to be started or updated */
    {
      if((pEntry->EntryStatus == gPriceReceivedStatus_c)||
         (pEntry->EntryStatus == gPriceUpdateStatus_c))
      {
        pEntry->EntryStatus = gPriceStartedStatus_c;
        /* Call the App to signal that a Price was started; User should check EntryStatus */
        BeeAppUpdateDevice (0, gZclUI_PriceEvt_c, 0, 0, pEntry);
      }
      
    }
  
  if(currentTime < startTime)
    nextTime = startTime - currentTime;
  else
    if(currentTime < stopTime)
      nextTime = stopTime - currentTime;
  if (nextTime)
    TMR_StartSecondTimer(gPriceClientTimerID,(uint16_t)nextTime, TimerClientPriceCallBack);
  
}

/******************************************************************************/
/* Process the received Publish Price */
static zbStatus_t ZCL_ProcessClientPublishPrice(zclCmdPrice_PublishPriceRsp_t *pMsg)
{
  uint8_t updateIndex;
  zbStatus_t status = gZclFailure_c; 
  uint8_t newEntry;

  FLib_MemInPlaceCpy(&pMsg->IssuerEvt, &(pMsg->RateLabel.aStr[pMsg->RateLabel.length]), 
                     (sizeof(zclCmdPrice_PublishPriceRsp_t) - sizeof(ProviderID_t)- sizeof(zclStr12_t)));

  /* Check if it is a PriceUpdate */
  updateIndex = CheckForPriceUpdate(pMsg, (publishPriceEntry_t *)&gaClientPriceTable[0], gNumofClientPrices_c);
  /*if the Publish Price is not an update and is not rejected, add it in the table */
  if(updateIndex == 0xff)
  {
    /* Add the new Price information in the table */
    newEntry = AddPriceInTable((publishPriceEntry_t *)&gaClientPriceTable[0], gNumofClientPrices_c, pMsg);
    if (newEntry != 0xff)
      status = gZclSuccess_c; 
  }
  else
  {
    if(updateIndex != 0xfe)
    {
      /* the price was updated */
      gaClientPriceTable[updateIndex].EntryStatus = gPriceUpdateStatus_c;
      /* Call the App to signal that a Price was updated; User should check EntryStatus */
      BeeAppUpdateDevice (0, gZclUI_PriceEvt_c, 0, 0, &gaClientPriceTable[updateIndex]);
      status = gZclSuccess_c;
    }
  }
  
  if(status == gZclSuccess_c)
    TS_SendEvent(gZclTaskId, gzclEvtHandleClientPrices_c);
  
  return status;
}

#if gInterPanCommunicationEnabled_c
/******************************************************************************/
static void StoreInterPanAddr(InterPanAddrInfo_t *pAddrInfo)
{
  uint8_t i, newEntry = 0xff;
  
  for(i = 0; i < gNumOfInterPanAddr_c; i++)
  {
    /* If PanID already exist in the table, overwrite the existing entry*/
    if(gaInterPanAddrTable[i].entryStatus != 0x00 &&
       FLib_MemCmp(&(pAddrInfo->dstPanId), &(gaInterPanAddrTable[i].addrInfo.dstPanId), sizeof(zbPanId_t)) )
    {
      FLib_MemCpy(&gaInterPanAddrTable[i], pAddrInfo, sizeof(InterPanAddrInfo_t));
      return;
    }
    if(newEntry == 0xff && 
       gaInterPanAddrTable[i].entryStatus ==0x00)
      newEntry = i;
  }
  
  if(newEntry != 0xff)
  {
    FLib_MemCpy(&gaInterPanAddrTable[newEntry], pAddrInfo, sizeof(InterPanAddrInfo_t));
    gaInterPanAddrTable[newEntry].entryStatus = 0xff;
  } 
}
#endif
/******************************************************************************/
static void RegisterDevForPrices(afAddrInfo_t *pAddrInfo)
{
  uint8_t *pIEEE, aExtAddr[8];
  uint8_t i, newEntry = 0xff;
  
  pIEEE = APS_GetIeeeAddress((uint8_t*)&pAddrInfo->dstAddr.aNwkAddr, aExtAddr);
  if(!pIEEE)
    return;
  
  for(i = 0; i < gNumOfPriceAddr_c; i++)
  {
    if(gaPriceAddrTable[i].entryStatus != 0x00 &&
       Cmp8Bytes(pIEEE, (uint8_t*)&gaPriceAddrTable[i].iEEEaddr))
    {
      FLib_MemCpy(&gaPriceAddrTable[i].addrInfo, pAddrInfo,sizeof(afAddrInfo_t));
      return;
    }
    if(newEntry == 0xff && 
       gaPriceAddrTable[i].entryStatus == 0x00)
      newEntry = i;
  }
  
  if(newEntry != 0xff)
  {
    FLib_MemCpy(&gaPriceAddrTable[newEntry].iEEEaddr, pIEEE, 8);
    FLib_MemCpy(&gaPriceAddrTable[newEntry].addrInfo, pAddrInfo, sizeof(afAddrInfo_t));
    gaPriceAddrTable[newEntry].entryStatus = 0xff;
  } 
  
}

#if gInterPanCommunicationEnabled_c
/******************************************************************************/
static void InterPanJitterTimerCallBack(tmrTimerID_t timerID)
{
  uint8_t status;

  (void)timerID;
  gaInterPanAddrTable[mInterPanIndex].addrInfo.dstAddrMode = gAddrModeShort_c; 
  BeeUtilSetToF(&gaInterPanAddrTable[mInterPanIndex].addrInfo.dstAddr, sizeof(zbNwkAddr_t)); ///broadcast address 
  status = ZCL_SendPublishPrice((addrInfoType_t *)&gaInterPanAddrTable[mInterPanIndex],
                                &gaServerPriceTable[mUpdatePriceIndex], TRUE);
  if(status == gZclSuccess_c)
    ++mInterPanIndex;
  TS_SendEvent(gZclTaskId, gzclEvtHandlePublishPriceUpdate_c);
}
#endif /* #if gInterPanCommunicationEnabled_c */
/******************************************************************************/
/* Handle the Price Update */
void ZCL_HandlePublishPriceUpdate(void)
{
  uint8_t status;
  
  /* Check the Price addr table for SE device and send the publish price */
  if(mRegIndex < gNumOfPriceAddr_c)
  {
    if(gaPriceAddrTable[mRegIndex].entryStatus)
    {
      status = ZCL_SendPublishPrice((addrInfoType_t *)&gaPriceAddrTable[mRegIndex].addrInfo,
                                    &gaServerPriceTable[mUpdatePriceIndex], FALSE);
      if(status == gZclSuccess_c)
	       ++mRegIndex;
      TS_SendEvent(gZclTaskId, gzclEvtHandlePublishPriceUpdate_c);
      return;
      
    }
    
  }
  
#if gInterPanCommunicationEnabled_c  
{ 
  static tmrTimerID_t interPanTimerID = 0x00;
  uint16_t randomTime;
    
  /* Check the InterPan Price addr table for non-Se device and send the publish price */  
  if(mInterPanIndex < gNumOfInterPanAddr_c)
  {
    if(gaInterPanAddrTable[mInterPanIndex].entryStatus != 0x00)
    { 
      if(!interPanTimerID)
        interPanTimerID = TMR_AllocateTimer();
      if(interPanTimerID)
      {  
        randomTime = (uint16_t)gInterPanMaxJitterTime_c/10;
        randomTime = (uint16_t)GetRandomRange(0, (uint8_t)randomTime) * 10;
        TMR_StartSingleShotTimer (interPanTimerID, randomTime, InterPanJitterTimerCallBack);
      } 
    }
    
  }
}  
#endif /* #if gInterPanCommunicationEnabled_c */
}


/******************************************************************************/
/* Process the Get Current Price Req; reply with the active scheduled price */
static zbStatus_t ZCL_ProcessGetCurrPriceReq
(
addrInfoType_t *pAddrInfo,
zclCmdPrice_GetCurrPriceReq_t * pGetCurrPrice,
bool_t IsInterPanFlag)
{
  uint8_t i;
  uint32_t currentTime, startTime, stopTime;
  uint16_t duration;
  
  pGetCurrPrice->CmdOptions = pGetCurrPrice->CmdOptions & gGetCurrPrice_RequestorRxOnWhenIdle_c;
  
  if(pGetCurrPrice->CmdOptions)
  {
    if(IsInterPanFlag)
    {
#if gInterPanCommunicationEnabled_c       
      //Store InterPanAddr 
      StoreInterPanAddr((InterPanAddrInfo_t *)pAddrInfo);
#else
      return gZclFailure_c;
#endif /* #if gInterPanCommunicationEnabled_c  */
      
    }
    else
    {
      //set the device as being registered having the Rx on Idle(to send Price update) 
      RegisterDevForPrices((afAddrInfo_t *)pAddrInfo);
    }
  }
   /* here get the currentTime */
  currentTime = ZCL_GetUTCTime();
  /* reply with the active scheduled price */
  for(i = 0; i < gNumofServerPrices_c; i++)
  {
    if(gaServerPriceTable[i].EntryStatus != 0x00)
    {
      startTime = OTA2Native32(gaServerPriceTable[i].Price.StartTime);
      duration = OTA2Native16(gaServerPriceTable[i].Price.DurationInMinutes);
      stopTime = startTime + ((uint32_t)duration*60);
      if((startTime == 0x00000000) || (startTime == 0xFFFFFFFF) ||
         ((startTime <= currentTime) && (currentTime <= stopTime)))
	        return ZCL_SendPublishPrice(pAddrInfo, &gaServerPriceTable[i], IsInterPanFlag);	
    }
  }
  
  return gZclNotFound_c;  
}


/******************************************************************************/
void ZCL_HandleGetScheduledPrices(void)
{
  uint32_t  startTime;
  uint8_t status;
  
  if(mIndex < gNumofServerPrices_c)
  {
    /* Check if the entry is used and if there are more scheduled price to be send*/
    if((gaServerPriceTable[mIndex].EntryStatus != 0x00) && mGetNumOfPriceEvts)
    {
      startTime = OTA2Native32(gaServerPriceTable[mIndex].EffectiveStartTime);
      if(mGetPriceStartTime <= startTime)
      {
        /* Send This Pubish Price */
       	status = ZCL_SendPublishPrice(&mAddrInfo, &gaServerPriceTable[mIndex], mIsInterPanFlag);
      	if(status == gZclSuccess_c)
      	{
          --mGetNumOfPriceEvts;
      	}
        
      }
      /* GO and send the next price */
      mIndex++;
      TS_SendEvent(gZclTaskId, gzclEvtHandleGetScheduledPrices_c);
    }
    
  }
}

/******************************************************************************/
static zbStatus_t ZCL_ProcessGetScheduledPricesReq
(
addrInfoType_t *pAddrInfo, 
zclCmdPrice_GetScheduledPricesReq_t * pGetScheduledPrice, 
bool_t IsInterPanFlag)
{
  FLib_MemCpy(&mAddrInfo, pAddrInfo, sizeof(addrInfoType_t));

  mGetPriceStartTime = OTA2Native32(pGetScheduledPrice->StartTime);
  mGetNumOfPriceEvts = pGetScheduledPrice->NumOfEvts;
  if (!mGetNumOfPriceEvts)
  {
    mGetNumOfPriceEvts = gNumofServerPrices_c;
    mGetPriceStartTime = 0x00000000; /*all price information*/
  }
  mIsInterPanFlag = IsInterPanFlag;
  mIndex = 0;
  TS_SendEvent(gZclTaskId, gzclEvtHandleGetScheduledPrices_c);
  return gZclSuccess_c;
} 
/******************************************************************************/

/******************************************************************************/
/******************************************************************************/
/* get address ready for InterPan reply */
static void PrepareInterPanForReply(InterPanAddrInfo_t *pAdrrDest, zbInterPanDataIndication_t *pIndication)
{
  
  pAdrrDest->srcAddrMode = pIndication->srcAddrMode;
  pAdrrDest->dstAddrMode = pIndication->srcAddrMode;
  FLib_MemCpy(pAdrrDest->dstPanId, pIndication->srcPanId, sizeof(zbPanId_t));
  FLib_MemCpy(pAdrrDest->dstAddr.aIeeeAddr, pIndication->aSrcAddr.aIeeeAddr, sizeof(zbIeeeAddr_t));
  FLib_MemCpy(pAdrrDest->aProfileId, pIndication->aProfileId, sizeof(zbProfileId_t));
  FLib_MemCpy(pAdrrDest->aClusterId, pIndication->aClusterId, sizeof(zbClusterId_t));
}

/******************************************************************************/
/* Handle PRICE Inter Pan Server\Client Indications                           */ 
/******************************************************************************/
zbStatus_t ZCL_InterPanPriceClusterClient
(
	zbInterPanDataIndication_t *pIndication, /* IN: */
	afDeviceDef_t *pDev                /* IN: */
) 
{
 zclCmd_t *pCmd;
zbStatus_t status = gZclSuccess_c;

(void)pDev;
pCmd = (zclCmd_t *)&(((zclFrame_t *)pIndication->pAsdu)->command);
if(*pCmd == gZclCmdPrice_PublishPriceRsp_c)
{
    status = ZCL_ProcessClientPublishPrice((zclCmdPrice_PublishPriceRsp_t *)(pCmd+sizeof(zclCmd_t)));
    if(status != gZclSuccess_c)
    {
      return status;
    }
    else
    {
      return ZCL_SendInterPriceAck(pIndication);
    }
}

 return gZclUnsupportedClusterCommand_c;
}

/******************************************************************************/
zbStatus_t ZCL_InterPanPriceClusterServer
(
zbInterPanDataIndication_t *pIndication, /* IN: */
afDeviceDef_t *pDev                /* IN: */
) 
{
  
  zclCmd_t Cmd;
  zbStatus_t status = gZclSuccessDefaultRsp_c;
  addrInfoType_t replyAddrInfo;
  zclCmdPrice_GetCurrPriceReq_t *pGetCurrPrice;
  zclCmdPrice_GetScheduledPricesReq_t *pGetScheduledPrice;
  (void) pDev;
  /* Get the cmd and the SE message */
  Cmd = ((zclFrame_t *)pIndication->pAsdu)->command;
  /* get address ready for reply */
  PrepareInterPanForReply((InterPanAddrInfo_t *)&replyAddrInfo, pIndication); 
  switch(Cmd) {
  case gZclCmdPrice_GetCurrPriceReq_c:
    pGetCurrPrice = (zclCmdPrice_GetCurrPriceReq_t *)((uint8_t*)pIndication->pAsdu + sizeof(zclFrame_t));
    status = ZCL_ProcessGetCurrPriceReq(&replyAddrInfo, pGetCurrPrice, TRUE);
    
    break;
    
  case gZclCmdPrice_GetScheduledPricesReq_c:
    pGetScheduledPrice = (zclCmdPrice_GetScheduledPricesReq_t *)((uint8_t*)pIndication->pAsdu + sizeof(zclFrame_t));
    status = ZCL_ProcessGetScheduledPricesReq(&replyAddrInfo, pGetScheduledPrice, TRUE);
    break;  
    
  case gZclCmdPrice_PriceAck_c:
    status = gZclSuccess_c;
    break;
    
  default:
    status = gZclUnsupportedClusterCommand_c;
    break;
    
  }
  
  return status;
  
}


/******************************************************************************/
void ZCL_DeleteClientPrice(uint8_t *pEvtId)
{
  uint8_t i;
  for(i = 0; i < gNumofClientPrices_c; i++)
  {
    if (gaClientPriceTable[i].EntryStatus != 0x00)
    {
      if(FLib_MemCmp(pEvtId, gaClientPriceTable[i].Price.IssuerEvt, 4))
      {
        gaClientPriceTable[i].EntryStatus = 0x00;
        return;
      }
      
    }  
  }
}

/******************************************************************************/
void ZCL_DeleteServerScheduledPrices(void)
{
  uint8_t i;
  for(i = 0; i < gNumofServerPrices_c; i++)
    gaServerPriceTable[i].EntryStatus = 0x00;
}

/******************************************************************************/
/*Update a price from the Provider.
    In the case of an update to the
pricing information from the commodity provider, the Publish Price command
should be unicast to all individually registered devices implementing the Price
Cluster on the ZigBee Smart Energy network. When responding to a request via
the Inter-PAN SAP, the Publish Price command should be broadcast to the PAN of
the requester after a random delay between 0 and 0.5 seconds, to avoid a potential
broadcast storm of packets.*/
zbStatus_t ZCL_UpdateServerPriceEvents( zclCmdPrice_PublishPriceRsp_t *pMsg)
{
  uint8_t updateIndex;
  
  updateIndex = CheckForPriceUpdate(pMsg, (publishPriceEntry_t *)&gaServerPriceTable[0], gNumofServerPrices_c);
  /* the Price is updated??? */
  if (updateIndex < 0xFE)
  {
    /* Send the Publish Prices to all SE or Non-SE registered devices */
    mRegIndex = 0; 
    mInterPanIndex = 0;
    mUpdatePriceIndex = updateIndex;
    TS_SendEvent(gZclTaskId, gzclEvtHandlePublishPriceUpdate_c);
    return gZclSuccess_c;
  }
  else
    return gZclFailure_c;
}

/******************************************************************************/
/* Store Price information received from the Provider 
     The server side doesn't keep track of the price status, only stores the 
     received prices and take care that Nested and overlapping Publish Price commands not to occur*/
zbStatus_t ZCL_ScheduleServerPriceEvents ( zclCmdPrice_PublishPriceRsp_t *pMsg)
{
  uint8_t i, newEntry;
  uint32_t currentTime, startTime,  stopTime;
  uint32_t msgStartTime, msgStopTime; 
  uint16_t duration;

  /* here get the currentTime */
  currentTime = ZCL_GetUTCTime();
  msgStartTime = OTA2Native32(pMsg->StartTime);
  if((msgStartTime == 0x00000000) || (msgStartTime == 0xffffffff))
    msgStartTime = currentTime;
  duration = OTA2Native16(pMsg->DurationInMinutes);
  msgStopTime = msgStartTime + (60*(uint32_t)duration);
  
  //if(msgStopTime <= currentTime)
  //	return status;
  
  /* Nested and overlapping Publish Price commands are not allowed */
  for(i = 0; i < gNumofServerPrices_c; i++)
  {
    
    if(gaServerPriceTable[i].EntryStatus == 0x00)
      continue;
    /* Get the timing */
    startTime = OTA2Native32(gaServerPriceTable[i].EffectiveStartTime);
    duration = OTA2Native16(gaServerPriceTable[i].Price.DurationInMinutes);
    stopTime = startTime + (60*(uint32_t)duration);
    if ((msgStartTime >= stopTime)||(startTime >= msgStopTime))
      continue;
    /*the price overlapp... take no action*/
    return gZclFailure_c;
  }
  
  
  newEntry = AddPriceInTable((publishPriceEntry_t *)&gaServerPriceTable[0], gNumofServerPrices_c, pMsg);
  if(newEntry == 0xff)
    return gZclFailure_c;
 return gZclSuccess_c;
} 

/******************************************************************************/
zbStatus_t ZCL_PriceClusterServer
(
     zbApsdeDataIndication_t *pIndication, /* IN: */
     afDeviceDef_t *pDev                /* IN: */
) 
{
  
  zclCmd_t Cmd;
  zbStatus_t status = gZclSuccessDefaultRsp_c;
  addrInfoType_t replyAddrInfo;
  zclCmdPrice_GetCurrPriceReq_t *pGetCurrPrice;
  zclCmdPrice_GetScheduledPricesReq_t *pGetScheduledPrice;
  (void) pDev;
  /* Get the cmd and the SE message */
  Cmd = ((zclFrame_t *)pIndication->pAsdu)->command;
  /* get address ready for reply */
  AF_PrepareForReply((afAddrInfo_t*)&replyAddrInfo, pIndication); 
  switch(Cmd) {
  case gZclCmdPrice_GetCurrPriceReq_c:
    pGetCurrPrice = (zclCmdPrice_GetCurrPriceReq_t *)((uint8_t*)pIndication->pAsdu + sizeof(zclFrame_t));
    status = ZCL_ProcessGetCurrPriceReq(&replyAddrInfo, pGetCurrPrice, FALSE);
    break;
    
  case gZclCmdPrice_GetScheduledPricesReq_c:
    pGetScheduledPrice = (zclCmdPrice_GetScheduledPricesReq_t *)((uint8_t*)pIndication->pAsdu + sizeof(zclFrame_t));
    status = ZCL_ProcessGetScheduledPricesReq(&replyAddrInfo, pGetScheduledPrice, FALSE);
    break;
    
  case gZclCmdPrice_PriceAck_c:
    status = gZclSuccess_c;
    break;
    
  default:
    status = gZclUnsupportedClusterCommand_c;
    break;
    
  }
  
  return status;
}
  
/*****************************************************************************/

zbStatus_t ZCL_PriceClusterClient
(
     zbApsdeDataIndication_t *pIndication, /* IN: */
     afDeviceDef_t *pDev                /* IN: */
) 
{
  zclCmd_t *pCmd;
  zbStatus_t status;
  
  (void)pDev;
  pCmd = &(((zclFrame_t *)pIndication->pAsdu)->command);
  if(*pCmd == gZclCmdPrice_PublishPriceRsp_c)
  {
    status =  ZCL_ProcessClientPublishPrice((zclCmdPrice_PublishPriceRsp_t *)(pCmd+sizeof(zclCmd_t)));
    if(status != gZclSuccess_c)
    {
      return status;
    }
    else
    {
      return ZCL_SendPriceAck(pIndication);
    }
  }
  return gZclUnsupportedClusterCommand_c;
}

/******************************************************************************/
/******************************************************************************/
/* The LC Event command is generated in response to receiving a Get Scheduled Events
command */
static zbStatus_t ZCL_SendLdCtlEvt(addrInfoType_t *pAddrInfo,zclLdCtrl_EventsTableEntry_t * pMsg) 
{
  zclDmndRspLdCtrl_LdCtrlEvtReq_t req;
  
  FLib_MemCpy(&req.addrInfo, pAddrInfo, sizeof(afAddrInfo_t));	
  req.zclTransactionId =  gZclTransactionId++;
  FLib_MemCpy(&req.cmdFrame, (uint8_t *)pMsg + MbrOfs(zclLdCtrl_EventsTableEntry_t, cmdFrame), sizeof(zclCmdDmndRspLdCtrl_LdCtrlEvtReq_t));
  return ZclDmndRspLdCtrl_LdCtrlEvtReq(&req);
}


/******************************************************************************/
void ZCL_HandleGetScheduledLdCtlEvts(void)
{
  uint32_t  startTime;
  uint8_t status;
  
  if(mIndexLdCtl < gNumOfServerEventsTableEntry_c)
  {
    /* Check if the entry is used and if there are more scheduled events to be send*/
    if((gaServerEventsTable[mIndexLdCtl].EntryStatus != 0x00) && mGetNumOfLdCtlEvts)
    {
      startTime = OTA2Native32(gaServerEventsTable[mIndexLdCtl].cmdFrame.StartTime);
      if(mGetLdCtlEvtStartTime <= startTime)
      {
        /* Send This Load Control Event */
       	status = ZCL_SendLdCtlEvt(&mAddrInfo, &gaServerEventsTable[mIndexLdCtl]);
        if(status == gZclSuccess_c)
      	{
          --mGetNumOfLdCtlEvts;
      	}
        
      }
      /* GO and send the next Event */
      mIndexLdCtl++;
      TS_SendEvent(gZclTaskId, gzclEvtHandleGetScheduledLdCtlEvts_c);
    }
    
  }
}

/******************************************************************************/
static zbStatus_t ZCL_ProcessGetScheduledEvtsReq
(
addrInfoType_t *pAddrInfo, 
zclCmdDmndRspLdCtrl_GetScheduledEvts_t * pGetScheduledEvts
)
{
  FLib_MemCpy(&mAddrInfo, pAddrInfo, sizeof(addrInfoType_t));

  mGetLdCtlEvtStartTime = OTA2Native32(pGetScheduledEvts->EvtStartTime);
  mGetNumOfLdCtlEvts = pGetScheduledEvts->NumOfEvts;
  if (!mGetNumOfLdCtlEvts)
  {
    mGetNumOfLdCtlEvts = gNumOfServerEventsTableEntry_c;
    mGetLdCtlEvtStartTime = 0x00000000; /*all information*/
  }
  mIndexLdCtl = 0;
  TS_SendEvent(gZclTaskId, gzclEvtHandleGetScheduledLdCtlEvts_c);
  return gZclSuccess_c;
}
/*****************************************************************************/
/*****************************************************************************/

/******************************************************************************/
/* Create and Send the Report Event Status OTA...                             */ 
/*****************************************************************************/
zbStatus_t ZCL_SendReportEvtStatus
(
afAddrInfo_t *pAddrInfo, 
zclCmdDmndRspLdCtrl_LdCtrlEvtReq_t *pMsg, 
uint8_t eventStatus, 
bool_t invalidValueFlag /* if TRUE sent RES with invalid values for fields */
)
{
  zclDmndRspLdCtrl_ReportEvtStatus_t req;
  zclCmdDmndRspLdCtrl_ReportEvtStatus_t *pCmdFrame;
  uint32_t currentTime;
  Signature_t signature;
  
  pCmdFrame = &(req.cmdFrame);
  /* Prepare the RES */
  FLib_MemCpy(&req, pAddrInfo, sizeof(afAddrInfo_t));
  req.zclTransactionId = gZclTransactionId++;
  FLib_MemCpy(pCmdFrame->IssuerEvtID, pMsg->IssuerEvtID, sizeof(SEEvtId_t));
  pCmdFrame->EvtStatus = eventStatus; 
  /* here get the currentTime  */
  currentTime = ZCL_GetUTCTime();

  currentTime = Native2OTA32(currentTime);
  pCmdFrame->EvtStatusTime = currentTime;
  if(!invalidValueFlag)
  {
    pCmdFrame->CritLevApplied = pMsg->CritLev;
    FLib_MemCpy(&(pCmdFrame->CoolTempSetPntApplied), 
                &(pMsg->CoolingTempSetPoint), (2 + sizeof(int8_t) + 2*sizeof(LCDRSetPoint_t)) );
    
  }
  else
  {
    /* set invalid values */
    pCmdFrame->CritLevApplied = 0x00; 
    pCmdFrame->CoolTempSetPntApplied = Native2OTA16(gZclCmdDmndRspLdCtrl_OptionalTempSetPoint_c); 	
    pCmdFrame->HeatTempSetPntApplied = Native2OTA16(gZclCmdDmndRspLdCtrl_OptionalTempSetPoint_c);
    pCmdFrame->AverageLdAdjustmentPercentage = gZclCmdDmndRspLdCtrl_OptionalAverageLdAdjustmentPercentage_c;
    pCmdFrame->DutyCycle = gZclCmdDmndRspLdCtrl_OptionalDutyCycle_c;
    pCmdFrame->EvtCtrl =0x00;
  }
  pCmdFrame->SignatureType = gSELCDR_SignatureType_c;
  ZCL_ApplyECDSASign((uint8_t*)pCmdFrame, (sizeof(zclCmdDmndRspLdCtrl_ReportEvtStatus_t)-sizeof(Signature_t)), (uint8_t*)&signature);
  FLib_MemCpy(&(pCmdFrame->Signature[0]), &signature, sizeof(Signature_t));
  /* Send the RES over the air */
 return ZclDmndRspLdCtrl_ReportEvtStatus(&req);
}

/******************************************************************************/
/* Find an Event in the Events Table...                                                                                   */
uint8_t FindEventInEventsTable(uint8_t *pEvtId)
{
  uint8_t index; 
  zclLdCtrl_EventsTableEntry_t *pEntry = &gaEventsTable[0];
  for(index = 0; index < gNumOfEventsTableEntry_c; index++)
  {  
    /* if not used go to next entry*/  
    if(((zclLdCtrl_EventsTableEntry_t *)(pEntry+index))->EntryStatus == gEntryUsed_c)
    {
      if(FLib_MemCmp(((zclLdCtrl_EventsTableEntry_t *)(pEntry+index))->cmdFrame.IssuerEvtID, pEvtId, sizeof(SEEvtId_t)))
        return index; 
    }
  }
  return 0xFF; 
  
}   

/******************************************************************************/
/* An Entry is empty if the EntryStatus is NOT Used or NotSupersededClass is 0x0000 (all class bits was overlapped)*/
static uint8_t FindEmptyEntryInEventsTable(void)
{
  uint8_t index;
  zclLdCtrl_EventsTableEntry_t *pEntry;
  for(index = 0; index < gNumOfEventsTableEntry_c; index++)
  {  
    pEntry = &gaEventsTable[index];
    /* if not used go to next entry*/  
    if(pEntry->EntryStatus == gEntryNotUsed_c)
      return index;
    else /*if class is superseded, use that entry*/
      if (pEntry->NotSupersededClass == 0x0000 &&
          pEntry->SupersededTime == 0x00000000 )
        return index;
  }
        
  /*if all entries are used and the class bits wasn't overlaped, we have NO free entry in table*/
  /* keep the same entries */
  for(index = 0; index < gNumOfEventsTableEntry_c; index++)
  { 
    pEntry = &gaEventsTable[index];
    if(pEntry->EntryStatus == gEntryUsed_c)
    {
      pEntry->NotSupersededClass = 0xffff;
      pEntry->SupersededTime = 0xffffffff;
    }
  }
  return 0xff;
}  

/******************************************************************************/
/* An Entry is empty if the EntryStatus is NOT Used*/
static uint8_t FindEmptyEntryInServerEventsTable(void)
{
  uint8_t index;
  zclLdCtrl_EventsTableEntry_t *pEntry;
  for(index = 0; index < gNumOfServerEventsTableEntry_c; index++)
  {  
    pEntry = &gaServerEventsTable[index];
    /* if not used go to next entry*/  
    if(pEntry->EntryStatus == gEntryNotUsed_c)
      return index;
   }
  return 0xff;
}  

/******************************************************************************/
/* Add a new Event in the EventsTable */
static void AddNewEntry
(
zclLdCtrl_EventsTableEntry_t *pDst, 
zclCmdDmndRspLdCtrl_LdCtrlEvtReq_t *pSrc,
afAddrInfo_t *pAddrInfo
)
{
  FLib_MemCpy(&(pDst->addrInfo), pAddrInfo, sizeof(afAddrInfo_t));
  FLib_MemCpy(&(pDst->cmdFrame), pSrc, sizeof(zclCmdDmndRspLdCtrl_LdCtrlEvtReq_t));
  pDst->CurrentStatus = gSELCDR_LdCtrlEvtCode_CmdRcvd_c;
  pDst->NextStatus = gSELCDR_LdCtrlEvtCode_Started_c;
  pDst->IsSuccessiveEvent = FALSE;
  /*Set  fields as beeing invalid... not used yet*/
  BeeUtilSetToF(&(pDst->CancelTime), (2*sizeof(ZCLTime_t) + 2*sizeof(LCDRDevCls_t) +1));
  pDst->EntryStatus = gEntryUsed_c;
}

/******************************************************************************/
static void CheckForSuccessiveEvents(uint8_t msgIndex)
{
  zclCmdDmndRspLdCtrl_LdCtrlEvtReq_t *pEvent;
  zclLdCtrl_EventsTableEntry_t *pEntry,*pMsg;
  uint32_t msgStartTime, entryStartTime ;
  uint32_t msgStopTime, entryStopTime;
  uint16_t msgDuration, entryDuration; 
  uint8_t index;
  /* point to message */
  pMsg = &gaEventsTable[msgIndex];
  msgDuration = OTA2Native16(pMsg->cmdFrame.DurationInMinutes);
  msgStartTime = OTA2Native32(pMsg->cmdFrame.StartTime);
  msgStopTime = msgStartTime + ((uint32_t)msgDuration * 60);
  for(index = 0; index < gNumOfEventsTableEntry_c; index++)
  {  
    pEntry = &gaEventsTable[index];
    /* if not used go to next entry*/  
    if((pEntry->EntryStatus == gEntryNotUsed_c)||(index == msgIndex) )
      continue;
    /* point to entry */
    pEvent = (zclCmdDmndRspLdCtrl_LdCtrlEvtReq_t *)&pEntry->cmdFrame;
    /* check if the class is overlaping */
    if (!(pEvent->DevGroupClass & pMsg->cmdFrame.DevGroupClass))
      continue;	
    /* get the entry timing */
    entryDuration = OTA2Native16(pEvent->DurationInMinutes);
    entryStartTime = OTA2Native32(pEvent->StartTime);
    entryStopTime = entryStartTime + ((uint32_t)entryDuration * 60);
    /*Check if the new event is a succesive event */
    if ((msgStartTime == entryStopTime)||(entryStartTime == msgStopTime))
    {
      if(!pEntry->IsSuccessiveEvent)
        pEntry->IsSuccessiveEvent = TRUE;
      if(!pMsg->IsSuccessiveEvent)
        pMsg->IsSuccessiveEvent = TRUE;
    } 
  }/* end for(...) */
  
}


/******************************************************************************/
/* Accepting voluntary load control events */
void ZCL_AcceptVoluntaryLdCrtlEvt(bool_t flag)
{
  mAcceptVoluntaryLdCrtlEvt = flag;
}


/******************************************************************************/
/* Update the load contol table of events...
Check if the event can be handled(to set it having received status). 
An event is considered received if it is added in the EventsTable[], superseding or Not one or more events.
the received event should be filtered (Class, Utility) and the overlapping rules should be checked
*/ 
/*****************************************************************************/
static uint8_t ZCL_ScheduleEvents(afAddrInfo_t *pAddrInfo, zclCmdDmndRspLdCtrl_LdCtrlEvtReq_t *pMsg)
{
  uint8_t status = gZclSuccess_c;
  uint8_t utilityGroup;
  uint16_t devGroupClass, entryClass;
  uint32_t currentTime;
  uint32_t msgStartTime, entryStartTime ;
  uint32_t msgStopTime, entryStopTime, OTATime;
  uint16_t msgDuration, entryDuration;  
  zclCmdDmndRspLdCtrl_LdCtrlEvtReq_t *pEvent;
  zclLdCtrl_EventsTableEntry_t *pEntry;
  uint8_t index;
  //uint8_t attrData[4];
  uint8_t attrLen;
  
  /* the previous received event is still not added in the Table Entry */
  if (mNewEntryIndex != 0xff)
    return status;
  (void)ZCL_GetAttribute(pAddrInfo->srcEndPoint,pAddrInfo->aClusterId, gZclAttrUtilityGroup_c, &utilityGroup, &attrLen);
  (void)ZCL_GetAttribute(pAddrInfo->srcEndPoint,pAddrInfo->aClusterId, gZclAttrDevCls_c, &devGroupClass, &attrLen);
  
  /* Filter the device Class and Utility (if no match, no RES is send)*/
  pMsg->DevGroupClass = pMsg->DevGroupClass & devGroupClass;
  if((!pMsg->DevGroupClass) || (pMsg->UtilityGroup != utilityGroup))
    return gZclInvalidField_c; /* NO RES is send */

  /* Check the criticality Level */
  if((pMsg->CritLev > gZclCmdDmndRspLdCtrl_MaxCritLev_c )||
     (pMsg->CritLev >= gZclSECritLev_Green_c && pMsg->CritLev <= gZclSECritLev_5_c && !mAcceptVoluntaryLdCrtlEvt))
       return gZclInvalidField_c; /* NO RES is send */
  
  /* Check for duplicate event */
  if(FindEventInEventsTable((uint8_t*)&(pMsg->IssuerEvtID)) != 0xff)
    return gZclDuplicateExists_c; 
  
  /* Check event with the End Time in the past (Event Status set to 0xFB) */
  msgDuration = OTA2Native16(pMsg->DurationInMinutes);
  msgStartTime = OTA2Native32(pMsg->StartTime);
  
 /* here get the currentTime  */ 
  currentTime = ZCL_GetUTCTime();
  /*Event is started now???*/
  if(msgStartTime == 0x00000000)
  {
    /*Set the start time for the new event(keep the current time) */
    msgStartTime = currentTime;
    /*set it OTA value */
    OTATime = Native2OTA32(currentTime);
    pMsg->StartTime = OTATime;
  }
  msgStopTime = msgStartTime + ((uint32_t)msgDuration * 60);
  /* Event had expired?? */
  if (msgStopTime <= currentTime)
  {
    status = ZCL_SendReportEvtStatus(pAddrInfo, pMsg, gSELCDR_LdCtrlEvtCode_EvtHadExpired_c, FALSE);
    return status; 
  }
  
  /* Check the overlapping rules for scheduled and not executing events. */
  /* After the Report Event Status command is successfully sent, the End Device can remove the
  previous event schedule.(  Event can be received??...Can be added in the Events Table??... table is not full??).
  No RES will be send if table is full )
  */
  for(index = 0; index < gNumOfEventsTableEntry_c; index++)
  {  
    pEntry = &gaEventsTable[index];
    /* if not used go to next entry*/ 
    if(pEntry->EntryStatus == gEntryNotUsed_c)
      continue;
    /* point to entry */
    pEvent = (zclCmdDmndRspLdCtrl_LdCtrlEvtReq_t *)&pEntry->cmdFrame;
    /* check if the class is overlaping */
    entryClass = pEvent->DevGroupClass; 
    if (!(entryClass & pMsg->DevGroupClass))
      continue; 
    /* get the entry timing */
    entryDuration = OTA2Native16(pEvent->DurationInMinutes);
    entryStartTime = OTA2Native32(pEvent->StartTime);
    entryStopTime = entryStartTime + ((uint32_t)entryDuration * 60);
    
    /*Check if the new event and the entry event are overlaping */
    if ((msgStartTime >= entryStopTime)||(entryStartTime >= msgStopTime))
      continue;
    
    /* Here the entry of the table is Overlapped */
    /* overlap only the specified class */
    /* supersede the original event strictly for that device class */
    entryClass = entryClass & (entryClass^pMsg->DevGroupClass);
    /*if NotSupersededClass is 0x0000, all the class bits was overlapped (event will be superseded)*/
    pEntry->NotSupersededClass = entryClass;
    
    /* resolve the case when the running event is superseded with a event that will start later*/
    if((entryStartTime <= currentTime) && (msgStartTime > currentTime))
      OTATime = Native2OTA32(msgStartTime);
    else
      OTATime = 0x00000000;
    pEntry->SupersededTime =OTATime;
    
  } /* end for(index = 0; index...*/
  
  /* Find an empty entry to schedule it */  
  mNewEntryIndex = FindEmptyEntryInEventsTable();
  if(mNewEntryIndex != 0xff)
  { /* keep the new event info */
    FLib_MemCpy(&mNewLDCtrlEvt, pMsg, sizeof(zclCmdDmndRspLdCtrl_LdCtrlEvtReq_t));
    FLib_MemCpy(&mNewAddrInfo,pAddrInfo, sizeof(mNewAddrInfo));
    /* Send RES with Receiving Status, but the event is not added in the Events Table yet */
    status = ZCL_SendReportEvtStatus(&mNewAddrInfo, &mNewLDCtrlEvt, gSELCDR_LdCtrlEvtCode_CmdRcvd_c, FALSE);
  }
  else	
    return gZclFailure_c; /* event can't be load (or added in table) */
  
  
  if(status == gZclSuccess_c)
    TS_SendEvent(gZclTaskId, gZclEvtHandleLdCtrl_c);  
  return status;
}

/******************************************************************************/
/* Process the Cancel Load Control Event command                                                            */ 
/*****************************************************************************/
static zbStatus_t ZCL_ProcessCancelLdCtrlEvt(zbApsdeDataIndication_t *pIndication)
{

  uint8_t status = gZclSuccess_c;
  zclCmdDmndRspLdCtrl_CancelLdCtrlEvtReq_t *pMsg;
  zclLdCtrl_EventsTableEntry_t *pEntry;
  uint8_t index;
  afAddrInfo_t addrInfo;
  uint32_t msgCancelTime, entryStopTime, currentTime;
  uint16_t duration;
  uint8_t utilityGroup;
  uint8_t attrLen;

  /* get address ready for reply */
  AF_PrepareForReply(&addrInfo, pIndication); 
  pMsg = (zclCmdDmndRspLdCtrl_CancelLdCtrlEvtReq_t *)((uint8_t*)pIndication->pAsdu +sizeof(zclFrame_t));
  (void)ZCL_GetAttribute(addrInfo.srcEndPoint,addrInfo.aClusterId, gZclAttrUtilityGroup_c, &utilityGroup, &attrLen);

  if(pMsg->UtilityGroup != utilityGroup)
  {
    /* send RES (InvalidCancelCmdDefault) with invalid values*/
    status = ZCL_SendReportEvtStatus(&addrInfo,(zclCmdDmndRspLdCtrl_LdCtrlEvtReq_t *)pMsg, gSELCDR_LdCtrlEvtCode_EvtInvalidCancelCmdDefault_c, TRUE);
    return status; 
  }
  /* Find the entry table event */
  index = FindEventInEventsTable(pMsg->IssuerEvtID);
  if(index == 0xff)
  {
    /* send RES (undef status ) with invalid values*/
    status = ZCL_SendReportEvtStatus(&addrInfo,(zclCmdDmndRspLdCtrl_LdCtrlEvtReq_t *)pMsg, gSELCDR_LdCtrlEvtCode_EvtUndef_c, TRUE);
    return status; 
  }
  else
  {
  pEntry = &gaEventsTable[index];
  pMsg->DevGroupClass &= pEntry->cmdFrame.DevGroupClass;
  }
  
  /* class is not matching; take NO action */
  if(!pMsg->DevGroupClass)
    return status;
  
  msgCancelTime = OTA2Native32(pMsg->EffectiveTime);
  /* here get the currentTime */
  currentTime = ZCL_GetUTCTime();
  
  /*Event is Canceled Now???*/
  if(msgCancelTime == 0x00000000)
  {
    msgCancelTime = currentTime;
    currentTime = Native2OTA32(currentTime);
    pMsg->EffectiveTime = currentTime;
  }
  
  entryStopTime = OTA2Native32(pEntry->cmdFrame.StartTime);
  duration = OTA2Native16(pEntry->cmdFrame.DurationInMinutes);
  entryStopTime = entryStopTime + ((uint32_t)duration * 60);
  
  if(entryStopTime <= msgCancelTime)
  {
    /* send RES (invalid effective time) with invalid values */
    status = ZCL_SendReportEvtStatus(&addrInfo,(zclCmdDmndRspLdCtrl_LdCtrlEvtReq_t *)pMsg, gSELCDR_LdCtrlEvtCode_EvtInvalidEffectiveTime_c, TRUE);
	return status; 
  }
  /* Set the fields information and let the ZCL_HandleScheduledEventNow() to  handle it */
  pEntry->CancelCtrl = pMsg->CancelCtrl; /* Set the Cancel Control */
  pEntry->CancelClass = pMsg->DevGroupClass; /*for what class the event is canceled*/  
  pEntry->CancelTime = pMsg->EffectiveTime; /* set cancel time */
  
  if (status == gZclSuccess_c)
       TS_SendEvent(gZclTaskId, gZclEvtHandleLdCtrl_c);  
  
  return status;
  
}



/******************************************************************************/
/* Process the Cancel All Load Control Event command...                                */ 
/*****************************************************************************/
static zbStatus_t ZCL_ProcessCancelAllLdCtrlEvtst(zbApsdeDataIndication_t * pIndication)
{
  
  uint8_t status;
  zclCmdDmndRspLdCtrl_CancelAllLdCtrlEvtsReq_t *pMsg;
  zclLdCtrl_EventsTableEntry_t *pEntry;
  uint8_t index;
  
  pMsg = (zclCmdDmndRspLdCtrl_CancelAllLdCtrlEvtsReq_t *)((uint8_t*)pIndication->pAsdu +sizeof(zclFrame_t));
  status = gZclFailure_c;
  for(index = 0; index < gNumOfEventsTableEntry_c; index++)
  {  
    pEntry = &gaEventsTable[index];
    /* if not used go to next entry*/  
    if(pEntry->EntryStatus == gEntryUsed_c)
    {
      pEntry->CancelCtrl = pMsg->CancelCtrl; 
      pEntry->CancelClass = 0xffff; /*invalid... not used */ 
      pEntry->CancelTime = 0x00000000; /* cancel now */
      status = gZclSuccess_c; 
    }
  }
  
  if (status == gZclSuccess_c)
        TS_SendEvent(gZclTaskId, gZclEvtHandleLdCtrl_c);  
  
  return status;

}

/******************************************************************************/
/*  Timer Callback that handles the scheduled Events */ 
static void LdCtrlEvtTimerCallback(tmrTimerID_t timerID)
{
   (void) timerID;
   TS_SendEvent(gZclTaskId, gZclEvtHandleLdCtrl_c);  
}


/******************************************************************************/
/* LdCtrlJitterTimerCallBack()     */ 
/*****************************************************************************/
void LdCtrlJitterTimerCallBack(tmrTimerID_t timerID)
{
  zclLdCtrl_EventsTableEntry_t *pEntry;
  uint8_t status, previousEntryStatus;
  
  pEntry = (zclLdCtrl_EventsTableEntry_t*)&gaEventsTable[mReportStatusEntryIndex];
  if (pEntry->EntryStatus == gEntryNotUsed_c)
    return;
  
  /*save the previuos status */
  previousEntryStatus = pEntry->CurrentStatus; 
  
  /* Check if the event is already started and an "OptOUT" status was sent */
  if((previousEntryStatus == gSELCDR_LdCtrlEvtCode_UserHaveToChooseOptOut_c)&&
     ((pEntry->NextStatus == gSELCDR_LdCtrlEvtCode_Completed_c)||
      (pEntry->NextStatus == gSELCDR_LdCtrlEvtCode_EvtCompletedWithNoUser_c)) )
    pEntry->NextStatus = gSELCDR_LdCtrlEvtCode_EvtPrtlCompletedWithUserOptOut_c;
  /* check if an "OptIn" status was sent previous
  and the event was started or already had been started */
  if((previousEntryStatus == gSELCDR_LdCtrlEvtCode_UserHaveToChooseOptIn_c)&&
     (pEntry->NextStatus == gSELCDR_LdCtrlEvtCode_Completed_c))
    pEntry->NextStatus = gSELCDR_LdCtrlEvtCode_EvtPrtlCompletedWithUserOptIn_c;
  
  /*check if the event was started and before it was sent an "OptOut" status (to set the next status EvtCompletedWithNoUser) */
  if((previousEntryStatus == gSELCDR_LdCtrlEvtCode_UserHaveToChooseOptOut_c)&&
     (pEntry->NextStatus == gSELCDR_LdCtrlEvtCode_Started_c))
    pEntry->NextStatus = gSELCDR_LdCtrlEvtCode_EvtCompletedWithNoUser_c;
  
  status = ZCL_SendReportEvtStatus((afAddrInfo_t *)pEntry, &pEntry->cmdFrame, pEntry->NextStatus, FALSE);
  if (status == gZclSuccess_c)
  {
    /* set the current status (the Report Evt Status was sent)*/
    pEntry->CurrentStatus = pEntry->NextStatus;
    
    /* Call the BeeAppUpdateDevice */
    /* user should check the CurrentStatus of the Event */
    BeeAppUpdateDevice(((afAddrInfo_t *)pEntry)->srcEndPoint, gZclUI_LdCtrlEvt_c, 0, ((afAddrInfo_t *)pEntry)->aClusterId, pEntry); 
    
    if(pEntry->CurrentStatus == gSELCDR_LdCtrlEvtCode_Started_c)
      pEntry->NextStatus = gSELCDR_LdCtrlEvtCode_Completed_c;
    else
      if((pEntry->CurrentStatus == gSELCDR_LdCtrlEvtCode_Completed_c)||
         (pEntry->CurrentStatus == gSELCDR_LdCtrlEvtCode_EvtCompletedWithNoUser_c)||
           (pEntry->CurrentStatus == gSELCDR_LdCtrlEvtCode_EvtPrtlCompletedWithUserOptIn_c)||
             (pEntry->CurrentStatus == gSELCDR_LdCtrlEvtCode_EvtPrtlCompletedWithUserOptOut_c)||
               (pEntry->CurrentStatus == gSELCDR_LdCtrlEvtCode_EvtCancelled_c)||
                 (pEntry->CurrentStatus == gSELCDR_LdCtrlEvtCode_EvtSuperseded_c) )
        pEntry->EntryStatus = gEntryNotUsed_c;
    
    /* check if an "OptIn" status was sent previous
    and the event was started or already had been started */
    if((previousEntryStatus == gSELCDR_LdCtrlEvtCode_UserHaveToChooseOptIn_c)&&
       (pEntry->CurrentStatus == gSELCDR_LdCtrlEvtCode_Started_c) )
      pEntry->NextStatus = gSELCDR_LdCtrlEvtCode_EvtPrtlCompletedWithUserOptIn_c;
    
  }/* end  if (status == gZclSuccess_c) */
  else
  {
    /* Try again later */
    TMR_StartSingleShotTimer (timerID, 10, LdCtrlJitterTimerCallBack);
    return;
  }
  /* Handle the next Load Control Event*/             
  TS_SendEvent(gZclTaskId, gZclEvtHandleLdCtrl_c); 
}

/******************************************************************************/
/* Handle the Scheduled events(find the nerest and shortest event that has to ocur...   )          */ 
/*****************************************************************************/
void ZCL_HandleReportEventsStatus(void)
{
  uint16_t randomTime;
  /* The timer should be allocated */
  if(!jitterTimerID)
    return;
  
  if(!TMR_IsTimerActive(jitterTimerID))
  { 
    /* Delayed the Report Status Event after a random delay between 0 and 5 seconds, to avoid a potential storm of
    packets; this is done if a timer can be allocated */ 
    /* Get a random time and send the Report Status Event */
    randomTime = (uint16_t)GetRandomRange(10, 50);
    TMR_StartSingleShotTimer(jitterTimerID, randomTime, LdCtrlJitterTimerCallBack); 
  }
  else
  {
    /* handle next time */
    TS_SendEvent(gZclTaskId, gZclEvtHandleLdCtrl_c); 
  }
}

/******************************************************************************/
/* Handle the Scheduled events(find the nerest and shortest event that has to ocur...   )          */ 
/*****************************************************************************/

void ZCL_HandleScheduledEventNow(void)
{
  static uint8_t startRandomize, stopRandomize;
  uint32_t currentTime;
  uint16_t cancelClass, notSupersededClass;
  uint32_t entryStartTime, entryStopTime, cancelTime, minTiming = 0xffffffff, supersededTime;
  uint16_t entryDuration;  
  zclCmdDmndRspLdCtrl_LdCtrlEvtReq_t *pEvent;
  zclLdCtrl_EventsTableEntry_t *pEntry;
  uint8_t noEntryFlag = FALSE;
  uint8_t index, randomTime;
  uint8_t attrLen;
  
  /* If a new entry was received, add it in the table if the entry is not used; 
     if it is used then supersede it first  */
  if(mNewEntryIndex != 0xff)
  {
    zclLdCtrl_EventsTableEntry_t *pNewEntry = &gaEventsTable[mNewEntryIndex];
    if (pNewEntry->EntryStatus == gEntryNotUsed_c)
    {
      /* add the new event and check it for succesive events */
      AddNewEntry(pNewEntry, &mNewLDCtrlEvt, &mNewAddrInfo);
      CheckForSuccessiveEvents(mNewEntryIndex);
      /* Call the App to signal that an Load Control Event was received. The user should check the Current Status */
      BeeAppUpdateDevice(mNewAddrInfo.srcEndPoint, gZclUI_LdCtrlEvt_c, 0, mNewAddrInfo.aClusterId, pNewEntry); 
      /* if a new event is received get the randomize timing */
      mGetRandomFlag = TRUE;
      /* New events can be received */
      mNewEntryIndex = 0xff;
      /* Handle the next Load Control Event*/             
      TS_SendEvent(gZclTaskId, gZclEvtHandleLdCtrl_c); 
      return;

    }
    else
    {
      /*all Class bits are overlapped; supersede the entry */
      pNewEntry->NextStatus = gSELCDR_LdCtrlEvtCode_EvtSuperseded_c;
      mReportStatusEntryIndex =mNewEntryIndex;
      /* send the RES */
      TS_SendEvent(gZclTaskId, gzclEvtHandleReportEventsStatus_c);
      return;
    }
    
  }
    /*
    Randomization:
    - maintain its current state in the random period...
    - Check Effective End Time overlaps the Effective Start Time of a Successive Event, the Effective Start Time takes precedence
    - Devices shall not artificially create a gap between Successive Events
    ( use Start < Stop Randomization to prevent this)
    - It is permissible to have gaps when events are not Successive Events or Overlapping Events
    */
    if(mGetRandomFlag)
    {
      mGetRandomFlag = FALSE;
      (void)ZCL_GetAttribute(mNewAddrInfo.srcEndPoint, mNewAddrInfo.aClusterId, gZclAttrStartRandomizeMin_c, &startRandomize, &attrLen);
      (void)ZCL_GetAttribute(mNewAddrInfo.srcEndPoint, mNewAddrInfo.aClusterId, gZclAttrStopRandomizeMin_c, &stopRandomize, &attrLen);
      if(startRandomize)
        startRandomize = GetRandomRange(0, startRandomize);
      if(stopRandomize)
        stopRandomize = GetRandomRange(0, stopRandomize);
    }
  
    /* here get the currentTime  */ 
    currentTime = ZCL_GetUTCTime();
    for(index = 0; index < gNumOfEventsTableEntry_c; index++)
    { 
      pEntry = &gaEventsTable[index];
      /* if not used go to next entry*/  
      if(pEntry->EntryStatus == gEntryNotUsed_c)
        continue;
	  /* there are entries in table(not all entries had expired) */
      noEntryFlag = TRUE;
      /* point to event */
      pEvent = (zclCmdDmndRspLdCtrl_LdCtrlEvtReq_t *)&pEntry->cmdFrame;

/* Handle the supersed time */
      /*get the NotSupersededClass */
      notSupersededClass = pEntry->NotSupersededClass;
      /* Event is superseded???*/
      if(notSupersededClass != 0xffff)
      {
        /*All class bits are superseded???*/
        if(notSupersededClass == 0x0000)
        { 
          supersededTime = OTA2Native32(pEntry->SupersededTime);
          if(supersededTime <= currentTime)
          {
            /*all Class bits are overlapped; supersede the entry */
            pEntry->NextStatus = gSELCDR_LdCtrlEvtCode_EvtSuperseded_c;
            mReportStatusEntryIndex =index;
            /* send the RES */
            TS_SendEvent(gZclTaskId, gzclEvtHandleReportEventsStatus_c);
            return;
          }
        }          
        else 
        {
          /*Keep the entry with the class bits that was not superseded */  
          pEntry->cmdFrame.DevGroupClass = notSupersededClass;
          pEntry->NotSupersededClass = 0xffff;
        }  
      }

/* Handle the cancel time */
      /* get the timing */
      cancelTime = OTA2Native32(pEntry->CancelTime);
      /* Event is Canceled??? */
      if (cancelTime != 0xffffffff)
      {
        /*Is it using randomization???*/
        if(pEntry->CancelCtrl & gCancelCtrlEndRandomization_c)
          randomTime = stopRandomize;
        else
          randomTime = 0;
        /* Check if the cmd have to execute */
        cancelTime = cancelTime + (60 * (uint32_t)randomTime); /*random time is in minutes*/
        if(cancelTime <= currentTime)
        {
          cancelClass= pEntry->CancelClass;
          /*Cancel only the specified class*/
          notSupersededClass = pEvent->DevGroupClass & (cancelClass^pEvent->DevGroupClass);
          /*check if the Class is specified (CancelAll cmd don't specifies the class)*/
          if((cancelClass != 0xffff) && notSupersededClass)
          {           
            /*Keep the entry that wasn't Overlapped */
            pEvent->DevGroupClass = notSupersededClass;
            /*Set Cancel fields as beeing invalid... not used yet*/
            BeeUtilSetToF(&(pEntry->CancelTime), (sizeof(ZCLTime_t) + sizeof(LCDRDevCls_t) +1));
          }
          else
          {
            mReportStatusEntryIndex =index;
            pEntry->NextStatus = gSELCDR_LdCtrlEvtCode_EvtCancelled_c;
            TS_SendEvent(gZclTaskId, gzclEvtHandleReportEventsStatus_c);
            return;
          }          
        }/* if(cancelTime  <= currentTime)...*/
        else /* else      if(cancelTime <= currentTime)...*/
        {
          if( minTiming > cancelTime)
            minTiming = cancelTime;
        } 
      }/* if (cancelTime != 0xffffffff)...*/

/* Handle the start time and stop time */
      entryDuration = OTA2Native16(pEvent->DurationInMinutes);
      entryStartTime = OTA2Native32(pEvent->StartTime);
      entryStopTime = entryStartTime + ((uint32_t)entryDuration * 60);
      
      /*Is it using randomization???*/
      if(pEvent->EvtCtrl & 0x01)
      {
        /* If it is an succesive event, use the shortest random time for entryStartTime (to avoid gaps)*/
        if(pEntry->IsSuccessiveEvent)
          randomTime = (startRandomize <= stopRandomize)?startRandomize:stopRandomize;
        else
          randomTime = startRandomize;
      }
      else
        randomTime = 0;
      entryStartTime = entryStartTime + (60 * (uint32_t)randomTime); /*random time is in minutes*/
      
      if(pEvent->EvtCtrl & 0x02)
        randomTime = stopRandomize;
      else
        randomTime = 0;    
      entryStopTime = entryStopTime + (60 * (uint32_t)randomTime); /*random time is in minutes*/
      /* Check if the cmd have to execute */
      if( ((entryStartTime <= currentTime)&&(pEntry->NextStatus == gSELCDR_LdCtrlEvtCode_Started_c))||
         (entryStopTime <= currentTime) )
      {
        /* Send the RES */
        mReportStatusEntryIndex = index;
        TS_SendEvent(gZclTaskId, gzclEvtHandleReportEventsStatus_c);
        return; 
      }
      
      if(entryStartTime > currentTime)
      {
        if( minTiming > entryStartTime)
          minTiming = entryStartTime;
      }
      else
        if(entryStopTime > currentTime)
        {
          if( minTiming > entryStopTime)
            minTiming = entryStopTime;
        } 
    } /* end  for(index = 0; index < gNumOfEventsTableEntry_c; */
    
    /* Get Random Timing... next time when ZCL_HandleScheduledEventNow() is called  */
    mGetRandomFlag = TRUE;
    if(!noEntryFlag || (minTiming == 0xffffffff))
      TMR_StopSecondTimer (gLdLdCtrlTimerID);
    else{
      TMR_StartSecondTimer (gLdLdCtrlTimerID, (uint16_t)(minTiming - currentTime), LdCtrlEvtTimerCallback);
    }
}


/*****************************************************************************
  ZCL_SetOptStatusOnEvent

  Set "Opt In" or "Opt Out" Status for an event Id.
*****************************************************************************/
uint8_t ZCL_SetOptStatusOnEvent(uint8_t *pEvtId, uint8_t optStatus) 
{
  uint8_t index;
  afAddrInfo_t *pAddrInfo;
  
  if(!pEvtId) 
  {
    return 0xff;
  }
  
  index = FindEventInEventsTable(pEvtId);
  if(index == 0xff) { 
    return 0xff;
  }
  
  pAddrInfo =  (afAddrInfo_t *)(&gaEventsTable[index]);
  /* Set the CurrentStatus */
  gaEventsTable[index].CurrentStatus =  optStatus;
  (void)ZCL_SendReportEvtStatus(pAddrInfo, (zclCmdDmndRspLdCtrl_LdCtrlEvtReq_t *)&(gaEventsTable[index].cmdFrame), optStatus, FALSE);
  BeeAppUpdateDevice(pAddrInfo->srcEndPoint, gZclUI_LdCtrlEvt_c, 0, pAddrInfo->aClusterId, &gaEventsTable[index]);
  return index;
}

/*****************************************************************************
  ZCL_DeleteLdCtrlEvent()

  Set "Opt In" or "Opt Out" Status for an event Id.
*****************************************************************************/
uint8_t ZCL_DeleteLdCtrlEvent(uint8_t *pEvtId) 
{
  uint8_t index;
  
  if(!pEvtId) 
  {
    return 0xff;
  }
  
  index = FindEventInEventsTable(pEvtId);
  if(index == 0xff) { 
    return 0xff;
  }
  
  gaEventsTable[index].EntryStatus = gEntryNotUsed_c;
  return index;
}

/******************************************************************************/
/* Process the Received Load Control Event....                                */ 
/*****************************************************************************/
static zbStatus_t ZCL_ProcessLdCtrlEvt(zbApsdeDataIndication_t *pIndication)
{
  uint8_t status = gZclSuccess_c;
  zclCmdDmndRspLdCtrl_LdCtrlEvtReq_t *pMsg;
  afAddrInfo_t addrInfo;
  uint16_t durationInMinutes;
  uint16_t cooling, heating;
  
  /* get the load control event request */
  pMsg = (zclCmdDmndRspLdCtrl_LdCtrlEvtReq_t *)((uint8_t*)pIndication->pAsdu + sizeof(zclFrame_t));
  /* get address ready for reply */
  AF_PrepareForReply(&addrInfo, pIndication); 
  /* check the fields ranges */
  durationInMinutes = OTA2Native16(pMsg->DurationInMinutes);
  if( durationInMinutes > gZclCmdDmndRspLdCtrl_MaxDurationInMinutes_c  )
    status = gZclInvalidField_c;
  
  cooling = OTA2Native16(pMsg->CoolingTempSetPoint);
  heating = OTA2Native16(pMsg->HeatingTempSetPoint); 
  if( (cooling != gZclCmdDmndRspLdCtrl_OptionalTempSetPoint_c) &&
     (heating != gZclCmdDmndRspLdCtrl_OptionalTempSetPoint_c) &&
       (pMsg->CoolingTempOffset != gZclCmdDmndRspLdCtrl_OptionalTempOffset_c) &&
         (pMsg->HeatingTempOffset != gZclCmdDmndRspLdCtrl_OptionalTempOffset_c) )
  {
    if(cooling > gZclCmdDmndRspLdCtrl_MaxTempSetPoint_c||
       heating > gZclCmdDmndRspLdCtrl_MaxTempSetPoint_c )
      status = gZclInvalidField_c;
  }
  else
  {
    if( (cooling == 0x8000 &&  heating != 0x8000)||
       (cooling != 0x8000 &&  heating == 0x8000)||
         (pMsg->CoolingTempOffset != 0xff && pMsg->HeatingTempOffset == 0xff)||
           (pMsg->CoolingTempOffset == 0xff && pMsg->HeatingTempOffset != 0xff))
      status = gZclInvalidField_c;
  }
  
  if( (pMsg->AverageLdAdjustmentPercentage > gZclCmdDmndRspLdCtrl_MaxAverageLdAdjustmentPercentage_c ||
       pMsg->AverageLdAdjustmentPercentage < gZclCmdDmndRspLdCtrl_MinAverageLdAdjustmentPercentage_c )&&
     pMsg->AverageLdAdjustmentPercentage != gZclCmdDmndRspLdCtrl_OptionalAverageLdAdjustmentPercentage_c )
    status = gZclInvalidField_c;
  
  if( pMsg->DutyCycle > gZclCmdDmndRspLdCtrl_MaxDutyCycle_c &&
     pMsg->DutyCycle != gZclCmdDmndRspLdCtrl_OptionalDutyCycle_c )
    status = gZclInvalidField_c;
  
  if(status == gZclInvalidField_c)
  {
    /* Send the Event status rejected */ 
    status = ZCL_SendReportEvtStatus(&addrInfo, pMsg, gSELCDR_LdCtrlEvtCode_LdCtrlEvtCmdRjctd_c, FALSE);  
  }
  else
  {
    status = ZCL_ScheduleEvents(&addrInfo, pMsg); 
  }
  return status;
}

/******************************************************************************/
void ZCL_DeleteServerScheduledEvents(void)
{
  uint8_t i;
  for(i = 0; i < gNumOfServerEventsTableEntry_c; i++)
    gaServerEventsTable[i].EntryStatus = 0x00;
}

/******************************************************************************/
/* Store Load Control information received from the Provider 
     The server side doesn't keep track of the  status, only stores the 
     received events and take care that Nested and overlapping  commands not to occur*/
zbStatus_t ZCL_ScheduleServerLdCtrlEvents (zclCmdDmndRspLdCtrl_LdCtrlEvtReq_t *pMsg)
{
  uint8_t i;
  uint32_t currentTime, startTime,  stopTime;
  uint32_t msgStartTime, msgStopTime; 
  uint16_t duration;

  /* here get the currentTime */
  currentTime = ZCL_GetUTCTime();
  msgStartTime = OTA2Native32(pMsg->StartTime);
  if((msgStartTime == 0x00000000) || (msgStartTime == 0xffffffff))
    msgStartTime = currentTime;
  duration = OTA2Native16(pMsg->DurationInMinutes);
  msgStopTime = msgStartTime + (60*(uint32_t)duration);
  
  //if(msgStopTime <= currentTime)
  //	return status;
  
  /* Nested and overlapping Load Control Event commands are not allowed */
  for(i = 0; i < gNumOfServerEventsTableEntry_c; i++)
  {
    if(gaServerEventsTable[i].EntryStatus == 0x00)
      continue;
    /* Get the timing */
    startTime = OTA2Native32(gaServerEventsTable[i].cmdFrame.StartTime);
    duration = OTA2Native16(gaServerEventsTable[i].cmdFrame.DurationInMinutes);
    stopTime = startTime + (60*(uint32_t)duration);
    if ((msgStartTime >= stopTime)||(startTime >= msgStopTime))
      continue;
    return gZclFailure_c;
  }
  
  mNewEntryIndex = FindEmptyEntryInServerEventsTable();
  
  if(mNewEntryIndex != 0xff)
  {
    /* add the new event and check it for succesive events */
    zclLdCtrl_EventsTableEntry_t *pNewEntry = &gaServerEventsTable[mNewEntryIndex];
    AddNewEntry(pNewEntry, pMsg, &mNewAddrInfo);
  }
              
  if(mNewEntryIndex == 0xff)
    return gZclFailure_c;
  return gZclSuccess_c;
} 



/* Procesing Client side Cluster for Demand Response and Load Control Event */
zbStatus_t ZCL_DmndRspLdCtrlClusterClient
(
zbApsdeDataIndication_t *pIndication, /* IN: */
afDeviceDef_t *pDev                /* IN: */
) 
{
  zclCmd_t Cmd;
  zbStatus_t status = gZclSuccessDefaultRsp_c;
  (void) pDev;
  /* Get the cmd and the SE message */
  Cmd = ((zclFrame_t *)pIndication->pAsdu)->command;
  switch(Cmd) {
  case gZclCmdDmndRspLdCtrl_LdCtrlEvtReq_c:
    status = ZCL_ProcessLdCtrlEvt(pIndication);
    if (status == gZclInvalidField_c) {      
    /* If there is a invalid field in the request, the request will be ignored and no
    response sent */
      status = gZclSuccess_c; 
    }
    break;
    
  case gZclCmdDmndRspLdCtrl_CancelLdCtrlEvtReq_c:
     status = ZCL_ProcessCancelLdCtrlEvt(pIndication);
    break;  
    
  case gZclCmdDmndRspLdCtrl_CancelAllLdCtrlEvtsReq_c:
     status = ZCL_ProcessCancelAllLdCtrlEvtst(pIndication);
    
    break;
    
  default:
    status = gZclUnsupportedClusterCommand_c;
    break;
    
  }
  
  return status;
  
}

/* Procesing Server side Cluster for Demand Response and Load Control Event */
zbStatus_t ZCL_DmndRspLdCtrlClusterServer
(
zbApsdeDataIndication_t *pIndication, /* IN: */
afDeviceDef_t *pDev                /* IN: */
) 
{
  zclCmd_t Cmd;
  addrInfoType_t replyAddrInfo;
  zbStatus_t status = gZclSuccessDefaultRsp_c;
  zclCmdDmndRspLdCtrl_GetScheduledEvts_t * pGetScheduledLdCtlEvt;
  
  (void) pDev;
  /* Get the cmd and the SE message */
  Cmd = ((zclFrame_t *)pIndication->pAsdu)->command;
  /* get address ready for reply */
  AF_PrepareForReply((afAddrInfo_t*)&replyAddrInfo, pIndication);
  switch(Cmd) {
  case gZclCmdDmndRspLdCtrl_GetScheduledEvts_c:
    pGetScheduledLdCtlEvt = (zclCmdDmndRspLdCtrl_GetScheduledEvts_t *)((uint8_t*)pIndication->pAsdu + sizeof(zclFrame_t));
    status = ZCL_ProcessGetScheduledEvtsReq(&replyAddrInfo, pGetScheduledLdCtlEvt);
    break;
  case gZclCmdDmndRspLdCtrl_ReportEvtStatus_c:
    status = gZclSuccess_c;
    break;
  default:
    status = gZclUnsupportedClusterCommand_c;
    break;
    
  }
  
  return status;
}

/*Updates the Meter Status Attribute*/
zbStatus_t ZclSmplMet_UpdateMetStatus
(
 zclSmplMet_UpdateMetStatus_t *pMsg
)
{
  zbClusterId_t ClusterId={gaZclClusterSmplMet_c};  
  return ZCL_SetAttribute(appEndPoint, ClusterId, gZclAttrSmplMetMSStatus_c, &pMsg->metStatus);
}

/*****************************************************************************/
zbStatus_t ZCL_SmplMetClusterServer
(
	zbApsdeDataIndication_t *pIndication, /* IN: */
	afDeviceDef_t *pDev                /* IN: */
) 
{
  zclCmd_t Cmd;
  zbStatus_t status = gZclSuccessDefaultRsp_c;

  (void) pDev;
  Cmd = ((zclFrame_t *)pIndication->pAsdu)->command;
  
  switch(Cmd) {
    case gZclCmdSmplMet_GetProfReq_c: 
    {
      zclFrame_t *pFrame;
      zclSmplMet_GetProfRsp_t *pRsp;
      zclCmdSmplMet_GetProfReq_t *pCmdFrame;
      pRsp = MSG_Alloc(sizeof(zclSmplMet_GetProfRsp_t) + (gMaxNumberOfPeriodsDelivered_c*3) );
      if(pRsp)
      {
        /* prepare for response in the address info (back to sender) */
        AF_PrepareForReply(&pRsp->addrInfo, pIndication);
        pFrame = (void *)pIndication->pAsdu;
        pRsp->zclTransactionId = pFrame->transactionId;
        pCmdFrame = (void *)(pFrame + 1);
        /* Buildng the response */
        ZCL_BuildGetProfResponse(pCmdFrame->IntrvChannel, pCmdFrame->EndTime, pCmdFrame->NumOfPeriods, &pRsp->cmdFrame);
        status = gZclSuccess_c;
        (void) ZclSmplMet_GetProfRsp(pRsp);        
        
        MSG_Free(pRsp);
      }
    }
    break;  
  case gZclCmdSmplMet_RemoveMirrorReq_c:
    break;
  case gZclCmdSmplMet_ReqFastPollModeReq_c:
    status = ZCL_ProcessReqFastPollEvt(pIndication);
    break;
  default:
    status = gZclUnsupportedClusterCommand_c;
    break;
  }
  
  return status;
}


static zbStatus_t ZCL_ProcessReqFastPollEvt
(
zbApsdeDataIndication_t *pIndication  //IN: pointer to APSDE Data Indication
)
{
#if gZclFastPollMode_d
  uint8_t temp8 = 0;
  zclSmplMet_ReqFastPollModeRsp_t response;
    
  zbStatus_t status = gZclSuccess_c;
  zclCmdSmplMet_ReqFastPollModeReq_t *pMsg;
  /* get the request fast poll mode request */
  pMsg = (zclCmdSmplMet_ReqFastPollModeReq_t *)((uint8_t*)pIndication->pAsdu + sizeof(zclFrame_t));
    
  /*set the values for the response*/
  if(ZCL_GetAttribute(pIndication->dstEndPoint,pIndication->aClusterId,gZclAttrSmplMetRISFastPollUpdatePeriod_c,&temp8, NULL))
  {
    return gZclFailure_c;
  }
  if(pMsg->updatePeriod <= temp8)
  {
    response.cmdFrame.appliedUpdatePeriod = temp8;
  }
  else
  {
    response.cmdFrame.appliedUpdatePeriod = pMsg->updatePeriod;
  }
  /*Calculate mcFastPollRemainingTime*/
  /*if the device hasn't allready reached the maximum time it can stay in fast poll mode, recalculate mcFastPollRemainingTime*/
  if(mcFastPollRemainingTime + mcTimeInFastPollMode < gASL_ZclSmplMet_MaxFastPollInterval_d)
  {
    /*If the device isn't in fast poll mode the timer is started*/
    if(!mcFastPollRemainingTime)
    {
      gFastPollTimerID = TMR_AllocateTimer();
      TMR_StartSecondTimer(gFastPollTimerID, 1, FastPollTimerCallBack);
    }
    /*Can the new required fast poll mode request time be included in the maximum fast poll interval time?*/
    if(pMsg->duration * 60 < gASL_ZclSmplMet_MaxFastPollInterval_d - mcTimeInFastPollMode)
    {
      /*If the new duration is higher than the fast poll remaining time update mcFastPollRemainingTime*/
      if(pMsg->duration * 60 > mcFastPollRemainingTime)
      {
        mcFastPollRemainingTime = pMsg->duration * 60;
      }/*else there is no need to update mcFastPollRemainingTime*/
    }
    else
    {
      mcFastPollRemainingTime = gASL_ZclSmplMet_MaxFastPollInterval_d - mcTimeInFastPollMode;
    }
    mcFastPollEndTime = ZCL_GetUTCTime() + (ZCLTime_t)mcFastPollRemainingTime;
  }/*else there is no need to update mcFastPollRemainingTime*/
  
  /*mcFastPollEndTime is used instead of ZCL_GetUTCTime() + (ZCLTime_t)mcFastPollRemainingTime because in some cases
  there can be a +/-1second difference in the reported time of 2 responses that should have the same end time*/
  response.cmdFrame.EndTime = Native2OTA32(mcFastPollEndTime);
  /*build the response*/
  AF_PrepareForReply(&response.addrInfo, pIndication);
  response.zclTransactionId = ((zclFrame_t *)pIndication->pAsdu)->transactionId;
  status = ZclSmplMet_ReqFastPollModeRsp(&response);
  return status;
#else
  (void)pIndication;
  (void)mcFastPollEndTime;
  return gZclUnsupportedClusterCommand_c;
#endif
}

static void FastPollTimerCallBack
(
tmrTimerID_t tmrID
)
{
  (void)tmrID;
  if(mcFastPollRemainingTime ==0)
  {
    TMR_FreeTimer(gFastPollTimerID);
    mcTimeInFastPollMode = 0;
  }
  else
  {
    mcFastPollRemainingTime--;
    mcTimeInFastPollMode++;
    TMR_StartSecondTimer(gFastPollTimerID, 1, FastPollTimerCallBack);
  }
}
#if gZclFastPollMode_d
/******************************************************************************
* ZclSmplMet_GetFastPollRemainingTime
*
* Returns
*   fastPollRemainingTime - Remaining time in seconds that the device must maintain
*                           fast poll mode.
*
******************************************************************************/
uint16_t ZclSmplMet_GetFastPollRemainingTime()
{
  return mcFastPollRemainingTime;
}
#endif
/******************************************************************************
* Identify Cluster Client, receives the  group cluster responses.
*
* IN: pIndication   pointer to indication. Must be non-null
* IN: pDevice       pointer to device definition. Must be non-null.
*
* Returns
*   gZbSuccess_c    worked
*   gZclUnsupportedClusterCommand_c   not supported response
*
******************************************************************************/
zbStatus_t ZCL_SmplMetClusterClient
  (
  zbApsdeDataIndication_t *pIndication, 
  afDeviceDef_t *pDevice
  )
{
  zclFrame_t *pFrame;
  zclCmd_t command;
  
  /* prevent compiler warning */
  (void)pDevice;
  pFrame = (void *)pIndication->pAsdu;
  /* handle the command */
  command = pFrame->command;
  
  switch(command) {
    case gZclCmdSmplMet_GetProfReq_c:
    case gZclCmdSmplMet_MirrorRsp_c:
    case gZclCmdSmplMet_MirrorRemovedRsp_c:
      return gZbSuccess_c;
#if gZclFastPollMode_d
    case gZclCmdSmplMet_ReqFastPollModeRsp_c:
      return gZbSuccess_c;
#endif
  default:
      return gZclUnsupportedClusterCommand_c;
  }
}

/*****************************************************************************/
void UpdatePowerConsumptionTimerCallBack(tmrTimerID_t tmrID)
{
  UpdatePowerConsumption();
  (void)tmrID; /* Unused parameter. */
}

void UpdatePowerConsumption(void)
{
  Summ_t newRISCurrSummDlvrd;
  uint8_t randomRange;
  uint8_t rangeIndexMax, rangeIndexMin;
  zbClusterId_t ClusterId={gaZclClusterSmplMet_c}; 
  /* This is only for demo, the intervals should be calculated by reading the meter and finding the used power from last reading */
  (void)ZCL_GetAttribute(appEndPoint, ClusterId, gZclAttrSmplMetRISCurrSummDlvrd_c, &newRISCurrSummDlvrd, NULL);  
  #if(gProfIntrvPeriod_c == gZclSEProfIntrvPeriod_Daily_c)
      rangeIndexMin=150;
      rangeIndexMax=255;
  #elif(gProfIntrvPeriod_c == gZclSEProfIntrvPeriod_60mins_c)
      rangeIndexMin=120;
      rangeIndexMax=224;
  #elif(gProfIntrvPeriod_c == gZclSEProfIntrvPeriod_30mins_c)  
      rangeIndexMin=80;
      rangeIndexMax=192;
  #elif(gProfIntrvPeriod_c == gZclSEProfIntrvPeriod_15mins_c)    
      rangeIndexMin=50;
      rangeIndexMax=160;
  #elif(gProfIntrvPeriod_c == gZclSEProfIntrvPeriod_10mins_c)      
      rangeIndexMin=30;
      rangeIndexMax=128;
  #elif(gProfIntrvPeriod_c == gZclSEProfIntrvPeriod_7dot5mins_c)
      rangeIndexMin=15;
      rangeIndexMax=96;
  #elif(gProfIntrvPeriod_c == gZclSEProfIntrvPeriod_5mins_c)
      rangeIndexMin=10;
      rangeIndexMax=64;
  #else(gProfIntrvPeriod_c == gZclSEProfIntrvPeriod_2dot5mins_c)  
      rangeIndexMin=0;
      rangeIndexMax=32;
  #endif
  
  randomRange = GetRandomRange(rangeIndexMin, rangeIndexMax);
  Add8ByteTo6ByteArray(randomRange, (uint8_t *)&newRISCurrSummDlvrd);
  (void)ZCL_SetAttribute(appEndPoint, ClusterId, gZclAttrSmplMetRISCurrSummDlvrd_c, &newRISCurrSummDlvrd);

  TMR_StartSecondTimer(gUpdatePowerConsumptionTimerID, gUpdateConsumption_c, UpdatePowerConsumptionTimerCallBack);  
}
/*****************************************************************************/
void GetProfileTestTimerCallBack(tmrTimerID_t tmrID)
{
  Consmp *consumption;
  Summ_t newRISCurrSummDlvrd;
  zbClusterId_t ClusterId={gaZclClusterSmplMet_c}; 
  (void)tmrID;
  
  ProfileIntervalTable[ProfIntrvHead].IntrvChannel = gIntrvChannel_ConsmpDlvrd_c;
  ProfileIntervalTable[ProfIntrvHead].endTime = ZCL_GetUTCTime();
  /* Simulate used power from last readings */
  (void)ZCL_GetAttribute(appEndPoint, ClusterId, gZclAttrSmplMetRISCurrSummDlvrd_c, &newRISCurrSummDlvrd, NULL);
  consumption = CalculateConsumptionFrom6ByteArray( (uint8_t *)&newRISCurrSummDlvrd, (uint8_t *)&LastRISCurrSummDlvrd);
  FLib_MemCpy(&LastRISCurrSummDlvrd, &newRISCurrSummDlvrd, sizeof(Summ_t));
  
  FLib_MemCpy(&ProfileIntervalTable[ProfIntrvHead++].Intrv, consumption, sizeof(Consmp));
  
  if(ProfIntrvHead >= gMaxNumberOfPeriodsDelivered_c)
    ProfIntrvHead=0;
  if(ProfIntrvHead == ProfIntrvTail)
  {
    ++ProfIntrvTail;
    if(ProfIntrvTail >= gMaxNumberOfPeriodsDelivered_c)
      ProfIntrvTail=0;
  }
  
/* Start the GetProfile test timer */
#if(gProfIntrvPeriod_c < 5)
  TMR_StartMinuteTimer(gGetProfileTestTimerID, gTimerValue_c, GetProfileTestTimerCallBack);
#else
  TMR_StartSecondTimer(gGetProfileTestTimerID, gTimerValue_c, GetProfileTestTimerCallBack);
#endif

}

/*****************************************************************************/
static Consmp *CalculateConsumptionFrom6ByteArray(uint8_t *pNew, uint8_t *pOld)
{
  uint32_t old32Bit, new32Bit;
  
  // OTA/ZCL Domain - Little Endian 
  FLib_MemCpy(&new32Bit, &pNew[0], sizeof(uint32_t));
  FLib_MemCpy(&old32Bit, &pOld[0], sizeof(uint32_t));
  
  new32Bit=OTA2Native32(new32Bit);
  old32Bit=OTA2Native32(old32Bit);
  
   // Native Domain - MC1322x (Little)  MC1321x,QE,HCS08 (BIG) 
  if(new32Bit >= old32Bit)
  {
    new32Bit = (new32Bit - old32Bit);
  }
  else
  {
    new32Bit = (new32Bit + old32Bit);
  }
  
  new32Bit=Native2OTA32 (new32Bit);  
   // OTA/ZCL Domain - Little Endian 
  FLib_MemCpy(&consumptionValue[0], &new32Bit, sizeof(Consmp));
  
  return &consumptionValue;
 }

/*****************************************************************************/
static void Add8ByteTo6ByteArray(uint8_t value, uint8_t *pSumm_t)
{
  uint32_t LSB2Bytes=0, tmp=0, MSB4Bytes=0;
  
  // OTA/ZCL Domain - Little Endian 
  FLib_MemCpy(&LSB2Bytes, &pSumm_t[0], sizeof(uint16_t));
  FLib_MemCpy(&MSB4Bytes, &pSumm_t[2], sizeof(uint32_t));
  
  LSB2Bytes=OTA2Native32(LSB2Bytes);
  MSB4Bytes=OTA2Native32(MSB4Bytes);
  
  // Native Domain - MC1322x (Little)  MC1321x,QE,HCS08 (BIG)
  LSB2Bytes += value;
  
  tmp = LSB2Bytes >> 16;
  
  MSB4Bytes += tmp;

  // OTA/ZCL Domain - Little Endian 
  pSumm_t[0] = (uint8_t) (LSB2Bytes);
  pSumm_t[1] = (uint8_t) (LSB2Bytes>>8);
  pSumm_t[2] = (uint8_t) (MSB4Bytes);
  pSumm_t[3] = (uint8_t) (MSB4Bytes>>8);
  pSumm_t[4] = (uint8_t) (MSB4Bytes>>16);
  pSumm_t[5] = (uint8_t) (MSB4Bytes>>24);
  
}
/*****************************************************************************/
zbStatus_t ZCL_MsgClusterClient
(
	zbApsdeDataIndication_t *pIndication, /* IN: */
	afDeviceDef_t *pDev                /* IN: */
) 
{
  zclCmd_t Cmd;
  zbStatus_t status = gZclSuccessDefaultRsp_c;

 (void) pDev;

 Cmd = ((zclFrame_t *)pIndication->pAsdu)->command;
 /*Both Display message and Cancel message commands are passed up to the application
   so it can take appropiate action on the display.
 */
 switch(Cmd) {
  case gZclCmdMsg_DisplayMsgReq_c: 
    {
      afAddrInfo_t addrInfo;    
      zclCmdMsg_DisplayMsgReq_t *pReq;
      zclSEGenericRxCmd_t rxCmd;
      /* prepare for response in the address info (back to sender) */
      AF_PrepareForReply(&addrInfo, pIndication);
      FLib_MemCpy(&(MsgResponseSourceAddr.msgAddrInfo), &addrInfo, sizeof(afAddrInfo_t));
      pReq = (zclCmdMsg_DisplayMsgReq_t *) ((pIndication->pAsdu) + sizeof(zclFrame_t)); 
      rxCmd.pSECmd = (void*)pReq;
      if (pIndication->fragmentHdr.pData)
        rxCmd.pRxFrag = &pIndication->fragmentHdr;
      else
        rxCmd.pRxFrag = NULL;
      MsgResponseSourceAddr.msgControl = pReq->MsgCtrl;
      BeeAppUpdateDevice(pIndication->dstEndPoint, gZclUI_MsgDisplayMessageReceived_c, 0, 0, &rxCmd);
    }  
  break;  
 case gZclCmdMsg_CancelMsgReq_c:
  {
    zclCmdMsg_CancelMsgReq_t *pReq = (zclCmdMsg_CancelMsgReq_t *) (pIndication->pAsdu + sizeof(zclFrame_t));  
    BeeAppUpdateDevice(pIndication->dstEndPoint, gZclUI_MsgCancelMessageReceived_c, 0, 0, pReq);
  }  
  break;   
  default:
    status = gZclUnsupportedClusterCommand_c;
  break;  
 }
 return status;
}

/*****************************************************************************/
zbStatus_t ZCL_MsgClusterServer
(
	zbApsdeDataIndication_t *pIndication, /* IN: */
	afDeviceDef_t *pDev                /* IN: */
) 
{
  zclCmd_t Cmd;
  zbStatus_t status = gZclSuccessDefaultRsp_c;

 (void) pDev;

 Cmd = ((zclFrame_t *)pIndication->pAsdu)->command;

 switch(Cmd) {
  case gZclCmdMsg_GetLastMsgReq_c: 
  {
    zclGetLastMsgReq_t *pReq;
    pReq = (zclGetLastMsgReq_t *) ((pIndication->pAsdu) + sizeof(zclFrame_t));  
    BeeAppUpdateDevice(pIndication->dstEndPoint, gZclUI_MsgGetLastMessageReceived_c, 0, 0, pReq);
  }  
  break;  
 case gZclCmdMsg_MsgConfReq_c:
  {
    zclCmdMsg_MsgConfReq_t *pReq = (zclCmdMsg_MsgConfReq_t *) (pIndication->pAsdu + sizeof(zclFrame_t));  
    BeeAppUpdateDevice(pIndication->dstEndPoint, gZclUI_MsgMessageConfirmReceived_c, 0, 0, pReq);
  }  
  break;   
  default:
    status = gZclUnsupportedClusterCommand_c;
  break;  
 }
 return status;
}

/******************************************************************************/
/* Handle Message Inter Pan Server\Client Indications                           */ 
/******************************************************************************/
zbStatus_t ZCL_InterPanMsgClusterClient
(
	zbInterPanDataIndication_t *pIndication, /* IN: */
	afDeviceDef_t *pDev                /* IN: */
) 
{
  zclCmd_t Cmd;
  zbStatus_t status = gZclSuccessDefaultRsp_c;
 (void) pDev;

 Cmd = ((zclFrame_t *)pIndication->pAsdu)->command;
 /*Both Display message and Cancel message commands are passed up to the application
   so it can take appropiate action on the display.
 */
 switch(Cmd) {
  case gZclCmdMsg_DisplayMsgReq_c: 
  {
      InterPanAddrInfo_t addrInfo;    
      zclCmdMsg_DisplayMsgReq_t *pReq;
      zclSEGenericRxCmd_t rxCmd;
      /* prepare for response in the address info (back to sender) */
      PrepareInterPanForReply(&addrInfo, pIndication);
      FLib_MemCpy(&(MsgResponseSourceAddr.msgAddrInfo), &addrInfo, sizeof(InterPanAddrInfo_t));
      pReq = (zclCmdMsg_DisplayMsgReq_t *) ((pIndication->pAsdu) + sizeof(zclFrame_t));  
      rxCmd.pSECmd = (void*)pReq;
      rxCmd.pRxFrag = NULL;
      MsgResponseSourceAddr.msgControl = (pReq->MsgCtrl | 0x02); // set msg conf as beeing InterPan
      BeeAppUpdateDevice(0, gZclUI_MsgDisplayMessageReceived_c, 0, 0, &rxCmd);
  }  
  break;  
 case gZclCmdMsg_CancelMsgReq_c:
  {
    zclCmdMsg_CancelMsgReq_t *pReq = (zclCmdMsg_CancelMsgReq_t *) (pIndication->pAsdu + sizeof(zclFrame_t));  
    BeeAppUpdateDevice(0, gZclUI_MsgCancelMessageReceived_c, 0, 0, pReq);
  }  
  break;   
  default:
    status = gZclUnsupportedClusterCommand_c;
  break;  
 }
 return status;
}

/******************************************************************************/
zbStatus_t ZCL_InterPanMsgClusterServer
(
zbInterPanDataIndication_t *pIndication, /* IN: */
afDeviceDef_t *pDev                /* IN: */
) 
{
  zclCmd_t Cmd;
  zbStatus_t status = gZclSuccessDefaultRsp_c;

 (void) pDev;

 Cmd = ((zclFrame_t *)pIndication->pAsdu)->command;

 switch(Cmd) {
  case gZclCmdMsg_GetLastMsgReq_c: 
  {
    zclInterPanGetLastMsgReq_t *pReq;
    pReq = (zclInterPanGetLastMsgReq_t *) ((pIndication->pAsdu) + sizeof(zclFrame_t));  
    BeeAppUpdateDevice(0, gZclUI_MsgGetLastMessageReceived_c, 0, 0, pReq);
  }  
  break;  
 case gZclCmdMsg_MsgConfReq_c:
  {
    zclCmdMsg_MsgConfReq_t *pReq = (zclCmdMsg_MsgConfReq_t *) (pIndication->pAsdu + sizeof(zclFrame_t));  
    BeeAppUpdateDevice(0, gZclUI_MsgMessageConfirmReceived_c, 0, 0, pReq);
  }  
  break;   
  default:
    status = gZclUnsupportedClusterCommand_c;
  break;  
 }
 return status;
  
}

/*****************************************************************************/
static void MsgDisplayTimerCallBack(tmrTimerID_t tmrID)
{
   BeeAppUpdateDevice(0, gZclUI_MsgUpdateTimeInDisplay_c, 0, 0, NULL); 
   TMR_StartSecondTimer(gMsgDisplayTimerID, 1, MsgDisplayTimerCallBack);
   (void)tmrID; /* Unused parameter. */
}


/*****************************************************************************/
msgAddrInfo_t *ZCL_GetMsgSourceaddrForResponse(void)
{
  return &MsgResponseSourceAddr;
}
/*****************************************************************************/
zbStatus_t ZCL_PrePaymentClusterServer
(
	zbApsdeDataIndication_t *pIndication, /* IN: */
	afDeviceDef_t *pDev                /* IN: */
) 
{
 (void) pIndication;
 (void) pDev;
 return gZclUnsupportedClusterCommand_c;
}

/*****************************************************************************/
/* Free memory used by key estab cluster                                     */
/*****************************************************************************/
void KeyEstabFreeMemory(void)
{
  if(pOppositeImplicitCert)
  {
    MSG_Free(pOppositeImplicitCert);
    pOppositeImplicitCert = NULL;
  }
  if(KeyData)
  {
    MSG_Free(KeyData);
    KeyData = NULL;
  }
  
}

#if gZclKeyEstabDebugTimer_d
void KeyEstabDebugTimerCallBack(tmrTimerID_t tmrID)
{
  zclCmd_t TransactionId;
  
  (void) tmrID;
  TMR_FreeTimer(tmrID);
  TransactionId = ((zclFrame_t *)pKeyEstabDataIndication->pAsdu)->transactionId; 
  
  switch(KeyEstabState)
  {  
  case KeyEstab_KeyEstabInitatedState_c:
    (void) TransactionId;
    ZCL_SendEphemeralDataReq(KeyData->u_ephemeralPublicKey, pKeyEstabDataIndication);
    KeyEstabState = KeyEstab_EphemeralState_c;
    MSG_Free(pKeyEstabDataIndication);
    break;
  case KeyEstab_EphemeralState_c:
#if gCoordinatorCapability_d
    ZCL_SendEphemeralDataRsp(KeyData->v_ephemeralPublicKey, pKeyEstabDataIndication, TransactionId);
    KeyEstabState = KeyEstab_ConfirmDataState_c;
    MSG_Free(pKeyEstabDataIndication);
#else    
    ZCL_SendConfirmKeyDataReq(KeyData->MACU, pKeyEstabDataIndication);
    KeyEstabState = KeyEstab_ConfirmDataState_c;
    MSG_Free(pKeyEstabDataIndication);
#endif    
    break;
  case KeyEstab_ConfirmDataState_c:
#if gCoordinatorCapability_d    
    ZCL_SendConfirmKeyDataRsp(KeyData->MACV, pKeyEstabDataIndication, TransactionId);
    KeyEstabState = KeyEstab_InitState_c;
    MSG_Free(pKeyEstabDataIndication);
    KeyEstabFreeMemory();
#endif
    break;
  default:
    break;    
  }
}
#endif

/*****************************************************************************/
/* Timout timer callback function                                            */
/* If timeout triggered this function will reset state machine and clean up  */
/*****************************************************************************/
void KeyEstabTimeoutCallback(tmrTimerID_t tmrid)
{
  (void) tmrid;
  TMR_FreeTimer(tmrid); 
  if (KeyEstabState != KeyEstab_InitState_c) 
  {    
    KeyEstabState = KeyEstab_InitState_c;
    KeyEstabFreeMemory();
    BeeAppUpdateDevice(0, gZclUI_KeyEstabTimeout_c, 0, 0, NULL);
  }    
}

/*****************************************************************************/
/* Stops and frees timer used for key estab timeout                          */
/*****************************************************************************/
void KeyEstabStopTimeout(void)
{
    TMR_StopTimer(KeyEstabTimerId);
    TMR_FreeTimer(KeyEstabTimerId); 
    KeyEstabTimerId = gTmrInvalidTimerID_c;
}

/*****************************************************************************/
/* Resets timeout timer so timeout period is started again                   */
/*****************************************************************************/
void ResetKeyEstabTimeout(void)
{
 // use the biggest timeout value reported, but minimum 2 seconds.
  uint8_t timeout = FLib_GetMax(EphemeralDataGenerateTime, ConfirmKeyGenerateTime );
  timeout = FLib_GetMax(gKeyEstab_MinimumTimeout_c , timeout );
  
  TMR_StartSingleShotTimer(KeyEstabTimerId, TmrSeconds(timeout), KeyEstabTimeoutCallback);
}

/*****************************************************************************/
/* Initializes (allocates timer and start timeout timer                      */
/*****************************************************************************/
void InitAndStartKeyEstabTimeout(void)
{
  KeyEstabTimerId = TMR_AllocateTimer();
  ResetKeyEstabTimeout();
}
/*****************************************************************************/
/* Allocates memory used during a key establishment session                  */
/*****************************************************************************/
bool_t KeyEstabAllocateMemory(void)
{
  KeyData = MSG_Alloc(sizeof(KeyEstab_KeyData_t));
  if (KeyData)
  {
    BeeUtilZeroMemory(KeyData, sizeof(KeyEstab_KeyData_t));
    pOppositeImplicitCert = MSG_Alloc(sizeof(IdentifyCert_t));
    if (pOppositeImplicitCert)
      return TRUE;
  }
  // memory allocation failed - free memory
  KeyEstabFreeMemory();
  return FALSE;  
}
/*****************************************************************************/
/* Initializes Key estab. state machine and allocates memory needed          */
/*****************************************************************************/
bool_t InitKeyEstabStateMachine(void) {
  if(KeyEstabState == KeyEstab_InitState_c) 
  {
    if( KeyEstabAllocateMemory()) {      
      KeyEstabState = KeyEstab_KeyEstabInitatedState_c;
      return TRUE;
    }    
  }
 return FALSE;  
}
/*****************************************************************************/
/* Sends terminate key estab to server and resets statemachine               */
/*****************************************************************************/
void ZCL_TerminateKeyEstabServer(zbApsdeDataIndication_t *pIndication, uint8_t ErrorCode,bool_t ResetStateMachine)
{  
  ZCL_SendTerminateKeyEstabServerReq(ErrorCode, gKeyEstab_DefaultWaitTime_c,pIndication);
  KeyEstabStopTimeout();
  if (ResetStateMachine) 
  {    
    KeyEstabState = KeyEstab_InitState_c;
    KeyEstabFreeMemory();
    BeeAppUpdateDevice(pIndication->dstEndPoint, gZclUI_KeyEstabFailed_c, 0, 0, pIndication);
  }  
}
/*****************************************************************************/
/* Sends terminate key estab to client and resets statemachine               */
/*****************************************************************************/
void ZCL_TerminateKeyEstabClient(zbApsdeDataIndication_t *pIndication, uint8_t ErrorCode,bool_t ResetStateMachine) 
{

  ZCL_SendTerminateKeyEstabClientReq(ErrorCode, gKeyEstab_DefaultWaitTime_c,pIndication,((zclFrame_t *)pIndication->pAsdu)->transactionId);
  KeyEstabStopTimeout();
  if (ResetStateMachine) 
  {    
    KeyEstabState = KeyEstab_InitState_c;
    KeyEstabFreeMemory();
    BeeAppUpdateDevice(pIndication->dstEndPoint, gZclUI_KeyEstabFailed_c, 0, 0, pIndication);    
  }
}
/*****************************************************************************/
/* calculates key, MACU, MAC for the server/responder side                   */
/*****************************************************************************/
void GenerateKeyMACUMACVResponder(void)
{       // Generate MAC key and MAC data  
        SSP_HashKeyDerivationFunction(KeyData->SharedSecret,gZclCmdKeyEstab_SharedSecretSize_c, KeyData->mackey);
        //MACtag 1, initator Ieee, responer ieee, Initiator Ep pub key, responder ep pub key, shared secret Z, output.
        SSP_HashGenerateSecretTag(2,(uint8_t*) pOppositeImplicitCert->Subject,(uint8_t*) DeviceImplicitCert.Subject, KeyData->u_ephemeralPublicKey,gZclCmdKeyEstab_CompressedPubKeySize_c,KeyData->v_ephemeralPublicKey,gZclCmdKeyEstab_CompressedPubKeySize_c,KeyData->SharedSecret,KeyData->MACU,FALSE, NULL);
        //MACtag 2, responder Ieee, initiator ieee, responder Ep pub key, initiator ep pub key, shared secret Z, output.
        SSP_HashGenerateSecretTag(3,(uint8_t*) DeviceImplicitCert.Subject, (uint8_t*)pOppositeImplicitCert->Subject, KeyData->v_ephemeralPublicKey,gZclCmdKeyEstab_CompressedPubKeySize_c,KeyData->u_ephemeralPublicKey,gZclCmdKeyEstab_CompressedPubKeySize_c,KeyData->SharedSecret,KeyData->MACV,FALSE, NULL);          
        //MACtag 1, initator Ieee, responer ieee, Initiator Ep pub key, responder ep pub key, shared secret Z, output.

}
/*****************************************************************************/
/* calculates key, MACU, MAC for the client/initiator side                   */
/*****************************************************************************/
void GenerateKeyMACUMACVInitiator(void)
{
// Generate MAC key and MAC data  
        SSP_HashKeyDerivationFunction(KeyData->SharedSecret, gZclCmdKeyEstab_SharedSecretSize_c, KeyData->mackey);
        //MACtag 1, initator Ieee, responer ieee, Initiator Ep pub key, responder ep pub key, shared secret Z, output.
        SSP_HashGenerateSecretTag(2,(uint8_t*) DeviceImplicitCert.Subject,(uint8_t*) pOppositeImplicitCert->Subject, KeyData->u_ephemeralPublicKey,gZclCmdKeyEstab_CompressedPubKeySize_c,KeyData->v_ephemeralPublicKey,gZclCmdKeyEstab_CompressedPubKeySize_c,KeyData->SharedSecret,KeyData->MACU, FALSE, NULL);
        //MACtag 2, responder Ieee, initiator ieee, responder Ep pub key, initiator ep pub key, shared secret Z, output.
        SSP_HashGenerateSecretTag(3,(uint8_t*) pOppositeImplicitCert->Subject,(uint8_t*) DeviceImplicitCert.Subject,KeyData->v_ephemeralPublicKey,gZclCmdKeyEstab_CompressedPubKeySize_c,KeyData->u_ephemeralPublicKey,gZclCmdKeyEstab_CompressedPubKeySize_c,KeyData->SharedSecret,KeyData->MACV, FALSE, NULL);                
}
/*****************************************************************************/
/* Check if the sender and the length of the InitKeyEstabReq is correct      */
/*****************************************************************************/
bool_t ValidateKeyEstabMessage(zbApsdeDataIndication_t *pIndication, IdentifyCert_t *pCert) 
{
zbNwkAddr_t aSrcAddr; 
zbIeeeAddr_t aCertIeee;
#if gComboDeviceCapability_d || gCoordinatorCapability_d
zbNwkAddr_t aCertNwk;
#endif
uint8_t asduLen=pIndication->asduLength; 
FLib_MemCpyReverseOrder(aCertIeee,pCert->Subject,sizeof(zbIeeeAddr_t));
Copy2Bytes(aSrcAddr,pIndication->aSrcAddr);
#if gComboDeviceCapability_d 
  if (((NlmeGetRequest(gDevType_c) != gCoordinator_c) && (Cmp2BytesToZero(aSrcAddr) && IsEqual8Bytes(aCertIeee, ApsmeGetRequest(gApsTrustCenterAddress_c))))||((NlmeGetRequest(gDevType_c) == gCoordinator_c) && IsEqual2Bytes(APS_GetNwkAddress(aCertIeee,aCertNwk),aSrcAddr)))
#elif gCoordinatorCapability_d
  if(IsEqual2Bytes(APS_GetNwkAddress(aCertIeee,aCertNwk),aSrcAddr))
#else
  if(Cmp2BytesToZero(aSrcAddr) && IsEqual8Bytes(aCertIeee, ApsmeGetRequest(gApsTrustCenterAddress_c)))
#endif 
  {
#if gZclAcceptLongCert_d
    if((asduLen>=sizeof(ZclCmdKeyEstab_InitKeyEstabReq_t)+sizeof(zclFrame_t)))
#else
    if((asduLen==sizeof(ZclCmdKeyEstab_InitKeyEstabReq_t)+sizeof(zclFrame_t)))
#endif
      return TRUE;
  } 
  return FALSE; 
} 

/*****************************************************************************/
/* Key Establishment Cluster server, only one session at a time is supported */
/*****************************************************************************/
zbStatus_t ZCL_KeyEstabClusterServer
(
zbApsdeDataIndication_t *pIndication, /* IN: */
afDeviceDef_t *pDev                /* IN: */
) 
{
  zclCmd_t Cmd;
  zclCmd_t TransactionId;
  zbStatus_t status = gZbSuccess_c;
  (void) pDev;
  
  
  Cmd = ((zclFrame_t *)pIndication->pAsdu)->command;
  TransactionId = ((zclFrame_t *)pIndication->pAsdu)->transactionId;
  switch(Cmd)
  {   
  case gZclCmdKeyEstab_InitKeyEstabReq_c:
    {
      ZclCmdKeyEstab_InitKeyEstabReq_t *pReq;
      IdentifyCert_t *pCert;
      pReq = (ZclCmdKeyEstab_InitKeyEstabReq_t *) ((pIndication->pAsdu) + sizeof(zclFrame_t));         
      pCert = (IdentifyCert_t *) pReq->IdentityIDU;
      if (KeyEstabState == KeyEstab_InitState_c)
      {
        // check whether request orginate from coordiantor, if device is not a coordinator.
        if (((NlmeGetRequest(gDevType_c) != gCoordinator_c) && (Cmp2BytesToZero(pIndication->aSrcAddr))) ||
            (NlmeGetRequest(gDevType_c) == gCoordinator_c))
        {
          // certificate validation function
          if(ValidateKeyEstabMessage(pIndication,pCert))
          {
            // validate whether the securitysuite is the correct type
            if(IsEqual2BytesInt(pReq->SecuritySuite, gKeyEstabSuite_CBKE_ECMQV_c))
            {
              // validate whether issuer ID is correct:
              if(FLib_MemCmp(pCert->Issuer, (void *)CertAuthIssuerID, sizeof(CertAuthIssuerID)))
              { 
                if (KeyEstabAllocateMemory()) {
                  FLib_MemCpy((uint8_t*) pOppositeImplicitCert,(void *)pCert, sizeof(IdentifyCert_t));
                  // Issuer ID is correct, send response back.
                  EphemeralDataGenerateTime = pReq->EphemeralDataGenerateTime;
                  ConfirmKeyGenerateTime    = pReq->ConfirmKeyGenerateTime;
                  InitAndStartKeyEstabTimeout();
#if !gZclKeyEstabDebugInhibitInitRsp_d                  
                  (void) ZCL_SendInitiatKeyEstRsp(&DeviceImplicitCert, pIndication,TransactionId);
#endif                  
                  KeyEstabState = KeyEstab_EphemeralState_c;
                }
                else
                { // Memory coult not be allocated.
                  ZCL_TerminateKeyEstabClient(pIndication, gZclCmdKeyEstab_TermNoResources_c, TRUE);
                }
              }
              else
              { // Issuer ID was not the correct, send terminate message
                ZCL_TerminateKeyEstabClient(pIndication, gZclCmdKeyEstab_TermUnknownIssuer_c,TRUE);
              }          
            }
            else
            {
              //only CBKE-ECMQV supported
              ZCL_TerminateKeyEstabClient(pIndication, gZclCmdKeyEstab_TermUnsupportedSuite_c,FALSE);          
            }
          }
          else
          {
            //invalid InitKeyEstabReq/certificate
            ZCL_TerminateKeyEstabClient(pIndication, gZclCmdKeyEstab_TermBadMessage_c, TRUE);
          }
        }
        else
        {
          // discard, message when initate message is not from coordinator. 
          ZCL_TerminateKeyEstabClient(pIndication, gZclCmdKeyEstab_TermBadMessage_c,TRUE);    
        }               
      }
      else
      {
        //Device is in the middle of key establishment with another device, send terminate message with status: no resources.
        ZCL_SendTerminateKeyEstabClientReq(gZclCmdKeyEstab_TermNoResources_c, gKeyEstab_DefaultWaitTime_c,pIndication,((zclFrame_t *)pIndication->pAsdu)->transactionId);
      }
    }
    break;      
  case gZclCmdKeyEstab_EphemeralDataReq_c:
    {
      
      if (KeyEstabState == KeyEstab_EphemeralState_c)
      {
        uint8_t aNwkAddrLocalCpy[2];
        zbIeeeAddr_t clientIeee;
        FLib_MemCpyReverseOrder(clientIeee,pOppositeImplicitCert->Subject,sizeof(zbIeeeAddr_t));
        //check if the originator of the ephemeral data request is the originator of the init key estab req
        if(IsEqual2Bytes(APS_GetNwkAddress(clientIeee,aNwkAddrLocalCpy),pIndication->aSrcAddr))
        {
          ZclCmdKeyEstab_EphemeralDataReq_t *pReq;
      
          pReq = (ZclCmdKeyEstab_EphemeralDataReq_t *) ((pIndication->pAsdu) + sizeof(zclFrame_t));         
          FLib_MemCpy(KeyData->u_ephemeralPublicKey,pReq->EphemeralDataQEU, gZclCmdKeyEstab_CompressedPubKeySize_c);
          if (ZSE_ECCGenerateKey(KeyData->ephemeralPrivateKey, KeyData->v_ephemeralPublicKey, ECC_GetRandomDataFunc, NULL, 0) == 0x00) // 0x00 = MCE_SUCCESS
          {
            ResetKeyEstabTimeout();
          }
          // Send response:
          // Generate keybits
          if(ZSE_ECCKeyBitGenerate(DevicePrivateKey, KeyData->ephemeralPrivateKey, KeyData->v_ephemeralPublicKey,
                                   (uint8_t *)pOppositeImplicitCert, KeyData->u_ephemeralPublicKey, (uint8_t *)CertAuthPubKey,KeyData->SharedSecret, ECC_HashFunc, NULL, 0)==0x00)
          {
            GenerateKeyMACUMACVResponder();
#if gZclKeyEstabDebugTimer_d
            KeyEstabDebugTimerId = TMR_AllocateTimer();
            pKeyEstabDataIndication = MSG_Alloc(sizeof(zbApsdeDataIndication_t));
            FLib_MemCpy(pKeyEstabDataIndication, pIndication, sizeof(zbApsdeDataIndication_t));
            TMR_StartTimer(KeyEstabDebugTimerId, gTmrSingleShotTimer_c, gZclKeyEstabDebugTimeInterval_d, KeyEstabDebugTimerCallBack);   
#else

            KeyEstabState = KeyEstab_ConfirmDataState_c;
            ZCL_SendEphemeralDataRsp(KeyData->v_ephemeralPublicKey, pIndication,TransactionId);
#endif            
          }
          else
          {
            ZCL_TerminateKeyEstabClient(pIndication, gZclCmdKeyEstab_TermBadMessage_c, TRUE);
          }
        }
        else
        {
          ZCL_SendTerminateKeyEstabClientReq(gZclCmdKeyEstab_TermBadMessage_c, gKeyEstab_DefaultWaitTime_c,pIndication,((zclFrame_t *)pIndication->pAsdu)->transactionId);
        }
      }
      else
      { // command is out of sequence... send terminate message
        ZCL_SendTerminateKeyEstabClientReq(gZclCmdKeyEstab_TermBadMessage_c, gKeyEstab_DefaultWaitTime_c,pIndication,((zclFrame_t *)pIndication->pAsdu)->transactionId);
      }
    }
    break;      
  case gZclCmdKeyEstab_ConfirmKeyDataReq_c:
    if (KeyEstabState == KeyEstab_ConfirmDataState_c)
    {
      uint8_t aNwkAddrLocalCpy[2];
      zbIeeeAddr_t clientIeee;
      FLib_MemCpyReverseOrder(clientIeee,pOppositeImplicitCert->Subject,sizeof(zbIeeeAddr_t));
      //check if the originator of the confirm key data request is the originator of the init key estab req
      if(IsEqual2Bytes(APS_GetNwkAddress(clientIeee,aNwkAddrLocalCpy),pIndication->aSrcAddr))
      {
        ZclCmdKeyEstab_ConfirmKeyDataReq_t *pReq;
        pReq = (ZclCmdKeyEstab_ConfirmKeyDataReq_t *) ((pIndication->pAsdu) + sizeof(zclFrame_t));         
        
        if (FLib_MemCmp(pReq->SecureMsgAuthCodeMACU, KeyData->MACU, gZclCmdKeyEstab_AesMMOHashSize_c))
        {
          uint8_t keytype = gApplicationLinkKey_c;
          zbIeeeAddr_t addr;
          KeyEstabStopTimeout();
#if gZclKeyEstabDebugTimer_d
          KeyEstabDebugTimerId = TMR_AllocateTimer();
          pKeyEstabDataIndication = MSG_Alloc(sizeof(zbApsdeDataIndication_t));
          FLib_MemCpy(pKeyEstabDataIndication, pIndication, sizeof(zbApsdeDataIndication_t));
          TMR_StartTimer(KeyEstabDebugTimerId, gTmrSingleShotTimer_c, gZclKeyEstabDebugTimeInterval_d, KeyEstabDebugTimerCallBack);   
#else          
          ZCL_SendConfirmKeyDataRsp(KeyData->MACV,pIndication,TransactionId);
          KeyEstabState = KeyEstab_InitState_c;
#endif          
          // call BeeAppDeviceJoined to tell that device has been registered.
          // install new key:
          // Ieee address in certificate is Big endian, convert to little endian before adding it.
          Copy8Bytes(addr,pOppositeImplicitCert->Subject);
          Swap8Bytes(addr);                
          if (IsEqual8Bytes(addr, ApsmeGetRequest(gApsTrustCenterAddress_c)))          
          {
            keytype = gTrustCenterLinkKey_c;
          }        
          APS_RegisterLinkKeyData(addr,keytype,(uint8_t *) KeyData->mackey);        
          BeeAppUpdateDevice(pIndication->dstEndPoint, gZclUI_KeyEstabSuccesful_c, 0, 0, addr);
#if !gZclKeyEstabDebugTimer_d          
          KeyEstabFreeMemory();
#endif          
        }
        else
        {
          // call BeeAppDeviceJoined to tell that a MACU failed to validate.
          ZCL_TerminateKeyEstabClient(pIndication, gZclCmdKeyEstab_TermBadKeyConfirm_c, TRUE);
        }
      }
      else
      {
        ZCL_SendTerminateKeyEstabClientReq(gZclCmdKeyEstab_TermBadMessage_c, gKeyEstab_DefaultWaitTime_c,pIndication,((zclFrame_t *)pIndication->pAsdu)->transactionId);
      }
    }
    else 
    {  // command is out of sequence... send terminate message
      ZCL_SendTerminateKeyEstabClientReq(gZclCmdKeyEstab_TermBadMessage_c, gKeyEstab_DefaultWaitTime_c,pIndication,((zclFrame_t *)pIndication->pAsdu)->transactionId);
    }
    break;      
  case gZclCmdKeyEstab_TerminateKeyEstabServer_c:
    {
      uint8_t aNwkAddrLocalCpy[2];
      zbIeeeAddr_t clientIeee;
      FLib_MemCpyReverseOrder(clientIeee,pOppositeImplicitCert->Subject,sizeof(zbIeeeAddr_t));
      //accept terminate key estab server only from the device involved in key estab
      if(IsEqual2Bytes(APS_GetNwkAddress(clientIeee,aNwkAddrLocalCpy),pIndication->aSrcAddr))
      {
        // call BeeAppDeviceJoined to tell that device failed to register.
        KeyEstabState = KeyEstab_InitState_c;
        KeyEstabFreeMemory();
        BeeAppUpdateDevice(pIndication->dstEndPoint, gZclUI_KeyEstabFailed_c, 0, 0, pIndication);    
      }
    }
    break;          
  default:
    status = gZclUnsupportedClusterCommand_c;
    break;
  }
  return status;
}

zbStatus_t ZCL_KeyEstabClusterClient
(
	zbApsdeDataIndication_t *pIndication, /* IN: */
	afDeviceDef_t *pDev                /* IN: */
) 
{
  zclCmd_t Cmd;
  zbStatus_t status = gZbSuccess_c;

  (void) pDev; 
  Cmd = ((zclFrame_t *)pIndication->pAsdu)->command;

  switch(Cmd)
  {   
  case gZclCmdKeyEstab_InitKeyEstabRsp_c:
    {
      ZclCmdKeyEstab_InitKeyEstabRsp_t *pReq;
      IdentifyCert_t *pCert;
      pReq = (ZclCmdKeyEstab_InitKeyEstabRsp_t *) ((pIndication->pAsdu) + sizeof(zclFrame_t));         
      pCert = (IdentifyCert_t *) pReq->IdentityIDV;
      if(KeyEstabState == KeyEstab_KeyEstabInitatedState_c)
      {
        if (ValidateKeyEstabMessage(pIndication,pCert))
        {
          // validate whether issuer ID is correct:
          if(FLib_MemCmp(pCert->Issuer, (void *)CertAuthIssuerID, sizeof(CertAuthIssuerID)) == TRUE)         
          {           
            FLib_MemCpy((void *)pOppositeImplicitCert,(void *)pCert, sizeof(IdentifyCert_t));          
            EphemeralDataGenerateTime = pReq->EphemeralDataGenerateTime;
            ConfirmKeyGenerateTime    = pReq->ConfirmKeyGenerateTime;
            // Issuer ID is correct, send ephemerial Data request back.
            if(ZSE_ECCGenerateKey(KeyData->ephemeralPrivateKey, KeyData->u_ephemeralPublicKey, ECC_GetRandomDataFunc, NULL, 0) == 0x00)//0x00 = MCE_SUCCESS
            {
              ResetKeyEstabTimeout();
              
#if gZclKeyEstabDebugTimer_d
              KeyEstabDebugTimerId = TMR_AllocateTimer();
              pKeyEstabDataIndication = MSG_Alloc(sizeof(zbApsdeDataIndication_t));
              FLib_MemCpy(pKeyEstabDataIndication, pIndication, sizeof(zbApsdeDataIndication_t));
              TMR_StartTimer(KeyEstabDebugTimerId, gTmrSingleShotTimer_c, gZclKeyEstabDebugTimeInterval_d, KeyEstabDebugTimerCallBack);   
#else
              ZCL_SendEphemeralDataReq(KeyData->u_ephemeralPublicKey,pIndication);
              KeyEstabState = KeyEstab_EphemeralState_c;
#endif              
            }                            
          }
          else
          {
            // send terminate key estab. with UNKNOWN_ISSUER status           
            ZCL_TerminateKeyEstabServer(pIndication, gZclCmdKeyEstab_TermUnknownIssuer_c, TRUE);                 
          }                  
        }
        else
        {
          // cluster is not participating in key establishment
          // send terminate key estab. with BAD_MESSAGE status
          ZCL_TerminateKeyEstabServer(pIndication, gZclCmdKeyEstab_TermBadMessage_c, TRUE);                 
        }
      }
      else
      {
        // command is out of sequence... send terminate message
        ZCL_SendTerminateKeyEstabServerReq(gZclCmdKeyEstab_TermBadMessage_c, gKeyEstab_DefaultWaitTime_c,pIndication);
      }
    }
    break;      
  case gZclCmdKeyEstab_EphemeralDataRsp_c:
    {
      if (KeyEstabState == KeyEstab_EphemeralState_c)
      {
        uint8_t aNwkAddrLocalCpy[2];
        zbIeeeAddr_t serverIeee;
        FLib_MemCpyReverseOrder(serverIeee,pOppositeImplicitCert->Subject,sizeof(zbIeeeAddr_t));
        //check that the originator of the ephemeral data response is correct
        if(IsEqual2Bytes(APS_GetNwkAddress(serverIeee,aNwkAddrLocalCpy),pIndication->aSrcAddr))
        {
          ZclCmdKeyEstab_EphemeralDataRsp_t *pReq;
          pReq = (ZclCmdKeyEstab_EphemeralDataRsp_t *) ((pIndication->pAsdu) + sizeof(zclFrame_t));
          FLib_MemCpy(KeyData->v_ephemeralPublicKey,pReq->EphemeralDataQEV, gZclCmdKeyEstab_CompressedPubKeySize_c);
          // Generate keybits 
          if (ZSE_ECCKeyBitGenerate(DevicePrivateKey, KeyData->ephemeralPrivateKey, KeyData->u_ephemeralPublicKey,
                                    (uint8_t *)pOppositeImplicitCert, KeyData->v_ephemeralPublicKey, (uint8_t *)CertAuthPubKey,KeyData->SharedSecret, ECC_HashFunc, NULL, 0)==0x00)
          {
            // Generate MAC key and MAC data  
            GenerateKeyMACUMACVInitiator();        
            ResetKeyEstabTimeout();
            
#if gZclKeyEstabDebugTimer_d
            KeyEstabDebugTimerId = TMR_AllocateTimer();
            pKeyEstabDataIndication = MSG_Alloc(sizeof(zbApsdeDataIndication_t));
            FLib_MemCpy(pKeyEstabDataIndication, pIndication, sizeof(zbApsdeDataIndication_t));
            TMR_StartTimer(KeyEstabDebugTimerId, gTmrSingleShotTimer_c, gZclKeyEstabDebugTimeInterval_d, KeyEstabDebugTimerCallBack);   
#else
            ZCL_SendConfirmKeyDataReq(KeyData->MACU,pIndication);        
            KeyEstabState = KeyEstab_ConfirmDataState_c;
#endif            
          }
          else
          {
            ZCL_TerminateKeyEstabServer(pIndication, gZclCmdKeyEstab_TermBadMessage_c, TRUE);
          }
        }
        else
        {
          //Originator of the ephemeral data response is different than the originator of the init key estab rsp
          ZCL_SendTerminateKeyEstabServerReq(gZclCmdKeyEstab_TermBadMessage_c, gKeyEstab_DefaultWaitTime_c,pIndication);
        }
      }
      else
      {
        // command is out of sequence... send terminate message
        ZCL_TerminateKeyEstabServer(pIndication, gZclCmdKeyEstab_TermBadMessage_c, TRUE);
      }
    }
    break;      
  case gZclCmdKeyEstab_ConfirmKeyDataRsp_c:
    {
      ZclCmdKeyEstab_ConfirmKeyDataRsp_t *pReq;
      pReq = (ZclCmdKeyEstab_ConfirmKeyDataRsp_t *) ((pIndication->pAsdu) + sizeof(zclFrame_t));
      if (KeyEstabState == KeyEstab_ConfirmDataState_c)
      {
        uint8_t aNwkAddrLocalCpy[2];
        zbIeeeAddr_t serverIeee;
        FLib_MemCpyReverseOrder(serverIeee,pOppositeImplicitCert->Subject,sizeof(zbIeeeAddr_t));
        //check that the originator of the confirm key data response is correct
        if(IsEqual2Bytes(APS_GetNwkAddress(serverIeee,aNwkAddrLocalCpy),pIndication->aSrcAddr))
        {
          if (FLib_MemCmp(pReq->SecureMsgAuthCodeMACV, KeyData->MACV,gZclCmdKeyEstab_AesMMOHashSize_c))
          {
            // Call BeeAppDeviceUpdate and tell that device has been registered successfully
            // install key:
            // Ieee address in certificate is Big endian, convert to little endian before adding it.
            uint8_t keytype = gApplicationLinkKey_c;
            zbIeeeAddr_t addr;
            KeyEstabStopTimeout();
            Copy8Bytes(addr,pOppositeImplicitCert->Subject);
            Swap8Bytes(addr);
            if (IsEqual8Bytes(addr, ApsmeGetRequest(gApsTrustCenterAddress_c)))          
            {
              keytype = gTrustCenterLinkKey_c;
            }        
            APS_RegisterLinkKeyData(addr,keytype,(uint8_t *) KeyData->mackey);
            BeeAppUpdateDevice(pIndication->dstEndPoint, gZclUI_KeyEstabSuccesful_c, 0, 0, addr);          
            KeyEstabFreeMemory();
            KeyEstabState = KeyEstab_InitState_c;
          }
          else
          {
            ZCL_TerminateKeyEstabServer(pIndication, gZclCmdKeyEstab_TermBadKeyConfirm_c, TRUE);
          }
        }
        else
        {
          //Originator of the confirm key data response different than the originator of the init key estab rsp
          ZCL_SendTerminateKeyEstabServerReq(gZclCmdKeyEstab_TermBadMessage_c, gKeyEstab_DefaultWaitTime_c,pIndication);
        }
      }
      else
      {
        // command is out of sequence... send terminate message
        ZCL_SendTerminateKeyEstabServerReq(gZclCmdKeyEstab_TermBadMessage_c, gKeyEstab_DefaultWaitTime_c,pIndication);
      }
    }
    break;      
  case gZclCmdKeyEstab_TerminateKeyEstabClient_c:
    {
      uint8_t aNwkAddrLocalCpy[2];
      zbIeeeAddr_t serverIeee;
      FLib_MemCpyReverseOrder(serverIeee,pOppositeImplicitCert->Subject,sizeof(zbIeeeAddr_t));
      //accept terminate key estab client only in KeyEstab_KeyEstabInitatedState_c or from the device involved in key estab.
      if((KeyEstabState == KeyEstab_KeyEstabInitatedState_c) || (IsEqual2Bytes(APS_GetNwkAddress(serverIeee,aNwkAddrLocalCpy),pIndication->aSrcAddr)))
      {
        // call BeeAppDeviceUpdate and tell that device failed to register.
        KeyEstabState = KeyEstab_InitState_c;
        KeyEstabFreeMemory();
        BeeAppUpdateDevice(pIndication->dstEndPoint, gZclUI_KeyEstabFailed_c, 0, 0, pIndication);
      }
    }
    break;          
  default:
    status = gZclUnsupportedClusterCommand_c;
    break;
  }
 return status;
}

/*****************************************************************************/
/*-------- Message cluster commands ---------*/
/*Message cluster client commands */
zbStatus_t ZclMsg_DisplayMsgReq
(
zclDisplayMsgReq_t *pReq
)
{
  uint8_t Length;
  /*1 is the length byte*/
  Length = (pReq->cmdFrame.length) + (sizeof(zclCmdMsg_DisplayMsgReq_t) - sizeof(uint8_t));	
  
  return ZCL_SendServerReqSeqPassed(gZclCmdMsg_DisplayMsgReq_c, Length,(zclGenericReq_t *)pReq);	
}

zbStatus_t ZclMsg_CancelMsgReq
(
zclCancelMsgReq_t *pReq
)
{
  return ZCL_SendServerReqSeqPassed(gZclCmdMsg_CancelMsgReq_c, sizeof(zclCmdMsg_CancelMsgReq_t),(zclGenericReq_t *)pReq);
}

/*Message cluster Server commands */
zbStatus_t ZclMsg_GetLastMsgReq
(
zclGetLastMsgReq_t *pReq
)
{
  return ZCL_SendClientReqSeqPassed(gZclCmdMsg_GetLastMsgReq_c, 0 , (zclGenericReq_t *)pReq);
}


zbStatus_t ZclMsg_MsgConf
(
zclMsgConfReq_t *pReq
)
{
  return ZCL_SendClientRspSeqPassed(gZclCmdMsg_CancelMsgReq_c, sizeof(zclCmdMsg_MsgConfReq_t),(zclGenericReq_t *)pReq);
}



#if gInterPanCommunicationEnabled_c 
zbStatus_t ZclMsg_InterPanDisplayMsgReq
(
zclInterPanDisplayMsgReq_t *pReq
)
{
  uint8_t Length;
  /*1 is the length byte*/
  Length = (pReq->cmdFrame.length) + (sizeof(zclCmdMsg_DisplayMsgReq_t) - sizeof(uint8_t));	
  
  return ZCL_SendInterPanServerReqSeqPassed(gZclCmdMsg_DisplayMsgReq_c, Length,pReq);	
}

zbStatus_t ZclMsg_InterPanCancelMsgReq
(
zclInterPanCancelMsgReq_t *pReq
)
{ 
  return ZCL_SendInterPanServerReqSeqPassed(gZclCmdMsg_CancelMsgReq_c, sizeof(zclCmdMsg_CancelMsgReq_t),pReq);
}

/*Message cluster Server commands */
zbStatus_t ZclMsg_InterPanGetLastMsgReq
(
zclInterPanGetLastMsgReq_t *pReq
)
{
  return ZCL_SendInterPanClientReqSeqPassed(gZclCmdMsg_GetLastMsgReq_c, 0 , pReq);
}


zbStatus_t ZclMsg_InterPanMsgConf
(
zclInterPanMsgConfReq_t *pReq
)
{
  return ZCL_SendInterPanClientRspSeqPassed(gZclCmdMsg_CancelMsgReq_c, sizeof(zclCmdMsg_MsgConfReq_t),pReq);
}
#endif /*#if gInterPanCommunicationEnabled_c */

/*-------- Simple Metering cluster commands ---------*/
/*Simple Metering cluster client commands */
zbStatus_t ZclSmplMet_GetProfReq
(
zclSmplMet_GetProfReq_t *pReq
)
{
  return ZCL_SendClientReqSeqPassed(gZclCmdSmplMet_GetProfReq_c, sizeof(zclCmdSmplMet_GetProfReq_t),(zclGenericReq_t *)pReq);	
}
#if gZclFastPollMode_d
zbStatus_t ZclSmplMet_ReqFastPollModeReq
(
zclSmplMet_ReqFastPollModeReq_t *pReq
)
{
  return ZCL_SendClientReqSeqPassed(gZclCmdSmplMet_ReqFastPollModeReq_c, sizeof(zclCmdSmplMet_ReqFastPollModeReq_t),(zclGenericReq_t *)pReq);	
}
#endif

/*Simple Metering cluster server commands */
void ZCL_BuildGetProfResponse
(   uint8_t IntrvChannel,                 /* IN: */
    ZCLTime_t EndTime,                    /* IN:  note little endian*/
    uint8_t NumberOfPeriods,              /* IN: */
    zclCmdSmplMet_GetProfRsp_t *pProfRsp  /* OUT: */
)
{
  /* Search for entries that match end time in Profile intervals table */ 
  uint8_t *pTmpIntrvs,i,NumOfPeriodsDlvrd=0, SearchPointer;  
  uint32_t endTime;
  bool_t EntryInTable=FALSE, endTimeValid=FALSE, IntrvChannelSupported=FALSE;
  uint8_t status = gSMGetProfRsp_SuccessStatus_c;
  pTmpIntrvs = (uint8_t *)pProfRsp->Intrvs;
  endTime = OTA2Native32(EndTime);  
  if(ProfIntrvHead == 0)
      SearchPointer = gMaxNumberOfPeriodsDelivered_c;
  else
    SearchPointer = ProfIntrvHead-1;
  
  if(ProfIntrvTail > ProfIntrvHead)
    i = ProfIntrvHead + (gMaxNumberOfPeriodsDelivered_c-ProfIntrvTail)+1;
  else
    i = (ProfIntrvHead - ProfIntrvTail);
  
  if(i)
    EntryInTable = TRUE;
  
  for(; i>0 && NumOfPeriodsDlvrd < NumberOfPeriods; --i)
  {
    /* Search the profile table for entries */
    if(ProfileIntervalTable[SearchPointer].IntrvChannel == IntrvChannel)
    {
      IntrvChannelSupported=TRUE;
      /* Check if endTime is valid */
      if(ProfileIntervalTable[SearchPointer].endTime <= endTime || endTime == 0x00000000)
      {
        endTimeValid=TRUE;
        if(NumOfPeriodsDlvrd==0)
          FLib_MemCpy(&pProfRsp->EndTime, &ProfileIntervalTable[SearchPointer].endTime, sizeof(uint32_t));
        NumOfPeriodsDlvrd++;
        FLib_MemCpy(pTmpIntrvs, &ProfileIntervalTable[SearchPointer].Intrv, sizeof(Consmp));
        pTmpIntrvs += sizeof(IntrvForProfiling_t);
      }
      else
      {
        /* endTime is invalid, check next entry */
        endTimeValid=FALSE;
      }
      
    }
 //   else
 //   {
 //     IntrvChannelSupported=FALSE;
 //   }
    if(SearchPointer==0)
    {
      SearchPointer = gMaxNumberOfPeriodsDelivered_c;
    }
    --SearchPointer;
    
  }
 
  pProfRsp->ProfIntrvPeriod = gProfIntrvPeriod_c;
  pProfRsp->NumOfPeriodsDlvrd = NumOfPeriodsDlvrd;
  /* check if status field value is undefined  interval channel requested */
  if((IntrvChannel != gIntrvChannel_ConsmpDlvrd_c) && (IntrvChannel != gIntrvChannel_ConsmpRcvd_c))
     status = gSMGetProfRsp_UndefIntrvChannelStatus_c;
  /* check if status field value is interval channel not supported */
  else if(EntryInTable == TRUE && IntrvChannelSupported == FALSE)
    status = gSMGetProfRsp_IntrvChannelNotSupportedStatus_c;
   /* check if status field value is invalid end time */
  /*else if() 
    status = gSMGetProfRsp_InvalidEndTimeStatus_c;*/  
  /* check if status field value is no intervals available for the requested time */
  else if((EntryInTable == TRUE && IntrvChannelSupported==TRUE && endTimeValid==FALSE)|| 
          (EntryInTable == FALSE))
    status = gSMGetProfRsp_NoIntrvsAvailableStatus_c;
  /* check if status field value is more periods requested than can be returned */
  else if(NumberOfPeriods > NumOfPeriodsDlvrd)
    status = gSMGetProfRsp_MorePeriodsRequestedStatus_c;
  
  pProfRsp->Status=status;

}

/*Simple Metering cluster Server commands */
zbStatus_t ZclSmplMet_GetProfRsp
(
zclSmplMet_GetProfRsp_t *pReq
)
{
  uint8_t length;
  length = MbrOfs(zclCmdSmplMet_GetProfRsp_t,Intrvs) + 
    (pReq->cmdFrame.NumOfPeriodsDlvrd * sizeof(pReq->cmdFrame.Intrvs[0]));
  return ZCL_SendServerRspSeqPassed(gZclCmdSmplMet_GetProfRsp_c, length,(zclGenericReq_t *)pReq);
}

#if gZclFastPollMode_d
zbStatus_t ZclSmplMet_ReqFastPollModeRsp
(
zclSmplMet_ReqFastPollModeRsp_t *pReq
)
{
  return ZCL_SendServerRspSeqPassed(gZclCmdSmplMet_ReqFastPollModeRsp_c, sizeof(zclCmdSmplMet_ReqFastPollModeRsp_t),(zclGenericReq_t *)pReq);
}
#endif

/*-------- DRLC cluster commands ---------*/
/*DRLC cluster client commands */
zbStatus_t ZclDmndRspLdCtrl_ReportEvtStatus
(
zclDmndRspLdCtrl_ReportEvtStatus_t *pReq
)
{
  uint8_t length;
#if gASL_ZclDmndRspLdCtrl_ReportEvtStatus_d
  uint8_t entryIndex;
  
  if (pReq->cmdFrame.EvtStatus == gSELCDR_LdCtrlEvtCode_UserHaveToChooseOptOut_c ||
      pReq->cmdFrame.EvtStatus == gSELCDR_LdCtrlEvtCode_UserHaveToChooseOptIn_c)
  {
    entryIndex = FindEventInEventsTable((uint8_t *)&(pReq->cmdFrame.IssuerEvtID));
    if(entryIndex != 0xff)
      gaEventsTable[entryIndex].CurrentStatus = pReq->cmdFrame.EvtStatus;
  }
#endif  
  length = sizeof(zclCmdDmndRspLdCtrl_ReportEvtStatus_t);
  return ZCL_SendClientRspSeqPassed(gZclCmdDmndRspLdCtrl_ReportEvtStatus_c, length ,(zclGenericReq_t *)pReq); 
  
}

/*DRLC cluster Server commands */
zbStatus_t ZclDmndRspLdCtrl_LdCtrlEvtReq
(
zclDmndRspLdCtrl_LdCtrlEvtReq_t *pReq
)
{
  
  return ZCL_SendServerReqSeqPassed(gZclCmdDmndRspLdCtrl_LdCtrlEvtReq_c, sizeof(zclCmdDmndRspLdCtrl_LdCtrlEvtReq_t),(zclGenericReq_t *)pReq);	
}

zbStatus_t ZclDmndRspLdCtrl_CancelLdCtrlEvtReq
(
zclDmndRspLdCtrl_CancelLdCtrlEvtReq_t *pReq
)
{ 
  return ZCL_SendServerReqSeqPassed(gZclCmdDmndRspLdCtrl_CancelLdCtrlEvtReq_c, sizeof(zclCmdDmndRspLdCtrl_CancelLdCtrlEvtReq_t),(zclGenericReq_t *)pReq);	
}

zbStatus_t ZclDmndRspLdCtrl_CancelAllLdCtrlEvtReq
(
zclDmndRspLdCtrl_CancelAllLdCtrlEvtsReq_t *pReq
)
{
  return ZCL_SendServerReqSeqPassed(gZclCmdDmndRspLdCtrl_CancelAllLdCtrlEvtsReq_c, sizeof(zclCmdDmndRspLdCtrl_CancelAllLdCtrlEvtsReq_t),(zclGenericReq_t *)pReq);	
}

zbStatus_t ZclDmndRspLdCtrl_GetScheduledEvtsReq
(
zclDmndRspLdCtrl_GetScheduledEvts_t *pReq
)
{
  return ZCL_SendClientReqSeqPassed(gZclCmdDmndRspLdCtrl_GetScheduledEvts_c, sizeof(zclCmdDmndRspLdCtrl_GetScheduledEvts_t),(zclGenericReq_t *)pReq);	
}

/*-------- Price cluster commands ---------*/
zbStatus_t zclPrice_GetCurrPriceReq
(
zclPrice_GetCurrPriceReq_t *pReq
)
{
  return ZCL_SendClientReqSeqPassed(gZclCmdPrice_GetCurrPriceReq_c, sizeof(zclCmdPrice_GetCurrPriceReq_t),(zclGenericReq_t *)pReq);	
}


zbStatus_t zclPrice_GetScheduledPricesReq
(
zclPrice_GetScheduledPricesReq_t *pReq
)
{ 
  
  return ZCL_SendClientReqSeqPassed(gZclCmdPrice_GetScheduledPricesReq_c, sizeof(zclCmdPrice_GetScheduledPricesReq_t),(zclGenericReq_t *)pReq);	
}

zbStatus_t zclPrice_PublishPriceRsp
(
zclPrice_PublishPriceRsp_t *pReq
)
{
  uint8_t *pSrc,*pDst;
  uint8_t length;
  /*NOTE - depending on whether the Rate Label has a fixed length this function needs to package
  the telegram.
  The Stucture allows for 12 bytes of text, which is a size of 12 including the length.(11 byte of data)     
  aka
  copy from currentTime to rest of telegram to the location of Ratelabel+sizeof(length)+length.
  
  */
  
  pSrc  = (uint8_t *) &pReq->cmdFrame.IssuerEvt[0];
  pDst  = &pReq->cmdFrame.RateLabel.aStr[pReq->cmdFrame.RateLabel.length];  
  /*1 is the length byte*/
  length = sizeof(zclCmdPrice_PublishPriceRsp_t) - (sizeof(ProviderID_t) + sizeof(uint8_t) + pReq->cmdFrame.RateLabel.length);  
  FLib_MemInPlaceCpy(pDst, pSrc, length);
  length = sizeof(zclCmdPrice_PublishPriceRsp_t) - (11-pReq->cmdFrame.RateLabel.length);
  
  return ZCL_SendServerRspSeqPassed(gZclCmdPrice_PublishPriceRsp_c, length,(zclGenericReq_t *)pReq);	
}

#if gInterPanCommunicationEnabled_c

zbStatus_t zclPrice_InterPanGetCurrPriceReq
(
zclPrice_InterPanGetCurrPriceReq_t *pReq
)
{
  
  return ZCL_SendInterPanClientReqSeqPassed(gZclCmdPrice_GetCurrPriceReq_c, sizeof(zclCmdPrice_GetCurrPriceReq_t), pReq);	
}


zbStatus_t zclPrice_InterPanGetScheduledPricesReq
(
zclPrice_InterPanGetScheduledPricesReq_t *pReq
)
{ 
  
  return ZCL_SendInterPanClientReqSeqPassed(gZclCmdPrice_GetScheduledPricesReq_c, sizeof(zclCmdPrice_GetScheduledPricesReq_t), pReq);	
}


zbStatus_t zclPrice_InterPanPublishPriceRsp
(
zclPrice_InterPanPublishPriceRsp_t *pReq
)
{
  uint8_t *pSrc,*pDst;
  uint8_t length;
  /*NOTE - depending on whether the Rate Label has a fixed length this function needs to package
  the telegram.
  The Stucture allows for 12 bytes of text, which is a size of 12 including the length.(11 byte of data)     
  aka
  copy from currentTime to rest of telegram to the location of Ratelabel+sizeof(length)+length.
  
  */
  pSrc  = (uint8_t *) &pReq->cmdFrame.IssuerEvt[0];
  pDst  = &pReq->cmdFrame.RateLabel.aStr[pReq->cmdFrame.RateLabel.length];  
  /*1 is the length byte*/
  length = sizeof(zclCmdPrice_PublishPriceRsp_t) - (sizeof(ProviderID_t) + sizeof(uint8_t) + pReq->cmdFrame.RateLabel.length);  
  FLib_MemInPlaceCpy(pDst, pSrc, length);
  length = sizeof(zclCmdPrice_PublishPriceRsp_t) - (11-pReq->cmdFrame.RateLabel.length);
  return ZCL_SendInterPanServerRspSeqPassed(gZclCmdPrice_PublishPriceRsp_c, length, pReq);
}
#endif /* #if gInterPanCommunicationEnabled_c */

/*---------------- SE Tunneling  ---------------------*/

zbStatus_t ZCL_SETunnelingServer
(
	zbApsdeDataIndication_t *pIndication, /* IN: */
	afDeviceDef_t *pDev                /* IN: */
) 
{
  zclCmd_t Cmd;
  zbStatus_t status = gZclSuccessDefaultRsp_c;

 (void) pDev;

 Cmd = ((zclFrame_t *)pIndication->pAsdu)->command;

 switch(Cmd) {
  case gZclCmdSETunneling_Client_RequestTunnelReq_c: 
  {
    //zclCmdSETunneling_RequestTunnelReq_t *pReq = (zclCmdSETunneling_RequestTunnelReq_t *) ((pIndication->pAsdu) + sizeof(zclFrame_t));  
    //BeeAppUpdateDevice(pIndication->dstEndPoint, gZclUI_SETunneling_RequestTunnelReqReceived_c, 0, 0, pReq);
  }  
  break;  
  case gZclCmdSETunneling_Client_CloseTunnelReq_c:
  {
    //zclCmdSETunneling_CloseTunnelReq_t *pReq = (zclCmdSETunneling_CloseTunnelReq_t *) (pIndication->pAsdu + sizeof(zclFrame_t));  
  }  
  break;
  case gZclCmdSETunneling_Client_TransferDataReq_c:
  {
    //zclCmdSETunneling_TransferDataReq_t *pReq = (zclCmdSETunneling_TransferDataReq_t *) (pIndication->pAsdu + sizeof(zclFrame_t));  
  }  
  break;
  case gZclCmdSETunneling_Client_TransferDataErrorReq_c:
  {
    //zclCmdSETunneling_TransferDataErrorReq_t *pReq = (zclCmdSETunneling_TransferDataErrorReq_t *) (pIndication->pAsdu + sizeof(zclFrame_t));  
  }  
  break;
#ifdef gASL_ZclSETunnelingOptionals_d
  case gZclCmdSETunneling_Client_AckTransferDataRsp_c:
  {
    //zclCmdSETunneling_AckTransferDataRsp_t *pReq = (zclCmdSETunneling_AckTransferDataRsp_t *) (pIndication->pAsdu + sizeof(zclFrame_t));  
  }
  break;
  case gZclCmdSETunneling_Client_ReadyDataReq_c:
  {
    //zclCmdSETunneling_ReadyDataReq_t *pReq = (zclCmdSETunneling_ReadyDataReq_t *) (pIndication->pAsdu + sizeof(zclFrame_t));  
  }
  break;
#endif
  default:
    status = gZclUnsupportedClusterCommand_c;
  break;  
 }
 return status;
}

zbStatus_t ZCL_SETunnelingClient
(
	zbApsdeDataIndication_t *pIndication, /* IN: */
	afDeviceDef_t *pDev                /* IN: */
) 
{
  zclCmd_t Cmd;
  zbStatus_t status = gZclSuccessDefaultRsp_c;

 (void) pDev;

 Cmd = ((zclFrame_t *)pIndication->pAsdu)->command;
 switch(Cmd) {
  case gZclCmdSETunneling_Server_RequestTunnelRsp_c: 
  {
    //zclCmdSETunneling_RequestTunnelRsp_t *pReq = (zclCmdSETunneling_RequestTunnelRsp_t *) ((pIndication->pAsdu) + sizeof(zclFrame_t));  
  }  
  break;  
  case gZclCmdSETunneling_Server_TransferDataReq_c:
  {
    //zclCmdSETunneling_TransferDataReq_t *pReq = (zclCmdSETunneling_TransferDataReq_t *) (pIndication->pAsdu + sizeof(zclFrame_t));  
  }  
  break;
  case gZclCmdSETunneling_Server_TransferDataErrorReq_c:
  {
    //zclCmdSETunneling_TransferDataErrorReq_t *pReq = (zclCmdSETunneling_TransferDataErrorReq_t *) (pIndication->pAsdu + sizeof(zclFrame_t));
  }  
  break;
#ifdef gASL_ZclSETunnelingOptionals_d
  case gZclCmdSETunneling_Server_AckTransferDataRsp_c:
  {
    //zclCmdSETunneling_AckTransferDataRsp_t *pReq = (zclCmdSETunneling_AckTransferDataRsp_t *) (pIndication->pAsdu + sizeof(zclFrame_t));  
  }
  break;
  case gZclCmdSETunneling_Server_ReadyDataReq_c:
  {
    //zclCmdSETunneling_ReadyDataReq_t *pReq = (zclCmdSETunneling_ReadyDataReq_t *) (pIndication->pAsdu + sizeof(zclFrame_t));  
  }
  break;
#endif
  default:
    status = gZclUnsupportedClusterCommand_c;
  break;  
 }
 return status;
}

//client generated commands
zbStatus_t zclSETunneling_Client_RequestTunnelReq
(
zclSETunneling_RequestTunnelReq_t *pReq
)
{
  return ZCL_SendClientReqSeqPassed(gZclCmdSETunneling_Client_RequestTunnelReq_c, sizeof(zclCmdSETunneling_RequestTunnelReq_t),(zclGenericReq_t *)pReq);
}

zbStatus_t zclSETunneling_Client_CloseTunnelReq
(
zclSETunneling_CloseTunnelReq_t *pReq
)
{
  return ZCL_SendClientReqSeqPassed(gZclCmdSETunneling_Client_CloseTunnelReq_c, sizeof(zclCmdSETunneling_CloseTunnelReq_t),(zclGenericReq_t *)pReq);
}

zbStatus_t zclSETunneling_Client_TransferDataReq
(
zclSETunneling_TransferDataReq_t *pReq
)
{
  return ZCL_SendClientReqSeqPassed(gZclCmdSETunneling_Client_TransferDataReq_c, sizeof(zclCmdSETunneling_TransferDataReq_t) + pReq->cmdFrame.data.length - 1,(zclGenericReq_t *)pReq);
}

zbStatus_t zclSETunneling_Client_TransferDataErrorReq
(
zclSETunneling_TransferDataErrorReq_t *pReq
)
{
  return ZCL_SendClientReqSeqPassed(gZclCmdSETunneling_Client_TransferDataErrorReq_c, sizeof(zclCmdSETunneling_TransferDataErrorReq_t),(zclGenericReq_t *)pReq);
}

zbStatus_t zclSETunneling_Client_AckTransferDataRsp
(
zclSETunneling_AckTransferDataRsp_t *pReq
)
{
  return ZCL_SendClientRspSeqPassed(gZclCmdSETunneling_Client_AckTransferDataRsp_c, sizeof(zclCmdSETunneling_AckTransferDataRsp_t),(zclGenericReq_t *)pReq);
}

zbStatus_t zclSETunneling_Client_ReadyDataReq
(
zclSETunneling_ReadyDataReq_t *pReq
)
{
  return ZCL_SendClientReqSeqPassed(gZclCmdSETunneling_Client_ReadyDataReq_c, sizeof(zclCmdSETunneling_ReadyDataReq_t),(zclGenericReq_t *)pReq);
}

//server generated commands
zbStatus_t zclSETunneling_Server_RequestTunnelRsp
(
zclSETunneling_RequestTunnelRsp_t *pReq
)
{
  return ZCL_SendServerRspSeqPassed(gZclCmdSETunneling_Server_RequestTunnelRsp_c, sizeof(zclCmdSETunneling_RequestTunnelRsp_t),(zclGenericReq_t *)pReq);
}

zbStatus_t zclSETunneling_Server_TransferDataReq
(
zclSETunneling_TransferDataReq_t *pReq
)
{
  return ZCL_SendServerReqSeqPassed(gZclCmdSETunneling_Server_TransferDataReq_c, sizeof(zclCmdSETunneling_TransferDataReq_t),(zclGenericReq_t *)pReq);
}

zbStatus_t zclSETunneling_Server_TransferDataErrorReq
(
zclSETunneling_TransferDataErrorReq_t *pReq
)
{
  return ZCL_SendServerReqSeqPassed(gZclCmdSETunneling_Server_TransferDataErrorReq_c, sizeof(zclCmdSETunneling_TransferDataErrorReq_t),(zclGenericReq_t *)pReq);
}

zbStatus_t zclSETunneling_Server_AckTransferDataRsp
(
zclSETunneling_AckTransferDataRsp_t *pReq
)
{
  return ZCL_SendServerRspSeqPassed(gZclCmdSETunneling_Server_AckTransferDataRsp_c, sizeof(zclCmdSETunneling_AckTransferDataRsp_t),(zclGenericReq_t *)pReq);
}

zbStatus_t zclSETunneling_Server_ReadyDataReq
(
zclSETunneling_ReadyDataReq_t *pReq
)
{
  return ZCL_SendServerReqSeqPassed(gZclCmdSETunneling_Server_ReadyDataReq_c, sizeof(zclCmdSETunneling_ReadyDataReq_t),(zclGenericReq_t *)pReq);
}

/*---------------- Prepayment Cluster  ---------------------*/
//client indication
zbStatus_t ZCL_PrepaymentClient
(
	zbApsdeDataIndication_t *pIndication, /* IN: */
	afDeviceDef_t *pDev                /* IN: */
)
{
  zclCmd_t Cmd;
  zbStatus_t status = gZclSuccessDefaultRsp_c;

 (void) pDev;

 Cmd = ((zclFrame_t *)pIndication->pAsdu)->command;
 switch(Cmd) {
  case gZclCmdPrepayment_Server_SupplyStatRsp_c: 
  {
    return gZclSuccess_c;
  }    
  default:
    status = gZclUnsupportedClusterCommand_c;
  break;  
 }
 return status;
}

//server indication
zbStatus_t ZCL_PrepaymentServer
(
	zbApsdeDataIndication_t *pIndication, /* IN: */
	afDeviceDef_t *pDev                /* IN: */
)
{
  zclCmd_t Cmd;
  zbStatus_t status = gZclSuccessDefaultRsp_c;

 (void) pDev;

 Cmd = ((zclFrame_t *)pIndication->pAsdu)->command;
 switch(Cmd) {
  case gZclCmdPrepayment_Client_SelAvailEmergCreditReq_c: 
  {
    return gZclSuccess_c;
  }
  case gZclCmdPrepayment_Client_ChangeSupplyReq_c:
  {
    return ZCL_ProcessChangeSupplyReq(pIndication);
  }
  default:
  {
    status = gZclUnsupportedClusterCommand_c;
  }
  break;  
 }
 return status;
}

//client generated commands
zbStatus_t zclPrepayment_Client_SelAvailEmergCreditReq
(
zclPrepayment_SelAvailEmergCreditReq_t *pReq
)
{
  return ZCL_SendClientReqSeqPassed(gZclCmdPrepayment_Client_SelAvailEmergCreditReq_c, sizeof(zclCmdPrepayment_SelAvailEmergCreditReq_t),(zclGenericReq_t *)pReq);
}

zbStatus_t zclPrepayment_Client_ChangeSupplyReq
(
zclPrepayment_ChangeSupplyReq_t *pReq
)
{
  return ZCL_SendClientReqSeqPassed(gZclCmdPrepayment_Client_ChangeSupplyReq_c, sizeof(zclCmdPrepayment_ChangeSupplyReq_t),(zclGenericReq_t *)pReq);
}

//server generated commands
static zbStatus_t zclPrepayment_Server_SupplyStatusRsp
(
zclPrepayment_SupplyStatRsp_t *pReq
)
{
  return ZCL_SendServerRspSeqPassed(gZclCmdPrepayment_Server_SupplyStatRsp_c, sizeof(zclCmdPrepayment_SupplyStatRsp_t),(zclGenericReq_t *)pReq);
}

static zbStatus_t ZCL_ProcessChangeSupplyReq
(
zbApsdeDataIndication_t *pIndication
)
{
  zclCmdPrepayment_ChangeSupplyReq_t  *pMsg;
  pMsg = (zclCmdPrepayment_ChangeSupplyReq_t*)((uint8_t*)pIndication->pAsdu + sizeof(zclFrame_t));
  if(pMsg->originatorId & 0x08)//Acknowledge Required?
    return ZCL_SendSupplyStatusRsp(pIndication);
  else
    return gZclSuccess_c;
}

static zbStatus_t ZCL_SendSupplyStatusRsp
(
zbApsdeDataIndication_t *pIndication
)
{
  zclPrepayment_SupplyStatRsp_t       response;
  zclCmdPrepayment_ChangeSupplyReq_t  *pMsg;
  //get the request to exctract the required info
  pMsg = (zclCmdPrepayment_ChangeSupplyReq_t*)((uint8_t*)pIndication->pAsdu + sizeof(zclFrame_t));
  //prepare the response
  AF_PrepareForReply(&response.addrInfo, pIndication);
  response.zclTransactionId = ((zclFrame_t *)pIndication->pAsdu)->transactionId;
  FLib_MemCpy(&response.cmdFrame.providerId, &pMsg->providerId, sizeof(ProviderID_t));
  response.cmdFrame.implementationDateTime = pMsg->implementationDateTime;
  response.cmdFrame.supplyStatus = pMsg->proposedSupplyStatus;//application should set this
  
  return zclPrepayment_Server_SupplyStatusRsp(&response);
}
/*-------- key Establishment cluster commands ---------*/

void ZCL_ApplyECDSASign(uint8_t *pBufIn, uint8_t lenIn, uint8_t *pBufOut)
{
  uint8_t digest[16];
  /* Hash the pBufIn */
  BeeUtilZeroMemory(digest, 16);
  SSP_MatyasMeyerOseasHash(pBufIn, lenIn, digest);

  (void)ZSE_ECDSASign( DevicePrivateKey, &digest[0],
                ECC_GetRandomDataFunc, pBufOut, (pBufOut + 21), NULL, 0 );

}


zbStatus_t zclKeyEstab_InitKeyEstabReq
(
ZclKeyEstab_InitKeyEstabReq_t *pReq
) 
{
 	Set2Bytes(pReq->addrInfo.aClusterId, gKeyEstabCluster_c);
  return ZCL_SendClientReqSeqPassed(gZclCmdKeyEstab_InitKeyEstabReq_c, sizeof(ZclCmdKeyEstab_InitKeyEstabReq_t),(zclGenericReq_t *)pReq);
}

zbStatus_t zclKeyEstab_EphemeralDataReq
(
ZclKeyEstab_EphemeralDataReq_t *pReq
) 
{
		Set2Bytes(pReq->addrInfo.aClusterId, gKeyEstabCluster_c);
  return ZCL_SendClientReqSeqPassed(gZclCmdKeyEstab_EphemeralDataReq_c, sizeof(ZclCmdKeyEstab_EphemeralDataReq_t),(zclGenericReq_t *)pReq);
}


zbStatus_t zclKeyEstab_ConfirmKeyDataReq
(
ZclKeyEstab_ConfirmKeyDataReq_t *pReq
) 
{
  	Set2Bytes(pReq->addrInfo.aClusterId, gKeyEstabCluster_c);
  return ZCL_SendClientReqSeqPassed(gZclCmdKeyEstab_ConfirmKeyDataReq_c, sizeof(ZclCmdKeyEstab_ConfirmKeyDataReq_t),(zclGenericReq_t *)pReq);
}

zbStatus_t zclKeyEstab_TerminateKeyEstabServer
(
ZclKeyEstab_TerminateKeyEstabServer_t *pReq
) 
{
  	Set2Bytes(pReq->addrInfo.aClusterId, gKeyEstabCluster_c);
  return ZCL_SendClientReqSeqPassed(gZclCmdKeyEstab_TerminateKeyEstabServer_c, sizeof(ZclCmdKeyEstab_TerminateKeyEstabServer_t),(zclGenericReq_t *)pReq);
}

zbStatus_t zclKeyEstab_InitKeyEstabRsp
(
ZclKeyEstab_InitKeyEstabRsp_t *pReq
) 
{
  	Set2Bytes(pReq->addrInfo.aClusterId, gKeyEstabCluster_c);
  return ZCL_SendServerRspSeqPassed(gZclCmdKeyEstab_InitKeyEstabRsp_c, sizeof(ZclCmdKeyEstab_InitKeyEstabRsp_t),(zclGenericReq_t *)pReq);
}

zbStatus_t zclKeyEstab_EphemeralDataRsp
(
ZclKeyEstab_EphemeralDataRsp_t *pReq
) 
{
  	Set2Bytes(pReq->addrInfo.aClusterId, gKeyEstabCluster_c);
  return ZCL_SendServerRspSeqPassed(gZclCmdKeyEstab_EphemeralDataRsp_c, sizeof(ZclCmdKeyEstab_EphemeralDataRsp_t),(zclGenericReq_t *)pReq);
}


zbStatus_t zclKeyEstab_ConfirmKeyDataRsp
(
ZclKeyEstab_ConfirmKeyDataRsp_t *pReq
) 
{ 
  	Set2Bytes(pReq->addrInfo.aClusterId, gKeyEstabCluster_c);
  return ZCL_SendServerRspSeqPassed(gZclCmdKeyEstab_ConfirmKeyDataRsp_c, sizeof(ZclCmdKeyEstab_ConfirmKeyDataRsp_t),(zclGenericReq_t *)pReq);
}

zbStatus_t zclKeyEstab_TerminateKeyEstabClient
(
ZclKeyEstab_TerminateKeyEstabClient_t *pReq
) 
{
	Set2Bytes(pReq->addrInfo.aClusterId, gKeyEstabCluster_c);
  return ZCL_SendServerRspSeqPassed(gZclCmdKeyEstab_TerminateKeyEstabClient_c, sizeof(ZclCmdKeyEstab_TerminateKeyEstabClient_t),(zclGenericReq_t *)pReq);
}

bool_t ZCL_InitiateKeyEstab(zbEndPoint_t DstEndpoint,zbEndPoint_t SrcEndpoint, zbNwkAddr_t DstAddr) {
  // Init key estab. state machine. (key establishment inprogress)
   if (InitKeyEstabStateMachine())  
   {
#if !gZclKeyEstabDebugMode_d
    ZCL_SendInitiatKeyEstReq(&DeviceImplicitCert, DstEndpoint, SrcEndpoint,(uint8_t*) DstAddr);
#else
    (void)DstEndpoint;
    (void)SrcEndpoint;
    (void)DstAddr;
#endif 
    InitAndStartKeyEstabTimeout();
    return TRUE;
   } else {
     return FALSE;   
   }
}

bool_t InitClientKeyEstab(void)
{
  if (InitKeyEstabStateMachine())
  {
    InitAndStartKeyEstabTimeout();
    return TRUE;
  }
  return FALSE;
}
  
void ZCL_SendInitiatKeyEstReq(IdentifyCert_t *Cert, zbEndPoint_t DstEndpoint,zbEndPoint_t SrcEndpoint, zbNwkAddr_t DstAddr) {
ZclKeyEstab_InitKeyEstabReq_t *pReq;
  
  pReq = MSG_Alloc(sizeof(ZclKeyEstab_InitKeyEstabReq_t)); 
  
  if(pReq) {
  pReq->addrInfo.dstAddrMode = gZbAddrMode16Bit_c;
  Copy2Bytes(pReq->addrInfo.dstAddr.aNwkAddr, DstAddr);
  pReq->addrInfo.dstEndPoint = DstEndpoint;
  pReq->addrInfo.srcEndPoint = SrcEndpoint;
  pReq->addrInfo.txOptions = 0;
  pReq->addrInfo.radiusCounter = afDefaultRadius_c;
  pReq->zclTransactionId = gZclTransactionId++;
	
  pReq->cmdFrame.EphemeralDataGenerateTime = gKeyEstab_DefaultEphemeralDataGenerateTime_c;
  pReq->cmdFrame.ConfirmKeyGenerateTime = gKeyEstab_DefaultConfirmKeyGenerateTime_c;
  Set2Bytes(pReq->cmdFrame.SecuritySuite,gKeyEstabSuite_CBKE_ECMQV_c);
  
  FLib_MemCpy(pReq->cmdFrame.IdentityIDU,(uint8_t *)Cert ,sizeof(IdentifyCert_t)/sizeof(uint8_t));
  
  (void) zclKeyEstab_InitKeyEstabReq(pReq);
  MSG_Free(pReq);
  }
}

void ZCL_SendEphemeralDataReq(uint8_t *EphemeralPubKey, zbApsdeDataIndication_t *pIndication) {
ZclKeyEstab_EphemeralDataReq_t *pReq;
  
  pReq = MSG_Alloc(sizeof(ZclKeyEstab_EphemeralDataReq_t)); 
  
  if(pReq) {
    AF_PrepareForReply(&pReq->addrInfo, pIndication);
	
  pReq->zclTransactionId = gZclTransactionId++;
	
  FLib_MemCpy(pReq->cmdFrame.EphemeralDataQEU,(uint8_t *)EphemeralPubKey ,gZclCmdKeyEstab_CompressedPubKeySize_c);
  
  (void) zclKeyEstab_EphemeralDataReq(pReq);
  MSG_Free(pReq);
  }
}
void ZCL_SendConfirmKeyDataReq(uint8_t *MACU, zbApsdeDataIndication_t *pIndication) {
ZclKeyEstab_ConfirmKeyDataReq_t *pReq;
  
  pReq = MSG_Alloc(sizeof(ZclKeyEstab_ConfirmKeyDataReq_t)); 
  
  if(pReq) {
    AF_PrepareForReply(&pReq->addrInfo, pIndication);
	
  pReq->zclTransactionId = gZclTransactionId++;
	
  FLib_MemCpy(pReq->cmdFrame.SecureMsgAuthCodeMACU,(uint8_t *)MACU ,gZclCmdKeyEstab_AesMMOHashSize_c);
  
  (void) zclKeyEstab_ConfirmKeyDataReq(pReq);
  MSG_Free(pReq);
  }
}

void ZCL_SendTerminateKeyEstabServerReq(uint8_t Status,uint8_t WaitCode, zbApsdeDataIndication_t *pIndication) {
ZclKeyEstab_TerminateKeyEstabServer_t *pReq;
  
  pReq = MSG_Alloc(sizeof(ZclKeyEstab_TerminateKeyEstabServer_t)); 
  
  if(pReq) {
    AF_PrepareForReply(&pReq->addrInfo, pIndication);
	
  pReq->zclTransactionId = gZclTransactionId++;
	
  pReq->cmdFrame.StatusCode  = Status;
  pReq->cmdFrame.WaitCode    = WaitCode;
  Set2Bytes(pReq->cmdFrame.SecuritySuite,gKeyEstabSuite_CBKE_ECMQV_c);  
  
  (void) zclKeyEstab_TerminateKeyEstabServer(pReq);
  MSG_Free(pReq);
  }
}




void ZCL_SendInitiatKeyEstRsp(IdentifyCert_t *Cert,  zbApsdeDataIndication_t *pIndication, uint8_t transactionid) {
ZclKeyEstab_InitKeyEstabRsp_t *pReq;
  
  pReq = MSG_Alloc(sizeof(ZclKeyEstab_InitKeyEstabReq_t)); 
  
  if(pReq) {
    AF_PrepareForReply(&pReq->addrInfo, pIndication);
	
  pReq->zclTransactionId = transactionid;
	
  pReq->cmdFrame.EphemeralDataGenerateTime = gKeyEstab_DefaultEphemeralDataGenerateTime_c;
  pReq->cmdFrame.ConfirmKeyGenerateTime = gKeyEstab_DefaultConfirmKeyGenerateTime_c;
  Set2Bytes(pReq->cmdFrame.SecuritySuite,gKeyEstabSuite_CBKE_ECMQV_c);
  
  FLib_MemCpy(pReq->cmdFrame.IdentityIDV,(uint8_t *)Cert ,sizeof(IdentifyCert_t)/sizeof(uint8_t));
  
  (void) zclKeyEstab_InitKeyEstabRsp(pReq);
  MSG_Free(pReq);
  }
}

void  ZCL_SendEphemeralDataRsp(uint8_t *EphemeralPubKey,  zbApsdeDataIndication_t *pIndication, uint8_t transactionid) {
ZclKeyEstab_EphemeralDataRsp_t *pReq;
  
  pReq = MSG_Alloc(sizeof(ZclKeyEstab_EphemeralDataRsp_t)); 
  
  if(pReq) {
    AF_PrepareForReply(&pReq->addrInfo, pIndication);
	
  pReq->zclTransactionId = transactionid;	

  FLib_MemCpy(pReq->cmdFrame.EphemeralDataQEV,(uint8_t *)EphemeralPubKey ,gZclCmdKeyEstab_CompressedPubKeySize_c);
  
  (void) zclKeyEstab_EphemeralDataRsp(pReq);
  MSG_Free(pReq);
  }
}

void  ZCL_SendConfirmKeyDataRsp(uint8_t *MACV, zbApsdeDataIndication_t *pIndication, uint8_t transactionid) {
ZclKeyEstab_ConfirmKeyDataRsp_t *pReq;
  
  pReq = MSG_Alloc(sizeof(ZclKeyEstab_ConfirmKeyDataRsp_t)); 
  
  if(pReq) {
    AF_PrepareForReply(&pReq->addrInfo, pIndication);
	
  pReq->zclTransactionId = transactionid;	

  FLib_MemCpy(pReq->cmdFrame.SecureMsgAuthCodeMACV,(uint8_t *)MACV ,gZclCmdKeyEstab_AesMMOHashSize_c);
  
  (void) zclKeyEstab_ConfirmKeyDataRsp(pReq);
  MSG_Free(pReq);
  }
}

void  ZCL_SendTerminateKeyEstabClientReq(uint8_t Status, uint8_t WaitCode,zbApsdeDataIndication_t *pIndication , uint8_t transactionid) {
ZclKeyEstab_TerminateKeyEstabClient_t *pReq;
  
  pReq = MSG_Alloc(sizeof(ZclKeyEstab_TerminateKeyEstabClient_t)); 
  
  if(pReq) {
    AF_PrepareForReply(&pReq->addrInfo, pIndication);
    	
  pReq->zclTransactionId = transactionid;	

  pReq->cmdFrame.StatusCode  = Status;
  pReq->cmdFrame.WaitCode    = WaitCode;
  Set2Bytes(pReq->cmdFrame.SecuritySuite,gKeyEstabSuite_CBKE_ECMQV_c);  

  (void) zclKeyEstab_TerminateKeyEstabClient(pReq);
  MSG_Free(pReq);
  }
}

ESPRegisterDevFunc *pSE_ESPRegFunc = NULL;
ESPDeRegisterDevFunc *pSE_ESPDeRegFunc = NULL;

void ZCL_Register_EspRegisterDeviceFunctions(ESPRegisterDevFunc *RegFunc,ESPDeRegisterDevFunc *DeRegFunc)
{
  pSE_ESPRegFunc = RegFunc;
  pSE_ESPDeRegFunc = DeRegFunc;
}


/******************************************************************************
*******************************************************************************
* Private functions
*******************************************************************************
******************************************************************************/
int ECC_GetRandomDataFunc(unsigned char *buffer, unsigned long sz)
{
        unsigned long i;
        for(i=0;i<sz;i++){
                ((unsigned char *)buffer)[i] =(uint8_t) GetRandomNumber();
        }
        return 0x00; //success
} 

int ECC_HashFunc(unsigned char *digest, unsigned long sz, unsigned char *data)
{
/* Insert to use Certicom's AES MMO function*/
/*  aesMmoHash(digest, sz, data);  */
  /*zero initialize digest before generating hash*/
  BeeUtilZeroMemory(digest,16);
  SSP_MatyasMeyerOseasHash(data, (uint8_t) sz, digest);  
  return 0x00; //success
}

/******************************************************************************/ 
uint16_t crc_reflect(uint16_t data, uint8_t data_len)
{
    uint8_t i;
    uint16_t ret;
 
    ret = 0;
    for (i = 0; i < data_len; i++)
    {
        if (data & 0x01) {
            ret = (ret << 1) | 1;
        } else {
            ret = ret << 1;
        }
        data >>= 1;
    }
    return ret;
}
 
/******************************************************************************/ 
uint16_t crc_update(uint16_t crc16, uint8_t *pInput, uint8_t length)
{
uint8_t data;
  while (length--)
  {
 data = (uint8_t)crc_reflect(*pInput++, 8); // pInput may need to be shifted 8 bits up if running code on a big endian machine.
 
    crc16 = (uint8_t)(crc16 >> 8) | (crc16 << 8);
    crc16 ^= data; // x^1
    crc16 ^= (uint8_t)(crc16 & 0xff) >> 4; // ???
    crc16 ^= (crc16 << 8) << 4;    // x^12
    crc16 ^= ((crc16 & 0xff) << 4) << 1;    // X^5
  }
  return (0xFFFF^crc_reflect(crc16, 16));
}
/******************************************************************************/
/******************************************************************************
 
Name        : GenerateKeyFromInstallCode
Input(s)    : pInstallationCode: pointer to an installation code (max 144bit incl crc16)
 
              length: length of installation key (in bytes incl CRC)
 
In/Output(s): pKey: pointer to key (or hash)
 
Description : This function computes an initial key based on an installation code
 
*******************************************************************************/
bool_t GenerateKeyFromInstallCode(uint8_t length,uint8_t* pInstallationCode, uint8_t *pKey)
{
  // 1. Validate InstallationCode (CRC16)
  // 2. Pad installation code (M) to obtain M' 
  // 3. Compute hash using AES128bit engine
  uint16_t mCrc16;
  uint16_t vCrc16 = 0xffff;
  mCrc16=(pInstallationCode[length-1]<<8 | pInstallationCode[length-2]);
  vCrc16 = crc_update(vCrc16,pInstallationCode, length-2);
  // Check CRC
  if(mCrc16!=vCrc16){
    return FALSE;
  } 
  else{
    SSP_MatyasMeyerOseasHash(pInstallationCode,length ,pKey);
    return TRUE;  
  }
}

/* place functions here that will be shared by other modules */
zbStatus_t ZCL_ESPRegisterDevice(EspRegisterDevice_t *Devinfo)
{
  zbAESKey_t key = {0};
  index_t    i;
  zbNwkAddr_t unknownAddr = {0xFF,0xFF};
  i = ZCL_AddToRegTable(Devinfo);
  if (i != 0xFF)
  {
    if (GenerateKeyFromInstallCode(RegistrationTable[i].DevInfo.InstallCodeLength,RegistrationTable[i].DevInfo.InstallCode,key)) {    
      if (APS_AddToAddressMapPermanent(Devinfo->Addr,unknownAddr))
        APS_RegisterLinkKeyData(RegistrationTable[i].DevInfo.Addr,gApplicationLinkKey_c,(uint8_t *) key);        
      else
        return gZclNoMem_c;    
    return gZclSuccess_c;  
    } else
     return gZclFailure_c;
  }
  return gZclNoMem_c;
    
}

zbStatus_t ZCL_ESPDeRegisterDevice(EspDeRegisterDevice_t *Devinfo)
{
  index_t Index;
  Index = ZCL_FindIeeeInRegTable(Devinfo->Addr);
  if (Index != 0xff)
  {
    ASL_Mgmt_Leave_req(NULL, (uint8_t *)gaBroadcastRxOnIdle,RegistrationTable[Index].DevInfo.Addr,0);
    APS_RemoveSecurityMaterialEntry(RegistrationTable[Index].DevInfo.Addr);
    APS_RemoveFromAddressMap(Devinfo->Addr);
    Fill8BytesToZero(RegistrationTable[Index].DevInfo.Addr); 
    return gZclSuccess_c;
  } else
    return gZclNoMem_c;   
}

index_t ZCL_FindIeeeInRegTable(zbIeeeAddr_t aExtAddr)
{
  index_t i;

  for(i=0; i<RegistrationTableEntries_c; ++i)
  {
    /* found the entry */
    if(IsEqual8Bytes(RegistrationTable[i].DevInfo.Addr, aExtAddr))
      return i;
  }
  return 0xFF;
}

index_t ZCL_AddToRegTable(EspRegisterDevice_t *Devinfo)
{
  index_t i,a;
  index_t iFree;

  /* indicate we haven't found a free one yet */
  iFree = REgTable_InvalidIndex_c;

  for(i=0; i<RegistrationTableEntries_c; ++i)
  {
    /* found the entry */
    if(IsEqual8Bytes(RegistrationTable[a].DevInfo.Addr, Devinfo->Addr))
    {
      /*Ieee address already exist in table, exist and do nothing*/
      return REgTable_InvalidIndex_c;
    }

    /* record first free entry */
    if(iFree == REgTable_InvalidIndex_c && Cmp8BytesToZero(RegistrationTable[a].DevInfo.Addr))
      iFree = i;

    /* on to next record */
    ++a;
  }
  /* return indicating full */
  if(iFree == REgTable_InvalidIndex_c)
    return iFree;

  /* add in new entry */
  FLib_MemCpy((void*) &RegistrationTable[iFree].DevInfo, (void*)Devinfo, sizeof(EspRegisterDevice_t));
#if gEccIncluded_d == 1
  RegistrationTable[iFree].DevStatus = RegTable_DevStatusIntialState_c;
#else
  // If Ecc library is not included, then set that device has done key estab.
  RegistrationTable[iFree].DevStatus = RegTable_DevStatusKeyEstabComplete_c;  
#endif  
  return iFree;
}

void ZCL_ESPInit(void)
{
    ZCL_Register_EspRegisterDeviceFunctions(ZCL_ESPRegisterDevice, ZCL_ESPDeRegisterDevice);
}

void ZCL_SetKeyEstabComplete(zbIeeeAddr_t aExtAddr)
{
    index_t Index;
    Index = ZCL_FindIeeeInRegTable(aExtAddr); 
    if (Index != REgTable_InvalidIndex_c)
    RegistrationTable[Index].DevStatus = RegTable_DevStatusKeyEstabComplete_c;
}  

bool_t ZCL_IsKeyEstablishmentCompleted(zbIeeeAddr_t aExtAddr)
{
    index_t Index;
    Index = ZCL_FindIeeeInRegTable(aExtAddr); 
    if (Index != REgTable_InvalidIndex_c)
      return (RegistrationTable[Index].DevStatus == RegTable_DevStatusKeyEstabComplete_c);  

return FALSE;    
}

bool_t ZCl_SEClusterSecuritycheck(zbApsdeDataIndication_t *pIndication)
{
  uint16_t cluster;

   /*if packets are from our self then do not check security level */
  if (IsSelfNwkAddress(pIndication->aSrcAddr)) 
  {  
    return TRUE;
  }

  Copy2Bytes(&cluster, pIndication->aClusterId);
  /*  cluster = OTA2Native16(cluster);*/
  #if gStandardSecurity_d
  if((cluster == gZclClusterBasic_c)    ||
     (cluster == gZclClusterIdentify_c) ||
     (cluster == gKeyEstabCluster_c)) 
  {
    if((pIndication->fSecurityStatus & gApsSecurityStatus_Nwk_Key_c) == gApsSecurityStatus_Nwk_Key_c)
      return TRUE;   
    else
      return FALSE;
  }


  if((cluster == gZclClusterSmplMet_c) ||
     (cluster == gZclClusterMsg_c)      ||
     (cluster == gZclClusterPrice_c)    ||   
     (cluster == gZclClusterDmndRspLdCtrl_c) ||
     (cluster == gZclClusterTime_c)     ||
     (cluster == gZclClusterCommissioning_c))
   {
#if gApsLinkKeySecurity_d  
    if((pIndication->fSecurityStatus & gApsSecurityStatus_Link_Key_c) == gApsSecurityStatus_Link_Key_c)
      return TRUE;
    else
      return FALSE;
#else
   return TRUE;
#endif    
   }
#endif
  return TRUE; 
}

