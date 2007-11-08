#define __WUDANG_SOURCES__

#include "wuaux.h"
#include <assert.h>
#include "wu.h"
#include "wuerr.h"
#include "wuclients.h"
#include "wusnapshots.h"
#include "wuversions.h"
#include "wudock.h"

#include "common/base.h"
#include "sm/llmgr/llmgr.h"
#include "sm/bufmgr/bm_core.h"
#include "sm/bufmgr/blk_mngmt.h"
#include "common/sm_vmm_data.h"
#include "common/u/u.h"
#include "common/u/umutex.h"
#include "sm/bufmgr/bm_functions.h"
#include "sm/trmgr.h"
#include <windows.h>

#define WU_SWAPPED_XPTRS_COUNT	1

/* global variables */ 

static TIMESTAMP timestamp = INVALID_TIMESTAMP;
static uMutexType gMutex;
static int isSynchrObjectsInitialized = 0;

static XPTR swapped[WU_SWAPPED_XPTRS_COUNT] = {};
static size_t numSwapped = 0;

/* utility functions */ 

static
int InitSynchrObjects()
{
	int success = 0;
	if (isSynchrObjectsInitialized)
	{
		WuSetLastErrorMacro(WUERR_FUNCTION_INVALID_IN_THIS_STATE);
	}
	else if (uMutexInit(&gMutex,__sys_call_error)!=0) {}
	else
	{
		isSynchrObjectsInitialized=1;
		success=1;
	}
	return success;
}

static
int DeinitSynchrObjects()
{
	int success = 0;

	if (!isSynchrObjectsInitialized)
	{
		WuSetLastErrorMacro(WUERR_FUNCTION_INVALID_IN_THIS_STATE);
	}
	else 
	{
		if (uMutexDestroy(&gMutex,__sys_call_error)==0) success = 1;
		isSynchrObjectsInitialized=0;
	}
	return success;
}

static
int BufferIdFromRamoffs(ramoffs ofs)
{
	assert(ofs % PAGE_SIZE == 0);
	return ofs / PAGE_SIZE;
}

static
ramoffs RamoffsFromBufferId(int id)
{
	return id * PAGE_SIZE;
}

static
int LocateBlockHeader(int bufferId, vmm_sm_blk_hdr **header)
{
	int success = 0;
	
	assert(header); *header=NULL;
	if (bufferId<0)
	{
		WuSetLastErrorMacro(WUERR_BAD_PARAMS);
	}
	else
	{
		*header = (vmm_sm_blk_hdr*)OffsetPtr(buf_mem_addr, RamoffsFromBufferId(bufferId));
		success = 1;
	}
	return success;
}

static int
GetSwapCode()
{
	int code = -1;
	if (numSwapped < WU_SWAPPED_XPTRS_COUNT)
	{
		code = ClGetCurrentClientId(NULL);
	}
	return code;
}

static void
PushSwappedXptr(XPTR xptr)
{
	if (numSwapped < WU_SWAPPED_XPTRS_COUNT)
	{
		swapped[numSwapped++] = xptr;
	}
}

XPTR PopSwappedXptr()
{
	XPTR xptr=0;
	if (numSwapped > 0) xptr = swapped[--numSwapped];
	return xptr;
}

/* wiring functions */ 

static 
int PutBlockToBuffer (XPTR xptr, int *bufferId)
{
	int success = 0;
	XPTR swapped = 0;
	ramoffs ofs = 0;

	assert(bufferId); *bufferId = -1;
	try
	{
		swapped = 
			WuInternaliseXptr(put_block_to_buffer(GetSwapCode(), WuExternaliseXptr(xptr), &ofs, true));

		PushSwappedXptr(swapped);
		*bufferId = BufferIdFromRamoffs(ofs);
		success = 1;
	}
	WU_CATCH_EXCEPTIONS()

	return success;
}

