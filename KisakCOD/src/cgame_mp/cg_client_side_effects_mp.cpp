#include "cg_local_mp.h"
#include "cg_public_mp.h"
#include <universal/com_files.h>

const dvar_t *cg_clientSideEffects;
int g_clientEntSoundCount;
ClientEntSound g_clientEntSounds[128];

void __cdecl CG_StartClientSideEffects(int localClientNum)
{
    char fxfilename[256]; // [esp+0h] [ebp-108h] BYREF
    const char *mapname; // [esp+104h] [ebp-4h]

    if (!cg_clientSideEffects)
        MyAssertHandler(".\\cgame_mp\\cg_client_side_effects_mp.cpp", 554, 0, "%s", "cg_clientSideEffects");
    if (!localClientNum)
        g_clientEntSoundCount = 0;
    if (cg_clientSideEffects->current.enabled)
    {
        mapname = Dvar_GetString("mapname");
        if (!mapname || !*mapname)
            MyAssertHandler(".\\cgame_mp\\cg_client_side_effects_mp.cpp", 563, 0, "%s", "mapname && mapname[0]");
        Com_sprintf(fxfilename, 0x100u, "maps/mp/%s_fx.gsc", mapname);
        CG_LoadClientEffectMapping(fxfilename);
        Com_sprintf(fxfilename, 0x100u, "maps/createfx/%s_fx.gsc", mapname);
        CG_LoadClientEffects(localClientNum, fxfilename);
    }
}

void __cdecl CG_LoadClientEffects_LoadObj(int localClientNum, const char *filename)
{
    char *buffer; // [esp+0h] [ebp-8h] BYREF
    int size; // [esp+4h] [ebp-4h]

    size = FS_ReadFile(filename, (void **)&buffer);
    if (size >= 0)
    {
        CG_ParseClientEffects(localClientNum, buffer);
        FS_FreeFile(buffer);
    }
    else
    {
        Com_PrintError(1, "file not found: %s\n", filename);
    }
}

void __cdecl CG_LoadClientEffects(int localClientNum, const char *filename)
{
    if (useFastFile->current.enabled)
        CG_LoadClientEffects_FastFile(localClientNum, filename);
    else
        CG_LoadClientEffects_LoadObj(localClientNum, filename);
}

void __cdecl CG_ParseClientEffects(int localClientNum, char *buffer)
{
    char errorText[128]; // [esp+0h] [ebp-88h] BYREF
    const char *line; // [esp+84h] [ebp-4h]

    line = CG_SkipLine(buffer, "//_createfx generated. Do not touch!!");
    if (line)
    {
        line = CG_SkipLine((char *)line, "main()");
        if (line)
        {
            line = CG_SkipLine((char *)line, "{");
            if (line)
            {
                if (CG_MatchLineStartingWith(line, "//"))
                    line = CG_SkipLineStartingWith((char *)line, "//");
                while (line && *line != 125 && *line)
                {
                    if (CG_MatchLineStartingWith(line, "ent = maps\\mp\\_createfx::createLoopSound();"))
                    {
                        line = CG_ParseSound(localClientNum, (char *)line);
                        if (!line)
                            return;
                    }
                    else
                    {
                        if (!CG_MatchLineStartingWith(line, "ent = maps\\mp\\_utility::createOneshotEffect( "))
                        {
                            I_strncpyz(errorText, (char *)line, 128);
                            Com_PrintError(
                                1,
                                "Expected 'ent = maps\\mp\\_createfx::createLoopSound();' or 'ent = maps\\mp\\_utility::createOneshotEffe"
                                "ct' instead of '%s' in map's effect file\n",
                                errorText);
                            return;
                        }
                        line = CG_ParseEffect(localClientNum, (char *)line);
                        if (!line)
                            return;
                    }
                }
                line = CG_SkipLineStartingWith((char *)line, "}");
                if (line)
                {
                    if (*line)
                    {
                        I_strncpyz(errorText, (char *)line, 128);
                        Com_PrintError(1, "Unexpected data after parsing '%s' map's effect file\n", errorText);
                    }
                }
            }
        }
    }
}

