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