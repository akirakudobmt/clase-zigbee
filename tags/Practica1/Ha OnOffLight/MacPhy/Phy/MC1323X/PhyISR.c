/************************************************************************************
* MC1323X: This module is the PHY layer MC1323X (interrupt) handlers
*
* (c) Copyright 2009, Freescale, Inc.  All rights reserved.
*
* No part of this document must be reproduced in any form - including copied,
* transcribed, printed or by any electronic means - without specific written
* permission from Freescale Semiconductor.
*
************************************************************************************/


/************************************************************************************
*************************************************************************************
* Includes
*************************************************************************************
************************************************************************************/
#include "EmbeddedTypes.h"
#include "IrqControlLib.h"
#include "IoConfig.h"
#include "Phy.h"
#include "PhyDebugMC1323X.h"
#include "PlatformInit.h"
#include "UART_Interface.h"


/************************************************************************************
*************************************************************************************
* Public memory declarations
*************************************************************************************
************************************************************************************/
uint8_t mPhyLastRxLQI;
void (*pfPhyRxTxEndCallback)(void) = NULL;
extern void (*pfPhyRxTxStartCallback)(void);

/************************************************************************************
*************************************************************************************
* Private constants
*************************************************************************************
************************************************************************************/


/************************************************************************************
*************************************************************************************
* Private memory declarations
*************************************************************************************
************************************************************************************/

static phyRxParams_t * mpRxParams = NULL;


/************************************************************************************
*************************************************************************************
* Private prototypes
*************************************************************************************
************************************************************************************/

/* Place it in NON_BANKED memory */
#ifdef MEMORY_MODEL_BANKED
#pragma CODE_SEG __NEAR_SEG NON_BANKED
#else
#pragma CODE_SEG DEFAULT
#endif /* MEMORY_MODEL_BANKED */

static void    PhyISR_SeqCleanup(void);
static void    PhyISR_TimeoutCleanup(void);
static uint8_t Phy_LqiConvert(void);
static uint8_t Phy_GetEnergyLevel(void);


/************************************************************************************
*************************************************************************************
* Public functions
*************************************************************************************
************************************************************************************/

uint8_t PhyGetLastRxLqiValue(void)
{
  return mPhyLastRxLQI;
}

/************************************************************************************
*************************************************************************************
* Private functions
*************************************************************************************
************************************************************************************/
// Needed to initialize Timer1 vector
void DummyIsr(void)
{
  ;
}

void (*pTimer1Isr)(void) = DummyIsr; // Makes it possible to dynamically change interrupt vector for testing!

/***********************************************************************************/
void PhyPassRxParams(phyRxParams_t * pRxParam)
{
  mpRxParams = pRxParam;
}

/***********************************************************************************/

static void PhyISR_SeqCleanup(void)
{
  PP_PHY_STATUS1 =  PP_PHY_STATUS1_TMR3_IRQ | PP_PHY_STATUS1_TMR2_IRQ;
  PP_PHY_STATUS2 =  PP_PHY_STATUS2_CCA_IRQ  | PP_PHY_STATUS2_RX_IRQ | PP_PHY_STATUS2_TX_IRQ | PP_PHY_STATUS2_SEQ_IRQ | PP_PHY_STATUS2_LO1UNLOCK_IRQ | PP_PHY_STATUS2_FLTR_FAIL_IRQ | PP_PHY_STATUS2_RX_WMRK_IRQ;
  PP_PHY_CTL4   &= ~PP_PHY_CTL4_TMR3MSK_BIT; // unmask TMR3 interrupt
  PP_PHY_CTL3   |=  PP_PHY_CTL3_SEQ_MSK_BIT | PP_PHY_CTL3_TX_MSK_BIT | PP_PHY_CTL3_RX_MSK_BIT | PP_PHY_CTL3_CCA_MSK_BIT; // mask SEQ, RX, TX and CCA interrupts
}

/***********************************************************************************/

