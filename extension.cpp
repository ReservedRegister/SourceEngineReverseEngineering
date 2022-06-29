#include "extension.h"

BmsUtils g_BmsUtils;		/**< Global singleton for extension's main interface */
SMEXT_LINK(&g_BmsUtils);

struct link_map *engine_srv_lm;
struct link_map *server_srv_lm;
struct link_map *materialsystem_srv_lm;

uint32_t engine_srv;
uint32_t server_srv;
uint32_t materialsystem_srv;

uint32_t engine_srv_size;
uint32_t server_srv_size;
uint32_t materialsystem_srv_size;

uint32_t hook_exclude_list_offset[512] = {};
uint32_t hook_exclude_list_base[512] = {};
uint32_t memory_prots_save_list[512] = {};

bool BmsUtils::SDK_OnLoad(char *error, size_t maxlen, bool late)
{
    AllowWriteToMappedMemory();

    char* root_dir = getenv("PWD");
    size_t max_path_length = 1024;

    char server_srv_fullpath[max_path_length];
    char engine_srv_fullpath[max_path_length];
    char materialsystem_srv_fullpath[max_path_length];

    snprintf(server_srv_fullpath, max_path_length, "%s/bms/bin/server_srv.so", root_dir);
    snprintf(engine_srv_fullpath, max_path_length, "%s/bin/engine_srv.so", root_dir);
    snprintf(materialsystem_srv_fullpath, max_path_length, "%s/bin/materialsystem_srv.so", root_dir);

    engine_srv_lm = (struct link_map*)(dlopen(engine_srv_fullpath, RTLD_NOW));
    server_srv_lm = (struct link_map*)(dlopen(server_srv_fullpath, RTLD_NOW));
    materialsystem_srv_lm = (struct link_map*)(dlopen(materialsystem_srv_fullpath, RTLD_NOW));

    engine_srv_size = 0x2C2000;
    server_srv_size = 0xFD7000;
    materialsystem_srv_size = 0x167000;

    engine_srv = engine_srv_lm->l_addr;
    server_srv = server_srv_lm->l_addr;
    materialsystem_srv = materialsystem_srv_lm->l_addr;

    PopulateHookExclusionLists();
    HookFunctionsWithC();
    return true;
}

void BmsUtils::SDK_OnAllLoaded()
{
    HookFunctionsWithCpp();
    RestoreMemoryProtections();
    DisableCacheCvars();
    rootconsole->ConsolePrint("----------------------  " SMEXT_CONF_NAME " loaded!" "  ----------------------");
}

void DisableCacheCvars()
{
    pTwoArgProt pDynamicTwoArgFunc;
    pDynamicTwoArgFunc = (pTwoArgProt)(engine_srv + 0x001B1340);

    pDynamicTwoArgFunc(0, (uint32_t)"mod_forcetouchdata 0");
    pDynamicTwoArgFunc(0, (uint32_t)"mod_forcedata 0");
}

uint32_t CallocHook(uint32_t nitems, uint32_t size)
{
    if(nitems <= 0) return (uint32_t)calloc(nitems, size);

    uint32_t enlarged_size = nitems*1.115+8;
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

    uint32_t newRef = (uint32_t)malloc(size*1.115+8);
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
    uint32_t new_ref = (uint32_t)realloc((void*)old_ptr, new_size*1.115+8);

    /*void* returnAddr = __builtin_return_address(0);
    RemoveAllocationRef(mallocAllocations, (void*)old_ptr, true);
    MallocRef* new_ref_value = CreateNewMallocRef((void*)new_ref, (void*)new_size, (void*)((uint32_t)returnAddr - 5), (void*)"malloc");
    InsertToMallocRefList(mallocAllocations, new_ref_value, true);*/

    return new_ref;
}

