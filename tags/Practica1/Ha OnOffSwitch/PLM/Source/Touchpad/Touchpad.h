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

#ifndef _TOUCHPAD_H
#define _TOUCHPAD_H

/*****************************************************************************
******************************************************************************
* Private macros 
******************************************************************************
*****************************************************************************/

/* List of several touchpad devices */
//#define TM1235
#define TM1440
//#define TM617

#ifdef TM1235
  /* Function $11: 2D sensors */                        
  #define PDT_F11_2D_QUERY_BASE               (0x87)
  #define PDT_F11_2D_COMMAND_BASE             (0x66)
  #define PDT_F11_2D_CONTROL_BASE             (0x30)
  #define PDT_F11_2D_DATA_BASE                (0x18) 
    /* Function $08: BIST Built-In Self Test      */
  #define PDT_F08_BIST_QUERY_BASE             (0x85)
  #define PDT_F08_BIST_COMMAND_BASE           (0x65)
  #define PDT_F08_BIST_CONTROL_BASE           (0x2A)
  #define PDT_F08_BIST_DATA_BASE              (0x15) 
  /* Function $01: RMI Device control             */
  #define PDT_F01_RMI_QUERY_BASE              (0x70)
  #define PDT_F01_RMI_COMMAND_BASE            (0x64)
  #define PDT_F01_RMI_CONTROL_BASE            (0x28)
  #define PDT_F01_RMI_DATA_BASE               (0x13)  
  /* Function $34: Flash memory management        */
  #define PDT_F34_FLASH_QUERY_BASE            (0x67)
  #define PDT_F34_FLASH_COMMAND_BASE          (0x00)
  #define PDT_F34_FLASH_CONTROL_BASE          (0x00)
  #define PDT_F34_FLASH_DATA_BASE             (0x00)
  
  
  /* Data Registers Offset within their function base address */
  /* Data Registers Offset for Function 01 (BIST)             */
  #define mRMI_DeviceStatus_Offset_c          (0x00)        
  #define mRMI_InterruptStatus_Offset_c       (0x01)
  /* Data Registers Offset for Function 08 (RMI device)       */
  #define mBIST_TestNumberControl_Offset_c    (0x00)
  #define mBIST_OverallResult_Offset_c        (0x01)
  #define mBIST_TestSpecificResult_Offset_c   (0x02)
  /* Data Registers Offset for Function 11 (2D sensors)       */
  #define m2D_FingerState_Offset_c            (0x00)
  #define m2D_GestureFlags0_Offset_c          (0x0C)
  #define m2D_GestureFlags1_Offset_c          (0x0D)
 
        
  /* Query Registers Offset within their function base address */
  /* Query Registers Offset for Function 01 (RMI device)       */ 
  #define mRMI_ProductIdQuery0_Offset_c       (0x0B)
  /* Query Registers Offset for Function 11 (2D sensors)       */ 
  #define m2D_PerDeviceQuery_Offset_c         (0x00)
  #define m2D_ReportingMode_Offset_c          (0x01)
  #define m2D_NoOfXElectrodes_Offset_c        (0x02)
  #define m2D_NoOfYElectrodes_Offset_c        (0x03)
  #define m2D_MaximumElectrodes_Offset_c      (0x04)
  #define m2D_AbsoluteQuery_Offset_c          (0x05)   
  #define m2D_GestureQuery_Offset_c           (0x06)
  #define m2D_Query8_Offset_c                 (0x07)
  
  
  /* Control Register Offset within their function base address */
  /* Control Registers Offset for Function 01 (RMI device)      */
  #define mRMI_DeviceControl_Offset_c         (0x00)    
  #define mRMI_InterruptEnable0_Offset_c      (0x01)
  /* Control Registers Offset for Function 08 (BIST)            */ 
  #define mBIST_Test1LimitLow_Offset_c        (0x00)  
  #define mBIST_Test1LimitHigh_Offset_c       (0x01)     
  #define mBIST_Test1LimitDiff_Offset_c       (0x02)  
  #define mBIST_Test2LimitLow_Offset_c        (0x03)  
  #define mBIST_Test2LimitHigh_Offset_c       (0x04)  
  #define mBIST_Test2LimitDiff_Offset_c       (0x05) 
  /* Control Registers Offset for Function 11 (2D sensor)       */  
  #define m2D_ReportMode_Offset_c             (0x00)  
  #define m2D_PalmReject_Offset_c             (0x01)     
  #define m2D_DeltaX_Thresh_Offset_c          (0x02)  
  #define m2D_DeltaY_Thresh_Offset_c          (0x03)  
  #define m2D_Velocity_Offset_c               (0x04)  
  #define m2D_Acceleration_Offset_c           (0x05)  
  #define m2D_MaxXPositionLow_Offset_c        (0x06)  
  #define m2D_MaxXPositionHigh_Offset_c       (0x07)    
  #define m2D_MaxYPositionLow_Offset_c        (0x08)  
  #define m2D_MaxYPositionHigh_Offset_c       (0x09)  
  #define m2D_GestureEnable1_Offset_c         (0x0A)    
  #define m2D_SensorMap0_Offset_c             (0x0B)
  #define m2D_SensorMap1_Offset_c             (0x0C)
  #define m2D_SensorMap2_Offset_c             (0x0D)
  #define m2D_SensorMap3_Offset_c             (0x0E)   
  #define m2D_SensorMap4_Offset_c             (0x0F)
  #define m2D_SensorMap5_Offset_c             (0x10)
  #define m2D_SensorMap6_Offset_c             (0x11)
  #define m2D_SensorMap7_Offset_c             (0x12) 
  #define m2D_SensorMap8_Offset_c             (0x13)
  #define m2D_SensorMap9_Offset_c             (0x14)
  #define m2D_SensorMap10_Offset_c            (0x15)
  #define m2D_SensorMap11_Offset_c            (0x16)
  #define m2D_SensorMap12_Offset_c            (0x17)
  #define m2D_SensorMap13_Offset_c            (0x18)    
  #define m2D_SensorMap14_Offset_c            (0x19)
  #define m2D_SensorMap15_Offset_c            (0x1A)
  #define m2D_SensorMap16_Offset_c            (0x1B)
  #define m2D_SensorMap17_Offset_c            (0x1C)
  #define m2D_Sensitivity0_Offset_c           (0x1D)
  #define m2D_Sensitivity1_Offset_c           (0x1E)
  #define m2D_Sensitivity2_Offset_c           (0x1F)
  #define m2D_Sensitivity3_Offset_c           (0x20)    
  #define m2D_Sensitivity4_Offset_c           (0x21)
  #define m2D_Sensitivity5_Offset_c           (0x22)
  #define m2D_Sensitivity6_Offset_c           (0x23)
  #define m2D_Sensitivity7_Offset_c           (0x24) 
  #define m2D_Sensitivity8_Offset_c           (0x25)
  #define m2D_Sensitivity9_Offset_c           (0x26)
  #define m2D_Sensitivity10_Offset_c          (0x27)
  #define m2D_Sensitivity11_Offset_c          (0x28)
  #define m2D_Sensitivity12_Offset_c          (0x29)
  #define m2D_Sensitivity13_Offset_c          (0x2A)    
  #define m2D_Sensitivity14_Offset_c          (0x2B)
  #define m2D_Sensitivity15_Offset_c          (0x2C)
  #define m2D_Sensitivity16_Offset_c          (0x2D)
  #define m2D_Sensitivity17_Offset_c          (0x2E)  
  #define m2D_MaxTapTime_Offset_c             (0x2F)  
  #define m2D_MinPressTime_Offset_c           (0x30)
  #define m2D_MaxTapDistance_Offset_c         (0x31)     
  #define m2D_MinFlickDistance_Offset_c       (0x32)  
  #define m2D_MinFlickSpeed_Offset_c          (0x33)         
    
  /* Offsets from 0x3C to 0x1FB are reserved and must not be used */    
  #define mConfigurationCRC1_Offset_c         (0x1FC)  
  #define mConfigurationCRC2_Offset_c         (0x1FD)     
  #define mConfigurationCRC3_Offset_c         (0x1FE)  
  #define mConfigurationCRC4_Offset_c         (0x1FF) 
  
  /* Command Register Offset within their function base address */
  /* Command Registers Offset for Function 01 (RMI device) */
  #define mRMI_DeviceCommand_Offset_c         (0x00)                            
