#include "extension.h"

SynergyUtils g_SynUtils;		/**< Global singleton for extension's main interface */
SMEXT_LINK(&g_SynUtils);

struct link_map *engine_lm;
struct link_map *datacache_lm;
struct link_map *dedicated_lm;
struct link_map *materialsystem_srv_lm;
struct link_map *vphysics_srv_lm;
struct link_map *scenefilecache_lm;
struct link_map *soundemittersystem_lm;
struct link_map *soundemittersystem_srv_lm;
struct link_map *studiorender_srv_lm;
struct link_map *server_srv_lm;

uint32_t engine_srv;
uint32_t datacache_srv;
uint32_t dedicated_srv;
uint32_t materialsystem_srv;
uint32_t vphysics_srv;
uint32_t scenefilecache;
uint32_t soundemittersystem;
uint32_t soundemittersystem_srv;
uint32_t studiorender_srv;
uint32_t server_srv;

uint32_t engine_srv_size;
uint32_t datacache_srv_size;
uint32_t dedicated_srv_size;
uint32_t materialsystem_srv_size;
uint32_t vphysics_srv_size;
uint32_t scenefilecache_size;
uint32_t soundemittersystem_size;
uint32_t soundemittersystem_srv_size;
uint32_t studiorender_srv_size;
uint32_t server_srv_size;

uint32_t sv;
uint32_t g_ModelLoader;
uint32_t g_DataCache;
uint32_t g_MDLCache;
uint32_t s_ServerPlugin;
uint32_t SaveRestoreGlobal;
uint32_t CGlobalEntityList;

uint32_t EdtLoadFuncAddr;
uint32_t Flush;
uint32_t HostChangelevel;
uint32_t SpawnServer;
uint32_t PopulatePoseParametersAddr;
uint32_t EndFunctionAddr;
uint32_t OldFunctionAddr;
uint32_t sub_654260_addr;
uint32_t sub_628F00_addr;
uint32_t MainTransitionCallAddr;
uint32_t CreateEntityByNameAddr;
uint32_t SaveGameStateAddr;
uint32_t TransitionRestoreMainCallOrigAddr;
uint32_t KillEntityDirectCallAddr;
uint32_t VehicleRollermineFunctionAddr;
uint32_t OrigSaveCallAddr;
uint32_t OriginalTriggerMovedAddr;
uint32_t DoorFinalFunctionAddr;
uint32_t GetNumClientsAddr;
uint32_t GetNumProxiesAddr;
uint32_t FindEntityByClassnameAddr;
uint32_t origAutosaveCall;
uint32_t origRestoreCall;
uint32_t UnloadUnreferencedModelsFuncAddr;
uint32_t EnqueueCommandAddr;
uint32_t CreateEntityCallAddr;
uint32_t OrigManhackFuncAddr;
uint32_t DispatchSpawnAddr;
uint32_t ActivateEntityAddr;
uint32_t AutosaveLoadOrigAddr;
uint32_t InactivateClientsAddr;
uint32_t ReconnectClientsAddr;
uint32_t PlayerLoadOrigAddr;
uint32_t CleanupDeleteListAddr;

pOneArgProt pDynamicOneArgFunc;
pTwoArgProt pDynamicTwoArgFunc;
pThreeArgProt pDynamicThreeArgFunc;
pFourArgProt pDynamicFourArgFunc;
pFiveArgProt pDynamicFiveArgFunc;
pSixArgProt pDynamicSixArgFunc;

pOneArgProt pKillEntityDirectFunc;
pTwoArgProt pEdtLoadFunc;
pThreeArgProt pHostChangelevelFunc;
pThreeArgProt pFlushFunc;
pThreeArgProt pSpawnServerFunc;
pThreeArgProt PopulatePoseParameters;
pThreeArgProt EndFunction;
pOneArgProt OldFunction;
pOneArgProt sub_654260;
pOneArgProt sub_628F00;
pTwoArgProt MainTransitionCall;
pTwoArgProt CreateEntityByName;
pThreeArgProt SaveGameState;
pFourArgProt pTransitionRestoreMainCall;
pOneArgProt pCallVehicleRollermineFunction;
pOneArgProt pCallOrigSaveFunction;
pTwoArgProt pCallOriginalTriggerMoved;
pFiveArgProt pDoorFinalFunction;
pOneArgProt GetNumClients;
pOneArgProt GetNumProxies;
pThreeArgProt FindEntityByClassname;
pThreeArgProt pOrigAutosaveCallFunc;
pOneArgProt UnloadUnreferencedModels;
pOneArgProt EnqueueCommandFunc;
pTwoArgProt CreateEntityCallFunc;
pOneArgProt OrigManhackFunc;
pOneArgProt pDispatchSpawnFunc;
pOneArgProt pActivateEntityFunc;
pTwoArgProt pRestoreFileCallFunc;
pThreeArgProt AutosaveLoadOrig;
pOneArgProt InactivateClients;
pOneArgProt ReconnectClients;
pOneArgProt PlayerLoadOrig;
pOneArgProt CleanupDeleteList;


pthread_mutex_t malloc_ref_lock;
pthread_mutex_t value_list_lock;

uint32_t hook_exclude_list[512] = {};
uint32_t memory_prots_save_list[512] = {};

ValueList leakedResourcesSaveRestoreSystem;
ValueList leakedResourcesEdtSystem;
ValueList antiCycleListDoors;

MallocRefList mallocAllocations;
PlayerSaveList playerSaveList;

char* global_map;
char* last_map;
bool transition;
bool savegame;
bool savegame_lock;
bool restoring;
bool protect_player;
bool gamestart;
bool gamestart_lock;
bool restore_delay;
bool restore_delay_lock;
int frames;
uint32_t global_map_ents;

void* delete_operator_array_addr;
void* delete_operator_addr;

void* new_operator_addr;
void* new_operator_array_addr;

void* strcpy_chk_addr;

bool SynergyUtils::SDK_OnLoad(char *error, size_t maxlen, bool late)
{
    AllowWriteToMappedMemory();

    int pthread_init_one = pthread_mutex_init(&value_list_lock, NULL);
    int pthread_init_two = pthread_mutex_init(&malloc_ref_lock, NULL);

    if(pthread_init_one != 0 || pthread_init_two != 0)
    {
        rootconsole->ConsolePrint("\nMutex init for lists thread safeness has failed\n");
        exit(EXIT_FAILURE);
    }

    global_map = (char*) malloc(1024);
    last_map = (char*) malloc(1024);
    transition = false;
    savegame = false;
    savegame_lock = false;
    restoring = false;
    protect_player = false;
    frames = 0;
    gamestart = false;
    gamestart_lock = false;
    restore_delay = false;
    restore_delay_lock = false;
    global_map_ents = 0;

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

    snprintf(last_map, 1024, "d1_trainstation_06");
    snprintf(global_map, 1024, "d1_trainstation_06");

    engine_lm = (struct link_map*)(dlopen(engine_srv_fullpath, RTLD_NOW));
    datacache_lm = (struct link_map*)(dlopen(datacache_srv_fullpath, RTLD_NOW));
    dedicated_lm = (struct link_map*)(dlopen(dedicated_srv_fullpath, RTLD_NOW));
    materialsystem_srv_lm = (struct link_map*)(dlopen(materialsystem_srv_fullpath, RTLD_NOW));
    vphysics_srv_lm = (struct link_map*)(dlopen(vphysics_srv_fullpath, RTLD_NOW));
    scenefilecache_lm = (struct link_map*)(dlopen(scenefilecache_fullpath, RTLD_NOW));
    soundemittersystem_lm = (struct link_map*)(dlopen(soundemittersystem_fullpath, RTLD_NOW));
    soundemittersystem_srv_lm = (struct link_map*)(dlopen(soundemittersystem_srv_fullpath, RTLD_NOW));
    studiorender_srv_lm = (struct link_map*)(dlopen(studiorender_srv_fullpath, RTLD_NOW));
    server_srv_lm = (struct link_map*)(dlopen(server_srv_fullpath, RTLD_NOW));

    engine_srv_size = 0x2C0000;
    datacache_srv_size = 0x74000;
    dedicated_srv_size = 0x26D000;
    materialsystem_srv_size = 0x135000;
    vphysics_srv_size = 0x1A5000;
    scenefilecache_size = 0x14000;
    soundemittersystem_size = 0x33000;
    soundemittersystem_srv_size = 0x33000;
    studiorender_srv_size = 0xD3000;
    server_srv_size = 0xFB1000;

    engine_srv = engine_lm->l_addr;
    datacache_srv = datacache_lm->l_addr;
    dedicated_srv = dedicated_lm->l_addr;
    materialsystem_srv = materialsystem_srv_lm->l_addr;
    vphysics_srv = vphysics_srv_lm->l_addr;
    scenefilecache = scenefilecache_lm->l_addr;
    soundemittersystem = soundemittersystem_lm->l_addr;
    soundemittersystem_srv = soundemittersystem_srv_lm->l_addr;
    studiorender_srv = studiorender_srv_lm->l_addr;
    server_srv = server_srv_lm->l_addr;

    RestoreLinkedLists();
    SaveProcessId();
    
    antiCycleListDoors = AllocateValuesList();
    mallocAllocations = AllocateMallocRefList();
    playerSaveList = AllocatePlayerSaveList();

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
    PopulatePoseParametersAddr = server_srv + 0x00628220;
    EndFunctionAddr = server_srv + 0x00B66D60;
    OldFunctionAddr = server_srv + 0x0085F0E0;
    sub_654260_addr = server_srv + 0x00654260;
    sub_628F00_addr = server_srv + 0x00628F00;
    MainTransitionCallAddr = server_srv + 0x004AED50;
    CreateEntityByNameAddr = server_srv + 0x009AFCA0;
    SaveGameStateAddr = server_srv + 0x00AF3990;
    TransitionRestoreMainCallOrigAddr = server_srv + 0x00AF46C0;
    KillEntityDirectCallAddr = server_srv + 0x00B64500;
    VehicleRollermineFunctionAddr = server_srv + 0x00654970;
    OrigSaveCallAddr = server_srv + 0x00AF33F0;
    OriginalTriggerMovedAddr = engine_srv + 0x001D8FD0;
    DoorFinalFunctionAddr = server_srv + 0x00A94600;
    GetNumClientsAddr = engine_srv + 0x000D3030;
    GetNumProxiesAddr = engine_srv + 0x000D3080;
    FindEntityByClassnameAddr = server_srv + 0x006B2740;
    origAutosaveCall = server_srv + 0x00AF3990;
    origRestoreCall = server_srv + 0x00AF2A60;
    UnloadUnreferencedModelsFuncAddr = engine_srv + 0x0014D6E0;
    EnqueueCommandAddr = engine_srv + 0x000DE9C0;
    CreateEntityCallAddr = server_srv + 0x00B62220;
    OrigManhackFuncAddr = server_srv + 0x0047E5D0;
    DispatchSpawnAddr = server_srv + 0x00B68190;
    ActivateEntityAddr = server_srv + 0x65DB30;
    AutosaveLoadOrigAddr = server_srv + 0x00AF4530;
    InactivateClientsAddr = engine_srv + 0x000D5DA0;
    ReconnectClientsAddr = engine_srv + 0x000D5E50;
    PlayerLoadOrigAddr = server_srv + 0x00B02DB0;
    CleanupDeleteListAddr = server_srv + 0x006B2510;

    pEdtLoadFunc = (pTwoArgProt)EdtLoadFuncAddr;
    pHostChangelevelFunc = (pThreeArgProt)HostChangelevel;
    pFlushFunc = (pThreeArgProt)Flush;
    pSpawnServerFunc = (pThreeArgProt)SpawnServer;
    PopulatePoseParameters = (pThreeArgProt)PopulatePoseParametersAddr;
    EndFunction = (pThreeArgProt)EndFunctionAddr;
    OldFunction = (pOneArgProt)OldFunctionAddr;
    sub_654260 = (pOneArgProt)sub_654260_addr;
    sub_628F00 = (pOneArgProt)sub_628F00_addr;
    MainTransitionCall = (pTwoArgProt)MainTransitionCallAddr;
    CreateEntityByName = (pTwoArgProt)CreateEntityByNameAddr;
    SaveGameState = (pThreeArgProt)SaveGameStateAddr;
    pTransitionRestoreMainCall = (pFourArgProt)TransitionRestoreMainCallOrigAddr;
    pKillEntityDirectFunc = (pOneArgProt)KillEntityDirectCallAddr;
    pCallVehicleRollermineFunction = (pOneArgProt)VehicleRollermineFunctionAddr;
    pCallOrigSaveFunction = (pOneArgProt)OrigSaveCallAddr;
    pCallOriginalTriggerMoved = (pTwoArgProt)OriginalTriggerMovedAddr;
    pDoorFinalFunction = (pFiveArgProt)DoorFinalFunctionAddr;
    GetNumClients = (pOneArgProt)GetNumClientsAddr;
    GetNumProxies = (pOneArgProt)GetNumProxiesAddr;
    FindEntityByClassname = (pThreeArgProt)FindEntityByClassnameAddr;
    pOrigAutosaveCallFunc = (pThreeArgProt)origAutosaveCall;
    UnloadUnreferencedModels = (pOneArgProt)UnloadUnreferencedModelsFuncAddr;
    EnqueueCommandFunc = (pOneArgProt)EnqueueCommandAddr;
    CreateEntityCallFunc = (pTwoArgProt)CreateEntityCallAddr;
    OrigManhackFunc = (pOneArgProt)OrigManhackFuncAddr;
    pDispatchSpawnFunc = (pOneArgProt)DispatchSpawnAddr;
    pActivateEntityFunc = (pOneArgProt)ActivateEntityAddr;
    pRestoreFileCallFunc = (pTwoArgProt)origRestoreCall;
    AutosaveLoadOrig = (pThreeArgProt)AutosaveLoadOrigAddr;
    InactivateClients = (pOneArgProt)InactivateClientsAddr;
    ReconnectClients = (pOneArgProt)ReconnectClientsAddr;
    PlayerLoadOrig = (pOneArgProt)PlayerLoadOrigAddr;
    CleanupDeleteList = (pOneArgProt)CleanupDeleteListAddr;

    delete_operator_array_addr = (void*) ( *(uint32_t*)(server_srv + 0x0041C8CD + 1) + (server_srv + 0x0041C8CD) + 5 );
    delete_operator_addr = (void*) ( *(uint32_t*)(server_srv + 0x0041C8FD + 1) + (server_srv + 0x0041C8FD) + 5 );

    new_operator_array_addr = (void*) ( *(uint32_t*)(server_srv + 0x0041CAB9 + 1) + (server_srv + 0x0041CAB9) + 5 );
    new_operator_addr = (void*) ( *(uint32_t*)(server_srv + 0x0041CA39 + 1) + (server_srv + 0x0041CA39) + 5 );

    strcpy_chk_addr = (void*) ( *(uint32_t*)(server_srv + 0x0039D18E + 1) + (server_srv + 0x0039D18E) + 5 );

    HookFunctionsWithC();
    return true;
}

void SynergyUtils::SDK_OnAllLoaded()
{    
    PatchRestoring();
    HookSaveRestoreOne();
    HookSaveRestoreTwo();
    HookSaveRestoreThree();
    HookSavingOne();
    HookSavingTwo();
    PatchAutosave();
    PatchRestore();
    HookEdtSystem();
    //PatchEdtSystem();
    HookSpawnServer();
    HookHostChangelevel();
    PatchDropships();
    PatchOthers();

    HookFunctionsWithCpp();

    RestoreMemoryProtections();
    rootconsole->ConsolePrint("----------------------  " SMEXT_CONF_NAME " loaded!" "  ----------------------");
}

