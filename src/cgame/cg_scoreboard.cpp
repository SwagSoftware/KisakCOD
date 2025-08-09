#ifndef KISAK_SP
#error This file is for SinglePlayer only
#endif

#include "cg_scoreboard.h"
#include "cg_newdraw.h"
#include "cg_main.h"
#include <ui/ui.h>
#include <stringed/stringed_hooks.h>
#include "cg_servercmds.h"

float __cdecl CG_FadeObjectives(const cg_s *cgameGlob)
{
    double v2; // fp1
    float *v3; // r3

    if (!(unsigned __int8)CG_IsHudHidden())
    {
        if (cgameGlob->showScores)
        {
            v2 = 1.0;
            return *((float *)&v2 + 1);
        }
        v3 = CG_FadeColor(cgameGlob->time, cgameGlob->scoreFadeTime, 300, 300);
        if (v3)
        {
            v2 = v3[3];
            return *((float *)&v2 + 1);
        }
    }
    v2 = 0.0;
    return *((float *)&v2 + 1);
}

void __cdecl CG_DrawObjectiveBackdrop(const cg_s *cgameGlob, const float *color)
{
    int height; // [sp+50h] [-40h] BYREF
    int width; // [sp+54h] [-3Ch] BYREF
    float aspect[2]; // [sp+58h] [-38h] BYREF

    if (CG_FadeObjectives(cgameGlob) != 0.0)
    {
        CL_GetScreenDimensions(&width, &height, aspect);
        UI_FillRectPhysical(0.0, 0.0, width, height, color);
    }
}

void __cdecl CG_DrawObjectiveHeader(
    int localClientNum,
    const rectDef_s *rect,
    Font_s *font,
    double scale,
    float *color,
    int textStyle)
{
    double v11; // fp1
    char *v12; // r3
    int v13; // r8
    ScreenPlacement *v14; // r30
    int v15; // r7
    double v16; // fp8
    double v17; // fp7
    double v18; // fp6
    double v19; // fp5
    double v20; // fp4
    int v21; // r4
    double v22; // fp31
    int v23; // r4
    long double v24; // fp2
    double v25; // fp29
    long double v26; // fp2
    long double v27; // fp2
    __int64 v28; // r11
    double v29; // fp30
    int v30; // r4
    double v31; // fp1
    const float *v32; // r3
    float v33; // [sp+8h] [-B8h]
    float v34; // [sp+10h] [-B0h]
    float v35; // [sp+18h] [-A8h]
    float v36; // [sp+20h] [-A0h]
    float v37; // [sp+28h] [-98h]
    float v38; // [sp+30h] [-90h]
    float v39; // [sp+38h] [-88h]
    float v40; // [sp+40h] [-80h]
    float v41; // [sp+48h] [-78h]
    float v42; // [sp+50h] [-70h]
    float v43; // [sp+58h] [-68h]
    float v44; // [sp+60h] [-60h]
    float v45; // [sp+68h] [-58h]
    float v46; // [sp+70h] [-50h]

    if (localClientNum)
        MyAssertHandler(
            "c:\\trees\\cod3\\cod3src\\src\\cgame\\cg_local.h",
            910,
            0,
            "%s\n\t(localClientNum) = %i",
            "(localClientNum == 0)",
            localClientNum);
    v11 = CG_FadeObjectives(cgArray);
    if (v11 != 0.0)
    {
        *(float *)(textStyle + 12) = v11;
        v12 = UI_SafeTranslateString("CGAME_MISSIONOBJECTIVES");
        v13 = 68 * localClientNum;
        v14 = &scrPlaceView[localClientNum];
        UI_DrawText(
            v14,
            v12,
            0x7FFFFFFF,
            font,
            rect->x,
            rect->y,
            v15,
            v13,
            scale,
            (const float *)rect->horzAlign,
            rect->vertAlign);
        v22 = ScrPlace_ApplyX(v14, rect->x, v21);
        v25 = (float)(ScrPlace_ApplyX(v14, -rect->x, v23) - (float)v22);
        *(double *)&v24 = (float)(v14->scaleVirtualToReal[0] + (float)0.5);
        v26 = floor(v24);
        *(double *)&v26 = (float)((float)(v14->scaleVirtualToReal[1] * (float)2.0) + (float)0.5);
        v27 = floor(v26);
        LODWORD(v28) = (int)(float)*(double *)&v27;
        v29 = (float)v28;
        v31 = ScrPlace_ApplyY(v14, rect->y, v30);
        UI_FillRectPhysical(v22, (float)((float)v31 + (float)v29), v25, v29, v32);
    }
}

