#include "cg_local.h"
#include "cg_public.h"
#include <database/database.h>
#include <client_mp/client_mp.h>
#include <devgui/devgui.h>
#include <cgame_mp/cg_local_mp.h>
#include <universal/q_parse.h>

const dvar_t *nightVisionFadeInOutTime;
const dvar_t *nightVisionPowerOnTime;
const dvar_t *nightVisionDisableEffects;

const char *MYDEFAULTVISIONNAME = "default";

struct visField_t // sizeof=0xC
{                                       // ...
    const char *name;                   // ...
    int offset;
    int fieldType;
};
visField_t visionDefFields[16] =
{
  { "r_glow", 0, 0 },
  { "r_glowBloomCutoff", 4, 1 },
  { "r_glowBloomDesaturation", 8, 1 },
  { "r_glowBloomIntensity0", 12, 1 },
  { "r_glowBloomIntensity1", 16, 1 },
  { "r_glowRadius0", 20, 1 },
  { "r_glowRadius1", 24, 1 },
  { "r_glowSkyBleedIntensity0", 28, 1 },
  { "r_glowSkyBleedIntensity1", 32, 1 },
  { "r_filmEnable", 36, 0 },
  { "r_filmBrightness", 40, 1 },
  { "r_filmContrast", 44, 1 },
  { "r_filmDesaturation", 48, 1 },
  { "r_filmInvert", 52, 0 },
  { "r_filmLightTint", 56, 2 },
  { "r_filmDarkTint", 68, 2 }
}; // idb

void __cdecl CG_RegisterVisionSetsDvars()
{
    DvarLimits min; // [esp+4h] [ebp-10h]
    DvarLimits mina; // [esp+4h] [ebp-10h]

    min.value.max = 10000.0;
    min.value.min = 0.0;
    nightVisionFadeInOutTime = Dvar_RegisterFloat(
        "nightVisionFadeInOutTime",
        0.1,
        min,
        0x1000u,
        "How long the fade to/from black lasts when putting on or removing night vision goggles.");
    mina.value.max = 10000.0;
    mina.value.min = 0.0;
    nightVisionPowerOnTime = Dvar_RegisterFloat(
        "nightVisionPowerOnTime",
        0.30000001,
        mina,
        0x1000u,
        "How long the black-to-nightvision fade lasts when turning on the goggles.");
    nightVisionDisableEffects = Dvar_RegisterBool("nightVisionDisableEffects", 0, 0x1000u, "");
}

void __cdecl CG_InitVisionSetsMenu()
{
    DB_EnumXAssets(ASSET_TYPE_RAWFILE, (void(__cdecl *)(XAssetHeader, void *))CG_AddVisionSetMenuItem, 0, 0);
}

void __cdecl CG_AddVisionSetMenuItem(XAssetHeader header)
{
    char devguiPath[260]; // [esp+0h] [ebp-310h] BYREF
    const char *visionSetNameEnd; // [esp+104h] [ebp-20Ch]
    char visionSetName[256]; // [esp+108h] [ebp-208h] BYREF
    char command[256]; // [esp+208h] [ebp-108h] BYREF
    const char *visionSetNameBegin; // [esp+30Ch] [ebp-4h]

    if (!header.xmodelPieces)
        MyAssertHandler(".\\cgame\\cg_visionsets.cpp", 38, 0, "%s", "header.rawfile");
    if (!header.xmodelPieces->name)
        MyAssertHandler(".\\cgame\\cg_visionsets.cpp", 39, 0, "%s", "header.rawfile->name");
    visionSetNameEnd = I_stristr(header.xmodelPieces->name, ".vision");
    if (visionSetNameEnd)
    {
        visionSetNameBegin = I_stristr(header.xmodelPieces->name, "/");
        if (!visionSetNameBegin)
            MyAssertHandler(".\\cgame\\cg_visionsets.cpp", 47, 1, "%s", "visionSetNameBegin");
        ++visionSetNameBegin;
        if (visionSetNameEnd - visionSetNameBegin >= 256)
            MyAssertHandler(
                ".\\cgame\\cg_visionsets.cpp",
                51,
                1,
                "%s",
                "visionSetNameEnd - visionSetNameBegin < static_cast<int>( sizeof( visionSetName ) )");
        strncpy(
            (unsigned __int8 *)visionSetName,
            (unsigned __int8 *)visionSetNameBegin,
            visionSetNameEnd - visionSetNameBegin);
        visionSetName[visionSetNameEnd - visionSetNameBegin] = 0;
        _snprintf(devguiPath, 0x100u, "Renderer/Vision Sets/%s", visionSetName);
        _snprintf(command, 0x100u, "VisionSetNaked %s", visionSetName);
        DevGui_AddCommand(devguiPath, command);
    }
}

