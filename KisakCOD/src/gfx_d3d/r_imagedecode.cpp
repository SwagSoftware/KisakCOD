#include "r_image.h"
#include <universal/com_memory.h>
#include <universal/com_files.h>
#include <qcommon/qcommon.h>

unsigned int __cdecl Image_CountMipmapsForFile(const GfxImageFileHeader *fileHeader)
{
    return Image_CountMipmaps(
        fileHeader->flags,
        fileHeader->dimensions[0],
        fileHeader->dimensions[1],
        fileHeader->dimensions[2]);
}

void __cdecl Image_FreeRawPixels(GfxRawImage *image)
{
    Z_Free(image->pixels, 22);
}

void __cdecl Image_CopyBitmapData(GfxRawImage *image, GfxImageFileHeader *imageFile, unsigned __int8 *imageData)
{
    int pixelCount; // [esp+4h] [ebp-8h]
    GfxRawPixel *pixel; // [esp+8h] [ebp-4h]

    pixelCount = imageFile->dimensions[1] * imageFile->dimensions[0];
    pixel = image->pixels;
    switch (imageFile->format)
    {
    case 1u:
    case 6u:
        while (pixelCount)
        {
            pixel->r = imageData[2];
            pixel->g = imageData[1];
            pixel->b = *imageData;
            pixel->a = imageData[3];
            imageData += 4;
            ++pixel;
            --pixelCount;
        }
        break;
    case 2u:
    case 7u:
        while (pixelCount)
        {
            pixel->r = imageData[2];
            pixel->g = imageData[1];
            pixel->b = *imageData;
            pixel->a = -1;
            imageData += 3;
            ++pixel;
            --pixelCount;
        }
        break;
    case 3u:
    case 8u:
        while (pixelCount)
        {
            pixel->r = *imageData;
            pixel->g = *imageData;
            pixel->b = *imageData;
            pixel->a = imageData[1];
            imageData += 2;
            ++pixel;
            --pixelCount;
        }
        break;
    case 4u:
    case 9u:
        while (pixelCount)
        {
            pixel->r = *imageData;
            pixel->g = *imageData;
            pixel->b = *imageData;
            pixel->a = -1;
            ++imageData;
            ++pixel;
            --pixelCount;
        }
        break;
    case 5u:
    case 0xAu:
        while (pixelCount)
        {
            pixel->r = 0;
            pixel->g = 0;
            pixel->b = 0;
            pixel->a = *imageData++;
            ++pixel;
            --pixelCount;
        }
        break;
    default:
        if (!alwaysfails)
            MyAssertHandler(".\\r_imagedecode.cpp", 139, 1, "unhandled case");
        break;
    }
}

void __cdecl Image_DecodeBitmap(
    GfxRawImage *image,
    GfxImageFileHeader *imageFile,
    unsigned __int8 *imageData,
    int bytesPerPixel)
{
    int v4; // [esp+0h] [ebp-28h]
    int v5; // [esp+4h] [ebp-24h]
    int face; // [esp+1Ch] [ebp-Ch]
    int faceCount; // [esp+20h] [ebp-8h]
    int mipLevel; // [esp+24h] [ebp-4h]

    if (!image)
        MyAssertHandler(".\\r_imagedecode.cpp", 236, 0, "%s", "image");
    if (!imageFile)
        MyAssertHandler(".\\r_imagedecode.cpp", 237, 0, "%s", "imageFile");
    if ((imageFile->flags & 4) != 0)
        faceCount = 6;
    else
        faceCount = 1;
    for (mipLevel = Image_CountMipmapsForFile_0(imageFile) - 1; mipLevel >= 0; --mipLevel)
    {
        if (imageFile->dimensions[0] >> mipLevel > 1)
            v5 = imageFile->dimensions[0] >> mipLevel;
        else
            v5 = 1;
        if (imageFile->dimensions[1] >> mipLevel > 1)
            v4 = imageFile->dimensions[1] >> mipLevel;
        else
            v4 = 1;
        for (face = 0; face < faceCount; ++face)
        {
            if (!face && !mipLevel)
                Image_CopyBitmapData(image, imageFile, imageData);
            imageData += bytesPerPixel * v4 * v5;
        }
    }
}

