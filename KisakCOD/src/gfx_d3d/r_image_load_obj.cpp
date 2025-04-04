#include "r_image.h"
#include "r_dvars.h"
#include "r_init.h"


unsigned int __cdecl Image_GetCardMemoryAmountForMipLevel(
    _D3DFORMAT format,
    unsigned int mipWidth,
    unsigned int mipHeight,
    unsigned int mipDepth)
{
    unsigned int result; // eax
    const char *v5; // eax

    if (format > D3DFMT_D16)
    {
        if (format > D3DFMT_DXT1)
        {
            if (format == D3DFMT_DXT3 || format == D3DFMT_DXT5)
                return 16 * mipDepth * ((mipHeight + 3) >> 2) * ((mipWidth + 3) >> 2);
        }
        else
        {
            if (format == D3DFMT_DXT1)
                return 8 * mipDepth * ((mipHeight + 3) >> 2) * ((mipWidth + 3) >> 2);
            if (format == D3DFMT_G16R16F || format == D3DFMT_R32F)
                return 4 * mipDepth * mipHeight * mipWidth;
        }
    LABEL_17:
        if (!alwaysfails)
        {
            v5 = va("unhandled case: %d", format);
            MyAssertHandler(".\\r_image_load_common.cpp", 170, 1, v5);
        }
        return 0;
    }
    else if (format == D3DFMT_D16)
    {
        return 2 * mipDepth * mipHeight * mipWidth;
    }
    else
    {
        switch (format)
        {
        case D3DFMT_A8R8G8B8:
        case D3DFMT_X8R8G8B8:
        case D3DFMT_D24S8:
            return 4 * mipDepth * mipHeight * mipWidth;
        case D3DFMT_A8:
        case D3DFMT_L8:
            result = mipDepth * mipHeight * mipWidth;
            break;
        case D3DFMT_A8L8:
            return 2 * mipDepth * mipHeight * mipWidth;
        default:
            goto LABEL_17;
        }
    }
    return result;
}

unsigned int __cdecl Image_GetCardMemoryAmount(
    char imageFlags,
    _D3DFORMAT format,
    unsigned int width,
    unsigned int height,
    unsigned int depth)
{
    unsigned int v6; // [esp+0h] [ebp-1Ch]
    unsigned int v7; // [esp+4h] [ebp-18h]
    unsigned int v8; // [esp+8h] [ebp-14h]
    unsigned int memory; // [esp+18h] [ebp-4h]

    memory = Image_GetCardMemoryAmountForMipLevel(format, width, height, depth);
    if ((imageFlags & 2) == 0)
    {
        while (depth + height + width > 3)
        {
            if (width >> 1 > 1)
                v8 = width >> 1;
            else
                v8 = 1;
            width = v8;
            if (height >> 1 > 1)
                v7 = height >> 1;
            else
                v7 = 1;
            height = v7;
            if (depth >> 1 > 1)
                v6 = depth >> 1;
            else
                v6 = 1;
            depth = v6;
            memory += Image_GetCardMemoryAmountForMipLevel(format, v8, v7, v6);
        }
    }
    if ((imageFlags & 4) != 0)
        memory *= 6;
    return memory;
}

void __cdecl Image_TrackTotalMemory(GfxImage *image, int platform, int memory)
{
    if (!Image_IsCodeImage(image->track))
        imageGlobals.totalMemory.platform[platform] += memory - image->cardMemory.platform[platform];
}

void __cdecl Image_TrackTexture(GfxImage *image, char imageFlags, _D3DFORMAT format, int width, int height, int depth)
{
    unsigned int CardMemoryAmount; // eax
    unsigned int v7; // [esp+0h] [ebp-2Ch]
    unsigned int v8; // [esp+4h] [ebp-28h]
    unsigned int v9; // [esp+8h] [ebp-24h]
    int memory; // [esp+18h] [ebp-14h]
    int platform; // [esp+28h] [ebp-4h]

    for (platform = 0; platform < 2; ++platform)
    {
        if ((imageFlags & 1) != 0)
        {
            CardMemoryAmount = Image_GetCardMemoryAmount(imageFlags, format, width, height, depth);
        }
        else
        {
            if (width >> image->picmip.platform[platform] > 1)
                v9 = width >> image->picmip.platform[platform];
            else
                v9 = 1;
            if (height >> image->picmip.platform[platform] > 1)
                v8 = height >> image->picmip.platform[platform];
            else
                v8 = 1;
            if (depth >> image->picmip.platform[platform] > 1)
                v7 = depth >> image->picmip.platform[platform];
            else
                v7 = 1;
            CardMemoryAmount = Image_GetCardMemoryAmount(imageFlags, format, v9, v8, v7);
        }
        memory = CardMemoryAmount;
        if (image->cardMemory.platform[platform] && image->cardMemory.platform[platform] != CardMemoryAmount)
            MyAssertHandler(
                ".\\r_image_load_common.cpp",
                131,
                0,
                "%s",
                "!image->cardMemory.platform[platform] || (image->cardMemory.platform[platform] == memory)");
        if (!useFastFile->current.enabled)
            Image_TrackTotalMemory(image, platform, memory);
        image->cardMemory.platform[platform] = memory;
    }
}

