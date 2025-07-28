#pragma once

#ifndef KISAK_SP
#error This file is for SinglePlayer only
#endif

#include <bgame/bg_local.h>
#include <client/client.h>

void __cdecl CG_ShutdownEntity(int localClientNum, centity_s *cent);
void __cdecl CG_InitEntity(centity_s *cent);
void __cdecl CG_ResetEntity(int localClientNum, centity_s *cent);
void __cdecl CG_SetInitialSnapshot(int localClientNum);
int __cdecl CG_DObjCloneToBuffer(int localClientNum, centity_s *cent, const XAnimTree_s *serverTree);
void __cdecl CG_FreeTree(XAnimTree_s *tree, centity_s *cent);
void __cdecl CG_UpdateSnapshotNum(int localClientNum);
snapshot_s *__cdecl CG_ReadNextSnapshot(int localClientNum);
void __cdecl CG_CheckSnapshot(int localClientNum, const char *caller);
void __cdecl CG_ServerDObjClean(int entnum);
void __cdecl CG_SetNextSnap(int localClientNum);
void __cdecl CG_ProcessNextSnap(int localClientNum);
void __cdecl CG_CreateNextSnap(int localClientNum, double dtime, int readNext);
void __cdecl CG_FirstSnapshot(int localClientNum);
void __cdecl CG_ProcessDemoSnapshots(int localClientNum);
void __cdecl CG_ProcessSnapshots(int localClientNum);
