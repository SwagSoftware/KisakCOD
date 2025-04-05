#include "xanim_calc.h"
#include <universal/q_shared.h>


void __cdecl XAnimCalc(
    const DObj_s *obj,
    XAnimInfo *info,
    float weightScale,
    bool bClear,
    bool bNormQuat,
    XAnimCalcAnimInfo *animInfo,
    int rotTransArrayIndex,
    DObjAnimMat *rotTransArray)
{
    XAnimInfo *firstInfo; // [esp+18h] [ebp-28h]
    XAnimInfo *secondInfo; // [esp+1Ch] [ebp-24h]
    XAnimInfo *secondInfoa; // [esp+1Ch] [ebp-24h]
    const XAnimTree_s *tree; // [esp+20h] [ebp-20h]
    unsigned int secondInfoIndex; // [esp+24h] [ebp-1Ch]
    DObjAnimMat *calcBuffer; // [esp+28h] [ebp-18h]
    unsigned int firstInfoIndex; // [esp+2Ch] [ebp-14h]
    bool secondChildFound; // [esp+32h] [ebp-Eh]
    bool additiveChildExists; // [esp+33h] [ebp-Dh]
    int allocedCalcBuffer; // [esp+34h] [ebp-Ch]
    float weight; // [esp+38h] [ebp-8h]
    float weighta; // [esp+38h] [ebp-8h]
    float firstWeight; // [esp+3Ch] [ebp-4h]

    tree = obj->tree;
    if (!obj->tree)
        MyAssertHandler(".\\xanim\\xanim_calc.cpp", 1293, 0, "%s", "tree");
    if (!tree->anims)
        MyAssertHandler(".\\xanim\\xanim_calc.cpp", 1294, 0, "%s", "tree->anims");
    if (info->tree != tree)
        MyAssertHandler(".\\xanim\\xanim_calc.cpp", 1298, 0, "%s", "info->tree == tree");
    if (!info->inuse)
        MyAssertHandler(".\\xanim\\xanim_calc.cpp", 1300, 0, "%s", "info->inuse");
    if (info->animToModel)
    {
        if (bClear)
            XAnimClearRotTransArray(obj, rotTransArray, animInfo);
        XAnimCalcLeaf(info, weightScale, rotTransArray, animInfo);
    }
    else
    {
        firstWeight = 0.0;
        firstInfo = 0;
        for (firstInfoIndex = info->children; firstInfoIndex; firstInfoIndex = firstInfo->next)
        {
            firstInfo = GetAnimInfo(firstInfoIndex);
            if (firstInfo->tree != tree)
                MyAssertHandler(".\\xanim\\xanim_calc.cpp", 1325, 0, "%s", "firstInfo->tree == tree");
            if (!firstInfo->inuse)
                MyAssertHandler(".\\xanim\\xanim_calc.cpp", 1327, 0, "%s", "firstInfo->inuse");
            firstWeight = firstInfo->state.weight;
            if (firstWeight != 0.0)
            {
                if (IsInfoAdditive(firstInfo))
                    firstInfoIndex = 0;
                break;
            }
        }
        if (firstInfoIndex)
        {
            if (!firstInfo)
                MyAssertHandler(".\\xanim\\xanim_calc.cpp", 1352, 0, "%s", "firstInfo");
            secondChildFound = 0;
            calcBuffer = 0;
            allocedCalcBuffer = 0;
            for (secondInfoIndex = firstInfo->next; secondInfoIndex; secondInfoIndex = secondInfo->next)
            {
                secondInfo = GetAnimInfo(secondInfoIndex);
                if (secondInfo->tree != tree)
                    MyAssertHandler(".\\xanim\\xanim_calc.cpp", 1363, 0, "%s", "secondInfo->tree == tree");
                if (!secondInfo->inuse)
                    MyAssertHandler(".\\xanim\\xanim_calc.cpp", 1365, 0, "%s", "secondInfo->inuse");
                weight = secondInfo->state.weight;
                if (weight != 0.0)
                {
                    if (IsInfoAdditive(secondInfo))
                        break;
                    if (!secondChildFound)
                    {
                        secondChildFound = 1;
                        if (bClear)
                        {
                            calcBuffer = rotTransArray;
                        }
                        else
                        {
                            calcBuffer = XAnimGetCalcBuffer(animInfo, obj, &rotTransArrayIndex);
                            if (!calcBuffer)
                                return;
                            allocedCalcBuffer = 1;
                        }
                        XAnimCalc(obj, firstInfo, firstWeight, 1, 1, animInfo, rotTransArrayIndex, calcBuffer);
                    }
                    if (!calcBuffer)
                        MyAssertHandler(".\\xanim\\xanim_calc.cpp", 1403, 0, "%s", "calcBuffer");
                    XAnimCalc(obj, secondInfo, weight, 0, 1, animInfo, rotTransArrayIndex, calcBuffer);
                }
            }
            if (secondChildFound)
            {
                if (!bNormQuat && !bClear)
                    MyAssertHandler(".\\xanim\\xanim_calc.cpp", 1410, 0, "%s", "bNormQuat || bClear");
                if (bNormQuat)
                {
                    if (bClear)
                        XAnimNormalizeRotScaleTransArray(obj->numBones, animInfo, weightScale, rotTransArray);
                    else
                        XAnimMadRotTransArray(obj->numBones, animInfo, weightScale, calcBuffer, rotTransArray);
                }
                else
                {
                    XAnimScaleRotTransArray(obj->numBones, animInfo, rotTransArray);
                }
            }
            else
            {
                XAnimCalc(obj, firstInfo, weightScale, bClear, bNormQuat, animInfo, rotTransArrayIndex, rotTransArray);
            }
            if (secondInfoIndex)
            {
                if (allocedCalcBuffer || (calcBuffer = XAnimGetCalcBuffer(animInfo, obj, &rotTransArrayIndex)) != 0)
                {
                    additiveChildExists = 0;
                    while (secondInfoIndex)
                    {
                        secondInfoa = GetAnimInfo(secondInfoIndex);
                        if (secondInfoa->tree != tree)
                            MyAssertHandler(".\\xanim\\xanim_calc.cpp", 1452, 0, "%s", "secondInfo->tree == tree");
                        if (!secondInfoa->inuse)
                            MyAssertHandler(".\\xanim\\xanim_calc.cpp", 1454, 0, "%s", "secondInfo->inuse");
                        if (!IsInfoAdditive(secondInfoa))
                            MyAssertHandler(".\\xanim\\xanim_calc.cpp", 1457, 0, "%s", "IsInfoAdditive( secondInfo )");
                        weighta = secondInfoa->state.weight;
                        if (weighta != 0.0)
                        {
                            XAnimCalc(obj, secondInfoa, weighta, !additiveChildExists, 1, animInfo, rotTransArrayIndex, calcBuffer);
                            additiveChildExists = 1;
                        }
                        secondInfoIndex = secondInfoa->next;
                    }
                    if (additiveChildExists)
                        XAnimApplyAdditives(rotTransArray, calcBuffer, weightScale, obj->numBones, animInfo);
                }
            }
        }
        else if (bClear)
        {
            XAnimClearRotTransArray(obj, rotTransArray, animInfo);
        }
    }
}

