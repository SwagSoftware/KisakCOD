#pragma once

#ifndef KISAK_SP
#error This file is for SinglePlayer only
#endif

#include <bgame/bg_local.h>
#include <client/client.h>

void __fastcall CG_ShutdownEntity(int localClientNum, centity_s *cent);
void __fastcall CG_InitEntity(centity_s *cent);
void __fastcall CG_ResetEntity(int localClientNum, centity_s *cent);
void __fastcall CG_SetInitialSnapshot(int localClientNum);
int __fastcall CG_DObjCloneToBuffer(int localClientNum, centity_s *cent, const XAnimTree_s *serverTree);
void __fastcall CG_FreeTree(XAnimTree_s *tree, centity_s *cent);
void __fastcall CG_UpdateSnapshotNum(int localClientNum);
snapshot_s *__fastcall CG_ReadNextSnapshot(int localClientNum);
void __fastcall CG_CheckSnapshot(int localClientNum, const char *caller);
void __fastcall CG_ServerDObjClean(int entnum);
void __fastcall CG_SetNextSnap(int localClientNum);
void __fastcall CG_ProcessNextSnap(int localClientNum);
void __fastcall CG_CreateNextSnap(int localClientNum, double dtime, int readNext, int a4);
void __fastcall CG_FirstSnapshot(int localClientNum, int a2);
void __fastcall CG_ProcessDemoSnapshots(int localClientNum, int a2);
void __fastcall CG_ProcessSnapshots(int localClientNum);
