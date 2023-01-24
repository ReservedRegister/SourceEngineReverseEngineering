#include "ext_main.h"
#include "core.h"

void InitExtension()
{
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

    disable_delete_list = false;
    isTicking = false;
    CGlobalEntityList = server_srv + 0x018711E0;
    deleteList = AllocateValuesList();

    PopulateHookExclusionLists();
    ApplySingleHooks();
    DisableCacheCvars();
    HookFunctions();

    RestoreMemoryProtections();

    rootconsole->ConsolePrint("----------------------  " SMEXT_CONF_NAME " " SMEXT_CONF_VERSION " loaded!" "  ----------------------");
}

void ApplySingleHooks()
{
    uint32_t offset = 0;

    uint32_t hook_game_frame_delete_list = server_srv + 0x00944FAF;
    offset = (uint32_t)Hooks::GameFrameHook - hook_game_frame_delete_list - 5;
    *(uint32_t*)(hook_game_frame_delete_list+1) = offset;

    uint32_t delete_list_call = server_srv + 0x00944F61;
    memset((void*)delete_list_call, 0x90, 5);

    delete_list_call = server_srv + 0x00944FC5;
    memset((void*)delete_list_call, 0x90, 5);

    delete_list_call = server_srv + 0x00A7AC57;
    memset((void*)delete_list_call, 0x90, 5);
}

