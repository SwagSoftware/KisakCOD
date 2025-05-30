#include "fx_system.h"

#include <xanim/xanim.h>
#include <xanim/dobj.h>
#include <xanim/dobj_utils.h>

#include <client_mp/client_mp.h>

#include <physics/phys_local.h>

#include <win32/win_local.h>
#include <aim_assist/aim_assist.h>
#include <cgame_mp/cg_local_mp.h>

void __cdecl FX_SpawnlAlFutureLooping(
    FxSystem *system,
    FxEffect *effect,
    int elemDefFirst,
    int elemDefCount,
    FxSpatialFrame* frameBegin,
    FxSpatialFrame* frameEnd,
    int msecWhenPlayed,
    int msecUpdateBegin)
{
    int elemDefIndex; // [esp+64h] [ebp-4h]

    if (!effect)
        MyAssertHandler(".\\EffectsCore\\fx_update.cpp", 465, 0, "%s", "effect");
    if (!effect->def)
        MyAssertHandler(".\\EffectsCore\\fx_update.cpp", 468, 0, "%s", "effectDef");
    for (elemDefIndex = elemDefFirst; elemDefIndex != elemDefCount + elemDefFirst; ++elemDefIndex)
    {
        if (effect->def->elemDefs[elemDefIndex].spawn.looping.count != 0x7FFFFFFF)
            FX_SpawnLoopingElems(
                system,
                effect,
                elemDefIndex,
                frameBegin,
                frameEnd,
                msecWhenPlayed,
                msecUpdateBegin,
                0x7FFFFFFF);
    }
}
void __cdecl FX_SpawnTrailLoopingElems(
    FxSystem* system,
    FxEffect* effect,
    FxTrail* trail,
    FxSpatialFrame* frameBegin,
    FxSpatialFrame* frameEnd,
    int msecWhenPlayed,
    int msecUpdateBegin,
    int msecUpdateEnd,
    float distanceTravelledBegin,
    float distanceTravelledEnd)
{
    float v10; // [esp+1Ch] [ebp-50h]
    float v11; // [esp+20h] [ebp-4Ch]
    float v12; // [esp+24h] [ebp-48h]
    float lerp; // [esp+2Ch] [ebp-40h]
    FxSpatialFrame frameWhenPlayed; // [esp+34h] [ebp-38h] BYREF
    float distSpawn; // [esp+50h] [ebp-1Ch]
    float normalizedTotalDistance; // [esp+54h] [ebp-18h]
    const FxEffectDef* effectDef; // [esp+58h] [ebp-14h]
    const FxElemDef* elemDef; // [esp+5Ch] [ebp-10h]
    float normalizedDistanceTraversed; // [esp+60h] [ebp-Ch]
    float normalizedDistanceRemaining; // [esp+64h] [ebp-8h]
    float normalizedDistanceBeforeSpawn; // [esp+68h] [ebp-4h]

    if (!effect)
        MyAssertHandler(".\\EffectsCore\\fx_update.cpp", 101, 0, "%s", "effect");
    effectDef = effect->def;
    if (!effectDef)
        MyAssertHandler(".\\EffectsCore\\fx_update.cpp", 104, 0, "%s", "effectDef");
    if (trail->defIndex >= (effectDef->elemDefCountEmission
        + effectDef->elemDefCountOneShot
        + effectDef->elemDefCountLooping))
        MyAssertHandler(
            ".\\EffectsCore\\fx_update.cpp",
            106,
            0,
            "trail->defIndex doesn't index effectDef->elemDefCountLooping + effectDef->elemDefCountOneShot + effectDef->elemDef"
            "CountEmission\n"
            "\t%i not in [0, %i)",
            trail->defIndex,
            effectDef->elemDefCountEmission + effectDef->elemDefCountOneShot + effectDef->elemDefCountLooping);
    if (trail->defIndex >= effectDef->elemDefCountLooping
        && trail->defIndex < effectDef->elemDefCountOneShot + effectDef->elemDefCountLooping)
    {
        MyAssertHandler(
            ".\\EffectsCore\\fx_update.cpp",
            107,
            0,
            "%s",
            "trail->defIndex < effectDef->elemDefCountLooping || trail->defIndex >= effectDef->elemDefCountLooping + effectDef-"
            ">elemDefCountOneShot");
    }
    if (msecWhenPlayed > msecUpdateBegin || msecUpdateBegin > msecUpdateEnd)
        MyAssertHandler(
            ".\\EffectsCore\\fx_update.cpp",
            108,
            0,
            "msecUpdateBegin not in [msecWhenPlayed, msecUpdateEnd]\n\t%g not in [%g, %g]",
            msecUpdateBegin,
            msecWhenPlayed,
            msecUpdateEnd);
    elemDef = &effect->def->elemDefs[trail->defIndex];
    if (elemDef->elemType != 3)
        MyAssertHandler(
            ".\\EffectsCore\\fx_update.cpp",
            112,
            0,
            "%s\n\t(elemDef->elemType) = %i",
            "(elemDef->elemType == FX_ELEM_TYPE_TRAIL)",
            elemDef->elemType);
    if (!elemDef->trailDef)
        MyAssertHandler(".\\EffectsCore\\fx_update.cpp", 113, 0, "%s", "elemDef->trailDef");
    normalizedTotalDistance = (distanceTravelledEnd - distanceTravelledBegin) / elemDef->trailDef->splitDist;
    v12 = distanceTravelledBegin / elemDef->trailDef->splitDist;
    v11 = floor(v12);
    v10 = v12 - v11;
    normalizedDistanceBeforeSpawn = 1.0 - v10;
    normalizedDistanceTraversed = 0.0;
    normalizedDistanceRemaining = normalizedTotalDistance;
    while (normalizedDistanceBeforeSpawn < normalizedDistanceRemaining)
    {
        normalizedDistanceTraversed = normalizedDistanceTraversed + normalizedDistanceBeforeSpawn;
        lerp = normalizedDistanceTraversed / normalizedTotalDistance;
        distSpawn = (distanceTravelledEnd - distanceTravelledBegin) * lerp + distanceTravelledBegin;
        Vec3Lerp(frameBegin->origin, frameEnd->origin, lerp, frameWhenPlayed.origin);
        Vec4Lerp(frameBegin->quat, frameEnd->quat, lerp, frameWhenPlayed.quat);
        Vec4Normalize(frameWhenPlayed.quat);
        FX_SpawnTrailElem_Cull(
            system,
            effect,
            trail,
            &frameWhenPlayed,
            msecUpdateBegin + ((msecUpdateEnd - msecUpdateBegin) * lerp),
            distSpawn);
        normalizedDistanceRemaining = normalizedDistanceRemaining - normalizedDistanceBeforeSpawn;
        normalizedDistanceBeforeSpawn = 1.0;
    }
}

void __cdecl FX_SpawnLoopingElems(
    FxSystem *system,
    FxEffect *effect,
    int elemDefIndex,
    const FxSpatialFrame *frameBegin,
    const FxSpatialFrame *frameEnd,
    long double msecWhenPlayed,
    long double msecUpdateBegin,
    long double msecUpdateEnd)
{
    const FxEffectDef *effectDef; // [esp+40h] [ebp-3Ch]
    const FxElemDef *elemDef; // [esp+44h] [ebp-38h]
    int msecNextSpawn; // [esp+48h] [ebp-34h]
    float lerp; // [esp+4Ch] [ebp-30h]
    int spawnedCount; // [esp+54h] [ebp-28h]
    FxSpatialFrame frameWhenPlayed; // [esp+58h] [ebp-24h] BYREF
    int maxUpdateMsec; // [esp+74h] [ebp-8h]
    int updateMsec; // [esp+78h] [ebp-4h]

    if (!effect)
        MyAssertHandler(".\\EffectsCore\\fx_update.cpp", 350, 0, "%s", "effect");
    effectDef = effect->def;
    if (!effect->def)
        MyAssertHandler(".\\EffectsCore\\fx_update.cpp", 353, 0, "%s", "effectDef");
    if (elemDefIndex >= (effectDef->elemDefCountEmission + effectDef->elemDefCountOneShot
        + effectDef->elemDefCountLooping))
        MyAssertHandler(
            ".\\EffectsCore\\fx_update.cpp",
            355,
            0,
            "elemDefIndex doesn't index effectDef->elemDefCountLooping + effectDef->elemDefCountOneShot + effectDef->elemDefCou"
            "ntEmission\n"
            "\t%i not in [0, %i)",
            elemDefIndex,
            effectDef->elemDefCountEmission + effectDef->elemDefCountOneShot + effectDef->elemDefCountLooping);
    if (elemDefIndex >= effectDef->elemDefCountLooping
        && elemDefIndex < effectDef->elemDefCountOneShot + effectDef->elemDefCountLooping)
    {
        MyAssertHandler(
            ".\\EffectsCore\\fx_update.cpp",
            356,
            0,
            "%s",
            "elemDefIndex < effectDef->elemDefCountLooping || elemDefIndex >= effectDef->elemDefCountLooping + effectDef->elemDefCountOneShot");
    }
    if (msecWhenPlayed > msecUpdateBegin || msecUpdateBegin > msecUpdateEnd)
        MyAssertHandler(
            ".\\EffectsCore\\fx_update.cpp",
            357,
            0,
            "msecUpdateBegin not in [msecWhenPlayed, msecUpdateEnd]\n\t%g not in [%g, %g]",
            msecUpdateBegin,
            msecWhenPlayed,
            msecUpdateEnd);
    elemDef = &effect->def->elemDefs[elemDefIndex];
    if (elemDef->elemType != 3)
    {
        if (msecUpdateEnd != 0x7FFFFFFF)
        {
            updateMsec = msecUpdateEnd - msecUpdateBegin;
            if (msecUpdateEnd - msecUpdateBegin > 128)
            {
                maxUpdateMsec = FX_LimitStabilizeTimeForElemDef_Recurse(elemDef, 0, updateMsec) + 1;
                elemDef = &effect->def->elemDefs[elemDefIndex];
                if (updateMsec > maxUpdateMsec)
                    msecUpdateBegin += updateMsec - maxUpdateMsec;
            }
        }
        spawnedCount = (msecUpdateBegin - msecWhenPlayed) / elemDef->spawn.looping.intervalMsec + 1;
        msecNextSpawn = msecWhenPlayed + elemDef->spawn.looping.intervalMsec * spawnedCount;
        qmemcpy(&frameWhenPlayed, frameBegin, sizeof(frameWhenPlayed));
        while (msecNextSpawn <= msecUpdateEnd && spawnedCount < elemDef->spawn.looping.count)
        {
            lerp = (msecNextSpawn - msecUpdateBegin) / (msecUpdateEnd - msecUpdateBegin);
            Vec3Lerp(frameBegin->origin, frameEnd->origin, lerp, frameWhenPlayed.origin);
            Vec4Lerp(frameBegin->quat, frameEnd->quat, lerp, frameWhenPlayed.quat);
            Vec4Normalize(frameWhenPlayed.quat);
            FX_SpawnElem(system, effect, elemDefIndex, &frameWhenPlayed, msecNextSpawn, 0.0, spawnedCount);
            elemDef = &effect->def->elemDefs[elemDefIndex];
            ++spawnedCount;
            msecNextSpawn += elemDef->spawn.looping.intervalMsec;
        }
    }
}

void __cdecl FX_SpawnAllFutureLooping(
    FxSystem *system,
    FxEffect *effect,
    int elemDefFirst,
    int elemDefCount,
    const FxSpatialFrame *frameBegin,
    const FxSpatialFrame *frameEnd,
    long double msecWhenPlayed,
    long double msecUpdateBegin,
    long double msecUpdateEnd)
{
    long double v9; // [esp-4h] [ebp-6Ch]
    long double v10; // [esp+4h] [ebp-64h]
    int elemDefIndex; // [esp+64h] [ebp-4h]

    if (!effect)
        MyAssertHandler(".\\EffectsCore\\fx_update.cpp", 465, 0, "%s", "effect");
    if (!effect->def)
        MyAssertHandler(".\\EffectsCore\\fx_update.cpp", 468, 0, "%s", "effectDef");
    for (elemDefIndex = elemDefFirst; elemDefIndex != elemDefCount + elemDefFirst; ++elemDefIndex)
    {
        if (effect->def->elemDefs[elemDefIndex].spawn.looping.count != 0x7FFFFFFF)
        {
            LODWORD(v9) = 0x7FFFFFFF;
            FX_SpawnLoopingElems(system, effect, elemDefIndex, frameBegin, frameEnd, msecWhenPlayed, msecUpdateBegin, msecUpdateEnd);
        }
    }
}

int __cdecl FX_LimitStabilizeTimeForElemDef_Recurse(
    const FxElemDef *elemDef,
    bool needToSpawnSystem,
    int originalUpdateTime)
{
    int v5; // [esp+4h] [ebp-40h]
    int v6; // [esp+8h] [ebp-3Ch]
    int v7; // [esp+Ch] [ebp-38h]
    int v8; // [esp+10h] [ebp-34h]
    int v9; // [esp+14h] [ebp-30h]
    int v10; // [esp+18h] [ebp-2Ch]
    int v11; // [esp+1Ch] [ebp-28h]
    int v12; // [esp+20h] [ebp-24h]
    int v13; // [esp+24h] [ebp-20h]
    int selfStabilizeTime; // [esp+30h] [ebp-14h]
    FxElemVisuals *visArray; // [esp+34h] [ebp-10h]
    int maxStabilizeTime; // [esp+38h] [ebp-Ch]
    int visIndex; // [esp+40h] [ebp-4h]

    if (!elemDef)
        MyAssertHandler(".\\EffectsCore\\fx_update.cpp", 243, 0, "%s", "elemDef");
    selfStabilizeTime = FX_LimitStabilizeTimeForElemDef_SelfOnly(elemDef, needToSpawnSystem);
    maxStabilizeTime = selfStabilizeTime;
    if (selfStabilizeTime >= originalUpdateTime)
        return originalUpdateTime;
    if (elemDef->elemType == 10)
    {
        if (elemDef->visualCount == 1)
        {
            v13 = FX_LimitStabilizeTimeForEffectDef_Recurse(elemDef->visuals.instance.effectDef.handle, originalUpdateTime);
            if (selfStabilizeTime < v13)
                v8 = v13;
            else
                v8 = selfStabilizeTime;
            maxStabilizeTime = v8;
            if (v8 >= originalUpdateTime)
                return originalUpdateTime;
        }
        else
        {
            visArray = elemDef->visuals.array;
            for (visIndex = 0; visIndex < elemDef->visualCount; ++visIndex)
            {
                v12 = FX_LimitStabilizeTimeForEffectDef_Recurse(visArray[visIndex].effectDef.handle, originalUpdateTime);
                if (maxStabilizeTime < v12)
                    v7 = v12;
                else
                    v7 = maxStabilizeTime;
                maxStabilizeTime = v7;
                if (v7 >= originalUpdateTime)
                    return originalUpdateTime;
            }
        }
    }
    if (elemDef->effectOnDeath.handle)
    {
        v11 = selfStabilizeTime
            + FX_LimitStabilizeTimeForEffectDef_Recurse(elemDef->effectOnDeath.handle, originalUpdateTime);
        v6 = maxStabilizeTime < v11 ? v11 : maxStabilizeTime;
        maxStabilizeTime = v6;
        if (v6 >= originalUpdateTime)
            return originalUpdateTime;
    }
    if (elemDef->effectOnImpact.handle)
    {
        v10 = selfStabilizeTime
            + FX_LimitStabilizeTimeForEffectDef_Recurse(elemDef->effectOnImpact.handle, originalUpdateTime);
        v5 = maxStabilizeTime < v10 ? v10 : maxStabilizeTime;
        maxStabilizeTime = v5;
        if (v5 >= originalUpdateTime)
            return originalUpdateTime;
    }
    if (elemDef->effectEmitted.handle)
    {
        v9 = selfStabilizeTime
            + FX_LimitStabilizeTimeForEffectDef_Recurse(elemDef->effectEmitted.handle, originalUpdateTime);
        if (maxStabilizeTime < v9)
            return v9;
        else
            return maxStabilizeTime;
    }
    return maxStabilizeTime;
}

