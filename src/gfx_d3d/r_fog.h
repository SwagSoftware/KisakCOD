#pragma once
#include <universal/memfile.h>


void __cdecl R_ClearFogs();
void __cdecl R_SetFogFromServer(float start, unsigned __int8 r, unsigned __int8 g, unsigned __int8 b, float density);
void __cdecl R_SwitchFog(unsigned int fogvar, int startTime, int transitionTime);
void __cdecl R_ArchiveFogState(MemoryFile *memFile);