void __cdecl CG_VisionSetsUpdate(int localClientNum)
{
    int idx; // [esp+4h] [ebp-4h]

    if (localClientNum)
        MyAssertHandler(
            "c:\\trees\\cod3\\src\\cgame\\../cgame_mp/cg_local_mp.h",
            1071,
            0,
            "%s\n\t(localClientNum) = %i",
            "(localClientNum == 0)",
            localClientNum);
    if (!cgArray[0].visionNameNaked[0])
        SetDefaultVision(localClientNum);
    for (idx = 0; idx < 2; ++idx)
        UpdateVarsLerp(
            cgArray[0].time,
            &cgArray[0].visionSetFrom[idx],
            &cgArray[0].visionSetTo[idx],
            &cgArray[0].visionSetLerpData[idx],
            &cgArray[0].visionSetCurrent[idx]);
}

void __cdecl UpdateVarsLerp(
    int time,
    const visionSetVars_t *from,
    const visionSetVars_t *to,
    visionSetLerpData_t *lerpData,
    visionSetVars_t *result)
{
    int fieldType; // [esp+18h] [ebp-34h]
    float v6; // [esp+1Ch] [ebp-30h]
    float v7; // [esp+20h] [ebp-2Ch]
    float v8; // [esp+24h] [ebp-28h]
    visionSetLerpStyle_t style; // [esp+2Ch] [ebp-20h]
    float v10; // [esp+30h] [ebp-1Ch]
    float *voidFrom; // [esp+38h] [ebp-14h]
    float *voidTo; // [esp+3Ch] [ebp-10h]
    float *voidResult; // [esp+40h] [ebp-Ch]
    float fraction; // [esp+44h] [ebp-8h]
    int fieldNum; // [esp+48h] [ebp-4h]

    if (!from)
        MyAssertHandler(".\\cgame\\cg_visionsets.cpp", 376, 0, "%s", "from");
    if (!to)
        MyAssertHandler(".\\cgame\\cg_visionsets.cpp", 377, 0, "%s", "to");
    if (!lerpData)
        MyAssertHandler(".\\cgame\\cg_visionsets.cpp", 378, 0, "%s", "lerpData");
    if (!result)
        MyAssertHandler(".\\cgame\\cg_visionsets.cpp", 379, 0, "%s", "result");
    if (lerpData->style >= (unsigned int)VISIONSETLERP_TO_LINEAR)
    {
        if (lerpData->timeDuration + lerpData->timeStart >= time)
        {
            fraction = (double)(time - lerpData->timeStart) / (double)lerpData->timeDuration;
            v8 = fraction - 1.0;
            if (v8 < 0.0)
                v10 = (double)(time - lerpData->timeStart) / (double)lerpData->timeDuration;
            else
                v10 = 1.0;
            v7 = 0.0 - fraction;
            if (v7 < 0.0)
                v6 = v10;
            else
                v6 = 0.0;
            for (fieldNum = 0; fieldNum < 16; ++fieldNum)
            {
                voidFrom = (float *)(&from->glowEnable + visionDefFields[fieldNum].offset);
                voidTo = (float *)(&to->glowEnable + visionDefFields[fieldNum].offset);
                voidResult = (float *)(&result->glowEnable + visionDefFields[fieldNum].offset);
                fieldType = visionDefFields[fieldNum].fieldType;
                if (fieldType)
                {
                    if (fieldType == 1)
                    {
                        *voidResult = LerpFloat(*voidFrom, *voidTo, v6, lerpData->style);
                    }
                    else
                    {
                        if (visionDefFields[fieldNum].fieldType != 2)
                            MyAssertHandler(".\\cgame\\cg_visionsets.cpp", 442, 0, "%s", "fieldDef->fieldType == DTYPE_VEC3");
                        LerpVec3(voidFrom, voidTo, v6, lerpData->style, voidResult);
                    }
                }
                else
                {
                    *(_BYTE *)voidResult = LerpBool(*(_BYTE *)voidFrom, *(_BYTE *)voidTo, v6, lerpData->style);
                }
            }
        }
        else
        {
            style = lerpData->style;
            if (style < VISIONSETLERP_TO_LINEAR || style > VISIONSETLERP_TO_SMOOTH)
            {
                if (lerpData->style != VISIONSETLERP_BACKFORTH_LINEAR && lerpData->style != VISIONSETLERP_BACKFORTH_SMOOTH)
                    MyAssertHandler(
                        ".\\cgame\\cg_visionsets.cpp",
                        405,
                        0,
                        "%s",
                        "(lerpData->style == VISIONSETLERP_BACKFORTH_LINEAR) || (lerpData->style == VISIONSETLERP_BACKFORTH_SMOOTH)");
                memcpy(result, from, sizeof(visionSetVars_t));
            }
            else
            {
                memcpy(result, to, sizeof(visionSetVars_t));
            }
            lerpData->style = VISIONSETLERP_NONE;
        }
    }
}

bool __cdecl LerpBool(bool from, bool to, float fraction, visionSetLerpStyle_t style)
{
    if (style == VISIONSETLERP_NONE)
        MyAssertHandler(".\\cgame\\cg_visionsets.cpp", 303, 0, "%s", "style != VISIONSETLERP_NONE");
    if (style < VISIONSETLERP_BACKFORTH_LINEAR || style > VISIONSETLERP_BACKFORTH_SMOOTH || fraction < 0.5)
        return to;
    else
        return from;
}

