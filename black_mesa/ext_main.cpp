#include "extension.h"
#include "util.h"
#include "core.h"
#include "ext_main.h"
#include "hooks_specific.h"

#include <math.h>

bool InitExtensionBlackMesa()
{
    if(loaded_extension)
    {
        rootconsole->ConsolePrint("Attempted to load extension twice!");
        return false;
    }

    InitCoreBlackMesa();
    AllowWriteToMappedMemory();

    char* root_dir = getenv("PWD");
    size_t max_path_length = 1024;

    char server_srv_fullpath[max_path_length];
    char engine_srv_fullpath[max_path_length];
    char materialsystem_srv_fullpath[max_path_length];
    char vphysics_srv_fullpath[max_path_length];
    char dedicated_srv_fullpath[max_path_length];
    char datacache_srv_fullpath[max_path_length];

    snprintf(server_srv_fullpath, max_path_length, "/bms/bin/server_srv.so");
    snprintf(engine_srv_fullpath, max_path_length, "/bin/engine_srv.so");
    snprintf(materialsystem_srv_fullpath, max_path_length, "/bin/materialsystem_srv.so");
    snprintf(vphysics_srv_fullpath, max_path_length, "/bin/vphysics_srv.so");
    snprintf(dedicated_srv_fullpath, max_path_length, "/bin/dedicated_srv.so");
    snprintf(datacache_srv_fullpath, max_path_length, "/bin/datacache_srv.so");

    Library* engine_srv_lib = FindLibrary(engine_srv_fullpath, true);
    Library* server_srv_lib = FindLibrary(server_srv_fullpath, true);
    Library* materialsystem_srv_lib = FindLibrary(materialsystem_srv_fullpath, true);
    Library* vphysics_srv_lib = FindLibrary(vphysics_srv_fullpath, true);
    Library* dedicated_srv_lib = FindLibrary(dedicated_srv_fullpath, true);
    Library* datacache_srv_lib = FindLibrary(datacache_srv_fullpath, true);

    if(!(engine_srv_lib && server_srv_lib && materialsystem_srv_lib && vphysics_srv_lib && dedicated_srv_lib && datacache_srv_lib))
    {
        ClearLoadedLibraries();
        RestoreMemoryProtections();
        rootconsole->ConsolePrint("----------------------  Failed to load Black Mesa " SMEXT_CONF_NAME " " SMEXT_CONF_VERSION "  ----------------------");
        return false;
    }

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

    CollisionRulesChanged = (pOneArgProt)(server_srv + 0x00294C60);
    FindEntityByClassname = (pThreeArgProt)(server_srv + 0x007E7030);
    SetSolidFlags = (pTwoArgProt)(server_srv + 0x00336C60);

    disable_delete_list = false;
    isTicking = false;
    player_spawned = false;
    hooked_delete_counter = 0;
    normal_delete_counter = 0;
    CGlobalEntityList = server_srv + 0x017B6BE0;
    global_vpk_cache_buffer = (uint32_t)malloc(0x00100000);
    current_vpk_buffer_ref = 0;
    server_sleeping = false;

    leakedResourcesVpkSystem = AllocateValuesList();

    offsets.classname_offset = 0x64;
    offsets.abs_origin_offset = 0x294;
    offsets.abs_angles_offset = 0x310;
    offsets.origin_offset = 0x31C;
    offsets.mnetwork_offset = 0x20;
    offsets.refhandle_offset = 0x334;
    offsets.iserver_offset = 0x14;

    functions.RemoveEntityNormal = (pTwoArgProt)(RemoveEntityNormalBlackMesa);
    functions.InstaKill = (pTwoArgProt)(InstaKillBlackMesa);
    functions.GetCBaseEntity = (pOneArgProt)(GetCBaseEntityBlackMesa);
    functions.IsMarkedForDeletion = (pOneArgProt)(server_srv + 0x00A2B520);

    PopulateHookExclusionListsBlackMesa();

    ApplyPatchesBlackMesa();
    ApplyPatchesSpecificBlackMesa();
    
    HookFunctionsBlackMesa();
    HookFunctionsSpecificBlackMesa();

    RestoreMemoryProtections();

    rootconsole->ConsolePrint("----------------------  Black Mesa " SMEXT_CONF_NAME " " SMEXT_CONF_VERSION " loaded!" "  ----------------------");
    loaded_extension = true;
    return true;
}

