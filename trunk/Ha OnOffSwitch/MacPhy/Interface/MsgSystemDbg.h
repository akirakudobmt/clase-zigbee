/************************************************************************************
* This is the header file for the memory and message module.
*
* (c) Copyright 2007, Freescale, Inc.  All rights reserved.
*
* No part of this document must be reproduced in any form - including copied,
* transcribed, printed or by any electronic means - without specific written
* permission from Freescale Semiconductor.
*
************************************************************************************/


#ifndef _MSG_SYSTEMDBG_H_
#define _MSG_SYSTEMDBG_H_

#include "PlatformToMacPhyDbgConfig.h" // For the MM_DEBUG define.
#include "NwkMacInterface.h"
#include "AppToMacPhyConfig.h"
#include "FunctionLib.h"

/************************************************************************************
*************************************************************************************
* Public macros
*************************************************************************************
************************************************************************************/

// maximum length for buffer to allow for req and payload for all layers
#define gMaxRxTxDataLength_c (147)
#define gMaxMsduDataLength_c (118)

/*
Description of data packet format in transmitting direction:
Please note that the packet formatted message does not¨include the Preamble
sequence, Start of frame delimiter and the CRC (FCS) because these fields
are read and processed by the MC1319x hardware.

------------------------------------------------------------------------------------
Name	           Size     Range	        Notes
------------------------------------------------------------------------------------
Frame length     1	      9-127         Note that this is the length used in the PHY
      	                                packet to be sent over the air. It does not
                                        specify the length of this message.
Link quality	   1	      0x00-0xff	      
Header length    1                      Length of header in bytes
Header type      1                      Enum for variations of header types
Frame control	   2	      Bitmap	
Sequence	no.    1	      0x00-0xff
Dst PAN ID	     0/2		
Dst Address      0/2/8		
Src PAN ID	     0/2		
Src Address      0/2/8		
Frame counter	   0/4		                 For security. Running increment
Key seq counter  0/1		                 For security.
Msdu	           0-102		               May be encrypted
Integrity code   0/4/8/16	  	           May be encrypted
------------------------------------------------------------------------------------


The Frame control field contains the following bits:
------------------------------------------------------------------------------------
Name	            Index  Size	 Range	   Notes
------------------------------------------------------------------------------------
Frame type	      0-2	   3	   0x0-0x3	 Beacon, Data, Acknowledgement, MAC command
Security enabled	3	     1		           From TxOptions
Frame pending	    4	     1		           Controlled by MEM/MCPS/MLME
Ack. req.	        5	     1		           From TxOptions
Intra PAN	        6	     1		
DstAddrMode	      10-11	 2	   0x0-0x3	
SrcAddrMode	      14-15	 2	   0x0-0x3
------------------------------------------------------------------------------------
*/
struct rxPacket_tag{
  uint8_t frameLength;
  uint8_t linkQuality;
  uint8_t headerLength;
  uint8_t rxData[gMaxRxTxDataLength_c];
  zbClock24_t timeStamp;
};                

/*
Description of data packet format in transmitting direction:
Please note that the packet formatted message does not include the Preamble
sequence, Start of frame delimiter and the CRC (FCS) because these fields are
added by the MC1319x hardware.
Also, please note that this message includes the msduHandle, and the txInfo
fields. These are included for internal bookkeeping and are not transmitted
over the air. The msduHandle is valid for data packets only.
Length of entire packet is frameLength + 1 ("+1" includes length, MsduHandle
and Message type info fields - excludes CRC field).

------------------------------------------------------------------------------------
Name	           Size	    Range	        Notes
------------------------------------------------------------------------------------
Frame length     1	      9-127         Note that this is the length used in the PHY
      	                                packet to be sent over the air. It does not
                                        specify the length of this message.
MsduHandle	     1		                    
txInfo 	         1		                  Direct/Indirect/GTS, Allow pending frame modify
Frame control	   2	      Bitmap	
Sequence	no.    1	      0x00-0xff	    Controlled by MCPS/MLME modules!
Dst PAN ID       0/2		
Dst Address      0/2/8		
Src PAN ID  	   0/2		
Src Address      0/2/8		
Frame counter	   0/4		                For security. Running increment
Key seq counter  0/1		                For security. Set by network
msdu	           0-102		              May be encrypted
Integrity code   0/4/8/16		            May be encrypted
------------------------------------------------------------------------------------

The Frame control field is identical to  the one defined for rxPacket type.
*/
struct txPacket_tag{
  uint8_t frameLength; // Length of MPDU/PSDU (length of txData + 2 for CRC)
  uint8_t txInfo; // Direct/indirect/GTS infom; allow frame pending changes in MEM; etc.
  struct {
    uint8_t csmaCaNb;   // The number of csma-ca attemps.
    uint8_t csmaCaBe;   // The csma-ca backoff exponent.
    uint8_t txCount;    // The number of tx attempts.
  } csmaAndTx;
  uint16_t expireTime;  // Only used for coordinator capability 
  uint8_t msduHandle;   // Valid for data packets only
  uint8_t txData[gMaxRxTxDataLength_c];
};

  // Maximum buffer sizes to be used:
