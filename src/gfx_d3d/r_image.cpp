#include "r_image.h"
#include <qcommon/mem_track.h>
#include <qcommon/qcommon.h>
#include <universal/com_memory.h>
#include <qcommon/cmd.h>
#include <database/database.h>
#include "r_init.h"
#include "r_dvars.h"
#include <universal/com_files.h>
#include "rb_logfile.h"
#include <universal/profile.h>
#include "r_pixelcost_load_obj.h"
#include "r_utils.h"
#include "r_texturemem.h"
#include "rb_state.h"
#include "r_state.h"

#include <algorithm>

//ImgGlobals imageGlobals; // LWSS: moved to db_registry for DEDICATED
GfxImage g_imageProgs[14];

struct BuiltinImageConstructorTable // sizeof=0x8
{                                       // ...
    const char *name;                   // ...
    void(__cdecl *LoadCallback)(GfxImage *); // ...
};
const BuiltinImageConstructorTable constructorTable[8] =
{
    {"$white", Image_LoadWhite},
    {"$black", Image_LoadBlack},
    {"$black_3d", Image_LoadBlack3D},
    {"$black_cube", Image_LoadBlackCube},
    {"$gray", Image_LoadGray},
    {"$identitynormalmap", Image_LoadIdentityNormalMap},
    {"$outdoor", (void(__cdecl *)(GfxImage *))Outdoor_SetRendererOutdoorLookupMatrix},
    {"$pixelcostcolorcode", Image_LoadPixelCostColorCode}
};

void __cdecl TRACK_r_image()
{
    track_static_alloc_internal(g_imageProgs, 504, "g_imageProgs", 18);
    track_static_alloc_internal(imageTypeName, 40, "imageTypeName", 18);
}

void __cdecl R_DelayLoadImage(XAssetHeader header)
{
    LONG externalDataSize; // [esp+4h] [ebp-8h]
    HRESULT hr; // [esp+8h] [ebp-4h]

    if (HIBYTE(header.xmodelPieces[2].numpieces))
    {
        HIBYTE(header.xmodelPieces[2].numpieces) = 0;
        externalDataSize = header.xmodelPieces[1].numpieces;
        header.xmodelPieces[1].numpieces = 0;
        header.xmodelPieces[1].pieces = 0;
        if (r_loadForRenderer->current.enabled && !dx.deviceLost)
        {
            if (!Image_LoadFromFile(header.image))
                Image_AssignDefaultTexture(header.image);
            if (!header.xmodelPieces->numpieces)
            {
                hr = dx.device->TestCooperativeLevel();
                if (hr != 0x88760868 && hr != 0x88760869)
                    Com_Error(ERR_DROP, "Couldn't load image '%s'\n", header.xmodelPieces[2].pieces);
            }
        }
        DB_LoadedExternalData(externalDataSize);
    }
}

void __cdecl R_GetImageList(ImageList *imageList)
{
    if (!imageList)
        MyAssertHandler(".\\r_image.cpp", 173, 0, "%s", "imageList");
    imageList->count = 0;
    DB_EnumXAssets(ASSET_TYPE_IMAGE, (void(__cdecl *)(XAssetHeader, void *))R_AddImageToList, imageList, 1);
}

void __cdecl R_AddImageToList(XAssetHeader header, ImageList* data)
{
    if (data->count >= 0x800)
        MyAssertHandler(".\\r_image.cpp", 165, 0, "%s", "imageList->count < ARRAY_COUNT( imageList->image )");
    data->image[data->count++] = header.image;
}

void __cdecl R_SumOfUsedImages(Image_MemUsage *usage)
{
    const char *v1; // eax
    GfxImage *v2; // [esp+0h] [ebp-2040h]
    unsigned int v3[4]; // [esp+4h] [ebp-203Ch] BYREF
    int v4; // [esp+14h] [ebp-202Ch]
    int v5; // [esp+18h] [ebp-2028h]
    int v6; // [esp+1Ch] [ebp-2024h]
    int v7; // [esp+20h] [ebp-2020h]
    int v8; // [esp+24h] [ebp-201Ch]
    int v9; // [esp+28h] [ebp-2018h]
    int v10; // [esp+2Ch] [ebp-2014h]
    unsigned int i; // [esp+30h] [ebp-2010h]
    int v12; // [esp+34h] [ebp-200Ch]
    ImageList imageList; // [esp+38h] [ebp-2008h] BYREF

    if (!usage)
        MyAssertHandler(".\\r_image.cpp", 190, 0, "%s", "usage");
    R_GetImageList(&imageList);
    memset(v3, 0, sizeof(v3));
    v4 = 0;
    v5 = 0;
    v6 = 0;
    v7 = 0;
    v8 = 0;
    v9 = 0;
    v12 = 0;
    for (i = 0; i < imageList.count; ++i)
    {
        v2 = imageList.image[i];
        if (!v2)
            MyAssertHandler(".\\r_image.cpp", 202, 0, "%s", "image");
        v10 = v2->cardMemory.platform[0];
        v3[v2->track] += v10;
        if (!Image_IsCodeImage(v2->track))
            v12 += v10;
    }
    usage->total = v12;
    usage->lightmap = v4;
    if (!dx.deviceLost && usage->total != imageGlobals.totalMemory.platform[0])
    {
        v1 = va("%i != %i", usage->total, imageGlobals.totalMemory.platform[0]);
        MyAssertHandler(
            ".\\r_image.cpp",
            223,
            0,
            "%s\n\t%s",
            "dx.deviceLost || usage->total == imageGlobals.totalMemory.platform[PICMIP_PLATFORM_USED]",
            v1);
    }
    usage->minspec = imageGlobals.totalMemory.platform[1];
}

void __cdecl Image_Release(GfxImage *image)
{
    int platform; // [esp+0h] [ebp-4h]

    if (!image)
        MyAssertHandler(".\\r_image.cpp", 255, 0, "%s", "image");
    if (!Image_IsCodeImage(image->track))
    {
        for (platform = 0; platform < 2; ++platform)
            imageGlobals.totalMemory.platform[platform] -= image->cardMemory.platform[platform];
    }
    if (image->texture.basemap)
    {
        //image->texture.basemap->Release(image->texture.basemap);
        image->texture.basemap->Release();
        image->texture.basemap = 0;
        image->cardMemory.platform[0] = 0;
        image->cardMemory.platform[1] = 0;
    }
    else if (r_loadForRenderer->current.enabled)
    {
        if (image->cardMemory.platform[0])
            MyAssertHandler(".\\r_image.cpp", 292, 0, "%s", "!image->cardMemory.platform[PICMIP_PLATFORM_USED]");
    }
}

