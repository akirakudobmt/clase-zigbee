/*****************************************************************************
* Data definitions for the application client of the NV storage module.
*
* c) Copyright 2008, Freescale, Inc. All rights reserved.
*
*
* No part of this document must be reproduced in any form - including copied,
* transcribed, printed or by any electronic means - without specific written
* permission from Freescale Semiconductor.
*
*****************************************************************************/

#include "EmbeddedTypes.h"
#include "AppToPlatformConfig.h"
#include "NVM_Interface.h"
#include "portconfig.h"
#include "NV_Data.h"
#include "FunctionLib.h"
#include "Platforminit.h"

#if gBeeStackIncluded_d
#include "BeeStackConfiguration.h"
#include "BeeStack_Globals.h"
//#include "BeeStackCalculations.h"
#include "BeeStackParameters.h"
/* IMPORTANT: See the comments at the beginning of NVM_Interface.h. */
#include "ASL_UserInterface.h"
#include "HaProfile.h"
#endif



/*****************************************************************************
******************************************************************************
* Private macros
******************************************************************************
*****************************************************************************/
#if defined(PROCESSOR_QE128)
 #define SYSTEM_CLOCK_16MHZ    /* System Clock for QE 128 */
#elif defined(PROCESSOR_MC1323X)
 #define SYSTEM_CLOCK_16MHZ    /* System Clock for MC1323X */
#else
// PROCESSOR_HCS08
 #define SYSTEM_CLOCK_16MHZ    /* System Clock for HCS08 */
#endif
/*****************************************************************************
******************************************************************************
* Public memory declarations
******************************************************************************
*****************************************************************************/

/* The linker command file places this structure in a fixed location, to
 * make it easy to find it during manufacturing.
 * WARNING: The size of the FREESCALE_VERSION_STRINGS section in the linker
 * command file must be manually coordinated with the size of the
 * gFreescaleVersionStrings struct.
 */
#pragma CONST_SEG FREESCALE_PROD_DATA
/*NOTE: DO NOT CHANGE ORDER OF THESE 2 structures*/
FreescaleVersionStrings_t const gFreescaleVersionStrings = {
  0x0006,                               /* uint16_t NV_RAM_Version; */

  /* uint8_t MAC_Version[4]; */
  0x01,0xFF,0xFF,0xFF,
  /* uint8_t PHY_Version[4]; */
  0x02,0xFF,0xFF,0xFF,
  /* uint8_t STACK_Version[4]; */
  0x03,0xFF,0xFF,0xFF,

  /* uint8_t APP_Version[4]; */
  0x04,0xFF,0xFF,0xFF,

  /* uint8_t HWName_Revision[4]; */
  0x05,0xFF,0xFF,0xFF,
  /* uint8_t SerialNumber[4]; */
  0x06,0x02,0x03,0x04,

  0x020F,                               /* uint16_t ProductionSite; */
  0x01,                                 /* uint8_t CountryCode; */
  0x04,                                 /* uint8_t ProductionWeekCode; */
  0x04,                                 /* uint8_t ProductionYearCode; */
  0x00,                                 /* uint8_t MCU_Manufacture; */
  0x02,                                 /* uint8_t MCU_Version; */
  0x00                                  /*padding*/
};

#if defined(PROCESSOR_QE128)

