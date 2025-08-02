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

    // Normalize view direction's X and Y
    float horizLen = sqrtf(viewDir[0] * viewDir[0] + viewDir[1] * viewDir[1]);
    if (horizLen == 0.0f)
    {
        // If horizontal length zero, just use viewDir as is
        horizLen = 1.0f;
    }

    float normViewDirX = viewDir[0] / horizLen;
    float normViewDirY = viewDir[1] / horizLen;
    float normViewDirZ = 0.0f;

    if (horizLen == 0.0f)
    {
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

        // If horizLen was zero, treat vertical difference as zero for projection
        if (horizLen == 0.0f)
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
    _BYTE v11[24]; // [sp+50h] [-90h] BYREF
    char v12; // [sp+68h] [-78h] BYREF

    v0 = (_QWORD*)v11;
    v1 = 8;
    do
    {
        *v0++ = 0x800000000LL;
        --v1;
    } while (v1);
    v2 = 0;
    v3 = 0;
    v4 = (unsigned int*)v11;
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
        qsort(v11, v2, 4u, (int(__cdecl *)(const void *, const void *))G_PruneCorpsesSortCmp);
        v7 = v2 - 6;
        v8 = (int *)&v12;
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
    double v12; // fp10
    double v13; // fp12
    double v14; // fp11
    double v15; // fp9
    double v16; // fp8
    double v17; // fp1
    double v18; // fp7
    double v19; // fp13
    double v20; // fp12
    double v21; // fp27
    double v22; // fp26
    double v23; // fp28
    double v24; // fp12
    double v25; // fp11
    double v26; // fp13
    double v27; // fp0
    double v28; // fp9
    double v29; // fp7
    double v30; // fp13
    double v31; // fp12
    double v32; // fp13
    double v33; // fp12
    double v34; // fp31
    double v35; // fp1
    float v37; // [sp+50h] [-F0h] BYREF
    float v38; // [sp+54h] [-ECh]
    float v39; // [sp+58h] [-E8h]
    float v40; // [sp+60h] [-E0h] BYREF
    float v41; // [sp+64h] [-DCh]
    float v42; // [sp+68h] [-D8h]
    float v43[4]; // [sp+70h] [-D0h] BYREF
    float v44[4]; // [sp+80h] [-C0h] BYREF
    float v45[4]; // [sp+90h] [-B0h] BYREF
    trace_t v46[2]; // [sp+A0h] [-A0h] BYREF

    v12 = *vDir;
    v13 = vOrigin[1];
    v14 = (float)(vOrigin[2] + (float)30.0);
    v15 = vDir[1];
    v16 = vDir[2];
    v37 = *vOrigin;
    v38 = v13;
    v44[0] = -4.0;
    v44[1] = -4.0;
    v39 = v14;
    v44[2] = 0.0;
    v43[0] = 4.0;
    v43[1] = 4.0;
    v43[2] = 8.0;
    v40 = (float)((float)v12 * (float)10.9) + v37;
    v41 = (float)((float)v15 * (float)10.9) + (float)v13;
    v42 = (float)((float)v16 * (float)10.9) + (float)v14;
    G_TraceCapsule(v46, &v37, v44, v43, &v40, iEntNum, iClipMask);
    if (v46[0].fraction == 0.0)
        goto LABEL_2;
    v18 = (float)(v42 - v39);
    v42 = vOrigin[2] - (float)30.0;
    v19 = (float)((float)((float)(v40 - v37) * v46[0].fraction) + v37);
    v40 = (float)((float)(v40 - v37) * v46[0].fraction) + v37;
    v37 = v19;
    v20 = (float)((float)((float)(v41 - v38) * v46[0].fraction) + v38);
    v41 = (float)((float)(v41 - v38) * v46[0].fraction) + v38;
    v38 = v20;
    v39 = (float)((float)v18 * v46[0].fraction) + v39;
    G_TraceCapsule(v46, &v37, v44, v43, &v40, iEntNum, iClipMask);
    if (v46[0].startsolid || v46[0].fraction >= 1.0)
    {
        v21 = *vCenter;
        v22 = vCenter[1];
        v23 = vCenter[2];
    }
    else
    {
        v21 = (float)((float)((float)(v40 - v37) * v46[0].fraction) + v37);
        v22 = (float)((float)((float)(v41 - v38) * v46[0].fraction) + v38);
        v23 = (float)((float)((float)(v42 - v39) * v46[0].fraction) + v39);
    }
    v24 = (float)(vOrigin[2] + (float)30.0);
    v25 = *vDir;
    v37 = *vOrigin;
    v26 = vOrigin[1];
    v27 = (float)-(float)((float)(vDir[1] * (float)10.9) - vOrigin[1]);
    v40 = -(float)((float)((float)v25 * (float)10.9) - v37);
    v28 = vDir[2];
    v41 = v27;
    v38 = v26;
    v39 = v24;
    v42 = -(float)((float)((float)v28 * (float)10.9) - (float)v24);
    G_TraceCapsule(v46, &v37, v44, v43, &v40, iEntNum, iClipMask);
    if (v46[0].fraction == 0.0)
    {
    LABEL_2:
        *pfAngle = 0.0;
        v17 = vCenter[2];
    }
    else
    {
        v29 = (float)(v42 - v39);
        v42 = vOrigin[2] - (float)30.0;
        v30 = (float)((float)((float)(v40 - v37) * v46[0].fraction) + v37);
        v40 = (float)((float)(v40 - v37) * v46[0].fraction) + v37;
        v37 = v30;
        v31 = (float)((float)((float)(v41 - v38) * v46[0].fraction) + v38);
        v41 = (float)((float)(v41 - v38) * v46[0].fraction) + v38;
        v38 = v31;
        v39 = (float)((float)v29 * v46[0].fraction) + v39;
        G_TraceCapsule(v46, &v37, v44, v43, &v40, iEntNum, iClipMask);
        if (v46[0].startsolid || v46[0].fraction >= 1.0)
        {
            v32 = *vCenter;
            v33 = vCenter[1];
            v34 = vCenter[2];
        }
        else
        {
            v32 = (float)((float)((float)(v40 - v37) * v46[0].fraction) + v37);
            v33 = (float)((float)((float)(v41 - v38) * v46[0].fraction) + v38);
            v34 = (float)((float)((float)(v42 - v39) * v46[0].fraction) + v39);
        }
        if (v21 == v32 && v22 == v33 && v23 == v34)
        {
            *pfAngle = 0.0;
        }
        else
        {
            v45[0] = (float)v21 - (float)v32;
            v45[1] = (float)v22 - (float)v33;
            v45[2] = (float)v23 - (float)v34;
            v35 = vectopitch(v45);
            *pfAngle = AngleNormalize180(v35);
        }
        v17 = (float)((float)((float)(vCenter[2] + (float)v34) + (float)v23) * (float)0.33333334);
    }
    return *((float *)&v17 + 1);
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
    float v20; // [sp+50h] [-90h] BYREF
    float v21; // [sp+54h] [-8Ch]
    float v22; // [sp+58h] [-88h]
    float v23; // [sp+60h] [-80h] BYREF
    float v24; // [sp+64h] [-7Ch]
    float v25; // [sp+68h] [-78h]
    float right[4]; // [sp+70h] [-70h] BYREF
    float v27[4]; // [sp+80h] [-60h] BYREF

    iassert(pfPitch);

    v15 = vOrigin[2];
    v20 = *vOrigin;
    v23 = v20;
    v21 = vOrigin[1];
    v22 = (float)v15 + (float)30.0;
    v24 = v21;
    v25 = (float)v15 - (float)30.0;
    YawVectors(fYaw, NULL, right); // KISAKTODO: "right" might not be right
    v16 = Actor_SetBodyPlantAngle((int)iEntNum, iClipMask, vOrigin, vOrigin, right, pfPitch);
    if (pfHeight)
    {
        if (fabsf(*pfPitch) >= 30.0)
            *pfHeight = 0.0;
        else
            v16 = (float)((float)(Actor_SetBodyPlantAngle((int)iEntNum, iClipMask, vOrigin, vOrigin, v27, pfHeight)
                + (float)v16)
                * (float)0.5);
    }
    if (pfRoll)
    {
        v17 = (float)((float)v16 - vOrigin[2]);
        if (v17 < 0.0)
        {
            v20 = *vOrigin;
            v23 = v20;
            v18 = vOrigin[2];
            v19 = vOrigin[1];
            v22 = vOrigin[2];
            v21 = v19;
            v24 = v19;
            v25 = (float)v18 - (float)1.0;
            if (G_TraceCapsuleComplete(&v20, actorMins, actorMaxs, &v23, (int)iEntNum, iClipMask))
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
        // Otherwise, move from current toward newValue by 'rate' units, respecting delta’s sign
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
    float pitch; // [sp+50h] [-50h] BYREF
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

            float pitchDiff = AngleSubtract(pitch, p_proneInfo->fTorsoPitch);
            if (I_fabs(pitchDiff) <= 6.0f)
            {
                p_proneInfo->fTorsoPitch = pitch;
            }
            else
            {
                float sign = (pitchDiff >= 0.0f) ? 1.0f : -1.0f;
                p_proneInfo->fTorsoPitch += (pitchDiff * 6.0f * sign);
            }

            float waistPitchDiff = AngleSubtract(roll, p_proneInfo->fWaistPitch);
            if (I_fabs(waistPitchDiff) <= 6.0)
            {
                p_proneInfo->fWaistPitch = roll;
            }
            else
            {
                float sign = (waistPitchDiff >= 0.0f) ? 1.0f : -1.0f;
                p_proneInfo->fWaistPitch += (waistPitchDiff * 6.0f);
            }

            float heightDiff = height - p_proneInfo->fBodyHeight;
            if (I_fabs(heightDiff) <= 0.6f)
            {
                p_proneInfo->fBodyHeight = height;
            }
            else
            {
                float sign = (heightDiff >= 0.0f) ? 1.0f : -1.0f;
                p_proneInfo->fBodyHeight += (heightDiff * 6.0f);
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

            float diff = AngleSubtract(pitch, p_ProneInfo->fTorsoPitch);
            if (I_fabs(diff) <= 6.0)
            {
                p_ProneInfo->fTorsoPitch = pitch;
            }
            else
            {
                float sign = (diff >= 0.0f) ? 1.0f : -1.0f;
                p_ProneInfo->fTorsoPitch += (diff * 6.0f * sign);
            }

            float delta = height - p_ProneInfo->fBodyHeight;
            if (fabsf(delta) <= 0.6f)
            {
                p_ProneInfo->fBodyHeight = height;
            }
            else
            {
                float sign = (delta >= 0.0f) ? 1.0f : -1.0f;
                p_ProneInfo->fBodyHeight += (delta * 0.6f * sign);
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
    self->handler = 3;
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
    Scr_FreeEntityNum(self->s.number, 0);
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

