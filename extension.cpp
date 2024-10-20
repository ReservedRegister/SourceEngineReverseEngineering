#include "extension.h"

extern void InitExtensionBlackMesa();
extern void InitExtensionSynergy();

extern void InitUtil();

SynergyUtils g_SynUtils;		/**< Global singleton for extension's main interface */
SMEXT_LINK(&g_SynUtils);

void SynergyUtils::SDK_OnAllLoaded()
{
    InitUtil();
    
    InitExtensionBlackMesa();
    InitExtensionSynergy();
}