static
int MarkBufferDirty(int bufferId)
{
	int success = 0;
	vmm_sm_blk_hdr *header = NULL;
	if (LocateBlockHeader(bufferId, &header))
	{
		header->is_changed = true;
		success = 1;
	}
	return success;
}

static
int GrantExclusiveAccessToBuffer(int bufferId)
{
	int success = 0;
	vmm_sm_blk_hdr *header = NULL;
	if (LocateBlockHeader(bufferId, &header))
	{
		header->trid_wr_access = ClGetCurrentClientId(NULL);
		success = 1;
	}
	return 1;
}

static
int AllocateDataBlock (XPTR *pXptr, int *pBufferId)
{
	int success = 0, bufferId = -1;
	XPTR bigXptr=0, bigSwapped=0;
	xptr lilXptr, lilSwapped;
	ramoffs ofs = 0;
	vmm_sm_blk_hdr *header = NULL;

	assert(pXptr && pBufferId); *pXptr=0; *pBufferId=-1;
	try
	{
		bm_allocate_data_block(GetSwapCode(), &lilXptr, &ofs, &lilSwapped);

		bigXptr = WuInternaliseXptr(lilXptr);
		bigSwapped = WuInternaliseXptr(lilSwapped);
		PushSwappedXptr(bigSwapped);
		bufferId = BufferIdFromRamoffs(ofs);
		success = 1;
	}
	WU_CATCH_EXCEPTIONS()
	*pBufferId = bufferId; *pXptr = bigXptr;

	return success;
}

static
int AllocateTempBlock (XPTR *pXptr, int *pBufferId)
{
	int success = 0, bufferId = -1;
	XPTR bigXptr=0, bigSwapped=0;
	xptr lilXptr, lilSwapped;
	ramoffs ofs = 0;
	vmm_sm_blk_hdr *header = NULL;

	assert(pXptr && pBufferId); *pXptr=0; *pBufferId=-1;
	try
	{
		bm_allocate_tmp_block(GetSwapCode(), &lilXptr, &ofs, &lilSwapped);

		bigXptr = WuInternaliseXptr(lilXptr);
		bigSwapped = WuInternaliseXptr(lilSwapped);
		PushSwappedXptr(bigSwapped);
		bufferId = BufferIdFromRamoffs(ofs);
		success = 1;
	}
	WU_CATCH_EXCEPTIONS()
	*pBufferId = bufferId; *pXptr = bigXptr;

	return success;
}

static
int AllocateDataBlockAndCopyData (XPTR *xptr, int *bufferId, int srcBufferId)
{
	int success = 0;
	vmm_sm_blk_hdr *srcHeader = NULL, *destHeader = NULL;

	assert(xptr && bufferId); *xptr=0; *bufferId=-1;

	if (!LocateBlockHeader(srcBufferId, &srcHeader))
	{
		/* error! */ 
	}
	else
	{
		/* we protect a source buffer from beeng victimized by get_free_buffer */ 
		buffer_on_stake = srcBufferId;
		if (AllocateDataBlock(xptr, bufferId) && LocateBlockHeader(*bufferId, &destHeader))
		{
			memcpy(destHeader, srcHeader, PAGE_SIZE);
			success = 1;
		}
		buffer_on_stake = -1;
	}
	return success;
}

static
int FreeBlock(XPTR bigXptr)
{
	int success=0;
	xptr lilXptr;
	lilXptr = WuExternaliseXptr(bigXptr);
	try
	{
		delete_data_block(lilXptr);
		success=1;
	}
	WU_CATCH_EXCEPTIONS();
	return success;
}

static
int LocateVersionsHeader(int bufferId, VersionsHeader **veHeader)
{
	int success = 0;
	vmm_sm_blk_hdr *blockHeader = NULL;

	assert(veHeader); *veHeader=NULL;
	if (LocateBlockHeader(bufferId, &blockHeader))
	{
		*veHeader = &blockHeader->versionsHeader;
		success = 1;
	}
	return success;
}

