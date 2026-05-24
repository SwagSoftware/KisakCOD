#pragma once
#include "r_material.h"
#include <xanim/xanim.h>

 enum $92364187413C9A0320C404614F91083D : __int32
 {
     PICMIP_PLATFORM_USED    = 0x0,
     PICMIP_PLATFORM_MINSPEC = 0x1,
     PICMIP_PLATFORM_COUNT   = 0x2,
 };

enum GfxRefBlendMode : __int32
{                                       // ...
    BLENDMODE_OPAQUE = 0x0,
    BLENDMODE_BLEND = 0x1,
    BLENDMODE_GT0 = 0x2,
    BLENDMODE_GE128 = 0x3,
    BLENDMODE_LT128 = 0x4,
    BLENDMODE_ADD = 0x5,
};
enum file_image_flags_t : __int32
{
    IMG_FLAG_NOPICMIP = 0x1,
    IMG_FLAG_NOMIPMAPS = 0x2,
    IMG_FLAG_CUBEMAP = 0x4,
    IMG_FLAG_VOLMAP = 0x8,
    IMG_FLAG_STREAMING = 0x10,
    IMG_FLAG_LEGACY_NORMALS = 0x20,
    IMG_FLAG_CLAMP_U = 0x40,
    IMG_FLAG_CLAMP_V = 0x80,
    IMG_FLAG_DYNAMIC = 0x10000,
    IMG_FLAG_RENDER_TARGET = 0x20000,
    IMG_FLAG_SYSTEMMEM = 0x40000,
};
enum $E681A048096CB9E4B36F1590F98F8E52 : __int32
{
    IMG_CATEGORY_UNKNOWN = 0x0,
    IMG_CATEGORY_AUTO_GENERATED = 0x1,
    IMG_CATEGORY_LIGHTMAP = 0x2,
    IMG_CATEGORY_LOAD_FROM_FILE = 0x3,
    IMG_CATEGORY_RAW = 0x4,
    IMG_CATEGORY_FIRST_UNMANAGED = 0x5,
    IMG_CATEGORY_WATER = 0x5,
    IMG_CATEGORY_RENDERTARGET = 0x6,
    IMG_CATEGORY_TEMP = 0x7,
};
enum $1B8EAFF1434832E143B04F7E036A82BD : __int32
{
    TS_2D = 0x0,
    TS_FUNCTION = 0x1,
    TS_COLOR_MAP = 0x2,
    TS_UNUSED_1 = 0x3,
    TS_UNUSED_2 = 0x4,
    TS_NORMAL_MAP = 0x5,
    TS_UNUSED_3 = 0x6,
    TS_UNUSED_4 = 0x7,
    TS_SPECULAR_MAP = 0x8,
    TS_UNUSED_5 = 0x9,
    TS_UNUSED_6 = 0xA,
    TS_WATER_MAP = 0xB,
};
struct GfxRawPixel // sizeof=0x4
{                                       // ...
    uint8_t r;                  // ...
    uint8_t g;                  // ...
    uint8_t b;                  // ...
    uint8_t a;                  // ...
};
struct GfxRawImage // sizeof=0x54
{                                       // ...
    char name[64];
    GfxRefBlendMode blendMode;
    bool hasAlpha;
    // padding byte
    // padding byte
    // padding byte
    int width;                          // ...
    int height;
    GfxRawPixel *pixels;                // ...
};

struct ImageList // sizeof=0x2004
{                                       // ...
    uint32_t count;                 // ...
    GfxImage *image[2048];              // ...
};

struct Image_MemUsage // sizeof=0xC
{                                       // ...
    int total;                          // ...
    int lightmap;
    int minspec;                        // ...
};

#define IMAGE_HASH_TABLE_SIZE 2048 // lwss add
struct ImgGlobals //$C12090365A206BC63E0695BF82A7DA9E // sizeof=0x2014
{                                       // ...
    GfxImage *imageHashTable[IMAGE_HASH_TABLE_SIZE];     // ...
    int picmip;                         // ...
    int picmipBump;                     // ...
    int picmipSpec;                     // ...
    CardMemory totalMemory;             // ...
};