bool __cdecl IsInfoAdditive(const XAnimInfo *info)
{
    if (!info)
        MyAssertHandler(".\\xanim\\xanim_calc.cpp", 147, 0, "%s", "info");
    return !info->animToModel && (info->animParent.flags & 0x10) != 0;
}

void __cdecl XAnimClearRotTransArray(const DObj_s *obj, DObjAnimMat *rotTransArray, XAnimCalcAnimInfo *info)
{
    unsigned int modelPartIndex; // [esp+4h] [ebp-4h]

    for (modelPartIndex = 0; (int)modelPartIndex < obj->numBones; ++modelPartIndex)
    {
        if (modelPartIndex >= 0x80)
            MyAssertHandler("c:\\trees\\cod3\\src\\xanim\\../qcommon/bitarray.h", 66, 0, "%s", "pos < BIT_COUNT");
        if ((info->ignorePartBits.array[modelPartIndex >> 5] & (0x80000000 >> (modelPartIndex & 0x1F))) == 0)
        {
            rotTransArray->quat[0] = 0.0;
            rotTransArray->quat[1] = 0.0;
            rotTransArray->quat[2] = 0.0;
            rotTransArray->quat[3] = 0.0;
            rotTransArray->transWeight = 0.0;
            rotTransArray->trans[0] = 0.0;
            rotTransArray->trans[1] = 0.0;
            rotTransArray->trans[2] = 0.0;
        }
        ++rotTransArray;
    }
}

void __cdecl XAnimCalcLeaf(XAnimInfo *info, float weightScale, DObjAnimMat *rotTransArray, XAnimCalcAnimInfo *animInfo)
{
    const char *v4; // eax
    bool v5; // [esp+14h] [ebp-20h]
    float time; // [esp+24h] [ebp-10h]
    char *animToModel; // [esp+28h] [ebp-Ch]
    int i; // [esp+2Ch] [ebp-8h]
    XAnimParts *parts; // [esp+30h] [ebp-4h]

    if (!info->inuse)
        MyAssertHandler(".\\xanim\\xanim_calc.cpp", 1112, 0, "%s", "info->inuse");
    parts = info->parts;
    if (!parts)
        MyAssertHandler(".\\xanim\\xanim_calc.cpp", 1116, 0, "%s", "parts");
    if (!info->animToModel)
        MyAssertHandler(".\\xanim\\xanim_calc.cpp", 1118, 0, "%s", "info->animToModel");
    animToModel = SL_ConvertToString(info->animToModel);
    for (i = 0; i < 4; ++i)
        animInfo->animPartBits.array[i] |= *(unsigned int *)&animToModel[4 * i] & ~animInfo->ignorePartBits.array[i];
    time = info->state.currentAnimTime;
    if (time < 0.0)
        MyAssertHandler(".\\xanim\\xanim_calc.cpp", 1127, 0, "%s\n\t(time) = %g", "(time >= 0)", time);
    if (parts->bLoop)
        v5 = time < 1.0;
    else
        v5 = time <= 1.0;
    if (!v5)
    {
        v4 = va("time: %f, parts->bLoop: %d", time, parts->bLoop);
        MyAssertHandler(".\\xanim\\xanim_calc.cpp", 1129, 0, "%s\n\t%s", "parts->bLoop ? (time < 1.f) : (time <= 1.f)", v4);
    }
    if (time != 1.0 && parts->numframes)
    {
        if (parts->numframes >= 0x100u)
            XAnimCalcParts_unsigned_short_(
                parts,
                (const unsigned __int8 *)animToModel + 17,
                time,
                weightScale,
                rotTransArray,
                &animInfo->ignorePartBits);
        else
            XAnimCalcParts_unsigned_char_(
                parts,
                (const unsigned __int8 *)animToModel + 17,
                time,
                weightScale,
                rotTransArray,
                &animInfo->ignorePartBits);
    }
    else
    {
        if (parts->bLoop)
            MyAssertHandler(".\\xanim\\xanim_calc.cpp", 1133, 0, "%s", "!parts->bLoop");
        XAnimCalcNonLoopEnd(
            parts,
            (const unsigned __int8 *)animToModel + 17,
            weightScale,
            rotTransArray,
            &animInfo->ignorePartBits);
    }
}

