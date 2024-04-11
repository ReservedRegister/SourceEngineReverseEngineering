#include "extension.h"
#include "core.h"
#include "hooks_specific.h"

void ApplyPatchesSpecific()
{
    uint32_t player_patch_one = server_srv + 0x00589CD9;
    memset((void*)player_patch_one, 0x90, 6);
    
    *(uint8_t*)(player_patch_one) = 0xE9;
    *(uint32_t*)(player_patch_one+1) = 0x19EF;

    uint32_t player_patch_two = server_srv + 0x0058B8D3;
    memset((void*)player_patch_two, 0x90, 6);

    *(uint8_t*)(player_patch_two) = 0xE9;
    *(uint32_t*)(player_patch_two+1) = 0x71E;

    uint32_t vphysicsupdatepatch = server_srv + 0x00413E5B;
    memset((void*)vphysicsupdatepatch, 0x90, 12);

    // SET STACK POINTER
    *(uint8_t*)(vphysicsupdatepatch) = 0x89;
    *(uint8_t*)(vphysicsupdatepatch+1) = 0x1C;
    *(uint8_t*)(vphysicsupdatepatch+2) = 0x24;

    // SET HOOK ADDRESS
    vphysicsupdatepatch = server_srv + 0x00413E62;
    offset = (uint32_t)NativeHooks::VphysicsUpdateWarningHook - vphysicsupdatepatch - 5;
    *(uint8_t*)(vphysicsupdatepatch) = 0xE8;
    *(uint32_t*)(vphysicsupdatepatch+1) = offset;

    uint32_t vphysicsupdatepatch_two = server_srv + 0x00413E92;
    memset((void*)vphysicsupdatepatch_two, 0x90, 7);

    // SET STACK POINTER
    *(uint8_t*)(vphysicsupdatepatch_two) = 0x89;
    *(uint8_t*)(vphysicsupdatepatch_two+1) = 0x1C;
    *(uint8_t*)(vphysicsupdatepatch_two+2) = 0x24;

    // SET HOOK ADDRESS
    vphysicsupdatepatch_two = server_srv + 0x00413EC2;
    offset = (uint32_t)NativeHooks::VphysicsUpdateWarningHook - vphysicsupdatepatch_two - 5;
    *(uint8_t*)(vphysicsupdatepatch_two) = 0xE8;
    *(uint32_t*)(vphysicsupdatepatch_two+1) = offset;
}

void HookFunctionsSpecific()
{
    HookFunctionInSharedObject(server_srv, server_srv_size, (void*)(server_srv + 0x005696E0), (void*)NativeHooks::VTableFixHook);
    HookFunctionInSharedObject(server_srv, server_srv_size, (void*)(server_srv + 0x007C5F20), (void*)NativeHooks::LaunchMortarHook);
    HookFunctionInSharedObject(server_srv, server_srv_size, (void*)(server_srv + 0x004E4C10), (void*)NativeHooks::PhysEnableEntityCollisionsHook);
    HookFunctionInSharedObject(server_srv, server_srv_size, (void*)(server_srv + 0x006AC000), (void*)NativeHooks::CanSelectSchedule);
    HookFunctionInSharedObject(server_srv, server_srv_size, (void*)(server_srv + 0x00643FE0), (void*)NativeHooks::AbsolutePosHook);
    HookFunctionInSharedObject(server_srv, server_srv_size, (void*)(server_srv + 0x00A7CD50), (void*)NativeHooks::EnumElementHook);
    HookFunctionInSharedObject(server_srv, server_srv_size, (void*)(server_srv + 0x005B4EB0), (void*)NativeHooks::YawHook);
    HookFunctionInSharedObject(server_srv, server_srv_size, (void*)(server_srv + 0x0082DFE0), (void*)NativeHooks::CNihiBallzDestructor);
    HookFunctionInSharedObject(server_srv, server_srv_size, (void*)(server_srv + 0x00A1F550), (void*)NativeHooks::InputApplySettingsHook);
    HookFunctionInSharedObject(server_srv, server_srv_size, (void*)(server_srv + 0x00905160), (void*)NativeHooks::InputSetCSMVolumeHook);
    HookFunctionInSharedObject(server_srv, server_srv_size, (void*)(server_srv + 0x008A7200), (void*)NativeHooks::CPropHevCharger_ShouldApplyEffect);
    HookFunctionInSharedObject(server_srv, server_srv_size, (void*)(server_srv + 0x008A7700), (void*)NativeHooks::CPropRadiationCharger_ShouldApplyEffect);
    HookFunctionInSharedObject(server_srv, server_srv_size, (void*)(server_srv + 0x00B01EE0), (void*)NativeHooks::ScriptThinkEntCheck);
    HookFunctionInSharedObject(server_srv, server_srv_size, (void*)(server_srv + 0x0062E0E0), (void*)NativeHooks::TakeDamageHook);
}

