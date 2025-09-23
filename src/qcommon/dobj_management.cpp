#include "qcommon.h"
#include "mem_track.h"
#include "threads.h"
#include <xanim/dobj.h>

#define CLIENT_DOBJ_HANDLE_MAX (MAX_GENTITIES + 128)
#define SERVER_DOBJ_HANDLE_MAX (MAX_GENTITIES)

#define DOBJ_HANDLE_MAX 2048

static DObj_s objBuf[DOBJ_HANDLE_MAX];
static bool objAlloced[DOBJ_HANDLE_MAX];
static __int16 clientObjMap[CLIENT_DOBJ_HANDLE_MAX];
static __int16 serverObjMap[SERVER_DOBJ_HANDLE_MAX];
static int objFreeCount;
static int com_lastDObjIndex;

// LWSS: used in SP (KISAKTODO: could MP use this?)
static __int16 clientObjMapBuffered[CLIENT_DOBJ_HANDLE_MAX];
static unsigned __int8 serverObjDirty[272];

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

DObj_s *Com_GetClientDObjBuffered(unsigned int handle, int localClientNum)
{
    unsigned int v4; // r31
    unsigned int v5; // r31

    iassert(handle >= 0 && handle < CLIENT_DOBJ_HANDLE_MAX);

    v4 = 2304 * localClientNum + handle;

    //iassert(((unsigned)handle < (sizeof(clientObjMapBuffered) / (sizeof(clientObjMapBuffered[0]) * (sizeof(clientObjMapBuffered) != 4 || sizeof(clientObjMapBuffered[0]) <= 4))));
    v5 = v4;

    iassert((unsigned)clientObjMapBuffered[handle] < DOBJ_HANDLE_MAX);

    if (clientObjMapBuffered[v5])
        return &objBuf[clientObjMapBuffered[v5]];
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

void Com_ServerDObjClean(int handle)
{
    iassert((unsigned)handle < SERVER_DOBJ_HANDLE_MAX);
    serverObjDirty[handle >> 3] &= ~(1 << (handle & 7));
}

bool Com_ServerDObjDirty(int handle)
{
    iassert((unsigned)handle < SERVER_DOBJ_HANDLE_MAX);
    return ((1 << (handle & 7)) & serverObjDirty[handle >> 3]) != 0;
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

#ifdef KISAK_SP
    serverObjDirty[(int)handle >> 3] |= 1 << (handle & 7);
#endif

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

#ifdef KISAK_SP
        serverObjDirty[(int)handle >> 3] |= 1 << (handle & 7);
#endif

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
    Com_Memset(objAlloced, 0, ARRAY_COUNT(objAlloced) * sizeof(bool));
    objFreeCount = ARRAY_COUNT(objAlloced) - 1;
#ifdef KISAK_SP
    Com_Memset(serverObjDirty, 0, 272);
#endif
    Com_Memset(clientObjMap, 0, ARRAY_COUNT(clientObjMap) * sizeof(__int16));
    Com_Memset(serverObjMap, 0, ARRAY_COUNT(serverObjMap) * sizeof(__int16));
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
        for (int i = 0; i < DOBJ_HANDLE_MAX; ++i)
        {
            iassert(!objAlloced[i]);
        }
        for (int i = 0; i < CLIENT_DOBJ_HANDLE_MAX; ++i)
        {
            iassert(!clientObjMap[i]);
        }
#ifdef KISAK_SP
        for (int i = 0; i < CLIENT_DOBJ_HANDLE_MAX; ++i)
        {
            iassert(!clientObjMapBuffered[i]);
        }
#endif
        for (int i = 0; i < SERVER_DOBJ_HANDLE_MAX; ++i)
        {
            iassert(!serverObjMap[i]);
        }
        iassert((objFreeCount == (sizeof(objAlloced) / (sizeof(objAlloced[0]) * (sizeof(objAlloced) != 4 || sizeof(objAlloced[0]) <= 4))) - 1));
    }
}

DObj_s *Com_DObjCloneToBuffer(unsigned int entnum)
{
    unsigned int v2; // r27
    __int16 serverDobjIndex; // r11
    unsigned int v4; // r26
    unsigned int FreeDObjIndex; // r30

    if (entnum >= 0x880)
        MyAssertHandler(
            "c:\\trees\\cod3\\cod3src\\src\\qcommon\\dobj_management.cpp",
            325,
            0,
            "%s",
            "(unsigned)entnum < ARRAY_COUNT( serverObjMap )");
    v2 = entnum;
    serverDobjIndex = serverObjMap[entnum];
    v4 = serverDobjIndex;
    iassert( serverDobjIndex );
    if (entnum >= 0x900)
        MyAssertHandler(
            "c:\\trees\\cod3\\cod3src\\src\\qcommon\\dobj_management.cpp",
            329,
            0,
            "%s",
            "(unsigned)entnum < ARRAY_COUNT( clientObjMapBuffered )");
    if (clientObjMapBuffered[v2])
        MyAssertHandler(
            "c:\\trees\\cod3\\cod3src\\src\\qcommon\\dobj_management.cpp",
            330,
            0,
            "%s",
            "!clientObjMapBuffered[entnum]");
    FreeDObjIndex = Com_GetFreeDObjIndex();
    if (entnum >= 0x900)
        MyAssertHandler(
            "c:\\trees\\cod3\\cod3src\\src\\qcommon\\dobj_management.cpp",
            334,
            0,
            "entnum doesn't index ARRAY_COUNT( clientObjMapBuffered )\n\t%i not in [0, %i)",
            entnum,
            2304);

    clientObjMapBuffered[v2] = FreeDObjIndex;

    if (v4 >= 0x800)
        MyAssertHandler(
            "c:\\trees\\cod3\\cod3src\\src\\qcommon\\dobj_management.cpp",
            337,
            0,
            "%s\n\t(serverDobjIndex) = %i",
            "((unsigned)serverDobjIndex < 2048)",
            v4);
    if (FreeDObjIndex >= 0x800)
        MyAssertHandler(
            "c:\\trees\\cod3\\cod3src\\src\\qcommon\\dobj_management.cpp",
            338,
            0,
            "%s\n\t(clientDobjIndex) = %i",
            "((unsigned)clientDobjIndex < 2048)",
            FreeDObjIndex);
    DObjClone(&objBuf[v4], &objBuf[FreeDObjIndex]);
    if (!objFreeCount)
        Com_Error(ERR_DROP, "No free DObjs");

    return &objBuf[FreeDObjIndex];
}

void Com_DObjCloneFromBuffer(unsigned int entnum)
{
    unsigned int v2; // r31

    if (entnum >= 0x900)
        MyAssertHandler(
            "c:\\trees\\cod3\\cod3src\\src\\qcommon\\dobj_management.cpp",
            356,
            0,
            "%s",
            "(unsigned)entnum < ARRAY_COUNT( clientObjMap )");
    v2 = entnum;
    if (clientObjMap[entnum])
        MyAssertHandler(
            "c:\\trees\\cod3\\cod3src\\src\\qcommon\\dobj_management.cpp",
            357,
            0,
            "%s\n\t(entnum) = %i",
            "(!clientObjMap[entnum])",
            entnum);

    if (clientObjMapBuffered[v2])
    {
        clientObjMap[v2] = clientObjMapBuffered[v2];
        clientObjMapBuffered[v2] = 0;
    }
}