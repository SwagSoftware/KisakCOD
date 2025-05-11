#include "snd_local.h"
#include "snd_public.h"
#include <qcommon/mem_track.h>
#include <universal/q_shared.h>
#include <qcommon/qcommon.h>
#include <qcommon/cmd.h>
#include <cgame_mp/cg_local_mp.h>
#include <universal/com_sndalias.h>
#include <database/database.h>
#include <universal/q_parse.h>
#include <client/client.h>

struct AsyncPlaySound // sizeof=0x14
{                                       // ...
    snd_alias_t *alias;
    SndEntHandle sndEnt;
    float origin[3];
};

const char *snd_draw3DNames[5] = { "Off", "Targets", "Names", "Verbose", NULL }; // idb

const dvar_t *snd_cinematicVolumeScale;
const dvar_t *snd_enable3D;
const dvar_t *snd_enableEq;
const dvar_t *snd_debugReplace;
const dvar_t *snd_debugAlias;
const dvar_t *snd_enable2D;
//const dvar_t *snd_khz;
const dvar_t *snd_draw3D;
const dvar_t *snd_volume;
const dvar_t *snd_errorOnMissing;
const dvar_t *snd_drawEqEnts;
const dvar_t *snd_enableReverb;
const dvar_t *snd_bits;
const dvar_t *snd_slaveFadeTime;
const dvar_t *snd_enableStream;
const dvar_t *snd_drawEqChannels;
const dvar_t *snd_levelFadeTime;
const dvar_t *snd_touchStreamFilesOnLoad;

snd_local_t g_snd;
snd_physics g_sndPhysics;

unsigned int g_FXPlaySoundCount;
AsyncPlaySound g_FXPlaySounds[32];

const char *snd_roomStrings[27] =
{
  "generic",
  "paddedcell",
  "room",
  "bathroom",
  "livingroom",
  "stoneroom",
  "auditorium",
  "concerthall",
  "cave",
  "arena",
  "hangar",
  "carpetedhallway",
  "hallway",
  "stonecorridor",
  "alley",
  "forest",
  "city",
  "mountains",
  "quarry",
  "plain",
  "parkinglot",
  "sewerpipe",
  "underwater",
  "drugged",
  "dizzy",
  "psychotic",
  NULL
}; // idb

void __cdecl TRACK_snd()
{
    track_static_alloc_internal(&g_snd, 32504, "g_snd", 13);
}

void __cdecl SND_DebugAliasPrint(bool condition, const snd_alias_t *alias, const char *msg)
{
    if (condition && !I_stricmp(snd_debugAlias->current.string, alias->aliasName))
        Com_DPrintf(9, "^5SND_DEBUG_ALIAS (%s): %s\n", alias->aliasName, msg);
}

int __cdecl SND_GetEntChannelCount()
{
    return g_snd.entchannel_count;
}

bool __cdecl SND_IsStreamChannelLoading(int index)
{
    if (!g_snd.max_stream_channels)
        MyAssertHandler(".\\snd.cpp", 148, 0, "%s", "g_snd.max_stream_channels");
    if (index < 40 || index >= g_snd.max_stream_channels + 40)
        MyAssertHandler(
            ".\\snd.cpp",
            149,
            0,
            "%s\n\t(index) = %i",
            "(index >= ((0 + 8) + 32) && index < ((0 + 8) + 32) + g_snd.max_stream_channels)",
            index);
    return g_snd.chaninfo[index].soundFileInfo.loadingState == SFLS_LOADING;
}

bool __cdecl SND_HasFreeVoice(int entchannel)
{
    int loadingStreamCount; // [esp+0h] [ebp-8h]
    int index; // [esp+4h] [ebp-4h]

    if (entchannel < 0 || entchannel >= g_snd.entchannel_count)
        MyAssertHandler(
            ".\\snd.cpp",
            156,
            0,
            "%s\n\t(entchannel) = %i",
            "(entchannel >= 0 && entchannel < g_snd.entchannel_count)",
            entchannel);
    if (g_snd.entchaninfo[entchannel].maxVoices <= 0)
        MyAssertHandler(
            ".\\snd.cpp",
            157,
            0,
            "%s\n\t(g_snd.entchaninfo[entchannel].maxVoices) = %i",
            "(g_snd.entchaninfo[entchannel].maxVoices > 0)",
            g_snd.entchaninfo[entchannel].maxVoices);
    loadingStreamCount = 0;
    for (index = 40; index < 53; ++index)
    {
        if (SND_IsStreamChannelLoading(index))
        {
            if (entchannel == g_snd.chaninfo[index].entchannel)
                ++loadingStreamCount;
        }
    }
    return loadingStreamCount + g_snd.entchaninfo[entchannel].voiceCount < g_snd.entchaninfo[entchannel].maxVoices;
}

void __cdecl SND_AddVoice(int entchannel)
{
    const char *v1; // eax

    if (entchannel < 0 || entchannel >= g_snd.entchannel_count)
        MyAssertHandler(
            ".\\snd.cpp",
            175,
            0,
            "%s\n\t(entchannel) = %i",
            "(entchannel >= 0 && entchannel < g_snd.entchannel_count)",
            entchannel);
    if (g_snd.entchaninfo[entchannel].voiceCount >= g_snd.entchaninfo[entchannel].maxVoices)
    {
        v1 = va(
            "Not enough voices: entchannel = %s, maxvoices = %i",
            g_snd.entchaninfo[entchannel].name,
            g_snd.entchaninfo[entchannel].maxVoices);
        MyAssertHandler(
            ".\\snd.cpp",
            176,
            0,
            "%s\n\t%s",
            "g_snd.entchaninfo[entchannel].voiceCount < g_snd.entchaninfo[entchannel].maxVoices",
            v1);
    }
    ++g_snd.entchaninfo[entchannel].voiceCount;
}

void __cdecl SND_RemoveVoice(int entchannel)
{
    if (entchannel < 0 || entchannel >= g_snd.entchannel_count)
        MyAssertHandler(
            ".\\snd.cpp",
            184,
            0,
            "%s\n\t(entchannel) = %i",
            "(entchannel >= 0 && entchannel < g_snd.entchannel_count)",
            entchannel);
    if (g_snd.entchaninfo[entchannel].voiceCount <= 0)
        MyAssertHandler(
            ".\\snd.cpp",
            185,
            0,
            "%s\n\t(g_snd.entchaninfo[entchannel].voiceCount) = %i",
            "(g_snd.entchaninfo[entchannel].voiceCount > 0)",
            g_snd.entchaninfo[entchannel].voiceCount);
    --g_snd.entchaninfo[entchannel].voiceCount;
}

int __cdecl SND_GetPriority(int entchannel)
{
    if (entchannel < 0 || entchannel >= g_snd.entchannel_count)
        MyAssertHandler(
            ".\\snd.cpp",
            193,
            0,
            "%s\n\t(entchannel) = %i",
            "(entchannel >= 0 && entchannel < g_snd.entchannel_count)",
            entchannel);
    return g_snd.entchaninfo[entchannel].priority;
}

bool __cdecl SND_IsRestricted(int entchannel)
{
    if (entchannel < 0 || entchannel >= g_snd.entchannel_count)
        MyAssertHandler(
            ".\\snd.cpp",
            200,
            0,
            "%s\n\t(entchannel) = %i",
            "(entchannel >= 0 && entchannel < g_snd.entchannel_count)",
            entchannel);
    return g_snd.entchaninfo[entchannel].isRestricted;
}

bool __cdecl SND_IsAliasChannel3D(int entchannel)
{
    if (entchannel < 0 || entchannel >= g_snd.entchannel_count)
        MyAssertHandler(
            ".\\snd.cpp",
            207,
            0,
            "%s\n\t(entchannel) = %i",
            "(entchannel >= 0 && entchannel < g_snd.entchannel_count)",
            entchannel);
    return g_snd.entchaninfo[entchannel].is3d;
}

bool __cdecl SND_IsPausable(int entchannel)
{
    if (entchannel < 0 || entchannel >= g_snd.entchannel_count)
        MyAssertHandler(
            ".\\snd.cpp",
            214,
            0,
            "%s\n\t(entchannel) = %i",
            "(entchannel >= 0 && entchannel < g_snd.entchannel_count)",
            entchannel);
    return g_snd.entchaninfo[entchannel].isPausable;
}

snd_entchannel_info_t *__cdecl SND_GetEntChannelName(int entchannel)
{
    if (entchannel < 0 || entchannel >= g_snd.entchannel_count)
        MyAssertHandler(
            ".\\snd.cpp",
            221,
            0,
            "%s\n\t(entchannel) = %i",
            "(entchannel >= 0 && entchannel < g_snd.entchannel_count)",
            entchannel);
    return &g_snd.entchaninfo[entchannel];
}

int __cdecl SND_GetEntChannelFromName(const char *channelName)
{
    int chanIdx; // [esp+0h] [ebp-4h]

    if (!channelName)
        MyAssertHandler(".\\snd.cpp", 230, 0, "%s", "channelName");
    for (chanIdx = 0; chanIdx < g_snd.entchannel_count; ++chanIdx)
    {
        if (!I_stricmp(channelName, g_snd.entchaninfo[chanIdx].name))
            return chanIdx;
    }
    return -1;
}

char __cdecl SND_ValidateEnvEffectsPriorityValue(const char *priorityName, int *priority)
{
    const char *priorityStrings[3]; // [esp+0h] [ebp-10h]
    int stringIndex; // [esp+Ch] [ebp-4h]

    priorityStrings[0] = "none";
    priorityStrings[1] = "level";
    priorityStrings[2] = "shellshock";
    if (!priorityName)
        MyAssertHandler(".\\snd.cpp", 255, 0, "%s", "priorityName");
    if (!priority)
        MyAssertHandler(".\\snd.cpp", 256, 0, "%s", "priority");
    for (stringIndex = 1; stringIndex < 3; ++stringIndex)
    {
        if (!I_stricmp(priorityName, priorityStrings[stringIndex]))
        {
            *priority = stringIndex;
            return 1;
        }
    }
    Com_Printf(9, "invalid priority string '%s', it must be one of the following strings:\n", priorityName);
    for (stringIndex = 1; stringIndex < 3; ++stringIndex)
        Com_Printf(9, "  %s\n", priorityStrings[stringIndex]);
    return 0;
}

void __cdecl SND_SetEnvironmentEffects_f()
{
    const char *v0; // eax
    const char *v1; // eax
    const char *v2; // eax
    const char *v3; // eax
    const char *roomstring; // [esp+Ch] [ebp-18h]
    float drylevel; // [esp+14h] [ebp-10h]
    float wetlevel; // [esp+18h] [ebp-Ch]
    int priority; // [esp+1Ch] [ebp-8h] BYREF
    int fademsec; // [esp+20h] [ebp-4h]

    if (Cmd_Argc() == 6)
    {
        v0 = Cmd_Argv(1);
        if (SND_ValidateEnvEffectsPriorityValue(v0, &priority))
        {
            roomstring = Cmd_Argv(2);
            if (SND_RoomtypeFromString(roomstring) || !I_stricmp(roomstring, snd_roomStrings[0]))
            {
                v1 = Cmd_Argv(3);
                drylevel = atof(v1);
                if (drylevel >= 0.0 && drylevel <= 1.0)
                {
                    v2 = Cmd_Argv(4);
                    wetlevel = atof(v2);
                    if (wetlevel >= 0.0 && wetlevel <= 1.0)
                    {
                        v3 = Cmd_Argv(5);
                        fademsec = atoi(v3);
                        if (fademsec >= 0)
                            SND_SetEnvironmentEffects(priority, roomstring, drylevel, wetlevel, fademsec);
                        else
                            Com_Printf(9, "invalid 'fademsec' %i, must be greater than or equal to zero\n", fademsec);
                    }
                    else
                    {
                        Com_Printf(9, "invalid 'wetlevel' %g, must be in the range of 0.0-1.0\n", wetlevel);
                    }
                }
                else
                {
                    Com_Printf(9, "invalid 'drylevel' %g, must be in the range of 0.0-1.0\n", drylevel);
                }
            }
        }
    }
    else
    {
        Com_Printf(
            9,
            "USAGE: snd_setEnvironmentEffects <const char *priority> <const char *roomstring> <float drylevel> <float wetlevel>"
            " <int fademsec>\n");
    }
}

int __cdecl SND_RoomtypeFromString(const char *string)
{
    int stringIndex; // [esp+0h] [ebp-4h]
    int stringIndexa; // [esp+0h] [ebp-4h]

    if (!string)
        MyAssertHandler(".\\snd.cpp", 279, 0, "%s", "string");
    for (stringIndex = 0; snd_roomStrings[stringIndex]; ++stringIndex)
    {
        if (!I_stricmp(string, snd_roomStrings[stringIndex]))
            return stringIndex;
    }
    Com_Printf(9, "invalid roomtype string '%s', it must be one of the following strings:\n", string);
    for (stringIndexa = 0; snd_roomStrings[stringIndexa]; ++stringIndexa)
    {
        if (*snd_roomStrings[stringIndexa])
            Com_Printf(9, "  %s\n", snd_roomStrings[stringIndexa]);
    }
    return 0;
}

void __cdecl SND_DeactivateEnvironmentEffects_f()
{
    const char *v0; // eax
    const char *v1; // eax
    int priority; // [esp+0h] [ebp-8h] BYREF
    int fademsec; // [esp+4h] [ebp-4h]

    if (Cmd_Argc() == 3)
    {
        v0 = Cmd_Argv(1);
        if (SND_ValidateEnvEffectsPriorityValue(v0, &priority))
        {
            v1 = Cmd_Argv(2);
            fademsec = atoi(v1);
            if (fademsec >= 0)
                SND_DeactivateEnvironmentEffects(priority, fademsec);
            else
                Com_Printf(9, "invalid 'fademsec' %i, must be greater than or equal to zero\n", fademsec);
        }
    }
    else
    {
        Com_Printf(9, "USAGE: snd_deactivateEnvironmentEffects <int priority> <int fademsec>\n");
    }
}

void __cdecl SND_SetEq_f()
{
    const char *v0; // eax
    const char *v1; // eax
    const char *v2; // eax
    const char *v3; // eax
    int band; // [esp+Ch] [ebp-1Ch] BYREF
    float q; // [esp+10h] [ebp-18h]
    int entchannel; // [esp+14h] [ebp-14h] BYREF
    int eqIndex; // [esp+18h] [ebp-10h] BYREF
    SND_EQTYPE type; // [esp+1Ch] [ebp-Ch]
    float freq; // [esp+20h] [ebp-8h]
    float gain; // [esp+24h] [ebp-4h]

    if (Cmd_Argc() == 8)
    {
        if (SND_ParseChannelAndBand_f(&entchannel, &eqIndex, &band))
        {
            v0 = Cmd_Argv(4);
            type = (SND_EQTYPE)SND_EqTypeFromString(v0);
            if (type != SND_EQTYPE_COUNT)
            {
                v1 = Cmd_Argv(5);
                gain = atof(v1);
                v2 = Cmd_Argv(6);
                freq = atof(v2);
                if (freq >= 0.0 && freq <= 20000.0)
                {
                    v3 = Cmd_Argv(7);
                    q = atof(v3);
                    if (q > 0.0)
                        SND_SetEqParams(entchannel, eqIndex, band, type, gain, freq, q);
                    else
                        Com_Printf(9, "invalid 'q' %f, must be > 0\n", q);
                }
                else
                {
                    Com_Printf(9, "invalid 'freq' %f, must be >= 0 and <= %i\n", freq, 20000);
                }
            }
        }
    }
    else
    {
        Com_Printf(
            9,
            "USAGE: snd_setEq <const char *channelName> <int eqIndex> <int band> <const char *type> <float gain> <float freq> <float q>\n");
        SND_PrintEqParams();
    }
}

SND_EQTYPE __cdecl SND_EqTypeFromString(const char *typeString)
{
    int stringIndex; // [esp+0h] [ebp-4h]
    int stringIndexa; // [esp+0h] [ebp-4h]

    if (!typeString)
        MyAssertHandler(".\\snd.cpp", 375, 0, "%s", "typeString");
    for (stringIndex = 0; snd_eqTypeStrings[stringIndex]; ++stringIndex)
    {
        if (!I_stricmp(typeString, snd_eqTypeStrings[stringIndex]))
            return (SND_EQTYPE)stringIndex;
    }
    Com_Printf(9, "invalid eq type string '%s', it must be one of the following strings:\n", typeString);
    for (stringIndexa = 0; snd_eqTypeStrings[stringIndexa]; ++stringIndexa)
    {
        if (*snd_eqTypeStrings[stringIndexa])
            Com_Printf(9, "  %s\n", snd_eqTypeStrings[stringIndexa]);
    }
    return (SND_EQTYPE)5;
}

char __cdecl SND_ParseChannelAndBand_f(int *entchannel, int *eqIndex, int *band)
{
    const char *v4; // eax
    const char *v5; // eax
    const char *channelName; // [esp+0h] [ebp-4h]

    if (!entchannel)
        MyAssertHandler(".\\snd.cpp", 398, 0, "%s", "entchannel");
    if (!band)
        MyAssertHandler(".\\snd.cpp", 399, 0, "%s", "band");
    if (!eqIndex)
        MyAssertHandler(".\\snd.cpp", 400, 0, "%s", "eqIndex");
    channelName = Cmd_Argv(1);
    *entchannel = SND_GetEntChannelFromName(channelName);
    if (*entchannel >= 0)
    {
        v4 = Cmd_Argv(2);
        *eqIndex = atoi(v4);
        if ((unsigned int)*eqIndex < 2)
        {
            v5 = Cmd_Argv(3);
            *band = atoi(v5);
            if ((unsigned int)*band <= 2)
            {
                return 1;
            }
            else
            {
                Com_Printf(9, "invalid 'band' %i, must be >= 0 and < %i\n", *band, 3);
                return 0;
            }
        }
        else
        {
            Com_Printf(9, "invalid 'eqIndex' %i, must be >= 0 and < %i\n", *eqIndex, 2);
            return 0;
        }
    }
    else
    {
        Com_Printf(9, "Unknown channel name (%s), please check channel definitions file\n", channelName);
        return 0;
    }
}

void __cdecl SND_SetEqFreq_f()
{
    const char *v0; // eax
    int band; // [esp+Ch] [ebp-10h] BYREF
    int entchannel; // [esp+10h] [ebp-Ch] BYREF
    int eqIndex; // [esp+14h] [ebp-8h] BYREF
    float freq; // [esp+18h] [ebp-4h]

    if (Cmd_Argc() == 5)
    {
        if (SND_ParseChannelAndBand_f(&entchannel, &eqIndex, &band))
        {
            v0 = Cmd_Argv(4);
            freq = atof(v0);
            if (freq >= 0.0 && freq <= 20000.0)
                SND_SetEqFreq(entchannel, eqIndex, band, freq);
            else
                Com_Printf(9, "invalid 'freq' %f, must be >= 0 and < %i\n", freq, 20000);
        }
    }
    else
    {
        Com_Printf(9, "USAGE: snd_setEqFreq <const char *channelName> <int eqIndex> <int band> <float freq>\n");
    }
}

void __cdecl SND_SetEqType_f()
{
    const char *v0; // eax
    int band; // [esp+0h] [ebp-10h] BYREF
    int entchannel; // [esp+4h] [ebp-Ch] BYREF
    int eqIndex; // [esp+8h] [ebp-8h] BYREF
    SND_EQTYPE type; // [esp+Ch] [ebp-4h]

    if (Cmd_Argc() == 5)
    {
        if (SND_ParseChannelAndBand_f(&entchannel, &eqIndex, &band))
        {
            v0 = Cmd_Argv(4);
            type = SND_EqTypeFromString(v0);
            if (type != SND_EQTYPE_COUNT)
                SND_SetEqType(entchannel, eqIndex, band, type);
        }
    }
    else
    {
        Com_Printf(9, "USAGE: snd_setEqType <const char *channelName> <int eqIndex> <int band> <const char *type>\n");
    }
}

void __cdecl SND_SetEqGain_f()
{
    const char *v0; // eax
    int band; // [esp+4h] [ebp-10h] BYREF
    int entchannel; // [esp+8h] [ebp-Ch] BYREF
    int eqIndex; // [esp+Ch] [ebp-8h] BYREF
    float gain; // [esp+10h] [ebp-4h]

    if (Cmd_Argc() == 5)
    {
        if (SND_ParseChannelAndBand_f(&entchannel, &eqIndex, &band))
        {
            v0 = Cmd_Argv(4);
            gain = atof(v0);
            SND_SetEqGain(entchannel, eqIndex, band, gain);
        }
    }
    else
    {
        Com_Printf(9, "USAGE: snd_setEqGain <const char *channelName> <int eqIndex> <int band> <float gain>\n");
    }
}

void __cdecl SND_SetEqQ_f()
{
    const char *v0; // eax
    int band; // [esp+8h] [ebp-10h] BYREF
    float q; // [esp+Ch] [ebp-Ch]
    int entchannel; // [esp+10h] [ebp-8h] BYREF
    int eqIndex; // [esp+14h] [ebp-4h] BYREF

    if (Cmd_Argc() == 5)
    {
        if (SND_ParseChannelAndBand_f(&entchannel, &eqIndex, &band))
        {
            v0 = Cmd_Argv(4);
            q = atof(v0);
            if (q > 0.0)
                SND_SetEqQ(entchannel, eqIndex, band, q);
            else
                Com_Printf(9, "invalid 'q' %f, must be > 0\n", q);
        }
    }
    else
    {
        Com_Printf(9, "USAGE: snd_setEqQ <const char *channelName> <int eqIndex> <int band> <float q>\n");
    }
}

void __cdecl SND_DeactivateEq_f()
{
    const char *v0; // eax
    const char *v1; // eax
    const char *channelName; // [esp+4h] [ebp-10h]
    const char *channelNamea; // [esp+4h] [ebp-10h]
    unsigned int band; // [esp+8h] [ebp-Ch]
    unsigned int eqIndex; // [esp+Ch] [ebp-8h]
    int argc; // [esp+10h] [ebp-4h]

    argc = Cmd_Argc();
    if (argc >= 2 && argc <= 4)
    {
        v0 = Cmd_Argv(1);
        eqIndex = atoi(v0);
        if (eqIndex < 2)
        {
            if (argc == 2)
            {
                SND_DeactivateAllEq(eqIndex);
            }
            else if (argc == 3)
            {
                channelNamea = Cmd_Argv(2);
                SND_DeactivateChannelEq(channelNamea, eqIndex);
            }
            else
            {
                channelName = Cmd_Argv(2);
                v1 = Cmd_Argv(3);
                band = atoi(v1);
                if (band <= 2)
                    SND_DeactivateEq(channelName, eqIndex, band);
                else
                    Com_Printf(9, "invalid 'band' %i, must be >= 0 and < %i\n", band, 3);
            }
        }
        else
        {
            Com_Printf(9, "invalid 'eqIndex' %i, must be >= 0 and < %i\n", eqIndex, 2);
        }
    }
    else
    {
        Com_Printf(9, "USAGE: snd_deactivateEq <int eqIndex> [(optional) const char *channelName] [(optional) int band]\n");
    }
}

char __cdecl SND_AnyActiveListeners()
{
    int listenerIndex; // [esp+0h] [ebp-4h]

    for (listenerIndex = 0; listenerIndex < 2; ++listenerIndex)
    {
        if (g_snd.listeners[listenerIndex].active)
            return 1;
    }
    return 0;
}

int __cdecl SND_GetListenerIndexNearestToOrigin(const float *origin)
{
    float v2; // [esp+4h] [ebp-20h]
    float diff[3]; // [esp+8h] [ebp-1Ch] BYREF
    int nearest; // [esp+14h] [ebp-10h]
    float dist[2]; // [esp+18h] [ebp-Ch]
    int i; // [esp+20h] [ebp-4h]

    if (!origin)
        MyAssertHandler(".\\snd.cpp", 648, 0, "%s", "origin");
    for (i = 0; i < 2; ++i)
    {
        if (g_snd.listeners[i].active)
        {
            Vec3Sub(g_snd.listeners[i].orient.origin, origin, diff);
            v2 = Vec3LengthSq(diff);
        }
        else
        {
            v2 = 3.4028235e38;
        }
        dist[i] = v2;
    }
    if (g_snd.amplifier.listener->active && dist[1] > (double)(g_snd.amplifier.maxRadius * g_snd.amplifier.maxRadius))
        dist[1] = 3.4028235e38;
    i = 1;
    nearest = 0;
    while (i < 2)
    {
        if (dist[i] < (double)dist[nearest])
            nearest = i;
        ++i;
    }
    return nearest;
}

void __cdecl SND_DisconnectListener(int localClientNum)
{
    if (localClientNum)
        MyAssertHandler(
            ".\\snd.cpp",
            669,
            0,
            "localClientNum doesn't index STATIC_MAX_LOCAL_CLIENTS\n\t%i not in [0, %i)",
            localClientNum,
            1);
    memset((unsigned __int8 *)&g_snd.listeners[localClientNum], 0, sizeof(g_snd.listeners[localClientNum]));
}

