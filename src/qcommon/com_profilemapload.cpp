#include "qcommon.h"
#include "mem_track.h"
#include "threads.h"
#include <universal/timing.h>
#include <client/client.h>

#include <universal/com_math.h>

#include <algorithm>

mapLoadProfile_t mapLoadProfile;

const dvar_t *com_profileLoading;

static const float PROFLOAD_FONT_SCALE = 0.36f;
static const float PROFLOAD_TEXT_COLOR[4] = { 0.0f, 1.0f, 0.0f, 1.0f };
static const float PROFLOAD_BACKGROUND_COLOR[4] = { 0.0f, 0.0f, 0.0f, 0.8f };

void __cdecl TRACK_com_profilemapload()
{
    track_static_alloc_internal(&mapLoadProfile, 43136, "mapLoadProfile", 0);
}

bool __cdecl ProfLoad_IsActive()
{
    return mapLoadProfile.isLoading;
}

void __cdecl ProfLoad_BeginTrackedValue(MapProfileTrackedValue type)
{
    MapProfileEntry *entry; // [esp+0h] [ebp-Ch]
    unsigned __int64 ticks; // [esp+4h] [ebp-8h]

    if (mapLoadProfile.isLoading && mapLoadProfile.currentEntry && Sys_IsMainThread())
    {
        ticks = __rdtsc();
        ProfLoad_BeginTrackedValueTicks(&mapLoadProfile.elements[type], ticks);
        ++mapLoadProfile.elementAccessCount[type];
        for (entry = mapLoadProfile.currentEntry; entry; entry = entry->parent)
            ProfLoad_BeginTrackedValueTicks(&entry->elements[type], ticks);
    }
}

void __cdecl ProfLoad_BeginTrackedValueTicks(MapProfileElement *value, unsigned __int64 ticks)
{
    iassert( value->ticksStart == 0 );
    value->ticksStart = ticks;
}

void __cdecl ProfLoad_EndTrackedValue(MapProfileTrackedValue type)
{
    MapProfileEntry *entry; // [esp+0h] [ebp-Ch]
    unsigned __int64 ticks; // [esp+4h] [ebp-8h]

    if (mapLoadProfile.isLoading && mapLoadProfile.currentEntry && Sys_IsMainThread())
    {
        ticks = __rdtsc();
        ProfLoad_EndTrackedValueTicks(&mapLoadProfile.elements[type], ticks);
        for (entry = mapLoadProfile.currentEntry; entry; entry = entry->parent)
            ProfLoad_EndTrackedValueTicks(&entry->elements[type], ticks);
    }
}

void __cdecl ProfLoad_EndTrackedValueTicks(MapProfileElement *value, unsigned __int64 ticks)
{
    iassert( value->ticksStart != 0 );
    value->ticksTotal += ticks - value->ticksStart;
    value->ticksStart = 0;
}

void __cdecl ProfLoad_Init()
{
    com_profileLoading = Dvar_RegisterBool("profile_show_loading", 0, DVAR_NOFLAG, "Show map load profiler");
    mapLoadProfile.isLoading = 0;
}

void __cdecl ProfLoad_Activate()
{
    iassert( mapLoadProfile.isLoading == false );
    memset((unsigned __int8 *)&mapLoadProfile, 0, sizeof(mapLoadProfile));
    mapLoadProfile.isLoading = 1;
    mapLoadProfile.ticksStart = __rdtsc();
    mapLoadProfile.ticksFinish = mapLoadProfile.ticksStart;
    Com_Printf(12, "^6Activating map load profiler\n");
}

void __cdecl ProfLoad_Deactivate()
{
    iassert( mapLoadProfile.isLoading == true );
    mapLoadProfile.ticksFinish = __rdtsc();
    mapLoadProfile.isLoading = 0;
    ProfLoad_Print();
}