const char *__cdecl CG_SkipLine(char *line, const char *skipLine)
{
    char *linea; // [esp+8h] [ebp+8h]

    linea = CG_SkipText(line, skipLine);
    if (linea)
        return CG_SkipWhiteSpace(linea);
    else
        return 0;
}

const char *__cdecl CG_SkipWhiteSpace(const char *line)
{
    while (isspace(*line) && *line)
        ++line;
    return line;
}

char *__cdecl CG_SkipText(char *line, const char *skipText)
{
    char errorText[128]; // [esp+10h] [ebp-88h] BYREF
    int lineLength; // [esp+94h] [ebp-4h]

    lineLength = strlen(skipText);
    if (!I_strncmp(skipText, line, lineLength))
        return &line[lineLength];
    I_strncpyz(errorText, line, 128);
    Com_PrintError(1, "Unexpected text '%s' when trying to find '%s' in map's effect file\n", errorText, skipText);
    return 0;
}

const char *__cdecl CG_SkipLineStartingWith(char *line, const char *skipLine)
{
    char *linea; // [esp+8h] [ebp+8h]

    linea = CG_SkipText(line, skipLine);
    if (linea)
        return CG_SkipRestOfLine(linea);
    else
        return 0;
}

const char *__cdecl CG_SkipRestOfLine(const char *line)
{
    while (*line != 10 && *line != 13 && *line)
        ++line;
    return CG_SkipWhiteSpace(line);
}

bool __cdecl CG_MatchLineStartingWith(const char *line, const char *startLine)
{
    return I_strncmp(startLine, line, strlen(startLine)) == 0;
}

const char *__cdecl CG_ParseSound(int localClientNum, char *line)
{
    float origin[3]; // [esp+0h] [ebp-11Ch] BYREF
    char soundalias[256]; // [esp+Ch] [ebp-110h] BYREF
    float angles[3]; // [esp+110h] [ebp-Ch] BYREF
    char *linea; // [esp+128h] [ebp+Ch]
    char *lineb; // [esp+128h] [ebp+Ch]
    char *linec; // [esp+128h] [ebp+Ch]
    char *lined; // [esp+128h] [ebp+Ch]
    char *linee; // [esp+128h] [ebp+Ch]
    char *linef; // [esp+128h] [ebp+Ch]
    const char *lineg; // [esp+128h] [ebp+Ch]

    linea = (char *)CG_SkipLine(line, "ent = maps\\mp\\_createfx::createLoopSound();");
    if (!linea)
        return 0;
    lineb = CG_SkipText(linea, "ent.v[ \"origin\" ] = (");
    if (!lineb)
        return 0;
    linec = (char *)CG_ParseVec3Finish(lineb, origin);
    if (!linec)
        return 0;
    lined = CG_SkipText(linec, "ent.v[ \"angles\" ] = (");
    if (!lined)
        return 0;
    linee = (char *)CG_ParseVec3Finish(lined, angles);
    if (!linee)
        return 0;
    linef = CG_SkipText(linee, "ent.v[ \"soundalias\" ] = ");
    if (!linef)
        return 0;
    lineg = CG_ParseStringFinish(linef, soundalias, 0x100u);
    if (!lineg)
        return 0;
    if (!localClientNum)
        CG_AddClientEntSound(origin, soundalias);
    return lineg;
}

void __cdecl CG_AddClientEntSound(const float *origin, const char *soundalias)
{
    ClientEntSound *v2; // [esp+0h] [ebp-4h]

    if (g_clientEntSoundCount == 128)
    {
        Com_PrintError(1, "Too many client ent sounds.  Increase MAX_CLIENT_ENT_SOUNDS.\n");
    }
    else
    {
        v2 = &g_clientEntSounds[g_clientEntSoundCount];
        v2->origin[0] = *origin;
        v2->origin[1] = origin[1];
        v2->origin[2] = origin[2];
        g_clientEntSounds[g_clientEntSoundCount++].aliasList = Com_FindSoundAlias(soundalias);
    }
}

