#include "rb_stats.h"
#include <qcommon/mem_track.h>
#include <qcommon/cmd.h>


//struct GfxViewStats *g_viewStats 85b938d0     gfx_d3d : rb_stats.obj
//struct GfxPrimStats *backupPrimStats 85b938d4     gfx_d3d : rb_stats.obj
//int marker_rb_stats      85b938d8     gfx_d3d : rb_stats.obj
//struct GfxFrameStats g_frameStatsCur 85b958e0     gfx_d3d : rb_stats.obj
//struct GfxPrimStats *g_primStats 85b95b54     gfx_d3d : rb_stats.obj

int histogramHistory[64][2][16];

GfxPrimStats *backupPrimStats;
GfxViewStats *g_viewStats;
GfxPrimStats *g_primStats;
GfxFrameStats g_frameStatsCur;

const int drawPrimHistogramLimit[15] =
{
    0xA, 0x19, 0x32, 0x64, 0xC8, 0x12C, 0x190, 0x258, 0x320, 0x3E8, 0x4B0, 0x578, 0x640, 0x708, 0x7D0
};
const char *primStatsLabel[10] =
{
    "world",
    "smodel cached",
    "smodel rigid",
    "xmodel rigid",
    "xmodel skinned",
    "bmodel",
    "fx",
    "hud & 2d",
    "debug",
    "code"
};
const unsigned int s_stencilFuncTable_52[8] =
{
    1, 2, 3, 4, 5, 6, 7, 8
};
const unsigned int s_cullTable_52[4] =
{
    0, 1, 3, 2
};
const unsigned int s_blendTable_52[11] =
{
    0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10
};
const unsigned int s_blendOpTable_52[6] =
{
    0, 1, 2, 3, 4, 5
};
const unsigned int s_depthTestTable_52[4] =
{
    8, 2, 3, 4
};
const unsigned int s_stencilOpTable_52[8] =
{
    1, 2, 3, 4, 5, 6, 7, 8
};



void __cdecl TRACK_rb_stats()
{
    track_static_alloc_internal(histogramHistory, 0x2000, "histogramHistory", 10);
}

void __cdecl R_TrackPrims(GfxCmdBufState *state, GfxPrimStatsTarget target)
{
    if (backupPrimStats)
        MyAssertHandler(".\\rb_stats.cpp", 74, 0, "%s", "backupPrimStats == NULL");
    if (!g_viewStats)
        MyAssertHandler(".\\rb_stats.cpp", 75, 0, "%s", "g_viewStats");
    if (g_primStats && g_primStats != &g_viewStats->primStats[target])
        MyAssertHandler(
            ".\\rb_stats.cpp",
            76,
            0,
            "%s\n\t(target) = %i",
            "(g_primStats == 0 || g_primStats == &g_viewStats->primStats[target])",
            target);
    g_primStats = &g_viewStats->primStats[target];
}

void __cdecl RB_TrackImmediatePrims(GfxPrimStatsTarget target)
{
    if (backupPrimStats)
        MyAssertHandler(".\\rb_stats.cpp", 83, 0, "%s", "backupPrimStats == NULL");
    if (!g_viewStats)
        MyAssertHandler(".\\rb_stats.cpp", 84, 0, "%s", "g_viewStats");
    backupPrimStats = g_primStats;
    g_primStats = &g_viewStats->primStats[target];
}

void __cdecl RB_EndTrackImmediatePrims()
{
    g_primStats = backupPrimStats;
    backupPrimStats = 0;
}

void __cdecl RB_TrackDrawPrimCall(int triCount)
{
    int histogramIndex; // [esp+0h] [ebp-4h]

    if (!g_primStats)
        MyAssertHandler(".\\rb_stats.cpp", 101, 0, "%s", "g_primStats");
    g_primStats->triCount += triCount;
    ++g_primStats->primCount;
    for (histogramIndex = 0; histogramIndex < 15 && triCount > drawPrimHistogramLimit[histogramIndex]; ++histogramIndex)
        ;
    ++g_viewStats->drawPrimHistogram[histogramIndex];
}

int __cdecl RB_Stats_TotalIndexCount()
{
    int totalIndexCount; // [esp+0h] [ebp-14h]
    int primIndex; // [esp+Ch] [ebp-8h]
    int viewIndex; // [esp+10h] [ebp-4h]

    totalIndexCount = 0;
    for (viewIndex = 0; viewIndex < 2; ++viewIndex)
    {
        for (primIndex = 0; primIndex < 10; ++primIndex)
            totalIndexCount += g_frameStatsCur.viewStats[viewIndex].primStats[primIndex].dynamicIndexCount
            + g_frameStatsCur.viewStats[viewIndex].primStats[primIndex].staticIndexCount;
    }
    return totalIndexCount;
}

