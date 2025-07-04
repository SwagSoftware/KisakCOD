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
    int32_t offset;
    int32_t fieldType;
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

    min.value.max = 10000.0f;
    min.value.min = 0.0f;
    nightVisionFadeInOutTime = Dvar_RegisterFloat(
        "nightVisionFadeInOutTime",
        0.1f,
        min,
        DVAR_SAVED,
        "How long the fade to/from black lasts when putting on or removing night vision goggles.");
    mina.value.max = 10000.0f;
    mina.value.min = 0.0f;
    nightVisionPowerOnTime = Dvar_RegisterFloat(
        "nightVisionPowerOnTime",
        0.30000001f,
        mina,
        DVAR_SAVED,
        "How long the black-to-nightvision fade lasts when turning on the goggles.");
    nightVisionDisableEffects = Dvar_RegisterBool("nightVisionDisableEffects", 0, DVAR_SAVED, "");
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
        strncpy(visionSetName, visionSetNameBegin, visionSetNameEnd - visionSetNameBegin);
        visionSetName[visionSetNameEnd - visionSetNameBegin] = 0;
        _snprintf(devguiPath, 0x100u, "Renderer/Vision Sets/%s", visionSetName);
        _snprintf(command, 0x100u, "VisionSetNaked %s", visionSetName);
        DevGui_AddCommand(devguiPath, command);
    }
}

void __cdecl CG_VisionSetsUpdate(int32_t localClientNum)
{
    int32_t idx; // [esp+4h] [ebp-4h]

    cg_s *cgameGlob;

    cgameGlob = CG_GetLocalClientGlobals(localClientNum);

    if (!cgameGlob->visionNameNaked[0])
        SetDefaultVision(localClientNum);

    for (idx = 0; idx < 2; ++idx)
        UpdateVarsLerp(
            cgameGlob->time,
            &cgameGlob->visionSetFrom[idx],
            &cgameGlob->visionSetTo[idx],
            &cgameGlob->visionSetLerpData[idx],
            &cgameGlob->visionSetCurrent[idx]);
}

void __cdecl UpdateVarsLerp(
    int32_t time,
    const visionSetVars_t *from,
    const visionSetVars_t *to,
    visionSetLerpData_t *lerpData,
    visionSetVars_t *result)
{
    int32_t fieldType; // [esp+18h] [ebp-34h]
    float v6; // [esp+1Ch] [ebp-30h]
    float v7; // [esp+20h] [ebp-2Ch]
    float v8; // [esp+24h] [ebp-28h]
    visionSetLerpStyle_t style; // [esp+2Ch] [ebp-20h]
    float v10; // [esp+30h] [ebp-1Ch]
    float *voidFrom; // [esp+38h] [ebp-14h]
    float *voidTo; // [esp+3Ch] [ebp-10h]
    float *voidResult; // [esp+40h] [ebp-Ch]
    float fraction; // [esp+44h] [ebp-8h]
    int32_t fieldNum; // [esp+48h] [ebp-4h]

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
    int32_t localClientNum,
    visionSetMode_t mode,
    visionSetLerpStyle_t style,
    char *nameTo,
    int32_t duration)
{
    cg_s *cgameGlob;

    bcassert(mode, VISIONSETMODECOUNT);

    cgameGlob = CG_GetLocalClientGlobals(localClientNum);

    if (duration <= 0 || cgameGlob->visionSetLerpData[mode].style == VISIONSETLERP_UNDEFINED)
        return VisionSetCurrent(localClientNum, mode, nameTo);

    if (!GetVisionSet(localClientNum, nameTo, &cgameGlob->visionSetTo[mode]))
        return 0;

    memcpy(&cgameGlob->visionSetFrom[mode], &cgameGlob->visionSetCurrent[mode], sizeof(cgameGlob->visionSetFrom[mode]));
    cgameGlob->visionSetLerpData[mode].style = style;
    cgameGlob->visionSetLerpData[mode].timeDuration = duration;
    cgameGlob->visionSetLerpData[mode].timeStart = cgameGlob->time;

    return 1;
}

