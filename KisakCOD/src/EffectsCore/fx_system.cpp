#include "fx_system.h"

#include <qcommon/mem_track.h>
#include <qcommon/threads.h>

#include <physics/phys_local.h>

#include <gfx_d3d/rb_light.h>

#include <universal/com_sndalias.h>

#include <client_mp/client_mp.h>

#include <win32/win_local.h>

int fx_maxLocalClients;
int fx_serverVisClient;

FxSystem fx_systemPool[1];
FxSystemBuffers fx_systemBufferPool[1];
FxMarksSystem fx_marksSystemPool[1];

void __cdecl TRACK_fx_system()
{
    track_static_alloc_internal(fx_systemPool, 2656, "fx_systemPool", 8);
    track_static_alloc_internal(fx_systemBufferPool, 291968, "fx_systemBufferPool", 8);
    track_static_alloc_internal(fx_marksSystemPool, 294940, "fx_marksSystemPool", 8);
}

FxSystem *__cdecl FX_GetSystem(int clientIndex)
{
    if (clientIndex)
        MyAssertHandler(
            ".\\EffectsCore\\fx_system.cpp",
            140,
            0,
            "%s\n\t(clientIndex) = %i",
            "(clientIndex == 0)",
            clientIndex);
    return fx_systemPool;
}

FxSystemBuffers *__cdecl FX_GetSystemBuffers(int clientIndex)
{
    if (clientIndex)
        MyAssertHandler(
            ".\\EffectsCore\\fx_system.cpp",
            152,
            0,
            "%s\n\t(clientIndex) = %i",
            "(clientIndex == 0)",
            clientIndex);
    return fx_systemBufferPool;
}

void __cdecl FX_LinkSystemBuffers(FxSystem *system, FxSystemBuffers *systemBuffers)
{
    system->elems = systemBuffers->elems;
    system->effects = systemBuffers->effects;
    system->trails = systemBuffers->trails;
    system->trailElems = systemBuffers->trailElems;
    system->visState = systemBuffers->visState;
    system->deferredElems = systemBuffers->deferredElems;
}

void __cdecl FX_InitSystem(int localClientNum)
{
    FxSystem *system; // [esp+4h] [ebp-8h]
    FxSystemBuffers *systemBuffers; // [esp+8h] [ebp-4h]

    system = FX_GetSystem(localClientNum);
    if (!system)
        MyAssertHandler(".\\EffectsCore\\fx_system.cpp", 463, 0, "%s", "system");
    memset((unsigned __int8 *)system, 0, sizeof(FxSystem));
    systemBuffers = FX_GetSystemBuffers(localClientNum);
    if (!systemBuffers)
        MyAssertHandler(".\\EffectsCore\\fx_system.cpp", 466, 0, "%s", "systemBuffers");
    memset((unsigned __int8 *)systemBuffers, 0, sizeof(FxSystemBuffers));
    FX_LinkSystemBuffers(system, systemBuffers);
    FX_RegisterDvars();
    CL_ResetStats_f();
    FX_ResetSystem(system);
    system->msecNow = 0;
    system->msecDraw = -1;
    system->cameraPrev.isValid = 1;
    system->cameraPrev.frustumPlaneCount = 0;
    system->frameCount = 1;
    if (system->firstActiveEffect)
        MyAssertHandler(".\\EffectsCore\\fx_system.cpp", 479, 1, "%s", "system->firstActiveEffect == 0");
    if (system->firstNewEffect)
        MyAssertHandler(".\\EffectsCore\\fx_system.cpp", 480, 1, "%s", "system->firstNewEffect == 0");
    if (system->firstFreeEffect)
        MyAssertHandler(".\\EffectsCore\\fx_system.cpp", 481, 1, "%s", "system->firstFreeEffect == 0");
    if (localClientNum)
        MyAssertHandler(
            "c:\\trees\\cod3\\src\\effectscore\\fx_marks.h",
            139,
            0,
            "%s\n\t(clientIndex) = %i",
            "(clientIndex == 0)",
            localClientNum);
    FX_InitMarksSystem(fx_marksSystemPool);
    system->localClientNum = localClientNum;
    system->isInitialized = 1;
    fx_serverVisClient = localClientNum;
}

void __cdecl FX_ResetSystem(FxSystem *system)
{
    FxPool<FxTrail> *trails; // [esp+0h] [ebp-28h]
    int k; // [esp+8h] [ebp-20h]
    FxPool<FxTrailElem> *trailElems; // [esp+Ch] [ebp-1Ch]
    int j; // [esp+14h] [ebp-14h]
    FxPool<FxElem> *elems; // [esp+18h] [ebp-10h]
    int i; // [esp+20h] [ebp-8h]
    int effectIndex; // [esp+24h] [ebp-4h]

    system->effects->def = 0;
    for (effectIndex = 0; effectIndex < 1024; ++effectIndex)
        system->allEffectHandles[effectIndex] = FX_EffectToHandle(system, &system->effects[effectIndex]);
    system->firstActiveEffect = 0;
    system->firstNewEffect = 0;
    system->firstFreeEffect = 0;
    system->iteratorCount = 0;
    system->deferredElemCount = 0;
    elems = system->elems;
    system->firstFreeElem = 0;
    for (i = 0; i < 2047; ++i)
        elems[i].nextFree = i + 1;
    elems[i].nextFree = -1;
    system->activeElemCount = 0;
    trailElems = system->trailElems;
    system->firstFreeTrailElem = 0;
    for (j = 0; j < 2047; ++j)
        trailElems[j].nextFree = j + 1;
    trailElems[j].nextFree = -1;
    system->activeTrailElemCount = 0;
    trails = system->trails;
    system->firstFreeTrail = 0;
    for (k = 0; k < 127; ++k)
        trails[k].nextFree = k + 1;
    trails[k].nextFree = -1;
    system->activeTrailCount = 0;
    system->activeSpotLightEffectCount = 0;
    system->activeSpotLightElemCount = 0;
    system->gfxCloudCount = 0;
    system->visState->blockerCount = 0;
    system->visStateBufferRead = system->visState;
    system->visStateBufferWrite = system->visState + 1;
}

int __cdecl FX_EffectToHandle(FxSystem *system, FxEffect *effect)
{
    const char *v2; // eax

    if (!system)
        MyAssertHandler("c:\\trees\\cod3\\src\\effectscore\\fx_system.h", 241, 0, "%s", "system");
    if (!effect || effect < system->effects || effect >= &system->effects[1024])
    {
        v2 = va("%p %p", system->effects, effect);
        MyAssertHandler(
            "c:\\trees\\cod3\\src\\effectscore\\fx_system.h",
            248,
            0,
            "%s\n\t%s",
            "effect && effect >= &system->effects[0] && effect < &system->effects[FX_EFFECT_LIMIT]",
            v2);
    }
    return ((char *)effect - (char *)system->effects) / 4;
}


void __cdecl FX_ShutdownSystem(int localClientNum)
{
    FxSystem *system; // [esp+0h] [ebp-8h]
    FxSystemBuffers *systemBuffers; // [esp+4h] [ebp-4h]

    system = FX_GetSystem(localClientNum);
    systemBuffers = FX_GetSystemBuffers(localClientNum);
    if (!system)
        MyAssertHandler(".\\EffectsCore\\fx_system.cpp", 503, 0, "%s", "system");
    memset((unsigned __int8 *)system, 0, sizeof(FxSystem));
    if (!systemBuffers)
        MyAssertHandler(".\\EffectsCore\\fx_system.cpp", 505, 0, "%s", "systemBuffers");
    memset((unsigned __int8 *)systemBuffers, 0, sizeof(FxSystemBuffers));
    if (system->isInitialized)
        MyAssertHandler(".\\EffectsCore\\fx_system.cpp", 507, 1, "%s", "!system->isInitialized");
    FX_UnregisterAll();
}

void __cdecl FX_RelocateSystem(FxSystem *system, int relocationDistance)
{
    if (relocationDistance)
    {
        system->visStateBufferRead = (const FxVisState *)((char *)system->visStateBufferRead + relocationDistance);
        system->visStateBufferWrite = (FxVisState *)((char *)system->visStateBufferWrite + relocationDistance);
    }
}

void __cdecl FX_EffectNoLongerReferenced(FxSystem *system, FxEffect *remoteEffect)
{
    const char *v2; // eax
    int oldStatusValue; // [esp+14h] [ebp-8h]
    FxEffect *remoteOwner; // [esp+18h] [ebp-4h]

    if (!remoteEffect)
        MyAssertHandler(".\\EffectsCore\\fx_system.cpp", 677, 0, "%s", "remoteEffect");
    if ((unsigned __int16)remoteEffect->status != 1)
        MyAssertHandler(".\\EffectsCore\\fx_system.cpp", 690, 0, "%s", "(effect->status & FX_STATUS_REF_COUNT_MASK) == 1");
    if ((remoteEffect->status & 0x7FE0000) != 0)
    {
        v2 = va("%s, %i", remoteEffect->def->name, remoteEffect->status);
        MyAssertHandler(
            ".\\EffectsCore\\fx_system.cpp",
            691,
            0,
            "%s\n\t%s",
            "(effect->status & FX_STATUS_OWNED_EFFECTS_MASK) == 0",
            v2);
    }
    remoteOwner = FX_EffectFromHandle(system, remoteEffect->owner);
    if ((remoteEffect->status & 0x10000000) == 0)
    {
        if ((remoteOwner->status & 0x7FE0000) == 0)
            MyAssertHandler(
                ".\\EffectsCore\\fx_system.cpp",
                708,
                0,
                "%s\n\t(owner->status) = %i",
                "((owner->status & FX_STATUS_OWNED_EFFECTS_MASK) > 0)",
                remoteOwner->status);
        oldStatusValue = InterlockedExchangeAdd(&remoteOwner->status, -131072);
        if ((oldStatusValue & 0xF801FFFF) != ((oldStatusValue - 0x20000) & 0xF801FFFF))
            MyAssertHandler(
                ".\\EffectsCore\\fx_system.cpp",
                717,
                0,
                "%s\n\t(oldStatusValue) = %i",
                "((oldStatusValue & ~FX_STATUS_OWNED_EFFECTS_MASK) == ((oldStatusValue - (1 << FX_STATUS_OWNED_EFFECTS_SHIFT)) & "
                "~FX_STATUS_OWNED_EFFECTS_MASK))",
                oldStatusValue);
        FX_DelRefToEffect(system, remoteOwner);
    }
    system->needsGarbageCollection = 1;
}

void __cdecl FX_DelRefToEffect(FxSystem *system, FxEffect *effect)
{
    if (!effect)
        MyAssertHandler("c:\\trees\\cod3\\src\\effectscore\\fx_system.h", 393, 0, "%s", "effect");
    if ((unsigned __int16)effect->status == 1)
        FX_EffectNoLongerReferenced(system, effect);
    if (!(unsigned __int16)effect->status)
        MyAssertHandler(
            "c:\\trees\\cod3\\src\\effectscore\\fx_system.h",
            411,
            0,
            "%s\n\t(effect->status & FX_STATUS_REF_COUNT_MASK) = %i",
            "((effect->status & FX_STATUS_REF_COUNT_MASK) > 0)",
            (unsigned __int16)effect->status);
    InterlockedDecrement(&effect->status);
}

void __cdecl FX_RunGarbageCollection(FxSystem *system)
{
    unsigned __int16 effectHandle; // [esp+8h] [ebp-818h]
    unsigned int freedCount; // [esp+Ch] [ebp-814h]
    unsigned __int16 freedHandles[1026]; // [esp+10h] [ebp-810h]
    FxEffect *effect; // [esp+818h] [ebp-8h]
    int activeIndex; // [esp+81Ch] [ebp-4h]

    if (!system)
        MyAssertHandler(".\\EffectsCore\\fx_system.cpp", 779, 0, "%s", "system");
    if (system->needsGarbageCollection && FX_BeginIteratingOverEffects_Exclusive(system))
    {
        system->needsGarbageCollection = 0;
        activeIndex = system->firstNewEffect;
        freedCount = 0;
        while (activeIndex != system->firstActiveEffect)
        {
            effectHandle = system->allEffectHandles[--activeIndex & 0x3FF];
            effect = FX_EffectFromHandle(system, effectHandle);
            if ((unsigned __int16)effect->status)
            {
                system->allEffectHandles[((_WORD)freedCount + (_WORD)activeIndex) & 0x3FF] = effectHandle;
            }
            else
            {
                FX_RunGarbageCollection_FreeTrails(system, effect);
                FX_RunGarbageCollection_FreeSpotLight(system, effectHandle);
                freedHandles[freedCount++] = effectHandle;
            }
        }
        while (freedCount)
        {
            system->allEffectHandles[activeIndex++ & 0x3FF] = freedHandles[--freedCount];
            effect = FX_EffectFromHandle(system, freedHandles[freedCount]);
            memset((unsigned __int8 *)effect, 0, sizeof(FxEffect));
        }
        system->firstActiveEffect = activeIndex;
        system->iteratorCount = 0;
    }
}

