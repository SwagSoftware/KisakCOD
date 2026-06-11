#ifndef KISAK_SP 
#error This file is for SinglePlayer only 
#endif

#include "actor_corpse.h"

#include <xanim/xanim.h>
#include "g_scr_main.h"
#include "g_local.h"
#include <cgame/cg_local.h>
#include <script/scr_const.h>
#include "sentient.h"
#include "g_main.h"
#include "actor.h"
#include "actor_event_listeners.h"

float playerEyePos[3];

XAnimTree_s *__cdecl G_GetActorCorpseIndexAnimTree(unsigned int index)
{
    if (index >= 0x10)
        MyAssertHandler(
            "c:\\trees\\cod3\\cod3src\\src\\game\\actor_corpse.cpp",
            29,
            0,
            "index doesn't index ARRAY_COUNT( g_scr_data.actorCorpseInfo )\n\t%i not in [0, %i)",
            index,
            16);
    return g_scr_data.actorCorpseInfo[index].tree;
}

int __cdecl G_GetActorCorpseIndex(gentity_s *ent)
{
    int number; // r9
    int result; // r3
    int *p_entnum; // r11

    number = ent->s.number;
    result = 0;
    p_entnum = &g_scr_data.actorCorpseInfo[0].entnum;
    while (*p_entnum != number)
    {
        p_entnum += 8;
        ++result;
        if ((int)p_entnum >= (int)&g_scr_data.actorBackup)
        {
            if (!alwaysfails)
                MyAssertHandler(
                    "c:\\trees\\cod3\\cod3src\\src\\game\\actor_corpse.cpp",
                    51,
                    0,
                    "G_GetActorCorpseIndex called for non actor corpse");
            return 0;
        }
    }
    return result;
}

int G_GetFreeActorCorpseIndex(int reuse)
{
    int closestBehindIndex = 0;
    int closestSideIndex = 0;
    int farthestIndex = 0;

    double maxDistBehind = -1.0;
    double maxDistSide = -1.0;
    double maxDist = -1.0;

    gentity_s *playerEnt = G_Find(0, 284, scr_const.player);
    iassert(playerEnt);
    iassert(playerEnt->sentient);

    // Get player's eye position and view direction
    float playerEyePos[3];
    Sentient_GetEyePosition(playerEnt->sentient, playerEyePos);

    float viewDir[3] = { 0 };
    G_GetPlayerViewDirection(playerEnt, &viewDir[0], 0, 0);

    // Normalize view direction's X and Y. PowerPC fsel selects 1.0 as the divisor
    // when horizLen is exactly zero (degenerate case: looking straight up/down),
    // otherwise divides by horizLen to get a unit-length 2D direction.
    float horizLen = sqrtf(viewDir[0] * viewDir[0] + viewDir[1] * viewDir[1]);
    float divisor = (horizLen <= 0.0f) ? 1.0f : horizLen;
    float reciprocal = 1.0f / divisor;

    float normViewDirX = viewDir[0] * reciprocal;
    float normViewDirY = viewDir[1] * reciprocal;
    float normViewDirZ = 0.0f;

    // Default: project onto horizontal plane (player looking ~horizontally).
    bool projectHorizontal = true;
    if (horizLen == 0.0f)
    {
        // Looking straight up/down: re-read viewDir and use it raw (not normalized);
        // include the Z component so the dot product accounts for vertical-only views.
        projectHorizontal = false;
        G_GetPlayerViewDirection(playerEnt, &viewDir[0], 0, 0);
        normViewDirX = viewDir[0];
        normViewDirY = viewDir[1];
        normViewDirZ = viewDir[2];
    }

    int foundAnyCorpse = 0;
    int selectedIndex = -1;

    if (level.actorCorpseCount <= 0)
    {
        // No corpses, but reuse requested - this should not happen
        iassert(!reuse || !level.actorCorpseCount);
        return -1;
    }

    // Loop through corpse info entries
    for (int i = 0; i < level.actorCorpseCount; ++i)
    {
        int entnum = g_scr_data.actorCorpseInfo[i].entnum;
        if (entnum == -1)
        {
            if (reuse)
                return i;  // Return first free corpse slot if reuse requested
            continue;
        }

        gentity_s *corpseEnt = &level.gentities[entnum];
        foundAnyCorpse = 1;

        // Compute relative vector from player eye to corpse position
        float dx = corpseEnt->r.currentOrigin[0] - playerEyePos[0];
        float dy = corpseEnt->r.currentOrigin[1] - playerEyePos[1];
        float dz = corpseEnt->r.currentOrigin[2] - playerEyePos[2];

        // Collapse to horizontal plane in the normal case; keep full 3D delta only
        // when the player is looking straight up/down.
        if (projectHorizontal)
            dz = 0.0f;

        // Project vector onto normalized view direction
        float dotView = dx * normViewDirX + dy * normViewDirY + dz * normViewDirZ;

        // Calculate squared distance to corpse
        float distSq = dx * dx + dy * dy + dz * dz;

        // Find corpse behind player (dotView < 0) with max distance squared
        if (dotView < 0.0f && maxDistBehind < distSq)
        {
            maxDistBehind = distSq;
            closestBehindIndex = i;
        }

        // Find corpse somewhat to the side (dotView^2 <= distSq * 0.5) with max distance squared
        if ((dotView * dotView) <= (distSq * 0.5f) && maxDistSide < distSq)
        {
            maxDistSide = distSq;
            closestSideIndex = i;
        }

        // Find corpse with max distance squared overall
        if (maxDist < distSq)
        {
            maxDist = distSq;
            farthestIndex = i;
        }
    }

    if (foundAnyCorpse)
    {
        // Choose corpse index based on proximity heuristics
        if (maxDistBehind == -1.0)
        {
            // No corpse behind player, pick closest to side or farthest
            selectedIndex = (maxDistSide == -1.0) ? farthestIndex : closestSideIndex;
        }
        else
        {
            selectedIndex = closestBehindIndex;
        }

        // Free entity at selected corpse index and mark slot free
        G_FreeEntity(&level.gentities[g_scr_data.actorCorpseInfo[selectedIndex].entnum]);
        g_scr_data.actorCorpseInfo[selectedIndex].entnum = -1;

        return selectedIndex;
    }

    return -1;
}


