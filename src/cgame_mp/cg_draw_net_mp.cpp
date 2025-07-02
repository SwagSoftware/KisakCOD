#ifndef KISAK_MP
#error This File is MultiPlayer Only
#endif

#include "cg_local_mp.h"
#include "cg_public_mp.h"

#include <client/client.h>

#include <ui/ui.h>
#include <server_mp/server_mp.h>

// struct lagometer_t lagometer 82829b60     cg_draw_net_mp.obj

unsigned __int8 *s_clientAnalysisData;
int s_sampleNum;
int s_entitySamples[10][18];

struct lagometer_t // sizeof=0x608
{                                       // ...
    int frameSamples[128];              // ...
    int frameCount;                     // ...
    int snapshotFlags[128];             // ...
    int snapshotSamples[128];           // ...
    int snapshotCount;                  // ...
};

lagometer_t lagometer;

void __cdecl CG_AddLagometerFrameInfo(const cg_s *cgameGlob)
{
    lagometer.frameSamples[lagometer.frameCount & 0x7F] = cgameGlob->time - cgameGlob->latestSnapshotTime;
    ++lagometer.frameCount;
}

void __cdecl CG_AddLagometerSnapshotInfo(snapshot_s *snap)
{
    if (snap)
    {
        lagometer.snapshotSamples[lagometer.snapshotCount & 0x7F] = snap->ping;
        lagometer.snapshotFlags[lagometer.snapshotCount & 0x7F] = snap->snapFlags;
    }
    else
    {
        lagometer.snapshotSamples[lagometer.snapshotCount & 0x7F] = -1;
    }
    ++lagometer.snapshotCount;
}

