#ifndef KISAK_SP 
#error This file is for SinglePlayer only 
#endif

#include "actor.h"
#include "actor_turret.h"
#include "actor_events.h"
#include "turret.h"
#include "g_local.h"
#include <script/scr_vm.h>
#include <script/scr_const.h>
#include "actor_threat.h"
#include "actor_state.h"
#include <cgame/cg_ents.h>
#include "g_main.h"
#include "actor_orientation.h"
#include "actor_senses.h"
#include "actor_grenade.h"

void __cdecl Actor_Turret_Touch(actor_s *self, gentity_s *pOther)
{
    sentient_s *sentient; // r4

    sentient = pOther->sentient;
    if (sentient)
        Actor_GetPerfectInfo(self, sentient);
}

int __cdecl Actor_IsUsingTurret(actor_s *self)
{
    gentity_s *pTurret; // r11
    gentity_s *v3; // r3
    unsigned __int8 v4; // r11

    pTurret = self->pTurret;
    if (!pTurret)
        return 0;
    if (!pTurret->r.ownerNum.isDefined())
        return 0;
    v3 = self->pTurret->r.ownerNum.ent();
    v4 = 1;
    if (v3 != self->ent)
        return 0;
    return v4;
}

int __cdecl Actor_UseTurret(actor_s *self, gentity_s *pTurret)
{
    int result; // r3

    result = turret_canuse(self, pTurret);
    if (result)
    {
        self->pTurret = pTurret;
        result = 1;
        pTurret->flags |= 0x4000000u;
    }
    else
    {
        self->pTurret = 0;
    }
    return result;
}

bool __cdecl Actor_Turret_Start(actor_s *self, ai_state_t ePrevState)
{
    gentity_s *pTurret; // r28
    TurretInfo *pTurretInfo; // r29
    double initialYawmax; // fp13
    unsigned int v7; // r11
    unsigned int v8; // r11

    if (!self)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\actor_turret.cpp", 27, 0, "%s", "self");
    if (!self->sentient)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\actor_turret.cpp", 28, 0, "%s", "self->sentient");
    pTurret = self->pTurret;
    if (!pTurret)
        return 0;
    if (!pTurret->r.inuse)
        MyAssertHandler(
            "c:\\trees\\cod3\\cod3src\\src\\game\\actor_turret.cpp",
            31,
            0,
            "%s",
            "!pTurret || pTurret->r.inuse");
    if (pTurret->active || !G_EntLinkTo(self->ent, pTurret, 0))
        return 0;
    if ((unsigned __int8)Actor_IsUsingTurret(self))
        MyAssertHandler(
            "c:\\trees\\cod3\\cod3src\\src\\game\\actor_turret.cpp",
            36,
            0,
            "%s",
            "!Actor_IsUsingTurret( self )");
    pTurret->active = 1;
    iassert(!pTurret->r.ownerNum.isDefined());
    pTurret->r.ownerNum.setEnt(self->ent);
    if (Scr_IsSystemActive())
        Scr_Notify(pTurret, scr_const.turretownerchange, 0);
    if (!(unsigned __int8)Actor_IsUsingTurret(self))
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\actor_turret.cpp", 46, 0, "%s", "Actor_IsUsingTurret( self )");
    if (!G_EntIsLinkedTo(self->ent, pTurret))
        MyAssertHandler(
            "c:\\trees\\cod3\\cod3src\\src\\game\\actor_turret.cpp",
            47,
            0,
            "%s",
            "G_EntIsLinkedTo( self->ent, pTurret )");
    turret_ClearTargetEnt(pTurret);
    pTurretInfo = pTurret->pTurretInfo;
    if (!pTurretInfo)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\actor_turret.cpp", 52, 0, "%s", "pTurretInfo");
    initialYawmax = pTurretInfo->initialYawmax;
    v7 = pTurretInfo->flags & 0xFFFFFFF7;
    pTurretInfo->arcmin[1] = pTurretInfo->initialYawmin;
    pTurretInfo->arcmax[1] = initialYawmax;
    v8 = v7 & 0xFFFFFE1F | 0x100;
    pTurretInfo->flags = v8;
    if ((v8 & 0x200) != 0)
        MyAssertHandler(
            "c:\\trees\\cod3\\cod3src\\src\\game\\actor_turret.cpp",
            59,
            0,
            "%s",
            "!(pTurretInfo->flags & TURRET_PITCH_CAP)");
    Actor_CanAttackAll(self);
    if (ePrevState != AIS_PAIN)
    {
        pTurretInfo->flags &= ~0x10u;
        pTurretInfo->detachSentient.setSentient(NULL);
    }
    Actor_ClearKeepClaimedNode(self);
    Sentient_ClaimNode(self->sentient, 0);
    Actor_ClearPath(self);
    return 1;
}

