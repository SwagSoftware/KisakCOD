#include "r_workercmds_common.h"
#include <qcommon/threads.h>
#include "r_workercmds.h"
#include <qcommon/mem_track.h>
#include <EffectsCore/fx_system.h>
#include <universal/profile.h>


void __cdecl R_ProcessCmd_UpdateFxSpotLight(FxCmd *cmd)
{
    FX_UpdateSpotLight(cmd);
    Sys_SetUpdateSpotLightEffectEvent();
}

void __cdecl R_ProcessCmd_UpdateFxNonDependent(FxCmd *cmd)
{
    FX_UpdateNonDependent(cmd);
    Sys_SetUpdateNonDependentEffectsEvent();
}

void __cdecl R_ProcessCmd_UpdateFxRemaining(FxCmd *cmd)
{
    FxGenerateVertsCmd genVertsCmd; // [esp+30h] [ebp-48h] BYREF

    FX_UpdateRemaining(cmd);
    if (sys_smp_allowed->current.enabled)
    {
        PROF_SCOPED("WaitDepFX");
        Sys_WaitUpdateNonDependentEffectsCompleted();
    }
    FX_EndUpdate(cmd->localClientNum);
    R_AddWorkerCmd(WRKCMD_GENERATE_MARK_VERTS, (unsigned __int8 *)cmd);
    KISAK_NULLSUB();
    FX_AddNonSpriteDrawSurfs(cmd);
    FX_FillGenerateVertsCmd(cmd->localClientNum, &genVertsCmd);
    R_AddWorkerCmd(WRKCMD_GENERATE_FX_VERTS, (unsigned __int8 *)&genVertsCmd);
}

void __cdecl R_UpdateSpotLightEffect(FxCmd *cmd)
{
    Sys_ResetUpdateSpotLightEffectEvent();
    R_AddWorkerCmd(WRKCMD_UPDATE_FX_SPOT_LIGHT, (unsigned __int8 *)cmd);
}

void __cdecl R_UpdateNonDependentEffects(FxCmd *cmd)
{
    Sys_ResetUpdateNonDependentEffectsEvent();
    R_AddWorkerCmd(WRKCMD_UPDATE_FX_NON_DEPENDENT, (unsigned __int8 *)cmd);
}

void __cdecl R_UpdateRemainingEffects(FxCmd *cmd)
{
    R_AddWorkerCmd(WRKCMD_UPDATE_FX_REMAINING, (unsigned __int8 *)cmd);
}

void __cdecl R_UpdateXModelBoundsDelayed(GfxSceneEntity *sceneEnt)
{
    R_AddWorkerCmd(WRKCMD_BOUNDS_ENT_DELAYED, (unsigned __int8 *)&sceneEnt);
}

void __cdecl R_SkinGfxEntityDelayed(GfxSceneEntity *sceneEnt)
{
    R_AddWorkerCmd(WRKCMD_SKIN_ENT_DELAYED, (unsigned __int8 *)&sceneEnt);
}

