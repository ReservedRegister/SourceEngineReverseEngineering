#ifndef EXT_MAIN_H
#define EXT_MAIN_H

void InitExtension();
void ApplyPatches();
void PatchRestore();
void HookVpkSystem();
void HookSaveRestoreOne();
void HookSaveRestoreTwo();
void HookSaveRestoreThree();
void HookSavingOne();
void HookSavingTwo();
void HookEdtSystem();
void HookSpawnServer();
void HookHostChangelevel();
void PatchOthers();
void HookFunctions();
void PopulateHookPointers();

class Hooks
{
public:
	static uint32_t CallocHook(uint32_t nitems, uint32_t size);
	static uint32_t MallocHookSmall(uint32_t size);
	static uint32_t MallocHookLarge(uint32_t size);
	static uint32_t ReallocHook(uint32_t old_ptr, uint32_t new_size);
	static uint32_t OperatorNewHook(uint32_t size);
	static uint32_t OperatorNewArrayHook(uint32_t size);
	static uint32_t SaveHookDirectMalloc(uint32_t size);
	static uint32_t SaveHookDirectRealloc(uint32_t old_ptr, uint32_t new_size);
	static uint32_t EdtSystemHookFunc(uint32_t arg1);
	static uint32_t PreEdtLoad(uint32_t arg1, uint32_t arg2);
	static uint32_t SaveRestoreMemManage(uint32_t arg0, uint32_t arg1);
	static uint32_t RestoreOverride();
	static uint32_t TransitionEntityCreateCall(uint32_t arg1, uint32_t arg2);
	static uint32_t TransitionRestoreMain(uint32_t arg1, uint32_t arg2, uint32_t arg3, uint32_t arg4);
	static uint32_t SV_TriggerMovedFix(uint32_t arg1, uint32_t arg2);
	static uint32_t DoorCycleResolve(uint32_t arg1, uint32_t arg2, uint32_t arg3, uint32_t arg4, uint32_t arg5);
	static uint32_t CreateEntityByNameHook(uint32_t arg0, uint32_t arg1);
	static uint32_t PlayerSpawnDirectHook(uint32_t arg0);
	static uint32_t SpawnServerHookFunc(uint32_t arg1, uint32_t arg2, uint32_t arg3);
	static uint32_t HostChangelevelHook(uint32_t arg1, uint32_t arg2, uint32_t arg3);
	static uint32_t DropshipSpawnHook(uint32_t arg0);
	static uint32_t LookupPoseParameterDropshipHook(uint32_t dropship_object, uint32_t arg1, uint32_t arg2);
	static uint32_t DirectMallocHookDedicatedSrv(uint32_t arg0);
	static uint32_t memcpyNetworkHook(uint32_t dest, uint32_t src, uint32_t size);
	static uint32_t LevelChangedHookFrameSnaps(uint32_t arg0);
	static uint32_t EmptyCall();
	static uint32_t PlayerloadSavedHook(uint32_t arg0, uint32_t arg1);
	static uint32_t SimulateEntitiesHook(uint8_t simulating);
	static uint32_t ServiceEventQueueHook();
	static uint32_t HookEntityDelete(uint32_t arg0);
	static uint32_t SaveOverride(uint32_t arg1);
	static uint32_t SavegameInternalFunction(uint32_t arg0);
	static uint32_t PlayerSpawnHook(uint32_t arg0, uint32_t arg1, uint32_t arg2);
	static uint32_t LevelChangeSafeHook(uint32_t arg0);
	static uint32_t PhysSimEnt(uint32_t arg0);
	static uint32_t FindEntityByHandle(uint32_t arg0, uint32_t arg1);
	static uint32_t FindEntityByClassnameHook(uint32_t arg0, uint32_t arg1, uint32_t arg2);
	static uint32_t CleanupDeleteListHook(uint32_t arg0);
	static uint32_t FindEntityByName(uint32_t arg0, uint32_t arg1, uint32_t arg2, uint32_t arg3, uint32_t arg4, uint32_t arg5, uint32_t arg6);
	static uint32_t HookInstaKill(uint32_t arg0);
	static uint32_t SV_FrameHook(uint32_t arg0);
	static uint32_t PlayerDeathHook(uint32_t arg0);
	static uint32_t SetSolidFlagsHook(uint32_t arg0, uint32_t arg1);
	static uint32_t PackedStoreConstructorHook(uint32_t arg0, uint32_t arg1, uint32_t arg2, uint32_t arg3, uint32_t arg4);
	static uint32_t PackedStoreDestructorHook(uint32_t arg0);
	static uint32_t UTIL_RemoveHookFailsafe(uint32_t arg0);
	static uint32_t AcceptInputHook(uint32_t arg0, uint32_t arg1, uint32_t arg2, uint32_t arg3, uint32_t arg4, uint32_t arg5);
	static uint32_t UpdateOnRemove(uint32_t arg0);
	static uint32_t UTIL_PrecacheOther_Hook(uint32_t arg0, uint32_t arg1);
	static uint32_t GetClientSteamIDHook(uint32_t arg0, uint32_t arg1);
	static uint32_t fix_wheels_hook(uint32_t arg0, uint32_t arg1, uint32_t arg2);
	static uint32_t AutosaveLoadHook(uint32_t arg0, uint32_t arg1, uint32_t arg2);
	static uint32_t SetGlobalState(uint32_t arg0, uint32_t arg1);
	static uint32_t ParseMapEntities(uint32_t arg0, uint32_t arg1, uint32_t arg2);
	static uint32_t MainPlayerRestoreHook(uint32_t arg0, uint32_t arg1, uint32_t arg2);
	static uint32_t RepairPlayerRestore(uint32_t arg0, uint32_t arg1, uint32_t arg2);
	static uint32_t MainSaveEntitiesFunc(uint32_t arg0, uint32_t arg1);
};

#endif