/* Used to initialize gNvMacDataSet if no NV storage copy is found. */
HardwareParameters_t const gHardwareParameters = {
  gaHardwareParametersDelimiterString_c,    /* initialString */

  #ifdef SYSTEM_CLOCK_8MHZ
    0x08, /* 8 MHz */
    /* uint16_t Abel_Clock_Out_Setting; */
      CLOCK_OUT_SETTING,
    /* uint16_t Abel_HF_Calibration; */
      HF_CALIBRATION,
    /* ICG clock  */
      0x08,                                 /* uint8_t NV_ICSC1; */
    /* 0x20 => CPU clk=32 MHz, Buc clk = 16 MHz */
      0x00,                                 /* uint8_t NV_ICSC2; */
    /* ICS TRIM */
      0x9F,                                 /* uint8_t NV_ICSTRM; */
    /* ICSSC */    
      0x00,                                 /* uint8_t NV_ICSSC; */
  #endif SYSTEM_CLOCK_8MHZ

  #ifdef SYSTEM_CLOCK_16MHZ
    0x10, /* 16 MHz */
    /* uint16_t Abel_Clock_Out_Setting; */
      CLOCK_OUT_SETTING,
    /* uint16_t Abel_HF_Calibration; */
      HF_CALIBRATION,
    /* ICG clock  */
      0x08,                                 /* uint8_t NV_ICSC1; */
      //0x00,                                 /* uint8_t NV_ICSC1; */
    /* 0x20 => CPU clk=32 MHz, Buc clk = 16 MHz */
      0x00,                                 /* uint8_t NV_ICSC2; */
    /* ICS TRIM */
      0x9F,                                 /* uint8_t NV_ICSTRM; */
    /* ICSSC */    
      0x40,                                 /* uint8_t NV_ICSSC; */
  #endif SYSTEM_CLOCK_16MHZ

  #ifdef SYSTEM_CLOCK_16780MHZ
    0x10, /* 16 MHz */
    /* uint16_t Abel_Clock_Out_Setting; */
      CLOCK_OUT_SETTING,
    /* uint16_t Abel_HF_Calibration; */
      HF_CALIBRATION,
    /* ICG clock  */
    0x04,                                 /* uint8_t NV_ICSC1; */
    /* 0x20 => CPU clk=32 MHz, Buc clk = 16 MHz */
    0x00,                                 /* uint8_t NV_ICSC2; */

    /* ICS TRIM */
    0xB0,                                 /* uint8_t NV_ICSTRM; */
    /* ICSSC */    
    0x50,                                 /* uint8_t NV_ICSSC; */
  #endif SYSTEM_CLOCK_16780MHZ

    /* uint8_t MAC_Address[8]; */
    0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,

    0xBC,                                 /* uint8_t defaultPowerLevel; */

  #ifdef TARGET_TOROWEAP
    #if (gTargetQE128EVB_d == 1) 
     FALSE,
    #else      
     TRUE,                                 /* uint8_t useDualAntenna; */
    #endif 
  #elif gTarget_UserDefined_d  
    gUseExternalAntennaSwitch_c, 
  #else
     FALSE,                                /* uint8_t useDualAntenna; */
  #endif /* TARGET_TOROWEAP */

    CHANNEL_PA,
    gaHardwareParametersDelimiterString_c,    /* terminalString */
  };


#elif defined(PROCESSOR_MC1323X) 
//tbd
/* Used to initialize gNvMacDataSet if no NV storage copy is found. */
HardwareParameters_t const gHardwareParameters = {
  gaHardwareParametersDelimiterString_c,    /* initialString */

  #ifdef SYSTEM_CLOCK_8MHZ
    0x08, /* 8 MHz Bus_Frequency_In_MHz */
    /* SOMC1 register should be set by application */
    //0x2C, // SOMC1_value (RDIV = 1;XTAL0EN = 0;1KHZ_EN =1; XTAL1EN = 1;TAL1_TST_OUT = 0;CLKS_TST_OUT = 0)    
    0x9E, // Xtal_Trim
  #endif SYSTEM_CLOCK_8MHZ

  #ifdef SYSTEM_CLOCK_16MHZ
    0x10, /* 16 MHz Bus_Frequency_In_MHz */
    /* SOMC1 register should be set by application */
    //0x0C, // SOMC1_value (RDIV = 0;XTAL0EN = 0;1KHZ_EN =1; XTAL1EN = 1;TAL1_TST_OUT = 0;CLKS_TST_OUT = 0)
    0x9E, // Xtal_Trim                                 /* uint8_t NV_ICSSC; */
  #endif SYSTEM_CLOCK_16MHZ
    0xff,//SCGC1
    0xff,//SCGC2
   /* uint8_t MAC_Address[8]; */
    0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,
    0xB,                                 /* uint8_t defaultPowerLevel; */
    FALSE,/* uint8_t useDualAntenna; */
    gaMC1323x_CCA_Threshold_c, /* ccaThreshold */
    gaMC1323x_CCA_Offset_c,   /* ccaOffset */
    CHANNEL_PA,
    gaHardwareParametersDelimiterString_c,    /* terminalString */
  };


#else
// PROCESSOR_HCS08