void __cdecl SND_SetListener(int localClientNum, int clientNum, const float *origin, const float (*axis)[3])
{
    snd_listener *v4; // [esp+0h] [ebp-4h]

    if (g_snd.Initialized2d)
    {
        if (!origin)
            MyAssertHandler(".\\snd.cpp", 680, 0, "%s", "origin");
        if (!axis)
            MyAssertHandler(".\\snd.cpp", 681, 0, "%s", "axis");
        if (localClientNum)
            MyAssertHandler(
                ".\\snd.cpp",
                682,
                0,
                "localClientNum doesn't index STATIC_MAX_LOCAL_CLIENTS\n\t%i not in [0, %i)",
                localClientNum,
                1);
        AxisCopy(*(const mat3x3 *)axis, g_snd.listeners[localClientNum].orient.axis);
        v4 = &g_snd.listeners[localClientNum];
        v4->orient.origin[0] = *origin;
        v4->orient.origin[1] = origin[1];
        v4->orient.origin[2] = origin[2];
        g_snd.listeners[localClientNum].clientNum = clientNum;
        g_snd.listeners[localClientNum].active = 1;
        if (g_snd.amplifier.listener->active)
            AxisCopy(*(const mat3x3*)axis, g_snd.amplifier.listener->orient.axis);
    }
}

void __cdecl SND_SaveListeners(snd_listener *listeners)
{
    if (!listeners)
        MyAssertHandler(".\\snd.cpp", 697, 0, "%s", "listeners");
    memcpy(listeners, g_snd.listeners, 0x70u);
}

void __cdecl SND_RestoreListeners(snd_listener *listeners)
{
    if (!listeners)
        MyAssertHandler(".\\snd.cpp", 705, 0, "%s", "listeners");
    memcpy(g_snd.listeners, listeners, sizeof(g_snd.listeners));
}

int __cdecl SND_SetPlaybackIdNotPlayed(unsigned int index)
{
    if (index > 0x34)
        MyAssertHandler(".\\snd.cpp", 713, 0, "%s", "index >= 0 && index < SND_MAX_CHANNELS");
    g_snd.chaninfo[index].playbackId = -1;
    return -1;
}

int __cdecl SND_AcquirePlaybackId(unsigned int index, int totalMsec)
{
    snd_channel_info_t *chanInfo; // [esp+0h] [ebp-4h]

    if (index > 0x34)
        MyAssertHandler(".\\snd.cpp", 725, 0, "%s", "index >= 0 && index < SND_MAX_CHANNELS");
    chanInfo = &g_snd.chaninfo[index];
    if ((chanInfo->alias0->flags & 1) != 0)
    {
        chanInfo->totalMsec = 0;
        chanInfo->playbackId = 0;
    }
    else
    {
        chanInfo->totalMsec = totalMsec;
        chanInfo->playbackId = g_snd.playbackIdCounter++;
    }
    if (g_snd.playbackIdCounter < 1)
        g_snd.playbackIdCounter = 1;
    return chanInfo->playbackId;
}

char __cdecl SND_AddLengthNotify(int playbackId, const snd_alias_t *lengthNotifyData, SndLengthId id)
{
    snd_channel_info_t *chanInfo; // [esp+0h] [ebp-Ch]
    int lengthNotifyIndex; // [esp+4h] [ebp-8h]
    int chanInfoIndex; // [esp+8h] [ebp-4h]

    if ((unsigned int)id >= SndLengthNotifyCount)
        MyAssertHandler(".\\snd.cpp", 776, 0, "id doesn't index SndLengthNotifyCount\n\t%i not in [0, %i)", id, 2);
    if (playbackId != -1 && playbackId)
    {
        for (chanInfoIndex = 0; chanInfoIndex < 53 && g_snd.chaninfo[chanInfoIndex].playbackId != playbackId; ++chanInfoIndex)
            ;
        if (chanInfoIndex == 53)
        {
            return 0;
        }
        else
        {
            chanInfo = &g_snd.chaninfo[chanInfoIndex];
            if (chanInfo->totalMsec < 0)
            {
                for (lengthNotifyIndex = 0; lengthNotifyIndex < chanInfo->lengthNotifyInfo.count; ++lengthNotifyIndex)
                {
                    if (chanInfo->lengthNotifyInfo.id[lengthNotifyIndex] == id
                        && (const snd_alias_t *)chanInfo->lengthNotifyInfo.data[lengthNotifyIndex] == lengthNotifyData)
                    {
                        return 1;
                    }
                }
                if ((unsigned int)lengthNotifyIndex >= 4)
                    MyAssertHandler(
                        ".\\snd.cpp",
                        806,
                        0,
                        "lengthNotifyIndex doesn't index SND_LENGTHNOTIFY_COUNT\n\t%i not in [0, %i)",
                        lengthNotifyIndex,
                        4);
                ++chanInfo->lengthNotifyInfo.count;
                chanInfo->lengthNotifyInfo.id[lengthNotifyIndex] = id;
                chanInfo->lengthNotifyInfo.data[lengthNotifyIndex] = (void *)lengthNotifyData;
            }
            else
            {
                DoLengthNotify(chanInfo->totalMsec, lengthNotifyData, id);
            }
            return 1;
        }
    }
    else
    {
        DoLengthNotify(0, lengthNotifyData, id);
        return 1;
    }
}

void __cdecl DoLengthNotify(int msec, const snd_alias_t *lengthNotifyData, SndLengthId id)
{
    const char *v3; // eax

    if (id)
    {
        if (id == SndLengthNotify_Subtitle)
        {
            CG_SubtitleSndLengthNotify(msec, lengthNotifyData);
        }
        else if (!alwaysfails)
        {
            v3 = va("Unknown snd length notify id: %i\n", id);
            MyAssertHandler(".\\snd.cpp", 762, 0, v3);
        }
    }
}

char __cdecl SND_GetKnownLength(int playbackId, int *msec)
{
    int chanInfoIndex; // [esp+0h] [ebp-4h]

    if (!msec)
        MyAssertHandler(".\\snd.cpp", 822, 0, "%s", "msec");
    *msec = 0;
    if (playbackId == -1 || !playbackId)
        return 1;
    for (chanInfoIndex = 0; chanInfoIndex < 53 && g_snd.chaninfo[chanInfoIndex].playbackId != playbackId; ++chanInfoIndex)
        ;
    if (chanInfoIndex == 53)
        return 0;
    if (g_snd.chaninfo[chanInfoIndex].totalMsec < 0)
        return 0;
    *msec = g_snd.chaninfo[chanInfoIndex].totalMsec;
    return 1;
}

double __cdecl SND_GetLerpedSlavePercentage(float baseSlavePercentage)
{
    return (float)(1.0 - (1.0 - baseSlavePercentage) * g_snd.slaveLerp);
}

double __cdecl SND_Attenuate(SndCurve *volumeFalloffCurve, float radius, float mindist, float maxdist)
{
    float radiusa; // [esp+10h] [ebp+Ch]
    float radiusb; // [esp+10h] [ebp+Ch]

    if (!volumeFalloffCurve)
        MyAssertHandler(".\\snd.cpp", 877, 0, "%s", "volumeFalloffCurve");
    radiusa = radius - mindist;
    if (radiusa <= 0.0)
        return 1.0;
    if (mindist >= (double)maxdist)
        MyAssertHandler(".\\snd.cpp", 883, 0, "%s", "maxdist > mindist");
    radiusb = radiusa / (maxdist - mindist);
    if (radiusb < 1.0)
        return Com_GetVolumeFalloffCurveValue(volumeFalloffCurve, radiusb);
    else
        return 0.0;
}

void __cdecl SND_GetCurrent3DPosition(SndEntHandle sndEnt, float *offset, float *pos_out)
{
    float org[3]; // [esp+Ch] [ebp-30h] BYREF
    float axis[3][3]; // [esp+18h] [ebp-24h] BYREF

    if (!offset)
        MyAssertHandler(".\\snd.cpp", 898, 0, "%s", "offset");
    if (!pos_out)
        MyAssertHandler(".\\snd.cpp", 899, 0, "%s", "pos_out");
    CG_GetSoundEntityOrientation(sndEnt, org, axis);
    Vec3Mad(org, *offset, axis[0], org);
    Vec3Mad(org, offset[1], axis[1], org);
    Vec3Mad(org, offset[2], axis[2], org);
    *pos_out = org[0];
    pos_out[1] = org[1];
    pos_out[2] = org[2];
}

void __cdecl SND_ResetChannelInfo(int index)
{
    if (g_snd.chaninfo[index].entchannel < 0 || g_snd.chaninfo[index].entchannel >= g_snd.entchannel_count)
        MyAssertHandler(
            ".\\snd.cpp",
            911,
            0,
            "%s\n\t(g_snd.chaninfo[index].entchannel) = %i",
            "(g_snd.chaninfo[index].entchannel >= 0 && g_snd.chaninfo[index].entchannel < g_snd.entchannel_count)",
            g_snd.chaninfo[index].entchannel);
    g_snd.chaninfo[index].paused = 0;
    g_snd.chaninfo[index].startDelay = 0;
    g_snd.chaninfo[index].soundFileInfo.loadingState = SFLS_UNLOADED;
    g_snd.chaninfo[index].alias0 = 0;
}

void __cdecl SND_SetChannelStartInfo(unsigned int index, SndStartAliasInfo *SndStartAliasInfo)
{
    double v2; // st7
    double v3; // st7
    double v4; // st7
    sndLengthNotifyInfo *p_lengthNotifyInfo; // ecx
    bool v6; // [esp+0h] [ebp-48h]
    float *v7; // [esp+4h] [ebp-44h]
    float offset[3]; // [esp+8h] [ebp-40h] BYREF
    float org[3]; // [esp+14h] [ebp-34h] BYREF
    float axis[3][3]; // [esp+20h] [ebp-28h] BYREF
    snd_channel_info_t *chanInfo; // [esp+44h] [ebp-4h]

    if (index > 0x34)
        MyAssertHandler(".\\snd.cpp", 925, 0, "%s", "index >= 0 && index < SND_MAX_CHANNELS");
    if (SndStartAliasInfo->system > (unsigned int)SASYS_GAME)
        MyAssertHandler(
            ".\\snd.cpp",
            926,
            0,
            "%s\n\t(SndStartAliasInfo->system) = %i",
            "(SndStartAliasInfo->system >= 0 && SndStartAliasInfo->system < SASYS_COUNT)",
            SndStartAliasInfo->system);
    if (!SndStartAliasInfo->alias0)
        MyAssertHandler(".\\snd.cpp", 927, 0, "%s", "SndStartAliasInfo->alias0");
    if (!SndStartAliasInfo->alias1)
        MyAssertHandler(".\\snd.cpp", 928, 0, "%s", "SndStartAliasInfo->alias1");
    chanInfo = &g_snd.chaninfo[index];
    if (SndStartAliasInfo->sndEnt.field.entIndex != 0xFFFF
        && SND_IsAliasChannel3D((SndStartAliasInfo->alias0->flags & 0x3F00) >> 8))
    {
        CG_GetSoundEntityOrientation(SndStartAliasInfo->sndEnt, org, axis);
        Vec3Sub(SndStartAliasInfo->org, org, offset);
        v2 = Vec3Dot(offset, axis[0]);
        chanInfo->offset[0] = v2;
        v3 = Vec3Dot(offset, axis[1]);
        chanInfo->offset[1] = v3;
        v4 = Vec3Dot(offset, axis[2]);
        chanInfo->offset[2] = v4;
    }
    else
    {
        v7 = chanInfo->offset;
        chanInfo->offset[0] = 0.0;
        v7[1] = 0.0;
        v7[2] = 0.0;
    }
    chanInfo->sndEnt.field.entIndex = SndStartAliasInfo->sndEnt.field.entIndex;
    chanInfo->entchannel = (SndStartAliasInfo->alias0->flags & 0x3F00) >> 8;
    chanInfo->basevolume = SndStartAliasInfo->volume;
    chanInfo->pitch = SndStartAliasInfo->pitch;
    chanInfo->alias0 = SndStartAliasInfo->alias0;
    chanInfo->alias1 = SndStartAliasInfo->alias1;
    chanInfo->lerp = SndStartAliasInfo->lerp;
    chanInfo->startDelay = SndStartAliasInfo->startDelay;
    chanInfo->looptime = g_snd.looptime;
    v6 = g_snd.paused && g_snd.pauseSettings[(SndStartAliasInfo->alias0->flags & 0x3F00) >> 8];
    chanInfo->paused = v6;
    chanInfo->master = SndStartAliasInfo->master;
    chanInfo->system = SndStartAliasInfo->system;
    chanInfo->startTime = g_snd.time + chanInfo->startDelay;
    chanInfo->timescale = SndStartAliasInfo->timescale;
    p_lengthNotifyInfo = &chanInfo->lengthNotifyInfo;
    chanInfo->lengthNotifyInfo.id[0] = SndLengthNotify_Script;
    p_lengthNotifyInfo->id[1] = SndLengthNotify_Script;
    p_lengthNotifyInfo->id[2] = SndLengthNotify_Script;
    p_lengthNotifyInfo->id[3] = SndLengthNotify_Script;
    p_lengthNotifyInfo->data[0] = 0;
    p_lengthNotifyInfo->data[1] = 0;
    p_lengthNotifyInfo->data[2] = 0;
    p_lengthNotifyInfo->data[3] = 0;
    p_lengthNotifyInfo->count = 0;
}

void __cdecl SND_SetSoundFileChannelInfo(
    unsigned int index,
    int srcChannelCount,
    int baserate,
    int total_msec,
    int start_msec,
    SndFileLoadingState loadingState)
{
    if (index > 0x34)
        MyAssertHandler(".\\snd.cpp", 971, 0, "%s", "index >= 0 && index < SND_MAX_CHANNELS");
    g_snd.chaninfo[index].soundFileInfo.loadingState = loadingState;
    g_snd.chaninfo[index].soundFileInfo.srcChannelCount = srcChannelCount;
    g_snd.chaninfo[index].soundFileInfo.baserate = baserate;
    g_snd.chaninfo[index].soundFileInfo.endtime = total_msec + g_snd.time - start_msec;
}

int __cdecl SND_FindFree2DChannel(SndStartAliasInfo *startAliasInfo, int entchannel)
{
    snd_entchannel_info_t *EntChannelName; // eax
    bool HasFreeVoice; // al
    int v5; // eax
    bool v6; // al
    int v7; // eax
    bool v8; // al
    int v9; // eax
    int v10; // [esp-Ch] [ebp-10h]
    const snd_alias_t *alias0; // [esp-8h] [ebp-Ch]
    const char *aliasName; // [esp-8h] [ebp-Ch]
    const snd_alias_t *v13; // [esp-8h] [ebp-Ch]
    const char *v14; // [esp-8h] [ebp-Ch]
    const snd_alias_t *v15; // [esp-8h] [ebp-Ch]
    const char *v16; // [esp-8h] [ebp-Ch]
    const char *v17; // [esp-4h] [ebp-8h]
    int Priority; // [esp-4h] [ebp-8h]
    const char *v19; // [esp-4h] [ebp-8h]
    int v20; // [esp-4h] [ebp-8h]
    const char *v21; // [esp-4h] [ebp-8h]
    int v22; // [esp-4h] [ebp-8h]
    int i; // [esp+0h] [ebp-4h]
    int ia; // [esp+0h] [ebp-4h]

    EntChannelName = SND_GetEntChannelName(entchannel);
    v17 = va("Max voices reached for entchannel %s", EntChannelName->name);
    alias0 = startAliasInfo->alias0;
    HasFreeVoice = SND_HasFreeVoice(entchannel);
    SND_DebugAliasPrint(!HasFreeVoice, alias0, v17);
    if (!SND_HasFreeVoice(entchannel))
        return -1;
    for (i = 0; i < g_snd.max_2D_channels; ++i)
    {
        if (SND_Is2DChannelFree(i))
            return i;
    }
    ia = SND_FindReplaceableChannel(startAliasInfo, entchannel, 0, g_snd.max_2D_channels);
    if (ia >= 0)
    {
        if (!SND_Is2DChannelFree(ia))
        {
            Priority = SND_GetPriority(g_snd.chaninfo[ia].entchannel);
            aliasName = g_snd.chaninfo[ia].alias0->aliasName;
            v5 = SND_GetPriority(entchannel);
            v19 = va("(prio %i) => Replacing '%s' (prio: %i)", v5, aliasName, Priority);
            v13 = startAliasInfo->alias0;
            v6 = SND_Is2DChannelFree(ia);
            SND_DebugAliasPrint(!v6, v13, v19);
            v20 = SND_GetPriority(entchannel);
            v14 = startAliasInfo->alias0->aliasName;
            v7 = SND_GetPriority(g_snd.chaninfo[ia].entchannel);
            v21 = va("(prio %i ) => Replaced by '%s' (prio: %i)", v7, v14, v20);
            v15 = g_snd.chaninfo[ia].alias0;
            v8 = SND_Is2DChannelFree(ia);
            SND_DebugAliasPrint(!v8, v15, v21);
            if (snd_debugReplace->current.enabled
                && ((g_snd.chaninfo[ia].alias0->flags & 1) != 0
                    || g_snd.chaninfo[ia].totalMsec + g_snd.chaninfo[ia].startTime - g_snd.time > 0))
            {
                v22 = SND_GetPriority(entchannel);
                v16 = startAliasInfo->alias0->aliasName;
                v10 = g_snd.chaninfo[ia].totalMsec + g_snd.chaninfo[ia].startTime - g_snd.time;
                v9 = SND_GetPriority(g_snd.chaninfo[ia].entchannel);
                Com_Printf(
                    14,
                    "Stopping 2d sound channel that's playing '%s' (prio: %i, %ims left) so we can play '%s' (prio: %i) instead\n",
                    g_snd.chaninfo[ia].alias0->aliasName,
                    v9,
                    v10,
                    v16,
                    v22);
            }
        }
        SND_Stop2DChannel(ia);
    }
    SND_DebugAliasPrint(ia < 0, startAliasInfo->alias0, "No free channels");
    return ia;
}

int __cdecl SND_FindReplaceableChannel(
    SndStartAliasInfo *startAliasInfo,
    int entchannel,
    unsigned int first,
    int count)
{
    int Priority; // eax
    float v6; // [esp+0h] [ebp-70h]
    snd_listener *v7; // [esp+4h] [ebp-6Ch]
    float v8[3]; // [esp+8h] [ebp-68h] BYREF
    float *origin; // [esp+14h] [ebp-5Ch]
    float v[3]; // [esp+18h] [ebp-58h] BYREF
    float *a; // [esp+24h] [ebp-4Ch]
    float diff[3]; // [esp+28h] [ebp-48h] BYREF
    int timeLeft; // [esp+34h] [ebp-3Ch]
    int newSubtitle; // [esp+38h] [ebp-38h]
    int chanprio; // [esp+3Ch] [ebp-34h]
    bool is3d; // [esp+43h] [ebp-2Dh]
    float minMetric; // [esp+44h] [ebp-2Ch]
    int replaceable; // [esp+48h] [ebp-28h]
    float org[3]; // [esp+4Ch] [ebp-24h] BYREF
    const snd_alias_t *alias; // [esp+58h] [ebp-18h]
    int i; // [esp+5Ch] [ebp-14h]
    int prio; // [esp+60h] [ebp-10h]
    bool applySameSndEntCondition; // [esp+67h] [ebp-9h]
    snd_channel_info_t *chaninfo; // [esp+68h] [ebp-8h]
    float metric; // [esp+6Ch] [ebp-4h]

    if (!startAliasInfo)
        MyAssertHandler(".\\snd.cpp", 996, 0, "%s", "startAliasInfo");
    if (first > 0x34)
        MyAssertHandler(
            ".\\snd.cpp",
            997,
            0,
            "%s\n\t(first) = %i",
            "(first >= 0 && first < (32 + (SND_TRACK_COUNT + 8) + 8))",
            first);
    if (count < 0 || count >(int)(53 - first))
        MyAssertHandler(
            ".\\snd.cpp",
            998,
            0,
            "%s\n\t(count) = %i",
            "(count >= 0 && count <= (32 + (SND_TRACK_COUNT + 8) + 8) - first)",
            count);
    if (entchannel < 0 || entchannel >= g_snd.entchannel_count)
        MyAssertHandler(
            ".\\snd.cpp",
            999,
            0,
            "%s\n\t(entchannel) = %i",
            "(entchannel >= 0 && entchannel < g_snd.entchannel_count)",
            entchannel);
    applySameSndEntCondition = 0;
    is3d = SND_IsAliasChannel3D(entchannel);
    prio = SND_GetPriority(entchannel);
    if (is3d)
    {
        a = g_snd.listeners[SND_GetListenerIndexNearestToOrigin(startAliasInfo->org)].orient.origin;
        Vec3Sub(a, startAliasInfo->org, diff);
        v6 = Vec3LengthSq(diff);
    }
    else
    {
        v6 = -startAliasInfo->volume;
    }
    minMetric = v6;
    replaceable = -1;
    if (!startAliasInfo->alias0)
        MyAssertHandler(".\\snd.cpp", 1007, 0, "%s", "startAliasInfo->alias0");
    newSubtitle = startAliasInfo->alias0->subtitle != 0;
    for (i = first; i < (int)(count + first); ++i)
    {
        if (i >= 53)
            MyAssertHandler(".\\snd.cpp", 1012, 0, "%s", "i < SND_MAX_CHANNELS");
        chaninfo = &g_snd.chaninfo[i];
        if (!chaninfo)
            MyAssertHandler(".\\snd.cpp", 1014, 0, "%s", "chaninfo");
        alias = chaninfo->alias0;
        if (!alias)
            MyAssertHandler(".\\snd.cpp", 1017, 0, "%s", "alias");
        timeLeft = chaninfo->totalMsec + chaninfo->startTime - g_snd.time;
        if ((alias->flags & 1) == 0 && timeLeft <= 0)
            return i;
        if (newSubtitle || !alias->subtitle)
        {
            chanprio = SND_GetPriority(chaninfo->entchannel);
            Priority = SND_GetPriority(entchannel);
            if (chanprio <= Priority
                && (!applySameSndEntCondition || chaninfo->sndEnt.field.entIndex == startAliasInfo->sndEnt.field.entIndex))
            {
                if ((applySameSndEntCondition || chaninfo->sndEnt.field.entIndex != startAliasInfo->sndEnt.field.entIndex)
                    && chanprio >= prio)
                {
                    if (chanprio == prio)
                    {
                        if (is3d)
                        {
                            SND_GetCurrent3DPosition(chaninfo->sndEnt, chaninfo->offset, org);
                            v7 = &g_snd.listeners[SND_GetListenerIndexNearestToOrigin(org)];
                            Vec3Sub(v7->orient.origin, org, v8);
                            metric = Vec3LengthSq(v8);
                        }
                        else
                        {
                            metric = -chaninfo->basevolume;
                        }
                        if (minMetric < (double)metric)
                        {
                            replaceable = i;
                            minMetric = metric;
                        }
                    }
                }
                else
                {
                    applySameSndEntCondition = chaninfo->sndEnt.field.entIndex == startAliasInfo->sndEnt.field.entIndex;
                    prio = chanprio;
                    replaceable = i;
                    if (is3d)
                    {
                        SND_GetCurrent3DPosition(chaninfo->sndEnt, chaninfo->offset, org);
                        origin = g_snd.listeners[SND_GetListenerIndexNearestToOrigin(org)].orient.origin;
                        Vec3Sub(origin, org, v);
                        minMetric = Vec3LengthSq(v);
                    }
                    else
                    {
                        minMetric = -chaninfo->basevolume;
                    }
                }
            }
        }
    }
    return replaceable;
}

