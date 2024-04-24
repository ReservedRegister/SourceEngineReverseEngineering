#include "extension.h"
#include "core.h"
#include "ext_main.h"
#include "hooks_specific.h"

#include <math.h>

void InitExtension()
{
    InitCore();
    AllowWriteToMappedMemory();

    char* root_dir = getenv("PWD");
    size_t max_path_length = 1024;

    char server_srv_fullpath[max_path_length];
    char engine_srv_fullpath[max_path_length];
    char materialsystem_srv_fullpath[max_path_length];
    char vphysics_srv_fullpath[max_path_length];
    char dedicated_srv_fullpath[max_path_length];
    char datacache_srv_fullpath[max_path_length];

    snprintf(server_srv_fullpath, max_path_length, "%s/bms/bin/server_srv.so", root_dir);
    snprintf(engine_srv_fullpath, max_path_length, "%s/bin/engine_srv.so", root_dir);
    snprintf(materialsystem_srv_fullpath, max_path_length, "%s/bin/materialsystem_srv.so", root_dir);
    snprintf(vphysics_srv_fullpath, max_path_length, "%s/bin/vphysics_srv.so", root_dir);
    snprintf(dedicated_srv_fullpath, max_path_length, "%s/bin/dedicated_srv.so", root_dir);
    snprintf(datacache_srv_fullpath, max_path_length, "%s/bin/datacache_srv.so", root_dir);

    Library* engine_srv_lib = FindLibrary(engine_srv_fullpath, false);
    Library* server_srv_lib = FindLibrary(server_srv_fullpath, false);
    Library* materialsystem_srv_lib = FindLibrary(materialsystem_srv_fullpath, false);
    Library* vphysics_srv_lib = FindLibrary(vphysics_srv_fullpath, false);
    Library* dedicated_srv_lib = FindLibrary(dedicated_srv_fullpath, false);
    Library* datacache_srv_lib = FindLibrary(datacache_srv_fullpath, false);

    engine_srv = engine_srv_lib->library_base_address;
    server_srv = server_srv_lib->library_base_address;
    materialsystem_srv = materialsystem_srv_lib->library_base_address;
    vphysics_srv = vphysics_srv_lib->library_base_address;
    dedicated_srv = dedicated_srv_lib->library_base_address;
    datacache_srv = datacache_srv_lib->library_base_address;

    engine_srv_size = engine_srv_lib->library_size;
    server_srv_size = server_srv_lib->library_size;
    materialsystem_srv_size = materialsystem_srv_lib->library_size;
    vphysics_srv_size = vphysics_srv_lib->library_size;
    dedicated_srv_size = dedicated_srv_lib->library_size;
    datacache_srv_size = datacache_srv_lib->library_size;

    FindEntityByClassname = (pThreeArgProt)(server_srv + 0x007E7030);
    SetSolidFlags = (pTwoArgProt)(server_srv + 0x00336C60);

    disable_delete_list = false;
    isTicking = false;
    player_spawned = false;
    hooked_delete_counter = 0;
    normal_delete_counter = 0;
    CGlobalEntityList = server_srv + 0x017B6BE0;
    server_sleeping = false;

    PopulateHookExclusionLists();

    ApplyPatches();
    ApplyPatchesSpecific();
    
    HookFunctions();
    HookFunctionsSpecific();

    RestoreMemoryProtections();

    rootconsole->ConsolePrint("----------------------  " SMEXT_CONF_NAME " " SMEXT_CONF_VERSION " loaded!" "  ----------------------");
}

void ApplyPatches()
{
    uint32_t offset = 0;

    uint32_t hook_game_frame_delete_list = server_srv + 0x008404F3;
    offset = (uint32_t)Hooks::SimulateEntitiesHook - hook_game_frame_delete_list - 5;
    *(uint32_t*)(hook_game_frame_delete_list+1) = offset;

    uint32_t eventqueue_hook = server_srv + 0x008404FD;
    offset = (uint32_t)Hooks::ServiceEventQueueHook - eventqueue_hook - 5;
    *(uint32_t*)(eventqueue_hook+1) = offset;

    uint32_t remove_post_systems = server_srv + 0x008404F8;
    memset((void*)remove_post_systems, 0x90, 5);

    //uint32_t bypass_code_ragdoll = server_srv + 0x008A3BDC;
    //memset((void*)bypass_code_ragdoll, 0x90, 6);

    //uint32_t delete_list_call = server_srv + 0x00944F61;
    //memset((void*)delete_list_call, 0x90, 5);

    //uint32_t delete_list_call_sim = server_srv + 0x00A7AC57;
    //memset((void*)delete_list_call_sim, 0x90, 5);

    //uint32_t delete_list_call_last = server_srv + 0x00944FC5;
    //memset((void*)delete_list_call_last, 0x90, 5);

    //CMessageEntity
    uint32_t bad_call_remove = server_srv + 0x008BB59A;
    memset((void*)bad_call_remove, 0x90, 5);

    //uint32_t patch_remove = server_srv + 0x00B66B0B;
    //memset((void*)patch_remove, 0x90, 5);

    //*(uint8_t*)(patch_remove) = 0x31;
    //*(uint8_t*)(patch_remove+1) = 0xC0;
}

