#ifdef _MSC_VER
#pragma once
#endif

#ifndef WUVERSIONS_INCLUDED
#define WUVERSIONS_INCLUDED

#include "wutypes.h"
#include "wubuffers.h"

struct VersionsResourceDemand
{
	size_t clientStateSize;
	size_t bufferStateSize;
};

struct VersionsClientInfo
{
	TIMESTAMP snapshotTs;
	int clientId;
	int isUsingSnapshot;
	int isRecoveryAgent;
};

struct VersionsSnapshotInfo
{
	TIMESTAMP snapshotTs;	/* timestamp of the newer snapshot */ 
	TIMESTAMP replacedTs;	/* what snapshot is replaced */ 
};

struct VersionsCheckpointInfo
{
	TIMESTAMP persistentTs;	/* what snapshot is turned persistent */ 
};

struct VersionsCreateVersionParams
{
	TIMESTAMP creationTs;	/*	timestamp of the working version creation */ 
	LXPTR lxptr;			/*	Logical XPTR of the block */ 
	XPTR lastCommitedXptr;	/*	physical XPTR of the last commited version */ 
	size_t alsoUsageSize;
	int *alsoUsage;				
};

struct VersionsSetup
{
	TICKET clientStateTicket;
	TICKET bufferStateTicket; 

	/*	Buffer functions 
		- renameBuffer - changes an xptr associated with buffer identified
		by bufferId. An error code is returned if a buffer associated with
		xptr already exists and it's id is not bufferId. 
		- loadBuffer - puts block identified by xptr in buffer (flags=0). 
		- unloadBuffer - marks buffer as free and flushes if dirty (flags=0).
		- getBufferInfo - stores buffer info in user-provided structure.
		- getBufferStateBlock - get state block stored in internal
		buffer structures.
		- fixBuffer - prevents buffer from beeing ejected.
		- protectBuffer - affect memory protection applied to the buffer
		when it is mapped in TRN next time (debug version also changes protection
		emediately for all TRNs the block is mapped to). 
		- markBufferDirty - either marks buffer dirty or removes this mark. */ 
	int (*renameBuffer)(int bufferId, XPTR xptr);
	int (*loadBuffer)(XPTR xptr, int *bufferId, int flags);
	int (*unloadBuffer)(int bufferId, int flags);
	int (*getBufferInfo)(int bufferId, BufferInfo *bufferInfo);
	int (*getBufferStateBlock)(int bufferId, TICKET ticket, void **data);
	int (*fixBuffer)(int bufferId, int orMask, int andNotMask);
	int (*protectBuffer)(int bufferId, int orMask, int andNotMask);
	int (*markBufferDirty)(int bufferId, int from, int to, int flags);

	/*	Alloc functions 
		- allocBlock - allocate block.
		- freeBlock - frees block, currently unused. */ 
	int (*allocBlock)(XPTR *xptr, XPTR proto);
	int (*freeBlock)(XPTR xptr);

	/* Timestamp functions 
		- getTimestamp returns unique timestamp, subsequent call must return larger value */ 
	int (*getTimestamp)(TIMESTAMP *timestamp);

	/*	Callbacks 
		- onCompleteBlockRelocation - called emediately after block who is not included 
		in the latest snapshot relocates. Block is already written at xptr,
		however it's copy at lxptr still exists.
		May be called in context where clientId!=GetCurrentCLientId(). */ 
	int (*onCompleteBlockRelocation)(int clientId, LXPTR lxptr, XPTR xptr);
	int (*onCreateVersion)(VersionsCreateVersionParams *);
};

/*	Determine the space required to be reserved in other components internal
	structures. Reservation size is known at compile time but we don't want to
	expose structures definition. It helps to isolate components from
	each other and allows virtually unlimited composition possibilities. */ 
void VeQueryResourceDemand(VersionsResourceDemand *versionsResourceDemand);

/*	Performs initial Ve initialisation. */ 
int VeInitialise();

/*	Completes initialisation. Installs callbacks and other components functions
	passed via VersionsSetup. They are used to hook components
	together. These functions are never called during startup or deinitialise
	(due to possibly cyclic component interdependencies). */ 
int VeStartup(VersionsSetup *setup);

/*	Shut down Ve. */ 
void VeDeinitialise();

/*	Initialises private structures instance for the client. */ 
int VeOnRegisterClient(VersionsClientInfo *clientInfo);

/*	Deinitialises private structures instance for the client.
	Cleanup is limited to freeing resources but any actions
	required by the protocol (for instance things to be done
	on transaction commit or rollback) are not performed. */ 
int VeOnUnregisterClient(int clientId);

/*	Puts block to buffer. The debug version will ensure that
	the block identified by lxptr was ever actually allocated
	and report error otherwise. */ 
int VeLoadBuffer(LXPTR lxptr, int *bufferId);

/*	Allocate a block - either a new one or a copy of the
	block identified by proto. In the later case a new
	version is created and any further requests for proto will
	rather return a newer block. If proto is a special value
	a brand new block is created (either data or temp
	depending on the proto value). Unless client allocated
	the block it is readonly for him. If client is not permited
	to create a version function returns error status emediately
	since wait-for-version-slot-availible should be rather
	implemented in Locks. */ 
int VeAllocBlock(LXPTR proto, LXPTR *lxptr);

/*	Free a block. Operation failes unless the same client
	allocated this block earlier. */ 
int VeFreeBlock(LXPTR lxptr);

/*	Used when transaction rolls back or during recovery. */ 
int VeRevertBlock(VersionsCreateVersionParams *);

/*	Updates info about active snapshots. The info is required for
	proper version identification. */ 
int VeOnSnapshot(VersionsSnapshotInfo *snapshotInfo);

/*	Updates info about the persistent snapshot. The info is required for
	to determine whether given block have a version included in persistent
	snapshot or not. Additionally necesarry buffer flushes are performed. */ 
int VeOnCheckpoint(VersionsCheckpointInfo *checkpointInfo);

/*	Monitor buffers state. MUST be hooked into buffer manager. */ 
int VeOnFlushBlock(int bufferId);

#endif