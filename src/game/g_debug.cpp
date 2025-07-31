#include "game_public.h"
#include <client/client.h>
#include <cgame/cg_public.h>

#include <universal/com_math.h>
#include <qcommon/qcommon.h>
#include <xanim/xanim.h>
#ifdef KISAK_SP
#include "g_main.h"
#endif

void __cdecl G_DebugLine(const float *start, const float *end, const float *color, int depthTest)
{
    CL_AddDebugLine(start, end, color, depthTest, 0, 1);
}

void __cdecl G_DebugLineWithDuration(
    const float *start,
    const float *end,
    const float *color,
    int depthTest,
    int duration)
{
    CL_AddDebugLine(start, end, color, depthTest, duration, 1);
}

void __cdecl G_DebugStar(const float *point, const float *color)
{
    CL_AddDebugStar(point, color, 0, 1);
}

void __cdecl G_DebugStarWithText(
    const float *point,
    const float *starColor,
    const float *textColor,
    char *string,
    float fontsize)
{
    CL_AddDebugStarWithText(point, starColor, textColor, string, fontsize, 0, 1);
}

void __cdecl G_DebugBox(
    const float *origin,
    const float *mins,
    const float *maxs,
    float yaw,
    const float *color,
    int depthTest,
    int duration)
{
    float v7; // [esp+0h] [ebp-94h]
    float v8; // [esp+10h] [ebp-84h]
    unsigned int j; // [esp+14h] [ebp-80h]
    float rotated; // [esp+18h] [ebp-7Ch]
    float rotated_4; // [esp+1Ch] [ebp-78h]
    unsigned int i; // [esp+24h] [ebp-70h]
    unsigned int ia; // [esp+24h] [ebp-70h]
    float fCos; // [esp+28h] [ebp-6Ch]
    float v[25]; // [esp+2Ch] [ebp-68h] BYREF
    float fSin; // [esp+90h] [ebp-4h]

    v8 = yaw * 0.01745329238474369;
    fCos = cos(v8);
    fSin = sin(v8);
    for (i = 0; i < 8; ++i)
    {
        for (j = 0; j < 3; ++j)
        {
            if ((i & (1 << j)) != 0)
                v7 = maxs[j];
            else
                v7 = mins[j];
            v[3 * i + j] = v7;
        }
        rotated = v[3 * i] * fCos - v[3 * i + 1] * fSin;
        rotated_4 = v[3 * i] * fSin + v[3 * i + 1] * fCos;
        v[3 * i] = rotated;
        v[3 * i + 1] = rotated_4;
        Vec3Add(&v[3 * i], origin, &v[3 * i]);
    }
    for (ia = 0; ia < 0xC; ++ia)
        G_DebugLineWithDuration(&v[3 * iEdgePairs[ia][0]], &v[3 * iEdgePairs[ia][1]], color, depthTest, duration);
}

void __cdecl G_DebugCircle(
    const float *center,
    float radius,
    const float *color,
    int depthTest,
    int onGround,
    int duration)
{
    float eyepos[3]; // [esp+18h] [ebp-18h] BYREF
    float dir[3]; // [esp+24h] [ebp-Ch] BYREF

    if (onGround)
    {
        dir[0] = 0.0;
        dir[1] = 0.0;
        dir[2] = 1.0;
    }
    else
    {
        eyepos[0] = level.clients->ps.origin[0];
        eyepos[1] = level.clients->ps.origin[1];
        eyepos[2] = level.clients->ps.origin[2];
        eyepos[2] = eyepos[2] + level.clients->ps.viewHeightCurrent;
        Vec3Sub(center, eyepos, dir);
    }
    G_DebugCircleEx(center, radius, dir, color, depthTest, duration);
}

void __cdecl G_DebugCircleEx(
    const float *center,
    float radius,
    const float *dir,
    const float *color,
    int depthTest,
    int duration)
{
    float fAngle; // [esp+1Ch] [ebp-F4h]
    float fCos; // [esp+20h] [ebp-F0h]
    float fCosa; // [esp+20h] [ebp-F0h]
    float fSin; // [esp+24h] [ebp-ECh]
    float fSina; // [esp+24h] [ebp-ECh]
    float normal[3]; // [esp+28h] [ebp-E8h] BYREF
    float right[3]; // [esp+34h] [ebp-DCh] BYREF
    float up[3]; // [esp+40h] [ebp-D0h] BYREF
    unsigned int i; // [esp+4Ch] [ebp-C4h]
    float v[16][3]; // [esp+50h] [ebp-C0h] BYREF

    Vec3NormalizeTo(dir, normal);
    PerpendicularVector(normal, right);
    Vec3Cross(normal, right, up);
    for (i = 0; i < 0x10; ++i)
    {
        fAngle = (double)i * 0.3926990926265717;
        fCos = cos(fAngle);
        fSin = sin(fAngle);
        fSina = fSin * radius;
        fCosa = fCos * radius;
        Vec3Mad(center, fSina, up, v[i]);
        Vec3Mad(v[i], fCosa, right, v[i]);
    }
    for (i = 0; i < 0x10; ++i)
        G_DebugLineWithDuration(v[i], v[(i + 1) % 0x10], color, depthTest, duration);
}

#ifdef KISAK_SP
struct DebugDrawBrushInfo
{
    int depthTest;
    int duration;
    float transform[4][3];
};

DebugDrawBrushInfo g_debugDrawBrushInfo;