const char *__cdecl CG_WordWrap(
    const char *inputText,
    Font_s *font,
    double scale,
    int maxChars,
    int maxPixelWidth,
    int *charCount,
    int *a7)
{
    const char *v13; // r11
    const char *v14; // r24
    int v15; // r28
    int v16; // r25
    int v17; // r31
    const char *v18; // r26
    int v19; // r30
    const char *v21; // [sp+50h] [-70h] BYREF

    if (!inputText)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\cgame\\cg_scoreboard.cpp", 128, 0, "%s", "inputText != NULL");
    v13 = inputText;
    v14 = inputText;
    v15 = 0;
    v16 = 0;
    v17 = 0;
    v21 = inputText;
    if (!*inputText)
    {
    LABEL_16:
        if (a7)
            *a7 = v16;
        return 0;
    }
    while (1)
    {
        v18 = v13;
        v19 = SEH_ReadCharFromString(&v21, 0);
        if (v19 == 10)
            break;
        ++v17;
        if (font)
            v15 = UI_TextWidth(inputText, v17, font, scale);
        if (v19 <= 32)
            goto LABEL_14;
        if (v17 > maxPixelWidth || font && v15 > (int)charCount)
        {
            if (!v16)
            {
                v14 = v18;
                v16 = v17 - 1;
            }
            if (a7)
                *a7 = v16;
            return v14;
        }
        if (Language_IsAsian())
        {
        LABEL_14:
            v13 = v21;
        LABEL_15:
            v14 = v13;
            v16 = v17;
            if (!*v13)
                goto LABEL_16;
        }
        else
        {
            v13 = v21;
            if (!*v21)
                goto LABEL_15;
        }
    }
    if (a7)
        *a7 = v17;
    return v21;
}