int __cdecl FX_LimitStabilizeTimeForElemDef_SelfOnly(const FxElemDef *elemDef, bool needToSpawnSystem)
{
    int result; // [esp+4h] [ebp-4h]

    if (elemDef->elemType == 3)
        return 0x7FFFFFFF;
    result = elemDef->spawnDelayMsec.amplitude
        + elemDef->spawnDelayMsec.base
        + elemDef->lifeSpanMsec.amplitude
        + elemDef->lifeSpanMsec.base;
    if (needToSpawnSystem && elemDef->spawn.looping.count > 0)
    {
        if (elemDef->spawn.looping.count == 0x7FFFFFFF)
            return 0x7FFFFFFF;
        result += elemDef->spawn.looping.intervalMsec * (elemDef->spawn.looping.count - 1);
    }
    return result;
}

int __cdecl FX_LimitStabilizeTimeForEffectDef_Recurse(const FxEffectDef *remoteEffectDef, int originalUpdateTime)
{
    int v3; // [esp+0h] [ebp-1Ch]
    int v4; // [esp+4h] [ebp-18h]
    int elemIter; // [esp+10h] [ebp-Ch]
    int maxStabilizeTime; // [esp+14h] [ebp-8h]
    int elemCount; // [esp+18h] [ebp-4h]

    if (!remoteEffectDef)
    {
        MyAssertHandler(".\\EffectsCore\\fx_update.cpp", 182, 0, "%s", "remoteEffectDef");
        MyAssertHandler(".\\EffectsCore\\fx_update.cpp", 197, 0, "%s", "effectDef");
    }
    elemCount = remoteEffectDef->elemDefCountEmission
        + remoteEffectDef->elemDefCountOneShot
        + remoteEffectDef->elemDefCountLooping;
    maxStabilizeTime = 0;
    for (elemIter = 0; elemIter != elemCount; ++elemIter)
    {
        v4 = FX_LimitStabilizeTimeForElemDef_Recurse(&remoteEffectDef->elemDefs[elemIter], 1, originalUpdateTime);
        if (maxStabilizeTime < v4)
            v3 = v4;
        else
            v3 = maxStabilizeTime;
        maxStabilizeTime = v3;
        if (v3 >= originalUpdateTime)
            return originalUpdateTime;
    }
    return maxStabilizeTime;
}

void __cdecl FX_BeginLooping(
    FxSystem* system,
    FxEffect* effect,
    int elemDefFirst,
    int elemDefCount,
    FxSpatialFrame* frameWhenPlayed,
    FxSpatialFrame* a2,
    int msecWhenPlayed,
    int msecNow)
{
    const FxElemDef* elemDef; // [esp+5Ch] [ebp-18h]
    unsigned __int16 trailHandle; // [esp+60h] [ebp-14h]
    FxPool<FxTrail>* trail; // [esp+64h] [ebp-10h]
    int elemDefStop; // [esp+6Ch] [ebp-8h]
    int elemDefIndex; // [esp+70h] [ebp-4h]
    int elemDefIndexa; // [esp+70h] [ebp-4h]

    if (!effect)
        MyAssertHandler(".\\EffectsCore\\fx_update.cpp", 492, 0, "%s", "effect");
    if (!effect->def)
        MyAssertHandler(".\\EffectsCore\\fx_update.cpp", 495, 0, "%s", "effectDef");
    elemDefStop = elemDefCount + elemDefFirst;
    for (elemDefIndex = elemDefFirst; elemDefIndex != elemDefStop; ++elemDefIndex)
    {
        if (effect->def->elemDefs[elemDefIndex].elemType != 3)
            FX_SpawnElem(system, effect, elemDefIndex, frameWhenPlayed, msecWhenPlayed, 0.0, 0);
    }
    for (trailHandle = effect->firstTrailHandle; trailHandle != 0xFFFF; trailHandle = trail->item.nextTrailHandle)
    {
        if (!system)
            MyAssertHandler("c:\\trees\\cod3\\src\\effectscore\\fx_system.h", 362, 0, "%s", "system");
        trail = FX_PoolFromHandle_Generic<FxTrail, 128>(system->trails, trailHandle);
        elemDefIndexa = trail->item.defIndex;
        if (elemDefIndexa >= elemDefFirst && elemDefIndexa < elemDefStop)
        {
            elemDef = &effect->def->elemDefs[elemDefIndexa];
            if (elemDef->elemType != 3)
                MyAssertHandler(
                    ".\\EffectsCore\\fx_update.cpp",
                    517,
                    0,
                    "%s\n\t(elemDef->elemType) = %i",
                    "(elemDef->elemType == FX_ELEM_TYPE_TRAIL)",
                    elemDef->elemType);
            FX_SpawnTrailElem_NoCull(system, effect, &trail->item, frameWhenPlayed, msecWhenPlayed, 0.0);
            if (msecNow <= msecWhenPlayed)
            {
                FX_SpawnTrailElem_NoCull(system, effect, &trail->item, frameWhenPlayed, msecWhenPlayed, 0.0);
            }
            else
            {
                FX_SpawnTrailLoopingElems(
                    system,
                    effect,
                    &trail->item,
                    frameWhenPlayed,
                    a2,
                    msecWhenPlayed,
                    msecWhenPlayed,
                    msecNow,
                    0.0,
                    effect->distanceTraveled);
                FX_SpawnTrailElem_NoCull(system, effect, &trail->item, a2, msecNow, 0.0);
            }
        }
    }
}

void __cdecl FX_TriggerOneShot(
    FxSystem *system,
    FxEffect *effect,
    int elemDefFirst,
    int elemDefCount,
    const FxSpatialFrame *frameWhenPlayed,
    int msecWhenPlayed)
{
    const FxEffectDef *effectDef; // [esp+14h] [ebp-Ch]
    int elemDefIndex; // [esp+1Ch] [ebp-4h]

    if (!effect)
        MyAssertHandler(".\\EffectsCore\\fx_update.cpp", 542, 0, "%s", "effect");
    effectDef = effect->def;
    if (!effect->def)
        MyAssertHandler(".\\EffectsCore\\fx_update.cpp", 545, 0, "%s", "effectDef");
    if (elemDefCount
        && (elemDefFirst < 0
            || elemDefFirst >= effectDef->elemDefCountEmission
            + effectDef->elemDefCountOneShot
            + effectDef->elemDefCountLooping))
    {
        MyAssertHandler(
            ".\\EffectsCore\\fx_update.cpp",
            547,
            0,
            "%s",
            "elemDefCount == 0 || (elemDefFirst >= 0 && elemDefFirst < effectDef->elemDefCountLooping + effectDef->elemDefCount"
            "OneShot + effectDef->elemDefCountEmission)");
    }
    if (elemDefCount < 0
        || elemDefCount + elemDefFirst > effectDef->elemDefCountEmission
        + effectDef->elemDefCountOneShot
        + effectDef->elemDefCountLooping)
    {
        MyAssertHandler(
            ".\\EffectsCore\\fx_update.cpp",
            548,
            0,
            "%s",
            "elemDefCount >= 0 && elemDefFirst + elemDefCount <= effectDef->elemDefCountLooping + effectDef->elemDefCountOneSho"
            "t + effectDef->elemDefCountEmission");
    }
    for (elemDefIndex = elemDefFirst; elemDefIndex != elemDefCount + elemDefFirst; ++elemDefIndex)
        FX_SpawnOneShotElems(system, effect, elemDefIndex, frameWhenPlayed, msecWhenPlayed);
}

void __cdecl FX_SpawnOneShotElems(
    FxSystem* system,
    FxEffect* effect,
    int elemDefIndex,
    const FxSpatialFrame* frameWhenPlayed,
    int msecWhenPlayed)
{
    const FxElemDef* elemDef; // [esp+10h] [ebp-Ch]
    int spawnCount; // [esp+14h] [ebp-8h]
    int spawnIndex; // [esp+18h] [ebp-4h]

    if (!effect)
        MyAssertHandler(".\\EffectsCore\\fx_update.cpp", 405, 0, "%s", "effect");
    if (!effect->def)
        MyAssertHandler(".\\EffectsCore\\fx_update.cpp", 408, 0, "%s", "effectDef");
    elemDef = &effect->def->elemDefs[elemDefIndex];
    if (elemDef->elemType != 3)
    {
        spawnCount = elemDef->spawn.looping.intervalMsec;
        if (elemDef->spawn.looping.count)
            spawnCount += ((elemDef->spawn.looping.count + 1) * LOWORD(fx_randomTable[effect->randomSeed + 19])) >> 16;
        for (spawnIndex = 0; spawnIndex < spawnCount; ++spawnIndex)
            FX_SpawnElem(system, effect, elemDefIndex, frameWhenPlayed, msecWhenPlayed, 0.0, spawnIndex);
    }
}

void __cdecl FX_StartNewEffect(FxSystem* system, FxEffect* effect)
{
    const FxEffectDef* def; // [esp+0h] [ebp-4h]

    def = effect->def;
    if (!effect->def)
        MyAssertHandler(".\\EffectsCore\\fx_update.cpp", 562, 0, "%s", "def");
    FX_BeginLooping(
        system,
        effect,
        0,
        def->elemDefCountLooping,
        &effect->frameAtSpawn,
        &effect->frameNow,
        effect->msecBegin,
        system->msecNow);
    FX_TriggerOneShot(
        system,
        effect,
        def->elemDefCountLooping,
        def->elemDefCountOneShot,
        &effect->frameAtSpawn,
        effect->msecBegin);
    FX_SortNewElemsInEffect(system, effect);
}

bool __cdecl FX_GetBoltTemporalBits(int localClientNum, int dobjHandle)
{
    return dobjHandle < 1022 && (CG_GetEntity(localClientNum, dobjHandle)->nextState.lerp.eFlags & 2) != 0;
}