static void PhyISR_TimeoutCleanup(void)
{
  PP_PHY_STATUS1 =  PP_PHY_STATUS1_TMR3_IRQ | PP_PHY_STATUS1_TMR2_IRQ;
  PP_PHY_STATUS2 =  PP_PHY_STATUS2_CCA_IRQ  | PP_PHY_STATUS2_RX_IRQ | PP_PHY_STATUS2_TX_IRQ | PP_PHY_STATUS2_SEQ_IRQ | PP_PHY_STATUS2_LO1UNLOCK_IRQ | PP_PHY_STATUS2_FLTR_FAIL_IRQ | PP_PHY_STATUS2_RX_WMRK_IRQ;
  PP_PHY_CTL3   |=  PP_PHY_CTL3_SEQ_MSK_BIT | PP_PHY_CTL3_TX_MSK_BIT | PP_PHY_CTL3_RX_MSK_BIT | PP_PHY_CTL3_CCA_MSK_BIT; // mask SEQ, RX, TX and CCA interrupts
  PP_PHY_CTL2   &= ~PP_PHY_CTL2_TC3TMOUT_BIT;
  PP_PHY_CTL4   &= ~PP_PHY_CTL4_TMR3CMP_EN_BIT;
  PP_PHY_CTL4   |=  PP_PHY_CTL4_TMR3MSK_BIT; // mask TMR3 interrupt
}

/***********************************************************************************/

static uint8_t Phy_LqiConvert(void)
{
  uint8_t     linkQuality = PP_PHY_CCAFNL;
  
#ifndef gLQIMappingForRF4CE_c
  // Recalculate the link quality to conform with other link quality measures
  // Make dynamics of the energy level vary from 0x00-0xff
  if (linkQuality > 190) {
    linkQuality = 190; //190 = -95dBm as floor (translates to 0x00)
  }
  if (linkQuality < 33) {
    linkQuality = 33; //33 = -16.5 dBm as top (saturation)
  }
  linkQuality = 190 - linkQuality;
  linkQuality = linkQuality + (linkQuality >> 1) + (linkQuality >> 3); // equivalent with multiply by 1.625
#else // gLQIMappingForRF4CE_c
  linkQuality = 255 - linkQuality;
#endif // gLQIMappingForRF4CE_c
  
  /* Save LQI value for ZTC */
  mPhyLastRxLQI = linkQuality;
  
  return linkQuality;
}

/************************************************************************************
* Get and convert energyLevel
*
* ED returns values between -75 dBm to about -11.5 dBm which are represented by decimal
* values 150 and 23 respectively.
*
************************************************************************************/

static uint8_t Phy_GetEnergyLevel(void)
{
  uint8_t   energyLevel = PP_PHY_CCAFNL;

  // Make dynamics of the energy level vary from 0x00-0xff
  if (energyLevel > 150)
  {
    energyLevel = 150; //150 = -75dBm as floor (translates to 0x00)
  }
  if (energyLevel < 23)
  {
    energyLevel = 23; //23 = -11.5 dBm as top (saturation)
  }
  energyLevel = 150 - energyLevel;  // 150 = -75dBm as floor (translates to 0xFF)
  
  // Now double the value to "stretch" the dynamics up to 255
  energyLevel = energyLevel << 1;

  return energyLevel;
}

/***********************************************************************************/

