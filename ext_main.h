#ifndef EXT_MAIN_H
#define EXT_MAIN_H

#include "extension.h"
#include "core.h"

typedef uintptr_t(*pThreeArgProt)(uintptr_t, uintptr_t, uintptr_t);
typedef uintptr_t(__thiscall* pTwoArgProtThis)(uintptr_t, uintptr_t);

class Hooks {
public:
	static uintptr_t MountGamePathsHook(uintptr_t arg0, uintptr_t arg1, uintptr_t arg2);
};

void InitExtension();
void ApplyPatches();

#endif