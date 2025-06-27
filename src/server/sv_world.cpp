#include "sv_world.h"
#include "sv_game.h"
#include <xanim/dobj.h>
#include <game_mp/g_utils_mp.h>
#include <universal/profile.h>


entityState_s_type_index __cdecl SV_ClipHandleForEntity(const gentity_s *ent)
{
    if (ent->r.bmodel)
        return ent->s.index;
    else
        return (entityState_s_type_index)CM_TempBoxModel(ent->r.mins, ent->r.maxs, ent->r.contents);
}

void __cdecl SV_UnlinkEntity(gentity_s *gEnt)
{
    svEntity_s *ent; // [esp+30h] [ebp-4h]

    ent = SV_SvEntityForGentity(gEnt);
    gEnt->r.linked = 0;
    Profile_Begin(42);
    CM_UnlinkEntity(ent);
    Profile_EndInternal(0);
}

float actorLocationalMins[3] = { -64.0f, -64.0f, -32.0f };
float actorLocationalMaxs[3] = { 64.0f, 64.0f, 72.0f };

void __cdecl SV_LinkEntity(gentity_s *gEnt)
{
    int v1; // eax
    float v2; // [esp+1Ch] [ebp-1B8h]
    float v3; // [esp+20h] [ebp-1B4h]
    float v4; // [esp+24h] [ebp-1B0h]
    float v5; // [esp+28h] [ebp-1ACh]
    float radius; // [esp+80h] [ebp-154h]
    float max; // [esp+84h] [ebp-150h]
    float maxa; // [esp+84h] [ebp-150h]
    int n; // [esp+88h] [ebp-14Ch]
    int m; // [esp+88h] [ebp-14Ch]
    int j; // [esp+8Ch] [ebp-148h]
    int lastLeaf; // [esp+90h] [ebp-144h] BYREF
    unsigned int clipHandle; // [esp+94h] [ebp-140h]
    DObj_s *obj; // [esp+98h] [ebp-13Ch]
    float absmin[3]; // [esp+9Ch] [ebp-138h] BYREF
    float *origin; // [esp+A8h] [ebp-12Ch]
    int cluster; // [esp+ACh] [ebp-128h]
    int k; // [esp+B0h] [ebp-124h]
    unsigned __int16 leafs[128]; // [esp+B4h] [ebp-120h] BYREF
    float *angles; // [esp+1B8h] [ebp-1Ch]
    float absmax[3]; // [esp+1BCh] [ebp-18h] BYREF
    int num_leafs; // [esp+1C8h] [ebp-Ch]
    svEntity_s *ent; // [esp+1CCh] [ebp-8h]
    int i; // [esp+1D0h] [ebp-4h]

    if (!gEnt->r.inuse)
        MyAssertHandler(".\\server\\sv_world.cpp", 108, 0, "%s", "gEnt->r.inuse");
    ent = SV_SvEntityForGentity(gEnt);
    if (gEnt->r.bmodel)
    {
        gEnt->s.solid = 0xFFFFFF;
    }
    else if ((gEnt->r.contents & 0x2000001) != 0)
    {
        i = (int)gEnt->r.maxs[0];
        if (i < 1)
            i = 1;
        if (i > 255)
            i = 255;
        j = (int)(1.0 - gEnt->r.mins[2]);
        if (j < 1)
            j = 1;
        if (j > 255)
            j = 255;
        k = (int)(gEnt->r.maxs[2] + 32.0);
        if (k < 1)
            k = 1;
        if (k > 255)
            k = 255;
        gEnt->s.solid = i | (j << 8) | (k << 16);
    }
    else
    {
        gEnt->s.solid = 0;
    }
    angles = gEnt->r.currentAngles;
    origin = gEnt->r.currentOrigin;
    if ((COERCE_UNSIGNED_INT(gEnt->r.currentAngles[0]) & 0x7F800000) == 0x7F800000
        || (COERCE_UNSIGNED_INT(angles[1]) & 0x7F800000) == 0x7F800000
        || (COERCE_UNSIGNED_INT(angles[2]) & 0x7F800000) == 0x7F800000)
    {
        MyAssertHandler(
            ".\\server\\sv_world.cpp",
            156,
            0,
            "%s",
            "!IS_NAN((angles)[0]) && !IS_NAN((angles)[1]) && !IS_NAN((angles)[2])");
    }
    if ((COERCE_UNSIGNED_INT(*origin) & 0x7F800000) == 0x7F800000
        || (COERCE_UNSIGNED_INT(origin[1]) & 0x7F800000) == 0x7F800000
        || (COERCE_UNSIGNED_INT(origin[2]) & 0x7F800000) == 0x7F800000)
    {
        MyAssertHandler(
            ".\\server\\sv_world.cpp",
            157,
            0,
            "%s",
            "!IS_NAN((origin)[0]) && !IS_NAN((origin)[1]) && !IS_NAN((origin)[2])");
    }
    SnapAngles(angles);
    if (!gEnt->r.bmodel || *angles == 0.0 && angles[1] == 0.0 && angles[2] == 0.0)
    {
        Vec3Add(origin, gEnt->r.mins, gEnt->r.absmin);
        Vec3Add(origin, gEnt->r.maxs, gEnt->r.absmax);
    }
    else if (*angles == 0.0 && angles[2] == 0.0)
    {
        maxa = RadiusFromBounds2D(gEnt->r.mins, gEnt->r.maxs);
        for (m = 0; m < 2; ++m)
        {
            gEnt->r.absmin[m] = origin[m] - maxa;
            gEnt->r.absmax[m] = origin[m] + maxa;
        }
        gEnt->r.absmin[2] = origin[2] + gEnt->r.mins[2];
        gEnt->r.absmax[2] = origin[2] + gEnt->r.maxs[2];
    }
    else
    {
        max = RadiusFromBounds(gEnt->r.mins, gEnt->r.maxs);
        for (n = 0; n < 3; ++n)
        {
            gEnt->r.absmin[n] = origin[n] - max;
            gEnt->r.absmax[n] = origin[n] + max;
        }
    }
    gEnt->r.absmin[0] = gEnt->r.absmin[0] - 1.0;
    gEnt->r.absmin[1] = gEnt->r.absmin[1] - 1.0;
    gEnt->r.absmin[2] = gEnt->r.absmin[2] - 1.0;
    gEnt->r.absmax[0] = gEnt->r.absmax[0] + 1.0;
    gEnt->r.absmax[1] = gEnt->r.absmax[1] + 1.0;
    gEnt->r.absmax[2] = gEnt->r.absmax[2] + 1.0;
    ent->numClusters = 0;
    ent->lastCluster = 0;
    if ((gEnt->r.svFlags & 0x19) == 0)
    {
        Profile_Begin(42);
        num_leafs = CM_BoxLeafnums(gEnt->r.absmin, gEnt->r.absmax, leafs, 128, &lastLeaf);
        Profile_EndInternal(0);
        if (!num_leafs)
        {
            Profile_Begin(42);
            CM_UnlinkEntity(ent);
            Profile_EndInternal(0);
            return;
        }
        for (i = 0; i < num_leafs; ++i)
        {
            cluster = CM_LeafCluster(leafs[i]);
            if (cluster != -1)
            {
                ent->clusternums[ent->numClusters++] = cluster;
                if (ent->numClusters == 16)
                    break;
            }
        }
        if (i != num_leafs)
        {
            v1 = CM_LeafCluster(lastLeaf);
            ent->lastCluster = v1;
        }
    }
    gEnt->r.linked = 1;
    Profile_Begin(42);
    if (gEnt->r.contents)
    {
        clipHandle = SV_ClipHandleForEntity(gEnt).brushmodel;
        obj = Com_GetServerDObj(gEnt->s.number);
        if (obj && (gEnt->r.svFlags & 6) != 0)
        {
            if ((gEnt->r.svFlags & 2) != 0)
            {
                absmin[0] = *origin + actorLocationalMins[0];
                absmin[1] = origin[1] + actorLocationalMins[1];
                absmax[0] = *origin + actorLocationalMaxs[0];
                absmax[1] = origin[1] + actorLocationalMaxs[1];
            }
            else
            {
                radius = DObjGetRadius(obj);
                v4 = *origin - radius;
                v5 = origin[1] - radius;
                absmin[0] = v4;
                absmin[1] = v5;
                v2 = radius + *origin;
                v3 = radius + origin[1];
                absmax[0] = v2;
                absmax[1] = v3;
            }
            CM_LinkEntity(ent, absmin, absmax, clipHandle);
            Profile_EndInternal(0);
        }
        else
        {
            CM_LinkEntity(ent, gEnt->r.absmin, gEnt->r.absmax, clipHandle);
            Profile_EndInternal(0);
        }
    }
    else
    {
        CM_UnlinkEntity(ent);
        Profile_EndInternal(0);
    }
}

