#include "dobj_utils.h"
#include "xmodel.h"
#include <universal/assertive.h>
#include <gfx_d3d/r_utils.h>
#include <universal/profile.h>

DObjAnimMat *__cdecl DObjGetRotTransArray(const DObj_s *obj)
{
    iassert(obj);
    return obj->skel.mat;
}

int __cdecl DObjGetNumModels(const DObj_s *obj)
{
    return obj->numModels;
}

int __cdecl DObjGetSurfaces(const DObj_s *obj, int *partBits, const char *lods)
{
    int j; // [esp+0h] [ebp-4Ch]
    int numBones; // [esp+4h] [ebp-48h]
    int numBonesa; // [esp+4h] [ebp-48h]
    int boneIndex; // [esp+8h] [ebp-44h]
    int numModels; // [esp+10h] [ebp-3Ch]
    XModel *model; // [esp+14h] [ebp-38h]
    XModel *modela; // [esp+14h] [ebp-38h]
    int surfaceCount; // [esp+18h] [ebp-34h]
    char targBoneIndexLow; // [esp+1Ch] [ebp-30h]
    int lod; // [esp+24h] [ebp-28h]
    int loda; // [esp+24h] [ebp-28h]
    int targBoneIndexHigh; // [esp+28h] [ebp-24h]
    int surfPartBits[7]; // [esp+2Ch] [ebp-20h] BYREF
    XModel **models; // [esp+48h] [ebp-4h]

    numModels = obj->numModels;
    iassert(numModels);
    models = obj->models;
    model = *models;
    numBones = XModelNumBones(*models);
    lod = *lods;
    if (lod < 0)
    {
        surfaceCount = 0;
        partBits[0] = 0;
        partBits[1] = 0;
        partBits[2] = 0;
        partBits[3] = 0;
    }
    else
    {
        surfaceCount = model->lodInfo[lod].numsurfs;
        partBits[0] = model->lodInfo[lod].partBits[0];
        partBits[1] = model->lodInfo[lod].partBits[1];
        partBits[2] = model->lodInfo[lod].partBits[2];
        partBits[3] = model->lodInfo[lod].partBits[3];
    }
    boneIndex = numBones;
    memset(surfPartBits, 0, sizeof(surfPartBits));
    for (j = 1; j < numModels; ++j)
    {
        modela = models[j];
        numBonesa = XModelNumBones(modela);
        loda = lods[j];
        if (loda >= 0)
        {
            surfaceCount += modela->lodInfo[loda].numsurfs;

            surfPartBits[3] = modela->lodInfo[loda].partBits[0];
            surfPartBits[4] = modela->lodInfo[loda].partBits[1];
            surfPartBits[5] = modela->lodInfo[loda].partBits[2];
            surfPartBits[6] = modela->lodInfo[loda].partBits[3];

            targBoneIndexHigh = boneIndex >> 5;
            targBoneIndexLow = boneIndex & 0x1F;

            if ((boneIndex & 0x1F) != 0)
            {
                *partBits |= (unsigned int)surfPartBits[3 - targBoneIndexHigh] >> targBoneIndexLow;
                partBits[1] |= ((unsigned int)surfPartBits[4 - targBoneIndexHigh] >> targBoneIndexLow)
                    | (surfPartBits[3 - targBoneIndexHigh] << (32 - targBoneIndexLow));
                partBits[2] |= ((unsigned int)surfPartBits[5 - targBoneIndexHigh] >> targBoneIndexLow)
                    | (surfPartBits[4 - targBoneIndexHigh] << (32 - targBoneIndexLow));
                partBits[3] |= ((unsigned int)surfPartBits[6 - targBoneIndexHigh] >> targBoneIndexLow)
                    | (surfPartBits[5 - targBoneIndexHigh] << (32 - targBoneIndexLow));
            }
            else
            {
                partBits[0] |= surfPartBits[3 - targBoneIndexHigh];
                partBits[1] |= surfPartBits[4 - targBoneIndexHigh];
                partBits[2] |= surfPartBits[5 - targBoneIndexHigh];
                partBits[3] |= surfPartBits[6 - targBoneIndexHigh];
            }
        }
        boneIndex += numBonesa;
    }
    return surfaceCount;
}