#define gMaxRxPacketBufferSize_c   (sizeof(struct rxPacket_tag))
#define gMaxTxPacketBufferSize_c   (sizeof(struct txPacket_tag))
#define gMaxMcpsDataReqSize_c      (sizeof(nwkToMcpsMessage_t) + gMaxMsduDataLength_c)
#define gMaxMcpsDataIndSize_c      (sizeof(mcpsToNwkMessage_t) + gMaxMsduDataLength_c)
#define gMaxMcpsDataBufferSize_c   (FLib_GetMax(gMaxMcpsDataReqSize_c, gMaxMcpsDataIndSize_c))
#define gMaxRxTxPacketBufferSize_c (FLib_GetMax(gMaxRxPacketBufferSize_c, gMaxTxPacketBufferSize_c))
#define gMaxPacketBufferSize_c     (FLib_GetMax(gMaxRxTxPacketBufferSize_c, gMaxMcpsDataBufferSize_c))


/************************************************************************************ 
************************************************************************************* 
* Public type definitions 
************************************************************************************* 
************************************************************************************/ 
  // This struct is used privately but must be declared
  // here in order for the anchor_t struct to work.
  // The struct is used in memory blocks for linking them
  // in single chained lists. The struct is embedded in
  // list elements, and transparent to the users. 

typedef struct listHeader_tag {
  struct listHeader_tag *pNext;
  struct pools_tag *pParentPool;
  uint8_t *fileName;
  uint16_t lineNumber;
  uint8_t *queueInfo;
} listHeader_t;

  // List anchor with head and tail elements. Used 
  // for both memory pools and message queues.
typedef struct anchor_tag {
  listHeader_t *pHead;
  listHeader_t *pTail;
  uint8_t numElements;
} anchor_t;

typedef struct poolStat_tag {
  uint8_t numBlocks;
  uint8_t allocatedBlocks;
  uint8_t allocatedBlocksPeak;
  uint16_t allocationFailures;
} poolStat_t;

typedef struct pools_tag {
  anchor_t anchor; // MUST be first element in pools_t struct
  //void *pEndAddress;
  uint8_t nextBlockSize;
  uint8_t blockSize;
  poolStat_t poolStat;
} pools_t;

typedef struct poolInfo_tag {
  uint8_t poolSize;
  uint8_t blockSize;
  uint8_t nextBlockSize;
} poolInfo_t;

typedef anchor_t msgQueue_t;


#define MSG_AllocType(type) MM_AllocDebug(sizeof(type), __FILE__, (uint16_t)(__LINE__))
#define MSG_Alloc(size) MM_AllocDebug(size, __FILE__, (uint16_t)(__LINE__))
#define MSG_Send(sap, msg) (sap##_SapHandlerDebug((void *)(msg), __FILE__, (uint16_t)(__LINE__)))
  // Not public functions any longer
#define MM_Alloc(size) MM_AllocDebug(size, __FILE__, (uint16_t)(__LINE__))
#define MM_AllocPool(pools, size) MM_AllocPoolDebug(pools, size, __FILE__, (uint16_t)(__LINE__))
#define MSG_Queue(anchor, element) { List_AddTail((anchor), (element)); (((listHeader_t *)(element))-1)->queueInfo = "Queued in " #anchor ; }

  // Free a message
#define MSG_Free(msg) MM_Free(msg)
  // Put a message in a queue at the head. 
#define MSG_QueueHead(anchor, element) List_AddHead((anchor), (element))
  // Get a message from a queue. Returns NULL if no messages in queue.
