#include "cg_local.h"
#include "cg_public.h"
#include <cgame_mp/cg_local_mp.h>
#include <universal/physicalmemory.h>
#include <physics/phys_local.h>
#include <gfx_d3d/r_init.h>
#include <server_mp/server.h>

#include <qcommon/mem_track.h>
#include <client/client.h>
#include <aim_assist/aim_assist.h>
#include <ui/ui.h>
#include <gfx_d3d/r_utils.h>
#include <sound/snd_public.h>
#include <EffectsCore/fx_system.h>
#include <ragdoll/ragdoll.h>
#include <script/scr_variable.h>
#include <script/scr_vm.h>
#include <sound/snd_local.h>
#include <sound/snd_public.h>

int previous;
int fps_previousTimes[32];
int fps_index;

const struct MemInfoData//$26A77A1ABB1A9087FD9203E2FD79C24D // sizeof=0x8
{                                       // ...
    const char *name;                   // ...
    int budgetKB;                       // ...
};

const /*$26A77A1ABB1A9087FD9203E2FD79C24D*/ MemInfoData meminfoData[37] =
{
  { "debug", 0 },
  { "free hunk", 0 },
  { "binaries", 0 },
  { "misc swap", 0 },
  { "", 0},
  { "AI", 2048 },
  { "AI nodes", 5120 },
  { "script", 4096 },
  { "FX", 5120 },
  { "ent/net", 97280 },
  { "misc", 0 },
  { "anim", 10240 },
  { "world gbl", 0 },
  { "sound gbl", 0 },
  { "min pc sound gbl", 0 },
  { "sound", 0 },
  { "min pc sound", 10240 },
  { "", 0 },
  { "gfx gbl", 0 },
  { "gfx images", 0 },
  { "gfx world", 0 },
  { "gfx model", 0 },
  { "gfx misc", 0 },
  { "gfx total", 0 },
  { "", 0 },
  { "coll misc", 0 },
  { "coll brush", 0 },
  { "coll model tri", 0 },
  { "coll terrain", 0 },
  { "coll total", 0 },
  { "map ents", 0 },
  { "temp", 0 },
  { "", 0 },
  { "localize", 3072 },
  { "ui", 3072 },
  { "min pc tex", 49152 },
  { "", 0 }
}; // idb

trStatistics_t rendererStats;

void __cdecl CG_CalculateFPS()
{
    DWORD v0; // eax
    int frameTime; // [esp+0h] [ebp-8h]

    v0 = Sys_Milliseconds();
    frameTime = v0 - previous;
    previous = v0;
    fps_previousTimes[fps_index % 32] = frameTime;
    ++fps_index;
}