INTERRUPT_KEYWORD void PhyHandlerTx_ISR(void)
{
  uint8_t xcvseqCopy, status2Copy;
#if gUart1_EnableHWFlowControl_d
  uint8_t flowCtrState = (PTDD & 0x04);
#endif
  
#if (gPpdMode_d != gPpdModeOFF_d) 
  volatile uint8_t temp;
#endif //(gPpdMode_d != gPpdModeOFF_d  
  PHY_DEBUG_LOG(PDBG_TX_VECT_INTERRUPT);

  status2Copy     =  PP_PHY_STATUS2 & (PP_PHY_STATUS2_SEQ_IRQ | PP_PHY_STATUS2_LO1UNLOCK_IRQ);
  xcvseqCopy      =  PhyGetSeqState();        // read XCVSEQ
  
#if gUart1_EnableHWFlowControl_d  
  //disable uart flow control
  PTDD |= 0x04;
#endif

  IrqControlLib_DisableMC1323xIrq();
  IrqControlLib_EnableAllIrqs();

  if(status2Copy & PP_PHY_STATUS2_LO1UNLOCK_IRQ)
  {
    #ifdef PHY_ISR_HOOKS
    PhyPlmeUnlockTx();
    #endif // PHY_ISR_HOOKS
    PhyISR_SeqCleanup();
#if (gPpdMode_d != gPpdModeOFF_d)    
    //PPD Mode is enabled
    IoIndirectWrite(0x63, 10);        //forces cg_en low
    temp = IoIndirectRead(0x63);      //this is a delay step
    temp = IoIndirectRead(0x63);      //this is a delay step        
    IoIndirectWrite(0x63, 0x00);      //releases cg_en
#endif //(gPpdMode_d != gPpdModeOFF_d)        
    PHY_DEBUG_LOG(PDBG_SYNC_LOSS_INDICATION);
    
    PP_PHY_CTL1    &= ~PP_PHY_CTL1_XCVSEQ_MASK; // set the PHY sequencer back to IDLE
    
    PhyPlmeSyncLossIndication();
    
    IrqControlLib_DisableAllIrqs();
    IrqControlLib_EnableMC1323xIrq();
    
#if gUart1_EnableHWFlowControl_d    
    //restore uart flow control state
    if(flowCtrState)
    {
      PTDD |= 0x04;
    }
    else
    {
      PTDD &= ~0x04;
    }
#endif
    
    return;
  }
  
  if(PP_PHY_STATUS1 &  PP_PHY_STATUS1_TMR3_IRQ)  
  {
   PhyISR_TimeoutCleanup();
   PHY_DEBUG_LOG(PDBG_TIMEOUT_INDICATION);
   //PPD Mode is enabled
#if (gPpdMode_d != gPpdModeOFF_d)      
    // set the HW sequencer to idle state
    temp = IoIndirectRead(0x17);
    IoIndirectWrite(0x17, 0x00);   //disable the PPD
#endif //(gPpdMode_d == gPpdModeOFF_d)            
    PP_PHY_CTL1    &= ~PP_PHY_CTL1_XCVSEQ_MASK; // set the PHY sequencer back to IDLE    
#if (gPpdMode_d != gPpdModeOFF_d)        
    IoIndirectWrite(0x17, temp); //restore the PPD mode
#endif //(gPpdMode_d != gPpdModeOFF_d) 

   PhyTimeRxTimeoutIndication();
   
   IrqControlLib_DisableAllIrqs();
   IrqControlLib_EnableMC1323xIrq();

#if gUart1_EnableHWFlowControl_d   
   //restore uart flow control state
   if(flowCtrState)
   {
     PTDD |= 0x04;
   }
   else
   {
     PTDD &= ~0x04;
   }
#endif

   return;
  }   

  //PPD Mode Abort
   //PPD Mode is enabled
#if (gPpdMode_d != gPpdModeOFF_d)      
    // set the HW sequencer to idle state
    temp = IoIndirectRead(0x17);
    IoIndirectWrite(0x17, 0x00);   //disable the PPD
#endif //(gPpdMode_d != gPpdModeOFF_d)            
    PP_PHY_CTL1    &= ~PP_PHY_CTL1_XCVSEQ_MASK; // set the PHY sequencer back to IDLE    
#if (gPpdMode_d != gPpdModeOFF_d)        
    IoIndirectWrite(0x17, temp); //restore the PPD mode
#endif //(gPpdMode_d != gPpdModeOFF_d) 
  
  PHY_DEBUG_LOG1(PDBG_SEQ_IRQ, xcvseqCopy);

  switch(xcvseqCopy)
  {
    case gTX_c:   
                  PhyISR_SeqCleanup();
                    
                  if(( PP_PHY_STATUS1 & PP_PHY_STATUS1_CCA_BIT ) && (PP_PHY_CTL1 & PP_PHY_CTL1_CCABFRTX_BIT))
                  {
                    PHY_DEBUG_LOG(PDBG_CCA_CONFIRM_BUSY);
                    PhyPlmeCcaConfirm(gChannelBusy_c);
                  }
                  else
                  {
                    PHY_DEBUG_LOG(PDBG_DATA_CONFIRM);
                    PhyPdDataConfirm();
                  }
                  
                  if (pfPhyRxTxEndCallback)
                    pfPhyRxTxEndCallback();
                  
                  break;
    
    case gTR_c:   
                  PhyISR_SeqCleanup();   
                  
                  if(( PP_PHY_STATUS1 & PP_PHY_STATUS1_CCA_BIT ) && (PP_PHY_CTL1 & PP_PHY_CTL1_CCABFRTX_BIT))
                  {
                      PHY_DEBUG_LOG(PDBG_CCA_CONFIRM_BUSY);
                      PhyPlmeCcaConfirm(gChannelBusy_c);
                  }
                  else
                  {
                      if(NULL != mpRxParams)
                      {
                         *((uint8_t*)&(mpRxParams->timeStamp)+3) = PP_PHY_TIMESTMP0;
                         *((uint8_t*)&(mpRxParams->timeStamp)+2) = PP_PHY_TIMESTMP8;
                         *((uint8_t*)&(mpRxParams->timeStamp)+1) = PP_PHY_TIMESTMP16;
                         *((uint8_t*)&(mpRxParams->timeStamp)+0) = 0;
                         mpRxParams->linkQuality = Phy_LqiConvert();
                      }
                    
                      PHY_DEBUG_LOG(PDBG_DATA_CONFIRM);
                      PhyPdDataConfirm();
                  }                    

                  if (pfPhyRxTxEndCallback)
                    pfPhyRxTxEndCallback();
                  
                  break;
                  
    default:      PhyISR_SeqCleanup();
                  PHY_DEBUG_LOG(PDBG_SYNC_LOSS_INDICATION);
                  PhyPlmeSyncLossIndication();
                  break;
  }
  
  IrqControlLib_DisableAllIrqs();
  IrqControlLib_EnableMC1323xIrq();
  
#if gUart1_EnableHWFlowControl_d  
  //restore uart flow control state
  if(flowCtrState)
  {
    PTDD |= 0x04;
  }
  else
  {
    PTDD &= ~0x04;
  }
#endif

}

