#include "r_debug.h"
#include "r_scene.h"

#include <qcommon/mem_track.h>

#include <win32/win_local.h>
#include <cgame/cg_public.h>
#include <universal/profile.h>

DebugGlobals debugGlobals;

void __cdecl TRACK_r_debug()
{
    track_static_alloc_internal(&debugGlobals, 84, "debugGlobals", 0);
}

void __cdecl R_AddDebugPolygon(DebugGlobals *debugGlobalsEntry, const float *color, int pointCount, float (*points)[3])
{
    GfxDebugPoly *v4; // [esp+30h] [ebp-4h]

    Sys_EnterCriticalSection(CRITSECT_DEBUG_LINE);
    if (pointCount + debugGlobalsEntry->vertCount > debugGlobalsEntry->vertLimit
        || debugGlobalsEntry->polyCount + 1 > debugGlobalsEntry->polyLimit)
    {
        Sys_LeaveCriticalSection(CRITSECT_DEBUG_LINE);
    }
    else
    {
        if (!debugGlobalsEntry->polys)
        {
            if (debugGlobalsEntry->verts)
                MyAssertHandler(".\\r_debug.cpp", 48, 0, "%s", "debugGlobalsEntry->verts == NULL");
            R_DebugAlloc((void **)&debugGlobalsEntry->polys, 24 * debugGlobalsEntry->polyLimit, "(debugGlobalsEntry->polys)");
            R_DebugAlloc((void **)&debugGlobalsEntry->verts, 12 * debugGlobalsEntry->vertLimit, "(debugGlobalsEntry->verts)");
        }
        if (debugGlobalsEntry->polys)
        {
            if (debugGlobalsEntry->verts)
            {
                debugGlobalsEntry->polys[debugGlobalsEntry->polyCount].firstVert = debugGlobalsEntry->vertCount;
                debugGlobalsEntry->polys[debugGlobalsEntry->polyCount].vertCount = pointCount;
                v4 = &debugGlobalsEntry->polys[debugGlobalsEntry->polyCount];
                v4->color[0] = *color;
                v4->color[1] = color[1];
                v4->color[2] = color[2];
                v4->color[3] = color[3];
                ++debugGlobalsEntry->polyCount;

                PROF_SCOPED("R_Memcpy");
                {
                    memcpy(
                        (unsigned __int8 *)debugGlobalsEntry->verts[debugGlobalsEntry->vertCount],
                        (unsigned __int8 *)points,
                        12 * pointCount);
                }

                debugGlobalsEntry->vertCount += pointCount;
                Sys_LeaveCriticalSection(CRITSECT_DEBUG_LINE);
            }
        }
    }
}

void __cdecl R_AddDebugLine(DebugGlobals *debugGlobalsEntry, const float *start, const float *end, const float *color)
{
    trDebugLine_t *pDebugLine; // [esp+8h] [ebp-4h]

    Sys_EnterCriticalSection(CRITSECT_DEBUG_LINE);
    if (debugGlobalsEntry->lineCount + 1 <= debugGlobalsEntry->lineLimit)
    {
        if (debugGlobalsEntry->lines
            || (R_DebugAlloc((void **)&debugGlobalsEntry->lines, 44 * debugGlobalsEntry->lineLimit, "R_AddDebugLine"),
                debugGlobalsEntry->lines))
        {
            pDebugLine = &debugGlobalsEntry->lines[debugGlobalsEntry->lineCount];
            pDebugLine->start[0] = *start;
            pDebugLine->start[1] = start[1];
            pDebugLine->start[2] = start[2];
            pDebugLine->end[0] = *end;
            pDebugLine->end[1] = end[1];
            pDebugLine->end[2] = end[2];
            pDebugLine->color[0] = *color;
            pDebugLine->color[1] = color[1];
            pDebugLine->color[2] = color[2];
            pDebugLine->color[3] = color[3];
            pDebugLine->depthTest = 0;
            ++debugGlobalsEntry->lineCount;
            Sys_LeaveCriticalSection(CRITSECT_DEBUG_LINE);
        }
    }
    else
    {
        Sys_LeaveCriticalSection(CRITSECT_DEBUG_LINE);
    }
}

