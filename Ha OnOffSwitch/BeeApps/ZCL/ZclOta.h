/******************************************************************************
* ZclOTA.h
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
#ifndef _ZCL_OTA_H
#define _ZCL_OTA_H

#include "AfApsInterface.h"
#include "AppAfInterface.h"
#include "BeeStackInterface.h"
#include "ZCL.h"
#include "ZclOptions.h"

/******************************************************************************
*******************************************************************************
* Public macros and data types definitions.
*******************************************************************************
******************************************************************************/

/******************************************
	OTA Cluster
*******************************************/

enum{
/* 6.7.7	ImageUpgradeStatus Attribute Values */
   gOTAUpgradeStatusNormal_c = 0x00,
   gOTAUpgradeStatusDownloadInProgress_c = 0x01,
   gOTAUpgradeStatusDownloadComplete_c = 0x02,
   gOTAUpgradeStatusWiatingToUpgrade_c = 0x03,
   gOTAUpgradeStatusCountDown_c = 0x04,
   gOTAUpgradeStatusWaitForMore_c = 0x05,
   gOTAUpgradeStatusMax_c = 0x06,
};

/* 6.10.1	OTA Cluster Command Identifiers */
#define gZclCmdOTA_ImageNotify_c                0x00 /* O - Direction 0x01 */
#define gZclCmdOTA_QueryNextImageRequest_c      0x01 /* M - Direction 0x00 */
#define gZclCmdOTA_QueryNextImageResponse_c     0x02 /* M - Direction 0x01 */
#define gZclCmdOTA_ImageBlockRequest_c          0x03 /* M - Direction 0x00 */
#define gZclCmdOTA_ImagePageRequest_c           0x04 /* O - Direction 0x00 */
#define gZclCmdOTA_ImageBlockResponse_c         0x05 /* M - Direction 0x01 */
#define gZclCmdOTA_UpgradeEndRequest_c          0x06 /* M - Direction 0x00 */
#define gZclCmdOTA_UpgradeEndResponse_c         0x07 /* M - Direction 0x01 */
#define gZclCmdOTA_QuerySpecificFileRequest_c   0x08 /* O - Direction 0x00 */
#define gZclCmdOTA_QuerySpecificFileResponse_c  0x09 /* O - Direction 0x01 */

/* 6.7	OTA Cluster Attributes */
#if (TRUE == gBigEndian_c)
#define gZclAttrOTA_UpgradeServerIdId_c               0x0000 /* M - Upgrade Server ID */
#define gZclAttrOTA_FileOffsetId_c                    0x0100 /* O - File Offset */
#define gZclAttrOTA_CurrentFileVersionId_c            0x0200 /* O -  Current File Version */
#define gZclAttrOTA_CurrentZigBeeStackVersionId_c     0x0300 /* O -  Current ZigBee Stack Version */
#define gZclAttrOTA_DownloadedFileVersionId_c         0x0400 /* O -  Downloaded File Version */
#define gZclAttrOTA_DownloadedZigBeeStackVersionId_c  0x0500 /* O -  Downloaded ZigBee Stack Version */
#define gZclAttrOTA_ImageUpgradeStatusId_c            0x0600 /* O -  Image Upgrade Status*/
#else
#define gZclAttrOTA_UpgradeServerIdId_c               0x0000 /* M - Upgrade Server ID */
#define gZclAttrOTA_FileOffsetId_c                    0x0001 /* O - File Offset */
#define gZclAttrOTA_CurrentFileVersionId_c            0x0002 /* O -  Current File Version */
#define gZclAttrOTA_CurrentZigBeeStackVersionId_c     0x0003 /* O -  Current ZigBee Stack Version */
#define gZclAttrOTA_DownloadedFileVersionId_c         0x0004 /* O -  Downloaded File Version */
#define gZclAttrOTA_DownloadedZigBeeStackVersionId_c  0x0005 /* O -  Downloaded ZigBee Stack Version */
#define gZclAttrOTA_ImageUpgradeStatusId_c            0x0006 /* O -  Image Upgrade Status*/
#endif

/* 6.3.2.6	Image Type */
#if (TRUE == gBigEndian_c)
#define gZclImageType_FSLSpecific_c                   0x0000 /* Manufacturer specific - 0x0000 – 0xffbf*/
#define gZclImageType_SecurityCredential_c            0xC0FF /* Security credential */
#define gZclImageType_Configuration_c                 0xC1FF /* Configuration */
#define gZclImageType_Log_c                           0xC2FF /* Log */
#define gZclImageType_WildCard_c                      0xFFFF /* Wild card */
#else
#define gZclImageType_FSLSpecific_c                   0x0000 /* Manufacturer specific - 0x0000 – 0xffbf*/
#define gZclImageType_SecurityCredential_c            0xFFC0 /* Security credential */
#define gZclImageType_Configuration_c                 0xFFC1 /* Configuration */
#define gZclImageType_Log_c                           0xFFC2 /* Log */
#define gZclImageType_WildCard_c                      0xFFFF /* Wild card */
#endif

