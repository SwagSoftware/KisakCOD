#include "dobj.h"
#include <universal/profile.h>
#include "xanim.h"
#include "xanim_calc.h"

void __cdecl DObjCalcSkel(const DObj_s *obj, int *partBits)
{
    const unsigned __int8 *pos; // [esp+54h] [ebp-64h]
    int j; // [esp+58h] [ebp-60h]
    const unsigned __int8 *modelParents; // [esp+5Ch] [ebp-5Ch]
    const unsigned __int8 *duplicateParts; // [esp+60h] [ebp-58h]
    unsigned int boneIndex; // [esp+64h] [ebp-54h]
    unsigned int boneIndexa; // [esp+64h] [ebp-54h]
    int controlPartBits[4]; // [esp+68h] [ebp-50h] BYREF
    int numModels; // [esp+78h] [ebp-40h]
    XModel *model; // [esp+7Ch] [ebp-3Ch]
    unsigned int modelParent; // [esp+80h] [ebp-38h]
    DSkel *skel; // [esp+84h] [ebp-34h]
    int calcPartBits[4]; // [esp+88h] [ebp-30h] BYREF
    int ignorePartBits[4]; // [esp+98h] [ebp-20h] BYREF
    int i; // [esp+A8h] [ebp-10h]
    XModel **models; // [esp+ACh] [ebp-Ch]
    bool bFinished; // [esp+B3h] [ebp-5h]
    const int *savedDuplicatePartBits; // [esp+B4h] [ebp-4h]
    int savedregs; // [esp+B8h] [ebp+0h] BYREF

    Profile_Begin(310);
    iassert(obj);
    skel = (DSkel *)&obj->skel;
    iassert(skel);
    bFinished = 1;
    for (i = 0; i < 4; ++i)
    {
        ignorePartBits[i] = skel->partBits.skel[i] | ~partBits[i];
        if (ignorePartBits[i] != -1)
            bFinished = 0;
    }

    if (bFinished)
    {
        Profile_EndInternal(0);
        return;
    }

    DObjCalcAnim(obj, partBits);
    iassert(obj->duplicateParts);
    savedDuplicatePartBits = (const int *)SL_ConvertToString(obj->duplicateParts);
    duplicateParts = (const unsigned __int8 *)(savedDuplicatePartBits + 4);
    GetControlAndDuplicatePartBits(obj, partBits, ignorePartBits, savedDuplicatePartBits, calcPartBits, controlPartBits);
    numModels = obj->numModels;
    boneIndex = 0;
    pos = duplicateParts;
    models = obj->models;
    modelParents = (const unsigned __int8 *)&models[numModels];
    for (j = 0; j < numModels; ++j)
    {
        model = models[j];
        modelParent = modelParents[j];
        pos = CalcSkelDuplicateBones(model, skel, boneIndex, pos);

        if (modelParent == 255)
            CalcSkelRootBonesNoParentOrDuplicate(model, skel, boneIndex, calcPartBits);
        else
            CalcSkelRootBonesWithParent(model, skel, boneIndex, modelParent, calcPartBits, controlPartBits);

        CalcSkelNonRootBones(model, skel, boneIndex + model->numRootBones, calcPartBits, controlPartBits);
        boneIndex += model->numBones;
    }

    iassert(!(*pos));
    for (boneIndexa = 0; boneIndexa < obj->numBones; ++boneIndexa)
    {
        //if ((skel->partBits.anim[boneIndexa >> 5] & (0x80000000 >> (boneIndexa & 0x1F))) != 0)
        if (skel->partBits.anim.testBit(boneIndexa))
        {
            iassert(!IS_NAN(skel->mat[boneIndexa].quat[0]) 
                && !IS_NAN(skel->mat[boneIndexa].quat[1]) 
                && !IS_NAN(skel->mat[boneIndexa].quat[2]) 
                && !IS_NAN(skel->mat[boneIndexa].quat[3])
            );
            iassert(!IS_NAN(skel->mat[boneIndexa].trans[0]) && !IS_NAN(skel->mat[boneIndexa].trans[1]) && !IS_NAN(skel->mat[boneIndexa].trans[2]));
        }
    }
    Profile_EndInternal(0);
}

