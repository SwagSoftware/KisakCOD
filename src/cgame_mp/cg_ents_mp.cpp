#ifndef KISAK_MP
#error This File is MultiPlayer Only
#endif

#include "cg_local_mp.h"

#include <aim_assist/aim_assist.h>

#include <EffectsCore/fx_system.h>

#include <gfx_d3d/r_scene.h>
#include <gfx_d3d/r_workercmds_common.h>

#include <script/scr_const.h>

#include <physics/phys_local.h>

#include <ragdoll/ragdoll.h>

#include <qcommon/com_bsp.h>

#include <win32/win_local.h>

#include <xanim/dobj.h>
#include <xanim/dobj_utils.h>
#include <script/scr_memorytree.h>
#include <game_mp/g_public_mp.h>
#include <universal/profile.h>

float g_entMoveTolVec[3] = { 16.0f, 16.0f, 16.0f };

void __cdecl CG_Player_PreControllers(DObj_s *obj, centity_s *cent)
{
    clientInfo_t *ci; // [esp+Ch] [ebp-8h]
    int32_t i; // [esp+10h] [ebp-4h]
    cg_s *cgameGlob;

    cgameGlob = CG_GetLocalClientGlobals(cent->pose.localClientNum);
    cent->pose.eType = cent->nextState.eType;
    iassert(cent->pose.eType == cent->nextState.eType);
    bcassert(cent->nextState.clientNum, MAX_CLIENTS);
    ci = &cgameGlob->bgs.clientinfo[cent->nextState.clientNum];
    if (ci->infoValid)
    {
        BG_Player_DoControllersSetup(&cent->nextState, ci, cgameGlob->frametime);
        for (i = 0; i < 6; ++i)
            DObjGetBoneIndex(obj, *controller_names[i], &cent->pose.player.tag[i]);
        cent->pose.fx.triggerTime = (int)&ci->control;
    }
    else
    {
        cent->pose.fx.triggerTime = 0;
    }
}

void __cdecl CG_mg42_PreControllers(DObj_s *obj, centity_s *cent)
{
    float v2; // [esp+14h] [ebp-80h]
    float v3; // [esp+18h] [ebp-7Ch]
    float v4; // [esp+1Ch] [ebp-78h]
    float v5; // [esp+20h] [ebp-74h]
    float v6; // [esp+24h] [ebp-70h]
    float v7; // [esp+28h] [ebp-6Ch]
    float v8; // [esp+2Ch] [ebp-68h]
    float v9; // [esp+30h] [ebp-64h]
    float v10; // [esp+34h] [ebp-60h]
    float v11; // [esp+38h] [ebp-5Ch]
    float v12; // [esp+3Ch] [ebp-58h]
    float v13; // [esp+40h] [ebp-54h]
    bool v14; // [esp+44h] [ebp-50h]
    float v15; // [esp+48h] [ebp-4Ch]
    float v16; // [esp+50h] [ebp-44h]
    float v17; // [esp+54h] [ebp-40h]
    float v18; // [esp+58h] [ebp-3Ch]
    float v19; // [esp+60h] [ebp-34h]
    float v20; // [esp+64h] [ebp-30h]
    float v21; // [esp+68h] [ebp-2Ch]
    float frameInterpolation; // [esp+70h] [ebp-24h]
    float v23; // [esp+74h] [ebp-20h]
    uint32_t playAnim; // [esp+7Ch] [ebp-18h]
    const cg_s *cgameGlob;

    iassert(cent->nextState.eType == ET_MG42);

    cgameGlob = CG_GetLocalClientGlobals(cent->pose.localClientNum);
    v14 = (cgameGlob->predictedPlayerState.eFlags & 0x300) != 0
        && cgameGlob->predictedPlayerState.viewlocked_entNum == cent->nextState.number;
    cent->pose.turret.playerUsing = v14;
    if (cent->pose.turret.playerUsing)
    {
        cent->pose.turret.viewAngles = cgameGlob->refdefViewAngles;
        cent->pose.cullIn = 0;
    }
    else
    {
        v21 = cent->currentState.u.turret.gunAngles[0];
        frameInterpolation = cgameGlob->frameInterpolation;
        v13 = cent->nextState.lerp.u.turret.gunAngles[0] - v21;
        v23 = v13 * 0.002777777845039964f;
        v12 = v23 + 0.5f;
        v11 = floor(v12);
        v10 = (v23 - v11) * 360.0f;
        cent->pose.turret.angles.pitch = v10 * frameInterpolation + v21;
        v18 = cent->currentState.u.turret.gunAngles[1];
        v19 = cgameGlob->frameInterpolation;
        v9 = cent->nextState.lerp.u.turret.gunAngles[1] - v18;
        v20 = v9 * 0.002777777845039964f;
        v8 = v20 + 0.5f;
        v7 = floor(v8);
        v6 = (v20 - v7) * 360.0f;
        cent->pose.turret.angles.yaw = v6 * v19 + v18;
    }
    v15 = cent->currentState.u.turret.gunAngles[2];
    v16 = cgameGlob->frameInterpolation;
    v5 = cent->nextState.lerp.u.turret.gunAngles[2] - v15;
    v17 = v5 * 0.002777777845039964f;
    v4 = v17 + 0.5f;
    v3 = floor(v4);
    v2 = (v17 - v3) * 360.0f;
    cent->pose.turret.barrelPitch = v2 * v16 + v15;
    DObjGetBoneIndex(obj, scr_const.tag_aim, &cent->pose.turret.tag_aim);
    DObjGetBoneIndex(obj, scr_const.tag_aim_animated, &cent->pose.turret.tag_aim_animated);
    DObjGetBoneIndex(obj, scr_const.tag_flash, &cent->pose.turret.tag_flash);
    DObjGetTree(obj);
    if ((cgameGlob->predictedPlayerState.eFlags & 0x300) != 0
        && cgameGlob->predictedPlayerState.viewlocked_entNum == cent->nextState.number)
    {
        playAnim = 1;
    }
    else if ((cent->nextState.lerp.eFlags & 0x40) != 0)
    {
        playAnim = 2;
    }
    else
    {
        playAnim = 1;
    }
    XAnimSetGoalWeightKnobAll(obj, playAnim, 0, 1.0f, 0.1f, 1.0f, 0, 0);
}