void __cdecl SnapAngles(float *vAngles)
{
    float delta; // [esp+10h] [ebp-Ch]
    int rounded; // [esp+14h] [ebp-8h]
    int i; // [esp+18h] [ebp-4h]

    for (i = 0; i < 3; ++i)
    {
        rounded = (int)(vAngles[i]);
        delta = (double)rounded - vAngles[i];
        if (delta * delta < 0.000001f)
            vAngles[i] = (float)rounded;
    }
}

void __cdecl SV_PointTraceToEntity(const pointtrace_t *clip, svEntity_s *check, trace_t *trace)
{
    const char *v3; // eax
    __int64 v4; // [esp+8h] [ebp-F8h]
    float v5; // [esp+1Ch] [ebp-E4h]
    unsigned __int16 number; // [esp+22h] [ebp-DEh]
    float v7; // [esp+28h] [ebp-D8h]
    float v8; // [esp+38h] [ebp-C8h]
    float v9; // [esp+3Ch] [ebp-C4h]
    float v10; // [esp+40h] [ebp-C0h]
    float v11; // [esp+44h] [ebp-BCh]
    float v12; // [esp+48h] [ebp-B8h]
    float v13; // [esp+4Ch] [ebp-B4h]
    float radius; // [esp+58h] [ebp-A8h]
    gentity_s *touch; // [esp+5Ch] [ebp-A4h]
    float entAxis[4][3]; // [esp+60h] [ebp-A0h] BYREF
    unsigned int clipHandle; // [esp+90h] [ebp-70h]
    DObj_s *obj; // [esp+94h] [ebp-6Ch]
    float absmin[3]; // [esp+98h] [ebp-68h] BYREF
    float localStart[3]; // [esp+A4h] [ebp-5Ch] BYREF
    const float *angles; // [esp+B0h] [ebp-50h]
    DObjTrace_s objTrace; // [esp+B4h] [ebp-4Ch] BYREF
    float localEnd[3]; // [esp+D0h] [ebp-30h] BYREF
    float absmax[3]; // [esp+DCh] [ebp-24h] BYREF
    int entnum; // [esp+E8h] [ebp-18h]
    int partBits[4]; // [esp+ECh] [ebp-14h] BYREF
    float oldFraction; // [esp+FCh] [ebp-4h]

    entnum = check - sv.svEntities;
    touch = SV_GentityNum(entnum);
    if ((touch->r.contents & clip->contentmask) == 0
        || clip->ignoreEntParams
        && clip->ignoreEntParams->baseEntity != 1023
        && (clip->ignoreEntParams->ignoreSelf && entnum == clip->ignoreEntParams->baseEntity
            || clip->ignoreEntParams->ignoreParent && entnum == clip->ignoreEntParams->parentEntity
            || EntHandle::isDefined(&touch->r.ownerNum)
            && (clip->ignoreEntParams->ignoreSiblings
                && EntHandle::entnum(&touch->r.ownerNum) == clip->ignoreEntParams->parentEntity
                && entnum != clip->ignoreEntParams->baseEntity
                || clip->ignoreEntParams->ignoreChildren
                && EntHandle::entnum(&touch->r.ownerNum) == clip->ignoreEntParams->baseEntity)))
    {
        return;
    }
    obj = SV_LocationalTraceDObj(clip, touch);
    if (obj)
    {
        if ((touch->r.svFlags & 4) != 0)
        {
            if (!DObjHasContents(obj, clip->contentmask))
                return;
            entAxis[3][0] = touch->r.currentOrigin[0];
            entAxis[3][1] = touch->r.currentOrigin[1];
            entAxis[3][2] = touch->r.currentOrigin[2];
            radius = DObjGetRadius(obj);
            v11 = entAxis[3][0] - radius;
            v12 = entAxis[3][1] - radius;
            v13 = entAxis[3][2] - radius;
            absmin[0] = v11;
            absmin[1] = v12;
            absmin[2] = v13;
            v8 = radius + entAxis[3][0];
            v9 = radius + entAxis[3][1];
            v10 = radius + entAxis[3][2];
            absmax[0] = v8;
            absmax[1] = v9;
            absmax[2] = v10;
        }
        else
        {
            if (!clip->priorityMap)
                MyAssertHandler(".\\server\\sv_world.cpp", 442, 0, "%s", "clip->priorityMap");
            entAxis[3][0] = touch->r.currentOrigin[0];
            entAxis[3][1] = touch->r.currentOrigin[1];
            entAxis[3][2] = touch->r.currentOrigin[2];
            Vec3Add(entAxis[3], actorLocationalMins, absmin);
            Vec3Add(entAxis[3], actorLocationalMaxs, absmax);
        }
        if (!CM_TraceBox(&clip->extents, absmin, absmax, trace->fraction))
        {
            AnglesToAxis(touch->r.currentAngles, entAxis);
            MatrixTransposeTransformVector43(clip->extents.start, entAxis, localStart);
            MatrixTransposeTransformVector43(clip->extents.end, entAxis, localEnd);
            objTrace.fraction = trace->fraction;
            if ((touch->r.svFlags & 4) != 0)
            {
                DObjGeomTracelinePartBits(obj, clip->contentmask, partBits);
                G_DObjCalcPose(touch, partBits);
                DObjGeomTraceline(obj, localStart, localEnd, clip->contentmask, &objTrace);
            }
            else
            {
                DObjTracelinePartBits(obj, partBits);
                G_DObjCalcPose(touch, partBits);
                DObjTraceline(obj, localStart, localEnd, clip->priorityMap, &objTrace);
            }
            if (trace->fraction > objTrace.fraction)
            {
                if (objTrace.fraction >= 1.0 || objTrace.fraction < 0.0)
                    MyAssertHandler(
                        ".\\server\\sv_world.cpp",
                        474,
                        0,
                        "%s\n\t(objTrace.fraction) = %g",
                        "(objTrace.fraction < 1.0f && objTrace.fraction >= 0)",
                        objTrace.fraction);
                trace->fraction = objTrace.fraction;
                trace->surfaceFlags = objTrace.surfaceflags;
                trace->modelIndex = objTrace.modelIndex;
                trace->partName = objTrace.partName;
                trace->partGroup = objTrace.partGroup;
                MatrixTransformVector(objTrace.normal, *(const mat3x3*)&entAxis, trace->normal);
                v7 = Vec3Length(trace->normal) - 1.0;
                v5 = fabs(v7);
                if (v5 >= 0.01 && Vec3Length(trace->normal) >= 0.01)
                {
                    v3 = va("%g %g %g", trace->normal[0], trace->normal[1], trace->normal[2]);
                    MyAssertHandler(
                        ".\\server\\sv_world.cpp",
                        482,
                        0,
                        "%s\n\t%s",
                        "(I_fabs( Vec3Length( trace->normal ) - 1.0f ) < 0.01) || (Vec3Length( trace->normal ) < 0.01)",
                        v3);
                }
                trace->walkable = trace->normal[2] >= 0.699999988079071;
                goto LABEL_40;
            }
        }
    }
    else if ((check->linkcontents & clip->contentmask) != 0
        && !CM_TraceBox(&clip->extents, touch->r.absmin, touch->r.absmax, trace->fraction))
    {
        clipHandle = SV_ClipHandleForEntity(touch).brushmodel;
        angles = touch->r.currentAngles;
        if (!touch->r.bmodel)
            angles = vec3_origin;
        oldFraction = trace->fraction;
        HIDWORD(v4) = clip->contentmask;
        LODWORD(v4) = clipHandle;
        CM_TransformedBoxTrace(
            trace,
            clip->extents.start,
            clip->extents.end,
            vec3_origin,
            vec3_origin,
            v4,
            touch->r.currentOrigin,
            angles);
        if (oldFraction > trace->fraction)
        {
            trace->modelIndex = 0;
            trace->partName = 0;
            trace->partGroup = 0;
        LABEL_40:
            if (touch->s.number != LOWORD(touch->s.number))
                MyAssertHandler(
                    ".\\server\\sv_world.cpp",
                    512,
                    0,
                    "%s",
                    "touch->s.number == static_cast<unsigned short>( touch->s.number )");
            number = touch->s.number;
            if (!trace)
                MyAssertHandler("c:\\trees\\cod3\\src\\server_mp\\../qcommon/cm_public.h", 135, 0, "%s", "trace");
            trace->hitType = TRACE_HITTYPE_ENTITY;
            trace->hitId = number;
            trace->contents = touch->r.contents;
            trace->material = 0;
        }
    }
}