/***********************************************************************************/

INTERRUPT_KEYWORD void PhyHandlerRx_ISR(void)
{
  uint8_t xcvseqCopy, status2Copy;
  
#if gUart1_EnableHWFlowControl_d
  uint8_t flowCtrState = (PTDD & 0x04);
#endif

#if (gPpdMode_d != gPpdModeOFF_d)  
  volatile uint8_t temp;
#endif //(gPpdMode_d != gPpdModeOFF_d)
  PHY_DEBUG_LOG(PDBG_RX_VECT_INTERRUPT);

  status2Copy     =  PP_PHY_STATUS2 & (PP_PHY_STATUS2_SEQ_IRQ | PP_PHY_STATUS2_LO1UNLOCK_IRQ);
  xcvseqCopy      =  PhyGetSeqState();        // read XCVSEQ
  
#if gUart1_EnableHWFlowControl_d  
  //disable uart flow control
  PTDD |= 0x04;
#endif
  
  IrqControlLib_DisableMC1323xIrq();
  IrqControlLib_EnableAllIrqs();
  
  if(status2Copy & PP_PHY_STATUS2_LO1UNLOCK_IRQ)
  {
    #ifdef PHY_ISR_HOOKS
    PhyPlmeUnlockRx();
    #endif // PHY_ISR_HOOKS
    PhyISR_SeqCleanup();
    //PPD Mode is enabled
#if (gPpdMode_d != gPpdModeOFF_d)        
    IoIndirectWrite(0x63, 10);        //forces cg_en low
    temp = IoIndirectRead(0x63); //this is a delay step
    temp = IoIndirectRead(0x63); //this is a delay step        
    IoIndirectWrite(0x63, 0x00);      //releases cg_en
#endif //(gPpdMode_d != gPpdModeOFF_d)    
    
    PHY_DEBUG_LOG(PDBG_SYNC_LOSS_INDICATION);
    PP_PHY_CTL1    &= ~PP_PHY_CTL1_XCVSEQ_MASK; // set the PHY sequencer back to IDLE        
    PhyPlmeSyncLossIndication();
    
    IrqControlLib_DisableAllIrqs();
    IrqControlLib_EnableMC1323xIrq();

#if gUart1_EnableHWFlowControl_d   
    //restore uart flow control state
    if(flowCtrState)
    {
      PTDD |= 0x04;
    }
    else
    {
      PTDD &= ~0x04;
    }
#endif

    return;
  }
  
  if(PP_PHY_STATUS1 &  PP_PHY_STATUS1_TMR3_IRQ)
  {
   PhyISR_TimeoutCleanup();  
   PHY_DEBUG_LOG(PDBG_TIMEOUT_INDICATION);
   //PPD Mode is enabled
#if (gPpdMode_d != gPpdModeOFF_d)      
    // set the HW sequencer to idle state
    temp = IoIndirectRead(0x17);
    IoIndirectWrite(0x17, 0x00);   //disable the PPD
#endif //(gPpdMode_d != gPpdModeOFF_d)            
    PP_PHY_CTL1    &= ~PP_PHY_CTL1_XCVSEQ_MASK; // set the PHY sequencer back to IDLE    
#if (gPpdMode_d != gPpdModeOFF_d)        
    IoIndirectWrite(0x17, temp); //restore the PPD mode
#endif //(gPpdMode_d != gPpdModeOFF_d) 
     
   
   PhyTimeRxTimeoutIndication();

   IrqControlLib_DisableAllIrqs();
   IrqControlLib_EnableMC1323xIrq();

#if gUart1_EnableHWFlowControl_d   
   //restore uart flow control state
   if(flowCtrState)
   {
     PTDD |= 0x04;
   }
   else
   {
     PTDD &= ~0x04;
   }
#endif

   return;
  }  
  
   //PPD Mode is enabled
#if (gPpdMode_d != gPpdModeOFF_d)      
    // set the HW sequencer to idle state
    temp = IoIndirectRead(0x17);
    IoIndirectWrite(0x17, 0x00);   //disable the PPD
#endif //(gPpdMode_d != gPpdModeOFF_d)            
    PP_PHY_CTL1    &= ~PP_PHY_CTL1_XCVSEQ_MASK; // set the PHY sequencer back to IDLE    
#if (gPpdMode_d != gPpdModeOFF_d)        
    IoIndirectWrite(0x17, temp); //restore the PPD mode
#endif //(gPpdMode_d != gPpdModeOFF_d) 
  

  PHY_DEBUG_LOG1(PDBG_SEQ_IRQ, xcvseqCopy);

  switch(xcvseqCopy)
  {
    case gCCA_c:  // verify the presence of irqs: CCA - yes, SEQ - yes, UNLOCK - no
                  PhyISR_SeqCleanup();
                  if( (PP_PHY_CTL2 & PP_PHY_CTL2_CCA_TYPE_MASK) == (gCcaED_c << PP_PHY_CTL2_CCA_TYPE_POS) )
                  {
                   // Ed
                   PhyPlmeEdConfirm(Phy_GetEnergyLevel());
                  } 
                  else
                  {
                   // CCA
                   if( PP_PHY_STATUS1 & PP_PHY_STATUS1_CCA_BIT )
                   {                      
                       PHY_DEBUG_LOG(PDBG_CCA_CONFIRM_BUSY);
                       PhyPlmeCcaConfirm(gChannelBusy_c);
                   }
                   else
                   {
                       PHY_DEBUG_LOG(PDBG_CCA_CONFIRM_IDLE);
                       PhyPlmeCcaConfirm(gChannelIdle_c);
                   }
                  }                    
                  break;
    case gCCCA_c: 
                  PhyISR_SeqCleanup();
                  PHY_DEBUG_LOG(PDBG_CCA_CONFIRM_IDLE);
                  PhyPlmeCcaConfirm(gChannelIdle_c);
                  
                  break;
    case gRX_c:   
                  PhyISR_SeqCleanup();

                  if(NULL != mpRxParams)
                  {
                        *((uint8_t*)&(mpRxParams->timeStamp)+3) = PP_PHY_TIMESTMP0;
                        *((uint8_t*)&(mpRxParams->timeStamp)+2) = PP_PHY_TIMESTMP8;
                        *((uint8_t*)&(mpRxParams->timeStamp)+1) = PP_PHY_TIMESTMP16;
                        *((uint8_t*)&(mpRxParams->timeStamp)+0) = 0;
                        mpRxParams->linkQuality = Phy_LqiConvert();
                  }
                  
                  PHY_DEBUG_LOG(PDBG_DATA_INDICATION);
                  PhyPdDataIndication();
                  
                  if (pfPhyRxTxEndCallback)
                    pfPhyRxTxEndCallback();
                  
                  break;
                  
    default:      PhyISR_SeqCleanup();
                  PHY_DEBUG_LOG(PDBG_SYNC_LOSS_INDICATION);
                  PhyPlmeSyncLossIndication();
                  break;
  }

#if gUart1_EnableHWFlowControl_d  
  //restore uart flow control state
  if(flowCtrState)
  {
    PTDD |= 0x04;
  }
  else
  {
    PTDD &= ~0x04;
  }
#endif  
    
  IrqControlLib_DisableAllIrqs();
  IrqControlLib_EnableMC1323xIrq();
}