void ProfLoad_Print()
{
    int fileReadCount; // [esp+6Ch] [ebp-18h]
    int fileOpenCount; // [esp+74h] [ebp-10h]
    int fileSeekCount; // [esp+80h] [ebp-4h]

    ProfLoad_CalculateSelfTicks();
    ProfLoad_PrintTree();
    ProfLoad_PrintHotSpots();
    fileOpenCount = mapLoadProfile.elementAccessCount[0];
    fileSeekCount = mapLoadProfile.elementAccessCount[1];
    fileReadCount = mapLoadProfile.elementAccessCount[2];
    Com_Printf(
        12,
        "^6Total Load Time: %5.4f\n",
        (double)((double)(mapLoadProfile.ticksFinish - mapLoadProfile.ticksStart)
            * msecPerRawTimerTick
            * EQUAL_EPSILON));
    Com_Printf(
        12,
        "^6Profiled Time: %5.4f\n",
        (double)((double)mapLoadProfile.ticksProfiled * msecPerRawTimerTick * EQUAL_EPSILON));
    Com_Printf(
        12,
        "^6Unprofiled Time: %5.4f\n",
        (double)((double)(mapLoadProfile.ticksFinish - mapLoadProfile.ticksStart - mapLoadProfile.ticksProfiled)
            * msecPerRawTimerTick
            * EQUAL_EPSILON));
    Com_Printf(
        12,
        "^6File Open Time: %5.4f, Accessed %d times\n",
        (double)((double)mapLoadProfile.elements[0].ticksTotal * msecPerRawTimerTick * EQUAL_EPSILON),
        fileOpenCount);
    Com_Printf(
        12,
        "^6File Seek Time: %5.4f, Accessed %d times\n",
        (double)((double)mapLoadProfile.elements[1].ticksTotal * msecPerRawTimerTick * EQUAL_EPSILON),
        fileSeekCount);
    Com_Printf(
        12,
        "^6File Read Time: %5.4f, Accessed %d times\n",
        (double)((double)mapLoadProfile.elements[2].ticksTotal * msecPerRawTimerTick * EQUAL_EPSILON),
        fileReadCount);
}

void  ProfLoad_CalculateSelfTicks()
{
    int v0; // ecx
    MapProfileEntry *parentEntry; // [esp+8h] [ebp-10h]
    MapProfileEntry *entry; // [esp+Ch] [ebp-Ch]
    MapProfileEntry *entrya; // [esp+Ch] [ebp-Ch]
    int entryIndex; // [esp+10h] [ebp-8h]
    int entryIndexa; // [esp+10h] [ebp-8h]
    int elemIndex; // [esp+14h] [ebp-4h]
    int elemIndexa; // [esp+14h] [ebp-4h]

    for (entryIndex = 0; entryIndex < mapLoadProfile.profileEntryCount; ++entryIndex)
    {
        entry = &mapLoadProfile.profileEntries[entryIndex];
        LODWORD(entry->ticksSelf) = entry->ticksTotal;
        HIDWORD(entry->ticksSelf) = HIDWORD(entry->ticksTotal);
        for (elemIndex = 0; elemIndex < 3; ++elemIndex)
        {
            v0 = elemIndex;
            LODWORD(entry->elements[v0].ticksSelf) = entry->elements[elemIndex].ticksTotal;
            HIDWORD(entry->elements[v0].ticksSelf) = HIDWORD(entry->elements[elemIndex].ticksTotal);
        }
    }
    for (entryIndexa = 0; entryIndexa < mapLoadProfile.profileEntryCount; ++entryIndexa)
    {
        entrya = &mapLoadProfile.profileEntries[entryIndexa];
        parentEntry = entrya->parent;
        if (parentEntry)
        {
            parentEntry->ticksSelf -= entrya->ticksTotal;
            for (elemIndexa = 0; elemIndexa < 3; ++elemIndexa)
                parentEntry->elements[elemIndexa].ticksSelf -= entrya->elements[elemIndexa].ticksTotal;
        }
    }
}

