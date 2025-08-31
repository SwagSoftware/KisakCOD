#pragma once
#include <universal/memfile.h>

enum $CF43A0974C3EB2799D9079D7BDE5CE8D : __int32
{
	FOG_NONE = 0x0,
	FOG_SERVER = 0x1,
	FOG_CURRENT = 0x2,
	FOG_LAST = 0x3,
	FOG_TARGET = 0x4,
	FOG_COUNT = 0x5,
};

void __cdecl R_ClearFogs();
void __cdecl R_SetFogFromServer(float start, unsigned __int8 r, unsigned __int8 g, unsigned __int8 b, float density);
void __cdecl R_SwitchFog(unsigned int fogvar, int startTime, int transitionTime);
void __cdecl R_ArchiveFogState(MemoryFile *memFile);
