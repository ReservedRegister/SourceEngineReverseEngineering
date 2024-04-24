void ApplyPatchesSpecific();
void HookFunctionsSpecific();

class NativeHooks
{
public:
	static uint32_t CNihiBallzDestructor(uint32_t arg0);
	static uint32_t InputApplySettingsHook(uint32_t arg0, uint32_t arg1);
};