double __cdecl LerpFloat(float from, float to, float fraction, visionSetLerpStyle_t style)
{
    double v5; // st7
    float v8; // [esp+8h] [ebp-18h]
    float v9; // [esp+Ch] [ebp-14h]
    float v11; // [esp+14h] [ebp-Ch]
    float v12; // [esp+18h] [ebp-8h]

    if (style == VISIONSETLERP_NONE)
        MyAssertHandler(".\\cgame\\cg_visionsets.cpp", 322, 0, "%s", "style != VISIONSETLERP_NONE");
    switch (style)
    {
    case VISIONSETLERP_TO_LINEAR:
        return (float)((to - from) * fraction + from);
    case VISIONSETLERP_TO_SMOOTH:
        v12 = fraction * 3.141592741012573 * 0.5;
        v11 = sin(v12);
        fraction = v11;
        return (float)((to - from) * fraction + from);
    case VISIONSETLERP_BACKFORTH_SMOOTH:
        v9 = fraction * 3.141592741012573 * 0.5;
        v8 = sin(v9);
        fraction = v8;
        break;
    }
    if (style != VISIONSETLERP_BACKFORTH_SMOOTH && style != VISIONSETLERP_BACKFORTH_LINEAR)
        MyAssertHandler(
            ".\\cgame\\cg_visionsets.cpp",
            336,
            0,
            "%s",
            "(style == VISIONSETLERP_BACKFORTH_SMOOTH) || (style == VISIONSETLERP_BACKFORTH_LINEAR)");
    if (fraction >= 0.5)
    {
        return (float)((from - to) * (fraction - 0.5) + (from - to) * (fraction - 0.5) + to);
    }
    else
    {
        v5 = (to - from) * fraction;
        return (float)(v5 + v5 + from);
    }
}

void __cdecl LerpVec3(float *from, float *to, float fraction, visionSetLerpStyle_t style, float *result)
{
    *result = LerpFloat(*from, *to, fraction, style);
    result[1] = LerpFloat(from[1], to[1], fraction, style);
    result[2] = LerpFloat(from[2], to[2], fraction, style);
}

char __cdecl CG_VisionSetStartLerp_To(
    int localClientNum,
    visionSetMode_t mode,
    visionSetLerpStyle_t style,
    char *nameTo,
    int duration)
{
    if ((unsigned int)mode > VISIONSETMODECOUNT)
        MyAssertHandler(
            ".\\cgame\\cg_visionsets.cpp",
            500,
            0,
            "mode not in [0, VISIONSETMODECOUNT]\n\t%i not in [%i, %i]",
            mode,
            0,
            2);
    if (localClientNum)
        MyAssertHandler(
            "c:\\trees\\cod3\\src\\cgame\\../cgame_mp/cg_local_mp.h",
            1071,
            0,
            "%s\n\t(localClientNum) = %i",
            "(localClientNum == 0)",
            localClientNum);
    if (duration <= 0 || cgArray[0].visionSetLerpData[mode].style == VISIONSETLERP_UNDEFINED)
        return VisionSetCurrent(localClientNum, mode, nameTo);
    if (!GetVisionSet(localClientNum, nameTo, &cgArray[0].visionSetTo[mode]))
        return 0;
    memcpy(&cgArray[0].visionSetFrom[mode], &cgArray[0].visionSetCurrent[mode], sizeof(cgArray[0].visionSetFrom[mode]));
    cgArray[0].visionSetLerpData[mode].style = style;
    cgArray[0].visionSetLerpData[mode].timeDuration = duration;
    cgArray[0].visionSetLerpData[mode].timeStart = cgArray[0].time;
    return 1;
}

char __cdecl GetVisionSet(int localClientNum, char *name, visionSetVars_t *resultSettings)
{
    int idx; // [esp+10h] [ebp-4h]
    int idxa; // [esp+10h] [ebp-4h]

    if (!name)
        MyAssertHandler(".\\cgame\\cg_visionsets.cpp", 262, 0, "%s", "name");
    if (!resultSettings)
        MyAssertHandler(".\\cgame\\cg_visionsets.cpp", 263, 0, "%s", "resultSettings");
    if (!*name)
        return 0;
    if (localClientNum)
        MyAssertHandler(
            "c:\\trees\\cod3\\src\\cgame\\../cgame_mp/cg_local_mp.h",
            1071,
            0,
            "%s\n\t(localClientNum) = %i",
            "(localClientNum == 0)",
            localClientNum);
    for (idx = 0; idx < 4 && I_stricmp(name, cgArray[0].visionSetPreLoadedName[idx]); ++idx)
        ;
    if (idx == 4)
    {
        for (idxa = 0; idxa < 4 && cgArray[0].visionSetPreLoadedName[idxa][0]; ++idxa)
            ;
        if (idxa == 4)
            idxa = 0;
        if (LoadVisionFile(name, &cgArray[0].visionSetPreLoaded[idxa]))
        {
            memcpy(resultSettings, &cgArray[0].visionSetPreLoaded[idxa], sizeof(visionSetVars_t));
            I_strncpyz(cgArray[0].visionSetPreLoadedName[idxa], name, 64);
            return 1;
        }
        else
        {
            return 0;
        }
    }
    else
    {
        memcpy(resultSettings, &cgArray[0].visionSetPreLoaded[idx], sizeof(visionSetVars_t));
        return 1;
    }
}

