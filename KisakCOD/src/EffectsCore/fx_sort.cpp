#include "fx_system.h"


void __cdecl FX_SortEffects(FxSystem *system)
{
    float diff[9]; // [esp+20h] [ebp-1048h] BYREF
    FxEffect *firstEffect; // [esp+44h] [ebp-1024h]
    float v3[1024]; // [esp+48h] [ebp-1020h]
    int v4; // [esp+1048h] [ebp-20h]
    unsigned __int16 v5; // [esp+104Ch] [ebp-1Ch]
    float *a; // [esp+1050h] [ebp-18h]
    volatile int j; // [esp+1054h] [ebp-14h]
    int v8; // [esp+1058h] [ebp-10h]
    FxEffect *secondEffect; // [esp+105Ch] [ebp-Ch]
    float v10; // [esp+1060h] [ebp-8h]
    volatile int i; // [esp+1064h] [ebp-4h]

    //Profile_Begin(199);
    if (!system)
        MyAssertHandler(".\\EffectsCore\\fx_sort.cpp", 98, 0, "%s", "system");
    a = (float *)system;
    FX_WaitBeginIteratingOverEffects_Exclusive(system);
    for (i = system->firstActiveEffect; i != system->firstNewEffect; ++i)
    {
        v5 = system->allEffectHandles[i & 0x3FF];
        firstEffect = FX_EffectFromHandle(system, v5);
        Vec3Sub(a, firstEffect->frameNow.origin, diff);
        v10 = Vec3LengthSq(diff);
        for (j = i; j != system->firstActiveEffect; --j)
        {
            v4 = ((_WORD)j - 1) & 0x3FF;
            if (v3[v4] >= v10 * 0.9998999834060669)
            {
                if (v3[v4] >= v10 * 1.000100016593933)
                    break;
                secondEffect = FX_EffectFromHandle(system, system->allEffectHandles[v4]);
                if (secondEffect->owner != firstEffect->owner || !FX_FirstEffectIsFurther(firstEffect, secondEffect))
                    break;
            }
            v8 = j & 0x3FF;
            v3[v8] = v3[v4];
            system->allEffectHandles[v8] = system->allEffectHandles[v4];
        }
        v8 = j & 0x3FF;
        v3[v8] = v10;
        system->allEffectHandles[v8] = v5;
    }
    system->iteratorCount = 0;
    //Profile_EndInternal(0);
}

void __cdecl FX_WaitBeginIteratingOverEffects_Exclusive(FxSystem *system)
{
    volatile long *Destination; // [esp+0h] [ebp-4h]

    if (system->isArchiving)
        MyAssertHandler("c:\\trees\\cod3\\src\\effectscore\\fx_system.h", 512, 0, "%s", "!system->isArchiving");
    Destination = &system->iteratorCount;
    do
    {
        while (*Destination)
            ;
    } while (InterlockedCompareExchange(Destination, -1, 0));
}

bool __cdecl FX_FirstEffectIsFurther(FxEffect *firstEffect, FxEffect *secondEffect)
{
    if (firstEffect->boltAndSortOrder.sortOrder == 255
        && secondEffect->boltAndSortOrder.sortOrder == 255)
    {
        return 0;
    }
    if (firstEffect->boltAndSortOrder.sortOrder == 255)
        firstEffect->boltAndSortOrder.sortOrder = FX_CalcRunnerParentSortOrder(firstEffect);
    if (secondEffect->boltAndSortOrder.sortOrder == 255)
        secondEffect->boltAndSortOrder.sortOrder = FX_CalcRunnerParentSortOrder(secondEffect);
    return firstEffect->boltAndSortOrder.sortOrder < secondEffect->boltAndSortOrder.sortOrder;
}

int __cdecl FX_CalcRunnerParentSortOrder(FxEffect *effect)
{
    int v3; // [esp+8h] [ebp-20h]
    const FxEffectDef *def; // [esp+10h] [ebp-18h]
    int totalNonRunnerElemDefs; // [esp+14h] [ebp-14h]
    const FxElemDef *elemDef; // [esp+18h] [ebp-10h]
    int totalSortOrder; // [esp+20h] [ebp-8h]
    int elemDefIndex; // [esp+24h] [ebp-4h]

    if (!effect)
        MyAssertHandler(".\\EffectsCore\\fx_sort.cpp", 39, 0, "%s", "effect");
    def = effect->def;
    if (!effect->def)
        MyAssertHandler(".\\EffectsCore\\fx_sort.cpp", 42, 0, "%s", "def");
    totalSortOrder = 0;
    totalNonRunnerElemDefs = 0;
    for (elemDefIndex = 0;
        elemDefIndex < def->elemDefCountLooping + def->elemDefCountOneShot + def->elemDefCountEmission;
        ++elemDefIndex)
    {
        elemDef = &effect->def->elemDefs[elemDefIndex];
        if (elemDef->elemType != 10)
        {
            totalSortOrder += elemDef->sortOrder;
            ++totalNonRunnerElemDefs;
        }
    }
    if (totalNonRunnerElemDefs <= 0)
        return 0;
    if (totalSortOrder / totalNonRunnerElemDefs < 254)
        v3 = totalSortOrder / totalNonRunnerElemDefs;
    else
        v3 = 254;
    if (v3 > 0)
        return v3;
    else
        return 0;
}

