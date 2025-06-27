#include "r_image.h"
#include <universal/q_shared.h>
#include "rb_logfile.h"
#include <universal/profile.h>
#include "r_init.h"

unsigned int __cdecl Image_CubemapFace(unsigned int faceIndex)
{
    iassert(faceIndex < 6);
    return faceIndex;
}

void __cdecl Image_GetPicmip(const GfxImage *image, Picmip *picmip)
{
    iassert(image);
    iassert(picmip);

    if (image->noPicmip)
        *picmip = NULL;
    else
        Image_PicmipForSemantic(image->semantic, picmip);
}

void __cdecl Image_PicmipForSemantic(unsigned __int8 semantic, Picmip *picmip)
{
    const char *v2; // eax
    int picmipUsed; // [esp+4h] [ebp-4h]

    switch (semantic)
    {
    case 0u:
    case 1u:
        goto $LN7_78;
    case 2u:
    case 0xBu:
        picmipUsed = imageGlobals.picmip;
        goto LABEL_8;
    case 5u:
        picmipUsed = imageGlobals.picmipBump;
        goto LABEL_8;
    case 8u:
        picmipUsed = imageGlobals.picmipSpec;
    LABEL_8:
        picmip->platform[1] = 2;
        if (picmipUsed >= 0)
        {
            if (picmipUsed > 3)
                LOBYTE(picmipUsed) = 3;
        }
        else
        {
            LOBYTE(picmipUsed) = 0;
        }
        picmip->platform[0] = picmipUsed;
        break;
    default:
        if (!alwaysfails)
        {
            v2 = va("unhandled case: %d", semantic);
            MyAssertHandler(".\\r_image.cpp", 644, 1, v2);
        }
    $LN7_78:
        *picmip = 0;
        break;
    }
}

int __cdecl Image_SourceBytesPerSlice_PC(_D3DFORMAT format, int width, int height)
{
    switch (format)
    {
    case D3DFMT_DXT3:
    case D3DFMT_DXT5:
        return 16 * ((height + 3) >> 2) * ((width + 3) >> 2);
    case D3DFMT_DXT1:
        return 8 * ((height + 3) >> 2) * ((width + 3) >> 2);
    case D3DFMT_D16:
         return 2 * height * width;
    case D3DFMT_R32F:
         return 4 * height * width;
    case D3DFMT_D24S8:
    case D3DFMT_A8R8G8B8:
        return 4 * height * width;
    case D3DFMT_X8R8G8B8:
        return 3 * height * width;
        break;
    case D3DFMT_A8:
    case D3DFMT_L8:
        return height * width;
        break;
    case D3DFMT_A8L8:
        return 2 * height * width;
    default:
        if (!alwaysfails)
        {
            MyAssertHandler(".\\r_image_load_common.cpp", 295, 1, va("unhandled case: %d", format));
        }
    }

    return 0;
}


