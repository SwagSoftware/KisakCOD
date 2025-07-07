#ifndef KISAK_SP 
#error This file is for SinglePlayer only 
#endif

#include "actor_corpse.h"

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

int __cdecl G_GetFreeActorCorpseIndex(int reuse)
{
    int v2; // r25
    int v3; // r26
    int v4; // r27
    double v5; // fp30
    double v6; // fp28
    double v7; // fp29
    gentity_s *v8; // r31
    char v9; // r30
    double v10; // fp0
    bool v12; // mr_fpscr50
    double v14; // fp0
    double v15; // fp9
    double v16; // fp10
    double v17; // fp11
    int v18; // r8
    int result; // r3
    int *p_entnum; // r7
    gentity_s *v21; // r11
    double v22; // fp0
    double v23; // fp13
    double v24; // fp0
    int v25; // r31
    float v26; // [sp+50h] [-90h] BYREF
    float v27; // [sp+54h] [-8Ch]
    float v28; // [sp+58h] [-88h]
    float v29; // [sp+60h] [-80h] BYREF
    float v30; // [sp+64h] [-7Ch]
    float v31; // [sp+68h] [-78h]

    v2 = 0;
    v3 = 0;
    v4 = 0;
    v5 = -1.0;
    v6 = -1.0;
    v7 = -1.0;
    v8 = G_Find(0, 284, scr_const.player);
    if (!v8)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\actor_corpse.cpp", 88, 0, "%s", "ent");
    if (!v8->sentient)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\actor_corpse.cpp", 89, 0, "%s", "ent->sentient");
    Sentient_GetEyePosition(v8->sentient, &v29);
    G_GetPlayerViewDirection(v8, &v26, 0, 0);
    v9 = 1;
    v10 = __fsqrts((float)((float)(v26 * v26) + (float)(v27 * v27)));
    _FP10 = -v10;
    v12 = v10 == 0.0;
    __asm { fsel      f0, f10, f11, f0 }
    v14 = (float)((float)1.0 / (float)_FP0);
    v15 = (float)(v26 * (float)v14);
    v26 = v26 * (float)v14;
    v16 = (float)((float)v14 * v27);
    v27 = (float)v14 * v27;
    v17 = (float)((float)v14 * (float)0.0);
    v28 = (float)v14 * (float)0.0;
    if (v12)
    {
        v9 = 0;
        G_GetPlayerViewDirection(v8, &v26, 0, 0);
        v17 = v28;
        v16 = v27;
        v15 = v26;
    }
    v18 = 0;
    result = 0;
    if (level.actorCorpseCount <= 0)
        goto LABEL_24;
    p_entnum = &g_scr_data.actorCorpseInfo[0].entnum;
    do
    {
        if (*p_entnum == -1)
        {
            if (reuse)
                return result;
        }
        else
        {
            v21 = &level.gentities[*p_entnum];
            v18 = 1;
            v22 = (float)(v21->r.currentOrigin[2] - v31);
            if (v9)
                v22 = 0.0;
            v23 = (float)((float)((float)v22 * (float)v17)
                + (float)((float)((float)(v21->r.currentOrigin[0] - v29) * (float)v15)
                    + (float)((float)(v21->r.currentOrigin[1] - v30) * (float)v16)));
            v24 = (float)((float)((float)v22 * (float)v22)
                + (float)((float)((float)(v21->r.currentOrigin[1] - v30) * (float)(v21->r.currentOrigin[1] - v30))
                    + (float)((float)(v21->r.currentOrigin[0] - v29) * (float)(v21->r.currentOrigin[0] - v29))));
            if (v23 < 0.0 && v5 < v24)
            {
                v5 = v24;
                v2 = result;
            }
            if ((float)((float)v23 * (float)v23) <= (double)(float)((float)v24 * (float)0.5) && v6 < v24)
            {
                v6 = v24;
                v3 = result;
            }
            if (v7 < v24)
            {
                v7 = v24;
                v4 = result;
            }
        }
        ++result;
        p_entnum += 8;
    } while (result < level.actorCorpseCount);
    if (v18)
    {
        if (v5 == -1.0)
        {
            v25 = v3;
            if (v6 == -1.0)
                v25 = v4;
        }
        else
        {
            v25 = v2;
        }
        G_FreeEntity(&level.gentities[g_scr_data.actorCorpseInfo[v25].entnum]);
        result = v25;
        g_scr_data.actorCorpseInfo[v25].entnum = -1;
    }
    else
    {
    LABEL_24:
        if (reuse)
        {
            if (level.actorCorpseCount)
                MyAssertHandler(
                    "c:\\trees\\cod3\\cod3src\\src\\game\\actor_corpse.cpp",
                    145,
                    0,
                    "%s",
                    "!reuse || !level.actorCorpseCount");
        }
        return -1;
    }
    return result;
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
    gentity_s *v6; // r30
    int v7; // r27
    int *v8; // r29
    int v9; // r31
    unsigned int entnum; // r7
    _BYTE v11[24]; // [sp+50h] [-90h] BYREF
    char v12; // [sp+68h] [-78h] BYREF

    v0 = v11;
    v1 = 8;
    do
    {
        *v0++ = 0x800000000LL;
        --v1;
    } while (v1);
    v2 = 0;
    v3 = 0;
    v4 = v11;
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
        v6 = G_Find(0, 284, scr_const.player);
        if (!v6)
            MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\actor_corpse.cpp", 265, 0, "%s", "ent");
        if (!v6->sentient)
            MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\actor_corpse.cpp", 266, 0, "%s", "ent->sentient");
        Sentient_GetEyePosition(v6->sentient, playerEyePos);
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
    float *iEntNum,
    int iClipMask,
    float *vOrigin,
    double fYaw,
    float *pfPitch,
    float *pfRoll,
    float *pfHeight,
    float *a8)
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
    float v26[4]; // [sp+70h] [-70h] BYREF
    float v27[4]; // [sp+80h] [-60h] BYREF

    if (!pfRoll)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\actor_corpse.cpp", 408, 0, "%s", "pfPitch");
    v15 = vOrigin[2];
    v20 = *vOrigin;
    v23 = v20;
    v21 = vOrigin[1];
    v22 = (float)v15 + (float)30.0;
    v24 = v21;
    v25 = (float)v15 - (float)30.0;
    YawVectors(fYaw, iEntNum, v26);
    v16 = Actor_SetBodyPlantAngle((int)iEntNum, iClipMask, vOrigin, vOrigin, v26, pfRoll);
    if (pfHeight)
    {
        if (__fabs(*pfRoll) >= 30.0)
            *pfHeight = 0.0;
        else
            v16 = (float)((float)(Actor_SetBodyPlantAngle((int)iEntNum, iClipMask, vOrigin, vOrigin, v27, pfHeight)
                + (float)v16)
                * (float)0.5);
    }
    if (a8)
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
        *a8 = v17;
    }
}

