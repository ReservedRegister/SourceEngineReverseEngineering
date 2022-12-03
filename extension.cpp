#include "extension.h"

BmsUtils g_BmsUtils;		/**< Global singleton for extension's main interface */
SMEXT_LINK(&g_BmsUtils);

struct link_map *engine_srv_lm;
struct link_map *server_srv_lm;
struct link_map *materialsystem_srv_lm;
struct link_map *vphysics_srv_lm;
struct link_map *dedicated_srv_lm;
struct link_map *datacache_srv_lm;

uint32_t engine_srv;
uint32_t server_srv;
uint32_t materialsystem_srv;
uint32_t vphysics_srv;
uint32_t dedicated_srv;
uint32_t datacache_srv;

uint32_t engine_srv_size;
uint32_t server_srv_size;
uint32_t materialsystem_srv_size;
uint32_t vphysics_srv_size;
uint32_t dedicated_srv_size;
uint32_t datacache_srv_size;

uint32_t hook_exclude_list_offset[512] = {};
uint32_t hook_exclude_list_base[512] = {};
uint32_t memory_prots_save_list[512] = {};

bool isTicking;
bool disable_delete_list;
uint32_t CGlobalEntityList;
ValueList deleteList;

bool BmsUtils::SDK_OnLoad(char *error, size_t maxlen, bool late)
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

    engine_srv_lm = (struct link_map*)(dlopen(engine_srv_fullpath, RTLD_NOW));
    server_srv_lm = (struct link_map*)(dlopen(server_srv_fullpath, RTLD_NOW));
    materialsystem_srv_lm = (struct link_map*)(dlopen(materialsystem_srv_fullpath, RTLD_NOW));
    vphysics_srv_lm = (struct link_map*)(dlopen(vphysics_srv_fullpath, RTLD_NOW));
    dedicated_srv_lm = (struct link_map*)(dlopen(dedicated_srv_fullpath, RTLD_NOW));
    datacache_srv_lm = (struct link_map*)(dlopen(datacache_srv_fullpath, RTLD_NOW));

    engine_srv_size = 0x2C2000;
    server_srv_size = 0xFD7000;
    materialsystem_srv_size = 0x167000;
    vphysics_srv_size = 0x1B4000;
    dedicated_srv_size = 0x251000;
    datacache_srv_size = 0x7B000;

    engine_srv = engine_srv_lm->l_addr;
    server_srv = server_srv_lm->l_addr;
    materialsystem_srv = materialsystem_srv_lm->l_addr;
    vphysics_srv = vphysics_srv_lm->l_addr;
    dedicated_srv = dedicated_srv_lm->l_addr;
    datacache_srv = datacache_srv_lm->l_addr;

    disable_delete_list = false;
    isTicking = false;
    CGlobalEntityList = server_srv + 0x018711E0;
    deleteList = AllocateValuesList();

    PopulateHookExclusionLists();
    HookFunctionsWithC();
    return true;
}

void BmsUtils::SDK_OnAllLoaded()
{
    ApplySingleHooks();
    HookFunctionsWithCpp();
    RestoreMemoryProtections();
    DisableCacheCvars();
    rootconsole->ConsolePrint("----------------------  " SMEXT_CONF_NAME " " SMEXT_CONF_VERSION " loaded!" "  ----------------------");
}