int __cdecl SND_FindFree3DChannel(SndStartAliasInfo *startAliasInfo, int entchannel)
{
    int v3; // eax
    bool v4; // al
    int v5; // eax
    bool v6; // al
    int v7; // eax
    int v8; // [esp-4h] [ebp-48h]
    const char *v9; // [esp+8h] [ebp-3Ch]
    int v10; // [esp+Ch] [ebp-38h]
    const char *aliasName; // [esp+10h] [ebp-34h]
    const snd_alias_t *alias0; // [esp+10h] [ebp-34h]
    const char *v13; // [esp+10h] [ebp-34h]
    const snd_alias_t *v14; // [esp+10h] [ebp-34h]
    double v15; // [esp+10h] [ebp-34h]
    int Priority; // [esp+14h] [ebp-30h]
    const char *v17; // [esp+14h] [ebp-30h]
    int v18; // [esp+14h] [ebp-30h]
    const char *v19; // [esp+14h] [ebp-30h]
    float v20; // [esp+18h] [ebp-2Ch]
    snd_listener *v21; // [esp+1Ch] [ebp-28h]
    float v[3]; // [esp+20h] [ebp-24h] BYREF
    float v23; // [esp+2Ch] [ebp-18h]
    float *a; // [esp+30h] [ebp-14h]
    float diff[3]; // [esp+34h] [ebp-10h] BYREF
    int i; // [esp+40h] [ebp-4h]

    if (!SND_HasFreeVoice(entchannel))
        return -1;
    for (i = 0; i < g_snd.max_3D_channels; ++i)
    {
        if (SND_Is3DChannelFree(i + 8))
            return i + 8;
    }
    i = SND_FindReplaceableChannel(startAliasInfo, entchannel, 8u, g_snd.max_3D_channels);
    if (i >= 0)
    {
        if (!SND_Is3DChannelFree(i))
        {
            Priority = SND_GetPriority(g_snd.chaninfo[i].entchannel);
            aliasName = g_snd.chaninfo[i].alias0->aliasName;
            v3 = SND_GetPriority(entchannel);
            v17 = va("(prio %i) => Replacing '%s' (prio: %i)", v3, aliasName, Priority);
            alias0 = startAliasInfo->alias0;
            v4 = SND_Is3DChannelFree(i);
            SND_DebugAliasPrint(!v4, alias0, v17);
            v18 = SND_GetPriority(entchannel);
            v13 = startAliasInfo->alias0->aliasName;
            v5 = SND_GetPriority(g_snd.chaninfo[i].entchannel);
            v19 = va("(prio %i ) => Replaced by '%s' (prio: %i)", v5, v13, v18);
            v14 = g_snd.chaninfo[i].alias0;
            v6 = SND_Is3DChannelFree(i);
            SND_DebugAliasPrint(!v6, v14, v19);
            if (snd_debugReplace->current.enabled
                && ((g_snd.chaninfo[i].alias0->flags & 1) != 0
                    || g_snd.chaninfo[i].totalMsec + g_snd.chaninfo[i].startTime - g_snd.time > 0))
            {
                a = g_snd.listeners[SND_GetListenerIndexNearestToOrigin(startAliasInfo->org)].orient.origin;
                Vec3Sub(a, startAliasInfo->org, diff);
                v23 = Vec3LengthSq(diff);
                v21 = &g_snd.listeners[SND_GetListenerIndexNearestToOrigin(g_snd.chaninfo[i].org)];
                Vec3Sub(v21->orient.origin, g_snd.chaninfo[i].org, v);
                v20 = Vec3LengthSq(v);
                v15 = v23;
                v10 = SND_GetPriority(entchannel);
                v9 = startAliasInfo->alias0->aliasName;
                v8 = g_snd.chaninfo[i].totalMsec + g_snd.chaninfo[i].startTime - g_snd.time;
                v7 = SND_GetPriority(g_snd.chaninfo[i].entchannel);
                Com_DPrintf(
                    14,
                    "Stopping 3d sound channel that's playing '%s' (prio: %i, %ims left, dist: %f) so we can play '%s' (prio: %i, d"
                    "ist: %f) instead\n",
                    g_snd.chaninfo[i].alias0->aliasName,
                    v7,
                    v8,
                    v20,
                    v9,
                    v10,
                    v15);
            }
        }
        SND_Stop3DChannel(i);
    }
    SND_DebugAliasPrint(i < 0, startAliasInfo->alias0, "No free channels");
    return i;
}

void __cdecl DB_SaveSounds()
{
    int i; // [esp+0h] [ebp-4h]

    for (i = 0; i < 53; ++i)
        SND_Archive(&g_snd.chaninfo[i]);
}

void __cdecl SND_Archive(snd_channel_info_t *chaninfo)
{
    snd_alias_list_t *aliasList; // [esp+8h] [ebp-8h]
    snd_alias_list_t *aliasLista; // [esp+8h] [ebp-8h]
    const snd_alias_t *alias; // [esp+Ch] [ebp-4h]
    const snd_alias_t *aliasa; // [esp+Ch] [ebp-4h]

    alias = chaninfo->alias0;
    if (alias)
    {
        aliasList = DB_FindXAssetHeader(ASSET_TYPE_SOUND, alias->aliasName).sound;
        if (!aliasList)
            MyAssertHandler(".\\snd.cpp", 1328, 0, "%s", "aliasList");
        chaninfo->saveIndex0 = SND_GetAliasOffset(alias);
        chaninfo->alias0 = (const snd_alias_t *)aliasList;
    }
    aliasa = chaninfo->alias1;
    if (aliasa)
    {
        aliasLista = DB_FindXAssetHeader(ASSET_TYPE_SOUND, aliasa->aliasName).sound;
        if (!aliasLista)
            MyAssertHandler(".\\snd.cpp", 1338, 0, "%s", "aliasList");
        chaninfo->saveIndex1 = SND_GetAliasOffset(aliasa);
        chaninfo->alias1 = (const snd_alias_t *)aliasLista;
    }
}

void __cdecl DB_LoadSounds()
{
    int i; // [esp+0h] [ebp-4h]

    for (i = 0; i < 53; ++i)
        SND_Unarchive(&g_snd.chaninfo[i]);
}

void __cdecl SND_Unarchive(snd_channel_info_t *chaninfo)
{
    snd_alias_list_t *aliasList; // [esp+0h] [ebp-4h]
    snd_alias_list_t *aliasLista; // [esp+0h] [ebp-4h]

    aliasList = (snd_alias_list_t *)chaninfo->alias0;
    if (aliasList)
    {
        if (!aliasList->head)
            MyAssertHandler(".\\snd.cpp", 1353, 0, "%s", "aliasList->head");
        chaninfo->alias0 = SND_GetAliasWithOffset(aliasList->aliasName, chaninfo->saveIndex0);
    }
    aliasLista = (snd_alias_list_t *)chaninfo->alias1;
    if (aliasLista)
    {
        if (!aliasLista->head)
            MyAssertHandler(".\\snd.cpp", 1364, 0, "%s", "aliasList->head");
        chaninfo->alias1 = SND_GetAliasWithOffset(aliasLista->aliasName, chaninfo->saveIndex1);
    }
}

void __cdecl SND_StopSoundAliasOnEnt(SndEntHandle sndEnt, const char *aliasName)
{
    StopSoundAliasesOnEnt(sndEnt, aliasName);
}

void __cdecl StopSoundAliasesOnEnt(SndEntHandle sndEnt, const char *aliasName)
{
    int v2; // eax
    int v3; // eax
    const snd_alias_t *alias0; // [esp-8h] [ebp-14h]
    const snd_alias_t *alias1; // [esp-8h] [ebp-14h]
    void(__cdecl * stopChannel)(int); // [esp+0h] [ebp-Ch]
    int chanIdx; // [esp+4h] [ebp-8h]
    snd_channel_info_t *chaninfo; // [esp+8h] [ebp-4h]

    if (g_snd.Initialized2d)
    {
        stopChannel = 0;
        for (chanIdx = 0; ; ++chanIdx)
        {
            if (chanIdx >= 53)
                return;
            chaninfo = &g_snd.chaninfo[chanIdx];
            if (chaninfo->sndEnt.field.entIndex == sndEnt.field.entIndex)
            {
                if (chanIdx < 0 || chanIdx >= g_snd.max_2D_channels)
                {
                    if (chanIdx < 8 || chanIdx >= g_snd.max_3D_channels + 8)
                    {
                        if (chanIdx >= 40 && chanIdx < g_snd.max_stream_channels + 40)
                        {
                            if (SND_IsStreamChannelFree(chanIdx))
                                continue;
                            stopChannel = SND_StopStreamChannel;
                        }
                    }
                    else
                    {
                        if (SND_Is3DChannelFree(chanIdx))
                            continue;
                        stopChannel = SND_Stop3DChannel;
                    }
                }
                else
                {
                    if (SND_Is2DChannelFree(chanIdx))
                        continue;
                    stopChannel = SND_Stop2DChannel;
                }
                if (!stopChannel)
                    MyAssertHandler(".\\snd.cpp", 1738, 1, "%s", "stopChannel");
                if (aliasName)
                {
                    if (chaninfo->alias0 && !I_stricmp(chaninfo->alias0->aliasName, aliasName)
                        || chaninfo->alias1 && !I_stricmp(chaninfo->alias1->aliasName, aliasName))
                    {
                        alias0 = chaninfo->alias0;
                        v2 = I_stricmp(alias0->aliasName, aliasName);
                        SND_DebugAliasPrint(v2 == 0, alias0, "stopped on entity by name");
                        alias1 = chaninfo->alias1;
                        v3 = I_stricmp(alias1->aliasName, aliasName);
                        SND_DebugAliasPrint(v3 == 0, alias1, "stopped on entity by name");
                        stopChannel(chanIdx);
                    }
                }
                else
                {
                    SND_DebugAliasPrint(1, chaninfo->alias0, "stopped all on entity");
                    stopChannel(chanIdx);
                }
            }
        }
    }
}

void __cdecl SND_StopSoundsOnEnt(SndEntHandle sndEnt)
{
    StopSoundAliasesOnEnt(sndEnt, 0);
}

void __cdecl SND_InitFXSounds()
{
    g_FXPlaySoundCount = 0;
}

void __cdecl SND_AddPlayFXSoundAlias(snd_alias_t *alias, SndEntHandle sndEnt, const float *origin)
{
    AsyncPlaySound *sound; // [esp+0h] [ebp-4h]

    if (g_FXPlaySoundCount == 32)
    {
        Com_PrintError(20, "ERROR: too many FX sounds %d\n", 32);
    }
    else
    {
        sound = &g_FXPlaySounds[g_FXPlaySoundCount];
        sound->alias = alias;
        sound->sndEnt = sndEnt;
        sound->origin[0] = *origin;
        sound->origin[1] = origin[1];
        sound->origin[2] = origin[2];
        ++g_FXPlaySoundCount;
    }
}

void __cdecl Snd_AssertAliasValid(snd_alias_t *alias)
{
    if (!alias)
        MyAssertHandler(".\\snd.cpp", 1798, 0, "%s", "alias");
    if ((unsigned int)((alias->flags & 0x3F00) >> 8) >= g_snd.entchannel_count)
        MyAssertHandler(
            ".\\snd.cpp",
            1799,
            0,
            "SNDALIASFLAGS_GET_CHANNEL( alias->flags ) doesn't index g_snd.entchannel_count\n\t%i not in [0, %i)",
            (alias->flags & 0x3F00) >> 8,
            g_snd.entchannel_count);
}

void __cdecl SND_PlayFXSounds()
{
    unsigned int soundIndex; // [esp+4h] [ebp-4h]

    for (soundIndex = 0; soundIndex < g_FXPlaySoundCount; ++soundIndex)
        SND_PlaySoundAlias(
            g_FXPlaySounds[soundIndex].alias,
            g_FXPlaySounds[soundIndex].sndEnt,
            g_FXPlaySounds[soundIndex].origin,
            0,
            SASYS_CGAME);
}

int __cdecl SND_PlaySoundAlias(
    const snd_alias_t *alias,
    SndEntHandle sndEnt,
    const float *org,
    int timeshift,
    snd_alias_system_t system)
{
    iassert(org);

    if (alias)
        return SND_PlaySoundAlias_Internal(alias, alias, 0.0, 1.0, sndEnt, org, 0, timeshift, 0, 1, system);
    else
        return -1;
}

int __cdecl SND_PlaySoundAlias_Internal(
    const snd_alias_t *alias0,
    const snd_alias_t *alias1,
    float lerp,
    float volumeScale,
    SndEntHandle sndEnt,
    const float *org,
    int *pChannel,
    int timeshift,
    bool treatAsMaster,
    bool useTimescale,
    snd_alias_system_t system)
{
    const char *v12; // eax
    const char *v13; // eax
    int v14; // [esp+24h] [ebp-7Ch]
    bool v15; // [esp+28h] [ebp-78h]
    float v16; // [esp+2Ch] [ebp-74h]
    snd_listener *a; // [esp+38h] [ebp-68h]
    float diff[3]; // [esp+3Ch] [ebp-64h] BYREF
    snd_alias_t *tertiaryAlias; // [esp+48h] [ebp-58h]
    int playbackId; // [esp+4Ch] [ebp-54h]
    SndStartAliasInfo startAliasInfo; // [esp+50h] [ebp-50h] BYREF
    int alias0Channel; // [esp+88h] [ebp-18h]
    snd_alias_t *secondaryAlias; // [esp+8Ch] [ebp-14h]
    float distListenerSq; // [esp+90h] [ebp-10h]
    bool outOfRange; // [esp+97h] [ebp-9h]
    int secondaryAliasRecursionCounter; // [esp+98h] [ebp-8h]
    float distMax; // [esp+9Ch] [ebp-4h]

    if (!alias0)
        MyAssertHandler(".\\snd.cpp", 1542, 0, "%s", "alias0");
    if (!alias1)
        MyAssertHandler(".\\snd.cpp", 1543, 0, "%s", "alias1");
    if (!org)
        MyAssertHandler(".\\snd.cpp", 1544, 0, "%s", "org");
    playbackId = -1;
    outOfRange = 0;
    if (!g_snd.Initialized2d)
        return playbackId;
    if (pChannel)
        *pChannel = -1;
    alias0Channel = (alias0->flags & 0x3F00) >> 8;
    if (SND_IsAliasChannel3D(alias0Channel))
    {
        distMax = (1.0 - lerp) * alias0->distMax + alias1->distMax * lerp;
        a = &g_snd.listeners[SND_GetListenerIndexNearestToOrigin(org)];
        Vec3Sub(a->orient.origin, org, diff);
        distListenerSq = Vec3LengthSq(diff);
        outOfRange = distListenerSq > distMax * distMax;
        if (*(_BYTE *)snd_debugAlias->current.integer)
        {
            v16 = sqrt(distListenerSq);
            v12 = va("Not playing, out of range: %.1f > %.1f", v16, distMax);
            SND_DebugAliasPrint(outOfRange, alias0, v12);
        }
    }
    if (!outOfRange)
    {
        if (SND_ContinueLoopingSound(alias0, alias1, lerp, volumeScale, sndEnt, org, pChannel))
        {
            if (alias0->secondaryAliasName)
            {
                secondaryAlias = Com_PickSoundAlias(alias0->secondaryAliasName);
                if (secondaryAlias)
                {
                    if ((secondaryAlias->flags & 1) != 0)
                        SND_PlaySoundAlias_Internal(
                            secondaryAlias,
                            secondaryAlias,
                            lerp,
                            volumeScale,
                            sndEnt,
                            org,
                            0,
                            timeshift,
                            treatAsMaster,
                            useTimescale,
                            system);
                }
            }
            return 0;
        }
        if (SND_IsRestricted(alias0Channel))
            SND_StopEntityChannel(sndEnt, alias0Channel);
        if (SND_IsNullSoundFile(alias0->soundFile))
            return -1;
        SND_ChoosePitchAndVolume(alias0, alias1, lerp, volumeScale, &startAliasInfo.volume, &startAliasInfo.pitch);
        startAliasInfo.alias0 = alias0;
        startAliasInfo.alias1 = alias1;
        startAliasInfo.lerp = lerp;
        startAliasInfo.sndEnt = sndEnt;
        startAliasInfo.org[0] = *org;
        startAliasInfo.org[1] = org[1];
        startAliasInfo.org[2] = org[2];
        startAliasInfo.timeshift = timeshift;
        startAliasInfo.fraction = 0.0;
        startAliasInfo.startDelay = alias0->startDelay;
        v15 = treatAsMaster || (alias0->flags & 2) != 0;
        startAliasInfo.master = v15;
        startAliasInfo.timescale = useTimescale;
        startAliasInfo.system = system;
        v14 = (alias0->flags & 0xC0) >> 6;
        if (v14 == 1)
        {
            playbackId = SND_StartAliasSample(&startAliasInfo, pChannel);
        }
        else if (v14 == 2)
        {
            playbackId = SND_StartAliasStream(&startAliasInfo, pChannel);
        }
        else
        {
            if (!alwaysfails)
            {
                v13 = va("unhandled sound alias type %i", (alias0->flags & 0xC0) >> 6);
                MyAssertHandler(".\\snd.cpp", 1609, 0, v13);
            }
            playbackId = -1;
        }
    }
    if (alias0->secondaryAliasName)
    {
        secondaryAlias = Com_PickSoundAlias(alias0->secondaryAliasName);
        if (secondaryAlias)
        {
            if ((alias0->flags & 1) != 0 || (secondaryAlias->flags & 1) == 0)
            {
                if ((alias0->flags & 1) != 0
                    && (secondaryAlias->flags & 1) != 0
                    && !Com_AliasNameRefersToSingleAlias(alias0->secondaryAliasName))
                {
                    Com_PrintError(
                        9,
                        "Error: a looping alias cannot have a looping secondary aliasName that refers to multiple aliases.\n"
                        "Alias sequence: '%s'->'%s'\n",
                        alias0->aliasName,
                        alias0->secondaryAliasName);
                }
            }
            else
            {
                Com_PrintError(
                    9,
                    "Error: a non-looping alias cannot have a looping secondary alias.\nAlias sequence: '%s'->'%s'\n",
                    alias0->aliasName,
                    alias0->secondaryAliasName);
            }
            secondaryAliasRecursionCounter = 0;
            for (tertiaryAlias = secondaryAlias;
                tertiaryAlias && tertiaryAlias->secondaryAliasName && secondaryAliasRecursionCounter < 10;
                tertiaryAlias = Com_PickSoundAlias(tertiaryAlias->secondaryAliasName))
            {
                if (!I_stricmp(alias0->aliasName, tertiaryAlias->secondaryAliasName))
                {
                    Com_PrintError(
                        9,
                        "Error: Infinite recursion in secondary aliases sequenced together.\nAlias sequence start: '%s'->'%s'\n",
                        alias0->aliasName,
                        alias0->secondaryAliasName);
                    return -1;
                }
                ++secondaryAliasRecursionCounter;
            }
            SND_PlaySoundAlias_Internal(
                secondaryAlias,
                secondaryAlias,
                lerp,
                volumeScale,
                sndEnt,
                org,
                0,
                timeshift,
                treatAsMaster,
                useTimescale,
                system);
        }
        else
        {
            Com_PrintError(9, "Error: unable to find '%s' alias\n", alias0->secondaryAliasName);
        }
    }
    SND_DebugAliasPrint(playbackId != -1, alias0, "Started");
    return playbackId;
}

void __cdecl SND_StopEntityChannel(SndEntHandle sndEnt, int entchannel)
{
    int i; // [esp+0h] [ebp-4h]
    int ia; // [esp+0h] [ebp-4h]
    int ib; // [esp+0h] [ebp-4h]

    for (i = 8; i < g_snd.max_3D_channels + 8; ++i)
    {
        if (g_snd.chaninfo[i].sndEnt.field.entIndex == sndEnt.field.entIndex
            && g_snd.chaninfo[i].entchannel == entchannel
            && !SND_Is3DChannelFree(i))
        {
            SND_DebugAliasPrint(1, g_snd.chaninfo[i].alias0, "stopped on entchannel");
            SND_Stop3DChannel(i);
        }
    }
    for (ia = 40; ia < g_snd.max_stream_channels + 40; ++ia)
    {
        if (g_snd.chaninfo[ia].sndEnt.field.entIndex == sndEnt.field.entIndex
            && g_snd.chaninfo[ia].entchannel == entchannel
            && !SND_IsStreamChannelFree(ia))
        {
            SND_StopStreamChannel(ia);
        }
    }
    for (ib = 0; ib < g_snd.max_2D_channels; ++ib)
    {
        if (g_snd.chaninfo[ib].sndEnt.field.entIndex == sndEnt.field.entIndex
            && g_snd.chaninfo[ib].entchannel == entchannel
            && !SND_Is2DChannelFree(ib))
        {
            SND_Stop2DChannel(ib);
        }
    }
}

int __cdecl SND_StartAliasSample(SndStartAliasInfo *startAliasInfo, int *pChannel)
{
    char filename[132]; // [esp+0h] [ebp-88h] BYREF

    if (!startAliasInfo->alias0)
        MyAssertHandler(".\\snd.cpp", 1237, 0, "%s", "startAliasInfo->alias0");
    if ((startAliasInfo->alias0->flags & 0xC0) >> 6 != 1)
        MyAssertHandler(
            ".\\snd.cpp",
            1238,
            0,
            "%s",
            "SNDALIASFLAGS_GET_TYPE( startAliasInfo->alias0->flags ) == SAT_LOADED");
    if (!startAliasInfo->alias1)
        MyAssertHandler(".\\snd.cpp", 1239, 0, "%s", "startAliasInfo->alias1");
    if ((startAliasInfo->alias1->flags & 0xC0) >> 6 != 1)
        MyAssertHandler(
            ".\\snd.cpp",
            1240,
            0,
            "%s",
            "SNDALIASFLAGS_GET_TYPE( startAliasInfo->alias1->flags ) == SAT_LOADED");
    if (startAliasInfo->alias0->soundFile->exists)
    {
        if (SND_IsAliasChannel3D((startAliasInfo->alias0->flags & 0x3F00) >> 8))
        {
            if (snd_enable3D->current.enabled)
            {
                if (SND_AnyActiveListeners())
                {
                    return SND_StartAlias3DSample(startAliasInfo, pChannel);
                }
                else
                {
                    Com_Error(
                        ERR_DROP,
                        "attempted to play spatialized alias '%s' while there is no active listener. Most likely this means you tried"
                        " to play a spatialized sound while not in a level.\n",
                        startAliasInfo->alias0->aliasName);
                    return 0;
                }
            }
            else
            {
                return 0;
            }
        }
        else if (snd_enable2D->current.enabled)
        {
            return SND_StartAlias2DSample(startAliasInfo, pChannel);
        }
        else
        {
            return 0;
        }
    }
    else
    {
        Com_GetSoundFileName(startAliasInfo->alias0, filename, 128);
        Com_DPrintf(
            9,
            "Tried to play sound '%s' from alias '%s', but it was not successfully loaded.\n",
            filename,
            startAliasInfo->alias0->aliasName);
        if (pChannel)
            *pChannel = -1;
        return 0;
    }
}

int __cdecl SND_StartAliasStream(SndStartAliasInfo *startAliasInfo, int *pChannel)
{
    int index; // [esp+4h] [ebp-4h]

    if (!startAliasInfo->alias0)
        MyAssertHandler(".\\snd.cpp", 1292, 0, "%s", "startAliasInfo->alias0");
    if ((startAliasInfo->alias0->flags & 0xC0) >> 6 != 2)
        MyAssertHandler(
            ".\\snd.cpp",
            1293,
            0,
            "%s",
            "SNDALIASFLAGS_GET_TYPE( startAliasInfo->alias0->flags ) == SAT_STREAMED");
    if (!startAliasInfo->alias1)
        MyAssertHandler(".\\snd.cpp", 1294, 0, "%s", "startAliasInfo->alias1");
    if ((startAliasInfo->alias1->flags & 0xC0) >> 6 != 2)
        MyAssertHandler(
            ".\\snd.cpp",
            1295,
            0,
            "%s",
            "SNDALIASFLAGS_GET_TYPE( startAliasInfo->alias1->flags ) == SAT_STREAMED");
    index = SND_FindFreeStreamChannel(startAliasInfo, (startAliasInfo->alias0->flags & 0x3F00) >> 8);
    if (pChannel)
        *pChannel = index;
    if (index < 0)
        return -1;
    if (index < 40 || index >= g_snd.max_stream_channels + 40)
        MyAssertHandler(
            ".\\snd.cpp",
            1305,
            0,
            "%s\n\t(index) = %i",
            "(index >= ((0 + 8) + 32) && index < ((0 + 8) + 32) + g_snd.max_stream_channels)",
            index);
    if (!snd_enableStream->current.enabled)
        return -1;
    if (SND_IsAliasChannel3D((startAliasInfo->alias0->flags & 0x3F00) >> 8) && !SND_AnyActiveListeners())
        Com_Error(
            ERR_DROP,
            "attempted to play spatialized alias '%s' while there is no active listener. Most likely this means you tried to pl"
            "ay a spatialized sound while not in a level.\n",
            startAliasInfo->alias0->aliasName);
    return SND_StartAliasStreamOnChannel(startAliasInfo, index);
}