void ApplyPatchesBlackMesa()
{
    uint32_t offset = 0;

    uint32_t patch_vpk_cache_buffer = dedicated_srv + 0x000B57D2;
    memset((void*)patch_vpk_cache_buffer, 0x90, 0x17);
    *(uint8_t*)(patch_vpk_cache_buffer) = 0x89;
    *(uint8_t*)(patch_vpk_cache_buffer+1) = 0x34;
    *(uint8_t*)(patch_vpk_cache_buffer+2) = 0x24;

    patch_vpk_cache_buffer = dedicated_srv + 0x000B57D2+3;
    offset = (uint32_t)HooksBlackMesa::VpkCacheBufferAllocHook - patch_vpk_cache_buffer - 5;
    *(uint8_t*)(patch_vpk_cache_buffer) = 0xE8;
    *(uint32_t*)(patch_vpk_cache_buffer+1) = offset;

    uint32_t force_jump_vpk_allocation = dedicated_srv + 0x000B5736;
    memset((void*)force_jump_vpk_allocation, 0x90, 6);
    *(uint8_t*)(force_jump_vpk_allocation) = 0xE9;
    *(uint32_t*)(force_jump_vpk_allocation+1) = 0x97;

    uint32_t hook_game_frame_delete_list = server_srv + 0x008404F3;
    offset = (uint32_t)HooksBlackMesa::SimulateEntitiesHook - hook_game_frame_delete_list - 5;
    *(uint32_t*)(hook_game_frame_delete_list+1) = offset;

    uint32_t eventqueue_hook = server_srv + 0x008404FD;
    offset = (uint32_t)HooksBlackMesa::ServiceEventQueueHook - eventqueue_hook - 5;
    *(uint32_t*)(eventqueue_hook+1) = offset;

    uint32_t fix_vphysics_pair_crash = server_srv + 0x00378906;
    *(uint8_t*)(fix_vphysics_pair_crash) = 0xEB;

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

void HookFunctionsBlackMesa()
{
    HookFunctionInSharedObject(server_srv, server_srv_size, (void*)(server_srv + 0x008B4C80), (void*)HooksBlackMesa::CreateEntityByNameHook);
    HookFunctionInSharedObject(server_srv, server_srv_size, (void*)(server_srv + 0x007E6D20), (void*)HooksBlackMesa::CleanupDeleteListHook);
    HookFunctionInSharedObject(server_srv, server_srv_size, (void*)(server_srv + 0x00A92160), (void*)HooksBlackMesa::UTIL_RemoveHook);
    HookFunctionInSharedObject(engine_srv, engine_srv_size, (void*)(engine_srv + 0x000E6D70), (void*)HooksBlackMesa::HostChangelevelHook);
    HookFunctionInSharedObject(server_srv, server_srv_size, (void*)(server_srv + 0x009919D0), (void*)HooksBlackMesa::PhysSimEnt);
    HookFunctionInSharedObject(server_srv, server_srv_size, (void*)(server_srv + 0x00A92260), (void*)HooksBlackMesa::HookInstaKill);

    //RagdollBreakingDisable
    HookFunctionInSharedObject(server_srv, server_srv_size, (void*)(server_srv + 0x0078FC70), (void*)HooksBlackMesa::EmptyCall);

    HookFunctionInSharedObject(server_srv, server_srv_size, (void*)(server_srv + 0x004F5B50), (void*)HooksBlackMesa::AcceptInputHook);
    HookFunctionInSharedObject(server_srv, server_srv_size, (void*)(server_srv + 0x004FD670), (void*)HooksBlackMesa::UpdateOnRemove);
    HookFunctionInSharedObject(server_srv, server_srv_size, (void*)(server_srv + 0x005983C0), (void*)HooksBlackMesa::PlayerSpawnHook);
    HookFunctionInSharedObject(server_srv, server_srv_size, (void*)(server_srv + 0x0061B4C0), (void*)HooksBlackMesa::CXenShieldController_UpdateOnRemoveHook);
    HookFunctionInSharedObject(server_srv, server_srv_size, (void*)(server_srv + 0x00A95FF0), (void*)HooksBlackMesa::UTIL_PrecacheOther_Hook);
    HookFunctionInSharedObject(server_srv, server_srv_size, (void*)(server_srv + 0x00A92540), (void*)HooksBlackMesa::UTIL_GetLocalPlayerHook);
    HookFunctionInSharedObject(server_srv, server_srv_size, (void*)(server_srv + 0x0046B510), (void*)HooksBlackMesa::TestGroundMove);
    HookFunctionInSharedObject(server_srv, server_srv_size, (void*)(server_srv + 0x00294D00), (void*)HooksBlackMesa::VPhysicsSetObjectHook);
    HookFunctionInSharedObject(server_srv, server_srv_size, (void*)(server_srv + 0x00A02D40), (void*)HooksBlackMesa::ShouldHitEntityHook);
    HookFunctionInSharedObject(server_srv, server_srv_size, (void*)(server_srv + 0x00294C60), (void*)HooksBlackMesa::CollisionRulesChangedHook);
    HookFunctionInSharedObject(dedicated_srv, dedicated_srv_size, (void*)(dedicated_srv + 0x000B5460), (void*)HooksBlackMesa::CanSatisfyVpkCacheHook);
    HookFunctionInSharedObject(dedicated_srv, dedicated_srv_size, (void*)(dedicated_srv + 0x000B1AE0), (void*)HooksBlackMesa::PackedStoreDestructorHook);
}

uint32_t HooksBlackMesa::CanSatisfyVpkCacheHook(uint32_t arg0, uint32_t arg1, uint32_t arg2, uint32_t arg3, uint32_t arg4, uint32_t arg5, uint32_t arg6)
{
    pOneArgProt pDynamicOneArgFunc;
    pSevenArgProt pDynamicSevenArgFunc;

    pDynamicSevenArgFunc = (pSevenArgProt)(dedicated_srv + 0x000B5460);
    uint32_t returnVal = pDynamicSevenArgFunc(arg0, arg1, arg2, arg3, arg4, arg5, arg6);

    if(current_vpk_buffer_ref)
    {
        uint32_t allocated_vpk_buffer = *(uint32_t*)(current_vpk_buffer_ref+0x10);

        if(allocated_vpk_buffer && global_vpk_cache_buffer == allocated_vpk_buffer)
        {
            //rootconsole->ConsolePrint("Removed global vpk buffer from VPK tree!");
            *(uint32_t*)(current_vpk_buffer_ref+0x10) = 0;
        }
        else
        {
            rootconsole->ConsolePrint("Failed to remove global vpk buffer!!!");
            exit(1);
        }

        current_vpk_buffer_ref = 0;
    }

    return returnVal;
}

uint32_t HooksBlackMesa::PackedStoreDestructorHook(uint32_t arg0)
{
    //Remove ref to store only valid objects!
    pOneArgProt pDynamicOneArgFunc;

    pDynamicOneArgFunc = (pOneArgProt)(dedicated_srv + 0x000B1AE0);
    uint32_t returnVal = pDynamicOneArgFunc(arg0);

    Value* a_leak = *leakedResourcesVpkSystem;

    while(a_leak)
    {
        VpkMemoryLeak* the_leak = (VpkMemoryLeak*)(a_leak->value);
        uint32_t packed_object = the_leak->packed_ref;

        if(packed_object == arg0)
        {
            ValueList vpk_leak_list = the_leak->leaked_refs;
            
            int removed_items = DeleteAllValuesInList(vpk_leak_list, true, NULL);

            rootconsole->ConsolePrint("[VPK Hook] released [%d] memory leaks!", removed_items);

            bool success = RemoveFromValuesList(leakedResourcesVpkSystem, the_leak, NULL);

            free(vpk_leak_list);
            free(the_leak);

            if(!success)
            {
                rootconsole->ConsolePrint("[VPK Hook] Expected to remove leak but failed!");
                exit(EXIT_FAILURE);
            }

            return returnVal;
        }

        a_leak = a_leak->nextVal;
    }

    return returnVal;
}

uint32_t HooksBlackMesa::VpkCacheBufferAllocHook(uint32_t arg0)
{
    uint32_t ebp = 0;
    asm volatile ("movl %%ebp, %0" : "=r" (ebp));

    uint32_t arg0_return = *(uint32_t*)(ebp-4);
    uint32_t packed_store_ref = arg0_return-0x228;

    uint32_t vpk_buffer = *(uint32_t*)(arg0+0x10);

    if(vpk_buffer == 0)
    {
        current_vpk_buffer_ref = arg0;
        return global_vpk_cache_buffer;
    }

    bool saved_reference = false;

    Value* a_leak = *leakedResourcesVpkSystem;

    while(a_leak)
    {
        VpkMemoryLeak* the_leak = (VpkMemoryLeak*)(a_leak->value);
        uint32_t packed_object = the_leak->packed_ref;

        if(packed_object == packed_store_ref)
        {
            saved_reference = true;

            ValueList vpk_leak_list = the_leak->leaked_refs;

            Value* new_vpk_leak = CreateNewValue((void*)(vpk_buffer));
            bool added = InsertToValuesList(vpk_leak_list, new_vpk_leak, NULL, false, true);

            if(added)
            {
                rootconsole->ConsolePrint("[VPK Hook] " HOOK_MSG, vpk_buffer);
            }

            break;
        }

        a_leak = a_leak->nextVal;
    }

    if(!saved_reference)
    {
        VpkMemoryLeak* omg_leaks = (VpkMemoryLeak*)(malloc(sizeof(VpkMemoryLeak)));
        ValueList empty_list = AllocateValuesList();

        Value* original_vpk_buffer = CreateNewValue((void*)vpk_buffer);
        InsertToValuesList(empty_list, original_vpk_buffer, NULL, false, false);

        omg_leaks->packed_ref = packed_store_ref;
        omg_leaks->leaked_refs = empty_list;

        Value* leaked_resource = CreateNewValue((void*)omg_leaks);
        InsertToValuesList(leakedResourcesVpkSystem, leaked_resource, NULL, false, false);

        rootconsole->ConsolePrint("[VPK Hook First] " HOOK_MSG, vpk_buffer);
    }

    return vpk_buffer;
}

uint32_t HooksBlackMesa::CollisionRulesChangedHook(uint32_t arg0)
{
    InsertEntityToCollisionsList(arg0);
    return 0;
}

uint32_t HooksBlackMesa::VPhysicsSetObjectHook(uint32_t arg0, uint32_t arg1)
{
    pOneArgProt pDynamicOneArgFunc;
    pTwoArgProt pDynamicTwoArgFunc;

    uint32_t vphysics_object = *(uint32_t*)(arg0+0x1F8);

    if(vphysics_object)
    {
        rootconsole->ConsolePrint("Attempting override existing vphysics object!!!!");
        return 0;
    }

    *(uint32_t*)(arg0+0x1F8) = arg1;
    return 0;
}

uint32_t HooksBlackMesa::ShouldHitEntityHook(uint32_t arg0, uint32_t arg1, uint32_t arg2)
{
    pOneArgProt pDynamicOneArgFunc;
    pThreeArgProt pDynamicThreeArgFunc;

    if(arg1)
    {
        pDynamicOneArgFunc = (pOneArgProt)( *(uint32_t*)((*(uint32_t*)(arg1))+0x18) );
        uint32_t object = pDynamicOneArgFunc(arg1);

        if(IsEntityValid(object))
        {
            uint32_t vphysics_object = *(uint32_t*)(object+0x1F8);

            if(vphysics_object)
            {
                pDynamicThreeArgFunc = (pThreeArgProt)(server_srv + 0x00A02D40);
                return pDynamicThreeArgFunc(arg0, arg1, arg2);
            }
        }
    }

    rootconsole->ConsolePrint("ShouldHitEntity failed!");
    return 0;
}

uint32_t HooksBlackMesa::UTIL_GetLocalPlayerHook()
{
    return FindEntityByClassname(CGlobalEntityList, 0, (uint32_t)"player");
}

uint32_t HooksBlackMesa::UTIL_PrecacheOther_Hook(uint32_t arg0, uint32_t arg1)
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

        InstaKillBlackMesa(piVar2, false);
    }

    return 0;
}

