#include "extension.h"

#ifndef EXT_MAIN_H
#define EXT_MAIN_H

void InitExtension();
void ApplySingleHooks();
void HookFunctions();
void DisableCacheCvars();

class Hooks
{
public:
	static uint32_t EmptyCall();
	static uint32_t SpawnServerHook(uint32_t arg0, uint32_t arg1);
	static uint32_t CreateEntityByNameHook(uint32_t arg0, uint32_t arg1);
	static uint32_t GameFrameHook(uint32_t arg0);
	static uint32_t Util_RemoveHook(uint32_t arg0);
	static uint32_t HostChangelevelHook(uint32_t arg0, uint32_t arg1, uint32_t arg2);
	static uint32_t CleanupDeleteListHook(uint32_t arg0);
	static uint32_t PhysSimEnt(uint32_t arg0);
	static uint32_t HookInstaKill(uint32_t arg0);
	static uint32_t SV_FrameHook(uint32_t arg0);
	static uint32_t TakeDamageAliveHook(uint32_t arg0, uint32_t arg1);
	static uint32_t IRelationTypeHook(uint32_t arg0, uint32_t arg1);
    static uint32_t CallocHook(uint32_t nitems, uint32_t size);
    static uint32_t MallocHook(uint32_t size);
    static uint32_t ReallocHook(uint32_t old_ptr, uint32_t new_size);
    static uint32_t OperatorNewArrayHook(uint32_t size);
	static uint32_t ClearEntitiesHook(uint32_t arg0);
};

#endif