#include "r_reflection_probe.h"
#include "r_dvars.h"
#include <qcommon/com_bsp.h>
#include "r_bsp.h"
#include "r_screenshot.h"
#include "r_rendercmds.h"
#include <EffectsCore/fx_system.h>
#include "r_scene.h"
#include "r_dpvs.h"
#include "r_workercmds_common.h"
#include <universal/com_files.h>
#include <universal/q_parse.h>
#include "r_image.h"
#include <win32/win_local.h>

int s_numColorCorrectionDataEntries;
ColorCorrectionData s_colorCorrectionDataEntries[1024];

char __cdecl R_CopyReflectionsFromLumpData(
    DiskGfxReflectionProbe *probeRawData,
    const DiskGfxReflectionProbe *probeRawLumpData,
    int lumpProbeCount)
{
    const DiskGfxReflectionProbe *v5; // [esp+4h] [ebp-8h]
    int probeIndex; // [esp+8h] [ebp-4h]

    for (probeIndex = 0; probeIndex < lumpProbeCount; ++probeIndex)
    {
        v5 = &probeRawLumpData[probeIndex];
        if (v5->origin[0] == probeRawData->origin[0]
            && v5->origin[1] == probeRawData->origin[1]
            && v5->origin[2] == probeRawData->origin[2])
        {
            memcpy(probeRawData->pixels, probeRawLumpData[probeIndex].pixels, sizeof(probeRawData->pixels));
            return 1;
        }
    }
    return 0;
}

void __cdecl R_CalcCubeMapViewValues(refdef_s *refdef, CubemapShot cubemapShot, int cubemapSize)
{
    refdef->x = 0;
    refdef->y = 0;
    refdef->width = cubemapSize + 2;
    refdef->height = cubemapSize + 2;
    refdef->tanHalfFovX = (cubemapSize + 2) / cubemapSize;
    refdef->tanHalfFovY = refdef->tanHalfFovX;
    refdef->zNear = 0.0;
    switch (cubemapShot)
    {
    case CUBEMAPSHOT_RIGHT:
        refdef->viewaxis[0][0] = 1.0;
        refdef->viewaxis[0][1] = 0.0;
        refdef->viewaxis[0][2] = 0.0;
        refdef->viewaxis[1][0] = 0.0;
        refdef->viewaxis[1][1] = 1.0;
        refdef->viewaxis[1][2] = 0.0;
        refdef->viewaxis[2][0] = 0.0;
        refdef->viewaxis[2][1] = 0.0;
        refdef->viewaxis[2][2] = 1.0;
        break;
    case CUBEMAPSHOT_LEFT:
        refdef->viewaxis[0][0] = -1.0;
        refdef->viewaxis[0][1] = 0.0;
        refdef->viewaxis[0][2] = 0.0;
        refdef->viewaxis[1][0] = 0.0;
        refdef->viewaxis[1][1] = -1.0;
        refdef->viewaxis[1][2] = 0.0;
        refdef->viewaxis[2][0] = 0.0;
        refdef->viewaxis[2][1] = 0.0;
        refdef->viewaxis[2][2] = 1.0;
        break;
    case CUBEMAPSHOT_FRONT:
        refdef->viewaxis[0][0] = 0.0;
        refdef->viewaxis[0][1] = -1.0;
        refdef->viewaxis[0][2] = 0.0;
        refdef->viewaxis[1][0] = 1.0;
        refdef->viewaxis[1][1] = 0.0;
        refdef->viewaxis[1][2] = 0.0;
        refdef->viewaxis[2][0] = 0.0;
        refdef->viewaxis[2][1] = 0.0;
        refdef->viewaxis[2][2] = 1.0;
        break;
    case CUBEMAPSHOT_UP:
        refdef->viewaxis[0][0] = 0.0;
        refdef->viewaxis[0][1] = 0.0;
        refdef->viewaxis[0][2] = 1.0;
        refdef->viewaxis[1][0] = 0.0;
        refdef->viewaxis[1][1] = 1.0;
        refdef->viewaxis[1][2] = 0.0;
        refdef->viewaxis[2][0] = -1.0;
        refdef->viewaxis[2][1] = 0.0;
        refdef->viewaxis[2][2] = 0.0;
        break;
    case CUBEMAPSHOT_DOWN:
        refdef->viewaxis[0][0] = 0.0;
        refdef->viewaxis[0][1] = 0.0;
        refdef->viewaxis[0][2] = -1.0;
        refdef->viewaxis[1][0] = 0.0;
        refdef->viewaxis[1][1] = 1.0;
        refdef->viewaxis[1][2] = 0.0;
        refdef->viewaxis[2][0] = 1.0;
        refdef->viewaxis[2][1] = 0.0;
        refdef->viewaxis[2][2] = 0.0;
        break;
    default:
        if (cubemapShot != CUBEMAPSHOT_BACK)
            MyAssertHandler(
                ".\\r_reflection_probe.cpp",
                78,
                1,
                "%s\n\t(cubemapShot) = %i",
                "(cubemapShot == CUBEMAPSHOT_BACK)",
                cubemapShot);
        refdef->viewaxis[0][0] = 0.0;
        refdef->viewaxis[0][1] = 1.0;
        refdef->viewaxis[0][2] = 0.0;
        refdef->viewaxis[1][0] = -1.0;
        refdef->viewaxis[1][1] = 0.0;
        refdef->viewaxis[1][2] = 0.0;
        refdef->viewaxis[2][0] = 0.0;
        refdef->viewaxis[2][1] = 0.0;
        refdef->viewaxis[2][2] = 1.0;
        break;
    }
}

