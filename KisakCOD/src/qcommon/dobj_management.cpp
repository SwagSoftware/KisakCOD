#include "qcommon.h"
#include "mem_track.h"
#include "threads.h"

char objBuf[2048][100];
bool objAlloced[2048];
__int16 clientObjMap[1152];
__int16 serverObjMap[1024];
int objFreeCount;
int com_lastDObjIndex;

void __cdecl TRACK_dobj_management()
{
    track_static_alloc_internal(objBuf, 204800, "objBuf", 11);
    track_static_alloc_internal(objAlloced, 2048, "objAlloced", 11);
    track_static_alloc_internal(clientObjMap, 2304, "clientObjMap", 11);
    track_static_alloc_internal(serverObjMap, 2048, "serverObjMap", 11);
}

DObj_s *__cdecl Com_GetClientDObj(unsigned int handle, int localClientNum)
{
    unsigned int handlea; // [esp+Ch] [ebp+8h]

    if (handle >= 0x480)
        MyAssertHandler(
            ".\\qcommon\\dobj_management.cpp",
            80,
            0,
            "%s\n\t(handle) = %i",
            "(handle >= 0 && handle < (((1<<10)) + 128))",
            handle);
    if (localClientNum)
        MyAssertHandler(
            ".\\qcommon\\dobj_management.cpp",
            81,
            0,
            "localClientNum doesn't index MAX_LOCAL_CLIENTS\n\t%i not in [0, %i)",
            localClientNum,
            1);
    handlea = handle + 1152 * localClientNum;
    if (handlea >= 0x480)
        MyAssertHandler(
            ".\\qcommon\\dobj_management.cpp",
            85,
            0,
            "%s\n\t(handle) = %i",
            "((unsigned)handle < (sizeof( clientObjMap ) / (sizeof( clientObjMap[0] ) * (sizeof( clientObjMap ) != 4 || sizeof("
            " clientObjMap[0] ) <= 4))))",
            handlea);
    if ((unsigned int)clientObjMap[handlea] >= 0x800)
        MyAssertHandler(
            ".\\qcommon\\dobj_management.cpp",
            86,
            0,
            "%s\n\t(clientObjMap[handle]) = %i",
            "((unsigned)clientObjMap[handle] < 2048)",
            clientObjMap[handlea]);
    if (clientObjMap[handlea])
        return (DObj_s *)objBuf[clientObjMap[handlea]];
    else
        return 0;
}

DObj_s *__cdecl Com_GetServerDObj(unsigned int handle)
{
    if (handle >= 0x400)
        MyAssertHandler(
            ".\\qcommon\\dobj_management.cpp",
            99,
            0,
            "%s\n\t(handle) = %i",
            "((unsigned)handle < (sizeof( serverObjMap ) / (sizeof( serverObjMap[0] ) * (sizeof( serverObjMap ) != 4 || sizeof("
            " serverObjMap[0] ) <= 4))))",
            handle);
    if ((unsigned int)serverObjMap[handle] >= 0x800)
        MyAssertHandler(".\\qcommon\\dobj_management.cpp", 100, 0, "%s", "(unsigned)serverObjMap[handle] < DOBJ_HANDLE_MAX");
    if (serverObjMap[handle])
        return (DObj_s *)objBuf[serverObjMap[handle]];
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

    if (!dobjModels)
        MyAssertHandler(".\\qcommon\\dobj_management.cpp", 223, 0, "%s", "dobjModels");
    if (handle >= 0x480)
        MyAssertHandler(
            ".\\qcommon\\dobj_management.cpp",
            224,
            0,
            "%s\n\t(handle) = %i",
            "((unsigned)handle < (((1<<10)) + 128))",
            handle);
    if (Com_GetClientDObj(handle, localClientNum))
        MyAssertHandler(".\\qcommon\\dobj_management.cpp", 225, 0, "%s", "!Com_GetClientDObj( handle, localClientNum )");
    index = Com_GetFreeDObjIndex();
    if (handle >= 0x480)
        MyAssertHandler(".\\qcommon\\dobj_management.cpp", 232, 0, "%s", "(unsigned)handle < ARRAY_COUNT( clientObjMap )");
    if ((int)handle < 1152 * localClientNum)
        MyAssertHandler(
            ".\\qcommon\\dobj_management.cpp",
            234,
            0,
            "%s",
            "handle >= localClientNum * CLIENT_DOBJ_HANDLE_MAX");
    if ((int)handle >= 1152 * localClientNum + 1152)
        MyAssertHandler(
            ".\\qcommon\\dobj_management.cpp",
            235,
            0,
            "%s",
            "handle < localClientNum * CLIENT_DOBJ_HANDLE_MAX + CLIENT_DOBJ_HANDLE_MAX");
    clientObjMap[handle] = index;
    if (index >= 0x800)
        MyAssertHandler(".\\qcommon\\dobj_management.cpp", 238, 0, "%s", "(unsigned)index < DOBJ_HANDLE_MAX");
    DObjCreate(dobjModels, numModels, tree, objBuf[index], 0);
    if (!objFreeCount)
        Com_Error(ERR_DROP, "No free DObjs");
    return (DObj_s *)objBuf[index];
}

