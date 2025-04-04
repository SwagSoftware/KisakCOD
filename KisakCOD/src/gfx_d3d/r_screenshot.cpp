#include "r_screenshot.h"


void __cdecl TRACK_r_screenshot()
{
    track_static_alloc_internal((void *)cubemapShotAxis, 252, "cubemapShotAxis", 18);
    track_static_alloc_internal(&cubeShotGlob, 24, "cubeShotGlob", 18);
}

void __cdecl R_LevelShot()
{
    unsigned __int8 *buffer; // [esp+0h] [ebp-10Ch]
    char checkname[260]; // [esp+4h] [ebp-108h] BYREF

    sprintf(checkname, "levelshots/%s.tga", rgp.world->baseName);
    buffer = R_TakeResampledScreenshot(128, 128, 3, 18);
    if (buffer)
    {
        *(unsigned int *)buffer = 0;
        *((unsigned int *)buffer + 1) = 0;
        *((unsigned int *)buffer + 2) = 0;
        *((unsigned int *)buffer + 3) = 0;
        *((_WORD *)buffer + 8) = 0;
        buffer[2] = 2;
        buffer[12] = 0x80;
        buffer[14] = 0x80;
        buffer[16] = 24;
        FS_WriteFile(checkname, (char *)buffer, 0xC012u);
        Z_Free((char *)buffer, 22);
        Com_Printf(8, "Wrote %s\n", checkname);
    }
}

void __cdecl R_SaveGameShot(const char *saveName)
{
    char filename[256]; // [esp+0h] [ebp-110h] BYREF
    unsigned __int8 *pixels; // [esp+104h] [ebp-Ch]
    int width; // [esp+108h] [ebp-8h]
    int height; // [esp+10Ch] [ebp-4h]

    width = 512;
    height = 512;
    sprintf(filename, "%s.jpg", saveName);
    pixels = R_TakeResampledScreenshot(width, height, 3, 0);
    if (pixels)
    {
        R_SaveJpg(filename, 90, width, height, pixels);
        Z_Free((char *)pixels, 22);
    }
}

void __cdecl R_BeginCubemapShot(int pixelWidthHeight, int pixelBorder)
{
    if (pixelWidthHeight <= 0)
        MyAssertHandler(
            ".\\r_screenshot.cpp",
            909,
            0,
            "%s\n\t(pixelWidthHeight) = %i",
            "(pixelWidthHeight > 0)",
            pixelWidthHeight);
    if (pixelWidthHeight >= 0x10000)
        MyAssertHandler(
            ".\\r_screenshot.cpp",
            910,
            0,
            "%s\n\t(pixelWidthHeight) = %i",
            "(pixelWidthHeight < 65536)",
            pixelWidthHeight);
    if (pixelBorder < 0)
        MyAssertHandler(".\\r_screenshot.cpp", 911, 0, "%s\n\t(pixelBorder) = %i", "(pixelBorder >= 0)", pixelBorder);
    if (pixelBorder >= pixelWidthHeight)
        MyAssertHandler(
            ".\\r_screenshot.cpp",
            912,
            0,
            "%s\n\t(pixelBorder) = %i",
            "(pixelBorder < pixelWidthHeight)",
            pixelBorder);
    gfxMetrics.cubemapShotRes = pixelWidthHeight;
    gfxMetrics.cubemapShotPixelBorder = pixelBorder;
    R_CubemapShotSetInitialState();
}

