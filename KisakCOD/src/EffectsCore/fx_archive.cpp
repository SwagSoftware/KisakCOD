#include "fx_system.h"


void __cdecl FX_Restore(int clientIndex, MemoryFile *memFile)
{
    int v2; // [esp+0h] [ebp-201Ch] BYREF
    FxEffectDefTable table; // [esp+4h] [ebp-2018h] BYREF
    int v4; // [esp+200Ch] [ebp-10h]
    int relocationDistance; // [esp+2010h] [ebp-Ch]
    void *p; // [esp+2014h] [ebp-8h]
    FxSystemBuffers *systemBuffers; // [esp+2018h] [ebp-4h]

    p = FX_GetSystem(clientIndex);
    if (!p)
        MyAssertHandler(".\\EffectsCore\\fx_archive.cpp", 220, 0, "%s", "system");
    systemBuffers = FX_GetSystemBuffers(clientIndex);
    if (!systemBuffers)
        MyAssertHandler(".\\EffectsCore\\fx_archive.cpp", 223, 0, "%s", "systemBuffers");
    FX_RestoreEffectDefTable(memFile, &table);
    MemFile_ReadData(memFile, 2656, (unsigned __int8 *)p);
    if (!*((_BYTE *)p + 2526) || *((unsigned int *)p + 627))
        Com_Error(ERR_DROP, "Invalid save file");
    FX_LinkSystemBuffers((FxSystem *)p, systemBuffers);
    MemFile_ReadData(memFile, 291968, (unsigned __int8 *)systemBuffers);
    FX_FixupEffectDefHandles((FxSystem *)p, &table);
    MemFile_ReadData(memFile, 4, (unsigned __int8 *)&v2);
    v4 = v2;
    relocationDistance = (int)p - v2;
    FX_RelocateSystem((FxSystem *)p, (int)p - v2);
    FX_RestorePhysicsData((FxSystem *)p, memFile);
    *((_BYTE *)p + 2526) = 0;
}

void __cdecl FX_RestoreEffectDefTable(MemoryFile *memFile, FxEffectDefTable *table)
{
    unsigned int p; // [esp+0h] [ebp-10h] BYREF
    const FxEffectDef *effectDef; // [esp+4h] [ebp-Ch]
    unsigned int key; // [esp+8h] [ebp-8h]
    const char *effectDefName; // [esp+Ch] [ebp-4h]

    table->count = 0;
    while (1)
    {
        effectDefName = MemFile_ReadCString(memFile);
        if (!*effectDefName)
            break;
        MemFile_ReadData(memFile, 4, (unsigned __int8 *)&p);
        key = p;
        effectDef = FX_Register((char *)effectDefName);
        FX_AddEffectDefTableEntry(table, key, effectDef);
    }
}

void __cdecl FX_AddEffectDefTableEntry(FxEffectDefTable *table, unsigned int key, const FxEffectDef *effectDef)
{
    if (!table)
        MyAssertHandler(".\\EffectsCore\\fx_archive.cpp", 47, 0, "%s", "table");
    if (table->count >= 0x400u)
        MyAssertHandler(
            ".\\EffectsCore\\fx_archive.cpp",
            48,
            0,
            "table->count doesn't index ARRAY_COUNT( table->entries )\n\t%i not in [0, %i)",
            table->count,
            1024);
    if (!effectDef)
        MyAssertHandler(".\\EffectsCore\\fx_archive.cpp", 49, 0, "%s", "effectDef");
    table->entries[table->count].key = key;
    table->entries[table->count++].effectDef = effectDef;
}

void __cdecl FX_FixupEffectDefHandles(FxSystem *system, FxEffectDefTable *table)
{
    const FxEffectDef *effectDef; // [esp+Ch] [ebp-10h]
    FxEffect *effect; // [esp+10h] [ebp-Ch]
    volatile int activeIndex; // [esp+18h] [ebp-4h]

    if (!system)
        MyAssertHandler(".\\EffectsCore\\fx_archive.cpp", 131, 0, "%s", "system");
    if (!system->isArchiving)
        MyAssertHandler(".\\EffectsCore\\fx_archive.cpp", 132, 0, "%s", "system->isArchiving");
    for (activeIndex = system->firstActiveEffect; activeIndex != system->firstNewEffect; ++activeIndex)
    {
        effect = FX_EffectFromHandle(system, system->allEffectHandles[activeIndex & 0x3FF]);
        effectDef = FX_FindEffectDefInTable(table, (unsigned int)effect->def);
        if (!effectDef)
            MyAssertHandler(".\\EffectsCore\\fx_archive.cpp", 139, 0, "%s", "effectDef");
        effect->def = effectDef;
    }
}