void __cdecl Image_DecompressDxt1_Internal(
    DdsBlock_Dxt1_t *dxt1,
    GfxRawImage *image,
    int x,
    int y,
    bool nonTransparent)
{
    float v5; // [esp+1Ch] [ebp-1CCh]
    float v6; // [esp+30h] [ebp-1B8h]
    float v7; // [esp+44h] [ebp-1A4h]
    float v8; // [esp+58h] [ebp-190h]
    float v9; // [esp+6Ch] [ebp-17Ch]
    float v10; // [esp+80h] [ebp-168h]
    float v11; // [esp+94h] [ebp-154h]
    float v12; // [esp+A8h] [ebp-140h]
    float v13; // [esp+BCh] [ebp-12Ch]
    float v14; // [esp+D0h] [ebp-118h]
    float v15; // [esp+E4h] [ebp-104h]
    float v16; // [esp+F8h] [ebp-F0h]
    float v17; // [esp+10Ch] [ebp-DCh]
    float v18; // [esp+120h] [ebp-C8h]
    float v19; // [esp+134h] [ebp-B4h]
    float v20; // [esp+148h] [ebp-A0h]
    float v21; // [esp+15Ch] [ebp-8Ch]
    float v22; // [esp+170h] [ebp-78h]
    float v23; // [esp+184h] [ebp-64h]
    float v24; // [esp+198h] [ebp-50h]
    float v25; // [esp+1ACh] [ebp-3Ch]
    float r; // [esp+1BCh] [ebp-2Ch]
    float r_4; // [esp+1C0h] [ebp-28h]
    float g; // [esp+1C4h] [ebp-24h]
    float g_4; // [esp+1C8h] [ebp-20h]
    float b; // [esp+1CCh] [ebp-1Ch]
    float b_4; // [esp+1D0h] [ebp-18h]
    GfxRawPixel pixel[4]; // [esp+1D4h] [ebp-14h]
    int dy; // [esp+1E4h] [ebp-4h]

    r = (HIBYTE(dxt1->color0.rgb) >> 3) * 0.03125;
    g = ((dxt1->color0.rgb >> 5) & 0x3F) * 0.015625;
    b = (dxt1->color0.rgb & 0x1F) * 0.03125;
    r_4 = (HIBYTE(dxt1->color1.rgb) >> 3) * 0.03125;
    g_4 = ((dxt1->color1.rgb >> 5) & 0x3F) * 0.015625;
    b_4 = (dxt1->color1.rgb & 0x1F) * 0.03125;
    if (nonTransparent || dxt1->color0.rgb > dxt1->color1.rgb)
    {
        v25 = r * 255.0;
        pixel[0].r = (v25 + 9.313225746154785e-10);
        v24 = g * 255.0;
        pixel[0].g = (v24 + 9.313225746154785e-10);
        v23 = b * 255.0;
        pixel[0].b = (v23 + 9.313225746154785e-10);
        pixel[0].a = -1;
        v22 = r_4 * 255.0;
        pixel[1].r = (v22 + 9.313225746154785e-10);
        v21 = g_4 * 255.0;
        pixel[1].g = (v21 + 9.313225746154785e-10);
        v20 = b_4 * 255.0;
        pixel[1].b = (v20 + 9.313225746154785e-10);
        pixel[1].a = -1;
        v19 = r_4 * 85.0 + r * 170.0;
        pixel[2].r = (v19 + 9.313225746154785e-10);
        v18 = g_4 * 85.0 + g * 170.0;
        pixel[2].g = (v18 + 9.313225746154785e-10);
        v17 = b_4 * 85.0 + b * 170.0;
        pixel[2].b = (v17 + 9.313225746154785e-10);
        pixel[2].a = -1;
        v16 = r_4 * 170.0 + r * 85.0;
        pixel[3].r = (v16 + 9.313225746154785e-10);
        v15 = g_4 * 170.0 + g * 85.0;
        pixel[3].g = (v15 + 9.313225746154785e-10);
        v14 = b_4 * 170.0 + b * 85.0;
        pixel[3].b = (v14 + 9.313225746154785e-10);
        pixel[3].a = -1;
    }
    else
    {
        v13 = r * 255.0;
        pixel[0].r = (v13 + 9.313225746154785e-10);
        v12 = g * 255.0;
        pixel[0].g = (v12 + 9.313225746154785e-10);
        v11 = b * 255.0;
        pixel[0].b = (v11 + 9.313225746154785e-10);
        pixel[0].a = -1;
        v10 = r_4 * 255.0;
        pixel[1].r = (v10 + 9.313225746154785e-10);
        v9 = g_4 * 255.0;
        pixel[1].g = (v9 + 9.313225746154785e-10);
        v8 = b_4 * 255.0;
        pixel[1].b = (v8 + 9.313225746154785e-10);
        pixel[1].a = -1;
        v7 = r_4 * 127.5 + r * 127.5;
        pixel[2].r = (v7 + 9.313225746154785e-10);
        v6 = g_4 * 127.5 + g * 127.5;
        pixel[2].g = (v6 + 9.313225746154785e-10);
        v5 = b_4 * 127.5 + b * 127.5;
        pixel[2].b = (v5 + 9.313225746154785e-10);
        pixel[2].a = -1;
        pixel[3].r = 0;
        pixel[3].g = 0;
        pixel[3].b = 0;
        pixel[3].a = 0;
    }
    for (dy = 0; dy < 4; ++dy)
    {
        image->pixels[x + image->width * (dy + y)] = pixel[dxt1->bits[dy] & 3];
        image->pixels[x + 1 + image->width * (dy + y)] = pixel[(dxt1->bits[dy] >> 2) & 3];
        image->pixels[x + 2 + image->width * (dy + y)] = pixel[(dxt1->bits[dy] >> 4) & 3];
        image->pixels[x + 3 + image->width * (dy + y)] = pixel[(dxt1->bits[dy] >> 6) & 3];
    }
}