void PatchRestoring()
{
    uint32_t nop_patch_list[128] = 
    {
        0x00AF4F98,5,0x00AF4655,5,0x00AF467D,2,0x0068795A,0x12,0x004AE331,0x8,0x00AF4EA0,0x27,
        0x009924F3,0x3B,0x009927E1,0xF,0x00992640,5,0x008C1DC0,0x8,0x00B021A3,0x15,0x00AF43EE,5,0x00AF43FC,5,
        0x0096026E,5,0x00815EF0,5,0x0073CDFC,5,0x0073C6D3,2,0x0073C6FD,0xA
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

    /*uint32_t spawn_patch_one = server_srv + 0x0098FFF9;
    memset((void*)spawn_patch_one, 0x90, 0x15);

    *(uint8_t*)(spawn_patch_one) = 0x89;
    *(uint8_t*)(spawn_patch_one+1) = 0x1C;
    *(uint8_t*)(spawn_patch_one+2) = 0x24;

    spawn_patch_one = server_srv + (0x0098FFF9+3);
    offset = (uint32_t)pPlayerSpawnDirectHookPtr - spawn_patch_one - 5;
    *(uint8_t*)(spawn_patch_one) = 0xE8;
    *(uint32_t*)(spawn_patch_one+1) = offset;

    spawn_patch_one = server_srv + (0x0098FFF9+8);
    *(uint8_t*)(spawn_patch_one) = 0xE9;
    *(uint32_t*)(spawn_patch_one+1) = 0x17F;*/

    uint32_t exploit_patch_one = engine_srv + 0x0016A9B0;
    *(uint8_t*)(exploit_patch_one) = 0xE9;
    *(uint32_t*)(exploit_patch_one+1) = -0x258;

    *(uint16_t*)((server_srv + 0x0096026E)) = 0xC031;
    *(uint16_t*)((server_srv + 0x00815EF0)) = 0xC031;

    memset((void*)(engine_srv + 0x00136808), 0x90, 0xD);

    memset((void*)(engine_srv + 0x0012AA28), 0x90, 5);
    memset((void*)(engine_srv + 0x001AF717), 0x90, 5);

    /*uint32_t fix_null_ent_crash_cfire = server_srv + 0x007159A4;
    *(uint8_t*)(fix_null_ent_crash_cfire) = 0xE9;
    *(uint32_t*)(fix_null_ent_crash_cfire+1) = 0x2D;*/

    memset((void*)(dedicated_srv + 0x000BE6F6), 0x90, 5);
    *(uint16_t*)((dedicated_srv + 0x000BE6F6)) = 0xC031;

    uint32_t jmp_new_lvl = server_srv + 0x0073C760;
    *(uint8_t*)(jmp_new_lvl) = 0xE9;
    *(uint32_t*)(jmp_new_lvl+1) = 0x4B;

    uint32_t jmp_to_fix_heli = server_srv + 0x00960275;
    *(uint8_t*)(jmp_to_fix_heli) = 0xE9;
    *(uint32_t*)(jmp_to_fix_heli+1) = 0x44E;

    uint32_t jmp_fix_heli_two = server_srv + 0x00815EF7;
    *(uint8_t*)(jmp_fix_heli_two) = 0xEB;

    //get rid of message spam on sound pointer
    uint32_t it_said_it_causes_corruption = server_srv + 0x00A5436D;
    *(uint8_t*)(it_said_it_causes_corruption) = 0xC3;

    uint32_t begin_map_load_patch = datacache_srv + 0x0005A4CC;
    *(uint32_t*)(begin_map_load_patch) = (uint32_t)FrameLockHook;

    uint32_t packet_crash_exploit_patch = engine_srv + 0x001DBE8E;
    *(uint8_t*)(packet_crash_exploit_patch) = 0xEB;

    uint32_t null_manhack_patch = server_srv + 0x008C1DC8;
    *(uint8_t*)(null_manhack_patch) = 0xE9;
    *(uint32_t*)(null_manhack_patch+1) = 0xAC;

    /*uint32_t skip_end_clear_global_list = server_srv + 0x006B3FE5;
    *(uint8_t*)(skip_end_clear_global_list) = 0xE9;
    *(uint32_t*)(skip_end_clear_global_list+1) = 0x47;*/

    /*uint32_t save_system_skip = server_srv + 0x00AF3414;
    *(uint8_t*)(save_system_skip) = 0xE9;
    *(uint32_t*)(save_system_skip+1) = 0x165;

    uint32_t lvl_shutdown_skip = server_srv + 0x00737DB9;
    *(uint8_t*)(lvl_shutdown_skip) = 0xE9;
    *(uint32_t*)(lvl_shutdown_skip+1) = 0xA7;*/

    /*uint32_t transition_system_patch_one = server_srv + 0x00AF470F;
    *(uint8_t*)(transition_system_patch_one) = 0xE9;
    *(uint32_t*)(transition_system_patch_one+1) = 0x6C;*/

    uint32_t transition_system_patch_two = server_srv + 0x00AF4B11;
    *(uint8_t*)(transition_system_patch_two) = 0xE9;
    *(uint32_t*)(transition_system_patch_two+1) = -0x208;

    uint32_t transition_system_patch_three = server_srv + 0x00AF47B2;
    *(uint8_t*)(transition_system_patch_three) = 0xE9;
    *(uint32_t*)(transition_system_patch_three+1) = 0x179;

    uint32_t transition_system_patch_four = server_srv + 0x00AF4856;
    *(uint8_t*)(transition_system_patch_four) = 0xE9;
    *(uint32_t*)(transition_system_patch_four+1) = 0xB3;

    /*uint32_t save_system_patch_one = server_srv + 0x00AF3410;
    *(uint8_t*)(save_system_patch_one) = 0xE9;
    *(uint32_t*)(save_system_patch_one+1) = 0x169;*/

    uint32_t transition_savefile_load_patch = server_srv + 0x00AF493D;
    offset = (uint32_t)SavegameInitialLoad - transition_savefile_load_patch - 5;
    *(uint32_t*)(transition_savefile_load_patch+1) = offset;

    uint32_t transition_call_patch_one = server_srv + 0x00AF4A42;
    offset = (uint32_t)TransitionArgUpdateHook - transition_call_patch_one - 5;
    *(uint32_t*)(transition_call_patch_one+1) = offset;

    uint32_t transition_call_patch_two = server_srv + 0x00AF4994;
    offset = (uint32_t)TransitionArgUpdateHookTwo - transition_call_patch_two - 5;
    *(uint32_t*)(transition_call_patch_two+1) = offset;

    uint32_t transition_call_patch_three = server_srv + 0x00AF47A5;
    offset = (uint32_t)TransitionArgUpdateHookThree - transition_call_patch_three - 5;
    *(uint32_t*)(transition_call_patch_three+1) = offset;

    uint32_t remove_evidence_of_call = server_srv + 0x00CACE94;
    *(uint32_t*)(remove_evidence_of_call) = (uint32_t)g_SynUtils.getCppAddr(Hooks::EmptyCall);

    uint32_t remove_evidence_of_call_v2 = server_srv + 0x00CACE60;
    *(uint32_t*)(remove_evidence_of_call_v2) = (uint32_t)g_SynUtils.getCppAddr(Hooks::EmptyCall);

    uint32_t remove_evidence_of_call_v3 = server_srv + 0x00CACE74;
    *(uint32_t*)(remove_evidence_of_call_v3) = (uint32_t)g_SynUtils.getCppAddr(Hooks::EmptyCall);

    uint32_t remove_evidence_of_call_v4 = server_srv + 0x00CACE78;
    *(uint32_t*)(remove_evidence_of_call_v4) = (uint32_t)g_SynUtils.getCppAddr(Hooks::EmptyCall);

    uint32_t remove_evidence_of_call_v5 = server_srv + 0x00CACE6C;
    *(uint32_t*)(remove_evidence_of_call_v5) = (uint32_t)g_SynUtils.getCppAddr(Hooks::EmptyCall);

    uint32_t remove_evidence_of_call_v6 = server_srv + 0x00CACE80;
    *(uint32_t*)(remove_evidence_of_call_v6) = (uint32_t)g_SynUtils.getCppAddr(Hooks::EmptyCall);

    uint32_t remove_evidence_of_call_v7 = server_srv + 0x00CACE84;
    *(uint32_t*)(remove_evidence_of_call_v7) = (uint32_t)g_SynUtils.getCppAddr(Hooks::EmptyCall);

    uint32_t remove_evidence_of_call_v8 = server_srv + 0x00CACE88;
    *(uint32_t*)(remove_evidence_of_call_v8) = (uint32_t)g_SynUtils.getCppAddr(Hooks::EmptyCall);

    uint32_t remove_evidence_of_call_v9 = server_srv + 0x00CACE70;
    *(uint32_t*)(remove_evidence_of_call_v9) = (uint32_t)g_SynUtils.getCppAddr(Hooks::EmptyCall);

    uint32_t player_restore_full_remove = server_srv + 0x00AF4E12;
    *(uint8_t*)(player_restore_full_remove) = 0xE9;
    *(uint32_t*)(player_restore_full_remove+1) = 0x89;

    uint32_t remove_player_file_restoring = server_srv + 0x00AF4E79;
    memset((void*)remove_player_file_restoring, 0x90, 5);
    *(uint16_t*)(remove_player_file_restoring) = 0xC031;

    uint32_t mainPlayersRestorePatch = server_srv + 0x00AF4124;
    *(uint8_t*)(mainPlayersRestorePatch) = 0xEB;

    uint32_t patch_player_restore_asm = server_srv + 0x00AF408B;
    *(uint8_t*)(patch_player_restore_asm) = 0xE9;
    *(uint32_t*)(patch_player_restore_asm+1) = -0x1F7;

    uint32_t clientPutInServerRestoreCancel = server_srv + 0x00B030A1;
    *(uint8_t*)(clientPutInServerRestoreCancel) = 0xEB;

    uint32_t clientActiveRestoreCancel = server_srv + 0x00B031F7;
    *(uint8_t*)(clientActiveRestoreCancel) = 0xEB;

    /*uint32_t orig_dll_discovery_v1 = server_srv + 0x004AE30E;
    *(uint8_t*)(orig_dll_discovery_v1) = 0xE9;
    *(uint32_t*)(orig_dll_discovery_v1+1) = 0x4C;*/

    /*uint32_t orig_dll_discovery_v2 = server_srv + 0x004AE78D;
    ChangeMemoryProtections(orig_dll_discovery_v2, 6);
    memset((void*)orig_dll_discovery_v2, 0x90, 6);
    RestoreMemoryProtections(orig_dll_discovery_v2, 6);*/

    uint32_t patch_restore_stack = server_srv + 0x004AE5B0;
    *(uint8_t*)(patch_restore_stack) = 0x89;
    *(uint8_t*)(patch_restore_stack+1) = 0x1C;
    *(uint8_t*)(patch_restore_stack+2) = 0x24;

    uint32_t patch_restore_base = server_srv + 0x004AE5B3;
    offset = (uint32_t)RestoreSystemPatchStart - patch_restore_base - 5;
    *(uint8_t*)(patch_restore_base) = 0xE8;
    *(uint32_t*)(patch_restore_base+1) = offset;

    uint32_t hook_game_frame_delete_list = server_srv + 0x00739B32;
    offset = (uint32_t)g_SynUtils.getCppAddr(Hooks::GameFrameHook) - hook_game_frame_delete_list - 5;
    *(uint32_t*)(hook_game_frame_delete_list+1) = offset;

    /*uint32_t main_spawn_call_jmp_one = server_srv + 0x0099265B;
    ChangeMemoryProtections(main_spawn_call_jmp_one, 5);
    *(uint8_t*)(main_spawn_call_jmp_one) = 0xE9;
    *(uint32_t*)(main_spawn_call_jmp_one+1) = 0x80;
    RestoreMemoryProtections(main_spawn_call_jmp_one, 5);

    uint32_t main_spawn_call_jmp_two = server_srv + 0x00992834;
    ChangeMemoryProtections(main_spawn_call_jmp_two, 5);
    *(uint8_t*)(main_spawn_call_jmp_two) = 0xE9;
    *(uint32_t*)(main_spawn_call_jmp_two+1) = 0x127;
    RestoreMemoryProtections(main_spawn_call_jmp_two, 5);

    uint32_t main_spawn_call_jmp_three = server_srv + 0x0099279A;
    ChangeMemoryProtections(main_spawn_call_jmp_three, 5);
    *(uint8_t*)(main_spawn_call_jmp_three) = 0xE9;
    *(uint32_t*)(main_spawn_call_jmp_three+1) = 0x20;
    RestoreMemoryProtections(main_spawn_call_jmp_three, 5);

    uint32_t main_spawn_call_jmp_four = server_srv + 0x009925B6;
    ChangeMemoryProtections(main_spawn_call_jmp_four, 5);
    *(uint8_t*)(main_spawn_call_jmp_four) = 0xE9;
    *(uint32_t*)(main_spawn_call_jmp_four+1) = 0x20;
    RestoreMemoryProtections(main_spawn_call_jmp_four, 5);*/


    uint32_t JMP_DIRECT = server_srv + 0x004AE682;
    *(uint8_t*)(JMP_DIRECT) = 0xE9;
    *(uint32_t*)(JMP_DIRECT+1) = -0xD7;

    uint32_t JMP_TOEND_RESTORE = server_srv + 0x00AF4B6A;
    *(uint8_t*)(JMP_TOEND_RESTORE) = 0xE9;
    *(uint32_t*)(JMP_TOEND_RESTORE+1) = 0x143;

    uint32_t JMP_TOEND_RESTORE_THREE = server_srv + 0x00AF4D2A;
    *(uint8_t*)(JMP_TOEND_RESTORE_THREE) = 0xE9;
    *(uint32_t*)(JMP_TOEND_RESTORE_THREE+1) = 0x232;

    uint32_t JMP_PATCH_GLOBALRESTORE_ONE = server_srv + 0x00AF45D0;
    *(uint8_t*)(JMP_PATCH_GLOBALRESTORE_ONE) = 0xE9;
    *(uint32_t*)(JMP_PATCH_GLOBALRESTORE_ONE+1) = 0x45;

    uint32_t remove_end_of_ent_restore = server_srv + 0x004AE434;
    memset((void*)remove_end_of_ent_restore, 0x90, 5);

    *(uint8_t*)(remove_end_of_ent_restore) = 0xBE;
    *(uint8_t*)(remove_end_of_ent_restore+1) = 0x00;
    *(uint8_t*)(remove_end_of_ent_restore+2) = 0x00;
    *(uint8_t*)(remove_end_of_ent_restore+3) = 0x00;
    *(uint8_t*)(remove_end_of_ent_restore+4) = 0x00;

    uint32_t jmp_to_end_restore_func = server_srv + (0x004AE434+5);
    *(uint8_t*)(jmp_to_end_restore_func) = 0xE9;
    *(uint32_t*)(jmp_to_end_restore_func+1) = 0x62;


    //Disable ResponseSystem saving
    uint32_t disable_response_saving = server_srv + 0x00C70710;
    *(uint32_t*)(disable_response_saving) = (uint32_t)g_SynUtils.getCppAddr(Hooks::EmptyCall);

    //Disable EventQueue saving
    /*uint32_t disable_event_queue_saving = server_srv + 0x00C817B0;
    ChangeMemoryProtections(disable_event_queue_saving, 4);
    *(uint32_t*)(disable_event_queue_saving) = (uint32_t)pEmptyCallOneArgPtr;
    RestoreMemoryProtections(disable_event_queue_saving, 4);*/

    //Disable Achievement saving
    uint32_t disable_achievement_saving = server_srv + 0x00C17570;
    *(uint32_t*)(disable_achievement_saving) = (uint32_t)g_SynUtils.getCppAddr(Hooks::EmptyCall);

    //Disable SOMETHING saving
    uint32_t disable_something_saving = server_srv + 0x00D7DE90;
    *(uint32_t*)(disable_something_saving) = (uint32_t)g_SynUtils.getCppAddr(Hooks::EmptyCall);

    //Disable SOMETHING saving
    uint32_t disable_something_saving_two = server_srv + 0x00C84630;
    *(uint32_t*)(disable_something_saving_two) = (uint32_t)g_SynUtils.getCppAddr(Hooks::EmptyCall);

    //Disable SOMETHING saving
    uint32_t disable_something_saving_three = server_srv + 0x00C709F0;
    *(uint32_t*)(disable_something_saving_three) = (uint32_t)g_SynUtils.getCppAddr(Hooks::EmptyCall);



    //Disable AI restoring
    uint32_t disable_ai_restoring = server_srv + 0x00C70A04;
    *(uint32_t*)(disable_ai_restoring) = (uint32_t)g_SynUtils.getCppAddr(Hooks::EmptyCall);

    //Disable Template restoring
    uint32_t disable_template_restoring = server_srv + 0x00D7DEA4;
    *(uint32_t*)(disable_template_restoring) = (uint32_t)g_SynUtils.getCppAddr(Hooks::EmptyCall);

    //Disable ResponseSystem restoring
    uint32_t disable_response_restoring = server_srv + 0x00C70724;
    *(uint32_t*)(disable_response_restoring) = (uint32_t)g_SynUtils.getCppAddr(Hooks::EmptyCall);

    //Disable Commentary restoring
    uint32_t disable_commentary_restoring = server_srv + 0x00C84644;
    *(uint32_t*)(disable_commentary_restoring) = (uint32_t)g_SynUtils.getCppAddr(Hooks::EmptyCall);

    //Disable EventQueue restoring
    /*uint32_t disable_event_queue_restoring = server_srv + 0x00C817C4;
    ChangeMemoryProtections(disable_event_queue_restoring, 4);
    *(uint32_t*)(disable_event_queue_restoring) = (uint32_t)pEmptyCallOneArgPtr;
    RestoreMemoryProtections(disable_event_queue_restoring, 4);*/

    //Disable Achievement restoring
    uint32_t disable_achievement_restoring = server_srv + 0x00C17584;
    *(uint32_t*)(disable_achievement_restoring) = (uint32_t)g_SynUtils.getCppAddr(Hooks::EmptyCall);




    /*uint32_t restore_location_patches[128] = 
    {
        0x004ABE3F+1,0x004AE201+1,0x004AE66D+1,0x004AE6E6+1,0x004AEE98+1,0x00AF225C+1,0x00AF2279+2,0x00AF22F7+1,
        0x00AF319D+1,0x00AF41D8+1,0x00AF4284+1,0x004AA44B+1,0x004AA49B+2,0x004AA559+2,0x004ADE61+1,0x004AF0A5+1,
        0x004AF238+1,0x004AF28B+2,0x004AF423+3,0x004AF556+1,0x00AF2477+1,0x004AD1EB+2,0x004A98E3+2,0x004A9E23+2,
        0x004AA043+2,0x004AA2A3+2,0x004ABAD3+2,0x004ABD33+2,0x004A9537+2,0x004AE96B+2
    };

    for(int i = 0; i < 128; i++)
    {
        if(restore_location_patches[i] == 0)
            continue;

        uint32_t address = server_srv + restore_location_patches[i];

        ChangeMemoryProtections(address, 1);
        *(uint8_t*)(address) = 0x38;
        RestoreMemoryProtections(address, 1);
        rootconsole->ConsolePrint("Patching restore address: [%X]", restore_location_patches[i]);
    }

    //0x004AA4F8
    //0x004ADEEA
    //0x004AF2E8

    uint32_t other_patch_location_one = server_srv + 0x004AF2E8;
    ChangeMemoryProtections(other_patch_location_one, 10);
    memset((void*)other_patch_location_one, 0x90, 7);
    RestoreMemoryProtections(other_patch_location_one, 10);

    uint32_t other_patch_location_two = server_srv + 0x004ADEEA;
    ChangeMemoryProtections(other_patch_location_two, 10);
    memset((void*)other_patch_location_two, 0x90, 7);
    RestoreMemoryProtections(other_patch_location_two, 10);

    uint32_t other_patch_location_three = server_srv + 0x004AA4F8;
    ChangeMemoryProtections(other_patch_location_three, 10);
    memset((void*)other_patch_location_three, 0x90, 7);
    RestoreMemoryProtections(other_patch_location_three, 10);*/

    /*uint32_t patch_0x38_offset[128] = 
    {
        0x004AE3DE,0x004AE325
    };

    for(int i = 0; i < 128; i++)
    {
        if(patch_0x38_offset[i] == 0)
            continue;

        uint32_t address = server_srv + patch_0x38_offset[i];

        ChangeMemoryProtections(address, 3);
        memset((void*)address, 0x90, 3);
        RestoreMemoryProtections(address, 3);
        rootconsole->ConsolePrint("Patching restore address: [%X]", patch_0x38_offset[i]);
    }*/
}

uint32_t GetFileSize(char* file_name)
{
    FILE *fp;
    uint32_t file_offset = 0;
    
    rootconsole->ConsolePrint("Opening file [%s]", file_name);
    fp = fopen(file_name,"r");
    
    if(fp == NULL)
    {
        rootconsole->ConsolePrint("Error in opening file [%s]", file_name);
        return file_offset;
    }

    while(!feof(fp))
    {
        fgetc(fp);
        file_offset++;
    }

   fclose(fp);
   return file_offset;
}

uint32_t GetCBaseEntity(uint32_t EHandle)
{
    uint32_t shift_right = EHandle >> 0x0C;
    uint32_t disassembly = EHandle & 0xFFF;
    disassembly = disassembly << 0x4;
    disassembly = CGlobalEntityList + disassembly;

    if( ((*(uint32_t*)(disassembly+0x08))) == shift_right)
    {
        uint32_t CBaseEntity = *(uint32_t*)(disassembly+0x04);
        return CBaseEntity;
    }

    return 0;
}

uint32_t GetEntityField(uint32_t dmap, uint32_t firstEnt, uint32_t subdmap_offset, uint32_t deep, uint32_t searchField)
{
    while(dmap)
    {
        int fields = *(int*)(dmap+0x04);
        uint32_t dataDesc = *(uint32_t*)dmap;
        const char* dataClassName = (const char*)( *(uint32_t*) (dmap+0x08) );

        dataDesc = dataDesc+0x04;

        for(int i = 0; i < fields; i++)
        {
            const char* fieldName = (const char*)(*(uint32_t*)dataDesc);
            int offset_raw = *(int*)(dataDesc+0x04);
            int offset = offset_raw + subdmap_offset;
            fieldtype_t fieldType = *(fieldtype_t*)(dataDesc-0x04);
            short flags = *(short*)(dataDesc+0xE);
            int fieldSizeInBytes = *(int*)(dataDesc+0x24);
            const char* externalName = (const char*)(*(uint32_t*)(dataDesc+0x10));

            if((flags & (FTYPEDESC_GLOBAL)) || (flags & (FTYPEDESC_SAVE)) || (flags & (FTYPEDESC_KEY)) || (flags & (FTYPEDESC_INPUT)))
            {
                if(strcmp(fieldName, (char*)searchField) == 0)
                {
                    return firstEnt+offset;
                }
            }

            dataDesc = dataDesc+0x38;
        }

        dmap = *(uint32_t*)(dmap+0x0C);
    }

    return 0;
}

void GivePlayerWeapons(uint32_t player_object, bool force_give)
{
    uint32_t equip_coop = 0;
    while((equip_coop = FindEntityByClassname(CGlobalEntityList, equip_coop, (uint32_t)"info_player_equip")) != 0)
    {
        pDynamicOneArgFunc = (pOneArgProt)(*(uint32_t*)((*(uint32_t*)equip_coop)+0x30));
        uint32_t dmap = pDynamicOneArgFunc(equip_coop);
        uint32_t m_bDisabled = GetEntityField(dmap, equip_coop, 0, 0, (uint32_t)"m_bDisabled");

        if((m_bDisabled && *(uint8_t*)(m_bDisabled) == 0) || force_give)
        {
            uint8_t esi_36C = *(uint8_t*)(equip_coop+0x36C);
            *(uint8_t*)(equip_coop+0x36C) = 0;
                
            pDynamicTwoArgFunc = (pTwoArgProt)(  *(uint32_t*)((*(uint32_t*)(equip_coop))+0x2F0)  );
            pDynamicTwoArgFunc(equip_coop, player_object);

            *(uint8_t*)(equip_coop+0x36C) = esi_36C;
        }
    }
}

void SendEntityInput(uint32_t ref_handle, uint32_t inputName, uint32_t activator, uint32_t caller, uint32_t val, uint32_t outputId)
{
    uint32_t entity = GetCBaseEntity(ref_handle);

    if(entity == 0)
    {
        rootconsole->ConsolePrint(EXT_PREFIX "Failed to send entity input - entity does not exist!");
        return;
    }
    
    //rootconsole->ConsolePrint("Sending entity input: [%s] to entity", inputName);
    pDynamicSixArgFunc = (pSixArgProt)(*(uint32_t*)((*(uint32_t*)entity)+0x94));
    pDynamicSixArgFunc(entity, inputName, activator, caller, (uint32_t)&val, outputId);
}

ValueList AllocateValuesList()
{
    ValueList list = (ValueList) malloc(sizeof(ValueList));
    *list = NULL;
    return list;
}

FieldList AllocateFieldList()
{
    FieldList list = (FieldList) malloc(sizeof(FieldList));
  	*list = NULL;
  	return list;
}

MallocRefList AllocateMallocRefList()
{
    MallocRefList list = (MallocRefList) malloc(sizeof(MallocRefList));
    *list = NULL;
    return list;
}

PlayerSaveList AllocatePlayerSaveList()
{
    PlayerSaveList list = (PlayerSaveList) malloc(sizeof(PlayerSaveList));
    *list = NULL;
    return list;
}

SavedEntity* CreateNewSavedEntity(void* entRefHandleInput, void* classnameInput, FieldList fieldListInput)
{
    SavedEntity* savedEnt = (SavedEntity*) malloc(sizeof(SavedEntity));

    savedEnt->refHandle = entRefHandleInput;
    savedEnt->clsname = classnameInput;
    savedEnt->fieldData = fieldListInput;
    savedEnt->nextEnt = NULL;
    return savedEnt;
}

Field* CreateNewField(void* labelInput, void* keyInput, void* typeInput, void* flagsInput, void* offsetInput, ValueList valuesInput)
{
    Field* field = (Field*) malloc(sizeof(Field));

    field->label = labelInput;
    field->key = keyInput;
    field->type = typeInput;
    field->flags = flagsInput;
    field->offset = offsetInput;
    field->fieldVals = valuesInput;
    field->nextField = NULL;
    return field;
}

PlayerSave* CreateNewPlayerSave(SavedEntity* player_save_input)
{
    PlayerSave* player_save = (PlayerSave*) malloc(sizeof(PlayerSave));

    player_save->saved_player = player_save_input;
    player_save->nextPlayer = NULL;
    return player_save;
}

MallocRef* CreateNewMallocRef(void* ref_input, void* size_input, void* alloc_location_input, void* alloc_type_input)
{
    MallocRef* new_ref = (MallocRef*) malloc(sizeof(MallocRef));

    new_ref->ref = ref_input;
    new_ref->ref_size = size_input;
    new_ref->alloc_location = alloc_location_input;
    new_ref->alloc_type = alloc_type_input;
    new_ref->nextRef = NULL;

    return new_ref;
}

Value* CreateNewValue(void* valueInput)
{
    Value* val = (Value*) malloc(sizeof(Value));

    val->value = valueInput;
    val->nextVal = NULL;
    return val;
}

void InsertFieldToFieldList(FieldList list, Field* head)
{
    head->nextField = *list;
    *list = head;
}

void DeleteAllValuesInList(ValueList list, bool free_val, bool lock_mutex)
{
    if(lock_mutex) while(pthread_mutex_trylock(&value_list_lock) != 0);

    if(!list || !*list)
    {
        if(lock_mutex) pthread_mutex_unlock(&value_list_lock);
        return;
    }
    
    Value* aValue = *list;

    while(aValue)
    {
        Value* detachedValue = aValue->nextVal;
        if(free_val) free(aValue->value);
        free(aValue);
        aValue = detachedValue;
    }

    *list = NULL;
    if(lock_mutex) pthread_mutex_unlock(&value_list_lock);
}

void DeleteAllValuesInMallocRefList(MallocRefList list, bool lock_mutex)
{
    if(lock_mutex) while(pthread_mutex_trylock(&malloc_ref_lock) != 0);

    if(!list || !*list)
    {
        if(lock_mutex) pthread_mutex_unlock(&malloc_ref_lock);
        return;
    }
    
    MallocRef* aValue = *list;

    while(aValue)
    {
        MallocRef* detachedValue = aValue->nextRef;
        free(aValue);
        aValue = detachedValue;
    }

    *list = NULL;
    if(lock_mutex) pthread_mutex_unlock(&malloc_ref_lock);
}

void* copy_val(void* val, size_t copy_size) {
    if(val == 0)
        return 0;
    
    void* copy_ptr = malloc(copy_size);
    memcpy(copy_ptr, val, copy_size);
    return copy_ptr;
}

bool IsInValuesList(ValueList list, void* searchVal, bool lock_mutex)
{
    if(lock_mutex) while(pthread_mutex_trylock(&value_list_lock) != 0);

    Value* aValue = *list;

    while(aValue)
    {
        if((uint32_t)aValue->value == (uint32_t)searchVal)
        {
            if(lock_mutex) pthread_mutex_unlock(&value_list_lock);
            return true;
        }
        
        aValue = aValue->nextVal;
    }

    if(lock_mutex) pthread_mutex_unlock(&value_list_lock);
    return false;
}

bool RemoveFromValuesList(ValueList list, void* searchVal, bool lock_mutex)
{
    if(lock_mutex) while(pthread_mutex_trylock(&value_list_lock) != 0);

    Value* aValue = *list;

    if(aValue == NULL)
    {
        if(lock_mutex) pthread_mutex_unlock(&value_list_lock);
        return false;
    }

    //search at the start of the list
    if(((uint32_t)aValue->value) == ((uint32_t)searchVal))
    {
        Value* detachedValue = aValue->nextVal;
        free(*list);
        *list = detachedValue;
        if(lock_mutex) pthread_mutex_unlock(&value_list_lock);
        return true;
    }

    //search the rest of the list
    while(aValue->nextVal)
    {
        if(((uint32_t)aValue->nextVal->value) == ((uint32_t)searchVal))
        {
            Value* detachedValue = aValue->nextVal->nextVal;

            free(aValue->nextVal);
            aValue->nextVal = detachedValue;
            if(lock_mutex) pthread_mutex_unlock(&value_list_lock);
            return true;
        }

        aValue = aValue->nextVal;
    }

    if(lock_mutex) pthread_mutex_unlock(&value_list_lock);
    return false;
}

void InsertToValuesList(ValueList list, Value* head, bool tail, bool duplicate_chk, bool lock_mutex)
{
    if(lock_mutex) while(pthread_mutex_trylock(&value_list_lock) != 0);

    if(duplicate_chk)
    {
        Value* aValue = *list;

        while(aValue)
        {
            if((uint32_t)aValue->value == (uint32_t)head->value)
            {
                if(lock_mutex) pthread_mutex_unlock(&value_list_lock);
                return;
            }
        
            aValue = aValue->nextVal;
        }
    }

    if(tail)
    {
        Value* aValue = *list;

        while(aValue)
        {
            if(aValue->nextVal == NULL)
            {
                aValue->nextVal = head;
                return;
            }

            aValue = aValue->nextVal;
        }
    }

    head->nextVal = *list;
    *list = head;

    if(lock_mutex) pthread_mutex_unlock(&value_list_lock);
}

void InsertToPlayerSaveList(PlayerSaveList list, PlayerSave* head)
{
    head->nextPlayer = *list;
    *list = head;
}

bool IsInMallocRefList(MallocRefList list, void* searchVal, bool lock_mutex)
{
    if(lock_mutex) while(pthread_mutex_trylock(&malloc_ref_lock) != 0);
    
    MallocRef* aValue = *list;

    while(aValue)
    {
        if((uint32_t)aValue->ref == (uint32_t)searchVal)
        {
            if(lock_mutex) pthread_mutex_unlock(&malloc_ref_lock);
            return true;
        }
        
        aValue = aValue->nextRef;
    }

    if(lock_mutex) pthread_mutex_unlock(&malloc_ref_lock);
    return false;
}

uint32_t RemoveAllocationRef(MallocRefList list, void* searchVal, bool lock_mutex)
{
    if(lock_mutex) while(pthread_mutex_trylock(&malloc_ref_lock) != 0);
    
    MallocRef* aValue = *list;

    if(aValue == NULL)
    {
        if(lock_mutex) pthread_mutex_unlock(&malloc_ref_lock);
        return -1;
    }

    //search at the start of the list
    if(((uint32_t)aValue->ref) == ((uint32_t)searchVal))
    {
        MallocRef* detachedValue = aValue->nextRef;
        uint32_t itemSize = (uint32_t)aValue->ref_size;
        free(*list);
        *list = detachedValue;
        if(lock_mutex) pthread_mutex_unlock(&malloc_ref_lock);
        return 0;
    }

    //search the rest of the list
    while(aValue->nextRef)
    {
        if(((uint32_t)aValue->nextRef->ref) == ((uint32_t)searchVal))
        {
            MallocRef* detachedValue = aValue->nextRef->nextRef;

            uint32_t itemSize = (uint32_t)aValue->nextRef->ref_size;
            free(aValue->nextRef);
            aValue->nextRef = detachedValue;
            if(lock_mutex) pthread_mutex_unlock(&malloc_ref_lock);
            return 0;
        }

        aValue = aValue->nextRef;
    }

    if(lock_mutex) pthread_mutex_unlock(&malloc_ref_lock);
    return -1;
}

MallocRef* SearchForMallocRef(MallocRefList list, void* searchVal, bool lock_mutex)
{
    if(lock_mutex) while(pthread_mutex_trylock(&malloc_ref_lock) != 0);

    MallocRef* aValue = *list;

    while(aValue)
    {
        if((uint32_t)aValue->ref == (uint32_t)searchVal)
        {
            if(lock_mutex) pthread_mutex_unlock(&malloc_ref_lock);
            return aValue;
        }
        
        aValue = aValue->nextRef;
    }

    if(lock_mutex) pthread_mutex_unlock(&malloc_ref_lock);
    return NULL;
}

MallocRef* SearchForMallocRefInRange(MallocRefList list, void* searchVal, bool lock_mutex)
{
    if(lock_mutex) while(pthread_mutex_trylock(&malloc_ref_lock) != 0);

    MallocRef* aValue = *list;

    while(aValue)
    {
        if( (  (uint32_t)searchVal >= (uint32_t)aValue->ref  )  &&  ( (uint32_t)searchVal <= ((uint32_t)aValue->ref+(uint32_t)aValue->ref_size)  )  )
        {
            if(lock_mutex) pthread_mutex_unlock(&malloc_ref_lock);
            return aValue;
        }
        
        aValue = aValue->nextRef;
    }

    if(lock_mutex) pthread_mutex_unlock(&malloc_ref_lock);
    return NULL;
}

int MallocRefListSize(MallocRefList list, bool lock_mutex)
{
    if(lock_mutex) while(pthread_mutex_trylock(&malloc_ref_lock) != 0);

    MallocRef* aValue = *list;
    int counter = 0;

    while(aValue)
    {
        counter++;
        aValue = aValue->nextRef;
    }

    if(lock_mutex) pthread_mutex_unlock(&malloc_ref_lock);
    return counter;
}

void InsertToMallocRefList(MallocRefList list, MallocRef* head, bool lock_mutex)
{
    if(lock_mutex) while(pthread_mutex_trylock(&malloc_ref_lock) != 0);
    
    MallocRef* aValue = *list;

    while(aValue)
    {
        if((uint32_t)aValue->ref == (uint32_t)head->ref)
        {
            aValue->ref = head->ref;
            aValue->ref_size = head->ref_size;
            aValue->alloc_location = head->alloc_location;
            aValue->alloc_type = head->alloc_type;
            if(lock_mutex) pthread_mutex_unlock(&malloc_ref_lock);
            return;
        }
        
        aValue = aValue->nextRef;
    }

    head->nextRef = *list;
    *list = head;

    if(lock_mutex)  pthread_mutex_unlock(&malloc_ref_lock);
}

uint32_t Hooks::EmptyCall()
{
    return 0;
}

uint32_t Hooks::HookEntityDelete(uint32_t arg0)
{
    if(arg0 == 0)
    {
        rootconsole->ConsolePrint("Could not kill entity [NULL]");
        return 0;
    }

    uint32_t m_refHandle = *(uint32_t*)(arg0+0x350);
    uint32_t chk_ref = GetCBaseEntity(m_refHandle);

    if(chk_ref)
    {
        char* classname = (char*) ( *(uint32_t*)(chk_ref+0x68) );

        if(strcmp(classname, "player") == 0 && protect_player)
        {
            rootconsole->ConsolePrint(EXT_PREFIX "Tried killing player but was protected!");
            return 0;
        }

        pKillEntityDirectFunc(chk_ref);
    }
    else rootconsole->ConsolePrint(EXT_PREFIX "Could not kill entity [Invalid Ehandle]");

    return 0;
}

FieldList SaveEntityFields(uint32_t dmap, uint32_t firstEnt, uint32_t subdmap_offset, uint32_t deep, FieldList fieldList)
{
    while(dmap)
    {
        int fields = *(int*)(dmap+0x04);
        uint32_t dataDesc = *(uint32_t*)dmap;
        const char* dataClassName = (const char*)( *(uint32_t*) (dmap+0x08) );

        dataDesc = dataDesc+0x04;

        for(int i = 0; i < fields; i++)
        {
            const char* fieldName = (const char*)(*(uint32_t*)dataDesc);
            int offset_raw = *(int*)(dataDesc+0x04);
            int offset = offset_raw + subdmap_offset;
            fieldtype_t fieldType = *(fieldtype_t*)(dataDesc-0x04);
            short flags = *(short*)(dataDesc+0xE);
            int fieldSizeInBytes = *(int*)(dataDesc+0x24);
            const char* externalName = (const char*)(*(uint32_t*)(dataDesc+0x10));

            if((flags & (FTYPEDESC_SAVE)))
            {
                if(fieldType == FIELD_FLOAT || fieldType == FIELD_TIME)
                {
                    float val = *(float*)  (firstEnt+offset);

                    /*if(fieldType == FIELD_FLOAT)
                        rootconsole->ConsolePrint("[FIELD_FLOAT] [%s] [%s] [%f]", fieldName, externalName, val);
                    else
                        rootconsole->ConsolePrint("[FIELD_TIME] [%s] [%s] [%f]", fieldName, externalName, val);*/
                        
                        
                    ValueList vals = AllocateValuesList();
                    Value* val1 = CreateNewValue(copy_val(&val, sizeof(float)));
                    InsertToValuesList(vals, val1, false, false, false);

                    void* fieldNameCpy = 0;
                    if(fieldName != 0) fieldNameCpy = copy_val((void*)fieldName, strlen((char*)fieldName)+1);
                    void* externameNameCpy = 0;
                    if(externalName != 0) externameNameCpy = copy_val((void*)externalName, strlen((char*)externalName)+1);
                    void* fieldTypeCpy = copy_val(&fieldType, sizeof(fieldtype_t));
                    void* flagsCpy = copy_val(&flags, sizeof(short));
                    void* offsetCpy = copy_val(&offset, sizeof(int));
                    
                    Field* newField = CreateNewField(fieldNameCpy, externameNameCpy, fieldTypeCpy, flagsCpy, offsetCpy, vals);
                    InsertFieldToFieldList(fieldList, newField);
                }
                else if(fieldType == FIELD_STRING || fieldType == FIELD_INTEGER || fieldType == FIELD_COLOR32
                || fieldType == FIELD_TICK || fieldType == FIELD_MODELNAME
                || fieldType == FIELD_SOUNDNAME || fieldType == FIELD_MODELINDEX || fieldType == FIELD_MATERIALINDEX)
                {
                    int val = *(int*) (firstEnt+offset);

                    /*if(fieldType == FIELD_STRING)
                        rootconsole->ConsolePrint("[FIELD_STRING] [%s] [%s] [%s]", fieldName, externalName, val);
                    else if(fieldType == FIELD_INTEGER)
                        rootconsole->ConsolePrint("[FIELD_INTEGER] [%s] [%s] [%d]", fieldName, externalName, val);
                    else if(fieldType == FIELD_COLOR32)
                        rootconsole->ConsolePrint("[FIELD_COLOR32] [%s] [%s] [%d]", fieldName, externalName, val);
                    else if(fieldType == FIELD_TICK)
                        rootconsole->ConsolePrint("[FIELD_TICK] [%s] [%s] [%d]", fieldName, externalName, val);
                    else if(fieldType == FIELD_MODELNAME)
                        rootconsole->ConsolePrint("[FIELD_MODELNAME] [%s] [%s] [%s]", fieldName, externalName, val);
                    else if(fieldType == FIELD_SOUNDNAME)
                        rootconsole->ConsolePrint("[FIELD_SOUNDNAME] [%s] [%s] [%s]", fieldName, externalName, val);
                    else if(fieldType == FIELD_MODELINDEX)
                        rootconsole->ConsolePrint("[FIELD_MODELINDEX] [%s] [%s] [%d]", fieldName, externalName, val);
                    else
                        rootconsole->ConsolePrint("[FIELD_MATERIALINDEX] [%s] [%s] [%d]", fieldName, externalName, val);*/

                    ValueList vals = AllocateValuesList();
                    Value* val1 = CreateNewValue(copy_val(&val, sizeof(int)));
                    InsertToValuesList(vals, val1, false, false, false);

                    void* fieldNameCpy = 0;
                    if(fieldName != 0) fieldNameCpy = copy_val((void*)fieldName, strlen((char*)fieldName)+1);
                    void* externameNameCpy = 0;
                    if(externalName != 0) externameNameCpy = copy_val((void*)externalName, strlen((char*)externalName)+1);
                    void* fieldTypeCpy = copy_val(&fieldType, sizeof(fieldtype_t));
                    void* flagsCpy = copy_val(&flags, sizeof(short));
                    void* offsetCpy = copy_val(&offset, sizeof(int));
                    
                    Field* newField = CreateNewField(fieldNameCpy, externameNameCpy, fieldTypeCpy, flagsCpy, offsetCpy, vals);
                    InsertFieldToFieldList(fieldList, newField);
                }
                else if(fieldType == FIELD_VECTOR || fieldType == FIELD_POSITION_VECTOR)
                {
                    if(strcmp(fieldName, "m_vecAbsOrigin") != 0 && strcmp(fieldName, "m_angAbsRotation") != 0 &&
                    strcmp(fieldName, "m_vecOrigin") != 0 && strcmp(fieldName, "m_angRotation") != 0)
                    {
                        dataDesc = dataDesc+0x38;
                        continue;
                    }
                    
                    float first  = *(float*)  (firstEnt+offset);
                    float second = *(float*)  (firstEnt+offset+0x4);
                    float third  = *(float*)  (firstEnt+offset+0x8);

                    /*if(fieldType == FIELD_VECTOR)
                        rootconsole->ConsolePrint("[FIELD_VECTOR] [%s] [%s] [%f] [%f] [%f]", fieldName, externalName, first, second, third);
                    else
                        rootconsole->ConsolePrint("[FIELD_POSITION_VECTOR] [%s] [%s] [%f] [%f] [%f]", fieldName, externalName, first, second, third);*/

                    ValueList vals = AllocateValuesList();
                    Value* val1 = CreateNewValue(copy_val(&first, sizeof(float)));
                    Value* val2 = CreateNewValue(copy_val(&second, sizeof(float)));
                    Value* val3 = CreateNewValue(copy_val(&third, sizeof(float)));
                    InsertToValuesList(vals, val3, false, false, false);
                    InsertToValuesList(vals, val2, false, false, false);
                    InsertToValuesList(vals, val1, false, false, false);

                    void* fieldNameCpy = 0;
                    if(fieldName != 0) fieldNameCpy = copy_val((void*)fieldName, strlen((char*)fieldName)+1);
                    void* externameNameCpy = 0;
                    if(externalName != 0) externameNameCpy = copy_val((void*)externalName, strlen((char*)externalName)+1);
                    void* fieldTypeCpy = copy_val(&fieldType, sizeof(fieldtype_t));
                    void* flagsCpy = copy_val(&flags, sizeof(short));
                    void* offsetCpy = copy_val(&offset, sizeof(int));
                    
                    Field* newField = CreateNewField(fieldNameCpy, externameNameCpy, fieldTypeCpy, flagsCpy, offsetCpy, vals);
                    InsertFieldToFieldList(fieldList, newField);
                }
                else if(fieldType == FIELD_BOOLEAN || fieldType == FIELD_CHARACTER)
                {
                    char val = *(char*)  (firstEnt+offset);

                    /*if(fieldType == FIELD_BOOLEAN)
                        rootconsole->ConsolePrint("[FIELD_BOOLEAN] [%s] [%s] [%hd]", fieldName, externalName, val);
                    else
                        rootconsole->ConsolePrint("[FIELD_CHARACTER] [%s] [%s] [%hd]", fieldName, externalName, val);*/

                    ValueList vals = AllocateValuesList();
                    Value* val1 = CreateNewValue(copy_val(&val, sizeof(char)));
                    InsertToValuesList(vals, val1, false, false, false);

                    void* fieldNameCpy = 0;
                    if(fieldName != 0) fieldNameCpy = copy_val((void*)fieldName, strlen((char*)fieldName)+1);
                    void* externameNameCpy = 0;
                    if(externalName != 0) externameNameCpy = copy_val((void*)externalName, strlen((char*)externalName)+1);
                    void* fieldTypeCpy = copy_val(&fieldType, sizeof(fieldtype_t));
                    void* flagsCpy = copy_val(&flags, sizeof(short));
                    void* offsetCpy = copy_val(&offset, sizeof(int));
                    
                    Field* newField = CreateNewField(fieldNameCpy, externameNameCpy, fieldTypeCpy, flagsCpy, offsetCpy, vals);
                    InsertFieldToFieldList(fieldList, newField);
                }
                else if(fieldType == FIELD_SHORT)
                {
                    short val = *(short*)  (firstEnt+offset);

                    //rootconsole->ConsolePrint("[FIELD_SHORT] [%s] [%s] [%hi]", fieldName, externalName, val);

                    ValueList vals = AllocateValuesList();
                    Value* val1 = CreateNewValue(copy_val(&val, sizeof(short)));
                    InsertToValuesList(vals, val1, false, false, false);

                    void* fieldNameCpy = 0;
                    if(fieldName != 0) fieldNameCpy = copy_val((void*)fieldName, strlen((char*)fieldName)+1);
                    void* externameNameCpy = 0;
                    if(externalName != 0) externameNameCpy = copy_val((void*)externalName, strlen((char*)externalName)+1);
                    void* fieldTypeCpy = copy_val(&fieldType, sizeof(fieldtype_t));
                    void* flagsCpy = copy_val(&flags, sizeof(short));
                    void* offsetCpy = copy_val(&offset, sizeof(int));
                    
                    Field* newField = CreateNewField(fieldNameCpy, externameNameCpy, fieldTypeCpy, flagsCpy, offsetCpy, vals);
                    InsertFieldToFieldList(fieldList, newField);
                }
                /*else if(fieldType == FIELD_EMBEDDED)
                {
                    uint32_t sub_dmap = *(uint32_t*)(dataDesc+0x20);

                    if(sub_dmap != 0x0)
                    {
                        SaveEntityFields(sub_dmap, firstEnt, subdmap_offset+offset, deep+1, fieldList);
                    }
                }*/
            }

            dataDesc = dataDesc+0x38;
        }

        dmap = *(uint32_t*)(dmap+0x0C);
    }

    return fieldList;
}

uint32_t CallocHook(uint32_t nitems, uint32_t size)
{
    void* returnAddr = __builtin_return_address(0);

    uint32_t enlarged_size = nitems*2.0;
    uint32_t newRef = (uint32_t)calloc(enlarged_size, size);
    //rootconsole->ConsolePrint("malloc() ref: [%X] size: [%X] list_size [%d]", newRef, size, MallocRefListSize(mallocAllocations));
    //rootconsole->ConsolePrint("malloc() ref: [%X] size: [%X]", newRef, size);

    /*MallocRef* new_ref_value = CreateNewMallocRef((void*)newRef, (void*)enlarged_size, (void*)((uint32_t)returnAddr - 5), (void*)"malloc");
    InsertToMallocRefList(mallocAllocations, new_ref_value, true);*/

    return newRef;
}

uint32_t MallocHook(uint32_t size)
{
    if(size <= 0) return (uint32_t)malloc(size);

    void* returnAddr = __builtin_return_address(0);
    uint32_t newRef = (uint32_t)malloc(size*2.5);
    //rootconsole->ConsolePrint("malloc() ref: [%X] size: [%X] list_size [%d]", newRef, size, MallocRefListSize(mallocAllocations));
    //rootconsole->ConsolePrint("malloc() ref: [%X] size: [%X]", newRef, size);

    /*MallocRef* new_ref_value = CreateNewMallocRef((void*)newRef, (void*)enlarged_size, (void*)((uint32_t)returnAddr - 5), (void*)"malloc");
    InsertToMallocRefList(mallocAllocations, new_ref_value, true);*/

    return newRef;
}

uint32_t ReallocHook(uint32_t old_ptr, uint32_t new_size)
{
    if(new_size <= 0) return (uint32_t)realloc((void*)old_ptr, new_size);

    void* returnAddr = __builtin_return_address(0);
    uint32_t new_ref = (uint32_t)realloc((void*)old_ptr, new_size*2.5);

    /*RemoveAllocationRef(mallocAllocations, (void*)old_ptr, true);
    MallocRef* new_ref_value = CreateNewMallocRef((void*)new_ref, (void*)new_size, (void*)((uint32_t)returnAddr - 5), (void*)"malloc");
    InsertToMallocRefList(mallocAllocations, new_ref_value, true);*/

    return new_ref;
}

uint32_t FreeHook(uint32_t ref_tofree)
{
    if(ref_tofree == 0)
        return 0;

    void* returnAddr = __builtin_return_address(0);

    MallocRef* searchResult = SearchForMallocRef(mallocAllocations, (void*)ref_tofree, true);

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

        RemoveAllocationRef(mallocAllocations, (void*)ref_tofree, true);
        return 0;
    }

    //rootconsole->ConsolePrint("error in free detected - [%X]", ((uint32_t)returnAddr - 5));
    free((void*)ref_tofree);
    return 0;
}