void R_CubemapShotSetInitialState()
{
    const char *v0; // eax
    int hr; // [esp+8h] [ebp-4h]

    R_SetRenderTargetSize(&gfxCmdBufSourceState, R_RENDERTARGET_FRAME_BUFFER);
    R_SetRenderTarget(gfxCmdBufContext, R_RENDERTARGET_FRAME_BUFFER);
    do
    {
        if (r_logFile && r_logFile->current.integer)
            RB_LogPrint(
                "dx.device->Clear( 0, 0, 0x00000001l | 0x00000002l | 0x00000004l, ((D3DCOLOR)((((255)&0xff)<<24)|(((255)&0xff)<<1"
                "6)|(((0)&0xff)<<8)|((255)&0xff))), ((123987 / (((((123987 / ((((0 || 0 || (123987 / ((-123987)) == 123987 / (123"
                "987))) ? (123987) : (-123987)) * ((0 || 0 || (123987 / ((-123987)) == 123987 / (123987))) == 0 || (0 || 0 || (12"
                "3987 / ((-123987)) == 123987 / (123987))) == 1))) == 123987 / (123987))) ? (123987) : (-123987)) * (((123987 / ("
                "(((0 || 0 || (123987 / ((-123987)) == 123987 / (123987))) ? (123987) : (-123987)) * ((0 || 0 || (123987 / ((-123"
                "987)) == 123987 / (123987))) == 0 || (0 || 0 || (123987 / ((-123987)) == 123987 / (123987))) == 1))) == 123987 /"
                " (123987))) == 0 || ((123987 / ((((0 || 0 || (123987 / ((-123987)) == 123987 / (123987))) ? (123987) : (-123987)"
                ") * ((0 || 0 || (123987 / ((-123987)) == 123987 / (123987))) == 0 || (0 || 0 || (123987 / ((-123987)) == 123987 "
                "/ (123987))) == 1))) == 123987 / (123987))) == 1))) == 123987 / (123987)) ? 0.0f : 1.0f), 0 )\n");
        hr = ((int(__stdcall *)(IDirect3DDevice9 *, unsigned int, unsigned int, int, int, unsigned int, unsigned int))dx.device->Clear)(
            dx.device,
            0,
            0,
            7,
            -65281,
            1.0,
            0);
        if (hr < 0)
        {
            do
            {
                ++g_disableRendering;
                v0 = R_ErrorDescription(hr);
                Com_Error(
                    ERR_FATAL,
                    ".\\r_screenshot.cpp (%i) dx.device->Clear( 0, 0, 0x00000001l | 0x00000002l | 0x00000004l, ((D3DCOLOR)((((255)&"
                    "0xff)<<24)|(((255)&0xff)<<16)|(((0)&0xff)<<8)|((255)&0xff))), ((123987 / (((((123987 / ((((0 || 0 || (123987 /"
                    " ((-123987)) == 123987 / (123987))) ? (123987) : (-123987)) * ((0 || 0 || (123987 / ((-123987)) == 123987 / (1"
                    "23987))) == 0 || (0 || 0 || (123987 / ((-123987)) == 123987 / (123987))) == 1))) == 123987 / (123987))) ? (123"
                    "987) : (-123987)) * (((123987 / ((((0 || 0 || (123987 / ((-123987)) == 123987 / (123987))) ? (123987) : (-1239"
                    "87)) * ((0 || 0 || (123987 / ((-123987)) == 123987 / (123987))) == 0 || (0 || 0 || (123987 / ((-123987)) == 12"
                    "3987 / (123987))) == 1))) == 123987 / (123987))) == 0 || ((123987 / ((((0 || 0 || (123987 / ((-123987)) == 123"
                    "987 / (123987))) ? (123987) : (-123987)) * ((0 || 0 || (123987 / ((-123987)) == 123987 / (123987))) == 0 || (0"
                    " || 0 || (123987 / ((-123987)) == 123987 / (123987))) == 1))) == 123987 / (123987))) == 1))) == 123987 / (1239"
                    "87)) ? 0.0f : 1.0f), 0 ) failed: %s\n",
                    895,
                    v0);
            } while (alwaysfails);
        }
    } while (alwaysfails);
}

void __cdecl R_EndCubemapShot(CubemapShot shotIndex)
{
    R_CopyCubemapShot(shotIndex);
    R_CubemapShotRestoreState();
}