void __cdecl XAnimCalcNonLoopEnd(
    const XAnimParts *parts,
    const unsigned __int8 *animToModel,
    float weightScale,
    DObjAnimMat *rotTransArray,
    const bitarray<128> *ignorePartBits)
{
    float *trans; // edx
    float *v6; // ecx
    int *v7; // [esp+54h] [ebp-1C8h]
    __int16 *v8; // [esp+58h] [ebp-1C4h]
    unsigned __int8 *v9; // [esp+5Ch] [ebp-1C0h]
    float v10[5]; // [esp+74h] [ebp-1A8h] BYREF
    float *quat; // [esp+88h] [ebp-194h]
    float v12[5]; // [esp+8Ch] [ebp-190h] BYREF
    float *result; // [esp+A0h] [ebp-17Ch]
    float v14[5]; // [esp+A4h] [ebp-178h] BYREF
    unsigned int v15; // [esp+B8h] [ebp-164h]
    float *start; // [esp+BCh] [ebp-160h]
    float dir[2]; // [esp+C0h] [ebp-15Ch] BYREF
    __int64 v18; // [esp+C8h] [ebp-154h]
    unsigned int v19; // [esp+D4h] [ebp-148h]
    float v20; // [esp+D8h] [ebp-144h]
    float v21; // [esp+DCh] [ebp-140h]
    float v22; // [esp+E0h] [ebp-13Ch]
    float v23; // [esp+E4h] [ebp-138h]
    float v24; // [esp+E8h] [ebp-134h]
    float v25; // [esp+ECh] [ebp-130h]
    float v26; // [esp+F0h] [ebp-12Ch]
    float v27; // [esp+F4h] [ebp-128h]
    __int16 *v28; // [esp+F8h] [ebp-124h]
    float v29; // [esp+FCh] [ebp-120h]
    float v30; // [esp+100h] [ebp-11Ch]
    float v31; // [esp+104h] [ebp-118h]
    float v32; // [esp+108h] [ebp-114h]
    float v33; // [esp+10Ch] [ebp-110h]
    float v34; // [esp+110h] [ebp-10Ch]
    float v35; // [esp+114h] [ebp-108h]
    float v36; // [esp+118h] [ebp-104h]
    float v37; // [esp+11Ch] [ebp-100h]
    float v38; // [esp+120h] [ebp-FCh]
    float v39; // [esp+124h] [ebp-F8h]
    float transWeight; // [esp+128h] [ebp-F4h]
    float v41; // [esp+12Ch] [ebp-F0h]
    float v42; // [esp+130h] [ebp-ECh]
    float v43; // [esp+134h] [ebp-E8h]
    float v44; // [esp+138h] [ebp-E4h]
    float v45; // [esp+13Ch] [ebp-E0h]
    float v46; // [esp+140h] [ebp-DCh]
    float v47; // [esp+144h] [ebp-D8h]
    float v48; // [esp+148h] [ebp-D4h]
    XAnimDynamicFrames frame; // [esp+14Ch] [ebp-D0h]
    float4 sizeVec; // [esp+150h] [ebp-CCh]
    float v51; // [esp+160h] [ebp-BCh]
    float v52; // [esp+164h] [ebp-B8h]
    float v53; // [esp+168h] [ebp-B4h]
    float v54; // [esp+16Ch] [ebp-B0h]
    float4 posVec; // [esp+170h] [ebp-ACh]
    float4 lerp; // [esp+180h] [ebp-9Ch]
    float4 minsVec; // [esp+190h] [ebp-8Ch]
    float v58; // [esp+1A0h] [ebp-7Ch]
    float v59; // [esp+1A4h] [ebp-78h]
    float v60; // [esp+1A8h] [ebp-74h]
    float v61; // [esp+1ACh] [ebp-70h]
    float v62; // [esp+1B0h] [ebp-6Ch]
    float v63; // [esp+1B4h] [ebp-68h]
    float v64; // [esp+1B8h] [ebp-64h]
    float v65; // [esp+1BCh] [ebp-60h]
    float v66; // [esp+1C0h] [ebp-5Ch]
    float v67; // [esp+1C4h] [ebp-58h]
    float v68; // [esp+1C8h] [ebp-54h]
    float v69; // [esp+1CCh] [ebp-50h]
    __int16 *v70; // [esp+1D0h] [ebp-4Ch]
    float4 frameVec; // [esp+1D4h] [ebp-48h]
    const __int16 *rotLastFrame; // [esp+1E8h] [ebp-34h]
    int useSmallIndices; // [esp+1ECh] [ebp-30h]
    __int16 *dataShort; // [esp+1F0h] [ebp-2Ch]
    DObjAnimMat *totalRotTrans; // [esp+1F4h] [ebp-28h]
    unsigned int animPartIndex; // [esp+1F8h] [ebp-24h]
    unsigned __int8 *dataByte; // [esp+1FCh] [ebp-20h]
    int *randomDataInt; // [esp+200h] [ebp-1Ch]
    unsigned int size; // [esp+204h] [ebp-18h]
    int *dataInt; // [esp+208h] [ebp-14h]
    __int16 *randomDataShort; // [esp+20Ch] [ebp-10h]
    unsigned __int8 *randomDataByte; // [esp+210h] [ebp-Ch]
    unsigned int tableSize; // [esp+214h] [ebp-8h]
    int modelPartIndex; // [esp+218h] [ebp-4h]

    if (parts->bLoop)
        MyAssertHandler(".\\xanim\\xanim_calc.cpp", 300, 0, "%s", "!parts->bLoop");
    if (parts->dataByte)
        v9 = parts->dataByte;
    else
        v9 = 0;
    dataByte = v9;
    if (parts->dataShort)
        v8 = parts->dataShort;
    else
        v8 = 0;
    dataShort = v8;
    if (parts->dataInt)
        v7 = parts->dataInt;
    else
        v7 = 0;
    dataInt = v7;
    randomDataByte = parts->randomDataByte;
    randomDataShort = parts->randomDataShort;
    randomDataInt = parts->randomDataInt;
    useSmallIndices = parts->numframes < 0x100u;
    animPartIndex = 0;
    size = parts->boneCount[0];
    while (animPartIndex < size)
    {
        modelPartIndex = animToModel[animPartIndex];
        if ((unsigned int)modelPartIndex >= 0x80)
        {
            MyAssertHandler(
                ".\\xanim\\xanim_calc.cpp",
                322,
                0,
                "modelPartIndex doesn't index DOBJ_MAX_PARTS\n\t%i not in [0, %i)",
                modelPartIndex,
                128);
            MyAssertHandler("c:\\trees\\cod3\\src\\xanim\\../qcommon/bitarray.h", 66, 0, "%s", "pos < BIT_COUNT");
        }
        if ((ignorePartBits->array[(unsigned int)modelPartIndex >> 5] & (0x80000000 >> (modelPartIndex & 0x1F))) == 0)
        {
            totalRotTrans = &rotTransArray[modelPartIndex];
            totalRotTrans->quat[3] = totalRotTrans->quat[3] + weightScale;
        }
        ++animPartIndex;
    }
    size += parts->boneCount[1];
    while (animPartIndex < size)
    {
        modelPartIndex = animToModel[animPartIndex];
        if ((unsigned int)modelPartIndex >= 0x80)
            MyAssertHandler(
                ".\\xanim\\xanim_calc.cpp",
                461,
                0,
                "modelPartIndex doesn't index DOBJ_MAX_PARTS\n\t%i not in [0, %i)",
                modelPartIndex,
                128);
        tableSize = (unsigned __int16)*dataShort++;
        if (useSmallIndices)
        {
            dataByte += tableSize + 1;
        }
        else if (tableSize >= 0x40)
        {
            v19 = ((tableSize - 1) >> 8) + 1;
            dataShort += v19 + 1;
        }
        else
        {
            dataShort += tableSize + 1;
        }
        if ((unsigned int)modelPartIndex >= 0x80)
            MyAssertHandler("c:\\trees\\cod3\\src\\xanim\\../qcommon/bitarray.h", 66, 0, "%s", "pos < BIT_COUNT");
        if ((ignorePartBits->array[(unsigned int)modelPartIndex >> 5] & (0x80000000 >> (modelPartIndex & 0x1F))) == 0)
        {
            rotLastFrame = &randomDataShort[2 * tableSize];
            frameVec.v[0] = 0.0;
            frameVec.v[1] = 0.0;
            frameVec.v[2] = (double)*rotLastFrame * 0.00003051850944757462;
            frameVec.v[3] = (double)rotLastFrame[1] * 0.00003051850944757462;
            start = rotTransArray[modelPartIndex].quat;
            dir[0] = 0.0;
            dir[1] = 0.0;
            v18 = *(_QWORD *)&frameVec.unitVec[2].packed;
            Vec4Mad(start, weightScale, dir, start);
        }
        ++animPartIndex;
        randomDataShort += 2 * tableSize + 2;
    }
    size += parts->boneCount[2];
    while (animPartIndex < size)
    {
        modelPartIndex = animToModel[animPartIndex];
        if ((unsigned int)modelPartIndex >= 0x80)
            MyAssertHandler(
                ".\\xanim\\xanim_calc.cpp",
                487,
                0,
                "modelPartIndex doesn't index DOBJ_MAX_PARTS\n\t%i not in [0, %i)",
                modelPartIndex,
                128);
        tableSize = (unsigned __int16)*dataShort++;
        if (useSmallIndices)
        {
            dataByte += tableSize + 1;
        }
        else if (tableSize >= 0x40)
        {
            v15 = ((tableSize - 1) >> 8) + 1;
            dataShort += v15 + 1;
        }
        else
        {
            dataShort += tableSize + 1;
        }
        if ((unsigned int)modelPartIndex >= 0x80)
            MyAssertHandler("c:\\trees\\cod3\\src\\xanim\\../qcommon/bitarray.h", 66, 0, "%s", "pos < BIT_COUNT");
        if ((ignorePartBits->array[(unsigned int)modelPartIndex >> 5] & (0x80000000 >> (modelPartIndex & 0x1F))) == 0)
        {
            v70 = &randomDataShort[4 * tableSize];
            v66 = (double)*v70 * 0.00003051850944757462;
            v67 = (double)v70[1] * 0.00003051850944757462;
            v68 = (double)v70[2] * 0.00003051850944757462;
            v69 = (double)v70[3] * 0.00003051850944757462;
            result = rotTransArray[modelPartIndex].quat;
            v14[0] = v66;
            v14[1] = v67;
            v14[2] = v68;
            v14[3] = v69;
            Vec4Mad(result, weightScale, v14, result);
        }
        ++animPartIndex;
        randomDataShort += 4 * tableSize + 4;
    }
    size += parts->boneCount[3];
    while (animPartIndex < size)
    {
        modelPartIndex = animToModel[animPartIndex];
        if ((unsigned int)modelPartIndex >= 0x80)
            MyAssertHandler(
                ".\\xanim\\xanim_calc.cpp",
                512,
                0,
                "modelPartIndex doesn't index DOBJ_MAX_PARTS\n\t%i not in [0, %i)",
                modelPartIndex,
                128);
        if ((unsigned int)modelPartIndex >= 0x80)
            MyAssertHandler("c:\\trees\\cod3\\src\\xanim\\../qcommon/bitarray.h", 66, 0, "%s", "pos < BIT_COUNT");
        if ((ignorePartBits->array[(unsigned int)modelPartIndex >> 5] & (0x80000000 >> (modelPartIndex & 0x1F))) == 0)
        {
            v62 = 0.0;
            v63 = 0.0;
            v64 = (double)*dataShort * 0.00003051850944757462;
            v65 = (double)dataShort[1] * 0.00003051850944757462;
            quat = rotTransArray[modelPartIndex].quat;
            v12[0] = 0.0;
            v12[1] = 0.0;
            v12[2] = v64;
            v12[3] = v65;
            Vec4Mad(quat, weightScale, v12, quat);
        }
        ++animPartIndex;
        dataShort += 2;
    }
    size += parts->boneCount[4];
    while (animPartIndex < size)
    {
        modelPartIndex = animToModel[animPartIndex];
        if ((unsigned int)modelPartIndex >= 0x80)
            MyAssertHandler(
                ".\\xanim\\xanim_calc.cpp",
                531,
                0,
                "modelPartIndex doesn't index DOBJ_MAX_PARTS\n\t%i not in [0, %i)",
                modelPartIndex,
                128);
        if ((unsigned int)modelPartIndex >= 0x80)
            MyAssertHandler("c:\\trees\\cod3\\src\\xanim\\../qcommon/bitarray.h", 66, 0, "%s", "pos < BIT_COUNT");
        if ((ignorePartBits->array[(unsigned int)modelPartIndex >> 5] & (0x80000000 >> (modelPartIndex & 0x1F))) == 0)
        {
            v58 = (double)*dataShort * 0.00003051850944757462;
            v59 = (double)dataShort[1] * 0.00003051850944757462;
            v60 = (double)dataShort[2] * 0.00003051850944757462;
            v61 = (double)dataShort[3] * 0.00003051850944757462;
            v10[0] = v58;
            v10[1] = v59;
            v10[2] = v60;
            v10[3] = v61;
            Vec4Mad(rotTransArray[modelPartIndex].quat, weightScale, v10, rotTransArray[modelPartIndex].quat);
        }
        ++animPartIndex;
        dataShort += 4;
    }
    animPartIndex = 0;
    size = parts->boneCount[5];
    while (animPartIndex < size)
    {
        modelPartIndex = animToModel[*dataByte++];
        if ((unsigned int)modelPartIndex >= 0x80)
            MyAssertHandler(
                ".\\xanim\\xanim_calc.cpp",
                558,
                0,
                "modelPartIndex doesn't index DOBJ_MAX_PARTS\n\t%i not in [0, %i)",
                modelPartIndex,
                128);
        tableSize = (unsigned __int16)*dataShort++;
        if (useSmallIndices)
        {
            dataByte += tableSize + 1;
        }
        else if (tableSize >= 0x40)
        {
            dataShort += ((tableSize - 1) >> 8) + 2;
        }
        else
        {
            dataShort += tableSize + 1;
        }
        if ((unsigned int)modelPartIndex >= 0x80)
            MyAssertHandler("c:\\trees\\cod3\\src\\xanim\\../qcommon/bitarray.h", 66, 0, "%s", "pos < BIT_COUNT");
        if ((ignorePartBits->array[(unsigned int)modelPartIndex >> 5] & (0x80000000 >> (modelPartIndex & 0x1F))) == 0)
        {
            totalRotTrans = &rotTransArray[modelPartIndex];
            posVec = *(float4 *)totalRotTrans->trans;
            frame._1 = (unsigned __int8 (*)[3]) & randomDataByte[3 * tableSize];
            lerp.v[0] = (float)*(unsigned __int8 *)frame._1;
            lerp.v[1] = (float)(*frame._1)[1];
            lerp.v[2] = (float)(*frame._1)[2];
            lerp.v[3] = 0.0;
            minsVec.v[0] = *(float *)dataInt;
            minsVec.v[1] = *((float *)dataInt + 1);
            minsVec.v[2] = *((float *)dataInt + 2);
            minsVec.v[3] = 0.0;
            sizeVec.v[0] = *((float *)dataInt + 3);
            sizeVec.v[1] = *((float *)dataInt + 4);
            sizeVec.v[2] = *((float *)dataInt + 5);
            sizeVec.v[3] = 0.0;
            v51 = sizeVec.v[0] * lerp.v[0] + minsVec.v[0];
            v52 = sizeVec.v[1] * lerp.v[1] + minsVec.v[1];
            v53 = sizeVec.v[2] * lerp.v[2] + minsVec.v[2];
            v54 = (float)0.0 * (float)0.0 + (float)0.0;
            posVec.v[0] = weightScale * v51 + posVec.v[0];
            posVec.v[1] = weightScale * v52 + posVec.v[1];
            posVec.v[2] = weightScale * v53 + posVec.v[2];
            posVec.v[3] = weightScale * v54 + posVec.v[3];
            *(float4 *)totalRotTrans->trans = posVec;
            totalRotTrans->transWeight = totalRotTrans->transWeight + weightScale;
        }
        ++animPartIndex;
        dataInt += 6;
        randomDataByte += 3 * tableSize + 3;
    }
    size += parts->boneCount[6];
    while (animPartIndex < size)
    {
        modelPartIndex = animToModel[*dataByte++];
        if ((unsigned int)modelPartIndex >= 0x80)
            MyAssertHandler(
                ".\\xanim\\xanim_calc.cpp",
                596,
                0,
                "modelPartIndex doesn't index DOBJ_MAX_PARTS\n\t%i not in [0, %i)",
                modelPartIndex,
                128);
        tableSize = (unsigned __int16)*dataShort++;
        if (useSmallIndices)
        {
            dataByte += tableSize + 1;
        }
        else if (tableSize >= 0x40)
        {
            dataShort += ((tableSize - 1) >> 8) + 2;
        }
        else
        {
            dataShort += tableSize + 1;
        }
        if ((unsigned int)modelPartIndex >= 0x80)
            MyAssertHandler("c:\\trees\\cod3\\src\\xanim\\../qcommon/bitarray.h", 66, 0, "%s", "pos < BIT_COUNT");
        if ((ignorePartBits->array[(unsigned int)modelPartIndex >> 5] & (0x80000000 >> (modelPartIndex & 0x1F))) == 0)
        {
            totalRotTrans = &rotTransArray[modelPartIndex];
            v37 = totalRotTrans->trans[0];
            v38 = totalRotTrans->trans[1];
            v39 = totalRotTrans->trans[2];
            transWeight = totalRotTrans->transWeight;
            v28 = &randomDataShort[3 * tableSize];
            v41 = (float)(unsigned __int16)*v28;
            v42 = (float)(unsigned __int16)v28[1];
            v43 = (float)(unsigned __int16)v28[2];
            v44 = 0.0;
            v45 = *(float *)dataInt;
            v46 = *((float *)dataInt + 1);
            v47 = *((float *)dataInt + 2);
            v48 = 0.0;
            v29 = *((float *)dataInt + 3);
            v30 = *((float *)dataInt + 4);
            v31 = *((float *)dataInt + 5);
            v32 = 0.0;
            v33 = v29 * v41 + v45;
            v34 = v30 * v42 + v46;
            v35 = v31 * v43 + v47;
            v36 = (float)0.0 * (float)0.0 + (float)0.0;
            v37 = weightScale * v33 + v37;
            v38 = weightScale * v34 + v38;
            v39 = weightScale * v35 + v39;
            transWeight = weightScale * v36 + transWeight;
            trans = totalRotTrans->trans;
            totalRotTrans->trans[0] = v37;
            trans[1] = v38;
            trans[2] = v39;
            trans[3] = transWeight;
            totalRotTrans->transWeight = totalRotTrans->transWeight + weightScale;
        }
        ++animPartIndex;
        dataInt += 6;
        randomDataShort += 3 * tableSize + 3;
    }
    size += parts->boneCount[7];
    while (animPartIndex < size)
    {
        modelPartIndex = animToModel[*dataByte];
        if ((unsigned int)modelPartIndex >= 0x80)
            MyAssertHandler(
                ".\\xanim\\xanim_calc.cpp",
                629,
                0,
                "modelPartIndex doesn't index DOBJ_MAX_PARTS\n\t%i not in [0, %i)",
                modelPartIndex,
                128);
        if ((unsigned int)modelPartIndex >= 0x80)
            MyAssertHandler("c:\\trees\\cod3\\src\\xanim\\../qcommon/bitarray.h", 66, 0, "%s", "pos < BIT_COUNT");
        if ((ignorePartBits->array[(unsigned int)modelPartIndex >> 5] & (0x80000000 >> (modelPartIndex & 0x1F))) == 0)
        {
            totalRotTrans = &rotTransArray[modelPartIndex];
            v24 = totalRotTrans->trans[0];
            v25 = totalRotTrans->trans[1];
            v26 = totalRotTrans->trans[2];
            v27 = totalRotTrans->transWeight;
            v20 = *(float *)dataInt;
            v21 = *((float *)dataInt + 1);
            v22 = *((float *)dataInt + 2);
            v23 = 0.0;
            v24 = weightScale * v20 + v24;
            v25 = weightScale * v21 + v25;
            v26 = weightScale * v22 + v26;
            v27 = weightScale * (float)0.0 + v27;
            v6 = totalRotTrans->trans;
            totalRotTrans->trans[0] = v24;
            v6[1] = v25;
            v6[2] = v26;
            v6[3] = v27;
            totalRotTrans->transWeight = totalRotTrans->transWeight + weightScale;
        }
        ++animPartIndex;
        ++dataByte;
        dataInt += 3;
    }
    size += parts->boneCount[8];
    while (animPartIndex < size)
    {
        modelPartIndex = animToModel[*dataByte];
        if ((unsigned int)modelPartIndex >= 0x80)
            MyAssertHandler(
                ".\\xanim\\xanim_calc.cpp",
                649,
                0,
                "modelPartIndex doesn't index DOBJ_MAX_PARTS\n\t%i not in [0, %i)",
                modelPartIndex,
                128);
        if ((unsigned int)modelPartIndex >= 0x80)
            MyAssertHandler("c:\\trees\\cod3\\src\\xanim\\../qcommon/bitarray.h", 66, 0, "%s", "pos < BIT_COUNT");
        if ((ignorePartBits->array[(unsigned int)modelPartIndex >> 5] & (0x80000000 >> (modelPartIndex & 0x1F))) == 0)
        {
            totalRotTrans = &rotTransArray[modelPartIndex];
            totalRotTrans->transWeight = totalRotTrans->transWeight + weightScale;
        }
        ++animPartIndex;
        ++dataByte;
    }
}

