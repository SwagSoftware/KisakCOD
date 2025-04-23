#include "r_material.h"
#include <qcommon/threads.h>
#include "r_dvars.h"
#include "r_init.h"
#include "r_rendercmds.h"
#include <database/database.h>
#include "rb_uploadshaders.h"

$4ABF24606230B73E4E420CE33A1F14B1 mtlOverrideGlob;

const GfxMtlFeatureMap s_materialFeatures[20] =
{
  { "s0", 4u, 0u, false },
  { "s1", 4u, 0u, false },
  { "s2", 4u, 0u, false },
  { "s3", 4u, 0u, false },
  { "s4", 4u, 0u, false },
  { "d0", 8u, 0u, false },
  { "d1", 8u, 0u, false },
  { "d2", 8u, 0u, false },
  { "d3", 8u, 0u, false },
  { "d4", 8u, 0u, false },
  { "n0", 16u, 0u, false },
  { "n1", 16u, 0u, false },
  { "n2", 16u, 0u, false },
  { "n3", 16u, 0u, false },
  { "n4", 16u, 0u, false },
  { "zfeather", 1u, 0u, false },
  { "outdoor", 2u, 0u, false },
  { "sm", 384u, 128u, true },
  { "hsm", 384u, 256u, true },
  { "twk", 32u, 0u, false }
}; // idb

void __cdecl Material_GetRemappedFeatures_RunTime(unsigned int *mask, unsigned int *value)
{
    if (!mask)
        MyAssertHandler(".\\r_material_override.cpp", 100, 0, "%s", "mask");
    if (!value)
        MyAssertHandler(".\\r_material_override.cpp", 101, 0, "%s", "value");
    *mask = 0;
    *value = 0;
    if (!r_detail->current.enabled)
        *mask |= 8u;
    if (!r_specular->current.enabled)
        *mask |= 4u;
    if (!r_normal->current.integer)
        *mask |= 0x10u;
    if (!r_zFeather->current.enabled)
        *mask |= 1u;
    if (!r_outdoor->current.enabled)
        *mask |= 2u;
    *mask |= 0x180u;
    *value |= gfxMetrics.hasHardwareShadowmap ? 256 : 128;
    if (r_envMapOverride->current.enabled)
        *mask |= 0x20u;
}

void __cdecl Material_ForEachTechniqueSet_FastFile(void(__cdecl *callback)(MaterialTechniqueSet *))
{
    unsigned int inData[1024]; // [esp+0h] [ebp-1008h] BYREF
    int v2; // [esp+1000h] [ebp-8h]

    v2 = 0;
    DB_EnumXAssets(
        ASSET_TYPE_TECHNIQUE_SET,
        (void(__cdecl *)(XAssetHeader, void *))Material_CollateTechniqueSets,
        inData,
        0);
    while (v2)
        callback((MaterialTechniqueSet *)inData[--v2]);
}

void __cdecl Material_ForEachTechniqueSet_LoadObj(void(__cdecl *callback)(MaterialTechniqueSet *))
{
    unsigned int hashIndex; // [esp+0h] [ebp-8h]
    MaterialTechniqueSet *techSet; // [esp+4h] [ebp-4h]

    for (hashIndex = 0; hashIndex < 0x400; ++hashIndex)
    {
        techSet = materialGlobals.techniqueSetHashTable[hashIndex];
        if (techSet)
            callback(techSet);
    }
}

void __cdecl Material_ForEachTechniqueSet(void(__cdecl *callback)(MaterialTechniqueSet *))
{
    if (useFastFile->current.enabled)
        Material_ForEachTechniqueSet_FastFile(callback);
    else
        Material_ForEachTechniqueSet_LoadObj(callback);
}

const GfxMtlFeatureMap *__cdecl Material_FindFeature(
    const char *featureName,
    const GfxMtlFeatureMap *featureMap,
    unsigned int featureCount)
{
    unsigned int featureIndex; // [esp+14h] [ebp-4h]

    for (featureIndex = 0; featureIndex < featureCount; ++featureIndex)
    {
        if (!strcmp(featureName, featureMap[featureIndex].name))
            return &featureMap[featureIndex];
    }
    return 0;
}

unsigned int __cdecl Material_NextTechniqueSetNameToken(const char **parse, char *token)
{
    unsigned int tokenLen; // [esp+0h] [ebp-4h]

    tokenLen = 0;
    while (**parse)
    {
        token[tokenLen] = **parse;
        if (token[tokenLen] == '_')
        {
            ++*parse;
            break;
        }
        if (tokenLen && isdigit(token[tokenLen - 1]) && !isdigit(token[tokenLen]))
            break;
        ++tokenLen;
        ++*parse;
    }
    token[tokenLen] = 0;
    return tokenLen;
}