void __cdecl R_CopyCubemapShot(CubemapShot shotIndex)
{
    int sizeInBytes; // [esp+4h] [ebp-4h]

    if (shotIndex <= CUBEMAPSHOT_NONE || shotIndex >= CUBEMAPSHOT_COUNT)
        MyAssertHandler(
            ".\\r_screenshot.cpp",
            854,
            0,
            "%s\n\t(shotIndex) = %i",
            "(shotIndex > CUBEMAPSHOT_NONE && shotIndex < CUBEMAPSHOT_COUNT)",
            shotIndex);
    sizeInBytes = 4 * gfxMetrics.cubemapShotRes * gfxMetrics.cubemapShotRes;
    cubeShotGlob.pixels[shotIndex - 1] = (unsigned __int8 *)Z_VirtualAlloc(sizeInBytes, "R_CopyCubemapShot", 22);
    if (!cubeShotGlob.pixels[shotIndex - 1])
        MyAssertHandler(".\\r_screenshot.cpp", 860, 1, "%s", "cubeShotGlob.pixels[imgIndex]");
    R_CubemapShotCopySurfaceToBuffer(cubeShotGlob.pixels[shotIndex - 1], sizeInBytes);
}

void __cdecl R_CubemapShotCopySurfaceToBuffer(unsigned __int8 *buffer, int bufferSizeInBytes)
{
    if (!buffer)
        MyAssertHandler(".\\r_screenshot.cpp", 769, 0, "%s", "buffer");
    if (bufferSizeInBytes <= 0)
        MyAssertHandler(
            ".\\r_screenshot.cpp",
            770,
            0,
            "%s\n\t(bufferSizeInBytes) = %i",
            "(bufferSizeInBytes > 0)",
            bufferSizeInBytes);
    R_GetBackBufferData(
        gfxMetrics.cubemapShotPixelBorder,
        gfxMetrics.cubemapShotPixelBorder,
        gfxMetrics.cubemapShotRes,
        gfxMetrics.cubemapShotRes,
        4,
        buffer);
    R_CubemapShotFlipVerticalBuffer(buffer);
    if (vidConfig.deviceSupportsGamma)
        R_GammaCorrect(buffer, bufferSizeInBytes);
}

void R_CubemapShotRestoreState()
{
    R_SetRenderTargetSize(&gfxCmdBufSourceState, R_RENDERTARGET_FRAME_BUFFER);
    R_SetRenderTarget(gfxCmdBufContext, R_RENDERTARGET_FRAME_BUFFER);
}

void __cdecl R_SaveCubemapShot(char *filename, CubemapShot shotIndex, float n0, float n1)
{
    R_CubemapShotWriteTargaFile(filename, shotIndex, n0, n1);
}

void __cdecl R_CubemapShotWriteTargaFile(char *filename, CubemapShot shotIndex, float n0, float n1)
{
    int imgIndex; // [esp+Ch] [ebp-Ch]
    int fileSize; // [esp+10h] [ebp-8h]
    unsigned __int8 *targa; // [esp+14h] [ebp-4h]

    if (!filename)
        MyAssertHandler(".\\r_screenshot.cpp", 871, 0, "%s", "filename");
    if (shotIndex <= CUBEMAPSHOT_NONE || shotIndex >= CUBEMAPSHOT_COUNT)
        MyAssertHandler(
            ".\\r_screenshot.cpp",
            872,
            0,
            "%s\n\t(shotIndex) = %i",
            "(shotIndex > CUBEMAPSHOT_NONE && shotIndex < CUBEMAPSHOT_COUNT)",
            shotIndex);
    imgIndex = shotIndex - 1;
    fileSize = 4 * gfxMetrics.cubemapShotRes * gfxMetrics.cubemapShotRes + 18;
    targa = (unsigned __int8 *)Z_VirtualAlloc(fileSize, "R_CubemapShotWriteTargaFile", 22);
    if (!targa)
        MyAssertHandler(".\\r_screenshot.cpp", 877, 1, "%s", "targa");
    R_CubemapShotWriteTargaHeader(gfxMetrics.cubemapShotRes, targa);
    R_CubemapShotCopyBufferToTarga(cubeShotGlob.pixels[imgIndex], targa);
    R_CubemapShotApplyFresnelToTarga(shotIndex, n0, n1, targa);
    FS_WriteFile(filename, (char *)targa, fileSize);
    Z_VirtualFree(targa);
    Z_VirtualFree(cubeShotGlob.pixels[imgIndex]);
}