uint32_t OperatorNewHook(uint32_t size)
{
    if(size <= 0) return (uint32_t)malloc(size);

    void* returnAddr = __builtin_return_address(0);
    uint32_t newRef = (uint32_t)malloc(size*2.5);
    //rootconsole->ConsolePrint("malloc() ref: [%X] size: [%X] list_size [%d]", newRef, size, MallocRefListSize(mallocAllocations));
    //rootconsole->ConsolePrint("malloc() ref: [%X] size: [%X]", newRef, size);

    /*MallocRef* new_ref_value = CreateNewMallocRef((void*)newRef, (void*)size, (void*)((uint32_t)returnAddr - 5), (void*)"operator_new");
    InsertToMallocRefList(mallocAllocations, new_ref_value, true);*/

    return newRef;
}

uint32_t OperatorNewArrayHook(uint32_t size)
{
    if(size <= 0) return (uint32_t)malloc(size);
    return (uint32_t)malloc(size*2.5);
    

    void* returnAddr = __builtin_return_address(0);

    uint32_t newRef = (uint32_t)operator new[](size);
    //rootconsole->ConsolePrint("malloc() ref: [%X] size: [%X] list_size [%d]", newRef, size, MallocRefListSize(mallocAllocations));
    //rootconsole->ConsolePrint("malloc() ref: [%X] size: [%X]", newRef, size);

    MallocRef* new_ref_value = CreateNewMallocRef((void*)newRef, (void*)size, (void*)((uint32_t)returnAddr - 5), (void*)"operator_new_array");
    InsertToMallocRefList(mallocAllocations, new_ref_value, true);

    return newRef;
}