void __cdecl DObjGetSurfaceData(const DObj_s *obj, const float *origin, float scale, char *lods)
{
    XModelLodRampType lodRampType; // [esp+Ch] [ebp-18h]
    XModel *model; // [esp+10h] [ebp-14h]
    float adjustedDist; // [esp+14h] [ebp-10h]
    float baseDist; // [esp+18h] [ebp-Ch]
    int modelCount; // [esp+1Ch] [ebp-8h]
    int modelIndex; // [esp+20h] [ebp-4h]

    iassert(obj);
    modelCount = DObjGetNumModels(obj);
    iassert(modelCount <= DOBJ_MAX_SUBMODELS);
    iassert(scale != 0.0);

    baseDist = R_GetBaseLodDist(origin) * (1.0 / scale);

    iassert(!IS_NAN(scale));
    iassert(!IS_NAN(baseDist));

    for (modelIndex = 0; modelIndex < modelCount; ++modelIndex)
    {
        model = DObjGetModel(obj, modelIndex);
        lodRampType = XModelGetLodRampType(model);
        adjustedDist = R_GetAdjustedLodDist(baseDist, lodRampType);
        lods[modelIndex] = DObjGetLodForDist(obj, modelIndex, adjustedDist);
    }
}

void __cdecl DObjGetBoneInfo(const DObj_s *obj, XBoneInfo **boneInfo)
{
    int j; // [esp+0h] [ebp-14h]
    XModel *model; // [esp+4h] [ebp-10h]
    int size; // [esp+8h] [ebp-Ch]
    int i; // [esp+Ch] [ebp-8h]
    XModel **models; // [esp+10h] [ebp-4h]

    models = obj->models;
    for (j = 0; j < obj->numModels; ++j)
    {
        model = models[j];
        size = model->numBones;
        for (i = 0; i < size; ++i)
            *boneInfo++ = &model->boneInfo[i];
    }
}

int __cdecl DObjNumBones(const DObj_s *obj)
{
    return obj->numBones;
}

int __cdecl DObjGetLodForDist(const DObj_s *obj, int modelIndex, float dist)
{
    return XModelGetLodForDist(obj->models[modelIndex], dist);
}

void __cdecl DObjGetSetBones(const DObj_s *obj, int *setPartBits)
{
    setPartBits[0] = obj->skel.partBits.anim[0];
    setPartBits[1] = obj->skel.partBits.anim[1];
    setPartBits[2] = obj->skel.partBits.anim[2];
    setPartBits[3] = obj->skel.partBits.anim[3];
}

unsigned int __cdecl DObjGetRootBoneCount(const DObj_s *obj)
{
    XModel *model;

    model = DObjGetModel(obj, 0);
    iassert(model->numRootBones);
    return model->numRootBones;
}

int __cdecl DObjSetRotTransIndex(DObj_s *obj, const int *partBits, int boneIndex)
{
    DSkel *skel; // [esp+0h] [ebp-Ch]
    int boneIndexHigh; // [esp+4h] [ebp-8h]
    unsigned int boneIndexLow; // [esp+8h] [ebp-4h]

    iassert(obj);
    iassert(obj->skel.mat);
    iassert(boneIndex >= 0);
    iassert(boneIndex < obj->numBones);

    boneIndexHigh = boneIndex >> 5;
    boneIndexLow = 0x80000000 >> (boneIndex & 0x1F);
    if ((boneIndexLow & partBits[boneIndex >> 5]) == 0)
        return 0;
    skel = &obj->skel;

    if ((boneIndexLow & obj->skel.partBits.anim[boneIndexHigh]) != 0)
        return 0;

    iassert(!(skel->partBits.skel[boneIndexHigh] & boneIndexLow));
    skel->partBits.anim[boneIndexHigh] |= boneIndexLow;
    return 1;
}

