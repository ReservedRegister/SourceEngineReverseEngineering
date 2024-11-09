#include "extension.h"
#include "util.h"
#include "core.h"

ValueList ragdoll_entity_list_created;
ValueList ragdoll_entity_list;
uint32_t first_ragdoll_gib;
int ragdoll_delete_frame_counter;
int ragdoll_delete_entities_total;
int ragdoll_breaking_gib_counter;
bool is_currently_ragdoll_breaking;

void InitCoreBlackMesa()
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

uint32_t GetCBaseEntityBlackMesa(uint32_t EHandle)
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

void PopulateHookExclusionListsBlackMesa()
{

}

void CorrectVphysicsEntity(uint32_t ent)
{
    pThreeArgProt pDynamicThreeArgFunc;
    pFourArgProt pDynamicFourArgFunc;

    if(IsEntityValid(ent))
    {
        uint32_t vphysics_object = *(uint32_t*)(ent+0x1F8);

        if(vphysics_object)
        {
            Vector current_origin;
            Vector current_angles;
            Vector empty_vector;

            //GetPosition
            pDynamicThreeArgFunc = (pThreeArgProt)(  *(uint32_t*)((*(uint32_t*)(vphysics_object))+0xC0)  );
            pDynamicThreeArgFunc(vphysics_object, (uint32_t)&current_origin, (uint32_t)&current_angles);

            //rootconsole->ConsolePrint("%f %f %f", current_angles.x, current_angles.y, current_angles.z);

            if(!IsEntityPositionReasonable((uint32_t)&current_origin) && !IsEntityPositionReasonable((uint32_t)&current_angles))
            {
                //SetPosition
                pDynamicFourArgFunc = (pFourArgProt)(  *(uint32_t*)((*(uint32_t*)(vphysics_object))+0xB8)  );
                pDynamicFourArgFunc(vphysics_object, (uint32_t)&empty_vector, (uint32_t)&empty_vector, 1);

                rootconsole->ConsolePrint("Corrected vphysics origin & angles!");

                return;
            }

            if(!IsEntityPositionReasonable((uint32_t)&current_origin))
            {
                //SetPosition
                pDynamicFourArgFunc = (pFourArgProt)(  *(uint32_t*)((*(uint32_t*)(vphysics_object))+0xB8)  );
                pDynamicFourArgFunc(vphysics_object, (uint32_t)&empty_vector, (uint32_t)&current_angles, 1);

                rootconsole->ConsolePrint("Corrected vphysics origin!");
            }

            if(!IsEntityPositionReasonable((uint32_t)&current_angles))
            {
                //SetPosition
                pDynamicFourArgFunc = (pFourArgProt)(  *(uint32_t*)((*(uint32_t*)(vphysics_object))+0xB8)  );
                pDynamicFourArgFunc(vphysics_object, (uint32_t)&current_origin, (uint32_t)&empty_vector, 1);

                rootconsole->ConsolePrint("Corrected vphysics angles!");
            }
        }
    }
}

bool AddEntityToRagdollRemoveList(uint32_t object)
{
    if(IsEntityValid(object))
    {
        uint32_t refHandle = *(uint32_t*)(object+offsets.refhandle_offset);

        Value* new_refhandle = CreateNewValue((void*)refHandle);
        InsertToValuesList(ragdoll_entity_list, new_refhandle, NULL, true, false);

        ragdoll_delete_frame_counter = 0;
        ragdoll_delete_entities_total++;

        //rootconsole->ConsolePrint("Added entity to ragdoll break list! [%d]", ValueListItems(ragdoll_entity_list, NULL));
        return true;
    }

    rootconsole->ConsolePrint("Failed to add entity to ragdoll break list!");
    return false;
}

bool RemoveRagdollBreakingEntity(uint32_t ent)
{
    if(IsEntityValid(ent))
    {
        uint32_t refHandleFromRemovedEnt = *(uint32_t*)(ent+offsets.refhandle_offset);
        Value* firstEnt = *ragdoll_entity_list_created;

        while(firstEnt)
        {
            uint32_t refHandle = (uint32_t)firstEnt->value;

            if(refHandle == refHandleFromRemovedEnt)
            {
                bool success = RemoveFromValuesList(ragdoll_entity_list_created, (void*)refHandle, NULL);

                if(!success)
                {
                    rootconsole->ConsolePrint("Failed to manage ragdoll break lists!");
                    exit(EXIT_FAILURE);
                }

                AddEntityToRagdollRemoveList(ent);
                return true;
            }

            firstEnt = firstEnt->nextVal;
        }
    }

    return false;
}

//Warning dangerous function

void FlushRagdollBreakingEntities()
{
    // Make sure that ragdoll_entity_list_created is empty because all the entities should have been removed from that list before using this function

    if(*ragdoll_entity_list_created != NULL)
    {
        rootconsole->ConsolePrint("Failed to flush ragdoll entities!");
        exit(EXIT_FAILURE);
    }

    while(true)
    {
        RemoveRagdollBreakEntities(true);

        if(*ragdoll_entity_list == NULL)
            break;
    }
}

void RemoveRagdollBreakEntities(bool bypass)
{
    if(ragdoll_delete_frame_counter > 5 || bypass)
    {
        Value* firstEnt = *ragdoll_entity_list;

        if((firstEnt && ragdoll_delete_entities_total > 60) || (firstEnt && bypass))
        {
            uint32_t refHandle = (uint32_t)firstEnt->value;
            uint32_t object = functions.GetCBaseEntity(refHandle);

            if(IsEntityValid(object))
            {
                if(bypass)
                {
                    rootconsole->ConsolePrint("Flushing ragdoll breaking entity!");
                }
                else
                {
                    rootconsole->ConsolePrint("Removing ragdoll entity gib!");
                }

                functions.RemoveEntityNormal(object, true);
            }

            ragdoll_delete_entities_total--;

            Value* nextEnt = firstEnt->nextVal;
            *ragdoll_entity_list = nextEnt;

            free(firstEnt);
        }

        ragdoll_delete_frame_counter = 0;
    }
}

