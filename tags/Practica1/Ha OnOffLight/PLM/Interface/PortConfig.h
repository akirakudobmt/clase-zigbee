/************************************************************************************
* Mapping of the IO ports and pins.
*
* (c) Copyright 2006, Freescale Semiconductor, Inc. All rights reserved.
*
*
* No part of this document must be reproduced in any form - including copied,
* transcribed, printed or by any electronic means - without specific written
* permission from Freescale Semiconductor.
************************************************************************************/

#ifndef _PORTCONFIG_H_
#define _PORTCONFIG_H_

#include "IoConfig.h"
#include "AppToPlatformConfig.h"

#if gTargetAxiomGB60_d == 1
#include "TargetAxiomGB60.h"
#endif /* gTargetAxiomGB60_d */

#if gTargetSARD_d == 1
#include "TargetMC1319xSARD.h"
#endif /* gTargetSARD_d */

#if gTargetEVB_d == 1
#include "TargetMC1319xEVB.h"
#endif

#if gTargetMC13213NCB_d == 1
#include "TargetMC1321xNCB.h"
#endif /* gTargetMC13213NCB_d */

#if gTargetMC13213SRB_d == 1
#include "TargetMC1321xSRB.h"
#endif /* gTargetMC13213SRB_d */

#if gTargetQE128EVB_d == 1
#include "TargetQE128xEVB.h"
#endif

#if gTargetMC1323xREM_d == 1
#include "TargetMC1323xREM.h"
#endif

#if gTargetMC1323xRCM_d == 1
#include "TargetMC1323xRCM.h"
#endif