float __cdecl Actor_Orient_LerpWithLimit(double current, double newValue, double delta, double rate)
{
    double v5; // fp1

    if (__fabs(delta) <= rate)
    {
        v5 = newValue;
    }
    else
    {
        __asm { fsel      f0, f3, f0, f13 }
        v5 = (float)((float)((float)_FP0 * (float)rate) + (float)current);
    }
    return *((float *)&v5 + 1);
}

void __cdecl Actor_OrientCorpseToGround(gentity_s *self, int bLerp, int a3, float *a4)
{
    int eType; // r11
    actor_prone_info_s *p_proneInfo; // r30
    double v8; // fp1
    float *number; // r3
    float *currentOrigin; // r5
    unsigned int v11; // r4
    double v12; // fp29
    double v15; // fp0
    double v16; // fp29
    double fWaistPitch; // fp2
    double v18; // fp1
    double v21; // fp13
    double fBodyHeight; // fp0
    double v23; // fp11
    float v26; // [sp+50h] [-50h] BYREF
    float v27; // [sp+54h] [-4Ch] BYREF
    float v28[4]; // [sp+58h] [-48h] BYREF

    eType = self->s.eType;
    if (eType != 16 && eType != 14)
        MyAssertHandler(
            "c:\\trees\\cod3\\cod3src\\src\\game\\actor_corpse.cpp",
            472,
            0,
            "%s",
            "(self->s.eType == ET_ACTOR_CORPSE) || (self->s.eType == ET_ACTOR)");
    if (self->s.eType == 16)
        p_proneInfo = &g_scr_data.actorCorpseInfo[G_GetActorCorpseIndex(self)].proneInfo;
    else
        p_proneInfo = &self->actor->ProneInfo;
    if (p_proneInfo->bCorpseOrientation)
    {
        v8 = self->r.currentAngles[1];
        number = (float *)self->s.number;
        currentOrigin = self->r.currentOrigin;
        v11 = self->clipmask & 0xFDFF3FFF;
        if (bLerp)
        {
            Actor_GetBodyPlantAngles(number, v11, currentOrigin, v8, a4, &v26, &v27, v28);
            v12 = v26;
            _FP1 = AngleSubtract(v26, p_proneInfo->fTorsoPitch);
            if (__fabs(_FP1) <= 6.0)
            {
                v15 = v12;
            }
            else
            {
                __asm { fsel      f13, f1, f31, f30 }
                v15 = (float)((float)((float)_FP13 * (float)6.0) + p_proneInfo->fTorsoPitch);
            }
            v16 = v27;
            fWaistPitch = p_proneInfo->fWaistPitch;
            v18 = v27;
            p_proneInfo->fTorsoPitch = v15;
            _FP1 = AngleSubtract(v18, fWaistPitch);
            if (__fabs(_FP1) <= 6.0)
            {
                v21 = v16;
            }
            else
            {
                __asm { fsel      f13, f1, f31, f30 }
                v21 = (float)((float)((float)_FP13 * 6.0) + p_proneInfo->fWaistPitch);
            }
            fBodyHeight = p_proneInfo->fBodyHeight;
            v23 = v28[0];
            p_proneInfo->fWaistPitch = v21;
            _FP13 = (float)((float)v23 - (float)fBodyHeight);
            if (__fabs(_FP13) <= 0.60000002)
            {
                p_proneInfo->fBodyHeight = v23;
            }
            else
            {
                __asm { fsel      f13, f13, f31, f30 }
                p_proneInfo->fBodyHeight = (float)((float)_FP13 * 0.60000002) + (float)fBodyHeight;
            }
        }
        else
        {
            Actor_GetBodyPlantAngles(
                number,
                v11,
                currentOrigin,
                v8,
                a4,
                &p_proneInfo->fTorsoPitch,
                &p_proneInfo->fWaistPitch,
                &p_proneInfo->fBodyHeight);
        }
    }
}

