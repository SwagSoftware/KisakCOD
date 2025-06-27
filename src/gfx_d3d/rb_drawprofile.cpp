#include "rb_drawprofile.h"
#include <qcommon/mem_track.h>
#include "r_dvars.h"
#include <qcommon/qcommon.h>
#include "rb_shade.h"
#include <qcommon/threads.h>
#include <script/scr_parser.h>

DrawProfileGlobals drawProfGlob;
float g_tally;


void __cdecl TRACK_rb_drawprofile()
{
    track_static_alloc_internal(&drawProfGlob, 33192, "drawProfGlob", 0);
}

void __cdecl RB_AddProfileThread(int threadContext)
{
    ProfileStack *prof_stack; // [esp+10h] [ebp-14h]
    ProfileReadableGlobal *global; // [esp+14h] [ebp-10h]
    int profEnum; // [esp+18h] [ebp-Ch]
    int atomType; // [esp+1Ch] [ebp-8h]
    profile_t *prof; // [esp+20h] [ebp-4h]

    prof_stack = Profile_GetStackForContext(threadContext);
    for (profEnum = 0; profEnum < 432; ++profEnum)
    {
        prof = &prof_stack->prof_array[profEnum];
        global = &drawProfGlob.global[profEnum];
        global->hits += prof_stack->prof_array[profEnum].read.hits;
        for (atomType = 0; atomType < 1; ++atomType)
        {
            global->read.self.value[atomType] += prof->read.self.value[atomType];
            global->read.total.value[atomType] += prof->read.total.value[atomType];
        }
        global->selfClks = (double)prof_stack->prof_array[profEnum].read.self.value[0] + global->selfClks;
        global->totalClks = (double)prof_stack->prof_array[profEnum].read.total.value[0] + global->totalClks;
        global->read.hits += prof_stack->prof_array[profEnum].read.hits;
    }
}

char __cdecl RB_IsUsingAnyProfile()
{
    unsigned int probeIter; // [esp+0h] [ebp-4h]

    if (profile->current.integer)
        return 1;
    for (probeIter = 0; probeIter < 5; ++probeIter)
    {
        if (prof_probe[probeIter]->current.integer)
            return 1;
    }
    return 0;
}

void __cdecl RB_DrawSlowProfileOverlay(int(__cdecl *compare)(const void *, const void *))
{
    int row; // [esp+8h] [ebp-10h]
    int probeIndex; // [esp+Ch] [ebp-Ch]
    DWORD currTime; // [esp+10h] [ebp-8h]
    float y; // [esp+14h] [ebp-4h]

    currTime = Sys_Milliseconds();
    if (prof_sortTime->current.value * 1000.0 <= (double)(int)(currTime - drawProfGlob.lastSortTime)
        || (int)(currTime - drawProfGlob.lastSortTime) < 0)
    {
        drawProfGlob.lastSortTime = currTime;
        for (probeIndex = 0; probeIndex < 432; ++probeIndex)
            drawProfGlob.sortedProbeIndices[probeIndex] = probeIndex;
        qsort(drawProfGlob.sortedProbeIndices, 432, sizeof(drawProfGlob.sortedProbeIndices[0]), compare);
    }
    RB_DrawAllProfileBackgrounds(0, profile_rowcount->current.integer);
    RB_DrawProfileHistory(0);
    y = RB_DrawProfileLabels();
    g_tally = 0.0;
    for (row = 0; row < profile_rowcount->current.integer; ++row)
        y = RB_DrawProfileRow(drawProfGlob.sortedProbeIndices[row], 0, y);
}

