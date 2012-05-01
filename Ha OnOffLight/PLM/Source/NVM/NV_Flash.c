/*****************************************************************************
* Non-volatile storage module local implementation.
*
* Copyright (c) 2006, Freescale, Inc. All rights reserved.
*
*
* No part of this document must be reproduced in any form - including copied,
* transcribed, printed or by any electronic means - without specific written
* permission from Freescale Semiconductor.
*
*****************************************************************************/

#include "EmbeddedTypes.h"
#include "TS_Interface.h"

#ifndef gBeeStackIncluded_d
#define gBeeStackIncluded_d 0
#endif

#if gBeeStackIncluded_d
#include "Beestack_Globals.h"
#endif /* gBeeStackIncluded_d */

#include "NV_FlashHAL.h"
#include "NV_Flash.h"
#include "FunctionLib.h"

/*****************************************************************************
******************************************************************************
* Private macros
******************************************************************************
*****************************************************************************/

/* Size of a member of a struct. */
#ifndef MbrSizeof
#define MbrSizeof(type, member)     (sizeof(((type *) 0)->member))
#endif

#define  gNVPageMaxOffset_c  (NvSize_t)( sizeof(NvRawPage_t) - sizeof(NvStructuredPageHeader_t))

/* There must be at least one more flash page than there are data sets, to */
/* allow for one copy of each data set plus a page for a new copy of one */
/* data set. */
#if gNvNumberOfRawPages_c <= gNvNumberOfDataSets_c
#error
#endif

/* this macro must reflect the bit used for the idle task defined in BeeApp.c */
#define gIdleTaskNVIntervalEvent_c  ( 1 << 0 )


/*****************************************************************************
******************************************************************************
* Private prototypes
******************************************************************************
*****************************************************************************/
#if gNvStorageIncluded_d
static index_t NvDataSetIndexFromID(NvDataSetID_t dataSetID);
static index_t NvFindDataSet(NvDataSetID_t dataSetID);
static bool_t NvIsValidDataSet(index_t pageIndex);
static void NvSaveDataSet(index_t dataSetIndex);
 #if gNvMultiStorageIncluded_d
 static bool_t NvInPageMultiSave( index_t dataSetIndex );
 #endif
#else
#define NvDataSetIndexFromID(dataSetID)  0
#define NvFindDataSet(dataSetID)  0
#define NvIsValidDataSet(pageIndex)  FALSE
#define NvSaveDataSet(dataSetIndex)
#endif /* gNvStorageIncluded_d */

#if gNvSelfTest_d
static void NvSelfTest(void);
#endif

/*****************************************************************************
******************************************************************************
* Private type definitions
******************************************************************************
*****************************************************************************/

/* One entry per data set. */
typedef struct NvDataSetInfo_tag {
  bool_t saveNextInterval;
  NvSaveInterval_t ticksToNextSave;
  NvSaveCounter_t countsToNextSave;
} NvDataSetInfo_t;

typedef struct NvMultiSavedDataInfo_tag {
  void * pSource;
  NvSize_t sourceLength;
} NvMultiSavedDataInfo_t;

typedef struct msMarker_tag {
  uint8_t header;
  uint8_t trailer;
} msMarker_t;


/*****************************************************************************
******************************************************************************
* Private memory declarations
******************************************************************************
*****************************************************************************/

#if gNvStorageIncluded_d
/* Table of dirty flags, one per dataset in NvDataSetDescriptionTable[]. */
static NvDataSetInfo_t maNvDirtyFlags[gNvNumberOfDataSets_c];

#if gNvMultiStorageIncluded_d
static  NvMultiSavedDataInfo_t maNvMultiSavedDataInfo[gNvNumberOfDataSets_c];
#endif

/* Minimum number of calls to NvTimerTick() between saves of a given dataset. */
NvSaveInterval_t gNvMinimumTicksBetweenSaves = gNvMinimumTicksBetweenSaves_c;
/* Minimum number of calls to NvSaveOnIdle() between saves of a given dataset. */
static NvSaveCounter_t gNvCountsBetweenSaves = gNvCountsBetweenSaves_c;

/* If this counter is != 0, do not save to NV Storage. */
static uint8_t mNvCriticalSectionFlag = 0;
#endif                                  /* gNvStorageIncluded_d */

/* Scratch data sets used by the internal unit test. */
#if gNvSelfTest_d

#ifdef  gNvNumberOfDataSets_c
#undef  gNvNumberOfDataSets_c
#define gNvNumberOfDataSets_c   3
#endif

