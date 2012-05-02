/*****************************************************************************
* Touchpad Hardware Abstraction Layer
*
* Copyright (c) 2010, Freescale, Inc. All rights reserved
*
* No part of this document must be reproduced in any form - including copied,
* transcribed, printed or by any electronic means, - without specific written 
* permission from Freescale Semiconductor
* 
*****************************************************************************/

#ifndef _TOUCHPAD_INTERFACE_H
#define _TOUCHPAD_INTERFACE_H

#include "EmbeddedTypes.h"
#include "IIC_Interface.h"

/*****************************************************************************
******************************************************************************
* Public macros 
******************************************************************************
*****************************************************************************/
/* Defines whether or not the TP-HAL is included in the application */
#ifndef gTouchpadIncluded_d
  #define gTouchpadIncluded_d  FALSE
#endif

/* Depending how the interrupt line of the touchpad is connected, it can
   work in 3 distinct modes:
   1. Interrupt mode, connected to external IRQ line of MCU. In this case,
make sure  that inside the isrvectors.c file, the function 'TP_InterruptHandler'
is set as external IRQ interrupt handler (vector 2)
   2. Interrupt mode, connected to Pin 0 of keyboard 2. In this case,
make sure  that inside the isrvectors.c file, the function 'TP_InterruptHandler'
is set as KBI2 interrupt handler(vector 23)
   3. Polling mode. In this case, make sure that the function 'TP_InterruptHandler'
will be periodically called. Use an interval timer to achieve this. Selection of
the timer interval depends on the resolution needed for aquiring the data.
50 ms are recommanded. */


#define gTP_IrqIsrMode_c                        1
#define gTP_KbiIsrMode_c                        2
#define gTP_PollingMode_c                       3

#ifndef gTP_Mode_c
#define gTP_Mode_c gTP_PollingMode_c
#endif /* gTP_Mode_c */


/* Generic status codes returned by the most Touchpad functions */
#define TP_OK                           (0x00)
#define TP_ERR_INVALID_PARAM            (0x01)
#define TP_ERR_IIC_COMM                 (0x02)
#define TP_ERR_GESTURE_NOT_SUPPORTED    (0x03)
#define TP_ERR_DEVICE_NOT_READY         (0x04)

/* Specifies the priority for touchpad task unless if it is not specified in other 
locations */
#ifndef gTP_TaskPriority_c
  #define gTP_TaskPriority_c            (0x08)
#endif

/* Possible baud rates for IIC */
#define gTP_NormalMode_c                gIIC_BaudRate_100000_c
#define gTP_FastMode_c                  gIIC_BaudRate_400000_c

#ifndef gTP_Default_IIC_CommRate_c
  #define gTP_Default_IIC_CommRate_c    gTP_NormalMode_c
#endif

/* Slave address for the touchpad sensor device */
#define gTP_SlaveAddress_c              (0x20)

/* Maximum number of characters for the product ID string */
#define gTP_ProdIdLengthMax_c           (10)

/* Number of fingers supported */
#define gTP_MaxNoOfFingers              (2)

/*****************************************************************************
******************************************************************************
* Public type definitions 
******************************************************************************
*****************************************************************************/

/* Possible status code returned by the touchpad sensor */
typedef enum
{
  gTP_NoError_c     = 0,
  gTP_ResetOccured_c,
  gTP_InvalidConfig_c,
  gTP_DeviceFailure_c,
  gTP_FirmwareCRCFailure_c,
  gTP_CRCInProgress_c  
} deviceCode_t;

/* Possible reported mode of the sensor when a finger is present on the surface */
typedef enum
{
  gTP_ContinuousRepMode_c     = 0,
  gTP_ReducedRepMode_c,
  gTP_FingerChangeRepMode_c
} reportingMode_t;

/* Function identifier used by the touchpad task to recognize the function 
   which initiate the IIC transfer */
typedef enum
{
  gTP_GetDevInfoId_c = 1,
  gTP_GetDevConfigId_c,
  gTP_SetDevConfigId_c,
  gTP_IsrId_c,
  gTP_DevRestartId_c  
} funcId_t;

/* Type definition for the press's configuration parameter */
typedef uint8_t pressConfigParam_t;
/* Type definition for the palm config parameter */
typedef uint8_t palmConfigParam_t;
/* Status code returned by touchpad functions */
typedef uint8_t statusCode_t;
/* Information associated to pinch gesture */
typedef int8_t  pinchGestureInfo_t;

/* Structure containing the device gesture list capability */
typedef struct gestList_tag
{
	bool_t 		                mHasPinch;
 	bool_t			              mHasPress;
 	bool_t			              mHasFlick;
	bool_t			              mHasSingleTap;
	bool_t			              mHasDoubleTap;
	bool_t			              mHasTapAndHold;
 	bool_t			              mHasRotate; 
	bool_t			              mHasPalm; 
} gestList_t;

/* Structure containing the device configuration */
typedef struct devInfo_tag
{
  uint8_t                   mProdId[gTP_ProdIdLengthMax_c];  
  uint8_t                   mNoOfFingers;
  uint8_t                   mReportedMode;
  bool_t                    mHasAbs;
  bool_t                    mHasRel;
  bool_t                    mHasSensAdj;
  bool_t                    mHasGesture; 
  gestList_t                mGestList;
  uint16_t                  mMaxXPos;
  uint16_t                  mMaxYPos;
} devInfo_t;

/* Structure containing the which gestures are enabled */
typedef struct gestIntEn_tag
{
	bool_t 		                mPinchIntEn;
 	bool_t			              mPressIntEn;
 	bool_t			              mFlickIntEn;
	bool_t			              mSingleTapIntEn;
	bool_t		              	mDoubleTapIntEn;
	bool_t		              	mTapAndHoldIntEn;
 	bool_t	              		mRotateIntEn; 
	bool_t	              		mPalmIntEn; 
} gestIntEn_t;
             