void __cdecl G_RemoveActorCorpses(unsigned int allowedCorpseCount)
{
    signed int v2; // r28
    int actorCorpseCount; // r10
    int *p_entnum; // r30

    if (allowedCorpseCount > 0x10)
        MyAssertHandler(
            "c:\\trees\\cod3\\cod3src\\src\\game\\actor_corpse.cpp",
            173,
            0,
            "%s\n\t(allowedCorpseCount) = %i",
            "(allowedCorpseCount >= 0 && allowedCorpseCount <= 16)",
            allowedCorpseCount);
    v2 = allowedCorpseCount;
    actorCorpseCount = level.actorCorpseCount;
    if ((int)allowedCorpseCount < level.actorCorpseCount)
    {
        p_entnum = &g_scr_data.actorCorpseInfo[allowedCorpseCount].entnum;
        do
        {
            if (*p_entnum >= 0)
            {
                G_FreeEntity(&level.gentities[*p_entnum]);
                actorCorpseCount = level.actorCorpseCount;
            }
            ++v2;
            p_entnum += 8;
        } while (v2 < actorCorpseCount);
    }
    level.actorCorpseCount = allowedCorpseCount;
}

void __cdecl G_UpdateActorCorpses()
{
    signed int integer; // r31

    integer = ai_corpseCount->current.integer;
    if (integer < 1 || integer > 16)
        MyAssertHandler(
            "c:\\trees\\cod3\\cod3src\\src\\game\\actor_corpse.cpp",
            197,
            0,
            "%s\n\t(actorCorpseCount) = %i",
            "(actorCorpseCount >= 1 && actorCorpseCount <= 16)",
            ai_corpseCount->current.integer);
    if (level.actorCorpseCount != integer)
        G_RemoveActorCorpses(integer);
}

