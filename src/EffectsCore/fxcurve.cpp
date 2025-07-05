#include "fx_system.h"
#include <universal/com_memory.h>


double __cdecl FxCurve_Interpolate1d(const float *key, float intermediateTime)
{
    const char *v2; // eax
    float value1; // [esp+1Ch] [ebp-10h]
    float value0; // [esp+20h] [ebp-Ch]
    float time1; // [esp+24h] [ebp-8h]
    float time0; // [esp+28h] [ebp-4h]

    time0 = *key;
    time1 = key[2];
    value0 = key[1];
    value1 = key[3];
    if (intermediateTime < (double)time0 || time1 < (double)intermediateTime || time1 == time0)
    {
        v2 = va("%g, %g, %g", time0, time1, intermediateTime);
        MyAssertHandler(
            "c:\\trees\\cod3\\src\\effectscore\\FxCurve.h",
            56,
            0,
            "%s\n\t%s",
            "time0 <= intermediateTime && intermediateTime <= time1 && time0 != time1",
            v2);
    }
    return (float)((intermediateTime - time0) * (value1 - value0) / (time1 - time0) + value0);
}

void __cdecl FxCurveIterator_Create(FxCurveIterator *createe, const FxCurve *master)
{
    if (!createe)
        MyAssertHandler(".\\EffectsCore\\FxCurve.cpp", 62, 0, "%s", "createe");
    if (!master)
        MyAssertHandler(".\\EffectsCore\\FxCurve.cpp", 64, 0, "%s", "master");
    if (master->keyCount <= 0)
        MyAssertHandler(
            ".\\EffectsCore\\FxCurve.cpp",
            65,
            0,
            "%s\n\t(master->keyCount) = %i",
            "(master->keyCount > 0)",
            master->keyCount);
    if (master->dimensionCount <= 0)
        MyAssertHandler(
            ".\\EffectsCore\\FxCurve.cpp",
            66,
            0,
            "%s\n\t(master->dimensionCount) = %i",
            "(master->dimensionCount > 0)",
            master->dimensionCount);
    if (master == (const FxCurve *)-8)
        MyAssertHandler(".\\EffectsCore\\FxCurve.cpp", 67, 0, "%s", "master->keys");
    createe->master = master;
    createe->currentKeyIndex = 0;
}

void __cdecl FxCurveIterator_Release(FxCurveIterator *releasee)
{
    if (!releasee)
        MyAssertHandler(".\\EffectsCore\\FxCurve.cpp", 83, 0, "%s", "releasee");
    if (!releasee->master)
        MyAssertHandler(".\\EffectsCore\\FxCurve.cpp", 84, 0, "%s", "releasee->master");
    releasee->master = 0;
}

void __cdecl FxCurve_Interpolate3d(const float *key, float intermediateTime, float *result)
{
    const char *v3; // eax
    float fraction; // [esp+18h] [ebp-14h]
    float time1; // [esp+24h] [ebp-8h]
    float time0; // [esp+28h] [ebp-4h]

    time0 = *key;
    time1 = key[4];
    if (intermediateTime < (double)time0 || time1 < (double)intermediateTime || time1 == time0)
    {
        v3 = va("%g, %g, %g", time0, time1, intermediateTime);
        MyAssertHandler(
            "c:\\trees\\cod3\\src\\effectscore\\FxCurve.h",
            74,
            0,
            "%s\n\t%s",
            "time0 <= intermediateTime && intermediateTime <= time1 && time0 != time1",
            v3);
    }
    fraction = (intermediateTime - time0) / (time1 - time0);
    Vec3Lerp(key + 1, key + 5, fraction, result);
}


