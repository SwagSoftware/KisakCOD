#include "cg_local.h"
#include "cg_public.h"
#include <xanim/dobj.h>
#include <script/scr_const.h>
#include <EffectsCore/fx_system.h>

#ifdef KISAK_MP
#include <cgame_mp/cg_local_mp.h>
#elif KISAK_SP
#include "cg_ents.h"
#include "cg_main.h"
#endif



void __cdecl CG_Laser_Add(
    centity_s *cent,
    DObj_s *obj,
    cpose_t *pose,
    const float *viewerPos,
    LaserOwnerEnum laserOwner)
{
    unsigned __int8 boneIndex; // [esp+3h] [ebp-31h] BYREF
    orientation_t orient; // [esp+4h] [ebp-30h] BYREF

    boneIndex = -2;
    if (DObjGetBoneIndex(obj, scr_const.tag_laser, &boneIndex)
        || (boneIndex = -2, DObjGetBoneIndex(obj, scr_const.tag_flash, &boneIndex)))
    {
        CG_DObjGetWorldBoneMatrix(pose, obj, boneIndex, orient.axis, orient.origin);
        CG_Laser_Add_Core(cent, obj, &orient, viewerPos, laserOwner);
    }
}

void __cdecl CG_Laser_Add_Core(
    centity_s *cent,
    DObj_s *obj,
    orientation_t *orient,
    const float *viewerPos,
    LaserOwnerEnum laserOwner)
{
    float scale; // [esp+Ch] [ebp-118h]
    float diff[3]; // [esp+18h] [ebp-10Ch] BYREF
    int v7; // [esp+24h] [ebp-100h]
    int v8; // [esp+28h] [ebp-FCh]
    int v9; // [esp+2Ch] [ebp-F8h]
    float laserLightAverageDist; // [esp+30h] [ebp-F4h]
    float laserLength; // [esp+34h] [ebp-F0h]
    float laserLightBeginDist; // [esp+38h] [ebp-ECh]
    float mins[3]; // [esp+3Ch] [ebp-E8h] BYREF
    FxBeam beam; // [esp+48h] [ebp-DCh] BYREF
    float laserLightEndDist; // [esp+7Ch] [ebp-A8h]
    float laserRange; // [esp+80h] [ebp-A4h]
    float distanceBetweenViewerAndLaserEnd; // [esp+84h] [ebp-A0h]
    trace_t traceResults; // [esp+88h] [ebp-9Ch] BYREF
    float maxs[3]; // [esp+B4h] [ebp-70h] BYREF
    float endBrightness; // [esp+C0h] [ebp-64h]
    float laserBegin[3]; // [esp+C4h] [ebp-60h] BYREF
    float laserBeginLightPos[3]; // [esp+D0h] [ebp-54h] BYREF
    float laserEndWidenScale; // [esp+DCh] [ebp-48h]
    float laserEndLightPos[3]; // [esp+E0h] [ebp-44h] BYREF
    unsigned __int8 endColorByte; // [esp+EFh] [ebp-35h]
    FxPostLight postLight; // [esp+F0h] [ebp-34h] BYREF
    float laserEnd[3]; // [esp+118h] [ebp-Ch] BYREF

    Com_Memset((unsigned int *)&traceResults, 0, 44);
    if ((unsigned int)laserOwner > LASER_OWNER_PLAYER)
        MyAssertHandler(
            ".\\cgame\\cg_laser.cpp",
            43,
            0,
            "%s\n\t(laserOwner) = %i",
            "(laserOwner == LASER_OWNER_PLAYER || laserOwner == LASER_OWNER_NON_PLAYER)",
            laserOwner);
    if (laserOwner == LASER_OWNER_PLAYER)
        laserRange = cg_laserRangePlayer->current.value;
    else
        laserRange = cg_laserRange->current.value;
    mins[0] = 0.0;
    mins[1] = 0.0;
    mins[2] = 0.0;
    maxs[0] = 0.0;
    maxs[1] = 0.0;
    maxs[2] = 0.0;
    laserBegin[0] = orient->origin[0];
    laserBegin[1] = orient->origin[1];
    laserBegin[2] = orient->origin[2];
    Vec3Mad(orient->origin, laserRange, orient->axis[0], laserEnd);
    CG_TraceCapsule(&traceResults, laserBegin, mins, maxs, laserEnd, cent->nextState.number, 0x2806831);
    laserLength = traceResults.fraction * laserRange;
    scale = laserLength - cg_laserEndOffset->current.value;
    Vec3Mad(orient->origin, scale, orient->axis[0], laserEnd);
    if (traceResults.fraction > 1.000100016593933)
        MyAssertHandler(".\\cgame\\cg_laser.cpp", 60, 0, "%s", "traceResults.fraction <= 1.0001f");
    beam.begin[0] = laserBegin[0];
    beam.begin[1] = laserBegin[1];
    beam.begin[2] = laserBegin[2];
    beam.end[0] = laserEnd[0];
    beam.end[1] = laserEnd[1];
    beam.end[2] = laserEnd[2];
    beam.beginColor.packed = -1;
    endBrightness = 1.0 - traceResults.fraction;
    v9 = (int)(endBrightness * 255.0);
    if (v9 >= 0)
    {
        if (v9 <= 255)
            v8 = v9;
        else
            v8 = 255;
    }
    else
    {
        v8 = 0;
    }
    endColorByte = v8;
    v7 = ((unsigned __int8)v8 << 24) | 0xFFFFFF;
    beam.endColor.packed = v7;
    Vec3Sub(viewerPos, laserEnd, diff);
    distanceBetweenViewerAndLaserEnd = Vec3Length(diff);
    laserEndWidenScale = distanceBetweenViewerAndLaserEnd * cg_laserFlarePct->current.value * 0.009999999776482582 + 1.0;
    beam.beginRadius = cg_laserRadius->current.value;
    beam.endRadius = cg_laserRadius->current.value * laserEndWidenScale;
    beam.material = cgMedia.laserMaterial;
    beam.segmentCount = 1;
    beam.wiggleDist = 0.0;
    FX_Beam_Add(&beam);
    if (cg_laserLight->current.enabled)
    {
        laserLightBeginDist = cg_laserLightBeginOffset->current.value;
        laserLightEndDist = laserLength - cg_laserLightEndOffset->current.value;
        if ((traceResults.contents & 0x2004000) != 0)
            laserLightEndDist = laserLightEndDist + cg_laserLightBodyTweak->current.value;
        if (laserLightEndDist - laserLightBeginDist < 4.0)
        {
            laserLightAverageDist = (laserLightBeginDist + laserLightEndDist) * 0.5;
            laserLightEndDist = laserLightAverageDist + 2.0;
            laserLightBeginDist = laserLightAverageDist - 2.0;
        }
        Vec3Mad(orient->origin, laserLightBeginDist, orient->axis[0], laserBeginLightPos);
        Vec3Mad(orient->origin, laserLightEndDist, orient->axis[0], laserEndLightPos);
        postLight.begin[0] = laserBeginLightPos[0];
        postLight.begin[1] = laserBeginLightPos[1];
        postLight.begin[2] = laserBeginLightPos[2];
        postLight.end[0] = laserEndLightPos[0];
        postLight.end[1] = laserEndLightPos[1];
        postLight.end[2] = laserEndLightPos[2];
        postLight.radius = cg_laserLightRadius->current.value * laserEndWidenScale;
        postLight.color.packed = -1;
        postLight.material = cgMedia.laserLightMaterial;
        FX_PostLight_Add(&postLight);
    }
}