void  CG_UpdateBModelWorldBounds(uint32_t localClientNum, centity_s *cent, int32_t forceFilter)
{
    // TODO(mrsteyk): re-decompiled, check validity!

    //float* v4; // [esp-10h] [ebp-1F0h]
    float maxs[3]; // [esp-Ch] [ebp-1ECh] BYREF
    float v6; // [esp+0h] [ebp-1E0h]
    float mins[3]; // [esp+4h] [ebp-1DCh] BYREF
    float v8[8]; // [esp+10h] [ebp-1D0h]
    float v9; // [esp+30h] [ebp-1B0h]
    float v10; // [esp+34h] [ebp-1ACh]
    float v11; // [esp+38h] [ebp-1A8h]
    float4 rotatedBounds[2]; // [esp+3Ch] [ebp-1A4h] BYREF
    int32_t v13; // [esp+5Ch] [ebp-184h]
    __int64 v14; // [esp+60h] [ebp-180h]
    int32_t v15; // [esp+68h] [ebp-178h]
    int32_t v16; // [esp+6Ch] [ebp-174h]
    int32_t v17; // [esp+70h] [ebp-170h]
    int32_t v18; // [esp+74h] [ebp-16Ch]
    int32_t v19; // [esp+78h] [ebp-168h]
    __int64 v20; // [esp+7Ch] [ebp-164h]
    int32_t v21; // [esp+84h] [ebp-15Ch]
    int32_t v22; // [esp+88h] [ebp-158h]
    int32_t v23; // [esp+8Ch] [ebp-154h]
    float* v24; // [esp+90h] [ebp-150h]
    __int64 v25; // [esp+94h] [ebp-14Ch]
    int32_t v26; // [esp+9Ch] [ebp-144h]
    int32_t v27; // [esp+A0h] [ebp-140h]
    __int64 v28; // [esp+A4h] [ebp-13Ch]
    int32_t v29; // [esp+ACh] [ebp-134h]
    int32_t v30; // [esp+B0h] [ebp-130h]
    int32_t v31; // [esp+B4h] [ebp-12Ch]
    int32_t v32; // [esp+B8h] [ebp-128h]
    int32_t v33; // [esp+BCh] [ebp-124h]
    __int64 v34; // [esp+C0h] [ebp-120h]
    int32_t v35; // [esp+C8h] [ebp-118h]
    int32_t v36; // [esp+CCh] [ebp-114h]
    int32_t v37; // [esp+D0h] [ebp-110h]
    float* v38; // [esp+D4h] [ebp-10Ch]
    __int64 v39; // [esp+D8h] [ebp-108h]
    int32_t v40; // [esp+E0h] [ebp-100h]
    int32_t v41; // [esp+E4h] [ebp-FCh]
    __int64 v42; // [esp+E8h] [ebp-F8h]
    int32_t v43; // [esp+F0h] [ebp-F0h]
    int32_t v44; // [esp+F4h] [ebp-ECh]
    int32_t v45; // [esp+F8h] [ebp-E8h]
    int32_t v46; // [esp+FCh] [ebp-E4h]
    int32_t v47; // [esp+100h] [ebp-E0h]
    __int64 v48; // [esp+104h] [ebp-DCh]
    int32_t v49; // [esp+10Ch] [ebp-D4h]
    int32_t v50; // [esp+110h] [ebp-D0h]
    int32_t v51; // [esp+114h] [ebp-CCh]
    __int64 v52; // [esp+118h] [ebp-C8h]
    float v53; // [esp+120h] [ebp-C0h]
    float v54; // [esp+124h] [ebp-BCh]
    float* v55; // [esp+128h] [ebp-B8h]
    __int64 v56; // [esp+12Ch] [ebp-B4h]
    float v57; // [esp+134h] [ebp-ACh]
    float v58; // [esp+138h] [ebp-A8h]
    float v59; // [esp+13Ch] [ebp-A4h]
    float v60; // [esp+140h] [ebp-A0h]
    float v61; // [esp+144h] [ebp-9Ch]
    float v62; // [esp+148h] [ebp-98h]
    float v63; // [esp+14Ch] [ebp-94h]
    float v64; // [esp+150h] [ebp-90h]
    float v65; // [esp+154h] [ebp-8Ch]
    float v66; // [esp+158h] [ebp-88h]
    float* v67; // [esp+15Ch] [ebp-84h]
    float v68; // [esp+160h] [ebp-80h]
    float v69; // [esp+164h] [ebp-7Ch]
    float v70; // [esp+168h] [ebp-78h]
    float v71; // [esp+16Ch] [ebp-74h]
    float* v72; // [esp+170h] [ebp-70h]
    float v73; // [esp+174h] [ebp-6Ch]
    float v74; // [esp+178h] [ebp-68h]
    float v75; // [esp+17Ch] [ebp-64h]
    float v76; // [esp+180h] [ebp-60h]
    float* origin; // [esp+184h] [ebp-5Ch]
    float v78[3][3]; // [esp+188h] [ebp-58h] BYREF
    float axis_24[4]; // [esp+1ACh] [ebp-34h]
    float bounds_4[3]; // [esp+1BCh] [ebp-24h] BYREF
    int32_t v81; // [esp+1C8h] [ebp-18h]
    GfxBrushModel* brush; // [esp+1CCh] [ebp-14h]
    //int32_t bounds_28; // [esp+1D4h] [ebp-Ch]
    //GfxBrushModel* bmodel; // [esp+1D8h] [ebp-8h]
    //GfxBrushModel* retaddr; // [esp+1E0h] [ebp+0h]

    //bounds_28 = a1;
    //bmodel = retaddr;
    brush = R_GetBrushModel(cent->nextState.index.brushmodel);
    axis_24[0] = brush->bounds[0][0];
    axis_24[1] = brush->bounds[0][1];
    axis_24[2] = brush->bounds[0][2];
    axis_24[3] = brush->bounds[1][0];
    bounds_4[0] = brush->bounds[1][0];
    bounds_4[1] = brush->bounds[1][1];
    bounds_4[2] = brush->bounds[1][2];
    v81 = *(_DWORD*)&brush->surfaceCount;
    AnglesToAxis(cent->pose.angles, v78);
    origin = cent->pose.origin;
    v73 = v78[0][0];
    v74 = v78[0][1];
    v75 = v78[0][2];
    v76 = 0.0f;
    v72 = v78[1];
    v68 = v78[1][0];
    v69 = v78[1][1];
    v70 = v78[1][2];
    v71 = 0.0f;
    v67 = v78[2];
    v63 = v78[2][0];
    v64 = v78[2][1];
    v65 = v78[2][2];
    v66 = 0.0f;
    v59 = cent->pose.origin[0];
    v60 = cent->pose.origin[1];
    v61 = cent->pose.origin[2];
    v62 = 0.0f;
    *(float*)&v56 = axis_24[0];
    *((float*)&v56 + 1) = axis_24[0];
    v57 = axis_24[0];
    v58 = axis_24[0];
    v55 = bounds_4;
    *(float*)&v52 = bounds_4[0];
    *((float*)&v52 + 1) = bounds_4[0];
    v53 = bounds_4[0];
    v54 = bounds_4[0];
    if (v78[0][0] >= 0.0)
        v51 = 0;
    else
        v51 = -1;
    LODWORD(v48) = v51;
    if (v74 >= 0.0)
        v47 = 0;
    else
        v47 = -1;
    HIDWORD(v48) = v47;
    if (v75 >= 0.0)
        v46 = 0;
    else
        v46 = -1;
    v49 = v46;
    if (v76 >= 0.0)
        v45 = 0;
    else
        v45 = -1;
    v50 = v45;
    v42 = v52 & v48 | v56 & ~v48;
    v43 = LODWORD(v53) & v49 | LODWORD(v57) & ~v49;
    v44 = LODWORD(v54) & v50 | LODWORD(v58) & ~v50;
    v39 = v56 & v48 | v52 & ~v48;
    v40 = LODWORD(v57) & v49 | LODWORD(v53) & ~v49;
    v41 = LODWORD(v58) & v50 | LODWORD(v54) & ~v50;
    *(float*)&v56 = axis_24[1];
    *((float*)&v56 + 1) = axis_24[1];
    v57 = axis_24[1];
    v58 = axis_24[1];
    v38 = bounds_4;
    *(float*)&v52 = bounds_4[1];
    *((float*)&v52 + 1) = bounds_4[1];
    v53 = bounds_4[1];
    v54 = bounds_4[1];
    if (v68 >= 0.0f)
        v37 = 0;
    else
        v37 = -1;
    LODWORD(v34) = v37;
    if (v69 >= 0.0f)
        v33 = 0;
    else
        v33 = -1;
    HIDWORD(v34) = v33;
    if (v70 >= 0.0f)
        v32 = 0;
    else
        v32 = -1;
    v35 = v32;
    if (v71 >= 0.0f)
        v31 = 0;
    else
        v31 = -1;
    v36 = v31;
    v28 = v52 & v34 | v56 & ~v34;
    v29 = LODWORD(v53) & v35 | LODWORD(v57) & ~v35;
    v30 = LODWORD(v54) & v36 | LODWORD(v58) & ~v36;
    v25 = v56 & v34 | v52 & ~v34;
    v26 = LODWORD(v57) & v35 | LODWORD(v53) & ~v35;
    v27 = LODWORD(v58) & v36 | LODWORD(v54) & ~v36;
    *(float*)&v56 = axis_24[2];
    *((float*)&v56 + 1) = axis_24[2];
    v57 = axis_24[2];
    v58 = axis_24[2];
    v24 = bounds_4;
    *(float*)&v52 = bounds_4[2];
    *((float*)&v52 + 1) = bounds_4[2];
    v53 = bounds_4[2];
    v54 = bounds_4[2];
    if (v63 >= 0.0f)
        v23 = 0;
    else
        v23 = -1;
    LODWORD(v20) = v23;
    if (v64 >= 0.0f)
        v19 = 0;
    else
        v19 = -1;
    HIDWORD(v20) = v19;
    if (v65 >= 0.0f)
        v18 = 0;
    else
        v18 = -1;
    v21 = v18;
    if (v66 >= 0.0f)
        v17 = 0;
    else
        v17 = -1;
    v22 = v17;
    v14 = v52 & v20 | v56 & ~v20;
    v15 = LODWORD(v53) & v21 | LODWORD(v57) & ~v21;
    v16 = LODWORD(v54) & v22 | LODWORD(v58) & ~v22;
    *(_QWORD*)&rotatedBounds[1].unitVec[1].packed = v56 & v20 | v52 & ~v20;
    rotatedBounds[1].u[3] = LODWORD(v57) & v21 | LODWORD(v53) & ~v21;
    v13 = LODWORD(v58) & v22 | LODWORD(v54) & ~v22;
    v9 = *(float*)&v42 * v73 + v59;
    v10 = *((float*)&v42 + 1) * v74 + v60;
    v11 = *(float*)&v43 * v75 + v61;
    rotatedBounds[0].v[0] = *(float*)&v44 * v76 + v62;
    v9 = *(float*)&v28 * v68 + v9;
    v10 = *((float*)&v28 + 1) * v69 + v10;
    v11 = *(float*)&v29 * v70 + v11;
    rotatedBounds[0].v[0] = *(float*)&v30 * v71 + rotatedBounds[0].v[0];
    v9 = *(float*)&v14 * v63 + v9;
    v10 = *((float*)&v14 + 1) * v64 + v10;
    v11 = *(float*)&v15 * v65 + v11;
    rotatedBounds[0].v[0] = *(float*)&v16 * v66 + rotatedBounds[0].v[0];
    LODWORD(v8[7]) = (uintptr_t) &rotatedBounds[0].v[1];
    rotatedBounds[0].v[1] = *(float*)&v39 * v73 + v59;
    rotatedBounds[0].v[2] = *((float*)&v39 + 1) * v74 + v60;
    rotatedBounds[0].v[3] = *(float*)&v40 * v75 + v61;
    rotatedBounds[1].v[0] = *(float*)&v41 * v76 + v62;
    LODWORD(v8[6]) = (uintptr_t) &rotatedBounds[0].v[1];
    LODWORD(v8[5]) = (uintptr_t) &rotatedBounds[0].v[1];
    rotatedBounds[0].v[1] = *(float*)&v25 * v68 + rotatedBounds[0].v[1];
    rotatedBounds[0].v[2] = *((float*)&v25 + 1) * v69 + rotatedBounds[0].v[2];
    rotatedBounds[0].v[3] = *(float*)&v26 * v70 + rotatedBounds[0].v[3];
    rotatedBounds[1].v[0] = *(float*)&v27 * v71 + rotatedBounds[1].v[0];
    LODWORD(v8[4]) = (uintptr_t) &rotatedBounds[0].v[1];
    LODWORD(v8[3]) = (uintptr_t) &rotatedBounds[0].v[1];
    rotatedBounds[0].v[1] = rotatedBounds[1].v[1] * v63 + rotatedBounds[0].v[1];
    rotatedBounds[0].v[2] = rotatedBounds[1].v[2] * v64 + rotatedBounds[0].v[2];
    rotatedBounds[0].v[3] = rotatedBounds[1].v[3] * v65 + rotatedBounds[0].v[3];
    rotatedBounds[1].v[0] = *(float*)&v13 * v66 + rotatedBounds[1].v[0];
    mins[0] = v9;
    mins[1] = v10;
    mins[2] = v11;
    v8[0] = rotatedBounds[0].v[0];
    *(_QWORD*)maxs = *(_QWORD*)&rotatedBounds[0].unitVec[1].packed;
    maxs[2] = rotatedBounds[0].v[3];
    v6 = rotatedBounds[1].v[0];
    if (forceFilter)
        goto LABEL_41;
    if (!CG_VecLessThan(brush->writable.mins, mins) || !CG_VecLessThan(maxs, brush->writable.maxs))
    {
        Vec3Sub(mins, g_entMoveTolVec, mins);
        Vec3Add(maxs, g_entMoveTolVec, maxs);
    LABEL_41:
        brush->writable.mins[0] = mins[0];
        brush->writable.mins[1] = mins[1];
        brush->writable.mins[2] = mins[2];
        brush->writable.maxs[0] = maxs[0];
        brush->writable.maxs[1] = maxs[1];
        brush->writable.maxs[2] = maxs[2];
        R_LinkBModelEntity(localClientNum, cent->nextState.number, brush);
    }
}

// KISAKTODO: Remove this stupid function
bool __cdecl CG_VecLessThan(float *a, float *b)
{
    float v3; // [esp+4h] [ebp-1Ch]
    float v4; // [esp+8h] [ebp-18h]
    float v5; // [esp+Ch] [ebp-14h]
    float v6; // [esp+10h] [ebp-10h]
    float v7; // [esp+14h] [ebp-Ch]
    float v8; // [esp+18h] [ebp-8h]
    float v9; // [esp+1Ch] [ebp-4h]

    v8 = *a - *b;
    v9 = a[1] - b[1];
    v5 = v8 - v9;
    if (v5 < 0.0)
        v6 = a[1] - b[1];
    else
        v6 = *a - *b;
    v7 = a[2] - b[2];
    v4 = v6 - v7;
    if (v4 < 0.0)
        v3 = a[2] - b[2];
    else
        v3 = v6;
    return v3 <= 0.0;
}