int __cdecl RB_Stats_TotalVertexCount()
{
    int totalVertexCount; // [esp+0h] [ebp-14h]
    int primIndex; // [esp+Ch] [ebp-8h]
    int viewIndex; // [esp+10h] [ebp-4h]

    totalVertexCount = 0;
    for (viewIndex = 0; viewIndex < 2; ++viewIndex)
    {
        for (primIndex = 0; primIndex < 10; ++primIndex)
            totalVertexCount += g_frameStatsCur.viewStats[viewIndex].primStats[primIndex].dynamicVertexCount
            + g_frameStatsCur.viewStats[viewIndex].primStats[primIndex].staticVertexCount;
    }
    return totalVertexCount;
}

int __cdecl RB_Stats_TotalPrimCount()
{
    int totalPrimCount; // [esp+4h] [ebp-10h]
    int primIndex; // [esp+Ch] [ebp-8h]
    int viewIndex; // [esp+10h] [ebp-4h]

    totalPrimCount = 0;
    for (viewIndex = 0; viewIndex < 2; ++viewIndex)
    {
        for (primIndex = 0; primIndex < 10; ++primIndex)
            totalPrimCount += g_frameStatsCur.viewStats[viewIndex].primStats[primIndex].primCount;
    }
    return totalPrimCount;
}

int __cdecl RB_Stats_ViewIndexCount(const GfxViewStats *viewStats)
{
    int totalIndexCount; // [esp+0h] [ebp-Ch]
    int primIndex; // [esp+8h] [ebp-4h]

    totalIndexCount = 0;
    for (primIndex = 0; primIndex < 10; ++primIndex)
        totalIndexCount += viewStats->primStats[primIndex].dynamicIndexCount
        + viewStats->primStats[primIndex].staticIndexCount;
    return totalIndexCount;
}

void __cdecl RB_Stats_f()
{
    const char *v0; // eax
    const char *v1; // eax
    const char *v2; // eax

    if (Cmd_Argc() > 2)
        goto LABEL_2;
    R_SyncRenderThread();
    if (Cmd_Argc() == 2)
    {
        v1 = Cmd_Argv(1);
        if (!I_stricmp(v1, "max"))
        {
            RB_Stats_Display(&backEnd.frameStatsMax);
            return;
        }
        v2 = Cmd_Argv(1);
        if (I_stricmp(v2, "cur"))
        {
        LABEL_2:
            v0 = Cmd_Argv(0);
            Com_Printf(8, "USAGE: %s [cur|max]\n", v0);
            return;
        }
    }
    RB_Stats_Display(&g_frameStatsCur);
}

void __cdecl RB_Stats_Display(const GfxFrameStats *frameStats)
{
    int v1; // eax
    int v2; // eax

    RB_Stats_Summarize("Visible", frameStats->viewStats);
    RB_Stats_Summarize("Shadow", &frameStats->viewStats[1]);
    Com_Printf(8, "SUMMARY:\n");
    Com_Printf(
        8,
        "%i raw tris (%i geo, %i fx)\n",
        frameStats->fxIndexCount / 3 + frameStats->geoIndexCount / 3,
        frameStats->geoIndexCount / 3,
        frameStats->fxIndexCount / 3);
    v1 = RB_Stats_TotalIndexCount();
    Com_Printf(8, "%i total tris\n", v1 / 3);
    v2 = RB_Stats_TotalPrimCount();
    Com_Printf(8, "%i total prims\n", v2);
}

