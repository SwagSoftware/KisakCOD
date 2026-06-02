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
        pTurret->flags |= FL_ACTOR_TURRET;
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

// (aislop)
// Drives an AI manning a turret. Each think:
//   1. Pick the pitch/yaw turret-animation pair whose absolute delta brings the gun
//      to the height the AI wants to aim at, and blend them on the server DObj.
//   2. If the desired pitch is outside the animated range, push the pitch cap out and
//      flag the turret as having exceeded its anim pitch.
//   3. Compose the resulting gun transform with the turret's world transform to get the
//      AI's origin/angles, easing in via an origin/angle error that decays over time.
//   4. For free-standing turrets, move the AI to that origin; if it gets wedged against a
//      non-living obstruction, tighten the firing arc or detach from the turret.
//
actor_think_result_t __cdecl Actor_Turret_PostThink(actor_s *self)
{
    iassert(self);

    const gentity_s *pTurret = self->pTurret;
    iassert(pTurret);
    iassert(pTurret->r.inuse);

    gentity_s *ent = self->ent;
    iassert(ent);

    DObjAnimMat *tagWeaponMtx = G_DObjGetLocalTagMatrix(pTurret, scr_const.tag_weapon);
    if (!tagWeaponMtx)
    {
        Com_PrintWarning(18, "WARNING: aborting turret behavior since 'tag_weapon' does not exist\n");
        Actor_StopUseTurret(self);
        Actor_SetState(self, AIS_EXPOSED);
        return ACTOR_THINK_REPEAT;
    }

    iassert(self->eAnimMode != AI_ANIM_UNKNOWN);
    if (!self->turretAnimSet)
    {
        Com_PrintWarning(18, "WARNING: aborting turret behavior since no turret animation specified\n");
        Actor_StopUseTurret(self);
        Actor_SetState(self, AIS_EXPOSED);
        return ACTOR_THINK_REPEAT;
    }

    unsigned int turretAnim = self->turretAnim;
    iassert(pTurret->s.weapon);
    WeaponDef *weapDef = BG_GetWeaponDef(pTurret->s.weapon);
    iassert(weapDef->weapClass == WEAPCLASS_TURRET);

    // Current turret yaw, and the gun height the AI needs to reach.
    mat3x3 tagWeaponAxis;
    LocalConvertQuatToMat(tagWeaponMtx, tagWeaponAxis);
    float fTurretYaw = vectosignedyaw(tagWeaponAxis[0]);

    iassert(ent->tagInfo);
    float fDesiredZ = ent->tagInfo->axis[3][2] - tagWeaponMtx->trans[2];

    XAnimTree_s *animTree = G_GetActorAnimTree(self);
    iassert(animTree);
    const XAnim_s *anims = XAnimGetAnims(animTree);

    int numPitchAnims = XAnimGetNumChildren(anims, turretAnim);
    if (!numPitchAnims)
        Com_Error(ERR_DROP, "anim '%s' has no children", XAnimGetAnimDebugName(anims, turretAnim));

    DObj_s *serverDObj = Com_GetServerDObj(ent->s.number);

    // Walk the pitch animations from the top down until one reaches fDesiredZ. The
    // bracketing pair is (this pitch level, the one above it); each level's yaw pair is
    // (yawAnim0, yawAnim1) blended by fYawFrac.
    unsigned int pitchAnim    = 0;
    unsigned int yawAnim0     = 0;
    unsigned int yawAnim1     = 0;
    float        fYawFrac     = 0.0f;
    int          iPrevYawIdx  = 0;
    float        fPrevYawFrac = 0.0f;
    float        fPrevZ       = 0.0f;
    float        rot[2];
    float        trans[3];

    int iPitchStep = 0;
    do
    {
        pitchAnim = XAnimGetChildAt(anims, turretAnim, numPitchAnims - iPitchStep - 1);

        int numYawAnims = XAnimGetNumChildren(anims, pitchAnim);
        if (!numYawAnims)
            Com_Error(ERR_DROP, "anim '%s' has no children", XAnimGetAnimDebugName(anims, pitchAnim));

        iassert(weapDef->fAnimHorRotateInc != 0.0f);

        // Map the turret yaw onto a fractional yaw-animation index, clamped to the set.
        float fYawIdx = (float)numYawAnims * 0.5f + fTurretYaw / weapDef->fAnimHorRotateInc;
        if (fYawIdx < 0.0f)
            fYawIdx = 0.0f;
        else if (fYawIdx >= (float)(numYawAnims - 1))
            fYawIdx = (float)(numYawAnims - 1);

        int iYawIdx = (int)fYawIdx;
        fYawFrac = fYawIdx - (float)iYawIdx;

        yawAnim0 = XAnimGetChildAt(anims, pitchAnim, iYawIdx);
        XAnimGetAbsDelta(anims, yawAnim0, rot, trans, 0.0f);

        if (fYawFrac != 0.0f)
        {
            yawAnim1 = XAnimGetChildAt(anims, pitchAnim, iYawIdx + 1);
            float tmpTrans[3];
            XAnimGetAbsDelta(anims, yawAnim1, rot, tmpTrans, 0.0f);
            trans[0] += (tmpTrans[0] - trans[0]) * fYawFrac;
            trans[1] += (tmpTrans[1] - trans[1]) * fYawFrac;
            trans[2] += (tmpTrans[2] - trans[2]) * fYawFrac;
        }

        if (trans[2] >= fDesiredZ)
            break;

        ++iPitchStep;
        fPrevZ       = trans[2];
        iPrevYawIdx  = iYawIdx;
        fPrevYawFrac = fYawFrac;
    } while (iPitchStep < numPitchAnims);

    // Apply the yaw blend for the level we stopped on.
    XAnimClearTreeGoalWeightsStrict(animTree, turretAnim, 0.0f);
    XAnimSetGoalWeight(serverDObj, yawAnim0, 1.0f - fYawFrac, 0.0f, 1.0f, 0, 0, 0);
    if (fYawFrac != 0.0f)
        XAnimSetGoalWeight(serverDObj, yawAnim1, fYawFrac, 0.0f, 1.0f, 0, 0, 0);

    TurretInfo *pTurretInfo = pTurret->pTurretInfo;
    iassert(pTurretInfo);

    if (iPitchStep != 0 && iPitchStep != numPitchAnims)
    {
        // Desired pitch lies between two animated levels: blend them.
        int flags = pTurretInfo->flags;
        bool clearPitchCap;
        if ((flags & 0x20) == 0
            || pTurretInfo->originError[0] != 0.0f
            || pTurretInfo->originError[1] != 0.0f
            || pTurretInfo->originError[2] != 0.0f)
        {
            clearPitchCap = true;
        }
        else if ((flags & 0x200) != 0)
        {
            // Ease the active pitch cap toward its arc limit; clear it once reached.
            if ((flags & 0x400) != 0)
            {
                pTurretInfo->pitchCap -= 0.1f;
                clearPitchCap = (pTurretInfo->pitchCap <= pTurretInfo->arcmin[0]);
            }
            else
            {
                pTurretInfo->pitchCap += 0.1f;
                clearPitchCap = (pTurretInfo->pitchCap >= pTurretInfo->arcmax[0]);
            }
        }
        else
        {
            clearPitchCap = false;
        }
        if (clearPitchCap)
            pTurretInfo->flags = flags & ~0x200;

        iassert((trans[2] - fPrevZ) != 0.0f);
        float fPitchFrac = (fDesiredZ - fPrevZ) / (trans[2] - fPrevZ);
        XAnimSetGoalWeight(serverDObj, pitchAnim, fPitchFrac, 0.0f, 1.0f, 0, 0, 0);

        unsigned int prevPitchAnim = XAnimGetChildAt(anims, turretAnim, numPitchAnims - iPitchStep);
        XAnimSetGoalWeight(serverDObj, prevPitchAnim, 1.0f - fPitchFrac, 0.0f, 1.0f, 0, 0, 0);

        unsigned int prevYawAnim0 = XAnimGetChildAt(anims, prevPitchAnim, iPrevYawIdx);
        XAnimSetGoalWeight(serverDObj, prevYawAnim0, 1.0f - fPrevYawFrac, 0.0f, 1.0f, 0, 0, 0);
        if (fPrevYawFrac != 0.0f)
        {
            unsigned int prevYawAnim1 = XAnimGetChildAt(anims, prevPitchAnim, iPrevYawIdx + 1);
            XAnimSetGoalWeight(serverDObj, prevYawAnim1, fPrevYawFrac, 0.0f, 1.0f, 0, 0, 0);
        }
    }
    else
    {
        // Desired pitch is past the top (iPitchStep == 0) or bottom (== numPitchAnims) of
        // the animated range: measure the overshoot and push the pitch cap out by it.
        DObjAnimMat *tagAimMtx = G_DObjGetLocalTagMatrix(pTurret, scr_const.tag_aim);
        if (!tagAimMtx)
        {
            Com_PrintWarning(18, "WARNING: aborting turret behavior since 'tag_aim' does not exist\n");
            Actor_StopUseTurret(self);
            Actor_SetState(self, AIS_EXPOSED);
            return ACTOR_THINK_REPEAT;
        }

        int flags = pTurretInfo->flags;
        if ((flags & 0x20) != 0
            && pTurretInfo->originError[0] == 0.0f
            && pTurretInfo->originError[1] == 0.0f
            && pTurretInfo->originError[2] == 0.0f)
        {
            // Angle between the weapon->aim direction and the weapon->target direction.
            float dirTurret[2];
            dirTurret[0] = Vec2Distance(tagWeaponMtx->trans, tagAimMtx->trans);
            dirTurret[1] = tagWeaponMtx->trans[2] - tagAimMtx->trans[2];

            float dirEnt[2];
            dirEnt[0] = dirTurret[0];
            dirEnt[1] = (ent->tagInfo->axis[3][2] - trans[2]) - tagAimMtx->trans[2];

            Vec2Normalize(dirTurret);
            Vec2Normalize(dirEnt);

            float cosPitchError = dirEnt[1] * dirTurret[1] + dirEnt[0] * dirTurret[0];
            float fAnimPitchError;
            if (cosPitchError < 0.0f)
                fAnimPitchError = 90.0f;
            else if (cosPitchError > 1.0f)
                fAnimPitchError = 0.0f;
            else
                fAnimPitchError = (float)(acos(cosPitchError) * 57.295776);

            float aimAngles[3];
            UnitQuatToAngles(tagAimMtx->quat, aimAngles);
            float fAimPitch = aimAngles[0];
            if (fAimPitch > 180.0f)
                fAimPitch -= 360.0f;

            iassert(fAnimPitchError >= 0.0f);

            if (iPitchStep)   // overshot the bottom: cap rises above the target
            {
                pTurretInfo->flags    = flags | 0x600;
                pTurretInfo->pitchCap = fAimPitch + fAnimPitchError;
            }
            else              // overshot the top: cap drops below the target
            {
                pTurretInfo->flags    = (flags & ~0x600) | 0x200;
                pTurretInfo->pitchCap = fAimPitch - fAnimPitchError;
            }
        }
        else
        {
            pTurretInfo->flags = flags & ~0x200;
        }

        self->pszDebugInfo = (pTurretInfo->flags & 2) ? "auto_turret EXCEEDED ANIM PITCH"
                                                      : "manual_turret EXCEEDED ANIM PITCH";
        XAnimSetGoalWeight(serverDObj, pitchAnim, 1.0f, 0.0f, 1.0f, 0, 0, 0);
    }

    // --- Resolve the AI's world origin/angles from the blended turret delta ---
    XAnimCalcAbsDelta(serverDObj, turretAnim, rot, trans);
    VectorAngleMultiply(trans, fTurretYaw);

    // animMat: the gun-delta transform expressed in the turret's local frame.
    float animMat[4][3];
    animMat[3][0] = tagWeaponMtx->trans[0] + trans[0];
    animMat[3][1] = tagWeaponMtx->trans[1] + trans[1];
    animMat[3][2] = tagWeaponMtx->trans[2] + trans[2];
    // NOTE: the IDA writes the rotation into this matrix; the prior port wrote it through
    // an uninitialized pointer (v56). Target animMat's 3x3 rotation.
    YawToAxis(RotationToYaw(rot) + fTurretYaw, (mat3x3&)animMat);

    // turretMat: the turret's world transform.
    float turretMat[4][3];
    AnglesToAxis(pTurret->r.currentAngles, (float (*)[3])turretMat);
    turretMat[3][0] = pTurret->r.currentOrigin[0];
    turretMat[3][1] = pTurret->r.currentOrigin[1];
    turretMat[3][2] = pTurret->r.currentOrigin[2];

    float worldMat[4][3];
    MatrixMultiply43((const mat4x3&)animMat, (const mat4x3&)turretMat, (mat4x3&)worldMat);

    float newAngles[3];
    AxisToAngles((const mat3x3&)worldMat, newAngles);

    float newOrigin[3];
    newOrigin[0] = worldMat[3][0];
    newOrigin[1] = worldMat[3][1];
    newOrigin[2] = worldMat[3][2];

    // On the first think after mounting, seed the origin/angle error so the AI eases into
    // the computed pose instead of snapping (unless the level is still loading).
    if ((pTurretInfo->flags & 0x20) == 0)
    {
        pTurretInfo->flags |= 0x20;
        if (level.loading == LOADING_LEVEL)
        {
            Vec3Copy(ent->r.currentOrigin, newOrigin);
            Vec3Copy(ent->r.currentAngles, newAngles);
            Vec3Clear(pTurretInfo->originError);
            Vec3Clear(pTurretInfo->anglesError);
        }
        else
        {
            pTurretInfo->originError[0] = ent->r.currentOrigin[0] - newOrigin[0];
            pTurretInfo->originError[1] = ent->r.currentOrigin[1] - newOrigin[1];
            pTurretInfo->originError[2] = ent->r.currentOrigin[2] - newOrigin[2];
            AnglesSubtract(ent->r.currentAngles, newAngles, pTurretInfo->anglesError);
        }
    }

    G_ReduceOriginError(newOrigin, pTurretInfo->originError, 3.0);
    G_ReduceAnglesError(newAngles, pTurretInfo->anglesError, 27.0);

    Vec3Copy(newAngles, ent->r.currentAngles);

    Actor_SetDesiredAngles(&self->CodeOrient, ent->r.currentAngles[0], ent->r.currentAngles[1]);
    Actor_SetLookAngles(self, ent->r.currentAngles[0], ent->r.currentAngles[1]);

    if (pTurret->tagInfo)
    {
        // Turret is mounted on another entity: just place the AI, no physics move.
        Vec3Copy(newOrigin, ent->r.currentOrigin);
        G_CalcTagAxis(ent, 1);
        Vec3Clear(self->Physics.vVelocity);
        Vec3Clear(self->Physics.vWishDelta);
        return ACTOR_THINK_DONE;
    }

    // Free-standing turret: physically move the AI toward the computed origin.
    self->Physics.ePhysicsType = AIPHYS_NORMAL_ABSOLUTE;
    self->Physics.vWishDelta[0] = newOrigin[0] - ent->r.currentOrigin[0];
    self->Physics.vWishDelta[1] = newOrigin[1] - ent->r.currentOrigin[1];
    self->Physics.vWishDelta[2] = 0.0f;
    Actor_DoMove(self);
    G_TouchEnts(ent, self->Physics.iNumTouch, self->Physics.iTouchEnts);
    G_CalcTagAxis(ent, 0);

    int iHitEntnum = self->Physics.iHitEntnum;
    if (iHitEntnum == ENTITYNUM_NONE)
    {
        Vec3Clear(self->Physics.vVelocity);
        Vec3Clear(self->Physics.vWishDelta);
        return ACTOR_THINK_DONE;
    }
    if (level.time - self->iStateTime < 1000)
    {
        Vec3Clear(self->Physics.vVelocity);
        Vec3Clear(self->Physics.vWishDelta);
        return ACTOR_THINK_DONE;
    }

    // Only react to an obstruction once the gun is settled inside its arc.
    float fGunPitch = pTurret->s.lerp.u.turret.gunAngles[0];
    if (fGunPitch > pTurretInfo->arcmax[0] || fGunPitch < pTurretInfo->arcmin[0])
    {
        Vec3Clear(self->Physics.vVelocity);
        Vec3Clear(self->Physics.vWishDelta);
        return ACTOR_THINK_DONE;
    }

    float fGunYaw = pTurret->s.lerp.u.turret.gunAngles[1];
    if (fGunYaw > pTurretInfo->arcmax[1]
        || fGunYaw < pTurretInfo->arcmin[1]
        || pTurretInfo->originError[0] != 0.0f
        || pTurretInfo->originError[1] != 0.0f
        || pTurretInfo->originError[2] != 0.0f
        || pTurretInfo->anglesError[0] != 0.0f
        || pTurretInfo->anglesError[1] != 0.0f
        || pTurretInfo->anglesError[2] != 0.0f)
    {
        Vec3Clear(self->Physics.vVelocity);
        Vec3Clear(self->Physics.vWishDelta);
        return ACTOR_THINK_DONE;
    }

    gentity_s *pHitEnt = &level.gentities[iHitEntnum];
    if (!pHitEnt->sentient)
    {
        // Wedged against a non-living obstruction while traversing: shrink the arc toward
        // it, and detach from the turret once that side of the arc has fully collapsed.
        if (fGunYaw < 0.0f)
        {
            pTurretInfo->arcmin[1] += 1.0f;
            if (pTurretInfo->arcmin[1] <= 0.0f)
            {
                Com_PrintWarning(18, "WARNING: capping rightarc of turret at (%.2f, %.2f, %.2f) to %.2f\n",
                    pTurret->r.currentOrigin[0], pTurret->r.currentOrigin[1], pTurret->r.currentOrigin[2],
                    -pTurretInfo->arcmin[1]);
                Vec3Clear(self->Physics.vVelocity);
                Vec3Clear(self->Physics.vWishDelta);
                return ACTOR_THINK_DONE;
            }
            pTurretInfo->arcmin[1] = 0.0f;
        }
        else
        {
            pTurretInfo->arcmax[1] -= 1.0f;
            if (pTurretInfo->arcmax[1] >= 0.0f)
            {
                Com_PrintWarning(18, "WARNING: capping leftarc of turret at (%.2f, %.2f, %.2f) to %.2f\n",
                    pTurret->r.currentOrigin[0], pTurret->r.currentOrigin[1], pTurret->r.currentOrigin[2],
                    pTurretInfo->arcmax[1]);
                Vec3Clear(self->Physics.vVelocity);
                Vec3Clear(self->Physics.vWishDelta);
                return ACTOR_THINK_DONE;
            }
            pTurretInfo->arcmax[1] = 0.0f;
        }

        Com_PrintWarning(18,
            "WARNING: AI %d at (%.2f, %.2f, %.2f) with turret angles (%.2f, %.2f) detaching from turret due to obstruction\n",
            self->ent->s.number,
            self->ent->r.currentOrigin[0], self->ent->r.currentOrigin[1], self->ent->r.currentOrigin[2],
            pTurret->s.lerp.u.turret.gunAngles[0], pTurret->s.lerp.u.turret.gunAngles[1]);
    }

    Actor_StopUseTurret(self);
    if (pHitEnt->client)
    {
        if (((1 << pHitEnt->sentient->eTeam) & ~(1 << Sentient_EnemyTeam(self->ent->sentient->eTeam))) != 0)
        {
            Scr_AddEntity(pHitEnt);
            Scr_Notify(self->ent, scr_const.trigger, 1u);
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
        if (!Actor_KnowAboutEnemy(self, 0))
            self->useEnemyGoal = 0;
        if (!Actor_KeepClaimedNode(self))
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
                        Actor_SetAnimScript(self, v13, AI_MOVE_STOP, AI_ANIM_MOVE_CODE);
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
#undef v81
#undef v82
#undef v83
#undef v85
#undef v86
#undef v87
#undef v88
#undef v89
#undef v90
#undef v91

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