/* Data set 1. */
static uint8_t NvTestDataArray1[] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
static char NvTestDataString1[] = "For months, we have been triumphantly retreating before a demoralized enemy who is advancing in utter disorder.";
static uint8_t NvTestDataArray2[] = {222, 173, 190, 239, 0, 222, 202, 251, 173, 0};

static NvDataItemDescription_t const gaNvDataSet01[] = {
  {(void *) NvTestDataArray1,  sizeof(NvTestDataArray1)},
  {(void *) NvTestDataString1, sizeof(NvTestDataString1)},
  {(void *) NvTestDataArray2,  sizeof(NvTestDataArray2)},
  {NULL, 0}
};

/* Data set 2. */
static uint8_t scratchValue01 = 0x37;
static NvDataItemDescription_t const gaNvDataSet02[] = {
  {(void *) &scratchValue01, sizeof(scratchValue01)},
  {NULL, 0}
};

/* Data set 3. Empty. Test this boundry case. */
static NvDataItemDescription_t const gaNvDataSet03[] = {
  {NULL, 0}
};

/* Table of data sets. Required by the NV storage module. */
/* If the NV storage module's self test flag is on, it will */
/* use it's own internal copy of this table, so it can't be */
/* defined here. */
#define gNvDataSet1ID_c     0x17
#define gNvDataSet2ID_c     0x99
#define gNvDataSet3ID_c     0x42

static NvDataSetDescription_t const NvDataSetTable[gNvNumberOfDataSets_c] = {
  { gNvDataSet1ID_c, gaNvDataSet01 },
  { gNvDataSet2ID_c, gaNvDataSet02 },
  { gNvDataSet3ID_c, gaNvDataSet03 }
};
#endif                                  /* gNvSelfTest_d */

extern bool_t gNVInit;

/*****************************************************************************
******************************************************************************
* Public functions
******************************************************************************
*****************************************************************************/

/* There may be operations that take place over extended times that must
 * be atomic from the point of view of NV Storage saves. These routines
 * increment/decrement a counter; when the counter is non-zero, no NV
 * saves will be done. Note that this does not affect NV restores.
 */

void NvClearCriticalSection(void) {
#if gNvStorageIncluded_d
  if(mNvCriticalSectionFlag)  /* dg - in case of set/clear mismatch */
    --mNvCriticalSectionFlag;
#endif /* gNvStorageIncluded_d */
}


void NvSetCriticalSection(void)
{
#if gNvStorageIncluded_d
  ++mNvCriticalSectionFlag;
#endif /* gNvStorageIncluded_d */
}


/****************************************************************************/

/* Called from the idle task to process save-on-next-idle and save-on-count. */
#if gNvStorageIncluded_d
void NvIdle(void)
{
  index_t   i;
  bool_t    bNvOperationPerformed = FALSE;

  if (mNvCriticalSectionFlag) {
    return;
  }
  for (i = 0; i < gNvNumberOfDataSets_c; ++i) 
  {
  #if gNvMultiStorageIncluded_d
    if( (maNvMultiSavedDataInfo[i].pSource != NULL ) && maNvMultiSavedDataInfo[i].sourceLength ) 
      {
      bNvOperationPerformed = TRUE;
      if( FALSE == NvInPageMultiSave(i) )
        {
        NvSaveOnIdle(NvDataSetTable[i].dataSetID) ;
        maNvMultiSavedDataInfo[i].pSource = NULL;
        maNvMultiSavedDataInfo[i].sourceLength = 0;
        }
      }
   #endif  
  
    if (!maNvDirtyFlags[i].countsToNextSave) 
    {
      NvSaveDataSet(i);
      bNvOperationPerformed = TRUE;
    }
  }
  
  /* Restore the receiver state, if it was closed in order to write information in Flash */
  if(bNvOperationPerformed)
    NvOperationEnd();
}/* NvIdle() */
#endif /* gNvStorageIncluded_d */

/****************************************************************************/

/* Return TRUE if the given data set is dirty. */
#if gNvStorageIncluded_d
bool_t NvIsDataSetDirty
  (
  NvDataSetID_t dataSetID
 )
{
  index_t dataSetIndex = NvDataSetIndexFromID(dataSetID);
  return (    maNvDirtyFlags[dataSetIndex].saveNextInterval
          || (maNvDirtyFlags[dataSetIndex].countsToNextSave != gNvCountsBetweenSaves_c));
}/* NvIsDataSetDirty() */
#endif /* gNvStorageIncluded_d */

/****************************************************************************/