#if gTarget_UserDefined_d == 1

  #define NotDefined 

  /* macro used for enabling Low Noise Amplifier*/
  #define EnableLNA  NotDefined
  
  /* macro used for enabling Power Amplifier*/
  #define EnablePA   NotDefined
  
  /* macro used for disabling Low Noise Amplifier*/
  #define DisableLNA NotDefined
  
  /* macro used for disabling Power Amplifier*/
  #define DisablePA NotDefined

  #define MC1319x    0
  #define MC1321x    1  
  #define MC1320x    MC1321x  
  #define MC1323x    2
  #define gUserTransceiverType_d MC1321x  
  #define gUseQE128_d   FALSE
  #define gUseMC1323x_d FALSE

  #define gUseExternalAntennaSwitch_c TRUE
  
  /* Define LED port mapping */
  #define LED1_PORT  PTDD
  #define LED2_PORT  PTDD
  #define LED3_PORT  PTDD
  #define LED4_PORT  PTDD
  
  #define LED1_PIN  0x10
  #define LED2_PIN  0x20
  #define LED3_PIN  0x40
  #define LED4_PIN  0x80

  #define LED5_PORT  PTCD
  #define LED5_PIN   0x10
  #define LED5_MASK  (LED5_PIN)
                                                
  #define LED1ON            LED1_PORT &= ~LED1_PIN;
  #define LED1OFF           LED1_PORT |= LED1_PIN;
  #define LED1TOGGLE        LED1_PORT ^= LED1_PIN;

  #define LED2ON            LED2_PORT &= ~LED2_PIN;
  #define LED2OFF           LED2_PORT |= LED2_PIN;
  #define LED2TOGGLE        LED2_PORT ^= LED2_PIN;

  #define LED3ON            LED3_PORT &= ~LED3_PIN;
  #define LED3OFF           LED3_PORT |= LED3_PIN;
  #define LED3TOGGLE        LED3_PORT ^= LED3_PIN;

  #define LED4ON            LED4_PORT &= ~LED4_PIN;
  #define LED4OFF           LED4_PORT |= LED4_PIN;
  #define LED4TOGGLE        LED4_PORT ^= LED4_PIN;

  #define LED5ON            LED5_PORT &= ~LED5_PIN;
  #define LED5OFF           LED5_PORT |= LED5_PIN;
  #define LED5TOGGLE        LED5_PORT ^= LED5_PIN;

  /* Turn off LEDs */
  #define TURN_OFF_LEDS     LED1OFF;\
                            LED2OFF;\
                            LED3OFF;\
                            LED4OFF;
  /* Turn on LEDs */
  #define TURN_ON_LEDS      LED1ON;\
                            LED2ON;\
                            LED3ON;\
                            LED4ON;

  /* Define PORT PINS mapping */
  #define PORT_PINS         PTAD
  #define PORT1_PIN         0x01
  #define PORT2_PIN         0x02
  #define PORT3_PIN         0x04
  #define PORT4_PIN         0x08
  #define PORT_PINS_MASK    (PORT1_PIN | PORT2_PIN | PORT3_PIN | PORT4_PIN)

  /* Define PCB switch mapping */
  
  #define SWITCH1_PORT       PTAD
  #define SWITCH2_PORT       PTAD
  #define SWITCH3_PORT       PTAD
  #define SWITCH4_PORT       PTAD
  
  #define mSWITCH1_MASK     0x04
  #define mSWITCH2_MASK     0x08
  #define mSWITCH3_MASK     0x10
  #define mSWITCH4_MASK     0x20
  #define mSWITCH_MASK      (mSWITCH1_MASK | mSWITCH2_MASK |  mSWITCH3_MASK | mSWITCH4_MASK)
  
  #define mSWITCH_PORT_GET  (((mSWITCH1_PORT_c & gSWITCH1_MASK_c) ^ gSWITCH1_MASK_c) |\
                             ((mSWITCH2_PORT_c & gSWITCH2_MASK_c) ^ gSWITCH2_MASK_c) |\
                             ((mSWITCH3_PORT_c & gSWITCH3_MASK_c) ^ gSWITCH3_MASK_c) |\
                             ((mSWITCH4_PORT_c & gSWITCH4_MASK_c) ^ gSWITCH4_MASK_c))

  /* Define HW pin mapping */
  #define gMC1319xAttnPort     PTDD
  #define gMC1319xRxTxPort     PTDD
  #define gMC1319xResetPort    PTDD
  
  #define gMC1319xAttnMask_c   0x01
  #define gMC1319xRxTxMask_c   0x02
  #define gMC1319xResetMask_c  0x08

  #define gMC1319xGPIO1Port    PTED
  #define gMC1319xGPIO2Port    PTED
  #define gMC1319xAntSwPort    PTBD

  #define gMC1319xGPIO1Mask_c  0x80
  #define gMC1319xGPIO2Mask_c  0x40
  #define gMC1319xAntSwMask_c  0x00
  
  #ifdef PROCESSOR_QE128  
	  #define gMC1319xSPIPort      PTBD

	  #define gMC1319xSpiTxD1Mask_c  0x01
	  #define gMC1319xSpiRxD1Mask_c  0x02
	  #define gMC1319xSpiSsMask_c    0x20
	  #define gMC1319xSpiMisoMask_c  0x10
	  #define gMC1319xSpiMosiMask_c  0x08
	  #define gMC1319xSpiSpsckMask_c 0x04
  #else
	  #define gMC1319xSPIPort      PTED

	  #define gMC1319xSpiTxD1Mask_c  (1<<0)
	  #define gMC1319xSpiRxD1Mask_c  (1<<1)
	  #define gMC1319xSpiSsMask_c    (1<<2)
	  #define gMC1319xSpiMisoMask_c  (1<<3)
	  #define gMC1319xSpiMosiMask_c  (1<<4)
	  #define gMC1319xSpiSpsckMask_c (1<<5)
  #endif
  
  #ifdef PROCESSOR_QE128
    #define PORT_OFFSET       0x1840
    #define OFFSET(PORT_D)    ((((uint16_t)(&PORT_D)) << 1) + PORT_OFFSET)
    #define mPORT_DD(PORT_D)  (*((volatile unsigned char*)(&PORT_D + 1)))
    #define mPORT_PE(PORT_D)  (*((volatile unsigned char*)(OFFSET(PORT_D))))
    #define mPORT_SE(PORT_D)  (*((volatile unsigned char*)(OFFSET(PORT_D) + 1)))
    
  #elif defined(PROCESSOR_MC1323X) 
    #define PORT_OFFSET       0x1830
    #define OFFSET(PORT_D)    ((((uint16_t)(&PORT_D)) << 1) + PORT_OFFSET)
    #define mPORT_DD(PORT_D)  (*((volatile unsigned char*)(&PORT_D + 1)))
    #define mPORT_PE(PORT_D)  (*((volatile unsigned char*)(OFFSET(PORT_D))))
    #define mPORT_SE(PORT_D)  (*((volatile unsigned char*)(OFFSET(PORT_D) + 1)))
  #else
    #define mPORT_PE(PORT_D)  (*((volatile unsigned char*)(&PORT_D+1)))
    #define mPORT_SE(PORT_D)  (*((volatile unsigned char*)(&PORT_D+2)))
    #define mPORT_DD(PORT_D)  (*((volatile unsigned char*)(&PORT_D+3)))
  #endif
    
  /* Define HW port mapping (Refer to MCU IO spreadsheet or schematic for details) */
  
  /* Port A - Not used by MC1319x driver. Available for application. */
  
  #define gPTADValue_c      0x00
  #define gPTAPEValue_c     0x3C
  #define gPTADDValue_c     0x42
  #define gPTADSValue_c     0x00
  
  #if defined(PROCESSOR_QE128) || defined(PROCESSOR_MC1323X)
    #define  mSETUP_PORT_A    PTAD  = gPTADValue_c;\
                              PTAPE = gPTAPEValue_c;\
                              PTADD = gPTADDValue_c;\
                              PTADS = gPTADSValue_c;
  #else
    #define  mSETUP_PORT_A    PTAD  = gPTADValue_c;\
                              PTAPE = gPTAPEValue_c;\
                              PTADD = gPTADDValue_c;
  #endif

  /* Port B - Not used by MC1319x driver. Available for application. */
  
  #define gPTBDValue_c      0x00 // Port value
  #define gPTBPEValue_c     0x00 // Pullup Enable  1 PullUp enable 0 PullUp disable
  #define gPTBDDValue_c     0x00 // Data direction 1 output , 
  #define gPTBDSValue_c     0x00
  
  #if defined(PROCESSOR_QE128) || defined(PROCESSOR_MC1323X)
    #define  mSETUP_PORT_B    PTBD  = gPTBDValue_c;\
                              PTBPE = gPTBPEValue_c;\
                              PTBDD = gPTBDDValue_c;\
                              PTBDS = gPTBDSValue_c;
  #else                            
    #define  mSETUP_PORT_B    PTBD  = gPTBDValue_c;\
                              PTBPE = gPTBPEValue_c;\
                              PTBDD = gPTBDDValue_c;    
  #endif
  
  /* Port C - Not used by MC1319x driver. Available for application. */
  
  #define gPTCDValue_c      0x00
  #define gPTCPEValue_c     0x00
  #define gPTCDDValue_c     0x10
  #define gPTCDSValue_c     0x00

  #if defined(PROCESSOR_QE128) || defined(PROCESSOR_MC1323X)  
    #define  mSETUP_PORT_C    PTCD  = gPTCDValue_c;\
                              PTCPE = gPTCPEValue_c;\
                              PTCDD = gPTCDDValue_c;\
                              PTCDS = gPTCDSValue_c;
  #else                            
    #define  mSETUP_PORT_C    PTCD  = gPTCDValue_c;\
                              PTCPE = gPTCPEValue_c;\
                              PTCDD = gPTCDDValue_c;
  #endif

  /* Port D - Used by MC1319x driver. Add pins for application if needed. */
  
  #define gPTDDValue_c      0x02
  #define gPTDPEValue_c     0x00
  #define gPTDDDValue_c     0xFB
  #define gPTDDSValue_c     0x00
  
  #if defined(PROCESSOR_QE128) || defined(PROCESSOR_MC1323X)
    #define  mSETUP_PORT_D    PTDD  = gPTDDValue_c;\
                              PTDPE = gPTDPEValue_c;\
                              PTDDD = gPTDDDValue_c;\
                              PTDDS = gPTDDSValue_c;
  #else                              
    #define  mSETUP_PORT_D    PTDD  = gPTDDValue_c;\
                              PTDPE = gPTDPEValue_c;\
                              PTDDD = gPTDDDValue_c;
  #endif

  /* Port E - Used by MC1319x driver. Add pins for application if needed. */
  
  #define gPTEDValue_c      0x14
  #define gPTEPEValue_c     0x00
  #define gPTEDDValue_c     0xFF
  #define gPTEDSValue_c     0x00

  
  #ifdef PROCESSOR_QE128
    #define  mSETUP_PORT_E    PTED  = gPTEDValue_c;\
                              PTEPE = gPTEPEValue_c;\
                              PTEDD = gPTEDDValue_c;\
                              PTEDS = gPTEDSValue_c;
  #elif defined(PROCESSOR_MC1323X)
    #define  mSETUP_PORT_E 
  #else
    #define  mSETUP_PORT_E    PTED  = gPTEDValue_c;\
                              PTEPE = gPTEPEValue_c;\
                              PTEDD = gPTEDDValue_c;  
  #endif

  /* Port F - Not used by MC1319x driver. Available for application. */
  
  #define gPTFDValue_c      0x00
  #define gPTFPEValue_c     0x00
  #define gPTFDDValue_c     0x00
  #define gPTFDSValue_c     0x00
  
  #ifdef PROCESSOR_QE128
    #define  mSETUP_PORT_F    PTFD  = gPTFDValue_c;\
                              PTFPE = gPTFPEValue_c;\
                              PTFDD = gPTFDDValue_c;\
                              PTFDS = gPTFDSValue_c;
  #elif defined (PROCESSOR_MC1323X)                              
    #define  mSETUP_PORT_F
  #else
    #define  mSETUP_PORT_F    PTFD  = gPTFDValue_c;\
                              PTFPE = gPTFPEValue_c;\
                              PTFDD = gPTFDDValue_c;                              
  #endif
  
  // Port G - Not used by MC1319x driver. Available for application.
  
  #define gPTGDValue_c      0x00
  #define gPTGPEValue_c     0x00
  #define gPTGDDValue_c     0x00
  #define gPTGDSValue_c     0x00

  #ifdef PROCESSOR_QE128  
    #define mSETUP_PORT_G     PTGD  = gPTGDValue_c;\
                              PTGPE = gPTGPEValue_c;\
     						              PTGDD = gPTGDDValue_c;\
     						              PTGDS = gPTGDSValue_c;
  #elif defined(PROCESSOR_MC1323X)
    #define mSETUP_PORT_G
  #else
    #define mSETUP_PORT_G     PTGD  = gPTGDValue_c;\
                              PTGPE = gPTGPEValue_c;\
     						              PTGDD = gPTGDDValue_c;       						              
  #endif

  // Port H - Not used by MC1319x driver. Available for application.
  // Note only present on the QE.
  
  #define gPTHDValue_c      0x00
  #define gPTHPEValue_c     0x00
  #define gPTHDDValue_c     0x00
  #define gPTHDSValue_c     0x00
  
  #ifdef PROCESSOR_QE128
    #define mSETUP_PORT_H     PTHD  = gPTHDValue_c;\
                              PTHPE = gPTHPEValue_c;\
     						              PTHDD = gPTHDDValue_c;\
     						              PTHDS = gPTHDSValue_c;
  #endif
 #ifdef PROCESSOR_MC1323X 
  
  #define KBI_ENABLE_MATRIX    FALSE
  #define KBI_MATRIX_NUM_ROWS  0
  #define KBI_MATRIX_NUM_COLS  0 

 #endif   

 #define mSETUP_SWICH_PORT  mPORT_PE(SWITCH1_PORT) |= mSWITCH1_MASK;\
                            mPORT_PE(SWITCH2_PORT) |= mSWITCH2_MASK;\
                            mPORT_PE(SWITCH3_PORT) |= mSWITCH3_MASK;\
                            mPORT_PE(SWITCH4_PORT) |= mSWITCH4_MASK;\
                            mPORT_DD(SWITCH1_PORT) &= ~mSWITCH1_MASK;\
                            mPORT_DD(SWITCH2_PORT) &= ~mSWITCH2_MASK;\
                            mPORT_DD(SWITCH3_PORT) &= ~mSWITCH3_MASK;\
                            mPORT_DD(SWITCH4_PORT) &= ~mSWITCH4_MASK;
                            
  
 #ifdef PROCESSOR_MC1323X
  #define mSETUP_TRANCEIVER_PORT
 #else
  #define mSETUP_TRANCEIVER_PORT  gMC1319xResetPort &= ~gMC1319xResetMask_c;\
                                  gMC1319xAttnPort &= ~gMC1319xAttnMask_c;\
                                  gMC1319xRxTxPort |=  gMC1319xRxTxMask_c;\
                                  mPORT_DD(gMC1319xResetPort) |= gMC1319xResetMask_c;\
                                  mPORT_DD(gMC1319xAttnPort)  |= gMC1319xAttnMask_c;\
                                  mPORT_DD(gMC1319xRxTxPort)  |= gMC1319xRxTxMask_c;\
                                  mPORT_PE(gMC1319xGPIO1Port) &= ~gMC1319xGPIO1Mask_c;\
                                  mPORT_PE(gMC1319xGPIO2Port) &= ~gMC1319xGPIO2Mask_c;\
                                  mPORT_DD(gMC1319xGPIO1Port) &= ~gMC1319xGPIO1Mask_c;\
                                  mPORT_DD(gMC1319xGPIO2Port) &= ~gMC1319xGPIO2Mask_c;\
                                  gMC1319xAntSwPort &= ~gMC1319xAntSwMask_c;\
                                  mPORT_DD(gMC1319xAntSwPort)  |= gMC1319xAntSwMask_c;\
                                  gMC1319xSPIPort  |= (gMC1319xSpiMosiMask_c | gMC1319xSpiSsMask_c);\
                                  mPORT_PE(gMC1319xSPIPort) &= (~(gMC1319xSpiMisoMask_c  | gMC1319xSpiRxD1Mask_c));\
                                  mPORT_DD(gMC1319xSPIPort) |= ((gMC1319xSpiTxD1Mask_c  | gMC1319xSpiSsMask_c | gMC1319xSpiMosiMask_c  | gMC1319xSpiSpsckMask_c ))\
                                         & (~(gMC1319xSpiMisoMask_c  | gMC1319xSpiRxD1Mask_c));
 #endif
 

  #define mSETUP_LED_PORT         LED1_PORT |= LED1_PIN;\
                                  LED2_PORT |= LED2_PIN;\
                                  LED3_PORT |= LED3_PIN;\
                                  LED4_PORT |= LED4_PIN;\
                                  mPORT_DD(LED1_PORT)  |= LED1_PIN;\
                                  mPORT_DD(LED2_PORT)  |= LED2_PIN;\
                                  mPORT_DD(LED3_PORT)  |= LED3_PIN;\
                                  mPORT_DD(LED4_PORT)  |= LED4_PIN;\
                                  LED5_PORT |= LED5_MASK;\
                                  mPORT_DD(LED5_PORT) |= LED5_MASK;
 
