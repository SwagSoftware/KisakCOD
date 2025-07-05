#include "database.h"

#include <qcommon/threads.h>
#include <win32/win_local.h>
#include <client_mp/client_mp.h>
#include <universal/com_files.h>

#include <gfx_d3d/r_image.h>
#include <gfx_d3d/r_buffers.h>

//uint32_t volatile g_loadingAssets      828e3f3c     db_file_load.obj
//int32_t marker_db_file_load  828e3f40     db_file_load.obj

struct DB_LoadData // sizeof=0x68
{                                       // ...
    void* f;                            // ...
    const char* filename;               // ...
    XZoneMemory* zoneMem;               // ...
    int32_t outstandingReads;               // ...
    OVERLAPPED overlapped;             // ...
    z_stream_s stream;                  // ...
    uint8_t* compressBufferStart; // ...
    uint8_t* compressBufferEnd; // ...
    void(__cdecl* interrupt)();        // ...
    int32_t allocType;                      // ...
};

bool g_minimumFastFileLoaded;

DB_LoadData g_load;
LONG g_loadedSize;
LONG g_loadedExternalBytes;
volatile int32_t g_totalSize;
volatile int32_t g_totalExternalBytes;
int32_t g_trackLoadProgress;

XAssetList g_varXAssetList;

void __cdecl DB_CancelLoadXFile()
{
    if (g_load.compressBufferStart)
    {
        while (g_load.outstandingReads)
            DB_WaitXFileStage();
        DB_AuthLoad_InflateEnd(&g_load.stream);
        if (!g_load.f)
            MyAssertHandler(".\\database\\db_file_load.cpp", 165, 0, "%s", "g_load.f");
        CloseHandle(g_load.f);
    }
}

int32_t DB_WaitXFileStage()
{
    int32_t result; // eax

    if (!g_load.f)
        MyAssertHandler(".\\database\\db_file_load.cpp", 278, 0, "%s", "g_load.f");
    if (g_load.outstandingReads <= 0)
        MyAssertHandler(".\\database\\db_file_load.cpp", 280, 0, "%s", "g_load.outstandingReads > 0");
    --g_load.outstandingReads;
    SleepEx(0xFFFFFFFF, 1);
    result = InterlockedIncrement(&g_loadedSize);
    g_load.stream.avail_in += 0x40000;
    return result;
}

void __cdecl DB_LoadedExternalData(int32_t size)
{
    InterlockedExchangeAdd(&g_loadedExternalBytes, size);
}

double __cdecl DB_GetLoadedFraction()
{
    double loadedBytesInternal; // [esp+14h] [ebp-20h]
    double totalBytesInternal; // [esp+1Ch] [ebp-18h]
    double loadedBytesExternal; // [esp+24h] [ebp-10h]
    double totalBytesExternal; // [esp+2Ch] [ebp-8h]

    if (!g_totalSize)
        return 0.0;
    totalBytesInternal = (double)g_totalSize * 262144.0;
    loadedBytesInternal = (double)g_loadedSize * 262144.0;
    if (loadedBytesInternal < 0.0)
        MyAssertHandler(".\\database\\db_file_load.cpp", 341, 0, "%s", "loadedBytesInternal >= 0");
    if (totalBytesInternal < loadedBytesInternal)
        loadedBytesInternal = totalBytesInternal;
    totalBytesExternal = (double)g_totalExternalBytes;
    loadedBytesExternal = (double)g_loadedExternalBytes;
    if (totalBytesExternal < loadedBytesExternal)
        loadedBytesExternal = totalBytesExternal;
    return (float)((loadedBytesInternal + loadedBytesExternal) / (totalBytesInternal + totalBytesExternal));
}

void __cdecl DB_LoadXFileData(uint8_t *pos, uint32_t size)
{
    const char *v2; // eax
    uint32_t err; // [esp+0h] [ebp-4h]

    if (!size)
        MyAssertHandler(".\\database\\db_file_load.cpp", 366, 0, "%s", "size");
    if (!g_load.f)
        MyAssertHandler(".\\database\\db_file_load.cpp", 368, 0, "%s", "g_load.f");
    if (g_load.stream.avail_out)
        MyAssertHandler(".\\database\\db_file_load.cpp", 369, 0, "%s", "!g_load.stream.avail_out");
    g_load.stream.next_out = pos;
    g_load.stream.avail_out = size;
    while (1)
    {
        if (!g_load.stream.avail_in)
            goto LABEL_19;
        err = DB_AuthLoad_Inflate(&g_load.stream, 2);
        if (err >= 2)
        {
            KISAK_NULLSUB();
            DB_CancelLoadXFile();
            Com_Error(ERR_DROP, "Fastfile for zone '%s' appears corrupt or unreadable (code %i.)", g_load.filename, err + 110);
        }
        if (g_load.f)
        {
            if ((uint32_t)(g_load.stream.next_in - g_load.compressBufferStart) > 0x80000)
                MyAssertHandler(
                    ".\\database\\db_file_load.cpp",
                    392,
                    0,
                    "%s",
                    "static_cast< unsigned >( g_load.stream.next_in - g_load.compressBufferStart ) <= FILE_BUFFER_SIZE * 2");
            if (g_load.stream.next_in == g_load.compressBufferEnd)
                g_load.stream.next_in = g_load.compressBufferStart;
        }
        if (!g_load.stream.avail_out)
            break;
        if (err)
        {
            v2 = va("Invalid fast file '%s' (%d != Z_OK)", g_load.filename, err);
            MyAssertHandler(".\\database\\db_file_load.cpp", 402, 0, "%s\n\t%s", "err == Z_OK", v2);
        }
    LABEL_19:
        DB_WaitXFileStage();
        DB_ReadXFileStage();
    }
}