void __cdecl Image_Setup(GfxImage *image, int width, int height, int depth, int imageFlags, _D3DFORMAT imageFormat)
{
    unsigned int mipmapCount; // [esp+0h] [ebp-4h]

    if (!image)
        MyAssertHandler(".\\r_image_load_common.cpp", 1058, 0, "%s", "image");
    image->width = width;
    image->height = height;
    image->depth = depth;
    if (image->cardMemory.platform[0])
        MyAssertHandler(".\\r_image_load_common.cpp", 1065, 0, "%s", "!image->cardMemory.platform[PICMIP_PLATFORM_USED]");
    mipmapCount = (imageFlags & 2) != 0;
    if (r_loadForRenderer->current.enabled)
    {
        if ((imageFlags & 4) != 0)
        {
            Image_CreateCubeTexture_PC(image, image->width, mipmapCount, imageFormat);
        }
        else if ((imageFlags & 8) != 0)
        {
            Image_Create3DTexture_PC(image, image->width, image->height, image->depth, mipmapCount, imageFlags, imageFormat);
        }
        else
        {
            Image_Create2DTexture_PC(image, image->width, image->height, mipmapCount, imageFlags, imageFormat);
        }
        Image_TrackTexture(image, imageFlags, imageFormat, width, height, depth);
        if (image->delayLoadPixels)
            MyAssertHandler(".\\r_image_load_common.cpp", 1137, 0, "%s", "!image->delayLoadPixels");
    }
}

void __cdecl Image_SetupFromFile(GfxImage *image, const GfxImageFileHeader *fileHeader, _D3DFORMAT imageFormat)
{
    unsigned int v3; // [esp+0h] [ebp-2Ch]
    unsigned int v4; // [esp+4h] [ebp-28h]
    unsigned int v5; // [esp+8h] [ebp-24h]
    unsigned __int8 picmip; // [esp+28h] [ebp-4h]

    if (!image)
        MyAssertHandler(".\\r_image_load_obj.cpp", 130, 0, "%s", "image");
    if (!fileHeader)
        MyAssertHandler(".\\r_image_load_obj.cpp", 131, 0, "%s", "fileHeader");
    picmip = image->picmip.platform[0];
    if ((int)((unsigned int)fileHeader->dimensions[0] >> picmip) > 1)
        v5 = (unsigned int)fileHeader->dimensions[0] >> picmip;
    else
        v5 = 1;
    if ((int)((unsigned int)fileHeader->dimensions[1] >> picmip) > 1)
        v4 = (unsigned int)fileHeader->dimensions[1] >> picmip;
    else
        v4 = 1;
    if ((int)((unsigned int)fileHeader->dimensions[2] >> picmip) > 1)
        v3 = (unsigned int)fileHeader->dimensions[2] >> picmip;
    else
        v3 = 1;
    Image_Setup(image, v5, v4, v3, fileHeader->flags, imageFormat);
    if (image->cardMemory.platform[0] <= 0)
        MyAssertHandler(".\\r_image_load_obj.cpp", 142, 0, "%s", "image->cardMemory.platform[PICMIP_PLATFORM_USED] > 0");
}