double __cdecl CG_DrawFPS(const ScreenPlacement *scrPlace, float y, meminfo_t *meminfo)
{
    float frac; // [esp+20h] [ebp-B8h]
    float v5; // [esp+24h] [ebp-B4h]
    float v6; // [esp+28h] [ebp-B0h]
    float v7; // [esp+34h] [ebp-A4h]
    float v8; // [esp+48h] [ebp-90h]
    float v9; // [esp+4Ch] [ebp-8Ch]
    float v10; // [esp+50h] [ebp-88h]
    float v11; // [esp+54h] [ebp-84h]
    float v13; // [esp+74h] [ebp-64h]
    float v14; // [esp+84h] [ebp-54h]
    float freeMem; // [esp+8Ch] [ebp-4Ch]
    int fps; // [esp+90h] [ebp-48h]
    float mspf; // [esp+94h] [ebp-44h]
    float farRight; // [esp+98h] [ebp-40h]
    int total; // [esp+A0h] [ebp-38h]
    float varColor[4]; // [esp+A8h] [ebp-30h] BYREF
    int maxTime; // [esp+B8h] [ebp-20h] BYREF
    int fpsMin; // [esp+BCh] [ebp-1Ch]
    int minTime; // [esp+C0h] [ebp-18h] BYREF
    float variance; // [esp+C4h] [ebp-14h]
    float average; // [esp+C8h] [ebp-10h] BYREF
    char *s; // [esp+CCh] [ebp-Ch]
    int i; // [esp+D0h] [ebp-8h]
    const float *color; // [esp+D4h] [ebp-4h]
    float yf; // [esp+E4h] [ebp+Ch]
    float ya; // [esp+E4h] [ebp+Ch]
    float yg; // [esp+E4h] [ebp+Ch]
    float yb; // [esp+E4h] [ebp+Ch]
    float yh; // [esp+E4h] [ebp+Ch]
    float yc; // [esp+E4h] [ebp+Ch]
    float yi; // [esp+E4h] [ebp+Ch]
    float yj; // [esp+E4h] [ebp+Ch]
    float yk; // [esp+E4h] [ebp+Ch]
    float yl; // [esp+E4h] [ebp+Ch]
    float yd; // [esp+E4h] [ebp+Ch]
    float ye; // [esp+E4h] [ebp+Ch]

    if (fps_index < 32)
        return y;
    farRight = cg_debugInfoCornerOffset->current.value + scrPlace->virtualViewableMax[0] - scrPlace->virtualViewableMin[0];
    minTime = 0x7FFFFFFF;
    maxTime = 0;
    total = 0;
    for (i = 0; i < 32; ++i)
    {
        total += fps_previousTimes[i];
        if (minTime > fps_previousTimes[i])
            minTime = fps_previousTimes[i];
        if (maxTime < fps_previousTimes[i])
            maxTime = fps_previousTimes[i];
    }
    average = (double)total / 32.0;
    variance = 0.0;
    for (i = 0; i < 32; ++i)
    {
        v14 = (double)fps_previousTimes[i] - average;
        v7 = fabs(v14);
        variance = variance + v7;
    }
    variance = variance / 32.0;
    if (!total)
        total = 1;
    if (minTime <= 0)
        minTime = 1;
    fpsMin = (int)(1000.0f / maxTime);
    fps = (int)(32000.0f / total);
    v10 = (double)R_TextWidth(" cg ms/frame", 0, cgMedia.smallDevFont) * 1.0;
    v8 = (double)(fps - 55) / 10.0;
    v6 = v8 - 1.0;
    if (v6 < 0.0)
        v9 = (double)(fps - 55) / 10.0;
    else
        v9 = 1.0;
    v5 = 0.0 - v8;
    if (v5 < 0.0)
        frac = v9;
    else
        frac = 0.0;
    Vec4Lerp(colorRed, colorWhite, frac, varColor);
    color = varColor;
    s = va("(%i-%i, %i) %i", fpsMin, (int)(1000.0f / minTime), (int)(variance), fps);
    yf = CG_CornerDebugPrint(scrPlace, farRight, y, v10, s, (char *)" FPS", color) + y;
    mspf = (double)total / 32.0;
    s = va("(%i-%i) %1.2f", minTime, maxTime, mspf);
    ya = CG_CornerDebugPrint(scrPlace, farRight, yf, v10, s, (char *)" cg ms/frame", colorWhite) + yf;
    if (sv.profile.frameTime > 0.0)
    {
        s = va("(%.0f-%.0f) %.2f", sv.serverFrameTimeMin, sv.serverFrameTimeMax, sv.profile.frameTime);
        yg = CG_CornerDebugPrint(scrPlace, farRight, ya, v10, s, (char *)" sv ms/frame", colorWhite) + ya;
        s = va("(%.2f) %.2f", 50.0 / sv.profile.wallClockTime, sv.profile.wallClockTime);
        ya = CG_CornerDebugPrint(scrPlace, farRight, yg, v10, s, (char *)" wall clock", colorWhite) + yg;
    }
    freeMem = (double)PMem_GetFreeAmount() / 1024.0 / 1024.0;
    if (freeMem >= 5.0)
    {
        if (freeMem >= 10.0)
            color = colorWhiteFaded;
        else
            color = colorRedFaded;
    }
    else if (CG_Flash(500))
    {
        color = colorRedFaded;
    }
    else
    {
        color = colorWhiteFaded;
    }
    s = va("%3.1f", freeMem);
    yb = CG_CornerDebugPrint(scrPlace, farRight, ya, v10, s, (char *)" free mem", colorWhite) + ya;
    if (cg_drawFPS->current.integer >= 2)
    {
        R_TrackStatistics(&rendererStats);
        yh = CG_CornerDebugPrintCaption(scrPlace, farRight, yb, v10, (char *)"-Scene-", colorGreenFaded) + yb;
        s = va("%i", rendererStats.c_viewIndexes / 3);
        yc = CG_CornerDebugPrint(scrPlace, farRight, yh, v10, s, (char *)" view tris", colorWhite) + yh;
        if (rendererStats.c_shadowIndexes)
        {
            s = va("%i", rendererStats.c_shadowIndexes / 3);
            yc = CG_CornerDebugPrint(scrPlace, farRight, yc, v10, s, (char *)" shadow tris", colorWhite) + yc;
        }
        s = va("%i", rendererStats.c_indexes / 3);
        yi = CG_CornerDebugPrint(scrPlace, farRight, yc, v10, s, (char *)" raw geo tris", colorWhite) + yc;
        s = va("%i", rendererStats.c_fxIndexes / 3);
        yj = CG_CornerDebugPrint(scrPlace, farRight, yi, v10, s, (char *)" raw fx tris", colorWhite) + yi;
        s = va("%i", rendererStats.c_batches);
        yk = CG_CornerDebugPrint(scrPlace, farRight, yj, v10, s, (char *)" prim", colorWhite) + yj;
        yl = CG_CornerDebugPrintCaption(scrPlace, farRight, yk, v10, (char *)"-Level-", colorGreenFaded) + yk;
        s = va("%d", rendererStats.c_imageUsage.total / 0x100000);
        yd = CG_CornerDebugPrint(scrPlace, farRight, yl, v10, s, (char *)" tex", colorWhite) + yl;
        if (rendererStats.c_imageUsage.minspec > 0x3000000)
        {
            color = colorRed;
            s = va("(!budget is %g!) %d", 48.0, rendererStats.c_imageUsage.minspec / 0x100000);
        }
        else
        {
            color = colorWhite;
            s = va("%d", rendererStats.c_imageUsage.minspec / 0x100000);
        }
        ye = CG_CornerDebugPrint(scrPlace, farRight, yd, v10, s, (char *)" min pc tex", color) + yd;
        if (meminfoData[35].budgetKB << 10 != 0x3000000)
            MyAssertHandler(
                ".\\cgame\\cg_draw_debug.cpp",
                415,
                0,
                "%s",
                "MAX_MINSPEC_TEXTURE_USAGE == meminfoData[TRACK_MINSPEC_IMAGES].budgetKB * 1024");
        meminfo->typeTotal[35] = 0;
        s = va("%i", meminfo->nonSwapMinSpecTotal / 0x100000);
        yb = CG_CornerDebugPrint(scrPlace, farRight, ye, v10, s, (char *)" min pc mem", colorWhite) + ye;
    }
    Phys_PerformanceEndFrame();
    Phys_GetPerformance(&average, &minTime, &maxTime);
    s = va("(%i-%i) %3.1f", minTime, maxTime, average);
    return (float)(CG_CornerDebugPrint(scrPlace, farRight, yb, v10, s, (char*)" phys ms/fr", colorWhite) + yb);    
}

