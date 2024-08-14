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

    transition = false;
    savegame = false;
    savegame_lock = false;
    restoring = false;
    protect_player = false;
    save_frames = 0;
    restore_frames = 0;
    after_restore_frames = 0;
    game_start_frames = 0;
    isTicking = false;
    restore_delay = false;
    restore_delay_lock = false;
    disable_delete_list = false;
    firstplayer_hasjoined = false;
    hooked_delete_counter = 0;
    normal_delete_counter = 0;
    hasSavedOnce = false;
    reset_viewcontrol = false;
    saving_game_rightnow = false;
    weapon_substitute = 0xFFFFFFFF;
    server_sleeping = false;
    car_delay_for_save = 15;
    removing_ents_restore = false;
    restore_start_delay = 201;
    fake_sequence_mem = (uint32_t)malloc(1024);
    player_restore_failed = false;
    global_vpk_cache_buffer = (uint32_t)malloc(0x00100000);

    pthread_mutex_init(&playerDeathQueueLock, NULL);
    pthread_mutex_init(&collisionListLock, NULL);

    char* root_dir = getenv("PWD");
    const size_t max_path_length = 1024;

    char server_srv_fullpath[max_path_length];
    char engine_srv_fullpath[max_path_length];
    char scenefilecache_fullpath[max_path_length];
    char soundemittersystem_fullpath[max_path_length];
    char dedicated_srv_fullpath[max_path_length];
    char soundemittersystem_srv_fullpath[max_path_length];
    char materialsystem_srv_fullpath[max_path_length];
    char studiorender_srv_fullpath[max_path_length];
    char vphysics_srv_fullpath[max_path_length];
    char datacache_srv_fullpath[max_path_length];
    char sdktools_path[max_path_length];

    snprintf(server_srv_fullpath, max_path_length, "/synergy/bin/server_srv.so");
    snprintf(engine_srv_fullpath, max_path_length, "/bin/engine_srv.so");
    snprintf(scenefilecache_fullpath, max_path_length, "/bin/scenefilecache.so");
    snprintf(soundemittersystem_fullpath, max_path_length, "/bin/soundemittersystem.so");
    snprintf(dedicated_srv_fullpath, max_path_length, "/bin/dedicated_srv.so");
    snprintf(soundemittersystem_srv_fullpath, max_path_length, "/bin/soundemittersystem_srv.so");
    snprintf(materialsystem_srv_fullpath, max_path_length, "/bin/materialsystem_srv.so");
    snprintf(studiorender_srv_fullpath, max_path_length, "/bin/studiorender_srv.so");
    snprintf(vphysics_srv_fullpath, max_path_length, "/bin/vphysics_srv.so");
    snprintf(datacache_srv_fullpath, max_path_length, "/bin/datacache_srv.so");
    snprintf(sdktools_path, max_path_length, "/extensions/sdktools.ext.2.sdk2013.so");

    Library* engine_srv_lib = FindLibrary(engine_srv_fullpath, true);
    Library* datacache_srv_lib = FindLibrary(datacache_srv_fullpath, true);
    Library* dedicated_srv_lib = FindLibrary(dedicated_srv_fullpath, true);
    Library* materialsystem_srv_lib = FindLibrary(materialsystem_srv_fullpath, true);
    Library* vphysics_srv_lib = FindLibrary(vphysics_srv_fullpath, true);
    Library* scenefilecache_lib = FindLibrary(scenefilecache_fullpath, true);
    Library* soundemittersystem_lib = FindLibrary(soundemittersystem_fullpath, true);
    Library* soundemittersystem_srv_lib = FindLibrary(soundemittersystem_srv_fullpath, true);
    Library* studiorender_srv_lib = FindLibrary(studiorender_srv_fullpath, true);
    Library* server_srv_lib = FindLibrary(server_srv_fullpath, true);
    Library* sdktools_lib = FindLibrary(sdktools_path, true);

    if(!(engine_srv_lib && datacache_srv_lib && dedicated_srv_lib && 
    materialsystem_srv_lib && vphysics_srv_lib && scenefilecache_lib && 
    soundemittersystem_lib && soundemittersystem_srv_lib && studiorender_srv_lib &&
    server_srv_lib && sdktools_lib))
    {
        ClearLoadedLibraries();
        RestoreMemoryProtections();
        rootconsole->ConsolePrint("----------------------  Failed to load Synergy " SMEXT_CONF_NAME " " SMEXT_CONF_VERSION "  ----------------------");
        return false;
    }

    rootconsole->ConsolePrint("engine_srv_lib [%X] size [%X]", engine_srv_lib->library_base_address, engine_srv_lib->library_size);
    rootconsole->ConsolePrint("datacache_srv_lib [%X] size [%X]", datacache_srv_lib->library_base_address, datacache_srv_lib->library_size);
    rootconsole->ConsolePrint("dedicated_srv_lib [%X] size [%X]", dedicated_srv_lib->library_base_address, dedicated_srv_lib->library_size);
    rootconsole->ConsolePrint("materialsystem_srv_lib [%X] size [%X]", materialsystem_srv_lib->library_base_address, materialsystem_srv_lib->library_size);
    rootconsole->ConsolePrint("vphysics_srv_lib [%X] size [%X]", vphysics_srv_lib->library_base_address, vphysics_srv_lib->library_size);
    rootconsole->ConsolePrint("scenefilecache_lib [%X] size [%X]", scenefilecache_lib->library_base_address, scenefilecache_lib->library_size);
    rootconsole->ConsolePrint("soundemittersystem_lib [%X] size [%X]", soundemittersystem_lib->library_base_address, soundemittersystem_lib->library_size);
    rootconsole->ConsolePrint("soundemittersystem_srv_lib [%X] size [%X]", soundemittersystem_srv_lib->library_base_address, soundemittersystem_srv_lib->library_size);
    rootconsole->ConsolePrint("studiorender_srv_lib [%X] size [%X]", studiorender_srv_lib->library_base_address, studiorender_srv_lib->library_size);
    rootconsole->ConsolePrint("server_srv_lib [%X] size [%X]", server_srv_lib->library_base_address, server_srv_lib->library_size);
    rootconsole->ConsolePrint("sdktools_lib [%X] size [%X]", sdktools_lib->library_base_address, sdktools_lib->library_size);

    engine_srv = engine_srv_lib->library_base_address;
    datacache_srv = datacache_srv_lib->library_base_address;
    dedicated_srv = dedicated_srv_lib->library_base_address;
    materialsystem_srv = materialsystem_srv_lib->library_base_address;
    vphysics_srv = vphysics_srv_lib->library_base_address;
    scenefilecache = scenefilecache_lib->library_base_address;
    soundemittersystem = soundemittersystem_lib->library_base_address;
    soundemittersystem_srv = soundemittersystem_srv_lib->library_base_address;
    studiorender_srv = studiorender_srv_lib->library_base_address;
    server_srv = server_srv_lib->library_base_address;
    sdktools = sdktools_lib->library_base_address;

    engine_srv_size = engine_srv_lib->library_size;
    datacache_srv_size = datacache_srv_lib->library_size;
    dedicated_srv_size = dedicated_srv_lib->library_size;
    materialsystem_srv_size = materialsystem_srv_lib->library_size;
    vphysics_srv_size = vphysics_srv_lib->library_size;
    scenefilecache_size = scenefilecache_lib->library_size;
    soundemittersystem_size = soundemittersystem_lib->library_size;
    soundemittersystem_srv_size = soundemittersystem_srv_lib->library_size;
    studiorender_srv_size = studiorender_srv_lib->library_size;
    server_srv_size = server_srv_lib->library_size;
    sdktools_size = sdktools_lib->library_size;

    sdktools_passed = IsAllowedToPatchSdkTools(sdktools, sdktools_size);

    RestoreLinkedLists();
    SaveProcessId();
    
    antiCycleListDoors = AllocateValuesList();
    playerSaveList = AllocatePlayerSaveList();
    entityDeleteList = AllocateValuesList();
    playerDeathQueue = AllocateValuesList();
    collisionList = AllocateValuesList();
    viewcontrolresetlist = AllocateValuesList();
    saved_triggers = AllocateValuesList();
    new_player_join_ref = AllocateValuesList();

    //pOneArgProt pDynamicOneArgFunc;
    //pDynamicOneArgFunc = (pOneArgProt)(server_srv + 0x004C5950);
    //uint32_t currentTag = pDynamicOneArgFunc(server_srv + 0x00FE17E0);

    //rootconsole->ConsolePrint("\n\n\nCURRENT TAG: %s", currentTag);

    sv = engine_srv + 0x00324580;
    g_ModelLoader = engine_srv + 0x00317380;
    g_DataCache = datacache_srv + 0x00074EC0;
    g_MDLCache = datacache_srv + 0x00075140;
    s_ServerPlugin = engine_srv + 0x0034EF20;
    SaveRestoreGlobal = server_srv + 0x010121E0;
    CGlobalEntityList = server_srv + 0x00FF8740;

    Flush = datacache_srv + 0x0002BCD0;
    HostChangelevel = engine_srv + 0x0012A960;
    SpawnServer = engine_srv + 0x001AFC90;
    EdtLoadFuncAddr = server_srv + 0x00AEF9E0;
    LookupPoseParameterAddr = server_srv + 0x00628220;
    sub_654260_addr = server_srv + 0x00654260;
    sub_628F00_addr = server_srv + 0x00628F00;
    SaveGameStateAddr = server_srv + 0x00AF3990;
    TransitionRestoreMainCallOrigAddr = server_srv + 0x00AF46C0;
    OriginalTriggerMovedAddr = engine_srv + 0x001D8FD0;
    DoorFinalFunctionAddr = server_srv + 0x00A94600;
    GetNumClientsAddr = engine_srv + 0x000D3030;
    GetNumProxiesAddr = engine_srv + 0x000D3080;
    EnqueueCommandAddr = engine_srv + 0x000DE9C0;
    DispatchSpawnAddr = server_srv + 0x00B68190;
    ActivateEntityAddr = server_srv + 0x0065DB30;
    MakeEntityDormantAddr = server_srv + 0x00652B10;
    AutosaveLoadOrigAddr = server_srv + 0x00AF4530;
    InactivateClientsAddr = engine_srv + 0x000D5DA0;
    ReconnectClientsAddr = engine_srv + 0x000D5E50;
    PrintToClientAddr = server_srv + 0x00B66880;

    CollisionRulesChanged = (pOneArgProt)(server_srv + 0x003D8D20);
    pEdtLoadFunc = (pTwoArgProt)EdtLoadFuncAddr;
    pHostChangelevelFunc = (pThreeArgProt)HostChangelevel;
    pFlushFunc = (pThreeArgProt)Flush;
    pSpawnServerFunc = (pThreeArgProt)SpawnServer;
    LookupPoseParameter = (pThreeArgProt)LookupPoseParameterAddr;
    sub_654260 = (pOneArgProt)sub_654260_addr;
    sub_628F00 = (pOneArgProt)sub_628F00_addr;
    SaveGameState = (pThreeArgProt)SaveGameStateAddr;
    pTransitionRestoreMainCall = (pFourArgProt)TransitionRestoreMainCallOrigAddr;
    pCallOriginalTriggerMoved = (pTwoArgProt)OriginalTriggerMovedAddr;
    pDoorFinalFunction = (pFiveArgProt)DoorFinalFunctionAddr;
    GetNumClients = (pOneArgProt)GetNumClientsAddr;
    GetNumProxies = (pOneArgProt)GetNumProxiesAddr;
    EnqueueCommandFunc = (pOneArgProt)EnqueueCommandAddr;
    pDispatchSpawnFunc = (pOneArgProt)DispatchSpawnAddr;
    pActivateEntityFunc = (pOneArgProt)ActivateEntityAddr;
    AutosaveLoadOrig = (pThreeArgProt)AutosaveLoadOrigAddr;
    InactivateClients = (pOneArgProt)InactivateClientsAddr;
    ReconnectClients = (pOneArgProt)ReconnectClientsAddr;
    MakeEntityDormant = (pOneArgProt)(MakeEntityDormantAddr);
    PrintToClient = (pSevenArgProt)(PrintToClientAddr);

    delete_operator_array_addr = (void*) ( *(uint32_t*)(server_srv + 0x0041C8CD + 1) + (server_srv + 0x0041C8CD) + 5 );
    delete_operator_addr = (void*) ( *(uint32_t*)(server_srv + 0x0041C8FD + 1) + (server_srv + 0x0041C8FD) + 5 );

    new_operator_array_addr = (void*) ( *(uint32_t*)(server_srv + 0x0041CAB9 + 1) + (server_srv + 0x0041CAB9) + 5 );
    new_operator_addr = (void*) ( *(uint32_t*)(server_srv + 0x0041CA39 + 1) + (server_srv + 0x0041CA39) + 5 );

    strcpy_chk_addr = (void*) ( *(uint32_t*)(server_srv + 0x0039D18E + 1) + (server_srv + 0x0039D18E) + 5 );

    rootconsole->ConsolePrint("\n\nServer Map: [%s]\n\n", sv+0x11);

    snprintf(last_map, 1024, "%s", (char*)(sv+0x11));
    snprintf(global_map, 1024, "%s", (char*)(sv+0x11));

    offsets.classname_offset = 0x68;
    offsets.abs_origin_offset = 0x298;
    offsets.abs_angles_offset = 0x32C;
    offsets.origin_offset = 0x338;
    offsets.mnetwork_offset = 0x24;
    offsets.refhandle_offset = 0x350;
    offsets.iserver_offset = 0x18;

    functions.RemoveEntityNormal = (pTwoArgProt)(RemoveEntityNormalSynergy);
    functions.InstaKill = (pTwoArgProt)(InstaKillSynergy);
    functions.GetCBaseEntity = (pOneArgProt)(GetCBaseEntitySynergy);
    functions.IsMarkedForDeletion = (pOneArgProt)(server_srv + 0x00AC7EF0);

    PopulateHookPointers();
    PopulateHookExclusionListsSynergy();

    ApplyPatchesSynergy();
    ApplyPatchesSpecificSynergy();

    HookSaveRestoreOne();
    HookSaveRestoreTwo();
    HookSaveRestoreThree();
    HookSavingOne();
    HookSavingTwo();
    PatchRestore();
    HookVpkSystem();
    HookEdtSystem();
    HookSpawnServer();
    HookHostChangelevel();
    PatchOthers();

    HookFunctionsSynergy();
    HookFunctionsSpecificSynergy();

    RestoreMemoryProtections();
    rootconsole->ConsolePrint("----------------------  Synergy " SMEXT_CONF_NAME " loaded!" "  ----------------------");
    loaded_extension = true;
    return true;
}