char __cdecl LoadVisionFile(const char *name, visionSetVars_t *resultSettings)
{
    char *fileBuf; // [esp+0h] [ebp-50h]
    char success; // [esp+7h] [ebp-49h]
    char fullPath[68]; // [esp+8h] [ebp-48h] BYREF

    if (!name)
        MyAssertHandler(".\\cgame\\cg_visionsets.cpp", 241, 0, "%s", "name");
    if (!resultSettings)
        MyAssertHandler(".\\cgame\\cg_visionsets.cpp", 242, 0, "%s", "resultSettings");
    fileBuf = RawBufferOpen(name, "vision/%s.vision");
    if (!fileBuf)
        return 0;
    Com_sprintf(fullPath, 0x40u, "vision/%s.vision", name);
    success = LoadVisionSettingsFromBuffer(fileBuf, fullPath, resultSettings);
    Com_UnloadRawTextFile(fileBuf);
    return success;
}

char *__cdecl RawBufferOpen(const char *name, const char *formatFullPath)
{
    char *filebuf; // [esp+0h] [ebp-4Ch]
    char fullpath[68]; // [esp+4h] [ebp-48h] BYREF

    Com_sprintf(fullpath, 0x40u, formatFullPath, name);
    filebuf = Com_LoadRawTextFile(fullpath);
    if (filebuf)
        return filebuf;
    Com_PrintError(17, "couldn't open '%s'.\n", fullpath);
    Com_sprintf(fullpath, 0x40u, formatFullPath, "default");
    filebuf = Com_LoadRawTextFile(fullpath);
    if (filebuf)
        return filebuf;
    Com_PrintError(17, "couldn't open '%s'. This is a default file that you should have.\n", fullpath);
    return 0;
}

char __cdecl LoadVisionSettingsFromBuffer(const char *buffer, const char *filename, visionSetVars_t *settings)
{
    signed int fieldNum; // [esp+0h] [ebp-20h]
    bool wasRead[16]; // [esp+4h] [ebp-1Ch] BYREF
    const char *token; // [esp+1Ch] [ebp-4h]

    if (!settings)
        MyAssertHandler(".\\cgame\\cg_visionsets.cpp", 195, 0, "%s", "settings");
    memset(wasRead, 0, sizeof(wasRead));
    Com_BeginParseSession(filename);
    while (1)
    {
        token = (const char *)Com_Parse(&buffer);
        if (!*token)
            break;
        for (fieldNum = 0; ; ++fieldNum)
        {
            if (fieldNum >= 16)
            {
                Com_PrintWarning(16, "WARNING: unknown dvar '%s' in file '%s'\n", token, filename);
                goto next_var;
            }
            if (!wasRead[fieldNum] && !I_stricmp(token, visionDefFields[fieldNum].name))
                break;
        }
        token = (const char *)Com_ParseOnLine(&buffer);
        if (ApplyTokenToField(fieldNum, token, settings))
            wasRead[fieldNum] = 1;
        else
            Com_PrintWarning(16, "WARNING: malformed dvar '%s' in file '%s'\n", token, filename);
    next_var:
        Com_SkipRestOfLine(&buffer);
    }
    Com_EndParseSession();
    return 1;
}

char __cdecl ApplyTokenToField(unsigned int fieldNum, const char *token, visionSetVars_t *settings)
{
    int fieldType; // [esp+0h] [ebp-30h]
    int tempInt; // [esp+Ch] [ebp-24h] BYREF
    float *vec3Field; // [esp+10h] [ebp-20h]
    void *voidField; // [esp+14h] [ebp-1Ch]
    int scanResult; // [esp+18h] [ebp-18h]
    float tempVec[3]; // [esp+1Ch] [ebp-14h] BYREF
    float tempFloat; // [esp+28h] [ebp-8h] BYREF
    bool *boolField; // [esp+2Ch] [ebp-4h]

    if (fieldNum > 0x10)
        MyAssertHandler(
            ".\\cgame\\cg_visionsets.cpp",
            151,
            0,
            "fieldNum not in [0, numVisionDefFields]\n\t%i not in [%i, %i]",
            fieldNum,
            0,
            16);
    if (!token)
        MyAssertHandler(".\\cgame\\cg_visionsets.cpp", 152, 0, "%s", "token");
    if (!settings)
        MyAssertHandler(".\\cgame\\cg_visionsets.cpp", 153, 0, "%s", "settings");
    voidField = &settings->glowEnable + visionDefFields[fieldNum].offset;
    fieldType = visionDefFields[fieldNum].fieldType;
    if (fieldType)
    {
        if (fieldType == 1)
        {
            if (sscanf(token, "%f", &tempFloat) != 1)
                return 0;
            *(float *)voidField = tempFloat;
        }
        else
        {
            if (visionDefFields[fieldNum].fieldType != 2)
                MyAssertHandler(".\\cgame\\cg_visionsets.cpp", 176, 0, "%s", "fieldDef->fieldType == DTYPE_VEC3");
            vec3Field = (float *)voidField;
            if (sscanf(token, "%f %f %f", tempVec, &tempVec[1], &tempVec[2]) != 3)
                return 0;
            *vec3Field = tempVec[0];
            vec3Field[1] = tempVec[1];
            vec3Field[2] = tempVec[2];
        }
    }
    else
    {
        boolField = (bool *)voidField;
        scanResult = sscanf(token, "%i", &tempInt);
        if (scanResult != 1)
            return 0;
        *boolField = tempInt != 0;
    }
    return 1;
}