static
int OnCompleteBlockRelocation(int clientId, LXPTR lxptr, XPTR xptr)
{
	int success=0;
	WuVersionEntry versionEntry = {lxptr, xptr};
	try
	{
		LONG_LSN lsn=ll_add_pers_snapshot_block_info(&versionEntry);
		ll_logical_log_flush_lsn(lsn);	
		success=1;
	}
	WU_CATCH_EXCEPTIONS()
	return success;
}

static
int OnDiscardSnapshot(TIMESTAMP snapshotTs)
{
	int success=0;
	try
	{
		WuDbgDump(-1,0);
		PhOnSnapshotDelete(snapshotTs);
		success=1;
	}
	WU_CATCH_EXCEPTIONS()
	return success;
}

static
int OnBeforeDiscardSnapshot(TIMESTAMP snapshotTs, int *bDenyDiscarding)
{
	return 1;
}

/* public api */ 

int WuGetTimestamp(TIMESTAMP *ts)
{
	int success = 0;
	assert(ts); *ts=INVALID_TIMESTAMP;
	if (!IsValidTimestamp(timestamp))
	{
		WuSetLastErrorMacro(WUERR_MAX_TIMESTAMP_VALUE_EXCEEDED);
	}
	else
	{
		*ts=timestamp++;
		success = 1;
	}
	return success;
}

int WuSetTimestamp(TIMESTAMP ts)
{
	int success=0;
	if (!IsValidTimestamp(ts))
	{
		WuSetLastErrorMacro(WUERR_BAD_TIMESTAMP);
	}
	else
	{
		timestamp = ts;
	}
	return success;
}


int WuInit(int isRecoveryMode, int isVersionsDisabled, TIMESTAMP persSnapshotTs)
{
	int success = 0;
	ClSetup clSetup = {};
	VeSetup veSetup = {};
	SnSetup snSetup = {};

	/* static */ 
	clSetup.maxClientsCount = CHARISMA_MAX_TRNS_NUMBER;
	clSetup.maxSizePerClient = 0x10000;

	veSetup.allocateBlock = AllocateDataBlock;
	veSetup.allocateBlockAndCopyData = AllocateDataBlockAndCopyData;
	veSetup.freeBlock = FreeBlock;
	veSetup.grantExclusiveAccessToBuffer = GrantExclusiveAccessToBuffer;
	veSetup.locateVersionsHeader = LocateVersionsHeader;
	veSetup.markBufferDirty = MarkBufferDirty;
	veSetup.putBlockToBuffer = PutBlockToBuffer;

	snSetup.maxClientsCount = CHARISMA_MAX_TRNS_NUMBER;
	snSetup.freeBlock = VeFreeBlockLowAndUpdateRestrictions;
	snSetup.getTimestamp = WuGetTimestamp;
	snSetup.onBeforeDiscardSnapshot = OnBeforeDiscardSnapshot;
	snSetup.onDiscardSnapshot = OnDiscardSnapshot;

	/* param */ 
	snSetup.initialPersSnapshotTs = persSnapshotTs;
	if (isRecoveryMode || isVersionsDisabled)
	{
		veSetup.flags |= VE_SETUP_DISABLE_VERSIONS_FLAG;
		snSetup.flags |= SN_SETUP_DISABLE_VERSIONS_FLAG;
	}
	if (!DEBUGI)
	{
		veSetup.flags |= VE_SETUP_DISABLE_CORRECTNESS_CHECKS_FLAG;
	}

	if (!InitSynchrObjects() ||
		!ClInitialize() ||
		!SnInitialize() ||
		!VeInitialize())
	{
		/* error! */ 
	}
	else
	{
		VeResourceDemand veResourceDemand = {};
		SnResourceDemand snResourceDemand = {};

		VeQueryResourceDemand(&veResourceDemand);
		SnQueryResourceDemand(&snResourceDemand);

		if (!ClReserveStateBlocks(&veSetup.clientStateTicket, veResourceDemand.clientStateSize) ||
			!ClReserveStateBlocks(&snSetup.clientStateTicket, snResourceDemand.clientStateSize) ||
			!ClStartup(&clSetup) ||
			!VeStartup(&veSetup) ||
			!SnStartup(&snSetup))
		{
			/* error! */ 
		}
		else 
		{
			try
			{
				PhOnInitialSnapshotCreate(persSnapshotTs);
				success = 1;
			}
			WU_CATCH_EXCEPTIONS()
		}
	}

	if (!success)
	{
		VeDeinitialize();
		SnDeinitialize();
		ClDeinitialize();
		DeinitSynchrObjects();
	}
	return success;
}