bool __cdecl FX_BeginIteratingOverEffects_Exclusive(FxSystem *system)
{
    if (system->isArchiving)
        MyAssertHandler("c:\\trees\\cod3\\src\\effectscore\\fx_system.h", 523, 0, "%s", "!system->isArchiving");
    return InterlockedCompareExchange(&system->iteratorCount, -1, 0) == 0;
}

void __cdecl FX_RunGarbageCollection_FreeSpotLight(FxSystem *system, unsigned __int16 effectHandle)
{
    if (system->activeSpotLightEffectCount && system->activeSpotLightEffectHandle == effectHandle)
    {
        if (system->activeSpotLightEffectCount != 1)
            MyAssertHandler(".\\EffectsCore\\fx_system.cpp", 739, 0, "%s", "system->activeSpotLightEffectCount == 1");
        InterlockedDecrement(&system->activeSpotLightEffectCount);
    }
}

void __cdecl FX_RunGarbageCollection_FreeTrails(FxSystem *system, FxEffect *effect)
{
    unsigned __int16 firstTrailHandle; // [esp+Ah] [ebp-6h]
    FxPool<FxTrail> *trail; // [esp+Ch] [ebp-4h]

    while (effect->firstTrailHandle != 0xFFFF)
    {
        firstTrailHandle = effect->firstTrailHandle;
        if (!system)
            MyAssertHandler("c:\\trees\\cod3\\src\\effectscore\\fx_system.h", 362, 0, "%s", "system");
        trail = FX_PoolFromHandle_Generic<FxTrail, 128>(system->trails, firstTrailHandle);
        effect->firstTrailHandle = trail->item.nextTrailHandle;
        trail->nextFree = 0;
        *(unsigned int *)&trail->item.lastElemHandle = 0;
        FX_FreePool_Generic_FxTrail_((FxTrail *)trail, &system->firstFreeTrail, system->trails);
        InterlockedDecrement(&system->activeTrailCount);
    }
}

void __cdecl FX_SpawnEffect_AllocTrails(FxSystem *system, FxEffect *effect)
{
    const FxEffectDef *def; // [esp+4h] [ebp-1Ch]
    FxPool<FxTrail> *remoteTrail; // [esp+Ch] [ebp-14h]
    int elemDefCount; // [esp+10h] [ebp-10h]
    int elemDefIter; // [esp+14h] [ebp-Ch]
    int localTrail; // [esp+18h] [ebp-8h]

    def = effect->def;
    if (!effect->def)
        MyAssertHandler(".\\EffectsCore\\fx_system.cpp", 968, 0, "%s", "def");
    elemDefCount = def->elemDefCountOneShot + def->elemDefCountLooping + def->elemDefCountEmission;
    for (elemDefIter = 0; elemDefIter != elemDefCount; ++elemDefIter)
    {
        if (effect->def->elemDefs[elemDefIter].elemType == 3)
        {
            remoteTrail = FX_AllocTrail(system);
            if (!remoteTrail)
                return;

            if ((char)elemDefIter != elemDefIter)
                MyAssertHandler(
                    ".\\EffectsCore\\fx_system.cpp",
                    984,
                    0,
                    "%s\n\t(elemDefIter) = %i",
                    "(localTrail.defIndex == elemDefIter)",
                    elemDefIter);

            if (!system)
                MyAssertHandler("c:\\trees\\cod3\\src\\effectscore\\fx_system.h", 355, 0, "%s", "system");
            effect->firstTrailHandle = FX_PoolToHandle_Generic<FxTrail, 128>(system->trails, (FxTrail *)remoteTrail);
            
            // KISAKTODO verify this
            
            remoteTrail->nextFree = effect->firstTrailHandle | 0xFFFF0000;
            remoteTrail->item.lastElemHandle = -1;
            remoteTrail->item.defIndex = elemDefIter;
            remoteTrail->item.sequence = 0;
        }
    }
}

FxPool<FxTrail>* __cdecl FX_AllocPool_Generic_FxTrail_(
    volatile long* firstFreeIndex,
    FxPool<FxTrail>* pool,
    volatile long* activeCount)
{
    FxPool<FxTrail>* item; // [esp+0h] [ebp-8h]
    unsigned int itemIndex; // [esp+4h] [ebp-4h]

    Sys_EnterCriticalSection(CRITSECT_FX_ALLOC);
    itemIndex = *firstFreeIndex;
    if (*firstFreeIndex != -1 && itemIndex >= 0x80)
        MyAssertHandler(
            ".\\EffectsCore\\fx_system.cpp",
            188,
            0,
            "%s",
            "itemIndex == -1 || (itemIndex >= 0 && itemIndex < ITEM_TYPE::POOL_SIZE)");
    if (itemIndex == -1)
    {
        Sys_LeaveCriticalSection(CRITSECT_FX_ALLOC);
        return 0;
    }
    else
    {
        item = &pool[itemIndex];
        if (item->nextFree != -1 && item->nextFree >= 0x80u)
            MyAssertHandler(
                ".\\EffectsCore\\fx_system.cpp",
                200,
                0,
                "%s",
                "item->nextFree == -1 || (item->nextFree >= 0 && item->nextFree < ITEM_TYPE::POOL_SIZE)");
        *firstFreeIndex = item->nextFree;
        ++*activeCount;
        
        Sys_LeaveCriticalSection(CRITSECT_FX_ALLOC);
        return &pool[itemIndex];
    }
}

FxPool<FxTrailElem>* __cdecl FX_AllocPool_Generic_FxTrailElem_(
    volatile int* firstFreeIndex,
    FxPool<FxTrailElem>* pool,
    volatile int* activeCount)
{
    FxPool<FxTrailElem>* item; // [esp+0h] [ebp-8h]
    unsigned int itemIndex; // [esp+4h] [ebp-4h]

    Sys_EnterCriticalSection(CRITSECT_FX_ALLOC);
    itemIndex = *firstFreeIndex;
    if (*firstFreeIndex != -1 && itemIndex >= 0x800)
        MyAssertHandler(
            ".\\EffectsCore\\fx_system.cpp",
            188,
            0,
            "%s",
            "itemIndex == -1 || (itemIndex >= 0 && itemIndex < ITEM_TYPE::POOL_SIZE)");
    if (itemIndex == -1)
    {
        Sys_LeaveCriticalSection(CRITSECT_FX_ALLOC);
        return 0;
    }
    else
    {
        item = &pool[itemIndex];
        if (item->nextFree != -1 && item->nextFree >= 0x800u)
            MyAssertHandler(
                ".\\EffectsCore\\fx_system.cpp",
                200,
                0,
                "%s",
                "item->nextFree == -1 || (item->nextFree >= 0 && item->nextFree < ITEM_TYPE::POOL_SIZE)");
        *firstFreeIndex = item->nextFree;
        ++*activeCount;
        
        Sys_LeaveCriticalSection(CRITSECT_FX_ALLOC);
        return &pool[itemIndex];
    }
}

FxPool<FxElem>* __cdecl FX_AllocPool_Generic_FxElem_(
    volatile int* firstFreeIndex,
    FxPool<FxElem>* pool,
    volatile int* activeCount)
{
    FxPool<FxElem>* item; // [esp+0h] [ebp-8h]
    unsigned int itemIndex; // [esp+4h] [ebp-4h]

    Sys_EnterCriticalSection(CRITSECT_FX_ALLOC);
    itemIndex = *firstFreeIndex;
    if (*firstFreeIndex != -1 && itemIndex >= 0x800)
        MyAssertHandler(
            ".\\EffectsCore\\fx_system.cpp",
            188,
            0,
            "%s",
            "itemIndex == -1 || (itemIndex >= 0 && itemIndex < ITEM_TYPE::POOL_SIZE)");
    if (itemIndex == -1)
    {
        Sys_LeaveCriticalSection(CRITSECT_FX_ALLOC);
        return 0;
    }
    else
    {
        item = &pool[itemIndex];
        if (item->nextFree != -1 && item->nextFree >= 0x800u)
            MyAssertHandler(
                ".\\EffectsCore\\fx_system.cpp",
                200,
                0,
                "%s",
                "item->nextFree == -1 || (item->nextFree >= 0 && item->nextFree < ITEM_TYPE::POOL_SIZE)");
        *firstFreeIndex = item->nextFree;
        ++*activeCount;
        Sys_LeaveCriticalSection(CRITSECT_FX_ALLOC);
        return &pool[itemIndex];
    }
}

void __cdecl FX_FreePool_Generic_FxElem_(FxElem* item, volatile long* firstFreeIndex, FxPool<FxElem>* pool)
{
    volatile unsigned int freedIndex; // [esp+4h] [ebp-4h]

    freedIndex = ((char*)item - (char*)pool) / 40;
    if (freedIndex >= 0x800)
        MyAssertHandler(
            ".\\EffectsCore\\fx_system.cpp",
            228,
            0,
            "%s",
            "freedIndex >= 0 && freedIndex < ITEM_TYPE::POOL_SIZE");
    Sys_EnterCriticalSection(CRITSECT_FX_ALLOC);
    if (*firstFreeIndex != -1 && *firstFreeIndex >= 0x800u)
        MyAssertHandler(
            ".\\EffectsCore\\fx_system.cpp",
            243,
            0,
            "%s",
            "*firstFreeIndex == -1 || (*firstFreeIndex >= 0 && *firstFreeIndex < ITEM_TYPE::POOL_SIZE)");
    *(_DWORD*)&item->defIndex = *firstFreeIndex;
    *firstFreeIndex = freedIndex;
    Sys_LeaveCriticalSection(CRITSECT_FX_ALLOC);
}

void __cdecl FX_FreePool_Generic_FxTrailElem_(
    FxTrailElem* item,
    volatile long* firstFreeIndex,
    FxPool<FxTrailElem>* pool)
{
    volatile unsigned int freedIndex; // [esp+4h] [ebp-4h]

    freedIndex = ((char*)item - (char*)pool) >> 5;
    if (freedIndex >= 0x800)
        MyAssertHandler(
            ".\\EffectsCore\\fx_system.cpp",
            228,
            0,
            "%s",
            "freedIndex >= 0 && freedIndex < ITEM_TYPE::POOL_SIZE");
    Sys_EnterCriticalSection(CRITSECT_FX_ALLOC);
    if (*firstFreeIndex != -1 && *firstFreeIndex >= 0x800u)
        MyAssertHandler(
            ".\\EffectsCore\\fx_system.cpp",
            243,
            0,
            "%s",
            "*firstFreeIndex == -1 || (*firstFreeIndex >= 0 && *firstFreeIndex < ITEM_TYPE::POOL_SIZE)");
    LODWORD(item->origin[0]) = *firstFreeIndex;
    *firstFreeIndex = freedIndex;
    Sys_LeaveCriticalSection(CRITSECT_FX_ALLOC);
}

void __cdecl FX_FreePool_Generic_FxTrail_(FxTrail* item, volatile long* firstFreeIndex, FxPool<FxTrail>* pool)
{
    volatile unsigned int freedIndex; // [esp+4h] [ebp-4h]

    freedIndex = ((char*)item - (char*)pool) >> 3;
    if (freedIndex >= 0x80)
        MyAssertHandler(
            ".\\EffectsCore\\fx_system.cpp",
            228,
            0,
            "%s",
            "freedIndex >= 0 && freedIndex < ITEM_TYPE::POOL_SIZE");
    Sys_EnterCriticalSection(CRITSECT_FX_ALLOC);
    if (*firstFreeIndex != -1 && *firstFreeIndex >= 0x80u)
        MyAssertHandler(
            ".\\EffectsCore\\fx_system.cpp",
            243,
            0,
            "%s",
            "*firstFreeIndex == -1 || (*firstFreeIndex >= 0 && *firstFreeIndex < ITEM_TYPE::POOL_SIZE)");
    *(_DWORD*)&item->nextTrailHandle = *firstFreeIndex;
    *firstFreeIndex = freedIndex;
    Sys_LeaveCriticalSection(CRITSECT_FX_ALLOC);
}