struct GfxImageFileHeader // sizeof=0x1C
{                                       // ...
    char tag[3];
    uint8_t version;
    uint8_t format;
    uint8_t flags;              // ...
    __int16 dimensions[3];              // ...
    int fileSizeForPicmip[4];           // ...
};

void __cdecl TRACK_r_image();
void __cdecl R_DelayLoadImage(XAssetHeader header);
void __cdecl R_ShutdownImages();
void __cdecl Image_SetupAndLoad(
    GfxImage *image,
    int width,
    int height,
    int depth,
    int imageFlags,
    _D3DFORMAT imageFormat);
GfxImage *__cdecl Image_Alloc(
    char *name,
    uint8_t category,
    uint8_t semantic,
    uint8_t imageTrack);
void __cdecl Image_Free(GfxImage *image);
void __cdecl R_GetImageList(ImageList *imageList);
void __cdecl R_AddImageToList(XAssetHeader header, ImageList *data);
void __cdecl R_SumOfUsedImages(Image_MemUsage *usage);
void __cdecl Image_Release(GfxImage *image);
GfxImage *__cdecl Image_AllocProg(int imageProgType, uint8_t category, uint8_t semantic);
void __cdecl Image_SetupRenderTarget(
    GfxImage *image,
    uint16_t width,
    uint16_t height,
    _D3DFORMAT imageFormat);
void __cdecl Load_Texture(GfxTexture *remoteLoadDef, GfxImage *image);
GfxImage *__cdecl Image_FindExisting(const char *name);
GfxImage *__cdecl Image_FindExisting_FastFile(const char *name);
GfxImage *__cdecl Image_Register(const char *imageName, uint8_t semantic, int imageTrack);
GfxImage *__cdecl Image_Register_FastFile(const char *imageName);
char __cdecl Image_ValidateHeader(GfxImageFileHeader *imageFile, const char *filepath);
IDirect3DSurface9 *__cdecl Image_GetSurface(GfxImage *image);
void __cdecl R_InitImages();
void R_InitCodeImages();
void __cdecl R_ImageList_f();
bool __cdecl Image_IsCodeImage(int track);
bool __cdecl imagecompare(GfxImage *image1, GfxImage *image2);
_D3DFORMAT __cdecl R_ImagePixelFormat(const GfxImage *image);

char __cdecl Image_LoadFromFile(GfxImage *image);
char __cdecl Image_AssignDefaultTexture(GfxImage *image);
int __cdecl Image_GetAvailableHashLocation(const char *name);
void __cdecl Image_LoadSolid(
    GfxImage *image,
    uint8_t r,
    uint8_t g,
    uint8_t b,
    uint8_t a);
GfxImage *__cdecl Image_LoadBuiltin(char *name, uint8_t semantic, uint8_t imageTrack);
double __cdecl Outdoor_TraceHeightInWorld(float worldX, float worldY);
int __cdecl Outdoor_TransformToTextureClamped(int dimension, float inWorld);
void __cdecl Image_LoadWhite(GfxImage *image);
void __cdecl Image_LoadBlack(GfxImage *image);
void __cdecl Image_LoadBlack3D(GfxImage *image);
void __cdecl Image_LoadBlackCube(GfxImage *image);
void __cdecl Image_LoadPixelCostColorCode(GfxImage *image);
void __cdecl Image_LoadGray(GfxImage *image);
void __cdecl Image_LoadIdentityNormalMap(GfxImage *image);
void __cdecl Outdoor_SetRendererOutdoorLookupMatrix(GfxWorld *world);


void __cdecl R_ReleaseLostImages();
void __cdecl R_ReloadLostImages();

void __cdecl Image_UploadData(
    const GfxImage *image,
    _D3DFORMAT format,
    _D3DCUBEMAP_FACES face,
    uint32_t mipLevel,
    uint8_t *src);

void __cdecl Image_CreateCubeTexture_PC(
    GfxImage *image,
    uint16_t edgeLen,
    uint32_t mipmapCount,
    _D3DFORMAT imageFormat);

void __cdecl Image_Create3DTexture_PC(
    GfxImage *image,
    uint16_t width,
    uint16_t height,
    uint16_t depth,
    uint32_t mipmapCount,
    int imageFlags,
    _D3DFORMAT imageFormat);

