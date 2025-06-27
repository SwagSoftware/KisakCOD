#include "r_buffers.h"
#include <qcommon/mem_track.h>
#include "r_init.h"
#include <universal/com_memory.h>
#include "r_dvars.h"
#include "rb_logfile.h"
#include "r_utils.h"
#include <universal/profile.h>


//struct GfxBuffers gfxBuf   85b3aa20     gfx_d3d : r_buffers.obj
GfxBuffers gfxBuf;

void __cdecl TRACK_r_buffers()
{
    track_static_alloc_internal(&gfxBuf, 2359456, "gfxBuf", 18);
}

void *__cdecl R_AllocDynamicVertexBuffer(IDirect3DVertexBuffer9 **vb, int sizeInBytes)
{
    int hr; // [esp+0h] [ebp-4h]

    iassert(vb);
    iassert(sizeInBytes > 0);

    if (!r_loadForRenderer->current.enabled)
        return 0;

    hr = dx.device->CreateVertexBuffer(sizeInBytes, 520, 0, D3DPOOL_DEFAULT, vb, 0);
    if (hr < 0)
    {
        R_FatalInitError(va("DirectX didn't create a %i-byte dynamic vertex buffer: %s\n", sizeInBytes, R_ErrorDescription(hr)));
    }
    return 0;
}

void *__cdecl R_AllocStaticVertexBuffer(IDirect3DVertexBuffer9 **vb, int sizeInBytes)
{
    const char *v3; // eax
    const char *v4; // eax
    const char *v5; // eax
    const char *v6; // eax
    int hr; // [esp+0h] [ebp-8h]
    int hra; // [esp+0h] [ebp-8h]
    void *vertexBufferData; // [esp+4h] [ebp-4h] BYREF

    if (!vb)
        MyAssertHandler(".\\r_buffers.cpp", 184, 0, "%s", "vb");
    if (sizeInBytes <= 0)
        MyAssertHandler(".\\r_buffers.cpp", 185, 0, "%s\n\t(sizeInBytes) = %i", "(sizeInBytes > 0)", sizeInBytes);
    if (!r_loadForRenderer->current.enabled)
        return 0;
    hr = dx.device->CreateVertexBuffer(sizeInBytes, 8, 0, D3DPOOL_DEFAULT, vb, 0);
    if (hr < 0)
    {
        v3 = R_ErrorDescription(hr);
        v4 = va("DirectX didn't create a %i-byte vertex buffer: %s\n", sizeInBytes, v3);
        R_FatalInitError(v4);
    }
    //hra = (*vb)->Lock(*vb, 0, 0, &vertexBufferData, 0);
    hra = (*vb)->Lock(0, 0, &vertexBufferData, 0);
    if (hra < 0)
    {
        v5 = R_ErrorDescription(hra);
        v6 = va("DirectX didn't lock a vertex buffer: %s\n", v5);
        R_FatalInitError(v6);
    }
    return vertexBufferData;
}

void *__cdecl R_AllocDynamicIndexBuffer(IDirect3DIndexBuffer9 **ib, unsigned int sizeInBytes)
{
    const char *v3; // eax
    const char *v4; // eax
    int hr; // [esp+0h] [ebp-4h]

    if (!r_loadForRenderer->current.enabled)
        return 0;
    //hr = dx.device->CreateIndexBuffer(dx.device, sizeInBytes, 520u, D3DFMT_INDEX16, D3DPOOL_DEFAULT, ib, 0);
    hr = dx.device->CreateIndexBuffer(sizeInBytes, 520, D3DFMT_INDEX16, D3DPOOL_DEFAULT, ib, 0);
    if (hr < 0)
    {
        v3 = R_ErrorDescription(hr);
        v4 = va("Couldn't create a %i-byte dynamic index buffer: %s", sizeInBytes, v3);
        R_FatalInitError(v4);
    }
    return 0;
}