void __cdecl Image_DecompressDxt1(unsigned __int8 *block, GfxRawImage *image, int x, int y)
{
    Image_DecompressDxt1_Internal((DdsBlock_Dxt1_t*)block, image, x, y, 0);
}

void __cdecl Image_DecompressDxt3(unsigned __int8 *block, GfxRawImage *image, int x, int y)
{
    int dy; // [esp+4h] [ebp-4h]

    Image_DecompressDxt1_Internal((DdsBlock_Dxt1_t*)(block + 1), image, x, y, 1);
    for (dy = 0; dy < 4; ++dy)
    {
        image->pixels[x + image->width * (dy + y)].a = 17 * (block[2 * dy] & 0xF);
        image->pixels[x + 1 + image->width * (dy + y)].a = 17 * ((block[2 * dy] >> 4) & 0xF);
        image->pixels[x + 2 + image->width * (dy + y)].a = 17 * (block[2 * dy + 1] & 0xF);
        image->pixels[x + 3 + image->width * (dy + y)].a = 17 * ((block[2 * dy + 1] >> 4) & 0xF);
    }
}

void __cdecl Image_DecompressDxt5(unsigned __int8 *block, GfxRawImage *image, int x, int y)
{
    float v4; // [esp+30h] [ebp-E8h]
    float v5; // [esp+44h] [ebp-D4h]
    float v6; // [esp+58h] [ebp-C0h]
    float v7; // [esp+6Ch] [ebp-ACh]
    float v8; // [esp+80h] [ebp-98h]
    float v9; // [esp+94h] [ebp-84h]
    float v10; // [esp+A8h] [ebp-70h]
    float v11; // [esp+BCh] [ebp-5Ch]
    float v12; // [esp+D0h] [ebp-48h]
    float v13; // [esp+E4h] [ebp-34h]
    int i; // [esp+F4h] [ebp-24h]
    unsigned int used; // [esp+F8h] [ebp-20h]
    int bit; // [esp+FCh] [ebp-1Ch]
    unsigned __int8 a[8]; // [esp+104h] [ebp-14h]
    int dy; // [esp+110h] [ebp-8h]
    int sample; // [esp+114h] [ebp-4h]

    Image_DecompressDxt1_Internal((DdsBlock_Dxt1_t*)(block + 1), image, x, y, 1);
    a[0] = *block;
    a[1] = block[1];
    if (*block <= block[1])
    {
        v7 = (block[1] + 4 * *block) * 0.2000000029802322;
        a[2] = (v7 + 9.313225746154785e-10);
        v6 = (3 * *block + 2 * block[1]) * 0.2000000029802322;
        a[3] = (v6 + 9.313225746154785e-10);
        v5 = (3 * block[1] + 2 * *block) * 0.2000000029802322;
        a[4] = (v5 + 9.313225746154785e-10);
        v4 = (*block + 4 * block[1]) * 0.2000000029802322;
        a[5] = (v4 + 9.313225746154785e-10);
        a[6] = 0;
        a[7] = -1;
    }
    else
    {
        v13 = (block[1] + 6 * *block) * 0.1428571492433548;
        a[2] = (v13 + 9.313225746154785e-10);
        v12 = (5 * *block + 2 * block[1]) * 0.1428571492433548;
        a[3] = (v12 + 9.313225746154785e-10);
        v11 = (3 * block[1] + 4 * *block) * 0.1428571492433548;
        a[4] = (v11 + 9.313225746154785e-10);
        v10 = (3 * *block + 4 * block[1]) * 0.1428571492433548;
        a[5] = (v10 + 9.313225746154785e-10);
        v9 = (5 * block[1] + 2 * *block) * 0.1428571492433548;
        a[6] = (v9 + 9.313225746154785e-10);
        v8 = (6 * block[1] + *block) * 0.1428571492433548;
        a[7] = (v8 + 9.313225746154785e-10);
    }
    sample = *(block + 1);
    bit = 0;
    used = 2;
    for (dy = 0; dy < 4; ++dy)
    {
        for (i = 0; i < 4; ++i)
        {
            image->pixels[image->width * (dy + y) + i + x].a = a[(sample >> bit) & 7];
            bit += 3;
            if (bit >= 8)
            {
                bit -= 8;
                sample >>= 8;
                if (used < 6)
                {
                    sample |= block[used + 2] << 8;
                    ++used;
                }
            }
        }
    }
}

