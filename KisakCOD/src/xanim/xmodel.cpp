#include "xmodel.h"

#include "dobj.h"
#include "dobj_utils.h"

#include <qcommon/qcommon.h>
#include <qcommon/mem_track.h>
#include <universal/com_memory.h>
#include <universal/com_files.h>
#include <database/database.h>
#include <devgui/devgui.h>
#include <physics/phys_local.h>

XModelDefault g_default;
Material *g_materials[1];

void __cdecl XModelPartsFree(XModelPartsLoad *modelParts)
{
    int size; // [esp+0h] [ebp-Ch]
    int i; // [esp+4h] [ebp-8h]
    unsigned __int16 *boneNames; // [esp+8h] [ebp-4h]

    iassert(modelParts);

    boneNames = modelParts->boneNames;
    size = modelParts->numBones;
    for (i = 0; i < size; ++i)
        SL_RemoveRefToString(boneNames[i]);
}

bool __cdecl XModelBad(const XModel *model)
{
    iassert(model);

    if (useFastFile->current.enabled)
        return DB_IsXAssetDefault(ASSET_TYPE_XMODEL, model->name);
    else
        return model->bad;
}

void __cdecl TRACK_xmodel()
{
    track_static_alloc_internal(&g_default, sizeof(g_default), "g_default", 11);
}

XModel *__cdecl XModelPrecache(char *name, void *(__cdecl *Alloc)(int), void *(__cdecl *AllocColl)(int))
{
    if (useFastFile->current.enabled)
        return XModelPrecache_FastFile(name);
    else
        return XModelPrecache_LoadObj(name, Alloc, AllocColl);
}

static XModelPartsLoad *__cdecl XModelCreateDefaultParts()
{
    g_default.modelParts.parentList = g_default.parentList;
    g_default.modelParts.boneNames = (unsigned __int16 *)&g_default;
    g_default.modelParts.quats = 0;
    g_default.modelParts.trans = 0;
    g_default.modelParts.numBones = 1;
    g_default.modelParts.numRootBones = 1;
    g_default.modelParts.partClassification = g_default.partClassification;
    g_default.partClassification[0] = 0;
    g_default.boneNames[0] = 0;
    return &g_default.modelParts;
}

void __cdecl XModelCopyXModelParts(const XModelPartsLoad *modelParts, XModel *model)
{
    model->numBones = modelParts->numBones;
    model->numRootBones = modelParts->numRootBones;
    model->boneNames = modelParts->boneNames;
    model->parentList = modelParts->parentList;
    model->quats = modelParts->quats;
    model->trans = modelParts->trans;
    model->partClassification = modelParts->partClassification;
    model->baseMat = modelParts->baseMat;
}

static void __cdecl XModelMakeDefault(XModel *model)
{
    const XModelPartsLoad *DefaultParts; // eax

    model->bad = 1;
    DefaultParts = XModelCreateDefaultParts();
    XModelCopyXModelParts(DefaultParts, model);
    memset((unsigned __int8 *)model->lodInfo, 0, sizeof(model->lodInfo));
    model->numLods = 1;
    model->collLod = 0;
    model->name = "DEFAULT";
    model->surfs = 0;
    model->materialHandles = g_materials;
    g_materials[0] = Material_RegisterHandle("mc/$default", 8);
    g_default.boneInfo.bounds[0][0] = -16.0;
    g_default.boneInfo.bounds[0][1] = -16.0;
    g_default.boneInfo.bounds[0][2] = -16.0;
    g_default.boneInfo.bounds[1][0] = 16.0;
    g_default.boneInfo.bounds[1][1] = 16.0;
    g_default.boneInfo.bounds[1][2] = 16.0;
    model->boneInfo = &g_default.boneInfo;
}

static XModel *__cdecl XModelCreateDefault(void *(__cdecl *Alloc)(int))
{
    XModel *model; // [esp+0h] [ebp-4h]

    model = (XModel *)Alloc(332);
    XModelMakeDefault(model);
    return model;
}

static XModel *__cdecl XModelDefaultModel(const char *name, void *(__cdecl *Alloc)(int))
{
    XModel *model; // [esp+0h] [ebp-4h]

    model = XModelCreateDefault(Alloc);
    Hunk_SetDataForFile(5, name, model, Alloc);
    return model;
}

XModel *__cdecl XModelPrecache_LoadObj(char *name, void *(__cdecl *Alloc)(int), void *(__cdecl *AllocColl)(int))
{
    XModel *model; // [esp+0h] [ebp-4h]
    XModel *modela; // [esp+0h] [ebp-4h]

    model = (XModel *)Hunk_FindDataForFile(5, name);
    if (model)
        return model;
    //ProfLoad_Begin("Load xmodel");
    modela = XModelLoad(name, Alloc, AllocColl);
    //ProfLoad_End();
    if (modela)
    {
        modela->name = Hunk_SetDataForFile(5, name, modela, Alloc);
        return modela;
    }
    else
    {
        Com_PrintError(19, "ERROR: Cannot find xmodel '%s'.\n", name);
        return XModelDefaultModel(name, Alloc);
    }
}

XModel *__cdecl XModelPrecache_FastFile(const char *name)
{
    return DB_FindXAssetHeader(ASSET_TYPE_XMODEL, name).model;
}

XModel *__cdecl XModelLoad(char *name, void *(__cdecl *Alloc)(int), void *(__cdecl *AllocColl)(int))
{
    XModel *model; // [esp+0h] [ebp-4h]

    model = XModelLoadFile(name, Alloc, AllocColl);
    if (model)
        return model;
    else
        return 0;
}