void __cdecl SV_ClipMoveToEntity(const moveclip_t *clip, svEntity_s *check, trace_t *trace)
{
    __int64 v3; // [esp-Ch] [ebp-40h]
    unsigned __int16 number; // [esp+6h] [ebp-2Eh]
    gentity_s *touch; // [esp+8h] [ebp-2Ch]
    unsigned int clipHandle; // [esp+Ch] [ebp-28h]
    float absmin[3]; // [esp+10h] [ebp-24h] BYREF
    const float *angles; // [esp+1Ch] [ebp-18h]
    float absmax[3]; // [esp+20h] [ebp-14h] BYREF
    int entnum; // [esp+2Ch] [ebp-8h]
    float oldFraction; // [esp+30h] [ebp-4h]

    entnum = check - sv.svEntities;
    touch = SV_GentityNum(entnum);
    if ((touch->r.contents & clip->contentmask) != 0
        && (clip->passEntityNum == 1023
            || entnum != clip->passEntityNum
            && (!EntHandle::isDefined(&touch->r.ownerNum)
                || EntHandle::entnum(&touch->r.ownerNum) != clip->passEntityNum
                && EntHandle::entnum(&touch->r.ownerNum) != clip->passOwnerNum)))
    {
        Vec3Add(touch->r.absmin, clip->mins, absmin);
        Vec3Add(touch->r.absmax, clip->maxs, absmax);
        if (!CM_TraceBox(&clip->extents, absmin, absmax, trace->fraction))
        {
            clipHandle = SV_ClipHandleForEntity(touch).brushmodel;
            angles = touch->r.currentAngles;
            if (!touch->r.bmodel)
                angles = vec3_origin;
            oldFraction = trace->fraction;
            HIDWORD(v3) = clip->contentmask;
            LODWORD(v3) = clipHandle;
            CM_TransformedBoxTrace(
                trace,
                clip->extents.start,
                clip->extents.end,
                clip->mins,
                clip->maxs,
                v3,
                touch->r.currentOrigin,
                angles);
            if (oldFraction > (double)trace->fraction)
            {
                if (touch->s.number != LOWORD(touch->s.number))
                    MyAssertHandler(
                        ".\\server\\sv_world.cpp",
                        344,
                        0,
                        "%s",
                        "touch->s.number == static_cast<unsigned short>( touch->s.number )");
                number = touch->s.number;
                if (!trace)
                    MyAssertHandler("c:\\trees\\cod3\\src\\server_mp\\../qcommon/cm_public.h", 135, 0, "%s", "trace");
                trace->hitType = TRACE_HITTYPE_ENTITY;
                trace->hitId = number;
            }
        }
    }
}