void DrawBrushPoly(int numPoints, float (*points)[3], const float *color)
{
    int v5; // r30
    const float *v6; // r29
    float v7[4]; // [sp+50h] [-50h] BYREF
    float v8[16]; // [sp+60h] [-40h] BYREF

    v5 = numPoints - 1;
    MatrixTransformVector43((const float *)points, g_debugDrawBrushInfo.transform, v8);
    if (v5 > 0)
    {
        v6 = &(*points)[3];
        do
        {
            MatrixTransformVector43(v6, g_debugDrawBrushInfo.transform, v7);
            CL_AddDebugLine(v8, v7, color, g_debugDrawBrushInfo.depthTest, g_debugDrawBrushInfo.duration, 1);
            v8[0] = v7[0];
            --v5;
            v6 += 3;
            v8[1] = v7[1];
            v8[2] = v7[2];
        } while (v5);
    }
    MatrixTransformVector43((const float *)points, g_debugDrawBrushInfo.transform, v7);
    CL_AddDebugLine(v8, v7, color, g_debugDrawBrushInfo.depthTest, g_debugDrawBrushInfo.duration, 1);
}

void G_DebugDrawBrush_r(cLeafBrushNode_s *node, const float *color)
{
    int v4; // r30
    unsigned __int16 *brushes; // r31

    while (1)
    {
        iassert(node);
        if (node->leafBrushCount > 0)
            break;
        ++node;
    }
    v4 = 0;
    brushes = node->data.leaf.brushes;
    do
    {
        CM_ShowSingleBrushCollision((const cbrush_t *)((char *)cm.brushes + 16 * *brushes + 16 * __ROL4__(*brushes, 2)), color, DrawBrushPoly);
        ++v4;
        ++brushes;
    } while (v4 < node->leafBrushCount);
}

#ifdef KISAK_SP
void G_DebugDrawBrushModel(gentity_s *entity, const float *color, int depthTest, int duration)
{
    unsigned int v8; // r29
    cmodel_t *v9; // r31

    iassert(entity);

    v8 = entity->s.index.item;

    if (entity->s.index.item)
    {
        g_debugDrawBrushInfo.depthTest = depthTest;
        g_debugDrawBrushInfo.duration = duration;
        AnglesToAxis(entity->r.currentAngles, g_debugDrawBrushInfo.transform);
        g_debugDrawBrushInfo.transform[3][0] = entity->r.currentOrigin[0];
        g_debugDrawBrushInfo.transform[3][1] = entity->r.currentOrigin[1];
        g_debugDrawBrushInfo.transform[3][2] = entity->r.currentOrigin[2];
        v9 = CM_ClipHandleToModel(v8);
        if (!v9)
            MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\g_debug.cpp", 319, 0, "%s", "cmod");
        G_DebugDrawBrush_r(&cm.leafbrushNodes[v9->leaf.leafBrushNode], color);
    }
}
#endif

void G_DebugPlane(
    const float * const normal,
    float dist,
    const float * const origin,
    const float * const color,
    float size,
    int depthTest,
    int duration)
{
    double v14; // fp13
    double v15; // fp29
    double v16; // fp1
    double v17; // fp10
    double v18; // fp9
    double v19; // fp13
    double v20; // fp0
    float v21; // [sp+50h] [-90h] BYREF
    float v22; // [sp+54h] [-8Ch]
    float v23[3]; // [sp+58h] [-88h] BYREF
    float v26[3]; // [sp+68h] [-78h] BYREF
    float v29[3]; // [sp+78h] [-68h] BYREF
    float v32[3]; // [sp+88h] [-58h] BYREF

    v14 = (float)(color[1] * normal[1]);
    v22 = normal[1];
    v15 = (float)((float)dist - (float)((float)(*color * *normal) + (float)v14));
    v21 = *normal;
    v16 = Vec2Normalize(&v21);
    v17 = *color;
    v18 = color[1];
    v32[2] = (float)((float)size * (float)0.5) + color[2];
    v23[2] = v32[2];
    v19 = -(float)((float)v15 / (float)v16);
    v32[0] = (float)(v21 * (float)((float)v15 / (float)v16)) + (float)v17;
    v32[1] = (float)(v22 * (float)((float)v15 / (float)v16)) + (float)v18;
    v23[0] = (float)((float)v19 * v21) + v32[0];
    v23[1] = (float)((float)v19 * v22) + v32[1];
    CL_AddDebugLine(v32, v23, color, depthTest, duration, 1);

    v20 = -v22;
    v26[2] = color[2];
    v23[2] = v26[2];
    v26[0] = (float)((float)((float)size * (float)0.5) * (float)v20) + v32[0];
    v26[1] = (float)((float)((float)size * (float)0.5) * v21) + v32[1];
    v23[0] = (float)((float)((float)size * (float)-0.5) * (float)v20) + v32[0];
    v23[2] = (float)((float)((float)size * (float)-0.5) * v21) + v32[1];
    CL_AddDebugLine(v26, v23, color, depthTest, duration, 1);

    v26[2] = v26[2] + (float)size;
    v23[2] = v23[2] + (float)size;
    CL_AddDebugLine(v26, v23, color, depthTest, duration, 1);

    v29[0] = v26[0];
    v29[1] = v26[1];
    v29[2] = v26[2] - (float)size;
    CL_AddDebugLine(v26, v29, color, depthTest, duration, 1);

    v29[0] = v23[0];
    v29[1] = v23[1];
    v29[2] = v23[2] - (float)size;
    CL_AddDebugLine(v23, v29, color, depthTest, duration, 1);
}
#endif