void __cdecl CG_AdjustPositionForMover(
    int32_t localClientNum,
    const float *in,
    int32_t moverNum,
    int32_t fromTime,
    int32_t toTime,
    float *out,
    float *outDeltaAngles)
{
    float origin[3]; // [esp+0h] [ebp-4Ch] BYREF
    centity_s *cent; // [esp+Ch] [ebp-40h]
    float angles[3]; // [esp+10h] [ebp-3Ch] BYREF
    float deltaAngles[3]; // [esp+1Ch] [ebp-30h] BYREF
    float oldOrigin[3]; // [esp+28h] [ebp-24h] BYREF
    float oldAngles[3]; // [esp+34h] [ebp-18h] BYREF
    float deltaOrigin[3]; // [esp+40h] [ebp-Ch] BYREF

    if (outDeltaAngles)
    {
        *outDeltaAngles = 0.0;
        outDeltaAngles[1] = 0.0;
        outDeltaAngles[2] = 0.0;
    }
    if (moverNum > 0 && moverNum < 1022)
    {
        cent = CG_GetEntity(localClientNum, moverNum);
        if (cent->nextState.eType == 6 || cent->nextState.eType == 13)
        {
            BG_EvaluateTrajectory(&cent->currentState.pos, fromTime, oldOrigin);
            BG_EvaluateTrajectory(&cent->currentState.apos, fromTime, oldAngles);
            BG_EvaluateTrajectory(&cent->currentState.pos, toTime, origin);
            BG_EvaluateTrajectory(&cent->currentState.apos, toTime, angles);
            Vec3Sub(origin, oldOrigin, deltaOrigin);
            Vec3Sub(angles, oldAngles, deltaAngles);
            Vec3Add(in, deltaOrigin, out);
            if (outDeltaAngles)
            {
                *outDeltaAngles = deltaAngles[0];
                outDeltaAngles[1] = deltaAngles[1];
                outDeltaAngles[2] = deltaAngles[2];
            }
        }
        else
        {
            *out = *in;
            out[1] = in[1];
            out[2] = in[2];
        }
    }
    else
    {
        *out = *in;
        out[1] = in[1];
        out[2] = in[2];
    }
}

void __cdecl CG_SetFrameInterpolation(int32_t localClientNum)
{
    int32_t delta; // [esp+4h] [ebp-8h]
    cg_s *cgameGlob;

    cgameGlob = CG_GetLocalClientGlobals(localClientNum);

    iassert(cgameGlob->snap);
    iassert(cgameGlob->nextSnap);

    delta = cgameGlob->nextSnap->serverTime - cgameGlob->snap->serverTime;
    if (delta)
    {
        cgameGlob->frameInterpolation = (double)(cgameGlob->time - cgameGlob->snap->serverTime) / (double)delta;
        if (cgameGlob->frameInterpolation < 0.0)
            cgameGlob->frameInterpolation = 0.0;
    }
    else
    {
        cgameGlob->frameInterpolation = 0.0;
    }
}

void __cdecl CG_ProcessClientNoteTracks(cg_s *cgameGlob, uint32_t clientNum)
{
    XAnimNotify_s *noteList; // [esp+4h] [ebp-Ch] BYREF
    int32_t i; // [esp+8h] [ebp-8h]
    int32_t listSize; // [esp+Ch] [ebp-4h]

    if (clientNum < 0x40)
    {
        listSize = DObjGetClientNotifyList(&noteList);
        for (i = 0; i < listSize; ++i)
        {
            if (noteList[i].type == 1)
            {
                if (I_stricmp(noteList[i].name, "anim_gunhand = \"left\""))
                {
                    if (!I_stricmp(noteList[i].name, "anim_gunhand = \"right\""))
                    {
                        cgameGlob->bgs.clientinfo[clientNum].leftHandGun = 0;
                        cgameGlob->bgs.clientinfo[clientNum].dobjDirty = 1;
                    }
                }
                else
                {
                    cgameGlob->bgs.clientinfo[clientNum].leftHandGun = 1;
                    cgameGlob->bgs.clientinfo[clientNum].dobjDirty = 1;
                }
            }
        }
    }
}

void __cdecl CG_AddPacketEntity(int32_t localClientNum, int32_t entnum)
{
    bool v2; // [esp+4h] [ebp-84h]
    bool v4; // [esp+Ch] [ebp-7Ch]
    bool v5; // [esp+10h] [ebp-78h]
    bool v7; // [esp+18h] [ebp-70h]
    float *v8; // [esp+24h] [ebp-64h]
    float *v9; // [esp+28h] [ebp-60h]
    float diff[3]; // [esp+2Ch] [ebp-5Ch] BYREF
    float *v11; // [esp+38h] [ebp-50h]
    float *v12; // [esp+3Ch] [ebp-4Ch]
    float *v13; // [esp+40h] [ebp-48h]
    float *v14; // [esp+44h] [ebp-44h]
    float *v15; // [esp+48h] [ebp-40h]
    float radius; // [esp+4Ch] [ebp-3Ch]
    DObj_s *obj; // [esp+50h] [ebp-38h]
    float newAngles[3]; // [esp+54h] [ebp-34h] BYREF
    int32_t vehSlot; // [esp+60h] [ebp-28h]
    float origin[3]; // [esp+64h] [ebp-24h]
    centity_s *cent; // [esp+70h] [ebp-18h]
    float angles[3]; // [esp+74h] [ebp-14h]
    bool entMoved; // [esp+83h] [ebp-5h]
    uint32_t eType; // [esp+84h] [ebp-4h]
    int32_t savedregs; // [esp+88h] [ebp+0h] BYREF

    cent = CG_GetEntity(localClientNum, entnum);
    eType = cent->nextState.eType;
    v15 = cent->pose.origin;
    origin[0] = cent->pose.origin[0];
    origin[1] = cent->pose.origin[1];
    origin[2] = cent->pose.origin[2];
    v14 = cent->pose.angles;
    angles[0] = cent->pose.angles[0];
    angles[1] = cent->pose.angles[1];
    angles[2] = cent->pose.angles[2];
    entMoved = 0;
    if ((eType == 6 || eType == 13) && cent->nextState.solid == 0xFFFFFF)
    {
        CG_CalcEntityLerpPositions(localClientNum, cent);
        v4 = origin[0] == cent->pose.origin[0] && origin[1] == cent->pose.origin[1] && origin[2] == cent->pose.origin[2];
        v2 = 1;
        if (v4)
        {
            if (angles[0] == cent->pose.angles[0] && angles[1] == cent->pose.angles[1] && angles[2] == cent->pose.angles[2])
                v2 = 0;
        }
        entMoved = v2;
        if (v2)
            CG_UpdateBModelWorldBounds(localClientNum, cent, 0);
    }
    else
    {
        if (eType == 1 && CG_VehEntityUsingVehicle(localClientNum, entnum))
        {
            CG_VehSeatTransformForPlayer(localClientNum, entnum, cent->pose.origin, newAngles);
            vehSlot = CG_VehPlayerVehicleSlot(localClientNum, entnum);
            if (vehSlot != 1)
            {
                v13 = cent->pose.angles;
                cent->pose.angles[0] = newAngles[0];
                v13[1] = newAngles[1];
                v13[2] = newAngles[2];
            }
        }
        else
        {
            CG_CalcEntityLerpPositions(localClientNum, cent);
        }
        v12 = cent->pose.origin;
        v7 = origin[0] == cent->pose.origin[0] && origin[1] == v12[1] && origin[2] == v12[2];
        v5 = 1;
        if (v7)
        {
            v11 = cent->pose.angles;
            if (angles[0] == cent->pose.angles[0] && angles[1] == v11[1] && angles[2] == v11[2])
                v5 = 0;
        }
        entMoved = v5;
        Vec3Sub(cg_entityOriginArray[localClientNum][entnum], cent->pose.origin, diff);
        if (Vec3LengthSq(diff) > 256.0)
        {
            v8 = cg_entityOriginArray[localClientNum][entnum];
            v9 = cent->pose.origin;
            *v8 = cent->pose.origin[0];
            v8[1] = v9[1];
            v8[2] = v9[2];
            obj = Com_GetClientDObj(entnum, localClientNum);
            if (obj)
            {
                radius = DObjGetRadius(obj) + 16.0;
                R_LinkDObjEntity(localClientNum, entnum, cent->pose.origin, radius);
            }
        }
    }
    if (cent->pose.physObjId == -1)
    {
        if (CG_IsEntityLinked(localClientNum, entnum))
            CG_UnlinkEntity(localClientNum, entnum);
    }
    else
    {
        if (CG_IsEntityLinked(localClientNum, entnum))
        {
            if (entMoved)
                CG_LinkEntity(localClientNum, entnum);
        }
        else if (CG_EntityNeedsLinked(localClientNum, entnum))
        {
            CG_LinkEntity(localClientNum, entnum);
        }
        CG_UpdateClientDobjPartBits(cent, entnum, localClientNum);
        CG_ProcessEntity(localClientNum, cent);
    }
}

void __cdecl CG_UpdateClientDobjPartBits(centity_s *cent, int32_t entnum, int32_t localClientNum)
{
    DObj_s *obj; // [esp+0h] [ebp-14h]
    uint32_t oldPartBits[4]; // [esp+4h] [ebp-10h] BYREF

    if (!cent)
        MyAssertHandler(".\\cgame_mp\\cg_ents_mp.cpp", 632, 0, "%s", "cent");
    obj = Com_GetClientDObj(entnum, localClientNum);
    if (obj)
    {
        DObjGetHidePartBits(obj, oldPartBits);
        DObjSetHidePartBits(obj, cent->nextState.partBits);
        FX_MarkEntUpdateHidePartBits(oldPartBits, cent->nextState.partBits, localClientNum, entnum);
    }
}

int32_t __cdecl CG_AddPacketEntities(int32_t localClientNum)
{
    int32_t viewlocked_entNum; // [esp+0h] [ebp-154h]
    uint32_t linkedPlayerCount; // [esp+34h] [ebp-120h]
    int32_t lockedView; // [esp+38h] [ebp-11Ch]
    centity_s *cent; // [esp+40h] [ebp-114h]
    int32_t linkedPlayers[64]; // [esp+44h] [ebp-110h]
    int32_t num; // [esp+144h] [ebp-10h]
    int32_t entnum; // [esp+148h] [ebp-Ch]
    int32_t lockedViewEntNum; // [esp+14Ch] [ebp-8h]
    uint32_t eType; // [esp+150h] [ebp-4h]
    cg_s *cgameGlob;

    KISAK_NULLSUB();

    PROF_SCOPED("CG_AddPacketEntities");

    cgameGlob = CG_GetLocalClientGlobals(localClientNum);

    cgameGlob->rumbleScale = 0.0;
    if ((cgameGlob->predictedPlayerState.eFlags & 0x300) != 0)
        viewlocked_entNum = cgameGlob->predictedPlayerState.viewlocked_entNum;
    else
        viewlocked_entNum = 1023;
    lockedViewEntNum = viewlocked_entNum;
    lockedView = 0;
    linkedPlayerCount = 0;
    CG_AddClientSideSounds(localClientNum);
    for (num = 0; num < cgameGlob->nextSnap->numEntities; ++num)
    {
        entnum = cgameGlob->nextSnap->entities[num].number;
        cent = CG_GetEntity(localClientNum, entnum);
        eType = cent->nextState.eType;
        if (eType < 0x11)
        {
            if (entnum == lockedViewEntNum)
            {
                lockedView = 1;
                CG_CalcEntityLerpPositions(localClientNum, cent);
            }
            else if (eType == 1 && CG_VehEntityUsingVehicle(localClientNum, entnum))
            {
                bcassert(linkedPlayerCount, MAX_CLIENTS);
                linkedPlayers[linkedPlayerCount++] = entnum;
            }
            else
            {
                CG_AddPacketEntity(localClientNum, entnum);
            }
        }
    }
    for (num = 0; num < (int)linkedPlayerCount; ++num)
    {
        entnum = linkedPlayers[num];
        CG_GetEntity(localClientNum, entnum);
        CG_AddPacketEntity(localClientNum, entnum);
    }
    return lockedView;
}