int __cdecl SV_PointSightTraceToEntity(const sightpointtrace_t *clip, svEntity_s *check)
{
    float v3; // [esp+10h] [ebp-F0h]
    float v4; // [esp+14h] [ebp-ECh]
    float v5; // [esp+18h] [ebp-E8h]
    float v6; // [esp+1Ch] [ebp-E4h]
    float v7; // [esp+20h] [ebp-E0h]
    float v8; // [esp+24h] [ebp-DCh]
    float radius; // [esp+30h] [ebp-D0h]
    gentity_s *touch; // [esp+34h] [ebp-CCh]
    float entAxis[4][3]; // [esp+3Ch] [ebp-C4h] BYREF
    unsigned int clipHandle; // [esp+6Ch] [ebp-94h]
    TraceExtents extents; // [esp+70h] [ebp-90h] BYREF
    DObj_s *obj; // [esp+94h] [ebp-6Ch]
    float absmin[3]; // [esp+98h] [ebp-68h] BYREF
    float localStart[3]; // [esp+A4h] [ebp-5Ch] BYREF
    const float *angles; // [esp+B0h] [ebp-50h]
    DObjTrace_s objTrace; // [esp+B4h] [ebp-4Ch] BYREF
    float localEnd[3]; // [esp+D0h] [ebp-30h] BYREF
    float absmax[3]; // [esp+DCh] [ebp-24h] BYREF
    int passEntityIdx; // [esp+E8h] [ebp-18h]
    int entnum; // [esp+ECh] [ebp-14h]
    int partBits[4]; // [esp+F0h] [ebp-10h] BYREF

    entnum = check - sv.svEntities;
    touch = SV_GentityNum(entnum);
    if ((touch->r.contents & clip->contentmask) == 0)
        return 0;
    for (passEntityIdx = 0; passEntityIdx < 2; ++passEntityIdx)
    {
        if (clip->passEntityNum[passEntityIdx] != 1023)
        {
            if (entnum == clip->passEntityNum[passEntityIdx])
                return 0;
            if (EntHandle::isDefined(&touch->r.ownerNum)
                && EntHandle::entnum(&touch->r.ownerNum) == clip->passEntityNum[passEntityIdx])
            {
                return 0;
            }
        }
    }
    obj = SV_LocationalSightTraceDObj(clip, touch);
    if (obj)
    {
        if ((touch->r.svFlags & 4) != 0)
        {
            if (!DObjHasContents(obj, clip->contentmask))
                return 0;
            entAxis[3][0] = touch->r.currentOrigin[0];
            entAxis[3][1] = touch->r.currentOrigin[1];
            entAxis[3][2] = touch->r.currentOrigin[2];
            radius = DObjGetRadius(obj);
            v6 = entAxis[3][0] - radius;
            v7 = entAxis[3][1] - radius;
            v8 = entAxis[3][2] - radius;
            absmin[0] = v6;
            absmin[1] = v7;
            absmin[2] = v8;
            v3 = radius + entAxis[3][0];
            v4 = radius + entAxis[3][1];
            v5 = radius + entAxis[3][2];
            absmax[0] = v3;
            absmax[1] = v4;
            absmax[2] = v5;
        }
        else
        {
            if (!clip->priorityMap)
                MyAssertHandler(".\\server\\sv_world.cpp", 634, 0, "%s", "clip->priorityMap");
            entAxis[3][0] = touch->r.currentOrigin[0];
            entAxis[3][1] = touch->r.currentOrigin[1];
            entAxis[3][2] = touch->r.currentOrigin[2];
            Vec3Add(entAxis[3], actorLocationalMins, absmin);
            Vec3Add(entAxis[3], actorLocationalMaxs, absmax);
        }
        extents.start[0] = clip->start[0];
        extents.start[1] = clip->start[1];
        extents.start[2] = clip->start[2];
        extents.end[0] = clip->end[0];
        extents.end[1] = clip->end[1];
        extents.end[2] = clip->end[2];
        CM_CalcTraceExtents(&extents);
        if (CM_TraceBox(&extents, absmin, absmax, 1.0))
            return 0;
        AnglesToAxis(touch->r.currentAngles, entAxis);
        MatrixTransposeTransformVector43(extents.start, entAxis, localStart);
        MatrixTransposeTransformVector43(extents.end, entAxis, localEnd);
        objTrace.fraction = 1.0;
        if ((touch->r.svFlags & 4) != 0)
        {
            DObjGeomTracelinePartBits(obj, clip->contentmask, partBits);
            G_DObjCalcPose(touch, partBits);
            DObjGeomTraceline(obj, localStart, localEnd, clip->contentmask, &objTrace);
        }
        else
        {
            DObjTracelinePartBits(obj, partBits);
            G_DObjCalcPose(touch, partBits);
            DObjTraceline(obj, localStart, localEnd, clip->priorityMap, &objTrace);
        }
        if (objTrace.fraction < 1.0)
            return -1;
    }
    else
    {
        clipHandle = SV_ClipHandleForEntity(touch).brushmodel;
        angles = touch->r.currentAngles;
        if (!touch->r.bmodel)
            angles = vec3_origin;
        if (CM_TransformedBoxSightTrace(
            0,
            clip->start,
            clip->end,
            vec3_origin,
            vec3_origin,
            clipHandle,
            clip->contentmask,
            touch->r.currentOrigin,
            angles))
        {
            return -1;
        }
    }
    return 0;
}

DObj_s *__cdecl SV_LocationalTraceDObj(const pointtrace_t *clip, const gentity_s *touch)
{
    if (!clip->bLocational)
        return 0;
    if ((touch->r.svFlags & 6) == 0)
        return 0;
    if ((touch->r.svFlags & 2) == 0 || clip->priorityMap)
        return Com_GetServerDObj(touch->s.number);
    return 0;
}

int __cdecl SV_ClipSightToEntity(const sightclip_t *clip, svEntity_s *check)
{
    gentity_s *touch; // [esp+0h] [ebp-14h]
    unsigned int clipHandle; // [esp+8h] [ebp-Ch]
    const float *angles; // [esp+Ch] [ebp-8h]
    int entnum; // [esp+10h] [ebp-4h]

    entnum = check - sv.svEntities;
    touch = SV_GentityNum(entnum);
    if ((touch->r.contents & clip->contentmask) == 0)
        return 0;
    if (clip->passEntityNum[0] != 1023)
    {
        if (entnum == clip->passEntityNum[0])
            return 0;
        if (EntHandle::isDefined(&touch->r.ownerNum) && EntHandle::entnum(&touch->r.ownerNum) == clip->passEntityNum[0])
            return 0;
    }
    if (clip->passEntityNum[1] == 1023)
        goto LABEL_15;
    if (entnum == clip->passEntityNum[1])
        return 0;
    if (EntHandle::isDefined(&touch->r.ownerNum) && EntHandle::entnum(&touch->r.ownerNum) == clip->passEntityNum[1])
        return 0;
LABEL_15:
    clipHandle = SV_ClipHandleForEntity(touch).brushmodel;
    angles = touch->r.currentAngles;
    if (!touch->r.bmodel)
        angles = vec3_origin;
    if (CM_TransformedBoxSightTrace(
        0,
        clip->start,
        clip->end,
        clip->mins,
        clip->maxs,
        clipHandle,
        clip->contentmask,
        touch->r.currentOrigin,
        angles))
    {
        return -1;
    }
    else
    {
        return 0;
    }
}

DObj_s *__cdecl SV_LocationalSightTraceDObj(const sightpointtrace_t *clip, const gentity_s *touch)
{
    if (!clip->locational)
        return 0;
    if ((touch->r.svFlags & 6) == 0)
        return 0;
    if ((touch->r.svFlags & 2) == 0 || clip->priorityMap)
        return Com_GetServerDObj(touch->s.number);
    return 0;
}

void __cdecl SV_SetupIgnoreEntParams(IgnoreEntParams *ignoreEntParams, int baseEntity)
{
    gentity_s *base; // [esp+0h] [ebp-4h]

    ignoreEntParams->baseEntity = baseEntity;
    if (baseEntity == 1023)
    {
        ignoreEntParams->parentEntity = -1;
    }
    else
    {
        base = SV_GentityNum(baseEntity);
        if (EntHandle::isDefined(&base->r.ownerNum))
            ignoreEntParams->parentEntity = EntHandle::entnum(&base->r.ownerNum);
        else
            ignoreEntParams->parentEntity = -1;
    }
    ignoreEntParams->ignoreSelf = 1;
    ignoreEntParams->ignoreChildren = 1;
    ignoreEntParams->ignoreSiblings = 1;
    ignoreEntParams->ignoreParent = 0;
}