char __cdecl VisionSetCurrent(int localClientNum, visionSetMode_t mode, char *name)
{
    if (localClientNum)
        MyAssertHandler(
            "c:\\trees\\cod3\\src\\cgame\\../cgame_mp/cg_local_mp.h",
            1071,
            0,
            "%s\n\t(localClientNum) = %i",
            "(localClientNum == 0)",
            localClientNum);
    if (!GetVisionSet(localClientNum, name, &cgArray[0].visionSetCurrent[mode]))
        return 0;
    cgArray[0].visionSetLerpData[mode].style = VISIONSETLERP_NONE;
    return 1;
}

void __cdecl SetDefaultVision(int localClientNum)
{
    if (localClientNum)
        MyAssertHandler(
            "c:\\trees\\cod3\\src\\cgame\\../cgame_mp/cg_local_mp.h",
            1071,
            0,
            "%s\n\t(localClientNum) = %i",
            "(localClientNum == 0)",
            localClientNum);
    I_strncpyz(cgArray[0].visionNameNaked, (char *)MYDEFAULTVISIONNAME, 64);
    CG_VisionSetStartLerp_To(localClientNum, VISIONSETMODE_NAKED, VISIONSETLERP_TO_SMOOTH, cgArray[0].visionNameNaked, 0);
}

void __cdecl CG_VisionSetConfigString_Naked(int localClientNum)
{
    parseInfo_t *v1; // eax
    int duration; // [esp+0h] [ebp-10h]
    const char *configString; // [esp+8h] [ebp-8h] BYREF
    const char *token; // [esp+Ch] [ebp-4h]

    if (localClientNum)
        MyAssertHandler(
            "c:\\trees\\cod3\\src\\cgame\\../cgame_mp/cg_local_mp.h",
            1071,
            0,
            "%s\n\t(localClientNum) = %i",
            "(localClientNum == 0)",
            localClientNum);
    configString = CL_GetConfigString(localClientNum, 0x338u);
    token = (const char *)Com_Parse(&configString);
    I_strncpyz(cgArray[0].visionNameNaked, (char *)token, 64);
    v1 = Com_Parse(&configString);
    duration = atoi(v1->token);
    CG_VisionSetStartLerp_To(
        localClientNum,
        VISIONSETMODE_NAKED,
        VISIONSETLERP_TO_SMOOTH,
        cgArray[0].visionNameNaked,
        duration);
}

void __cdecl CG_VisionSetConfigString_Night(int localClientNum)
{
    parseInfo_t *v1; // eax
    int duration; // [esp+0h] [ebp-10h]
    const char *configString; // [esp+8h] [ebp-8h] BYREF
    const char *token; // [esp+Ch] [ebp-4h]

    if (localClientNum)
        MyAssertHandler(
            "c:\\trees\\cod3\\src\\cgame\\../cgame_mp/cg_local_mp.h",
            1071,
            0,
            "%s\n\t(localClientNum) = %i",
            "(localClientNum == 0)",
            localClientNum);
    configString = CL_GetConfigString(localClientNum, 0x339u);
    token = (const char *)Com_Parse(&configString);
    I_strncpyz(cgArray[0].visionNameNight, (char *)token, 64);
    v1 = Com_Parse(&configString);
    duration = atoi(v1->token);
    CG_VisionSetStartLerp_To(
        localClientNum,
        VISIONSETMODE_NIGHT,
        VISIONSETLERP_TO_SMOOTH,
        cgArray[0].visionNameNight,
        duration);
}

