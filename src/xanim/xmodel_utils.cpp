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
    iassert(model);
    return model->name;
}

int __cdecl XModelGetSurfaces(const XModel *model, XSurface **surfaces, int lod)
{
    iassert(model);
    iassert(surfaces);
    iassert(lod >= 0);
    bcassert(model->lodInfo[lod].surfIndex, model->numsurfs);
    const XModelLodInfo *lodInfo = model->lodInfo;
    iassert(lodInfo->surfIndex + lodInfo->numsurfs <= model->numsurfs);
    *surfaces = &model->surfs[lodInfo[lod].surfIndex];
    return lodInfo[lod].numsurfs;
}

XSurface *__cdecl XModelGetSurface(const XModel *model, int lod, int surfIndex)
{
    unsigned int modelSurfIndex; // [esp+0h] [ebp-4h]

    iassert(lod >= 0);
    modelSurfIndex = surfIndex + model->lodInfo[lod].surfIndex;
    bcassert(modelSurfIndex, model->numsurfs);
    return &model->surfs[modelSurfIndex];
}

const XModelLodInfo *__cdecl XModelGetLodInfo(const XModel *model, int lod)
{
    iassert(model);
    iassert(lod >= 0);

    return &model->lodInfo[lod];
}

unsigned int __cdecl XModelGetSurfCount(const XModel *model, int lod)
{
    iassert(model);
    iassert(lod >= 0);

    return model->lodInfo[lod].numsurfs;
}

Material **__cdecl XModelGetSkins(const XModel *model, int lod)
{
    iassert(model);
    iassert(lod >= 0);

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
    iassert(model);

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
    iassert((unsigned)lodLevel < MAX_LODS);

    g_testLods[lodLevel].dist = dist;
    g_testLods[lodLevel].enabled = dist >= 0.0;
}

double __cdecl XModelGetLodDist(const XModel *model, unsigned int lod)
{
    iassert(model);
    bcassert(lod, model->numLods);

    return model->lodInfo[lod].dist;
}

int __cdecl XModelGetContents(const XModel *model)
{
    iassert(model);

    return model->contents;
}

int __cdecl XModelGetStaticModelCacheVertCount(XModel *model, unsigned int lod)
{
    iassert(model);
    bcassert(lod, MAX_LODS);
    iassert(model->lodInfo[lod].smcIndexPlusOne != 0);

    return 1 << model->lodInfo[lod].smcAllocBits;
}