void HookFunctions()
{
    HookFunctionInSharedObject(server_srv, server_srv_size, (void*)(server_srv + 0x008B4C80), (void*)Hooks::CreateEntityByNameHook);
    HookFunctionInSharedObject(server_srv, server_srv_size, (void*)(server_srv + 0x007E6D20), (void*)Hooks::CleanupDeleteListHook);
    HookFunctionInSharedObject(server_srv, server_srv_size, (void*)(server_srv + 0x00A92160), (void*)Hooks::UTIL_RemoveHook);
    HookFunctionInSharedObject(engine_srv, engine_srv_size, (void*)(engine_srv + 0x000E6D70), (void*)Hooks::HostChangelevelHook);
    HookFunctionInSharedObject(server_srv, server_srv_size, (void*)(server_srv + 0x009919D0), (void*)Hooks::PhysSimEnt);
    HookFunctionInSharedObject(server_srv, server_srv_size, (void*)(server_srv + 0x00A92260), (void*)Hooks::HookInstaKill);

    //RagdollBreaking
    HookFunctionInSharedObject(server_srv, server_srv_size, (void*)(server_srv + 0x0078FC70), (void*)Hooks::EmptyCall);

    HookFunctionInSharedObject(server_srv, server_srv_size, (void*)(server_srv + 0x004F5B50), (void*)Hooks::AcceptInputHook);
    HookFunctionInSharedObject(server_srv, server_srv_size, (void*)(server_srv + 0x004FD670), (void*)Hooks::UpdateOnRemove);
    HookFunctionInSharedObject(server_srv, server_srv_size, (void*)(server_srv + 0x005983C0), (void*)Hooks::PlayerSpawnHook);
    HookFunctionInSharedObject(server_srv, server_srv_size, (void*)(server_srv + 0x0061B4C0), (void*)Hooks::CXenShieldController_UpdateOnRemoveHook);
    HookFunctionInSharedObject(server_srv, server_srv_size, (void*)(server_srv + 0x00A95FF0), (void*)Hooks::UTIL_PrecacheOther_Hook);
    HookFunctionInSharedObject(server_srv, server_srv_size, (void*)(server_srv + 0x00A92540), (void*)Hooks::UTIL_GetLocalPlayerHook);

    HookFunctionInSharedObject(server_srv, server_srv_size, (void*)(server_srv + 0x0046B510), (void*)Hooks::TestGroundMove);
}

uint32_t Hooks::UTIL_GetLocalPlayerHook()
{
    pZeroArgProt pDynamicZeroArgProt;

    pDynamicZeroArgProt = (pZeroArgProt)(server_srv + 0x00A92540);
    uint32_t returnVal = pDynamicZeroArgProt();

    if(!returnVal)
    {
        return FindEntityByClassname(CGlobalEntityList, 0, (uint32_t)"player");
    }

    return returnVal;
}

uint32_t Hooks::UTIL_PrecacheOther_Hook(uint32_t arg0, uint32_t arg1)
{
    pZeroArgProt pDynamicZeroArgProt;
    pOneArgProt pDynamicOneArgFunc;
    pTwoArgProt pDynamicTwoArgFunc;
    
    uint8_t cVar1;
    uint32_t piVar2;
    uint32_t local_10;

    pDynamicTwoArgFunc = (pTwoArgProt)(server_srv + 0x00A95F80);
    cVar1 = pDynamicTwoArgFunc(server_srv + 0x0180F300, arg0);

    if(cVar1 == 0) return 0;

    pDynamicTwoArgFunc = (pTwoArgProt)(server_srv + 0x008B4C80);
    piVar2 = pDynamicTwoArgFunc(arg0, 0xFFFFFFFF);


    if(piVar2 != 0)
    {
        if((arg1 != 0) && (*(uint8_t*)arg1 != '\0'))
        {
            pTwoArgProt AllocPooledString = (pTwoArgProt)(server_srv + 0x00359FF0);
            AllocPooledString((uint32_t)(&local_10), arg1);

            //this[0x87] = local_10;
            *(uint32_t*)(piVar2+0x21C) = local_10;

            pOneArgProt DispatchUpdateTransmitState = (pOneArgProt)(server_srv + 0x004F3510);
            DispatchUpdateTransmitState(piVar2);

            //pcVar2 = param_2;
        }

        rootconsole->ConsolePrint("Precache %s", arg0);
        
        //Precache
        pDynamicOneArgFunc = (pOneArgProt)(  *(uint32_t*)((*(uint32_t*)(piVar2))+0x68)  );
        pDynamicOneArgFunc(piVar2);

        InstaKill(piVar2, false);
    }

    return 0;
}

