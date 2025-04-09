#include "r_material.h"
#include "r_utils.h"

char __cdecl MaterialTechniqueSet_FindHashLocation(const char *name, int *foundHashIndex)
{
    int hashIndex; // [esp+0h] [ebp-4h]

    if (!name)
        MyAssertHandler(".\\r_material.cpp", 816, 0, "%s", "name");
    for (hashIndex = R_HashAssetName(name) & 0x3FF;
        materialGlobals.techniqueSetHashTable[hashIndex];
        hashIndex = ((_WORD)hashIndex + 1) & 0x3FF)
    {
        if (!I_stricmp(materialGlobals.techniqueSetHashTable[hashIndex]->name, name))
        {
            *foundHashIndex = hashIndex;
            return 1;
        }
    }
    *foundHashIndex = hashIndex;
    return 0;
}

MaterialTechniqueSet *__cdecl Material_RegisterTechniqueSet(const char *name)
{
    GfxRenderer renderer; // [esp+0h] [ebp-8h]
    MaterialTechniqueSet *techniqueSet; // [esp+4h] [ebp-4h]
    MaterialTechniqueSet *techniqueSeta; // [esp+4h] [ebp-4h]

    techniqueSet = Material_FindTechniqueSet(name, MTL_TECHSET_NOT_FOUND_RETURN_NULL);
    if (techniqueSet)
        return techniqueSet;
    renderer = r_rendererInUse->current.integer;
    ProfLoad_Begin("Load technique set");
    techniqueSeta = Material_LoadTechniqueSet(name, renderer);
    ProfLoad_End();
    if (!techniqueSeta)
        return 0;
    Material_SetTechniqueSet(name, techniqueSeta);
    if (g_generateOverrideTechniques)
        Material_RegisterOverriddenTechniqueSets(name);
    return techniqueSeta;
}

MaterialTechniqueSet *__cdecl Material_FindTechniqueSet_LoadObj(
    const char *name,
    MtlTechSetNotFoundBehavior notFoundBehavior)
{
    MaterialTechniqueSet *defaultTechSet; // [esp+0h] [ebp-8h]
    int hashIndex; // [esp+4h] [ebp-4h] BYREF

    if (!name)
        MyAssertHandler(".\\r_material.cpp", 839, 0, "%s", "name");
    if (MaterialTechniqueSet_FindHashLocation(name, &hashIndex))
        return materialGlobals.techniqueSetHashTable[hashIndex];
    if (notFoundBehavior == MTL_TECHSET_NOT_FOUND_RETURN_NULL)
        return 0;
    defaultTechSet = Material_RegisterTechniqueSet("default");
    if (!defaultTechSet)
        MyAssertHandler(".\\r_material.cpp", 847, 0, "%s", "defaultTechSet");
    return defaultTechSet;
}

void __cdecl Material_GetInfo(Material *handle, MaterialInfo *matInfo)
{
    if (!handle)
        MyAssertHandler(".\\r_material_load_obj.cpp", 6907, 0, "%s", "handle");
    if (!matInfo)
        MyAssertHandler(".\\r_material_load_obj.cpp", 6908, 0, "%s", "matInfo");
    *matInfo = Material_FromHandle(handle)->info;
}

void __cdecl Material_Add(Material *material, unsigned __int16 hashIndex)
{
    unsigned __int64 v2; // rax
    unsigned int v3; // ecx

    if (!material)
        MyAssertHandler(".\\r_material.cpp", 1084, 0, "%s", "material");
    rgp.needSortMaterials = 1;
    if (rg.materialHashTable[hashIndex])
        MyAssertHandler(".\\r_material.cpp", 1088, 1, "%s", "rg.materialHashTable[hashIndex] == NULL");
    material->info.hashIndex = hashIndex;
    v2 = (unsigned __int64)(rgp.materialCount & 0x7FF) << 29;
    v3 = HIDWORD(v2) | HIDWORD(material->info.drawSurf.packed) & 0xFFFFFF00;
    *(_DWORD *)&material->info.drawSurf.fields = v2 | *(_DWORD *)&material->info.drawSurf.fields & 0x1FFFFFFF;
    HIDWORD(material->info.drawSurf.packed) = v3;
    rgp.sortedMaterials[rgp.materialCount] = material;
    rg.materialHashTable[hashIndex] = material;
    if (++rgp.materialCount == 2048)
        Com_Error(ERR_FATAL, "Too many unique materials (%i or more)\n", 2048);
}