int ProfLoad_PrintTree()
{
    int result; // eax
    const MapProfileEntry *entry; // [esp+0h] [ebp-10Ch]
    char rowText[256]; // [esp+4h] [ebp-108h] BYREF
    int profileIndex; // [esp+108h] [ebp-4h]

    for (profileIndex = 0; profileIndex < mapLoadProfile.profileEntryCount; ++profileIndex)
    {
        entry = &mapLoadProfile.profileEntries[profileIndex];
        ProfLoad_GetEntryRowText(entry, rowText, 256);
        Com_Printf(12, "^6%*c %s\n", 2 * entry->indent + 1, 32, rowText);
        result = profileIndex + 1;
    }
    return result;
}

void __cdecl ProfLoad_GetEntryRowText(const MapProfileEntry *entry, char *rowText, int sizeofRowText)
{
    Com_sprintf(
        rowText,
        sizeofRowText,
        "%s - %5.3f total, %5.3f self, %5.3f file, %i hits",
        entry->label,
        (double)((double)entry->ticksTotal * msecPerRawTimerTick * EQUAL_EPSILON),
        (double)((double)entry->ticksSelf * msecPerRawTimerTick * EQUAL_EPSILON),
        (double)((double)(entry->elements[1].ticksSelf + entry->elements[2].ticksSelf + entry->elements[0].ticksSelf)
            * msecPerRawTimerTick
            * EQUAL_EPSILON),
        entry->accessCount);
}

