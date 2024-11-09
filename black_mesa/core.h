#ifndef CORE_H
#define CORE_H

#define HOOK_MSG "Saved memory reference to leaked resources list: [%X]"
#define EXT_PREFIX "[BlackMesaUtils] "

extern ValueList ragdoll_entity_list_created;
extern ValueList ragdoll_entity_list;
extern uint32_t first_ragdoll_gib;
extern int ragdoll_delete_frame_counter;
extern int ragdoll_delete_entities_total;
extern int ragdoll_breaking_gib_counter;
extern bool is_currently_ragdoll_breaking;

void InitCoreBlackMesa();
void PopulateHookExclusionListsBlackMesa();
uint32_t GetCBaseEntityBlackMesa(uint32_t EHandle);
void InstaKillBlackMesa(uint32_t entity_object, bool validate);
void RemoveEntityNormalBlackMesa(uint32_t entity_object, bool validate);

void CheckForLocation();
bool RemoveRagdollBreakingEntity(uint32_t ent);
void RemoveRagdollBreakEntities(bool bypass);
bool AddEntityToRagdollRemoveList(uint32_t object);
void CorrectVphysicsEntity(uint32_t ent);
void FlushRagdollBreakingEntities();

#endif