char __cdecl FX_GetBoneOrientation(int localClientNum, unsigned int dobjHandle, int boneIndex, orientation_t *orient)
{
    DObj_s *obj; // [esp+60h] [ebp-8h]
    centity_s *pose; // [esp+64h] [ebp-4h]

    if (dobjHandle >= 0x480)
        MyAssertHandler(
            ".\\EffectsCore\\fx_update.cpp",
            1351,
            0,
            "dobjHandle doesn't index CLIENT_DOBJ_HANDLE_MAX\n\t%i not in [0, %i)",
            dobjHandle,
            1152);
    if (!orient)
        MyAssertHandler(".\\EffectsCore\\fx_update.cpp", 1352, 0, "%s", "orient");
    if (!FX_GetBoneOrientation_IsDObjEntityValid(localClientNum, dobjHandle))
        return 0;
    if (boneIndex >= 0)
    {
        obj = Com_GetClientDObj(dobjHandle, localClientNum);
        if (obj)
        {
            if (boneIndex < DObjNumBones(obj))
            {
                pose = (centity_s *)CG_GetPose(localClientNum, dobjHandle);
                if (CG_DObjGetWorldBoneMatrix(&pose->pose, obj, boneIndex, orient->axis, orient->origin))
                {
                    if ((COERCE_UNSIGNED_INT(orient->origin[0]) & 0x7F800000) == 0x7F800000
                        || (COERCE_UNSIGNED_INT(orient->origin[1]) & 0x7F800000) == 0x7F800000
                        || (COERCE_UNSIGNED_INT(orient->origin[2]) & 0x7F800000) == 0x7F800000)
                    {
                        MyAssertHandler(
                            ".\\EffectsCore\\fx_update.cpp",
                            1381,
                            0,
                            "%s",
                            "!IS_NAN((orient->origin)[0]) && !IS_NAN((orient->origin)[1]) && !IS_NAN((orient->origin)[2])");
                    }
                    if ((COERCE_UNSIGNED_INT(orient->axis[0][0]) & 0x7F800000) == 0x7F800000
                        || (COERCE_UNSIGNED_INT(orient->axis[0][1]) & 0x7F800000) == 0x7F800000
                        || (COERCE_UNSIGNED_INT(orient->axis[0][2]) & 0x7F800000) == 0x7F800000)
                    {
                        MyAssertHandler(
                            ".\\EffectsCore\\fx_update.cpp",
                            1382,
                            0,
                            "%s",
                            "!IS_NAN((orient->axis[0])[0]) && !IS_NAN((orient->axis[0])[1]) && !IS_NAN((orient->axis[0])[2])");
                    }
                    if ((COERCE_UNSIGNED_INT(orient->axis[1][0]) & 0x7F800000) == 0x7F800000
                        || (COERCE_UNSIGNED_INT(orient->axis[1][1]) & 0x7F800000) == 0x7F800000
                        || (COERCE_UNSIGNED_INT(orient->axis[1][2]) & 0x7F800000) == 0x7F800000)
                    {
                        MyAssertHandler(
                            ".\\EffectsCore\\fx_update.cpp",
                            1383,
                            0,
                            "%s",
                            "!IS_NAN((orient->axis[1])[0]) && !IS_NAN((orient->axis[1])[1]) && !IS_NAN((orient->axis[1])[2])");
                    }
                    if ((COERCE_UNSIGNED_INT(orient->axis[2][0]) & 0x7F800000) == 0x7F800000
                        || (COERCE_UNSIGNED_INT(orient->axis[2][1]) & 0x7F800000) == 0x7F800000
                        || (COERCE_UNSIGNED_INT(orient->axis[2][2]) & 0x7F800000) == 0x7F800000)
                    {
                        MyAssertHandler(
                            ".\\EffectsCore\\fx_update.cpp",
                            1384,
                            0,
                            "%s",
                            "!IS_NAN((orient->axis[2])[0]) && !IS_NAN((orient->axis[2])[1]) && !IS_NAN((orient->axis[2])[2])");
                    }
                    return 1;
                }
                else
                {
                    return 0;
                }
            }
            else
            {
                return 0;
            }
        }
        else
        {
            return 0;
        }
    }
    else
    {
        CG_GetDObjOrientation(localClientNum, dobjHandle, orient->axis, orient->origin);
        if ((COERCE_UNSIGNED_INT(orient->axis[0][0]) & 0x7F800000) == 0x7F800000
            || (COERCE_UNSIGNED_INT(orient->axis[0][1]) & 0x7F800000) == 0x7F800000
            || (COERCE_UNSIGNED_INT(orient->axis[0][2]) & 0x7F800000) == 0x7F800000)
        {
            MyAssertHandler(
                ".\\EffectsCore\\fx_update.cpp",
                1363,
                0,
                "%s",
                "!IS_NAN((orient->axis[0])[0]) && !IS_NAN((orient->axis[0])[1]) && !IS_NAN((orient->axis[0])[2])");
        }
        if ((COERCE_UNSIGNED_INT(orient->axis[1][0]) & 0x7F800000) == 0x7F800000
            || (COERCE_UNSIGNED_INT(orient->axis[1][1]) & 0x7F800000) == 0x7F800000
            || (COERCE_UNSIGNED_INT(orient->axis[1][2]) & 0x7F800000) == 0x7F800000)
        {
            MyAssertHandler(
                ".\\EffectsCore\\fx_update.cpp",
                1364,
                0,
                "%s",
                "!IS_NAN((orient->axis[1])[0]) && !IS_NAN((orient->axis[1])[1]) && !IS_NAN((orient->axis[1])[2])");
        }
        if ((COERCE_UNSIGNED_INT(orient->axis[2][0]) & 0x7F800000) == 0x7F800000
            || (COERCE_UNSIGNED_INT(orient->axis[2][1]) & 0x7F800000) == 0x7F800000
            || (COERCE_UNSIGNED_INT(orient->axis[2][2]) & 0x7F800000) == 0x7F800000)
        {
            MyAssertHandler(
                ".\\EffectsCore\\fx_update.cpp",
                1365,
                0,
                "%s",
                "!IS_NAN((orient->axis[2])[0]) && !IS_NAN((orient->axis[2])[1]) && !IS_NAN((orient->axis[2])[2])");
        }
        if ((COERCE_UNSIGNED_INT(orient->origin[0]) & 0x7F800000) == 0x7F800000
            || (COERCE_UNSIGNED_INT(orient->origin[1]) & 0x7F800000) == 0x7F800000
            || (COERCE_UNSIGNED_INT(orient->origin[2]) & 0x7F800000) == 0x7F800000)
        {
            MyAssertHandler(
                ".\\EffectsCore\\fx_update.cpp",
                1366,
                0,
                "%s",
                "!IS_NAN((orient->origin)[0]) && !IS_NAN((orient->origin)[1]) && !IS_NAN((orient->origin)[2])");
        }
        return 1;
    }
}

bool __cdecl FX_GetBoneOrientation_IsDObjEntityValid(int localClientNum, int dobjHandle)
{
    return dobjHandle >= 1022 || CG_GetEntity(localClientNum, dobjHandle)->nextValid;
}

void __cdecl FX_UpdateEffectPartial(
    FxSystem* system,
    FxEffect* effect,
    int msecUpdateBegin,
    int msecUpdateEnd,
    float distanceTravelledBegin,
    float distanceTravelledEnd,
    unsigned __int16* elemHandleStart,
    unsigned __int16* elemHandleStop,
    unsigned __int16* trailElemStart,
    unsigned __int16* trailElemStop)
{
    int v10; // edx
    double v11; // [esp+4h] [ebp-3Ch]
    unsigned int v12; // [esp+14h] [ebp-2Ch]
    unsigned __int16 v13; // [esp+14h] [ebp-2Ch]
    unsigned __int16 v14; // [esp+18h] [ebp-28h]
    const FxEffectDef* def; // [esp+1Ch] [ebp-24h]
    unsigned __int16 trailHandle; // [esp+24h] [ebp-1Ch]
    FxTrail trail; // [esp+28h] [ebp-18h] BYREF
    unsigned int trailIter; // [esp+30h] [ebp-10h]
    FxTrail* remoteTrail; // [esp+34h] [ebp-Ch]
    unsigned __int16 startHandle; // [esp+38h] [ebp-8h]
    unsigned int elemClass; // [esp+3Ch] [ebp-4h]

    if (effect->msecLastUpdate > msecUpdateEnd)
        MyAssertHandler(
            ".\\EffectsCore\\fx_update.cpp",
            1573,
            0,
            "effect->msecLastUpdate <= msecUpdateEnd\n\t%g, %g",
            (double)effect->msecLastUpdate,
            (double)msecUpdateEnd);
    if ((effect->status & 0x10000) != 0)
    {
        def = effect->def;
        FX_ProcessLooping(
            system,
            effect,
            0,
            effect->def->elemDefCountLooping,
            &effect->framePrev,
            &effect->frameNow,
            effect->msecBegin,
            msecUpdateBegin,
            msecUpdateEnd,
            distanceTravelledBegin,
            distanceTravelledEnd);
        if (msecUpdateEnd - effect->msecBegin > def->msecLoopingLife)
            FX_StopEffect(system, effect);
    }
    for (elemClass = 0; elemClass < 3; ++elemClass)
    {
        FX_UpdateEffectPartialForClass(
            system,
            effect,
            msecUpdateBegin,
            msecUpdateEnd,
            elemHandleStart[elemClass],
            elemHandleStop[elemClass],
            elemClass);
    }
    trailIter = 0;
    for (trailHandle = effect->firstTrailHandle; trailHandle != 0xFFFF; trailHandle = trail.nextTrailHandle)
    {
        if (!system)
            MyAssertHandler("c:\\trees\\cod3\\src\\effectscore\\fx_system.h", 362, 0, "%s", "system");
        remoteTrail = (FxTrail*)FX_PoolFromHandle_Generic<FxTrail, 128>(system->trails, trailHandle);
        v10 = *(_DWORD*)&remoteTrail->lastElemHandle;
        *(_DWORD*)&trail.nextTrailHandle = *(_DWORD*)&remoteTrail->nextTrailHandle;
        *(_DWORD*)&trail.lastElemHandle = v10;
        if (trailElemStart)
            v14 = trailElemStart[trailIter];
        else
            v14 = -1;
        startHandle = v14;
        if (trailElemStop)
            v13 = trailElemStop[trailIter];
        else
            v13 = -1;
        FX_UpdateEffectPartialTrail(
            system,
            effect,
            &trail,
            msecUpdateBegin,
            msecUpdateEnd,
            distanceTravelledBegin,
            distanceTravelledEnd,
            startHandle,
            v13,
            &effect->frameNow);
        *remoteTrail = trail;
        ++trailIter;
    }
    effect->msecLastUpdate = msecUpdateEnd;
}

void __cdecl FX_ProcessLooping(
    FxSystem* system,
    FxEffect* effect,
    int elemDefFirst,
    int elemDefCount,
    FxSpatialFrame* frameBegin,
    FxSpatialFrame* frameEnd,
    int msecWhenPlayed,
    int msecUpdateBegin,
    int msecUpdateEnd,
    float distanceTravelledBegin,
    float distanceTravelledEnd)
{
    unsigned __int16 trailHandle; // [esp+60h] [ebp-14h]
    FxPool<FxTrail>* trail; // [esp+64h] [ebp-10h]
    int elemDefEnd; // [esp+6Ch] [ebp-8h]
    int elemDefIndex; // [esp+70h] [ebp-4h]
    int elemDefIndexa; // [esp+70h] [ebp-4h]

    if (!effect)
        MyAssertHandler(".\\EffectsCore\\fx_update.cpp", 434, 0, "%s", "effect");
    if (!effect->def)
        MyAssertHandler(".\\EffectsCore\\fx_update.cpp", 437, 0, "%s", "effectDef");
    elemDefEnd = elemDefCount + elemDefFirst;
    for (elemDefIndex = elemDefFirst; elemDefIndex != elemDefEnd; ++elemDefIndex)
        FX_SpawnLoopingElems(
            system,
            effect,
            elemDefIndex,
            frameBegin,
            frameEnd,
            msecWhenPlayed,
            msecUpdateBegin,
            msecUpdateEnd);
    for (trailHandle = effect->firstTrailHandle; trailHandle != 0xFFFF; trailHandle = trail->item.nextTrailHandle)
    {
        if (!system)
            MyAssertHandler("c:\\trees\\cod3\\src\\effectscore\\fx_system.h", 362, 0, "%s", "system");
        trail = FX_PoolFromHandle_Generic<FxTrail, 128>(system->trails, trailHandle);
        elemDefIndexa = trail->item.defIndex;
        if (elemDefIndexa >= elemDefFirst && elemDefIndexa < elemDefEnd)
            FX_SpawnTrailLoopingElems(
                system,
                effect,
                &trail->item,
                frameBegin,
                frameEnd,
                msecWhenPlayed,
                msecUpdateBegin,
                msecUpdateEnd,
                distanceTravelledBegin,
                distanceTravelledEnd);
    }
}


void __cdecl FX_UpdateEffectPartialForClass(
    FxSystem* system,
    FxEffect* effect,
    int msecUpdateBegin,
    int msecUpdateEnd,
    unsigned __int16 elemHandleStart,
    unsigned __int16 elemHandleStop,
    unsigned int elemClass)
{
    int v7; // [esp+Ch] [ebp-28h]
    int lifeSpan; // [esp+14h] [ebp-20h]
    unsigned __int16 elemHandle; // [esp+1Ch] [ebp-18h]
    FxUpdateResult updateResult; // [esp+20h] [ebp-14h]
    unsigned __int16 elemHandleNext; // [esp+24h] [ebp-10h]
    FxPool<FxElem>* elem; // [esp+28h] [ebp-Ch]
    FxPool<FxElem>* elema; // [esp+28h] [ebp-Ch]
    unsigned int passCount; // [esp+2Ch] [ebp-8h]
    unsigned __int16 elemHandleFirstExisting; // [esp+30h] [ebp-4h]

    int unk1;
    int unk2;

    if (effect->msecLastUpdate > msecUpdateEnd)
        MyAssertHandler(
            ".\\EffectsCore\\fx_update.cpp",
            1436,
            0,
            "effect->msecLastUpdate <= msecUpdateEnd\n\t%g, %g",
            effect->msecLastUpdate,
            msecUpdateEnd);
    elemHandleFirstExisting = effect->firstElemHandle[elemClass];
    passCount = 1;
    do
    {
        for (elemHandle = elemHandleStart; elemHandle != elemHandleStop; elemHandle = elemHandleNext)
        {
            if (elemHandle == 0xFFFF)
            {
                Com_Printf(0, "---- EFFECT ABOUT TO ASSERT ----\n");
                Com_Printf(0, "effect '%s' spawned at %i pass %i\n", effect->def->name, effect->msecBegin, passCount);
                Com_Printf(
                    0,
                    "looping from %i to %i, first existing is %i\n",
                    elemHandleStart,
                    elemHandleStop,
                    elemHandleFirstExisting);
                v7 = msecUpdateEnd - msecUpdateBegin;
                Com_Printf(
                    0,
                    "update period is from %d to %d (%d ms)\n", // change from lg to d
                    msecUpdateEnd, msecUpdateBegin,
                    v7);
                Com_Printf(0, "here's the active elem list:\n");
                for (elemHandle = effect->firstElemHandle[elemClass];
                    elemHandle != 0xFFFF;
                    elemHandle = elem->item.nextElemHandleInEffect)
                {
                    if (!system)
                        MyAssertHandler("c:\\trees\\cod3\\src\\effectscore\\fx_system.h", 334, 0, "%s", "system");
                    // KISAKTODO this is extremely dubious at best
                    elem = FX_PoolFromHandle_Generic<FxElem, 2048>(system->elems, elemHandle);
                    unk1 = (elem->item.msecBegin + effect->randomSeed + 296 * (unsigned int)elem->item.sequence)
                        % 0x1DF;
                    unk2 = (int)&effect->def->elemDefs[elem->item.defIndex].lifeSpanMsec;
                    lifeSpan = *(_DWORD*)unk2
                        + (((*(_DWORD*)(unk2 + 4) + 1) * LOWORD(fx_randomTable[unk1 + 17])) >> 16);
                    Com_Printf(
                        0,
                        "  elem %i def %i seq %i spawn %i die %i\n",
                        elemHandle,
                        elem->item.defIndex,
                        elem->item.sequence,
                        elem->item.msecBegin,
                        lifeSpan + elem->item.msecBegin);
                }
                if (!alwaysfails)
                    MyAssertHandler(".\\EffectsCore\\fx_update.cpp", 1467, 0, "Big bad effects assert.  Include assert log.");
            }
            if (!system)
                MyAssertHandler("c:\\trees\\cod3\\src\\effectscore\\fx_system.h", 334, 0, "%s", "system");
            elema = FX_PoolFromHandle_Generic<FxElem, 2048>(system->elems, elemHandle);
            updateResult = FX_UpdateElement(system, effect, &elema->item, msecUpdateBegin, msecUpdateEnd);
            elemHandleNext = elema->item.nextElemHandleInEffect;
            if (updateResult == FX_UPDATE_REMOVE)
            {
                FX_FreeElem(system, elemHandle, effect, elemClass);
                if (elemHandleFirstExisting == elemHandle)
                    elemHandleFirstExisting = elemHandleNext;
            }
        }
        elemHandleStop = elemHandleFirstExisting;
        elemHandleFirstExisting = effect->firstElemHandle[elemClass];
        elemHandleStart = elemHandleFirstExisting;
        ++passCount;
    } while (elemHandleFirstExisting != elemHandleStop);
}

