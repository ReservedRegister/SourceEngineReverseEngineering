#include "extension.h"

#ifndef EXT_MAIN_H
#define EXT_MAIN_H

void InitExtension();
void ApplyPatches();
void HookFunctions();
void DisableCacheCvars();

class Hooks
{
public:
	static uint32_t EmptyCall();
	static uint32_t SpawnServerHook(uint32_t arg0, uint32_t arg1);
	static uint32_t CreateEntityByNameHook(uint32_t arg0, uint32_t arg1);
	static uint32_t SimulateEntitiesHook(uint32_t arg0);
	static uint32_t ServiceEventQueueHook();
	static uint32_t UTIL_RemoveHook(uint32_t arg0);
	static uint32_t HostChangelevelHook(uint32_t arg0, uint32_t arg1, uint32_t arg2);
	static uint32_t CleanupDeleteListHook(uint32_t arg0);
	static uint32_t PhysSimEnt(uint32_t arg0);
	static uint32_t HookInstaKill(uint32_t arg0);
	static uint32_t SV_FrameHook(uint32_t arg0);
	static uint32_t TakeDamageHook(uint32_t arg0, uint32_t arg1);
	static uint32_t IRelationTypeHook(uint32_t arg0, uint32_t arg1);
    static uint32_t CallocHook(uint32_t nitems, uint32_t size);
    static uint32_t MallocHook(uint32_t size);
    static uint32_t ReallocHook(uint32_t old_ptr, uint32_t new_size);
    static uint32_t OperatorNewArrayHook(uint32_t size);
	static uint32_t UTIL_GetLocalPlayerHook();
	static uint32_t AcceptInputHook(uint32_t arg0, uint32_t arg1, uint32_t arg2, uint32_t arg3, uint32_t arg4, uint32_t arg5);
	static uint32_t VphysicsUpdateWarningHook(uint32_t arg0);
	static uint32_t HookFinalDeleteCall(uint32_t arg0);
	static uint32_t CPropHevCharger_ShouldApplyEffect(uint32_t arg0, uint32_t arg1);
	static uint32_t CPropRadiationCharger_ShouldApplyEffect(uint32_t arg0, uint32_t arg1);
	static uint32_t ScriptThinkEntCheck(uint32_t arg0);
	static uint32_t UpdateOnRemove(uint32_t arg0);
	static uint32_t PlayerSpawnHook(uint32_t arg0);
	static uint32_t CXenShieldController_UpdateOnRemoveHook(uint32_t arg0);
	static uint32_t CNihiBallzDestructor(uint32_t arg0);
	static uint32_t InputApplySettingsHook(uint32_t arg0, uint32_t arg1);
	static uint32_t InputSetCSMVolumeHook(uint32_t arg0, uint32_t arg1);
	static uint32_t AbsolutePosHook(uint32_t arg0);
	static uint32_t EnumElementHook(uint32_t arg0, uint32_t arg1);
	static uint32_t YawHook(uint32_t arg0);
};

#endif