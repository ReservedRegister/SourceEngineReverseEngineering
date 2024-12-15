#include "extension.h"
#include "util.h"
#include "core.h"
#include "ext_main.h"

ValueList leakedResourcesSaveRestoreSystem;
ValueList leakedResourcesEdtSystem;

bool sdktools_passed;

void InitCoreSynergy()
{
    our_libraries[0] = (uint32_t)malloc(1024);
    snprintf((char*)our_libraries[0], 1024, "%s", "/synergy/bin/server.so");

    our_libraries[1] = (uint32_t)malloc(1024);
    snprintf((char*)our_libraries[1], 1024, "%s", "/bin/engine_srv.so");

    our_libraries[2] = (uint32_t)malloc(1024);
    snprintf((char*)our_libraries[2], 1024, "%s", "/bin/dedicated_srv.so");

    our_libraries[3] = (uint32_t)malloc(1024);
    snprintf((char*)our_libraries[3], 1024, "%s", "/bin/vphysics_srv.so");

    our_libraries[4] = (uint32_t)malloc(1024);
    snprintf((char*)our_libraries[4], 1024, "%s", "/extensions/sdktools.ext.2.sdk2013.so");
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
    uint32_t EntityList = fields.CGlobalEntityList;
    uint32_t refHandle = (EHandle & 0xFFF) << 4;

    EHandle = EHandle >> 0x0C;

    if(*(uint32_t*)(EntityList+refHandle+8) == EHandle)
    {
        uint32_t CBaseEntity = *(uint32_t*)(EntityList+refHandle+4);
        return CBaseEntity;
    }

    return 0;
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

void InstaKillSynergy(uint32_t entity_object, bool validate)
{
    pZeroArgProt pDynamicZeroArgFunc;
    pOneArgProt pDynamicOneArgFunc;
    pTwoArgProt pDynamicTwoArgFunc;

    if(entity_object == 0) return;

    uint32_t refHandleInsta = *(uint32_t*)(entity_object+offsets.refhandle_offset);
    char* classname = (char*)( *(uint32_t*)(entity_object+offsets.classname_offset));
    uint32_t cbase_chk = functions.GetCBaseEntity(refHandleInsta);

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
            rootconsole->ConsolePrint("\n\nFailed to verify entity for fast kill [%X]\n\n", (uint32_t)__builtin_return_address(0) - server);
            exit(EXIT_FAILURE);
            return;
        }
    }

    uint32_t isMarked = functions.IsMarkedForDeletion(cbase_chk+offsets.iserver_offset);

    if(isMarked)
    {
        rootconsole->ConsolePrint("Attempted to kill an entity twice in UTIL_RemoveImmediate(CBaseEntity*)");
        return;
    }

    if((*(uint32_t*)(cbase_chk + 0x128) & 1) == 0)
    {
        if(*(uint32_t*)(server + 0x00F3A570) == 0)
        {
            // FAST DELETE ONLY

            //hooked_delete_counter++;

            *(uint32_t*)(cbase_chk + 0x128) = *(uint32_t*)(cbase_chk + 0x128) | 1;

            //UpdateOnRemove
            pDynamicOneArgFunc = (pOneArgProt)(  *(uint32_t*)((*(uint32_t*)(cbase_chk))+0x1AC) );
            pDynamicOneArgFunc(cbase_chk);

            //CALL RELEASE
            uint32_t iServerObj = cbase_chk+offsets.iserver_offset;

            pDynamicOneArgFunc = (pOneArgProt)(  *(uint32_t*)((*(uint32_t*)(iServerObj))+0x10) );
            pDynamicOneArgFunc(iServerObj);
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

    char* classname = (char*)(*(uint32_t*)(entity_object+offsets.classname_offset));
    uint32_t m_refHandle = *(uint32_t*)(entity_object+offsets.refhandle_offset);
    uint32_t chk_ref = functions.GetCBaseEntity(m_refHandle);

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
        if(strcmp(classname, "player") == 0)
        {
            rootconsole->ConsolePrint(EXT_PREFIX "Tried killing player but was protected!");
            return;
        }

        //Check if entity was already removed!
        uint32_t isMarked = functions.IsMarkedForDeletion(chk_ref+offsets.iserver_offset);

        if(isMarked) return;

        //UTIL_Remove(IServerNetworkable*)
        pDynamicOneArgFunc = (pOneArgProt)(server + 0x0089F690);
        pDynamicOneArgFunc(chk_ref+offsets.iserver_offset);

        //Check if entity was removed!
        isMarked = functions.IsMarkedForDeletion(chk_ref+offsets.iserver_offset);

        if(isMarked)
        {
            if(*(uint32_t*)(server + 0x00F3A570) != 0)
            {
                hooked_delete_counter++;
            }
        }

        return;
    }

    if(classname)
    {
        rootconsole->ConsolePrint(EXT_PREFIX "Could not kill entity [Invalid Ehandle] [%s] [%X]", classname, (uint32_t)__builtin_return_address(0) - server);
    }
    else
    {
        rootconsole->ConsolePrint(EXT_PREFIX "Could not kill entity [Invalid Ehandle] [%X]", (uint32_t)__builtin_return_address(0) - server);
    }

    exit(EXIT_FAILURE);
}