/* The MAC data set is special. Some of the data in it is used to initialize
 * the CPU and other hardware, before the C main() program begins, and is
 * also used later by the main C code.
 * The C initialized data section in memory has not been initialized yet,
 * so we can't depend on anything in RAM. The .bss (zero'ed memory) and
 * initialized data section will be initialized later, so we can't store
 * anything there for the main C code to use.
 * This structure is in RAM, in it's own linker section, which is marked to
 * not be initialized at all by the usual C runtime .bss and initialzed data
 * procedures. Code in the C runtime startup (crt0.c) handles initializing
 * it in a non-standard way.
 *
 * WARNING: The size of the HARDWARE_INIT_PARAMETERS section in the linker
 * command file must be manually coordinated with the size of the
 * HardwareInitParameters_t struct.
 */
 
HardwareParameters_t const gHardwareParameters = {
  gaHardwareParametersDelimiterString_c,    /* initialString */

#ifdef SYSTEM_CLOCK_8MHZ
  /* 8 MHz  */
  0x08,                                 /* uint8_t Bus_Frequency_In_MHz; */
  /* uint16_t Abel_Clock_Out_Setting; */
    CLOCK_OUT_SETTING,
  /* uint16_t Abel_HF_Calibration; */
    HF_CALIBRATION,
  /* ICG  */
  0x18,                                 /* uint8_t NV_ICGC1; */
  /* 0x00 => CPU clk=16 MHz, Buc clk = 8 MHz  */
  0x00,                                 /* uint8_t NV_ICGC2; */

  /* ICG Filter */
  0x02,                                 /* uint8_t NV_ICGFLTU; */
  0x40,                                 /* uint8_t NV_ICGFLTL; */
#endif SYSTEM_CLOCK_8MHZ

#ifdef SYSTEM_CLOCK_12MHZ
  /* 12 MHz */
  0x0C,                                 /* uint8_t Bus_Frequency_In_MHz; */
  /* uint16_t Abel_Clock_Out_Setting; */
    CLOCK_OUT_SETTING,
  /* uint16_t Abel_HF_Calibration; */
    HF_CALIBRATION,
  /* ICG clock  */
  0x18,                                 /* uint8_t NV_ICGC1; */
  /* 0x41 => CPU clk=24 MHz, Buc clk = 12 MHz  */
  0x41,                                 /* uint8_t NV_ICGC2; */

  /* ICG Filter */
  0x02,                                 /* uint8_t NV_ICGFLTU; */
  0x40,                                 /* uint8_t NV_ICGFLTL; */
#endif SYSTEM_CLOCK_12MHZ

#ifdef SYSTEM_CLOCK_16MHZ
  0x10, /* 16 MHz */
  /* uint16_t Abel_Clock_Out_Setting; */
    CLOCK_OUT_SETTING,
  /* uint16_t Abel_HF_Calibration; */
    HF_CALIBRATION,
  /* ICG clock  */
  0x18,                                 /* uint8_t NV_ICGC1; */
  /* 0x20 => CPU clk=32 MHz, Buc clk = 16 MHz */
  0x20,                                 /* uint8_t NV_ICGC2; */

  /* ICG Filter */
  0x02,                                 /* uint8_t NV_ICGFLTU; */
  0x40,                                 /* uint8_t NV_ICGFLTL; */
#endif SYSTEM_CLOCK_16MHZ

#ifdef SYSTEM_CLOCK_16780MHZ
  /* 16 MHz -> ~16,78 MHz */
  0x10,
  /* uint16_t Abel_Clock_Out_Setting; */
    CLOCK_OUT_SETTING,
  /* uint16_t Abel_HF_Calibration; */
    HF_CALIBRATION,
  /* ICG clock  */
  0x18,                                 /* uint8_t NV_ICGC1; */
  /* 0x60 => CPU clk=32,78 MHz, Buc clk = 16,39 MHz */
  0x60,                                 /* uint8_t NV_ICGC2; */

  /* ICG Filter */
  0x02,                                 /* uint8_t NV_ICGFLTU; */
  0x40,                                 /* uint8_t NV_ICGFLTL; */
#endif SYSTEM_CLOCK_16780MHZ

  /* uint8_t MAC_Address[8]; */
  0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,

  0xBC,                                 /* uint8_t defaultPowerLevel; */

#ifdef TARGET_TOROWEAP      
  TRUE,                                 /* uint8_t useDualAntenna; */                                
#elif gTarget_UserDefined_d  
  gUseExternalAntennaSwitch_c, 
#else
  FALSE,                                /* uint8_t useDualAntenna; */
#endif /* TARGET_TOROWEAP */

  CHANNEL_PA,
  gaHardwareParametersDelimiterString_c,    /* terminalString */
};