void __cdecl FX_SortNewElemsInEffect(FxSystem *system, FxEffect *effect)
{
    unsigned __int16 elemHandle; // [esp+8h] [ebp-Ch]
    unsigned __int16 elemHandlea; // [esp+8h] [ebp-Ch]
    unsigned __int16 stopElemHandle; // [esp+Ch] [ebp-8h]
    FxPool<FxElem> *elema; // [esp+10h] [ebp-4h]
    FxPool<FxElem> *elem; // [esp+10h] [ebp-4h]

    elemHandle = effect->firstElemHandle[0];
    stopElemHandle = effect->firstSortedElemHandle;
    if (elemHandle != stopElemHandle)
    {
        effect->firstElemHandle[0] = stopElemHandle;
        if (stopElemHandle != 0xFFFF)
        {
            if (!system)
                MyAssertHandler("c:\\trees\\cod3\\src\\effectscore\\fx_system.h", 334, 0, "%s", "system");
            FX_PoolFromHandle_Generic<FxElem, 2048>(system->elems, stopElemHandle)->item.prevElemHandleInEffect = -1;
        }
        do
        {
            if (!system)
                MyAssertHandler("c:\\trees\\cod3\\src\\effectscore\\fx_system.h", 334, 0, "%s", "system");
            elema = FX_PoolFromHandle_Generic<FxElem, 2048>(system->elems, elemHandle);
            elemHandle = elema->item.nextElemHandleInEffect;
            FX_SortSpriteElemIntoEffect(system, effect, (FxElem *)elema);
        } while (elemHandle != stopElemHandle);
        effect->firstSortedElemHandle = effect->firstElemHandle[0];
        for (elemHandlea = effect->firstElemHandle[0]; elemHandlea != 0xFFFF; elemHandlea = elem->item.nextElemHandleInEffect)
        {
            if (!system)
                MyAssertHandler("c:\\trees\\cod3\\src\\effectscore\\fx_system.h", 334, 0, "%s", "system");
            elem = FX_PoolFromHandle_Generic<FxElem, 2048>(system->elems, elemHandlea);
            if (effect->def->elemDefs[elem->item.defIndex].elemType > 3u)
                MyAssertHandler(
                    ".\\EffectsCore\\fx_sort.cpp",
                    272,
                    0,
                    "FX_GetEffectElemDef( effect, elem->defIndex )->elemType <= FX_ELEM_TYPE_LAST_SPRITE\n\t%i, %i",
                    effect->def->elemDefs[elem->item.defIndex].elemType,
                    3);
        }
    }
}

void __cdecl FX_SortSpriteElemIntoEffect(FxSystem *system, FxEffect *effect, FxElem *elem)
{
    unsigned __int16 v3; // [esp+5Ah] [ebp-2Ah]
    FxInsertSortElem sortElem; // [esp+5Ch] [ebp-28h] BYREF
    unsigned __int16 elemHandle; // [esp+70h] [ebp-14h]
    FxElem *nextElem; // [esp+74h] [ebp-10h]
    unsigned __int16 *prevNextElemHandle; // [esp+78h] [ebp-Ch]
    FxElem *prevElem; // [esp+7Ch] [ebp-8h]
    unsigned __int16 prevElemHandle; // [esp+80h] [ebp-4h]

    nextElem = 0;
    prevElemHandle = -1;
    prevNextElemHandle = effect->firstElemHandle;
    if (effect->firstElemHandle[0] != 0xFFFF)
    {
        FX_GetInsertSortElem(system, effect, elem, &sortElem);
        if (sortElem.defSortOrder < 0)
            MyAssertHandler(".\\EffectsCore\\fx_sort.cpp", 215, 0, "%s", "sortElem.defSortOrder >= 0");
        do
        {
            v3 = *prevNextElemHandle;
            if (!system)
                MyAssertHandler("c:\\trees\\cod3\\src\\effectscore\\fx_system.h", 334, 0, "%s", "system");
            nextElem = (FxElem *)FX_PoolFromHandle_Generic<FxElem, 2048>(system->elems, v3);
            if (!FX_ExistingElemSortsBeforeNewElem(system, effect, nextElem, &sortElem))
                break;
            prevElem = nextElem;
            prevElemHandle = *prevNextElemHandle;
            prevNextElemHandle = &nextElem->nextElemHandleInEffect;
        } while (nextElem->nextElemHandleInEffect != 0xFFFF);
    }
    elem->nextElemHandleInEffect = *prevNextElemHandle;
    elem->prevElemHandleInEffect = prevElemHandle;
    if (!system)
        MyAssertHandler("c:\\trees\\cod3\\src\\effectscore\\fx_system.h", 327, 0, "%s", "system");
    elemHandle = FX_PoolToHandle_Generic<FxElem, 2048>(system->elems, elem);
    *prevNextElemHandle = elemHandle;
    if (elem->nextElemHandleInEffect != 0xFFFF)
        nextElem->prevElemHandleInEffect = elemHandle;
}