void TriggerMovedFailsafe()
{
    pOneArgProt pDynamicOneArgFunc;
    if(!sdktools_passed) return;
    
    uint32_t trigger_ent = 0;

    while((trigger_ent = functions.FindEntityByClassname(fields.CGlobalEntityList, trigger_ent, (uint32_t)"trigger_multiple")) != 0)
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
            //PrintToClient(player, 2, (uint32_t)printMessage, 0, 0, 0, 0);

            *(uint8_t*)(ebx+0x10) = 1;

            //First Call
            pDynamicTwoArgFunc = (pTwoArgProt)(  *(uint32_t*)((*(uint32_t*)(ebx_eight))+0x14) );
            pDynamicTwoArgFunc(ebx_eight, var2c+8);


            pDynamicTwoArgFunc = (pTwoArgProt)(  *(uint32_t*)((*(uint32_t*)(gamehelpers))+0x48)  );
            uint32_t esi = pDynamicTwoArgFunc(gamehelpers, pTrigger);

            pDynamicTwoArgFunc = (pTwoArgProt)(  *(uint32_t*)((*(uint32_t*)(gamehelpers))+0x58)  );
            uint32_t secondArgReturn_earlier = pDynamicTwoArgFunc(gamehelpers, esi);

            snprintf(printMessage, 2048, "ent_id [%d]", secondArgReturn_earlier);
            //PrintToClient(player, 2, (uint32_t)printMessage, 0, 0, 0, 0);

            pDynamicTwoArgFunc = (pTwoArgProt)(  *(uint32_t*)(*(uint32_t*)(ebx_eight)) );
            pDynamicTwoArgFunc(ebx_eight, secondArgReturn_earlier);




            pDynamicTwoArgFunc = (pTwoArgProt)(  *(uint32_t*)((*(uint32_t*)(gamehelpers))+0x4C)  );
            uint32_t secondArgReturn = pDynamicTwoArgFunc(gamehelpers, player);

            snprintf(printMessage, 2048, "ent_id [%d]", secondArgReturn);
            //PrintToClient(player, 2, (uint32_t)printMessage, 0, 0, 0, 0);

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
        //PrintToClient(player, 2, (uint32_t)printMessage, 0, 0, 0, 0);

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

    while((player = functions.FindEntityByClassname(fields.CGlobalEntityList, player, (uint32_t)"player")) != 0)
    {
        pDynamicOneArgFunc = (pOneArgProt)(*(uint32_t*)((*(uint32_t*)pTrigger)+0x30));
        uint32_t dmap = pDynamicOneArgFunc(pTrigger);

        uint32_t startDisabled = 0;
        //uint32_t startDisabled = GetEntityField(dmap, pTrigger, 0, 0, (uint32_t)"m_bDisabled");
        uint8_t startDisabled_Val = *(uint8_t*)(startDisabled);

        if(startDisabled_Val)
        {
            continue;
        }

        uint32_t trigger_abs = 0;
        uint32_t trigger_mins = 0;
        uint32_t trigger_maxs = 0;
        //uint32_t trigger_abs = GetEntityField(dmap, pTrigger, 0, 0, (uint32_t)"m_vecAbsOrigin");
        //uint32_t trigger_mins = GetEntityField(dmap, pTrigger, 0, 0, (uint32_t)"m_EDT_vecMins");
        //uint32_t trigger_maxs = GetEntityField(dmap, pTrigger, 0, 0, (uint32_t)"m_EDT_vecMaxs");

        pDynamicOneArgFunc = (pOneArgProt)(*(uint32_t*)((*(uint32_t*)player)+0x30));
        dmap = pDynamicOneArgFunc(player);

        uint32_t player_abs = 0;
        //uint32_t player_abs = GetEntityField(dmap, player, 0, 0, (uint32_t)"m_vecAbsOrigin");

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
                    //PrintToClient(player, 2, (uint32_t)printMessage, 0, 0, 0, 0);

                    //FireOutput
                    //pDynamicFourArgFunc = (pFourArgProt)(server + 0x00687CC0);
                    //pDynamicFourArgFunc(pTrigger+0x4A8, player, pTrigger, 0);

                    //FireOutput
                    //pDynamicFourArgFunc = (pFourArgProt)(server + 0x00687CC0);
                    //pDynamicFourArgFunc(pTrigger+0x404, player, pTrigger, 0);

                    HandleSourcemodOutputHook(pTrigger+0x3EC, player, pTrigger, (uint32_t)"OnStartTouch");
                    //HandleSourcemodOutputHook(pTrigger+0x4A8, player, pTrigger, (uint32_t)"OnTrigger");

                    //FireOutput
                    //pDynamicFourArgFunc = (pFourArgProt)(server + 0x00687CC0);
                    //pDynamicFourArgFunc(pTrigger+0x3EC, player, pTrigger, 0);

                    //uint32_t variant = 0;
                    //pDynamicThreeArgFunc = (pThreeArgProt)(server + 0x00687BE0);
                    //pDynamicThreeArgFunc((uint32_t)(&variant), 0, 0);

                    //FireOutputWithoutSmHook
                    //pDynamicFiveArgFunc = (pFiveArgProt)(server + 0x00686560);
                    //pDynamicFiveArgFunc(pTrigger+0x3EC, (uint32_t)(&variant), player, pTrigger, 0);

                    //StartTouch
                    //pDynamicTwoArgFunc = (pTwoArgProt)(server + 0x0047D7B0);
                    //pDynamicTwoArgFunc(pTrigger, player);

                    //rootconsole->ConsolePrint("touch");
                }
            }
        }

        free(trigger_vecMinsAbs);
        free(trigger_vecMaxsAbs);
    }
}