uint32_t HooksBlackMesa::UpdateOnRemove(uint32_t arg0)
{
    pOneArgProt pDynamicOneArgFunc;

    normal_delete_counter++;

    pDynamicOneArgFunc = (pOneArgProt)(server_srv + 0x004FD670);
    return pDynamicOneArgFunc(arg0);
}

uint32_t HooksBlackMesa::EmptyCall()
{
    return 0;
}

uint32_t HooksBlackMesa::CallocHook(uint32_t nitems, uint32_t size)
{
    if(nitems <= 0) return (uint32_t)calloc(nitems, size);
    uint32_t enlarged_size = nitems*2.0;
    uint32_t newRef = (uint32_t)calloc(enlarged_size, size);
    return newRef;
}

uint32_t HooksBlackMesa::MallocHook(uint32_t size)
{
    if(size <= 0) return (uint32_t)malloc(size);
    uint32_t newRef = (uint32_t)malloc(size*2.0);
    return newRef;
}

uint32_t HooksBlackMesa::ReallocHook(uint32_t old_ptr, uint32_t new_size)
{
    if(new_size <= 0) return (uint32_t)realloc((void*)old_ptr, new_size);
    uint32_t new_ref = (uint32_t)realloc((void*)old_ptr, new_size*2.0);
    return new_ref;
}