bool __cdecl CG_Flash(int timeMs)
{
    return (int)Sys_Milliseconds() % (2 * timeMs) < timeMs;
}

double __cdecl CG_CornerDebugPrint(
    const ScreenPlacement *sP,
    float posX,
    float posY,
    float labelWidth,
    char *text,
    char *label,
    const float *color)
{
    int v9; // [esp+24h] [ebp-18h]
    float x; // [esp+2Ch] [ebp-10h]
    int textDelta; // [esp+30h] [ebp-Ch]
    int yDelta; // [esp+34h] [ebp-8h]
    int labelDelta; // [esp+38h] [ebp-4h]

    if (cg_drawFPSLabels->current.enabled)
    {
        x = posX - labelWidth;
        textDelta = CG_DrawDevString(sP, x, posY, 1.0f, 1.1f, text, color, 6, cgMedia.smallDevFont);
        labelDelta = CG_DrawDevString(sP, x, posY, 1.0f, 1.1f, label, colorWhiteFaded, 5, cgMedia.smallDevFont);
        if (textDelta < labelDelta)
            v9 = labelDelta;
        else
            v9 = textDelta;
        yDelta = v9;
    }
    else
    {
        yDelta = CG_DrawDevString(sP, posX, posY, 1.0f, 1.1f, text, color, 6, cgMedia.smallDevFont);
    }
    return (float)((float)yDelta * 0.75f);
}

