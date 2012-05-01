/************************************************************************************
* Japan TELEC header file.
*
* (c) Copyright 2007, Freescale, Inc.  All rights reserved.
*
* No part of this document must be reproduced in any form - including copied,
* transcribed, printed or by any electronic means - without specific written
* permission from Freescale Semiconductor.
*
************************************************************************************/

#ifndef _TELEC_H_
#define _TELEC_H_

#define BER_EN (1<<15)    /* Abel_reg30 */
#define PAEN_MASK (1<<15) /* Abel_reg8 */
#define PSM_MASK (1<<3) /* Abel_reg31*/

/************************************************************************************
* Mainloop called from outer main loop.
*   
* Interface assumptions:
*   None
*   
* Return value:
*   None
* 
************************************************************************************/
void ASP_TestMain(void);

/************************************************************************************
* Start tests by calling this function
*   
* Interface assumptions:
*   None
*   
* Return value:
*   None
* 
************************************************************************************/
void ASP_TelecTest(uint8_t mode);

/************************************************************************************
* Transmit raw data on the selected channel. The function will return when
* the data transmission has finished.
*   
* Interface assumptions:
*   length must be smaller than 125. If it is bigger, the function will return
*   without transmitting any data.
*   
* Return value:
*   None
* 
************************************************************************************/
#ifdef PROCESSOR_MC1323X
void ASP_TelecSendRawData(uint8_t* dataPtr);
#else
void ASP_TelecSendRawData(uint8_t length, uint8_t* dataPtr);
#endif //PROCESSOR_MC1323X

/************************************************************************************
* Set new frequency channel
*   
* Interface assumptions:
*   None
*   
* Return value:
*   None
* 
************************************************************************************/
void ASP_TelecSetFreq(uint8_t channel);


#endif //_TELEC_H_