#include "extension.h"
#include "core.h"
#include "ext_main.h"
#include "hooks_specific.h"

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

    FindEntityByClassname = (pThreeArgProt)(server_srv + 0x008F3870);
    SetSolidFlags = (pTwoArgProt)(server_srv + 0x004A7700);

    disable_delete_list = false;
    isTicking = false;
    player_spawned = false;
    hooked_delete_counter = 0;
    normal_delete_counter = 0;
    CGlobalEntityList = server_srv + 0x018711E0;
    server_sleeping = false;

    PopulateHookExclusionLists();

    ApplyPatches();
    ApplyPatchesSpecific();

    DisableCacheCvars();
    
    HookFunctions();
    HookFunctionsSpecific();

    RestoreMemoryProtections();

    rootconsole->ConsolePrint("----------------------  " SMEXT_CONF_NAME " " SMEXT_CONF_VERSION " loaded!" "  ----------------------");
}

void ApplyPatches()
{
    uint32_t offset = 0;

    uint32_t hook_game_frame_delete_list = server_srv + 0x00944FAF;
    offset = (uint32_t)Hooks::SimulateEntitiesHook - hook_game_frame_delete_list - 5;
    *(uint32_t*)(hook_game_frame_delete_list+1) = offset;

    uint32_t eventqueue_hook = server_srv + 0x00944FB9;
    offset = (uint32_t)Hooks::ServiceEventQueueHook - eventqueue_hook - 5;
    *(uint32_t*)(eventqueue_hook+1) = offset;

    uint32_t fix_destructor = vphysics_srv + 0x000471B4;
    *(uint8_t*)(fix_destructor) = 0xEB;

    uint32_t remove_post_systems = server_srv + 0x00944FB4;
    memset((void*)remove_post_systems, 0x90, 5);

    //uint32_t bypass_code_ragdoll = server_srv + 0x008A3BDC;
    //memset((void*)bypass_code_ragdoll, 0x90, 6);

    //uint32_t delete_list_call = server_srv + 0x00944F61;
    //memset((void*)delete_list_call, 0x90, 5);

    //uint32_t delete_list_call_sim = server_srv + 0x00A7AC57;
    //memset((void*)delete_list_call_sim, 0x90, 5);

    //uint32_t delete_list_call_last = server_srv + 0x00944FC5;
    //memset((void*)delete_list_call_last, 0x90, 5);

    uint32_t bad_call_remove = server_srv + 0x009B5054;
    memset((void*)bad_call_remove, 0x90, 5);

    //uint32_t patch_remove = server_srv + 0x00B66B0B;
    //memset((void*)patch_remove, 0x90, 5);

    //*(uint8_t*)(patch_remove) = 0x31;
    //*(uint8_t*)(patch_remove+1) = 0xC0;

    uint32_t jmp_vphys = server_srv + 0x004E4146;
    *(uint8_t*)(jmp_vphys) = 0xEB;

    uint32_t sim_patch = server_srv + 0x00A7ADB4;
    memset((void*)sim_patch, 0x90, 6);

    uint32_t fix_script_think = server_srv + 0x00B0416D;
    *(uint8_t*)(fix_script_think) = 0xE9;
    *(uint32_t*)(fix_script_think+1) = 0xAC;
}