void HookFunctions()
{
    /*rootconsole->ConsolePrint("patching calloc()");
    HookFunctionInSharedObject(server_srv, server_srv_size, (void*)calloc, (void*)CallocHook);
    HookFunctionInSharedObject(engine_srv, engine_srv_size, (void*)calloc, (void*)CallocHook);
    HookFunctionInSharedObject(materialsystem_srv, materialsystem_srv_size, (void*)calloc, (void*)CallocHook);
    HookFunctionInSharedObject(vphysics_srv, vphysics_srv_size, (void*)calloc, (void*)CallocHook);
    HookFunctionInSharedObject(dedicated_srv, dedicated_srv_size, (void*)calloc, (void*)CallocHook);
    HookFunctionInSharedObject(datacache_srv, datacache_srv_size, (void*)calloc, (void*)CallocHook);*/
    //rootconsole->ConsolePrint("patching malloc()");
    //HookFunctionInSharedObject(server_srv, server_srv_size, (void*)malloc, (void*)MallocHook);
    /*HookFunctionInSharedObject(engine_srv, engine_srv_size, (void*)malloc, (void*)MallocHook);
    HookFunctionInSharedObject(materialsystem_srv, materialsystem_srv_size, (void*)malloc, (void*)MallocHook);
    HookFunctionInSharedObject(vphysics_srv, vphysics_srv_size, (void*)malloc, (void*)MallocHook);
    HookFunctionInSharedObject(dedicated_srv, dedicated_srv_size, (void*)malloc, (void*)MallocHook);
    HookFunctionInSharedObject(datacache_srv, datacache_srv_size, (void*)malloc, (void*)MallocHook);*/
    /*rootconsole->ConsolePrint("patching realloc()");
    HookFunctionInSharedObject(server_srv, server_srv_size, (void*)realloc, (void*)ReallocHook);
    HookFunctionInSharedObject(engine_srv, engine_srv_size, (void*)realloc, (void*)ReallocHook);
    HookFunctionInSharedObject(materialsystem_srv, materialsystem_srv_size, (void*)realloc, (void*)ReallocHook);
    HookFunctionInSharedObject(vphysics_srv, vphysics_srv_size, (void*)realloc, (void*)ReallocHook);
    HookFunctionInSharedObject(dedicated_srv, dedicated_srv_size, (void*)realloc, (void*)ReallocHook);
    HookFunctionInSharedObject(datacache_srv, datacache_srv_size, (void*)realloc, (void*)ReallocHook);
    rootconsole->ConsolePrint("patching new[]()");
    HookFunctionInSharedObject(server_srv, server_srv_size, (void*)(server_srv + 0x00540820), (void*)OperatorNewArrayHook);
    HookFunctionInSharedObject(engine_srv, engine_srv_size, (void*)(engine_srv + 0x000A7C70), (void*)OperatorNewArrayHook);
    HookFunctionInSharedObject(materialsystem_srv, materialsystem_srv_size, (void*)(materialsystem_srv + 0x00031230), (void*)OperatorNewArrayHook);
    HookFunctionInSharedObject(vphysics_srv, vphysics_srv_size, (void*)(vphysics_srv + 0x0002B4E0), (void*)OperatorNewArrayHook);
    HookFunctionInSharedObject(datacache_srv, datacache_srv_size, (void*)(datacache_srv + 0x00027A40), (void*)OperatorNewArrayHook);*/

    HookFunctionInSharedObject(server_srv, server_srv_size, (void*)(server_srv + 0x009AF380), (void*)Hooks::CreateEntityByNameHook);
    HookFunctionInSharedObject(server_srv, server_srv_size, (void*)(server_srv + 0x00942190), (void*)Hooks::SpawnServerHook);
    HookFunctionInSharedObject(server_srv, server_srv_size, (void*)(server_srv + 0x008F3640), (void*)Hooks::CleanupDeleteListHook);
    HookFunctionInSharedObject(engine_srv, engine_srv_size, (void*)(engine_srv + 0x001957B0), (void*)Hooks::SV_FrameHook);
    HookFunctionInSharedObject(server_srv, server_srv_size, (void*)(server_srv + 0x00B66B70), (void*)Hooks::Util_RemoveHook);
    HookFunctionInSharedObject(engine_srv, engine_srv_size, (void*)(engine_srv + 0x0011CB10), (void*)Hooks::HostChangelevelHook);
    HookFunctionInSharedObject(server_srv, server_srv_size, (void*)(server_srv + 0x00A7A730), (void*)Hooks::PhysSimEnt);
    HookFunctionInSharedObject(server_srv, server_srv_size, (void*)(server_srv + 0x004CA9E0), (void*)Hooks::EmptyCall);
    HookFunctionInSharedObject(server_srv, server_srv_size, (void*)(server_srv + 0x006BD6F0), (void*)Hooks::EmptyCall);
    HookFunctionInSharedObject(server_srv, server_srv_size, (void*)(server_srv + 0x00A7AC00), (void*)Hooks::EmptyCall);
    HookFunctionInSharedObject(server_srv, server_srv_size, (void*)(server_srv + 0x004CAA00), (void*)Hooks::EmptyCall);
    HookFunctionInSharedObject(server_srv, server_srv_size, (void*)(server_srv + 0x008C9950), (void*)Hooks::EmptyCall);
    HookFunctionInSharedObject(server_srv, server_srv_size, (void*)(server_srv + 0x00AB1D20), (void*)Hooks::EmptyCall);
    HookFunctionInSharedObject(server_srv, server_srv_size, (void*)(server_srv + 0x00B66BC0), (void*)Hooks::HookInstaKill);


    HookFunctionInSharedObject(server_srv, server_srv_size, (void*)(server_srv + 0x008A39C0), (void*)Hooks::EmptyCall);
    //HookFunctionInSharedObject(server_srv, server_srv_size, (void*)(server_srv + 0x0070BD10), (void*)Hooks::EmptyCall);
    //HookFunctionInSharedObject(server_srv, server_srv_size, (void*)(server_srv + 0x004ED8F0), (void*)Hooks::EmptyCall);
    //HookFunctionInSharedObject(server_srv, server_srv_size, (void*)(server_srv + 0x00525F30), (void*)CalcPoseSingleHook);


    HookFunctionInSharedObject(server_srv, server_srv_size, (void*)(server_srv + 0x0057B840), (void*)Hooks::TakeDamageAliveHook);
    HookFunctionInSharedObject(server_srv, server_srv_size, (void*)(server_srv + 0x008835B0), (void*)Hooks::IRelationTypeHook);
    //HookFunctionInSharedObject(server_srv, server_srv_size, (void*)(server_srv + 0x0054CC80), (void*)Hooks::EmptyCall);

    HookFunctionInSharedObject(server_srv, server_srv_size, (void*)(server_srv + 0x0052B020), (void*)Hooks::EmptyCall);
    HookFunctionInSharedObject(server_srv, server_srv_size, (void*)(server_srv + 0x0052A7B0), (void*)Hooks::EmptyCall);
    HookFunctionInSharedObject(server_srv, server_srv_size, (void*)(server_srv + 0x008F55D0), (void*)Hooks::ClearEntitiesHook);
}

void DisableCacheCvars()
{
    pTwoArgProt pDynamicTwoArgFunc;
    pDynamicTwoArgFunc = (pTwoArgProt)(engine_srv + 0x001B1340);

    //pDynamicTwoArgFunc(0, (uint32_t)"mod_forcetouchdata 0");
    //pDynamicTwoArgFunc(0, (uint32_t)"mod_forcedata 0");
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
    //rootconsole->ConsolePrint("malloc() ref: [%X] size: [%X] list_size [%d]", newRef, size, MallocRefListSize(mallocAllocations));
    //rootconsole->ConsolePrint("malloc() ref: [%X] size: [%X]", newRef, size);

    /*void* returnAddr = __builtin_return_address(0);
    MallocRef* new_ref_value = CreateNewMallocRef((void*)newRef, (void*)enlarged_size, (void*)((uint32_t)returnAddr - 5), (void*)"malloc");
    InsertToMallocRefList(mallocAllocations, new_ref_value, true);*/

    return newRef;
}

