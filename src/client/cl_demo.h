#pragma once

#ifndef KISAK_SP
#error This file is for SinglePlayer only
#endif

#include <universal/memfile.h>
#include <qcommon/ent.h>
#include <qcommon/msg_mp.h> // LWSS: this is OK, msg_t matches

void __cdecl CL_WriteDemoShortCString(MemoryFile *memFile, const char *string);
const char *__cdecl CL_ReadDemoShortCString(MemoryFile *memFile, char *string);
void __cdecl CL_WriteDemoDObjModel(MemoryFile *memFile, const DObjModel_s *dobjModel);
void __cdecl CL_ReadDemoDObjModel(MemoryFile *memFile, DObjModel_s *dobjModel);
void __cdecl CL_WriteAnimTree(MemoryFile *memFile, int entnum, const XAnimTree_s *tree);
XAnimTree_s *__cdecl CL_ReadAnimTree(MemoryFile *memFile, int entnum);
void __cdecl CL_WriteDemoDObj(int entnum, const DObj_s *obj);
void __cdecl CL_ReadDemoDObj(int entnum);
int CL_WriteDemoDObjs();
void CL_ClearDemoDObjs();
int CL_ReadDemoDObjs();
void __cdecl CL_WriteDemoEntityState(const entityState_s *es);
void __cdecl CL_ReadDemoEntityState(entityState_s *es);
void __cdecl CL_WriteDemoSnapshotData();
void __cdecl CL_ReadDemoSnapshotData();
void __cdecl CL_WriteDemoMessage(msg_t *msg, int headerBytes);
void __cdecl CL_StopRecord_f();
void __cdecl CL_Record_f();
void CL_DemoPlaybackStartup();
void __cdecl CL_PlayDemo_f();
void __cdecl CL_CheckStartPlayingDemo();
void __cdecl CL_NextDemo();
int __cdecl CL_DemoPlaybackTime();
bool __cdecl CL_DemoPlaying();
bool __cdecl CL_DemoRecording();
int __cdecl CL_TimeDemoPlaying();
void CL_DemoCompleted();
int __cdecl CL_GetDemoMessage(msg_t *buf, unsigned __int8 *bufData, int bufDataSize);
void __cdecl CL_ReadDemoMessage();
void __cdecl CL_ReadDemoMessagesUntilNextSnap();
void __cdecl CL_FinishLoadingDemo();
void __cdecl CL_StartPlayingDemo();