/* Configuration parameters to defining a 'tap' gesture */
typedef struct tapConfigParam_tag
{
  uint8_t                   mMaximumTapDistance;
  uint8_t                   mMaximumTapTime;
} tapConfigParam_t;

/* Configuration parameters to defining a 'flick' gesture */
typedef struct flickConfigParam_tag
{
  uint8_t                   mMinimumFlickDistance;
  uint8_t                   mMinimumFlickSpeed;
} flickConfigParam_t;

/* Type definition for the structure containing a collection of the parameters defining 
   each gesture in part */
typedef struct gestConfigParam_tag
{
	tapConfigParam_t  		    mTapConfigParam;
 	flickConfigParam_t	  	  mFlickConfigParam; 
	pressConfigParam_t		    mPressConfigParam;
	palmConfigParam_t	 	      mPalmConfigParam;
} gestConfigParam_t;

/* Structure containig the status list of function available */
typedef struct funcIntEn_tag
{
  bool_t                    mGpioEn;
  bool_t                    mSensorEn;
  bool_t                    mDevStatusEn;
  bool_t                    mSelfTestEn;
  bool_t                    mFlashEn;                 
} funcIntEn_t;
 
/* Structure containing the device configuration list */
typedef struct devConfig_tag
{
  funcIntEn_t               mFuncIntEn;
  gestIntEn_t               mGestIntEn;
  gestConfigParam_t         mGestConfigParam;
} devConfig_t;

/* Structure containing the finger information */
typedef struct fingerInfo_tag
{
  uint8_t                   mFingerStatus;
  uint16_t                  mPosX;                                              
  uint16_t                  mPosY;
  uint8_t                   mWidthX;
  uint8_t                   mWidthY;
  uint8_t                   mFingerStrength;
  int8_t                    mDeltaX;
  int8_t                    mDeltaY;
} fingerInfo_t;

/* The list of recognized gesture status */
typedef struct gestStatus_tag
{
	bool_t 	                	mPinch;
 	bool_t			              mPress;
 	bool_t		              	mFlick;
	bool_t              			mSingleTap;
	bool_t	              		mDoubleTap;
	bool_t		              	mTapAndHold;
 	bool_t		              	mRotate; 
	bool_t		              	mPalm; 
} gestStatusList_t;

/* Structure containing the additional information for a 'flick' gesture */
typedef struct flickGestureInfo_tag
{
	int8_t 		                mFlickDistanceX;
 	int8_t			              mFlickDistanceY;
	uint8_t		                mFlickTime; 	 
} flickGestureInfo_t;

/* Type definition for the structure containing the parameters defining a 
   'rotate' gesture */
typedef struct rotateGestureInfo_tag
{
	int8_t 		                mRotateMotion;
 	uint8_t			              mFingersDistance;
} rotateGestureInfo_t;

/* Structure containing a collection of the pointers to the information of 
   each gesture in part */
typedef struct gestInfo_tag
{
	pinchGestureInfo_t		    mPinchGestureInfo;	
	flickGestureInfo_t		    mFlickGestureInfo; 
	rotateGestureInfo_t       mRotateGestureInfo;	
} gestInfo_t;

/* Strucutre containing all information required by the application callback */
typedef struct tpCallbackInfo_tag
{
  funcId_t                  funcId;
  bool_t                    status;              
  devInfo_t                 devInfo;
  devConfig_t               devConfig;
  fingerInfo_t              fingerInfo[gTP_MaxNoOfFingers];      
  gestStatusList_t          gestStatusList;
  gestInfo_t                gestInfo;
  
} tpCallbackInfo_t;

/* Type definition for the pointer to the callback function */
typedef void (*tpCallback_t)(tpCallbackInfo_t tpCallbackInfo);

/*****************************************************************************
******************************************************************************
* Public memory declaration 
******************************************************************************
*****************************************************************************/


/*****************************************************************************
******************************************************************************
* Public prototypes 
******************************************************************************
*****************************************************************************/

/* Place it in NON_BANKED memory */
#ifdef MEMORY_MODEL_BANKED
#pragma CODE_SEG __NEAR_SEG NON_BANKED
#else
#pragma CODE_SEG DEFAULT
#endif  /* MEMORY_MODEL_BANKED */
/* TP Interrupt Service Routine */
#if(gTP_Mode_c == gTP_PollingMode_c)
  /* When touchpad is used in polling mode, TP_InterruptHandler
  should be called periodically by the application */
  extern void TP_InterruptHandler(void);  
#else
  extern INTERRUPT_KEYWORD void TP_InterruptHandler(void);
#endif


#pragma CODE_SEG DEFAULT


/*****************************************************************************/
/* Initialize the IIC Platform Component, TP task */
extern statusCode_t TP_Init(void);

/* Uniniti the sensor device, the IIC Platform component and disallocates all variable */
extern void TP_Uninit(void);

/* Reset the touchpad sensor */
extern statusCode_t TP_DevRestart(void);

/* Collects the information about the device configuration */
extern statusCode_t TP_GetDevInfo(void);

/* Collects information about gesture parameters */
extern statusCode_t TP_GetDevConfig(void); 

/* Configure the list of gestures to be identified together with their associated parameters */
extern statusCode_t TP_SetDevConfig(devConfig_t mDevConfig);

/* Provides a pointer to the callback function for any touchpad event */
extern statusCode_t TP_SetCallback(tpCallback_t pfTpCallback);

#endif /* _TOUCHPAD_INTERFACE_H */