#endif

#ifdef TM1440
  /* Function $30: LED/GPIO control */
  #define PDT_F30_LED_QUERY_BASE              (0x8A)
  #define PDT_F30_LED_COMMAND_BASE            (0x00)
  #define PDT_F30_LED_CONTROL_BASE            (0x5F)
  #define PDT_F30_LED_DATA_BASE               (0x25)    
  /* Function $11: 2D sensors */              
  #define PDT_F11_2D_QUERY_BASE               (0x82)
  #define PDT_F11_2D_COMMAND_BASE             (0x63)
  #define PDT_F11_2D_CONTROL_BASE             (0x28)
  #define PDT_F11_2D_DATA_BASE                (0x15) 
  /* Function $01: RMI Device control */
  #define PDT_F01_RMI_QUERY_BASE              (0x6D)
  #define PDT_F01_RMI_COMMAND_BASE            (0x62)
  #define PDT_F01_RMI_CONTROL_BASE            (0x26)
  #define PDT_F01_RMI_DATA_BASE               (0x13)
  /* Function $34: Flash memory management */
  #define PDT_F34_FLASH_QUERY_BASE            (0x64)
  #define PDT_F34_FLASH_COMMAND_BASE          (0x00)
  #define PDT_F34_FLASH_CONTROL_BASE          (0x00)
  #define PDT_F34_FLASH_DATA_BASE             (0x00)
  
  
  /* Data Registers Offset within their function base address */
  /* Data Registers Offset for Function 01 (BIST)             */
  #define mRMI_DeviceStatus_Offset_c          (0x00)
  #define mRMI_InterruptStatus_Offset_c       (0x01)
  /* Data Registers Offset for Function 11 (2D sensors)       */
  #define m2D_FingerState_Offset_c            (0x00)
  #define m2D_GestureFlags0_Offset_c          (0x0C)
  #define m2D_GestureFlags1_Offset_c          (0x0D)   
 
  /* Query Registers Offset within their function base address */
  /* Query Registers Offset for Function 01 (RMI device)       */ 
  #define mRMI_ProductIdQuery0_Offset_c       (0x0B)  
  /* Query Registers Offset for Function 11 (2D sensors)       */  
  #define m2D_PerDeviceQuery_Offset_c         (0x00)
  #define m2D_ReportingMode_Offset_c          (0x01)
  #define m2D_NoOfXElectrodes_Offset_c        (0x02)
  #define m2D_NoOfYElectrodes_Offset_c        (0x03)
  #define m2D_MaximumElectrodes_Offset_c      (0x04)
  #define m2D_AbsoluteQuery_Offset_c          (0x05)   
  #define m2D_GestureQuery_Offset_c           (0x06)
  #define m2D_Query8_Offset_c                 (0x07) 
  
  
  /* Control Register Offset within the configuration block */
  /* Control Registers offset for Function 01 (RMI device)  */
  #define mRMI_DeviceControl_Offset_c         (0x00)  
  #define mRMI_InterruptEnable0_Offset_c      (0x01)  
  /* Control Registers offset for Function 11 (2D sensor)   */    
  #define m2D_ReportMode_Offset_c             (0x00)  
  #define m2D_PalmReject_Offset_c             (0x01)     
  #define m2D_DeltaX_Thresh_Offset_c          (0x02)  
  #define m2D_DeltaY_Thresh_Offset_c          (0x03)  
  #define m2D_Velocity_Offset_c               (0x04)  
  #define m2D_Acceleration_Offset_c           (0x05)  
  #define m2D_MaxXPositionLow_Offset_c        (0x06)  
  #define m2D_MaxXPositionHigh_Offset_c       (0x07)    
  #define m2D_MaxYPositionLow_Offset_c        (0x08)  
  #define m2D_MaxYPositionHigh_Offset_c       (0x09)  
  #define m2D_GestureEnable1_Offset_c         (0x0A)    
  #define m2D_SensorMap0_Offset_c             (0x0B)
  #define m2D_SensorMap1_Offset_c             (0x0C)
  #define m2D_SensorMap2_Offset_c             (0x0D)
  #define m2D_SensorMap3_Offset_c             (0x0E)    
  #define m2D_SensorMap4_Offset_c             (0x0F)
  #define m2D_SensorMap5_Offset_c             (0x10)
  #define m2D_SensorMap6_Offset_c             (0x11)
  #define m2D_SensorMap7_Offset_c             (0x12) 
  #define m2D_SensorMap8_Offset_c             (0x13)
  #define m2D_SensorMap9_Offset_c             (0x14)
  #define m2D_SensorMap10_Offset_c            (0x15)
  #define m2D_SensorMap11_Offset_c            (0x16)
  #define m2D_SensorMap12_Offset_c            (0x17)
  #define m2D_SensorMap13_Offset_c            (0x18)    
  #define m2D_SensorMap14_Offset_c            (0x19)
  #define m2D_SensorMap15_Offset_c            (0x1A)
  #define m2D_SensorMap16_Offset_c            (0x1B)
  #define m2D_SensorMap17_Offset_c            (0x1C) 
  #define m2D_SensorMap18_Offset_c            (0x1D)
  #define m2D_SensorMap19_Offset_c            (0x1E)
  /* Offsets from 0x21 to 0x34 are reserved and must not be used */    
  #define m2D_MaxTapTime_Offset_c             (0x33)  
  #define m2D_MaxTapDistance_Offset_c         (0x34)     
  #define m2D_MinFlickDistance_Offset_c       (0x35)  
  #define m2D_MinFlickSpeed_Offset_c          (0x36)    
  #define mGPIO_Led_General_Offset_c          (0x37)  
  #define mGPIO_Direction_Offset_c            (0x38)  
  #define mGPIO_Data_Offset_c                 (0x39)
  /* Offsets from 0x3C to 0x1FB are reserved and must not be used */    
  #define mConfigurationCRC1_Offset_c         (0x1FC)  
  #define mConfigurationCRC2_Offset_c         (0x1FD)     
  #define mConfigurationCRC3_Offset_c         (0x1FE)  
  #define mConfigurationCRC4_Offset_c         (0x1FF)  
  
  /* Command Register Offset within their function base address */
  /* Command Registers Offset for Function 01 (RMI device) */
  #define mRMI_DeviceCommand_Offset_c         (0x00)   
  /****************************************************************************/
  #define m2D_MinPressTime_Offset_c           (0x30)                       
