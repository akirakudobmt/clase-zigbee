/************************************************************************************
* MC1323X: Implements Packet Processor hardware access functionalities
* Handle initializations
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
#include "IoConfig.h"
#include "IrqControlLib.h"
#include "Phy.h"
#include "PhyDebugMC1323X.h"
#include "PlatformInit.h"


/************************************************************************************
*************************************************************************************
* Private macros
*************************************************************************************
************************************************************************************/

// Address mode indentifiers. Used for both network and MAC interfaces
#define gPhyAddrModeNoAddr_c        (0)
#define gPhyAddrModeInvalid_c       (1)
#define gPhyAddrMode16BitAddr_c     (2)
#define gPhyAddrMode64BitAddr_c     (3)

// CTL2 register settings for HW CCA types
#define  PHY_PP_CCA_TYPE_LINEAR     (0<<3)
#define  PHY_PP_CCA_TYPE_CCA        (1<<3) // default HW setting
#define  PHY_PP_CCA_TYPE_ED         (2<<3)

// PP_PHY_FRMREVTMR register settings for Event Timer prescaler
#define  PHY_PP_TMR_PRESCALER_2US   (2<<5)
#define  PHY_PP_TMR_PRESCALER_4US   (3<<5)
#define  PHY_PP_TMR_PRESCALER_8US   (4<<5)
#define  PHY_PP_TMR_PRESCALER_16US  (5<<5)
#define  PHY_PP_TMR_PRESCALER_32US  (6<<5)
#define  PHY_PP_TMR_PRESCALER_64US  (7<<5)

#define PHY_RNG_DELAY 0x0271
/***********************************************************************************/

#ifdef MAC_PHY_DEBUG  
  #define  PP_PHY_CTL3_RX_WMRK_MSK_DEBUG   0
#else
  #define  PP_PHY_CTL3_RX_WMRK_MSK_DEBUG   PP_PHY_CTL3_RX_WMRK_MSK_BIT
#endif // MAC_PHY_DEBUG

/************************************************************************************
*************************************************************************************
* Private prototypes
*************************************************************************************
************************************************************************************/

#ifdef MEMORY_MODEL_BANKED
#pragma CODE_SEG __NEAR_SEG NON_BANKED
#else
#pragma CODE_SEG DEFAULT
#endif /* MEMORY_MODEL_BANKED */

extern void PhyPassRxParams(phyRxParams_t * pRxParams);

/************************************************************************************
*************************************************************************************
* Private type definitions
*************************************************************************************
************************************************************************************/

/************************************************************************************
*************************************************************************************
* Public memory declarations
*************************************************************************************
************************************************************************************/

/************************************************************************************
*************************************************************************************
* Private memory declarations
*************************************************************************************
************************************************************************************/

/************************************************************************************
*************************************************************************************
* Public functions
*************************************************************************************
************************************************************************************/