void __cdecl GetControlAndDuplicatePartBits(
    const DObj_s *obj,
    const int *partBits,
    const int *ignorePartBits,
    const int *savedDuplicatePartBits,
    int *calcPartBits,
    int *controlPartBits)
{
    const char *v6; // eax
    int boneIndex; // [esp+4h] [ebp-14h]
    DSkel *skel; // [esp+8h] [ebp-10h]
    int i; // [esp+10h] [ebp-8h]
    unsigned int boneIndexLow; // [esp+14h] [ebp-4h]

    skel = (DSkel *)&obj->skel;
    if (obj == (const DObj_s *)-20)
        MyAssertHandler(".\\xanim\\dobj_skel.cpp", 86, 0, "%s", "skel");
    for (i = 0; i < 4; ++i)
    {
        skel->partBits.skel[i] |= partBits[i];
        controlPartBits[i] = skel->partBits.control[i];
        calcPartBits[i] = ~(savedDuplicatePartBits[i] | ignorePartBits[i]);
        if ((savedDuplicatePartBits[i] & controlPartBits[i]) != 0)
        {
            DObjDumpInfo(obj);
            for (boneIndex = 0; boneIndex < obj->numBones; ++boneIndex)
            {
                boneIndexLow = 0x80000000 >> (boneIndex & 0x1F);
                if ((boneIndexLow & controlPartBits[boneIndex >> 5]) != 0
                    && (boneIndexLow & savedDuplicatePartBits[boneIndex >> 5]) != 0)
                {
                    break;
                }
            }
            if (!alwaysfails)
            {
                v6 = va("control/meld conflict on bone %d - see the console log for details", boneIndex);
                MyAssertHandler(".\\xanim\\dobj_skel.cpp", 107, 0, v6);
            }
        }
    }
}

const unsigned __int8 *__cdecl CalcSkelDuplicateBones(
    const XModel *model,
    DSkel *skel,
    int minBoneIndex,
    const unsigned __int8 *pos)
{
    int boneIndex; // [esp+8h] [ebp-10h]
    DObjAnimMat *mat; // [esp+Ch] [ebp-Ch]
    int parentIndex; // [esp+10h] [ebp-8h]
    unsigned int maxBoneIndex; // [esp+14h] [ebp-4h]

    mat = skel->mat;
    maxBoneIndex = minBoneIndex + model->numBones;
    while (1)
    {
        boneIndex = pos[0] - 1;
        if (boneIndex >= maxBoneIndex)
            break;
        parentIndex = pos[1] - 1;
        iassert(parentIndex < boneIndex);
        memcpy(&mat[boneIndex], &mat[parentIndex], sizeof(DObjAnimMat));
        pos += 2;
    }
    return pos;
}