FxUpdateResult __cdecl FX_UpdateElement(
    FxSystem *system,
    FxEffect *effect,
    FxElem *elem,
    int msecUpdateBegin,
    int msecUpdateEnd)
{
    float msec; // [esp+0h] [ebp-140h]
    bool v7; // [esp+10h] [ebp-130h]
    int physObjId; // [esp+8Ch] [ebp-B4h]
    FxUpdateElem update; // [esp+A8h] [ebp-98h] BYREF
    const FxElemDef *elemDef; // [esp+128h] [ebp-18h]
    FxUpdateResult updateResult; // [esp+12Ch] [ebp-14h] BYREF
    float elemOriginPrev[3]; // [esp+130h] [ebp-10h] BYREF
    bool goToRest; // [esp+13Fh] [ebp-1h]

    if (!elem)
        MyAssertHandler(".\\EffectsCore\\fx_update.cpp", 1266, 0, "%s", "elem");
    updateResult = FX_UPDATE_KEEP;
    if (FX_UpdateElement_SetupUpdate(
        effect,
        msecUpdateBegin,
        msecUpdateEnd,
        elem->defIndex,
        elem->atRestFraction,
        elem->msecBegin,
        elem->sequence,
        elem->origin,
        &update))
    {
        FX_UpdateElement_TruncateToElemEnd(&update, &updateResult);
        if (updateResult)
        {
            if (!FX_UpdateElement_TruncateToElemBegin(&update, &updateResult))
                return updateResult;
            elemOriginPrev[0] = elem->origin[0];
            elemOriginPrev[1] = elem->origin[1];
            elemOriginPrev[2] = elem->origin[2];
            physObjId = elem->physObjId;
            update.elemBaseVel = elem->baseVel;
            update.physObjId = physObjId;
            update.onGround = 0;
            //Profile_Begin(194);
            updateResult = (FxUpdateResult)FX_UpdateElementPosition(system, &update); // KISAKTODO type safety
            //Profile_EndInternal(0);
            FX_UpdateElement_HandleEmitting(system, elem, &update, elemOriginPrev, &updateResult);
        }
        elemDef = FX_GetUpdateElemDef(&update);
        if (updateResult)
        {
            v7 = 0;
            if (update.atRestFraction == 255
                && elemOriginPrev[0] == elem->origin[0]
                && elemOriginPrev[1] == elem->origin[1]
                && elemOriginPrev[2] == elem->origin[2])
            {
                v7 = 1;
            }
            goToRest = v7;
            if (v7 && ((elemDef->flags & 0x100) == 0 || update.onGround))
            {
                msec = (float)update.msecUpdateEnd;
                elem->atRestFraction = (int)FX_GetAtRestFraction(&update, msec);
            }
            else
            {
                elem->atRestFraction = update.atRestFraction;
            }
        }
        else if (elemDef->effectOnDeath.handle)
        {
            FX_SpawnDeathEffect(system, &update);
        }
    }
    return updateResult;
}

const FxElemDef *__cdecl FX_GetUpdateElemDef(const FxUpdateElem *update)
{
    if (!update->effect)
        MyAssertHandler(".\\EffectsCore\\fx_update.cpp", 79, 0, "%s", "update->effect");
    return &update->effect->def->elemDefs[update->elemIndex];
}

double __cdecl FX_GetAtRestFraction(const FxUpdateElem *update, float msec)
{
    float v4; // [esp+Ch] [ebp-8h]
    float v5; // [esp+10h] [ebp-4h]

    v4 = msec - (double)update->msecElemBegin;
    v5 = v4 * 255.0 / update->msecLifeSpan - 0.25;
    return (float)ceil(v5);
}

int __cdecl FX_UpdateElementPosition(FxSystem* system, FxUpdateElem* update)
{
    const FxElemDef* elemDef; // [esp+4h] [ebp-4h]

    elemDef = FX_GetUpdateElemDef(update);
    if (elemDef->elemType == 5 && (elemDef->flags & 0x8000000) != 0)
        return 1;
    if ((elemDef->flags & 0x100) != 0)
        return FX_UpdateElementPosition_Colliding(system, update);
    if ((elemDef->flags & 0x6000000) != 0)
        return FX_UpdateElementPosition_NonColliding(update);
    return FX_UpdateElementPosition_Local(update);
}

int __cdecl FX_UpdateElementPosition_Colliding(FxSystem* system, FxUpdateElem* update)
{
    int msecUpdateBegin; // [esp+0h] [ebp-14h]
    float xyzWorldOld[3]; // [esp+4h] [ebp-10h] BYREF
    int msecUpdatePartial; // [esp+10h] [ebp-4h]

    if (update->atRestFraction == 255)
    {
        FX_OrientationPosToWorldPos(&update->orient, update->elemOrigin, xyzWorldOld);
        msecUpdateBegin = update->msecUpdateBegin;
        for (msecUpdatePartial = msecUpdateBegin + 50; msecUpdatePartial < update->msecUpdateEnd; msecUpdatePartial += 50)
        {
            if (!FX_UpdateElementPosition_CollidingStep(system, update, msecUpdateBegin, msecUpdatePartial, xyzWorldOld))
                return 0;
            msecUpdateBegin = msecUpdatePartial;
        }
        return FX_UpdateElementPosition_CollidingStep(system, update, msecUpdateBegin, update->msecUpdateEnd, xyzWorldOld);
    }
    else
    {
        FX_OrientationPosToWorldPos(&update->orient, update->elemOrigin, update->posWorld);
        return 1;
    }
}

int __cdecl FX_UpdateElementPosition_CollidingStep(
        FxSystem *system,
        FxUpdateElem *update,
        int msecUpdateBegin,
        int msecUpdateEnd,
        float *xyzWorldOld)
{
  const FxElemDef *elemDef; // [esp+34h] [ebp-34h]
  int traceMask; // [esp+38h] [ebp-30h]
  trace_t trace; // [esp+3Ch] [ebp-2Ch] BYREF

  traceMask = 2065;
  elemDef = FX_GetUpdateElemDef(update);
  if ( elemDef->useItemClip )
    traceMask = 3089;
  do
  {
    update->onGround = 0;
    FX_NextElementPosition(update, msecUpdateBegin, msecUpdateEnd);
    // Profile_Begin(203);
    CM_BoxTrace(&trace, xyzWorldOld, update->posWorld, elemDef->collMins, elemDef->collMaxs, 0, traceMask);
    // Profile_EndInternal(0);
    if ( !FX_TraceHitSomething(&trace) )
      break;
    if ( trace.normal[2] > 0.699999988079071 )
      update->onGround = 1;
    msecUpdateBegin = FX_CollisionResponse(system, update, &trace, msecUpdateBegin, msecUpdateEnd, xyzWorldOld);
    elemDef = FX_GetUpdateElemDef(update);
    if ( (elemDef->flags & 0x200) != 0 )
      return 0;
  }
  while ( msecUpdateBegin != msecUpdateEnd );
  FX_OrientationPosFromWorldPos(&update->orient, update->posWorld, update->elemOrigin);
  *xyzWorldOld = update->posWorld[0];
  xyzWorldOld[1] = update->posWorld[1];
  xyzWorldOld[2] = update->posWorld[2];
  return 1;
}

void __cdecl FX_NextElementPosition(FxUpdateElem* update, int msecUpdateBegin, int msecUpdateEnd)
{
    const char* v3; // eax
    float* elemOrigin; // [esp+18h] [ebp-20h]
    float gravityScale; // [esp+1Ch] [ebp-1Ch]
    float secDuration; // [esp+20h] [ebp-18h]
    const FxElemDef* elemDef; // [esp+24h] [ebp-14h]
    float posLocal[3]; // [esp+28h] [ebp-10h] BYREF
    float deltaVelFromGravity; // [esp+34h] [ebp-4h]

    if (msecUpdateEnd - msecUpdateBegin <= 0)
    {
        v3 = va("[%d, %d]", msecUpdateEnd, msecUpdateBegin);
        MyAssertHandler(".\\EffectsCore\\fx_update.cpp", 823, 0, "%s\n\t%s", "msecUpdateEnd - msecUpdateBegin > 0", v3);
    }
    elemOrigin = update->elemOrigin;
    posLocal[0] = *elemOrigin;
    posLocal[1] = elemOrigin[1];
    posLocal[2] = elemOrigin[2];
    FX_NextElementPosition_NoExternalForces(update, msecUpdateBegin, msecUpdateEnd, posLocal, update->posWorld);
    secDuration = (double)(msecUpdateEnd - msecUpdateBegin) * 0.001000000047497451;
    Vec3Mad(update->posWorld, secDuration, update->elemBaseVel, update->posWorld);
    elemDef = FX_GetUpdateElemDef(update);
    gravityScale = elemDef->gravity.amplitude * fx_randomTable[update->randomSeed + 15] + elemDef->gravity.base;
    deltaVelFromGravity = gravityScale * 800.0 * secDuration;
    update->elemBaseVel[2] = update->elemBaseVel[2] - deltaVelFromGravity;
    update->posWorld[2] = update->posWorld[2] - deltaVelFromGravity * secDuration * 0.5;
}

void __cdecl FX_NextElementPosition_NoExternalForces(
    FxUpdateElem* update,
    int msecUpdateBegin,
    int msecUpdateEnd,
    float* posLocal,
    float* posWorld)
{
    const char* v5; // eax
    double v6; // [esp+10h] [ebp-10h]
    float normUpdateEnd; // [esp+18h] [ebp-8h]
    float normUpdateBegin; // [esp+1Ch] [ebp-4h]

    if (msecUpdateEnd - msecUpdateBegin <= 0)
    {
        v5 = va("[%d, %d]", msecUpdateEnd, msecUpdateBegin, msecUpdateBegin, msecUpdateEnd);
        MyAssertHandler(".\\EffectsCore\\fx_update.cpp", 804, 0, "%s\n\t%s", "msecUpdateEnd - msecUpdateBegin > 0", v5);
    }
    if (!posLocal)
        MyAssertHandler(".\\EffectsCore\\fx_update.cpp", 805, 0, "%s", "posLocal");
    normUpdateBegin = (double)(msecUpdateBegin - update->msecElemBegin) / update->msecLifeSpan;
    normUpdateEnd = (double)(msecUpdateEnd - update->msecElemBegin) / update->msecLifeSpan;
    FX_IntegrateVelocity(update, normUpdateBegin, normUpdateEnd, posLocal, posWorld);
}

void __cdecl FX_IntegrateVelocity(const FxUpdateElem *update, float t0, float t1, float *posLocal, float *posWorld)
{
    const char *v5; // eax
    const char *v6; // eax
    char *v7; // eax
    char *v8; // eax
    double v9; // [esp+18h] [ebp-80h]
    int v10; // [esp+20h] [ebp-78h]
    int v11; // [esp+24h] [ebp-74h]
    float integralScale; // [esp+64h] [ebp-34h]
    float startPoint; // [esp+68h] [ebp-30h]
    float endPoint; // [esp+6Ch] [ebp-2Ch]
    const FxElemDef *elemDef; // [esp+70h] [ebp-28h]
    float startLerp; // [esp+74h] [ebp-24h]
    int startIndex; // [esp+78h] [ebp-20h]
    const FxElemVelStateSample *samples; // [esp+7Ch] [ebp-1Ch]
    int intervalCount; // [esp+80h] [ebp-18h]
    int endIndex; // [esp+84h] [ebp-14h]
    float rangeLerp[3]; // [esp+88h] [ebp-10h] BYREF
    float endLerp; // [esp+94h] [ebp-4h]

    if (!update)
        MyAssertHandler(".\\EffectsCore\\fx_update.cpp", 759, 0, "%s", "update");
    elemDef = FX_GetUpdateElemDef(update);
    if (!elemDef)
        MyAssertHandler(".\\EffectsCore\\fx_update.cpp", 762, 0, "%s", "elemDef");
    if (!elemDef->velSamples)
        MyAssertHandler(".\\EffectsCore\\fx_update.cpp", 763, 0, "%s", "elemDef->velSamples");
    if (!elemDef->velIntervalCount)
        MyAssertHandler(
            ".\\EffectsCore\\fx_update.cpp",
            764,
            0,
            "%s\n\t(elemDef->velIntervalCount) = %i",
            "(elemDef->velIntervalCount >= 1)",
            elemDef->velIntervalCount);
    if (t0 < 0.0 || t1 <= (double)t0 || t1 > 1.0)
    {
        v5 = va("%g, %g", t0, t1);
        MyAssertHandler(".\\EffectsCore\\fx_update.cpp", 765, 0, "%s\n\t%s", "0.0f <= t0 && t0 < t1 && t1 <= 1.0f", v5);
    }
    rangeLerp[0] = fx_randomTable[update->randomSeed];
    rangeLerp[1] = fx_randomTable[update->randomSeed + 1];
    rangeLerp[2] = fx_randomTable[update->randomSeed + 2];
    integralScale = update->msecLifeSpan;
    samples = elemDef->velSamples;
    intervalCount = elemDef->velIntervalCount;
    if (intervalCount == 1)
    {
        FX_IntegrateVelocityInSegment(
            elemDef->flags,
            &update->orient,
            samples,
            t0,
            t1,
            rangeLerp,
            integralScale,
            posLocal,
            posWorld);
    }
    else
    {
        startPoint = (double)intervalCount * t0;
        *((float *)&v9 + 1) = floor(startPoint);
        startIndex = (int)*((float *)&v9 + 1);
        startLerp = startPoint - (double)startIndex;
        endPoint = (double)intervalCount * t1;
        *(float *)&v9 = ceil(endPoint);
        endIndex = (int)*(float *)&v9 - 1;
        endLerp = endPoint - (double)endIndex;
        if (startIndex > endIndex)
        {
            v6 = va("%i > %i for %g to %g on %i intervals", startIndex, endIndex, t0, t1, intervalCount);
            MyAssertHandler(".\\EffectsCore\\fx_update.cpp", 788, 1, "%s\n\t%s", "startIndex <= endIndex", v6);
        }
        if (startIndex < 0 || startIndex >= intervalCount)
        {
            iassert(0);
            //v7 = va("%i for %g on %i intervals", startIndex, t0, intervalCount);
            //MyAssertHandler(
            //    ".\\EffectsCore\\fx_update.cpp",
            //    789,
            //    1,
            //    "%s\n\t(va( \"%i for %g on %i intervals\", startIndex, t0, intervalCount )) = %i",
            //    "(startIndex >= 0 && startIndex < intervalCount)",
            //    v7,
            //    v9,
            //    v10,
            //    v11);
        }
        if (endIndex < 0 || endIndex >= intervalCount)
        {
            iassert(0);
            //v8 = va("%i for %g on %i intervals", endIndex, t1, intervalCount);
            //MyAssertHandler(
            //    ".\\EffectsCore\\fx_update.cpp",
            //    790,
            //    1,
            //    "%s\n\t(va( \"%i for %g on %i intervals\", endIndex, t1, intervalCount )) = %i",
            //    "(endIndex >= 0 && endIndex < intervalCount)",
            //    v8,
            //    v9,
            //    v10,
            //    v11);
        }
        if (startIndex == endIndex)
            FX_IntegrateVelocityInSegment(
                elemDef->flags,
                &update->orient,
                &samples[startIndex],
                startLerp,
                endLerp,
                rangeLerp,
                integralScale,
                posLocal,
                posWorld);
        else
            FX_IntegrateVelocityAcrossSegments(
                elemDef->flags,
                &update->orient,
                &samples[startIndex],
                &samples[endIndex],
                startLerp,
                endLerp,
                rangeLerp,
                integralScale,
                posLocal,
                posWorld);
    }
}