int WuRelease()
{
	int failure=0;
	if (!SnShutdown() || !VeShutdown()) failure=1;
	SnDeinitialize();
	VeDeinitialize();
	ClDeinitialize();
	if (!DeinitSynchrObjects()) failure=1;
	return (failure==0);
}

int WuOnBeginCheckpoint()
{
	int success = 0;
	TIMESTAMP ts = INVALID_TIMESTAMP;
	if (uMutexLock(&gMutex,__sys_call_error)==0)
	{
		success = SnOnBeginCheckpoint(&ts);
		uMutexUnlock(&gMutex,__sys_call_error);
	}
	return success;
}

int WuOnCompleteCheckpoint()
{
	int success = 0;
	if (uMutexLock(&gMutex,__sys_call_error)==0)
	{
		success = SnOnCompleteCheckpoint();
	}
	return success;
}

struct WuEnumerateAdapter
{
	WuEnumerateVersionsParams *params;
	WuEnumerateVersionsProc enumProc;
};

static
int WuEnumerateHelperProc(WuEnumerateVersionsParams *params, 
						  WuVersionEntry *buf, 
						  size_t count, int isGarbage)
{
	int success=0;
	void *userData = NULL;
	WuEnumerateAdapter *adapter = (WuEnumerateAdapter *)params->userData;

	userData = adapter->params->userData;
	*(adapter->params) = *params;
	adapter->params->userData = userData;

	try
	{
		success = adapter->enumProc(adapter->params, buf, count, isGarbage);
	}
	WU_CATCH_EXCEPTIONS()

	return success;
}

int WuEnumerateVersionsForCheckpoint(WuEnumerateVersionsParams *params,
									 WuEnumerateVersionsProc enumProc)
{

	int success;
	WuEnumerateAdapter adapter = {params, enumProc};
	WuEnumerateVersionsParams params2 = {};

	assert(params);
	params2.userData = &adapter;
	
	if (uMutexLock(&gMutex,__sys_call_error)!=0) {}
	else
	{
		if (!SnEnumerateVersionsForCheckpoint(&params2,WuEnumerateHelperProc)) {}
		else
		{
			success = 1;
		}
		uMutexUnlock(&gMutex,__sys_call_error);
	}
	return success;
}

int WuAllocateDataBlock(int sid, xptr *p, ramoffs *offs, xptr *swapped)
{
	int success=0, bufferId=0;
	LXPTR lxptr;

	assert(p && offs && swapped);
	if (uMutexLock(&gMutex,__sys_call_error)!=0) {}
	else
	{
		if (!ClSetCurrentClientId(sid)) {}
		else
		{
			*p=XNULL;
			*offs=0;
			*swapped=XNULL;
			if (VeAllocateBlock(&lxptr,&bufferId))
			{
				*p=WuExternaliseXptr(lxptr);
				*offs=RamoffsFromBufferId(bufferId);
				success=1;
			}
			ClSetCurrentClientId(-1);
		}
		uMutexUnlock(&gMutex, __sys_call_error);
	}
	*swapped = WuExternaliseXptr(PopSwappedXptr());
	return success;
}