void __cdecl CalcSkelRootBonesNoParentOrDuplicate(
    const XModel *model,
    DSkel *skel,
    int minBoneIndex,
    int *calcPartBits)
{
    DWORD v5; // eax
    int v6; // [esp+0h] [ebp-50h]
    float *v; // [esp+20h] [ebp-30h]
    float v8; // [esp+24h] [ebp-2Ch]
    int boneIndex; // [esp+2Ch] [ebp-24h]
    int maxBoneIndexHigh; // [esp+30h] [ebp-20h]
    unsigned int bits; // [esp+38h] [ebp-18h]
    unsigned int boneBit;
    DObjAnimMat *mat; // [esp+3Ch] [ebp-14h]
    int boneIndexHigh; // [esp+40h] [ebp-10h]
    int boneIndexLow; // [esp+44h] [ebp-Ch]
    int maxBoneIndexa; // [esp+4Ch] [ebp-4h]
    int maxBoneIndex; // [esp+4Ch] [ebp-4h]

    maxBoneIndexa = minBoneIndex + model->numRootBones;
    boneIndexHigh = minBoneIndex >> 5;
    maxBoneIndexHigh = (maxBoneIndexa - 1) >> 5;
    maxBoneIndex = maxBoneIndexa - 32 * (minBoneIndex >> 5);
    mat = skel->mat;
    while (boneIndexHigh <= maxBoneIndexHigh)
    {
        bits = calcPartBits[boneIndexHigh];

        if (maxBoneIndex > 32)
            v6 = 32;
        else
            v6 = maxBoneIndex;

        while (1)
        {
            if (!_BitScanReverse(&v5, bits))
                v5 = 63; // `CountLeadingZeros'::`2': : notFound;
            boneIndexLow = v5 ^ 0x1F;
            if ((v5 ^ 0x1F) >= v6)
                break;
            boneIndex = boneIndexLow + 32 * boneIndexHigh;
            boneBit = (0x80000000 >> boneIndexLow);
            iassert(bits & boneBit);
            bits &= ~(boneBit);
            calcPartBits[boneIndexHigh] = bits;
            v = mat[boneIndex].quat;
            v8 = Vec4LengthSq(v);
            if (v8 == 0.0f)
            {
                v[3] = 1.0f;
                v[7] = 2.0f;
            }
            else
            {
                v[7] = 2.0f / v8;
            }
            iassert(!IS_NAN(mat[boneIndex].quat[0]) && !IS_NAN(mat[boneIndex].quat[1]) && !IS_NAN(mat[boneIndex].quat[2]) && !IS_NAN(mat[boneIndex].quat[3]));
            iassert(!IS_NAN(mat[boneIndex].trans[0]) && !IS_NAN(mat[boneIndex].trans[1]) && !IS_NAN(mat[boneIndex].trans[2]));
        }
        ++boneIndexHigh;
        maxBoneIndex -= 32;
    }
}

void __cdecl CalcSkelRootBonesWithParent(
    const XModel *model,
    DSkel *skel,
    unsigned int minBoneIndex,
    unsigned int modelParent,
    int *calcPartBits,
    const int *controlPartBits)
{
    DWORD v7; // eax
    float *trans; // [esp+18h] [ebp-F4h]
    float result[3]; // [esp+40h] [ebp-CCh] BYREF
    const DObjAnimMat *parentMat; // [esp+D0h] [ebp-3Ch]
    DObjAnimMat *childMat; // [esp+D4h] [ebp-38h]
    unsigned int boneIndex; // [esp+D8h] [ebp-34h]
    unsigned int maxBoneIndexHigh; // [esp+DCh] [ebp-30h]
    unsigned int maxBoneIndexLow; // [esp+E0h] [ebp-2Ch]
    float quat[4]; // [esp+E4h] [ebp-28h]
    int bits; // [esp+F4h] [ebp-18h]
    const DObjAnimMat *mat; // [esp+F8h] [ebp-14h]
    unsigned int boneIndexHigh; // [esp+FCh] [ebp-10h]
    unsigned int boneIndexLow; // [esp+100h] [ebp-Ch]
    int boneBit; // [esp+104h] [ebp-8h]
    unsigned int maxBoneIndex; // [esp+108h] [ebp-4h]

    maxBoneIndex = minBoneIndex + model->numRootBones;
    boneIndexHigh = minBoneIndex >> 5;
    maxBoneIndexHigh = (maxBoneIndex - 1) >> 5;
    maxBoneIndex -= 32 * (minBoneIndex >> 5);
    mat = skel->mat;
    parentMat = &mat[modelParent];
    while (boneIndexHigh <= maxBoneIndexHigh)
    {
        bits = calcPartBits[boneIndexHigh];
        if (maxBoneIndex > 32)
            maxBoneIndexLow = 32;
        else
            maxBoneIndexLow = maxBoneIndex;
        while (1)
        {
            if (!_BitScanReverse(&v7, bits))
                v7 = 63;
            boneIndexLow = v7 ^ 0x1F;
            if ((v7 ^ 0x1Fu) >= maxBoneIndexLow)
                break;
            boneIndex = boneIndexLow + 32 * boneIndexHigh;
            boneBit = 0x80000000 >> boneIndexLow;
            iassert(bits & boneBit);
            bits &= ~boneBit;
            calcPartBits[boneIndexHigh] = bits;
            iassert(modelParent < boneIndex);
            iassert(skel->partBits.anim[boneIndexHigh] & boneBit);
            iassert(skel->partBits.skel[modelParent >> 5] & (HIGH_BIT >> (modelParent & 31)));
            iassert(skel->partBits.anim[modelParent >> 5] & (HIGH_BIT >> (modelParent & 31)));
            childMat = &skel->mat[boneIndex];
            if ((boneBit & controlPartBits[boneIndexHigh]) != 0)
            {
                iassert(skel->partBits.skel[0] & HIGH_BIT);
                iassert(skel->partBits.anim[0] & HIGH_BIT);
                QuatMultiplyReverseInverse(mat->quat, parentMat->quat, quat);
                QuatMultiplyReverseEquals(quat, childMat->quat);
                QuatMultiplyEquals(mat->quat, childMat->quat);
            }
            else
            {
                QuatMultiplyEquals(parentMat->quat, childMat->quat);
            }
            iassert(!IS_NAN(childMat->quat[0]) && !IS_NAN(childMat->quat[1]) && !IS_NAN(childMat->quat[2]) && !IS_NAN(childMat->quat[3]));
            iassert(!IS_NAN(childMat->trans[0]) && !IS_NAN(childMat->trans[1]) && !IS_NAN(childMat->trans[2]));

            if (Vec4LengthSq(childMat->quat) == 0.0f)
            {
                childMat->quat[3] = 1.0f;
                childMat->transWeight = 2.0f;
            }
            else
            {
                childMat->transWeight = 2.0f / Vec4LengthSq(childMat->quat);
            }

            MatrixTransformVectorQuatTransEquals(parentMat, childMat->trans);

            iassert(!IS_NAN(childMat->trans[0]) && !IS_NAN(childMat->trans[1]) && !IS_NAN(childMat->trans[2]));
        }
        ++boneIndexHigh;
        maxBoneIndex -= 32;
    }
}

