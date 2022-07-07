#include <sys/mman.h>
#include <link.h>

#define HOOK_MSG "Saved memory reference to leaked resources list: [%X]"
#define EXT_PREFIX "[BlackMesaUtils] "

typedef struct _Value {
	void* value;
	struct _Value* nextVal;
} Value;

typedef Value** ValueList;

enum MDLCacheFlush_t
{
	MDLCACHE_FLUSH_STUDIOHDR		= 0x01,
	MDLCACHE_FLUSH_STUDIOHWDATA		= 0x02,
	MDLCACHE_FLUSH_VCOLLIDE			= 0x04,
	MDLCACHE_FLUSH_ANIMBLOCK		= 0x08,
	MDLCACHE_FLUSH_VIRTUALMODEL		= 0x10,
	MDLCACHE_FLUSH_AUTOPLAY         = 0x20,
	MDLCACHE_FLUSH_VERTEXES         = 0x40,

	MDLCACHE_FLUSH_IGNORELOCK       = 0x80000000,
	MDLCACHE_FLUSH_ALL              = 0xFFFFFFFF
};

typedef uint32_t (*pZeroArgProt)();
typedef uint32_t (*pOneArgProt)(uint32_t);
typedef uint32_t (*pTwoArgProt)(uint32_t, uint32_t);
typedef uint32_t (*pThreeArgProt)(uint32_t, uint32_t, uint32_t);
typedef uint32_t (*pFourArgProt)(uint32_t, uint32_t, uint32_t, uint32_t);
typedef uint32_t (*pFiveArgProt)(uint32_t, uint32_t, uint32_t, uint32_t, uint32_t);
typedef uint32_t (*pSixArgProt)(uint32_t, uint32_t, uint32_t, uint32_t, uint32_t, uint32_t);
typedef uint32_t (*pSevenArgProt)(uint32_t, uint32_t, uint32_t, uint32_t, uint32_t, uint32_t, uint32_t);

ValueList AllocateValuesList();
Value* CreateNewValue(void* valueInput);
void DeleteAllValuesInList(ValueList list, bool free_val, bool lock_mutex);
bool IsInValuesList(ValueList list, void* searchVal, bool lock_mutex);
bool RemoveFromValuesList(ValueList list, void* searchVal, bool lock_mutex);
bool InsertToValuesList(ValueList list, Value* head, bool tail, bool duplicate_chk, bool lock_mutex);
int ValueListItems(ValueList list, bool lock_mutex);

void ApplySingleHooks();
void AllowWriteToMappedMemory();
void RestoreMemoryProtections();
void HookFunctionInSharedObject(uint32_t base_address, uint32_t size, void* target_pointer, void* hook_pointer);
void HookFunctionsWithC();
void HookFunctionsWithCpp();
void DisableCacheCvars();
void PopulateHookExclusionLists();
bool IsAddressExcluded(uint32_t base_address, uint32_t search_address);
uint32_t GetCBaseEntity(uint32_t EHandle);

uint32_t CallocHook(uint32_t nitems, uint32_t size);
uint32_t MallocHook(uint32_t size);
uint32_t ReallocHook(uint32_t old_ptr, uint32_t new_size);
uint32_t OperatorNewArrayHook(uint32_t size);


/**
 * vim: set ts=4 :
 * =============================================================================
 * SourceMod Sample Extension
 * Copyright (C) 2004-2008 AlliedModders LLC.  All rights reserved.
 * =============================================================================
 *
 * This program is free software; you can redistribute it and/or modify it under
 * the terms of the GNU General Public License, version 3.0, as published by the
 * Free Software Foundation.
 * 
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more
 * details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * As a special exception, AlliedModders LLC gives you permission to link the
 * code of this program (as well as its derivative works) to "Half-Life 2," the
 * "Source Engine," the "SourcePawn JIT," and any Game MODs that run on software
 * by the Valve Corporation.  You must obey the GNU General Public License in
 * all respects for all other code used.  Additionally, AlliedModders LLC grants
 * this exception to all derivative works.  AlliedModders LLC defines further
 * exceptions, found in LICENSE.txt (as of this writing, version JULY-31-2007),
 * or <http://www.sourcemod.net/license.php>.
 *
 * Version: $Id$
 */