/************************************************************************************
*************************************************************************************
Uart Port setup is for choosing the flow control pins RTS and CTS. 
The user can choose the port and the pin for RTS or CTS.  
*************************************************************************************
************************************************************************************/  
  #define gSCI1RTSPort_c  PTAD
  #define gSCI1RTSPin_c   0x02
  /* Set the pin direction OUTPUT*/
  #define mSETUP_SCI1RTS_OUTPUT mPORT_DD(gSCI1RTSPort_c) |= gSCI1RTSPin_c;
   
  #define gSCI1CTSPort_c  PTAD
  #define gSCI1CTSPin_c   0x01
  /* Set the pin direction INPUT*/
  #define mSETUP_SCI1CTS_INPUT mPORT_DD(gSCI1CTSPort_c) &= ~gSCI1CTSPin_c;
   
  #define gSCI2RTSPort_c  PTAD
  #define gSCI2RTSPin_c   0x40
  /* Set the pin direction OUTPUT*/
  #define mSETUP_SCI2RTS_OUTPUT mPORT_DD(gSCI2RTSPort_c) |= gSCI2RTSPin_c;

  #define gSCI2CTSPort_c  PTAD
  #define gSCI2CTSPin_c   0x80
  /* Set the pin direction INPUT*/
  #define mSETUP_SCI2CTS_INPUT mPORT_DD(gSCI2CTSPort_c) &= ~gSCI2CTSPin_c;

  #define mSETUP_SCIxPINS mSETUP_SCI1RTS_OUTPUT\
                          mSETUP_SCI1CTS_INPUT\
                          mSETUP_SCI2RTS_OUTPUT\
                          mSETUP_SCI2CTS_INPUT
                        

#endif /* gTarget_UserDefined_c */

#endif /* _PORTCONFIG_H_ */