/* Call this once, before calling any other NV function.
 *
 * Any page that does not contain a valid, recognized data set is erased, in
 * preparation for later reuse.
 *
 * It is unlikely, but possible, that there could be more than one copy of one
 * or more data sets in NV storage. Normally the page containing the older
 * version is erased after the new version is written, but a reset at just
 * the wrong time could prevent the erasure.
 *
 * Having more than one copy of any data set is 1) unnecessary, 2) uses space
 * that may be needed for future writes, and 3) would complicate other NV code.
 * This function scans NV storage for alternate versions of the same data set,
 * and if it finds one, erases the older copy (or copies).
 */
#if gNvStorageIncluded_d
void NvModuleInit
(
	void
)
{
#if gNvDebug_d
	NvDataItemDescription_t const *pDataItemDescriptions;
	index_t dataSetIndex;
	NvSize_t dataSetSize;
#endif
	index_t i;
	index_t pageIndex;
	struct seqAndID_tag {
		NvDataSetID_t id;
		index_t pageIndex;
		NvDataSetSequenceNumber_t sequenceNumber;
	} seqAndID[gNvNumberOfRawPages_c];
	NvStructuredPageHeader_t *pThisHeader;
	index_t validDataSetsFound = 0;
#if gNvDebug_d
	/* The CodeWarrior HCS08 compiler complains about constant == constant. */
	uint16_t NvMaxDataSetSize = gNvMaxDataSetSize_c;
#endif

	/* initialize flash layer */
	NvHalInit();

	/* No data set starts dirty. */
	for (i = 0; i < gNvNumberOfDataSets_c; ++i)
	{
		maNvDirtyFlags[i].countsToNextSave = gNvCountsBetweenSaves;
	}

	/* Scan for and erase duplicate copies of data sets. This code is very */
	/* slow (N^2), but it only runs once, and N is small. Data set IDs are */
	/* arbitrary values, so its necessary to search through the list. */

	/* Visit every raw page. If it isn't a valid data set, and it isn't clean, */
	/* erase it. */
	for (pageIndex = 0; pageIndex < gNvNumberOfRawPages_c; ++pageIndex)
	{
		if (!NvIsValidDataSet(pageIndex))
		{
			NvHalErasePage(maNvRawPageAddressTable[pageIndex],mNvFlashPageEraseCmd_c);
		}
		else
		{
			/* Add this one to the list of valid pages we've visited. */
			pThisHeader = &((NvStructuredPage_t *) maNvRawPageAddressTable[pageIndex])->header;
			seqAndID[validDataSetsFound].id             = pThisHeader->dataSetID;
			seqAndID[validDataSetsFound].pageIndex      = pageIndex;
			seqAndID[validDataSetsFound].sequenceNumber = pThisHeader->sequenceNumber;

			/* Does this page contain the same data set as a page we've already visited? */
			for (i = 0; i < validDataSetsFound; ++i)
			{
				if (seqAndID[i].id == pThisHeader->dataSetID)
				{
					/* Erase whichever copy has the older sequence number. */
					/* Mark the erased page so it's ID will never be == a valid page's ID. */
					if (seqAndID[i].sequenceNumber - pThisHeader->sequenceNumber == 1)
					{
						NvHalErasePage(maNvRawPageAddressTable[pageIndex], mNvFlashPageEraseCmd_c);
						seqAndID[validDataSetsFound].id = gNvInvalidDataSetID_c;
					}
					else
					{
						NvHalErasePage(maNvRawPageAddressTable[(seqAndID[i].pageIndex)],
														mNvFlashPageEraseCmd_c);
						seqAndID[i].id = gNvInvalidDataSetID_c;
					}
				}/* if (seqAndID[i].id == ... */
			}/* for (i = 0; i < ... */

			++validDataSetsFound;
		}/* if (!NvIsValidDataSet(... else */
	}/* for (pageIndex = 0; ... */

#if gNvDebug_d
	/* NVM_Interface.h defines the size of the client area of a page as an */
	/* unjustified immediate integer constant. Make sure that its correct. */
	/* Assert if it isn't. */
	if (NvMaxDataSetSize != MbrSizeof(NvStructuredPage_t, clientData))
	{
		for (;;)
		{ }
	}

	/* Verify that all of the data sets defined by the client code will */
	/* fit into the NV storage page size. Doing this once here simplifies */
	/* other code. */
	for (dataSetIndex = 0; dataSetIndex < gNvNumberOfDataSets_c; ++dataSetIndex)
	{
		pDataItemDescriptions = NvDataSetTable[dataSetIndex].pItemDescriptions;
		if (!pDataItemDescriptions)
		{
			continue;
		}
		dataSetSize = 0;

		while (pDataItemDescriptions->length)
		{
			dataSetSize += pDataItemDescriptions->length;
			++pDataItemDescriptions;
		}

		/* Assert if the data set is too big. */
		if (dataSetSize > MbrSizeof(NvStructuredPage_t, clientData))
		{
			for (;;)
			{ }
		}
	}
#endif   /* gNvDebug_d */

#if gNvSelfTest_d
	NvSelfTest();
#endif
}/* NvModuleInit() */
#endif /* gNvStorageIncluded_d */

