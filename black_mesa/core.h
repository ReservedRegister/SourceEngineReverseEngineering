#ifndef CORE_H
#define CORE_H

#define HOOK_MSG "Saved memory reference to leaked resources list: [%X]"
#define EXT_PREFIX "[BlackMesaUtils] "

void InitCoreBlackMesa();
void PopulateHookExclusionListsBlackMesa();
uint32_t GetCBaseEntityBlackMesa(uint32_t EHandle);
void InstaKillBlackMesa(uint32_t entity_object, bool validate);
void RemoveEntityNormalBlackMesa(uint32_t entity_object, bool validate);

void CheckForLocation();

#endif