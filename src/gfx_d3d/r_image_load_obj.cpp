#include "r_image.h"
#include "r_dvars.h"
#include "r_init.h"
#include "r_utils.h"
#include <universal/com_files.h>

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
        iassert( s_imageLoadBuf );
    }
    mem = &s_imageLoadBuf[s_imageLoadBytesUsed];
    s_imageLoadBytesUsed += bytesa;
    return mem;
}

void __cdecl Image_FreeTempMemory(unsigned __int8 *mem, int bytes)
{
    bytes = (bytes + 3) & 0xFFFFFFFC;
    iassert( mem + bytes == s_imageLoadBuf + s_imageLoadBytesUsed );
    s_imageLoadBytesUsed -= bytes;
}

void __cdecl Image_Generate2D(GfxImage *image, unsigned __int8 *pixels, int width, int height, _D3DFORMAT imageFormat)
{
    iassert( pixels );
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
    iassert( image->cardMemory.platform[PICMIP_PLATFORM_USED] > 0 );
    Image_UploadData(image, imageFormat, D3DCUBEMAP_FACE_POSITIVE_X, 0, pixels);
}

void __cdecl Image_ExpandBgr(const unsigned __int8 *src, unsigned int count, unsigned __int8 *dst)
{
    iassert( src );
    iassert( dst );
    iassert( (count > 0) );
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
        if (!IsFastFileLoad())
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

    iassert( image );
    iassert( fileHeader );
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
    iassert( image->cardMemory.platform[PICMIP_PLATFORM_USED] > 0 );
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
    iassert( pixels );
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
    iassert( image->cardMemory.platform[PICMIP_PLATFORM_USED] > 0 );
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

    iassert( pixels );
    iassert( edgeLen > 0 );
    iassert( IsPowerOf2( edgeLen ) );
    iassert( mipCount <= 15 );
    imageFlags = 5;
    if (mipCount == 1)
        imageFlags = 7;
    Image_Setup(image, edgeLen, edgeLen, 1, imageFlags, imageFormat);
    iassert( image->cardMemory.platform[PICMIP_PLATFORM_USED] > 0 );
    for (faceIndex = 0; faceIndex < 6; ++faceIndex)
    {
        face = (_D3DCUBEMAP_FACES)Image_CubemapFace(faceIndex);
        for (mipIndex = 0; mipIndex < mipCount; ++mipIndex)
            Image_UploadData(image, imageFormat, face, mipIndex, (unsigned __int8 *)(&(*pixels)[15 * faceIndex])[mipIndex]);
    }
}

void __cdecl Image_BuildWaterMap(GfxImage *image)
{
    iassert( image );
    Image_SetupAndLoad(image, image->width, image->height, 1, 65537, D3DFMT_L8);
}

void __cdecl Image_LoadDxtc(
    GfxImage *image,
    const GfxImageFileHeader *fileHeader,
    const unsigned __int8 *data,
    _D3DFORMAT format,
    int bytesPerBlock)
{
    unsigned int v5; // [esp+0h] [ebp-34h]
    unsigned int v6; // [esp+4h] [ebp-30h]
    int mipCount; // [esp+14h] [ebp-20h]
    _D3DCUBEMAP_FACES face; // [esp+20h] [ebp-14h]
    unsigned int faceCount; // [esp+24h] [ebp-10h]
    unsigned int faceIndex; // [esp+28h] [ebp-Ch]
    int mipLevel; // [esp+2Ch] [ebp-8h]
    int picmip; // [esp+30h] [ebp-4h]

    iassert( image );
    iassert( fileHeader );
    iassert( data );
    if (format != D3DFMT_DXT1 && format != D3DFMT_DXT3 && format != D3DFMT_DXT5)
        MyAssertHandler(
            ".\\r_image_load_obj.cpp",
            391,
            0,
            "%s",
            "format == GFX_PF_DXT1 || format == GFX_PF_DXT3 || format == GFX_PF_DXT5");
    //iassert( bytesPerBlock == (format == GFX_PF_DXT1 ? 8 : 16) );
    iassert( bytesPerBlock == (format == D3DFMT_DXT1 ? 8 : 16) );
    Image_SetupFromFile(image, fileHeader, format);
    if (image->mapType == MAPTYPE_CUBE)
        faceCount = 6;
    else
        faceCount = 1;
    mipCount = Image_CountMipmapsForFile(fileHeader);
    picmip = image->picmip.platform[0];
    for (mipLevel = mipCount - 1; mipLevel >= picmip; --mipLevel)
    {
        if ((int)((unsigned int)fileHeader->dimensions[0] >> mipLevel) > 1)
            v6 = (unsigned int)fileHeader->dimensions[0] >> mipLevel;
        else
            v6 = 1;
        if ((int)((unsigned int)fileHeader->dimensions[1] >> mipLevel) > 1)
            v5 = (unsigned int)fileHeader->dimensions[1] >> mipLevel;
        else
            v5 = 1;
        for (faceIndex = 0; faceIndex < faceCount; ++faceIndex)
        {
            face = (_D3DCUBEMAP_FACES)Image_CubemapFace(faceIndex);
            Image_UploadData(image, format, face, mipLevel - picmip, (unsigned char *)data);
            data += bytesPerBlock * ((int)(v5 + 3) >> 2) * ((int)(v6 + 3) >> 2);
        }
    }
}

