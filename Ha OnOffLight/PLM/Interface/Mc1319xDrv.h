/************************************************************************************
* This file contains the driver primitives for SPI and GPIO interfacing from the
* MAC/PHY to the MC1319x. The primitives may be implemented as macros or functions
* depending on the speed of the MCU and effectiveness of the compiler. There is a
* potential trade-off between code space and execution speed through this choice.
*
*
* Copyright (c) 2006, Freescale, Inc.  All rights reserved.
*
*
* No part of this document must be reproduced in any form - including copied,
* transcribed, printed or by any electronic means - without specific written
* permission from Freescale Semiconductor.
*
*****************************************************************************/
#ifndef _MC1319XDRV_H_
#define _MC1319XDRV_H_

#include "EmbeddedTypes.h"
#include "PortConfig.h"

/* Place it in NON_BANKED memory */
#ifdef MEMORY_MODEL_BANKED
#pragma CODE_SEG __NEAR_SEG NON_BANKED
#else
#pragma CODE_SEG DEFAULT
#endif /* MEMORY_MODEL_BANKED */

/************************************************************************************
* Local macros for controlling MC1319x GPIO pins
************************************************************************************/
#define MC1319xDrv_AttEnable()      { gMC1319xAttnPort  |=  gMC1319xAttnMask_c;  }
#define MC1319xDrv_AttDisable()     { gMC1319xAttnPort  &= ~gMC1319xAttnMask_c;  }
#define MC1319xDrv_AssertReset()    { gMC1319xResetPort &= ~gMC1319xResetMask_c; }
#define MC1319xDrv_DeassertReset()  { gMC1319xResetPort |=  gMC1319xResetMask_c; }


/************************************************************************************
* Global Driver variables for getting MC1319x GPIO pin values 
************************************************************************************/
extern uint8_t* pMC1319xDrvPortGpio1; 
extern uint8_t* pMC1319xDrvPortGpio2; 
extern uint8_t  mMC1319xDrvMaskGpio1; 
extern uint8_t  mMC1319xDrvMaskGpio2; 


/************************************************************************************
*************************************************************************************
* Interface functions/macros
*************************************************************************************
************************************************************************************/

/************************************************************************************
* Check if MC1319x Transceiver is active through GPIO1.
*
* Used at the end of packet reception when polling the transceiver state. 
* Read the state of the GPIO1 on the transceiver. 
* GPIO1 high signifies transceiver active; low signifies transceiver inactive.
*   
* Interface assumptions:
*   None
*   
* Return value:
*   None
* 
************************************************************************************/
#define MC1319xDrv_IsActive()               (*pMC1319xDrvPortGpio1 & mMC1319xDrvMaskGpio1)      // Read of MC1319x GPIO1 pin 

/************************************************************************************
* Check if CRC valid or Channel busy on MC1319x Transceiver GPIO2.
* 
* Essentially identical functions. Used at the end of packet reception 
* and end of a Cca, respectively to check the state of GPIO2. 
* Depending on the transceiver state this GPIO indicates either 
* the result of the CRC performed on a received frame or 
* the result of the Cca algorithm. 
* GPIO2 high signifies 'CRC valid' or 'Channel busy'; 
* GPIO2 low signifies 'CRC invalid' or 'Channel clear'
*
* Interface assumptions:
*   None
*   
* Return value:
*   None
* 
************************************************************************************/
#define MC1319xDrv_IsCrcOk()                (*pMC1319xDrvPortGpio2 & mMC1319xDrvMaskGpio2)      // Read of MC1319x GPIO2 pin 
#define MC1319xDrv_IsCcaBusy()              (*pMC1319xDrvPortGpio2 & mMC1319xDrvMaskGpio2)      // Read of MC1319x GPIO2 pin 


/************************************************************************************
* Driver prototype functions for setting MC1319x GPIO pins
************************************************************************************/

/************************************************************************************
* Set MC1319x RxTxEnable pin on MC1319x Transceiver.
*
* Ensures that actions written to the MC1319x command register do not take effect 
* until after the release (clr) of the pin.   
*
* Interface assumptions:
*   None
*   
* Return value:
*   None
* 
************************************************************************************/
void MC1319xDrv_RxTxEnable(void);                         

/************************************************************************************
* Clr MC1319x RxTxEnable pin on MC1319x Transceiver.
*
* Ensures that actions written to the MC1319x command register do not take effect 
* until after the release (clr) of the pin.   
*   
* Interface assumptions:
*   None
*   
* Return value:
*   None
* 
************************************************************************************/
void MC1319xDrv_RxTxDisable(void);                       

