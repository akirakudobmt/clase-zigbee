/******************************************************************************
* ZclSecAndSafe.h
*
* This module contains code that handles ZigBee Cluster Library commands and
* clusters for the Security and safety clusters.
*
* Copyright (c) 2009, Freescale, Inc.  All rights reserved.
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

#ifndef _ZCL_SECANDSAFE_H
#define _ZCL_SECANDSAFE_H

#include "ZclOptions.h"
#include "AfApsInterface.h"
#include "AppAfInterface.h"
#include "BeeStackInterface.h"
#include "EmbeddedTypes.h"
#include "ZCL.h"


/******************************************************************************
*******************************************************************************
* Public macros and data types definitions.
*******************************************************************************
******************************************************************************/


#if ( TRUE == gBigEndian_c )
#define gZclClusterIASZone_c      0x0005    
#define gZclClusterIASACE_c       0x0105  
#define gZclClusterIASWD_c        0x0205  
#else /* #if ( TRUE == gBigEndian_c ) */
#define gZclClusterIASZone_c      0x0500  
#define gZclClusterIASACE_c       0x0501  
#define gZclClusterIASWD_c        0x0502 
#endif



/********************************************************
	IAS Zone Cluster (ZigBee Cluster Library Chapter 8.2)
********************************************************/

#if ( TRUE == gBigEndian_c )
#define gZclAttrIASZoneZoneInformationSet_c   0x0000    /* Zone Information attributes Set */
#define gZclAttrIASZoneZoneSettingsSet_c      0x0100    /* Zone Settings attributes Set */
#else
#define gZclAttrIASZoneZoneInformationSet_c   0x0000    /* Zone Information attributes Set */
#define gZclAttrIASZoneZoneSettingsSet_c      0x0001    /* Zone Settings attributes Set */
#endif /* #if ( TRUE == gBigEndian_c ) */

/* 8.2.2.2.1 Zone Information Attributes Set */
#if ( TRUE == gBigEndian_c )
#define gZclAttrZoneInformationZoneStateSet_c     0x0000    /* M - Zone State attributes */
#define gZclAttrZoneInformationZoneTypeSet_c      0x0100    /* M - Zone Type attributes */
#define gZclAttrZoneInformationZoneStatusSet_c    0x0200    /* M - Zone Status attributes */
#else
#define gZclAttrZoneInformationZoneStateSet_c     0x0000    /* M - Zone State attributes */
#define gZclAttrZoneInformationZoneTypeSet_c      0x0001    /* M - Zone Type attributes */
#define gZclAttrZoneInformationZoneStatusSet_c    0x0002    /* M - Zone Status attributes */
#endif /* #if ( TRUE == gBigEndian_c ) */

/* 8.2.2.2.1.1 Zone State Attributes */
enum
{
   gZclZoneState_NotEnrolled_c = 0x00,       /* Not Enrolled */
   gZclZoneState_Enrolled_c,                 /* Enrolled */
                                             /* 0x02-0xFF Reserved */
};

/* 8.2.2.2.1.2 Zone Type Attributes */

#if ( TRUE == gBigEndian_c )
#define gZclZoneType_StandardCIE_c               0x0000   /* Standard CIE */
#define gZclZoneType_MotionSensor_c              0x0D00   /* Motion Sensor */
#define gZclZoneType_ContactSwitch_c             0x1500   /* Contact Switch */
#define gZclZoneType_FireSensor_c                0x2800   /* Fire Sensor */
#define gZclZoneType_WaterSensor_c               0x2A00   /* Water Sensor */
#define gZclZoneType_GasSensor_c                 0x2B00   /* Gas Sensor */
#define gZclZoneType_PersonalEmergencyDevice_c   0x2C00   /* Personal Emergency Device */
#define gZclZoneType_RemoteControl_c             0x2D00   /* Vibration/Movement Sensor */
#define gZclZoneType_KeyFob_c                    0x0F01   /* Key fob */
#define gZclZoneType_Keypad_c                    0x1501   /* Keypad */
#define gZclZoneType_StandardWarning_c           0x1D02   /* Standard Warning Device */
#define gZclZoneType_InvalidZone_c               0xFFFF   /* Invalid Zone Type */
                                                          /* Other Values < 0x7FFF are Reserved */
                                                          /* 0x8000-0xFFFE Reserved for manufacturer 
                                                              specific types */