void __cdecl Material_GetHashIndex(const char *name, unsigned __int16 *hashIndex, bool *exists)
{
    unsigned __int16 beginHashIndex; // [esp+14h] [ebp-4h]

    if (!name)
        MyAssertHandler(".\\r_material.cpp", 1193, 0, "%s", "name");
    if (!hashIndex)
        MyAssertHandler(".\\r_material.cpp", 1194, 0, "%s", "hashIndex");
    if (!exists)
        MyAssertHandler(".\\r_material.cpp", 1195, 0, "%s", "exists");
    beginHashIndex = R_HashAssetName(name) % 0x7FF;
    *hashIndex = beginHashIndex;
    do
    {
        if (!rg.materialHashTable[*hashIndex])
            break;
        if (!strcmp(rg.materialHashTable[*hashIndex]->info.name, name))
        {
            *exists = 1;
            return;
        }
        *hashIndex = ((unsigned int)*hashIndex + 1) % 0x7FF;
    } while (*hashIndex != beginHashIndex);
    *exists = 0;
}

Material *__cdecl Material_Duplicate(Material *mtlCopy, char *name)
{
    unsigned int v3; // [esp+8h] [ebp-30h]
    const char *nameBackup; // [esp+18h] [ebp-20h]
    Material *mtlNewa; // [esp+1Ch] [ebp-1Ch]
    unsigned __int8 *mtlNew; // [esp+1Ch] [ebp-1Ch]
    int constantTableSize; // [esp+24h] [ebp-14h]
    unsigned __int16 hashIndex[3]; // [esp+28h] [ebp-10h] BYREF
    bool exists; // [esp+2Fh] [ebp-9h] BYREF
    unsigned int textureTableSize; // [esp+30h] [ebp-8h]
    unsigned int stateBitsTableSize; // [esp+34h] [ebp-4h]

    if (!mtlCopy)
        MyAssertHandler(".\\r_material.cpp", 1110, 0, "%s", "mtlCopy");
    if (!name)
        MyAssertHandler(".\\r_material.cpp", 1111, 0, "%s", "name");
    Material_GetHashIndex(name, hashIndex, &exists);
    if (exists)
    {
        mtlNewa = rg.materialHashTable[hashIndex[0]];
        nameBackup = mtlNewa->info.name;
        memcpy(mtlNewa, mtlCopy, sizeof(Material));
        mtlNewa->info.name = nameBackup;
        rgp.needSortMaterials = 1;
        return mtlNewa;
    }
    else
    {
        v3 = strlen(name);
        mtlNew = Material_Alloc(v3 + 81);
        memcpy(mtlNew, mtlCopy, 0x50u);
        *(_DWORD *)mtlNew = mtlNew + 80;
        memcpy(*(unsigned __int8 **)mtlNew, (unsigned __int8 *)name, v3 + 1);
        stateBitsTableSize = 8 * mtlCopy->stateBitsCount;
        *((_DWORD *)mtlNew + 19) = Material_Alloc(stateBitsTableSize);
        memcpy(*((unsigned __int8 **)mtlNew + 19), (unsigned __int8 *)mtlCopy->stateBitsTable, stateBitsTableSize);
        if (mtlCopy->textureTable)
        {
            textureTableSize = 12 * mtlCopy->textureCount;
            *((_DWORD *)mtlNew + 17) = Material_Alloc(textureTableSize);
            memcpy(*((unsigned __int8 **)mtlNew + 17), (unsigned __int8 *)mtlCopy->textureTable, textureTableSize);
        }
        if (mtlCopy->constantTable)
        {
            constantTableSize = 32 * mtlCopy->constantCount;
            *((_DWORD *)mtlNew + 18) = Material_Alloc(constantTableSize);
            memcpy(*((unsigned __int8 **)mtlNew + 18), (unsigned __int8 *)mtlCopy->constantTable, constantTableSize);
        }
        Material_Add((Material *)mtlNew, hashIndex[0]);
        return (Material *)mtlNew;
    }
}