int __cdecl SND_FindFreeStreamChannel(SndStartAliasInfo *startAliasInfo, int entchannel)
{
    int v3; // eax
    const char *v4; // eax
    int v5; // eax
    const char *v6; // eax
    int v7; // eax
    int v8; // [esp-4h] [ebp-74h]
    const char *v9; // [esp+8h] [ebp-68h]
    int v10; // [esp+Ch] [ebp-64h]
    const char *aliasName; // [esp+10h] [ebp-60h]
    const char *v12; // [esp+10h] [ebp-60h]
    double v13; // [esp+10h] [ebp-60h]
    int Priority; // [esp+14h] [ebp-5Ch]
    int v15; // [esp+14h] [ebp-5Ch]
    double v16; // [esp+18h] [ebp-58h]
    float v17; // [esp+20h] [ebp-50h]
    snd_listener *v18; // [esp+24h] [ebp-4Ch]
    float v19[3]; // [esp+28h] [ebp-48h] BYREF
    float v20; // [esp+34h] [ebp-3Ch]
    float *a; // [esp+38h] [ebp-38h]
    float v22[3]; // [esp+3Ch] [ebp-34h] BYREF
    float v[3]; // [esp+48h] [ebp-28h] BYREF
    float v24; // [esp+54h] [ebp-1Ch]
    float diff[4]; // [esp+58h] [ebp-18h] BYREF
    float *listenerOrg; // [esp+68h] [ebp-8h]
    int i; // [esp+6Ch] [ebp-4h]

    if (!SND_HasFreeVoice(entchannel))
        return -1;
    for (i = 5; i < g_snd.max_stream_channels; ++i)
    {
        if (SND_IsStreamChannelFree(i + 40))
            return i + 40;
    }
    i = SND_FindReplaceableChannel(startAliasInfo, entchannel, 0x2Du, g_snd.max_stream_channels - 5);
    if (i >= 0)
    {
        if (g_snd.chaninfo[i].alias0 == startAliasInfo->alias0)
        {
            diff[3] = 0.0;
            listenerOrg = g_snd.listeners[0].orient.origin;
            Vec3Sub(g_snd.listeners[0].orient.origin, startAliasInfo->org, diff);
            v24 = Vec3LengthSq(diff);
            Vec3Sub(listenerOrg, g_snd.chaninfo[i].org, v);
            v16 = v24;
            if (Vec3LengthSq(v) <= v16)
                i = -1;
            else
                SND_StopStreamChannel(i);
        }
        else
        {
            SND_StopStreamChannel(i);
        }
        if (i >= 0 && !SND_IsStreamChannelFree(i))
        {
            Priority = SND_GetPriority(g_snd.chaninfo[i].entchannel);
            aliasName = g_snd.chaninfo[i].alias0->aliasName;
            v3 = SND_GetPriority(entchannel);
            v4 = va("(prio %i) => Replacing '%s' (prio: %i)", v3, aliasName, Priority);
            SND_DebugAliasPrint(1, startAliasInfo->alias0, v4);
            v15 = SND_GetPriority(entchannel);
            v12 = startAliasInfo->alias0->aliasName;
            v5 = SND_GetPriority(g_snd.chaninfo[i].entchannel);
            v6 = va("(prio %i ) => Replaced by '%s' (prio: %i)", v5, v12, v15);
            SND_DebugAliasPrint(1, g_snd.chaninfo[i].alias0, v6);
            if (snd_debugReplace->current.enabled
                && ((g_snd.chaninfo[i].alias0->flags & 1) != 0
                    || g_snd.chaninfo[i].totalMsec + g_snd.chaninfo[i].startTime - g_snd.time > 0))
            {
                a = g_snd.listeners[SND_GetListenerIndexNearestToOrigin(startAliasInfo->org)].orient.origin;
                Vec3Sub(a, startAliasInfo->org, v22);
                v20 = Vec3LengthSq(v22);
                v18 = &g_snd.listeners[SND_GetListenerIndexNearestToOrigin(g_snd.chaninfo[i].org)];
                Vec3Sub(v18->orient.origin, g_snd.chaninfo[i].org, v19);
                v17 = Vec3LengthSq(v19);
                v13 = v20;
                v10 = SND_GetPriority(entchannel);
                v9 = startAliasInfo->alias0->aliasName;
                v8 = g_snd.chaninfo[i].totalMsec + g_snd.chaninfo[i].startTime - g_snd.time;
                v7 = SND_GetPriority(g_snd.chaninfo[i].entchannel);
                Com_DPrintf(
                    9,
                    "Stopping stream sound channel that's playing '%s' (prio: %i, %ims left, dist: %f) so we can play '%s' (prio: %"
                    "i, dist: %f) instead\n",
                    g_snd.chaninfo[i].alias0->aliasName,
                    v7,
                    v8,
                    v17,
                    v9,
                    v10,
                    v13);
            }
        }
    }
    SND_DebugAliasPrint(i < 0, startAliasInfo->alias0, "No free channels");
    return i;
}

void __cdecl SND_ChoosePitchAndVolume(
    const snd_alias_t *alias0,
    const snd_alias_t *alias1,
    float lerp,
    float volumeScale,
    float *volume,
    float *pitch)
{
    float v6; // [esp+8h] [ebp-2Ch]
    float v7; // [esp+Ch] [ebp-28h]
    float v8; // [esp+10h] [ebp-24h]
    float v9; // [esp+1Ch] [ebp-18h]
    float v10; // [esp+20h] [ebp-14h]
    float volMax; // [esp+24h] [ebp-10h]
    float pitchMax; // [esp+28h] [ebp-Ch]
    float volMin; // [esp+2Ch] [ebp-8h]
    float pitchMin; // [esp+30h] [ebp-4h]

    if (!alias0)
        MyAssertHandler(".\\snd.cpp", 1401, 0, "%s", "alias0");
    if (!alias1)
        MyAssertHandler(".\\snd.cpp", 1402, 0, "%s", "alias1");
    if (!volume)
        MyAssertHandler(".\\snd.cpp", 1403, 0, "%s", "volume");
    if (!pitch)
        MyAssertHandler(".\\snd.cpp", 1404, 0, "%s", "pitch");
    volMin = (1.0 - lerp) * alias0->volMin + alias1->volMin * lerp;
    volMax = (1.0 - lerp) * alias0->volMax + alias1->volMax * lerp;
    pitchMin = (1.0 - lerp) * alias0->pitchMin + alias1->pitchMin * lerp;
    pitchMax = (1.0 - lerp) * alias0->pitchMax + alias1->pitchMax * lerp;
    *volume = random() * (volMax - volMin) + volMin;
    *volume = *volume * volumeScale;
    v9 = *volume;
    v8 = v9 - 1.0;
    if (v8 < 0.0)
        v10 = v9;
    else
        v10 = 1.0;
    v7 = 0.0 - v9;
    if (v7 < 0.0)
        v6 = v10;
    else
        v6 = 0.0;
    *volume = v6;
    *pitch = random() * (pitchMax - pitchMin) + pitchMin;
}

char __cdecl SND_ContinueLoopingSound(
    const snd_alias_t *alias0,
    const snd_alias_t *alias1,
    float lerp,
    float volumeScale,
    SndEntHandle sndEnt,
    const float *org,
    int *pChannel)
{
    float *v8; // [esp+10h] [ebp-8Ch]
    int i; // [esp+98h] [ebp-4h]
    signed int ia; // [esp+98h] [ebp-4h]
    int ib; // [esp+98h] [ebp-4h]

    for (i = 8; i < g_snd.max_3D_channels + 8; ++i)
    {
        if (g_snd.chaninfo[i].sndEnt.field.entIndex == sndEnt.field.entIndex && !SND_Is3DChannelFree(i))
        {
            if (!g_snd.chaninfo[i].alias0)
                MyAssertHandler(".\\snd.cpp", 1456, 0, "%s", "g_snd.chaninfo[i].alias0");
            if (!g_snd.chaninfo[i].alias1)
                MyAssertHandler(".\\snd.cpp", 1457, 0, "%s", "g_snd.chaninfo[i].alias1");
            if ((g_snd.chaninfo[i].alias0->flags & 1) != 0
                && g_snd.chaninfo[i].alias0->aliasName == alias0->aliasName
                && g_snd.chaninfo[i].alias1->aliasName == alias1->aliasName)
            {
                SND_ContinueLoopingSound_Internal(i, lerp, volumeScale, pChannel, SND_Set3DChannelPlaybackRate);
                SND_Set3DPosition(i, org);
                return 1;
            }
        }
    }
    for (ia = 0; ia < g_snd.max_2D_channels; ++ia)
    {
        if (g_snd.chaninfo[ia].sndEnt.field.entIndex == sndEnt.field.entIndex && !SND_Is2DChannelFree(ia))
        {
            if (!g_snd.chaninfo[ia].alias0)
                MyAssertHandler(".\\snd.cpp", 1477, 0, "%s", "g_snd.chaninfo[i].alias0");
            if (!g_snd.chaninfo[ia].alias1)
                MyAssertHandler(".\\snd.cpp", 1478, 0, "%s", "g_snd.chaninfo[i].alias1");
            if ((g_snd.chaninfo[ia].alias0->flags & 1) != 0
                && g_snd.chaninfo[ia].alias0->aliasName == alias0->aliasName
                && g_snd.chaninfo[ia].alias1->aliasName == alias1->aliasName)
            {
                SND_ContinueLoopingSound_Internal(ia, lerp, volumeScale, pChannel, SND_Set2DChannelPlaybackRate);
                return 1;
            }
        }
    }
    for (ib = 40; ib < g_snd.max_stream_channels + 40; ++ib)
    {
        if (g_snd.chaninfo[ib].sndEnt.field.entIndex == sndEnt.field.entIndex && !SND_IsStreamChannelFree(ib))
        {
            if (!g_snd.chaninfo[ib].alias0)
                MyAssertHandler(".\\snd.cpp", 1497, 0, "%s", "g_snd.chaninfo[i].alias0");
            if (!g_snd.chaninfo[ib].alias1)
                MyAssertHandler(".\\snd.cpp", 1498, 0, "%s", "g_snd.chaninfo[i].alias1");
            if ((g_snd.chaninfo[ib].alias0->flags & 1) != 0
                && g_snd.chaninfo[ib].alias0->aliasName == alias0->aliasName
                && g_snd.chaninfo[ib].alias1->aliasName == alias1->aliasName)
            {
                SND_ContinueLoopingSound_Internal(ib, lerp, volumeScale, pChannel, SND_SetStreamChannelPlaybackRate);
                v8 = g_snd.chaninfo[ib].org;
                *v8 = *org;
                v8[1] = org[1];
                v8[2] = org[2];
                return 1;
            }
        }
    }
    return 0;
}

void __cdecl SND_ContinueLoopingSound_Internal(
    unsigned int chanIndex,
    float lerp,
    float volumeScale,
    int *pChannel,
    void(__cdecl *setPlaybackRateFunc)(int, int))
{
    double v5; // st7
    float v6; // [esp+8h] [ebp-40h]
    float v7; // [esp+Ch] [ebp-3Ch]
    float v8; // [esp+10h] [ebp-38h]
    float v9; // [esp+14h] [ebp-34h]
    float v10; // [esp+18h] [ebp-30h]
    float v11; // [esp+1Ch] [ebp-2Ch]
    float v12; // [esp+2Ch] [ebp-1Ch]
    float basevolume; // [esp+3Ch] [ebp-Ch]
    float v14; // [esp+40h] [ebp-8h]

    if (chanIndex > 0x34)
        MyAssertHandler(
            ".\\snd.cpp",
            1422,
            0,
            "%s\n\t(chanIndex) = %i",
            "(chanIndex >= 0 && chanIndex < (32 + (SND_TRACK_COUNT + 8) + 8))",
            chanIndex);
    if (lerp < 0.0 || lerp > 1.0)
        MyAssertHandler(".\\snd.cpp", 1423, 0, "%s\n\t(lerp) = %g", "(lerp >= 0 && lerp <= 1)", lerp);
    if (!setPlaybackRateFunc)
        MyAssertHandler(".\\snd.cpp", 1424, 0, "%s", "setPlaybackRateFunc");
    v10 = (1.0 - lerp) * g_snd.chaninfo[chanIndex].alias0->volMin + g_snd.chaninfo[chanIndex].alias1->volMin * lerp;
    g_snd.chaninfo[chanIndex].basevolume = v10;
    g_snd.chaninfo[chanIndex].basevolume = g_snd.chaninfo[chanIndex].basevolume * volumeScale;
    basevolume = g_snd.chaninfo[chanIndex].basevolume;
    v9 = basevolume - 1.0;
    if (v9 < 0.0)
        v14 = basevolume;
    else
        v14 = 1.0;
    v8 = 0.0 - basevolume;
    if (v8 < 0.0)
        v7 = v14;
    else
        v7 = 0.0;
    g_snd.chaninfo[chanIndex].basevolume = v7;
    v6 = (1.0 - lerp) * g_snd.chaninfo[chanIndex].alias0->pitchMin + g_snd.chaninfo[chanIndex].alias1->pitchMin * lerp;
    g_snd.chaninfo[chanIndex].pitch = v6;
    if (g_snd.chaninfo[chanIndex].timescale)
    {
        v12 = g_snd.timescale * ((double)g_snd.chaninfo[chanIndex].soundFileInfo.baserate * g_snd.chaninfo[chanIndex].pitch);
        v5 = v12 + 9.313225746154785e-10;
    }
    else
    {
        v11 = (double)g_snd.chaninfo[chanIndex].soundFileInfo.baserate * g_snd.chaninfo[chanIndex].pitch;
        v5 = v11 + 9.313225746154785e-10;
    }
    setPlaybackRateFunc(chanIndex, (int)v5);
    g_snd.chaninfo[chanIndex].looptime = g_snd.looptime;
    g_snd.chaninfo[chanIndex].lerp = lerp;
    if (pChannel)
        *pChannel = chanIndex;
}

BOOL __cdecl StreamFileNameIsNullSound(const StreamFileName *streamFileName)
{
    return !*streamFileName->info.raw.dir && !I_stricmp("null.wav", streamFileName->info.raw.name);
}

bool __cdecl SND_IsNullSoundFile(const SoundFile *soundFile)
{
    if (soundFile->type == 2)
        return StreamFileNameIsNullSound((const StreamFileName *)&soundFile->u);
    if (soundFile->type != 1)
        MyAssertHandler(".\\snd.cpp", 1522, 0, "soundFile->type == SAT_LOADED\n\t%i, %i", soundFile->type, 1);
    return I_stricmp("null.wav", soundFile->u.loadSnd->name) == 0;
}

int __cdecl SND_PlaySoundAliasAsMaster(
    const snd_alias_t *alias,
    SndEntHandle sndEnt,
    const float *org,
    int timeshift,
    snd_alias_system_t system)
{
    if (!org)
        MyAssertHandler(".\\snd.cpp", 1829, 0, "%s", "org");
    if (alias)
        return SND_PlaySoundAlias_Internal(alias, alias, 0.0, 1.0, sndEnt, org, 0, timeshift, 1, 1, system);
    else
        return -1;
}

int __cdecl SND_PlayBlendedSoundAliases(
    const snd_alias_t *alias0,
    const snd_alias_t *alias1,
    float lerp,
    float volumeScale,
    SndEntHandle sndEnt,
    const float *org,
    int timeshift,
    snd_alias_system_t system)
{
    if (!org)
        MyAssertHandler(".\\snd.cpp", 1958, 0, "%s", "org");
    if (!alias0 || !alias1)
        return -1;
    SND_ValidateSoundAliasBlend(alias0, alias1, 1);
    return SND_PlaySoundAlias_Internal(alias0, alias1, lerp, volumeScale, sndEnt, org, 0, timeshift, 0, 1, system);
}

char __cdecl SND_ValidateSoundAliasBlend(const snd_alias_t *alias0, const snd_alias_t *alias1, bool bReport)
{
    if (!alias0)
        MyAssertHandler(".\\snd.cpp", 1839, 0, "%s", "alias0");
    if (!alias1)
        MyAssertHandler(".\\snd.cpp", 1840, 0, "%s", "alias1");
    if (alias0 == alias1)
        return 1;
    if (alias0->soundFile == alias1->soundFile)
    {
        if (alias0->volumeFalloffCurve == alias1->volumeFalloffCurve)
        {
            if ((alias0->flags & 1) == (alias1->flags & 1))
            {
                if ((alias0->flags & 0xC0) >> 6 == (alias1->flags & 0xC0) >> 6)
                {
                    if ((alias0->flags & 0x3F00) >> 8 == (alias1->flags & 0x3F00) >> 8)
                    {
                        if ((alias0->flags & 2) == (alias1->flags & 2))
                        {
                            if ((alias0->flags & 4) == (alias1->flags & 4))
                            {
                                if ((alias0->flags & 8) == (alias1->flags & 8))
                                {
                                    if ((alias0->flags & 0x10) == (alias1->flags & 0x10))
                                    {
                                        if (alias1->slavePercentage == alias0->slavePercentage)
                                        {
                                            if (alias0->startDelay == alias1->startDelay)
                                            {
                                                if ((alias0->flags & 1) == 0
                                                    || alias0->pitchMax == alias0->pitchMin && alias1->pitchMax == alias1->pitchMin)
                                                {
                                                    if ((alias0->flags & 1) == 0
                                                        || alias0->volMax == alias0->volMin && alias1->volMax == alias1->volMin)
                                                    {
                                                        if (alias0->secondaryAliasName || alias1->secondaryAliasName)
                                                        {
                                                            if (bReport)
                                                                Com_Error(ERR_DROP, "tried to blend between sound aliases %s and %s, but one or both has a secondary alias", alias0->aliasName, alias1->aliasName);
                                                            return 0;
                                                        }
                                                        else
                                                        {
                                                            return 1;
                                                        }
                                                    }
                                                    else
                                                    {
                                                        if (bReport)
                                                            Com_Error(ERR_DROP, "tried to blend between sound aliases %s and %s, but they are looping and at least one of them has a random volume", alias0->aliasName, alias1->aliasName);
                                                        return 0;
                                                    }
                                                }
                                                else
                                                {
                                                    if (bReport)
                                                        Com_Error(ERR_DROP, "tried to blend between sound aliases %s and %s, but htey are looping and at least one of them has a random pitch", alias0->aliasName, alias1->aliasName);
                                                    return 0;
                                                }
                                            }
                                            else
                                            {
                                                if (bReport)
                                                    Com_Error(ERR_DROP, "tried to blend aliases %s and %s, but they do not have the same start delay.", alias0->aliasName, alias1->aliasName);
                                                return 0;
                                            }
                                        }
                                        else
                                        {
                                            if (bReport)
                                                Com_Error(ERR_DROP, "tried to blend between sound aliases %s and %s, but they do not have the same slave percentages", alias0->aliasName, alias1->aliasName);
                                            return 0;
                                        }
                                    }
                                    else
                                    {
                                        if (bReport)
                                            Com_Error(ERR_DROP, "tried to blend between sound aliases %s and %s, but they don't use the same nowetlevel setting", alias0->aliasName, alias1->aliasName);
                                        return 0;
                                    }
                                }
                                else
                                {
                                    if (bReport)
                                        Com_Error(ERR_DROP, "tried to blend between sound aliases %s and %s but they don't use the same fulldrylevel setting", alias0->aliasName, alias1->aliasName);
                                    return 0;
                                }
                            }
                            else
                            {
                                if (bReport)
                                    Com_Error(ERR_DROP, "tried to blend between sound aliases %s and %s, but only one of them is a 'slave' alias", alias0->aliasName, alias1->aliasName);
                                return 0;
                            }
                        }
                        else
                        {
                            if (bReport)
                                Com_Error(ERR_DROP, "tried to blend between sound aliases %s and %s, but only one of them is a 'master' alias", alias0->aliasName, alias1->aliasName);
                            return 0;
                        }
                    }
                    else
                    {
                        if (bReport)
                            Com_Error(ERR_DROP, "tried to blend between sound aliases %s and %s, but they don't use the same channel", alias0->aliasName, alias1->aliasName);
                        return 0;
                    }
                }
                else
                {
                    if (bReport)
                        Com_Error(ERR_DROP, "tried to blend between sound aliases %s and %s, but they are not both loaded or both streamed", alias0->aliasName, alias1->aliasName);
                    return 0;
                }
            }
            else
            {
                if (bReport)
                    Com_Error(ERR_DROP, "tried to blend between sound aliases %s and %s, but they do not have the same looping status", alias0->aliasName, alias1->aliasName);
                return 0;
            }
        }
        else
        {
            if (bReport)
                Com_Error(ERR_DROP, "tried to blend between sound aliases %s and %s, but they do not have the same volume falloff curve", alias0->aliasName, alias1->aliasName);
            return 0;
        }
    }
    else
    {
        if (bReport)
            Com_Error(ERR_DROP, "tried to blend between sound aliases %s and %s, but they don't use the same sound file", alias0->aliasName, alias1->aliasName);
        return 0;
    }
}

int __cdecl SND_PlayLocalSoundAlias(unsigned int localClientNum, const snd_alias_t *alias, snd_alias_system_t system)
{
    if ((unsigned int)system >= SASYS_COUNT)
        MyAssertHandler(".\\snd.cpp", 2010, 0, "system doesn't index SASYS_COUNT\n\t%i not in [0, %i)", system, 3);
    if (localClientNum >= 2)
        MyAssertHandler(
            ".\\snd.cpp",
            2011,
            0,
            "localClientNum doesn't index ARRAY_COUNT( g_snd.listeners )\n\t%i not in [0, %i)",
            localClientNum,
            2);

    //int __cdecl SND_PlaySoundAlias_Internal(
    //    const snd_alias_t * alias0,
    //    const snd_alias_t * alias1,
    //    float lerp,
    //    float volumeScale,
    //    SndEntHandle * sndEnt,
    //    SndEntHandle * org,
    //    float *pChannel,
    //    int timeshift,
    //    bool treatAsMaster,
    //    bool useTimescale,
    //    snd_alias_system_t system)

    return SND_PlaySoundAlias_Internal(
        alias,
        alias,
        0.0,
        1.0,
        0,
        g_snd.listeners[localClientNum].orient.origin,
        0,
        0,
        0,
        1,
        system);
}

int __cdecl SND_PlayLocalSoundAliasByName(
    unsigned int localClientNum,
    const char *aliasname,
    snd_alias_system_t system)
{
    snd_alias_t *alias; // [esp+0h] [ebp-4h]

    if (aliasname && (alias = Com_PickSoundAlias(aliasname)) != 0)
        return SND_PlayLocalSoundAlias(localClientNum, alias, system);
    else
        return -1;
}

void __cdecl SND_ResetPauseSettingsToDefaults()
{
    int i; // [esp+0h] [ebp-4h]

    for (i = 0; i < 64; ++i)
        g_snd.pauseSettings[i] = g_snd.defaultPauseSettings[i];
}

void __cdecl SND_PlayMusicAlias(
    int localClientNum,
    const snd_alias_t *alias,
    bool useTimescale,
    snd_alias_system_t system)
{
    if (g_snd.Initialized2d && alias)
    {
        if (SND_IsStreamChannelFree(40))
            SND_StartBackground(localClientNum, 0, alias, 0, 0.0, useTimescale, system);
        else
            Com_PrintWarning(9, "Unable to play music alias %s\n", alias->aliasName);
    }
}

void __cdecl SND_StartBackground(
    int localClientNum,
    unsigned int track,
    const snd_alias_t *alias,
    int fadetime,
    float fraction,
    bool useTimescale,
    snd_alias_system_t system)
{
    double v7; // [esp+8h] [ebp-DCh]
    double v8; // [esp+10h] [ebp-D4h]
    char filename[128]; // [esp+1Ch] [ebp-C8h] BYREF
    float volume; // [esp+A0h] [ebp-44h]
    SndStartAliasInfo startAliasInfo; // [esp+A4h] [ebp-40h] BYREF
    int channel; // [esp+DCh] [ebp-8h]
    float pitch; // [esp+E0h] [ebp-4h]

    if (track > 4)
        MyAssertHandler(".\\snd.cpp", 2168, 0, "%s\n\t(track) = %i", "(track >= 0 && track < SND_TRACK_COUNT)", track);
    if (fraction < 0.0 || fraction > 1.0)
        MyAssertHandler(".\\snd.cpp", 2169, 0, "%s\n\t(fraction) = %g", "(fraction >= 0 && fraction <= 1)", fraction);
    if (!alias)
        MyAssertHandler(".\\snd.cpp", 2170, 0, "%s", "alias");
    if (fadetime < 0)
        MyAssertHandler(".\\snd.cpp", 2171, 0, "%s\n\t(fadetime) = %i", "(fadetime >= 0)", fadetime);
    SND_UpdatePause();
    if (SND_IsAliasChannel3D((alias->flags & 0x3F00) >> 8))
    {
        Com_GetSoundFileName(alias, filename, 128);
        Com_Error(ERR_DROP, "lias %s sound %s played as an ambient / music track uses a 3D channel type; should probably be channel 'local'", alias->aliasName, filename);
    }
    if ((alias->flags & 0xC0) >> 6 != 2)
    {
        Com_GetSoundFileName(alias, filename, 128);
        Com_Error(ERR_DROP, "alias %s sound %s played as an ambient / music track is not streamed; type must be 'streamed'", alias->aliasName, filename);
    }
    v8 = alias->volMax - alias->volMin;
    volume = random() * v8 + alias->volMin;
    v7 = alias->pitchMax - alias->pitchMin;
    pitch = random() * v7 + alias->pitchMin;
    channel = track + 40;
    if ((int)(track + 40) < 40 || channel >= g_snd.max_stream_channels + 40)
        MyAssertHandler(
            ".\\snd.cpp",
            2190,
            0,
            "%s\n\t(channel) = %i",
            "(channel >= ((0 + 8) + 32) && channel < ((0 + 8) + 32) + g_snd.max_stream_channels)",
            channel);
    if (!SND_IsStreamChannelFree(channel))
        SND_StopStreamChannel(channel);
    g_snd.chaninfo[channel].basevolume = volume;
    g_snd.background[track].goalvolume = g_snd.chaninfo[channel].basevolume;
    if (fadetime <= 0)
    {
        g_snd.background[track].goalrate = 0.0;
    }
    else
    {
        g_snd.background[track].goalrate = g_snd.chaninfo[channel].basevolume / (double)fadetime;
        g_snd.chaninfo[channel].basevolume = 0.0;
    }
    startAliasInfo.alias0 = alias;
    startAliasInfo.alias1 = alias;
    startAliasInfo.lerp = 0.0;
    startAliasInfo.sndEnt.field.entIndex = 0xFFFF;
    startAliasInfo.org[0] = g_snd.listeners[localClientNum].orient.origin[0];
    startAliasInfo.org[1] = g_snd.listeners[localClientNum].orient.origin[1];
    startAliasInfo.org[2] = g_snd.listeners[localClientNum].orient.origin[2];
    startAliasInfo.volume = g_snd.chaninfo[channel].basevolume;
    startAliasInfo.pitch = pitch;
    startAliasInfo.timeshift = 0;
    startAliasInfo.fraction = fraction;
    startAliasInfo.startDelay = alias->startDelay;
    startAliasInfo.master = (alias->flags & 2) != 0;
    startAliasInfo.system = system;
    startAliasInfo.timescale = useTimescale;
    if (snd_enableStream->current.enabled)
        SND_StartAliasStreamOnChannel(&startAliasInfo, channel);
}