void __cdecl Image_Upload2D_CopyDataBlock_PC(
    int width,
    int height,
    unsigned __int8 *src,
    _D3DFORMAT format,
    int dstPitch,
    unsigned __int8 *dst)
{
    const char *v6; // eax
    const char *v7; // eax
    signed int srcStride; // [esp+48h] [ebp-Ch]
    int y; // [esp+4Ch] [ebp-8h]
    int dy; // [esp+50h] [ebp-4h]

    iassert(src);
    iassert(dst);

    if (format <= D3DFMT_A8L8)
    {
        if (format != D3DFMT_A8L8)
        {
            switch (format)
            {
            case D3DFMT_A8R8G8B8:
            case D3DFMT_X8R8G8B8:
                srcStride = 4 * width;
                dy = 1;
                goto LABEL_20;
            case D3DFMT_A8:
            case D3DFMT_L8:
                srcStride = width;
                dy = 1;
                goto LABEL_20;
            default:
                goto LABEL_17;
            }
        }
        srcStride = 2 * width;
        dy = 1;
        goto LABEL_20;
    }
    if (format == D3DFMT_DXT1)
    {
        srcStride = 8 * ((width + 3) >> 2);
        dy = 4;
    LABEL_20:
        if (dstPitch < srcStride)
        {
            MyAssertHandler(".\\r_image_load_common.cpp", 525, 0, "%s\n\t%s", "dstPitch >= srcStride", va("%i x %i: %i < %i", width, height, dstPitch, srcStride));
        }
        if (dstPitch == srcStride)
        {
            PROF_SCOPED("R_memcpy");
            memcpy(dst, src, srcStride * ((height - 1) / dy + 1));
        }
        else
        {
            for (y = 0; y < height; y += dy)
            {
                PROF_SCOPED("R_memcpy");
                memcpy(dst, src, srcStride);
                dst += dstPitch;
                src += srcStride;
            }
        }
        return;
    }
    if (format == D3DFMT_DXT3 || format == D3DFMT_DXT5)
    {
        srcStride = 16 * ((width + 3) >> 2);
        dy = 4;
        goto LABEL_20;
    }
LABEL_17:
    if (!alwaysfails)
    {
        MyAssertHandler(".\\r_image_load_common.cpp", 521, 1, va("unhandled case: %d", format));
    }
}

void __cdecl Image_Upload3D_CopyData_PC(
    const GfxImage *image,
    _D3DFORMAT format,
    unsigned int mipLevel,
    unsigned __int8 *src)
{
    const char *v4; // eax
    const char *v5; // eax
    int v6; // [esp+0h] [ebp-44h]
    int v7; // [esp+4h] [ebp-40h]
    int v8; // [esp+8h] [ebp-3Ch]
    int v9; // [esp+18h] [ebp-2Ch]
    int hr; // [esp+1Ch] [ebp-28h]
    int srcRowPitch; // [esp+24h] [ebp-20h]
    int sliceIndex; // [esp+28h] [ebp-1Ch]
    _D3DLOCKED_BOX lockedBox; // [esp+2Ch] [ebp-18h] BYREF
    int width; // [esp+38h] [ebp-Ch]
    int height; // [esp+3Ch] [ebp-8h]
    unsigned __int8 *dst; // [esp+40h] [ebp-4h]

    iassert(image);
    iassert(image->mapType == MAPTYPE_3D);

    if (image->width >> mipLevel > 1)
        v8 = image->width >> mipLevel;
    else
        v8 = 1;
    width = v8;
    if (image->height >> mipLevel > 1)
        v7 = image->height >> mipLevel;
    else
        v7 = 1;
    height = v7;
    if (image->depth >> mipLevel > 1)
        v6 = image->depth >> mipLevel;
    else
        v6 = 1;
    srcRowPitch = Image_SourceBytesPerSlice_PC(format, width, height);
    iassert(image->texture.volmap);
    do
    {
        if (r_logFile && r_logFile->current.integer)
            RB_LogPrint("image->texture.volmap->LockBox( mipLevel, &lockedBox, 0, 0 )\n");

        hr = image->texture.volmap->LockBox(mipLevel, &lockedBox, NULL, NULL);
        if (hr < 0)
        {
            do
            {
                ++g_disableRendering;
                Com_Error(
                    ERR_FATAL,
                    ".\\r_image_load_common.cpp (%i) image->texture.volmap->LockBox( mipLevel, &lockedBox, 0, 0 ) failed: %s\n",
                    709,
                    R_ErrorDescription(hr));
            } while (alwaysfails);
        }
    } while (alwaysfails);

    dst = (unsigned __int8 *)lockedBox.pBits;
    for (sliceIndex = 0; sliceIndex < v6; ++sliceIndex)
    {
        Image_Upload2D_CopyDataBlock_PC(width, height, src, format, lockedBox.RowPitch, dst);
        src += srcRowPitch;
        dst += lockedBox.SlicePitch;
    }
    do
    {
        if (r_logFile && r_logFile->current.integer)
            RB_LogPrint("image->texture.volmap->UnlockBox( mipLevel )\n");

        v9 = image->texture.volmap->UnlockBox(mipLevel);
        if (v9 < 0)
        {
            do
            {
                ++g_disableRendering;
                Com_Error(
                    ERR_FATAL,
                    ".\\r_image_load_common.cpp (%i) image->texture.volmap->UnlockBox( mipLevel ) failed: %s\n",
                    719,
                    R_ErrorDescription(v9));
            } while (alwaysfails);
        }
    } while (alwaysfails);
}