FxPool<FxTrail> *__cdecl FX_AllocTrail(FxSystem *system)
{
    return FX_AllocPool_Generic_FxTrail_(&system->firstFreeTrail, system->trails, &system->activeTrailCount);
}

unsigned __int16 __cdecl FX_CalculatePackedLighting(const float *origin)
{
    unsigned __int8 color[4]; // [esp+4h] [ebp-4h] BYREF

    R_GetAverageLightingAtPoint(origin, color);
    return ((color[2] & 0xF8) << 8) | (8 * (color[1] & 0xF8)) | ((color[0] & 0xF8) >> 3);
}
FxEffect* __cdecl FX_SpawnEffect(
    FxSystem* system,
    const FxEffectDef* remoteDef,
    int msecBegin,
    const float* origin,
    const float (*axis)[3],
    int dobjHandle,
    int boneIndex,
    int runnerSortOrder,
    unsigned __int16 owner,
    unsigned int markEntnum)
{
    volatile long* Destination; // [esp+Ch] [ebp-34h]
    unsigned __int16 effectHandle; // [esp+1Ch] [ebp-24h]
    int allocIndex; // [esp+20h] [ebp-20h]
    FxEffect* ownerEffect; // [esp+28h] [ebp-18h]
    FxEffect* remoteEffect; // [esp+2Ch] [ebp-14h]
    LONG oldStatusValue; // [esp+30h] [ebp-10h]
    char isSpotLightEffect; // [esp+3Bh] [ebp-5h]
    unsigned int elemClass; // [esp+3Ch] [ebp-4h]

    if (!system)
        MyAssertHandler(".\\EffectsCore\\fx_system.cpp", 1120, 0, "%s", "system");
    if (system->isArchiving)
        MyAssertHandler(".\\EffectsCore\\fx_system.cpp", 1121, 0, "%s", "!system->isArchiving");
    if (!remoteDef)
        MyAssertHandler(".\\EffectsCore\\fx_system.cpp", 1122, 0, "%s", "remoteDef");
    if (!origin)
        MyAssertHandler(".\\EffectsCore\\fx_system.cpp", 1123, 0, "%s", "origin");
    if (!axis)
        MyAssertHandler(".\\EffectsCore\\fx_system.cpp", 1124, 0, "%s", "axis");
    if (fx_cull_effect_spawn->current.enabled && FX_CullEffectForSpawn(&system->cameraPrev, remoteDef, origin))
        return 0;
    isSpotLightEffect = FX_IsSpotLightEffect(system, remoteDef);
    if (!isSpotLightEffect || FX_CanAllocSpotLightEffect(system))
    {
        allocIndex = InterlockedExchangeAdd(&system->firstFreeEffect, 1);
        while (allocIndex - system->firstActiveEffect >= 1024)
        {
            if (InterlockedCompareExchange(&system->firstFreeEffect, allocIndex, allocIndex + 1) == allocIndex + 1)
                return 0;
        }
        effectHandle = system->allEffectHandles[allocIndex & 0x3FF];
        remoteEffect = FX_EffectFromHandle(system, effectHandle);
        remoteEffect->def = remoteDef;
        remoteEffect->status = remoteDef->msecLoopingLife != 0 ? 536936450 : 536870913;
        for (elemClass = 0; elemClass < 3; ++elemClass)
            remoteEffect->firstElemHandle[elemClass] = -1;
        remoteEffect->firstSortedElemHandle = -1;
        if ((remoteDef->flags & 1) != 0)
            remoteEffect->packedLighting = FX_CalculatePackedLighting(origin);
        else
            remoteEffect->packedLighting = 255;
        remoteEffect->msecBegin = msecBegin;
        remoteEffect->msecLastUpdate = remoteEffect->msecBegin;
        remoteEffect->distanceTraveled = 0.0;
        FX_SetEffectRandomSeed(remoteEffect, remoteDef);
        remoteEffect->firstTrailHandle = -1;
        FX_SpawnEffect_AllocTrails(system, remoteEffect);
        if (isSpotLightEffect)
            FX_SpawnEffect_AllocSpotLightEffect(system, remoteEffect);
        if ((remoteEffect->status & 0x7FE0000) != 0)
            MyAssertHandler(
                ".\\EffectsCore\\fx_system.cpp",
                1216,
                0,
                "%s\n\t(effect->status) = %i",
                "((effect->status & FX_STATUS_OWNED_EFFECTS_MASK) == 0)",
                remoteEffect->status);
        if (owner == 0xFFFF)
        {
            remoteEffect->owner = effectHandle;
            remoteEffect->status |= 0x10000000u;
        }
        else
        {
            remoteEffect->owner = owner;
            ownerEffect = FX_EffectFromHandle(system, owner);
            FX_AddRefToEffect(system, ownerEffect);
            oldStatusValue = InterlockedExchangeAdd(&ownerEffect->status, 0x20000);
            if ((oldStatusValue & 0xF801FFFF) != ((oldStatusValue + 0x20000) & 0xF801FFFF))
                MyAssertHandler(
                    ".\\EffectsCore\\fx_system.cpp",
                    1249,
                    0,
                    "%s\n\t(oldStatusValue) = %i",
                    "((oldStatusValue & ~FX_STATUS_OWNED_EFFECTS_MASK) == ((oldStatusValue + (1 << FX_STATUS_OWNED_EFFECTS_SHIFT)) "
                    "& ~FX_STATUS_OWNED_EFFECTS_MASK))",
                    oldStatusValue);
            if ((ownerEffect->status & 0x7FE0000) == 0)
                MyAssertHandler(
                    ".\\EffectsCore\\fx_system.cpp",
                    1250,
                    0,
                    "%s\n\t(ownerEffect->status) = %i",
                    "((ownerEffect->status & FX_STATUS_OWNED_EFFECTS_MASK) > 0)",
                    ownerEffect->status);
        }
        if (dobjHandle < 0)
            MyAssertHandler(".\\EffectsCore\\fx_system.cpp", 1253, 0, "%s", "dobjHandle >= 0");

        // KISAKTODO cleanup bitfield mess
        remoteEffect->boltAndSortOrder = (FxBoltAndSortOrder)(((boneIndex & 0x7FF) << 13)
            | *(_DWORD*)&remoteEffect->boltAndSortOrder & 0xFF001FFF);
        if (((*(_DWORD*)&remoteEffect->boltAndSortOrder >> 13) & 0x7FF) != boneIndex)
            MyAssertHandler(
                ".\\EffectsCore\\fx_system.cpp",
                1256,
                0,
                "effect->boltAndSortOrder.boneIndex == static_cast<uint>( boneIndex )\n\t%i, %i",
                (*(_DWORD*)&remoteEffect->boltAndSortOrder >> 13) & 0x7FF,
                boneIndex);
        *(_DWORD*)&remoteEffect->boltAndSortOrder &= ~0x1000u;
        if (markEntnum == 1023)
        {
            if (boneIndex == 2047 && dobjHandle != 4095)
                MyAssertHandler(
                    ".\\EffectsCore\\fx_system.cpp",
                    1269,
                    0,
                    "%s\n\t(dobjHandle) = %i",
                    "(!(boneIndex == ((1 << 11) - 1) && dobjHandle != ((1 << 12) - 1)))",
                    dobjHandle);
            if (boneIndex < 0)
                MyAssertHandler(".\\EffectsCore\\fx_system.cpp", 1270, 0, "%s", "boneIndex >= 0");
            remoteEffect->boltAndSortOrder = (FxBoltAndSortOrder)(dobjHandle & 0xFFF
                | *(_DWORD*)&remoteEffect->boltAndSortOrder & 0xFFFFF000);
            if ((*(_DWORD*)&remoteEffect->boltAndSortOrder & 0xFFF) != dobjHandle)
                MyAssertHandler(
                    ".\\EffectsCore\\fx_system.cpp",
                    1272,
                    0,
                    "effect->boltAndSortOrder.dobjHandle == static_cast<uint>( dobjHandle )\n\t%i, %i",
                    *(_DWORD*)&remoteEffect->boltAndSortOrder & 0xFFF,
                    dobjHandle);
            remoteEffect->boltAndSortOrder = (FxBoltAndSortOrder)((FX_GetBoltTemporalBits(system->localClientNum, dobjHandle) << 12)
                | *(_DWORD*)&remoteEffect->boltAndSortOrder & 0xFFFFEFFF);
        }
        else
        {
            if (boneIndex != 2047)
                MyAssertHandler(".\\EffectsCore\\fx_system.cpp", 1261, 0, "%s", "boneIndex == FX_BONE_INDEX_NONE");
            if (dobjHandle != 4095)
                MyAssertHandler(".\\EffectsCore\\fx_system.cpp", 1262, 0, "%s", "dobjHandle == FX_DOBJ_HANDLE_NONE");
            if (markEntnum >= 0xFFF)
                MyAssertHandler(
                    ".\\EffectsCore\\fx_system.cpp",
                    1263,
                    0,
                    "markEntnum doesn't index FX_DOBJ_HANDLE_NONE\n\t%i not in [0, %i)",
                    markEntnum,
                    4095);
            remoteEffect->boltAndSortOrder = (FxBoltAndSortOrder)(markEntnum & 0xFFF
                | *(_DWORD*)&remoteEffect->boltAndSortOrder & 0xFFFFF000);
            if ((*(_DWORD*)&remoteEffect->boltAndSortOrder & 0xFFF) != markEntnum)
                MyAssertHandler(
                    ".\\EffectsCore\\fx_system.cpp",
                    1265,
                    0,
                    "%s\n\t(markEntnum) = %i",
                    "(effect->boltAndSortOrder.dobjHandle == markEntnum)",
                    markEntnum);
        }
        remoteEffect->boltAndSortOrder = (FxBoltAndSortOrder)(((unsigned __int8)runnerSortOrder << 24)
            | *(_DWORD*)&remoteEffect->boltAndSortOrder & 0xFFFFFF);
        if ((unsigned __int8)HIBYTE(*(_DWORD*)&remoteEffect->boltAndSortOrder) != runnerSortOrder)
            MyAssertHandler(
                ".\\EffectsCore\\fx_system.cpp",
                1279,
                0,
                "effect->boltAndSortOrder.sortOrder == static_cast<uint>( runnerSortOrder )\n\t%i, %i",
                HIBYTE(*(_DWORD*)&remoteEffect->boltAndSortOrder),
                runnerSortOrder);
        remoteEffect->frameAtSpawn.origin[0] = *origin;
        remoteEffect->frameAtSpawn.origin[1] = origin[1];
        remoteEffect->frameAtSpawn.origin[2] = origin[2];
        AxisToQuat(axis, remoteEffect->frameAtSpawn.quat);
        memcpy(&remoteEffect->framePrev, &remoteEffect->frameAtSpawn, sizeof(remoteEffect->framePrev));
        memcpy(&remoteEffect->frameNow, &remoteEffect->frameAtSpawn, sizeof(remoteEffect->frameNow));
        Destination = &system->firstNewEffect;
        do
        {
            while (*Destination != allocIndex)
                ;
        } while (InterlockedCompareExchange(Destination, allocIndex + 1, allocIndex) != allocIndex);
        FX_StartNewEffect(system, remoteEffect);
        InterlockedExchangeAdd(&remoteEffect->status, -536870912);
        return remoteEffect;
    }
    else
    {
        R_WarnOncePerFrame(R_WARN_SPOT_LIGHT_LIMIT);
        return 0;
    }
}

void __cdecl FX_AddRefToEffect(FxSystem *__formal, FxEffect *effect)
{
    if (!effect)
        MyAssertHandler("c:\\trees\\cod3\\src\\effectscore\\fx_system.h", 369, 0, "%s", "effect");
    if (!(unsigned __int16)effect->status)
        MyAssertHandler(
            "c:\\trees\\cod3\\src\\effectscore\\fx_system.h",
            372,
            0,
            "%s\n\t(effect->status & FX_STATUS_REF_COUNT_MASK) = %i",
            "((effect->status & FX_STATUS_REF_COUNT_MASK) > 0)",
            (unsigned __int16)effect->status);
    InterlockedIncrement(&effect->status);
}

