/************************************************************************************
* This header file is provided as part of the interface to the Freescale 802.15.4
* MAC and PHY layer.
*
* This file contains initialization related functionality.
*
* (c) Copyright 2007, Freescale, Inc.  All rights reserved.
*
* No part of this document must be reproduced in any form - including copied,
* transcribed, printed or by any electronic means - without specific written
* permission from Freescale Semiconductor.
*
************************************************************************************/
#ifndef _MACPHYINIT_H_
#define _MACPHYINIT_H_

  // The platform configuration file must be included for
  // gVersionStringCapability_d, gVersionStringBegin_d and gVersionStringEnd_d
#include "PlatformToMacPhyConfig.h"

  // Max 4 characters
#define Database_Label_Version	"209"
#define MAC_Version_No		"209"
#define PHY_Version_No		"209"

#if gVersionStringCapability_d

#pragma gVersionStringMacBegin_d
extern const uint8_t Freescale_Copyright[54];
extern const uint8_t Firmware_Database_Label[40];
extern const uint8_t MAC_Version[47];
#pragma gVersionStringMacEnd_d


#pragma gVersionStringPhyBegin_d
extern const uint8_t PHY_Version[47];
#pragma gVersionStringPhyEnd_d



#endif // gVersionStringCapability_d

/************************************************************************************
* Stack init function called from platform upon reset. 
*
* Interface assumptions:
*   Typically called from Init_802_15_4() in platform
*
* Return value:
*   None
*
************************************************************************************/
void MacPhyInit_Initialize
(
  bool_t relaxedTiming,       // Specifies if the MAC PHY should run with relaxed
                              // timing (LIFS/SIFS) due to a platform/mcu/clock 
                              // frequency timing issue.
  uint16_t phyRfCalibration,  // MSB = CCA Energy detect threshold: abs(power in dBm)*2,
                              //       0x96=-75dBm
                              // LSB = Power compensation Offset added to RSSI. 
                              //       Typically 0x74 FFJ/JK 13/01/04 (Abel 013)
  uint8_t defaultPowerLevel,  // The default power level to be used for all
                              // transmissions. Can be dynamically changed using the 
                              // ASP command gAppAspSetPowerLevelReq_c. The default
                              // value is specific for the Transceiver used 
                              // (gTransceiverType_d). For gMC1319x_c and gMC1321x_c,
                              // the default value is 0xBC.
  uint8_t *paPowerLevelLimits,// Pointer to an array of 16 power level limits for each
                              // specific channel. The MAC ensures that the actual
                              // power level of a specific channel will not exceed this
                              // limit, event though the defaultPowerLevel is greater.
                              // Index 0 corresponds to channel 11. The default values
                              // are {0xFF, 0xFF, ..., 0xFF} in which case this array
                              // will have no effect. The array must contain values
                              // for 16 channels. If this pointer is set to NULL, 
                              // this functionality will be disabled, and the array
                              // will not be used to limit the power levels.
  bool_t useDualAntenna       // Specifies if the MAC should use single or dual antenna 
                              // for Tx/Rx. If the value is FALSE (0), the MAC will use
                              // a single antenna and if it is TRUE (1), the MAC will
                              // use two antennas.
);

/************************************************************************************
* Writes the 8 byte IEEE extended address.
*
* Interface assumptions:
*   None
*
* Return value:
*   None
*
************************************************************************************/
void MacPhyInit_WriteExtAddress
(
  uint8_t *pExtendedAddress // A pointer to the 8 byte IEEE address. The address is
                            // copied into local MAC RAM.
);

/************************************************************************************
* Reads the 8 byte IEEE extended address.
*
* Interface assumptions:
*   None
*
* Return value:
*   A pointer to the extended address.
*
************************************************************************************/
uint8_t *MacPhyInit_ReadExtAddress(void);


#endif /* _MACPHYINIT_H_ */