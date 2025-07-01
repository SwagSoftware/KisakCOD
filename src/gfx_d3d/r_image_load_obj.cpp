#include "r_image.h"
#include "r_dvars.h"
#include "r_init.h"
#include "r_utils.h"

unsigned __int8 *s_imageLoadBuf;
unsigned int s_imageLoadBytesUsed;

unsigned __int8 *__cdecl Image_AllocTempMemory(int bytes)
{
    unsigned __int8 *mem; // [esp+10h] [ebp-4h]
    unsigned int bytesa; // [esp+1Ch] [ebp+8h]

    bytesa = (bytes + 3) & 0xFFFFFFFC;
    if (bytesa + s_imageLoadBytesUsed > 0x600000)
        Com_Error(
            ERR_DROP,
            "Needed to allocate at least %.1f MB to load images",
            (double)(bytesa + s_imageLoadBytesUsed) * 0.00000095367431640625);
    if (!s_imageLoadBuf)
    {
        s_imageLoadBuf = (unsigned __int8 *)Z_VirtualAlloc(6291456, "Image_AllocTempMemory", 18);
        if (!s_imageLoadBuf)
            MyAssertHandler(".\\r_image_load_obj.cpp", 207, 0, "%s", "s_imageLoadBuf");
    }
    mem = &s_imageLoadBuf[s_imageLoadBytesUsed];
    s_imageLoadBytesUsed += bytesa;
    return mem;
}

void __cdecl Image_FreeTempMemory(unsigned __int8 *mem, int bytes)
{
    unsigned int bytesa; // [esp+Ch] [ebp+Ch]

    bytesa = (bytes + 3) & 0xFFFFFFFC;
    if (&mem[bytesa] != &s_imageLoadBuf[s_imageLoadBytesUsed])
        MyAssertHandler(".\\r_image_load_obj.cpp", 224, 0, "%s", "mem + bytes == s_imageLoadBuf + s_imageLoadBytesUsed");
    s_imageLoadBytesUsed -= bytesa;
}

void __cdecl Image_Generate2D(GfxImage *image, unsigned __int8 *pixels, int width, int height, _D3DFORMAT imageFormat)
{
    if (!pixels)
        MyAssertHandler(".\\r_image_load_obj.cpp", 860, 0, "%s", "pixels");
    if (width <= 0 || (width & (width - 1)) != 0)
        MyAssertHandler(
            ".\\r_image_load_obj.cpp",
            861,
            0,
            "%s\n\t(width) = %i",
            "(width > 0 && (((width) & ((width) - 1)) == 0))",
            width);
    if (height <= 0 || (height & (height - 1)) != 0)
        MyAssertHandler(
            ".\\r_image_load_obj.cpp",
            862,
            0,
            "%s\n\t(height) = %i",
            "(height > 0 && (((height) & ((height) - 1)) == 0))",
            height);
    Image_Setup(image, width, height, 1, 3, imageFormat);
    if (image->cardMemory.platform[0] <= 0)
        MyAssertHandler(".\\r_image_load_obj.cpp", 867, 0, "%s", "image->cardMemory.platform[PICMIP_PLATFORM_USED] > 0");
    Image_UploadData(image, imageFormat, D3DCUBEMAP_FACE_POSITIVE_X, 0, pixels);
}

void __cdecl Image_ExpandBgr(const unsigned __int8 *src, unsigned int count, unsigned __int8 *dst)
{
    if (!src)
        MyAssertHandler(".\\r_image_load_obj.cpp", 160, 0, "%s", "src");
    if (!dst)
        MyAssertHandler(".\\r_image_load_obj.cpp", 161, 0, "%s", "dst");
    if (!count)
        MyAssertHandler(".\\r_image_load_obj.cpp", 162, 0, "%s\n\t(count) = %i", "(count > 0)", 0);
    do
    {
        *dst = *src;
        dst[1] = src[1];
        dst[2] = src[2];
        dst[3] = -1;
        dst += 4;
        src += 3;
        --count;
    } while (count);
}

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

GfxImage *__cdecl Image_FindExisting_LoadObj(const char *name)
{
    GfxImage *image; // [esp+14h] [ebp-8h]
    int hashIndex; // [esp+18h] [ebp-4h]

    hashIndex = R_HashAssetName(name) & 0x7FF;
    for (image = imageGlobals.imageHashTable[hashIndex];
        image && strcmp(name, image->name);
        image = imageGlobals.imageHashTable[hashIndex])
    {
        hashIndex = ((_WORD)hashIndex + 1) & 0x7FF;
    }
    return !Image_IsProg(image) ? image : 0;
}

