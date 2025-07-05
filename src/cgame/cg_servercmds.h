#pragma once

#ifndef KISAK_SP
#error This file is for SinglePlayer only
#endif

#include "cg_local.h"

void __fastcall CG_ParseServerInfo(int localClientNum);
void __fastcall CG_ParseCullDist(int localClientNum);
void __fastcall CG_ParseSunLight(int localClientNum);
void __fastcall CG_ParseSunDirection(int localClientNum);
void __fastcall CG_ParseFog(int time);
void __fastcall CG_PrecacheScriptMenu(int localClientNum, int iConfigNum);
void __fastcall CG_RegisterServerMaterial(int localClientNum, int num);
void __fastcall CG_RegisterServerMaterials(int localClientNum);
void __fastcall CG_ConfigStringModifiedInternal(int localClientNum, unsigned int stringIndex);
void __fastcall CG_ConfigStringModified(int localClientNum);
void __fastcall CG_ShutdownPhysics(int localClientNum);
void __fastcall CG_OpenScriptMenu(int localClientNum);
void __fastcall CG_CheckOpenWaitingScriptMenu();
void __fastcall CG_CloseScriptMenu(bool allowResponse);
void __fastcall CG_MenuShowNotify(int localClientNum, int menuToShow);
void __fastcall CG_HudMenuShowAllTimed(int localClientNum);
void CG_EqCommand();
void CG_DeactivateEqCmd();
void CG_ReverbCmd();
void CG_DeactivateReverbCmd();
void __fastcall CG_SetChannelVolCmd(int localClientNum);
void CG_DeactivateChannelVolCmd();
void __fastcall LocalSound(int localClientNum);
void __fastcall LocalSoundStop(int localClientNum);
void CG_ReachedCheckpoint();
void __fastcall CG_GameSaveFailed(cg_s *cgameGlob);
void __fastcall CG_BlurServerCommand(int localClientNum);
void __fastcall CG_SlowServerCommand(int localClientNum);
void __fastcall CG_SetClientDvarFromServer(const char *dvarname, const char *value);
void CG_ParseAmp();
void __fastcall CG_ParsePhysGravityDir(int localClientNum);
void __fastcall CG_DispatchServerCommand(int localClientNum);
void __fastcall CG_ServerCommand(int localClientNum);
void __fastcall CG_ExecuteNewServerCommands(int localClientNum, int latestSequence);
void __fastcall CG_MapInit(int restart);