void __cdecl CG_DrawSnapshotAnalysis(int localClientNum)
{
    int v1; // eax
    float v2; // [esp+Ch] [ebp-D8h]
    double v3; // [esp+2Ch] [ebp-B8h]
    float v4; // [esp+34h] [ebp-B0h]
    float v5; // [esp+38h] [ebp-ACh]
    float v6; // [esp+40h] [ebp-A4h]
    float v7; // [esp+44h] [ebp-A0h]
    float v8; // [esp+50h] [ebp-94h]
    float v9; // [esp+54h] [ebp-90h]
    float v10; // [esp+58h] [ebp-8Ch]
    int client; // [esp+68h] [ebp-7Ch]
    int frame; // [esp+6Ch] [ebp-78h]
    int arrayFrame; // [esp+70h] [ebp-74h]
    int arrayFramea; // [esp+70h] [ebp-74h]
    int arrayFrameb; // [esp+70h] [ebp-74h]
    int bitsUsed; // [esp+74h] [ebp-70h]
    int field; // [esp+78h] [ebp-6Ch]
    int fielda; // [esp+78h] [ebp-6Ch]
    unsigned int fieldb; // [esp+78h] [ebp-6Ch]
    int fieldc; // [esp+78h] [ebp-6Ch]
    int sortedSamples[13]; // [esp+7Ch] [ebp-68h] BYREF
    const char *string; // [esp+B0h] [ebp-34h]
    cg_s *cgameGlob; // [esp+B4h] [ebp-30h]
    float height; // [esp+B8h] [ebp-2Ch]
    float width; // [esp+BCh] [ebp-28h]
    cgs_t *cgs; // [esp+C0h] [ebp-24h]
    int entity; // [esp+C4h] [ebp-20h]
    int column; // [esp+C8h] [ebp-1Ch]
    float x; // [esp+CCh] [ebp-18h]
    float y; // [esp+D0h] [ebp-14h]
    const float (*color)[4]; // [esp+D4h] [ebp-10h]
    int fieldCount; // [esp+D8h] [ebp-Ch]
    int v; // [esp+DCh] [ebp-8h]
    int eType; // [esp+E0h] [ebp-4h]

    fieldCount = 13;

    cgs = CG_GetLocalClientStaticGlobals(localClientNum);

    if (cgs->localServer)
    {
        if (net_showprofile->current.integer)
        {
            client = cg_packetAnalysisClient->current.integer;
            if (client >= 0)
            {
                x = -350.0f;
                y = 100.0f;
                width = 100.0f;
                height = 100.0f;
                cgameGlob = CG_GetLocalClientGlobals(localClientNum);
                if (SV_NewPacketAnalysisReady())
                {
                    arrayFrame = cgameGlob->packetAnalysisFrameCount % 100;
                    for (field = 0; field < 13; ++field)
                    {
                        if (g_bitsSent[client][field] <= 2000)
                        {
                            iassert(255 * g_bitsSent[client][field] / 2000 >= 0 && 255 * static_cast<float>(g_bitsSent[client][field]) / 2000 < 256);
                            cgameGlob->bitsSent[arrayFrame][field] = (int)((double)g_bitsSent[client][field] * 255.0 / 2000.0);
                        }
                        else
                        {
                            cgameGlob->bitsSent[arrayFrame][field] = -1;
                        }
                    }
                    arrayFramea = cgameGlob->packetAnalysisFrameCount % 10;
                    memset((unsigned __int8 *)cgameGlob->entBitsUsed[arrayFramea], 0, sizeof(cgameGlob->entBitsUsed[arrayFramea]));
                    memset((unsigned __int8 *)cgameGlob->numEntsSent[arrayFramea], 0, sizeof(cgameGlob->numEntsSent[arrayFramea]));
                    memset(
                        (unsigned __int8 *)cgameGlob->numEntFields[arrayFramea],
                        0,
                        sizeof(cgameGlob->numEntFields[arrayFramea]));
                    for (entity = 0; entity < 1024; ++entity)
                    {
                        bitsUsed = g_currentSnapshotPerEntity[client][entity];
                        if (bitsUsed)
                        {
                            eType = CG_GetEntity(localClientNum, entity)->nextState.eType;
                            if (eType > 17)
                                eType = 17;
                            cgameGlob->entBitsUsed[arrayFramea][eType] += bitsUsed;
                            ++cgameGlob->numEntsSent[arrayFramea][eType];
                            cgameGlob->numEntFields[arrayFramea][eType] += g_currentSnapshotFieldsPerEntity[client][entity];
                            if (eType == 1)
                                cgameGlob->numEntFields[arrayFramea][1] += g_currentSnapshotPlayerStateFields[client];
                        }
                    }
                    if (++cgameGlob->numSnapshots > 10)
                        cgameGlob->numSnapshots = 10;
                    SV_ClearPacketAnalysis();
                    ++cgameGlob->packetAnalysisFrameCount;
                }
                v10 = y - height;
                UI_DrawHandlePic(&scrPlaceView[localClientNum], x, v10, width, height, 3, 1, colorBlack, cgMedia.whiteMaterial);
                color = (const float (*)[4])colorWhite;
                if (cg_packetAnalysisClient->current.integer >= 0x40u)
                    MyAssertHandler(
                        ".\\cgame_mp\\cg_draw_net_mp.cpp",
                        260,
                        0,
                        "cg_packetAnalysisClient->current.integer doesn't index MAX_CLIENTS\n\t%i not in [0, %i)",
                        cg_packetAnalysisClient->current.integer,
                        64);
                v9 = y + 15.0f;
                v8 = x + 5.0f;
                UI_DrawText(
                    &scrPlaceView[localClientNum],
                    cgameGlob->bgs.clientinfo[cg_packetAnalysisClient->current.integer].name,
                    32,
                    cgMedia.smallDevFont,
                    v8,
                    v9,
                    3,
                    1,
                    0.40000001f,
                    (const float *)color,
                    3);
                for (frame = 0; frame < 100; ++frame)
                {
                    arrayFrameb = (cgameGlob->packetAnalysisFrameCount + frame) % 100;
                    s_clientAnalysisData = cgameGlob->bitsSent[arrayFrameb];
                    for (fielda = 0; fielda < 13; ++fielda)
                        sortedSamples[fielda] = fielda;
                    qsort(sortedSamples, 0xDu, 4u, (int(__cdecl *)(const void *, const void *))CG_ComparePacketAnalysisSamples);
                    for (column = 0; column < 13; ++column)
                    {
                        fieldb = sortedSamples[column];
                        if (fieldb > 0xC)
                            MyAssertHandler(
                                ".\\cgame_mp\\cg_draw_net_mp.cpp",
                                280,
                                0,
                                "%s\n\t(field) = %i",
                                "(field >= 0 && field < ANALYZE_SNAPSHOT_DATATYPE_COUNT)",
                                fieldb);
                        v = (int)((float)cgameGlob->bitsSent[arrayFrameb][fieldb] * height / 255.0f);
                        if (v)
                        {
                            switch (fieldb)
                            {
                            case 0u:
                                color = (const float (*)[4])colorRed;
                                goto LABEL_57;
                            case 1u:
                                color = (const float (*)[4])colorGreen;
                                goto LABEL_57;
                            case 2u:
                                color = (const float (*)[4])colorBlue;
                                goto LABEL_57;
                            case 3u:
                            case 5u:
                            case 9u:
                                continue;
                            case 4u:
                                color = (const float (*)[4])colorOrange;
                                goto LABEL_57;
                            case 6u:
                                color = (const float (*)[4])colorLtBlue;
                                goto LABEL_57;
                            case 7u:
                                color = (const float (*)[4])colorYellow;
                                goto LABEL_57;
                            case 8u:
                                color = (const float (*)[4])colorMagenta;
                                goto LABEL_57;
                            case 0xAu:
                                color = (const float (*)[4])colorLtCyan;
                                goto LABEL_57;
                            case 0xBu:
                                color = (const float (*)[4])colorLtOrange;
                                goto LABEL_57;
                            case 0xCu:
                                color = (const float (*)[4])colorLtGrey;
                                goto LABEL_57;
                            default:
                                color = (const float (*)[4])colorWhite;
                            LABEL_57:
                                v2 = (float)v;
                                v7 = y - (float)v;
                                v6 = x + width - (double)frame - 1.0f;
                                CL_DrawStretchPic(
                                    &scrPlaceView[localClientNum],
                                    v6,
                                    v7,
                                    1.0f,
                                    v2,
                                    3,
                                    1,
                                    0.0f,
                                    0.0f,
                                    0.0f,
                                    0.0f,
                                    (const float *)color,
                                    cgMedia.whiteMaterial);
                                break;
                            }
                        }
                    }
                }
                x = width + 10.0f + x;
                y = (float)cg_packetAnalysisTextY->current.integer;
                fieldc = 0;
                while (fieldc < 13)
                {
                    switch (fieldc)
                    {
                    case 0:
                        color = (const float (*)[4])colorRed;
                        string = "Entity update";
                        goto LABEL_76;
                    case 1:
                        color = (const float (*)[4])colorGreen;
                        string = "New entity";
                        goto LABEL_76;
                    case 2:
                        color = (const float (*)[4])colorBlue;
                        string = "Removed entity";
                        goto LABEL_76;
                    case 3:
                    case 5:
                    case 9:
                        goto LABEL_60;
                    case 4:
                        color = (const float (*)[4])colorOrange;
                        string = "Temp entity";
                        goto LABEL_76;
                    case 6:
                        color = (const float (*)[4])colorLtBlue;
                        string = "Client update";
                        goto LABEL_76;
                    case 7:
                        color = (const float (*)[4])colorYellow;
                        string = "New client";
                        goto LABEL_76;
                    case 8:
                        color = (const float (*)[4])colorMagenta;
                        string = "Removed client";
                        goto LABEL_76;
                    case 10:
                        color = (const float (*)[4])colorLtCyan;
                        string = "Playerstate update";
                        goto LABEL_76;
                    case 11:
                        color = (const float (*)[4])colorLtOrange;
                        string = "Full playerstate";
                        goto LABEL_76;
                    case 12:
                        color = (const float (*)[4])colorLtGrey;
                        string = "Server commands";
                        goto LABEL_76;
                    default:
                        if (!alwaysfails)
                            MyAssertHandler(
                                ".\\cgame_mp\\cg_draw_net_mp.cpp",
                                383,
                                0,
                                "Missing handler for snapshot data source type");
                        color = (const float (*)[4])colorWhite;
                        string = (char *)"";
                    LABEL_76:
                        v5 = y - 5.0;
                        UI_DrawHandlePic(
                            &scrPlaceView[localClientNum],
                            x,
                            v5,
                            3.0,
                            3.0,
                            3,
                            1,
                            (const float *)color,
                            cgMedia.whiteMaterial);
                        v4 = x + 5.0;
                        UI_DrawText(
                            &scrPlaceView[localClientNum],
                            string,
                            32,
                            cgMedia.smallDevFont,
                            v4,
                            y,
                            3,
                            1,
                            cg_packetAnalysisTextScale->current.value,
                            (const float *)color,
                            3);
                        v1 = UI_TextHeight(cgMedia.smallDevFont, cg_packetAnalysisTextScale->current.value);
                        y = (double)v1 + y;
                    LABEL_60:
                        ++fieldc;
                        break;
                    }
                }
            }
        }
    }
}