void __cdecl CG_VisionSetMyChanges()
{
    unsigned int visSetIdx; // [esp+8h] [ebp-8h]
    int localClientNum; // [esp+Ch] [ebp-4h]

    for (localClientNum = 0; localClientNum < 1; ++localClientNum)
    {
        if (localClientNum)
            MyAssertHandler(
                "c:\\trees\\cod3\\src\\cgame\\../client_mp/client_mp.h",
                1063,
                0,
                "%s\n\t(localClientNum) = %i",
                "(localClientNum == 0)",
                localClientNum);
        if (clientUIActives[0].cgameInitialized)
        {
            if (localClientNum)
                MyAssertHandler(
                    "c:\\trees\\cod3\\src\\cgame\\../cgame_mp/cg_local_mp.h",
                    1071,
                    0,
                    "%s\n\t(localClientNum) = %i",
                    "(localClientNum == 0)",
                    localClientNum);
            for (visSetIdx = 0; visSetIdx < 4; ++visSetIdx)
                cgArray[0].visionSetPreLoadedName[visSetIdx][0] = 0;
            if (cgArray[0].visionNameNaked[0])
                CG_VisionSetStartLerp_To(
                    localClientNum,
                    VISIONSETMODE_NAKED,
                    VISIONSETLERP_TO_LINEAR,
                    cgArray[0].visionNameNaked,
                    0);
            if (cgArray[0].visionNameNight[0])
                CG_VisionSetStartLerp_To(
                    localClientNum,
                    VISIONSETMODE_NIGHT,
                    VISIONSETLERP_TO_LINEAR,
                    cgArray[0].visionNameNight,
                    0);
        }
    }
}

void __cdecl CG_VisionSetUpdateTweaksFromFile_Glow()
{
    visionSetVars_t setVars; // [esp+8h] [ebp-50h] BYREF

    if (LoadVisionFileForTweaks(&setVars))
    {
        Dvar_SetBoolByName("r_glowTweakEnable", setVars.glowEnable);
        Dvar_SetFloatByName("r_glowTweakRadius0", setVars.glowRadius0);
        Dvar_SetFloatByName("r_glowTweakRadius1", setVars.glowRadius1);
        Dvar_SetFloatByName("r_glowTweakSkyBleedIntensity0", setVars.glowSkyBleedIntensity0);
        Dvar_SetFloatByName("r_glowTweakSkyBleedIntensity1", setVars.glowSkyBleedIntensity1);
        Dvar_SetFloatByName("r_glowTweakBloomIntensity0", setVars.glowBloomIntensity0);
        Dvar_SetFloatByName("r_glowTweakBloomIntensity1", setVars.glowBloomIntensity1);
        Dvar_SetFloatByName("r_glowTweakBloomCutoff", setVars.glowBloomCutoff);
        Dvar_SetFloatByName("r_glowTweakBloomDesaturation", setVars.glowBloomDesaturation);
    }
    else
    {
        Com_PrintWarning(16, "WARNING: Couldn't update glow tweak vars from file.  Vision file is likely not in use.\n");
    }
}

bool __cdecl LoadVisionFileForTweaks(visionSetVars_t *setVars)
{
    char *setName; // [esp+Ch] [ebp-4h]

    if (CG_LookingThroughNightVision(0))
        setName = cgArray[0].visionNameNight;
    else
        setName = cgArray[0].visionNameNaked;
    return *setName && GetVisionSet(0, setName, setVars) != 0;
}

void __cdecl CG_VisionSetUpdateTweaksFromFile_Film()
{
    visionSetVars_t setVars; // [esp+10h] [ebp-50h] BYREF

    if (LoadVisionFileForTweaks(&setVars))
    {
        Dvar_SetBoolByName("r_filmTweakEnable", setVars.filmEnable);
        Dvar_SetFloatByName("r_filmTweakContrast", setVars.filmContrast);
        Dvar_SetFloatByName("r_filmTweakBrightness", setVars.filmBrightness);
        Dvar_SetFloatByName("r_filmTweakDesaturation", setVars.filmDesaturation);
        Dvar_SetBoolByName("r_filmTweakInvert", setVars.filmInvert);
        Dvar_SetVec3ByName(
            "r_filmTweakLightTint",
            setVars.filmLightTint[0],
            setVars.filmLightTint[1],
            setVars.filmLightTint[2]);
        Dvar_SetVec3ByName("r_filmTweakDarkTint", setVars.filmDarkTint[0], setVars.filmDarkTint[1], setVars.filmDarkTint[2]);
    }
    else
    {
        Com_PrintWarning(16, "WARNING: Couldn't update film tweak vars from file.  Vision file is likely not in use.\n");
    }
}