void __cdecl RB_DrawProfile()
{
    ProfileReadable *p_read; // edx
    ProfileReadableGlobal *global; // [esp+8h] [ebp-14h]
    int profEnum; // [esp+Ch] [ebp-10h]
    int profEnuma; // [esp+Ch] [ebp-10h]
    int profEnumb; // [esp+Ch] [ebp-10h]
    int atomType; // [esp+10h] [ebp-Ch]
    bool modified; // [esp+17h] [ebp-5h]
    int threadContext; // [esp+18h] [ebp-4h]
    int threadContexta; // [esp+18h] [ebp-4h]

    if (RB_IsUsingAnyProfile())
    {
        modified = profile->modified;
        if (profile_thread->modified)
            modified = 1;
        if (modified)
        {
            memset((unsigned __int8 *)drawProfGlob.global, 0, sizeof(drawProfGlob.global));
            for (profEnum = 0; profEnum < 432; ++profEnum)
                drawProfGlob.global[profEnum].min.value[0] = -1;
            Dvar_ClearModified((dvar_s*)profile);
            Dvar_ClearModified((dvar_s*)profile_thread);
        }
        else
        {
            for (profEnuma = 0; profEnuma < 432; ++profEnuma)
            {
                p_read = &drawProfGlob.global[profEnuma].read;
                p_read->hits = 0;
                p_read->total.value[0] = 0;
                p_read->self.value[0] = 0;
            }
        }
        threadContext = Profile_GetDisplayThread();
        if (threadContext >= gfxCfg.threadContextCount)
        {
            for (threadContexta = 0; threadContexta < gfxCfg.threadContextCount; ++threadContexta)
                RB_AddProfileThread(threadContexta);
        }
        else
        {
            RB_AddProfileThread(threadContext);
        }
        for (profEnumb = 0; profEnumb < 432; ++profEnumb)
        {
            global = &drawProfGlob.global[profEnumb];
            ++global->sequence;
            for (atomType = 0; atomType < 1; ++atomType)
            {
                if (global->max.value[atomType] < global->read.total.value[atomType])
                    global->max.value[atomType] = global->read.total.value[atomType];
                if (global->min.value[atomType] > global->read.total.value[atomType])
                    global->min.value[atomType] = global->read.total.value[atomType];
                if (global->read.hits > global->maxHits)
                    global->maxHits = global->read.hits;
                if (global->maxSelf.value[atomType] < global->read.self.value[atomType])
                    global->maxSelf.value[atomType] = global->read.self.value[atomType];
            }
        }
        if (tess.indexCount)
            RB_EndTessSurface();
        switch (profile->current.integer)
        {
        case 0:
            RB_DrawProfileHistory(0);
            break;
        case 1:
            RB_DrawSlowProfileOverlay((int(__cdecl *)(const void *, const void *))CompareSelfTimes);
            break;
        case 2:
            RB_DrawSlowProfileOverlay((int(__cdecl *)(const void *, const void *))CompareTotalTimes);
            break;
        case 3:
            RB_DrawSlowProfileOverlay((int(__cdecl *)(const void *, const void *))CompareAvgSelfTimes);
            break;
        case 4:
            RB_DrawSlowProfileOverlay((int(__cdecl *)(const void *, const void *))CompareAvgTotalTimes);
            break;
        case 5:
            RB_DrawSlowProfileOverlay((int(__cdecl *)(const void *, const void *))CompareMaxTimes);
            break;
        case 6:
            RB_DrawSlowProfileOverlay((int(__cdecl *)(const void *, const void *))CompareMaxSelfTimes);
            break;
        default:
            RB_DrawProfileOverlay();
            break;
        }
        if (tess.indexCount)
            RB_EndTessSurface();
    }
}

void __cdecl RB_DrawProfileHistory(const ProfileSettings *profSettings)
{
    int ProfileHistoryProbeIndex; // eax
    int historyProfEnum[5]; // [esp+18h] [ebp-28h]
    int profEnum; // [esp+2Ch] [ebp-14h]
    int parity; // [esp+30h] [ebp-10h]
    int probeIndex; // [esp+34h] [ebp-Ch]
    float x; // [esp+38h] [ebp-8h]
    float y; // [esp+3Ch] [ebp-4h]

    for (probeIndex = 0; probeIndex < 5; ++probeIndex)
    {
        ProfileHistoryProbeIndex = RB_GetProfileHistoryProbeIndex(probeIndex, profSettings);
        historyProfEnum[probeIndex] = ProfileHistoryProbeIndex;
    }
    x = drawProfGlob.fontWidth * 0.5;
    y = (float)(vidConfig.displayHeight - 98);
    for (probeIndex = 0; probeIndex < 5; ++probeIndex)
    {
        profEnum = historyProfEnum[probeIndex];
        if (profEnum)
        {
            parity = Profile_GetEnumParity(profEnum);
            RB_DrawProfileHistoryGraph(&drawProfGlob.global[profEnum].read, parity, probeIndex, x, y);
            x = x + 124.0;
        }
    }
    x = drawProfGlob.fontWidth * 0.5;
    y = (float)(vidConfig.displayHeight - 98);
    for (probeIndex = 0; probeIndex < 5; ++probeIndex)
    {
        if (historyProfEnum[probeIndex])
        {
            RB_DrawProfileHistoryLabel(historyProfEnum[probeIndex], x, y);
            x = x + 124.0;
        }
    }
}

