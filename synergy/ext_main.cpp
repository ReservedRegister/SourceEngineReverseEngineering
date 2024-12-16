#include "extension.h"
#include "util.h"
#include "core.h"
#include "ext_main.h"
#include "hooks_specific.h"

bool InitExtensionSynergy()
{
    if(loaded_extension)
    {
        rootconsole->ConsolePrint("Attempted to load extension twice!");
        return false;
    }

    InitCoreSynergy();
    AllowWriteToMappedMemory();

    char* root_dir = getenv("PWD");
    const size_t max_path_length = 1024;

    char server_fullpath[max_path_length];
    char engine_srv_fullpath[max_path_length];
    char dedicated_srv_fullpath[max_path_length];
    char vphysics_srv_fullpath[max_path_length];
    char sdktools_path[max_path_length];

    snprintf(server_fullpath, max_path_length, "/synergy/bin/server.so");
    snprintf(engine_srv_fullpath, max_path_length, "/bin/engine_srv.so");
    snprintf(dedicated_srv_fullpath, max_path_length, "/bin/dedicated_srv.so");
    snprintf(vphysics_srv_fullpath, max_path_length, "/bin/vphysics_srv.so");
    snprintf(sdktools_path, max_path_length, "/extensions/sdktools.ext.2.sdk2013.so");

    Library* engine_srv_lib = FindLibrary(engine_srv_fullpath, true);
    Library* dedicated_srv_lib = FindLibrary(dedicated_srv_fullpath, true);
    Library* vphysics_srv_lib = FindLibrary(vphysics_srv_fullpath, true);
    Library* server_lib = FindLibrary(server_fullpath, true);
    Library* sdktools_lib = FindLibrary(sdktools_path, true);

    if(!(engine_srv_lib && dedicated_srv_lib && vphysics_srv_lib &&
    server_lib && sdktools_lib))
    {
        ClearLoadedLibraries();
        RestoreMemoryProtections();
        rootconsole->ConsolePrint("----------------------  Failed to load Synergy " SMEXT_CONF_NAME " " SMEXT_CONF_VERSION "  ----------------------");
        return false;
    }

    rootconsole->ConsolePrint("engine_srv_lib [%X] size [%X]", engine_srv_lib->library_base_address, engine_srv_lib->library_size);
    rootconsole->ConsolePrint("dedicated_srv_lib [%X] size [%X]", dedicated_srv_lib->library_base_address, dedicated_srv_lib->library_size);
    rootconsole->ConsolePrint("vphysics_srv_lib [%X] size [%X]", vphysics_srv_lib->library_base_address, vphysics_srv_lib->library_size);
    rootconsole->ConsolePrint("server_lib [%X] size [%X]", server_lib->library_base_address, server_lib->library_size);
    rootconsole->ConsolePrint("sdktools_lib [%X] size [%X]", sdktools_lib->library_base_address, sdktools_lib->library_size);

    engine_srv = engine_srv_lib->library_base_address;
    dedicated_srv = dedicated_srv_lib->library_base_address;
    vphysics_srv = vphysics_srv_lib->library_base_address;
    server = server_lib->library_base_address;
    sdktools = sdktools_lib->library_base_address;

    engine_srv_size = engine_srv_lib->library_size;
    dedicated_srv_size = dedicated_srv_lib->library_size;
    vphysics_srv_size = vphysics_srv_lib->library_size;
    server_size = server_lib->library_size;
    sdktools_size = sdktools_lib->library_size;

    sdktools_passed = IsAllowedToPatchSdkTools(sdktools, sdktools_size);

    RestoreLinkedLists();
    SaveProcessId();

    fields.CGlobalEntityList = server + 0x00EA9E7C;
    fields.sv = engine_srv + 0x0099F470;

    offsets.classname_offset = 0x74;
    offsets.abs_origin_offset = 0x2A4;
    offsets.abs_angles_offset = 0x338;
    offsets.abs_velocity_offset = 0x23C;
    offsets.origin_offset = 0x344;
    offsets.mnetwork_offset = 0x30;
    offsets.refhandle_offset = 0x35C;
    offsets.iserver_offset = 0x24;
    offsets.collision_property_offset = 0x170;
    offsets.m_CollisionGroup_offset = 516;

    functions.RemoveEntityNormal = (pTwoArgProt)(RemoveEntityNormalSynergy);
    functions.InstaKill = (pTwoArgProt)(InstaKillSynergy);
    functions.GetCBaseEntity = (pOneArgProt)(GetCBaseEntitySynergy);
    functions.IsMarkedForDeletion = (pOneArgProt)(server + 0x0083DEE0);
    functions.SetSolidFlags = (pTwoArgProt)(server + 0x00615450);
    functions.DisableEntityCollisions = (pTwoArgProt)(server + 0x0077AEA0);
    functions.EnableEntityCollisions = (pTwoArgProt)(server + 0x0077B000);
    functions.CollisionRulesChanged = (pOneArgProt)(server + 0x005CFE60);
    functions.FindEntityByClassname = (pThreeArgProt)(server + 0x0064AE30);

    PopulateHookExclusionListsSynergy();

    ApplyPatchesSynergy();
    ApplyPatchesSpecificSynergy();

    HookFunctionsSynergy();
    HookFunctionsSpecificSynergy();

    RestoreMemoryProtections();

    rootconsole->ConsolePrint("\n\nServer Map: [%s]\n\n", fields.sv+0x11);
    rootconsole->ConsolePrint("----------------------  Synergy " SMEXT_CONF_NAME " " SMEXT_CONF_VERSION " loaded  ----------------------");
    loaded_extension = true;

    return true;
}

