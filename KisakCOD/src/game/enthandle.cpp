#include "enthandle.h"


//Line 53042:  0006 : 0050c928       unsigned int g_maxUsedEntHandle   82cbc928     enthandle.obj

void __cdecl EntHandleDissociate(gentity_s *ent)
{
    EntHandleDissociateInternal((EntHandleList *)(2 * (ent - g_entities) + 21409160));
}

void __cdecl EntHandleDissociateInternal(EntHandleList *entHandleList)
{
    const char *v1; // eax
    unsigned int infoIndexHead; // [esp+4h] [ebp-8h]
    unsigned int infoIndex; // [esp+8h] [ebp-4h]

    infoIndexHead = entHandleList->infoIndex;
    if (entHandleList->infoIndex)
    {
        entHandleList->infoIndex = 0;
        infoIndex = infoIndexHead;
        do
        {
            if (!g_entHandleInfoArray[infoIndex].handle)
            {
                v1 = va("%d %p", infoIndex, (const void *)(8 * infoIndex + 21411224));
                MyAssertHandler(".\\game\\enthandle.cpp", 137, 0, "%s\n\t%s", "info->handle", v1);
            }
            *(unsigned int *)g_entHandleInfoArray[infoIndex].handle = 0;
            g_entHandleInfoArray[infoIndex].handle = 0;
            if (!g_usedEntHandle)
                MyAssertHandler(".\\game\\enthandle.cpp", 142, 0, "%s", "g_usedEntHandle");
            --g_usedEntHandle;
            infoIndex = g_entHandleInfoArray[infoIndex].next;
        } while (infoIndex != infoIndexHead);
        g_entHandleInfoArray[g_entHandleInfoArray[infoIndexHead].prev].next = g_entHandleInfoHead;
        g_entHandleInfoHead = infoIndexHead;
    }
}

void __cdecl RemoveEntHandleInfo(EntHandleList *entHandleList, unsigned int oldInfoIndex)
{
    const char *v2; // eax
    EntHandleInfo *prev; // [esp+Ch] [ebp-4h]

    if (oldInfoIndex)
    {
        if (entHandleList->infoIndex == oldInfoIndex)
            entHandleList->infoIndex = oldInfoIndex != g_entHandleInfoArray[oldInfoIndex].next
            ? g_entHandleInfoArray[oldInfoIndex].next
            : 0;
        if (!g_entHandleInfoArray[oldInfoIndex].handle)
        {
            v2 = va("%d %p", oldInfoIndex, (const void *)(8 * oldInfoIndex + 21411224));
            MyAssertHandler(".\\game\\enthandle.cpp", 194, 0, "%s\n\t%s", "info->handle", v2);
        }
        g_entHandleInfoArray[oldInfoIndex].handle = 0;
        if (!g_usedEntHandle)
            MyAssertHandler(".\\game\\enthandle.cpp", 196, 0, "%s", "g_usedEntHandle");
        --g_usedEntHandle;
        prev = (EntHandleInfo *)(8 * g_entHandleInfoArray[oldInfoIndex].prev + 21411224);
        g_entHandleInfoArray[g_entHandleInfoArray[oldInfoIndex].next].prev = g_entHandleInfoArray[oldInfoIndex].prev;
        prev->next = g_entHandleInfoArray[oldInfoIndex].next;
        g_entHandleInfoArray[oldInfoIndex].next = g_entHandleInfoHead;
        g_entHandleInfoHead = oldInfoIndex;
    }
}

unsigned int __cdecl AddEntHandleInfo(EntHandleList *entHandleList, void *handle)
{
    unsigned int v3; // [esp+0h] [ebp-14h]
    unsigned int infoIndexHead; // [esp+8h] [ebp-Ch]
    unsigned int infoIndex; // [esp+Ch] [ebp-8h]
    EntHandleInfo *infoHead; // [esp+10h] [ebp-4h]

    infoIndex = g_entHandleInfoHead;
    if (!g_entHandleInfoHead)
        Com_Error(ERR_DROP, "ENT_HANDLE_COUNT exceeded - increase size");
    g_entHandleInfoHead = g_entHandleInfoArray[infoIndex].next;
    if ((int)g_maxUsedEntHandle < (int)++g_usedEntHandle)
        v3 = g_usedEntHandle;
    else
        v3 = g_maxUsedEntHandle;
    g_maxUsedEntHandle = v3;
    g_entHandleInfoArray[infoIndex].handle = handle;
    infoIndexHead = entHandleList->infoIndex;
    if (entHandleList->infoIndex)
    {
        infoHead = (EntHandleInfo *)(8 * infoIndexHead + 21411224);
        g_entHandleInfoArray[infoIndex].next = g_entHandleInfoArray[infoIndexHead].next;
        g_entHandleInfoArray[infoIndex].prev = infoIndexHead;
        g_entHandleInfoArray[infoHead->next].prev = infoIndex;
        infoHead->next = infoIndex;
    }
    else
    {
        g_entHandleInfoArray[infoIndex].next = infoIndex;
        g_entHandleInfoArray[infoIndex].prev = infoIndex;
        entHandleList->infoIndex = infoIndex;
    }
    return infoIndex;
}