int __cdecl Com_GetFreeDObjIndex()
{
    int i; // [esp+0h] [ebp-4h]
    int ia; // [esp+0h] [ebp-4h]

    if (!Sys_IsMainThread())
        MyAssertHandler(".\\qcommon\\dobj_management.cpp", 168, 0, "%s", "Sys_IsMainThread()");
    for (i = com_lastDObjIndex + 1; i < 2048; ++i)
    {
        if (!objAlloced[i])
        {
            com_lastDObjIndex = i;
            if (!i)
                MyAssertHandler(".\\qcommon\\dobj_management.cpp", 177, 0, "%s", "i");
            if ((unsigned int)i >= 0x800)
                MyAssertHandler(".\\qcommon\\dobj_management.cpp", 178, 0, "%s", "(unsigned)i < ARRAY_COUNT( objAlloced )");
            objAlloced[i] = 1;
            if (!objFreeCount)
                MyAssertHandler(".\\qcommon\\dobj_management.cpp", 180, 0, "%s", "objFreeCount");
            --objFreeCount;
            return i;
        }
    }
    for (ia = 1; ia <= com_lastDObjIndex; ++ia)
    {
        if (!objAlloced[ia])
        {
            com_lastDObjIndex = ia;
            if (!ia)
                MyAssertHandler(".\\qcommon\\dobj_management.cpp", 195, 0, "%s", "i");
            if ((unsigned int)ia >= 0x800)
                MyAssertHandler(".\\qcommon\\dobj_management.cpp", 196, 0, "%s", "(unsigned)i < ARRAY_COUNT( objAlloced )");
            objAlloced[ia] = 1;
            if (!objFreeCount)
                MyAssertHandler(".\\qcommon\\dobj_management.cpp", 198, 0, "%s", "objFreeCount");
            --objFreeCount;
            return ia;
        }
    }
    if (!alwaysfails)
        MyAssertHandler(".\\qcommon\\dobj_management.cpp", 208, 0, "unreachable");
    return 0;
}

void __cdecl Com_ClientDObjClearAllSkel()
{
    int handleOffset; // [esp+4h] [ebp-4h]

    for (handleOffset = 0; handleOffset < 1152; ++handleOffset)
    {
        if (clientObjMap[handleOffset])
            DObjSkelClear((const DObj_s *)objBuf[clientObjMap[handleOffset]]);
    }
}