GfxImage *__cdecl Image_AllocProg(int imageProgType, unsigned __int8 category, unsigned __int8 semantic)
{
    GfxImage *image; // [esp+0h] [ebp-Ch]
    const char *name; // [esp+4h] [ebp-8h]

    image = &g_imageProgs[imageProgType];
    if (!image)
        MyAssertHandler(".\\r_image.cpp", 363, 1, "%s", "image");
    name = g_imageProgNames[imageProgType];
    image->name = name;
    if (!category)
        MyAssertHandler(".\\r_image.cpp", 368, 0, "%s", "category != IMG_CATEGORY_UNKNOWN");
    image->category = category;
    image->semantic = semantic;
    image->track = 0;
    imageGlobals.imageHashTable[Image_GetAvailableHashLocation(name)] = image;
    return &g_imageProgs[imageProgType];
}

void __cdecl Image_SetupAndLoad(
    GfxImage *image,
    int width,
    int height,
    int depth,
    int imageFlags,
    _D3DFORMAT imageFormat)
{
    Image_Setup(image, width, height, depth, imageFlags, imageFormat);
}

void __cdecl R_ShutdownImages()
{
    GfxImage *image; // [esp+0h] [ebp-2014h]
    GfxImage *imagea; // [esp+0h] [ebp-2014h]
    int v2; // [esp+4h] [ebp-2010h]
    unsigned int i; // [esp+8h] [ebp-200Ch]
    _DWORD v4[2049]; // [esp+Ch] [ebp-2008h]
    int j; // [esp+2010h] [ebp-4h]

    RB_UnbindAllImages();
    v2 = 0;
    for (i = 0; i < 0x800; ++i)
    {
        image = imageGlobals.imageHashTable[i];
        if (image)
        {
            if (Image_IsProg(image))
                v4[v2++] = (_DWORD)image;
            else
                Image_Free(imageGlobals.imageHashTable[i]);
        }
    }
    memset((unsigned __int8 *)&imageGlobals, 0, 0x2000u);
    for (j = 0; j < v2; ++j)
    {
        imagea = (GfxImage *)v4[j];
        imageGlobals.imageHashTable[Image_GetAvailableHashLocation(imagea->name)] = imagea;
    }
}

void __cdecl Image_SetupRenderTarget(
    GfxImage *image,
    unsigned __int16 width,
    unsigned __int16 height,
    _D3DFORMAT imageFormat)
{
    if (!image)
        MyAssertHandler(".\\r_image.cpp", 629, 0, "%s", "image");
    if (image->semantic)
        MyAssertHandler(".\\r_image.cpp", 630, 0, "%s", "image->semantic == TS_2D");
    Image_SetupAndLoad(image, width, height, 1, 131075, imageFormat);
}

void __cdecl Load_Texture(GfxTexture *remoteLoadDef, GfxImage *image)
{
    unsigned int mipDepth; // [esp+0h] [ebp-60h]
    unsigned int mipHeight; // [esp+4h] [ebp-5Ch]
    unsigned int mipWidth; // [esp+8h] [ebp-58h]
    _D3DCUBEMAP_FACES v5; // [esp+Ch] [ebp-54h]
    unsigned __int16 v6; // [esp+14h] [ebp-4Ch]
    unsigned __int16 v7; // [esp+18h] [ebp-48h]
    GfxImageLoadDef *loadDef; // [esp+34h] [ebp-2Ch]
    LONG externalDataSize; // [esp+38h] [ebp-28h]
    signed int mipCount; // [esp+3Ch] [ebp-24h]
    unsigned char *data; // [esp+40h] [ebp-20h]
    int faceCount; // [esp+50h] [ebp-10h]
    signed int faceIndex; // [esp+54h] [ebp-Ch]
    _D3DFORMAT imageFormat; // [esp+58h] [ebp-8h]
    signed int mipLevel; // [esp+5Ch] [ebp-4h]

    loadDef = remoteLoadDef->loadDef;
    iassert(loadDef == image->texture.loadDef);

    image->texture.basemap = 0;
    if (r_loadForRenderer->current.enabled)
    {
        imageFormat = loadDef->format;
        if (loadDef->resourceSize)
        {
            image->delayLoadPixels = 0;
            if (image->mapType == MAPTYPE_2D)
            {
                Image_Create2DTexture_PC(
                    image,
                    loadDef->dimensions[0],
                    loadDef->dimensions[1],
                    loadDef->levelCount,
                    0,
                    imageFormat);
                faceCount = 1;
            }
            else if (image->mapType == MAPTYPE_3D)
            {
                Image_Create3DTexture_PC(
                    image,
                    loadDef->dimensions[0],
                    loadDef->dimensions[1],
                    loadDef->dimensions[2],
                    loadDef->levelCount,
                    0,
                    imageFormat);
                faceCount = 1;
            }
            else
            {
                iassert(image->mapType == MAPTYPE_CUBE);
                Image_CreateCubeTexture_PC(image, loadDef->dimensions[0], loadDef->levelCount, imageFormat);
                faceCount = 6;
            }
            data = &loadDef->data[0];
            mipCount = Image_CountMipmaps(loadDef->flags, image->width, image->height, image->depth);
            for (faceIndex = 0; faceIndex < faceCount; ++faceIndex)
            {
                if (faceCount == 1)
                    v5 = D3DCUBEMAP_FACE_POSITIVE_X;
                else
                    v5 = (D3DCUBEMAP_FACES)Image_CubemapFace(faceIndex);
                for (mipLevel = 0; mipLevel < mipCount; ++mipLevel)
                {
                    Image_UploadData(image, imageFormat, v5, mipLevel, data);
                    if (image->width >> mipLevel > 1)
                        mipWidth = image->width >> mipLevel;
                    else
                        mipWidth = 1;
                    if (image->height >> mipLevel > 1)
                        mipHeight = image->height >> mipLevel;
                    else
                        mipHeight = 1;
                    if (image->depth >> mipLevel > 1)
                        mipDepth = image->depth >> mipLevel;
                    else
                        mipDepth = 1;
                    data += Image_GetCardMemoryAmountForMipLevel(imageFormat, mipWidth, mipHeight, mipDepth);
                }
            }
            iassert(data == &loadDef->data[loadDef->resourceSize]);
        }
        else if (image->category == 5)
        {
            image->delayLoadPixels = 0;
            if (loadDef->dimensions[0] >> r_picmip_water->current.integer < 4)
                v7 = 4;
            else
                v7 = loadDef->dimensions[0] >> r_picmip_water->current.integer;
            if (loadDef->dimensions[1] >> r_picmip_water->current.integer < 4)
                v6 = 4;
            else
                v6 = loadDef->dimensions[1] >> r_picmip_water->current.integer;
            image->cardMemory.platform[0] = 0;
            image->cardMemory.platform[1] = 0;
            Image_Create2DTexture_PC(image, v7, v6, loadDef->levelCount, 0x10000, imageFormat);
        }
        else
        {
            if (image->cardMemory.platform[0] != Image_GetCardMemoryAmount(
                loadDef->flags,
                loadDef->format,
                loadDef->dimensions[0],
                loadDef->dimensions[1],
                loadDef->dimensions[2]))
                MyAssertHandler(
                    ".\\r_image.cpp",
                    788,
                    1,
                    "%s\n\t(image->name) = %s",
                    "(static_cast< uint >( image->cardMemory.platform[PICMIP_PLATFORM_USED] ) == Image_GetCardMemoryAmount( loadDef"
                    "->flags, static_cast< GfxPixelFormat >( loadDef->format ), loadDef->dimensions[0], loadDef->dimensions[1], loa"
                    "dDef->dimensions[2] ))",
                    image->name);
            if (image->texture.basemap)
                MyAssertHandler(
                    ".\\r_image.cpp",
                    789,
                    1,
                    "%s\n\t(image->name) = %s",
                    "(image->texture.basemap == 0)",
                    image->name);
            if (!image->delayLoadPixels)
            {
                externalDataSize = image->cardMemory.platform[0];
                image->cardMemory.platform[0] = 0;
                image->cardMemory.platform[1] = 0;
                if (!Image_LoadFromFile(image))
                    Com_Error(ERR_DROP, "Couldn't load image '%s'\n", image->name);
                DB_LoadedExternalData(externalDataSize);
            }
        }
    }
}

