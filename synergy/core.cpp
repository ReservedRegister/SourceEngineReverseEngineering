#include "extension.h"
#include "util.h"
#include "core.h"
#include "ext_main.h"
#include "sdktools.h"

uint32_t EdtLoadFuncAddr;
uint32_t Flush;
uint32_t HostChangelevel;
uint32_t SpawnServer;
uint32_t LookupPoseParameterAddr;
uint32_t sub_654260_addr;
uint32_t sub_628F00_addr;
uint32_t SaveGameStateAddr;
uint32_t TransitionRestoreMainCallOrigAddr;
uint32_t OriginalTriggerMovedAddr;
uint32_t DoorFinalFunctionAddr;
uint32_t GetNumClientsAddr;
uint32_t GetNumProxiesAddr;
uint32_t EnqueueCommandAddr;
uint32_t DispatchSpawnAddr;
uint32_t ActivateEntityAddr;
uint32_t AutosaveLoadOrigAddr;
uint32_t InactivateClientsAddr;
uint32_t ReconnectClientsAddr;
uint32_t MakeEntityDormantAddr;
uint32_t PrintToClientAddr;

pTwoArgProt pEdtLoadFunc;
pThreeArgProt pHostChangelevelFunc;
pThreeArgProt pFlushFunc;
pThreeArgProt pSpawnServerFunc;
pThreeArgProt LookupPoseParameter;
pOneArgProt sub_654260;
pOneArgProt sub_628F00;
pThreeArgProt SaveGameState;
pFourArgProt pTransitionRestoreMainCall;
pTwoArgProt pCallOriginalTriggerMoved;
pFiveArgProt pDoorFinalFunction;
pOneArgProt GetNumClients;
pOneArgProt GetNumProxies;
pOneArgProt EnqueueCommandFunc;
pOneArgProt pDispatchSpawnFunc;
pOneArgProt pActivateEntityFunc;
pThreeArgProt AutosaveLoadOrig;
pOneArgProt InactivateClients;
pOneArgProt ReconnectClients;
pOneArgProt MakeEntityDormant;
pSevenArgProt PrintToClient;

ValueList leakedResourcesSaveRestoreSystem;
ValueList leakedResourcesEdtSystem;
ValueList antiCycleListDoors;
ValueList entityDeleteList;
ValueList playerDeathQueue;
ValueList collisionList;
ValueList viewcontrolresetlist;
ValueList saved_triggers;
ValueList new_player_join_ref;
PlayerSaveList playerSaveList;

char global_map[1024];
char last_map[1024];
char next_map[1024];
bool transition;
bool savegame;
bool savegame_lock;
bool restoring;
bool protect_player;
bool restore_delay;
bool restore_delay_lock;
bool hasSavedOnce;
bool reset_viewcontrol;
bool sdktools_passed;
bool saving_game_rightnow;
int save_frames;
int restore_frames;
int after_restore_frames;
int game_start_frames;
int car_delay_for_save;
bool removing_ents_restore;
int restore_start_delay;
uint32_t fake_sequence_mem;
bool player_restore_failed;

void* delete_operator_array_addr;
void* delete_operator_addr;
void* new_operator_addr;
void* new_operator_array_addr;
void* strcpy_chk_addr;

pthread_mutex_t playerDeathQueueLock;
pthread_mutex_t collisionListLock;

uint32_t sv;
uint32_t g_ModelLoader;
uint32_t g_DataCache;
uint32_t g_MDLCache;
uint32_t s_ServerPlugin;
uint32_t SaveRestoreGlobal;
uint32_t weapon_substitute;

pOneArgProt UTIL_Remove__External;
pTwoArgProt CreateEntityByNameHook__External;
pOneArgProt CleanupDeleteListHook__External;
pThreeArgProt PlayerSpawnHook__External;
pOneArgProt UTIL_RemoveInternal__External;
pThreeArgProt MainPlayerRestore__External;

void InitCoreSynergy()
{
    our_libraries[0] = (uint32_t)malloc(1024);
    snprintf((char*)our_libraries[0], 1024, "%s", "/synergy/bin/server_srv.so");

    our_libraries[1] = (uint32_t)malloc(1024);
    snprintf((char*)our_libraries[1], 1024, "%s", "/bin/engine_srv.so");

    our_libraries[2] = (uint32_t)malloc(1024);
    snprintf((char*)our_libraries[2], 1024, "%s", "/bin/scenefilecache.so");

    our_libraries[3] = (uint32_t)malloc(1024);
    snprintf((char*)our_libraries[3], 1024, "%s", "/bin/soundemittersystem.so");

    our_libraries[4] = (uint32_t)malloc(1024);
    snprintf((char*)our_libraries[4], 1024, "%s", "/bin/dedicated_srv.so");

    our_libraries[5] = (uint32_t)malloc(1024);
    snprintf((char*)our_libraries[5], 1024, "%s", "/bin/soundemittersystem_srv.so");

    our_libraries[6] = (uint32_t)malloc(1024);
    snprintf((char*)our_libraries[6], 1024, "%s", "/bin/materialsystem_srv.so");

    our_libraries[7] = (uint32_t)malloc(1024);
    snprintf((char*)our_libraries[7], 1024, "%s", "/bin/studiorender_srv.so");

    our_libraries[8] = (uint32_t)malloc(1024);
    snprintf((char*)our_libraries[8], 1024, "%s", "/bin/datacache_srv.so");

    our_libraries[9] = (uint32_t)malloc(1024);
    snprintf((char*)our_libraries[9], 1024, "%s", "/bin/vphysics_srv.so");

    our_libraries[10] = (uint32_t)malloc(1024);
    snprintf((char*)our_libraries[10], 1024, "%s", "/extensions/sdktools.ext.2.sdk2013.so");
}

void AttemptToRestoreGame()
{
    pZeroArgProt pDynamicZeroArgFunc;

    if(restore_start_delay == 200)
    {   
        //RestoreGame
        //pDynamicZeroArgFunc = (pZeroArgProt)(server_srv + 0x00AF2A10);
        //pDynamicZeroArgFunc();
    }
}

bool IsAllowedToPatchSdkTools(uint32_t lib_base, uint32_t lib_size)
{
    uint32_t lib_integrity_chk_addr = lib_base + 0x00057919;
    uint32_t str_len = 11;

    bool integrity_chk = (lib_integrity_chk_addr + str_len) <= (lib_base + lib_size);

    if(integrity_chk)
    {
        char* ext_ver = (char*)lib_integrity_chk_addr;

        if(strcmp(ext_ver, "1.10.0.6503") == 0)
        {
            rootconsole->ConsolePrint("\nSDKTools Memory Integrity Passed!\n");
            return true;
        }
    }

    return false;
}

void PopulateHookExclusionListsSynergy()
{
    
}

uint32_t GetCBaseEntitySynergy(uint32_t EHandle)
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

void catoutputs(char* destination, char* source)
{
    if(destination && source)
    {
        strcat(destination, ",");
        strcat(destination, source);
    }
    else if(destination)
    {
        strcat(destination, ",");
    }
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

            if((fieldType == FIELD_CUSTOM) && (flags & FTYPEDESC_OUTPUT))
            {
                if(strcmp(fieldName, (char*)searchField) == 0)
                {
                    uint32_t val = *(uint32_t*)(firstEnt+offset+0x14);
                    ValueList outputs_list = AllocateValuesList();

                    while(val != 0)
                    {
                        char* firstVar  =  (char*) ( *(uint32_t*)(val)      ) ;
                        char* secondVar =  (char*) ( *(uint32_t*)(val+0x04) );
                        char* thirdVar  =  (char*) ( *(uint32_t*)(val+0x08) );
                        float fourthVar    =  *(float*)(val+0x0C);
                        int fifthVar       =  *(int*)(val+0x10);
                        int sixthVar       =  *(int*)(val+0x14);

                        //rootconsole->ConsolePrint("[FIELD_CUSTOM] [%s] [%s] [%s] [%s] [%s] [%f] [%d] [%d]", fieldName, externalName, firstVar, secondVar, thirdVar, fourthVar, fifthVar, sixthVar);

                        char* output_ready = (char*)malloc(4096);
                        strncpy(output_ready, firstVar, 4096);
                        
                        catoutputs(output_ready, secondVar);
                        catoutputs(output_ready, thirdVar);

                        char fourth_to_string[512];
                        snprintf(fourth_to_string, 512, "%f", fourthVar);
                        catoutputs(output_ready, fourth_to_string);

                        char fifth_to_string[512];
                        snprintf(fifth_to_string, 512, "%d", fifthVar);
                        catoutputs(output_ready, fifth_to_string);

                        Value* output = CreateNewValue((void*)output_ready);
                        InsertToValuesList(outputs_list, output, NULL, false, false);

                        val = *(uint32_t*)(val+0x18);
                    }

                    return (uint32_t)outputs_list;
                }
            }
            else if((flags & (FTYPEDESC_GLOBAL)) || (flags & (FTYPEDESC_SAVE)) || (flags & (FTYPEDESC_KEY)) || (flags & (FTYPEDESC_INPUT)))
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
                    InsertToValuesList(vals, val1, NULL, false, false);

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
                    InsertToValuesList(vals, val1, NULL, false, false);

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
                    InsertToValuesList(vals, val3, NULL, false, false);
                    InsertToValuesList(vals, val2, NULL, false, false);
                    InsertToValuesList(vals, val1, NULL, false, false);

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
                    InsertToValuesList(vals, val1, NULL, false, false);

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
                    InsertToValuesList(vals, val1, NULL, false, false);

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

void SendEntityInput(uint32_t ref_handle, uint32_t inputName, uint32_t activator, uint32_t caller, uint32_t val, uint32_t outputId)
{
    uint32_t entity = GetCBaseEntitySynergy(ref_handle);

    if(entity == 0)
    {
        rootconsole->ConsolePrint(EXT_PREFIX "Failed to send entity input - entity does not exist!");
        return;
    }
    
    //rootconsole->ConsolePrint("Sending entity input: [%s] to entity", inputName);
    pSixArgProt pDynamicSixArgFunc = (pSixArgProt)(*(uint32_t*)((*(uint32_t*)entity)+0x94));
    pDynamicSixArgFunc(entity, inputName, activator, caller, (uint32_t)&val, outputId);
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
    leakedResourcesVpkSystem = AllocateValuesList();
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
        InsertToValuesList(currentRestoreList, leak, NULL, false, true);
    }

    free(file_line);
    fclose(list_file);
}