void __cdecl R_CubemapShotWriteTargaHeader(int res, unsigned __int8 *fileBuffer)
{
    if (!fileBuffer)
        MyAssertHandler(".\\r_screenshot.cpp", 785, 0, "%s", "fileBuffer");
    if (res <= 0)
        MyAssertHandler(".\\r_screenshot.cpp", 786, 0, "%s\n\t(res) = %i", "(res > 0)", res);
    *(unsigned int *)fileBuffer = 0;
    *((unsigned int *)fileBuffer + 1) = 0;
    *((unsigned int *)fileBuffer + 2) = 0;
    *((unsigned int *)fileBuffer + 3) = 0;
    *((_WORD *)fileBuffer + 8) = 0;
    fileBuffer[2] = 2;
    *((_WORD *)fileBuffer + 6) = res;
    fileBuffer[14] = fileBuffer[12];
    fileBuffer[15] = fileBuffer[13];
    fileBuffer[16] = 32;
}

void __cdecl R_CubemapShotCopyBufferToTarga(const unsigned __int8 *srcBuffer, unsigned __int8 *targa)
{
    int srcIndex; // [esp+0h] [ebp-10h]
    int colIndex; // [esp+4h] [ebp-Ch]
    int rowIndex; // [esp+8h] [ebp-8h]
    int dstIndex; // [esp+Ch] [ebp-4h]

    if (!srcBuffer)
        MyAssertHandler(".\\r_screenshot.cpp", 808, 0, "%s", "srcBuffer");
    if (!targa)
        MyAssertHandler(".\\r_screenshot.cpp", 809, 0, "%s", "targa");
    for (rowIndex = 0; rowIndex < gfxMetrics.cubemapShotRes; ++rowIndex)
    {
        for (colIndex = 0; colIndex < gfxMetrics.cubemapShotRes; ++colIndex)
        {
            srcIndex = 4 * (colIndex + rowIndex * gfxMetrics.cubemapShotRes);
            dstIndex = srcIndex + 18;
            targa[dstIndex + 2] = srcBuffer[srcIndex + 2];
            targa[dstIndex + 1] = srcBuffer[srcIndex + 1];
            targa[dstIndex] = srcBuffer[srcIndex];
            targa[dstIndex + 3] = srcBuffer[srcIndex + 3];
        }
    }
}

void __cdecl R_CubemapShotApplyFresnelToTarga(CubemapShot shotIndex, float n0, float n1, unsigned __int8 *targa)
{
    int colIndex; // [esp+8h] [ebp-10h]
    int rowIndex; // [esp+Ch] [ebp-Ch]
    int dstIndex; // [esp+10h] [ebp-8h]

    if (!targa)
        MyAssertHandler(".\\r_screenshot.cpp", 834, 0, "%s", "targa");
    if (shotIndex <= CUBEMAPSHOT_NONE || shotIndex >= CUBEMAPSHOT_COUNT)
        MyAssertHandler(
            ".\\r_screenshot.cpp",
            835,
            0,
            "%s\n\t(shotIndex) = %i",
            "(shotIndex > CUBEMAPSHOT_NONE && shotIndex < CUBEMAPSHOT_COUNT)",
            shotIndex);
    for (rowIndex = 0; rowIndex < gfxMetrics.cubemapShotRes; ++rowIndex)
    {
        for (colIndex = 0; colIndex < gfxMetrics.cubemapShotRes; ++colIndex)
        {
            dstIndex = 4 * (colIndex + rowIndex * gfxMetrics.cubemapShotRes) + 18;
            targa[dstIndex + 3] = R_CubemapShotCalcReflectionFactor(shotIndex, colIndex, rowIndex, n0, n1);
        }
    }
}