DObjAnimMat *__cdecl XAnimGetCalcBuffer(XAnimCalcAnimInfo *info, const DObj_s *obj, int *rotTransArrayIndex)
{
    DObjAnimMat *calcBuffer; // [esp+0h] [ebp-8h]
    int index; // [esp+4h] [ebp-4h]

    calcBuffer = &info->rotTransArray[*rotTransArrayIndex];
    index = *rotTransArrayIndex + obj->numBones;
    if (index <= 768)
    {
        *rotTransArrayIndex = index;
        return calcBuffer;
    }
    else
    {
        Com_PrintWarning(19, "max calculation buffer exceeded\n");
        DObjDisplayAnim(obj, "");
        return 0;
    }
}

void __cdecl XAnimScaleRotTransArray(int numBones, const XAnimCalcAnimInfo *info, DObjAnimMat *rotTransArray)
{
    float r; // [esp+8h] [ebp-8h]
    unsigned int i; // [esp+Ch] [ebp-4h]

    for (i = 0; (int)i < numBones; ++i)
    {
        if (i >= 0x80)
            MyAssertHandler("c:\\trees\\cod3\\src\\xanim\\../qcommon/bitarray.h", 66, 0, "%s", "pos < BIT_COUNT");
        if ((info->ignorePartBits.array[i >> 5] & (0x80000000 >> (i & 0x1F))) == 0 && rotTransArray->transWeight != 0.0)
        {
            r = 1.0 / rotTransArray->transWeight;
            Vec4Scale(rotTransArray->quat, r, rotTransArray->quat);
            Vec3Scale(rotTransArray->trans, r, rotTransArray->trans);
        }
        ++rotTransArray;
    }
}

