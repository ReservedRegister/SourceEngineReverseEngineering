#include "extension.h"
#include "core.h"
#include "ext_main.h"
#include "hooks_specific.h"

void InitExtension()
{
    InitCore();
    AllowWriteToMappedMemory();

    transition = false;
    savegame = false;
    savegame_lock = false;
    restoring = false;
    protect_player = false;
    mapHasEnded = false;
    mapHasEndedDelay = false;
    save_frames = 0;
    restore_frames = 0;
    delay_frames = 60;
    enqueue_delay_frames = 5;
    mapHasEndedDelayFrames = 0;
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

    pthread_mutex_init(&playerDeathQueueLock, NULL);
    pthread_mutex_init(&collisionListLock, NULL);
    pthread_mutex_init(&cmdbufflistlock, NULL);

    char* root_dir = getenv("PWD");
    size_t max_path_length = 1024;

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

    snprintf(server_srv_fullpath, max_path_length, "%s/synergy/bin/server_srv.so", root_dir);
    snprintf(engine_srv_fullpath, max_path_length, "%s/bin/engine_srv.so", root_dir);
    snprintf(scenefilecache_fullpath, max_path_length, "%s/bin/scenefilecache.so", root_dir);
    snprintf(soundemittersystem_fullpath, max_path_length, "%s/bin/soundemittersystem.so", root_dir);
    snprintf(dedicated_srv_fullpath, max_path_length, "%s/bin/dedicated_srv.so", root_dir);
    snprintf(soundemittersystem_srv_fullpath, max_path_length, "%s/bin/soundemittersystem_srv.so", root_dir);
    snprintf(materialsystem_srv_fullpath, max_path_length, "%s/bin/materialsystem_srv.so", root_dir);
    snprintf(studiorender_srv_fullpath, max_path_length, "%s/bin/studiorender_srv.so", root_dir);
    snprintf(vphysics_srv_fullpath, max_path_length, "%s/bin/vphysics_srv.so", root_dir);
    snprintf(datacache_srv_fullpath, max_path_length, "%s/bin/datacache_srv.so", root_dir);
    snprintf(sdktools_path, max_path_length, "/extensions/sdktools.ext.2.sdk2013.so");

    Library* engine_srv_lib = FindLibrary(engine_srv_fullpath, false);
    Library* datacache_srv_lib = FindLibrary(datacache_srv_fullpath, false);
    Library* dedicated_srv_lib = FindLibrary(dedicated_srv_fullpath, false);
    Library* materialsystem_srv_lib = FindLibrary(materialsystem_srv_fullpath, false);
    Library* vphysics_srv_lib = FindLibrary(vphysics_srv_fullpath, false);
    Library* scenefilecache_lib = FindLibrary(scenefilecache_fullpath, false);
    Library* soundemittersystem_lib = FindLibrary(soundemittersystem_fullpath, false);
    Library* soundemittersystem_srv_lib = FindLibrary(soundemittersystem_srv_fullpath, false);
    Library* studiorender_srv_lib = FindLibrary(studiorender_srv_fullpath, false);
    Library* server_srv_lib = FindLibrary(server_srv_fullpath, false);
    Library* sdktools_lib = FindLibrary(sdktools_path, true);

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
    mallocAllocations = AllocateMallocRefList();
    playerSaveList = AllocatePlayerSaveList();
    entityDeleteList = AllocateValuesList();
    playerDeathQueue = AllocateValuesList();
    collisionList = AllocateValuesList();
    cmdbufflist = AllocateValuesList();
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
    VehicleRollermineFunctionAddr = server_srv + 0x00654970;
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

    pEdtLoadFunc = (pTwoArgProt)EdtLoadFuncAddr;
    pHostChangelevelFunc = (pThreeArgProt)HostChangelevel;
    pFlushFunc = (pThreeArgProt)Flush;
    pSpawnServerFunc = (pThreeArgProt)SpawnServer;
    LookupPoseParameter = (pThreeArgProt)LookupPoseParameterAddr;
    sub_654260 = (pOneArgProt)sub_654260_addr;
    sub_628F00 = (pOneArgProt)sub_628F00_addr;
    SaveGameState = (pThreeArgProt)SaveGameStateAddr;
    pTransitionRestoreMainCall = (pFourArgProt)TransitionRestoreMainCallOrigAddr;
    pCallVehicleRollermineFunction = (pOneArgProt)VehicleRollermineFunctionAddr;
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

    PopulateHookPointers();
    PopulateHookExclusionLists();

    ApplyPatches();
    ApplyPatchesSpecific();

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

    HookFunctions();
    HookFunctionsSpecific();

    RestoreMemoryProtections();
    rootconsole->ConsolePrint("----------------------  " SMEXT_CONF_NAME " loaded!" "  ----------------------");
}

void PopulateHookPointers()
{
    UTIL_Remove__External = (pOneArgProt)((uint32_t)Hooks::HookEntityDelete);
    FindEntityByClassnameHook__External = (pThreeArgProt)((uint32_t)Hooks::FindEntityByClassnameHook);
    CreateEntityByNameHook__External = (pTwoArgProt)((uint32_t)Hooks::CreateEntityByNameHook);
    CleanupDeleteListHook__External = (pOneArgProt)((uint32_t)Hooks::CleanupDeleteListHook);
    PlayerSpawnHook__External = (pThreeArgProt)((uint32_t)Hooks::PlayerSpawnHook);
    UTIL_RemoveInternal__External = (pOneArgProt)((uint32_t)Hooks::UTIL_RemoveHookFailsafe);
    MainPlayerRestore__External = (pThreeArgProt)((uint32_t)Hooks::MainPlayerRestoreHook);
}

void ApplyPatches()
{
    uint32_t nop_patch_list[128] = 
    {
        0x009924F3,0x3B,0x009927E1,0xF,0x008C1DC0,0x8,
        0x0096026E,5,0x00815EF0,5,

        //0x00739AF6,5,0x00739B37,5,0x00739B3C,5,

        //remove broken code
        0x009B56B4,5,0x00B027D5,8,

        //post systems remove
        0x00739B37,5,

        //restore
        0x00AF4361,0x14,

        //bug
        //0x00B02DCF,6

        //CleanupDeleteList calls
        /*0x00739AF1,5,0x00A316F0,5,0x00739B48,5*/
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

    uint32_t fix_ai = server_srv + 0x005703B2;
    *(uint8_t*)(fix_ai) = 0xB8;
    *(uint8_t*)(fix_ai+1) = 0xFF;
    *(uint8_t*)(fix_ai+2) = 0xFF;
    *(uint8_t*)(fix_ai+3) = 0xFF;

    uint32_t save_fix = server_srv + 0x004AF323;
    offset = (uint32_t)Hooks::MainSaveEntitiesFunc - save_fix - 5;
    *(uint32_t*)(save_fix+1) = offset;

    uint32_t restore_fix = server_srv + 0x00AF4380;
    offset = (uint32_t)Hooks::RepairPlayerRestore - restore_fix - 5;
    *(uint32_t*)(restore_fix+1) = offset;

    uint32_t hunter_fix_crash = server_srv + 0x005799BB;
    offset = (uint32_t)Hooks::HunterCrashFixTwo - hunter_fix_crash - 5;
    *(uint32_t*)(hunter_fix_crash+1) = offset;

    uint32_t jmp_vphys = server_srv + 0x00499346;
    *(uint8_t*)(jmp_vphys) = 0xEB;

    uint32_t vphysicsupdatepatch = server_srv + 0x003D97EB;
    memset((void*)vphysicsupdatepatch, 0x90, 12);

    // SET STACK POINTER
    *(uint8_t*)(vphysicsupdatepatch) = 0x89;
    *(uint8_t*)(vphysicsupdatepatch+1) = 0x1C;
    *(uint8_t*)(vphysicsupdatepatch+2) = 0x24;

    // SET HOOK ADDRESS
    vphysicsupdatepatch = server_srv + 0x003D97F2;
    offset = (uint32_t)Hooks::VphysicsUpdateWarningHook - vphysicsupdatepatch - 5;
    *(uint8_t*)(vphysicsupdatepatch) = 0xE8;
    *(uint32_t*)(vphysicsupdatepatch+1) = offset;

    uint32_t vphysicsupdatepatch_two = server_srv + 0x003D9822;
    memset((void*)vphysicsupdatepatch_two, 0x90, 7);

    // SET STACK POINTER
    *(uint8_t*)(vphysicsupdatepatch_two) = 0x89;
    *(uint8_t*)(vphysicsupdatepatch_two+1) = 0x1C;
    *(uint8_t*)(vphysicsupdatepatch_two+2) = 0x24;

    // SET HOOK ADDRESS
    vphysicsupdatepatch_two = server_srv + 0x003D9852;
    offset = (uint32_t)Hooks::VphysicsUpdateWarningHook - vphysicsupdatepatch_two - 5;
    *(uint8_t*)(vphysicsupdatepatch_two) = 0xE8;
    *(uint32_t*)(vphysicsupdatepatch_two+1) = offset;

    uint32_t vphysicsupdatepatch_three = server_srv + 0x00657ED3;
    memset((void*)vphysicsupdatepatch_three, 0x90, 7);

    // SET STACK POINTER
    *(uint8_t*)(vphysicsupdatepatch_three) = 0x89;
    *(uint8_t*)(vphysicsupdatepatch_three+1) = 0x1C;
    *(uint8_t*)(vphysicsupdatepatch_three+2) = 0x24;

    // SET HOOK ADDRESS
    vphysicsupdatepatch_three = server_srv + 0x00657EE3;
    offset = (uint32_t)Hooks::VphysicsUpdateWarningHook - vphysicsupdatepatch_three - 5;
    *(uint8_t*)(vphysicsupdatepatch_three) = 0xE8;
    *(uint32_t*)(vphysicsupdatepatch_three+1) = offset;

    uint32_t vphysicsupdatepatch_four = server_srv + 0x00658E2C;
    memset((void*)vphysicsupdatepatch_four, 0x90, 7);

    // SET STACK POINTER
    *(uint8_t*)(vphysicsupdatepatch_four) = 0x89;
    *(uint8_t*)(vphysicsupdatepatch_four+1) = 0x34;
    *(uint8_t*)(vphysicsupdatepatch_four+2) = 0x24;

    // SET HOOK ADDRESS
    vphysicsupdatepatch_four = server_srv + 0x00658E33;
    offset = (uint32_t)Hooks::VphysicsUpdateWarningHook - vphysicsupdatepatch_four - 5;
    *(uint8_t*)(vphysicsupdatepatch_four) = 0xE8;
    *(uint32_t*)(vphysicsupdatepatch_four+1) = offset;

    /*uint32_t player_think_patch_two = server_srv + 0x0098FFF3;
    *(uint8_t*)(player_think_patch_two) = 0xE9;
    *(uint32_t*)(player_think_patch_two+1) = 0xC8;*/

    uint32_t dropship_patch_one = server_srv + 0x0085F22C;
    offset = (uint32_t)Hooks::LookupPoseParameterDropshipHook - dropship_patch_one - 5;
    *(uint32_t*)(dropship_patch_one+1) = offset;

    uint32_t dropship_patch_two = server_srv + 0x0085F266;
    offset = (uint32_t)Hooks::LookupPoseParameterDropshipHook - dropship_patch_two - 5;
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

    /*uint32_t fix_null_ent_crash_cfire = server_srv + 0x007159A4;
    *(uint8_t*)(fix_null_ent_crash_cfire) = 0xE9;
    *(uint32_t*)(fix_null_ent_crash_cfire+1) = 0x2D;*/

    /*uint32_t save_system_ent_list_patch = server_srv + 0x004AF339;
    *(uint8_t*)(save_system_ent_list_patch) = 0xE9;
    *(uint32_t*)(save_system_ent_list_patch+1) = 0xFB;*/

    /*uint32_t jmp_to_touch = server_srv + 0x0047D7C2;
    *(uint8_t*)(jmp_to_touch) = 0xE9;
    *(uint32_t*)(jmp_to_touch+1) = 0xD;*/

    uint32_t jmp_to_fix_heli = server_srv + 0x00960275;
    *(uint8_t*)(jmp_to_fix_heli) = 0xE9;
    *(uint32_t*)(jmp_to_fix_heli+1) = 0x44E;

    uint32_t jmp_fix_heli_two = server_srv + 0x00815EF7;
    *(uint8_t*)(jmp_fix_heli_two) = 0xEB;

    //get rid of message spam on sound pointer
    uint32_t it_said_it_causes_corruption = server_srv + 0x00A5436D;
    *(uint8_t*)(it_said_it_causes_corruption) = 0xC3;

    uint32_t begin_map_load_patch = datacache_srv + 0x0005A4CC;
    *(uint32_t*)(begin_map_load_patch) = (uint32_t)Hooks::FrameLockHook;

    /*uint32_t packet_crash_exploit_patch = engine_srv + 0x001DBE8E;
    *(uint8_t*)(packet_crash_exploit_patch) = 0xEB;*/

    uint32_t null_manhack_patch = server_srv + 0x008C1DC8;
    *(uint8_t*)(null_manhack_patch) = 0xE9;
    *(uint32_t*)(null_manhack_patch+1) = 0xAC;

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

    /*uint32_t player_restore_full_remove = server_srv + 0x00AF4E12;
    *(uint8_t*)(player_restore_full_remove) = 0xE9;
    *(uint32_t*)(player_restore_full_remove+1) = 0x89;

    uint32_t remove_player_file_restoring = server_srv + 0x00AF4E79;
    memset((void*)remove_player_file_restoring, 0x90, 5);
    *(uint16_t*)(remove_player_file_restoring) = 0xC031;*/

    //uint32_t mainPlayersRestorePatch = server_srv + 0x00AF4124;
    //*(uint8_t*)(mainPlayersRestorePatch) = 0xEB;

    /*uint32_t patch_player_restore_asm = server_srv + 0x00AF408B;
    *(uint8_t*)(patch_player_restore_asm) = 0xE9;
    *(uint32_t*)(patch_player_restore_asm+1) = -0x1F7;*/

    uint32_t clientPutInServerRestoreCancel = server_srv + 0x00B030A1;
    *(uint8_t*)(clientPutInServerRestoreCancel) = 0xEB;

    uint32_t clientActiveRestoreCancel = server_srv + 0x00B031F7;
    *(uint8_t*)(clientActiveRestoreCancel) = 0xEB;

    uint32_t hook_game_frame_delete_list = server_srv + 0x00739B32;
    offset = (uint32_t)Hooks::SimulateEntitiesHook - hook_game_frame_delete_list - 5;
    *(uint32_t*)(hook_game_frame_delete_list+1) = offset;

    uint32_t hook_event_queue = server_srv + 0x00739B3C;
    offset = (uint32_t)Hooks::ServiceEventQueueHook - hook_event_queue - 5;
    *(uint32_t*)(hook_event_queue+1) = offset;

    /*uint32_t changelevel_patch = server_srv + 0x004CB2FE;
    memset((void*)changelevel_patch, 0x90, 0xF);
    offset = (uint32_t)Hooks::IsAllowChangelevel - changelevel_patch - 5;
    *(uint8_t*)(changelevel_patch) = 0xE8;
    *(uint32_t*)(changelevel_patch+1) = offset;

    *(uint8_t*)(changelevel_patch+5) = 0x83;
    *(uint8_t*)(changelevel_patch+6) = 0xF8;
    *(uint8_t*)(changelevel_patch+7) = 0x02;

    uint32_t changelevel_patch_two = server_srv + 0x004CB306;
    offset = (server_srv + 0x004CB4D5) - changelevel_patch_two - 6;
    *(uint8_t*)(changelevel_patch_two) = 0x0F;
    *(uint8_t*)(changelevel_patch_two+1) = 0x84;
    *(uint32_t*)(changelevel_patch_two+2) = offset;*/
}

uint32_t Hooks::HunterCrashFixTwo(uint32_t arg0)
{
    pOneArgProt pDynamicOneArgFunc;

    if(server_sleeping)
    {
        rootconsole->ConsolePrint("Server Sleeping!");
        return 0;
    }

    pDynamicOneArgFunc = (pOneArgProt)(server_srv + 0x00579750);
    return pDynamicOneArgFunc(arg0);
}

uint32_t Hooks::MainPlayerRestoreHook(uint32_t arg0, uint32_t arg1, uint32_t arg2)
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

uint32_t Hooks::AutosaveLoadHook(uint32_t arg0, uint32_t arg1, uint32_t arg2)
{
    return 0;
}

uint32_t Hooks::EmptyCall()
{
    return 0;
}

uint32_t Hooks::UTIL_PrecacheOther_Hook(uint32_t arg0, uint32_t arg1)
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

        InstaKill(piVar2, false);
    }

    return 0;
}