int WuAllocateTempBlock(int sid, xptr *p, ramoffs *offs, xptr *swapped)
{
	XPTR bigXptr = 0;
	int success = 0, bufferId = -1;

	if (uMutexLock(&gMutex,__sys_call_error)!=0) {}
	else
	{
		if (!ClSetCurrentClientId(sid)) {}
		else
		{
			if (AllocateTempBlock(&bigXptr, &bufferId) && GrantExclusiveAccessToBuffer(bufferId))
			{
				*p = WuExternaliseXptr(bigXptr);
				*offs = RamoffsFromBufferId(bufferId);
				success=1;
			}
			ClSetCurrentClientId(-1);
		}
		uMutexUnlock(&gMutex, __sys_call_error);
	}
	*swapped = WuExternaliseXptr(PopSwappedXptr());
	return success;
}

int WuCreateBlockVersion(int sid, xptr p, ramoffs *offs, xptr *swapped)
{
	LXPTR lxptr=0;
	int success=0, bufferId=0;

	assert(offs && swapped);
	*offs=0;
	*swapped=XNULL;
	if (uMutexLock(&gMutex,__sys_call_error)!=0) {}
	else
	{
		if (!ClSetCurrentClientId(sid)) {}
		else
		{
			if (IS_TMP_BLOCK(p))
			{
				WuSetLastErrorMacro(WUERR_VERSIONS_UNSUPPORTED_FOR_THIS_BLOCK_TYPE);
			}
			else
			{
				lxptr=WuInternaliseXptr(p);
				if (VeCreateBlockVersion(lxptr, &bufferId))
				{
					*offs=RamoffsFromBufferId(bufferId);
					success=1;
				}
			}
			ClSetCurrentClientId(-1);
		}
		uMutexUnlock(&gMutex, __sys_call_error);
	}
	*swapped = WuExternaliseXptr(PopSwappedXptr());
	return success;
}

int WuDeleteBlock(int sid, xptr p)
{
	LXPTR lxptr=0;
	int success=0, bufferId=0;

	if (uMutexLock(&gMutex,__sys_call_error)!=0) {}
	else
	{
		if (!ClSetCurrentClientId(sid)) {}
		else
		{
			if (IS_TMP_BLOCK(p))
			{
				try
				{
					bm_delete_block(sid,p);
					success=1;
				}
				WU_CATCH_EXCEPTIONS();
			}
			else
			{
				lxptr = WuInternaliseXptr(p);
				success=VeFreeBlock(lxptr);
			}
			ClSetCurrentClientId(-1);
		}
		uMutexUnlock(&gMutex, __sys_call_error);
	}
	return success;
}

int WuGetBlock(int sid, xptr p, ramoffs *offs, xptr *swapped)
{
	LXPTR lxptr=0;
	int success=0, bufferId=0;
	
	assert(offs && swapped);
	if (uMutexLock(&gMutex,__sys_call_error)!=0) {}
	else
	{
		if (!ClSetCurrentClientId(sid)) {}
		else
		{
			lxptr = WuInternaliseXptr(p);
			*offs=0;
			*swapped=XNULL;
			if (IS_DATA_BLOCK(p))
			{
				success = VePutBlockToBuffer(lxptr,&bufferId);
			}
			else
			{
				success = 
					(PutBlockToBuffer(lxptr, &bufferId) && GrantExclusiveAccessToBuffer(bufferId));
			}
					
			if (success) *offs=RamoffsFromBufferId(bufferId);
			ClSetCurrentClientId(-1);
		}
		uMutexUnlock(&gMutex, __sys_call_error);
	}
	*swapped = WuExternaliseXptr(PopSwappedXptr());
	return success;
}

