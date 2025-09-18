#ifndef KISAK_SP
#error This file is for SinglePlayer only
#endif

#include "cg_snapshot.h"
#include <EffectsCore/fx_system.h>
#include <ragdoll/ragdoll.h>
#include "cg_ents.h"
#include <aim_assist/aim_assist.h>
#include "cg_main.h"
#include "cg_actors.h"
#include <gfx_d3d/r_dpvs.h>
#include "cg_view.h"
#include <game/actor.h>
#include <game/g_local.h>
#include <gfx_d3d/r_model.h>
#include "cg_servercmds.h"

unsigned int g_centInPrevSnapshot[68]{ 0 };
bool g_clientDirty[MAX_GENTITIES];

void __cdecl CG_ShutdownEntity(int localClientNum, centity_s *cent)
{
    int oldEType; // r11
    FxEffect *effect; // r4
    trajectory_t *p_pos; // r29
    int ragdollHandle; // r3
    int physObjId; // r4

    oldEType = cent->oldEType;
    if (oldEType == 8 || oldEType == 7)
    {
        effect = cent->pose.fx.effect;
        if (effect)
        {
            FX_ThroughWithEffect(localClientNum, effect);
            cent->pose.fx.effect = 0;
        }
    }
    if (cent->pose.isRagdoll
        || (p_pos = &cent->currentState.pos, (unsigned __int8)Com_IsRagdollTrajectory(&cent->currentState.pos)))
    {
        ragdollHandle = cent->pose.ragdollHandle;
        if (ragdollHandle)
            Ragdoll_Remove(ragdollHandle);
        p_pos = &cent->currentState.pos;
        cent->pose.isRagdoll = 0;
        cent->pose.ragdollHandle = 0;
        cent->currentState.pos.trType = TR_STATIONARY;
        cent->currentState.apos.trType = TR_STATIONARY;
    }
    physObjId = cent->pose.physObjId;
    if (physObjId && physObjId != -1)
        goto LABEL_14;
    if (p_pos->trType != TR_PHYSICS)
        return;
    if (physObjId != -1)
    {
    LABEL_14:
        if (physObjId)
            Phys_ObjDestroy(PHYS_WORLD_FX, (dxBody*)physObjId);
    }
    p_pos->trType = TR_STATIONARY;
    cent->currentState.apos.trType = TR_STATIONARY;
    cent->pose.physObjId = 0;
}

void __cdecl CG_InitEntity(centity_s *cent)
{
    int eType; // r11

    eType = cent->nextState.eType;
    if (eType == 8 || eType == 7)
    {
        cent->pose.actor.proneType = 0;
        cent->pose.fx.effect = 0;
    }
}

void __cdecl CG_ResetEntity(int localClientNum, centity_s *cent)
{
    int eType; // r11
    unsigned __int8 v5; // r11
    int time; // r29
    int number; // r4
    int v8; // r3
    const DObj_s *ClientDObj; // r4
    float *v10; // r11
    unsigned __int8 *wheelBoneIndex; // r11
    int v12; // ctr
    const DObj_s *v13; // r4

    if (cent->currentState.useCount != cent->nextState.lerp.useCount)
    {
        cent->previousEventSequence = 0;
        cent->lightingOrigin[0] = 0.0;
        cent->lightingOrigin[1] = 0.0;
        cent->lightingOrigin[2] = 0.0;
        CG_ShutdownEntity(localClientNum, cent);
        eType = cent->nextState.eType;
        if (eType == 8 || eType == 7)
        {
            cent->pose.actor.proneType = 0;
            cent->pose.fx.effect = 0;
        }
        CG_ClearUnion(localClientNum, cent);
    }
    AimAssist_ClearEntityReference(localClientNum, cent->nextState.number);
    memcpy(&cent->currentState, &cent->nextState.lerp, sizeof(cent->currentState));
    v5 = cent->nextState.eType;
    cent->bTrailMade = 0;
    cent->pose.cullIn = 0;
    cent->oldEType = v5;
    if (localClientNum)
        MyAssertHandler(
            "c:\\trees\\cod3\\cod3src\\src\\cgame\\cg_local.h",
            910,
            0,
            "%s\n\t(localClientNum) = %i",
            "(localClientNum == 0)",
            localClientNum);
    time = cgArray[0].time;
    BG_EvaluateTrajectory(&cent->nextState.lerp.pos, cgArray[0].time, cent->pose.origin);
    BG_EvaluateTrajectory(&cent->nextState.lerp.apos, time, cent->pose.angles);
    number = cent->nextState.number;
    cent->pose.eType = cent->nextState.eType;
    CG_UnlinkEntity(localClientNum, number);
    switch (cent->nextState.eType)
    {
    case 5u:
        if (cent->nextState.solid != 0xFFFFFF)
            goto LABEL_11;
        CG_UpdateBModelWorldBounds(localClientNum, cent, 1);
        return;
    case 0xAu:
        v8 = cent->nextState.number;
        cent->pose.actor.proneType = 0;
        cent->pose.turret.tag_aim = -2;
        cent->pose.turret.tag_aim_animated = -2;
        cent->pose.turret.tag_flash = -2;
        cent->pose.turret.playerUsing = 1;
        ClientDObj = Com_GetClientDObj(v8, localClientNum);
        if (ClientDObj)
            CG_mg42_PreControllers(localClientNum, ClientDObj, cent);
        goto LABEL_11;
    case 0xBu:
    case 0xDu:
        cent->pose.vehicle.tag_body = -2;
        cent->pose.vehicle.tag_turret = -2;
        cent->pose.vehicle.tag_barrel = -2;
        wheelBoneIndex = cent->pose.vehicle.wheelBoneIndex;
        v12 = 6;
        do
        {
            *wheelBoneIndex++ = -2;
            --v12;
        } while (v12);
        v13 = Com_GetClientDObj(cent->nextState.number, localClientNum);
        if (v13)
            CG_Vehicle_PreControllers(localClientNum, v13, cent);
        goto LABEL_11;
    case 0xEu:
    case 0x10u:
        CG_Actor_PreControllers(cent);
        goto LABEL_11;
    default:
    LABEL_11:
        v10 = cg_entityOriginArray[localClientNum][cent->nextState.number];
        *v10 = 131072.0;
        v10[1] = 131072.0;
        v10[2] = 131072.0;
        return;
    }
}