int __cdecl G_PruneCorpsesSortCmp(int *a, int *b)
{
    int v2; // r27
    int v3; // r29
    unsigned int num_entities; // r8
    unsigned int entnum; // r7
    int v6; // r27
    unsigned int v7; // r7
    gentity_s *v8; // r11
    gentity_s *v9; // r10
    double v10; // fp0

    v2 = *b;
    v3 = *a;
    num_entities = level.num_entities;
    entnum = g_scr_data.actorCorpseInfo[v3].entnum;
    if (entnum >= level.num_entities)
    {
        MyAssertHandler(
            "c:\\trees\\cod3\\cod3src\\src\\game\\actor_corpse.cpp",
            220,
            0,
            "g_scr_data.actorCorpseInfo[aIdx].entnum doesn't index level.num_entities\n\t%i not in [0, %i)",
            entnum,
            level.num_entities);
        num_entities = level.num_entities;
    }
    v6 = v2;
    v7 = g_scr_data.actorCorpseInfo[v6].entnum;
    if (v7 >= num_entities)
        MyAssertHandler(
            "c:\\trees\\cod3\\cod3src\\src\\game\\actor_corpse.cpp",
            221,
            0,
            "g_scr_data.actorCorpseInfo[bIdx].entnum doesn't index level.num_entities\n\t%i not in [0, %i)",
            v7,
            num_entities);
    v8 = &level.gentities[g_scr_data.actorCorpseInfo[v3].entnum];
    v9 = &level.gentities[g_scr_data.actorCorpseInfo[v6].entnum];
    v10 = (float)((float)((float)((float)(v8->r.currentOrigin[1] - playerEyePos[1])
        * (float)(v8->r.currentOrigin[1] - playerEyePos[1]))
        + (float)((float)((float)(v8->r.currentOrigin[0] - playerEyePos[0])
            * (float)(v8->r.currentOrigin[0] - playerEyePos[0]))
            + (float)((float)(v8->r.currentOrigin[2] - playerEyePos[2])
                * (float)(v8->r.currentOrigin[2] - playerEyePos[2]))))
        - (float)((float)((float)(v9->r.currentOrigin[1] - playerEyePos[1])
            * (float)(v9->r.currentOrigin[1] - playerEyePos[1]))
            + (float)((float)((float)(v9->r.currentOrigin[0] - playerEyePos[0])
                * (float)(v9->r.currentOrigin[0] - playerEyePos[0]))
                + (float)((float)(v9->r.currentOrigin[2] - playerEyePos[2])
                    * (float)(v9->r.currentOrigin[2] - playerEyePos[2])))));
    if (v10 >= -0.000002)
        return v10 > 0.000002;
    else
        return -1;
}

void __cdecl G_PruneLoadedCorpses()
{
    _QWORD *v0; // r11
    int v1; // ctr
    signed int v2; // r28
    int v3; // r11
    unsigned int *v4; // r9
    int *p_entnum; // r10
    gentity_s *ent; // r30
    int v7; // r27
    int *v8; // r29
    int v9; // r31
    unsigned int entnum; // r7
    // KISAKFIX: IDA pseudocode rendered the 64-byte int[16] buffer as `_BYTE v11[24]; char v12`
    // (stack-slot collapse — v11 ended at sp+0x68, v12 at sp+0x68 was really `&v11[6]`). Kisak
    // port copied this literally: the QWORD-fill loop writes 64 bytes into v11[24] (UB), and
    // the "v8 = &v12" read is undefined unless v11/v12 happen to be contiguous. With >6 corpses
    // on a savegame load, this corrupts stack (return addr, qsort comparator ptr, locals).
    int corpseIndices[16]; // [sp+50h] BYREF — was: _BYTE v11[24] + char v12

    v0 = (_QWORD*)corpseIndices;
    v1 = 8;
    do
    {
        *v0++ = 0x800000000LL;
        --v1;
    } while (v1);
    v2 = 0;
    v3 = 0;
    v4 = (unsigned int*)corpseIndices;
    p_entnum = &g_scr_data.actorCorpseInfo[0].entnum;
    do
    {
        if (*p_entnum >= 0)
        {
            *v4 = v3;
            ++v2;
            ++v4;
        }
        p_entnum += 8;
        ++v3;
    } while ((int)p_entnum < (int)&g_scr_data.actorBackup);
    if (v2 > 6)
    {
        ent = G_Find(0, 284, scr_const.player);
        iassert(ent);
        iassert(ent->sentient);
        Sentient_GetEyePosition(ent->sentient, playerEyePos);
        qsort(corpseIndices, v2, 4u, (int(__cdecl *)(const void *, const void *))G_PruneCorpsesSortCmp);
        v7 = v2 - 6;
        v8 = &corpseIndices[6]; // was: (int*)&v12 — really &v11[6] after stack-slot fix
        do
        {
            v9 = *v8;
            entnum = g_scr_data.actorCorpseInfo[v9].entnum;
            if (entnum >= level.num_entities)
                MyAssertHandler(
                    "c:\\trees\\cod3\\cod3src\\src\\game\\actor_corpse.cpp",
                    275,
                    0,
                    "g_scr_data.actorCorpseInfo[corpseIdx].entnum doesn't index level.num_entities\n\t%i not in [0, %i)",
                    entnum,
                    level.num_entities);
            G_FreeEntity(&level.gentities[g_scr_data.actorCorpseInfo[v9].entnum]);
            --v7;
            ++v8;
            g_scr_data.actorCorpseInfo[v9].entnum = -1;
        } while (v7);
    }
}