void __cdecl XModelCalcBasePose(XModelPartsLoad *modelParts)
{
    float *v1; // [esp+18h] [ebp-A0h]
    DObjAnimMat *v; // [esp+1Ch] [ebp-9Ch]
    float v3; // [esp+34h] [ebp-84h]
    float v4; // [esp+38h] [ebp-80h]
    float v5; // [esp+3Ch] [ebp-7Ch]
    float v6; // [esp+40h] [ebp-78h]
    float result[3]; // [esp+44h] [ebp-74h] BYREF
    float v8; // [esp+50h] [ebp-68h]
    float v9; // [esp+54h] [ebp-64h]
    float v10; // [esp+58h] [ebp-60h]
    float v11; // [esp+5Ch] [ebp-5Ch]
    float v12; // [esp+60h] [ebp-58h]
    float v13; // [esp+64h] [ebp-54h]
    float v14; // [esp+68h] [ebp-50h]
    float v15; // [esp+6Ch] [ebp-4Ch]
    float v16; // [esp+70h] [ebp-48h]
    float v17; // [esp+74h] [ebp-44h]
    float v18; // [esp+78h] [ebp-40h]
    float v19; // [esp+7Ch] [ebp-3Ch]
    float v20; // [esp+80h] [ebp-38h]
    float v21; // [esp+84h] [ebp-34h]
    float len; // [esp+88h] [ebp-30h]
    float *v23; // [esp+8Ch] [ebp-2Ch]
    int numBones; // [esp+90h] [ebp-28h]
    float *trans; // [esp+94h] [ebp-24h]
    __int16 *quats; // [esp+98h] [ebp-20h]
    unsigned __int8 *parentList; // [esp+9Ch] [ebp-1Ch]
    int i; // [esp+A0h] [ebp-18h]
    float tempQuat[4]; // [esp+A4h] [ebp-14h] BYREF
    DObjAnimMat *quatTrans; // [esp+B4h] [ebp-4h]

    parentList = modelParts->parentList;
    numBones = modelParts->numBones;
    quats = modelParts->quats;
    trans = modelParts->trans;
    quatTrans = modelParts->baseMat;

    for (i = modelParts->numRootBones; i; --i)
    {
        quatTrans->quat[0] = 0.0f;
        quatTrans->quat[1] = 0.0f;
        quatTrans->quat[2] = 0.0f;
        quatTrans->quat[3] = 1.0f;

        quatTrans->trans[0] = 0.0f;
        quatTrans->trans[1] = 0.0f;
        quatTrans->trans[2] = 0.0f;

        quatTrans->transWeight = 2.0f;
        --i;
        ++quatTrans;
    }

    i = numBones - modelParts->numRootBones;

    while (i)
    {
        tempQuat[0] = quats[0] * 0.00003051850944757462;
        tempQuat[1] = quats[1] * 0.00003051850944757462;
        tempQuat[2] = quats[2] * 0.00003051850944757462;
        tempQuat[3] = quats[3] * 0.00003051850944757462;
        QuatMultiply(tempQuat, quatTrans[-*parentList].quat, quatTrans->quat);
        len = Vec4LengthSq(quatTrans->quat);
        if (len == 0.0f)
        {
            quatTrans->quat[3] = 1.0f;
            quatTrans->transWeight = 2.0f;
        }
        else
        {
            quatTrans->transWeight = 2.0f / len;
        }
        MatrixTransformVectorQuatTrans(trans, &quatTrans[-*parentList], quatTrans->trans);
        --i;
        quats += 4;
        trans += 3;
        ++quatTrans;
        ++parentList;
    }
}

static XModelPartsLoad *__cdecl XModelPartsLoadFile(XModel *model, const char *name, void *(__cdecl *Alloc)(int))
{
    unsigned __int8 *v4; // eax
    unsigned __int16 prev; // ax
    unsigned __int8 *v6; // [esp+10h] [ebp-A8h]
    float v7; // [esp+18h] [ebp-A0h]
    float v8; // [esp+1Ch] [ebp-9Ch]
    float v9; // [esp+20h] [ebp-98h]
    __int16 v10; // [esp+24h] [ebp-94h]
    __int16 v11; // [esp+28h] [ebp-90h]
    __int16 v12; // [esp+2Ch] [ebp-8Ch]
    const unsigned __int8 *pos; // [esp+30h] [ebp-88h] BYREF
    int numBones; // [esp+34h] [ebp-84h]
    char filename[64]; // [esp+38h] [ebp-80h] BYREF
    int numRootBones; // [esp+7Ch] [ebp-3Ch]
    __int16 numChildBones; // [esp+80h] [ebp-38h]
    unsigned __int8 *buf; // [esp+84h] [ebp-34h] BYREF
    float *trans; // [esp+88h] [ebp-30h]
    __int16 version; // [esp+8Ch] [ebp-2Ch]
    int len; // [esp+90h] [ebp-28h]
    int size; // [esp+94h] [ebp-24h]
    __int16 *quats; // [esp+98h] [ebp-20h]
    int fileSize; // [esp+9Ch] [ebp-1Ch]
    unsigned __int8 *parentList; // [esp+A0h] [ebp-18h]
    int index; // [esp+A4h] [ebp-14h]
    int i; // [esp+A8h] [ebp-10h]
    XModelPartsLoad *modelParts; // [esp+ACh] [ebp-Ch]
    bool useBones; // [esp+B3h] [ebp-5h]
    unsigned __int16 *boneNames; // [esp+B4h] [ebp-4h]

    if (Com_sprintf(filename, 0x40u, "xmodelparts/%s", name) >= 0)
    {
        fileSize = FS_ReadFile(filename, (void **)&buf);
        if (fileSize >= 0)
        {
            if (fileSize)
            {
                if (!buf)
                    MyAssertHandler(".\\xanim\\xmodel_load_obj.cpp", 156, 0, "%s", "buf");
                pos = buf;
                v12 = *(_WORD *)buf;
                pos = buf + 2;
                version = v12;
                if (v12 == 25)
                {
                    v11 = *(_WORD *)pos;
                    pos += 2;
                    numChildBones = v11;
                    v10 = *(_WORD *)pos;
                    pos += 2;
                    numRootBones = v10;
                    numBones = v10 + v11;
                    size = 2 * numBones;
                    boneNames = (unsigned __int16 *)Alloc(2 * numBones);
                    model->memUsage += size;
                    if (numBones < 128)
                    {
                        size = numChildBones;
                        if (numChildBones)
                            v6 = (unsigned __int8 *)Alloc(size);
                        else
                            v6 = 0;
                        parentList = v6;
                        model->memUsage += size;
                        size = 28;
                        modelParts = (XModelPartsLoad *)Alloc(28);
                        model->memUsage += size;
                        modelParts->parentList = parentList;
                        modelParts->boneNames = boneNames;
                        size = 32 * numBones;
                        modelParts->baseMat = (DObjAnimMat *)Alloc(32 * numBones);
                        model->memUsage += size;
                        if (numChildBones)
                        {
                            size = 8 * numChildBones;
                            modelParts->quats = (__int16 *)Alloc(size);
                            model->memUsage += size;
                            size = 16 * numChildBones;
                            modelParts->trans = (float *)Alloc(size);
                            model->memUsage += size;
                        }
                        else
                        {
                            modelParts->quats = 0;
                            modelParts->trans = 0;
                        }
                        size = numBones;
                        v4 = (unsigned __int8 *)Alloc(numBones);
                        modelParts->partClassification = v4;
                        model->memUsage += size;
                        modelParts->numBones = numBones;
                        if (modelParts->numBones != numBones)
                            MyAssertHandler(".\\xanim\\xmodel_load_obj.cpp", 219, 0, "%s", "modelParts->numBones == numBones");
                        modelParts->numRootBones = numRootBones;
                        if (modelParts->numRootBones != numRootBones)
                            MyAssertHandler(".\\xanim\\xmodel_load_obj.cpp", 222, 0, "%s", "modelParts->numRootBones == numRootBones");
                        quats = modelParts->quats;
                        trans = modelParts->trans;
                        i = numRootBones;
                        while (i < numBones)
                        {
                            index = *pos++;
                            if (index >= i)
                                MyAssertHandler(".\\xanim\\xmodel_load_obj.cpp", 233, 0, "%s", "index < i");
                            *parentList = i - index;
                            if (i - index != *parentList)
                                MyAssertHandler(".\\xanim\\xmodel_load_obj.cpp", 235, 0, "%s", "i - index == *parentList");
                            v9 = *(float *)pos;
                            pos += 4;
                            *trans = v9;
                            v8 = *(float *)pos;
                            pos += 4;
                            trans[1] = v8;
                            v7 = *(float *)pos;
                            pos += 4;
                            trans[2] = v7;
                            ConsumeQuatNoSwap(&pos, quats);
                            ++i;
                            quats += 4;
                            trans += 3;
                            ++parentList;
                        }
                        for (i = 0; i < numBones; ++i)
                        {
                            len = strlen((const char *)pos) + 1;
                            prev = SL_GetStringOfSize((char *)pos, 0, len, 10).prev;
                            boneNames[i] = prev;
                            pos += len;
                        }
                        memcpy(modelParts->partClassification, (unsigned __int8 *)pos, numBones);
                        pos += numBones;
                        useBones = *pos++ != 0;
                        FS_FreeFile((char *)buf);
                        XModelCalcBasePose(modelParts);
                        if (!useBones)
                            memset((unsigned __int8 *)modelParts->trans, 0, 16 * numChildBones);
                        return modelParts;
                    }
                    else
                    {
                        FS_FreeFile((char *)buf);
                        Com_PrintError(19, "ERROR: xmodel '%s' has more than %d bones\n", name, 127);
                        return 0;
                    }
                }
                else
                {
                    FS_FreeFile((char *)buf);
                    Com_PrintError(19, "ERROR: xmodelparts '%s' out of date (version %d, expecting %d).\n", name, version, 25);
                    return 0;
                }
            }
            else
            {
                Com_PrintError(19, "ERROR: xmodelparts '%s' has 0 length\n", name);
                FS_FreeFile((char *)buf);
                return 0;
            }
        }
        else
        {
            if (buf)
                MyAssertHandler(".\\xanim\\xmodel_load_obj.cpp", 144, 0, "%s", "!buf");
            Com_PrintError(19, "ERROR: xmodelparts '%s' not found\n", name);
            return 0;
        }
    }
    else
    {
        Com_PrintError(19, "ERROR: filename '%s' too long\n", filename);
        return 0;
    }
}
static XModelPartsLoad *__cdecl XModelPartsFindData(const char *name)
{
    return (XModelPartsLoad *)Hunk_FindDataForFile(4, name);
}