uint32_t Hooks::MallocHook(uint32_t size)
{
    if(size <= 0) return (uint32_t)malloc(size);
    //if(size <= 8192) return (uint32_t)malloc(size*100.0);

    uint32_t newRef = (uint32_t)malloc(size*2.0);
    //rootconsole->ConsolePrint("malloc() ref: [%X] size: [%X] list_size [%d]", newRef, size, MallocRefListSize(mallocAllocations));
    //rootconsole->ConsolePrint("malloc() ref: [%X] size: [%X]", newRef, size);

    /*void* returnAddr = __builtin_return_address(0);
    MallocRef* new_ref_value = CreateNewMallocRef((void*)newRef, (void*)enlarged_size, (void*)((uint32_t)returnAddr - 5), (void*)"malloc");
    InsertToMallocRefList(mallocAllocations, new_ref_value, true);*/

    return newRef;
}

uint32_t Hooks::ReallocHook(uint32_t old_ptr, uint32_t new_size)
{
    if(new_size <= 0) return (uint32_t)realloc((void*)old_ptr, new_size);
    uint32_t new_ref = (uint32_t)realloc((void*)old_ptr, new_size*2.0);

    /*void* returnAddr = __builtin_return_address(0);
    RemoveAllocationRef(mallocAllocations, (void*)old_ptr, true);
    MallocRef* new_ref_value = CreateNewMallocRef((void*)new_ref, (void*)new_size, (void*)((uint32_t)returnAddr - 5), (void*)"malloc");
    InsertToMallocRefList(mallocAllocations, new_ref_value, true);*/

    return new_ref;
}

uint32_t Hooks::OperatorNewArrayHook(uint32_t size)
{
    if(size <= 0) return (uint32_t)malloc(size);
    uint32_t newRef = (uint32_t)malloc(size*2.0);
    //rootconsole->ConsolePrint("malloc() ref: [%X] size: [%X] list_size [%d]", newRef, size, MallocRefListSize(mallocAllocations));
    //rootconsole->ConsolePrint("malloc() ref: [%X] size: [%X]", newRef, size);

    /*void* returnAddr = __builtin_return_address(0);
    MallocRef* new_ref_value = CreateNewMallocRef((void*)newRef, (void*)size, (void*)((uint32_t)returnAddr - 5), (void*)"operator_new_array");
    InsertToMallocRefList(mallocAllocations, new_ref_value, true);*/

    return newRef;
}

