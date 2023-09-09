#include "ext_main.h"
#include "core.h"

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

    disable_delete_list = false;
    isTicking = false;
    player_spawned = false;
    hooked_delete_counter = 0;
    normal_delete_counter = 0;
    CGlobalEntityList = server_srv + 0x018711E0;
    deleteList = AllocateValuesList();
    server_sleeping = false;

    PopulateHookExclusionLists();
    ApplyPatches();
    DisableCacheCvars();
    HookFunctions();

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

    //uint32_t delete_list_call = server_srv + 0x00944F61;
    //memset((void*)delete_list_call, 0x90, 5);

    //uint32_t delete_list_call_sim = server_srv + 0x00A7AC57;
    //memset((void*)delete_list_call_sim, 0x90, 5);

    //delete_list_call = server_srv + 0x00944FC5;
    //memset((void*)delete_list_call, 0x90, 5);

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
    offset = (uint32_t)Hooks::VphysicsUpdateWarningHook - vphysicsupdatepatch - 5;
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
    offset = (uint32_t)Hooks::VphysicsUpdateWarningHook - vphysicsupdatepatch_two - 5;
    *(uint8_t*)(vphysicsupdatepatch_two) = 0xE8;
    *(uint32_t*)(vphysicsupdatepatch_two+1) = offset;
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

    rootconsole->ConsolePrint("patching malloc()");
    HookFunctionInSharedObject(server_srv, server_srv_size, (void*)malloc, (void*)Hooks::MallocHook);
    HookFunctionInSharedObject(engine_srv, engine_srv_size, (void*)malloc, (void*)Hooks::MallocHook);
    //HookFunctionInSharedObject(materialsystem_srv, materialsystem_srv_size, (void*)malloc, (void*)MallocHook);
    HookFunctionInSharedObject(vphysics_srv, vphysics_srv_size, (void*)malloc, (void*)Hooks::MallocHook);
    HookFunctionInSharedObject(dedicated_srv, dedicated_srv_size, (void*)malloc, (void*)Hooks::MallocHook);
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
    //HookFunctionInSharedObject(server_srv, server_srv_size, (void*)(server_srv + 0x00525F30), (void*)CalcPoseSingleHook);


    HookFunctionInSharedObject(server_srv, server_srv_size, (void*)(server_srv + 0x0062E0E0), (void*)Hooks::TakeDamageHook);
    HookFunctionInSharedObject(server_srv, server_srv_size, (void*)(server_srv + 0x008835B0), (void*)Hooks::IRelationTypeHook);
    //HookFunctionInSharedObject(server_srv, server_srv_size, (void*)(server_srv + 0x0054CC80), (void*)Hooks::EmptyCall);

    HookFunctionInSharedObject(server_srv, server_srv_size, (void*)(server_srv + 0x0052B020), (void*)Hooks::EmptyCall);
    HookFunctionInSharedObject(server_srv, server_srv_size, (void*)(server_srv + 0x0052A7B0), (void*)Hooks::EmptyCall);
    HookFunctionInSharedObject(server_srv, server_srv_size, (void*)(server_srv + 0x00644C00), (void*)Hooks::AcceptInputHook);
    HookFunctionInSharedObject(server_srv, server_srv_size, (void*)(server_srv + 0x008A7200), (void*)Hooks::CPropHevCharger_ShouldApplyEffect);
    HookFunctionInSharedObject(server_srv, server_srv_size, (void*)(server_srv + 0x008A7700), (void*)Hooks::CPropRadiationCharger_ShouldApplyEffect);
    HookFunctionInSharedObject(server_srv, server_srv_size, (void*)(server_srv + 0x00B01EE0), (void*)Hooks::ScriptThinkEntCheck);
    HookFunctionInSharedObject(server_srv, server_srv_size, (void*)(server_srv + 0x0064BE10), (void*)Hooks::UpdateOnRemove);
    HookFunctionInSharedObject(server_srv, server_srv_size, (void*)(server_srv + 0x006D6160), (void*)Hooks::PlayerSpawnHook);
    HookFunctionInSharedObject(server_srv, server_srv_size, (void*)(server_srv + 0x0074DA80), (void*)Hooks::CXenShieldController_UpdateOnRemoveHook);
    HookFunctionInSharedObject(server_srv, server_srv_size, (void*)(server_srv + 0x0082DFE0), (void*)Hooks::CNihiBallzDestructor);
    HookFunctionInSharedObject(server_srv, server_srv_size, (void*)(server_srv + 0x00A1F550), (void*)Hooks::InputApplySettingsHook);
    HookFunctionInSharedObject(server_srv, server_srv_size, (void*)(server_srv + 0x00905160), (void*)Hooks::InputSetCSMVolumeHook);
    HookFunctionInSharedObject(server_srv, server_srv_size, (void*)(server_srv + 0x00B08190), (void*)Hooks::HookFinalDeleteCall);
    HookFunctionInSharedObject(server_srv, server_srv_size, (void*)(server_srv + 0x00643FE0), (void*)Hooks::AbsolutePosHook);
    HookFunctionInSharedObject(server_srv, server_srv_size, (void*)(server_srv + 0x00A7CD50), (void*)Hooks::EnumElementHook);
    HookFunctionInSharedObject(server_srv, server_srv_size, (void*)(server_srv + 0x005B4EB0), (void*)Hooks::YawHook);
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
    return FindEntityByClassname(CGlobalEntityList, 0, (uint32_t)"player");
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
        rootconsole->ConsolePrint("Precache %s", arg0);
        
        //Precache
        pDynamicOneArgFunc = (pOneArgProt)(  *(uint32_t*)((*(uint32_t*)(piVar2))+0x68)  );
        pDynamicOneArgFunc(piVar2);

        InstaKill(piVar2, false);
    }

    return 0;
}