char __cdecl FX_CullEffectForSpawn(const FxCamera *camera, const FxEffectDef *effectDef, const float *origin)
{
    const FxElemDef *localDefs; // [esp+18h] [ebp-Ch]
    int elemDefCount; // [esp+1Ch] [ebp-8h]
    int elemDefIndex; // [esp+20h] [ebp-4h]

    elemDefCount = effectDef->elemDefCountOneShot + effectDef->elemDefCountLooping;
    localDefs = effectDef->elemDefs;
    for (elemDefIndex = 0; elemDefIndex < elemDefCount; ++elemDefIndex)
    {
        if (!FX_CullElemForSpawn(camera, &localDefs[elemDefIndex], origin))
            return 0;
    }
    return 1;
}

bool __cdecl FX_CullElemForSpawn(const FxCamera *camera, const FxElemDef *elemDef, const float *origin)
{
    float v4; // [esp+4h] [ebp-18h]
    float diff[3]; // [esp+Ch] [ebp-10h] BYREF
    float dist; // [esp+18h] [ebp-4h]

    if (elemDef->spawnRange.amplitude != 0.0)
    {
        Vec3Sub(camera->origin, origin, diff);
        v4 = Vec3Length(diff);
        dist = v4 - elemDef->spawnRange.base;
        if (dist < 0.0 || elemDef->spawnRange.amplitude < (double)dist)
            return 1;
    }
    return (elemDef->flags & 4) != 0
        && FX_CullSphere(camera, camera->frustumPlaneCount, origin, elemDef->spawnFrustumCullRadius);
}

void __cdecl FX_SetEffectRandomSeed(FxEffect *effect, const FxEffectDef *remoteDef)
{
    if (FX_EffectAffectsGameplay(remoteDef))
        effect->randomSeed = (479 * ((unsigned int)(214013 * effect->msecBegin + 2531011) >> 17)) >> 15;
    else
        effect->randomSeed = 479 * rand() / 0x8000;
}

char __cdecl FX_EffectAffectsGameplay(const FxEffectDef *remoteEffectDef)
{
    bool result; // [esp+7h] [ebp-19h]
    const FxElemDef *elemDef; // [esp+8h] [ebp-18h]
    unsigned int elemDefCount; // [esp+Ch] [ebp-14h]
    FxElemVisuals *visArray; // [esp+10h] [ebp-10h]
    unsigned int visIndex; // [esp+18h] [ebp-8h]
    unsigned int elemDefIndex; // [esp+1Ch] [ebp-4h]

    if (!remoteEffectDef)
        MyAssertHandler(".\\EffectsCore\\fx_system.cpp", 867, 0, "%s", "remoteEffectDef");
    elemDefCount = remoteEffectDef->elemDefCountEmission
        + remoteEffectDef->elemDefCountOneShot
        + remoteEffectDef->elemDefCountLooping;
    result = 0;
    for (elemDefIndex = 0; elemDefIndex < elemDefCount; ++elemDefIndex)
    {
        elemDef = &remoteEffectDef->elemDefs[elemDefIndex];
        if ((elemDef->flags & 0x1000) != 0)
            return 1;
        if (elemDef->effectOnDeath.handle && FX_EffectAffectsGameplay(elemDef->effectOnDeath.handle))
            return 1;
        if (elemDef->effectOnImpact.handle && FX_EffectAffectsGameplay(elemDef->effectOnImpact.handle))
            return 1;
        if (elemDef->effectEmitted.handle && FX_EffectAffectsGameplay(elemDef->effectEmitted.handle))
            return 1;
        if (elemDef->elemType == 10)
        {
            if (elemDef->visualCount == 1)
            {
                if (FX_EffectAffectsGameplay(elemDef->visuals.instance.effectDef.handle))
                    return 1;
            }
            else
            {
                visArray = elemDef->visuals.array;
                for (visIndex = 0; visIndex < elemDef->visualCount; ++visIndex)
                {
                    if (FX_EffectAffectsGameplay(visArray[visIndex].effectDef.handle))
                    {
                        result = 1;
                        break;
                    }
                }
            }
        }
    }
    return result;
}

char __cdecl FX_IsSpotLightEffect(FxSystem *system, const FxEffectDef *def)
{
    int elemDefIter; // [esp+4h] [ebp-4h]

    for (elemDefIter = 0;
        elemDefIter != def->elemDefCountOneShot + def->elemDefCountLooping + def->elemDefCountEmission;
        ++elemDefIter)
    {
        if (def->elemDefs[elemDefIter].elemType == 7)
            return 1;
    }
    return 0;
}

bool __cdecl FX_CanAllocSpotLightEffect(const FxSystem *system)
{
    return system->activeSpotLightEffectCount < 1;
}

char __cdecl FX_SpawnEffect_AllocSpotLightEffect(FxSystem *system, FxEffect *effect)
{
    const FxEffectDef *def; // [esp+4h] [ebp-10h]
    int elemDefCount; // [esp+Ch] [ebp-8h]
    int elemDefIter; // [esp+10h] [ebp-4h]

    def = effect->def;
    if (!effect->def)
        MyAssertHandler(".\\EffectsCore\\fx_system.cpp", 1031, 0, "%s", "def");
    elemDefCount = def->elemDefCountOneShot + def->elemDefCountLooping + def->elemDefCountEmission;
    for (elemDefIter = 0; elemDefIter != elemDefCount; ++elemDefIter)
    {
        if (effect->def->elemDefs[elemDefIter].elemType == 7)
        {
            if (system->activeSpotLightEffectCount >= 1)
                MyAssertHandler(
                    ".\\EffectsCore\\fx_system.cpp",
                    1040,
                    0,
                    "%s",
                    "system->activeSpotLightEffectCount < FX_SPOT_LIGHT_LIMIT");
            ++system->activeSpotLightEffectCount;
            system->activeSpotLightEffectHandle = FX_EffectToHandle(system, effect);
        }
    }
    return 1;
}

FxEffect *__cdecl FX_SpawnOrientedEffect(
    int localClientNum,
    const FxEffectDef *def,
    int msecBegin,
    const float *origin,
    const float (*axis)[3],
    unsigned int markEntnum)
{
    FxSystem *system; // [esp+0h] [ebp-4h]

    if (!fx_enable->current.enabled)
        return 0;
    system = FX_GetSystem(localClientNum);
    if (!system)
        MyAssertHandler(".\\EffectsCore\\fx_system.cpp", 1323, 0, "%s", "system");
    return FX_SpawnEffect(system, def, msecBegin, origin, axis, 4095, 2047, 255, 0xFFFFu, markEntnum);
}

void __cdecl FX_AssertAllocatedEffect(int localClientNum, FxEffect *effect)
{
    FxSystem *system; // [esp+0h] [ebp-4h]

    system = FX_GetSystem(localClientNum);
    if (!system)
        MyAssertHandler(".\\EffectsCore\\fx_system.cpp", 1335, 0, "%s", "system");
    FX_EffectToHandle(system, effect);
    if (!(unsigned __int16)effect->status)
        MyAssertHandler(".\\EffectsCore\\fx_system.cpp", 1337, 0, "%s", "(effect->status & FX_STATUS_REF_COUNT_MASK) != 0");
}

void __cdecl FX_PlayOrientedEffectWithMarkEntity(
    int localClientNum,
    const FxEffectDef *def,
    int startMsec,
    const float *origin,
    const float (*axis)[3],
    unsigned int markEntnum)
{
    FxEffect *effect; // [esp+4h] [ebp-8h]
    FxSystem *system; // [esp+8h] [ebp-4h]

    system = FX_GetSystem(localClientNum);
    effect = FX_SpawnOrientedEffect(localClientNum, def, startMsec, origin, axis, markEntnum);
    if (effect)
        FX_DelRefToEffect(system, effect);
}

void __cdecl FX_PlayOrientedEffect(
    int localClientNum,
    const FxEffectDef *def,
    int startMsec,
    const float *origin,
    const float (*axis)[3])
{
    FxEffect *effect; // [esp+4h] [ebp-8h]
    FxSystem *system; // [esp+8h] [ebp-4h]

    system = FX_GetSystem(localClientNum);
    effect = FX_SpawnOrientedEffect(localClientNum, def, startMsec, origin, axis, 0x3FFu);
    if (effect)
        FX_DelRefToEffect(system, effect);
}

FxEffect *__cdecl FX_SpawnBoltedEffect(
    int localClientNum,
    const FxEffectDef *def,
    int msecBegin,
    unsigned int dobjHandle,
    unsigned int boneIndex)
{
    orientation_t orient; // [esp+0h] [ebp-34h] BYREF
    FxSystem *system; // [esp+30h] [ebp-4h]

    if (!fx_enable->current.enabled)
        return 0;
    if (!FX_GetBoneOrientation(localClientNum, dobjHandle, boneIndex, &orient))
        return 0;
    if (FX_NeedsBoltUpdate(def))
    {
        if (dobjHandle >= 0xFFF)
            MyAssertHandler(
                ".\\EffectsCore\\fx_system.cpp",
                1408,
                0,
                "dobjHandle doesn't index FX_DOBJ_HANDLE_NONE\n\t%i not in [0, %i)",
                dobjHandle,
                4095);
        if (boneIndex >= 0x7FF)
            MyAssertHandler(
                ".\\EffectsCore\\fx_system.cpp",
                1409,
                0,
                "boneIndex doesn't index FX_BONE_INDEX_NONE\n\t%i not in [0, %i)",
                boneIndex,
                2047);
    }
    else
    {
        dobjHandle = 4095;
        boneIndex = 2047;
    }
    system = FX_GetSystem(localClientNum);
    return FX_SpawnEffect(system, def, msecBegin, orient.origin, orient.axis, dobjHandle, boneIndex, 255, 0xFFFFu, 0x3FFu);
}

char __cdecl FX_NeedsBoltUpdate(const FxEffectDef *def)
{
    int elemDefIndex; // [esp+4h] [ebp-4h]

    if (!def)
        MyAssertHandler(".\\EffectsCore\\fx_system.cpp", 1372, 0, "%s", "def");
    for (elemDefIndex = 0; elemDefIndex < def->elemDefCountOneShot + def->elemDefCountLooping; ++elemDefIndex)
    {
        if (def->elemDefs[elemDefIndex].elemType == 3)
            return 1;
        if ((def->elemDefs[elemDefIndex].flags & 0xC0) == 0x80)
            return 1;
    }
    return 0;
}