int __cdecl CG_ComparePacketAnalysisSamples(int *a, int *b)
{
    int field0; // [esp+0h] [ebp-8h]
    int field1; // [esp+4h] [ebp-4h]

    field0 = *a;
    field1 = *b;
    if (!s_clientAnalysisData)
        MyAssertHandler(".\\cgame_mp\\cg_draw_net_mp.cpp", 164, 0, "%s", "s_clientAnalysisData");
    return s_clientAnalysisData[field1] - s_clientAnalysisData[field0];
}

void __cdecl CG_DrawSnapshotEntityAnalysis(int localClientNum)
{
    const dvar_s *v2; // kr00_4
    const char *v3; // eax
    char *v4; // eax
    const char *v5; // eax
    Font_s *smallDevFont; // [esp+2Ch] [ebp-110h]
    float scale; // [esp+40h] [ebp-FCh]
    float v8; // [esp+50h] [ebp-ECh]
    float v9; // [esp+54h] [ebp-E8h]
    float w; // [esp+58h] [ebp-E4h]
    float v11; // [esp+60h] [ebp-DCh]
    float v12; // [esp+64h] [ebp-D8h]
    float v13; // [esp+74h] [ebp-C8h]
    float v14; // [esp+78h] [ebp-C4h]
    float v15; // [esp+80h] [ebp-BCh]
    float v16; // [esp+84h] [ebp-B8h]
    float v17; // [esp+88h] [ebp-B4h]
    float v18; // [esp+8Ch] [ebp-B0h]
    float graphx; // [esp+94h] [ebp-A8h]
    float lineHeight; // [esp+9Ch] [ebp-A0h]
    int field; // [esp+A0h] [ebp-9Ch]
    int fielda; // [esp+A0h] [ebp-9Ch]
    int entsSentWhenMax; // [esp+A4h] [ebp-98h]
    int maxEntsSent; // [esp+A8h] [ebp-94h]
    int totalBitsUsed; // [esp+ACh] [ebp-90h]
    const char *string; // [esp+B0h] [ebp-8Ch]
    int sortedSamples[18]; // [esp+B4h] [ebp-88h] BYREF
    cg_s *cgameGlob; // [esp+FCh] [ebp-40h]
    float graphy; // [esp+100h] [ebp-3Ch]
    float height; // [esp+104h] [ebp-38h]
    float width; // [esp+108h] [ebp-34h]
    int totalEntFields; // [esp+10Ch] [ebp-30h]
    int maxBitsUsed; // [esp+110h] [ebp-2Ch]
    cgs_t *cgs; // [esp+114h] [ebp-28h]
    int stat; // [esp+118h] [ebp-24h]
    float x; // [esp+11Ch] [ebp-20h]
    float y; // [esp+120h] [ebp-1Ch]
    const float (*color)[4]; // [esp+124h] [ebp-18h]
    int sample; // [esp+128h] [ebp-14h]
    int count; // [esp+12Ch] [ebp-10h]
    int eType; // [esp+130h] [ebp-Ch]
    int totalEntsSent; // [esp+134h] [ebp-8h]
    int maxEntFields; // [esp+138h] [ebp-4h]

    cgs = CG_GetLocalClientStaticGlobals(localClientNum);

    if (cgs->localServer)
    {
        cgameGlob = CG_GetLocalClientGlobals(localClientNum);
        if (net_showprofile->current.integer && cg_packetAnalysisClient->current.integer >= 0)
        {
            graphx = 10.0f;
            graphy = 100.0f;
            width = 80.0f;
            height = 100.0f;
            v17 = 100.0f - 100.0f;
            UI_DrawHandlePic(&scrPlaceView[localClientNum], 10.0f, v17, 80.0f, 100.0, 1, 1, colorBlack, cgMedia.whiteMaterial);
            color = (const float (*)[4])colorWhite;
            if (cg_packetAnalysisClient->current.integer >= 0x40u)
                MyAssertHandler(
                    ".\\cgame_mp\\cg_draw_net_mp.cpp",
                    457,
                    0,
                    "cg_packetAnalysisClient->current.integer doesn't index MAX_CLIENTS\n\t%i not in [0, %i)",
                    cg_packetAnalysisClient->current.integer,
                    64);
            v16 = graphy + 15.0f;
            v15 = graphx + 5.0f;
            UI_DrawText(
                &scrPlaceView[localClientNum],
                cgameGlob->bgs.clientinfo[cg_packetAnalysisClient->current.integer].name,
                32,
                cgMedia.smallDevFont,
                v15,
                v16,
                1,
                1,
                0.40000001f,
                (const float *)color,
                3);
            x = graphx + width + 10.0f;
            y = (float)cg_packetAnalysisEntTextY->current.integer;
            for (eType = 0; eType <= 17; ++eType)
            {
                switch (eType)
                {
                case 0:
                    string = "General";
                    color = (const float (*)[4])colorOrange;
                    break;
                case 1:
                    string = "Players";
                    color = (const float (*)[4])colorRed;
                    break;
                case 2:
                    string = "Corpses";
                    color = (const float (*)[4])colorGreen;
                    break;
                case 3:
                    string = "Items";
                    color = (const float (*)[4])colorBlue;
                    break;
                case 4:
                    string = "Missiles";
                    color = (const float (*)[4])colorLtGreen;
                    break;
                case 5:
                    string = "Invisible Ents";
                    color = (const float (*)[4])colorLtBlue;
                    break;
                case 6:
                    string = "Scriptmover";
                    color = (const float (*)[4])colorMagenta;
                    break;
                case 7:
                    string = "Sound Blend";
                    color = (const float (*)[4])colorLtOrange;
                    break;
                case 8:
                    string = "FX";
                    color = (const float (*)[4])colorLtGrey;
                    break;
                case 9:
                    string = "Loop FX";
                    color = (const float (*)[4])colorLtCyan;
                    break;
                case 10:
                    string = "Primary Light";
                    color = (const float (*)[4])colorLtBlue;
                    break;
                case 11:
                    string = "MG42";
                    color = (const float (*)[4])colorMdYellow;
                    break;
                case 12:
                    string = "Helicopter";
                    color = (const float (*)[4])colorLtBlue;
                    break;
                case 13:
                    string = "Plane";
                    color = (const float (*)[4])colorLtOrange;
                    break;
                case 14:
                    string = "Vehicle";
                    color = (const float (*)[4])colorLtOrange;
                    break;
                case 15:
                    string = "Vehicle Collmap";
                    color = (const float (*)[4])colorLtBlue;
                    break;
                case 16:
                    string = "Vehicle Corpse";
                    color = (const float (*)[4])colorLtBlue;
                    break;
                case 17:
                    string = "Events";
                    color = (const float (*)[4])colorYellow;
                    break;
                default:
                    if (!alwaysfails)
                    {
                        v3 = va("Missing handler for snapshot entity type %i", eType);
                        MyAssertHandler(".\\cgame_mp\\cg_draw_net_mp.cpp", 539, 0, v3);
                    }
                    string = "";
                    break;
                }
                totalBitsUsed = 0;
                totalEntsSent = 0;
                totalEntFields = 0;
                maxBitsUsed = 0;
                entsSentWhenMax = 0;
                maxEntsSent = 0;
                maxEntFields = 0;
                for (sample = 0; sample < 10; ++sample)
                {
                    s_entitySamples[(cgameGlob->packetAnalysisFrameCount - sample - 1) % 10][eType] = cgameGlob->entBitsUsed[sample][eType];
                    totalBitsUsed += cgameGlob->entBitsUsed[sample][eType];
                    totalEntsSent += cgameGlob->numEntsSent[sample][eType];
                    totalEntFields += cgameGlob->numEntFields[sample][eType];
                    if (cgameGlob->entBitsUsed[sample][eType] > maxBitsUsed)
                    {
                        maxBitsUsed = cgameGlob->entBitsUsed[sample][eType];
                        entsSentWhenMax = cgameGlob->numEntsSent[sample][eType];
                    }
                    if (cgameGlob->numEntsSent[sample][eType] > maxEntsSent)
                        maxEntsSent = cgameGlob->numEntsSent[sample][eType];
                    if (cgameGlob->numEntFields[sample][eType] > maxEntFields)
                    {
                        maxEntFields = cgameGlob->numEntFields[sample][eType];
                    }
                }
                if (totalEntsSent)
                    count = totalEntsSent;
                else
                    count = 1;
                if (entsSentWhenMax)
                {
                    v14 = y - 8.0;
                    UI_DrawHandlePic(
                        &scrPlaceView[localClientNum],
                        x,
                        v14,
                        3.0,
                        3.0,
                        3,
                        1,
                        (const float *)color,
                        cgMedia.whiteMaterial);
                    scale = cg_packetAnalysisEntTextScale->current.value;
                    v13 = x + 5.0;
                    smallDevFont = cgMedia.smallDevFont;
                    v4 = va(
                        "%s: %.1fx%.1fb (%.1f fields), max %ix%.1fb (%i f)",
                        string,
                        (float)totalEntsSent * 1.0f / 10.0f,
                        (float)totalBitsUsed * 1.0f / (float)(8 * count),
                        (float)totalEntFields * 1.0f / (float)count,
                        maxEntsSent,
                        (float)maxBitsUsed * 1.0f / (float)(8 * entsSentWhenMax),
                        maxEntFields);
                    UI_DrawText(&scrPlaceView[localClientNum], v4, 80, smallDevFont, v13, y, 1, 1, scale, (const float *)color, 3);
                    y = (float)UI_TextHeight(cgMedia.smallDevFont, cg_packetAnalysisEntTextScale->current.value) + y;
                }
            }
            for (sample = 0; sample < 10; ++sample)
            {
                s_sampleNum = sample;
                for (field = 0; field <= 17; ++field)
                    sortedSamples[field] = field;
                qsort(sortedSamples, 0x12u, 4u, (int(__cdecl *)(const void *, const void *))CG_CompareEntityAnalysisSamples);
                for (fielda = 0; fielda <= 17; ++fielda)
                {
                    eType = sortedSamples[fielda];
                    stat = s_entitySamples[sample][eType];
                    v18 = (float)stat * 1.0f / 1600.0f;
                    v12 = v18 - 1.0f;
                    if (v12 < 0.0f)
                        v11 = (float)stat * 1.0f / 1600.0f;
                    else
                        v11 = 1.0f;
                    if (v11 > 0.0f)
                    {
                        switch (eType)
                        {
                        case 0:
                            color = (const float (*)[4])colorOrange;
                            break;
                        case 1:
                            color = (const float (*)[4])colorRed;
                            break;
                        case 2:
                            color = (const float (*)[4])colorGreen;
                            break;
                        case 3:
                            color = (const float (*)[4])colorBlue;
                            break;
                        case 4:
                            color = (const float (*)[4])colorLtGreen;
                            break;
                        case 5:
                            color = (const float (*)[4])colorLtBlue;
                            break;
                        case 6:
                            color = (const float (*)[4])colorMagenta;
                            break;
                        case 7:
                            color = (const float (*)[4])colorLtOrange;
                            break;
                        case 8:
                            color = (const float (*)[4])colorLtGrey;
                            break;
                        case 9:
                            color = (const float (*)[4])colorLtCyan;
                            break;
                        case 10:
                            color = (const float (*)[4])colorLtOrange;
                            break;
                        case 11:
                            color = (const float (*)[4])colorMdYellow;
                            break;
                        case 12:
                            color = (const float (*)[4])colorLtBlue;
                            break;
                        case 13:
                            color = (const float (*)[4])colorLtOrange;
                            break;
                        case 14:
                            color = (const float (*)[4])colorLtOrange;
                            break;
                        case 15:
                            color = (const float (*)[4])colorLtBlue;
                            break;
                        case 17:
                            color = (const float (*)[4])colorYellow;
                            break;
                        default:
                            if (!alwaysfails)
                            {
                                v5 = va("Missing handler for snapshot entity type %i", eType);
                                MyAssertHandler(".\\cgame_mp\\cg_draw_net_mp.cpp", 653, 0, v5);
                            }
                            break;
                        }
                        lineHeight = v11 * height;
                        w = width / 10.0f;
                        v9 = graphy - lineHeight;
                        v8 = (double)sample * width / 10.0f + graphx;
                        UI_DrawHandlePic(
                            &scrPlaceView[localClientNum],
                            v8,
                            v9,
                            w,
                            lineHeight,
                            1,
                            1,
                            (const float *)color,
                            cgMedia.whiteMaterial);
                    }
                }
            }
        }
    }
}