const char *__cdecl CG_ParseVec3Finish(char *line, float *origin)
{
    char errorText[132]; // [esp+0h] [ebp-88h] BYREF

    if (sscanf(line, "%f, %f, %f", origin, origin + 1, origin + 2) == 3)
        return CG_SkipRestOfLine(line);
    I_strncpyz(errorText, line, 128);
    Com_PrintError(1, "Expected 3 floats instead of '%s'\n", errorText);
    return 0;
}

const char *__cdecl CG_ParseStringFinish(char *line, char *text, unsigned int bufferSize)
{
    char *linea; // [esp+8h] [ebp+8h]

    linea = CG_ParseString(line, text, bufferSize);
    if (linea)
        return CG_SkipRestOfLine(linea);
    else
        return 0;
}

char *__cdecl CG_ParseString(char *line, char *text, unsigned int bufferSize)
{
    char errorText[128]; // [esp+0h] [ebp-88h] BYREF
    unsigned int charCount; // [esp+84h] [ebp-4h]

    if (*line != 34)
    {
        I_strncpyz(errorText, line, 128);
        Com_PrintError(1, "Expected a quoted string instead of '%s'\n", errorText);
    }
    for (charCount = 0; line[charCount + 1] != 34 && line[charCount + 1] && charCount < bufferSize; ++charCount)
        text[charCount] = line[charCount + 1];
    if (charCount == bufferSize)
    {
        I_strncpyz(errorText, line, 128);
        Com_PrintError(1, "String was longer than expected '%s'\n", errorText);
        return 0;
    }
    else
    {
        text[charCount] = 0;
        return &line[charCount + 2];
    }
}

char *__cdecl CG_ParseEffect(int localClientNum, char *line)
{
    float delay; // [esp+0h] [ebp-288h] BYREF
    float origin[3]; // [esp+4h] [ebp-284h] BYREF
    char soundalias[256]; // [esp+10h] [ebp-278h] BYREF
    const FxEffectDef *fxDef; // [esp+110h] [ebp-178h]
    float angles[3]; // [esp+114h] [ebp-174h] BYREF
    char fxDefName[256]; // [esp+120h] [ebp-168h] BYREF
    char fxDefFileName[64]; // [esp+220h] [ebp-68h] BYREF
    float axis[3][3]; // [esp+264h] [ebp-24h] BYREF
    char *linea; // [esp+294h] [ebp+Ch]
    char *lineb; // [esp+294h] [ebp+Ch]
    char *linec; // [esp+294h] [ebp+Ch]
    char *lined; // [esp+294h] [ebp+Ch]
    char *linee; // [esp+294h] [ebp+Ch]
    char *linef; // [esp+294h] [ebp+Ch]
    char *lineg; // [esp+294h] [ebp+Ch]
    char *lineh; // [esp+294h] [ebp+Ch]
    char *linei; // [esp+294h] [ebp+Ch]
    char *linej; // [esp+294h] [ebp+Ch]

    linea = (char *)CG_SkipLineStartingWith(line, "ent = maps\\mp\\_utility::createOneshotEffect( ");
    if (!linea)
        return 0;
    lineb = CG_SkipText(linea, "ent.v[ \"origin\" ] = (");
    if (!lineb)
        return 0;
    linec = (char *)CG_ParseVec3Finish(lineb, origin);
    if (!linec)
        return 0;
    lined = CG_SkipText(linec, "ent.v[ \"angles\" ] = (");
    if (!lined)
        return 0;
    linee = (char *)CG_ParseVec3Finish(lined, angles);
    if (!linee)
        return 0;
    linef = CG_SkipText(linee, "ent.v[ \"fxid\" ] = ");
    if (!linef)
        return 0;
    lineg = (char *)CG_ParseStringFinish(linef, fxDefName, 0x100u);
    if (!lineg)
        return 0;
    lineh = CG_SkipText(lineg, "ent.v[ \"delay\" ] = ");
    if (!lineh)
        return 0;
    linei = (char *)CG_ParseFloatFinish(lineh, &delay);
    if (!linei)
        return 0;
    if (CG_MatchLineStartingWith(linei, "ent.v[ \"soundalias\" ] = "))
    {
        linej = CG_SkipText(linei, "ent.v[ \"soundalias\" ] = ");
        if (!linej)
            return 0;
        linei = (char *)CG_ParseStringFinish(linej, soundalias, 0x100u);
        if (!linei)
            return 0;
        if (!localClientNum)
            CG_AddClientEntSound(origin, soundalias);
    }
    if (CG_FindFileName(fxDefName, fxDefFileName, 64))
    {
        fxDef = FX_Register(fxDefFileName);
        AnglesToAxis(angles, axis);
        FX_SpawnOrientedEffect(localClientNum, fxDef, (int)(delay * 1000.0), origin, axis, 0x3FFu);
    }
    return linei;
}