void __cdecl CG_DObjUpdateInfo(const cg_s *cgameGlob, DObj_s *obj, bool notify)
{
    float dtime; // [esp+8h] [ebp-4h]

    dtime = (double)cgameGlob->frametime * EQUAL_EPSILON;
    DObjUpdateClientInfo(obj, dtime, notify);
}

int32_t __cdecl CG_DObjGetWorldBoneMatrix(
    const cpose_t *pose,
    DObj_s *obj,
    int32_t boneIndex,
    float (*tagMat)[3],
    float *origin)
{
    DObjAnimMat *mat; // [esp+54h] [ebp-4h]

    iassert(obj);
    iassert(pose);
    mat = CG_DObjGetLocalBoneMatrix(pose, obj, boneIndex);
    if (!mat)
        return 0;

    ConvertQuatToMat(mat, tagMat);

    iassert(pose->localClientNum == 0);
    Vec3Add(mat->trans, CG_GetLocalClientGlobals(0)->refdef.viewOffset, origin);
    return 1;
}

DObjAnimMat *__cdecl CG_DObjGetLocalBoneMatrix(const cpose_t *pose, DObj_s *obj, int32_t boneIndex)
{
    DObjAnimMat *mat; // [esp+34h] [ebp-4h]

    iassert(obj);
    {
        PROF_SCOPED("CG_DObjGetLocalTagMatrix");
        CG_DObjCalcBone(pose, obj, boneIndex);
    }
    mat = DObjGetRotTransArray(obj);
    if (mat)
        return &mat[boneIndex];
    else
        return 0;
}

int32_t __cdecl CG_DObjGetWorldTagMatrix(
    const cpose_t *pose,
    DObj_s *obj,
    uint32_t tagName,
    float (*tagMat)[3],
    float *origin)
{
    DObjAnimMat *mat; // [esp+54h] [ebp-4h]

    iassert(obj);
    iassert(pose);

    mat = CG_DObjGetLocalTagMatrix(pose, obj, tagName);
    if (!mat)
        return 0;

    ConvertQuatToMat(mat, tagMat);

    iassert(pose->localClientNum == 0);
    Vec3Add(mat->trans, CG_GetLocalClientGlobals(0)->refdef.viewOffset, origin);
    return 1;
}

DObjAnimMat *__cdecl CG_DObjGetLocalTagMatrix(const cpose_t *pose, DObj_s *obj, uint32_t tagName)
{
    unsigned __int8 boneIndex; // [esp+3h] [ebp-1h] BYREF

    iassert(obj);
    boneIndex = -2;

    if (DObjGetBoneIndex(obj, tagName, &boneIndex))
        return CG_DObjGetLocalBoneMatrix(pose, obj, boneIndex);
    else
        return 0;
}

int32_t __cdecl CG_DObjGetWorldTagPos(const cpose_t *pose, DObj_s *obj, uint32_t tagName, float *pos)
{
    DObjAnimMat *mat; // [esp+8h] [ebp-4h]

    iassert(obj);
    iassert(pose);

    mat = CG_DObjGetLocalTagMatrix(pose, obj, tagName);

    if (!mat)
        return 0;

    Vec3Add(mat->trans, CG_GetLocalClientGlobals(pose->localClientNum)->refdef.viewOffset, pos);

    return 1;
}

cpose_t*__cdecl CG_GetPose(int32_t localClientNum, uint32_t handle)
{
    iassert(handle >= 0 && handle < (((1 << 10)) + 128));

    if ((int)handle < 1024)
        return &CG_GetEntity(localClientNum, handle)->pose;

    iassert(handle >= ((1 << 10)) && handle - ((1 << 10)) < 128);

    return &CG_GetLocalClientGlobals(localClientNum)->viewModelPose;
}

void __cdecl CG_CalcEntityLerpPositions(int32_t localClientNum, centity_s *cent)
{
    uint32_t corpseIndex; // [esp+18h] [ebp-8h]
    clientInfo_t *ci; // [esp+1Ch] [ebp-4h]
    clientInfo_t *cia; // [esp+1Ch] [ebp-4h]
    cg_s *cgameGlob;
    cgs_t *cgs;

    cgameGlob = CG_GetLocalClientGlobals(localClientNum);

    if (cent->currentState.pos.trType == TR_PHYSICS)
    {
        CG_CalcEntityPhysicsPositions(localClientNum, cent);
    }
    else if (cent->currentState.pos.trType == TR_INTERPOLATE && cent->nextState.lerp.pos.trType != TR_PHYSICS
        || cent->currentState.pos.trType == TR_LINEAR_STOP && cent->nextState.number < 64)
    {
        CG_InterpolateEntityPosition(cgameGlob, cent);
    }
    else
    {
        BG_EvaluateTrajectory(&cent->currentState.pos, cgameGlob->time, cent->pose.origin);
        BG_EvaluateTrajectory(&cent->currentState.apos, cgameGlob->time, cent->pose.angles);
        if (cent->nextState.eType == 1)
        {
            bcassert(cent->nextState.clientNum, MAX_CLIENTS);

            ci = &cgameGlob->bgs.clientinfo[cent->nextState.clientNum];
            ci->lerpMoveDir = (float)cent->nextState.lerp.u.loopFx.period;
            ci->playerAngles[0] = cent->pose.angles[0];
            ci->playerAngles[1] = cent->pose.angles[1];
            ci->playerAngles[2] = cent->pose.angles[2];
            cent->pose.angles[0] = 0.0f;
            cent->pose.angles[2] = 0.0f;
            ci->lerpLean = cent->nextState.lerp.u.turret.gunAngles[0];
        }
        else if (cent->nextState.eType == 2)
        {
            corpseIndex = cent->nextState.number - 64;
            iassert((unsigned)corpseIndex < MAX_CLIENT_CORPSES);
            cgs = CG_GetLocalClientStaticGlobals(localClientNum);
            cia = &cgs->corpseinfo[corpseIndex];
            cgs->corpseinfo[corpseIndex].lerpMoveDir = (float)cent->nextState.lerp.u.loopFx.period;
            cia->playerAngles[0] = cent->pose.angles[0];
            cia->playerAngles[1] = cent->pose.angles[1];
            cia->playerAngles[2] = cent->pose.angles[2];
            cent->pose.angles[0] = 0.0;
            cent->pose.angles[2] = 0.0;
            cia->lerpLean = cent->nextState.lerp.u.turret.gunAngles[0];
        }
        if (cent != &cgameGlob->predictedPlayerEntity)
            CG_AdjustPositionForMover(
                localClientNum,
                cent->pose.origin,
                cent->nextState.groundEntityNum,
                cgameGlob->snap->serverTime,
                cgameGlob->time,
                cent->pose.origin,
                0);
        if (CG_IsRagdollTrajectory(&cent->currentState.pos))
            CG_CalcEntityRagdollPositions(localClientNum, cent);
    }
}

void __cdecl CG_InterpolateEntityPosition(cg_s *cgameGlob, centity_s *cent)
{
    float v2; // [esp+8h] [ebp-B4h]
    float v3; // [esp+Ch] [ebp-B0h]
    float v4; // [esp+10h] [ebp-ACh]
    float v5; // [esp+14h] [ebp-A8h]
    float v6; // [esp+18h] [ebp-A4h]
    float v7; // [esp+1Ch] [ebp-A0h]
    float v8; // [esp+20h] [ebp-9Ch]
    float v9; // [esp+24h] [ebp-98h]
    float v10; // [esp+28h] [ebp-94h]
    float v11; // [esp+2Ch] [ebp-90h]
    float v12; // [esp+30h] [ebp-8Ch]
    float v13; // [esp+34h] [ebp-88h]
    float v14; // [esp+38h] [ebp-84h]
    float v15; // [esp+3Ch] [ebp-80h]
    float v16; // [esp+40h] [ebp-7Ch]
    float v17; // [esp+44h] [ebp-78h]
    float v18; // [esp+48h] [ebp-74h]
    float v19; // [esp+4Ch] [ebp-70h]
    float v20; // [esp+50h] [ebp-6Ch]
    float v21; // [esp+54h] [ebp-68h]
    float v22; // [esp+58h] [ebp-64h]
    float v23; // [esp+60h] [ebp-5Ch]
    float *playerAngles; // [esp+64h] [ebp-58h]
    float period; // [esp+6Ch] [ebp-50h]
    float v26; // [esp+70h] [ebp-4Ch]
    float v27; // [esp+74h] [ebp-48h]
    float v28; // [esp+78h] [ebp-44h]
    float v29; // [esp+80h] [ebp-3Ch]
    float v30; // [esp+84h] [ebp-38h]
    float v31; // [esp+8Ch] [ebp-30h]
    float v32; // [esp+90h] [ebp-2Ch]
    float v33; // [esp+98h] [ebp-24h]
    float next[3]; // [esp+9Ch] [ebp-20h] BYREF
    float f; // [esp+A8h] [ebp-14h]
    clientInfo_t *ci; // [esp+ACh] [ebp-10h]
    float current[3]; // [esp+B0h] [ebp-Ch] BYREF

    if (!cgameGlob->snap)
        MyAssertHandler(".\\cgame_mp\\cg_ents_mp.cpp", 1186, 0, "%s", "cgameGlob->snap");
    if (!cgameGlob->nextSnap)
        MyAssertHandler(".\\cgame_mp\\cg_ents_mp.cpp", 1187, 0, "%s", "cgameGlob->nextSnap");
    if (cent->nextState.lerp.pos.trType == TR_PHYSICS)
        MyAssertHandler(".\\cgame_mp\\cg_ents_mp.cpp", 1188, 0, "%s", "cent->nextState.lerp.pos.trType != TR_PHYSICS");
    f = cgameGlob->frameInterpolation;
    BG_EvaluateTrajectory(&cent->currentState.pos, cgameGlob->snap->serverTime, current);
    BG_EvaluateTrajectory(&cent->nextState.lerp.pos, cgameGlob->nextSnap->serverTime, next);
    Vec3Lerp(current, next, f, cent->pose.origin);
    BG_EvaluateTrajectory(&cent->currentState.apos, cgameGlob->snap->serverTime, current);
    BG_EvaluateTrajectory(&cent->nextState.lerp.apos, cgameGlob->nextSnap->serverTime, next);
    v32 = current[0];
    v21 = next[0] - current[0];
    v33 = v21 * 0.002777777845039964;
    v20 = v33 + 0.5;
    v19 = floor(v20);
    v18 = (v33 - v19) * 360.0;
    cent->pose.angles[0] = v18 * f + v32;
    v30 = current[1];
    v17 = next[1] - current[1];
    v31 = v17 * 0.002777777845039964;
    v16 = v31 + 0.5;
    v15 = floor(v16);
    v14 = (v31 - v15) * 360.0;
    cent->pose.angles[1] = v14 * f + v30;
    v28 = current[2];
    v13 = next[2] - current[2];
    v29 = v13 * 0.002777777845039964;
    v12 = v29 + 0.5;
    v11 = floor(v12);
    v10 = (v29 - v11) * 360.0;
    cent->pose.angles[2] = v10 * f + v28;
    if (cent->nextState.eType == 1)
    {
        if (cent->nextState.clientNum >= 0x40u)
            MyAssertHandler(
                ".\\cgame_mp\\cg_ents_mp.cpp",
                1208,
                0,
                "cent->nextState.clientNum doesn't index MAX_CLIENTS\n\t%i not in [0, %i)",
                cent->nextState.clientNum,
                64);
        ci = &cgameGlob->bgs.clientinfo[cent->nextState.clientNum];
        period = (float)cent->currentState.u.loopFx.period;
        v26 = (float)cent->nextState.lerp.u.loopFx.period;
        v9 = v26 - period;
        v27 = v9 * 0.002777777845039964;
        v8 = v27 + 0.5;
        v7 = floor(v8);
        v6 = (v27 - v7) * 360.0;
        ci->lerpMoveDir = v6 * f + period;
        playerAngles = ci->playerAngles;
        ci->playerAngles[0] = cent->pose.angles[0];
        playerAngles[1] = cent->pose.angles[1];
        playerAngles[2] = cent->pose.angles[2];
        cent->pose.angles[0] = 0.0;
        cent->pose.angles[2] = 0.0;
        v22 = cent->currentState.u.turret.gunAngles[0];
        v5 = cent->nextState.lerp.u.turret.gunAngles[0] - v22;
        v23 = v5 * 0.002777777845039964;
        v4 = v23 + 0.5;
        v3 = floor(v4);
        v2 = (v23 - v3) * 360.0;
        ci->lerpLean = v2 * f + v22;
    }
}