double __cdecl CG_CornerDebugPrintCaption(
    const ScreenPlacement *sP,
    float posX,
    float posY,
    float labelWidth,
    char *text,
    const float *color)
{
    float x; // [esp+24h] [ebp-8h]
    int yDelta; // [esp+28h] [ebp-4h]

    if (cg_drawFPSLabels->current.enabled)
    {
        x = posX - labelWidth;
        yDelta = CG_DrawDevString(sP, x, posY, 1.0f, 1.1f, text, color, 7, cgMedia.smallDevFont);
    }
    else
    {
        yDelta = CG_DrawDevString(sP, posX, posY, 1.0f, 1.1f, text, color, 6, cgMedia.smallDevFont);
    }
    return (float)((float)yDelta * 0.75);
}

void __cdecl CG_DrawUpperRightDebugInfo(int localClientNum)
{
    meminfo_t meminfo; // [esp+8h] [ebp-A8h] BYREF
    float y; // [esp+ACh] [ebp-4h]

    track_getbasicinfo(&meminfo);
    R_TrackStatistics(0);
    y = cg_debugInfoCornerOffset->current.vector[1];
    if (cg_drawFPS->current.integer)
        y = CG_DrawFPS(&scrPlaceFull, y, &meminfo);
    if (com_statmon->current.enabled)
        y = CG_DrawStatmon(&scrPlaceFull, y, &meminfo);
    if (cg_drawSnapshot->current.enabled)
        CG_DrawSnapshot(localClientNum, y);
}

double __cdecl CG_DrawSnapshot(int localClientNum, float posY)
{
    char *v2; // eax
    char *v3; // eax
    float v5; // [esp+1Ch] [ebp-1Ch]
    char *str; // [esp+20h] [ebp-18h]
    const ScreenPlacement *scrPlace; // [esp+24h] [ebp-14h]
    float posX; // [esp+30h] [ebp-8h]
    float posYa; // [esp+44h] [ebp+Ch]
    float posYb; // [esp+44h] [ebp+Ch]
    float posYc; // [esp+44h] [ebp+Ch]

    if (localClientNum)
    {
        MyAssertHandler(
            "c:\\trees\\cod3\\src\\cgame\\../cgame_mp/cg_local_mp.h",
            1071,
            0,
            "%s\n\t(localClientNum) = %i",
            "(localClientNum == 0)",
            localClientNum);
        MyAssertHandler(
            "c:\\trees\\cod3\\src\\cgame\\../cgame_mp/cg_local_mp.h",
            1083,
            0,
            "%s\n\t(localClientNum) = %i",
            "(localClientNum == 0)",
            localClientNum);
    }
    scrPlace = &scrPlaceView[localClientNum];
    posX = cg_debugInfoCornerOffset->current.value + scrPlace->virtualViewableMax[0] - scrPlace->virtualViewableMin[0];
    v5 = (double)R_TextWidth(" server time", 0, cgMedia.smallDevFont) * 1.0;
    posYa = CG_CornerDebugPrintCaption(scrPlace, posX, posY, v5, (char*)"-Snapshot-", colorGreenFaded) + posY;
    v2 = va("%i", cgArray[0].nextSnap->serverTime);
    posYb = CG_CornerDebugPrint(scrPlace, posX, posYa, v5, v2, (char *)" server time", colorWhite) + posYa;
    v3 = va("%i", cgArray[0].latestSnapshotNum);
    posYc = CG_CornerDebugPrint(scrPlace, posX, posYb, v5, v3, (char *)" snap num", colorWhite) + posYb;
    str = va("%i", cgsArray[0].serverCommandSequence);
    return (float)(CG_CornerDebugPrint(scrPlace, posX, posYc, v5, str, (char *)" cmd", colorWhite) + posYc);
}

double __cdecl CG_DrawStatmon(const ScreenPlacement *scrPlace, float y, meminfo_t *meminfo)
{
    char *v3; // eax
    float farRight; // [esp+18h] [ebp-Ch]
    char *s; // [esp+1Ch] [ebp-8h]
    int i; // [esp+20h] [ebp-4h]

    farRight = cg_debugInfoCornerOffset->current.value + scrPlace->virtualViewableMax[0] - scrPlace->virtualViewableMin[0];
    for (i = 0; i < 37; ++i)
    {
        if (meminfoData[i].budgetKB)
        {
            if (meminfo->typeTotal[i] > meminfoData[i].budgetKB << 10)
            {
                v3 = va("%s (%g)      ", meminfoData[i].name, (double)meminfoData[i].budgetKB / 1024.0);
                CG_CornerDebugPrint(scrPlace, farRight, y, 0.0, v3, (char *)"", colorRedFaded);
                s = va("%i", meminfo->typeTotal[i] / 0x100000);
                y = CG_CornerDebugPrint(scrPlace, farRight, y, 0.0, s, (char *)"", colorRedFaded) + y;
            }
        }
    }
    return y;
}