const char *__cdecl CG_ParseFloatFinish(char *line, float *value)
{
    char errorText[132]; // [esp+0h] [ebp-88h] BYREF

    if (sscanf(line, "%f", value) == 1)
        return CG_SkipRestOfLine(line);
    I_strncpyz(errorText, line, 128);
    Com_PrintError(1, "Expected a float instead of '%s'\n", errorText);
    return 0;
}

char __cdecl CG_FindFileName(const char *name, char *filename, int size)
{
    int i; // [esp+0h] [ebp-4h]

    for (i = 0; i < g_effectDefMapEntries; ++i)
    {
        if (!I_strcmp(g_effectDefMap[i].name, name))
        {
            I_strncpyz(filename, g_effectDefMap[i].filename, size);
            return 1;
        }
    }
    Com_PrintError(1, "Couldn't find '%s' in _fx.gsc map.\n", name);
    return 0;
}

void __cdecl CG_LoadClientEffects_FastFile(int localClientNum, const char *filename)
{
    RawFile *rawfile; // [esp+4h] [ebp-4h]

    rawfile = DB_FindXAssetHeader(ASSET_TYPE_RAWFILE, filename).rawfile;
    if (rawfile)
        CG_ParseClientEffects(localClientNum, (char *)rawfile->buffer);
    else
        Com_PrintError(1, "file not found: %s\n", filename);
}

void __cdecl CG_LoadClientEffectMapping(const char *filename)
{
    if (useFastFile->current.enabled)
        CG_LoadClientEffectMapping_FastFile(filename);
    else
        CG_LoadClientEffectMapping_LoadObj(filename);
}

void __cdecl CG_ParseClientEffectMapping(const char *buffer)
{
    char filename[64]; // [esp+0h] [ebp-88h] BYREF
    char name[64]; // [esp+40h] [ebp-48h] BYREF
    const char *line; // [esp+84h] [ebp-4h]

    g_effectDefMapEntries = 0;
    line = CG_SkipWhiteSpace(buffer);
    while (line && *line)
    {
        if (CG_MatchLineStartingWith(line, "level._effect["))
        {
            line = CG_SkipText((char *)line, "level._effect[");
            if (!line)
                return;
            line = CG_SkipWhiteSpace(line);
            line = CG_ParseString((char *)line, name, 0x40u);
            if (!line)
                return;
            line = CG_SkipWhiteSpace(line);
            line = CG_SkipText((char *)line, "]");
            if (!line)
                return;
            line = CG_SkipWhiteSpace(line);
            line = CG_SkipText((char *)line, "=");
            if (!line)
                return;
            line = CG_SkipWhiteSpace(line);
            line = CG_SkipText((char *)line, "loadfx");
            if (!line)
                return;
            line = CG_SkipWhiteSpace(line);
            line = CG_SkipText((char *)line, "(");
            if (!line)
                return;
            line = CG_SkipWhiteSpace(line);
            line = CG_ParseStringFinish((char *)line, filename, 0x40u);
            if (!line)
                return;
            CG_AddPairToMap(name, filename);
        }
        else
        {
            line = CG_SkipRestOfLine(line);
        }
    }
}