uint32_t DeleteOperatorHook(uint32_t ref_tofree)
{
    if(ref_tofree == 0)
        return 0;
        
    free((void*)ref_tofree);
    return 0;

    void* returnAddr = __builtin_return_address(0);

    MallocRef* searchResult = SearchForMallocRef(mallocAllocations, (void*)ref_tofree, true);

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

        RemoveAllocationRef(mallocAllocations, (void*)ref_tofree, true);
        return 0;
    }

    //rootconsole->ConsolePrint("error in delete detected - [%X]", ((uint32_t)returnAddr - 5));
    operator delete ((void*)ref_tofree);
    return 0;
}

uint32_t DeleteOperatorArrayHook(uint32_t ref_tofree)
{
    if(ref_tofree == 0)
        return 0;

    free((void*)ref_tofree);
    return 0;
        
    void* returnAddr = __builtin_return_address(0);

    MallocRef* searchResult = SearchForMallocRef(mallocAllocations, (void*)ref_tofree, true);

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

        RemoveAllocationRef(mallocAllocations, (void*)ref_tofree, true);
        return 0;
    }
    
    //rootconsole->ConsolePrint("error in delete[] detected - [%X]", ((uint32_t)returnAddr - 5));
    operator delete[] ((void*)ref_tofree);
    return 0;
}

