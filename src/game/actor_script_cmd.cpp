#ifndef KISAK_SP 
#error This file is for SinglePlayer only 
#endif

#include "actor.h"
#include "actor_script_cmd.h"
#include "g_main.h"
#include <script/scr_vm.h>
#include <server/sv_game.h>
#include "actor_state.h"
#include "actor_orientation.h"
#include "actor_cover_arrival.h"
#include "actor_aim.h"
#include "game_public.h"
#include "g_local.h"
#include "actor_cover.h"
#include "actor_exposed.h"
#include "actor_threat.h"
#include "g_actor_prone.h"
#include "actor_lookat.h"
#include "actor_senses.h"
#include <script/scr_const.h>
#include "actor_spawner.h"
#include "actor_grenade.h"
#include "actor_turret.h"
#include "turret.h"
#include "g_public.h"

enum DEBUGMAYMOVE_LIFT_ENUM : __int32
{
    DEBUGMAYMOVE_NOT_LIFTED = 0x0,
    DEBUGMAYMOVE_LIFTED = 0x1,
};

actor_s *__cdecl Actor_Get(scr_entref_t entref)
{
    actor_s *result; // r3
    unsigned __int16 v2; // [sp+74h] [+14h]

    v2 = entref.entnum;
    if (entref.classnum)
        goto LABEL_5;
    if (entref.entnum >= 0x880u)
        MyAssertHandler(
            "c:\\trees\\cod3\\cod3src\\src\\game\\actor_script_cmd.cpp",
            68,
            0,
            "%s",
            "entref.entnum < MAX_GENTITIES");
    result = g_entities[v2].actor;
    if (!result)
    {
    LABEL_5:
        Scr_ObjectError("not an actor");
        return 0;
    }
    return result;
}

void __cdecl Actor_SetScriptGoalPos(actor_s *self, const float *vGoalPos, pathnode_t *node)
{
    gentity_s *volume; // r5

    self->scriptGoalEnt.setEnt(NULL);
    Actor_ClearKeepClaimedNode(self);
    self->scriptGoal.pos[0] = *vGoalPos;
    self->scriptGoal.pos[1] = vGoalPos[1];
    self->scriptGoal.pos[2] = vGoalPos[2];
    volume = self->scriptGoal.volume;
    self->scriptGoal.node = node;
    if (volume)
    {
        if (!SV_EntityContact(vGoalPos, vGoalPos, volume))
            self->scriptGoal.volume = 0;
    }
}

void __cdecl ActorCmd_StartScriptedAnim(scr_entref_t entref)
{
    Actor_Get(entref);
    if (Scr_GetNumParam() > 6)
        Scr_Error("too many parameters");
    ScrCmd_animscriptedInternal(entref, 0);
}

void __cdecl Actor_StartArrivalState(actor_s *self, ai_state_t newState)
{
    gentity_s *ent; // r30
    double Float; // fp1
    float v6[12]; // [sp+50h] [-30h] BYREF

    ent = self->ent;
    if (!self->ent)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\actor_script_cmd.cpp", 130, 0, "%s", "ent");
    if (newState > AIS_DEATH)
    {
        Actor_PushState(self, newState);
    }
    else
    {
        if (self->simulatedStateLevel)
            return;
        Actor_SetState(self, newState);
    }
    Scr_GetVector(0, v6);
    self->arrivalInfo.animscriptOverrideOriginError[0] = v6[0] - ent->r.currentOrigin[0];
    self->arrivalInfo.animscriptOverrideOriginError[1] = v6[1] - ent->r.currentOrigin[1];
    self->arrivalInfo.animscriptOverrideOriginError[2] = v6[2] - ent->r.currentOrigin[2];
    Float = Scr_GetFloat(1);
    self->Physics.vVelocity[0] = 0.0;
    self->Physics.vVelocity[1] = 0.0;
    self->Physics.vVelocity[2] = 0.0;
    self->Physics.vWishDelta[0] = 0.0;
    self->Physics.vWishDelta[1] = 0.0;
    self->Physics.vWishDelta[2] = 0.0;
    self->ScriptOrient.eMode = AI_ORIENT_INVALID;
    Actor_SetDesiredAngles(&self->CodeOrient, 0.0, Float);
    Actor_ClearPath(self);
}

void __cdecl ActorCmd_StartCoverArrival(scr_entref_t entref)
{
    actor_s *v1; // r3

    v1 = Actor_Get(entref);
    Actor_StartArrivalState(v1, AIS_COVERARRIVAL);
}

void __cdecl ActorCmd_StartTraverseArrival(scr_entref_t entref)
{
    actor_s *v1; // r3

    v1 = Actor_Get(entref);
    Actor_StartArrivalState(v1, AIS_NEGOTIATION);
}

void __cdecl ActorCmd_CheckCoverExitPosWithPath(scr_entref_t entref)
{
    actor_s *v1; // r31
    bool v2; // r3
    float v3[4]; // [sp+50h] [-20h] BYREF

    v1 = Actor_Get(entref);
    Scr_GetVector(0, v3);
    v2 = Actor_CheckCoverLeave(v1, v3);
    Scr_AddBool(v2);
}

void __cdecl ActorCmd_Shoot(scr_entref_t entref)
{
    actor_s *actor; // r31
    double accuracyMod; // fp31
    float *posOverride = NULL; // r5
    float pos[3]; // [sp+50h] [-30h] BYREF

    actor = Actor_Get(entref);
    accuracyMod = 1.0;
    if (Scr_GetNumParam() == 1)
    {
        accuracyMod = Scr_GetFloat(0);
        if (accuracyMod < 0.0)
            Scr_ParamError(0, "accuracy mod must be nonnegative");
    }
    if (Scr_GetNumParam() >= 2)
    {
        Scr_GetVector(1, pos);
        posOverride = pos;
    }

    Actor_Shoot(actor, accuracyMod, (float(*)[3])posOverride, NOT_LAST_SHOT_IN_CLIP);
}

void __cdecl ActorCmd_ShootBlank(scr_entref_t entref)
{
    actor_s *v1; // r31
    const char *v2; // r3
    unsigned int WeaponIndexForName; // r3
    WeaponDef *WeaponDef; // r3
    const char *v5; // r3

    v1 = Actor_Get(entref);
    v2 = SL_ConvertToString(v1->weaponName);
    WeaponIndexForName = G_GetWeaponIndexForName(v2);
    WeaponDef = BG_GetWeaponDef(WeaponIndexForName);
    if (WeaponDef->weapType)
    {
        v5 = va("ShootBlank() only works with bullet weapons.  Using weapon [%s]", WeaponDef->szInternalName);
        Scr_Error(v5);
    }
    Actor_ShootBlank(v1);
}

void __cdecl ActorCmd_Melee(scr_entref_t entref)
{
    actor_s *v1; // r31
    const float *v2; // r4
    gentity_s *v3; // r3
    float v4[4]; // [sp+50h] [-20h] BYREF

    v1 = Actor_Get(entref);
    if (Scr_GetNumParam())
    {
        Scr_GetVector(0, v4);
        v2 = v4;
    }
    else
    {
        v2 = 0;
    }
    v3 = Actor_Melee(v1, v2);
    if (v3)
        Scr_AddEntity(v3);
}

void __cdecl ActorCmd_UpdatePlayerSightAccuracy(scr_entref_t entref)
{
    actor_s *self; // r31
    sentient_s *enemy; // r3

    self = Actor_Get(entref);
    enemy = Actor_GetTargetSentient(self);
    if (!enemy)
        goto LABEL_6;

    iassert(enemy->ent);

    if (enemy->ent->client)
        self->playerSightAccuracy = Actor_GetPlayerSightAccuracy(self, enemy);
    else
        LABEL_6:
    self->playerSightAccuracy = 1.0;
}

void __cdecl ActorCmd_FindCoverNode(scr_entref_t entref)
{
    actor_s *v1; // r3

    v1 = Actor_Get(entref);
    Actor_Cover_FindCoverNode(v1);
}

void __cdecl ActorCmd_FindBestCoverNode(scr_entref_t entref)
{
    actor_s *v1; // r3
    pathnode_t *BestCover; // r3

    v1 = Actor_Get(entref);
    BestCover = Actor_Cover_FindBestCover(v1);
    if (BestCover)
        Scr_AddPathnode(BestCover);
}

void __cdecl ActorCmd_GetCoverNode(scr_entref_t entref)
{
    actor_s *v1; // r3
    pathnode_t *CoverNode; // r3

    v1 = Actor_Get(entref);
    CoverNode = Actor_Cover_GetCoverNode(v1);
    if (CoverNode)
        Scr_AddPathnode(CoverNode);
}

void __cdecl ActorCmd_UseCoverNode(scr_entref_t entref)
{
    actor_s *v1; // r31
    pathnode_t *Pathnode; // r3
    unsigned __int8 v3; // r3

    v1 = Actor_Get(entref);
    if (v1->fixedNode)
        Scr_Error("cannot change node when using fixedNode mode");
    if ((unsigned __int8)Actor_KeepClaimedNode(v1))
        Scr_Error("cannot change node when keepclaimednode is set");
    Pathnode = Scr_GetPathnode(0);
    v3 = Actor_Cover_UseCoverNode(v1, Pathnode);
    Scr_AddBool(v3);
}

void __cdecl ActorCmd_ReacquireStep(scr_entref_t entref)
{
    actor_s *v1; // r31
    double Float; // fp1
    unsigned __int8 v3; // r3

    v1 = Actor_Get(entref);
    Float = Scr_GetFloat(0);
    v3 = Actor_Exposed_ReacquireStepMove(v1, Float);
    Scr_AddBool(v3);
}

void __cdecl ActorCmd_FindReacquireNode(scr_entref_t entref)
{
    actor_s *v1; // r3

    v1 = Actor_Get(entref);
    Actor_Exposed_FindReacquireNode(v1);
}

void __cdecl ActorCmd_GetReacquireNode(scr_entref_t entref)
{
    actor_s *v1; // r3
    pathnode_t *ReacquireNode; // r3

    v1 = Actor_Get(entref);
    ReacquireNode = Actor_Exposed_GetReacquireNode(v1);
    if (ReacquireNode)
        Scr_AddPathnode(ReacquireNode);
}

void __cdecl ActorCmd_UseReacquireNode(scr_entref_t entref)
{
    actor_s *v1; // r31
    pathnode_t *Pathnode; // r3
    unsigned __int8 v3; // r3

    v1 = Actor_Get(entref);
    if ((unsigned __int8)Actor_KeepClaimedNode(v1))
        Scr_Error("cannot change node when keepclaimednode is set");
    Pathnode = Scr_GetPathnode(0);
    v3 = Actor_Exposed_UseReacquireNode(v1, Pathnode);
    Scr_AddBool(v3);
}

void __cdecl ActorCmd_FindReacquireDirectPath(scr_entref_t entref)
{
    actor_s *v1; // r30
    bool v2; // r31

    v1 = Actor_Get(entref);
    v2 = 0;
    if (Scr_GetNumParam())
        v2 = Scr_GetInt(0) != 0;
    Actor_Exposed_FindReacquireDirectPath(v1, v2);
}

void __cdecl ActorCmd_FindReacquireProximatePath(scr_entref_t entref)
{
    actor_s *v1; // r30
    char v2; // r31

    v1 = Actor_Get(entref);
    v2 = 0;
    if (Scr_GetNumParam())
        v2 = Scr_GetInt(0) != 0;
    Actor_Exposed_FindReacquireProximatePath(v1, v2);
}

void __cdecl ActorCmd_TrimPathToAttack(scr_entref_t entref)
{
    actor_s *v1; // r31
    int v2; // r3

    v1 = Actor_Get(entref);
    if (!(unsigned __int8)Actor_MayReacquireMove(v1))
        Scr_Error("TrimPathToAttack may only called after calling FindReacquireDirectPath or FindReacquireProximatePath");
    v2 = Actor_TrimPathToAttack(v1);
    Scr_AddBool(v2);
}

void __cdecl ActorCmd_ReacquireMove(scr_entref_t entref)
{
    actor_s *v1; // r31
    unsigned __int8 started; // r3

    v1 = Actor_Get(entref);
    if (!(unsigned __int8)Actor_MayReacquireMove(v1))
        Scr_Error("ReacquireMove may only called after calling FindReacquireDirectPath or FindReacquireProximatePath");
    started = Actor_Exposed_StartReacquireMove(v1);
    Scr_AddBool(started);
}

void __cdecl ActorCmd_FlagEnemyUnattackable(scr_entref_t entref)
{
    actor_s *v1; // r3

    v1 = Actor_Get(entref);
    Actor_FlagEnemyUnattackable(v1);
}

void __cdecl ActorCmd_SetAimAnims(scr_entref_t entref)
{
    actor_s *v1; // r31
    XAnimTree_s *ActorAnimTree; // r30

    v1 = Actor_Get(entref);
    ActorAnimTree = G_GetActorAnimTree(v1);
    v1->animSets.aimLow = Scr_GetAnim(0, ActorAnimTree).index;
    v1->animSets.aimLevel = Scr_GetAnim(1, ActorAnimTree).index;
    v1->animSets.aimHigh = Scr_GetAnim(2, ActorAnimTree).index;
    v1->animSets.shootLow = Scr_GetAnim(3, ActorAnimTree).index;
    v1->animSets.shootLevel = Scr_GetAnim(4, ActorAnimTree).index;
    v1->animSets.shootHigh = Scr_GetAnim(5, ActorAnimTree).index;
}