void __cdecl Actor_OrientPitchToGround(gentity_s *self, int bLerp, int a3, float *a4)
{
    actor_prone_info_s *p_ProneInfo; // r30
    double v7; // fp1
    float *number; // r3
    unsigned int v9; // r4
    float *currentOrigin; // r5
    double v11; // fp31
    double v14; // fp13
    double fBodyHeight; // fp0
    double v16; // fp9
    float v19; // [sp+50h] [-30h] BYREF
    float v20; // [sp+54h] [-2Ch] BYREF

    if (self->s.eType != 14)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\actor_corpse.cpp", 513, 0, "%s", "self->s.eType == ET_ACTOR");
    p_ProneInfo = &self->actor->ProneInfo;
    if (self->actor->ProneInfo.orientPitch)
    {
        v7 = self->r.currentAngles[1];
        number = (float *)self->s.number;
        v9 = self->clipmask & 0xFDFF3FFF;
        currentOrigin = self->r.currentOrigin;
        if (bLerp)
        {
            Actor_GetBodyPlantAngles(number, v9, currentOrigin, v7, a4, &v19, 0, &v20);
            v11 = v19;
            _FP1 = AngleSubtract(v19, p_ProneInfo->fTorsoPitch);
            if (__fabs(_FP1) <= 6.0)
            {
                v14 = v11;
            }
            else
            {
                __asm { fsel      f13, f1, f11, f10 }
                v14 = (float)((float)((float)_FP13 * (float)6.0) + p_ProneInfo->fTorsoPitch);
            }
            fBodyHeight = p_ProneInfo->fBodyHeight;
            v16 = v20;
            p_ProneInfo->fTorsoPitch = v14;
            _FP13 = (float)((float)v16 - (float)fBodyHeight);
            if (__fabs(_FP13) <= 0.60000002)
            {
                p_ProneInfo->fBodyHeight = v16;
            }
            else
            {
                __asm { fsel      f13, f13, f11, f10 }
                p_ProneInfo->fBodyHeight = (float)((float)_FP13 * 0.60000002) + (float)fBodyHeight;
            }
        }
        else
        {
            Actor_GetBodyPlantAngles(
                number,
                v9,
                currentOrigin,
                v7,
                a4,
                &self->actor->ProneInfo.fTorsoPitch,
                0,
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
    v8 = v25;
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
    self->s.eType = 16;
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
    v21 = v25;
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
        Actor_OrientCorpseToGround(self, 0, v19, v18);
    }
    SV_LinkEntity(self);
    return 1;
}

XAnimTree_s *__cdecl G_GetActorCorpseAnimTree(gentity_s *ent)
{
    return g_scr_data.actorCorpseInfo[G_GetActorCorpseIndex(ent)].tree;
}

