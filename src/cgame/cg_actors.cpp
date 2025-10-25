#ifndef KISAK_SP
#error This file is for SinglePlayer only
#endif

#include "cg_actors.h"
#include "cg_main.h"
#include <bgame/bg_public.h>
#include <bgame/bg_actor_prone.h>
#include <game/g_actor_prone.h>
#include <ragdoll/ragdoll.h>
#include "cg_ents.h"
#include <gfx_d3d/r_scene.h>
#include "cg_local.h"
#include "cg_compassfriendlies.h"

void __cdecl CG_Actor_PreControllers(centity_s *cent)
{
    int EntityIndex; // r3
    actor_prone_info_s *ClientActorProneInfo; // r3
    actor_prone_info_s *v4; // r31
    int time; // r29
    double ActorProneFraction; // fp1
    double v7; // fp13

    EntityIndex = CG_GetEntityIndex(cent);
    ClientActorProneInfo = G_BypassForCG_GetClientActorProneInfo(EntityIndex);
    v4 = ClientActorProneInfo;
    time = cgArray[0].time;
    if (ClientActorProneInfo
        && (BG_ActorIsProne(ClientActorProneInfo, cgArray[0].time) || v4->bCorpseOrientation || v4->orientPitch))
    {
        ActorProneFraction = BG_GetActorProneFraction(v4, time);
        if (v4->bCorpseOrientation)
        {
            cent->pose.actor.proneType = 2;
            cent->pose.actor.pitch = v4->fTorsoPitch * (float)ActorProneFraction;
            cent->pose.actor.roll = v4->fWaistPitch * (float)ActorProneFraction;
            cent->pose.actor.height = v4->fBodyHeight * (float)ActorProneFraction;
        }
        else
        {
            cent->pose.actor.proneType = 1;
            v7 = (float)(v4->fTorsoPitch * (float)ActorProneFraction);
            cent->pose.actor.height = 0.0;
            cent->pose.actor.pitch = v7;
        }
    }
    else
    {
        cent->pose.actor.proneType = 0;
    }
}

void __cdecl CG_Actor(int localClientNum, centity_s *cent)
{
    entityState_s *p_nextState; // r30
    DObj_s *ClientDObj; // r29
    bool isRagdoll; // r11
    int ragdollHandle; // r3
    unsigned int RenderFlagForRefEntity; // r3
    cg_s *LocalClientGlobals; // r3
    float v10[6]; // [sp+50h] [-50h] BYREF

    p_nextState = &cent->nextState;
    if ((cent->nextState.lerp.eFlags & 0x20) == 0)
    {
        ClientDObj = Com_GetClientDObj(cent->nextState.number, 0);
        if (ClientDObj)
        {
            CG_Actor_PreControllers(cent);
            isRagdoll = cent->pose.isRagdoll;
            v10[0] = cent->pose.origin[0];
            v10[2] = cent->pose.origin[2] + (float)32.0;
            v10[1] = cent->pose.origin[1];
            if (isRagdoll)
            {
                ragdollHandle = cent->pose.ragdollHandle;
                if (ragdollHandle)
                    Ragdoll_GetRootOrigin(ragdollHandle, cent->pose.origin);
            }
            RenderFlagForRefEntity = CG_GetRenderFlagForRefEntity(p_nextState->lerp.eFlags);
            R_AddDObjToScene(ClientDObj, &cent->pose, p_nextState->number, RenderFlagForRefEntity | 4, v10, 0.0);
            if (CG_LookingThroughNightVision(localClientNum) && (p_nextState->lerp.eFlags & 0x4000) != 0
                || cg_laserForceOn->current.enabled)
            {
                LocalClientGlobals = CG_GetLocalClientGlobals(localClientNum);
                CG_Laser_Add(cent, ClientDObj, &cent->pose, LocalClientGlobals->refdef.viewOffset, LASER_OWNER_NON_PLAYER);
            }
            if (p_nextState->eType == 14)
                CG_CompassUpdateActorInfo(localClientNum, p_nextState->number);
        }
    }
}

void __cdecl CG_ActorSpawner(centity_s *cent)
{
    float mins[3]; // [sp+50h] [-30h] BYREF
    float maxs[3]; // [sp+60h] [-20h] BYREF

    mins[0] = -16.0;
    mins[1] = -16.0;
    mins[2] = 0.0;

    maxs[0] = 16.0;
    maxs[1] = 16.0;
    maxs[2] = 72.0;

    CG_DebugBox(cent->pose.origin, mins, maxs, 0.0f, colorRed, 1, 0);
}