/************************************************************************************
* Set MC1319x antenna switch to Rx on MC1319x Transceiver - if switch available.
*
* It is intended that activation of an optional LNA for Rx is performed in connection 
* with these calls. Execution time must however be kept low very low (2-3us max). 
* Leave empty if no support for antenna switch is desired.
*   
* Interface assumptions:
*   None
*   
* Return value:
*   None
* 
************************************************************************************/
void MC1319xDrv_RxAntennaSwitchEnable(void);   

/************************************************************************************
* Set MC1319x antenna switch to Tx on MC1319x Transceiver - if switch available.
*   
* It is intended that activation of an optional PA for Tx is performed in connection 
* with these calls. Execution time must however be kept low very low (2-3us max). 
* Leave empty if no support for antenna switch is desired.
*
 Interface assumptions:
*   None
*   
* Return value:
*   None
* 
************************************************************************************/
void MC1319xDrv_TxAntennaSwitchEnable(void); 

/************************************************************************************
* Turn off LNA if present on MC1319x RF frontend.
*
* This primitive is invoked on receiver shut down. 
* It is assumed that the enabling of this device is performed using the 
* associated Rx Antenna Switch primitive. 
*
*   
* Interface assumptions:
*   None
*   
* Return value:
*   None
* 
************************************************************************************/
void MC1319xDrv_RxLnaDisable(void);

/************************************************************************************
* Turn off PA if present on MC1319x RF frontend.
*   
* This primitive is invoked on transmitter shut down. 
* It is assumed that the enabling of this device is performed using the 
* associated Tx Antenna Switch primitive. 
*
*   None
*   
* Return value:
*   None
* 
************************************************************************************/
void MC1319xDrv_TxPaDisable(void);



/************************************************************************************
* Driver prototype functions for init, reset and wakeup 
************************************************************************************/

/************************************************************************************
* Init MC1319x driver.
*
* During MCU platform initialization at power-up this primitive may be invoked by 
* the platform to initialize the MCU SPI port, which is connected to the MC1319x.
* 
*  
* Interface assumptions:
*   None
*   
* Return value:
*   None
* 
************************************************************************************/
void MC1319xDrv_Init(void);    

/************************************************************************************
* Reset MC1319x Transceiver through reset pin.
*
* During MCU platform initialization at power-up this primitive may be invoked by 
* the platform to perform a hardware reset of the MC1319x transceiver. 
* This must be done by asserting an active low pulse onto the RST pin through an MCU GPIO.    
*   
* Interface assumptions:
*   None
*   
* Return value:
*   None
* 
************************************************************************************/
void MC1319xDrv_Reset(void);  

/************************************************************************************
* Pulsed wakeup of MC1319x Transceiver through Attn pin.
*
* Through the ASP interface a wake up from power save functionality is supported. 
* This functionality requires an ability in the driver to pulse the ATTN signal on 
* the MC1319x transceiver. 
* This wake up primitive must assert an active low pulse onto ATTN with a duration of ~10us.
* When a wake up pulse has been sent on the ATTN pin while the MC1319x was
* in low power mode the MC1319x will wake up and issue either a doze or a hipernate irq,
* which is handled in the MC1319x handler.
*
* Interface assumptions:
*   None
*   
* Return value:
*   None
* 
************************************************************************************/
void MC1319xDrv_WakeUp(void);  

/************************************************************************************
* Writes the clock out control register of the MC1319x Transceiver. The register
* is used for configuring the clock out option of the MC1319x and setting up the
* crystal trim value. 
*
* It is the responsibility of the Platform Software to ensure that clocko is
* properly configured on system initialization.
*   
* Interface assumptions:
*   None
*   
* Return value:
*   None
* 
************************************************************************************/
#define MC1319xDrv_WriteClockoCtrl(content) MC1319xDrv_WriteSpiAsync(ABEL_regA, content);


/**********************************************************************
* Prototypes for below public MC1319xDrv macros                       *
* Tranceiver Access Drivers via SPI                                   *
**********************************************************************/
void MC1319xDrv_ReadSpi(uint8_t addr, uint16_t *pb);
void MC1319xDrv_ReadSpiInt(uint8_t addr, uint16_t *pb);
void MC1319xDrv_ReadSpiIntLE(uint8_t addr, uint16_t *pb);

void MC1319xDrv_WriteSpi(uint8_t addr, uint16_t content);
void MC1319xDrv_WriteSpiInt(uint8_t addr, uint16_t content);
void MC1319xDrv_WriteSpiIntFast(uint8_t addr, uint16_t content);


/**********************************************************************
* Public MC1319xDrv macros                                            *
* Tranceiver Access Drivers via SPI                                   *
**********************************************************************/