void __cdecl CG_SetInitialSnapshot(int localClientNum)
{
    snapshot_s *nextSnap; // r31
    int *p_numEntities; // r28
    int v4; // r7
    int v5; // r30
    int *entityNums; // r31
    centity_s *Entity; // r3
    float v8[4]; // [sp+50h] [-70h] BYREF
    float v9[8][3]; // [sp+60h] [-60h] BYREF

    memset(g_centInPrevSnapshot, 0, sizeof(g_centInPrevSnapshot));
    R_InitSceneData(localClientNum);
    if (localClientNum)
        MyAssertHandler(
            "c:\\trees\\cod3\\cod3src\\src\\cgame\\cg_local.h",
            910,
            0,
            "%s\n\t(localClientNum) = %i",
            "(localClientNum == 0)",
            localClientNum);
    if (cgArray[0].snap)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\cgame\\cg_snapshot.cpp", 184, 0, "%s", "!cgameGlob->snap");
    nextSnap = cgArray[0].nextSnap;
    if (!cgArray[0].nextSnap)
    {
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\cgame\\cg_snapshot.cpp", 185, 0, "%s", "cgameGlob->nextSnap");
        nextSnap = cgArray[0].nextSnap;
    }
    cgArray[0].loaded = 1;
    cgArray[0].snap = nextSnap;
    cgArray[0].clientNum = nextSnap->ps.clientNum;
    cgArray[0].time = nextSnap->serverTime;
    cgArray[0].oldTime = cgArray[0].time;
    v8[0] = nextSnap->ps.origin[0];
    v8[1] = nextSnap->ps.origin[1];
    v8[2] = nextSnap->ps.viewHeightCurrent + nextSnap->ps.origin[2];
    AnglesToAxis(nextSnap->ps.viewangles, v9);
    SND_SetListener(localClientNum, nextSnap->ps.clientNum, v8, v9);
    CG_ClearEntityCollWorld(localClientNum);
    CG_InitView(localClientNum);
    p_numEntities = &nextSnap->numEntities;
    cgArray[0].snap = 0;
    v4 = *p_numEntities;
    if (*p_numEntities < 0 || v4 > 2048)
        MyAssertHandler(
            "c:\\trees\\cod3\\cod3src\\src\\cgame\\cg_snapshot.cpp",
            208,
            0,
            "snap->numEntities not in [0, MAX_ENTITIES_IN_SNAPSHOT]\n\t%i not in [%i, %i]",
            v4,
            0,
            2048);
    v5 = 0;
    if (*p_numEntities > 0)
    {
        entityNums = nextSnap->entityNums;
        do
        {
            Entity = CG_GetEntity(localClientNum, *entityNums);
            ++v5;
            ++entityNums;
            Entity->currentState.useCount = Entity->nextState.lerp.useCount;
        } while (v5 < *p_numEntities);
    }
}

int __cdecl CG_DObjCloneToBuffer(int localClientNum, centity_s *cent, const XAnimTree_s *serverTree)
{
    int eType; // r11
    int result; // r3
    XAnimTree_s *SmallTree; // r30
    XAnim_s *Anims; // r3
    DObj_s *v10; // r3
    float *v11; // r11

    if (!serverTree)
    {
        SmallTree = 0;
        goto LABEL_11;
    }
    eType = cent->nextState.eType;
    if (eType != 14 && eType != 16 || cent->nextState.lerp.u.actor.species)
    {
        Anims = XAnimGetAnims(serverTree);
        SmallTree = Com_XAnimCreateSmallTree(Anims);
        if (!SmallTree)
            MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\cgame\\cg_snapshot.cpp", 241, 0, "%s", "tree");
        goto LABEL_9;
    }
    result = (int)G_AllocAnimClientTree();
    SmallTree = (XAnimTree_s *)result;
    if (result)
    {
    LABEL_9:
        XAnimCloneClientAnimTree(serverTree, SmallTree);
    LABEL_11:
        v10 = Com_DObjCloneToBuffer(cent->nextState.number);
        DObjSetTree(v10, SmallTree);
        result = 1;
        v11 = cg_entityOriginArray[localClientNum][cent->nextState.number];
        *v11 = 131072.0;
        v11[1] = 131072.0;
        v11[2] = 131072.0;
    }
    return result;
}

void __cdecl CG_FreeTree(XAnimTree_s *tree, centity_s *cent)
{
    int oldEType; // r11

    oldEType = cent->oldEType;
    if ((oldEType == 14 || oldEType == 16) && !cent->currentState.u.actor.species)
        G_FreeAnimClientTree(tree);
    else
        Com_XAnimFreeSmallTree(tree);
}

void __cdecl CG_UpdateSnapshotNum(int localClientNum)
{
    int v2; // r10
    int n; // [sp+50h] [-20h] BYREF

    if (localClientNum)
        MyAssertHandler(
            "c:\\trees\\cod3\\cod3src\\src\\cgame\\cg_local.h",
            910,
            0,
            "%s\n\t(localClientNum) = %i",
            "(localClientNum == 0)",
            localClientNum);
    CL_GetCurrentSnapshotNumber(localClientNum, &n, &cgArray[0].latestSnapshotTime);
    v2 = n;
    if (n != cgArray[0].latestSnapshotNum)
    {
        if (n < cgArray[0].latestSnapshotNum)
        {
            Com_Error(ERR_DROP, "CG_UpdateSnapshotNum: n < cgameGlob->latestSnapshotNum");
            v2 = n;
        }
        cgArray[0].latestSnapshotNum = v2;
    }
}