void __cdecl RB_DrawProfileHistoryGraph(const ProfileReadable *read, int parity, int probeIndex, float x, float y)
{
    float v5; // [esp+14h] [ebp-44h]
    float v6; // [esp+1Ch] [ebp-3Ch]
    float v7; // [esp+24h] [ebp-34h]
    float height; // [esp+28h] [ebp-30h]
    int j; // [esp+40h] [ebp-18h]
    int ja; // [esp+40h] [ebp-18h]
    float pixelsPerClock; // [esp+44h] [ebp-14h]
    int j0; // [esp+48h] [ebp-10h]
    unsigned int historyIndex; // [esp+4Ch] [ebp-Ch]
    int maxMsec; // [esp+50h] [ebp-8h]

    if (!read)
        MyAssertHandler(".\\rb_drawprofile.cpp", 175, 0, "%s", "read");
    if (drawProfGlob.log[probeIndex].parity != parity)
    {
        historyIndex = drawProfGlob.sortedProbeIndices[362 * probeIndex - 1809] % 0x78u;
        ++drawProfGlob.sortedProbeIndices[362 * probeIndex - 1809];
        *(ProfileReadable *)&drawProfGlob.sortedProbeIndices[362 * probeIndex - 1808 + 3 * historyIndex] = *read;
        drawProfGlob.log[probeIndex].parity = parity;
    }
    height = drawProfGlob.fontHeight + 64.0;
    v7 = y - drawProfGlob.fontHeight;
    RB_DrawProfileRect(x, v7, 120.0, height, (GfxColor)1426063360);
    maxMsec = prof_probeMaxMsec->current.integer;
    if (maxMsec < 1)
        maxMsec = 1;
    pixelsPerClock = *((float *)Sys_GetValue(0) + 20782) * 64.0 / (double)maxMsec;
    j0 = drawProfGlob.sortedProbeIndices[362 * probeIndex - 1809] - 120;
    if (j0 < 0)
        j0 = 0;
    for (j = j0; j < drawProfGlob.sortedProbeIndices[362 * probeIndex - 1809]; ++j)
    {
        v6 = x + 119.0 - (double)(drawProfGlob.sortedProbeIndices[362 * probeIndex - 1809] - j);
        RB_DrawProfileBar(
            v6,
            y,
            pixelsPerClock,
            drawProfGlob.sortedProbeIndices[362 * probeIndex - 1807 + 3 * (j % 120)],
            (GfxColor)-256);
    }
    for (ja = j0; ja < drawProfGlob.sortedProbeIndices[362 * probeIndex - 1809]; ++ja)
    {
        v5 = x + 119.0 - (double)(drawProfGlob.sortedProbeIndices[362 * probeIndex - 1809] - ja);
        RB_DrawProfileBar(
            v5,
            y,
            pixelsPerClock,
            drawProfGlob.sortedProbeIndices[362 * probeIndex - 1806 + 3 * (ja % 120)],
            (GfxColor)-65536);
    }
}

void __cdecl RB_DrawProfileRect(float x, float y, float width, float height, GfxColor color)
{
    RB_DrawStretchPic(rgp.whiteMaterial, x, y, width, height, 0.0, 0.0, 1.0, 1.0, color.packed, GFX_PRIM_STATS_DEBUG);
}

void __cdecl RB_DrawProfileBar(float x, float y, float pixelsPerClock, unsigned int clockCount, GfxColor color)
{
    float v5; // [esp+14h] [ebp-10h]
    float height; // [esp+20h] [ebp-4h]

    height = (double)clockCount * pixelsPerClock;
    if (height >= 1.0)
    {
        if (height > 64.0)
            height = 64.0;
    }
    else
    {
        height = 1.0;
    }
    v5 = y + 64.0 - height;
    RB_DrawProfileRect(x, v5, 1.0, height, color);
}

void __cdecl RB_DrawProfileHistoryLabel(int profEnum, float x, float y)
{
    float v3; // [esp+10h] [ebp-94h]
    unsigned int charLimit; // [esp+20h] [ebp-84h]
    char label[120]; // [esp+24h] [ebp-80h] BYREF
    const char *name; // [esp+A0h] [ebp-4h]

    name = prof_enumNames[profEnum];
    charLimit = (int)(120.0f / drawProfGlob.fontWidth) + 1;
    iassert(charLimit <= ARRAY_COUNT(label));
    I_strncpyz(label, (char *)name, charLimit);
    RB_DrawText(label, drawProfGlob.font, x, y, drawProfGlob.textColor);
}