void PopulateHookPointers()
{
    UTIL_Remove__External = (pOneArgProt)((uint32_t)HooksSynergy::HookEntityDelete);
    FindEntityByClassname = (pThreeArgProt)((uint32_t)HooksSynergy::FindEntityByClassnameHook);
    CreateEntityByNameHook__External = (pTwoArgProt)((uint32_t)HooksSynergy::CreateEntityByNameHook);
    CleanupDeleteListHook__External = (pOneArgProt)((uint32_t)HooksSynergy::CleanupDeleteListHook);
    PlayerSpawnHook__External = (pThreeArgProt)((uint32_t)HooksSynergy::PlayerSpawnHook);
    UTIL_RemoveInternal__External = (pOneArgProt)((uint32_t)HooksSynergy::UTIL_RemoveHookFailsafe);
    MainPlayerRestore__External = (pThreeArgProt)((uint32_t)HooksSynergy::MainPlayerRestoreHook);
}

void ApplyPatchesSynergy()
{
    uint32_t nop_patch_list[128] = 
    {
        //player patch
        0x009924F3,0x3B,0x009927E1,0xF,

        //heli patch
        0x0096026E,5,0x00815EF0,5,

        //0x00739AF6,5,0x00739B37,5,0x00739B3C,5,

        //remove broken code
        0x009B56B4,5,0x00B027D5,8,

        //post systems remove
        0x00739B37,5,

        //restore
        0x00AF4361,0x14,

        //weapons
        0x004FD574,2,

        //CleanupDeleteList calls
        /*0x00739AF1,5,0x00A316F0,5,0x00739B48,5*/

        0x00AE9E34,7
    };

    for(int i = 0; i < 128 && i+1 < 128; i = i+2)
    {
        uint32_t patch_location = nop_patch_list[i];
        uint32_t patch_location_length = nop_patch_list[i+1];

        if(patch_location == 0 || patch_location_length == 0)
            continue;

        patch_location = server_srv + patch_location;

        memset((void*)patch_location, 0x90, patch_location_length);
        //*(uint16_t*)(patch_location) = 0xC031;
    }

    uint32_t offset = 0;

    uint32_t vehicle_spawner_fix = server_srv + 0x00AE9E3B;
    *(uint8_t*)(vehicle_spawner_fix) = 0xEB;

    uint32_t remove_stdcall = server_srv + 0x008BFA8E;
    memset((void*)remove_stdcall, 0x90, 3);
    *(uint8_t*)(remove_stdcall) = 0xC3;

    uint32_t fix_ai = server_srv + 0x005703B2;
    *(uint8_t*)(fix_ai) = 0xB8;
    *(uint8_t*)(fix_ai+1) = 0xFF;
    *(uint8_t*)(fix_ai+2) = 0xFF;
    *(uint8_t*)(fix_ai+3) = 0xFF;

    uint32_t save_fix = server_srv + 0x004AF323;
    offset = (uint32_t)HooksSynergy::MainSaveEntitiesFunc - save_fix - 5;
    *(uint32_t*)(save_fix+1) = offset;

    uint32_t restore_fix = server_srv + 0x00AF4380;
    offset = (uint32_t)HooksSynergy::RepairPlayerRestore - restore_fix - 5;
    *(uint32_t*)(restore_fix+1) = offset;

    uint32_t jmp_vphys = server_srv + 0x00499346;
    *(uint8_t*)(jmp_vphys) = 0xEB;

    /*uint32_t player_think_patch_two = server_srv + 0x0098FFF3;
    *(uint8_t*)(player_think_patch_two) = 0xE9;
    *(uint32_t*)(player_think_patch_two+1) = 0xC8;*/

    uint32_t dropship_patch_one = server_srv + 0x0085F22C;
    offset = (uint32_t)HooksSynergy::LookupPoseParameterDropshipHook - dropship_patch_one - 5;
    *(uint32_t*)(dropship_patch_one+1) = offset;

    uint32_t dropship_patch_two = server_srv + 0x0085F266;
    offset = (uint32_t)HooksSynergy::LookupPoseParameterDropshipHook - dropship_patch_two - 5;
    *(uint32_t*)(dropship_patch_two+1) = offset;

    *(uint16_t*)((server_srv + 0x0096026E)) = 0xC031;
    *(uint16_t*)((server_srv + 0x00815EF0)) = 0xC031;

    if(sdktools_passed)
    {
        memset((void*)(sdktools + 0x00016903), 0x90, 2);
        memset((void*)(sdktools + 0x00016907), 0x90, 2);
    }

    //memset((void*)(engine_srv + 0x0012AA28), 0x90, 5);
    //memset((void*)(engine_srv + 0x001AF717), 0x90, 5);

    memset((void*)(engine_srv + 0x00136812), 0x90, 3);

    /*uint32_t save_system_ent_list_patch = server_srv + 0x004AF339;
    *(uint8_t*)(save_system_ent_list_patch) = 0xE9;
    *(uint32_t*)(save_system_ent_list_patch+1) = 0xFB;*/

    uint32_t jmp_to_fix_heli = server_srv + 0x00960275;
    *(uint8_t*)(jmp_to_fix_heli) = 0xE9;
    *(uint32_t*)(jmp_to_fix_heli+1) = 0x44E;

    uint32_t jmp_fix_heli_two = server_srv + 0x00815EF7;
    *(uint8_t*)(jmp_fix_heli_two) = 0xEB;

    //get rid of message spam on sound pointer
    uint32_t it_said_it_causes_corruption = server_srv + 0x00A5436D;
    *(uint8_t*)(it_said_it_causes_corruption) = 0xC3;

    /*uint32_t packet_crash_exploit_patch = engine_srv + 0x001DBE8E;
    *(uint8_t*)(packet_crash_exploit_patch) = 0xEB;*/

    /*uint32_t skip_end_clear_global_list = server_srv + 0x006B3FE5;
    *(uint8_t*)(skip_end_clear_global_list) = 0xE9;
    *(uint32_t*)(skip_end_clear_global_list+1) = 0x47;*/

    /*uint32_t save_system_skip = server_srv + 0x00AF3412;
    *(uint8_t*)(save_system_skip) = 0xE9;
    *(uint32_t*)(save_system_skip+1) = 0x167;*/

    /*uint32_t lvl_shutdown_skip = server_srv + 0x00737DA9;
    *(uint8_t*)(lvl_shutdown_skip) = 0xE9;
    *(uint32_t*)(lvl_shutdown_skip+1) = 0xC4;*/

    /*uint32_t save_system_patch_one = server_srv + 0x00AF3410;
    *(uint8_t*)(save_system_patch_one) = 0xE9;
    *(uint32_t*)(save_system_patch_one+1) = 0x169;*/

    uint32_t clientPutInServerRestoreCancel = server_srv + 0x00B030A1;
    *(uint8_t*)(clientPutInServerRestoreCancel) = 0xEB;

    uint32_t clientActiveRestoreCancel = server_srv + 0x00B031F7;
    *(uint8_t*)(clientActiveRestoreCancel) = 0xEB;

    uint32_t hook_game_frame_delete_list = server_srv + 0x00739B32;
    offset = (uint32_t)HooksSynergy::SimulateEntitiesHook - hook_game_frame_delete_list - 5;
    *(uint32_t*)(hook_game_frame_delete_list+1) = offset;

    uint32_t hook_event_queue = server_srv + 0x00739B3C;
    offset = (uint32_t)HooksSynergy::ServiceEventQueueHook - hook_event_queue - 5;
    *(uint32_t*)(hook_event_queue+1) = offset;
}

uint32_t HooksSynergy::MainPlayerRestoreHook(uint32_t arg0, uint32_t arg1, uint32_t arg2)
{
    pThreeArgProt pDynamicThreeArgFunc;

    uint32_t playerID = *(uint32_t*)(arg1+0x974);

    if(playerID == 0)
    {
        rootconsole->ConsolePrint("Player was not authenticated properly");
        return 0;
    }

    rootconsole->ConsolePrint("Restore Player ID: [%p]", playerID);

    pDynamicThreeArgFunc = (pThreeArgProt)(server_srv + 0x00AF4110);
    return pDynamicThreeArgFunc(arg0, arg1, arg2);
}

uint32_t HooksSynergy::AutosaveLoadHook(uint32_t arg0, uint32_t arg1, uint32_t arg2)
{
    return 0;
}

uint32_t HooksSynergy::EmptyCall()
{
    return 0;
}