snapshot_s *__cdecl CG_ReadNextSnapshot(int localClientNum)
{
    int v2; // r3
    snapshot_s *v3; // r31

    if (localClientNum)
        MyAssertHandler(
            "c:\\trees\\cod3\\cod3src\\src\\cgame\\cg_local.h",
            910,
            0,
            "%s\n\t(localClientNum) = %i",
            "(localClientNum == 0)",
            localClientNum);
    v2 = localClientNum;
    v3 = &cgArray[0].activeSnapshots[cgArray[0].activeSnapshots == cgArray[0].snap];
    CL_GetSnapshot(v2, v3);
    return v3;
}

// Debug function
void __cdecl CG_CheckSnapshot(int localClientNum, const char *caller)
{
    int numEntities; // r11
    int *entityNums; // r10
    int v6; // r11
    int *v7; // r10
    int i; // r31
    const char *v9; // r3
    unsigned char v10[MAX_GENTITIES]; // [sp+50h] [-8E0h] BYREF

    if (localClientNum)
        MyAssertHandler(
            "c:\\trees\\cod3\\cod3src\\src\\cgame\\cg_local.h",
            910,
            0,
            "%s\n\t(localClientNum) = %i",
            "(localClientNum == 0)",
            localClientNum);

    memset(v10, 0, MAX_GENTITIES);

    if (cgArray[0].snap)
    {
        numEntities = cgArray[0].snap->numEntities;
        if (numEntities > 0)
        {
            entityNums = cgArray[0].snap->entityNums;
            do
            {
                --numEntities;
                v10[*entityNums++] = 1;
            } while (numEntities);
        }
    }
    if (cgArray[0].nextSnap)
    {
        v6 = cgArray[0].nextSnap->numEntities;
        if (v6 > 0)
        {
            v7 = cgArray[0].nextSnap->entityNums;
            do
            {
                --v6;
                v10[*v7++] = 1;
            } while (v6);
        }
    }
    for (i = 0; i < 2176; ++i)
    {
        if (!v10[i])
        {
            if (g_clientDirty[i])
                MyAssertHandler(
                    "c:\\trees\\cod3\\cod3src\\src\\cgame\\cg_snapshot.cpp",
                    378,
                    0,
                    "%s\n\t(entnum) = %i",
                    "(!g_clientDirty[entnum])",
                    i);
            if (Com_GetClientDObj(i, localClientNum))
            {
                v9 = va("%s: entnum %d client %d", caller, i, localClientNum);
                MyAssertHandler(
                    "c:\\trees\\cod3\\cod3src\\src\\cgame\\cg_snapshot.cpp",
                    379,
                    0,
                    "%s\n\t%s",
                    "!Com_GetClientDObj( entnum, localClientNum )",
                    v9);
            }
        }
    }
}

void __cdecl CG_ServerDObjClean(int entnum)
{
    Com_ServerDObjClean(entnum);
    g_clientDirty[entnum] = 1;
}

