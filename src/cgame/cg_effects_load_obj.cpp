#include "cg_local.h"
#include "cg_public.h"
#include <database/database.h>
#include <universal/com_memory.h>
#include <universal/com_files.h>
#include <universal/q_parse.h>
#include <universal/surfaceflags.h>
#include <EffectsCore/fx_system.h>

const char *const g_FleshTypeName[4] =
{
  "flesh_body_nonfatal",
  "flesh_body_fatal",
  "flesh_head_nonfatal",
  "flesh_head_fatal"
}; // idb

const char *const g_TypeName[12] =
{
  "bullet_small_normal",
  "bullet_small_exit",
  "bullet_large_normal",
  "bullet_large_exit",
  "shotgun_normal",
  "shotgun_exit",
  "bullet_ap_normal",
  "bullet_ap_exit",
  "grenade_bounce",
  "grenade_explode",
  "rocket_explode",
  "projectile_dud"
}; // idb

char *__cdecl CG_ParseImpactEffects(
    const char *filename,
    const char *buf,
    int iTypeCount,
    const char *const *pszTypeName,
    EffectFile *effectFile)
{
    char v6; // cl
    char *v7; // [esp+8h] [ebp-34h]
    parseInfo_t *v8; // [esp+Ch] [ebp-30h]
    int v9; // [esp+10h] [ebp-2Ch]
    unsigned int i; // [esp+20h] [ebp-1Ch]
    int iEffectType; // [esp+24h] [ebp-18h]
    char *effectName; // [esp+2Ch] [ebp-10h]
    int iFleshType; // [esp+30h] [ebp-Ch]
    int iSurfaceType; // [esp+34h] [ebp-8h]
    parseInfo_t *token; // [esp+38h] [ebp-4h]
    parseInfo_t *tokena; // [esp+38h] [ebp-4h]
    parseInfo_t *tokenb; // [esp+38h] [ebp-4h]

    while (1)
    {
        token = Com_Parse(&buf);
        if (!buf)
            return 0;
        if (token->token[0] && token->token[0] != 35)
            break;
    LABEL_5:
        Com_SkipRestOfLine(&buf);
    }
    for (iEffectType = 0; iEffectType < iTypeCount && I_stricmp(pszTypeName[iEffectType], token->token); ++iEffectType)
        ;
    if (iEffectType == iTypeCount)
        return va("unknown effect type '%s' in first column of file '%s'", token->token, filename);
    tokena = Com_ParseOnLine(&buf);
    if (!tokena->token[0])
        return va("missing surface/flesh type in second column of file '%s'", filename);
    iSurfaceType = Com_SurfaceTypeFromName(tokena->token);
    for (i = 0; i < 4; ++i)
    {
        if (!I_stricmp(tokena->token, g_FleshTypeName[i]))
        {
            iFleshType = i;
            goto LABEL_20;
        }
    }
    iFleshType = -1;
LABEL_20:
    if (iSurfaceType >= 0 && iFleshType >= 0)
        MyAssertHandler(".\\cgame\\cg_effects_load_obj.cpp", 167, 0, "%s", "iSurfaceType < 0 || iFleshType < 0");
    if (iSurfaceType < 0 && iFleshType < 0)
        return va("unknown surface/flesh type '%s' in second column of file '%s'", tokena->token, filename);
    tokenb = Com_ParseOnLine(&buf);
    v9 = strlen(tokenb->token);
    if (v9 < 64)
    {
        effectName = (char *)Hunk_AllocateTempMemory(v9 + 1, "CG_ParseImpactEffects");
        v8 = tokenb;
        v7 = effectName;
        do
        {
            v6 = v8->token[0];
            *v7 = v8->token[0];
            v8 = (parseInfo_t *)((char *)v8 + 1);
            ++v7;
        } while (v6);
        if (iSurfaceType < 0 && iFleshType < 0)
            MyAssertHandler(".\\cgame\\cg_effects_load_obj.cpp", 179, 0, "%s", "iSurfaceType >= 0 || iFleshType >= 0");
        if (iSurfaceType < 0)
            effectFile->flesh[iEffectType][iFleshType] = effectName;
        else
            effectFile->nonflesh[iEffectType][iSurfaceType] = effectName;
        goto LABEL_5;
    }
    return va(
        "effect filename '%s' in third column of file '%s' is longer than %i characters",
        tokenb->token,
        filename,
        63);
}