void __cdecl ActorCmd_AimAtPos(scr_entref_t entref)
{
    actor_s *v1; // r31
    double v2; // fp30
    double v3; // fp28
    double v4; // fp31
    double v5; // fp29
    double v6; // fp27
    double v9; // fp13
    double v10; // fp0
    double v11; // fp27
    DObj_s *ServerDObj; // r30
    int v13; // r7
    unsigned int v14; // r6
    unsigned int v15; // r5
    int v16; // r7
    unsigned int v17; // r6
    unsigned int v18; // r5
    int v19; // r7
    unsigned int v20; // r6
    unsigned int v21; // r5
    int v22; // r7
    unsigned int v23; // r6
    unsigned int v24; // r5
    int v25; // r7
    unsigned int v26; // r6
    unsigned int v27; // r5
    int v28; // r7
    unsigned int v29; // r6
    unsigned int v30; // r5
    float v31[4]; // [sp+50h] [-60h] BYREF
    float v32; // [sp+60h] [-50h] BYREF
    float v33; // [sp+64h] [-4Ch]
    float v34; // [sp+68h] [-48h]

    v1 = Actor_Get(entref);
    v2 = 0.0;
    v3 = 0.0;
    Scr_GetVector(0, v31);
    v4 = v31[0];
    v5 = v31[1];
    v6 = v31[2];
    Sentient_GetEyePosition(v1->sentient, &v32);

    //_FP12 = -sqrtf((float)((float)((float)((float)v6 - v34) * (float)((float)v6 - v34))
    //    + (float)((float)((float)((float)v5 - v33) * (float)((float)v5 - v33))
    //        + (float)((float)((float)v4 - v32) * (float)((float)v4 - v32)))));
    //__asm { fsel      f13, f12, f31, f13 }
    //v9 = (float)((float)1.0 / (float)_FP13);

    // aislop
    {
        float len = sqrtf((v6 - v34) * (v6 - v34)
            + (v5 - v33) * (v5 - v33)
            + (v4 - v32) * (v4 - v32));
        float neg = -len;
        float denom = (neg >= 0.0f) ? neg : neg;
        v9 = 1.0f / denom;
    }

    v10 = -(float)((float)v9 * (float)((float)v6 - v34));
    if (v10 >= 0.0)
    {
        v2 = -(float)((float)v9 * (float)((float)v6 - v34));
        v11 = (float)((float)1.0 - (float)v10);
    }
    else
    {
        v3 = -v10;
        v11 = (float)((float)1.0 - (float)-v10);
    }
    ServerDObj = Com_GetServerDObj(v1->ent->s.number);
    XAnimSetCompleteGoalWeight(ServerDObj, v1->animSets.aimLow, v2, 0.1, 1.0, v15, v14, v13);
    XAnimSetCompleteGoalWeight(ServerDObj, v1->animSets.aimLevel, v11, 0.1, 1.0, v18, v17, v16);
    XAnimSetCompleteGoalWeight(ServerDObj, v1->animSets.aimHigh, v3, 0.1, 1.0, v21, v20, v19);
    XAnimSetGoalWeight(ServerDObj, v1->animSets.shootLow, v2, 0.1, 1.0, v24, v23, v22);
    XAnimSetGoalWeight(ServerDObj, v1->animSets.shootLevel, v11, 0.1, 1.0, v27, v26, v25);
    XAnimSetGoalWeight(ServerDObj, v1->animSets.shootHigh, v3, 0.1, 1.0, v30, v29, v28);
    Scr_AddFloat(0.1);
}

void __cdecl ActorCmd_EnterProne(scr_entref_t entref)
{
    actor_s *v1; // r3
    actor_s *v2; // r31
    int v3; // [sp+50h] [-20h]

    v1 = Actor_Get(entref);
    v2 = v1;
    if (v1->fInvProneAnimLowPitch == 0.0 && v1->fInvProneAnimHighPitch == 0.0
        || !v1->animSets.animProneLow
        || !v1->animSets.animProneLevel
        || !v1->animSets.animProneHigh)
    {
        Scr_Error("Must call SetProneAnimNodes before calling EnterProne");
    }
    v3 = (int)(float)(Scr_GetFloat(0) * (float)1000.0);
    G_ActorEnterProne(v2, v3);
}

void __cdecl ActorCmd_ExitProne(scr_entref_t entref)
{
    actor_s *v1; // r31
    int v2; // [sp+50h] [-20h]

    v1 = Actor_Get(entref);
    v2 = (int)(float)(Scr_GetFloat(0) * (float)1000.0);
    G_ActorExitProne(v1, v2);
}

void __cdecl ActorCmd_SetProneAnimNodes(scr_entref_t entref)
{
    float downAng; // fp29
    float upAng; // fp31
    actor_s *v4; // r31
    XAnimTree_s *ActorAnimTree; // r30
    XAnimTree_s *v6; // r5
    XAnimTree_s *v7; // r5
    XAnimTree_s *v8; // r5

    downAng = Scr_GetFloat(0);
    upAng = Scr_GetFloat(1);

    v4 = Actor_Get(entref);
    ActorAnimTree = G_GetActorAnimTree(v4);

    if (downAng >= 0.0)
        Scr_Error("Down angle (parameter 1) must be set to be less than 0.");

    if (upAng <= 0.0)
        Scr_Error("Up angle (parameter 2) must be set to be greater than 0.");

    v4->fInvProneAnimLowPitch = 1.0f / (float)downAng;
    v4->fInvProneAnimHighPitch = 1.0f / (float)upAng;

    v4->animSets.animProneLow = Scr_GetAnim(2, ActorAnimTree).index;
    v4->animSets.animProneLevel = Scr_GetAnim(3, ActorAnimTree).index;
    v4->animSets.animProneHigh = Scr_GetAnim(4, ActorAnimTree).index;
}

void __cdecl ActorCmd_UpdateProne(scr_entref_t entref)
{
    actor_s *v1; // r31
    XAnimTree_s *ActorAnimTree; // r30
    XAnimTree_s *v3; // r5
    XAnimTree_s *v4; // r5
    double Float; // fp31
    double v6; // fp30
    double v7; // fp29
    DObj_s *ServerDObj; // r30
    int v9; // r7
    unsigned int v10; // r6
    unsigned int v11; // r5
    int v12; // r7
    unsigned int v13; // r6
    unsigned int v14; // r5
    float *v15; // r6
    float *v16; // r5
    long double v17; // fp2
    scr_anim_s Anim; // [sp+50h] [-40h]
    scr_anim_s v19; // [sp+54h] [-3Ch]

    v1 = Actor_Get(entref);
    if (BG_ActorIsProne(&v1->ProneInfo, level.time))
    {
        ActorAnimTree = G_GetActorAnimTree(v1);
        Anim = Scr_GetAnim(0, ActorAnimTree);
        v19 = Scr_GetAnim(1, ActorAnimTree);
        Float = Scr_GetFloat(2);
        v6 = Scr_GetFloat(3);
        v7 = Scr_GetFloat(4);
        ServerDObj = Com_GetServerDObj(v1->ent->s.number);
        XAnimSetCompleteGoalWeight(ServerDObj, Anim.index, Float, v6, v7, v11, v10, v9);
        XAnimSetCompleteGoalWeight(ServerDObj, v19.index, Float, v6, v7, v14, v13, v12);
        Actor_UpdateProneInformation(v1, 0, v16, v15, v17);
    }
}

void __cdecl ActorCmd_ClearPitchOrient(scr_entref_t entref)
{
    actor_s *v1; // r3

    v1 = Actor_Get(entref);
    v1->ProneInfo.prone = 0;
    v1->ProneInfo.orientPitch = 0;
}

void __cdecl ActorCmd_SetLookAtAnimNodes(scr_entref_t entref)
{
    actor_s *v1; // r30
    XAnimTree_s *ActorAnimTree; // r31
    XAnimTree_s *v3; // r5
    XAnimTree_s *v4; // r5
    XAnimTree_s *v5; // r5
    scr_anim_s v6; // [sp+50h] [-30h]
    scr_anim_s v7; // [sp+54h] [-2Ch]
    scr_anim_s Anim; // [sp+58h] [-28h]

    v1 = Actor_Get(entref);
    ActorAnimTree = G_GetActorAnimTree(v1);
    Anim = Scr_GetAnim(0, ActorAnimTree);
    v7 = Scr_GetAnim(1, ActorAnimTree);
    v6 = Scr_GetAnim(2, ActorAnimTree);
    Actor_SetLookAtAnimNodes(v1, Anim.index, v7.index, v6.index);
}

void __cdecl ActorCmd_SetLookAt(scr_entref_t entref)
{
    double Float; // fp31
    actor_s *v2; // r31
    float v3[6]; // [sp+50h] [-30h] BYREF

    Float = 0.0;
    v2 = Actor_Get(entref);
    if (!v2->lookAtInfo.bLookAtSetup)
        Scr_Error("LookAt Called without setLookAtAnimNodes being called first.");
    Scr_GetVector(0, v3);
    if (Scr_GetNumParam() > 1)
        Float = Scr_GetFloat(1);
    Actor_SetLookAt(v2, v3, Float);
}

void __cdecl ActorCmd_SetLookAtYawLimits(scr_entref_t entref)
{
    actor_s *v1; // r31
    double Float; // fp31
    double v3; // fp30
    double v4; // fp1

    v1 = Actor_Get(entref);
    Float = Scr_GetFloat(2);
    v3 = Scr_GetFloat(1);
    v4 = Scr_GetFloat(0);
    Actor_SetLookAtYawLimits(v1, v4, v3, Float);
}

void __cdecl ActorCmd_StopLookAt(scr_entref_t entref)
{
    double Float; // fp31
    actor_s *v2; // r31

    Float = 0.0;
    v2 = Actor_Get(entref);
    if (Scr_GetNumParam())
        Float = Scr_GetFloat(0);
    Actor_StopLookAt(v2, Float);
}

void __cdecl ActorCmd_CanShoot(scr_entref_t entref)
{
    actor_s *self; // r31
    bool CanShootFrom; // r31
    const float *color; // r5
    float muzzlePos[3]; // [sp+50h] [-40h] BYREF
    float offset[4]; // [sp+60h] [-30h] BYREF
    float targetPos[4]; // [sp+70h] [-20h] BYREF

    self = Actor_Get(entref);
    Scr_GetVector(0, targetPos);
    if (!Actor_GetMuzzleInfo(self, muzzlePos, 0))
    {
        Scr_Error(va("Couldn't find %s in entity %d", "tag_flash", self->ent->s.number));
    }
    if (Scr_GetNumParam() > 1)
    {
        Scr_GetVector(1u, offset);
        muzzlePos[0] += offset[0];
        muzzlePos[1] += offset[1];
        muzzlePos[2] += offset[2];
    }
    CanShootFrom = Actor_CanShootFrom(self, targetPos, muzzlePos);
    if (ai_ShowCanshootChecks->current.enabled)
    {
        if (CanShootFrom)
            color = colorGreen;
        else
            color = colorRed;
        G_DebugLineWithDuration(targetPos, muzzlePos, color, 0, 30);
    }
    Scr_AddInt(CanShootFrom);
}

void __cdecl ActorCmd_CanSee(scr_entref_t entref)
{
    actor_s *v1; // r30
    gentity_s *pOther; // r31
    bool CanSeeSentient; // r3
    int v4; // [sp+50h] [-20h]

    v1 = Actor_Get(entref);
    pOther = Scr_GetEntity(0);

    iassert(pOther);

    if (pOther->sentient)
    {
        if (Scr_GetNumParam() <= 1)
        {
            CanSeeSentient = Actor_CanSeeSentient(v1, pOther->sentient, 250);
        }
        else
        {
            v4 = (int)(float)(Scr_GetFloat(0) * (float)1000.0);
            CanSeeSentient = Actor_CanSeeSentient(v1, pOther->sentient, v4);
        }
    }
    else
    {
        CanSeeSentient = Actor_CanSeeEntity(v1, pOther);
    }
    Scr_AddBool(CanSeeSentient);
}

