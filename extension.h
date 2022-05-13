#include <sys/mman.h>
#include <link.h>

#include "sdktools.h"

#define HOOK_MSG "Saved memory reference to leaked resources list: [%X]"
#define EXT_PREFIX "[SynergyUtils] "

typedef struct _Value {
	void* value;
	struct _Value* nextVal;
} Value;

typedef Value** ValueList;

typedef struct _MallocRef {
	void* ref;
	void* ref_size;
	void* alloc_location;
	void* alloc_type;
	struct _MallocRef* nextRef;
} MallocRef;

typedef MallocRef** MallocRefList;

typedef struct _Field {
	void* label;
	void* key;
	void* type;
	void* flags;
	void* offset;
	ValueList fieldVals;
	struct _Field* nextField;
} Field;

typedef Field** FieldList;

typedef struct _SavedEntity {
	void* refHandle;
	void* clsname;
	FieldList fieldData;
	struct _SavedEntity* nextEnt;
} SavedEntity;

typedef SavedEntity** SaveData;

typedef struct _PlayerSave {
	SavedEntity* saved_player;
	struct _PlayerSave* nextPlayer;
} PlayerSave;

typedef PlayerSave** PlayerSaveList;

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

uint32_t GetFileSize(char* file_name);
uint32_t GetCBaseEntity(uint32_t EHandle);
uint32_t GetEntityField(uint32_t dmap, uint32_t firstEnt, uint32_t subdmap_offset, uint32_t deep, uint32_t searchField);
FieldList SaveEntityFields(uint32_t dmap, uint32_t firstEnt, uint32_t subdmap_offset, uint32_t deep, FieldList fieldList);
void GivePlayerWeapons(uint32_t player_object, bool force_give);
void SendEntityInput(uint32_t ref_handle, uint32_t inputName, uint32_t activator, uint32_t caller, uint32_t val, uint32_t outputId);
uint32_t SaveGameSafe(bool use_internal_savename);
void SaveLinkedList(ValueList leakList);
void RestoreLinkedLists();
void DestroyLinkedLists();
void SaveProcessId();
int ReleaseLeakedMemory(ValueList leakList, bool destroy, uint32_t current_cap, uint32_t allowed_cap, uint32_t free_perc);
void ReleasePlayerSavedList();
SavedEntity* SaveEntity(uint32_t firstEnt);
void SavePlayers();
void DisablePlayerViewControl();
void EnablePlayerViewControl();
void CleanPlayerEnts(bool no_parent);
void RestorePlayers();
void PatchRestoring();
void AllowWriteToMappedMemory();
void RestoreMemoryProtections();
void HookFunctionInSharedObject(uint32_t base_address, uint32_t size, void* target_pointer, void* hook_pointer);
void PatchAutosave();
void PatchRestore();
void HookVpkSystem();
void PatchVpkSystem();
void HookSaveRestoreOne();
void HookSaveRestoreTwo();
void HookSaveRestoreThree();
void HookSavingOne();
void HookSavingTwo();
void HookEdtSystem();
void PatchEdtSystem();
void PatchDropships();
void HookSpawnServer();
void HookHostChangelevel();
void PatchOthers();
void HookFunctionsWithC();
void HookFunctionsWithCpp();
void SimulateEntities(bool simulating);

ValueList AllocateValuesList();
FieldList AllocateFieldList();
MallocRefList AllocateMallocRefList();
PlayerSaveList AllocatePlayerSaveList();

SavedEntity* CreateNewSavedEntity(void* entRefHandleInput, void* classnameInput, FieldList fieldListInput);
Field* CreateNewField(void* labelInput, void* keyInput, void* typeInput, void* flagsInput, void* offsetInput, ValueList valuesInput);
Value* CreateNewValue(void* valueInput);
PlayerSave* CreateNewPlayerSave(SavedEntity* player_save_input);
MallocRef* CreateNewMallocRef(void* ref_input, void* size_input, void* alloc_location_input, void* alloc_type_input);

void InsertFieldToFieldList(FieldList list, Field* head);
void DeleteAllValuesInList(ValueList list, bool free_val, bool lock_mutex);
void DeleteAllValuesInMallocRefList(MallocRefList list, bool lock_mutex);
bool IsInValuesList(ValueList list, void* searchVal, bool lock_mutex);
bool RemoveFromValuesList(ValueList list, void* searchVal, bool lock_mutex);
void InsertToValuesList(ValueList list, Value* head, bool tail, bool duplicate_chk, bool lock_mutex);
void InsertToPlayerSaveList(PlayerSaveList list, PlayerSave* head);
bool IsInMallocRefList(MallocRefList list, void* searchVal, bool lock_mutex);
uint32_t RemoveAllocationRef(MallocRefList list, void* searchVal, bool lock_mutex);
MallocRef* SearchForMallocRef(MallocRefList list, void* searchVal, bool lock_mutex);
MallocRef* SearchForMallocRefInRange(MallocRefList list, void* searchVal, bool lock_mutex);
int MallocRefListSize(MallocRefList list, bool lock_mutex);
int ValueListItems(ValueList list, bool lock_mutex);
void InsertToMallocRefList(MallocRefList list, MallocRef* head, bool lock_mutex);