/* 6.3.2.8	ZigBee Stack Version */
#if (TRUE == gBigEndian_c)
#define gZclStackVer_ZigBee2006_c                         0x0000 /* ZigBee 2006 */
#define gZclStackVer_ZigBee2007_c                         0x0100 /* ZigBee 2007 */
#define gZclStackVer_ZigBeePro_c                          0x0200 /* ZigBee Pro */
#define gZclStackVer_ZigBeeIP_c                           0x0300 /* ZigBee IP */
#else
#define gZclStackVer_ZigBee2006_c                         0x0001 /* ZigBee 2006 */
#define gZclStackVer_ZigBee2007_c                         0x0002 /* ZigBee 2007 */
#define gZclStackVer_ZigBeePro_c                          0x0003 /* ZigBee Pro */
#define gZclStackVer_ZigBeeIP_c                           0x0004 /* ZigBee IP */
#endif

/* USED sub elements tag ID's */
#if (TRUE == gBigEndian_c)
#define gZclOtaSectorBitmapTagId_c	0x00F0
#define gZclOtaUpgradeImageTagId_c	0x0000
#else
#define gZclOtaSectorBitmapTagId_c	0xF000
#define gZclOtaUpgradeImageTagId_c	0x0000
#endif
/* Supported header version*/
#if (TRUE == gBigEndian_c)
#define gZclOtaHeaderVersion_c  0x0001
#else
#define gZclOtaHeaderVersion_c  0x0100
#endif

#ifdef __IAR_SYSTEMS_ICC__
#pragma pack(1)
#endif

typedef struct zclOTAAttrsRAM_tag
{
  int8_t      UpgradeServerId[8];        /* Upgrade server IEEE address */
#if gZclClusterOptionals_d
  uint32_t    FileOffset; 
  uint32_t    CurrentFileVersion;
  uint32_t    DownloadedFileVersion;
  uint16_t    CurrentZigBeeStackVersion;
  uint16_t    DownloadedZigBeeStackVersion;
  uint8_t     ImageUpgradeStatus;
#endif
} zclOTAAttrsRAM_t;

/* 6.8	OTA Server Cluster Parameters */
typedef struct zclOTAServerParams_tag
{
  uint8_t   querryJitter;
  uint8_t   dataSize;
  uint8_t*  pOtaImageData;
  uint32_t  currentTime;
  uint32_t  upgradeRequestTime;
} zclOTAServerParams_t;

/* OTA Client Cluster Parameters */
typedef struct zclOTAClientParams_tag
{
  uint16_t manufacturerCode;
  uint16_t imageType;
  uint32_t currentFileVersion;
  uint32_t downloadedFileVersion;
  uint16_t hardwareVersion;
  uint8_t  maxDataSize;
} zclOTAClientParams_t;

/* OTA Client upgrade session data */
typedef enum clientSessionState_tag
{
  init_c,
  processHeader_c,
  processUpgradeImageTag_c,
  processUpgradeImage_c,
  processBitmapTag_c,
  processBitmap_c,
  stateMax_c
} clientSessionState_t;

typedef struct zclOTAClientSession_tag
{
  bool_t                sessionStarted;
  uint32_t              fileLength;
  uint32_t              currentOffset;
  uint32_t              downloadingFileVersion;
  uint8_t               bytesNeededForState;
  uint8_t*              pStateBuffer;
  uint8_t               stateBufferIndex;  
  clientSessionState_t  state;  
}zclOTAClientSession_t;

typedef struct zclOTABlockCallbackState_tag
{
  uint8_t               dstAddr[2];
  uint8_t               dstEndpoint;
  uint8_t               blockSize;
  uint8_t               blockData[1];
} zclOTABlockCallbackState_t;

//Header control field bit definitions
#define SECURITY_CREDENTIAL_VERSION_PRESENT	(1<<0)
#define DEVICE_SPECIFIC_FILE				(1<<1)
#define HARDWARE_VERSION_PRESENT			(1<<2)

typedef struct zclOTAFileHeader_tag
{
	uint32_t	fileIdentifier;
  uint16_t	headerVersion;
	uint16_t	headerLength;
	uint16_t	fieldControl;
	uint16_t	manufacturerCode;
	uint16_t	imageType;
	uint32_t	fileVersion;
	uint16_t	stackVersion;
	uint8_t	  headerString[32];
	uint32_t	totalImageSize;
  uint16_t  minHWVersion;
  uint16_t  maxHWVersion;
}zclOTAFileHeader_t;

