/******************************************************************************
* ZclOTA.c
*
* This module contains code that handles ZigBee Cluster Library commands and
* clusters for OTA (Over The Air upgrading cluster).
*
* Copyright (c) 2011, Freescale, Inc.  All rights reserved.
*
*
* No part of this document must be reproduced in any form - including copied,
* transcribed, printed or by any electronic means - without specific written
* permission from Freescale Semiconductor.
*
* Documents used in this specification:
* [R1] - 095264r17.pdf
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
#include "ZclOptions.h"

#if gZclEnableOTAServer_d || gZclEnableOTAClient_d

#include "OtaSupport.h"
#include "ZclOta.h"
#include "ZtcInterface.h"



#ifndef __IAR_SYSTEMS_ICC__
#include "PwrLib.h"
#ifdef MC13226Included_d
    #undef MC13226Included_d
#endif    
    #define MC13226Included_d 0
#else
#include "crm.h"
#endif

/******************************************************************************
*******************************************************************************
* Private macros
*******************************************************************************
******************************************************************************/

#define OTA_MANUFACTURER_CODE_FSL         0x1004
#define OTA_MANUFACTURER_CODE_MATCH_ALL   0xFFFF


#define OTA_IMAGE_TYPE_CURRENT            0x0000

#define OTA_IMAGE_TYPE_MAX_MANUFACTURER   0xFFBF
#define OTA_IMAGE_TYPE_CREDENTIAL         0xFFC0
#define OTA_IMAGE_TYPE_CONFIGURATION      0xFFC1
#define OTA_IMAGE_TYPE_LOG                0xFFC2  
#define OTA_IMAGE_TYPE_MATCH_ALL          0xFFFF


#define OTA_FILE_VERSION_CURRENT          0x30103010
#define OTA_FILE_VERSION_DL_DEFAULT       0x00000000

#ifdef __IAR_SYSTEMS_ICC__
#if MC13226Included_d
#define OTA_HW_VERSION_CURRENT            0x2260 
#else
#define OTA_HW_VERSION_CURRENT            0x2240
#endif
#else  //  __IAR_SYSTEMS_ICC__
#if gTargetQE128EVB_d                    
#define OTA_HW_VERSION_CURRENT            0x2028
#else
#if gTargetMC1323xRCM_d || gTargetMC1323xREM_d
#define OTA_HW_VERSION_CURRENT            0x2028
#endif
#endif
#endif

#define OTA_CLIENT_MAX_DATA_SIZE          0x30
#define OTA_CLIENT_MAX_RANDOM_JITTER      100

#define gHaZtcOpCodeGroup_c                 0x70 /* opcode group used for HA commands/events */

#define gOTAImageNotify_c                   0xD0
#define gOTAQueryNextImageRequest_c         0xD1
#define gOTAQueryNextImageResponse_c        0xD2
#define gOTAUpgradeEndRequest_c             0xD3
#define gOTAUpgradeEndResponse_c            0xD4
#define gOTASetClientParams_c               0xD5
#define gOTABlockRequest_c                  0xD6
#define gOTABlockResponse_c                 0xD7

#define HEADER_LEN_OFFSET                   6
#define SESSION_BUFFER_SIZE                 128
#define BLOCK_PROCESSING_CALLBACK_DELAY     50  // ms

/******************************************************************************
*******************************************************************************
* Private prototypes
*******************************************************************************
******************************************************************************/

#if gZclEnableOTAServer_d
zbStatus_t ZCL_OTAImageNotify(zclZtcOTAImageNotify_t* pZtcImageNotifyParams);
zbStatus_t ZCL_OTANextImageResponse(zclZtcOTANextImageResponse_t* pZtcNextImageResponseParams);
zbStatus_t ZCL_OTABlockResponse(zclZtcOTABlockResponse_t *pZtcBlockResponseParams);
zbStatus_t ZCL_OTAUpgradeEndResponse(ztcZclOTAUpgdareEndResponse_t* pZtcOTAUpgdareEndResponse);
#endif

#if gZclEnableOTAClient_d
void ZCL_OTASetClientParams(zclOTAClientParams_t* pClientParams);
zbStatus_t ZCL_OTAImageRequest(zclZtcOTANextImageRequest_t* pZtcNextImageRequestParams);
zbStatus_t ZCL_OTABlockRequest(zclZtcOTABlockRequest_t *pZtcBlockRequestParams);
zbStatus_t ZCL_OTAUpgradeEndRequest(zclZtcOTAUpgradeEndRequest_t *pZtcUpgradeEndParams);
void OTAClusterClientAbortSession(void);
zbStatus_t OTAClusterClientStartSession(uint32_t fileLength, uint32_t fileVersion);
zbStatus_t OTAClusterClientRunImageProcessStateMachine();
zbStatus_t OTAClusterClientProcessBlock(uint8_t *pImageBlock, uint8_t blockLength);
static void OTAClusterClientProcessBlockTimerCallback(uint8_t tmr);
static void OTAClusterCPUResetCallback(uint8_t tmr);
#endif

/******************************************************************************
*******************************************************************************
* Private type definitions
*******************************************************************************
******************************************************************************/

/******************************************************************************
*******************************************************************************
* Public memory definitions
*******************************************************************************
******************************************************************************/

/******************************************************************************
*******************************************************************************
* Private memory declarations
*******************************************************************************
******************************************************************************/

/******************************
  OTA Cluster Data
  See ZCL Specification Section 6.3
*******************************/

zclOTAAttrsRAM_t gOTAAttrs;

const zclAttrDef_t gaZclOTAClusterAttrDef[] = {
  { gZclAttrOTA_UpgradeServerIdId_c,              gZclDataTypeArray_c,gZclAttrFlagsCommon_c|gZclAttrFlagsRdOnly_c,MbrSizeof(zclOTAAttrsRAM_t, UpgradeServerId),  &gOTAAttrs.UpgradeServerId},
#if gZclClusterOptionals_d
  { gZclAttrOTA_FileOffsetId_c,                   gZclDataTypeInt32_c,gZclAttrFlagsCommon_c|gZclAttrFlagsRdOnly_c,sizeof(uint32_t), &gOTAAttrs.FileOffset},
  { gZclAttrOTA_CurrentFileVersionId_c,           gZclDataTypeInt32_c,gZclAttrFlagsCommon_c|gZclAttrFlagsRdOnly_c,sizeof(uint32_t), &gOTAAttrs.CurrentFileVersion},
  { gZclAttrOTA_CurrentZigBeeStackVersionId_c,    gZclDataTypeInt16_c,gZclAttrFlagsCommon_c|gZclAttrFlagsRdOnly_c,sizeof(uint16_t), &gOTAAttrs.CurrentZigBeeStackVersion},
  { gZclAttrOTA_DownloadedFileVersionId_c,        gZclDataTypeInt32_c,gZclAttrFlagsCommon_c|gZclAttrFlagsRdOnly_c,sizeof(uint32_t), &gOTAAttrs.DownloadedFileVersion},
  { gZclAttrOTA_DownloadedZigBeeStackVersionId_c, gZclDataTypeInt16_c,gZclAttrFlagsCommon_c|gZclAttrFlagsRdOnly_c,sizeof(uint16_t), &gOTAAttrs.DownloadedZigBeeStackVersion},
  { gZclAttrOTA_ImageUpgradeStatusId_c,           gZclDataTypeInt8_c, gZclAttrFlagsCommon_c|gZclAttrFlagsRdOnly_c,sizeof(uint8_t), &gOTAAttrs.ImageUpgradeStatus},
#endif
};