void ApplySingleHooks()
{
    uint32_t offset = 0;

    uint32_t hook_game_frame_delete_list = server_srv + 0x00944FAF;
    offset = (uint32_t)BmsUtils::getCppAddr(Hooks::GameFrameHook) - hook_game_frame_delete_list - 5;
    *(uint32_t*)(hook_game_frame_delete_list+1) = offset;

    /*uint32_t panim_crash_fix_two = server_srv + 0x0052098D;
    *(uint8_t*)(panim_crash_fix_two) = 0xE9;
    *(uint32_t*)(panim_crash_fix_two+1) = 0x1D6;

    uint32_t delete_panim_call = server_srv + 0x0053CF11;
    *(uint8_t*)(delete_panim_call) = 0xEB;

    uint32_t delete_panim_call_two = server_srv + 0x0053CFC5;
    memset((void*)delete_panim_call_two, 0x90, 2);*/

    /*uint32_t delete_list_call = server_srv + 0x00944F61;
    memset((void*)delete_list_call, 0x90, 5);

    delete_list_call = server_srv + 0x00944FC5;
    memset((void*)delete_list_call, 0x90, 5);

    delete_list_call = server_srv + 0x00A7AC57;
    memset((void*)delete_list_call, 0x90, 5);*/
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

uint32_t CallocHook(uint32_t nitems, uint32_t size)
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

uint32_t MallocHook(uint32_t size)
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

uint32_t ReallocHook(uint32_t old_ptr, uint32_t new_size)
{
    if(new_size <= 0) return (uint32_t)realloc((void*)old_ptr, new_size);
    uint32_t new_ref = (uint32_t)realloc((void*)old_ptr, new_size*2.0);

    /*void* returnAddr = __builtin_return_address(0);
    RemoveAllocationRef(mallocAllocations, (void*)old_ptr, true);
    MallocRef* new_ref_value = CreateNewMallocRef((void*)new_ref, (void*)new_size, (void*)((uint32_t)returnAddr - 5), (void*)"malloc");
    InsertToMallocRefList(mallocAllocations, new_ref_value, true);*/

    return new_ref;
}

uint32_t OperatorNewArrayHook(uint32_t size)
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

uint32_t GetCBaseEntity(uint32_t EHandle)
{
    uint32_t shift_right = EHandle >> 0x0D;
    uint32_t disassembly = EHandle & 0x1FFF;
    disassembly = disassembly << 0x4;
    disassembly = CGlobalEntityList + disassembly;

    if( ((*(uint32_t*)(disassembly+0x08))) == shift_right)
    {
        uint32_t CBaseEntity = *(uint32_t*)(disassembly+0x04);
        return CBaseEntity;
    }

    return 0;
}

uint32_t Hooks::HostChangelevelHook(uint32_t arg0, uint32_t arg1, uint32_t arg2)
{
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

uint32_t CalcPoseSingleHook(uint32_t arg0, uint32_t arg1, uint32_t arg2, uint32_t arg3, uint32_t arg4, uint32_t arg5, uint32_t arg6,
uint32_t arg7, uint32_t arg8)
{
    pOneArgProt pDynamicOneArgFunc;
    pNineArgProt pDynamicNineArgProt;

    pDynamicOneArgFunc = (pOneArgProt)(engine_srv + 0x000C9DC0);
    uint32_t mapName = pDynamicOneArgFunc(engine_srv + 0x003329C0);

    if(strncmp((char*)mapName, "bm_c4", 5) == 0 || strncmp((char*)mapName, "bm_c5", 5) == 0)
    {
        pDynamicNineArgProt = (pNineArgProt)(server_srv + 0x00525F30);
        return pDynamicNineArgProt(arg0, arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8);
    }

    //rootconsole->ConsolePrint("CalcPoseSingle() is only allowed for Xen maps!");
    return 0;
}

uint32_t Hooks::Util_RemoveHook(uint32_t arg0)
{
    pOneArgProt pDynamicOneArgFunc;
    if(arg0 == 0) return 0;

    uint32_t refHandle = *(uint32_t*)(arg0+0x334);
    char* clsname = (char*)(*(uint32_t*)(arg0+0x64));

    pDynamicOneArgFunc = (pOneArgProt)(server_srv + 0x00B66B70);
    return pDynamicOneArgFunc(arg0);
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

    pDynamicOneArgFunc = (pOneArgProt)(engine_srv + 0x001957B0);
    return pDynamicOneArgFunc(arg0);
}

uint32_t Hooks::GameFrameHook(uint32_t arg0)
{
    pOneArgProt pDynamicOneArgFunc;
    pThreeArgProt pDynamicThreeArgFunc;

    isTicking = true;

    //PreSystems
    pDynamicOneArgFunc = (pOneArgProt)(server_srv + 0x004CA9E0);
    pDynamicOneArgFunc(0);

    //PostSystems
    pDynamicOneArgFunc = (pOneArgProt)(server_srv + 0x004CAA00);
    pDynamicOneArgFunc(0);

    //UpdateClientData
    pDynamicOneArgFunc = (pOneArgProt)(server_srv + 0x00AB1D20);
    pDynamicOneArgFunc(0);

    //StartFrame
    pDynamicOneArgFunc = (pOneArgProt)(server_srv + 0x006BD6F0);
    pDynamicOneArgFunc(0);

    //SimulateEntities
    pDynamicOneArgFunc = (pOneArgProt)(server_srv + 0x00A7AC00);
    pDynamicOneArgFunc(arg0);

    //ServiceEventQueue
    pDynamicOneArgFunc = (pOneArgProt)(server_srv + 0x008C9950);
    pDynamicOneArgFunc(0);
    return 0;
}

uint32_t Hooks::HookInstaKill(uint32_t arg0)
{
    pThreeArgProt pDynamicThreeArgFunc;
    pOneArgProt pDynamicOneArgFunc;

    char* clsname =  (char*) ( *(uint32_t*)(arg0+0x64) );

    if(isTicking && strcmp(clsname, "player") != 0)
    {
        rootconsole->ConsolePrint("slow killed instead [%s]", clsname);
        Hooks::Util_RemoveHook(arg0);
        return 0;
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

    if(isMarked) return 0;

    disable_delete_list = true;
    pDynamicOneArgFunc = (pOneArgProt)(server_srv + 0x00A7A730);
    uint32_t returnVal = pDynamicOneArgFunc(arg0);
    disable_delete_list = false;
    return returnVal;
}

uint32_t Hooks::SpawnServerHook(uint32_t arg0, uint32_t arg1)
{
    rootconsole->ConsolePrint(EXT_PREFIX "SpawnServer Hooked 5\n\n");
    pOneArgProt pDynamicOneArgFunc;
    pTwoArgProt pDynamicTwoArgFunc;
    pThreeArgProt pDynamicThreeArgFunc;

    //UnloadAllModels
    pDynamicTwoArgFunc = (pTwoArgProt)(engine_srv + 0x0013AE80);
    pDynamicTwoArgFunc(engine_srv + 0x00320560, 0);

    //Flush materials
    //pDynamicTwoArgFunc = (pTwoArgProt)(materialsystem_srv + 0x0003D280);
    //pDynamicTwoArgFunc(materialsystem_srv + 0x00166B20, 1);

    //ReloadTextures
    pDynamicOneArgFunc = (pOneArgProt)(materialsystem_srv + 0x0003D240);
    pDynamicOneArgFunc(materialsystem_srv + 0x00166B20);

    //ReloadAllMaterials
    pDynamicTwoArgFunc = (pTwoArgProt)(materialsystem_srv + 0x0003D220);
    pDynamicTwoArgFunc(materialsystem_srv + 0x00166B20, 0);

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

        if(strcasestr(file_line, "/bin/") == NULL)
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

void PopulateHookExclusionLists()
{
    hook_exclude_list_base[0] = server_srv;
    hook_exclude_list_offset[0] = 0x0052B02C;

    hook_exclude_list_base[1] = server_srv;
    hook_exclude_list_offset[1] = 0x00628096;

    hook_exclude_list_base[2] = server_srv;
    hook_exclude_list_offset[2] = 0x006248D9;

    hook_exclude_list_base[3] = server_srv;
    hook_exclude_list_offset[3] = 0x0052B131;
}

bool IsAddressExcluded(uint32_t base_address, uint32_t search_address)
{
    for(int i = 0; i < 512; i++)
    {
        if(hook_exclude_list_offset[i] == 0 || hook_exclude_list_base[i] == 0)
            continue;

        uint32_t patch_address = base_address + hook_exclude_list_offset[i];

        if(patch_address == search_address && hook_exclude_list_base[i] == base_address)
            return true;
    }

    return false;
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
            if(IsAddressExcluded(base_address, search_address))
            {
                rootconsole->ConsolePrint("(abs) Skipped patch at [%X]", search_address);
                search_address++;
                continue;
            }

            //rootconsole->ConsolePrint("Patched abs address: [%X]", search_address);
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
                if(IsAddressExcluded(base_address, search_address))
                {
                    rootconsole->ConsolePrint("(unsigned) Skipped patch at [%X]", search_address);
                    search_address++;
                    continue;
                }

                //rootconsole->ConsolePrint("(unsigned) Hooked address: [%X]", search_address - base_address);
                uint32_t offset = (uint32_t)hook_pointer - search_address - 5;
                *(uint32_t*)(search_address+1) = offset;
            }
            else
            {
                //check signed addition
                chk = search_address + (int32_t)call_address + 5;

                if(chk == (uint32_t)target_pointer)
                {
                    if(IsAddressExcluded(base_address, search_address))
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

void HookFunctionsWithC()
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
}

void HookFunctionsWithCpp()
{
    HookFunctionInSharedObject(server_srv, server_srv_size, (void*)(server_srv + 0x009AF380), BmsUtils::getCppAddr(Hooks::CreateEntityByNameHook));
    //HookFunctionInSharedObject(server_srv, server_srv_size, (void*)(server_srv + 0x00942190), BmsUtils::getCppAddr(Hooks::SpawnServerHook));
    HookFunctionInSharedObject(server_srv, server_srv_size, (void*)(server_srv + 0x008F3640), BmsUtils::getCppAddr(Hooks::CleanupDeleteListHook));
    HookFunctionInSharedObject(engine_srv, engine_srv_size, (void*)(engine_srv + 0x001957B0), BmsUtils::getCppAddr(Hooks::SV_FrameHook));
    HookFunctionInSharedObject(server_srv, server_srv_size, (void*)(server_srv + 0x00B66B70), BmsUtils::getCppAddr(Hooks::Util_RemoveHook));
    HookFunctionInSharedObject(engine_srv, engine_srv_size, (void*)(engine_srv + 0x0011CB10), BmsUtils::getCppAddr(Hooks::HostChangelevelHook));
    HookFunctionInSharedObject(server_srv, server_srv_size, (void*)(server_srv + 0x00A7A730), BmsUtils::getCppAddr(Hooks::PhysSimEnt));
    HookFunctionInSharedObject(server_srv, server_srv_size, (void*)(server_srv + 0x004CA9E0), BmsUtils::getCppAddr(Hooks::EmptyCall));
    HookFunctionInSharedObject(server_srv, server_srv_size, (void*)(server_srv + 0x006BD6F0), BmsUtils::getCppAddr(Hooks::EmptyCall));
    HookFunctionInSharedObject(server_srv, server_srv_size, (void*)(server_srv + 0x00A7AC00), BmsUtils::getCppAddr(Hooks::EmptyCall));
    HookFunctionInSharedObject(server_srv, server_srv_size, (void*)(server_srv + 0x004CAA00), BmsUtils::getCppAddr(Hooks::EmptyCall));
    HookFunctionInSharedObject(server_srv, server_srv_size, (void*)(server_srv + 0x008C9950), BmsUtils::getCppAddr(Hooks::EmptyCall));
    HookFunctionInSharedObject(server_srv, server_srv_size, (void*)(server_srv + 0x00AB1D20), BmsUtils::getCppAddr(Hooks::EmptyCall));
    //HookFunctionInSharedObject(server_srv, server_srv_size, (void*)(server_srv + 0x00B66BC0), BmsUtils::getCppAddr(Hooks::HookInstaKill));


    HookFunctionInSharedObject(server_srv, server_srv_size, (void*)(server_srv + 0x008A39C0), BmsUtils::getCppAddr(Hooks::EmptyCall));
    //HookFunctionInSharedObject(server_srv, server_srv_size, (void*)(server_srv + 0x0070BD10), BmsUtils::getCppAddr(Hooks::EmptyCall));
    //HookFunctionInSharedObject(server_srv, server_srv_size, (void*)(server_srv + 0x004ED8F0), BmsUtils::getCppAddr(Hooks::EmptyCall));
    //HookFunctionInSharedObject(server_srv, server_srv_size, (void*)(server_srv + 0x00525F30), (void*)CalcPoseSingleHook);


    HookFunctionInSharedObject(server_srv, server_srv_size, (void*)(server_srv + 0x0057B840), BmsUtils::getCppAddr(Hooks::TakeDamageAliveHook));
    HookFunctionInSharedObject(server_srv, server_srv_size, (void*)(server_srv + 0x008835B0), BmsUtils::getCppAddr(Hooks::IRelationTypeHook));
    //HookFunctionInSharedObject(server_srv, server_srv_size, (void*)(server_srv + 0x0054CC80), BmsUtils::getCppAddr(Hooks::EmptyCall));

    HookFunctionInSharedObject(server_srv, server_srv_size, (void*)(server_srv + 0x0052B020), BmsUtils::getCppAddr(Hooks::EmptyCall));
    HookFunctionInSharedObject(server_srv, server_srv_size, (void*)(server_srv + 0x0052A7B0), BmsUtils::getCppAddr(Hooks::EmptyCall));
}

void* BmsUtils::getCppAddr(auto classAddr)
{
    return (void*&)classAddr;
}

ValueList AllocateValuesList()
{
    ValueList list = (ValueList) malloc(sizeof(ValueList));
    *list = NULL;
    return list;
}

Value* CreateNewValue(void* valueInput)
{
    Value* val = (Value*) malloc(sizeof(Value));

    val->value = valueInput;
    val->nextVal = NULL;
    return val;
}

void DeleteAllValuesInList(ValueList list, bool free_val, pthread_mutex_t* lockInput)
{
    while(pthread_mutex_trylock(lockInput) != 0);

    if(!list || !*list)
    {
        pthread_mutex_unlock(lockInput);
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
    pthread_mutex_unlock(lockInput);
}

bool IsInValuesList(ValueList list, void* searchVal, pthread_mutex_t* lockInput)
{
    while(pthread_mutex_trylock(lockInput) != 0);

    Value* aValue = *list;

    while(aValue)
    {
        if((uint32_t)aValue->value == (uint32_t)searchVal)
        {
            pthread_mutex_unlock(lockInput);
            return true;
        }
        
        aValue = aValue->nextVal;
    }

    pthread_mutex_unlock(lockInput);
    return false;
}

bool RemoveFromValuesList(ValueList list, void* searchVal, pthread_mutex_t* lockInput)
{
    while(pthread_mutex_trylock(lockInput) != 0);

    Value* aValue = *list;

    if(aValue == NULL)
    {
        pthread_mutex_unlock(lockInput);
        return false;
    }

    //search at the start of the list
    if(((uint32_t)aValue->value) == ((uint32_t)searchVal))
    {
        Value* detachedValue = aValue->nextVal;
        free(*list);
        *list = detachedValue;
        pthread_mutex_unlock(lockInput);
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
            pthread_mutex_unlock(lockInput);
            return true;
        }

        aValue = aValue->nextVal;
    }

    pthread_mutex_unlock(lockInput);
    return false;
}

int ValueListItems(ValueList list, pthread_mutex_t* lockInput)
{
    while(pthread_mutex_trylock(lockInput) != 0);

    Value* aValue = *list;
    int counter = 0;

    while(aValue)
    {
        counter++;
        aValue = aValue->nextVal;
    }

    pthread_mutex_unlock(lockInput);
    return counter;
}

bool InsertToValuesList(ValueList list, Value* head, pthread_mutex_t* lockInput, bool tail, bool duplicate_chk)
{
    while(pthread_mutex_trylock(lockInput) != 0);

    if(duplicate_chk)
    {
        Value* aValue = *list;

        while(aValue)
        {
            if((uint32_t)aValue->value == (uint32_t)head->value)
            {
                pthread_mutex_unlock(lockInput);
                return false;
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
                pthread_mutex_unlock(lockInput);
                return true;
            }

            aValue = aValue->nextVal;
        }
    }

    head->nextVal = *list;
    *list = head;

    pthread_mutex_unlock(lockInput);
    return true;
}