volatile uint8_t i;
void PhyInit(void)
{
  PHY_DEBUG_LOG(PDBG_PHY_INIT);
  PhyPassRxParams(NULL);

  // reset the TRX logic
  IoIndirectWrite(PP_PHY_IAR_RESET, PP_PHY_IAR_BIT_TRCV_RST);

  //  PP_PHY_CTL2 unmask global TRX interrupts, enable 16 bit mode for TC2 - TC2 prime EN,
  PP_PHY_CTL2 =    (PP_PHY_CTL2_TC2PRIME_EN_BIT | \
                   (PP_PHY_CTL2_CCA_TYPE_MASK & PHY_PP_CCA_TYPE_CCA )) \
                & ~(PP_PHY_CTL2_TRCV_MSK_BIT    | \
                    PP_PHY_CTL2_PANCORDNTR_BIT  | \
                    PP_PHY_CTL2_TC3TMOUT_BIT    | \
                    PP_PHY_CTL2_TMRLOAD_BIT     | \
                    PP_PHY_CTL2_PROMISCUOUS_BIT);

  // clear all PP IRQ bits to avoid unexpected interrupts immediately after init
  PP_PHY_STATUS1 =  PP_PHY_STATUS1_TMR4_IRQ | \
                    PP_PHY_STATUS1_TMR3_IRQ | \
                    PP_PHY_STATUS1_TMR2_IRQ | \
                    PP_PHY_STATUS1_TMR1_IRQ;

  PP_PHY_STATUS2 =  PP_PHY_STATUS2_LO1UNLOCK_IRQ | \
                    PP_PHY_STATUS2_FLTR_FAIL_IRQ | \
                    PP_PHY_STATUS2_RX_WMRK_IRQ   | \
                    PP_PHY_STATUS2_CCA_IRQ       | \
                    PP_PHY_STATUS2_RX_IRQ        | \
                    PP_PHY_STATUS2_TX_IRQ        | \
                    PP_PHY_STATUS2_SEQ_IRQ;


#ifdef MAC_PHY_DEBUG  
  //  for debug purposes only - log all incoming SFD's
  PP_PHY_RXWTRMARK = 0;
#endif // MAC_PHY_DEBUG


  //  PP_PHY_CTL1 default HW settings  + AUTOACK enabled
  PP_PHY_CTL1 =     PP_PHY_CTL1_AUTOACK_BIT   & \
                  ~(PP_PHY_CTL1_TMRTRIGEN_BIT | \
                    PP_PHY_CTL1_SLOTTED_BIT   | \
                    PP_PHY_CTL1_CCABFRTX_BIT  | \
                    PP_PHY_CTL1_RXACKRQD_BIT  | \
                    PP_PHY_CTL1_XCVSEQ_MASK);

  //  PP_PHY_CTL3 : disable all interrupts, but RxWtrMark for debug purposes
  PP_PHY_CTL3  =    PP_PHY_CTL3_CRC_MSK_BIT       | \
                    PP_PHY_CTL3_CCA_MSK_BIT       | \
                    PP_PHY_CTL3_RX_WMRK_MSK_DEBUG | \
                    PP_PHY_CTL3_RX_MSK_BIT        | \
                    PP_PHY_CTL3_FLTR_FAIL_MSK_BIT | \
                    PP_PHY_CTL3_TX_MSK_BIT        | \
                    PP_PHY_CTL3_LO1UNLOCK_MSK_BIT | \
                    PP_PHY_CTL3_SEQ_MSK_BIT;

  //  PP_PHY_CTL4 : disable all timers and their interrupts
  PP_PHY_CTL4  =   (PP_PHY_CTL4_TMR4MSK_BIT     | \
                    PP_PHY_CTL4_TMR3MSK_BIT     | \
                    PP_PHY_CTL4_TMR2MSK_BIT     | \
                    PP_PHY_CTL4_TMR1MSK_BIT)    & \
                  ~(PP_PHY_CTL4_TMR4CMP_EN_BIT  | \
                    PP_PHY_CTL4_TMR3CMP_EN_BIT  | \
                    PP_PHY_CTL4_TMR2CMP_EN_BIT  | \
                    PP_PHY_CTL4_TMR1CMP_EN_BIT);

  //  PP_PHY_FRMREVTMR
  PP_PHY_FRMREVTMR = PP_PHY_FRMREVTMR_TMPRSC_MASK & PHY_PP_TMR_PRESCALER_16US;

  PP_PHY_SCTL = ((0x0F << PP_PHY_SCTL_INDEX_POS) | \
                    PP_PHY_SCTL_ACK_FP_BIT)      & \
                  ~(PP_PHY_SCTL_INDEX_DIS_BIT    | \
                    PP_PHY_SCTL_INDEX_EN_BIT     | \
                    PP_PHY_SCTL_SRC_ADDR_EN_BIT);

  PP_PHY_RXFRMFLT = PP_PHY_RXFRMFLT_CMD_FT_BIT   | \
                    PP_PHY_RXFRMFLT_DATA_FT_BIT  | \
                    PP_PHY_RXFRMFLT_BEACON_FT_BIT;

  //MC1323x IFR emulation
  IoIndirectWrite(0x18, 0x48); //"cg_md_bw=1, cg_ct_force=8  (Project Sync 8875)"
  IoIndirectWrite(0x06, 0x06); //"lo1_vco_buff_en=1, lo1_vco_buff_boost=1"
  IoIndirectWrite(0x14, 0x75); //"ampmixer_iref=7,ampmixer_lobuf1_ibias=1,  ampmixer_lobuf1_vbias=1"
  IoIndirectWrite(0x1A, 0x49); //mixpad_sel=9
  IoIndirectWrite(0x3D, 0x07); //Increase RxACK Turnaround Time by 14 nS  (Project Sync 8949)
  IoIndirectWrite(0x1D, 0x40); //mic_early (enable VREGM simul with VREGA)
  IoIndirectWrite(0x0F, 0x15); // VREGM trim

  //MC1323x setup
  IoIndirectWrite(0x61, 0x01); // set vrega_en_ovrd_en
  IoIndirectWrite(0x60, 0x01); // set vrega_en_ovrd
  //MC1323x PSM manual calibration
  //200 us delay
  for(i=0;i<250;i++)
  {
	;
  }
  IoIndirectWrite(0x10, 0x2F); // psm manual calibration
  IoIndirectWrite(0x10, 0x3F); // psm manual calibration
                                
  (void)PhyPlmeSetCurrentChannelRequest(0x0B); //2405 MHz
  (void)PhyPlmeSetPwrLevelRequest(0x0C); //set the power level to 0dBm
  (void)PhyPpSetCcaThreshold(0x00);

#if (gPpdMode_d != gPpdModeOFF_d)  
  PPD_InitPpdMode();
#endif //gPpdMode_d             
  
  PP_PHY_MAXFRMLEN = 0x7F;
}