void __cdecl Actor_DetachTurret(actor_s *self)
{
    gentity_s *pTurret; // r11
    gentity_s *pTurret; // r28
    gentity_s *ent; // r3
    TurretInfo *pTurretInfo; // r31

    iassert(self);

    iassert(Actor_IsUsingTurret(self));

    pTurret = self->pTurret;

    iassert(pTurret);
    iassert(pTurret->r.inuse);
    iassert(pTurret->active);

    G_DeactivateTurret(pTurret);

    iassert(pTurret->r.ownerNum.isDefined() && (pTurret->r.ownerNum.ent() == self->ent));

    pTurret->r.ownerNum.setEnt(NULL);
    if (Scr_IsSystemActive())
        Scr_Notify(pTurret, scr_const.turretownerchange, 0);

    iassert(!Actor_IsUsingTurret(self));

    if (G_EntIsLinkedTo(self->ent, pTurret))
    {
        ent = self->ent;
        if (pTurret->tagInfo)
            G_EntLinkTo(ent, pTurret->tagInfo->parent, pTurret->tagInfo->name);
        else
            G_EntUnlink(ent);
    }
    pTurretInfo = pTurret->pTurretInfo;
    if (!pTurretInfo)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\actor_turret.cpp", 122, 0, "%s", "pTurretInfo");
    pTurretInfo->flags &= ~0x200u;
}

void __cdecl Actor_Turret_Finish(actor_s *self, ai_state_t eNextState)
{
    gentity_s *pTurret; // r11
    gentity_s *v5; // r3
    char v6; // r11
    sentient_s *sentient; // r3

    pTurret = self->pTurret;
    if (!pTurret
        || !pTurret->r.ownerNum.isDefined()
        || (v5 = self->pTurret->r.ownerNum.ent(), v6 = 1, v5 != self->ent))
    {
        v6 = 0;
    }
    if (v6)
    {
        Actor_DetachTurret(self);
        sentient = self->sentient;
        if (sentient->pClaimedNode)
            Path_RelinquishNodeNow(sentient);
        if (eNextState == AIS_DEATH)
            self->bDropWeapon = 0;
    }
}

void __cdecl Actor_Turret_Suspend(actor_s *self, ai_state_t eNextState)
{
    if (Actor_IsUsingTurret(self))
        Actor_DetachTurret(self);
}

void __cdecl Actor_StopUseTurret(actor_s *self)
{
    iassert(self);
    iassert(self->sentient);

    if (Actor_IsUsingTurret(self))
        Actor_DetachTurret(self);
    self->pTurret = 0;
}