void __cdecl SV_Trace(
    trace_t *results,
    float *start,
    float *mins,
    float *maxs,
    float *end,
    const IgnoreEntParams *ignoreEntParams,
    int contentmask,
    int locational,
    unsigned __int8 *priorityMap,
    int staticmodels)
{
    gentity_s *v10; // eax
    gentity_s *v11; // eax
    gentity_s *v12; // eax
    gentity_s *v13; // eax
    const char *v14; // eax
    gentity_s *v15; // eax
    gentity_s *v16; // eax
    gentity_s *v17; // eax
    gentity_s *v18; // eax
    const char *v19; // eax
    int v20; // [esp+8h] [ebp-108h]
    int v21; // [esp+Ch] [ebp-104h]
    moveclip_t result; // [esp+78h] [ebp-98h] BYREF
    pointtrace_t clip; // [esp+D0h] [ebp-40h] BYREF
    float delta[3]; // [esp+104h] [ebp-Ch] BYREF

    if (!mins)
        MyAssertHandler(".\\server\\sv_world.cpp", 728, 0, "%s", "mins");
    if (!maxs)
        MyAssertHandler(".\\server\\sv_world.cpp", 729, 0, "%s", "maxs");
    if (!start)
        MyAssertHandler(".\\server\\sv_world.cpp", 730, 0, "%s", "start");
    if (!end)
        MyAssertHandler(".\\server\\sv_world.cpp", 731, 0, "%s", "end");
    if ((COERCE_UNSIGNED_INT(*start) & 0x7F800000) == 0x7F800000
        || (COERCE_UNSIGNED_INT(start[1]) & 0x7F800000) == 0x7F800000
        || (COERCE_UNSIGNED_INT(start[2]) & 0x7F800000) == 0x7F800000)
    {
        MyAssertHandler(
            ".\\server\\sv_world.cpp",
            732,
            0,
            "%s",
            "!IS_NAN((start)[0]) && !IS_NAN((start)[1]) && !IS_NAN((start)[2])");
    }
    if ((COERCE_UNSIGNED_INT(*mins) & 0x7F800000) == 0x7F800000
        || (COERCE_UNSIGNED_INT(mins[1]) & 0x7F800000) == 0x7F800000
        || (COERCE_UNSIGNED_INT(mins[2]) & 0x7F800000) == 0x7F800000)
    {
        MyAssertHandler(
            ".\\server\\sv_world.cpp",
            733,
            0,
            "%s",
            "!IS_NAN((mins)[0]) && !IS_NAN((mins)[1]) && !IS_NAN((mins)[2])");
    }
    if ((COERCE_UNSIGNED_INT(*maxs) & 0x7F800000) == 0x7F800000
        || (COERCE_UNSIGNED_INT(maxs[1]) & 0x7F800000) == 0x7F800000
        || (COERCE_UNSIGNED_INT(maxs[2]) & 0x7F800000) == 0x7F800000)
    {
        MyAssertHandler(
            ".\\server\\sv_world.cpp",
            734,
            0,
            "%s",
            "!IS_NAN((maxs)[0]) && !IS_NAN((maxs)[1]) && !IS_NAN((maxs)[2])");
    }
    if ((COERCE_UNSIGNED_INT(*end) & 0x7F800000) == 0x7F800000
        || (COERCE_UNSIGNED_INT(end[1]) & 0x7F800000) == 0x7F800000
        || (COERCE_UNSIGNED_INT(end[2]) & 0x7F800000) == 0x7F800000)
    {
        MyAssertHandler(
            ".\\server\\sv_world.cpp",
            735,
            0,
            "%s",
            "!IS_NAN((end)[0]) && !IS_NAN((end)[1]) && !IS_NAN((end)[2])");
    }
    Profile_Begin(37);
    CM_BoxTrace(results, start, end, mins, maxs, 0, contentmask);
    if ((COERCE_UNSIGNED_INT(results->fraction) & 0x7F800000) == 0x7F800000)
        MyAssertHandler(".\\server\\sv_world.cpp", 742, 0, "%s", "!IS_NAN(results->fraction)");
    if (results->fraction == 1.0)
    {
        if (!results)
            MyAssertHandler("c:\\trees\\cod3\\src\\server_mp\\../qcommon/cm_public.h", 135, 0, "%s", "trace");
        results->hitType = TRACE_HITTYPE_NONE;
        results->hitId = 0;
    }
    else
    {
        if (!results)
            MyAssertHandler("c:\\trees\\cod3\\src\\server_mp\\../qcommon/cm_public.h", 135, 0, "%s", "trace");
        results->hitType = TRACE_HITTYPE_ENTITY;
        results->hitId = 1022;
    }
    if (results->fraction == 0.0)
        goto LABEL_35;
    if (*mins > (double)*maxs)
        MyAssertHandler(".\\server\\sv_world.cpp", 755, 0, "%s", "maxs[0] >= mins[0]");
    if (mins[1] > (double)maxs[1])
        MyAssertHandler(".\\server\\sv_world.cpp", 756, 0, "%s", "maxs[1] >= mins[1]");
    if (mins[2] > (double)maxs[2])
        MyAssertHandler(".\\server\\sv_world.cpp", 757, 0, "%s", "maxs[2] >= mins[2]");
    if (*maxs - *mins + maxs[1] - mins[1] + maxs[2] - mins[2] == 0.0)
    {
        if (staticmodels)
        {
            CM_PointTraceStaticModels(results, start, end, contentmask);
            if ((COERCE_UNSIGNED_INT(results->fraction) & 0x7F800000) == 0x7F800000)
                MyAssertHandler(".\\server\\sv_world.cpp", 768, 0, "%s", "!IS_NAN(results->fraction)");
            if (results->fraction == 0.0)
            {
            LABEL_35:
                Profile_EndInternal(0);
                return;
            }
        }
        clip.contentmask = contentmask;
        clip.extents.start[0] = *start;
        clip.extents.start[1] = start[1];
        clip.extents.start[2] = start[2];
        clip.extents.end[0] = *end;
        clip.extents.end[1] = end[1];
        clip.extents.end[2] = end[2];
        CM_CalcTraceExtents(&clip.extents);
        clip.ignoreEntParams = ignoreEntParams;
        clip.bLocational = locational;
        clip.priorityMap = priorityMap;
        if (ignoreEntParams->baseEntity != 1023 && ignoreEntParams->parentEntity != -1)
        {
            v10 = SV_GentityNum(ignoreEntParams->baseEntity);
            if (!EntHandle::isDefined(&v10->r.ownerNum)
                || (v11 = SV_GentityNum(ignoreEntParams->baseEntity),
                    ignoreEntParams->parentEntity != EntHandle::entnum(&v11->r.ownerNum)))
            {
                v12 = SV_GentityNum(ignoreEntParams->baseEntity);
                if (EntHandle::isDefined(&v12->r.ownerNum))
                {
                    v13 = SV_GentityNum(ignoreEntParams->baseEntity);
                    v21 = EntHandle::entnum(&v13->r.ownerNum);
                    v14 = va(
                        "base: %d; parent: %d; base's parent: %d\n",
                        ignoreEntParams->baseEntity,
                        ignoreEntParams->parentEntity,
                        v21);
                }
                else
                {
                    v14 = va(
                        "base: %d; parent: %d; base's parent: %d\n",
                        ignoreEntParams->baseEntity,
                        ignoreEntParams->parentEntity,
                        1023);
                }
                MyAssertHandler(
                    ".\\server\\sv_world.cpp",
                    785,
                    0,
                    "%s\n\t%s",
                    "ignoreEntParams->baseEntity == ENTITYNUM_NONE || ignoreEntParams->parentEntity == -1 || (SV_GentityNum( ignore"
                    "EntParams->baseEntity )->r.ownerNum.isDefined() && (uint)ignoreEntParams->parentEntity == SV_GentityNum( ignor"
                    "eEntParams->baseEntity )->r.ownerNum.entnum())",
                    v14);
            }
        }
        CM_PointTraceToEntities(&clip, results);
    }
    else
    {
        if (staticmodels)
            MyAssertHandler(".\\server\\sv_world.cpp", 798, 0, "%s", "!staticmodels");
        if (locational)
            MyAssertHandler(".\\server\\sv_world.cpp", 799, 0, "%s", "!locational");
        result.contentmask = contentmask;
        result.passEntityNum = ignoreEntParams->baseEntity;
        if (ignoreEntParams->baseEntity != 1023 && ignoreEntParams->parentEntity != -1)
        {
            v15 = SV_GentityNum(ignoreEntParams->baseEntity);
            if (!EntHandle::isDefined(&v15->r.ownerNum)
                || (v16 = SV_GentityNum(ignoreEntParams->baseEntity),
                    ignoreEntParams->parentEntity != EntHandle::entnum(&v16->r.ownerNum)))
            {
                v17 = SV_GentityNum(ignoreEntParams->baseEntity);
                if (EntHandle::isDefined(&v17->r.ownerNum))
                {
                    v18 = SV_GentityNum(ignoreEntParams->baseEntity);
                    v20 = EntHandle::entnum(&v18->r.ownerNum);
                    v19 = va(
                        "base: %d; parent: %d; base's parent: %d\n",
                        ignoreEntParams->baseEntity,
                        ignoreEntParams->parentEntity,
                        v20);
                }
                else
                {
                    v19 = va(
                        "base: %d; parent: %d; base's parent: %d\n",
                        ignoreEntParams->baseEntity,
                        ignoreEntParams->parentEntity,
                        1023);
                }
                MyAssertHandler(
                    ".\\server\\sv_world.cpp",
                    804,
                    0,
                    "%s\n\t%s",
                    "ignoreEntParams->baseEntity == ENTITYNUM_NONE || ignoreEntParams->parentEntity == -1 || (SV_GentityNum( ignore"
                    "EntParams->baseEntity )->r.ownerNum.isDefined() && (uint)ignoreEntParams->parentEntity == SV_GentityNum( ignor"
                    "eEntParams->baseEntity )->r.ownerNum.entnum())",
                    v19);
            }
        }
        result.passOwnerNum = ignoreEntParams->parentEntity;
        Vec3Sub(maxs, mins, result.outerSize);
        Vec3Scale(result.outerSize, 0.5, result.outerSize);
        result.maxs[0] = result.outerSize[0];
        result.maxs[1] = result.outerSize[1];
        result.maxs[2] = result.outerSize[2];
        Vec3Scale(result.outerSize, -1.0, result.mins);
        result.outerSize[0] = result.outerSize[0] + 1.0;
        result.outerSize[1] = result.outerSize[1] + 1.0;
        result.outerSize[2] = result.outerSize[2] + 1.0;
        Vec3Add(maxs, mins, delta);
        Vec3Scale(delta, 0.5, delta);
        Vec3Add(start, delta, result.extents.start);
        Vec3Add(end, delta, result.extents.end);
        CM_CalcTraceExtents(&result.extents);
        CM_ClipMoveToEntities(&result, results);
    }
    Profile_EndInternal(0);
}

