#include "database.h"

void __cdecl Load_ScriptStringCustom(uint16_t *var)
{
    *var = (uint16_t)varXAssetList->stringList.strings[*var];
}

void __cdecl Mark_ScriptStringCustom(uint16_t *var)
{
    if (*var)
        SL_AddUser(*var, 4u);
}