void __cdecl ActorCorpse_Free(gentity_s *ent)
{
    int ActorCorpseIndex; // r3
    int number; // r10
    int v4; // r31

    ActorCorpseIndex = G_GetActorCorpseIndex(ent);
    number = ent->s.number;
    v4 = ActorCorpseIndex;
    if (g_scr_data.actorCorpseInfo[ActorCorpseIndex].entnum != number)
        MyAssertHandler(
            "c:\\trees\\cod3\\cod3src\\src\\game\\actor_corpse.cpp",
            294,
            0,
            "%s",
            "g_scr_data.actorCorpseInfo[actorCorpseIndex].entnum == ent->s.number");
    g_scr_data.actorCorpseInfo[v4].entnum = -1;
}

void __cdecl Actor_CorpseThink(gentity_s *self)
{
    tagInfo_s *tagInfo; // r10

    //Profile_Begin(224);
    tagInfo = self->tagInfo;
    self->nextthink = level.time + 50;
    if (tagInfo)
        G_GeneralLink(self);
    //Profile_EndInternal(0);
}

float __cdecl Actor_SetBodyPlantAngle(
    int iEntNum,
    int iClipMask,
    float *vOrigin,
    const float *vCenter,
    const float *vDir,
    float *pfAngle)
{
    float vStart[3]; // [sp+50h] [-F0h] BYREF // v37
    float vEnd[3]; // [sp+60h] [-E0h] BYREF // v40
    float vMaxs[3]; // [sp+70h] [-D0h] BYREF
    float vMins[3]; // [sp+80h] [-C0h] BYREF
    trace_t trace; // [sp+A0h] [-A0h] BYREF

    float vPointA[3];
    float vPointB[3];
    float vDelta[3];

    static const float fStartUp = 30.0f;
    static const float fEndDown = 30.0f;
    static const float fSize = 4.1f;

    vStart[0] = vOrigin[0];
    vStart[1] = vOrigin[1];
    vStart[2] = (vOrigin[2] + 30.0f);

    vMins[0] = -4.0f;
    vMins[1] = -4.0f;
    vMins[2] = 0.0f;

    vMaxs[0] = 4.0f;
    vMaxs[1] = 4.0f;
    vMaxs[2] = 8.0f;

    vEnd[0] = (vDir[0] * 10.9f) + vStart[0];
    vEnd[1] = (vDir[1] * 10.9f) + vOrigin[1];
    vEnd[2] = (vDir[2] * 10.9f) + (vOrigin[2] + 30.0f);

    G_TraceCapsule(&trace, vStart, vMins, vMaxs, vEnd, iEntNum, iClipMask);

    if (trace.fraction == 0.0)
    {
        *pfAngle = 0.0f;
        return vCenter[2];
    }

    Vec3Lerp(vStart, vEnd, trace.fraction, vStart);
    vEnd[0] = vStart[0];
    vEnd[1] = vStart[1];
    vEnd[2] = vStart[2];
    vEnd[2] = vOrigin[2] - fEndDown;

    G_TraceCapsule(&trace, vStart, vMins, vMaxs, vEnd, iEntNum, iClipMask);

    if (trace.startsolid || trace.fraction >= 1.0)
    {
        vPointA[0] = vCenter[0];
        vPointA[1] = vCenter[1];
        vPointA[2] = vCenter[2];
    }
    else
    {
        Vec3Lerp(vStart, vEnd, trace.fraction, vPointA);
    }

    vStart[0] = vOrigin[0];
    vStart[1] = vOrigin[1];
    vStart[2] = vOrigin[2];
    vStart[2] = vStart[2] + fStartUp;

    vEnd[0] = ((-(15.0f - fSize)) * vDir[0]) + vStart[0];
    vEnd[1] = ((-(15.0f - fSize)) * vDir[1]) + vStart[1];
    vEnd[2] = ((-(15.0f - fSize)) * vDir[2]) + vStart[2];

    G_TraceCapsule(&trace, vStart, vMins, vMaxs, vEnd, iEntNum, iClipMask);

    if (trace.fraction == 0.0)
    {
        *pfAngle = 0.0f;
        return vCenter[2];
    }
    else
    {
        Vec3Lerp(vStart, vEnd, trace.fraction, vStart);
        vEnd[0] = vStart[0];
        vEnd[1] = vStart[1];
        vEnd[2] = vStart[2];
        vEnd[2] = vOrigin[2] - fEndDown;

        G_TraceCapsule(&trace, vStart, vMins, vMaxs, vEnd, iEntNum, iClipMask);

        if (trace.startsolid || trace.fraction >= 1.0f)
        {
            vPointB[0] = vCenter[0];
            vPointB[1] = vCenter[1];
            vPointB[2] = vCenter[2];
        }
        else
        {
            Vec3Lerp(vStart, vEnd, trace.fraction, vPointB);
        }

        if (vPointA[0] == vPointB[0] && vPointA[1] == vPointB[1] && vPointA[2] == vPointB[2])
        {
            *pfAngle = 0.0f;
        }
        else
        {
            vDelta[0] = vPointA[0] - vPointB[0];
            vDelta[1] = vPointA[1] - vPointB[1];
            vDelta[2] = vPointA[2] - vPointB[2];
            float angle = vectopitch(vDelta);
            *pfAngle = AngleNormalize180(angle);
        }
        return (vCenter[2] + vPointA[2] + vPointB[2]) / 3.0;
    }
}