uint32_t Hooks::HookEntityDelete(uint32_t arg0)
{
    RemoveEntityNormal(arg0, true);
    return 0;
}

uint32_t Hooks::UTIL_RemoveHookFailsafe(uint32_t arg0)
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
    uint32_t object = GetCBaseEntity(refHandle);

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

uint32_t Hooks::CleanupDeleteListHook(uint32_t arg0)
{
    if(disable_delete_list) return 0;

    pOneArgProt pDynamicOneArgFunc;
    pTwoArgProt pDynamicTwoArgFunc;

    //CleanupDeleteList
    pDynamicOneArgFunc = (pOneArgProt)(server_srv + 0x006B2510);
    return pDynamicOneArgFunc(arg0);
}

uint32_t Hooks::CallocHook(uint32_t nitems, uint32_t size)
{
    if(nitems <= 0) return (uint32_t)calloc(nitems, size);

    uint32_t enlarged_size = nitems*5.0;
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

    uint32_t newRef = (uint32_t)malloc(size*5.0);
    
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
    uint32_t new_ref = (uint32_t)realloc((void*)old_ptr, new_size*5.0);

    /*void* returnAddr = __builtin_return_address(0);
    RemoveAllocationRef(mallocAllocations, (void*)old_ptr, true);
    MallocRef* new_ref_value = CreateNewMallocRef((void*)new_ref, (void*)new_size, (void*)((uint32_t)returnAddr - 5), (void*)"malloc");
    InsertToMallocRefList(mallocAllocations, new_ref_value, true);*/

    return new_ref;
}

uint32_t Hooks::OperatorNewHook(uint32_t size)
{
    if(size <= 0) return (uint32_t)malloc(size);
    uint32_t newRef = (uint32_t)malloc(size*5.0);
    //rootconsole->ConsolePrint("malloc() ref: [%X] size: [%X] list_size [%d]", newRef, size, MallocRefListSize(mallocAllocations));
    //rootconsole->ConsolePrint("malloc() ref: [%X] size: [%X]", newRef, size);

    /*void* returnAddr = __builtin_return_address(0);
    MallocRef* new_ref_value = CreateNewMallocRef((void*)newRef, (void*)size, (void*)((uint32_t)returnAddr - 5), (void*)"operator_new");
    InsertToMallocRefList(mallocAllocations, new_ref_value, true);*/

    return newRef;
}

uint32_t Hooks::OperatorNewArrayHook(uint32_t size)
{
    if(size <= 0) return (uint32_t)malloc(size);
    uint32_t newRef = (uint32_t)malloc(size*5.0);
    //rootconsole->ConsolePrint("malloc() ref: [%X] size: [%X] list_size [%d]", newRef, size, MallocRefListSize(mallocAllocations));
    //rootconsole->ConsolePrint("malloc() ref: [%X] size: [%X]", newRef, size);

    /*void* returnAddr = __builtin_return_address(0);
    MallocRef* new_ref_value = CreateNewMallocRef((void*)newRef, (void*)size, (void*)((uint32_t)returnAddr - 5), (void*)"operator_new_array");
    InsertToMallocRefList(mallocAllocations, new_ref_value, true);*/

    return newRef;
}

uint32_t Hooks::FreeHook(uint32_t ref_tofree)
{
    if(ref_tofree == 0)
        return 0;

    operator delete((void*)ref_tofree);
    return 0;

    void* returnAddr = __builtin_return_address(0);

    MallocRef* searchResult = SearchForMallocRef(mallocAllocations, (void*)ref_tofree, NULL);

    if(searchResult)
    {
        if(searchResult->alloc_type == (void*)"malloc")
        {
            free((void*)ref_tofree);
        }
        else if(searchResult->alloc_type == (void*)"operator_new")
        {
            operator delete ((void*)ref_tofree);
            rootconsole->ConsolePrint("     Used wrong deallocation type - corrected!");
            rootconsole->ConsolePrint("used free() - should be operator delete ->[%X] alloc_src->[%X]\n\n", ((uint32_t)returnAddr - 5), searchResult->alloc_location);
        }
        else
        {
            operator delete[] ((void*)ref_tofree);
            rootconsole->ConsolePrint("     Used wrong deallocation type - corrected!");
            rootconsole->ConsolePrint("used free() - should be operator delete[] ->[%X] alloc_src->[%X]\n\n", ((uint32_t)returnAddr - 5), searchResult->alloc_location);
        }

        RemoveAllocationRef(mallocAllocations, (void*)ref_tofree, NULL);
        return 0;
    }

    //rootconsole->ConsolePrint("error in free detected - [%X]", ((uint32_t)returnAddr - 5));
    free((void*)ref_tofree);
    return 0;
}

uint32_t Hooks::DeleteOperatorHook(uint32_t ref_tofree)
{
    if(ref_tofree == 0)
        return 0;
        
    free((void*)ref_tofree);
    return 0;

    void* returnAddr = __builtin_return_address(0);

    MallocRef* searchResult = SearchForMallocRef(mallocAllocations, (void*)ref_tofree, NULL);

    if(searchResult)
    {
        if(searchResult->alloc_type == (void*)"malloc")
        {
            free((void*)ref_tofree);
            rootconsole->ConsolePrint("     Used wrong deallocation type - corrected!");
            rootconsole->ConsolePrint("used operator delete - should be free() ->[%X] alloc_src->[%X]\n\n", ((uint32_t)returnAddr - 5), searchResult->alloc_location);
        }
        else if(searchResult->alloc_type == (void*)"operator_new")
        {
            operator delete ((void*)ref_tofree);
        }
        else
        {
            operator delete[] ((void*)ref_tofree);
            rootconsole->ConsolePrint("     Used wrong deallocation type - corrected!");
            rootconsole->ConsolePrint("used operator delete - should be operator delete[] ->[%X] alloc_src->[%X]\n\n", ((uint32_t)returnAddr - 5), searchResult->alloc_location);
        }

        RemoveAllocationRef(mallocAllocations, (void*)ref_tofree, NULL);
        return 0;
    }

    //rootconsole->ConsolePrint("error in delete detected - [%X]", ((uint32_t)returnAddr - 5));
    operator delete ((void*)ref_tofree);
    return 0;
}

uint32_t Hooks::DeleteOperatorArrayHook(uint32_t ref_tofree)
{
    if(ref_tofree == 0)
        return 0;

    free((void*)ref_tofree);
    return 0;
        
    void* returnAddr = __builtin_return_address(0);

    MallocRef* searchResult = SearchForMallocRef(mallocAllocations, (void*)ref_tofree, NULL);

    if(searchResult)
    {
        if(searchResult->alloc_type == (void*)"malloc")
        {
            free((void*)ref_tofree);
            rootconsole->ConsolePrint("     Used wrong deallocation type - corrected!");
            rootconsole->ConsolePrint("used operator delete[] - should be free() ->[%X] alloc_src->[%X]\n\n", ((uint32_t)returnAddr - 5), searchResult->alloc_location);
        }
        else if(searchResult->alloc_type == (void*)"operator_new")
        {
            operator delete ((void*)ref_tofree);
            rootconsole->ConsolePrint("     Used wrong deallocation type - corrected!");
            rootconsole->ConsolePrint("used operator delete[] - should be operator delete ->[%X] alloc_src->[%X]\n\n", ((uint32_t)returnAddr - 5), searchResult->alloc_location);
        }
        else
        {
            operator delete[] ((void*)ref_tofree);
        }

        RemoveAllocationRef(mallocAllocations, (void*)ref_tofree, NULL);
        return 0;
    }
    
    //rootconsole->ConsolePrint("error in delete[] detected - [%X]", ((uint32_t)returnAddr - 5));
    operator delete[] ((void*)ref_tofree);
    return 0;
}

uint32_t Hooks::MemcpyHook(uint32_t dest, uint32_t src, uint32_t size)
{
    MallocRef* searchResult = SearchForMallocRefInRange(mallocAllocations, (void*)dest, NULL);

    if(searchResult)
    {
        uint32_t max_write_size = (uint32_t)searchResult->ref+(uint32_t)searchResult->ref_size;
        uint32_t actual_write_size = dest+size;

        if(actual_write_size > max_write_size)
        {
            void* one = __builtin_return_address(0);
            uint32_t one_file_offset = (uint32_t)one - server_srv;

            //rootconsole->ConsolePrint("orig ref: [%X] offset ref: [%X] correct size: [%X] promgrmr size: [%X]\n", searchResult->ref, dest, searchResult->ref_size, size);

            //save resource to heap corruption list
            /*Value* new_heap_corruption = CreateNewValue(searchResult->alloc_location);
            InsertToValuesList(heapCorruptionsList, new_heap_corruption);*/
            rootconsole->ConsolePrint("-<>-     Buffer overflow detected - prevented heap corruption!!! memcpy() alloc_src: [%X]", searchResult->alloc_location);
            rootconsole->ConsolePrint("alloc_ref: [%X] last_func: [%X] base_ref: [%X] max_size: [%X] dest: [%X] dest_size: [%X]\n\n", searchResult->ref, one_file_offset, searchResult->ref, searchResult->ref_size, dest, size);

            /*uint32_t pointer_offset = dest - (uint32_t)searchResult->ref;
            uint32_t overflow_size = actual_write_size - max_write_size;
            rootconsole->ConsolePrint("FREEING LEAK!");
            free(searchResult->ref);
            rootconsole->ConsolePrint("FREED LEAK!");
            uint32_t new_ptr = (uint32_t)realloc(searchResult->ref, (uint32_t)searchResult->ref_size+overflow_size);

            rootconsole->ConsolePrint("updated old: [%X] to new: [%X]", searchResult->ref, new_ptr);

            searchResult->ref = (void*)new_ptr;
            searchResult->ref_size = (void*)size;

            uint32_t save_node = (uint32_t)searchResult->save_point;
            uint32_t offset = (uint32_t)searchResult->save_point_offset;*/

            //uint32_t returnVal = (uint32_t)memcpy((void*)(new_ptr+pointer_offset), (const void*)src, (size_t)size);
            //*(uint32_t*)(save_node+offset) = new_ptr;
            //return returnVal;
            return 0;
        }
    }

    return (uint32_t)memcpy((void*)dest, (const void*)src, (size_t)size);
}

