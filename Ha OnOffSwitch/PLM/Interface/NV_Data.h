/*****************************************************************************
* Declarations for the application client of the NV storage module.
*
* c) Copyright 2006, Freescale, Inc. All rights reserved.
*
*
* No part of this document must be reproduced in any form - including copied,
* transcribed, printed or by any electronic means - without specific written
* permission from Freescale Semiconductor.
*
*****************************************************************************/

#ifndef _NV_DATA_H_
#define _NV_DATA_H_

#include "EmbeddedTypes.h"


/* IMPORTANT: See the comments at the beginning of NVM_Interface.h. */

#ifndef gNvSelfTest_d
#define gNvSelfTest_d   0
#endif

/*****************************************************************************
******************************************************************************
* Public macros
******************************************************************************
*****************************************************************************/

/* Define the data set IDs. These values are part of the method for */
/* for recognizing data sets in NV storage. Any 8 bit value can be used. */
/* IMPORTANT: If the format of a data set or of any of its parts changes, */
/* this value must be changed. If it isn't, the NV storage code will not */
/* know the difference between the old and new formats of the data set. */
/* The convention here is not to remove the old definitions when a format */
/* changes, to avoid reusing IDs, at least for a while. */

/* IDs used by data set 1. */
#define gNvDataSet_01_v00_ID_c  0x10
#define gNvDataSet_01_v01_ID_c  0x11
#define gNvDataSet_01_v02_ID_c  0x12

/* IDs used by data set 2. */
#define gNvDataSet_02_v00_ID_c  0x20

/* IDs used by data set 3. */
#define gNvDataSet_03_v00_ID_c  0x30
#define gNvDataSet_03_v01_ID_c  0x31

/* IDs used by data set 4. */
#define gNvDataSet_04_v00_ID_c  0x40

/* Current IDs for each data set. */
#define gNvDataSet_Nwk_ID_c      gNvDataSet_01_v02_ID_c
#define gNvDataSet_App_ID_c      gNvDataSet_02_v00_ID_c
#define gNvDataSet_Sec_ID_c      gNvDataSet_03_v01_ID_c  /* Added for security */
#define gNvDataSet_Aps_Sec_ID_c  gNvDataSet_04_v00_ID_c  /* Aps data and Aps security data. */

/*****************************************************************************
******************************************************************************
* Private macros
******************************************************************************
*****************************************************************************/

/* Values for Abel register 0x04  */
/* MSB = CCA Energy detect threshold: abs(power in dBm)*2, 0x96 = -75dBm, */
/* 0x82 = -65dBm */
#define ABEL_CCA_ENERGY_DETECT_THRESHOLD 0x9600

/* LSB = Power compensation Offset added to RSSI. */
/* Typical 0x74 FFJ/JK 13/01/04 (Abel 013). */
#define ABEL_POWER_COMPENSATION_OFFSET   0x0074

/*PA level for each individual channel*/
/* Channel number                 11   12   13 	 14   15   16   17   18   19   20   21   22   23   24   25   26 */
#ifndef CHANNEL_PA
#define CHANNEL_PA              0xBC,0xBC,0xBC,0xBC,0xBC,0xBC,0xBC,0xBC,0xBC,0xBC,0xBC,0xBC,0xBC,0xBC,0xBC,0xBC
#endif


/*****************************************************************************
* Register 0x0A, bit (15:8)
* Xtal trim - crystal oscillator capacitor trim
* Setting       AbelX.X
* ---------    ------------
*   0x00         Default from reset
*****************************************************************************/

#define ABEL_XTAL_TRIM (0x36 << 8) /* MSB */

/*****************************************************************************
* Register 0x0A, bit (7:6)
* Xtal bias - crystal oscillator bias adjustment
* Setting       Abel2.0            Abel2.1
* ---------    ------------       ------------
*   00           5 (Doze)           5          (Default from reset)
*   01           8 (Idle)           8
*   10           11                 11 (Doze, Idle)
*   11           84 (Boost)         84 (Boost)
*****************************************************************************/