void __cdecl Actor_GetBodyPlantAngles(
    int iEntNum,
    int iClipMask,
    float *vOrigin,
    float fYaw,
    float *pfPitch,
    float *pfRoll,
    float *pfHeight)
{
    double v15; // fp12
    double v16; // fp30
    double v17; // fp31
    double v18; // fp12
    double v19; // fp13
    // KISAKFIX: IDA had v20/v21/v22 at sp+0x50/0x54/0x58 (start vec) and v23/v24/v25 at
    // sp+0x60/0x64/0x68 (end vec). Both passed as &v20 / &v23 to G_TraceCapsuleComplete
    // expecting float[3]. Pack into arrays so the address-of-first-element works.
    float traceStart[3]; // was v20 (BYREF) + v21 + v22
    float traceEnd[3];   // was v23 (BYREF) + v24 + v25
    // KISAKFIX: IDA shows two BYREF float[3] locals (var_70/var_60) populated by
    // a single YawVectors call. On PPC ABI r4 = var_70 (forward output) and
    // r5 = var_60 (right output). First Actor_SetBodyPlantAngle uses var_70
    // (forward); second uses var_60 (right). Kisak port called
    // `YawVectors(fYaw, NULL, right)` — which on x86 cdecl puts the RIGHT vec
    // into the `right` array and never populates the second slot — then passed
    // `right` to the first SetBodyPlantAngle (where IDA passes forward) and
    // `v27` (UNINITIALIZED) to the second (where IDA passes right). Body-plant
    // angle math is fed wrong vectors.
    float forward[3];
    float right[3];

    iassert(pfPitch);

    v15 = vOrigin[2];
    traceStart[0] = *vOrigin;
    traceEnd[0] = traceStart[0];
    traceStart[1] = vOrigin[1];
    traceStart[2] = (float)v15 + (float)30.0;
    traceEnd[1] = traceStart[1];
    traceEnd[2] = (float)v15 - (float)30.0;
    YawVectors(fYaw, forward, right);
    v16 = Actor_SetBodyPlantAngle((int)iEntNum, iClipMask, vOrigin, vOrigin, forward, pfPitch);
    if (pfHeight)
    {
        if (fabsf(*pfPitch) >= 30.0)
            *pfHeight = 0.0;
        else
            v16 = (float)((float)(Actor_SetBodyPlantAngle((int)iEntNum, iClipMask, vOrigin, vOrigin, right, pfHeight)
                + (float)v16)
                * (float)0.5);
    }
    if (pfRoll)
    {
        v17 = (float)((float)v16 - vOrigin[2]);
        if (v17 < 0.0)
        {
            traceStart[0] = *vOrigin;
            traceEnd[0] = traceStart[0];
            v18 = vOrigin[2];
            v19 = vOrigin[1];
            traceStart[2] = vOrigin[2];
            traceStart[1] = v19;
            traceEnd[1] = v19;
            traceEnd[2] = (float)v18 - (float)1.0;
            if (G_TraceCapsuleComplete(traceStart, actorMins, actorMaxs, traceEnd, (int)iEntNum, iClipMask))
                v17 = 0.0;
        }
        *pfRoll = v17;
    }
}