void __cdecl Image_Upload2D_CopyData_PC(
    const GfxImage *image,
    _D3DFORMAT format,
    _D3DCUBEMAP_FACES face,
    unsigned int mipLevel,
    unsigned __int8 *src)
{
    const char *v5; // eax
    const char *v6; // eax
    const char *v7; // eax
    const char *v8; // eax
    unsigned int v9; // [esp+0h] [ebp-30h]
    unsigned int v10; // [esp+4h] [ebp-2Ch]
    int v11; // [esp+10h] [ebp-20h]
    int v12; // [esp+14h] [ebp-1Ch]
    int v13; // [esp+18h] [ebp-18h]
    int hr; // [esp+1Ch] [ebp-14h]
    _D3DLOCKED_RECT lockedRect; // [esp+20h] [ebp-10h] BYREF
    unsigned int width; // [esp+28h] [ebp-8h]
    unsigned int height; // [esp+2Ch] [ebp-4h]

    if (image->width >> mipLevel > 1)
        v10 = image->width >> mipLevel;
    else
        v10 = 1;
    width = v10;
    if (image->height >> mipLevel > 1)
        v9 = image->height >> mipLevel;
    else
        v9 = 1;
    height = v9;
    if (image->mapType == MAPTYPE_2D)
    {
        iassert(image->texture.map);
        do
        {
            if (r_logFile && r_logFile->current.integer)
                RB_LogPrint("image->texture.map->LockRect( mipLevel, &lockedRect, 0, 0 )\n");

            hr = image->texture.map->LockRect(mipLevel, &lockedRect, 0, 0);
            if (hr < 0)
            {
                do
                {
                    ++g_disableRendering;
                    Com_Error(
                        ERR_FATAL,
                        ".\\r_image_load_common.cpp (%i) image->texture.map->LockRect( mipLevel, &lockedRect, 0, 0 ) failed: %s\n",
                        561,
                        R_ErrorDescription(hr));
                } while (alwaysfails);
            }
        } while (alwaysfails);
        Image_Upload2D_CopyDataBlock_PC(width, height, src, format, lockedRect.Pitch, (unsigned __int8 *)lockedRect.pBits);
        do
        {
            if (r_logFile && r_logFile->current.integer)
                RB_LogPrint("image->texture.map->UnlockRect( mipLevel )\n");

            v13 = image->texture.map->UnlockRect(mipLevel);
            if (v13 < 0)
            {
                do
                {
                    ++g_disableRendering;
                    Com_Error(
                        ERR_FATAL,
                        ".\\r_image_load_common.cpp (%i) image->texture.map->UnlockRect( mipLevel ) failed: %s\n",
                        563,
                        R_ErrorDescription(v13));
                } while (alwaysfails);
            }
        } while (alwaysfails);
    }
    else
    {
        iassert(image->mapType == MAPTYPE_CUBE);
        iassert((face == D3DCUBEMAP_FACE_POSITIVE_X || face == D3DCUBEMAP_FACE_NEGATIVE_X || face == D3DCUBEMAP_FACE_POSITIVE_Y || face == D3DCUBEMAP_FACE_NEGATIVE_Y || face == D3DCUBEMAP_FACE_POSITIVE_Z || face == D3DCUBEMAP_FACE_NEGATIVE_Z));
        iassert(image->texture.cubemap);

        do
        {
            if (r_logFile && r_logFile->current.integer)
                RB_LogPrint("image->texture.cubemap->LockRect( face, mipLevel, &lockedRect, 0, 0 )\n");
            v12 = image->texture.cubemap->LockRect(face, mipLevel, &lockedRect, 0, 0);
            if (v12 < 0)
            {
                do
                {
                    ++g_disableRendering;
                    Com_Error(
                        ERR_FATAL,
                        ".\\r_image_load_common.cpp (%i) image->texture.cubemap->LockRect( face, mipLevel, &lockedRect, 0, 0 ) failed: %s\n",
                        571,
                        R_ErrorDescription(v12));
                } while (alwaysfails);
            }
        } while (alwaysfails);
        Image_Upload2D_CopyDataBlock_PC(width, height, src, format, lockedRect.Pitch, (unsigned __int8 *)lockedRect.pBits);
        do
        {
            if (r_logFile && r_logFile->current.integer)
                RB_LogPrint("image->texture.cubemap->UnlockRect( face, mipLevel )\n");

            v11 = image->texture.cubemap->UnlockRect(face, mipLevel);
            if (v11 < 0)
            {
                do
                {
                    ++g_disableRendering;
                    Com_Error(
                        ERR_FATAL,
                        ".\\r_image_load_common.cpp (%i) image->texture.cubemap->UnlockRect( face, mipLevel ) failed: %s\n",
                        573,
                        R_ErrorDescription(v11));
                } while (alwaysfails);
            }
        } while (alwaysfails);
    }
}