void ProfLoad_PrintHotSpots()
{
    int v0; // eax
    MapProfileEntry *v1; // ecx
    unsigned __int64 v2; // kr08_8
    int v3; // eax
    MapProfileHotSpot *v4; // eax
    MapProfileHotSpot *v5; // edx
    unsigned __int64 v6; // kr10_8
    int v7; // edx
    unsigned __int64 v8; // kr18_8
    int v9; // ecx
    int v10; // [esp+34h] [ebp-24A4h]
    MapProfileHotSpot v11[384]; // [esp+A0h] [ebp-2438h] BYREF
    MapProfileEntry *v12; // [esp+24A4h] [ebp-34h]
    int j; // [esp+24A8h] [ebp-30h]
    int v14; // [esp+24ACh] [ebp-2Ch]
    long double v15; // [esp+24B0h] [ebp-28h]
    long double v16; // [esp+24B8h] [ebp-20h]
    long double v17; // [esp+24C0h] [ebp-18h]
    long double v18; // [esp+24C8h] [ebp-10h]
    int i; // [esp+24D4h] [ebp-4h]

    for (i = 0; i < mapLoadProfile.profileEntryCount; ++i)
    {
        v12 = &mapLoadProfile.profileEntries[i];
        v11[i].label = v12->label;
        v11[i].accessCount = v12->accessCount;
        v0 = i;
        v1 = v12;
        LODWORD(v11[v0].ticksSelf) = v12->ticksSelf;
        HIDWORD(v11[v0].ticksSelf) = HIDWORD(v1->ticksSelf);
        v2 = v12->elements[2].ticksSelf + v12->elements[1].ticksSelf + v12->elements[0].ticksSelf;
        v3 = i;
        LODWORD(v11[v3].ticksFile) = v2;
        HIDWORD(v11[v3].ticksFile) = HIDWORD(v2);
    }
    //std::_Sort<MapProfileHotSpot *, int, bool(__cdecl *)(MapProfileHotSpot const &, MapProfileHotSpot const &)>(
    //    v11,
    //    &v11[mapLoadProfile.profileEntryCount],
    //    24 * mapLoadProfile.profileEntryCount / 24,
    //    (bool(__cdecl *)(const MapProfileHotSpot *, const MapProfileHotSpot *))ProfLoad_CompareHotSpotNames);
    //std::sort(&v11[0], &v11[mapLoadProfile.profileEntryCount], 24 * mapLoadProfile.profileEntryCount / 24, ProfLoad_CompareHotSpotNames);
    std::sort(&v11[0], &v11[mapLoadProfile.profileEntryCount], ProfLoad_CompareHotSpotNames);
    v14 = 0;
    i = 0;
    while (i != mapLoadProfile.profileEntryCount)
    {
        v4 = &v11[i];
        v5 = &v11[v14];
        v5->label = v4->label;
        v5->accessCount = v4->accessCount;
        LODWORD(v5->ticksSelf) = v4->ticksSelf;
        HIDWORD(v5->ticksSelf) = HIDWORD(v4->ticksSelf);
        LODWORD(v5->ticksFile) = v4->ticksFile;
        HIDWORD(v5->ticksFile) = HIDWORD(v4->ticksFile);
        while (++i != mapLoadProfile.profileEntryCount && v11[i].label == v11[v14].label)
        {
            v11[v14].accessCount += v11[i].accessCount;
            v6 = __PAIR64__(HIDWORD(v11[i].ticksSelf), v11[i].ticksSelf)
                + __PAIR64__(HIDWORD(v11[v14].ticksSelf), v11[v14].ticksSelf);
            v7 = v14;
            LODWORD(v11[v7].ticksSelf) = LODWORD(v11[i].ticksSelf) + LODWORD(v11[v14].ticksSelf);
            HIDWORD(v11[v7].ticksSelf) = HIDWORD(v6);
            v8 = __PAIR64__(HIDWORD(v11[i].ticksFile), v11[i].ticksFile)
                + __PAIR64__(HIDWORD(v11[v14].ticksFile), v11[v14].ticksFile);
            v9 = v14;
            LODWORD(v11[v9].ticksFile) = LODWORD(v11[i].ticksFile) + LODWORD(v11[v14].ticksFile);
            HIDWORD(v11[v9].ticksFile) = HIDWORD(v8);
        }
        ++v14;
    }
    //std::_Sort<MapProfileHotSpot *, int, bool(__cdecl *)(MapProfileHotSpot const &, MapProfileHotSpot const &)>(
    //    v11,
    //    &v11[v14],
    //    24 * v14 / 24,
    //    ProfLoad_CompareHotSpotTicks);
    std::sort(&v11[0], &v11[v14], ProfLoad_CompareHotSpotTicks);
    Com_Printf(12, "\n\n^6---------- Load time hot spots ----------\n");
    if (v14 > 16)
        v10 = 16;
    else
        v10 = v14;
    v14 = v10;
    v15 = 0.0;
    v16 = 0.0;
    for (j = 0; j < v14; ++j)
    {
        v18 = (double)__PAIR64__(HIDWORD(v11[j].ticksSelf), v11[j].ticksSelf) * msecPerRawTimerTick * EQUAL_EPSILON;
        v17 = (double)__PAIR64__(HIDWORD(v11[j].ticksFile), v11[j].ticksFile) * msecPerRawTimerTick * EQUAL_EPSILON;
        v15 = v15 + v18;
        v16 = v16 + v17;
        Com_Printf(
            12,
            "^6%s: %5.3f self, %5.3f file, %i hits\n",
            v11[j].label,
            (double)v18,
            (double)v17,
            v11[j].accessCount);
    }
    Com_Printf(12, "\n^6Hot spot total time: %5.3f self, %5.3f file\n\n", (double)v15, (double)v16);
}

bool __cdecl ProfLoad_CompareHotSpotNames(const MapProfileHotSpot &hotSpot0, const MapProfileHotSpot &hotSpot1)
{
    return hotSpot1.label < hotSpot0.label;
}

bool __cdecl ProfLoad_CompareHotSpotTicks(const MapProfileHotSpot &hotSpot0, const MapProfileHotSpot &hotSpot1)
{
    return hotSpot0.ticksSelf > hotSpot1.ticksSelf;
}

void __cdecl ProfLoad_Begin(const char *label)
{
    int v1; // [esp+0h] [ebp-10h]
    MapProfileEntry *entry; // [esp+Ch] [ebp-4h]

    if (mapLoadProfile.isLoading && Sys_IsMainThread())
    {
        entry = Com_GetEntryForNewLabel(label);
        ++entry->accessCount;
        entry->parent = mapLoadProfile.currentEntry;
        if (entry->parent)
            v1 = entry->parent->indent + 1;
        else
            v1 = 0;
        entry->indent = v1;
        mapLoadProfile.currentEntry = entry;
        entry->ticksStart = __rdtsc();
        entry->label = label;
    }
}