#define MSG_DeQueue(anchor) List_RemoveHead(anchor)
  // Check if a message is pending in a queue. Returns
  // TRUE if any pending messages, and FALSE otherwise.
#define MSG_Pending(anchor) ((anchor)->pHead != 0)
#define MSG_InitQueue(anchor) List_ClearAnchor(anchor)


/************************************************************************************
* This function initializes the message module private variables. Must be
* called at boot time, or if device is reset. Currently the module supports
* up to 4 memory pools, but pools may be added or removed. The following 
* constants control the memory pool layout:
* 
* gMmPoolSize0_c,  gMmBlockSize0_c
* gMmPoolSize1_c,  gMmBlockSize1_c
* gMmPoolSize2_c,  gMmBlockSize2_c
* gMmPoolSize3_c,  gMmBlockSize3_c
* 
* The gMmPoolSize*_c constants determine the number of blocks in a pool, and
* gMmBlockSize*_c is the number of bytes in each block for the corresponding pool.
* The number of bytes is rounded up to a value so that each block is aligned to
* a machine dependant boundary in order to avoid bus errors during pool access.
*
* The total amount of heap required is given by the constant mMmTotalPoolSize_c.
*
* Interface assumptions:
*   None
*   
************************************************************************************/
void MM_Init
  (
  uint8_t *pHeap,         // IN: Memory heap. Caller must be sure to make this big enough
  const poolInfo_t *pPoolInfo,  // IN: Memory layout information
  pools_t *pPools         // OUT: Will be initialized with requested memory pools.
  );

/************************************************************************************
* Deallocate a memory block by putting it in the corresponding pool of free blocks.
*   
* Interface assumptions:
*   None
*   
* Return value:
*   None.
* 
************************************************************************************/
void MM_Free
  (
  void *pBlock // IN: Block of memory to free
  );

/************************************************************************************
* This function can be used to add a memory block to the specified pool during
* runtime. The following example shows how to add two buffers to the MAC buffer
* pool:
*
*   // Declare an array with two buffers
*   uint8_t myExtraMacBuffers[2][sizeof(listHeader_t) + gMaxPacketBufferSize_c];
*   // Get a pointer to the MAC pools. The size must be correct.
*   pools_t *macDataBufferPool = MM_GetMacPool(gMaxPacketBufferSize_c);
*   // Add the two extra buffers to the MAC buffer pool.
*   MM_AddToPool(macDataBufferPool, myExtraMacBuffers[0]);
*   MM_AddToPool(macDataBufferPool, myExtraMacBuffers[1]);
*
* Interface assumptions:
*   None
*   
* Return value:
*   None
* 
************************************************************************************/
void MM_AddToPool
  (
  pools_t *pPool,
  void *pBlock
  );

/************************************************************************************
* This function returns a pointer to the MAC buffer pool which corresponds exactly
* to the size argument. This is used with the MM_AddToPool() function to add buffers
* to the MAC buffer pool.
*
* Interface assumptions:
*   None
*   
* Return value:
*   Pointer to the requested buffer pool
* 
************************************************************************************/
pools_t *MM_GetMacPool
  (
  uint8_t size // IN: Exact size which represents the requested buffer pool.
  );


/************************************************************************************
* Initialize a list anchor with a NULL list header. Used for preparing an anchor for
* first time use
*
* Interface assumptions:
*   None
*   
* Return value:
*   None.
* 
************************************************************************************/
void List_ClearAnchor
  (
  anchor_t *pAnchor // IN: Anchor of list to reset
  );

/************************************************************************************
* Links a list element to the tail of the list given by the anchor argument.
* This function is amongst other useful for FIFO queues if combined with
* List_RemoveHead.
*
* Interface assumptions:
*   None
*   
* Return value:
*   None.
* 
************************************************************************************/
void List_AddTail
  (
  anchor_t *pAnchor, //IN: Anchor of list to add element to
  void *pBlock       //IN: Element to add to tail of list
  );

/************************************************************************************
* Links a list element to the head of the list given by the anchor argument.
* Useful for FILO buffers (push/pop stacks). This function should only be 
* included in the build if really required.
*
* Interface assumptions:
*   None
*   
* Return value:
*   None.
* 
************************************************************************************/
void List_AddHead
  (
  anchor_t *pAnchor, //IN: Anchor of list to add element to
  void *pBlock       //IN: Element to add to head of list
  );