void __cdecl CG_DrawPerformanceWarnings()
{
    signed int time; // [esp+20h] [ebp-18h]
    const statmonitor_s *stats; // [esp+24h] [ebp-14h] BYREF
    int i; // [esp+28h] [ebp-10h]
    float x; // [esp+2Ch] [ebp-Ch]
    float y; // [esp+30h] [ebp-8h]
    int statCount; // [esp+34h] [ebp-4h] BYREF

    time = Sys_Milliseconds();
    StatMon_GetStatsArray(&stats, &statCount);
    x = 2.0;
    y = 200.0;
    for (i = 0; i < statCount; ++i)
    {
        if (stats[i].endtime >= time)
            UI_DrawHandlePic(&scrPlaceFull, x, y, 32.0, 32.0, 1, 1, 0, stats[i].material);
        x = x + 34.0;
        if (x + 32.0 > 68.0)
        {
            x = 2.0;
            y = y + 34.0;
        }
    }
}

void __cdecl CG_DrawDebugOverlays(int localClientNum)
{
    if (cg_drawMaterial->current.integer)
    {
        CG_DrawMaterial(localClientNum, cg_drawMaterial->current.unsignedInt);
    }
    else
    {
        if (player_debugHealth->current.enabled)
            CG_DrawDebugPlayerHealth(localClientNum);
        AimAssist_DrawDebugOverlay(localClientNum);
    }
}

void __cdecl CG_DrawMaterial(int localClientNum, unsigned int drawMaterialType)
{
    int v2; // [esp+18h] [ebp-206Ch]
    int v3; // [esp+1Ch] [ebp-2068h]
    char surfaceFlags[4096]; // [esp+24h] [ebp-2060h] BYREF
    int traceMask[4]; // [esp+1024h] [ebp-1060h]
    char name[64]; // [esp+1034h] [ebp-1050h] BYREF
    float x; // [esp+1074h] [ebp-1010h]
    float y; // [esp+1078h] [ebp-100Ch]
    char contents[4100]; // [esp+107Ch] [ebp-1008h] BYREF

    traceMask[0] = 0;
    traceMask[1] = 1;
    traceMask[2] = 0x2806831;
    traceMask[3] = 0x2810011;
    if (!drawMaterialType)
        MyAssertHandler(".\\cgame\\cg_draw_debug.cpp", 847, 0, "%s", "drawMaterialType != 0");
    if (drawMaterialType >= 4)
        MyAssertHandler(
            ".\\cgame\\cg_draw_debug.cpp",
            848,
            0,
            "drawMaterialType doesn't index ARRAY_COUNT( traceMasks )\n\t%i not in [0, %i)",
            drawMaterialType,
            4);
    if (localClientNum)
        MyAssertHandler(
            "c:\\trees\\cod3\\src\\cgame\\../cgame_mp/cg_local_mp.h",
            1071,
            0,
            "%s\n\t(localClientNum) = %i",
            "(localClientNum == 0)",
            localClientNum);
    if (R_PickMaterial(
        traceMask[drawMaterialType],
        cgArray[0].refdef.vieworg,
        cgArray[0].refdef.viewaxis[0],
        name,
        surfaceFlags,
        contents,
        0x1000u))
    {
        x = 8.0;
        y = 240.0;
        v3 = CG_DrawSmallDevStringColor(&scrPlaceView[localClientNum], 8.0, 240.0, name, colorWhite, 5);
        y = (double)v3 + y;
        v2 = CG_DrawSmallDevStringColor(&scrPlaceView[localClientNum], x, y, surfaceFlags, colorWhite, 5);
        y = (double)v2 + y;
        CG_DrawSmallDevStringColor(&scrPlaceView[localClientNum], x, y, contents, colorWhite, 5);
    }
}