char __cdecl CG_LookingThroughNightVision(int localClientNum)
{
    int weapIndex; // [esp+4h] [ebp-10h]
    WeaponDef *weapDef; // [esp+10h] [ebp-4h]

    if (localClientNum)
        MyAssertHandler(
            "c:\\trees\\cod3\\src\\cgame\\../client_mp/client_mp.h",
            1112,
            0,
            "%s\n\t(localClientNum) = %i",
            "(localClientNum == 0)",
            localClientNum);
    if (clientUIActives[0].connectionState < CA_ACTIVE)
        return 0;
    if (nightVisionDisableEffects->current.enabled)
        return 0;
    if (localClientNum)
        MyAssertHandler(
            "c:\\trees\\cod3\\src\\cgame\\../cgame_mp/cg_local_mp.h",
            1071,
            0,
            "%s\n\t(localClientNum) = %i",
            "(localClientNum == 0)",
            localClientNum);
    weapIndex = BG_GetViewmodelWeaponIndex(&cgArray[0].predictedPlayerState);
    weapDef = BG_GetWeaponDef(weapIndex);
    if (cgArray[0].predictedPlayerState.weaponstate == 25)
    {
        if (weapDef->nightVisionWearTime - cgArray[0].predictedPlayerState.weaponTime >= weapDef->nightVisionWearTimePowerUp)
            return 1;
    }
    else if (cgArray[0].predictedPlayerState.weaponstate == 26)
    {
        if (weapDef->nightVisionRemoveTime - cgArray[0].predictedPlayerState.weaponTime <= weapDef->nightVisionRemoveTimePowerDown)
            return 1;
    }
    else if ((cgArray[0].predictedPlayerState.weapFlags & 0x40) != 0)
    {
        return 1;
    }
    return 0;
}

void __cdecl CG_VisionSetApplyToRefdef(int localClientNum)
{
    float fade; // [esp+14h] [ebp-1Ch]
    GfxFilm *film; // [esp+1Ch] [ebp-14h]
    GfxGlow *glow; // [esp+20h] [ebp-10h]
    visionSetMode_t visionChannel; // [esp+2Ch] [ebp-4h]

    if (localClientNum)
        MyAssertHandler(
            "c:\\trees\\cod3\\src\\cgame\\../cgame_mp/cg_local_mp.h",
            1071,
            0,
            "%s\n\t(localClientNum) = %i",
            "(localClientNum == 0)",
            localClientNum);
    film = &cgArray[0].refdef.film;
    glow = &cgArray[0].refdef.glow;
    visionChannel = (visionSetMode_t)(CG_LookingThroughNightVision(localClientNum) != 0);
    if (cgArray[0].visionSetLerpData[visionChannel].style)
    {
        film->enabled = cgArray[0].visionSetCurrent[visionChannel].filmEnable;
        cgArray[0].refdef.film.brightness = cgArray[0].visionSetCurrent[visionChannel].filmBrightness;
        cgArray[0].refdef.film.contrast = cgArray[0].visionSetCurrent[visionChannel].filmContrast;
        cgArray[0].refdef.film.desaturation = cgArray[0].visionSetCurrent[visionChannel].filmDesaturation;
        cgArray[0].refdef.film.invert = cgArray[0].visionSetCurrent[visionChannel].filmInvert;
        cgArray[0].refdef.film.tintDark[0] = cgArray[0].visionSetCurrent[visionChannel].filmDarkTint[0];
        cgArray[0].refdef.film.tintDark[1] = cgArray[0].visionSetCurrent[visionChannel].filmDarkTint[1];
        cgArray[0].refdef.film.tintDark[2] = cgArray[0].visionSetCurrent[visionChannel].filmDarkTint[2];
        cgArray[0].refdef.film.tintLight[0] = cgArray[0].visionSetCurrent[visionChannel].filmLightTint[0];
        cgArray[0].refdef.film.tintLight[1] = cgArray[0].visionSetCurrent[visionChannel].filmLightTint[1];
        cgArray[0].refdef.film.tintLight[2] = cgArray[0].visionSetCurrent[visionChannel].filmLightTint[2];
        glow->enabled = cgArray[0].visionSetCurrent[visionChannel].glowEnable;
        cgArray[0].refdef.glow.bloomCutoff = cgArray[0].visionSetCurrent[visionChannel].glowBloomCutoff;
        cgArray[0].refdef.glow.bloomDesaturation = cgArray[0].visionSetCurrent[visionChannel].glowBloomDesaturation;
        cgArray[0].refdef.glow.bloomIntensity = cgArray[0].visionSetCurrent[visionChannel].glowBloomIntensity0;
        cgArray[0].refdef.glow.radius = cgArray[0].visionSetCurrent[visionChannel].glowRadius0;
        fade = VisionFadeValue(localClientNum);
        if (fade != 1.0)
            FadeRefDef(&cgArray[0].refdef, fade);
    }
    else
    {
        film->enabled = 0;
        glow->enabled = 0;
    }
}