/****************************************************************************/

/* Search for a specific data structure in NV storage, delimited by a known
 * string at the beginning and end of the struct, and a known structure length.
 * The length includes both strings. The "strings" contain arbitrary bytes;
 * they are not assumed to be null-terminated C strings.
 *
 * This function is only indended for use by by the early startup code (crt0
 * and PlatformInit), which needs to find a struct containing hardware
 * initialization values.
 *
 * At that point in the startup process, the stack is available, but no data
 * in RAM has been initialized yet. ONLY const data (in ROM) is dependable.
 * NvModuleInit() has not been called yet to initialize this module. Be very
 * careful about calling local functions.
 * NvHalInit() has not been called yet. Do not call HAL functions, directly
 * or indirectly.
 *
 * Return a pointer to the NV storage copy of the data if found.
 * Return NULL if not found.
 */


/****************************************************************************/

/* Copy the most recent version of a data set from NV storage to RAM. */
/* Note that the copy will succeed if a valid copy of the data set is found */
/* in NV storage, regardless of the state of the data set's dirty flag. */
/* Return TRUE if the copy is successful. */
#if gNvStorageIncluded_d
bool_t NvRestoreDataSet
  (
  NvDataSetID_t dataSetID
  )
{
  NvDataItemDescription_t const *pDataItemDescriptions;
  index_t dataSetIndex;
  index_t pageIndex;
  NvSize_t pageOffset;

  /* Find the data set description. */
  dataSetIndex = NvDataSetIndexFromID(dataSetID);

  /* Find the data set in NV storage. */
  pageIndex = NvFindDataSet(dataSetID);
  if (pageIndex == gNvInvalidPageIndex_c) {
    return FALSE;
  }

  pDataItemDescriptions = NvDataSetTable[dataSetIndex].pItemDescriptions;

  /* Start reading just after the page header. */
  pageOffset = sizeof(NvStructuredPageHeader_t);

  /* Copy data from the NV storage page to the destination data set. */
  while (pDataItemDescriptions->length) {
    NvHalRead(maNvRawPageAddressTable[pageIndex], pageOffset, pDataItemDescriptions->pointer, pDataItemDescriptions->length);
    pageOffset += pDataItemDescriptions->length;
    ++pDataItemDescriptions;
  }

  maNvDirtyFlags[dataSetIndex].saveNextInterval = FALSE;
  maNvDirtyFlags[dataSetIndex].countsToNextSave = gNvCountsBetweenSaves_c;

  return TRUE;
}/* NvRestoreDataSet() */
#endif /* gNvStorageIncluded_d */

/****************************************************************************/

/* Return TRUE if the copy is successful. */
#if (gNvStorageIncluded_d && gNvMultiStorageIncluded_d )
bool_t NvRestoreMultiSavedData
  (
                                                                                     
  NvDataSetID_t dataSetID , void * pDest , uint16_t destLength
  )
{
  NvDataItemDescription_t const *pDataItemDescriptions;
  index_t dataSetIndex;
  index_t pageIndex;
  NvSize_t pageOffset, dataOffset;
  uint8_t  *pPageAddress;
    
  dataSetIndex = NvDataSetIndexFromID(dataSetID);
  pageIndex = NvFindDataSet(dataSetID);
	if (pageIndex == gNvInvalidPageIndex_c)
	{
		return FALSE;
	}
  pPageAddress = maNvRawPageAddressTable[pageIndex];
  pDataItemDescriptions = NvDataSetTable[dataSetIndex].pItemDescriptions;
  pageOffset = sizeof(NvStructuredPageHeader_t);

  while (pDataItemDescriptions->length) {
    pageOffset += pDataItemDescriptions->length;
    ++pDataItemDescriptions;
  }
  dataOffset = 0;
 
 while(pageOffset + destLength + sizeof(msMarker_t) <= gNVPageMaxOffset_c ) 
 {
  if( pPageAddress[pageOffset] == 0xff ) 
    {
    break;
    }
  if(*((uint16_t*)(pPageAddress + pageOffset)) == 0x0f0f )
    {
    dataOffset = pageOffset;
    }
  pageOffset += destLength + sizeof(msMarker_t);
 }
 if(dataOffset) 
 {
  NvHalRead(pPageAddress, dataOffset + sizeof(msMarker_t) ,	(uint8_t*)pDest,destLength) ;
 }
 return TRUE;

  ////////////////////////////////////////////////

}/* NvRestoreMultiSavedData*/
#endif /* (gNvStorageIncluded_d && gNvMultiStorageIncluded_d )*/