void __cdecl CG_DrawDebugPlayerHealth(int localClientNum)
{
    float w; // [esp+30h] [ebp-2Ch]
    float v2; // [esp+34h] [ebp-28h]
    float v3; // [esp+38h] [ebp-24h]
    float v4; // [esp+3Ch] [ebp-20h]
    float v5; // [esp+40h] [ebp-1Ch]
    float health; // [esp+48h] [ebp-14h]
    float healtha; // [esp+48h] [ebp-14h]
    float color[4]; // [esp+4Ch] [ebp-10h] BYREF

    if (!player_debugHealth->current.enabled)
        MyAssertHandler(".\\cgame\\cg_draw_debug.cpp", 902, 0, "%s", "player_debugHealth->current.enabled");
    if (localClientNum)
        MyAssertHandler(
            "c:\\trees\\cod3\\src\\cgame\\../cgame_mp/cg_local_mp.h",
            1071,
            0,
            "%s\n\t(localClientNum) = %i",
            "(localClientNum == 0)",
            localClientNum);
    if (cgArray[0].predictedPlayerState.stats[0] && cgArray[0].predictedPlayerState.stats[2])
    {
        health = (double)cgArray[0].predictedPlayerState.stats[0] / (double)cgArray[0].predictedPlayerState.stats[2];
        v4 = health - 1.0;
        if (v4 < 0.0)
            v5 = (double)cgArray[0].predictedPlayerState.stats[0] / (double)cgArray[0].predictedPlayerState.stats[2];
        else
            v5 = 1.0;
        v3 = 0.0 - health;
        if (v3 < 0.0)
            v2 = v5;
        else
            v2 = 0.0;
        healtha = v2;
    }
    else
    {
        healtha = 0.0;
    }
    color[0] = 0.0;
    color[1] = 0.0;
    color[2] = 0.0;
    color[3] = 1.0;
    CL_DrawStretchPic(
        &scrPlaceView[localClientNum],
        10.0,
        10.0,
        100.0,
        10.0,
        1,
        1,
        0.0,
        0.0,
        1.0,
        1.0,
        color,
        cgMedia.whiteMaterial);
    color[0] = 0.0;
    color[1] = 1.0;
    color[2] = 0.0;
    color[3] = 1.0;
    w = healtha * 100.0;
    CL_DrawStretchPic(
        &scrPlaceView[localClientNum],
        10.0,
        10.0,
        w,
        10.0,
        1,
        1,
        0.0,
        0.0,
        healtha,
        1.0,
        color,
        cgMedia.whiteMaterial);
}

void __cdecl CG_DrawFullScreenDebugOverlays(int localClientNum)
{
    if (cg_drawVersion->current.enabled)
        CG_DrawVersion();
    if (snd_drawEqChannels->current.enabled)
        CG_DrawSoundEqOverlay(localClientNum);
    CG_DrawPerformanceWarnings();
    if (fx_mark_profile->current.integer <= 0)
    {
        if (fx_profile->current.integer <= 0)
        {
            if (snd_drawInfo->current.integer)
            {
                CG_DrawSoundOverlay(&scrPlaceFull);
            }
            else if (cg_drawScriptUsage->current.enabled)
            {
                CG_DrawScriptUsage(&scrPlaceFull);
            }
            else if (cg_drawMaterial->current.integer)
            {
                CG_DrawMaterial(localClientNum, cg_drawMaterial->current.unsignedInt);
            }
            else
            {
                if (phys_drawDebugInfo->current.enabled)
                    Phys_DrawDebugText(&scrPlaceFull);
                Ragdoll_DebugDraw();
            }
        }
        else
        {
            CG_DrawFxProfile(fx_profile->current.integer - 1);
        }
    }
    else
    {
        CG_DrawFxMarkProfile(fx_mark_profile->current.integer - 1);
    }
}

void __cdecl CG_DrawScriptUsage(const ScreenPlacement *scrPlace)
{
    unsigned int NumScriptVars; // eax
    char *v2; // eax
    unsigned int NumScriptThreads; // eax
    char *v4; // eax
    int StringUsage; // eax
    char *v6; // eax

    NumScriptVars = Scr_GetNumScriptVars();
    v2 = va("num vars:    %d", NumScriptVars);
    CG_DrawStringExt(scrPlace, 400.0, 80.0, v2, colorWhite, 1, 1, 16.0);
    NumScriptThreads = Scr_GetNumScriptThreads();
    v4 = va("num threads: %d", NumScriptThreads);
    CG_DrawStringExt(scrPlace, 400.0, 96.0, v4, colorWhite, 1, 1, 16.0);
    StringUsage = Scr_GetStringUsage();
    v6 = va("string usage: %d", StringUsage);
    CG_DrawStringExt(scrPlace, 400.0, 112.0, v6, colorWhite, 1, 1, 16.0);
}