void __cdecl FX_PlayBoltedEffect(
    int localClientNum,
    const FxEffectDef *def,
    int startMsec,
    unsigned int dobjHandle,
    unsigned int boneIndex)
{
    FxEffect *effect; // [esp+4h] [ebp-8h]
    FxSystem *system; // [esp+8h] [ebp-4h]

    system = FX_GetSystem(localClientNum);
    effect = FX_SpawnBoltedEffect(localClientNum, def, startMsec, dobjHandle, boneIndex);
    if (effect)
        FX_DelRefToEffect(system, effect);
}
void __cdecl FX_RetriggerEffect(int localClientNum, FxEffect* effect, int msecBegin)
{
    volatile long* Destination; // [esp+1Ch] [ebp-54h]
    volatile LONG Comperand; // [esp+20h] [ebp-50h]
    unsigned __int16 lastOldTrailElemHandle[8]; // [esp+34h] [ebp-3Ch] BYREF
    int trailCount; // [esp+44h] [ebp-2Ch] BYREF
    unsigned __int16 lastElemHandle[5]; // [esp+48h] [ebp-28h] BYREF
    bool catchUpNewElems; // [esp+53h] [ebp-1Dh]
    unsigned __int16 firstOldElemHandle[4]; // [esp+54h] [ebp-1Ch] BYREF
    FxSystem* system; // [esp+64h] [ebp-Ch]
    bool hasPendingLoopElems; // [esp+6Bh] [ebp-5h]
    unsigned int elemClass; // [esp+6Ch] [ebp-4h]

    if (!(unsigned __int16)effect->status)
        MyAssertHandler(".\\EffectsCore\\fx_system.cpp", 1461, 0, "%s", "(effect->status & FX_STATUS_REF_COUNT_MASK) != 0");
    while (InterlockedExchangeAdd(&effect->status, 0x20000000) >= 0x20000000)
        InterlockedExchangeAdd(&effect->status, -536870912);
    system = FX_GetSystem(localClientNum);
    FX_AddRefToEffect(system, effect);
    if ((effect->status & 0x10000) != 0)
    {
        FX_SpawnAllFutureLooping(
            system,
            effect,
            0,
            effect->def->elemDefCountLooping,
            &effect->framePrev,
            &effect->frameNow,
            effect->msecBegin,
            effect->msecLastUpdate);
        FX_StopEffect(system, effect);
    }
    for (elemClass = 0; elemClass < 3; ++elemClass)
        firstOldElemHandle[elemClass] = effect->firstElemHandle[elemClass];
    FX_GetTrailHandleList_Last(system, effect, lastOldTrailElemHandle, &trailCount);
    catchUpNewElems = msecBegin < effect->msecLastUpdate;
    if (msecBegin > effect->msecLastUpdate)
    {
        for (elemClass = 0; elemClass < 3; ++elemClass)
            lastElemHandle[elemClass] = -1;
        FX_UpdateEffectPartial(
            system,
            effect,
            effect->msecLastUpdate,
            msecBegin,
            0.0,
            0.0,
            firstOldElemHandle,
            lastElemHandle,
            0,
            lastOldTrailElemHandle);
    }
    effect->msecBegin = msecBegin;
    effect->distanceTraveled = 0.0;
    FX_BeginLooping(
        system,
        effect,
        0,
        effect->def->elemDefCountLooping,
        &effect->frameNow,
        &effect->frameNow,
        msecBegin,
        msecBegin);
    FX_TriggerOneShot(
        system,
        effect,
        effect->def->elemDefCountLooping,
        effect->def->elemDefCountOneShot,
        &effect->frameNow,
        msecBegin);
    hasPendingLoopElems = effect->def->msecLoopingLife != 0;
    if (hasPendingLoopElems)
    {
        Destination = &effect->status;
        do
            Comperand = *Destination;
        while (InterlockedCompareExchange(Destination, Comperand | 0x10000, Comperand) != Comperand);
    }
    if (catchUpNewElems)
        FX_UpdateEffectPartial(
            system,
            effect,
            effect->msecBegin,
            effect->msecLastUpdate,
            0.0,
            0.0,
            effect->firstElemHandle,
            firstOldElemHandle,
            lastOldTrailElemHandle,
            0);
    FX_SortNewElemsInEffect(system, effect);
    if (!hasPendingLoopElems)
        FX_DelRefToEffect(system, effect);
    InterlockedExchangeAdd(&effect->status, -536870912);
}

void __cdecl FX_GetTrailHandleList_Last(
    FxSystem *system,
    FxEffect *effect,
    unsigned __int16 *outHandleList,
    int *outTrailCount)
{
    unsigned __int16 trailHandle; // [esp+0h] [ebp-Ch]
    FxPool<FxTrail> *trail; // [esp+4h] [ebp-8h]
    unsigned int trailIndex; // [esp+8h] [ebp-4h]

    trailIndex = 0;
    for (trailHandle = effect->firstTrailHandle; trailHandle != 0xFFFF; trailHandle = trail->item.nextTrailHandle)
    {
        if (!system)
            MyAssertHandler("c:\\trees\\cod3\\src\\effectscore\\fx_system.h", 362, 0, "%s", "system");
        trail = FX_PoolFromHandle_Generic<FxTrail, 128>(system->trails, trailHandle);
        if (trailIndex >= 2)
            MyAssertHandler(
                ".\\EffectsCore\\fx_system.cpp",
                1442,
                0,
                "%s\n\t(trailIndex) = %i",
                "(trailIndex < (sizeof( outHandleList ) / (sizeof( outHandleList[0] ) * (sizeof( outHandleList ) != 4 || sizeof( "
                "outHandleList[0] ) <= 4))))",
                trailIndex);
        outHandleList[trailIndex++] = trail->item.lastElemHandle;
    }
    *outTrailCount = trailIndex;
}

void __cdecl FX_ThroughWithEffect(int localClientNum, FxEffect *effect)
{
    FxSystem *system; // [esp+4h] [ebp-4h]

    system = FX_GetSystem(localClientNum);
    if (!system)
        MyAssertHandler(".\\EffectsCore\\fx_system.cpp", 1517, 0, "%s", "system");
    if (system->isInitialized)
    {
        if (!(unsigned __int16)effect->status)
            MyAssertHandler(
                ".\\EffectsCore\\fx_system.cpp",
                1521,
                0,
                "%s",
                "(effect->status & FX_STATUS_REF_COUNT_MASK) != 0");
        while (InterlockedExchangeAdd(&effect->status, 0x20000000) >= 0x20000000)
            InterlockedExchangeAdd(&effect->status, -536870912);
        FX_KillEffect(system, effect);
        InterlockedExchangeAdd(&effect->status, -536870912);
        FX_DelRefToEffect(system, effect);
    }
}

void __cdecl FX_StopEffect(FxSystem *system, FxEffect *effect)
{
    unsigned __int16 effectHandle; // [esp+20h] [ebp-14h]
    unsigned __int16 stoppedEffectHandle; // [esp+24h] [ebp-10h]
    FxEffect *otherEffect; // [esp+2Ch] [ebp-8h]
    volatile int activeIndex; // [esp+30h] [ebp-4h]

    if (!effect)
        MyAssertHandler(".\\EffectsCore\\fx_system.cpp", 1569, 0, "%s", "effect");
    if ((unsigned __int16)effect->status)
    {
        //Profile_Begin(213);
        FX_AddRefToEffect(system, effect);
        FX_StopEffectNonRecursive(system, effect);
        if((effect->status & 0x7FE0000) != 0)
        {
            stoppedEffectHandle = FX_EffectToHandle(system, effect);
            FX_BeginIteratingOverEffects_Cooperative(system);
            for (activeIndex = system->firstActiveEffect; activeIndex != system->firstNewEffect; ++activeIndex)
            {
                effectHandle = system->allEffectHandles[activeIndex & 0x3FF];
                if (effectHandle != stoppedEffectHandle)
                {
                    otherEffect = FX_EffectFromHandle(system, effectHandle);
                    if (otherEffect->owner == stoppedEffectHandle)
                        FX_StopEffect(system, otherEffect);
                }
            }
            if (!InterlockedDecrement(&system->iteratorCount) && system->needsGarbageCollection)
                FX_RunGarbageCollection(system);
        }
        FX_DelRefToEffect(system, effect);
        //Profile_EndInternal(0);
    }
    else if ((effect->status & 0x10000) != 0)
    {
        MyAssertHandler(
            ".\\EffectsCore\\fx_system.cpp",
            1573,
            0,
            "%s",
            "!(effect->status & FX_STATUS_HAS_PENDING_LOOP_ELEMS)");
    }
}

void __cdecl FX_StopEffectNonRecursive(FxSystem *system, FxEffect *effect)
{
    volatile int status; // [esp+4h] [ebp-4h]

    if (!effect)
        MyAssertHandler(".\\EffectsCore\\fx_system.cpp", 1541, 0, "%s", "effect");
    while (1)
    {
        status = effect->status;
        if ((status & 0x10000) == 0)
            break;
        if (InterlockedCompareExchange(&effect->status, status & 0xFFFEFFFF, status) == status)
        {
            FX_DelRefToEffect(system, effect);
            return;
        }
    }
}

void __cdecl FX_KillEffect(FxSystem* system, FxEffect* effect)
{
    unsigned __int16 effectHandle; // [esp+Ch] [ebp-14h]
    unsigned __int16 killedEffectHandle; // [esp+10h] [ebp-10h]
    FxEffect* otherEffect; // [esp+18h] [ebp-8h]
    volatile int activeIndex; // [esp+1Ch] [ebp-4h]

    if (!effect)
        MyAssertHandler(".\\EffectsCore\\fx_system.cpp", 1653, 0, "%s", "effect");
    if (!(unsigned __int16)effect->status)
        MyAssertHandler(".\\EffectsCore\\fx_system.cpp", 1654, 0, "%s", "(effect->status & FX_STATUS_REF_COUNT_MASK) != 0");
    if ((effect->status & 0x60000000) == 0)
        MyAssertHandler(".\\EffectsCore\\fx_system.cpp", 1656, 0, "%s", "(effect->status & FX_STATUS_IS_LOCKED_MASK) != 0");
    FX_AddRefToEffect(system, effect);
    FX_RemoveAllEffectElems(system, effect);
    if ((effect->status & 0x7FE0000) != 0)
    {
        killedEffectHandle = FX_EffectToHandle(system, effect);
        FX_BeginIteratingOverEffects_Cooperative(system);
        activeIndex = system->firstActiveEffect;
        while ((effect->status & 0x7FE0000) != 0)
        {
            if (activeIndex == system->firstNewEffect)
                MyAssertHandler(
                    ".\\EffectsCore\\fx_system.cpp",
                    1668,
                    1,
                    "activeIndex != system->firstNewEffect\n\t%i, %i",
                    activeIndex,
                    system->firstNewEffect);
            effectHandle = system->allEffectHandles[activeIndex & 0x3FF];
            if (effectHandle != killedEffectHandle)
            {
                otherEffect = FX_EffectFromHandle(system, effectHandle);
                if (otherEffect->owner == killedEffectHandle)
                {
                    while (InterlockedExchangeAdd(&otherEffect->status, 0x20000000) >= 0x20000000)
                        InterlockedExchangeAdd(&otherEffect->status, -536870912);
                    if ((otherEffect->status & 0x7FE0000) != 0)
                        MyAssertHandler(
                            ".\\EffectsCore\\fx_system.cpp",
                            1685,
                            0,
                            "%s\n\t(otherEffect->status) = %i",
                            "((otherEffect->status & FX_STATUS_OWNED_EFFECTS_MASK) == 0)",
                            otherEffect->status);
                    if ((unsigned __int16)otherEffect->status)
                        FX_RemoveAllEffectElems(system, otherEffect);
                    InterlockedExchangeAdd(&otherEffect->status, -536870912);
                }
            }
            ++activeIndex;
        }
        if (!InterlockedDecrement(&system->iteratorCount) && system->needsGarbageCollection)
            FX_RunGarbageCollection(system);
    }
    FX_DelRefToEffect(system, effect);
}

void __cdecl FX_RemoveAllEffectElems(FxSystem *system, FxEffect *effect)
{
    unsigned __int16 trailHandle; // [esp+4h] [ebp-Ch]
    FxPool<FxTrail> *trail; // [esp+8h] [ebp-8h]
    unsigned int elemClass; // [esp+Ch] [ebp-4h]

    if (!effect)
        MyAssertHandler(".\\EffectsCore\\fx_system.cpp", 1618, 0, "%s", "effect");
    FX_AddRefToEffect(system, effect);
    FX_StopEffect(system, effect);
    for (elemClass = 0; elemClass < 3; ++elemClass)
    {
        while (effect->firstElemHandle[elemClass] != 0xFFFF)
            FX_FreeElem(system, effect->firstElemHandle[elemClass], effect, elemClass);
    }
    for (trailHandle = effect->firstTrailHandle; trailHandle != 0xFFFF; trailHandle = trail->item.nextTrailHandle)
    {
        if (!system)
            MyAssertHandler("c:\\trees\\cod3\\src\\effectscore\\fx_system.h", 362, 0, "%s", "system");
        for (trail = FX_PoolFromHandle_Generic<FxTrail, 128>(system->trails, trailHandle);
            trail->item.firstElemHandle != 0xFFFF;
            FX_FreeTrailElem(system, trail->item.firstElemHandle, effect, (FxTrail *)trail))
        {
            ;
        }
    }
    if (system->activeSpotLightElemCount > 0
        && effect == FX_EffectFromHandle(system, system->activeSpotLightEffectHandle))
    {
        FX_FreeSpotLightElem(system, system->activeSpotLightElemHandle, effect);
    }
    FX_DelRefToEffect(system, effect);
}

void __cdecl FX_KillEffectDef(int localClientNum, const FxEffectDef *def)
{
    FxEffect *effect; // [esp+Ch] [ebp-Ch]
    FxSystem *system; // [esp+10h] [ebp-8h]
    int activeIndex; // [esp+14h] [ebp-4h]

    system = FX_GetSystem(localClientNum);
    FX_BeginIteratingOverEffects_Cooperative(system);
    for (activeIndex = system->firstActiveEffect; activeIndex != system->firstFreeEffect; ++activeIndex)
    {
        effect = FX_EffectFromHandle(system, system->allEffectHandles[activeIndex & 0x3FF]);
        if (effect->def == def && (unsigned __int16)effect->status)
        {
            while (InterlockedExchangeAdd(&effect->status, 0x20000000) >= 0x20000000)
                InterlockedExchangeAdd(&effect->status, -536870912);
            FX_KillEffect(system, effect);
            InterlockedExchangeAdd(&effect->status, -536870912);
        }
    }
    if (!InterlockedDecrement(&system->iteratorCount) && system->needsGarbageCollection)
        FX_RunGarbageCollection(system);
}