int ReleaseLeakedMemory(ValueList leakList, bool destroy, uint32_t current_cap, uint32_t allowed_cap, uint32_t free_perc)
{
    if(!leakList)
        return 0;
    
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
            return 0;
        }

        rootconsole->ConsolePrint("[%s] Attempted to free leaks from an empty leaked resources list!", listName);
        return 0;
    }

    if(destroy)
    {
        //Save references to be freed if extension is reloaded
        SaveLinkedList(leakList);
    }

    if((current_cap < allowed_cap) && !destroy)
        return 0;

    int total_items = ValueListItems(leakList, NULL);
    int free_total_items = (float)free_perc / 100.0 * total_items;
    int has_freed_items = 0;

    while(leak)
    {
        Value* detachedValue = leak->nextVal;

        if(!destroy)
        {
            //rootconsole->ConsolePrint("[%s] FREED MEMORY LEAK WITH REF: [%X]", listName, leak->value);
            free(leak->value);
            has_freed_items++;
        }

        free(leak);
        leak = detachedValue;
        
        if((has_freed_items >= free_total_items) && !destroy)
        {
            //Re-chain the list to point to the first valid value!
            *leakList = leak;
            break;
        }
    }

    rootconsole->ConsolePrint("FREED [%d] memory allocations", has_freed_items);

    if(destroy)
    {
        free(leakList);
        leakList = NULL;
    }

    return has_freed_items;
}