double __cdecl VisionFadeValue(int localClientNum)
{
    float v3; // [esp+4h] [ebp-50h]
    float v4; // [esp+8h] [ebp-4Ch]
    float v6; // [esp+14h] [ebp-40h]
    float v7; // [esp+18h] [ebp-3Ch]
    float v9; // [esp+24h] [ebp-30h]
    float v10; // [esp+28h] [ebp-2Ch]
    float v11; // [esp+30h] [ebp-24h]
    float v12; // [esp+34h] [ebp-20h]
    float v13; // [esp+38h] [ebp-1Ch]
    float deltac; // [esp+3Ch] [ebp-18h]
    float delta; // [esp+3Ch] [ebp-18h]
    float deltad; // [esp+3Ch] [ebp-18h]
    float deltaa; // [esp+3Ch] [ebp-18h]
    float deltae; // [esp+3Ch] [ebp-18h]
    float deltab; // [esp+3Ch] [ebp-18h]
    int weapIndex; // [esp+44h] [ebp-10h]
    int timePassed; // [esp+48h] [ebp-Ch]
    int timePasseda; // [esp+48h] [ebp-Ch]
    WeaponDef *weapDef; // [esp+50h] [ebp-4h]

    if (localClientNum)
        MyAssertHandler(
            "c:\\trees\\cod3\\src\\cgame\\../cgame_mp/cg_local_mp.h",
            1071,
            0,
            "%s\n\t(localClientNum) = %i",
            "(localClientNum == 0)",
            localClientNum);
    weapIndex = BG_GetViewmodelWeaponIndex(&cgArray[0].predictedPlayerState);
    weapDef = BG_GetWeaponDef(weapIndex);
    if (!weapIndex)
        return 1.0;
    if (cgArray[0].predictedPlayerState.weaponstate == 25)
    {
        timePassed = weapDef->nightVisionWearTime - cgArray[0].predictedPlayerState.weaponTime;
        if (timePassed > weapDef->nightVisionWearTimeFadeOutEnd)
        {
            if (timePassed < weapDef->nightVisionWearTimePowerUp)
            {
                return 0.0;
            }
            else
            {
                deltad = (float)(timePassed - weapDef->nightVisionWearTimePowerUp);
                deltaa = deltad / (nightVisionPowerOnTime->current.value * 1000.0);
                v7 = deltaa - 1.0;
                if (v7 < 0.0)
                    v12 = deltaa;
                else
                    v12 = 1.0;
                v6 = 0.0 - deltaa;
                if (v6 < 0.0)
                    return v12;
                else
                    return (float)0.0;
            }
        }
        else
        {
            deltac = (float)(weapDef->nightVisionWearTimeFadeOutEnd - timePassed);
            delta = deltac / (nightVisionFadeInOutTime->current.value * 1000.0);
            v10 = delta - 1.0;
            if (v10 < 0.0)
                v13 = delta;
            else
                v13 = 1.0;
            v9 = 0.0 - delta;
            if (v9 < 0.0)
                return v13;
            else
                return (float)0.0;
        }
    }
    else if (cgArray[0].predictedPlayerState.weaponstate == 26)
    {
        timePasseda = weapDef->nightVisionRemoveTime - cgArray[0].predictedPlayerState.weaponTime;
        if (timePasseda >= weapDef->nightVisionRemoveTimePowerDown)
        {
            if (timePasseda < weapDef->nightVisionRemoveTimeFadeInStart)
            {
                return 0.0;
            }
            else
            {
                deltae = (float)(timePasseda - weapDef->nightVisionRemoveTimeFadeInStart);
                deltab = deltae / (nightVisionFadeInOutTime->current.value * 1000.0);
                v4 = deltab - 1.0;
                if (v4 < 0.0)
                    v11 = deltab;
                else
                    v11 = 1.0;
                v3 = 0.0 - deltab;
                if (v3 < 0.0)
                    return v11;
                else
                    return (float)0.0;
            }
        }
        else
        {
            return 1.0;
        }
    }
    else
    {
        return 1.0;
    }
}

void __cdecl FadeRefDef(refdef_s *rd, float brightness)
{
    if (!rd)
        MyAssertHandler(".\\cgame\\cg_visionsets.cpp", 806, 0, "%s", "rd");
    if (brightness < 0.0 || brightness > 1.0)
        MyAssertHandler(
            ".\\cgame\\cg_visionsets.cpp",
            807,
            0,
            "%s\n\t(brightness) = %g",
            "((brightness >= 0.f) && (brightness <= 1.f))",
            brightness);
    rd->glow.bloomCutoff = brightness * rd->glow.bloomCutoff + (1.0 - brightness) * 1.0;
    rd->glow.bloomDesaturation = brightness * rd->glow.bloomDesaturation + (1.0 - brightness) * 0.0;
    rd->film.brightness = brightness * rd->film.brightness + (1.0 - brightness) * -1.0;
    rd->film.contrast = brightness * rd->film.contrast + (1.0 - brightness) * 0.0;
    rd->film.desaturation = brightness * rd->film.desaturation + (1.0 - brightness) * 0.0;
    rd->film.tintDark[0] = brightness * rd->film.tintDark[0] + (1.0 - brightness) * 0.0;
    rd->film.tintDark[1] = brightness * rd->film.tintDark[1] + (1.0 - brightness) * 0.0;
    rd->film.tintDark[2] = brightness * rd->film.tintDark[2] + (1.0 - brightness) * 0.0;
    rd->film.tintLight[0] = brightness * rd->film.tintLight[0] + (1.0 - brightness) * 0.0;
    rd->film.tintLight[1] = brightness * rd->film.tintLight[1] + (1.0 - brightness) * 0.0;
    rd->film.tintLight[2] = brightness * rd->film.tintLight[2] + (1.0 - brightness) * 0.0;
}

