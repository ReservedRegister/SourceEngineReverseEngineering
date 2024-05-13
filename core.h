#ifndef CORE_H
#define CORE_H

#define HOOK_MSG "Saved memory reference to leaked resources list: [%X]"
#define EXT_PREFIX "[SynergyUtils] "

typedef uint32_t (*pZeroArgProt)();
typedef uint32_t (*pOneArgProt)(uint32_t);
typedef uint32_t (*pTwoArgProt)(uint32_t, uint32_t);
typedef uint32_t (*pThreeArgProt)(uint32_t, uint32_t, uint32_t);
typedef uint32_t (*pFourArgProt)(uint32_t, uint32_t, uint32_t, uint32_t);
typedef uint32_t (*pFiveArgProt)(uint32_t, uint32_t, uint32_t, uint32_t, uint32_t);
typedef uint32_t (*pSixArgProt)(uint32_t, uint32_t, uint32_t, uint32_t, uint32_t, uint32_t);
typedef uint32_t (*pSevenArgProt)(uint32_t, uint32_t, uint32_t, uint32_t, uint32_t, uint32_t, uint32_t);

typedef uint32_t (__attribute__((regparm(2))) *pTwoArgProtOptLink)(uint32_t, uint32_t);

typedef struct _Library {
	char* library_signature;
	uint32_t library_base_address;
	uint32_t library_size;
} Library;

typedef struct _Value {
	void* value;
	struct _Value* nextVal;
} Value;

typedef Value** ValueList;

typedef struct _VpkMemoryLeak {
	uint32_t packed_ref;
	ValueList leaked_refs;
} VpkMemoryLeak;

typedef struct _EntityFrameCount {
	uint32_t entity_ref;
	int frames;
} EntityFrameCount;

typedef struct _EntityOrigin {
	uint32_t refHandle;
	float x;
	float y;
	float z;
} EntityOrigin;

typedef struct _Vector {
	float x;
	float y;
	float z;
} Vector;

typedef struct _EntityKV {
	uint32_t entityRef;
	uint32_t key;
	uint32_t value;
} EntityKV;

typedef struct _Field {
	void* label;
	void* key;
	void* type;
	void* flags;
	void* offset;
	ValueList fieldVals;
	struct _Field* nextField;
} Field;

typedef Field** FieldList;

typedef struct _SavedEntity {
	void* refHandle;
	void* clsname;
	FieldList fieldData;
	struct _SavedEntity* nextEnt;
} SavedEntity;

typedef SavedEntity** SaveData;

typedef struct _PlayerSave {
	SavedEntity* saved_player;
	struct _PlayerSave* nextPlayer;
} PlayerSave;

typedef PlayerSave** PlayerSaveList;

extern uint32_t engine_srv;
extern uint32_t datacache_srv;
extern uint32_t dedicated_srv;
extern uint32_t materialsystem_srv;
extern uint32_t vphysics_srv;
extern uint32_t scenefilecache;
extern uint32_t soundemittersystem;
extern uint32_t soundemittersystem_srv;
extern uint32_t studiorender_srv;
extern uint32_t server_srv;
extern uint32_t sdktools;

extern uint32_t engine_srv_size;
extern uint32_t datacache_srv_size;
extern uint32_t dedicated_srv_size;
extern uint32_t materialsystem_srv_size;
extern uint32_t vphysics_srv_size;
extern uint32_t scenefilecache_size;
extern uint32_t soundemittersystem_size;
extern uint32_t soundemittersystem_srv_size;
extern uint32_t studiorender_srv_size;
extern uint32_t server_srv_size;
extern uint32_t sdktools_size;

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
extern ValueList leakedResourcesVpkSystem;
extern ValueList leakedResourcesEdtSystem;
extern ValueList antiCycleListDoors;
extern ValueList entityDeleteList;
extern ValueList playerDeathQueue;
extern ValueList collisionList;
extern ValueList cmdbufflist;
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
extern bool disable_delete_list;
extern bool isTicking;
extern bool hasSavedOnce;
extern bool firstplayer_hasjoined;
extern bool mapHasEnded;
extern bool mapHasEndedDelay;
extern bool reset_viewcontrol;
extern bool sdktools_passed;
extern bool saving_game_rightnow;
extern int hooked_delete_counter;
extern int normal_delete_counter;
extern int save_frames;
extern int restore_frames;
extern int delay_frames;
extern int mapHasEndedDelayFrames;
extern int enqueue_delay_frames;
extern int after_restore_frames;
extern int game_start_frames;
extern bool server_sleeping;
extern int car_delay_for_save;
extern bool removing_ents_restore;
extern int restore_start_delay;
extern bool player_restore_failed;
extern int waiting_shoot_frames;