void __cdecl FX_IntegrateVelocityAcrossSegments(
    int elemDefFlags,
    const orientation_t *orient,
    const FxElemVelStateSample *velState0,
    const FxElemVelStateSample *velState1,
    float t0,
    float t1,
    const float *amplitudeScale,
    float integralScale,
    float *posLocal,
    float *posWorld)
{
    float v10; // [esp+10h] [ebp-28h]
    float v11; // [esp+10h] [ebp-28h]
    float t1ScaledByIntegral; // [esp+1Ch] [ebp-1Ch]
    float w0[2]; // [esp+20h] [ebp-18h] BYREF
    const FxElemVelStateSample *localVelState0; // [esp+28h] [ebp-10h]
    float w1[2]; // [esp+2Ch] [ebp-Ch] BYREF
    float t0ScaledByIntegral; // [esp+34h] [ebp-4h]

    localVelState0 = velState0;
    if (t0 < 0.0 || t0 > 1.0)
        MyAssertHandler(
            ".\\EffectsCore\\fx_update.cpp",
            672,
            0,
            "t0 not in [0.0f, 1.0f]\n\t%g not in [%g, %g]",
            t0,
            0.0,
            1.0);
    if (t1 < 0.0 || t1 > 1.0)
        MyAssertHandler(
            ".\\EffectsCore\\fx_update.cpp",
            673,
            0,
            "t1 not in [0.0f, 1.0f]\n\t%g not in [%g, %g]",
            t1,
            0.0,
            1.0);
    t0ScaledByIntegral = -integralScale * t0;
    w0[1] = t0 * 0.5 * t0ScaledByIntegral;
    w0[0] = t0ScaledByIntegral - w0[1];
    t1ScaledByIntegral = t1 * integralScale;
    w1[1] = t1 * 0.5 * t1ScaledByIntegral;
    w1[0] = t1ScaledByIntegral - w1[1];
    if ((elemDefFlags & 0x1000000) != 0)
    {
        FX_IntegrateVelocityFromZeroInSegment(
            &velState1->local,
            &velState1[1].local,
            w1,
            amplitudeScale,
            integralScale,
            posLocal);
        v10 = -integralScale;
        FX_IntegrateVelocityFromZeroInSegment(
            &localVelState0->local,
            &localVelState0[1].local,
            w0,
            amplitudeScale,
            v10,
            posLocal);
    }
    FX_OrientationPosToWorldPos(orient, posLocal, posWorld);
    if ((elemDefFlags & 0x2000000) != 0)
    {
        FX_IntegrateVelocityFromZeroInSegment(
            &velState1->world,
            &velState1[1].world,
            w1,
            amplitudeScale,
            integralScale,
            posWorld);
        v11 = -integralScale;
        FX_IntegrateVelocityFromZeroInSegment(
            &localVelState0->world,
            &localVelState0[1].world,
            w0,
            amplitudeScale,
            v11,
            posWorld);
    }
}

void __cdecl FX_IntegrateVelocityFromZeroInSegment(
    const FxElemVelStateInFrame *statePrev,
    const FxElemVelStateInFrame *stateNext,
    float *weight,
    const float *amplitudeScale,
    float integralScale,
    float *pos)
{
    *pos = (*amplitudeScale * statePrev->totalDelta.amplitude[0] + statePrev->totalDelta.base[0]) * integralScale + *pos;
    *pos = (*amplitudeScale * statePrev->velocity.amplitude[0] + statePrev->velocity.base[0]) * *weight + *pos;
    *pos = (*amplitudeScale * stateNext->velocity.amplitude[0] + stateNext->velocity.base[0]) * weight[1] + *pos;
    pos[1] = (amplitudeScale[1] * statePrev->totalDelta.amplitude[1] + statePrev->totalDelta.base[1]) * integralScale
        + pos[1];
    pos[1] = (amplitudeScale[1] * statePrev->velocity.amplitude[1] + statePrev->velocity.base[1]) * *weight + pos[1];
    pos[1] = (amplitudeScale[1] * stateNext->velocity.amplitude[1] + stateNext->velocity.base[1]) * weight[1] + pos[1];
    pos[2] = (amplitudeScale[2] * statePrev->totalDelta.amplitude[2] + statePrev->totalDelta.base[2]) * integralScale
        + pos[2];
    pos[2] = (amplitudeScale[2] * statePrev->velocity.amplitude[2] + statePrev->velocity.base[2]) * *weight + pos[2];
    pos[2] = (amplitudeScale[2] * stateNext->velocity.amplitude[2] + stateNext->velocity.base[2]) * weight[1] + pos[2];
}

void __cdecl FX_IntegrateVelocityInSegment(
    int elemDefFlags,
    const orientation_t *orient,
    const FxElemVelStateSample *velState,
    float t0,
    float t1,
    const float *amplitudeScale,
    float integralScale,
    float *posLocal,
    float *posWorld)
{
    float weight[2]; // [esp+Ch] [ebp-8h] BYREF

    if (t0 < 0.0 || t0 > 1.0)
        MyAssertHandler(".\\EffectsCore\\fx_update.cpp", 726, 0, "%s\n\t(t0) = %g", "(t0 >= 0.0f && t0 <= 1.0f)", t0);
    if (t1 < 0.0 || t1 > 1.0)
        MyAssertHandler(".\\EffectsCore\\fx_update.cpp", 727, 0, "%s\n\t(t1) = %g", "(t1 >= 0.0f && t1 <= 1.0f)", t1);
    weight[1] = integralScale * 0.5 * (t1 * t1 - t0 * t0);
    weight[0] = (t1 - t0) * integralScale - weight[1];
    if ((elemDefFlags & 0x1000000) != 0)
        FX_IntegrateVelocityInSegmentInFrame(
            &velState->local,
            &velState[1].local,
            weight,
            amplitudeScale,
            integralScale,
            posLocal);
    FX_OrientationPosToWorldPos(orient, posLocal, posWorld);
    if ((elemDefFlags & 0x2000000) != 0)
        FX_IntegrateVelocityInSegmentInFrame(
            &velState->world,
            &velState[1].world,
            weight,
            amplitudeScale,
            integralScale,
            posWorld);
}

void __cdecl FX_IntegrateVelocityInSegmentInFrame(
    const FxElemVelStateInFrame *statePrev,
    const FxElemVelStateInFrame *stateNext,
    const float *weight,
    const float *amplitudeScale,
    float integralScale,
    float *pos)
{
    *pos = (*amplitudeScale * statePrev->velocity.amplitude[0] + statePrev->velocity.base[0]) * *weight + *pos;
    *pos = (*amplitudeScale * stateNext->velocity.amplitude[0] + stateNext->velocity.base[0]) * weight[1] + *pos;
    pos[1] = (amplitudeScale[1] * statePrev->velocity.amplitude[1] + statePrev->velocity.base[1]) * *weight + pos[1];
    pos[1] = (amplitudeScale[1] * stateNext->velocity.amplitude[1] + stateNext->velocity.base[1]) * weight[1] + pos[1];
    pos[2] = (amplitudeScale[2] * statePrev->velocity.amplitude[2] + statePrev->velocity.base[2]) * *weight + pos[2];
    pos[2] = (amplitudeScale[2] * stateNext->velocity.amplitude[2] + stateNext->velocity.base[2]) * weight[1] + pos[2];
}

bool __cdecl FX_TraceHitSomething(const trace_t *trace)
{
    if (!trace)
        MyAssertHandler(".\\EffectsCore\\fx_update.cpp", 843, 0, "%s", "trace");
    return !trace->startsolid && !trace->allsolid && trace->fraction != 1.0;
}

int __cdecl FX_CollisionResponse(
    FxSystem *system,
    FxUpdateElem *update,
    const trace_t *trace,
    int msecUpdateBegin,
    int msecUpdateEnd,
    float *xyzWorldOld)
{
    const char *v7; // eax
    float fraction_4; // [esp+14h] [ebp-9Ch]
    float scale; // [esp+18h] [ebp-98h]
    float v10; // [esp+1Ch] [ebp-94h]
    float v11; // [esp+28h] [ebp-88h]
    float v12; // [esp+60h] [ebp-50h]
    float postImpactVelocity[3]; // [esp+64h] [ebp-4Ch] BYREF
    float gravityScale; // [esp+70h] [ebp-40h]
    const FxElemDef *elemDef; // [esp+74h] [ebp-3Ch]
    int msecOnImpact; // [esp+78h] [ebp-38h]
    float velDelta[3]; // [esp+7Ch] [ebp-34h] BYREF
    float msecElapsed; // [esp+88h] [ebp-28h]
    float scaledPreImpactVelocity[3]; // [esp+8Ch] [ebp-24h] BYREF
    float velocityAlongNormal; // [esp+98h] [ebp-18h]
    float preImpactVelocity[3]; // [esp+9Ch] [ebp-14h] BYREF
    float reflectionFactor; // [esp+A8h] [ebp-8h]
    float overshotDeltaVelFromGravity; // [esp+ACh] [ebp-4h]

    elemDef = FX_GetUpdateElemDef(update);
    if (msecUpdateBegin >= msecUpdateEnd)
        MyAssertHandler(".\\EffectsCore\\fx_update.cpp", 888, 0, "%s", "msecUpdateBegin < msecUpdateEnd");
    Vec3Lerp(xyzWorldOld, update->posWorld, trace->fraction, update->posWorld);
    v12 = (double)(msecUpdateEnd - msecUpdateBegin) * trace->fraction;
    v11 = floor(v12);
    msecOnImpact = msecUpdateBegin + (int)v11;
    if (msecOnImpact >= msecUpdateEnd)
        MyAssertHandler(".\\EffectsCore\\fx_update.cpp", 892, 0, "%s", "msecOnImpact < msecUpdateEnd");
    if ((elemDef->flags & 0x200) != 0 || msecOnImpact == update->msecElemEnd)
    {
        if (elemDef->effectOnImpact.handle)
            FX_SpawnImpactEffect(system, update, elemDef->effectOnImpact.handle, msecOnImpact, trace->normal);
        return msecOnImpact;
    }
    else
    {
        gravityScale = elemDef->gravity.amplitude * fx_randomTable[update->randomSeed + 15] + elemDef->gravity.base;
        overshotDeltaVelFromGravity = gravityScale * 0.800000011920929 * (double)(msecUpdateEnd - msecOnImpact);
        update->elemBaseVel[2] = update->elemBaseVel[2] + overshotDeltaVelFromGravity;
        msecElapsed = (float)(msecOnImpact - update->msecElemBegin);
        FX_GetVelocityAtTime(
            elemDef,
            update->randomSeed,
            update->msecLifeSpan,
            msecElapsed,
            &update->orient,
            update->elemBaseVel,
            preImpactVelocity);
        if ((LODWORD(preImpactVelocity[0]) & 0x7F800000) == 0x7F800000
            || (LODWORD(preImpactVelocity[1]) & 0x7F800000) == 0x7F800000
            || (LODWORD(preImpactVelocity[2]) & 0x7F800000) == 0x7F800000)
        {
            MyAssertHandler(
                ".\\EffectsCore\\fx_update.cpp",
                907,
                0,
                "%s",
                "!IS_NAN((preImpactVelocity)[0]) && !IS_NAN((preImpactVelocity)[1]) && !IS_NAN((preImpactVelocity)[2])");
        }
        if (Vec3LengthSq(preImpactVelocity) >= 9.99999995904e11)
        {
            v7 = va("%g %g %g", preImpactVelocity[0], preImpactVelocity[1], preImpactVelocity[2]);
            MyAssertHandler(
                ".\\EffectsCore\\fx_update.cpp",
                908,
                0,
                "%s\n\t%s",
                "Vec3LengthSq( preImpactVelocity ) < 1.0e12f",
                v7);
        }
        reflectionFactor = elemDef->reflectionFactor.amplitude * fx_randomTable[update->randomSeed + 16]
            + elemDef->reflectionFactor.base;
        Vec3Scale(preImpactVelocity, reflectionFactor, scaledPreImpactVelocity);
        if (elemDef->effectOnImpact.handle && Vec3LengthSq(preImpactVelocity) > 1.0)
        {
            FX_SpawnImpactEffect(system, update, elemDef->effectOnImpact.handle, msecOnImpact, trace->normal);
            elemDef = FX_GetUpdateElemDef(update);
        }
        if (msecOnImpact == msecUpdateBegin
            && (++msecOnImpact, Vec3LengthSq(scaledPreImpactVelocity) <= 1.0)
            && trace->normal[2] > 0.699999988079071)
        {
            fraction_4 = (float)msecOnImpact;
            update->atRestFraction = (int)FX_GetAtRestFraction(update, fraction_4);
            return msecUpdateEnd;
        }
        else
        {
            velocityAlongNormal = Vec3Dot(scaledPreImpactVelocity, trace->normal);
            if ((LODWORD(velocityAlongNormal) & 0x7F800000) == 0x7F800000)
                MyAssertHandler(".\\EffectsCore\\fx_update.cpp", 933, 0, "%s", "!IS_NAN(velocityAlongNormal)");
            v10 = fabs(velocityAlongNormal);
            if (v10 >= 1000000.0)
                MyAssertHandler(
                    ".\\EffectsCore\\fx_update.cpp",
                    934,
                    0,
                    "%s\n\t(velocityAlongNormal) = %g",
                    "(I_fabs( velocityAlongNormal ) < 1.0e6f)",
                    velocityAlongNormal);
            scale = velocityAlongNormal * -2.0;
            Vec3Mad(scaledPreImpactVelocity, scale, trace->normal, postImpactVelocity);
            Vec3Sub(postImpactVelocity, preImpactVelocity, velDelta);
            Vec3Add(update->elemBaseVel, velDelta, update->elemBaseVel);
            if ((COERCE_UNSIGNED_INT(*update->elemBaseVel) & 0x7F800000) == 0x7F800000
                || (COERCE_UNSIGNED_INT(update->elemBaseVel[1]) & 0x7F800000) == 0x7F800000
                || (COERCE_UNSIGNED_INT(update->elemBaseVel[2]) & 0x7F800000) == 0x7F800000)
            {
                MyAssertHandler(
                    ".\\EffectsCore\\fx_update.cpp",
                    939,
                    0,
                    "%s",
                    "!IS_NAN((update->elemBaseVel)[0]) && !IS_NAN((update->elemBaseVel)[1]) && !IS_NAN((update->elemBaseVel)[2])");
            }
            FX_OrientationPosFromWorldPos(&update->orient, update->posWorld, update->elemOrigin);
            *xyzWorldOld = update->posWorld[0];
            xyzWorldOld[1] = update->posWorld[1];
            xyzWorldOld[2] = update->posWorld[2];
            return msecOnImpact;
        }
    }
}

