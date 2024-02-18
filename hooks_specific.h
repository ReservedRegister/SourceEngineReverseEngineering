#ifndef HOOKS_SPECIFIC_H
#define HOOKS_SPECIFIC_H

void HookFunctionsSpecific();

class NativeHooks
{
public:
	static uint32_t HelicopterCrashFix(uint32_t arg0);
	static uint32_t AiHintNpcCombinePatch(uint32_t arg0, uint32_t arg1);
	static uint32_t TransitionFixTheSecond(uint32_t arg0);
	static uint32_t PatchAnotherPlayerAccessCrash(uint32_t arg0);
	static uint32_t BarneyThinkHook(uint32_t arg0, uint32_t arg1, uint32_t arg2);
	static uint32_t ChkHandle(uint32_t arg0, uint32_t arg1);
	static uint32_t FixBaseEntityNullCrash(uint32_t arg0, uint32_t arg1, uint32_t arg2);
	static uint32_t DropshipSimulationCrashFix(uint32_t arg0, uint32_t arg1, uint32_t arg2, uint32_t arg3);
	static uint32_t Outland_07_Patch(uint32_t arg0, uint32_t arg1);
	static uint32_t AssaultNpcFix(uint32_t arg0, uint32_t arg1);
	static uint32_t BaseNPCHook(uint32_t arg0, uint32_t arg1);
	static uint32_t SpotlightHook(uint32_t arg0, uint32_t arg1);
	static uint32_t StuckCrashFix(uint32_t arg0, uint32_t arg1);
	__attribute__((regparm(2))) static uint32_t WeirdCrashPleaseFix(uint32_t arg0, uint32_t arg1);
	static uint32_t CitizenNullCrashFix(uint32_t arg0);
	static uint32_t OldRefUpdateFixOne(uint32_t arg0);
	static uint32_t Outland_07_Patch_Two(uint32_t arg0, uint32_t arg1, uint32_t arg2, uint32_t arg3);
	static uint32_t PatchMissingCheck(uint32_t arg0, uint32_t arg1, uint32_t arg2);
	static uint32_t FixExplodeInputCrash(uint32_t arg0);
	static uint32_t ManhackSpriteEntVerify(uint32_t arg0, uint32_t arg1);
	static uint32_t HelicopterBadDetected(uint32_t arg0);
	static uint32_t PropCombineBall(uint32_t arg0, uint32_t arg1, uint32_t arg2);
	static uint32_t TakeDamageHeliFix(uint32_t arg0, uint32_t arg1, uint32_t arg2, uint32_t arg3);
	static uint32_t StringCmpHook(uint32_t arg0, uint32_t arg1);
	static uint32_t HunterThinkCrashFix(uint32_t arg0);
	static uint32_t FVisibleHook(uint32_t arg0, uint32_t arg1, uint32_t arg2, uint32_t arg3);
	static uint32_t AnotherObjectMissingCheck(uint32_t arg0, uint32_t arg1, uint32_t arg2);
	static uint32_t InterPenetrationFix(uint32_t arg0, uint32_t arg1, uint32_t arg2, uint32_t arg3, uint32_t arg4);
	static uint32_t AiThinkFix(uint32_t arg0, uint32_t arg1);
};

#endif