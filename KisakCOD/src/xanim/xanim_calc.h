#pragma once
#include "xanim.h"
#include "dobj.h"
#include <qcommon/bitarray.h>

struct XAnimCalcAnimInfo // sizeof=0x6020
{
    DObjAnimMat rotTransArray[768];
    bitarray<128> animPartBits;
    bitarray<128> ignorePartBits;
};

struct XAnimToXModel // sizeof=0x94
{                                       // ...
    bitarray<128> partBits;             // ...
    unsigned __int8 boneCount;          // ...
    unsigned __int8 boneIndex[128];     // ...
    // padding byte
    // padding byte
    // padding byte
};

void __cdecl XAnimCalc(
    const DObj_s *obj,
    XAnimInfo *info,
    float weightScale,
    bool bClear,
    bool bNormQuat,
    XAnimCalcAnimInfo *animInfo,
    int rotTransArrayIndex,
    DObjAnimMat *rotTransArray);
bool __cdecl IsInfoAdditive(const XAnimInfo *info);
void __cdecl XAnimClearRotTransArray(const DObj_s *obj, DObjAnimMat *rotTransArray, XAnimCalcAnimInfo *info);
void __cdecl XAnimCalcLeaf(XAnimInfo *info, float weightScale, DObjAnimMat *rotTransArray, XAnimCalcAnimInfo *animInfo);
void __cdecl XAnimCalcNonLoopEnd(
    const XAnimParts *parts,
    const unsigned __int8 *animToModel,
    float weightScale,
    DObjAnimMat *rotTransArray,
    const bitarray<128> *ignorePartBits);
DObjAnimMat *__cdecl XAnimGetCalcBuffer(XAnimCalcAnimInfo *info, const DObj_s *obj, int *rotTransArrayIndex);
void __cdecl XAnimScaleRotTransArray(int numBones, const XAnimCalcAnimInfo *info, DObjAnimMat *rotTransArray);
void __cdecl XAnimNormalizeRotScaleTransArray(
    int numBones,
    const XAnimCalcAnimInfo *info,
    float weightScale,
    DObjAnimMat *rotTransArray);
void __cdecl XAnimMadRotTransArray(
    int numBones,
    const XAnimCalcAnimInfo *info,
    float weightScale,
    const DObjAnimMat *rotTrans,
    DObjAnimMat *totalRotTrans);
void __cdecl XAnimApplyAdditives(
    DObjAnimMat *rotTransArray,
    DObjAnimMat *additiveArray,
    float weight,
    int boneCount,
    XAnimCalcAnimInfo *info);
void __cdecl XAnim_CalcRotDeltaEntire(const XAnimDeltaPart *animDelta, float *rotDelta);
void __cdecl XAnimWeightedAccumLerpedTrans(
    float4 fromVec,
    float4 toVec,
    float keyFrameLerpFrac,
    float weightScale,
    const float *dataInt,
    DObjAnimMat *totalRotTrans);
void __cdecl XAnimWeightedAccumTrans(float weightScale, int *dataInt, DObjAnimMat *totalRotTrans);

void __cdecl XAnim_CalcRotDeltaDuring_unsigned_short_(
    const XAnimDeltaPart *animDelta,
    float time,
    int frameCount,
    float *rotDelta);

void __cdecl XAnim_CalcPosDeltaDuring_unsigned_short_(
    const XAnimDeltaPart *animDelta,
    float time,
    int frameCount,
    float4 *posDelta);

void __cdecl XAnim_CalcRotDeltaDuring_unsigned_char_(
    const XAnimDeltaPart *animDelta,
    float time,
    int frameCount,
    float *rotDelta);

void __cdecl XAnim_CalcPosDeltaDuring_unsigned_char_(
    const XAnimDeltaPart *animDelta,
    float time,
    int frameCount,
    float4 *posDelta);

void DObjCalcAnim(const struct DObj_s *obj, int *partBits);