void __cdecl CG_SetNextSnap(int localClientNum)
{
    snapshot_s *snap; // r18
    int *p_numEntities; // r24
    int v4; // r25
    int *entityNums; // r26
    int v6; // r31
    bool v7; // r8
    centity_s *Entity; // r28
    DObj_s *ClientDObj; // r29
    centity_s *v10; // r28
    XAnimTree_s *Tree; // r29
    int oldEType; // r11
    DObj_s *v13; // r3
    snapshot_s *nextSnap; // r26
    int v15; // r25
    int *v16; // r28
    int v17; // r31
    centity_s *v18; // r29
    int v19; // r9
    unsigned int v20; // r11
    int v21; // r10
    int v22; // r11
    int eFlags; // r10
    centity_s *v24; // r31
    bool nextValid; // r10
    int v26; // r31
    int *p_duration; // r28
    const DObj_s *v28; // r3
    XAnimTree_s *v29; // r29
    int v30; // r11
    DObj_s *v31; // r3
    DObj_s *v32; // r3
    FxMarkDObjUpdateContext v33; // [sp+60h] [-2C0h] BYREF
    unsigned int centInPrevSnapshot[68]; // [sp+170h] [-1B0h] BYREF

    cg_s *cgameGlob = CG_GetLocalClientGlobals(localClientNum);

    iassert(cgameGlob->createdNextSnap);
    cgameGlob->createdNextSnap = 0;
    CG_CheckSnapshot(localClientNum, "CG_SetNextSnap-pre");
    memset(centInPrevSnapshot, 0, sizeof(centInPrevSnapshot));

    snap = cgArray[0].snap;
    if (cgArray[0].snap)
    {
        p_numEntities = &cgArray[0].snap->numEntities;
        v4 = 0;
        if (cgArray[0].snap->numEntities > 0)
        {
            entityNums = cgArray[0].snap->entityNums;
            do
            {
                v6 = *entityNums;
                v7 = g_clientDirty[*entityNums];
                centInPrevSnapshot[*entityNums >> 5] |= 0x80000000 >> (*entityNums & 0x1F);
                if (v7)
                {
                    g_clientDirty[v6] = 0;
                    Entity = CG_GetEntity(localClientNum, v6);
                    ClientDObj = Com_GetClientDObj(v6, localClientNum);
                    FX_MarkEntUpdateBegin(
                        &v33,
                        ClientDObj,
                        Entity->nextState.solid == 0xFFFFFF,
                        Entity->nextState.index.brushmodel);
                    v10 = CG_GetEntity(localClientNum, v6);
                    if (ClientDObj)
                    {
                        Tree = DObjGetTree(ClientDObj);
                        Com_SafeClientDObjFree(v6, localClientNum);
                        if (Tree)
                        {
                            oldEType = v10->oldEType;
                            if ((oldEType == 14 || oldEType == 16) && !v10->currentState.u.actor.species)
                                G_FreeAnimClientTree(Tree);
                            else
                                Com_XAnimFreeSmallTree(Tree);
                        }
                    }
                    Com_DObjCloneFromBuffer(v6);
                    v13 = Com_GetClientDObj(v6, localClientNum);
                    FX_MarkEntUpdateEnd(
                        &v33,
                        localClientNum,
                        v6,
                        v13,
                        v10->nextState.solid == 0xFFFFFF,
                        v10->nextState.index.brushmodel);
                }
                ++v4;
                ++entityNums;
            } while (v4 < *p_numEntities);
        }
    }
    nextSnap = cgArray[0].nextSnap;
    cgArray[0].playerTeleported = 0;
    if (cgArray[0].nextSnap)
    {
        if (nextSnap->serverTime != G_GetServerSnapTime())
            MyAssertHandler(
                "c:\\trees\\cod3\\cod3src\\src\\cgame\\cg_snapshot.cpp",
                646,
                0,
                "%s",
                "nextSnap->serverTime == G_GetServerSnapTime()");
        if (!snap)
        {
            cgArray[0].snap = nextSnap;
            snap = nextSnap;
            cgArray[0].playerTeleported = 1;
        }
        CG_SetFrameInterpolation(localClientNum);
        v15 = 0;
        if (nextSnap->numEntities > 0)
        {
            v16 = nextSnap->entityNums;
            do
            {
                v17 = *v16;
                v18 = CG_GetEntity(localClientNum, *v16);
                if (!v18->nextValid)
                    MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\cgame\\cg_snapshot.cpp", 662, 0, "%s", "cent->nextValid");
                v19 = v17 >> 5;
                v20 = 0x80000000 >> (v17 & 0x1F);
                v21 = centInPrevSnapshot[v19];
                //if ((_cntlzw(v21 & v20) & 0x20) != 0)
                if (((v21 & v20) == 0))
                {
                    if (g_clientDirty[v17])
                    {
                        g_clientDirty[v17] = 0;
                        v31 = Com_GetClientDObj(v17, localClientNum);
                        FX_MarkEntUpdateBegin(&v33, v31, v18->nextState.solid == 0xFFFFFF, v18->nextState.index.brushmodel);
                        Com_DObjCloneFromBuffer(v17);
                        v32 = Com_GetClientDObj(v17, localClientNum);
                        FX_MarkEntUpdateEnd(
                            &v33,
                            localClientNum,
                            v17,
                            v32,
                            v18->nextState.solid == 0xFFFFFF,
                            v18->nextState.index.brushmodel);
                    }
                }
                else
                {
                    v22 = v21 & ~v20;
                    eFlags = v18->currentState.eFlags;
                    centInPrevSnapshot[v19] = v22;
                    if ((((unsigned __int8)v18->nextState.lerp.eFlags ^ (unsigned __int8)eFlags) & 2) == 0
                        && v18->currentState.useCount == v18->nextState.lerp.useCount)
                    {
                        goto LABEL_31;
                    }
                }
                CG_ResetEntity(localClientNum, v18);
            LABEL_31:
                ++v15;
                ++v16;
            } while (v15 < nextSnap->numEntities);
        }
        v24 = CG_GetEntity(localClientNum, nextSnap->ps.clientNum);
        nextValid = v24->nextValid;
        v24->nextState.number = nextSnap->ps.clientNum;
        if (!nextValid)
            MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\cgame\\cg_snapshot.cpp", 689, 0, "%s", "cent->nextValid");
        if (snap == nextSnap
            || (((unsigned __int8)v24->nextState.lerp.eFlags ^ (unsigned __int8)v24->currentState.eFlags) & 2) != 0)
        {
            memcpy(&snap->ps, &nextSnap->ps, sizeof(snap->ps));
            CG_ResetEntity(localClientNum, v24);
            cgArray[0].predictedError[0] = 0.0;
            cgArray[0].predictedError[1] = 0.0;
            cgArray[0].playerTeleported = 1;
            cgArray[0].predictedError[2] = 0.0;
        }
    }
    v26 = 0;
    p_duration = &cg_entitiesArray[0][0].currentState.u.earthquake.duration;
    do
    {
        if (((0x80000000 >> (v26 & 0x1F)) & centInPrevSnapshot[v26 >> 5]) != 0)
        {
            if (localClientNum)
                MyAssertHandler(
                    "c:\\trees\\cod3\\cod3src\\src\\cgame\\cg_local.h",
                    931,
                    0,
                    "%s\n\t(localClientNum) = %i",
                    "(localClientNum == 0)",
                    localClientNum);
            if ((unsigned int)v26 >= 0x880)
                MyAssertHandler(
                    "c:\\trees\\cod3\\cod3src\\src\\cgame\\cg_local.h",
                    932,
                    0,
                    "entityIndex doesn't index MAX_GENTITIES\n\t%i not in [0, %i)",
                    v26,
                    2176);
            CG_ShutdownEntity(localClientNum, (centity_s *)(p_duration - 42));
            FX_MarkEntDetachAll(localClientNum, v26);
            v28 = Com_GetClientDObj(v26, localClientNum);
            if (v28)
            {
                v29 = DObjGetTree(v28);
                Com_SafeClientDObjFree(v26, localClientNum);
                if (v29)
                {
                    v30 = *((unsigned __int8 *)p_duration + 187);
                    if ((v30 == 14 || v30 == 16) && !*(_BYTE *)p_duration)
                        G_FreeAnimClientTree(v29);
                    else
                        Com_XAnimFreeSmallTree(v29);
                }
            }
        }
        p_duration += 93;
        ++v26;
    } while ((int)p_duration < (int)&cg_subtitleWidthStandard);
    CG_CheckSnapshot(localClientNum, "CG_SetNextSnap-post");
}