void SND_UpdatePause()
{
    bool paused; // [esp+3h] [ebp-1h]

    paused = cl_paused->current.integer != 0;
    if (paused != g_snd.paused)
    {
        if (cl_paused->current.integer)
            SND_PauseSounds();
        else
            SND_UnpauseSounds();
    }
    if (paused != g_snd.paused)
        MyAssertHandler(".\\snd.cpp", 2156, 0, "%s", "paused == g_snd.paused");
}

int SND_PauseSounds()
{
    int result; // eax
    int i; // [esp+0h] [ebp-4h]
    int ia; // [esp+0h] [ebp-4h]
    int ib; // [esp+0h] [ebp-4h]

    result = g_snd.Initialized2d;
    if (g_snd.Initialized2d && !g_snd.paused)
    {
        for (i = 0; i < g_snd.max_2D_channels; ++i)
        {
            if (!SND_Is2DChannelFree(i) && g_snd.pauseSettings[(g_snd.chaninfo[i].alias0->flags & 0x3F00) >> 8])
                SND_Pause2DChannel(i);
        }
        for (ia = 8; ia < g_snd.max_3D_channels + 8; ++ia)
        {
            if (!SND_Is3DChannelFree(ia) && g_snd.pauseSettings[(g_snd.chaninfo[ia].alias0->flags & 0x3F00) >> 8])
                SND_Pause3DChannel(ia);
        }
        for (ib = 40; ; ++ib)
        {
            result = g_snd.max_stream_channels + 40;
            if (ib >= g_snd.max_stream_channels + 40)
                break;
            if (!SND_IsStreamChannelFree(ib) && g_snd.pauseSettings[(g_snd.chaninfo[ib].alias0->flags & 0x3F00) >> 8])
                SND_PauseStreamChannel(ib);
        }
        g_snd.paused = 1;
        g_snd.pausetime = g_snd.time;
    }
    return result;
}

void SND_UnpauseSounds()
{
    int i; // [esp+0h] [ebp-8h]
    int ia; // [esp+0h] [ebp-8h]
    int ib; // [esp+0h] [ebp-8h]
    int timeshift; // [esp+4h] [ebp-4h]

    if (g_snd.Initialized2d && g_snd.paused)
    {
        timeshift = g_snd.time - g_snd.pausetime;
        for (i = 0; i < g_snd.max_2D_channels; ++i)
        {
            if (!SND_Is2DChannelFree(i) && g_snd.chaninfo[i].paused)
                SND_Unpause2DChannel(i, timeshift);
        }
        for (ia = 8; ia < g_snd.max_3D_channels + 8; ++ia)
        {
            if (!SND_Is3DChannelFree(ia) && g_snd.chaninfo[ia].paused)
                SND_Unpause3DChannel(ia, timeshift);
        }
        for (ib = 40; ib < g_snd.max_stream_channels + 40; ++ib)
        {
            if (!SND_IsStreamChannelFree(ib) && g_snd.chaninfo[ib].paused)
                SND_UnpauseStreamChannel(ib, timeshift);
        }
        g_snd.paused = 0;
        g_snd.pausetime = 0;
    }
}

void __cdecl SND_StopMusic(int fadetime)
{
    if (g_snd.Initialized2d)
        SND_StopBackground(0, fadetime);
}

void __cdecl SND_StopBackground(unsigned int track, int fadetime)
{
    if (track > 4)
        MyAssertHandler(".\\snd.cpp", 2228, 0, "%s\n\t(track) = %i", "(track >= 0 && track < SND_TRACK_COUNT)", track);
    if (fadetime < 0)
        MyAssertHandler(".\\snd.cpp", 2229, 0, "%s\n\t(fadetime) = %i", "(fadetime >= 0)", fadetime);
    if (!SND_IsStreamChannelFree(track + 40))
    {
        if (fadetime)
        {
            if (g_snd.background[track].goalvolume > 0.0)
            {
                g_snd.background[track].goalrate = -g_snd.background[track].goalvolume / (double)fadetime;
                g_snd.background[track].goalvolume = 0.0;
            }
        }
        else
        {
            SND_StopStreamChannel(track + 40);
        }
    }
}

void __cdecl SND_PlayAmbientAlias(
    int localClientNum,
    const snd_alias_t *alias,
    int fadetime,
    snd_alias_system_t system)
{
    const snd_alias_t *v4; // esi
    double v5; // [esp+10h] [ebp-38h]
    int nextTrack; // [esp+1Ch] [ebp-2Ch]
    float volume; // [esp+20h] [ebp-28h]
    float fraction; // [esp+28h] [ebp-20h]
    snd_background_info_t *trackptr; // [esp+30h] [ebp-18h]
    bool hasSecondary; // [esp+36h] [ebp-12h]
    bool playedNew; // [esp+37h] [ebp-11h]
    const snd_alias_t *aliases[2]; // [esp+38h] [ebp-10h]
    int i; // [esp+40h] [ebp-8h]
    int tracknum; // [esp+44h] [ebp-4h]

    if (g_snd.Initialized2d)
    {
        if (fadetime < 0)
            MyAssertHandler(".\\snd.cpp", 2290, 0, "%s", "fadetime >= 0");
        if (g_snd.ambient_track != 1 && g_snd.ambient_track != 3)
            MyAssertHandler(
                ".\\snd.cpp",
                2291,
                0,
                "%s",
                "g_snd.ambient_track == SND_TRACK_AMBIENT_PRIMARY_0 || g_snd.ambient_track == SND_TRACK_AMBIENT_PRIMARY_1");
        if (alias)
        {
            hasSecondary = alias->secondaryAliasName != 0;
            aliases[0] = alias;
            aliases[1] = 0;
            nextTrack = 4 - g_snd.ambient_track;
            playedNew = 0;
            if ((alias->flags & 0x20) != 0)
                fraction = random();
            else
                fraction = 0.0;
            if (hasSecondary)
                aliases[1] = Com_PickSoundAlias(alias->secondaryAliasName);
            for (i = 0; ; ++i)
            {
                if (i >= 2)
                {
                    if (playedNew)
                        g_snd.ambient_track = nextTrack;
                    return;
                }
                tracknum = g_snd.ambient_track + i + 40;
                if (!aliases[i] || SND_IsStreamChannelFree(tracknum))
                    goto LABEL_32;
                if (g_snd.chaninfo[tracknum].alias0->aliasName == aliases[i]->aliasName)
                    continue;
                if (g_snd.chaninfo[tracknum].alias0->soundFile == aliases[i]->soundFile
                    && (g_snd.chaninfo[tracknum].alias0->flags & 1) == (aliases[i]->flags & 1)
                    && aliases[i]->pitchMin == g_snd.chaninfo[tracknum].alias0->pitchMin
                    && aliases[i]->pitchMax == g_snd.chaninfo[tracknum].alias0->pitchMax
                    && (g_snd.chaninfo[tracknum].alias0->flags & 0x3F00) >> 8 == (aliases[i]->flags & 0x3F00) >> 8)
                {
                    g_snd.chaninfo[tracknum].alias0 = aliases[i];
                    g_snd.chaninfo[tracknum].alias1 = aliases[i];
                    v4 = aliases[i];
                    v5 = v4->volMax - v4->volMin;
                    volume = random() * v5 + v4->volMin;
                    trackptr = (snd_background_info_t *)(8 * (i + g_snd.ambient_track) + 235289552);
                    if (fadetime)
                        g_snd.background[i + g_snd.ambient_track].goalrate = (volume
                            - g_snd.background[i + g_snd.ambient_track].goalvolume)
                        / (double)fadetime;
                    else
                        g_snd.background[i + g_snd.ambient_track].goalrate = 0.0;
                    trackptr->goalvolume = volume;
                }
                else
                {
                LABEL_32:
                    SND_StopBackground(i + g_snd.ambient_track, fadetime);
                    if (aliases[i])
                    {
                        if (!i)
                            playedNew = 1;
                        if (playedNew)
                            SND_StartBackground(localClientNum, i + nextTrack, aliases[i], fadetime, fraction, 1, system);
                        else
                            SND_StartBackground(localClientNum, i + g_snd.ambient_track, aliases[i], fadetime, fraction, 1, system);
                    }
                }
            }
        }
    }
}

void __cdecl SND_StopAmbient(int localClientNum, int fadetime)
{
    if (g_snd.Initialized2d)
    {
        if (fadetime < 0)
            MyAssertHandler(".\\snd.cpp", 2380, 0, "%s", "fadetime >= 0");
        SND_StopBackground(1u, fadetime);
        SND_StopBackground(2u, fadetime);
        SND_StopBackground(3u, fadetime);
        SND_StopBackground(4u, fadetime);
    }
}

void __cdecl SND_FadeAllSounds(float volume, int fadetime)
{
    if (volume < 0.0)
        MyAssertHandler(".\\snd.cpp", 2391, 0, "%s", "volume >= 0");
    if (fadetime < 0)
        MyAssertHandler(".\\snd.cpp", 2392, 0, "%s", "fadetime >= 0");
    g_snd.mastervol.goalvolume = volume;
    g_snd.mastervol.goalrate = volume - g_snd.mastervol.volume;
    if (fadetime)
    {
        g_snd.mastervol.goalrate = g_snd.mastervol.goalrate / (double)fadetime;
    }
    else if (volume == 0.0)
    {
        SND_StopSounds(SND_STOP_ALL);
    }
}

void __cdecl SND_SetChannelVolumes(int priority, const float *channelvolume, int fademsec)
{
    float v3; // [esp+4h] [ebp-Ch]
    snd_channelvolgroup *channelVolGroup; // [esp+8h] [ebp-8h]
    int i; // [esp+Ch] [ebp-4h]
    int ia; // [esp+Ch] [ebp-4h]

    if (priority <= 0 || priority >= 4)
        MyAssertHandler(
            ".\\snd.cpp",
            2413,
            0,
            "%s\n\t(priority) = %i",
            "(priority > SND_CHANNELVOLPRIO_NONE && priority < SND_CHANNELVOLPRIO_COUNT)",
            priority);
    if (!channelvolume)
        MyAssertHandler(".\\snd.cpp", 2414, 0, "%s", "channelvolume");
    if (fademsec < 0)
        MyAssertHandler(".\\snd.cpp", 2415, 0, "%s", "fademsec >= 0");
    channelVolGroup = &g_snd.channelVolGroups[priority];
    channelVolGroup->active = 1;
    if (fademsec < 1)
        fademsec = 1;
    for (i = 0; i < SND_GetEntChannelCount(); ++i)
    {
        if (channelvolume[i] < 0.0 || channelvolume[i] > 1.0)
            MyAssertHandler(".\\snd.cpp", 2425, 0, "%s", "channelvolume[i] >= 0.0f && channelvolume[i] <= 1.0f");
        channelVolGroup->channelvol[i].goalvolume = channelvolume[i];
        channelVolGroup->channelvol[i].volume = g_snd.channelvol->channelvol[i].volume;
        v3 = (channelvolume[i] - channelVolGroup->channelvol[i].volume) / (double)fademsec;
        channelVolGroup->channelvol[i].goalrate = v3;
    }
    if (channelVolGroup != g_snd.channelvol)
    {
        for (ia = priority + 1; ia < 4; ++ia)
        {
            if (g_snd.channelVolGroups[ia].active)
                return;
        }
        g_snd.channelvol = &g_snd.channelVolGroups[priority];
    }
}

void __cdecl SND_DeactivateChannelVolumes(int priority, int fademsec)
{
    snd_channelvolgroup *channelVolGroup; // [esp+4h] [ebp-8h]
    int i; // [esp+8h] [ebp-4h]
    int ia; // [esp+8h] [ebp-4h]

    if (g_snd.Initialized2d)
    {
        if (priority <= 0 || priority >= 4)
            MyAssertHandler(
                ".\\snd.cpp",
                2457,
                0,
                "%s\n\t(priority) = %i",
                "(priority > SND_CHANNELVOLPRIO_NONE && priority < SND_CHANNELVOLPRIO_COUNT)",
                priority);
        if (fademsec < 0)
            MyAssertHandler(".\\snd.cpp", 2458, 0, "%s", "fademsec >= 0");
        channelVolGroup = &g_snd.channelVolGroups[priority];
        channelVolGroup->active = 0;
        if (channelVolGroup == g_snd.channelvol)
        {
            for (i = priority - 1; i >= 0 && !g_snd.channelVolGroups[i].active; --i)
                ;
            if (i < 0)
                MyAssertHandler(".\\snd.cpp", 2471, 0, "%s", "i >= SND_CHANNELVOLPRIO_NONE");
            if (fademsec < 1)
                fademsec = 1;
            g_snd.channelvol = &g_snd.channelVolGroups[i];
            for (ia = 0; ia < SND_GetEntChannelCount(); ++ia)
            {
                g_snd.channelvol->channelvol[ia].volume = channelVolGroup->channelvol[ia].volume;
                g_snd.channelvol->channelvol[ia].goalrate = (g_snd.channelvol->channelvol[ia].goalvolume
                    - channelVolGroup->channelvol[ia].volume)
                    / (double)fademsec;
            }
        }
    }
}

void __cdecl SND_UpdateLoopingSounds()
{
    int i; // [esp+30h] [ebp-4h]
    int ia; // [esp+30h] [ebp-4h]
    int ib; // [esp+30h] [ebp-4h]

    if (g_snd.Initialized2d && !g_snd.paused)
    {
        //Profile_Begin(339);
        for (i = 8; i < g_snd.max_3D_channels + 8; ++i)
        {
            if (!SND_Is3DChannelFree(i))
            {
                if (!g_snd.chaninfo[i].alias0)
                    MyAssertHandler(".\\snd.cpp", 2534, 0, "%s", "g_snd.chaninfo[i].alias0");
                if ((g_snd.chaninfo[i].alias0->flags & 1) != 0 && g_snd.chaninfo[i].looptime != g_snd.looptime)
                    SND_Stop3DChannel(i);
            }
        }
        for (ia = 0; ia < g_snd.max_2D_channels; ++ia)
        {
            if (!SND_Is2DChannelFree(ia))
            {
                if (!g_snd.chaninfo[ia].alias0)
                    MyAssertHandler(".\\snd.cpp", 2545, 0, "%s", "g_snd.chaninfo[i].alias0");
                if ((g_snd.chaninfo[ia].alias0->flags & 1) != 0 && g_snd.chaninfo[ia].looptime != g_snd.looptime)
                    SND_Stop2DChannel(ia);
            }
        }
        for (ib = 40; ib < g_snd.max_stream_channels + 40; ++ib)
        {
            if (!SND_IsStreamChannelFree(ib))
            {
                if (!g_snd.chaninfo[ib].alias0)
                    MyAssertHandler(".\\snd.cpp", 2556, 0, "%s", "g_snd.chaninfo[i].alias0");
                if ((g_snd.chaninfo[ib].alias0->flags & 1) != 0 && g_snd.chaninfo[ib].looptime != g_snd.looptime)
                    SND_StopStreamChannel(ib);
            }
        }
        g_snd.looptime = g_snd.time;
        //Profile_EndInternal(0);
    }
}

char __cdecl SND_UpdateBackgroundVolume(unsigned int track, int frametime)
{
    float volume; // [esp+0h] [ebp-8h]
    int channel; // [esp+4h] [ebp-4h]

    if (track > 4)
        MyAssertHandler(".\\snd.cpp", 2581, 0, "%s\n\t(track) = %i", "(track >= 0 && track < SND_TRACK_COUNT)", track);
    channel = track + 40;
    if (SND_IsAliasChannel3D((g_snd.chaninfo[track + 40].alias0->flags & 0x3F00) >> 8))
        MyAssertHandler(
            ".\\snd.cpp",
            2584,
            0,
            "%s",
            "!SND_IsAliasChannel3D( SNDALIASFLAGS_GET_CHANNEL( g_snd.chaninfo[channel].alias0->flags ) )");
    volume = (double)frametime * g_snd.background[track].goalrate + g_snd.chaninfo[channel].basevolume;
    if (g_snd.background[track].goalrate <= 0.0)
    {
        if (g_snd.background[track].goalvolume > (double)volume)
        {
            volume = g_snd.background[track].goalvolume;
            if (volume == 0.0)
            {
                SND_StopStreamChannel(channel);
                return 0;
            }
        }
    }
    else if (g_snd.background[track].goalvolume < (double)volume)
    {
        volume = g_snd.background[track].goalvolume;
    }
    g_snd.chaninfo[channel].looptime = g_snd.looptime;
    g_snd.chaninfo[channel].basevolume = volume;
    return 1;
}

void __cdecl SND_SetEnvironmentEffects(
    int priority,
    const char *roomstring,
    float drylevel,
    float wetlevel,
    int fademsec)
{
    snd_enveffect *effect; // [esp+0h] [ebp-Ch]
    int i; // [esp+4h] [ebp-8h]
    int roomtype; // [esp+8h] [ebp-4h]

    if (priority <= 0 || priority >= 3)
        MyAssertHandler(
            ".\\snd.cpp",
            2773,
            0,
            "%s\n\t(priority) = %i",
            "(priority > SND_ENVEFFECTPRIO_NONE && priority < SND_ENVEFFECTPRIO_COUNT)",
            priority);
    if (!roomstring)
        MyAssertHandler(".\\snd.cpp", 2774, 0, "%s", "roomstring");
    if (fademsec < 0)
        MyAssertHandler(".\\snd.cpp", 2775, 0, "%s", "fademsec >= 0");
    if (drylevel < 0.0 || drylevel > 1.0)
        MyAssertHandler(".\\snd.cpp", 2776, 0, "%s", "drylevel >= 0 && drylevel <= 1");
    if (wetlevel < 0.0 || wetlevel > 1.0)
        MyAssertHandler(".\\snd.cpp", 2777, 0, "%s", "wetlevel >= 0 && wetlevel <= 1");
    if (g_snd.Initialized2d)
    {
        effect = &g_snd.envEffects[priority];
        effect->active = 1;
        roomtype = SND_RoomtypeFromString(roomstring);
        effect->roomtype = roomtype;
        if (fademsec < 1)
            fademsec = 1;
        effect->drygoal = drylevel;
        effect->drylevel = g_snd.effect->drylevel;
        effect->dryrate = (drylevel - g_snd.effect->drylevel) / (double)fademsec;
        effect->wetgoal = wetlevel;
        effect->wetlevel = g_snd.effect->wetlevel;
        effect->wetrate = (wetlevel - g_snd.effect->wetlevel) / (double)fademsec;
        if (effect == g_snd.effect)
        {
            SND_SetRoomtype(roomtype);
        }
        else
        {
            for (i = priority + 1; i < 3; ++i)
            {
                if (g_snd.envEffects[i].active)
                    return;
            }
            g_snd.effect = &g_snd.envEffects[priority];
            SND_SetRoomtype(roomtype);
        }
    }
}

void __cdecl SND_DeactivateEnvironmentEffects(int priority, int fademsec)
{
    snd_enveffect *effect; // [esp+0h] [ebp-8h]
    int i; // [esp+4h] [ebp-4h]

    if (priority <= 0 || priority >= 3)
        MyAssertHandler(
            ".\\snd.cpp",
            2825,
            0,
            "%s\n\t(priority) = %i",
            "(priority > SND_ENVEFFECTPRIO_NONE && priority < SND_ENVEFFECTPRIO_COUNT)",
            priority);
    if (fademsec < 0)
        MyAssertHandler(".\\snd.cpp", 2826, 0, "%s", "fademsec >= 0");
    effect = &g_snd.envEffects[priority];
    effect->active = 0;
    if (effect == g_snd.effect)
    {
        for (i = priority - 1; i >= 0 && !g_snd.envEffects[i].active; --i)
            ;
        if (i < 0)
            MyAssertHandler(".\\snd.cpp", 2839, 0, "%s", "i >= SND_ENVEFFECTPRIO_NONE");
        if (fademsec < 1)
            fademsec = 1;
        g_snd.effect = &g_snd.envEffects[i];
        SND_SetRoomtype(g_snd.envEffects[i].roomtype);
        g_snd.effect->drylevel = effect->drylevel;
        g_snd.effect->dryrate = (g_snd.effect->drygoal - effect->drylevel) / (double)fademsec;
        g_snd.effect->wetlevel = effect->wetlevel;
        g_snd.effect->wetrate = (g_snd.effect->wetgoal - effect->wetlevel) / (double)fademsec;
    }
}

void __cdecl SND_UpdateReverbs()
{
    int i; // [esp+0h] [ebp-4h]
    int ia; // [esp+0h] [ebp-4h]
    int ib; // [esp+0h] [ebp-4h]

    for (i = 8; i < g_snd.max_3D_channels + 8; ++i)
    {
        if (!SND_Is3DChannelFree(i))
            SND_Update3DChannelReverb(i);
    }
    for (ia = 0; ia < g_snd.max_2D_channels; ++ia)
    {
        if (!SND_Is2DChannelFree(ia))
            SND_Update2DChannelReverb(ia);
    }
    for (ib = 40; ib < g_snd.max_stream_channels + 40; ++ib)
    {
        if (!SND_IsStreamChannelFree(ib))
            SND_UpdateStreamChannelReverb(ib);
    }
}

void __cdecl SND_DeactivateAllEq(int eqIndex)
{
    signed int band; // [esp+0h] [ebp-8h]
    signed int entchannel; // [esp+4h] [ebp-4h]

    for (entchannel = 0; entchannel < 64; ++entchannel)
    {
        for (band = 0; band < 3; ++band)
            SND_DisableEq(entchannel, eqIndex, band);
    }
}

void __cdecl SND_DeactivateChannelEq(const char *channelName, int eqIndex)
{
    signed int band; // [esp+0h] [ebp-4h]

    for (band = 0; band < 3; ++band)
        SND_DeactivateEq(channelName, eqIndex, band);
}

void __cdecl SND_DeactivateEq(const char *channelName, int eqIndex, unsigned int band)
{
    signed int entchannel; // [esp+0h] [ebp-4h]

    entchannel = SND_GetEntChannelFromName(channelName);
    if (entchannel >= 0)
        SND_DisableEq(entchannel, eqIndex, band);
    else
        Com_PrintError(9, "Unknown channel name (%s), please check channel definitions file\n", channelName);
}

void __cdecl SND_Update()
{
    unsigned intv0; // eax
    int frametime; // [esp+30h] [ebp-20h]
    MemoryFile memFile; // [esp+34h] [ebp-1Ch] BYREF

    if (g_snd.Initialized2d)
    {
        //Profile_Begin(339);
        g_snd.cpu = SND_GetDriverCPUPercentage();
        if (com_statmon->current.enabled && SND_ShouldGiveCpuWarning())
            StatMon_Warning(2, 3000, "code_warning_soundcpu");
        int v0 = Sys_Milliseconds();
        frametime = v0 - g_snd.time;
        g_snd.time = v0;
        CL_ResetStats_f();
        SND_UpdatePause();
        SND_UpdateMasterVolumes(frametime);
        if (!g_snd.paused)
        {
            if (g_snd.restore.size)
            {
                MemFile_InitForReading(&memFile, g_snd.restore.size, g_snd.restore.buffer, g_snd.restore.compress);
                SND_Restore(&memFile);
                MemFile_MoveToSegment(&memFile, -1);
                if (memFile.bytesUsed != g_snd.restore.size)
                    MyAssertHandler(".\\snd.cpp", 3279, 0, "%s", "memFile.bytesUsed == g_snd.restore.size");
                g_snd.restore.size = 0;
            }
            SND_UpdateTimeScale();
            SND_UpdateRoomEffects(frametime);
        }
        SND_UpdateAllChannels(frametime);
        SND_UpdatePhysics();
        SND_DriverPostUpdate();
        DebugDrawWorldSounds(snd_draw3D->current.integer);
        //Profile_EndInternal(0);
    }
}

void __cdecl SND_UpdateMasterVolumes(int frametime)
{
    int i; // [esp+0h] [ebp-4h]

    for (i = 0; i < SND_GetEntChannelCount(); ++i)
    {
        if (!g_snd.paused || !SND_IsPausable(i))
            SND_UpdateVolume(&g_snd.channelvol->channelvol[i], frametime);
    }
    if (g_snd.mastervol.goalrate == 0.0)
    {
        if (!snd_volume->modified)
            return;
    }
    else
    {
        SND_UpdateVolume(&g_snd.mastervol, frametime);
        if (g_snd.mastervol.volume == 0.0 && g_snd.mastervol.goalrate == 0.0)
            SND_StopSounds(SND_STOP_ALL);
    }
    Dvar_ClearModified((dvar_s*)snd_volume);
    g_snd.volume = g_snd.mastervol.volume * snd_volume->current.value * 0.75;
}

void __cdecl SND_UpdateVolume(snd_volume_info_t *volinfo, int frametime)
{
    volinfo->volume = (double)frametime * volinfo->goalrate + volinfo->volume;
    if (volinfo->goalrate >= 0.0)
    {
        if (volinfo->goalvolume < (double)volinfo->volume)
        {
            volinfo->volume = volinfo->goalvolume;
            volinfo->goalrate = 0.0;
        }
    }
    else if (volinfo->goalvolume > (double)volinfo->volume)
    {
        volinfo->volume = volinfo->goalvolume;
        volinfo->goalrate = 0.0;
    }
}

