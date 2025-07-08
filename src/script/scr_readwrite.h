#pragma once
#include <universal/memfile.h>

#ifndef KISAK_SP 
#error This file is for SinglePlayer only 
#endif
#include "scr_variable.h"
#include <server/server.h> // SaveImmediate

void __fastcall WriteByte(unsigned __int8 b, MemoryFile *memFile);
void __fastcall WriteShort(unsigned __int16 i, MemoryFile *memFile);
void __fastcall WriteString(unsigned __int16 str, MemoryFile *memFile);
void __fastcall SafeWriteString(unsigned __int16 str, MemoryFile *memFile);
int __fastcall Scr_ReadString(MemoryFile *memFile);
int __fastcall Scr_ReadOptionalString(MemoryFile *memFile);
void __fastcall WriteInt(int i, MemoryFile *memFile);
void WriteFloat(float f, MemoryFile *memFile);
void __fastcall WriteVector(float *v, MemoryFile *memFile);
const float *__fastcall Scr_ReadVec3(MemoryFile *memFile);
void __fastcall WriteCodepos(const char *pos, MemoryFile *memFile);
const char *__fastcall Scr_ReadCodepos(MemoryFile *memFile);
unsigned int __fastcall Scr_CheckIdHistory(unsigned int index);
void __fastcall WriteId(unsigned int id, unsigned int opcode, MemoryFile *memFile);
unsigned int __fastcall Scr_ReadId(MemoryFile *memFile, unsigned int opcode);
void __fastcall WriteStack(const VariableStackBuffer *stackBuf, MemoryFile *memFile);
VariableStackBuffer *__fastcall Scr_ReadStack(MemoryFile *memFile);
void __fastcall Scr_DoLoadEntryInternal(VariableValue *value, MemoryFile *memFile);
int __fastcall Scr_DoLoadEntry(VariableValue *value, bool isArray, MemoryFile *memFile);
void __fastcall AddSaveObjectInternal(unsigned int parentId);
unsigned int __fastcall Scr_ConvertThreadFromLoad(unsigned __int16 handle);
void __fastcall Scr_DoLoadObjectInfo(unsigned __int16 parentId, MemoryFile *memFile);
void __fastcall Scr_ReadGameEntry(MemoryFile *memFile);
void __fastcall Scr_SaveShutdown(bool savegame);
void __fastcall Scr_LoadPre(int sys, MemoryFile *memFile);
void __fastcall Scr_LoadShutdown();
void __fastcall DoSaveEntryInternal(unsigned int type, VariableUnion *u, MemoryFile *memFile);
void __fastcall Scr_SaveSource(MemoryFile *memFile);
void __fastcall SaveMemory_SaveWriteImmediate(const void *buffer, unsigned int len, SaveImmediate *save);
void __fastcall Scr_SaveSourceImmediate(SaveImmediate *save);
void __fastcall Scr_LoadSource(MemoryFile *memFile, void *fileHandle);
void __fastcall Scr_SkipSource(MemoryFile *memFile, void *fileHandle);
void __fastcall AddSaveStackInternal(const VariableStackBuffer *stackBuf);
void __fastcall AddSaveEntryInternal(unsigned int type, const VariableStackBuffer *u);
// local variable allocation has failed, the output may be wrong!
void __fastcall DoSaveEntry(VariableValue *value, VariableValue *name, bool isArray, MemoryFile *memFile);
void __fastcall AddSaveObjectChildren(unsigned int parentId);
void __fastcall AddSaveObject(unsigned int parentId);
void __fastcall DoSaveObjectInfo(unsigned int parentId, MemoryFile *memFile);
int __fastcall Scr_ConvertThreadToSave(unsigned __int16 handle);
void __fastcall WriteGameEntry(MemoryFile *memFile);
void __fastcall Scr_SavePost(MemoryFile *memFile);
void __fastcall AddSaveStack(const VariableStackBuffer *stackBuf);
void __fastcall AddSaveEntry(unsigned int type, const VariableStackBuffer *u);
void __fastcall Scr_SavePre(int sys);