void __cdecl XAnimNormalizeRotScaleTransArray(
    int numBones,
    const XAnimCalcAnimInfo *info,
    float weightScale,
    DObjAnimMat *rotTransArray)
{
    float v4; // [esp+8h] [ebp-24h]
    float v5; // [esp+Ch] [ebp-20h]
    int r; // [esp+24h] [ebp-8h]
    unsigned int i; // [esp+28h] [ebp-4h]

    for (i = 0; (int)i < numBones; ++i)
    {
        if (i >= 0x80)
            MyAssertHandler("c:\\trees\\cod3\\src\\xanim\\../qcommon/bitarray.h", 66, 0, "%s", "pos < BIT_COUNT");
        if ((info->ignorePartBits.array[i >> 5] & (0x80000000 >> (i & 0x1F))) == 0)
        {
            *(float *)&r = Vec4LengthSq(rotTransArray->quat);
            if (*(float *)&r != 0.0)
            {
                v5 = I_rsqrt(r) * weightScale;
                Vec4Scale(rotTransArray->quat, v5, rotTransArray->quat);
            }
            if (rotTransArray->transWeight != 0.0)
            {
                v4 = weightScale / rotTransArray->transWeight;
                Vec3Scale(rotTransArray->trans, v4, rotTransArray->trans);
                rotTransArray->transWeight = weightScale;
            }
        }
        ++rotTransArray;
    }
}