void __cdecl SND_UpdateAllChannels(int frametime)
{
    int i; // [esp+0h] [ebp-4h]
    int ia; // [esp+0h] [ebp-4h]
    int ib; // [esp+0h] [ebp-4h]

    SND_UpdateSlaveLerp(frametime);
    for (i = 8; i < g_snd.max_3D_channels + 8; ++i)
    {
        if (!SND_Is3DChannelFree(i))
            SND_Update3DChannel(i, frametime);
    }
    for (ia = 40; ia < g_snd.max_stream_channels + 40; ++ia)
    {
        if (!SND_IsStreamChannelFree(ia))
            SND_UpdateStreamChannel(ia, frametime);
    }
    for (ib = 0; ib < g_snd.max_2D_channels; ++ib)
    {
        if (!SND_Is2DChannelFree(ib))
            SND_Update2DChannel(ib, frametime);
    }
}

void __cdecl SND_UpdateSlaveLerp(int frametime)
{
    double v1; // st7
    float v2; // [esp+0h] [ebp-1Ch]
    float v3; // [esp+4h] [ebp-18h]
    float v4; // [esp+8h] [ebp-14h]
    float v5; // [esp+Ch] [ebp-10h]
    float slaveLerp; // [esp+10h] [ebp-Ch]
    bool masterPlaying; // [esp+17h] [ebp-5h]
    int i; // [esp+18h] [ebp-4h]
    int ia; // [esp+18h] [ebp-4h]
    int ib; // [esp+18h] [ebp-4h]

    masterPlaying = 0;
    for (i = 8; !masterPlaying && i < g_snd.max_3D_channels + 8; ++i)
    {
        if (SND_Is3DChannelPlaying(i))
            masterPlaying = g_snd.chaninfo[i].master;
    }
    for (ia = 40; !masterPlaying && ia < g_snd.max_stream_channels + 40; ++ia)
    {
        if (SND_IsStreamChannelPlaying(ia))
            masterPlaying = g_snd.chaninfo[ia].master;
    }
    for (ib = 0; !masterPlaying && ib < g_snd.max_2D_channels; ++ib)
    {
        if (SND_Is2DChannelPlaying(ib))
            masterPlaying = g_snd.chaninfo[ib].master;
    }
    if (snd_slaveFadeTime->current.integer)
    {
        if (masterPlaying)
            v1 = (double)frametime / (double)snd_slaveFadeTime->current.integer + g_snd.slaveLerp;
        else
            v1 = g_snd.slaveLerp - (double)frametime / (double)snd_slaveFadeTime->current.integer;
        g_snd.slaveLerp = v1;
        v4 = g_snd.slaveLerp - 1.0;
        if (v4 < 0.0)
            slaveLerp = g_snd.slaveLerp;
        else
            slaveLerp = 1.0;
        v3 = 0.0 - g_snd.slaveLerp;
        if (v3 < 0.0)
            v2 = slaveLerp;
        else
            v2 = 0.0;
        g_snd.slaveLerp = v2;
    }
    else
    {
        if (masterPlaying)
            v5 = 1.0;
        else
            v5 = 0.0;
        g_snd.slaveLerp = v5;
    }
}

bool __cdecl SND_Is2DChannelPlaying(int index)
{
    if (index < 0 || index >= g_snd.max_2D_channels)
        MyAssertHandler(
            ".\\snd.cpp",
            2660,
            0,
            "%s\n\t(index) = %i",
            "(index >= 0 && index < 0 + g_snd.max_2D_channels)",
            index);
    return !SND_Is2DChannelFree(index) && !g_snd.chaninfo[index].startDelay;
}

bool __cdecl SND_Is3DChannelPlaying(int index)
{
    if (index < 8 || index >= g_snd.max_3D_channels + 8)
        MyAssertHandler(
            ".\\snd.cpp",
            2668,
            0,
            "%s\n\t(index) = %i",
            "(index >= (0 + 8) && index < (0 + 8) + g_snd.max_3D_channels)",
            index);
    return !SND_Is3DChannelFree(index) && !g_snd.chaninfo[index].startDelay;
}

bool __cdecl SND_IsStreamChannelPlaying(int index)
{
    if (index < 40 || index >= g_snd.max_stream_channels + 40)
        MyAssertHandler(
            ".\\snd.cpp",
            2676,
            0,
            "%s\n\t(index) = %i",
            "(index >= ((0 + 8) + 32) && index < ((0 + 8) + 32) + g_snd.max_stream_channels)",
            index);
    return !SND_IsStreamChannelFree(index) && g_snd.chaninfo[index].startDelay <= 0;
}

void __cdecl SND_UpdateRoomEffects(int frametime)
{
    if (g_snd.effect->dryrate != 0.0 || g_snd.effect->wetrate != 0.0)
    {
        if (g_snd.effect->dryrate != 0.0)
        {
            g_snd.effect->drylevel = (double)frametime * g_snd.effect->dryrate + g_snd.effect->drylevel;
            if (g_snd.effect->dryrate >= 0.0)
            {
                if (g_snd.effect->drygoal <= (double)g_snd.effect->drylevel)
                {
                    g_snd.effect->drylevel = g_snd.effect->drygoal;
                    g_snd.effect->dryrate = 0.0;
                }
            }
            else if (g_snd.effect->drygoal >= (double)g_snd.effect->drylevel)
            {
                g_snd.effect->drylevel = g_snd.effect->drygoal;
                g_snd.effect->dryrate = 0.0;
            }
        }
        if (g_snd.effect->wetrate != 0.0)
        {
            g_snd.effect->wetlevel = (double)frametime * g_snd.effect->wetrate + g_snd.effect->wetlevel;
            if (g_snd.effect->wetrate >= 0.0)
            {
                if (g_snd.effect->wetgoal <= (double)g_snd.effect->wetlevel)
                {
                    g_snd.effect->wetlevel = g_snd.effect->wetgoal;
                    g_snd.effect->wetrate = 0.0;
                }
            }
            else if (g_snd.effect->wetgoal >= (double)g_snd.effect->wetlevel)
            {
                g_snd.effect->wetlevel = g_snd.effect->wetgoal;
                g_snd.effect->wetrate = 0.0;
            }
        }
        SND_UpdateReverbs();
    }
}

void SND_UpdateTimeScale()
{
    float v0; // [esp+24h] [ebp-40h]
    float v1; // [esp+34h] [ebp-30h]
    float v2; // [esp+44h] [ebp-20h]
    float timescale; // [esp+58h] [ebp-Ch]
    float factor; // [esp+5Ch] [ebp-8h]
    int i; // [esp+60h] [ebp-4h]
    int ia; // [esp+60h] [ebp-4h]
    int ib; // [esp+60h] [ebp-4h]

    timescale = Com_GetTimescaleForSnd();
    if (timescale <= 0.0)
        timescale = 1.0;
    if (g_snd.timescale != timescale)
    {
        if (timescale == 0.0)
            MyAssertHandler(".\\snd.cpp", 2997, 0, "%s", "timescale");
        if (g_snd.timescale == 0.0)
            MyAssertHandler(".\\snd.cpp", 2998, 0, "%s", "g_snd.timescale");
        factor = timescale / g_snd.timescale;
        g_snd.timescale = timescale;
        for (i = 8; i < g_snd.max_3D_channels + 8; ++i)
        {
            if (!SND_Is3DChannelFree(i) && g_snd.chaninfo[i].timescale)
            {
                v2 = (double)SND_Get3DChannelPlaybackRate(i) * factor;
                SND_Set3DChannelPlaybackRate(i, (int)(v2 + 9.313225746154785e-10));
            }
        }
        for (ia = 40; ia < g_snd.max_stream_channels + 40; ++ia)
        {
            if (!SND_IsStreamChannelFree(ia) && g_snd.chaninfo[ia].timescale)
            {
                v1 = (double)SND_GetStreamChannelPlaybackRate(ia) * factor;
                SND_SetStreamChannelPlaybackRate(ia, (int)(v1 + 9.313225746154785e-10));
            }
        }
        for (ib = 0; ib < g_snd.max_2D_channels; ++ib)
        {
            if (!SND_Is2DChannelFree(ib) && g_snd.chaninfo[ib].timescale)
            {
                v0 = (double)SND_Get2DChannelPlaybackRate(ib) * factor;
                SND_Set2DChannelPlaybackRate(ib, (int)(v0 + 9.313225746154785e-10));
            }
        }
    }
}

void __cdecl DebugDrawWorldSounds(int debugDrawStyle)
{
    int closestId; // [esp+0h] [ebp-1214h] BYREF
    int dst[1153]; // [esp+4h] [ebp-1210h] BYREF
    int index; // [esp+1208h] [ebp-Ch]
    float closestIdDotProd; // [esp+120Ch] [ebp-8h] BYREF
    int entchannel; // [esp+1210h] [ebp-4h]

    if (!g_snd.Initialized2d)
        MyAssertHandler(".\\snd.cpp", 3157, 0, "%s", "g_snd.Initialized2d");
    if (debugDrawStyle)
    {
        closestId = -1;
        closestIdDotProd = -2.0;
        memset((unsigned __int8 *)dst, 0, 0x1200u);
        for (index = 8; index < g_snd.max_3D_channels + 8; ++index)
        {
            if (!SND_Is3DChannelFree(index))
                DebugDrawWorldSound3D(index, debugDrawStyle, dst, &closestId, &closestIdDotProd);
        }
        for (index = 40; index < g_snd.max_stream_channels + 40; ++index)
        {
            if (!SND_IsStreamChannelFree(index))
            {
                if (!g_snd.chaninfo[index].alias0)
                    MyAssertHandler(".\\snd.cpp", 3178, 0, "%s", "g_snd.chaninfo[idx].alias0");
                entchannel = (g_snd.chaninfo[index].alias0->flags & 0x3F00) >> 8;
                if (SND_IsAliasChannel3D(entchannel))
                    DebugDrawWorldSound3D(index, debugDrawStyle, dst, &closestId, &closestIdDotProd);
            }
        }
        if (closestId != -1 && closestIdDotProd >= 0.93000001)
            DebugDrawWorldSound3D(closestId, 3, dst, 0, 0);
    }
}

void __cdecl DebugDrawWorldSound3D(
    unsigned int idx,
    int debugDrawStyle,
    int *offsets,
    int *closestId,
    float *closestIdDotProd)
{
    float diff[3]; // [esp+44h] [ebp-14Ch] BYREF
    float sndDir[3]; // [esp+50h] [ebp-140h] BYREF
    float dot; // [esp+5Ch] [ebp-134h]
    char buffer[256]; // [esp+60h] [ebp-130h] BYREF
    float dist; // [esp+164h] [ebp-2Ch]
    int time; // [esp+168h] [ebp-28h]
    const float *starColor; // [esp+16Ch] [ebp-24h]
    float org[3]; // [esp+170h] [ebp-20h] BYREF
    float fontsize; // [esp+17Ch] [ebp-14h]
    int listenerId; // [esp+180h] [ebp-10h]
    float origZ; // [esp+184h] [ebp-Ch]
    const char *text; // [esp+188h] [ebp-8h]
    snd_channel_info_t *chaninfo; // [esp+18Ch] [ebp-4h]

    if (idx > 0x34)
        MyAssertHandler(
            ".\\snd.cpp",
            3050,
            0,
            "%s\n\t(idx) = %i",
            "(( idx >= 0 ) && ( idx < (32 + (SND_TRACK_COUNT + 8) + 8) ))",
            idx);
    chaninfo = &g_snd.chaninfo[idx];
    if (!chaninfo->alias0)
        MyAssertHandler(".\\snd.cpp", 3053, 0, "%s", "chaninfo->alias0");
    SND_GetCurrent3DPosition(chaninfo->sndEnt, chaninfo->offset, org);
    listenerId = SND_GetListenerIndexNearestToOrigin(org);
    Vec3Sub(g_snd.listeners[listenerId].orient.origin, org, diff);
    dist = Vec3Length(diff);
    fontsize = FontSizeForDistance(dist);
    if (g_snd.pausetime)
        time = g_snd.pausetime - chaninfo->startTime;
    else
        time = g_snd.time - chaninfo->startTime;
    if (debugDrawStyle != 3)
    {
        if (!closestId)
            MyAssertHandler(".\\snd.cpp", 3070, 0, "%s", "closestId");
        if (!closestIdDotProd)
            MyAssertHandler(".\\snd.cpp", 3071, 0, "%s", "closestIdDotProd");
        Vec3Sub(org, g_snd.listeners[listenerId].orient.origin, sndDir);
        Vec3Normalize(sndDir);
        dot = Vec3Dot(sndDir, g_snd.listeners[listenerId].orient.axis[0]);
        if (*closestIdDotProd < (double)dot)
        {
            *closestId = idx;
            *closestIdDotProd = dot;
        }
    }
    origZ = org[2];
    if (chaninfo->sndEnt.field.entIndex == 1022)
    {
        starColor = colorGreen;
    }
    else
    {
        starColor = colorLtCyan;
        org[2] = (double)offsets[chaninfo->sndEnt.field.entIndex] + org[2];
    }
    if (debugDrawStyle != 1)
    {
        if (debugDrawStyle != 2)
        {
            if (debugDrawStyle == 3)
            {
                fontsize = fontsize * 0.8500000238418579;
                text = va("Details: %s", chaninfo->alias0->aliasName);
                if (offsets[chaninfo->sndEnt.field.entIndex])
                    CL_AddDebugString(org, colorWhiteFaded, fontsize, (char *)text, 0, 1);
                else
                    CL_AddDebugStarWithText(org, starColor, colorWhiteFaded, (char *)text, fontsize, 1, 0);
                if (!chaninfo->alias0->soundFile)
                    MyAssertHandler(".\\snd.cpp", 3114, 0, "%s", "chaninfo->alias0->soundFile");
                org[2] = org[2] - fontsize * 12.0;
                strcpy(buffer, "File: ");
                Com_GetSoundFileName(
                    chaninfo->alias0,
                    &buffer[&buffer[strlen(buffer) + 1] - &buffer[1]],
                    256 - (&buffer[strlen(buffer) + 1] - &buffer[1]));
                CL_AddDebugString(org, colorWhiteFaded, fontsize, buffer, 0, 1);
                if (chaninfo->sndEnt.field.entIndex == 1022)
                {
                    org[2] = org[2] - fontsize * 12.0;
                    text = va("Owner: World");
                }
                else
                {
                    org[2] = org[2] - fontsize * 12.0;
                    text = va("Owner: entity #%i", chaninfo->sndEnt.field.entIndex);
                }
                CL_AddDebugString(org, colorWhiteFaded, fontsize, (char *)text, 0, 1);
                org[2] = org[2] - fontsize * 12.0;
                text = va("Distance: %.0f / %.0f", dist, chaninfo->alias0->distMax);
                CL_AddDebugString(org, colorWhiteFaded, fontsize, (char *)text, 0, 1);
                org[2] = org[2] - fontsize * 12.0;
                text = va("Time: %.3f", (double)time / 1000.0);
                CL_AddDebugString(org, colorWhiteFaded, fontsize, (char *)text, 0, 1);
            }
            goto LABEL_35;
        }
        CL_AddDebugString(org, colorWhiteFaded, fontsize, (char *)chaninfo->alias0->aliasName, 0, 1);
    }
    if (!offsets[chaninfo->sndEnt.field.entIndex])
        CL_AddDebugStar(org, starColor, 1, 0);
LABEL_35:
    offsets[chaninfo->sndEnt.field.entIndex] = (int)(org[2] - fontsize * 1.25 * 12.0 - origZ);
}

double __cdecl FontSizeForDistance(float distance)
{
    if (distance < 10.0)
        return 0.050000001;
    return (float)(distance / 2000.0 * 3.5);
}

void SND_UpdatePhysics()
{
    snd_alias_t *alias; // [esp+4h] [ebp-Ch]
    int i; // [esp+8h] [ebp-8h]
    int count; // [esp+Ch] [ebp-4h]

    Sys_EnterCriticalSection(CRITSECT_AUDIO_PHYSICS);
    count = g_sndPhysics.count;
    g_sndPhysics.count = 0;
    if (SND_AnyActiveListeners())
    {
        if (count > 32)
            MyAssertHandler(".\\snd.cpp", 3211, 0, "%s", "count <= SND_MAX_PHYSICS");
        for (i = 0; i < count; ++i)
        {
            alias = Com_PickSoundAliasFromList(g_sndPhysics.info[i].aliasList);
            SND_PlaySoundAlias(alias, (SndEntHandle)1022, g_sndPhysics.info[i].org, 0, SASYS_CGAME);
        }
    }
    Sys_LeaveCriticalSection(CRITSECT_AUDIO_PHYSICS);
}

bool __cdecl SND_ShouldGiveCpuWarning()
{
    if (g_snd.cpu > 2)
        return clientUIActives[0].connectionState >= CA_ACTIVE;
    else
        return false;
}

void __cdecl SND_StopSounds(snd_stopsounds_arg_t which)
{
    char v1; // [esp+0h] [ebp-8h]
    int i; // [esp+4h] [ebp-4h]
    int ia; // [esp+4h] [ebp-4h]
    int ib; // [esp+4h] [ebp-4h]
    int ic; // [esp+4h] [ebp-4h]
    int id; // [esp+4h] [ebp-4h]
    int ie; // [esp+4h] [ebp-4h]

    if (g_snd.Initialized2d)
    {
        if ((which & 8) == 0)
        {
            for (i = 0; i < g_snd.max_2D_channels; ++i)
            {
                if (!SND_Is2DChannelFree(i))
                    SND_Stop2DChannel(i);
            }
            for (ia = 8; ia < g_snd.max_3D_channels + 8; ++ia)
            {
                if (!SND_Is3DChannelFree(ia))
                    SND_Stop3DChannel(ia);
            }
        }
        for (ib = 40; ib < g_snd.max_stream_channels + 40; ++ib)
        {
            if (!SND_IsStreamChannelFree(ib) && ((which & 2) == 0 || ib != 40))
            {
                if ((which & 4) == 0 || (ib < 41 || ib > 44 ? (v1 = 0) : (v1 = 1), !v1))
                    SND_StopStreamChannel(ib);
            }
        }
        if ((which & 1) == 0)
        {
            for (ic = 1; ic < 3; ++ic)
                SND_DeactivateEnvironmentEffects(ic, 0);
        }
        if ((which & 0x10) == 0)
        {
            for (id = 1; id < 4; ++id)
                SND_DeactivateChannelVolumes(id, 0);
        }
        for (ie = 0; ie < 2; ++ie)
            SND_DeactivateAllEq(ie);
    }
}

cmd_function_s SND_SetEnvironmentEffects_f_VAR;
cmd_function_s SND_DeactivateEnvironmentEffects_f_VAR;
cmd_function_s SND_PlayLocal_f_VAR;
cmd_function_s SND_SetEq_f_VAR;
cmd_function_s SND_SetEqFreq_f_VAR;
cmd_function_s SND_SetEqGain_f_VAR;
cmd_function_s SND_SetEqQ_f_VAR;
cmd_function_s SND_SetEqType_f_VAR;
cmd_function_s SND_DeactivateEq_f_VAR;

void __cdecl SND_Init()
{
    DvarLimits min; // [esp+4h] [ebp-20h]
    DvarLimits mina; // [esp+4h] [ebp-20h]
    int i; // [esp+1Ch] [ebp-8h]
    int ia; // [esp+1Ch] [ebp-8h]

    Com_Printf(9, "\n------- sound system initialization -------\n");
    snd_errorOnMissing = Dvar_RegisterBool("snd_errorOnMissing", 0, 1u, "Cause a Com_Error if a sound file is missing.");
    min.value.max = 1.0;
    min.value.min = 0.0;
    snd_volume = Dvar_RegisterFloat("snd_volume", 0.80000001, min, 1u, "Game sound master volume");
    snd_slaveFadeTime = Dvar_RegisterInt(
        "snd_slaveFadeTime",
        500,
        (DvarLimits)0x138800000000LL,
        0x81u,
        "The amount of time in milliseconds for a 'slave' sound\n"
        "to fade its volumes when a master sound starts or stops");
    snd_enable2D = Dvar_RegisterBool("snd_enable2D", 1, 0x80u, "Enable 2D sounds");
    snd_enable3D = Dvar_RegisterBool("snd_enable3D", 1, 0x80u, "Enable 3D sounds");
    snd_enableStream = Dvar_RegisterBool("snd_enableStream", 1, 0x80u, "Enable streamed sounds");
    snd_enableReverb = Dvar_RegisterBool("snd_enableReverb", 1, 0x80u, "Enable sound reverberation");
    snd_enableEq = Dvar_RegisterBool("snd_enableEq", 1, 1u, "Enable equalization filter");
    snd_draw3D = Dvar_RegisterEnum("snd_draw3D", snd_draw3DNames, 0, 0x80u, "Draw the position and info of world sounds");
    snd_levelFadeTime = Dvar_RegisterInt(
        "snd_levelFadeTime",
        250,
        (DvarLimits)0x138800000000LL,
        0x81u,
        "The amout of time in milliseconds for all audio to fade in at the start of a level");
    mina.value.max = 1.0;
    mina.value.min = 0.0;
    snd_cinematicVolumeScale = Dvar_RegisterFloat(
        "snd_cinematicVolumeScale",
        0.85000002,
        mina,
        1u,
        "Scales the volume of Bink videos.");
    snd_drawEqEnts = Dvar_RegisterBool(
        "snd_drawEqEnts",
        0,
        0x81u,
        "Show which ents can have EQ turned on/off, which ones are on (green) and off (magenta)");
    snd_drawEqChannels = Dvar_RegisterBool("snd_drawEqChannels", 0, 0x81u, "Draw overlay of EQ settings for each channel");
    snd_debugReplace = Dvar_RegisterBool(
        "snd_debugReplace",
        0,
        0x81u,
        "Print out information about when we stop a playing sound to play another");
    snd_debugAlias = Dvar_RegisterString(
        "snd_debugAlias",
        (char *)"",
        0x80u,
        "Print out tracking information about a particular alias");
    snd_touchStreamFilesOnLoad = Dvar_RegisterBool(
        "snd_touchStreamFilesOnLoad",
        0,
        1u,
        "Check whether stream sound files exist while loading");
    g_snd.effect = g_snd.envEffects;
    g_snd.envEffects[0].roomtype = 0;
    g_snd.envEffects[0].drylevel = 1.0;
    g_snd.effect->drygoal = 1.0;
    g_snd.effect->dryrate = 0.0;
    g_snd.effect->wetlevel = 0.0;
    g_snd.effect->wetgoal = 0.0;
    g_snd.effect->wetrate = 0.0;
    g_snd.effect->active = 1;
    g_snd.amplifier.listener = &g_snd.listeners[1];
    SND_InitEntChannels();
    g_snd.playbackIdCounter = 1;
    g_snd.mastervol.volume = 1.0;
    g_snd.mastervol.goalvolume = 1.0;
    g_snd.mastervol.goalrate = 0.0;
    g_snd.channelvol = g_snd.channelVolGroups;
    for (i = 0; i < SND_GetEntChannelCount(); ++i)
    {
        g_snd.channelvol->channelvol[i].volume = 1.0;
        g_snd.channelvol->channelvol[i].goalvolume = 1.0;
        g_snd.channelvol->channelvol[i].goalrate = 0.0;
    }
    g_snd.channelvol->active = 1;
    g_snd.time = Sys_Milliseconds();
    g_snd.looptime = g_snd.time;
    g_snd.slaveLerp = 0.0;
    g_snd.volume = snd_volume->current.value * 0.75;
    for (ia = 0; ia < SND_GetEntChannelCount(); ++ia)
        g_snd.defaultPauseSettings[ia] = SND_IsPausable(ia);
    SND_ResetPauseSettingsToDefaults();
    g_sndPhysics.count = 0;
    Cmd_AddCommandInternal(
        "snd_setEnvironmentEffects",
        SND_SetEnvironmentEffects_f,
        &SND_SetEnvironmentEffects_f_VAR);
    Cmd_AddCommandInternal(
        "snd_deactivateEnvironmentEffects",
        SND_DeactivateEnvironmentEffects_f,
        &SND_DeactivateEnvironmentEffects_f_VAR);
    Cmd_AddCommandInternal("snd_playLocal", SND_PlayLocal_f, &SND_PlayLocal_f_VAR);
    Cmd_AddCommandInternal("snd_setEq", SND_SetEq_f, &SND_SetEq_f_VAR);
    Cmd_AddCommandInternal("snd_setEqFreq", SND_SetEqFreq_f, &SND_SetEqFreq_f_VAR);
    Cmd_AddCommandInternal("snd_setEqGain", SND_SetEqGain_f, &SND_SetEqGain_f_VAR);
    Cmd_AddCommandInternal("snd_setEqQ", SND_SetEqQ_f, &SND_SetEqQ_f_VAR);
    Cmd_AddCommandInternal("snd_setEqType", SND_SetEqType_f, &SND_SetEqType_f_VAR);
    Cmd_AddCommandInternal("snd_deactivateEq", SND_DeactivateEq_f, &SND_DeactivateEq_f_VAR);
    Com_Printf(9, "------- sound system successfully initialized -------\n");
    Voice_Init();
}