void *__cdecl R_AllocStaticIndexBuffer(IDirect3DIndexBuffer9 **ib, int sizeInBytes)
{
    void *indexBufferData; // [esp+4h] [ebp-4h] BYREF

    if (!ib)
        MyAssertHandler(".\\r_buffers.cpp", 240, 0, "%s", "ib");
    if (sizeInBytes <= 0)
        MyAssertHandler(".\\r_buffers.cpp", 241, 0, "%s\n\t(sizeInBytes) = %i", "(sizeInBytes > 0)", sizeInBytes);
    if (!r_loadForRenderer->current.enabled)
        return 0;
    //if (((int(__thiscall *)(IDirect3DDevice9 *, IDirect3DDevice9 *, int, int, int, unsigned int, IDirect3DIndexBuffer9 **, unsigned int))dx.device->CreateIndexBuffer)(
    //    dx.device,
    //    dx.device,
    //    sizeInBytes,
    //    8,
    //    101,
    //    0,
    //    ib,
    //    0) < 0)
    //    return 0;
    if (dx.device->CreateIndexBuffer(sizeInBytes, 8, (D3DFORMAT)101, D3DPOOL_DEFAULT, ib, 0) < 0)
    {
        return 0;
    }
    //if ((*ib)->Lock(*ib, 0, 0, &indexBufferData, 0) >= 0)
    //    return indexBufferData;
    //(*ib)->Release(*ib);
    if ((*ib)->Lock(0, 0, &indexBufferData, 0) >= 0)
    {
        return indexBufferData;
    }
    (*ib)->Release();
    return 0;
}

void __cdecl Load_VertexBuffer(IDirect3DVertexBuffer9 **vb, unsigned __int8 *bufferData, int sizeInBytes)
{
    unsigned __int8 *v3; // eax

    if (r_loadForRenderer->current.enabled && bufferData)
    {
        v3 = (unsigned __int8 *)R_AllocStaticVertexBuffer(vb, sizeInBytes);
        memcpy(v3, bufferData, sizeInBytes);
        R_FinishStaticVertexBuffer(*vb);
    }
    else
    {
        *vb = 0;
    }
}

void __cdecl R_InitDynamicVertexBufferState(GfxVertexBufferState *vb, int bytes)
{
    unsigned __int8 *verts; // [esp+0h] [ebp-4h]

    if (!vb)
        MyAssertHandler(".\\r_buffers.cpp", 405, 0, "%s", "vb");
    vb->used = 0;
    vb->total = bytes;
    verts = (unsigned __int8 *)R_AllocDynamicVertexBuffer(&vb->buffer, bytes);
    if (verts)
        MyAssertHandler(".\\r_buffers.cpp", 416, 1, "%s", "verts == NULL");
    vb->verts = verts;
}

void __cdecl R_InitDynamicIndexBufferState(GfxIndexBufferState *ib, int indexCount)
{
    if (!ib)
        MyAssertHandler(".\\r_buffers.cpp", 429, 0, "%s", "ib");
    ib->used = 0;
    ib->total = indexCount;
    if (R_AllocDynamicIndexBuffer(&ib->buffer, 2 * indexCount))
        MyAssertHandler(".\\r_buffers.cpp", 441, 1, "%s", "indices == NULL");
}

void __cdecl R_InitDynamicIndices(GfxDynamicIndices *ib, int indexCount)
{
    unsigned __int16 *indices; // [esp+0h] [ebp-8h]

    if (!ib)
        MyAssertHandler(".\\r_buffers.cpp", 453, 0, "%s", "ib");
    ib->used = 0;
    ib->total = indexCount;
    indices = (unsigned __int16 *)Z_VirtualAlloc(2 * indexCount, "Dynamic Index Buffer", 18);
    if (!indices)
        MyAssertHandler(".\\r_buffers.cpp", 474, 1, "%s", "indices != NULL");
    ib->indices = indices;
}

void __cdecl R_ShutdownDynamicIndices(GfxDynamicIndices *ib)
{
    if (!ib->indices)
        MyAssertHandler(".\\r_buffers.cpp", 557, 0, "%s", "ib->indices");
    Z_VirtualFree(ib->indices);
    ib->indices = 0;
}