void __cdecl CG_DrawObjectiveList(
    int localClientNum,
    const rectDef_s *rect,
    Font_s *font,
    double scale,
    float *color,
    int textStyle)
{
    const char *v9; // r29
    const char *v11; // r28
    double v12; // fp1
    __int64 v13; // r11
    const float *v14; // r6
    int v15; // r5
    int v16; // r4
    long double v17; // fp2
    double h; // fp0
    double v19; // fp28
    double v20; // fp25
    double y; // fp31
    double v22; // fp30
    double v23; // fp19
    Material *v24; // r7
    int *p_icon; // r24
    Material *checkbox_active; // r11
    const char *v27; // r30
    double v28; // fp29
    const dvar_s *v29; // r11
    int *integer; // r28
    int v31; // r8
    const char *v32; // r3
    double v33; // fp8
    double v34; // fp7
    double v35; // fp6
    double v36; // fp5
    double v37; // fp4
    const char *v38; // r29
    double v39; // fp30
    long double v40; // fp2
    double w; // fp0
    double x; // fp13
    __int64 v43; // r11
    double v44; // fp31
    int v45; // r4
    double v46; // fp1
    const float *v47; // r6
    int v48; // r5
    int v49; // r4
    float v50; // [sp+8h] [-1D8h]
    float v51; // [sp+10h] [-1D0h]
    float v52; // [sp+18h] [-1C8h]
    float v53; // [sp+20h] [-1C0h]
    float v54; // [sp+28h] [-1B8h]
    float v55; // [sp+30h] [-1B0h]
    float v56; // [sp+38h] [-1A8h]
    float v57; // [sp+40h] [-1A0h]
    float v58; // [sp+48h] [-198h]
    float v59; // [sp+50h] [-190h]
    float v60; // [sp+58h] [-188h]
    float v61; // [sp+60h] [-180h]
    float v62; // [sp+68h] [-178h]
    float v63; // [sp+70h] [-170h]
    __int64 v64; // [sp+80h] [-160h] BYREF
    const char *v65; // [sp+88h] [-158h]
    __int64 v66; // [sp+90h] [-150h]
    float v67; // [sp+A0h] [-140h]
    float v68; // [sp+A4h] [-13Ch]
    float v69; // [sp+A8h] [-138h]
    float v70; // [sp+ACh] [-134h]
    float v71; // [sp+B0h] [-130h]
    float v72; // [sp+B4h] [-12Ch]
    float v73; // [sp+B8h] [-128h]
    float v74; // [sp+BCh] [-124h]
    float v75; // [sp+C0h] [-120h]
    float v76; // [sp+C4h] [-11Ch]
    float v77; // [sp+C8h] [-118h]
    float v78; // [sp+CCh] [-114h]
    float v79; // [sp+D0h] [-110h]
    float v80; // [sp+D4h] [-10Ch]
    float v81; // [sp+D8h] [-108h]
    float v82; // [sp+DCh] [-104h]

    v9 = "%s\n\t(localClientNum) = %i";
    v11 = "(localClientNum == 0)";
    HIDWORD(v66) = (uintptr_t)"%s\n\t(localClientNum) = %i";
    v65 = "(localClientNum == 0)";
    if (localClientNum)
        MyAssertHandler(
            "c:\\trees\\cod3\\cod3src\\src\\cgame\\cg_local.h",
            910,
            0,
            "%s\n\t(localClientNum) = %i",
            "(localClientNum == 0)",
            localClientNum);
    v12 = CG_FadeObjectives(cgArray);
    if (v12 != 0.0)
    {
        v70 = v12;
        v78 = v12;
        v74 = v12;
        v82 = v12;
        v67 = 0.60000002;
        v68 = 0.60000002;
        v69 = 0.60000002;
        v75 = 1.0;
        v76 = 1.0;
        v77 = 1.0;
        v71 = 0.34999999;
        v72 = 0.34999999;
        v73 = 0.34999999;
        v79 = 0.40000001;
        v80 = 0.30000001;
        v81 = 0.30000001;
        LODWORD(v13) = UI_TextHeight(font, scale);
        h = rect->h;
        v64 = v13;
        v19 = (float)v13;
        if (v19 <= h)
            v20 = h;
        else
            v20 = (float)v13;
        y = rect->y;
        v22 = 0.0;
        v23 = (float)(rect->y - (float)v13);
        v24 = (Material *)0x82000000;
        p_icon = &cgArray[0].objectives[0].icon;
        do
        {
            switch (*(p_icon - 283))
            {
            case 1:
                if (*p_icon)
                    goto LABEL_11;
                checkbox_active = cgMedia.checkbox_active;
                goto LABEL_17;
            case 2:
                y = (float)((float)((float)y + (float)v20) + (float)4.0);
                break;
            case 3:
                checkbox_active = cgMedia.checkbox_done;
                goto LABEL_17;
            case 4:
                if (*p_icon)
                    LABEL_11 :
                    checkbox_active = CG_ObjectiveIcon(*p_icon, 0);
                else
                    checkbox_active = cgMedia.checkbox_current;
                goto LABEL_17;
            case 5:
                checkbox_active = cgMedia.checkbox_fail;
            LABEL_17:
                if (checkbox_active)
                {
                    //v22 = (float)((float)y - rect->h);
                    CL_DrawStretchPic(
                        &scrPlaceView[localClientNum],
                        rect->x,
                        y - rect->h,
                        rect->w,
                        rect->h,
                        rect->horzAlign,
                        rect->vertAlign,
                        0.0,
                        0.0,
                        1.0,
                        1.0,
                        colorWhite,// KISAKTODO: arg bad
                        v24);
                }
                v27 = SEH_LocalizeTextMessage((const char *)p_icon - 1032, "objective text", LOCMSG_SAFE);
                v28 = (float)((float)(rect->w + rect->x) + (float)12.0);
                if (localClientNum)
                    MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\cgame\\cg_local.h", 917, 0, v9, v11, localClientNum);
                if (cgsArray[0].viewAspect <= 1.3333334)
                    v29 = cg_objectiveListWrapCountStandard;
                else
                    v29 = cg_objectiveListWrapCountWidescreen;
                integer = (int *)v29->current.integer;
                do
                {
                    v32 = CG_WordWrap(v27, font, scale, v15, 0x7FFFFFFF, integer, (int *)&v64);
                    v15 = HIDWORD(v64);
                    v38 = v32;
                    if (HIDWORD(v64))
                    {
                        v22 = (float)((float)((float)((float)v19 - rect->h) * (float)0.5) + (float)y);
                        UI_DrawText(
                            &scrPlaceView[localClientNum],
                            v27,
                            SHIDWORD(v64),
                            font,
                            v28,
                            v22,
                            (int)v24,
                            v31,
                            scale,
                            (const float *)rect->horzAlign,
                            rect->vertAlign);
                        y = (float)((float)((float)y + (float)v20) + (float)4.0);
                    }
                    v27 = v38;
                } while (v38);
                v11 = v65;
                v9 = (const char *)HIDWORD(v66);
                break;
            default:
                break;
            }
            p_icon += 284;
        } while ((int)p_icon < (int)&cgArray[0].visionSetPreLoaded[0].filmLightTint[2]);
        if (v22 != 0.0)
        {
            v39 = (float)((float)v22 - (float)v23);
            if (v39 != 0.0)
            {
                *(double *)&v17 = (float)(scrPlaceView[localClientNum].scaleVirtualToReal[0] + (float)0.5);
                v40 = floor(v17);
                w = rect->w;
                x = rect->x;
                HIDWORD(v64) = (int)(float)*(double *)&v40;
                LODWORD(v43) = HIDWORD(v64);
                v66 = v43;
                v44 = (float)v43;
                v46 = ScrPlace_ApplyX(
                    &scrPlaceView[localClientNum],
                    (float)((float)((float)((float)((float)12.0 - (float)v43) * (float)0.5) + (float)w) + (float)x),
                    v45);
                UI_FillRect(&scrPlaceView[localClientNum], v46, v23, v44, v39, v49, v48, v47);
            }
        }
    }
}