void __cdecl R_AddDebugBox(DebugGlobals *debugGlobalsEntry, const float *mins, const float *maxs, const float *color)
{
    float v4; // [esp+0h] [ebp-6Ch]
    unsigned int j; // [esp+4h] [ebp-68h]
    unsigned int i; // [esp+8h] [ebp-64h]
    unsigned int ia; // [esp+8h] [ebp-64h]
    float v[8][3]; // [esp+Ch] [ebp-60h] BYREF

    for (i = 0; i < 8; ++i)
    {
        for (j = 0; j < 3; ++j)
        {
            if ((i & (1 << j)) != 0)
                v4 = maxs[j];
            else
                v4 = mins[j];
            v[i][j] = v4;
        }
    }
    for (ia = 0; ia < 0xC; ++ia)
        R_AddDebugLine(debugGlobalsEntry, v[iEdgePairs[ia][0]], v[iEdgePairs[ia][1]], color);
}

void __cdecl R_AddDebugString(
    DebugGlobals *debugGlobalsEntry,
    const float *origin,
    const float *color,
    float scale,
    const char *string)
{
    trDebugString_t *pDebugString; // [esp+4h] [ebp-4h]

    Sys_EnterCriticalSection(CRITSECT_DEBUG_LINE);
    if (debugGlobalsEntry->stringCount + 1 <= debugGlobalsEntry->stringLimit)
    {
        if (debugGlobalsEntry->strings
            || (R_DebugAlloc((void **)&debugGlobalsEntry->strings, debugGlobalsEntry->stringLimit << 7, "R_AddDebugString"),
                debugGlobalsEntry->strings))
        {
            pDebugString = &debugGlobalsEntry->strings[debugGlobalsEntry->stringCount];
            pDebugString->xyz[0] = *origin;
            pDebugString->xyz[1] = origin[1];
            pDebugString->xyz[2] = origin[2];
            pDebugString->color[0] = *color;
            pDebugString->color[1] = color[1];
            pDebugString->color[2] = color[2];
            pDebugString->color[3] = color[3];
            pDebugString->scale = scale;
            strncpy(pDebugString->text, string, sizeof(pDebugString->text) - 1);
            pDebugString->text[95] = 0;
            ++debugGlobalsEntry->stringCount;
            Sys_LeaveCriticalSection(CRITSECT_DEBUG_LINE);
        }
    }
    else
    {
        Sys_LeaveCriticalSection(CRITSECT_DEBUG_LINE);
    }
}

void __cdecl R_AddScaledDebugString(
    DebugGlobals *debugGlobalsEntry,
    const GfxViewParms *viewParms,
    const float *origin,
    const float *color,
    const char *string)
{
    float delta[3]; // [esp+14h] [ebp-14h] BYREF
    float scale; // [esp+20h] [ebp-8h]
    float dot; // [esp+24h] [ebp-4h]

    Vec3Sub(origin, viewParms->origin, delta);
    scale = Vec3Normalize(delta);
    dot = Vec3Dot(delta, viewParms->axis[0]);
    scale = (dot - 0.9950000047683716) * scale;
    if (scale < 1.0)
        scale = 1.0;
    R_AddDebugString(debugGlobalsEntry, origin, color, scale, string);
}

void __cdecl R_InitDebugEntry(DebugGlobals *debugGlobalsEntry)
{
    memset((unsigned __int8 *)debugGlobalsEntry, 0, sizeof(DebugGlobals));
    debugGlobalsEntry->vertLimit = 4096;
    debugGlobalsEntry->polyLimit = 512;
    debugGlobalsEntry->stringLimit = 4096;
    debugGlobalsEntry->lineLimit = 0x4000;
    debugGlobalsEntry->plumeLimit = 4096;
}

void __cdecl R_InitDebug()
{
    R_InitDebugEntry(&debugGlobals);
}