void __cdecl CG_ProcessNextSnap(int localClientNum)
{
    unsigned int *v1; // r27
    int v3; // r25
    int v4; // r23
    unsigned int v5; // r31
    DWORD i; // r11
    unsigned int v7; // r29
    unsigned int v8; // r30
    snapshot_s *nextSnap; // r23
    snapshot_s *snap; // r17
    int viewmodelIndex; // r11
    const char *ConfigString; // r3
    const char *v13; // r31
    XModel *v14; // r3
    int v15; // r30
    int *p_numEntities; // r26
    int *entityNums; // r29
    unsigned int v18; // r31

    v1 = g_centInPrevSnapshot;
    v3 = 0;
    v4 = 68;
    do
    {
        v5 = *v1;
        //for (i = _cntlzw(*v1); i < 0x20; i = _cntlzw(v5))
        //{
        //    v7 = 0x80000000 >> i;
        //    v8 = v3 + i;
        //    if (((0x80000000 >> i) & v5) == 0)
        //        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\cgame\\cg_snapshot.cpp", 759, 0, "%s", "bits & bit");
        //    *v1 = 0;
        //    v5 &= ~v7;
        //    R_UnlinkEntity(localClientNum, v8);
        //    CG_UnlinkEntity(localClientNum, v8);
        //}
        while (_BitScanReverse(&i, v5))
        {
            unsigned int v7 = 0x80000000 >> (31 - i);  // Same as 1 << i
            unsigned int v8 = v3 + (31 - i);           // Match _cntlzw bit index (MSB-first)

            if ((v5 & v7) == 0)
                MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\cgame\\cg_snapshot.cpp", 759, 0, "%s", "bits & bit");

            *v1 = 0;
            v5 &= ~v7;

            R_UnlinkEntity(localClientNum, v8);
            CG_UnlinkEntity(localClientNum, v8);
        }
        --v4;
        v3 += 32;
        ++v1;
    } while (v4);
    if (localClientNum)
        MyAssertHandler(
            "c:\\trees\\cod3\\cod3src\\src\\cgame\\cg_local.h",
            910,
            0,
            "%s\n\t(localClientNum) = %i",
            "(localClientNum == 0)",
            localClientNum);
    CG_CheckSnapshot(localClientNum, "CG_ProcessNextSnap - pre");
    nextSnap = cgArray[0].nextSnap;
    if (!cgArray[0].nextSnap)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\cgame\\cg_snapshot.cpp", 776, 0, "%s", "nextSnap");
    snap = cgArray[0].snap;
    if (!cgArray[0].snap)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\cgame\\cg_snapshot.cpp", 779, 0, "%s", "snap");
    CG_UpdateViewOffset(localClientNum);
    CG_ExecuteNewServerCommands(localClientNum, nextSnap->serverCommandSequence);
    CG_CheckOpenWaitingScriptMenu();
    viewmodelIndex = nextSnap->ps.viewmodelIndex;
    if (viewmodelIndex > 0)
    {
        ConfigString = CL_GetConfigString(localClientNum, viewmodelIndex + 1155);
        v13 = ConfigString;
        if (!ConfigString || !*ConfigString)
            MyAssertHandler(
                "c:\\trees\\cod3\\cod3src\\src\\cgame\\cg_snapshot.cpp",
                793,
                0,
                "%s",
                "modelName && modelName[0]");
        v14 = R_RegisterModel(v13);
        CG_UpdateHandViewmodels(localClientNum, v14);
    }
    CG_UpdateWeaponViewmodels(localClientNum);
    if (cgArray[0].snap != cgArray[0].nextSnap)
    {
        v15 = 0;
        p_numEntities = &nextSnap->numEntities;
        if (*p_numEntities > 0)
        {
            entityNums = nextSnap->entityNums;
            do
            {
                v18 = *entityNums;
                if (localClientNum)
                    MyAssertHandler(
                        "c:\\trees\\cod3\\cod3src\\src\\cgame\\cg_local.h",
                        931,
                        0,
                        "%s\n\t(localClientNum) = %i",
                        "(localClientNum == 0)",
                        localClientNum);
                if (v18 >= 0x880)
                    MyAssertHandler(
                        "c:\\trees\\cod3\\cod3src\\src\\cgame\\cg_local.h",
                        932,
                        0,
                        "entityIndex doesn't index MAX_GENTITIES\n\t%i not in [0, %i)",
                        v18,
                        2176);
                CG_CheckEvents(localClientNum, &cg_entitiesArray[0][v18]);
                ++v15;
                ++entityNums;
            } while (v15 < *p_numEntities);
        }
        if (cgArray[0].demoType || cg_nopredict->current.enabled)
            CG_TransitionPlayerState(localClientNum, &nextSnap->ps, &snap->ps);
        CG_CheckSnapshot(localClientNum, "CG_ProcessNextSnap - post");
    }
}