#define ABEL_XTAL_BIAS_CURRENT 0x40 /* LSB  */

/*****************************************************************************
* Register 0x0A, bit (5:3)
* Chip rate
* Setting       AbelX.X
* ---------    ------------
*   0x00         Default from reset
*****************************************************************************/

#define ABEL_CHIP_RATE 0x00 /* LSB  */

/*****************************************************************************
* Register 0x0A, bit (2:0)
* CLKO rate - selects the clock frequency of the CLKO out pin
* Setting       AbelX.X
* ---------    ------------
*   000          16MHz
*   001          8MHz
*   010          4MHz
*   011          2MHz
*   100          1MHz
*   101          62,5kHz
*   110          31,25kHz (default from reset)
*   111          15,625kHz
*****************************************************************************/

#ifdef SYSTEM_CLOCK_16780MHZ
#define ABEL_CLKO_FREQ 0x06 /* LSB  */
#else
#define ABEL_CLKO_FREQ 0x05 /* LSB  */
#endif

/*****************************************************************************
* CLOCK_OUT_SETTING and HF_CALIBRATION
* Values depending on target
*****************************************************************************/           
#if (defined(TARGET_TOROWEAP)) // MC1321x RF

  #define gaMC1321x_CCA_Threshold_c 0x96
  #define gaMC1321x_CCA_Offset_c    0x9B    
  #define CLOCK_OUT_SETTING   0x7E85   
  #define HF_CALIBRATION      (( gaMC1321x_CCA_Threshold_c << 8 ) | ( gaMC1321x_CCA_Offset_c ))
  
#elif (defined(TARGET_USER_DEFINED) ) //User RF 

 #if (gUserTransceiverType_d == MC1321x)
  #define gaMC1321x_CCA_Threshold_c 0x96
  #define gaMC1321x_CCA_Offset_c    0x9B
  #define CLOCK_OUT_SETTING   0x7E85 
  #define HF_CALIBRATION      (( gaMC1321x_CCA_Threshold_c << 8 ) | ( gaMC1321x_CCA_Offset_c ))
  
 #elif (gUserTransceiverType_d == MC1319x)
  #define gaMC1319x_CCA_Threshold_c 0x96
  #define gaMC1319x_CCA_Offset_c    0x9B
  #define CLOCK_OUT_SETTING   (ABEL_XTAL_TRIM | ABEL_XTAL_BIAS_CURRENT | ABEL_CHIP_RATE | ABEL_CLKO_FREQ)
  #define HF_CALIBRATION      (( gaMC1319x_CCA_Threshold_c << 8 ) | (gaMC1319x_CCA_Offset_c))
  
 #else /* MC1323x*/ 
  #define gaMC1323x_CCA_Threshold_c 0x96
  #define gaMC1323x_CCA_Offset_c    0x9B
 #endif
 
#elif (defined(TARGET_MC1323x) )
    //CCA Threshold
  #define gaMC1323x_CCA_Threshold_c 0x96
  #define gaMC1323x_CCA_Offset_c    0x9B
  
#else    //MC1319x RF
  #define gaMC1319x_CCA_Threshold_c 0x96
  #define gaMC1319x_CCA_Offset_c    0x9B
 #define CLOCK_OUT_SETTING   (ABEL_XTAL_TRIM | ABEL_XTAL_BIAS_CURRENT | ABEL_CHIP_RATE | ABEL_CLKO_FREQ)
 #define HF_CALIBRATION      (( gaMC1319x_CCA_Threshold_c << 8 ) | ( gaMC1319x_CCA_Offset_c ))
#endif TARGET_TOROWEAP


/*****************************************************************************
******************************************************************************
* Public type definitions
******************************************************************************
*****************************************************************************/