void __cdecl XModelPartsSetData(const char *name, XModelPartsLoad *modelParts, void *(__cdecl *Alloc)(int))
{
    Hunk_SetDataForFile(4, name, modelParts, Alloc);

}
XModelPartsLoad *__cdecl XModelPartsPrecache(XModel *model, const char *name, void *(__cdecl *Alloc)(int))
{
    XModelPartsLoad *modelParts; // [esp+0h] [ebp-4h]
    XModelPartsLoad *modelPartsa; // [esp+0h] [ebp-4h]

    modelParts = XModelPartsFindData(name);
    if (modelParts)
        return modelParts;
    modelPartsa = XModelPartsLoadFile(model, name, Alloc);
    if (modelPartsa)
    {
        XModelPartsSetData(name, modelPartsa, Alloc);
        return modelPartsa;
    }
    else
    {
        Com_PrintError(19, "ERROR: Cannot find xmodelparts '%s'.\n", name);
        return 0;
    }
}

double __cdecl XModelGetRadius(const XModel *model)
{
    return model->radius;
}

void __cdecl XModelGetBounds(const XModel *model, float *mins, float *maxs)
{
    *mins = model->mins[0];
    mins[1] = model->mins[1];
    mins[2] = model->mins[2];
    *maxs = model->maxs[0];
    maxs[1] = model->maxs[1];
    maxs[2] = model->maxs[2];
}

int __cdecl XModelGetMemUsage(const XModel *model)
{
    return model->memUsage;
}