uint32_t Hooks::MemsetHook(uint32_t dest, uint32_t byte, uint32_t size)
{   
    MallocRef* searchResult = SearchForMallocRefInRange(mallocAllocations, (void*)dest, NULL);

    if(searchResult)
    {
        uint32_t max_write_size = (uint32_t)searchResult->ref+(uint32_t)searchResult->ref_size;
        uint32_t actual_write_size = dest+size;

        if(actual_write_size > max_write_size)
        {
            void* one = __builtin_return_address(0);
            uint32_t one_file_offset = (uint32_t)one - server_srv;
            //rootconsole->ConsolePrint("orig ref: [%X] offset ref: [%X] correct size: [%X] promgrmr size: [%X]\n", searchResult->ref, dest, searchResult->ref_size, size);

            //save resource to heap corruption list
            /*Value* new_heap_corruption = CreateNewValue(searchResult->alloc_location);
            InsertToValuesList(heapCorruptionsList, new_heap_corruption);*/
            rootconsole->ConsolePrint("-<>-     Buffer overflow detected - prevented heap corruption!!! memset() alloc_src: [%X]", searchResult->alloc_location);
            rootconsole->ConsolePrint("alloc_ref: [%X] last_func: [%X] base_ref: [%X] max_size: [%X] dest: [%X] dest_size: [%X]\n\n", searchResult->ref, one_file_offset, searchResult->ref, searchResult->ref_size, dest, size);

            /*uint32_t pointer_offset = dest - (uint32_t)searchResult->ref;
            uint32_t overflow_size = actual_write_size - max_write_size;
            uint32_t new_ptr = (uint32_t)realloc(searchResult->ref, (uint32_t)searchResult->ref_size+overflow_size);
            searchResult->ref = (void*)new_ptr;
            searchResult->ref_size = (void*)size;

            uint32_t save_node = (uint32_t)searchResult->save_point;
            uint32_t offset = (uint32_t)searchResult->save_point_offset;

            uint32_t returnVal = (uint32_t)memset((void*)(new_ptr+pointer_offset), (int)byte, (size_t)size);
            *(uint32_t*)(save_node+offset) = new_ptr;
            rootconsole->ConsolePrint("updated old: [%X] to new: [%X]", searchResult->ref, new_ptr);
            return returnVal;*/
            return 0;
        }
    }

    return (uint32_t)memset((void*)dest, (int)byte, (size_t)size);
}

uint32_t Hooks::MemmoveHook(uint32_t dest, uint32_t src, uint32_t size)
{   
    MallocRef* searchResult = SearchForMallocRefInRange(mallocAllocations, (void*)dest, NULL);

    if(searchResult)
    {
        uint32_t max_write_size = (uint32_t)searchResult->ref+(uint32_t)searchResult->ref_size;
        uint32_t actual_write_size = dest+size;

        if(actual_write_size > max_write_size)
        {
            //save resource to heap corruption list
            /*Value* new_heap_corruption = CreateNewValue(searchResult->alloc_location);
            InsertToValuesList(heapCorruptionsList, new_heap_corruption);*/
            rootconsole->ConsolePrint("-<>-     Buffer overflow detected - prevented heap corruption!!! memmove() alloc_src: [%X]", searchResult->alloc_location);
            return 0;
        }
    }

    return (uint32_t)memmove((void*)dest, (const void*)src, (size_t)size);
}

uint32_t Hooks::StrncpyHook(uint32_t dest, uint32_t src, uint32_t size)
{   
    MallocRef* searchResult = SearchForMallocRefInRange(mallocAllocations, (void*)dest, NULL);

    if(searchResult)
    {
        uint32_t max_write_size = (uint32_t)searchResult->ref+(uint32_t)searchResult->ref_size;
        uint32_t actual_write_size = dest+size;

        if(actual_write_size > max_write_size)
        {
            //void* one = __builtin_return_address(0);
            //void* two = __builtin_return_address(1);
            //void* three = __builtin_return_address(2);
            //void* four = __builtin_return_address(3);

            //rootconsole->ConsolePrint("call stack: [%X] [%X] [%X] [%X]", one);//, two, three, four);
            //rootconsole->ConsolePrint("orig ref: [%X] offset ref: [%X] correct size: [%X] promgrmr size: [%X]\n\n", searchResult->ref, dest, searchResult->ref_size, size);
            /*Value* new_heap_corruption = CreateNewValue(searchResult->alloc_location);
            InsertToValuesList(heapCorruptionsList, new_heap_corruption);*/
            rootconsole->ConsolePrint("-<>-     Buffer overflow detected - prevented heap corruption!!! strncpy() alloc_src: [%X]", searchResult->alloc_location);
            return 0;
        }
    }

    return (uint32_t)strncpy((char*)dest, (const char*)src, (size_t)size);
}

uint32_t Hooks::StrcpyHook(uint32_t dest, uint32_t src)
{   
    MallocRef* searchResult = SearchForMallocRefInRange(mallocAllocations, (void*)dest, NULL);

    if(searchResult)
    {
        uint32_t max_write_size = (uint32_t)searchResult->ref+(uint32_t)searchResult->ref_size;
        uint32_t actual_write_size = strlen((char*)src)+1;

        if(actual_write_size > max_write_size)
        {
            //void* one = __builtin_return_address(0);
            //void* two = __builtin_return_address(1);
            //void* three = __builtin_return_address(2);
            //void* four = __builtin_return_address(3);

            //rootconsole->ConsolePrint("call stack: [%X] [%X] [%X] [%X]", one);//, two, three, four);
            //rootconsole->ConsolePrint("orig ref: [%X] offset ref: [%X] correct size: [%X] promgrmr size: [%X]\n\n", searchResult->ref, dest, searchResult->ref_size, size);
            /*Value* new_heap_corruption = CreateNewValue(searchResult->alloc_location);
            InsertToValuesList(heapCorruptionsList, new_heap_corruption);*/
            rootconsole->ConsolePrint("-<>-     Buffer overflow detected - prevented heap corruption!!! strcpy() alloc_src: [%X]", searchResult->alloc_location);
            return 0;
        }
    }

    return (uint32_t)strcpy((char*)dest, (const char*)src);
}

uint32_t Hooks::FrameLockHook(uint32_t arg0)
{
    pOneArgProt pDynamicOneArgFunc;

    pDynamicOneArgFunc = (pOneArgProt)(datacache_srv + 0x00038060);
    return pDynamicOneArgFunc(arg0);
}

void PatchRestore()
{
    int length = 5;

    uint32_t restore_call_one = server_srv + 0x0073C780;
    uint32_t restore_call_two = server_srv + 0x00AF46AB;
    uint32_t restore_call_three = server_srv + 0x004D9A40;

    uint32_t offset_three = (uint32_t)Hooks::RestoreOverride - restore_call_three - 5;

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
    uint32_t offset = (uint32_t)Hooks::DirectMallocHookDedicatedSrv - start - 5;

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

    uint32_t offset = (uint32_t)Hooks::SaveHookDirectMalloc - patch_location_one - 5;

    *(uint8_t*)(patch_location_one) = 0xE8;
    *(uint32_t*)(patch_location_one + 1) = offset;

    *(uint8_t*)(patch_location_two) = 0xEB;

    rootconsole->ConsolePrint("--------------------- Save/Restore part 1 hooked ---------------------");
}

void HookSaveRestoreTwo()
{
    int length = 5;

    uint32_t start = server_srv + 0x004B04B7;
    uint32_t offset = (uint32_t)Hooks::SaveHookDirectMalloc - start - 5;

    *(uint8_t*)(start) = 0xE8;
    *(uint32_t*)(start+1) = offset;

    rootconsole->ConsolePrint("--------------------- Save/Restore part 2 hooked ---------------------");
}

void HookSaveRestoreThree()
{
    int length = 5;

    uint32_t start = server_srv + 0x004B0442;
    uint32_t offset = (uint32_t)Hooks::SaveHookDirectRealloc - start - 5;

    *(uint8_t*)(start) = 0xE8;
    *(uint32_t*)(start+1) = offset;

    rootconsole->ConsolePrint("--------------------- Save/Restore part 3 hooked ---------------------");
}

void HookSavingOne()
{
    int length = 5;

    uint32_t start = server_srv + 0x004B0262;
    uint32_t offset = (uint32_t)Hooks::SaveHookDirectRealloc - start - 5;

    *(uint8_t*)(start) = 0xE8;
    *(uint32_t*)(start+1) = offset;

    rootconsole->ConsolePrint("--------------------- Save part 1 hooked ---------------------");
}

void HookSavingTwo()
{
    int length = 5;

    uint32_t start = server_srv + 0x004B02A3;
    uint32_t offset = (uint32_t)Hooks::SaveHookDirectMalloc - start - 5;

    *(uint8_t*)(start) = 0xE8;
    *(uint32_t*)(start+1) = offset;

    rootconsole->ConsolePrint("--------------------- Save part 2 hooked ---------------------");
}

void HookEdtSystem()
{
    int length = 5;

    uint32_t start = server_srv + 0x00AEFC34;
    uint32_t offset = (uint32_t)Hooks::EdtSystemHookFunc - start - 5;

    *(uint8_t*)(start) = 0xE8;
    *(uint32_t*)(start+1) = offset;

    rootconsole->ConsolePrint("--------------------- EDT system hooked ---------------------");   
}

void HookSpawnServer()
{
    int length = 5;

    uint32_t start = engine_srv + 0x0012AA56;
    uint32_t offset = (uint32_t)Hooks::SpawnServerHookFunc - start - 5;

    *(uint8_t*)(start) = 0xE8;
    *(uint32_t*)(start+1) = offset;


    rootconsole->ConsolePrint("--------------------- SpawnServer hooked ---------------------");   
}

void HookHostChangelevel()
{
    int length = 5;

    uint32_t start = engine_srv + 0x00136865;
    uint32_t offset = (uint32_t)Hooks::HostChangelevelHook - start - 5;

    *(uint8_t*)(start) = 0xE8;
    *(uint32_t*)(start+1) = offset;


    rootconsole->ConsolePrint("--------------------- Changelevel hooked ---------------------");   
}

