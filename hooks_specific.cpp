#include "extension.h"
#include "core.h"
#include "hooks_specific.h"

void ApplyPatchesSpecific()
{
    uint32_t offset = 0;
}

void HookFunctionsSpecific()
{
    HookFunctionInSharedObject(server_srv, server_srv_size, (void*)(server_srv + 0x0070F600), (void*)NativeHooks::CNihiBallzDestructor);
    HookFunctionInSharedObject(server_srv, server_srv_size, (void*)(server_srv + 0x0092E1D0), (void*)NativeHooks::InputApplySettingsHook);
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