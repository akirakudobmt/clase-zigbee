/******************************************************************************
* ZclSecAndSafe.c
*
* This module contains code that handles ZigBee Cluster Library commands and
* clusters for the Security and safety clusters.
*
* Copyright (c) 2009, Freescale, Inc.  All rights reserved.
*
*
* No part of this document must be reproduced in any form - including copied,
* transcribed, printed or by any electronic means - without specific written
* permission from Freescale Semiconductor.
*
* Documents used in this specification:
* [R1] - 053520r16ZB_HA_PTG-Home-Automation-Profile.pdf
* [R2] - 075123r00ZB_AFG-ZigBee_Cluster_Library_Specification.pdf
*******************************************************************************/
#include "EmbeddedTypes.h"
#include "zigbee.h"
#include "FunctionLib.h"
#include "BeeStackConfiguration.h"
#include "BeeStack_Globals.h"
#include "AppAfInterface.h"
#include "AfApsInterface.h"
#include "BeeStackInterface.h"

#include "zcl.h"
#include "ZclSecAndSafe.h"
#include "HcProfile.h"


/******************************************************************************
*******************************************************************************
* Private macros
*******************************************************************************
******************************************************************************/

/******************************************************************************
*******************************************************************************
* Private prototypes
*******************************************************************************
******************************************************************************/

/******************************************************************************
*******************************************************************************
* Private type definitions
*******************************************************************************
******************************************************************************/

/******************************************************************************
*******************************************************************************
* Public memory definitions
*******************************************************************************
******************************************************************************/

/******************************************************************************
*******************************************************************************
* Private memory declarations
*******************************************************************************
******************************************************************************/


/*****************************************************************************/
zbStatus_t ZCL_IASZoneClusterClient
(
    zbApsdeDataIndication_t *pIndication, 
    afDeviceDef_t *pDev
)
{
    zclFrame_t *pFrame;
    zclCmd_t command;

    /* prevent compiler warning */
    (void)pDev;
    pFrame = (void *)pIndication->pAsdu;
    /* handle the command */
    command = pFrame->command;  
    switch(command)
    {
        //case gZclCmdTxIASZone_ZoneStatusChange_c:
        case gZclCmdTxIASZone_ZoneEnrollRequest_c:

          /* These should be passed up to a host*/
          return gZbSuccess_c;
        default:
          return gZclUnsupportedClusterCommand_c;
    }
}


/*****************************************************************************/
zbStatus_t ZCL_IASZoneClusterServer
(
    zbApsdeDataIndication_t *pIndication, 
    afDeviceDef_t *pDev
)
{
    zclFrame_t *pFrame;
    zclCmd_t command;

    /* prevent compiler warning */
    (void)pDev;
    pFrame = (void *)pIndication->pAsdu;
    /* handle the command */
    command = pFrame->command;  
    switch(command)
    {
        case gZclCmdRxIASZone_ZoneEnrollResponse_c:

          /* These should be passed up to a host*/
          return gZbSuccess_c;
        default:
          return gZclUnsupportedClusterCommand_c;
    }

}


/*****************************************************************************/
zbStatus_t ZCL_IASWDClusterClient
(
    zbApsdeDataIndication_t *pIndication, 
    afDeviceDef_t *pDev
)
{
    zclFrame_t *pFrame;
    zclCmd_t command;

    /* prevent compiler warning */
    (void)pDev;
    pFrame = (void *)pIndication->pAsdu;
    /* handle the command */
    command = pFrame->command;  
    switch(command)
    {
       default:
          return gZclUnsupportedClusterCommand_c;
    }

}


/*****************************************************************************/
zbStatus_t ZCL_IASWDServer
(
    zbApsdeDataIndication_t *pIndication, 
    afDeviceDef_t *pDev
)
{
    zclFrame_t *pFrame;
    zclCmd_t command;

    /* prevent compiler warning */
    (void)pDev;
    pFrame = (void *)pIndication->pAsdu;
    /* handle the command */
    command = pFrame->command;  
    switch(command)
    {
        case gZclCmdRxIASWD_StartWarning_c:
        case gZclCmdRxIASWD_Squawk_c:

          /* These should be passed up to a host*/
          return gZbSuccess_c;
        default:
          return gZclUnsupportedClusterCommand_c;
    }

}


/*****************************************************************************/
zbStatus_t IASZone_ZoneStatusChange
(
    zclCmdIASZone_ZoneStatusChange_t *pReq
)
{
   // pReq->zclTransactionId = gZclTransactionId++;
//    Set2Bytes(pReq->addrInfo.aClusterId, gZclClusterIASZone_c);	
    return ZCL_SendClientReqSeqPassed(gZclCmdTxIASZone_ZoneStatusChange_c,0,(zclGenericReq_t *)pReq);
}



/*****************************************************************************/
zbStatus_t IASZone_ZoneEnrollRequest
(
    zclCmdIASZone_ZoneEnrollRequest_t *pReq
)
{
//    pReq->zclTransactionId = gZclTransactionId++;
//    Set2Bytes(pReq->addrInfo.aClusterId, gZclClusterIASZone_c);	
    return ZCL_SendClientReqSeqPassed(gZclCmdTxIASZone_ZoneEnrollRequest_c,0,(zclGenericReq_t *)pReq);
}

/*****************************************************************************/
zbStatus_t IASZone_ZoneEnrollResponse
(
    zclCmdIASZone_ZoneEnrollResponse_t *pReq
)  
{
 //   pReq->zclTransactionId = gZclTransactionId++;
 //   Set2Bytes(pReq->addrInfo.aClusterId, gZclClusterIASZone_c);	
    return ZCL_SendClientReqSeqPassed(gZclCmdRxIASZone_ZoneEnrollResponse_c,0,(zclGenericReq_t *)pReq);
}

/*****************************************************************************/

                          
zbStatus_t IASWD_StartWarning
(
    zclCmdIASWD_StartWarning_t *pReq
)
{
 //   pReq->zclTransactionId = gZclTransactionId++;
 //   Set2Bytes(pReq->addrInfo.aClusterId, gZclClusterIASWD_c);	
    return ZCL_SendClientReqSeqPassed(gZclCmdRxIASWD_StartWarning_c,0,(zclGenericReq_t *)pReq);
}

/*****************************************************************************/
zbStatus_t IASWD_Squawk
(
    zclCmdIASWD_Squawk_t *pReq
)     
{
 //   pReq->zclTransactionId = gZclTransactionId++;
 //   Set2Bytes(pReq->addrInfo.aClusterId, gZclClusterIASWD_c);	
    return ZCL_SendClientReqSeqPassed(gZclCmdRxIASWD_Squawk_c,0,(zclGenericReq_t *)pReq);
}

/*****************************************************************************/


