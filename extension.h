#include <sys/mman.h>
#include <link.h>

#include "sdktools.h"

#define HOOK_MSG "Saved memory reference to leaked resources list: [%X]"
#define EXT_PREFIX "[SynergyUtils] "

typedef struct _Value {
	void* value;
	struct _Value* nextVal;
} Value;

typedef Value** ValueList;

typedef struct _MallocRef {
	void* ref;
	void* ref_size;
	void* alloc_location;
	void* alloc_type;
	struct _MallocRef* nextRef;
} MallocRef;

typedef MallocRef** MallocRefList;

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

typedef uint32_t (*pZeroArgProt)();
typedef uint32_t (*pOneArgProt)(uint32_t);
typedef uint32_t (*pTwoArgProt)(uint32_t, uint32_t);
typedef uint32_t (*pThreeArgProt)(uint32_t, uint32_t, uint32_t);
typedef uint32_t (*pFourArgProt)(uint32_t, uint32_t, uint32_t, uint32_t);
typedef uint32_t (*pFiveArgProt)(uint32_t, uint32_t, uint32_t, uint32_t, uint32_t);
typedef uint32_t (*pSixArgProt)(uint32_t, uint32_t, uint32_t, uint32_t, uint32_t, uint32_t);
typedef uint32_t (*pSevenArgProt)(uint32_t, uint32_t, uint32_t, uint32_t, uint32_t, uint32_t, uint32_t);

/**
 * vim: set ts=4 :
 * =============================================================================
 * SourceMod Sample Extension
 * Copyright (C) 2004-2008 AlliedModders LLC.  All rights reserved.
 * =============================================================================
 *
 * This program is free software; you can redistribute it and/or modify it under
 * the terms of the GNU General Public License, version 3.0, as published by the
 * Free Software Foundation.
 * 
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more
 * details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * As a special exception, AlliedModders LLC gives you permission to link the
 * code of this program (as well as its derivative works) to "Half-Life 2," the
 * "Source Engine," the "SourcePawn JIT," and any Game MODs that run on software
 * by the Valve Corporation.  You must obey the GNU General Public License in
 * all respects for all other code used.  Additionally, AlliedModders LLC grants
 * this exception to all derivative works.  AlliedModders LLC defines further
 * exceptions, found in LICENSE.txt (as of this writing, version JULY-31-2007),
 * or <http://www.sourcemod.net/license.php>.
 *
 * Version: $Id$
 */

#ifndef _INCLUDE_SOURCEMOD_EXTENSION_PROPER_H_
#define _INCLUDE_SOURCEMOD_EXTENSION_PROPER_H_

/**
 * @file extension.h
 * @brief Sample extension code header.
 */

#include "smsdk_ext.h"


/**
 * @brief Sample implementation of the SDK Extension.
 * Note: Uncomment one of the pre-defined virtual functions in order to use it.
 */
class SynergyUtils : public SDKExtension
{
public:
	/**
	 * @brief This is called after the initial loading sequence has been processed.
	 *
	 * @param error		Error message buffer.
	 * @param maxlen	Size of error message buffer.
	 * @param late		Whether or not the module was loaded after map load.
	 * @return			True to succeed loading, false to fail.
	 */
	//virtual bool SDK_OnLoad(char *error, size_t maxlen, bool late);
	
	/**
	 * @brief This is called right before the extension is unloaded.
	 */
	//virtual void SDK_OnUnload();

	/**
	 * @brief This is called once all known extensions have been loaded.
	 * Note: It is is a good idea to add natives here, if any are provided.
	 */
	virtual void SDK_OnAllLoaded();

	/**
	 * @brief Called when the pause state is changed.
	 */
	//virtual void SDK_OnPauseChange(bool paused);

	/**
	 * @brief this is called when Core wants to know if your extension is working.
	 *
	 * @param error		Error message buffer.
	 * @param maxlen	Size of error message buffer.
	 * @return			True if working, false otherwise.
	 */
	//virtual bool QueryRunning(char *error, size_t maxlen);
public:
#if defined SMEXT_CONF_METAMOD
	/**
	 * @brief Called when Metamod is attached, before the extension version is called.
	 *
	 * @param error			Error buffer.
	 * @param maxlen		Maximum size of error buffer.
	 * @param late			Whether or not Metamod considers this a late load.
	 * @return				True to succeed, false to fail.
	 */
	//virtual bool SDK_OnMetamodLoad(ISmmAPI *ismm, char *error, size_t maxlen, bool late);

	/**
	 * @brief Called when Metamod is detaching, after the extension version is called.
	 * NOTE: By default this is blocked unless sent from SourceMod.
	 *
	 * @param error			Error buffer.
	 * @param maxlen		Maximum size of error buffer.
	 * @return				True to succeed, false to fail.
	 */
	//virtual bool SDK_OnMetamodUnload(char *error, size_t maxlen);

	/**
	 * @brief Called when Metamod's pause state is changing.
	 * NOTE: By default this is blocked unless sent from SourceMod.
	 *
	 * @param paused		Pause state being set.
	 * @param error			Error buffer.
	 * @param maxlen		Maximum size of error buffer.
	 * @return				True to succeed, false to fail.
	 */
	//virtual bool SDK_OnMetamodPauseChange(bool paused, char *error, size_t maxlen);
#endif
};

#endif // _INCLUDE_SOURCEMOD_EXTENSION_PROPER_H_