uint32_t Hooks::HostChangelevelHook(uint32_t arg0, uint32_t arg1, uint32_t arg2)
{
    pOneArgProt pDynamicOneArgFunc;
    pTwoArgProt pDynamicTwoArgFunc;
    pThreeArgProt pDynamicThreeArgFunc;
    isTicking = false;

    /*uint32_t entity = 0;

    //NextHandle
    pDynamicTwoArgFunc = (pTwoArgProt)(server_srv + 0x008F37E0);

    while((entity = pDynamicTwoArgFunc(CGlobalEntityList, entity)) != 0)
    {
        char* clsname = (char*)(*(uint32_t*)(entity+0x64));
        rootconsole->ConsolePrint("Destroying vphysics [%s]", clsname);

        //Clear - EventQueue
        pDynamicOneArgFunc = (pOneArgProt)(server_srv + 0x008C88C0);
        pDynamicOneArgFunc(server_srv + 0x01869800);

        //VphysicsDestroyObject
        pDynamicOneArgFunc = (pOneArgProt)( *(uint32_t*)((*(uint32_t*)(entity))+0x2A0) );
        pDynamicOneArgFunc(entity);
    }*/

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

uint32_t Hooks::Util_RemoveHook(uint32_t arg0)
{
    pOneArgProt pDynamicOneArgFunc;
    if(arg0 == 0) return 0;

    uint32_t refHandle = *(uint32_t*)(arg0+0x334);
    uint32_t object_verify = GetCBaseEntity(refHandle);

    if(object_verify)
    {
        char* clsname = (char*)(*(uint32_t*)(object_verify+0x64));
        rootconsole->ConsolePrint("Destroying [%s]", clsname);

        //VphysicsDestroyObject
        pDynamicOneArgFunc = (pOneArgProt)( *(uint32_t*)((*(uint32_t*)(object_verify))+0x2A0) );
        pDynamicOneArgFunc(object_verify);

        rootconsole->ConsolePrint("Destroyed [%s]", clsname);

        //UTIL_Remove(CBaseEntity*)
        pDynamicOneArgFunc = (pOneArgProt)(server_srv + 0x00B66B70);
        return pDynamicOneArgFunc(object_verify);
    }

    rootconsole->ConsolePrint("Failed to verify entity object!");
    return 0;
}

uint32_t Hooks::TakeDamageAliveHook(uint32_t arg0, uint32_t arg1)
{
    pTwoArgProt pDynamicTwoArgFunc;

    if(arg1)
    {
        uint32_t chkRef = *(uint32_t*)(arg1+0x28);
        uint32_t object = GetCBaseEntity(chkRef);

        if(object)
        {
            pDynamicTwoArgFunc = (pTwoArgProt)(server_srv + 0x0057B840);
            return pDynamicTwoArgFunc(arg0, arg1);
        }
    }

    rootconsole->ConsolePrint("Fixed crash in take damage function");
    return 0;
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

    Hooks::CleanupDeleteListHook(0);

    pDynamicOneArgFunc = (pOneArgProt)(engine_srv + 0x001957B0);
    return pDynamicOneArgFunc(arg0);
}

uint32_t Hooks::GameFrameHook(uint32_t arg0)
{
    pOneArgProt pDynamicOneArgFunc;
    pThreeArgProt pDynamicThreeArgFunc;
    isTicking = true;

    Hooks::CleanupDeleteListHook(0);

    //SimulateEntities
    pDynamicOneArgFunc = (pOneArgProt)(server_srv + 0x00A7AC00);
    pDynamicOneArgFunc(arg0);

    Hooks::CleanupDeleteListHook(0);

    //ServiceEventQueue
    pDynamicOneArgFunc = (pOneArgProt)(server_srv + 0x008C9950);
    pDynamicOneArgFunc(0);

    Hooks::CleanupDeleteListHook(0);

    //StartFrame
    pDynamicOneArgFunc = (pOneArgProt)(server_srv + 0x006BD6F0);
    pDynamicOneArgFunc(0);

    Hooks::CleanupDeleteListHook(0);

    //UpdateClientData
    pDynamicOneArgFunc = (pOneArgProt)(server_srv + 0x00AB1D20);
    pDynamicOneArgFunc(0);

    Hooks::CleanupDeleteListHook(0);

    //PreSystems
    pDynamicOneArgFunc = (pOneArgProt)(server_srv + 0x004CA9E0);
    pDynamicOneArgFunc(0);

    Hooks::CleanupDeleteListHook(0);

    //PostSystems
    pDynamicOneArgFunc = (pOneArgProt)(server_srv + 0x004CAA00);
    pDynamicOneArgFunc(0);

    Hooks::CleanupDeleteListHook(0);
    return 0;
}

uint32_t Hooks::HookInstaKill(uint32_t arg0)
{
    pThreeArgProt pDynamicThreeArgFunc;
    pOneArgProt pDynamicOneArgFunc;

    char* clsname =  (char*) ( *(uint32_t*)(arg0+0x64) );

    if(isTicking)
    {
        rootconsole->ConsolePrint("fast killed [%s]", clsname);
    }

    pDynamicOneArgFunc = (pOneArgProt)(server_srv + 0x00B66BC0);
    return pDynamicOneArgFunc(arg0);
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
    char* clsname =  (char*) ( *(uint32_t*)(arg0+0x64) );
    uint32_t refHandle = *(uint32_t*)(arg0+0x334);

    //IsMarkedForDeletion
    pDynamicOneArgFunc = (pOneArgProt)(server_srv + 0x00B08580);
    uint32_t isMarked = pDynamicOneArgFunc(arg0+0x14);

    if(isMarked)
    {
        rootconsole->ConsolePrint("Attempted to simulate marked entity [%s]", clsname);
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

    pDynamicTwoArgFunc = (pTwoArgProt)(server_srv + 0x00942190);
    return pDynamicTwoArgFunc(arg0, arg1);
}

uint32_t Hooks::ClearEntitiesHook(uint32_t arg0)
{
    pOneArgProt pDynamicOneArgFunc;
    pTwoArgProt pDynamicTwoArgFunc;

    pDynamicOneArgFunc = (pOneArgProt)(server_srv + 0x008F55D0);
    return pDynamicOneArgFunc(arg0);
}