uint32_t HooksBlackMesa::OperatorNewArrayHook(uint32_t size)
{
    if(size <= 0) return (uint32_t)malloc(size);
    uint32_t newRef = (uint32_t)malloc(size*2.0);
    return newRef;
}

uint32_t HooksBlackMesa::HostChangelevelHook(uint32_t arg0, uint32_t arg1, uint32_t arg2)
{
    pOneArgProt pDynamicOneArgFunc;
    pTwoArgProt pDynamicTwoArgFunc;
    pThreeArgProt pDynamicThreeArgFunc;

    LogVpkMemoryLeaks();

    isTicking = false;
    player_spawned = false;

    pDynamicThreeArgFunc = (pThreeArgProt)(engine_srv + 0x000E6D70);
    return pDynamicThreeArgFunc(arg0, arg1, arg2);
}

uint32_t HooksBlackMesa::CleanupDeleteListHook(uint32_t arg0)
{
    pOneArgProt pDynamicOneArgFunc;
    if(disable_delete_list) return 0;

    pDynamicOneArgFunc = (pOneArgProt)(server_srv + 0x007E6D20);
    return pDynamicOneArgFunc(0);
}

uint32_t HooksBlackMesa::UTIL_RemoveHook(uint32_t arg0)
{
    if(arg0 == 0)
    {
        rootconsole->ConsolePrint("Remove was NULL");
        return 0;
    }
    
    uint32_t cbaseobject = arg0-0x14;
    RemoveEntityNormalBlackMesa(cbaseobject, true);
    return 0;
}