unsigned int __cdecl Material_ExtendTechniqueSetName(
    char *nameSoFar,
    unsigned int nameLen,
    char *token,
    unsigned int tokenLen,
    bool prependUnderscore)
{
    if (prependUnderscore)
        nameSoFar[nameLen++] = '_';
    if (tokenLen + nameLen >= 0x40)
        Com_Error(ERR_DROP, "Can't extend techset name '%s' with '%s'; would exceed %i chars", nameSoFar, token, 63);
    memcpy((unsigned __int8 *)&nameSoFar[nameLen], (unsigned __int8 *)token, tokenLen + 1);
    return tokenLen + nameLen;
}

void __cdecl Material_RemapTechniqueSetName(
    const char *techSetName,
    char *remapName,
    unsigned int remapMask,
    unsigned int remapValue,
    const GfxMtlFeatureMap *featureMap,
    unsigned int featureCount)
{
    bool v6; // [esp+10h] [ebp-6Ch]
    unsigned int featureIndex; // [esp+14h] [ebp-68h]
    const GfxMtlFeatureMap *feature; // [esp+1Ch] [ebp-60h]
    const GfxMtlFeatureMap *altFeature; // [esp+20h] [ebp-5Ch]
    unsigned int maskedRemapValue; // [esp+24h] [ebp-58h]
    unsigned int tokenLen; // [esp+28h] [ebp-54h]
    int remapNameLen; // [esp+2Ch] [ebp-50h]
    const char *parse; // [esp+30h] [ebp-4Ch] BYREF
    char token[68]; // [esp+34h] [ebp-48h] BYREF

    if (!techSetName)
        MyAssertHandler(".\\r_material_override.cpp", 262, 0, "%s", "techSetName");
    parse = techSetName;
    remapNameLen = 0;
    *remapName = 0;
    if (!r_rendererInUse->current.integer)
    {
        *(_DWORD *)remapName = *(_DWORD *)"sm2/";
        remapNameLen = 4;
    }
    if (!strncmp(techSetName, "sm2/", 4u))
        parse = techSetName + 4;
    while (1)
    {
        v6 = remapNameLen && *(parse - 1) == 95;
        tokenLen = Material_NextTechniqueSetNameToken(&parse, token);
        if (!tokenLen)
            break;
        feature = Material_FindFeature(token, featureMap, featureCount);
        if (feature && (remapMask & feature->mask) != 0)
        {
            if (feature->value)
            {
                maskedRemapValue = feature->mask & remapValue;
                if (maskedRemapValue)
                {
                    for (featureIndex = 0; ; ++featureIndex)
                    {
                        if (featureIndex == featureCount)
                            MyAssertHandler(".\\r_material_override.cpp", 304, 0, "%s", "featureIndex != featureCount");
                        altFeature = &featureMap[featureIndex];
                        if (altFeature->mask == feature->mask && altFeature->value == maskedRemapValue)
                            break;
                    }
                    remapNameLen = Material_ExtendTechniqueSetName(
                        remapName,
                        remapNameLen,
                        (char *)altFeature->name,
                        strlen(altFeature->name),
                        v6);
                }
            }
            else if ((feature->mask & remapValue) != 0)
            {
                MyAssertHandler(
                    ".\\r_material_override.cpp",
                    294,
                    0,
                    "%s\n\t(feature->name) = %s",
                    "((remapValue & feature->mask) == 0)",
                    feature->name);
            }
        }
        else
        {
            remapNameLen = Material_ExtendTechniqueSetName(remapName, remapNameLen, token, tokenLen, v6);
        }
    }
}

void __cdecl AssertValidRemappedTechniqueSet(MaterialTechniqueSet *techSet)
{
    const char *v1; // eax
    const char *name; // [esp+4h] [ebp-8h]
    unsigned int techTypeIter; // [esp+8h] [ebp-4h]

    if (!techSet)
        MyAssertHandler(".\\r_material_override.cpp", 327, 0, "%s", "techSet");
    if (!techSet->remappedTechniqueSet)
        MyAssertHandler(".\\r_material_override.cpp", 328, 0, "%s", "techSet->remappedTechniqueSet");
    if (techSet->remappedTechniqueSet != techSet)
    {
        for (techTypeIter = 0; techTypeIter < 0x22; ++techTypeIter)
        {
            if (!techSet->techniques[techTypeIter] && techSet->remappedTechniqueSet->techniques[techTypeIter])
            {
                name = techSet->remappedTechniqueSet->techniques[techTypeIter]->name;
                if (techSet->techniques[techTypeIter])
                    v1 = va(
                        "%i: %s:%s -> %s:%s",
                        techTypeIter,
                        techSet->name,
                        techSet->techniques[techTypeIter]->name,
                        techSet->remappedTechniqueSet->name,
                        name);
                else
                    v1 = va(
                        "%i: %s:%s -> %s:%s",
                        techTypeIter,
                        techSet->name,
                        "(null)",
                        techSet->remappedTechniqueSet->name,
                        name);
                MyAssertHandler(
                    ".\\r_material_override.cpp",
                    333,
                    0,
                    "%s\n\t%s",
                    "techSet->techniques[techTypeIter] != NULL || techSet->remappedTechniqueSet->techniques[techTypeIter] == NULL",
                    v1);
            }
        }
    }
}