typedef struct zclOTAFileSubElement_tag
{
	uint16_t	id;
	uint32_t  length;
}zclOTAFileSubElement_t;

/* 6.10.2	OTA Cluster Status Codes */
typedef uint8_t zclOTAStatus_t;

#define gZclOTASuccess_c              0x00 /* Success Operation */
#define gZclOTAAbort_c                0x95 /* Failed case when a client or a server decides to abort the upgrade process. */
#define gZclOTANotAuthorized_c        0x7E /* Server is not authorized to upgrade the client. */
#define gZclOTAInvalidImage_c         0x96 /* Invalid OTA upgrade image. */
#define gZclOTAWaitForData_c          0x97 /* Server does not have data block available yet. */
#define gZclOTANoImageAvailable_c     0x98 /* No OTA upgrade image available for a particular client. */
#define gZclOTAMalformedCommand_c     0x80 /* The command received is badly formatted. */
#define gZclOTAUnsupClusterCommand_c  0x81 /* Such command is not supported on the device. */
#define gZclOTARequireMoreImage_c     0x99 /* The client still requires more OTA upgrade image files in order to successfully upgrade. */

/* Commands definition */

/********************************************/
/*                                          */
/* 6.10.3	Image Notify Command              */
/*                                          */
/********************************************/
typedef enum
{
  gJitter_c             = 0x00,
  gJitterMfg_c          = 0x01,
  gJitterMfgImage_c     = 0x02,
  gJitterMfgImageFile_c = 0x03,
  gPayloadMax_c         = 0x04
} zclOTAImageNotifyPayload_t;

typedef struct zclOTAImageNotify_tag {
  zclOTAImageNotifyPayload_t payloadType;
  uint8_t   queryJitter;
  uint16_t  manufacturerCode;
  uint16_t  imageType;
  uint32_t  fileVersion;
} zclOTAImageNotify_t;

/* ZTC support structure for the image notify command */
typedef struct zclZtcOTAImageNotify_tag {
  zbNwkAddr_t   aNwkAddr;
  zbEndPoint_t  endPoint;
  zclOTAImageNotify_t zclOTAImageNotify;
} zclZtcOTAImageNotify_t;

/********************************************/
/*                                          */
/* 6.10.4	Query Next Image Request Command  */
/*                                          */
/********************************************/
#define gZclOTANextImageRequest_HwVersionPresent  0x01

typedef struct zclOTANextImageRequest_tag
{
  uint8_t   fieldControl;
  uint16_t  manufacturerCode;
  uint16_t  imageType;
  uint32_t  fileVersion;
  uint16_t  hardwareVersion;
} zclOTANextImageRequest_t;

/* ZTC support structure for the querry next image request*/
typedef struct zclZtcOTANextImageRequest_tag {
  zbNwkAddr_t   aNwkAddr;
  zbEndPoint_t  endPoint;
  zclOTANextImageRequest_t zclOTANextImageRequest;
} zclZtcOTANextImageRequest_t;

/********************************************/
/*                                          */
/* 6.10.5	Query Next Image Response Command */
/*                                          */
/********************************************/
typedef struct zclOTANextImageResponse_tag
{
  zclOTAStatus_t  status;
  uint16_t        manufacturerCode;
  uint16_t        imageType;
  uint32_t        fileVersion;
  uint32_t        imageSize;
} zclOTANextImageResponse_t;

/* ZTC support structure for the querry next image response*/
typedef struct zclZtcOTANextImageResponse_tag {
  zbNwkAddr_t   aNwkAddr;
  zbEndPoint_t  endPoint;
  zclOTANextImageResponse_t zclOTANextImageResponse;
} zclZtcOTANextImageResponse_t;


/********************************************/
/*                                          */
/* 6.10.6	Image Block Request Command       */
/*                                          */
/********************************************/
#define gZclOTABlockRequest_IeeePresent  0x01

typedef struct zclOTABlockRequest_tag
{
  uint8_t   fieldControl;     /* Always 0 for now*/
  uint16_t  manufacturerCode;
  uint16_t  imageType;
  uint32_t  fileVersion;
  uint32_t  fileOffset;
  uint8_t   maxDataSize;
} zclOTABlockRequest_t;

/* ZTC support structure for the block request*/
typedef struct zclZtcOTABlockRequest_tag
{
  zbNwkAddr_t   aNwkAddr;
  zbEndPoint_t  endPoint;
  zclOTABlockRequest_t  zclOTABlockRequest;
} zclZtcOTABlockRequest_t;

/* 6.10.7	Image Page Request Command */
/* OPTIONAL COMMAND, CURRENTLY NOT SUPPORTED */

