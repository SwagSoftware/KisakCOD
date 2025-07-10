#pragma once

#ifndef KISAK_SP 
#error This file is for SinglePlayer only 
#endif

#include "xanim.h"

void __fastcall XAnimArchiveAnimState(XAnimState *state, MemoryFile *memFile);
void __fastcall XAnimLoadAnimInfo(XAnimInfo *info, MemoryFile *memFile);
void __fastcall XAnimSaveAnimInfo(XAnimInfo *info, MemoryFile *memFile);
void __fastcall XAnimLoadAnimTree(DObj_s *obj, MemoryFile *memFile);
void __fastcall XAnimSaveAnimTree_r(const XAnimTree_s *tree, MemoryFile *memFile, int infoIndex);
void __fastcall XAnimSaveAnimTree(const DObj_s *obj, MemoryFile *memFile);