bool __cdecl Image_IsProg(GfxImage *image)
{
    return image >= g_imageProgs && image < (GfxImage *)&imageGlobals;
}

void __cdecl Image_Generate3D(
    GfxImage *image,
    unsigned __int8 *pixels,
    int width,
    int height,
    int depth,
    _D3DFORMAT imageFormat)
{
    if (!pixels)
        MyAssertHandler(".\\r_image_load_obj.cpp", 895, 0, "%s", "pixels");
    if (width <= 0 || (width & (width - 1)) != 0)
        MyAssertHandler(
            ".\\r_image_load_obj.cpp",
            896,
            0,
            "%s\n\t(width) = %i",
            "(width > 0 && (((width) & ((width) - 1)) == 0))",
            width);
    if (height <= 0 || (height & (height - 1)) != 0)
        MyAssertHandler(
            ".\\r_image_load_obj.cpp",
            897,
            0,
            "%s\n\t(height) = %i",
            "(height > 0 && (((height) & ((height) - 1)) == 0))",
            height);
    if (depth <= 0 || (depth & (depth - 1)) != 0)
        MyAssertHandler(
            ".\\r_image_load_obj.cpp",
            898,
            0,
            "%s\n\t(depth) = %i",
            "(depth > 0 && (((depth) & ((depth) - 1)) == 0))",
            depth);
    Image_Setup(image, width, height, depth, 11, imageFormat);
    if (image->cardMemory.platform[0] <= 0)
        MyAssertHandler(".\\r_image_load_obj.cpp", 903, 0, "%s", "image->cardMemory.platform[PICMIP_PLATFORM_USED] > 0");
    Image_UploadData(image, imageFormat, D3DCUBEMAP_FACE_POSITIVE_X, 0, pixels);
}

void __cdecl Image_GenerateCube(
    GfxImage *image,
    const unsigned __int8 *(*pixels)[15],
    int edgeLen,
    _D3DFORMAT imageFormat,
    unsigned int mipCount)
{
    _D3DCUBEMAP_FACES face; // [esp+0h] [ebp-10h]
    unsigned int mipIndex; // [esp+4h] [ebp-Ch]
    unsigned int faceIndex; // [esp+8h] [ebp-8h]
    unsigned __int8 imageFlags; // [esp+Fh] [ebp-1h]

    if (!pixels)
        MyAssertHandler(".\\r_image_load_obj.cpp", 936, 0, "%s", "pixels");
    if (edgeLen <= 0)
        MyAssertHandler(".\\r_image_load_obj.cpp", 937, 0, "%s", "edgeLen > 0");
    if ((edgeLen & (edgeLen - 1)) != 0)
        MyAssertHandler(".\\r_image_load_obj.cpp", 938, 0, "%s", "IsPowerOf2( edgeLen )");
    if (mipCount > 0xF)
        MyAssertHandler(".\\r_image_load_obj.cpp", 939, 0, "%s", "mipCount <= 15");
    imageFlags = 5;
    if (mipCount == 1)
        imageFlags = 7;
    Image_Setup(image, edgeLen, edgeLen, 1, imageFlags, imageFormat);
    if (image->cardMemory.platform[0] <= 0)
        MyAssertHandler(".\\r_image_load_obj.cpp", 948, 0, "%s", "image->cardMemory.platform[PICMIP_PLATFORM_USED] > 0");
    for (faceIndex = 0; faceIndex < 6; ++faceIndex)
    {
        face = (_D3DCUBEMAP_FACES)Image_CubemapFace(faceIndex);
        for (mipIndex = 0; mipIndex < mipCount; ++mipIndex)
            Image_UploadData(image, imageFormat, face, mipIndex, (unsigned __int8 *)(&(*pixels)[15 * faceIndex])[mipIndex]);
    }
}

void __cdecl Image_BuildWaterMap(GfxImage *image)
{
    if (!image)
        MyAssertHandler(".\\r_image_load_obj.cpp", 1149, 0, "%s", "image");
    Image_SetupAndLoad(image, image->width, image->height, 1, 65537, D3DFMT_L8);
}