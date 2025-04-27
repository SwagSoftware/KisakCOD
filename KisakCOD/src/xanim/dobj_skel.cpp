#include "dobj.h"
#include <universal/profile.h>
#include "xanim.h"
#include "xanim_calc.h"

void DObjCalcAnim(const DObj_s *obj, int *partBits)
{
    void *v3; // esp
    const char *v4; // eax
    const char *v5; // eax
    float v6; // [esp+38h] [ebp-60FCh]
    float v7; // [esp+38h] [ebp-60FCh]
    float v8; // [esp+3Ch] [ebp-60F8h]
    float v9; // [esp+3Ch] [ebp-60F8h]
    float v10; // [esp+40h] [ebp-60F4h]
    float v11; // [esp+40h] [ebp-60F4h]
    float v12; // [esp+44h] [ebp-60F0h]
    float v13; // [esp+44h] [ebp-60F0h]
    __int16 *quats; // [esp+68h] [ebp-60CCh]
    int mm; // [esp+90h] [ebp-60A4h]
    int v16; // [esp+90h] [ebp-60A4h]
    XModel *v17; // [esp+94h] [ebp-60A0h]
    int kk; // [esp+98h] [ebp-609Ch]
    XModel **models; // [esp+9Ch] [ebp-6098h]
    unsigned int v20; // [esp+A0h] [ebp-6094h]
    XAnimInfo *AnimInfo; // [esp+A4h] [ebp-6090h]
    XAnimTree_s *tree; // [esp+A8h] [ebp-608Ch]
    int jj; // [esp+ACh] [ebp-6088h]
    int ii; // [esp+B0h] [ebp-6084h]
    unsigned int n; // [esp+D0h] [ebp-6064h]
    DObjAnimMat *mat; // [esp+D4h] [ebp-6060h]
    float *quat; // [esp+D4h] [ebp-6060h]
    char v28; // [esp+DBh] [ebp-6059h]
    int m; // [esp+DCh] [ebp-6058h]
    int k; // [esp+E0h] [ebp-6054h]
    int j; // [esp+E4h] [ebp-6050h]
    XAnimCalcAnimInfo v32; // [esp+E8h] [ebp-604Ch] BYREF
    int i; // [esp+6114h] [ebp-20h]
    DSkel *v34; // [esp+6118h] [ebp-1Ch]
    int *v35; // [esp+611Ch] [ebp-18h]
    DSkel *p_skel; // [esp+6120h] [ebp-14h]
    int v37; // [esp+6128h] [ebp-Ch]
    void *v38; // [esp+612Ch] [ebp-8h]
    void *retaddr; // [esp+6134h] [ebp+0h]

    v38 = retaddr;
    v3 = alloca(24844);
    Profile_Begin(309);
    if (!obj)
        MyAssertHandler(".\\xanim\\xanim_calc.cpp", 1504, 0, "%s", "obj");
    p_skel = (DSkel *)&obj->skel;
    if (obj == (const DObj_s *)-20)
        MyAssertHandler(".\\xanim\\xanim_calc.cpp", 1507, 0, "%s", "skel");
    v35 = partBits;
    v34 = p_skel;
    for (i = 0; i < 4; ++i)
        v32.animPartBits.array[i] = v35[i];
    for (j = 0; j < 4; ++j)
        v32.animPartBits.array[j] = ~v32.animPartBits.array[j];
    for (k = 0; k < 4; ++k)
        v32.animPartBits.array[k] |= v34->partBits.anim[k];
    for (m = 0; m < 4; ++m)
    {
        if (v32.animPartBits.array[m] != -1)
        {
            v28 = 0;
            goto LABEL_20;
        }
    }
    v28 = 1;
LABEL_20:
    if (v28)
    {
        Profile_EndInternal(0);
    }
    else
    {
        mat = p_skel->mat;
        for (n = 0; (int)n < obj->numBones; ++n)
        {
            if (n >= 0x80)
                MyAssertHandler("c:\\trees\\cod3\\src\\xanim\\../qcommon/bitarray.h", 66, 0, "%s", "pos < BIT_COUNT");
            if ((v34->partBits.anim[n >> 5] & (0x80000000 >> (n & 0x1F))) != 0)
            {
                if ((COERCE_UNSIGNED_INT(mat[n].quat[0]) & 0x7F800000) == 0x7F800000
                    || (COERCE_UNSIGNED_INT(mat[n].quat[1]) & 0x7F800000) == 0x7F800000
                    || (COERCE_UNSIGNED_INT(mat[n].quat[2]) & 0x7F800000) == 0x7F800000
                    || (COERCE_UNSIGNED_INT(mat[n].quat[3]) & 0x7F800000) == 0x7F800000)
                {
                    MyAssertHandler(
                        ".\\xanim\\xanim_calc.cpp",
                        1532,
                        0,
                        "%s",
                        "!IS_NAN((mat[bone].quat)[0]) && !IS_NAN((mat[bone].quat)[1]) && !IS_NAN((mat[bone].quat)[2]) && !IS_NAN((mat"
                        "[bone].quat)[3])");
                }
                if ((COERCE_UNSIGNED_INT(mat[n].trans[0]) & 0x7F800000) == 0x7F800000
                    || (COERCE_UNSIGNED_INT(mat[n].trans[1]) & 0x7F800000) == 0x7F800000
                    || (COERCE_UNSIGNED_INT(mat[n].trans[2]) & 0x7F800000) == 0x7F800000)
                {
                    MyAssertHandler(
                        ".\\xanim\\xanim_calc.cpp",
                        1533,
                        0,
                        "%s",
                        "!IS_NAN((mat[bone].trans)[0]) && !IS_NAN((mat[bone].trans)[1]) && !IS_NAN((mat[bone].trans)[2])");
                }
            }
        }
        for (ii = 0; ii < 4; ++ii)
            v34->partBits.anim[ii] |= v35[ii];
        for (jj = 0; jj < 4; ++jj)
            v32.ignorePartBits.array[jj] = v32.animPartBits.array[jj];
        tree = obj->tree;
        if (obj->tree && tree->children)
        {
            InterlockedIncrement(&tree->calcRefCount);
            if (tree->modifyRefCount)
                MyAssertHandler(".\\xanim\\xanim_calc.cpp", 1553, 0, "%s", "!tree->modifyRefCount");
            //bitarray<128>::setBit(&v32.ignorePartBits, 0x7Fu);
            v32.ignorePartBits.setBit(0x7F);
            AnimInfo = GetAnimInfo(tree->children);
            XAnimCalc(obj, AnimInfo, 1.0, 1, 0, &v32, 0, p_skel->mat);
            if (tree->modifyRefCount)
                MyAssertHandler(".\\xanim\\xanim_calc.cpp", 1562, 0, "%s", "!tree->modifyRefCount");
            InterlockedDecrement(&tree->calcRefCount);
        }
        v20 = 0;
        quat = p_skel->mat->quat;
        models = obj->models;
        for (kk = 0; kk < obj->numModels; ++kk)
        {
            v17 = models[kk];
            for (mm = v17->numRootBones; mm; --mm)
            {
                if (v20 >= 0x80)
                    MyAssertHandler("c:\\trees\\cod3\\src\\xanim\\../qcommon/bitarray.h", 66, 0, "%s", "pos < BIT_COUNT");
                if ((v32.animPartBits.array[v20 >> 5] & (0x80000000 >> (v20 & 0x1F))) != 0)
                {
                    if (v20 >= 0x80)
                        MyAssertHandler("c:\\trees\\cod3\\src\\xanim\\../qcommon/bitarray.h", 66, 0, "%s", "pos < BIT_COUNT");
                    if ((v34->partBits.anim[v20 >> 5] & (0x80000000 >> (v20 & 0x1F))) != 0)
                    {
                        if ((int)v20 >= obj->numBones)
                        {
                            v4 = va("model %d: '%s'", kk, models[kk]->name);
                            MyAssertHandler(".\\xanim\\xanim_calc.cpp", 1587, 0, "%s\n\t%s", "boneIndex < obj->numBones", v4);
                        }
                        if ((COERCE_UNSIGNED_INT(*quat) & 0x7F800000) == 0x7F800000
                            || (COERCE_UNSIGNED_INT(quat[1]) & 0x7F800000) == 0x7F800000
                            || (COERCE_UNSIGNED_INT(quat[2]) & 0x7F800000) == 0x7F800000
                            || (COERCE_UNSIGNED_INT(quat[3]) & 0x7F800000) == 0x7F800000)
                        {
                            v5 = va("model %d: '%s'", kk, models[kk]->name);
                            MyAssertHandler(
                                ".\\xanim\\xanim_calc.cpp",
                                1588,
                                0,
                                "%s\n\t%s",
                                "(!IS_NAN((mat->quat)[0]) && !IS_NAN((mat->quat)[1]) && !IS_NAN((mat->quat)[2]) && !IS_NAN((mat->quat)[3]))",
                                v5);
                        }
                        if ((COERCE_UNSIGNED_INT(quat[4]) & 0x7F800000) == 0x7F800000
                            || (COERCE_UNSIGNED_INT(quat[5]) & 0x7F800000) == 0x7F800000
                            || (COERCE_UNSIGNED_INT(quat[6]) & 0x7F800000) == 0x7F800000)
                        {
                            MyAssertHandler(
                                ".\\xanim\\xanim_calc.cpp",
                                1589,
                                0,
                                "%s",
                                "!IS_NAN((mat->trans)[0]) && !IS_NAN((mat->trans)[1]) && !IS_NAN((mat->trans)[2])");
                        }
                    }
                }
                else
                {
                    *quat = 0.0;
                    quat[1] = 0.0;
                    quat[2] = 0.0;
                    quat[3] = 1.0;
                    quat[4] = 0.0;
                    quat[5] = 0.0;
                    quat[6] = 0.0;
                    quat[7] = 0.0;
                }
                quat += 8;
                ++v20;
            }
            quats = v17->quats;
            v16 = v17->numBones - v17->numRootBones;
            while (v16)
            {
                if (v20 >= 0x80)
                    MyAssertHandler("c:\\trees\\cod3\\src\\xanim\\../qcommon/bitarray.h", 66, 0, "%s", "pos < BIT_COUNT");
                if ((v32.animPartBits.array[v20 >> 5] & (0x80000000 >> (v20 & 0x1F))) != 0)
                {
                    if (v20 >= 0x80)
                        MyAssertHandler("c:\\trees\\cod3\\src\\xanim\\../qcommon/bitarray.h", 66, 0, "%s", "pos < BIT_COUNT");
                    if ((v34->partBits.anim[v20 >> 5] & (0x80000000 >> (v20 & 0x1F))) != 0)
                    {
                        if ((COERCE_UNSIGNED_INT(*quat) & 0x7F800000) == 0x7F800000
                            || (COERCE_UNSIGNED_INT(quat[1]) & 0x7F800000) == 0x7F800000
                            || (COERCE_UNSIGNED_INT(quat[2]) & 0x7F800000) == 0x7F800000
                            || (COERCE_UNSIGNED_INT(quat[3]) & 0x7F800000) == 0x7F800000)
                        {
                            MyAssertHandler(
                                ".\\xanim\\xanim_calc.cpp",
                                1614,
                                0,
                                "%s",
                                "!IS_NAN((mat->quat)[0]) && !IS_NAN((mat->quat)[1]) && !IS_NAN((mat->quat)[2]) && !IS_NAN((mat->quat)[3])");
                        }
                        if ((COERCE_UNSIGNED_INT(quat[4]) & 0x7F800000) == 0x7F800000
                            || (COERCE_UNSIGNED_INT(quat[5]) & 0x7F800000) == 0x7F800000
                            || (COERCE_UNSIGNED_INT(quat[6]) & 0x7F800000) == 0x7F800000)
                        {
                            MyAssertHandler(
                                ".\\xanim\\xanim_calc.cpp",
                                1615,
                                0,
                                "%s",
                                "!IS_NAN((mat->trans)[0]) && !IS_NAN((mat->trans)[1]) && !IS_NAN((mat->trans)[2])");
                        }
                    }
                }
                else
                {
                    v6 = (float)*quats;
                    v8 = (float)quats[1];
                    v10 = (float)quats[2];
                    v12 = (float)quats[3];
                    v7 = 0.000030518509 * v6;
                    v9 = 0.000030518509 * v8;
                    v11 = 0.000030518509 * v10;
                    v13 = 0.000030518509 * v12;
                    *quat = v7;
                    quat[1] = v9;
                    quat[2] = v11;
                    quat[3] = v13;
                    quat[4] = 0.0;
                    quat[5] = 0.0;
                    quat[6] = 0.0;
                    quat[7] = 0.0;
                }
                --v16;
                quat += 8;
                ++v20;
                quats += 4;
            }
        }
        Profile_EndInternal(0);
    }
}

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

    //Profile_Begin(310);
    if (!obj)
        MyAssertHandler(".\\xanim\\dobj_skel.cpp", 382, 0, "%s", "obj");
    skel = (DSkel *)&obj->skel;
    if (obj == (const DObj_s *)-20)
        MyAssertHandler(".\\xanim\\dobj_skel.cpp", 385, 0, "%s", "skel");
    bFinished = 1;
    for (i = 0; i < 4; ++i)
    {
        ignorePartBits[i] = skel->partBits.skel[i] | ~partBits[i];
        if (ignorePartBits[i] != -1)
            bFinished = 0;
    }
    if (bFinished)
    {
        //Profile_EndInternal(0);
    }
    else
    {
        DObjCalcAnim(obj, partBits);
        if (!obj->duplicateParts)
            MyAssertHandler(".\\xanim\\dobj_skel.cpp", 404, 0, "%s", "obj->duplicateParts");
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
        if (*pos)
            MyAssertHandler(".\\xanim\\dobj_skel.cpp", 433, 0, "%s", "!(*pos)");
        for (boneIndexa = 0; boneIndexa < obj->numBones; ++boneIndexa)
        {
            if ((skel->partBits.anim[boneIndexa >> 5] & (0x80000000 >> (boneIndexa & 0x1F))) != 0)
            {
                if ((COERCE_UNSIGNED_INT(skel->mat[boneIndexa].quat[0]) & 0x7F800000) == 0x7F800000
                    || (COERCE_UNSIGNED_INT(skel->mat[boneIndexa].quat[1]) & 0x7F800000) == 0x7F800000
                    || (COERCE_UNSIGNED_INT(skel->mat[boneIndexa].quat[2]) & 0x7F800000) == 0x7F800000
                    || (COERCE_UNSIGNED_INT(skel->mat[boneIndexa].quat[3]) & 0x7F800000) == 0x7F800000)
                {
                    MyAssertHandler(
                        ".\\xanim\\dobj_skel.cpp",
                        440,
                        0,
                        "%s",
                        "!IS_NAN((skel->mat[boneIndex].quat)[0]) && !IS_NAN((skel->mat[boneIndex].quat)[1]) && !IS_NAN((skel->mat[bon"
                        "eIndex].quat)[2]) && !IS_NAN((skel->mat[boneIndex].quat)[3])");
                }
                if ((COERCE_UNSIGNED_INT(skel->mat[boneIndexa].trans[0]) & 0x7F800000) == 0x7F800000
                    || (COERCE_UNSIGNED_INT(skel->mat[boneIndexa].trans[1]) & 0x7F800000) == 0x7F800000
                    || (COERCE_UNSIGNED_INT(skel->mat[boneIndexa].trans[2]) & 0x7F800000) == 0x7F800000)
                {
                    MyAssertHandler(
                        ".\\xanim\\dobj_skel.cpp",
                        441,
                        0,
                        "%s",
                        "!IS_NAN((skel->mat[boneIndex].trans)[0]) && !IS_NAN((skel->mat[boneIndex].trans)[1]) && !IS_NAN((skel->mat[b"
                        "oneIndex].trans)[2])");
                }
            }
        }
        //Profile_EndInternal(0);
    }
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
        boneIndex = *pos - 1;
        if (boneIndex >= maxBoneIndex)
            break;
        parentIndex = pos[1] - 1;
        if (parentIndex >= boneIndex)
            MyAssertHandler(".\\xanim\\dobj_skel.cpp", 131, 0, "%s", "parentIndex < boneIndex");
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
            if (((0x80000000 >> boneIndexLow) & bits) == 0)
                MyAssertHandler(".\\xanim\\dobj_skel.cpp", 174, 0, "%s", "bits & boneBit");
            bits &= ~(0x80000000 >> boneIndexLow);
            calcPartBits[boneIndexHigh] = bits;
            v = mat[boneIndex].quat;
            v8 = Vec4LengthSq(v);
            if (v8 == 0.0)
            {
                v[3] = 1.0;
                v[7] = 2.0;
            }
            else
            {
                v[7] = 2.0 / v8;
            }
            if ((COERCE_UNSIGNED_INT(mat[boneIndex].quat[0]) & 0x7F800000) == 0x7F800000
                || (COERCE_UNSIGNED_INT(mat[boneIndex].quat[1]) & 0x7F800000) == 0x7F800000
                || (COERCE_UNSIGNED_INT(mat[boneIndex].quat[2]) & 0x7F800000) == 0x7F800000
                || (COERCE_UNSIGNED_INT(mat[boneIndex].quat[3]) & 0x7F800000) == 0x7F800000)
            {
                MyAssertHandler(
                    ".\\xanim\\dobj_skel.cpp",
                    180,
                    0,
                    "%s",
                    "!IS_NAN((mat[boneIndex].quat)[0]) && !IS_NAN((mat[boneIndex].quat)[1]) && !IS_NAN((mat[boneIndex].quat)[2]) &&"
                    " !IS_NAN((mat[boneIndex].quat)[3])");
            }
            if ((COERCE_UNSIGNED_INT(mat[boneIndex].trans[0]) & 0x7F800000) == 0x7F800000
                || (COERCE_UNSIGNED_INT(mat[boneIndex].trans[1]) & 0x7F800000) == 0x7F800000
                || (COERCE_UNSIGNED_INT(mat[boneIndex].trans[2]) & 0x7F800000) == 0x7F800000)
            {
                MyAssertHandler(
                    ".\\xanim\\dobj_skel.cpp",
                    181,
                    0,
                    "%s",
                    "!IS_NAN((mat[boneIndex].trans)[0]) && !IS_NAN((mat[boneIndex].trans)[1]) && !IS_NAN((mat[boneIndex].trans)[2])");
            }
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
    unsigned int v8; // [esp+8h] [ebp-104h]
    float *trans; // [esp+18h] [ebp-F4h]
    float v10; // [esp+30h] [ebp-DCh]
    float v11; // [esp+34h] [ebp-D8h]
    float v12; // [esp+38h] [ebp-D4h]
    float v13; // [esp+3Ch] [ebp-D0h]
    float result; // [esp+40h] [ebp-CCh] BYREF
    float v15; // [esp+44h] [ebp-C8h]
    float v16; // [esp+48h] [ebp-C4h]
    float v17; // [esp+4Ch] [ebp-C0h]
    float v18; // [esp+50h] [ebp-BCh]
    float v19; // [esp+54h] [ebp-B8h]
    float v20; // [esp+58h] [ebp-B4h]
    float v21; // [esp+5Ch] [ebp-B0h]
    float v22; // [esp+60h] [ebp-ACh]
    float v23; // [esp+64h] [ebp-A8h]
    float v24; // [esp+68h] [ebp-A4h]
    float v25; // [esp+6Ch] [ebp-A0h]
    float v26; // [esp+70h] [ebp-9Ch]
    float v27; // [esp+74h] [ebp-98h]
    float v28; // [esp+78h] [ebp-94h]
    float v29; // [esp+7Ch] [ebp-90h]
    float v30; // [esp+80h] [ebp-8Ch]
    float v31; // [esp+84h] [ebp-88h]
    float v32; // [esp+88h] [ebp-84h]
    float v33; // [esp+8Ch] [ebp-80h]
    float v34; // [esp+90h] [ebp-7Ch]
    float v35; // [esp+94h] [ebp-78h]
    float v36; // [esp+98h] [ebp-74h]
    float v37; // [esp+9Ch] [ebp-70h]
    float v38; // [esp+A0h] [ebp-6Ch]
    float v39; // [esp+A4h] [ebp-68h]
    float v40; // [esp+A8h] [ebp-64h]
    float v41; // [esp+ACh] [ebp-60h]
    float v42; // [esp+B0h] [ebp-5Ch]
    float v43; // [esp+B4h] [ebp-58h]
    float v44; // [esp+B8h] [ebp-54h]
    float v45; // [esp+BCh] [ebp-50h]
    float v46; // [esp+C0h] [ebp-4Ch]
    float v47; // [esp+C4h] [ebp-48h]
    float v48; // [esp+C8h] [ebp-44h]
    float v49; // [esp+CCh] [ebp-40h]
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
        if ((int)maxBoneIndex > 32)
            v8 = 32;
        else
            v8 = maxBoneIndex;
        maxBoneIndexLow = v8;
        while (1)
        {
            if (!_BitScanReverse(&v7, bits))
                v7 = 63;// `CountLeadingZeros'::`2': : notFound;
            boneIndexLow = v7 ^ 0x1F;
            if ((v7 ^ 0x1Fu) >= maxBoneIndexLow)
                break;
            boneIndex = boneIndexLow + 32 * boneIndexHigh;
            boneBit = 0x80000000 >> boneIndexLow;
            if (((0x80000000 >> boneIndexLow) & bits) == 0)
                MyAssertHandler(".\\xanim\\dobj_skel.cpp", 227, 0, "%s", "bits & boneBit");
            bits &= ~boneBit;
            calcPartBits[boneIndexHigh] = bits;
            if (modelParent >= boneIndex)
                MyAssertHandler(".\\xanim\\dobj_skel.cpp", 231, 0, "%s", "modelParent < boneIndex");
            if ((boneBit & skel->partBits.anim[boneIndexHigh]) == 0)
                MyAssertHandler(".\\xanim\\dobj_skel.cpp", 232, 0, "%s", "skel->partBits.anim[boneIndexHigh] & boneBit");
            if ((skel->partBits.skel[modelParent >> 5] & (0x80000000 >> (modelParent & 0x1F))) == 0)
                MyAssertHandler(
                    ".\\xanim\\dobj_skel.cpp",
                    233,
                    0,
                    "%s",
                    "skel->partBits.skel[modelParent >> 5] & (HIGH_BIT >> (modelParent & 31))");
            if ((skel->partBits.anim[modelParent >> 5] & (0x80000000 >> (modelParent & 0x1F))) == 0)
                MyAssertHandler(
                    ".\\xanim\\dobj_skel.cpp",
                    234,
                    0,
                    "%s",
                    "skel->partBits.anim[modelParent >> 5] & (HIGH_BIT >> (modelParent & 31))");
            childMat = &skel->mat[boneIndex];
            if ((boneBit & controlPartBits[boneIndexHigh]) != 0)
            {
                if (skel->partBits.skel[0] >= 0)
                    MyAssertHandler(".\\xanim\\dobj_skel.cpp", 243, 0, "%s", "skel->partBits.skel[0] & HIGH_BIT");
                if (skel->partBits.anim[0] >= 0)
                    MyAssertHandler(".\\xanim\\dobj_skel.cpp", 244, 0, "%s", "skel->partBits.anim[0] & HIGH_BIT");
                quat[0] = parentMat->quat[0] * mat->quat[3]
                    - parentMat->quat[3] * mat->quat[0]
                    - parentMat->quat[2] * mat->quat[1]
                    + parentMat->quat[1] * mat->quat[2];
                quat[1] = parentMat->quat[1] * mat->quat[3]
                    + parentMat->quat[2] * mat->quat[0]
                    - parentMat->quat[3] * mat->quat[1]
                    - parentMat->quat[0] * mat->quat[2];
                quat[2] = parentMat->quat[2] * mat->quat[3]
                    - parentMat->quat[1] * mat->quat[0]
                    + parentMat->quat[0] * mat->quat[1]
                    - parentMat->quat[3] * mat->quat[2];
                quat[3] = parentMat->quat[3] * mat->quat[3]
                    + parentMat->quat[0] * mat->quat[0]
                    + parentMat->quat[1] * mat->quat[1]
                    + parentMat->quat[2] * mat->quat[2];
                v44 = quat[0] * childMat->quat[3]
                    + quat[3] * childMat->quat[0]
                    + quat[2] * childMat->quat[1]
                    - quat[1] * childMat->quat[2];
                v45 = quat[1] * childMat->quat[3]
                    - quat[2] * childMat->quat[0]
                    + quat[3] * childMat->quat[1]
                    + quat[0] * childMat->quat[2];
                v46 = quat[2] * childMat->quat[3]
                    + quat[1] * childMat->quat[0]
                    - quat[0] * childMat->quat[1]
                    + quat[3] * childMat->quat[2];
                childMat->quat[3] = quat[3] * childMat->quat[3]
                    - quat[0] * childMat->quat[0]
                    - quat[1] * childMat->quat[1]
                    - quat[2] * childMat->quat[2];
                childMat->quat[0] = v44;
                childMat->quat[1] = v45;
                childMat->quat[2] = v46;
                v41 = childMat->quat[0] * mat->quat[3]
                    + childMat->quat[3] * mat->quat[0]
                    + childMat->quat[2] * mat->quat[1]
                    - childMat->quat[1] * mat->quat[2];
                v42 = childMat->quat[1] * mat->quat[3]
                    - childMat->quat[2] * mat->quat[0]
                    + childMat->quat[3] * mat->quat[1]
                    + childMat->quat[0] * mat->quat[2];
                v43 = childMat->quat[2] * mat->quat[3]
                    + childMat->quat[1] * mat->quat[0]
                    - childMat->quat[0] * mat->quat[1]
                    + childMat->quat[3] * mat->quat[2];
                childMat->quat[3] = childMat->quat[3] * mat->quat[3]
                    - childMat->quat[0] * mat->quat[0]
                    - childMat->quat[1] * mat->quat[1]
                    - childMat->quat[2] * mat->quat[2];
                childMat->quat[0] = v41;
                childMat->quat[1] = v42;
                childMat->quat[2] = v43;
            }
            else
            {
                v47 = childMat->quat[0] * parentMat->quat[3]
                    + childMat->quat[3] * parentMat->quat[0]
                    + childMat->quat[2] * parentMat->quat[1]
                    - childMat->quat[1] * parentMat->quat[2];
                v48 = childMat->quat[1] * parentMat->quat[3]
                    - childMat->quat[2] * parentMat->quat[0]
                    + childMat->quat[3] * parentMat->quat[1]
                    + childMat->quat[0] * parentMat->quat[2];
                v49 = childMat->quat[2] * parentMat->quat[3]
                    + childMat->quat[1] * parentMat->quat[0]
                    - childMat->quat[0] * parentMat->quat[1]
                    + childMat->quat[3] * parentMat->quat[2];
                childMat->quat[3] = childMat->quat[3] * parentMat->quat[3]
                    - childMat->quat[0] * parentMat->quat[0]
                    - childMat->quat[1] * parentMat->quat[1]
                    - childMat->quat[2] * parentMat->quat[2];
                childMat->quat[0] = v47;
                childMat->quat[1] = v48;
                childMat->quat[2] = v49;
            }
            v40 = childMat->quat[0];
            if ((LODWORD(v40) & 0x7F800000) == 0x7F800000
                || (v39 = childMat->quat[1], (LODWORD(v39) & 0x7F800000) == 0x7F800000)
                || (v38 = childMat->quat[2], (LODWORD(v38) & 0x7F800000) == 0x7F800000)
                || (v37 = childMat->quat[3], (LODWORD(v37) & 0x7F800000) == 0x7F800000))
            {
                MyAssertHandler(
                    ".\\xanim\\dobj_skel.cpp",
                    251,
                    0,
                    "%s",
                    "!IS_NAN((childMat->quat)[0]) && !IS_NAN((childMat->quat)[1]) && !IS_NAN((childMat->quat)[2]) && !IS_NAN((childMat->quat)[3])");
            }
            v36 = childMat->trans[0];
            if ((LODWORD(v36) & 0x7F800000) == 0x7F800000
                || (v35 = childMat->trans[1], (LODWORD(v35) & 0x7F800000) == 0x7F800000)
                || (v34 = childMat->trans[2], (LODWORD(v34) & 0x7F800000) == 0x7F800000))
            {
                MyAssertHandler(
                    ".\\xanim\\dobj_skel.cpp",
                    252,
                    0,
                    "%s",
                    "!IS_NAN((childMat->trans)[0]) && !IS_NAN((childMat->trans)[1]) && !IS_NAN((childMat->trans)[2])");
            }
            v33 = Vec4LengthSq(childMat->quat);
            if (v33 == 0.0)
            {
                childMat->quat[3] = 1.0;
                childMat->transWeight = 2.0;
            }
            else
            {
                childMat->transWeight = 2.0 / v33;
            }
            trans = childMat->trans;
            if ((COERCE_UNSIGNED_INT(parentMat->quat[0]) & 0x7F800000) == 0x7F800000
                || (COERCE_UNSIGNED_INT(parentMat->quat[1]) & 0x7F800000) == 0x7F800000
                || (COERCE_UNSIGNED_INT(parentMat->quat[2]) & 0x7F800000) == 0x7F800000
                || (COERCE_UNSIGNED_INT(parentMat->quat[3]) & 0x7F800000) == 0x7F800000)
            {
                MyAssertHandler(
                    "c:\\trees\\cod3\\src\\xanim\\xanim_public.h",
                    432,
                    0,
                    "%s",
                    "!IS_NAN((mat->quat)[0]) && !IS_NAN((mat->quat)[1]) && !IS_NAN((mat->quat)[2]) && !IS_NAN((mat->quat)[3])");
            }
            if ((COERCE_UNSIGNED_INT(parentMat->transWeight) & 0x7F800000) == 0x7F800000)
                MyAssertHandler("c:\\trees\\cod3\\src\\xanim\\xanim_public.h", 433, 0, "%s", "!IS_NAN(mat->transWeight)");
            Vec3Scale(parentMat->quat, parentMat->transWeight, &result);
            v20 = result * parentMat->quat[0];
            v11 = result * parentMat->quat[1];
            v18 = result * parentMat->quat[2];
            v21 = result * parentMat->quat[3];
            v10 = v15 * parentMat->quat[1];
            v19 = v15 * parentMat->quat[2];
            v17 = v15 * parentMat->quat[3];
            v12 = v16 * parentMat->quat[2];
            v13 = v16 * parentMat->quat[3];
            v24 = 1.0 - (v10 + v12);
            v25 = v11 + v13;
            v26 = v18 - v17;
            v27 = v11 - v13;
            v28 = 1.0 - (v20 + v12);
            v29 = v19 + v21;
            v30 = v18 + v17;
            v31 = v19 - v21;
            v32 = 1.0 - (v20 + v10);
            v22 = *trans * v24 + trans[1] * v27 + trans[2] * v30 + parentMat->trans[0];
            v23 = *trans * v25 + trans[1] * v28 + trans[2] * v31 + parentMat->trans[1];
            trans[2] = *trans * v26 + trans[1] * v29 + trans[2] * v32 + parentMat->trans[2];
            *trans = v22;
            trans[1] = v23;
            if ((COERCE_UNSIGNED_INT(childMat->trans[0]) & 0x7F800000) == 0x7F800000
                || (COERCE_UNSIGNED_INT(childMat->trans[1]) & 0x7F800000) == 0x7F800000
                || (COERCE_UNSIGNED_INT(childMat->trans[2]) & 0x7F800000) == 0x7F800000)
            {
                MyAssertHandler(
                    ".\\xanim\\dobj_skel.cpp",
                    257,
                    0,
                    "%s",
                    "!IS_NAN((childMat->trans)[0]) && !IS_NAN((childMat->trans)[1]) && !IS_NAN((childMat->trans)[2])");
            }
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
    int v7; // [esp+8h] [ebp-124h]
    float *v8; // [esp+18h] [ebp-114h]
    float v9; // [esp+30h] [ebp-FCh]
    float v10; // [esp+34h] [ebp-F8h]
    float v11; // [esp+38h] [ebp-F4h]
    float v12; // [esp+3Ch] [ebp-F0h]
    float result; // [esp+40h] [ebp-ECh] BYREF
    float v14; // [esp+44h] [ebp-E8h]
    float v15; // [esp+48h] [ebp-E4h]
    float v16; // [esp+4Ch] [ebp-E0h]
    float v17; // [esp+50h] [ebp-DCh]
    float v18; // [esp+54h] [ebp-D8h]
    float v19; // [esp+58h] [ebp-D4h]
    float v20; // [esp+5Ch] [ebp-D0h]
    float v21; // [esp+60h] [ebp-CCh]
    float v22; // [esp+64h] [ebp-C8h]
    float v23; // [esp+68h] [ebp-C4h]
    float v24; // [esp+6Ch] [ebp-C0h]
    float v25; // [esp+70h] [ebp-BCh]
    float v26; // [esp+74h] [ebp-B8h]
    float v27; // [esp+78h] [ebp-B4h]
    float v28; // [esp+7Ch] [ebp-B0h]
    float v29; // [esp+80h] [ebp-ACh]
    float v30; // [esp+84h] [ebp-A8h]
    float v31; // [esp+88h] [ebp-A4h]
    float v32; // [esp+8Ch] [ebp-A0h]
    float v33; // [esp+90h] [ebp-9Ch]
    float v34; // [esp+94h] [ebp-98h]
    float v35; // [esp+98h] [ebp-94h]
    float v36; // [esp+9Ch] [ebp-90h]
    float v37; // [esp+A0h] [ebp-8Ch]
    float v38; // [esp+A4h] [ebp-88h]
    float v39; // [esp+A8h] [ebp-84h]
    float v40; // [esp+ACh] [ebp-80h]
    float v41; // [esp+B0h] [ebp-7Ch]
    float v42; // [esp+B4h] [ebp-78h]
    float v43; // [esp+B8h] [ebp-74h]
    float v44; // [esp+BCh] [ebp-70h]
    float v45; // [esp+C0h] [ebp-6Ch]
    float v46; // [esp+C4h] [ebp-68h]
    float v47; // [esp+C8h] [ebp-64h]
    float v48; // [esp+CCh] [ebp-60h]
    float v49; // [esp+D0h] [ebp-5Ch]
    float v50; // [esp+D4h] [ebp-58h]
    float v51; // [esp+D8h] [ebp-54h]
    int v52; // [esp+DCh] [ebp-50h]
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
    maxBoneIndex -= 32 * (minBoneIndex >> 5);
    mat = skel->mat;
    while (boneIndexHigh <= maxBoneIndexHigh)
    {
        bits = calcPartBits[boneIndexHigh];
        if (maxBoneIndex > 32)
            v7 = 32;
        else
            v7 = maxBoneIndex;
        maxBoneIndexLow = v7;
        while (1)
        {
            if (!_BitScanReverse(&v6, bits))
                v6 = 63;// `CountLeadingZeros'::`2': : notFound;
            v52 = v6 ^ 0x1F;
            boneIndexLow = v6 ^ 0x1F;
            if ((v6 ^ 0x1F) >= maxBoneIndexLow)
                break;
            boneIndex = boneIndexLow + 32 * boneIndexHigh;
            boneBit = 0x80000000 >> boneIndexLow;
            if (((0x80000000 >> boneIndexLow) & bits) == 0)
                MyAssertHandler(".\\xanim\\dobj_skel.cpp", 306, 0, "%s", "bits & boneBit");
            bits &= ~boneBit;
            calcPartBits[boneIndexHigh] = bits;
            childMat = &mat[boneIndex];
            boneOffset = boneIndex - minBoneIndex;
            if (boneIndex - minBoneIndex >= model->numBones - (unsigned int)model->numRootBones)
                MyAssertHandler(
                    ".\\xanim\\dobj_skel.cpp",
                    313,
                    0,
                    "boneOffset doesn't index model->numBones - model->numRootBones\n\t%i not in [0, %i)",
                    boneOffset,
                    model->numBones - model->numRootBones);
            parentList = &model->parentList[boneOffset];
            parentOffset = *parentList;
            parentMat = &childMat[-parentOffset];
            if ((boneBit & skel->partBits.anim[boneIndexHigh]) == 0)
                MyAssertHandler(".\\xanim\\dobj_skel.cpp", 319, 0, "%s", "skel->partBits.anim[boneIndexHigh] & boneBit");
            if ((skel->partBits.skel[(boneIndex - parentOffset) >> 5] & (0x80000000 >> ((boneIndex - parentOffset) & 0x1F))) == 0)
                MyAssertHandler(
                    ".\\xanim\\dobj_skel.cpp",
                    320,
                    0,
                    "%s",
                    "skel->partBits.skel[(boneIndex - parentOffset) >> 5] & (HIGH_BIT >> ((boneIndex - parentOffset) & 31))");
            if ((skel->partBits.anim[(boneIndex - parentOffset) >> 5] & (0x80000000 >> ((boneIndex - parentOffset) & 0x1F))) == 0)
                MyAssertHandler(
                    ".\\xanim\\dobj_skel.cpp",
                    321,
                    0,
                    "%s",
                    "skel->partBits.anim[(boneIndex - parentOffset) >> 5] & (HIGH_BIT >> ((boneIndex - parentOffset) & 31))");
            if ((boneBit & controlPartBits[boneIndexHigh]) != 0)
            {
                if (skel->partBits.skel[0] >= 0)
                    MyAssertHandler(".\\xanim\\dobj_skel.cpp", 329, 0, "%s", "skel->partBits.skel[0] & HIGH_BIT");
                if (skel->partBits.anim[0] >= 0)
                    MyAssertHandler(".\\xanim\\dobj_skel.cpp", 330, 0, "%s", "skel->partBits.anim[0] & HIGH_BIT");
                quat[0] = parentMat->quat[0] * mat->quat[3]
                    - parentMat->quat[3] * mat->quat[0]
                    - parentMat->quat[2] * mat->quat[1]
                    + parentMat->quat[1] * mat->quat[2];
                quat[1] = parentMat->quat[1] * mat->quat[3]
                    + parentMat->quat[2] * mat->quat[0]
                    - parentMat->quat[3] * mat->quat[1]
                    - parentMat->quat[0] * mat->quat[2];
                quat[2] = parentMat->quat[2] * mat->quat[3]
                    - parentMat->quat[1] * mat->quat[0]
                    + parentMat->quat[0] * mat->quat[1]
                    - parentMat->quat[3] * mat->quat[2];
                quat[3] = parentMat->quat[3] * mat->quat[3]
                    + parentMat->quat[0] * mat->quat[0]
                    + parentMat->quat[1] * mat->quat[1]
                    + parentMat->quat[2] * mat->quat[2];
                v46 = quat[0] * childMat->quat[3]
                    + quat[3] * childMat->quat[0]
                    + quat[2] * childMat->quat[1]
                    - quat[1] * childMat->quat[2];
                v47 = quat[1] * childMat->quat[3]
                    - quat[2] * childMat->quat[0]
                    + quat[3] * childMat->quat[1]
                    + quat[0] * childMat->quat[2];
                v48 = quat[2] * childMat->quat[3]
                    + quat[1] * childMat->quat[0]
                    - quat[0] * childMat->quat[1]
                    + quat[3] * childMat->quat[2];
                childMat->quat[3] = quat[3] * childMat->quat[3]
                    - quat[0] * childMat->quat[0]
                    - quat[1] * childMat->quat[1]
                    - quat[2] * childMat->quat[2];
                childMat->quat[0] = v46;
                childMat->quat[1] = v47;
                childMat->quat[2] = v48;
                v43 = childMat->quat[0] * mat->quat[3]
                    + childMat->quat[3] * mat->quat[0]
                    + childMat->quat[2] * mat->quat[1]
                    - childMat->quat[1] * mat->quat[2];
                v44 = childMat->quat[1] * mat->quat[3]
                    - childMat->quat[2] * mat->quat[0]
                    + childMat->quat[3] * mat->quat[1]
                    + childMat->quat[0] * mat->quat[2];
                v45 = childMat->quat[2] * mat->quat[3]
                    + childMat->quat[1] * mat->quat[0]
                    - childMat->quat[0] * mat->quat[1]
                    + childMat->quat[3] * mat->quat[2];
                childMat->quat[3] = childMat->quat[3] * mat->quat[3]
                    - childMat->quat[0] * mat->quat[0]
                    - childMat->quat[1] * mat->quat[1]
                    - childMat->quat[2] * mat->quat[2];
                childMat->quat[0] = v43;
                childMat->quat[1] = v44;
                childMat->quat[2] = v45;
            }
            else
            {
                v49 = childMat->quat[0] * parentMat->quat[3]
                    + childMat->quat[3] * parentMat->quat[0]
                    + childMat->quat[2] * parentMat->quat[1]
                    - childMat->quat[1] * parentMat->quat[2];
                v50 = childMat->quat[1] * parentMat->quat[3]
                    - childMat->quat[2] * parentMat->quat[0]
                    + childMat->quat[3] * parentMat->quat[1]
                    + childMat->quat[0] * parentMat->quat[2];
                v51 = childMat->quat[2] * parentMat->quat[3]
                    + childMat->quat[1] * parentMat->quat[0]
                    - childMat->quat[0] * parentMat->quat[1]
                    + childMat->quat[3] * parentMat->quat[2];
                childMat->quat[3] = childMat->quat[3] * parentMat->quat[3]
                    - childMat->quat[0] * parentMat->quat[0]
                    - childMat->quat[1] * parentMat->quat[1]
                    - childMat->quat[2] * parentMat->quat[2];
                childMat->quat[0] = v49;
                childMat->quat[1] = v50;
                childMat->quat[2] = v51;
            }
            v42 = childMat->quat[0];
            if ((LODWORD(v42) & 0x7F800000) == 0x7F800000
                || (v41 = childMat->quat[1], (LODWORD(v41) & 0x7F800000) == 0x7F800000)
                || (v40 = childMat->quat[2], (LODWORD(v40) & 0x7F800000) == 0x7F800000)
                || (v39 = childMat->quat[3], (LODWORD(v39) & 0x7F800000) == 0x7F800000))
            {
                MyAssertHandler(
                    ".\\xanim\\dobj_skel.cpp",
                    337,
                    0,
                    "%s",
                    "!IS_NAN((childMat->quat)[0]) && !IS_NAN((childMat->quat)[1]) && !IS_NAN((childMat->quat)[2]) && !IS_NAN((childMat->quat)[3])");
            }
            v38 = childMat->trans[0];
            if ((LODWORD(v38) & 0x7F800000) == 0x7F800000
                || (v37 = childMat->trans[1], (LODWORD(v37) & 0x7F800000) == 0x7F800000)
                || (v36 = childMat->trans[2], (LODWORD(v36) & 0x7F800000) == 0x7F800000))
            {
                MyAssertHandler(
                    ".\\xanim\\dobj_skel.cpp",
                    338,
                    0,
                    "%s",
                    "!IS_NAN((childMat->trans)[0]) && !IS_NAN((childMat->trans)[1]) && !IS_NAN((childMat->trans)[2])");
            }
            v35 = Vec4LengthSq(childMat->quat);
            if (v35 == 0.0)
            {
                childMat->quat[3] = 1.0;
                childMat->transWeight = 2.0;
            }
            else
            {
                childMat->transWeight = 2.0 / v35;
            }
            trans = &model->trans[3 * boneOffset];
            v34 = *trans;
            if ((LODWORD(v34) & 0x7F800000) == 0x7F800000
                || (v33 = trans[1], (LODWORD(v33) & 0x7F800000) == 0x7F800000)
                || (v32 = trans[2], (LODWORD(v32) & 0x7F800000) == 0x7F800000))
            {
                MyAssertHandler(
                    ".\\xanim\\dobj_skel.cpp",
                    343,
                    0,
                    "%s",
                    "!IS_NAN((trans)[0]) && !IS_NAN((trans)[1]) && !IS_NAN((trans)[2])");
            }
            Vec3Add(childMat->trans, trans, childMat->trans);
            v8 = childMat->trans;
            if ((COERCE_UNSIGNED_INT(parentMat->quat[0]) & 0x7F800000) == 0x7F800000
                || (COERCE_UNSIGNED_INT(parentMat->quat[1]) & 0x7F800000) == 0x7F800000
                || (COERCE_UNSIGNED_INT(parentMat->quat[2]) & 0x7F800000) == 0x7F800000
                || (COERCE_UNSIGNED_INT(parentMat->quat[3]) & 0x7F800000) == 0x7F800000)
            {
                MyAssertHandler(
                    "c:\\trees\\cod3\\src\\xanim\\xanim_public.h",
                    432,
                    0,
                    "%s",
                    "!IS_NAN((mat->quat)[0]) && !IS_NAN((mat->quat)[1]) && !IS_NAN((mat->quat)[2]) && !IS_NAN((mat->quat)[3])");
            }
            if ((COERCE_UNSIGNED_INT(parentMat->transWeight) & 0x7F800000) == 0x7F800000)
                MyAssertHandler("c:\\trees\\cod3\\src\\xanim\\xanim_public.h", 433, 0, "%s", "!IS_NAN(mat->transWeight)");
            Vec3Scale(parentMat->quat, parentMat->transWeight, &result);
            v19 = result * parentMat->quat[0];
            v10 = result * parentMat->quat[1];
            v17 = result * parentMat->quat[2];
            v20 = result * parentMat->quat[3];
            v9 = v14 * parentMat->quat[1];
            v18 = v14 * parentMat->quat[2];
            v16 = v14 * parentMat->quat[3];
            v11 = v15 * parentMat->quat[2];
            v12 = v15 * parentMat->quat[3];
            v23 = 1.0 - (v9 + v11);
            v24 = v10 + v12;
            v25 = v17 - v16;
            v26 = v10 - v12;
            v27 = 1.0 - (v19 + v11);
            v28 = v18 + v20;
            v29 = v17 + v16;
            v30 = v18 - v20;
            v31 = 1.0 - (v19 + v9);
            v21 = *v8 * v23 + v8[1] * v26 + v8[2] * v29 + parentMat->trans[0];
            v22 = *v8 * v24 + v8[1] * v27 + v8[2] * v30 + parentMat->trans[1];
            v8[2] = *v8 * v25 + v8[1] * v28 + v8[2] * v31 + parentMat->trans[2];
            *v8 = v21;
            v8[1] = v22;
            if ((COERCE_UNSIGNED_INT(childMat->trans[0]) & 0x7F800000) == 0x7F800000
                || (COERCE_UNSIGNED_INT(childMat->trans[1]) & 0x7F800000) == 0x7F800000
                || (COERCE_UNSIGNED_INT(childMat->trans[2]) & 0x7F800000) == 0x7F800000)
            {
                MyAssertHandler(
                    ".\\xanim\\dobj_skel.cpp",
                    348,
                    0,
                    "%s",
                    "!IS_NAN((childMat->trans)[0]) && !IS_NAN((childMat->trans)[1]) && !IS_NAN((childMat->trans)[2])");
            }
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
                mat->quat[0] = (double)*quats * 0.00003051850944757462;
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