int __cdecl RB_GetProfileHistoryProbeIndex(int historyIndex, const ProfileSettings *profSettings)
{
    if (prof_probe[historyIndex]->current.integer)
        return prof_probe[historyIndex]->current.integer;
    if (profSettings)
        return profSettings->defaultProbeIndex[historyIndex];
    return 0;
}

int RB_DrawProfileOverlay()
{
    int result; // eax
    const ProfileSettings *profSettings; // [esp+4h] [ebp-10h]
    int i; // [esp+Ch] [ebp-8h]
    float y; // [esp+10h] [ebp-4h]

    profSettings = &s_profileArrays[profile->current.integer - 7];
    RB_DrawAllProfileBackgrounds(profSettings, profSettings->infoCount);
    RB_DrawProfileHistory(profSettings);
    y = RB_DrawProfileLabels();
    g_tally = 0.0;
    for (i = 0; i < profSettings->infoCount; ++i)
    {
        y = RB_DrawProfileRow(profSettings->profDrawInfo[i].probeIndex, profSettings->profDrawInfo[i].indentation, y);
        result = i + 1;
    }
    return result;
}

void __cdecl RB_DrawAllProfileBackgrounds(const ProfileSettings *profSettings, int rowCount)
{
    float y; // [esp+4h] [ebp-8h]
    int rowIndex; // [esp+8h] [ebp-4h]

    if (profSettings && profSettings->infoCount != rowCount)
        MyAssertHandler(
            ".\\rb_drawprofile.cpp",
            285,
            0,
            "%s",
            "profSettings == NULL || profSettings->infoCount == rowCount");
    y = drawProfGlob.fontHeight * 3.0;
    RB_DrawProfileBackground(y);
    for (rowIndex = 0; rowIndex < rowCount; ++rowIndex)
    {
        y = y + drawProfGlob.fontHeight;
        if (!profSettings || profSettings->profDrawInfo[rowIndex].probeIndex)
            RB_DrawProfileBackground(y);
    }
}

void __cdecl RB_DrawProfileBackground(float y)
{
    float v1; // [esp+28h] [ebp-18h]
    float h; // [esp+2Ch] [ebp-14h]
    float width; // [esp+34h] [ebp-Ch]
    float x; // [esp+38h] [ebp-8h]

    x = drawProfGlob.fontWidth * 0.5;
    width = drawProfGlob.fontWidth * 101.0;
    h = drawProfGlob.fontHeight - 2.0;
    v1 = y + 1.0;
    RB_DrawStretchPic(rgp.whiteMaterial, x, v1, width, h, 0.0, 0.0, 1.0, 1.0, 0x55000000u, GFX_PRIM_STATS_DEBUG);
}

double __cdecl RB_DrawProfileLabels()
{
    float y; // [esp+10h] [ebp-4h]

    y = drawProfGlob.fontHeight * 3.0;
    RB_DrawProfileString(1, y, "Probe Name", drawProfGlob.labelColor);
    RB_DrawProfileString(28, y, "  Self", drawProfGlob.labelColor);
    RB_DrawProfileString(34, y, " Total", drawProfGlob.labelColor);
    RB_DrawProfileString(40, y, " AvSlf", drawProfGlob.labelColor);
    RB_DrawProfileString(46, y, " MaxSlf", drawProfGlob.labelColor);
    RB_DrawProfileString(53, y, "  Tally", drawProfGlob.labelColor);
    RB_DrawProfileString(60, y, " AvTot", drawProfGlob.labelColor);
    RB_DrawProfileString(66, y, "   Min", drawProfGlob.labelColor);
    RB_DrawProfileString(72, y, "    Max", drawProfGlob.labelColor);
    RB_DrawProfileString(79, y, "  Hit", drawProfGlob.labelColor);
    RB_DrawProfileString(84, y, "  AvH", drawProfGlob.labelColor);
    RB_DrawProfileString(89, y, "   ApH", drawProfGlob.labelColor);
    RB_DrawProfileString(95, y, "  MaxH", drawProfGlob.labelColor);
    return (float)(y + drawProfGlob.fontHeight);
}

void __cdecl RB_DrawProfileString(int column, float y, const char *s, GfxColor textColor)
{
    float v4; // [esp+Ch] [ebp-8h]
    float x; // [esp+10h] [ebp-4h]

    x = (double)column * drawProfGlob.fontWidth;
    v4 = y + drawProfGlob.fontHeight;
    RB_DrawText(s, drawProfGlob.font, x, v4, textColor);
}