#endif /* PROCESSOR_QE128 */


#pragma CONST_SEG DEFAULT



/*****************************************************************************
******************************************************************************
* Public memory declarations
******************************************************************************
*****************************************************************************/

/*****************************************************************************************
* WARNING: ANY CHANGE INTO ANY OF THE DATA SETS WILL BE REFLECTED INTO THE FUNCITON WHO
* HANDLES THE NV DATA ZdoNwkMng_SaveToNvm, SO PLEASE ADJUST ANY CHANGE MADE TO DATA SETS
* INTO THE FUNCTION LOCATED AT ZdoNwkManager.c
******************************************************************************************/
/* Define the NV storage data sets. Each table must end with "{NULL, 0}". */

/* If the NV storage module's self test flag is on, the NV code will use */
/* it's own internal data. */
#if !gNvSelfTest_d
#if gBeeStackIncluded_d
/* This data set contains network layer variables to be preserved across resets */
NvDataItemDescription_t const gaNvNwkDataSet[] = {
  {&gNwkData,               sizeof(gNwkData)},
  {gaNeighborTable,         sizeof(neighborTable_t) * gNwkInfobaseMaxNeighborTableEntry_c },
  {&gBeeStackConfig,        sizeof(beeStackConfigParams_t)},
  {NULL, 0}       /* Required end-of-table marker. */
};

/* This data set holds the Address map data and the Aps secure material set. */
NvDataItemDescription_t const gaNvApsAndSecurityDataSet[] = {
  {gaApsAddressMap, sizeof(zbAddressMap_t) * gApsMaxAddrMapEntries_c},
  {gaApsAddressMapBitMask, sizeof(uint8_t) * gApsAddressMapBitMaskInBits_c},
#if gApsLinkKeySecurity_d
  {gaApsDeviceKeyPairSet, sizeof(zbApsDeviceKeyPairSet_t) * gApsMaxSecureMaterialEntries_c},
#endif
  {NULL, 0}       /* Required end-of-table marker. */
};

/* This data set holds the NwkSecure Frame counters and the key table for APS security. */
#if gStandardSecurity_d || gHighSecurity_d
NvDataItemDescription_t const gaNvSecurityDataSet[] = {
  {gaIncomingFrameCounterSet1, sizeof(zbIncomingFrameCounterSet_t) * gNwkInfobaseMaxNeighborTableEntry_c},
  {gaIncomingFrameCounterSet2, sizeof(zbIncomingFrameCounterSet_t) * gNwkInfobaseMaxNeighborTableEntry_c},
#if (gApsLinkKeySecurity_d)
  {gaApsKeySet,                sizeof(zbAESKey_t) * gApsMaxLinkKeys_c},
#endif
  {NULL, 0}       /* Required end-of-table marker. */
};
#endif

#endif


/* different for each application */
extern NvDataItemDescription_t const gaNvAppDataSet[];

/*****************************************************************************************
* WARNING: ANY CHANGE INTO ANY OF THE DATA SETS WILL BE REFLECTED INTO THE FUNCITON WHO
* HANDLES THE NV DATA ZdoNwkMng_SaveToNvm, SO PLEASE ADJUST ANY CHANGE MADE TO DATA SETS
* INTO THE FUNCTION LOCATED AT ZdoNwkManager.c
******************************************************************************************/
/* Table of data sets. Required by the NV storage module. */
/* There must be gNvNumberOfDataSets_c entries in this table. If the */
/* number of entries changes, gNvNumberOfDataSets_c must be changed. */
NvDataSetDescription_t const NvDataSetTable[gNvNumberOfDataSets_c] = {
  {gNvDataSet_Nwk_ID_c, gaNvNwkDataSet},
  {gNvDataSet_App_ID_c, gaNvAppDataSet},
  {gNvDataSet_Aps_Sec_ID_c, gaNvApsAndSecurityDataSet}
#if gStandardSecurity_d || gHighSecurity_d
  ,{gNvDataSet_Sec_ID_c, gaNvSecurityDataSet}
#endif
};
#endif                                  /* #if !gNvSelfTest_d */