void PatchOthers()
{
    int length_one = 5;
    int length_two = 145;
    int length_three = 5;
    int length_four = 4;
    int length_five = 2;
    int length_six = 2;
    int length_seven = 0x6;
    int length_eight = 5;
    int length_nine = 3;
    int length_ten = 5;
    int length_eleven = 5;
    int length_twelve = 5;
    int length_thirteen = 5;
    int length_fourteen = 5;
    int length_fifthteen = 4;
    int length_sixteen = 5;
    int length_seventeen = 4;
    int length_eighteen = 6;
    int length_nineteen = 0x11;
    int length_twenty = 0xE;
    int length_twentyone = 2;
    int length_enlarge = 5;

    uint32_t patch_location_one = server_srv + 0x004AF470;
    uint32_t patch_location_two = server_srv + 0x00AF24F8;
    uint32_t patch_location_three = engine_srv + 0x00148ACD;
    uint32_t patch_location_four = server_srv + 0x00D8AABC;
    uint32_t patch_location_five = server_srv + 0x008B6787;
    uint32_t patch_location_six = server_srv +  0x009AFD30;
    uint32_t patch_location_seven = server_srv + 0x00AF4918;
    uint32_t patch_location_eight = server_srv + 0x008B675F;
    uint32_t patch_location_nine = server_srv + 0x008B6796;
    uint32_t patch_location_ten = server_srv + 0x0073C7CB;
    uint32_t patch_location_eleven = server_srv + 0x00B02FDC;
    uint32_t patch_location_twelve = server_srv + 0x004AE993;
    uint32_t patch_location_fourteen = server_srv + 0x00AF4D91;
    uint32_t patch_location_fifthteen = server_srv + 0x00D65010;
    uint32_t patch_location_sixteen = engine_srv + 0x001CB601;
    uint32_t patch_location_seventeen = server_srv + 0x00D65550;
    uint32_t patch_location_eighteen = server_srv + 0x008B2A8C;
    uint32_t patch_location_nineteen = server_srv + 0x008E7D32;
    uint32_t patch_location_twenty = server_srv + 0x008E7D35;
    uint32_t patch_location_twentyone = server_srv + 0x008B2A7B;


    uint32_t offset = 0;

    //*(uint8_t*)(patch_location_one + 1) = 0xA7;


    
    //uint32_t offset = (uint32_t)SaveRestoreMemManage - patch_location_two - 5;
    //*(uint8_t*)(patch_location_two) = 0xE8;
    //*(uint32_t*)(patch_location_two+1) = offset;
    //memset((void*)(patch_location_two+5), 0x90, length_two - 5);


    //*(uint8_t*)(patch_location_three) = 0xE9;
    //*(uint32_t*)(patch_location_three + 1) = -0x148;

    //memset((void*)patch_location_four, 0x90, length_four);
    //*(uint16_t*)(patch_location_four) = 0xC031;
    
    //*(uint32_t*)(patch_location_four) = (uint32_t)CreateEntityByNameHook;

    //*(uint32_t*)(patch_location_four) = (uint32_t)phook_53D20FPtr;

    //offset = ((uint32_t)pMapInitEndPtr) - patch_location_five - 5;
    //*(uint32_t*)(patch_location_five + 1) = offset;

    //memset((void*)patch_location_five, 0x90, length_five);
    //*(uint16_t*)(patch_location_five) = 0xC031;

    //memset((void*)patch_location_six, 0x90, length_six);

    //memset((void*)patch_location_seven, 0x90, length_seven);


    //offset = (uint32_t)Hooks::FixManhackCrash - patch_location_eight - 5;
    //*(uint32_t*)(patch_location_eight + 1) = offset;
    //*(uint8_t*)(patch_location_eight) = 0xE9;
    //*(uint32_t*)(patch_location_eight + 1) = 0x364;
    //memset((void*)patch_location_eight, 0x90, length_eight);
    //*(uint8_t*)(patch_location_eight) = 0xEB;
    //*(uint8_t*)(patch_location_eight+1) = -0x47;

    //memset((void*)patch_location_nine, 0x90, length_nine);
    //*(uint16_t*)(patch_location_nine) = 0xC031;

    //memset((void*)patch_location_eleven, 0x90, length_eleven);
    //*(uint16_t*)(patch_location_eleven) = 0xC031;

    offset = (uint32_t)Hooks::SV_TriggerMovedFix - patch_location_sixteen - 5;
    *(uint32_t*)(patch_location_sixteen + 1) = offset;


    *(uint8_t*)(patch_location_fourteen) = 0xE9;
    *(uint32_t*)(patch_location_fourteen + 1) = 0x1F0;

    uint32_t offset_three = (uint32_t)Hooks::TransitionEntityCreateCall - patch_location_twelve - 5;
    *(uint8_t*)(patch_location_twelve) = 0xE8;
    *(uint32_t*)(patch_location_twelve + 1) = offset_three;

    uint32_t offset_four = (uint32_t)Hooks::TransitionRestoreMain - patch_location_ten - 5;
    *(uint8_t*)(patch_location_ten) = 0xE8;
    *(uint32_t*)(patch_location_ten + 1) = offset_four;

    *(uint32_t*)(patch_location_fifthteen) = (uint32_t)Hooks::DoorCycleResolve;
    *(uint32_t*)(patch_location_seventeen) = (uint32_t)Hooks::DoorCycleResolve;

    //memset((void*)patch_location_eighteen, 0x90, length_eighteen);
    //offset = (uint32_t)Hooks::FixTransitionCrash - patch_location_eighteen - 5;
    //*(uint32_t*)(patch_location_eighteen) = 0xE8;
    //*(uint32_t*)(patch_location_eighteen + 1) = offset;

    memset((void*)patch_location_nineteen, 0x90, length_nineteen);
    *(uint8_t*)(patch_location_nineteen) = 0x89;
    *(uint8_t*)(patch_location_nineteen + 1) = 0x34;
    *(uint8_t*)(patch_location_nineteen + 2) = 0x24;


    uint32_t offset_five = (uint32_t)Hooks::VehicleRollermineCheck - patch_location_twenty - 5;
    *(uint8_t*)(patch_location_twenty) = 0xE8;
    *(uint32_t*)(patch_location_twenty + 1) = offset_five;
    *(uint16_t*)(patch_location_twenty + 5) = 0xC085;
    *(uint16_t*)(patch_location_twenty + 7) = 0x840F;
    *(uint32_t*)(patch_location_twenty + 9) = 0x2CD;

    //memset((void*)patch_location_twentyone, 0x90, length_twentyone);

    /*uint32_t patch_save_system_one = server_srv + 0x004AED69;
    offset = (uint32_t)pTransitionEntsHookPtr - patch_save_system_one - 5;
    *(uint32_t*)(patch_save_system_one+1) = offset;*/

    uint32_t patch_another_cycle = server_srv + 0x00A95A9E;
    *(uint8_t*)(patch_another_cycle) = 0xEB;

    uint32_t yet_another_cycle = server_srv + 0x00A8653B;
    *(uint8_t*)(yet_another_cycle) = 0xEB;

    uint32_t fix_null_crash_patch = server_srv + 0x0075B9B8;
    offset = (uint32_t)Hooks::FixNullCrash - fix_null_crash_patch - 5;
    *(uint32_t*)(fix_null_crash_patch+1) = offset;

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
    offset = (uint32_t)Hooks::memcpyNetworkHook - memcpy_hook_one - 5;
    *(uint32_t*)(memcpy_hook_one+1) = offset;

    uint32_t memcpy_hook_two = engine_srv + 0x0016ABE7;
    offset = (uint32_t)Hooks::memcpyNetworkHook - memcpy_hook_two - 5;
    *(uint32_t*)(memcpy_hook_two+1) = offset;

    rootconsole->ConsolePrint("--------------------- Other save system parts patched ---------------------");
}

uint32_t Hooks::SaveHookDirectMalloc(uint32_t size)
{
    uint32_t new_size = size*6.0;
    uint32_t ref = (uint32_t)malloc(new_size);
    memset((void*)ref, 0, new_size);
    //rootconsole->ConsolePrint("malloc() [Save/Restore Hook] " HOOK_MSG " size: [%d]", ref, new_size);

    Value* leak = CreateNewValue((void*)ref);
    InsertToValuesList(leakedResourcesSaveRestoreSystem, leak, NULL, false, false);

    return ref;
}

uint32_t Hooks::SaveHookDirectRealloc(uint32_t old_ptr, uint32_t new_size)
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

uint32_t Hooks::UpdateOnRemove(uint32_t arg0)
{
    pOneArgProt pDynamicOneArgFunc;

    char* classname = (char*)(*(uint32_t*)(arg0+0x68));

    normal_delete_counter++;

    //rootconsole->ConsolePrint("normal counter: [%d] [%s]", normal_delete_counter, classname);

    pDynamicOneArgFunc = (pOneArgProt)(server_srv + 0x0065BD80);
    return pDynamicOneArgFunc(arg0);
}

uint32_t Hooks::ParseMapEntities(uint32_t arg0, uint32_t arg1, uint32_t arg2)
{
    pOneArgProt pDynamicOneArgFunc;
    pThreeArgProt pDynamicThreeArgFunc;

    pDynamicThreeArgFunc = (pThreeArgProt)(server_srv + 0x009B09F0);
    uint32_t returnVal = pDynamicThreeArgFunc(arg0, arg1, arg2);

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

        while((mainEnt = Hooks::FindEntityByClassnameHook(CGlobalEntityList, mainEnt, (uint32_t)"*")) != 0)
        {
            char* classname = (char*) ( *(uint32_t*)(mainEnt+0x68) );
            uint32_t refHandle = *(uint32_t*)(mainEnt+0x350);

            pDynamicOneArgFunc = (pOneArgProt)(server_srv + 0x00AF29F0);
            uint8_t allowEntRestore = pDynamicOneArgFunc((uint32_t)classname);

            if(allowEntRestore)
            {
                Hooks::HookEntityDelete(mainEnt);
            }
        }

        //FlushEventQueue
        //pDynamicOneArgFunc = (pOneArgProt)(server_srv + 0x006863F0);
        //pDynamicOneArgFunc(server_srv + 0x00FF3020);

        Hooks::CleanupDeleteListHook(0);

        removing_ents_restore = false;
        
        AutosaveLoadOrig(*(uint32_t*)(server_srv + 0x00FA0CF0), (uint32_t)current_map, 0);
        *(uint8_t*)(server_srv + 0x01012130) = 1;

        //EndRestoreEntities
        pDynamicOneArgFunc = (pOneArgProt)(server_srv + 0x0073CBD0);
        pDynamicOneArgFunc(0);

        //BeginRestoreEntities
        pDynamicOneArgFunc = (pOneArgProt)(server_srv + 0x0073B880);
        pDynamicOneArgFunc(0);
    }

    return returnVal;
}

uint32_t Hooks::EdtSystemHookFunc(uint32_t arg1)
{
    uint32_t ref = (uint32_t)malloc(arg1*3.0);
    rootconsole->ConsolePrint("[EDT Hook] " HOOK_MSG, ref);

    Value* leak = CreateNewValue((void*)ref);
    InsertToValuesList(leakedResourcesEdtSystem, leak, NULL, true, true);

    return ref;
}

uint32_t Hooks::PreEdtLoad(uint32_t arg1, uint32_t arg2)
{
    //DONT USE!

    /*uint32_t returnVal = pEdtLoadFunc(arg1, arg2);
    ReleaseLeakedMemory(leakedResourcesEdtSystem, false, 0, 0, 100);
    return returnVal;*/
    return 0;
}

uint32_t Hooks::SaveRestoreMemManage(uint32_t arg0, uint32_t arg1)
{
    pTwoArgProt pDynamicTwoArgFunc;
    
    pDynamicTwoArgFunc = (pTwoArgProt)(server_srv + 0x00AF24F0);
    uint32_t returnVal = pDynamicTwoArgFunc(arg0, arg1);

    ReleaseLeakedMemory(leakedResourcesSaveRestoreSystem, false, 0, 0, 100);

    return returnVal;
}

uint32_t Hooks::RestoreOverride()
{
    pOneArgProt pDynamicOneArgFunc;

    if(savegame || restoring || mapHasEnded || server_sleeping) return 0;

    if(!hasSavedOnce)
    {
        rootconsole->ConsolePrint("Failed to restore - no valid autosave file found!");
        return 0;
    }

    restoring = true;

    isTicking = false;
    protect_player = true;

    *(uint8_t*)((*(uint32_t*)(server_srv + 0x00FA0CF0)) + 0x130) = 1;
    uint32_t main_engine_global = *(uint32_t*)(server_srv + 0x00109A3E0);

    rootconsole->ConsolePrint("Clearing entities!");

    removing_ents_restore = true;

    uint32_t mainEnt = 0;
    while((mainEnt = Hooks::FindEntityByClassnameHook(CGlobalEntityList, mainEnt, (uint32_t)"*")) != 0)
    {
        char* classname = (char*) ( *(uint32_t*)(mainEnt+0x68) );
        uint32_t refHandle = *(uint32_t*)(mainEnt+0x350);

        pDynamicOneArgFunc = (pOneArgProt)(server_srv + 0x00AF29F0);
        uint8_t allowEntRestore = pDynamicOneArgFunc((uint32_t)classname);

        if(allowEntRestore)
        {
            Hooks::HookEntityDelete(mainEnt);
        }
    }

    //FlushEventQueue
    //pDynamicOneArgFunc = (pOneArgProt)(server_srv + 0x006863F0);
    //pDynamicOneArgFunc(server_srv + 0x00FF3020);

    Hooks::CleanupDeleteListHook(0);

    removing_ents_restore = false;


    //EDICT REUSE
    pDynamicOneArgFunc = (pOneArgProt)(  *(uint32_t*) ((*(uint32_t*)(*(uint32_t*)(server_srv + 0x01012420)))+0x16C)  );
    pDynamicOneArgFunc(*(uint32_t*)(server_srv + 0x01012420));

    pDynamicOneArgFunc = (pOneArgProt)(server_srv + 0x0073B880);
    pDynamicOneArgFunc(0);

    AutosaveLoadOrig(*(uint32_t*)(server_srv + 0x00FA0CF0), (uint32_t)"autosave", 0);
    RestorePlayers();

    *(uint8_t*)(server_srv + 0x01012130) = 1;

    pDynamicOneArgFunc = (pOneArgProt)(server_srv + 0x0073CBD0);
    pDynamicOneArgFunc(0);
    
    *(uint8_t*)((*(uint32_t*)(server_srv + 0x00FA0CF0)) + 0x130) = 0;

    reset_viewcontrol = true;
    after_restore_frames = 0;

    InsertViewcontrolsToResetList();
    DisableViewControls();

    protect_player = false;
    restore_delay = true;

    Hooks::CleanupDeleteListHook(0);

    savegame = true;
    savegame_lock = true;
    save_frames = 3;

    restoring = false;

    SaveGame_Extension();

    savegame = false;
    savegame_lock = false;
    save_frames = 0;

    restoring = true;

    Hooks::CleanupDeleteListHook(0);

    return 0;
}