int __cdecl XModelTraceLine(
    const XModel *model,
    trace_t *results,
    const float *localStart,
    const float *localEnd,
    int contentmask)
{
    double v6; // st7
    double v7; // st7
    double v8; // st7
    double v9; // st7
    float v10; // [esp+Ch] [ebp-74h]
    float v11; // [esp+10h] [ebp-70h]
    float endDist; // [esp+18h] [ebp-68h]
    int j; // [esp+1Ch] [ebp-64h]
    TraceExtents boneExtents; // [esp+20h] [ebp-60h] BYREF
    float t; // [esp+44h] [ebp-3Ch]
    float delta[3]; // [esp+48h] [ebp-38h] BYREF
    float hitFrac; // [esp+54h] [ebp-2Ch]
    const XModelCollTri_s *ctri; // [esp+58h] [ebp-28h]
    float frac; // [esp+5Ch] [ebp-24h]
    int partIndex; // [esp+60h] [ebp-20h]
    float startDist; // [esp+64h] [ebp-1Ch]
    const XModelCollSurf_s *csurf; // [esp+68h] [ebp-18h]
    float hit[3]; // [esp+6Ch] [ebp-14h] BYREF
    float s; // [esp+78h] [ebp-8h]
    unsigned int i; // [esp+7Ch] [ebp-4h]

    if (model->collLod < 0)
        return -1;
    partIndex = -1;
    boneExtents.start[0] = *localStart;
    boneExtents.start[1] = localStart[1];
    boneExtents.start[2] = localStart[2];
    boneExtents.end[0] = *localEnd;
    boneExtents.end[1] = localEnd[1];
    boneExtents.end[2] = localEnd[2];
    CM_CalcTraceExtents(&boneExtents);
    Vec3Sub(boneExtents.end, boneExtents.start, delta);
    for (i = 0; i < model->numCollSurfs; ++i)
    {
        csurf = &model->collSurfs[i];
        if ((contentmask & csurf->contents) != 0 && !CM_TraceBox(&boneExtents, (float*)csurf->mins, (float *)csurf->maxs, results->fraction))
        {
            for (j = 0; j < csurf->numCollTris; ++j)
            {
                ctri = &csurf->collTris[j];
                v6 = Vec3Dot(boneExtents.end, ctri->plane);
                endDist = v6 - ctri->plane[3];
                if (endDist < 0.0)
                {
                    v7 = Vec3Dot(boneExtents.start, ctri->plane);
                    startDist = v7 - ctri->plane[3];
                    if (startDist > 0.0)
                    {
                        frac = (startDist - 0.125) / (startDist - endDist);
                        v11 = frac - 0.0;
                        v10 = v11 < 0.0 ? 0.0 : frac;
                        frac = v10;
                        if (results->fraction > (double)v10)
                        {
                            hitFrac = startDist / (startDist - endDist);
                            Vec3Mad(boneExtents.start, hitFrac, delta, hit);
                            v8 = Vec3Dot(hit, ctri->svec);
                            s = v8 - ctri->svec[3];
                            if (s >= -EQUAL_EPSILON && s <= 1.001000046730042)
                            {
                                v9 = Vec3Dot(hit, ctri->tvec);
                                t = v9 - ctri->tvec[3];
                                if (t >= -EQUAL_EPSILON && s + t <= 1.001000046730042)
                                {
                                    partIndex = csurf->boneIdx;
                                    results->startsolid = 0;
                                    results->allsolid = 0;
                                    results->fraction = frac;
                                    if (results->fraction < 0.0 || results->fraction > 1.0)
                                        MyAssertHandler(
                                            ".\\xanim\\xmodel.cpp",
                                            499,
                                            1,
                                            "%s\n\t(results->fraction) = %g",
                                            "(results->fraction >= 0 && results->fraction <= 1.0f)",
                                            results->fraction);
                                    results->surfaceFlags = csurf->surfFlags;
                                    results->contents = csurf->contents;
                                    results->normal[0] = ctri->plane[0];
                                    results->normal[1] = ctri->plane[1];
                                    results->normal[2] = ctri->plane[2];
                                }
                            }
                        }
                    }
                }
            }
        }
    }
    return partIndex;
}