#ifndef _INCLUDE_SOURCEMOD_EXTENSION_PROPER_H_
#define _INCLUDE_SOURCEMOD_EXTENSION_PROPER_H_

/**
 * @file extension.h
 * @brief Sample extension code header.
 */

#include "smsdk_ext.h"

class Hooks
{
public:
	static uint32_t EmptyCall();
	static uint32_t SpawnServerHook(uint32_t arg0, uint32_t arg1);
	static uint32_t CreateEntityByNameHook(uint32_t arg0, uint32_t arg1);
	static uint32_t GameFrameHook(uint32_t arg0);
	static uint32_t Util_RemoveHook(uint32_t arg0);
	static uint32_t HostChangelevelHook(uint32_t arg0, uint32_t arg1, uint32_t arg2);
	static uint32_t CleanupDeleteListHook();
	static uint32_t PhysSimEnt(uint32_t arg0);
	static uint32_t PhysCleanup();
	static uint32_t AcceptInputHook(uint32_t arg0, uint32_t arg1, uint32_t arg2, uint32_t arg3, uint32_t arg4, uint32_t arg5);
	static uint32_t CreateNoSpawnHook(uint32_t arg0, uint32_t arg1, uint32_t arg2, uint32_t arg3);
};

/**
 * @brief Sample implementation of the SDK Extension.
 * Note: Uncomment one of the pre-defined virtual functions in order to use it.
 */
class BmsUtils : public SDKExtension
{
public:
	void* getCppAddr(auto classAddr);

	/**
	 * @brief This is called after the initial loading sequence has been processed.
	 *
	 * @param error		Error message buffer.
	 * @param maxlen	Size of error message buffer.
	 * @param late		Whether or not the module was loaded after map load.
	 * @return			True to succeed loading, false to fail.
	 */
	virtual bool SDK_OnLoad(char *error, size_t maxlen, bool late);
	
	/**
	 * @brief This is called right before the extension is unloaded.
	 */
	//virtual void SDK_OnUnload();

	/**
	 * @brief This is called once all known extensions have been loaded.
	 * Note: It is is a good idea to add natives here, if any are provided.
	 */
	virtual void SDK_OnAllLoaded();

	/**
	 * @brief Called when the pause state is changed.
	 */
	//virtual void SDK_OnPauseChange(bool paused);

	/**
	 * @brief this is called when Core wants to know if your extension is working.
	 *
	 * @param error		Error message buffer.
	 * @param maxlen	Size of error message buffer.
	 * @return			True if working, false otherwise.
	 */
	//virtual bool QueryRunning(char *error, size_t maxlen);
public:
#if defined SMEXT_CONF_METAMOD
	/**
	 * @brief Called when Metamod is attached, before the extension version is called.
	 *
	 * @param error			Error buffer.
	 * @param maxlen		Maximum size of error buffer.
	 * @param late			Whether or not Metamod considers this a late load.
	 * @return				True to succeed, false to fail.
	 */
	//virtual bool SDK_OnMetamodLoad(ISmmAPI *ismm, char *error, size_t maxlen, bool late);

	/**
	 * @brief Called when Metamod is detaching, after the extension version is called.
	 * NOTE: By default this is blocked unless sent from SourceMod.
	 *
	 * @param error			Error buffer.
	 * @param maxlen		Maximum size of error buffer.
	 * @return				True to succeed, false to fail.
	 */
	//virtual bool SDK_OnMetamodUnload(char *error, size_t maxlen);

	/**
	 * @brief Called when Metamod's pause state is changing.
	 * NOTE: By default this is blocked unless sent from SourceMod.
	 *
	 * @param paused		Pause state being set.
	 * @param error			Error buffer.
	 * @param maxlen		Maximum size of error buffer.
	 * @return				True to succeed, false to fail.
	 */
	//virtual bool SDK_OnMetamodPauseChange(bool paused, char *error, size_t maxlen);
#endif
};

#endif // _INCLUDE_SOURCEMOD_EXTENSION_PROPER_H_