GfxImage *__cdecl Image_FindExisting(const char *name)
{
    if (useFastFile->current.enabled)
        return Image_FindExisting_FastFile(name);
    else
        return Image_FindExisting_LoadObj(name);
}

GfxImage *__cdecl Image_FindExisting_FastFile(const char *name)
{
    return DB_FindXAssetHeader(ASSET_TYPE_IMAGE, name).image;
}

GfxImage *__cdecl Image_Register(const char *imageName, unsigned __int8 semantic, int imageTrack)
{
    if (useFastFile->current.enabled)
        return (GfxImage *)Image_Register_FastFile(imageName);
    else
        return Image_Register_LoadObj((char*)imageName, semantic, imageTrack);
}

GfxImage *__cdecl Image_Register_FastFile(const char *imageName)
{
    return Image_FindExisting(imageName);
}

char __cdecl Image_LoadFromFile(GfxImage *image)
{
    return Image_LoadFromFileWithReader(image, (int(__cdecl *)(const char *, int *))FS_FOpenFileReadDatabase);
}

char __cdecl Image_ValidateHeader(GfxImageFileHeader *imageFile, const char *filepath)
{
    if (imageFile->tag[0] == 73 && imageFile->tag[1] == 87 && imageFile->tag[2] == 105)
    {
        if (imageFile->version == 6)
        {
            return 1;
        }
        else
        {
            Com_PrintError(8, "ERROR: image '%s' is version %i but should be version %i\n", filepath, imageFile->version, 6);
            return 0;
        }
    }
    else
    {
        Com_PrintError(8, "ERROR: image '%s' is not an IW image\n", filepath);
        return 0;
    }
}

void __cdecl Image_PrintTruncatedFileError(const char *filepath)
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

    if (!image)
        MyAssertHandler(".\\r_image_load_obj.cpp", 639, 0, "%s", "image");
    if (image->category != 3)
        MyAssertHandler(".\\r_image_load_obj.cpp", 641, 0, "%s", "image->category == IMG_CATEGORY_LOAD_FROM_FILE");
    if (image->texture.basemap)
        MyAssertHandler(".\\r_image_load_obj.cpp", 643, 0, "%s", "!image->texture.basemap");
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

    if (!image)
        MyAssertHandler(".\\r_image_load_obj.cpp", 328, 0, "%s", "image");
    if (!fileHeader)
        MyAssertHandler(".\\r_image_load_obj.cpp", 329, 0, "%s", "fileHeader");
    if (!data)
        MyAssertHandler(".\\r_image_load_obj.cpp", 330, 0, "%s", "data");
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

unsigned int __cdecl Image_CountMipmaps(char imageFlags, unsigned int width, unsigned int height, unsigned int depth)
{
    unsigned int mipRes; // [esp+0h] [ebp-8h]
    unsigned int mipCount; // [esp+4h] [ebp-4h]

    if ((imageFlags & 2) != 0)
        return 1;
    mipCount = 1;
    for (mipRes = 1; mipRes < width || mipRes < height || mipRes < depth; mipRes *= 2)
        ++mipCount;
    return mipCount;
}
unsigned int __cdecl Image_CountMipmapsForFile(const GfxImageFileHeader *fileHeader)
{
    return Image_CountMipmaps(
        fileHeader->flags,
        fileHeader->dimensions[0],
        fileHeader->dimensions[1],
        fileHeader->dimensions[2]);
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

    if (!image)
        MyAssertHandler(".\\r_image_load_obj.cpp", 385, 0, "%s", "image");
    if (!fileHeader)
        MyAssertHandler(".\\r_image_load_obj.cpp", 386, 0, "%s", "fileHeader");
    if (!data)
        MyAssertHandler(".\\r_image_load_obj.cpp", 387, 0, "%s", "data");
    if (format != D3DFMT_DXT1 && format != D3DFMT_DXT3 && format != D3DFMT_DXT5)
        MyAssertHandler(
            ".\\r_image_load_obj.cpp",
            391,
            0,
            "%s",
            "format == GFX_PF_DXT1 || format == GFX_PF_DXT3 || format == GFX_PF_DXT5");
    if (bytesPerBlock != 8 * (format != D3DFMT_DXT1) + 8)
        MyAssertHandler(".\\r_image_load_obj.cpp", 393, 0, "%s", "bytesPerBlock == (format == GFX_PF_DXT1 ? 8 : 16)");
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
            Image_UploadData(image, format, face, mipLevel - picmip, (unsigned char*)data);
            data += bytesPerBlock * ((int)(v5 + 3) >> 2) * ((int)(v6 + 3) >> 2);
        }
    }
}

void __cdecl Image_UploadData(
    const GfxImage *image,
    _D3DFORMAT format,
    _D3DCUBEMAP_FACES face,
    unsigned int mipLevel,
    unsigned __int8 *src)
{
    if (image->mapType != MAPTYPE_CUBE || !mipLevel || gfxMetrics.canMipCubemaps)
    {
        if (image->mapType == MAPTYPE_3D)
            Image_Upload3D_CopyData_PC(image, format, mipLevel, src);
        else
            Image_Upload2D_CopyData_PC(image, format, face, mipLevel, src);
    }
}