int WuOnRegisterTransaction(int sid, int isUsingSnapshot, TIMESTAMP *snapshotTs, int *persHeapIndex)
{
	int success=0;

	assert(snapshotTs && persHeapIndex);
	
	if (uMutexLock(&gMutex,__sys_call_error)!=0) {}
	else
	{
		if (!ClRegisterClient(&sid,1)) {}
		else
		{
			if (!ClSetCurrentClientId(sid)) {}
			else
			{
				if (!SnOnRegisterClient(isUsingSnapshot)) {}
				else if (!SnGetTransactionSnapshotTs(snapshotTs)) {}
				else if (!VeOnRegisterClient()) {}
				else if (!ClMarkClientReady(sid)) {}
				else
				try
				{
					if (isUsingSnapshot)
					{
						*persHeapIndex=GetPhIndex(*snapshotTs);
					}
					else
					{
						*persHeapIndex=-1;
					}
					success=1;
				}
				WU_CATCH_EXCEPTIONS()
				ClSetCurrentClientId(-1);
			}
			if (!success) ClUnregisterClient(sid);			
		}
		uMutexUnlock(&gMutex, __sys_call_error);
	}
	return success;
}

int WuOnCommitTransaction(int sid)
{
	TIMESTAMP currentSnapshotTs = INVALID_TIMESTAMP;
	int success=0;

	if (uMutexLock(&gMutex,__sys_call_error)!=0) {}
	else
	{
		if (!ClSetCurrentClientId(sid)) {}
		else
		{
			success = (SnOnTransactionEnd(&currentSnapshotTs) &&
					   VeOnTransactionEnd(VE_COMMIT_TRANSACTION, currentSnapshotTs));
			ClSetCurrentClientId(-1);
		}
		uMutexUnlock(&gMutex, __sys_call_error);
	}
	return success;
}

int WuOnRollbackTransaction(int sid)
{
	TIMESTAMP currentSnapshotTs = INVALID_TIMESTAMP;
	int success=0;

	if (uMutexLock(&gMutex,__sys_call_error)!=0) {}
	else
	{
		if (!ClSetCurrentClientId(sid)) {}
		else
		{
			if (!VeOnTransactionEnd(VE_ROLLBACK_TRANSACTION, INVALID_TIMESTAMP) ||
				!SnOnTransactionEnd(&currentSnapshotTs)) {}
			else
			{
				success=1;
			}
			ClSetCurrentClientId(-1);
		}		
		uMutexUnlock(&gMutex, __sys_call_error);
	}
	return success;
}


int WuOnUnregisterTransaction(int sid)
{
	int success=0;

	if (uMutexLock(&gMutex,__sys_call_error)!=0) {}
	else
	{
		if (!ClSetCurrentClientId(sid)) {}
		else
		{
			if (!ClMarkClientLeaving(sid)) {}
			else if (!VeOnUnregisterClient()) {}
			else if (!SnOnUnregisterClient()) {}
			else
			{
				success=1;
			}
			WuDbgDump(-1,0);
			ClSetCurrentClientId(-1);
			success=ClUnregisterClient(sid);
		}
		uMutexUnlock(&gMutex, __sys_call_error);
	}
	return success;
}

int WuGatherSnapshotsStats(WuSnapshotStats *stats)
{
	int success=0;

	assert(stats);
	if (uMutexLock(&gMutex,__sys_call_error)!=0) {}
	else
	{
		success = SnGatherSnapshotStats(stats);
		uMutexUnlock(&gMutex, __sys_call_error);
	}
	return success;
}