/***********************************************************************************/

void    PhyPpSetPromiscuous(bool_t mode)
{
  PHY_DEBUG_LOG(PDBG_PP_SET_PROMISC);
  if(TRUE == mode)
  {
    PP_PHY_CTL2     |=   PP_PHY_CTL2_PROMISCUOUS_BIT;
    PP_PHY_RXFRMFLT |=   PP_PHY_RXFRMFLT_ACK_FT_BIT | PP_PHY_RXFRMFLT_NS_FT_BIT;
  }
  else
  {
    PP_PHY_CTL2     &=  ~PP_PHY_CTL2_PROMISCUOUS_BIT;
    PP_PHY_RXFRMFLT &= ~(PP_PHY_RXFRMFLT_ACK_FT_BIT | PP_PHY_RXFRMFLT_NS_FT_BIT);
  }
}

/***********************************************************************************/


void    PhyAbort(void)
{
  uint8_t irqStatus;
  volatile uint8_t time = 0;
#if (gPpdMode_d != gPpdModeOFF_d)      
  volatile uint8_t temp;
#endif //(gPpdMode_d != gPpdModeOFF_d)    
  
  IrqControlLib_BackupIrqStatus(irqStatus);
  IrqControlLib_DisableAllIrqs();

  PHY_DEBUG_LOG(PDBG_PHY_ABORT);
  
  // disable timer trigger of the already programmed packet processor sequence, if any
  PP_PHY_CTL1   &=  ~PP_PHY_CTL1_TMRTRIGEN_BIT;
  
  // give the FSM enough time to start if it was triggered
  time = PP_PHY_EVENTTMR0;
  while((PP_PHY_EVENTTMR0 - time) < 2);

  // make sure that we abort in HW only if the sequence was actually started (tmr triggered)
  if((gIdle_c != PhyGetSeqState()) && (PP_PHY_FSM != 0))
  {
#if (gPpdMode_d != gPpdModeOFF_d)      
    // set the HW sequencer to idle state
    temp = IoIndirectRead(0x17);
    IoIndirectWrite(0x17, 0x00);   //disable the PPD
#endif //(gPpdMode_d != gPpdModeOFF_d)    
    
    
    PP_PHY_CTL1 &= ~PP_PHY_CTL1_XCVSEQ_MASK;  
    
#if (gPpdMode_d != gPpdModeOFF_d)        
    IoIndirectWrite(0x17, temp); //restore the PPD mode
#endif //(gPpdMode_d != gPpdModeOFF_d)        
    
    while (PP_PHY_FSM != 0)
    ;
  }
  
  

  // mask SEQ interrupt
  PP_PHY_CTL3   |=  PP_PHY_CTL3_SEQ_MSK_BIT;
  
  // stop timers
  PP_PHY_CTL2   &=  ~PP_PHY_CTL2_TC3TMOUT_BIT;
  PP_PHY_CTL4   &= ~(PP_PHY_CTL4_TMR3CMP_EN_BIT | PP_PHY_CTL4_TMR2CMP_EN_BIT);
  PP_PHY_CTL4   |=   PP_PHY_CTL4_TMR2MSK_BIT    | PP_PHY_CTL4_TMR3MSK_BIT;     // mask TMR2 and TMR3 interrupts
  
  PhyPassRxParams(NULL); 

  // clear all PP IRQ bits to avoid unexpected interrupts
  PP_PHY_STATUS1  = PP_PHY_STATUS1;
  PP_PHY_STATUS2  = PP_PHY_STATUS2;

  IrqControlLib_RestoreIrqStatus(irqStatus);
}

/***********************************************************************************/

void    PhyPpSetPanId(uint8_t *pPanId)
{
  PHY_DEBUG_LOG(PDBG_PP_SET_PANID);

#ifdef PHY_PARAMETERS_VALIDATION
  if(NULL == pPanId)
  {
    return;
  }
#endif // PHY_PARAMETERS_VALIDATION

  PP_PHY_PANIDH = pPanId[1];
  PP_PHY_PANIDL = pPanId[0];
}