const zclAttrDefList_t gZclOTAClusterAttrDefList = {
  NumberOfElements(gaZclOTAClusterAttrDef),
  gaZclOTAClusterAttrDef
};



//  Size optimization. The values represent
//  should represent the size of the Image Notify Command
//  based on the payload type.
const uint8_t imgNotifySizeArray[gPayloadMax_c] = {2, 4, 6, 10};

#if gZclEnableOTAClient_d

zclOTAClientParams_t  clientParams = {
                                      OTA_MANUFACTURER_CODE_FSL,
                                      OTA_IMAGE_TYPE_CURRENT,
                                      OTA_FILE_VERSION_CURRENT,
                                      OTA_FILE_VERSION_DL_DEFAULT,
                                      OTA_HW_VERSION_CURRENT,
                                      OTA_CLIENT_MAX_DATA_SIZE
                                     };


zclOTAClientSession_t clientSession = { 
                                        FALSE,      // sessionStarted,
                                        0x00000000, // fileLength
                                        0x00000000, // currentOffset
                                        0x00000000, // downloadingFileVersion
                                        0x00,       // bytesNeedForNextState
                                        NULL,       // pStateBuffer
                                        0x00,       // stateBufferIndex
                                        init_c      // state
                                       };

zclOTABlockCallbackState_t* gpBlockCallbackState = NULL;
tmrTimerID_t gBlockCallbackTimer = 0;
        
#endif


/******************************************************************************
*******************************************************************************
* Public functions
*******************************************************************************
******************************************************************************/


/*****************************************************************************
******************************************************************************
*
* ZCL OTA SERVER FUNCTIONS
*
******************************************************************************
*****************************************************************************/

#if gZclEnableOTAServer_d
/*****************************************************************************
* ZCL_OTAClusterServer Function
*
* Incoming OTA cluster frame processing loop for cluster server side
*****************************************************************************/
zbStatus_t ZCL_OTAClusterServer
  (
  zbApsdeDataIndication_t *pIndication, 
  afDeviceDef_t *pDevice
  )
{
    zclFrame_t *pFrame;
    zclOTANextImageRequest_t      *pNextImageReq;
    zclZtcOTANextImageRequest_t   ztcNextImageReq;
    zclOTABlockRequest_t          *pBlockRequest;
    zclZtcOTABlockRequest_t       ztcBlockRequest;
    zclOTAUpgradeEndRequest_t     *pUpgradeEndRequest;
    zclZtcOTAUpgradeEndRequest_t  ztcUpgradeEndRequest;
    zbStatus_t status = gZclSuccess_c;    

    //Parameter not used, avoid compiler warning
    (void)pDevice;
    
    //ZCL frame
    pFrame = (zclFrame_t*)pIndication->pAsdu;

    //Hande the commands
    switch(pFrame->command)
    {
      case gZclCmdOTA_QueryNextImageRequest_c:
        //Pack the needed info and send it up to ZTC
        //Src address information. It is always a 16 bit NWK address
      	Copy2Bytes(&ztcNextImageReq.aNwkAddr,&pIndication->aSrcAddr);
        ztcNextImageReq.endPoint = pIndication->srcEndPoint;
        //Copy the actual request
        pNextImageReq = (zclOTANextImageRequest_t*)(pIndication->pAsdu + sizeof(zclFrame_t));
        FLib_MemCpy(&ztcNextImageReq.zclOTANextImageRequest, pNextImageReq, sizeof(zclOTANextImageRequest_t));
        
        //Send the request up

#ifndef gHostApp_d  
        ZTCQueue_QueueToTestClient((const uint8_t*)&ztcNextImageReq, gHaZtcOpCodeGroup_c, gOTAQueryNextImageRequest_c, sizeof(zclZtcOTANextImageRequest_t));
#else
        ZTCQueue_QueueToTestClient(gpHostAppUart, (const uint8_t*)&ztcNextImageReq, gHaZtcOpCodeGroup_c, gOTAQueryNextImageRequest_c, sizeof(zclZtcOTANextImageRequest_t));
#endif        
        break;
      case gZclCmdOTA_ImageBlockRequest_c:
        //Pack the needed info and send it up to ZTC
        //Src address information. It is always a 16 bit NWK address
      	Copy2Bytes(&ztcBlockRequest.aNwkAddr,&pIndication->aSrcAddr);
        ztcBlockRequest.endPoint = pIndication->srcEndPoint;
        //Copy the actual request
        pBlockRequest = (zclOTABlockRequest_t*)(pIndication->pAsdu + sizeof(zclFrame_t));
        FLib_MemCpy(&ztcBlockRequest.zclOTABlockRequest, pBlockRequest, sizeof(zclOTABlockRequest_t));
        //Send the request up
#ifndef gHostApp_d  
        ZTCQueue_QueueToTestClient((const uint8_t*)&ztcBlockRequest, gHaZtcOpCodeGroup_c, gOTABlockRequest_c, sizeof(zclZtcOTABlockRequest_t));
#else
        ZTCQueue_QueueToTestClient(gpHostAppUart, (const uint8_t*)&ztcBlockRequest, gHaZtcOpCodeGroup_c, gOTABlockRequest_c, sizeof(zclZtcOTABlockRequest_t));
#endif        
        break;
    case gZclCmdOTA_UpgradeEndRequest_c:
        //Pack the needed info and send it up to ZTC
        //Src address information. It is always a 16 bit NWK address
        Copy2Bytes(&ztcUpgradeEndRequest.aNwkAddr, &pIndication->aSrcAddr);
        ztcUpgradeEndRequest.endPoint = pIndication->srcEndPoint;
        //Copy the actual request
        pUpgradeEndRequest = (zclOTAUpgradeEndRequest_t*)(pIndication->pAsdu + sizeof(zclFrame_t));
        FLib_MemCpy(&ztcUpgradeEndRequest.zclOTAUpgradeEndRequest, pUpgradeEndRequest, sizeof(zclOTAUpgradeEndRequest_t));
        //Send the request up
#ifndef gHostApp_d  
        ZTCQueue_QueueToTestClient((const uint8_t*)&ztcUpgradeEndRequest, gHaZtcOpCodeGroup_c, gOTAUpgradeEndRequest_c, sizeof(zclOTAUpgradeEndRequest_t));
#else
        ZTCQueue_QueueToTestClient(gpHostAppUart, (const uint8_t*)&ztcUpgradeEndRequest, gHaZtcOpCodeGroup_c, gOTAUpgradeEndRequest_c, sizeof(zclOTAUpgradeEndRequest_t));
#endif        
        break;
        // command not supported on this cluster
      default:
        return gZclUnsupportedClusterCommand_c;
    }
    return status;
}