int __cdecl CG_CompareEntityAnalysisSamples(unsigned int *a, unsigned int *b)
{
    return s_entitySamples[s_sampleNum][*b] - s_entitySamples[s_sampleNum][*a];
}

void __cdecl CG_DrawPingAnalysis(int localClientNum)
{
    float v1; // [esp+30h] [ebp-60h]
    float v2; // [esp+38h] [ebp-58h]
    float v3; // [esp+3Ch] [ebp-54h]
    float v4; // [esp+4Ch] [ebp-44h]
    float v5; // [esp+50h] [ebp-40h]
    float v6; // [esp+54h] [ebp-3Ch]
    float v7; // [esp+58h] [ebp-38h]
    int client; // [esp+5Ch] [ebp-34h]
    int frame; // [esp+60h] [ebp-30h]
    int ping; // [esp+64h] [ebp-2Ch]
    float height; // [esp+6Ch] [ebp-24h]
    float lineColor[6]; // [esp+70h] [ebp-20h] BYREF
    float x; // [esp+88h] [ebp-8h]
    float y; // [esp+8Ch] [ebp-4h]
    cgs_t *cgs;

    cgs = CG_GetLocalClientStaticGlobals(localClientNum);

    if (cgs->localServer)
    {
        if (net_showprofile->current.integer)
        {
            client = cg_packetAnalysisClient->current.integer;
            if (client >= 0)
            {
                x = -150.0f;
                y = 100.0f;
                height = 10.0f;
                lineColor[0] = colorWhite[0];
                lineColor[1] = colorWhite[1];
                lineColor[2] = colorWhite[2];
                lineColor[3] = colorWhite[3];
                //LODWORD(lineColor[4]) = colorWhite;
                v5 = 100.0f + 15.0f;
                v4 = -150.0f + 5.0f;
                UI_DrawText(
                    &scrPlaceView[localClientNum],
                    "Ping",
                    32,
                    cgMedia.smallDevFont,
                    v4,
                    v5,
                    3,
                    1,
                    0.40000001f,
                    colorWhite,
                    3);
                for (frame = 31; frame >= 0; --frame)
                {
                    ping = SV_GetClientSnapshotPing(client, frame);
                    if (ping >= 0)
                    {
                        if (ping >= 100)
                        {
                            if (ping >= 200)
                            {
                                v7 = 1.0f - (ping - 200) * 1.0f / 200.0f * 0.800000011920929f;
                                v3 = 0.2f - v7;
                                if (v3 < 0.0f)
                                    v6 = 1.0f - (ping - 200) * 1.0f / 200.0f * 0.800000011920929f;
                                else
                                    v6 = 0.2f;
                                lineColor[0] = v6;
                                lineColor[1] = 0.0f;
                                lineColor[2] = 0.0f;
                                lineColor[3] = 1.0f;
                            }
                            else
                            {
                                lineColor[0] = 1.0f - (ping - 100) * 1.0f / 100.0f * 0.800000011920929f;
                                lineColor[1] = lineColor[0];
                                lineColor[2] = 0.0f;
                                lineColor[3] = 1.0f;
                            }
                        }
                        else
                        {
                            lineColor[0] = 0.0f;
                            lineColor[1] = 1.0f - ping * 1.0f / 100.0f * 0.800000011920929f;
                            lineColor[2] = 0.0f;
                            lineColor[3] = 1.0f;
                        }
                    }
                    else
                    {
                        lineColor[0] = 0.0f;
                        lineColor[1] = 0.0f;
                        lineColor[2] = 0.0f;
                        lineColor[3] = 1.0f;
                    }
                    v2 = y - height;
                    v1 = (3 * frame) + x;
                    CL_DrawStretchPic(
                        &scrPlaceView[localClientNum],
                        v1,
                        v2,
                        3.0f,
                        height,
                        3,
                        1,
                        0.0,
                        0.0,
                        0.0,
                        0.0,
                        lineColor,
                        cgMedia.whiteMaterial);
                }
            }
        }
    }
}