void __cdecl Material_RemapTechniqueSet(MaterialTechniqueSet *techSet)
{
    char remapName[260]; // [esp+14h] [ebp-108h] BYREF

    if (!techSet)
        MyAssertHandler(".\\r_material_override.cpp", 365, 0, "%s", "techSet");
    Material_RemapTechniqueSetName(
        techSet->name,
        remapName,
        mtlOverrideGlob.remapMask,
        mtlOverrideGlob.remapValue,
        s_materialFeatures,
        0x14u);
    if (!strcmp(techSet->name, remapName)
        || (techSet->remappedTechniqueSet = Material_FindTechniqueSet(remapName, MTL_TECHSET_NOT_FOUND_RETURN_NULL)) == 0)
    {
        techSet->remappedTechniqueSet = techSet;
    }
    else
    {
        AssertValidRemappedTechniqueSet(techSet);
    }
}

void __cdecl Material_OverrideTechniqueSets()
{
    unsigned int remapValue; // [esp+0h] [ebp-8h] BYREF
    unsigned int remapMask; // [esp+4h] [ebp-4h] BYREF

    if (!Sys_IsRenderThread())
    {
        if (!Sys_IsMainThread())
            MyAssertHandler(".\\r_material_override.cpp", 394, 1, "%s", "Sys_IsMainThread()");
        Material_GetRemappedFeatures_RunTime(&remapMask, &remapValue);
        if (mtlOverrideGlob.isDirty || mtlOverrideGlob.remapMask != remapMask || mtlOverrideGlob.remapValue != remapValue)
        {
            mtlOverrideGlob.isDirty = 0;
            mtlOverrideGlob.remapMask = remapMask;
            mtlOverrideGlob.remapValue = remapValue;
            rgp.needSortMaterials = 1;
            R_SyncRenderThread();
            Material_ForEachTechniqueSet(Material_RemapTechniqueSet);
        }
    }
}

void __cdecl Material_OriginalRemapTechniqueSet(MaterialTechniqueSet *techSet)
{
    char remapName[68]; // [esp+0h] [ebp-48h] BYREF

    if (!techSet)
        MyAssertHandler(".\\r_material_override.cpp", 341, 0, "%s", "techSet");
    if (r_rendererInUse->current.integer || !strncmp(techSet->name, "sm2/", 4u))
    {
        techSet->remappedTechniqueSet = techSet;
    }
    else
    {
        *(_DWORD *)remapName = *(_DWORD *)"sm2/";
        strncpy((unsigned __int8 *)&remapName[4], (unsigned __int8 *)techSet->name, 0x3Cu);
        remapName[63] = 0;
        techSet->remappedTechniqueSet = Material_FindTechniqueSet(remapName, MTL_TECHSET_NOT_FOUND_RETURN_DEFAULT);
        AssertValidRemappedTechniqueSet(techSet);
    }
}

void __cdecl Material_DirtyTechniqueSetOverrides()
{
    mtlOverrideGlob.isDirty = 1;
}

void __cdecl Material_ClearShaderUploadList()
{
    mtlUploadGlob.get = 0;
    mtlUploadGlob.put = 0;
    mtlUploadGlob.techTypeIter = 0;
}

bool __cdecl Material_WouldTechniqueSetBeOverridden(const MaterialTechniqueSet *techSet)
{
    unsigned int remapValue; // [esp+14h] [ebp-10Ch] BYREF
    char remapName[256]; // [esp+18h] [ebp-108h] BYREF
    unsigned int remapMask; // [esp+11Ch] [ebp-4h] BYREF

    if (!techSet)
        MyAssertHandler(".\\r_material_override.cpp", 418, 0, "%s", "techSet");
    Material_GetRemappedFeatures_RunTime(&remapMask, &remapValue);
    Material_RemapTechniqueSetName(techSet->name, remapName, remapMask, remapValue, s_materialFeatures, 0x14u);
    return strcmp(techSet->name, remapName) != 0;
}