/***********************************************************************************/

INTERRUPT_KEYWORD void PhyHandlerTmr_ISR(void)
{
#if gUart1_EnableHWFlowControl_d 
  uint8_t flowCtrState = (PTDD & 0x04);
#endif
#if (gPpdMode_d != gPpdModeOFF_d)    
  volatile uint8_t temp;
#endif //(gPpdMode_d != gPpdModeOFF_d)    

#if gUart1_EnableHWFlowControl_d
    //disable uart flow control
  PTDD |= 0x04;
#endif
  
  IrqControlLib_DisableMC1323xIrq();
  IrqControlLib_EnableAllIrqs();
  
 
  PHY_DEBUG_LOG(PDBG_TIMERS_VECT_INTERRUPT);
  
  // TMR2 IRQ servicing - start event
  if(PP_PHY_STATUS1 & PP_PHY_STATUS1_TMR2_IRQ)
  {
    PP_PHY_CTL1    &= ~PP_PHY_CTL1_TMRTRIGEN_BIT;
    PP_PHY_CTL4    &= ~PP_PHY_CTL4_TMR2CMP_EN_BIT;
    PP_PHY_CTL4    |=  PP_PHY_CTL4_TMR2MSK_BIT; // mask TMR2 interrupt
    PP_PHY_STATUS1  =  PP_PHY_STATUS1_TMR2_IRQ;
    PHY_DEBUG_LOG(PDBG_START_INDICATION);
    PhyTimeStartEventIndication();
  }

  // TMR3 IRQ servicing - timeout event
  if(PP_PHY_STATUS1 & PP_PHY_STATUS1_TMR3_IRQ)
  {
    PP_PHY_CTL2    &= ~PP_PHY_CTL2_TC3TMOUT_BIT;
    PP_PHY_CTL4    &= ~PP_PHY_CTL4_TMR3CMP_EN_BIT;
    PP_PHY_CTL4    |=  PP_PHY_CTL4_TMR3MSK_BIT; // mask TMR3 interrupt
    PP_PHY_STATUS1  =  PP_PHY_STATUS1_TMR3_IRQ;
    PHY_DEBUG_LOG(PDBG_TIMEOUT_INDICATION);

    //PPD Mode is enabled
#if (gPpdMode_d != gPpdModeOFF_d)        
    IoIndirectWrite(0x63, 10);        //forces cg_en low
    temp = IoIndirectRead(0x63); //this is a delay step
    temp = IoIndirectRead(0x63); //this is a delay step        
    IoIndirectWrite(0x63, 0x00);      //releases cg_en
#endif //(gPpdMode_d != gPpdModeOFF_d)    

    /* Ensure that we're not issuing TimeoutIndication while the Automated sequence is still in progress */
    /* TMR3 can expire during R-T turnaround for example, case in which the sequence is not interrupted */
    if(gIdle_c == PhyGetSeqState())
    {      
     PhyTimeRxTimeoutIndication();
    }
  }
  
    // TMR1 IRQ servicing
  if(PP_PHY_STATUS1 & PP_PHY_STATUS1_TMR1_IRQ)
  {
    PP_PHY_CTL4    &= ~PP_PHY_CTL4_TMR1CMP_EN_BIT; 
    PP_PHY_CTL4    |=  PP_PHY_CTL4_TMR1MSK_BIT; // mask TMR1 interrupt
    PP_PHY_STATUS1 = PP_PHY_STATUS1_TMR1_IRQ;
     
    pTimer1Isr();
  }
  
  // TMR4 IRQ servicing
  if(PP_PHY_STATUS1 & PP_PHY_STATUS1_TMR4_IRQ)
  {
     PP_PHY_STATUS1 = PP_PHY_STATUS1_TMR4_IRQ;
  }
  IrqControlLib_DisableAllIrqs();
  IrqControlLib_EnableMC1323xIrq();

#if gUart1_EnableHWFlowControl_d  
  //restore uart flow control state
  if(flowCtrState)
  {
    PTDD |= 0x04;
  }
  else
  {
    PTDD &= ~0x04;
  }
#endif

}

/***********************************************************************************/

INTERRUPT_KEYWORD void PhyHandlerRxWtrmrk_ISR(void)
{
  PHY_DEBUG_LOG(PDBG_WTRMRK_VECT_INTERRUPT);
  if(PP_PHY_STATUS2 & PP_PHY_STATUS2_RX_WMRK_IRQ)
  {
    PP_PHY_STATUS2  =  PP_PHY_STATUS2_RX_WMRK_IRQ;  // clear RX_WMRK_IRQ bit
    #ifdef PHY_ISR_HOOKS
    PhyPlmeRxSfdDetect();
    #endif //  PHY_ISR_HOOKS
  }
}

/***********************************************************************************/

#pragma CODE_SEG DEFAULT