#endif

#ifdef TM617
  /* Function $11: 2D sensors              */
  #define PDT_F11_2D_QUERY_BASE               (0x8E)
  #define PDT_F11_2D_COMMAND_BASE             (0x6F)
  #define PDT_F11_2D_CONTROL_BASE             (0x26)
  #define PDT_F11_2D_DATA_BASE                (0x15) 
  /* Function $01: RMI Device control      */      
  #define PDT_F01_RMI_QUERY_BASE              (0x79)
  #define PDT_F01_RMI_COMMAND_BASE            (0x6E)
  #define PDT_F01_RMI_CONTROL_BASE            (0x24)
  #define PDT_F01_RMI_DATA_BASE               (0x13)
  /* Function $34: Flash memory management */
  #define PDT_F34_FLASH_QUERY_BASE            (0x70)
  #define PDT_F34_FLASH_COMMAND_BASE          (0x00)
  #define PDT_F34_FLASH_CONTROL_BASE          (0x00)
  #define PDT_F34_FLASH_DATA_BASE             (0x00)
  
  
  /* Data Registers Offset within their function base address */
  /* Data Registers Offset for Function 01 (BIST)             */
  #define mRMI_DeviceStatus_Offset_c          (0x00)
  #define mRMI_InterruptStatus_Offset_c       (0x01)
  /* Data Registers Offset for Function 11 (2D sensors)       */
  #define m2D_FingerState_Offset_c            (0x00)
  #define m2D_GestureFlags0_Offset_c          (0x0C)
  #define m2D_GestureFlags1_Offset_c          (0x0D) 
  
  
  /* Query Registers Offset within their function base address */
  /* Query Registers Offset for Function 01 (RMI device)       */ 
  #define mRMI_ProductIdQuery0_Offset_c       (0x00)  
  /* Query Registers Offset for Function 11 (2D sensors)       */  
  #define m2D_PerDeviceQuery_Offset_c         (0x00)
  #define m2D_ReportingMode_Offset_c          (0x01)
  #define m2D_NoOfXElectrodes_Offset_c        (0x02)
  #define m2D_NoOfYElectrodes_Offset_c        (0x03)
  #define m2D_MaximumElectrodes_Offset_c      (0x04)
  #define m2D_AbsoluteQuery_Offset_c          (0x05)   
  #define m2D_GestureQuery_Offset_c           (0x06)
  #define m2D_Query8_Offset_c                 (0x07)
  
  
  /* Control Register Offset within the configuration block */
  #define mRMI_DeviceControl_Offset_c         (0x00)  
  #define mRMI_InterruptEnable0_Offset_c      (0x01)  
  #define m2D_ReportMode_Offset_c             (0x02)  
  #define m2D_PalmReject_Offset_c             (0x03)     
  #define m2D_DeltaX_Thresh_Offset_c          (0x04)  
  #define m2D_DeltaY_Thresh_Offset_c          (0x05)  
  #define m2D_Velocity_Offset_c               (0x06)  
  #define m2D_Acceleration_Offset_c           (0x07)  
  #define m2D_MaxXPositionLow_Offset_c        (0x08)  
  #define m2D_MaxXPositionHigh_Offset_c       (0x09)    
  #define m2D_MaxYPositionLow_Offset_c        (0x0A)  
  #define m2D_MaxYPositionHigh_Offset_c       (0x0B)  
  #define m2D_GestureEnable1_Offset_c         (0x0C)
  #define m2D_GestureEnable2_Offset_c         (0x0D)       
  #define m2D_SensorMap0_Offset_c             (0x0E)
  #define m2D_SensorMap1_Offset_c             (0x0F)
  #define m2D_SensorMap2_Offset_c             (0x10)
  #define m2D_SensorMap3_Offset_c             (0x11)    
  #define m2D_SensorMap4_Offset_c             (0x12)
  #define m2D_SensorMap5_Offset_c             (0x13)
  #define m2D_SensorMap6_Offset_c             (0x14)
  #define m2D_SensorMap7_Offset_c             (0x15) 
  #define m2D_SensorMap8_Offset_c             (0x16)
  #define m2D_SensorMap9_Offset_c             (0x17)
  #define m2D_SensorMap10_Offset_c            (0x18)
  #define m2D_SensorMap11_Offset_c            (0x19)
  #define m2D_SensorMap12_Offset_c            (0x1A)
  #define m2D_SensorMap13_Offset_c            (0x1B)    
  #define m2D_SensorMap14_Offset_c            (0x1C)
  #define m2D_SensorMap15_Offset_c            (0x1D)
  #define m2D_SensorMap16_Offset_c            (0x1E)
  #define m2D_SensorMap17_Offset_c            (0x1F) 
  #define m2D_SensorMap18_Offset_c            (0x20)
  #define m2D_SensorMap19_Offset_c            (0x21)
  #define m2D_SensorMap20_Offset_c            (0x22)
  #define m2D_SensorMap21_Offset_c            (0x23)
  #define m2D_SensorMap22_Offset_c            (0x24)
  #define m2D_SensorMap23_Offset_c            (0x25)    
  #define m2D_SensorMap24_Offset_c            (0x26)
  #define m2D_SensorMap25_Offset_c            (0x27)
  #define m2D_SensorMap26_Offset_c            (0x28)  
  #define m2D_Sensitivity0_Offset_c           (0x29)
  #define m2D_Sensitivity1_Offset_c           (0x2A)
  #define m2D_Sensitivity2_Offset_c           (0x2B)
  #define m2D_Sensitivity3_Offset_c           (0x2C)    
  #define m2D_Sensitivity4_Offset_c           (0x2D)
  #define m2D_Sensitivity5_Offset_c           (0x2E)
  #define m2D_Sensitivity6_Offset_c           (0x2F)
  #define m2D_Sensitivity7_Offset_c           (0x30) 
  #define m2D_Sensitivity8_Offset_c           (0x31)
  #define m2D_Sensitivity9_Offset_c           (0x32)
  #define m2D_Sensitivity10_Offset_c          (0x33)
  #define m2D_Sensitivity11_Offset_c          (0x34)
  #define m2D_Sensitivity12_Offset_c          (0x35)
  #define m2D_Sensitivity13_Offset_c          (0x36)    
  #define m2D_Sensitivity14_Offset_c          (0x37)
  #define m2D_Sensitivity15_Offset_c          (0x38)
  #define m2D_Sensitivity16_Offset_c          (0x39)
  #define m2D_Sensitivity17_Offset_c          (0x3A) 
  #define m2D_Sensitivity18_Offset_c          (0x3B)
  #define m2D_Sensitivity19_Offset_c          (0x3C)
  #define m2D_Sensitivity20_Offset_c          (0x3D)
  #define m2D_Sensitivity21_Offset_c          (0x3E)
  #define m2D_Sensitivity22_Offset_c          (0x3F)
  #define m2D_Sensitivity23_Offset_c          (0x40)    
  #define m2D_Sensitivity24_Offset_c          (0x41)
  #define m2D_Sensitivity25_Offset_c          (0x42)
  #define m2D_Sensitivity26_Offset_c          (0x43)  
  #define m2D_TapTime_Offset_c                (0x44)  
  #define m2D_PressTime_Offset_c              (0x45)     
  #define m2D_TapXSize_Offset_c               (0x46)  
  #define m2D_TapYSize_Offset_c               (0x47)    
  #define m2D_FlickXSpeed_Offset_c            (0x48)  
  #define m2D_FlickYSpeed_Offset_c            (0x49)    
  /* Offsets from 0x4A to 0x1FB are reserved and must not be used */    
  #define mConfigurationCRC1_Offset_c         (0x1FC)  
  #define mConfigurationCRC2_Offset_c         (0x1FD)     
  #define mConfigurationCRC3_Offset_c         (0x1FE)  
  #define mConfigurationCRC4_Offset_c         (0x1FF) 
  
  
  #define m2D_MaxTapTime_Offset_c             m2D_TapTime_Offset_c
  #define m2D_MinPressTime_Offset_c           m2D_PressTime_Offset_c
  #define m2D_MaxTapDistance_Offset_c         m2D_TapXSize_Offset_c 
  #define m2D_MinFlickDistance_Offset_c       m2D_FlickXSpeed_Offset_c