char __cdecl DObjSetSkelRotTransIndex(DObj_s *obj, const int *partBits, int boneIndex)
{
    DSkel *skel; // [esp+0h] [ebp-Ch]
    int boneIndexHigh; // [esp+4h] [ebp-8h]
    unsigned int boneIndexLow; // [esp+8h] [ebp-4h]

    iassert(obj);
    iassert(obj->skel.mat);
    iassert(boneIndex >= 0);
    iassert(boneIndex < obj->numBones);

    boneIndexHigh = boneIndex >> 5;
    boneIndexLow = 0x80000000 >> (boneIndex & 0x1F);

    if ((boneIndexLow & partBits[boneIndex >> 5]) == 0)
        return true;
    skel = &obj->skel;

    if ((boneIndexLow & obj->skel.partBits.anim[boneIndexHigh]) != 0)
        return false;

    iassert(!(skel->partBits.skel[boneIndexHigh] & boneIndexLow));
    skel->partBits.anim[boneIndexHigh] |= boneIndexLow;
    skel->partBits.skel[boneIndexHigh] |= boneIndexLow;

    return true;
}

void __cdecl DObjSetControlTagAngles(DObj_s *obj, int *partBits, unsigned int boneIndex, float *angles)
{
    if (boneIndex < 254)
    {
        if (DObjSetControlRotTransIndex(obj, partBits, boneIndex))
            DObjSetLocalTagInternal(obj, vec3_origin, angles, boneIndex);

        return;
    }

    iassert((boneIndex == 254) || (boneIndex == 255));
}

XModel *__cdecl DObjGetModel(const DObj_s *obj, int modelIndex)
{
    iassert(obj);
    iassert(modelIndex < obj->numModels);

    // LWSS: blops has this for some reason, seems new
    //if ( modelIndex < 0 || modelIndex >= obj->numModels )
    //    return 0;

    return obj->models[modelIndex];
}

void __cdecl DObjSetLocalTag(
    DObj_s *obj,
    int *partBits,
    unsigned int boneIndex,
    const float *trans,
    const float *angles)
{
    if (boneIndex < 254)
    {
        if (DObjSetRotTransIndex(obj, partBits, boneIndex))
            DObjSetLocalTagInternal(obj, trans, angles, boneIndex);

        return;
    }

    iassert((boneIndex == 254) || (boneIndex == 255));
}

void __cdecl DObjSetLocalTagInternal(const DObj_s *obj, const float *trans, const float *angles, int boneIndex)
{
    DObjAnimMat *rotTrans; // [esp+0h] [ebp-4h]
    DObjAnimMat *rotTransa; // [esp+0h] [ebp-4h]

    rotTrans = DObjGetRotTransArray(obj);
    if (rotTrans)
    {
        rotTransa = &rotTrans[boneIndex];
        if (angles)
            DObjSetAngles(rotTransa, angles);
        else
            DObjClearAngles(rotTransa);
        DObjSetTrans(rotTransa, trans);
    }
}

void __cdecl DObjSetAngles(DObjAnimMat *rotTrans, const float *angles)
{
    float v2; // [esp+8h] [ebp-44h]
    float v3; // [esp+14h] [ebp-38h]
    float v4; // [esp+20h] [ebp-2Ch]
    float yawQuat; // [esp+24h] [ebp-28h]
    float yawQuat_4; // [esp+28h] [ebp-24h]
    float rollQuat; // [esp+2Ch] [ebp-20h]
    float rollQuat_4; // [esp+30h] [ebp-1Ch]
    float pitchQuat; // [esp+34h] [ebp-18h]
    float pitchQuat_4; // [esp+38h] [ebp-14h]
    float tempQuat; // [esp+3Ch] [ebp-10h]
    float tempQuat_4; // [esp+40h] [ebp-Ch]
    float tempQuat_8; // [esp+44h] [ebp-8h]
    float tempQuat_12; // [esp+48h] [ebp-4h]

    v4 = angles[1] * 0.008726646192371845;
    yawQuat_4 = cos(v4);
    yawQuat = sin(v4);
    v3 = *angles * 0.008726646192371845;
    pitchQuat_4 = cos(v3);
    pitchQuat = sin(v3);
    v2 = angles[2] * 0.008726646192371845;
    rollQuat_4 = cos(v2);
    rollQuat = sin(v2);
    tempQuat = -pitchQuat * yawQuat;
    tempQuat_4 = pitchQuat * yawQuat_4;
    tempQuat_8 = pitchQuat_4 * yawQuat;
    tempQuat_12 = pitchQuat_4 * yawQuat_4;
    rotTrans->quat[0] = rollQuat * tempQuat_12 + rollQuat_4 * tempQuat;
    rotTrans->quat[1] = rollQuat_4 * tempQuat_4 + rollQuat * tempQuat_8;
    rotTrans->quat[2] = -rollQuat * tempQuat_4 + rollQuat_4 * tempQuat_8;
    rotTrans->quat[3] = rollQuat_4 * tempQuat_12 - rollQuat * tempQuat;
}