void CG_DrawVersion()
{
    float v0; // [esp+1Ch] [ebp-48h]
    float v1; // [esp+20h] [ebp-44h]
    float x; // [esp+24h] [ebp-40h]
    float y; // [esp+28h] [ebp-3Ch]
    Font_s *font; // [esp+34h] [ebp-30h]
    float h; // [esp+5Ch] [ebp-8h]
    float w; // [esp+60h] [ebp-4h]

    float fontScale = 0.25f;
    float color[4] = { 0.40000001f, 0.69999999f, 1.0f, 0.69999999f };
    float shadowColor[4] = { 0.0f, 0.0f, 0.0f, 0.69f};

    font = UI_GetFontHandle(&scrPlaceFullUnsafe, 0, 0.5f);
    w = (float)UI_TextWidth(version->current.string, 0, font, 0.25f);
    h = (float)UI_TextHeight(font, 0.25);
    y = -h - cg_drawVersionY->current.value + 1.0f;
    x = -w - cg_drawVersionX->current.value + 1.0f;
    UI_DrawText(&scrPlaceFullUnsafe, (char *)version->current.integer, 0x7FFFFFFF, font, x, y, 3, 3, 0.25f, shadowColor, 0);
    v1 = -h - cg_drawVersionY->current.value;
    v0 = -w - cg_drawVersionX->current.value;
    UI_DrawText(
        &scrPlaceFullUnsafe,
        (char *)version->current.integer,
        0x7FFFFFFF,
        font,
        v0,
        v1,
        3,
        3,
        fontScale,
        color,
        0);
}

void __cdecl CG_DrawSoundEqOverlay(int localClientNum)
{
    snd_entchannel_info_t *EntChannelName; // eax
    float v2; // [esp+20h] [ebp-738h]
    bool v3; // [esp+24h] [ebp-734h]
    snd_eqoverlay_info_t info[64]; // [esp+28h] [ebp-730h] BYREF
    const ScreenPlacement *scrPlace; // [esp+72Ch] [ebp-2Ch]
    bool nextLine; // [esp+733h] [ebp-25h]
    int band; // [esp+734h] [ebp-24h]
    int entchannel; // [esp+738h] [ebp-20h]
    int index; // [esp+73Ch] [ebp-1Ch]
    SndEqParams *params; // [esp+740h] [ebp-18h]
    char *line; // [esp+744h] [ebp-14h]
    float charHeight; // [esp+748h] [ebp-10h]
    float x; // [esp+74Ch] [ebp-Ch]
    float y; // [esp+750h] [ebp-8h]
    int count; // [esp+754h] [ebp-4h]

    scrPlace = &scrPlaceView[localClientNum];
    count = RETURN_ZERO32();
    if (count > 0)
    {
        charHeight = 10.0f;
        x = 0.0f;
        CG_DrawStringExt(scrPlace, 0.0, 82.0f, (char*)"Current EQ Settings", colorWhite, 0, 1, 10.0f);
        y = 82.0f + 10.0f;
        for (entchannel = 0; entchannel < count; ++entchannel)
        {
            EntChannelName = SND_GetEntChannelName(entchannel);
            line = va("%s", EntChannelName->name);
            x = 0.0f;
            CG_DrawStringExt(scrPlace, 0.0f, y, line, colorWhite, 0, 1, charHeight);
            y = y + charHeight;
            for (band = 0; band < 3; ++band)
            {
                x = 24.0f;
                nextLine = 0;
                for (index = 0; index < 2; ++index)
                {
                    params = info[entchannel].params[index][band];
                    line = va(
                        "eqIndex: %1i band: %1i filter: %-8s %6.2f Hz %2.2f dB q: %2.2f",
                        index,
                        band,
                        snd_eqTypeStrings[params->type],
                        params->freq,
                        params->gain,
                        params->q);
                    v3 = nextLine || params->enabled;
                    nextLine = v3;
                    if (params->enabled)
                    {
                        v2 = x + 24.0f;
                        CG_DrawStringExt(scrPlace, v2, y, line, colorWhite, 0, 1, charHeight);
                    }
                    x = x + 344.0f;
                }
                if (nextLine)
                {
                    line = va("%.2f%%", info[entchannel].lerp * 100.0f);
                    CG_DrawStringExt(scrPlace, 0.0, y, line, colorYellow, 0, 1, charHeight);
                    y = y + charHeight;
                }
            }
        }
    }
}