void __cdecl CG_DrawPausedMenuLine(
    int localClientNum,
    const rectDef_s *rect,
    Font_s *font,
    double scale,
    float *color,
    int textStyle)
{
    double v11; // fp1
    __int64 v12; // r9
    double y; // fp12
    ScreenPlacement *v14; // r30
    double v15; // fp31
    long double v16; // fp2
    double v17; // fp30
    long double v18; // fp2
    __int64 v19; // r11
    double v20; // fp31
    double v21; // fp0
    double v22; // fp29
    int v23; // r4
    double v24; // fp1
    const float *v25; // r6
    int v26; // r5
    int v27; // r4
    __int64 v28; // [sp+50h] [-50h] BYREF

    if (localClientNum)
        MyAssertHandler(
            "c:\\trees\\cod3\\cod3src\\src\\cgame\\cg_local.h",
            910,
            0,
            "%s\n\t(localClientNum) = %i",
            "(localClientNum == 0)",
            localClientNum);
    v11 = CG_FadeObjectives(cgArray);
    if (v11 != 0.0)
    {
        *(float *)(textStyle + 12) = v11;
        LODWORD(v12) = UI_TextHeight(font, scale);
        y = rect->y;
        v28 = v12;
        v14 = &scrPlaceView[localClientNum];
        v15 = (float)v12;
        *(double *)&v16 = (float)(v14->scaleVirtualToReal[0] + (float)0.5);
        v17 = (float)((float)y - (float)v12);
        v18 = floor(v16);
        HIDWORD(v19) = &v28;
        v20 = (float)(rect->h * (float)v15);
        v21 = (float)*(double *)&v18;
        *(double *)&v18 = (float)(rect->x - (float)6.0);
        LODWORD(v19) = (int)v21;
        v28 = v19;
        v22 = (float)v19;
        v24 = ScrPlace_ApplyX(v14, *(double *)&v18, v23);
        UI_FillRect(v14, v24, v17, v22, v20, v27, v26, v25);
    }
}

int __cdecl CG_DrawScoreboard(int localClientNum)
{
    if (cg_paused->current.integer && cg_drawpaused->current.enabled)
        return 0;
    if (localClientNum)
        MyAssertHandler(
            "c:\\trees\\cod3\\cod3src\\src\\cgame\\cg_local.h",
            910,
            0,
            "%s\n\t(localClientNum) = %i",
            "(localClientNum == 0)",
            localClientNum);
    if (cgArray[0].predictedPlayerState.pm_type >= 5 || !cgArray[0].showScores)
        return 0;
    CG_HudMenuShowAllTimed(localClientNum);
    return 1;
}