void __cdecl Image_LoadWhite(GfxImage *image)
{
    Image_LoadSolid(image, 0xFFu, 0xFFu, 0xFFu, 0xFFu);
}

void __cdecl Image_LoadSolid(
    GfxImage *image,
    unsigned __int8 r,
    unsigned __int8 g,
    unsigned __int8 b,
    unsigned __int8 a)
{
    unsigned __int8 pic[4]; // [esp+4h] [ebp-4h] BYREF

    *(unsigned int *)pic = (a << 24) | b | (g << 8) | (r << 16);
    Image_Generate2D(image, pic, 1, 1, D3DFMT_A8R8G8B8);
}

void __cdecl Image_LoadBlack(GfxImage *image)
{
    Image_LoadSolid(image, 0, 0, 0, 0xFFu);
}

void __cdecl Image_LoadGray(GfxImage *image)
{
    Image_LoadSolid(image, 0x80u, 0x80u, 0x80u, 0x80u);
}

void __cdecl Image_LoadIdentityNormalMap(GfxImage *image)
{
    Image_LoadSolid(image, 0x80u, 0x80u, 0xFFu, 0x80u);
}

void __cdecl Image_LoadBlack3D(GfxImage *image)
{
    unsigned __int8 pic[4]; // [esp+4h] [ebp-4h] BYREF

    *(unsigned int *)pic = -16777216;
    Image_Generate3D(image, pic, 1, 1, 1, D3DFMT_A8R8G8B8);
}

void __cdecl Image_LoadBlackCube(GfxImage *image)
{
    const unsigned __int8 *pic[6][15]; // [esp+4h] [ebp-170h] BYREF
    unsigned __int8 pixel[4]; // [esp+170h] [ebp-4h] BYREF

    *(unsigned int *)pixel = -16777216;
    pic[0][0] = pixel;
    pic[1][0] = pixel;
    pic[2][0] = pixel;
    pic[3][0] = pixel;
    pic[4][0] = pixel;
    pic[5][0] = pixel;
    Image_GenerateCube(image, pic, 1, D3DFMT_A8R8G8B8, 1u);
}

void __cdecl Image_LoadPixelCostColorCode(GfxImage *image)
{
    unsigned __int8 pic[257][4]; // [esp+0h] [ebp-408h] BYREF

    RB_PixelCost_BuildColorCodeMap(pic, 256);
    Image_Generate2D(image, pic[0], 256, 1, D3DFMT_X8R8G8B8);
}

GfxImage *__cdecl Image_Load(char *name, unsigned __int8 semantic, unsigned __int8 imageTrack)
{
    GfxImage *image; // [esp+0h] [ebp-4h]

    if (*name == 36)
        return Image_LoadBuiltin(name, semantic, imageTrack);
    image = Image_Alloc(name, 3u, semantic, imageTrack);
    if (!image)
        MyAssertHandler(".\\r_image_load_obj.cpp", 1136, 1, "%s", "image");
    if (image->texture.basemap)
        MyAssertHandler(".\\r_image_load_obj.cpp", 1138, 1, "%s", "image->texture.basemap == NULL");
    if (Image_LoadFromFile(image))
        return image;
    else
        return 0;
}

GfxImage *__cdecl Image_LoadBuiltin(char *name, unsigned __int8 semantic, unsigned __int8 imageTrack)
{
    GfxImage *image; // [esp+14h] [ebp-8h]
    unsigned int tableIndex; // [esp+18h] [ebp-4h]

    for (tableIndex = 0; ; ++tableIndex)
    {
        if (tableIndex >= 8)
        {
            Com_PrintError(8, "ERROR: Unknown built-in image '%s'", name);
            return 0;
        }
        if (!strcmp(constructorTable[tableIndex].name, name))
            break;
    }
    image = Image_Alloc(name, 1u, semantic, imageTrack);
    if (!image)
        MyAssertHandler(".\\r_image_load_obj.cpp", 1117, 1, "%s", "image");
    constructorTable[tableIndex].LoadCallback(image);
    return image;
}

