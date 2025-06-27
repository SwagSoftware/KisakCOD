#include "rb_debug.h"
#include "r_gfx.h"
#include "rb_shade.h"
#include "rb_state.h"
#include "r_utils.h"
#include "rb_stats.h"
#include "rb_showcollision.h"

GfxPointVertex g_debugPolyVerts[2725];
GfxPointVertex g_debugExternLineVerts[2725];
GfxPointVertex g_debugLineVerts[2725];

void __cdecl RB_AddPlumeStrings(const GfxViewParms *viewParms)
{
    char *v1; // eax
    float v2; // [esp+Ch] [ebp-20h]
    float v3; // [esp+10h] [ebp-1Ch]
    int dt; // [esp+14h] [ebp-18h]
    float org[3]; // [esp+18h] [ebp-14h] BYREF
    int plumeIndex; // [esp+24h] [ebp-8h]
    float wiggle; // [esp+28h] [ebp-4h]

    for (plumeIndex = 0; plumeIndex < backEndData->debugGlobals.plumeCount; ++plumeIndex)
    {
        dt = gfxCmdBufSourceState.sceneDef.time - backEndData->debugGlobals.plumes[plumeIndex].startTime;
        if (dt >= 0 && dt <= backEndData->debugGlobals.plumes[plumeIndex].duration)
        {
            backEndData->debugGlobals.plumes[plumeIndex].color[3] = 1.0;
            if (2 * dt > backEndData->debugGlobals.plumes[plumeIndex].duration)
                backEndData->debugGlobals.plumes[plumeIndex].color[3] = 2.0
                - (dt + dt)
                / backEndData->debugGlobals.plumes[plumeIndex].duration;
            v3 = plumeIndex + dt * 0.01256637088954449;
            v2 = sin(v3);
            wiggle = v2 * 4.0;
            Vec3Mad(backEndData->debugGlobals.plumes[plumeIndex].origin, wiggle, viewParms->axis[1], org);
            org[2] = dt * 0.06400000303983688 + org[2];
            v1 = va("%i", backEndData->debugGlobals.plumes[plumeIndex].score);
            R_AddDebugString((DebugGlobals*)&backEndData->debugGlobals, org, backEndData->debugGlobals.plumes[plumeIndex].color, 0.5, v1);
        }
    }
}

void RB_DrawPolyInteriors()
{
    GfxDebugPoly *poly; // [esp+0h] [ebp-18h]
    float *verts; // [esp+4h] [ebp-14h]
    int polyIndex; // [esp+8h] [ebp-10h]
    int vertIndex; // [esp+10h] [ebp-8h]
    int vertIndexa; // [esp+10h] [ebp-8h]
    GfxColor color; // [esp+14h] [ebp-4h] BYREF

    if (backEndData->debugGlobals.polyCount <= 0)
        MyAssertHandler(".\\rb_debug.cpp", 83, 0, "%s", "backEndData->debugGlobals.polyCount > 0");
    RB_BeginSurface(rgp.whiteMaterial, TECHNIQUE_UNLIT);
    R_TrackPrims(&gfxCmdBufState, GFX_PRIM_STATS_DEBUG);
    for (polyIndex = 0; polyIndex < backEndData->debugGlobals.polyCount; ++polyIndex)
    {
        poly = &backEndData->debugGlobals.polys[polyIndex];
        verts = backEndData->debugGlobals.verts[poly->firstVert];
        R_ConvertColorToBytes(poly->color, (unsigned int*)&color);
        RB_CheckTessOverflow(poly->vertCount, 3 * (poly->vertCount - 2));
        for (vertIndex = 0; vertIndex < poly->vertCount; ++vertIndex)
            RB_SetPolyVert(&verts[3 * vertIndex], color, vertIndex + tess.vertexCount);
        for (vertIndexa = 2; vertIndexa < poly->vertCount; ++vertIndexa)
        {
            tess.indices[tess.indexCount] = tess.vertexCount;
            tess.indices[tess.indexCount + 1] = vertIndexa + LOWORD(tess.vertexCount);
            tess.indices[tess.indexCount + 2] = LOWORD(tess.vertexCount) + vertIndexa - 1;
            tess.indexCount += 3;
        }
        tess.vertexCount += poly->vertCount;
    }
    RB_EndTessSurface();
}

int __cdecl RB_AddDebugLine(
    const float *start,
    const float *end,
    const float *color,
    bool depthTest,
    int vertCount,
    int vertLimit,
    GfxPointVertex *verts)
{
    GfxPointVertex *v8; // [esp+4h] [ebp-8h]
    GfxPointVertex *v9; // [esp+8h] [ebp-4h]

    if (vertCount + 2 > vertLimit)
    {
        RB_DrawLines3D(vertCount / 2, 1, verts, depthTest);
        vertCount = 0;
    }
    R_ConvertColorToBytes(color, (unsigned int*)verts[vertCount].color);
    *verts[vertCount + 1].color = *verts[vertCount].color;
    v9 = &verts[vertCount];
    v9->xyz[0] = *start;
    v9->xyz[1] = start[1];
    v9->xyz[2] = start[2];
    v8 = &verts[vertCount + 1];
    v8->xyz[0] = *end;
    v8->xyz[1] = end[1];
    v8->xyz[2] = end[2];
    return vertCount + 2;
}

int __cdecl RB_EndDebugLines(bool depthTest, int vertCount, const GfxPointVertex *verts)
{
    if (vertCount)
        RB_DrawLines3D(vertCount, 1, verts, depthTest);
    return 0;
}