uint32_t HooksSynergy::UTIL_PrecacheOther_Hook(uint32_t arg0, uint32_t arg1)
{
    pZeroArgProt pDynamicZeroArgProt;
    pOneArgProt pDynamicOneArgFunc;
    pTwoArgProt pDynamicTwoArgFunc;
    
    uint8_t cVar1;
    uint32_t piVar2;
    uint32_t local_10;

    pDynamicTwoArgFunc = (pTwoArgProt)(server_srv + 0x00B67EA0);
    cVar1 = pDynamicTwoArgFunc(server_srv + 0x01084200, arg0);

    if(cVar1 == 0) return 0;

    pDynamicTwoArgFunc = (pTwoArgProt)(server_srv + 0x009AFCA0);
    piVar2 = pDynamicTwoArgFunc(arg0, 0xFFFFFFFF);


    if(piVar2 != 0)
    {
        if((arg1 != 0) && (*(uint8_t*)arg1 != '\0'))
        {
            pTwoArgProt AllocPooledString = (pTwoArgProt)(server_srv + 0x0041B5D0);
            AllocPooledString((uint32_t)(&local_10), arg1);

            //this[0x87] = local_10;
            *(uint32_t*)(piVar2+0x220) = local_10;

            pOneArgProt DispatchUpdateTransmitState = (pOneArgProt)(server_srv + 0x00652D90);
            DispatchUpdateTransmitState(piVar2);

            //pcVar2 = param_2;
        }

        rootconsole->ConsolePrint("Precache %s", arg0);

        //Precache
        pDynamicOneArgFunc = (pOneArgProt)(  *(uint32_t*)((*(uint32_t*)(piVar2))+0x60)  );
        pDynamicOneArgFunc(piVar2);

        InstaKillSynergy(piVar2, false);
    }

    return 0;
}

uint32_t HooksSynergy::HookEntityDelete(uint32_t arg0)
{
    RemoveEntityNormalSynergy(arg0, true);
    return 0;
}

uint32_t HooksSynergy::UTIL_RemoveHookFailsafe(uint32_t arg0)
{
    // THIS IS UTIL_Remove(IServerNetworable*)
    // THIS HOOK IS FOR UNUSUAL CALLS TO UTIL_Remove probably from sourcemod!

    pZeroArgProt pDynamicZeroArgFunc;
    pOneArgProt pDynamicOneArgFunc;
    pTwoArgProt pDynamicTwoArgFunc;

    if(arg0 == 0) return 0;

    uint32_t cbase = arg0-0x18;
    char* classname = (char*)(*(uint32_t*)(cbase+0x68));
    uint32_t refHandle = *(uint32_t*)(cbase+0x350);
    uint32_t object = GetCBaseEntitySynergy(refHandle);

    if(object)
    {
        //IsMarkedForDeletion
        pDynamicOneArgFunc = (pOneArgProt)(server_srv + 0x00AC7EF0);
        uint32_t isMarked = pDynamicOneArgFunc(object+0x18);

        if(isMarked)
        {
            //rootconsole->ConsolePrint("Attempted to kill an entity twice in UTIL_Remove(IServerNetworable*)");
            return 0;
        }

        //IsInPhysCallback
        pDynamicZeroArgFunc = (pZeroArgProt)(server_srv + 0x00A1A500);
        uint8_t returnVal = pDynamicZeroArgFunc();

        if(returnVal == 0)
        {
            hooked_delete_counter++;
        }

        //rootconsole->ConsolePrint("hook counter: [%d] [%s]", hooked_delete_counter, classname);
        //rootconsole->ConsolePrint("Removing [%s]", *(uint32_t*)(object+0x68));

        //UTIL_Remove(IServerNetworkable*)
        pDynamicOneArgFunc = (pOneArgProt)(server_srv + 0x00B64480);
        return pDynamicOneArgFunc(object+0x18);
    }

    rootconsole->ConsolePrint("ERROR: Failed to validate entity!");
    exit(EXIT_FAILURE);
    return 0;
}

uint32_t HooksSynergy::CleanupDeleteListHook(uint32_t arg0)
{
    if(disable_delete_list) return 0;

    pOneArgProt pDynamicOneArgFunc;
    pTwoArgProt pDynamicTwoArgFunc;

    //CleanupDeleteList
    pDynamicOneArgFunc = (pOneArgProt)(server_srv + 0x006B2510);
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
    
    return (uint32_t)malloc(size*1.3);
}

