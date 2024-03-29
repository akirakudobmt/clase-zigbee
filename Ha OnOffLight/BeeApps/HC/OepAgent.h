/************************************************************************************
* OepAgent.h
* IEEE 11073 Optimized Exchange Protocol Agent header file
*
* (c) Copyright 2010, Freescale, Inc.  All rights reserved.
*
*
* No part of this document must be reproduced in any form - including copied,
* transcribed, printed or by any electronic means - without specific written
* permission from Freescale Semiconductor.
*
************************************************************************************/
#ifndef _OEPAGENT_H_
#define _OEPAGENT_H_

#include "BeeStack_Globals.h"
#include "EmbeddedTypes.h"
#include "zigbee.h"
#include "zcl.h"
#include "HcProfile.h"
#include "Oep11073.h"

/******************************************************************************
*******************************************************************************
* Public Macros
*******************************************************************************
******************************************************************************/



/******************************************************************************
*******************************************************************************
* Public type definitions
*******************************************************************************
******************************************************************************/


/******************************************************************************
*******************************************************************************
* Public Memory Declarations
*******************************************************************************
******************************************************************************/


/******************************************************************************
*******************************************************************************
* Public Prototypes
*******************************************************************************
******************************************************************************/
void OepAgent_HandleApdu(OepFragmentedApdu_t *pApdu);
void OepAgentInit(zbEndPoint_t ep);
void OepAgentSetAddressInfo(Oep11073AddressInfo_t *addrInfo);


/**********************************************************************************/
#endif /* _OEPAGENT_H_ */
