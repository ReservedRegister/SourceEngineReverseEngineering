#include "extension.h"
#include "core.h"

uint32_t hook_exclude_list_offset[512] = {};
uint32_t hook_exclude_list_base[512] = {};
uint32_t memory_prots_save_list[512] = {};
uint32_t loaded_libraries[512] = {};
uint32_t our_libraries[512] = {};

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

pThreeArgProt FindEntityByClassname;
pTwoArgProt SetSolidFlags;

uint32_t CGlobalEntityList;
int hooked_delete_counter;
int normal_delete_counter;
bool isTicking;
bool disable_delete_list;
bool player_spawned;
bool server_sleeping;

void InitCore()
{
    //Populate our libraries

    our_libraries[0] = (uint32_t)malloc(1024);
    snprintf((char*)our_libraries[0], 1024, "%s", "/bms/bin/server_srv.so");

    our_libraries[1] = (uint32_t)malloc(1024);
    snprintf((char*)our_libraries[1], 1024, "%s", "/bin/engine_srv.so");

    our_libraries[2] = (uint32_t)malloc(1024);
    snprintf((char*)our_libraries[2], 1024, "%s", "/bin/materialsystem_srv.so");

    our_libraries[3] = (uint32_t)malloc(1024);
    snprintf((char*)our_libraries[3], 1024, "%s", "/bin/vphysics_srv.so");

    our_libraries[4] = (uint32_t)malloc(1024);
    snprintf((char*)our_libraries[4], 1024, "%s", "/bin/dedicated_srv.so");

    our_libraries[5] = (uint32_t)malloc(1024);
    snprintf((char*)our_libraries[5], 1024, "%s", "/bin/datacache_srv.so");
}

void* copy_val(void* val, size_t copy_size)
{
    if(val == 0)
        return 0;
    
    void* copy_ptr = malloc(copy_size);
    memcpy(copy_ptr, val, copy_size);
    return copy_ptr;
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

Library* FindLibrary(char* lib_name, bool less_intense_search)
{
    for(int i = 0; i < 512; i++)
    {
        if(loaded_libraries[i] == 0) continue;
        Library* existing_lib = (Library*)loaded_libraries[i];
        
        if(less_intense_search)
        {
            if(strcasestr(existing_lib->library_signature, lib_name) != NULL) return existing_lib;
        }

        if(strcmp(existing_lib->library_signature, lib_name) == 0) return existing_lib;
    }

    return NULL;
}

Library* LoadLibrary(char* library_full_path)
{
    if(library_full_path)
    {
        Library* found_lib = FindLibrary(library_full_path, false);
        if(found_lib) return found_lib;

        struct link_map* library_lm = (struct link_map*)(dlopen(library_full_path, RTLD_NOW));

        if(library_lm)
        {
            for(int i = 0; i < 512; i++)
            {
                if(loaded_libraries[i] == 0)
                {
                    Library* new_lib = (Library*)(malloc(sizeof(Library)));
                    new_lib->library_signature = (char*)copy_val(library_full_path, strlen(library_full_path)+1);
                    new_lib->library_base_address = library_lm->l_addr;
                    new_lib->library_size = 0;
                    loaded_libraries[i] = (uint32_t)new_lib;
                    
                    rootconsole->ConsolePrint("Loaded [%s]", library_full_path);
                    return new_lib;
                }
            }

            rootconsole->ConsolePrint("Failed to save library to list!");
            exit(EXIT_FAILURE);
        }
    }

    return NULL;
}

Library* getlibrary(char* file_line)
{
    for(int i = 0; i < 512; i++)
    {
        if(our_libraries[i] == 0) continue;

        char* match = strcasestr(file_line, (char*)our_libraries[i]);

        if(match)
        {
            int temp_char_reverser = 0;
            char* abs_path = NULL;

            while(abs_path == NULL)
            {
                if(*(char*)(match-temp_char_reverser) == ' ')
                {
                    if(*(char*)(match-temp_char_reverser+1) == '/')
                    {
                        abs_path = match-temp_char_reverser+1;
                    }
                }

                temp_char_reverser++;
            }

            /*char file_line_temp[512];
            snprintf(file_line_temp, 512, "%s", file_line);
            strtok(file_line_temp, " \t");
            for(int i = 0; i < 4; i++) strtok(NULL, " \t");
            char* abs_path = strtok(NULL, " \t");*/

            //rootconsole->ConsolePrint("abs [%s]", abs_path);

            Library* found_lib = LoadLibrary(abs_path);

            if(found_lib)
            {
                //rootconsole->ConsolePrint("Detected our library [%s]", our_libraries[i]);
                return found_lib;
            }
        }
    }

    return NULL;
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

        Library* currentLibrary = getlibrary(file_line);
        if(!currentLibrary) continue;

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

        if(start_address_parsed && end_address_parsed)
        {
            int address_size = end_address_parsed - start_address_parsed;

            for(int i = 0; i < 512 && i+1 < 512 && i+2 < 512; i = i+3)
            {
                if(memory_prots_save_list[i] == 0 && memory_prots_save_list[i+1] == 0 && memory_prots_save_list[i+2] == 0)
                {
                    memory_prots_save_list[i] = start_address_parsed;
                    memory_prots_save_list[i+1] = end_address_parsed;
                    memory_prots_save_list[i+2] = (uint32_t)save_protections;
                    //rootconsole->ConsolePrint("Saved [%X] [%X] [%s]", end_address_parsed, start_address_parsed, currentLibrary->library_signature);
                    break;
                }
            }

            currentLibrary->library_size += address_size;
        }

        free(file_line_cpy);
    }

    free(file_line);
    fclose(smaps_file);

    ForceMemoryAccess();
}