void __cdecl R_BspGenerateReflections()
{
    R_GenerateReflections((char*)rgp.world->name, rgp.world->reflectionProbes + 1, rgp.world->reflectionProbeCount - 1);
}

void __cdecl R_GenerateReflectionRawData(DiskGfxReflectionProbe *probeRawData)
{
    float zfar; // [esp+0h] [ebp-40B8h]
    FxCmd cmd; // [esp+4h] [ebp-40B4h] BYREF
    CubemapShot cubemapShot; // [esp+10h] [ebp-40A8h]
    int v4; // [esp+14h] [ebp-40A4h]
    refdef_s dst; // [esp+18h] [ebp-40A0h] BYREF

    v4 = 256;
    memset(&dst, 0, sizeof(dst));
    dst.vieworg[0] = probeRawData->origin[0];
    dst.vieworg[1] = probeRawData->origin[1];
    dst.vieworg[2] = probeRawData->origin[2];
    dst.localClientNum = 0;
    dst.time = 0;
    dst.blurRadius = 0.0;
    dst.useScissorViewport = 0;
    R_InitPrimaryLights(dst.primaryLights);
    for (cubemapShot = CUBEMAPSHOT_RIGHT; cubemapShot < CUBEMAPSHOT_COUNT; ++cubemapShot)
    {
        R_BeginCubemapShot(256, 1);
        R_BeginFrame();
        R_BeginSharedCmdList();
        R_ClearClientCmdList2D();
        R_ClearScene(0);
        FX_BeginUpdate(0);
        R_CalcCubeMapViewValues(&dst, cubemapShot, 256);
        R_SetLodOrigin(&dst);
        zfar = R_GetFarPlaneDist();
        FX_SetNextUpdateCamera(0, &dst, zfar);
        FX_FillUpdateCmd(0, &cmd);
        R_UpdateSpotLightEffect(&cmd);
        R_UpdateNonDependentEffects(&cmd);
        R_UpdateRemainingEffects(&cmd);
        R_RenderScene(&dst);
        R_EndFrame();
        R_IssueRenderCommands(0xFFFFFFFF);
        R_EndCubemapShot(cubemapShot);
    }
    R_CreateReflectionRawDataFromCubemapShot(probeRawData, 64);
}