/************************************************************************************
* Unlinks a list element from the head of the list given by the anchor argument.
*
* Interface assumptions:
*   None
*   
* Return value:
*   Pointer to the unlinked list element or NULL if no elements available.
* 
************************************************************************************/
void *List_RemoveHead
  (
  anchor_t *pAnchor  //IN: Anchor of list to remove head from
  );

/************************************************************************************
* Removes a list element from anywhere in the list.
*
* This function should be used somewhat like in the following example since it
* requires a pointer to the previous block relative to the current block:
*
*  void *pBlock = List_GetFirst(pMyList);
*  void *pPrev = NULL;
*  anchor_t *pMyList;
*
*  while(pBlock) {
*    if(CheckStuff(pBlock) == RIGHT_STUFF) {
*      List_Remove(pMyList, pPrev, pBlock);
*      break;
*    }
*    pPrev = pBlock;
*    pBlock = List_GetNext(pBlock);
*  }
*
* Interface assumptions:
*   None
*   
* Return value:
*   None
* 
************************************************************************************/
void List_Remove
  (
  anchor_t *pAnchor, //IN: List to remove block from
  void *pPrevBlock,  //IN: Previous List element used to fix the list after modification.
  void *pBlock       //IN: List element used to reference the next element in the list.
  );

/************************************************************************************
* Returns a pointer to the first element in the list.
*
* Interface assumptions:
*   None
*   
* Return value:
*   Pointer to the next list element or NULL if no elements available.
* 
************************************************************************************/
void *List_GetFirst
  (
  anchor_t *pAnchor  //IN: The list where the first element will be returned from.
  );

/************************************************************************************
* Returns a pointer to the next list element in the list.
*
* Interface assumptions:
*   None
*   
* Return value:
*   Pointer to the next list element or NULL if no elements available.
* 
************************************************************************************/
void *List_GetNext
  (
  void *pBlock  //IN: List element used to reference the next element in the list.
  );


/************************************************************************************
*************************************************************************************
* Private type declarations
*************************************************************************************
************************************************************************************/



  // Used for calculating size of blocks in pool #0
typedef union {
  nwkMessage_t    nwkMessage;
  panDescriptor_t panDescriptor;
} block1sizeOrigin_t;


/************************************************************************************
*************************************************************************************
* Private macro definitions
*************************************************************************************
************************************************************************************/

  // Number of pools
#if gBigMsgsMacOnly_d > 0
#define gMmNumPools_c 3 // 1 Small message pool, 1 Large data pool, 1 Large data pool (MAC private)
#else
#define gMmNumPools_c 2 // 1 Small message pool, 1 Large data pool
#endif // gBigMsgsMacOnly_d

  // The block sizes will be rounded up to multipla of largest
  // data integer type (a pointer or uint16_t for HCS08).
  // Extra space for list headers will be added automagically.
  // The block sizes must be in ascending order. If fewer than
  // 4 pools are used, then gMmPoolSize*_c, and gMmBlockSize*_c
  // for the unused pools must be defined to the value 0.

  // TBD: Currently many "small" messages uses union in deciding message size even if
  //      the message is smaller. Not a big problem, but could be improve and that
  //      may affect necessary sizes of messages!
#define gMmPoolSize0_c  (gTotalSmallMsgs_d)
#define gMmBlockSize0_c (sizeof(block1sizeOrigin_t)) // ~22 bytes

#define gMmPoolSize1_c  (gTotalBigMsgs_d - gBigMsgsMacOnly_d)
#define gMmBlockSize1_c gMaxPacketBufferSize_c

  // Pool[2] is private to the MAC.
#define gMmPoolSize2_c  gBigMsgsMacOnly_d
// Make sure that NWK-MLME data struct is SMALLER than this. Consider adding
// getMax(gMaxPacketBufferSize_c,sizeof(nwkToMcpsMessage_t)+aMaxMacFrameSize-1).
#if gMmPoolSize2_c != 0
 #define gMmBlockSize2_c gMaxPacketBufferSize_c
#else
 #define gMmBlockSize2_c 0
#endif // gBigMsgsMacOnly_d != 0

#define gMmPoolSize3_c  0
#define gMmBlockSize3_c 0


  // Make sure that all blocks are aligned correctly (  Round up: (((a + (s-1)) / s) * s), s=4 -> (((a+3) >> 2) << 2)  )