double __cdecl RB_DrawProfileRow(int probeIndex, int indentation, float y)
{
    const char *v3; // eax
    const char *v4; // eax
    const char *v5; // eax
    const char *v6; // eax
    const char *v7; // eax
    const char *v8; // eax
    const char *v9; // eax
    const char *v10; // eax
    const char *v11; // eax
    const char *v12; // eax
    const char *v13; // eax
    GfxColor v15; // [esp+8h] [ebp-C4h]
    GfxColor v16; // [esp+8h] [ebp-C4h]
    GfxColor v17; // [esp+8h] [ebp-C4h]
    GfxColor v18; // [esp+8h] [ebp-C4h]
    GfxColor v19; // [esp+8h] [ebp-C4h]
    GfxColor v20; // [esp+8h] [ebp-C4h]
    GfxColor v21; // [esp+8h] [ebp-C4h]
    GfxColor v22; // [esp+8h] [ebp-C4h]
    GfxColor v23; // [esp+8h] [ebp-C4h]
    GfxColor v24; // [esp+8h] [ebp-C4h]
    GfxColor v25; // [esp+8h] [ebp-C4h]
    char *s; // [esp+10h] [ebp-BCh]
    char *format; // [esp+14h] [ebp-B8h]
    double v29; // [esp+40h] [ebp-8Ch]
    float v30; // [esp+50h] [ebp-7Ch]
    float v31; // [esp+58h] [ebp-74h]
    float v32; // [esp+60h] [ebp-6Ch]
    float v33; // [esp+68h] [ebp-64h]
    float v34; // [esp+70h] [ebp-5Ch]
    ProfileReadableGlobal *global; // [esp+88h] [ebp-44h]
    float avgTotal; // [esp+8Ch] [ebp-40h]
    float avgHits; // [esp+94h] [ebp-38h]
    float avgTotPerHit; // [esp+98h] [ebp-34h]
    unsigned int minTime; // [esp+9Ch] [ebp-30h]
    float avgSelf; // [esp+A4h] [ebp-28h]
    char probeName[28]; // [esp+A8h] [ebp-24h] BYREF
    int col; // [esp+C8h] [ebp-4h]

    if (probeIndex)
    {
        global = &drawProfGlob.global[probeIndex];
        col = indentation + 1;
        if (!global->sequence)
            MyAssertHandler(".\\rb_drawprofile.cpp", 380, 0, "%s", "global->sequence");
        avgSelf = *((float *)Sys_GetValue(0) + 20782) * global->selfClks / (double)global->sequence;
        avgTotal = *((float *)Sys_GetValue(0) + 20782) * global->totalClks / (double)global->sequence;
        avgHits = (double)global->hits / (double)global->sequence;
        minTime = global->min.value[0];
        if (avgHits == 0.0)
            v29 = 0.0;
        else
            v29 = avgTotal / avgHits;
        I_strncpyz(probeName, (char *)prof_enumNames[probeIndex], 28 - indentation);
        RB_DrawProfileString(col, y, probeName, drawProfGlob.textColor);
        v34 = *((float *)Sys_GetValue(0) + 20782);
        v15.packed = (unsigned int)drawProfGlob.textColor;
        v3 = va("%6.2f", (double)drawProfGlob.global[probeIndex].read.self.value[0] * v34);
        RB_DrawProfileString(28, y, v3, v15);
        v33 = *((float *)Sys_GetValue(0) + 20782);
        v16.packed = (unsigned int)drawProfGlob.textColor;
        v4 = va("%6.2f", (double)drawProfGlob.global[probeIndex].read.total.value[0] * v33);
        RB_DrawProfileString(34, y, v4, v16);
        if (avgSelf > 30.0)
            avgSelf = 0.0;
        v17.packed = (unsigned int)drawProfGlob.textColor;
        v5 = va("%6.2f", avgSelf);
        RB_DrawProfileString(40, y, v5, v17);
        v32 = *((float *)Sys_GetValue(0) + 20782);
        v18.packed = (unsigned int)drawProfGlob.textColor;
        v6 = va("%7.2f", (double)global->maxSelf.value[0] * v32);
        RB_DrawProfileString(46, y, v6, v18);
        g_tally = g_tally + avgSelf;
        v19.packed = (unsigned int)drawProfGlob.textColor;
        v7 = va("%6.2f", g_tally);
        RB_DrawProfileString(53, y, v7, v19);
        v20.packed = (unsigned int)drawProfGlob.textColor;
        v8 = va("%6.2f", avgTotal);
        RB_DrawProfileString(60, y, v8, v20);
        v31 = *((float *)Sys_GetValue(0) + 20782);
        v21.packed = (unsigned int)drawProfGlob.textColor;
        v9 = va("%6.2f", (double)minTime * v31);
        RB_DrawProfileString(66, y, v9, v21);
        v30 = *((float *)Sys_GetValue(0) + 20782);
        v22.packed = (unsigned int)drawProfGlob.textColor;
        v10 = va("%7.2f", (double)global->max.value[0] * v30);
        RB_DrawProfileString(72, y, v10, v22);
        v23.packed = (unsigned int)drawProfGlob.textColor;
        v11 = va("%5u", drawProfGlob.global[probeIndex].read.hits);
        RB_DrawProfileString(79, y, v11, v23);
        if (avgHits == 0.0 || avgHits >= 10.0)
            format = (char*)"%5.0f";
        else
            format = (char*)"%5.1f";
        v24.packed = (unsigned int)drawProfGlob.textColor;
        v12 = va(format, avgHits);
        RB_DrawProfileString(84, y, v12, v24);
        if (avgHits == 0.0)
        {
            s = (char*)"      ";
        }
        else
        {
            avgTotPerHit = v29;
            s = va("%6.2f", avgTotPerHit);
        }
        RB_DrawProfileString(89, y, s, drawProfGlob.textColor);
        v25.packed = (unsigned int)drawProfGlob.textColor;
        v13 = va("%5u", global->maxHits);
        RB_DrawProfileString(95, y, v13, v25);
    }
    return (float)(y + drawProfGlob.fontHeight);
}