/******************************************************************************
* Request to send the image notify command
*
* Usually called by the ZTC/host
******************************************************************************/

zbStatus_t ZCL_OTAImageNotify(zclZtcOTAImageNotify_t* pZtcImageNotifyParams)
{
  afToApsdeMessage_t *pMsg;
  afAddrInfo_t addrInfo = {gZbAddrMode16Bit_c, {0x00, 0x00}, 0, {gaZclClusterOTA_c}, 0, gApsTxOptionAckTx_c, 1};
  zclFrameControl_t frameControl;
  uint8_t len;  
  
  //Create the destination address
	Copy2Bytes(&addrInfo.dstAddr,&pZtcImageNotifyParams->aNwkAddr);
  addrInfo.dstEndPoint = pZtcImageNotifyParams->endPoint;
  addrInfo.srcEndPoint = pZtcImageNotifyParams->endPoint;

  //Initialize the lenght of the command based on the payload type.
  if(pZtcImageNotifyParams->zclOTAImageNotify.payloadType < gPayloadMax_c)
  {
    len = imgNotifySizeArray[pZtcImageNotifyParams->zclOTAImageNotify.payloadType];
  }
  else
  {
    return gZclFailure_c;
  }
  if(!IsValidNwkUnicastAddr(addrInfo.dstAddr.aNwkAddr))
  {
    frameControl = gZclFrameControl_FrameTypeSpecific | gZclFrameControl_DirectionRsp | gZclFrameControl_DisableDefaultRsp;
  }
  else
  {
    frameControl = gZclFrameControl_FrameTypeSpecific | gZclFrameControl_DirectionRsp;
  }

  pMsg = ZCL_CreateFrame( &addrInfo, 
                          gZclCmdOTA_ImageNotify_c,
                          frameControl, 
                          NULL, 
                          &len,
                          &pZtcImageNotifyParams->zclOTAImageNotify);
  if(!pMsg)
    return gZclNoMem_c;
 return ZCL_DataRequestNoCopy(&addrInfo, len, pMsg);  
}

/******************************************************************************
* Request to send the image notify command
*
* Incoming OTA cluster frame processing loop for cluster client side
******************************************************************************/
zbStatus_t ZCL_OTANextImageResponse(zclZtcOTANextImageResponse_t* pZtcNextImageResponseParams)
{
  afToApsdeMessage_t *pMsg;
  afAddrInfo_t addrInfo = {gZbAddrMode16Bit_c, {0x00, 0x00}, 0, {gaZclClusterOTA_c}, 0, gApsTxOptionAckTx_c, 1};
  uint8_t len = sizeof(zclOTANextImageResponse_t);  
  
  //Create the destination address
  Copy2Bytes(&addrInfo.dstAddr,&pZtcNextImageResponseParams->aNwkAddr);
  addrInfo.dstEndPoint = pZtcNextImageResponseParams->endPoint;
  addrInfo.srcEndPoint = pZtcNextImageResponseParams->endPoint;
  
  pMsg = ZCL_CreateFrame( &addrInfo, 
                          gZclCmdOTA_QueryNextImageResponse_c,
                          gZclFrameControl_FrameTypeSpecific | gZclFrameControl_DirectionRsp | gZclFrameControl_DisableDefaultRsp, 
                          NULL, 
                          &len,
                          &pZtcNextImageResponseParams->zclOTANextImageResponse);
  if(!pMsg)
    return gZclNoMem_c;
  return ZCL_DataRequestNoCopy(&addrInfo, len, pMsg);  
}

/******************************************************************************
* ZCL_OTABlockResponse
*
* Send a data block to the client
******************************************************************************/
zbStatus_t ZCL_OTABlockResponse(zclZtcOTABlockResponse_t *pZtcBlockResponseParams)
{
  afToApsdeMessage_t *pMsg;
  afAddrInfo_t addrInfo = {gZbAddrMode16Bit_c, {0x00, 0x00}, 0, {gaZclClusterOTA_c}, 0, gApsTxOptionAckTx_c, 1};
  uint8_t len;   
  
  //Create the destination address
  Copy2Bytes(&addrInfo.dstAddr,&pZtcBlockResponseParams->aNwkAddr);
  addrInfo.dstEndPoint = pZtcBlockResponseParams->endPoint;
  addrInfo.srcEndPoint = pZtcBlockResponseParams->endPoint;
  
  //The lenght of the frame is dependant on the status
  switch(pZtcBlockResponseParams->zclOTABlockResponse.status)
  {
  case gZclOTASuccess_c:
    len = sizeof(zclOTABlockResponse_t) + pZtcBlockResponseParams->zclOTABlockResponse.msgData.success.dataSize - 1;
    break;
  case gZclOTAAbort_c:
    len = sizeof(zclOTAStatus_t);
    break;
  case gZclOTAWaitForData_c:
    len = sizeof(zclOTABlockResponseStatusWait_t) + 1;
    break;
  default:
    //Invalid status
    return gZclMalformedCommand_c;
  }
  
  pMsg = ZCL_CreateFrame( &addrInfo, 
                          gZclCmdOTA_ImageBlockResponse_c,
                          gZclFrameControl_FrameTypeSpecific | gZclFrameControl_DirectionRsp | gZclFrameControl_DisableDefaultRsp, 
                          NULL, 
                          &len,
                          &pZtcBlockResponseParams->zclOTABlockResponse);
  if(!pMsg)
    return gZclNoMem_c;
  return ZCL_DataRequestNoCopy(&addrInfo, len, pMsg);  
}