int RB_DrawPolyOutlines()
{
    GfxDebugPoly *poly; // [esp+0h] [ebp-1Ch]
    float *polyVerts; // [esp+4h] [ebp-18h]
    int polyIndex; // [esp+Ch] [ebp-10h]
    int vertCount; // [esp+10h] [ebp-Ch]
    int vertIndex; // [esp+14h] [ebp-8h]
    int vertIndexPrev; // [esp+18h] [ebp-4h]

    if (backEndData->debugGlobals.polyCount <= 0)
        MyAssertHandler(".\\rb_debug.cpp", 124, 0, "%s", "backEndData->debugGlobals.polyCount > 0");
    vertCount = 0;
    for (polyIndex = 0; polyIndex < backEndData->debugGlobals.polyCount; ++polyIndex)
    {
        poly = &backEndData->debugGlobals.polys[polyIndex];
        polyVerts = backEndData->debugGlobals.verts[poly->firstVert];
        vertIndexPrev = poly->vertCount - 1;
        for (vertIndex = 0; vertIndex < poly->vertCount; ++vertIndex)
        {
            vertCount = RB_AddDebugLine(
                &polyVerts[3 * vertIndexPrev],
                &polyVerts[3 * vertIndex],
                poly->color,
                0,
                vertCount,
                2725,
                g_debugPolyVerts);
            vertIndexPrev = vertIndex;
        }
    }
    return RB_EndDebugLines(0, vertCount / 2, g_debugPolyVerts);
}

void __cdecl RB_DrawDebugPolys()
{
    if (backEndData->debugGlobals.polyCount)
    {
        if (tess.indexCount)
            RB_EndTessSurface();
        R_Set3D(&gfxCmdBufSourceState);
        RB_DrawPolyInteriors();
        RB_DrawPolyOutlines();
        backEndData->debugGlobals.polyCount = 0;
    }
}

void __cdecl RB_DrawDebugLines(trDebugLine_t *lines, int lineCount, GfxPointVertex *verts)
{
    char lineDepthTest; // [esp+3h] [ebp-11h]
    int vertCount; // [esp+4h] [ebp-10h]
    int lineIndex; // [esp+Ch] [ebp-8h]
    char depthTest; // [esp+13h] [ebp-1h]

    if (lineCount)
    {
        if (tess.indexCount)
            RB_EndTessSurface();
        R_Set3D(&gfxCmdBufSourceState);
        depthTest = lines->depthTest != 0;
        vertCount = 0;
        for (lineIndex = 0; lineIndex < lineCount; ++lineIndex)
        {
            lineDepthTest = lines[lineIndex].depthTest != 0;
            if (depthTest != lineDepthTest)
            {
                vertCount = RB_EndDebugLines(depthTest, vertCount / 2, verts);
                depthTest = lineDepthTest;
            }
            vertCount = RB_AddDebugLine(
                lines[lineIndex].start,
                lines[lineIndex].end,
                lines[lineIndex].color,
                depthTest,
                vertCount,
                2725,
                verts);
        }
        RB_EndDebugLines(depthTest, vertCount / 2, verts);
    }
}

void __cdecl RB_DrawDebugStrings(trDebugString_t *strings, int stringCount)
{
    float scale; // [esp+0h] [ebp-28h]
    float scalea; // [esp+0h] [ebp-28h]
    float xStep[3]; // [esp+8h] [ebp-20h] BYREF
    float yStep[3]; // [esp+14h] [ebp-14h] BYREF
    int stringIndex; // [esp+20h] [ebp-8h]
    GfxColor color; // [esp+24h] [ebp-4h] BYREF

    if (stringCount)
    {
        if (tess.indexCount)
            RB_EndTessSurface();
        R_Set3D(&gfxCmdBufSourceState);
        for (stringIndex = 0; stringIndex < stringCount; ++stringIndex)
        {
            R_ConvertColorToBytes(strings[stringIndex].color, (unsigned int*)&color);
            scale = -strings[stringIndex].scale;
            Vec3Scale(gfxCmdBufSourceState.viewParms.axis[1], scale, xStep);
            scalea = -strings[stringIndex].scale;
            Vec3Scale(gfxCmdBufSourceState.viewParms.axis[2], scalea, yStep);
            RB_DrawTextInSpace(
                strings[stringIndex].text,
                backEnd.debugFont,
                strings[stringIndex].xyz,
                xStep,
                yStep,
                color.packed);
        }
        if (tess.indexCount)
            RB_EndTessSurface();
    }
}

void __cdecl RB_DrawDebug(const GfxViewParms *viewParms)
{
    if (!viewParms)
        MyAssertHandler(".\\rb_debug.cpp", 250, 0, "%s", "viewParms");
    RB_AddPlumeStrings(viewParms);
    RB_DrawDebugPolys();
    RB_DrawDebugLines(backEndData->debugGlobals.lines, backEndData->debugGlobals.lineCount, g_debugLineVerts);
    RB_DrawDebugLines(
        backEndData->debugGlobals.externLines,
        backEndData->debugGlobals.externLineCount,
        g_debugExternLineVerts);
    backEndData->debugGlobals.lineCount = 0;
    RB_DrawDebugStrings(backEndData->debugGlobals.strings, backEndData->debugGlobals.stringCount);
    RB_DrawDebugStrings(backEndData->debugGlobals.externStrings, backEndData->debugGlobals.externStringCount);
    if (tess.indexCount)
        RB_EndTessSurface();
}