void DB_ReadXFileStage()
{
    if (g_load.f)
    {
        if (g_load.outstandingReads)
            MyAssertHandler(".\\database\\db_file_load.cpp", 254, 0, "%s", "!g_load.outstandingReads");
        if (!DB_ReadData() && GetLastError() != 38)
            Com_Error(ERR_DROP, "Read error of file '%s'", g_load.filename);
    }
}

int32_t __cdecl DB_ReadData()
{
    uint8_t *fileBuffer; // [esp+0h] [ebp-4h]

    if (!g_load.compressBufferStart)
        MyAssertHandler(".\\database\\db_file_load.cpp", 188, 0, "%s", "g_load.compressBufferStart");
    if (!g_load.f)
        MyAssertHandler(".\\database\\db_file_load.cpp", 189, 0, "%s", "g_load.f");
    if (g_load.interrupt)
        g_load.interrupt();
    fileBuffer = &g_load.compressBufferStart[g_load.overlapped.Offset % 0x80000];
    Sys_WaitDatabaseThread();
    if (!ReadFileEx(g_load.f, fileBuffer, 0x40000u, &g_load.overlapped, (LPOVERLAPPED_COMPLETION_ROUTINE)DB_FileReadCompletion))
        return 0;
    ++g_load.outstandingReads;
    g_load.overlapped.Offset += 0x40000;
    return 1;
}

void __stdcall DB_FileReadCompletion(
    uint32_t dwErrorCode,
    uint32_t dwNumberOfBytesTransfered,
    _OVERLAPPED *lpOverlapped)
{
    ;
}

void __cdecl DB_LoadDelayedImages()
{
    uint32_t copyIter; // [esp+0h] [ebp-4h]

    DB_EnumXAssets(ASSET_TYPE_IMAGE, (void(__cdecl *)(XAssetHeader, void *))R_DelayLoadImage, 0, 0);
    for (copyIter = 0; copyIter < g_copyInfoCount; ++copyIter)
    {
        if (g_copyInfo[copyIter]->asset.type == ASSET_TYPE_IMAGE)
            R_DelayLoadImage(g_copyInfo[copyIter]->asset.header);
    }
}

void __cdecl DB_FinishGeometryBlocks(XZoneMemory *zoneMem)
{
    if (zoneMem->lockedVertexData)
    {
        R_FinishStaticVertexBuffer((IDirect3DVertexBuffer9*)zoneMem->vertexBuffer);
        zoneMem->lockedVertexData = 0;
    }
    if (zoneMem->lockedIndexData)
    {
        R_FinishStaticIndexBuffer((IDirect3DIndexBuffer9*)zoneMem->indexBuffer);
        zoneMem->lockedIndexData = 0;
    }
}