uint32_t NativeHooks::TakeDamageHook(uint32_t arg0, uint32_t arg1)
{
    pTwoArgProt pDynamicTwoArgFunc;

    if(arg1)
    {
        uint32_t chkRef = *(uint32_t*)(arg1+0x28);
        uint32_t object = GetCBaseEntity(chkRef);

        if(IsEntityValid(object))
        {
            pDynamicTwoArgFunc = (pTwoArgProt)(server_srv + 0x0062E0E0);
            return pDynamicTwoArgFunc(arg0, arg1);
        }
    }

    rootconsole->ConsolePrint("Fixed crash in take damage function");
    return 0;
}

uint32_t NativeHooks::VTableFixHook(uint32_t arg0, uint32_t arg1)
{
    pTwoArgProt pDynamicTwoArgFunc;

    if(IsEntityValid(arg1))
    {
        *(uint32_t*)(arg0+0x9E4) = refHandle;
        return 0;
    }
    
    *(uint32_t*)(arg0+0x9E4) = 0xFFFFFFFF;
    return 0;
}

uint32_t NativeHooks::EnumElementHook(uint32_t arg0, uint32_t arg1)
{
    pTwoArgProt pDynamicTwoArgFunc;

    if(IsEntityValid(arg1))
    {
        pDynamicTwoArgFunc = (pTwoArgProt)(server_srv + 0x00A7CD50);
        return pDynamicTwoArgFunc(arg0, arg1);
    }

    rootconsole->ConsolePrint("Attempted to use a dead object!");
    return 0;
}

uint32_t NativeHooks::AbsolutePosHook(uint32_t arg0)
{
    pOneArgProt pDynamicOneArgFunc;

    if(IsEntityValid(arg0))
    {
        pDynamicOneArgFunc = (pOneArgProt)(server_srv + 0x00643FE0);
        return pDynamicOneArgFunc(arg0);
    }

    rootconsole->ConsolePrint("Attempted to use a dead object!");
    return 0;
}

uint32_t NativeHooks::ScriptThinkEntCheck(uint32_t arg0)
{
    pOneArgProt pDynamicOneArgFunc;

    pDynamicOneArgFunc = (pOneArgProt)(server_srv + 0x00B01EE0);
    uint32_t returnVal = pDynamicOneArgFunc(arg0);

    uint32_t refHandle = *(uint32_t*)(arg0+0x3B0);
    uint32_t chkRef = GetCBaseEntity(refHandle);

    if(IsEntityValid(chkRef) == 0)
    {
        return 0;
    }

    return returnVal;
}

uint32_t NativeHooks::InputApplySettingsHook(uint32_t arg0, uint32_t arg1)
{
    pTwoArgProt pDynamicTwoArgFunc;

    uint32_t object = *(uint32_t*)(arg0+0x35C);

    if(IsEntityValid(object) == 0)
    {
        *(uint32_t*)(arg0+0x35C) = 0;
    }

    pDynamicTwoArgFunc = (pTwoArgProt)(server_srv + 0x00A1F550);
    return pDynamicTwoArgFunc(arg0, arg1);
}