DObj_s *__cdecl Com_ServerDObjCreate(
    DObjModel_s *dobjModels,
    unsigned __int16 numModels,
    XAnimTree_s *tree,
    unsigned int handle)
{
    unsigned int index; // [esp+0h] [ebp-4h]

    if (!dobjModels)
        MyAssertHandler(".\\qcommon\\dobj_management.cpp", 290, 0, "%s", "dobjModels");
    if (handle >= 0x400)
        MyAssertHandler(
            ".\\qcommon\\dobj_management.cpp",
            291,
            0,
            "%s\n\t(handle) = %i",
            "((unsigned)handle < ((1<<10)))",
            handle);
    if (Com_GetServerDObj(handle))
        MyAssertHandler(".\\qcommon\\dobj_management.cpp", 292, 0, "%s", "!Com_GetServerDObj( handle )");
    index = Com_GetFreeDObjIndex();
    if (handle >= 0x400)
        MyAssertHandler(".\\qcommon\\dobj_management.cpp", 299, 0, "%s", "(unsigned)handle < ARRAY_COUNT( serverObjMap )");
    serverObjMap[handle] = index;
    if (index >= 0x800)
        MyAssertHandler(".\\qcommon\\dobj_management.cpp", 301, 0, "%s", "(unsigned)index < DOBJ_HANDLE_MAX");
    DObjCreate(dobjModels, numModels, tree, objBuf[index], handle + 1);
    if (!objFreeCount)
        Com_Error(ERR_DROP, "No free DObjs");
    return (DObj_s *)objBuf[index];
}

void __cdecl Com_SafeClientDObjFree(unsigned int handle, int localClientNum)
{
    unsigned int index; // [esp+0h] [ebp-4h]

    if (handle >= 0x480)
    {
        MyAssertHandler(
            ".\\qcommon\\dobj_management.cpp",
            379,
            0,
            "%s\n\t(handle) = %i",
            "((unsigned)handle < (((1<<10)) + 128))",
            handle);
        MyAssertHandler(
            ".\\qcommon\\dobj_management.cpp",
            385,
            0,
            "%s\n\t(handle) = %i",
            "((unsigned)handle < (sizeof( clientObjMap ) / (sizeof( clientObjMap[0] ) * (sizeof( clientObjMap ) != 4 || sizeof("
            " clientObjMap[0] ) <= 4))))",
            handle);
    }
    if (handle - 1152 * localClientNum >= 0x480)
        MyAssertHandler(
            ".\\qcommon\\dobj_management.cpp",
            386,
            0,
            "handle - localClientNum * CLIENT_DOBJ_HANDLE_MAX doesn't index CLIENT_DOBJ_HANDLE_MAX\n\t%i not in [0, %i)",
            handle - 1152 * localClientNum,
            1152);
    index = clientObjMap[handle];
    if (clientObjMap[handle])
    {
        clientObjMap[handle] = 0;
        if (index >= 0x800)
            MyAssertHandler(".\\qcommon\\dobj_management.cpp", 393, 0, "%s", "(unsigned)index < ARRAY_COUNT( objAlloced )");
        if (!Sys_IsMainThread())
            MyAssertHandler(".\\qcommon\\dobj_management.cpp", 399, 0, "%s", "Sys_IsMainThread()");
        objAlloced[index] = 0;
        ++objFreeCount;
        DObjFree((DObj_s *)objBuf[index]);
    }
}

void __cdecl Com_SafeServerDObjFree(unsigned int handle)
{
    unsigned int index; // [esp+0h] [ebp-4h]

    if (handle >= 0x400)
        MyAssertHandler(
            ".\\qcommon\\dobj_management.cpp",
            423,
            0,
            "%s\n\t(handle) = %i",
            "((unsigned)handle < ((1<<10)))",
            handle);
    index = serverObjMap[handle];
    if (serverObjMap[handle])
    {
        serverObjMap[handle] = 0;
        if (index >= 0x800)
            MyAssertHandler(".\\qcommon\\dobj_management.cpp", 434, 0, "%s", "(unsigned)index < ARRAY_COUNT( objAlloced )");
        if (!Sys_IsMainThread())
            MyAssertHandler(".\\qcommon\\dobj_management.cpp", 440, 0, "%s", "Sys_IsMainThread()");
        objAlloced[index] = 0;
        ++objFreeCount;
        DObjFree((DObj_s *)objBuf[index]);
    }
}