void __cdecl ActorCmd_DropWeapon(scr_entref_t entref)
{
    actor_s *v1; // r29
    const char *String; // r31
    unsigned int ConstString; // r30
    double Float; // fp31
    int WeaponIndexForName; // r28
    const char *v6; // r3
    unsigned int tag_weapon_left; // r6
    gentity_s *v8; // r3
    double v9; // fp0
    double v10; // fp0
    double v11; // fp13
    double v12; // fp12
    double v15; // fp11
    double v16; // fp13
    double v17; // fp12
    float v18; // [sp+50h] [-40h]

    v1 = Actor_Get(entref);
    String = Scr_GetString(0);
    if (I_stricmp(String, "none"))
    {
        ConstString = Scr_GetConstString(1);
        Float = Scr_GetFloat(2);
        WeaponIndexForName = G_GetWeaponIndexForName(String);
        if (!WeaponIndexForName)
        {
            v6 = va("unknown weapon '%s' in dropWeapon", String);
            Scr_Error(v6);
        }
        if (ConstString == scr_const.left)
        {
            tag_weapon_left = scr_const.tag_weapon_left;
        }
        else if (ConstString == scr_const.right)
        {
            tag_weapon_left = scr_const.tag_weapon_right;
        }
        else
        {
            tag_weapon_left = 0;
        }
        v8 = Drop_Weapon(v1->ent, WeaponIndexForName, 0, tag_weapon_left);
        v9 = 0.0;
        if (Float == 0.0)
        {
            v8->s.lerp.pos.trDelta[0] = 0.0;
            v8->s.lerp.pos.trDelta[1] = 0.0;
        }
        else
        {
            v10 = (float)(v8->r.currentOrigin[0] - v1->ent->r.currentOrigin[0]);
            v11 = (float)(v8->r.currentOrigin[1] - v1->ent->r.currentOrigin[1]);
            v12 = (float)((float)(v8->r.currentOrigin[2]
                - (float)((float)((float)(v1->ent->r.maxs[2] - v1->ent->r.mins[2]) * (float)0.5)
                    + v1->ent->r.currentOrigin[2]))
                + (float)2.0);

            // aislop
            //_FP9 = -sqrtf((float)((float)((float)v12 * (float)v12)
            //    + (float)((float)((float)v11 * (float)v11) + (float)((float)v10 * (float)v10))));
            //__asm { fsel      f11, f9, f10, f11 }
            //v15 = (float)((float)1.0 / (float)_FP11);

            {
                float len = sqrtf(v12 * v12 + v11 * v11 + v10 * v10);
                float neg = -len;
                // Emulate fsel: if neg >= 0 use neg, else fallback to len (typical PowerPC pattern)
                float denom = (neg >= 0.0f ? neg : len);
                v15 = 1.0f / denom;
            }

            v16 = (float)((float)(v8->r.currentOrigin[1] - v1->ent->r.currentOrigin[1]) * (float)v15);
            v17 = (float)((float)v15
                * (float)((float)(v8->r.currentOrigin[2]
                    - (float)((float)((float)(v1->ent->r.maxs[2] - v1->ent->r.mins[2]) * (float)0.5)
                        + v1->ent->r.currentOrigin[2]))
                    + (float)2.0));
            v8->s.lerp.pos.trDelta[0] = (float)((float)v15 * (float)(v8->r.currentOrigin[0] - v1->ent->r.currentOrigin[0]))
                * (float)Float;
            v8->s.lerp.pos.trDelta[1] = (float)v16 * (float)Float;
            v9 = (float)((float)v17 * (float)Float);
        }
        v8->s.lerp.pos.trDelta[2] = v9;
        v18 = v8->s.lerp.pos.trDelta[0];
        v8->s.lerp.pos.trTime = level.time;
        if ((LODWORD(v18) & 0x7F800000) == 0x7F800000
            || (COERCE_UNSIGNED_INT(v8->s.lerp.pos.trDelta[1]) & 0x7F800000) == 0x7F800000
            || (COERCE_UNSIGNED_INT(v8->s.lerp.pos.trDelta[2]) & 0x7F800000) == 0x7F800000)
        {
            MyAssertHandler(
                "c:\\trees\\cod3\\cod3src\\src\\game\\actor_script_cmd.cpp",
                1138,
                0,
                "%s",
                "!IS_NAN((weapDef->s.lerp.pos.trDelta)[0]) && !IS_NAN((weapDef->s.lerp.pos.trDelta)[1]) && !IS_NAN((weapDef->s.le"
                "rp.pos.trDelta)[2])");
        }
    }
}

void __cdecl DEBUGMAYMOVE(float *vec1, float *vec2, const float *color, DEBUGMAYMOVE_LIFT_ENUM liftBehavior)
{
    double v4; // fp13
    double v5; // fp0
    double v6; // fp12
    float v7[2]; // [sp+50h] [-30h] BYREF
    float v8; // [sp+58h] [-28h]
    float v9[2]; // [sp+60h] [-20h] BYREF
    float v10; // [sp+68h] [-18h]

    if (ai_debugMayMove->current.enabled)
    {
        v9[0] = *vec1;
        v9[1] = vec1[1];
        v7[0] = *vec2;
        v4 = vec1[2];
        v5 = vec2[1];
        v6 = vec2[2];
        v10 = vec1[2];
        v7[1] = v5;
        v8 = v6;
        if (liftBehavior == DEBUGMAYMOVE_LIFTED)
        {
            v10 = (float)v4 + (float)32.0;
            v8 = (float)v6 + (float)32.0;
        }
        G_DebugLineWithDuration(v9, v7, color, 0, 100);
    }
}

int __cdecl MayMove_CheckFriendlyFire(actor_s *self, float *start, const float *end)
{
    double v6; // fp1
    double v7; // fp0
    int MoveOnlySuppressionPlanes; // r3
    int v9; // r10
    double v10; // fp9
    float *v11; // r9
    float *i; // r11
    double v14; // fp13
    float v15; // [sp+50h] [-80h] BYREF
    float v16; // [sp+54h] [-7Ch]
    float v17; // [sp+58h] [-78h]
    float v18[4]; // [sp+60h] [-70h] BYREF
    float v19[4]; // [sp+70h] [-60h] BYREF
    float v20[4]; // [sp+80h] [-50h] BYREF
    float v21[8][2]; // [sp+90h] [-40h] BYREF

    if (!Actor_IsMoveSuppressed(self))
        return 0;
    v15 = *end - *start;
    v16 = end[1] - start[1];
    v6 = Vec2Normalize(&v15);
    v17 = end[2];
    v7 = (float)((float)(v16 * (float)((float)v6 + (float)30.0)) + start[1]);
    v15 = (float)(v15 * (float)((float)v6 + (float)30.0)) + *start;
    v16 = v7;
    MoveOnlySuppressionPlanes = Actor_GetMoveOnlySuppressionPlanes(self, v21, v20);
    v9 = 0;
    if (MoveOnlySuppressionPlanes <= 0)
        return 0;
    v10 = start[1];
    v11 = v20;
    for (i = v21[0];
        (float)((float)(*v11 - (float)((float)(i[1] * start[1]) + (float)(*i * *start)))
            * (float)(*v11 - (float)((float)(*i * v15) + (float)(i[1] * v16)))) >= 0.0;
        i += 2)
    {
        ++v9;
        ++v11;
        if (v9 >= MoveOnlySuppressionPlanes)
            return 0;
    }
    if (ai_debugMayMove->current.enabled)
    {
        v14 = start[2];
        v18[0] = *start;
        v18[1] = v10;
        v19[0] = v15;
        v19[1] = v16;
        v18[2] = (float)v14 + (float)32.0;
        v19[2] = v17 + (float)32.0;
        G_DebugLineWithDuration(v18, v19, colorYellow, 0, 100);
    }
    return 1;
}

int __cdecl MayMove_TraceCheck(actor_s *self, float *vStart, float *vEnd, int allowStartSolid, int checkDrop)
{
    trace_t results; // [sp+A0h] [-70h] BYREF
    float vPointLow[3];
    float vPointHigh[3];
    float stepheight;
    float vTraceEndPos[3];

    static const float MAX_FALL_HEIGHT = 32.0f;

    iassert(self);
    iassert(vStart);
    iassert(vEnd);

    vPointHigh[0] = vEnd[0];
    vPointHigh[1] = vEnd[1];
    vPointHigh[2] = vEnd[2];

    vPointLow[0] = vEnd[0];
    vPointLow[1] = vEnd[1];
    vPointLow[2] = vEnd[2];

    vPointHigh[2] += 48.0;
    vPointLow[2] -= 48.0;

    G_TraceCapsule(&results, vPointHigh, vec3_origin, vec3_origin, vPointLow, self->ent->s.number, self->Physics.iTraceMask | 0x6000);

    if (results.allsolid)
    {
        DEBUGMAYMOVE(vPointHigh, vPointLow, colorOrange, DEBUGMAYMOVE_LIFTED);
        return 0;
    }
    else
    {
        if (self->Physics.prone)
            stepheight = 10.0f;
        else
            stepheight = 18.0f;

        if (checkDrop
            && (Vec3Lerp(vPointHigh, vPointLow, results.fraction, vPointLow),
                (float)(vStart[2] - vPointLow[2]) > MAX_FALL_HEIGHT))
        {
            DEBUGMAYMOVE(vEnd, vPointLow, colorOrange, DEBUGMAYMOVE_NOT_LIFTED);
            return 0;
        }
        else if (Path_PredictionTrace(
            vStart,
            vEnd,
            self->ent->s.number,
            self->Physics.iTraceMask | 0x8004,
            vTraceEndPos,
            stepheight,
            allowStartSolid))
        {
            return 1;
        }
        else
        {
            DEBUGMAYMOVE(vStart, vEnd, colorLtOrange, DEBUGMAYMOVE_LIFTED);
            return 0;
        }
    }
}

void __cdecl ActorCmd_MayMoveToPoint(scr_entref_t entref)
{
    actor_s *v1; // r31
    int v2; // r30
    float *p_eType; // r11
    double v4; // fp13
    int v5; // r3
    float *currentOrigin; // r3
    float v7[4]; // [sp+50h] [-60h] BYREF
    float v8[4]; // [sp+60h] [-50h] BYREF
    float v9[4]; // [sp+70h] [-40h] BYREF
    float v10[6]; // [sp+80h] [-30h] BYREF

    v1 = Actor_Get(entref);
    Scr_GetVector(0, v7);
    v2 = Scr_GetNumParam() <= 1 || Scr_GetInt(1) != 0;
    p_eType = (float *)&v1->ent->s.eType;
    if (v1->ent->tagInfo)
    {
        if (ai_debugMayMove->current.enabled)
        {
            v8[0] = p_eType[56];
            v8[1] = p_eType[57];
            v4 = p_eType[58];
            v9[0] = v7[0];
            v9[1] = v7[1];
            v8[2] = (float)v4 + (float)32.0;
            v9[2] = v7[2] + (float)32.0;
            G_DebugLineWithDuration(v8, v9, colorMagenta, 0, 100);
        }
    }
    else if (!MayMove_CheckFriendlyFire(v1, p_eType + 56, v7))
    {
        Sentient_GetOrigin(v1->sentient, v10);
        if (MayMove_TraceCheck(v1, v10, v7, 1, v2))
        {
            currentOrigin = v1->ent->r.currentOrigin;
            v1->mayMoveTime = level.time;
            DEBUGMAYMOVE(currentOrigin, v7, colorGreen, DEBUGMAYMOVE_LIFTED);
            v5 = 1;
            goto LABEL_11;
        }
    }
    v5 = 0;
LABEL_11:
    Scr_AddInt(v5);
}

void __cdecl ActorCmd_MayMoveFromPointToPoint(scr_entref_t entref)
{
    actor_s *v1; // r31
    int v2; // r7
    int v3; // r3
    float v4[4]; // [sp+50h] [-50h] BYREF
    float v5[4]; // [sp+60h] [-40h] BYREF
    float v6[4]; // [sp+70h] [-30h] BYREF
    float v7[4]; // [sp+80h] [-20h] BYREF

    v1 = Actor_Get(entref);
    Scr_GetVector(0, v5);
    Scr_GetVector(1u, v4);
    v2 = Scr_GetNumParam() <= 2 || Scr_GetInt(2) != 0;
    if (v1->ent->tagInfo)
    {
        if (ai_debugMayMove->current.enabled)
        {
            v6[0] = v5[0];
            v6[1] = v5[1];
            v7[0] = v4[0];
            v7[1] = v4[1];
            v6[2] = v5[2] + (float)32.0;
            v7[2] = v4[2] + (float)32.0;
            G_DebugLineWithDuration(v6, v7, colorMagenta, 0, 100);
        }
    }
    else if (MayMove_TraceCheck(v1, v5, v4, 0, v2))
    {
        v1->mayMoveTime = level.time;
        DEBUGMAYMOVE(v5, v4, colorGreen, DEBUGMAYMOVE_LIFTED);
        v3 = 1;
        goto LABEL_8;
    }
    v3 = 0;
LABEL_8:
    Scr_AddInt(v3);
}