/******************************************************************************
* Request to send upgrade end response command
*
* Send the response to client's upgrade end request
******************************************************************************/
zbStatus_t ZCL_OTAUpgradeEndResponse(ztcZclOTAUpgdareEndResponse_t* pZtcOTAUpgdareEndResponse)
{
  afToApsdeMessage_t *pMsg;
  afAddrInfo_t addrInfo = {gZbAddrMode16Bit_c, {0x00, 0x00}, 0, {gaZclClusterOTA_c}, 0, gApsTxOptionAckTx_c, 1};
  uint8_t len = sizeof(zclOTAUpgradeEndResponse_t);  
  
  //Create the destination address
	Copy2Bytes(&addrInfo.dstAddr,&pZtcOTAUpgdareEndResponse->aNwkAddr);
  addrInfo.dstEndPoint = pZtcOTAUpgdareEndResponse->endPoint;
  addrInfo.srcEndPoint = pZtcOTAUpgdareEndResponse->endPoint;
  
  pMsg = ZCL_CreateFrame( &addrInfo, 
                          gZclCmdOTA_UpgradeEndResponse_c,
                          gZclFrameControl_FrameTypeSpecific | gZclFrameControl_DirectionRsp | gZclFrameControl_DisableDefaultRsp, 
                          NULL, 
                          &len,
                          &pZtcOTAUpgdareEndResponse->zclOTAUpgradeEndResponse);
  if(!pMsg)
    return gZclNoMem_c;
  return ZCL_DataRequestNoCopy(&addrInfo, len, pMsg);  
}
#endif

/*****************************************************************************
******************************************************************************
*
* ZCL OTA CLIENT FUNCTIONS
*
******************************************************************************
*****************************************************************************/

#if gZclEnableOTAClient_d


/******************************************************************************
* ZCL_OTAClusterClient_EndUpgradeRequest
*
* Sends upgrade end request to server
******************************************************************************/  

zbStatus_t ZCL_OTAClusterClient_EndUpgradeRequest
(
  zbNwkAddr_t   aNwkAddr,
  zbEndPoint_t  endPoint,
  zclOTAStatus_t status
) 
{
  zclZtcOTAUpgradeEndRequest_t upgradeEndRequest;
  
  Copy2Bytes(&upgradeEndRequest.aNwkAddr, aNwkAddr);
  upgradeEndRequest.endPoint = endPoint;
  upgradeEndRequest.zclOTAUpgradeEndRequest.status = status;
  upgradeEndRequest.zclOTAUpgradeEndRequest.manufacturerCode = Native2OTA16(clientParams.manufacturerCode);
  upgradeEndRequest.zclOTAUpgradeEndRequest.imageType = Native2OTA16(clientParams.imageType);            
  upgradeEndRequest.zclOTAUpgradeEndRequest.fileVersion = Native2OTA32(clientSession.downloadingFileVersion);
  return ZCL_OTAUpgradeEndRequest(&upgradeEndRequest);
}

/******************************************************************************
* ZCL_OTAClusterClient_EndUpgradeAbortRequest
*
* Aborts current client session and sends end request with Abort status to server
******************************************************************************/  
zbStatus_t ZCL_OTAClusterClient_EndUpgradeAbortRequest
(
  zbNwkAddr_t   aNwkAddr,
  zbEndPoint_t  endPoint    
) 
{
  uint8_t result = ZCL_OTAClusterClient_EndUpgradeRequest(aNwkAddr, endPoint, gZclOTAAbort_c);
  OTAClusterClientAbortSession();
  if (result != gZclSuccess_c)
    return gZclOTAAbort_c;
  
  return gZclSuccess_c;
}
  

/******************************************************************************
* ZCL_OTAClusterClient_NextImageRequest
*
* Sends back a next image request (may be as a result of Image Notify)
******************************************************************************/  

zbStatus_t ZCL_OTAClusterClient_NextImageRequest 
(
  zbNwkAddr_t   aNwkAddr,
  zbEndPoint_t  endPoint    
)
{
  zclZtcOTANextImageRequest_t nextImageRequest;     
  
  //Send back the image request.
	Copy2Bytes(&nextImageRequest.aNwkAddr, aNwkAddr);
  nextImageRequest.endPoint = endPoint;
  nextImageRequest.zclOTANextImageRequest.fieldControl = gZclOTANextImageRequest_HwVersionPresent;
  nextImageRequest.zclOTANextImageRequest.manufacturerCode = Native2OTA16(clientParams.manufacturerCode);
  nextImageRequest.zclOTANextImageRequest.imageType = Native2OTA16(clientParams.imageType);
  nextImageRequest.zclOTANextImageRequest.fileVersion = Native2OTA32(clientParams.currentFileVersion);
  nextImageRequest.zclOTANextImageRequest.hardwareVersion = Native2OTA16(clientParams.hardwareVersion);
  return ZCL_OTAImageRequest(&nextImageRequest);  
}


/******************************************************************************
* ZCL_OTAClusterClient_ImageNotify_Handler
*
* Handles the receipt of an ImageNotify Command on the client
******************************************************************************/  
zbStatus_t ZCL_OTAClusterClient_ImageNotify_Handler 
(
  zbApsdeDataIndication_t *pIndication
)
{
  zclOTAImageNotify_t *pImageNotify;
  uint8_t frameLen;
  uint8_t clientJitter;
  zclStatus_t result;
  bool_t  dropPacket = FALSE;
  
  bool_t isUnicast = IsValidNwkUnicastAddr(pIndication->aDstAddr);
 
  pImageNotify =  (zclOTAImageNotify_t *)(pIndication->pAsdu + sizeof(zclFrame_t));
  frameLen = pIndication->asduLength - sizeof(zclFrame_t); 

  // invalid payload type or invalid data length for specified payload type data
  if (frameLen < sizeof(pImageNotify->payloadType) ||
      pImageNotify->payloadType >= gPayloadMax_c ||
      frameLen != imgNotifySizeArray[pImageNotify->payloadType])
    return  gZclMalformedCommand_c;
    
  // unicast
  if (isUnicast) 
  {
      // error cases
      if ((pImageNotify->payloadType != gJitter_c) || 
          (pImageNotify->queryJitter != OTA_CLIENT_MAX_RANDOM_JITTER))
      return  gZclMalformedCommand_c;
    else {
      // send back query next image
      result = ZCL_OTAClusterClient_NextImageRequest(pIndication->aSrcAddr, pIndication->srcEndPoint);
      if (result != gZclSuccess_c)
        result = gZclOTAAbort_c;
        return result;
    }
  }

  // broadcast/multicast
  
  // validate jitter  
  if (pImageNotify->queryJitter > OTA_CLIENT_MAX_RANDOM_JITTER)
    dropPacket = TRUE;
  else
    // compute random jitter
    clientJitter = GetRandomRange(0, OTA_CLIENT_MAX_RANDOM_JITTER);
  
  // validate manufacturer
  if (pImageNotify->payloadType > gJitter_c) 
  {
    pImageNotify->manufacturerCode = OTA2Native16(pImageNotify->manufacturerCode);
    if (pImageNotify->manufacturerCode != clientParams.manufacturerCode &&
        pImageNotify->manufacturerCode != OTA_MANUFACTURER_CODE_MATCH_ALL)
        dropPacket = TRUE;
  }
  
  // validate imageType
  if (pImageNotify->payloadType > gJitterMfg_c) 
  {
    pImageNotify->imageType = OTA2Native16(pImageNotify->imageType);
    if (pImageNotify->imageType != clientParams.imageType &&
        pImageNotify->imageType != OTA_IMAGE_TYPE_MATCH_ALL)
        dropPacket = TRUE;
  }
  
  // validate fileVersion
  if (pImageNotify->payloadType > gJitterMfgImage_c) 
  {
    pImageNotify->fileVersion = OTA2Native32(pImageNotify->fileVersion);
    if (pImageNotify->fileVersion == clientParams.currentFileVersion)
        dropPacket = TRUE;
  }
  
  
  if (!dropPacket &&  clientJitter <= pImageNotify->queryJitter)
    // send jitter message
    {
      result = ZCL_OTAClusterClient_NextImageRequest(pIndication->aSrcAddr, pIndication->srcEndPoint);   
      if (result != gZclSuccess_c)
        result = gZclOTAAbort_c;
        return result;      
    }
  else
    // no further processing
    return gZclSuccess_c; 
  
}