uint32_t MemcpyHook(uint32_t dest, uint32_t src, uint32_t size)
{
    MallocRef* searchResult = SearchForMallocRefInRange(mallocAllocations, (void*)dest, true);

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

uint32_t MemsetHook(uint32_t dest, uint32_t byte, uint32_t size)
{   
    MallocRef* searchResult = SearchForMallocRefInRange(mallocAllocations, (void*)dest, true);

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

uint32_t MemmoveHook(uint32_t dest, uint32_t src, uint32_t size)
{   
    MallocRef* searchResult = SearchForMallocRefInRange(mallocAllocations, (void*)dest, true);

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

uint32_t StrncpyHook(uint32_t dest, uint32_t src, uint32_t size)
{   
    MallocRef* searchResult = SearchForMallocRefInRange(mallocAllocations, (void*)dest, true);

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

uint32_t StrcpyHook(uint32_t dest, uint32_t src)
{   
    MallocRef* searchResult = SearchForMallocRefInRange(mallocAllocations, (void*)dest, true);

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

uint32_t SaveGameSafe(bool use_internal_savename)
{
    uint32_t deleteQueue = *(uint32_t*)(server_srv + 0x0100890C);
    uint32_t physQueue = *(uint32_t*)(server_srv + 0x01032AF0);
    uint32_t unk_arg0 = server_srv + 0x01032900;

    physQueue = physQueue + *(uint32_t*)(unk_arg0+0xC8);
    physQueue = physQueue + *(uint32_t*)(unk_arg0+0xF0);
    physQueue = physQueue + *(uint32_t*)(unk_arg0+0x104);
    physQueue = physQueue + *(uint32_t*)(unk_arg0+0x154);

    if(deleteQueue == 0 && physQueue == 0)
    {
        pDynamicOneArgFunc = (pOneArgProt)(server_srv + 0x007BA940);
        pDynamicOneArgFunc(0);

        uint32_t mainEnt = 0;
        while((mainEnt = FindEntityByClassname(CGlobalEntityList, mainEnt, (uint32_t)"*")) != 0)
        {
            char* classname = (char*) ( *(uint32_t*)(mainEnt+0x68) );
            
            if(strcmp(classname, "prop_vehicle_jeep") != 0 && strcmp(classname, "prop_vehicle_mp") != 0)
                continue;

            pDynamicOneArgFunc = (pOneArgProt)(*(uint32_t*)((*(uint32_t*)mainEnt)+0x30));
            uint32_t dmap = pDynamicOneArgFunc(mainEnt);
            uint32_t m_ModelName = GetEntityField(dmap, mainEnt, 0, 0, (uint32_t)"m_ModelName");

            if(m_ModelName == 0)
                continue;

            uint32_t model = *(uint32_t*)m_ModelName;

            if(model == 0)
                continue;

            bool fixed_name = false;

            for(int i = 0; i <= (int)strlen((char*)model); i++)
            {
                char byte = (char)(*(uint8_t*)(model+i));

                if(byte == '\\')
                {
                    *(uint8_t*)(model+i) = (uint8_t)'/';
                    fixed_name = true;
                }
            }

            if(fixed_name)
            {
                rootconsole->ConsolePrint("FIXED_MODEL_NAME: [%s]", model);
            }
        }

        if(use_internal_savename)
        {
            if(strcmp((char*)((*(uint32_t*)(server_srv + 0x00FA0CF0)) + 0x131), "") != 0)
            {
                rootconsole->ConsolePrint("Saving game to: %s", ((*(uint32_t*)(server_srv + 0x00FA0CF0)) + 0x131));
                return SaveGameState((*(uint32_t*)(server_srv + 0x00FA0CF0)), (uint32_t)((*(uint32_t*)(server_srv + 0x00FA0CF0)) + 0x131), (uint32_t)"AUTOSAVE");
            }
            
            return 0;
        }

        rootconsole->ConsolePrint("Saving game to: autosave");
        return SaveGameState((*(uint32_t*)(server_srv + 0x00FA0CF0)), (uint32_t)"autosave", (uint32_t)"AUTOSAVE");
    }

    return 0;
}

uint32_t FrameLockHook(uint32_t arg0)
{
    CleanupDeleteList(0);
    pDynamicOneArgFunc = (pOneArgProt)(server_srv + 0x006863F0);
    pDynamicOneArgFunc(server_srv + 0x00FF3020);
    
    rootconsole->ConsolePrint(EXT_PREFIX "Saving game for transition!");
    CleanupDeleteList(0);
    SaveGameSafe(true);
    CleanupDeleteList(0);

    //inactivate earlier
    InactivateClients(sv);
    CleanupDeleteList(0);

    pDynamicOneArgFunc = (pOneArgProt)(datacache_srv + 0x00038060);
    return pDynamicOneArgFunc(arg0);
}

uint32_t Hooks::GameFrameHook(uint8_t simulating)
{
    CleanupDeleteList(0);

    if(savegame && !savegame_lock && !gamestart_lock)
    {
        frames = 0;
        savegame_lock = true;
    }
    else if(savegame && frames >= 20)
    {
        if(!restoring) SaveGameSafe(false);
        savegame = false;
        savegame_lock = false;
    }
    else if(gamestart && !savegame && !savegame_lock && !gamestart_lock)
    {
        frames = 0;
        gamestart_lock = true;
    }
    else if(gamestart && frames >= 100)
    {
        rootconsole->ConsolePrint("\n\nGame started map [%s]\n\n", global_map);
        ReconnectClients(sv);
        gamestart = false;
        gamestart_lock = false;
    }
    else if(restore_delay && !restore_delay_lock)
    {
        frames = 0;
        restore_delay_lock = true;
    }
    else if(restore_delay && frames >= 50)
    {
        restoring = false;
        restore_delay = false;
        restore_delay_lock = false;
    }
    else if(frames >= 500)
        frames = 0;

    CleanupDeleteList(0);
    frames++;

    //Call orig funcs

    uint32_t ent = FindEntityByClassname(CGlobalEntityList, 0, (uint32_t)"player");

    //Dont simulate if there is no active player
    if(ent == 0)
        return 0;

    SimulateEntities((bool)simulating);

    //UpdateClientData
    pDynamicOneArgFunc = (pOneArgProt)(server_srv + 0x00A6A660);
    pDynamicOneArgFunc(0);

    //StartFrame
    pDynamicOneArgFunc = (pOneArgProt)(server_srv + 0x00B03590);
    pDynamicOneArgFunc(0);

    //ServiceEventQueue
    pDynamicOneArgFunc = (pOneArgProt)(server_srv + 0x00687440);
    pDynamicOneArgFunc(0);

    //PostSystems
    pDynamicOneArgFunc = (pOneArgProt)(server_srv + 0x00471320);
    pDynamicOneArgFunc(0);
    return 0;
}

void SimulateEntities(bool simulating)
{
    CleanupDeleteList(0);
    
    if(simulating)
    {
        uint32_t ent = 0;
        
        while((ent = FindEntityByClassname(CGlobalEntityList, ent, (uint32_t)"*")) != 0)
        {
            pDynamicOneArgFunc = (pOneArgProt)(server_srv + 0x00A311D0);
            pDynamicOneArgFunc(ent);
        }
    }
    else
    {
        uint32_t ent = 0;
        
        while((ent = FindEntityByClassname(CGlobalEntityList, ent, (uint32_t)"player")) != 0)
        {
            pDynamicOneArgFunc = (pOneArgProt)(server_srv + 0x00A52B70);
            pDynamicOneArgFunc(ent);

            pDynamicOneArgFunc = (pOneArgProt)(server_srv + 0x00A311D0);
            pDynamicOneArgFunc(ent);
        }
    }
}

void SaveLinkedList(ValueList leakList)
{
    char path[512];
    char* root_dir = getenv("PWD");
    snprintf(path, sizeof(path), "%s/leaked_resources.txt", root_dir);

    FILE* list_file = fopen(path, "a");

    if(!list_file && !leakList && !*leakList)
    {
        rootconsole->ConsolePrint("Error saving leaked resources!");
        return;
    }

    char listName[256];
    snprintf(listName, 256, "Unknown List");

    if(leakList == leakedResourcesSaveRestoreSystem)
        snprintf(listName, 256, "leakedResourcesSaveRestoreSystem");
    else if(leakList == leakedResourcesEdtSystem)
        snprintf(listName, 256, "leakedResourcesEdtSystem");

    rootconsole->ConsolePrint("Saving leaked resources list [%s]", listName);

    fprintf(list_file, "%s\n", listName); 

    Value* current = *leakList;

    while(current)
    {        
        fprintf(list_file, "%X\n", (uint32_t)current->value); 
        current = current->nextVal;
    }

    fclose(list_file);
}

void RestoreLinkedLists()
{
    leakedResourcesSaveRestoreSystem = AllocateValuesList();
    leakedResourcesEdtSystem = AllocateValuesList();

    ValueList currentRestoreList = NULL;

    char path[512];
    char* root_dir = getenv("PWD");
    snprintf(path, sizeof(path), "%s/leaked_resources.txt", root_dir);
    FILE* list_file = fopen(path, "r");    

    if(!list_file)
    {
        rootconsole->ConsolePrint("Error restoring leaked resources!");
        return;
    }

    char* file_line = (char*) malloc(1024);
    fgets(file_line, 1024, list_file);

    int ppid_file = strtol(file_line, NULL, 10);

    if(ppid_file != getppid())
    {
        rootconsole->ConsolePrint("Leaked resources were not restored due to parent process not matching!");
        fclose(list_file);
        return;
    }

    while(fgets(file_line, 1024, list_file))
    {
        sscanf(file_line, "%[^\n]s", file_line);
        if(strcmp(file_line, "\n") == 0) 
            continue;

        if(strncmp(file_line, "leakedResourcesSaveRestoreSystem", 32) == 0)
        {
            currentRestoreList = leakedResourcesSaveRestoreSystem;
            continue;
        }
        else if(strncmp(file_line, "leakedResourcesEdtSystem", 24) == 0)
        {
            currentRestoreList = leakedResourcesEdtSystem;
            continue;
        }

        if(!currentRestoreList)
            continue;

        uint32_t parsedRef = strtoul(file_line, NULL, 16);
        Value* leak = CreateNewValue((void*)parsedRef);
        rootconsole->ConsolePrint("Restored leaked reference: [%X]", parsedRef);
        InsertToValuesList(currentRestoreList, leak, false, true, false);
    }

    free(file_line);
    fclose(list_file);
}

void ReleaseLeakedMemory(ValueList leakList, bool destroy)
{
    if(!leakList)
        return;
    
    Value* leak = *leakList;
    char listName[256];
    snprintf(listName, 256, "Unknown List");

    if(leakList == leakedResourcesSaveRestoreSystem)
        snprintf(listName, 256, "Save/Restore Hook");
    else if(leakList == leakedResourcesEdtSystem)
        snprintf(listName, 256, "EDT Hook");

    if(!leak)
    {
        if(destroy)
        {
            free(leakList);
            leakList = NULL;
            return;
        }

        rootconsole->ConsolePrint("[%s] Attempted to free leaks from an empty leaked resources list!", listName);
        return;
    }

    if(destroy)
    {
        //Save references to be freed if extension is reloaded
        SaveLinkedList(leakList);
    }

    while(leak)
    {
        Value* detachedValue = leak->nextVal;

        if(!destroy)
        {
            rootconsole->ConsolePrint("[%s] FREED MEMORY LEAK WITH REF: [%X]", listName, leak->value);
            free(leak->value);
        }

        free(leak);
        leak = detachedValue;
    }

    *leakList = NULL;

    if(destroy)
    {
        free(leakList);
    }
}

void DestroyLinkedLists()
{
    ReleaseLeakedMemory(leakedResourcesSaveRestoreSystem, true);
    ReleaseLeakedMemory(leakedResourcesEdtSystem, true);

    rootconsole->ConsolePrint("---  Linked lists successfully destroyed  ---");
}

void SaveProcessId()
{
    char path[512];
    char* root_dir = getenv("PWD");
    snprintf(path, sizeof(path), "%s/leaked_resources.txt", root_dir);

    FILE* list_file = fopen(path, "w");

    if(!list_file)
    {
        rootconsole->ConsolePrint("Error saving leaked resources!");
        return;
    }

    fprintf(list_file, "%d\n", getppid()); 
    fclose(list_file);
}

void ReleasePlayerSavedList()
{
    PlayerSave* savedPlayer = *playerSaveList;

    while(savedPlayer)
    {
        PlayerSave* detachedPlayerSave = savedPlayer->nextPlayer;
        SavedEntity* savedEntity = savedPlayer->saved_player;

        FieldList fields = savedEntity->fieldData;
        Field* field = *fields;

        while(field)
        {
            Field* detachedField = field->nextField;

            DeleteAllValuesInList(field->fieldVals, true, false);
            free(field->label);
            free(field->key);
            free(field->type);
            free(field->flags);
            free(field->offset);
            free(field->fieldVals);

            free(field);
            field = detachedField;
        }

        free(savedEntity->clsname);
        free(savedEntity->fieldData);

        free(savedEntity);
        free(savedPlayer);

        savedPlayer = detachedPlayerSave;
    }

    *playerSaveList = NULL;
}

SavedEntity* SaveEntity(uint32_t firstEnt)
{
    if(firstEnt == 0)
        return 0;

    uint32_t m_RefEHandle = *(uint32_t*)(firstEnt+0x350);
    uint32_t classlabel = *(uint32_t*)(firstEnt+0x68);

    rootconsole->ConsolePrint("saving entity with classname: [%s]", classlabel);

    pDynamicOneArgFunc = (pOneArgProt)(*(uint32_t*)((*(uint32_t*)firstEnt)+0x30));
    uint32_t dmap = pDynamicOneArgFunc(firstEnt);

    FieldList fieldList = AllocateFieldList();
    void* classlabelCpy = copy_val((void*)classlabel, strlen((char*)classlabel)+1);
    return CreateNewSavedEntity((void*)m_RefEHandle, classlabelCpy, SaveEntityFields(dmap, firstEnt, 0, 0, fieldList));
}

void SavePlayers()
{
    ReleasePlayerSavedList();
    uint32_t firstEnt = 0;

    // Save players
    while((firstEnt = FindEntityByClassname(CGlobalEntityList, firstEnt, (uint32_t)"player")) != 0)
    {
        SavedEntity* saved_player = SaveEntity(firstEnt);

        uint32_t playerRefHandle = *(uint32_t*)(firstEnt+0x350);
        uint32_t m_Network = *(uint32_t*)(firstEnt+0x24);
        uint16_t playerIndex = *(uint16_t*)(m_Network+0x6);

        PlayerSave* player_save = CreateNewPlayerSave(saved_player);
        InsertToPlayerSaveList(playerSaveList, player_save);
    }
}

void DisablePlayerViewControl()
{
    uint32_t view_control = 0;
    while((view_control = FindEntityByClassname(CGlobalEntityList, view_control, (uint32_t)"point_viewcontrol")) != 0)
    {
        pDynamicOneArgFunc = (pOneArgProt)(server_srv + 0x00B52180);
        pDynamicOneArgFunc(view_control);
    }
}

void EnablePlayerViewControl()
{
    uint32_t view_control = 0;
    while((view_control = FindEntityByClassname(CGlobalEntityList, view_control, (uint32_t)"point_viewcontrol")) != 0)
    {
        uint32_t m_refHandle = *(uint32_t*)(view_control+0x350);
        
        pDynamicOneArgFunc = (pOneArgProt)(*(uint32_t*)((*(uint32_t*)view_control)+0x30));
        uint32_t dmap = pDynamicOneArgFunc(view_control);

        uint32_t m_state = GetEntityField(dmap, view_control, 0, 0, (uint32_t)"m_state");

        if(m_state && *(uint8_t*)(m_state) == 1)
        {
            pDynamicOneArgFunc = (pOneArgProt)(server_srv + 0x00B52180);
            pDynamicOneArgFunc(view_control);
            
            SendEntityInput(m_refHandle, (uint32_t)"Enable", 0, 0, 0, (uint32_t)-1);
        }
    }
}

void PatchAutosave()
{
    uint32_t autosave_call_one = server_srv + 0x00F81BFC;
    uint32_t autosave_call_two = server_srv + 0x00AF3D30;
    uint32_t autosave_call_three = server_srv + 0x00D86E58;
    uint32_t autosave_call_four = server_srv + 0x00D87198;
    uint32_t autosave_call_five = server_srv + 0x00737F76;

    //*(uint32_t*)(autosave_call_one) = (uint32_t)pCLogicAutosaveMainFuncPtr;

    //memset((void*)autosave_call_two, 0x90, 5);
    //*(uint16_t*)(autosave_call_two) = 0xC031;

    //*(uint32_t*)(autosave_call_three) = (uint32_t)pCTriggerToggleSaveMainFuncPtr;
    //*(uint32_t*)(autosave_call_four) = (uint32_t)pCTriggerSaveMainFuncPtr;


    uint32_t offset = (uint32_t)g_SynUtils.getCppAddr(Hooks::EmptyCall) - autosave_call_five - 5;
    *(uint32_t*)(autosave_call_five+1) = offset;

    rootconsole->ConsolePrint("--------------------- Save system patched ---------------------");
}

void PatchRestore()
{
    int length = 5;

    uint32_t restore_call_one = server_srv + 0x0073C780;
    uint32_t restore_call_two = server_srv + 0x00AF46AB;
    uint32_t restore_call_three = server_srv + 0x004D9A40;

    uint32_t offset_three = (uint32_t)RestoreOverride - restore_call_three - 5;

    memset((void*)restore_call_one, 0x90, length);
    *(uint16_t*)(restore_call_one) = 0xC031;

    //memset((void*)restore_call_two, 0x90, length);
    //*(uint16_t*)(restore_call_two) = 0xC031;

    *(uint8_t*)(restore_call_three) = 0xE8;
    *(uint32_t*)(restore_call_three+1) = offset_three;

    rootconsole->ConsolePrint("--------------------- Restore system patched ---------------------");
}

void HookSaveRestoreOne()
{
    int length_one = 5;
    int length_two = 1;

    uint32_t patch_location_one = server_srv + 0x004AA3E2;
    uint32_t patch_location_two = server_srv + 0x004AA3EB;

    uint32_t offset = (uint32_t)SaveHookDirectMalloc - patch_location_one - 5;

    *(uint8_t*)(patch_location_one) = 0xE8;
    *(uint32_t*)(patch_location_one + 1) = offset;

    *(uint8_t*)(patch_location_two) = 0xEB;

    rootconsole->ConsolePrint("--------------------- Save/Restore part 1 hooked ---------------------");
}

void HookSaveRestoreTwo()
{
    int length = 5;

    uint32_t start = server_srv + 0x004B04B7;
    uint32_t offset = (uint32_t)SaveHookDirectMalloc - start - 5;

    *(uint8_t*)(start) = 0xE8;
    *(uint32_t*)(start+1) = offset;

    rootconsole->ConsolePrint("--------------------- Save/Restore part 2 hooked ---------------------");
}

void HookSaveRestoreThree()
{
    int length = 5;

    uint32_t start = server_srv + 0x004B0442;
    uint32_t offset = (uint32_t)SaveHookDirectRealloc - start - 5;

    *(uint8_t*)(start) = 0xE8;
    *(uint32_t*)(start+1) = offset;

    rootconsole->ConsolePrint("--------------------- Save/Restore part 3 hooked ---------------------");
}

void HookSavingOne()
{
    int length = 5;

    uint32_t start = server_srv + 0x004B0262;
    uint32_t offset = (uint32_t)SaveHookDirectRealloc - start - 5;

    *(uint8_t*)(start) = 0xE8;
    *(uint32_t*)(start+1) = offset;

    rootconsole->ConsolePrint("--------------------- Save part 1 hooked ---------------------");
}

void HookSavingTwo()
{
    int length = 5;

    uint32_t start = server_srv + 0x004B02A3;
    uint32_t offset = (uint32_t)SaveHookDirectMalloc - start - 5;

    *(uint8_t*)(start) = 0xE8;
    *(uint32_t*)(start+1) = offset;

    rootconsole->ConsolePrint("--------------------- Save part 2 hooked ---------------------");
}

void HookEdtSystem()
{
    int length = 5;

    uint32_t start = server_srv + 0x00AEFC34;
    uint32_t offset = (uint32_t)EdtSystemHookFunc - start - 5;

    *(uint8_t*)(start) = 0xE8;
    *(uint32_t*)(start+1) = offset;

    rootconsole->ConsolePrint("--------------------- EDT system hooked ---------------------");   
}

void PatchEdtSystem()
{
    int length = 5;

    uint32_t start = server_srv + 0x0073C6CC;
    uint32_t offset = (uint32_t)PreEdtLoad - start - 5;

    *(uint8_t*)(start) = 0xE8;
    *(uint32_t*)(start+1) = offset;

    rootconsole->ConsolePrint("--------------------- EDT system patched ---------------------");   
}

void PatchDropships()
{
    int length = 5;
    int length_two = 4;

    uint32_t patch_location_one = server_srv + 0x0085F22C;
    uint32_t patch_location_two = server_srv + 0x0085F266;

    uint32_t patch_location_three = server_srv + 0x00CF1D58;
    uint32_t patch_location_four = server_srv + 0x0086B1B7;

    *(uint32_t*)(patch_location_three) = (uint32_t)g_SynUtils.getCppAddr(Hooks::EmptyCall);

    uint32_t offset_one = (uint32_t)DropshipsHook - patch_location_one - 5;
    uint32_t offset_two = (uint32_t)DropshipsHook - patch_location_two - 5;
    uint32_t offset_three = (uint32_t)CallLater - patch_location_four - 5;

    *(uint8_t*)((uint32_t)patch_location_one) = 0xE8;
    *(uint32_t*)((uint32_t)patch_location_one + 1) = offset_one;

    *(uint8_t*)((uint32_t)patch_location_two) = 0xE8;
    *(uint32_t*)((uint32_t)patch_location_two + 1) = offset_two;

    *(uint8_t*)((uint32_t)patch_location_four) = 0xE8;
    *(uint32_t*)((uint32_t)patch_location_four + 1) = offset_three;

    rootconsole->ConsolePrint("--------------------- Dropships patched ---------------------");
}

void HookSpawnServer()
{
    int length = 5;

    uint32_t start = engine_srv + 0x0012AA56;
    uint32_t offset = (uint32_t)SpawnServerHookFunc - start - 5;

    *(uint8_t*)(start) = 0xE8;
    *(uint32_t*)(start+1) = offset;


    rootconsole->ConsolePrint("--------------------- SpawnServer hooked ---------------------");   
}

void HookHostChangelevel()
{
    int length = 5;

    uint32_t start = engine_srv + 0x00136865;
    uint32_t offset = (uint32_t)HostChangelevelHook - start - 5;

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
    uint32_t patch_location_thirteen = server_srv + 0x0073BC3F;
    uint32_t patch_location_fourteen = server_srv + 0x00AF4D91;
    uint32_t patch_location_fifthteen = server_srv + 0x00D65010;
    uint32_t patch_location_sixteen = engine_srv + 0x001CB601;
    uint32_t patch_location_seventeen = server_srv + 0x00D65550;
    uint32_t patch_location_eighteen = server_srv + 0x008B2A8C;
    uint32_t patch_location_nineteen = server_srv + 0x008E7D32;
    uint32_t patch_location_twenty = server_srv + 0x008E7D35;
    uint32_t patch_location_twentyone = server_srv + 0x008B2A7B;




    *(uint8_t*)(patch_location_one + 1) = 0xA7;


    
    uint32_t offset = (uint32_t)SaveRestoreMemManage - patch_location_two - 5;
    *(uint8_t*)(patch_location_two) = 0xE8;
    *(uint32_t*)(patch_location_two+1) = offset;
    memset((void*)(patch_location_two+5), 0x90, length_two - 5);


    //*(uint8_t*)(patch_location_three) = 0xE9;
    //*(uint32_t*)(patch_location_three + 1) = -0x148;

    //memset((void*)patch_location_four, 0x90, length_four);
    //*(uint16_t*)(patch_location_four) = 0xC031;
    
    *(uint32_t*)(patch_location_four) = (uint32_t)CreateEntityByNameHook;

    //*(uint32_t*)(patch_location_four) = (uint32_t)phook_53D20FPtr;

    //offset = ((uint32_t)pMapInitEndPtr) - patch_location_five - 5;
    //*(uint32_t*)(patch_location_five + 1) = offset;

    memset((void*)patch_location_five, 0x90, length_five);
    //*(uint16_t*)(patch_location_five) = 0xC031;

    memset((void*)patch_location_six, 0x90, length_six);

    memset((void*)patch_location_seven, 0x90, length_seven);


    offset = (uint32_t)FixManhackCrash - patch_location_eight - 5;
    *(uint32_t*)(patch_location_eight + 1) = offset;
    //*(uint8_t*)(patch_location_eight) = 0xE9;
    //*(uint32_t*)(patch_location_eight + 1) = 0x364;
    //memset((void*)patch_location_eight, 0x90, length_eight);
    //*(uint8_t*)(patch_location_eight) = 0xEB;
    //*(uint8_t*)(patch_location_eight+1) = -0x47;

    memset((void*)patch_location_nine, 0x90, length_nine);
    *(uint16_t*)(patch_location_nine) = 0xC031;

    memset((void*)patch_location_eleven, 0x90, length_eleven);
    *(uint16_t*)(patch_location_eleven) = 0xC031;

    offset = (uint32_t)SV_TriggerMovedFix - patch_location_sixteen - 5;
    *(uint32_t*)(patch_location_sixteen + 1) = offset;


    *(uint8_t*)(patch_location_fourteen) = 0xE9;
    *(uint32_t*)(patch_location_fourteen + 1) = 0x1F0;

    uint32_t offset_two = (uint32_t)MainTransitionRestore - patch_location_thirteen - 5;
    *(uint8_t*)(patch_location_thirteen) = 0xE8;
    *(uint32_t*)(patch_location_thirteen + 1) = offset_two;

    uint32_t offset_three = (uint32_t)TransitionEntityCreateCall - patch_location_twelve - 5;
    *(uint8_t*)(patch_location_twelve) = 0xE8;
    *(uint32_t*)(patch_location_twelve + 1) = offset_three;

    uint32_t offset_four = (uint32_t)TransitionRestoreMain - patch_location_ten - 5;
    *(uint8_t*)(patch_location_ten) = 0xE8;
    *(uint32_t*)(patch_location_ten + 1) = offset_four;

    *(uint32_t*)(patch_location_fifthteen) = (uint32_t)DoorCycleResolve;
    *(uint32_t*)(patch_location_seventeen) = (uint32_t)DoorCycleResolve;

    memset((void*)patch_location_eighteen, 0x90, length_eighteen);
    offset = (uint32_t)FixTransitionCrash - patch_location_eighteen - 5;
    *(uint32_t*)(patch_location_eighteen) = 0xE8;
    *(uint32_t*)(patch_location_eighteen + 1) = offset;

    memset((void*)patch_location_nineteen, 0x90, length_nineteen);
    *(uint8_t*)(patch_location_nineteen) = 0x89;
    *(uint8_t*)(patch_location_nineteen + 1) = 0x34;
    *(uint8_t*)(patch_location_nineteen + 2) = 0x24;


    uint32_t offset_five = (uint32_t)VehicleRollermineCheck - patch_location_twenty - 5;
    *(uint8_t*)(patch_location_twenty) = 0xE8;
    *(uint32_t*)(patch_location_twenty + 1) = offset_five;
    *(uint16_t*)(patch_location_twenty + 5) = 0xC085;
    *(uint16_t*)(patch_location_twenty + 7) = 0x840F;
    *(uint32_t*)(patch_location_twenty + 9) = 0x2CD;

    memset((void*)patch_location_twentyone, 0x90, length_twentyone);

    /*uint32_t patch_save_system_one = server_srv + 0x004AED69;
    offset = (uint32_t)pTransitionEntsHookPtr - patch_save_system_one - 5;
    *(uint32_t*)(patch_save_system_one+1) = offset;*/

    uint32_t restore_patch_address_two = server_srv + 0x004AE788;
    memset((void*)restore_patch_address_two, 0x90, 0x28);
    *(uint8_t*)(restore_patch_address_two) = 0x89;
    *(uint8_t*)(restore_patch_address_two+1) = 0x1C;
    *(uint8_t*)(restore_patch_address_two+2) = 0x24;

    uint32_t restore_patch_address_three = server_srv + 0x4AE78B;
    offset = (uint32_t)RestoreSystemPatch - restore_patch_address_three - 5;
    *(uint8_t*)(restore_patch_address_three) = 0xE8;
    *(uint32_t*)(restore_patch_address_three+1) = offset;

    /*uint32_t restore_patch_address_four = server_srv + 0x00AF41EF;
    ChangeMemoryProtections(restore_patch_address_four, 2);
    memset((void*)restore_patch_address_four, 0x90, 2);
    RestoreMemoryProtections(restore_patch_address_four, 2);*/

    /*uint32_t restore_patch_address_five = server_srv + 0x00AF429B;
    ChangeMemoryProtections(restore_patch_address_five, 2);
    memset((void*)restore_patch_address_five, 0x90, 2);
    RestoreMemoryProtections(restore_patch_address_five, 2);*/

    uint32_t patch_another_cycle = server_srv + 0x00A95A9E;
    *(uint8_t*)(patch_another_cycle) = 0xEB;

    uint32_t yet_another_cycle = server_srv + 0x00A8653B;
    *(uint8_t*)(yet_another_cycle) = 0xEB;

    uint32_t fix_null_crash_patch = server_srv + 0x0075B9B8;
    offset = (uint32_t)FixNullCrash - fix_null_crash_patch - 5;
    *(uint32_t*)(fix_null_crash_patch+1) = offset;

    uint32_t scripted_sequence_patch_crash = server_srv + 0x00AC3F09;
    *(uint8_t*)(scripted_sequence_patch_crash) = 0xE9;
    *(uint32_t*)(scripted_sequence_patch_crash+1) = 0x70;

    //avoid causes high mem usage
    //0x00489F53
    //0x00489F17

    //0x003AEAB3
    //0x003AEA72

    //0x00A8441B
    //0x00A843C7

    //0x00486E83
    //0x00486E42

    //0x0039C673
    //0x0039C632

    //0x0060F783
    //0x0060F742

    //weird memory leak
    //0x004BBD43
    //0x004BBD02

    //0x005BEE23
    //0x005BEDEE

    //0x0083297B
    //0x00832904

    //Potential memory leak
    //0x00472723
    //0x004726E1

    rootconsole->ConsolePrint("--------------------- Other save system parts patched ---------------------");
}

uint32_t RestoreSystemPatch(uint32_t arg0)
{
    uint32_t clsname = *(uint32_t*)(arg0+0x20);
    //rootconsole->ConsolePrint("Restoring ent with clsname: [%s]", clsname);
    uint32_t newEnt = CreateEntityByName(clsname, (uint32_t)0xFFFFFFFF);
    return newEnt;
}

uint32_t RestoreSystemPatchStart(uint32_t arg0)
{
    // 1 = did not fail and jumps
    // 0 = failed and does not jump

    uint32_t chk_one = *(uint32_t*)(arg0+0x18);
    uint32_t chk_two = *(uint32_t*)(arg0+0x1C);
    uint8_t chk_three = *(uint8_t*)(arg0+0x1F);

    if(  !( (chk_one == 0) || ((chk_two & 0x40000000) != 0)  ) )
    {
        if((chk_three & 0x40) == 0)
        {
            pDynamicOneArgFunc = (pOneArgProt)(server_srv + 0x00AF29F0);
            uint8_t allowEntRestore = pDynamicOneArgFunc(*(uint32_t*)(arg0+0x20));

            if(allowEntRestore) return 1;
        }
    }
    
    return 0;
}

uint32_t SaveHookDirectMalloc(uint32_t size)
{
    uint32_t new_size = size*3.0;
    uint32_t ref = (uint32_t)malloc(new_size);
    memset((void*)ref, 0, new_size);
    rootconsole->ConsolePrint("malloc() [Save/Restore Hook] " HOOK_MSG " size: [%d]", ref, new_size);

    Value* leak = CreateNewValue((void*)ref);
    InsertToValuesList(leakedResourcesSaveRestoreSystem, leak, false, true, false);

    return ref;
}

uint32_t SaveHookDirectRealloc(uint32_t old_ptr, uint32_t new_size)
{
    new_size = new_size*3.0;
    uint32_t ref = (uint32_t)realloc((void*)old_ptr, new_size);
    rootconsole->ConsolePrint("realloc() [Save Hook] " HOOK_MSG " size: [%d]", ref, new_size);

    RemoveFromValuesList(leakedResourcesSaveRestoreSystem, (void*)old_ptr, false);

    Value* leak = CreateNewValue((void*)ref);
    InsertToValuesList(leakedResourcesSaveRestoreSystem, leak, false, true, false);

    return ref;
}

uint32_t EdtSystemHookFunc(uint32_t arg1)
{
    uint32_t ref = (uint32_t)malloc(arg1*3.0);
    rootconsole->ConsolePrint("[EDT Hook] " HOOK_MSG, ref);

    Value* leak = CreateNewValue((void*)ref);
    InsertToValuesList(leakedResourcesEdtSystem, leak, false, true, false);

    return ref;
}

uint32_t PreEdtLoad(uint32_t arg1, uint32_t arg2)
{
    uint32_t returnVal = pEdtLoadFunc(arg1, arg2);
    ReleaseLeakedMemory(leakedResourcesEdtSystem, false);
    return returnVal;
}

uint32_t SaveRestoreMemManage()
{
    ReleaseLeakedMemory(leakedResourcesSaveRestoreSystem, false);
    *(uint32_t*)((*(uint32_t*)SaveRestoreGlobal)+0x2C) = 0;
    return 0;
}

void CleanPlayerEnts(bool no_parent)
{
    //Cleanup old entities
    ValueList deleteList = AllocateValuesList();

    uint32_t oldEnt = 0;
    while((oldEnt = FindEntityByClassname(CGlobalEntityList, oldEnt, (uint32_t)"*")) != 0)
    {
        char* classname = (char*) ( *(uint32_t*)(oldEnt+0x68) );

        if(classname && ( strncmp(classname, "weapon_", 7) == 0 || strcmp(classname, "predicted_viewmodel") == 0 ) )
        {
            pDynamicOneArgFunc = (pOneArgProt)(*(uint32_t*)((*(uint32_t*)oldEnt)+0x30));
            uint32_t dmap = pDynamicOneArgFunc(oldEnt);

            uint32_t m_pParent = GetEntityField(dmap, oldEnt, 0, 0, (uint32_t)"m_pParent");

            if(m_pParent == 0)
                continue;

            uint32_t parent_object = GetCBaseEntity(*(int*)m_pParent);

            if(parent_object != 0)
            {
                uint32_t m_Network_Parent = *(uint32_t*)(parent_object+0x24);
                uint16_t ParentIndex = *(uint16_t*)(m_Network_Parent+0x6);

                uint32_t playerEnt = 0;
                while((playerEnt = FindEntityByClassname(CGlobalEntityList, playerEnt, (uint32_t)"player")) != 0)
                {
                    uint32_t m_Network_Player = *(uint32_t*)(playerEnt+0x24);
                    uint16_t playerIndex = *(uint16_t*)(m_Network_Player+0x6);
                    uint32_t playerRefHandle = *(uint32_t*)(playerEnt+0x350);

                    if(playerIndex == ParentIndex)
                    {
                        rootconsole->ConsolePrint("killed old entity [%s]", classname);
                        uint32_t refHandle = *(uint32_t*)(oldEnt+0x350);

                        Value* deleteEnt = CreateNewValue((void*)refHandle);
                        InsertToValuesList(deleteList, deleteEnt, false, true, false);
                        break;
                    }
                }
            }
            else if(no_parent)
            {
                rootconsole->ConsolePrint("killed entity without parent [%s]", classname);
                uint32_t refHandle = *(uint32_t*)(oldEnt+0x350);

                Value* deleteEnt = CreateNewValue((void*)refHandle);
                InsertToValuesList(deleteList, deleteEnt, false, true, false);
            }
        }
    }

    Value* delete_ent = *deleteList;

    while(delete_ent)
    {
        Value* detachedValue = delete_ent->nextVal;

        uint32_t refHandle = (uint32_t)delete_ent->value;
        uint32_t object = GetCBaseEntity(refHandle);
        if(object) Hooks::HookEntityDelete(object);

        free(delete_ent);
        delete_ent = detachedValue;
    }

    CleanupDeleteList(0);
    free(deleteList);
}

void RestorePlayers()
{
    if(playerSaveList == NULL)
        return;

    //Cleanup old entities
    CleanPlayerEnts(false);

    rootconsole->ConsolePrint("Restoring players...");
    
    uint32_t playerEnt = 0;
    while((playerEnt = FindEntityByClassname(CGlobalEntityList, playerEnt, (uint32_t)"player")) != 0)
    {
        uint32_t m_Network = *(uint32_t*)(playerEnt+0x24);
        uint16_t playerIndex = *(uint16_t*)(m_Network+0x6);
        uint32_t playerRefHandle = *(uint32_t*)(playerEnt+0x350);

        //Restore Player
        pDynamicThreeArgFunc = (pThreeArgProt)(server_srv + 0x00AF4110);
        uint8_t returnVal = pDynamicThreeArgFunc((*(uint32_t*)(server_srv + 0x00FA0CF0)), playerEnt, 1);

        if(returnVal == 0)
        {
            pDynamicThreeArgFunc = (pThreeArgProt)(server_srv + 0x00B01A90);
            pDynamicThreeArgFunc(playerEnt, 1, 1);

            pDynamicOneArgFunc = (pOneArgProt)(*(uint32_t*)((*(uint32_t*)(playerEnt))+0x5C));
            pDynamicOneArgFunc(playerEnt);
        }

        uint32_t global_one = *(uint32_t*)(server_srv + 0x01012420);
        global_one = *(uint32_t*)(global_one);

        pDynamicTwoArgFunc = (pTwoArgProt)(*(uint32_t*)(global_one+0x4C));
        uint32_t pEntity = pDynamicTwoArgFunc(*(uint32_t*)(server_srv + 0x01012420), playerIndex);

        pDynamicThreeArgFunc = (pThreeArgProt)(*(uint32_t*)(global_one+0x98));
        pDynamicThreeArgFunc(*(uint32_t*)(server_srv + 0x01012420), pEntity, (uint32_t)"__client_reset\n");

        if(returnVal) continue;

        

        /*PlayerSave* savedPlayer = *playerSaveList;

        while(savedPlayer)
        {
            SavedEntity* savedEntity = savedPlayer->saved_player;
            char* classname =  (char*) savedEntity->clsname;
            uint32_t entRefHandle = (uint32_t)savedEntity->refHandle;

            if(entRefHandle == playerRefHandle)
            {
                uint32_t object = GetCBaseEntity(entRefHandle);

                FieldList fields = savedEntity->fieldData;
                Field* field = *fields;

                while(field)
                {
                    char* fieldName = (char*) field->label;
                    char* externalName = (char*) field->key;
                    fieldtype_t fieldType = *(fieldtype_t*)field->type;
                    short flags = *(short*)field->flags;
                    int offset = *(int*)field->offset;

                    ValueList values = field->fieldVals;
                    Value* valueMember = *values;

                    if(fieldType == FIELD_VECTOR || fieldType == FIELD_POSITION_VECTOR)
                    {
                        rootconsole->ConsolePrint("         save-value: [%s] [%s] [%f] [%f] [%f]", fieldName, externalName
                                                                      , *(float*)   valueMember->value
                                                                      , *(float*)   valueMember->nextVal->value
                                                                      , *(float*)   valueMember->nextVal->nextVal->value);

                        *(float*)   (object+offset)     = *(float*)   valueMember->value;
                        *(float*)   (object+offset+0x4) = *(float*)   valueMember->nextVal->value;
                        *(float*)   (object+offset+0x8) = *(float*)   valueMember->nextVal->nextVal->value;
                    }

                    field = field->nextField;
                }

                break;
            }

            savedPlayer = savedPlayer->nextPlayer;
        }*/




        rootconsole->ConsolePrint("Failed to restore player giving weapons...");
        GivePlayerWeapons(playerEnt, true);
    }

    rootconsole->ConsolePrint("Finished restoring players!");
}

uint32_t RestoreOverride()
{
    CleanupDeleteList(0);
    pDynamicOneArgFunc = (pOneArgProt)(server_srv + 0x006863F0);
    pDynamicOneArgFunc(server_srv + 0x00FF3020);

    *(uint8_t*)((*(uint32_t*)(server_srv + 0x00FA0CF0)) + 0x130) = 1;

    uint32_t main_engine_global = *(uint32_t*)(server_srv + 0x00109A3E0);

    rootconsole->ConsolePrint("Clearing entities!");
    protect_player = true;

    pDynamicOneArgFunc = (pOneArgProt)(server_srv + 0x00ADDAC0);
    pDynamicOneArgFunc(0);

    *(uint8_t*)(server_srv + 0x00FF8740 + 0x10020) = 1;

    uint32_t mainEnt = 0;
    while((mainEnt = FindEntityByClassname(CGlobalEntityList, mainEnt, (uint32_t)"*")) != 0)
    {
        char* classname = (char*) ( *(uint32_t*)(mainEnt+0x68) );
        uint32_t refHandle = *(uint32_t*)(mainEnt+0x350);

        pDynamicOneArgFunc = (pOneArgProt)(server_srv + 0x00AF29F0);
        uint8_t allowEntRestore = pDynamicOneArgFunc((uint32_t)classname);

        if(allowEntRestore)
        {
            //call new kill sequence

            pDynamicOneArgFunc = (pOneArgProt)( *(uint32_t*) ( (*(uint32_t*)(mainEnt))+0x14 ) );
            uint32_t returnVal = pDynamicOneArgFunc(mainEnt);

            if(returnVal)
            {
                pDynamicOneArgFunc = (pOneArgProt)(  *(uint32_t*)( (*(uint32_t*)(main_engine_global))+0x64 )  );
                pDynamicOneArgFunc(main_engine_global);

                pDynamicOneArgFunc = (pOneArgProt)(server_srv + 0x00B64480);
                pDynamicOneArgFunc(returnVal);

                pDynamicOneArgFunc = (pOneArgProt)(  *(uint32_t*)( (*(uint32_t*)(main_engine_global))+0x68 )  );
                pDynamicOneArgFunc(main_engine_global);
            }
        }
    }

    CleanupDeleteList(0);

    pDynamicOneArgFunc = (pOneArgProt)(server_srv + 0x006863F0);
    pDynamicOneArgFunc(server_srv + 0x00FF3020);

    // SIM END

    *(uint32_t*)(server_srv + 0x0100890C) = 0;

    pDynamicOneArgFunc = (pOneArgProt)(server_srv + 0x006B52F0);
    pDynamicOneArgFunc(server_srv + 0x01008900);

    *(uint32_t*)(server_srv + 0x00F440C4) = 0xFFFFFFFF;

    uint32_t eax_val = *(uint32_t*)(server_srv + 0x01008900);
    *(uint32_t*)(server_srv + 0x01008910) = eax_val;
    
    *(uint8_t*)(server_srv + 0x00FF0990) = 0;

    *(uint8_t*)(server_srv + 0x00FF8740 + 0x10020) = 0;

    // -- END

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
    
    protect_player = false;
    restore_delay = true;
    return 0;
}

uint32_t TransitionArgUpdateHook(uint32_t arg0, uint32_t arg1)
{
    return strcmp((char*)((*(uint32_t*)(server_srv + 0x00FA0CF0)) + 0x131), (char*)arg1);
}

uint32_t TransitionArgUpdateHookTwo(uint32_t arg0, uint32_t arg1, uint32_t arg2)
{
    pDynamicThreeArgFunc = (pThreeArgProt)(server_srv + 0x00AF2370);
    return pDynamicThreeArgFunc(arg0, arg1, ((*(uint32_t*)(server_srv + 0x00FA0CF0)) + 0x131));
}

uint32_t TransitionArgUpdateHookThree()
{
    pDynamicOneArgFunc = (pOneArgProt)(server_srv + 0x00738250);
    return pDynamicOneArgFunc(0);
}

uint32_t Hooks::SavegameInternalFunction(uint32_t arg0)
{
    //SavePlayers();
    return pCallOrigSaveFunction(arg0);
}

uint32_t Hooks::ChkHandle(uint32_t arg0, uint32_t arg1)
{
    if(arg1 == 0)
    {
        pDynamicTwoArgFunc = (pTwoArgProt)(server_srv + 0x0064DD80);
        return pDynamicTwoArgFunc(arg0, arg1);
    }

    uint32_t m_RefEHandle = *(uint32_t*)(arg1+0x350);
    uint32_t object = GetCBaseEntity(m_RefEHandle);

    if(object != 0)
    {
        //call orig function
        pDynamicTwoArgFunc = (pTwoArgProt)(server_srv + 0x0064DD80);
        return pDynamicTwoArgFunc(arg0, object);
    }

    rootconsole->ConsolePrint("Invalid entity!!!");
    return 0;
}

uint32_t FixNullCrash(uint32_t arg0)
{
    if(arg0 == 0)
        return 0;

    pDynamicOneArgFunc = (pOneArgProt)(server_srv + 0x0075D540);
    return pDynamicOneArgFunc(arg0);
}

uint32_t TransitionEntsHook(uint32_t arg0, uint32_t arg1)
{
    uint32_t total_ents = *(uint32_t*)(arg0+0x574);
    uint32_t restore_buffer = *(uint32_t*)(arg0+0x578);

    uint32_t buffer_offset = 0;
    uint32_t ent_counter = 0;

    rootconsole->ConsolePrint("total_ents: [%d]", total_ents);

    while((int32_t)total_ents > (int32_t)ent_counter)
    {
        restore_buffer = *(uint32_t*)(arg0+0x578);
        restore_buffer = restore_buffer+buffer_offset;

        uint32_t _0x18_offset = *(uint32_t*)(restore_buffer+0x18);
        *(uint32_t*)(restore_buffer+0x10) = 0xFFFFFFFF;

        if(_0x18_offset)
        {
            uint32_t _0x4_offset = *(uint32_t*)(restore_buffer+0x4);

            if(_0x4_offset)
            {
                uint32_t clsname = *(uint32_t*)(restore_buffer+0x20);

                if(clsname)
                {
                    uint32_t _0x1C_check = *(uint32_t*)(restore_buffer+0x1C);

                    if((uint8_t)arg1 == (uint8_t)_0x1C_check)
                    {
                        if(strcmp((char*)clsname, "player") == 0)
                        {
                            ent_counter++;
                            buffer_offset = buffer_offset+0x3C;
                            continue;
                        }

                        uint32_t newEnt = CreateEntityByName(clsname, (uint32_t)0xFFFFFFFF);

                        if(newEnt)
                        {
                            uint32_t newEntDeref = *(uint32_t*)(newEnt);
                            pDynamicOneArgFunc = (pOneArgProt)(*(uint32_t*)(newEntDeref+0x0C));
                            uint32_t returnVal = pDynamicOneArgFunc(newEnt);

                            rootconsole->ConsolePrint(EXT_PREFIX "Transitioning [%s]", clsname);
                            *(uint32_t*)(restore_buffer+0x10) = *(uint32_t*)returnVal;
                        }
                    }
                }
            }
        }

        ent_counter++;
        buffer_offset = buffer_offset+0x3C;
    }

    return 0;
}

uint32_t MainTransitionRestore(uint32_t arg1, uint32_t arg2)
{
    //uint32_t ents = *(uint32_t*)(SaveGameBuffer+0x574);

    //*(uint32_t*)(arg1+0x574) = *(uint32_t*)(SaveGameBuffer+0x574);
    //*(uint32_t*)(arg1+0x578) = *(uint32_t*)(SaveGameBuffer+0x578);
    
    //uint32_t less = ents / 1.40;
    //rootconsole->ConsolePrint("SAVEGAMEBUFFER: Restore ents: [%d]", ents);
    //*(uint32_t*)(arg1+0x574) = less;
    return MainTransitionCall(arg1, arg2);
    //return 0;
}

uint32_t TransitionEntityCreateCall(uint32_t arg1, uint32_t arg2)
{
    /*if(strcasestr((char*)arg1, "env_") != NULL)
    {
        rootconsole->ConsolePrint("Prevented entity transition: [%s]", arg1);
        return 0;
    }*/

    rootconsole->ConsolePrint(EXT_PREFIX "Restoring %s", arg1);

    uint32_t object = (uint32_t)CreateEntityByName(arg1, arg2);
    uint32_t ref = *(uint32_t*)(object+0x350);

    /*Value* savedEntity = CreateNewValue((void*)ref);
    InsertToValuesList(entityList, savedEntity);*/

    return object;
}

uint32_t TransitionRestoreMain(uint32_t arg1, uint32_t arg2, uint32_t arg3, uint32_t arg4)
{
    // INIT LEVEL

    //BEGIN RESTORE ENTITIES
    pDynamicOneArgFunc = (pOneArgProt)(server_srv + 0x0073B880);
    pDynamicOneArgFunc(0);

    *(uint8_t*)(server_srv + 0x01012150) = 1;

    pDynamicOneArgFunc = (pOneArgProt)(server_srv + 0x0073D110);
    pDynamicOneArgFunc(server_srv + 0x01012430);

    pDynamicOneArgFunc = (pOneArgProt)(server_srv + 0x0073D190);
    pDynamicOneArgFunc(server_srv + 0x01012430);

    *(uint16_t*)(server_srv + 0x01012440) = (uint16_t)0xFFFFFFFF;
    *(uint16_t*)(server_srv + 0x01012444) = 0;
    *(uint32_t*)(server_srv + 0x01012448) = *(uint32_t*)(server_srv + 0x01012430);
    *(uint16_t*)(server_srv + 0x01012446) = (uint16_t)0xFFFFFFFF;



    //pDynamicOneArgFunc = (pOneArgProt)(engine_srv + 0x000E0A50);
    //uint32_t mapEnts = pDynamicOneArgFunc(0);
    //rootconsole->ConsolePrint("MAP ENTS HEAP AREA: [%X]", mapEnts);
    uint32_t arg4_internal = server_srv + 0x00CACDF0;

    //CALL MAIN LEVEL PARSE FUNC

    pDynamicThreeArgFunc = (pThreeArgProt)(server_srv + 0x009B09F0);
    pDynamicThreeArgFunc(global_map_ents, (uint32_t)&arg4_internal, 0);
    global_map_ents = 0;

    transition = true;

    /*if(strcmp(last_map, "ep2_outland_04") == 0 && strcmp(global_map, "ep2_outland_02") == 0)
    {
        uint32_t searchEnt = 0;

        while((searchEnt = FindEntityByClassname(CGlobalEntityList, searchEnt, (uint32_t)"logic_relay")) != 0)
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

        while((searchEnt = FindEntityByClassname(CGlobalEntityList, searchEnt, (uint32_t)"trigger_once")) != 0)
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

        while((searchEnt = FindEntityByClassname(CGlobalEntityList, searchEnt, (uint32_t)"point_template")) != 0)
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

        while((searchEnt = FindEntityByClassname(CGlobalEntityList, searchEnt, (uint32_t)"logic_relay")) != 0)
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
    
    uint32_t returnVal = pTransitionRestoreMainCall(arg1, arg2, arg3, arg4);
    SaveRestoreMemManage();

    pDynamicOneArgFunc = (pOneArgProt)(server_srv + 0x0073CBD0);
    pDynamicOneArgFunc(0);
    return returnVal;
}

uint32_t VehicleRollermineCheck(uint32_t arg1)
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

uint32_t SV_TriggerMovedFix(uint32_t arg1, uint32_t arg2)
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
        pDynamicOneArgFunc = (pOneArgProt)(*(uint32_t*)(edx_deref+0x10));
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

uint32_t DoorCycleResolve(uint32_t arg1, uint32_t arg2, uint32_t arg3, uint32_t arg4, uint32_t arg5)
{
    uint32_t something = *(uint32_t*)(server_srv + 0x00F4BA30);
    uint32_t mainValue = *(uint32_t*)(arg1+0x6E0);

    if(mainValue != 0xFFFFFFFF)
    {
        if(IsInValuesList(antiCycleListDoors, (void*)mainValue, false))
        {
            rootconsole->ConsolePrint("Cycle was detected and prevented hang!");
            DeleteAllValuesInList(antiCycleListDoors, false, false);
            return pDoorFinalFunction(arg1, arg2, arg3, arg4, arg5);
        }

        //Save ref for cycle resolving
        Value* val_save = CreateNewValue((void*)mainValue);
        InsertToValuesList(antiCycleListDoors, val_save, false, true, false);

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

    DeleteAllValuesInList(antiCycleListDoors, false, false);
    return pDoorFinalFunction(arg1, arg2, arg3, arg4, arg5);
}

uint32_t CreateEntityByNameHook(uint32_t arg0, uint32_t arg1)
{
    if(strcmp((char*)arg1, "prop_vehicle_mp") == 0)
    {
        return CreateEntityCallFunc(arg0, (uint32_t)"prop_vehicle_jeep");
    }

    return CreateEntityCallFunc(arg0, arg1);
}

uint32_t FixManhackCrash(uint32_t arg0)
{
    uint32_t returnVal = OrigManhackFunc(arg0);
    uint32_t chk = *(uint32_t*)(arg0+0x1FC);

    if(chk > (uint32_t)0xFFFF)
    {
        uint32_t chk_deref = *(uint32_t*)(chk);
        pDynamicOneArgFunc = (pOneArgProt)(*(uint32_t*)(chk_deref+0x64));
        pDynamicOneArgFunc(chk);
    }

    return returnVal;
}

uint32_t FixTransitionCrash(uint32_t arg0, uint32_t arg1, uint32_t arg2)
{
    if(arg0 > (uint32_t)0xFFFF)
    {
        uint32_t arg0_deref = *(uint32_t*)(arg0);
        pDynamicThreeArgFunc = (pThreeArgProt)(*(uint32_t*)(arg0_deref+0x0D0));
        return pDynamicThreeArgFunc(arg0, arg1, arg2);
    }

    return 0;
}

uint32_t Hooks::LevelChangeSafeHook(uint32_t arg0)
{
    //Flush - data cache
    uint32_t freed_bytes = pFlushFunc((uint32_t)g_DataCache, (uint32_t)true, (uint32_t)false);
    rootconsole->ConsolePrint("Freed [%d] bytes from cache!", freed_bytes);

    pDynamicOneArgFunc = (pOneArgProt)(server_srv + 0x004CCA80);
    return pDynamicOneArgFunc(arg0);
}

uint32_t Hooks::PlayerLoadHook(uint32_t arg0)
{
    uint32_t returnVal = PlayerLoadOrig(arg0);
    //rootconsole->ConsolePrint("called main new player join func!");

    uint32_t m_Network = *(uint32_t*)(arg0+0x24);
    uint16_t playerIndex = *(uint16_t*)(m_Network+0x6);

    uint32_t global_one = *(uint32_t*)(server_srv + 0x01012420);
    global_one = *(uint32_t*)(global_one);

    pDynamicTwoArgFunc = (pTwoArgProt)(*(uint32_t*)(global_one+0x4C));
    uint32_t pEntity = pDynamicTwoArgFunc(*(uint32_t*)(server_srv + 0x01012420), playerIndex);

    pDynamicThreeArgFunc = (pThreeArgProt)(*(uint32_t*)(global_one+0x98));
    pDynamicThreeArgFunc(*(uint32_t*)(server_srv + 0x01012420), pEntity, (uint32_t)"r_flushlod\n");

    return returnVal;
}

uint32_t Hooks::PlayerSpawnHook(uint32_t arg0, uint32_t arg1, uint32_t arg2)
{
    rootconsole->ConsolePrint("called the main spawn info sender!");
    pDynamicThreeArgFunc = (pThreeArgProt)(server_srv + 0x00B01A90);
    return pDynamicThreeArgFunc(arg0, 1, 1);
}

uint32_t Hooks::PlayerSpawnDirectHook(uint32_t arg0)
{
    rootconsole->ConsolePrint("[Main] Called the main player spawn func!");

    pDynamicThreeArgFunc = (pThreeArgProt)(server_srv + 0x00B01A90);
    pDynamicThreeArgFunc(arg0, 1, 1);

    pDynamicOneArgFunc = (pOneArgProt)(server_srv + 0x009924D0);
    uint32_t returnVal = pDynamicOneArgFunc(arg0);

    GivePlayerWeapons(arg0, false);
    return returnVal;
}

uint32_t SavegameInitialLoad(uint32_t arg0, uint32_t arg1)
{
    //get transition file name
    uint32_t savegame_name = (*(uint32_t*)(server_srv + 0x00FA0CF0)) + 0x131;
    rootconsole->ConsolePrint("Loading transition file %s", savegame_name);
    return pRestoreFileCallFunc(arg0, savegame_name);
}

uint32_t Hooks::UnmountPaths(uint32_t arg0)
{
    //CleanupDeleteList(0);

    //rootconsole->ConsolePrint("Flushing model cache!");
    //UnloadUnreferencedModels(g_ModelLoader);

    //scene_flush direct call
    //pDynamicOneArgFunc = (pOneArgProt)(server_srv + 0x00AAA840);
    //pDynamicOneArgFunc(0);

    //CleanupDeleteList(0);

    //Flush - mdl cache
    //pDynamicTwoArgFunc = (pTwoArgProt)(datacache_srv + 0x000381D0);
    //pDynamicTwoArgFunc(datacache_srv + 0x00075140, (uint32_t)MDLCACHE_FLUSH_ALL);

    //UnloadAllModels
    pDynamicTwoArgFunc = (pTwoArgProt)(engine_srv + 0x0014D480);
    pDynamicTwoArgFunc(engine_srv + 0x00317380, 0);

    //Flush - data cache
    //uint32_t freed_bytes = pFlushFunc((uint32_t)g_DataCache, (uint32_t)true, (uint32_t)false);
    //rootconsole->ConsolePrint("Freed [%d] bytes from cache!", freed_bytes);
    
    pDynamicOneArgFunc = (pOneArgProt)(server_srv + 0x004C5CA0);
    return pDynamicOneArgFunc(arg0);
}

uint32_t Hooks::LevelInitHook(uint32_t arg0, uint32_t arg1, uint32_t arg2)
{
    //SAVE REFERENCE BECAUSE ITS NEEDED FOR EDTREBUILD arg2
    global_map_ents = arg2;
    rootconsole->ConsolePrint("####Updated global map ents!#####");
    return 0;
}

uint32_t Hooks::BarneyThinkHook(uint32_t arg0, uint32_t arg1, uint32_t arg2)
{
    uint32_t deref_arg4 = *(uint32_t*)(arg1);

    if(deref_arg4)
    {
        uint32_t eHandle = *(uint32_t*)(deref_arg4+0x4);

        if(eHandle != 0xFFFFFFFF)
        {
            uint32_t object = GetCBaseEntity(eHandle);

            if(object)
            {
                //Call orig func
                pDynamicThreeArgFunc = (pThreeArgProt)(server_srv + 0x0057D930);
                return pDynamicThreeArgFunc(arg0, arg1, arg2);
            }
        }
    }

    rootconsole->ConsolePrint("Failed to obtain object!");
    return 0;
}

uint32_t Hooks::HunterCrashFix(uint32_t arg0)
{
    if(FindEntityByClassname(CGlobalEntityList, 0, (uint32_t)"player") != 0)
    {
        //Call orig func
        pDynamicOneArgFunc = (pOneArgProt)(server_srv + 0x006F6910);
        return pDynamicOneArgFunc(arg0);
    }

    rootconsole->ConsolePrint("Player did not exist!");
    return 0;
}

uint32_t Hooks::NET_BufferToBufferDecompress_Patch(uint32_t arg0, uint32_t arg1, uint32_t arg2, uint32_t arg3)
{
    return 0;
}

uint32_t Hooks::TransitionFixTheSecond(uint32_t arg0)
{
    if(arg0)
    {
        uint32_t eax = *(uint32_t*)(arg0+0x150);

        if(eax != 0xFFFFFFFF)
        {
            uint32_t object = GetCBaseEntity(eax);

            if(object)
            {
                uint32_t idk = *(uint32_t*)(object+0x1FC);

                if(idk)
                {
                    //rootconsole->ConsolePrint("Worked i guess!\n\n\n\n");
                    pDynamicOneArgFunc = (pOneArgProt)(server_srv + 0x005A8680);
                    return pDynamicOneArgFunc(arg0);
                }
            }
        }
    }

    return 0;
}

uint32_t Hooks::PatchAnotherPlayerAccessCrash(uint32_t arg0)
{
    uint32_t npc_combine_s = *(uint32_t*)(arg0+0x4);

    if(npc_combine_s)
    {
        pDynamicOneArgFunc = (pOneArgProt)(  *(uint32_t*)((*(uint32_t*)(npc_combine_s))+0x180)    );
        uint32_t returnVal = pDynamicOneArgFunc(npc_combine_s); 

        if(returnVal)
        {
            //Call orig
            pDynamicOneArgFunc = (pOneArgProt)(server_srv + 0x0058FBD0);
            return pDynamicOneArgFunc(arg0);
        }
    }

    rootconsole->ConsolePrint("nah");
    return 0;
}

uint32_t Hooks::PlayerloadSavedHook(uint32_t arg0, uint32_t arg1)
{
    if(savegame || restoring) return 0;
    restoring = true;
    pDynamicOneArgFunc = (pOneArgProt)(server_srv + 0x00A4B8C0);
    uint32_t returnVal = pDynamicOneArgFunc(arg0);
    return returnVal;
}

void AllowWriteToMappedMemory()
{
    for(int i = 0; i < 512; i++)
    {
        memory_prots_save_list[i] = 0;
    }

    FILE* smaps_file = fopen("/proc/self/smaps", "r");    

    if(!smaps_file)
    {
        rootconsole->ConsolePrint("Error opening smaps");
        return;
    }

    char* file_line = (char*) malloc(sizeof(char) * 1024);

    while(fgets(file_line, 1024, smaps_file))
    {
        sscanf(file_line, "%[^\n]s", file_line);

        if(strcasestr(file_line, "/Synergy/bin/") == NULL && strcasestr(file_line, "/Synergy/synergy/bin/") == NULL)
            continue;

        char* file_line_cpy = (char*) malloc(strlen(file_line)+1);
        snprintf(file_line_cpy, strlen(file_line)+1, "%s", file_line);

        char* address_range = strtok(file_line_cpy, " \t");
        char* protections = strtok(NULL, " \t");

        char* start_address = strtok(address_range, "-");
        char* end_address = strtok(NULL, "-");

        uint32_t start_address_parsed = 0;
        uint32_t end_address_parsed = 0;

        if(start_address) start_address_parsed = strtoul(start_address, NULL, 16);
        if(end_address) end_address_parsed = strtoul(end_address, NULL, 16);

        int save_protections = PROT_NONE;

        if(strstr(protections, "r") != 0)
            save_protections = PROT_READ;
        if(strstr(protections, "w") != 0)
            save_protections = save_protections | PROT_WRITE;
        if(strstr(protections, "x") != 0)
            save_protections = save_protections | PROT_EXEC;

        for(int i = 0; i < 512 && i+1 < 512 && i+2 < 512; i = i+3)
        {
            if(start_address_parsed && end_address_parsed &&
            memory_prots_save_list[i] == 0 && memory_prots_save_list[i+1] == 0 && memory_prots_save_list[i+2] == 0)
            {
                memory_prots_save_list[i] = start_address_parsed;
                memory_prots_save_list[i+1] = end_address_parsed;
                memory_prots_save_list[i+2] = (uint32_t)save_protections;
                break;
            }
        }

        if(start_address_parsed && end_address_parsed)
        {
            //rootconsole->ConsolePrint("%s", file_line);

            //Force write permissions
            size_t pagesize = sysconf(_SC_PAGE_SIZE);
            uint32_t pagestart = start_address_parsed & -pagesize;

            if(mprotect((void*)pagestart, end_address_parsed - start_address_parsed, PROT_READ | PROT_WRITE | PROT_EXEC) == -1)
            {
                perror("mprotect");
                exit(EXIT_FAILURE);
            }
        }

        free(file_line_cpy);
    }

    free(file_line);
    fclose(smaps_file);
}

void RestoreMemoryProtections()
{
    for(int i = 0; i < 512 && i+1 < 512 && i+2 < 512; i = i+3)
    {
        size_t pagesize = sysconf(_SC_PAGE_SIZE);
        uint32_t pagestart = memory_prots_save_list[i] & -pagesize;

        if(mprotect((void*)pagestart, memory_prots_save_list[i+1] - memory_prots_save_list[i], memory_prots_save_list[i+2]) == -1)
        {
            perror("mprotect");
            exit(EXIT_FAILURE);
        }

        memory_prots_save_list[i] = 0;
        memory_prots_save_list[i+1] = 0;
        memory_prots_save_list[i+2] = 0;
    }
}

void HookFunctionInSharedObject(uint32_t base_address, uint32_t size, void* target_pointer, void* hook_pointer)
{
    uint32_t search_address = base_address;
    uint32_t search_address_max = base_address+size;

    while(search_address <= search_address_max)
    {
        uint32_t four_byte_addr = *(uint32_t*)(search_address);

        if(four_byte_addr == (uint32_t)target_pointer)
        {
            rootconsole->ConsolePrint("Patched abs address: [%X]", search_address);
            *(uint32_t*)(search_address) = (uint32_t)hook_pointer;
            
            search_address++;
            continue;
        }

        uint8_t byte = *(uint8_t*)(search_address);

        if(byte == 0xE8 || byte == 0xE9)
        {
            uint32_t call_address = *(uint32_t*)(search_address + 1);
            uint32_t chk = search_address + call_address + 5;

            if(chk == (uint32_t)target_pointer)
            {
                bool skip = false;

                for(int i = 0; i < 512; i++)
                {
                    if(hook_exclude_list[i] == 0)
                        continue;

                    uint32_t patch_address = base_address + hook_exclude_list[i];

                    if(patch_address == search_address)
                    {
                        skip = true;
                        break;
                    }
                }

                if(skip)
                {
                    rootconsole->ConsolePrint("(unsigned) Skipped patch at [%X]", search_address);
                    search_address++;
                    continue;
                }

                rootconsole->ConsolePrint("(unsigned) Hooked address: [%X]", search_address - base_address);
                uint32_t offset = (uint32_t)hook_pointer - search_address - 5;
                *(uint32_t*)(search_address+1) = offset;
            }
            else
            {
                //check signed addition
                chk = search_address + (int32_t)call_address + 5;

                if(chk == (uint32_t)target_pointer)
                {
                    bool skip = false;

                    for(int i = 0; i < 512; i++)
                    {
                        if(hook_exclude_list[i] == 0)
                            continue;

                        uint32_t patch_address = base_address + hook_exclude_list[i];

                        if(patch_address == search_address)
                        {
                            skip = true;
                            break;
                        }
                    }

                    if(skip)
                    {
                        rootconsole->ConsolePrint("(signed) Skipped patch at [%X]", search_address);
                        search_address++;
                        continue;
                    }

                    rootconsole->ConsolePrint("(signed) Hooked address: [%X]", search_address - base_address);
                    uint32_t offset = (uint32_t)hook_pointer - search_address - 5;
                    *(uint32_t*)(search_address+1) = offset;
                }
            }
        }

        search_address++;
    }
}

uint32_t SpawnServerHookFunc(uint32_t arg1, uint32_t arg2, uint32_t arg3)
{
    snprintf(last_map, 1024, "%s", global_map);
    snprintf(global_map, 1024, "%s", (char*)arg2);

    CleanupDeleteList(0);
    uint32_t returnVal = pSpawnServerFunc(arg1, arg2, arg3);
    CleanupDeleteList(0);

    return returnVal;
}

uint32_t HostChangelevelHook(uint32_t arg1, uint32_t arg2, uint32_t arg3)
{
    transition = false;
    
    CleanupDeleteList(0);
    *(uint8_t*)(*(uint32_t*)(server_srv + 0x00FA0CF0)) = 0;
    uint32_t main_engine_global = *(uint32_t*)(server_srv + 0x00109A3E0);
    uint32_t returnVal = pHostChangelevelFunc(arg1, arg2, arg3);

    CleanupDeleteList(0);

    /*if(strcmp(global_map, "d3_c17_10a") == 0)
    {
        uint32_t searchEnt = FindEntityByClassname(CGlobalEntityList, 0, (uint32_t)"npc_barney");

        if(searchEnt != 0)
        {
            uint32_t m_refHandle = *(uint32_t*)(searchEnt+0x350);
            SendEntityInput(m_refHandle, (uint32_t)"Kill", 0, 0, 0, (uint32_t)-1);
        }
        
        uint32_t newEntity = CreateEntityByName((uint32_t)"npc_barney", (uint32_t)-1);
        pDynamicThreeArgFunc = (pThreeArgProt)(*(uint32_t*)((*(uint32_t*)newEntity)+0x78));
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

        while((searchEnt = FindEntityByClassname(CGlobalEntityList, searchEnt, (uint32_t)"prop_door_rotating")) != 0)
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
        pDynamicThreeArgFunc = (pThreeArgProt)(*(uint32_t*)((*(uint32_t*)newEntity)+0x78));
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
    savegame = true;
    gamestart = true;
    return returnVal;
}

uint32_t DropshipsHook(uint32_t arg1, uint32_t arg2, uint32_t arg3)
{
    uint32_t something = *(uint32_t*)(server_srv + 0x00F4BA30);
    uint32_t mainValue = *(uint32_t*)(((uint32_t)arg1)+0x1024);

    if(mainValue != 0xFFFFFFFF)
    {
        uint32_t aCheck = mainValue >> 0x0C;

        mainValue = mainValue & 0xFFF;
        mainValue = mainValue << 0x4;
        mainValue = mainValue + something;

        uint32_t lea = mainValue+0x4;
        uint32_t val = *(uint32_t*)(lea+0x4);
        if( val == aCheck )
        {
            uint32_t instance = *(uint32_t*)(mainValue+0x4);
            uint32_t eax = *(uint32_t*)(instance+0x4A4);

            if(eax == 0)
            {
                uint32_t returnVal = (uint32_t)sub_654260((uint32_t)instance);

                if(returnVal != 0)
                    sub_628F00((uint32_t)instance);

                eax = *(uint32_t*)(instance+0x4A4);

                if(eax != 0)
                {
                    uint32_t deref = *(uint32_t*)eax;

                    if(deref == 0)
                        eax = 0;
                }
            }

            rootconsole->ConsolePrint("everything worked in dropship hook!");
            return PopulatePoseParameters(arg1, eax, arg3);
        }
    }

    rootconsole->ConsolePrint("failed to find m_hContainer");
    return PopulatePoseParameters(arg1, arg2, arg3);
}

uint32_t CallLater(uint32_t arg1, uint32_t arg2, uint32_t arg3)
{
    OldFunction(arg1);
    return EndFunction(arg1, arg2, arg3);
}

void HookFunctionsWithC()
{
    rootconsole->ConsolePrint("patching calloc()");
    HookFunctionInSharedObject(server_srv, server_srv_size, (void*)calloc, (void*)CallocHook);
    HookFunctionInSharedObject(engine_srv, engine_srv_size, (void*)calloc, (void*)CallocHook);
    HookFunctionInSharedObject(datacache_srv, datacache_srv_size, (void*)calloc, (void*)CallocHook);
    HookFunctionInSharedObject(dedicated_srv, dedicated_srv_size, (void*)calloc, (void*)CallocHook);
    HookFunctionInSharedObject(materialsystem_srv, materialsystem_srv_size, (void*)calloc, (void*)CallocHook);
    HookFunctionInSharedObject(vphysics_srv, vphysics_srv_size, (void*)calloc, (void*)CallocHook);
    HookFunctionInSharedObject(scenefilecache, scenefilecache_size, (void*)calloc, (void*)CallocHook);
    HookFunctionInSharedObject(soundemittersystem, soundemittersystem_size, (void*)calloc, (void*)CallocHook);
    HookFunctionInSharedObject(soundemittersystem_srv, soundemittersystem_srv_size, (void*)calloc, (void*)CallocHook);
    HookFunctionInSharedObject(studiorender_srv, studiorender_srv_size, (void*)calloc, (void*)CallocHook);
    rootconsole->ConsolePrint("patching malloc()");
    HookFunctionInSharedObject(server_srv, server_srv_size, (void*)malloc, (void*)MallocHook);
    HookFunctionInSharedObject(engine_srv, engine_srv_size, (void*)malloc, (void*)MallocHook);
    HookFunctionInSharedObject(datacache_srv, datacache_srv_size, (void*)malloc, (void*)MallocHook);
    HookFunctionInSharedObject(dedicated_srv, dedicated_srv_size, (void*)malloc, (void*)MallocHook);
    HookFunctionInSharedObject(materialsystem_srv, materialsystem_srv_size, (void*)malloc, (void*)MallocHook);
    HookFunctionInSharedObject(vphysics_srv, vphysics_srv_size, (void*)malloc, (void*)MallocHook);
    HookFunctionInSharedObject(scenefilecache, scenefilecache_size, (void*)malloc, (void*)MallocHook);
    HookFunctionInSharedObject(soundemittersystem, soundemittersystem_size, (void*)malloc, (void*)MallocHook);
    HookFunctionInSharedObject(soundemittersystem_srv, soundemittersystem_srv_size, (void*)malloc, (void*)MallocHook);
    HookFunctionInSharedObject(studiorender_srv, studiorender_srv_size, (void*)malloc, (void*)MallocHook);
    rootconsole->ConsolePrint("patching realloc()");
    HookFunctionInSharedObject(server_srv, server_srv_size, (void*)realloc, (void*)ReallocHook);
    HookFunctionInSharedObject(engine_srv, engine_srv_size, (void*)realloc, (void*)ReallocHook);
    HookFunctionInSharedObject(datacache_srv, datacache_srv_size, (void*)realloc, (void*)ReallocHook);
    HookFunctionInSharedObject(dedicated_srv, dedicated_srv_size, (void*)realloc, (void*)ReallocHook);
    HookFunctionInSharedObject(materialsystem_srv, materialsystem_srv_size, (void*)realloc, (void*)ReallocHook);
    HookFunctionInSharedObject(vphysics_srv, vphysics_srv_size, (void*)realloc, (void*)ReallocHook);
    HookFunctionInSharedObject(scenefilecache, scenefilecache_size, (void*)realloc, (void*)ReallocHook);
    HookFunctionInSharedObject(soundemittersystem, soundemittersystem_size, (void*)realloc, (void*)ReallocHook);
    HookFunctionInSharedObject(soundemittersystem_srv, soundemittersystem_srv_size, (void*)realloc, (void*)ReallocHook);
    HookFunctionInSharedObject(studiorender_srv, studiorender_srv_size, (void*)realloc, (void*)ReallocHook);
    /*rootconsole->ConsolePrint("patching free()");
    HookFunctionInSharedObject(server_srv, server_srv_size, pFreePtr, pFreeHookPtr);
    HookFunctionInSharedObject(engine_srv, engine_srv_size, pFreePtr, pFreeHookPtr);
    HookFunctionInSharedObject(datacache_srv, datacache_srv_size, pFreePtr, pFreeHookPtr);
    HookFunctionInSharedObject(dedicated_srv, dedicated_srv_size, pFreePtr, pFreeHookPtr);
    HookFunctionInSharedObject(materialsystem_srv, materialsystem_srv_size, pFreePtr, pFreeHookPtr);
    HookFunctionInSharedObject(vphysics_srv, vphysics_srv_size, pFreePtr, pFreeHookPtr);
    HookFunctionInSharedObject(scenefilecache, scenefilecache_size, pFreePtr, pFreeHookPtr);
    HookFunctionInSharedObject(soundemittersystem, soundemittersystem_size, pFreePtr, pFreeHookPtr);
    HookFunctionInSharedObject(soundemittersystem_srv, soundemittersystem_srv_size, pFreePtr, pFreeHookPtr);
    HookFunctionInSharedObject(studiorender_srv, studiorender_srv_size, pFreePtr, pFreeHookPtr);*/

    
    rootconsole->ConsolePrint("patching operator new()");
    HookFunctionInSharedObject(server_srv, server_srv_size, new_operator_addr, (void*)OperatorNewHook);
    HookFunctionInSharedObject(engine_srv, engine_srv_size, new_operator_addr, (void*)OperatorNewHook);
    HookFunctionInSharedObject(datacache_srv, datacache_srv_size, new_operator_addr, (void*)OperatorNewHook);
    HookFunctionInSharedObject(dedicated_srv, dedicated_srv_size, new_operator_addr, (void*)OperatorNewHook);
    HookFunctionInSharedObject(materialsystem_srv, materialsystem_srv_size, new_operator_addr, (void*)OperatorNewHook);
    HookFunctionInSharedObject(vphysics_srv, vphysics_srv_size, new_operator_addr, (void*)OperatorNewHook);
    HookFunctionInSharedObject(scenefilecache, scenefilecache_size, new_operator_addr, (void*)OperatorNewHook);
    HookFunctionInSharedObject(soundemittersystem, soundemittersystem_size, new_operator_addr, (void*)OperatorNewHook);
    HookFunctionInSharedObject(soundemittersystem_srv, soundemittersystem_srv_size, new_operator_addr, (void*)OperatorNewHook);
    HookFunctionInSharedObject(studiorender_srv, studiorender_srv_size, new_operator_addr, (void*)OperatorNewHook);
    rootconsole->ConsolePrint("patching operator new[]()");
    HookFunctionInSharedObject(server_srv, server_srv_size, new_operator_array_addr, (void*)OperatorNewArrayHook);
    HookFunctionInSharedObject(engine_srv, engine_srv_size, new_operator_array_addr, (void*)OperatorNewArrayHook);
    HookFunctionInSharedObject(datacache_srv, datacache_srv_size, new_operator_array_addr, (void*)OperatorNewArrayHook);
    HookFunctionInSharedObject(dedicated_srv, dedicated_srv_size, new_operator_array_addr, (void*)OperatorNewArrayHook);
    HookFunctionInSharedObject(materialsystem_srv, materialsystem_srv_size, new_operator_array_addr, (void*)OperatorNewArrayHook);
    HookFunctionInSharedObject(vphysics_srv, vphysics_srv_size, new_operator_array_addr, (void*)OperatorNewArrayHook);
    HookFunctionInSharedObject(scenefilecache, scenefilecache_size, new_operator_array_addr, (void*)OperatorNewArrayHook);
    HookFunctionInSharedObject(soundemittersystem, soundemittersystem_size, new_operator_array_addr, (void*)OperatorNewArrayHook);
    HookFunctionInSharedObject(soundemittersystem_srv, soundemittersystem_srv_size, new_operator_array_addr, (void*)OperatorNewArrayHook);
    HookFunctionInSharedObject(studiorender_srv, studiorender_srv_size, new_operator_array_addr, (void*)OperatorNewArrayHook);
    rootconsole->ConsolePrint("patching operator delete");
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
    HookFunctionInSharedObject(studiorender_srv, studiorender_srv_size, delete_operator_array_addr, (void*)DeleteOperatorArrayHook);

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
}

void HookFunctionsWithCpp()
{
    HookFunctionInSharedObject(server_srv, server_srv_size, (void*)(server_srv + 0x004C5CA0), g_SynUtils.getCppAddr(Hooks::UnmountPaths));
    HookFunctionInSharedObject(server_srv, server_srv_size, (void*)(server_srv + 0x00A4B8C0), g_SynUtils.getCppAddr(Hooks::PlayerloadSavedHook));
    HookFunctionInSharedObject(server_srv, server_srv_size, (void*)(server_srv + 0x00AEFDB0), g_SynUtils.getCppAddr(Hooks::LevelInitHook));
    HookFunctionInSharedObject(server_srv, server_srv_size, (void*)(server_srv + 0x005A8680), g_SynUtils.getCppAddr(Hooks::TransitionFixTheSecond));
    HookFunctionInSharedObject(server_srv, server_srv_size, (void*)(server_srv + 0x0058FBD0), g_SynUtils.getCppAddr(Hooks::PatchAnotherPlayerAccessCrash));
    HookFunctionInSharedObject(server_srv, server_srv_size, (void*)(server_srv + 0x009924D0), g_SynUtils.getCppAddr(Hooks::PlayerSpawnDirectHook));
    HookFunctionInSharedObject(server_srv, server_srv_size, (void*)(server_srv + 0x00B64500), g_SynUtils.getCppAddr(Hooks::HookEntityDelete));
    HookFunctionInSharedObject(server_srv, server_srv_size, (void*)(server_srv + 0x00AF3990), g_SynUtils.getCppAddr(Hooks::SaveOverride));
    HookFunctionInSharedObject(server_srv, server_srv_size, (void*)(server_srv + 0x00B01A90), g_SynUtils.getCppAddr(Hooks::PlayerSpawnHook));
    HookFunctionInSharedObject(server_srv, server_srv_size, (void*)(server_srv + 0x00B02DB0), g_SynUtils.getCppAddr(Hooks::PlayerLoadHook));
    HookFunctionInSharedObject(server_srv, server_srv_size, (void*)(server_srv + 0x00AF33F0), g_SynUtils.getCppAddr(Hooks::SavegameInternalFunction));
    HookFunctionInSharedObject(server_srv, server_srv_size, (void*)(server_srv + 0x0064DD80), g_SynUtils.getCppAddr(Hooks::ChkHandle));
    HookFunctionInSharedObject(server_srv, server_srv_size, (void*)(server_srv + 0x0057D930), g_SynUtils.getCppAddr(Hooks::BarneyThinkHook));
    HookFunctionInSharedObject(server_srv, server_srv_size, (void*)(server_srv + 0x006F6910), g_SynUtils.getCppAddr(Hooks::HunterCrashFix));
    HookFunctionInSharedObject(engine_srv, engine_srv_size, (void*)(engine_srv + 0x000EBE10), g_SynUtils.getCppAddr(Hooks::NET_BufferToBufferDecompress_Patch));
    HookFunctionInSharedObject(server_srv, server_srv_size, (void*)(server_srv + 0x00AEF9E0), g_SynUtils.getCppAddr(Hooks::EmptyCall));
    HookFunctionInSharedObject(server_srv, server_srv_size, (void*)(server_srv + 0x00A6A660), g_SynUtils.getCppAddr(Hooks::EmptyCall));
    HookFunctionInSharedObject(server_srv, server_srv_size, (void*)(server_srv + 0x00687440), g_SynUtils.getCppAddr(Hooks::EmptyCall));
    HookFunctionInSharedObject(server_srv, server_srv_size, (void*)(server_srv + 0x00471300), g_SynUtils.getCppAddr(Hooks::EmptyCall));
    HookFunctionInSharedObject(server_srv, server_srv_size, (void*)(server_srv + 0x00B03590), g_SynUtils.getCppAddr(Hooks::EmptyCall));
    HookFunctionInSharedObject(server_srv, server_srv_size, (void*)(server_srv + 0x00471320), g_SynUtils.getCppAddr(Hooks::EmptyCall));
    HookFunctionInSharedObject(server_srv, server_srv_size, (void*)(server_srv + 0x004A5540), g_SynUtils.getCppAddr(Hooks::EmptyCall));
    HookFunctionInSharedObject(server_srv, server_srv_size, (void*)(server_srv + 0x004CCA80), g_SynUtils.getCppAddr(Hooks::LevelChangeSafeHook));
}

void* SynergyUtils::getCppAddr(auto classAddr)
{
    return (void*&)classAddr;
}