int __cdecl SV_TracePassed(
    float *start,
    float *mins,
    float *maxs,
    float *end,
    int passEntityNum0,
    int passEntityNum1,
    int contentmask,
    int locational,
    unsigned __int8 *priorityMap,
    int staticmodels)
{
    sightclip_t result; // [esp+84h] [ebp-80h] BYREF
    sightpointtrace_t clip; // [esp+CCh] [ebp-38h] BYREF
    float delta[3]; // [esp+F8h] [ebp-Ch] BYREF

    if (!mins)
        MyAssertHandler(".\\server\\sv_world.cpp", 846, 0, "%s", "mins");
    if (!maxs)
        MyAssertHandler(".\\server\\sv_world.cpp", 847, 0, "%s", "maxs");
    if (!start)
        MyAssertHandler(".\\server\\sv_world.cpp", 848, 0, "%s", "start");
    if (!end)
        MyAssertHandler(".\\server\\sv_world.cpp", 849, 0, "%s", "end");
    if ((COERCE_UNSIGNED_INT(*start) & 0x7F800000) == 0x7F800000
        || (COERCE_UNSIGNED_INT(start[1]) & 0x7F800000) == 0x7F800000
        || (COERCE_UNSIGNED_INT(start[2]) & 0x7F800000) == 0x7F800000)
    {
        MyAssertHandler(
            ".\\server\\sv_world.cpp",
            850,
            0,
            "%s",
            "!IS_NAN((start)[0]) && !IS_NAN((start)[1]) && !IS_NAN((start)[2])");
    }
    if ((COERCE_UNSIGNED_INT(*mins) & 0x7F800000) == 0x7F800000
        || (COERCE_UNSIGNED_INT(mins[1]) & 0x7F800000) == 0x7F800000
        || (COERCE_UNSIGNED_INT(mins[2]) & 0x7F800000) == 0x7F800000)
    {
        MyAssertHandler(
            ".\\server\\sv_world.cpp",
            851,
            0,
            "%s",
            "!IS_NAN((mins)[0]) && !IS_NAN((mins)[1]) && !IS_NAN((mins)[2])");
    }
    if ((COERCE_UNSIGNED_INT(*maxs) & 0x7F800000) == 0x7F800000
        || (COERCE_UNSIGNED_INT(maxs[1]) & 0x7F800000) == 0x7F800000
        || (COERCE_UNSIGNED_INT(maxs[2]) & 0x7F800000) == 0x7F800000)
    {
        MyAssertHandler(
            ".\\server\\sv_world.cpp",
            852,
            0,
            "%s",
            "!IS_NAN((maxs)[0]) && !IS_NAN((maxs)[1]) && !IS_NAN((maxs)[2])");
    }
    if ((COERCE_UNSIGNED_INT(*end) & 0x7F800000) == 0x7F800000
        || (COERCE_UNSIGNED_INT(end[1]) & 0x7F800000) == 0x7F800000
        || (COERCE_UNSIGNED_INT(end[2]) & 0x7F800000) == 0x7F800000)
    {
        MyAssertHandler(
            ".\\server\\sv_world.cpp",
            853,
            0,
            "%s",
            "!IS_NAN((end)[0]) && !IS_NAN((end)[1]) && !IS_NAN((end)[2])");
    }
    Profile_Begin(38);
    if (CM_BoxSightTrace(0, start, end, mins, maxs, 0, contentmask)
        || staticmodels && !CM_PointTraceStaticModelsComplete(start, end, contentmask))
    {
        Profile_EndInternal(0);
        return 0;
    }
    if (*mins > (double)*maxs)
        MyAssertHandler(".\\server\\sv_world.cpp", 874, 0, "%s", "maxs[0] >= mins[0]");
    if (mins[1] > (double)maxs[1])
        MyAssertHandler(".\\server\\sv_world.cpp", 875, 0, "%s", "maxs[1] >= mins[1]");
    if (mins[2] > (double)maxs[2])
        MyAssertHandler(".\\server\\sv_world.cpp", 876, 0, "%s", "maxs[2] >= mins[2]");
    if (*maxs - *mins + maxs[1] - mins[1] + maxs[2] - mins[2] == 0.0)
    {
        clip.contentmask = contentmask;
        clip.start[0] = *start;
        clip.start[1] = start[1];
        clip.start[2] = start[2];
        clip.end[0] = *end;
        clip.end[1] = end[1];
        clip.end[2] = end[2];
        clip.passEntityNum[0] = passEntityNum0;
        clip.passEntityNum[1] = passEntityNum1;
        clip.locational = locational;
        clip.priorityMap = priorityMap;
        if (CM_PointSightTraceToEntities(&clip))
        {
            Profile_EndInternal(0);
            return 0;
        }
    }
    else
    {
        if (locational)
            MyAssertHandler(".\\server\\sv_world.cpp", 901, 0, "%s", "!locational");
        result.contentmask = contentmask;
        result.passEntityNum[0] = passEntityNum0;
        result.passEntityNum[1] = passEntityNum1;
        Vec3Sub(maxs, mins, result.outerSize);
        Vec3Scale(result.outerSize, 0.5, result.outerSize);
        result.maxs[0] = result.outerSize[0];
        result.maxs[1] = result.outerSize[1];
        result.maxs[2] = result.outerSize[2];
        Vec3Scale(result.outerSize, -1.0, result.mins);
        result.outerSize[0] = result.outerSize[0] + 1.0;
        result.outerSize[1] = result.outerSize[1] + 1.0;
        result.outerSize[2] = result.outerSize[2] + 1.0;
        Vec3Add(maxs, mins, delta);
        Vec3Scale(delta, 0.5, delta);
        Vec3Add(start, delta, result.start);
        Vec3Add(end, delta, result.end);
        if (CM_ClipSightTraceToEntities(&result))
        {
            Profile_EndInternal(0);
            return 0;
        }
    }
    Profile_EndInternal(0);
    return 1;
}