/* The Fresscale version string struct is placed at a specific location in */
/* ROM by the linker command file, and is normally only written during */
/* manufacturing. */
typedef struct FreescaleVersionStrings_tag {
  uint16_t NV_RAM_Version;
  uint8_t MAC_Version[4];
  uint8_t PHY_Version[4];
  uint8_t STACK_Version[4];
  uint8_t APP_Version[4];
  uint8_t HWName_Revision[4];
  uint8_t SerialNumber[4];
  uint16_t ProductionSite;
  uint8_t CountryCode;
  uint8_t ProductionWeekCode;
  uint8_t ProductionYearCode;
  uint8_t MCU_Manufacture;
  uint8_t MCU_Version;
  uint8_t NOT_USED;
} FreescaleVersionStrings_t;

/* The HardwareParameters_t struct is treated specially by both the */
/* linker command file and the startup (crt0.c) code. See the comments */
/* in crt0.c. */
/* The initialString and terminalString are unsigned chars, instead of */
/* uint8_t, because the C standard guarantees the sizeof(unsigned char). */
/* These strings are needed by the startup code to locate the current */
/* copy of the structure in NV storage, if there is one, and must be the */
/* first and last fields in the type. */
#define gaHardwareParametersDelimiterString_c   "Delim"
#if defined(PROCESSOR_QE128)
typedef struct HardwareParameters_tag {
  unsigned char initialString[ sizeof( gaHardwareParametersDelimiterString_c )];
  uint8_t Bus_Frequency_In_MHz;
  uint16_t Abel_Clock_Out_Setting;
  uint16_t Abel_HF_Calibration;
  uint8_t NV_ICSC1;
  uint8_t NV_ICSC2;
  uint8_t NV_ICSTRM;
  uint8_t NV_ICSSC;
  uint8_t MAC_Address[8];
  uint8_t defaultPowerLevel;
  uint8_t useDualAntenna;
  uint8_t paPowerLevelLimits[16];  
  unsigned char terminalString[ sizeof( gaHardwareParametersDelimiterString_c )];
  } HardwareParameters_t;

 
#elif defined(PROCESSOR_MC1323X) 
typedef struct HardwareParameters_tag {
  unsigned char initialString[ sizeof( gaHardwareParametersDelimiterString_c )];
  uint8_t Bus_Frequency_In_MHz;
  /* SOMC1 register should be set by application */
  //uint8_t SOMC1_value;
  uint8_t Xtal_Trim;
  uint8_t SCGC1_value;
  uint8_t SCGC2_value;
  uint8_t MAC_Address[8];
  uint8_t defaultPowerLevel;
  uint8_t useDualAntenna;
  uint8_t ccaThreshold;
  uint8_t ccaOffset;
  uint8_t paPowerLevelLimits[16];  
  unsigned char terminalString[ sizeof( gaHardwareParametersDelimiterString_c )];
  } HardwareParameters_t;


#else
// PROCESSOR_HCS08
typedef struct HardwareParameters_tag {
  unsigned char initialString[ sizeof( gaHardwareParametersDelimiterString_c )];
  uint8_t Bus_Frequency_In_MHz;
  uint16_t Abel_Clock_Out_Setting;
  uint16_t Abel_HF_Calibration;
  uint8_t NV_ICGC1;
  uint8_t NV_ICGC2;
  uint8_t NV_ICGFLTU;
  uint8_t NV_ICGFLTL;
  uint8_t MAC_Address[8];
  uint8_t defaultPowerLevel;
  uint8_t useDualAntenna;
  uint8_t paPowerLevelLimits[16];  
  unsigned char terminalString[ sizeof( gaHardwareParametersDelimiterString_c )];
  } HardwareParameters_t;
#endif
/*****************************************************************************
******************************************************************************
* Public memory declarations
******************************************************************************
*****************************************************************************/

#pragma CONST_SEG FREESCALE_PROD_DATA
/*Do not change order!!!*/
extern FreescaleVersionStrings_t const gFreescaleVersionStrings;
extern HardwareParameters_t const gHardwareParameters;
#pragma CONST_SEG DEFAULT
#endif _NV_DATA_H_
