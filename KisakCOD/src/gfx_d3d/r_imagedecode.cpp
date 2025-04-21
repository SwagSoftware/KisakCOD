#include "r_image.h"
#include <universal/com_memory.h>

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
    Com_AssembleFilepath("images/", imageName, ".iwi", filepath, 64);
    if (FS_ReadFile(filepath, &imageFile) < 0)
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
    image->pixels = Z_Malloc(4 * image->height * image->width, "Image_GetRawPixels", 22);
    imageData = imageFile + 1;
    switch (imageFile->format)
    {
    case 1u:
        image->hasAlpha = 1;
        Image_DecodeBitmap(image, imageFile, imageData, 4);
        break;
    case 2u:
        image->hasAlpha = 0;
        Image_DecodeBitmap(image, imageFile, imageData, 3);
        break;
    case 3u:
        image->hasAlpha = 1;
        Image_DecodeBitmap(image, imageFile, imageData, 2);
        break;
    case 4u:
        image->hasAlpha = 0;
        Image_DecodeBitmap(image, imageFile, imageData, 1);
        break;
    case 5u:
        image->hasAlpha = 1;
        Image_DecodeBitmap(image, imageFile, imageData, 1);
        break;
    case 6u:
        image->hasAlpha = 1;
        Image_DecodeWavelet(image, imageFile, imageData, 4);
        break;
    case 7u:
        image->hasAlpha = 0;
        Image_DecodeWavelet(image, imageFile, imageData, 3);
        break;
    case 8u:
        image->hasAlpha = 1;
        Image_DecodeWavelet(image, imageFile, imageData, 2);
        break;
    case 9u:
        image->hasAlpha = 0;
        Image_DecodeWavelet(image, imageFile, imageData, 1);
        break;
    case 0xAu:
        image->hasAlpha = 1;
        Image_DecodeWavelet(image, imageFile, imageData, 1);
        break;
    case 0xBu:
        image->hasAlpha = 0;
        Image_DecodeDxtc(image, imageFile, imageData, 8);
        break;
    case 0xCu:
    case 0xDu:
        image->hasAlpha = 1;
        Image_DecodeDxtc(image, imageFile, imageData, 16);
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