void __cdecl Image_Create2DTexture_PC(
    GfxImage *image,
    unsigned __int16 width,
    unsigned __int16 height,
    unsigned int mipmapCount,
    int imageFlags,
    _D3DFORMAT imageFormat)
{
    HRESULT v6; // eax
    const char *v7; // eax
    const char *v8; // eax
    const char *v9; // eax
    HRESULT hr; // [esp+0h] [ebp-Ch]
    unsigned int usage; // [esp+4h] [ebp-8h]

    if (!image)
        MyAssertHandler(".\\r_image.cpp", 544, 0, "%s", "image");
    if (image->texture.basemap)
        MyAssertHandler(".\\r_image.cpp", 545, 0, "%s", "!image->texture.basemap");
    image->width = width;
    image->height = height;
    image->depth = 1;
    image->mapType = MAPTYPE_2D;
    usage = Image_GetUsage(imageFlags, imageFormat);
    if ((imageFlags & 0x40000) != 0)
        v6 = dx.device->CreateTexture(
            width,
            height,
            mipmapCount,
            usage,
            imageFormat,
            D3DPOOL_SYSTEMMEM,
            (IDirect3DTexture9 **)&image->texture,
            0);
    else
        v6 = dx.device->CreateTexture(
            width,
            height,
            mipmapCount,
            usage,
            imageFormat,
            (_D3DPOOL)(usage == 0),
            (IDirect3DTexture9 **)&image->texture,
            0);
    hr = v6;
    if (v6 < 0)
    {
        v7 = R_ErrorDescription(v6);
        Com_Error(
            ERR_DROP,
            "Create2DTexture( %s, %i, %i, %i, %i ) failed: %08x = %s",
            image->name,
            image->width,
            image->height,
            0,
            imageFormat,
            hr,
            v7);
    }
    if (hr != -2005530520 && !image->texture.basemap)
    {
        v8 = R_ErrorDescription(hr);
        v9 = va(
            "DirectX succeeded without creating texture for %s: size %ix%i, type %08x, hr %08x = %s",
            image->name,
            image->width,
            image->height,
            imageFormat,
            hr,
            v8);
        MyAssertHandler(".\\r_image.cpp", 562, 0, "%s\n\t%s", "hr == D3DERR_DEVICELOST || image->texture.map", v9);
    }
}


void __cdecl Image_Create2DTexture_PC(
        GfxImage *image,
        unsigned __int16 width,
        unsigned __int16 height,
        unsigned int mipmapCount,
        int imageFlags,
        _D3DFORMAT imageFormat)
{
  HRESULT v6; // eax
  const char *v7; // eax
  const char *v8; // eax
  const char *v9; // eax
  HRESULT hr; // [esp+0h] [ebp-Ch]
  unsigned int usage; // [esp+4h] [ebp-8h]

  if ( !image )
    MyAssertHandler(".\\r_image.cpp", 544, 0, "%s", "image");
  if ( image->texture.basemap )
    MyAssertHandler(".\\r_image.cpp", 545, 0, "%s", "!image->texture.basemap");
  image->width = width;
  image->height = height;
  image->depth = 1;
  image->mapType = MAPTYPE_2D;
  usage = Image_GetUsage(imageFlags, imageFormat);
  if ( (imageFlags & 0x40000) != 0 )
    v6 = dx.device->CreateTexture(
           width,
           height,
           mipmapCount,
           usage,
           imageFormat,
           D3DPOOL_SYSTEMMEM,
           (IDirect3DTexture9 **)&image->texture,
           0);
  else
    v6 = dx.device->CreateTexture(
           width,
           height,
           mipmapCount,
           usage,
           imageFormat,
           (_D3DPOOL)(usage == 0),
           (IDirect3DTexture9 **)&image->texture,
           0);
  hr = v6;
  if ( v6 < 0 )
  {
    v7 = R_ErrorDescription(v6);
    Com_Error(
      ERR_DROP,
      "Create2DTexture( %s, %i, %i, %i, %i ) failed: %08x = %s",
      image->name,
      image->width,
      image->height,
      0,
      imageFormat,
      hr,
      v7);
  }
  if ( hr != -2005530520 && !image->texture.basemap )
  {
    v8 = R_ErrorDescription(hr);
    v9 = va(
           "DirectX succeeded without creating texture for %s: size %ix%i, type %08x, hr %08x = %s",
           image->name,
           image->width,
           image->height,
           imageFormat,
           hr,
           v8);
    MyAssertHandler(".\\r_image.cpp", 562, 0, "%s\n\t%s", "hr == D3DERR_DEVICELOST || image->texture.map", v9);
  }
}