void __cdecl FX_KillAllEffects(int localClientNum)
{
    FxEffect *effect; // [esp+Ch] [ebp-Ch]
    FxSystem *system; // [esp+10h] [ebp-8h]
    int activeIndex; // [esp+14h] [ebp-4h]

    system = FX_GetSystem(localClientNum);
    if (!system)
        MyAssertHandler(".\\EffectsCore\\fx_system.cpp", 1732, 0, "%s", "system");
    if (system->isInitialized)
    {
        FX_BeginIteratingOverEffects_Cooperative(system);
        for (activeIndex = system->firstActiveEffect; activeIndex != system->firstNewEffect; ++activeIndex)
        {
            effect = FX_EffectFromHandle(system, system->allEffectHandles[activeIndex & 0x3FF]);
            if ((unsigned __int16)effect->status)
            {
                while (InterlockedExchangeAdd(&effect->status, 0x20000000) >= 0x20000000)
                    InterlockedExchangeAdd(&effect->status, -536870912);
                FX_KillEffect(system, effect);
                InterlockedExchangeAdd(&effect->status, -536870912);
            }
        }
        if (!InterlockedDecrement(&system->iteratorCount) && system->needsGarbageCollection)
            FX_RunGarbageCollection(system);
    }
}

void __cdecl FX_SpawnTrailElem_NoCull(
    FxSystem *system,
    FxEffect *effect,
    FxTrail *trail,
    const FxSpatialFrame *effectFrameWhenPlayed,
    int msecWhenPlayed,
    float distanceWhenPlayed)
{
    unsigned __int16 lastElemHandle; // [esp+12h] [ebp-4Ah]
    bool v7; // [esp+1Bh] [ebp-41h]
    int msecBegin; // [esp+20h] [ebp-3Ch]
    const FxElemDef *elemDef; // [esp+2Ch] [ebp-30h]
    unsigned int randomSeed; // [esp+30h] [ebp-2Ch]
    FxPool<FxTrailElem> *remoteTrailElem; // [esp+38h] [ebp-24h]
    float basis[2][3]; // [esp+3Ch] [ebp-20h] BYREF
    unsigned __int16 trailElemHandle; // [esp+54h] [ebp-8h]
    FxTrailElem *lastTrailElemInEffect; // [esp+58h] [ebp-4h]

    if (!system)
        MyAssertHandler(".\\EffectsCore\\fx_system.cpp", 1916, 0, "%s", "system");
    if (!effect)
        MyAssertHandler(".\\EffectsCore\\fx_system.cpp", 1917, 0, "%s", "effect");
    if (!effect->def)
        MyAssertHandler(".\\EffectsCore\\fx_system.cpp", 1919, 0, "%s", "effectDef");
    if (!trail)
        MyAssertHandler(".\\EffectsCore\\fx_system.cpp", 1920, 0, "%s", "trail");
    elemDef = &effect->def->elemDefs[trail->defIndex];
    if (elemDef->elemType != 3)
        MyAssertHandler(
            ".\\EffectsCore\\fx_system.cpp",
            1923,
            0,
            "%s\n\t(elemDef->elemType) = %i",
            "(elemDef->elemType == FX_ELEM_TYPE_TRAIL)",
            elemDef->elemType);
    msecBegin = elemDef->spawnDelayMsec.base + msecWhenPlayed;
    if (elemDef->spawnDelayMsec.amplitude)
        msecBegin += ((elemDef->spawnDelayMsec.amplitude + 1)
            * LOWORD(fx_randomTable[(msecBegin + (unsigned int)effect->randomSeed + 296 * trail->sequence) % 0x1DF
                + 18])) >> 16;
    randomSeed = (296 * trail->sequence + msecBegin + (unsigned int)effect->randomSeed) % 0x1DF;
    if (elemDef->effectOnImpact.handle)
    {
        v7 = 1;
    }
    else if (elemDef->effectOnDeath.handle)
    {
        v7 = 1;
    }
    else
    {
        v7 = elemDef->effectEmitted.handle != 0;
    }
    if (v7
        || msecBegin
        + (((elemDef->lifeSpanMsec.amplitude + 1) * LOWORD(fx_randomTable[randomSeed + 17])) >> 16)
        + elemDef->lifeSpanMsec.base > system->msecNow)
    {
        remoteTrailElem = FX_AllocTrailElem(system);
        if (remoteTrailElem)
        {
            FX_AddRefToEffect(system, effect);
            FX_GetOriginForTrailElem(
                effect,
                elemDef,
                effectFrameWhenPlayed,
                randomSeed,
                remoteTrailElem->item.origin,
                basis[0],
                basis[1]);
            if (!system)
                MyAssertHandler("c:\\trees\\cod3\\src\\effectscore\\fx_system.h", 341, 0, "%s", "system");
            trailElemHandle = FX_PoolToHandle_Generic<FxTrailElem, 2048>(system->trailElems, (FxTrailElem *)remoteTrailElem);
            if (trail->lastElemHandle == 0xFFFF)
            {
                if (trail->firstElemHandle != 0xFFFF)
                    MyAssertHandler(".\\EffectsCore\\fx_system.cpp", 1954, 0, "%s", "trail->firstElemHandle == FX_HANDLE_NONE");
                trail->firstElemHandle = trailElemHandle;
            }
            else
            {
                lastElemHandle = trail->lastElemHandle;
                if (!system)
                    MyAssertHandler("c:\\trees\\cod3\\src\\effectscore\\fx_system.h", 348, 0, "%s", "system");
                lastTrailElemInEffect = (FxTrailElem *)FX_PoolFromHandle_Generic<FxTrailElem, 2048>(
                    system->trailElems,
                    lastElemHandle);
                lastTrailElemInEffect->nextTrailElemHandle = trailElemHandle;
            }
            remoteTrailElem->item.nextTrailElemHandle = -1;
            trail->lastElemHandle = trailElemHandle;
            remoteTrailElem->item.msecBegin = msecBegin;
            remoteTrailElem->item.spawnDist = distanceWhenPlayed;
            remoteTrailElem->item.baseVelZ = 0;
            remoteTrailElem->item.sequence = trail->sequence++;
            FX_TrailElem_CompressBasis(basis, remoteTrailElem->item.basis);
        }
    }
}

FxPool<FxTrailElem> *__cdecl FX_AllocTrailElem(FxSystem *system)
{
    return FX_AllocPool_Generic_FxTrailElem_(
        &system->firstFreeTrailElem,
        system->trailElems,
        &system->activeTrailElemCount);
}

void __cdecl FX_SpawnTrailElem_Cull(
    FxSystem *system,
    FxEffect *effect,
    FxTrail *trail,
    const FxSpatialFrame *effectFrameWhenPlayed,
    int msecWhenPlayed,
    float distanceWhenPlayed)
{
    const FxElemDef *elemDef; // [esp+28h] [ebp-4h]

    if (!system)
        MyAssertHandler(".\\EffectsCore\\fx_system.cpp", 1981, 0, "%s", "system");
    if (!effect)
        MyAssertHandler(".\\EffectsCore\\fx_system.cpp", 1982, 0, "%s", "effect");
    if (!effect->def)
        MyAssertHandler(".\\EffectsCore\\fx_system.cpp", 1984, 0, "%s", "effectDef");
    if (!trail)
        MyAssertHandler(".\\EffectsCore\\fx_system.cpp", 1985, 0, "%s", "trail");
    elemDef = &effect->def->elemDefs[trail->defIndex];
    if (elemDef->elemType != 3)
        MyAssertHandler(
            ".\\EffectsCore\\fx_system.cpp",
            1988,
            0,
            "%s\n\t(elemDef->elemType) = %i",
            "(elemDef->elemType == FX_ELEM_TYPE_TRAIL)",
            elemDef->elemType);
    if (FX_CullTrailElem(&system->cameraPrev, elemDef, effectFrameWhenPlayed->origin, trail->sequence))
        ++trail->sequence;
    else
        FX_SpawnTrailElem_NoCull(system, effect, trail, effectFrameWhenPlayed, msecWhenPlayed, distanceWhenPlayed);
}

bool __cdecl FX_CullTrailElem(
    const FxCamera *camera,
    const FxElemDef *elemDef,
    const float *origin,
    unsigned __int8 sequence)
{
    float diff[3]; // [esp+0h] [ebp-1Ch] BYREF
    float cutoffMultiple; // [esp+Ch] [ebp-10h]
    float cutoffDist; // [esp+10h] [ebp-Ch]
    float distSq; // [esp+14h] [ebp-8h]
    float baseCutoffDist; // [esp+18h] [ebp-4h]

    baseCutoffDist = elemDef->spawnRange.base + elemDef->spawnRange.amplitude;
    if (baseCutoffDist == 0.0)
        return 0;
    if (!sequence)
        return 0;
    cutoffMultiple = 1.0;
    while ((sequence & 1) == 0)
    {
        cutoffMultiple = cutoffMultiple + 1.0;
        sequence >>= 1;
    }
    cutoffDist = baseCutoffDist * cutoffMultiple;
    Vec3Sub(camera->origin, origin, diff);
    distSq = Vec3LengthSq(diff);
    return distSq > cutoffDist * cutoffDist;
}

void __cdecl FX_SpawnSpotLightElem(FxSystem *system, FxElem *elem)
{
    if (system->activeSpotLightEffectCount <= 0)
        MyAssertHandler(".\\EffectsCore\\fx_system.cpp", 2038, 0, "%s", "system->activeSpotLightEffectCount > 0");
    if (system->activeSpotLightElemCount)
        MyAssertHandler(".\\EffectsCore\\fx_system.cpp", 2039, 0, "%s", "system->activeSpotLightElemCount == 0");
    ++system->activeSpotLightElemCount;
    if (!system)
        MyAssertHandler("c:\\trees\\cod3\\src\\effectscore\\fx_system.h", 327, 0, "%s", "system");
    system->activeSpotLightElemHandle = FX_PoolToHandle_Generic<FxElem, 2048>(system->elems, elem);
}