/******************************************************************************
* ZCL_OTAClusterClient_NextBlockRequest
*
* Sends back a next image request (may be as a result of Image Notify)
******************************************************************************/  

zbStatus_t ZCL_OTAClusterClient_NextBlockRequest 
(
  zbNwkAddr_t   aNwkAddr,
  zbEndPoint_t  endPoint    
)
{ 
  zclZtcOTABlockRequest_t     blockRequest;

  Copy2Bytes(&blockRequest.aNwkAddr,aNwkAddr);
  blockRequest.endPoint = endPoint;
  blockRequest.zclOTABlockRequest.fieldControl = 0x00;
  blockRequest.zclOTABlockRequest.manufacturerCode = Native2OTA16(clientParams.manufacturerCode);
  blockRequest.zclOTABlockRequest.imageType = Native2OTA16(clientParams.imageType);
  blockRequest.zclOTABlockRequest.fileVersion = Native2OTA32(clientSession.downloadingFileVersion);
  blockRequest.zclOTABlockRequest.fileOffset = Native2OTA32(clientSession.currentOffset);
  blockRequest.zclOTABlockRequest.maxDataSize = clientParams.maxDataSize;
  return ZCL_OTABlockRequest(&blockRequest);
}

/******************************************************************************
* ZCL_OTAClusterClient_NextImageRequest
*
* Sends back a next image request (may be as a result of Image Notify)
******************************************************************************/  

zbStatus_t ZCL_OTAClusterClient_NextImageResponse_Handler
(
  zbApsdeDataIndication_t *pIndication
)
{
  zclOTANextImageResponse_t   *pNextImageResponse;
  uint8_t result;

  pNextImageResponse = (zclOTANextImageResponse_t *)(pIndication->pAsdu + sizeof(zclFrame_t));
  
  // status is successful
  if(pNextImageResponse->status == gZclOTASuccess_c)
  {
    // validate frame length
    if (pIndication->asduLength - sizeof(zclFrame_t) != sizeof(zclOTANextImageResponse_t))
      return gZclMalformedCommand_c;
    
    pNextImageResponse->manufacturerCode = OTA2Native16(pNextImageResponse->manufacturerCode);
    pNextImageResponse->imageType = OTA2Native16(pNextImageResponse->imageType);
    pNextImageResponse->fileVersion = OTA2Native32(pNextImageResponse->fileVersion);
    pNextImageResponse->imageSize = OTA2Native32(pNextImageResponse->imageSize);    
    
    // validate frame params
    if ((pNextImageResponse->manufacturerCode != clientParams.manufacturerCode &&
        pNextImageResponse->manufacturerCode != OTA_MANUFACTURER_CODE_MATCH_ALL) ||
        (pNextImageResponse->imageType != clientParams.imageType &&
        pNextImageResponse->imageType != OTA_IMAGE_TYPE_MATCH_ALL ))   
      return gZclMalformedCommand_c;
    
    // begin client session
    result = OTAClusterClientStartSession(pNextImageResponse->imageSize, pNextImageResponse->fileVersion);
    if(result != gZbSuccess_c) return gZclOTAAbort_c;

    // send the first block request    
    result = ZCL_OTAClusterClient_NextBlockRequest(pIndication->aSrcAddr, pIndication->srcEndPoint);
    if(result != gZclSuccess_c) {
      return ZCL_OTAClusterClient_EndUpgradeAbortRequest(pIndication->aSrcAddr, pIndication->srcEndPoint);
    }
    return  gZclSuccess_c;
  }
  else
  {
    // no further processing
    return gZclSuccess_c;
  } 
}

/******************************************************************************
* ZCL_OTAClusterClient_BlockResponse_Handler
*
* Handles the block response indication
******************************************************************************/  