uint32_t Hooks::EnumElementHook(uint32_t arg0, uint32_t arg1)
{
    pTwoArgProt pDynamicTwoArgFunc;

    if(arg1 == 0) return 0;

    uint32_t real = GetCBaseEntity(*(uint32_t*)(arg1+0x334));

    if(real)
    {
        pDynamicTwoArgFunc = (pTwoArgProt)(server_srv + 0x00A7CD50);
        return pDynamicTwoArgFunc(arg0, arg1);
    }

    rootconsole->ConsolePrint("Attempted to use a dead object!");
    return 0;
}

uint32_t Hooks::AbsolutePosHook(uint32_t arg0)
{
    pOneArgProt pDynamicOneArgFunc;

    if(arg0 == 0) return 0;

    uint32_t real = GetCBaseEntity(*(uint32_t*)(arg0+0x334));

    if(real)
    {
        pDynamicOneArgFunc = (pOneArgProt)(server_srv + 0x00643FE0);
        return pDynamicOneArgFunc(arg0);
    }

    rootconsole->ConsolePrint("Attempted to use a dead object!");
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

uint32_t Hooks::ScriptThinkEntCheck(uint32_t arg0)
{
    pOneArgProt pDynamicOneArgFunc;

    pDynamicOneArgFunc = (pOneArgProt)(server_srv + 0x00B01EE0);
    uint32_t returnVal = pDynamicOneArgFunc(arg0);

    uint32_t refHandle = *(uint32_t*)(arg0+0x3B0);
    uint32_t chkRef = GetCBaseEntity(refHandle);

    if(!chkRef) return 0;
    return returnVal;
}

uint32_t Hooks::InputApplySettingsHook(uint32_t arg0, uint32_t arg1)
{
    pTwoArgProt pDynamicTwoArgFunc;

    uint32_t object = *(uint32_t*)(arg0+0x35C);

    if(object)
    {
        uint32_t object_chk = GetCBaseEntity(*(uint32_t*)(object+0x334));

        if(object_chk == 0)
        {
            *(uint32_t*)(arg0+0x35C) = 0;
        }
    }

    pDynamicTwoArgFunc = (pTwoArgProt)(server_srv + 0x00A1F550);
    return pDynamicTwoArgFunc(arg0, arg1);
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

uint32_t Hooks::HookFinalDeleteCall(uint32_t arg0)
{
    pOneArgProt pDynamicOneArgFunc;
    // DELETE THE PHYSICS OBJECT
    // CLEAR THE DEAD OBJECTS
    // DELETE THE ENTITY

    uint32_t object = *(uint32_t*)(arg0+8);
    if(object == 0) return 0;

    char* classname = (char*)(*(uint32_t*)(object+0x64));
    uint32_t refHandle = *(uint32_t*)(object+0x334);

    //VphysicsDestroyObject
    pDynamicOneArgFunc = (pOneArgProt)( *(uint32_t*)((*(uint32_t*)(object))+0x2A0) );
    pDynamicOneArgFunc(object);

    //Clean Phys
    pDynamicOneArgFunc = (pOneArgProt)(server_srv + 0x00A658D0);
    pDynamicOneArgFunc(0);

    //rootconsole->ConsolePrint("Removing! [%s]", *(uint32_t*)(object+0x64));

    //Delete Entity
    pDynamicOneArgFunc = (pOneArgProt)(server_srv + 0x00B08190);
    return pDynamicOneArgFunc(arg0);
}

uint32_t Hooks::VphysicsUpdateWarningHook(uint32_t arg0)
{
    pOneArgProt pDynamicOneArgFunc;

    rootconsole->ConsolePrint("Removing unreasonable entity [%s]", *(uint32_t*)(arg0+0x64));
    Hooks::UTIL_RemoveHook(arg0+0x14);
    return 0;
}

uint32_t Hooks::InputSetCSMVolumeHook(uint32_t arg0, uint32_t arg1)
{
    pTwoArgProt pDynamicTwoArgFunc;

    uint32_t fourth_offset = *(uint32_t*)(arg1+4);

    if(fourth_offset)
    {
        pDynamicTwoArgFunc = (pTwoArgProt)(server_srv + 0x00905160);
        return pDynamicTwoArgFunc(arg0, arg1);
    }

    rootconsole->ConsolePrint("Entity was NULL");
    return 0;
}

uint32_t Hooks::CNihiBallzDestructor(uint32_t arg0)
{
    pOneArgProt pDynamicOneArgFunc;

    uint32_t cbaseobject_one = *(uint32_t*)(arg0+0x72C);
    uint32_t cbaseobject_two = *(uint32_t*)(arg0+0x730);

    if(cbaseobject_one)
    {
        uint32_t refHandle_one = *(uint32_t*)(cbaseobject_one+0x334);
        uint32_t check_one = GetCBaseEntity(refHandle_one);

        if(check_one == 0)
        {
            *(uint32_t*)(arg0+0x72C) = 0;
        }
    }

    if(cbaseobject_two)
    {
        uint32_t refHandle_two = *(uint32_t*)(cbaseobject_two+0x334);
        uint32_t check_two = GetCBaseEntity(refHandle_two);

        if(check_two == 0)
        {
            *(uint32_t*)(arg0+0x730) = 0;
        }
    }

    pDynamicOneArgFunc = (pOneArgProt)(server_srv + 0x0082DFE0);
    return pDynamicOneArgFunc(arg0);
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

uint32_t Hooks::TakeDamageHook(uint32_t arg0, uint32_t arg1)
{
    pTwoArgProt pDynamicTwoArgFunc;

    if(arg1)
    {
        uint32_t chkRef = *(uint32_t*)(arg1+0x28);
        uint32_t object = GetCBaseEntity(chkRef);

        if(object)
        {
            pDynamicTwoArgFunc = (pTwoArgProt)(server_srv + 0x0062E0E0);
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

uint32_t Hooks::CPropHevCharger_ShouldApplyEffect(uint32_t arg0, uint32_t arg1)
{
    pTwoArgProt pDynamicTwoArgFunc;
    if(arg1 == 0) return 0;
    
    pDynamicTwoArgFunc = (pTwoArgProt)(server_srv + 0x008A7200);
    return pDynamicTwoArgFunc(arg0, arg1);
}

uint32_t Hooks::CPropRadiationCharger_ShouldApplyEffect(uint32_t arg0, uint32_t arg1)
{
    pTwoArgProt pDynamicTwoArgFunc;
    if(arg1 == 0) return 0;
    
    pDynamicTwoArgFunc = (pTwoArgProt)(server_srv + 0x008A7700);
    return pDynamicTwoArgFunc(arg0, arg1);
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

    //ServiceEventQueue
    pDynamicOneArgFunc = (pOneArgProt)(server_srv + 0x008C9950);
    uint32_t returnVal = pDynamicOneArgFunc(0);

    return returnVal;
}

uint32_t Hooks::SimulateEntitiesHook(uint32_t arg0)
{
    pOneArgProt pDynamicOneArgFunc;
    pThreeArgProt pDynamicThreeArgFunc;
    isTicking = true;

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

    //SimulateEntities
    pDynamicOneArgFunc = (pOneArgProt)(server_srv + 0x00A7AC00);
    pDynamicOneArgFunc(arg0);

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
    char* clsname =  (char*) ( *(uint32_t*)(arg0+0x64) );
    uint32_t refHandle = *(uint32_t*)(arg0+0x334);

    //IsMarkedForDeletion
    pDynamicOneArgFunc = (pOneArgProt)(server_srv + 0x00B08580);
    uint32_t isMarked = pDynamicOneArgFunc(arg0+0x14);

    if(isMarked)
    {
        rootconsole->ConsolePrint("Simulation ignored for [%s]", clsname);
        return 0;
    }

    if(server_sleeping && player_spawned)
    {
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

uint32_t Hooks::YawHook(uint32_t arg0)
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