#include "xanim_calc.h"
#include <universal/q_shared.h>
#include <qcommon/qcommon.h>
#include <universal/profile.h>

void __cdecl Short2LerpAsVec2(const __int16 *from, const __int16 *to, float frac, float *out)
{
    out[0] = frac * (to[0] - from[0]) + from[0];
    out[1] = frac * (to[1] - from[1]) + from[1];
}

void __cdecl XAnim_SetTime(float time, int frameCount, XAnimTime *animTime);

template <typename T>
void __cdecl XAnim_GetTimeIndex(
    const XAnimTime *animTime,
    const T*indices,
    int tableSize,
    int *keyFrameIndex,
    float *keyFrameLerpFrac);

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
    const XAnimTree_s *tree; // [esp+20h] [ebp-20h]
    unsigned int secondInfoIndex; // [esp+24h] [ebp-1Ch]
    DObjAnimMat *calcBuffer; // [esp+28h] [ebp-18h]
    unsigned int firstInfoIndex; // [esp+2Ch] [ebp-14h]
    bool secondChildFound; // [esp+32h] [ebp-Eh]
    bool additiveChildExists; // [esp+33h] [ebp-Dh]
    int allocedCalcBuffer; // [esp+34h] [ebp-Ch]
    float weight; // [esp+38h] [ebp-8h]
    float firstWeight; // [esp+3Ch] [ebp-4h]

    tree = obj->tree;
    iassert(tree);
    iassert(tree->anims);
    iassert(info->tree == tree);
    iassert(info->inuse);

    if (info->animToModel)
    {
        if (bClear)
            XAnimClearRotTransArray(obj, rotTransArray, animInfo);
        XAnimCalcLeaf(info, weightScale, rotTransArray, animInfo);
    }
    else
    {
        firstWeight = 0.0f;
        firstInfo = 0;
        for (firstInfoIndex = info->children; firstInfoIndex; firstInfoIndex = firstInfo->next)
        {
            firstInfo = GetAnimInfo(firstInfoIndex);
            iassert(firstInfo->tree == tree);
            iassert(firstInfo->inuse);
            firstWeight = firstInfo->state.weight;
            if (firstWeight != 0.0f)
            {
                if (IsInfoAdditive(firstInfo))
                    firstInfoIndex = 0;
                break;
            }
        }
        if (firstInfoIndex)
        {
            iassert(firstInfo);
            secondChildFound = 0;
            calcBuffer = 0;
            allocedCalcBuffer = 0;
            for (secondInfoIndex = firstInfo->next; secondInfoIndex; secondInfoIndex = secondInfo->next)
            {
                secondInfo = GetAnimInfo(secondInfoIndex);
                iassert(secondInfo->tree == tree);
                iassert(secondInfo->inuse);
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
                            XAnimCalc(obj, firstInfo, firstWeight, 1, 1, animInfo, rotTransArrayIndex, rotTransArray); // LWSS: add from blops.
                        }
                        else
                        {
                            calcBuffer = XAnimGetCalcBuffer(animInfo, obj, &rotTransArrayIndex);
                            if (!calcBuffer)
                                return;
                            allocedCalcBuffer = 1;
                            XAnimCalc(obj, firstInfo, firstWeight, 1, 1, animInfo, rotTransArrayIndex, calcBuffer); // LWSS: add from blops.
                        }
                        XAnimCalc(obj, firstInfo, firstWeight, 1, 1, animInfo, rotTransArrayIndex, calcBuffer);
                    }
                    iassert(calcBuffer);
                    XAnimCalc(obj, secondInfo, weight, 0, 1, animInfo, rotTransArrayIndex, calcBuffer);
                }
            }
            if (secondChildFound)
            {
                iassert(bNormQuat || bClear);
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
                        secondInfo = GetAnimInfo(secondInfoIndex);
                        iassert(secondInfo->tree == tree);
                        iassert(secondInfo->inuse);
                        iassert(IsInfoAdditive(secondInfo));
                        weight = secondInfo->state.weight;
                        if (weight != 0.0f)
                        {
                            XAnimCalc(obj, secondInfo, weight, !additiveChildExists, 1, animInfo, rotTransArrayIndex, calcBuffer);
                            additiveChildExists = 1;
                        }
                        secondInfoIndex = secondInfo->next;
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
        if (!info->ignorePartBits.testBit(modelPartIndex))
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

template <typename T>
void __cdecl XAnimCalcParts(
    const XAnimParts *parts,
    const unsigned __int8 *animToModel,
    float time,
    float weightScale,
    DObjAnimMat *rotTransArray,
    const bitarray<128> *ignorePartBits)
{
    unsigned __int64 v6; // kr00_8
    unsigned __int64 v7; // rax

    float scale; // [esp+4Ch] [ebp-254h]
    float v13; // [esp+50h] [ebp-250h]
    int *v14; // [esp+A4h] [ebp-1FCh]
    __int16 *v15; // [esp+A8h] [ebp-1F8h]
    unsigned __int8 *v16; // [esp+ACh] [ebp-1F4h]
    char v17; // [esp+B7h] [ebp-1E9h]
    const unsigned __int8 *v18; // [esp+B8h] [ebp-1E8h]
    char v19; // [esp+C3h] [ebp-1DDh]
    const unsigned __int8 *v20; // [esp+C4h] [ebp-1DCh]
    float v21[7]; // [esp+CCh] [ebp-1D4h] BYREF
    int v22; // [esp+E8h] [ebp-1B8h]
    float v23; // [esp+ECh] [ebp-1B4h]
    int v24; // [esp+F0h] [ebp-1B0h]
    float v25; // [esp+F4h] [ebp-1ACh]
    int v26; // [esp+F8h] [ebp-1A8h]
    float v27; // [esp+FCh] [ebp-1A4h]
    float v28; // [esp+100h] [ebp-1A0h]
    int v29; // [esp+104h] [ebp-19Ch]
    unsigned __int64 v30; // [esp+108h] [ebp-198h]
    float v[4]; // [esp+110h] [ebp-190h] BYREF
    float v32; // [esp+120h] [ebp-180h]
    float *v33; // [esp+124h] [ebp-17Ch]
    float v34[5]; // [esp+128h] [ebp-178h] BYREF
    float *quat; // [esp+13Ch] [ebp-164h]
    float dir[4] = { 0.0f, 0.0f, 0.0f, 0.0f }; // [esp+140h] [ebp-160h] BYREF
    __int64 v37; // [esp+148h] [ebp-158h]
    float v38; // [esp+154h] [ebp-14Ch]
    float *result; // [esp+158h] [ebp-148h]
    float v40; // [esp+15Ch] [ebp-144h]
    float v41; // [esp+160h] [ebp-140h]
    char v42; // [esp+167h] [ebp-139h]
    unsigned __int8 *v43; // [esp+168h] [ebp-138h]
    float v44; // [esp+16Ch] [ebp-134h]
    float *start; // [esp+170h] [ebp-130h]
    float scale1; // [esp+174h] [ebp-12Ch]
    float scale0; // [esp+178h] [ebp-128h]
    char v48; // [esp+17Fh] [ebp-121h]
    unsigned __int8 *v49; // [esp+180h] [ebp-120h]
    int v50; // [esp+184h] [ebp-11Ch] BYREF
    unsigned __int16 *v51; // [esp+188h] [ebp-118h]
    //__int64 v52; // [esp+18Ch] [ebp-114h]
    __int16 v52[4];
    float v53; // [esp+194h] [ebp-10Ch]
    float v54; // [esp+198h] [ebp-108h]
    float lerpFrac2; // [esp+19Ch] [ebp-104h] BYREF
    //__int64 v56; // [esp+1A0h] [ebp-100h]
    __int16 v56[4];
    float v57; // [esp+1A8h] [ebp-F8h]
    float v58; // [esp+1ACh] [ebp-F4h]
    int v59; // [esp+1B0h] [ebp-F0h] BYREF
    unsigned __int8 *v60; // [esp+1B4h] [ebp-ECh]
    //__int64 v61; // [esp+1B8h] [ebp-E8h]
    __int16 v61[4];
    float v62; // [esp+1C0h] [ebp-E0h]
    float v63; // [esp+1C4h] [ebp-DCh]
    float lerpFrac; // [esp+1C8h] [ebp-D8h] BYREF
    //__int64 v65; // [esp+1CCh] [ebp-D4h]
    __int16 v65[4];
    float v66; // [esp+1D4h] [ebp-CCh]
    float v67; // [esp+1D8h] [ebp-C8h]
    float v68; // [esp+1DCh] [ebp-C4h]
    float v69; // [esp+1E0h] [ebp-C0h]
    float v70; // [esp+1E4h] [ebp-BCh]
    float v71; // [esp+1E8h] [ebp-B8h]
    float v72; // [esp+1ECh] [ebp-B4h]
    float v73; // [esp+1F0h] [ebp-B0h]
    float v74; // [esp+1F4h] [ebp-ACh]
    float v75; // [esp+1F8h] [ebp-A8h]
    float4 frameVec; // [esp+1FCh] [ebp-A4h]
    int v77; // [esp+20Ch] [ebp-94h] BYREF
    __int16 *v78; // [esp+210h] [ebp-90h]
    float dir1[4]; // [esp+214h] [ebp-8Ch] BYREF
    float v80; // [esp+224h] [ebp-7Ch] BYREF
    float dir0[4]; // [esp+228h] [ebp-78h] BYREF
    int keyFrameIndex; // [esp+238h] [ebp-68h] BYREF
    const __int16 *frame; // [esp+23Ch] [ebp-64h]
    float4 toVec; // [esp+240h] [ebp-60h] BYREF
    float keyFrameLerpFrac; // [esp+250h] [ebp-50h] BYREF
    float4 fromVec; // [esp+254h] [ebp-4Ch] BYREF
    __int16 *dataShort; // [esp+268h] [ebp-38h]
    XAnimTime animTime; // [esp+26Ch] [ebp-34h] BYREF
    unsigned int animPartIndex; // [esp+278h] [ebp-28h]
    unsigned __int8 *dataByte; // [esp+27Ch] [ebp-24h]
    int *randomDataInt; // [esp+280h] [ebp-20h]
    unsigned int size; // [esp+284h] [ebp-1Ch]
    int *dataInt; // [esp+288h] [ebp-18h]
    __int16 *randomDataShort; // [esp+28Ch] [ebp-14h]
    unsigned __int8 *randomDataByte; // [esp+290h] [ebp-10h]
    unsigned int tableSize; // [esp+294h] [ebp-Ch]
    T *indices; // [esp+298h] [ebp-8h]
    int modelPartIndex; // [esp+29Ch] [ebp-4h]

    iassert(parts->numframes);
    iassert(time >= 0);
    iassert(time < 1.f);

    dataByte = parts->dataByte;
    dataShort = parts->dataShort;
    dataInt = parts->dataInt;
    randomDataByte = parts->randomDataByte;
    randomDataShort = parts->randomDataShort;
    randomDataInt = parts->randomDataInt;
    indices = (T*)parts->indices.data;
    XAnim_SetTime(time, parts->numframes, &animTime);
    animPartIndex = 0;
    size = parts->boneCount[0];

    while (animPartIndex < size)
    {
        modelPartIndex = animToModel[animPartIndex];
        iassert(modelPartIndex < DOBJ_MAX_PARTS);

        if (!ignorePartBits->testBit(modelPartIndex))
            rotTransArray[modelPartIndex].quat[3] = rotTransArray[modelPartIndex].quat[3] + weightScale;

        ++animPartIndex;
    }

    size += parts->boneCount[1];

    while (animPartIndex < size)
    {
        modelPartIndex = animToModel[animPartIndex];
        iassert(modelPartIndex < DOBJ_MAX_PARTS);
        tableSize = (unsigned short)*dataShort++;

        T *pIndices;

        if (tableSize >= 64 && sizeof(T) > 1)
        {
            pIndices = (T *)dataShort;
            int finalTableSize = ((tableSize - 1) >> 8) + 1;
            dataShort += finalTableSize + 1;
            if (ignorePartBits->testBit(modelPartIndex))
            {
                indices += tableSize + 1;
                v48 = 0;
                goto LABEL_45;
            }
            int tmpKeyframeIndex;
            XAnim_GetTimeIndex<T>(&animTime, pIndices, finalTableSize, &tmpKeyframeIndex, &animTime.time);
            tmpKeyframeIndex <<= 8;

            unsigned int keyframeDelta = CLAMP(tableSize - tmpKeyframeIndex, 0, 256);
            pIndices = &indices[tmpKeyframeIndex];
            XAnim_GetTimeIndex<T>(&animTime, pIndices, keyframeDelta, &keyFrameIndex, &keyFrameLerpFrac);
            keyFrameIndex += tmpKeyframeIndex;
            indices += tableSize + 1;
        }
        else
        {
            if constexpr (sizeof(T) == sizeof(unsigned char))
            {
                pIndices = (T *)dataByte;
                dataByte += tableSize + 1;
            }
            else
            {
                pIndices = (T *)dataShort;
                dataShort += tableSize + 1;
            }
            if (ignorePartBits->testBit(modelPartIndex))
            {
                v48 = 0;
                goto LABEL_45;
            }

            XAnim_GetTimeIndex<T>(&animTime, pIndices, tableSize, &keyFrameIndex, &keyFrameLerpFrac);
        }

        v48 = 1;
LABEL_45:
        if (v48)
        {
            frame = &randomDataShort[2 * keyFrameIndex];
            fromVec.v[0] = 0.0f;
            fromVec.v[1] = 0.0f;
            fromVec.v[2] = (float)frame[0] * (1.0f/32767.0f);
            fromVec.v[3] = (float)frame[1] * (1.0f/32767.0f);

            toVec.v[0] = 0.0f;
            toVec.v[1] = 0.0f;
            toVec.v[2] = (float)frame[2] * (1.0f/32767.0f);
            toVec.v[3] = (float)frame[3] * (1.0f/32767.0f);

            v44 = keyFrameLerpFrac;
            start = rotTransArray[modelPartIndex].quat;
            scale1 = weightScale * keyFrameLerpFrac;
            scale0 = weightScale - scale1;
            Vec4MadMad(start, scale0, fromVec.v, scale1, toVec.v, start);
        }
        ++animPartIndex;
        randomDataShort += 2 * tableSize + 2;
    }

    size += parts->boneCount[2];
    while (animPartIndex < size)
    {
        modelPartIndex = animToModel[animPartIndex];
        iassert(modelPartIndex < DOBJ_MAX_PARTS);
        tableSize = (unsigned short)*dataShort++;

        T *pIndices;

        if (tableSize >= 64 && sizeof(T) > 1)
        {
            pIndices = (T*)dataShort;
            int finalTableSize = ((tableSize - 1) >> 8) + 1;
            dataShort += finalTableSize + 1;

            if (ignorePartBits->testBit(modelPartIndex))
            {
                indices += tableSize + 1;
                v42 = 0;
                goto LABEL_67;
            }
            int tmpKeyIndex;
            XAnim_GetTimeIndex<T>(&animTime, pIndices, finalTableSize, &tmpKeyIndex, &animTime.time);
            tmpKeyIndex <<= 8;

            int tableDelta = CLAMP((tableSize - tmpKeyIndex), 0, 256);
            pIndices = &indices[tmpKeyIndex];
            XAnim_GetTimeIndex<T>(&animTime, pIndices, tableDelta, &v77, &v80);
            v77 += tmpKeyIndex;
            indices += tableSize + 1;
        }
        else
        {
            if constexpr (sizeof(T) == sizeof(unsigned char))
            {
                pIndices = (T *)dataByte;
                dataByte += tableSize + 1;
            }
            else
            {
                pIndices = (T *)dataShort;
                dataShort += tableSize + 1;
            }
            
            if (ignorePartBits->testBit(modelPartIndex))
            {
                v42 = 0;
                goto LABEL_67;
            }
            XAnim_GetTimeIndex<T>(&animTime, pIndices, tableSize, &v77, &v80);
        }
        v42 = 1;
LABEL_67:
        if (v42)
        {
            v78 = &randomDataShort[4 * v77];
            dir0[0] = v78[0] * (1.0f/32767.0f);
            dir0[1] = v78[1] * (1.0f/32767.0f);
            dir0[2] = v78[2] * (1.0f/32767.0f);
            dir0[3] = v78[3] * (1.0f/32767.0f);

            dir1[0] = v78[4] * (1.0f/32767.0f);
            dir1[1] = v78[5] * (1.0f/32767.0f);
            dir1[2] = v78[6] * (1.0f/32767.0f);
            dir1[3] = v78[7] * (1.0f/32767.0f);

            v38 = v80;
            result = rotTransArray[modelPartIndex].quat;
            v40 = weightScale * v80;
            v41 = weightScale - v40;
            Vec4MadMad(result, v41, dir0, v40, dir1, result);
        }
        ++animPartIndex;
        randomDataShort += 4 * tableSize + 4;
    }

    size += parts->boneCount[3];
    while (animPartIndex < size)
    {
        modelPartIndex = animToModel[animPartIndex];
        iassert(modelPartIndex < DOBJ_MAX_PARTS);

        if (!ignorePartBits->testBit(modelPartIndex))
        {
            frameVec.v[0] = 0.0;
            frameVec.v[1] = 0.0;
            frameVec.v[2] = dataShort[0] * (1.0f/32767.0f);
            frameVec.v[3] = dataShort[1] * (1.0f/32767.0f);
            quat = rotTransArray[modelPartIndex].quat;
            dir[0] = 0.0;
            dir[1] = 0.0;
            dir[2] = frameVec.v[2];
            dir[3] = frameVec.v[3];
            //v37 = *&frameVec.unitVec[2].packed;
            Vec4Mad(quat, weightScale, dir, quat);
        }
        ++animPartIndex;
        dataShort += 2;
    }

    size += parts->boneCount[4];
    while (animPartIndex < size)
    {
        modelPartIndex = animToModel[animPartIndex];
        iassert(modelPartIndex < DOBJ_MAX_PARTS);

        if (!ignorePartBits->testBit(modelPartIndex))
        {
            v72 = dataShort[0] * (1.0f/32767.0f);
            v73 = dataShort[1] * (1.0f/32767.0f);
            v74 = dataShort[2] * (1.0f/32767.0f);
            v75 = dataShort[3] * (1.0f/32767.0f);
            v33 = rotTransArray[modelPartIndex].quat;
            v34[0] = v72;
            v34[1] = v73;
            v34[2] = v74;
            v34[3] = v75;
            Vec4Mad(v33, weightScale, v34, v33);
        }
        ++animPartIndex;
        dataShort += 4;
    }

    // NOTE(mrsteyk): verify below claim
    iassert(animPartIndex >= size);
    // LWSS: it is not possible for this loop to go off (It's removed or optimized out in blops)
    //while (animPartIndex < size)
    //{
    //    modelPartIndex = animToModel[animPartIndex];
    //    if (modelPartIndex >= 0x80)
    //        MyAssertHandler(
    //            ".\\xanim\\xanim_calc.cpp",
    //            1010,
    //            0,
    //            "modelPartIndex doesn't index DOBJ_MAX_PARTS\n\t%i not in [0, %i)",
    //            modelPartIndex,
    //            128);
    //    if (modelPartIndex >= 0x80)
    //        MyAssertHandler("c:\\trees\\cod3\\src\\xanim\\../qcommon/bitarray.h", 66, 0, "%s", "pos < BIT_COUNT");
    //    if ((ignorePartBits->array[modelPartIndex >> 5] & (0x80000000 >> (modelPartIndex & 0x1F))) == 0)
    //    {
    //        LODWORD(v21[5]) = (uint32)dataShort;
    //        v6 = *dataShort << 32;
    //        v7 = dataShort[1] << 16;
    //        LODWORD(v30) = dataShort[2] | v7;
    //        HIDWORD(v30) = HIDWORD(v7) | HIDWORD(v6);
    //        v26 = v30 & 0x7FFF;
    //        LODWORD(v27) = -2 * (v30 & 0x4000);
    //        v[0] = (v27 - 3.0) * 256.015625;
    //        v24 = (v30 >> 15) & 0x7FFF;
    //        LODWORD(v25) = v24 + 1077936128 - 2 * ((v30 >> 15) & 0x4000);
    //        v[1] = (v25 - 3.0) * 256.015625;
    //        v22 = (v30 >> 30) & 0x7FFF;
    //        LODWORD(v23) = v22 + 1077936128 - 2 * ((v30 >> 30) & 0x4000);
    //        v[2] = (v23 - 3.0) * 256.015625;
    //        v[3] = 1.0;
    //        v32 = Vec4Length(v);
    //        if ((v30 & 0x800000000000LL) != 0)
    //            v13 = -1.0;
    //        else
    //            v13 = 1.0;
    //        v28 = v13;
    //        scale = v13 / v32;
    //        Vec4Scale(v, scale, v);
    //        v29 = (v30 >> 45) & 3;
    //        v68 = v[v29];
    //        v69 = v[(((v30 >> 45) & 3) + 1) & 3];
    //        v70 = v[(((v30 >> 45) & 3) + 2) & 3];
    //        v71 = v[(((v30 >> 45) & 3) + 3) & 3];
    //        v21[0] = v68;
    //        v21[1] = v69;
    //        v21[2] = v70;
    //        v21[3] = v71;
    //        Vec4Mad(rotTransArray[modelPartIndex].quat, weightScale, v21, rotTransArray[modelPartIndex].quat);
    //    }
    //    ++animPartIndex;
    //    dataShort += 3;
    //}

    animPartIndex = 0;
    size = parts->boneCount[5];
    while (animPartIndex < size)
    {
        modelPartIndex = animToModel[*dataByte++];
        iassert(modelPartIndex < DOBJ_MAX_PARTS);
        tableSize = (unsigned short)*dataShort++;

        T *pIndices;

        if (tableSize >= 64 && sizeof(T) > 1)
        {
            pIndices = (T *)dataShort;
            int finalTableSize = ((tableSize - 1) >> 8) + 1;
            dataShort += finalTableSize + 1;

            if (ignorePartBits->testBit(modelPartIndex))
            {
                indices += tableSize + 1;
                v19 = 0;
                goto LABEL_119;
            }

            int tmpKeyframeIndex;
            XAnim_GetTimeIndex<T>(&animTime, pIndices, finalTableSize, &tmpKeyframeIndex, &animTime.time);
            tmpKeyframeIndex <<= 8;
            int tableDelta = CLAMP((tableSize - tmpKeyframeIndex), 0, 256);
            pIndices = &indices[tmpKeyframeIndex];
            XAnim_GetTimeIndex<T>(&animTime, pIndices, tableDelta, &v59, &lerpFrac);
            v59 += tmpKeyframeIndex;
            indices += tableSize + 1;
        }
        else
        {
            if constexpr (sizeof(T) == sizeof(unsigned char))
            {
                pIndices = (T *)dataByte;
                dataByte += tableSize + 1;
            }
            else
            {
                pIndices = (T *)dataShort;
                dataShort += tableSize + 1;
            }

            if (ignorePartBits->testBit(modelPartIndex))
            {
                v19 = 0;
                goto LABEL_119;
            }
            XAnim_GetTimeIndex<T>(&animTime, pIndices, tableSize, &v59, &lerpFrac);
        }

        v19 = 1;
LABEL_119:
        if (v19)
        {
            v60 = &randomDataByte[3 * v59];

            float4 from;
            from.v[0] = (float)v60[0];
            from.v[1] = (float)v60[1];
            from.v[2] = (float)v60[2];
            from.v[3] = 0.0f;

            float4 to;
            to.v[0] = (float)v60[3];
            to.v[1] = (float)v60[4];
            to.v[2] = (float)v60[5];
            to.v[3] = 0.0f;

            XAnimWeightedAccumLerpedTrans(from, to, lerpFrac, weightScale, (const float *)dataInt, &rotTransArray[modelPartIndex]);
        }
        ++animPartIndex;
        dataInt += 6;
        randomDataByte += 3 * tableSize + 3;
    }

    size += parts->boneCount[6];
    while (animPartIndex < size)
    {
        modelPartIndex = animToModel[*dataByte++];
        iassert(modelPartIndex < DOBJ_MAX_PARTS);
        tableSize = (unsigned short)*dataShort++;

        T *pIndices;
        if (tableSize >= 64 && sizeof(T) > 1)
        {
            pIndices = (T*)dataShort;
            int finalTableSize = ((tableSize - 1) >> 8) + 1;
            dataShort += finalTableSize + 1;
            if (ignorePartBits->testBit(modelPartIndex))
            {
                indices += tableSize + 1;
                v17 = 0;
                goto LABEL_141;
            }
            int tmpKeyframeIndex;
            XAnim_GetTimeIndex<T>(&animTime, pIndices, finalTableSize, &tmpKeyframeIndex, &animTime.time);
            tmpKeyframeIndex <<= 8;

            int tableDelta = CLAMP((tableSize - tmpKeyframeIndex), 0, 256);
            pIndices = &indices[tmpKeyframeIndex];
            XAnim_GetTimeIndex<T>(&animTime, pIndices, tableDelta, &v50, &lerpFrac2);
            v50 += tmpKeyframeIndex;
            indices += tableSize + 1;
        }
        else
        {
            if constexpr (sizeof(T) == sizeof(unsigned char))
            {
                pIndices = (T *)dataByte;
                dataByte += tableSize + 1;
            }
            else
            {
                pIndices = (T *)dataShort;
                dataShort += tableSize + 1;
            }

            if (ignorePartBits->testBit(modelPartIndex))
            {
                v17 = 0;
                goto LABEL_141;
            }
            XAnim_GetTimeIndex<T>(&animTime, pIndices, tableSize, &v50, &lerpFrac2);
        }

        v17 = 1;
LABEL_141:
        if (v17)
        {
            v51 = (unsigned short *)&randomDataShort[3 * v50];

            float4 from;
            from.v[0] = (float)v51[0];
            from.v[1] = (float)v51[1];
            from.v[2] = (float)v51[2];
            from.v[3] = 0.0f;

            float4 to;
            to.v[0] = (float)v51[3];
            to.v[1] = (float)v51[4];
            to.v[2] = (float)v51[5];
            to.v[3] = 0.0f;

            XAnimWeightedAccumLerpedTrans(from, to, lerpFrac2, weightScale, (const float *)dataInt, &rotTransArray[modelPartIndex]);
        }
        ++animPartIndex;
        dataInt += 6;
        randomDataShort += 3 * tableSize + 3;
    }

    size += parts->boneCount[7];
    while (animPartIndex < size)
    {
        modelPartIndex = animToModel[*dataByte];
        iassert(modelPartIndex < DOBJ_MAX_PARTS);

        if (!ignorePartBits->testBit(modelPartIndex))
            XAnimWeightedAccumTrans(weightScale, dataInt, &rotTransArray[modelPartIndex]);

        ++animPartIndex;
        ++dataByte;
        dataInt += 3;
    }

    size += parts->boneCount[8];
    while (animPartIndex < size)
    {
        modelPartIndex = animToModel[*dataByte];
        iassert(modelPartIndex < DOBJ_MAX_PARTS);

        if (!ignorePartBits->testBit(modelPartIndex))
            rotTransArray[modelPartIndex].transWeight = rotTransArray[modelPartIndex].transWeight + weightScale;

        ++animPartIndex;
        ++dataByte;
    }
}

void __cdecl XAnimCalcLeaf(XAnimInfo *info, float weightScale, DObjAnimMat *rotTransArray, XAnimCalcAnimInfo *animInfo)
{
    bool v5; // [esp+14h] [ebp-20h]
    float time; // [esp+24h] [ebp-10h]
    XAnimToXModel *animToModel; // [esp+28h] [ebp-Ch]
    int i; // [esp+2Ch] [ebp-8h]
    XAnimParts *parts; // [esp+30h] [ebp-4h]

    iassert(info->inuse);
    parts = info->parts;
    iassert(parts);
    iassert(info->animToModel);

    animToModel = (XAnimToXModel*)SL_ConvertToString(info->animToModel);

    //for (i = 0; i < 4; ++i)
    //    animInfo->animPartBits.array[i] |= *(unsigned int *)&animToModel[4 * i] & ~animInfo->ignorePartBits.array[i]; // weird exception

    for (i = 0; i < 4; i++)
    {
        animInfo->animPartBits.array[i] |= animToModel->partBits.array[i] & ~animInfo->ignorePartBits.array[i];
    }

    time = info->state.currentAnimTime;

    iassert(time >= 0.0f);
    iassert(parts->bLoop ? (time < 1.f) : (time <= 1.f));

    if (time != 1.0f && parts->numframes)
    {
        if (parts->numframes >= 256)
            XAnimCalcParts<unsigned short>(
                parts,
                animToModel->boneIndex,
                time,
                weightScale,
                rotTransArray,
                &animInfo->ignorePartBits);
        else
            XAnimCalcParts<unsigned char>(
                parts,
                animToModel->boneIndex,
                time,
                weightScale,
                rotTransArray,
                &animInfo->ignorePartBits);
    }
    else
    {
        iassert(!parts->bLoop);
        XAnimCalcNonLoopEnd(
            parts,
            animToModel->boneIndex,
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
    float dir[4]; // [esp+C0h] [ebp-15Ch] BYREF
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

    iassert(!parts->bLoop);

    dataByte = parts->dataByte;
    dataShort = parts->dataShort;
    dataInt = parts->dataInt;
    randomDataByte = parts->randomDataByte;
    randomDataShort = parts->randomDataShort;
    randomDataInt = parts->randomDataInt;
    useSmallIndices = parts->numframes < 0x100u;
    animPartIndex = 0;

    size = parts->boneCount[0];
    while (animPartIndex < size)
    {
        modelPartIndex = animToModel[animPartIndex];
        iassert((unsigned int)modelPartIndex < DOBJ_MAX_PARTS);

        if (!ignorePartBits->testBit(modelPartIndex))
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
        iassert(modelPartIndex < DOBJ_MAX_PARTS);

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
        if (!ignorePartBits->testBit(modelPartIndex))
        {
            rotLastFrame = &randomDataShort[2 * tableSize];
            frameVec.v[0] = 0.0;
            frameVec.v[1] = 0.0;
            frameVec.v[2] = (double)*rotLastFrame * (1.0f/32767.0f);
            frameVec.v[3] = (double)rotLastFrame[1] * (1.0f/32767.0f);
            start = rotTransArray[modelPartIndex].quat;
            float scale = weightScale;
            // LWSS: we can just use frameVec (blops backport)
            //dir[0] = 0.0;
            //dir[1] = 0.0;
            //dir[2] = frameVec.v[2];
            ////v18 = *(_QWORD *)&frameVec.unitVec[2].packed;
            //dir[3] = frameVec.v[3];
            
            // LWSS: in blops, there's an if() here. Might as well add?
            if (((0.0f * rotTransArray[modelPartIndex].quat[0])
                + (0.0 * rotTransArray[modelPartIndex].quat[1])
                + (frameVec.v[2] * rotTransArray[modelPartIndex].quat[2])
                + (frameVec.v[3] * rotTransArray[modelPartIndex].quat[3])) < 0.0)
            {
                scale = -weightScale;
            }

            Vec4Mad(start, scale, frameVec.v, start);
        }
        ++animPartIndex;
        randomDataShort += 2 * tableSize + 2;
    }

    size += parts->boneCount[2];
    while (animPartIndex < size)
    {
        modelPartIndex = animToModel[animPartIndex];
        iassert(modelPartIndex < DOBJ_MAX_PARTS);
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
        if (!ignorePartBits->testBit(modelPartIndex))
        {
            v70 = &randomDataShort[4 * tableSize];
            result = rotTransArray[modelPartIndex].quat;
            v14[0] = (double)v70[0] * (1.0f/32767.0f);
            v14[1] = (double)v70[1] * (1.0f/32767.0f);
            v14[2] = (double)v70[2] * (1.0f/32767.0f);
            v14[3] = (double)v70[3] * (1.0f/32767.0f);

            float scale2 = weightScale;
            // LWSS: another if() from blops
            if ((float)((float)((float)((float)(v14[0] * rotTransArray[modelPartIndex].quat[0])
                + (float)(v14[1] * rotTransArray[modelPartIndex].quat[1]))
                + (float)(v14[2] * rotTransArray[modelPartIndex].quat[2]))
                + (float)(v14[3] * rotTransArray[modelPartIndex].quat[3])) < 0.0)
            {
                scale2 = -weightScale;
            }
            Vec4Mad(result, scale2, v14, result);
        }
        ++animPartIndex;
        randomDataShort += 4 * tableSize + 4;
    }

    size += parts->boneCount[3];
    while (animPartIndex < size)
    {
        modelPartIndex = animToModel[animPartIndex];
        iassert(modelPartIndex < DOBJ_MAX_PARTS);

        if (!ignorePartBits->testBit(modelPartIndex))
        {
            quat = rotTransArray[modelPartIndex].quat;
            v12[0] = 0.0f;
            v12[1] = 0.0f;
            v12[2] = (double)dataShort[0] * (1.0f/32767.0f);
            v12[3] = (double)dataShort[1] * (1.0f/32767.0f);

            float scale3 = weightScale;
            // LWSS: another if() from blops
            if ((float)((float)((float)((float)(0.0 * rotTransArray[modelPartIndex].quat[0])
                + (float)(0.0 * rotTransArray[modelPartIndex].quat[1]))
                + (float)(v12[2] * rotTransArray[modelPartIndex].quat[2]))
                + (float)(v12[3] * rotTransArray[modelPartIndex].quat[3])) < 0.0)
            {
                scale3 = -weightScale;
            }

            Vec4Mad(quat, scale3, v12, quat);
        }
        ++animPartIndex;
        dataShort += 2;
    }

    size += parts->boneCount[4];
    while (animPartIndex < size)
    {
        modelPartIndex = animToModel[animPartIndex];
        iassert(modelPartIndex < DOBJ_MAX_PARTS);
        if (!ignorePartBits->testBit(modelPartIndex))
        {
            v10[0] = (double)dataShort[0] * (1.0f/32767.0f);
            v10[1] = (double)dataShort[1] * (1.0f/32767.0f);
            v10[2] = (double)dataShort[2] * (1.0f/32767.0f);
            v10[3] = (double)dataShort[3] * (1.0f/32767.0f);

            float scale4 = weightScale;
            if ((float)((float)((float)((float)(v10[0] * rotTransArray[modelPartIndex].quat[0])
                + (float)(v10[1] * rotTransArray[modelPartIndex].quat[1]))
                + (float)(v10[2] * rotTransArray[modelPartIndex].quat[2]))
                + (float)(v10[3] * rotTransArray[modelPartIndex].quat[3])) < 0.0)
            {
                scale4 = -weightScale;
            }

            Vec4Mad(rotTransArray[modelPartIndex].quat, scale4, v10, rotTransArray[modelPartIndex].quat);
        }
        ++animPartIndex;
        dataShort += 4;
    }

    animPartIndex = 0;
    size = parts->boneCount[5];
    while (animPartIndex < size)
    {
        modelPartIndex = animToModel[*dataByte++];
        iassert(modelPartIndex < DOBJ_MAX_PARTS);
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

        if (!ignorePartBits->testBit(modelPartIndex))
        {
            totalRotTrans = &rotTransArray[modelPartIndex];
            posVec = *(float4 *)totalRotTrans->trans;

            uint8 *data = &randomDataByte[3 * tableSize];

            lerp.v[0] = data[0];
            lerp.v[1] = data[1];
            lerp.v[2] = data[2];
            lerp.v[3] = 0.0;

            minsVec.v[0] = *((float *)dataInt + 0);
            minsVec.v[1] = *((float *)dataInt + 1);
            minsVec.v[2] = *((float *)dataInt + 2);
            minsVec.v[3] = 0.0;

            sizeVec.v[0] = *((float *)dataInt + 3);
            sizeVec.v[1] = *((float *)dataInt + 4);
            sizeVec.v[2] = *((float *)dataInt + 5);
            sizeVec.v[3] = 0.0;

            posVec.v[0] = weightScale * sizeVec.v[0] * lerp.v[0] + minsVec.v[0] + posVec.v[0];
            posVec.v[1] = weightScale * sizeVec.v[1] * lerp.v[1] + minsVec.v[1] + posVec.v[1];
            posVec.v[2] = weightScale * sizeVec.v[2] * lerp.v[2] + minsVec.v[2] + posVec.v[2];
            posVec.v[3] = weightScale * (float)0.0f * (float)0.0f + (float)0.0f + posVec.v[3];
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
        iassert(modelPartIndex < DOBJ_MAX_PARTS);
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

        if (!ignorePartBits->testBit(modelPartIndex))
        {
            totalRotTrans = &rotTransArray[modelPartIndex];
            transWeight = totalRotTrans->transWeight;
            v28 = &randomDataShort[3 * tableSize];

            v41 = (float)(unsigned __int16)v28[0];
            v42 = (float)(unsigned __int16)v28[1];
            v43 = (float)(unsigned __int16)v28[2];
            v44 = 0.0;

            v45 = *((float *)dataInt + 0);
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
            v37 = weightScale * v33 + totalRotTrans->trans[0];
            v38 = weightScale * v34 + totalRotTrans->trans[1];
            v39 = weightScale * v35 + totalRotTrans->trans[2];
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
        iassert(modelPartIndex < DOBJ_MAX_PARTS);
        if (!ignorePartBits->testBit(modelPartIndex))
        {
            totalRotTrans = &rotTransArray[modelPartIndex];

            v24 = totalRotTrans->trans[0];
            v25 = totalRotTrans->trans[1];
            v26 = totalRotTrans->trans[2];
            v27 = totalRotTrans->transWeight;

            v20 = *((float *)dataInt + 0);
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
        iassert(modelPartIndex < DOBJ_MAX_PARTS);
        if (!ignorePartBits->testBit(modelPartIndex))
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
        if (!info->ignorePartBits.testBit(i) && rotTransArray->transWeight != 0.0)
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
    float r;

    for (unsigned int i = 0; (int)i < numBones; ++i)
    {
        if (!info->ignorePartBits.testBit(i))
        {
            r = Vec4LengthSq(rotTransArray->quat);
            if (r != 0.0)
            {
                Vec4Scale(rotTransArray->quat, (I_rsqrt(r) * weightScale), rotTransArray->quat);
            }
            if (rotTransArray->transWeight != 0.0)
            {
                Vec3Scale(rotTransArray->trans, (weightScale / rotTransArray->transWeight), rotTransArray->trans);
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
    float r;

    for (int i = 0; i < numBones; ++i)
    {
        if (!info->ignorePartBits.testBit(i))
        {
            r = Vec4LengthSq(rotTrans->quat);
            if (r != 0.0)
            {
                Vec4Mad(totalRotTrans->quat, (I_rsqrt(r) * weightScale), rotTrans->quat, totalRotTrans->quat);
            }
            if (rotTrans->transWeight != 0.0)
            {
                Vec3Mad(totalRotTrans->trans, (weightScale / rotTrans->transWeight), rotTrans->trans, totalRotTrans->trans);
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
    float rot[4]; // [esp+20h] [ebp-14h] BYREF
    const bitarray<128> *ignorePartBits; // [esp+30h] [ebp-4h]

    ignorePartBits = &info->ignorePartBits;
    for (int i = 0; (int)i < boneCount; ++i)
    {
        if (!ignorePartBits->testBit(i))
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

void __cdecl XAnim_CalcPosDeltaEntire(const XAnimDeltaPart *animDelta, float4 *posDelta)
{
    XAnimPartTrans *trans; // ecx
    unsigned short *v3; // [esp+20h] [ebp-44h]
    unsigned __int8 *v4; // [esp+24h] [ebp-40h]
    float sizeVec[2]; // [esp+30h] [ebp-34h]
    float lerp[4]; // [esp+3Ch] [ebp-28h]
    float minsVec[2]; // [esp+50h] [ebp-14h]
    XAnimPartTrans *posFrameDeltas; // [esp+60h] [ebp-4h]

    if (animDelta->trans)
    {
        posFrameDeltas = animDelta->trans;
        if (animDelta->trans->size)
        {
            trans = animDelta->trans;
            if (posFrameDeltas->smallTrans)
            {
                v4 = posFrameDeltas->u.frames.frames._1[posFrameDeltas->size];
                lerp[0] = (float)v4[0];
                lerp[1] = (float)v4[1];
                lerp[2] = (float)v4[2];
            }
            else
            {
                v3 = posFrameDeltas->u.frames.frames._2[posFrameDeltas->size];
                lerp[0] = (float)v3[0];
                lerp[1] = (float)v3[1];
                lerp[2] = (float)v3[2];
            }
            minsVec[0] = posFrameDeltas->u.frames.mins[1];
            minsVec[1] = posFrameDeltas->u.frames.mins[2];
            sizeVec[0] = posFrameDeltas->u.frames.size[1];
            sizeVec[1] = posFrameDeltas->u.frames.size[2];
            posDelta->v[0] = posFrameDeltas->u.frames.size[0] * lerp[0] + posFrameDeltas->u.frames.mins[0];
            posDelta->v[1] = sizeVec[0] * lerp[1] + minsVec[0];
            posDelta->v[2] = sizeVec[1] * lerp[2] + minsVec[1];
            posDelta->v[3] = (float)0.0f * (float)0.0f + (float)0.0f;
        }
        else
        {
            posDelta->v[0] = posFrameDeltas->u.frames.mins[0];
            posDelta->v[1] = posFrameDeltas->u.frames.mins[1];
            posDelta->v[2] = posFrameDeltas->u.frames.mins[2];
            posDelta->v[3] = 0.0f;
        }
    }
    else
    {
        posDelta->v[0] = 0.0;
        posDelta->v[1] = 0.0;
        posDelta->v[2] = 0.0;
        posDelta->v[3] = 0.0;
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
    float lerp[3]; // [esp+1Ch] [ebp-14h] BYREF
    const float *minsVec; // [esp+28h] [ebp-8h]

    Vec3Lerp(fromVec.v, toVec.v, keyFrameLerpFrac, lerp); // KISAKTODO: remove these shitty float4's
    minsVec = dataInt;
    Vec3Accum(dataInt, dataInt + 3, lerp, frameVec);
    Vec3Mad(totalRotTrans->trans, weightScale, frameVec, totalRotTrans->trans);
    totalRotTrans->transWeight = totalRotTrans->transWeight + weightScale;
}

void __cdecl XAnimWeightedAccumTrans(float weightScale, int *dataInt, DObjAnimMat *totalRotTrans)
{
    Vec3Mad(totalRotTrans->trans, weightScale, (const float *)dataInt, totalRotTrans->trans);
    totalRotTrans->transWeight = totalRotTrans->transWeight + weightScale;
}

template <typename T>
void XAnim_GetTimeIndex(
    const XAnimTime *animTime,
    const T *indices,
    int tableSize,
    int *keyFrameIndex,
    float *keyFrameLerpFrac)
{
    unsigned int low; // [esp+20h] [ebp-10h]
    unsigned int frameIndex; // [esp+24h] [ebp-Ch]
    int index; // [esp+28h] [ebp-8h]
    int high; // [esp+2Ch] [ebp-4h]

    index = (int)((float)tableSize * animTime->time);
    frameIndex = animTime->frameIndex;
    if (frameIndex >= indices[index])
    {
        if (frameIndex >= indices[index + 1])
        {
            low = index + 1;
            high = tableSize;
            while (frameIndex >= indices[++low])
            {
                index = (high + low) >> 1;
                if (frameIndex >= indices[index])
                {
                    low = index + 1;
                    if (frameIndex < indices[index + 1])
                        goto LABEL_16;
                }
                else
                {
                    high = (high + low) >> 1;
                }
            }
            index = low - 1;
        }
    }
    else
    {
        low = 0;
        high = (int)((float)tableSize * animTime->time);
        while (frameIndex < indices[--high])
        {
            index = (high + low) >> 1;
            if (frameIndex >= indices[index])
            {
                low = index + 1;
                if (frameIndex < indices[index + 1])
                    goto LABEL_16;
            }
            else
            {
                high = (high + low) >> 1;
            }
        }
        index = high;
    }
LABEL_16:
    iassert(frameIndex >= indices[index]);
    iassert(frameIndex < indices[index + 1]);
    *keyFrameLerpFrac = (animTime->frameFrac - indices[index]) / (indices[index + 1] - indices[index]);
    *keyFrameIndex = index;
    bcassert(*keyFrameLerpFrac, 1.0f);
}

template <typename T>
void XAnim_CalcPosDeltaDuring(
    const XAnimDeltaPart *animDelta,
    float time,
    int frameCount,
    float4 *posDelta)
{
    float v4; // [esp+68h] [ebp-98h]
    float v5; // [esp+6Ch] [ebp-94h]
    float v6; // [esp+70h] [ebp-90h]
    float v7; // [esp+74h] [ebp-8Ch]
    unsigned __int16 *v8; // [esp+7Ch] [ebp-84h]
    unsigned __int16 *v9; // [esp+80h] [ebp-80h]
    unsigned __int8 *v10; // [esp+84h] [ebp-7Ch]
    unsigned __int8 *v11; // [esp+88h] [ebp-78h]
    XAnimPartTransData *p_u; // [esp+8Ch] [ebp-74h]
    int keyFrameIndex; // [esp+90h] [ebp-70h] BYREF
    float4 sizeVec; // [esp+94h] [ebp-6Ch]
    float4 toVec; // [esp+A4h] [ebp-5Ch]
    XAnimTime animTime; // [esp+B4h] [ebp-4Ch] BYREF
    int nextKeyFrameIndex; // [esp+C0h] [ebp-40h]
    float keyFrameLerpFrac; // [esp+C4h] [ebp-3Ch] BYREF
    float4 fromVec; // [esp+C8h] [ebp-38h]
    float4 lerp; // [esp+D8h] [ebp-28h]
    float4 minsVec; // [esp+E8h] [ebp-18h]
    const XAnimPartTrans *posFrameDeltas; // [esp+FCh] [ebp-4h]

    iassert(frameCount && time != 1.0f);

    if (animDelta->trans)
    {
        posFrameDeltas = animDelta->trans;
        if (posFrameDeltas->size)
        { 
            XAnim_SetTime(time, frameCount, &animTime);
            XAnim_GetTimeIndex<T>(
                &animTime,
                (T*)posFrameDeltas->u.frames.indices._1,
                posFrameDeltas->size,
                &keyFrameIndex,
                &keyFrameLerpFrac);
            nextKeyFrameIndex = keyFrameIndex + 1;
            if (posFrameDeltas->smallTrans)
            {
                v11 = posFrameDeltas->u.frames.frames._1[keyFrameIndex];
                fromVec.v[0] = v11[0];
                fromVec.v[1] = v11[1];
                fromVec.v[2] = v11[2];
                fromVec.v[3] = 0.0f;

                v10 = posFrameDeltas->u.frames.frames._1[nextKeyFrameIndex];
                toVec.v[0] = v10[0];
                toVec.v[1] = v10[1];
                toVec.v[2] = v10[2];
            }
            else
            {
                v9 = posFrameDeltas->u.frames.frames._2[keyFrameIndex];
                fromVec.v[0] = v9[0];
                fromVec.v[1] = v9[1];
                fromVec.v[2] = v9[2];
                fromVec.v[3] = 0.0;

                v8 = posFrameDeltas->u.frames.frames._2[nextKeyFrameIndex];
                toVec.v[0] = v8[0];
                toVec.v[1] = v8[1];
                toVec.v[2] = v8[2];
            }
            toVec.v[3] = 0.0;
            v4 = toVec.v[0] - fromVec.v[0];
            v5 = toVec.v[1] - fromVec.v[1];
            v6 = toVec.v[2] - fromVec.v[2];
            v7 = 0.0 - fromVec.v[3];
            lerp.v[0] = keyFrameLerpFrac * v4 + fromVec.v[0];
            lerp.v[1] = keyFrameLerpFrac * v5 + fromVec.v[1];
            lerp.v[2] = keyFrameLerpFrac * v6 + fromVec.v[2];
            lerp.v[3] = keyFrameLerpFrac * v7 + fromVec.v[3];
            minsVec.v[0] = posFrameDeltas->u.frames.mins[0];
            minsVec.v[1] = posFrameDeltas->u.frames.mins[1];
            minsVec.v[2] = posFrameDeltas->u.frames.mins[2];
            minsVec.v[3] = 0.0;
            sizeVec.v[0] = posFrameDeltas->u.frames.size[0];
            sizeVec.v[1] = posFrameDeltas->u.frames.size[1];
            sizeVec.v[2] = posFrameDeltas->u.frames.size[2];
            sizeVec.v[3] = 0.0;
            posDelta->v[0] = sizeVec.v[0] * lerp.v[0] + minsVec.v[0];
            posDelta->v[1] = sizeVec.v[1] * lerp.v[1] + minsVec.v[1];
            posDelta->v[2] = sizeVec.v[2] * lerp.v[2] + minsVec.v[2];
            posDelta->v[3] = sizeVec.v[3] * lerp.v[3] + minsVec.v[3];
        }
        else
        {
            p_u = (XAnimPartTransData*)&posFrameDeltas->u;
            posDelta->v[0] = posFrameDeltas->u.frames.mins[0];
            posDelta->v[1] = p_u->frames.mins[1];
            posDelta->v[2] = p_u->frames.mins[2];
            posDelta->v[3] = 0.0;
        }
    }
    else
    {
        posDelta->v[0] = 0.0;
        posDelta->v[1] = 0.0;
        posDelta->v[2] = 0.0;
        posDelta->v[3] = 0.0;
    }
}

template <typename T>
void XAnim_CalcRotDeltaDuring(
    const XAnimDeltaPart *animDelta,
    float time,
    int frameCount,
    float *rotDelta)
{
    XAnimDeltaPartQuatData *p_u; // [esp+18h] [ebp-1Ch]
    int keyFrameIndex; // [esp+1Ch] [ebp-18h] BYREF
    XAnimTime animTime; // [esp+20h] [ebp-14h] BYREF
    float keyFrameLerpFrac; // [esp+2Ch] [ebp-8h] BYREF
    const XAnimDeltaPartQuat *rotFrameDeltas; // [esp+30h] [ebp-4h]

    iassert(frameCount && (time != 1.0f));

    if (animDelta->quat)
    {
        rotFrameDeltas = animDelta->quat;
        if (rotFrameDeltas->size)
        {
            XAnim_SetTime(time, frameCount, &animTime);
            XAnim_GetTimeIndex<T>(
                &animTime,
                (T*)rotFrameDeltas->u.frames.indices._1,
                rotFrameDeltas->size,
                &keyFrameIndex,
                &keyFrameLerpFrac);
            Short2LerpAsVec2(
                rotFrameDeltas->u.frames.frames[keyFrameIndex],
                rotFrameDeltas->u.frames.frames[keyFrameIndex + 1],
                keyFrameLerpFrac,
                rotDelta);
        }
        else
        {
            p_u = (XAnimDeltaPartQuatData*)&rotFrameDeltas->u;
            rotDelta[0] = rotFrameDeltas->u.frame0[0];
            rotDelta[1] = p_u->frame0[1];
        }
    }
    else
    {
        rotDelta[0] = 0.0f;
        rotDelta[1] = 32767.0f;
    }
}

void __cdecl XAnim_SetTime(float time, int frameCount, XAnimTime *animTime)
{
    animTime->time = time;
    animTime->frameFrac = frameCount * time;
    animTime->frameIndex = animTime->frameFrac;
}

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
    int numNonRootBones; // [esp+90h] [ebp-60A4h]
    XModel *model; // [esp+94h] [ebp-60A0h]
    int kk; // [esp+98h] [ebp-609Ch]
    XModel **models; // [esp+9Ch] [ebp-6098h]
    unsigned int boneIndex; // [esp+A0h] [ebp-6094h]
    XAnimInfo *AnimInfo; // [esp+A4h] [ebp-6090h]
    XAnimTree_s *tree; // [esp+A8h] [ebp-608Ch]
    int jj; // [esp+ACh] [ebp-6088h]
    int ii; // [esp+B0h] [ebp-6084h]
    unsigned int bone; // [esp+D0h] [ebp-6064h]
    DObjAnimMat *mat; // [esp+D4h] [ebp-6060h]
    //float *quat; // [esp+D4h] [ebp-6060h]
    char endEarly; // [esp+DBh] [ebp-6059h]
    int m; // [esp+DCh] [ebp-6058h]
    int k; // [esp+E0h] [ebp-6054h]
    int j; // [esp+E4h] [ebp-6050h]
    XAnimCalcAnimInfo info; // [esp+E8h] [ebp-604Ch] BYREF
    int i; // [esp+6114h] [ebp-20h]
    DSkel *p_skel; // [esp+6120h] [ebp-14h]
    int v37; // [esp+6128h] [ebp-Ch]
    void *v38; // [esp+612Ch] [ebp-8h]
    void *retaddr; // [esp+6134h] [ebp+0h]

    //v38 = retaddr;
    //v3 = alloca(24844); // LWSS: this was for `XAnimCalcAnimInfo`, which is a bigass struct

    PROF_SCOPED("DObjCalcAnim");
    iassert(obj);
    p_skel = (DSkel *)&obj->skel;

    for (i = 0; i < 4; ++i)
        info.animPartBits.array[i] = partBits[i];
    for (j = 0; j < 4; ++j)
        info.animPartBits.array[j] = ~info.animPartBits.array[j];
    for (k = 0; k < 4; ++k)
        info.animPartBits.array[k] |= p_skel->partBits.anim[k];
    for (m = 0; m < 4; ++m)
    {
        if (info.animPartBits.array[m] != -1)
        {
            endEarly = false;
            goto LABEL_20;
        }
    }
    endEarly = true;
LABEL_20:
    if (endEarly)
    {
        return;
    }

    mat = p_skel->mat;
    for (bone = 0; bone < obj->numBones; ++bone)
    {
        if (p_skel->partBits.anim.testBit(bone))
        {
            iassert(!IS_NAN(mat[bone].quat[0]) && !IS_NAN(mat[bone].quat[1]) && !IS_NAN(mat[bone].quat[2]) && !IS_NAN(mat[bone].quat[3]));
            iassert(!IS_NAN(mat[bone].trans[0]) && !IS_NAN(mat[bone].trans[1]) && !IS_NAN(mat[bone].trans[2]));
        }
    }
    for (ii = 0; ii < 4; ++ii)
        p_skel->partBits.anim[ii] |= partBits[ii];
    for (jj = 0; jj < 4; ++jj)
        info.ignorePartBits.array[jj] = info.animPartBits.array[jj];

    tree = obj->tree;
    if (tree && tree->children)
    {
        InterlockedIncrement(&tree->calcRefCount);
        iassert(!tree->modifyRefCount);
        info.ignorePartBits.setBit(127);
        AnimInfo = GetAnimInfo(tree->children);
        XAnimCalc(obj, AnimInfo, 1.0f, 1, 0, &info, 0, p_skel->mat);
        iassert(!tree->modifyRefCount);
        InterlockedDecrement(&tree->calcRefCount);
    }
    boneIndex = 0;
    models = obj->models;
    for (kk = 0; kk < obj->numModels; ++kk)
    {
        model = models[kk];
        for (mm = model->numRootBones; mm; --mm)
        {
            if (info.animPartBits.testBit(boneIndex))
            {
                if (p_skel->partBits.anim.testBit(boneIndex))
                {
                    iassert(boneIndex < obj->numBones);
                    iassert(!IS_NAN(mat->quat[0]) && !IS_NAN(mat->quat[1]) && !IS_NAN(mat->quat[2]) && !IS_NAN(mat->quat[3]));
                    iassert(!IS_NAN(mat->trans[0]) && !IS_NAN(mat->trans[1]) && !IS_NAN(mat->trans[2]));
                }
            }
            else
            {
                mat->quat[0] = 0.0f;
                mat->quat[1] = 0.0f;
                mat->quat[2] = 0.0f;
                mat->quat[3] = 1.0f;

                mat->trans[0] = 0.0f;
                mat->trans[1] = 0.0f;
                mat->trans[2] = 0.0f;

                mat->transWeight = 0.0f;
            }
            ++mat;
            ++boneIndex;
        }
        quats = model->quats;
        numNonRootBones = model->numBones - model->numRootBones;
        while (numNonRootBones)
        {
            if (info.animPartBits.testBit(boneIndex))
            {
                if (p_skel->partBits.anim.testBit(boneIndex))
                {
                    iassert(!IS_NAN(mat->quat[0]) && !IS_NAN(mat->quat[1]) && !IS_NAN(mat->quat[2]) && !IS_NAN(mat->quat[3]));
                    iassert(!IS_NAN(mat->trans[0]) && !IS_NAN(mat->trans[1]) && !IS_NAN(mat->trans[2]));
                }
            }
            else
            {
                mat->quat[0] = 0.000030518509 * (float)quats[0]; // LWSS: is this divide by (65536/2)?
                mat->quat[1] = 0.000030518509 * (float)quats[1];
                mat->quat[2] = 0.000030518509 * (float)quats[2];
                mat->quat[3] = 0.000030518509 * (float)quats[3];

                mat->trans[0] = 0.0f;
                mat->trans[1] = 0.0f;
                mat->trans[2] = 0.0f;
                    
                mat->transWeight = 0.0f;
            }
            --numNonRootBones;
            ++mat;
            ++boneIndex;
            quats += 4;
        }
    }
}

void __cdecl XAnim_CalcDeltaForTime(const XAnimParts *anim, float time, float *rotDelta, float4 *posDelta)
{
    int frameCount; // [esp+20h] [ebp-8h]
    XAnimDeltaPart *animDelta; // [esp+24h] [ebp-4h]

    if (time < 0.0 || time > 1.0)
        MyAssertHandler(
            ".\\xanim\\xanim_calc.cpp",
            1821,
            0,
            "time not in [0.0f, 1.0f]\n\t%g not in [%g, %g]",
            time,
            0.0,
            1.0);
    animDelta = anim->deltaPart;
    frameCount = anim->numframes;
    if (time == 1.0 || !anim->numframes)
    {
        XAnim_CalcRotDeltaEntire(animDelta, rotDelta);
        XAnim_CalcPosDeltaEntire(animDelta, posDelta);
    }
    else if (anim->numframes >= 0x100u)
    {
        XAnim_CalcRotDeltaDuring<unsigned short>(animDelta, time, frameCount, rotDelta);
        XAnim_CalcPosDeltaDuring<unsigned short>(animDelta, time, frameCount, posDelta);
    }
    else
    {
        XAnim_CalcRotDeltaDuring<unsigned char>(animDelta, time, frameCount, rotDelta);
        XAnim_CalcPosDeltaDuring<unsigned char>(animDelta, time, frameCount, posDelta);
    }
}