void ApplyPatchesSynergy()
{
    uint32_t offset = 0;

    uint32_t hook_dedicated_vpk_malloc = dedicated_srv + 0x000C81D4;
    offset = (uint32_t)HooksSynergy::DirectMallocHookDedicatedSrv - hook_dedicated_vpk_malloc - 5;
    *(uint32_t*)(hook_dedicated_vpk_malloc+1) = offset;

    uint32_t patch_stack_vpk_cache_allocation = dedicated_srv + 0x000C81CD;
    memset((void*)patch_stack_vpk_cache_allocation, 0x90, 7);

    *(uint8_t*)(patch_stack_vpk_cache_allocation) = 0x89;
    *(uint8_t*)(patch_stack_vpk_cache_allocation+1) = 0x34;
    *(uint8_t*)(patch_stack_vpk_cache_allocation+2) = 0x24;

    uint32_t force_jump_vpk_allocation = dedicated_srv + 0x000C80B3;
    memset((void*)force_jump_vpk_allocation, 0x90, 6);

    *(uint8_t*)(force_jump_vpk_allocation) = 0xE9;
    *(uint32_t*)(force_jump_vpk_allocation+1) = 0x112;

    if(sdktools_passed)
    {
        memset((void*)(sdktools + 0x00016903), 0x90, 2);
        memset((void*)(sdktools + 0x00016907), 0x90, 2);
    }

    uint32_t hook_game_frame = server + 0x006B0CD4;
    offset = (uint32_t)HooksSynergy::SimulateEntitiesHook - hook_game_frame - 5;
    *(uint32_t*)(hook_game_frame+1) = offset;

    uint32_t hook_service_event_queue = server + 0x006B0CFA;
    offset = (uint32_t)HooksSynergy::ServiceEventQueueHook - hook_service_event_queue - 5;
    *(uint32_t*)(hook_service_event_queue+1) = offset;


    // TEMP FIX
    // -----------------------------
    uint32_t remove_broken_call_player_spawn = server + 0x00C2A57F;
    memset((void*)remove_broken_call_player_spawn, 0x90, 5);

    *(uint8_t*)(remove_broken_call_player_spawn) = 0x31;
    *(uint8_t*)(remove_broken_call_player_spawn+1) = 0xC0;
    //------------------------------


    //CMessageEntity
    uint32_t remove_extra_call = server + 0x00705D5B;
    memset((void*)remove_extra_call, 0x90, 5);
}