void __cdecl ActorCmd_Teleport(scr_entref_t entref)
{
    actor_s *self; // r31
    float *angles; // r25
    gentity_s *ent; // r27
    double distSquared; // fp31
    gentity_s *player; // r30
    float vEyePos[3]; // [sp+60h] [-80h] BYREF
    float vSpawnPos[3];
    float vAngles[3];

    self = Actor_Get(entref);
    angles = 0;
    ent = self->ent;
    Scr_GetVector(0, vSpawnPos);
    if (Scr_GetNumParam() > 1)
    {
        Scr_GetVector(1, vAngles);
    }
    distSquared = Vec3DistanceSq(vSpawnPos, ent->r.currentOrigin);

    if (self->ent->tagInfo)
    {
    LABEL_15:
        ent->s.lerp.eFlags ^= 2u;
        goto LABEL_16;
    }
    else if (level.loading)
    {
        goto LABEL_16;
    }
    else if (distSquared > 100.0)
    {
        player = G_Find(0, 284, scr_const.player);
        iassert(player);
        iassert(player->sentient);
        Sentient_GetEyePosition(player->sentient, vEyePos);
        if (PointCouldSeeSpawn(vEyePos, vSpawnPos, player->s.number, entref.entnum))
        {
            Com_DPrintf(18, "Teleport (of actor %i) failed because player could see goal pos.\n", ent->s.number);
            Scr_AddInt(0);
            return;
        }
        if (PointCouldSeeSpawn(vEyePos, ent->r.currentOrigin, player->s.number, entref.entnum))
        {
            Com_DPrintf(18, "Teleport failed because player could see actor (%i).\n", ent->s.number);
            Scr_AddInt(0);
            return;
        }
        if (distSquared <= 16384.0)
            goto LABEL_16;
        goto LABEL_15;
    }
    if (Actor_HasPath(self) && self->Path.iPathEndTime)
    {
        Com_DPrintf(18, "Teleport failed because actor (%i) in mid-stopping.\n", ent->s.number);
        Scr_AddInt(0);
        return;
    }
LABEL_16:
    G_SetOrigin(ent, vSpawnPos);
    self->Physics.vVelocity[0] = 0.0;
    self->Physics.vVelocity[1] = 0.0;
    self->Physics.vVelocity[2] = 0.0;
    self->Physics.vWishDelta[0] = 0.0;
    self->Physics.vWishDelta[1] = 0.0;
    self->Physics.vWishDelta[2] = 0.0;
    if (self->useEnemyGoal && (!Actor_PointAtGoal(vSpawnPos, &self->codeGoal) || distSquared > 100.0))
    {
        self->useEnemyGoal = 0;
        Actor_UpdateGoalPos(self);
    }
    if (angles)
    {
        G_SetAngle(ent, angles);
        Actor_SetDesiredAngles(&self->CodeOrient, ent->r.currentAngles[0], ent->r.currentAngles[1]);
        if (self->ScriptOrient.eMode == AI_ORIENT_DONT_CHANGE)
            Actor_SetDesiredAngles(&self->ScriptOrient, ent->r.currentAngles[0], ent->r.currentAngles[1]);
        Actor_SetLookAngles(self, ent->r.currentAngles[0], ent->r.currentAngles[1]);
    }
    if (!self->ent->tagInfo && (level.loading || distSquared > 100.0))
    {
        Sentient_InvalidateNearestNode(self->sentient);
        Actor_ClearPath(self);
        Sentient_NearestNode(self->sentient);
    }
    Scr_AddInt(1);
}

void __cdecl ActorCmd_WithinApproxPathDist(scr_entref_t entref)
{
    actor_s *v1; // r31
    double Float; // fp1
    __int64 v3; // r11

    v1 = Actor_Get(entref);
    Float = Scr_GetFloat(0);
    LODWORD(v3) = Path_WithinApproxDist(&v1->Path, Float);
    Scr_AddFloat((float)v3);
}

void __cdecl ActorCmd_IsPathDirect(scr_entref_t entref)
{
    actor_s *v1; // r31
    int v2; // r3

    v1 = Actor_Get(entref);
    if (Actor_HasPath(v1))
        v2 = Path_CompleteLookahead(&v1->Path);
    else
        v2 = 0;
    Scr_AddInt(v2);
}

void __cdecl ActorCmd_AllowedStances(scr_entref_t entref)
{
    actor_s *v1; // r29
    int NumParam; // r27
    signed int v3; // r30
    unsigned int ConstString; // r3
    const char *v5; // r3
    const char *v6; // r3

    v1 = Actor_Get(entref);
    NumParam = Scr_GetNumParam();
    if (NumParam < 1)
        Scr_Error("no stances given in allowedStances()\n");
    v3 = 0;
    for (v1->eAllowedStances = STANCE_BAD; v3 < NumParam; ++v3)
    {
        ConstString = Scr_GetConstString(v3);
        if (ConstString == scr_const.stand)
        {
            v1->eAllowedStances |= STANCE_STAND; // KISAKTODO: check this |= operator works
        }
        else if (ConstString == scr_const.crouch)
        {
            v1->eAllowedStances |= STANCE_CROUCH;
        }
        else if (ConstString == scr_const.prone)
        {
            v1->eAllowedStances |= STANCE_PRONE;
        }
        else
        {
            v5 = SL_ConvertToString(ConstString);
            v6 = va("invalid stance '%s' in allowedStances()\n", v5);
            Scr_Error(v6);
        }
    }
    if (v1->eAllowedStances == STANCE_BAD)
    {
        v1->eAllowedStances = STANCE_ANY;
        Scr_Error("no allowed stances given");
    }
}

void __cdecl ActorCmd_IsStanceAllowed(scr_entref_t entref)
{
    actor_s *v1; // r31
    unsigned int ConstString; // r3
    int v3; // r29
    const char *v4; // r3
    const char *v5; // r3
    int v6; // r30
    ai_stance_e v7; // r3
    const pathnode_t *pClaimedNode; // r4
    ai_stance_e eAllowedStances; // r11

    v1 = Actor_Get(entref);
    ConstString = Scr_GetConstString(0);
    v3 = 0;
    if (ConstString == scr_const.stand)
    {
        v3 = 1;
    }
    else if (ConstString == scr_const.crouch)
    {
        v3 = 2;
    }
    else if (ConstString == scr_const.prone)
    {
        v3 = 4;
    }
    else
    {
        v4 = SL_ConvertToString(ConstString);
        v5 = va("invalid stance '%s' in isStanceAllowed()\n", v4);
        Scr_Error(v5);
    }
    v6 = 7;
    if (Actor_HasPath(v1))
    {
        v7 = Path_AllowedStancesForPath(&v1->Path);
    LABEL_13:
        v6 = v7;
        goto LABEL_14;
    }
    pClaimedNode = v1->sentient->pClaimedNode;
    if (pClaimedNode && Actor_PointNearNode(v1->ent->r.currentOrigin, pClaimedNode))
    {
        v7 = (ai_stance_e)Path_AllowedStancesForNode(v1->sentient->pClaimedNode);
        goto LABEL_13;
    }
LABEL_14:
    eAllowedStances = v1->eAllowedStances;
    if ((eAllowedStances & v6) != 0)
    {
        //eAllowedStances &= v6;
        int tmp = (int)eAllowedStances;
        tmp &= v6;
        eAllowedStances = (ai_stance_e)tmp;
    }
    Scr_AddInt((eAllowedStances & v3) != 0);
}

void __cdecl ActorCmd_IsSuppressionWaiting(scr_entref_t entref)
{
    actor_s *v1; // r3
    int IsSuppressionWaiting; // r3

    v1 = Actor_Get(entref);
    IsSuppressionWaiting = Actor_IsSuppressionWaiting(v1);
    Scr_AddInt(IsSuppressionWaiting);
}

void __cdecl ActorCmd_IsSuppressed(scr_entref_t entref)
{
    actor_s *v1; // r3
    int IsSuppressed; // r3

    v1 = Actor_Get(entref);
    IsSuppressed = Actor_IsSuppressed(v1);
    Scr_AddInt(IsSuppressed);
}

void __cdecl ActorCmd_IsMoveSuppressed(scr_entref_t entref)
{
    actor_s *v1; // r3
    int IsMoveSuppressed; // r3

    v1 = Actor_Get(entref);
    IsMoveSuppressed = Actor_IsMoveSuppressed(v1);
    Scr_AddInt(IsMoveSuppressed);
}

void __cdecl ActorCmd_CheckGrenadeThrow(scr_entref_t entref)
{
    actor_s *v1; // r31
    unsigned int ConstString; // r29
    double Float; // fp31
    gentity_s *ent; // r11
    bool v6; // r3
    float v7[4]; // [sp+50h] [-50h] BYREF
    float v8[6]; // [sp+60h] [-40h] BYREF

    v1 = Actor_Get(entref);
    if (!v1->sentient)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\actor_script_cmd.cpp", 1687, 0, "%s", "self->sentient");
    Scr_GetVector(0, v8);
    ConstString = Scr_GetConstString(1);
    Float = Scr_GetFloat(2);
    if (Actor_GetTargetSentient(v1) && v1->iGrenadeAmmo > 0)
    {
        ent = v1->ent;
        v7[0] = v1->ent->r.currentOrigin[0];
        v7[1] = ent->r.currentOrigin[1];
        v7[2] = ent->r.currentOrigin[2];
        v6 = Actor_Grenade_CheckToss(v1, v7, v8, ConstString, v1->vGrenadeTossPos, v1->vGrenadeTossVel, Float, 0);
        v1->bGrenadeTossValid = v6;
        if (v6)
        {
            Scr_SetString(&v1->GrenadeTossMethod, ConstString);
            Scr_AddVector(v1->vGrenadeTossVel);
        }
    }
}

void __cdecl ActorCmd_CheckGrenadeThrowPos(scr_entref_t entref)
{
    actor_s *self; // r31
    unsigned int ConstString; // r27
    gentity_s *ent; // r11
    int v4; // r10
    bool v5; // r3
    int v6; // [sp+8h] [-108h]
    int v7; // [sp+Ch] [-104h]
    int v8; // [sp+10h] [-100h]
    int v9; // [sp+14h] [-FCh]
    int v10; // [sp+18h] [-F8h]
    int v11; // [sp+1Ch] [-F4h]
    int v12; // [sp+20h] [-F0h]
    int v13; // [sp+24h] [-ECh]
    int v14; // [sp+28h] [-E8h]
    int v15; // [sp+2Ch] [-E4h]
    int v16; // [sp+30h] [-E0h]
    int v17; // [sp+34h] [-DCh]
    int v18; // [sp+38h] [-D8h]
    int v19; // [sp+3Ch] [-D4h]
    int v20; // [sp+40h] [-D0h]
    int v21; // [sp+44h] [-CCh]
    int v22; // [sp+48h] [-C8h]
    int v23; // [sp+4Ch] [-C4h]
    int v24; // [sp+50h] [-C0h]
    float vTargetPos[3]; // [sp+68h] [-A8h] BYREF
    float vStandPos[3]; // [sp+78h] [-98h] BYREF
    float vHandOffset[4]; // [sp+88h] [-88h] BYREF
    float v32[6]; // [sp+98h] [-78h] BYREF
    trace_t v33[2]; // [sp+B0h] [-60h] BYREF

    self = Actor_Get(entref);
    iassert(self->sentient);
    Scr_GetVector(0, vHandOffset);
    ConstString = Scr_GetConstString(1);
    Scr_GetVector(2, vTargetPos);
    if (self->iGrenadeAmmo > 0)
    {
        ent = self->ent;
        vStandPos[0] = self->ent->r.currentOrigin[0];
        vStandPos[1] = ent->r.currentOrigin[1];
        vStandPos[2] = ent->r.currentOrigin[2];
        v32[0] = vTargetPos[0];
        v32[1] = vTargetPos[1];
        v32[2] = vTargetPos[2] - (float)1.0;
        G_TraceCapsule(v33, vTargetPos, vec3_origin, vec3_origin, v32, ENTITYNUM_NONE, 2065);

        if (v33[0].fraction > 0.5)
            Com_Printf(18, "targetPos for checkGrenadeThrowPos not at ground level\n");

        self->vGrenadeTargetPos[0] = vTargetPos[0];
        self->vGrenadeTargetPos[1] = vTargetPos[1];
        self->vGrenadeTargetPos[2] = vTargetPos[2];

        iassert(!IS_NAN((vStandPos)[0]) && !IS_NAN((vStandPos)[1]) && !IS_NAN((vStandPos)[2]));
        iassert(!IS_NAN((vHandOffset)[0]) && !IS_NAN((vHandOffset)[1]) && !IS_NAN((vHandOffset)[2]));
        iassert(!IS_NAN((vTargetPos)[0]) && !IS_NAN((vTargetPos)[1]) && !IS_NAN((vTargetPos)[2]));
        
        v5 = Actor_Grenade_CheckTossPos(
            self,
            vStandPos,
            vHandOffset,
            vTargetPos,
            ConstString,
            self->vGrenadeTossPos,
            self->vGrenadeTossVel,
            0.0,
            0);

        self->bGrenadeTossValid = v5;

        if (v5)
        {
            Scr_SetString(&self->GrenadeTossMethod, ConstString);
            Scr_AddVector(self->vGrenadeTossVel);
        }
    }
}