#endif

/*****************************************************************************/
/*                                IRQ part                                   */
/*****************************************************************************/

#if(gTP_Mode_c == gTP_IrqIsrMode_c)
  /* Define the IRQSC */
  #define m_IRQSC_c                             IRQSC   

  /* Bits in IRQSC (Status and Control Register) */
  #define mIRQSC_IRQMOD_c                       (1<<0)
  #define mIRQSC_IRQIE_c                        (1<<1)
  #define mIRQSC_IRQACK_c                       (1<<2)
  #define mIRQSC_IRQF_c                         (1<<3)
  #define mIRQSC_IRQPE_c                        (1<<4)
  #define mIRQSC_IRQEDG_c                       (1<<5)
  #define mIRQSC_IRQPDD_c                       (1<<6)

  /* Reset the external interrupt capability */
  #define mIRQSC_Reset_c                        (0)

  /* Define IRQ pin sensitivity */
  #define mIRQ_FallingEdge_Sense_c              (0)
  #define mIRQ_RisingEdge_Sense_c               (mIRQSC_IRQEDG_c)
  #define mIRQ_FallingAndLow_Sense_c            (mIRQSC_IRQMOD_c)
  #define mIRQ_RisingAndHigh_Sense_c            (mIRQSC_IRQEDG_c | mIRQSC_IRQMOD_c)
