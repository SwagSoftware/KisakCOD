#include "qcommon.h"
#include "mem_track.h"
#include "threads.h"
#include <xanim/dobj.h>

#define CLIENT_DOBJ_HANDLE_MAX 1152 // 0x480
#define SERVER_DOBJ_HANDLE_MAX 1024 // 0x400
#define DOBJ_HANDLE_MAX 2048

static DObj_s objBuf[DOBJ_HANDLE_MAX];
static bool objAlloced[DOBJ_HANDLE_MAX];
static __int16 clientObjMap[CLIENT_DOBJ_HANDLE_MAX];
static __int16 serverObjMap[SERVER_DOBJ_HANDLE_MAX];
static int objFreeCount;
static int com_lastDObjIndex;

void __cdecl TRACK_dobj_management()
{
    track_static_alloc_internal(objBuf, 204800, "objBuf", 11);
    track_static_alloc_internal(objAlloced, 2048, "objAlloced", 11);
    track_static_alloc_internal(clientObjMap, 2304, "clientObjMap", 11);
    track_static_alloc_internal(serverObjMap, 2048, "serverObjMap", 11);
}

DObj_s *__cdecl Com_GetClientDObj(unsigned int handle, int localClientNum)
{
    iassert(handle >= 0 && handle < CLIENT_DOBJ_HANDLE_MAX);
    iassert(localClientNum == 0);

    handle = handle + CLIENT_DOBJ_HANDLE_MAX * localClientNum;

    iassert(((unsigned)handle < (sizeof(clientObjMap) / (sizeof(clientObjMap[0]) * (sizeof(clientObjMap) != 4 || sizeof(clientObjMap[0]) <= 4)))));
    iassert(((unsigned)clientObjMap[handle] < DOBJ_HANDLE_MAX));

    if (clientObjMap[handle])
        return &objBuf[clientObjMap[handle]];
    else
        return 0;
}

DObj_s *__cdecl Com_GetServerDObj(unsigned int handle)
{
    iassert(((unsigned)handle < (sizeof(serverObjMap) / (sizeof(serverObjMap[0]) * (sizeof(serverObjMap) != 4 || sizeof(serverObjMap[0]) <= 4)))));
    iassert((unsigned)serverObjMap[handle] < DOBJ_HANDLE_MAX);

    if (serverObjMap[handle])
        return &objBuf[serverObjMap[handle]];
    else
        return 0;
}

DObj_s *__cdecl Com_ClientDObjCreate(
    DObjModel_s *dobjModels,
    unsigned __int16 numModels,
    XAnimTree_s *tree,
    unsigned int handle,
    int localClientNum)
{
    unsigned int index; // [esp+0h] [ebp-4h]

    iassert(dobjModels);
    iassert(((unsigned)handle < (((1 << 10)) + 128)));
    iassert(!Com_GetClientDObj(handle, localClientNum));
    index = Com_GetFreeDObjIndex();
    iassert((unsigned)handle < CLIENT_DOBJ_HANDLE_MAX);
    iassert(handle >= localClientNum * CLIENT_DOBJ_HANDLE_MAX);
    iassert(handle < localClientNum * CLIENT_DOBJ_HANDLE_MAX + CLIENT_DOBJ_HANDLE_MAX);

    clientObjMap[handle] = index;

    iassert((unsigned)index < DOBJ_HANDLE_MAX);

    DObjCreate(dobjModels, numModels, tree, &objBuf[index], 0);

    if (!objFreeCount)
        Com_Error(ERR_DROP, "No free DObjs");

    return &objBuf[index];
}

int __cdecl Com_GetFreeDObjIndex()
{
    iassert(Sys_IsMainThread());

    for (int i = com_lastDObjIndex + 1; i < 2048; ++i)
    {
        if (!objAlloced[i])
        {
            com_lastDObjIndex = i;

            iassert(i);
            iassert((unsigned)i < DOBJ_HANDLE_MAX);

            objAlloced[i] = 1;

            iassert(objFreeCount);

            --objFreeCount;
            return i;
        }
    }
    for (int i = 1; i <= com_lastDObjIndex; ++i)
    {
        if (!objAlloced[i])
        {
            com_lastDObjIndex = i;
            iassert(i);
            iassert((unsigned)i < DOBJ_HANDLE_MAX);

            objAlloced[i] = 1;

            iassert(objFreeCount);

            --objFreeCount;
            return i;
        }
    }

    if (!alwaysfails)
        MyAssertHandler(".\\qcommon\\dobj_management.cpp", 208, 0, "unreachable");
    return 0;
}

