#include "enthandle.h"

#include <game_mp/g_main_mp.h>

#include <cgame_mp/cg_local_mp.h>

static EntHandleList g_entitiesHandleList[0x400];
static EntHandleInfo g_entHandleInfoArray[0x1000];

static int g_usedEntHandle;
static int g_entHandleInfoHead;
static int g_maxUsedEntHandle;

//Line 53042:  0006 : 0050c928       unsigned int g_maxUsedEntHandle   82cbc928     enthandle.obj

void __cdecl EntHandleDissociate(gentity_s *ent)
{
    EntHandleDissociateInternal(&g_entitiesHandleList[ent - g_entities]);
}

void __cdecl EntHandleDissociateInternal(EntHandleList *entHandleList)
{
    const char *v1; // eax
    EntHandleInfo *info; // [esp+0h] [ebp-Ch]
    unsigned int infoIndexHead; // [esp+4h] [ebp-8h]
    unsigned int infoIndex; // [esp+8h] [ebp-4h]

    infoIndexHead = entHandleList->infoIndex;
    if (entHandleList->infoIndex)
    {
        entHandleList->infoIndex = 0;
        infoIndex = infoIndexHead;
        do
        {
            info = &g_entHandleInfoArray[infoIndex];
            if (!info->handle)
            {
                v1 = va("%d %p", infoIndex, info);
                MyAssertHandler(".\\game\\enthandle.cpp", 137, 0, "%s\n\t%s", "info->handle", v1);
            }
            *(_DWORD *)info->handle = 0;
            info->handle = 0;
            if (!g_usedEntHandle)
                MyAssertHandler(".\\game\\enthandle.cpp", 142, 0, "%s", "g_usedEntHandle");
            --g_usedEntHandle;
            infoIndex = info->next;
        } while (infoIndex != infoIndexHead);
        g_entHandleInfoArray[g_entHandleInfoArray[infoIndexHead].prev].next = g_entHandleInfoHead;
        g_entHandleInfoHead = infoIndexHead;
    }
}

void __cdecl RemoveEntHandleInfo(EntHandleList *entHandleList, unsigned int oldInfoIndex)
{
    const char *v2; // eax
    EntHandleInfo *info; // [esp+0h] [ebp-10h]
    EntHandleInfo *prev; // [esp+Ch] [ebp-4h]

    if (oldInfoIndex)
    {
        if (entHandleList->infoIndex == oldInfoIndex)
            entHandleList->infoIndex = oldInfoIndex != g_entHandleInfoArray[oldInfoIndex].next
            ? g_entHandleInfoArray[oldInfoIndex].next
            : 0;
        info = &g_entHandleInfoArray[oldInfoIndex];
        if (!info->handle)
        {
            v2 = va("%d %p", oldInfoIndex, info);
            MyAssertHandler(".\\game\\enthandle.cpp", 194, 0, "%s\n\t%s", "info->handle", v2);
        }
        info->handle = 0;
        if (!g_usedEntHandle)
            MyAssertHandler(".\\game\\enthandle.cpp", 196, 0, "%s", "g_usedEntHandle");
        --g_usedEntHandle;
        prev = &g_entHandleInfoArray[info->prev];
        g_entHandleInfoArray[info->next].prev = info->prev;
        prev->next = info->next;
        info->next = g_entHandleInfoHead;
        g_entHandleInfoHead = oldInfoIndex;
    }
}

unsigned int __cdecl AddEntHandleInfo(EntHandleList *entHandleList, void *handle)
{
    unsigned int v3; // [esp+0h] [ebp-14h]
    EntHandleInfo *info; // [esp+4h] [ebp-10h]
    unsigned int infoIndexHead; // [esp+8h] [ebp-Ch]
    unsigned int infoIndex; // [esp+Ch] [ebp-8h]
    EntHandleInfo *infoHead; // [esp+10h] [ebp-4h]

    infoIndex = g_entHandleInfoHead;
    if (!g_entHandleInfoHead)
        Com_Error(ERR_DROP, "ENT_HANDLE_COUNT exceeded - increase size");
    info = &g_entHandleInfoArray[infoIndex];
    g_entHandleInfoHead = info->next;
    if (g_maxUsedEntHandle < ++g_usedEntHandle)
        v3 = g_usedEntHandle;
    else
        v3 = g_maxUsedEntHandle;
    g_maxUsedEntHandle = v3;
    info->handle = handle;
    infoIndexHead = entHandleList->infoIndex;
    if (entHandleList->infoIndex)
    {
        infoHead = &g_entHandleInfoArray[infoIndexHead];
        info->next = infoHead->next;
        info->prev = infoIndexHead;
        g_entHandleInfoArray[infoHead->next].prev = infoIndex;
        infoHead->next = infoIndex;
    }
    else
    {
        info->next = infoIndex;
        info->prev = infoIndex;
        entHandleList->infoIndex = infoIndex;
    }
    return infoIndex;
}