#elif (gTP_Mode_c == gTP_KbiIsrMode_c)
  #define m_KBI2SC_c                          (KBI2SC)
  #define m_KBI2PE_c                          (KBI2PE)
  #define m_KBI2ES_c                          (KBI2ES)
  
  /* Bits in KBI2SC (Status and Control Register) */
  #define mKBI2SC_KBIMOD_c                    (1<<0) 
  #define mKBI2SC_KBIE_c                      (1<<1)
  #define mKBI2SC_KBACK_c                     (1<<2)
  #define mKBI2SC_KBF_c                       (1<<3)
  
  /* Reset the external interrupt capability */
  #define mKBI2SC_Reset_c                     (0)
  
  #define mKBI2PE_KBIPE0_c                    (1<<0)
  #define mKBI2ES_KBEDG0_c                    (1<<0)
  
  /* Define KBI pin sensitivity */
  #define mKBI2_FallingEdge_Sense_c           (0)
  #define mKBI2_RisingEdge_Sense_c            (m_KBI2ES_c |= mKBI2ES_KBEDG0_c)
  #define mKBI2_FallingAndLow_Sense_c         (m_KBI2SC_c |= mKBI2SC_KBIMOD_c)
  #define mKBI2_RisingAndHigh_Sense_c         (m_KBI2ES_c |= mKBI2ES_KBEDG0_c;\ m_KBI2SC_c |= mKBI2SC_KBIMOD_c)