#else
#define gZclZoneType_StandardCIE_c               0x0000   /* Standard CIE */
#define gZclZoneType_MotionSensor_c              0x000D   /* Motion Sensor */
#define gZclZoneType_ContactSwitch_c             0x0015   /* Contact Switch */
#define gZclZoneType_FireSensor_c                0x0028   /* Fire Sensor */
#define gZclZoneType_WaterSensor_c               0x002A   /* Water Sensor */
#define gZclZoneType_GasSensor_c                 0x002B   /* Gas Sensor */
#define gZclZoneType_PersonalEmergencyDevice_c   0x002C   /* Personal Emergency Device */
#define gZclZoneType_RemoteControl_c             0x002D   /* Vibration/Movement Sensor */
#define gZclZoneType_KeyFob_c                    0x010F   /* Key fob */
#define gZclZoneType_Keypad_c                    0x0115   /* Keypad */
#define gZclZoneType_StandardWarning_c           0x021D   /* Standard Warning Device */
#define gZclZoneType_InvalidZone_c               0xFFFF   /* Invalid Zone Type */
                                                          /* Other Values < 0x7FFF are Reserved */
                                                          /* 0x8000-0xFFFE Reserved for manufacturer 
                                                              specific types */
#endif /* #if ( TRUE == gBigEndian_c ) */



/* 8.2.2.2.1.3 Zone Status Attributes */

#ifdef __IAR_SYSTEMS_ICC__
#pragma pack(1)
#endif

typedef struct gZclZoneStatus_tag
{
    unsigned Alarm1             :1;                                 
    unsigned Alarm2             :1;                                 
    unsigned Tamper             :1;                                 
    unsigned Battery            :1;                                
    unsigned SupervisionReports :1;                     
    unsigned RestoreReports     :1;                         
    unsigned Trouble            :1;                                
    unsigned ACmains            :1;
    uint8_t Reserved;                                       
}gZclZoneStatus_t;                   
  
#define ZONESTATUS_ALARMED         1
#define ZONESTATUS_NOT_ALARMED     0
#define ZONESTATUS_TAMPERED        1
#define ZONESTATUS_NOT_TAMPERED    0
#define ZONESTATUS_LOW_BATTERY     1
#define ZONESTATUS_BATTERY_OK      0
#define ZONESTATUS_REPORTS         1
#define ZONESTATUS_NOT_REPORT      0
#define ZONESTATUS_FAILURE         1
#define ZONESTATUS_OK              0
                                         
/* 8.2.2.2.2 Zone Settings Attributes Set */
#if ( TRUE == gBigEndian_c )
#define gZclAttrZoneSettingsIASCIEAddress_c     0x1000    /* M - IAS CIE Address attributes */
#else
#define gZclAttrZoneSettingsIASCIEAddress_c     0x0010    /* M - IAS CIE Address attributes *//
#endif /* #if ( TRUE == gBigEndian_c ) */    }

typedef uint16_t IAS_CIE_Address_t;

/* 8.2.2.3 IAS Zone server cluster Commands Received */

#define gZclCmdRxIASZone_ZoneEnrollResponse_c     0x00    /* M - Zone Enroll Response */


typedef struct zclCmdIASZone_ZoneEnrollResponse_tag 
{
 uint8_t  EnrollResponseCode;
 uint8_t  ZoneID;
}zclCmdIASZone_ZoneEnrollResponse_t; 

/* Enroll Response Code Enumeration*/
enum
{
   gEnrollResponseCode_Succes_c = 0x00,   /* Success */
   gEnrollResponseCode_NotSupported_c,    /* Not Supported */
   gEnrollResponseCode_NoEnrollPermit_c,  /* No Enroll Permit */
   gEnrollResponseCode_TooManyZones_c,    /* Too Many Zones */
                                          /* 0x04-0xFE Reserved */
};

/* 8.2.2.4 IAS Zone server cluster Commands Generated */

#define gZclCmdTxIASZone_ZoneStatusChange_c     0x00    /* M - Zone Status Change Notification */
#define gZclCmdTxIASZone_ZoneEnrollRequest_c    0x01    /* M - Zone Enroll Request */


typedef struct zclCmdIASZone_ZoneStatusChange_tag 
{
 uint16_t   ZoneStatus;
 uint8_t    ExtendedStatus;
}zclCmdIASZone_ZoneStatusChange_t; 