void __cdecl ActorCmd_ThrowGrenade(scr_entref_t entref)
{
    actor_s *self; // r31
    const DObj_s *ServerDObj; // r3
    int number; // r30
    const char *v7; // r3
    const char *v8; // r3
    const char *v11; // r3
    const char *v12; // r3
    WeaponDef *weapDef; // r30
    int iGrenadeAmmo; // r11
    int v15; // [sp+8h] [-A8h]
    int v16; // [sp+Ch] [-A4h]
    int v17; // [sp+10h] [-A0h]
    int v18; // [sp+14h] [-9Ch]
    int v19; // [sp+18h] [-98h]
    int v20; // [sp+1Ch] [-94h]
    int v21; // [sp+20h] [-90h]
    int v22; // [sp+24h] [-8Ch]
    int v23; // [sp+28h] [-88h]
    int v24; // [sp+2Ch] [-84h]
    int v25; // [sp+30h] [-80h]
    int v26; // [sp+34h] [-7Ch]
    int v27; // [sp+38h] [-78h]
    int v28; // [sp+3Ch] [-74h]
    int v29; // [sp+40h] [-70h]
    int v30; // [sp+44h] [-6Ch]
    int v31; // [sp+48h] [-68h]
    int v32; // [sp+4Ch] [-64h]
    int v33; // [sp+50h] [-60h]
    float velOut[4]; // [sp+60h] [-50h] BYREF
    float posOut[4]; // [sp+70h] [-40h] BYREF
    float vStandPos[12]; // [sp+80h] [-30h] BYREF

    self = Actor_Get(entref);

    if (self->ent->health > 0 && (self->pGrenade.isDefined() || self->bGrenadeTossValid))
    {
        if (!self->bGrenadeTargetValid)
            goto LABEL_8;
        if (!G_DObjGetWorldTagPos(self->ent, scr_const.grenade_return_hand_tag, vStandPos))
        {
            ServerDObj = Com_GetServerDObj(self->ent->s.number);
            number = self->ent->s.number;
            Scr_Error(va("Missing tag [%s] on entity [%d] (%s)\n", SL_ConvertToString(scr_const.grenade_return_hand_tag), number, DObjGetName(ServerDObj)));
            return;
        }
        if (!Actor_Grenade_CheckTossPos(
            self,
            vStandPos,
            (float *)vec3_origin,
            self->vGrenadeTargetPos,
            self->GrenadeTossMethod,
            posOut,
            velOut,
            0.0,
            1))
        {
        LABEL_8:
            posOut[0] = self->vGrenadeTossPos[0];
            posOut[1] = self->vGrenadeTossPos[1];
            posOut[2] = self->vGrenadeTossPos[2];

            velOut[0] = self->vGrenadeTossVel[0];
            velOut[1] = self->vGrenadeTossVel[1];
            velOut[2] = self->vGrenadeTossVel[2];
        }
        if (self->pGrenade.isDefined() && self->eState[self->stateLevel] == AIS_GRENADE_RESPONSE)
        {
            Actor_Grenade_Detach(self);
            G_InitGrenadeEntity(self->ent, self->pGrenade.ent());
            G_InitGrenadeMovement(self->pGrenade.ent(), posOut, velOut, 1);
        }
        else
        {
            if (!self->iGrenadeWeaponIndex)
            {
                v11 = SL_ConvertToString(self->ent->classname);
                v12 = va("Actor [%s] doesn't have a grenade weapon set.", v11);
                Scr_Error(v12);
            }
            weapDef = BG_GetWeaponDef(self->iGrenadeWeaponIndex);
            iassert(weapDef);
            G_FireGrenade(self->ent, posOut, velOut, self->iGrenadeWeaponIndex, 0, 1, weapDef->aiFuseTime);
            iGrenadeAmmo = self->iGrenadeAmmo;
            if (iGrenadeAmmo > 0)
                self->iGrenadeAmmo = iGrenadeAmmo - 1;
        }
    }
    self->bGrenadeTossValid = 0;
    Scr_SetString(&self->GrenadeTossMethod, 0);
}

bool __cdecl Actor_CheckGrenadeLaunch(actor_s *self, const float *vStartPos, const float *vOffset)
{
    const char *v6; // r3
    __int64 v7; // r11
    float *v8; // r7
    double v9; // fp31
    const char *v10; // r3
    bool result; // r3

    if (!self->ent->s.weapon || self->ent->s.weapon >= bg_lastParsedWeaponIndex)
    {
        v6 = va("checkgrenadelaunch: invalid weapon for entity %d", self->ent->s.number);
        Scr_Error(v6);
    }
    LODWORD(v7) = BG_GetWeaponDef(self->ent->s.weapon)->iProjectileSpeed;
    v9 = (float)v7;
    if (v9 <= 0.0)
    {
        v10 = va("checkgrenadelaunch: grenade launcher speed must be > 0");
        Scr_Error(v10);
    }
    result = Actor_GrenadeLauncher_CheckPos(
        self,
        vStartPos,
        vOffset,
        self->vGrenadeTargetPos,
        v9,
        v8,
        self->vGrenadeTossPos,
        self->vGrenadeTossVel);
    self->bGrenadeTossValid = result;
    return result;
}

void __cdecl ActorCmd_CheckGrenadeLaunch(scr_entref_t entref)
{
    actor_s *v1; // r31
    const sentient_s *TargetSentient; // r3
    const sentient_s *v3; // r30
    double v4; // fp13
    double v5; // fp12
    float v6[4]; // [sp+50h] [-50h] BYREF
    float v7[4]; // [sp+60h] [-40h] BYREF
    float v8[6]; // [sp+70h] [-30h] BYREF

    v1 = Actor_Get(entref);
    if (!v1->sentient)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\actor_script_cmd.cpp", 1906, 0, "%s", "self->sentient");
    TargetSentient = Actor_GetTargetSentient(v1);
    v3 = TargetSentient;
    if (TargetSentient)
    {
        Sentient_GetOrigin(TargetSentient, v6);
        Sentient_GetVelocity(v3, v7);
        v4 = (float)(v7[1] + v6[1]);
        v5 = (float)(v7[2] + v6[2]);
        v1->vGrenadeTargetPos[0] = v7[0] + v6[0];
        v1->vGrenadeTargetPos[1] = v4;
        v1->vGrenadeTargetPos[2] = v5;
        Scr_GetVector(0, v8);
        if (Actor_CheckGrenadeLaunch(v1, v1->ent->r.currentOrigin, v8))
            Scr_AddVector(v1->vGrenadeTossVel);
    }
}

void __cdecl ActorCmd_CheckGrenadeLaunchPos(scr_entref_t entref)
{
    actor_s *v1; // r31
    float v2[4]; // [sp+50h] [-30h] BYREF
    float v3[4]; // [sp+60h] [-20h] BYREF

    v1 = Actor_Get(entref);
    if (!v1->sentient)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\actor_script_cmd.cpp", 1948, 0, "%s", "self->sentient");
    Scr_GetVector(0, v3);
    Scr_GetVector(1u, v2);
    v1->vGrenadeTargetPos[0] = v2[0];
    v1->vGrenadeTargetPos[1] = v2[1];
    v1->vGrenadeTargetPos[2] = v2[2];
    if (Actor_CheckGrenadeLaunch(v1, v1->ent->r.currentOrigin, v3))
        Scr_AddVector(v1->vGrenadeTossVel);
}

void __cdecl ActorCmd_FireGrenadeLauncher(scr_entref_t entref)
{
    actor_s *v1; // r3
    actor_s *v2; // r31
    unsigned int ConstString; // r30
    int number; // r31
    const char *v5; // r3
    const char *v6; // r3
    bool v7; // r3
    gentity_s *v8; // r3
    float v9[6]; // [sp+50h] [-30h] BYREF

    v1 = Actor_Get(entref);
    v2 = v1;
    if (v1->ent->health > 0)
    {
        if (!v1->bGrenadeTossValid)
        {
            v1->bGrenadeTossValid = 0;
            Scr_SetString(&v1->GrenadeTossMethod, 0);
            return;
        }
        if (v1->bGrenadeTargetValid)
        {
            ConstString = Scr_GetConstString(0);
            if (!G_DObjGetWorldTagPos(v2->ent, ConstString, v9))
            {
                number = v2->ent->s.number;
                v5 = SL_ConvertToString(ConstString);
                v6 = va("Missing tag [%s] on entity [%d]\n", v5, number);
                Scr_Error(v6);
                return;
            }
            v7 = Actor_CheckGrenadeLaunch(v2, v9, 0);
            v2->bGrenadeTossValid = v7;
            if (v7)
            {
                v8 = G_FireGrenade(v2->ent, v9, v2->vGrenadeTossVel, v2->ent->s.weapon, v2->ent->s.weaponModel, 0, 0);
                v8->flags |= FL_STABLE_MISSILES;
            }
        }
    }
    v2->bGrenadeTossValid = 0;
}

void __cdecl ActorCmd_PickUpGrenade(scr_entref_t entref)
{
    actor_s *v1; // r31

    v1 = Actor_Get(entref);
    if (v1->pGrenade.isDefined())
        Actor_Grenade_Attach(v1);
}

void __cdecl ActorCmd_UseTurret(scr_entref_t entref)
{
    actor_s *v1; // r30
    gentity_s *Entity; // r31
    int v3; // r3

    v1 = Actor_Get(entref);
    Entity = Scr_GetEntity(0);
    if (!Entity->pTurretInfo)
        Scr_ParamError(0, "can only use a turret");
    v3 = Actor_UseTurret(v1, Entity);
    Scr_AddBool(v3);
}

void __cdecl ActorCmd_StopUseTurret(scr_entref_t entref)
{
    actor_s *v1; // r3

    v1 = Actor_Get(entref);
    Actor_StopUseTurret(v1);
}

void __cdecl ActorCmd_CanUseTurret(scr_entref_t entref)
{
    actor_s *v1; // r30
    gentity_s *Entity; // r31
    int v3; // r3

    v1 = Actor_Get(entref);
    Entity = Scr_GetEntity(0);
    if (!Entity->pTurretInfo)
        Scr_ParamError(0, "can only use a turret");
    v3 = turret_canuse(v1, Entity);
    Scr_AddBool(v3);
}

void __cdecl ActorCmd_TraverseMode(scr_entref_t entref)
{
    actor_s *v1; // r31
    unsigned int ConstString; // r3

    v1 = Actor_Get(entref);
    ConstString = Scr_GetConstString(0);
    if (ConstString == scr_const.gravity)
    {
        v1->eTraverseMode = AI_TRAVERSE_GRAVITY;
    }
    else if (ConstString == scr_const.nogravity)
    {
        v1->eTraverseMode = AI_TRAVERSE_NOGRAVITY;
    }
    else if (ConstString == scr_const.noclip)
    {
        v1->eTraverseMode = AI_TRAVERSE_NOCLIP;
    }
    else
    {
        Scr_Error("traverseMode must be 'gravity', 'nogravity', or 'noclip'\n");
    }
}

void __cdecl ActorCmd_AnimMode(scr_entref_t entref)
{
    actor_s *v1; // r28
    bool v2; // r29
    unsigned int ConstString; // r30
    gentity_s *ent; // r11

    v1 = Actor_Get(entref);
    v2 = 1;
    ConstString = Scr_GetConstString(0);
    if (Scr_GetNumParam() > 1)
        v2 = Scr_GetInt(1) != 0;
    if (ConstString != scr_const.none && v2)
        Actor_ClearPath(v1);
    ent = v1->ent;
    if (ConstString == scr_const.nophysics)
    {
        v1->eScriptSetAnimMode = AI_ANIM_NOPHYSICS;
        ent->flags |= FL_NO_AUTO_ANIM_UPDATE;
    }
    else
    {
        ent->flags &= ~(FL_NO_AUTO_ANIM_UPDATE);
        if (ConstString == scr_const.gravity)
        {
            v1->eScriptSetAnimMode = AI_ANIM_USE_BOTH_DELTAS;
        }
        else if (ConstString == scr_const.nogravity)
        {
            v1->eScriptSetAnimMode = AI_ANIM_USE_BOTH_DELTAS_NOGRAVITY;
        }
        else if (ConstString == scr_const.angle_deltas)
        {
            v1->eScriptSetAnimMode = AI_ANIM_USE_ANGLE_DELTAS;
        }
        else if (ConstString == scr_const.zonly_physics)
        {
            v1->eScriptSetAnimMode = AI_ANIM_USE_BOTH_DELTAS_ZONLY_PHYSICS;
        }
        else if (ConstString == scr_const.none)
        {
            v1->eScriptSetAnimMode = AI_ANIM_UNKNOWN;
        }
        else if (ConstString == scr_const.normal)
        {
            v1->eScriptSetAnimMode = AI_ANIM_MOVE_CODE;
        }
        else
        {
            Scr_Error("illegal call to animmode()\n");
        }
    }
}

