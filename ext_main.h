#include "extension.h"

#ifndef EXT_MAIN_H
#define EXT_MAIN_H

#include <sys/mman.h>
#include <link.h>
#include <math.h>

#include "sdktools.h"

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
void DisableCacheCvars();
void PopulateHookPointers();

class Hooks
{
public:
	static uint32_t CallocHook(uint32_t nitems, uint32_t size);
	static uint32_t MallocHook(uint32_t size);
	static uint32_t ReallocHook(uint32_t old_ptr, uint32_t new_size);
	static uint32_t FreeHook(uint32_t ref_tofree);
	static uint32_t OperatorNewHook(uint32_t size);
	static uint32_t OperatorNewArrayHook(uint32_t size);
	static uint32_t DeleteOperatorHook(uint32_t ref_tofree);
	static uint32_t DeleteOperatorArrayHook(uint32_t ref_tofree);
	static uint32_t MemcpyHook(uint32_t dest, uint32_t src, uint32_t size);
	static uint32_t MemsetHook(uint32_t dest, uint32_t byte, uint32_t size);
	static uint32_t MemmoveHook(uint32_t dest, uint32_t src, uint32_t size);
	static uint32_t StrncpyHook(uint32_t dest, uint32_t src, uint32_t size);
	static uint32_t StrcpyHook(uint32_t dest, uint32_t src);
	static uint32_t FrameLockHook(uint32_t arg0);
	static uint32_t RestoreSystemPatch(uint32_t arg0);
	static uint32_t RestoreSystemPatchStart(uint32_t arg0);
	static uint32_t SaveHookDirectMalloc(uint32_t size);
	static uint32_t SaveHookDirectRealloc(uint32_t old_ptr, uint32_t new_size);
	static uint32_t EdtSystemHookFunc(uint32_t arg1);
	static uint32_t PreEdtLoad(uint32_t arg1, uint32_t arg2);
	static uint32_t SaveRestoreMemManage(uint32_t arg0, uint32_t arg1);
	static uint32_t RestoreOverride();
	static uint32_t TransitionArgUpdateHook(uint32_t arg0, uint32_t arg1);
	static uint32_t FixNullCrash(uint32_t arg0);
	static uint32_t TransitionEntsHook(uint32_t arg0, uint32_t arg1);
	static uint32_t HelicopterCrashFix(uint32_t arg0);
	static uint32_t TransitionEntityCreateCall(uint32_t arg1, uint32_t arg2);
	static uint32_t TransitionRestoreMain(uint32_t arg1, uint32_t arg2, uint32_t arg3, uint32_t arg4);
	static uint32_t VehicleRollermineCheck(uint32_t arg1);
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
	static uint32_t AiHintNpcCombinePatch(uint32_t arg0, uint32_t arg1);
	static uint32_t IsAllowChangelevel();
	static uint32_t EmptyCall();
	static uint32_t UnmountPaths(uint32_t arg0);
	static uint32_t PlayerloadSavedHook(uint32_t arg0, uint32_t arg1);
	static uint32_t TransitionFixTheSecond(uint32_t arg0);
	static uint32_t PatchAnotherPlayerAccessCrash(uint32_t arg0);
	static uint32_t SimulateEntitiesHook(uint8_t simulating);
	static uint32_t ServiceEventQueueHook();
	static uint32_t HookEntityDelete(uint32_t arg0);
	static uint32_t SaveOverride(uint32_t arg1);
	static uint32_t BarneyThinkHook(uint32_t arg0, uint32_t arg1, uint32_t arg2);
	static uint32_t ChkHandle(uint32_t arg0, uint32_t arg1);
	static uint32_t SavegameInternalFunction(uint32_t arg0);
	static uint32_t PlayerLoadHook(uint32_t arg0);
	static uint32_t PlayerSpawnHook(uint32_t arg0, uint32_t arg1, uint32_t arg2);
	static uint32_t LevelChangeSafeHook(uint32_t arg0);
	static uint32_t ReadExHook(uint32_t arg0, uint32_t arg1, uint32_t arg2, uint32_t arg3, uint32_t arg4);
	static uint32_t PhysSimEnt(uint32_t arg0);
	static uint32_t GivePlayerWeaponsHook(uint32_t arg0);
	static uint32_t FindEntityByHandle(uint32_t arg0, uint32_t arg1);
	static uint32_t FindEntityByClassnameHook(uint32_t arg0, uint32_t arg1, uint32_t arg2);
	static uint32_t CleanupDeleteListHook(uint32_t arg0);
	static uint32_t FindEntityByName(uint32_t arg0, uint32_t arg1, uint32_t arg2, uint32_t arg3, uint32_t arg4, uint32_t arg5, uint32_t arg6);
	static uint32_t HookInstaKill(uint32_t arg0);
	static uint32_t SV_FrameHook(uint32_t arg0);
	static uint32_t FixBaseEntityNullCrash(uint32_t arg0, uint32_t arg1, uint32_t arg2);
	static uint32_t PlayerDeathHook(uint32_t arg0);
	static uint32_t VPhysicsSetObjectPort(uint32_t arg0, uint32_t arg1);
	static uint32_t VPhysicsInitShadowHook(uint32_t arg0, uint32_t arg1, uint32_t arg2, uint32_t arg3);
	static uint32_t CollisionRulesChangedHook(uint32_t arg0);
	static uint32_t SetSolidFlagsHook(uint32_t arg0, uint32_t arg1);
	static uint32_t DropshipSimulationCrashFix(uint32_t arg0, uint32_t arg1, uint32_t arg2, uint32_t arg3);
	static uint32_t PhysicsTouchTriggersHook(uint32_t arg0, uint32_t arg1);
	static uint32_t VphysicsUpdateWarningHook(uint32_t arg0);
	static uint32_t Outland_07_Patch(uint32_t arg0, uint32_t arg1);
	static uint32_t SV_TriggerMovedHook(uint32_t arg0, uint32_t arg1);
	static uint32_t AssaultNpcFix(uint32_t arg0, uint32_t arg1);
	static uint32_t PackedStoreConstructorHook(uint32_t arg0, uint32_t arg1, uint32_t arg2, uint32_t arg3, uint32_t arg4);
	static uint32_t PackedStoreDestructorHook(uint32_t arg0);
	static uint32_t UTIL_RemoveHookFailsafe(uint32_t arg0);
	static uint32_t WeaponGetHook(uint32_t arg0);
	static uint32_t AcceptInputHook(uint32_t arg0, uint32_t arg1, uint32_t arg2, uint32_t arg3, uint32_t arg4, uint32_t arg5);
	static uint32_t BaseNPCHook(uint32_t arg0, uint32_t arg1);
	static uint32_t UpdateOnRemove(uint32_t arg0);
	static uint32_t UTIL_PrecacheOther_Hook(uint32_t arg0, uint32_t arg1);
	static uint32_t GetClientSteamIDHook(uint32_t arg0, uint32_t arg1);
	static uint32_t fix_wheels_hook(uint32_t arg0, uint32_t arg1, uint32_t arg2);
	static uint32_t SpotlightHook(uint32_t arg0, uint32_t arg1);
	static uint32_t WeaponBugbaitFixHook(uint32_t arg0, uint32_t arg1);
	static uint32_t StuckCrashFix(uint32_t arg0, uint32_t arg1);
	__attribute__((regparm(2))) static uint32_t WeirdCrashPleaseFix(uint32_t arg0, uint32_t arg1);
	static uint32_t CitizenNullCrashFix(uint32_t arg0);
	static uint32_t OldRefUpdateFixOne(uint32_t arg0);
	static uint32_t AutosaveLoadHook(uint32_t arg0, uint32_t arg1, uint32_t arg2);
	static uint32_t Outland_07_Patch_Two(uint32_t arg0, uint32_t arg1, uint32_t arg2, uint32_t arg3);
	static uint32_t SetGlobalState(uint32_t arg0, uint32_t arg1);
	static uint32_t ParseMapEntities(uint32_t arg0, uint32_t arg1, uint32_t arg2);
	static uint32_t PatchMissingCheck(uint32_t arg0, uint32_t arg1, uint32_t arg2);
	static uint32_t MainPlayerRestoreHook(uint32_t arg0, uint32_t arg1, uint32_t arg2);
	static uint32_t pSeqdescHook(uint32_t arg0, uint32_t arg1);
	static uint32_t FixExplodeInputCrash(uint32_t arg0);
	static uint32_t ManhackSpriteEntVerify(uint32_t arg0, uint32_t arg1);
	static uint32_t HelicopterBadDetected(uint32_t arg0);
	static uint32_t PropCombineBall(uint32_t arg0, uint32_t arg1, uint32_t arg2);
	static uint32_t TakeDamageHeliFix(uint32_t arg0, uint32_t arg1, uint32_t arg2, uint32_t arg3);
};

#endif