void __cdecl Image_Create2DTexture_PC(
    GfxImage *image,
    uint16_t width,
    uint16_t height,
    uint32_t mipmapCount,
    int imageFlags,
    _D3DFORMAT imageFormat);

void __cdecl RB_UnbindAllImages();
void __cdecl R_SetPicmip();

void __cdecl R_DelayLoadImage(XAssetHeader header);

// r_image_utils
void __cdecl R_DownsampleMipMapBilinear(
    const uint8_t *src,
    int srcWidth,
    int srcHeight,
    int texelPitch,
    uint8_t *dst);

inline uint32_t __cdecl Image_GetUsage(int imageFlags, _D3DFORMAT imageFormat)
{
    if ((imageFlags & 0x20000) != 0)
    {
        if (imageFormat == D3DFMT_D24S8 || imageFormat == D3DFMT_D24X8 || imageFormat == D3DFMT_D16)
            return 2;
        else
            return 1;
    }
    else if ((imageFlags & 0x10000) != 0)
    {
        return 512;
    }
    else
    {
        return 0;
    }
}


// r_image_load_obj
void __cdecl Image_BuildWaterMap(GfxImage *image);
void __cdecl Image_SetupFromFile(GfxImage *image, const GfxImageFileHeader *fileHeader, _D3DFORMAT imageFormat);
uint32_t __cdecl Image_CountMipmaps(char imageFlags, uint32_t width, uint32_t height, uint32_t depth);
void __cdecl Image_Setup(GfxImage *image, int width, int height, int depth, int imageFlags, _D3DFORMAT imageFormat);
void __cdecl Image_TrackTexture(GfxImage *image, char imageFlags, _D3DFORMAT format, int width, int height, int depth);
uint32_t __cdecl Image_GetCardMemoryAmount(
    char imageFlags,
    _D3DFORMAT format,
    uint32_t width,
    uint32_t height,
    uint32_t depth);
uint32_t __cdecl Image_GetCardMemoryAmountForMipLevel(
    _D3DFORMAT format,
    uint32_t mipWidth,
    uint32_t mipHeight,
    uint32_t mipDepth);
void __cdecl Image_TrackTotalMemory(GfxImage *image, int platform, int memory);
uint8_t *__cdecl Image_AllocTempMemory(int bytes);
void __cdecl Image_FreeTempMemory(uint8_t *mem, int bytes);
GfxImage *__cdecl Image_FindExisting_LoadObj(const char *name);
bool __cdecl Image_IsProg(GfxImage *image);
void __cdecl Image_ExpandBgr(const uint8_t *src, uint32_t count, uint8_t *dst);
void __cdecl Image_Generate2D(GfxImage *image, uint8_t *pixels, int width, int height, _D3DFORMAT imageFormat);
void __cdecl Image_Generate3D(
    GfxImage *image,
    uint8_t *pixels,
    int width,
    int height,
    int depth,
    _D3DFORMAT imageFormat);
void __cdecl Image_GenerateCube(
    GfxImage *image,
    const uint8_t *(*pixels)[15],
    int edgeLen,
    _D3DFORMAT imageFormat,
    uint32_t mipCount);
void __cdecl Image_LoadBitmap(
    GfxImage *image,
    const GfxImageFileHeader *fileHeader,
    uint8_t *data,
    _D3DFORMAT format,
    int bytesPerPixel);
void __cdecl Image_LoadDxtc(
    GfxImage *image,
    const GfxImageFileHeader *fileHeader,
    const uint8_t *data,
    _D3DFORMAT format,
    int bytesPerBlock);
void __cdecl Image_LoadFromData(GfxImage *image, GfxImageFileHeader *fileHeader, uint8_t *srcData);
GfxImage *__cdecl Image_Register_LoadObj(char *imageName, uint8_t semantic, uint8_t imageTrack);
char __cdecl Image_LoadFromFileWithReader(GfxImage *image, int(__cdecl *OpenFileRead)(const char *, int *));