void __cdecl R_CreateDynamicBuffers()
{
    int bufferIter; // [esp+0h] [ebp-4h]
    int bufferItera; // [esp+0h] [ebp-4h]
    int bufferIterb; // [esp+0h] [ebp-4h]
    int bufferIterc; // [esp+0h] [ebp-4h]

    for (bufferIter = 0; bufferIter != 1; ++bufferIter)
        R_InitDynamicVertexBufferState(&gfxBuf.dynamicVertexBufferPool[bufferIter], 0x100000);
    gfxBuf.dynamicVertexBuffer = gfxBuf.dynamicVertexBufferPool;
    for (bufferItera = 0; bufferItera != 2; ++bufferItera)
        R_InitDynamicVertexBufferState(&gfxBuf.skinnedCacheVbPool[bufferItera], 0x480000);
    R_InitTempSkinBuf();
    for (bufferIterb = 0; bufferIterb != 1; ++bufferIterb)
        R_InitDynamicIndexBufferState(&gfxBuf.dynamicIndexBufferPool[bufferIterb], 0x100000);
    gfxBuf.dynamicIndexBuffer = gfxBuf.dynamicIndexBufferPool;
    for (bufferIterc = 0; bufferIterc != 2; ++bufferIterc)
        R_InitDynamicIndexBufferState(&gfxBuf.preTessIndexBufferPool[bufferIterc], 0x100000);
    gfxBuf.preTessIndexBuffer = gfxBuf.preTessIndexBufferPool;
    gfxBuf.preTessBufferFrame = 0;
}

void __cdecl R_FinishStaticIndexBuffer(IDirect3DIndexBuffer9 *ib)
{
    const char *v1; // eax
    int hr; // [esp+0h] [ebp-4h]

    do
    {
        if (r_logFile && r_logFile->current.integer)
            RB_LogPrint("ib->Unlock()\n");
        //hr = ib->Unlock(ib);
        hr = ib->Unlock();
        if (hr < 0)
        {
            do
            {
                ++g_disableRendering;
                v1 = R_ErrorDescription(hr);
                Com_Error(ERR_FATAL, ".\\r_buffers.cpp (%i) ib->Unlock() failed: %s\n", 266, v1);
            } while (alwaysfails);
        }
    } while (alwaysfails);
}

void __cdecl R_CreateParticleCloudBuffer()
{
    double *v0; // [esp+10h] [ebp-70h]
    float cornerTexCoords[4][2]; // [esp+14h] [ebp-6Ch] BYREF
    unsigned __int16 *particleIndicesIter; // [esp+34h] [ebp-4Ch]
    float pos[3]; // [esp+38h] [ebp-48h]
    int particleId; // [esp+44h] [ebp-3Ch]
    int indexSizeInBytes; // [esp+48h] [ebp-38h]
    int xIter; // [esp+4Ch] [ebp-34h]
    int vertSizeInBytes; // [esp+50h] [ebp-30h]
    unsigned __int16 *particleIndices; // [esp+54h] [ebp-2Ch]
    int yIter; // [esp+58h] [ebp-28h]
    int zIter; // [esp+5Ch] [ebp-24h]
    int cornerIter; // [esp+60h] [ebp-20h]
    GfxPosTexVertex *particleVerts; // [esp+64h] [ebp-1Ch]
    unsigned __int16 quadIndices[6]; // [esp+68h] [ebp-18h]
    GfxPosTexVertex *particleVertsIter; // [esp+78h] [ebp-8h]
    int indIter; // [esp+7Ch] [ebp-4h]

    cornerTexCoords[0][0] = 0.0;
    cornerTexCoords[0][1] = 0.0;
    cornerTexCoords[1][0] = 0.0;
    cornerTexCoords[1][1] = 1.0;
    cornerTexCoords[2][0] = 1.0;
    cornerTexCoords[2][1] = 0.0;
    cornerTexCoords[3][0] = 1.0;
    cornerTexCoords[3][1] = 1.0;
    quadIndices[0] = 0;
    quadIndices[1] = 1;
    quadIndices[2] = 2;
    quadIndices[3] = 2;
    quadIndices[4] = 1;
    quadIndices[5] = 3;
    vertSizeInBytes = 81920;
    indexSizeInBytes = 12288;
    particleVerts = (GfxPosTexVertex *)R_AllocStaticVertexBuffer(&gfxBuf.particleCloudVertexBuffer, 81920);
    particleIndices = (unsigned __int16 *)R_AllocStaticIndexBuffer(&gfxBuf.particleCloudIndexBuffer, 12288);
    particleVertsIter = particleVerts;
    particleIndicesIter = particleIndices;
    for (xIter = 0; xIter != 8; ++xIter)
    {
        for (yIter = 0; yIter != 8; ++yIter)
        {
            for (zIter = 0; zIter != 16; ++zIter)
            {
                particleId = zIter + (xIter << 7) + 16 * yIter;
                pos[0] = ((double)rand() / 32767.0 + (double)xIter) * 0.25 + -1.0;
                pos[1] = ((double)rand() / 32767.0 + (double)yIter) * 0.25 + -1.0;
                pos[2] = ((double)rand() / 32767.0 + (double)zIter) * 0.125 + -1.0;
                for (cornerIter = 0; cornerIter != 4; ++cornerIter)
                {
                    particleVertsIter->xyz[0] = pos[0];
                    particleVertsIter->xyz[1] = pos[1];
                    particleVertsIter->xyz[2] = pos[2];
                    v0 = (double *)cornerTexCoords[cornerIter];
                    *(double *)particleVertsIter->texCoord = *v0;
                    ++particleVertsIter;
                }
                for (indIter = 0; indIter != 6; ++indIter)
                {
                    *particleIndicesIter = quadIndices[indIter] + 4 * particleId;
                    if (quadIndices[indIter] + 4 * particleId != *particleIndicesIter)
                        MyAssertHandler(
                            ".\\r_buffers.cpp",
                            615,
                            0,
                            "%s",
                            "static_cast< int >( quadIndices[indIter] ) + particleId * 4 == static_cast< int >( *particleIndicesIter )");
                    ++particleIndicesIter;
                }
            }
        }
    }
    R_FinishStaticIndexBuffer(gfxBuf.particleCloudIndexBuffer);
    R_FinishStaticVertexBuffer(gfxBuf.particleCloudVertexBuffer);
}