#define mMmBlockSize0_c ((((gMmBlockSize0_c) + (sizeof(uint8_t *) - 1)) / sizeof(uint8_t *)) * sizeof(uint8_t *))
#define mMmBlockSize1_c ((((gMmBlockSize1_c) + (sizeof(uint8_t *) - 1)) / sizeof(uint8_t *)) * sizeof(uint8_t *))
#define mMmBlockSize2_c ((((gMmBlockSize2_c) + (sizeof(uint8_t *) - 1)) / sizeof(uint8_t *)) * sizeof(uint8_t *))
#define mMmBlockSize3_c ((((gMmBlockSize3_c) + (sizeof(uint8_t *) - 1)) / sizeof(uint8_t *)) * sizeof(uint8_t *))

  // The total number of bytes in each pool including list headers
#define mMmPoolByteSize0_c ((gMmPoolSize0_c) * (mMmBlockSize0_c + sizeof(listHeader_t)))
#define mMmPoolByteSize1_c ((gMmPoolSize1_c) * (mMmBlockSize1_c + sizeof(listHeader_t)))
#define mMmPoolByteSize2_c ((gMmPoolSize2_c) * (mMmBlockSize2_c + sizeof(listHeader_t)))
#define mMmPoolByteSize3_c ((gMmPoolSize3_c) * (mMmBlockSize3_c + sizeof(listHeader_t)))

  // Total number of bytes in all pools together
#define mMmTotalPoolSize_c (mMmPoolByteSize0_c + mMmPoolByteSize1_c + mMmPoolByteSize2_c + mMmPoolByteSize3_c)


#if gMmNumPools_c == 1
#define mMmBlockSizeLargest_c gMmBlockSize0_c // Size of largest block
#define mMmLastPoolIdx_c 0  // Index of last pool
#endif // gMmNumPools_c == 1

#if gMmNumPools_c == 2
#define mMmBlockSizeLargest_c gMmBlockSize1_c // Size of largest block
#define mMmLastPoolIdx_c 1 // Index of last pool
#endif // gMmNumPools_c == 2

#if gMmNumPools_c == 3
#define mMmBlockSizeLargest_c gMmBlockSize2_c // Size of largest block
#define mMmLastPoolIdx_c 2 // Index of last pool
#endif // gMmNumPools_c == 3

#if gMmNumPools_c == 4
#define mMmBlockSizeLargest_c gMmBlockSize3_c // Size of largest block
#define mMmLastPoolIdx_c 3 // Index of last pool
#endif // gMmNumPools_c == 4


typedef struct block0_tag {
  listHeader_t  listHeader;
  uint8_t       data[mMmBlockSize0_c];
} block0_t;

#if gMmNumPools_c > 1
typedef struct block1_tag {
  listHeader_t  listHeader;
  uint8_t       data[mMmBlockSize1_c];
} block1_t;
#endif // gMmNumPools_c > 1

#if gMmNumPools_c > 2
typedef struct block2_tag {
  listHeader_t  listHeader;
  uint8_t       data[mMmBlockSize2_c];
} block2_t;
#endif // gMmNumPools_c > 2

#if gMmNumPools_c > 3
typedef struct block3_tag {
  listHeader_t  listHeader;
  uint8_t       data[mMmBlockSize3_c];
} block3_t;
#endif // gMmNumPools_c > 3


  // Number of pools
#if gBigMsgsMacOnly_d > 0
#define gMmNumPools_c 3 // 1 Small message pool, 1 Large data pool, 1 Large data pool (MAC private)
#else
#define gMmNumPools_c 2 // 1 Small message pool, 1 Large data pool
#endif // gBigMsgsMacOnly_d

typedef struct macHeap_tag {
  block0_t block0[gMmPoolSize0_c];
#if gMmNumPools_c > 1
  block1_t block1[gMmPoolSize1_c];
#if (gMmNumPools_c > 2) && (gMmPoolSize2_c != 0)
  block2_t block2[gMmPoolSize2_c];
#if gMmNumPools_c > 3
  block3_t block3[gMmPoolSize3_c];
#endif // gMmNumPools_c > 3
#endif // gMmNumPools_c > 2
#endif // gMmNumPools_c > 1
} macHeap_t;

#endif /* _MSG_SYSTEMDBG_H_ */