zbStatus_t ZCL_OTAClusterClient_BlockResponse_Handler
(
  zbApsdeDataIndication_t *pIndication
) 
{
  zclOTABlockResponse_t *pBlockResponse;

  uint8_t result;
  
  //Handle succes status received
  //Verify the received parameters
  pBlockResponse = (zclOTABlockResponse_t *)(pIndication->pAsdu + sizeof(zclFrame_t));

  if(pBlockResponse->status == gZclOTASuccess_c)
  {
      // validate frame length
    if (pIndication->asduLength - sizeof(zclFrame_t) != 
        sizeof(zclOTAStatus_t) + MbrOfs(zclOTABlockResponseStatusSuccess_t, data) + 
        pBlockResponse->msgData.success.dataSize)
      return gZclMalformedCommand_c;
      
    // command fields
    pBlockResponse->msgData.success.manufacturerCode = OTA2Native16(pBlockResponse->msgData.success.manufacturerCode);
    pBlockResponse->msgData.success.imageType = OTA2Native16(pBlockResponse->msgData.success.imageType);
    pBlockResponse->msgData.success.fileVersion = OTA2Native32(pBlockResponse->msgData.success.fileVersion);
    pBlockResponse->msgData.success.fileOffset = OTA2Native32(pBlockResponse->msgData.success.fileOffset);
          
    // Error cases. Send back malformed command indication - no abort of the current upgrade executed.
    if((pBlockResponse->msgData.success.manufacturerCode != clientParams.manufacturerCode)||
       (pBlockResponse->msgData.success.imageType != clientParams.imageType)||
       (pBlockResponse->msgData.success.fileVersion != clientSession.downloadingFileVersion) ||
       (pBlockResponse->msgData.success.dataSize > clientParams.maxDataSize ) ||
       (pBlockResponse->msgData.success.dataSize + clientSession.currentOffset > clientSession.fileLength)) 
    {  
      return gZclMalformedCommand_c;
    }
    // handle out of sync packets by repeating the request (spec does not handle this as error)
    else if ( pBlockResponse->msgData.success.fileOffset != clientSession.currentOffset ) 
    {
      // send the first block request    
      result = ZCL_OTAClusterClient_NextBlockRequest(pIndication->aSrcAddr, pIndication->srcEndPoint);
      if(result != gZclSuccess_c) 
      {
        return ZCL_OTAClusterClient_EndUpgradeAbortRequest(pIndication->aSrcAddr, pIndication->srcEndPoint);
      }
      return  gZclSuccess_c;
    }
    else 
    {
      // Handle the received data chunk - push it to the image storage if we have a session started
      if(clientSession.sessionStarted == TRUE)
      {
        // do it on a timer to handle processing aps and 
        // writing to external memory sync issues
        gBlockCallbackTimer = TMR_AllocateTimer();
        gpBlockCallbackState = AF_MsgAlloc();

        if (gpBlockCallbackState && gBlockCallbackTimer) 
        {             
           Copy2Bytes(&gpBlockCallbackState->dstAddr, &pIndication->aSrcAddr);
           gpBlockCallbackState->dstEndpoint = pIndication->srcEndPoint;
           gpBlockCallbackState->blockSize = pBlockResponse->msgData.success.dataSize;
           FLib_MemCpy(gpBlockCallbackState->blockData,
                       pBlockResponse->msgData.success.data, 
                       pBlockResponse->msgData.success.dataSize);
           
           TMR_StartTimer(gBlockCallbackTimer, gTmrSingleShotTimer_c,
                          BLOCK_PROCESSING_CALLBACK_DELAY, OTAClusterClientProcessBlockTimerCallback);
                          
           return gZclSuccess_c;
        } 
        else 
        {
          return ZCL_OTAClusterClient_EndUpgradeAbortRequest(pIndication->aSrcAddr, pIndication->srcEndPoint);
        }
      }
      else 
      {
        return ZCL_OTAClusterClient_EndUpgradeAbortRequest(pIndication->aSrcAddr, pIndication->srcEndPoint);
      }
    }
  }
    else if(pBlockResponse->status == gZclOTAWaitForData_c)
    {
       return  gZclSuccess_c;
      // TMR_StartSecondTimer(gBlockCallbackTimer, OTA2Native32(pBlockResponse->msgData.wait.requestTime) -
        //              OTA2Native32(pBlockResponse->msgData.wait.currentTime), OTAClusterRepeatBlockReqTimerCallback);
    }
    else if(pBlockResponse->status == gZclOTAAbort_c)
    {
        OTAClusterClientAbortSession();
        return gZclSuccess_c;
    }
    else //unknown status
      //Error detected. Send back malformed command indication - no abort of the current upgrade executed.
      return gZclMalformedCommand_c;

}
  


/******************************************************************************
* ZCL_OTAClusterClient_UpgradeEndResponse_Handler
*
* Handles the upgrade end response indication
******************************************************************************/  

zbStatus_t ZCL_OTAClusterClient_UpgradeEndResponse_Handler
(
  zbApsdeDataIndication_t *pIndication
) 

{
  zclOTAUpgradeEndResponse_t *pUpgradeEndResponse;
    pUpgradeEndResponse = (zclOTAUpgradeEndResponse_t*)(pIndication->pAsdu + sizeof(zclFrame_t));
    pUpgradeEndResponse->manufacturerCode = OTA2Native16(pUpgradeEndResponse->manufacturerCode);
    pUpgradeEndResponse->imageType = OTA2Native16(pUpgradeEndResponse->imageType);
    pUpgradeEndResponse->fileVersion = OTA2Native32(pUpgradeEndResponse->fileVersion);
    pUpgradeEndResponse->currentTime = OTA2Native32(pUpgradeEndResponse->currentTime);
    pUpgradeEndResponse->upgradeTime = OTA2Native32(pUpgradeEndResponse->upgradeTime);
      
    //Verify the data received in the response  
    if((pUpgradeEndResponse->manufacturerCode != clientParams.manufacturerCode)||
       (pUpgradeEndResponse->imageType != clientParams.imageType)||
       (pUpgradeEndResponse->fileVersion != clientSession.downloadingFileVersion))
    {
      return gZclMalformedCommand_c;
    }
    else
    {
      uint32_t offset = pUpgradeEndResponse->upgradeTime - pUpgradeEndResponse->currentTime;

      if(pUpgradeEndResponse->upgradeTime != 0xFFFFFFFF)
      {
        // allow at least 1 second before reset
        if (offset == 0) offset++;
        // OTATODO this does not accept 32 bit second timers
        gBlockCallbackTimer = TMR_AllocateSecondTimer();
        TMR_StartSecondTimer(gBlockCallbackTimer, (uint16_t)offset, OTAClusterCPUResetCallback);        
      } 
      else 
      {
        // OTATODO do the next request in 60 minutes
      }
        
      return gZclSuccess_c;
    }

}


/******************************************************************************
* OTAClusterResetCallback
*
* Resets the node to upgrade after Upgrade End Response
******************************************************************************/
static void OTAClusterCPUResetCallback(uint8_t tmr) 
{
  (void)tmr;
#ifndef __IAR_SYSTEMS_ICC__
  // S08 platform reset
  PWRLib_Reset();               
#else
  CRM_SoftReset();
#endif  
}


/******************************************************************************
* ZCL_OTAClusterClient
*
* Incoming OTA cluster frame processing loop for cluster client side
******************************************************************************/
zbStatus_t ZCL_OTAClusterClient
(
  zbApsdeDataIndication_t *pIndication, 
  afDeviceDef_t *pDevice
)
{
    zclCmd_t            command;
    zclFrame_t          *pFrame;
    zbStatus_t          result = gZclUnsupportedClusterCommand_c;    

    
    /* prevent compiler warning */
    (void)pDevice;
    pFrame = (void *)pIndication->pAsdu;

    /* handle the command */
    command = pFrame->command;
    switch(command)
    {
      case gZclCmdOTA_ImageNotify_c:
        return ZCL_OTAClusterClient_ImageNotify_Handler(pIndication);
        
      case gZclCmdOTA_QueryNextImageResponse_c:
        return ZCL_OTAClusterClient_NextImageResponse_Handler(pIndication);

      case gZclCmdOTA_ImageBlockResponse_c:
        return ZCL_OTAClusterClient_BlockResponse_Handler(pIndication);
        
      case gZclCmdOTA_UpgradeEndResponse_c:
        return ZCL_OTAClusterClient_UpgradeEndResponse_Handler(pIndication);
      
      default:
        break;
    }
    return result;
}


/******************************************************************************
* ZCL_OTASetClientParams
*
* Interface function to set client parameters
******************************************************************************/

void ZCL_OTASetClientParams(zclOTAClientParams_t* pClientParams)
{
  FLib_MemCpy(&clientParams, pClientParams, sizeof(zclOTAClientParams_t));
}