void __cdecl DObjClearAngles(DObjAnimMat *rotTrans)
{
    rotTrans->quat[0] = 0.0f;
    rotTrans->quat[1] = 0.0f;
    rotTrans->quat[2] = 0.0f;
    rotTrans->quat[3] = 1.0f;
}

void __cdecl DObjSetTrans(DObjAnimMat *rotTrans, const float *trans)
{
    rotTrans->transWeight = 0.0;
    rotTrans->trans[0] = trans[0];
    rotTrans->trans[1] = trans[1];
    rotTrans->trans[2] = trans[2];
}

void __cdecl DObjCompleteHierarchyBits(const DObj_s *obj, int *partBits)
{
    int j; // [esp+38h] [ebp-B4h]
    const unsigned __int8 *pos; // [esp+3Ch] [ebp-B0h]
    int newBoneIndex; // [esp+40h] [ebp-ACh]
    int newBoneIndexa; // [esp+40h] [ebp-ACh]
    int newBoneIndexb; // [esp+40h] [ebp-ACh]
    const unsigned __int8 *modelParents; // [esp+44h] [ebp-A8h]
    const unsigned __int8 *duplicateParts; // [esp+48h] [ebp-A4h]
    int numModels; // [esp+4Ch] [ebp-A0h]
    XModel *subModel; // [esp+50h] [ebp-9Ch]
    int startIndex[33]; // [esp+54h] [ebp-98h]
    int localBoneIndex; // [esp+D8h] [ebp-14h]
    unsigned __int8 *parentList; // [esp+DCh] [ebp-10h]
    int objBoneIndex; // [esp+E0h] [ebp-Ch]
    const int *duplicatePartBits; // [esp+E4h] [ebp-8h]
    XModel **models; // [esp+E8h] [ebp-4h]

    PROF_SCOPED("DObjCompleteHierarchyBits");

    iassert(obj);
    iassert(obj->numBones > 0);
    objBoneIndex = obj->numBones - 1;
    numModels = obj->numModels;
    iassert(numModels > 0);
    iassert(obj->duplicateParts);

    duplicatePartBits = (const int *)SL_ConvertToString(obj->duplicateParts);
    duplicateParts = (const unsigned __int8 *)(duplicatePartBits + 4);
    newBoneIndex = 0;
    subModel = 0;
    models = obj->models;
    modelParents = (const unsigned __int8 *)&models[numModels];
    for (j = 0; j < numModels; ++j)
    {
        startIndex[j] = newBoneIndex;
        subModel = models[j];
        newBoneIndex = startIndex[j] + subModel->numBones;
        if (newBoneIndex > objBoneIndex)
            break;
    }
    iassert(j != numModels);
    partBits[0] |= 0x80000000;
    for (parentList = subModel->parentList; ; parentList = subModel->parentList)
    {
        while (1)
        {
            localBoneIndex = objBoneIndex - startIndex[j];
            if (localBoneIndex < 0)
                break;
            if ((partBits[objBoneIndex >> 5] & (0x80000000 >> (objBoneIndex & 0x1F))) != 0)
            {
                if ((duplicatePartBits[objBoneIndex >> 5] & (0x80000000 >> (objBoneIndex & 0x1F))) != 0)
                {
                    for (pos = duplicateParts; ; pos += 2)
                    {
                        iassert(*pos);
                        if (objBoneIndex == *pos - 1)
                            break;
                    }
                    newBoneIndexb = pos[1] - 1;
                    goto LABEL_34;
                }
                newBoneIndexa = localBoneIndex - subModel->numRootBones;
                if (newBoneIndexa >= 0)
                {
                    newBoneIndexb = objBoneIndex - parentList[newBoneIndexa];
                    goto LABEL_34;
                }
                newBoneIndexb = modelParents[j];
                if (newBoneIndexb == 255)
                {
                    --objBoneIndex;
                }
                else
                {
                LABEL_34:
                    iassert((unsigned)newBoneIndexb < obj->numBones);
                    partBits[newBoneIndexb >> 5] |= 0x80000000 >> (newBoneIndexb & 0x1F);
                    --objBoneIndex;
                }
            }
            else
            {
                --objBoneIndex;
            }
        }
        if (--j < 0)
            break;
        subModel = models[j];
    }
}