int __cdecl compare_impact_files(const char **pe0, const char **pe1);

int __cdecl CG_RegisterImpactEffects_Generic_29_char_const_____cdecl_int__(
    const char **szEffectFile,
    const char *pszTypeName,
    const FxEffectDef **fx,
    const char *(__cdecl *typeToNameFunc)(int),
    char *defaultEffectName)
{
    const char *v5; // eax
    char *effectName; // [esp+0h] [ebp-Ch]
    int iBadCount; // [esp+4h] [ebp-8h]
    int i; // [esp+8h] [ebp-4h]

    iBadCount = 0;
    for (i = 0; i < 29; ++i)
    {
        effectName = (char *)szEffectFile[i];
        if (!effectName)
            effectName = defaultEffectName;
        if (effectName)
        {
            if (!*effectName && defaultEffectName)
                effectName = defaultEffectName;
            if (*effectName)
                fx[i] = FX_Register(effectName);
            else
                fx[i] = 0;
        }
        else
        {
            v5 = typeToNameFunc(i);
            Com_Printf(21, "no entry for effect type '%s' on surface/flesh type '%s'\n", pszTypeName, v5);
            ++iBadCount;
            fx[i] = 0;
        }
    }
    return iBadCount;
}

int __cdecl CG_RegisterImpactEffects_NonFlesh(
    const char **szEffectFile,
    const char *pszTypeName,
    const FxEffectDef **fx)
{
    return CG_RegisterImpactEffects_Generic_29_char_const_____cdecl_int__(
        szEffectFile,
        pszTypeName,
        fx,
        (const char *(__cdecl *)(int))Com_SurfaceTypeToName,
        0);
}

void __cdecl CG_RegisterImpactEffectsForDir(char *dir, EffectFile *effectFile, char *listbuf)
{
    void *base[4097]; // [esp+10h] [ebp-4020h] BYREF
    void *buffer; // [esp+4014h] [ebp-1Ch]
    unsigned int num; // [esp+4018h] [ebp-18h]
    int len; // [esp+401Ch] [ebp-14h]
    char *v7; // [esp+4020h] [ebp-10h]
    char *qpath; // [esp+4024h] [ebp-Ch]
    int f; // [esp+4028h] [ebp-8h] BYREF
    unsigned int i; // [esp+402Ch] [ebp-4h]

    num = FS_GetFileList(dir, "csv", FS_LIST_PURE_ONLY, listbuf, 0x10000);
    if (num)
    {
        if (num > 0x1000)
            num = 4096;
        qpath = listbuf;
        for (i = 0; i < num; ++i)
        {
            base[i] = qpath;
            qpath += strlen(qpath) + 1;
        }
        qsort(base, num, 4u, (int(__cdecl *)(const void *, const void *))compare_impact_files);
        for (i = 0; i < num; ++i)
        {
            qpath = va("%s/%s", dir, (const char *)base[i]);
            len = FS_FOpenFileByMode(qpath, &f, FS_READ);
            if (len >= 0)
            {
                Hunk_CheckTempMemoryHighClear();
                buffer = (void *)Hunk_AllocateTempMemoryHigh(len + 1, "CG_RegisterImpactEffects");
                FS_Read((unsigned __int8 *)buffer, len, f);
                FS_FCloseFile(f);
                *((_BYTE *)buffer + len) = 0;
                Com_BeginParseSession(qpath);
                Com_SetCSV(1);
                v7 = CG_ParseImpactEffects(qpath, (const char *)buffer, 12, g_TypeName, effectFile);
                Com_EndParseSession();
                Hunk_ClearTempMemoryHigh();
                if (v7)
                {
                    Com_PrintError(21, "ERROR: %s", v7);
                    return;
                }
            }
        }
    }
}

int __cdecl CG_RegisterImpactEffects_Generic_4_char_const_____cdecl_int__(
    const char **szEffectFile,
    const char *pszTypeName,
    const FxEffectDef **fx,
    const char *(__cdecl *typeToNameFunc)(int),
    char *defaultEffectName)
{
    const char *v5; // eax
    char *effectName; // [esp+0h] [ebp-Ch]
    int iBadCount; // [esp+4h] [ebp-8h]
    int i; // [esp+8h] [ebp-4h]

    iBadCount = 0;
    for (i = 0; i < 4; ++i)
    {
        effectName = (char *)szEffectFile[i];
        if (!effectName)
            effectName = defaultEffectName;
        if (effectName)
        {
            if (!*effectName && defaultEffectName)
                effectName = defaultEffectName;
            if (*effectName)
                fx[i] = FX_Register(effectName);
            else
                fx[i] = 0;
        }
        else
        {
            v5 = typeToNameFunc(i);
            Com_Printf(21, "no entry for effect type '%s' on surface/flesh type '%s'\n", pszTypeName, v5);
            ++iBadCount;
            fx[i] = 0;
        }
    }
    return iBadCount;
}