#endif 

/*****************************************************************************/
/*                                 TP part                                   */
/*****************************************************************************/
/* Gestures mask */
#define gTP_PinchMask_c                         (1<<6)
#define gTP_PressMask_c                         (1<<5)
#define gTP_FlickMask_c                         (1<<4)
#define gTP_EarlyTapMask_c                      (1<<3)
#define gTP_DoubleTapMask_c                     (1<<2)
#define gTP_TapAndHoldMask_c                    (1<<1)
#define gTP_SingleTapMask_c                     (1<<0)

#define gTP_NoOfFingersMask_c                   (0x07)
#define gTP_RelModeMask_c                       (1<<3)
#define gTP_AbsModeMask_c                       (1<<4)
#define gTP_GestureMask_c                       (1<<5)
#define gTP_SensAdjMask_c                       (1<<6)

/* Interrupt Status masks */
#define gTP_FlashIntMask_c                      (1<<0)
#define gTP_StatusIntMask_c                     (1<<1)
#ifdef TM1235
  #define   gTP_BistIntMask_c                   (1<<2)
#elif (defined  TM1440) || (defined TM617)
  #define   gTP_BistIntMask_c                   (0) 
#endif   
#ifdef  TM1235
  #define gTP_Abs0IntMask_c                     (1<<3)