void __cdecl ActorCmd_OrientMode(scr_entref_t entref)
{
    actor_s *v1; // r31
    unsigned int ConstString; // r3
    double Float; // fp1
    gentity_s *ent; // r11
    float v5; // [sp+50h] [-50h] BYREF
    float v6; // [sp+54h] [-4Ch]
    float v7; // [sp+58h] [-48h]
    float v8[4]; // [sp+60h] [-40h] BYREF
    float v9[6]; // [sp+70h] [-30h] BYREF

    v1 = Actor_Get(entref);
    ConstString = Scr_GetConstString(0);
    if (ConstString == scr_const.face_angle)
    {
        v1->ScriptOrient.eMode = AI_ORIENT_DONT_CHANGE;
        Float = Scr_GetFloat(1);
        Actor_SetDesiredAngles(&v1->ScriptOrient, 0.0, Float);
    }
    else if (ConstString == scr_const.face_current)
    {
        v1->ScriptOrient.fDesiredBodyYaw = v1->fDesiredBodyYaw;
        v1->ScriptOrient.eMode = AI_ORIENT_DONT_CHANGE;
    }
    else if (ConstString == scr_const.face_direction)
    {
        Scr_GetVector(1u, v8);
        if (v8[0] == 0.0 && v8[1] == 0.0)
            Scr_Error("cannot face (0, 0, *)");
        v1->ScriptOrient.eMode = AI_ORIENT_DONT_CHANGE;
        Actor_FaceVector(&v1->ScriptOrient, v8);
    }
    else if (ConstString == scr_const.face_enemy)
    {
        v1->ScriptOrient.eMode = AI_ORIENT_TO_ENEMY;
    }
    else if (ConstString == scr_const.face_enemy_or_motion)
    {
        v1->ScriptOrient.eMode = AI_ORIENT_TO_ENEMY_OR_MOTION;
    }
    else if (ConstString == scr_const.face_goal)
    {
        v1->ScriptOrient.eMode = AI_ORIENT_TO_GOAL;
    }
    else if (ConstString == scr_const.face_motion)
    {
        v1->ScriptOrient.eMode = AI_ORIENT_TO_MOTION;
    }
    else if (ConstString == scr_const.face_point)
    {
        Scr_GetVector(1u, v9);
        ent = v1->ent;
        v5 = v9[0] - v1->ent->r.currentOrigin[0];
        v6 = v9[1] - ent->r.currentOrigin[1];
        v7 = v9[2] - ent->r.currentOrigin[2];
        v1->ScriptOrient.eMode = AI_ORIENT_DONT_CHANGE;
        if ((float)((float)(v5 * v5) + (float)(v6 * v6)) >= 1.0)
            Actor_FaceVector(&v1->ScriptOrient, &v5);
    }
    else if (ConstString == scr_const.face_default)
    {
        Actor_ClearScriptOrient(v1);
    }
    else
    {
        Scr_Error(
            "orientMode must be 'face angle', 'face current', 'face direction', 'face enemy', 'face enemy or motion', 'face goa"
            "l', 'face motion', 'face point', or 'face default'\n"
            "'face direction' and 'face point' take a second argument that is a vector giving the way to face\n"
            "'face angle' takes a second argument that is a yaw angle\n");
    }
    if (Scr_GetNumParam() > 2 && Scr_GetInt(2) > 0)
    {
        Actor_SetDesiredBodyAngle(&v1->CodeOrient, v1->ScriptOrient.fDesiredBodyYaw);
        Actor_SetBodyAngle(v1, v1->ScriptOrient.fDesiredBodyYaw);
    }
}

void __cdecl ActorCmd_GetMotionAngle(scr_entref_t entref)
{
    actor_s *v1; // r31
    double v2; // fp1
    double v3; // fp31
    long double v4; // fp2
    long double v5; // fp2
    double v6; // fp1
    double v7; // fp1

    v1 = Actor_Get(entref);
    if ((float)((float)(v1->Physics.vVelocity[0] * v1->Physics.vVelocity[0])
        + (float)(v1->Physics.vVelocity[1] * v1->Physics.vVelocity[1])) <= 1.0)
    {
        if (Actor_HasPath(v1))
        {
            v7 = vectoyaw(v1->Path.lookaheadDir);
            v6 = AngleSubtract(v7, v1->ent->r.currentAngles[1]);
        }
        else
        {
            v6 = 0.0;
        }
    }
    else
    {
        v2 = vectoyaw(v1->Physics.vVelocity);
        v3 = (float)((float)((float)v2 - v1->ent->r.currentAngles[1]) * (float)0.0027777778);
        *(double *)&v4 = (float)((float)((float)((float)v2 - v1->ent->r.currentAngles[1]) * (float)0.0027777778) + (float)0.5);
        v5 = floor(v4);
        v6 = (float)((float)((float)v3 - (float)*(double *)&v5) * (float)360.0);
    }
    Scr_AddFloat(v6);
}

void __cdecl ActorCmd_GetAnglesToLikelyEnemyPath(scr_entref_t entref)
{
    actor_s *v1; // r31

    v1 = Actor_Get(entref);
    if ((unsigned __int8)Actor_GetAnglesToLikelyEnemyPath(v1))
        Scr_AddVector(v1->anglesToLikelyEnemyPath);
}

void __cdecl ActorCmd_SetTurretAnim(scr_entref_t entref)
{
    actor_s *v1; // r31
    XAnimTree_s *ActorAnimTree; // r3
    XAnimTree_s *v3; // r5

    v1 = Actor_Get(entref);
    ActorAnimTree = G_GetActorAnimTree(v1);
    v1->turretAnim = Scr_GetAnim(0, ActorAnimTree).index;
    v1->turretAnimSet = 1;
}

void __cdecl ActorCmd_GetTurret(scr_entref_t entref)
{
    actor_s *v1; // r31

    v1 = Actor_Get(entref);
    if (Actor_IsUsingTurret(v1))
    {
        if (!v1->pTurret)
            MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\actor_script_cmd.cpp", 2392, 0, "%s", "self->pTurret");
        Scr_AddEntity(v1->pTurret);
    }
}

void __cdecl ActorCmd_BeginPrediction(scr_entref_t entref)
{
    actor_s *v1; // r31
    int actorPredictDepth; // r11
    XAnimTree_s *actorBackupXAnimTree; // r29
    const XAnimTree_s *ActorAnimTree; // r3
    actorBackup_s *actorBackup; // r11
    gentity_s *ent; // r11
    actorBackup_s *v7; // r10
    gentity_s *v8; // r11
    actorBackup_s *v9; // r10
    actorBackup_s *v10; // r11
    actorBackup_s *v11; // r11
    actorBackup_s *v12; // r11

    v1 = Actor_Get(entref);
    actorPredictDepth = level.actorPredictDepth;
    if (level.actorPredictDepth)
    {
        Scr_Error("beginPrediction already called");
        actorPredictDepth = level.actorPredictDepth;
    }
    level.actorPredictDepth = actorPredictDepth + 1;
    actorBackupXAnimTree = g_scr_data.actorBackupXAnimTree;
    if (!g_scr_data.actorBackupXAnimTree)
    {
        MyAssertHandler(
            "c:\\trees\\cod3\\cod3src\\src\\game\\actor_script_cmd.cpp",
            2416,
            0,
            "%s",
            "g_scr_data.actorBackupXAnimTree");
        actorBackupXAnimTree = g_scr_data.actorBackupXAnimTree;
    }
    ActorAnimTree = G_GetActorAnimTree(v1);
    XAnimCloneAnimTree(ActorAnimTree, actorBackupXAnimTree);
    actorBackup = g_scr_data.actorBackup;
    if (!g_scr_data.actorBackup)
    {
        MyAssertHandler(
            "c:\\trees\\cod3\\cod3src\\src\\game\\actor_script_cmd.cpp",
            2419,
            0,
            "%s",
            "g_scr_data.actorBackup");
        actorBackup = g_scr_data.actorBackup;
    }
    actorBackup->eAnimMode = v1->eAnimMode;
    g_scr_data.actorBackup->eScriptSetAnimMode = v1->eScriptSetAnimMode;
    g_scr_data.actorBackup->bUseGoalWeight = v1->bUseGoalWeight;
    memcpy(&g_scr_data.actorBackup->Physics, &v1->Physics, sizeof(g_scr_data.actorBackup->Physics));
    g_scr_data.actorBackup->ScriptOrient = v1->ScriptOrient;
    g_scr_data.actorBackup->CodeOrient = v1->CodeOrient;
    g_scr_data.actorBackup->fDesiredBodyYaw = v1->fDesiredBodyYaw;
    ent = v1->ent;
    v7 = g_scr_data.actorBackup;
    g_scr_data.actorBackup->currentOrigin[0] = v1->ent->r.currentOrigin[0];
    v7->currentOrigin[1] = ent->r.currentOrigin[1];
    v7->currentOrigin[2] = ent->r.currentOrigin[2];
    v8 = v1->ent;
    v9 = g_scr_data.actorBackup;
    g_scr_data.actorBackup->currentAngles[0] = v1->ent->r.currentAngles[0];
    v9->currentAngles[1] = v8->r.currentAngles[1];
    v9->currentAngles[2] = v8->r.currentAngles[2];
    v10 = g_scr_data.actorBackup;
    g_scr_data.actorBackup->vLookForward[0] = v1->vLookForward[0];
    v10->vLookForward[1] = v1->vLookForward[1];
    v10->vLookForward[2] = v1->vLookForward[2];
    v11 = g_scr_data.actorBackup;
    g_scr_data.actorBackup->vLookRight[0] = v1->vLookRight[0];
    v11->vLookRight[1] = v1->vLookRight[1];
    v11->vLookRight[2] = v1->vLookRight[2];
    v12 = g_scr_data.actorBackup;
    g_scr_data.actorBackup->vLookUp[0] = v1->vLookUp[0];
    v12->vLookUp[1] = v1->vLookUp[1];
    v12->vLookUp[2] = v1->vLookUp[2];
}

void __cdecl ActorCmd_EndPrediction(scr_entref_t entref)
{
    actor_s *v1; // r31
    XAnimTree_s *ActorAnimTree; // r3
    actorBackup_s *actorBackup; // r11
    float *p_eType; // r8
    actorBackup_s *v5; // r11
    actorBackup_s *v6; // r10
    float *v7; // r11
    actorBackup_s *v8; // r11
    actorBackup_s *v9; // r11
    actorBackup_s *v10; // r11

    v1 = Actor_Get(entref);
    if (--level.actorPredictDepth)
        Scr_Error("endPrediction already called");
    if (!g_scr_data.actorBackupXAnimTree)
        MyAssertHandler(
            "c:\\trees\\cod3\\cod3src\\src\\game\\actor_script_cmd.cpp",
            2453,
            0,
            "%s",
            "g_scr_data.actorBackupXAnimTree");
    ActorAnimTree = G_GetActorAnimTree(v1);
    XAnimCloneAnimTree(g_scr_data.actorBackupXAnimTree, ActorAnimTree);
    XAnimClearTree(g_scr_data.actorBackupXAnimTree);
    actorBackup = g_scr_data.actorBackup;
    if (!g_scr_data.actorBackup)
    {
        MyAssertHandler(
            "c:\\trees\\cod3\\cod3src\\src\\game\\actor_script_cmd.cpp",
            2457,
            0,
            "%s",
            "g_scr_data.actorBackup");
        actorBackup = g_scr_data.actorBackup;
    }
    v1->eAnimMode = actorBackup->eAnimMode;
    v1->eScriptSetAnimMode = g_scr_data.actorBackup->eScriptSetAnimMode;
    v1->bUseGoalWeight = g_scr_data.actorBackup->bUseGoalWeight;
    memcpy(&v1->Physics, &g_scr_data.actorBackup->Physics, sizeof(v1->Physics));
    p_eType = (float *)&v1->ent->s.eType;
    v1->ScriptOrient = g_scr_data.actorBackup->ScriptOrient;
    v1->CodeOrient = g_scr_data.actorBackup->CodeOrient;
    v1->fDesiredBodyYaw = g_scr_data.actorBackup->fDesiredBodyYaw;
    v5 = g_scr_data.actorBackup;
    p_eType[56] = g_scr_data.actorBackup->currentOrigin[0];
    p_eType[57] = v5->currentOrigin[1];
    p_eType[58] = v5->currentOrigin[2];
    v6 = g_scr_data.actorBackup;
    v7 = (float *)&v1->ent->s.eType;
    v7[59] = g_scr_data.actorBackup->currentAngles[0];
    v7[60] = v6->currentAngles[1];
    v7[61] = v6->currentAngles[2];
    v8 = g_scr_data.actorBackup;
    v1->vLookForward[0] = g_scr_data.actorBackup->vLookForward[0];
    v1->vLookForward[1] = v8->vLookForward[1];
    v1->vLookForward[2] = v8->vLookForward[2];
    v9 = g_scr_data.actorBackup;
    v1->vLookRight[0] = g_scr_data.actorBackup->vLookRight[0];
    v1->vLookRight[1] = v9->vLookRight[1];
    v1->vLookRight[2] = v9->vLookRight[2];
    v10 = g_scr_data.actorBackup;
    v1->vLookUp[0] = g_scr_data.actorBackup->vLookUp[0];
    v1->vLookUp[1] = v10->vLookUp[1];
    v1->vLookUp[2] = v10->vLookUp[2];
}