void __cdecl R_FinishStaticVertexBuffer(IDirect3DVertexBuffer9 *vb)
{
    const char *v1; // eax
    int hr; // [esp+0h] [ebp-4h]

    do
    {
        if (r_logFile && r_logFile->current.integer)
            RB_LogPrint("vb->Unlock()\n");
        //hr = vb->Unlock(vb);
        hr = vb->Unlock();
        if (hr < 0)
        {
            do
            {
                ++g_disableRendering;
                v1 = R_ErrorDescription(hr);
                Com_Error(ERR_FATAL, ".\\r_buffers.cpp (%i) vb->Unlock() failed: %s\n", 207, v1);
            } while (alwaysfails);
        }
    } while (alwaysfails);
}

void __cdecl R_UnlockVertexBuffer(IDirect3DVertexBuffer9* handle)
{
    if (!handle)
        MyAssertHandler(".\\r_buffers.cpp", 131, 0, "%s", "handle");
    handle->Unlock();
}

void *__cdecl R_LockVertexBuffer(IDirect3DVertexBuffer9 *handle, int offset, int bytes, int lockFlags)
{
    int hr; // [esp+0h] [ebp-8h]
    void *bufferData; // [esp+4h] [ebp-4h] BYREF

    if (!handle)
        MyAssertHandler(".\\r_buffers.cpp", 119, 0, "%s", "handle");
    if (dx.deviceLost)
        MyAssertHandler(".\\r_buffers.cpp", 120, 0, "%s", "!dx.deviceLost");
    //hr = ((int(__thiscall *)(IDirect3DVertexBuffer9 *, IDirect3DVertexBuffer9 *, int, int, void **, int))handle->Lock)(
    //    handle,
    //    handle,
    //    offset,
    //    bytes,
    //    &bufferData,
    //    lockFlags);
    hr = handle->Lock(offset, bytes, &bufferData, lockFlags);
    if (hr < 0)
        R_FatalLockError(hr);
    return bufferData;
}

void __cdecl R_ShutdownTempSkinBuf()
{
    GfxBackEndData *data; // [esp+0h] [ebp-8h]
    unsigned int i; // [esp+4h] [ebp-4h]

    for (i = 0; i < 2; ++i)
    {
        data = &s_backEndData[i];
        if (data->tempSkinBuf)
        {
            Z_VirtualFree(data->tempSkinBuf);
            data->tempSkinBuf = 0;
            data->tempSkinPos = 0;
        }
    }
}