void __cdecl SND_PlayLocal_f()
{
    const char *v0; // eax
    const char *v1; // eax
    const char *v2; // eax
    const char *v3; // eax
    const char *v4; // eax
    const char *v5; // eax
    snd_alias_t *v6; // eax
    const char *v7; // eax
    const char *v8; // eax
    const char *v9; // eax
    double v10; // [esp+Ch] [ebp-50h]
    double v11; // [esp+14h] [ebp-48h]
    double v12; // [esp+1Ch] [ebp-40h]
    float v13; // [esp+24h] [ebp-38h]
    float dist; // [esp+38h] [ebp-24h]
    float yaw; // [esp+3Ch] [ebp-20h]
    float pitch; // [esp+44h] [ebp-18h]
    const snd_alias_t *alias; // [esp+48h] [ebp-14h]
    float soundPos[4]; // [esp+4Ch] [ebp-10h] BYREF

    dist = 100.0;
    yaw = 0.0;
    pitch = 0.0;
    switch (Cmd_Argc())
    {
    case 2:
        goto $LN3_85;
    case 3:
        goto $LN4_100;
    case 4:
        goto $LN5_98;
    case 5:
        v0 = Cmd_Argv(4);
        pitch = atof(v0);
    $LN5_98:
        v1 = Cmd_Argv(3);
        yaw = atof(v1);
    $LN4_100:
        v2 = Cmd_Argv(2);
        dist = atof(v2);
    $LN3_85:
        v3 = Cmd_Argv(1);
        Com_PickSoundAlias(v3);
        v5 = Cmd_Argv(1);
        v6 = Com_PickSoundAlias(v5);
        alias = v6;
        if (v6)
        {
            RelativeToListener(g_snd.listeners, yaw, pitch, dist, soundPos);
            SND_PlaySoundAlias_Internal(alias, alias, 0.0, 1.0, 0, soundPos, 0, 0, 0, 1, SASYS_CGAME);
            v12 = soundPos[2];
            v11 = soundPos[1];
            v10 = soundPos[0];
            v8 = Cmd_Argv(1);
            Com_Printf(14, "Playing local sound alias \"%s\" at (%.2f, %.2f, %.2f).\n", v8, v10, v11, v12);
            if (dist >= 400.0)
                v13 = 1.5;
            else
                v13 = 0.5;
            soundPos[3] = v13;
            v9 = Cmd_Argv(1);

            // KISAKTODO: lmao
            CL_AddDebugStarWithText(soundPos, (const float *)"333?333?", (const float *)"333?333?", v9, v13, 200, 0);
        }
        else
        {
            v7 = Cmd_Argv(1);
            Com_Printf(14, "Couldn't find sound alias \"%s\".\n", v7);
        }
        break;
    default:
        v4 = Cmd_Argv(0);
        Com_Printf(0, "USAGE: %s <sndalias> [<dist> <yaw> <pitch>]\n", v4);
        break;
    }
}

void __cdecl RelativeToListener(const snd_listener *listener, float yaw, float pitch, float dist, float *result)
{
    float inputAngles[3]; // [esp+8h] [ebp-24h] BYREF
    float sndDir[3]; // [esp+14h] [ebp-18h] BYREF
    float clientAngles[3]; // [esp+20h] [ebp-Ch] BYREF

    inputAngles[1] = -yaw;
    inputAngles[0] = -pitch;
    inputAngles[2] = 0.0;
    AxisToAngles(listener->orient.axis, clientAngles);
    inputAngles[1] = inputAngles[1] + clientAngles[1];
    AngleVectors(inputAngles, sndDir, 0, 0);
    Vec3Scale(sndDir, dist, result);
    Vec3Add(listener->orient.origin, result, result);
}

void SND_InitEntChannels()
{
    char *buffer; // [esp+0h] [ebp-4h]

    buffer = Com_LoadRawTextFile("soundaliases/channels.def");
    if (!buffer)
        Com_Error(ERR_DROP, "unable to load entity channel file [%s].\n", "soundaliases/channels.def");
    SND_ParseEntChannelFile(buffer);
    Com_UnloadRawTextFile(buffer);
    BG_RegisterShockVolumeDvars();
}

void __cdecl SND_ParseEntChannelFile(const char *buffer)
{
    char v1; // [esp+3h] [ebp-7Dh]
    snd_entchannel_info_t *v2; // [esp+8h] [ebp-78h]
    char *v3; // [esp+Ch] [ebp-74h]
    char v4; // [esp+13h] [ebp-6Dh]
    char *v5; // [esp+18h] [ebp-68h]
    const char *v6; // [esp+1Ch] [ebp-64h]
    char channelName[68]; // [esp+30h] [ebp-50h] BYREF
    int maxVoices; // [esp+78h] [ebp-8h]
    const char *value; // [esp+7Ch] [ebp-4h]

    if (!buffer)
        MyAssertHandler(".\\snd.cpp", 3523, 0, "%s", "buffer");
    g_snd.entchannel_count = 0;
    Com_BeginParseSession("soundaliases/channels.def");
    Com_SetCSV(1);
    while (1)
    {
        value = (const char *)Com_Parse(&buffer);
        if (!buffer)
            break;
        if (*value && *value != 35)
        {
            if (strlen(value) > 0x40)
            {
                Com_EndParseSession();
                Com_Error(
                    ERR_DROP,
                    "channel name too long (max chars %d): %s in file [%s].\n",
                    64,
                    value,
                    "soundaliases/channels.def");
            }
            v6 = value;
            v5 = channelName;
            do
            {
                v4 = *v6;
                *v5++ = *v6++;
            } while (v4);
            value = (const char *)Com_ParseOnLine(&buffer);
            if (*value)
                g_snd.entchaninfo[g_snd.entchannel_count].priority = atoi(value);
            else
                g_snd.entchaninfo[g_snd.entchannel_count].priority = 0;
            value = (const char *)Com_ParseOnLine(&buffer);
            g_snd.entchaninfo[g_snd.entchannel_count].is3d = SND_BooleanFromString(value, "3d", "2d", 0);
            value = (const char *)Com_ParseOnLine(&buffer);
            g_snd.entchaninfo[g_snd.entchannel_count].isRestricted = SND_BooleanFromString(
                value,
                "restricted",
                "unrestricted",
                1);
            value = (const char *)Com_ParseOnLine(&buffer);
            g_snd.entchaninfo[g_snd.entchannel_count].isPausable = SND_BooleanFromString(value, "pause", "nopause", 1);
            value = (const char *)Com_ParseOnLine(&buffer);
            if (*value)
            {
                maxVoices = atoi(value);
                if (maxVoices <= 0)
                {
                    maxVoices = 53;
                    Com_PrintError(
                        9,
                        "channel '%s' has nonnumeric or negative value (%s) in file [%s], defaulting to max (%i).\n",
                        channelName,
                        value,
                        "soundaliases/channels.def",
                        53);
                }
                if (maxVoices > 53)
                {
                    maxVoices = 53;
                    Com_PrintError(
                        9,
                        "max number (%d) of voices exceeded for channel '%s' in file [%s], defaulting to max (%i).\n",
                        53,
                        channelName,
                        "soundaliases/channels.def",
                        53);
                }
            }
            else
            {
                maxVoices = 53;
            }
            v3 = channelName;
            v2 = &g_snd.entchaninfo[g_snd.entchannel_count];
            do
            {
                v1 = *v3;
                v2->name[0] = *v3++;
                v2 = (snd_entchannel_info_t *)((char *)v2 + 1);
            } while (v1);
            g_snd.entchaninfo[g_snd.entchannel_count].maxVoices = maxVoices;
            g_snd.entchaninfo[g_snd.entchannel_count++].voiceCount = 0;
            if (g_snd.entchannel_count > 64)
            {
                Com_EndParseSession();
                Com_Error(ERR_DROP, "channel definition file exceeded max number of channels (%d).\n", 64);
            }
        }
        Com_SkipRestOfLine(&buffer);
    }
    Com_EndParseSession();
}

char __cdecl SND_BooleanFromString(const char *value, const char *trueValue, const char *falseValue, bool defaultValue)
{
    if (!*value)
        return defaultValue;
    if (!I_stricmp(value, trueValue))
        return 1;
    if (!I_stricmp(value, falseValue))
        return 0;
    Com_PrintError(
        9,
        "unknown value (%s), should be either '%s' or '%s'.  using default: %d.\n",
        value,
        trueValue,
        falseValue,
        defaultValue);
    return defaultValue;
}

void __cdecl SND_Shutdown()
{
    Voice_Shutdown();
    if (g_snd.Initialized2d)
    {
        SND_StopSounds(SND_STOP_ALL);
        Com_UnloadSoundAliases(SASYS_CGAME);
        Com_UnloadSoundAliases(SASYS_UI);
        SND_ShutdownDriver();
        memset((unsigned __int8 *)&g_snd, 0, sizeof(g_snd));
        Cmd_RemoveCommand("snd_setEnvironmentEffects");
        Cmd_RemoveCommand("snd_deactivateEnvironmentEffects");
        Cmd_RemoveCommand("snd_playLocal");
        Cmd_RemoveCommand("snd_setEq");
        Cmd_RemoveCommand("snd_setEqFreq");
        Cmd_RemoveCommand("snd_setEqGain");
        Cmd_RemoveCommand("snd_setEqQ");
        Cmd_RemoveCommand("snd_setEqType");
        Cmd_RemoveCommand("snd_deactivateEq");
    }
}

void __cdecl SND_ShutdownChannels()
{
    SND_StopSounds(SND_STOP_ALL);
    memset((unsigned __int8 *)g_snd.chaninfo, 0, sizeof(g_snd.chaninfo));
}

void __cdecl SND_ErrorCleanup()
{
    g_snd.restore.size = 0;
}

void __cdecl SND_Save(MemoryFile *memFile)
{
    int i; // [esp+0h] [ebp-4h]
    int ia; // [esp+0h] [ebp-4h]
    int ib; // [esp+0h] [ebp-4h]
    int ic; // [esp+0h] [ebp-4h]
    int id; // [esp+0h] [ebp-4h]

    for (i = 1; i < 4; ++i)
        MemFile_WriteData(memFile, 772, &g_snd.channelVolGroups[i]);
    for (ia = 1; ia < 3; ++ia)
        MemFile_WriteData(memFile, 32, &g_snd.envEffects[ia]);
    SND_SaveEq(memFile);
    MemFile_WriteData(memFile, 8, g_snd.background);
    if (g_snd.Initialized2d)
    {
        for (ib = 8; ib < g_snd.max_3D_channels + 8; ++ib)
            SND_Save3DChannel(ib, memFile);
    }
    MemFile_WriteCString(memFile, (char *)"");
    if (g_snd.Initialized2d)
    {
        for (ic = 0; ic < g_snd.max_2D_channels; ++ic)
            SND_Save2DChannel(ic, memFile);
    }
    MemFile_WriteCString(memFile, (char *)"");
    if (g_snd.Initialized2d)
    {
        for (id = 40; id < g_snd.max_stream_channels + 40; ++id)
            SND_SaveStreamChannel(id, memFile);
    }
    MemFile_WriteCString(memFile, (char *)"");
}

void __cdecl SND_Save3DChannel(int chanIndex, MemoryFile *memFile)
{
    snd_save_3D_sample_t info; // [esp+0h] [ebp-18h] BYREF

    if (chanIndex < 8 || chanIndex >= g_snd.max_3D_channels + 8)
        MyAssertHandler(
            ".\\snd.cpp",
            3930,
            0,
            "%s\n\t(chanIndex) = %i",
            "(chanIndex >= (0 + 8) && chanIndex < (0 + 8) + g_snd.max_3D_channels)",
            chanIndex);
    if (!SND_Is3DChannelFree(chanIndex)
        && g_snd.chaninfo[chanIndex].system == SASYS_CGAME
        && SND_Get3DChannelLength(chanIndex))
    {
        if (SND_Get3DChannelPlaybackRate(chanIndex))
        {
            SND_Get3DChannelSaveInfo(chanIndex, &info);
            SND_SaveSoundAlias(g_snd.chaninfo[chanIndex].alias0, memFile);
            SND_SaveSoundAlias(g_snd.chaninfo[chanIndex].alias1, memFile);
            SND_SaveChanInfo(&g_snd.chaninfo[chanIndex], memFile);
            MemFile_WriteData(memFile, 24, &info);
        }
    }
}

void __cdecl SND_SaveSoundAlias(const snd_alias_t *alias, MemoryFile *memFile)
{
    __int16 p; // [esp+2h] [ebp-2h] BYREF

    MemFile_WriteCString(memFile, (char *)alias->aliasName);
    p = SND_GetAliasOffset(alias);
    MemFile_WriteData(memFile, 2, &p);
}

void __cdecl SND_SaveChanInfo(const snd_channel_info_t *chaninfo, MemoryFile *memFile)
{
    float lerp; // [esp+0h] [ebp-18h] BYREF
    float basevolume; // [esp+4h] [ebp-14h] BYREF
    int startTime; // [esp+8h] [ebp-10h] BYREF
    int startDelay; // [esp+Ch] [ebp-Ch] BYREF
    bool timescale; // [esp+13h] [ebp-5h] BYREF
    bool master; // [esp+14h] [ebp-4h] BYREF
    char entchannel; // [esp+15h] [ebp-3h] BYREF
    __int16 p; // [esp+16h] [ebp-2h] BYREF

    if (!chaninfo)
        MyAssertHandler(".\\snd.cpp", 3893, 0, "%s", "chaninfo");
    if (chaninfo->sndEnt.field.entIndex != (unsigned __int16)chaninfo->sndEnt.field.entIndex)
        MyAssertHandler(
            ".\\snd.cpp",
            3894,
            0,
            "%s\n\t(chaninfo->sndEnt.handle) = %i",
            "(chaninfo->sndEnt.handle == (chaninfo->sndEnt.handle & 0xFFFF))",
            chaninfo->sndEnt.field.entIndex);
    if (chaninfo->entchannel != (unsigned __int8)chaninfo->entchannel)
        MyAssertHandler(
            ".\\snd.cpp",
            3895,
            0,
            "%s\n\t(chaninfo->entchannel) = %i",
            "(chaninfo->entchannel == (chaninfo->entchannel & 0xFF))",
            chaninfo->entchannel);
    p = chaninfo->sndEnt.handle;
    MemFile_WriteData(memFile, 2, &p);
    entchannel = chaninfo->entchannel;
    MemFile_WriteData(memFile, 1, &entchannel);
    master = chaninfo->master;
    MemFile_WriteData(memFile, 1, &master);
    timescale = chaninfo->timescale;
    MemFile_WriteData(memFile, 1, &timescale);
    startDelay = chaninfo->startDelay;
    MemFile_WriteData(memFile, 4, &startDelay);
    startTime = chaninfo->startTime;
    MemFile_WriteData(memFile, 4, &startTime);
    basevolume = chaninfo->basevolume;
    MemFile_WriteData(memFile, 4, &basevolume);
    lerp = chaninfo->lerp;
    MemFile_WriteData(memFile, 4, &lerp);
    MemFile_WriteData(memFile, 12, chaninfo->offset);
    SND_SaveLengthNotifyInfo(&chaninfo->lengthNotifyInfo, memFile);
}

void __cdecl SND_SaveLengthNotifyInfo(const sndLengthNotifyInfo *info, MemoryFile *memFile)
{
    SndLengthId v2; // [esp+0h] [ebp-14h]
    void *v3; // [esp+4h] [ebp-10h] BYREF
    char v4; // [esp+Bh] [ebp-9h] BYREF
    int p; // [esp+Ch] [ebp-8h] BYREF
    int i; // [esp+10h] [ebp-4h]

    if (!info)
        MyAssertHandler(".\\snd.cpp", 3827, 0, "%s", "info");
    if (info->count >= 4u)
        MyAssertHandler(
            ".\\snd.cpp",
            3828,
            0,
            "info->count doesn't index SND_LENGTHNOTIFY_COUNT\n\t%i not in [0, %i)",
            info->count,
            4);
    if (!memFile)
        MyAssertHandler(".\\snd.cpp", 3829, 0, "%s", "memFile");
    p = info->count;
    MemFile_WriteData(memFile, 4, &p);
    if (info->count)
    {
        for (i = 0; i < info->count; ++i)
        {
            v4 = info->id[i];
            MemFile_WriteData(memFile, 1, &v4);
            v2 = info->id[i];
            if (v2)
            {
                if (v2 == SndLengthNotify_Subtitle)
                    SND_SaveSoundAlias((const snd_alias_t *)info->data[i], memFile);
            }
            else
            {
                v3 = info->data[i];
                MemFile_WriteData(memFile, 4, &v3);
            }
        }
    }
}

void __cdecl SND_Save2DChannel(int chanIndex, MemoryFile *memFile)
{
    snd_save_2D_sample_t info; // [esp+0h] [ebp-10h] BYREF

    if (chanIndex < 0 || chanIndex >= g_snd.max_2D_channels)
        MyAssertHandler(
            ".\\snd.cpp",
            4011,
            0,
            "%s\n\t(chanIndex) = %i",
            "(chanIndex >= 0 && chanIndex < 0 + g_snd.max_2D_channels)",
            chanIndex);
    if (!SND_Is2DChannelFree(chanIndex)
        && g_snd.chaninfo[chanIndex].system == SASYS_CGAME
        && SND_Get2DChannelLength(chanIndex))
    {
        if (SND_Get2DChannelPlaybackRate(chanIndex))
        {
            SND_Get2DChannelSaveInfo(chanIndex, &info);
            SND_SaveSoundAlias(g_snd.chaninfo[chanIndex].alias0, memFile);
            SND_SaveSoundAlias(g_snd.chaninfo[chanIndex].alias1, memFile);
            SND_SaveChanInfo(&g_snd.chaninfo[chanIndex], memFile);
            MemFile_WriteData(memFile, 16, &info);
        }
    }
}

void __cdecl SND_SaveStreamChannel(int chanIndex, MemoryFile *memFile)
{
    snd_save_stream_t info; // [esp+8h] [ebp-24h] BYREF
    bool isStreamForUi; // [esp+2Bh] [ebp-1h]

    if (chanIndex < 40 || chanIndex >= g_snd.max_stream_channels + 40)
        MyAssertHandler(
            ".\\snd.cpp",
            4092,
            0,
            "%s\n\t(chanIndex) = %i",
            "(chanIndex >= ((0 + 8) + 32) && chanIndex < ((0 + 8) + 32) + g_snd.max_stream_channels)",
            chanIndex);
    if (chanIndex < 45)
    {
        if (chanIndex >= 41)
            return;
        if (SND_IsStreamChannelFree(chanIndex))
        {
            MemFile_WriteCString(memFile, (char *)"");
            return;
        }
    }
    else if (SND_IsStreamChannelFree(chanIndex))
    {
        return;
    }
    isStreamForUi = g_snd.chaninfo[chanIndex].system != SASYS_CGAME;
    if (!isStreamForUi && SND_GetStreamChannelLength(chanIndex))
    {
        SND_GetStreamChannelSaveInfo(chanIndex, &info);
        SND_SaveSoundAlias(g_snd.chaninfo[chanIndex].alias0, memFile);
        SND_SaveSoundAlias(g_snd.chaninfo[chanIndex].alias1, memFile);
        SND_SaveChanInfo(&g_snd.chaninfo[chanIndex], memFile);
        MemFile_WriteData(memFile, 32, &info);
    }
    else if (chanIndex < 45)
    {
        MemFile_WriteCString(memFile, (char *)"");
    }
}

void __cdecl SND_Restore(MemoryFile *memFile)
{
    int i; // [esp+8h] [ebp-4h]
    int ia; // [esp+8h] [ebp-4h]
    int ib; // [esp+8h] [ebp-4h]
    int ic; // [esp+8h] [ebp-4h]
    int id; // [esp+8h] [ebp-4h]

    if (g_snd.Initialized2d)
    {
        for (i = 1; i < 4; ++i)
            MemFile_ReadData(memFile, 772, (unsigned __int8 *)&g_snd.channelVolGroups[i]);
        for (ia = 0; ia < 4; ++ia)
        {
            if (g_snd.channelVolGroups[ia].active)
                g_snd.channelvol = &g_snd.channelVolGroups[ia];
        }
        for (ib = 1; ib < 3; ++ib)
            MemFile_ReadData(memFile, 32, (unsigned __int8 *)&g_snd.envEffects[ib]);
        SND_RestoreEq(memFile);
        for (ic = 0; ic < 3; ++ic)
        {
            if (g_snd.envEffects[ic].active)
                g_snd.effect = &g_snd.envEffects[ic];
        }
        SND_SetRoomtype(g_snd.effect->roomtype);
        MemFile_ReadData(memFile, 8, (unsigned __int8 *)g_snd.background);
        while (SND_Restore3DChannel(memFile))
            ;
        while (SND_Restore2DChannel(memFile))
            ;
        for (id = 0; id < 5; ++id)
        {
            if (id + 40 < 41 || id + 40 > 44)
                SND_RestoreStreamChannel(id + 40, memFile);
        }
        while (SND_RestoreStreamChannel(-1, memFile))
            ;
    }
}

char __cdecl SND_Restore3DChannel(MemoryFile *memFile)
{
    float *offset; // [esp+Ch] [ebp-F4h]
    snd_save_3D_sample_t info; // [esp+10h] [ebp-F0h] BYREF
    int playbackId; // [esp+28h] [ebp-D8h]
    SndStartAliasInfo startAliasInfo; // [esp+2Ch] [ebp-D4h] BYREF
    int channel; // [esp+64h] [ebp-9Ch] BYREF
    snd_alias_t *alias1; // [esp+68h] [ebp-98h]
    snd_alias_t *alias0; // [esp+6Ch] [ebp-94h]
    snd_channel_info_t chaninfo; // [esp+70h] [ebp-90h] BYREF

    alias0 = SND_RestoreSoundAlias(memFile);
    if (!alias0)
        return 0;
    alias1 = SND_RestoreSoundAlias(memFile);
    if (!alias1)
        MyAssertHandler(".\\snd.cpp", 3964, 0, "%s", "alias1");
    SND_RestoreChanInfo(&chaninfo, memFile);
    MemFile_ReadData(memFile, 24, (unsigned __int8 *)&info);
    if (alias0->soundFile->type != (alias0->flags & 0xC0) >> 6)
        MyAssertHandler(".\\snd.cpp", 3969, 0, "%s", "alias0->soundFile->type == SNDALIASFLAGS_GET_TYPE( alias0->flags )");
    if (alias1->soundFile->type != (alias1->flags & 0xC0) >> 6)
        MyAssertHandler(".\\snd.cpp", 3970, 0, "%s", "alias1->soundFile->type == SNDALIASFLAGS_GET_TYPE( alias1->flags )");
    if (alias0->soundFile == alias1->soundFile
        && alias0->soundFile->exists
        && alias0->soundFile->type == 1
        && SND_ValidateSoundAliasBlend(alias0, alias1, 0))
    {
        if (!SND_AnyActiveListeners())
            MyAssertHandler(".\\snd.cpp", 3975, 0, "%s", "SND_AnyActiveListeners()");
        if (!snd_enable3D->current.enabled)
            return 1;
        startAliasInfo.alias0 = alias0;
        startAliasInfo.alias1 = alias1;
        startAliasInfo.lerp = chaninfo.lerp;
        startAliasInfo.sndEnt.field.entIndex = chaninfo.sndEnt.field.entIndex;
        startAliasInfo.org[0] = info.org[0];
        startAliasInfo.org[1] = info.org[1];
        startAliasInfo.org[2] = info.org[2];
        startAliasInfo.volume = chaninfo.basevolume;
        startAliasInfo.pitch = info.pitch;
        startAliasInfo.timeshift = 0;
        startAliasInfo.fraction = info.fraction;
        startAliasInfo.startDelay = chaninfo.startDelay;
        startAliasInfo.master = chaninfo.master;
        startAliasInfo.system = SASYS_CGAME;
        startAliasInfo.timescale = chaninfo.timescale;
        playbackId = SND_StartAlias3DSample(&startAliasInfo, &channel);
        if (playbackId != -1)
        {
            if (channel < 8 || channel >= g_snd.max_3D_channels + 8)
                MyAssertHandler(
                    ".\\snd.cpp",
                    3996,
                    0,
                    "%s\n\t(channel) = %i",
                    "(channel >= (0 + 8) && channel < (0 + 8) + g_snd.max_3D_channels)",
                    channel);
            offset = g_snd.chaninfo[channel].offset;
            *offset = chaninfo.offset[0];
            offset[1] = chaninfo.offset[1];
            offset[2] = chaninfo.offset[2];
            memcpy(
                &g_snd.chaninfo[channel].lengthNotifyInfo,
                &chaninfo.lengthNotifyInfo,
                sizeof(g_snd.chaninfo[channel].lengthNotifyInfo));
        }
    }
    return 1;
}

snd_alias_t *__cdecl SND_RestoreSoundAlias(MemoryFile *memFile)
{
    _WORD p[4]; // [esp+0h] [ebp-10h] BYREF
    const char *name; // [esp+8h] [ebp-8h]
    snd_alias_t *alias; // [esp+Ch] [ebp-4h]

    name = MemFile_ReadCString(memFile);
    if (!*name)
        return 0;
    MemFile_ReadData(memFile, 2, (unsigned __int8 *)p);
    p[2] = p[0];
    alias = SND_GetAliasWithOffset(name, p[0]);
    if (!alias)
        MyAssertHandler(".\\snd.cpp", 3820, 0, "%s", "alias");
    return alias;
}