void __cdecl CG_CreateNextSnap(int localClientNum, double dtime, int readNext)
{
    snapshot_s *nextSnap; // r18
    double value; // fp0
    int *p_numEntities; // r28
    int v10; // r29
    int *entityNums; // r30
    centity_s *Entity; // r31
    centity_s *v13; // r31
    //snapshot_s *nextSnap; // r25
    int v15; // r28
    int *v16; // r30
    int v17; // r31
    entityState_s *EntityState; // r29
    centity_s *v19; // r31
    centity_s *v20; // r31
    int v21; // r27
    unsigned int *v22; // r28
    unsigned int v23; // r31
    int v24; // r3
    const DObj_s *ServerDObj; // r3
    const DObj_s *v26; // r29
    const XAnimTree_s *Tree; // r30
    int v28; // r26
    int *v29; // r27
    int v30; // r31
    const DObj_s *v31; // r28
    int v32; // r11
    int v33; // r9
    unsigned int v34; // r11
    int v35; // r10
    int v36; // r11
    bool v37; // r10
    const DObj_s *ClientDObjBuffered; // r3
    DObj_s *v39; // r31
    const XAnimTree_s *v40; // r3
    int v41; // r8
    int *v42; // r9
    int v43; // r11
    unsigned int v44; // r10
    unsigned int v45[68]; // [sp+60h] [-1B0h] BYREF

    CG_CheckSnapshot(localClientNum, "CG_CreateNextSnap(pre)");
    if (localClientNum)
        MyAssertHandler(
            "c:\\trees\\cod3\\cod3src\\src\\cgame\\cg_local.h",
            910,
            0,
            "%s\n\t(localClientNum) = %i",
            "(localClientNum == 0)",
            localClientNum);
    if (cgArray[0].createdNextSnap)
    {
        iassert(!readNext);
        CG_SetNextSnap(localClientNum);
        if (cgArray[0].createdNextSnap)
            MyAssertHandler(
                "c:\\trees\\cod3\\cod3src\\src\\cgame\\cg_snapshot.cpp",
                418,
                0,
                "%s",
                "!cgameGlob->createdNextSnap");
    }
    nextSnap = cgArray[0].nextSnap;
    cgArray[0].createdNextSnap = 1;
    cgArray[0].snap = cgArray[0].nextSnap;
    if (sv_znear)
        value = sv_znear->current.value;
    else
        value = 0.0;
    cgArray[0].zNear = value;
    if (cgArray[0].nextSnap)
    {
        p_numEntities = &cgArray[0].nextSnap->numEntities;
        v10 = 0;
        if (cgArray[0].nextSnap->numEntities > 0)
        {
            entityNums = cgArray[0].nextSnap->entityNums;
            do
            {
                Entity = CG_GetEntity(localClientNum, *entityNums);
                if (Entity->currentState.useCount != Entity->nextState.lerp.useCount)
                    MyAssertHandler(
                        "c:\\trees\\cod3\\cod3src\\src\\cgame\\cg_snapshot.cpp",
                        441,
                        0,
                        "%s",
                        "cent->currentState.useCount == cent->nextState.lerp.useCount");
                if (!Entity->nextValid)
                    MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\cgame\\cg_snapshot.cpp", 442, 0, "%s", "cent->nextValid");
                Entity->nextValid = 0;
                memcpy(&Entity->currentState, &Entity->nextState.lerp, sizeof(Entity->currentState));
                ++v10;
                ++entityNums;
                Entity->oldEType = Entity->nextState.eType;
            } while (v10 < *p_numEntities);
        }
        v13 = CG_GetEntity(localClientNum, nextSnap->ps.clientNum);
        if (!v13->nextValid)
            MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\cgame\\cg_snapshot.cpp", 449, 0, "%s", "cent->nextValid");
        v13->nextValid = 0;
        memcpy(&v13->currentState, &v13->nextState.lerp, sizeof(v13->currentState));
        v13->oldEType = v13->nextState.eType;
    }
    if (readNext)
    {
        CG_UpdateSnapshotNum(localClientNum);
        nextSnap = CG_ReadNextSnapshot(localClientNum);
        iassert(nextSnap);
    }
    else
    {
        nextSnap = 0;
    }
    cgArray[0].nextSnap = nextSnap;
    if (nextSnap)
    {
        v15 = 0;
        if (nextSnap->numEntities > 0)
        {
            v16 = nextSnap->entityNums;
            do
            {
                v17 = *v16;
                //EntityState = ;
                v19 = CG_GetEntity(localClientNum, v17);
                memcpy(&v19->nextState, SV_GetEntityState(*v16), sizeof(v19->nextState));
                if (v19->nextValid)
                    MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\cgame\\cg_snapshot.cpp", 477, 0, "%s", "!cent->nextValid");
                v19->nextValid = 1;
                ++v15;
                ++v16;
            } while (v15 < nextSnap->numEntities);
        }
        v20 = CG_GetEntity(localClientNum, nextSnap->ps.clientNum);
        v20->nextState.number = nextSnap->ps.clientNum;
        BG_PlayerStateToEntityState(&nextSnap->ps, &v20->nextState, 0, 0);
        if (v20->nextValid)
            MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\cgame\\cg_snapshot.cpp", 484, 0, "%s", "!cent->nextValid");
        v20->nextValid = 1;
    }
    memset(v45, 0, sizeof(v45));
    if (nextSnap)
    {
        v21 = 0;
        if (nextSnap->numEntities > 0)
        {
            v22 = (unsigned int *)nextSnap->entityNums;
            do
            {
                v23 = *v22;
                v24 = *v22;
                v45[(int)*v22 >> 5] |= 0x80000000 >> (*v22 & 0x1F);
                if (Com_ServerDObjDirty(v24))
                {
                    Com_GetClientDObj(v23, localClientNum);
                    ServerDObj = Com_GetServerDObj(v23);
                    v26 = ServerDObj;
                    if (ServerDObj)
                        Tree = DObjGetTree(ServerDObj);
                    else
                        Tree = 0;
                    if (localClientNum)
                        MyAssertHandler(
                            "c:\\trees\\cod3\\cod3src\\src\\cgame\\cg_local.h",
                            931,
                            0,
                            "%s\n\t(localClientNum) = %i",
                            "(localClientNum == 0)",
                            localClientNum);
                    if (v23 >= 0x880)
                        MyAssertHandler(
                            "c:\\trees\\cod3\\cod3src\\src\\cgame\\cg_local.h",
                            932,
                            0,
                            "entityIndex doesn't index MAX_GENTITIES\n\t%i not in [0, %i)",
                            v23,
                            2176);
                    if (!v26 || CG_DObjCloneToBuffer(localClientNum, &cg_entitiesArray[0][v23], Tree))
                    {
                        Com_ServerDObjClean(v23);
                        g_clientDirty[v23] = 1;
                    }
                }
                ++v21;
                ++v22;
            } while (v21 < nextSnap->numEntities);
        }
    }
    if (nextSnap)
    {
        v28 = 0;
        if (nextSnap->numEntities > 0)
        {
            v29 = nextSnap->entityNums;
            do
            {
                v30 = *v29;
                if (localClientNum)
                    MyAssertHandler(
                        "c:\\trees\\cod3\\cod3src\\src\\cgame\\cg_local.h",
                        931,
                        0,
                        "%s\n\t(localClientNum) = %i",
                        "(localClientNum == 0)",
                        localClientNum);
                if ((unsigned int)v30 >= 0x880)
                    MyAssertHandler(
                        "c:\\trees\\cod3\\cod3src\\src\\cgame\\cg_local.h",
                        932,
                        0,
                        "entityIndex doesn't index MAX_GENTITIES\n\t%i not in [0, %i)",
                        v30,
                        2176);
                if (!cg_entitiesArray[0][v30].nextValid)
                    MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\cgame\\cg_snapshot.cpp", 524, 0, "%s", "cent->nextValid");
                v31 = Com_GetServerDObj(v30);
                v32 = v30 >> 5;
                if (v31)
                {
                    v33 = v32;
                    v35 = v45[v32];
                    v34 = 0x80000000 >> (v30 & 0x1F);
                    if ((v35 & v34) != 0)
                    {
                        v36 = v35 & ~v34;
                        v37 = g_clientDirty[v30];
                        v45[v33] = v36;
                        if (v37)
                            ClientDObjBuffered = Com_GetClientDObjBuffered(v30, localClientNum);
                        else
                            ClientDObjBuffered = Com_GetClientDObj(v30, localClientNum);
                        v39 = (DObj_s *)ClientDObjBuffered;
                        if (ClientDObjBuffered && DObjGetTree(ClientDObjBuffered))
                            DObjTransfer(v31, v39, dtime);
                    }
                    else
                    {
                        if (g_clientDirty[v30])
                            MyAssertHandler(
                                "c:\\trees\\cod3\\cod3src\\src\\cgame\\cg_snapshot.cpp",
                                547,
                                0,
                                "%s",
                                "!g_clientDirty[entnum]");
                        v40 = DObjGetTree(v31);
                        if (CG_DObjCloneToBuffer(localClientNum, &cg_entitiesArray[0][v30], v40))
                        {
                            Com_ServerDObjClean(v30);
                            g_clientDirty[v30] = 1;
                        }
                    }
                }
                else
                {
                    v45[v32] &= ~(0x80000000 >> (v30 & 0x1F));
                }
                ++v28;
                ++v29;
            } while (v28 < nextSnap->numEntities);
        }
    }
    if (nextSnap)
    {
        if (readNext)
        {
            v41 = 0;
            if (nextSnap->numEntities > 0)
            {
                v42 = nextSnap->entityNums;
                do
                {
                    v43 = *v42 >> 5;
                    v44 = 0x80000000 >> (*v42 & 0x1F);
                    if ((v45[v43] & v44) != 0)
                        g_centInPrevSnapshot[v43] |= v44;
                    ++v41;
                    ++v42;
                } while (v41 < nextSnap->numEntities);
            }
        }
    }
    CG_CheckSnapshot(localClientNum, "CG_CreateNextSnap(post)");
}