actor_think_result_t __cdecl Actor_Turret_PostThink(actor_s *self)
{
    gentity_s *ent; // r16
    DObjAnimMat *LocalTagMatrix; // r31
    actor_think_result_t result; // r3
    unsigned int turretAnim; // r15
    WeaponDef *WeaponDef; // r19
    double v7; // fp25
    double v8; // fp26
    XAnimTree_s *ActorAnimTree; // r22
    const XAnim_s *Anims; // r31
    int NumChildren; // r27
    unsigned int v12; // r21
    unsigned int v13; // r26
    double v14; // fp28
    double v15; // fp27
    const char *AnimDebugName; // r3
    DObj_s *ServerDObj; // r20
    int v18; // r25
    unsigned int ChildAt; // r28
    int v20; // r30
    __int64 v21; // r11
    const char *v22; // r3
    double v23; // fp0
    int v24; // r30
    double v25; // fp30
    unsigned int v26; // r29
    double v27; // fp0
    int v28; // r7
    unsigned int v29; // r6
    unsigned int v30; // r5
    int v31; // r7
    unsigned int v32; // r6
    unsigned int v33; // r5
    const gentity_s *v34; // r24
    TurretInfo *pTurretInfo; // r30
    int flags; // r11
    double v37; // fp0
    double v38; // fp13
    double v39; // fp0
    double v40; // fp13
    double v41; // fp30
    unsigned int v42; // r29
    int v43; // r7
    unsigned int v44; // r6
    unsigned int v45; // r5
    unsigned int v46; // r3
    int v47; // r7
    unsigned int v48; // r6
    unsigned int v49; // r5
    unsigned int v50; // r3
    int v51; // r7
    unsigned int v52; // r6
    unsigned int v53; // r5
    float *v54; // r29
    double v55; // fp1
    float (*v56)[3]; // r3
    int v57; // r11
    int v58; // r7
    unsigned int v59; // r6
    unsigned int v60; // r5
    DObjAnimMat *v61; // r3
    DObjAnimMat *v62; // r31
    int v63; // r11
    long double v64; // fp2
    double v65; // fp28
    long double v66; // fp2
    double v67; // fp30
    int v68; // r11
    unsigned int v69; // r11
    const char *v70; // r11
    int iHitEntnum; // r10
    double v72; // fp0
    double v73; // fp0
    gentity_s *v74; // r31
    double v75; // fp4
    double v76; // fp0
    float v77; // [sp+50h] [-220h] BYREF
    float v78; // [sp+54h] [-21Ch]
    float v79; // [sp+58h] [-218h]
    DObjAnimMat *v80; // [sp+5Ch] [-214h]
    float v81; // [sp+60h] [-210h] BYREF
    float v82; // [sp+64h] [-20Ch]
    float v83; // [sp+68h] [-208h]
    scr_const_t *v84; // [sp+6Ch] [-204h]
    float v85; // [sp+70h] [-200h] BYREF
    float v86; // [sp+74h] [-1FCh]
    float v87; // [sp+78h] [-1F8h]
    float v88; // [sp+80h] [-1F0h] BYREF
    float v89; // [sp+84h] [-1ECh]
    float v90; // [sp+88h] [-1E8h] BYREF
    float v91; // [sp+8Ch] [-1E4h]
    int v92; // [sp+90h] [-1E0h]
    float v93[2]; // [sp+98h] [-1D8h] BYREF
    const gentity_s *pTurret; // [sp+A0h] [-1D0h]
    __int64 v95; // [sp+A8h] [-1C8h] BYREF
    float v96[2]; // [sp+B8h] [-1B8h] BYREF
    float v97; // [sp+C0h] [-1B0h]
    __int64 v98; // [sp+C8h] [-1A8h]
    __int64 v99; // [sp+D0h] [-1A0h]
    float v100[12]; // [sp+E0h] [-190h] BYREF
    float v101[12]; // [sp+110h] [-160h] BYREF
    float v102[12]; // [sp+140h] [-130h] BYREF
    float v103[8][3]; // [sp+170h] [-100h] BYREF

    if (!self)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\actor_turret.cpp", 214, 0, "%s", "self");
    pTurret = self->pTurret;
    if (!pTurret)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\actor_turret.cpp", 218, 0, "%s", "pTurret");
    if (!pTurret->r.inuse)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\actor_turret.cpp", 219, 0, "%s", "pTurret->r.inuse");
    ent = self->ent;
    if (!self->ent)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\actor_turret.cpp", 222, 0, "%s", "ent");
    v84 = &scr_const;
    LocalTagMatrix = G_DObjGetLocalTagMatrix(pTurret, scr_const.tag_weapon);
    v80 = LocalTagMatrix;
    if (!LocalTagMatrix)
    {
        Com_PrintWarning(18, "WARNING: aborting turret behavior since 'tag_weapon' does not exist\n");
        Actor_StopUseTurret(self);
        Actor_SetState(self, AIS_EXPOSED);
        return ACTOR_THINK_REPEAT;
    }
    if (self->eAnimMode == AI_ANIM_UNKNOWN)
        MyAssertHandler(
            "c:\\trees\\cod3\\cod3src\\src\\game\\actor_turret.cpp",
            233,
            0,
            "%s",
            "self->eAnimMode != AI_ANIM_UNKNOWN");
    if (!self->turretAnimSet)
    {
        Com_PrintWarning(18, "WARNING: aborting turret behavior since no turret animation specified\n");
        Actor_StopUseTurret(self);
        Actor_SetState(self, AIS_EXPOSED);
        return ACTOR_THINK_REPEAT;
    }
    turretAnim = self->turretAnim;
    if (!pTurret->s.weapon)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\actor_turret.cpp", 245, 0, "%s", "pTurret->s.weapon");
    WeaponDef = BG_GetWeaponDef(pTurret->s.weapon);
    if (WeaponDef->weapClass != WEAPCLASS_TURRET)
        MyAssertHandler(
            "c:\\trees\\cod3\\cod3src\\src\\game\\actor_turret.cpp",
            249,
            0,
            "%s",
            "weapDef->weapClass == WEAPCLASS_TURRET");
    LocalConvertQuatToMat(LocalTagMatrix, v103);
    v7 = vectosignedyaw(v103[0]);
    if (!ent->tagInfo)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\actor_turret.cpp", 254, 0, "%s", "ent->tagInfo");
    v8 = (float)(ent->tagInfo->axis[3][2] - LocalTagMatrix->trans[2]);
    ActorAnimTree = G_GetActorAnimTree(self);
    if (!ActorAnimTree)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\actor_turret.cpp", 258, 0, "%s", "tree");
    Anims = XAnimGetAnims(ActorAnimTree);
    NumChildren = XAnimGetNumChildren(Anims, turretAnim);
    v12 = 0;
    v13 = 0;
    v14 = 0.0;
    v15 = 0.0;
    if (!NumChildren)
    {
        AnimDebugName = XAnimGetAnimDebugName(Anims, turretAnim);
        Com_Error(ERR_DROP, "anim '%s' has no children", AnimDebugName);
    }
    ServerDObj = Com_GetServerDObj(ent->s.number);
    v18 = 0;
    do
    {
        ChildAt = XAnimGetChildAt(Anims, turretAnim, NumChildren - v18 - 1);
        v20 = XAnimGetNumChildren(Anims, ChildAt);
        if (!v20)
        {
            v22 = XAnimGetAnimDebugName(Anims, ChildAt);
            Com_Error(ERR_DROP, "anim '%s' has no children", v22);
        }
        if (WeaponDef->fAnimHorRotateInc == 0.0)
            MyAssertHandler(
                "c:\\trees\\cod3\\cod3src\\src\\game\\actor_turret.cpp",
                281,
                0,
                "%s",
                "weapDef->fAnimHorRotateInc");
        LODWORD(v21) = v20;
        v23 = (float)((float)((float)v21 * (float)0.5) + (float)((float)v7 / WeaponDef->fAnimHorRotateInc));
        v99 = v21;
        if (v23 >= 0.0)
        {
            LODWORD(v21) = v20 - 1;
            v98 = v21;
            if (v23 >= (float)v21)
                v23 = (float)v21;
        }
        else
        {
            v23 = 0.0;
        }
        v92 = (int)v23;
        v24 = (int)v23;
        LODWORD(v21) = (int)v23;
        v95 = v21;
        v25 = (float)((float)v23 - (float)v21);
        v26 = XAnimGetChildAt(Anims, ChildAt, (int)v23);
        XAnimGetAbsDelta(Anims, v26, v93, &v77, 0.0);
        if (v25 == 0.0)
        {
            v27 = v79;
        }
        else
        {
            v13 = XAnimGetChildAt(Anims, ChildAt, v24 + 1);
            XAnimGetAbsDelta(Anims, v13, v93, v96, 0.0);
            v77 = (float)((float)(v96[0] - v77) * (float)v25) + v77;
            v78 = (float)((float)(v96[1] - v78) * (float)v25) + v78;
            v27 = (float)((float)((float)(v97 - v79) * (float)v25) + v79);
            v79 = (float)((float)(v97 - v79) * (float)v25) + v79;
        }
        if (v27 >= v8)
            break;
        ++v18;
        v14 = v27;
        v12 = v24;
        v15 = v25;
    } while (v18 < NumChildren);
    XAnimClearTreeGoalWeightsStrict(ActorAnimTree, turretAnim, 0.0);
    XAnimSetGoalWeight(ServerDObj, v26, (float)((float)1.0 - (float)v25), 0.0, 1.0, v30, v29, v28);
    if (v25 != 0.0)
        XAnimSetGoalWeight(ServerDObj, v13, v25, 0.0, 1.0, v33, v32, v31);
    v34 = pTurret;
    pTurretInfo = pTurret->pTurretInfo;
    if (!pTurretInfo)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\actor_turret.cpp", 320, 0, "%s", "pTurretInfo");
    if (v18 && v18 != NumChildren)
    {
        flags = pTurretInfo->flags;
        if ((flags & 0x20) == 0
            || pTurretInfo->originError[0] != 0.0
            || pTurretInfo->originError[1] != 0.0
            || pTurretInfo->originError[2] != 0.0)
        {
            goto LABEL_54;
        }
        if ((flags & 0x200) != 0)
        {
            if ((flags & 0x400) != 0)
            {
                v37 = (float)(pTurretInfo->pitchCap - (float)0.1);
                v38 = pTurretInfo->arcmin[0];
                pTurretInfo->pitchCap = pTurretInfo->pitchCap - (float)0.1;
                if (v37 <= v38)
                    goto LABEL_54;
            }
            else
            {
                v39 = (float)(pTurretInfo->pitchCap + (float)0.1);
                v40 = pTurretInfo->arcmax[0];
                pTurretInfo->pitchCap = pTurretInfo->pitchCap + (float)0.1;
                if (v39 >= v40)
                    LABEL_54:
                pTurretInfo->flags = flags & 0xFFFFFDFF;
            }
        }
        if ((float)(v79 - (float)v14) == 0.0)
            MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\actor_turret.cpp", 406, 0, "%s", "trans[2] - fPrevTransZ");
        v41 = (float)((float)((float)v8 - (float)v14) / (float)(v79 - (float)v14));
        XAnimSetGoalWeight(ServerDObj, ChildAt, v41, 0.0, 1.0, v33, v32, v31);
        v42 = XAnimGetChildAt(Anims, turretAnim, NumChildren - v18);
        XAnimSetGoalWeight(ServerDObj, v42, (float)((float)1.0 - (float)v41), 0.0, 1.0, v45, v44, v43);
        v46 = XAnimGetChildAt(Anims, v42, v12);
        XAnimSetGoalWeight(ServerDObj, v46, (float)((float)1.0 - (float)v15), 0.0, 1.0, v49, v48, v47);
        if (v15 != 0.0)
        {
            v50 = XAnimGetChildAt(Anims, v42, v12 + 1);
            XAnimSetGoalWeight(ServerDObj, v50, v15, 0.0, 1.0, v53, v52, v51);
        }
        v54 = (float *)v80;
        goto LABEL_60;
    }
    v61 = G_DObjGetLocalTagMatrix(v34, v84->tag_aim);
    v62 = v61;
    if (!v61)
    {
        Com_PrintWarning(18, "WARNING: aborting turret behavior since 'tag_aim' does not exist\n");
        Actor_StopUseTurret(self);
        Actor_SetState(self, AIS_EXPOSED);
        return ACTOR_THINK_REPEAT;
    }
    v63 = pTurretInfo->flags;
    if ((v63 & 0x20) != 0
        && pTurretInfo->originError[0] == 0.0
        && pTurretInfo->originError[1] == 0.0
        && pTurretInfo->originError[2] == 0.0)
    {
        v54 = (float *)v80;
        v90 = Vec2Distance(v80->trans, v61->trans);
        v91 = v54[6] - v62->trans[2];
        v88 = v90;
        v89 = (float)(ent->tagInfo->axis[3][2] - v79) - v62->trans[2];
        Vec2Normalize(&v90);
        Vec2Normalize(&v88);
        *(double *)&v64 = (float)((float)(v89 * v91) + (float)(v88 * v90));
        if (*(double *)&v64 >= 0.0)
        {
            if (*(double *)&v64 <= 1.0)
            {
                v66 = acos(v64);
                v65 = (float)((float)*(double *)&v66 * (float)57.295776);
            }
            else
            {
                v65 = 0.0;
            }
        }
        else
        {
            v65 = 90.0;
        }
        UnitQuatToAngles(v62->quat, (float *)&v95);
        v67 = *(float *)&v95;
        if (*(float *)&v95 > 180.0)
            v67 = (float)(*(float *)&v95 - (float)360.0);
        if (v65 < 0.0)
            MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\actor_turret.cpp", 360, 0, "%s", "animPitchError >= 0");
        v68 = pTurretInfo->flags;
        if (v18)
        {
            v69 = v68 | 0x600;
            pTurretInfo->pitchCap = (float)v67 + (float)v65;
        }
        else
        {
            v69 = v68 & 0xFFFFF9FF | 0x200;
            pTurretInfo->pitchCap = (float)v67 - (float)v65;
        }
    }
    else
    {
        v54 = (float *)v80;
        v69 = v63 & 0xFFFFFDFF;
    }
    pTurretInfo->flags = v69;
    if ((v69 & 2) != 0)
        v70 = "auto_turret EXCEEDED ANIM PITCH";
    else
        v70 = "manual_turret EXCEEDED ANIM PITCH";
    self->pszDebugInfo = v70;
    XAnimSetGoalWeight(ServerDObj, ChildAt, 1.0, 0.0, 1.0, v60, v59, v58);
LABEL_60:
    XAnimCalcAbsDelta(ServerDObj, turretAnim, v93, &v77);
    VectorAngleMultiply(&v77, v7);
    v100[9] = v54[4] + v77;
    v100[10] = v54[5] + v78;
    v100[11] = v54[6] + v79;
    v55 = RotationToYaw(v93);
    YawToAxis((float)((float)v55 + (float)v7), (mat3x3&)v56);
    AnglesToAxis(v34->r.currentAngles, (float (*)[3])v101);
    v101[9] = v34->r.currentOrigin[0];
    v101[10] = v34->r.currentOrigin[1];
    v101[11] = v34->r.currentOrigin[2];
    MatrixMultiply43((const mat4x3&)v100, (const mat4x3&)v101, (mat4x3&)v102);
    AxisToAngles((const mat3x3&)v102, &v85);
    v81 = v102[9];
    v82 = v102[10];
    v83 = v102[11];
    v57 = pTurretInfo->flags;
    if ((v57 & 0x20) == 0)
    {
        pTurretInfo->flags = v57 | 0x20;
        if (level.loading == LOADING_LEVEL)
        {
            v81 = ent->r.currentOrigin[0];
            v82 = ent->r.currentOrigin[1];
            v83 = ent->r.currentOrigin[2];
            v85 = ent->r.currentAngles[0];
            v86 = ent->r.currentAngles[1];
            v87 = ent->r.currentAngles[2];
            pTurretInfo->originError[0] = 0.0;
            pTurretInfo->originError[1] = 0.0;
            pTurretInfo->originError[2] = 0.0;
            pTurretInfo->anglesError[0] = 0.0;
            pTurretInfo->anglesError[1] = 0.0;
            pTurretInfo->anglesError[2] = 0.0;
        }
        else
        {
            pTurretInfo->originError[0] = ent->r.currentOrigin[0] - v81;
            pTurretInfo->originError[1] = ent->r.currentOrigin[1] - v82;
            pTurretInfo->originError[2] = ent->r.currentOrigin[2] - v83;
            AnglesSubtract(ent->r.currentAngles, &v85, pTurretInfo->anglesError);
        }
    }
    G_ReduceOriginError(&v81, pTurretInfo->originError, 3.0);
    G_ReduceAnglesError(&v85, pTurretInfo->anglesError, 27.0);
    ent->r.currentAngles[0] = v85;
    ent->r.currentAngles[1] = v86;
    ent->r.currentAngles[2] = v87;
    Actor_SetDesiredAngles(&self->CodeOrient, ent->r.currentAngles[0], ent->r.currentAngles[1]);
    Actor_SetLookAngles(self, ent->r.currentAngles[0], ent->r.currentAngles[1]);
    if (v34->tagInfo)
    {
        ent->r.currentOrigin[0] = v81;
        ent->r.currentOrigin[1] = v82;
        ent->r.currentOrigin[2] = v83;
        G_CalcTagAxis(ent, 1);
        goto success;
    }
    self->Physics.ePhysicsType = AIPHYS_NORMAL_ABSOLUTE;
    self->Physics.vWishDelta[0] = v81 - ent->r.currentOrigin[0];
    self->Physics.vWishDelta[1] = v82 - ent->r.currentOrigin[1];
    self->Physics.vWishDelta[2] = 0.0;
    Actor_DoMove(self);
    G_TouchEnts(ent, self->Physics.iNumTouch, self->Physics.iTouchEnts);
    G_CalcTagAxis(ent, 0);
    iHitEntnum = self->Physics.iHitEntnum;
    if (iHitEntnum == 2175)
        goto success;
    if (level.time - self->iStateTime < 1000)
        goto success;
    v72 = v34->s.lerp.u.turret.gunAngles[0];
    if (v72 > pTurretInfo->arcmax[0])
        goto success;
    if (v72 < pTurretInfo->arcmin[0])
        goto success;
    v73 = v34->s.lerp.u.turret.gunAngles[1];
    if (v73 > pTurretInfo->arcmax[1]
        || v73 < pTurretInfo->arcmin[1]
        || pTurretInfo->originError[0] != 0.0
        || pTurretInfo->originError[1] != 0.0
        || pTurretInfo->originError[2] != 0.0
        || pTurretInfo->anglesError[0] != 0.0
        || pTurretInfo->anglesError[1] != 0.0
        || pTurretInfo->anglesError[2] != 0.0)
    {
        goto success;
    }
    v74 = &level.gentities[iHitEntnum];
    if (!v74->sentient)
    {
        if (v73 < 0.0)
        {
            v76 = (float)(pTurretInfo->arcmin[1] + (float)1.0);
            pTurretInfo->arcmin[1] = pTurretInfo->arcmin[1] + (float)1.0;
            if (v76 <= 0.0)
            {
                Com_PrintWarning(
                    18,
                    "WARNING: capping rightarc of turret at (%.2f, %.2f, %.2f) to %.2f\n",
                    v34->r.currentOrigin[0],
                    v34->r.currentOrigin[1],
                    v34->r.currentOrigin[2],
                    -v76);
                goto success;
            }
            pTurretInfo->arcmin[1] = 0.0;
        }
        else
        {
            v75 = (float)(pTurretInfo->arcmax[1] - (float)1.0);
            pTurretInfo->arcmax[1] = pTurretInfo->arcmax[1] - (float)1.0;
            if (v75 >= 0.0)
            {
                Com_PrintWarning(
                    18,
                    "WARNING: capping leftarc of turret at (%.2f, %.2f, %.2f) to %.2f\n",
                    v34->r.currentOrigin[0],
                    v34->r.currentOrigin[1],
                    v34->r.currentOrigin[2],
                    v75);
            success:
                self->Physics.vVelocity[0] = 0.0;
                result = ACTOR_THINK_DONE;
                self->Physics.vVelocity[1] = 0.0;
                self->Physics.vVelocity[2] = 0.0;
                self->Physics.vWishDelta[0] = 0.0;
                self->Physics.vWishDelta[1] = 0.0;
                self->Physics.vWishDelta[2] = 0.0;
                return result;
            }
            pTurretInfo->arcmax[1] = 0.0;
        }
        Com_PrintWarning(
            18,
            "WARNING: AI %d at (%.2f, %.2f, %.2f) with turret angles (%.2f, %.2f) detaching from turret due to obstruction\n",
            0, // KISAKTODO: number
            self->ent->r.currentOrigin[0],
            self->ent->r.currentOrigin[1],
            self->ent->r.currentOrigin[2],
            v34->s.lerp.u.turret.gunAngles[0],
            v34->s.lerp.u.turret.gunAngles[2]);
    }
    Actor_StopUseTurret(self);
    if (v74->client)
    {
        if (((1 << v74->sentient->eTeam) & ~(1 << Sentient_EnemyTeam(self->ent->sentient->eTeam))) != 0)
        {
            Scr_AddEntity(v74);
            Scr_Notify(self->ent, v84->trigger, 1u);
        }
    }
    Actor_SetState(self, AIS_EXPOSED);
    return ACTOR_THINK_REPEAT;
}