unsigned __int8 __cdecl R_CubemapShotCalcReflectionFactor(
    int shotIndex,
    int colIndex,
    int rowIndex,
    float n0,
    float n1)
{
    float v6; // [esp+Ch] [ebp-48h]
    float v7; // [esp+14h] [ebp-40h]
    float scale; // [esp+1Ch] [ebp-38h]
    float v9; // [esp+28h] [ebp-2Ch]
    float dir[3]; // [esp+44h] [ebp-10h] BYREF
    float refraction; // [esp+50h] [ebp-4h]

    if (shotIndex <= 0)
        MyAssertHandler(
            ".\\r_screenshot.cpp",
            719,
            0,
            "%s\n\t(shotIndex) = %i",
            "(shotIndex > CUBEMAPSHOT_NONE)",
            shotIndex);
    if (shotIndex >= 7)
        MyAssertHandler(
            ".\\r_screenshot.cpp",
            720,
            0,
            "%s\n\t(shotIndex) = %i",
            "(shotIndex < CUBEMAPSHOT_COUNT)",
            shotIndex);
    if (colIndex < 0)
        MyAssertHandler(".\\r_screenshot.cpp", 721, 0, "%s\n\t(colIndex) = %i", "(colIndex >= 0)", colIndex);
    if (colIndex >= gfxMetrics.cubemapShotRes)
        MyAssertHandler(
            ".\\r_screenshot.cpp",
            722,
            0,
            "%s\n\t(colIndex) = %i",
            "(colIndex < gfxMetrics.cubemapShotRes)",
            colIndex);
    if (rowIndex < 0)
        MyAssertHandler(".\\r_screenshot.cpp", 723, 0, "%s\n\t(rowIndex) = %i", "(rowIndex >= 0)", rowIndex);
    if (rowIndex >= gfxMetrics.cubemapShotRes)
        MyAssertHandler(
            ".\\r_screenshot.cpp",
            724,
            0,
            "%s\n\t(rowIndex) = %i",
            "(rowIndex < gfxMetrics.cubemapShotRes)",
            rowIndex);
    if (n0 == 0.0)
        MyAssertHandler(".\\r_screenshot.cpp", 725, 0, "%s\n\t(n0) = %g", "(n0 != 0)", n0);
    if (n1 == 0.0)
        MyAssertHandler(".\\r_screenshot.cpp", 726, 0, "%s\n\t(n1) = %g", "(n1 != 0)", n1);
    scale = (double)gfxMetrics.cubemapShotRes * 0.5;
    Vec3Scale(cubemapShotAxis[shotIndex][0], scale, dir);
    v7 = (double)colIndex - (double)gfxMetrics.cubemapShotRes * 0.5 + 0.5;
    Vec3Mad(dir, v7, (const float *)(36 * shotIndex + 9391748), dir);
    v6 = (double)rowIndex - (double)gfxMetrics.cubemapShotRes * 0.5 + 0.5;
    Vec3Mad(dir, v6, (const float *)(36 * shotIndex + 9391760), dir);
    Vec3Normalize(dir);
    refraction = FresnelTerm(n0, n1, dir[2]);
    v9 = refraction * 255.0;
    return (int)(v9 + 9.313225746154785e-10);
}

void __cdecl R_LightingFromCubemapShots(const float *baseColor)
{
    int pixelsPerFace; // [esp+0h] [ebp-1Ch]
    float (*linearColors[6])[3]; // [esp+4h] [ebp-18h] BYREF

    pixelsPerFace = gfxMetrics.cubemapShotRes * gfxMetrics.cubemapShotRes;
    linearColors[0] = (float (*)[3])Z_VirtualAlloc(72 * pixelsPerFace, "R_LightingFromCubemapShot", 22);
    linearColors[1] = (float (*)[3])linearColors[0][pixelsPerFace];
    linearColors[2] = (float (*)[3])linearColors[1][pixelsPerFace];
    linearColors[3] = (float (*)[3])linearColors[2][pixelsPerFace];
    linearColors[4] = (float (*)[3])linearColors[3][pixelsPerFace];
    linearColors[5] = (float (*)[3])linearColors[4][pixelsPerFace];
    R_CubemapShotExtractLinearLight(
        cubeShotGlob.pixels,
        gfxMetrics.cubemapShotRes,
        gfxMetrics.cubemapShotRes,
        linearColors);
    R_CubemapLighting(linearColors, gfxMetrics.cubemapShotRes, gfxMetrics.cubemapShotRes, baseColor, cubeShotGlob.pixels);
    Z_VirtualFree(linearColors[0]);
}