void __cdecl CG_DrawSoundOverlay(const ScreenPlacement *scrPlace)
{
    snd_overlay_info_t info[64]; // [esp+20h] [ebp-4430h] BYREF
    float v2; // [esp+4428h] [ebp-28h]
    int Int; // [esp+442Ch] [ebp-24h]
    snd_overlay_type_t type; // [esp+4430h] [ebp-20h]
    int i; // [esp+4434h] [ebp-1Ch]
    float charHeight; // [esp+4438h] [ebp-18h]
    float x; // [esp+443Ch] [ebp-14h]
    float y; // [esp+4440h] [ebp-10h]
    char *string; // [esp+4444h] [ebp-Ch]
    int SoundOverlay; // [esp+4448h] [ebp-8h]
    int cpu; // [esp+444Ch] [ebp-4h] BYREF

    type = (snd_overlay_type_t)snd_drawInfo->current.integer;
    SoundOverlay = SND_GetSoundOverlay(type, info, 64, &cpu);
    if (SoundOverlay > 0)
    {
        x = 2.0f;
        y = 82.0f;
        v2 = 8.0f;
        charHeight = 10.0f;
        Int = Dvar_GetInt("snd_khz");
        string = va("Listing Active Sounds of Type: %s", *(const char **)(snd_drawInfo->domain.integer.max + 4 * type));
        CG_DrawStringExt(scrPlace, x, y, string, colorWhite, 0, 1, charHeight);
        y = y + charHeight;
        string = va("CPU: ^3%%%i ^7kHz: ^3%i ", cpu, Int);
        CG_DrawStringExt(scrPlace, x, y, string, colorWhite, 0, 1, charHeight);
        y = y + charHeight;
        for (i = 0; i < SoundOverlay; ++i)
        {
            if (info[i].pszSampleName[0])
                string = va(
                    "%2i (%s) %-20s -> %-50s vol^3%04.2f ^7rvol^3%04.2f ^7dist^3%5i ^7pit^3%04.2f",
                    i,
                    info[i].entchannel,
                    info[i].aliasName,
                    info[i].pszSampleName,
                    info[i].fBaseVolume,
                    info[i].fCurVolume,
                    info[i].dist,
                    info[i].fPitch);
            else
                string = va("%2i", i);
            CG_DrawStringExt(scrPlace, x, y, string, colorWhite, 0, 1, charHeight);
            y = y + charHeight;
        }
    }
}

void __cdecl CG_DrawFxProfile(int localClientNum)
{
    float profilePos[2]; // [esp+0h] [ebp-8h] BYREF

    profilePos[0] = 0.0f;
    profilePos[1] = 12.0f;
    FX_DrawProfile(localClientNum, (void(__cdecl *)(char *))CG_DrawFxText, profilePos);
}

void __cdecl CG_DrawFxText(char *text, float *profilePos)
{
    if (!text)
        MyAssertHandler(".\\cgame\\cg_draw_debug.cpp", 866, 0, "%s", "text");
    CL_DrawText(
        &scrPlaceFull,
        text,
        0x7FFFFFFF,
        cgMedia.smallDevFont,
        *profilePos,
        profilePos[1],
        1,
        1,
        1.0f,
        1.0f,
        colorWhiteFaded,
        128);
    profilePos[1] = profilePos[1] + 12.0f;
}

void __cdecl CG_DrawFxMarkProfile(int localClientNum)
{
    float profilePos[2]; // [esp+0h] [ebp-8h] BYREF

    profilePos[0] = 0.0f;
    profilePos[1] = 12.0f;
    FX_DrawMarkProfile(localClientNum, (void(*)(const char*, float*))CG_DrawFxText, profilePos);
}

void __cdecl Vec4Set(float *v, float x, float y, float z, float w)
{
    *v = x;
    v[1] = y;
    v[2] = z;
    v[3] = w;
}