void __cdecl CalcSkelNonRootBones(
    const XModel *model,
    DSkel *skel,
    int minBoneIndex,
    int *calcPartBits,
    const int *controlPartBits)
{
    DWORD v6; // eax
    float result[3]; // [esp+40h] [ebp-ECh] BYREF
    DObjAnimMat *childMat; // [esp+E0h] [ebp-4Ch]
    const DObjAnimMat *parentMat; // [esp+E4h] [ebp-48h]
    int boneIndex; // [esp+E8h] [ebp-44h]
    int maxBoneIndexHigh; // [esp+ECh] [ebp-40h]
    int maxBoneIndexLow; // [esp+F0h] [ebp-3Ch]
    const float *trans; // [esp+F4h] [ebp-38h]
    float quat[4]; // [esp+F8h] [ebp-34h]
    int bits; // [esp+108h] [ebp-24h]
    DObjAnimMat *mat; // [esp+10Ch] [ebp-20h]
    int boneOffset; // [esp+110h] [ebp-1Ch]
    const unsigned __int8 *parentList; // [esp+114h] [ebp-18h]
    int parentOffset; // [esp+118h] [ebp-14h]
    int boneIndexHigh; // [esp+11Ch] [ebp-10h]
    int boneIndexLow; // [esp+120h] [ebp-Ch]
    int boneBit; // [esp+124h] [ebp-8h]
    int maxBoneIndex; // [esp+128h] [ebp-4h]

    maxBoneIndex = minBoneIndex + model->numBones - model->numRootBones;
    boneIndexHigh = minBoneIndex >> 5;
    maxBoneIndexHigh = (maxBoneIndex - 1) >> 5;
    maxBoneIndex -= (32 * (minBoneIndex >> 5));
    mat = skel->mat;
    while (boneIndexHigh <= maxBoneIndexHigh)
    {
        bits = calcPartBits[boneIndexHigh];

        if (maxBoneIndex > 32)
            maxBoneIndexLow = 32;
        else
            maxBoneIndexLow = maxBoneIndex;

        while (1)
        {
            if (!_BitScanReverse(&v6, bits))
                v6 = 63;
            boneIndexLow = v6 ^ 0x1F;
            if ((v6 ^ 0x1F) >= maxBoneIndexLow)
                break;
            boneIndex = boneIndexLow + 32 * boneIndexHigh;
            boneBit = 0x80000000 >> boneIndexLow;
            iassert(bits & boneBit);
            bits &= ~boneBit;
            calcPartBits[boneIndexHigh] = bits;
            childMat = &mat[boneIndex];
            boneOffset = boneIndex - minBoneIndex;
            bcassert(boneOffset, (model->numBones - model->numRootBones));
            parentList = &model->parentList[boneOffset];
            parentOffset = *parentList;
            parentMat = &childMat[-parentOffset];
            iassert(skel->partBits.anim[boneIndexHigh] & boneBit);
            iassert(skel->partBits.skel[(boneIndex - parentOffset) >> 5] & (HIGH_BIT >> ((boneIndex - parentOffset) & 31)));
            iassert(skel->partBits.anim[(boneIndex - parentOffset) >> 5] & (HIGH_BIT >> ((boneIndex - parentOffset) & 31)));
            if ((boneBit & controlPartBits[boneIndexHigh]) != 0)
            {
                iassert(skel->partBits.skel[0] & HIGH_BIT);
                iassert(skel->partBits.anim[0] & HIGH_BIT);
                QuatMultiplyReverseInverse(mat->quat, parentMat->quat, quat);
                QuatMultiplyReverseEquals(quat, childMat->quat);
                QuatMultiplyEquals(mat->quat, childMat->quat);
            }
            else
            {
                QuatMultiplyEquals(parentMat->quat, childMat->quat);
            }
            iassert(!IS_NAN(childMat->quat[0]) && !IS_NAN(childMat->quat[1]) && !IS_NAN(childMat->quat[2]) && !IS_NAN(childMat->quat[3]));
            iassert(!IS_NAN((childMat->trans)[0]) && !IS_NAN((childMat->trans)[1]) && !IS_NAN((childMat->trans)[2]));

            if (Vec4LengthSq(childMat->quat) == 0.0f)
            {
                childMat->quat[3] = 1.0f;
                childMat->transWeight = 2.0f;
            }
            else
            {
                childMat->transWeight = 2.0f / Vec4LengthSq(childMat->quat);
            }

            trans = &model->trans[3 * boneOffset];
            iassert(!IS_NAN(trans[0]) && !IS_NAN(trans[1]) && !IS_NAN(trans[2]));
            Vec3Add(childMat->trans, trans, childMat->trans);
            MatrixTransformVectorQuatTransEquals(parentMat, childMat->trans);
            iassert(!IS_NAN(childMat->trans[0]) && !IS_NAN(childMat->trans[1]) && !IS_NAN(childMat->trans[2]));
        }
        ++boneIndexHigh;
        maxBoneIndex -= 32;
    }
}

