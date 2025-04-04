#pragma once
#include "r_material.h"

struct ImageList // sizeof=0x2004
{                                       // ...
    unsigned int count;                 // ...
    GfxImage *image[2048];              // ...
};

struct Image_MemUsage // sizeof=0xC
{                                       // ...
    int total;                          // ...
    int lightmap;
    int minspec;                        // ...
};

struct ImgGlobals //$C12090365A206BC63E0695BF82A7DA9E // sizeof=0x2014
{                                       // ...
    GfxImage *imageHashTable[2048];     // ...
    int picmip;                         // ...
    int picmipBump;                     // ...
    int picmipSpec;                     // ...
    CardMemory totalMemory;             // ...
};

struct GfxImageFileHeader // sizeof=0x1C
{                                       // ...
    char tag[3];
    unsigned __int8 version;
    unsigned __int8 format;
    unsigned __int8 flags;              // ...
    __int16 dimensions[3];              // ...
    int fileSizeForPicmip[4];           // ...
};

void __cdecl TRACK_r_image();
void __cdecl R_GetImageList(ImageList *imageList);
void __cdecl R_AddImageToList(XAssetHeader header, ImageList *data);
void __cdecl R_SumOfUsedImages(Image_MemUsage *usage);
void __cdecl Image_Release(GfxImage *image);
GfxImage *__cdecl Image_AllocProg(int imageProgType, unsigned __int8 category, unsigned __int8 semantic);
void __cdecl Image_SetupRenderTarget(
    GfxImage *image,
    unsigned __int16 width,
    unsigned __int16 height,
    _D3DFORMAT imageFormat);
void __cdecl Load_Texture(GfxTexture *remoteLoadDef, GfxImage *image);
GfxImage *__cdecl Image_FindExisting(const char *name);
GfxImage *__cdecl Image_FindExisting_FastFile(const char *name);
GfxImage *__cdecl Image_Register_LoadObj(char *imageName, unsigned __int8 semantic, unsigned __int8 imageTrack);
GfxImage *__cdecl Image_Register(const char *imageName, unsigned __int8 semantic, int imageTrack);
GfxImage *__cdecl Image_Register_FastFile(const char *imageName);
IDirect3DSurface9 *__cdecl Image_GetSurface(GfxImage *image);
void __cdecl R_InitImages();
void R_InitCodeImages();
void __cdecl R_ImageList_f();
bool __cdecl Image_IsCodeImage(int track);
bool __cdecl imagecompare(GfxImage *image1, GfxImage *image2);
_D3DFORMAT __cdecl R_ImagePixelFormat(const GfxImage *image);

void __cdecl Image_UploadData(
    const GfxImage *image,
    _D3DFORMAT format,
    _D3DCUBEMAP_FACES face,
    unsigned int mipLevel,
    unsigned __int8 *src);


// r_image_utils
void __cdecl R_DownsampleMipMapBilinear(
    const unsigned __int8 *src,
    int srcWidth,
    int srcHeight,
    int texelPitch,
    unsigned __int8 *dst);

extern ImgGlobals imageGlobals;