void EntHandle::setEnt(EntHandle *_this, gentity_s *ent)
{
    gentity_s *oldEnt; // [esp+4h] [ebp-8h]

    if (EntHandle::isDefined(_this))
    {
        oldEnt = EntHandle::ent(_this);
        if (ent == oldEnt)
            return;
        RemoveEntHandleInfo(&g_entitiesHandleList[oldEnt - g_entities], _this->infoIndex);
        if (!ent)
        {
            _this->number = 0;
            _this->infoIndex = 0;
            return;
        }
    }
    else if (!ent)
    {
        return;
    }
    _this->infoIndex = AddEntHandleInfo(&g_entitiesHandleList[ent - g_entities], _this);
    _this->number = ent - g_entities + 1;
}

int EntHandle::entnum(EntHandle *_this)
{
    if ((unsigned int)_this->number - 1 >= 0x3FF)
        MyAssertHandler(
            "c:\\trees\\cod3\\src\\game_mp\\g_public_mp.h",
            295,
            0,
            "number - 1 doesn't index ENTITYNUM_NONE\n\t%i not in [0, %i)",
            _this->number - 1,
            1023);
    if (!g_entities[_this->number - 1].r.inuse)
        MyAssertHandler(
            "c:\\trees\\cod3\\src\\game_mp\\g_public_mp.h",
            296,
            0,
            "%s\n\t(number - 1) = %i",
            "(g_entities[number - 1].r.inuse)",
            _this->number - 1);
    return _this->number - 1;
}

bool EntHandle::isDefined(EntHandle *_this)
{
    if (_this->number && !g_entities[_this->number - 1].r.inuse)
        MyAssertHandler(
            "c:\\trees\\cod3\\src\\game_mp\\g_public_mp.h",
            280,
            0,
            "%s\n\t(number - 1) = %i",
            "(!number || g_entities[number - 1].r.inuse)",
            _this->number - 1);
    return _this->number != 0;
}

gentity_s *EntHandle::ent(EntHandle *_this)
{
    if ((unsigned int)_this->number - 1 >= 0x3FF)
        MyAssertHandler(
            "c:\\trees\\cod3\\src\\game_mp\\g_public_mp.h",
            287,
            0,
            "number - 1 doesn't index ENTITYNUM_NONE\n\t%i not in [0, %i)",
            _this->number - 1,
            1023);
    
    if (!g_entities[_this->number - 1].r.inuse)
        MyAssertHandler(
            "c:\\trees\\cod3\\src\\game_mp\\g_public_mp.h",
            288,
            0,
            "%s\n\t(number - 1) = %i",
            "(g_entities[number - 1].r.inuse)",
            _this->number - 1);

    return &g_entities[_this->number - 1];
}

void EntHandle::Shutdown()
{
    const char *v0; // eax
    unsigned int usedEntHandle; // [esp+0h] [ebp-Ch]
    unsigned int i; // [esp+8h] [ebp-4h]

    if (g_usedEntHandle)
    {
        Com_Printf(1, "EntHandle BEGIN\n");
        usedEntHandle = 0;
        for (i = 1; i < 0x1000; ++i)
        {
            if (g_entHandleInfoArray[i].handle)
            {
                Com_Printf(1, "%p\n", &g_entHandleInfoArray[i]);
                ++usedEntHandle;
            }
        }
        Com_Printf(1, "EntHandle END\n");
        if (usedEntHandle != g_usedEntHandle)
        {
            v0 = va("%d %d", usedEntHandle, g_usedEntHandle);
            MyAssertHandler(".\\game\\enthandle.cpp", 103, 0, "%s\n\t%s", "usedEntHandle == g_usedEntHandle", v0);
        }
        if (g_usedEntHandle)
            MyAssertHandler(
                ".\\game\\enthandle.cpp",
                104,
                0,
                "%s\n\t(g_usedEntHandle) = %i",
                "(!g_usedEntHandle)",
                g_usedEntHandle);
    }
}

void __cdecl EntHandle::Init()
{
    unsigned int i; // [esp+4h] [ebp-4h]

    for (i = 1; i < 0x1000; ++i)
    {
        g_entHandleInfoArray[i].next = i + 1;
        g_entHandleInfoArray[i].handle = 0;
    }
    g_entHandleInfoArray[4095].next = 0;
    g_entHandleInfoHead = 1;
    memset((unsigned __int8 *)g_entitiesHandleList, 0, sizeof(g_entitiesHandleList));
    g_usedEntHandle = 0;
}