MapProfileEntry *__cdecl Com_GetEntryForNewLabel(const char *label)
{
    int entryIndex; // [esp+4h] [ebp-8h]
    int firstEntry; // [esp+8h] [ebp-4h]

    if (mapLoadProfile.currentEntry)
        firstEntry = mapLoadProfile.currentEntry - mapLoadProfile.profileEntries + 1;
    else
        firstEntry = 0;
    for (entryIndex = firstEntry; entryIndex < mapLoadProfile.profileEntryCount; ++entryIndex)
    {
        if (mapLoadProfile.profileEntries[entryIndex].parent == mapLoadProfile.currentEntry
            && mapLoadProfile.profileEntries[entryIndex].label == label)
        {
            return &mapLoadProfile.profileEntries[entryIndex];
        }
    }
    if (mapLoadProfile.profileEntryCount >= 384)
        MyAssertHandler(
            ".\\qcommon\\com_profilemapload.cpp",
            221,
            0,
            "%s",
            "mapLoadProfile.profileEntryCount < PROFLOAD_ENTRY_COUNT");
    return &mapLoadProfile.profileEntries[mapLoadProfile.profileEntryCount++];
}

void __cdecl ProfLoad_End()
{
    MapProfileEntry *entry; // [esp+0h] [ebp-14h]
    unsigned __int64 timeStepInTicks; // [esp+4h] [ebp-10h]

    if (mapLoadProfile.isLoading && Sys_IsMainThread())
    {
        entry = mapLoadProfile.currentEntry;
        iassert( entry );
        iassert( entry->label );
        mapLoadProfile.ticksFinish = __rdtsc();
        timeStepInTicks = mapLoadProfile.ticksFinish - entry->ticksStart;
        entry->ticksTotal += timeStepInTicks;
        if (!entry->parent)
            mapLoadProfile.ticksProfiled += timeStepInTicks;
        mapLoadProfile.currentEntry = entry->parent;
    }
}