uint32_t Hooks::DirectMallocHookDedicatedSrv(uint32_t arg0)
{
    register uint32_t ebp asm("ebp");
    uint32_t arg0_return = *(uint32_t*)(ebp-4);
    uint32_t packed_store_ref = arg0_return-0x228;


    uint32_t ref = (uint32_t)malloc(arg0*5.0);
    Value* a_leak = *leakedResourcesVpkSystem;

    while(a_leak)
    {
        VpkMemoryLeak* the_leak = (VpkMemoryLeak*)(a_leak->value);
        uint32_t packed_object = the_leak->packed_ref;

        if(packed_object == packed_store_ref)
        {
            ValueList vpk_leak_list = the_leak->leaked_refs;
            Value* new_vpk_leak = CreateNewValue((void*)(ref));
            InsertToValuesList(vpk_leak_list, new_vpk_leak, NULL, false, false);

            rootconsole->ConsolePrint("[VPK Hook] " HOOK_MSG, ref);
        }

        a_leak = a_leak->nextVal;
    }

    return ref;
}

uint32_t Hooks::PackedStoreConstructorHook(uint32_t arg0, uint32_t arg1, uint32_t arg2, uint32_t arg3, uint32_t arg4)
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

uint32_t Hooks::PackedStoreDestructorHook(uint32_t arg0)
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
            
            int removed_items = DeleteAllValuesInList(vpk_leak_list, NULL, true);

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

