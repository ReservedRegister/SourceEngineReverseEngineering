#include "extension.h"

#ifndef CORE_H
#define CORE_H

#include <sys/mman.h>
#include <link.h>

#define HOOK_MSG "Saved memory reference to leaked resources list: [%X]"
#define EXT_PREFIX "[BlackMesaUtils] "

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

extern uint32_t engine_srv;
extern uint32_t server_srv;
extern uint32_t materialsystem_srv;
extern uint32_t vphysics_srv;
extern uint32_t dedicated_srv;
extern uint32_t datacache_srv;

extern uint32_t engine_srv_size;
extern uint32_t server_srv_size;
extern uint32_t materialsystem_srv_size;
extern uint32_t vphysics_srv_size;
extern uint32_t dedicated_srv_size;
extern uint32_t datacache_srv_size;

extern pThreeArgProt FindEntityByClassname;

extern uint32_t CGlobalEntityList;
extern int hooked_delete_counter;
extern int normal_delete_counter;
extern bool isTicking;
extern bool disable_delete_list;
extern ValueList deleteList;


void* copy_val(void* val, size_t copy_size);
void ForceMemoryAccess();
Library* getlibrary(char* file_line);
Library* LoadLibrary();
Library* FindLibrary(char* lib_name, bool less_intense_search);
void PopulateHookExclusionLists();
bool IsAddressExcluded(uint32_t base_address, uint32_t search_address);
uint32_t GetCBaseEntity(uint32_t EHandle);
void AllowWriteToMappedMemory();
void RestoreMemoryProtections();
void HookFunctionInSharedObject(uint32_t base_address, uint32_t size, void* target_pointer, void* hook_pointer);

ValueList AllocateValuesList();
Value* CreateNewValue(void* valueInput);
void DeleteAllValuesInList(ValueList list, bool free_val, pthread_mutex_t* lockInput);
bool IsInValuesList(ValueList list, void* searchVal, pthread_mutex_t* lockInput);
bool RemoveFromValuesList(ValueList list, void* searchVal, pthread_mutex_t* lockInput);
bool InsertToValuesList(ValueList list, Value* head, pthread_mutex_t* lockInput, bool tail, bool duplicate_chk);
int ValueListItems(ValueList list, pthread_mutex_t* lockInput);

uint32_t IsEntityValid(uint32_t refHandle);
void DestroyVObjectForMarkedEnts();

#endif