void __cdecl Image_Construct(
    char *name,
    int nameSize,
    unsigned __int8 category,
    unsigned __int8 semantic,
    unsigned __int8 imageTrack,
    GfxImage *image)
{
    if (!name)
        MyAssertHandler(".\\r_image.cpp", 324, 0, "%s", "name");
    if (nameSize <= 0)
        MyAssertHandler(".\\r_image.cpp", 325, 0, "%s\n\t(nameSize) = %i", "(nameSize > 0)", nameSize);
    if (!image)
        MyAssertHandler(".\\r_image.cpp", 326, 0, "%s", "image");
    {
        PROF_SCOPED("R_memcpy");
        memcpy((unsigned __int8 *)image->name, (unsigned __int8 *)name, nameSize);
    }
    if (!category)
        MyAssertHandler(".\\r_image.cpp", 332, 0, "%s", "category != IMG_CATEGORY_UNKNOWN");
    image->category = category;
    image->semantic = semantic;
    if (image->noPicmip)
        MyAssertHandler(".\\r_image.cpp", 336, 1, "%s", "image->noPicmip == false");
    if (image->picmip.platform[0])
        MyAssertHandler(".\\r_image.cpp", 337, 1, "%s", "image->picmip.platform[PICMIP_PLATFORM_USED] == 0");
    if (image->picmip.platform[1])
        MyAssertHandler(".\\r_image.cpp", 339, 1, "%s", "image->picmip.platform[PICMIP_PLATFORM_MINSPEC] == 0");
    image->track = imageTrack;
}
int __cdecl Image_GetAvailableHashLocation(const char *name)
{
    int hashIndex; // [esp+0h] [ebp-4h]

    for (hashIndex = R_HashAssetName(name) & 0x7FF;
        imageGlobals.imageHashTable[hashIndex];
        hashIndex = ((_WORD)hashIndex + 1) & 0x7FF)
    {
        ;
    }
    return hashIndex;
}
GfxImage *__cdecl Image_Alloc(
    char *name,
    unsigned __int8 category,
    unsigned __int8 semantic,
    unsigned __int8 imageTrack)
{
    unsigned int v5; // [esp+0h] [ebp-20h]
    GfxImage *image; // [esp+10h] [ebp-10h]

    if (!name)
        MyAssertHandler(".\\r_image.cpp", 409, 0, "%s", "name");
    v5 = strlen(name);
    image = (GfxImage *)Hunk_Alloc(v5 + 37, "Image_Alloc", 22);
    if (!image)
        MyAssertHandler(".\\r_image.cpp", 418, 1, "%s", "image");
    image->name = (const char *)&image[1];
    Image_Construct(name, v5 + 1, category, semantic, imageTrack, image);
    imageGlobals.imageHashTable[Image_GetAvailableHashLocation(name)] = image;
    return image;
}
void __cdecl Image_Free(GfxImage *image)
{
    Image_Release(image);
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

IDirect3DSurface9 *__cdecl Image_GetSurface(GfxImage *image)
{
    const char *v1; // eax
    int hr; // [esp+0h] [ebp-8h]
    IDirect3DSurface9 *surface; // [esp+4h] [ebp-4h] BYREF

    if (!image)
        MyAssertHandler(".\\r_image.cpp", 1128, 0, "%s", "image");
    if (image->mapType != MAPTYPE_2D)
        MyAssertHandler(".\\r_image.cpp", 1129, 0, "%s", "image->mapType == MAPTYPE_2D");
    if (!image->texture.basemap)
        MyAssertHandler(".\\r_image.cpp", 1130, 0, "%s", "image->texture.map");
    do
    {
        if (r_logFile && r_logFile->current.integer)
            RB_LogPrint("image->texture.map->GetSurfaceLevel( 0, &surface )\n");
        //hr = ((int(__stdcall *)(unsigned int, unsigned int, unsigned int))image->texture.basemap->__vftable[1].AddRef)(
        //    (GfxTexture)image->texture.basemap,
        //    0,
        //    &surface);
        hr = image->texture.map->GetSurfaceLevel(0, &surface);
        if (hr < 0)
        {
            do
            {
                ++g_disableRendering;
                v1 = R_ErrorDescription(hr);
                Com_Error(
                    ERR_FATAL,
                    ".\\r_image.cpp (%i) image->texture.map->GetSurfaceLevel( 0, &surface ) failed: %s\n",
                    1132,
                    v1);
            } while (alwaysfails);
        }
    } while (alwaysfails);
    return surface;
}

void __cdecl R_SetPicmip()
{
    unsigned int texMemInMegs; // [esp+0h] [ebp-10h]
    unsigned int sysMemInMegs; // [esp+4h] [ebp-Ch]
    bool cappedPicmip; // [esp+Bh] [ebp-5h]
    int minPicmip; // [esp+Ch] [ebp-4h]

    if (!dx.device)
        MyAssertHandler(".\\r_image.cpp", 1233, 0, "%s", "dx.device");
    texMemInMegs = R_AvailableTextureMemory();
    sysMemInMegs = Dvar_GetInt("sys_sysMB");
    if (!r_reflectionProbeGenerate)
        MyAssertHandler(".\\r_image.cpp", 1243, 0, "%s", "r_reflectionProbeGenerate");
    if (r_reflectionProbeGenerate->current.enabled)
    {
        Com_Printf(8, "Picmip is set to lowest quality for generating reflections.\n");
        imageGlobals.picmip = 2;
        imageGlobals.picmipBump = 2;
        imageGlobals.picmipSpec = 2;
    }
    else
    {
        if (r_picmip_manual->current.enabled)
        {
            Com_Printf(8, "Picmip is set manually.\n");
            imageGlobals.picmip = r_picmip->current.integer;
            imageGlobals.picmipBump = r_picmip_bump->current.integer;
            imageGlobals.picmipSpec = r_picmip_spec->current.integer;
        }
        else
        {
            Com_Printf(8, "Texture detail is set automatically.\n");
            if (texMemInMegs < 0x1C2)
            {
                if (texMemInMegs < 0x12C)
                {
                    imageGlobals.picmip = texMemInMegs < 0xC8;
                    imageGlobals.picmipBump = 1;
                }
                else
                {
                    imageGlobals.picmip = 0;
                    imageGlobals.picmipBump = 0;
                }
                imageGlobals.picmipSpec = 1;
            }
            else
            {
                imageGlobals.picmip = 0;
                imageGlobals.picmipBump = 0;
                imageGlobals.picmipSpec = 0;
            }
            if (sysMemInMegs > 0x180)
                minPicmip = sysMemInMegs <= 0x280;
            else
                minPicmip = 2;
            if (minPicmip)
            {
                cappedPicmip = 0;
                if (imageGlobals.picmip < minPicmip)
                {
                    imageGlobals.picmip = minPicmip;
                    cappedPicmip = 1;
                }
                if (imageGlobals.picmipBump < minPicmip)
                {
                    imageGlobals.picmipBump = minPicmip;
                    cappedPicmip = 1;
                }
                if (imageGlobals.picmipSpec < minPicmip)
                {
                    imageGlobals.picmipSpec = minPicmip;
                    cappedPicmip = 1;
                }
                if (cappedPicmip)
                    Com_Printf(
                        8,
                        "Reducing texture detail based on total system memory of %i MB to improve load times.\n",
                        sysMemInMegs);
            }
            Dvar_SetInt(r_picmip, imageGlobals.picmip);
            Dvar_SetInt(r_picmip_bump, imageGlobals.picmipBump);
            Dvar_SetInt(r_picmip_spec, imageGlobals.picmipSpec);
        }
        if (!r_specular->current.enabled || !r_rendererInUse->current.integer)
            imageGlobals.picmipSpec = 3;
        Com_Printf(
            8,
            "Using picmip %i on most textures, %i on normal maps, and %i on specular maps\n",
            imageGlobals.picmip,
            imageGlobals.picmipBump,
            imageGlobals.picmipSpec);
    }
}

void R_InitRawImage()
{
    rgp.rawImage = Image_AllocProg(11, 4u, 0);
    if (!rgp.rawImage)
        MyAssertHandler(".\\r_image.cpp", 1142, 1, "%s", "rgp.rawImage");
}

void __cdecl R_InitImages()
{
    int i; // [esp+0h] [ebp-4h]

    for (i = 0; i < 2; ++i)
    {
        if (imageGlobals.totalMemory.platform[i])
            MyAssertHandler(".\\r_image.cpp", 1342, 0, "%s", "imageGlobals.totalMemory.platform[i] == 0");
    }
    R_SetPicmip();
    R_InitCodeImages();
    RB_InitImages();
    R_InitRawImage();
    rg.waterFloatTime = rg.waterFloatTime + 1.0;
}

bool __cdecl Image_IsCodeImage(int track)
{
    return track >= 0 && (track <= 1 || track == 4);
}

void R_InitCodeImages()
{
    rgp.whiteImage = Image_Register("$white", 1u, 0);
    if (!rgp.whiteImage)
        MyAssertHandler(".\\r_image.cpp", 1186, 1, "%s", "rgp.whiteImage");
    rgp.blackImage = Image_Register("$black", 1u, 0);
    if (!rgp.blackImage)
        MyAssertHandler(".\\r_image.cpp", 1189, 1, "%s", "rgp.blackImage");
    rgp.blackImage3D = Image_Register("$black_3d", 1u, 0);
    if (!rgp.blackImage3D)
        MyAssertHandler(".\\r_image.cpp", 1193, 1, "%s", "rgp.blackImage3D");
    rgp.blackImageCube = Image_Register("$black_cube", 1u, 0);
    if (!rgp.blackImageCube)
        MyAssertHandler(".\\r_image.cpp", 1196, 1, "%s", "rgp.blackImageCube");
    rgp.grayImage = Image_Register("$gray", 1u, 0);
    if (!rgp.grayImage)
        MyAssertHandler(".\\r_image.cpp", 1200, 1, "%s", "rgp.grayImage");
    rgp.identityNormalMapImage = Image_Register("$identitynormalmap", 1u, 0);
    if (!rgp.identityNormalMapImage)
        MyAssertHandler(".\\r_image.cpp", 1203, 1, "%s", "rgp.identityNormalMapImage");
    rgp.pixelCostColorCodeImage = Image_Register("$pixelcostcolorcode", 1u, 0);
    if (!rgp.pixelCostColorCodeImage)
        MyAssertHandler(".\\r_image.cpp", 1207, 1, "%s", "rgp.pixelCostColorCodeImage");
}

void __cdecl R_ImageList_f()
{
    const char *v0; // eax
    _D3DFORMAT v1; // eax
    const char *v2; // eax
    const char *fmt; // [esp+Ch] [ebp-2110h]
    GfxImage *image; // [esp+A0h] [ebp-207Ch]
    int v5; // [esp+A4h] [ebp-2078h]
    bool v6; // [esp+ABh] [ebp-2071h]
    unsigned __int8 dst[80]; // [esp+ACh] [ebp-2070h] BYREF
    unsigned int v8[2]; // [esp+FCh] [ebp-2020h]
    _D3DFORMAT v9; // [esp+104h] [ebp-2018h]
    unsigned int i; // [esp+108h] [ebp-2014h]
    ImageList imageList; // [esp+10Ch] [ebp-2010h] BYREF
    int j; // [esp+2114h] [ebp-8h]
    float v13; // [esp+2118h] [ebp-4h]

    v6 = 0;
    if (Cmd_Argc() == 2)
    {
        v0 = Cmd_Argv(1);
        v6 = I_stricmp(v0, "all") == 0;
    }
    v8[0] = 0;
    v8[1] = 0;
    memset(dst, 0, sizeof(dst));
    R_GetImageList(&imageList);
    if (v6)
    {
        for (i = 0; i < 0xE && imageList.count < 0x800; ++i)
        {
            if (g_imageProgs[i].mapType)
                imageList.image[imageList.count++] = &g_imageProgs[i];
        }
    }
    //std::sort(
    //    imageList.image,
    //    &imageList.image[imageList.count],
    //    (signed int)(4 * imageList.count) >> 2,
    //    imagecompare);
    std::sort(&imageList.image[0], &imageList.image[imageList.count], imagecompare);
    Com_Printf(8, "\n-fmt- -dimension-");
    for (j = 0; j < 2; ++j)
        Com_Printf(8, "%s", g_platform_name[j]);
    Com_Printf(8, "  --name-------\n");
    for (i = 0; i < imageList.count; ++i)
    {
        image = imageList.image[i];
        Com_Printf(8, "%4i x %-4i ", image->width, image->height);
        v1 = R_ImagePixelFormat(image);
        v9 = v1;
        if (v1 > D3DFMT_A8L8)
        {
            if (v1 > D3DFMT_DXT3)
            {
                if (v1 == D3DFMT_DXT5)
                {
                    Com_Printf(8, "DXT5  ");
                    goto LABEL_36;
                }
            }
            else
            {
                switch (v1)
                {
                case D3DFMT_DXT3:
                    Com_Printf(8, "DXT3  ");
                    goto LABEL_36;
                case D3DFMT_R32F:
                    Com_Printf(8, "R32F  ");
                    goto LABEL_36;
                case D3DFMT_DXT1:
                    Com_Printf(8, "DXT1  ");
                    goto LABEL_36;
                }
            }
        LABEL_34:
            if (!alwaysfails)
            {
                v2 = va("unhandled case: %d", v9);
                MyAssertHandler(".\\r_image.cpp", 1539, 1, v2);
            }
        }
        else if (v1 == D3DFMT_A8L8)
        {
            Com_Printf(8, "AL16  ");
        }
        else
        {
            switch (v1)
            {
            case D3DFMT_A8R8G8B8:
                Com_Printf(8, "RGBA32");
                break;
            case D3DFMT_X8R8G8B8:
                Com_Printf(8, "RGB32 ");
                break;
            case D3DFMT_A8:
                Com_Printf(8, "A8    ");
                break;
            case D3DFMT_L8:
                Com_Printf(8, "L8    ");
                break;
            default:
                goto LABEL_34;
            }
        }
    LABEL_36:
        Com_Printf(8, "  %s", imageTypeName[image->track]);
        for (j = 0; j < 2; ++j)
        {
            v13 = (double)image->cardMemory.platform[j] / 1024.0;
            if (v13 >= 10.0)
                fmt = "%7.0fk";
            else
                fmt = "%7.1fk";
            Com_Printf(8, fmt, v13);
            v5 = image->cardMemory.platform[j];
            if (!useFastFile->current.enabled)
            {
                *(unsigned int *)&dst[8 * image->track + 4 * j] += v5;
                if (!v6 && Image_IsCodeImage(image->track))
                    continue;
            }
            v8[j] += v5;
        }
        Com_Printf(8, "  %s\n", image->name);
    }
    Com_Printf(8, " ---------\n");
    Com_Printf(8, " %i total images\n", imageList.count);
    for (j = 0; j < 2; ++j)
        Com_Printf(8, " %5.1f MB %s total image size\n", (double)(int)v8[j] / 1048576.0, g_platform_name[j]);
    if (!useFastFile->current.enabled)
    {
        Com_Printf(8, "\n");
        Com_Printf(8, "       ");
        for (j = 0; j < 2; ++j)
            Com_Printf(8, "%s", g_platform_name[j]);
        Com_Printf(8, "\n");
        for (i = 0; i < 0xA; ++i)
        {
            Com_Printf(8, "%s:", imageTypeName[i]);
            for (j = 0; j < 2; ++j)
                Com_Printf(8, "  %5.1f", (double)*(int *)&dst[8 * i + 4 * j] / 1048576.0);
            Com_Printf(8, "  MB\n");
        }
    }
    Com_Printf(8, "Related commands: meminfo, imagelist, gfx_world, gfx_model, cg_drawfps, com_statmon, tempmeminfo\n");
}

bool __cdecl imagecompare(GfxImage *image1, GfxImage *image2)
{
    if (image1->track > (int)image2->track)
        return 0;
    if (image1->track >= (int)image2->track)
        return image1->cardMemory.platform[0] < image2->cardMemory.platform[0];
    return 1;
}

void __cdecl R_FreeLostImage(XAssetHeader header)
{
    if (!header.xmodelPieces)
        MyAssertHandler(".\\r_image.cpp", 989, 0, "%s", "image");
    if (!BYTE2(header.xmodelPieces[2].numpieces))
        MyAssertHandler(".\\r_image.cpp", 990, 0, "%s", "image->category != IMG_CATEGORY_UNKNOWN");
    if (BYTE2(header.xmodelPieces[2].numpieces) >= 5u)
        Image_Release(header.image);
}

char __cdecl Image_ReloadFromFile(GfxImage *image)
{
    return Image_LoadFromFileWithReader(image, (int(__cdecl *)(const char *, int *))FS_FOpenFileRead);
}

char __cdecl R_DuplicateTexture(GfxImage *dstImage, const GfxImage *srcImage)
{
    if (!srcImage || !srcImage->texture.basemap)
        return 0;
    dstImage->texture.basemap = srcImage->texture.basemap;
    //dstImage->texture.basemap->AddRef(dstImage->texture.basemap);
    dstImage->texture.basemap->AddRef();
    return 1;
}

char __cdecl Image_AssignDefaultTexture(GfxImage *image)
{
    if (image->mapType != MAPTYPE_2D)
        return 0;
    if (image->semantic == 5)
        return R_DuplicateTexture(image, rgp.identityNormalMapImage);
    if (image->semantic == 8)
        return R_DuplicateTexture(image, rgp.blackImage);
    return R_DuplicateTexture(image, rgp.whiteImage);
}

void __cdecl Image_Rebuild(GfxImage *image)
{
    const char *v1; // eax
    unsigned __int8 category; // [esp+0h] [ebp-4h]

    if (!image)
        MyAssertHandler(".\\r_image.cpp", 890, 0, "%s", "image");
    if (!image->category)
        MyAssertHandler(".\\r_image.cpp", 891, 0, "%s", "image->category != IMG_CATEGORY_UNKNOWN");
    if (image->category < 5u)
        MyAssertHandler(".\\r_image.cpp", 894, 0, "%s", "image->category >= IMG_CATEGORY_FIRST_UNMANAGED");
    if (image->texture.basemap)
        MyAssertHandler(".\\r_image.cpp", 896, 0, "%s", "!image->texture.basemap");
    category = image->category;
    if (category == 5)
    {
        Image_BuildWaterMap(image);
    }
    else if (category == 6)
    {
        if (!alwaysfails)
            MyAssertHandler(".\\r_image.cpp", 905, 1, "non-prog image cannot be a render target");
    }
    else if (!alwaysfails)
    {
        v1 = va("unhandled case %i", image->category);
        MyAssertHandler(".\\r_image.cpp", 909, 1, v1);
    }
}

void __cdecl R_RebuildLostImage(XAssetHeader header)
{
    if (!header.xmodelPieces)
        MyAssertHandler(".\\r_image.cpp", 1004, 0, "%s", "image");
    if (!BYTE2(header.xmodelPieces[2].numpieces))
        MyAssertHandler(".\\r_image.cpp", 1005, 0, "%s", "image->category != IMG_CATEGORY_UNKNOWN");
    if (!header.xmodelPieces->numpieces)
    {
        if (BYTE2(header.xmodelPieces[2].numpieces) < 5u)
        {
            if (BYTE2(header.xmodelPieces[2].numpieces) == 3)
            {
                if (!HIBYTE(header.xmodelPieces[2].numpieces)
                    && !Image_ReloadFromFile(header.image)
                    && !Image_AssignDefaultTexture(header.image))
                {
                    Com_Error(
                        ERR_DROP,
                        "Couldn't load image '%s' to recover from a lost device",
                        (const char *)header.xmodelPieces[2].pieces);
                }
            }
            else
            {
                Com_Error(
                    ERR_DROP,
                    "No way to recover image '%s' from a lost device",
                    (const char *)header.xmodelPieces[2].pieces);
            }
        }
        else if (!Image_IsProg(header.image))
        {
            Image_Rebuild(header.image);
        }
    }
}

void __cdecl R_ReloadLostImages()
{
    DB_EnumXAssets(ASSET_TYPE_IMAGE, (void(__cdecl *)(XAssetHeader, void *))R_RebuildLostImage, 0, 1);
}

void __cdecl R_ReleaseLostImages()
{
    rg.waterFloatTime = rg.waterFloatTime + 1.0;
    DB_EnumXAssets(ASSET_TYPE_IMAGE, (void(__cdecl *)(XAssetHeader, void *))R_FreeLostImage, 0, 1);
}

_D3DFORMAT __cdecl R_ImagePixelFormat(const GfxImage *image)
{
    const char *v2; // eax
    MapType mapType; // [esp+0h] [ebp-40h]
    _D3DSURFACE_DESC surfaceDesc; // [esp+4h] [ebp-3Ch] BYREF
    _D3DVOLUME_DESC volumeDesc; // [esp+24h] [ebp-1Ch] BYREF

    mapType = image->mapType;
    if (image->mapType == MAPTYPE_2D)
    {
        if (!image->texture.map)
            MyAssertHandler(".\\r_image.cpp", 1388, 0, "%s", "image->texture.map");

        image->texture.map->GetLevelDesc(0, &surfaceDesc);
        return surfaceDesc.Format;
    }

    if (mapType == MAPTYPE_3D)
    {
        if (!image->texture.volmap)
            MyAssertHandler(".\\r_image.cpp", 1398, 0, "%s", "image->texture.volmap");
        //image->texture.basemap->__vftable[1].QueryInterface(image->texture.basemap, 0, (void **)&volumeDesc);
        image->texture.volmap->GetLevelDesc(0, &volumeDesc);
        return volumeDesc.Format;
    }
    if (mapType == MAPTYPE_CUBE)
    {
        if (!image->texture.cubemap)
            MyAssertHandler(".\\r_image.cpp", 1393, 0, "%s", "image->texture.cubemap");
    LABEL_7:
        //image->texture.basemap->__vftable[1].QueryInterface(image->texture.basemap, 0, (void **)&surfaceDesc);
        image->texture.cubemap->GetLevelDesc(0, &surfaceDesc);
        return surfaceDesc.Format;
    }

    if (!alwaysfails)
    {
        v2 = va("unhandled case %i for %s", image->mapType, image->name);
        MyAssertHandler(".\\r_image.cpp", 1403, 1, v2);
    }
    return (_D3DFORMAT)0;
}


void __cdecl Image_CreateCubeTexture_PC(
    GfxImage *image,
    unsigned __int16 edgeLen,
    unsigned int mipmapCount,
    _D3DFORMAT imageFormat)
{
    const char *v4; // eax
    const char *v5; // eax
    const char *v6; // eax
    int hr; // [esp+0h] [ebp-4h]

    if (!image)
        MyAssertHandler(".\\r_image.cpp", 605, 0, "%s", "image");
    if (image->texture.basemap)
        MyAssertHandler(".\\r_image.cpp", 606, 0, "%s", "!image->texture.basemap");
    image->width = edgeLen;
    image->height = edgeLen;
    image->depth = 1;
    image->mapType = MAPTYPE_CUBE;
    if (!gfxMetrics.canMipCubemaps)
        mipmapCount = 1;
    //hr = dx.device->CreateCubeTexture(
    //    dx.device,
    //    edgeLen,
    //    mipmapCount,
    //    0,
    //    imageFormat,
    //    D3DPOOL_MANAGED,
    //    (IDirect3DCubeTexture9 **)&image->texture,
    //    0);
    hr = dx.device->CreateCubeTexture(edgeLen, mipmapCount, 0, imageFormat, D3DPOOL_MANAGED, (IDirect3DCubeTexture9 **)&image->texture, 0);
    if (hr < 0)
    {
        v4 = R_ErrorDescription(hr);
        Com_Error(
            ERR_DROP,
            "CreateCubeTexture ( %s, %i, %i, %i ) failed: %08x = %s",
            image->name,
            image->width,
            mipmapCount,
            imageFormat,
            hr,
            v4);
    }
    if (hr != -2005530520 && !image->texture.basemap)
    {
        v5 = R_ErrorDescription(hr);
        v6 = va(
            "DirectX succeeded without creating cube texture for %s: size %ix%i, type %08x, hr %08x = %s",
            image->name,
            image->width,
            image->height,
            imageFormat,
            hr,
            v5);
        MyAssertHandler(".\\r_image.cpp", 621, 0, "%s\n\t%s", "hr == D3DERR_DEVICELOST || image->texture.map", v6);
    }
}


void __cdecl Image_Create3DTexture_PC(
    GfxImage *image,
    unsigned __int16 width,
    unsigned __int16 height,
    unsigned __int16 depth,
    unsigned int mipmapCount,
    int imageFlags,
    _D3DFORMAT imageFormat)
{
    HRESULT v7; // eax
    const char *v8; // eax
    const char *v9; // eax
    const char *v10; // eax
    HRESULT hr; // [esp+0h] [ebp-Ch]
    unsigned int usage; // [esp+4h] [ebp-8h]

    if (!image)
        MyAssertHandler(".\\r_image.cpp", 575, 0, "%s", "image");
    if (image->texture.basemap)
        MyAssertHandler(".\\r_image.cpp", 576, 0, "%s", "!image->texture.basemap");
    image->width = width;
    image->height = height;
    image->depth = depth;
    image->mapType = MAPTYPE_3D;
    usage = Image_GetUsage(imageFlags, imageFormat);
    if ((imageFlags & 0x40000) != 0)
    {
        //v7 = dx.device->CreateVolumeTexture(
        //    dx.device,
        //    width,
        //    height,
        //    depth,
        //    mipmapCount,
        //    0,
        //    imageFormat,
        //    D3DPOOL_SYSTEMMEM,
        //    (IDirect3DVolumeTexture9 **)&image->texture,
        //    0);
        v7 = dx.device->CreateVolumeTexture(width, height, depth, mipmapCount, 0, imageFormat, D3DPOOL_SYSTEMMEM, (IDirect3DVolumeTexture9 **)&image->texture, 0);
    }
    else
    {
        //v7 = dx.device->CreateVolumeTexture(
        //    dx.device,
        //    width,
        //    height,
        //    depth,
        //    mipmapCount,
        //    0,
        //    imageFormat,
        //    (_D3DPOOL)(usage == 0),
        //    (IDirect3DVolumeTexture9 **)&image->texture,
        //    0);
        v7 = dx.device->CreateVolumeTexture(width, height, depth, mipmapCount, 0, imageFormat, (_D3DPOOL)(usage == 0), (IDirect3DVolumeTexture9 **)&image->texture, 0);
    }
    hr = v7;
    if (v7 < 0)
    {
        v8 = R_ErrorDescription(v7);
        Com_Error(
            ERR_DROP,
            "Create3DTexture( %s, %i, %i, %i, %i, %i ) failed: %08x = %s",
            image->name,
            image->width,
            image->height,
            image->depth,
            0,
            imageFormat,
            hr,
            v8);
    }
    if (hr != -2005530520 && !image->texture.basemap)
    {
        v9 = R_ErrorDescription(hr);
        v10 = va(
            "DirectX succeeded without creating 3D texture for %s: size %ix%ix%i, type %08x, hr %08x = %s",
            image->name,
            image->width,
            image->height,
            image->depth,
            imageFormat,
            hr,
            v9);
        MyAssertHandler(".\\r_image.cpp", 594, 0, "%s\n\t%s", "hr == D3DERR_DEVICELOST || image->texture.map", v10);
    }
}

void __cdecl RB_UnbindAllImages()
{
    unsigned int samplerIndex; // [esp+0h] [ebp-4h]

    if (dx.device && !dx.deviceLost)
    {
        for (samplerIndex = 0; samplerIndex < vidConfig.maxTextureMaps; ++samplerIndex)
            R_DisableSampler(&gfxCmdBufState, samplerIndex);
    }
}

void __cdecl Image_Reload(GfxImage *image)
{
    if (!image)
        MyAssertHandler(".\\r_image.cpp", 1092, 0, "%s", "image");
    Image_Release(image);
    if (!Image_ReloadFromFile(image) && !Image_AssignDefaultTexture(image))
        Com_Error(ERR_FATAL, "failed to load image '%s'", image->name);
}

void __cdecl Image_UpdatePicmip(GfxImage *image)
{
    Picmip picmip; // [esp+0h] [ebp-4h] BYREF

    if (!image)
        MyAssertHandler(".\\r_image.cpp", 1078, 0, "%s", "image");
    if (image->category == 3 && !image->noPicmip)
    {
        Image_GetPicmip(image, &picmip);
        if (image->picmip.platform[0] != picmip.platform[0])
            Image_Reload(image);
    }
}