uint32_t Hooks::SavegameInternalFunction(uint32_t arg0)
{
    pOneArgProt pDynamicOneArgFunc;
    
    FixModelnameSlashes();
    Hooks::CleanupDeleteListHook(0);

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

uint32_t Hooks::LevelChangedHookFrameSnaps(uint32_t arg0)
{
    pOneArgProt pDynamicOneArgFunc;

    ReleaseLeakedPackedEntities();

    pDynamicOneArgFunc = (pOneArgProt)(engine_srv + 0x001A5FB0);
    return pDynamicOneArgFunc(arg0);
}

uint32_t Hooks::FixNullCrash(uint32_t arg0)
{
    if(arg0 == 0)
    {
        rootconsole->ConsolePrint("NULL was passed!");
        return 0;
    }

    pOneArgProt pDynamicOneArgFunc = (pOneArgProt)(server_srv + 0x0075D540);
    return pDynamicOneArgFunc(arg0);
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

    uint32_t sim_ent_ref = *(uint32_t*)(arg0+0x350);
    uint32_t object_check = GetCBaseEntity(sim_ent_ref);

    if(object_check == 0)
    {
        rootconsole->ConsolePrint("Passed in non-existant simulation entity!");
        exit(EXIT_FAILURE);
        return 0;
    }

    char* clsname =  (char*) ( *(uint32_t*)(arg0+0x68) );

    if(mapHasEnded) return 0;

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

uint32_t Hooks::TransitionEntityCreateCall(uint32_t arg1, uint32_t arg2)
{
    rootconsole->ConsolePrint(EXT_PREFIX "Restoring %s", arg1);

    uint32_t object = Hooks::CreateEntityByNameHook(arg1, arg2);

    if(object)
    {
        uint32_t ref = *(uint32_t*)(object+0x350);
    }

    return object;
}

uint32_t Hooks::TransitionRestoreMain(uint32_t arg1, uint32_t arg2, uint32_t arg3, uint32_t arg4)
{
    pOneArgProt pDynamicOneArgFunc;
    transition = true;

    /*if(strcmp(last_map, "ep2_outland_04") == 0 && strcmp(global_map, "ep2_outland_02") == 0)
    {
        uint32_t searchEnt = 0;

        while((searchEnt = Hooks::FindEntityByClassnameHook(CGlobalEntityList, searchEnt, (uint32_t)"logic_relay")) != 0)
        {
            char* targetname = (char*)  *(uint32_t*)(searchEnt+0x124);
            if(!targetname)
                continue;

            if(strcmp(targetname, "debug_choreo_start_in_elevator") == 0)
            {
                uint32_t m_refHandle = *(uint32_t*)(searchEnt+0x350);
                rootconsole->ConsolePrint("Triggered relay - debug_choreo_start_in_elevator");
                SendEntityInput(m_refHandle, (uint32_t)"Trigger", 0, 0, 0, (uint32_t)-1);
                SendEntityInput(m_refHandle, (uint32_t)"Kill", 0, 0, 0, (uint32_t)-1);
                break;
            }
        }

        searchEnt = 0;

        while((searchEnt = Hooks::FindEntityByClassnameHook(CGlobalEntityList, searchEnt, (uint32_t)"trigger_once")) != 0)
        {
            char* targetname = (char*)  *(uint32_t*)(searchEnt+0x124);
            if(!targetname)
                continue;

            if(strcmp(targetname, "elevator_actor_setup_trigger") == 0)
            {
                uint32_t m_refHandle = *(uint32_t*)(searchEnt+0x350);
                rootconsole->ConsolePrint("Triggered trigger - elevator_actor_setup_trigger");
                SendEntityInput(m_refHandle, (uint32_t)"Trigger", 0, 0, 0, (uint32_t)-1);
                SendEntityInput(m_refHandle, (uint32_t)"Kill", 0, 0, 0, (uint32_t)-1);
                break;
            }
        }

        searchEnt = 0;

        while((searchEnt = Hooks::FindEntityByClassnameHook(CGlobalEntityList, searchEnt, (uint32_t)"point_template")) != 0)
        {
            char* targetname = (char*)  *(uint32_t*)(searchEnt+0x124);
            if(!targetname)
                continue;

            if(strcmp(targetname, "gman_template") == 0)
            {
                uint32_t m_refHandle = *(uint32_t*)(searchEnt+0x350);
                rootconsole->ConsolePrint("Spawned template - gman_template");
                SendEntityInput(m_refHandle, (uint32_t)"ForceSpawn", 0, 0, 0, (uint32_t)-1);
                SendEntityInput(m_refHandle, (uint32_t)"Kill", 0, 0, 0, (uint32_t)-1);
                break;
            }
        }

        searchEnt = 0;

        while((searchEnt = Hooks::FindEntityByClassnameHook(CGlobalEntityList, searchEnt, (uint32_t)"logic_relay")) != 0)
        {
            char* targetname = (char*)  *(uint32_t*)(searchEnt+0x124);
            if(!targetname)
                continue;

            if(strcmp(targetname, "start_the_elevator_rl") == 0)
            {
                uint32_t m_refHandle = *(uint32_t*)(searchEnt+0x350);
                rootconsole->ConsolePrint("Triggered relay - start_the_elevator_rl");
                SendEntityInput(m_refHandle, (uint32_t)"Trigger", 0, 0, 0, (uint32_t)-1);
                SendEntityInput(m_refHandle, (uint32_t)"Kill", 0, 0, 0, (uint32_t)-1);
                break;
            }
        }
    }*/

    if(strcmp((char*)arg2, (char*)last_map) == 0)
    {
        rootconsole->ConsolePrint("oldmap matched successfully!");
    }
    else
    {
        rootconsole->ConsolePrint("Failed to preserve oldmap: [%s] [%s]", arg2, last_map);
        arg2 = (uint32_t)last_map;
    }
    
    uint32_t returnVal = pTransitionRestoreMainCall(arg1, arg2, arg3, arg4);

    //EndRestoreEntities
    pDynamicOneArgFunc = (pOneArgProt)(server_srv + 0x0073CBD0);
    pDynamicOneArgFunc(0);

    return returnVal;
}

uint32_t Hooks::VehicleRollermineCheck(uint32_t arg1)
{
    //0 = failure
    //1 = success

    if(arg1 != 0)
    {
        if( ( (*(uint8_t*)(arg1+0x11D)) & 8) != 0 )
            pCallVehicleRollermineFunction(arg1);
        
        return 1;
    }

    rootconsole->ConsolePrint(EXT_PREFIX "Failed to jolt vehicle - crash prevented");
    return 0;
}

uint32_t Hooks::SaveOverride(uint32_t arg1)
{
    savegame = true;
    return 1;
}

uint32_t Hooks::SV_TriggerMovedFix(uint32_t arg1, uint32_t arg2)
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

uint32_t Hooks::memcpyNetworkHook(uint32_t dest, uint32_t src, uint32_t size)
{
    if(size <= 4096 && size >= 0)
    {
        return (uint32_t)memcpy((void*)dest, (void*)src, size);
    }

    rootconsole->ConsolePrint("NET_EXPLOIT %d", size);
    return 0;
}

uint32_t Hooks::DoorCycleResolve(uint32_t arg1, uint32_t arg2, uint32_t arg3, uint32_t arg4, uint32_t arg5)
{
    uint32_t something = *(uint32_t*)(server_srv + 0x00F4BA30);
    uint32_t mainValue = *(uint32_t*)(arg1+0x6E0);

    if(mainValue != 0xFFFFFFFF)
    {
        if(IsInValuesList(antiCycleListDoors, (void*)mainValue, NULL))
        {
            rootconsole->ConsolePrint("Cycle was detected and prevented hang!");
            DeleteAllValuesInList(antiCycleListDoors, NULL, false);
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

    DeleteAllValuesInList(antiCycleListDoors, NULL, false);
    return pDoorFinalFunction(arg1, arg2, arg3, arg4, arg5);
}

uint32_t Hooks::CreateEntityByNameHook(uint32_t arg0, uint32_t arg1)
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

uint32_t Hooks::LevelChangeSafeHook(uint32_t arg0)
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

uint32_t Hooks::PlayerSpawnHook(uint32_t arg0, uint32_t arg1, uint32_t arg2)
{
    rootconsole->ConsolePrint("called the main spawn info sender!");
    pThreeArgProt pDynamicThreeArgFunc;
    
    pDynamicThreeArgFunc = (pThreeArgProt)(server_srv + 0x00B01A90);
    return pDynamicThreeArgFunc(arg0, 1, 1);
}

uint32_t Hooks::PlayerSpawnDirectHook(uint32_t arg0)
{
    rootconsole->ConsolePrint("[Main] Called the main player spawn func!");

    pOneArgProt pDynamicOneArgFunc;
    pThreeArgProt pDynamicThreeArgFunc;

    Hooks::PlayerSpawnHook(arg0, 1, 1);

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

uint32_t Hooks::MainSaveEntitiesFunc(uint32_t arg0, uint32_t arg1)
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

uint32_t Hooks::FindEntityByHandle(uint32_t arg0, uint32_t arg1)
{
    pOneArgProt pDynamicOneArgFunc;
    pTwoArgProt pDynamicTwoArgFunc = (pTwoArgProt)(server_srv + 0x006B26B0);
    uint32_t object = pDynamicTwoArgFunc(arg0, arg1);

    return object;
}

uint32_t Hooks::FindEntityByClassnameHook(uint32_t arg0, uint32_t arg1, uint32_t arg2)
{
    pOneArgProt pDynamicOneArgFunc;
    pThreeArgProt pDynamicThreeArgFunc = (pThreeArgProt)(server_srv + 0x006B2740);
    uint32_t object = pDynamicThreeArgFunc(arg0, arg1, arg2);

    return object;
}

uint32_t Hooks::FindEntityByName(uint32_t arg0, uint32_t arg1, uint32_t arg2, uint32_t arg3, uint32_t arg4, uint32_t arg5, uint32_t arg6)
{
    pOneArgProt pDynamicOneArgFunc;
    pSevenArgProt pDynamicSevenArgFunc = (pSevenArgProt)(server_srv + 0x006B2CA0);
    uint32_t object = pDynamicSevenArgFunc(arg0, arg1, arg2, arg3, arg4, arg5, arg6);

    return object;
}

uint32_t Hooks::PlayerloadSavedHook(uint32_t arg0, uint32_t arg1)
{
    pZeroArgProt pDynamicZeroArgFunc;
    pOneArgProt pDynamicOneArgFunc;

    if(savegame || restoring || mapHasEnded || server_sleeping) return 0;

    if(!hasSavedOnce)
    {
        rootconsole->ConsolePrint("Failed to restore - no valid autosave file found!");
        return 0;
    }

    DisableViewControls();

    restore_start_delay = 0;
    return 0;
    
    //pDynamicOneArgFunc = (pOneArgProt)(server_srv + 0x00A4B8C0);
    //return pDynamicOneArgFunc(arg0);
}

uint32_t Hooks::SpawnServerHookFunc(uint32_t arg1, uint32_t arg2, uint32_t arg3)
{
    snprintf(last_map, 1024, "%s", global_map);
    snprintf(global_map, 1024, "%s", (char*)arg2);
    return pSpawnServerFunc(arg1, arg2, arg3);
}

uint32_t Hooks::HostChangelevelHook(uint32_t arg1, uint32_t arg2, uint32_t arg3)
{
    pOneArgProt pDynamicOneArgFunc;

    restoring = false;
    isTicking = false;
    hasSavedOnce = false;

    LogVpkMemoryLeaks();

    DeleteAllValuesInList(cmdbufflist, &cmdbufflistlock, true);

    uint32_t player = 0;

    while((player = Hooks::FindEntityByClassnameHook(CGlobalEntityList, player, (uint32_t)"player")) != 0)
    {
        //Ragdoll
        //pDynamicOneArgFunc = (pOneArgProt)(server_srv + 0x0098D1A0);
        //pDynamicOneArgFunc(player);

        uint32_t ragdoll_ref = *(uint32_t*)(player+0x1594);
        uint32_t ragdoll = GetCBaseEntity(ragdoll_ref);
        Hooks::HookInstaKill(ragdoll);

        //*(uint32_t*)(player+0x1594) = 0xFFFFFFFF;
    }

    restore_start_delay = 201;

    firstplayer_hasjoined = false;
    transition = false;
    mapHasEnded = false;
    mapHasEndedDelay = false;

    uint32_t gpGlobals_i_think = *(uint32_t*)(server_srv + 0x00FA0CF0);
    *(uint8_t*)(gpGlobals_i_think) = 0;

    uint32_t returnVal = pHostChangelevelFunc(arg1, arg2, arg3);

    /*if(strcmp(global_map, "d3_c17_10a") == 0)
    {
        uint32_t searchEnt = Hooks::FindEntityByClassnameHook(CGlobalEntityList, 0, (uint32_t)"npc_barney");

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

        while((searchEnt = Hooks::FindEntityByClassnameHook(CGlobalEntityList, searchEnt, (uint32_t)"prop_door_rotating")) != 0)
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

uint32_t Hooks::LookupPoseParameterDropshipHook(uint32_t dropship_object, uint32_t arg1, uint32_t arg2)
{
    uint32_t m_hContainer = *(uint32_t*)(dropship_object+0x1024);
    uint32_t m_hContainer_object = GetCBaseEntity(m_hContainer);

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

uint32_t Hooks::DropshipSpawnHook(uint32_t arg0)
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

uint32_t Hooks::fix_wheels_hook(uint32_t arg0, uint32_t arg1, uint32_t arg2)
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

uint32_t Hooks::PlayerDeathHook(uint32_t arg0)
{
    if(mapHasEnded) return 0;
    
    uint32_t refHandle = *(uint32_t*)(arg0+0x350);
    Value* newPlayer = CreateNewValue((void*)refHandle);
    InsertToValuesList(playerDeathQueue, newPlayer, &playerDeathQueueLock, false, false);
    return 0;
}

uint32_t Hooks::HookInstaKill(uint32_t arg0)
{
    InstaKill(arg0, true);
    return 0;
}

uint32_t Hooks::SV_FrameHook(uint32_t arg0)
{
    pOneArgProt pDynamicOneArgFunc;

    save_frames++;
    restore_frames++;
    delay_frames++;
    enqueue_delay_frames++;
    mapHasEndedDelayFrames++;
    game_start_frames++;
    car_delay_for_save++;
    restore_start_delay++;

    if(save_frames >= 500) save_frames = 0;
    if(restore_frames >= 500) restore_frames = 0;
    if(delay_frames >= 5000) delay_frames = 60;
    if(enqueue_delay_frames >= 500) enqueue_delay_frames = 5;
    if(mapHasEndedDelayFrames >= 500) mapHasEndedDelayFrames = 0;
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

uint32_t Hooks::ServiceEventQueueHook()
{
    pZeroArgProt pDynamicZeroArgFunc;
    Hooks::CleanupDeleteListHook(0);
    return 0;
}

uint32_t Hooks::SimulateEntitiesHook(uint8_t simulating)
{
    isTicking = true;
    pZeroArgProt pDynamicZeroArgFunc;
    pOneArgProt pDynamicOneArgFunc;

    Hooks::CleanupDeleteListHook(0);

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

    uint32_t firstplayer = Hooks::FindEntityByClassnameHook(CGlobalEntityList, 0, (uint32_t)"player");

    if(!firstplayer)
    {
        server_sleeping = true;
    }
    else
    {
        server_sleeping = false;
    }

    Hooks::CleanupDeleteListHook(0);

    SaveGame_Extension();

    Hooks::CleanupDeleteListHook(0);

    FlushPlayerDeaths();
    ResetView();
    UpdatePlayersDonor();
    AttemptToRestoreGame();

    Hooks::CleanupDeleteListHook(0);

    //SimulateEntities
    pDynamicOneArgFunc = (pOneArgProt)(server_srv + 0x00A316A0);
    pDynamicOneArgFunc(simulating);

    Hooks::CleanupDeleteListHook(0);

    //ServiceEventQueue
    pDynamicZeroArgFunc = (pZeroArgProt)(server_srv + 0x00687440);
    pDynamicZeroArgFunc();

    Hooks::CleanupDeleteListHook(0);

    //PostSystems
    pDynamicOneArgFunc = (pOneArgProt)(server_srv + 0x00471320);
    pDynamicOneArgFunc(0);

    Hooks::CleanupDeleteListHook(0);

    uint8_t deferMindist = *(uint8_t*)(vphysics_srv + 0x001AC980);

    if(deferMindist)
    {
        rootconsole->ConsolePrint("Warning defer mindist was set! Physics might break!");
    }

    *(uint8_t*)(vphysics_srv + 0x001AC980) = 0;
    
    ProcessChangelevelDelay();
    //TriggerMovedFailsafe();
    return 0;
}

uint32_t Hooks::GetClientSteamIDHook(uint32_t arg0, uint32_t arg1)
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

uint32_t Hooks::IsAllowChangelevel()
{
    pOneArgProt pDynamicOneArgFunc;

    register uint32_t ebp asm("ebp");
    uint32_t stack = ebp-(-0x570); //account for function prologue
    uint32_t arg0 = *(uint32_t*)(stack+(-0x550));
    uint32_t total_args = *(uint32_t*)(arg0);

    if(mapHasEnded && delay_frames >= 60)
    {
        pDynamicOneArgFunc = (pOneArgProt)(  *(uint32_t*) ( ( *(uint32_t*) ( *(uint32_t*)(server_srv + 0x01012420) )  )+8 )    );
        return pDynamicOneArgFunc(*(uint32_t*)(server_srv + 0x01012420));
    }
    else if(!mapHasEnded) delay_frames = 0;

    rootconsole->ConsolePrint("args_total [%d]", total_args);

    if(total_args == 2)
    {
        uint32_t firstArg = *(uint32_t*)(arg0+0x40C);
        rootconsole->ConsolePrint("args_2 : [%s]", firstArg);
        snprintf(next_map, 1024, "%s", (char*)firstArg);

        void* next_map_cpy = copy_val((void*)next_map, strlen(next_map)+1);
        Value* cmd = CreateNewValue(next_map_cpy);
        InsertToValuesList(cmdbufflist, cmd, &cmdbufflistlock, true, false);

        mapHasEnded = true;
        return 2;
    }
    else if(total_args == 3)
    {
        uint32_t firstArg = *(uint32_t*)(arg0+0x40C);
        uint32_t secondArg = *(uint32_t*)(arg0+0x410);
        rootconsole->ConsolePrint("args_3 : [%s] [%s]", firstArg, secondArg);
        snprintf(next_map, 1024, "%s %s", (char*)firstArg, (char*)secondArg);

        void* next_map_cpy = copy_val((void*)next_map, strlen(next_map)+1);
        Value* cmd = CreateNewValue(next_map_cpy);
        InsertToValuesList(cmdbufflist, cmd, &cmdbufflistlock, true, false);

        mapHasEnded = true;
        return 2;
    }

    pDynamicOneArgFunc = (pOneArgProt)(  *(uint32_t*) ( ( *(uint32_t*) ( *(uint32_t*)(server_srv + 0x01012420) )  )+8 )    );
    return pDynamicOneArgFunc(*(uint32_t*)(server_srv + 0x01012420));
}

uint32_t Hooks::SetSolidFlagsHook(uint32_t arg0, uint32_t arg1)
{
    pOneArgProt pDynamicOneArgFunc;
    pTwoArgProt pDynamicTwoArgFunc;

    uint32_t object = arg0-0x164;
    uint32_t clsname = *(uint32_t*)(object+0x68);

    pDynamicTwoArgFunc = (pTwoArgProt)(server_srv + 0x003F98A0);
    return pDynamicTwoArgFunc(arg0, arg1);
}

uint32_t Hooks::VphysicsUpdateWarningHook(uint32_t arg0)
{
    uint32_t refHandle = *(uint32_t*)(arg0+0x350);
    uint32_t object = GetCBaseEntity(refHandle);

    if(object)
    {
        char* classname = (char*)(*(uint32_t*)(arg0+0x68));

        if(classname)
        {
            rootconsole->ConsolePrint("Removing unreasonable entity! [%s]", classname);
        }
        else
        {
            rootconsole->ConsolePrint("Removing unreasonable entity!");
        }

        Hooks::HookEntityDelete(arg0);
        return 0;
    }

    rootconsole->ConsolePrint("Invalid Entity in vphysics!");
    exit(EXIT_FAILURE);
    return 0;
}

uint32_t Hooks::AcceptInputHook(uint32_t arg0, uint32_t arg1, uint32_t arg2, uint32_t arg3, uint32_t arg4, uint32_t arg5)
{
    pOneArgProt pDynamicOneArgFunc;
    pTwoArgProt pDynamicTwoArgFunc;
    pSixArgProt pDynamicSixArgProt;

    // CBaseEntity arg0 arg2 arg3

    bool failure = false;

    if(IsEntityValid(arg0) == 0) failure = true;

    if(failure)
    {
        rootconsole->ConsolePrint("AcceptInput blocked on marked entity");
        return 0;
    }

    //Passed sanity check
    pDynamicSixArgProt = (pSixArgProt)(server_srv + 0x00654F80);
    return pDynamicSixArgProt(arg0, arg1, arg2, arg3, arg4, arg5);
}

uint32_t Hooks::SetGlobalState(uint32_t arg0, uint32_t arg1)
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

uint32_t Hooks::RepairPlayerRestore(uint32_t arg0, uint32_t arg1, uint32_t arg2)
{
    // arg0 is scrubbed

    uint32_t classname = *(uint32_t*)(arg1+0x68);

    rootconsole->ConsolePrint("Restore failed for [%s] [%s]", classname, arg2);
    RemoveEntityNormal(arg1, true);

    player_restore_failed = true;
    return 0;
}

void HookFunctions()
{
    HookFunctionInSharedObject(server_srv, server_srv_size, (void*)(server_srv + 0x009B09F0), (void*)Hooks::ParseMapEntities);
    HookFunctionInSharedObject(server_srv, server_srv_size, (void*)(server_srv + 0x00AF24F0), (void*)Hooks::SaveRestoreMemManage);
    HookFunctionInSharedObject(server_srv, server_srv_size, (void*)(server_srv + 0x009AFCA0), (void*)Hooks::CreateEntityByNameHook);
    HookFunctionInSharedObject(server_srv, server_srv_size, (void*)(server_srv + 0x00AEF9E0), (void*)Hooks::PreEdtLoad);
    HookFunctionInSharedObject(engine_srv, engine_srv_size, (void*)(engine_srv + 0x001A5FB0), (void*)Hooks::LevelChangedHookFrameSnaps);
    HookFunctionInSharedObject(engine_srv, engine_srv_size, (void*)(engine_srv + 0x001CC730), (void*)Hooks::GetClientSteamIDHook);
    HookFunctionInSharedObject(vphysics_srv, vphysics_srv_size, (void*)(vphysics_srv + 0x000D6820), (void*)Hooks::fix_wheels_hook);
    HookFunctionInSharedObject(server_srv, server_srv_size, (void*)(server_srv + 0x00AF4530), (void*)Hooks::AutosaveLoadHook);
    //HookFunctionInSharedObject(server_srv, server_srv_size, (void*)(server_srv + 0x00652B10), (void*)Hooks::EmptyCall);
    HookFunctionInSharedObject(server_srv, server_srv_size, (void*)(server_srv + 0x0074A4A0), (void*)Hooks::SetGlobalState);
    HookFunctionInSharedObject(server_srv, server_srv_size, (void*)(server_srv + 0x00AF4110), (void*)Hooks::MainPlayerRestoreHook);

    /*rootconsole->ConsolePrint("patching calloc()");
    HookFunctionInSharedObject(server_srv, server_srv_size, (void*)calloc, (void*)Hooks::CallocHook);
    HookFunctionInSharedObject(engine_srv, engine_srv_size, (void*)calloc, (void*)Hooks::CallocHook);
    HookFunctionInSharedObject(datacache_srv, datacache_srv_size, (void*)calloc, (void*)Hooks::CallocHook);
    HookFunctionInSharedObject(dedicated_srv, dedicated_srv_size, (void*)calloc, (void*)Hooks::CallocHook);
    HookFunctionInSharedObject(materialsystem_srv, materialsystem_srv_size, (void*)calloc, (void*)Hooks::CallocHook);
    HookFunctionInSharedObject(vphysics_srv, vphysics_srv_size, (void*)calloc, (void*)Hooks::CallocHook);
    HookFunctionInSharedObject(scenefilecache, scenefilecache_size, (void*)calloc, (void*)Hooks::CallocHook);
    HookFunctionInSharedObject(soundemittersystem, soundemittersystem_size, (void*)calloc, (void*)Hooks::CallocHook);
    HookFunctionInSharedObject(soundemittersystem_srv, soundemittersystem_srv_size, (void*)calloc, (void*)Hooks::CallocHook);
    HookFunctionInSharedObject(studiorender_srv, studiorender_srv_size, (void*)calloc, (void*)Hooks::CallocHook);*/

    rootconsole->ConsolePrint("patching malloc()");
    HookFunctionInSharedObject(server_srv, server_srv_size, (void*)malloc, (void*)Hooks::MallocHook);
    /*HookFunctionInSharedObject(engine_srv, engine_srv_size, (void*)malloc, (void*)Hooks::MallocHook);
    HookFunctionInSharedObject(datacache_srv, datacache_srv_size, (void*)malloc, (void*)Hooks::MallocHook);
    HookFunctionInSharedObject(dedicated_srv, dedicated_srv_size, (void*)malloc, (void*)Hooks::MallocHook);
    HookFunctionInSharedObject(materialsystem_srv, materialsystem_srv_size, (void*)malloc, (void*)Hooks::MallocHook);
    HookFunctionInSharedObject(vphysics_srv, vphysics_srv_size, (void*)malloc, (void*)Hooks::MallocHook);
    HookFunctionInSharedObject(scenefilecache, scenefilecache_size, (void*)malloc, (void*)Hooks::MallocHook);
    HookFunctionInSharedObject(soundemittersystem, soundemittersystem_size, (void*)malloc, (void*)Hooks::MallocHook);
    HookFunctionInSharedObject(soundemittersystem_srv, soundemittersystem_srv_size, (void*)malloc, (void*)Hooks::MallocHook);
    HookFunctionInSharedObject(studiorender_srv, studiorender_srv_size, (void*)malloc, (void*)Hooks::MallocHook);*/

    /*rootconsole->ConsolePrint("patching realloc()");
    HookFunctionInSharedObject(server_srv, server_srv_size, (void*)realloc, (void*)Hooks::ReallocHook);
    HookFunctionInSharedObject(engine_srv, engine_srv_size, (void*)realloc, (void*)Hooks::ReallocHook);
    HookFunctionInSharedObject(datacache_srv, datacache_srv_size, (void*)realloc, (void*)Hooks::ReallocHook);
    HookFunctionInSharedObject(dedicated_srv, dedicated_srv_size, (void*)realloc, (void*)Hooks::ReallocHook);
    HookFunctionInSharedObject(materialsystem_srv, materialsystem_srv_size, (void*)realloc, (void*)Hooks::ReallocHook);
    HookFunctionInSharedObject(vphysics_srv, vphysics_srv_size, (void*)realloc, (void*)Hooks::ReallocHook);
    HookFunctionInSharedObject(scenefilecache, scenefilecache_size, (void*)realloc, (void*)Hooks::ReallocHook);
    HookFunctionInSharedObject(soundemittersystem, soundemittersystem_size, (void*)realloc, (void*)Hooks::ReallocHook);
    HookFunctionInSharedObject(soundemittersystem_srv, soundemittersystem_srv_size, (void*)realloc, (void*)Hooks::ReallocHook);
    HookFunctionInSharedObject(studiorender_srv, studiorender_srv_size, (void*)realloc, (void*)Hooks::ReallocHook);*/
    
    /*rootconsole->ConsolePrint("patching operator new");
    HookFunctionInSharedObject(server_srv, server_srv_size, new_operator_addr, (void*)Hooks::OperatorNewHook);
    HookFunctionInSharedObject(engine_srv, engine_srv_size, new_operator_addr, (void*)Hooks::OperatorNewHook);
    HookFunctionInSharedObject(datacache_srv, datacache_srv_size, new_operator_addr, (void*)Hooks::OperatorNewHook);
    HookFunctionInSharedObject(dedicated_srv, dedicated_srv_size, new_operator_addr, (void*)Hooks::OperatorNewHook);
    HookFunctionInSharedObject(materialsystem_srv, materialsystem_srv_size, new_operator_addr, (void*)Hooks::OperatorNewHook);
    HookFunctionInSharedObject(vphysics_srv, vphysics_srv_size, new_operator_addr, (void*)Hooks::OperatorNewHook);
    HookFunctionInSharedObject(scenefilecache, scenefilecache_size, new_operator_addr, (void*)Hooks::OperatorNewHook);
    HookFunctionInSharedObject(soundemittersystem, soundemittersystem_size, new_operator_addr, (void*)Hooks::OperatorNewHook);
    HookFunctionInSharedObject(soundemittersystem_srv, soundemittersystem_srv_size, new_operator_addr, (void*)Hooks::OperatorNewHook);
    HookFunctionInSharedObject(studiorender_srv, studiorender_srv_size, new_operator_addr, (void*)Hooks::OperatorNewHook);*/

    rootconsole->ConsolePrint("patching operator new[]");
    HookFunctionInSharedObject(server_srv, server_srv_size, new_operator_array_addr, (void*)Hooks::OperatorNewArrayHook);
    HookFunctionInSharedObject(engine_srv, engine_srv_size, new_operator_array_addr, (void*)Hooks::OperatorNewArrayHook);
    HookFunctionInSharedObject(datacache_srv, datacache_srv_size, new_operator_array_addr, (void*)Hooks::OperatorNewArrayHook);
    HookFunctionInSharedObject(dedicated_srv, dedicated_srv_size, new_operator_array_addr, (void*)Hooks::OperatorNewArrayHook);
    HookFunctionInSharedObject(materialsystem_srv, materialsystem_srv_size, new_operator_array_addr, (void*)Hooks::OperatorNewArrayHook);
    HookFunctionInSharedObject(vphysics_srv, vphysics_srv_size, new_operator_array_addr, (void*)Hooks::OperatorNewArrayHook);
    HookFunctionInSharedObject(scenefilecache, scenefilecache_size, new_operator_array_addr, (void*)Hooks::OperatorNewArrayHook);
    HookFunctionInSharedObject(soundemittersystem, soundemittersystem_size, new_operator_array_addr, (void*)Hooks::OperatorNewArrayHook);
    HookFunctionInSharedObject(soundemittersystem_srv, soundemittersystem_srv_size, new_operator_array_addr, (void*)Hooks::OperatorNewArrayHook);
    HookFunctionInSharedObject(studiorender_srv, studiorender_srv_size, new_operator_array_addr, (void*)Hooks::OperatorNewArrayHook);

    /*rootconsole->ConsolePrint("patching free()");
    HookFunctionInSharedObject(server_srv, server_srv_size, (void*)free, (void*)FreeHook);
    HookFunctionInSharedObject(engine_srv, engine_srv_size, (void*)free, (void*)FreeHook);
    HookFunctionInSharedObject(datacache_srv, datacache_srv_size, (void*)free, (void*)FreeHook);
    HookFunctionInSharedObject(dedicated_srv, dedicated_srv_size, (void*)free, (void*)FreeHook);
    HookFunctionInSharedObject(materialsystem_srv, materialsystem_srv_size, (void*)free, (void*)FreeHook);
    HookFunctionInSharedObject(vphysics_srv, vphysics_srv_size, (void*)free, (void*)FreeHook);
    HookFunctionInSharedObject(scenefilecache, scenefilecache_size, (void*)free, (void*)FreeHook);
    HookFunctionInSharedObject(soundemittersystem, soundemittersystem_size, (void*)free, (void*)FreeHook);
    HookFunctionInSharedObject(soundemittersystem_srv, soundemittersystem_srv_size, (void*)free, (void*)FreeHook);
    HookFunctionInSharedObject(studiorender_srv, studiorender_srv_size, (void*)free, (void*)FreeHook);*/

    /*rootconsole->ConsolePrint("patching operator delete");
    HookFunctionInSharedObject(server_srv, server_srv_size, delete_operator_addr, (void*)DeleteOperatorHook);
    HookFunctionInSharedObject(engine_srv, engine_srv_size, delete_operator_addr, (void*)DeleteOperatorHook);
    HookFunctionInSharedObject(datacache_srv, datacache_srv_size, delete_operator_addr, (void*)DeleteOperatorHook);
    HookFunctionInSharedObject(dedicated_srv, dedicated_srv_size, delete_operator_addr, (void*)DeleteOperatorHook);
    HookFunctionInSharedObject(materialsystem_srv, materialsystem_srv_size, delete_operator_addr, (void*)DeleteOperatorHook);
    HookFunctionInSharedObject(vphysics_srv, vphysics_srv_size, delete_operator_addr, (void*)DeleteOperatorHook);
    HookFunctionInSharedObject(scenefilecache, scenefilecache_size, delete_operator_addr, (void*)DeleteOperatorHook);
    HookFunctionInSharedObject(soundemittersystem, soundemittersystem_size, delete_operator_addr, (void*)DeleteOperatorHook);
    HookFunctionInSharedObject(soundemittersystem_srv, soundemittersystem_srv_size, delete_operator_addr, (void*)DeleteOperatorHook);
    HookFunctionInSharedObject(studiorender_srv, studiorender_srv_size, delete_operator_addr, (void*)DeleteOperatorHook);

    rootconsole->ConsolePrint("patching operator delete[]");
    HookFunctionInSharedObject(server_srv, server_srv_size, delete_operator_array_addr, (void*)DeleteOperatorArrayHook);
    HookFunctionInSharedObject(engine_srv, engine_srv_size, delete_operator_array_addr, (void*)DeleteOperatorArrayHook);
    HookFunctionInSharedObject(datacache_srv, datacache_srv_size, delete_operator_array_addr, (void*)DeleteOperatorArrayHook);
    HookFunctionInSharedObject(dedicated_srv, dedicated_srv_size, delete_operator_array_addr, (void*)DeleteOperatorArrayHook);
    HookFunctionInSharedObject(materialsystem_srv, materialsystem_srv_size, delete_operator_array_addr, (void*)DeleteOperatorArrayHook);
    HookFunctionInSharedObject(vphysics_srv, vphysics_srv_size, delete_operator_array_addr, (void*)DeleteOperatorArrayHook);
    HookFunctionInSharedObject(scenefilecache, scenefilecache_size, delete_operator_array_addr, (void*)DeleteOperatorArrayHook);
    HookFunctionInSharedObject(soundemittersystem, soundemittersystem_size, delete_operator_array_addr, (void*)DeleteOperatorArrayHook);
    HookFunctionInSharedObject(soundemittersystem_srv, soundemittersystem_srv_size, delete_operator_array_addr, (void*)DeleteOperatorArrayHook);
    HookFunctionInSharedObject(studiorender_srv, studiorender_srv_size, delete_operator_array_addr, (void*)DeleteOperatorArrayHook);*/


    /*rootconsole->ConsolePrint("patching memcpy()");
    HookFunctionInSharedObject(server_srv, server_srv_size, pMemcpyPtr, pMemcpyHookPtr);
    HookFunctionInSharedObject(engine_srv, engine_srv_size, pMemcpyPtr, pMemcpyHookPtr);
    HookFunctionInSharedObject(datacache_srv, datacache_srv_size, pMemcpyPtr, pMemcpyHookPtr);
    HookFunctionInSharedObject(dedicated_srv, dedicated_srv_size, pMemcpyPtr, pMemcpyHookPtr);
    HookFunctionInSharedObject(materialsystem_srv, materialsystem_srv_size, pMemcpyPtr, pMemcpyHookPtr);
    HookFunctionInSharedObject(vphysics_srv, vphysics_srv_size, pMemcpyPtr, pMemcpyHookPtr);
    HookFunctionInSharedObject(scenefilecache, scenefilecache_size, pMemcpyPtr, pMemcpyHookPtr);
    HookFunctionInSharedObject(soundemittersystem, soundemittersystem_size, pMemcpyPtr, pMemcpyHookPtr);
    HookFunctionInSharedObject(soundemittersystem_srv, soundemittersystem_srv_size, pMemcpyPtr, pMemcpyHookPtr);
    HookFunctionInSharedObject(studiorender_srv, studiorender_srv_size, pMemcpyPtr, pMemcpyHookPtr);
    rootconsole->ConsolePrint("patching memset()");
    HookFunctionInSharedObject(server_srv, server_srv_size, pMemsetPtr, pMemsetHookPtr);
    HookFunctionInSharedObject(engine_srv, engine_srv_size, pMemsetPtr, pMemsetHookPtr);
    HookFunctionInSharedObject(datacache_srv, datacache_srv_size, pMemsetPtr, pMemsetHookPtr);
    HookFunctionInSharedObject(dedicated_srv, dedicated_srv_size, pMemsetPtr, pMemsetHookPtr);
    HookFunctionInSharedObject(materialsystem_srv, materialsystem_srv_size, pMemsetPtr, pMemsetHookPtr);
    HookFunctionInSharedObject(vphysics_srv, vphysics_srv_size, pMemsetPtr, pMemsetHookPtr);
    HookFunctionInSharedObject(scenefilecache, scenefilecache_size, pMemsetPtr, pMemsetHookPtr);
    HookFunctionInSharedObject(soundemittersystem, soundemittersystem_size, pMemsetPtr, pMemsetHookPtr);
    HookFunctionInSharedObject(soundemittersystem_srv, soundemittersystem_srv_size, pMemsetPtr, pMemsetHookPtr);
    HookFunctionInSharedObject(studiorender_srv, studiorender_srv_size, pMemsetPtr, pMemsetHookPtr);
    rootconsole->ConsolePrint("patching memmove()");
    HookFunctionInSharedObject(server_srv, server_srv_size, pMemmovePtr, pMemmoveHookPtr);
    HookFunctionInSharedObject(engine_srv, engine_srv_size, pMemmovePtr, pMemmoveHookPtr);
    HookFunctionInSharedObject(datacache_srv, datacache_srv_size, pMemmovePtr, pMemmoveHookPtr);
    HookFunctionInSharedObject(dedicated_srv, dedicated_srv_size, pMemmovePtr, pMemmoveHookPtr);
    HookFunctionInSharedObject(materialsystem_srv, materialsystem_srv_size, pMemmovePtr, pMemmoveHookPtr);
    HookFunctionInSharedObject(vphysics_srv, vphysics_srv_size, pMemmovePtr, pMemmoveHookPtr);
    HookFunctionInSharedObject(scenefilecache, scenefilecache_size, pMemmovePtr, pMemmoveHookPtr);
    HookFunctionInSharedObject(soundemittersystem, soundemittersystem_size, pMemmovePtr, pMemmoveHookPtr);
    HookFunctionInSharedObject(soundemittersystem_srv, soundemittersystem_srv_size, pMemmovePtr, pMemmoveHookPtr);
    HookFunctionInSharedObject(studiorender_srv, studiorender_srv_size, pMemmovePtr, pMemmoveHookPtr);
    rootconsole->ConsolePrint("patching strncpy()");
    HookFunctionInSharedObject(server_srv, server_srv_size, pStrncpyPtr, pStrncpyHookPtr);
    HookFunctionInSharedObject(engine_srv, engine_srv_size, pStrncpyPtr, pStrncpyHookPtr);
    HookFunctionInSharedObject(datacache_srv, datacache_srv_size, pStrncpyPtr, pStrncpyHookPtr);
    HookFunctionInSharedObject(dedicated_srv, dedicated_srv_size, pStrncpyPtr, pStrncpyHookPtr);
    HookFunctionInSharedObject(materialsystem_srv, materialsystem_srv_size, pStrncpyPtr, pStrncpyHookPtr);
    HookFunctionInSharedObject(vphysics_srv, vphysics_srv_size, pStrncpyPtr, pStrncpyHookPtr);
    HookFunctionInSharedObject(scenefilecache, scenefilecache_size, pStrncpyPtr, pStrncpyHookPtr);
    HookFunctionInSharedObject(soundemittersystem, soundemittersystem_size, pStrncpyPtr, pStrncpyHookPtr);
    HookFunctionInSharedObject(soundemittersystem_srv, soundemittersystem_srv_size, pStrncpyPtr, pStrncpyHookPtr);
    HookFunctionInSharedObject(studiorender_srv, studiorender_srv_size, pStrncpyPtr, pStrncpyHookPtr);*/
    /*rootconsole->ConsolePrint("patching strcpy_chk()");
    HookFunctionInSharedObject(server_srv, server_srv_size, strcpy_chk_addr, pStrncpyHookPtr);
    HookFunctionInSharedObject(engine_srv, engine_srv_size, strcpy_chk_addr, pStrncpyHookPtr);
    HookFunctionInSharedObject(datacache_srv, datacache_srv_size, strcpy_chk_addr, pStrncpyHookPtr);
    HookFunctionInSharedObject(dedicated_srv, dedicated_srv_size, strcpy_chk_addr, pStrncpyHookPtr);
    HookFunctionInSharedObject(materialsystem_srv, materialsystem_srv_size, strcpy_chk_addr, pStrncpyHookPtr);
    HookFunctionInSharedObject(vphysics_srv, vphysics_srv_size, strcpy_chk_addr, pStrncpyHookPtr);
    HookFunctionInSharedObject(scenefilecache, scenefilecache_size, strcpy_chk_addr, pStrncpyHookPtr);
    HookFunctionInSharedObject(soundemittersystem, soundemittersystem_size, strcpy_chk_addr, pStrncpyHookPtr);
    HookFunctionInSharedObject(soundemittersystem_srv, soundemittersystem_srv_size, strcpy_chk_addr, pStrncpyHookPtr);
    HookFunctionInSharedObject(studiorender_srv, studiorender_srv_size, strcpy_chk_addr, pStrncpyHookPtr);*/
    /*rootconsole->ConsolePrint("patching strcpy()");
    HookFunctionInSharedObject(server_srv, server_srv_size, pStrcpyPtr, pStrcpyHookPtr);
    HookFunctionInSharedObject(engine_srv, engine_srv_size, pStrcpyPtr, pStrcpyHookPtr);
    HookFunctionInSharedObject(datacache_srv, datacache_srv_size, pStrcpyPtr, pStrcpyHookPtr);
    HookFunctionInSharedObject(dedicated_srv, dedicated_srv_size, pStrcpyPtr, pStrcpyHookPtr);
    HookFunctionInSharedObject(materialsystem_srv, materialsystem_srv_size, pStrcpyPtr, pStrcpyHookPtr);
    HookFunctionInSharedObject(vphysics_srv, vphysics_srv_size, pStrcpyPtr, pStrcpyHookPtr);
    HookFunctionInSharedObject(scenefilecache, scenefilecache_size, pStrcpyPtr, pStrcpyHookPtr);
    HookFunctionInSharedObject(soundemittersystem, soundemittersystem_size, pStrcpyPtr, pStrcpyHookPtr);
    HookFunctionInSharedObject(soundemittersystem_srv, soundemittersystem_srv_size, pStrcpyPtr, pStrcpyHookPtr);
    HookFunctionInSharedObject(studiorender_srv, studiorender_srv_size, pStrcpyPtr, pStrcpyHookPtr);*/



    HookFunctionInSharedObject(server_srv, server_srv_size, (void*)(server_srv + 0x00A4B8C0), (void*)Hooks::PlayerloadSavedHook);
    HookFunctionInSharedObject(server_srv, server_srv_size, (void*)(server_srv + 0x00B043C0), (void*)Hooks::PlayerSpawnDirectHook);
    HookFunctionInSharedObject(server_srv, server_srv_size, (void*)(server_srv + 0x006B26B0), (void*)Hooks::FindEntityByHandle);
    HookFunctionInSharedObject(server_srv, server_srv_size, (void*)(server_srv + 0x006B2740), (void*)Hooks::FindEntityByClassnameHook);
    HookFunctionInSharedObject(server_srv, server_srv_size, (void*)(server_srv + 0x006B2CA0), (void*)Hooks::FindEntityByName);
    HookFunctionInSharedObject(server_srv, server_srv_size, (void*)(server_srv + 0x006B2510), (void*)Hooks::CleanupDeleteListHook);
    HookFunctionInSharedObject(server_srv, server_srv_size, (void*)(server_srv + 0x00B64500), (void*)Hooks::HookEntityDelete);
    HookFunctionInSharedObject(server_srv, server_srv_size, (void*)(server_srv + 0x00B64480), (void*)Hooks::UTIL_RemoveHookFailsafe);
    HookFunctionInSharedObject(server_srv, server_srv_size, (void*)(server_srv + 0x00B64630), (void*)Hooks::HookInstaKill);
    HookFunctionInSharedObject(server_srv, server_srv_size, (void*)(server_srv + 0x00AF3990), (void*)Hooks::SaveOverride);
    HookFunctionInSharedObject(server_srv, server_srv_size, (void*)(server_srv + 0x00B01A90), (void*)Hooks::PlayerSpawnHook);
    HookFunctionInSharedObject(server_srv, server_srv_size, (void*)(server_srv + 0x00AF33F0), (void*)Hooks::SavegameInternalFunction);
    HookFunctionInSharedObject(server_srv, server_srv_size, (void*)(server_srv + 0x00A311D0), (void*)Hooks::PhysSimEnt);
    HookFunctionInSharedObject(server_srv, server_srv_size, (void*)(server_srv + 0x00AEFDB0), (void*)Hooks::EmptyCall);
    //HookFunctionInSharedObject(server_srv, server_srv_size, (void*)(server_srv + 0x00687440), (void*)Hooks::EmptyCall);
    //HookFunctionInSharedObject(server_srv, server_srv_size, (void*)(server_srv + 0x00471300), (void*)Hooks::EmptyCall);
    //HookFunctionInSharedObject(server_srv, server_srv_size, (void*)(server_srv + 0x00B03590), (void*)Hooks::EmptyCall);
    //HookFunctionInSharedObject(server_srv, server_srv_size, (void*)(server_srv + 0x00AF3120), (void*)Hooks::EmptyCall);
    //HookFunctionInSharedObject(datacache_srv, datacache_srv_size, (void*)(datacache_srv + 0x000381D0), (void*)Hooks::EmptyCall);
    HookFunctionInSharedObject(server_srv, server_srv_size, (void*)(server_srv + 0x004CCA80), (void*)Hooks::LevelChangeSafeHook);
    HookFunctionInSharedObject(engine_srv, engine_srv_size, (void*)(engine_srv + 0x001B1800), (void*)Hooks::SV_FrameHook);
    //HookFunctionInSharedObject(server_srv, server_srv_size, (void*)(server_srv + 0x0098D1A0), (void*)Hooks::PlayerDeathHook);



    //HookFunctionInSharedObject(server_srv, server_srv_size, (void*)(server_srv + 0x00499CD0), (void*)Hooks::EmptyCall);
    //HookFunctionInSharedObject(server_srv, server_srv_size, (void*)(server_srv + 0x00499D40), (void*)Hooks::EmptyCall);
    /*HookFunctionInSharedObject(server_srv, server_srv_size, (void*)(server_srv + 0x00499E00), (void*)EmptyCall);

    HookFunctionInSharedObject(server_srv, server_srv_size, (void*)(server_srv + 0x00A84FA0), (void*)EmptyCall);
    HookFunctionInSharedObject(server_srv, server_srv_size, (void*)(server_srv + 0x009ADED0), (void*)EmptyCall);
    HookFunctionInSharedObject(server_srv, server_srv_size, (void*)(server_srv + 0x0090B320), (void*)EmptyCall);

    HookFunctionInSharedObject(server_srv, server_srv_size, (void*)(server_srv + 0x00A84F70), (void*)EmptyCall);
    HookFunctionInSharedObject(server_srv, server_srv_size, (void*)(server_srv + 0x009ADE50), (void*)EmptyCall);
    HookFunctionInSharedObject(server_srv, server_srv_size, (void*)(server_srv + 0x0090B2A0), (void*)EmptyCall);*/


    HookFunctionInSharedObject(server_srv, server_srv_size, (void*)(server_srv + 0x003F98A0), (void*)Hooks::SetSolidFlagsHook);
    HookFunctionInSharedObject(server_srv, server_srv_size, (void*)(server_srv + 0x0086A6A0), (void*)Hooks::DropshipSpawnHook);
    HookFunctionInSharedObject(dedicated_srv, dedicated_srv_size, (void*)(dedicated_srv + 0x000BD1B0), (void*)Hooks::PackedStoreConstructorHook);
    HookFunctionInSharedObject(dedicated_srv, dedicated_srv_size, (void*)(dedicated_srv + 0x000BAE80), (void*)Hooks::PackedStoreDestructorHook);
    HookFunctionInSharedObject(server_srv, server_srv_size, (void*)(server_srv + 0x00654F80), (void*)Hooks::AcceptInputHook);
    HookFunctionInSharedObject(server_srv, server_srv_size, (void*)(server_srv + 0x0065BD80), (void*)Hooks::UpdateOnRemove);
    HookFunctionInSharedObject(server_srv, server_srv_size, (void*)(server_srv + 0x00B67F10), (void*)Hooks::UTIL_PrecacheOther_Hook);
}