/******************************************************************************
* ZCL_OTAImageRequest
*
* Request to send a image request
******************************************************************************/
zbStatus_t ZCL_OTAImageRequest(zclZtcOTANextImageRequest_t* pZtcNextImageRequestParams)
{
  afToApsdeMessage_t *pMsg;
  afAddrInfo_t addrInfo = {gZbAddrMode16Bit_c, {0x00, 0x00}, 0, {gaZclClusterOTA_c}, 0, gApsTxOptionAckTx_c, 1};
  uint8_t len = sizeof(zclOTANextImageRequest_t);  
  
  //Create the destination address
	Copy2Bytes(&addrInfo.dstAddr,&pZtcNextImageRequestParams->aNwkAddr);
  addrInfo.dstEndPoint = pZtcNextImageRequestParams->endPoint;
  addrInfo.srcEndPoint = pZtcNextImageRequestParams->endPoint;
  
  pMsg = ZCL_CreateFrame( &addrInfo, 
                          gZclCmdOTA_QueryNextImageRequest_c,
                          gZclFrameControl_FrameTypeSpecific, 
                          NULL, 
                          &len,
                          &pZtcNextImageRequestParams->zclOTANextImageRequest);
  if(!pMsg)
    return gZclNoMem_c;
 return ZCL_DataRequestNoCopy(&addrInfo, len, pMsg);
}

/******************************************************************************
* ZCL_OTABlockRequest
*
* Request to send an image block
******************************************************************************/
zbStatus_t ZCL_OTABlockRequest(zclZtcOTABlockRequest_t *pZtcBlockRequestParams)
{
  afToApsdeMessage_t *pMsg;
  afAddrInfo_t addrInfo = {gZbAddrMode16Bit_c, {0x00, 0x00}, 0, {gaZclClusterOTA_c}, 0, gApsTxOptionAckTx_c, 1};
  uint8_t len = sizeof(zclOTABlockRequest_t);

  //Create the destination address
	Copy2Bytes(&addrInfo.dstAddr,&pZtcBlockRequestParams->aNwkAddr);
  addrInfo.dstEndPoint = pZtcBlockRequestParams->endPoint;
  addrInfo.srcEndPoint = pZtcBlockRequestParams->endPoint;

  pMsg = ZCL_CreateFrame( &addrInfo, 
                          gZclCmdOTA_ImageBlockRequest_c,
                          gZclFrameControl_FrameTypeSpecific, 
                          NULL, 
                          &len,
                          &pZtcBlockRequestParams->zclOTABlockRequest);
  if(!pMsg)
    return gZclNoMem_c;
 return ZCL_DataRequestNoCopy(&addrInfo, len, pMsg);  
}

/******************************************************************************
* ZCL_OTAUpgradeEndRequest
*
* Request to send an upgrade end request
******************************************************************************/
zbStatus_t ZCL_OTAUpgradeEndRequest(zclZtcOTAUpgradeEndRequest_t *pZtcUpgradeEndParams)
{
  afToApsdeMessage_t *pMsg;
  afAddrInfo_t addrInfo = {gZbAddrMode16Bit_c, {0x00, 0x00}, 0, {gaZclClusterOTA_c}, 0, gApsTxOptionAckTx_c, 1};
  uint8_t len;

  //Create the destination address
	Copy2Bytes(&addrInfo.dstAddr,&pZtcUpgradeEndParams->aNwkAddr);
  addrInfo.dstEndPoint = pZtcUpgradeEndParams->endPoint;
  addrInfo.srcEndPoint = pZtcUpgradeEndParams->endPoint;

  len = sizeof(zclOTAUpgradeEndRequest_t);

  
  pMsg = ZCL_CreateFrame( &addrInfo, 
                          gZclCmdOTA_UpgradeEndRequest_c,
                          gZclFrameControl_FrameTypeSpecific, 
                          NULL, 
                          &len,
                          &pZtcUpgradeEndParams->zclOTAUpgradeEndRequest);
  if(!pMsg)
    return gZclNoMem_c;
 return ZCL_DataRequestNoCopy(&addrInfo, len, pMsg);  
}

/******************************************************************************
* OTAClusterClientAbortSession
*
* Private function. Abort a started image download session
******************************************************************************/
void OTAClusterClientAbortSession()
{
  clientSession.sessionStarted = FALSE;
  clientSession.fileLength = 0;
  clientSession.currentOffset = 0;  
  if(clientSession.pStateBuffer != NULL)
  {
    MSG_Free(clientSession.pStateBuffer);
    clientSession.pStateBuffer = NULL;
  }
}

/******************************************************************************
* OTAClusterClientStartSession
*
* Private function. Starts an image download session.
******************************************************************************/

zbStatus_t OTAClusterClientStartSession(uint32_t fileLength, uint32_t fileVersion)
{
  //Download file management
  clientSession.sessionStarted = TRUE;
  clientSession.fileLength = fileLength;
  clientSession.downloadingFileVersion = fileVersion;
  clientSession.currentOffset = 0;  
  //Buffer management and state initialization for image file stream processing
  clientSession.state = init_c;
  clientSession.bytesNeededForState = HEADER_LEN_OFFSET + sizeof(uint16_t);
  clientSession.stateBufferIndex = 0;
  if(clientSession.pStateBuffer == NULL)
  {
    clientSession.pStateBuffer = MSG_Alloc(SESSION_BUFFER_SIZE);
  }
  if(clientSession.pStateBuffer == NULL) return gZbNoMem_c;
  return gZbSuccess_c;
}