uint32_t Hooks::UpdateOnRemove(uint32_t arg0)
{
    pOneArgProt pDynamicOneArgFunc;

    normal_delete_counter++;

    pDynamicOneArgFunc = (pOneArgProt)(server_srv + 0x004FD670);
    return pDynamicOneArgFunc(arg0);
}

uint32_t Hooks::EmptyCall()
{
    return 0;
}

uint32_t Hooks::CallocHook(uint32_t nitems, uint32_t size)
{
    if(nitems <= 0) return (uint32_t)calloc(nitems, size);
    uint32_t enlarged_size = nitems*2.0;
    uint32_t newRef = (uint32_t)calloc(enlarged_size, size);
    return newRef;
}

uint32_t Hooks::MallocHook(uint32_t size)
{
    if(size <= 0) return (uint32_t)malloc(size);
    uint32_t newRef = (uint32_t)malloc(size*2.0);
    return newRef;
}

uint32_t Hooks::ReallocHook(uint32_t old_ptr, uint32_t new_size)
{
    if(new_size <= 0) return (uint32_t)realloc((void*)old_ptr, new_size);
    uint32_t new_ref = (uint32_t)realloc((void*)old_ptr, new_size*2.0);
    return new_ref;
}

uint32_t Hooks::OperatorNewArrayHook(uint32_t size)
{
    if(size <= 0) return (uint32_t)malloc(size);
    uint32_t newRef = (uint32_t)malloc(size*2.0);
    return newRef;
}

uint32_t Hooks::HostChangelevelHook(uint32_t arg0, uint32_t arg1, uint32_t arg2)
{
    pOneArgProt pDynamicOneArgFunc;
    pTwoArgProt pDynamicTwoArgFunc;
    pThreeArgProt pDynamicThreeArgFunc;

    isTicking = false;
    player_spawned = false;

    pDynamicThreeArgFunc = (pThreeArgProt)(engine_srv + 0x000E6D70);
    return pDynamicThreeArgFunc(arg0, arg1, arg2);
}

uint32_t Hooks::CleanupDeleteListHook(uint32_t arg0)
{
    pOneArgProt pDynamicOneArgFunc;
    if(disable_delete_list) return 0;

    pDynamicOneArgFunc = (pOneArgProt)(server_srv + 0x007E6D20);
    return pDynamicOneArgFunc(0);
}

uint32_t Hooks::UTIL_RemoveHook(uint32_t arg0)
{
    if(arg0 == 0)
    {
        rootconsole->ConsolePrint("Remove was NULL");
        return 0;
    }
    
    uint32_t cbaseobject = arg0-0x14;
    RemoveEntityNormal(cbaseobject, true);
    return 0;
}

uint32_t Hooks::CXenShieldController_UpdateOnRemoveHook(uint32_t arg0)
{
    pOneArgProt pDynamicOneArgFunc;

    pDynamicOneArgFunc = (pOneArgProt)(server_srv + 0x0061B4C0);
    uint32_t returnVal = pDynamicOneArgFunc(arg0);

    //Add missing call to UpdateOnRemove
    Hooks::UpdateOnRemove(arg0);

    return returnVal;
}

uint32_t Hooks::PlayerSpawnHook(uint32_t arg0)
{
    pOneArgProt pDynamicOneArgFunc;

    pDynamicOneArgFunc = (pOneArgProt)(server_srv + 0x005983C0);
    uint32_t returnVal = pDynamicOneArgFunc(arg0);

    player_spawned = true;

    return returnVal;
}

uint32_t Hooks::ServiceEventQueueHook()
{
    pOneArgProt pDynamicOneArgFunc;
    return 0;
}