char __cdecl GetVisionSet(int32_t localClientNum, char *name, visionSetVars_t *resultSettings)
{
    int32_t idx; // [esp+10h] [ebp-4h]
    int32_t idxa; // [esp+10h] [ebp-4h]
    cg_s *cgameGlob;

    iassert(name);
    iassert(resultSettings);

    if (!*name)
        return 0;

    cgameGlob = CG_GetLocalClientGlobals(localClientNum);

    for (idx = 0; idx < 4 && I_stricmp(name, cgameGlob->visionSetPreLoadedName[idx]); ++idx)
        ;
    if (idx == 4)
    {
        for (idxa = 0; idxa < 4 && cgameGlob->visionSetPreLoadedName[idxa][0]; ++idxa)
            ;
        if (idxa == 4)
            idxa = 0;
        if (LoadVisionFile(name, &cgameGlob->visionSetPreLoaded[idxa]))
        {
            memcpy(resultSettings, &cgameGlob->visionSetPreLoaded[idxa], sizeof(visionSetVars_t));
            I_strncpyz(cgameGlob->visionSetPreLoadedName[idxa], name, 64);
            return 1;
        }
        else
        {
            return 0;
        }
    }
    else
    {
        memcpy(resultSettings, &cgameGlob->visionSetPreLoaded[idx], sizeof(visionSetVars_t));
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
    int32_t fieldNum; // [esp+0h] [ebp-20h]
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

char __cdecl ApplyTokenToField(uint32_t fieldNum, const char *token, visionSetVars_t *settings)
{
    int32_t fieldType; // [esp+0h] [ebp-30h]
    int32_t tempInt; // [esp+Ch] [ebp-24h] BYREF
    float *vec3Field; // [esp+10h] [ebp-20h]
    void *voidField; // [esp+14h] [ebp-1Ch]
    int32_t scanResult; // [esp+18h] [ebp-18h]
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

char __cdecl VisionSetCurrent(int32_t localClientNum, visionSetMode_t mode, char *name)
{
    cg_s *cgameGlob;

    cgameGlob = CG_GetLocalClientGlobals(localClientNum);

    if (!GetVisionSet(localClientNum, name, &cgameGlob->visionSetCurrent[mode]))
        return 0;

    cgameGlob->visionSetLerpData[mode].style = VISIONSETLERP_NONE;

    return 1;
}

void __cdecl SetDefaultVision(int32_t localClientNum)
{
    cg_s *cgameGlob;

    cgameGlob = CG_GetLocalClientGlobals(localClientNum);

    I_strncpyz(cgameGlob->visionNameNaked, (char *)MYDEFAULTVISIONNAME, 64);
    CG_VisionSetStartLerp_To(localClientNum, VISIONSETMODE_NAKED, VISIONSETLERP_TO_SMOOTH, cgameGlob->visionNameNaked, 0);
}

void __cdecl CG_VisionSetConfigString_Naked(int32_t localClientNum)
{
    parseInfo_t *v1; // eax
    int32_t duration; // [esp+0h] [ebp-10h]
    const char *configString; // [esp+8h] [ebp-8h] BYREF
    const char *token; // [esp+Ch] [ebp-4h]
    cg_s *cgameGlob;

    cgameGlob = CG_GetLocalClientGlobals(localClientNum);

    configString = CL_GetConfigString(localClientNum, 0x338u);
    token = (const char *)Com_Parse(&configString);
    I_strncpyz(cgameGlob->visionNameNaked, (char *)token, 64);
    v1 = Com_Parse(&configString);
    duration = atoi(v1->token);
    CG_VisionSetStartLerp_To(
        localClientNum,
        VISIONSETMODE_NAKED,
        VISIONSETLERP_TO_SMOOTH,
        cgameGlob->visionNameNaked,
        duration);
}

void __cdecl CG_VisionSetConfigString_Night(int32_t localClientNum)
{
    parseInfo_t *v1; // eax
    int32_t duration; // [esp+0h] [ebp-10h]
    const char *configString; // [esp+8h] [ebp-8h] BYREF
    const char *token; // [esp+Ch] [ebp-4h]
    cg_s *cgameGlob;

    cgameGlob = CG_GetLocalClientGlobals(localClientNum);

    configString = CL_GetConfigString(localClientNum, 0x339u);
    token = (const char *)Com_Parse(&configString);
    I_strncpyz(cgameGlob->visionNameNight, (char *)token, 64);
    v1 = Com_Parse(&configString);
    duration = atoi(v1->token);
    CG_VisionSetStartLerp_To(
        localClientNum,
        VISIONSETMODE_NIGHT,
        VISIONSETLERP_TO_SMOOTH,
        cgameGlob->visionNameNight,
        duration);
}

void __cdecl CG_VisionSetMyChanges()
{
    uint32_t visSetIdx; // [esp+8h] [ebp-8h]
    int32_t localClientNum; // [esp+Ch] [ebp-4h]
    cg_s *cgameGlob;

    for (localClientNum = 0; localClientNum < 1; ++localClientNum)
    {
        cgameGlob = CG_GetLocalClientGlobals(localClientNum);

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
                cgameGlob->visionSetPreLoadedName[visSetIdx][0] = 0;
            if (cgameGlob->visionNameNaked[0])
                CG_VisionSetStartLerp_To(
                    localClientNum,
                    VISIONSETMODE_NAKED,
                    VISIONSETLERP_TO_LINEAR,
                    cgameGlob->visionNameNaked,
                    0);
            if (cgameGlob->visionNameNight[0])
                CG_VisionSetStartLerp_To(
                    localClientNum,
                    VISIONSETMODE_NIGHT,
                    VISIONSETLERP_TO_LINEAR,
                    cgameGlob->visionNameNight,
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
    cg_s *cgameGlob;

    cgameGlob = CG_GetLocalClientGlobals(0);

    if (CG_LookingThroughNightVision(0))
        setName = cgameGlob->visionNameNight;
    else
        setName = cgameGlob->visionNameNaked;
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

char __cdecl CG_LookingThroughNightVision(int32_t localClientNum)
{
    int32_t weapIndex; // [esp+4h] [ebp-10h]
    WeaponDef *weapDef; // [esp+10h] [ebp-4h]

    // idk what this is yet
    //if (localClientNum)
    //    MyAssertHandler(
    //        "c:\\trees\\cod3\\src\\cgame\\../client_mp/client_mp.h",
    //        1112,
    //        0,
    //        "%s\n\t(localClientNum) = %i",
    //        "(localClientNum == 0)",
    //        localClientNum);

    if (clientUIActives[0].connectionState < CA_ACTIVE)
        return 0;
    if (nightVisionDisableEffects->current.enabled)
        return 0;

    cg_s *cgameGlob = CG_GetLocalClientGlobals(localClientNum);

    weapIndex = BG_GetViewmodelWeaponIndex(&cgameGlob->predictedPlayerState);
    weapDef = BG_GetWeaponDef(weapIndex);

    if (cgameGlob->predictedPlayerState.weaponstate == 25)
    {
        if (weapDef->nightVisionWearTime - cgameGlob->predictedPlayerState.weaponTime >= weapDef->nightVisionWearTimePowerUp)
            return 1;
    }
    else if (cgameGlob->predictedPlayerState.weaponstate == 26)
    {
        if (weapDef->nightVisionRemoveTime - cgameGlob->predictedPlayerState.weaponTime <= weapDef->nightVisionRemoveTimePowerDown)
            return 1;
    }
    else if ((cgameGlob->predictedPlayerState.weapFlags & 0x40) != 0)
    {
        return 1;
    }
    return 0;
}

void __cdecl CG_VisionSetApplyToRefdef(int32_t localClientNum)
{
    float fade; // [esp+14h] [ebp-1Ch]
    GfxFilm *film; // [esp+1Ch] [ebp-14h]
    GfxGlow *glow; // [esp+20h] [ebp-10h]
    visionSetMode_t visionChannel; // [esp+2Ch] [ebp-4h]
    cg_s *cgameGlob;

    cgameGlob = CG_GetLocalClientGlobals(localClientNum);

    film = &cgameGlob->refdef.film;
    glow = &cgameGlob->refdef.glow;

    visionChannel = (visionSetMode_t)(CG_LookingThroughNightVision(localClientNum) != 0);

    if (cgameGlob->visionSetLerpData[visionChannel].style)
    {
        film->enabled = cgameGlob->visionSetCurrent[visionChannel].filmEnable;
        cgameGlob->refdef.film.brightness = cgameGlob->visionSetCurrent[visionChannel].filmBrightness;
        cgameGlob->refdef.film.contrast = cgameGlob->visionSetCurrent[visionChannel].filmContrast;
        cgameGlob->refdef.film.desaturation = cgameGlob->visionSetCurrent[visionChannel].filmDesaturation;
        cgameGlob->refdef.film.invert = cgameGlob->visionSetCurrent[visionChannel].filmInvert;
        cgameGlob->refdef.film.tintDark[0] = cgameGlob->visionSetCurrent[visionChannel].filmDarkTint[0];
        cgameGlob->refdef.film.tintDark[1] = cgameGlob->visionSetCurrent[visionChannel].filmDarkTint[1];
        cgameGlob->refdef.film.tintDark[2] = cgameGlob->visionSetCurrent[visionChannel].filmDarkTint[2];
        cgameGlob->refdef.film.tintLight[0] = cgameGlob->visionSetCurrent[visionChannel].filmLightTint[0];
        cgameGlob->refdef.film.tintLight[1] = cgameGlob->visionSetCurrent[visionChannel].filmLightTint[1];
        cgameGlob->refdef.film.tintLight[2] = cgameGlob->visionSetCurrent[visionChannel].filmLightTint[2];
        glow->enabled = cgameGlob->visionSetCurrent[visionChannel].glowEnable;
        cgameGlob->refdef.glow.bloomCutoff = cgameGlob->visionSetCurrent[visionChannel].glowBloomCutoff;
        cgameGlob->refdef.glow.bloomDesaturation = cgameGlob->visionSetCurrent[visionChannel].glowBloomDesaturation;
        cgameGlob->refdef.glow.bloomIntensity = cgameGlob->visionSetCurrent[visionChannel].glowBloomIntensity0;
        cgameGlob->refdef.glow.radius = cgameGlob->visionSetCurrent[visionChannel].glowRadius0;
        fade = VisionFadeValue(localClientNum);
        if (fade != 1.0)
            FadeRefDef(&cgameGlob->refdef, fade);
    }
    else
    {
        film->enabled = 0;
        glow->enabled = 0;
    }
}

double __cdecl VisionFadeValue(int32_t localClientNum)
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
    int32_t weapIndex; // [esp+44h] [ebp-10h]
    int32_t timePassed; // [esp+48h] [ebp-Ch]
    int32_t timePasseda; // [esp+48h] [ebp-Ch]
    WeaponDef *weapDef; // [esp+50h] [ebp-4h]
    cg_s *cgameGlob;

    cgameGlob = CG_GetLocalClientGlobals(localClientNum);
    weapIndex = BG_GetViewmodelWeaponIndex(&cgameGlob->predictedPlayerState);
    weapDef = BG_GetWeaponDef(weapIndex);
    if (!weapIndex)
        return 1.0;
    if (cgameGlob->predictedPlayerState.weaponstate == 25)
    {
        timePassed = weapDef->nightVisionWearTime - cgameGlob->predictedPlayerState.weaponTime;
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
    else if (cgameGlob->predictedPlayerState.weaponstate == 26)
    {
        timePasseda = weapDef->nightVisionRemoveTime - cgameGlob->predictedPlayerState.weaponTime;
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