/******************************************************************************
* OTAClusterClientRunImageProcessStateMachine
*
* Private function. Process a block of received data.
******************************************************************************/
zbStatus_t OTAClusterClientRunImageProcessStateMachine()
{
  zclOTAFileHeader_t* pHeader;
  zclOTAFileSubElement_t* pSubElement;
  static uint32_t subElementLen;
  uint16_t headerLen;
  
  
  switch(clientSession.state)
  {
  case init_c:  
    //In the init state we only extract the header length and move to the next state.
    //The bytes received so far are not consumed
    headerLen = *(uint16_t*)(clientSession.pStateBuffer + HEADER_LEN_OFFSET);
    clientSession.bytesNeededForState = (uint8_t)OTA2Native16(headerLen);
    clientSession.state = processHeader_c;
    break;
  case processHeader_c:
    pHeader = (zclOTAFileHeader_t*)clientSession.pStateBuffer;
    //Check the header for consistency
    if((pHeader->headerVersion != gZclOtaHeaderVersion_c)||(OTA2Native16(pHeader->imageType) != clientParams.imageType)||
       (OTA2Native16(pHeader->manufacturerCode) != clientParams.manufacturerCode))
    {
      return gZbFailed_c;
    }
    //check the field control for supported features - upgrade file destination and security credential version not supported 
    if((OTA2Native16(pHeader->fieldControl) & SECURITY_CREDENTIAL_VERSION_PRESENT)||(OTA2Native16(pHeader->fieldControl) & DEVICE_SPECIFIC_FILE))
    {
      return gZbFailed_c;
    }
    //If HW version is specified, verify it against our own
    if(OTA2Native16(pHeader->fieldControl) & HARDWARE_VERSION_PRESENT)
    {
      if(!((OTA2Native16(pHeader->minHWVersion) <= clientParams.hardwareVersion)&&(clientParams.hardwareVersion <= OTA2Native16(pHeader->maxHWVersion))))
      {
        return gZbFailed_c;
      }
    }
    //If we got here it means we are ready for the upgrade image tag.
    //All bytes from the buffer have been processed. The next state requires receiving a sub-element
    clientSession.state = processUpgradeImageTag_c;
    clientSession.bytesNeededForState = sizeof(zclOTAFileSubElement_t);
    clientSession.stateBufferIndex = 0;
  break;
  case processUpgradeImageTag_c:
    pSubElement = (zclOTAFileSubElement_t*)clientSession.pStateBuffer;
    //Check that we have the expected image sub element
    if(pSubElement->id != gZclOtaUpgradeImageTagId_c)
    {
        return gZbFailed_c;
    }
    //All OK, get the image length
    subElementLen = OTA2Native32(pSubElement->length);
    //Start the FLASH upgrade process
    if(OTA_StartImage(subElementLen) != gOtaSucess_c)
    {
      return gZbFailed_c;
    }     
    //Prepare for next state
    clientSession.state = processUpgradeImage_c;
    
    clientSession.bytesNeededForState = (uint8_t)((subElementLen > SESSION_BUFFER_SIZE) ? SESSION_BUFFER_SIZE : subElementLen);
    clientSession.stateBufferIndex = 0;
    break;
  case processUpgradeImage_c:
    //New image chunk arrived. upgradeImageLen is updated by the OTA platform component.
    if(OTA_PushImageChunk(clientSession.pStateBuffer, clientSession.bytesNeededForState, NULL) != gOtaSucess_c)
    {
      return gZbFailed_c;
    }
    subElementLen-=clientSession.bytesNeededForState;
    //Prepare for next chunk or next state if the image was downloaded
    if(subElementLen != 0)
    {
      //More chuncks to come
      clientSession.bytesNeededForState =  (uint8_t)((subElementLen > SESSION_BUFFER_SIZE) ? SESSION_BUFFER_SIZE : subElementLen);
      clientSession.stateBufferIndex = 0;
    }
    else
    {
      //We need to move to the next state
      clientSession.state = processBitmapTag_c;
      clientSession.bytesNeededForState = sizeof(zclOTAFileSubElement_t);
      clientSession.stateBufferIndex = 0;
    }
    break;
  case processBitmapTag_c:
    pSubElement = (zclOTAFileSubElement_t*)clientSession.pStateBuffer;
    //Check that we have the expected image sub element
    if(pSubElement->id != gZclOtaSectorBitmapTagId_c)
    {
        return gZbFailed_c;
    }
    //Prepare for next state
    clientSession.state = processBitmap_c;
    clientSession.bytesNeededForState =  (uint8_t)OTA2Native32(pSubElement->length); //The bitmap length
    clientSession.stateBufferIndex = 0;
    break;
  case processBitmap_c:
    //We need to close the written image here; commit image has different prototype on ARM7 vs S08
#if (gBigEndian_c != TRUE)
    if(OTA_CommitImage(FALSE, *(uint32_t *)(clientSession.pStateBuffer)) != gOtaSucess_c) return gZbFailed_c;
#else
    if(OTA_CommitImage(clientSession.pStateBuffer) != gOtaSucess_c) return gZbFailed_c;
#endif    
    //Advance to an illegal state. This state machine should not be called again in this upgrade session.
    clientSession.state = stateMax_c;
    clientSession.bytesNeededForState = 1; //The bitmap length
    clientSession.stateBufferIndex = 0;
    break;
  case stateMax_c:
  default:
    return gZbFailed_c;
  }  

  return gZbSuccess_c;
}

/******************************************************************************
* OTAClusterClientProcessBlock
*
* Private function. Process a block of received data.
******************************************************************************/
zbStatus_t OTAClusterClientProcessBlock(uint8_t *pImageBlock, uint8_t blockLength)
{
  uint8_t bytesToCopy;
  uint8_t bytesCopied = 0;
  zbStatus_t result = gZbSuccess_c;
  while(bytesCopied < blockLength)
  {
    bytesToCopy = clientSession.bytesNeededForState - clientSession.stateBufferIndex;
    if(bytesToCopy > (blockLength - bytesCopied))
    {
      bytesToCopy = (blockLength - bytesCopied);
    }
    FLib_MemCpy(clientSession.pStateBuffer + clientSession.stateBufferIndex, pImageBlock + bytesCopied, bytesToCopy);
    bytesCopied +=bytesToCopy;
    clientSession.stateBufferIndex+=bytesToCopy;
    if(clientSession.stateBufferIndex == clientSession.bytesNeededForState)
    {
      result = OTAClusterClientRunImageProcessStateMachine();
      if(result != gZbSuccess_c) return result;
    }
  }
  return result;
}


/******************************************************************************
* OTAClusterClientProcessBlockTimerCallback
*
* Timer callback to process block indications
******************************************************************************/
static void OTAClusterClientProcessBlockTimerCallback(uint8_t tmr) 
{
  zbStatus_t          result = gZclUnsupportedClusterCommand_c;    
  (void) tmr;
                                                               
  result = OTAClusterClientProcessBlock(gpBlockCallbackState->blockData, gpBlockCallbackState->blockSize);
  
  if(result != gZbSuccess_c)
  {
     (void)ZCL_OTAClusterClient_EndUpgradeAbortRequest(gpBlockCallbackState->dstAddr,  gpBlockCallbackState->dstEndpoint);
  }
  //Update the transfer info
  clientSession.currentOffset += gpBlockCallbackState->blockSize;
  if(clientSession.currentOffset < clientSession.fileLength)
  {
    //More data to be received - send back a block request
    (void)ZCL_OTAClusterClient_NextBlockRequest(gpBlockCallbackState->dstAddr, gpBlockCallbackState->dstEndpoint);
  }
  else 
  {
    //Save relevant data on the new image
    clientParams.downloadedFileVersion = clientSession.downloadingFileVersion;
    //All image data received. Issue upgrade end request.
    
    (void)ZCL_OTAClusterClient_EndUpgradeRequest(gpBlockCallbackState->dstAddr, gpBlockCallbackState->dstEndpoint, gZclSuccess_c);    
  }

  TMR_FreeTimer(gBlockCallbackTimer);
  MSG_Free(gpBlockCallbackState);
               
 }

#endif


#endif