void __cdecl ActorCmd_LerpPosition(scr_entref_t entref)
{
    actor_s *v1; // r31
    float *p_eType; // r30
    float v3; // [sp+50h] [-40h] BYREF
    float v4; // [sp+54h] [-3Ch]
    float v5; // [sp+58h] [-38h]
    float v6[6]; // [sp+60h] [-30h] BYREF

    v1 = Actor_Get(entref);
    if (v1->eScriptSetAnimMode != AI_ANIM_NOPHYSICS)
        Scr_Error("cannot lerp position if animMode is not 'nophysics'");
    p_eType = (float *)&v1->ent->s.eType;
    Scr_GetVector(0, &v3);
    Scr_GetVector(1u, v6);
    p_eType[59] = v6[0];
    p_eType[60] = v6[1];
    p_eType[61] = v6[2];
    Actor_SetDesiredAngles(&v1->CodeOrient, p_eType[59], p_eType[60]);
    v1->Physics.vVelocity[0] = v3 - p_eType[56];
    v1->Physics.vVelocity[1] = v4 - p_eType[57];
    v1->Physics.vVelocity[2] = v5 - p_eType[58];
    v1->Physics.vVelocity[0] = v1->Physics.vVelocity[0] * (float)20.0;
    v1->Physics.vVelocity[1] = v1->Physics.vVelocity[1] * (float)20.0;
    v1->Physics.vVelocity[2] = v1->Physics.vVelocity[2] * (float)20.0;
    v1->Physics.vWishDelta[0] = 0.0;
    v1->Physics.vWishDelta[1] = 0.0;
    v1->Physics.vWishDelta[2] = 0.0;
    p_eType[56] = v3;
    p_eType[57] = v4;
    p_eType[58] = v5;
    v1->Physics.groundEntNum = ENTITYNUM_NONE;
}

void __cdecl ActorCmd_PredictOriginAndAngles(scr_entref_t entref)
{
    actor_s *self; // r29
    gentity_s *ent; // r28
    int eScriptSetAnimMode; // r11

    self = Actor_Get(entref);
    iassert(self);
    ent = self->ent;
    iassert(ent);

    if (ent->tagInfo)
        Scr_Error("cannot predict when linked to an entity");

    if (self->eScriptSetAnimMode == AI_ANIM_NOPHYSICS)
        Scr_Error("cannot predict when using no physics");

    eScriptSetAnimMode = self->eScriptSetAnimMode;

    if (!eScriptSetAnimMode)
        eScriptSetAnimMode = 4;

    self->eAnimMode = (ai_animmode_t)eScriptSetAnimMode;
    self->bUseGoalWeight = 1;
    Actor_PredictOriginAndAngles(self);
}

void __cdecl ActorCmd_PredictAnim(scr_entref_t entref)
{
    actor_s *v1; // r3

    v1 = Actor_Get(entref);
    Actor_PredictAnim(v1);
}

void __cdecl Actor_GetEntType(int entnum)
{
    unsigned __int16 obstacle; // r11

    if (entnum == ENTITYNUM_NONE)
    {
        Scr_AddConstString(scr_const.none);
    }
    else
    {
        if ((level.gentities[entnum].flags & 0x400) != 0)
            obstacle = scr_const.obstacle;
        else
            obstacle = scr_const.world;
        Scr_AddConstString(obstacle);
    }
}

void __cdecl ActorCmd_GetHitEntType(scr_entref_t entref)
{
    int iHitEntnum; // r11
    unsigned __int16 obstacle; // r11

    iHitEntnum = Actor_Get(entref)->Physics.iHitEntnum;
    if (iHitEntnum == ENTITYNUM_NONE)
    {
        Scr_AddConstString(scr_const.none);
    }
    else
    {
        if ((level.gentities[iHitEntnum].flags & 0x400) != 0)
            obstacle = scr_const.obstacle;
        else
            obstacle = scr_const.world;
        Scr_AddConstString(obstacle);
    }
}

void __cdecl ActorCmd_GetHitYaw(scr_entref_t entref)
{
    actor_s *v1; // r31
    double v2; // fp1
    float v3[4]; // [sp+50h] [-20h] BYREF

    v1 = Actor_Get(entref);
    if (v1->Physics.iHitEntnum == ENTITYNUM_NONE)
        Scr_Error("nothing was hit");
    v3[0] = -v1->Physics.vHitNormal[0];
    v3[1] = -v1->Physics.vHitNormal[1];
    v3[2] = 0.0;
    v2 = vectoyaw(v3);
    Scr_AddFloat(v2);
}

void __cdecl ActorCmd_GetGroundEntType(scr_entref_t entref)
{
    int groundEntNum; // r11
    unsigned __int16 obstacle; // r11

    groundEntNum = Actor_Get(entref)->Physics.groundEntNum;
    if (groundEntNum == ENTITYNUM_NONE)
    {
        Scr_AddConstString(scr_const.none);
    }
    else
    {
        if ((level.gentities[groundEntNum].flags & 0x400) != 0)
            obstacle = scr_const.obstacle;
        else
            obstacle = scr_const.world;
        Scr_AddConstString(obstacle);
    }
}

void __cdecl ActorCmd_IsDeflected(scr_entref_t entref)
{
    actor_s *v1; // r3

    v1 = Actor_Get(entref);
    Scr_AddInt(v1->Physics.bDeflected);
}

void __cdecl ActorCmd_trackScriptState(scr_entref_t entref)
{
    actor_s *v1; // r31
    unsigned int ConstString; // r3
    unsigned int v3; // r3
    unsigned int lastScriptState; // r3
    const char *v5; // r29
    const char *v6; // r3
    const char *v7; // r3

    v1 = Actor_Get(entref);
    if (Scr_GetNumParam() != 2)
        Scr_Error("trackScriptState newStateName, reasonForTransition");
    Scr_SetString(&v1->lastScriptState, v1->scriptState);
    ConstString = Scr_GetConstString(0);
    Scr_SetString(&v1->scriptState, ConstString);
    v3 = Scr_GetConstString(1);
    Scr_SetString(&v1->stateChangeReason, v3);
    lastScriptState = v1->lastScriptState;
    if (v1->scriptState == lastScriptState)
    {
        v5 = SL_ConvertToString(lastScriptState);
        v6 = SL_ConvertToString(v1->scriptState);
        v7 = va(
            "trackScriptState should only be called on script state transitions.  Called for state %s from state %s.",
            v6,
            v5);
        Scr_ErrorWithDialogMessage(v7, "");
    }
}

// attributes: thunk
void __cdecl ActorCmd_DumpHistory(scr_entref_t entref)
{
    Actor_Get(entref);
}

void __cdecl ScrCmd_animcustom(scr_entref_t entref)
{
    actor_s *v1; // r31
    int Func; // r30

    v1 = Actor_Get(entref);
    if (Actor_PushState(v1, AIS_CUSTOMANIM))
    {
        Func = Scr_GetFunc(0);
        Actor_KillAnimScript(v1);
        v1->AnimScriptSpecific.func = Func;
        Scr_SetString(&v1->AnimScriptSpecific.name, scr_const._custom);
    }
}

void __cdecl ScrCmd_CanAttackEnemyNode(scr_entref_t entref)
{
    actor_s *v1; // r31
    const pathnode_t *v2; // r30
    sentient_s *TargetSentient; // r3
    const pathnode_t *v4; // r3
    int v5; // r3
    bool v6; // zf

    v1 = Actor_Get(entref);
    if (!Actor_GetTargetSentient(v1)
        || (v2 = Sentient_NearestNode(v1->sentient),
            TargetSentient = Actor_GetTargetSentient(v1),
            v4 = Sentient_NearestNode(TargetSentient),
            !v2)
        || !v4
        || (v6 = Path_NodesVisible(v2, v4) != 0, v5 = 1, !v6))
    {
        v5 = 0;
    }
    Scr_AddInt(v5);
}

void __cdecl ScrCmd_GetNegotiationStartNode(scr_entref_t entref)
{
    actor_s *v1; // r31
    pathnode_t *v2; // r3

    v1 = Actor_Get(entref);
    if (Path_HasNegotiationNode(&v1->Path))
    {
        if (v1->Path.wNegotiationStartNode >= v1->Path.wPathLen)
            MyAssertHandler(
                "c:\\trees\\cod3\\cod3src\\src\\game\\actor_script_cmd.cpp",
                2793,
                0,
                "%s",
                "self->Path.wNegotiationStartNode < self->Path.wPathLen");
        if (v1->Path.pts[v1->Path.wNegotiationStartNode].iNodeNum < 0)
            MyAssertHandler(
                "c:\\trees\\cod3\\cod3src\\src\\game\\actor_script_cmd.cpp",
                2794,
                0,
                "%s",
                "self->Path.pts[self->Path.wNegotiationStartNode].iNodeNum >= 0");
        v2 = Path_ConvertIndexToNode(v1->Path.pts[v1->Path.wNegotiationStartNode].iNodeNum);
        Scr_AddPathnode(v2);
    }
}

void __cdecl ScrCmd_GetNegotiationEndNode(scr_entref_t entref)
{
    actor_s *v1; // r31
    pathnode_t *v2; // r3

    v1 = Actor_Get(entref);
    if (Path_HasNegotiationNode(&v1->Path))
    {
        if (v1->Path.wNegotiationStartNode >= v1->Path.wPathLen)
            MyAssertHandler(
                "c:\\trees\\cod3\\cod3src\\src\\game\\actor_script_cmd.cpp",
                2817,
                0,
                "%s",
                "self->Path.wNegotiationStartNode < self->Path.wPathLen");
        if (v1->Physics.vHitNormal[7 * v1->Path.wNegotiationStartNode + 2] < 0.0)
            MyAssertHandler(
                "c:\\trees\\cod3\\cod3src\\src\\game\\actor_script_cmd.cpp",
                2818,
                0,
                "%s",
                "self->Path.pts[self->Path.wNegotiationStartNode - 1].iNodeNum >= 0");
        v2 = Path_ConvertIndexToNode(LODWORD(v1->Physics.vHitNormal[7 * v1->Path.wNegotiationStartNode + 2]));
        Scr_AddPathnode(v2);
    }
}

void __cdecl ActorCmd_CheckProne(scr_entref_t entref)
{
    double Float; // fp31
    unsigned int Int; // r3
    bool v3; // r7
    float *v4; // r6
    float *v5; // r5
    bool v6; // r3
    proneCheckType_t v7; // [sp+8h] [-98h]
    float v8[4]; // [sp+80h] [-20h] BYREF
    unsigned __int16 v9; // [sp+B4h] [+14h]

    v9 = entref.entnum;
    Actor_Get(entref);
    Scr_GetVector(0, v8);
    Float = Scr_GetFloat(1);
    Int = Scr_GetInt(2);
    //v6 = BG_CheckProneValid(v9, v8, 15.0, 48.0, Float, v5, v4, v3, 0, 0, (_cntlzw(Int) & 0x20) == 0, v7, 50.0);
    v6 = BG_CheckProneValid(v9, v8, 15.0, 48.0, Float, v5, v4, v3, 0, 0, Int != 0, v7, 50.0);
    Scr_AddBool(v6);
}

void __cdecl ActorCmd_PushPlayer(scr_entref_t entref)
{
    actor_s *v1; // r31
    int Int; // r3
    int iTraceMask; // r11
    unsigned int v4; // r11

    v1 = Actor_Get(entref);
    Int = Scr_GetInt(0);
    iTraceMask = v1->Physics.iTraceMask;
    if (Int)
        v4 = iTraceMask & 0xFDFFFFFF;
    else
        v4 = iTraceMask | 0x2000000;
    v1->Physics.iTraceMask = v4;
}

void __cdecl Actor_SetScriptGoalVolume(actor_s *self, gentity_s *volume)
{
    iassert(volume);
    iassert(volume->r.inuse);
    iassert(!self->scriptGoalEnt.isDefined());
    iassert(SV_EntityContact(self->scriptGoal.pos, self->scriptGoal.pos, volume));

    self->scriptGoal.volume = volume;
}

void __cdecl ActorCmd_SetGoalNode(scr_entref_t entref)
{
    actor_s *self; // r30
    pathnode_t *Pathnode; // r3

    self = Actor_Get(entref);
    Pathnode = Scr_GetPathnode(0);

    if (!Pathnode->constant.totalLinkCount && (Pathnode->constant.spawnflags & 1) == 0)
    {
        Com_PrintError(
            18,
            "AI %d's goal node at (%0.f, %0.f, %0.f) does not have any path links\n",
            0, // KISAKTODO: index...
            Pathnode->constant.vOrigin[0],
            Pathnode->constant.vOrigin[1],
            Pathnode->constant.vOrigin[2]
        );
    }
        
          
    Actor_SetScriptGoalPos(self, Pathnode->constant.vOrigin, Pathnode);
}

void __cdecl Actor_SetScriptGoalEntity(actor_s *self, gentity_s *pGoalEnt)
{
    iassert(pGoalEnt);
    iassert(pGoalEnt->r.inuse);
    Actor_ClearKeepClaimedNode(self);
    self->scriptGoalEnt.setEnt(pGoalEnt);
    self->scriptGoal.node = 0;
    self->scriptGoal.volume = 0;
}

void __cdecl ActorCmd_SetGoalPos(scr_entref_t entref)
{
    actor_s *v1; // r31
    float v2[4]; // [sp+50h] [-20h] BYREF

    v1 = Actor_Get(entref);
    Scr_GetVector(0, v2);
    Actor_SetScriptGoalPos(v1, v2, 0);
}