void __cdecl CG_ParseObjectiveChange(int localClientNum, unsigned int num)
{
    const char *ConfigString; // r26
    float *v5; // r30
    int v6; // r31
    const char *v7; // r3
    int v8; // r7
    const char *v9; // r4
    int v10; // r29
    float *v11; // r31
    const char *v12; // r3
    int v13; // r31
    const char *v14; // r3
    int v15; // r3
    const char *v16; // r3
    char v17[16]; // [sp+50h] [-60h] BYREF

    if (localClientNum)
        MyAssertHandler(
            "c:\\trees\\cod3\\cod3src\\src\\cgame\\cg_local.h",
            910,
            0,
            "%s\n\t(localClientNum) = %i",
            "(localClientNum == 0)",
            localClientNum);
    ConfigString = CL_GetConfigString(localClientNum, num);
    if (num - 11 >= 0x10)
        MyAssertHandler(
            "c:\\trees\\cod3\\cod3src\\src\\cgame\\cg_scoreboard.cpp",
            420,
            0,
            "objectiveIndex doesn't index MAX_OBJECTIVES\n\t%i not in [0, %i)",
            num - 11,
            16);
    v5 = &cgArray[0].refdef.primaryLights[69].origin[284 * num + 1];
    if (!*ConfigString)
    {
        *v5 = 0.0;
    LABEL_17:
        v5[1] = 0.0;
        v5[4] = 0.0;
        v5[2] = 0.0;
        v5[3] = 0.0;
        v5[5] = 0.0;
        v5[6] = 0.0;
        v5[7] = 0.0;
        v5[8] = 0.0;
        v5[9] = 0.0;
        v5[10] = 0.0;
        v5[11] = 0.0;
        v5[12] = 0.0;
        v5[13] = 0.0;
        v5[14] = 0.0;
        v5[15] = 0.0;
        v5[16] = 0.0;
        v5[17] = 0.0;
        v5[18] = 0.0;
        v5[19] = 0.0;
        v5[20] = 0.0;
        v5[21] = 0.0;
        v5[22] = 0.0;
        v5[23] = 0.0;
        v5[24] = 0.0;
        *((_BYTE *)v5 + 100) = 0;
        v5[281] = NAN;
        v5[282] = 0.0;
        v5[283] = 0.0;
        return;
    }
    v6 = *(unsigned int *)v5;
    v7 = Info_ValueForKey(ConfigString, "state");
    if (*v7)
        *(unsigned int *)v5 = atol(v7);
    else
        *v5 = 0.0;
    v8 = *(unsigned int *)v5;
    if (*(int *)v5 < 0 || v8 > 5)
        MyAssertHandler(
            "c:\\trees\\cod3\\cod3src\\src\\cgame\\cg_scoreboard.cpp",
            439,
            0,
            "objectiveInfo->state not in [OBJST_EMPTY, OBJST_NUMSTATES - 1]\n\t%i not in [%i, %i]",
            v8,
            0,
            5);
    if (v6 != 4 && *(unsigned int *)v5 == 4)
        v5[281] = *(float *)&cgArray[0].time;
    if (!*(unsigned int *)v5)
        goto LABEL_17;
    v9 = Info_ValueForKey(ConfigString, "str");
    if (*v9)
        I_strncpyz((char *)v5 + 100, v9, 1024);
    else
        *((_BYTE *)v5 + 100) = 0;
    v10 = 0;
    v11 = v5 + 3;
    do
    {
        sprintf(v17, "org%d", v10);
        v12 = Info_ValueForKey(ConfigString, v17);
        *(v11 - 2) = 0.0;
        *(v11 - 1) = 0.0;
        *v11 = 0.0;
        if (*v12)
            sscanf(v12, "%f %f %f", v11 - 2, v11 - 1, v11);
        ++v10;
        v11 += 3;
    } while (v10 < 8);
    v13 = *((unsigned int *)v5 + 282);
    v14 = Info_ValueForKey(ConfigString, "ring");
    if (*v14)
        v15 = atol(v14);
    else
        v15 = 0;
    *((unsigned int *)v5 + 282) = v15;
    if (v13 != v15)
        v5[281] = *(float *)&cgArray[0].time;
    v16 = Info_ValueForKey(ConfigString, "icon");
    if (*v16)
        *((unsigned int *)v5 + 283) = atol(v16);
    else
        v5[283] = 0.0;
}