void __cdecl Image_CopyDxtcData(GfxRawImage *image, GfxImageFileHeader *imageFile, unsigned __int8 *imageData)
{
    unsigned __int8 format; // [esp+0h] [ebp-14h]
    int blockSize; // [esp+4h] [ebp-10h]
    void(__cdecl * DecompressDxtcBlock)(unsigned __int8 *, GfxRawImage *, int, int); // [esp+8h] [ebp-Ch]
    int x; // [esp+Ch] [ebp-8h]
    int y; // [esp+10h] [ebp-4h]

    format = imageFile->format;
    switch (format)
    {
    case 0xBu:
        blockSize = 8;
        DecompressDxtcBlock = Image_DecompressDxt1;
        break;
    case 0xCu:
        blockSize = 16;
        DecompressDxtcBlock = Image_DecompressDxt3;
        break;
    case 0xDu:
        blockSize = 16;
        DecompressDxtcBlock = Image_DecompressDxt5;
        break;
    default:
        if (!alwaysfails)
            MyAssertHandler(".\\r_imagedecode.cpp", 437, 1, "unhandled case");
        return;
    }
    for (y = 0; y < image->height; y += 4)
    {
        for (x = 0; x < image->width; x += 4)
        {
            DecompressDxtcBlock(imageData, image, x, y);
            imageData += blockSize;
        }
    }
}