/***********************************************************************************/

void    PhyPpSetShortAddr(uint8_t *pShortAddr)
{
  PHY_DEBUG_LOG(PDBG_PP_SET_SHORTADDR);
  
#ifdef PHY_PARAMETERS_VALIDATION
  if(NULL == pShortAddr)
  {
    return;
  }
#endif // PHY_PARAMETERS_VALIDATION

  PP_PHY_SHRTADDRH = pShortAddr[1];
  PP_PHY_SHRTADDRL = pShortAddr[0];
}

/***********************************************************************************/

void    PhyPpSetLongAddr(uint8_t *pLongAddr)
{
  PHY_DEBUG_LOG(PDBG_PP_SET_LONGADDR);

#ifdef PHY_PARAMETERS_VALIDATION
  if(NULL == pLongAddr)
  {
    return;
  }
#endif // PHY_PARAMETERS_VALIDATION

  PP_PHY_LONGADR0  = pLongAddr[0];
  PP_PHY_LONGADR8  = pLongAddr[1];
  PP_PHY_LONGADR16 = pLongAddr[2];
  PP_PHY_LONGADR24 = pLongAddr[3];
  PP_PHY_LONGADR32 = pLongAddr[4];
  PP_PHY_LONGADR40 = pLongAddr[5];
  PP_PHY_LONGADR48 = pLongAddr[6];
  PP_PHY_LONGADR56 = pLongAddr[7];
}

/***********************************************************************************/

void    PhyPpSetMacRole(bool_t macRole)
{
  PHY_DEBUG_LOG(PDBG_PP_SET_MACROLE);
  if(gMacRole_PanCoord_c == macRole)
  {
    PP_PHY_CTL2 |=  PP_PHY_CTL2_PANCORDNTR_BIT;
  }
  else
  {
    PP_PHY_CTL2 &= ~PP_PHY_CTL2_PANCORDNTR_BIT;
  }
}

/***********************************************************************************/

void    PhyPpSetCcaThreshold(uint8_t ccaThreshold)
{
  PP_PHY_CCATHRSHD = ccaThreshold;
}

/***********************************************************************************/

bool_t  PhyPpIsTxAckDataPending(void)
{
  if(PP_PHY_STATUS1 & PP_PHY_STATUS1_SRCADDR_BIT)
  {
    return TRUE;
  }
  return FALSE;
}

/***********************************************************************************/

bool_t  PhyPpIsRxAckDataPending(void)
{
  if(PP_PHY_STATUS2 & PP_PHY_STATUS2_FRPEND_BIT)
  {
    return TRUE;
  }
  return FALSE;
}

/***********************************************************************************/

bool_t  PhyPpIsPollIndication(void)
{
  if(PP_PHY_STATUS1 & PP_PHY_STATUS1_PI_BIT)
  {
    return TRUE;
  }
  return FALSE;
}

/***********************************************************************************/

void    PhyPp_AddToIndirect(uint8_t index, uint8_t *pPanId, uint8_t *pAddr, uint8_t AddrMode)
{
  uint16_t mAddrChecksum, tmp16;
  uint8_t  iAddr;

  PHY_DEBUG_LOG1(PDBG_PP_ADD_INDIRECT, index);

#ifdef PHY_PARAMETERS_VALIDATION
  if((NULL == pPanId) || (NULL == pAddr))
  {
    return;
  }
  if( (gPhyAddrMode64BitAddr_c != AddrMode) && (gPhyAddrMode16BitAddr_c != AddrMode) )
  {
    return;
  }
#endif // PHY_PARAMETERS_VALIDATION

  mAddrChecksum  =  * pPanId;
  mAddrChecksum |= (*(pPanId+1))<<8;
  if( gPhyAddrMode16BitAddr_c == AddrMode )
  {
    // chksum = (dstPANID + dstAddr[15:0]) % 65536
    iAddr = 1;
  } 
  else // 64 bit address mode
  {
    // chksum = (dstPANID + dstAddr[15: 0]) % 65536
    // chksum = (chksum   + dstAddr[31:15]) % 65536
    // chksum = (chksum   + dstAddr[47:32]) % 65536
    // chksum = (chksum   + dstAddr[63:48]) % 65536
    iAddr = 4;
  }

  do
  {
    tmp16  =  * pAddr;
    tmp16 |= (*(pAddr+1))<<8;
    mAddrChecksum += tmp16;
    
    pAddr += 2;
    iAddr--;
  } while(iAddr != 0);

  PP_PHY_SCTL = ((index << PP_PHY_SCTL_INDEX_POS) | PP_PHY_SCTL_SRC_ADDR_EN_BIT) & ~(PP_PHY_SCTL_INDEX_DIS_BIT  |   PP_PHY_SCTL_INDEX_EN_BIT  | PP_PHY_SCTL_ACK_FP_BIT);
  PP_PHY_SADRSUMD = mAddrChecksum;
  PP_PHY_SCTL = ((index << PP_PHY_SCTL_INDEX_POS) | PP_PHY_SCTL_SRC_ADDR_EN_BIT  |   PP_PHY_SCTL_INDEX_EN_BIT)  & ~(PP_PHY_SCTL_INDEX_DIS_BIT | PP_PHY_SCTL_ACK_FP_BIT);
}