// aislop
float Actor_Orient_LerpWithLimit(float current, float newValue, float delta, float rate)
{
    // If the difference (delta) is less than or equal to rate, snap to newValue
    if (fabsf(delta) <= rate)
    {
        return newValue;
    }
    else
    {
        // Otherwise, move from current toward newValue by 'rate' units, respecting delta�s sign
        float sign = (delta > 0) ? 1.0f : -1.0f;
        return current + sign * rate;
    }
}

void __cdecl Actor_OrientCorpseToGround(gentity_s *self, int bLerp)
{
    int eType; // r11
    actor_prone_info_s *p_proneInfo; // r30
    double currentYaw; // fp1
    float pitch; // fp1
    int entNum; // r3
    float *origin; // r5
    unsigned int clipMask; // r4
    float roll; // [sp+54h] [-4Ch] BYREF
    float height; //v28

    eType = self->s.eType;

    iassert((self->s.eType == ET_ACTOR_CORPSE) || (self->s.eType == ET_ACTOR));

    if (self->s.eType == ET_ACTOR_CORPSE)
        p_proneInfo = &g_scr_data.actorCorpseInfo[G_GetActorCorpseIndex(self)].proneInfo;
    else
        p_proneInfo = &self->actor->ProneInfo;

    if (p_proneInfo->bCorpseOrientation)
    {
        currentYaw = self->r.currentAngles[YAW];
        entNum = self->s.number;
        origin = self->r.currentOrigin;
        clipMask = self->clipmask & 0xFDFF3FFF;
        if (bLerp)
        {
            Actor_GetBodyPlantAngles(entNum, clipMask, origin, currentYaw, &pitch, &roll, &height);

            // KISAKFIX: IDA Actor_OrientCorpseToGround at 0x821ea480 uses
            // `fsel + fmadds` = `current += sign * rate` (rate-limited step toward
            // target by ±rate). Kisak port wrote `current += diff * rate * sign`
            // which = `current += |diff| * rate` (always positive, no rate limit)
            // — overshoots every frame, e.g. typical 20° pitchDiff → 120° jump.
            // Also body-height rate is 0.6 (not 6.0) and the waist-pitch path was
            // missing the `* sign` entirely. Result: corpse pitch/waist/height
            // spasm wildly.
            float pitchDiff = AngleSubtract(pitch, p_proneInfo->fTorsoPitch);
            if (I_fabs(pitchDiff) <= 6.0f)
            {
                p_proneInfo->fTorsoPitch = pitch;
            }
            else
            {
                float sign = (pitchDiff >= 0.0f) ? 1.0f : -1.0f;
                p_proneInfo->fTorsoPitch += sign * 6.0f;
            }

            float waistPitchDiff = AngleSubtract(roll, p_proneInfo->fWaistPitch);
            if (I_fabs(waistPitchDiff) <= 6.0)
            {
                p_proneInfo->fWaistPitch = roll;
            }
            else
            {
                float sign = (waistPitchDiff >= 0.0f) ? 1.0f : -1.0f;
                p_proneInfo->fWaistPitch += sign * 6.0f;
            }

            float heightDiff = height - p_proneInfo->fBodyHeight;
            if (I_fabs(heightDiff) <= 0.6f)
            {
                p_proneInfo->fBodyHeight = height;
            }
            else
            {
                float sign = (heightDiff >= 0.0f) ? 1.0f : -1.0f;
                p_proneInfo->fBodyHeight += sign * 0.60000002f;
            }
        }
        else
        {
            Actor_GetBodyPlantAngles(
                entNum,
                clipMask,
                origin,
                currentYaw,
                &p_proneInfo->fTorsoPitch,
                &p_proneInfo->fWaistPitch,
                &p_proneInfo->fBodyHeight);
        }
    }
}