void __cdecl CG_DrawLagometer(int localClientNum)
{
    float v1; // [esp+30h] [ebp-64h]
    float v2; // [esp+34h] [ebp-60h]
    float v3; // [esp+38h] [ebp-5Ch]
    float v4; // [esp+3Ch] [ebp-58h]
    float v5; // [esp+40h] [ebp-54h]
    float v6; // [esp+44h] [ebp-50h]
    float v7; // [esp+48h] [ebp-4Ch]
    float v8; // [esp+4Ch] [ebp-48h]
    float v9; // [esp+50h] [ebp-44h]
    const float *hcolor; // [esp+54h] [ebp-40h]
    ScreenPlacement *scrPlace; // [esp+58h] [ebp-3Ch]
    float range; // [esp+5Ch] [ebp-38h]
    float rangea; // [esp+5Ch] [ebp-38h]
    float vscale; // [esp+60h] [ebp-34h]
    float vscalea; // [esp+60h] [ebp-34h]
    float v16; // [esp+64h] [ebp-30h]
    float v17; // [esp+6Ch] [ebp-28h]
    float ay; // [esp+74h] [ebp-20h]
    int a; // [esp+78h] [ebp-1Ch]
    int aa; // [esp+78h] [ebp-1Ch]
    float aw; // [esp+84h] [ebp-10h]
    float vd; // [esp+8Ch] [ebp-8h]
    float v; // [esp+8Ch] [ebp-8h]
    float va; // [esp+8Ch] [ebp-8h]
    float vb; // [esp+8Ch] [ebp-8h]
    float vc; // [esp+8Ch] [ebp-8h]
    float mid; // [esp+90h] [ebp-4h]
    cgs_t *cgs;

    cgs = CG_GetLocalClientStaticGlobals(localClientNum);

    if (cg_drawLagometer->current.enabled && !cgs->localServer)
    {
        scrPlace = &scrPlaceView[localClientNum];
        UI_DrawHandlePic(scrPlace, -55.0f, -140.0f, 48.0f, 48.0f, 3, 3, 0, cgMedia.lagometerMaterial);
        v17 = -55.0f;
        ay = -140.0f;
        aw = 48.0f;
        v16 = 48.0f;
        range = (float)48.0f / 3.0f;
        mid = (float)-140.0f + range;
        for (a = 0; aw > (float)a; ++a)
        {
            vd = (float)lagometer.frameSamples[(LOBYTE(lagometer.frameCount) - 1 - (_BYTE)a) & 0x7F];
            vscale = range / 300.0f;
            v = vd * vscale;
            if (v <= 0.0f)
            {
                if (v < 0.0f)
                {
                    va = -v;
                    if (range < (float)va)
                        va = 48.0f / 3.0f;
                    v7 = v17 + aw - (float)a;
                    CL_DrawStretchPic(scrPlace, v7, mid, 1.0f, va, 3, 3, 0.0f, 0.0f, 0.0f, 0.0f, colorBlue, cgMedia.whiteMaterial);
                }
            }
            else
            {
                if (range < v)
                    v = 48.0f / 3.0f;
                v9 = mid - v;
                v8 = v17 + aw - (float)a;
                CL_DrawStretchPic(scrPlace, v8, v9, 1.0f, v, 3, 3, 0.0f, 0.0f, 0.0f, 0.0f, colorYellow, cgMedia.whiteMaterial);
            }
        }
        rangea = v16 / 2.0f;
        for (aa = 0; aw > (float)aa; ++aa)
        {
            vb = (float)lagometer.snapshotSamples[(LOBYTE(lagometer.snapshotCount) - 1 - (_BYTE)aa) & 0x7F];
            if (vb <= 0.0f)
            {
                if (vb < 0.0f)
                {
                    v4 = ay + v16 - rangea;
                    v3 = v17 + aw - (double)aa;
                    CL_DrawStretchPic(scrPlace, v3, v4, 1.0f, rangea, 3, 3, 0.0f, 0.0f, 0.0f, 0.0f, colorRed, cgMedia.whiteMaterial);
                }
            }
            else
            {
                if ((lagometer.snapshotFlags[(LOBYTE(lagometer.snapshotCount) - 1 - (_BYTE)aa) & 0x7F] & 1) != 0)
                    hcolor = colorYellow;
                else
                    hcolor = colorGreen;
                vscalea = rangea / 900.0f;
                vc = vb * vscalea;
                if (rangea < (float)vc)
                    vc = v16 / 2.0f;
                v6 = ay + v16 - vc;
                v5 = v17 + aw - (float)aa;
                CL_DrawStretchPic(scrPlace, v5, v6, 1.0f, vc, 3, 3, 0.0f, 0.0f, 0.0f, 0.0f, hcolor, cgMedia.whiteMaterial);
            }
        }
        if (cg_nopredict->current.enabled || cg_synchronousClients->current.enabled)
        {
            v2 = ay + 480.0f;
            v1 = v17 + 640.0f + aw;
            CG_DrawBigDevString(scrPlace, v1, v2, (char*)"snc", 1.0f, 10);
        }
    }
    CG_DrawDisconnect(localClientNum);
}