void DestroyLinkedLists()
{
    ReleaseLeakedMemory(leakedResourcesSaveRestoreSystem, true, 0, 0, 100);
    ReleaseLeakedMemory(leakedResourcesVpkSystem, true, 0, 0, 100);
    ReleaseLeakedMemory(leakedResourcesEdtSystem, true, 0, 0, 100);

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

SavedEntity* SaveEntity(uint32_t firstEnt)
{
    if(firstEnt == 0)
        return 0;

    uint32_t m_RefEHandle = *(uint32_t*)(firstEnt+0x350);
    uint32_t classlabel = *(uint32_t*)(firstEnt+0x68);

    rootconsole->ConsolePrint("saving entity with classname: [%s]", classlabel);

    pOneArgProt pDynamicOneArgFunc = (pOneArgProt)(*(uint32_t*)((*(uint32_t*)firstEnt)+0x30));
    uint32_t dmap = pDynamicOneArgFunc(firstEnt);

    FieldList fieldList = AllocateFieldList();
    void* classlabelCpy = copy_val((void*)classlabel, strlen((char*)classlabel)+1);
    return CreateNewSavedEntity((void*)m_RefEHandle, classlabelCpy, SaveEntityFields(dmap, firstEnt, 0, 0, fieldList));
}

void EnableViewControl(uint32_t viewControl)
{
    uint32_t m_refHandle = *(uint32_t*)(viewControl+0x350);
    SendEntityInput(m_refHandle, (uint32_t)"Enable", 0, 0, 0, (uint32_t)-1);
}

void DisableViewControls()
{
    uint32_t viewControl = 0;

    while((viewControl = functions.FindEntityByClassname(CGlobalEntityList, viewControl, (uint32_t)"point_viewcontrol")) != 0)
    {
        uint32_t m_refHandle = *(uint32_t*)(viewControl+0x350);
        
        pOneArgProt pDynamicOneArgFunc = (pOneArgProt)(*(uint32_t*)((*(uint32_t*)viewControl)+0x30));
        uint32_t dmap = pDynamicOneArgFunc(viewControl);

        uint32_t m_state = GetEntityField(dmap, viewControl, 0, 0, (uint32_t)"m_state");

        if(m_state && *(uint8_t*)(m_state) == 1)
        {
            SendEntityInput(m_refHandle, (uint32_t)"Disable", 0, 0, 0, (uint32_t)-1);
        }
    }
}

int GetTotalClientsInGame()
{
    pOneArgProt pDynamicOneArgFunc;
    
    //EntityGetFunctionByEdict
    pDynamicOneArgFunc = (pOneArgProt)(server_srv + 0x00B646A0);
    uint32_t global_var = *(uint32_t*)(server_srv + 0x010121E0);
    uint32_t maxPlayers = *(uint32_t*)(global_var+0x14);

    //rootconsole->ConsolePrint("maxPlayers: %d", maxPlayers);
    int total_clients = 0;

    for(uint32_t i = 1; i <= maxPlayers; i++)
    {
        uint32_t player_object = pDynamicOneArgFunc(i);

        if(player_object)
        {
            total_clients++;
        }
    }

    return total_clients;
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

            DeleteAllValuesInList(field->fieldVals, true, NULL);
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

void UpdateOnRemoveDelayedEntities()
{
    Value* entityRef = *entityDeleteList;

    while(entityRef)
    {
        Value* nextNode = entityRef->nextVal;
        uint32_t object = GetCBaseEntitySynergy((uint32_t)entityRef->value);

        if(object)
        {
            pOneArgProt UpdateOnRemove = (pOneArgProt)(*(uint32_t*)((*(uint32_t*)(object))+0x1A8));
            UpdateOnRemove(object);
        }

        free(entityRef);
        entityRef = nextNode;
    }

    *entityDeleteList = NULL;
}

void SaveTriggersDatamaps()
{
    pOneArgProt pDynamicOneArgFunc;
    uint32_t ent = 0;

    while((ent = functions.FindEntityByClassname(CGlobalEntityList, ent, (uint32_t)"*")) != 0)
    {
        char* clsname = (char*)(*(uint32_t*)(ent+0x68));
        char* targetname_current = (char*)  *(uint32_t*)(ent+0x124);
        uint32_t RefHandle = *(uint32_t*)(ent+0x350);

        if(  !( strcmp(clsname, "trigger_once") == 0 || strcmp(clsname, "trigger_multiple") == 0 )  ) continue;

        pDynamicOneArgFunc = (pOneArgProt)(*(uint32_t*)((*(uint32_t*)ent)+0x30));
        uint32_t dmap = pDynamicOneArgFunc(ent);
        
        uint32_t edt_mins = GetEntityField(dmap, ent, 0, 0, (uint32_t)"m_EDT_vecMins");
        uint32_t edt_maxs = GetEntityField(dmap, ent, 0, 0, (uint32_t)"m_EDT_vecMaxs");
        char** parentname = (char**)GetEntityField(dmap, ent, 0, 0, (uint32_t)"m_iParent");

        if(!edt_mins || !edt_maxs) continue;
        else if(edt_mins && edt_maxs)
        {
            float x_mins = *(float*)(edt_mins);
            float y_mins = *(float*)(edt_mins+0x4);
            float z_mins = *(float*)(edt_mins+0x8);
            
            float x_maxs = *(float*)(edt_maxs);
            float y_maxs = *(float*)(edt_maxs+0x4);
            float z_maxs = *(float*)(edt_maxs+0x8);

            if
            (
            x_mins == 0 && y_mins == 0 && z_mins == 0 &&
            x_maxs == 0 && y_maxs == 0 && z_maxs == 0
            )
            continue;
        }

        rootconsole->ConsolePrint("Saving [%s]", clsname);

        ValueList kvs = AllocateValuesList();

        char** classname = (char**)GetEntityField(dmap, ent, 0, 0, (uint32_t)"m_iClassname");

        if(classname)
        {
            EntityKV* kv = (EntityKV*)(malloc(sizeof(EntityKV)));

            char* ent_key = (char*)malloc(sizeof("classname"));
            snprintf(ent_key, sizeof("classname"), "classname");

            void* ent_val = copy_val(*classname, strlen(*classname)+1);

            kv->key = (uint32_t)ent_key;
            kv->value = (uint32_t)ent_val;

            Value* kv_val = CreateNewValue((void*)kv);
            InsertToValuesList(kvs, kv_val, NULL, true, false);
        }



        uint32_t origin = 0;

        if(parentname && *parentname && strcmp(*parentname, "") != 0)
        {
            origin = GetEntityField(dmap, ent, 0, 0, (uint32_t)"m_vecAbsOrigin");

            EntityKV* kv = (EntityKV*)(malloc(sizeof(EntityKV)));

            char* ent_key = (char*)malloc(sizeof("parentname"));
            snprintf(ent_key, sizeof("parentname"), "parentname");

            void* ent_val = copy_val(*parentname, strlen(*parentname)+1);

            kv->key = (uint32_t)ent_key;
            kv->value = (uint32_t)ent_val;

            Value* kv_val = CreateNewValue((void*)kv);
            InsertToValuesList(kvs, kv_val, NULL, true, false);
        }
        else
        {
            origin = GetEntityField(dmap, ent, 0, 0, (uint32_t)"m_vecOrigin");
        }

        if(origin)
        {
            EntityKV* kv = (EntityKV*)(malloc(sizeof(EntityKV)));

            char* ent_key = (char*)malloc(sizeof("origin"));
            snprintf(ent_key, sizeof("origin"), "origin");

            float x = *(float*)(origin);
            float y = *(float*)(origin+0x4);
            float z = *(float*)(origin+0x8);

            char* ent_val = (char*)malloc(2048);
            snprintf(ent_val, 2048, "%f %f %f", x, y, z);

            kv->key = (uint32_t)ent_key;
            kv->value = (uint32_t)ent_val;

            Value* kv_val = CreateNewValue((void*)kv);
            InsertToValuesList(kvs, kv_val, NULL, true, false);
        }

        if(edt_mins)
        {
            EntityKV* kv = (EntityKV*)(malloc(sizeof(EntityKV)));

            char* ent_key = (char*)malloc(sizeof("edt_mins"));
            snprintf(ent_key, sizeof("edt_mins"), "edt_mins");

            float x = *(float*)(edt_mins);
            float y = *(float*)(edt_mins+0x4);
            float z = *(float*)(edt_mins+0x8);

            char* ent_val = (char*)malloc(2048);
            snprintf(ent_val, 2048, "%f %f %f", x, y, z);

            kv->key = (uint32_t)ent_key;
            kv->value = (uint32_t)ent_val;

            Value* kv_val = CreateNewValue((void*)kv);
            InsertToValuesList(kvs, kv_val, NULL, true, false);
        }

        if(edt_maxs)
        {
            EntityKV* kv = (EntityKV*)(malloc(sizeof(EntityKV)));

            char* ent_key = (char*)malloc(sizeof("edt_maxs"));
            snprintf(ent_key, sizeof("edt_maxs"), "edt_maxs");

            float x = *(float*)(edt_maxs);
            float y = *(float*)(edt_maxs+0x4);
            float z = *(float*)(edt_maxs+0x8);

            char* ent_val = (char*)malloc(2048);
            snprintf(ent_val, 2048, "%f %f %f", x, y, z);

            kv->key = (uint32_t)ent_key;
            kv->value = (uint32_t)ent_val;

            Value* kv_val = CreateNewValue((void*)kv);
            InsertToValuesList(kvs, kv_val, NULL, true, false);
        }



        uint32_t spawnflags = GetEntityField(dmap, ent, 0, 0, (uint32_t)"m_spawnflags");

        if(spawnflags)
        {
            EntityKV* kv = (EntityKV*)(malloc(sizeof(EntityKV)));

            char* ent_key = (char*)malloc(sizeof("spawnflags"));
            snprintf(ent_key, sizeof("spawnflags"), "spawnflags");

            uint32_t spawnflags_val = *(uint32_t*)(spawnflags);
            char* to_string_val = (char*)malloc(32);
            snprintf(to_string_val, 32, "%d", spawnflags_val);

            kv->key = (uint32_t)ent_key;
            kv->value = (uint32_t)to_string_val;

            Value* kv_val = CreateNewValue((void*)kv);
            InsertToValuesList(kvs, kv_val, NULL, true, false);
        }

        uint32_t startDisabled = GetEntityField(dmap, ent, 0, 0, (uint32_t)"m_bDisabled");

        if(startDisabled)
        {
            EntityKV* kv = (EntityKV*)(malloc(sizeof(EntityKV)));

            char* ent_key = (char*)malloc(sizeof("startdisabled"));
            snprintf(ent_key, sizeof("startdisabled"), "startdisabled");

            uint8_t val = *(uint8_t*)(startDisabled);
            char* to_string_val = (char*)malloc(32);
            snprintf(to_string_val, 32, "%d", val);

            kv->key = (uint32_t)ent_key;
            kv->value = (uint32_t)to_string_val;

            Value* kv_val = CreateNewValue((void*)kv);
            InsertToValuesList(kvs, kv_val, NULL, true, false);
        }



        char** targetname = (char**)GetEntityField(dmap, ent, 0, 0, (uint32_t)"m_iName");

        if(targetname)
        {
            if(*targetname)
            {
                EntityKV* kv = (EntityKV*)(malloc(sizeof(EntityKV)));

                char* ent_key = (char*)malloc(sizeof("targetname"));
                snprintf(ent_key, sizeof("targetname"), "targetname");

                void* ent_val = copy_val(*targetname, strlen(*targetname)+1);

                kv->key = (uint32_t)ent_key;
                kv->value = (uint32_t)ent_val;

                Value* kv_val = CreateNewValue((void*)kv);
                InsertToValuesList(kvs, kv_val, NULL, true, false);
            }
        }

        char** filterName = (char**)GetEntityField(dmap, ent, 0, 0, (uint32_t)"m_iFilterName");

        if(filterName)
        {
            if(*filterName)
            {
                EntityKV* kv = (EntityKV*)(malloc(sizeof(EntityKV)));

                char* ent_key = (char*)malloc(sizeof("filtername"));
                snprintf(ent_key, sizeof("filtername"), "filtername");

                void* ent_val = copy_val(*filterName, strlen(*filterName)+1);

                kv->key = (uint32_t)ent_key;
                kv->value = (uint32_t)ent_val;

                Value* kv_val = CreateNewValue((void*)kv);
                InsertToValuesList(kvs, kv_val, NULL, true, false);
            }
        }

        char** globalname = (char**)GetEntityField(dmap, ent, 0, 0, (uint32_t)"m_iGlobalname");

        if(globalname)
        {
            if(*globalname)
            {
                EntityKV* kv = (EntityKV*)(malloc(sizeof(EntityKV)));

                char* ent_key = (char*)malloc(sizeof("globalname"));
                snprintf(ent_key, sizeof("globalname"), "globalname");

                void* ent_val = copy_val(*globalname, strlen(*globalname)+1);

                kv->key = (uint32_t)ent_key;
                kv->value = (uint32_t)ent_val;

                Value* kv_val = CreateNewValue((void*)kv);
                InsertToValuesList(kvs, kv_val, NULL, true, false);
            }
        }



        uint32_t trigger_outputs = GetEntityField(dmap, ent, 0, 0, (uint32_t)"m_OnTrigger");

        if(trigger_outputs)
        {
            ValueList outputs_list = (ValueList)trigger_outputs;
            Value* output = *outputs_list;

            while(output)
            {
                Value* nextOutput = output->nextVal;
                EntityKV* kv = (EntityKV*)(malloc(sizeof(EntityKV)));

                char* ent_key = (char*)malloc(sizeof("OnTrigger"));
                snprintf(ent_key, sizeof("OnTrigger"), "OnTrigger");

                kv->key = (uint32_t)ent_key;
                kv->value = (uint32_t)output->value;

                Value* kv_val = CreateNewValue((void*)kv);
                InsertToValuesList(kvs, kv_val, NULL, true, false);

                free(output);
                output = nextOutput;
            }

            free((void*)trigger_outputs);
        }

        uint32_t starttouch_outputs = GetEntityField(dmap, ent, 0, 0, (uint32_t)"m_OnStartTouch");

        if(starttouch_outputs)
        {
            ValueList outputs_list = (ValueList)starttouch_outputs;
            Value* output = *outputs_list;

            while(output)
            {
                Value* nextOutput = output->nextVal;
                EntityKV* kv = (EntityKV*)(malloc(sizeof(EntityKV)));

                char* ent_key = (char*)malloc(sizeof("OnStartTouch"));
                snprintf(ent_key, sizeof("OnStartTouch"), "OnStartTouch");

                kv->key = (uint32_t)ent_key;
                kv->value = (uint32_t)output->value;

                Value* kv_val = CreateNewValue((void*)kv);
                InsertToValuesList(kvs, kv_val, NULL, true, false);

                free(output);
                output = nextOutput;
            }

            free((void*)starttouch_outputs);
        }

        uint32_t starttouchall_outputs = GetEntityField(dmap, ent, 0, 0, (uint32_t)"m_OnStartTouchAll");

        if(starttouchall_outputs)
        {
            ValueList outputs_list = (ValueList)starttouchall_outputs;
            Value* output = *outputs_list;

            while(output)
            {
                Value* nextOutput = output->nextVal;
                EntityKV* kv = (EntityKV*)(malloc(sizeof(EntityKV)));

                char* ent_key = (char*)malloc(sizeof("OnStartTouchAll"));
                snprintf(ent_key, sizeof("OnStartTouchAll"), "OnStartTouchAll");

                kv->key = (uint32_t)ent_key;
                kv->value = (uint32_t)output->value;

                Value* kv_val = CreateNewValue((void*)kv);
                InsertToValuesList(kvs, kv_val, NULL, true, false);

                free(output);
                output = nextOutput;
            }

            free((void*)starttouchall_outputs);
        }

        uint32_t endtouch_outputs = GetEntityField(dmap, ent, 0, 0, (uint32_t)"m_OnEndTouch");

        if(endtouch_outputs)
        {
            ValueList outputs_list = (ValueList)endtouch_outputs;
            Value* output = *outputs_list;

            while(output)
            {
                Value* nextOutput = output->nextVal;
                EntityKV* kv = (EntityKV*)(malloc(sizeof(EntityKV)));

                char* ent_key = (char*)malloc(sizeof("OnEndTouch"));
                snprintf(ent_key, sizeof("OnEndTouch"), "OnEndTouch");

                kv->key = (uint32_t)ent_key;
                kv->value = (uint32_t)output->value;

                Value* kv_val = CreateNewValue((void*)kv);
                InsertToValuesList(kvs, kv_val, NULL, true, false);

                free(output);
                output = nextOutput;
            }

            free((void*)endtouch_outputs);
        }

        uint32_t endtouchall_outputs = GetEntityField(dmap, ent, 0, 0, (uint32_t)"m_OnEndTouchAll");

        if(endtouchall_outputs)
        {
            ValueList outputs_list = (ValueList)endtouchall_outputs;
            Value* output = *outputs_list;

            while(output)
            {
                Value* nextOutput = output->nextVal;
                EntityKV* kv = (EntityKV*)(malloc(sizeof(EntityKV)));

                char* ent_key = (char*)malloc(sizeof("OnEndTouchAll"));
                snprintf(ent_key, sizeof("OnEndTouchAll"), "OnEndTouchAll");

                kv->key = (uint32_t)ent_key;
                kv->value = (uint32_t)output->value;

                Value* kv_val = CreateNewValue((void*)kv);
                InsertToValuesList(kvs, kv_val, NULL, true, false);

                free(output);
                output = nextOutput;
            }

            free((void*)endtouchall_outputs);
        }

        uint32_t touching_outputs = GetEntityField(dmap, ent, 0, 0, (uint32_t)"m_OnTouching");

        if(touching_outputs)
        {
            ValueList outputs_list = (ValueList)touching_outputs;
            Value* output = *outputs_list;

            while(output)
            {
                Value* nextOutput = output->nextVal;
                EntityKV* kv = (EntityKV*)(malloc(sizeof(EntityKV)));

                char* ent_key = (char*)malloc(sizeof("OnTouching"));
                snprintf(ent_key, sizeof("OnTouching"), "OnTouching");

                kv->key = (uint32_t)ent_key;
                kv->value = (uint32_t)output->value;

                Value* kv_val = CreateNewValue((void*)kv);
                InsertToValuesList(kvs, kv_val, NULL, true, false);

                free(output);
                output = nextOutput;
            }

            free((void*)touching_outputs);
        }

        uint32_t nottouching_outputs = GetEntityField(dmap, ent, 0, 0, (uint32_t)"m_OnNotTouching");

        if(nottouching_outputs)
        {
            ValueList outputs_list = (ValueList)nottouching_outputs;
            Value* output = *outputs_list;

            while(output)
            {
                Value* nextOutput = output->nextVal;
                EntityKV* kv = (EntityKV*)(malloc(sizeof(EntityKV)));

                char* ent_key = (char*)malloc(sizeof("OnNotTouching"));
                snprintf(ent_key, sizeof("OnNotTouching"), "OnNotTouching");

                kv->key = (uint32_t)ent_key;
                kv->value = (uint32_t)output->value;

                Value* kv_val = CreateNewValue((void*)kv);
                InsertToValuesList(kvs, kv_val, NULL, true, false);

                free(output);
                output = nextOutput;
            }

            free((void*)nottouching_outputs);
        }

        Value* new_saved_trigger = CreateNewValue((void*)kvs);
        InsertToValuesList(saved_triggers, new_saved_trigger, NULL, false, false);

        //Delete now because it will restore!
        UTIL_Remove__External(ent);
    }
}

bool IsSynergyMemoryCorrect()
{
    if
    (
    *(uint8_t*)((server_srv + 0x00544C51)+0) == 0xF3    && 
    *(uint8_t*)((server_srv + 0x00544C51)+1) == 0x0F    && 
    *(uint8_t*)((server_srv + 0x00544C51)+2) == 0x10    && 
    *(uint8_t*)((server_srv + 0x00544C51)+3) == 0x05    && 

    *(uint32_t*)((server_srv + 0x00544C51)+4) == (server_srv + 0x00C3111C) &&

    *(uint8_t*)((server_srv + 0x00440693)+0) == 0xF3   &&
    *(uint8_t*)((server_srv + 0x00440693)+1) == 0x0F   &&
    *(uint8_t*)((server_srv + 0x00440693)+2) == 0x10   &&
    *(uint8_t*)((server_srv + 0x00440693)+3) == 0x05   &&

    *(uint32_t*)((server_srv + 0x00440693)+4) == (server_srv + 0x00C3111C)
    )
    {
        return true;
    }

    return false;
}

void ForceSynergyMemoryCorrection()
{
    AllowWriteToMappedMemory();

    //FORCE MEMORY
    *(uint8_t*)((server_srv + 0x00544C51)+0) = 0xF3;
    *(uint8_t*)((server_srv + 0x00544C51)+1) = 0x0F;
    *(uint8_t*)((server_srv + 0x00544C51)+2) = 0x10;
    *(uint8_t*)((server_srv + 0x00544C51)+3) = 0x05;
    *(uint32_t*)((server_srv + 0x00544C51)+4) = (server_srv + 0x00C3111C);

    *(uint8_t*)((server_srv + 0x00440693)+0) = 0xF3;
    *(uint8_t*)((server_srv + 0x00440693)+1) = 0x0F;
    *(uint8_t*)((server_srv + 0x00440693)+2) = 0x10;
    *(uint8_t*)((server_srv + 0x00440693)+3) = 0x05;
    *(uint32_t*)((server_srv + 0x00440693)+4) = (server_srv + 0x00C3111C);

    RestoreMemoryProtections();
}

void ReleaseSavedTriggers()
{
    rootconsole->ConsolePrint("Releasing saved triggers...");
    
    Value* saved_trigger = *saved_triggers;

    while(saved_trigger)
    {
        Value* next_saved_trigger = saved_trigger->nextVal;
        ValueList kvs = (ValueList)(saved_trigger->value);
        Value* kv = *kvs;

        while(kv)
        {
            Value* nextKv = kv->nextVal;
            EntityKV* struct_keyvalue = (EntityKV*)(kv->value);

            free((void*)struct_keyvalue->key);
            free((void*)struct_keyvalue->value);

            free(kv->value);
            free(kv);
            kv = nextKv;
        }

        free(kvs);
        free(saved_trigger);
        saved_trigger = next_saved_trigger;
    }

    *saved_triggers = NULL;
}

void InstaKillSynergy(uint32_t entity_object, bool validate)
{
    pZeroArgProt pDynamicZeroArgFunc;
    pOneArgProt pDynamicOneArgFunc;
    pTwoArgProt pDynamicTwoArgFunc;

    if(entity_object == 0) return;

    uint32_t refHandleInsta = *(uint32_t*)(entity_object+0x350);
    char* classname = (char*)( *(uint32_t*)(entity_object+0x68));
    uint32_t cbase_chk = GetCBaseEntitySynergy(refHandleInsta);

    if(cbase_chk == 0)
    {
        if(!validate)
        {
            if(classname)
            {
                rootconsole->ConsolePrint("Warning: Entity delete request granted without validation! [%s]", classname);
            }
            else
            {
                rootconsole->ConsolePrint("Warning: Entity delete request granted without validation!");
            }

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
    pDynamicOneArgFunc = (pOneArgProt)(server_srv + 0x00AC7EF0);
    uint32_t isMarked = pDynamicOneArgFunc(cbase_chk+0x18);

    if(isMarked)
    {
        rootconsole->ConsolePrint("Attempted to kill an entity twice in UTIL_RemoveImmediate(CBaseEntity*)");
        return;
    }

    //PhysIsInCallback
    pDynamicOneArgFunc = (pOneArgProt)(server_srv + 0x00A1A500);
    uint32_t isInCallback = pDynamicOneArgFunc(0);

    if(isInCallback)
    {
        rootconsole->ConsolePrint("Should not be! (Insta)");
        exit(EXIT_FAILURE);
        
        //CCollisionEvent - AddRemoveObject
        //pDynamicTwoArgFunc = (pTwoArgProt)(server_srv + 0x00A1EDA0);
        //pDynamicTwoArgFunc(server_srv + 0x01032900, cbase_chk+0x18);
        return;
    }
    
    if(isTicking)
    {
        rootconsole->ConsolePrint("fast killed [%s]", classname);
        
        char printMsg[512];
        snprintf(printMsg, 512, "fast killed [%s] [%X]", classname, (uint32_t)__builtin_return_address(0) - server_srv);

        uint32_t player = 0;

        while((player = functions.FindEntityByClassname(CGlobalEntityList, player, (uint32_t)"player")) != 0)
        {
            PrintToClient(player, 2, (uint32_t)printMsg, 0, 0, 0, 0);
        }
    }

    if((*(uint32_t*)(cbase_chk+0x11C) & 1) == 0)
    {
        if(*(uint32_t*)(server_srv + 0x01086820) == 0)
        {
            // FAST DELETE ONLY

            hooked_delete_counter++;

            //VphysicsDestroyObject
            //pDynamicOneArgFunc = (pOneArgProt)( *(uint32_t*)((*(uint32_t*)(cbase_chk))+0x274) );
            //pDynamicOneArgFunc(cbase_chk);

            *(uint8_t*)(server_srv + 0x01084244) = 0;
            *(uint32_t*)(cbase_chk+0x11C) = *(uint32_t*)(cbase_chk+0x11C) | 1;

            //UpdateOnRemove
            pDynamicOneArgFunc = (pOneArgProt)(  *(uint32_t*)((*(uint32_t*)(cbase_chk))+0x1A8) );
            pDynamicOneArgFunc(cbase_chk);

            *(uint8_t*)(server_srv + 0x01084245) = 1;

            //CALL RELEASE
            uint32_t iServerObj = cbase_chk+0x18;

            pDynamicOneArgFunc = (pOneArgProt)(  *(uint32_t*)((*(uint32_t*)(iServerObj))+0x10) );
            pDynamicOneArgFunc(iServerObj);

            *(uint8_t*)(server_srv + 0x01084245) = 0;
        }
        else
        {
            RemoveEntityNormalSynergy(cbase_chk, validate);
        }
    }
}

void RemoveEntityNormalSynergy(uint32_t entity_object, bool validate)
{
    pOneArgProt pDynamicOneArgFunc;
    
    if(entity_object == 0)
    {
        rootconsole->ConsolePrint("Could not kill entity [NULL]");
        return;
    }

    char* classname = (char*)(*(uint32_t*)(entity_object+0x68));
    uint32_t m_refHandle = *(uint32_t*)(entity_object+0x350);
    uint32_t chk_ref = GetCBaseEntitySynergy(m_refHandle);

    if(chk_ref == 0)
    {
        if(!validate)
        {
            if(classname)
            {
                rootconsole->ConsolePrint("Warning: Entity delete request granted without validation! [%s]", classname);
            }
            else
            {
                rootconsole->ConsolePrint("Warning: Entity delete request granted without validation!");
            }
            
            chk_ref = entity_object;
        }
    }

    if(chk_ref)
    {
        //IsMarkedForDeletion
        pDynamicOneArgFunc = (pOneArgProt)(server_srv + 0x00AC7EF0);
        uint32_t isMarked = pDynamicOneArgFunc(chk_ref+0x18);

        if(isMarked)
        {
            //rootconsole->ConsolePrint("Attempted to kill an entity twice in UTIL_Remove(CBaseEntity*)");
            return;
        }

        if(strcmp(classname, "player") == 0)
        {
            rootconsole->ConsolePrint(EXT_PREFIX "Tried killing player but was protected!");
            return;
        }

        if(saving_game_rightnow)
        {
            rootconsole->ConsolePrint("Attempted to kill an entity while making a save file! [%s] [%X]", classname, (uint32_t)__builtin_return_address(0) - server_srv);
            exit(EXIT_FAILURE);
        }

        UTIL_RemoveInternal__External(chk_ref+0x18);
    }
    else
    {
        rootconsole->ConsolePrint(EXT_PREFIX "Could not kill entity [Invalid Ehandle]");
        exit(EXIT_FAILURE);
    }
}

void RebuildSavedTriggers()
{
    pOneArgProt pDynamicOneArgFunc;
    pTwoArgProt pDynamicTwoArgFunc;
    pThreeArgProt pDynamicThreeArgFunc;
    uint32_t main_engine_global = *(uint32_t*)(server_srv + 0x00109A3E0);

    pDynamicOneArgFunc = (pOneArgProt)(  *(uint32_t*)( (*(uint32_t*)(main_engine_global))+0x64 )  );
    pDynamicOneArgFunc(main_engine_global);

    Value* saved_trigger = *saved_triggers;

    while(saved_trigger)
    {
        uint32_t newTrigger = 0;
        Vector* world_origin = NULL;
        uint32_t parentName = 0;

        //IsPrecacheAllowed
        pDynamicOneArgFunc = (pOneArgProt)(server_srv + 0x003D7610);
        uint32_t origVal = pDynamicOneArgFunc(0);

        //SetAllowPrecache
        pDynamicOneArgFunc = (pOneArgProt)(server_srv + 0x003D7620);
        pDynamicOneArgFunc(1);

        ValueList kvs = (ValueList)saved_trigger->value;
        Value* kv_node = *kvs;

        if(kv_node)
        {
            EntityKV* kv = (EntityKV*)(kv_node->value);

            if(strcmp((char*)kv->key, "classname") == 0)
            {
                rootconsole->ConsolePrint("Restoring [%s]", kv->value);
                newTrigger = CreateEntityByNameHook__External((uint32_t)kv->value, (uint32_t)-1);

                //Precache
                pDynamicOneArgFunc = (pOneArgProt)(  *(uint32_t*)((*(uint32_t*)(newTrigger))+0x60)  );
                pDynamicOneArgFunc(newTrigger);
            }
        }

        while(kv_node->nextVal && newTrigger)
        {
            EntityKV* kv = (EntityKV*)(kv_node->nextVal->value);
            rootconsole->ConsolePrint("[%s] [%s]", kv->key, kv->value);

            //DispatchKeyValue
            pDynamicThreeArgFunc = (pThreeArgProt)(*(uint32_t*)((*(uint32_t*)newTrigger)+0x78));
            pDynamicThreeArgFunc(newTrigger, (uint32_t)kv->key, (uint32_t)kv->value);

            if(strcmp((char*)kv->key, "origin") == 0)
            {
                float x,y,z;
                char* pEnd = NULL;
                x = strtof((char*)kv->value, &pEnd);
                y = strtof(pEnd, &pEnd);
                z = strtof(pEnd, NULL);

                world_origin = (Vector*)(malloc(sizeof(Vector)));
                world_origin->x = x;
                world_origin->y = y;
                world_origin->z = z;
            }
            else if(strcmp((char*)kv->key, "parentname") == 0)
            {
                parentName = kv->value;
            }

            kv_node = kv_node->nextVal;
        }

        if(newTrigger)
        {
            rootconsole->ConsolePrint("Activated restored entity");
            pDispatchSpawnFunc(newTrigger);

            //Activate
            pDynamicOneArgFunc = (pOneArgProt)(  *(uint32_t*)((*(uint32_t*)(newTrigger))+0x88)  );
            pDynamicOneArgFunc(newTrigger);

            if(world_origin)
            {
                //UTIL_SetOrigin
                //pDynamicThreeArgFunc = (pThreeArgProt)(server_srv + 0x00B67000);
                //pDynamicThreeArgFunc(newTrigger, (uint32_t)world_origin, 1);

                rootconsole->ConsolePrint("Updated origin [%f] [%f] [%f]", world_origin->x, world_origin->y, world_origin->z);

                free(world_origin);
            }

            if(parentName)
            {
                uint32_t dataDummy = (uint32_t)malloc(4096);
                *(uint32_t*)(dataDummy+8) = parentName;
                *(uint32_t*)(dataDummy) = 0;
                *(uint32_t*)(dataDummy+0x18) = 2;

                //SetParent
                pDynamicTwoArgFunc = (pTwoArgProt)(server_srv + 0x00650020);
                pDynamicTwoArgFunc(newTrigger, dataDummy);

                rootconsole->ConsolePrint("SetParent [%s]", parentName);

                free((void*)dataDummy);
            }
        }

        //SetAllowPrecache
        pDynamicOneArgFunc = (pOneArgProt)(server_srv + 0x003D7620);
        pDynamicOneArgFunc(origVal);

        saved_trigger = saved_trigger->nextVal;
    }

    pDynamicOneArgFunc = (pOneArgProt)(  *(uint32_t*)( (*(uint32_t*)(main_engine_global))+0x68 )  );
    pDynamicOneArgFunc(main_engine_global);
}

void SavePlayers()
{
    ReleasePlayerSavedList();
    uint32_t firstEnt = 0;

    // Save players
    while((firstEnt = functions.FindEntityByClassname(CGlobalEntityList, firstEnt, (uint32_t)"player")) != 0)
    {
        SavedEntity* saved_player = SaveEntity(firstEnt);

        uint32_t playerRefHandle = *(uint32_t*)(firstEnt+0x350);
        uint32_t m_Network = *(uint32_t*)(firstEnt+0x24);
        uint16_t playerIndex = *(uint16_t*)(m_Network+0x6);

        PlayerSave* player_save = CreateNewPlayerSave(saved_player);
        InsertToPlayerSaveList(playerSaveList, player_save);
    }
}

void SaveGame_Extension()
{
    if(savegame && !savegame_lock)
    {
        save_frames = 0;
        savegame_lock = true;
    }
    else if(savegame && savegame_lock && save_frames >= 3)
    {
        save_frames = 3;
        
        if(!restoring && isCollisionListEmpty())
        {
            car_delay_for_save = 0;
            
            SaveGameSafe(false);

            hasSavedOnce = true;
            savegame = false;
            savegame_lock = false;
        }
    }
}

void PopCollisionChanges()
{
    pOneArgProt pDynamicOneArgFunc;
    pTwoArgProt pDynamicTwoArgFunc;

    if(pthread_mutex_lock(&collisionListLock) != 0) return;
    Value* val = *collisionList;

    if(val)
    {
        Value* nextVal = val->nextVal;
        uint32_t object = IsEntityCollisionReady((uint32_t)val->value);

        if(object)
        {
            pDynamicOneArgFunc = (pOneArgProt)(server_srv + 0x003D8D20);
            pDynamicOneArgFunc(object);
        }

        free(val);
        *collisionList = nextVal;
    }
    
    pthread_mutex_unlock(&collisionListLock);
}

uint32_t IsEntityCollisionReady(uint32_t refHandle)
{
    pOneArgProt pDynamicOneArgFunc;
    pTwoArgProt pDynamicTwoArgFunc;

    uint32_t object = GetCBaseEntitySynergy(refHandle);

    if(object)
    {
        return object;
    }

    return 0;
}

void RemoveInvalidEntries()
{
    pOneArgProt pDynamicOneArgFunc;

    if(pthread_mutex_lock(&collisionListLock) != 0) return;
    Value* val = *collisionList;

    if(val)
    {
        while(val)
        {
            if(!IsEntityCollisionReady((uint32_t)val->value))
            {
                Value* nextVal = val->nextVal;
                free(val);
                *collisionList = nextVal;
                val = nextVal;
            }
            else
            {
                break;
            }
        }
        
        while(val && val->nextVal)
        {
            Value* nextVal = val->nextVal->nextVal;

            if(!IsEntityCollisionReady((uint32_t)val->nextVal->value))
            {
                free(val->nextVal);
                val->nextVal = nextVal;
                continue;
            }

            val = val->nextVal;
        }
    }

    pthread_mutex_unlock(&collisionListLock);   
}

void FlushCollisionChanges()
{
    pOneArgProt pDynamicOneArgFunc;
    pTwoArgProt pDynamicTwoArgFunc;

    if(pthread_mutex_lock(&collisionListLock) != 0) return;
    Value* val = *collisionList;

    while(val)
    {
        Value* nextVal = val->nextVal;
        uint32_t object = IsEntityCollisionReady((uint32_t)val->value);

        if(object)
        {
            pDynamicOneArgFunc = (pOneArgProt)(server_srv + 0x003D8D20);
            pDynamicOneArgFunc(object);
        }

        free(val);
        val = nextVal;
    }

    *collisionList = NULL;
    pthread_mutex_unlock(&collisionListLock);
}

void InsertViewcontrolsToResetList()
{
    uint32_t viewControl = 0;

    while((viewControl = functions.FindEntityByClassname(CGlobalEntityList, viewControl, (uint32_t)"point_viewcontrol")) != 0)
    {
        uint32_t m_refHandle = *(uint32_t*)(viewControl+0x350);
        
        pOneArgProt pDynamicOneArgFunc = (pOneArgProt)(*(uint32_t*)((*(uint32_t*)viewControl)+0x30));
        uint32_t dmap = pDynamicOneArgFunc(viewControl);

        uint32_t m_state = GetEntityField(dmap, viewControl, 0, 0, (uint32_t)"m_state");

        if(m_state && *(uint8_t*)(m_state) == 1)
        {
            Value* refHandleSave = CreateNewValue((void*)m_refHandle);
            InsertToValuesList(viewcontrolresetlist, refHandleSave, NULL, false, false);
        }
    }
}

void ResetView()
{
    after_restore_frames++;

    if(reset_viewcontrol && after_restore_frames >= 40)
    {
        ResetViewcontrolFromList();
        reset_viewcontrol = false;
    }
}

void ResetViewcontrolFromList()
{
    Value* control = *viewcontrolresetlist;

    while(control)
    {
        Value* nextVal = control->nextVal;
        uint32_t view_ref = (uint32_t)(control->value);
        uint32_t object = GetCBaseEntitySynergy(view_ref);

        if(object)
        {
            EnableViewControl(object);
            rootconsole->ConsolePrint("[%X] control reset", view_ref);
        }

        free(control);
        control = nextVal;
    }

    *viewcontrolresetlist = NULL;
}

void GivePlayerWeapons(uint32_t player_object, bool force_give)
{
    uint32_t equip_coop = 0;

    while((equip_coop = functions.FindEntityByClassname(CGlobalEntityList, equip_coop, (uint32_t)"info_player_equip")) != 0)
    {
        pOneArgProt pDynamicOneArgFunc = (pOneArgProt)(*(uint32_t*)((*(uint32_t*)equip_coop)+0x30));
        uint32_t dmap = pDynamicOneArgFunc(equip_coop);
        uint32_t m_bDisabled = GetEntityField(dmap, equip_coop, 0, 0, (uint32_t)"m_bDisabled");

        if((m_bDisabled && *(uint8_t*)(m_bDisabled) == 0) || force_give)
        { 
            pTwoArgProt pDynamicTwoArgFunc = (pTwoArgProt)(  *(uint32_t*)((*(uint32_t*)(equip_coop))+0x2F0)  );
            pDynamicTwoArgFunc(equip_coop, player_object);
        }
    }
}

void FixModelnameSlashes()
{
    uint32_t mainEnt = 0;

    while((mainEnt = functions.FindEntityByClassname(CGlobalEntityList, mainEnt, (uint32_t)"*")) != 0)
    {
        char* classname = (char*) ( *(uint32_t*)(mainEnt+0x68) );
        
        if(strcmp(classname, "prop_vehicle_jeep") != 0 && strcmp(classname, "prop_vehicle_mp") != 0)
            continue;

        pOneArgProt pDynamicOneArgFunc = (pOneArgProt)(*(uint32_t*)((*(uint32_t*)mainEnt)+0x30));
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
}

uint32_t SaveGameSafe(bool use_internal_savename)
{
    if(use_internal_savename)
    {
        char* internal_name = (char*) ((*(uint32_t*)(server_srv + 0x00FA0CF0)) + 0x131);
        
        if(strcmp(internal_name, "") != 0)
        {
            rootconsole->ConsolePrint("Saving game to: [%s]", internal_name);
            SaveGameState(*(uint32_t*)(server_srv + 0x00FA0CF0), (uint32_t)internal_name, (uint32_t)"AUTOSAVE");
            return 1;
        }

        return 0;
    }

    rootconsole->ConsolePrint("Saving game to: autosave");
    SaveGameState(*(uint32_t*)(server_srv + 0x00FA0CF0), (uint32_t)"autosave", (uint32_t)"AUTOSAVE");
    return 1;
}

void FlushPlayerDeaths()
{
    pOneArgProt pDynamicOneArgFunc;
    uint32_t main_engine_global = *(uint32_t*)(server_srv + 0x00109A3E0);

    if(pthread_mutex_lock(&playerDeathQueueLock) != 0) return;
    Value* playerRef = *playerDeathQueue;

    while(playerRef)
    {
        Value* nextVal = playerRef->nextVal;
        uint32_t plr_object = GetCBaseEntitySynergy((uint32_t)playerRef->value);

        if(plr_object)
        {
            rootconsole->ConsolePrint("Dequeued a player!");
            pDynamicOneArgFunc = (pOneArgProt)(server_srv + 0x0098D1A0);
            pDynamicOneArgFunc(plr_object);
        }

        free(playerRef);
        playerRef = nextVal;
    }

    *playerDeathQueue = NULL;
    pthread_mutex_unlock(&playerDeathQueueLock);
}

void TriggerMovedFailsafe()
{
    pOneArgProt pDynamicOneArgFunc;
    if(!sdktools_passed) return;
    
    uint32_t trigger_ent = 0;

    while((trigger_ent = functions.FindEntityByClassname(CGlobalEntityList, trigger_ent, (uint32_t)"trigger_multiple")) != 0)
    {
        pTwoArgProt pDynamicTwoArgFunc;
        char* targetname = (char*)  *(uint32_t*)(trigger_ent+0x124);

        if(!targetname) continue;
        if(strncmp(targetname, "lambdacache", 11) != 0) continue;

        TriggerMovedExtension(trigger_ent);
    }
}

bool HandleSourcemodOutputHook(uint32_t arg0, uint32_t player, uint32_t pTrigger, uint32_t outputName)
{
    pTwoArgProt pDynamicTwoArgFunc;
    pThreeArgProt pDynamicThreeArgFunc;
    pFourArgProt pDynamicFourArgFunc;
    
    uint32_t g_OutputManager = sdktools + 0x0006C2B4;
    uint32_t gamehelpers = *(uint32_t*)(sdktools + 0x00073014);

    uint32_t var10 = 0;
    uint32_t var2c = 0;
    uint32_t var28 = (uint32_t)malloc(2048);

    uint32_t arg0_internal = *(uint32_t*)(g_OutputManager+0xC);
    uint32_t arg1_internal = *(uint32_t*)(pTrigger+0x68);
    uint32_t call_address = *(uint32_t*)((*(uint32_t*)(arg0_internal))+0x4);

    pDynamicThreeArgFunc = (pThreeArgProt)(call_address);
    pDynamicThreeArgFunc(arg0_internal, arg1_internal, (uint32_t)(&var10)); 

    /*pDynamicFourArgFunc = (pFourArgProt)(sdktools + 0x00016B80);
    pDynamicFourArgFunc(var28, 0, 0, arg0);

    pDynamicThreeArgFunc = (pThreeArgProt)(*(uint32_t*)((*(uint32_t*)(*(uint32_t*)(g_OutputManager+8)))+4));
    uint8_t foundOutput = pDynamicThreeArgFunc(*(uint32_t*)(g_OutputManager+8), var28, (uint32_t)(&var2c));*/

    uint32_t arg0_internal_two = *(uint32_t*)var10;
    uint32_t arg1_internal_two = (uint32_t)outputName;
    uint32_t call_address_two = *(uint32_t*)((*(uint32_t*)(arg0_internal_two))+4);

    pDynamicThreeArgFunc = (pThreeArgProt)(call_address_two);
    uint8_t foundOutput = pDynamicThreeArgFunc(arg0_internal_two, arg1_internal_two, (uint32_t)(&var2c));

    int counter = 0;

    if(foundOutput)
    {
        //rootconsole->ConsolePrint("Worked!");
        uint32_t var34 = 0;
        
        uint32_t ebx = var2c;
        ebx = *(uint32_t*)(ebx);
        ebx = *(uint32_t*)(ebx+4);

        while(true)
        {
            counter++;
            var34 = ebx;
            ebx = *(uint32_t*)(ebx);

            uint32_t single_hook_ent = *(uint32_t*)(ebx);

            if(single_hook_ent != 0xFFFFFFFF)
            {
                ebx = *(uint32_t*)(var34+4);
                if(ebx == *(uint32_t*)var2c) break;
                rootconsole->ConsolePrint("Skipped single hook!");
                continue;
            }

            uint32_t ebx_eight = *(uint32_t*)(ebx+8);

            char printMessage[2048];
            snprintf(printMessage, 2048, "output to trigger [%s]", (char*)var2c+8);
            PrintToClient(player, 2, (uint32_t)printMessage, 0, 0, 0, 0);

            *(uint8_t*)(ebx+0x10) = 1;

            //First Call
            pDynamicTwoArgFunc = (pTwoArgProt)(  *(uint32_t*)((*(uint32_t*)(ebx_eight))+0x14) );
            pDynamicTwoArgFunc(ebx_eight, var2c+8);


            pDynamicTwoArgFunc = (pTwoArgProt)(  *(uint32_t*)((*(uint32_t*)(gamehelpers))+0x48)  );
            uint32_t esi = pDynamicTwoArgFunc(gamehelpers, pTrigger);

            pDynamicTwoArgFunc = (pTwoArgProt)(  *(uint32_t*)((*(uint32_t*)(gamehelpers))+0x58)  );
            uint32_t secondArgReturn_earlier = pDynamicTwoArgFunc(gamehelpers, esi);

            snprintf(printMessage, 2048, "ent_id [%d]", secondArgReturn_earlier);
            PrintToClient(player, 2, (uint32_t)printMessage, 0, 0, 0, 0);

            pDynamicTwoArgFunc = (pTwoArgProt)(  *(uint32_t*)(*(uint32_t*)(ebx_eight)) );
            pDynamicTwoArgFunc(ebx_eight, secondArgReturn_earlier);




            pDynamicTwoArgFunc = (pTwoArgProt)(  *(uint32_t*)((*(uint32_t*)(gamehelpers))+0x4C)  );
            uint32_t secondArgReturn = pDynamicTwoArgFunc(gamehelpers, player);

            snprintf(printMessage, 2048, "ent_id [%d]", secondArgReturn);
            PrintToClient(player, 2, (uint32_t)printMessage, 0, 0, 0, 0);

            pDynamicTwoArgFunc = (pTwoArgProt)(  *(uint32_t*)(*(uint32_t*)(ebx_eight)) );
            pDynamicTwoArgFunc(ebx_eight, secondArgReturn);



            pDynamicTwoArgFunc = (pTwoArgProt)(  *(uint32_t*)((*(uint32_t*)(ebx_eight))+8) );
            pDynamicTwoArgFunc(ebx_eight, 0);

            //Last Call
            pDynamicTwoArgFunc = (pTwoArgProt)(  *(uint32_t*)((*(uint32_t*)(ebx_eight))+0x20) );
            pDynamicTwoArgFunc(ebx_eight, (uint32_t)(&var10));

            *(uint8_t*)(ebx+0x10) = 0;

            ebx = *(uint32_t*)(var34+4);
            if(ebx == *(uint32_t*)var2c) break;
        }

        char printMessage[2048];
        snprintf(printMessage, 2048, "callbacks [%d]", counter);
        PrintToClient(player, 2, (uint32_t)printMessage, 0, 0, 0, 0);

        free((void*)var28);
        return true;
    }
    /*else
    {
        uint32_t arg0_internal_two = *(uint32_t*)var10;
        uint32_t arg1_internal_two = (uint32_t)outputName;
        uint32_t call_address_two = *(uint32_t*)((*(uint32_t*)(arg0_internal_two))+4);

        pDynamicThreeArgFunc = (pThreeArgProt)(call_address_two);
        uint8_t foundOutput = pDynamicThreeArgFunc(arg0_internal_two, arg1_internal_two, (uint32_t)(&var2c));

        if(foundOutput)
        {
            uint32_t arg0_internal_insert = *(uint32_t*)(g_OutputManager+8);

            //Insert
            pDynamicThreeArgFunc = (pThreeArgProt)( *(uint32_t*)(*(uint32_t*)arg0_internal_insert) );
            pDynamicThreeArgFunc(arg0_internal_insert, var28, var2c);
        }

        rootconsole->ConsolePrint("Inserted the data!");
    }*/

    free((void*)var28);
    return false;
}

void TriggerMovedExtension(uint32_t pTrigger)
{
    pOneArgProt pDynamicOneArgFunc;
    pTwoArgProt pDynamicTwoArgFunc;
    pThreeArgProt pDynamicThreeArgFunc;
    pFourArgProt pDynamicFourArgFunc;
    pFiveArgProt pDynamicFiveArgFunc;
    char* targetname = (char*)  *(uint32_t*)(pTrigger+0x124);
    uint32_t player = 0;

    while((player = functions.FindEntityByClassname(CGlobalEntityList, player, (uint32_t)"player")) != 0)
    {
        pDynamicOneArgFunc = (pOneArgProt)(*(uint32_t*)((*(uint32_t*)pTrigger)+0x30));
        uint32_t dmap = pDynamicOneArgFunc(pTrigger);

        uint32_t startDisabled = GetEntityField(dmap, pTrigger, 0, 0, (uint32_t)"m_bDisabled");
        uint8_t startDisabled_Val = *(uint8_t*)(startDisabled);

        if(startDisabled_Val)
        {
            continue;
        }

        uint32_t trigger_abs = GetEntityField(dmap, pTrigger, 0, 0, (uint32_t)"m_vecAbsOrigin");
        uint32_t trigger_mins = GetEntityField(dmap, pTrigger, 0, 0, (uint32_t)"m_EDT_vecMins");
        uint32_t trigger_maxs = GetEntityField(dmap, pTrigger, 0, 0, (uint32_t)"m_EDT_vecMaxs");

        pDynamicOneArgFunc = (pOneArgProt)(*(uint32_t*)((*(uint32_t*)player)+0x30));
        dmap = pDynamicOneArgFunc(player);

        uint32_t player_abs = GetEntityField(dmap, player, 0, 0, (uint32_t)"m_vecAbsOrigin");

        Vector* trigger_vecMinsAbs = (Vector*)(malloc(sizeof(Vector)));
        trigger_vecMinsAbs->x = *(float*)(trigger_abs);
        trigger_vecMinsAbs->y = *(float*)(trigger_abs+0x4);
        trigger_vecMinsAbs->z = *(float*)(trigger_abs+0x8);

        trigger_vecMinsAbs->x = trigger_vecMinsAbs->x + *(float*)(trigger_mins)     - 15;
        trigger_vecMinsAbs->y = trigger_vecMinsAbs->y + *(float*)(trigger_mins+0x4) - 15;
        trigger_vecMinsAbs->z = trigger_vecMinsAbs->z + *(float*)(trigger_mins+0x8) - 35;

        Vector* trigger_vecMaxsAbs = (Vector*)(malloc(sizeof(Vector)));
        trigger_vecMaxsAbs->x = *(float*)(trigger_abs);
        trigger_vecMaxsAbs->y = *(float*)(trigger_abs+0x4);
        trigger_vecMaxsAbs->z = *(float*)(trigger_abs+0x8);

        trigger_vecMaxsAbs->x = trigger_vecMaxsAbs->x + *(float*)(trigger_maxs)     + 15;
        trigger_vecMaxsAbs->y = trigger_vecMaxsAbs->y + *(float*)(trigger_maxs+0x4) + 15;
        trigger_vecMaxsAbs->z = trigger_vecMaxsAbs->z + *(float*)(trigger_maxs+0x8) + 35;

        if(trigger_vecMinsAbs->x <= *(float*)(player_abs) && *(float*)(player_abs) <= trigger_vecMaxsAbs->x)
        {
            if(trigger_vecMinsAbs->y <= *(float*)(player_abs+0x4) && *(float*)(player_abs+0x4) <= trigger_vecMaxsAbs->y)
            {
                if(trigger_vecMinsAbs->z <= *(float*)(player_abs+0x8) && *(float*)(player_abs+0x8) <= trigger_vecMaxsAbs->z)
                {
                    char printMessage[2048];
                    snprintf(printMessage, 2048, "triggering [%s]", targetname);
                    PrintToClient(player, 2, (uint32_t)printMessage, 0, 0, 0, 0);

                    //FireOutput
                    //pDynamicFourArgFunc = (pFourArgProt)(server_srv + 0x00687CC0);
                    //pDynamicFourArgFunc(pTrigger+0x4A8, player, pTrigger, 0);

                    //FireOutput
                    //pDynamicFourArgFunc = (pFourArgProt)(server_srv + 0x00687CC0);
                    //pDynamicFourArgFunc(pTrigger+0x404, player, pTrigger, 0);

                    HandleSourcemodOutputHook(pTrigger+0x3EC, player, pTrigger, (uint32_t)"OnStartTouch");
                    //HandleSourcemodOutputHook(pTrigger+0x4A8, player, pTrigger, (uint32_t)"OnTrigger");

                    //FireOutput
                    //pDynamicFourArgFunc = (pFourArgProt)(server_srv + 0x00687CC0);
                    //pDynamicFourArgFunc(pTrigger+0x3EC, player, pTrigger, 0);

                    //uint32_t variant = 0;
                    //pDynamicThreeArgFunc = (pThreeArgProt)(server_srv + 0x00687BE0);
                    //pDynamicThreeArgFunc((uint32_t)(&variant), 0, 0);

                    //FireOutputWithoutSmHook
                    //pDynamicFiveArgFunc = (pFiveArgProt)(server_srv + 0x00686560);
                    //pDynamicFiveArgFunc(pTrigger+0x3EC, (uint32_t)(&variant), player, pTrigger, 0);

                    //StartTouch
                    //pDynamicTwoArgFunc = (pTwoArgProt)(server_srv + 0x0047D7B0);
                    //pDynamicTwoArgFunc(pTrigger, player);

                    //rootconsole->ConsolePrint("touch");
                }
            }
        }

        free(trigger_vecMinsAbs);
        free(trigger_vecMaxsAbs);
    }
}

bool isCollisionListEmpty()
{
    if(pthread_mutex_lock(&collisionListLock) != 0) return false;
    bool isEmpty = *collisionList == NULL;
    pthread_mutex_unlock(&collisionListLock);
    return isEmpty;
}

bool isAnyClientConnecting()
{
    int total_clients = GetNumClients(sv);
    int total_clients_ingame = GetTotalClientsInGame();

    if(total_clients == total_clients_ingame) return false;
    return true;
}

void ReleaseLeakedPackedEntities()
{
    pTwoArgProt pDynamicTwoArgFunc;
    uint32_t snapManager = *(uint32_t*)(engine_srv + 0x002BEF30);

    int freed_leaks = 0;

    for(int i = 0; i < 2048; i++)
    {
        //rootconsole->ConsolePrint("trying [%d] ent", i);
        uint32_t computed_ref = *(uint32_t*)(snapManager+(i+0x18)*4+8);

        if(computed_ref != 0)
        {
            pDynamicTwoArgFunc = (pTwoArgProt)(engine_srv + 0x001A6070);
            pDynamicTwoArgFunc(snapManager, computed_ref);

            *(uint32_t*)(snapManager+(i+0x18)*4+8) = 0;
            freed_leaks++;
        }
    }

    rootconsole->ConsolePrint("Purged [%d] packed ents!", freed_leaks);
}

void RestorePlayers()
{
    pOneArgProt pDynamicOneArgFunc;
    pThreeArgProt pDynamicThreeArgFunc;

    rootconsole->ConsolePrint("Restoring players...");
    
    uint32_t playerEnt = 0;
    while((playerEnt = functions.FindEntityByClassname(CGlobalEntityList, playerEnt, (uint32_t)"player")) != 0)
    {
        uint32_t m_Network = *(uint32_t*)(playerEnt+0x24);
        uint16_t playerIndex = *(uint16_t*)(m_Network+0x6);
        uint32_t playerRefHandle = *(uint32_t*)(playerEnt+0x350);

        //Restore Player
        uint8_t returnVal = MainPlayerRestore__External((*(uint32_t*)(server_srv + 0x00FA0CF0)), playerEnt, 1);

        player_restore_failed = false;

        if(returnVal == 0)
        {
            PlayerSpawnHook__External(playerEnt, 1, 1);

            pOneArgProt pDynamicOneArgFunc = (pOneArgProt)(*(uint32_t*)((*(uint32_t*)(playerEnt))+0x5C));
            pDynamicOneArgFunc(playerEnt);
        }

        uint32_t global_one = *(uint32_t*)(server_srv + 0x01012420);
        global_one = *(uint32_t*)(global_one);

        pTwoArgProt pDynamicTwoArgFunc = (pTwoArgProt)(*(uint32_t*)(global_one+0x4C));
        uint32_t pEntity = pDynamicTwoArgFunc(*(uint32_t*)(server_srv + 0x01012420), playerIndex);

        pDynamicThreeArgFunc = (pThreeArgProt)(*(uint32_t*)(global_one+0x98));
        pDynamicThreeArgFunc(*(uint32_t*)(server_srv + 0x01012420), pEntity, (uint32_t)"__client_reset\n");
    }

    rootconsole->ConsolePrint("Finished restoring players!");
}

void DestroyVObjectForMarkedEnts()
{
    pOneArgProt pDynamicOneArgFunc;
    pTwoArgProt pDynamicTwoArgFunc;

    int ent_size = *(int*)(server_srv + 0x0100890C);
    uint32_t g_DeleteList = *(uint32_t*)(server_srv + 0x01008900);

    if(ent_size > 0)
    {
        for(int i = 0; i < ent_size; i++)
        {
            uint32_t iServerObj = *(uint32_t*)(g_DeleteList+i*4);
            uint32_t cbase = *(uint32_t*)(iServerObj+8);
            uint32_t m_refHandle = *(uint32_t*)(cbase+0x350);
            uint32_t verified_cbase = GetCBaseEntitySynergy(m_refHandle);

            if(verified_cbase)
            {
                //rootconsole->ConsolePrint("v obj dest! [%s]", *(uint32_t*)(cbase+0x68));

                //VphysicsDestroyObject
                pDynamicOneArgFunc = (pOneArgProt)( *(uint32_t*)((*(uint32_t*)(verified_cbase))+0x274) );
                pDynamicOneArgFunc(verified_cbase);
                continue;
            }

            rootconsole->ConsolePrint("Critical error invalid entity object!");
            exit(EXIT_FAILURE);
        }
    }
}

void UpdatePlayersDonor()
{
    pOneArgProt pDynamicOneArgFunc;
    Value* first_player = *new_player_join_ref;
    ValueList left_players = AllocateValuesList();

    while(first_player)
    {
        Value* next_player = first_player->nextVal;
        EntityFrameCount* entity_delay = (EntityFrameCount*)(first_player->value);
        uint32_t player = GetCBaseEntitySynergy(entity_delay->entity_ref);

        if(player && entity_delay->frames >= 300)
        {
            //DonorColorParser
            pDynamicOneArgFunc = (pOneArgProt)(server_srv + 0x0098F010);
            pDynamicOneArgFunc(player);

            //rootconsole->ConsolePrint("\nUpdated Color\n");

            free(entity_delay);
        }
        else
        {
            entity_delay->frames = entity_delay->frames + 1;
            Value* left_player = CreateNewValue((void*)entity_delay);
            InsertToValuesList(left_players, left_player, NULL, false, false);
        }

        free(first_player);
        first_player = next_player;
    }

    free(new_player_join_ref);
    new_player_join_ref = left_players;
}

void CorrectNpcAi(uint32_t arg0)
{
    pOneArgProt pDynamicOneArgFunc;

    uint32_t ai_object = *(uint32_t*)(arg0+0x0BC0);

    if(ai_object)
    {
        bool failure = false;
        int iVar10 = 0;

        do
        {
            uint32_t uVar11 = *(uint32_t*)(ai_object + 8 + iVar10 * 4);
            uint32_t object = GetCBaseEntitySynergy(uVar11);

            if(IsEntityValid(object) == 0)
            {
                //INVALID FOUND
                rootconsole->ConsolePrint("Invalid Entity in Combine AI [%d]", iVar10);

                failure = true;
            }

            iVar10 = iVar10 + 1;
        }
        while(iVar10 < *(int*)(ai_object + 0x48));

        if(failure)
        {
            //Clear
            pDynamicOneArgFunc = (pOneArgProt)(server_srv + 0x00571A00);
            pDynamicOneArgFunc(arg0);
        }
    }
}