void __cdecl FX_GetInsertSortElem(
    const FxSystem *system,
    const FxEffect *effect,
    const FxElem *elem,
    FxInsertSortElem *sortElem)
{
    float diff[3]; // [esp+0h] [ebp-54h] BYREF
    const FxEffectDef *def; // [esp+Ch] [ebp-48h]
    float posWorld[3]; // [esp+10h] [ebp-44h] BYREF
    const FxElemDef *elemDef; // [esp+1Ch] [ebp-38h]
    int randomSeed; // [esp+20h] [ebp-34h]
    orientation_t orient; // [esp+24h] [ebp-30h] BYREF

    sortElem->msecBegin = elem->msecBegin;
    sortElem->defIndex = elem->defIndex;
    def = effect->def;
    elemDef = &def->elemDefs[elem->defIndex];
    sortElem->elemType = elemDef->elemType;
    if (elemDef->elemType > 3u)
        MyAssertHandler(
            ".\\EffectsCore\\fx_sort.cpp",
            159,
            0,
            "%s\n\t(elemDef->elemType) = %i",
            "(elemDef->elemType <= FX_ELEM_TYPE_LAST_SPRITE)",
            elemDef->elemType);
    sortElem->defSortOrder = elemDef->sortOrder;
    randomSeed = (296 * elem->sequence + elem->msecBegin + (unsigned int)effect->randomSeed) % 0x1DF;
    FX_GetOrientation(elemDef, &effect->frameAtSpawn, &effect->frameNow, randomSeed, &orient);
    FX_OrientationPosToWorldPos(&orient, elem->origin, posWorld);
    Vec3Sub(system->cameraPrev.origin, posWorld, diff);
    sortElem->distToCamSq = Vec3LengthSq(diff);
}

bool __cdecl FX_ExistingElemSortsBeforeNewElem(
    const FxSystem *system,
    const FxEffect *effect,
    const FxElem *elem,
    const FxInsertSortElem *sortElemNew)
{
    float diff[3]; // [esp+4h] [ebp-58h] BYREF
    const FxEffectDef *def; // [esp+10h] [ebp-4Ch]
    float posWorld[3]; // [esp+14h] [ebp-48h] BYREF
    float distToCamSq; // [esp+20h] [ebp-3Ch]
    const FxElemDef *elemDef; // [esp+24h] [ebp-38h]
    int randomSeed; // [esp+28h] [ebp-34h]
    orientation_t orient; // [esp+2Ch] [ebp-30h] BYREF

    def = effect->def;
    elemDef = &def->elemDefs[elem->defIndex];
    if (elemDef->elemType == 3)
        MyAssertHandler(".\\EffectsCore\\fx_sort.cpp", 180, 0, "%s", "elemDef->elemType != FX_ELEM_TYPE_TRAIL");
    if (elemDef->elemType > 3u)
        return 1;
    if (!elemDef->visualCount)
        return 0;
    if (elemDef->sortOrder < sortElemNew->defSortOrder)
        return 1;
    if (elemDef->sortOrder > sortElemNew->defSortOrder)
        return 0;
    randomSeed = (elem->msecBegin + effect->randomSeed + 296 * (unsigned int)elem->sequence) % 0x1DF;
    FX_GetOrientation(elemDef, &effect->frameAtSpawn, &effect->frameNow, randomSeed, &orient);
    FX_OrientationPosToWorldPos(&orient, elem->origin, posWorld);
    Vec3Sub(system->cameraPrev.origin, posWorld, diff);
    distToCamSq = Vec3LengthSq(diff);
    return sortElemNew->distToCamSq < (double)distToCamSq;
}