void __cdecl RB_Stats_Summarize(const char *label, const GfxViewStats *viewStats)
{
    int barBlocks; // [esp+8h] [ebp-30h]
    GfxPrimStats total; // [esp+Ch] [ebp-2Ch] BYREF
    int barIndex; // [esp+24h] [ebp-14h]
    int maxPrimsInHistogram; // [esp+28h] [ebp-10h]
    const GfxPrimStats *primStats; // [esp+2Ch] [ebp-Ch]
    int primIndex; // [esp+30h] [ebp-8h]
    int histogramIndex; // [esp+34h] [ebp-4h]

    Com_Printf(8, "\n=== %s Geometry (%i drawsurfs) ===\n", label, viewStats->drawSurfCount);
    Com_Printf(8, "+--------------+-----+-------+-------+-------+-------+-------+\n");
    Com_Printf(8, "|geometry type |prims|   tris|s indxs|s verts|d indxs|d verts|\n");
    Com_Printf(8, "+--------------+-----+-------+-------+-------+-------+-------+\n");
    memset(&total, 0, sizeof(total));
    for (primIndex = 0; primIndex < 10; ++primIndex)
    {
        primStats = &viewStats->primStats[primIndex];
        RB_Stats_SummarizePrimStats(primStatsLabel[primIndex], primStats);
        RB_Stats_AccumulatePrimStats(primStats, &total);
    }
    Com_Printf(8, "+--------------+-----+-------+-------+-------+-------+-------+\n");
    RB_Stats_SummarizePrimStats("total", &total);
    Com_Printf(8, "+--------------+-----+-------+-------+-------+-------+-------+\n");
    maxPrimsInHistogram = 0;
    for (histogramIndex = 0; histogramIndex < 16; ++histogramIndex)
    {
        if (maxPrimsInHistogram < viewStats->drawPrimHistogram[histogramIndex])
            maxPrimsInHistogram = viewStats->drawPrimHistogram[histogramIndex];
    }
    if (maxPrimsInHistogram)
    {
        Com_Printf(8, "\nTriangles Per Primitive Histogram:\n");
        for (histogramIndex = 0; histogramIndex < 16; ++histogramIndex)
        {
            if (histogramIndex >= 15)
                Com_Printf(8, " > %4i: ", s_stencilFuncTable_52[histogramIndex + 7]);
            else
                Com_Printf(8, "<= %4i: ", drawPrimHistogramLimit[histogramIndex]);
            Com_Printf(
                8,
                "%3i (%4.1f%%) ",
                viewStats->drawPrimHistogram[histogramIndex],
                (double)viewStats->drawPrimHistogram[histogramIndex] * 100.0 / (double)total.primCount);
            barBlocks = (40 * viewStats->drawPrimHistogram[histogramIndex] + maxPrimsInHistogram / 2) / maxPrimsInHistogram;
            for (barIndex = 0; barIndex < barBlocks; ++barIndex)
                Com_Printf(8, "#");
            Com_Printf(8, "\n");
        }
    }
}

void __cdecl RB_Stats_AccumulatePrimStats(const GfxPrimStats *primStats, GfxPrimStats *total)
{
    total->primCount += primStats->primCount;
    total->triCount += primStats->triCount;
    total->staticIndexCount += primStats->staticIndexCount;
    total->staticVertexCount += primStats->staticVertexCount;
    total->dynamicIndexCount += primStats->dynamicIndexCount;
    total->dynamicVertexCount += primStats->dynamicVertexCount;
}

void __cdecl RB_Stats_SummarizePrimStats(const char *label, const GfxPrimStats *primStats)
{
    char *v2; // [esp+0h] [ebp-3Ch]
    char *v3; // [esp+4h] [ebp-38h]
    char *v4; // [esp+8h] [ebp-34h]
    char *v5; // [esp+Ch] [ebp-30h]
    char *v6; // [esp+10h] [ebp-2Ch]
    char *v7; // [esp+14h] [ebp-28h]
    char text[32]; // [esp+18h] [ebp-24h] BYREF

    Com_Printf(8, "|%-14s", label);
    if (primStats->primCount)
    {
        v7 = itoa(primStats->primCount, text, 0xAu);
        Com_Printf(8, "|%5s", v7);
    }
    else
    {
        Com_Printf(8, "|%5s", "");
    }
    if (primStats->triCount)
    {
        v6 = itoa(primStats->triCount, text, 0xAu);
        Com_Printf(8, "|%7s", v6);
    }
    else
    {
        Com_Printf(8, "|%7s", "");
    }
    if (primStats->staticIndexCount)
    {
        v5 = itoa(primStats->staticIndexCount, text, 0xAu);
        Com_Printf(8, "|%7s", v5);
    }
    else
    {
        Com_Printf(8, "|%7s", "");
    }
    if (primStats->staticVertexCount)
    {
        v4 = itoa(primStats->staticVertexCount, text, 0xAu);
        Com_Printf(8, "|%7s", v4);
    }
    else
    {
        Com_Printf(8, "|%7s", "");
    }
    if (primStats->dynamicIndexCount)
    {
        v3 = itoa(primStats->dynamicIndexCount, text, 0xAu);
        Com_Printf(8, "|%7s", v3);
    }
    else
    {
        Com_Printf(8, "|%7s", "");
    }
    if (primStats->dynamicVertexCount)
    {
        v2 = itoa(primStats->dynamicVertexCount, text, 0xAu);
        Com_Printf(8, "|%7s|\n", v2);
    }
    else
    {
        Com_Printf(8, "|%7s|\n", "");
    }
}