void __cdecl DB_LoadXFileInternal()
{
    int32_t err; // [esp+8h] [ebp-4Ch]
    bool fileIsSecure; // [esp+Fh] [ebp-45h]
    uint32_t version; // [esp+10h] [ebp-44h]
    XFile file; // [esp+14h] [ebp-40h] BYREF
    int32_t fileSize; // [esp+40h] [ebp-14h]
    const char *failureReason; // [esp+44h] [ebp-10h]
    char magic[8]; // [esp+48h] [ebp-Ch] BYREF

    iassert(g_load.f);
    DB_ReadXFileStage();
    if (!g_load.outstandingReads)
        Com_Error(ERR_DROP, "Fastfile for zone '%s' is empty.", g_load.filename);
    DB_WaitXFileStage();
    DB_ReadXFileStage();
    if (g_load.stream.avail_in < 8)
        MyAssertHandler(".\\database\\db_file_load.cpp", 598, 0, "%s", "sizeof( magic ) <= g_load.stream.avail_in");
    *(uint32_t *)magic = *(uint32_t *)g_load.stream.next_in;
    *(uint32_t *)&magic[4] = *((uint32_t *)g_load.stream.next_in + 1);
    g_load.stream.next_in += 8;
    g_load.stream.avail_in -= 8;
    if (memcmp(magic, "IWff0100", 8u) && memcmp(magic, "IWffu100", 8u))
    {
        KISAK_NULLSUB();
        Com_Error(ERR_DROP, "Fastfile for zone '%s' is corrupt or unreadable.", g_load.filename);
    }
    iassert(sizeof(version) <= g_load.stream.avail_in);
    version = *(uint32_t *)g_load.stream.next_in;
    g_load.stream.next_in += 4;
    g_load.stream.avail_in -= 4;
    if (version != 5)
    {
        if (version >= 5)
            Com_Error(
                ERR_DROP,
                "Fastfile for zone '%s' is newer than client executable (version %d, expecting %d)",
                g_load.filename,
                version,
                5);
        else
            Com_Error(
                ERR_DROP,
                "Fastfile for zone '%s' is out of date (version %d, expecting %d)",
                g_load.filename,
                version,
                5);
    }
    fileIsSecure = memcmp(magic, "IWffu100", 8u) != 0;
    err = DB_AuthLoad_InflateInit(&g_load.stream, fileIsSecure);
    failureReason = 0;
    if (fileIsSecure)
        failureReason = "authenticated file not supported";
    if (err)
        failureReason = "init failed";
    if (failureReason)
    {
        KISAK_NULLSUB();
        DB_CancelLoadXFile();
        Com_Error(ERR_DROP, "Fastfile for zone '%s' could not be loaded (%s)", g_load.filename, failureReason);
    }
    
    DB_LoadXFileData((uint8_t *)&file, sizeof(XFile));
    if (g_trackLoadProgress)
    {
        fileSize = GetFileSize(g_load.f, 0);
        if (file.externalSize + fileSize >= 0x100000)
        {
            g_totalSize = (fileSize + 0x3FFFF) / 0x40000 - g_loadedSize;
            g_loadedSize = 0;
            g_totalExternalBytes = file.externalSize - g_loadedExternalBytes;
            g_loadedExternalBytes = 0;
        }
    }
    DB_AllocXZoneMemory(file.blockSize, g_load.filename, g_load.zoneMem, g_load.allocType);
    DB_InitStreams(g_load.zoneMem);
    Load_XAssetListCustom();
    DB_PushStreamPos(4);
    if (varXAssetList->assets)
    {
        varXAssetList->assets = AllocLoad_FxElemVisStateSample();
        varXAsset = varXAssetList->assets;
        Load_XAssetArrayCustom(varXAssetList->assetCount);
    }
    DB_PopStreamPos();
    DB_FinishGeometryBlocks(g_load.zoneMem);
    --g_loadingAssets;
    Load_DelayStream();
    DB_LoadDelayedImages();
    iassert(g_load.compressBufferStart);
    Com_Printf(10, "Loaded zone '%s'\n", g_load.filename);
    if (!g_minimumFastFileLoaded)
        g_minimumFastFileLoaded = I_stricmp("localized_code_post_gfx_mp", g_load.filename) == 0;
    DB_CancelLoadXFile();
}

bool __cdecl DB_IsMinimumFastFileLoaded()
{
    return g_minimumFastFileLoaded;
}

void Load_XAssetListCustom()
{
    varXAssetList = &g_varXAssetList;
    
    DB_LoadXFileData((uint8_t *)&g_varXAssetList, sizeof(XAssetList));
    DB_PushStreamPos(4u);
    varScriptStringList = &varXAssetList->stringList;
    Load_ScriptStringList(0);
    DB_PopStreamPos();
}

void __cdecl Load_XAssetArrayCustom(int32_t count)
{
    XAsset *var; // [esp+0h] [ebp-8h]
    int32_t i; // [esp+4h] [ebp-4h]

    Load_Stream(1, (uint8_t *)varXAsset, 8 * count);
    var = varXAsset;
    for (i = 0; i < count; ++i)
    {
        varXAsset = var;
        Load_XAsset(0);
        ++var;
    }
}

void __cdecl DB_ResetZoneSize(int32_t trackLoadProgress)
{
    g_totalSize = 0;
    g_loadedSize = 0;
    g_totalExternalBytes = 0;
    g_loadedExternalBytes = 0;
    g_trackLoadProgress = trackLoadProgress;
}

void __cdecl DB_LoadXFile(
    const char *path,
    void *f,
    const char *filename,
    XZoneMemory *zoneMem,
    void(__cdecl *interrupt)(),
    uint8_t *buf,
    int32_t allocType)
{
    if (((uintptr_t)buf & 3) != 0)
        MyAssertHandler(".\\database\\db_file_load.cpp", 749, 0, "%s", "!(reinterpret_cast< psize_int >( buf ) & 3)");
    memset((uint8_t *)&g_load, 0, sizeof(g_load));
    g_load.f = f;
    g_load.filename = filename;
    g_load.zoneMem = zoneMem;
    g_load.interrupt = interrupt;
    g_load.allocType = allocType;
    if (g_load.compressBufferStart)
        MyAssertHandler(".\\database\\db_file_load.cpp", 762, 0, "%s", "!g_load.compressBufferStart");
    if (!g_load.f)
        MyAssertHandler(".\\database\\db_file_load.cpp", 764, 0, "%s", "g_load.f");
    if (!buf)
        MyAssertHandler(".\\database\\db_file_load.cpp", 766, 0, "%s", "buf");
    g_load.compressBufferStart = buf;
    g_load.compressBufferEnd = buf + 0x80000;
    g_load.stream.next_in = buf;
    g_load.stream.avail_in = 0;
}