void HookFunctions()
{
    //rootconsole->ConsolePrint("patching calloc()");
    //HookFunctionInSharedObject(server_srv, server_srv_size, (void*)calloc, (void*)Hooks::CallocHook);
    //HookFunctionInSharedObject(engine_srv, engine_srv_size, (void*)calloc, (void*)Hooks::CallocHook);
    //HookFunctionInSharedObject(materialsystem_srv, materialsystem_srv_size, (void*)calloc, (void*)CallocHook);
    //HookFunctionInSharedObject(vphysics_srv, vphysics_srv_size, (void*)calloc, (void*)Hooks::CallocHook);
    //HookFunctionInSharedObject(dedicated_srv, dedicated_srv_size, (void*)calloc, (void*)Hooks::CallocHook);
    //HookFunctionInSharedObject(datacache_srv, datacache_srv_size, (void*)calloc, (void*)CallocHook);

    //->ConsolePrint("patching malloc()");
    //HookFunctionInSharedObject(server_srv, server_srv_size, (void*)malloc, (void*)Hooks::MallocHook);
    //HookFunctionInSharedObject(engine_srv, engine_srv_size, (void*)malloc, (void*)Hooks::MallocHook);
    //HookFunctionInSharedObject(materialsystem_srv, materialsystem_srv_size, (void*)malloc, (void*)MallocHook);
    //HookFunctionInSharedObject(vphysics_srv, vphysics_srv_size, (void*)malloc, (void*)Hooks::MallocHook);
    //HookFunctionInSharedObject(dedicated_srv, dedicated_srv_size, (void*)malloc, (void*)Hooks::MallocHook);
    //HookFunctionInSharedObject(datacache_srv, datacache_srv_size, (void*)malloc, (void*)MallocHook);

    //rootconsole->ConsolePrint("patching realloc()");
    //HookFunctionInSharedObject(server_srv, server_srv_size, (void*)realloc, (void*)Hooks::ReallocHook);
    //HookFunctionInSharedObject(engine_srv, engine_srv_size, (void*)realloc, (void*)Hooks::ReallocHook);
    //HookFunctionInSharedObject(materialsystem_srv, materialsystem_srv_size, (void*)realloc, (void*)ReallocHook);
    //HookFunctionInSharedObject(vphysics_srv, vphysics_srv_size, (void*)realloc, (void*)Hooks::ReallocHook);
    //HookFunctionInSharedObject(dedicated_srv, dedicated_srv_size, (void*)realloc, (void*)Hooks::ReallocHook);
    //HookFunctionInSharedObject(datacache_srv, datacache_srv_size, (void*)realloc, (void*)ReallocHook);

    /*rootconsole->ConsolePrint("patching new[]()");
    HookFunctionInSharedObject(server_srv, server_srv_size, (void*)(server_srv + 0x00540820), (void*)OperatorNewArrayHook);
    HookFunctionInSharedObject(engine_srv, engine_srv_size, (void*)(engine_srv + 0x000A7C70), (void*)OperatorNewArrayHook);
    HookFunctionInSharedObject(materialsystem_srv, materialsystem_srv_size, (void*)(materialsystem_srv + 0x00031230), (void*)OperatorNewArrayHook);
    HookFunctionInSharedObject(vphysics_srv, vphysics_srv_size, (void*)(vphysics_srv + 0x0002B4E0), (void*)OperatorNewArrayHook);
    HookFunctionInSharedObject(datacache_srv, datacache_srv_size, (void*)(datacache_srv + 0x00027A40), (void*)OperatorNewArrayHook);*/

    HookFunctionInSharedObject(server_srv, server_srv_size, (void*)(server_srv + 0x009AF380), (void*)Hooks::CreateEntityByNameHook);
    HookFunctionInSharedObject(server_srv, server_srv_size, (void*)(server_srv + 0x00942190), (void*)Hooks::SpawnServerHook);
    HookFunctionInSharedObject(server_srv, server_srv_size, (void*)(server_srv + 0x008F3640), (void*)Hooks::CleanupDeleteListHook);
    HookFunctionInSharedObject(engine_srv, engine_srv_size, (void*)(engine_srv + 0x001957B0), (void*)Hooks::SV_FrameHook);
    HookFunctionInSharedObject(server_srv, server_srv_size, (void*)(server_srv + 0x00B66AF0), (void*)Hooks::UTIL_RemoveHook);
    HookFunctionInSharedObject(engine_srv, engine_srv_size, (void*)(engine_srv + 0x0011CB10), (void*)Hooks::HostChangelevelHook);
    HookFunctionInSharedObject(server_srv, server_srv_size, (void*)(server_srv + 0x00A7A730), (void*)Hooks::PhysSimEnt);
    //HookFunctionInSharedObject(server_srv, server_srv_size, (void*)(server_srv + 0x004CA9E0), (void*)Hooks::EmptyCall);
    //HookFunctionInSharedObject(server_srv, server_srv_size, (void*)(server_srv + 0x006BD6F0), (void*)Hooks::EmptyCall);
    //HookFunctionInSharedObject(server_srv, server_srv_size, (void*)(server_srv + 0x008C9950), (void*)Hooks::EmptyCall);
    //HookFunctionInSharedObject(server_srv, server_srv_size, (void*)(server_srv + 0x00AB1D20), (void*)Hooks::EmptyCall);
    HookFunctionInSharedObject(server_srv, server_srv_size, (void*)(server_srv + 0x00B66BC0), (void*)Hooks::HookInstaKill);


    HookFunctionInSharedObject(server_srv, server_srv_size, (void*)(server_srv + 0x008A39C0), (void*)Hooks::EmptyCall);
    //HookFunctionInSharedObject(server_srv, server_srv_size, (void*)(server_srv + 0x0070BD10), (void*)Hooks::EmptyCall);
    //HookFunctionInSharedObject(server_srv, server_srv_size, (void*)(server_srv + 0x004ED8F0), (void*)Hooks::EmptyCall);


    HookFunctionInSharedObject(server_srv, server_srv_size, (void*)(server_srv + 0x008835B0), (void*)Hooks::IRelationTypeHook);
    //HookFunctionInSharedObject(server_srv, server_srv_size, (void*)(server_srv + 0x0054CC80), (void*)Hooks::EmptyCall);

    HookFunctionInSharedObject(server_srv, server_srv_size, (void*)(server_srv + 0x0052B020), (void*)Hooks::EmptyCall);
    HookFunctionInSharedObject(server_srv, server_srv_size, (void*)(server_srv + 0x0052A7B0), (void*)Hooks::EmptyCall);
    HookFunctionInSharedObject(server_srv, server_srv_size, (void*)(server_srv + 0x00644C00), (void*)Hooks::AcceptInputHook);
    HookFunctionInSharedObject(server_srv, server_srv_size, (void*)(server_srv + 0x0064BE10), (void*)Hooks::UpdateOnRemove);
    HookFunctionInSharedObject(server_srv, server_srv_size, (void*)(server_srv + 0x006D6160), (void*)Hooks::PlayerSpawnHook);
    HookFunctionInSharedObject(server_srv, server_srv_size, (void*)(server_srv + 0x0074DA80), (void*)Hooks::CXenShieldController_UpdateOnRemoveHook);
    HookFunctionInSharedObject(server_srv, server_srv_size, (void*)(server_srv + 0x00B6A350), (void*)Hooks::UTIL_PrecacheOther_Hook);
    HookFunctionInSharedObject(server_srv, server_srv_size, (void*)(server_srv + 0x00B66E20), (void*)Hooks::UTIL_GetLocalPlayerHook);
}