/****************************************************************************/

/* Save the data set on the next call to NvIdle(). */
/* Use the save-on-count mechanism. */
void NvSaveOnIdle
(
	NvDataSetID_t dataSetID
)
{
#if !gNvStorageIncluded_d
(void)dataSetID;
#else
  maNvDirtyFlags[NvDataSetIndexFromID(dataSetID)].countsToNextSave = 0;
#endif /* gNvStorageIncluded_d */
}


/****************************************************************************/

void NvInPageMultiSaveOnIdle
(
	NvDataSetID_t dataSetID , void * pSource , uint16_t sourceLength
)
{
#if (gNvStorageIncluded_d && gNvMultiStorageIncluded_d )  
  index_t index;
  index = NvDataSetIndexFromID(dataSetID);
  maNvMultiSavedDataInfo[index].pSource = pSource;
  maNvMultiSavedDataInfo[index].sourceLength = sourceLength;  
#else 
  (void)dataSetID;
  (void)pSource;
  (void)sourceLength; 
  return; 
#endif
}


/****************************************************************************/
void NvSaveOnInterval
  (
  NvDataSetID_t dataSetID
 )
{
#if !gNvStorageIncluded_d
(void)dataSetID;
#else
  index_t dataSetIndex = NvDataSetIndexFromID(dataSetID);

  if (!maNvDirtyFlags[dataSetIndex].saveNextInterval)
	{
    maNvDirtyFlags[dataSetIndex].ticksToNextSave  = gNvMinimumTicksBetweenSaves_c;
    maNvDirtyFlags[dataSetIndex].saveNextInterval = TRUE;
    TS_SendEvent(gIdleTaskID, gIdleTaskNVIntervalEvent_c);
  }
#endif /* gNvStorageIncluded_d */
}/* NvSaveOnInterval() */


/****************************************************************************/

/* Decrement the counter. Once it reaches 0, the next call to NvIdle() will */
/* save the data set. */
void NvSaveOnCount
  (
  NvDataSetID_t dataSetID
 )
{
#if !gNvStorageIncluded_d
(void)dataSetID;
#else
  index_t dataSetIndex = NvDataSetIndexFromID(dataSetID);

  if (maNvDirtyFlags[dataSetIndex].countsToNextSave) {
    --maNvDirtyFlags[dataSetIndex].countsToNextSave;
  }
#endif /* gNvStorageIncluded_d */
}/* NvSaveOnCount() */

/****************************************************************************/

/* Set the timer used by NvSaveOnInterval(). Takes effect after the next */
/* save. */
#if gNvStorageIncluded_d
void NvSetMinimumTicksBetweenSaves
  (
  NvSaveInterval_t newInterval
 )
{
  gNvMinimumTicksBetweenSaves = newInterval;
}/* NvSetMinimumTicksBetweenSaves() */
#endif /* gNvStorageIncluded_d */

/****************************************************************************/

/* Set the counter trigger value used by NvSaveOnCount(). Takes effect */
/* after the next save. */
#if gNvStorageIncluded_d
void NvSetCountsBetweenSaves
  (
  NvSaveCounter_t newCounter
 )
{
  gNvCountsBetweenSaves = newCounter;
}/* NvSetCountsBetweenSaves() */
#endif /* gNvStorageIncluded_d */

/****************************************************************************/

/* Called from the idle task to process save-on-interval requests. */
/* Returns FALSE if the timer tick counters for all data sets have reached */
/* zero. In this case, the timer can be turned off. */
/* Returns TRUE if any of the data sets' timer tick counters have not yet */
/* counted down to zero. In this case, the timer should be active. */
#if gNvStorageIncluded_d
bool_t NvTimerTick(bool_t countTick)
{
  index_t i;
  bool_t fTicksLeft = FALSE;
  NvDataSetInfo_t *pDirtyFlags;

  if (countTick) {
    pDirtyFlags = maNvDirtyFlags;
    for (i = 0; i < gNvNumberOfDataSets_c; ++i) {
      
      if(pDirtyFlags->ticksToNextSave)
        --(pDirtyFlags->ticksToNextSave);
      if(pDirtyFlags->ticksToNextSave)
        fTicksLeft = TRUE;
      if (pDirtyFlags->saveNextInterval && !pDirtyFlags->ticksToNextSave)
      {
        if(!mNvCriticalSectionFlag)
        {
          NvSaveDataSet(i);
        }
        else
        {
          pDirtyFlags->countsToNextSave = 0;
        }
      }

      ++pDirtyFlags;
    }
  }

  return fTicksLeft;
}/* NvTimerTick() */
#endif /* gNvStorageIncluded_d */