void __cdecl CG_FirstSnapshot(int localClientNum)
{
    unsigned int i; // r31
    const char *v4; // r3
    snapshot_s *nextSnap; // r11

    if (localClientNum)
        MyAssertHandler(
            "c:\\trees\\cod3\\cod3src\\src\\cgame\\cg_local.h",
            910,
            0,
            "%s\n\t(localClientNum) = %i",
            "(localClientNum == 0)",
            localClientNum);
    if (cgArray[0].snap)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\cgame\\cg_snapshot.cpp", 835, 0, "%s", "!cgameGlob->snap");
    if (cgArray[0].nextSnap)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\cgame\\cg_snapshot.cpp", 836, 0, "%s", "!cgameGlob->nextSnap");
    for (i = 0; i < 0x880; ++i)
    {
        if (g_clientDirty[i])
            MyAssertHandler(
                "c:\\trees\\cod3\\cod3src\\src\\cgame\\cg_snapshot.cpp",
                839,
                0,
                "%s\n\t(i) = %i",
                "(!g_clientDirty[i])",
                i);
    }
    CG_CreateNextSnap(localClientNum, 0.0, 1);
    CG_SetInitialSnapshot(localClientNum);
    CG_SetNextSnap(localClientNum);
    CG_ProcessNextSnap(localClientNum);
    if (!cgArray[0].snap)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\cgame\\cg_snapshot.cpp", 847, 0, "%s", "cgameGlob->snap");
    if (!cgArray[0].nextSnap)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\cgame\\cg_snapshot.cpp", 848, 0, "%s", "cgameGlob->nextSnap");
    if (cgArray[0].nextSnap->serverTime != G_GetServerSnapTime())
        MyAssertHandler(
            "c:\\trees\\cod3\\cod3src\\src\\cgame\\cg_snapshot.cpp",
            849,
            0,
            "%s",
            "cgameGlob->nextSnap->serverTime == G_GetServerSnapTime()");
    AimAssist_Setup(localClientNum);
    v4 = va(
        "spawned: %.1f %.1f %.1f",
        cgArray[0].snap->ps.origin[0],
        cgArray[0].snap->ps.origin[1],
        cgArray[0].snap->ps.origin[2]
    );
    //LSP_LogString(cl_controller_in_use, v4);
    if (!cgArray[0].snap)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\cgame\\cg_snapshot.cpp", 867, 0, "%s", "cgameGlob->snap");
    if (cgArray[0].time - cgArray[0].snap->serverTime < 0)
        MyAssertHandler(
            "c:\\trees\\cod3\\cod3src\\src\\cgame\\cg_snapshot.cpp",
            868,
            0,
            "%s",
            "cgameGlob->time - cgameGlob->snap->serverTime >= 0");
    nextSnap = cgArray[0].nextSnap;
    if (!cgArray[0].nextSnap)
    {
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\cgame\\cg_snapshot.cpp", 869, 0, "%s", "cgameGlob->nextSnap");
        nextSnap = cgArray[0].nextSnap;
    }
    if (nextSnap != cgArray[0].snap && nextSnap->serverTime - cgArray[0].time <= 0)
        MyAssertHandler(
            "c:\\trees\\cod3\\cod3src\\src\\cgame\\cg_snapshot.cpp",
            870,
            0,
            "%s",
            "cgameGlob->nextSnap == cgameGlob->snap || cgameGlob->nextSnap->serverTime - cgameGlob->time > 0");
    if (cgArray[0].nextSnap->serverTime != G_GetServerSnapTime())
        MyAssertHandler(
            "c:\\trees\\cod3\\cod3src\\src\\cgame\\cg_snapshot.cpp",
            871,
            0,
            "%s",
            "cgameGlob->nextSnap->serverTime == G_GetServerSnapTime()");
}