static GfxImage *__cdecl Image_Load(char *name, unsigned __int8 semantic, unsigned __int8 imageTrack)
{
    GfxImage *image; // [esp+0h] [ebp-4h]

    if (*name == 36)
        return Image_LoadBuiltin(name, semantic, imageTrack);
    image = Image_Alloc(name, 3u, semantic, imageTrack);
    iassert( image );
    iassert( image->texture.basemap == NULL );
    if (Image_LoadFromFile(image))
        return image;
    else
        return 0;
}

static void __cdecl Image_PrintTruncatedFileError(const char *filepath)
{
    Com_PrintError(8, "ERROR: image '%s' is truncated.  Delete the file and run converter to fix.\n", filepath);
}

char __cdecl Image_LoadFromFileWithReader(GfxImage *image, int(__cdecl *OpenFileRead)(const char *, int *))
{
    int v3; // eax
    int v4; // [esp+0h] [ebp-84h]
    unsigned __int8 *imageData; // [esp+Ch] [ebp-78h]
    GfxImageFileHeader fileHeader; // [esp+10h] [ebp-74h] BYREF
    int fileSize; // [esp+2Ch] [ebp-58h]
    int fileHandle; // [esp+30h] [ebp-54h] BYREF
    char filepath[68]; // [esp+34h] [ebp-50h] BYREF
    int picmip; // [esp+7Ch] [ebp-8h]
    int readSize; // [esp+80h] [ebp-4h]

    iassert( image );
    iassert( image->category == IMG_CATEGORY_LOAD_FROM_FILE );
    iassert( !image->texture.basemap );
    if (Com_sprintf(filepath, 64, "%s%s%s", "images/", image->name, ".iwi") >= 0)
    {
        fileSize = OpenFileRead(filepath, &fileHandle);
        if (fileSize >= 0)
        {
            if ((unsigned int)fileSize < 0x1C)
                MyAssertHandler(
                    ".\\r_image_load_obj.cpp",
                    659,
                    0,
                    "%s\n\t(filepath) = %s",
                    "(fileSize >= sizeof( fileHeader ))",
                    filepath);
            if (FS_Read((unsigned __int8 *)&fileHeader, sizeof(GfxImageFileHeader), fileHandle) == sizeof(GfxImageFileHeader))
            {
                if (Image_ValidateHeader(&fileHeader, filepath))
                {
                    if ((fileHeader.flags & 3) != 0
                        || (fileHeader.dimensions[1] < fileHeader.dimensions[0]
                            ? (v4 = fileHeader.dimensions[1])
                            : (v4 = fileHeader.dimensions[0]),
                            v4 < 32))
                    {
                        image->noPicmip = 1;
                    }
                    Image_GetPicmip(image, &image->picmip);
                    picmip = image->picmip.platform[0];
                    if (fileHeader.fileSizeForPicmip[0] != fileSize)
                        MyAssertHandler(
                            ".\\r_image_load_obj.cpp",
                            684,
                            0,
                            "fileHeader.fileSizeForPicmip[0] == fileSize\n\t%i, %i",
                            fileHeader.fileSizeForPicmip[0],
                            fileSize);
                    readSize = fileHeader.fileSizeForPicmip[picmip] - 28;
                    imageData = Image_AllocTempMemory(readSize);
                    if (FS_Read(imageData, readSize, fileHandle) == readSize)
                    {
                        FS_FCloseFile(fileHandle);
                        Image_LoadFromData(image, &fileHeader, imageData);
                        Image_FreeTempMemory(imageData, readSize);
                        return 1;
                    }
                    else
                    {
                        Image_PrintTruncatedFileError(filepath);
                        Image_FreeTempMemory(imageData, readSize);
                        FS_FCloseFile(fileHandle);
                        return 0;
                    }
                }
                else
                {
                    FS_FCloseFile(fileHandle);
                    return 0;
                }
            }
            else
            {
                Image_PrintTruncatedFileError(filepath);
                FS_FCloseFile(fileHandle);
                return 0;
            }
        }
        else
        {
            Com_PrintError(8, "ERROR: image '%s' is missing\n", filepath);
            return 0;
        }
    }
    else
    {
        Com_PrintError(8, "ERROR: filename '%s' too long\n", filepath);
        return 0;
    }
}