uint32_t HooksBlackMesa::CXenShieldController_UpdateOnRemoveHook(uint32_t arg0)
{
    pOneArgProt pDynamicOneArgFunc;

    pDynamicOneArgFunc = (pOneArgProt)(server_srv + 0x0061B4C0);
    uint32_t returnVal = pDynamicOneArgFunc(arg0);

    //Add missing call to UpdateOnRemove
    HooksBlackMesa::UpdateOnRemove(arg0);

    return returnVal;
}

uint32_t HooksBlackMesa::PlayerSpawnHook(uint32_t arg0)
{
    pOneArgProt pDynamicOneArgFunc;

    pDynamicOneArgFunc = (pOneArgProt)(server_srv + 0x005983C0);
    uint32_t returnVal = pDynamicOneArgFunc(arg0);

    player_spawned = true;

    return returnVal;
}

uint32_t HooksBlackMesa::ServiceEventQueueHook()
{
    pOneArgProt pDynamicOneArgFunc;
    return 0;
}

uint32_t HooksBlackMesa::SimulateEntitiesHook(uint32_t arg0)
{
    pOneArgProt pDynamicOneArgFunc;
    pThreeArgProt pDynamicThreeArgFunc;
    isTicking = true;

    HooksBlackMesa::CleanupDeleteListHook(0);

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

    HooksBlackMesa::CleanupDeleteListHook(0);

    CheckForLocation();

    HooksBlackMesa::CleanupDeleteListHook(0);

    //SimulateEntities
    pDynamicOneArgFunc = (pOneArgProt)(server_srv + 0x00991F80);
    pDynamicOneArgFunc(arg0);

    UpdateCollisionsForMarkedEntities();

    HooksBlackMesa::CleanupDeleteListHook(0);

    //ServiceEventQueue
    pDynamicOneArgFunc = (pOneArgProt)(server_srv + 0x007B92B0);
    pDynamicOneArgFunc(0);

    UpdateCollisionsForMarkedEntities();

    HooksBlackMesa::CleanupDeleteListHook(0);

    UpdateAllCollisions();

    HooksBlackMesa::CleanupDeleteListHook(0);

    RemoveBadEnts();

    HooksBlackMesa::CleanupDeleteListHook(0);

    //PostSystems
    pDynamicOneArgFunc = (pOneArgProt)(server_srv + 0x0035C740);
    pDynamicOneArgFunc(0);

    HooksBlackMesa::CleanupDeleteListHook(0);

    return 0;
}