const FxCurve *__cdecl FxCurve_AllocAndCreateWithKeys(float *keyArray, int32_t dimensionCount, int32_t keyCount)
{
    int32_t createdKeyCount; // [esp+14h] [ebp-1Ch]
    int32_t keySize; // [esp+18h] [ebp-18h]
    bool addKeyAtStart; // [esp+1Fh] [ebp-11h]
    int32_t keyIndex; // [esp+20h] [ebp-10h]
    int32_t keyIndexa; // [esp+20h] [ebp-10h]
    bool addKeyAtEnd; // [esp+27h] [ebp-9h]
    uint8_t *newCurve; // [esp+28h] [ebp-8h]
    int32_t elementIndex; // [esp+2Ch] [ebp-4h]
    int32_t elementIndexa; // [esp+2Ch] [ebp-4h]

    if (!keyArray)
        MyAssertHandler(".\\EffectsCore\\FxCurve_load_obj.cpp", 19, 0, "%s", "keyArray");
    if (keyCount <= 0)
        MyAssertHandler(".\\EffectsCore\\FxCurve_load_obj.cpp", 20, 0, "%s\n\t(keyCount) = %i", "(keyCount > 0)", keyCount);
    if (dimensionCount <= 0)
        MyAssertHandler(
            ".\\EffectsCore\\FxCurve_load_obj.cpp",
            21,
            0,
            "%s\n\t(dimensionCount) = %i",
            "(dimensionCount > 0)",
            dimensionCount);
    keySize = dimensionCount + 1;
    addKeyAtStart = *keyArray != 0.0;
    addKeyAtEnd = keyArray[(dimensionCount + 1) * (keyCount - 1)] != 1.0;
    createdKeyCount = addKeyAtEnd + keyCount + addKeyAtStart;
    if (createdKeyCount < 2)
        MyAssertHandler(
            ".\\EffectsCore\\FxCurve_load_obj.cpp",
            29,
            1,
            "%s\n\t(createdKeyCount) = %i",
            "(createdKeyCount >= 2)",
            createdKeyCount);
    newCurve = Hunk_AllocAlign(4 * createdKeyCount * keySize + 8, 4, "FxCurve_AllocAndCreateWithKeys", 8);
    if (!newCurve)
        MyAssertHandler(".\\EffectsCore\\FxCurve_load_obj.cpp", 38, 0, "%s", "newCurve");
    *(_DWORD *)newCurve = dimensionCount;
    keyIndex = 0;
    if (addKeyAtStart)
    {
        *((float *)newCurve + 2) = 0.0;
        for (elementIndex = 0; elementIndex != dimensionCount; ++elementIndex)
            *(float *)&newCurve[4 * elementIndex + 12] = keyArray[elementIndex + 1];
        keyIndex = 1;
    }
    memcpy(&newCurve[4 * keySize * keyIndex + 8], (uint8_t *)keyArray, 4 * keySize * keyCount);
    keyIndexa = keyCount + keyIndex;
    if (addKeyAtEnd)
    {
        *(float *)&newCurve[4 * keySize * keyIndexa + 8] = 1.0;
        for (elementIndexa = 0; elementIndexa != dimensionCount; ++elementIndexa)
            *(float *)&newCurve[4 * keySize * keyIndexa + 12 + 4 * elementIndexa] = keyArray[elementIndexa + 1];
        ++keyIndexa;
    }
    if (keyIndexa != createdKeyCount)
        MyAssertHandler(
            ".\\EffectsCore\\FxCurve_load_obj.cpp",
            61,
            1,
            "keyIndex == createdKeyCount\n\t%i, %i",
            keyIndexa,
            createdKeyCount);
    if (*((float *)newCurve + 2) != 0.0)
        MyAssertHandler(
            ".\\EffectsCore\\FxCurve_load_obj.cpp",
            62,
            1,
            "%s\n\t(newCurve->keys[0 * keySize + 0]) = %g",
            "(newCurve->keys[0 * keySize + 0] == 0.0f)",
            *((float *)newCurve + 2));
    if (*(float *)&newCurve[4 * keySize * (createdKeyCount - 1) + 8] != 1.0)
        MyAssertHandler(
            ".\\EffectsCore\\FxCurve_load_obj.cpp",
            63,
            1,
            "%s\n\t(newCurve->keys[(createdKeyCount - 1) * keySize + 0]) = %g",
            "(newCurve->keys[(createdKeyCount - 1) * keySize + 0] == 1.0f)",
            *(float *)&newCurve[4 * keySize * (createdKeyCount - 1) + 8]);
    *((_DWORD *)newCurve + 1) = keyCount;
    return (const FxCurve *)newCurve;
}