void __cdecl DObjCalcBaseSkel(const DObj_s *obj, DObjAnimMat *mat, int *partBits)
{
    const unsigned __int8 *pos; // [esp+8h] [ebp-94h]
    int j; // [esp+Ch] [ebp-90h]
    const unsigned __int8 *modelParents; // [esp+10h] [ebp-8Ch]
    const unsigned __int8 *duplicateParts; // [esp+14h] [ebp-88h]
    unsigned int boneIndex; // [esp+18h] [ebp-84h]
    int controlPartBits[4]; // [esp+1Ch] [ebp-80h] BYREF
    int numModels; // [esp+2Ch] [ebp-70h]
    XModel *model; // [esp+30h] [ebp-6Ch]
    unsigned int modelParent; // [esp+34h] [ebp-68h]
    DSkel skel; // [esp+38h] [ebp-64h] BYREF
    int calcPartBits[4]; // [esp+70h] [ebp-2Ch] BYREF
    int ignorePartBits[4]; // [esp+80h] [ebp-1Ch] BYREF
    int i; // [esp+90h] [ebp-Ch]
    XModel **models; // [esp+94h] [ebp-8h]
    const int *savedDuplicatePartBits; // [esp+98h] [ebp-4h]

    if (!obj)
        MyAssertHandler(".\\xanim\\dobj_skel.cpp", 531, 0, "%s", "obj");
    if (!mat)
        MyAssertHandler(".\\xanim\\dobj_skel.cpp", 532, 0, "%s", "mat");
    skel.mat = mat;
    skel.timeStamp = 0;
    for (i = 0; i < 4; ++i)
    {
        skel.partBits.skel[i] = partBits[i];
        skel.partBits.anim[i] = skel.partBits.skel[i];
        skel.partBits.control[i] = 0;
        ignorePartBits[i] = ~partBits[i];
    }
    DObjCalcBaseAnim(obj, mat, partBits);
    if (!obj->duplicateParts)
        MyAssertHandler(".\\xanim\\dobj_skel.cpp", 546, 0, "%s", "obj->duplicateParts");
    savedDuplicatePartBits = (const int *)SL_ConvertToString(obj->duplicateParts);
    duplicateParts = (const unsigned __int8 *)(savedDuplicatePartBits + 4);
    DObjGetBaseControlAndDuplicatePartBits(
        obj,
        partBits,
        ignorePartBits,
        savedDuplicatePartBits,
        calcPartBits,
        controlPartBits);
    numModels = obj->numModels;
    boneIndex = 0;
    pos = duplicateParts;
    models = obj->models;
    modelParents = (const unsigned __int8 *)&models[numModels];
    for (j = 0; j < numModels; ++j)
    {
        model = models[j];
        modelParent = modelParents[j];
        pos = CalcSkelDuplicateBones(model, &skel, boneIndex, pos);
        if (modelParent == 255)
            CalcSkelRootBonesNoParentOrDuplicate(model, &skel, boneIndex, calcPartBits);
        else
            CalcSkelRootBonesWithParent(model, &skel, boneIndex, modelParent, calcPartBits, controlPartBits);
        CalcSkelNonRootBones(model, &skel, boneIndex + model->numRootBones, calcPartBits, controlPartBits);
        boneIndex += model->numBones;
    }
}