void __cdecl ProfLoad_DrawOverlay(rectDef_s *rect)
{
    MapProfileEntry *v1; // eax
    int fileReadCount; // [esp+7Ch] [ebp-128h]
    Font_s *profileFont; // [esp+88h] [ebp-11Ch]
    int fileOpenCount; // [esp+8Ch] [ebp-118h]
    char line[256]; // [esp+94h] [ebp-110h] BYREF
    float y; // [esp+198h] [ebp-Ch]
    MapProfileElement *fileOpenElement; // [esp+19Ch] [ebp-8h]
    int fileSeekCount; // [esp+1A0h] [ebp-4h]

    if (com_profileLoading->current.enabled)
    {
        y = 395.0f;
        profileFont = UI_GetFontHandle(&scrPlaceFull, 0, 0.36000001f);
        UI_FillRect(
            &scrPlaceFull,
            rect->x,
            rect->y,
            rect->w,
            rect->h,
            rect->horzAlign,
            rect->vertAlign,
            PROFLOAD_BACKGROUND_COLOR);
        ProfLoad_DrawTree();
        fileOpenElement = mapLoadProfile.elements;
        fileOpenCount = mapLoadProfile.elementAccessCount[0];
        fileSeekCount = mapLoadProfile.elementAccessCount[1];
        fileReadCount = mapLoadProfile.elementAccessCount[2];
        sprintf(
            line,
            "Total Load Time: %5.4f",
            (double)((double)(mapLoadProfile.ticksFinish - mapLoadProfile.ticksStart)
                * msecPerRawTimerTick
                * EQUAL_EPSILON));
        UI_DrawText(
            &scrPlaceFull,
            line,
            256,
            profileFont,
            60.0f,
            y,
            rect->horzAlign,
            rect->vertAlign,
            0.36000001f,
            PROFLOAD_TEXT_COLOR,
            0);
        sprintf(
            line,
            "File Open Time: %5.4f, Accessed %d times",
            (double)((double)fileOpenElement->ticksTotal * msecPerRawTimerTick * EQUAL_EPSILON),
            fileOpenCount);
        UI_DrawText(
            &scrPlaceFull,
            line,
            256,
            profileFont,
            250.0,
            y,
            rect->horzAlign,
            rect->vertAlign,
            0.36000001f,
            PROFLOAD_TEXT_COLOR,
            0);
        y = (double)15 + y;
        sprintf(
            line,
            "Profiled Time: %5.4f",
            (double)((double)mapLoadProfile.ticksProfiled * msecPerRawTimerTick * EQUAL_EPSILON));
        UI_DrawText(
            &scrPlaceFull,
            line,
            256,
            profileFont,
            60.0,
            y,
            rect->horzAlign,
            rect->vertAlign,
            0.36000001f,
            PROFLOAD_TEXT_COLOR,
            0);
        sprintf(
            line,
            "File Seek Time: %5.4f, Accessed %d times",
            (double)((double)mapLoadProfile.elements[1].ticksTotal * msecPerRawTimerTick * EQUAL_EPSILON),
            fileSeekCount);
        UI_DrawText(
            &scrPlaceFull,
            line,
            256,
            profileFont,
            250.0f,
            y,
            rect->horzAlign,
            rect->vertAlign,
            0.36000001f,
            PROFLOAD_TEXT_COLOR,
            0);
        y = (double)15 + y;
        sprintf(
            line,
            "Unprofiled Time: %5.4f",
            (double)((double)(mapLoadProfile.ticksFinish - mapLoadProfile.ticksStart - mapLoadProfile.ticksProfiled)
                * msecPerRawTimerTick
                * EQUAL_EPSILON));
        UI_DrawText(
            &scrPlaceFull,
            line,
            256,
            profileFont,
            60.0f,
            y,
            rect->horzAlign,
            rect->vertAlign,
            0.36000001f,
            PROFLOAD_TEXT_COLOR,
            0);
        sprintf(
            line,
            "File Read Time: %5.4f, Accessed %d times",
            (double)((double)mapLoadProfile.elements[2].ticksTotal * msecPerRawTimerTick * EQUAL_EPSILON),
            fileReadCount);
        UI_DrawText(
            &scrPlaceFull,
            line,
            256,
            profileFont,
            250.0f,
            y,
            rect->horzAlign,
            rect->vertAlign,
            0.36000001f,
            PROFLOAD_TEXT_COLOR,
            0);
    }
}

int ProfLoad_DrawTree()
{
    int result; // eax
    int v1; // [esp+24h] [ebp-124h]
    float textx; // [esp+2Ch] [ebp-11Ch]
    float texty; // [esp+30h] [ebp-118h]
    char rowText[256]; // [esp+38h] [ebp-110h] BYREF
    Font_s *profileFont; // [esp+13Ch] [ebp-Ch]
    int textIndex; // [esp+140h] [ebp-8h]
    int profileIndex; // [esp+144h] [ebp-4h]

    ProfLoad_CalculateSelfTicks();
    textIndex = 0;
    profileFont = UI_GetFontHandle(&scrPlaceFull, 0, 0.36000001f);
    result = mapLoadProfile.profileEntryCount - 16;
    if (mapLoadProfile.profileEntryCount - 16 > 0)
        v1 = mapLoadProfile.profileEntryCount - 16;
    else
        v1 = 0;
    for (profileIndex = v1; profileIndex < mapLoadProfile.profileEntryCount; ++profileIndex)
    {
        textx = (float)(5 * mapLoadProfile.profileEntries[profileIndex].indent + 60);
        texty = (float)(16 * textIndex + 70);
        ProfLoad_GetEntryRowText(&mapLoadProfile.profileEntries[profileIndex], rowText, 256);
        UI_DrawText(&scrPlaceFull, rowText, 256, profileFont, textx, texty, 0, 0, 0.36000001f, PROFLOAD_TEXT_COLOR, 0);
        ++textIndex;
        result = profileIndex + 1;
    }
    return result;
}