void __cdecl SV_SightTrace(
    int *hitNum,
    float *start,
    float *mins,
    float *maxs,
    float *end,
    int passEntityNum0,
    int passEntityNum1,
    int contentmask)
{
    int v8; // eax
    sightclip_t result; // [esp+68h] [ebp-80h] BYREF
    sightpointtrace_t clip; // [esp+B0h] [ebp-38h] BYREF
    float delta[3]; // [esp+DCh] [ebp-Ch] BYREF

    Profile_Begin(233);
    if (!mins)
        MyAssertHandler(".\\server\\sv_world.cpp", 947, 0, "%s", "mins");
    if (!maxs)
        MyAssertHandler(".\\server\\sv_world.cpp", 948, 0, "%s", "maxs");
    if (!start)
        MyAssertHandler(".\\server\\sv_world.cpp", 949, 0, "%s", "start");
    if (!end)
        MyAssertHandler(".\\server\\sv_world.cpp", 950, 0, "%s", "end");
    if ((COERCE_UNSIGNED_INT(*start) & 0x7F800000) == 0x7F800000
        || (COERCE_UNSIGNED_INT(start[1]) & 0x7F800000) == 0x7F800000
        || (COERCE_UNSIGNED_INT(start[2]) & 0x7F800000) == 0x7F800000)
    {
        MyAssertHandler(
            ".\\server\\sv_world.cpp",
            951,
            0,
            "%s",
            "!IS_NAN((start)[0]) && !IS_NAN((start)[1]) && !IS_NAN((start)[2])");
    }
    if ((COERCE_UNSIGNED_INT(*mins) & 0x7F800000) == 0x7F800000
        || (COERCE_UNSIGNED_INT(mins[1]) & 0x7F800000) == 0x7F800000
        || (COERCE_UNSIGNED_INT(mins[2]) & 0x7F800000) == 0x7F800000)
    {
        MyAssertHandler(
            ".\\server\\sv_world.cpp",
            952,
            0,
            "%s",
            "!IS_NAN((mins)[0]) && !IS_NAN((mins)[1]) && !IS_NAN((mins)[2])");
    }
    if ((COERCE_UNSIGNED_INT(*maxs) & 0x7F800000) == 0x7F800000
        || (COERCE_UNSIGNED_INT(maxs[1]) & 0x7F800000) == 0x7F800000
        || (COERCE_UNSIGNED_INT(maxs[2]) & 0x7F800000) == 0x7F800000)
    {
        MyAssertHandler(
            ".\\server\\sv_world.cpp",
            953,
            0,
            "%s",
            "!IS_NAN((maxs)[0]) && !IS_NAN((maxs)[1]) && !IS_NAN((maxs)[2])");
    }
    if ((COERCE_UNSIGNED_INT(*end) & 0x7F800000) == 0x7F800000
        || (COERCE_UNSIGNED_INT(end[1]) & 0x7F800000) == 0x7F800000
        || (COERCE_UNSIGNED_INT(end[2]) & 0x7F800000) == 0x7F800000)
    {
        MyAssertHandler(
            ".\\server\\sv_world.cpp",
            954,
            0,
            "%s",
            "!IS_NAN((end)[0]) && !IS_NAN((end)[1]) && !IS_NAN((end)[2])");
    }
    *hitNum = CM_BoxSightTrace(*hitNum, start, end, mins, maxs, 0, contentmask);
    if (*hitNum)
    {
        Profile_EndInternal(0);
    }
    else
    {
        if (*mins > (double)*maxs)
            MyAssertHandler(".\\server\\sv_world.cpp", 964, 0, "%s", "maxs[0] >= mins[0]");
        if (mins[1] > (double)maxs[1])
            MyAssertHandler(".\\server\\sv_world.cpp", 965, 0, "%s", "maxs[1] >= mins[1]");
        if (mins[2] > (double)maxs[2])
            MyAssertHandler(".\\server\\sv_world.cpp", 966, 0, "%s", "maxs[2] >= mins[2]");
        if (*maxs - *mins + maxs[1] - mins[1] + maxs[2] - mins[2] == 0.0)
        {
            clip.contentmask = contentmask;
            clip.start[0] = *start;
            clip.start[1] = start[1];
            clip.start[2] = start[2];
            clip.end[0] = *end;
            clip.end[1] = end[1];
            clip.end[2] = end[2];
            clip.passEntityNum[0] = passEntityNum0;
            clip.passEntityNum[1] = passEntityNum1;
            clip.locational = 0;
            v8 = CM_PointSightTraceToEntities(&clip);
        }
        else
        {
            result.contentmask = contentmask;
            result.passEntityNum[0] = passEntityNum0;
            result.passEntityNum[1] = passEntityNum1;
            Vec3Sub(maxs, mins, result.outerSize);
            Vec3Scale(result.outerSize, 0.5, result.outerSize);
            result.maxs[0] = result.outerSize[0];
            result.maxs[1] = result.outerSize[1];
            result.maxs[2] = result.outerSize[2];
            Vec3Scale(result.outerSize, -1.0, result.mins);
            result.outerSize[0] = result.outerSize[0] + 1.0;
            result.outerSize[1] = result.outerSize[1] + 1.0;
            result.outerSize[2] = result.outerSize[2] + 1.0;
            Vec3Add(maxs, mins, delta);
            Vec3Scale(delta, 0.5, delta);
            Vec3Add(start, delta, result.start);
            Vec3Add(end, delta, result.end);
            v8 = CM_ClipSightTraceToEntities(&result);
        }
        *hitNum = v8;
        Profile_EndInternal(0);
    }
}