void __cdecl R_GenerateReflectionRawDataAll(DiskGfxReflectionProbe *probeRawData, int probeCount, bool *generateProbe)
{
    int probeIndex; // [esp+0h] [ebp-4h]

    for (probeIndex = 0; probeIndex < probeCount; ++probeIndex)
    {
        if (generateProbe[probeIndex])
            R_GenerateReflectionRawData(&probeRawData[probeIndex]);
    }
}

const char *fields_2[6] = { "name", "black_level", "white_level", "gamma", "saturation", NULL }; // idb
char __cdecl R_VerifyFieldNames(const char **buf, const char *filename)
{
    int fieldIndex; // [esp+0h] [ebp-8h]
    parseInfo_t *token; // [esp+4h] [ebp-4h]

    for (fieldIndex = 0; fields_2[fieldIndex]; ++fieldIndex)
    {
        token = Com_Parse(buf);
        if (I_stricmp(fields_2[fieldIndex], token->token))
        {
            Com_PrintError(1, "R_VerifyFieldNames: file %s column header %d was %s instead of %s", filename, fieldIndex, token, fields_2[fieldIndex]);
            return 0;
        }
    }
    Com_SkipRestOfLine(buf);
    return 1;
}

void __cdecl R_ParseColorCorrectionData(const char *buf, const char *filename)
{
    ColorCorrectionData *ccd; // [esp+10h] [ebp-8h]
    parseInfo_t *token; // [esp+14h] [ebp-4h]

    iassert( s_numColorCorrectionDataEntries == 0 );
    if (R_VerifyFieldNames(&buf, filename))
    {
        while (1)
        {
            token = Com_Parse(&buf);
            if (!buf)
                break;
            if (s_numColorCorrectionDataEntries == 1024)
            {
                Com_PrintError(1, "R_ParseColorCorrectionData: file %s max color correction entries [%d] exceeded. Ignoring the rest of the file", filename, 1024, filename);
                return;
            }
            ccd = &s_colorCorrectionDataEntries[s_numColorCorrectionDataEntries++];
            iassert( token );
            if (strlen(token->token) >= 0x40)
                Com_PrintError(1, "R_ParseColorCorrectionData: file %s truncating name because %s is too longer than %d", filename, token, 64);
            I_strncpyz(ccd->name, token->token, 64);
            ccd->black_level = Com_ParseFloatOnLine(&buf);
            ccd->white_level = Com_ParseFloatOnLine(&buf);
            ccd->gamma = Com_ParseFloatOnLine(&buf);
            ccd->saturation = Com_ParseFloatOnLine(&buf);
            ccd->range = ccd->white_level - ccd->black_level;
            Com_SkipRestOfLine(&buf);
        }
    }
}

void R_LoadColorCorrectionData()
{
    unsigned __int8 *filebuf; // [esp+4h] [ebp-Ch]
    int fileSize; // [esp+8h] [ebp-8h]
    int f; // [esp+Ch] [ebp-4h] BYREF

    fileSize = FS_FOpenFileByMode((char*)"reflections/reflections.csv", &f, FS_READ);
    if (fileSize >= 0)
    {
        Hunk_CheckTempMemoryHighClear();
        filebuf = (unsigned __int8 *)Hunk_AllocateTempMemoryHigh(fileSize + 1, "R_LoadColorCorrectionData");
        FS_Read(filebuf, fileSize, f);
        FS_FCloseFile(f);
        filebuf[fileSize] = 0;
        Com_BeginParseSession("reflections/reflections.csv");
        Com_SetCSV(1);
        R_ParseColorCorrectionData((const char*)filebuf, "reflections/reflections.csv");
        Com_EndParseSession();
        Hunk_ClearTempMemoryHigh();
    }
    else
    {
        Com_PrintError(1, "R_LoadColorCorrectionData: failed to open %s", "reflections/reflections.csv");
    }
}

ColorCorrectionData *R_CreateDefaultColorCorrectionEntry()
{
    ColorCorrectionData *result; // eax
    ColorCorrectionData *ccd; // [esp+0h] [ebp-4h]

    iassert( s_numColorCorrectionDataEntries == 0 );
    ccd = &s_colorCorrectionDataEntries[s_numColorCorrectionDataEntries++];
    I_strncpyz(ccd->name, "default", 64);
    ccd->black_level = 0.0;
    ccd->white_level = 1.0;
    ccd->gamma = 1.0;
    ccd->saturation = 1.0;
    result = ccd;
    ccd->range = ccd->white_level - ccd->black_level;
    return result;
}

