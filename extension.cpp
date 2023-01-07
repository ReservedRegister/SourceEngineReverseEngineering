#include "extension.h"
#include "ext_main.h"

BmsUtils g_BmsUtils;		/**< Global singleton for extension's main interface */
SMEXT_LINK(&g_BmsUtils);

void BmsUtils::SDK_OnAllLoaded()
{
    InitExtension();
}