void __cdecl SND_RestoreChanInfo(snd_channel_info_t *chaninfo, MemoryFile *memFile)
{
    int v2; // [esp+2Ch] [ebp-10h] BYREF
    int v3; // [esp+30h] [ebp-Ch] BYREF
    unsigned __int8 v4; // [esp+35h] [ebp-7h] BYREF
    unsigned __int8 v5; // [esp+36h] [ebp-6h] BYREF
    unsigned __int8 v6; // [esp+37h] [ebp-5h] BYREF
    unsigned __int8 p[4]; // [esp+38h] [ebp-4h] BYREF

    memset((unsigned __int8 *)chaninfo, 0, sizeof(snd_channel_info_t));
    MemFile_ReadData(memFile, 2, p);
    chaninfo->sndEnt.field.entIndex = *(unsigned __int16 *)p;
    MemFile_ReadData(memFile, 1, &v6);
    chaninfo->entchannel = v6;
    MemFile_ReadData(memFile, 1, &v5);
    chaninfo->master = v5 != 0;
    MemFile_ReadData(memFile, 1, &v4);
    chaninfo->timescale = v4 != 0;
    MemFile_ReadData(memFile, 4, (unsigned __int8 *)&v3);
    chaninfo->startDelay = v3;
    MemFile_ReadData(memFile, 4, (unsigned __int8 *)&v2);
    chaninfo->startTime = v2;
    chaninfo->basevolume = MemFile_ReadFloat(memFile);
    chaninfo->lerp = MemFile_ReadFloat(memFile);
    chaninfo->offset[0] = MemFile_ReadFloat(memFile);
    chaninfo->offset[1] = MemFile_ReadFloat(memFile);
    chaninfo->offset[2] = MemFile_ReadFloat(memFile);
    SND_RestoreLengthNotifyInfo(memFile, &chaninfo->lengthNotifyInfo);
}

void __cdecl SND_RestoreLengthNotifyInfo(MemoryFile *memFile, sndLengthNotifyInfo *info)
{
    snd_alias_t *v2; // eax
    void *v3; // [esp+4h] [ebp-14h] BYREF
    unsigned __int8 v4; // [esp+Bh] [ebp-Dh] BYREF
    int p; // [esp+Ch] [ebp-Ch] BYREF
    int i; // [esp+10h] [ebp-8h]
    SndLengthId id; // [esp+14h] [ebp-4h]

    if (!info)
        MyAssertHandler(".\\snd.cpp", 3855, 0, "%s", "info");
    if (!memFile)
        MyAssertHandler(".\\snd.cpp", 3856, 0, "%s", "memFile");
    MemFile_ReadData(memFile, 4, (unsigned __int8 *)&p);
    info->count = p;
    if (info->count >= 4u)
        MyAssertHandler(
            ".\\snd.cpp",
            3862,
            0,
            "info->count doesn't index SND_LENGTHNOTIFY_COUNT\n\t%i not in [0, %i)",
            info->count,
            4);
    if (info->count)
    {
        for (i = 0; i < info->count; ++i)
        {
            MemFile_ReadData(memFile, 1, &v4);
            id = (SndLengthId)v4;
            if (v4 >= 2u)
                MyAssertHandler(".\\snd.cpp", 3869, 0, "id doesn't index SndLengthNotifyCount\n\t%i not in [0, %i)", id, 2);
            info->id[i] = id;
            if (id)
            {
                if (id == SndLengthNotify_Subtitle)
                {
                    v2 = SND_RestoreSoundAlias(memFile);
                    info->data[i] = (void *)v2;
                }
            }
            else
            {
                MemFile_ReadData(memFile, 4, (unsigned __int8 *)&v3);
                info->data[i] = v3;
            }
        }
    }
}

char __cdecl SND_Restore2DChannel(MemoryFile *memFile)
{
    float *offset; // [esp+Ch] [ebp-ECh]
    snd_save_2D_sample_t info; // [esp+10h] [ebp-E8h] BYREF
    int playbackId; // [esp+20h] [ebp-D8h]
    SndStartAliasInfo startAliasInfo; // [esp+24h] [ebp-D4h] BYREF
    int channel; // [esp+5Ch] [ebp-9Ch] BYREF
    snd_alias_t *alias1; // [esp+60h] [ebp-98h]
    snd_alias_t *alias0; // [esp+64h] [ebp-94h]
    snd_channel_info_t chaninfo; // [esp+68h] [ebp-90h] BYREF

    alias0 = SND_RestoreSoundAlias(memFile);
    if (!alias0)
        return 0;
    alias1 = SND_RestoreSoundAlias(memFile);
    if (!alias1)
        MyAssertHandler(".\\snd.cpp", 4045, 0, "%s", "alias1");
    SND_RestoreChanInfo(&chaninfo, memFile);
    MemFile_ReadData(memFile, 16, (unsigned __int8 *)&info);
    if (alias0->soundFile->type != (alias0->flags & 0xC0) >> 6)
        MyAssertHandler(".\\snd.cpp", 4050, 0, "%s", "alias0->soundFile->type == SNDALIASFLAGS_GET_TYPE( alias0->flags )");
    if (alias1->soundFile->type != (alias1->flags & 0xC0) >> 6)
        MyAssertHandler(".\\snd.cpp", 4051, 0, "%s", "alias1->soundFile->type == SNDALIASFLAGS_GET_TYPE( alias1->flags )");
    if (alias0->soundFile == alias1->soundFile
        && alias0->soundFile->exists
        && alias0->soundFile->type == 1
        && SND_ValidateSoundAliasBlend(alias0, alias1, 0))
    {
        if (!snd_enable2D->current.enabled)
            return 1;
        startAliasInfo.alias0 = alias0;
        startAliasInfo.alias1 = alias1;
        startAliasInfo.lerp = chaninfo.lerp;
        startAliasInfo.sndEnt.field.entIndex = chaninfo.sndEnt.field.entIndex;
        startAliasInfo.volume = chaninfo.basevolume;
        startAliasInfo.pitch = info.pitch;
        startAliasInfo.timeshift = 0;
        startAliasInfo.fraction = info.fraction;
        startAliasInfo.startDelay = chaninfo.startDelay;
        startAliasInfo.master = chaninfo.master;
        startAliasInfo.system = SASYS_CGAME;
        startAliasInfo.timescale = chaninfo.timescale;
        playbackId = SND_StartAlias2DSample(&startAliasInfo, &channel);
        if (playbackId != -1)
        {
            if (channel < 0 || channel >= g_snd.max_2D_channels)
                MyAssertHandler(
                    ".\\snd.cpp",
                    4075,
                    0,
                    "%s\n\t(channel) = %i",
                    "(channel >= 0 && channel < 0 + g_snd.max_2D_channels)",
                    channel);
            SND_Set2DChannelFromSaveInfo(channel, &info);
            offset = g_snd.chaninfo[channel].offset;
            *offset = chaninfo.offset[0];
            offset[1] = chaninfo.offset[1];
            offset[2] = chaninfo.offset[2];
            memcpy(
                &g_snd.chaninfo[channel].lengthNotifyInfo,
                &chaninfo.lengthNotifyInfo,
                sizeof(g_snd.chaninfo[channel].lengthNotifyInfo));
        }
    }
    return 1;
}

char __cdecl SND_RestoreStreamChannel(int channel, MemoryFile *memFile)
{
    const char *v3; // eax
    const char *v4; // eax
    float *offset; // [esp+14h] [ebp-120h]
    float v6; // [esp+1Ch] [ebp-118h]
    float v7; // [esp+30h] [ebp-104h]
    int playbackId; // [esp+40h] [ebp-F4h]
    snd_save_stream_t info; // [esp+44h] [ebp-F0h] BYREF
    SndStartAliasInfo startAliasInfo; // [esp+64h] [ebp-D0h] BYREF
    snd_alias_t *alias1; // [esp+9Ch] [ebp-98h]
    snd_alias_t *alias0; // [esp+A0h] [ebp-94h]
    snd_channel_info_t chaninfo; // [esp+A4h] [ebp-90h] BYREF

    alias0 = SND_RestoreSoundAlias(memFile);
    if (!alias0)
        return 0;
    alias1 = SND_RestoreSoundAlias(memFile);
    if (!alias1)
        MyAssertHandler(".\\snd.cpp", 4144, 0, "%s", "alias1");
    SND_RestoreChanInfo(&chaninfo, memFile);
    MemFile_ReadData(memFile, 32, (unsigned __int8 *)&info);
    if ((alias0->flags & 0xC0) >> 6 == 2
        && (alias1->flags & 0xC0) >> 6 == 2
        && SND_ValidateSoundAliasBlend(alias0, alias1, 0))
    {
        startAliasInfo.alias0 = alias0;
        startAliasInfo.alias1 = alias1;
        startAliasInfo.lerp = chaninfo.lerp;
        startAliasInfo.sndEnt.field.entIndex = chaninfo.sndEnt.field.entIndex;
        startAliasInfo.org[0] = info.org[0];
        startAliasInfo.org[1] = info.org[1];
        startAliasInfo.org[2] = info.org[2];
        startAliasInfo.volume = info.basevolume;
        startAliasInfo.pitch = 1.0;
        startAliasInfo.timeshift = 0;
        startAliasInfo.fraction = info.fraction;
        startAliasInfo.startDelay = chaninfo.startDelay;
        startAliasInfo.system = SASYS_CGAME;
        startAliasInfo.timescale = chaninfo.timescale;
        if (channel >= 0)
        {
            if (SND_IsAliasChannel3D((alias0->flags & 0x3F00) >> 8) || channel >= 45)
            {
                v3 = va(
                    "sound alias '%s' on aliaschannel #%d tried to play on stream channel #%d",
                    alias0->aliasName,
                    (alias0->flags & 0x3F00) >> 8,
                    channel - 40);
                MyAssertHandler(
                    ".\\snd.cpp",
                    4173,
                    0,
                    "%s\n\t%s",
                    "!SND_IsAliasChannel3D( SNDALIASFLAGS_GET_CHANNEL( alias0->flags ) ) && channel < SND_FIRST_STREAM_CHANNEL + SND_TRACK_COUNT",
                    v3);
            }
            if (!snd_enableStream->current.enabled)
                return 1;
            startAliasInfo.master = chaninfo.master;
            playbackId = SND_StartAliasStreamOnChannel(&startAliasInfo, channel);
        }
        else
        {
            startAliasInfo.master = 0;
            playbackId = SND_StartAliasStream(&startAliasInfo, &channel);
        }
        if (playbackId != -1 && !SND_IsStreamChannelFree(channel))
        {
            v4 = va("restarted at %.3f", info.fraction);
            SND_DebugAliasPrint(playbackId != -1, startAliasInfo.alias0, v4);
            SND_SetStreamChannelFromSaveInfo(channel, &info);
            if (g_snd.chaninfo[channel].timescale)
            {
                v7 = g_snd.timescale * ((double)info.rate * g_snd.chaninfo[channel].pitch);
                SND_SetStreamChannelPlaybackRate(channel, (int)(v7 + 9.313225746154785e-10));
            }
            else
            {
                v6 = (double)info.rate * g_snd.chaninfo[channel].pitch;
                SND_SetStreamChannelPlaybackRate(channel, (int)(v6 + 9.313225746154785e-10));
            }
            offset = g_snd.chaninfo[channel].offset;
            *offset = chaninfo.offset[0];
            offset[1] = chaninfo.offset[1];
            offset[2] = chaninfo.offset[2];
            memcpy(
                &g_snd.chaninfo[channel].lengthNotifyInfo,
                &chaninfo.lengthNotifyInfo,
                sizeof(g_snd.chaninfo[channel].lengthNotifyInfo));
        }
    }
    return 1;
}

int __cdecl SND_GetSoundOverlay(snd_overlay_type_t type, snd_overlay_info_t *info, int maxcount, int *cpu)
{
    if (!info)
        MyAssertHandler(".\\snd.cpp", 4465, 0, "%s", "info");
    if (maxcount <= 0)
        MyAssertHandler(".\\snd.cpp", 4466, 0, "%s", "maxcount > 0");
    if (!g_snd.Initialized2d)
        return 0;
    if (cpu)
        *cpu = g_snd.cpu;
    switch (type)
    {
    case SND_OVERLAY_3D:
        return SND_GetSoundOverlay3D(info, maxcount);
    case SND_OVERLAY_STREAM:
        return SND_GetSoundOverlayStream(info, maxcount);
    case SND_OVERLAY_2D:
        return SND_GetSoundOverlay2D(info, maxcount);
    }
    return 0;
}

int __cdecl SND_GetSoundOverlay2D(snd_overlay_info_t *info, int maxcount)
{
    snd_entchannel_info_t *EntChannelName; // eax
    int rate; // [esp+0h] [ebp-Ch]
    int i; // [esp+8h] [ebp-4h]

    if (maxcount > g_snd.max_2D_channels)
        maxcount = g_snd.max_2D_channels;
    for (i = 0; i < maxcount; ++i)
    {
        if (!SND_Is2DChannelFree(i) && g_snd.chaninfo[i].soundFileInfo.loadingState == SFLS_LOADED)
        {
            if (g_snd.chaninfo[i].soundFileInfo.baserate <= 0)
                MyAssertHandler(".\\snd.cpp", 4333, 0, "%s", "g_snd.chaninfo[channel].soundFileInfo.baserate > 0");
            if (!g_snd.chaninfo[i].alias0)
                MyAssertHandler(".\\snd.cpp", 4334, 0, "%s", "g_snd.chaninfo[channel].alias0");
            Com_GetSoundFileName(g_snd.chaninfo[i].alias0, info[i].pszSampleName, 128);
            Com_sprintf(info[i].aliasName, 0x40u, "%s", g_snd.chaninfo[i].alias0->aliasName);
            EntChannelName = SND_GetEntChannelName(g_snd.chaninfo[i].entchannel);
            Com_sprintf(info[i].entchannel, 0x40u, "%s", EntChannelName->name);
            rate = SND_Get2DChannelPlaybackRate(i);
            if (!rate)
                rate = g_snd.chaninfo[i].soundFileInfo.baserate;
            info[i].fPitch = (double)rate / (double)g_snd.chaninfo[i].soundFileInfo.baserate;
            info[i].fBaseVolume = g_snd.chaninfo[i].basevolume;
            info[i].fCurVolume = SND_Get2DChannelVolume(i);
            if (g_snd.volume != 0.0)
                info[i].fCurVolume = info[i].fCurVolume / g_snd.volume;
            info[i].dist = -1;
        }
        else
        {
            info[i].pszSampleName[0] = 0;
        }
    }
    return maxcount;
}

int __cdecl SND_GetSoundOverlay3D(snd_overlay_info_t *info, int maxcount)
{
    snd_entchannel_info_t *EntChannelName; // eax
    double v3; // st7
    snd_listener *a; // [esp+0h] [ebp-30h]
    float diff[3]; // [esp+8h] [ebp-28h] BYREF
    int rate; // [esp+14h] [ebp-1Ch]
    float dist; // [esp+18h] [ebp-18h]
    int channel; // [esp+1Ch] [ebp-14h]
    float org[3]; // [esp+20h] [ebp-10h] BYREF
    int i; // [esp+2Ch] [ebp-4h]

    if (maxcount > g_snd.max_3D_channels)
        maxcount = g_snd.max_3D_channels;
    for (i = 0; i < maxcount; ++i)
    {
        channel = i + 8;
        if (!SND_Is3DChannelFree(i + 8) && g_snd.chaninfo[channel].soundFileInfo.loadingState == SFLS_LOADED)
        {
            if (g_snd.chaninfo[channel].soundFileInfo.baserate <= 0)
                MyAssertHandler(".\\snd.cpp", 4375, 0, "%s", "g_snd.chaninfo[channel].soundFileInfo.baserate > 0");
            if (!g_snd.chaninfo[channel].alias0)
                MyAssertHandler(".\\snd.cpp", 4376, 0, "%s", "g_snd.chaninfo[channel].alias0");
            Com_GetSoundFileName(g_snd.chaninfo[channel].alias0, info[i].pszSampleName, 128);
            Com_sprintf(info[i].aliasName, 0x40u, "%s", g_snd.chaninfo[channel].alias0->aliasName);
            EntChannelName = SND_GetEntChannelName(g_snd.chaninfo[channel].entchannel);
            Com_sprintf(info[i].entchannel, 0x40u, "%s", EntChannelName->name);
            rate = SND_Get3DChannelPlaybackRate(channel);
            if (!rate)
                rate = g_snd.chaninfo[channel].soundFileInfo.baserate;
            info[i].fPitch = (double)rate / (double)g_snd.chaninfo[channel].soundFileInfo.baserate;
            info[i].fBaseVolume = g_snd.chaninfo[channel].basevolume;
            v3 = SND_Get3DChannelVolume(channel);
            info[i].fCurVolume = v3;
            if (g_snd.volume != 0.0)
                info[i].fCurVolume = info[i].fCurVolume / g_snd.volume;
            SND_GetCurrent3DPosition(g_snd.chaninfo[channel].sndEnt, g_snd.chaninfo[channel].offset, org);
            a = &g_snd.listeners[SND_GetListenerIndexNearestToOrigin(org)];
            Vec3Sub(a->orient.origin, org, diff);
            dist = Vec3Length(diff);
            info[i].dist = (int)dist;
        }
        else
        {
            info[i].pszSampleName[0] = 0;
        }
    }
    return maxcount;
}

int __cdecl SND_GetSoundOverlayStream(snd_overlay_info_t *info, int maxcount)
{
    snd_entchannel_info_t *EntChannelName; // eax
    double StreamChannelVolume; // st7
    int StreamChannelPlaybackRate; // [esp+0h] [ebp-30h]
    snd_listener *a; // [esp+4h] [ebp-2Ch]
    float diff[3]; // [esp+Ch] [ebp-24h] BYREF
    float dist; // [esp+18h] [ebp-18h]
    int channel; // [esp+1Ch] [ebp-14h]
    float org[3]; // [esp+20h] [ebp-10h] BYREF
    int i; // [esp+2Ch] [ebp-4h]

    if (maxcount > g_snd.max_stream_channels)
        maxcount = g_snd.max_stream_channels;
    for (i = 0; i < maxcount; ++i)
    {
        channel = i + 40;
        if (!SND_IsStreamChannelFree(i + 40) && g_snd.chaninfo[channel].soundFileInfo.loadingState == SFLS_LOADED)
        {
            if (g_snd.chaninfo[channel].soundFileInfo.baserate <= 0)
                MyAssertHandler(".\\snd.cpp", 4427, 0, "%s", "g_snd.chaninfo[channel].soundFileInfo.baserate > 0");
            if (!g_snd.chaninfo[channel].alias0)
                MyAssertHandler(".\\snd.cpp", 4428, 0, "%s", "g_snd.chaninfo[channel].alias0");
            Com_GetSoundFileName(g_snd.chaninfo[channel].alias0, info[i].pszSampleName, 128);
            Com_sprintf(info[i].aliasName, 0x40u, "%s", g_snd.chaninfo[channel].alias0->aliasName);
            EntChannelName = SND_GetEntChannelName(g_snd.chaninfo[channel].entchannel);
            Com_sprintf(info[i].entchannel, 0x40u, "%s", EntChannelName->name);
            StreamChannelPlaybackRate = SND_GetStreamChannelPlaybackRate(channel);
            info[i].fPitch = (double)StreamChannelPlaybackRate / (double)g_snd.chaninfo[channel].soundFileInfo.baserate;
            info[i].fBaseVolume = g_snd.chaninfo[channel].basevolume;
            StreamChannelVolume = SND_GetStreamChannelVolume(channel);
            info[i].fCurVolume = StreamChannelVolume;
            if (g_snd.volume != 0.0)
                info[i].fCurVolume = info[i].fCurVolume / g_snd.volume;
            dist = 0.0;
            if (SND_IsAliasChannel3D((g_snd.chaninfo[channel].alias0->flags & 0x3F00) >> 8))
            {
                SND_GetCurrent3DPosition(g_snd.chaninfo[channel].sndEnt, g_snd.chaninfo[channel].offset, org);
                a = &g_snd.listeners[SND_GetListenerIndexNearestToOrigin(org)];
                Vec3Sub(a->orient.origin, org, diff);
                dist = Vec3Length(diff);
                info[i].dist = (int)dist;
            }
            else
            {
                info[i].dist = -1;
            }
        }
        else
        {
            info[i].pszSampleName[0] = 0;
        }
    }
    return maxcount;
}

void __cdecl SND_StopChannelAndPlayChainAlias(unsigned int chanId)
{
    snd_alias_t *chainAlias; // [esp+0h] [ebp-18h]
    unsigned int sndEnt; // [esp+4h] [ebp-14h]
    float org[3]; // [esp+8h] [ebp-10h] BYREF
    snd_channel_info_t *chaninfo; // [esp+14h] [ebp-4h]

    if (chanId > 0x34)
        MyAssertHandler(
            ".\\snd.cpp",
            4507,
            0,
            "%s\n\t(chanId) = %i",
            "(( chanId >= 0 ) && ( chanId < (32 + (SND_TRACK_COUNT + 8) + 8) ))",
            chanId);
    chaninfo = &g_snd.chaninfo[chanId];
    if (!chaninfo->alias0)
        MyAssertHandler(".\\snd.cpp", 4510, 0, "%s", "chaninfo->alias0");
    if (chaninfo->alias0->chainAliasName)
    {
        sndEnt = chaninfo->sndEnt.field.entIndex;
        SND_GetCurrent3DPosition((SndEntHandle)sndEnt, chaninfo->offset, org);
        chainAlias = Com_PickSoundAlias(chaninfo->alias0->chainAliasName);
        if (chainAlias == chaninfo->alias0)
        {
            Com_PrintError(
                9,
                "Soundalias \"%s\" is trying to chain to itself - check sound .csv files and correct.\n",
                chaninfo->alias0->chainAliasName);
            StopChannel(chanId);
        }
        else
        {
            StopChannel(chanId);
            if (chainAlias)
                SND_PlaySoundAlias(chainAlias, (SndEntHandle)sndEnt, org, 0, SASYS_CGAME);
        }
    }
    else
    {
        StopChannel(chanId);
    }
}

void __cdecl StopChannel(int chanId)
{
    if ((unsigned int)chanId > 0x34)
        MyAssertHandler(
            ".\\snd.cpp",
            4490,
            0,
            "%s\n\t(chanId) = %i",
            "(( chanId >= 0 ) && ( chanId < (32 + (SND_TRACK_COUNT + 8) + 8) ))",
            chanId);
    if (chanId < 40)
    {
        if (chanId < 8)
            SND_Stop2DChannel(chanId);
        else
            SND_Stop3DChannel(chanId);
    }
    else
    {
        SND_StopStreamChannel(chanId);
    }
}

void __cdecl SND_AddPhysicsSound(snd_alias_list_t *aliasList, float *org)
{
    float *v2; // [esp+0h] [ebp-4h]

    if (!aliasList)
        MyAssertHandler(".\\snd.cpp", 4548, 0, "%s", "aliasList");
    Sys_EnterCriticalSection(CRITSECT_AUDIO_PHYSICS);
    if (g_sndPhysics.count < 32)
    {
        g_sndPhysics.info[g_sndPhysics.count].aliasList = aliasList;
        v2 = g_sndPhysics.info[g_sndPhysics.count].org;
        *v2 = *org;
        v2[1] = org[1];
        v2[2] = org[2];
        ++g_sndPhysics.count;
    }
    Sys_LeaveCriticalSection(CRITSECT_AUDIO_PHYSICS);
}

double __cdecl SND_GetVolumeNormalized()
{
    return (float)(g_snd.volume * 1.333333373069763);
}

void __cdecl SND_SetHWND(HWND hwnd)
{
    if (g_snd.Initialized2d)
        AIL_set_DirectSound_HWND(milesGlob.driver, hwnd);
}

void __cdecl SND_SetData(MssSoundCOD4 *mssSound, void *srcData)
{
    // KISAKTODO: double check MssSound struct usage here. It looks 'okay' at first glance

    _AILMIXINFO mixinfo; // [esp+Ch] [ebp-80h] BYREF
    int digitalFormat; // [esp+88h] [ebp-4h]

    if (mssSound->info.rate > g_snd.playback_rate && mssSound->info.format != 17)
    {
        memset(&mixinfo, 0, sizeof(mixinfo));
        qmemcpy(&mixinfo, mssSound, 0x24u);
        mixinfo.Info.data_ptr = srcData;
        mixinfo.Info.initial_ptr = srcData;
        while (mssSound->info.rate > g_snd.playback_rate)
        {
            mssSound->info.rate >>= 1;
            mssSound->info.samples >>= 1;
        }
        digitalFormat = MSS_DigitalFormatType(mssSound->info.format, mssSound->info.bits, mssSound->info.channels);
        mssSound->info.data_len = AIL_size_processed_digital_audio(mssSound->info.rate, digitalFormat, 1, &mixinfo);
        mssSound->data = MSS_Alloc(mssSound->info.data_len, mssSound->info.rate);
        AIL_process_digital_audio(
            mssSound->data,
            mssSound->info.data_len,
            mssSound->info.rate,
            mssSound->info.format,
            1,
            &mixinfo);
    }
    else
    {
        mssSound->data = MSS_Alloc(mssSound->info.data_len, mssSound->info.rate);
        Com_Memcpy(mssSound->data, srcData, mssSound->info.data_len);
    }
    mssSound->info.data_ptr = mssSound->data;
    mssSound->info.initial_ptr = mssSound->data;
}