void __cdecl FX_SpawnElem(
    FxSystem *system,
    FxEffect *effect,
    int elemDefIndex,
    const FxSpatialFrame *effectFrameWhenPlayed,
    int msecWhenPlayed,
    float distanceWhenPlayed,
    int sequence)
{
    unsigned __int16 v7; // ax
    unsigned __int16 nextElemHandleInEffect; // [esp+0h] [ebp-80h]
    unsigned __int8 elemType; // [esp+3h] [ebp-7Dh]
    bool v10; // [esp+47h] [ebp-39h]
    int msecBegin; // [esp+64h] [ebp-1Ch]
    const FxElemDef *elemDef; // [esp+6Ch] [ebp-14h]
    unsigned int randomSeed; // [esp+74h] [ebp-Ch]
    FxPool<FxElem> *elem; // [esp+78h] [ebp-8h]
    unsigned int elemClass; // [esp+7Ch] [ebp-4h]

    if (!system)
        MyAssertHandler(".\\EffectsCore\\fx_system.cpp", 2062, 0, "%s", "system");
    if (!effect)
        MyAssertHandler(".\\EffectsCore\\fx_system.cpp", 2063, 0, "%s", "effect");
    if (!effect->def)
        MyAssertHandler(".\\EffectsCore\\fx_system.cpp", 2064, 0, "%s", "effect->def");
    elemDef = &effect->def->elemDefs[elemDefIndex];
    if (elemDef->elemType == 3)
        MyAssertHandler(".\\EffectsCore\\fx_system.cpp", 2066, 0, "%s", "elemDef->elemType != FX_ELEM_TYPE_TRAIL");
    if (!fx_cull_elem_spawn->current.enabled
        || !FX_CullElemForSpawn(&system->cameraPrev, elemDef, effectFrameWhenPlayed->origin))
    {
        msecBegin = elemDef->spawnDelayMsec.base + msecWhenPlayed;
        if (elemDef->spawnDelayMsec.amplitude)
            msecBegin += ((elemDef->spawnDelayMsec.amplitude + 1)
                * LOWORD(fx_randomTable[(296 * sequence + msecBegin + (unsigned int)effect->randomSeed) % 0x1DF + 18])) >> 16;
        randomSeed = (msecBegin + effect->randomSeed + 296 * (unsigned int)(unsigned __int8)sequence) % 0x1DF;
        switch (elemDef->elemType)
        {
        case 0xAu:
            FX_SpawnRunner(system, effect, elemDef, effectFrameWhenPlayed, randomSeed, msecBegin);
            break;
        case 9u:
            if (((*(unsigned int *)&effect->boltAndSortOrder >> 13) & 0x7FF) != 0x7FF
                || (*(unsigned int *)&effect->boltAndSortOrder & 0xFFF) == 0xFFF)
            {
                FX_CreateImpactMark(system->localClientNum, elemDef, effectFrameWhenPlayed, randomSeed, 0x3FFu);
            }
            else
            {
                FX_CreateImpactMark(
                    system->localClientNum,
                    elemDef,
                    effectFrameWhenPlayed,
                    randomSeed,
                    *(unsigned int *)&effect->boltAndSortOrder & 0xFFF);
            }
            break;
        case 8u:
            FX_SpawnSound(system->localClientNum, effect, elemDef, effectFrameWhenPlayed, randomSeed);
            break;
        default:
            if (elemDef->effectOnImpact.handle)
            {
                v10 = 1;
            }
            else if (elemDef->effectOnDeath.handle)
            {
                v10 = 1;
            }
            else
            {
                v10 = elemDef->effectEmitted.handle != 0;
            }
            if (v10
                || msecBegin
                + (((elemDef->lifeSpanMsec.amplitude + 1) * LOWORD(fx_randomTable[randomSeed + 17])) >> 16)
                + elemDef->lifeSpanMsec.base > system->msecNow)
            {
                elem = FX_AllocElem(system);
                if (elem)
                {
                    FX_AddRefToEffect(system, effect);
                    elem->item.defIndex = elemDefIndex;
                    elem->item.sequence = sequence;
                    elem->item.atRestFraction = -1;
                    elem->item.emitResidual = 0;
                    elem->item.msecBegin = msecBegin;
                    if (randomSeed != (296 * elem->item.sequence + elem->item.msecBegin + (unsigned int)effect->randomSeed)
                        % 0x1DF)
                        MyAssertHandler(
                            ".\\EffectsCore\\fx_system.cpp",
                            2147,
                            0,
                            "%s",
                            "randomSeed == FX_ElemRandomSeed( effect->randomSeed, elem->msecBegin, elem->sequence )");
                    FX_GetOriginForElem(effect, elemDef, effectFrameWhenPlayed, randomSeed, elem->item.origin);
                    elem->item.baseVel[0] = 0.0;
                    elem->item.baseVel[1] = 0.0;
                    elem->item.baseVel[2] = 0.0;
                    if (elemDef->elemType == 3)
                        elem->item.u.trailTexCoord = distanceWhenPlayed / (double)elemDef->trailDef->repeatDist;
                    elem->item.prevElemHandleInEffect = -1;
                    if (elemDef->elemType == 7)
                    {
                        FX_SpawnSpotLightElem(system, (FxElem *)elem);
                    }
                    else
                    {
                        elemType = elemDef->elemType;
                        if (elemType > 3u)
                        {
                            if (elemType == 4)
                                elemClass = 2;
                            else
                                elemClass = 1;
                        }
                        else
                        {
                            elemClass = 0;
                        }
                        elem->item.nextElemHandleInEffect = effect->firstElemHandle[elemClass];
                        if (!system)
                            MyAssertHandler("c:\\trees\\cod3\\src\\effectscore\\fx_system.h", 327, 0, "%s", "system");
                        effect->firstElemHandle[elemClass] = FX_PoolToHandle_Generic<FxElem, 2048>(system->elems, (FxElem *)elem);
                        if (elem->item.nextElemHandleInEffect != 0xFFFF)
                        {
                            nextElemHandleInEffect = elem->item.nextElemHandleInEffect;
                            if (!system)
                                MyAssertHandler("c:\\trees\\cod3\\src\\effectscore\\fx_system.h", 334, 0, "%s", "system");
                            FX_PoolFromHandle_Generic<FxElem, 2048>(system->elems, nextElemHandleInEffect)->item.prevElemHandleInEffect = effect->firstElemHandle[elemClass];
                        }
                        if (elemDef->elemType == 5)
                        {
                            elem->item.u.lightingHandle = 0;
                            if ((elemDef->flags & 0x8000000) != 0
                                && !FX_SpawnModelPhysics(system, effect, elemDef, randomSeed, (FxElem*)elem))
                            {
                                if (!system)
                                    MyAssertHandler("c:\\trees\\cod3\\src\\effectscore\\fx_system.h", 327, 0, "%s", "system");
                                v7 = FX_PoolToHandle_Generic<FxElem, 2048>(system->elems, (FxElem*)elem);
                                FX_FreeElem(system, v7, effect, elemClass);
                            }
                        }
                    }
                }
                else
                {
                    R_WarnOncePerFrame(R_WARN_FX_ELEM_LIMIT);
                }
            }
            break;
        }
    }
}

FxPool<FxElem> *__cdecl FX_AllocElem(FxSystem *system)
{
    return FX_AllocPool_Generic_FxElem_(&system->firstFreeElem, system->elems, &system->activeElemCount);
}

void __cdecl FX_SpawnRunner(
    FxSystem *system,
    FxEffect *effect,
    const FxElemDef *remoteElemDef,
    const FxSpatialFrame *effectFrameWhenPlayed,
    int randomSeed,
    int msecWhenPlayed)
{
    int v6; // [esp+0h] [ebp-88h]
    int sortOrder; // [esp+Ch] [ebp-7Ch]
    const FxEffectDef *effectDef; // [esp+20h] [ebp-68h]
    FxEffect *spawnedEffect; // [esp+28h] [ebp-60h]
    float *usedAxis; // [esp+30h] [ebp-58h]
    float rotatedAxis[3][3]; // [esp+34h] [ebp-54h] BYREF
    float spawnOrigin[3]; // [esp+58h] [ebp-30h] BYREF
    float axis[3][3]; // [esp+64h] [ebp-24h] BYREF

    FX_GetSpawnOrigin(effectFrameWhenPlayed, remoteElemDef, randomSeed, spawnOrigin);
    FX_OffsetSpawnOrigin(effectFrameWhenPlayed, remoteElemDef, randomSeed, spawnOrigin);
    effectDef = FX_GetElemVisuals(remoteElemDef, randomSeed).effectDef.handle;
    UnitQuatToAxis(effectFrameWhenPlayed->quat, axis);
    if ((remoteElemDef->flags & 8) != 0)
    {
        FX_RandomlyRotateAxis(axis, randomSeed, rotatedAxis);
        usedAxis = rotatedAxis[0];
    }
    else
    {
        usedAxis = axis[0];
    }
    if (remoteElemDef->sortOrder == 255)
        sortOrder = 255;
    else
        sortOrder = remoteElemDef->sortOrder;
    if (sortOrder > 0)
        v6 = sortOrder;
    else
        v6 = 0;
    if (((*(unsigned int *)&effect->boltAndSortOrder >> 13) & 0x7FF) == 0x7FF)
    {
        if ((*(unsigned int *)&effect->boltAndSortOrder & 0xFFF) == 0xFFF)
            spawnedEffect = FX_SpawnEffect(
                system,
                effectDef,
                msecWhenPlayed,
                spawnOrigin,
                (const float (*)[3])usedAxis,
                4095,
                2047,
                v6,
                effect->owner,
                0x3FFu);
        else
            spawnedEffect = FX_SpawnEffect(
                system,
                effectDef,
                msecWhenPlayed,
                spawnOrigin,
                (const float (*)[3])usedAxis,
                4095,
                2047,
                v6,
                effect->owner,
                *(unsigned int *)&effect->boltAndSortOrder & 0xFFF);
    }
    else
    {
        spawnedEffect = FX_SpawnEffect(
            system,
            effectDef,
            msecWhenPlayed,
            spawnOrigin,
            (const float (*)[3])usedAxis,
            *(unsigned int *)&effect->boltAndSortOrder & 0xFFF,
            (*(unsigned int *)&effect->boltAndSortOrder >> 13) & 0x7FF,
            v6,
            effect->owner,
            0x3FFu);
    }
    if (spawnedEffect)
        FX_DelRefToEffect(system, spawnedEffect);
}

bool __cdecl FX_SpawnModelPhysics(
    FxSystem* system,
    FxEffect* effect,
    const FxElemDef* elemDef,
    int randomSeed,
    FxElem* elem)
{
    float v6; // [esp+14h] [ebp-C8h]
    float v7; // [esp+18h] [ebp-C4h]
    float v8; // [esp+1Ch] [ebp-C0h]
    float velocity[3]; // [esp+4Ch] [ebp-90h] BYREF
    float angularVelocity[3]; // [esp+58h] [ebp-84h] BYREF
    FxElemVisuals visuals; // [esp+64h] [ebp-78h]
    float msecLifeSpan; // [esp+68h] [ebp-74h]
    float quat[4]; // [esp+6Ch] [ebp-70h] BYREF
    orientation_t orient; // [esp+7Ch] [ebp-60h] BYREF
    float worldOrigin[3]; // [esp+ACh] [ebp-30h] BYREF
    float axis[3][3]; // [esp+B8h] [ebp-24h] BYREF

    FX_GetOrientation(elemDef, &effect->frameAtSpawn, &effect->frameNow, randomSeed, &orient);
    FX_OrientationPosToWorldPos(&orient, elem->origin, worldOrigin);
    FX_GetElemAxis(elemDef, randomSeed, &orient, 0.0, axis);
    AxisToQuat((const float (*)[3][3])axis, quat);
    msecLifeSpan = (float)((((elemDef->lifeSpanMsec.amplitude + 1) * LOWORD(fx_randomTable[randomSeed + 17])) >> 16)
        + elemDef->lifeSpanMsec.base);
    FX_GetVelocityAtTime(elemDef, randomSeed, msecLifeSpan, 0.0, &orient, elem->baseVel, velocity);
    v8 = elemDef->angularVelocity[0].amplitude * fx_randomTable[randomSeed + 3] + elemDef->angularVelocity[0].base;
    angularVelocity[0] = v8 * 1000.0;
    v7 = elemDef->angularVelocity[1].amplitude * fx_randomTable[randomSeed + 4] + elemDef->angularVelocity[1].base;
    angularVelocity[1] = v7 * 1000.0;
    v6 = elemDef->angularVelocity[2].amplitude * fx_randomTable[randomSeed + 5] + elemDef->angularVelocity[2].base;
    angularVelocity[2] = v6 * 1000.0;
    Sys_EnterCriticalSection(CRITSECT_PHYSICS);
    visuals.anonymous = FX_GetElemVisuals(elemDef, randomSeed).anonymous;
    if (!*((_DWORD*)visuals.anonymous + 53))
        MyAssertHandler(".\\EffectsCore\\fx_system.cpp", 1853, 0, "%s", "visuals.model->physPreset");
    elem->physObjId = (int)Phys_ObjCreate(
        PHYS_WORLD_FX,
        worldOrigin,
        quat,
        velocity,
        *((const PhysPreset**)visuals.anonymous + 53));
    if (elem->physObjId)
    {
        Phys_ObjSetCollisionFromXModel(visuals.model, PHYS_WORLD_FX, (dxBody*)elem->physObjId);
        Phys_ObjSetAngularVelocity((dxBody*)elem->physObjId, angularVelocity);
    }
    Sys_LeaveCriticalSection(CRITSECT_PHYSICS);
    return elem->physObjId != 0;
}