int __cdecl CompareSelfTimes(unsigned int *e0, unsigned int *e1)
{
    if (drawProfGlob.global[*e1].read.self.value[0] == drawProfGlob.global[*e0].read.self.value[0])
        return *e1 - *e0;
    else
        return drawProfGlob.global[*e1].read.self.value[0] - drawProfGlob.global[*e0].read.self.value[0];
}

int __cdecl CompareTotalTimes(unsigned int *e0, unsigned int *e1)
{
    if (drawProfGlob.global[*e1].read.total.value[0] == drawProfGlob.global[*e0].read.total.value[0])
        return *e1 - *e0;
    else
        return drawProfGlob.global[*e1].read.total.value[0] - drawProfGlob.global[*e0].read.total.value[0];
}

int __cdecl CompareAvgTotalTimes(int *e0, int *e1)
{
    double delta; // [esp+0h] [ebp-10h]
    int i0; // [esp+8h] [ebp-8h]
    int i1; // [esp+Ch] [ebp-4h]

    i0 = *e0;
    i1 = *e1;
    delta = (double)drawProfGlob.global[i0].sequence * drawProfGlob.global[i1].totalClks
        - (double)drawProfGlob.global[i1].sequence * drawProfGlob.global[i0].totalClks;
    if (delta < 0.0)
        return -1;
    if (delta <= 0.0)
        return i1 - i0;
    return 1;
}

int __cdecl CompareMaxTimes(unsigned int *e0, unsigned int *e1)
{
    int delta; // [esp+0h] [ebp-Ch]

    delta = drawProfGlob.global[*e1].max.value[0] - drawProfGlob.global[*e0].max.value[0];
    if ((double)delta < 0.0)
        return -1;
    if ((double)delta <= 0.0)
        return *e1 - *e0;
    return 1;
}

int __cdecl CompareAvgSelfTimes(int *e0, int *e1)
{
    double delta; // [esp+0h] [ebp-10h]
    int i0; // [esp+8h] [ebp-8h]
    int i1; // [esp+Ch] [ebp-4h]

    i0 = *e0;
    i1 = *e1;
    delta = (double)drawProfGlob.global[i0].sequence * drawProfGlob.global[i1].selfClks
        - (double)drawProfGlob.global[i1].sequence * drawProfGlob.global[i0].selfClks;
    if (delta < 0.0)
        return -1;
    if (delta <= 0.0)
        return i1 - i0;
    return 1;
}

int __cdecl CompareMaxSelfTimes(unsigned int *e0, unsigned int *e1)
{
    int delta; // [esp+0h] [ebp-Ch]

    delta = drawProfGlob.global[*e1].maxSelf.value[0] - drawProfGlob.global[*e0].maxSelf.value[0];
    if ((double)delta < 0.0)
        return -1;
    if ((double)delta <= 0.0)
        return *e1 - *e0;
    return 1;
}