/***********************************************************************************/

void     PhyPp_RemoveFromIndirect(uint8_t index)
{
  PHY_DEBUG_LOG1(PDBG_PP_REMOVE_INDIRECT, index);
  PP_PHY_SCTL = ((index << PP_PHY_SCTL_INDEX_POS) | PP_PHY_SCTL_SRC_ADDR_EN_BIT  |   PP_PHY_SCTL_INDEX_DIS_BIT) & ~(PP_PHY_SCTL_INDEX_EN_BIT  | PP_PHY_SCTL_ACK_FP_BIT);
}

/***********************************************************************************/

uint8_t PhyPpReadLatestIndex(void)
{
  uint8_t retVal;
  retVal = (PP_PHY_SCTL >> PP_PHY_SCTL_INDEX_POS);
  PHY_DEBUG_LOG1(PDBG_PP_READ_LATEST_INDEX, retVal);
  return retVal;
}

/***********************************************************************************/

void    PhyEnableBER(void)
{
  IoIndirectWrite(PP_PHY_IAR_DIG_TESTMODE,  PP_PHY_IAR_BIT_FPGA_EN |  PP_PHY_IAR_BIT_IDEAL_PSM_EN | PP_PHY_IAR_BIT_MODE_TMUX_EN | PP_PHY_IAR_BIT_BER_EN);
}

/***********************************************************************************/

void    PhyDisableBER(void)
{
  IoIndirectWrite(PP_PHY_IAR_DIG_TESTMODE,  PP_PHY_IAR_BIT_FPGA_EN |  PP_PHY_IAR_BIT_IDEAL_PSM_EN | PP_PHY_IAR_BIT_MODE_TMUX_EN);
}

/***********************************************************************************/

void PhyGetRandomNo(uint32_t *pRandomNo) {
      
  uint32_t startTime, endTime;
  uint8_t  *ptr = (uint8_t *)pRandomNo;
  uint8_t  i = 4, iar60save, iar61save;
  
   //slightly alter the FRACL register for increasing the randomness
  PP_PHY_LO1FRACL ++;
  
  // backup 0x60 and 0x61 IARs
  iar60save = IoIndirectRead(0x60);
  iar61save = IoIndirectRead(0x61);
  

   // Turn ON the override enable
  IoIndirectWrite(0x61, 0x07);
   // override method
  IoIndirectWrite(0x60, 0x07);
  
  PhyTimeReadClock(&startTime);        
  PhyTimeReadClock(&endTime);  
  
   // wait 10000 symbols
  while( ((endTime - startTime) & 0x00FFFFFF) < PHY_RNG_DELAY) {
    PhyTimeReadClock(&endTime);              
  }
  
   // override method
  IoIndirectWrite(0x60, 0x03);  
  
  PP_PHY_INDACCIDX = 0x0A;  
  while(i--) {
    *ptr++ = PP_PHY_INDACCREG;
  }
  
  PP_PHY_LO1FRACL --;
  
  //restore IARs 0x60 and 0x61
  IoIndirectWrite(0x60, iar60save);
  IoIndirectWrite(0x61, iar61save);
}

/***********************************************************************************/

void    PhyEnableTxNoModulation(void)
{
  uint8_t tmp = IoIndirectRead(0x13);
  IoIndirectWrite(0x13, tmp | 0x80);
}

/***********************************************************************************/

void    PhyDisableTxNoModulation(void)
{
  uint8_t tmp = IoIndirectRead(0x13);
  IoIndirectWrite(0x13, tmp & 0x7F);
}

/***********************************************************************************/

/************************************************************************************
*************************************************************************************
* Module debug stuff
*************************************************************************************
************************************************************************************/

#pragma CODE_SEG DEFAULT
