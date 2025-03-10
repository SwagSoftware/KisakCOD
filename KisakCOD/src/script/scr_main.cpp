#include "scr_main.h"
#include "scr_stringlist.h"

#include <universal/assertive.h>
#include <universal/q_shared.h>

bool Scr_IsInOpcodeMemory(char const* pos)
{
    iassert(scrVarPub.programBuffer);
    iassert(pos);
    return pos - scrVarPub.programBuffer < scrCompilePub.programLen;
}

bool Scr_IsIdentifier(char const* token)
{
    while (*token)
    {
        if (!I_iscsym(*token))
            return 0;
        ++token;
    }
    return 1;
}

unsigned int SL_TransferToCanonicalString(unsigned int stringValue)
{
	iassert(stringValue);
	SL_TransferRefToUser(stringValue, 2u);

	if (scrCompilePub.canonicalStrings[stringValue])
		return scrCompilePub.canonicalStrings[stringValue];

	scrCompilePub.canonicalStrings[stringValue] = ++scrVarPub.canonicalStrCount;

	return scrVarPub.canonicalStrCount;
}

unsigned int __cdecl SL_GetCanonicalString(char* str)
{
    unsigned int v1; // eax
    unsigned int v3; // eax

    v1 = SL_FindString(str);
    if (scrCompilePub.canonicalStrings[v1])
        return scrCompilePub.canonicalStrings[v1];
    v3 = SL_GetString_(str, 0, 16);
    return SL_TransferToCanonicalString(v3);
}

void SL_BeginLoadScripts()
{
    memset((unsigned __int8*)scrCompilePub.canonicalStrings, 0, sizeof(scrCompilePub.canonicalStrings));
    scrVarPub.canonicalStrCount = 0;
}