/*****************************************************************************
******************************************************************************
* Private functions
******************************************************************************
*****************************************************************************/

/* Compare two data set sequence numbers. */
/* Return   -1 if left  < right */
/*           0 if left == right */
/*           1 if left  > right */
/* Data set sequence numbers are unsigned, and modulo a power of two. Thus, */
/* 0 > (unsigned) -1. */
/* #if gNvStorageIncluded_d */
/* int8_t NvCmpSequenceNumbers */
/*   (*/
/*   uint8_t left, */
/*   uint8_t right */
/*  ) */
/* { */
/*   if (left == right) { */
/*     return 0; */
/*   } */

/*   if ((left - right) & 0x80) { */
/*     return -1; */
/*   } */

/*   return 1; */
/* }                                       /\* NvCmpSequenceNumbers() *\/ */
/* #endif                                  /\* #if gNvStorageIncluded_d *\/ */

/****************************************************************************/

/* Given a data set ID, return it's index in the NvDataSetTable[].
 * Data set IDs are arbitrary values, so they have to be searched for.
 */
#if gNvStorageIncluded_d
static index_t NvDataSetIndexFromID
  (
  NvDataSetID_t dataSetID
 )
{
  index_t i;

  for (i = 0; i < gNvNumberOfDataSets_c; ++i) {
    if (NvDataSetTable[i].pItemDescriptions
         && (NvDataSetTable[i].dataSetID == dataSetID)) {
      return i;
    }
  }

  /* Can't find it. Must be a programming error in the caller. There's */
  /* no good way to handle errors, but at least this makes it obvious */
  /* during debugging. */
  for (;;) { }
}/* NvDataSetIndexFromID() */
#endif /* gNvStorageIncluded_d */

/****************************************************************************/

/* Search NV storage for a given data set. Return it's index if it is found */
/* and is valid. Return gNvInvalidPageIndex_c otherwise. */
#if gNvStorageIncluded_d
static index_t NvFindDataSet
  (
  NvDataSetID_t dataSetID
 )
{
  index_t i;

  for (i = 0; i < gNvNumberOfRawPages_c; ++i) {
    if (((NvStructuredPage_t *) maNvRawPageAddressTable[i])->header.dataSetID == dataSetID) {
      return i;
    }
  }

  return gNvInvalidPageIndex_c;
}/* NvFindDataSet() */
#endif                                  /* gNvStorageIncluded_d */

/****************************************************************************/

/* Examine an NV storage page. Check the magic number, check that the header
 * and trailer match, and check that the data set ID is recognized by the
 * application's table of data set descriptions.
 * Return TRUE if it looks legit; FALSE otherwise.
 */
#if gNvStorageIncluded_d
static bool_t NvIsValidDataSet
  (
  index_t pageIndex
 )
{
  bool_t legitDataSetID;
  index_t i;
  NvStructuredPage_t *pPage = ((NvStructuredPage_t *) maNvRawPageAddressTable[pageIndex]);

  /* Is the data set ID legit? */
  for (legitDataSetID = FALSE, i = 0; i < gNvNumberOfDataSets_c; ++i) {
    if (NvDataSetTable[i].dataSetID
         && (NvDataSetTable[i].dataSetID == pPage->header.dataSetID)) {
      legitDataSetID = TRUE;
      break;
    }
  }

  /* Do the header and trailer match? */
  /* Is the magic number legit? */
  if (legitDataSetID
       && FLib_MemCmp(&pPage->header, &pPage->trailer, sizeof(pPage->header))
       && (pPage->header.magicNumber == mNvMagicNumber_c)) {
    return TRUE;
  }

  return FALSE;
}                                       /* NvIsValidDataSet() */
#endif                                  /* gNvStorageIncluded_d */

/****************************************************************************/

/*
Save the contents of a data set, regardless of the state of the set's
 * dirty flag. Always writes a full page.
 *
 * Retry if the HAL reports an error. There isn't anything that the caller
 * can do about errors, so reporting one would be pointless.
 *
 * The flash controller reports errors that it detects, but power supply
 * fluctuations may still cause undetected errors. Its tempting to compute
 * some kind of checksum, and verify it after write, and also when a data
 * set is restored. This would complicate the code somewhat, and may not
 * be a common enough problem to be worth the extra code space.
 */