void __cdecl ActorCmd_SetGoalEntity(scr_entref_t entref)
{
    actor_s *v1; // r31
    gentity_s *Entity; // r30

    v1 = Actor_Get(entref);
    Entity = Scr_GetEntity(0);
    Actor_ClearKeepClaimedNode(v1);
    Actor_SetScriptGoalEntity(v1, Entity);
}

void __cdecl ActorCmd_SetGoalVolume(scr_entref_t entref)
{
    actor_s *self; // r31
    gentity_s *Entity; // r30

    self = Actor_Get(entref);
    if (self->scriptGoalEnt.isDefined())
        Scr_Error("cannot set goal volume when a goal entity is set");
    Entity = Scr_GetEntity(0);
    if (!SV_EntityContact(self->scriptGoal.pos, self->scriptGoal.pos, Entity))
        Scr_Error("cannot set goal volume which does not contain goal position");
    Actor_ClearKeepClaimedNode(self);
    Actor_SetScriptGoalVolume(self, Entity);
}

void __cdecl ActorCmd_GetGoalVolume(scr_entref_t entref)
{
    gentity_s *volume; // r3

    volume = Actor_Get(entref)->scriptGoal.volume;
    if (volume)
        Scr_AddEntity(volume);
}

void __cdecl ActorCmd_ClearGoalVolume(scr_entref_t entref)
{
    Actor_Get(entref)->scriptGoal.volume = 0;
}

void __cdecl ActorCmd_SetFixedNodeSafeVolume(scr_entref_t entref)
{
    actor_s *self; // r30
    gentity_s *Entity; // r31

    self = Actor_Get(entref);
    Entity = Scr_GetEntity(0);
    self->fixedNodeSafeVolume.setEnt(Entity);
    self->fixedNodeSafeVolumeRadiusSq = RadiusFromBounds2DSq(Entity->r.mins, Entity->r.maxs);
}

void __cdecl ActorCmd_GetFixedNodeSafeVolume(scr_entref_t entref)
{
    EntHandle *p_fixedNodeSafeVolume; // r31

    p_fixedNodeSafeVolume = &Actor_Get(entref)->fixedNodeSafeVolume;
    if (p_fixedNodeSafeVolume->isDefined())
    {
        Scr_AddEntity(p_fixedNodeSafeVolume->ent());
    }
}

void __cdecl ActorCmd_ClearFixedNodeSafeVolume(scr_entref_t entref)
{
    actor_s *self;

    self = Actor_Get(entref);
    self->fixedNodeSafeVolume.setEnt(NULL);
    self->fixedNodeSafeVolumeRadiusSq = 0.0f;
}

void __cdecl ActorCmd_IsInGoal(scr_entref_t entref)
{
    actor_s *v1; // r31
    unsigned __int8 v2; // r3
    float v3[4]; // [sp+50h] [-20h] BYREF

    v1 = Actor_Get(entref);
    if (Scr_GetNumParam() == 1)
    {
        Scr_GetVector(0, v3);
        v2 = Actor_PointAtGoal(v3, &v1->codeGoal);
        Scr_AddBool(v2);
    }
    else
    {
        Scr_Error("illegal call to isingoal()\n");
    }
}

void __cdecl ActorCmd_SetOverrideRunToPos(scr_entref_t entref)
{
    actor_s *self; // r31
    float pos[3];

    self = Actor_Get(entref);
    Scr_GetVector(0, pos);

    bool posEqual = 
        pos[0] == self->arrivalInfo.animscriptOverrideRunToPos[0]
        && pos[1] == self->arrivalInfo.animscriptOverrideRunToPos[1]
        && pos[2] == self->arrivalInfo.animscriptOverrideRunToPos[2];

    if (!posEqual || !self->arrivalInfo.animscriptOverrideRunTo)
    {
        Scr_Notify(self->ent, scr_const.goal_changed, 0);
    }

    self->arrivalInfo.animscriptOverrideRunTo = 1;
    self->arrivalInfo.animscriptOverrideRunToPos[0] = pos[0];
    self->arrivalInfo.animscriptOverrideRunToPos[1] = pos[1];
    self->arrivalInfo.animscriptOverrideRunToPos[2] = pos[2];
}

void __cdecl ActorCmd_NearNode(scr_entref_t entref)
{
    actor_s *v1; // r31
    const pathnode_t *Pathnode; // r3
    bool v3; // r3

    v1 = Actor_Get(entref);
    Pathnode = Scr_GetPathnode(0);
    v3 = Actor_PointNearNode(v1->ent->r.currentOrigin, Pathnode);
    Scr_AddBool(v3);
}

void __cdecl ActorCmd_ClearEnemy(scr_entref_t entref)
{
    actor_s *self; // r31

    self = Actor_Get(entref);

    iassert(self->sentient);

    if (Actor_GetTargetSentient(self))
    {
        self->sentientInfo[Actor_GetTargetSentient(self) - level.sentients].lastKnownPosTime = 0;
        self->faceLikelyEnemyPathNode = 0;
    }
    if (self->sentient->scriptTargetEnt.isDefined())
    {
        if (Actor_GetTargetEntity(self) == self->sentient->scriptTargetEnt.ent())
            self->sentient->scriptTargetEnt.setEnt(NULL);
    }
    Sentient_SetEnemy(self->sentient, 0, 1);
}

void __cdecl ActorCmd_SetEntityTarget(scr_entref_t entref)
{
    actor_s *self; // r29
    gentity_s *targetEnt; // r28
    double Float; // fp1
    double v5; // fp0
    sentient_s *sentient; // r3

    self = Actor_Get(entref);
    targetEnt = Scr_GetEntity(0);
    iassert(targetEnt);

    if (targetEnt->sentient)
        Scr_Error("Do not use setentitytarget to set an AI or player as a target");
    
    iassert(self->sentient);

    self->sentient->scriptTargetEnt.setEnt(targetEnt);

    if (Scr_GetNumParam() <= 1)
    {
        v5 = 1.0;
        self->sentient->entityTargetThreat = 1.0;
    }
    else
    {
        Float = Scr_GetFloat(1);

        // aislop
        //_FP12 = -Float;
        //v5 = 1.0;
        //_FP13 = (float)((float)Float - (float)1.0);
        //__asm { fsel      f11, f13, f0, f1 }
        //__asm { fsel      f13, f12, f13, f11 }
        //self->sentient->entityTargetThreat = _FP13;

        self->sentient->entityTargetThreat = (Float >= 0.0f) ? (Float - 1.0f) : 1.0f;
    }
    sentient = self->sentient;
    if (sentient->entityTargetThreat == v5)
        Sentient_SetEnemy(sentient, targetEnt, 1);
}

void __cdecl ActorCmd_ClearEntityTarget(scr_entref_t entref)
{
    actor_s *self; // r31

    self = Actor_Get(entref);
    if (self->sentient->scriptTargetEnt.isDefined())
    {
        if (Actor_GetTargetEntity(self) == self->sentient->scriptTargetEnt.ent())
            Sentient_SetEnemy(self->sentient, 0, 1);
    }
    self->sentient->scriptTargetEnt.setEnt(NULL);
}

void __cdecl ActorCmd_SetPotentialThreat(scr_entref_t entref)
{
    actor_s *v1; // r31
    double Float; // fp1

    v1 = Actor_Get(entref);
    if (!v1 || Scr_GetNumParam() != 1)
        Scr_Error("illegal call to setpotentialthreat()\n");
    Float = Scr_GetFloat(0);
    Actor_SetPotentialThreat(&v1->potentialThreat, Float);
}

void __cdecl ActorCmd_ClearPotentialThreat(scr_entref_t entref)
{
    actor_s *v1; // r31

    v1 = Actor_Get(entref);
    if (!v1 || Scr_GetNumParam())
        Scr_Error("illegal call to clearpotentialthreat()\n");
    Actor_ClearPotentialThreat(&v1->potentialThreat);
}

static const char *USAGEMSG = "Invalid call to setFlashBanged().";

void __cdecl ActorCmd_SetFlashBanged(scr_entref_t entref)
{
    actor_s *v1; // r31
    double Float; // fp31
    unsigned int NumParam; // r3
    int Int; // r3

    v1 = Actor_Get(entref);
    Float = 1.0;
    NumParam = Scr_GetNumParam();
    if (NumParam != 1)
    {
        if (NumParam != 2)
        {
            Scr_Error(USAGEMSG);
            return;
        }
        Float = Scr_GetFloat(1);
    }
    Int = Scr_GetInt(0);
    Actor_SetFlashed(v1, Int, Float);
}

static const char *USAGEMSG_0 = "Invalid call to setFlashbangImmunity().";

void __cdecl ActorCmd_SetFlashbangImmunity(scr_entref_t entref)
{
    actor_s *v1; // r31

    v1 = Actor_Get(entref);
    if (Scr_GetNumParam() == 1)
        v1->flashBangImmunity = Scr_GetInt(0);
    else
        Scr_Error(USAGEMSG_0);
}

void __cdecl ActorCmd_GetFlashBangedStrength(scr_entref_t entref)
{
    actor_s *v1; // r3

    v1 = Actor_Get(entref);
    Scr_AddFloat(v1->flashBangedStrength);
}

void __cdecl ActorCmd_SetEngagementMinDist(scr_entref_t entref)
{
    actor_s *v1; // r31
    double Float; // fp2
    double engageMinDist; // fp1
    const char *v4; // r3

    v1 = Actor_Get(entref);
    v1->engageMinDist = Scr_GetFloat(0);
    Float = Scr_GetFloat(1);
    engageMinDist = v1->engageMinDist;
    v1->engageMinFalloffDist = Float;
    if (Float > engageMinDist)
    {
        v4 = va((const char *)HIDWORD(engageMinDist), LODWORD(engageMinDist), LODWORD(Float));
        Scr_Error(v4);
    }
}

void __cdecl ActorCmd_SetEngagementMaxDist(scr_entref_t entref)
{
    actor_s *v1; // r31
    double Float; // fp2
    double engageMaxDist; // fp1
    const char *v4; // r3

    v1 = Actor_Get(entref);
    v1->engageMaxDist = Scr_GetFloat(0);
    Float = Scr_GetFloat(1);
    engageMaxDist = v1->engageMaxDist;
    v1->engageMaxFalloffDist = Float;
    if (Float < engageMaxDist)
    {
        v4 = va((const char *)HIDWORD(engageMaxDist), LODWORD(engageMaxDist), LODWORD(Float));
        Scr_Error(v4);
    }
}

void __cdecl ActorCmd_IsKnownEnemyInRadius(scr_entref_t entref)
{
    const actor_s *v1; // r31
    double Float; // fp1
    bool v3; // r3
    float v4[4]; // [sp+50h] [-20h] BYREF

    v1 = Actor_Get(entref);
    Scr_GetVector(0, v4);
    Float = Scr_GetFloat(1);
    v3 = Actor_IsKnownEnemyInRegion(v1, 0, v4, Float) == 0;
    Scr_AddInt(v3);
}

void __cdecl ActorCmd_IsKnownEnemyInVolume(scr_entref_t entref)
{
    const actor_s *v1; // r31
    const gentity_s *Entity; // r3
    bool v3; // r3

    v1 = Actor_Get(entref);
    Entity = Scr_GetEntity(0);
    v3 = Actor_IsKnownEnemyInRegion(v1, Entity, vec3_origin, 0.0) == 0;
    Scr_AddInt(v3);
}

void __cdecl ActorCmd_SetTalkToSpecies(scr_entref_t entref)
{
    actor_s *v1; // r26
    int v2; // r30
    int v3; // r27
    unsigned int ConstString; // r3
    char v5; // r10
    const unsigned __int16 **v6; // r11

    v1 = Actor_Get(entref);
    v2 = 0;
    v3 = 0;
    if (Scr_GetNumParam())
    {
        while (1)
        {
            ConstString = Scr_GetConstString(0);
            if (ConstString == scr_const.all)
                break;
            v5 = 0;
            v6 = g_AISpeciesNames;
            while (ConstString != **v6)
            {
                ++v6;
                ++v5;
                if ((uintptr_t)v6 >= (uintptr_t)g_AISpeciesNames[2])
                    goto LABEL_8;
            }
            v2 |= 1 << v5;
        LABEL_8:
            if (++v3 >= Scr_GetNumParam())
                goto LABEL_9;
        }
        v1->talkToSpecies = -1;
    }
    else
    {
    LABEL_9:
        v1->talkToSpecies = v2;
    }
}

void(__cdecl *__cdecl Actor_GetMethod(const char **pName))(scr_entref_t)
{
    int v1; // r6
    unsigned int v2; // r5
    const BuiltinMethodDef *i; // r7
    const char *actionString; // r10
    const char *v5; // r11
    int v6; // r8

    v1 = 0;
    v2 = 0;
    for (i = methods; ; ++i)
    {
        actionString = i->actionString;
        v5 = *pName;
        do
        {
            v6 = (unsigned __int8)*v5 - *(unsigned __int8 *)actionString;
            if (!*v5)
                break;
            ++v5;
            ++actionString;
        } while (!v6);
        if (!v6)
            break;
        v2 += 12;
        ++v1;
        if (v2 >= 0x4E0)
            return 0;
    }
    *pName = methods[v1].actionString;
    return methods[v1].actionFunc;
}

