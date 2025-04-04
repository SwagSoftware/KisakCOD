#include "r_image.h"

unsigned int __cdecl Image_CountMipmapsForFile(const GfxImageFileHeader *fileHeader)
{
    return Image_CountMipmaps(
        fileHeader->flags,
        fileHeader->dimensions[0],
        fileHeader->dimensions[1],
        fileHeader->dimensions[2]);
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