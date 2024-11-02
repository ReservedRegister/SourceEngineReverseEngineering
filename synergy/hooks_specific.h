#ifndef HOOKS_SPECIFIC_H
#define HOOKS_SPECIFIC_H

void ApplyPatchesSpecificSynergy();
void HookFunctionsSpecificSynergy();

class NativeHooks
{
public:
	static uint32_t HelicopterCrashFix(uint32_t arg0);
	static uint32_t AiHintNpcCombinePatch(uint32_t arg0, uint32_t arg1);
	static uint32_t TransitionFixTheSecond(uint32_t arg0);
	static uint32_t PatchAnotherPlayerAccessCrash(uint32_t arg0);
	static uint32_t BarneyThinkHook(uint32_t arg0, uint32_t arg1, uint32_t arg2);
	static uint32_t ChkHandle(uint32_t arg0, uint32_t arg1);
	static uint32_t AssaultNpcFix(uint32_t arg0, uint32_t arg1);
	static uint32_t BaseNPCHook(uint32_t arg0, uint32_t arg1);
	static uint32_t SpotlightHook(uint32_t arg0);
	static uint32_t StuckCrashFix(uint32_t arg0, uint32_t arg1);
	__attribute__((regparm(2))) static uint32_t WeirdCrashPleaseFix(uint32_t arg0, uint32_t arg1);
	static uint32_t CitizenNullCrashFix(uint32_t arg0);
	static uint32_t OldRefUpdateFixOne(uint32_t arg0);
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
	static uint32_t CrashFixForHibernation(uint32_t arg0);
	static uint32_t PatchMissingCheckTwo(uint32_t arg0);
	static uint32_t FixMissingObjectHook(uint32_t arg0, uint32_t arg1, uint32_t arg2, uint32_t arg3);
	static uint32_t WeaponGetHook(uint32_t arg0);
	static uint32_t WeaponBugbaitFixHook(uint32_t arg0, uint32_t arg1);
	static uint32_t pSeqdescHook(uint32_t arg0, uint32_t arg1);
	static uint32_t StriderCrashFix(uint32_t arg0);
	static uint32_t HibernateCrashMore(uint32_t arg0);
	static uint32_t HunterCrashFixTwo(uint32_t arg0);
	static uint32_t VehicleRollermineFix(uint32_t arg0);
	static uint32_t FixStructNullCrash(uint32_t arg0);
	static uint32_t FixNullCrash(uint32_t arg0);
	static uint32_t FixOldManhackCrash(uint32_t arg0);
	static uint32_t SomeEntBadUsageFix(uint32_t arg0);
	static uint32_t CombineAttackFix(uint32_t arg0, uint32_t arg1, uint32_t arg2);
	static uint32_t ManhackAiFix(uint32_t arg0, uint32_t arg1);
	static uint32_t Outland_07_Patch(uint32_t arg0, uint32_t arg1);
	static uint32_t Outland_07_Patch_Two(uint32_t arg0, uint32_t arg1, uint32_t arg2, uint32_t arg3);
	static uint32_t Outland_08_Patch(uint32_t arg0, uint32_t arg1);
	static uint32_t ZombiePatchHook(uint32_t arg0, uint32_t arg1, uint32_t arg2, uint32_t arg3, uint32_t arg4, uint32_t arg5);
	static uint32_t NpcSpawnFix(uint32_t arg0);
	static uint32_t FixAnotherAiCrash(uint32_t arg0, uint32_t arg1, uint32_t arg2, uint32_t arg3);
	static uint32_t FixCombineGoalCrash(uint32_t arg0);
	static uint32_t EntVerifyFixThink(uint32_t arg0);
	static uint32_t PatchNpcCrashTrainstation02(uint32_t arg0);
	static uint32_t MissingVphysicsObjectCrashIdk(uint32_t arg0, uint32_t arg1, uint32_t arg2);
};

#endif