void __cdecl CG_CalcEntityPhysicsPositions(int32_t localClientNum, centity_s *cent)
{
    cgs_t *cgs;

    iassert(cent);
    iassert(cent->currentState.pos.trType == TR_PHYSICS && cent->currentState.apos.trType == TR_PHYSICS);

    if (cent->nextValid && (cent->nextState.lerp.eFlags & 0x20) == 0 && cent->nextState.solid != 0xFFFFFF)
    {
        cgs = CG_GetLocalClientStaticGlobals(localClientNum);
        
        if (CG_PreProcess_GetDObj(
            localClientNum,
            cent->nextState.number,
            cent->nextState.eType,
            cgs->gameModels[cent->nextState.index.brushmodel])
            && !CG_ExpiredLaunch(localClientNum, cent))
        {
            if (!cent->pose.physObjId)
                CG_CreatePhysicsObject(localClientNum, cent);
            if (cent->pose.physObjId == -1)
            {
                cent->pose.origin[0] = cent->currentState.pos.trBase[0];
                cent->pose.origin[1] = cent->currentState.pos.trBase[1];
                cent->pose.origin[2] = cent->currentState.pos.trBase[2];
                cent->pose.angles[0] = cent->currentState.apos.trBase[0];
                cent->pose.angles[1] = cent->currentState.apos.trBase[1];
                cent->pose.angles[2] = cent->currentState.apos.trBase[2];
            }
            else
            {
                CG_UpdatePhysicsPose(cent);
            }
        }
    }
}

void __cdecl CG_CreatePhysicsObject(int32_t localClientNum, centity_s *cent)
{
    const char *v2; // eax
    const char *Name; // eax
    float velocity[3]; // [esp+1Ch] [ebp-44h] BYREF
    int32_t physObjId; // [esp+28h] [ebp-38h]
    DObj_s *obj; // [esp+2Ch] [ebp-34h]
    PhysPreset *physPreset; // [esp+30h] [ebp-30h]
    float quat[4]; // [esp+34h] [ebp-2Ch] BYREF
    float speed; // [esp+44h] [ebp-1Ch]
    float direction[3]; // [esp+48h] [ebp-18h] BYREF
    float position[3]; // [esp+54h] [ebp-Ch] BYREF

    velocity[0] = 0.0;
    velocity[1] = 0.0;
    velocity[2] = 0.0;
    position[0] = cent->currentState.pos.trBase[0];
    position[1] = cent->currentState.pos.trBase[1];
    position[2] = cent->currentState.pos.trBase[2];
    AnglesToQuat(cent->currentState.apos.trBase, quat);
    obj = Com_GetClientDObj(cent->nextState.number, localClientNum);
    if (!obj)
        MyAssertHandler(".\\cgame_mp\\cg_ents_mp.cpp", 1237, 0, "%s", "obj");
    physPreset = DObjGetPhysPreset(obj);
    if (physPreset)
    {
        Sys_EnterCriticalSection(CRITSECT_PHYSICS);
        physObjId = (int)Phys_ObjCreate(PHYS_WORLD_FX, position, quat, velocity, physPreset);
        if (physObjId)
        {
            DObjPhysicsSetCollisionFromXModel(obj, PHYS_WORLD_FX, (dxBody *)physObjId);
            direction[0] = cent->currentState.apos.trDelta[0];
            direction[1] = cent->currentState.apos.trDelta[1];
            direction[2] = cent->currentState.apos.trDelta[2];
            speed = Vec3Normalize(direction);
            Phys_ObjBulletImpact(
                PHYS_WORLD_FX,
                (dxBody *)physObjId,
                cent->currentState.pos.trDelta,
                direction,
                speed,
                physPreset->bulletForceScale);
            Sys_LeaveCriticalSection(CRITSECT_PHYSICS);
            cent->pose.physObjId = physObjId;
        }
        else
        {
            Name = DObjGetName(obj);
            Com_PrintWarning(1, "Failed to create physics object for '%s'.\n", Name);
            cent->pose.physObjId = -1;
            Sys_LeaveCriticalSection(CRITSECT_PHYSICS);
        }
    }
    else
    {
        cent->pose.physObjId = -1;
        v2 = DObjGetName(obj);
        Com_PrintWarning(1, "Failed to create physics object for '%s'.  No physics preset.\n", v2);
    }
}

void __cdecl CG_UpdatePhysicsPose(centity_s *cent)
{
    float quat[4]; // [esp+0h] [ebp-10h] BYREF

    if (!cent->pose.physObjId || cent->pose.physObjId == -1)
        MyAssertHandler(
            ".\\cgame_mp\\cg_ents_mp.cpp",
            1281,
            0,
            "%s",
            "cent->pose.physObjId != PHYS_OBJ_ID_NULL && cent->pose.physObjId != PHYS_OBJ_ID_DEAD");
    Sys_EnterCriticalSection(CRITSECT_PHYSICS);
    Phys_ObjGetInterpolatedState(PHYS_WORLD_FX, (dxBody *)cent->pose.physObjId, cent->pose.origin, quat);
    Sys_LeaveCriticalSection(CRITSECT_PHYSICS);
    UnitQuatToAngles(quat, cent->pose.angles);
}

char __cdecl CG_ExpiredLaunch(int32_t localClientNum, centity_s *cent)
{
    iassert(cent->nextValid);

    cg_s *cgameGlob = CG_GetLocalClientGlobals(localClientNum);

    if (cent->pose.physObjId || cgameGlob->time <= cent->nextState.lerp.pos.trTime + 1000)
        return 0;
    cent->pose.physObjId = -1;
    return 1;
}

void __cdecl CG_CalcEntityRagdollPositions(int32_t localClientNum, centity_s *cent)
{
    if (!cent)
        MyAssertHandler(".\\cgame_mp\\cg_ents_mp.cpp", 1400, 0, "%s", "cent");
    if (!CG_IsRagdollTrajectory(&cent->currentState.pos) && !CG_IsRagdollTrajectory(&cent->currentState.apos))
        MyAssertHandler(
            ".\\cgame_mp\\cg_ents_mp.cpp",
            1401,
            0,
            "%s",
            "CG_IsRagdollTrajectory( &cent->currentState.pos ) || CG_IsRagdollTrajectory( &cent->currentState.apos )");
    if (!cent->pose.ragdollHandle && !cent->pose.killcamRagdollHandle)
        CG_CreateRagdollObject(localClientNum, cent);
    if (cent->pose.ragdollHandle || cent->pose.killcamRagdollHandle)
        CG_UpdateRagdollPose(cent);
}

void __cdecl CG_CreateRagdollObject(int32_t localClientNum, centity_s *cent)
{
    int32_t RagdollForDObj; // eax
    bool shareRagdoll; // [esp+Ah] [ebp-2h]
    bool reset; // [esp+Bh] [ebp-1h]
    const cg_s *cgameGlob;

    cgameGlob = CG_GetLocalClientGlobals(localClientNum);

    if (cgameGlob->inKillCam)
        reset = 0;
    else
        reset = (cent->nextState.lerp.eFlags & 0x80000) != 0;
    if (cent->nextState.eType == 2 && cent->nextState.clientNum == cgameGlob->clientNum && cgameGlob->inKillCam)
    {
        shareRagdoll = 0;
        RagdollForDObj = Ragdoll_CreateRagdollForDObj(localClientNum, 0, cent->nextState.number, reset, 0);
    }
    else
    {
        shareRagdoll = 1;
        RagdollForDObj = Ragdoll_CreateRagdollForDObj(localClientNum, 0, cent->nextState.number, reset, 1);
    }
    if (shareRagdoll)
        cent->pose.ragdollHandle = RagdollForDObj;
    else
        cent->pose.killcamRagdollHandle = RagdollForDObj;
    cent->pose.isRagdoll = 1;
}

void __cdecl CG_UpdateRagdollPose(centity_s *cent)
{
    if (!cent->pose.ragdollHandle && !cent->pose.killcamRagdollHandle)
        MyAssertHandler(
            ".\\cgame_mp\\cg_ents_mp.cpp",
            1389,
            0,
            "%s",
            "cent->pose.ragdollHandle != RAGDOLL_INVALID || cent->pose.killcamRagdollHandle != RAGDOLL_INVALID");
    if (cent->pose.killcamRagdollHandle)
        Ragdoll_GetRootOrigin(cent->pose.killcamRagdollHandle, cent->pose.origin);
    else
        Ragdoll_GetRootOrigin(cent->pose.ragdollHandle, cent->pose.origin);
}

