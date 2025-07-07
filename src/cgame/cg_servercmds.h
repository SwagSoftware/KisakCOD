#pragma once

#ifndef KISAK_SP
#error This file is for SinglePlayer only
#endif

#include "cg_local.h"

void __cdecl CG_ParseServerInfo(int localClientNum);
void __cdecl CG_ParseCullDist(int localClientNum);
void __cdecl CG_ParseSunLight(int localClientNum);
void __cdecl CG_ParseSunDirection(int localClientNum);
void __cdecl CG_ParseFog(int time);
void __cdecl CG_PrecacheScriptMenu(int localClientNum, int iConfigNum);
void __cdecl CG_RegisterServerMaterial(int localClientNum, int num);
void __cdecl CG_RegisterServerMaterials(int localClientNum);
void __cdecl CG_ConfigStringModifiedInternal(int localClientNum, unsigned int stringIndex);
void __cdecl CG_ConfigStringModified(int localClientNum);
void __cdecl CG_ShutdownPhysics(int localClientNum);
void __cdecl CG_OpenScriptMenu(int localClientNum);
void __cdecl CG_CheckOpenWaitingScriptMenu();
void __cdecl CG_CloseScriptMenu(bool allowResponse);
void __cdecl CG_MenuShowNotify(int localClientNum, int menuToShow);
void __cdecl CG_HudMenuShowAllTimed(int localClientNum);
void CG_EqCommand();
void CG_DeactivateEqCmd();
void CG_ReverbCmd();
void CG_DeactivateReverbCmd();
void __cdecl CG_SetChannelVolCmd(int localClientNum);
void CG_DeactivateChannelVolCmd();
void __cdecl LocalSound(int localClientNum);
void __cdecl LocalSoundStop(int localClientNum);
void CG_ReachedCheckpoint();
void __cdecl CG_GameSaveFailed(cg_s *cgameGlob);
void __cdecl CG_BlurServerCommand(int localClientNum);
void __cdecl CG_SlowServerCommand(int localClientNum);
void __cdecl CG_SetClientDvarFromServer(const char *dvarname, const char *value);
void CG_ParseAmp();
void __cdecl CG_ParsePhysGravityDir(int localClientNum);
void __cdecl CG_DispatchServerCommand(int localClientNum);
void __cdecl CG_ServerCommand(int localClientNum);
void __cdecl CG_ExecuteNewServerCommands(int localClientNum, int latestSequence);
void __cdecl CG_MapInit(int restart);