int __cdecl SV_SightTraceToEntity(float *start, float *mins, float *maxs, float *end, int entityNum, int contentmask)
{
    double v7; // st7
    unsigned int clipHandle; // [esp+34h] [ebp-2Ch]
    float boxmins[3]; // [esp+38h] [ebp-28h]
    const float *origin; // [esp+44h] [ebp-1Ch]
    float boxmaxs[3]; // [esp+48h] [ebp-18h]
    const float *angles; // [esp+54h] [ebp-Ch]
    gentity_s *ent; // [esp+58h] [ebp-8h]
    int i; // [esp+5Ch] [ebp-4h]

    ent = SV_GentityNum(entityNum);
    if ((ent->r.contents & contentmask) == 0)
        return 0;
    if (!mins)
        MyAssertHandler(".\\server\\sv_world.cpp", 1036, 0, "%s", "mins");
    if (!maxs)
        MyAssertHandler(".\\server\\sv_world.cpp", 1037, 0, "%s", "maxs");
    if (!start)
        MyAssertHandler(".\\server\\sv_world.cpp", 1038, 0, "%s", "start");
    if (!end)
        MyAssertHandler(".\\server\\sv_world.cpp", 1039, 0, "%s", "end");
    if ((COERCE_UNSIGNED_INT(*start) & 0x7F800000) == 0x7F800000
        || (COERCE_UNSIGNED_INT(start[1]) & 0x7F800000) == 0x7F800000
        || (COERCE_UNSIGNED_INT(start[2]) & 0x7F800000) == 0x7F800000)
    {
        MyAssertHandler(
            ".\\server\\sv_world.cpp",
            1040,
            0,
            "%s",
            "!IS_NAN((start)[0]) && !IS_NAN((start)[1]) && !IS_NAN((start)[2])");
    }
    if ((COERCE_UNSIGNED_INT(*mins) & 0x7F800000) == 0x7F800000
        || (COERCE_UNSIGNED_INT(mins[1]) & 0x7F800000) == 0x7F800000
        || (COERCE_UNSIGNED_INT(mins[2]) & 0x7F800000) == 0x7F800000)
    {
        MyAssertHandler(
            ".\\server\\sv_world.cpp",
            1041,
            0,
            "%s",
            "!IS_NAN((mins)[0]) && !IS_NAN((mins)[1]) && !IS_NAN((mins)[2])");
    }
    if ((COERCE_UNSIGNED_INT(*maxs) & 0x7F800000) == 0x7F800000
        || (COERCE_UNSIGNED_INT(maxs[1]) & 0x7F800000) == 0x7F800000
        || (COERCE_UNSIGNED_INT(maxs[2]) & 0x7F800000) == 0x7F800000)
    {
        MyAssertHandler(
            ".\\server\\sv_world.cpp",
            1042,
            0,
            "%s",
            "!IS_NAN((maxs)[0]) && !IS_NAN((maxs)[1]) && !IS_NAN((maxs)[2])");
    }
    if ((COERCE_UNSIGNED_INT(*end) & 0x7F800000) == 0x7F800000
        || (COERCE_UNSIGNED_INT(end[1]) & 0x7F800000) == 0x7F800000
        || (COERCE_UNSIGNED_INT(end[2]) & 0x7F800000) == 0x7F800000)
    {
        MyAssertHandler(
            ".\\server\\sv_world.cpp",
            1043,
            0,
            "%s",
            "!IS_NAN((end)[0]) && !IS_NAN((end)[1]) && !IS_NAN((end)[2])");
    }
    for (i = 0; i < 3; ++i)
    {
        if (start[i] >= (double)end[i])
        {
            boxmins[i] = end[i] + mins[i] - 1.0;
            v7 = start[i] + maxs[i] + 1.0;
        }
        else
        {
            boxmins[i] = start[i] + mins[i] - 1.0;
            v7 = end[i] + maxs[i] + 1.0;
        }
        boxmaxs[i] = v7;
    }
    if (boxmaxs[0] < (double)ent->r.absmin[0]
        || boxmaxs[1] < (double)ent->r.absmin[1]
        || boxmaxs[2] < (double)ent->r.absmin[2]
        || boxmins[0] > (double)ent->r.absmax[0]
        || boxmins[1] > (double)ent->r.absmax[1]
        || boxmins[2] > (double)ent->r.absmax[2])
    {
        return 0;
    }
    clipHandle = SV_ClipHandleForEntity(ent).brushmodel;
    origin = ent->r.currentOrigin;
    angles = ent->r.currentAngles;
    if (!ent->r.bmodel)
        angles = vec3_origin;
    if (CM_TransformedBoxSightTrace(0, start, end, mins, maxs, clipHandle, contentmask, origin, angles))
        return -1;
    else
        return 0;
}

int __cdecl SV_PointContents(float *p, int passEntityNum, int contentmask)
{
    int v3; // eax
    int entityList[1025]; // [esp+0h] [ebp-1018h] BYREF
    int v6; // [esp+1004h] [ebp-14h]
    unsigned int model; // [esp+1008h] [ebp-10h]
    gentity_s *ent; // [esp+100Ch] [ebp-Ch]
    int v9; // [esp+1010h] [ebp-8h]
    int i; // [esp+1014h] [ebp-4h]

    v6 = CM_PointContents(p, 0);
    v9 = CM_AreaEntities(p, p, entityList, 1024, contentmask);
    for (i = 0; i < v9; ++i)
    {
        if (entityList[i] != passEntityNum)
        {
            ent = SV_GentityNum(entityList[i]);
            model = SV_ClipHandleForEntity(ent).brushmodel;
            v3 = CM_TransformedPointContents(p, model, ent->r.currentOrigin, ent->r.currentAngles);
            v6 |= v3;
        }
    }
    return contentmask & v6;
}

