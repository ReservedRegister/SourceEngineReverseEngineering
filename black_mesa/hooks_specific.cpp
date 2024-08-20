#include "extension.h"
#include "util.h"
#include "core.h"
#include "hooks_specific.h"

void ApplyPatchesSpecificBlackMesa()
{
    uint32_t offset = 0;
}

void HookFunctionsSpecificBlackMesa()
{
    HookFunctionInSharedObject(server_srv, server_srv_size, (void*)(server_srv + 0x0070F600), (void*)NativeHooks::CNihiBallzDestructor);
    HookFunctionInSharedObject(server_srv, server_srv_size, (void*)(server_srv + 0x0092E1D0), (void*)NativeHooks::InputApplySettingsHook);
    HookFunctionInSharedObject(server_srv, server_srv_size, (void*)(server_srv + 0x007FA870), (void*)NativeHooks::InputSetCSMVolumeHook);
    HookFunctionInSharedObject(server_srv, server_srv_size, (void*)(server_srv + 0x004F4E80), (void*)NativeHooks::CalcAbsolutePosition);
    HookFunctionInSharedObject(server_srv, server_srv_size, (void*)(server_srv + 0x00994570), (void*)NativeHooks::EnumElementHook);
    HookFunctionInSharedObject(server_srv, server_srv_size, (void*)(server_srv + 0x004F0FC0), (void*)NativeHooks::TakeDamageHook);
    HookFunctionInSharedObject(server_srv, server_srv_size, (void*)(server_srv + 0x00793A60), (void*)NativeHooks::CPropHevCharger_ShouldApplyEffect);
    HookFunctionInSharedObject(server_srv, server_srv_size, (void*)(server_srv + 0x00793FD0), (void*)NativeHooks::CPropRadiationCharger_ShouldApplyEffect);
    HookFunctionInSharedObject(server_srv, server_srv_size, (void*)(server_srv + 0x00A24570), (void*)NativeHooks::ScriptThinkEntCheck);
    HookFunctionInSharedObject(server_srv, server_srv_size, (void*)(server_srv + 0x0069DB20), (void*)NativeHooks::LaunchMortarHook);
}

uint32_t NativeHooks::LaunchMortarHook(uint32_t arg0)
{
    pOneArgProt pDynamicOneArgFunc;

    if(IsEntityValid(arg0))
    {
        pDynamicOneArgFunc = (pOneArgProt)(server_srv + 0x0069DB20);
        return pDynamicOneArgFunc(arg0);
    }

    rootconsole->ConsolePrint("Gonarch was invalid!");
    return 0;
}

uint32_t NativeHooks::ScriptThinkEntCheck(uint32_t arg0)
{
    pOneArgProt pDynamicOneArgFunc;

    pDynamicOneArgFunc = (pOneArgProt)(server_srv + 0x00A24570);
    uint32_t returnVal = pDynamicOneArgFunc(arg0);

    uint32_t refHandle = *(uint32_t*)(arg0+0x3B0);
    uint32_t chkRef = GetCBaseEntityBlackMesa(refHandle);

    if(!chkRef) return 0;
    return returnVal;
}

uint32_t NativeHooks::CalcAbsolutePosition(uint32_t arg0)
{
    pOneArgProt pDynamicOneArgFunc;

    if(IsEntityValid(arg0))
    {
        pDynamicOneArgFunc = (pOneArgProt)(server_srv + 0x004F4E80);
        return pDynamicOneArgFunc(arg0);
    }

    //rootconsole->ConsolePrint("Attempted to use a dead object!");
    return 0;
}

uint32_t NativeHooks::InputSetCSMVolumeHook(uint32_t arg0, uint32_t arg1)
{
    pTwoArgProt pDynamicTwoArgFunc;

    if(arg1)
    {
        uint32_t base_offset = *(uint32_t*)(arg1);
        uint32_t fourth_offset = *(uint32_t*)(arg1+4);

        if(base_offset && fourth_offset)
        {
            pDynamicTwoArgFunc = (pTwoArgProt)(server_srv + 0x007FA870);
            return pDynamicTwoArgFunc(arg0, arg1);
        }
    }

    rootconsole->ConsolePrint("Entity was NULL");
    return 0;
}

uint32_t NativeHooks::InputApplySettingsHook(uint32_t arg0, uint32_t arg1)
{
    pTwoArgProt pDynamicTwoArgFunc;

    uint32_t object = *(uint32_t*)(arg0+0x35C);

    if(IsEntityValid(object) == 0)
    {
        *(uint32_t*)(arg0+0x35C) = 0;
    }

    pDynamicTwoArgFunc = (pTwoArgProt)(server_srv + 0x0092E1D0);
    return pDynamicTwoArgFunc(arg0, arg1);
}


uint32_t NativeHooks::CNihiBallzDestructor(uint32_t arg0)
{
    pOneArgProt pDynamicOneArgFunc;

    uint32_t cbaseobject_one = *(uint32_t*)(arg0+0x72C);
    uint32_t cbaseobject_two = *(uint32_t*)(arg0+0x730);

    if(IsEntityValid(cbaseobject_one) == 0)
    {
        *(uint32_t*)(arg0+0x72C) = 0;
    }

    if(IsEntityValid(cbaseobject_two) == 0)
    {
        *(uint32_t*)(arg0+0x730) = 0;
    }

    pDynamicOneArgFunc = (pOneArgProt)(server_srv + 0x0070F600);
    return pDynamicOneArgFunc(arg0);
}

uint32_t NativeHooks::EnumElementHook(uint32_t arg0, uint32_t arg1)
{
    pTwoArgProt pDynamicTwoArgFunc;

    if(IsEntityValid(arg1))
    {
        pDynamicTwoArgFunc = (pTwoArgProt)(server_srv + 0x00994570);
        return pDynamicTwoArgFunc(arg0, arg1);
    }

    //rootconsole->ConsolePrint("Attempted to use a dead object!");
    return 0;
}

uint32_t NativeHooks::TakeDamageHook(uint32_t arg0, uint32_t arg1)
{
    pTwoArgProt pDynamicTwoArgFunc;

    if(arg1)
    {
        uint32_t chkRef = *(uint32_t*)(arg1+0x28);
        uint32_t object = GetCBaseEntityBlackMesa(chkRef);

        if(IsEntityValid(object))
        {
            pDynamicTwoArgFunc = (pTwoArgProt)(server_srv + 0x004F0FC0);
            return pDynamicTwoArgFunc(arg0, arg1);
        }
    }

    rootconsole->ConsolePrint("Fixed crash in take damage function");
    return 0;
}

uint32_t NativeHooks::CPropHevCharger_ShouldApplyEffect(uint32_t arg0, uint32_t arg1)
{
    pTwoArgProt pDynamicTwoArgFunc;
    if(arg1 == 0) return 0;
    
    pDynamicTwoArgFunc = (pTwoArgProt)(server_srv + 0x00793A60);
    return pDynamicTwoArgFunc(arg0, arg1);
}

uint32_t NativeHooks::CPropRadiationCharger_ShouldApplyEffect(uint32_t arg0, uint32_t arg1)
{
    pTwoArgProt pDynamicTwoArgFunc;
    if(arg1 == 0) return 0;
    
    pDynamicTwoArgFunc = (pTwoArgProt)(server_srv + 0x00793FD0);
    return pDynamicTwoArgFunc(arg0, arg1);
}