bool __cdecl CL_IsServerRestarting(int localClientNum)
{
    return CL_GetLocalClientConnection(localClientNum)->isServerRestarting;

}
void __cdecl CG_DrawDisconnect(int localClientNum)
{
    Material *disconnectMaterial; // [esp+24h] [ebp-48h]
    Font_s *font; // [esp+28h] [ebp-44h]
    ScreenPlacement *scrPlace; // [esp+2Ch] [ebp-40h]
    int cmdNum; // [esp+34h] [ebp-38h]
    char *s; // [esp+38h] [ebp-34h]
    float x; // [esp+3Ch] [ebp-30h]
    usercmd_s cmd; // [esp+44h] [ebp-28h] BYREF
    int w; // [esp+68h] [ebp-4h]
    cg_s *cgameGlob;

    if (!CL_IsServerRestarting(localClientNum))
    {
        cgameGlob = CG_GetLocalClientGlobals(localClientNum);
        cmdNum = CL_GetCurrentCmdNumber(localClientNum) - 127;
        CL_GetUserCmd(localClientNum, cmdNum, &cmd);
        if (cmd.serverTime > cgameGlob->nextSnap->ps.commandTime && cmd.serverTime <= cgameGlob->time)
        {
            scrPlace = &scrPlaceView[localClientNum];
            s = UI_SafeTranslateString("CGAME_CONNECTIONINTERUPTED");
            font = UI_GetFontHandle(scrPlace, 0, 0.5);
            w = UI_TextWidth(s, 0, font, 0.5);
            x = (float)((640 - w) / 2);
            UI_DrawText(scrPlace, s, 0x7FFFFFFF, font, x, 100.0, 0, 0, 0.5, colorWhite, 3);
            if (((cgameGlob->time >> 9) & 1) == 0)
            {
                disconnectMaterial = Material_RegisterHandle("net_disconnect", 7);
                UI_DrawHandlePic(scrPlace, 296.0, 320.0, 48.0, 48.0, 0, 0, 0, disconnectMaterial);
            }
        }
    }
}