void __cdecl FX_SpawnImpactEffect(
    FxSystem *system,
    const FxUpdateElem *update,
    const FxEffectDef *impactEffect,
    int msecOnImpact,
    const float *impactNormal)
{
    FxEffect *effect; // [esp+8h] [ebp-28h]
    float axis[3][3]; // [esp+Ch] [ebp-24h] BYREF

    axis[0][0] = *impactNormal;
    axis[0][1] = impactNormal[1];
    axis[0][2] = impactNormal[2];
    Vec3Basis_RightHanded(axis[0], axis[1], axis[2]);
    effect = FX_SpawnEffect(
        system,
        impactEffect,
        msecOnImpact,
        update->posWorld,
        axis,
        4095,
        2047,
        255,
        update->effect->owner,
        0x3FFu);
    if (effect)
        FX_DelRefToEffect(system, effect);
}

int __cdecl FX_UpdateElementPosition_NonColliding(FxUpdateElem *update)
{
    FX_NextElementPosition(update, update->msecUpdateBegin, update->msecUpdateEnd);
    FX_OrientationPosFromWorldPos(&update->orient, update->posWorld, update->elemOrigin);
    return 1;
}

int __cdecl FX_UpdateElementPosition_Local(FxUpdateElem* update)
{
    const char* v1; // eax
    float* elemBaseVel; // [esp+1Ch] [ebp-4h]

    elemBaseVel = update->elemBaseVel;
    if (0.0 != elemBaseVel[0] || 0.0 != elemBaseVel[1] || 0.0 != elemBaseVel[2])
    {
        v1 = va(
            "effect %s def %i baseVel %g %g %g",
            update->effect->def->name,
            update->elemIndex,
            *update->elemBaseVel,
            update->elemBaseVel[1],
            update->elemBaseVel[2]);
        MyAssertHandler(
            ".\\EffectsCore\\fx_update.cpp",
            1028,
            0,
            "%s\n\t%s",
            "Vec3Compare( update->elemBaseVel, vec3_origin )",
            v1);
    }
    FX_NextElementPosition_NoExternalForces(
        update,
        update->msecUpdateBegin,
        update->msecUpdateEnd,
        update->elemOrigin,
        update->posWorld);
    return 1;
}

void __cdecl FX_SpawnDeathEffect(FxSystem* system, FxUpdateElem* update)
{
    FxEffect* effect; // [esp+10h] [ebp-54h]
    FxSpatialFrame frame; // [esp+14h] [ebp-50h] BYREF
    const FxElemDef* elemDef; // [esp+30h] [ebp-34h]
    orientation_t orientPrev; // [esp+34h] [ebp-30h] BYREF

    elemDef = FX_GetUpdateElemDef(update);
    FX_GetOrientation(elemDef, &update->effect->frameAtSpawn, &update->effect->framePrev, update->randomSeed, &orientPrev);
    FX_OrientationPosToWorldPos(&orientPrev, update->elemOrigin, frame.origin);
    effect = FX_SpawnEffect(
        system,
        elemDef->effectOnDeath.handle,
        update->msecUpdateBegin,
        frame.origin,
        orientPrev.axis,
        4095,
        2047,
        255,
        update->effect->owner,
        0x3FFu);
    if (effect)
        FX_DelRefToEffect(system, effect);
}

char __cdecl FX_UpdateElement_SetupUpdate(
    FxEffect *effect,
    int msecUpdateBegin,
    int msecUpdateEnd,
    unsigned int elemDefIndex,
    int elemAtRestFraction,
    int elemMsecBegin,
    int elemSequence,
    float *elemOrigin,
    FxUpdateElem *update)
{
    const FxEffectDef *def; // [esp+4h] [ebp-Ch]
    int msecLifeSpan; // [esp+8h] [ebp-8h]
    const FxElemDef *elemDef; // [esp+Ch] [ebp-4h]

    memset((unsigned __int8 *)update, 0xD0u, sizeof(FxUpdateElem));
    update->effect = effect;
    update->msecUpdateBegin = msecUpdateBegin;
    update->msecUpdateEnd = msecUpdateEnd;
    update->msecElemBegin = elemMsecBegin;
    if (update->msecUpdateBegin > update->msecUpdateEnd)
        MyAssertHandler(
            ".\\EffectsCore\\fx_update.cpp",
            1113,
            0,
            "update->msecUpdateBegin <= update->msecUpdateEnd\n\t%i, %i",
            update->msecUpdateBegin,
            update->msecUpdateEnd);
    if (update->msecUpdateEnd < update->msecElemBegin)
        return 0;
    def = effect->def;
    if (elemDefIndex >= def->elemDefCountEmission + def->elemDefCountOneShot + def->elemDefCountLooping)
        MyAssertHandler(
            ".\\EffectsCore\\fx_update.cpp",
            1119,
            0,
            "elemDefIndex doesn't index def->elemDefCountLooping + def->elemDefCountOneShot + def->elemDefCountEmission\n"
            "\t%i not in [0, %i)",
            elemDefIndex,
            def->elemDefCountEmission + def->elemDefCountOneShot + def->elemDefCountLooping);
    update->elemIndex = elemDefIndex;
    update->atRestFraction = elemAtRestFraction;
    update->randomSeed = (296 * elemSequence + elemMsecBegin + (unsigned int)effect->randomSeed) % 0x1DF;
    update->sequence = elemSequence;
    elemDef = FX_GetUpdateElemDef(update);
    msecLifeSpan = elemDef->lifeSpanMsec.base
        + (((elemDef->lifeSpanMsec.amplitude + 1) * LOWORD(fx_randomTable[update->randomSeed + 17])) >> 16);
    update->msecElemEnd = msecLifeSpan + update->msecElemBegin;
    update->msecLifeSpan = (float)msecLifeSpan;
    update->elemOrigin = elemOrigin;
    return 1;
}

void __cdecl FX_UpdateElement_TruncateToElemEnd(FxUpdateElem *update, FxUpdateResult *outUpdateResult)
{
    if (update->msecUpdateEnd >= update->msecElemEnd)
    {
        if (FX_GetUpdateElemDef(update)->effectEmitted.handle)
        {
            update->msecUpdateEnd = update->msecElemEnd;
            if (update->msecUpdateBegin > update->msecUpdateEnd)
                MyAssertHandler(
                    ".\\EffectsCore\\fx_update.cpp",
                    1146,
                    0,
                    "update->msecUpdateBegin <= update->msecUpdateEnd\n\t%g, %g",
                    (double)update->msecUpdateBegin,
                    (double)update->msecUpdateEnd);
        }
        else
        {
            *outUpdateResult = FX_UPDATE_REMOVE;
        }
    }
}

void __cdecl FX_UpdateElement_HandleEmitting(
    FxSystem* system,
    FxElem* elem,
    FxUpdateElem* update,
    const float* elemOriginPrev,
    FxUpdateResult* outUpdateResult)
{
    FxSpatialFrame frameBegin; // [esp+A4h] [ebp-70h] BYREF
    const FxElemDef* elemDef; // [esp+C0h] [ebp-54h]
    FxEffect* effect; // [esp+C4h] [ebp-50h]
    orientation_t orientPrev; // [esp+C8h] [ebp-4Ch] BYREF
    FxSpatialFrame frameEnd; // [esp+F8h] [ebp-1Ch] BYREF

    elemDef = FX_GetUpdateElemDef(update);
    if (elemDef->effectEmitted.handle)
    {
        effect = update->effect;
        FX_GetOrientation(elemDef, &effect->frameAtSpawn, &effect->framePrev, update->randomSeed, &orientPrev);
        FX_OrientationPosToWorldPos(&orientPrev, elemOriginPrev, frameBegin.origin);
        FX_GetQuatForOrientation(effect, elemDef, &effect->framePrev, &orientPrev, frameBegin.quat);
        frameEnd.origin[0] = update->posWorld[0];
        frameEnd.origin[1] = update->posWorld[1];
        frameEnd.origin[2] = update->posWorld[2];
        FX_GetQuatForOrientation(effect, elemDef, &effect->frameNow, &update->orient, frameEnd.quat);
        elem->emitResidual = FX_ProcessEmitting(system, update, elem->emitResidual, &frameBegin, &frameEnd);
        if (update->msecUpdateEnd == update->msecElemEnd)
            *outUpdateResult = FX_UPDATE_REMOVE;
    }
}

unsigned __int8 __cdecl FX_ProcessEmitting(
    FxSystem* system,
    FxUpdateElem* update,
    unsigned __int8 emitResidual,
    FxSpatialFrame* frameBegin,
    FxSpatialFrame* frameEnd)
{
    const char* v6; // eax
    float v7; // [esp+10h] [ebp-C0h]
    float v8; // [esp+18h] [ebp-B8h]
    float v9; // [esp+1Ch] [ebp-B4h]
    float v10; // [esp+28h] [ebp-A8h]
    FxEffect* effect; // [esp+38h] [ebp-98h]
    float v12; // [esp+40h] [ebp-90h]
    float v13; // [esp+48h] [ebp-88h]
    const FxElemDef* elemDef; // [esp+6Ch] [ebp-64h]
    float maxDistPerEmit; // [esp+70h] [ebp-60h]
    float lerp; // [esp+74h] [ebp-5Ch]
    float baseDistPerEmit; // [esp+78h] [ebp-58h]
    float distInUpdate; // [esp+7Ch] [ebp-54h]
    float residuala; // [esp+80h] [ebp-50h]
    float residual; // [esp+80h] [ebp-50h]
    FxSpatialFrame frameElemNow; // [esp+84h] [ebp-4Ch] BYREF
    float axisSpawn[3][3]; // [esp+A0h] [ebp-30h] BYREF
    float distNextEmit; // [esp+C4h] [ebp-Ch]
    int msecAtSpawn; // [esp+C8h] [ebp-8h]
    float distLastEmit; // [esp+CCh] [ebp-4h]

    Vec3Sub(frameEnd->origin, frameBegin->origin, axisSpawn[0]);
    distInUpdate = Vec3Normalize(axisSpawn[0]);
    if (distInUpdate == 0.0)
        return emitResidual;
    elemDef = FX_GetUpdateElemDef(update);
    v10 = elemDef->emitDist.amplitude * fx_randomTable[update->randomSeed + 20] + elemDef->emitDist.base;
    baseDistPerEmit = elemDef->emitDistVariance.base + v10;
    maxDistPerEmit = baseDistPerEmit + elemDef->emitDistVariance.amplitude;
    residuala = (double)emitResidual * maxDistPerEmit * 0.00390625;
    distNextEmit = -residuala;
    while (1)
    {
        distLastEmit = distNextEmit;
        distNextEmit = (double)rand() * 0.000030517578125 * elemDef->emitDistVariance.amplitude
            + baseDistPerEmit
            + distNextEmit;
        if (distInUpdate < (double)distNextEmit)
            break;
        v9 = distNextEmit - 0.0;
        if (v9 < 0.0)
            v8 = 0.0;
        else
            v8 = distNextEmit;
        distNextEmit = v8;
        lerp = v8 / distInUpdate;
        v12 = (double)(update->msecUpdateEnd - update->msecUpdateBegin) * lerp;
        v7 = floor(v12);
        msecAtSpawn = update->msecUpdateBegin + (int)v7;
        Vec3Lerp(frameBegin->origin, frameEnd->origin, lerp, frameElemNow.origin);
        Vec4Lerp(frameBegin->quat, frameEnd->quat, lerp, frameElemNow.quat);
        Vec4Normalize(frameElemNow.quat);
        PerpendicularVector(axisSpawn[0], axisSpawn[1]);
        Vec3Cross(axisSpawn[0], axisSpawn[1], axisSpawn[2]);
        effect = FX_SpawnEffect(
            system,
            elemDef->effectEmitted.handle,
            msecAtSpawn,
            frameElemNow.origin,
            axisSpawn,
            4095,
            2047,
            255,
            update->effect->owner,
            0x3FFu);
        if (effect)
            FX_DelRefToEffect(system, effect);
        elemDef = FX_GetUpdateElemDef(update);
    }
    residual = distInUpdate - distLastEmit;
    if (residual < -0.001000000047497451 || residual > maxDistPerEmit + 0.001000000047497451)
    {
        v6 = va("%g, %g", residual, maxDistPerEmit);
        MyAssertHandler(
            ".\\EffectsCore\\fx_update.cpp",
            613,
            0,
            "%s\n\t%s",
            "residual >= -0.001f && residual <= maxDistPerEmit + 0.001f",
            v6);
    }
    v13 = residual * 256.0 / maxDistPerEmit;
    return (int)(v13 + 9.313225746154785e-10);
}

void __cdecl FX_GetQuatForOrientation(
    const FxEffect *effect,
    const FxElemDef *elemDef,
    const FxSpatialFrame *frameNow,
    orientation_t *orient,
    float *quat)
{
    int runFlags; // [esp+4h] [ebp-4h]

    runFlags = elemDef->flags & 0xC0;
    if (runFlags)
    {
        if (runFlags == 64)
        {
            *quat = effect->frameAtSpawn.quat[0];
            quat[1] = effect->frameAtSpawn.quat[1];
            quat[2] = effect->frameAtSpawn.quat[2];
            quat[3] = effect->frameAtSpawn.quat[3];
        }
        else if (runFlags == 128)
        {
            *quat = frameNow->quat[0];
            quat[1] = frameNow->quat[1];
            quat[2] = frameNow->quat[2];
            quat[3] = frameNow->quat[3];
        }
        else
        {
            if (runFlags != 192)
                MyAssertHandler(
                    ".\\EffectsCore\\fx_update.cpp",
                    1081,
                    0,
                    "%s\n\t(runFlags) = %i",
                    "(runFlags == FX_ELEM_RUN_RELATIVE_TO_OFFSET)",
                    runFlags);
            AxisToQuat(orient->axis, quat);
        }
    }
    else
    {
        *quat = 0.0;
        quat[1] = 0.0;
        quat[2] = 0.0;
        quat[3] = 1.0;
    }
}