uint32_t Hooks::SimulateEntitiesHook(uint32_t arg0)
{
    pOneArgProt pDynamicOneArgFunc;
    pThreeArgProt pDynamicThreeArgFunc;
    isTicking = true;

    Hooks::CleanupDeleteListHook(0);

    if(hooked_delete_counter == normal_delete_counter)
    {
        hooked_delete_counter = 0;
        normal_delete_counter = 0;
    }
    else
    {
        rootconsole->ConsolePrint("Critical error - entity count mismatch!");
        exit(EXIT_FAILURE);
    }

    uint32_t firstPlayer = FindEntityByClassname(CGlobalEntityList, 0, (uint32_t)"player");

    if(!firstPlayer)
    {
        server_sleeping = true;
    }
    else
    {
        server_sleeping = false;
    }

    Hooks::CleanupDeleteListHook(0);

    //PostSystems
    pDynamicOneArgFunc = (pOneArgProt)(server_srv + 0x0035C740);
    pDynamicOneArgFunc(0);

    Hooks::CleanupDeleteListHook(0);

    CheckForLocation();

    Hooks::CleanupDeleteListHook(0);

    //SimulateEntities
    pDynamicOneArgFunc = (pOneArgProt)(server_srv + 0x00991F80);
    pDynamicOneArgFunc(arg0);

    Hooks::CleanupDeleteListHook(0);

    //ServiceEventQueue
    pDynamicOneArgFunc = (pOneArgProt)(server_srv + 0x007B92B0);
    pDynamicOneArgFunc(0);

    Hooks::CleanupDeleteListHook(0);

    return 0;
}

uint32_t Hooks::HookInstaKill(uint32_t arg0)
{
    InstaKill(arg0, true);
    return 0;
}

uint32_t Hooks::CreateEntityByNameHook(uint32_t arg0, uint32_t arg1)
{
    pTwoArgProt pDynamicTwoArgFunc;
    pThreeArgProt pDynamicThreeArgFunc;

    pDynamicTwoArgFunc = (pTwoArgProt)(server_srv + 0x008B4C80);
    return pDynamicTwoArgFunc(arg0, arg1);
}

uint32_t Hooks::PhysSimEnt(uint32_t arg0)
{
    pOneArgProt pDynamicOneArgFunc;

    if(arg0 == 0)
    {
        rootconsole->ConsolePrint("Passed NULL simulation entity!");
        exit(EXIT_FAILURE);
        return 0;
    }

    uint32_t refHandle = *(uint32_t*)(arg0+0x334);
    uint32_t object_check = GetCBaseEntity(refHandle);

    if(object_check == 0)
    {
        rootconsole->ConsolePrint("Passed in non-existant simulation entity!");
        exit(EXIT_FAILURE);
        return 0;
    }

    char* clsname =  (char*) ( *(uint32_t*)(arg0+0x64) );

    //IsMarkedForDeletion
    pDynamicOneArgFunc = (pOneArgProt)(server_srv + 0x00A2B520);
    uint32_t isMarked = pDynamicOneArgFunc(arg0+0x14);

    if(isMarked)
    {
        rootconsole->ConsolePrint("Simulation ignored for [%s]", clsname);
        return 0;
    }

    disable_delete_list = true;
    pDynamicOneArgFunc = (pOneArgProt)(server_srv + 0x009919D0);
    uint32_t returnVal = pDynamicOneArgFunc(arg0);
    disable_delete_list = false;
    return returnVal;
}

uint32_t Hooks::AcceptInputHook(uint32_t arg0, uint32_t arg1, uint32_t arg2, uint32_t arg3, uint32_t arg4, uint32_t arg5)
{
    pOneArgProt pDynamicOneArgFunc;
    pTwoArgProt pDynamicTwoArgFunc;

    // CBaseEntity arg0 arg2 arg3

    bool failure = false;

    if(IsEntityValid(arg0) == 0) failure = true;

    if(failure)
    {
        //rootconsole->ConsolePrint("AcceptInput blocked a marked entity!");
        return 0;
    }

    //Passed sanity check
    pSixArgProt pDynamicSixArgProt;
    pDynamicSixArgProt = (pSixArgProt)(server_srv + 0x004F5B50);
    return pDynamicSixArgProt(arg0, arg1, arg2, arg3, arg4, arg5);
}

uint32_t Hooks::TestGroundMove(uint32_t arg0, uint32_t arg1, uint32_t arg2, uint32_t arg3, uint32_t arg4, uint32_t arg5, uint32_t arg6)
{
    pSevenArgProt pDynamicSevenArgProt;

    float inf_val_chk = *(float*)(arg6+0x20);

    if(isinf(inf_val_chk))
    {
        rootconsole->ConsolePrint("+Inf detected!");
        return 0;
    }

    pDynamicSevenArgProt = (pSevenArgProt)(server_srv + 0x0046B510);
    return pDynamicSevenArgProt(arg0, arg1, arg2, arg3, arg4, arg5, arg6);
}