uint32_t HooksSynergy::EmptyCall()
{
    return 0;
}

uint32_t HooksSynergy::EmptyCallStdCall(uint32_t arg0)
{
    return 0;
}

uint32_t HooksSynergy::UTIL_RemoveHookFailsafe(uint32_t arg0)
{
    // THIS IS UTIL_Remove(IServerNetworable*)
    // THIS HOOK IS FOR UNUSUAL CALLS TO UTIL_Remove probably from sourcemod!

    if(arg0 == 0) return 0;
    functions.RemoveEntityNormal(arg0-offsets.iserver_offset, true);
    return 0;
}

uint32_t HooksSynergy::CleanupDeleteListHook(uint32_t arg0)
{
    pOneArgProt pDynamicOneArgFunc;

    pDynamicOneArgFunc = (pOneArgProt)(server + 0x0064A870);
    return pDynamicOneArgFunc(arg0);
}

uint32_t HooksSynergy::CallocHook(uint32_t nitems, uint32_t size)
{
    if(nitems <= 0) return (uint32_t)calloc(nitems, size);

    uint32_t enlarged_size = nitems*2.5;
    return (uint32_t)calloc(enlarged_size, size);
}

uint32_t HooksSynergy::MallocHookSmall(uint32_t size)
{
    if(size <= 0) return (uint32_t)malloc(size);
    
    return (uint32_t)malloc(size*1.2);
}

uint32_t HooksSynergy::MallocHookLarge(uint32_t size)
{
    if(size <= 0) return (uint32_t)malloc(size);

    return (uint32_t)malloc(size*2.2);
}

uint32_t HooksSynergy::OperatorNewHook(uint32_t size)
{
    if(size <= 0) return (uint32_t)operator new(size);

    return (uint32_t)operator new(size*1.4);
}

uint32_t HooksSynergy::OperatorNewArrayHook(uint32_t size)
{
    if(size <= 0) return (uint32_t)operator new[](size);

    return (uint32_t)operator new[](size*3.0);
}

uint32_t HooksSynergy::ReallocHook(uint32_t old_ptr, uint32_t new_size)
{
    if(new_size <= 0) return (uint32_t)realloc((void*)old_ptr, new_size);

    return (uint32_t)realloc((void*)old_ptr, new_size*1.2);
}

uint32_t HooksSynergy::UpdateOnRemove(uint32_t arg0)
{
    pOneArgProt pDynamicOneArgFunc;

    char* classname = (char*)(*(uint32_t*)(arg0+offsets.classname_offset));

    if(*(uint32_t*)(server + 0x00F3A570) != 0)
    {
        normal_delete_counter++;
    }

    //rootconsole->ConsolePrint("normal counter: [%d] [%d] [%s]", normal_delete_counter, hooked_delete_counter, classname);

    pDynamicOneArgFunc = (pOneArgProt)(server + 0x005AEC70);
    return pDynamicOneArgFunc(arg0);
}

uint32_t HooksSynergy::DirectMallocHookDedicatedSrv(uint32_t arg0)
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