void __cdecl FX_GetOriginForElem(
    FxEffect *effect,
    const FxElemDef *elemDef,
    const FxSpatialFrame *effectFrameWhenPlayed,
    int randomSeed,
    float *outOrigin)
{
    FxSpatialFrame *p_frameAtSpawn; // [esp+0h] [ebp-3Ch]
    float effectFrameAxis[3][3]; // [esp+4h] [ebp-38h] BYREF
    const FxSpatialFrame *effectFrame; // [esp+28h] [ebp-14h]
    float delta[3]; // [esp+2Ch] [ebp-10h] BYREF
    int runFlags; // [esp+38h] [ebp-4h]

    runFlags = elemDef->flags & 0xC0;
    if (runFlags == 64)
        p_frameAtSpawn = &effect->frameAtSpawn;
    else
        p_frameAtSpawn = effectFrameWhenPlayed;
    effectFrame = p_frameAtSpawn;
    if (runFlags == 192)
    {
        *outOrigin = 0.0;
        outOrigin[1] = 0.0;
        outOrigin[2] = 0.0;
    }
    else
    {
        UnitQuatToAxis(effectFrame->quat, effectFrameAxis);
        FX_GetSpawnOrigin(effectFrame, elemDef, randomSeed, outOrigin);
        FX_OffsetSpawnOrigin(effectFrame, elemDef, randomSeed, outOrigin);
        if (runFlags == 128 || runFlags == 64)
        {
            Vec3Sub(outOrigin, effectFrame->origin, delta);
            *outOrigin = Vec3Dot(delta, effectFrameAxis[0]);
            outOrigin[1] = Vec3Dot(delta, effectFrameAxis[1]);
            outOrigin[2] = Vec3Dot(delta, effectFrameAxis[2]);
        }
    }
}

void __cdecl FX_SpawnSound(
    int localClientNumber,
    FxEffect *effect,
    const FxElemDef *elemDef,
    const FxSpatialFrame *effectFrameWhenPlayed,
    int randomSeed)
{
    FxElemVisuals visuals; // [esp+Ch] [ebp-14h]
    snd_alias_list_t *alias_list; // [esp+10h] [ebp-10h]
    float spawnOrigin[3]; // [esp+14h] [ebp-Ch] BYREF

    if (elemDef->elemType != 8)
        MyAssertHandler(
            ".\\EffectsCore\\fx_system.cpp",
            2007,
            0,
            "%s\n\t(elemDef->elemType) = %i",
            "(elemDef->elemType == FX_ELEM_TYPE_SOUND)",
            elemDef->elemType);
    FX_GetSpawnOrigin(effectFrameWhenPlayed, elemDef, randomSeed, spawnOrigin);
    FX_OffsetSpawnOrigin(effectFrameWhenPlayed, elemDef, randomSeed, spawnOrigin);
    visuals.anonymous = FX_GetElemVisuals(elemDef, randomSeed).anonymous;
    alias_list = Com_FindSoundAlias(visuals.effectDef.name);
    if (alias_list)
    {
        if (SND_AnyActiveListeners())
        {
            if (Sys_IsMainThread())
                CG_PlaySoundAlias(localClientNumber, 1022, spawnOrigin, alias_list);
            else
                CG_AddFXSoundAlias(localClientNumber, spawnOrigin, alias_list);
        }
    }
    else
    {
        Com_PrintWarning(21, "Failed to find sound alias '%s'\n", visuals.effectDef.name);
    }
}

void __cdecl FX_FreeElem(FxSystem* system, unsigned __int16 elemHandle, FxEffect* effect, unsigned int elemClass)
{
    unsigned __int16 prevElemHandleInEffect; // [esp+10h] [ebp-14h]
    unsigned __int16 nextElemHandleInEffect; // [esp+12h] [ebp-12h]
    const FxElemDef* elemDef; // [esp+14h] [ebp-10h]
    FxPool<FxElem>* elem; // [esp+20h] [ebp-4h]

    if (!system)
        MyAssertHandler("c:\\trees\\cod3\\src\\effectscore\\fx_system.h", 334, 0, "%s", "system");
    elem = FX_PoolFromHandle_Generic<FxElem, 2048>(system->elems, elemHandle);
    if (!elemClass && effect->firstSortedElemHandle == elemHandle)
        effect->firstSortedElemHandle = elem->item.nextElemHandleInEffect;
    if (elem->item.nextElemHandleInEffect != 0xFFFF)
    {
        nextElemHandleInEffect = elem->item.nextElemHandleInEffect;
        if (!system)
            MyAssertHandler("c:\\trees\\cod3\\src\\effectscore\\fx_system.h", 334, 0, "%s", "system");
        FX_PoolFromHandle_Generic<FxElem, 2048>(system->elems, nextElemHandleInEffect)->item.prevElemHandleInEffect = elem->item.prevElemHandleInEffect;
    }
    if (elem->item.prevElemHandleInEffect == 0xFFFF)
    {
        if (effect->firstElemHandle[elemClass] != elemHandle)
            MyAssertHandler(
                ".\\EffectsCore\\fx_system.cpp",
                2207,
                0,
                "%s",
                "effect->firstElemHandle[elemClass] == elemHandle");
        effect->firstElemHandle[elemClass] = elem->item.nextElemHandleInEffect;
    }
    else
    {
        prevElemHandleInEffect = elem->item.prevElemHandleInEffect;
        if (!system)
            MyAssertHandler("c:\\trees\\cod3\\src\\effectscore\\fx_system.h", 334, 0, "%s", "system");
        FX_PoolFromHandle_Generic<FxElem, 2048>(system->elems, prevElemHandleInEffect)->item.nextElemHandleInEffect = elem->item.nextElemHandleInEffect;
    }
    elemDef = &effect->def->elemDefs[elem->item.defIndex];
    if (elemDef->elemType == 5 && (elemDef->flags & 0x8000000) != 0 && elem->item.physObjId)
    {
        Sys_EnterCriticalSection(CRITSECT_PHYSICS);
        Phys_ObjDestroy(PHYS_WORLD_FX, (dxBody*)elem->item.physObjId);
        Sys_LeaveCriticalSection(CRITSECT_PHYSICS);
    }
    elem->nextFree = 0;
    *(_DWORD*)&elem->item.nextElemHandleInEffect = 0;
    elem->item.msecBegin = 0;
    elem->item.baseVel[0] = 0.0;
    elem->item.baseVel[1] = 0.0;
    elem->item.baseVel[2] = 0.0;
    elem->item.physObjId = 0;
    elem->item.origin[1] = 0.0;
    elem->item.origin[2] = 0.0;
    elem->item.u.trailTexCoord = 0.0;
    FX_FreePool_Generic_FxElem_((FxElem*)elem, &system->firstFreeElem, system->elems);
    FX_DelRefToEffect(system, effect);
    InterlockedDecrement(&system->activeElemCount);
}

void __cdecl FX_FreeTrailElem(FxSystem *system, unsigned __int16 trailElemHandle, FxEffect *effect, FxTrail *trail)
{
    FxPool<FxTrailElem> *trailElem; // [esp+10h] [ebp-4h]

    if (!system)
        MyAssertHandler("c:\\trees\\cod3\\src\\effectscore\\fx_system.h", 348, 0, "%s", "system");
    trailElem = FX_PoolFromHandle_Generic<FxTrailElem, 2048>(system->trailElems, trailElemHandle);
    if (trail->firstElemHandle != trailElemHandle)
        MyAssertHandler(".\\EffectsCore\\fx_system.cpp", 2256, 0, "%s", "trail->firstElemHandle == trailElemHandle");
    if (trail->lastElemHandle == trailElemHandle)
    {
        if (trail->firstElemHandle != trailElemHandle)
            MyAssertHandler(".\\EffectsCore\\fx_system.cpp", 2260, 0, "%s", "trail->firstElemHandle == trailElemHandle");
        trail->lastElemHandle = -1;
    }
    trail->firstElemHandle = trailElem->item.nextTrailElemHandle;
    trailElem->nextFree = 0;
    trailElem->item.origin[1] = 0.0;
    trailElem->item.origin[2] = 0.0;
    trailElem->item.spawnDist = 0.0;
    trailElem->item.msecBegin = 0;
    *(unsigned int *)&trailElem->item.nextTrailElemHandle = 0;
    *(unsigned int *)&trailElem->item.basis[0][0] = 0;
    *(unsigned int *)&trailElem->item.basis[1][1] = 0;
    FX_FreePool_Generic_FxTrailElem_((FxTrailElem *)trailElem, &system->firstFreeTrailElem, system->trailElems);
    FX_DelRefToEffect(system, effect);
    InterlockedDecrement(&system->activeTrailElemCount);
}

void __cdecl FX_FreeSpotLightElem(FxSystem *system, unsigned __int16 elemHandle, FxEffect *effect)
{
    FxPool<FxElem> *v3; // eax
    unsigned __int16 activeSpotLightElemHandle; // [esp+Eh] [ebp-6h]

    if (system->activeSpotLightEffectCount <= 0 || system->activeSpotLightElemCount <= 0)
        MyAssertHandler(
            ".\\EffectsCore\\fx_system.cpp",
            2288,
            0,
            "%s",
            "system->activeSpotLightEffectCount > 0 && system->activeSpotLightElemCount > 0");
    activeSpotLightElemHandle = system->activeSpotLightElemHandle;
    if (!system)
        MyAssertHandler("c:\\trees\\cod3\\src\\effectscore\\fx_system.h", 334, 0, "%s", "system");
    v3 = FX_PoolFromHandle_Generic<FxElem, 2048>(system->elems, activeSpotLightElemHandle);
    v3->nextFree = 0;
    *(unsigned int *)&v3->item.nextElemHandleInEffect = 0;
    v3->item.msecBegin = 0;
    v3->item.baseVel[0] = 0.0;
    v3->item.baseVel[1] = 0.0;
    v3->item.baseVel[2] = 0.0;
    v3->item.physObjId = 0;
    v3->item.origin[1] = 0.0;
    v3->item.origin[2] = 0.0;
    v3->item.u.trailTexCoord = 0.0;
    FX_FreePool_Generic_FxElem_((FxElem *)v3, &system->firstFreeElem, system->elems);
    FX_DelRefToEffect(system, effect);
    InterlockedDecrement(&system->activeElemCount);
    InterlockedDecrement(&system->activeSpotLightElemCount);
}

double __cdecl FX_GetClientVisibility(int localClientNum, const float *start, const float *end)
{
    float v4; // [esp+14h] [ebp-9Ch]
    float v5; // [esp+18h] [ebp-98h]
    float diff[11]; // [esp+38h] [ebp-78h] BYREF
    const FxVisBlocker *visBlocker; // [esp+64h] [ebp-4Ch]
    float totalVis; // [esp+68h] [ebp-48h]
    const FxVisState *visState; // [esp+6Ch] [ebp-44h]
    float dir[3]; // [esp+70h] [ebp-40h] BYREF
    float halfLen; // [esp+7Ch] [ebp-34h]
    int blockerIndex; // [esp+80h] [ebp-30h]
    float len; // [esp+84h] [ebp-2Ch]
    FxSystem *system; // [esp+88h] [ebp-28h]
    float projDir[3]; // [esp+8Ch] [ebp-24h] BYREF
    float projPt[3]; // [esp+98h] [ebp-18h] BYREF
    float dot; // [esp+A4h] [ebp-Ch]
    float distSq; // [esp+A8h] [ebp-8h]
    float blockerRadius; // [esp+ACh] [ebp-4h]

    system = FX_GetSystem(localClientNum);
    visState = system->visStateBufferRead;
    if (!visState || !visState->blockerCount)
        return 1.0;
    //Profile_Begin(212);
    Vec3Sub(end, start, dir);
    len = Vec3Normalize(dir);
    if (fx_visMinTraceDist->current.value <= (double)len)
    {
        halfLen = len * 0.5;
        totalVis = 1.0;
        if (visState->blockerCount > 256)
            MyAssertHandler(
                ".\\EffectsCore\\fx_system.cpp",
                2355,
                0,
                "visState->blockerCount <= FX_VIS_BLOCKER_LIMIT\n\t%i, %i",
                visState->blockerCount,
                256);
        for (blockerIndex = 0; blockerIndex < visState->blockerCount; ++blockerIndex)
        {
            visBlocker = &visState->blocker[blockerIndex];
            Vec3Sub(visBlocker->origin, start, projDir);
            dot = Vec3Dot(projDir, dir);
            v5 = dot - halfLen;
            v4 = fabs(v5);
            if (halfLen >= (double)v4)
            {
                Vec3Mad(start, dot, dir, projPt);
                Vec3Sub(projPt, visBlocker->origin, diff);
                distSq = Vec3LengthSq(diff);
                blockerRadius = (double)visBlocker->radius * 0.0625;
                if (distSq < blockerRadius * blockerRadius)
                    totalVis = (double)visBlocker->visibility * 0.0000152587890625 * totalVis;
            }
        }
        //Profile_EndInternal(0);
        return totalVis;
    }
    else
    {
        //Profile_EndInternal(0);
        return 1.0;
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