const ColorCorrectionData *__cdecl R_FindColorCorrectionData(const char *name)
{
    int i; // [esp+0h] [ebp-4h]

    iassert( name );
    if (!*name)
        return R_FindColorCorrectionData("default");
    for (i = 0; i < s_numColorCorrectionDataEntries; ++i)
    {
        if (!I_stricmp(s_colorCorrectionDataEntries[i].name, name))
            return &s_colorCorrectionDataEntries[i];
    }
    Com_PrintError(1, "R_FindColorCorrectionData: failed to find color correction entry %s. Using %s instead.", name, s_colorCorrectionDataEntries);
    return s_colorCorrectionDataEntries;
}

const float colorIntensityScale[3] = { 0.114f, 0.587f, 0.299f };

void __cdecl R_ColorCorrectBGRAPixel(const ColorCorrectionData *ccd, const unsigned __int8 *from, unsigned __int8 *to)
{
    float v3; // [esp+18h] [ebp-64h]
    float v4; // [esp+1Ch] [ebp-60h]
    float v5; // [esp+20h] [ebp-5Ch]
    float v6; // [esp+24h] [ebp-58h]
    float v7; // [esp+28h] [ebp-54h]
    float v8; // [esp+2Ch] [ebp-50h]
    float v9; // [esp+30h] [ebp-4Ch]
    float v10; // [esp+34h] [ebp-48h]
    float v11; // [esp+3Ch] [ebp-40h]
    float v12; // [esp+40h] [ebp-3Ch]
    float v13; // [esp+44h] [ebp-38h]
    float v14; // [esp+54h] [ebp-28h]
    int m; // [esp+58h] [ebp-24h]
    int k; // [esp+5Ch] [ebp-20h]
    int j; // [esp+60h] [ebp-1Ch]
    int i; // [esp+64h] [ebp-18h]
    float intensity; // [esp+68h] [ebp-14h]
    float maxIntensity; // [esp+6Ch] [ebp-10h]
    float color[3]; // [esp+70h] [ebp-Ch] BYREF

    iassert( ccd );
    for (i = 0; i < 3; ++i)
    {
        color[i] = from[i] / 255.0;
        color[i] = (color[i] - ccd->black_level) / ccd->range;
        v14 = color[i];
        v10 = v14 - 0.0;
        if (v10 < 0.0)
            v9 = 0.0;
        else
            v9 = v14;
        color[i] = v9;
        v8 = pow(color[i], ccd->gamma);
        color[i] = v8;
    }
    intensity = Vec3Dot(color, colorIntensityScale);
    for (j = 0; j < 3; ++j)
        color[j] = ccd->saturation * color[j] + (1.0 - ccd->saturation) * intensity;
    maxIntensity = 0.1f;
    for (k = 0; k < 3; ++k)
    {
        v13 = color[k];
        v7 = 4.0 - v13;
        if (v7 < 0.0)
            v6 = 4.0;
        else
            v6 = v13;
        color[k] = v6;
        if (maxIntensity < color[k])
            maxIntensity = color[k];
    }
    for (m = 0; m < 3; ++m)
    {
        color[m] = color[m] / maxIntensity;
        v11 = color[m];
        v5 = v11 - 1.0;
        if (v5 < 0.0)
            v12 = v11;
        else
            v12 = 1.0;
        v4 = 0.0 - v11;
        if (v4 < 0.0)
            v3 = v12;
        else
            v3 = 0.0;
        color[m] = v3;
        to[m] = (color[m] * 255.0);
    }
    to[3] = (maxIntensity / 4.0 * 255.0);
}