extern void* delete_operator_array_addr;
extern void* delete_operator_addr;
extern void* new_operator_addr;
extern void* new_operator_array_addr;
extern void* strcpy_chk_addr;

extern pthread_mutex_t playerDeathQueueLock;
extern pthread_mutex_t collisionListLock;
extern pthread_mutex_t cmdbufflistlock;

extern uint32_t CGlobalEntityList;
extern uint32_t sv;
extern uint32_t g_ModelLoader;
extern uint32_t g_DataCache;
extern uint32_t g_MDLCache;
extern uint32_t s_ServerPlugin;
extern uint32_t SaveRestoreGlobal;
extern uint32_t weapon_substitute;
extern uint32_t fake_sequence_mem;

extern pOneArgProt UTIL_Remove__External;
extern pThreeArgProt FindEntityByClassnameHook__External;
extern pTwoArgProt CreateEntityByNameHook__External;
extern pOneArgProt CleanupDeleteListHook__External;
extern pThreeArgProt PlayerSpawnHook__External;
extern pOneArgProt UTIL_RemoveInternal__External;
extern pThreeArgProt MainPlayerRestore__External;

bool IsAllowedToPatchSdkTools(uint32_t lib_base, uint32_t lib_size);
void ForceMemoryAccess();
Library* getlibrary(char* file_line);
Library* LoadLibrary();
Library* FindLibrary(char* lib_name, bool less_intense_search);
uint32_t GetFileSize(char* file_name);
uint32_t GetCBaseEntity(uint32_t EHandle);
uint32_t GetEntityField(uint32_t dmap, uint32_t firstEnt, uint32_t subdmap_offset, uint32_t deep, uint32_t searchField);
void* copy_val(void* val, size_t copy_size);
void AllowWriteToMappedMemory();
void RestoreMemoryProtections();
void HookFunctionInSharedObject(uint32_t base_address, uint32_t size, void* target_pointer, void* hook_pointer);
bool IsAddressExcluded(uint32_t base_address, uint32_t search_address);
void PopulateHookExclusionLists();
EntityKV* CreateNewEntityKV(uint32_t refHandle, uint32_t keyIn, uint32_t valueIn);
ValueList AllocateValuesList();
FieldList AllocateFieldList();
PlayerSaveList AllocatePlayerSaveList();
SavedEntity* CreateNewSavedEntity(void* entRefHandleInput, void* classnameInput, FieldList fieldListInput);
Field* CreateNewField(void* labelInput, void* keyInput, void* typeInput, void* flagsInput, void* offsetInput, ValueList valuesInput);
Value* CreateNewValue(void* valueInput);
PlayerSave* CreateNewPlayerSave(SavedEntity* player_save_input);
void InsertFieldToFieldList(FieldList list, Field* head);
int DeleteAllValuesInList(ValueList list, pthread_mutex_t* passed_lock, bool free_val);
bool IsInValuesList(ValueList list, void* searchVal, pthread_mutex_t* passed_lock);
bool RemoveFromValuesList(ValueList list, void* searchVal, pthread_mutex_t* passed_lock);
void InsertToValuesList(ValueList list, Value* head, pthread_mutex_t* passed_lock, bool tail, bool duplicate_chk);
void InsertToPlayerSaveList(PlayerSaveList list, PlayerSave* head);
int ValueListItems(ValueList list, pthread_mutex_t* passed_lock);
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
void ProcessChangelevelDelay();
bool isAnyClientConnecting();
bool isCollisionListEmpty();
void ReleaseLeakedPackedEntities();
void RestorePlayers();
void CleanPlayerEnts(bool no_parent);
void SaveGame_Extension();
uint32_t IsEntityValid(uint32_t entity);
void DestroyVObjectForMarkedEnts();
void UpdatePlayersDonor();
void InitCore();
void RemoveEntityNormal(uint32_t entity_object, bool validate);
void InstaKill(uint32_t entity_object, bool validate);
void LogVpkMemoryLeaks();
void AttemptToRestoreGame();
void ResetView();
bool IsSynergyMemoryCorrect();
void ForceSynergyMemoryCorrection();
void CorrectNpcAi(uint32_t arg0);
void RemoveBadEnts();
bool IsEntityPositionReasonable(uint32_t v);
void UpdateAllCollisions();

#endif