void __cdecl XAnimMadRotTransArray(
    int numBones,
    const XAnimCalcAnimInfo *info,
    float weightScale,
    const DObjAnimMat *rotTrans,
    DObjAnimMat *totalRotTrans)
{
    float v5; // [esp+Ch] [ebp-24h]
    float v6; // [esp+10h] [ebp-20h]
    int r; // [esp+28h] [ebp-8h]
    unsigned int i; // [esp+2Ch] [ebp-4h]

    for (i = 0; (int)i < numBones; ++i)
    {
        if (i >= 0x80)
            MyAssertHandler("c:\\trees\\cod3\\src\\xanim\\../qcommon/bitarray.h", 66, 0, "%s", "pos < BIT_COUNT");
        if ((info->ignorePartBits.array[i >> 5] & (0x80000000 >> (i & 0x1F))) == 0)
        {
            *(float *)&r = Vec4LengthSq(rotTrans->quat);
            if (*(float *)&r != 0.0)
            {
                v6 = I_rsqrt(r) * weightScale;
                Vec4Mad(totalRotTrans->quat, v6, rotTrans->quat, totalRotTrans->quat);
            }
            if (rotTrans->transWeight != 0.0)
            {
                v5 = weightScale / rotTrans->transWeight;
                Vec3Mad(totalRotTrans->trans, v5, rotTrans->trans, totalRotTrans->trans);
                totalRotTrans->transWeight = totalRotTrans->transWeight + weightScale;
            }
        }
        ++rotTrans;
        ++totalRotTrans;
    }
}