typedef struct zclCmdIASZone_ZoneEnrollRequest_tag
{
 uint16_t   ZoneType;
 uint16_t   ManufaturerCode;
}zclCmdIASZone_ZoneEnrollRequest_t;


/******************************************************
	IAS WD Cluster (ZigBee Cluster Library Chapter 8.4)
******************************************************/

/* 8.4.2.2 IAS WD Attributes */
#if ( TRUE == gBigEndian_c )
#define gZclAttrIASWDMaxDuration_c     0x0000    /* M - Max Duration */
#else
#define gZclAttrIASWDMaxDuration_c     0x0000    /* M - Max Duration */
#endif /* #if ( TRUE == gBigEndian_c ) */

/* 8.4.2.3 IAS WD server cluster Commands Received */
enum
{
   gZclCmdRxIASWD_StartWarning_c = 0x00,    /* M - Start Warning */
   gZclCmdRxIASWD_Squawk_c,                 /* M - Squawk */
                                            /* 0x02-0xFF Reserved */
};


typedef struct WarningModeAndStrobe_tag
{
 unsigned   WarningMode : 4;
 unsigned   Strobe : 2;
 unsigned   Reserved : 2;
}WarningModeAndStrobe_t;


/* Start Warning Payload */
typedef struct zclCmdIASWD_StartWarning_tag
{
 WarningModeAndStrobe_t   WarningModeandStrobe;
 uint16_t                 WarningDuration;
}zclCmdIASWD_StartWarning_t; 


enum
{
  WarningMode_Stop = 0,     /* Stop (no warning) */
  WarningMode_Burglar,      /* Burglar */
  WarningMode_Fire,         /* Fire */
  WarningMode_Emergency,    /* Emergency */ 
                            /* 0x04-0xFF Reserved */  
};

enum
{
  Strobe_NoStrobe = 0,      /* No Strobe */
  Strobe_StrobeParallel,    /* Use Strobe in Parallel to warning */
                            /* 2-3 Reserved */
};   


/* Squawk Payload */
typedef struct zclCmdIASWD_Squawk_tag
{
 unsigned   SquawkMode : 4;
 unsigned   Strobe : 1;
 unsigned   Reserved : 1;
 unsigned   SquawkLevel : 2;
}zclCmdIASWD_Squawk_t;  

enum
{
  SquawkMode_SystemArmed = 0,     /* Notification sound for "System is Armed" */
  SquawkMode_SystemDisarmed,      /* Notification sound for "System is DisArmed" */
                                  /* 0x02-0xFF Reserved */  
};   

#define NO_STROBE           0
#define STROBE_PARALLEL     1    

enum
{
  SquawkLevel_LowLevel = 0,     /* Low Level Sound */
  SquawkLevel_MediumLevel,      /* Medium Level Sound */
  SquawkLevel_HighLevel,        /* High Level Sound */
  SquawkLevel_VeryHighLevel,    /* Very High Level Sound */
};              

#ifdef __IAR_SYSTEMS_ICC__
#pragma pack()
#endif


/******************************************************************************
*******************************************************************************
* Public functions prototypes
*******************************************************************************
******************************************************************************/

zbStatus_t ZCL_IASZoneClusterClient(zbApsdeDataIndication_t *pIndication, afDeviceDef_t *pDev);
zbStatus_t ZCL_IASZoneClusterServer(zbApsdeDataIndication_t *pIndication, afDeviceDef_t *pDev);
zbStatus_t ZCL_IASWDClusterClient(zbApsdeDataIndication_t *pIndication, afDeviceDef_t *pDev);
zbStatus_t ZCL_IASWDServer(zbApsdeDataIndication_t *pIndication, afDeviceDef_t *pDev);

zbStatus_t IASZone_ZoneStatusChange(zclCmdIASZone_ZoneStatusChange_t *pReq);
zbStatus_t IASZone_ZoneEnrollRequest(zclCmdIASZone_ZoneEnrollRequest_t *pReq);
zbStatus_t IASZone_ZoneEnrollResponse(zclCmdIASZone_ZoneEnrollResponse_t *pReq);
                          
zbStatus_t IASWD_StartWarning(zclCmdIASWD_StartWarning_t *pReq);
zbStatus_t IASWD_Squawk(zclCmdIASWD_Squawk_t *pReq);


#endif
