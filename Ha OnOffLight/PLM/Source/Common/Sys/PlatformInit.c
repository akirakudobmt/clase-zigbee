/************************************************************************************
* This file contains Platform specific implentations of BTA functions.
* For the Windows Simulator Platform.
*
*
* (c) Copyright 2006, Freescale Semiconductor, Inc. All rights reserved.
*
*
* No part of this document must be reproduced in any form - including copied,
* transcribed, printed or by any electronic means - without specific written
* permission from Freescale Semiconductor.
************************************************************************************/


/************************************************************************************
* Includes
************************************************************************************/
#include "IoConfig.h"
#include "PortConfig.h"
#include "IrqControlLib.h"
#include "Mc1319xDrv.h"
#include "Crt0.h"
#include "NV_Data.h"
#include "PowerManager.h"
#include "MacPhyInit.h"
#include "PlatformInit.h"

#include "FunctionLib.h"  // To write ext addr - pls remove this @KDO
//#include "MacConst.h" // To get aExtendedAddress - must be possible to access 

extern uint8_t aExtendedAddress[8];
/*************************************************************************************
*************************************************************************************
* External platform references
*************************************************************************************
************************************************************************************/
  // NOTE!!! This code is only added to get the symbols in the build. The linker will remove these symbols if not referenced.
  // Hope to find a better way 21.10.03 MVC 
#ifdef MEMORY_MODEL_BANKED  
typedef void __near(* __near ISR_func_t)(void);
#else
typedef void (*__near ISR_func_t)(void);
#endif /* MEMORY_MODEL_BANKED */
extern const ISR_func_t ISR_vectors[]; 
extern const ISR_func_t Reset_vector[];	 // The system reset vector

/*************************************************************************************
*************************************************************************************
* Private functions
*************************************************************************************
************************************************************************************/


/************************************************************************************
*************************************************************************************
* Public function(s)
*************************************************************************************
************************************************************************************/



/************************************************************************************
* Perform the complete GPIO port initialization
*
* Interface assumptions:
*
* Return value:
*   NONE
*
* Revision history:
*
************************************************************************************/
void PlatformPortInit(void)
{
  // Platform ports default configuration/initialization
  // Setup port A
  mSETUP_PORT_A

  // Setup Port B
  mSETUP_PORT_B

  // Setup port C
  mSETUP_PORT_C

  // Setup port D
  mSETUP_PORT_D

  // Setup port E
  mSETUP_PORT_E

  // Setup port F
  mSETUP_PORT_F

  // Setup port G
  mSETUP_PORT_G

#ifdef PROCESSOR_QE128  
  // Setup port H  
  mSETUP_PORT_H
#endif  
#if (gTarget_UserDefined_d == 1) 
  mSETUP_TRANCEIVER_PORT
  mSETUP_SWICH_PORT
  mSETUP_LED_PORT
  mSETUP_SCIxPINS
#endif 

#ifdef I_AM_A_SNIFFER
  TPM1SC = 0x0c; // Use BUSCLK and prescale with factor 16 (to get 1 us)
  TPM1C1SC = 0x04; // Disable interrupt, Input capture, Capture on rising edge
#endif I_AM_A_SNIFFER

}


/************************************************************************************
* Initialize platform specific stuff
* NVRAM, Bootloader, MC1319x driver etc.
*
* Interface assumptions:
*
*   NOTE!!! This First section of this function code is added to ensure that the symbols 
*   are included in the build. The linker will remove these symbols if not referenced.
*   Hope to find a better way 21.10.03 MVC 
*
* Return value:
*   NONE
*
* Revision history:
*
************************************************************************************/
void Platform_Init(void) {

volatile uint8_t dummy;
/************* Symbol include section begin *************************/

/*Touch gFreescaleVersionStrings to ensure that the linker includes it in code */
  dummy = gFreescaleVersionStrings.MCU_Version;

/* Touch ISR_Vectors to ensure that the linker includes it in code */
  dummy = (uint8_t)ISR_vectors[0];    
  dummy = (uint8_t)none_volatile_struct.nv_backkey[0];  
  dummy = (uint8_t)Reset_vector[0];
  
#ifdef FLASH_LIB_ENABLED
  	// Add this compiler define if the flash library is included in the application
	NV_Flash_Setup();
#endif FLASH_LIB_ENABLED

 


/************* Symbol include section end ****************************/


    // Init MCU specifics: GPIO ports, SPI - and reset MC1319x
  PlatformPortInit();   // Initializes GPIO port settings.
  #ifndef PROCESSOR_MC1323X
  MC1319xDrv_Init();    // Init SPI for MC1319x driver
  MC1319xDrv_Reset();   // Reset MC1319x
  #endif  
  
    // Ensure MCU clk is stable
  PowerManager_WaitForPowerUp();
}


/************************************************************************************
* Initialize platform and stack. 
*
* This function is the main initialization procedure, which will be called from the 
* startup code of the project. 
* If the code is build as a stand-alone application, this function is
* called from main() below. 
*
* Interface assumptions:
*
* Return value:
*   NONE
*
************************************************************************************/
void Init_802_15_4(void)
{
  bool_t relaxedTiming;  

  relaxedTiming = gHardwareParameters.Bus_Frequency_In_MHz<12;
  
  // Init 802.15.4 MAC/PHY stack
#ifndef PROCESSOR_MC1323X
  MacPhyInit_Initialize(relaxedTiming, gHardwareParameters.Abel_HF_Calibration,
  gHardwareParameters.defaultPowerLevel, (uint8_t*)gHardwareParameters.paPowerLevelLimits, gHardwareParameters.useDualAntenna);
#else
  MacPhyInit_Initialize(relaxedTiming, 0,
  gHardwareParameters.defaultPowerLevel, (uint8_t*)gHardwareParameters.paPowerLevelLimits, FALSE);    
  // Aplly NV_Data radio settings
  PP_PHY_CCATHRSHD = gHardwareParameters.ccaThreshold;
  PP_PHY_CCAOFSCMP = gHardwareParameters.ccaOffset;  
#endif  

#ifndef PROCESSOR_MC1323X  
  FLib_MemCpy(aExtendedAddress, ( void * ) gHardwareParameters.MAC_Address, 8);
#else
  MacPhyInit_WriteExtAddress((uint8_t*)&gHardwareParameters.MAC_Address[0]); 
#endif
}

/************************************************************************************
* Initialize and setups the PPD capability
*
************************************************************************************/


#ifdef PROCESSOR_MC1323X
void PPD_InitPpdMode(void) 
{        
  #if (gPpdMode_d == gPpdModeOFF_d)
    IoIndirectWrite(0x17, 0x00);      
  #elif (gPpdMode_d == gPpdModeON_LowThrs)
    IoIndirectWrite(0x17, 0xF9);  
    IoIndirectWrite(0x45, 0x17);      
  #elif (gPpdMode_d == gPpdModeON_MidThrs)
    IoIndirectWrite(0x17, 0xF9);  
    IoIndirectWrite(0x45, 0x16);        
  #elif (gPpdMode_d == gPpdModeON_HighThrs)
    IoIndirectWrite(0x17, 0xF9);  
    IoIndirectWrite(0x45, 0x15);      
  #endif //gPpdMode_d
}
#endif //PROCESSOR_MC1323X