DObj_s *__cdecl CG_PreProcess_GetDObj(int32_t localClientNum, int32_t entIndex, int32_t entType, XModel *model)
{
    XAnimTree_s *Tree; // [esp+18h] [ebp-20h]
    float *v6; // [esp+1Ch] [ebp-1Ch]
    DObjModel_s dobjModel; // [esp+20h] [ebp-18h] BYREF
    DObj_s *obj; // [esp+28h] [ebp-10h]
    XAnimTree_s *animTree; // [esp+2Ch] [ebp-Ch]
    centity_s *cent; // [esp+30h] [ebp-8h]
    XAnim_s *anims; // [esp+34h] [ebp-4h]

    obj = Com_GetClientDObj(entIndex, localClientNum);
    cent = CG_GetEntity(localClientNum, entIndex);
    if (obj && (!model || !CG_CheckDObjInfoMatches(localClientNum, entIndex, entType, model)))
    {
        if (cent->pose.physObjId != -1 && cent->pose.physObjId)
        {
            if (CG_IsEntityLinked(localClientNum, cent->nextState.number))
                CG_UnlinkEntity(localClientNum, cent->nextState.number);
            Phys_ObjDestroy(PHYS_WORLD_FX, (dxBody *)cent->pose.physObjId);
            cent->pose.physObjId = 0;
        }
        FX_MarkEntDetachAll(localClientNum, entIndex);
        CG_SafeDObjFree(localClientNum, entIndex);
        obj = 0;
    }
    if (!obj && model)
    {
        if (cent->tree)
            MyAssertHandler(".\\cgame_mp\\cg_ents_mp.cpp", 1568, 0, "%s", "!cent->tree");
        anims = CG_GetAnimations(localClientNum, entIndex, entType);
        if (anims)
            Tree = XAnimCreateTree(anims, (void *(__cdecl *)(int))CG_AllocAnimTree);
        else
            Tree = 0;
        animTree = Tree;
        cent->tree = Tree;
        dobjModel.model = model;
        dobjModel.boneName = 0;
        dobjModel.ignoreCollision = 1;
        obj = Com_ClientDObjCreate(&dobjModel, 1u, animTree, entIndex, localClientNum);
        CG_SetDObjInfo(localClientNum, entIndex, entType, model);
        v6 = cg_entityOriginArray[localClientNum][cent->nextState.number];
        *v6 = 131072.0f;
        v6[1] = 131072.0f;
        v6[2] = 131072.0f;
        if (entType == 12)
            XAnimSetCompleteGoalWeight(obj, 1u, 1.0f, 0.2f, 1.5f, 0, 0, 1);
    }
    return obj;
}

XAnim_s *__cdecl CG_GetAnimations(int32_t localClientNum, uint32_t entIndex, int32_t entType)
{
    centity_s *cent; // [esp+4h] [ebp-4h]
    centity_s *centa; // [esp+4h] [ebp-4h]

    if (entType == 11)
    {
        cent = CG_GetEntity(localClientNum, entIndex);
        return CG_GetMG42Anims(cent);
    }
    else if (entType == 12)
    {
        centa = CG_GetEntity(localClientNum, entIndex);
        return CG_GetHelicopterAnims(centa);
    }
    else
    {
        return 0;
    }
}

XAnim_s *__cdecl CG_GetMG42Anims(centity_s *cent)
{
    WeaponDef *weapDef; // [esp+0h] [ebp-8h]
    XAnim_s *pAnims; // [esp+4h] [ebp-4h]

    iassert(cent->nextState.weapon);
    weapDef = BG_GetWeaponDef(cent->nextState.weapon);
    pAnims = XAnimCreateAnims("MG42", 3u, (void *(__cdecl *)(int))Hunk_AllocXAnimClient);
    iassert(pAnims);
    XAnimBlend(pAnims, 0, "root", 1u, 2u, 0);
    BG_CreateXAnim(pAnims, 1u, (char *)weapDef->szXAnims[1]);
    BG_CreateXAnim(pAnims, 2u, (char *)weapDef->szXAnims[3]);
    return pAnims;
}

XAnim_s *__cdecl CG_GetHelicopterAnims(centity_s *cent)
{
    XAnim_s *pAnims; // [esp+4h] [ebp-4h]

    if (!cent->nextState.weapon)
        MyAssertHandler(".\\cgame_mp\\cg_ents_mp.cpp", 230, 0, "%s", "cent->nextState.weapon");
    BG_GetWeaponDef(cent->nextState.weapon);
    pAnims = XAnimCreateAnims("helicopter", 2u, (void *(__cdecl *)(int))Hunk_AllocXAnimClient);
    if (!pAnims)
        MyAssertHandler(".\\cgame_mp\\cg_ents_mp.cpp", 234, 0, "%s", "pAnims");
    XAnimBlend(pAnims, 0, "root", 1u, 1u, 0);
    BG_CreateXAnim(pAnims, 1u, "bh_rotors");
    return pAnims;
}

char *__cdecl CG_AllocAnimTree(int32_t size)
{
    return (char*)MT_Alloc(size, 5);
}

void __cdecl CG_DObjCalcBone(const cpose_t *pose, DObj_s *obj, int32_t boneIndex)
{
    int32_t partBits[4]; // [esp+0h] [ebp-10h] BYREF

    iassert(obj);
    iassert(pose);

    DObjLock(obj);

    if (CL_DObjCreateSkelForBone(obj, boneIndex))
    {
        DObjUnlock(obj);
    }
    else
    {
        DObjGetHierarchyBits(obj, boneIndex, partBits);
        CG_DoControllers(pose, obj, partBits);
        DObjCalcSkel(obj, partBits);
        DObjUnlock(obj);
    }
}

void __cdecl CG_ClearUnion(int32_t localClientNum, centity_s *cent)
{
    switch (cent->pose.eTypeUnion)
    {
    case 1u:
        *(_QWORD *)&cent->pose.player.control = 0;
        cent->pose.turret.barrelPitch = 0.0;
        break;
    case 8u:
    case 9u:
        if (cent->pose.fx.effect)
            FX_ThroughWithEffect(localClientNum, cent->pose.fx.effect);
        *(_QWORD *)&cent->pose.player.control = 0;
        break;
    case 0xBu:
        *(_QWORD *)&cent->pose.player.control = 0;
        *((_QWORD *)&cent->pose.fx + 1) = 0;
        break;
    case 0xCu:
    case 0xEu:
        *(_QWORD *)&cent->pose.player.control = 0;
        *((_QWORD *)&cent->pose.fx + 1) = 0;
        *((_QWORD *)&cent->pose.fx + 2) = 0;
        *((_QWORD *)&cent->pose.fx + 3) = 0;
        *((uint32_t *)&cent->pose.fx + 8) = 0;
        break;
    default:
        break;
    }
    cent->pose.eTypeUnion = 0;
}

void __cdecl CG_SetUnionType(int32_t localClientNum, centity_s *cent)
{
    switch (cent->nextState.eType)
    {
    case 1:
    case 8:
    case 9:
    case 0xB:
    case 0xC:
    case 0xE:
        cent->pose.eTypeUnion = cent->nextState.eType;
        if (cent->pose.eTypeUnion != cent->nextState.eType)
            MyAssertHandler(".\\cgame_mp\\cg_ents_mp.cpp", 1674, 0, "%s", "cent->pose.eTypeUnion == cent->nextState.eType");
        break;
    default:
        cent->pose.eTypeUnion = 0;
        break;
    }
}

void __cdecl CG_UpdatePoseUnion(int32_t localClientNum, centity_s *cent)
{
    CG_ClearUnion(localClientNum, cent);
    CG_SetUnionType(localClientNum, cent);
}

void __cdecl CG_ProcessEntity(int32_t localClientNum, centity_s *cent)
{
    CG_EntityEffects(localClientNum, cent);
    if (cent->nextState.eType != cent->pose.eTypeUnion)
        CG_UpdatePoseUnion(localClientNum, cent);
    switch (cent->nextState.eType)
    {
    case 0:
        CG_General(localClientNum, cent);
        break;
    case 1:
        CG_Player(localClientNum, cent);
        break;
    case 2:
        CG_Corpse(localClientNum, cent);
        break;
    case 3:
        CG_Item(localClientNum, cent);
        break;
    case 4:
        CG_Missile(localClientNum, cent);
        break;
    case 5:
        return;
    case 6:
        goto $LN6_8;
    case 7:
        CG_SoundBlend(localClientNum, cent);
        break;
    case 8:
        CG_Fx(localClientNum, cent);
        break;
    case 9:
        CG_LoopFx(localClientNum, cent);
        break;
    case 0xA:
        CG_PrimaryLight(localClientNum, cent);
        break;
    case 0xB:
        CG_mg42(localClientNum, cent);
        break;
    case 0xC:
    case 0xE:
        CG_VehProcessEntity(localClientNum, cent);
        CG_CompassUpdateVehicleInfo(localClientNum, cent->nextState.number);
        break;
    case 0xD:
        CG_CompassUpdateVehicleInfo(localClientNum, cent->nextState.number);
    $LN6_8:
        CG_ScriptMover(localClientNum, cent);
        break;
    default:
        Com_Error(ERR_DROP, "Bad entity type: %i", cent->nextState.eType);
        break;
    }
}

void __cdecl CG_General(int32_t localClientNum, centity_s *cent)
{
    DObj_s *obj; // [esp+4h] [ebp-18h]
    float lightingOrigin[3]; // [esp+10h] [ebp-Ch] BYREF
    cgs_t *cgs;

    if ((cent->nextState.lerp.eFlags & 0x20) == 0)
    {
        cgs = CG_GetLocalClientStaticGlobals(localClientNum);

        obj = CG_PreProcess_GetDObj(
            localClientNum,
            cent->nextState.number,
            cent->nextState.eType,
            cgs->gameModels[cent->nextState.index.brushmodel]);
        if (obj)
        {
            CG_LockLightingOrigin(cent, lightingOrigin);
            R_AddDObjToScene(obj, &cent->pose, cent->nextState.number, 0, lightingOrigin, 0.0);
        }
    }
}

void __cdecl CG_LockLightingOrigin(centity_s *cent, float *lightingOrigin)
{
    if ((cent->nextState.lerp.eFlags & 0x400) != 0)
    {
        if (0.0 == cent->lightingOrigin[0] && 0.0 == cent->lightingOrigin[1] && 0.0 == cent->lightingOrigin[2])
        {
            cent->lightingOrigin[0] = cent->pose.origin[0];
            cent->lightingOrigin[1] = cent->pose.origin[1];
            cent->lightingOrigin[2] = cent->pose.origin[2];
        }
        *lightingOrigin = cent->lightingOrigin[0];
        lightingOrigin[1] = cent->lightingOrigin[1];
        lightingOrigin[2] = cent->lightingOrigin[2];
    }
    cent->lightingOrigin[0] = 0.0;
    cent->lightingOrigin[1] = 0.0;
    cent->lightingOrigin[2] = 0.0;
    *lightingOrigin = cent->pose.origin[0];
    lightingOrigin[1] = cent->pose.origin[1];
    lightingOrigin[2] = cent->pose.origin[2];
    lightingOrigin[2] = lightingOrigin[2] + 4.0;
}