#elif (defined  TM1440) || (defined TM617)
  #define gTP_Abs0IntMask_c                     (1<<2)
#endif
#ifdef  TM1440
  #define gTP_GpioIntMask_c                     (1<<3)
#elif (defined  TM1235) || (defined TM617)
  #define gTP_GpioIntMask_c                     (0)
#endif

#define gTP_PalmMask_c                          (1<<0)
#define gTP_RotateMask_c                        (1<<1)

#define gTP_ReportingModeMask_c                 (0x07)

/* Finger info size */
#define gTP_FingerAbsValSize_c                  (5)
#define gTP_FingerRelValSize_c                  (2)
#define gTP_MaxFingerInfoSize_c                 (gTP_FingerAbsValSize_c + gTP_FingerRelValSize_c)
#define gTP_GestureStatusSize_c                 (2)
#define gTP_GestureInfoSize_c                   (3)

/* Bit manipulation */
#define gTP_LowNibbleMask_c                     (0x0F)
#define gTP_HighNibbleMask_c                    (gTP_LowNibbleMask_c << 4)


#define TP_Ceil(a, b)                           ((!((a) % (b))) ? ((uint8_t)((a)/(b))) : ((uint8_t)((a)/(b)) + 1))
#define TP_FingerStateSize(fingerNo)            TP_Ceil(fingerNo, 4)  

#define gTP_FingerStateMask_c                   (0x03) 

/* Initiate a device configuration */
#define gTP_Configured_c                        (1<<7)         
#define gTP_Unconfigured_c                      gTP_Configured_c

/* Reset command */
#define gTP_DevReset_c                          (1<<0)

#endif /* _TOUCHPAD_H */