void __cdecl Com_ClientDObjClearAllSkel()
{
    int handleOffset; // [esp+4h] [ebp-4h]

    for (handleOffset = 0; handleOffset < CLIENT_DOBJ_HANDLE_MAX; ++handleOffset)
    {
        if (clientObjMap[handleOffset])
            DObjSkelClear(&objBuf[clientObjMap[handleOffset]]);
    }
}

DObj_s *__cdecl Com_ServerDObjCreate(
    DObjModel_s *dobjModels,
    unsigned __int16 numModels,
    XAnimTree_s *tree,
    unsigned int handle)
{
    unsigned int index; // [esp+0h] [ebp-4h]

    iassert(dobjModels);
    iassert(handle < SERVER_DOBJ_HANDLE_MAX);
    iassert(!Com_GetServerDObj(handle));

    index = Com_GetFreeDObjIndex();

    iassert((unsigned)handle < SERVER_DOBJ_HANDLE_MAX);

    serverObjMap[handle] = index;

    iassert((unsigned)index < DOBJ_HANDLE_MAX);

    DObjCreate(dobjModels, numModels, tree, &objBuf[index], handle + 1);

    if (!objFreeCount)
        Com_Error(ERR_DROP, "No free DObjs");

    return &objBuf[index];
}

void __cdecl Com_SafeClientDObjFree(unsigned int handle, int localClientNum)
{
    unsigned int index; // [esp+0h] [ebp-4h]

    iassert(handle < CLIENT_DOBJ_HANDLE_MAX);

    iassert((handle - localClientNum * CLIENT_DOBJ_HANDLE_MAX) < CLIENT_DOBJ_HANDLE_MAX);

    index = clientObjMap[handle];

    if (clientObjMap[handle])
    {
        clientObjMap[handle] = 0;

        iassert((unsigned)index < DOBJ_HANDLE_MAX);
        iassert(Sys_IsMainThread());

        objAlloced[index] = 0;
        ++objFreeCount;
        DObjFree(&objBuf[index]);
    }
}

void __cdecl Com_SafeServerDObjFree(unsigned int handle)
{
    unsigned int index; // [esp+0h] [ebp-4h]

    iassert(handle < SERVER_DOBJ_HANDLE_MAX);

    index = serverObjMap[handle];

    if (serverObjMap[handle])
    {
        serverObjMap[handle] = 0;

        iassert((unsigned)index < DOBJ_HANDLE_MAX);
        iassert(Sys_IsMainThread());

        objAlloced[index] = 0;
        ++objFreeCount;
        DObjFree(&objBuf[index]);
    }
}

static int g_bDObjInited;
void __cdecl Com_InitDObj()
{
    Com_Memset(objAlloced, 0, 2048);
    objFreeCount = 2047;
    Com_Memset(clientObjMap, 0, 2304);
    Com_Memset(serverObjMap, 0, 2048);
    com_lastDObjIndex = 1;
    g_bDObjInited = 1;
}

void __cdecl Com_ShutdownDObj()
{
    const char *v0; // eax
    const char *v1; // eax

    if (g_bDObjInited)
    {
        g_bDObjInited = 0;
        for (int i = 0; i < 0x800; ++i)
        {
            iassert(!objAlloced[i]);
        }
        for (int i = 0; i < 0x480; ++i)
        {
            iassert(!clientObjMap[i]);
        }
        for (int i = 0; i < 0x400; ++i)
        {
            iassert(!serverObjMap[i]);
        }
        iassert((objFreeCount == (sizeof(objAlloced) / (sizeof(objAlloced[0]) * (sizeof(objAlloced) != 4 || sizeof(objAlloced[0]) <= 4))) - 1));
    }
}
