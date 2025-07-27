#pragma once
#include <universal/memfile.h>

#ifndef KISAK_SP 
#error This file is for SinglePlayer only 
#endif
#include "scr_variable.h"
#include <server/server.h> // SaveImmediate

void __cdecl WriteByte(unsigned __int8 b, MemoryFile *memFile);
void __cdecl WriteShort(unsigned __int16 i, MemoryFile *memFile);
void __cdecl WriteString(unsigned __int16 str, MemoryFile *memFile);
void __cdecl SafeWriteString(unsigned __int16 str, MemoryFile *memFile);
int __cdecl Scr_ReadString(MemoryFile *memFile);
int __cdecl Scr_ReadOptionalString(MemoryFile *memFile);
void __cdecl WriteInt(int i, MemoryFile *memFile);
void WriteFloat(float f, MemoryFile *memFile);
void __cdecl WriteVector(float *v, MemoryFile *memFile);
const float *__cdecl Scr_ReadVec3(MemoryFile *memFile);
void __cdecl WriteCodepos(const char *pos, MemoryFile *memFile);
const char *__cdecl Scr_ReadCodepos(MemoryFile *memFile);
unsigned int __cdecl Scr_CheckIdHistory(unsigned int index);
void __cdecl WriteId(unsigned int id, unsigned int opcode, MemoryFile *memFile);
unsigned int __cdecl Scr_ReadId(MemoryFile *memFile, unsigned int opcode);
void __cdecl WriteStack(const VariableStackBuffer *stackBuf, MemoryFile *memFile);
VariableStackBuffer *__cdecl Scr_ReadStack(MemoryFile *memFile);
void __cdecl Scr_DoLoadEntryInternal(VariableValue *value, MemoryFile *memFile);
int __cdecl Scr_DoLoadEntry(VariableValue *value, bool isArray, MemoryFile *memFile);
void __cdecl AddSaveObjectInternal(unsigned int parentId);
unsigned int __cdecl Scr_ConvertThreadFromLoad(unsigned __int16 handle);
void __cdecl Scr_DoLoadObjectInfo(unsigned __int16 parentId, MemoryFile *memFile);
void __cdecl Scr_ReadGameEntry(MemoryFile *memFile);
void __cdecl Scr_SaveShutdown(bool savegame);
void __cdecl Scr_LoadPre(int sys, MemoryFile *memFile);
void __cdecl Scr_LoadShutdown();
void __cdecl DoSaveEntryInternal(unsigned int type, VariableUnion *u, MemoryFile *memFile);
void __cdecl Scr_SaveSource(MemoryFile *memFile);
void __cdecl SaveMemory_SaveWriteImmediate(const void *buffer, unsigned int len, SaveImmediate *save);
void __cdecl Scr_SaveSourceImmediate(SaveImmediate *save);
void __cdecl Scr_LoadSource(MemoryFile *memFile, void *fileHandle);
void __cdecl Scr_SkipSource(MemoryFile *memFile, void *fileHandle);
void __cdecl AddSaveStackInternal(const VariableStackBuffer *stackBuf);
void __cdecl AddSaveEntryInternal(unsigned int type, const VariableStackBuffer *u);
// local variable allocation has failed, the output may be wrong!
void __cdecl DoSaveEntry(VariableValue *value, VariableValue *name, bool isArray, MemoryFile *memFile);
void __cdecl AddSaveObjectChildren(unsigned int parentId);
void __cdecl AddSaveObject(unsigned int parentId);
void __cdecl DoSaveObjectInfo(unsigned int parentId, MemoryFile *memFile);
int __cdecl Scr_ConvertThreadToSave(unsigned __int16 handle);
void __cdecl WriteGameEntry(MemoryFile *memFile);
void __cdecl Scr_SavePost(MemoryFile *memFile);
void __cdecl AddSaveStack(const VariableStackBuffer *stackBuf);
void __cdecl AddSaveEntry(unsigned int type, const VariableStackBuffer *u);
void __cdecl Scr_SavePre(int sys);