void __cdecl R_CopyBlockFromBgraToPixelColorWithColorCorrection(
    const ColorCorrectionData *colorCorrectionData,
    unsigned __int8 *to,
    const unsigned __int8 *from,
    unsigned int blockSize)
{
    unsigned int offset; // [esp+0h] [ebp-4h]

    iassert( blockSize );
    iassert( blockSize % 4 == 0 );
    offset = 0;
    do
    {
        R_ColorCorrectBGRAPixel(colorCorrectionData, &from[offset], &to[offset]);
        offset += 4;
    } while (offset != blockSize);
}

GfxImage *__cdecl R_GenerateReflectionImageFromRawData(const unsigned __int8 *rawPixels, int probeIndex)
{
    char *v2; // eax
    int v4; // [esp+0h] [ebp-188h]
    const unsigned __int8 *pixels[6][15]; // [esp+8h] [ebp-180h] BYREF
    int imgIndex; // [esp+170h] [ebp-18h]
    int mipmapLevelSize; // [esp+174h] [ebp-14h]
    GfxImage *reflectionImage; // [esp+178h] [ebp-10h]
    _D3DFORMAT imageFormat; // [esp+17Ch] [ebp-Ch]
    int scaledSize; // [esp+180h] [ebp-8h]
    int mipLevel; // [esp+184h] [ebp-4h]

    imageFormat = D3DFMT_A8R8G8B8;
    mipLevel = 0;
    mipmapLevelSize = 0x4000;
    for (imgIndex = 0; imgIndex < 6; ++imgIndex)
    {
        pixels[imgIndex][0] = rawPixels;
        rawPixels += mipmapLevelSize;
    }
    for (imgIndex = 0; imgIndex < 6; ++imgIndex)
    {
        scaledSize = 64;
        mipLevel = 1;
        do
        {
            if (scaledSize >> 1 > 1)
                v4 = scaledSize >> 1;
            else
                v4 = 1;
            scaledSize = v4;
            mipmapLevelSize = v4 * 4 * v4;
            pixels[imgIndex][mipLevel] = rawPixels;
            rawPixels += mipmapLevelSize;
            ++mipLevel;
        } while (scaledSize != 1);
    }
    v2 = va("*reflection_probe%i", probeIndex);
    reflectionImage = Image_Alloc(v2, 1u, 1u, 0);
    iassert( reflectionImage );
    Image_GenerateCube(reflectionImage, pixels, 64, imageFormat, mipLevel);
    return reflectionImage;
}

void __cdecl R_GenerateReflectionImages(
    GfxReflectionProbe *probes,
    const DiskGfxReflectionProbe *probeRawData,
    int probeCount,
    int probeBaseIndex)
{
    const ColorCorrectionData *ColorCorrectionData; // eax
    GfxReflectionProbe *v5; // [esp+0h] [ebp-14h]
    const DiskGfxReflectionProbe *v6; // [esp+4h] [ebp-10h]
    unsigned __int8 *pixels; // [esp+Ch] [ebp-8h]
    int probeIndex; // [esp+10h] [ebp-4h]

    if (!s_numColorCorrectionDataEntries)
        R_LoadColorCorrectionData();
    if (!s_numColorCorrectionDataEntries)
        R_CreateDefaultColorCorrectionEntry();
    pixels = (unsigned __int8 *)Hunk_AllocateTempMemory(131064, "R_GenerateReflectionImages");
    for (probeIndex = 0; probeIndex < probeCount; ++probeIndex)
    {
        ColorCorrectionData = R_FindColorCorrectionData(probeRawData[probeIndex].colorCorrectionFilename);
        v5 = &probes[probeIndex];
        v6 = &probeRawData[probeIndex];
        v5->origin[0] = v6->origin[0];
        v5->origin[1] = v6->origin[1];
        v5->origin[2] = v6->origin[2];
        R_CopyBlockFromBgraToPixelColorWithColorCorrection(ColorCorrectionData, pixels, v6->pixels, 0x1FFF8u);
        v5->reflectionImage = R_GenerateReflectionImageFromRawData(pixels, probeBaseIndex + probeIndex);
    }
    Hunk_FreeTempMemory((char*)pixels);
}