uint32_t HooksSynergy::MallocHookLarge(uint32_t size)
{
    if(size <= 0) return (uint32_t)malloc(size);

    return (uint32_t)malloc(size*3.0);
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

void PatchRestore()
{
    int length = 5;

    uint32_t restore_call_one = server_srv + 0x0073C780;
    uint32_t restore_call_two = server_srv + 0x00AF46AB;
    uint32_t restore_call_three = server_srv + 0x004D9A40;

    uint32_t offset_three = (uint32_t)HooksSynergy::RestoreOverride - restore_call_three - 5;

    //memset((void*)restore_call_one, 0x90, length);
    //*(uint16_t*)(restore_call_one) = 0xC031;

    //memset((void*)restore_call_two, 0x90, length);
    //*(uint16_t*)(restore_call_two) = 0xC031;

    *(uint8_t*)(restore_call_three) = 0xE8;
    *(uint32_t*)(restore_call_three+1) = offset_three;

    rootconsole->ConsolePrint("--------------------- Restore system patched ---------------------");
}

void HookVpkSystem()
{
    int length = 5;

    uint32_t start = dedicated_srv + 0x000BE4F4;
    uint32_t offset = (uint32_t)HooksSynergy::DirectMallocHookDedicatedSrv - start - 5;

    *(uint8_t*)(start) = 0xE8;
    *(uint32_t*)(start+1) = offset;

    rootconsole->ConsolePrint("--------------------- VPK system hooked ---------------------");
}

void HookSaveRestoreOne()
{
    int length_one = 5;
    int length_two = 1;

    uint32_t patch_location_one = server_srv + 0x004AA3E2;
    uint32_t patch_location_two = server_srv + 0x004AA3EB;

    uint32_t offset = (uint32_t)HooksSynergy::SaveHookDirectMalloc - patch_location_one - 5;

    *(uint8_t*)(patch_location_one) = 0xE8;
    *(uint32_t*)(patch_location_one + 1) = offset;

    *(uint8_t*)(patch_location_two) = 0xEB;

    rootconsole->ConsolePrint("--------------------- Save/Restore part 1 hooked ---------------------");
}

void HookSaveRestoreTwo()
{
    int length = 5;

    uint32_t start = server_srv + 0x004B04B7;
    uint32_t offset = (uint32_t)HooksSynergy::SaveHookDirectMalloc - start - 5;

    *(uint8_t*)(start) = 0xE8;
    *(uint32_t*)(start+1) = offset;

    rootconsole->ConsolePrint("--------------------- Save/Restore part 2 hooked ---------------------");
}

void HookSaveRestoreThree()
{
    int length = 5;

    uint32_t start = server_srv + 0x004B0442;
    uint32_t offset = (uint32_t)HooksSynergy::SaveHookDirectRealloc - start - 5;

    *(uint8_t*)(start) = 0xE8;
    *(uint32_t*)(start+1) = offset;

    rootconsole->ConsolePrint("--------------------- Save/Restore part 3 hooked ---------------------");
}

void HookSavingOne()
{
    int length = 5;

    uint32_t start = server_srv + 0x004B0262;
    uint32_t offset = (uint32_t)HooksSynergy::SaveHookDirectRealloc - start - 5;

    *(uint8_t*)(start) = 0xE8;
    *(uint32_t*)(start+1) = offset;

    rootconsole->ConsolePrint("--------------------- Save part 1 hooked ---------------------");
}

void HookSavingTwo()
{
    int length = 5;

    uint32_t start = server_srv + 0x004B02A3;
    uint32_t offset = (uint32_t)HooksSynergy::SaveHookDirectMalloc - start - 5;

    *(uint8_t*)(start) = 0xE8;
    *(uint32_t*)(start+1) = offset;

    rootconsole->ConsolePrint("--------------------- Save part 2 hooked ---------------------");
}

void HookEdtSystem()
{
    int length = 5;

    uint32_t start = server_srv + 0x00AEFC34;
    uint32_t offset = (uint32_t)HooksSynergy::EdtSystemHookFunc - start - 5;

    *(uint8_t*)(start) = 0xE8;
    *(uint32_t*)(start+1) = offset;

    rootconsole->ConsolePrint("--------------------- EDT system hooked ---------------------");   
}

void HookSpawnServer()
{
    int length = 5;

    uint32_t start = engine_srv + 0x0012AA56;
    uint32_t offset = (uint32_t)HooksSynergy::SpawnServerHookFunc - start - 5;

    *(uint8_t*)(start) = 0xE8;
    *(uint32_t*)(start+1) = offset;


    rootconsole->ConsolePrint("--------------------- SpawnServer hooked ---------------------");   
}

void HookHostChangelevel()
{
    int length = 5;

    uint32_t start = engine_srv + 0x00136865;
    uint32_t offset = (uint32_t)HooksSynergy::HostChangelevelHook - start - 5;

    *(uint8_t*)(start) = 0xE8;
    *(uint32_t*)(start+1) = offset;


    rootconsole->ConsolePrint("--------------------- Changelevel hooked ---------------------");   
}

void PatchOthers()
{
    uint32_t patch_location_ten = server_srv + 0x0073C7CB;
    uint32_t patch_location_twelve = server_srv + 0x004AE993;
    uint32_t patch_location_fifthteen = server_srv + 0x00D65010;
    uint32_t patch_location_sixteen = engine_srv + 0x001CB601;
    uint32_t patch_location_seventeen = server_srv + 0x00D65550;

    uint32_t offset = 0;

    offset = (uint32_t)HooksSynergy::SV_TriggerMovedFix - patch_location_sixteen - 5;
    *(uint32_t*)(patch_location_sixteen + 1) = offset;

    offset = (uint32_t)HooksSynergy::TransitionEntityCreateCall - patch_location_twelve - 5;
    *(uint8_t*)(patch_location_twelve) = 0xE8;
    *(uint32_t*)(patch_location_twelve + 1) = offset;

    offset = (uint32_t)HooksSynergy::TransitionRestoreMain - patch_location_ten - 5;
    *(uint8_t*)(patch_location_ten) = 0xE8;
    *(uint32_t*)(patch_location_ten + 1) = offset;

    *(uint32_t*)(patch_location_fifthteen) = (uint32_t)HooksSynergy::DoorCycleResolve;
    *(uint32_t*)(patch_location_seventeen) = (uint32_t)HooksSynergy::DoorCycleResolve;

    uint32_t patch_another_cycle = server_srv + 0x00A95A9E;
    *(uint8_t*)(patch_another_cycle) = 0xEB;

    uint32_t yet_another_cycle = server_srv + 0x00A8653B;
    *(uint8_t*)(yet_another_cycle) = 0xEB;

    uint32_t scripted_sequence_patch_crash = server_srv + 0x00AC3F09;
    *(uint8_t*)(scripted_sequence_patch_crash) = 0xE9;
    *(uint32_t*)(scripted_sequence_patch_crash+1) = 0x70;

    //PATCH NETWORK EXPLOIT ONE
    uint32_t bf_read_base = engine_srv + 0x001DBE93;
    memset((void*)bf_read_base, 0x90, 0xC);

    *(uint8_t*)(bf_read_base) = 0x85;
    *(uint8_t*)(bf_read_base+1) = 0xD2;

    *(uint8_t*)(bf_read_base+2) = 0x74;
    *(uint8_t*)(bf_read_base+3) = 0x13;

    //PATCH NETWORK EXPLOIT TWO
    uint32_t memcpy_hook_one = engine_srv + 0x000EBE87;
    offset = (uint32_t)HooksSynergy::memcpyNetworkHook - memcpy_hook_one - 5;
    *(uint32_t*)(memcpy_hook_one+1) = offset;

    uint32_t memcpy_hook_two = engine_srv + 0x0016ABE7;
    offset = (uint32_t)HooksSynergy::memcpyNetworkHook - memcpy_hook_two - 5;
    *(uint32_t*)(memcpy_hook_two+1) = offset;

    rootconsole->ConsolePrint("--------------------- Other parts patched ---------------------");
}

uint32_t HooksSynergy::SaveHookDirectMalloc(uint32_t size)
{
    uint32_t new_size = size*5.0;
    uint32_t ref = (uint32_t)malloc(new_size);
    memset((void*)ref, 0, new_size);
    //rootconsole->ConsolePrint("malloc() [Save/Restore Hook] " HOOK_MSG " size: [%d]", ref, new_size);

    Value* leak = CreateNewValue((void*)ref);
    InsertToValuesList(leakedResourcesSaveRestoreSystem, leak, NULL, false, false);

    return ref;
}

uint32_t HooksSynergy::SaveHookDirectRealloc(uint32_t old_ptr, uint32_t new_size)
{
    uint32_t enlarged_size = new_size;

    if(new_size > 0)
    {
        uint32_t enlarged_size = new_size*5.0;
    }
    
    uint32_t ref = (uint32_t)realloc((void*)old_ptr, enlarged_size);
    //rootconsole->ConsolePrint("realloc() [Save Hook] " HOOK_MSG " size: [%d]", ref, enlarged_size);

    RemoveFromValuesList(leakedResourcesSaveRestoreSystem, (void*)old_ptr, NULL);

    Value* leak = CreateNewValue((void*)ref);
    InsertToValuesList(leakedResourcesSaveRestoreSystem, leak, NULL, false, false);

    return ref;
}

uint32_t HooksSynergy::UpdateOnRemove(uint32_t arg0)
{
    pOneArgProt pDynamicOneArgFunc;

    char* classname = (char*)(*(uint32_t*)(arg0+0x68));

    normal_delete_counter++;

    //rootconsole->ConsolePrint("normal counter: [%d] [%s]", normal_delete_counter, classname);

    pDynamicOneArgFunc = (pOneArgProt)(server_srv + 0x0065BD80);
    return pDynamicOneArgFunc(arg0);
}

uint32_t HooksSynergy::ParseMapEntities(uint32_t arg0, uint32_t arg1, uint32_t arg2)
{
    pOneArgProt pDynamicOneArgFunc;
    pThreeArgProt pDynamicThreeArgFunc;

    pDynamicThreeArgFunc = (pThreeArgProt)(server_srv + 0x009B09F0);
    return pDynamicThreeArgFunc(arg0, arg1, arg2);
}

uint32_t HooksSynergy::EdtSystemHookFunc(uint32_t arg1)
{
    uint32_t ref = (uint32_t)malloc(arg1*3.0);
    rootconsole->ConsolePrint("[EDT Hook] " HOOK_MSG, ref);

    Value* leak = CreateNewValue((void*)ref);
    InsertToValuesList(leakedResourcesEdtSystem, leak, NULL, true, true);

    return ref;
}

uint32_t HooksSynergy::PreEdtLoad(uint32_t arg1, uint32_t arg2)
{
    //DONT USE!

    /*uint32_t returnVal = pEdtLoadFunc(arg1, arg2);
    ReleaseLeakedMemory(leakedResourcesEdtSystem, false, 0, 0, 100);
    return returnVal;*/
    return 0;
}

uint32_t HooksSynergy::SaveRestoreMemManage(uint32_t arg0, uint32_t arg1)
{
    pTwoArgProt pDynamicTwoArgFunc;
    
    pDynamicTwoArgFunc = (pTwoArgProt)(server_srv + 0x00AF24F0);
    uint32_t returnVal = pDynamicTwoArgFunc(arg0, arg1);

    ReleaseLeakedMemory(leakedResourcesSaveRestoreSystem, false, 0, 0, 100);

    return returnVal;
}

uint32_t HooksSynergy::RestoreOverride()
{
    pOneArgProt pDynamicOneArgFunc;

    if(savegame || restoring || server_sleeping) return 0;

    if(!hasSavedOnce)
    {
        rootconsole->ConsolePrint("Failed to restore - no valid autosave file found!");
        return 0;
    }

    restoring = true;

    isTicking = false;
    protect_player = true;

    *(uint8_t*)((*(uint32_t*)(server_srv + 0x00FA0CF0)) + 0x130) = 1;

    //BeginRestoreEntities
    pDynamicOneArgFunc = (pOneArgProt)(server_srv + 0x0073B880);
    pDynamicOneArgFunc(0);

    uint32_t main_engine_global = *(uint32_t*)(server_srv + 0x00109A3E0);

    rootconsole->ConsolePrint("Clearing entities!");

    removing_ents_restore = true;

    uint32_t mainEnt = 0;
    while((mainEnt = HooksSynergy::FindEntityByClassnameHook(CGlobalEntityList, mainEnt, (uint32_t)"*")) != 0)
    {
        char* classname = (char*) ( *(uint32_t*)(mainEnt+0x68) );
        uint32_t refHandle = *(uint32_t*)(mainEnt+0x350);

        pDynamicOneArgFunc = (pOneArgProt)(server_srv + 0x00AF29F0);
        uint8_t allowEntRestore = pDynamicOneArgFunc((uint32_t)classname);

        if(allowEntRestore)
        {
            HooksSynergy::HookEntityDelete(mainEnt);
        }
    }

    HooksSynergy::CleanupDeleteListHook(0);

    removing_ents_restore = false;

    //EDICT REUSE
    pDynamicOneArgFunc = (pOneArgProt)(  *(uint32_t*) ((*(uint32_t*)(*(uint32_t*)(server_srv + 0x01012420)))+0x16C)  );
    pDynamicOneArgFunc(*(uint32_t*)(server_srv + 0x01012420));

    AutosaveLoadOrig(*(uint32_t*)(server_srv + 0x00FA0CF0), (uint32_t)"autosave", 0);
    RestorePlayers();

    *(uint8_t*)(server_srv + 0x01012130) = 1;

    HooksSynergy::CleanupDeleteListHook(0);

    //EndRestoreEntities
    pDynamicOneArgFunc = (pOneArgProt)(server_srv + 0x0073CBD0);
    pDynamicOneArgFunc(0);
    
    *(uint8_t*)((*(uint32_t*)(server_srv + 0x00FA0CF0)) + 0x130) = 0;

    reset_viewcontrol = true;
    after_restore_frames = 0;

    InsertViewcontrolsToResetList();
    DisableViewControls();

    protect_player = false;
    restore_delay = true;

    return 0;
}

uint32_t HooksSynergy::DirectMallocHookDedicatedSrv(uint32_t arg0)
{

    memset((void*)global_vpk_cache_buffer, 0, 0x00100000);
    return global_vpk_cache_buffer;



    uint32_t ebp = 0;
    asm volatile ("movl %%ebp, %0" : "=r" (ebp));

    uint32_t arg0_return = *(uint32_t*)(ebp-4);
    uint32_t packed_store_ref = arg0_return-0x228;

    bool saved_reference = false;


    uint32_t ref = (uint32_t)malloc(arg0*3.0);
    
    Value* a_leak = *leakedResourcesVpkSystem;

    while(a_leak)
    {
        VpkMemoryLeak* the_leak = (VpkMemoryLeak*)(a_leak->value);
        uint32_t packed_object = the_leak->packed_ref;

        if(packed_object == packed_store_ref)
        {
            saved_reference = true;

            ValueList vpk_leak_list = the_leak->leaked_refs;
            Value* new_vpk_leak = CreateNewValue((void*)(ref));
            InsertToValuesList(vpk_leak_list, new_vpk_leak, NULL, false, false);

            rootconsole->ConsolePrint("[VPK Hook] " HOOK_MSG, ref);
        }

        a_leak = a_leak->nextVal;
    }

    if(!saved_reference)
    {
        rootconsole->ConsolePrint("Failed to allocate leaked resource!");
        exit(1);
    }

    return ref;
}

uint32_t HooksSynergy::PackedStoreConstructorHook(uint32_t arg0, uint32_t arg1, uint32_t arg2, uint32_t arg3, uint32_t arg4)
{
    //Save ref for leak mapping
    pFiveArgProt pDynamicFiveArgFunc;

    pDynamicFiveArgFunc = (pFiveArgProt)(dedicated_srv + 0x000BD1B0);
    uint32_t returnVal = pDynamicFiveArgFunc(arg0, arg1, arg2, arg3, arg4);

    Value* existing_structs = *leakedResourcesVpkSystem;

    while(existing_structs)
    {
        VpkMemoryLeak* v_leak = (VpkMemoryLeak*)(existing_structs->value);

        if(v_leak->packed_ref == arg0)
        {
            rootconsole->ConsolePrint("[VPK Hook] Object already found");
            return returnVal;
        }

        existing_structs = existing_structs->nextVal;
    }

    VpkMemoryLeak* omg_leaks = (VpkMemoryLeak*)(malloc(sizeof(VpkMemoryLeak)));
    ValueList empty_list = AllocateValuesList();

    omg_leaks->packed_ref = arg0;
    omg_leaks->leaked_refs = empty_list;

    Value* leaked_resource = CreateNewValue((void*)omg_leaks);
    InsertToValuesList(leakedResourcesVpkSystem, leaked_resource, NULL, false, false);

    return returnVal;
}

uint32_t HooksSynergy::PackedStoreDestructorHook(uint32_t arg0)
{
    //Remove ref to store only valid objects!
    pOneArgProt pDynamicOneArgFunc;

    pDynamicOneArgFunc = (pOneArgProt)(dedicated_srv + 0x000BAE80);
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

uint32_t HooksSynergy::SavegameInternalFunction(uint32_t arg0)
{
    pOneArgProt pDynamicOneArgFunc;
    
    FixModelnameSlashes();
    HooksSynergy::CleanupDeleteListHook(0);

    uint32_t deleteQueue = *(uint32_t*)(server_srv + 0x0100890C);
    uint32_t physQueue = *(uint32_t*)(server_srv + 0x01032AF0);
    uint32_t unk_arg0 = server_srv + 0x01032900;

    physQueue = physQueue + *(uint32_t*)(unk_arg0+0xC8);
    physQueue = physQueue + *(uint32_t*)(unk_arg0+0xF0);
    physQueue = physQueue + *(uint32_t*)(unk_arg0+0x104);
    physQueue = physQueue + *(uint32_t*)(unk_arg0+0x154);

    if(deleteQueue == 0 && physQueue == 0)
    {
        rootconsole->ConsolePrint("Save-system determined its safe to make a save file!");

        saving_game_rightnow = true;

        pDynamicOneArgFunc = (pOneArgProt)(server_srv + 0x00AF33F0);
        uint32_t returnVal = pDynamicOneArgFunc(arg0);

        saving_game_rightnow = false;
        
        return returnVal;
    }

    rootconsole->ConsolePrint("WARNING: delete and/or physics queue was NOT empty!");
    return 0;
}

uint32_t HooksSynergy::LevelChangedHookFrameSnaps(uint32_t arg0)
{
    pOneArgProt pDynamicOneArgFunc;

    ReleaseLeakedPackedEntities();

    pDynamicOneArgFunc = (pOneArgProt)(engine_srv + 0x001A5FB0);
    return pDynamicOneArgFunc(arg0);
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

    uint32_t sim_ent_ref = *(uint32_t*)(arg0+0x350);
    uint32_t object_check = GetCBaseEntitySynergy(sim_ent_ref);

    if(object_check == 0)
    {
        rootconsole->ConsolePrint("Passed in non-existant simulation entity!");
        exit(EXIT_FAILURE);
        return 0;
    }

    char* clsname =  (char*) ( *(uint32_t*)(arg0+0x68) );

    //IsMarkedForDeletion
    pDynamicOneArgFunc = (pOneArgProt)(server_srv + 0x00AC7EF0);
    uint32_t isMarked = pDynamicOneArgFunc(arg0+0x18);

    if(isMarked)
    {
        rootconsole->ConsolePrint("Simulation ignored for [%s]", clsname);
        return 0;
    }
    
    disable_delete_list = true;
    pDynamicOneArgFunc = (pOneArgProt)(server_srv + 0x00A311D0);
    uint32_t returnVal = pDynamicOneArgFunc(arg0);
    disable_delete_list = false;
    return returnVal;
}

uint32_t HooksSynergy::TransitionEntityCreateCall(uint32_t arg1, uint32_t arg2)
{
    rootconsole->ConsolePrint(EXT_PREFIX "Restoring %s", arg1);

    uint32_t object = HooksSynergy::CreateEntityByNameHook(arg1, arg2);

    if(object)
    {
        uint32_t ref = *(uint32_t*)(object+0x350);
    }

    return object;
}

uint32_t HooksSynergy::TransitionRestoreMain(uint32_t arg1, uint32_t arg2, uint32_t arg3, uint32_t arg4)
{
    pOneArgProt pDynamicOneArgFunc;
    transition = true;

    if(strcmp((char*)arg2, (char*)last_map) == 0)
    {
        rootconsole->ConsolePrint("oldmap matched successfully!");
    }
    else
    {
        rootconsole->ConsolePrint("Failed to preserve oldmap: [%s] [%s]", arg2, last_map);
        arg2 = (uint32_t)last_map;
    }

    uint32_t main_engine_global = *(uint32_t*)(server_srv + 0x00109A3E0);
    uint32_t someStuff = *(uint32_t*)(server_srv + 0x0107375C);
    uint32_t someStuff_24 = *(uint32_t*)(someStuff+0x24);
    uint32_t current_map = sv + 0x11;

    char savefile[512];
    char* root_dir = getenv("PWD");
    snprintf(savefile, 512, "%s/synergy/%s%s.hl1", root_dir, (char*)someStuff_24, (char*)current_map);

    rootconsole->ConsolePrint("[%s]", savefile);

    if(access(savefile, F_OK) == 0)
    {
        removing_ents_restore = true;

        uint32_t mainEnt = 0;

        while((mainEnt = HooksSynergy::FindEntityByClassnameHook(CGlobalEntityList, mainEnt, (uint32_t)"*")) != 0)
        {
            char* classname = (char*) ( *(uint32_t*)(mainEnt+0x68) );
            uint32_t refHandle = *(uint32_t*)(mainEnt+0x350);

            pDynamicOneArgFunc = (pOneArgProt)(server_srv + 0x00AF29F0);
            uint8_t allowEntRestore = pDynamicOneArgFunc((uint32_t)classname);

            if(allowEntRestore)
            {
                HooksSynergy::HookEntityDelete(mainEnt);
            }
        }

        HooksSynergy::CleanupDeleteListHook(0);

        removing_ents_restore = false;

        //EDICT REUSE
        pDynamicOneArgFunc = (pOneArgProt)(  *(uint32_t*) ((*(uint32_t*)(*(uint32_t*)(server_srv + 0x01012420)))+0x16C)  );
        pDynamicOneArgFunc(*(uint32_t*)(server_srv + 0x01012420));
        
        AutosaveLoadOrig(*(uint32_t*)(server_srv + 0x00FA0CF0), (uint32_t)current_map, 0);
        *(uint8_t*)(server_srv + 0x01012130) = 1;
    }
    
    uint32_t returnVal = pTransitionRestoreMainCall(arg1, arg2, arg3, arg4);

    HooksSynergy::CleanupDeleteListHook(0);

    return returnVal;
}

uint32_t HooksSynergy::SaveOverride(uint32_t arg1)
{
    savegame = true;
    return 1;
}

uint32_t HooksSynergy::SV_TriggerMovedFix(uint32_t arg1, uint32_t arg2)
{
    if(*(uint8_t*)arg1 == 4)
    {
        rootconsole->ConsolePrint("Prevented triggering a corrupted trigger!");
        return 0;
    }

    uint32_t edx = *(uint32_t*)(arg1+0x0C);

    if(edx)
    {
        uint32_t edx_deref = *(uint32_t*)edx;
        pOneArgProt pDynamicOneArgFunc = (pOneArgProt)(*(uint32_t*)(edx_deref+0x10));
        uint32_t returnVal = pDynamicOneArgFunc(edx);

        if(returnVal)
        {
            //Safe to procced
            return pCallOriginalTriggerMoved(arg1, arg2);
        }
    }

    rootconsole->ConsolePrint("Prevented triggering a corrupted trigger!");
    return 0;
}

uint32_t HooksSynergy::memcpyNetworkHook(uint32_t dest, uint32_t src, uint32_t size)
{
    if(size <= 4096 && size >= 0)
    {
        return (uint32_t)memcpy((void*)dest, (void*)src, size);
    }

    rootconsole->ConsolePrint("NET_EXPLOIT %d", size);
    return 0;
}

uint32_t HooksSynergy::DoorCycleResolve(uint32_t arg1, uint32_t arg2, uint32_t arg3, uint32_t arg4, uint32_t arg5)
{
    uint32_t something = *(uint32_t*)(server_srv + 0x00F4BA30);
    uint32_t mainValue = *(uint32_t*)(arg1+0x6E0);

    if(mainValue != 0xFFFFFFFF)
    {
        if(IsInValuesList(antiCycleListDoors, (void*)mainValue, NULL))
        {
            rootconsole->ConsolePrint("Cycle was detected and prevented hang!");
            DeleteAllValuesInList(antiCycleListDoors, false, NULL);
            return pDoorFinalFunction(arg1, arg2, arg3, arg4, arg5);
        }

        //Save ref for cycle resolving
        Value* val_save = CreateNewValue((void*)mainValue);
        InsertToValuesList(antiCycleListDoors, val_save, NULL, false, true);

        uint32_t aCheck = mainValue >> 0x0C;

        mainValue = mainValue & 0xFFF;
        mainValue = mainValue << 0x4;
        mainValue = mainValue + something;

        uint32_t lea = mainValue+0x4;
        uint32_t val = *(uint32_t*)(lea+0x4);

        if( val == aCheck )
        {
            uint32_t instance = *(uint32_t*)(mainValue+0x4);

            if(instance)
                DoorCycleResolve(instance, arg2, arg3, arg4, arg5);
        }
    }

    DeleteAllValuesInList(antiCycleListDoors, false, NULL);
    return pDoorFinalFunction(arg1, arg2, arg3, arg4, arg5);
}

uint32_t HooksSynergy::CreateEntityByNameHook(uint32_t arg0, uint32_t arg1)
{
    pTwoArgProt pDynamicTwoArgFunc;

    pDynamicTwoArgFunc = (pTwoArgProt)(server_srv + 0x009AFCA0);
    uint32_t returnVal = pDynamicTwoArgFunc(arg0, arg1);

    if(strcmp((char*)arg0, "prop_vehicle_mp") == 0)
    {
        *(uint8_t*)(returnVal+0x81A) = 1;
        *(uint8_t*)(returnVal+0x4BC) = 0;
    }

    return returnVal;
}

uint32_t HooksSynergy::LevelChangeSafeHook(uint32_t arg0)
{
    pOneArgProt pDynamicOneArgFunc;
    pTwoArgProt pDynamicTwoArgFunc;
    pThreeArgProt pDynamicThreeArgFunc;
    
    pDynamicOneArgFunc = (pOneArgProt)(server_srv + 0x004C5C20);
    pDynamicOneArgFunc(arg0);

    pDynamicOneArgFunc = (pOneArgProt)(server_srv + 0x004C5D50);
    uint8_t returnVal = pDynamicOneArgFunc(arg0);

    pDynamicOneArgFunc = (pOneArgProt)(server_srv + 0x004CC6D0);
    pDynamicOneArgFunc(arg0);

    if(returnVal != 0)
    {
        //UnloadAllModels
        pDynamicTwoArgFunc = (pTwoArgProt)(engine_srv + 0x0014D480);
        pDynamicTwoArgFunc(engine_srv + 0x00317380, 0);

        //ReloadAllMaterials
        //pDynamicTwoArgFunc = (pTwoArgProt)(materialsystem_srv + 0x0003E440);
        //pDynamicTwoArgFunc(materialsystem_srv + 0x00134B20, 0);

        //UnmountPaths
        pDynamicOneArgFunc = (pOneArgProt)(server_srv + 0x004C5CA0);
        pDynamicOneArgFunc(arg0);
        
        pDynamicOneArgFunc = (pOneArgProt)(server_srv + 0x004C9AF0);
        pDynamicOneArgFunc(arg0);
        pDynamicOneArgFunc = (pOneArgProt)(server_srv + 0x004CBC60);
        pDynamicOneArgFunc(arg0);

        pDynamicOneArgFunc = (pOneArgProt)(server_srv + 0x004C5950);
        char* currentTag = (char*) pDynamicOneArgFunc(arg0);

        if(strcasecmp(currentTag, "hl2") == 0)
        {
            //Content loader call
            pDynamicTwoArgFunc = (pTwoArgProt)(server_srv + 0x00B8EB50);
            pDynamicTwoArgFunc(server_srv + 0x00FB2180, 0);
        }
        else
        {
            //Content loader call
            pDynamicTwoArgFunc = (pTwoArgProt)(server_srv + 0x00B8EB50);
            pDynamicTwoArgFunc(server_srv + 0x00FB2180, 1);
        }

        //ReloadSoundSystem
        pDynamicOneArgFunc = (pOneArgProt)(server_srv + 0x004C5560);
        pDynamicOneArgFunc(arg0);

        //Reload model sounds cache
        pDynamicOneArgFunc = (pOneArgProt)(server_srv + 0x004C44A0);
        pDynamicOneArgFunc(arg0);

        pDynamicTwoArgFunc = (pTwoArgProt)(server_srv + 0x004994B0);
        pDynamicTwoArgFunc(*(uint32_t*)(server_srv + 0x00FC1FC0), *(uint32_t*)(server_srv + 0x01012410));

        pDynamicThreeArgFunc = (pThreeArgProt)(    *(uint32_t*) ((*(uint32_t*)((*(uint32_t*)(server_srv + 0x01012410))+4))+0x28)   );
        uint8_t returnVal = pDynamicThreeArgFunc( (*(uint32_t*)(server_srv + 0x01012410))+4, (uint32_t)"scenes.cache", server_srv + 0x00C16B35 );

        if(returnVal)
        {
            pDynamicThreeArgFunc = (pThreeArgProt)(    *(uint32_t*) ((*(uint32_t*)((*(uint32_t*)(server_srv + 0x01012410))+0))+0x3C)   );
            pDynamicThreeArgFunc( (*(uint32_t*)(server_srv + 0x01012410))+0, (uint32_t)"scenes.cache", server_srv + 0x00C16B35 );

            rootconsole->ConsolePrint("Removed old scenes.cache files!");
        }

        pDynamicOneArgFunc = (pOneArgProt)(    *(uint32_t*) ((*(uint32_t*)((*(uint32_t*)(server_srv + 0x010123B0))+0))+0x28)   );
        pDynamicOneArgFunc( (*(uint32_t*)(server_srv + 0x010123B0))+0 );

        //pTwoArgProt pDynamicTwoArgFunc = (pTwoArgProt)(server_srv + 0x0047BDF0);
        //pDynamicTwoArgFunc(1, 1);
    }

    //scene_flush direct call
    //pDynamicOneArgFunc = (pOneArgProt)(server_srv + 0x00AAA840);
    //pDynamicOneArgFunc(0);

    //Invalidate mdl cache
    //pDynamicOneArgFunc = (pOneArgProt)(server_srv + 0x0073C1C0);
    //pDynamicOneArgFunc(0);

    //Flush - data cache
    //uint32_t freed_bytes = pFlushFunc((uint32_t)g_DataCache, (uint32_t)false, (uint32_t)false);
    //rootconsole->ConsolePrint("Freed [%d] bytes from cache!", freed_bytes);

    //Flush - mdl cache
    //pTwoArgProt pDynamicTwoArgFunc = (pTwoArgProt)(datacache_srv + 0x000381D0);
    //pDynamicTwoArgFunc(datacache_srv + 0x00075140, (uint32_t)MDLCACHE_FLUSH_VERTEXES);
    return 0;

    //pDynamicOneArgFunc = (pOneArgProt)(server_srv + 0x004CCA80);
    //return pDynamicOneArgFunc(arg0);
}

uint32_t HooksSynergy::PlayerSpawnHook(uint32_t arg0, uint32_t arg1, uint32_t arg2)
{
    rootconsole->ConsolePrint("called the main spawn info sender!");
    pThreeArgProt pDynamicThreeArgFunc;
    
    pDynamicThreeArgFunc = (pThreeArgProt)(server_srv + 0x00B01A90);
    return pDynamicThreeArgFunc(arg0, 1, 1);
}

uint32_t HooksSynergy::PlayerSpawnDirectHook(uint32_t arg0)
{
    rootconsole->ConsolePrint("[Main] Called the main player spawn func!");

    pOneArgProt pDynamicOneArgFunc;
    pThreeArgProt pDynamicThreeArgFunc;

    HooksSynergy::PlayerSpawnHook(arg0, 1, 1);

    pDynamicOneArgFunc = (pOneArgProt)(server_srv + 0x00B043C0);
    uint32_t returnVal = pDynamicOneArgFunc(arg0);

    uint32_t player_ref = *(uint32_t*)(arg0+0x350);

    EntityFrameCount* entity_delay = (EntityFrameCount*)malloc(sizeof(EntityFrameCount));
    entity_delay->entity_ref = player_ref;
    entity_delay->frames = 0;

    Value* updated_color = CreateNewValue((void*)entity_delay);
    InsertToValuesList(new_player_join_ref, updated_color, NULL, false, false);

    firstplayer_hasjoined = true;
    return returnVal;
}

uint32_t HooksSynergy::MainSaveEntitiesFunc(uint32_t arg0, uint32_t arg1)
{
    pOneArgProt pDynamicOneArgFunc;
    pTwoArgProt pDynamicTwoArgFunc;

    pDynamicTwoArgFunc = (pTwoArgProt)(server_srv + 0x006B26B0);
    uint32_t object = pDynamicTwoArgFunc(arg0, arg1);

    while(true)
    {
        if(object)
        {
            char* classname = (char*)(*(uint32_t*)(object+0x68));

            if(classname)
            {
                if(strcmp(classname, "player") == 0)
                {
                    uint32_t playerID = *(uint32_t*)(object+0x974);

                    if(playerID == 0)
                    {
                        rootconsole->ConsolePrint("Player was not valid for saving!");

                        pDynamicTwoArgFunc = (pTwoArgProt)(server_srv + 0x006B26B0);
                        object = pDynamicTwoArgFunc(arg0, object);

                        continue;
                    }
                }
            }
        }

        break;
    }

    return object;
}

uint32_t HooksSynergy::FindEntityByHandle(uint32_t arg0, uint32_t arg1)
{
    pOneArgProt pDynamicOneArgFunc;
    pTwoArgProt pDynamicTwoArgFunc = (pTwoArgProt)(server_srv + 0x006B26B0);
    uint32_t object = pDynamicTwoArgFunc(arg0, arg1);

    return object;
}

uint32_t HooksSynergy::FindEntityByClassnameHook(uint32_t arg0, uint32_t arg1, uint32_t arg2)
{
    pOneArgProt pDynamicOneArgFunc;
    pThreeArgProt pDynamicThreeArgFunc = (pThreeArgProt)(server_srv + 0x006B2740);
    uint32_t object = pDynamicThreeArgFunc(arg0, arg1, arg2);

    return object;
}

uint32_t HooksSynergy::FindEntityByName(uint32_t arg0, uint32_t arg1, uint32_t arg2, uint32_t arg3, uint32_t arg4, uint32_t arg5, uint32_t arg6)
{
    pOneArgProt pDynamicOneArgFunc;
    pSevenArgProt pDynamicSevenArgFunc = (pSevenArgProt)(server_srv + 0x006B2CA0);
    uint32_t object = pDynamicSevenArgFunc(arg0, arg1, arg2, arg3, arg4, arg5, arg6);

    return object;
}

uint32_t HooksSynergy::PlayerloadSavedHook(uint32_t arg0, uint32_t arg1)
{
    pZeroArgProt pDynamicZeroArgFunc;
    pOneArgProt pDynamicOneArgFunc;

    if(savegame || restoring || server_sleeping) return 0;

    if(!hasSavedOnce)
    {
        rootconsole->ConsolePrint("Failed to restore - no valid autosave file found!");
        return 0;
    }

    DisableViewControls();

    restore_start_delay = 0;
    
    pDynamicOneArgFunc = (pOneArgProt)(server_srv + 0x00A4B8C0);
    return pDynamicOneArgFunc(arg0);
}

uint32_t HooksSynergy::SpawnServerHookFunc(uint32_t arg1, uint32_t arg2, uint32_t arg3)
{
    snprintf(last_map, 1024, "%s", global_map);
    snprintf(global_map, 1024, "%s", (char*)arg2);
    return pSpawnServerFunc(arg1, arg2, arg3);
}

uint32_t HooksSynergy::HostChangelevelHook(uint32_t arg1, uint32_t arg2, uint32_t arg3)
{
    pOneArgProt pDynamicOneArgFunc;

    restoring = false;
    isTicking = false;
    hasSavedOnce = false;

    LogVpkMemoryLeaks();

    uint32_t player = 0;

    while((player = HooksSynergy::FindEntityByClassnameHook(CGlobalEntityList, player, (uint32_t)"player")) != 0)
    {
        //Ragdoll
        //pDynamicOneArgFunc = (pOneArgProt)(server_srv + 0x0098D1A0);
        //pDynamicOneArgFunc(player);

        uint32_t ragdoll_ref = *(uint32_t*)(player+0x1594);
        uint32_t ragdoll = GetCBaseEntitySynergy(ragdoll_ref);
        HooksSynergy::HookInstaKill(ragdoll);

        //*(uint32_t*)(player+0x1594) = 0xFFFFFFFF;
    }

    restore_start_delay = 201;

    firstplayer_hasjoined = false;
    transition = false;

    uint32_t gpGlobals_i_think = *(uint32_t*)(server_srv + 0x00FA0CF0);
    *(uint8_t*)(gpGlobals_i_think) = 0;

    uint32_t returnVal = pHostChangelevelFunc(arg1, arg2, arg3);

    /*if(strcmp(global_map, "d3_c17_10a") == 0)
    {
        uint32_t searchEnt = HooksSynergy::FindEntityByClassnameHook(CGlobalEntityList, 0, (uint32_t)"npc_barney");

        if(searchEnt != 0)
        {
            uint32_t m_refHandle = *(uint32_t*)(searchEnt+0x350);
            SendEntityInput(m_refHandle, (uint32_t)"Kill", 0, 0, 0, (uint32_t)-1);
        }
        
        uint32_t newEntity = CreateEntityByName((uint32_t)"npc_barney", (uint32_t)-1);
        pThreeArgProt pDynamicThreeArgFunc = (pThreeArgProt)(*(uint32_t*)((*(uint32_t*)newEntity)+0x78));
        pDynamicThreeArgFunc(newEntity, (uint32_t)"origin", (uint32_t)"-4083 6789.41 12");
        pDynamicThreeArgFunc(newEntity, (uint32_t)"targetname", (uint32_t)"barney");
        pDynamicThreeArgFunc(newEntity, (uint32_t)"spawnflags", (uint32_t)"4");
        pDynamicThreeArgFunc(newEntity, (uint32_t)"additionalequipment", (uint32_t)"weapon_ar2");
        pDynamicThreeArgFunc(newEntity, (uint32_t)"OnDeath", (uint32_t)"barney_dead_text,ShowMessage,,0,-1");
        pDynamicThreeArgFunc(newEntity, (uint32_t)"OnDeath", (uint32_t)"barney_dead_fade,Reload,,0,-1");
        pDispatchSpawnFunc(newEntity);
        pActivateEntityFunc(newEntity);
    }
    else if(strcmp(global_map, "d2_prison_02") == 0)
    {
        uint32_t searchEnt = 0;

        while((searchEnt = HooksSynergy::FindEntityByClassnameHook(CGlobalEntityList, searchEnt, (uint32_t)"prop_door_rotating")) != 0)
        {
            char* targetname = (char*) ( *(uint32_t*)(searchEnt+0x124) );
            if(!targetname)
                continue;

            if(strcmp(targetname, "door_2") == 0)
            {
                rootconsole->ConsolePrint("Killed door_2");
                uint32_t m_refHandle = *(uint32_t*)(searchEnt+0x350);
                SendEntityInput(m_refHandle, (uint32_t)"Kill", 0, 0, 0, (uint32_t)-1);
                break;
            }
        }

        uint32_t newEntity = CreateEntityByName((uint32_t)"prop_door_rotating", (uint32_t)-1);
        pThreeArgProt pDynamicThreeArgFunc = (pThreeArgProt)(*(uint32_t*)((*(uint32_t*)newEntity)+0x78));
        pDynamicThreeArgFunc(newEntity, (uint32_t)"origin", (uint32_t)"-1155 3372.09 438");
        pDynamicThreeArgFunc(newEntity, (uint32_t)"angles", (uint32_t)"0 270 0");
        pDynamicThreeArgFunc(newEntity, (uint32_t)"speed", (uint32_t)"100");
        pDynamicThreeArgFunc(newEntity, (uint32_t)"skin", (uint32_t)"7");
        pDynamicThreeArgFunc(newEntity, (uint32_t)"hardware", (uint32_t)"1");
        pDynamicThreeArgFunc(newEntity, (uint32_t)"axis", (uint32_t)"573 -1603.91 1654, 573 -1603.91 1654");
        pDynamicThreeArgFunc(newEntity, (uint32_t)"model", (uint32_t)"models/props_c17/door01_left.mdl");
        pDynamicThreeArgFunc(newEntity, (uint32_t)"targetname", (uint32_t)"door_2");
        pDynamicThreeArgFunc(newEntity, (uint32_t)"OnFullyOpen", (uint32_t)"entrance_music,PlaySound,,0,1");
        pDispatchSpawnFunc(newEntity);
        pActivateEntityFunc(newEntity);

        rootconsole->ConsolePrint("Spawned door_2");
    }*/

    ReleasePlayerSavedList();

    //Remove soundent forever
    //pOneArgProt pDynamicOneArgFunc = (pOneArgProt)(server_srv + 0x00ADDAC0);
    //pDynamicOneArgFunc(0);

    restoring = false;

    if(!IsSynergyMemoryCorrect())
    {
        ForceSynergyMemoryCorrection();
        rootconsole->ConsolePrint("\n\nSynergy memory integrity failure\n\n");
    }

    ReleaseSavedTriggers();
    game_start_frames = 0;

    return returnVal;
}

uint32_t HooksSynergy::LookupPoseParameterDropshipHook(uint32_t dropship_object, uint32_t arg1, uint32_t arg2)
{
    uint32_t m_hContainer = *(uint32_t*)(dropship_object+0x1024);
    uint32_t m_hContainer_object = GetCBaseEntitySynergy(m_hContainer);

    if(m_hContainer_object)
    {
        uint32_t eax = *(uint32_t*)(m_hContainer_object+0x4A4);

        if(eax == 0)
        {
            uint32_t returnVal = sub_654260(m_hContainer_object);

            if(returnVal != 0)
                sub_628F00(m_hContainer_object);

            eax = *(uint32_t*)(m_hContainer_object+0x4A4);

            if(eax != 0)
            {
                uint32_t deref = *(uint32_t*)eax;

                if(deref == 0)
                    eax = 0;
            }
        }

        rootconsole->ConsolePrint("everything worked in dropship hook!");
        return LookupPoseParameter(dropship_object, eax, arg2);
    }

    rootconsole->ConsolePrint("Failed to prepare dropship turret gun!");
    return LookupPoseParameter(dropship_object, arg1, arg2);
}

uint32_t HooksSynergy::DropshipSpawnHook(uint32_t arg0)
{
    pOneArgProt pDynamicOneArgFunc;

    pDynamicOneArgFunc = (pOneArgProt)(server_srv + 0x0086A6A0);
    uint32_t returnVal = pDynamicOneArgFunc(arg0);

    *(uint8_t*)(server_srv + 0x0101D330) = 0;

    //PopulatePoseParameters - Dropship
    pDynamicOneArgFunc = (pOneArgProt)(server_srv + 0x0085F0E0);
    pDynamicOneArgFunc(arg0);

    return returnVal;
}

uint32_t HooksSynergy::fix_wheels_hook(uint32_t arg0, uint32_t arg1, uint32_t arg2)
{
    pThreeArgProt pDynamicThreeArgFunc;

    if(car_delay_for_save < 15)
    {
        rootconsole->ConsolePrint("Prevented vehicle exit!");
        return 0;
    }

    //rootconsole->ConsolePrint("Allowed usage!");
    
    pDynamicThreeArgFunc = (pThreeArgProt)(vphysics_srv + 0x000D6820);
    return pDynamicThreeArgFunc(arg0, arg1, arg2);
}

uint32_t HooksSynergy::PlayerDeathHook(uint32_t arg0)
{   
    uint32_t refHandle = *(uint32_t*)(arg0+0x350);
    Value* newPlayer = CreateNewValue((void*)refHandle);
    InsertToValuesList(playerDeathQueue, newPlayer, &playerDeathQueueLock, false, false);
    return 0;
}

uint32_t HooksSynergy::HookInstaKill(uint32_t arg0)
{
    InstaKillSynergy(arg0, true);
    return 0;
}

uint32_t HooksSynergy::SV_FrameHook(uint32_t arg0)
{
    pOneArgProt pDynamicOneArgFunc;

    save_frames++;
    restore_frames++;
    game_start_frames++;
    car_delay_for_save++;
    restore_start_delay++;

    if(save_frames >= 500) save_frames = 0;
    if(restore_frames >= 500) restore_frames = 0;
    if(after_restore_frames >= 500) after_restore_frames = 0;
    if(game_start_frames >= 1000) game_start_frames = 1000;
    if(car_delay_for_save >= 1000) car_delay_for_save = 15;
    if(restore_start_delay >= 1000) restore_start_delay = 201;

    if(restore_delay && !restore_delay_lock)
    {
        restore_frames = 0;
        restore_delay_lock = true;
    }
    else if(restore_delay && restore_delay_lock && restore_frames >= 50)
    {
        restoring = false;
        restore_delay = false;
        restore_delay_lock = false;
    }

    pDynamicOneArgFunc = (pOneArgProt)(engine_srv + 0x001B1800);
    return pDynamicOneArgFunc(arg0);
}

uint32_t HooksSynergy::ServiceEventQueueHook()
{
    pZeroArgProt pDynamicZeroArgFunc;
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

    uint32_t firstplayer = HooksSynergy::FindEntityByClassnameHook(CGlobalEntityList, 0, (uint32_t)"player");

    if(!firstplayer)
    {
        server_sleeping = true;
    }
    else
    {
        server_sleeping = false;
    }

    HooksSynergy::CleanupDeleteListHook(0);

    //SimulateEntities
    pDynamicOneArgFunc = (pOneArgProt)(server_srv + 0x00A316A0);
    pDynamicOneArgFunc(simulating);

    HooksSynergy::CleanupDeleteListHook(0);

    //ServiceEventQueue
    pDynamicZeroArgFunc = (pZeroArgProt)(server_srv + 0x00687440);
    pDynamicZeroArgFunc();

    HooksSynergy::CleanupDeleteListHook(0);

    SaveGame_Extension();

    HooksSynergy::CleanupDeleteListHook(0);

    FlushPlayerDeaths();
    ResetView();
    UpdatePlayersDonor();
    AttemptToRestoreGame();

    HooksSynergy::CleanupDeleteListHook(0);

    UpdateAllCollisions();

    HooksSynergy::CleanupDeleteListHook(0);

    RemoveBadEnts();

    HooksSynergy::CleanupDeleteListHook(0);

    //PostSystems
    pDynamicOneArgFunc = (pOneArgProt)(server_srv + 0x00471320);
    pDynamicOneArgFunc(0);

    HooksSynergy::CleanupDeleteListHook(0);

    uint8_t deferMindist = *(uint8_t*)(vphysics_srv + 0x001AC980);

    if(deferMindist)
    {
        rootconsole->ConsolePrint("Warning defer mindist was set! Physics might break!");
    }

    *(uint8_t*)(vphysics_srv + 0x001AC980) = 0;
    
    //TriggerMovedFailsafe();
    return 0;
}

uint32_t HooksSynergy::SetCollisionGroupHook(uint32_t arg0, uint32_t arg1)
{
    pTwoArgProt pDynamicTwoArgFunc;

    pDynamicTwoArgFunc = (pTwoArgProt)(server_srv + 0x003D9390);
    return pDynamicTwoArgFunc(arg0, arg1);
}

uint32_t HooksSynergy::SetSolidFlagsHook(uint32_t arg0, uint32_t arg1)
{
    pTwoArgProt pDynamicTwoArgFunc;

    pDynamicTwoArgFunc = (pTwoArgProt)(server_srv + 0x003F98A0);
    return pDynamicTwoArgFunc(arg0, arg1);
}

uint32_t HooksSynergy::VPhysicsSetObjectHook(uint32_t arg0, uint32_t arg1)
{
    pOneArgProt pDynamicOneArgFunc;
    pTwoArgProt pDynamicTwoArgFunc;

    uint32_t vphysics_object = *(uint32_t*)(arg0+0x1FC);

    if(vphysics_object)
    {
        rootconsole->ConsolePrint("Attempting override existing vphysics object!!!!");
        return 0;
    }

    *(uint32_t*)(arg0+0x1FC) = arg1;
    return 0;
}

uint32_t HooksSynergy::VPhysicsInitShadowHook(uint32_t arg0, uint32_t arg1, uint32_t arg2, uint32_t arg3)
{
    pFourArgProt pDynamicFourArgFunc;

    pDynamicFourArgFunc = (pFourArgProt)(server_srv + 0x003D8F30);
    return pDynamicFourArgFunc(arg0, arg1, arg2, arg3);
}

uint32_t HooksSynergy::GetClientSteamIDHook(uint32_t arg0, uint32_t arg1)
{
    pTwoArgProt pDynamicTwoArgFunc;

    if(!arg1)
    {
        rootconsole->ConsolePrint("GetClientSteamID failed NULL edict_t passed! [%p]", (uint32_t)__builtin_return_address(0) - server_srv);
        return 0;
    }

    pDynamicTwoArgFunc = (pTwoArgProt)(engine_srv + 0x001CC730);
    return pDynamicTwoArgFunc(arg0, arg1);
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
    pDynamicSixArgProt = (pSixArgProt)(server_srv + 0x00654F80);
    return pDynamicSixArgProt(arg0, arg1, arg2, arg3, arg4, arg5);
}

uint32_t HooksSynergy::SetGlobalState(uint32_t arg0, uint32_t arg1)
{
    pTwoArgProt pDynamicTwoArgFunc;

    if(removing_ents_restore)
    {
        rootconsole->ConsolePrint("Attempted to change global state while making a restore!");
        return 0;
    }

    pDynamicTwoArgFunc = (pTwoArgProt)(server_srv + 0x0074A4A0);
    return pDynamicTwoArgFunc(arg0, arg1);
}

uint32_t HooksSynergy::RepairPlayerRestore(uint32_t arg0, uint32_t arg1, uint32_t arg2)
{
    // arg0 is scrubbed

    uint32_t classname = *(uint32_t*)(arg1+0x68);

    rootconsole->ConsolePrint("Restore failed for [%s] [%s]", classname, arg2);
    RemoveEntityNormalSynergy(arg1, true);

    player_restore_failed = true;
    return 0;
}

uint32_t HooksSynergy::CollisionRulesChangedHook(uint32_t arg0)
{
    return 0;
}

uint32_t HooksSynergy::CanSatisfyVpkCacheHook(uint32_t arg0, uint32_t arg1, uint32_t arg2, uint32_t arg3, uint32_t arg4, uint32_t arg5, uint32_t arg6)
{
    pOneArgProt pDynamicOneArgFunc;
    pSevenArgProt pDynamicSevenArgFunc;

    uint32_t vpk_cache_tree = arg0+0x0D8;

    pDynamicOneArgFunc = (pOneArgProt)(dedicated_srv + 0x000C0000);
    pDynamicOneArgFunc(vpk_cache_tree);

    pDynamicSevenArgFunc = (pSevenArgProt)(dedicated_srv + 0x000BE520);
    return pDynamicSevenArgFunc(arg0, arg1, arg2, arg3, arg4, arg5, arg6);
}

void HookFunctionsSynergy()
{
    HookFunctionInSharedObject(server_srv, server_srv_size, (void*)(server_srv + 0x009B09F0), (void*)HooksSynergy::ParseMapEntities);
    HookFunctionInSharedObject(server_srv, server_srv_size, (void*)(server_srv + 0x00AF24F0), (void*)HooksSynergy::SaveRestoreMemManage);
    HookFunctionInSharedObject(server_srv, server_srv_size, (void*)(server_srv + 0x009AFCA0), (void*)HooksSynergy::CreateEntityByNameHook);
    HookFunctionInSharedObject(server_srv, server_srv_size, (void*)(server_srv + 0x00AEF9E0), (void*)HooksSynergy::PreEdtLoad);
    HookFunctionInSharedObject(engine_srv, engine_srv_size, (void*)(engine_srv + 0x001A5FB0), (void*)HooksSynergy::LevelChangedHookFrameSnaps);
    HookFunctionInSharedObject(engine_srv, engine_srv_size, (void*)(engine_srv + 0x001CC730), (void*)HooksSynergy::GetClientSteamIDHook);
    HookFunctionInSharedObject(vphysics_srv, vphysics_srv_size, (void*)(vphysics_srv + 0x000D6820), (void*)HooksSynergy::fix_wheels_hook);
    HookFunctionInSharedObject(server_srv, server_srv_size, (void*)(server_srv + 0x00AF4530), (void*)HooksSynergy::AutosaveLoadHook);
    //HookFunctionInSharedObject(server_srv, server_srv_size, (void*)(server_srv + 0x00652B10), (void*)HooksSynergy::EmptyCall);
    HookFunctionInSharedObject(server_srv, server_srv_size, (void*)(server_srv + 0x0074A4A0), (void*)HooksSynergy::SetGlobalState);
    HookFunctionInSharedObject(server_srv, server_srv_size, (void*)(server_srv + 0x00AF4110), (void*)HooksSynergy::MainPlayerRestoreHook);

    //rootconsole->ConsolePrint("patching calloc()");
    //HookFunctionInSharedObject(server_srv, server_srv_size, (void*)calloc, (void*)HooksSynergy::CallocHook);
    //HookFunctionInSharedObject(engine_srv, engine_srv_size, (void*)calloc, (void*)HooksSynergy::CallocHook);
    //HookFunctionInSharedObject(datacache_srv, datacache_srv_size, (void*)calloc, (void*)HooksSynergy::CallocHook);
    //HookFunctionInSharedObject(dedicated_srv, dedicated_srv_size, (void*)calloc, (void*)HooksSynergy::CallocHook);
    //HookFunctionInSharedObject(materialsystem_srv, materialsystem_srv_size, (void*)calloc, (void*)HooksSynergy::CallocHook);
    //HookFunctionInSharedObject(vphysics_srv, vphysics_srv_size, (void*)calloc, (void*)HooksSynergy::CallocHook);
    //HookFunctionInSharedObject(scenefilecache, scenefilecache_size, (void*)calloc, (void*)HooksSynergy::CallocHook);
    //HookFunctionInSharedObject(soundemittersystem, soundemittersystem_size, (void*)calloc, (void*)HooksSynergy::CallocHook);
    //HookFunctionInSharedObject(soundemittersystem_srv, soundemittersystem_srv_size, (void*)calloc, (void*)HooksSynergy::CallocHook);
    //HookFunctionInSharedObject(studiorender_srv, studiorender_srv_size, (void*)calloc, (void*)HooksSynergy::CallocHook);

    rootconsole->ConsolePrint("patching malloc()");
    HookFunctionInSharedObject(server_srv, server_srv_size, (void*)malloc, (void*)HooksSynergy::MallocHookSmall);
    //HookFunctionInSharedObject(engine_srv, engine_srv_size, (void*)malloc, (void*)HooksSynergy::MallocHook);
    //HookFunctionInSharedObject(datacache_srv, datacache_srv_size, (void*)malloc, (void*)HooksSynergy::MallocHook);
    HookFunctionInSharedObject(dedicated_srv, dedicated_srv_size, (void*)malloc, (void*)HooksSynergy::MallocHookLarge);
    //HookFunctionInSharedObject(materialsystem_srv, materialsystem_srv_size, (void*)malloc, (void*)HooksSynergy::MallocHook);
    //HookFunctionInSharedObject(vphysics_srv, vphysics_srv_size, (void*)malloc, (void*)HooksSynergy::MallocHook);
    //HookFunctionInSharedObject(scenefilecache, scenefilecache_size, (void*)malloc, (void*)HooksSynergy::MallocHook);
    //HookFunctionInSharedObject(soundemittersystem, soundemittersystem_size, (void*)malloc, (void*)HooksSynergy::MallocHook);
    //HookFunctionInSharedObject(soundemittersystem_srv, soundemittersystem_srv_size, (void*)malloc, (void*)HooksSynergy::MallocHook);
    //HookFunctionInSharedObject(studiorender_srv, studiorender_srv_size, (void*)malloc, (void*)HooksSynergy::MallocHook);

    //rootconsole->ConsolePrint("patching realloc()");
    //HookFunctionInSharedObject(server_srv, server_srv_size, (void*)realloc, (void*)HooksSynergy::ReallocHook);
    //HookFunctionInSharedObject(engine_srv, engine_srv_size, (void*)realloc, (void*)HooksSynergy::ReallocHook);
    //HookFunctionInSharedObject(datacache_srv, datacache_srv_size, (void*)realloc, (void*)HooksSynergy::ReallocHook);
    //HookFunctionInSharedObject(dedicated_srv, dedicated_srv_size, (void*)realloc, (void*)HooksSynergy::ReallocHook);
    //HookFunctionInSharedObject(materialsystem_srv, materialsystem_srv_size, (void*)realloc, (void*)HooksSynergy::ReallocHook);
    //HookFunctionInSharedObject(vphysics_srv, vphysics_srv_size, (void*)realloc, (void*)HooksSynergy::ReallocHook);
    //HookFunctionInSharedObject(scenefilecache, scenefilecache_size, (void*)realloc, (void*)HooksSynergy::ReallocHook);
    //HookFunctionInSharedObject(soundemittersystem, soundemittersystem_size, (void*)realloc, (void*)HooksSynergy::ReallocHook);
    //HookFunctionInSharedObject(soundemittersystem_srv, soundemittersystem_srv_size, (void*)realloc, (void*)HooksSynergy::ReallocHook);
    //HookFunctionInSharedObject(studiorender_srv, studiorender_srv_size, (void*)realloc, (void*)HooksSynergy::ReallocHook);
    
    //rootconsole->ConsolePrint("patching operator new");
    //HookFunctionInSharedObject(server_srv, server_srv_size, new_operator_addr, (void*)HooksSynergy::OperatorNewHook);
    //HookFunctionInSharedObject(engine_srv, engine_srv_size, new_operator_addr, (void*)HooksSynergy::OperatorNewHook);
    //HookFunctionInSharedObject(datacache_srv, datacache_srv_size, new_operator_addr, (void*)HooksSynergy::OperatorNewHook);
    //HookFunctionInSharedObject(dedicated_srv, dedicated_srv_size, new_operator_addr, (void*)HooksSynergy::OperatorNewHook);
    //HookFunctionInSharedObject(materialsystem_srv, materialsystem_srv_size, new_operator_addr, (void*)HooksSynergy::OperatorNewHook);
    //HookFunctionInSharedObject(vphysics_srv, vphysics_srv_size, new_operator_addr, (void*)HooksSynergy::OperatorNewHook);
    //HookFunctionInSharedObject(scenefilecache, scenefilecache_size, new_operator_addr, (void*)HooksSynergy::OperatorNewHook);
    //HookFunctionInSharedObject(soundemittersystem, soundemittersystem_size, new_operator_addr, (void*)HooksSynergy::OperatorNewHook);
    //HookFunctionInSharedObject(soundemittersystem_srv, soundemittersystem_srv_size, new_operator_addr, (void*)HooksSynergy::OperatorNewHook);
    //HookFunctionInSharedObject(studiorender_srv, studiorender_srv_size, new_operator_addr, (void*)HooksSynergy::OperatorNewHook);

    //rootconsole->ConsolePrint("patching operator new[]");
    //HookFunctionInSharedObject(server_srv, server_srv_size, new_operator_array_addr, (void*)HooksSynergy::OperatorNewArrayHook);
    //HookFunctionInSharedObject(engine_srv, engine_srv_size, new_operator_array_addr, (void*)HooksSynergy::OperatorNewArrayHook);
    //HookFunctionInSharedObject(datacache_srv, datacache_srv_size, new_operator_array_addr, (void*)HooksSynergy::OperatorNewArrayHook);
    //HookFunctionInSharedObject(dedicated_srv, dedicated_srv_size, new_operator_array_addr, (void*)HooksSynergy::OperatorNewArrayHook);
    //HookFunctionInSharedObject(materialsystem_srv, materialsystem_srv_size, new_operator_array_addr, (void*)HooksSynergy::OperatorNewArrayHook);
    //HookFunctionInSharedObject(vphysics_srv, vphysics_srv_size, new_operator_array_addr, (void*)HooksSynergy::OperatorNewArrayHook);
    //HookFunctionInSharedObject(scenefilecache, scenefilecache_size, new_operator_array_addr, (void*)HooksSynergy::OperatorNewArrayHook);
    //HookFunctionInSharedObject(soundemittersystem, soundemittersystem_size, new_operator_array_addr, (void*)HooksSynergy::OperatorNewArrayHook);
    //HookFunctionInSharedObject(soundemittersystem_srv, soundemittersystem_srv_size, new_operator_array_addr, (void*)HooksSynergy::OperatorNewArrayHook);
    //HookFunctionInSharedObject(studiorender_srv, studiorender_srv_size, new_operator_array_addr, (void*)HooksSynergy::OperatorNewArrayHook);




    HookFunctionInSharedObject(server_srv, server_srv_size, (void*)(server_srv + 0x00A4B8C0), (void*)HooksSynergy::PlayerloadSavedHook);
    HookFunctionInSharedObject(server_srv, server_srv_size, (void*)(server_srv + 0x00B043C0), (void*)HooksSynergy::PlayerSpawnDirectHook);
    HookFunctionInSharedObject(server_srv, server_srv_size, (void*)(server_srv + 0x006B26B0), (void*)HooksSynergy::FindEntityByHandle);
    HookFunctionInSharedObject(server_srv, server_srv_size, (void*)(server_srv + 0x006B2740), (void*)HooksSynergy::FindEntityByClassnameHook);
    HookFunctionInSharedObject(server_srv, server_srv_size, (void*)(server_srv + 0x006B2CA0), (void*)HooksSynergy::FindEntityByName);
    HookFunctionInSharedObject(server_srv, server_srv_size, (void*)(server_srv + 0x006B2510), (void*)HooksSynergy::CleanupDeleteListHook);
    HookFunctionInSharedObject(server_srv, server_srv_size, (void*)(server_srv + 0x00B64500), (void*)HooksSynergy::HookEntityDelete);
    HookFunctionInSharedObject(server_srv, server_srv_size, (void*)(server_srv + 0x00B64480), (void*)HooksSynergy::UTIL_RemoveHookFailsafe);
    HookFunctionInSharedObject(server_srv, server_srv_size, (void*)(server_srv + 0x00B64630), (void*)HooksSynergy::HookInstaKill);
    HookFunctionInSharedObject(server_srv, server_srv_size, (void*)(server_srv + 0x00AF3990), (void*)HooksSynergy::SaveOverride);
    HookFunctionInSharedObject(server_srv, server_srv_size, (void*)(server_srv + 0x00B01A90), (void*)HooksSynergy::PlayerSpawnHook);
    HookFunctionInSharedObject(server_srv, server_srv_size, (void*)(server_srv + 0x00AF33F0), (void*)HooksSynergy::SavegameInternalFunction);
    HookFunctionInSharedObject(server_srv, server_srv_size, (void*)(server_srv + 0x00A311D0), (void*)HooksSynergy::PhysSimEnt);
    HookFunctionInSharedObject(server_srv, server_srv_size, (void*)(server_srv + 0x00AEFDB0), (void*)HooksSynergy::EmptyCall);
    HookFunctionInSharedObject(server_srv, server_srv_size, (void*)(server_srv + 0x004CCA80), (void*)HooksSynergy::LevelChangeSafeHook);
    HookFunctionInSharedObject(engine_srv, engine_srv_size, (void*)(engine_srv + 0x001B1800), (void*)HooksSynergy::SV_FrameHook);
    //HookFunctionInSharedObject(server_srv, server_srv_size, (void*)(server_srv + 0x0098D1A0), (void*)HooksSynergy::PlayerDeathHook);
    HookFunctionInSharedObject(server_srv, server_srv_size, (void*)(server_srv + 0x0086A6A0), (void*)HooksSynergy::DropshipSpawnHook);
    HookFunctionInSharedObject(dedicated_srv, dedicated_srv_size, (void*)(dedicated_srv + 0x000BD1B0), (void*)HooksSynergy::PackedStoreConstructorHook);
    HookFunctionInSharedObject(dedicated_srv, dedicated_srv_size, (void*)(dedicated_srv + 0x000BAE80), (void*)HooksSynergy::PackedStoreDestructorHook);
    HookFunctionInSharedObject(server_srv, server_srv_size, (void*)(server_srv + 0x00654F80), (void*)HooksSynergy::AcceptInputHook);
    HookFunctionInSharedObject(server_srv, server_srv_size, (void*)(server_srv + 0x0065BD80), (void*)HooksSynergy::UpdateOnRemove);
    HookFunctionInSharedObject(server_srv, server_srv_size, (void*)(server_srv + 0x00B67F10), (void*)HooksSynergy::UTIL_PrecacheOther_Hook);
    HookFunctionInSharedObject(server_srv, server_srv_size, (void*)(server_srv + 0x003D8F30), (void*)HooksSynergy::VPhysicsInitShadowHook);
    HookFunctionInSharedObject(server_srv, server_srv_size, (void*)(server_srv + 0x003D8DA0), (void*)HooksSynergy::VPhysicsSetObjectHook);
    HookFunctionInSharedObject(server_srv, server_srv_size, (void*)(server_srv + 0x003D9390), (void*)HooksSynergy::SetCollisionGroupHook);
    HookFunctionInSharedObject(server_srv, server_srv_size, (void*)(server_srv + 0x003F98A0), (void*)HooksSynergy::SetSolidFlagsHook);
    //HookFunctionInSharedObject(server_srv, server_srv_size, (void*)(server_srv + 0x003D8D20), (void*)HooksSynergy::CollisionRulesChangedHook);

    HookFunctionInSharedObject(dedicated_srv, dedicated_srv_size, (void*)(dedicated_srv + 0x000BE520), (void*)HooksSynergy::CanSatisfyVpkCacheHook);
}