void ForceMemoryAccess()
{
    for(int i = 0; i < 512 && i+1 < 512 && i+2 < 512; i = i+3)
    {
        if(memory_prots_save_list[i] == 0 && memory_prots_save_list[i+1] == 0 && memory_prots_save_list[i+2] == 0) continue;
        
        size_t pagesize = sysconf(_SC_PAGE_SIZE);
        uint32_t pagestart = memory_prots_save_list[i] & -pagesize;

        if(mprotect((void*)pagestart, memory_prots_save_list[i+1] - memory_prots_save_list[i], PROT_READ | PROT_WRITE | PROT_EXEC) == -1)
        {
            //rootconsole->ConsolePrint("Failed protection change: [%X] [%X]", memory_prots_save_list[i+1], memory_prots_save_list[i]);

            //SELINUX shite

            //perror("mprotect");
            //exit(EXIT_FAILURE);
        }
        else
        {
            //rootconsole->ConsolePrint("Passed protection change: [%X] [%X]", memory_prots_save_list[i+1], memory_prots_save_list[i]);
        }
    }
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

uint32_t IsEntityValid(uint32_t entity)
{
    pOneArgProt pDynamicOneArgFunc;
    if(entity == 0) return entity;

    uint32_t object = GetCBaseEntity(*(uint32_t*)(entity+0x334));

    if(object)
    {
        //IsMarkedForDeletion
        pDynamicOneArgFunc = (pOneArgProt)(server_srv + 0x00B08580);
        uint32_t isMarked = pDynamicOneArgFunc(object+0x14);

        if(isMarked)
        {
            return 0;
        }

        return entity;
    }

    return 0;
}

void CheckForLocation()
{
    uint32_t sv = engine_srv + 0x003329C0;
    uint32_t current_map = sv+0x11;

    if(strcmp((char*)current_map, "bm_c2a3a") != 0)
    {
        //rootconsole->ConsolePrint("Location fix disabled!");
        return;
    }

    uint32_t player = 0;

    while((player = FindEntityByClassname(CGlobalEntityList, player, (uint32_t)"player")) != 0)
    {
        bool in_area = false;
        uint32_t player_abs = player+0x294;
        //rootconsole->ConsolePrint("[%f] [%f] [%f]", *(float*)(player_abs), *(float*)(player_abs+0x4), *(float*)(player_abs+0x8));

        Vector* trigger_vecMinsAbs = (Vector*)(malloc(sizeof(Vector)));
        trigger_vecMinsAbs->x = 1314.0;
        trigger_vecMinsAbs->y = 106.0;
        trigger_vecMinsAbs->z = -1349.0;

        Vector* trigger_vecMaxsAbs = (Vector*)(malloc(sizeof(Vector)));
        trigger_vecMaxsAbs->x = 2282.0;
        trigger_vecMaxsAbs->y = 947.0;
        trigger_vecMaxsAbs->z = -1102.0;

        if(trigger_vecMinsAbs->x <= *(float*)(player_abs) && *(float*)(player_abs) <= trigger_vecMaxsAbs->x)
        {
            if(trigger_vecMinsAbs->y <= *(float*)(player_abs+0x4) && *(float*)(player_abs+0x4) <= trigger_vecMaxsAbs->y)
            {
                if(trigger_vecMinsAbs->z <= *(float*)(player_abs+0x8) && *(float*)(player_abs+0x8) <= trigger_vecMaxsAbs->z)
                {
                    uint32_t collision_property = player+0x160;
                    uint16_t current_flags = *(uint16_t*)(collision_property+0x3C);

                    SetSolidFlags(collision_property, 4);
                    //rootconsole->ConsolePrint("bad area!");
                    in_area = true;
                }
            }
        }

        if(!in_area)
        {
            uint32_t collision_property = player+0x160;
            uint16_t current_flags = *(uint16_t*)(collision_property+0x3C);

            SetSolidFlags(collision_property, 16);
        }

        free(trigger_vecMinsAbs);
        free(trigger_vecMaxsAbs);
    }
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

void RemoveEntityNormal(uint32_t entity_object, bool validate)
{
    pZeroArgProt pDynamicZeroArgFunc;
    pOneArgProt pDynamicOneArgFunc;
    pTwoArgProt pDynamicTwoArgFunc;

    if(entity_object == 0) return;

    char* classname = (char*)(*(uint32_t*)(entity_object+0x64));
    uint32_t refHandle = *(uint32_t*)(entity_object+0x334);
    uint32_t object_verify = GetCBaseEntity(refHandle);

    if(object_verify == 0)
    {
        if(!validate)
        {
            rootconsole->ConsolePrint("Warning: Entity delete request granted without validation!");
            object_verify = entity_object;
        }
    }

    if(object_verify)
    {
        //VphysicsDestroyObject
        //pDynamicOneArgFunc = (pOneArgProt)( *(uint32_t*)((*(uint32_t*)(object_verify))+0x2A0) );
        //pDynamicOneArgFunc(object_verify);

        //rootconsole->ConsolePrint("Removing [%s]", clsname);

        //IsMarkedForDeletion
        pDynamicOneArgFunc = (pOneArgProt)(server_srv + 0x00B08580);
        uint32_t isMarked = pDynamicOneArgFunc(object_verify+0x14);

        if(isMarked)
        {
            //rootconsole->ConsolePrint("Attempted to kill a marked entity in UTIL_Remove(IServerNetworkable*)");
            return;
        }

        //IsInPhysCallback
        pDynamicZeroArgFunc = (pZeroArgProt)(server_srv + 0x00A63D80);
        uint8_t returnVal = pDynamicZeroArgFunc();

        if(returnVal == 0)
        {
            hooked_delete_counter++;
        }

        //UTIL_Remove(IServerNetworkable*)
        pDynamicOneArgFunc = (pOneArgProt)(server_srv + 0x00B66AF0);
        pDynamicOneArgFunc(object_verify+0x14);

        //rootconsole->ConsolePrint("Removed [%s]", clsname);

        return;
    }

    rootconsole->ConsolePrint("Failed to verify entity object!");
    exit(EXIT_FAILURE);
    return;
}

void InstaKill(uint32_t entity_object, bool validate)
{
    pZeroArgProt pDynamicZeroArgFunc;
    pThreeArgProt pDynamicThreeArgFunc;
    pOneArgProt pDynamicOneArgFunc;
    pTwoArgProt pDynamicTwoArgFunc;

    if(entity_object == 0) return;

    uint32_t refHandleInsta = *(uint32_t*)(entity_object+0x334);
    char* classname = (char*) ( *(uint32_t*)(entity_object+0x64) );
    uint32_t cbase_chk = GetCBaseEntity(refHandleInsta);

    if(cbase_chk == 0)
    {
        if(!validate)
        {
            rootconsole->ConsolePrint("Warning: Entity delete request granted without validation!");
            cbase_chk = entity_object;
        }
        else
        {
            rootconsole->ConsolePrint("\n\nFailed to verify entity for fast kill [%X]\n\n", (uint32_t)__builtin_return_address(0) - server_srv);
            exit(EXIT_FAILURE);
            return;
        }
    }

    //IsMarkedForDeletion
    pDynamicOneArgFunc = (pOneArgProt)(server_srv + 0x00B08580);
    uint32_t isMarked = pDynamicOneArgFunc(cbase_chk+0x14);

    if(isMarked)
    {
        rootconsole->ConsolePrint("Attempted to kill an entity twice in UTIL_RemoveImmediate(CBaseEntity*)");
        return;
    }

    //PhysIsInCallback
    pDynamicOneArgFunc = (pOneArgProt)(server_srv + 0x00A63D80);
    uint32_t isInCallback = pDynamicOneArgFunc(0);

    if(isInCallback)
    {
        rootconsole->ConsolePrint("Should not be! (Insta)");
        exit(EXIT_FAILURE);

        //CCollisionEvent - AddRemoveObject
        //pDynamicTwoArgFunc = (pTwoArgProt)(server_srv + 0x00A698D0);
        //pDynamicTwoArgFunc(server_srv + 0x018AE4C0, cbase_chk+0x14);
        return;
    }

    if(isTicking)
    {
        rootconsole->ConsolePrint("fast killed [%s]", classname);
    }

    if((*(uint32_t*)(cbase_chk+0x118) & 1) == 0)
    {
        if(*(uint32_t*)(server_srv + 0x018CBEC0) == 0)
        {
            // FAST DELETE ONLY

            hooked_delete_counter++;

            //VphysicsDestroyObject
            //pDynamicOneArgFunc = (pOneArgProt)( *(uint32_t*)((*(uint32_t*)(cbase_chk))+0x2A0) );
            //pDynamicOneArgFunc(cbase_chk);

            *(uint8_t*)(server_srv + 0x018C98E4) = 0;
            *(uint32_t*)(cbase_chk+0x118) = *(uint32_t*)(cbase_chk+0x118) | 1;

            //UpdateOnRemove
            pDynamicOneArgFunc = (pOneArgProt)(  *(uint32_t*)((*(uint32_t*)(cbase_chk))+0x1D0) );
            pDynamicOneArgFunc(cbase_chk);

            *(uint8_t*)(server_srv + 0x018C98E5) = 1;

            //CALL RELEASE
            uint32_t iServerObj = cbase_chk+0x14;

            pDynamicOneArgFunc = (pOneArgProt)(  *(uint32_t*)((*(uint32_t*)(iServerObj))+0x10) );
            pDynamicOneArgFunc(iServerObj);

            *(uint8_t*)(server_srv + 0x018C98E5) = 0;
        }
        else
        {
            RemoveEntityNormal(cbase_chk, validate);
            return;
        }
    }

    return;
}

void DestroyVObjectForMarkedEnts()
{
    pOneArgProt pDynamicOneArgFunc;
    pTwoArgProt pDynamicTwoArgFunc;
    
    int ent_size = *(int*)(server_srv + 0x018913AC);
    uint32_t g_DeleteList = *(uint32_t*)(server_srv + 0x018913A0);

    if(ent_size > 0)
    {
        for(int i = 0; i < ent_size; i++)
        {
            uint32_t iServerObj = *(uint32_t*)(g_DeleteList+i*4);
            uint32_t cbase = *(uint32_t*)(iServerObj+8);
            uint32_t refHandle = *(uint32_t*)(cbase+0x334);
            uint32_t cbase_verified = GetCBaseEntity(refHandle);

            if(cbase_verified)
            {
                //rootconsole->ConsolePrint("v obj dest! [%s]", *(uint32_t*)(cbase+0x64));

                //VphysicsDestroyObject
                pDynamicOneArgFunc = (pOneArgProt)( *(uint32_t*)((*(uint32_t*)(cbase_verified))+0x2A0) );
                pDynamicOneArgFunc(cbase_verified);
                continue;
            }

            rootconsole->ConsolePrint("Critical error invalid entity object!");
            exit(EXIT_FAILURE);
        }
    }
}