void __cdecl DObjCalcBaseAnim(const DObj_s *obj, DObjAnimMat *mat, int *partBits)
{
    int j; // [esp+18h] [ebp-14h]
    int boneIndex; // [esp+1Ch] [ebp-10h]
    XModel *model; // [esp+20h] [ebp-Ch]
    __int16 *quats; // [esp+24h] [ebp-8h]
    int i; // [esp+28h] [ebp-4h]
    int ia; // [esp+28h] [ebp-4h]

    if (!obj)
        MyAssertHandler(".\\xanim\\dobj_skel.cpp", 459, 0, "%s", "obj");
    if (!mat)
        MyAssertHandler(".\\xanim\\dobj_skel.cpp", 460, 0, "%s", "mat");
    boneIndex = 0;
    for (j = 0; j < obj->numModels; ++j)
    {
        model = obj->models[j];
        for (i = model->numRootBones; i; --i)
        {
            mat->quat[0] = 0.0;
            mat->quat[1] = 0.0;
            mat->quat[2] = 0.0;
            mat->quat[3] = 1.0;
            mat->trans[0] = 0.0;
            mat->trans[1] = 0.0;
            mat->trans[2] = 0.0;
            ++mat;
            ++boneIndex;
        }
        quats = model->quats;
        ia = model->numBones - model->numRootBones;
        while (ia)
        {
            if ((partBits[boneIndex >> 5] & (0x80000000 >> (boneIndex & 0x1F))) != 0)
            {
                mat->quat[0] = (double)quats[0] * 0.00003051850944757462;
                mat->quat[1] = (double)quats[1] * 0.00003051850944757462;
                mat->quat[2] = (double)quats[2] * 0.00003051850944757462;
                mat->quat[3] = (double)quats[3] * 0.00003051850944757462;
                mat->trans[0] = 0.0;
                mat->trans[1] = 0.0;
                mat->trans[2] = 0.0;
            }
            --ia;
            ++mat;
            ++boneIndex;
            quats += 4;
        }
    }
}

void __cdecl DObjGetBaseControlAndDuplicatePartBits(
    const DObj_s *obj,
    const int *partBits,
    const int *ignorePartBits,
    const int *savedDuplicatePartBits,
    int *calcPartBits,
    int *controlPartBits)
{
    int i; // [esp+4h] [ebp-4h]

    for (i = 0; i < 4; ++i)
    {
        controlPartBits[i] = 0;
        calcPartBits[i] = ~(savedDuplicatePartBits[i] | ignorePartBits[i]);
    }
}