uint32_t HooksBlackMesa::HookInstaKill(uint32_t arg0)
{
    InstaKillBlackMesa(arg0, true);
    return 0;
}

uint32_t HooksBlackMesa::CreateEntityByNameHook(uint32_t arg0, uint32_t arg1)
{
    pTwoArgProt pDynamicTwoArgFunc;
    pThreeArgProt pDynamicThreeArgFunc;

    pDynamicTwoArgFunc = (pTwoArgProt)(server_srv + 0x008B4C80);
    return pDynamicTwoArgFunc(arg0, arg1);
}

uint32_t HooksBlackMesa::PhysSimEnt(uint32_t arg0)
{
    pOneArgProt pDynamicOneArgFunc;

    if(arg0 == 0)
    {
        rootconsole->ConsolePrint("Passed NULL simulation entity!");
        exit(EXIT_FAILURE);
        return 0;
    }

    uint32_t refHandle = *(uint32_t*)(arg0+0x334);
    uint32_t object_check = GetCBaseEntityBlackMesa(refHandle);

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

uint32_t HooksBlackMesa::AcceptInputHook(uint32_t arg0, uint32_t arg1, uint32_t arg2, uint32_t arg3, uint32_t arg4, uint32_t arg5)
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

uint32_t HooksBlackMesa::TestGroundMove(uint32_t arg0, uint32_t arg1, uint32_t arg2, uint32_t arg3, uint32_t arg4, uint32_t arg5, uint32_t arg6)
{
    pSevenArgProt pDynamicSevenArgProt;

    if(arg6)
    {
        float inf_val_chk = *(float*)(arg6+0x20);

        if(isinf(inf_val_chk))
        {
            rootconsole->ConsolePrint("+Inf detected!");
            return 0;
        }
    }

    pDynamicSevenArgProt = (pSevenArgProt)(server_srv + 0x0046B510);
    return pDynamicSevenArgProt(arg0, arg1, arg2, arg3, arg4, arg5, arg6);
}