char __cdecl FX_UpdateElement_TruncateToElemBegin(FxUpdateElem* update, FxUpdateResult* outUpdateResult)
{
    const char* v3; // eax
    const FxElemDef* UpdateElemDef; // eax
    FxSpatialFrame* p_frameAtSpawn; // [esp-8h] [ebp-18h]
    FxSpatialFrame* p_frameNow; // [esp-4h] [ebp-14h]
    int randomSeed; // [esp+0h] [ebp-10h]
    const FxElemDef* elemDef; // [esp+Ch] [ebp-4h]

    if (update->msecUpdateBegin < update->msecElemBegin)
        update->msecUpdateBegin = update->msecElemBegin;
    if (update->msecUpdateBegin == update->msecUpdateEnd)
    {
        *outUpdateResult = (update->msecUpdateBegin < update->msecElemEnd) ? FX_UPDATE_KEEP : FX_UPDATE_REMOVE;
        return 0;
    }
    else
    {
        elemDef = FX_GetUpdateElemDef(update);
        if (update->msecLifeSpan <= 0.0)
        {
            v3 = va("%i + [0,%i) = %g", elemDef->lifeSpanMsec.base, elemDef->lifeSpanMsec.amplitude, update->msecLifeSpan);
            MyAssertHandler(".\\EffectsCore\\fx_update.cpp", 1197, 0, "%s\n\t%s", "update->msecLifeSpan > 0.0f", v3);
        }
        update->msecElapsed = (float)(update->msecUpdateEnd - update->msecElemBegin);
        update->normTimeUpdateEnd = update->msecElapsed / update->msecLifeSpan;
        if (update->normTimeUpdateEnd < 0.0 || update->normTimeUpdateEnd > 1.0)
            MyAssertHandler(
                ".\\EffectsCore\\fx_update.cpp",
                1202,
                0,
                "%s\n\t(update->normTimeUpdateEnd) = %g",
                "(update->normTimeUpdateEnd >= 0.0f && update->normTimeUpdateEnd <= 1.0f)",
                update->normTimeUpdateEnd);
        randomSeed = update->randomSeed;
        p_frameNow = &update->effect->frameNow;
        p_frameAtSpawn = &update->effect->frameAtSpawn;
        UpdateElemDef = FX_GetUpdateElemDef(update);
        FX_GetOrientation(UpdateElemDef, p_frameAtSpawn, p_frameNow, randomSeed, &update->orient);
        return 1;
    }
}

void __cdecl FX_UpdateEffectPartialTrail(
    FxSystem *system,
    FxEffect *effect,
    FxTrail *trail,
    int msecUpdateBegin,
    int msecUpdateEnd,
    float distanceTravelledBegin,
    float distanceTravelledEnd,
    unsigned __int16 trailElemHandleStart,
    unsigned __int16 trailElemHandleStop,
    FxSpatialFrame *frameNow)
{
    float basis[2][3]; // [esp+1Ch] [ebp-34h] BYREF
    FxTrailElem *trailElem; // [esp+38h] [ebp-18h]
    FxTrailElem *remoteTrailElem; // [esp+3Ch] [ebp-14h]
    unsigned __int16 trailElemHandleNext; // [esp+40h] [ebp-10h]
    unsigned __int16 trailElemHandleLast; // [esp+44h] [ebp-Ch]
    unsigned __int16 trailElemHandle; // [esp+48h] [ebp-8h]
    bool removable; // [esp+4Fh] [ebp-1h]

    trailElemHandleLast = -1;
    if (trailElemHandleStart == 0xFFFF)
        trailElemHandle = trail->firstElemHandle;
    else
        trailElemHandle = trailElemHandleStart;
    trailElem = 0;
    removable = trailElemHandle == trail->firstElemHandle;
    while (trailElemHandle != trailElemHandleStop)
    {
        if (trailElemHandle == 0xFFFF)
            MyAssertHandler(".\\EffectsCore\\fx_update.cpp", 1519, 0, "%s", "trailElemHandle != FX_HANDLE_NONE");
        if (!system)
            MyAssertHandler("c:\\trees\\cod3\\src\\effectscore\\fx_system.h", 348, 0, "%s", "system");
        remoteTrailElem = (FxTrailElem *)FX_PoolFromHandle_Generic<FxTrailElem, 2048>(system->trailElems, trailElemHandle);
        trailElem = remoteTrailElem;
        trailElemHandleNext = remoteTrailElem->nextTrailElemHandle;
        if (FX_UpdateTrailElement(system, effect, trail, remoteTrailElem, msecUpdateBegin, msecUpdateEnd))
            removable = 0;
        if (removable && trailElemHandleLast != 0xFFFF)
            FX_FreeTrailElem(system, trailElemHandleLast, effect, trail);
        trailElemHandleLast = trailElemHandle;
        trailElemHandle = trailElemHandleNext;
    }
    if (trailElemHandleLast != 0xFFFF && trailElemHandleLast == trail->lastElemHandle)
    {
        if (removable)
        {
            FX_FreeTrailElem(system, trailElemHandleLast, effect, trail);
        }
        else if ((effect->status & 0x10000) != 0)
        {
            if (!trailElem)
                MyAssertHandler(".\\EffectsCore\\fx_update.cpp", 1551, 0, "%s", "trailElem");
            trailElem->spawnDist = effect->distanceTraveled;
            FX_GetOriginForTrailElem(
                effect,
                &effect->def->elemDefs[trail->defIndex],
                frameNow,
                (296 * trailElem->sequence + trailElem->msecBegin + (unsigned int)effect->randomSeed) % 0x1DF,
                trailElem->origin,
                basis[0],
                basis[1]);
            FX_TrailElem_CompressBasis(basis, trailElem->basis);
        }
    }
}

void __cdecl FX_TrailElem_CompressBasis(const float (*inBasis)[3], char (*outBasis)[3])
{
    int v2; // [esp+0h] [ebp-10h]
    int v3; // [esp+4h] [ebp-Ch]
    int basisVecIter; // [esp+8h] [ebp-8h]
    int dimIter; // [esp+Ch] [ebp-4h]

    for (basisVecIter = 0; basisVecIter != 2; ++basisVecIter)
    {
        for (dimIter = 0; dimIter != 3; ++dimIter)
        {
            v3 = (int)((float)(*inBasis)[3 * basisVecIter + dimIter] * 127.0);
            if (v3 >= -128)
            {
                if (v3 <= 127)
                    v2 = (int)((float)(*inBasis)[3 * basisVecIter + dimIter] * 127.0);
                else
                    v2 = 127;
            }
            else
            {
                v2 = 0x80;
            }
            (*outBasis)[3 * basisVecIter + dimIter] = v2;
        }
    }
}

FxUpdateResult __cdecl FX_UpdateTrailElement(
    FxSystem *system,
    FxEffect *effect,
    FxTrail *trail,
    FxTrailElem *trailElem,
    int msecUpdateBegin,
    int msecUpdateEnd)
{
    int v7; // [esp+4h] [ebp-B4h]
    int v8; // [esp+8h] [ebp-B0h]
    float v9; // [esp+10h] [ebp-A8h]
    FxUpdateElem update; // [esp+28h] [ebp-90h] BYREF
    FxUpdateResult updateResult; // [esp+A8h] [ebp-10h] BYREF
    float baseVel[3]; // [esp+ACh] [ebp-Ch] BYREF

    updateResult = FX_UPDATE_KEEP;
    if (FX_UpdateElement_SetupUpdate(
        effect,
        msecUpdateBegin,
        msecUpdateEnd,
        trail->defIndex,
        0,
        trailElem->msecBegin,
        0,
        trailElem->origin,
        &update))
    {
        FX_UpdateElement_TruncateToElemEnd(&update, &updateResult);
        if (updateResult)
        {
            if (!FX_UpdateElement_TruncateToElemBegin(&update, &updateResult))
                return updateResult;
            v9 = (double)trailElem->baseVelZ * 0.001000000047497451;
            baseVel[0] = 0.0;
            baseVel[1] = 0.0;
            baseVel[2] = v9;
            update.elemBaseVel = baseVel;
            update.physObjId = 0;
            update.onGround = 0;
            //Profile_Begin(194);
            updateResult = (FxUpdateResult)FX_UpdateElementPosition(system, &update);
            //Profile_EndInternal(0);
            v8 = (int)(baseVel[2] / 0.001000000047497451);
            if (v8 >= -32768)
            {
                if (v8 <= 0x7FFF)
                    v7 = (int)(baseVel[2] / 0.001000000047497451);
                else
                    v7 = 0x7FFF;
            }
            else
            {
                v7 = 0x8000;
            }
            trailElem->baseVelZ = v7;
        }
    }
    return updateResult;
}

void __cdecl FX_UpdateSpotLight(FxCmd* cmd)
{
    FxEffect* effect; // [esp+8h] [ebp-8h]
    FxSystem* system; // [esp+Ch] [ebp-4h]

    if (fx_enable->current.enabled)
    {
        system = cmd->system;
        if (!cmd->system)
            MyAssertHandler(".\\EffectsCore\\fx_update.cpp", 1831, 0, "%s", "system");
        FX_BeginIteratingOverEffects_Cooperative(system);
        if (system->activeSpotLightEffectCount > 0)
        {
            if (system->activeSpotLightEffectCount != 1)
                MyAssertHandler(".\\EffectsCore\\fx_update.cpp", 1836, 0, "%s", "system->activeSpotLightEffectCount == 1");
            for (effect = FX_EffectFromHandle(system, system->activeSpotLightEffectHandle);
                InterlockedExchangeAdd(&effect->status, 0x20000000) >= 0x20000000;
                InterlockedExchangeAdd(&effect->status, -536870912))
            {
                ;
            }
            FX_UpdateSpotLightEffect(system, effect);
            InterlockedExchangeAdd(&effect->status, -536870912);
        }
        if (!InterlockedDecrement(&system->iteratorCount) && system->needsGarbageCollection)
            FX_RunGarbageCollection(system);
        if (fx_draw->current.enabled)
            FX_DrawSpotLight(system);
    }
}

void __cdecl FX_UpdateSpotLightEffect(FxSystem* system, FxEffect* effect)
{
    float v2; // [esp+20h] [ebp-28h]
    float diff[3]; // [esp+28h] [ebp-20h] BYREF
    unsigned __int16 lastElemHandle[4]; // [esp+34h] [ebp-14h] BYREF
    float newDistanceTraveled; // [esp+40h] [ebp-8h]
    unsigned int elemClass; // [esp+44h] [ebp-4h]

    if ((unsigned __int16)effect->status && effect->msecLastUpdate <= system->msecNow)
    {
        FX_UpdateEffectBolt(system, effect);
        system->activeSpotLightBoltDobj = effect->boltAndSortOrder.dobjHandle;
        Vec3Sub(effect->frameNow.origin, effect->framePrev.origin, diff);
        v2 = Vec3Length(diff);
        newDistanceTraveled = effect->distanceTraveled + v2;
        for (elemClass = 0; elemClass < 3; ++elemClass)
            lastElemHandle[elemClass] = -1;
        FX_UpdateSpotLightEffectPartial(system, effect, effect->msecLastUpdate, system->msecNow);
        FX_UpdateEffectPartial(
            system,
            effect,
            effect->msecLastUpdate,
            system->msecNow,
            effect->distanceTraveled,
            newDistanceTraveled,
            effect->firstElemHandle,
            lastElemHandle,
            0,
            0);
        FX_SortNewElemsInEffect(system, effect);
        memcpy(&effect->framePrev, &effect->frameNow, sizeof(effect->framePrev));
        effect->distanceTraveled = newDistanceTraveled;
    }
}

void __cdecl FX_UpdateSpotLightEffectPartial(
    FxSystem* system,
    FxEffect* effect,
    int msecUpdateBegin,
    int msecUpdateEnd)
{
    unsigned __int16 activeSpotLightElemHandle; // [esp+12h] [ebp-Ah]
    FxPool<FxElem>* elem; // [esp+18h] [ebp-4h]

    if (system->activeSpotLightEffectCount != 1)
        MyAssertHandler(".\\EffectsCore\\fx_update.cpp", 1411, 0, "%s", "system->activeSpotLightEffectCount == 1");
    if (effect != FX_EffectFromHandle(system, system->activeSpotLightEffectHandle))
        MyAssertHandler(
            ".\\EffectsCore\\fx_update.cpp",
            1412,
            0,
            "%s",
            "effect == FX_EffectFromHandle( system, system->activeSpotLightEffectHandle )");
    if (effect->msecLastUpdate > msecUpdateEnd)
        MyAssertHandler(
            ".\\EffectsCore\\fx_update.cpp",
            1413,
            0,
            "effect->msecLastUpdate <= msecUpdateEnd\n\t%g, %g",
            (double)effect->msecLastUpdate,
            (double)msecUpdateEnd);
    if (system->activeSpotLightElemCount)
    {
        activeSpotLightElemHandle = system->activeSpotLightElemHandle;
        if (!system)
            MyAssertHandler("c:\\trees\\cod3\\src\\effectscore\\fx_system.h", 334, 0, "%s", "system");
        elem = FX_PoolFromHandle_Generic<FxElem, 2048>(system->elems, activeSpotLightElemHandle);
        if (FX_UpdateElement(system, effect, (FxElem*)elem, msecUpdateBegin, msecUpdateEnd) == FX_UPDATE_REMOVE)
            FX_FreeSpotLightElem(system, system->activeSpotLightElemHandle, effect);
    }
}