FxEffect *__cdecl FX_EffectFromHandle(FxSystem *system, unsigned __int16 handle)
{
    const char *v2; // eax

    if (!system)
        MyAssertHandler("c:\\trees\\cod3\\src\\effectscore\\fx_system.h", 256, 0, "%s", "system");
    if (handle >= 0x8000u || handle % 0x20u)
    {
        v2 = va("%p %i", system->effects, handle);
        MyAssertHandler(
            "c:\\trees\\cod3\\src\\effectscore\\fx_system.h",
            257,
            0,
            "%s\n\t%s",
            "handle < FX_EFFECT_LIMIT * sizeof( FxEffect ) / FxEffect::HANDLE_SCALE && handle % (sizeof( FxEffect ) / FxEffect:"
            ":HANDLE_SCALE) == 0",
            v2);
    }
    return (FxEffect *)((char *)system->effects + 4 * handle);
}

const FxEffectDef *__cdecl FX_FindEffectDefInTable(const FxEffectDefTable *table, unsigned int key)
{
    int index; // [esp+0h] [ebp-4h]

    for (index = 0; index < table->count; ++index)
    {
        if (table->entries[index].key == key)
            return table->entries[index].effectDef;
    }
    return 0;
}

void __cdecl FX_RestorePhysicsData(FxSystem *system, MemoryFile *memFile)
{
    const XModel *visuals; // [esp+18h] [ebp-20h]
    unsigned __int16 elemHandle; // [esp+1Ch] [ebp-1Ch]
    const FxElemDef *elemDef; // [esp+20h] [ebp-18h]
    const FxEffect *effect; // [esp+24h] [ebp-14h]
    unsigned __int16 elemHandleNext; // [esp+2Ch] [ebp-Ch]
    FxPool<FxElem> *elem; // [esp+30h] [ebp-8h]
    volatile int activeIndex; // [esp+34h] [ebp-4h]

    if (!system)
        MyAssertHandler(".\\EffectsCore\\fx_archive.cpp", 185, 0, "%s", "system");
    if (!system->isArchiving)
        MyAssertHandler(".\\EffectsCore\\fx_archive.cpp", 186, 0, "%s", "system->isArchiving");
    for (activeIndex = system->firstActiveEffect; activeIndex != system->firstNewEffect; ++activeIndex)
    {
        effect = FX_EffectFromHandle(system, system->allEffectHandles[activeIndex & 0x3FF]);
        for (elemHandle = effect->firstElemHandle[1]; elemHandle != 0xFFFF; elemHandle = elemHandleNext)
        {
            if (!system)
                MyAssertHandler("c:\\trees\\cod3\\src\\effectscore\\fx_system.h", 334, 0, "%s", "system");
            elem = FX_PoolFromHandle_Generic<FxElem, 2048>(system->elems, elemHandle);
            elemDef = &effect->def->elemDefs[elem->item.defIndex];
            elemHandleNext = elem->item.nextElemHandleInEffect;
            if (elemDef->elemType == 5 && ((unsigned int)svs.snapshotClients[20582].attachModelIndex & elemDef->flags) != 0)
            {
                elem->item.physObjId = (int)Phys_ObjLoad(PHYS_WORLD_FX, memFile);
                visuals = FX_GetElemVisuals(
                    elemDef,
                    (296 * elem->item.sequence + elem->item.msecBegin + (unsigned int)effect->randomSeed) % 0x1DF).model;
                Phys_ObjSetCollisionFromXModel(visuals, PHYS_WORLD_FX, (dxBody *)elem->item.physObjId);
            }
        }
    }
}

FxElemVisuals __cdecl FX_GetElemVisuals(const FxElemDef *elemDef, int randomSeed)
{
    if (!elemDef->visualCount)
        MyAssertHandler(
            "c:\\trees\\cod3\\src\\effectscore\\fx_draw.h",
            79,
            0,
            "%s\n\t(elemDef->visualCount) = %i",
            "(elemDef->visualCount > 0)",
            elemDef->visualCount);
    if (elemDef->visualCount == 1)
        return elemDef->visuals.instance;
    else
        return (FxElemVisuals)elemDef->visuals.markArray->materials[(elemDef->visualCount
            * LOWORD(fx_randomTable[randomSeed + 21])) >> 16];
}