#if gNvStorageIncluded_d
static void NvSaveDataSet
  (
  index_t dataSetIndex
 )
{
  NvDataItemDescription_t const *pDataItemDescriptions;
  NvDataSetID_t dataSetID;

  index_t oldDataSetPageIndex;
  NvStructuredPageHeader_t pageHeader;
  index_t pageIndex;
  NvSize_t pageOffset;
  uint8_t retries;
  bool_t status;                        /* FALSE on HAL error. */
  unsigned char hexFF = 0xff;

  /* Search for an unused page. Almost as fast as keeping an array of */
  /* page status flags, and uses less RAM. */
	for (pageIndex = 0; pageIndex < gNvNumberOfRawPages_c; ++pageIndex)
	{
		if (!NvIsValidDataSet(pageIndex))
			break;
	}

  /* There should always be an available page. If there isn't, there is */
  /* either an internal error in this code or the application, or there's */
  /* hardware failure. Either way, there's nothing that can be done about */
  /* it except to ignore the attempted write. */
  if(pageIndex >= gNvNumberOfRawPages_c)
    return;

  dataSetID = NvDataSetTable[dataSetIndex].dataSetID;

  /* Set up the page header/trailer. */
  pageHeader.magicNumber = mNvMagicNumber_c;
  pageHeader.dataSetID = dataSetID;

  oldDataSetPageIndex = NvFindDataSet(dataSetID);
	if (oldDataSetPageIndex == gNvInvalidPageIndex_c)
	{
		pageHeader.sequenceNumber = 0;
	}
	else
	{
		pageHeader.sequenceNumber =
		((NvStructuredPage_t *) maNvRawPageAddressTable[oldDataSetPageIndex])->header.sequenceNumber + 1;
	}

  /* Keep trying until either it works, or it fails often enough that */
  /* it is unlikely to ever work. */
  NvOperationStart();
  retries = mNvFlashCmdRetries_c;
	while (retries--)
	{
		pDataItemDescriptions = NvDataSetTable[dataSetIndex].pItemDescriptions;
		pageOffset = 0;

		/* Write the page header to the beginning of the page. */
		status = NvHalWrite(maNvRawPageAddressTable[pageIndex],
												pageOffset,
												(unsigned char *) &pageHeader,
												sizeof(pageHeader));
		pageOffset += sizeof(pageHeader);

		/* Write the client data to the page. Note that the sizes of all of the
			data sets in NvDataSetTable[] were validated in NvModuleInit(), so it
			isn't necesary to check it here. */
		while (status && pDataItemDescriptions->length)
		{
			status = NvHalWrite(maNvRawPageAddressTable[pageIndex],
			pageOffset, pDataItemDescriptions->pointer, pDataItemDescriptions->length);
			pageOffset += pDataItemDescriptions->length;
			++pDataItemDescriptions;
		}

		/* Pad to the end of the page's data field. The HAL does not allow writing
			partial pages, but it doesn't check. */
		while (status && (pageOffset < sizeof(NvRawPage_t) - sizeof(pageHeader)))
		{
			status = NvHalWrite(maNvRawPageAddressTable[pageIndex],
													pageOffset,
													&hexFF,
													sizeof(hexFF));
			pageOffset += sizeof(hexFF);
		}

		/* Write the terminal copy of the header. If this works, we're done. */
		if (status && NvHalWrite(maNvRawPageAddressTable[pageIndex], pageOffset,
														(unsigned char *) &pageHeader, sizeof(pageHeader)))
		{
			break;
		}

		/* Something didn't work. Erase the page and try again. */
		NvHalErasePage(maNvRawPageAddressTable[pageIndex],mNvFlashPageEraseCmd_c);
	}/* while (retries--) */

	/* If it didn't work, don't update anything. */
	if (status)
	{
		/* The new copy is safe. Erase the old one. Rereading the data set to
			verify that it wrote correctly would be safer, but it would also be
			slower and would take more code. */
		if (oldDataSetPageIndex != gNvInvalidPageIndex_c)
		{
			NvHalErasePage(maNvRawPageAddressTable[oldDataSetPageIndex], mNvFlashPageEraseCmd_c);
		}
		maNvDirtyFlags[dataSetIndex].saveNextInterval = FALSE;
    maNvDirtyFlags[dataSetIndex].countsToNextSave = gNvCountsBetweenSaves_c;
	}/* if (status) */
}/* NvSaveDataSet() */
#endif                                  /* gNvStorageIncluded_d */