uint32_t NativeHooks::PhysEnableEntityCollisionsHook(uint32_t arg0, uint32_t arg1)
{
    pOneArgProt pDynamicOneArgFunc;
    pTwoArgProt pDynamicTwoArgFunc;

    if(IsEntityValid(arg0) && IsEntityValid(arg1))
    {
        pDynamicTwoArgFunc = (pTwoArgProt)(server_srv + 0x004E4C10);
        return pDynamicTwoArgFunc(arg0, arg1);
    }

    rootconsole->ConsolePrint("Validation failed!");
    return 0;
}

uint32_t NativeHooks::VphysicsUpdateWarningHook(uint32_t arg0)
{
    if(IsEntityValid(arg0))
    {
        char* classname = (char*)(*(uint32_t*)(arg0+0x64));

        if(classname)
        {
            rootconsole->ConsolePrint("Removing unreasonable entity! [%s]", classname);
        }
        else
        {
            rootconsole->ConsolePrint("Removing unreasonable entity!");
        }

        Hooks::UTIL_RemoveHook(arg0+0x14);
        return 0;
    }

    rootconsole->ConsolePrint("Invalid Entity in vphysics!");
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
            pDynamicTwoArgFunc = (pTwoArgProt)(server_srv + 0x00905160);
            return pDynamicTwoArgFunc(arg0, arg1);
        }
    }

    rootconsole->ConsolePrint("Entity was NULL");
    return 0;
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

    pDynamicOneArgFunc = (pOneArgProt)(server_srv + 0x0082DFE0);
    return pDynamicOneArgFunc(arg0);
}

uint32_t NativeHooks::LaunchMortarHook(uint32_t arg0)
{
    pOneArgProt pDynamicOneArgFunc;

    if(IsEntityValid(arg0))
    {
        pDynamicOneArgFunc = (pOneArgProt)(server_srv + 0x007C5F20);
        return pDynamicOneArgFunc(arg0);
    }

    rootconsole->ConsolePrint("Gonarch was invalid!");
    return 0;
}

uint32_t NativeHooks::CPropHevCharger_ShouldApplyEffect(uint32_t arg0, uint32_t arg1)
{
    pTwoArgProt pDynamicTwoArgFunc;
    if(arg1 == 0) return 0;
    
    pDynamicTwoArgFunc = (pTwoArgProt)(server_srv + 0x008A7200);
    return pDynamicTwoArgFunc(arg0, arg1);
}

uint32_t NativeHooks::CPropRadiationCharger_ShouldApplyEffect(uint32_t arg0, uint32_t arg1)
{
    pTwoArgProt pDynamicTwoArgFunc;
    if(arg1 == 0) return 0;
    
    pDynamicTwoArgFunc = (pTwoArgProt)(server_srv + 0x008A7700);
    return pDynamicTwoArgFunc(arg0, arg1);
}

uint32_t NativeHooks::YawHook(uint32_t arg0)
{
    pOneArgProt pDynamicOneArgFunc;

    if(arg0)
    {
        pDynamicOneArgFunc = (pOneArgProt)(server_srv + 0x005B4EB0);
        return pDynamicOneArgFunc(arg0);
    }

    rootconsole->ConsolePrint("Failed to find a real object!");
    return 0;
}

uint32_t NativeHooks::CanSelectSchedule(uint32_t arg0)
{
    pOneArgProt pDynamicOneArgFunc;

    uint32_t offset_four = *(uint32_t*)(arg0+4);

    if(offset_four)
    {
        pDynamicOneArgFunc = (pOneArgProt)(server_srv + 0x006AC000);
        return pDynamicOneArgFunc(arg0);
    }

    rootconsole->ConsolePrint("Failed to service AI schedule");
    return 0;
}