void __cdecl XAnimApplyAdditives(
    DObjAnimMat *rotTransArray,
    DObjAnimMat *additiveArray,
    float weight,
    int boneCount,
    XAnimCalcAnimInfo *info)
{
    float scale; // [esp+Ch] [ebp-28h]
    float v6; // [esp+10h] [ebp-24h]
    DObjAnimMat *v7; // [esp+14h] [ebp-20h]
    float r; // [esp+18h] [ebp-1Ch]
    float ra; // [esp+18h] [ebp-1Ch]
    unsigned int i; // [esp+1Ch] [ebp-18h]
    float rot[4]; // [esp+20h] [ebp-14h] BYREF
    const bitarray<128> *ignorePartBits; // [esp+30h] [ebp-4h]

    ignorePartBits = &info->ignorePartBits;
    for (i = 0; (int)i < boneCount; ++i)
    {
        if (i >= 0x80)
            MyAssertHandler("c:\\trees\\cod3\\src\\xanim\\../qcommon/bitarray.h", 66, 0, "%s", "pos < BIT_COUNT");
        if ((ignorePartBits->array[i >> 5] & (0x80000000 >> (i & 0x1F))) == 0)
        {
            r = Vec4LengthSq(additiveArray[i].quat);
            if (r != 0.0)
            {
                v6 = sqrt(r);
                scale = 1.0 / v6;
                Vec4Scale(additiveArray[i].quat, scale, additiveArray[i].quat);
                ra = v6 * weight;
                Vec3Scale(additiveArray[i].quat, ra, additiveArray[i].quat);
                additiveArray[i].quat[3] = ra * additiveArray[i].quat[3] + (1.0 - ra) * 1.0;
                QuatMultiply(additiveArray[i].quat, rotTransArray[i].quat, rot);
                v7 = &rotTransArray[i];
                v7->quat[0] = rot[0];
                v7->quat[1] = rot[1];
                v7->quat[2] = rot[2];
                v7->quat[3] = rot[3];
            }
            Vec3Mad(rotTransArray[i].trans, weight, additiveArray[i].trans, rotTransArray[i].trans);
        }
    }
}