uint32_t HooksSynergy::PackedStoreDestructorHook(uint32_t arg0)
{
    //Remove ref to store only valid objects!
    pOneArgProt pDynamicOneArgFunc;

    pDynamicOneArgFunc = (pOneArgProt)(dedicated_srv + 0x000C4B70);
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

uint32_t HooksSynergy::PhysSimEnt(uint32_t arg0)
{
    pOneArgProt pDynamicOneArgFunc;

    if(arg0 == 0)
    {
        rootconsole->ConsolePrint("Passed NULL simulation entity!");
        exit(EXIT_FAILURE);
        return 0;
    }

    uint32_t sim_ent_ref = *(uint32_t*)(arg0+offsets.refhandle_offset);
    uint32_t object_check = functions.GetCBaseEntity(sim_ent_ref);

    if(object_check == 0)
    {
        rootconsole->ConsolePrint("Passed in non-existant simulation entity!");
        exit(EXIT_FAILURE);
        return 0;
    }

    char* clsname =  (char*) ( *(uint32_t*)(arg0+offsets.classname_offset) );
    uint32_t isMarked = functions.IsMarkedForDeletion(arg0+offsets.iserver_offset);

    if(isMarked)
    {
        rootconsole->ConsolePrint("Simulation ignored for [%s]", clsname);
        return 0;
    }

    pDynamicOneArgFunc = (pOneArgProt)(server + 0x0074CFD0);
    return pDynamicOneArgFunc(arg0);
}

uint32_t HooksSynergy::CreateEntityByNameHook(uint32_t arg0, uint32_t arg1)
{
    pTwoArgProt pDynamicTwoArgFunc;

    pDynamicTwoArgFunc = (pTwoArgProt)(server + 0x006FF1E0);
    return pDynamicTwoArgFunc(arg0, arg1);
}

uint32_t HooksSynergy::HookInstaKill(uint32_t arg0)
{
    functions.InstaKill(arg0, true);
    return 0;
}

uint32_t HooksSynergy::ServiceEventQueueHook(uint32_t arg0)
{
    pOneArgProt pDynamicOneArgFunc;

    HooksSynergy::CleanupDeleteListHook(0);

    pDynamicOneArgFunc = (pOneArgProt)(server + 0x006076C0);
    pDynamicOneArgFunc(arg0);

    HooksSynergy::CleanupDeleteListHook(0);
    return 0;
}

uint32_t HooksSynergy::SimulateEntitiesHook(uint8_t simulating)
{
    isTicking = true;

    pZeroArgProt pDynamicZeroArgFunc;
    pOneArgProt pDynamicOneArgFunc;

    HooksSynergy::CleanupDeleteListHook(0);

    if(hooked_delete_counter == normal_delete_counter)
    {
        hooked_delete_counter = 0;
        normal_delete_counter = 0;
    }
    else
    {

        rootconsole->ConsolePrint("Final counter: [%d] [%d]", normal_delete_counter, hooked_delete_counter);
        rootconsole->ConsolePrint("Critical error - entity count mismatch!");
        exit(EXIT_FAILURE);
    }

    uint32_t firstplayer = functions.FindEntityByClassname(fields.CGlobalEntityList, 0, (uint32_t)"player");

    if(!firstplayer)
    {
        server_sleeping = true;
    }
    else
    {
        server_sleeping = false;
    }

    HooksSynergy::CleanupDeleteListHook(0);
    
    RemoveBadEnts();

    HooksSynergy::CleanupDeleteListHook(0);

    //SimulateEntities
    pDynamicOneArgFunc = (pOneArgProt)(server + 0x0074D2A0);
    pDynamicOneArgFunc(simulating);

    HooksSynergy::CleanupDeleteListHook(0);
    
    RemoveBadEnts();

    HooksSynergy::CleanupDeleteListHook(0);

    UpdateAllCollisions();

    HooksSynergy::CleanupDeleteListHook(0);
    
    //TriggerMovedFailsafe();
    return 0;
}

uint32_t HooksSynergy::VPhysicsSetObjectHook(uint32_t arg0, uint32_t arg1)
{
    pOneArgProt pDynamicOneArgFunc;
    pTwoArgProt pDynamicTwoArgFunc;

    if(IsEntityValid(arg0))
    {
        uint32_t vphysics_object = *(uint32_t*)(arg0+0x208);

        if(vphysics_object)
        {
            rootconsole->ConsolePrint("Attempting override existing vphysics object!!!!");
            return 0;
        }

        *(uint32_t*)(arg0+0x208) = arg1;

        return 0;
    }

    rootconsole->ConsolePrint("Entity was invalid failed to set vphysics object!");
    return 0;
}

uint32_t HooksSynergy::AcceptInputHook(uint32_t arg0, uint32_t arg1, uint32_t arg2, uint32_t arg3, uint32_t arg4, uint32_t arg5)
{
    pOneArgProt pDynamicOneArgFunc;
    pTwoArgProt pDynamicTwoArgFunc;
    pSixArgProt pDynamicSixArgProt;

    // CBaseEntity arg0 arg2 arg3

    bool failure = false;

    if(IsEntityValid(arg0) == 0) failure = true;

    if(failure)
    {
        //rootconsole->ConsolePrint("AcceptInput blocked on marked entity");
        return 0;
    }

    //Passed sanity check
    pDynamicSixArgProt = (pSixArgProt)(server + 0x005B4470);
    return pDynamicSixArgProt(arg0, arg1, arg2, arg3, arg4, arg5);
}

uint32_t HooksSynergy::CollisionRulesChangedHook(uint32_t arg0)
{
    InsertEntityToCollisionsList(arg0);
    return 0;
}

uint32_t HooksSynergy::CanSatisfyVpkCacheInternalHook(uint32_t arg0, uint32_t arg1, uint32_t arg2, uint32_t arg3, uint32_t arg4, uint32_t arg5, uint32_t arg6)
{
    pOneArgProt pDynamicOneArgFunc;
    pSevenArgProt pDynamicSevenArgFunc;

    pDynamicSevenArgFunc = (pSevenArgProt)(dedicated_srv + 0x000C7EB0);
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

uint32_t HooksSynergy::UTIL_RemoveBaseHook(uint32_t arg0)
{
    functions.RemoveEntityNormal(arg0, true);
    return 0;
}

void HookFunctionsSynergy()
{
    HookFunctionInSharedObject(server, server_size, (void*)(server + 0x006FF1E0), (void*)HooksSynergy::CreateEntityByNameHook);
    HookFunctionInSharedObject(server, server_size, (void*)(server + 0x0064A870), (void*)HooksSynergy::CleanupDeleteListHook);
    HookFunctionInSharedObject(server, server_size, (void*)(server + 0x0089F690), (void*)HooksSynergy::UTIL_RemoveHookFailsafe);
    HookFunctionInSharedObject(server, server_size, (void*)(server + 0x0089F7A0), (void*)HooksSynergy::UTIL_RemoveBaseHook);
    HookFunctionInSharedObject(server, server_size, (void*)(server + 0x0089F9C0), (void*)HooksSynergy::HookInstaKill);
    HookFunctionInSharedObject(server, server_size, (void*)(server + 0x0074CFD0), (void*)HooksSynergy::PhysSimEnt);
    HookFunctionInSharedObject(dedicated_srv, dedicated_srv_size, (void*)(dedicated_srv + 0x000C4B70), (void*)HooksSynergy::PackedStoreDestructorHook);
    HookFunctionInSharedObject(server, server_size, (void*)(server + 0x005B4470), (void*)HooksSynergy::AcceptInputHook);
    HookFunctionInSharedObject(server, server_size, (void*)(server + 0x005AEC70), (void*)HooksSynergy::UpdateOnRemove);
    HookFunctionInSharedObject(server, server_size, (void*)(server + 0x005CFE00), (void*)HooksSynergy::VPhysicsSetObjectHook);
    HookFunctionInSharedObject(server, server_size, (void*)(server + 0x005CFE60), (void*)HooksSynergy::CollisionRulesChangedHook);
    HookFunctionInSharedObject(dedicated_srv, dedicated_srv_size, (void*)(dedicated_srv + 0x000C7EB0), (void*)HooksSynergy::CanSatisfyVpkCacheInternalHook);


    //TEMP FIXES
    HookFunctionInSharedObject(server, server_size, (void*)(server + 0x00C296A0), (void*)HooksSynergy::EmptyCallStdCall);
    HookFunctionInSharedObject(server, server_size, (void*)(server + 0x00BE1DF0), (void*)HooksSynergy::EmptyCall);
}