void __cdecl Actor_OrientPitchToGround(gentity_s *self, int bLerp)
{
    actor_prone_info_s *p_ProneInfo; // r30
    float yaw; // fp1
    int number; // r3
    unsigned int clipMask; // r4
    float *currentOrigin; // r5
    float pitch; // [sp+50h] [-30h] BYREF
    float height; // [sp+54h] [-2Ch] BYREF

    iassert(self->s.eType == ET_ACTOR);

    p_ProneInfo = &self->actor->ProneInfo;
    if (self->actor->ProneInfo.orientPitch)
    {
        yaw = self->r.currentAngles[YAW];
        number = self->s.number;
        clipMask = self->clipmask & 0xFDFF3FFF;
        currentOrigin = self->r.currentOrigin;
        if (bLerp)
        {
            Actor_GetBodyPlantAngles(number, clipMask, currentOrigin, yaw, &pitch, NULL, &height);

            // KISAKFIX: same rate-limited-step bug as Actor_OrientCorpseToGround above.
            // IDA Actor_OrientPitchToGround at 0x821ea62c uses `current += sign * rate`;
            // kisak port wrote `current += diff * rate * sign` = `|diff| * rate`.
            // Affects ALIVE actors with `ProneInfo.orientPitch` set every Think tick —
            // prone enemies pitch-spasm and float/sink vertically.
            float diff = AngleSubtract(pitch, p_ProneInfo->fTorsoPitch);
            if (I_fabs(diff) <= 6.0)
            {
                p_ProneInfo->fTorsoPitch = pitch;
            }
            else
            {
                float sign = (diff >= 0.0f) ? 1.0f : -1.0f;
                p_ProneInfo->fTorsoPitch += sign * 6.0f;
            }

            float delta = height - p_ProneInfo->fBodyHeight;
            if (fabsf(delta) <= 0.6f)
            {
                p_ProneInfo->fBodyHeight = height;
            }
            else
            {
                float sign = (delta >= 0.0f) ? 1.0f : -1.0f;
                p_ProneInfo->fBodyHeight += sign * 0.60000002f;
            }
        }
        else
        {
            Actor_GetBodyPlantAngles(
                number,
                clipMask,
                currentOrigin,
                yaw,
                &self->actor->ProneInfo.fTorsoPitch,
                NULL,
                &self->actor->ProneInfo.fBodyHeight);
        }
    }
}