int WuTryAdvanceSnapshots(int *bSuccess)
{
	TIMESTAMP discardedSnapshotTs = INVALID_TIMESTAMP, curSnapshotTs = INVALID_TIMESTAMP;
	int success=0;

	assert(bSuccess); *bSuccess=0;
	if (uMutexLock(&gMutex,__sys_call_error)!=0) {}
	else
	{
		if (!SnTryAdvanceSnapshots(&curSnapshotTs) || curSnapshotTs==INVALID_TIMESTAMP) 
		{
			success =  (WuGetLastError() == WUERR_MAX_NUMBER_OF_SNAPSHOTS_EXCEEDED);
		}
		else
		try
		{
			PhOnSnapshotCreate(curSnapshotTs);
			*bSuccess=1;
			success=1;
		}
		WU_CATCH_EXCEPTIONS()
		uMutexUnlock(&gMutex, __sys_call_error);
	}

	return success;
}

void WuDbgDump(int selector, int reserved)
{
#if (EL_DEBUG == 1)
		if (selector & WU_DBG_DUMP_CLIENTS) ClDbgDump(reserved);
		if (selector & WU_DBG_DUMP_VERSIONS) VeDbgDump(reserved);
		if (selector & WU_DBG_DUMP_SNAPSHOTS) SnDbgDump(reserved);
#endif
}

/* public api, exn adapters */ 

void WuInitExn(int isRecoveryMode, int isVersionsDisabled, TIMESTAMP persSnapshotTs)
{
	if (!WuInit(isRecoveryMode, isVersionsDisabled, persSnapshotTs)) WuThrowException();
}

void WuReleaseExn()
{
	if (!WuRelease()) WuThrowException();
}

void WuOnBeginCheckpointExn()
{
	if (!WuOnBeginCheckpoint()) WuThrowException();
}

void WuEnumerateVersionsForCheckpointExn(WuEnumerateVersionsParams *params,
										 int(*saveListsProc)(WuEnumerateVersionsParams *params, WuVersionEntry *buf, size_t count, int isGarbage))
{
	if (!WuEnumerateVersionsForCheckpoint(params,saveListsProc)) WuThrowException();
}

void WuOnCompleteCheckpointExn()
{
	if (!WuOnCompleteCheckpoint()) WuThrowException();
}

void WuAllocateDataBlockExn(int sid, xptr *p, ramoffs *offs, xptr *swapped)
{
	if (!WuAllocateDataBlock(sid, p, offs, swapped)) WuThrowException();
}

void WuAllocateTempBlockExn(int sid, xptr *p, ramoffs *offs, xptr *swapped)
{
	if (!WuAllocateTempBlock(sid, p, offs, swapped)) WuThrowException(); 
}

void WuCreateBlockVersionExn(int sid, xptr p, ramoffs *offs, xptr *swapped)
{
	if (!WuCreateBlockVersion(sid, p, offs, swapped)) WuThrowException();
}

void WuDeleteBlockExn(int sid, xptr p)
{
	if (!WuDeleteBlock(sid, p)) WuThrowException();
}

void WuGetBlockExn(int sid, xptr p, ramoffs *offs, xptr *swapped)
{
	if (!WuGetBlock(sid, p, offs, swapped)) WuThrowException();
}

void WuOnRegisterTransactionExn(int sid, int isUsingSnapshot, TIMESTAMP *snapshotTs, int *persHeapIndex)
{
	if (!WuOnRegisterTransaction(sid, isUsingSnapshot, snapshotTs, persHeapIndex)) WuThrowException();
}

void WuOnCommitTransactionExn(int sid)
{
	if (!WuOnCommitTransaction(sid)) WuThrowException();
}

void WuOnRollbackTransactionExn(int sid)
{
	if (!WuOnRollbackTransaction(sid)) WuThrowException();
}

void WuOnUnregisterTransactionExn(int sid)
{
	if (!WuOnUnregisterTransaction(sid)) WuThrowException();
}

void WuGatherSnapshotsStatsExn(WuSnapshotStats *stats)
{
	if (!WuGatherSnapshotsStats(stats)) WuThrowException();
}

bool WuTryAdvanceSnapshotsExn()
{
	int bSuccess = 0;
	if (!WuTryAdvanceSnapshots(&bSuccess)) WuThrowException();
	return bSuccess;
}