// r_image_decode
struct ddscolor_t_s // sizeof=0x2
{                                       // ...
    uint16_t b : 5;
    uint16_t g : 6;
    uint16_t r : 5;
};
union ddscolor_t // sizeof=0x2
{                                       // ...
    ddscolor_t_s c;
    uint16_t rgb;
};
struct DdsBlock_Dxt1_t // sizeof=0x8
{                                       // ...
    ddscolor_t color0;
    ddscolor_t color1;
    uint8_t bits[4];
};
struct DdsBlock_Dxt3_t // sizeof=0x10
{
    uint8_t alpha[8];
    DdsBlock_Dxt1_t color;
};
void __cdecl Image_FreeRawPixels(GfxRawImage *image);
void __cdecl Image_GetRawPixels(char *imageName, GfxRawImage *image);
uint32_t __cdecl Image_CountMipmapsForFile(const GfxImageFileHeader *fileHeader);
int __cdecl Image_CountMipmapsForFile_0(GfxImageFileHeader *imageFile);
int __cdecl Image_CountMipmapsForFile(GfxImageFileHeader *imageFile);
void __cdecl Image_CopyBitmapData(GfxRawImage *image, GfxImageFileHeader *imageFile, uint8_t *imageData);
void __cdecl Image_DecodeWavelet(
    GfxRawImage *image,
    GfxImageFileHeader *imageFile,
    uint8_t *imageData,
    int bytesPerPixel);

// r_image_wavelet
struct WaveletDecode // sizeof=0x20
{                                       // ...
    uint16_t value;             // ...
    uint16_t bit;               // ...
    const unsigned char *data;                         // ...
    int width;                          // ...
    int height;                         // ...
    int channels;                       // ...
    int bpp;                            // ...
    int mipLevel;                       // ...
    bool dataInitialized;               // ...
    // padding byte
    // padding byte
    // padding byte
};
void __cdecl Image_LoadWavelet(
    GfxImage *image,
    const GfxImageFileHeader *fileHeader,
    const unsigned char *data,
    _D3DFORMAT format,
    int bytesPerPixel);
void __cdecl Wavelet_DecompressLevel(uint8_t *src, uint8_t *dst, WaveletDecode *decode);
void __cdecl Wavelet_ConsumeBits(uint16_t bitCount, WaveletDecode *decode);
int __cdecl Wavelet_DecodeValue(
    const struct WaveletHuffmanDecode *decodeTable,
    uint16_t bitCount,
    int bias,
    WaveletDecode *decode);
void __cdecl Wavelet_AddDeltaToMipmap(
    uint8_t *inout,
    int size,
    WaveletDecode *decode,
    const int *dstChanOffset);
void __cdecl Wavelet_DecompressLevel(uint8_t *src, uint8_t *dst, WaveletDecode *decode);
void __cdecl Image_DecodeWavelet(
    GfxRawImage *image,
    GfxImageFileHeader *imageFile,
    uint8_t *imageData,
    int bytesPerPixel);



extern ImgGlobals imageGlobals;
extern GfxImage g_imageProgs[14];



// r_image_load_common
uint32_t __cdecl Image_CubemapFace(uint32_t faceIndex);
void __cdecl Image_GetPicmip(const GfxImage *image, Picmip *picmip);
void __cdecl Image_PicmipForSemantic(uint8_t semantic, Picmip *picmip);
int __cdecl Image_SourceBytesPerSlice_PC(_D3DFORMAT format, int width, int height);
void __cdecl Image_Upload3D_CopyData_PC(
    const GfxImage *image,
    _D3DFORMAT format,
    uint32_t mipLevel,
    uint8_t *src);
void __cdecl Image_Upload2D_CopyDataBlock_PC(
    int width,
    int height,
    uint8_t *src,
    _D3DFORMAT format,
    int dstPitch,
    uint8_t *dst);
void __cdecl Image_Upload2D_CopyData_PC(
    const GfxImage *image,
    _D3DFORMAT format,
    _D3DCUBEMAP_FACES face,
    uint32_t mipLevel,
    uint8_t *src);

void __cdecl Image_TrackFullscreenTexture(
    GfxImage *image,
    int fullscreenWidth,
    int fullscreenHeight,
    int picmip,
    _D3DFORMAT format);

void __cdecl Image_Reload(GfxImage *image);
void __cdecl Image_UpdatePicmip(GfxImage *image);