void __cdecl XAnim_CalcRotDeltaEntire(const XAnimDeltaPart *animDelta, float *rotDelta)
{
    XAnimDeltaPartQuat *rotFrameDeltas; // [esp+8h] [ebp-8h]
    const __int16 *rotDeltaLastFrame; // [esp+Ch] [ebp-4h]

    if (animDelta->quat)
    {
        rotFrameDeltas = animDelta->quat;
        if (rotFrameDeltas->size)
            rotDeltaLastFrame = rotFrameDeltas->u.frames.frames[rotFrameDeltas->size];
        else
            rotDeltaLastFrame = (const __int16 *)&rotFrameDeltas->u;
        *rotDelta = (float)*rotDeltaLastFrame;
        rotDelta[1] = (float)rotDeltaLastFrame[1];
    }
    else
    {
        *rotDelta = 0.0;
        rotDelta[1] = 32767.0;
    }
}

void __cdecl XAnimWeightedAccumLerpedTrans(
    float4 fromVec,
    float4 toVec,
    float keyFrameLerpFrac,
    float weightScale,
    const float *dataInt,
    DObjAnimMat *totalRotTrans)
{
    float frameVec[3]; // [esp+10h] [ebp-20h] BYREF
    float lerp[5]; // [esp+1Ch] [ebp-14h] BYREF

    Vec3Lerp(fromVec.v, toVec.v, keyFrameLerpFrac, lerp);
    LODWORD(lerp[3]) = dataInt;
    Vec3Accum(dataInt, dataInt + 3, lerp, frameVec);
    Vec3Mad(totalRotTrans->trans, weightScale, frameVec, totalRotTrans->trans);
    totalRotTrans->transWeight = totalRotTrans->transWeight + weightScale;
}

void __cdecl XAnimWeightedAccumTrans(float weightScale, int *dataInt, DObjAnimMat *totalRotTrans)
{
    Vec3Mad(totalRotTrans->trans, weightScale, (const float *)dataInt, totalRotTrans->trans);
    totalRotTrans->transWeight = totalRotTrans->transWeight + weightScale;
}