bool __cdecl R_ReflectionProbeGenerateExitWhenDone()
{
    return r_reflectionProbeGenerate->current.enabled && r_reflectionProbeGenerateExit->current.enabled;
}

void __cdecl R_GenerateReflections(char *mapname, GfxReflectionProbe *probes, unsigned int probeCount)
{
    char v3; // al
    DiskGfxReflectionProbe *v4; // [esp+8h] [ebp-124h]
    GfxReflectionProbe *v5; // [esp+Ch] [ebp-120h]
    unsigned int lumpProbeCount; // [esp+10h] [ebp-11Ch] BYREF
    unsigned int version; // [esp+14h] [ebp-118h]
    const DiskGfxReflectionProbe *probeRawLumpData; // [esp+18h] [ebp-114h]
    DiskGfxReflectionProbe *probeRawGeneratedData; // [esp+1Ch] [ebp-110h]
    unsigned int probeIndex; // [esp+20h] [ebp-10Ch]
    bool generateProbe[256]; // [esp+24h] [ebp-108h] BYREF
    unsigned int lumpSize; // [esp+128h] [ebp-4h]

    iassert( r_reflectionProbeGenerate );
    iassert( probeCount < MAX_MAP_REFLECTION_PROBES );
    if (r_reflectionProbeGenerate->current.enabled)
    {
        Com_LoadBsp(mapname);
        version = Com_GetBspVersion();
        if (version != 22)
            Com_Error(
                ERR_DROP,
                "You can only generate reflections for BSP version %i, but the BSP is version %i.  You need to recompile the map.",
                22,
                version);
        probeRawLumpData = (DiskGfxReflectionProbe*)Com_GetBspLump(LUMP_REFLECTION_PROBES, 0x20044u, &lumpProbeCount);
        if (probeCount != lumpProbeCount)
            MyAssertHandler(
                ".\\r_reflection_probe.cpp",
                189,
                0,
                "probeCount == lumpProbeCount\n\t%i, %i",
                probeCount,
                lumpProbeCount);
        if (probeCount)
        {
            lumpSize = 131140 * probeCount;
            probeRawGeneratedData = (DiskGfxReflectionProbe*)Z_Malloc(131140 * probeCount, "R_GenerateReflections", 0);
            for (probeIndex = 0; probeIndex < lumpProbeCount; ++probeIndex)
            {
                qmemcpy(
                    probeRawGeneratedData[probeIndex].colorCorrectionFilename,
                    probeRawLumpData[probeIndex].colorCorrectionFilename,
                    sizeof(probeRawGeneratedData[probeIndex].colorCorrectionFilename));
                v4 = &probeRawGeneratedData[probeIndex];
                v5 = &probes[probeIndex];
                v4->origin[0] = v5->origin[0];
                v4->origin[1] = v5->origin[1];
                v4->origin[2] = v5->origin[2];
                if (r_reflectionProbeRegenerateAll->current.enabled)
                {
                    generateProbe[probeIndex] = 1;
                }
                else
                {
                    v3 = R_CopyReflectionsFromLumpData(&probeRawGeneratedData[probeIndex], probeRawLumpData, probeCount);
                    generateProbe[probeIndex] = v3 == 0;
                }
            }
            R_GenerateReflectionRawDataAll(probeRawGeneratedData, lumpProbeCount, generateProbe);
            Com_SaveLump(LUMP_REFLECTION_PROBES, probeRawGeneratedData, lumpSize, COM_SAVE_LUMP_AND_CLOSE);
            R_GenerateReflectionImages(probes, probeRawGeneratedData, probeCount, 0);
            Z_Free(probeRawGeneratedData, 0);
        }
        else
        {
            Com_SaveLump(LUMP_REFLECTION_PROBES, 0, 0, COM_SAVE_LUMP_AND_CLOSE);
        }
        if (R_ReflectionProbeGenerateExitWhenDone())
        {
            Sys_NormalExit();
            exit(0);
        }
        Dvar_SetBool(r_reflectionProbeGenerate, 0);
    }
}