#if (_MSC_VER > 1000)
#pragma once
#endif

#ifndef WUVERSIONS_H_INCLUDED
#define WUVERSIONS_H_INCLUDED

#include "wutypes.h"
#include "wubuffers.h"
#include "wusnapshots.h"
#include "wuincguard.h"

struct VeResourceDemand
{
	size_t clientStateSize;
	size_t bufferStateSize;
};

#define VE_SETUP_DISABLE_VERSIONS_FLAG				0x01
#define VE_SETUP_DISABLE_CORRECTNESS_CHECKS_FLAG	0x02

struct VeSetup
{
	/* params */ 
	int flags;

	/* tickets */ 
	TICKET clientStateTicket;
	TICKET bufferStateTicket; 

	/*	Buffer functions */ 
	int (*loadBuffer)(XPTR xptr, int *bufferId, int flags);
	int (*flushBuffer)(int bufferId, int flags);
	int (*getBufferInfo)(int bufferId, BufferInfo *bufferInfo);
	int (*getBufferStateBlock)(int bufferId, TICKET ticket, void **data);
	int (*fixBuffer)(int bufferId, int orMask, int andNotMask);
	int (*protectBuffer)(int bufferId, int orMask, int andNotMask);
	int (*markBufferDirty)(int bufferId, void *base, size_t size, int flags);

	/* copy data functions */ 
	int (*copyBlock)(XPTR dest, XPTR src, int flags);

	/*	Alloc functions */ 
	int (*allocBlock)(XPTR *xptr);
	int (*freeBlock)(XPTR xptr);

	/* Timestamp functions */ 
	int (*getTimestamp)(TIMESTAMP *timestamp);

	/* GC functions */ 
	int (*submitRequestForGc)(const SnRequestForGc *buf, size_t count);

	/* data layout functions */ 
	int (*locateHeader)(int bufferId, VersionsHeader **header);

	/*	Callbacks 
		- onCompleteBlockRelocation - called emediately after block who is not included 
		in the latest snapshot relocates. Block is already written at xptr,
		however it's copy at lxptr still exists.
		May be called in context where clientId!=GetCurrentCLientId(). */ 
	int (*onCompleteBlockRelocation)(int clientId, LXPTR lxptr, XPTR xptr);
};

int VeInitialize();

void VeDeinitialize();

void VeQueryResourceDemand(VeResourceDemand *resourceDemand);

int VeStartup(VeSetup *setup);

int VeOnRegisterClient();

int VeOnUnregisterClient();

int VeOnTransactionEnd(int how);

int VePutBlockToBuffer(LXPTR lxptr, int *bufferId, int flags);

int VeAllocateBlock(LXPTR *lxptr);

int VeCreateBlockVersion(LXPTR lxptr);

int VeFreeBlock(LXPTR lxptr);

/*	Do not call it unless you know what you are doing! */ 
int VeReallyFreeBlock(XPTR xptr);

void VeDbgDump(int reserved);

#endif
