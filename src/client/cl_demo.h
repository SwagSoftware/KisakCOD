#pragma once

#ifndef KISAK_SP
#error This file is for SinglePlayer only
#endif

#include <universal/memfile.h>
#include <qcommon/ent.h>
#include <qcommon/msg_mp.h> // LWSS: this is OK, msg_t matches

void __fastcall CL_WriteDemoShortCString(MemoryFile *memFile, const char *string);
const char *__fastcall CL_ReadDemoShortCString(MemoryFile *memFile, char *string);
void __fastcall CL_WriteDemoDObjModel(MemoryFile *memFile, const DObjModel_s *dobjModel);
void __fastcall CL_ReadDemoDObjModel(MemoryFile *memFile, DObjModel_s *dobjModel);
void __fastcall CL_WriteAnimTree(MemoryFile *memFile, int entnum, const XAnimTree_s *tree);
XAnimTree_s *__fastcall CL_ReadAnimTree(MemoryFile *memFile, int entnum);
void __fastcall CL_WriteDemoDObj(int entnum, const DObj_s *obj);
void __fastcall CL_ReadDemoDObj(int entnum);
int CL_WriteDemoDObjs();
void CL_ClearDemoDObjs();
int CL_ReadDemoDObjs();
void __fastcall CL_WriteDemoEntityState(const entityState_s *es);
void __fastcall CL_ReadDemoEntityState(entityState_s *es);
void __fastcall CL_WriteDemoSnapshotData();
void __fastcall CL_ReadDemoSnapshotData();
void __fastcall CL_WriteDemoMessage(msg_t *msg, int headerBytes);
void __fastcall CL_StopRecord_f();
void __fastcall CL_Record_f();
void CL_DemoPlaybackStartup();
void __fastcall CL_PlayDemo_f();
void __fastcall CL_CheckStartPlayingDemo();
void __fastcall CL_NextDemo();
int __fastcall CL_DemoPlaybackTime();
bool __fastcall CL_DemoPlaying();
bool __fastcall CL_DemoRecording();
int __fastcall CL_TimeDemoPlaying();
void CL_DemoCompleted();
int __fastcall CL_GetDemoMessage(msg_t *buf, unsigned __int8 *bufData, int bufDataSize);
void __fastcall CL_ReadDemoMessage();
void __fastcall CL_ReadDemoMessagesUntilNextSnap();
void __fastcall CL_FinishLoadingDemo();
void __fastcall CL_StartPlayingDemo();