int __cdecl Actor_BecomeCorpse(gentity_s *self)
{
    int FreeActorCorpseIndex; // r27
    actor_s *actor; // r29
    actor_prone_info_s *p_ProneInfo; // r31
    unsigned int ActorIndex; // r28
    int IsProne; // r3
    unsigned int *v8; // r11
    int v9; // ctr
    int v10; // r26
    trajectory_t *p_pos; // r31
    char IsRagdollTrajectory; // r3
    float *currentOrigin; // r4
    trType_t trType; // r29
    corpseInfo_t *v15; // r31
    unsigned __int16 death; // r4
    XAnimTree_s *v17; // r11
    float *v18; // r6
    int v19; // r5
    actor_prone_info_s *v20; // r11
    unsigned int *v21; // r9
    actor_prone_info_s *v22; // r8
    int v23; // ctr
    int time; // r10
    _BYTE v25[96]; // [sp+50h] [-60h] BYREF

    FreeActorCorpseIndex = G_GetFreeActorCorpseIndex(1);
    if (FreeActorCorpseIndex < 0)
        return 0;
    actor = self->actor;
    p_ProneInfo = &actor->ProneInfo;
    ActorIndex = G_GetActorIndex(actor);
    IsProne = BG_ActorIsProne(&actor->ProneInfo, level.time);
    v8 = (unsigned int*)v25;
    v9 = 6;
    do
    {
        *v8 = *(unsigned int *)&p_ProneInfo->bCorpseOrientation;
        p_ProneInfo = (actor_prone_info_s *)((char *)p_ProneInfo + 4);
        ++v8;
        --v9;
    } while (v9);
    if (IsProne || actor->eAnimMode == AI_ANIM_NOPHYSICS || (v10 = 1, self->tagInfo))
        v10 = 0;
    Actor_EventListener_RemoveEntity(self->s.number);
    Actor_Free(actor);
    if (self->actor)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\actor_corpse.cpp", 576, 0, "%s", "self->actor == NULL");
    if (self->sentient)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\actor_corpse.cpp", 577, 0, "%s", "self->sentient == NULL");
    self->physicsObject = 1;
    p_pos = &self->s.lerp.pos;
    self->handler = ENT_HANDLER_ACTOR_CORPSE;
    self->r.contents = 0x4000000;
    self->s.eType = ET_ACTOR_CORPSE;
    self->clipmask = 8519697;
    IsRagdollTrajectory = Com_IsRagdollTrajectory(&self->s.lerp.pos);
    currentOrigin = self->r.currentOrigin;
    if (IsRagdollTrajectory)
    {
        trType = p_pos->trType;
        G_SetOrigin(self, currentOrigin);
        v10 = 0;
        p_pos->trType = trType;
    }
    else
    {
        G_SetOrigin(self, currentOrigin);
        p_pos->trType = TR_STATIONARY;
    }
    v15 = &g_scr_data.actorCorpseInfo[FreeActorCorpseIndex];
    death = scr_const.death;
    v17 = g_scr_data.actorXAnimTrees[ActorIndex];
    g_scr_data.actorXAnimTrees[ActorIndex] = v15->tree;
    v15->tree = v17;
    v15->entnum = self->s.number;
    Scr_Notify(self, death, 0);
    Scr_FreeEntityNum(self->s.number, CLASS_NUM_ENTITY);
    G_DObjUpdate(self);
    v20 = &g_scr_data.actorCorpseInfo[FreeActorCorpseIndex].proneInfo;
    v21 = (unsigned int*)v25;
    v22 = v20;
    v23 = 6;
    do
    {
        *(unsigned int *)&v22->bCorpseOrientation = *v21++;
        v22 = (actor_prone_info_s *)((char *)v22 + 4);
        --v23;
    } while (v23);
    if (v10)
    {
        if (!v20->bCorpseOrientation)
        {
            time = level.time;
            v20->bCorpseOrientation = 1;
            v15->proneInfo.prone = 1;
            v15->proneInfo.fTorsoPitch = 0.0;
            v15->proneInfo.iProneTime = time;
            v15->proneInfo.fWaistPitch = 0.0;
            v15->proneInfo.iProneTrans = 500;
        }
        Actor_OrientCorpseToGround(self, 0);
    }
    SV_LinkEntity(self);
    return 1;
}

XAnimTree_s *__cdecl G_GetActorCorpseAnimTree(gentity_s *ent)
{
    return g_scr_data.actorCorpseInfo[G_GetActorCorpseIndex(ent)].tree;
}