void __cdecl Image_DecodeDxtc(
    GfxRawImage *image,
    GfxImageFileHeader *imageFile,
    unsigned __int8 *imageData,
    int bytesPerBlock)
{
    int v4; // [esp+0h] [ebp-28h]
    int v5; // [esp+4h] [ebp-24h]
    int face; // [esp+1Ch] [ebp-Ch]
    int faceCount; // [esp+20h] [ebp-8h]
    int mipLevel; // [esp+24h] [ebp-4h]

    if (!image)
        MyAssertHandler(".\\r_imagedecode.cpp", 461, 0, "%s", "image");
    if (!imageFile)
        MyAssertHandler(".\\r_imagedecode.cpp", 462, 0, "%s", "imageFile");
    if (bytesPerBlock != 8 * (imageFile->format != 11) + 8)
        MyAssertHandler(
            ".\\r_imagedecode.cpp",
            463,
            0,
            "%s",
            "bytesPerBlock == (imageFile->format == IMG_FORMAT_DXT1 ? 8 : 16)");
    if ((imageFile->flags & 4) != 0)
        faceCount = 6;
    else
        faceCount = 1;
    for (mipLevel = Image_CountMipmapsForFile_0(imageFile) - 1; mipLevel >= 0; --mipLevel)
    {
        if (imageFile->dimensions[0] >> mipLevel > 1)
            v5 = imageFile->dimensions[0] >> mipLevel;
        else
            v5 = 1;
        if (imageFile->dimensions[1] >> mipLevel > 1)
            v4 = imageFile->dimensions[1] >> mipLevel;
        else
            v4 = 1;
        for (face = 0; face < faceCount; ++face)
        {
            if (!face && !mipLevel)
                Image_CopyDxtcData(image, imageFile, imageData);
            imageData += bytesPerBlock * ((v4 + 3) >> 2) * ((v5 + 3) >> 2);
        }
    }
}

void __cdecl Image_GetRawPixels(char *imageName, GfxRawImage *image)
{
    char v2; // [esp+7h] [ebp-5Dh]
    GfxRawImage *v3; // [esp+Ch] [ebp-58h]
    char *v4; // [esp+10h] [ebp-54h]
    GfxImageFileHeader *imageData; // [esp+14h] [ebp-50h]
    char filepath[64]; // [esp+1Ch] [ebp-48h] BYREF
    GfxImageFileHeader *imageFile; // [esp+60h] [ebp-4h] BYREF

    if (!imageName)
        MyAssertHandler(".\\r_imagedecode.cpp", 492, 0, "%s", "imageName");
    Com_AssembleFilepath((char*)"images/", imageName, (char*)".iwi", filepath, 64);
    if (FS_ReadFile(filepath, (void**)&imageFile) < 0)
        Com_Error(ERR_DROP, "image '%s' is missing", filepath);
    if (!Image_ValidateHeader(imageFile, filepath))
        Com_Error(ERR_DROP, "image '%s' is not valid", filepath);
    v4 = imageName;
    v3 = image;
    do
    {
        v2 = *v4;
        v3->name[0] = *v4++;
        v3 = (v3 + 1);
    } while (v2);
    image->width = imageFile->dimensions[0];
    image->height = imageFile->dimensions[1];
    image->pixels = (GfxRawPixel*)Z_Malloc(4 * image->height * image->width, "Image_GetRawPixels", 22);
    imageData = imageFile + 1;
    switch (imageFile->format)
    {
    case 1u:
        image->hasAlpha = 1;
        Image_DecodeBitmap(image, imageFile, (unsigned char*)imageData, 4);
        break;
    case 2u:
        image->hasAlpha = 0;
        Image_DecodeBitmap(image, imageFile, (unsigned char *)imageData, 3);
        break;
    case 3u:
        image->hasAlpha = 1;
        Image_DecodeBitmap(image, imageFile, (unsigned char *)imageData, 2);
        break;
    case 4u:
        image->hasAlpha = 0;
        Image_DecodeBitmap(image, imageFile, (unsigned char *)imageData, 1);
        break;
    case 5u:
        image->hasAlpha = 1;
        Image_DecodeBitmap(image, imageFile, (unsigned char *)imageData, 1);
        break;
    case 6u:
        image->hasAlpha = 1;
        Image_DecodeWavelet(image, imageFile, (unsigned char *)imageData, 4);
        break;
    case 7u:
        image->hasAlpha = 0;
        Image_DecodeWavelet(image, imageFile, (unsigned char *)imageData, 3);
        break;
    case 8u:
        image->hasAlpha = 1;
        Image_DecodeWavelet(image, imageFile, (unsigned char *)imageData, 2);
        break;
    case 9u:
        image->hasAlpha = 0;
        Image_DecodeWavelet(image, imageFile, (unsigned char *)imageData, 1);
        break;
    case 0xAu:
        image->hasAlpha = 1;
        Image_DecodeWavelet(image, imageFile, (unsigned char *)imageData, 1);
        break;
    case 0xBu:
        image->hasAlpha = 0;
        Image_DecodeDxtc(image, imageFile, (unsigned char*)imageData, 8);
        break;
    case 0xCu:
    case 0xDu:
        image->hasAlpha = 1;
        Image_DecodeDxtc(image, imageFile, (unsigned char *)imageData, 16);
        break;
    default:
        if (!alwaysfails)
            MyAssertHandler(".\\r_imagedecode.cpp", 565, 1, "unhandled case");
        break;
    }
    FS_FreeFile(imageFile->tag);
}