void __cdecl R_FreeStaticVertexBuffer(IDirect3DVertexBuffer9 *vb)
{
    IDirect3DVertexBuffer9 *varCopy; // [esp+0h] [ebp-4h]

    do
    {
        if (r_logFile)
        {
            if (r_logFile->current.integer)
                RB_LogPrint("vb->Release()\n");
        }
        varCopy = vb;
        vb = 0;
        R_ReleaseAndSetNULL<IDirect3DSurface9>((IDirect3DSurface9 *)varCopy, "vb", ".\\r_buffers.cpp", 213);
    } while (alwaysfails);
}

void __cdecl R_FreeStaticIndexBuffer(IDirect3DIndexBuffer9 *ib)
{
    IDirect3DIndexBuffer9 *varCopy; // [esp+0h] [ebp-4h]

    do
    {
        if (r_logFile)
        {
            if (r_logFile->current.integer)
                RB_LogPrint("ib->Release()\n");
        }
        varCopy = ib;
        ib = 0;
        R_ReleaseAndSetNULL<IDirect3DDevice9>((IDirect3DSurface9 *)varCopy, "ib", ".\\r_buffers.cpp", 272);
    } while (alwaysfails);
}

void __cdecl R_DestroyParticleCloudBuffer()
{
    if (gfxBuf.particleCloudVertexBuffer)
    {
        R_FreeStaticVertexBuffer(gfxBuf.particleCloudVertexBuffer);
        gfxBuf.particleCloudVertexBuffer = 0;
    }
    if (gfxBuf.particleCloudIndexBuffer)
    {
        R_FreeStaticIndexBuffer(gfxBuf.particleCloudIndexBuffer);
        gfxBuf.particleCloudIndexBuffer = 0;
    }
}

void __cdecl R_DestroyDynamicBuffers()
{
    IDirect3DSurface9 *v0; // [esp+0h] [ebp-14h]
    IDirect3DSurface9 *buffer; // [esp+4h] [ebp-10h]
    IDirect3DSurface9 *var; // [esp+8h] [ebp-Ch]
    IDirect3DIndexBuffer9 *varCopy; // [esp+Ch] [ebp-8h]
    int bufferIter; // [esp+10h] [ebp-4h]
    int bufferItera; // [esp+10h] [ebp-4h]
    int bufferIterb; // [esp+10h] [ebp-4h]
    int bufferIterc; // [esp+10h] [ebp-4h]

    for (bufferIter = 0; bufferIter != 2; ++bufferIter)
    {
        if (gfxBuf.preTessIndexBufferPool[bufferIter].buffer)
        {
            do
            {
                if (r_logFile)
                {
                    if (r_logFile->current.integer)
                        RB_LogPrint("gfxBuf.preTessIndexBufferPool[bufferIter].buffer->Release()\n");
                }
                varCopy = gfxBuf.preTessIndexBufferPool[bufferIter].buffer;
                gfxBuf.preTessIndexBufferPool[bufferIter].buffer = 0;
                R_ReleaseAndSetNULL<IDirect3DDevice9>(
                    (IDirect3DSurface9 *)varCopy,
                    "gfxBuf.preTessIndexBufferPool[bufferIter].buffer",
                    ".\\r_buffers.cpp",
                    522);
            } while (alwaysfails);
        }
    }
    for (bufferItera = 0; bufferItera != 1; ++bufferItera)
    {
        if (gfxBuf.dynamicIndexBufferPool[bufferItera].buffer)
        {
            do
            {
                if (r_logFile && r_logFile->current.integer)
                    RB_LogPrint("gfxBuf.dynamicIndexBufferPool[bufferIter].buffer->Release()\n");
                var = (IDirect3DSurface9 *)gfxBuf.dynamicIndexBufferPool[bufferItera].buffer;
                gfxBuf.dynamicIndexBufferPool[bufferItera].buffer = 0;
                R_ReleaseAndSetNULL<IDirect3DDevice9>(
                    var,
                    "gfxBuf.dynamicIndexBufferPool[bufferIter].buffer",
                    ".\\r_buffers.cpp",
                    530);
            } while (alwaysfails);
        }
    }
    for (bufferIterb = 0; bufferIterb != 2; ++bufferIterb)
    {
        if (gfxBuf.skinnedCacheVbPool[bufferIterb].buffer)
        {
            do
            {
                if (r_logFile && r_logFile->current.integer)
                    RB_LogPrint("gfxBuf.skinnedCacheVbPool[bufferIter].buffer->Release()\n");
                buffer = (IDirect3DSurface9 *)gfxBuf.skinnedCacheVbPool[bufferIterb].buffer;
                gfxBuf.skinnedCacheVbPool[bufferIterb].buffer = 0;
                R_ReleaseAndSetNULL<IDirect3DDevice9>(
                    buffer,
                    "gfxBuf.skinnedCacheVbPool[bufferIter].buffer",
                    ".\\r_buffers.cpp",
                    537);
            } while (alwaysfails);
        }
    }
    for (bufferIterc = 0; bufferIterc != 1; ++bufferIterc)
    {
        if (gfxBuf.dynamicVertexBufferPool[bufferIterc].buffer)
        {
            do
            {
                if (r_logFile && r_logFile->current.integer)
                    RB_LogPrint("gfxBuf.dynamicVertexBufferPool[bufferIter].buffer->Release()\n");
                v0 = (IDirect3DSurface9 *)gfxBuf.dynamicVertexBufferPool[bufferIterc].buffer;
                gfxBuf.dynamicVertexBufferPool[bufferIterc].buffer = 0;
                R_ReleaseAndSetNULL<IDirect3DDevice9>(
                    v0,
                    "gfxBuf.dynamicVertexBufferPool[bufferIter].buffer",
                    ".\\r_buffers.cpp",
                    544);
            } while (alwaysfails);
        }
    }
    R_ShutdownTempSkinBuf();
}