uint32_t CallocHook(uint32_t nitems, uint32_t size);
uint32_t MallocHook(uint32_t size);
uint32_t ReallocHook(uint32_t old_ptr, uint32_t new_size);
uint32_t FreeHook(uint32_t ref_tofree);
uint32_t OperatorNewHook(uint32_t size);
uint32_t OperatorNewArrayHook(uint32_t size);
uint32_t DeleteOperatorHook(uint32_t ref_tofree);
uint32_t DeleteOperatorArrayHook(uint32_t ref_tofree);
uint32_t MemcpyHook(uint32_t dest, uint32_t src, uint32_t size);
uint32_t MemsetHook(uint32_t dest, uint32_t byte, uint32_t size);
uint32_t MemmoveHook(uint32_t dest, uint32_t src, uint32_t size);
uint32_t StrncpyHook(uint32_t dest, uint32_t src, uint32_t size);
uint32_t StrcpyHook(uint32_t dest, uint32_t src);
uint32_t FrameLockHook(uint32_t arg0);
uint32_t RestoreSystemPatch(uint32_t arg0);
uint32_t RestoreSystemPatchStart(uint32_t arg0);
uint32_t SaveHookDirectMalloc(uint32_t size);
uint32_t SaveHookDirectRealloc(uint32_t old_ptr, uint32_t new_size);
uint32_t VpkReloadHook(uint32_t arg1);
uint32_t EdtSystemHookFunc(uint32_t arg1);
uint32_t PreEdtLoad(uint32_t arg1, uint32_t arg2);
uint32_t SaveRestoreMemManage();
uint32_t RestoreOverride();
uint32_t TransitionArgUpdateHook(uint32_t arg0, uint32_t arg1);
uint32_t TransitionArgUpdateHookTwo(uint32_t arg0, uint32_t arg1, uint32_t arg2);
uint32_t TransitionArgUpdateHookThree();
uint32_t FixNullCrash(uint32_t arg0);
uint32_t TransitionEntsHook(uint32_t arg0, uint32_t arg1);
uint32_t MainTransitionRestore(uint32_t arg1, uint32_t arg2);
uint32_t TransitionEntityCreateCall(uint32_t arg1, uint32_t arg2);
uint32_t TransitionRestoreMain(uint32_t arg1, uint32_t arg2, uint32_t arg3, uint32_t arg4);
uint32_t VehicleRollermineCheck(uint32_t arg1);
uint32_t SV_TriggerMovedFix(uint32_t arg1, uint32_t arg2);
uint32_t DoorCycleResolve(uint32_t arg1, uint32_t arg2, uint32_t arg3, uint32_t arg4, uint32_t arg5);
uint32_t CreateEntityByNameHook(uint32_t arg0, uint32_t arg1);
uint32_t FixManhackCrash(uint32_t arg0);
uint32_t FixTransitionCrash(uint32_t arg0, uint32_t arg1, uint32_t arg2);
uint32_t PlayerSpawnDirectHook(uint32_t arg0);
uint32_t SavegameInitialLoad(uint32_t arg0, uint32_t arg1);
uint32_t SpawnServerHookFunc(uint32_t arg1, uint32_t arg2, uint32_t arg3);
uint32_t HostChangelevelHook(uint32_t arg1, uint32_t arg2, uint32_t arg3);
uint32_t DropshipsHook(uint32_t arg1, uint32_t arg2, uint32_t arg3);
uint32_t CallLater(uint32_t arg1, uint32_t arg2, uint32_t arg3);
uint32_t DirectMallocHookDedicatedSrv(uint32_t arg0);


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
		static uint32_t UnmountPaths(uint32_t arg0);
		static uint32_t PlayerloadSavedHook(uint32_t arg0, uint32_t arg1);
		static uint32_t LevelInitHook(uint32_t arg0, uint32_t arg1, uint32_t arg2);
		static uint32_t TransitionFixTheSecond(uint32_t arg0);
		static uint32_t PatchAnotherPlayerAccessCrash(uint32_t arg0);
		static uint32_t PlayerSpawnDirectHook(uint32_t arg0);
		static uint32_t GameFrameHook(uint8_t simulating);
		static uint32_t HookEntityDelete(uint32_t arg0);
		static uint32_t SaveOverride(uint32_t arg1);
		static uint32_t NET_BufferToBufferDecompress_Patch(uint32_t arg0, uint32_t arg1, uint32_t arg2, uint32_t arg3);
		static uint32_t HunterCrashFix(uint32_t arg0);
		static uint32_t BarneyThinkHook(uint32_t arg0, uint32_t arg1, uint32_t arg2);
		static uint32_t ChkHandle(uint32_t arg0, uint32_t arg1);
		static uint32_t SavegameInternalFunction(uint32_t arg0);
		static uint32_t PlayerLoadHook(uint32_t arg0);
		static uint32_t PlayerSpawnHook(uint32_t arg0, uint32_t arg1, uint32_t arg2);
		static uint32_t LevelChangeSafeHook(uint32_t arg0);
};

/**
 * @brief Sample implementation of the SDK Extension.
 * Note: Uncomment one of the pre-defined virtual functions in order to use it.
 */
class SynergyUtils : public SDKExtension
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