int g_bDObjInited;
void __cdecl Com_InitDObj()
{
    Com_Memset((unsigned int *)objAlloced, 0, 2048);
    objFreeCount = 2047;
    Com_Memset((unsigned int *)clientObjMap, 0, 2304);
    Com_Memset((unsigned int *)serverObjMap, 0, 2048);
    com_lastDObjIndex = 1;
    g_bDObjInited = 1;
}

void __cdecl Com_ShutdownDObj()
{
    const char *v0; // eax
    const char *v1; // eax
    unsigned int i; // [esp+0h] [ebp-4h]
    unsigned int ia; // [esp+0h] [ebp-4h]
    unsigned int ib; // [esp+0h] [ebp-4h]

    if (g_bDObjInited)
    {
        g_bDObjInited = 0;
        for (i = 0; i < 0x800; ++i)
        {
            if (objAlloced[i])
                MyAssertHandler(".\\qcommon\\dobj_management.cpp", 492, 0, "%s\n\t(i) = %i", "(!objAlloced[i])", i);
        }
        for (ia = 0; ia < 0x480; ++ia)
        {
            if (clientObjMap[ia])
            {
                v0 = va("i: %d, clientObjMap[i]: %d", ia, clientObjMap[ia]);
                MyAssertHandler(".\\qcommon\\dobj_management.cpp", 494, 0, "%s\n\t%s", "!clientObjMap[i]", v0);
            }
        }
        for (ib = 0; ib < 0x400; ++ib)
        {
            if (serverObjMap[ib])
            {
                v1 = va("i: %d, serverObjMap[i]: %d", ib, serverObjMap[ib]);
                MyAssertHandler(".\\qcommon\\dobj_management.cpp", 500, 0, "%s\n\t%s", "!serverObjMap[i]", v1);
            }
        }
        if (objFreeCount != 2047)
            MyAssertHandler(
                ".\\qcommon\\dobj_management.cpp",
                501,
                0,
                "%s\n\t(objFreeCount) = %i",
                "(objFreeCount == (sizeof( objAlloced ) / (sizeof( objAlloced[0] ) * (sizeof( objAlloced ) != 4 || sizeof( objAll"
                "oced[0] ) <= 4))) - 1)",
                objFreeCount);
    }
}

void __cdecl DB_SaveDObjs()
{
    signed int handle; // [esp+0h] [ebp-Ch]
    signed int handlea; // [esp+0h] [ebp-Ch]
    DObj_s *obj; // [esp+4h] [ebp-8h]
    DObj_s *obja; // [esp+4h] [ebp-8h]
    int localClientNum; // [esp+8h] [ebp-4h]

    for (localClientNum = 0; localClientNum < 1; ++localClientNum)
    {
        for (handle = 0; handle < 1152; ++handle)
        {
            obj = Com_GetClientDObj(handle, localClientNum);
            if (obj)
                DObjArchive(obj);
        }
    }
    for (handlea = 0; handlea < 1024; ++handlea)
    {
        obja = Com_GetServerDObj(handlea);
        if (obja)
            DObjArchive(obja);
    }
}

void __cdecl DB_LoadDObjs()
{
    signed int handle; // [esp+0h] [ebp-Ch]
    signed int handlea; // [esp+0h] [ebp-Ch]
    DObj_s *obj; // [esp+4h] [ebp-8h]
    DObj_s *obja; // [esp+4h] [ebp-8h]
    int localClientNum; // [esp+8h] [ebp-4h]

    for (localClientNum = 0; localClientNum < 1; ++localClientNum)
    {
        for (handle = 0; handle < 1152; ++handle)
        {
            obj = Com_GetClientDObj(handle, localClientNum);
            if (obj)
                DObjUnarchive(obj);
        }
    }
    for (handlea = 0; handlea < 1024; ++handlea)
    {
        obja = Com_GetServerDObj(handlea);
        if (obja)
            DObjUnarchive(obja);
    }
}