int __cdecl XModelTraceLineAnimated(
    const DObj_s *obj,
    unsigned int modelIndex,
    int baseBoneIndex,
    trace_t *results,
    const DObjAnimMat *boneMtxList,
    float *localStart,
    float *localEnd,
    int contentmask)
{
    double v9; // st7
    double v10; // st7
    double v11; // st7
    double v12; // st7
    float v13; // [esp+Ch] [ebp-1E0h]
    float v14; // [esp+10h] [ebp-1DCh]
    int v15; // [esp+14h] [ebp-1D8h]
    float v16; // [esp+38h] [ebp-1B4h]
    float v17; // [esp+3Ch] [ebp-1B0h]
    float v18; // [esp+40h] [ebp-1ACh]
    float v19; // [esp+44h] [ebp-1A8h]
    float v20[3]; // [esp+48h] [ebp-1A4h] BYREF
    float v21; // [esp+54h] [ebp-198h]
    float v22; // [esp+58h] [ebp-194h]
    float v23; // [esp+5Ch] [ebp-190h]
    float v24; // [esp+60h] [ebp-18Ch]
    float v25; // [esp+64h] [ebp-188h]
    float transWeight; // [esp+68h] [ebp-184h]
    float v27; // [esp+6Ch] [ebp-180h]
    float v28; // [esp+70h] [ebp-17Ch]
    float v29; // [esp+74h] [ebp-178h]
    float v30; // [esp+78h] [ebp-174h]
    float v31; // [esp+7Ch] [ebp-170h]
    float v32; // [esp+80h] [ebp-16Ch]
    float v33; // [esp+84h] [ebp-168h]
    float v34; // [esp+88h] [ebp-164h]
    float result[3]; // [esp+8Ch] [ebp-160h] BYREF
    float v36; // [esp+98h] [ebp-154h]
    float v37; // [esp+9Ch] [ebp-150h]
    float v38; // [esp+A0h] [ebp-14Ch]
    float v39; // [esp+A4h] [ebp-148h]
    float v40; // [esp+A8h] [ebp-144h]
    const float *trans; // [esp+ACh] [ebp-140h]
    const float *v42; // [esp+B0h] [ebp-13Ch]
    float endDist; // [esp+B4h] [ebp-138h]
    int j; // [esp+B8h] [ebp-134h]
    TraceExtents boneExtents; // [esp+BCh] [ebp-130h] BYREF
    const DObjAnimMat *baseMatList; // [esp+E0h] [ebp-10Ch]
    float t; // [esp+E4h] [ebp-108h]
    float delta[3]; // [esp+E8h] [ebp-104h] BYREF
    float invBaseMat[4][3]; // [esp+F4h] [ebp-F8h] BYREF
    float hitFrac; // [esp+124h] [ebp-C8h]
    float localStart2[3]; // [esp+128h] [ebp-C4h] BYREF
    const XModelCollTri_s *ctri; // [esp+134h] [ebp-B8h]
    float frac; // [esp+138h] [ebp-B4h]
    const XModel *model; // [esp+13Ch] [ebp-B0h]
    int partIndex; // [esp+140h] [ebp-ACh]
    float startDist; // [esp+144h] [ebp-A8h]
    const XModelCollSurf_s *csurf; // [esp+148h] [ebp-A4h]
    float localEnd2[3]; // [esp+14Ch] [ebp-A0h] BYREF
    int globalBoneIndex; // [esp+158h] [ebp-94h]
    float hit[3]; // [esp+15Ch] [ebp-90h] BYREF
    float in2[12]; // [esp+168h] [ebp-84h] BYREF
    unsigned int hidePartBits[4]; // [esp+198h] [ebp-54h] BYREF
    const DObjAnimMat *baseMat; // [esp+1A8h] [ebp-44h]
    float s; // [esp+1ACh] [ebp-40h]
    unsigned int i; // [esp+1B0h] [ebp-3Ch]
    int boneIdx; // [esp+1B4h] [ebp-38h]
    float axis[4][3]; // [esp+1B8h] [ebp-34h] BYREF
    const DObjAnimMat *boneMtx; // [esp+1E8h] [ebp-4h]

    model = obj->models[modelIndex];
    if (model->collLod < 0)
        return -1;
    partIndex = -1;
    baseMatList = XModelGetBasePose(model);
    DObjGetHidePartBits(obj, hidePartBits);
    for (i = 0; i < model->numCollSurfs; ++i)
    {
        csurf = &model->collSurfs[i];
        if ((contentmask & csurf->contents) != 0)
        {
            boneIdx = csurf->boneIdx;
            globalBoneIndex = boneIdx + baseBoneIndex;
            if ((hidePartBits[(boneIdx + baseBoneIndex) >> 5] & (0x80000000 >> ((boneIdx + baseBoneIndex) & 0x1F))) == 0)
            {
                boneMtx = &boneMtxList[boneIdx];
                baseMat = &baseMatList[boneIdx];
                if (Vec4Compare(boneMtx->quat, baseMat->quat)
                    && ((trans = baseMat->trans, v42 = boneMtx->trans, baseMat->trans[0] != boneMtx->trans[0])
                        || trans[1] != v42[1]
                        || trans[2] != v42[2]
                        ? (v15 = 0)
                        : (v15 = 1),
                        v15))
                {
                    boneExtents.start[0] = *localStart;
                    boneExtents.start[1] = localStart[1];
                    boneExtents.start[2] = localStart[2];
                    boneExtents.end[0] = *localEnd;
                    boneExtents.end[1] = localEnd[1];
                    boneExtents.end[2] = localEnd[2];
                }
                else
                {
                    v30 = baseMat->quat[0];
                    if ((LODWORD(v30) & 0x7F800000) == 0x7F800000
                        || (v29 = baseMat->quat[1], (LODWORD(v29) & 0x7F800000) == 0x7F800000)
                        || (v28 = baseMat->quat[2], (LODWORD(v28) & 0x7F800000) == 0x7F800000)
                        || (v27 = baseMat->quat[3], (LODWORD(v27) & 0x7F800000) == 0x7F800000))
                    {
                        MyAssertHandler(
                            "c:\\trees\\cod3\\src\\xanim\\xanim_public.h",
                            536,
                            0,
                            "%s",
                            "!IS_NAN((mat->quat)[0]) && !IS_NAN((mat->quat)[1]) && !IS_NAN((mat->quat)[2]) && !IS_NAN((mat->quat)[3])");
                    }
                    transWeight = baseMat->transWeight;
                    if ((LODWORD(transWeight) & 0x7F800000) == 0x7F800000)
                        MyAssertHandler("c:\\trees\\cod3\\src\\xanim\\xanim_public.h", 537, 0, "%s", "!IS_NAN(mat->transWeight)");
                    Vec3Scale(baseMat->quat, baseMat->transWeight, result);
                    v39 = result[0] * baseMat->quat[0];
                    v32 = result[0] * baseMat->quat[1];
                    v37 = result[0] * baseMat->quat[2];
                    v40 = result[0] * baseMat->quat[3];
                    v31 = result[1] * baseMat->quat[1];
                    v38 = result[1] * baseMat->quat[2];
                    v36 = result[1] * baseMat->quat[3];
                    v33 = result[2] * baseMat->quat[2];
                    v34 = result[2] * baseMat->quat[3];
                    invBaseMat[0][0] = 1.0 - (v31 + v33);
                    invBaseMat[0][1] = v32 - v34;
                    invBaseMat[0][2] = v37 + v36;
                    invBaseMat[1][0] = v32 + v34;
                    invBaseMat[1][1] = 1.0 - (v39 + v33);
                    invBaseMat[1][2] = v38 - v40;
                    invBaseMat[2][0] = v37 - v36;
                    invBaseMat[2][1] = v38 + v40;
                    invBaseMat[2][2] = 1.0 - (v39 + v31);
                    invBaseMat[3][0] = -(baseMat->trans[0] * invBaseMat[0][0]
                        + baseMat->trans[1] * invBaseMat[1][0]
                        + baseMat->trans[2] * invBaseMat[2][0]);
                    invBaseMat[3][1] = -(baseMat->trans[0] * invBaseMat[0][1]
                        + baseMat->trans[1] * invBaseMat[1][1]
                        + baseMat->trans[2] * invBaseMat[2][1]);
                    invBaseMat[3][2] = -(baseMat->trans[0] * invBaseMat[0][2]
                        + baseMat->trans[1] * invBaseMat[1][2]
                        + baseMat->trans[2] * invBaseMat[2][2]);
                    if ((COERCE_UNSIGNED_INT(boneMtx->quat[0]) & 0x7F800000) == 0x7F800000
                        || (COERCE_UNSIGNED_INT(boneMtx->quat[1]) & 0x7F800000) == 0x7F800000
                        || (COERCE_UNSIGNED_INT(boneMtx->quat[2]) & 0x7F800000) == 0x7F800000
                        || (COERCE_UNSIGNED_INT(boneMtx->quat[3]) & 0x7F800000) == 0x7F800000)
                    {
                        MyAssertHandler(
                            "c:\\trees\\cod3\\src\\xanim\\xanim_public.h",
                            432,
                            0,
                            "%s",
                            "!IS_NAN((mat->quat)[0]) && !IS_NAN((mat->quat)[1]) && !IS_NAN((mat->quat)[2]) && !IS_NAN((mat->quat)[3])");
                    }
                    if ((COERCE_UNSIGNED_INT(boneMtx->transWeight) & 0x7F800000) == 0x7F800000)
                        MyAssertHandler("c:\\trees\\cod3\\src\\xanim\\xanim_public.h", 433, 0, "%s", "!IS_NAN(mat->transWeight)");
                    Vec3Scale(boneMtx->quat, boneMtx->transWeight, v20);
                    v24 = v20[0] * boneMtx->quat[0];
                    v17 = v20[0] * boneMtx->quat[1];
                    v22 = v20[0] * boneMtx->quat[2];
                    v25 = v20[0] * boneMtx->quat[3];
                    v16 = v20[1] * boneMtx->quat[1];
                    v23 = v20[1] * boneMtx->quat[2];
                    v21 = v20[1] * boneMtx->quat[3];
                    v18 = v20[2] * boneMtx->quat[2];
                    v19 = v20[2] * boneMtx->quat[3];
                    in2[0] = 1.0 - (v16 + v18);
                    in2[1] = v17 + v19;
                    in2[2] = v22 - v21;
                    in2[3] = v17 - v19;
                    in2[4] = 1.0 - (v24 + v18);
                    in2[5] = v23 + v25;
                    in2[6] = v22 + v21;
                    in2[7] = v23 - v25;
                    in2[8] = 1.0 - (v24 + v16);
                    in2[9] = boneMtx->trans[0];
                    in2[10] = boneMtx->trans[1];
                    in2[11] = boneMtx->trans[2];
                    MatrixMultiply43(invBaseMat, *(mat4x3*)&in2[0], axis);
                    Vec3Sub(localStart, axis[3], localStart2);
                    Vec3Sub(localEnd, axis[3], localEnd2);
                    LocalMatrixTransposeTransformVector(localStart2, axis, boneExtents.start);
                    LocalMatrixTransposeTransformVector(localEnd2, axis, boneExtents.end);
                }
                CM_CalcTraceExtents(&boneExtents);
                if (!CM_TraceBox(&boneExtents, (float*) csurf->mins, (float*) csurf->maxs, results->fraction))
                {
                    Vec3Sub(boneExtents.end, boneExtents.start, delta);
                    for (j = 0; j < csurf->numCollTris; ++j)
                    {
                        ctri = &csurf->collTris[j];
                        v9 = Vec3Dot(boneExtents.end, ctri->plane);
                        endDist = v9 - ctri->plane[3];
                        if (endDist < 0.0)
                        {
                            v10 = Vec3Dot(boneExtents.start, ctri->plane);
                            startDist = v10 - ctri->plane[3];
                            if (startDist > 0.0)
                            {
                                frac = (startDist - 0.125) / (startDist - endDist);
                                v14 = frac - 0.0;
                                v13 = v14 < 0.0 ? 0.0 : frac;
                                frac = v13;
                                if (results->fraction > v13)
                                {
                                    hitFrac = startDist / (startDist - endDist);
                                    Vec3Mad(boneExtents.start, hitFrac, delta, hit);
                                    v11 = Vec3Dot(hit, ctri->svec);
                                    s = v11 - ctri->svec[3];
                                    if (s >= -EQUAL_EPSILON && s <= 1.001000046730042)
                                    {
                                        v12 = Vec3Dot(hit, ctri->tvec);
                                        t = v12 - ctri->tvec[3];
                                        if (t >= -EQUAL_EPSILON && s + t <= 1.001000046730042)
                                        {
                                            partIndex = csurf->boneIdx;
                                            results->startsolid = 0;
                                            results->allsolid = 0;
                                            results->fraction = frac;
                                            if (results->fraction < 0.0 || results->fraction > 1.0)
                                                MyAssertHandler(
                                                    ".\\xanim\\xmodel.cpp",
                                                    741,
                                                    1,
                                                    "%s\n\t(results->fraction) = %g",
                                                    "(results->fraction >= 0 && results->fraction <= 1.0f)",
                                                    results->fraction);
                                            results->surfaceFlags = csurf->surfFlags;
                                            results->contents = csurf->contents;
                                            results->normal[0] = ctri->plane[0];
                                            results->normal[1] = ctri->plane[1];
                                            results->normal[2] = ctri->plane[2];
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
    }
    return partIndex;
}

void __cdecl LocalMatrixTransposeTransformVector(const float *in1, const float (*in2)[3], float *out)
{
    *out = *in1 * (*in2)[0] + in1[1] * (float)(*in2)[1] + in1[2] * (float)(*in2)[2];
    out[1] = *in1 * (float)(*in2)[3] + in1[1] * (float)(*in2)[4] + in1[2] * (float)(*in2)[5];
    out[2] = *in1 * (float)(*in2)[6] + in1[1] * (float)(*in2)[7] + in1[2] * (float)(*in2)[8];
}

void __cdecl XModelTraceLineAnimatedPartBits(
    const DObj_s *obj,
    unsigned int modelIndex,
    int baseBoneIndex,
    int contentmask,
    int *partBits)
{
    const XModel *model; // [esp+0h] [ebp-24h]
    const XModelCollSurf_s *csurf; // [esp+4h] [ebp-20h]
    unsigned int hidePartBits[4]; // [esp+Ch] [ebp-18h] BYREF
    unsigned int i; // [esp+1Ch] [ebp-8h]
    int boneIdx; // [esp+20h] [ebp-4h]

    model = obj->models[modelIndex];
    if (model->collLod >= 0)
    {
        DObjGetHidePartBits(obj, hidePartBits);
        for (i = 0; i < model->numCollSurfs; ++i)
        {
            csurf = &model->collSurfs[i];
            if ((contentmask & csurf->contents) != 0)
            {
                boneIdx = csurf->boneIdx;
                if ((hidePartBits[(boneIdx + baseBoneIndex) >> 5] & (0x80000000 >> ((boneIdx + baseBoneIndex) & 0x1F))) == 0)
                    partBits[(boneIdx + baseBoneIndex) >> 5] |= 0x80000000 >> ((boneIdx + baseBoneIndex) & 0x1F);
            }
        }
    }
}

void __cdecl PrefetchArray_XSurfaceCollisionNode_(const XSurfaceCollisionNode *mem, unsigned int elementCount)
{
    const unsigned __int8 *memIter; // [esp+0h] [ebp-8h]

    if (!elementCount)
        MyAssertHandler(".\\xanim\\xmodel.cpp", 883, 0, "%s", "elementCount");
    for (memIter = (const unsigned __int8 *)((uintptr_t)mem & 0xFFFFFF80); 
        memIter <= (const unsigned __int8 *)(((uintptr_t) & mem[elementCount - 1].childCount + 1) & 0xFFFFFF80);
        memIter += 128)
        ;
}

char __cdecl XSurfaceVisitTrianglesInAabb(
    const XSurface *surface,
    unsigned int vertListIndex,
    const float *aabbMins,
    const float *aabbMaxs,
    bool(__cdecl *visitorFunc)(void *, const GfxPackedVertex **, const GfxPackedVertex **),
    void *visitorContext)
{
    XSurfaceGetTriCandidatesLocals locals; // [esp+6Ch] [ebp-2B0h] BYREF
    XRigidVertList *vertList; // [esp+318h] [ebp-4h]

    if (surface->deformed)
        MyAssertHandler(".\\xanim\\xmodel.cpp", 1093, 0, "%s", "!surface->deformed");
    if (vertListIndex >= surface->vertListCount)
        MyAssertHandler(
            ".\\xanim\\xmodel.cpp",
            1095,
            0,
            "%s\n\t(vertListIndex) = %i",
            "(vertListIndex >= 0 && vertListIndex < surface->vertListCount)",
            vertListIndex);
    vertList = &surface->vertList[vertListIndex];
    locals.tree = vertList->collisionTree;
    if (!locals.tree)
        MyAssertHandler(".\\xanim\\xmodel.cpp", 1098, 0, "%s", "locals.tree");
    PrefetchArray_XSurfaceCollisionNode_(locals.tree->nodes, 1u);
    locals.visitorFunc = visitorFunc;
    locals.visitorContext = visitorContext;
    locals.inIndices = surface->triIndices;
    locals.inVertices0 = surface->verts0;
    XSurfaceVisitTrianglesInAabb_ConvertAabb(locals.tree, aabbMins, aabbMaxs, locals.mins, locals.maxs);
    locals.nodeQueue[0].count = 1;
    locals.nodeQueueBegin = 0;
    locals.nodeQueueEnd = 1;
    memset(&locals.leafQueueBegin, 0, 28);
    while (locals.nodeQueueBegin != locals.nodeQueueEnd)
    {
        if (!XSurfaceVisitTrianglesInAabb_ProcessNode(&locals))
            return 0;
    }
    while (locals.leafQueueBegin != locals.leafQueueEnd)
    {
        if (!XSurfaceVisitTrianglesInAabb_ProcessLeaf(&locals))
            return 0;
    }
    while (locals.triangleQueueBegin != locals.triangleQueueEnd)
    {
        if (!XSurfaceVisitTrianglesInAabb_ProcessTriangles(&locals))
            return 0;
    }
    while (locals.vertexQueueBegin != locals.vertexQueueEnd)
    {
        if (!XSurfaceVisitTrianglesInAabb_ProcessVertices(&locals))
            return 0;
    }
    return 1;
}

void __cdecl XSurfaceVisitTrianglesInAabb_ConvertAabb(
    const XSurfaceCollisionTree *tree,
    const float *aabbMins,
    const float *aabbMaxs,
    int *mins,
    int *maxs)
{
    float translatedMaxs[3]; // [esp+90h] [ebp-30h] BYREF
    float translatedMins[3]; // [esp+9Ch] [ebp-24h] BYREF
    float transformedMins[3]; // [esp+A8h] [ebp-18h] BYREF
    float transformedMaxs[3]; // [esp+B4h] [ebp-Ch] BYREF

    Vec3Add(aabbMins, tree->trans, translatedMins);
    Vec3Add(aabbMaxs, tree->trans, translatedMaxs);
    Vec3Mul(translatedMins, tree->scale, transformedMins);
    Vec3Mul(translatedMaxs, tree->scale, transformedMaxs);

    static const float minfloat = -1000000.0f; // LWSS ADD
    static const float maxfloat = 1000000.0f; // LWSS ADD

    mins[0] = CLAMP((transformedMins[0] - 0.5), minfloat, maxfloat);
    mins[1] = CLAMP((transformedMins[1] - 0.5), minfloat, maxfloat);
    mins[2] = CLAMP((transformedMins[2] - 0.5), minfloat, maxfloat);

    maxs[0] = CLAMP((transformedMaxs[0] + 0.5), minfloat, maxfloat);
    maxs[1] = CLAMP((transformedMaxs[1] + 0.5), minfloat, maxfloat);
    maxs[2] = CLAMP((transformedMaxs[2] + 0.5), minfloat, maxfloat);
}

bool __cdecl XSurfaceVisitTrianglesInAabb_ProcessVertices(XSurfaceGetTriCandidatesLocals *locals)
{
    const GfxPackedVertex *verts0[3]; // [esp+4h] [ebp-10h] BYREF
    unsigned int vertIter; // [esp+10h] [ebp-4h]

    for (vertIter = 0; vertIter != 3; ++vertIter)
        verts0[vertIter] = &locals->inVertices0[locals->vertexQueue[locals->vertexQueueBegin][vertIter]];
    locals->vertexQueueBegin = ((unsigned __int8)locals->vertexQueueBegin + 1) & 3;

    return locals->visitorFunc(locals->visitorContext,verts0,verts0);
}

void __cdecl PrefetchArray_GfxPackedVertex_(const GfxPackedVertex *mem, unsigned int elementCount)
{
    const unsigned __int8 *memIter; // [esp+0h] [ebp-8h]

    if (!elementCount)
        MyAssertHandler(".\\xanim\\xmodel.cpp", 883, 0, "%s", "elementCount");
    for (memIter = (const unsigned __int8*)((uintptr_t)mem & 0xFFFFFF80); 
        memIter <= (const unsigned __int8*)(((uintptr_t) & mem[elementCount - 1].tangent + 3) & 0xFFFFFF80);
        memIter += 128)
        ;
}
char __cdecl XSurfaceVisitTrianglesInAabb_ProcessTriangles(XSurfaceGetTriCandidatesLocals *locals)
{
    unsigned __int16 index; // [esp+1Ch] [ebp-18h]
    unsigned int vertIter; // [esp+20h] [ebp-14h]
    unsigned int triangleIter; // [esp+24h] [ebp-10h]
    const unsigned __int16 *indexPtr; // [esp+28h] [ebp-Ch]
    unsigned int triangleCount; // [esp+2Ch] [ebp-8h]
    unsigned int triangleBegin; // [esp+30h] [ebp-4h]

    triangleBegin = locals->triangleQueue[locals->triangleQueueBegin].beginIndex;
    triangleCount = locals->triangleQueue[locals->triangleQueueBegin].count;
    locals->triangleQueueBegin = ((unsigned __int8)locals->triangleQueueBegin + 1) & 3;
    indexPtr = &locals->inIndices[3 * triangleBegin];
    for (triangleIter = 0; triangleIter != triangleCount; ++triangleIter)
    {
        for (vertIter = 0; vertIter != 3; ++vertIter)
        {
            index = *indexPtr;
            PrefetchArray_GfxPackedVertex_(&locals->inVertices0[*indexPtr++], 1u);
            locals->vertexQueue[locals->vertexQueueEnd][vertIter] = index;
        }
        locals->vertexQueueEnd = ((unsigned __int8)locals->vertexQueueEnd + 1) & 3;
        if (locals->vertexQueueBegin == locals->vertexQueueEnd && !XSurfaceVisitTrianglesInAabb_ProcessVertices(locals))
        {
            return 0;
        }
    }
    return 1;
}

void __cdecl PrefetchArray_XSurfaceCollisionLeaf_(const XSurfaceCollisionLeaf *mem, unsigned int elementCount)
{
    const unsigned __int8 *memIter; // [esp+0h] [ebp-8h]

    if (!elementCount)
        MyAssertHandler(".\\xanim\\xmodel.cpp", 883, 0, "%s", "elementCount");
    for (memIter = (const unsigned __int8*)((uintptr_t)mem & 0xFFFFFF80);
        memIter <= (const unsigned __int8*)(((uintptr_t) & mem[elementCount - 1].triangleBeginIndex + 1) & 0xFFFFFF80);
        memIter += 128)
    {
        ;
    }
}

char __cdecl XSurfaceVisitTrianglesInAabb_ProcessLeaf(XSurfaceGetTriCandidatesLocals *locals)
{
    XSurfaceCollisionLeaf *leaf; // [esp+3Ch] [ebp-18h]
    unsigned int indexCount; // [esp+40h] [ebp-14h]
    int indexBeginIndex; // [esp+44h] [ebp-10h]
    unsigned int leafBeginIndex; // [esp+48h] [ebp-Ch]
    unsigned int leafIndex; // [esp+4Ch] [ebp-8h]
    unsigned int leafEndIndex; // [esp+50h] [ebp-4h]

    leafBeginIndex = locals->leafQueue[locals->leafQueueBegin].beginIndex;
    leafEndIndex = locals->leafQueue[locals->leafQueueBegin].count + leafBeginIndex;
    locals->leafQueueBegin = ((unsigned __int8)locals->leafQueueBegin + 1) & 3;
    for (leafIndex = leafBeginIndex; leafIndex != leafEndIndex; ++leafIndex)
    {
        leaf = &locals->tree->leafs[leafIndex];
        if (leaf->triangleBeginIndex < 0x8000u)
        {
            indexBeginIndex = leaf->triangleBeginIndex;
            indexCount = 1;
        }
        else
        {
            indexBeginIndex = leaf->triangleBeginIndex - 0x8000;
            indexCount = 2;
        }
        PrefetchArray_XSurfaceCollisionLeaf_(
            (const XSurfaceCollisionLeaf *)&locals->inIndices[3 * indexBeginIndex],
            3 * indexCount);
        locals->triangleQueue[locals->triangleQueueEnd].beginIndex = indexBeginIndex;
        locals->triangleQueue[locals->triangleQueueEnd].count = indexCount;
        locals->triangleQueueEnd = ((unsigned __int8)locals->triangleQueueEnd + 1) & 3;
        if (locals->triangleQueueBegin == locals->triangleQueueEnd
            && !XSurfaceVisitTrianglesInAabb_ProcessTriangles(locals))
        {
            return 0;
        }
    }
    return 1;
}

char __cdecl XSurfaceVisitTrianglesInAabb_ProcessNode(XSurfaceGetTriCandidatesLocals *locals)
{
    unsigned int childBeginIndex; // [esp+48h] [ebp-1Ch]
    unsigned int childCount; // [esp+4Ch] [ebp-18h]
    unsigned int childCounta; // [esp+4Ch] [ebp-18h]
    XSurfaceCollisionNode *node; // [esp+50h] [ebp-14h]
    unsigned int nodeIndex; // [esp+58h] [ebp-Ch]
    unsigned int nodeEndIndex; // [esp+5Ch] [ebp-8h]
    unsigned int nodeBeginIndex; // [esp+60h] [ebp-4h]

    nodeBeginIndex = locals->nodeQueue[locals->nodeQueueBegin].beginIndex;
    nodeEndIndex = locals->nodeQueue[locals->nodeQueueBegin].count + nodeBeginIndex;
    locals->nodeQueueBegin = ((unsigned __int8)locals->nodeQueueBegin + 1) & 0x3F;
    for (nodeIndex = nodeBeginIndex; nodeIndex != nodeEndIndex; ++nodeIndex)
    {
        node = &locals->tree->nodes[nodeIndex];
        if (locals->maxs[0] >= node->aabb.mins[0]
            && locals->mins[0] <= node->aabb.maxs[0]
            && locals->maxs[1] >= node->aabb.mins[1]
            && locals->mins[1] <= node->aabb.maxs[1]
            && locals->maxs[2] >= node->aabb.mins[2]
            && locals->mins[2] <= node->aabb.maxs[2])
        {
            childBeginIndex = node->childBeginIndex;
            childCount = node->childCount;
            if (childCount < 0x8000)
            {
                PrefetchArray_XSurfaceCollisionNode_(&locals->tree->nodes[childBeginIndex], childCount);
                locals->nodeQueue[locals->nodeQueueEnd].beginIndex = childBeginIndex;
                locals->nodeQueue[locals->nodeQueueEnd].count = childCount;
                locals->nodeQueueEnd = ((unsigned __int8)locals->nodeQueueEnd + 1) & 0x3F;
                if (locals->nodeQueueBegin == locals->nodeQueueEnd)
                    MyAssertHandler(".\\xanim\\xmodel.cpp", 1079, 0, "%s", "locals.nodeQueueBegin != locals.nodeQueueEnd");
            }
            else
            {
                childCounta = childCount - 0x8000;
                PrefetchArray_XSurfaceCollisionLeaf_(&locals->tree->leafs[childBeginIndex], childCounta);
                locals->leafQueue[locals->leafQueueEnd].beginIndex = childBeginIndex;
                locals->leafQueue[locals->leafQueueEnd].count = childCounta;
                locals->leafQueueEnd = ((unsigned __int8)locals->leafQueueEnd + 1) & 3;
                if (locals->leafQueueBegin == locals->leafQueueEnd && !XSurfaceVisitTrianglesInAabb_ProcessLeaf(locals))
                    return 0;
            }
        }
    }
    return 1;
}

int __cdecl XModelGetBoneIndex(const XModel *model, unsigned int name, unsigned int offset, unsigned __int8 *index)
{
    unsigned int numBones; // [esp+0h] [ebp-Ch]
    unsigned int localBoneIndex; // [esp+4h] [ebp-8h]
    unsigned __int16 *boneNames; // [esp+8h] [ebp-4h]

    if (!index)
        MyAssertHandler(".\\xanim\\xmodel.cpp", 1184, 0, "%s", "index");
    boneNames = model->boneNames;
    numBones = model->numBones;
    if (numBones >= 0x80)
        MyAssertHandler(".\\xanim\\xmodel.cpp", 1189, 0, "%s", "numBones < DOBJ_MAX_PARTS");
    for (localBoneIndex = 0; ; ++localBoneIndex)
    {
        if (localBoneIndex >= numBones)
            return 0;
        if (name == boneNames[localBoneIndex])
            break;
    }
    *index = localBoneIndex + offset;
    if (*index != localBoneIndex + offset)
        MyAssertHandler(".\\xanim\\xmodel.cpp", 1196, 0, "%s", "*index == offset + localBoneIndex");
    return 1;
}

unsigned __int16 *__cdecl XModelBoneNames(XModel *model)
{
    return model->boneNames;
}