void __cdecl Image_LoadBitmap(
    GfxImage *image,
    const GfxImageFileHeader *fileHeader,
    unsigned __int8 *data,
    _D3DFORMAT format,
    int bytesPerPixel)
{
    unsigned int v5; // [esp+0h] [ebp-3Ch]
    unsigned int v6; // [esp+4h] [ebp-38h]
    unsigned int mipCount; // [esp+14h] [ebp-28h]
    _D3DCUBEMAP_FACES face; // [esp+20h] [ebp-1Ch]
    unsigned int faceCount; // [esp+24h] [ebp-18h]
    unsigned int faceIndex; // [esp+28h] [ebp-14h]
    unsigned __int8 *expandedData; // [esp+2Ch] [ebp-10h]
    int mipLevel; // [esp+30h] [ebp-Ch]
    int picmip; // [esp+34h] [ebp-8h]
    unsigned int expandedSize; // [esp+38h] [ebp-4h]

    iassert( image );
    iassert( fileHeader );
    iassert( data );
    Image_SetupFromFile(image, fileHeader, format);
    if (image->mapType == MAPTYPE_CUBE)
        faceCount = 6;
    else
        faceCount = 1;
    expandedData = 0;
    expandedSize = 4 * image->height * image->width;
    if (format == D3DFMT_X8R8G8B8)
        expandedData = Image_AllocTempMemory(expandedSize);
    mipCount = Image_CountMipmapsForFile(fileHeader);
    picmip = image->picmip.platform[0];
    for (mipLevel = mipCount - 1; mipLevel >= picmip; --mipLevel)
    {
        if ((int)((unsigned int)fileHeader->dimensions[0] >> mipLevel) > 1)
            v6 = (unsigned int)fileHeader->dimensions[0] >> mipLevel;
        else
            v6 = 1;
        if ((int)((unsigned int)fileHeader->dimensions[1] >> mipLevel) > 1)
            v5 = (unsigned int)fileHeader->dimensions[1] >> mipLevel;
        else
            v5 = 1;
        for (faceIndex = 0; faceIndex < faceCount; ++faceIndex)
        {
            face = (_D3DCUBEMAP_FACES)Image_CubemapFace(faceIndex);
            if (format == D3DFMT_X8R8G8B8)
            {
                Image_ExpandBgr(data, v5 * v6, expandedData);
                Image_UploadData(image, D3DFMT_X8R8G8B8, face, mipLevel - picmip, expandedData);
            }
            else
            {
                if (format == D3DFMT_A8R8G8B8)
                    KISAK_NULLSUB();
                Image_UploadData(image, format, face, mipLevel - picmip, data);
            }
            data += bytesPerPixel * v5 * v6;
        }
    }
    if (expandedData)
        Image_FreeTempMemory(expandedData, expandedSize);
}

void __cdecl Image_LoadFromData(GfxImage *image, GfxImageFileHeader *fileHeader, unsigned __int8 *srcData)
{
    const char *v3; // eax

    image->texture.basemap = 0;
    switch (fileHeader->format)
    {
    case 1u:
        Image_LoadBitmap(image, fileHeader, srcData, D3DFMT_A8R8G8B8, 4);
        break;
    case 2u:
        Image_LoadBitmap(image, fileHeader, srcData, D3DFMT_X8R8G8B8, 3);
        break;
    case 3u:
        Image_LoadBitmap(image, fileHeader, srcData, D3DFMT_A8L8, 2);
        break;
    case 4u:
        Image_LoadBitmap(image, fileHeader, srcData, D3DFMT_L8, 1);
        break;
    case 5u:
        Image_LoadBitmap(image, fileHeader, srcData, D3DFMT_A8, 1);
        break;
    case 6u:
        Image_LoadWavelet(image, fileHeader, srcData, D3DFMT_A8R8G8B8, 4);
        break;
    case 7u:
        Image_LoadWavelet(image, fileHeader, srcData, D3DFMT_X8R8G8B8, 3);
        break;
    case 8u:
        Image_LoadWavelet(image, fileHeader, srcData, D3DFMT_A8L8, 2);
        break;
    case 9u:
        Image_LoadWavelet(image, fileHeader, srcData, D3DFMT_L8, 1);
        break;
    case 10u:
        Image_LoadWavelet(image, fileHeader, srcData, D3DFMT_A8, 1);
        break;
    case 11u:
        Image_LoadDxtc(image, fileHeader, srcData, D3DFMT_DXT1, 8);
        break;
    case 12u:
        Image_LoadDxtc(image, fileHeader, srcData, D3DFMT_DXT3, 16);
        break;
    case 13u:
        Image_LoadDxtc(image, fileHeader, srcData, D3DFMT_DXT5, 16);
        break;
    default:
        if (!alwaysfails)
        {
            v3 = va("unhandled case: %d", fileHeader->format);
            MyAssertHandler(".\\r_image_load_obj.cpp", 522, 1, v3);
        }
        break;
    }
}

GfxImage *__cdecl Image_Register_LoadObj(char *imageName, unsigned __int8 semantic, unsigned __int8 imageTrack)
{
    GfxImage *image; // [esp+0h] [ebp-4h]
    GfxImage *imagea; // [esp+0h] [ebp-4h]

    image = Image_FindExisting(imageName);
    if (image)
        return image;
    ProfLoad_Begin("Load image");
    imagea = Image_Load(imageName, semantic, imageTrack);
    ProfLoad_End();
    if (!imagea)
        Com_PrintError(8, "ERROR: failed to load image '%s'\n", imageName);
    return imagea;
}