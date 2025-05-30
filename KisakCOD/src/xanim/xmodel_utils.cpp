#include "xmodel.h"
#include "xanim.h"

struct TestLod // sizeof=0x8
{                                       // ...
    bool enabled;                       // ...
    // padding byte
    // padding byte
    // padding byte
    float dist;                         // ...
};
TestLod g_testLods[4];

const char *__cdecl XModelGetName(const XModel *model)
{
    if (!model)
        MyAssertHandler(".\\xanim\\xmodel_utils.cpp", 39, 0, "%s", "model");
    return model->name;
}

int __cdecl XModelGetSurfaces(const XModel *model, XSurface **surfaces, int lod)
{
    if (!model)
        MyAssertHandler(".\\xanim\\xmodel_utils.cpp", 53, 0, "%s", "model");
    if (!surfaces)
        MyAssertHandler(".\\xanim\\xmodel_utils.cpp", 54, 0, "%s", "surfaces");
    if (lod < 0)
        MyAssertHandler(".\\xanim\\xmodel_utils.cpp", 55, 0, "%s", "lod >= 0");
    if (model->lodInfo[lod].surfIndex >= (unsigned int)model->numsurfs)
        MyAssertHandler(
            ".\\xanim\\xmodel_utils.cpp",
            58,
            0,
            "lodInfo->surfIndex doesn't index model->numsurfs\n\t%i not in [0, %i)",
            model->lodInfo[lod].surfIndex,
            model->numsurfs);
    if (model->lodInfo[lod].numsurfs + model->lodInfo[lod].surfIndex > model->numsurfs)
        MyAssertHandler(
            ".\\xanim\\xmodel_utils.cpp",
            59,
            0,
            "%s",
            "lodInfo->surfIndex + lodInfo->numsurfs <= model->numsurfs");
    *surfaces = &model->surfs[model->lodInfo[lod].surfIndex];
    return model->lodInfo[lod].numsurfs;
}

XSurface *__cdecl XModelGetSurface(const XModel *model, int lod, int surfIndex)
{
    unsigned int modelSurfIndex; // [esp+0h] [ebp-4h]

    if (lod < 0)
        MyAssertHandler(".\\xanim\\xmodel_utils.cpp", 75, 0, "%s", "lod >= 0");
    modelSurfIndex = surfIndex + model->lodInfo[lod].surfIndex;
    if (modelSurfIndex >= model->numsurfs)
        MyAssertHandler(
            ".\\xanim\\xmodel_utils.cpp",
            78,
            0,
            "modelSurfIndex doesn't index model->numsurfs\n\t%i not in [0, %i)",
            modelSurfIndex,
            model->numsurfs);
    return &model->surfs[modelSurfIndex];
}

const XModelLodInfo *__cdecl XModelGetLodInfo(const XModel *model, int lod)
{
    if (!model)
        MyAssertHandler(".\\xanim\\xmodel_utils.cpp", 91, 0, "%s", "model");
    if (lod < 0)
        MyAssertHandler(".\\xanim\\xmodel_utils.cpp", 92, 0, "%s", "lod >= 0");
    return &model->lodInfo[lod];
}

unsigned int __cdecl XModelGetSurfCount(const XModel *model, int lod)
{
    if (!model)
        MyAssertHandler(".\\xanim\\xmodel_utils.cpp", 156, 0, "%s", "model");
    if (lod < 0)
        MyAssertHandler(".\\xanim\\xmodel_utils.cpp", 157, 0, "%s", "lod >= 0");
    return model->lodInfo[lod].numsurfs;
}

Material **__cdecl XModelGetSkins(const XModel *model, int lod)
{
    if (!model)
        MyAssertHandler(".\\xanim\\xmodel_utils.cpp", 173, 0, "%s", "model");
    if (lod < 0)
        MyAssertHandler(".\\xanim\\xmodel_utils.cpp", 174, 0, "%s", "lod >= 0");
    return &model->materialHandles[model->lodInfo[lod].surfIndex];
}

XModelLodRampType __cdecl XModelGetLodRampType(const XModel *model)
{
    return (XModelLodRampType)model->lodRampType;
}

int __cdecl XModelGetNumLods(const XModel *model)
{
    return model->numLods;
}

double __cdecl XModelGetLodOutDist(const XModel *model)
{
    return *((float *)&model->parentList + 7 * XModelGetNumLods(model));
}

int __cdecl XModelNumBones(const XModel *model)
{
    if (!model)
        MyAssertHandler(".\\xanim\\xmodel_utils.cpp", 225, 0, "%s", "model");
    return model->numBones;
}

const DObjAnimMat *__cdecl XModelGetBasePose(const XModel *model)
{
    return model->baseMat;
}

XModelLodRampType __cdecl XModelGetLodForDist(const XModel *model, float dist)
{
    float v3; // [esp+0h] [ebp-14h]
    int lodIndex; // [esp+4h] [ebp-10h]
    int lodCount; // [esp+Ch] [ebp-8h]

    lodCount = XModelGetNumLods(model);
    for (lodIndex = 0; lodIndex < lodCount; ++lodIndex)
    {
        if (g_testLods[lodIndex].enabled)
            v3 = g_testLods[lodIndex].dist;
        else
            v3 = model->lodInfo[lodIndex].dist;
        if (v3 == 0.0 || v3 > (double)dist)
            return (XModelLodRampType)lodIndex;
    }
    return (XModelLodRampType)-1;
}

void __cdecl XModelSetTestLods(unsigned int lodLevel, float dist)
{
    if (lodLevel >= 4)
        MyAssertHandler(".\\xanim\\xmodel_utils.cpp", 295, 0, "%s", "(unsigned)lodLevel < MAX_LODS");
    g_testLods[lodLevel].dist = dist;
    g_testLods[lodLevel].enabled = dist >= 0.0;
}

double __cdecl XModelGetLodDist(const XModel *model, unsigned int lod)
{
    if (!model)
        MyAssertHandler(".\\xanim\\xmodel_utils.cpp", 304, 0, "%s", "model");
    if (lod >= model->numLods)
        MyAssertHandler(
            ".\\xanim\\xmodel_utils.cpp",
            305,
            0,
            "lod doesn't index model->numLods\n\t%i not in [0, %i)",
            lod,
            model->numLods);
    return model->lodInfo[lod].dist;
}

int __cdecl XModelGetContents(const XModel *model)
{
    if (!model)
        MyAssertHandler(".\\xanim\\xmodel_utils.cpp", 317, 0, "%s", "model");
    return model->contents;
}

int __cdecl XModelGetStaticModelCacheVertCount(XModel *model, unsigned int lod)
{
    if (!model)
        MyAssertHandler(".\\xanim\\xmodel_utils.cpp", 124, 0, "%s", "model");
    if (lod >= 4)
        MyAssertHandler(".\\xanim\\xmodel_utils.cpp", 125, 0, "lod doesn't index MAX_LODS\n\t%i not in [0, %i)", lod, 4);
    //if (!model->lodInfo[lod].smcIndexPlusOne)
    //    MyAssertHandler(".\\xanim\\xmodel_utils.cpp", 126, 0, "%s", "model->lodInfo[lod].smcIndexPlusOne != 0");
    return 1 << model->lodInfo[lod].smcAllocBits;
}