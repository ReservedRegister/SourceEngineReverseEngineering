#ifndef UTIL_H
#define UTIL_H

#include <link.h>

typedef uint32_t (*pZeroArgProt)();
typedef uint32_t (*pOneArgProt)(uint32_t);
typedef uint32_t (*pTwoArgProt)(uint32_t, uint32_t);
typedef uint32_t (*pThreeArgProt)(uint32_t, uint32_t, uint32_t);
typedef uint32_t (*pFourArgProt)(uint32_t, uint32_t, uint32_t, uint32_t);
typedef uint32_t (*pFiveArgProt)(uint32_t, uint32_t, uint32_t, uint32_t, uint32_t);
typedef uint32_t (*pSixArgProt)(uint32_t, uint32_t, uint32_t, uint32_t, uint32_t, uint32_t);
typedef uint32_t (*pSevenArgProt)(uint32_t, uint32_t, uint32_t, uint32_t, uint32_t, uint32_t, uint32_t);
typedef uint32_t (*pNineArgProt)(uint32_t, uint32_t, uint32_t, uint32_t, uint32_t, uint32_t, uint32_t, uint32_t, uint32_t);
typedef uint32_t (*pElevenArgProt)(uint32_t, uint32_t, uint32_t, uint32_t, uint32_t, uint32_t, uint32_t, uint32_t, uint32_t, uint32_t, uint32_t);

typedef uint32_t (__attribute__((regparm(2))) *pTwoArgProtOptLink)(uint32_t, uint32_t);

enum MDLCacheFlush_t
{
	MDLCACHE_FLUSH_STUDIOHDR		= 0x01,
	MDLCACHE_FLUSH_STUDIOHWDATA		= 0x02,
	MDLCACHE_FLUSH_VCOLLIDE			= 0x04,
	MDLCACHE_FLUSH_ANIMBLOCK		= 0x08,
	MDLCACHE_FLUSH_VIRTUALMODEL		= 0x10,
	MDLCACHE_FLUSH_AUTOPLAY         = 0x20,
	MDLCACHE_FLUSH_VERTEXES         = 0x40,

	MDLCACHE_FLUSH_IGNORELOCK       = 0x80000000,
	MDLCACHE_FLUSH_ALL              = 0xFFFFFFFF
};

typedef struct _game_offsets {
    uint32_t classname_offset;
    uint32_t abs_origin_offset;
    uint32_t abs_angles_offset;
	uint32_t abs_velocity_offset;
    uint32_t origin_offset;
    uint32_t refhandle_offset;
    uint32_t iserver_offset;
    uint32_t mnetwork_offset;
	uint32_t collision_property_offset;
	uint32_t m_CollisionGroup_offset;
} game_offsets;

typedef struct _game_functions {
    pTwoArgProt RemoveEntityNormal;
    pTwoArgProt InstaKill;
    pOneArgProt GetCBaseEntity;
    pOneArgProt IsMarkedForDeletion;
	pTwoArgProt SetSolidFlags;
	pTwoArgProt DisableEntityCollisions;
	pTwoArgProt EnableEntityCollisions;
} game_functions;

typedef struct _Vector {
	float x;
	float y;
	float z;
} Vector;

typedef struct _Library {
	struct link_map* library_linkmap;
	char* library_signature;
	uint32_t library_base_address;
	uint32_t library_size;
} Library;

typedef struct _Value {
	void* value;
	struct _Value* nextVal;
} Value;

typedef Value** ValueList;

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

extern game_offsets offsets;
extern game_functions functions;

extern bool loaded_extension;

extern uint32_t hook_exclude_list_offset[512];
extern uint32_t hook_exclude_list_base[512];
extern uint32_t memory_prots_save_list[512];
extern uint32_t our_libraries[512];
extern uint32_t loaded_libraries[512];
extern uint32_t collisions_entity_list[512];

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

extern bool isTicking;
extern bool disable_delete_list;
extern bool server_sleeping;
extern int hooked_delete_counter;
extern int normal_delete_counter;
extern uint32_t CGlobalEntityList;
extern uint32_t global_vpk_cache_buffer;
extern uint32_t current_vpk_buffer_ref;
extern ValueList leakedResourcesVpkSystem;

extern pOneArgProt CollisionRulesChanged;
extern pThreeArgProt FindEntityByClassname;

void* copy_val(void* val, size_t copy_size);
bool IsAddressExcluded(uint32_t base_address, uint32_t search_address);
void HookFunctionInSharedObject(uint32_t base_address, uint32_t size, void* target_pointer, void* hook_pointer);
Library* FindLibrary(char* lib_name, bool less_intense_search);
Library* LoadLibrary(char* library_full_path);
void ClearLoadedLibraries();
Library* getlibrary(char* file_line);
void AllowWriteToMappedMemory();
void ForceMemoryAccess();
void RestoreMemoryProtections();
bool IsVectorNaN(uint32_t base);
void UpdateAllCollisions();
void RemoveBadEnts();
bool IsEntityPositionReasonable(uint32_t v);
uint32_t IsEntityValid(uint32_t entity);
void LogVpkMemoryLeaks();
void FixPlayerCollisionGroup();

ValueList AllocateValuesList();
Value* CreateNewValue(void* valueInput);
int DeleteAllValuesInList(ValueList list, bool free_val, pthread_mutex_t* lockInput);
bool IsInValuesList(ValueList list, void* searchVal, pthread_mutex_t* lockInput);
bool RemoveFromValuesList(ValueList list, void* searchVal, pthread_mutex_t* lockInput);
int ValueListItems(ValueList list, pthread_mutex_t* lockInput);
bool InsertToValuesList(ValueList list, Value* head, pthread_mutex_t* lockInput, bool tail, bool duplicate_chk);
FieldList AllocateFieldList();
PlayerSaveList AllocatePlayerSaveList();
SavedEntity* CreateNewSavedEntity(void* entRefHandleInput, void* classnameInput, FieldList fieldListInput);
Field* CreateNewField(void* labelInput, void* keyInput, void* typeInput, void* flagsInput, void* offsetInput, ValueList valuesInput);
EntityKV* CreateNewEntityKV(uint32_t refHandle, uint32_t keyIn, uint32_t valueIn);
PlayerSave* CreateNewPlayerSave(SavedEntity* player_save_input);
void InsertFieldToFieldList(FieldList list, Field* head);
void InsertToPlayerSaveList(PlayerSaveList list, PlayerSave* head);
void InsertEntityToCollisionsList(uint32_t ent);
void DisablePlayerCollisions();
void DisablePlayerWorldSpawnCollision();

#endif