void __cdecl R_GetDirForCubemapPixel(int faceIndex, float x, float y, float *dir)
{
    Vec3Mad(cubemapShotAxis[faceIndex + 1][0], x, (const float *)(36 * (faceIndex + 1) + 9391748), dir);
    Vec3Mad(dir, y, (const float *)(36 * (faceIndex + 1) + 9391760), dir);
    Vec3Normalize(dir);
}

void __cdecl R_CubemapShotExtractLinearLight(
    unsigned __int8 **pixels,
    int width,
    int height,
    float (**linearColors)[3])
{
    float v4; // [esp+0h] [ebp-40h]
    float v5; // [esp+8h] [ebp-38h]
    float v6; // [esp+10h] [ebp-30h]
    float v7; // [esp+1Ch] [ebp-24h]
    float v8; // [esp+24h] [ebp-1Ch]
    float v9; // [esp+2Ch] [ebp-14h]
    int faceIndex; // [esp+30h] [ebp-10h]
    int x; // [esp+34h] [ebp-Ch]
    int y; // [esp+38h] [ebp-8h]
    int pixelIndex; // [esp+3Ch] [ebp-4h]

    for (faceIndex = 0; faceIndex < 6; ++faceIndex)
    {
        pixelIndex = 0;
        for (y = 0; y < height; ++y)
        {
            for (x = 0; x < width; ++x)
            {
                v9 = (double)pixels[faceIndex][4 * pixelIndex + 2] * 0.003921568859368563;
                v6 = pow(v9, 2.2);
                linearColors[faceIndex][pixelIndex][0] = v6;
                v8 = (double)pixels[faceIndex][4 * pixelIndex + 1] * 0.003921568859368563;
                v5 = pow(v8, 2.2);
                linearColors[faceIndex][pixelIndex][1] = v5;
                v7 = (double)pixels[faceIndex][4 * pixelIndex] * 0.003921568859368563;
                v4 = pow(v7, 2.2);
                linearColors[faceIndex][pixelIndex++][2] = v4;
            }
        }
    }
}

void __cdecl R_ScreenshotCommand(GfxScreenshotType type)
{
    const char *v1; // eax
    const char *v2; // eax
    char filename[260]; // [esp+44h] [ebp-110h] BYREF
    int silent; // [esp+14Ch] [ebp-8h]
    const char *extension; // [esp+150h] [ebp-4h]

    if (type)
    {
        if (type != R_SCREENSHOT_TGA)
        {
            if (!alwaysfails)
                MyAssertHandler(".\\r_screenshot.cpp", 651, 1, "unhandled case");
            return;
        }
        extension = "tga";
    }
    else
    {
        extension = "jpg";
    }
    if (!strcmp(Cmd_Argv(1), "levelshot"))
    {
        R_LevelShot();
        return;
    }
    if (!strcmp(Cmd_Argv(1), "savegame") && Cmd_Argc() == 3 && *Cmd_Argv(2))
    {
        v1 = Cmd_Argv(2);
        R_SaveGameShot(v1);
        return;
    }
    silent = strcmp(Cmd_Argv(1), "silent") == 0;
    if (Cmd_Argc() != 2 || silent)
    {
        while (lastNumber <= 9999)
        {
            R_ScreenshotFilename(lastNumber, extension, filename);
            if (!FS_FileExists(filename))
                break;
            ++lastNumber;
        }
        if (lastNumber >= 9999)
        {
            Com_Printf(8, "ScreenShot: Couldn't create a file\n");
            return;
        }
        ++lastNumber;
    }
    else
    {
        v2 = Cmd_Argv(1);
        Com_sprintf(filename, 0x100u, "screenshots/%s.%s", v2, extension);
    }
    if (type)
        R_TakeScreenshotTga(0, 0, vidConfig.displayWidth, vidConfig.displayHeight, filename);
    else
        R_TakeScreenshotJpg(0, 0, vidConfig.displayWidth, vidConfig.displayHeight, filename);
    if (!silent)
        Com_Printf(8, "Wrote %s\n", filename);
}