const char *__cdecl CG_FleshTypeToName(unsigned int fleshTypeId)
{
    if (fleshTypeId >= 4)
        MyAssertHandler(
            ".\\cgame\\cg_effects_load_obj.cpp",
            61,
            0,
            "%s\n\t(fleshTypeId) = %i",
            "(fleshTypeId >= 0 && fleshTypeId < (sizeof( g_FleshTypeName ) / (sizeof( g_FleshTypeName[0] ) * (sizeof( g_FleshTy"
            "peName ) != 4 || sizeof( g_FleshTypeName[0] ) <= 4))))",
            fleshTypeId);
    return g_FleshTypeName[fleshTypeId];
}

int __cdecl CG_RegisterImpactEffects_Flesh(
    const char **szEffectFile,
    const char *pszTypeName,
    const FxEffectDef **fx,
    char *defaultEffectName)
{
    return CG_RegisterImpactEffects_Generic_4_char_const_____cdecl_int__(
        szEffectFile,
        pszTypeName,
        fx,
        (const char *(__cdecl *)(int))CG_FleshTypeToName,
        defaultEffectName);
}

FxImpactTable *__cdecl CG_RegisterImpactEffects_LoadObj(const char *mapname)
{
    unsigned __int8 *v1; // eax
    int v2; // eax
    int v3; // eax
    EffectFile effectFile; // [esp+0h] [ebp-688h] BYREF
    char *listbuf; // [esp+630h] [ebp-58h]
    int iBadCount; // [esp+634h] [ebp-54h]
    unsigned int i; // [esp+638h] [ebp-50h]
    FxImpactTable *fx; // [esp+63Ch] [ebp-4Ch]
    char mapdir[68]; // [esp+640h] [ebp-48h] BYREF

    Hunk_CheckTempMemoryClear();
    listbuf = (char *)Hunk_AllocateTempMemory(0x10000, "CG_RegisterImpactEffects");
    memset((unsigned __int8 *)&effectFile, 0, sizeof(effectFile));
    CG_RegisterImpactEffectsForDir((char*)"fx", &effectFile, listbuf);
    if (mapname)
    {
        Com_sprintf(mapdir, 0x40u, "fx/maps/%s", mapname);
        CG_RegisterImpactEffectsForDir(mapdir, &effectFile, listbuf);
    }
    fx = (FxImpactTable *)Hunk_AllocAlign(8u, 4, "CG_RegisterImpactEffects", 8);
    v1 = Hunk_AllocAlign(0x630u, 4, "CG_RegisterImpactEffects", 8);
    fx->table = (FxImpactEntry *)v1;
    iBadCount = 0;
    for (i = 0; i < 0xC; ++i)
    {
        v2 = CG_RegisterImpactEffects_NonFlesh(effectFile.nonflesh[i], g_TypeName[i], fx->table[i].nonflesh);
        iBadCount += v2;
        v3 = CG_RegisterImpactEffects_Flesh(
            effectFile.flesh[i],
            g_TypeName[i],
            fx->table[i].flesh,
            (char *)effectFile.nonflesh[i][7]);
        iBadCount += v3;
    }
    if (iBadCount)
    {
        Com_PrintError(21, "ERROR: %i missing entries in effect CSV files (see console for details)", iBadCount);
        Hunk_ClearTempMemory();
        return 0;
    }
    else
    {
        fx->name = "";
        Hunk_ClearTempMemory();
        return fx;
    }
}

FxImpactTable *__cdecl CG_RegisterImpactEffects_FastFile()
{
    return DB_FindXAssetHeader(ASSET_TYPE_IMPACT_FX, "").impactFx;
}

FxImpactTable *__cdecl CG_RegisterImpactEffects(const char *mapname)
{
    if (IsFastFileLoad())
        return CG_RegisterImpactEffects_FastFile();
    else
        return CG_RegisterImpactEffects_LoadObj(mapname);
}