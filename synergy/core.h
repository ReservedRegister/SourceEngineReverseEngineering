#ifndef CORE_H
#define CORE_H

#define HOOK_MSG "Saved memory reference to leaked resources list: [%X]"
#define EXT_PREFIX "[SynergyUtils] "

extern uint32_t EdtLoadFuncAddr;
extern uint32_t Flush;
extern uint32_t HostChangelevel;
extern uint32_t SpawnServer;
extern uint32_t LookupPoseParameterAddr;
extern uint32_t sub_654260_addr;
extern uint32_t sub_628F00_addr;
extern uint32_t SaveGameStateAddr;
extern uint32_t TransitionRestoreMainCallOrigAddr;
extern uint32_t OriginalTriggerMovedAddr;
extern uint32_t DoorFinalFunctionAddr;
extern uint32_t GetNumClientsAddr;
extern uint32_t GetNumProxiesAddr;
extern uint32_t EnqueueCommandAddr;
extern uint32_t DispatchSpawnAddr;
extern uint32_t ActivateEntityAddr;
extern uint32_t AutosaveLoadOrigAddr;
extern uint32_t InactivateClientsAddr;
extern uint32_t ReconnectClientsAddr;
extern uint32_t MakeEntityDormantAddr;
extern uint32_t PrintToClientAddr;

extern pTwoArgProt pEdtLoadFunc;
extern pThreeArgProt pHostChangelevelFunc;
extern pThreeArgProt pFlushFunc;
extern pThreeArgProt pSpawnServerFunc;
extern pThreeArgProt LookupPoseParameter;
extern pOneArgProt sub_654260;
extern pOneArgProt sub_628F00;
extern pThreeArgProt SaveGameState;
extern pFourArgProt pTransitionRestoreMainCall;
extern pTwoArgProt pCallOriginalTriggerMoved;
extern pFiveArgProt pDoorFinalFunction;
extern pOneArgProt GetNumClients;
extern pOneArgProt GetNumProxies;
extern pOneArgProt EnqueueCommandFunc;
extern pOneArgProt pDispatchSpawnFunc;
extern pOneArgProt pActivateEntityFunc;
extern pThreeArgProt AutosaveLoadOrig;
extern pOneArgProt InactivateClients;
extern pOneArgProt ReconnectClients;
extern pOneArgProt MakeEntityDormant;
extern pSevenArgProt PrintToClient;

extern ValueList leakedResourcesSaveRestoreSystem;
extern ValueList leakedResourcesEdtSystem;
extern ValueList antiCycleListDoors;
extern ValueList entityDeleteList;
extern ValueList playerDeathQueue;
extern ValueList collisionList;
extern ValueList viewcontrolresetlist;
extern ValueList saved_triggers;
extern ValueList new_player_join_ref;
extern PlayerSaveList playerSaveList;

extern char global_map[1024];
extern char last_map[1024];
extern char next_map[1024];
extern bool transition;
extern bool savegame;
extern bool savegame_lock;
extern bool restoring;
extern bool protect_player;
extern bool restore_delay;
extern bool restore_delay_lock;
extern bool hasSavedOnce;
extern bool reset_viewcontrol;
extern bool sdktools_passed;
extern bool saving_game_rightnow;
extern int save_frames;
extern int restore_frames;
extern int after_restore_frames;
extern int game_start_frames;
extern int car_delay_for_save;
extern bool removing_ents_restore;
extern int restore_start_delay;
extern bool player_restore_failed;

extern void* delete_operator_array_addr;
extern void* delete_operator_addr;
extern void* new_operator_addr;
extern void* new_operator_array_addr;
extern void* strcpy_chk_addr;

extern pthread_mutex_t playerDeathQueueLock;
extern pthread_mutex_t collisionListLock;

extern uint32_t sv;
extern uint32_t g_ModelLoader;
extern uint32_t g_DataCache;
extern uint32_t g_MDLCache;
extern uint32_t s_ServerPlugin;
extern uint32_t SaveRestoreGlobal;
extern uint32_t weapon_substitute;
extern uint32_t fake_sequence_mem;

extern pOneArgProt UTIL_Remove__External;
extern pTwoArgProt CreateEntityByNameHook__External;
extern pOneArgProt CleanupDeleteListHook__External;
extern pThreeArgProt PlayerSpawnHook__External;
extern pOneArgProt UTIL_RemoveInternal__External;
extern pThreeArgProt MainPlayerRestore__External;

bool IsAllowedToPatchSdkTools(uint32_t lib_base, uint32_t lib_size);
uint32_t GetCBaseEntitySynergy(uint32_t EHandle);
uint32_t GetEntityField(uint32_t dmap, uint32_t firstEnt, uint32_t subdmap_offset, uint32_t deep, uint32_t searchField);
void PopulateHookExclusionListsSynergy();
bool hasTagAlreadyLoadedBefore(uint32_t arg0);
void SaveLinkedList(ValueList leakList);
void RestoreLinkedLists();
void DestroyLinkedLists();
void SaveProcessId();
int ReleaseLeakedMemory(ValueList leakList, bool destroy, uint32_t current_cap, uint32_t allowed_cap, uint32_t free_perc);
void ReleasePlayerSavedList();
FieldList SaveEntityFields(uint32_t dmap, uint32_t firstEnt, uint32_t subdmap_offset, uint32_t deep, FieldList fieldList);
void SendEntityInput(uint32_t ref_handle, uint32_t inputName, uint32_t activator, uint32_t caller, uint32_t val, uint32_t outputId);
SavedEntity* SaveEntity(uint32_t firstEnt);
void DisablePlayerViewControl(uint32_t viewcontrol);
void EnablePlayerViewControl(uint32_t viewcontrol);
int GetTotalClientsInGame();
void catoutputs(char* destination, char* source);
void UpdateOnRemoveDelayedEntities();
void SaveTriggersDatamaps();
void ReleaseSavedTriggers();
void RebuildSavedTriggers();
void SavePlayers();
void FlushCollisionChanges();
void RemoveInvalidEntries();
uint32_t IsEntityCollisionReady(uint32_t refHandle);
void PopCollisionChanges();
void ResetViewcontrolFromList();
void EnableViewControl(uint32_t viewControl);
void DisableViewControls();
void InsertViewcontrolsToResetList();
uint32_t SaveGameSafe(bool use_internal_savename);
void FixModelnameSlashes();
void GivePlayerWeapons(uint32_t player_object, bool force_give);
void FlushPlayerDeaths();
void TriggerMovedExtension(uint32_t pTrigger);
bool HandleSourcemodOutputHook(uint32_t arg0, uint32_t player, uint32_t pTrigger, uint32_t outputName);
void TriggerMovedFailsafe();
bool isAnyClientConnecting();
bool isCollisionListEmpty();
void ReleaseLeakedPackedEntities();
void RestorePlayers();
void SaveGame_Extension();
void DestroyVObjectForMarkedEnts();
void UpdatePlayersDonor();
void InitCoreSynergy();
void RemoveEntityNormalSynergy(uint32_t entity_object, bool validate);
void InstaKillSynergy(uint32_t entity_object, bool validate);
void AttemptToRestoreGame();
void ResetView();
bool IsSynergyMemoryCorrect();
void ForceSynergyMemoryCorrection();
void CorrectNpcAi(uint32_t arg0);

#endif