void __cdecl CG_AddPairToMap(char *name, char *filename)
{
    int i; // [esp+0h] [ebp-4h]

    for (i = 0; i < g_effectDefMapEntries; ++i)
    {
        if (!I_strcmp(g_effectDefMap[i].name, name))
        {
            if (I_strcmp(g_effectDefMap[i].filename, filename))
                Com_PrintError(
                    1,
                    "Tried to remap '%s' to '%s' previously mapped to '%s'\n",
                    g_effectDefMap[i].name,
                    g_effectDefMap[i].filename,
                    filename);
            return;
        }
    }
    if (g_effectDefMapEntries == 32)
    {
        Com_PrintError(1, "Failed to added mapping from '%s' to '%s'.  Increase MAX_CLIENT_EFFECT_DEFS.\n", name, filename);
    }
    else
    {
        I_strncpyz(g_effectDefMap[g_effectDefMapEntries].name, name, 64);
        I_strncpyz(g_effectDefMap[g_effectDefMapEntries].filename, filename, 64);
        ++g_effectDefMapEntries;
    }
}

void __cdecl CG_LoadClientEffectMapping_FastFile(const char *filename)
{
    RawFile *rawfile; // [esp+4h] [ebp-4h]

    rawfile = DB_FindXAssetHeader(ASSET_TYPE_RAWFILE, filename).rawfile;
    if (rawfile)
        CG_ParseClientEffectMapping(rawfile->buffer);
    else
        Com_PrintError(1, "file not found: %s\n", filename);
}

void __cdecl CG_ClientSideEffectsRegisterDvars()
{
    cg_clientSideEffects = Dvar_RegisterBool("clientSideEffects", 1, 0x80u, "Enable loading _fx.gsc files on the client");
}

void __cdecl CG_AddClientSideSounds(int localClientNum)
{
    int i; // [esp+0h] [ebp-4h]

    for (i = 0; i < g_clientEntSoundCount; ++i)
        CG_AddClientSideSound(localClientNum, i, &g_clientEntSounds[i]);
}

void __cdecl CG_AddClientSideSound(int localClientNum, int index, const ClientEntSound *sound)
{
    if (index + 1024 >= 1152)
        MyAssertHandler(
            ".\\cgame_mp\\cg_client_side_effects_mp.cpp",
            586,
            0,
            "%s",
            "soundClientEntIndex < MAX_SOUND_ENTITIES");
    CG_PlaySoundAlias(localClientNum, index + 1024, sound->origin, sound->aliasList);
}

void __cdecl CG_CopyClientSideSoundEntityOrientation(
    unsigned int clientSoundEntIndex,
    float *origin_out,
    float (*axis_out)[3])
{
    ClientEntSound *v3; // edx

    if (clientSoundEntIndex >= g_clientEntSoundCount)
        MyAssertHandler(
            ".\\cgame_mp\\cg_client_side_effects_mp.cpp",
            603,
            0,
            "clientSoundEntIndex doesn't index g_clientEntSoundCount\n\t%i not in [0, %i)",
            clientSoundEntIndex,
            g_clientEntSoundCount);
    AnglesToAxis(zeroVec3, axis_out);
    *origin_out = g_clientEntSounds[clientSoundEntIndex].origin[0];
    v3 = &g_clientEntSounds[clientSoundEntIndex];
    origin_out[1] = v3->origin[1];
    origin_out[2] = v3->origin[2];
}