void __cdecl FxCurveIterator_SampleTimeVec3(FxCurveIterator *source, float *replyVector, float time)
{
    FxCurveIterator_MoveToTime(source, time);
    if (source->currentKeyIndex >= (uint32_t)(source->master->keyCount - 1))
        MyAssertHandler(
            "c:\\trees\\cod3\\src\\effectscore\\FxCurve.h",
            148,
            0,
            "source->currentKeyIndex doesn't index source->master->keyCount - 1\n\t%i not in [0, %i)",
            source->currentKeyIndex,
            source->master->keyCount - 1);
    FxCurve_Interpolate3d(&source->master->keys[4 * source->currentKeyIndex], time, replyVector);
}

double __cdecl FxCurveIterator_SampleTime(FxCurveIterator *source, float time)
{
    FxCurveIterator_MoveToTime(source, time);
    if (source->currentKeyIndex >= (uint32_t)(source->master->keyCount - 1))
        MyAssertHandler(
            "c:\\trees\\cod3\\src\\effectscore\\FxCurve.h",
            126,
            0,
            "source->currentKeyIndex doesn't index source->master->keyCount - 1\n\t%i not in [0, %i)",
            source->currentKeyIndex,
            source->master->keyCount - 1);
    return (float)FxCurve_Interpolate1d(&source->master->keys[2 * source->currentKeyIndex], time);
}

void __cdecl FxCurveIterator_MoveToTime(FxCurveIterator *source, float time)
{
    const char *v2; // eax
    int32_t keySize; // [esp+8h] [ebp-8h]
    const float *key; // [esp+Ch] [ebp-4h]

    if (!source)
        MyAssertHandler("c:\\trees\\cod3\\src\\effectscore\\FxCurve.h", 85, 0, "%s", "source");
    if (!source->master)
        MyAssertHandler("c:\\trees\\cod3\\src\\effectscore\\FxCurve.h", 86, 0, "%s", "source->master");
    if (source->master == (const FxCurve *)-8)
        MyAssertHandler("c:\\trees\\cod3\\src\\effectscore\\FxCurve.h", 87, 0, "%s", "source->master->keys");
    if (time < 0.0 || time > 1.0)
        MyAssertHandler(
            "c:\\trees\\cod3\\src\\effectscore\\FxCurve.h",
            88,
            0,
            "%s\n\t(time) = %g",
            "(time >= 0.0f && time <= 1.0f)",
            time);
    if (source->master->keyCount <= 0)
        MyAssertHandler(
            "c:\\trees\\cod3\\src\\effectscore\\FxCurve.h",
            89,
            0,
            "%s\n\t(source->master->keyCount) = %i",
            "(source->master->keyCount > 0)",
            source->master->keyCount);
    if (source->master->dimensionCount <= 0)
        MyAssertHandler(
            "c:\\trees\\cod3\\src\\effectscore\\FxCurve.h",
            90,
            0,
            "%s\n\t(source->master->dimensionCount) = %i",
            "(source->master->dimensionCount > 0)",
            source->master->dimensionCount);
    if (source->currentKeyIndex >= (uint32_t)source->master->keyCount)
        MyAssertHandler(
            "c:\\trees\\cod3\\src\\effectscore\\FxCurve.h",
            91,
            0,
            "source->currentKeyIndex doesn't index source->master->keyCount\n\t%i not in [0, %i)",
            source->currentKeyIndex,
            source->master->keyCount);
    keySize = source->master->dimensionCount + 1;
    key = &source->master->keys[keySize * source->currentKeyIndex];
    if (*key > (double)time)
    {
        source->currentKeyIndex = 0;
        key = source->master->keys;
    }
    while (key[keySize] < (double)time)
    {
        ++source->currentKeyIndex;
        key += keySize;
    }
    if (key != &source->master->keys[keySize * source->currentKeyIndex])
    {
        v2 = va("%p != %p", key, &source->master->keys[keySize * source->currentKeyIndex]);
        MyAssertHandler(
            "c:\\trees\\cod3\\src\\effectscore\\FxCurve.h",
            108,
            1,
            "%s\n\t%s",
            "key == &source->master->keys[source->currentKeyIndex * keySize]",
            v2);
    }
    if (source->currentKeyIndex >= (uint32_t)source->master->keyCount)
        MyAssertHandler(
            "c:\\trees\\cod3\\src\\effectscore\\FxCurve.h",
            109,
            1,
            "source->currentKeyIndex doesn't index source->master->keyCount\n\t%i not in [0, %i)",
            source->currentKeyIndex,
            source->master->keyCount);
}