uint32_t OperatorNewHook(uint32_t size)
{
    if(size <= 0) return (uint32_t)malloc(size);
    uint32_t newRef = (uint32_t)malloc(size);
    //rootconsole->ConsolePrint("malloc() ref: [%X] size: [%X] list_size [%d]", newRef, size, MallocRefListSize(mallocAllocations));
    //rootconsole->ConsolePrint("malloc() ref: [%X] size: [%X]", newRef, size);

    /*void* returnAddr = __builtin_return_address(0);
    MallocRef* new_ref_value = CreateNewMallocRef((void*)newRef, (void*)size, (void*)((uint32_t)returnAddr - 5), (void*)"operator_new");
    InsertToMallocRefList(mallocAllocations, new_ref_value, true);*/

    return newRef;
}

uint32_t OperatorNewArrayHook(uint32_t size)
{
    if(size <= 0) return (uint32_t)malloc(size);
    uint32_t newRef = (uint32_t)malloc(size*1.115+8);
    //rootconsole->ConsolePrint("malloc() ref: [%X] size: [%X] list_size [%d]", newRef, size, MallocRefListSize(mallocAllocations));
    //rootconsole->ConsolePrint("malloc() ref: [%X] size: [%X]", newRef, size);

    /*void* returnAddr = __builtin_return_address(0);
    MallocRef* new_ref_value = CreateNewMallocRef((void*)newRef, (void*)size, (void*)((uint32_t)returnAddr - 5), (void*)"operator_new_array");
    InsertToMallocRefList(mallocAllocations, new_ref_value, true);*/

    return newRef;
}

uint32_t Hooks::EmptyCall()
{
    return 0;
}

uint32_t Hooks::SpawnServerHook(uint32_t arg0, uint32_t arg1)
{
    rootconsole->ConsolePrint(EXT_PREFIX "SpawnServer Hooked 4\n\n");
    pOneArgProt pDynamicOneArgFunc;
    pTwoArgProt pDynamicTwoArgFunc;

    //UnloadAllModels
    pDynamicTwoArgFunc = (pTwoArgProt)(engine_srv + 0x0013AE80);
    pDynamicTwoArgFunc(engine_srv + 0x00320560, 0);

    //Flush materials
    pDynamicTwoArgFunc = (pTwoArgProt)(materialsystem_srv + 0x0003D280);
    pDynamicTwoArgFunc(materialsystem_srv + 0x00166B20, 1);

    //ReloadAllMaterials
    pDynamicTwoArgFunc = (pTwoArgProt)(materialsystem_srv + 0x0003D220);
    pDynamicTwoArgFunc(materialsystem_srv + 0x00166B20, 0);

    //ReloadTextures
    pDynamicOneArgFunc = (pOneArgProt)(materialsystem_srv + 0x0003D240);
    pDynamicOneArgFunc(materialsystem_srv + 0x00166B20);

    //InvalidateMdlCache
    //pDynamicOneArgFunc = (pOneArgProt)(server_srv + 0x00947CC0);
    //pDynamicOneArgFunc(0);

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
    rootconsole->ConsolePrint("patching calloc()");
    HookFunctionInSharedObject(server_srv, server_srv_size, (void*)calloc, (void*)CallocHook);
    HookFunctionInSharedObject(engine_srv, engine_srv_size, (void*)calloc, (void*)CallocHook);
    rootconsole->ConsolePrint("patching malloc()");
    HookFunctionInSharedObject(server_srv, server_srv_size, (void*)malloc, (void*)MallocHook);
    HookFunctionInSharedObject(engine_srv, engine_srv_size, (void*)malloc, (void*)MallocHook);
    rootconsole->ConsolePrint("patching realloc()");
    HookFunctionInSharedObject(server_srv, server_srv_size, (void*)realloc, (void*)ReallocHook);
    HookFunctionInSharedObject(engine_srv, engine_srv_size, (void*)realloc, (void*)ReallocHook);
}

void HookFunctionsWithCpp()
{
    HookFunctionInSharedObject(server_srv, server_srv_size, (void*)(server_srv + 0x00942190), g_BmsUtils.getCppAddr(Hooks::SpawnServerHook));
}

void* BmsUtils::getCppAddr(auto classAddr)
{
    return (void*&)classAddr;
}