int __cdecl DObjSetControlRotTransIndex(DObj_s *obj, const int *partBits, int boneIndex)
{
    DSkel *skel; // [esp+0h] [ebp-Ch]
    int boneIndexHigh; // [esp+4h] [ebp-8h]
    unsigned int boneIndexLow; // [esp+8h] [ebp-4h]

    iassert(obj);
    iassert(obj->skel.mat);
    iassert(boneIndex >= 0);
    iassert(boneIndex < obj->numBones);

    boneIndexHigh = boneIndex >> 5;
    boneIndexLow = 0x80000000 >> (boneIndex & 0x1F);

    if ((boneIndexLow & partBits[boneIndex >> 5]) == 0)
        return 0;

    skel = &obj->skel;

    if ((boneIndexLow & obj->skel.partBits.anim[boneIndexHigh]) != 0)
        return 0;
    
    iassert(!(skel->partBits.skel[boneIndexHigh] & boneIndexLow));

    skel->partBits.control[boneIndexHigh] |= boneIndexLow;
    skel->partBits.anim[boneIndexHigh] |= boneIndexLow;
    return 1;
}

bool __cdecl DObjSkelExists(const DObj_s *obj, int timeStamp)
{
    if (obj->skel.timeStamp == timeStamp)
        return obj->skel.mat != 0;
    memset((unsigned __int8 *)&obj->skel, 0, sizeof(obj->skel));
    return 0;
}

void __cdecl DObjClearSkel(const DObj_s *obj)
{
    memset((unsigned __int8 *)&obj->skel.partBits, 0, sizeof(DSkelPartBits));
}

int __cdecl DObjSkelAreBonesUpToDate(const DObj_s *obj, int *partBits)
{
    int i; // [esp+4h] [ebp-4h]

    iassert(obj);

    for (i = 0; i < 4; ++i)
    {
        if ((partBits[i] & ~obj->skel.partBits.skel[i]) != 0)
            return 0;
    }
    return 1;
}

int __cdecl DObjGetAllocSkelSize(const DObj_s *obj)
{
    return 32 * obj->numBones;
}

void __cdecl DObjCreateSkel(DObj_s *obj, char *buf, int timeStamp)
{
    unsigned int AllocSkelSize; // eax
    int i; // [esp+30h] [ebp-4h]

    PROF_SCOPED("DObjCreateSkel");

    AllocSkelSize = DObjGetAllocSkelSize(obj);
    memset((unsigned __int8 *)buf, 0xFFu, AllocSkelSize); // KISAKTODO: this memset is removed in blops, might not be needed

    obj->skel.mat = (DObjAnimMat *)buf;
    obj->skel.timeStamp = timeStamp;

    for (i = 0; i < 4; ++i)
    {
        iassert(!obj->skel.partBits.anim[i]);
        iassert(!obj->skel.partBits.control[i]);
        iassert(!obj->skel.partBits.skel[i]);
    }
}

DObjAnimMat *__cdecl I_dmaGetDObjSkel(const DObj_s *obj)
{
    iassert(obj->skel.mat);
    return obj->skel.mat;
}

void __cdecl DObjGetHidePartBits(const DObj_s *obj, unsigned int *partBits)
{
    partBits[0] = obj->hidePartBits[0];
    partBits[1] = obj->hidePartBits[1];
    partBits[2] = obj->hidePartBits[2];
    partBits[3] = obj->hidePartBits[3];
}

void __cdecl DObjLock(DObj_s *obj)
{
    volatile unsigned int *Destination; // [esp+0h] [ebp-4h]

    Destination = &obj->locked;
    do
    {
        while (*Destination)
            ;
    } while (InterlockedCompareExchange(Destination, 1, 0));
}

void __cdecl DObjUnlock(DObj_s *obj)
{
    iassert(obj->locked);

    obj->locked = 0;
}