void __cdecl CG_Item(int32_t localClientNum, centity_s *cent)
{
    DObj_s *obj; // [esp+Ch] [ebp-20h]
    unsigned __int8 weapModel; // [esp+17h] [ebp-15h]
    int32_t weapIdx; // [esp+18h] [ebp-14h]
    float lightingOrigin[3]; // [esp+1Ch] [ebp-10h] BYREF
    WeaponDef *weapDef; // [esp+28h] [ebp-4h]

    if (cent->nextState.index.brushmodel >= 2048)
        Com_Error(ERR_DROP, "Bad item index %i on entity", cent->nextState.index.brushmodel);
    if ((cent->nextState.lerp.eFlags & 0x20) == 0)
    {
        weapIdx = cent->nextState.index.brushmodel % 128;
        weapModel = cent->nextState.index.brushmodel / 128;
        weapDef = BG_GetWeaponDef(weapIdx);
        if (!weapDef)
            MyAssertHandler(".\\cgame_mp\\cg_ents_mp.cpp", 127, 0, "%s", "weapDef");
        if (!weapDef->worldModel[weapModel])
            Com_Error(ERR_DROP, "No XModel loaded for item index %i, weap index %i, model %i (%s)", cent->nextState.index.brushmodel, weapIdx, weapModel, weapDef->szDisplayName);
        obj = CG_PreProcess_GetDObj(
            localClientNum,
            cent->nextState.number,
            cent->nextState.eType,
            weapDef->worldModel[weapModel]);
        if (obj)
        {
            lightingOrigin[0] = cent->pose.origin[0];
            lightingOrigin[1] = cent->pose.origin[1];
            lightingOrigin[2] = cent->pose.origin[2] + 4.0;
            R_AddDObjToScene(obj, &cent->pose, cent->nextState.number, 0, lightingOrigin, 0.0);
        }
    }
}

void __cdecl CG_EntityEffects(int32_t localClientNum, centity_s *cent)
{
    if (cent->nextState.loopSound)
        CG_AddEntityLoopSound(localClientNum, cent);
    if (CG_IsRagdollTrajectory(&cent->currentState.pos))
        CG_CalcEntityLerpPositions(localClientNum, cent);
}

void __cdecl CG_AddEntityLoopSound(int32_t localClientNum, const centity_s *cent)
{
    char *ConfigString; // eax
    char *v3; // eax
    float midpoint[3]; // [esp+0h] [ebp-1Ch] BYREF
    float origin[3]; // [esp+Ch] [ebp-10h] BYREF
    GfxBrushModel *bmodel; // [esp+18h] [ebp-4h]

    if (cent->nextState.solid == 0xFFFFFF)
    {
        bmodel = R_GetBrushModel(cent->nextState.index.brushmodel);
        Vec3Avg(bmodel->bounds[0], bmodel->bounds[1], midpoint);
        Vec3Add(cent->pose.origin, midpoint, origin);
        ConfigString = CL_GetConfigString(localClientNum, cent->nextState.loopSound + 1342);
        CG_PlaySoundAliasByName(localClientNum, cent->nextState.number, origin, ConfigString);
    }
    else
    {
        v3 = CL_GetConfigString(localClientNum, cent->nextState.loopSound + 1342);
        CG_PlaySoundAliasByName(localClientNum, cent->nextState.number, cent->pose.origin, v3);
    }
}

void __cdecl CG_mg42(int32_t localClientNum, centity_s *cent)
{
    DObj_s *obj; // [esp+Ch] [ebp-18h]
    float lightingOrigin[3]; // [esp+14h] [ebp-10h] BYREF
    const entityState_s *ns; // [esp+20h] [ebp-4h]
    const cg_s *cgameGlob;
    const cgs_t *cgs;

    cgs = CG_GetLocalClientStaticGlobals(localClientNum);
    cgameGlob = CG_GetLocalClientGlobals(localClientNum);

    ns = &cent->nextState;

    if ((cent->nextState.lerp.eFlags & 0x20) == 0)
    {
        obj = CG_PreProcess_GetDObj(localClientNum, ns->number, ns->eType, cgs->gameModels[ns->index.brushmodel]);
        if (obj)
        {
            CG_mg42_PreControllers(obj, cent);

            if (!CG_PlayerUsingScopedTurret(localClientNum))
                goto LABEL_12;

            if (cgameGlob->renderingThirdPerson)
                goto LABEL_12;

            if (cgameGlob->predictedPlayerState.viewlocked_entNum != ns->number)
            {
            LABEL_12:
                lightingOrigin[0] = cent->pose.origin[0];
                lightingOrigin[1] = cent->pose.origin[1];
                lightingOrigin[2] = cent->pose.origin[2] + 32.0f;
                R_AddDObjToScene(obj, &cent->pose, ns->number, 4, lightingOrigin, 0.0f);
            }
        }
    }
}

void __cdecl CG_Missile(int32_t localClientNum, centity_s *cent)
{
    DObj_s *obj; // [esp+10h] [ebp-20h]
    entityState_s *s1; // [esp+14h] [ebp-1Ch]
    float lightingOrigin[3]; // [esp+20h] [ebp-10h] BYREF
    WeaponDef *weapDef; // [esp+2Ch] [ebp-4h]

    s1 = &cent->nextState;
    if ((cent->nextState.lerp.eFlags & 0x20) == 0)
    {
        if (cent->nextState.lerp.u.missile.launchTime <= CG_GetLocalClientGlobals(localClientNum)->time)
        {
            if (cent->nextState.weapon >= BG_GetNumWeapons())
                cent->nextState.weapon = 0;
            if (localClientNum)
                MyAssertHandler(
                    "c:\\trees\\cod3\\src\\cgame_mp\\cg_local_mp.h",
                    1095,
                    0,
                    "%s\n\t(localClientNum) = %i",
                    "(localClientNum == 0)",
                    localClientNum);
            weapDef = BG_GetWeaponDef(cent->nextState.weapon);
            if (weapDef->projectileSound)
                CG_PlaySoundAlias(localClientNum, cent->nextState.number, cent->pose.origin, weapDef->projectileSound);
            obj = CG_PreProcess_GetDObj(
                localClientNum,
                cent->nextState.number,
                cent->nextState.eType,
                weapDef->projectileModel);
            if (obj)
            {
                if (weapDef->projTrailEffect && !cent->bTrailMade)
                {
                    cent->bTrailMade = 1;
                    CG_PlayBoltedEffect(localClientNum, weapDef->projTrailEffect, s1->number, scr_const.tag_fx);
                }
                lightingOrigin[0] = cent->pose.origin[0];
                lightingOrigin[1] = cent->pose.origin[1];
                lightingOrigin[2] = cent->pose.origin[2];
                lightingOrigin[2] = lightingOrigin[2] + 4.0;
                R_AddDObjToScene(obj, &cent->pose, s1->number, 0, lightingOrigin, 0.0);
                CG_AddHudGrenade(CG_GetLocalClientGlobals(localClientNum), cent);
            }
        }
    }
}

void __cdecl CG_ScriptMover(int32_t localClientNum, centity_s *cent)
{
    const GfxBrushModel *BrushModel; // eax
    uint32_t materialTime; // [esp+0h] [ebp-1Ch]
    DObj_s *obj; // [esp+4h] [ebp-18h]
    entityState_s *s1; // [esp+8h] [ebp-14h]
    float lightingOrigin[3]; // [esp+10h] [ebp-Ch] BYREF
    cgs_t *cgs;

    s1 = &cent->nextState;
    if ((cent->nextState.lerp.eFlags & 0x20) == 0)
    {
        cgs = CG_GetLocalClientStaticGlobals(localClientNum);
        
        if (cent->nextState.solid == 0xFFFFFF)
        {
            if (cent->nextValid && (cent->nextState.lerp.eFlags & 0x800) != 0)
                AimTarget_ProcessEntity(localClientNum, cent);
            materialTime = s1->number;
            BrushModel = R_GetBrushModel(cent->nextState.index.brushmodel);
            R_AddBrushModelToSceneFromAngles(BrushModel, cent->pose.origin, cent->pose.angles, materialTime);
        }
        else
        {
            obj = CG_PreProcess_GetDObj(
                localClientNum,
                cent->nextState.number,
                cent->nextState.eType,
                cgs->gameModels[cent->nextState.index.brushmodel]);
            if (obj)
            {
                CG_LockLightingOrigin(cent, lightingOrigin);
                R_AddDObjToScene(obj, &cent->pose, s1->number, 0, lightingOrigin, 0.0);
            }
        }
    }
}

void __cdecl CG_SoundBlend(int32_t localClientNum, centity_s *cent)
{
    char *ConfigString; // eax
    char *v3; // eax
    float lerp; // [esp+24h] [ebp-18h]
    snd_alias_t *alias1; // [esp+2Ch] [ebp-10h]
    snd_alias_t *alias0; // [esp+30h] [ebp-Ch]

    if (cent->nextState.eventParms[0])
    {
        if (cent->nextState.eventParms[1])
        {
            CL_GetConfigString(localClientNum, cent->nextState.eventParms[0] + 1342);
            if (CG_ShouldPlaySoundOnLocalClient())
            {
                ConfigString = CL_GetConfigString(localClientNum, cent->nextState.eventParms[0] + 1342);
                alias0 = CL_PickSoundAlias(ConfigString);
                v3 = CL_GetConfigString(localClientNum, cent->nextState.eventParms[1] + 1342);
                alias1 = CL_PickSoundAlias(v3);
                if (alias0)
                {
                    if (alias1)
                    {
                        lerp = (cent->nextState.lerp.u.turret.gunAngles[0] - cent->currentState.u.turret.gunAngles[0]) * CG_GetLocalClientGlobals(localClientNum)->frameInterpolation
                            + cent->currentState.u.turret.gunAngles[0];
                        iassert((lerp >= 0.0f) && (lerp <= 1.0f));
                        SND_PlayBlendedSoundAliases(
                            alias0,
                            alias1,
                            lerp,
                            1.0,
                            (SndEntHandle)cent->nextState.number,
                            cent->pose.origin,
                            0,
                            SASYS_CGAME);
                    }
                }
            }
        }
    }
}

void __cdecl CG_Fx(int32_t localClientNum, centity_s *cent)
{
    if (cent->pose.fx.triggerTime != cent->nextState.time2)
    {
        if (cent->pose.fx.effect)
        {
            FX_AssertAllocatedEffect(localClientNum, cent->pose.fx.effect);
            FX_ThroughWithEffect(localClientNum, cent->pose.fx.effect);
        }
        *(_QWORD *)&cent->pose.player.control = 0;
        cent->pose.fx.effect = CG_StartFx(localClientNum, cent, cent->nextState.time2);
        if (cent->pose.fx.effect)
            cent->pose.fx.triggerTime = cent->nextState.time2;
    }
}

FxEffect *__cdecl CG_StartFx(int32_t localClientNum, centity_s *cent, int32_t startAtTime)
{
    const FxEffectDef *fxDef; // [esp+0h] [ebp-30h]
    int32_t fxId; // [esp+8h] [ebp-28h]
    float axis[3][3]; // [esp+Ch] [ebp-24h] BYREF
    cgs_t *cgs;

    AnglesToAxis(cent->nextState.lerp.apos.trBase, axis);
    fxId = cent->nextState.un1.scale;
    if (fxId < 1 || fxId > 99)
        MyAssertHandler(
            ".\\cgame_mp\\cg_ents_mp.cpp",
            1024,
            0,
            "fxId not in [1, MAX_EFFECT_NAMES - 1]\n\t%i not in [%i, %i]",
            fxId,
            1,
            99);
    cgs = CG_GetLocalClientStaticGlobals(localClientNum);
    fxDef = cgs->fxs[fxId];
    iassert(fxDef);
    return FX_SpawnOrientedEffect(localClientNum, fxDef, startAtTime, cent->pose.origin, axis, 0x3FFu);
}