void __cdecl FX_UpdateEffectBolt(FxSystem *system, FxEffect *effect)
{
    int localClientNum; // [esp+4h] [ebp-38h]
    orientation_t orient; // [esp+8h] [ebp-34h] BYREF
    bool temporalBitsValid; // [esp+3Bh] [ebp-1h]

    if (effect->boltAndSortOrder.boneIndex != 0x7FF)
    {
        localClientNum = system->localClientNum;
        temporalBitsValid = FX_GetBoltTemporalBits(localClientNum, effect->boltAndSortOrder.dobjHandle) == effect->boltAndSortOrder.temporalBits;
        if (temporalBitsValid
            && FX_GetBoneOrientation(
                localClientNum,
                effect->boltAndSortOrder.dobjHandle,
                effect->boltAndSortOrder.boneIndex,
                &orient))
        {
            effect->frameNow.origin[0] = orient.origin[0];
            effect->frameNow.origin[1] = orient.origin[1];
            effect->frameNow.origin[2] = orient.origin[2];
            AxisToQuat(orient.axis, effect->frameNow.quat);
        }
        else
        {
            FX_StopEffect(system, effect);
            effect->boltAndSortOrder.boneIndex = 0x7FF;
            effect->boltAndSortOrder.dobjHandle = 0xFFF;
        }
    }
}

void __cdecl FX_UpdateNonDependent(FxCmd *cmd)
{
    if (fx_enable->current.enabled)
        FX_Update(cmd->system, cmd->localClientNum, 1);
}

void __cdecl FX_Update(FxSystem* system, int localClientNum, bool nonBoltedEffectsOnly)
{
    FxEffect* localEffect; // [esp+48h] [ebp-Ch]
    volatile int activeIndex; // [esp+4Ch] [ebp-8h]

    //Profile_Begin(193);
    if (!system)
        MyAssertHandler(".\\EffectsCore\\fx_update.cpp", 1762, 0, "%s", "system");
    FX_BeginIteratingOverEffects_Cooperative(system);
    for (activeIndex = system->firstActiveEffect; activeIndex != system->firstNewEffect; ++activeIndex)
    {
        localEffect = FX_EffectFromHandle(system, system->allEffectHandles[activeIndex & 0x3FF]);
        if (FX_ShouldProcessEffect(system, localEffect, nonBoltedEffectsOnly))
        {
            while (InterlockedExchangeAdd(&localEffect->status, 0x20000000) >= 0x20000000)
                InterlockedExchangeAdd(&localEffect->status, -536870912);
            FX_UpdateEffect(system, localEffect);
            InterlockedExchangeAdd(&localEffect->status, -536870912);
        }
    }
    if (!InterlockedDecrement(&system->iteratorCount) && system->needsGarbageCollection)
        FX_RunGarbageCollection(system);
    //Profile_EndInternal(0);
}

void __cdecl FX_UpdateEffect(FxSystem* system, FxEffect* effect)
{
    float v2; // [esp+20h] [ebp-28h]
    float diff[3]; // [esp+28h] [ebp-20h] BYREF
    unsigned __int16 lastElemHandle[4]; // [esp+34h] [ebp-14h] BYREF
    float newDistanceTraveled; // [esp+40h] [ebp-8h]
    unsigned int elemClass; // [esp+44h] [ebp-4h]

    if ((unsigned __int16)effect->status && effect->msecLastUpdate <= system->msecNow)
    {
        FX_UpdateEffectBolt(system, effect);
        Vec3Sub(effect->frameNow.origin, effect->framePrev.origin, diff);
        v2 = Vec3Length(diff);
        newDistanceTraveled = effect->distanceTraveled + v2;
        for (elemClass = 0; elemClass < 3; ++elemClass)
            lastElemHandle[elemClass] = -1;
        FX_UpdateEffectPartial(
            system,
            effect,
            effect->msecLastUpdate,
            system->msecNow,
            effect->distanceTraveled,
            newDistanceTraveled,
            effect->firstElemHandle,
            lastElemHandle,
            0,
            0);
        FX_SortNewElemsInEffect(system, effect);
        memcpy(&effect->framePrev, &effect->frameNow, sizeof(effect->framePrev));
        effect->distanceTraveled = newDistanceTraveled;
    }
}

bool __cdecl FX_ShouldProcessEffect(FxSystem *system, FxEffect *effect, bool nonBoltedEffectsOnly)
{
    return (!nonBoltedEffectsOnly || effect->boltAndSortOrder.boneIndex == 0x7FF)
        && InterlockedExchange(&effect->frameCount, system->frameCount) != system->frameCount;
}

void __cdecl FX_RunPhysics(int localClientNum)
{
    FxSystem *system; // [esp+0h] [ebp-4h]

    system = FX_GetSystem(localClientNum);
    Sys_EnterCriticalSection(CRITSECT_PHYSICS);
    Phys_RunToTime(localClientNum, PHYS_WORLD_FX, system->msecNow);
    Sys_LeaveCriticalSection(CRITSECT_PHYSICS);
}

void __cdecl FX_UpdateRemaining(FxCmd *cmd)
{
    if (fx_enable->current.enabled)
        FX_Update(cmd->system, cmd->localClientNum, 0);
    CG_AddSceneTracerBeams(cmd->localClientNum);
    CG_GenerateSceneVerts(cmd->localClientNum);
}

void __cdecl FX_BeginUpdate(int localClientNum)
{
    FX_BeginMarks(localClientNum);
    FX_Beam_Begin();
    FX_PostLight_Begin();
    FX_SpriteBegin();
}

void __cdecl FX_EndUpdate(int localClientNum)
{
    FxSystem *system; // [esp+38h] [ebp-4h]

    if (fx_enable->current.enabled)
    {
        //Profile_Begin(195);
        system = FX_GetSystem(localClientNum);
        if (!system)
            MyAssertHandler(".\\EffectsCore\\fx_update.cpp", 1936, 0, "%s", "system");
        memcpy(&system->cameraPrev, system, sizeof(system->cameraPrev));
        if (!system->cameraPrev.isValid)
            MyAssertHandler(".\\EffectsCore\\fx_update.cpp", 1939, 0, "%s", "system->cameraPrev.isValid");
        //Profile_EndInternal(0);
    }
}

void __cdecl FX_AddNonSpriteDrawSurfs(FxCmd *cmd)
{
    FxSystem *system; // [esp+30h] [ebp-4h]

    //Profile_Begin(200);
    system = cmd->system;
    if (!cmd->system)
        MyAssertHandler(".\\EffectsCore\\fx_update.cpp", 1952, 0, "%s", "system");
    if (fx_enable->current.enabled && fx_draw->current.enabled)
    {
        FX_SortEffects(system);
        FX_DrawNonSpriteElems(system);
    }
    //Profile_EndInternal(0);
}

void __cdecl FX_RewindTo(int localClientNum, int time)
{
    volatile long *Destination; // [esp+4h] [ebp-10ACh]
    volatile long Comperand; // [esp+8h] [ebp-10A8h]
    unsigned __int16 v4; // [esp+18h] [ebp-1098h]
    FxEffect *effect; // [esp+1Ch] [ebp-1094h]
    FxEffect *effecta; // [esp+1Ch] [ebp-1094h]
    FxEffect *effectb; // [esp+1Ch] [ebp-1094h]
    unsigned int dst[32]; // [esp+20h] [ebp-1090h] BYREF
    int bitNum; // [esp+A0h] [ebp-1010h]
    FxSystem *system; // [esp+A4h] [ebp-100Ch]
    FxEffect* v11[1024]; // [esp+A8h] [ebp-1008h]
    int v12; // [esp+10A8h] [ebp-8h]
    volatile int i; // [esp+10ACh] [ebp-4h]

    system = FX_GetSystem(localClientNum);
    if (!system)
        MyAssertHandler(".\\EffectsCore\\fx_update.cpp", 2052, 0, "%s", "system");
    if (time < system->msecNow)
    {
        system->msecNow = time;
        v12 = 0;
        memset((unsigned __int8 *)dst, 0, sizeof(dst));
        FX_BeginIteratingOverEffects_Cooperative(system);
        for (i = system->firstActiveEffect; i != system->firstNewEffect; ++i)
        {
            v4 = system->allEffectHandles[i & 0x3FF];
            effect = FX_EffectFromHandle(system, v4);
            v11[v12++] = effect;
            if ((unsigned __int16)effect->status && effect->msecBegin < time && effect->owner == v4)
            {
                FX_AddRefToEffect(system, effect);
                Com_BitSetAssert(dst, v12 - 1, 128);
            }
        }
        for (bitNum = 0; bitNum < v12; ++bitNum)
        {
            effecta = (FxEffect *)v11[bitNum];
            if ((unsigned __int16)effecta->status)
            {
                while (InterlockedExchangeAdd(&effecta->status, 0x20000000) >= 0x20000000)
                    InterlockedExchangeAdd(&effecta->status, -536870912);
                FX_KillEffect(system, effecta);
                InterlockedExchangeAdd(&effecta->status, -536870912);
            }
        }
        if (!InterlockedDecrement(&system->iteratorCount) && system->needsGarbageCollection)
            FX_RunGarbageCollection(system);
        for (bitNum = 0; bitNum < v12; ++bitNum)
        {
            if (Com_BitCheckAssert(dst, bitNum, 128))
            {
                effectb = (FxEffect *)v11[bitNum];
                effectb->msecLastUpdate = effectb->msecBegin;
                if (effectb->def->msecLoopingLife)
                {
                    Destination = &effectb->status;
                    do
                        Comperand = *Destination;
                    while (InterlockedCompareExchange(Destination, Comperand | 0x10000, Comperand) != Comperand);
                    FX_StartNewEffect(system, effectb);
                }
                else
                {
                    FX_StartNewEffect(system, effectb);
                    FX_DelRefToEffect(system, effectb);
                }
            }
        }
        if (system->needsGarbageCollection)
            FX_RunGarbageCollection(system);
    }
}

void __cdecl FX_SetNextUpdateCamera(int localClientNum, const refdef_s *refdef, float zfar)
{
    const char *v3; // eax
    float scale1; // [esp+14h] [ebp-50h]
    float scale1a; // [esp+14h] [ebp-50h]
    double v6; // [esp+18h] [ebp-4Ch]
    float v7; // [esp+20h] [ebp-44h]
    float v8; // [esp+24h] [ebp-40h]
    float v9; // [esp+38h] [ebp-2Ch]
    float v10; // [esp+3Ch] [ebp-28h]
    float sinHalfFov; // [esp+54h] [ebp-10h]
    float sinHalfFova; // [esp+54h] [ebp-10h]
    FxSystem *system; // [esp+58h] [ebp-Ch]
    float cosHalfFov; // [esp+5Ch] [ebp-8h]
    float cosHalfFova; // [esp+5Ch] [ebp-8h]
    unsigned int planeIndex; // [esp+60h] [ebp-4h]

    if (!refdef)
        MyAssertHandler(".\\EffectsCore\\fx_update.cpp", 2150, 0, "%s", "refdef");
    system = FX_GetSystem(localClientNum);
    system->camera.origin[0] = refdef->vieworg[0];
    system->camera.origin[1] = refdef->vieworg[1];
    system->camera.origin[2] = refdef->vieworg[2];
    AxisCopy(refdef->viewaxis, system->camera.axis);
    system->camera.frustum[0][0] = refdef->viewaxis[0][0];
    system->camera.frustum[0][1] = refdef->viewaxis[0][1];
    system->camera.frustum[0][2] = refdef->viewaxis[0][2];
    system->camera.viewOffset[0] = refdef->viewOffset[0];
    system->camera.viewOffset[1] = refdef->viewOffset[1];
    system->camera.viewOffset[2] = refdef->viewOffset[2];
    v10 = refdef->tanHalfFovX * refdef->tanHalfFovX + 1.0;
    v8 = sqrt(v10);
    cosHalfFov = 1.0 / v8;
    sinHalfFov = refdef->tanHalfFovX * cosHalfFov;
    Vec3ScaleMad(sinHalfFov, refdef->viewaxis[0], cosHalfFov, refdef->viewaxis[1], system->camera.frustum[1]);
    scale1 = -cosHalfFov;
    Vec3ScaleMad(sinHalfFov, refdef->viewaxis[0], scale1, refdef->viewaxis[1], system->camera.frustum[2]);
    v9 = refdef->tanHalfFovY * refdef->tanHalfFovY + 1.0;
    v7 = sqrt(v9);
    cosHalfFova = 1.0 / v7;
    sinHalfFova = refdef->tanHalfFovY * cosHalfFova;
    Vec3ScaleMad(sinHalfFova, refdef->viewaxis[0], cosHalfFova, refdef->viewaxis[2], system->camera.frustum[3]);
    scale1a = -cosHalfFova;
    Vec3ScaleMad(sinHalfFova, refdef->viewaxis[0], scale1a, refdef->viewaxis[2], system->camera.frustum[4]);
    system->camera.frustumPlaneCount = 5;
    for (planeIndex = 0; planeIndex < system->camera.frustumPlaneCount; ++planeIndex)
    {
        if (!Vec3IsNormalized(system->camera.frustum[planeIndex]))
        {
            v6 = Vec3Length(system->camera.frustum[planeIndex]);
            v3 = va(
                "(%g %g %g) len %g",
                system->camera.frustum[planeIndex][0],
                system->camera.frustum[planeIndex][1],
                system->camera.frustum[planeIndex][2],
                v6);
            MyAssertHandler(
                ".\\EffectsCore\\fx_update.cpp",
                2172,
                0,
                "%s\n\t%s",
                "Vec3IsNormalized( system->camera.frustum[planeIndex] )",
                v3);
        }
        system->camera.frustum[planeIndex][3] = Vec3Dot(system->camera.origin, system->camera.frustum[planeIndex]);
    }
    if (zfar > 0.0)
    {
        system->camera.frustum[5][0] = -refdef->viewaxis[0][0];
        system->camera.frustum[5][1] = -refdef->viewaxis[0][1];
        system->camera.frustum[5][2] = -refdef->viewaxis[0][2];
        system->camera.frustum[5][3] = -system->camera.frustum[0][3] - zfar;
        system->camera.frustumPlaneCount = 6;
    }
    InterlockedExchange(&system->camera.isValid, 1);
}

void __cdecl FX_SetNextUpdateTime(int localClientNum, int time)
{
    FxSystem *system; // [esp+0h] [ebp-4h]

    system = FX_GetSystem(localClientNum);
    if (time < system->msecNow)
        MyAssertHandler(
            ".\\EffectsCore\\fx_update.cpp",
            2193,
            0,
            "time >= system->msecNow\n\t%i, %i",
            time,
            system->msecNow);
    InterlockedExchange(&system->camera.isValid, 0);
    InterlockedExchange(&system->msecDraw, time);
    system->msecNow = time;
    if (++system->frameCount <= 0)
        system->frameCount = 1;
}

void __cdecl FX_FillUpdateCmd(int localClientNum, FxCmd *cmd)
{
    cmd->system = FX_GetSystem(localClientNum);
    cmd->localClientNum = localClientNum;
}