void __cdecl R_ShutdownDebugEntry(DebugGlobals *debugGlobalsEntry)
{
    R_DebugFree((void **)&debugGlobalsEntry->polys);
    R_DebugFree((void **)&debugGlobalsEntry->verts);
    R_DebugFree((void **)&debugGlobalsEntry->strings);
    R_DebugFree((void **)&debugGlobalsEntry->externStrings);
    R_DebugFree((void **)&debugGlobalsEntry->lines);
    R_DebugFree((void **)&debugGlobalsEntry->externLines);
    R_DebugFree((void **)&debugGlobalsEntry->plumes);
}

void __cdecl R_TransferDebugGlobals(DebugGlobals *debugGlobalsEntry)
{
    int dt; // [esp+8h] [ebp-8h]
    int plumeIndex; // [esp+Ch] [ebp-4h]

    plumeIndex = 0;
    while (plumeIndex < debugGlobals.plumeCount)
    {
        dt = scene.def.time - debugGlobals.plumes[plumeIndex].startTime;
        if (dt >= 0 && dt <= debugGlobals.plumes[plumeIndex].duration)
            ++plumeIndex;
        else
            memcpy(
                &debugGlobals.plumes[plumeIndex],
                &debugGlobals.plumes[--debugGlobals.plumeCount],
                sizeof(debugGlobals.plumes[plumeIndex]));
    }
    if (!debugGlobals.plumes)
        goto LABEL_11;
    if (debugGlobalsEntry->plumes
        || (R_DebugAlloc((void **)&debugGlobalsEntry->plumes, 40 * debugGlobals.plumeLimit, "(debugGlobalsEntry->plumes)"),
            debugGlobalsEntry->plumes))
    {
        memcpy(
            (unsigned __int8 *)debugGlobalsEntry->plumes,
            (unsigned __int8 *)debugGlobals.plumes,
            40 * debugGlobals.plumeCount);
    LABEL_11:
        debugGlobalsEntry->plumeCount = debugGlobals.plumeCount;
    }
}

void __cdecl R_ShutdownDebug()
{
    R_ShutdownDebugEntry(&debugGlobals);
}

void __cdecl R_CopyDebugStrings(
    trDebugString_t *clStrings,
    int clStringCnt,
    trDebugString_t *svStrings,
    int svStringCnt,
    int maxStringCount)
{
    DebugGlobals *dg; // [esp+0h] [ebp-Ch]
    unsigned __int8 *dest; // [esp+8h] [ebp-4h]

    dg = &frontEndDataOut->debugGlobals;
    if (!clStrings && !svStrings
        || frontEndDataOut->debugGlobals.externStrings
        || (R_DebugAlloc((void **)&frontEndDataOut->debugGlobals.externStrings, maxStringCount << 7, "R_CopyDebugStrings"),
            dg->externStrings))
    {
        dest = (unsigned __int8 *)dg->externStrings;
        if (clStrings)
        {
            memcpy(dest, (unsigned __int8 *)clStrings, clStringCnt << 7);
            dest += 128 * clStringCnt;
        }
        if (svStrings)
            memcpy(dest, (unsigned __int8 *)svStrings, svStringCnt << 7);
        dg->externStringCount = svStringCnt + clStringCnt;
    }
}

void __cdecl R_CopyDebugLines(
    trDebugLine_t *clLines,
    int clLineCnt,
    trDebugLine_t *svLines,
    int svLineCnt,
    int maxLineCount)
{
    DebugGlobals *dg; // [esp+0h] [ebp-Ch]
    unsigned __int8 *dest; // [esp+8h] [ebp-4h]

    dg = &frontEndDataOut->debugGlobals;
    if (!clLines && !svLines
        || frontEndDataOut->debugGlobals.externLines
        || (R_DebugAlloc((void **)&frontEndDataOut->debugGlobals.externLines, 44 * maxLineCount, "R_CopyDebugLines"),
            dg->externLines))
    {
        dest = (unsigned __int8 *)dg->externLines;
        if (clLines)
        {
            memcpy(dest, (unsigned __int8 *)clLines, 44 * clLineCnt);
            dest += 44 * clLineCnt;
        }
        if (svLines)
            memcpy(dest, (unsigned __int8 *)svLines, 44 * svLineCnt);
        dg->externLineCount = svLineCnt + clLineCnt;
    }
}