void __cdecl CG_LoopFx(int32_t localClientNum, centity_s *cent)
{
    double v2; // [esp+0h] [ebp-20h]
    float diff[3]; // [esp+8h] [ebp-18h] BYREF
    const cg_s *cgameGlob; // [esp+14h] [ebp-Ch]
    int32_t period; // [esp+18h] [ebp-8h]
    float cullDist; // [esp+1Ch] [ebp-4h]

    cgameGlob = CG_GetLocalClientGlobals(localClientNum);
    cullDist = cent->nextState.lerp.u.turret.gunAngles[0];
    if (cullDist == 0.0
        || (Vec3Sub(cent->pose.origin, cgameGlob->predictedPlayerState.origin, diff),
            v2 = cullDist * cullDist,
            Vec3LengthSq(diff) < v2))
    {
        period = cent->nextState.lerp.u.loopFx.period;
        iassert(period > 0);
        if (!cent->pose.fx.effect)
        {
            cent->pose.fx.effect = CG_StartFx(localClientNum, cent, cgameGlob->time);
            if (!cent->pose.fx.effect)
                return;
            cent->pose.fx.triggerTime = period + cgameGlob->time;
        }
        while (cgameGlob->time >= cent->pose.fx.triggerTime)
        {
            FX_RetriggerEffect(localClientNum, cent->pose.fx.effect, cent->pose.fx.triggerTime);
            cent->pose.fx.triggerTime += period;
        }
    }
}

void __cdecl CG_PrimaryLight(int32_t localClientNum, centity_s *cent)
{
    const char *v2; // eax
    GfxLight *light; // [esp+2Ch] [ebp-34h]
    float oldColor[4]; // [esp+30h] [ebp-30h] BYREF
    const ComPrimaryLight *refLight; // [esp+40h] [ebp-20h]
    float lightAngles[3]; // [esp+44h] [ebp-1Ch] BYREF
    float newColor[4]; // [esp+50h] [ebp-10h] BYREF
    cg_s *cgameGlob;

    iassert(cent->nextState.eType == ET_PRIMARY_LIGHT);
    iassert(cent->nextState.index.primaryLight != PRIMARY_LIGHT_NONE);
    iassert(comWorld.isInUse);

    if (cent->nextState.index.brushmodel >= comWorld.primaryLightCount)
    {
        iassert(comWorld.isInUse);
        bcassert(cent->nextState.index.primaryLight, Com_GetPrimaryLightCount());
    }

    cgameGlob = CG_GetLocalClientGlobals(localClientNum);
    light = &cgameGlob->refdef.primaryLights[cent->nextState.index.brushmodel];
    refLight = Com_GetPrimaryLight(cent->nextState.index.brushmodel);
    Byte4UnpackRgba((const unsigned __int8 *)&cent->currentState.u, oldColor);
    Byte4UnpackRgba((const unsigned __int8 *)&cent->nextState.lerp.u, newColor);
    Vec3Scale(oldColor, cent->currentState.u.turret.gunAngles[1], oldColor);
    Vec3Scale(newColor, cent->nextState.lerp.u.turret.gunAngles[1], newColor);
    Vec3Lerp(oldColor, newColor, cgameGlob->frameInterpolation, light->color);

    if (refLight->rotationLimit < 1.0)
    {
        BG_EvaluateTrajectory(&cent->nextState.lerp.apos, cgameGlob->time, lightAngles);
        AngleVectors(lightAngles, light->dir, 0, 0);
        light->dir[0] = -light->dir[0];
        light->dir[1] = -light->dir[1];
        light->dir[2] = -light->dir[2];

        if (refLight->rotationLimit > -1.0)
            CG_ClampPrimaryLightDir(light, refLight);
    }

    if (refLight->translationLimit > 0.0)
    {
        BG_EvaluateTrajectory(&cent->nextState.lerp.pos, cgameGlob->time, light->origin);
        CG_ClampPrimaryLightOrigin(light, refLight);
    }

    light->radius = (cent->nextState.lerp.u.turret.gunAngles[2] - cent->currentState.u.turret.gunAngles[2])
        * cgameGlob->frameInterpolation
        + cent->currentState.u.turret.gunAngles[2];
    light->cosHalfFovOuter = (cent->nextState.lerp.u.primaryLight.cosHalfFovOuter
        - cent->currentState.u.primaryLight.cosHalfFovOuter)
        * cgameGlob->frameInterpolation
        + cent->currentState.u.primaryLight.cosHalfFovOuter;
    light->cosHalfFovInner = (cent->nextState.lerp.u.primaryLight.cosHalfFovInner
        - cent->currentState.u.primaryLight.cosHalfFovInner)
        * cgameGlob->frameInterpolation
        + cent->currentState.u.primaryLight.cosHalfFovInner;
    light->exponent = (int)((double)(cent->nextState.lerp.u.primaryLight.colorAndExp[3]
        - cent->currentState.u.primaryLight.colorAndExp[3])
        * cgameGlob->frameInterpolation)
        + cent->currentState.u.primaryLight.colorAndExp[3];
    if (light->cosHalfFovOuter <= 0.0
        || light->cosHalfFovInner <= (double)light->cosHalfFovOuter
        || light->cosHalfFovInner > 1.0)
    {
        MyAssertHandler(
            ".\\cgame_mp\\cg_ents_mp.cpp",
            1169,
            0,
            "%s\n\t%s",
            "0.0f < light->cosHalfFovOuter && light->cosHalfFovOuter < light->cosHalfFovInner && light->cosHalfFovInner <= 1.0f",
            va("%g, %g", light->cosHalfFovOuter, light->cosHalfFovInner));
    }
}

const ComPrimaryLight *__cdecl Com_GetPrimaryLight(uint32_t primaryLightIndex)
{
    if (!comWorld.isInUse)
        MyAssertHandler("c:\\trees\\cod3\\src\\cgame_mp\\../qcommon/com_bsp_api.h", 31, 0, "%s", "comWorld.isInUse");
    if (primaryLightIndex >= comWorld.primaryLightCount)
        MyAssertHandler(
            "c:\\trees\\cod3\\src\\cgame_mp\\../qcommon/com_bsp_api.h",
            32,
            0,
            "primaryLightIndex doesn't index comWorld.primaryLightCount\n\t%i not in [0, %i)",
            primaryLightIndex,
            comWorld.primaryLightCount);
    return &comWorld.primaryLights[primaryLightIndex];
}

void __cdecl CG_ClampPrimaryLightOrigin(GfxLight *light, const ComPrimaryLight *refLight)
{
    float scale; // [esp+Ch] [ebp-20h]
    float v3; // [esp+10h] [ebp-1Ch]
    float v4; // [esp+14h] [ebp-18h]
    float deltaLenSq; // [esp+18h] [ebp-14h]
    float lightDelta[3]; // [esp+20h] [ebp-Ch] BYREF

    Vec3Sub(light->origin, refLight->origin, lightDelta);
    deltaLenSq = Vec3LengthSq(lightDelta);
    v4 = refLight->translationLimit * refLight->translationLimit;
    if (deltaLenSq >= (double)v4)
    {
        v3 = sqrt(deltaLenSq);
        scale = refLight->translationLimit / v3;
        Vec3Mad(refLight->origin, scale, lightDelta, light->origin);
    }
}

void __cdecl CG_ClampPrimaryLightDir(GfxLight *light, const ComPrimaryLight *refLight)
{
    const char *v2; // eax
    float scale; // [esp+14h] [ebp-40h]
    double v4; // [esp+18h] [ebp-3Ch]
    float v5; // [esp+20h] [ebp-34h]
    double rotationLimit; // [esp+24h] [ebp-30h]
    float v7; // [esp+2Ch] [ebp-28h]
    float v8; // [esp+30h] [ebp-24h]
    float v9; // [esp+3Ch] [ebp-18h]
    float cosTurnAngle; // [esp+40h] [ebp-14h]
    float perpendicular[4]; // [esp+44h] [ebp-10h] BYREF

    cosTurnAngle = Vec3Dot(light->dir, refLight->dir);
    if (refLight->rotationLimit > (double)cosTurnAngle)
    {
        scale = -cosTurnAngle;
        Vec3Mad(light->dir, scale, refLight->dir, perpendicular);
        v9 = (1.0 - refLight->rotationLimit * refLight->rotationLimit) / (1.0 - cosTurnAngle * cosTurnAngle);
        v7 = sqrt(v9);
        perpendicular[3] = v7;
        Vec3ScaleMad(refLight->rotationLimit, refLight->dir, v7, perpendicular, light->dir);
        if (!Vec3IsNormalized(light->dir))
        {
            v4 = Vec3Length(light->dir);
            v2 = va("(%g %g %g) len %g", light->dir[0], light->dir[1], light->dir[2], v4);
            MyAssertHandler(".\\cgame_mp\\cg_ents_mp.cpp", 1116, 0, "%s\n\t%s", "Vec3IsNormalized( light->dir )", v2);
        }
        rotationLimit = refLight->rotationLimit;
        v8 = Vec3Dot(light->dir, refLight->dir) - rotationLimit;
        v5 = fabs(v8);
        if (v5 > EQUAL_EPSILON)
            MyAssertHandler(
                ".\\cgame_mp\\cg_ents_mp.cpp",
                1117,
                0,
                "%s",
                "I_fabs( Vec3Dot( light->dir, refLight->dir ) - refLight->rotationLimit ) <= 0.001f");
    }
}

void __cdecl CG_GetPoseOrigin(const cpose_t *pose, float *origin)
{
    if (!pose)
        MyAssertHandler(".\\cgame_mp\\cg_ents_mp.cpp", 1769, 0, "%s", "pose");
    *origin = pose->origin[0];
    origin[1] = pose->origin[1];
    origin[2] = pose->origin[2];
}

void __cdecl CG_GetPoseAngles(const cpose_t *pose, float *angles)
{
    if (!pose)
        MyAssertHandler(".\\cgame_mp\\cg_ents_mp.cpp", 1776, 0, "%s", "pose");
    *angles = pose->angles[0];
    angles[1] = pose->angles[1];
    angles[2] = pose->angles[2];
}

float *__cdecl CG_GetEntityOrigin(int32_t localClientNum, uint32_t entnum)
{
    return CG_GetEntity(localClientNum, entnum)->pose.origin;
}

void __cdecl CG_PredictiveSkinCEntity(GfxSceneEntity *sceneEnt)
{
    cpose_t *pose; // [esp+0h] [ebp-4h]

    if (!sceneEnt)
        MyAssertHandler(".\\cgame_mp\\cg_ents_mp.cpp", 1810, 0, "%s", "sceneEnt");
    pose = sceneEnt->info.pose;
    if (pose->cullIn == 1)
    {
        pose->cullIn = 0;
        R_UpdateXModelBoundsDelayed(sceneEnt);
    }
    else if (pose->cullIn == 2)
    {
        pose->cullIn = 0;
        R_SkinGfxEntityDelayed(sceneEnt);
    }
}