/************************************************************************************
* MC1319x SPI Read access macros. 
*
* MC1319xDrv_ReadSpiAsync       - May be called from any context. 
*                                 Protected from mcu/app irq. Includes irq status backup
* MC1319xDrv_ReadSpiSync        - May only be called from MC1319x irq context. 
*                                 Protected from mcu/app irq. 
* MC1319xDrv_ReadSpiSyncLE      - Like MC1319xDrv_ReadSpiSync but with forced Little Endian
*
* MC1319xDrv_ReadSpiAsyncBurst  - Consequtive MC1319xDrv_ReadSpiAsync of 2 registers. 
*                                 Used for 24bit clock reads
*  
* MC1319xDrv_ReadSpiSyncBurst   - Consequtive MC1319xDrv_ReadSpiSync of 2 registers. 
*                                 Used for 24bit clock reads
* Interface assumptions:
*   OR's 0x80 onto register adress byte 'MC1319xReg' to specify SPI read
*   
* Return value:
*   Call by reference. Data returned in 'pRetVal' or 'pSrcMem' 
* 
************************************************************************************/
#define MC1319xDrv_ReadSpiAsync(MC1319xReg, pRetVal)      MC1319xDrv_ReadSpi        ((MC1319xReg | 0x80),(uint16_t *)pRetVal)
#define MC1319xDrv_ReadSpiSync(MC1319xReg, pRetVal)       MC1319xDrv_ReadSpiInt     ((MC1319xReg | 0x80),(uint16_t *)pRetVal)
#define MC1319xDrv_ReadSpiSyncLE(MC1319xReg, pRetVal)     MC1319xDrv_ReadSpiIntLE   ((MC1319xReg | 0x80),(uint16_t *)pRetVal)

// Burst SPI Read
#define MC1319xDrv_ReadSpiAsyncBurst(MC1319xReg, pSrcMem) MC1319xDrv_ReadSpiAsync   (((MC1319xReg+0) | 0x80), &((uint16_t*)pSrcMem)[0]); \
                                                          MC1319xDrv_ReadSpiAsync   (((MC1319xReg+1) | 0x80), &((uint16_t*)pSrcMem)[1])

#define MC1319xDrv_ReadSpiSyncBurst(MC1319xReg, pSrcMem)  MC1319xDrv_ReadSpiSync    (((MC1319xReg+0) | 0x80), &((uint16_t*)pSrcMem)[0]); \
                                                          MC1319xDrv_ReadSpiSync    (((MC1319xReg+1) | 0x80), &((uint16_t*)pSrcMem)[1])


/************************************************************************************
* MC1319x SPI Write access macros. 
*
* MC1319xDrv_WriteSpiAsync      - May be called from any context. 
*                                 Protected from mcu/app irq. Includes irq status backup
* MC1319xDrv_WriteSpiSync       - May only be called from MC1319x irq context. 
*                                 Protected from mcu/app irq. 
* MC1319xDrv_WriteSpiSyncFast   - May only be called from MC1319x irq context. 
*                                 NOT Protected from mcu/app irq. 
*
* MC1319xDrv_WriteSpiAsyncBurst - Consequtive MC1319xDrv_WriteSpiAsync of 2 registers. 
*                                 Used for 24bit clock writes. 
*                                 2x async calls ensure correct context and no blocking 
*                                 of sync calls for more than 1 word access duration.  
*  
* MC1319xDrv_WriteSpiSyncBurst  - Consequtive MC1319xDrv_WriteSpiSync of 2 registers. 
*                                 Used for 24bit clock writes
*
* Interface assumptions:
*   Adress byte 'MC1319xReg' has bit 7 == 0 to specify SPI write
*   Data word in 'regVal' or in '*pSrcMem'
*   
* Return value:
*   Call by reference. Data returned in 'content'  
* 
************************************************************************************/
#define MC1319xDrv_WriteSpiAsync(MC1319xReg, regVal)       MC1319xDrv_WriteSpi        (MC1319xReg, regVal)
#define MC1319xDrv_WriteSpiSync(MC1319xReg, regVal)        MC1319xDrv_WriteSpiInt     (MC1319xReg, regVal)
#define MC1319xDrv_WriteSpiSyncFast(MC1319xReg, regVal)    MC1319xDrv_WriteSpiIntFast (MC1319xReg, regVal)

// Burst SPI Write
#define MC1319xDrv_WriteSpiAsyncBurst(MC1319xReg, pSrcMem)  MC1319xDrv_WriteSpiAsync   ((MC1319xReg+0), ((uint16_t*)pSrcMem)[0]); \
                                                            MC1319xDrv_WriteSpiAsync   ((MC1319xReg+1), ((uint16_t*)pSrcMem)[1])

#define MC1319xDrv_WriteSpiSyncBurst(MC1319xReg, pSrcMem)   MC1319xDrv_WriteSpiSync    ((MC1319xReg+0), ((uint16_t*)pSrcMem)[0]); \
                                                            MC1319xDrv_WriteSpiSync    ((MC1319xReg+1), ((uint16_t*)pSrcMem)[1])
#pragma CODE_SEG DEFAULT

#endif /* _MC1319XDRV_H_ */