void __cdecl CG_ProcessDemoSnapshots(int localClientNum)
{
    snapshot_s *nextSnap; // r11
    __int64 v4; // r11

    if (localClientNum)
        MyAssertHandler(
            "c:\\trees\\cod3\\cod3src\\src\\cgame\\cg_local.h",
            910,
            0,
            "%s\n\t(localClientNum) = %i",
            "(localClientNum == 0)",
            localClientNum);
    if (!cgArray[0].snap)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\cgame\\cg_snapshot.cpp", 885, 0, "%s", "cgameGlob->snap");
    nextSnap = cgArray[0].nextSnap;
    if (!cgArray[0].nextSnap)
    {
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\cgame\\cg_snapshot.cpp", 886, 0, "%s", "cgameGlob->nextSnap");
        nextSnap = cgArray[0].nextSnap;
    }
    if (cgArray[0].time - cgArray[0].snap->serverTime < 0 || cgArray[0].time - nextSnap->serverTime >= 0)
    {
        HIDWORD(v4) = cgArray[0].frametime;
        LODWORD(v4) = cgArray[0].frametime - cgArray[0].animFrametime;
        CG_CreateNextSnap(localClientNum, (float)((float)v4 * (float)0.001), 1);
        CG_SetNextSnap(localClientNum);
        CG_ProcessNextSnap(localClientNum);
    }
}

void __cdecl CG_ProcessSnapshots(int localClientNum)
{
    snapshot_s *snap; // r11
    int serverTime; // r5
    const char *v5; // r3
    int ServerSnapTime; // r3
    snapshot_s *nextSnap; // r11
    int v8; // r4
    const char *v9; // r3

    //PIXBeginNamedEvent_Copy_NoVarArgs(0xFFFFFFFF, "process snapshots");
    //Profile_Begin(11);
    if (localClientNum)
        MyAssertHandler(
            "c:\\trees\\cod3\\cod3src\\src\\cgame\\cg_local.h",
            910,
            0,
            "%s\n\t(localClientNum) = %i",
            "(localClientNum == 0)",
            localClientNum);
    if (cgArray[0].demoType == DEMO_TYPE_CLIENT)
    {
        CG_ProcessDemoSnapshots(localClientNum);
    }
    else if (SV_WaitServerSnapshot())
    {
        CG_SetNextSnap(localClientNum);
        CG_ProcessNextSnap(localClientNum);
    }
    CG_SetFrameInterpolation(localClientNum);
    snap = cgArray[0].snap;
    if (!cgArray[0].snap)
    {
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\cgame\\cg_snapshot.cpp", 941, 0, "%s", "cgameGlob->snap");
        snap = cgArray[0].snap;
    }
    serverTime = snap->serverTime;
    if (cgArray[0].time - serverTime < 0)
    {
        v5 = va("%d %d", cgArray[0].time, serverTime);
        MyAssertHandler(
            "c:\\trees\\cod3\\cod3src\\src\\cgame\\cg_snapshot.cpp",
            942,
            0,
            "%s\n\t%s",
            "cgameGlob->time - cgameGlob->snap->serverTime >= 0",
            v5);
    }
    if (!cgArray[0].nextSnap)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\cgame\\cg_snapshot.cpp", 943, 0, "%s", "cgameGlob->nextSnap");
    ServerSnapTime = G_GetServerSnapTime();
    nextSnap = cgArray[0].nextSnap;
    if (cgArray[0].nextSnap->serverTime != ServerSnapTime)
    {
        MyAssertHandler(
            "c:\\trees\\cod3\\cod3src\\src\\cgame\\cg_snapshot.cpp",
            944,
            0,
            "%s",
            "cgameGlob->nextSnap->serverTime == G_GetServerSnapTime()");
        nextSnap = cgArray[0].nextSnap;
    }
    if (nextSnap != cgArray[0].snap)
    {
        v8 = nextSnap->serverTime;
        if (v8 - cgArray[0].time <= 0)
        {
            v9 = va("nextSnap->serverTime is %i, cgame->time is %i\n", v8, cgArray[0].time);
            MyAssertHandler(
                "c:\\trees\\cod3\\cod3src\\src\\cgame\\cg_snapshot.cpp",
                945,
                0,
                "%s\n\t%s",
                "cgameGlob->nextSnap == cgameGlob->snap || cgameGlob->nextSnap->serverTime - cgameGlob->time > 0",
                v9);
        }
    }
    //Profile_EndInternal(0);
    //PIXEndNamedEvent();
}