int __cdecl Image_CountMipmapsForFile(GfxImageFileHeader *imageFile)
{
    int v2; // [esp+0h] [ebp-1Ch]
    int v3; // [esp+4h] [ebp-18h]
    int mipCount; // [esp+10h] [ebp-Ch]
    int width; // [esp+14h] [ebp-8h]
    int height; // [esp+18h] [ebp-4h]

    if ((imageFile->flags & 2) != 0)
        return 1;
    mipCount = 1;
    width = imageFile->dimensions[0];
    height = imageFile->dimensions[1];
    if (imageFile->dimensions[2] != 1)
        MyAssertHandler(".\\r_imagedecode.cpp", 157, 1, "%s", "imageFile->dimensions[2] == 1");
    while (width > 1 || height > 1)
    {
        if (width >> 1 > 1)
            v3 = width >> 1;
        else
            v3 = 1;
        width = v3;
        if (height >> 1 > 1)
            v2 = height >> 1;
        else
            v2 = 1;
        height = v2;
        ++mipCount;
    }
    return mipCount;
}

int __cdecl Image_CountMipmapsForFile_0(GfxImageFileHeader *imageFile)
{
    int v2; // [esp+0h] [ebp-1Ch]
    int v3; // [esp+4h] [ebp-18h]
    int mipCount; // [esp+10h] [ebp-Ch]
    int width; // [esp+14h] [ebp-8h]
    int height; // [esp+18h] [ebp-4h]

    if ((imageFile->flags & 2) != 0)
        return 1;
    mipCount = 1;
    width = imageFile->dimensions[0];
    height = imageFile->dimensions[1];
    if (imageFile->dimensions[2] != 1)
        MyAssertHandler(".\\r_imagedecode.cpp", 157, 1, "%s", "imageFile->dimensions[2] == 1");
    while (width > 1 || height > 1)
    {
        if (width >> 1 > 1)
            v3 = width >> 1;
        else
            v3 = 1;
        width = v3;
        if (height >> 1 > 1)
            v2 = height >> 1;
        else
            v2 = 1;
        height = v2;
        ++mipCount;
    }
    return mipCount;
}

void __cdecl Image_CopyBitmapData(GfxRawImage *image, GfxImageFileHeader *imageFile, unsigned __int8 *imageData)
{
    int pixelCount; // [esp+4h] [ebp-8h]
    GfxRawPixel *pixel; // [esp+8h] [ebp-4h]

    pixelCount = imageFile->dimensions[1] * imageFile->dimensions[0];
    pixel = image->pixels;
    switch (imageFile->format)
    {
    case 1u:
    case 6u:
        while (pixelCount)
        {
            pixel->r = imageData[2];
            pixel->g = imageData[1];
            pixel->b = *imageData;
            pixel->a = imageData[3];
            imageData += 4;
            ++pixel;
            --pixelCount;
        }
        break;
    case 2u:
    case 7u:
        while (pixelCount)
        {
            pixel->r = imageData[2];
            pixel->g = imageData[1];
            pixel->b = *imageData;
            pixel->a = -1;
            imageData += 3;
            ++pixel;
            --pixelCount;
        }
        break;
    case 3u:
    case 8u:
        while (pixelCount)
        {
            pixel->r = *imageData;
            pixel->g = *imageData;
            pixel->b = *imageData;
            pixel->a = imageData[1];
            imageData += 2;
            ++pixel;
            --pixelCount;
        }
        break;
    case 4u:
    case 9u:
        while (pixelCount)
        {
            pixel->r = *imageData;
            pixel->g = *imageData;
            pixel->b = *imageData;
            pixel->a = -1;
            ++imageData;
            ++pixel;
            --pixelCount;
        }
        break;
    case 5u:
    case 0xAu:
        while (pixelCount)
        {
            pixel->r = 0;
            pixel->g = 0;
            pixel->b = 0;
            pixel->a = *imageData++;
            ++pixel;
            --pixelCount;
        }
        break;
    default:
        if (!alwaysfails)
            MyAssertHandler(".\\r_imagedecode.cpp", 139, 1, "unhandled case");
        break;
    }
}