void CheckForLocation()
{
    uint32_t sv = engine_srv + 0x00315E80;
    uint32_t current_map = sv+0x11;

    if(strcmp((char*)current_map, "bm_c2a3a") != 0)
    {
        //rootconsole->ConsolePrint("Location fix disabled!");
        return;
    }

    uint32_t player = 0;

    while((player = functions.FindEntityByClassname(CGlobalEntityList, player, (uint32_t)"player")) != 0)
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

                    functions.SetSolidFlags(collision_property, 4);
                    //rootconsole->ConsolePrint("bad area!");
                    in_area = true;
                }
            }
        }

        if(!in_area)
        {
            uint32_t collision_property = player+0x160;
            uint16_t current_flags = *(uint16_t*)(collision_property+0x3C);

            functions.SetSolidFlags(collision_property, 16);
        }

        free(trigger_vecMinsAbs);
        free(trigger_vecMaxsAbs);
    }
}

void RemoveEntityNormalBlackMesa(uint32_t entity_object, bool validate)
{
    pZeroArgProt pDynamicZeroArgFunc;
    pOneArgProt pDynamicOneArgFunc;
    pTwoArgProt pDynamicTwoArgFunc;

    if(entity_object == 0) return;

    char* classname = (char*)(*(uint32_t*)(entity_object+0x64));
    uint32_t refHandle = *(uint32_t*)(entity_object+0x334);
    uint32_t object_verify = GetCBaseEntityBlackMesa(refHandle);

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
        pDynamicOneArgFunc = (pOneArgProt)(server_srv + 0x00A2B520);
        uint32_t isMarked = pDynamicOneArgFunc(object_verify+0x14);

        if(isMarked)
        {
            //rootconsole->ConsolePrint("Attempted to kill a marked entity in UTIL_Remove(IServerNetworkable*)");
            return;
        }

        if(RemoveRagdollBreakingEntity(object_verify))
        {
            return;
        }

        //IsInPhysCallback
        pDynamicZeroArgFunc = (pZeroArgProt)(server_srv + 0x00978DF0);
        uint8_t returnVal = pDynamicZeroArgFunc();

        if(returnVal == 0)
        {
            hooked_delete_counter++;
        }

        //UTIL_Remove(IServerNetworkable*)
        pDynamicOneArgFunc = (pOneArgProt)(server_srv + 0x00A92160);
        pDynamicOneArgFunc(object_verify+0x14);

        //rootconsole->ConsolePrint("Removed [%s]", clsname);

        return;
    }

    rootconsole->ConsolePrint("Failed to verify entity object!");
    exit(EXIT_FAILURE);
}

void InstaKillBlackMesa(uint32_t entity_object, bool validate)
{
    pZeroArgProt pDynamicZeroArgFunc;
    pThreeArgProt pDynamicThreeArgFunc;
    pOneArgProt pDynamicOneArgFunc;
    pTwoArgProt pDynamicTwoArgFunc;

    if(entity_object == 0) return;

    uint32_t refHandleInsta = *(uint32_t*)(entity_object+0x334);
    char* classname = (char*) ( *(uint32_t*)(entity_object+0x64) );
    uint32_t cbase_chk = GetCBaseEntityBlackMesa(refHandleInsta);

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
    pDynamicOneArgFunc = (pOneArgProt)(server_srv + 0x00A2B520);
    uint32_t isMarked = pDynamicOneArgFunc(cbase_chk+0x14);

    if(isMarked)
    {
        rootconsole->ConsolePrint("Attempted to kill an entity twice in UTIL_RemoveImmediate(CBaseEntity*)");
        return;
    }

    //PhysIsInCallback
    pDynamicOneArgFunc = (pOneArgProt)(server_srv + 0x00978DF0);
    uint32_t isInCallback = pDynamicOneArgFunc(0);

    if(isInCallback)
    {
        rootconsole->ConsolePrint("Should not be! (Insta)");
        exit(EXIT_FAILURE);

        return;
    }

    if(isTicking)
    {
        rootconsole->ConsolePrint("fast killed [%s]", classname);
    }

    if((*(uint32_t*)(cbase_chk+0x118) & 1) == 0)
    {
        if(*(uint32_t*)(server_srv + 0x01811920) == 0)
        {
            // FAST DELETE ONLY

            hooked_delete_counter++;

            *(uint8_t*)(server_srv + 0x0180F344) = 0;
            *(uint32_t*)(cbase_chk+0x118) = *(uint32_t*)(cbase_chk+0x118) | 1;

            //UpdateOnRemove
            pDynamicOneArgFunc = (pOneArgProt)(  *(uint32_t*)((*(uint32_t*)(cbase_chk))+0x1D0) );
            pDynamicOneArgFunc(cbase_chk);

            *(uint8_t*)(server_srv + 0x0180F345) = 1;

            //CALL RELEASE
            uint32_t iServerObj = cbase_chk+0x14;

            pDynamicOneArgFunc = (pOneArgProt)(  *(uint32_t*)((*(uint32_t*)(iServerObj))+0x10) );
            pDynamicOneArgFunc(iServerObj);

            *(uint8_t*)(server_srv + 0x0180F345) = 0;
        }
        else
        {
            RemoveEntityNormalBlackMesa(cbase_chk, validate);
        }
    }
}