/********************************************/
/*                                          */
/* 6.10.8	Image Block Response Command      */
/*                                          */
/********************************************/
typedef struct zclOTABlockResponseStatusWait_tag
{
  uint32_t  currentTime;
  uint32_t  requestTime;
} zclOTABlockResponseStatusWait_t;

typedef struct zclOTABlockResponseStatusSuccess_tag
{
  uint16_t  manufacturerCode;
  uint16_t  imageType;
  uint32_t  fileVersion;
  uint32_t  fileOffset;
  uint8_t   dataSize;
  uint8_t   data[1];
} zclOTABlockResponseStatusSuccess_t;

typedef struct zclOTABlockResponse_tag
{
  zclOTAStatus_t  status;
  union {
    zclOTABlockResponseStatusWait_t     wait;   /* Valid when the command status is "WaitForData" */
    zclOTABlockResponseStatusSuccess_t  success; /* Valid when the command status is "Success" */
  } msgData;
}zclOTABlockResponse_t;

/* ZTC support structure for the block response*/
typedef struct zclZtcOTABlockResponse_tag
{
  zbNwkAddr_t   aNwkAddr;
  zbEndPoint_t  endPoint;
  zclOTABlockResponse_t  zclOTABlockResponse;
} zclZtcOTABlockResponse_t;

/********************************************/
/*                                          */
/* 6.10.9	Upgrade End Request Command       */
/*                                          */
/********************************************/
typedef struct zclOTAUpgradeEndRequest_tag
{
  zclOTAStatus_t  status;
  uint16_t        manufacturerCode;
  uint16_t        imageType;
  uint32_t        fileVersion;
} zclOTAUpgradeEndRequest_t;

/* ZTC support structure for the upgrade end request*/
typedef struct zclZtcOTAUpgradeEndRequest_tag
{
  zbNwkAddr_t   aNwkAddr;
  zbEndPoint_t  endPoint;
  zclOTAUpgradeEndRequest_t zclOTAUpgradeEndRequest;
} zclZtcOTAUpgradeEndRequest_t;


/********************************************/
/*                                          */
/* 6.10.10	Upgrade End Response Command    */
/*                                          */
/********************************************/
typedef struct zclOTAUpgradeEndResponse_tag
{
  uint16_t        manufacturerCode;
  uint16_t        imageType;
  uint32_t        fileVersion;
  uint32_t        currentTime;
  uint32_t        upgradeTime;
} zclOTAUpgradeEndResponse_t;

typedef struct ztcZclOTAUpgradeEndResponse_tag
{
  zbNwkAddr_t   aNwkAddr;
  zbEndPoint_t  endPoint;
  zclOTAUpgradeEndResponse_t zclOTAUpgradeEndResponse;
}ztcZclOTAUpgdareEndResponse_t;

/* 6.10.11	Query Specific File Request Command */
/* OPTIONAL COMMAND, CURRENTLY NOT SUPPORTED */

/* 6.10.12	Query Specific File Response Command */
/* OPTIONAL COMMAND, CURRENTLY NOT SUPPORTED */

/* ZCL ZTC structures */

#ifdef __IAR_SYSTEMS_ICC__
#pragma pack()
#endif
/******************************************************************************
*******************************************************************************
* Public functions prototypes
*******************************************************************************
******************************************************************************/
extern const zclAttrDef_t gaZclOTAClusterAttrDef[];
extern const zclAttrDefList_t gZclOTAClusterAttrDefList;

#if gAddValidationFuncPtrToClusterDef_c
bool_t  ZCL_OTAValidateAttributes(zbEndPoint_t endPoint, zbClusterId_t clusterId, void *pAttrDef);
#else
#define ZCL_OTAValidateAttributes
#endif

zbStatus_t ZCL_OTAClusterServer( zbApsdeDataIndication_t *pIndication, afDeviceDef_t *pDevice);
zbStatus_t ZCL_OTAClusterClient( zbApsdeDataIndication_t *pIndication, afDeviceDef_t *pDevice);
zbStatus_t ZCL_OTAImageNotify(zclZtcOTAImageNotify_t* pZtcImageNotifyParams);
void ZCL_OTASetClientParams(zclOTAClientParams_t* pClientParams);
zbStatus_t ZCL_OTANextImageResponse(zclZtcOTANextImageResponse_t* pZtcNextImageResponseParams);
zbStatus_t ZCL_OTABlockRequest(zclZtcOTABlockRequest_t *pZtcBlockRequestParams);
zbStatus_t ZCL_OTABlockResponse(zclZtcOTABlockResponse_t *pZtcBlockResponseParams);
zbStatus_t ZCL_OTAUpgradeEndResponse(ztcZclOTAUpgdareEndResponse_t* pZtcOTAUpgdareEndResponse);
#endif