void __cdecl Image_DecodeWavelet(
    GfxRawImage *image,
    GfxImageFileHeader *imageFile,
    unsigned __int8 *imageData,
    int bytesPerPixel)
{
    unsigned __int8 *TempMemory; // eax
    int v5; // [esp+0h] [ebp-90h]
    int v6; // [esp+4h] [ebp-8Ch]
    unsigned __int8 *from[6]; // [esp+10h] [ebp-80h]
    unsigned __int8 *pixels[6]; // [esp+28h] [ebp-68h]
    int sizeForLevel; // [esp+40h] [ebp-50h]
    int width; // [esp+44h] [ebp-4Ch]
    int height; // [esp+48h] [ebp-48h]
    int face; // [esp+4Ch] [ebp-44h]
    int faceCount; // [esp+50h] [ebp-40h]
    WaveletDecode decode; // [esp+54h] [ebp-3Ch] BYREF
    unsigned __int8 *to[6]; // [esp+74h] [ebp-1Ch]
    int totalSize; // [esp+8Ch] [ebp-4h]

    if (!image)
        MyAssertHandler(".\\r_imagedecode.cpp", 181, 0, "%s", "image");
    if (!imageFile)
        MyAssertHandler(".\\r_imagedecode.cpp", 182, 0, "%s", "imageFile");
    decode.value = 0;
    decode.bit = 0;
    decode.width = imageFile->dimensions[0];
    decode.height = imageFile->dimensions[1];
    decode.mipLevel = Image_CountMipmapsForFile_0(imageFile) - 1;
    decode.channels = bytesPerPixel;
    decode.bpp = bytesPerPixel;
    decode.dataInitialized = 0;
    if ((imageFile->flags & 4) != 0)
        faceCount = 6;
    else
        faceCount = 1;
    totalSize = bytesPerPixel * imageFile->dimensions[1] * imageFile->dimensions[0];
    for (face = 0; face < faceCount; ++face)
    {
        TempMemory = (unsigned __int8 *)Hunk_AllocateTempMemory(totalSize, "Image_LoadWavelet");
        pixels[face] = TempMemory;
        to[face] = 0;
    }
    decode.data = (char *)imageData;
    while (decode.mipLevel >= 0)
    {
        if (decode.width >> SLOBYTE(decode.mipLevel) > 1)
            v6 = decode.width >> SLOBYTE(decode.mipLevel);
        else
            v6 = 1;
        width = v6;
        if (decode.height >> SLOBYTE(decode.mipLevel) > 1)
            v5 = decode.height >> SLOBYTE(decode.mipLevel);
        else
            v5 = 1;
        height = v5;
        sizeForLevel = bytesPerPixel * v5 * width;
        for (face = 0; face < faceCount; ++face)
        {
            from[face] = to[face];
            to[face] = &pixels[face][totalSize - sizeForLevel];
            Wavelet_DecompressLevel(from[face], to[face], &decode);
            if (!face && !decode.mipLevel)
                Image_CopyBitmapData(image, imageFile, to[face]);
        }
        --decode.mipLevel;
    }
    for (face = faceCount - 1; face >= 0; --face)
        Hunk_FreeTempMemory((char *)pixels[face]);
}