void __cdecl RB_Stats_UpdateMaxs(const GfxFrameStats *frameStatsCur, GfxFrameStats *frameStatsMax)
{
    int index; // [esp+8h] [ebp-Ch]

    for (index = 0; index < 157; ++index)
    {
        if (*(&frameStatsMax->viewStats[0].primStats[0].primCount + index) < *(&frameStatsCur->viewStats[0].primStats[0].primCount
            + index))
            *(&frameStatsMax->viewStats[0].primStats[0].primCount + index) = *(&frameStatsCur->viewStats[0].primStats[0].primCount
                + index);
    }
}

int histogramHistoryIndex;
void __cdecl RB_DrawPrimHistogramOverlay()
{
    const char *v0; // eax
    Font_s *s1; // [esp+18h] [ebp-104h]
    float t1; // [esp+1Ch] [ebp-100h]
    GfxColor v3; // [esp+24h] [ebp-F8h]
    float v4; // [esp+28h] [ebp-F4h]
    float v5; // [esp+2Ch] [ebp-F0h]
    float v6; // [esp+30h] [ebp-ECh]
    float v7; // [esp+34h] [ebp-E8h]
    float v8; // [esp+38h] [ebp-E4h]
    float v9; // [esp+3Ch] [ebp-E0h]
    float v10; // [esp+40h] [ebp-DCh]
    float v11; // [esp+44h] [ebp-D8h]
    float v12; // [esp+48h] [ebp-D4h]
    float v13; // [esp+4Ch] [ebp-D0h]
    float v14; // [esp+50h] [ebp-CCh]
    float v15; // [esp+54h] [ebp-C8h]
    float v16; // [esp+58h] [ebp-C4h]
    float h; // [esp+5Ch] [ebp-C0h]
    float v18; // [esp+60h] [ebp-BCh]
    float v19; // [esp+64h] [ebp-B8h]
    const char *v20; // [esp+68h] [ebp-B4h]
    int rowMax; // [esp+A0h] [ebp-7Ch]
    float x1; // [esp+A4h] [ebp-78h]
    int subTotalPrims[2][3]; // [esp+A8h] [ebp-74h] BYREF
    unsigned int subTotalIndex; // [esp+C0h] [ebp-5Ch]
    const char *caption; // [esp+C4h] [ebp-58h]
    GfxColor colorNow[3]; // [esp+C8h] [ebp-54h]
    int historyIndex; // [esp+D4h] [ebp-48h]
    int totalPrims[2]; // [esp+D8h] [ebp-44h]
    float wMax; // [esp+E0h] [ebp-3Ch]
    float countWidth; // [esp+E4h] [ebp-38h]
    int maxPrimsInHistogram; // [esp+E8h] [ebp-34h]
    int curCount; // [esp+ECh] [ebp-30h]
    int viewStatsIndex; // [esp+F0h] [ebp-2Ch]
    GfxColor colorPeak[3]; // [esp+F4h] [ebp-28h]
    float fontHeight; // [esp+104h] [ebp-18h]
    GfxColor black; // [esp+108h] [ebp-14h]
    int histogramIndex; // [esp+10Ch] [ebp-10h]
    float x; // [esp+110h] [ebp-Ch]
    float y; // [esp+114h] [ebp-8h]
    float w; // [esp+118h] [ebp-4h]

    maxPrimsInHistogram = 0;
    for (histogramIndex = 0; histogramIndex < 16; ++histogramIndex)
    {
        for (viewStatsIndex = 0; viewStatsIndex < 2; ++viewStatsIndex)
        {
            if (maxPrimsInHistogram < backEnd.frameStatsMax.viewStats[viewStatsIndex].drawPrimHistogram[histogramIndex])
                maxPrimsInHistogram = backEnd.frameStatsMax.viewStats[viewStatsIndex].drawPrimHistogram[histogramIndex];
        }
    }
    if (maxPrimsInHistogram)
    {
        fontHeight = (float)R_TextHeight(backEnd.debugFont);
        countWidth = (float)R_TextWidth("8888 ", 5, backEnd.debugFont);
        x1 = (double)R_TextWidth("8888:", 5, backEnd.debugFont) + (float)48.0 + 4.0;
        y = 48.0;
        wMax = (360.0 - (float)48.0) / 2.0 - countWidth - 4.0;
        black.packed = -16777216;
        colorNow[0].packed = -49088;
        colorPeak[0].packed = -6291456;
        colorNow[1].packed = -192;
        colorPeak[1].packed = -6250496;
        colorNow[2].packed = -12517568;
        colorPeak[2].packed = -16736256;
        totalPrims[0] = 0;
        totalPrims[1] = 0;
        memset(subTotalPrims, 0, sizeof(subTotalPrims));
        for (histogramIndex = 0; histogramIndex < 16; ++histogramIndex)
        {
            if (histogramIndex >= 15)
                v20 = "more:";
            else
                v20 = va("%4i:", drawPrimHistogramLimit[histogramIndex]);
            caption = v20;
            v19 = y + fontHeight - 1.0;
            RB_DrawText(v20, backEnd.debugFont, 48.0, v19, (GfxColor)-1);
            x = x1;
            if (histogramIndex == 15 || drawPrimHistogramLimit[histogramIndex] > 300)
                subTotalIndex = 2;
            else
                subTotalIndex = drawPrimHistogramLimit[histogramIndex] > 50;
            for (viewStatsIndex = 0; viewStatsIndex < 2; ++viewStatsIndex)
            {
                curCount = g_frameStatsCur.viewStats[viewStatsIndex].drawPrimHistogram[histogramIndex];
                subTotalPrims[viewStatsIndex][subTotalIndex] += curCount;
                totalPrims[viewStatsIndex] += curCount;
                if (curCount)
                {
                    v3.packed = (unsigned int)colorNow[subTotalIndex];
                    v18 = y + fontHeight - 1.0;
                    t1 = x;
                    s1 = backEnd.debugFont;
                    v0 = va("%4i", curCount);
                    RB_DrawText(v0, s1, t1, v18, v3);
                }
                x = x + countWidth;
                rowMax = curCount;
                for (historyIndex = 0; historyIndex < 64; ++historyIndex)
                {
                    if (rowMax < histogramHistory[historyIndex][viewStatsIndex][histogramIndex])
                        rowMax = histogramHistory[historyIndex][viewStatsIndex][histogramIndex];
                }
                if (rowMax)
                {
                    w = (double)rowMax * wMax / (double)maxPrimsInHistogram;
                    h = fontHeight - 4.0;
                    v16 = w + 4.0;
                    v15 = y + 2.0;
                    v14 = x - 2.0;
                    RB_DrawStretchPic(rgp.whiteMaterial, v14, v15, v16, h, 0.0, 0.0, 1.0, 1.0, black.packed, GFX_PRIM_STATS_DEBUG);
                    v13 = fontHeight - 8.0;
                    v12 = y + 4.0;
                    RB_DrawStretchPic(
                        rgp.whiteMaterial,
                        x,
                        v12,
                        w,
                        v13,
                        0.0,
                        0.0,
                        1.0,
                        1.0,
                        colorPeak[subTotalIndex].packed,
                        GFX_PRIM_STATS_DEBUG);
                    if (curCount)
                    {
                        w = (double)curCount * wMax / (double)maxPrimsInHistogram;
                        v11 = fontHeight - 8.0;
                        v10 = y + 4.0;
                        RB_DrawStretchPic(
                            rgp.whiteMaterial,
                            x,
                            v10,
                            w,
                            v11,
                            0.0,
                            0.0,
                            1.0,
                            1.0,
                            colorNow[subTotalIndex].packed,
                            GFX_PRIM_STATS_DEBUG);
                    }
                }
                histogramHistory[histogramHistoryIndex & 0x3F][viewStatsIndex][histogramIndex] = curCount;
                x = wMax + 4.0 + x;
            }
            y = y + fontHeight;
        }
        x = x1;
        for (viewStatsIndex = 0; viewStatsIndex < 2; ++viewStatsIndex)
        {
            x = x + countWidth;
            if (totalPrims[viewStatsIndex])
            {
                v9 = fontHeight - 4.0;
                v8 = wMax + 4.0;
                v7 = y + 2.0;
                v6 = x - 2.0;
                RB_DrawStretchPic(rgp.whiteMaterial, v6, v7, v8, v9, 0.0, 0.0, 1.0, 1.0, black.packed, GFX_PRIM_STATS_DEBUG);
                for (subTotalIndex = 0; subTotalIndex < 3; ++subTotalIndex)
                {
                    w = (double)subTotalPrims[viewStatsIndex][subTotalIndex] * wMax / (double)totalPrims[viewStatsIndex];
                    v5 = fontHeight - 8.0;
                    v4 = y + 4.0;
                    RB_DrawStretchPic(
                        rgp.whiteMaterial,
                        x,
                        v4,
                        w,
                        v5,
                        0.0,
                        0.0,
                        1.0,
                        1.0,
                        colorNow[subTotalIndex].packed,
                        GFX_PRIM_STATS_DEBUG);
                    x = x + w;
                }
                x = x + 4.0;
            }
            else
            {
                x = wMax + 4.0 + x;
            }
        }
        ++histogramHistoryIndex;
    }
}