void DisableCacheCvars()
{
    pTwoArgProt pDynamicTwoArgFunc;
    pDynamicTwoArgFunc = (pTwoArgProt)(engine_srv + 0x001B1340);

    //pDynamicTwoArgFunc(0, (uint32_t)"mod_forcetouchdata 0");
    //pDynamicTwoArgFunc(0, (uint32_t)"mod_forcedata 0");
}

uint32_t Hooks::UTIL_GetLocalPlayerHook()
{
    pZeroArgProt pDynamicZeroArgProt;

    pDynamicZeroArgProt = (pZeroArgProt)(server_srv + 0x00B66E20);
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

    pDynamicTwoArgFunc = (pTwoArgProt)(server_srv + 0x00B6A2E0);
    cVar1 = pDynamicTwoArgFunc(server_srv + 0x018C98A0, arg0);

    if(cVar1 == 0) return 0;

    pDynamicTwoArgFunc = (pTwoArgProt)(server_srv + 0x009AF380);
    piVar2 = pDynamicTwoArgFunc(arg0, 0xFFFFFFFF);


    if(piVar2 != 0)
    {
        if((arg1 != 0) && (*(uint8_t*)arg1 != '\0'))
        {
            pTwoArgProt AllocPooledString = (pTwoArgProt)(server_srv + 0x004C86A0);
            AllocPooledString((uint32_t)(&local_10), arg1);

            //this[0x87] = local_10;
            *(uint32_t*)(piVar2+0x21C) = local_10;

            pOneArgProt DispatchUpdateTransmitState = (pOneArgProt)(server_srv + 0x00642940);
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

    pDynamicOneArgFunc = (pOneArgProt)(server_srv + 0x0064BE10);
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

    pDynamicThreeArgFunc = (pThreeArgProt)(engine_srv + 0x0011CB10);
    return pDynamicThreeArgFunc(arg0, arg1, arg2);
}

uint32_t Hooks::CleanupDeleteListHook(uint32_t arg0)
{
    pOneArgProt pDynamicOneArgFunc;
    if(disable_delete_list) return 0;

    pDynamicOneArgFunc = (pOneArgProt)(server_srv + 0x008F3640);
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

    pDynamicOneArgFunc = (pOneArgProt)(server_srv + 0x0074DA80);
    uint32_t returnVal = pDynamicOneArgFunc(arg0);

    //Add missing call to UpdateOnRemove
    Hooks::UpdateOnRemove(arg0);

    return returnVal;
}

uint32_t Hooks::IRelationTypeHook(uint32_t arg0, uint32_t arg1)
{
    pTwoArgProt pDynamicTwoArgFunc;

    if(arg1)
    {
        pDynamicTwoArgFunc = (pTwoArgProt)(server_srv + 0x008835B0);
        return pDynamicTwoArgFunc(arg0, arg1);
    }

    rootconsole->ConsolePrint("Fixed crash in IRelationType()");
    return 0;
}

uint32_t Hooks::SV_FrameHook(uint32_t arg0)
{
    pOneArgProt pDynamicOneArgFunc;

    pDynamicOneArgFunc = (pOneArgProt)(engine_srv + 0x001957B0);
    return pDynamicOneArgFunc(arg0);
}

uint32_t Hooks::PlayerSpawnHook(uint32_t arg0)
{
    pOneArgProt pDynamicOneArgFunc;

    pDynamicOneArgFunc = (pOneArgProt)(server_srv + 0x006D6160);
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
    pDynamicOneArgFunc = (pOneArgProt)(server_srv + 0x004CAA00);
    pDynamicOneArgFunc(0);

    Hooks::CleanupDeleteListHook(0);

    CheckForLocation();

    Hooks::CleanupDeleteListHook(0);

    //SimulateEntities
    pDynamicOneArgFunc = (pOneArgProt)(server_srv + 0x00A7AC00);
    pDynamicOneArgFunc(arg0);

    Hooks::CleanupDeleteListHook(0);

    //ServiceEventQueue
    pDynamicOneArgFunc = (pOneArgProt)(server_srv + 0x008C9950);
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

    pDynamicTwoArgFunc = (pTwoArgProt)(server_srv + 0x009AF380);
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
    pDynamicOneArgFunc = (pOneArgProt)(server_srv + 0x00B08580);
    uint32_t isMarked = pDynamicOneArgFunc(arg0+0x14);

    if(isMarked)
    {
        rootconsole->ConsolePrint("Simulation ignored for [%s]", clsname);
        return 0;
    }

    disable_delete_list = true;
    pDynamicOneArgFunc = (pOneArgProt)(server_srv + 0x00A7A730);
    uint32_t returnVal = pDynamicOneArgFunc(arg0);
    disable_delete_list = false;
    return returnVal;
}

uint32_t Hooks::SpawnServerHook(uint32_t arg0, uint32_t arg1)
{
    pOneArgProt pDynamicOneArgFunc;
    pTwoArgProt pDynamicTwoArgFunc;
    pThreeArgProt pDynamicThreeArgFunc;

    //UnloadAllModels
    //pDynamicTwoArgFunc = (pTwoArgProt)(engine_srv + 0x0013AE80);
    //pDynamicTwoArgFunc(engine_srv + 0x00320560, 0);

    //Flush materials
    //pDynamicTwoArgFunc = (pTwoArgProt)(materialsystem_srv + 0x0003D280);
    //pDynamicTwoArgFunc(materialsystem_srv + 0x00166B20, 1);

    //ReloadTextures
    //pDynamicOneArgFunc = (pOneArgProt)(materialsystem_srv + 0x0003D240);
    //pDynamicOneArgFunc(materialsystem_srv + 0x00166B20);

    //ReloadAllMaterials
    //pDynamicTwoArgFunc = (pTwoArgProt)(materialsystem_srv + 0x0003D220);
    //pDynamicTwoArgFunc(materialsystem_srv + 0x00166B20, 0);

    //InvalidateMdlCache
    //pDynamicOneArgFunc = (pOneArgProt)(server_srv + 0x00947CC0);
    //pDynamicOneArgFunc(0);

    //FlushAllUnlocked (bone cache)
    //pDynamicOneArgFunc = (pOneArgProt)(server_srv + 0x00B8BBF0);
    //pDynamicOneArgFunc(server_srv + 0x01014880);

    //flush particles
    //pDynamicOneArgFunc = (pOneArgProt)(server_srv + 0x00BE7650);
    //pDynamicOneArgFunc(server_srv + 0x018DF7C0);

    //flush datacache
    //pDynamicThreeArgFunc = (pThreeArgProt)(datacache_srv + 0x00028550);
    //pDynamicThreeArgFunc(datacache_srv + 0x0007C0C0, (uint32_t)false, (uint32_t)false);

    //flush mdlcache
    //pDynamicTwoArgFunc = (pTwoArgProt)(datacache_srv + 0x00031850);
    //pDynamicTwoArgFunc(datacache_srv + 0x0007C2E0, (uint32_t)MDLCACHE_FLUSH_ALL);

    player_spawned = false;

    pDynamicTwoArgFunc = (pTwoArgProt)(server_srv + 0x00942190);
    return pDynamicTwoArgFunc(arg0, arg1);
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
        rootconsole->ConsolePrint("AcceptInput blocked a marked entity!");
        return 0;
    }

    //Passed sanity check
    pSixArgProt pDynamicSixArgProt;
    pDynamicSixArgProt = (pSixArgProt)(server_srv + 0x00644C00);
    return pDynamicSixArgProt(arg0, arg1, arg2, arg3, arg4, arg5);
}