/****************************************************************************/

#if (gNvStorageIncluded_d && gNvMultiStorageIncluded_d )

static bool_t NvInPageMultiSave
  (
  index_t dataSetIndex
  )
{
  NvDataItemDescription_t const *pDataItemDescriptions;
  NvDataSetID_t dataSetID;
  uint8_t  *pPageAddress;
  msMarker_t msMarker;
  index_t pageIndex;
  NvSize_t pageOffset;
  
  dataSetID = NvDataSetTable[dataSetIndex].dataSetID;
  pageIndex = NvFindDataSet(dataSetID);
	if (pageIndex == gNvInvalidPageIndex_c)
	{
	//	return FALSE;
	// there is no saved page with this ID  
	for(;;){}
	  //  for debug purposes 
	}
  pPageAddress = maNvRawPageAddressTable[pageIndex];
  pDataItemDescriptions = NvDataSetTable[dataSetIndex].pItemDescriptions;
  pageOffset = sizeof(NvStructuredPageHeader_t);

  while (pDataItemDescriptions->length) {
    pageOffset += pDataItemDescriptions->length;
    ++pDataItemDescriptions;
  }

NvOperationStart();
for(;;)
{
  if ((pageOffset + maNvMultiSavedDataInfo[dataSetIndex].sourceLength + sizeof(msMarker_t)) > gNVPageMaxOffset_c ) 
    {
     return FALSE;
    }
  if(pPageAddress[pageOffset] == 0xff ) 
    {
    msMarker.header = 0xf;
    msMarker.trailer = 0xf;
    if(TRUE == NvHalUnbufferedWrite( pPageAddress , pageOffset , &msMarker.header , 1) )
      {
      if(TRUE == NvHalUnbufferedWrite( pPageAddress , pageOffset + sizeof(msMarker_t) , (uint8_t*)maNvMultiSavedDataInfo[dataSetIndex].pSource , maNvMultiSavedDataInfo[dataSetIndex].sourceLength )) 
        {
         if(TRUE == NvHalUnbufferedWrite( pPageAddress , pageOffset + sizeof(msMarker.header) , &msMarker.trailer , 1)) 
         {
          maNvMultiSavedDataInfo[dataSetIndex].pSource = NULL;
          maNvMultiSavedDataInfo[dataSetIndex].sourceLength = 0;
          
          return TRUE;
         }
        }
      } 
    else 
     {
      if(pPageAddress[pageOffset] == 0xff ) 
        {
         return FALSE;
        }
     }
      
    }
  pageOffset += maNvMultiSavedDataInfo[dataSetIndex].sourceLength + sizeof(msMarker_t);
 }
 

}/* NvInPageMultiSave */
#endif                                  /* (gNvStorageIncluded_d && gNvMultiStorageIncluded_d ) */


/****************************************************************************/

/* Unit test. This is a debugging aid; it isn't intended to test every */
/* possible case. It isn't even a good smoke test. It doesn't try to */
/* insure completely correct operation; the calls to NvSelfTestError() are */
/* only intended to provide convenient places to put debugger breakpoints. */

#if gNvSelfTest_d

uint8_t NvSelfTestErrors = 0;
static void NvSelfTestError(void)
{
  NvSelfTestErrors++;
}

static void NvSelfTest()
{
  uint8_t buf[16];
  index_t i;
  unsigned char const *p = NULL;

  /* Test valid page recognition for invalid pages. */
  for (i = 0; i < gNvNumberOfRawPages_c; ++i) {
    if (NvIsValidDataSet(i)) {
      NvSelfTestError();
    }
  }

  /* Write some data sets. */
  for (i = 0; i < gNvNumberOfDataSets_c; ++i) {
    NvSaveDataSet(NvDataSetTable[i].dataSetID);
  }


  /* Copy some data, change it, and restore the data set. */

  for (i = 0; i < sizeof(NvTestDataArray2); ++i) {
    buf[i] = NvTestDataArray2[i];
    NvTestDataArray2[i] = 0;
  }

  if (!NvRestoreDataSet(gNvDataSet1ID_c)) {
    NvSelfTestError();
  }

  for (i = 0; i < sizeof(NvTestDataArray2); ++i) {
    if (buf[i] != NvTestDataArray2[i]) {
      NvSelfTestError();
    }
  }

  /* Try writing a new version of each data set. */
  for (i = 0; i < gNvNumberOfDataSets_c; ++i) {
    NvSaveDataSet(NvDataSetTable[i].dataSetID);
  }
}                                       /* NvSelfTest() */
#endif                                  /* #if gNvSelfTest_d */
