#pragma once

#ifndef KISAK_SP 
#error This file is for SinglePlayer only 
#endif

#include "xanim.h"

void __cdecl XAnimArchiveAnimState(XAnimState *state, MemoryFile *memFile);
void __cdecl XAnimLoadAnimInfo(XAnimInfo *info, MemoryFile *memFile);
void __cdecl XAnimSaveAnimInfo(XAnimInfo *info, MemoryFile *memFile);
void __cdecl XAnimLoadAnimTree(DObj_s *obj, MemoryFile *memFile);
void __cdecl XAnimSaveAnimTree_r(const XAnimTree_s *tree, MemoryFile *memFile, int infoIndex);
void __cdecl XAnimSaveAnimTree(const DObj_s *obj, MemoryFile *memFile);