void __cdecl FX_Save(int clientIndex, MemoryFile *memFile)
{
    unsigned int UsedSize; // eax
    unsigned int v3; // eax
    FxSystem *p; // [esp+0h] [ebp-Ch] BYREF
    FxSystem *system; // [esp+4h] [ebp-8h]
    FxSystemBuffers *systemBuffers; // [esp+8h] [ebp-4h]

    system = FX_GetSystem(clientIndex);
    if (!system)
        MyAssertHandler(".\\EffectsCore\\fx_archive.cpp", 265, 0, "%s", "system");
    systemBuffers = FX_GetSystemBuffers(clientIndex);
    if (!systemBuffers)
        MyAssertHandler(".\\EffectsCore\\fx_archive.cpp", 267, 0, "%s", "systemBuffers");
    if (system->isArchiving)
        MyAssertHandler(".\\EffectsCore\\fx_archive.cpp", 270, 0, "%s", "!system->isArchiving");
    system->isArchiving = 1;
    FX_SaveEffectDefTable(system, memFile);
    MemFile_WriteData(memFile, 2656, system);
    UsedSize = MemFile_GetUsedSize(memFile);
    ProfMem_Begin("systemBuffers", UsedSize);
    MemFile_WriteData(memFile, 291968, systemBuffers);
    v3 = MemFile_GetUsedSize(memFile);
    ProfMem_End(v3);
    p = system;
    MemFile_WriteData(memFile, 4, &p);
    FX_SavePhysicsData(system, memFile);
    system->isArchiving = 0;
}

void __cdecl FX_SaveEffectDefTable(FxSystem *system, MemoryFile *memFile)
{
    if (useFastFile->current.enabled)
        FX_SaveEffectDefTable_FastFile(memFile);
    else
        FX_SaveEffectDefTable_LoadObj(memFile);
    MemFile_WriteCString(memFile, (char *)&String);
}

void __cdecl FX_SaveEffectDefTable_FastFile(MemoryFile *memFile)
{
    DB_EnumXAssets(
        ASSET_TYPE_FX,
        (void(__cdecl *)(XAssetHeader, void *))FX_SaveEffectDefTableEntry_FileLoadObj,
        memFile,
        0);
}

void __cdecl FX_SavePhysicsData(FxSystem *system, MemoryFile *memFile)
{
    unsigned __int16 elemHandle; // [esp+Ch] [ebp-18h]
    const FxElemDef *elemDef; // [esp+10h] [ebp-14h]
    const FxEffect *effect; // [esp+14h] [ebp-10h]
    unsigned __int16 elemHandleNext; // [esp+18h] [ebp-Ch]
    FxPool<FxElem> *elem; // [esp+1Ch] [ebp-8h]
    volatile int activeIndex; // [esp+20h] [ebp-4h]

    if (!system)
        MyAssertHandler(".\\EffectsCore\\fx_archive.cpp", 155, 0, "%s", "system");
    if (!system->isArchiving)
        MyAssertHandler(".\\EffectsCore\\fx_archive.cpp", 156, 0, "%s", "system->isArchiving");
    for (activeIndex = system->firstActiveEffect; activeIndex != system->firstNewEffect; ++activeIndex)
    {
        effect = FX_EffectFromHandle(system, system->allEffectHandles[activeIndex & 0x3FF]);
        for (elemHandle = effect->firstElemHandle[1]; elemHandle != 0xFFFF; elemHandle = elemHandleNext)
        {
            if (!system)
                MyAssertHandler("c:\\trees\\cod3\\src\\effectscore\\fx_system.h", 334, 0, "%s", "system");
            elem = FX_PoolFromHandle_Generic<FxElem, 2048>(system->elems, elemHandle);
            elemDef = &effect->def->elemDefs[elem->item.defIndex];
            elemHandleNext = elem->item.nextElemHandleInEffect;
            if (elemDef->elemType == 5 && (elemDef->flags & 0x8000000) != 0)
                Phys_ObjSave((dxBody *)elem->item.physObjId, memFile);
        }
    }
}

void __cdecl FX_Archive(int clientIndex, MemoryFile *memFile)
{
    if (MemFile_IsWriting(memFile))
        FX_Save(clientIndex, memFile);
    else
        FX_Restore(clientIndex, memFile);
}