void __cdecl RB_DrawProfileScript()
{
    const char *v0; // eax
    const char *v1; // eax
    const char *v2; // eax
    const char *v3; // eax
    const char *v4; // eax
    const char *v5; // eax
    const char *v6; // eax
    const char *v7; // eax
    const char *v8; // eax
    const char *v9; // eax
    const char *v10; // eax
    const char *v11; // eax
    const char *v12; // eax
    const char *v13; // eax
    GfxColor v14; // [esp+8h] [ebp-9Ch]
    GfxColor v15; // [esp+8h] [ebp-9Ch]
    GfxColor v16; // [esp+8h] [ebp-9Ch]
    GfxColor v17; // [esp+8h] [ebp-9Ch]
    GfxColor v18; // [esp+8h] [ebp-9Ch]
    GfxColor v19; // [esp+8h] [ebp-9Ch]
    GfxColor v20; // [esp+8h] [ebp-9Ch]
    GfxColor v21; // [esp+8h] [ebp-9Ch]
    GfxColor v22; // [esp+8h] [ebp-9Ch]
    GfxColor v23; // [esp+8h] [ebp-9Ch]
    GfxColor v24; // [esp+8h] [ebp-9Ch]
    GfxColor v25; // [esp+8h] [ebp-9Ch]
    GfxColor v26; // [esp+8h] [ebp-9Ch]
    GfxColor v27; // [esp+8h] [ebp-9Ch]
    float *Value; // [esp+40h] [ebp-64h]
    float v29; // [esp+44h] [ebp-60h]
    float v30; // [esp+4Ch] [ebp-58h]
    float v31; // [esp+54h] [ebp-50h]
    float v32; // [esp+5Ch] [ebp-48h]
    float v33; // [esp+64h] [ebp-40h]
    float v34; // [esp+6Ch] [ebp-38h]
    float v35; // [esp+74h] [ebp-30h]
    float v36; // [esp+7Ch] [ebp-28h]
    SourceBufferInfo *srcBuffer; // [esp+84h] [ebp-20h]
    unsigned int i; // [esp+88h] [ebp-1Ch]
    ProfileScript *profile; // [esp+94h] [ebp-10h]
    ProfileScript *profilea; // [esp+94h] [ebp-10h]
    float y; // [esp+9Ch] [ebp-8h]
    float ya; // [esp+9Ch] [ebp-8h]
    float yb; // [esp+9Ch] [ebp-8h]
    int profileIndex; // [esp+A0h] [ebp-4h]

    if (profile_script->current.enabled)
    {
        profile = Profile_GetScript();
            RB_DrawProfileBackground(30.0);
        RB_DrawProfileString(5, 30.0, "Probe Name", drawProfGlob.labelColor);
        RB_DrawProfileString(28, 30.0, " Current", drawProfGlob.labelColor);
        RB_DrawProfileString(38, 30.0, " Avg", drawProfGlob.labelColor);
        RB_DrawProfileString(48, 30.0, " Max", drawProfGlob.labelColor);
        RB_DrawProfileString(58, 30.0, "  Total", drawProfGlob.labelColor);
        y = (float)30.0 + drawProfGlob.fontHeight;
        for (profileIndex = 0; profileIndex < 40; ++profileIndex)
        {
            if (profile->profileScriptNames[profileIndex][0])
            {
                RB_DrawProfileBackground(y);
                v14.packed = (unsigned int)drawProfGlob.textColor;
                v0 = va("%02d", profileIndex);
                RB_DrawProfileString(1, y, v0, v14);
                RB_DrawProfileString(5, y, profile->profileScriptNames[profileIndex], drawProfGlob.textColor);
                v36 = *((float *)Sys_GetValue(0) + 20782);
                v15.packed = (unsigned int)drawProfGlob.textColor;
                v1 = va("%6.2f", (double)profile->totalTime[profileIndex] * v36);
                RB_DrawProfileString(28, y, v1, v15);
                v35 = *((float *)Sys_GetValue(0) + 20782);
                v16.packed = (unsigned int)drawProfGlob.textColor;
                v2 = va("%6.2f", (double)profile->avgTime[profileIndex] * v35);
                RB_DrawProfileString(38, y, v2, v16);
                v34 = *((float *)Sys_GetValue(0) + 20782);
                v17.packed = (unsigned int)drawProfGlob.textColor;
                v3 = va("%6.2f", (double)profile->maxTime[profileIndex] * v34);
                RB_DrawProfileString(48, y, v3, v17);
                v18.packed = (unsigned int)drawProfGlob.textColor;
                v4 = va("%8.0f", profile->cumulative[profileIndex]);
                RB_DrawProfileString(58, y, v4, v18);
                y = y + drawProfGlob.fontHeight;
            }
        }
    }
    else if (profile_script_by_file->current.enabled)
    {
        profilea = Profile_GetScript();
            RB_DrawProfileBackground(30.0);
        RB_DrawProfileString(1, 30.0, " File", drawProfGlob.labelColor);
        RB_DrawProfileString(33, 30.0, "Current", drawProfGlob.labelColor);
        RB_DrawProfileString(45, 30.0, "Avg", drawProfGlob.labelColor);
        RB_DrawProfileString(57, 30.0, "Max", drawProfGlob.labelColor);
        RB_DrawProfileString(69, 30.0, "BuiltIn", drawProfGlob.labelColor);
        RB_DrawProfileString(81, 30.0, "NonBuiltIn", drawProfGlob.labelColor);
        RB_DrawProfileString(93, 30.0, "Total", drawProfGlob.labelColor);
        ya = (float)30.0 + drawProfGlob.fontHeight;
        for (i = 0; i < 0x20; ++i)
        {
            srcBuffer = &scrParserPub.sourceBufferLookup[profilea->scriptSrcBufferIndex[i]];
            if (srcBuffer->totalTime != 0.0)
            {
                RB_DrawProfileBackground(ya);
                RB_DrawProfileString(1, ya, srcBuffer->buf, drawProfGlob.textColor);
                v33 = *((float *)Sys_GetValue(0) + 20782);
                v19.packed = (unsigned int)drawProfGlob.textColor;
                v5 = va("%6.2f", (double)srcBuffer->time * v33);
                RB_DrawProfileString(33, ya, v5, v19);
                v32 = *((float *)Sys_GetValue(0) + 20782);
                v20.packed = (unsigned int)drawProfGlob.textColor;
                v6 = va("%6.2f", (double)srcBuffer->avgTime * v32);
                RB_DrawProfileString(45, ya, v6, v20);
                v31 = *((float *)Sys_GetValue(0) + 20782);
                v21.packed = (unsigned int)drawProfGlob.textColor;
                v7 = va("%6.2f", (double)srcBuffer->maxTime * v31);
                RB_DrawProfileString(57, ya, v7, v21);
                v22.packed = (unsigned int)drawProfGlob.textColor;
                v8 = va("%8.0f", srcBuffer->totalBuiltIn);
                RB_DrawProfileString(69, ya, v8, v22);
                v23.packed = (unsigned int)drawProfGlob.textColor;
                v9 = va("%8.0f", srcBuffer->totalTime - srcBuffer->totalBuiltIn);
                RB_DrawProfileString(81, ya, v9, v23);
                v24.packed = (unsigned int)drawProfGlob.textColor;
                v10 = va("%8.0f", srcBuffer->totalTime);
                RB_DrawProfileString(93, ya, v10, v24);
                ya = ya + drawProfGlob.fontHeight;
            }
        }
        yb = ya + drawProfGlob.fontHeight;
        RB_DrawProfileBackground(yb);
        RB_DrawProfileString(1, yb, "Total", drawProfGlob.textColor);
        v30 = *((float *)Sys_GetValue(0) + 20782);
        v25.packed = (unsigned int)drawProfGlob.textColor;
        v11 = va("%6.2f", (double)profilea->srcTotal * v30);
        RB_DrawProfileString(33, yb, v11, v25);
        v29 = *((float *)Sys_GetValue(0) + 20782);
        v26.packed = (unsigned int)drawProfGlob.textColor;
        v12 = va("%6.2f", (double)profilea->srcAvgTime * v29);
        RB_DrawProfileString(45, yb, v12, v26);
        Value = (float *)Sys_GetValue(0);
        v27.packed = (unsigned int)drawProfGlob.textColor;
        v13 = va("%6.2f", (double)profilea->srcMaxTime * Value[20782]);
        RB_DrawProfileString(57, yb, v13, v27);
    }
}

void __cdecl RB_ProfileInit()
{
    drawProfGlob.font = R_RegisterFont("fonts/consoleFont", 3);
    drawProfGlob.fontWidth = (float)R_TextWidth("#", 1, drawProfGlob.font);
    drawProfGlob.fontHeight = (float)R_TextHeight(drawProfGlob.font);
    drawProfGlob.textColor.packed = -1;
    drawProfGlob.labelColor.packed = -256;
    memset((unsigned __int8 *)drawProfGlob.log, 0, sizeof(drawProfGlob.log));
    drawProfGlob.lastSortTime = 0;
}