int __cdecl Image_GetPlatformScreenWidth(int platform, int screenWidth)
{
    if (platform == 1)
        return 640;
    iassert(platform == PICMIP_PLATFORM_USED);
    return screenWidth;
}

int __cdecl Image_GetPlatformScreenHeight(int platform, int screenHeight)
{
    if (platform == 1)
        return 480;
    iassert(platform == PICMIP_PLATFORM_USED);
    return screenHeight;
}

void __cdecl Image_GetMipmapResolution(
    int baseWidth,
    int baseHeight,
    int mipmap,
    unsigned __int16 *mipWidth,
    unsigned __int16 *mipHeight)
{
    unsigned int v5; // [esp+0h] [ebp-10h]
    unsigned int v6; // [esp+4h] [ebp-Ch]

    iassert(baseWidth > 0);
    iassert(baseHeight > 0);
    iassert(mipmap >= 0);
    iassert(mipWidth);
    iassert(mipHeight);

    if ((int)((unsigned int)baseWidth >> mipmap) > 1)
        v6 = (unsigned int)baseWidth >> mipmap;
    else
        LOWORD(v6) = 1;

    *mipWidth = v6;
    if ((int)((unsigned int)baseHeight >> mipmap) > 1)
        v5 = (unsigned int)baseHeight >> mipmap;
    else
        LOWORD(v5) = 1;
    *mipHeight = v5;

    iassert(*mipWidth > 0);
    iassert(*mipHeight > 0);
}

void __cdecl Image_TrackFullscreenTexture(
    GfxImage *image,
    int fullscreenWidth,
    int fullscreenHeight,
    int picmip,
    _D3DFORMAT format)
{
    unsigned int memory; // [esp+0h] [ebp-18h]
    unsigned int platformHeight; // [esp+4h] [ebp-14h]
    unsigned __int16 width; // [esp+8h] [ebp-10h] BYREF
    unsigned __int16 height; // [esp+Ch] [ebp-Ch] BYREF
    int platformWidth; // [esp+10h] [ebp-8h]
    int platform; // [esp+14h] [ebp-4h]

    for (platform = 0; platform < 2; ++platform)
    {
        platformWidth = Image_GetPlatformScreenWidth(platform, fullscreenWidth);
        platformHeight = Image_GetPlatformScreenHeight(platform, fullscreenHeight);
        Image_GetMipmapResolution(platformWidth, platformHeight, picmip, &width, &height);
        memory = Image_GetCardMemoryAmount(3, format, width, height, 1u);
        if (!useFastFile->current.enabled)
            Image_TrackTotalMemory(image, platform, memory);
        image->cardMemory.platform[platform] = memory;
    }
}