void *__cdecl R_LockIndexBuffer(IDirect3DIndexBuffer9 *handle, int offset, int bytes, int lockFlags)
{
    int hr; // [esp+0h] [ebp-8h]
    void *bufferData; // [esp+4h] [ebp-4h] BYREF

    if (!handle)
        MyAssertHandler(".\\r_buffers.cpp", 141, 0, "%s", "handle");
    if (dx.deviceLost)
        MyAssertHandler(".\\r_buffers.cpp", 142, 0, "%s", "!dx.deviceLost");
    //hr = ((int(__thiscall *)(IDirect3DIndexBuffer9 *, IDirect3DIndexBuffer9 *, int, int, void **, int))handle->Lock)(
    //    handle,
    //    handle,
    //    offset,
    //    bytes,
    //    &bufferData,
    //    lockFlags);
    hr = handle->Lock(offset, bytes, &bufferData, lockFlags);
    if (hr < 0)
        R_FatalLockError(hr);
    return bufferData;
}

void __cdecl R_UnlockIndexBuffer(IDirect3DIndexBuffer9 *handle)
{
    if (!handle)
        MyAssertHandler(".\\r_buffers.cpp", 153, 0, "%s", "handle");
    handle->Unlock();
}

void __cdecl R_CreateWorldVertexBuffer(IDirect3DVertexBuffer9 **vb, int *srcData, unsigned int sizeInBytes)
{
    int dummyData; // [esp+30h] [ebp-8h] BYREF
    void *dstData; // [esp+34h] [ebp-4h]

    if (r_loadForRenderer->current.enabled)
    {
        if ((srcData == 0) != (sizeInBytes == 0))
            MyAssertHandler(".\\r_bsp_load_obj.cpp", 301, 0, "%s", "(srcData == NULL) == (sizeInBytes == 0)");
        if (!sizeInBytes)
        {
            dummyData = 0;
            srcData = &dummyData;
            sizeInBytes = 4;
        }
        dstData = R_AllocStaticVertexBuffer(vb, sizeInBytes);
        {
            PROF_SCOPED("R_Memcpy");
            Com_Memcpy(dstData, srcData, sizeInBytes);
        }
        R_FinishStaticVertexBuffer(*vb);
    }
    else
    {
        *vb = 0;
    }
}