actor_think_result_t __cdecl Actor_Turret_Think(actor_s *self)
{
    gentity_s *v3; // r3
    char isUsingTurret; // r11
    gentity_s *pTurret; // r29
    TurretInfo *pTurretInfo; // r28
    actor_s *v8; // r3
    unsigned int state; // r4
    const char *v10; // r3
    const char *v11; // r11
    unsigned int weapon; // r3
    scr_animscript_t *v13; // r29
    WeaponDef *WeaponDef; // r3
    const pathnode_t *v15; // r3
    pathnode_t *v16; // r30
    actor_think_result_t v17; // r31

    if (!self)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\actor_turret.cpp", 555, 0, "%s", "self");
    if (!self->sentient)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\actor_turret.cpp", 556, 0, "%s", "self->sentient");
    Actor_ClearPath(self);
    Actor_ClearPileUp(self);

    isUsingTurret = Actor_IsUsingTurret(self);

    if (isUsingTurret)
    {
        pTurret = self->pTurret;
        iassert(pTurret);
        iassert(pTurret->r.inuse);
        pTurretInfo = pTurret->pTurretInfo;
        iassert(pTurretInfo);
        if (!(unsigned __int8)Actor_KnowAboutEnemy(self, 0))
            self->useEnemyGoal = 0;
        if (!(unsigned __int8)Actor_KeepClaimedNode(self))
        {
            Actor_UpdateDesiredChainPos(self);
            Actor_UpdateGoalPos(self);
        }
        if (((pTurretInfo->flags & 0x2000) != 0 || Actor_PointNearGoal(pTurret->r.currentOrigin, &self->codeGoal, 92.0))
            && G_EntIsLinkedTo(self->ent, pTurret)
            && (pTurretInfo->flags & 0x80) == 0)
        {
            if (self->pGrenade.isDefined() && !pTurret->tagInfo)
            {
                if (!Actor_Grenade_IsPointSafe(self, self->ent->r.currentOrigin))
                {
                    Actor_StopUseTurret(self);
                    Actor_SetState(self, AIS_GRENADE_RESPONSE);
                    return ACTOR_THINK_REPEAT;
                }
                self->pGrenade.setEnt(NULL);
            }
            v8 = self;
            if (self->flashBanged)
                goto LABEL_53;
            Actor_PreThink(self);
            state = pTurretInfo->state;
            if (state)
            {
                if (state == 1)
                {
                    if ((pTurretInfo->flags & 2) != 0)
                        v11 = "auto_turret_firing_head";
                    else
                        v11 = "manual_turret_firing_head";
                }
                else
                {
                    if (state >= 3)
                    {
                        if (!alwaysfails)
                        {
                            v10 = va("unhandled case %i", state);
                            MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\actor_turret.cpp", 639, 0, v10);
                        }
                    LABEL_45:
                        weapon = self->pTurret->s.weapon;
                        v13 = (scr_animscript_t *)((char *)g_scr_data.anim.weapons + __ROL4__(weapon, 3));
                        if (!v13->func)
                        {
                            WeaponDef = BG_GetWeaponDef(weapon);
                            Com_Error(ERR_DROP, "no script specified for weapon info '%s' being used by AI", WeaponDef->szInternalName);
                        }
                        v15 = Sentient_NearestNode(self->sentient);
                        v16 = (pathnode_t *)v15;
                        if (v15)
                        {
                            if (Path_CanClaimNode(v15, self->sentient))
                                Path_ForceClaimNode(v16, self->sentient);
                        }
                        Actor_SetAnimScript(self, v13, 0, AI_ANIM_MOVE_CODE);
                        if ((unsigned __int8)Actor_IsUsingTurret(self))
                        {
                            self->bUseGoalWeight = 0;
                            //Profile_Begin(235);
                            v17 = Actor_Turret_PostThink(self);
                            //Profile_EndInternal(0);
                            return v17;
                        }
                        goto LABEL_52;
                    }
                    if ((pTurretInfo->flags & 2) != 0)
                        v11 = "auto_turret_firing_feet";
                    else
                        v11 = "manual_turret_firing_feet";
                }
            }
            else if ((pTurretInfo->flags & 2) != 0)
            {
                v11 = "auto_turret_idle";
            }
            else
            {
                v11 = "manual_turret_idle";
            }
            self->pszDebugInfo = v11;
            goto LABEL_45;
        }
    }
LABEL_52:
    v8 = self;
LABEL_53:
    Actor_StopUseTurret(v8);
    Actor_SetState(self, AIS_EXPOSED);
    return ACTOR_THINK_REPEAT;
}

void __cdecl Actor_Turret_Pain(
    actor_s *self,
    gentity_s *pAttacker,
    int iDamage,
    const float *vPoint,
    const int iMod,
    const float *vDir,
    const hitLocation_t hitLoc)
{
    TurretInfo *pTurretInfo; // r11
    int flags; // r10

    if (pAttacker->sentient
        && (unsigned __int8)Actor_IsUsingTurret(self)
        && pAttacker->sentient->eTeam == Sentient_EnemyTeam(self->sentient->eTeam))
    {
        if (!self->pTurret)
            MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\actor_turret.cpp", 708, 0, "%s", "self->pTurret");
        pTurretInfo = self->pTurret->pTurretInfo;
        flags = pTurretInfo->flags;
        if ((flags & 0x10) != 0)
        {
            Actor_StopUseTurret(self);
        }
        else
        {
            pTurretInfo->flags = flags | 0x10;
            pTurretInfo->detachSentient.setSentient(pAttacker->sentient);
        }
    }
}

