#include "r_model_lighting.h"
#include "r_dvars.h"
#include "r_staticmodelcache.h"
#include "r_dpvs.h"
#include "r_primarylights.h"
#include "rb_light.h"
#include "rb_logfile.h"

struct $D83B18AC5ED51685DB5F92059A920C50 // sizeof=0x4
{                                       // ...
    unsigned int baseIndex;             // ...
};

struct $616C0C4E0125F5DAA7F70C1AB2F0F42D // sizeof=0x6C
{                                       // ...
    float invImageHeight;               // ...
    $D83B18AC5ED51685DB5F92059A920C50 xmodel; // ...
    unsigned int totalEntryLimit;       // ...
    unsigned int entryBitsY;            // ...
    unsigned int imageHeight;           // ...
    const GfxEntity *entities;
    unsigned int modFrameCount;         // ...
    GfxImage *lightImages[2];           // ...
    GfxImage *image;                    // ...
    unsigned int xmodelEntryLimit;      // ...
    GfxLightingInfo *lightingInfo;      // ...
    float (*lightingOrigins)[3];        // ...
    int allocModelFail;                 // ...
    unsigned int *pixelFreeBits[4];     // ...
    unsigned int *prevPrevPixelFreeBits; // ...
    unsigned int *prevPixelFreeBits;    // ...
    unsigned int *currPixelFreeBits;    // ...
    unsigned int pixelFreeBitsSize;     // ...
    unsigned int pixelFreeBitsWordCount; // ...
    unsigned int pixelFreeRover;        // ...
    _D3DLOCKED_BOX lockedBox;           // ...
};

struct GfxSmodelLightGlob_s // sizeof=0x6080
{                                       // ...
    unsigned __int16 smodelIndex[4096]; // ...
    unsigned int usedFrameCount[4096];  // ...
    unsigned int entryLimit;            // ...
    unsigned int assignedCount;         // ...
    unsigned int freeableCount;         // ...
    unsigned int frameCount;            // ...
    int anyNewLighting;                 // ...
    unsigned int pad[27];
};
struct GfxSmodelLightGlob // sizeof=0xA080
{                                       // ...
    unsigned __int16 freeableHandles[4096]; // ...
    unsigned int lightingBits[2048];    // ...
    GfxSmodelLightGlob_s local; // ...
};

struct GfxFindLightForBox // sizeof=0x1C
{                                       // ...
    const GfxViewInfo *viewInfo;        // ...
    float midPoint[3];                  // ...
    float halfSize[3];                  // ...
};

struct GfxFindLightForSphere // sizeof=0x14
{                                       // ...
    const GfxViewInfo *viewInfo;        // ...
    float origin[3];                    // ...
    float radius;                       // ...
};

$616C0C4E0125F5DAA7F70C1AB2F0F42D modelLightGlob;

GfxSmodelLightGlob smodelLightGlob;

int s_modelLightingSampleDelta[64];

void __cdecl R_SetModelLightingCoords(unsigned __int16 handle, float *out)
{
    unsigned int entryIndex; // [esp+10h] [ebp-18h]
    float yCoord; // [esp+14h] [ebp-14h]
    float xCoord; // [esp+24h] [ebp-4h]

    entryIndex = R_ModelLightingIndexFromHandle(handle);
    xCoord = ((double)(4 * (entryIndex & 0x3F)) + 2.0) * 0.00390625;
    yCoord = ((double)((entryIndex >> 4) & 0xFFFFFFFC) + 2.0) * modelLightGlob.invImageHeight;
    out[0] = xCoord;
    out[1] = yCoord;
    out[2] = 0.5;
    out[3] = 1.0;
}

void __cdecl R_GetPackedStaticModelLightingCoords(unsigned int smodelIndex, PackedLightingCoords *packedCoords)
{
    unsigned int v2; // [esp+0h] [ebp-18h]
    unsigned int v3; // [esp+4h] [ebp-14h]
    unsigned int entryIndex; // [esp+8h] [ebp-10h]
    unsigned int xPixel; // [esp+10h] [ebp-8h]

    entryIndex = R_ModelLightingIndexFromHandle(rgp.world->dpvs.smodelDrawInsts[smodelIndex].lightingHandle);
    xPixel = 4 * (entryIndex & 0x3F);
    if ((modelLightGlob.imageHeight & (modelLightGlob.imageHeight - 1)) != 0)
        MyAssertHandler(".\\r_model_lighting.cpp", 833, 0, "%s", "IsPowerOf2( modelLightGlob.imageHeight )");
    if (modelLightGlob.imageHeight > 0x100)
        MyAssertHandler(
            ".\\r_model_lighting.cpp",
            834,
            0,
            "modelLightGlob.imageHeight <= GFX_ML_IMAGE_WIDTH\n\t%i, %i",
            modelLightGlob.imageHeight,
            256);
    v3 = xPixel + 2;
    if (xPixel + 2 != (unsigned __int8)(xPixel + 2))
        MyAssertHandler(
            "c:\\trees\\cod3\\src\\qcommon\\../universal/assertive.h",
            281,
            0,
            "i == static_cast< Type >( i )\n\t%i, %i",
            v3,
            (unsigned __int8)v3);
    packedCoords->array[0] = v3;
    v2 = ((entryIndex >> 4) & 0xFFFFFFFC) + 2 * (0x100 / modelLightGlob.imageHeight);
    if (v2 != (unsigned __int8)v2)
        MyAssertHandler(
            "c:\\trees\\cod3\\src\\qcommon\\../universal/assertive.h",
            281,
            0,
            "i == static_cast< Type >( i )\n\t%i, %i",
            v2,
            (unsigned __int8)v2);
    packedCoords->array[1] = v2;
    packedCoords->array[2] = 0x80;
    packedCoords->array[3] = 0;
}

unsigned int __cdecl R_ModelLightingIndexFromHandle(unsigned __int16 handle)
{
    iassert(handle && handle < modelLightGlob.totalEntryLimit);
    return handle - 1;
}

char __cdecl R_AllocStaticModelLighting(GfxStaticModelDrawInst *smodelDrawInst, unsigned int smodelIndex)
{
    unsigned __int16 handle; // [esp+0h] [ebp-10h]
    unsigned int smodelIndexPrev; // [esp+4h] [ebp-Ch]
    unsigned int entryIndex; // [esp+8h] [ebp-8h]

    iassert(rgp.world);
    handle = smodelDrawInst->lightingHandle;
    if (handle)
    {
        entryIndex = R_ModelLightingIndexFromHandle(handle);
        if (!r_cacheSModelLighting->current.enabled)
        {
            bcassert(smodelIndex >> 5, ARRAY_COUNT(smodelLightGlob.lightingBits));

            smodelLightGlob.lightingBits[smodelIndex >> 5] |= 0x80000000 >> (smodelIndex & 0x1F);
            smodelLightGlob.local.anyNewLighting = 1;
        }
    }
    else
    {
        if (smodelLightGlob.local.assignedCount >= smodelLightGlob.local.entryLimit)
        {
            do
            {
                if (!smodelLightGlob.local.freeableCount)
                    return 0;
                handle = smodelLightGlob.freeableHandles[--smodelLightGlob.local.freeableCount];
                entryIndex = R_ModelLightingIndexFromHandle(handle);
            } while (smodelLightGlob.local.frameCount == smodelLightGlob.local.usedFrameCount[entryIndex]);

            smodelIndexPrev = smodelLightGlob.local.smodelIndex[entryIndex];
            rgp.world->dpvs.smodelDrawInsts[smodelIndexPrev].lightingHandle = 0;
            R_UncacheStaticModel(smodelIndexPrev);
        }
        else
        {
            entryIndex = smodelLightGlob.local.assignedCount++;
            handle = entryIndex + 1;
        }
        iassert(handle);
        smodelDrawInst->lightingHandle = handle;
        iassert(smodelIndex == static_cast<unsigned short>(smodelIndex));
        smodelLightGlob.local.smodelIndex[entryIndex] = smodelIndex;
        bcassert(smodelIndex >> 5, ARRAY_COUNT(smodelLightGlob.lightingBits));
        smodelLightGlob.lightingBits[smodelIndex >> 5] |= 0x80000000 >> (smodelIndex & 0x1F);
        smodelLightGlob.local.anyNewLighting = 1;
    }
    smodelLightGlob.local.usedFrameCount[entryIndex] = smodelLightGlob.local.frameCount;
    return 1;
}

unsigned int __cdecl R_AllocModelLighting_PrimaryLight(
    float *lightingOrigin,
    unsigned int dynEntId,
    unsigned __int16 *cachedLightingHandle,
    GfxLightingInfo *lightingInfoOut)
{
    return R_AllocModelLighting(
        lightingOrigin,
        cachedLightingHandle,
        R_DynEntPrimaryLightCallback,
        &dynEntId,
        lightingInfoOut);
}

unsigned int __cdecl R_AllocModelLighting(
    float *lightingOrigin,
    unsigned __int16 *cachedLightingHandle,
    unsigned int(__cdecl *GetPrimaryLightCallback)(const void *),
    const void *userData,
    GfxLightingInfo *lightingInfoOut)
{
    DWORD v7; // eax
    int v8; // [esp+4h] [ebp-2Ch]
    float *v9; // [esp+8h] [ebp-28h]
    float *v10; // [esp+10h] [ebp-20h]
    unsigned int pixelFreeRover; // [esp+14h] [ebp-1Ch]
    unsigned int entryIndex; // [esp+18h] [ebp-18h]
    unsigned int usedCount; // [esp+1Ch] [ebp-14h]
    unsigned int usedIndex; // [esp+20h] [ebp-10h]
    unsigned int usedIndexa; // [esp+20h] [ebp-10h]
    unsigned __int16 lightingHandle; // [esp+28h] [ebp-8h]
    unsigned __int16 lightingHandlea; // [esp+28h] [ebp-8h]
    unsigned int nonSunPrimaryLightIndex; // [esp+2Ch] [ebp-4h]

    if (!cachedLightingHandle)
        MyAssertHandler(".\\r_model_lighting.cpp", 322, 0, "%s", "cachedLightingHandle");
    lightingHandle = *cachedLightingHandle;
    if (*cachedLightingHandle
        && r_cacheModelLighting->current.enabled
        && ((usedIndex = R_ModelLightingIndexFromHandle(lightingHandle) - modelLightGlob.xmodel.baseIndex,
            v10 = modelLightGlob.lightingOrigins[usedIndex],
            *v10 != *lightingOrigin)
            || v10[1] != lightingOrigin[1]
            || v10[2] != lightingOrigin[2]
            ? (v8 = 0)
            : (v8 = 1),
            v8))
    {
        modelLightGlob.currPixelFreeBits[usedIndex >> 5] &= ~(0x80000000 >> (usedIndex & 0x1F));
        *lightingInfoOut = modelLightGlob.lightingInfo[usedIndex];
        return lightingHandle;
    }
    else if (modelLightGlob.allocModelFail)
    {
        return 0;
    }
    else
    {
        pixelFreeRover = modelLightGlob.pixelFreeRover;
        while (1)
        {
            if (!_BitScanReverse(&v7, modelLightGlob.prevPrevPixelFreeBits[pixelFreeRover] & modelLightGlob.prevPixelFreeBits[pixelFreeRover & modelLightGlob.currPixelFreeBits[pixelFreeRover]]))
                v7 = 63;// `CountLeadingZeros'::`2': : notFound;
            usedCount = v7 ^ 0x1F;
            if ((v7 ^ 0x1Fu) < 0x20)
                break;
            pixelFreeRover = (pixelFreeRover + 1) % modelLightGlob.pixelFreeBitsWordCount;
            if (pixelFreeRover == modelLightGlob.pixelFreeRover)
            {
                modelLightGlob.allocModelFail = 1;
                R_WarnOncePerFrame(R_WARN_MODEL_LIGHT_CACHE);
                return 0;
            }
        }
        modelLightGlob.pixelFreeRover = pixelFreeRover;
        modelLightGlob.currPixelFreeBits[pixelFreeRover] &= ~(0x80000000 >> usedCount);
        usedIndexa = usedCount + 32 * pixelFreeRover;
        if (usedIndexa >= modelLightGlob.xmodelEntryLimit)
            MyAssertHandler(
                ".\\r_model_lighting.cpp",
                362,
                0,
                "usedIndex doesn't index modelLightGlob.xmodelEntryLimit\n\t%i not in [0, %i)",
                usedIndexa,
                modelLightGlob.xmodelEntryLimit);
        entryIndex = usedIndexa + modelLightGlob.xmodel.baseIndex;
        lightingHandlea = usedIndexa + LOWORD(modelLightGlob.xmodel.baseIndex) + 1;
        v9 = modelLightGlob.lightingOrigins[usedIndexa];
        *v9 = *lightingOrigin;
        v9[1] = lightingOrigin[1];
        v9[2] = lightingOrigin[2];
        *cachedLightingHandle = lightingHandlea;
        lightingInfoOut->reflectionProbeIndex = R_CalcReflectionProbeIndex(lightingOrigin);
        nonSunPrimaryLightIndex = GetPrimaryLightCallback(userData);
        lightingInfoOut->primaryLightIndex = R_CalcModelLighting(
            entryIndex,
            lightingOrigin,
            nonSunPrimaryLightIndex,
            GFX_MODELLIGHT_SHOW_MISSING);
        modelLightGlob.lightingInfo[usedIndexa] = *lightingInfoOut;
        return lightingHandlea;
    }
}

unsigned int __cdecl R_DynEntPrimaryLightCallback(const void *userData)
{
    DWORD *data = (DWORD *)userData;
    return rgp.world->nonSunPrimaryLightForModelDynEnt[*data];
}

unsigned int __cdecl R_AllocModelLighting_Box(
    const GfxViewInfo *viewInfo,
    float *lightingOrigin,
    const float *boxMins,
    const float *boxMaxs,
    unsigned __int16 *cachedLightingHandle,
    GfxLightingInfo *lightingInfoOut)
{
    GfxFindLightForBox boxWork; // [esp+0h] [ebp-1Ch] BYREF

    boxWork.viewInfo = viewInfo;
    Vec3Avg(boxMins, boxMaxs, boxWork.midPoint);
    Vec3Sub(boxWork.midPoint, boxMins, boxWork.halfSize);

    return R_AllocModelLighting(
        lightingOrigin,
        cachedLightingHandle,
        R_GetPrimaryLightForBoxCallback,
        &boxWork,
        lightingInfoOut);
}

unsigned int __cdecl R_GetPrimaryLightForBoxCallback(const void *userData)
{
    GfxFindLightForBox *boxWork = (GfxFindLightForBox *)userData;

    return R_GetNonSunPrimaryLightForBox(boxWork->viewInfo, boxWork->midPoint, boxWork->halfSize);
}

unsigned int __cdecl R_AllocModelLighting_Sphere(
    const GfxViewInfo *viewInfo,
    float *lightingOrigin,
    const float *origin,
    float radius,
    unsigned __int16 *cachedLightingHandle,
    GfxLightingInfo *lightingInfoOut)
{
    GfxFindLightForSphere sphereWork; // [esp+0h] [ebp-14h] BYREF

    sphereWork.viewInfo = viewInfo;
    sphereWork.origin[0] = origin[0];
    sphereWork.origin[1] = origin[1];
    sphereWork.origin[2] = origin[2];
    sphereWork.radius = radius;

    return R_AllocModelLighting(
        lightingOrigin,
        cachedLightingHandle,
        R_GetPrimaryLightForSphereCallback,
        &sphereWork,
        lightingInfoOut);
}

unsigned int __cdecl R_GetPrimaryLightForSphereCallback(const void *userData)
{
    GfxFindLightForSphere *sphereWork = (GfxFindLightForSphere *)userData;
    
    return R_GetNonSunPrimaryLightForSphere(sphereWork->viewInfo, sphereWork->origin, sphereWork->radius);
}

void __cdecl R_ToggleModelLightingFrame()
{
    unsigned int entryIndex; // [esp+0h] [ebp-4h]

    ++smodelLightGlob.local.frameCount;
    modelLightGlob.modFrameCount = (modelLightGlob.modFrameCount + 1) % 4;
    modelLightGlob.allocModelFail = 0;
    modelLightGlob.prevPrevPixelFreeBits = modelLightGlob.pixelFreeBits[(modelLightGlob.modFrameCount + 2) % 4];
    modelLightGlob.prevPixelFreeBits = modelLightGlob.pixelFreeBits[(modelLightGlob.modFrameCount + 3) % 4];
    modelLightGlob.currPixelFreeBits = modelLightGlob.pixelFreeBits[modelLightGlob.modFrameCount % 4];
    Com_Memset(modelLightGlob.currPixelFreeBits, 255, modelLightGlob.pixelFreeBitsSize);
    smodelLightGlob.local.freeableCount = 0;
    for (entryIndex = 0; entryIndex < smodelLightGlob.local.assignedCount; ++entryIndex)
    {
        if (smodelLightGlob.local.frameCount - smodelLightGlob.local.usedFrameCount[entryIndex] >= 4)
            smodelLightGlob.freeableHandles[smodelLightGlob.local.freeableCount++] = entryIndex + 1;
    }
}

unsigned int __cdecl R_CalcModelLighting(
    unsigned int entryIndex,
    const float *lightingOrigin,
    unsigned int nonSunPrimaryLightIndex,
    GfxModelLightExtrapolation extrapolateBehavior)
{
    CL_ResetStats_f();
    if (entryIndex != (unsigned __int16)entryIndex)
        MyAssertHandler(
            "c:\\trees\\cod3\\src\\qcommon\\../universal/assertive.h",
            281,
            0,
            "i == static_cast< Type >( i )\n\t%i, %i",
            entryIndex,
            (unsigned __int16)entryIndex);
    if (nonSunPrimaryLightIndex >= rgp.world->primaryLightCount)
        MyAssertHandler(
            ".\\r_model_lighting.cpp",
            680,
            0,
            "nonSunPrimaryLightIndex doesn't index rgp.world->primaryLightCount\n\t%i not in [0, %i)",
            nonSunPrimaryLightIndex,
            rgp.world->primaryLightCount);
    return R_GetLightingAtPoint(
        &rgp.world->lightGrid,
        lightingOrigin,
        nonSunPrimaryLightIndex,
        entryIndex,
        extrapolateBehavior);
}

void __cdecl R_BeginAllStaticModelLighting()
{
    unsigned int size; // [esp+0h] [ebp-4h]

    if (smodelLightGlob.local.anyNewLighting)
        MyAssertHandler(".\\r_model_lighting.cpp", 745, 0, "%s", "!smodelLightGlob.local.anyNewLighting");
    size = 4 * ((rgp.world->dpvs.smodelCount + 31) >> 5);
    if (size >= 0x2000)
        MyAssertHandler(".\\r_model_lighting.cpp", 749, 0, "%s", "size < sizeof( smodelLightGlob.lightingBits )");
    Com_Memset(smodelLightGlob.lightingBits, 0, size);
}

void __cdecl R_SetAllStaticModelLighting()
{
    DWORD v1; // eax
    unsigned int wordCount; // [esp+34h] [ebp-18h]
    unsigned int bits; // [esp+38h] [ebp-14h]
    unsigned int indexLow; // [esp+40h] [ebp-Ch]
    unsigned int wordIndex; // [esp+44h] [ebp-8h]

    if (smodelLightGlob.local.anyNewLighting)
    {
        smodelLightGlob.local.anyNewLighting = 0;
        //Profile_Begin(412);
        wordCount = (rgp.world->dpvs.smodelCount + 31) >> 5;
        if (wordCount >= 0x2000)
            MyAssertHandler(".\\r_model_lighting.cpp", 772, 0, "%s", "wordCount < sizeof( smodelLightGlob.lightingBits )");
        for (wordIndex = 0; wordIndex < wordCount; ++wordIndex)
        {
            bits = smodelLightGlob.lightingBits[wordIndex];
            if (bits)
            {
                while (1)
                {
                    if (!_BitScanReverse(&v1, bits))
                        v1 = 63;// `CountLeadingZeros'::`2': : notFound;
                    indexLow = v1 ^ 0x1F;
                    if ((v1 ^ 0x1Fu) >= 0x20)
                        break;
                    if (((0x80000000 >> indexLow) & bits) == 0)
                        MyAssertHandler(".\\r_model_lighting.cpp", 789, 0, "%s", "bits & bit");
                    bits &= ~(0x80000000 >> indexLow);
                    R_SetStaticModelLighting(indexLow + 32 * wordIndex);
                }
            }
        }
        //Profile_EndInternal(0);
    }
}

void __cdecl R_SetStaticModelLighting(unsigned int smodelIndex)
{
    unsigned int entryIndex; // [esp+0h] [ebp-18h]
    const GfxStaticModelDrawInst *smodelDrawInst; // [esp+4h] [ebp-14h]
    const GfxStaticModelInst *smodelInst; // [esp+8h] [ebp-10h]
    float lightingOrigin[3]; // [esp+Ch] [ebp-Ch] BYREF

    if (smodelIndex >= rgp.world->dpvs.smodelCount)
        MyAssertHandler(
            ".\\r_model_lighting.cpp",
            728,
            0,
            "smodelIndex doesn't index rgp.world->dpvs.smodelCount\n\t%i not in [0, %i)",
            smodelIndex,
            rgp.world->dpvs.smodelCount);
    smodelInst = &rgp.world->dpvs.smodelInsts[smodelIndex];
    Vec3Avg(smodelInst->mins, smodelInst->maxs, lightingOrigin);
    smodelDrawInst = &rgp.world->dpvs.smodelDrawInsts[smodelIndex];
    entryIndex = R_ModelLightingIndexFromHandle(smodelDrawInst->lightingHandle);
    if (smodelInst->groundLighting.packed)
        R_SetModelGroundLighting(entryIndex, (const unsigned __int8 *)&smodelInst->groundLighting);
    else
        R_CalcModelLighting(entryIndex, lightingOrigin, smodelDrawInst->primaryLightIndex, GFX_MODELLIGHT_EXTRAPOLATE);
}

void __cdecl R_SetModelGroundLighting(unsigned int entryIndex, const unsigned __int8 *groundLighting)
{
    GfxModelLightingPatch *patch; // [esp+8h] [ebp-4h]

    patch = R_BackEndDataAllocAndClearModelLightingPatch(frontEndDataOut);
    if (entryIndex != (unsigned __int16)entryIndex)
        MyAssertHandler(
            "c:\\trees\\cod3\\src\\qcommon\\../universal/assertive.h",
            281,
            0,
            "i == static_cast< Type >( i )\n\t%i, %i",
            entryIndex,
            (unsigned __int16)entryIndex);
    patch->modelLightingIndex = entryIndex;
    if (patch->colorsCount)
        MyAssertHandler(".\\r_model_lighting.cpp", 715, 1, "%s", "patch->colorsCount == 0");
    *(unsigned int *)patch->groundLighting = *(unsigned int *)groundLighting;
}

void __cdecl R_SetModelLightingCoordsForSource(unsigned __int16 handle, GfxCmdBufSourceState *source)
{
    R_SetModelLightingCoords(handle, source->input.consts[57]);
    R_DirtyCodeConstant(source, 0x39u);
}

void __cdecl R_SetStaticModelLightingCoordsForSource(unsigned int smodelIndex, GfxCmdBufSourceState *source)
{
    R_SetModelLightingCoords(rgp.world->dpvs.smodelDrawInsts[smodelIndex].lightingHandle, source->input.consts[57]);
    R_DirtyCodeConstant(source, 0x39u);
}


unsigned int R_SetModelLightingSampleDeltas()
{
    unsigned int result; // eax
    unsigned int i; // [esp+0h] [ebp-10h]
    unsigned int sampleIndex; // [esp+4h] [ebp-Ch]
    unsigned int dz; // [esp+8h] [ebp-8h]
    unsigned int dy; // [esp+Ch] [ebp-4h]

    sampleIndex = 0;
    for (dz = 0; dz < 4; ++dz)
    {
        for (dy = 0; dy < 4; ++dy)
        {
            for (i = 0; i < 4; ++i)
                s_modelLightingSampleDelta[sampleIndex++] = modelLightGlob.lockedBox.SlicePitch * dz
                + modelLightGlob.lockedBox.RowPitch * dy
                + 4 * i;
        }
        result = dz + 1;
    }
    return result;
}

void __cdecl R_SetModelLightingLookupScale(GfxCmdBufInput *input)
{
    float lookupScale[4]; // [esp+4h] [ebp-10h] BYREF

    lookupScale[0] = 0.005859375;
    lookupScale[1] = modelLightGlob.invImageHeight * 1.5;
    lookupScale[2] = 0.375;
    lookupScale[3] = 0.0;
    R_SetInputCodeConstantFromVec4(input, 0x26u, lookupScale);
}

void __cdecl R_SetupDynamicModelLighting(GfxCmdBufInput *input)
{
    R_SetInputCodeImageTexture(input, 3u, modelLightGlob.image);
    R_SetModelLightingLookupScale(input);
}

void __cdecl R_ShutdownModelLightingGlobals()
{
    unsigned int i; // [esp+0h] [ebp-4h]

    for (i = 0; i < 4; ++i)
        R_FreeGlobalVariable(modelLightGlob.pixelFreeBits[i]);
    R_FreeGlobalVariable(modelLightGlob.lightingOrigins);
    R_FreeGlobalVariable(modelLightGlob.lightingInfo);
}

void __cdecl R_InitModelLightingGlobals()
{
    DWORD v1; // eax
    unsigned int totalBitsNeeded; // [esp+Ch] [ebp-8h]
    unsigned int i; // [esp+10h] [ebp-4h]

    modelLightGlob.xmodelEntryLimit = gfxCfg.maxClientViews << 10;
    if (!_BitScanReverse(&v1, gfxCfg.maxClientViews << 10))
        v1 = 63;// `CountLeadingZeros'::`2': : notFound;
    totalBitsNeeded = 32 - (v1 ^ 31);
    for (smodelLightGlob.local.entryLimit = (1 << totalBitsNeeded) - modelLightGlob.xmodelEntryLimit;
        smodelLightGlob.local.entryLimit < 0x800;
        smodelLightGlob.local.entryLimit += 1 << totalBitsNeeded++)
    {
        ;
    }

    iassert(smodelLightGlob.local.entryLimit <= 4096);

    modelLightGlob.totalEntryLimit = 1 << totalBitsNeeded;
    modelLightGlob.entryBitsY = totalBitsNeeded - 6;
    modelLightGlob.imageHeight = 1 << (totalBitsNeeded - 6 + 2);
    modelLightGlob.invImageHeight = 1.0 / (double)(unsigned int)(1 << (totalBitsNeeded - 6 + 2));
    modelLightGlob.xmodel.baseIndex = smodelLightGlob.local.entryLimit;

    iassert(!(modelLightGlob.xmodelEntryLimit & 31));

    modelLightGlob.pixelFreeBitsSize = modelLightGlob.xmodelEntryLimit >> 3;
    modelLightGlob.pixelFreeBitsWordCount = modelLightGlob.xmodelEntryLimit >> 5;
    modelLightGlob.lightingOrigins = (float (*)[3])R_AllocModelLightingGlobal(12 * modelLightGlob.xmodelEntryLimit);

    for (i = 0; i < 4; ++i)
        modelLightGlob.pixelFreeBits[i] = (unsigned int *)R_AllocModelLightingGlobal(modelLightGlob.pixelFreeBitsSize);

    modelLightGlob.lightingInfo = (GfxLightingInfo *)R_AllocModelLightingGlobal(2 * modelLightGlob.xmodelEntryLimit);
    modelLightGlob.image = modelLightGlob.lightImages[0];
}

char *__cdecl R_AllocModelLightingGlobal(unsigned int bytes)
{
    return Z_VirtualAlloc(bytes, "R_AllocModelLightingGlobal", 18);
}

void __cdecl R_ResetModelLighting()
{
    unsigned int v0; // eax
    float *v1; // [esp+0h] [ebp-14h]
    unsigned int entryIndex; // [esp+4h] [ebp-10h]
    unsigned int usedIndex; // [esp+8h] [ebp-Ch]
    unsigned int i; // [esp+Ch] [ebp-8h]
    unsigned int smodelIndex; // [esp+10h] [ebp-4h]

    for (i = 0; i < 4; ++i)
        Com_Memset(modelLightGlob.pixelFreeBits[i], 255, modelLightGlob.pixelFreeBitsSize);

    for (usedIndex = 0; usedIndex < modelLightGlob.xmodelEntryLimit; ++usedIndex)
    {
        v1 = modelLightGlob.lightingOrigins[usedIndex];
        v1[0] = FLT_MAX;
        v1[1] = FLT_MAX;
        v1[2] = FLT_MAX;
    }

    iassert(smodelLightGlob.local.assignedCount == 0 || rgp.world != NULL);

    for (entryIndex = 0; entryIndex < smodelLightGlob.local.assignedCount; ++entryIndex)
    {
        smodelIndex = smodelLightGlob.local.smodelIndex[entryIndex];
        iassert(smodelIndex < rgp.world->dpvs.smodelCount);
        iassert(entryIndex == R_ModelLightingIndexFromHandle(rgp.world->dpvs.smodelDrawInsts[smodelIndex].lightingHandle));
        rgp.world->dpvs.smodelDrawInsts[smodelIndex].lightingHandle = 0;
    }

    smodelLightGlob.local.assignedCount = 0;
}

void __cdecl R_InitModelLightingImage()
{
    bool useAltUpdate; // [esp+1h] [ebp-1h]

    iassert(modelLightGlob.totalEntryLimit);

    useAltUpdate = Dvar_GetBool("r_altModelLightingUpdate");
    modelLightGlob.lightImages[0] = Image_AllocProg(12, 4u, 1u);
    if (useAltUpdate)
    {
        Image_SetupAndLoad(modelLightGlob.lightImages[0], 256, modelLightGlob.imageHeight, 4, 0x1000A, D3DFMT_A8R8G8B8);
        modelLightGlob.lightImages[1] = Image_AllocProg(13, 4u, 1u);
        Image_SetupAndLoad(modelLightGlob.lightImages[1], 256, modelLightGlob.imageHeight, 4, 0x4000A, D3DFMT_A8R8G8B8);
    }
    else
    {
        Image_SetupAndLoad(modelLightGlob.lightImages[0], 256, modelLightGlob.imageHeight, 4, 0xA, D3DFMT_A8R8G8B8);
        modelLightGlob.lightImages[1] = 0;
    }
    modelLightGlob.image = modelLightGlob.lightImages[0];
}

void __cdecl R_ShutdownModelLightingImage()
{
    int imgIndex; // [esp+0h] [ebp-4h]

    iassert(modelLightGlob.totalEntryLimit);

    for (imgIndex = 0; imgIndex < 2; ++imgIndex)
    {
        if (modelLightGlob.lightImages[imgIndex])
        {
            Image_Release(modelLightGlob.lightImages[imgIndex]);
            modelLightGlob.lightImages[imgIndex] = 0;
        }
    }
    modelLightGlob.image = 0;
}

void __cdecl R_InitStaticModelLighting()
{
    smodelLightGlob.local.assignedCount = 0;
    smodelLightGlob.local.freeableCount = 0;
}

void __cdecl R_ApplyLightGridColorsPatch(const GfxModelLightingPatch *patch, unsigned __int8 *pixels)
{
    GfxLightGridColors packed; // [esp+170h] [ebp-A8h] BYREF

    if (patch->colorsCount == 1)
    {
        R_SetLightGridColors(&rgp.world->lightGrid.colors[patch->colorsIndex[0]], patch->primaryLightWeight, pixels);
    }
    else
    {
        R_FixedPointBlendLightGridColors(
            &rgp.world->lightGrid,
            patch->colorsIndex,
            (unsigned short*)patch->colorsWeight,
            patch->colorsCount,
            &packed);
        R_SetLightGridColors(&packed, patch->primaryLightWeight, pixels);
    }
}

void __cdecl RB_PatchModelLighting(const GfxModelLightingPatch *patchList, unsigned int patchCount)
{
    const char *v2; // eax
    const char *v3; // eax
    const char *v4; // eax
    unsigned int modelLightingIndex; // [esp+4h] [ebp-4Ch]
    int v6; // [esp+8h] [ebp-48h]
    int v7; // [esp+Ch] [ebp-44h]
    int hr; // [esp+10h] [ebp-40h]
    unsigned __int8 *pixels; // [esp+18h] [ebp-38h]
    unsigned int sampleIndex; // [esp+1Ch] [ebp-34h]
    unsigned int patchIter; // [esp+20h] [ebp-30h]
    const GfxModelLightingPatch *patch; // [esp+24h] [ebp-2Ch]
    unsigned int y0; // [esp+28h] [ebp-28h]
    GfxImage *lightImage; // [esp+2Ch] [ebp-24h]
    _D3DBOX dirtyBox; // [esp+30h] [ebp-20h] BYREF
    unsigned int lockValue; // [esp+48h] [ebp-8h]
    bool useAltUpdate; // [esp+4Fh] [ebp-1h]

    if (patchCount)
    {
        if (modelLightGlob.lockedBox.pBits)
            MyAssertHandler(".\\r_model_lighting.cpp", 893, 0, "%s", "modelLightGlob.lockedBox.pBits == NULL");
        useAltUpdate = Dvar_GetBool("r_altModelLightingUpdate");
        if (useAltUpdate)
            lightImage = modelLightGlob.lightImages[1];
        else
            lightImage = modelLightGlob.lightImages[0];
        if (!lightImage)
            MyAssertHandler(".\\r_model_lighting.cpp", 901, 0, "%s", "lightImage");
        if (useAltUpdate)
        {
            memset(&dirtyBox, 0, 20);
            dirtyBox.Back = 4;
            lockValue = 0x8000;
        }
        else
        {
            lockValue = 0;
        }
        do
        {
            if (r_logFile && r_logFile->current.integer)
                RB_LogPrint("lightImage->texture.volmap->LockBox( 0, &modelLightGlob.lockedBox, 0, lockValue )\n");
            hr = lightImage->texture.volmap->LockBox(0, &modelLightGlob.lockedBox, 0, lockValue);
            //hr = (lightImage->texture.basemap->__vftable[1].Release)(
            //    lightImage->texture.basemap,
            //    0,
            //    &modelLightGlob.lockedBox,
            //    0,
            //    lockValue);
            if (hr < 0)
            {
                do
                {
                    ++g_disableRendering;
                    v2 = R_ErrorDescription(hr);
                    Com_Error(
                        ERR_FATAL,
                        ".\\r_model_lighting.cpp (%i) lightImage->texture.volmap->LockBox( 0, &modelLightGlob.lockedBox, 0, lockValue ) failed: %s\n",
                        916,
                        v2);
                } while (alwaysfails);
            }
        } while (alwaysfails);
        R_SetModelLightingSampleDeltas();
        R_SetLightGridSampleDeltas(modelLightGlob.lockedBox.RowPitch, modelLightGlob.lockedBox.SlicePitch);
        for (patchIter = 0; patchIter < patchCount; ++patchIter)
        {
            patch = &patchList[patchIter];
            modelLightingIndex = patch->modelLightingIndex;
            y0 = (modelLightingIndex >> 4) & 0xFFFFFFFC;
            if (useAltUpdate)
            {
                dirtyBox.Left = 4 * (modelLightingIndex & 0x3F);
                dirtyBox.Right = dirtyBox.Left + 4;
                dirtyBox.Top = (modelLightingIndex >> 4) & 0xFFFFFFFC;
                dirtyBox.Bottom = y0 + 4;
                //(lightImage->texture.basemap->__vftable[1].SetPrivateData)(lightImage->texture.basemap, &dirtyBox);
                
            }
            pixels = (unsigned char*)modelLightGlob.lockedBox.pBits
                + 16 * (modelLightingIndex & 0x3F)
                + modelLightGlob.lockedBox.RowPitch * y0;
            if (patch->colorsCount)
            {
                R_ApplyLightGridColorsPatch(patch, pixels);
            }
            else
            {
                for (sampleIndex = 0; sampleIndex < 0x40; ++sampleIndex)
                {
                    *(_DWORD *)&pixels[s_modelLightingSampleDelta[sampleIndex]] = *(_DWORD *)patch->groundLighting;
                    //*&pixels[s_modelLightingSampleDelta[sampleIndex]] = *patch->groundLighting;
                }
            }
        }
        if (!modelLightGlob.lockedBox.pBits)
            MyAssertHandler(".\\r_model_lighting.cpp", 947, 0, "%s", "modelLightGlob.lockedBox.pBits");
        do
        {
            if (r_logFile && r_logFile->current.integer)
                RB_LogPrint("lightImage->texture.volmap->UnlockBox( 0 )\n");
            //v7 = lightImage->texture.basemap->__vftable[1].GetDevice(lightImage->texture.basemap, 0);
            v7 = lightImage->texture.volmap->UnlockBox(0);
            if (v7 < 0)
            {
                do
                {
                    ++g_disableRendering;
                    v3 = R_ErrorDescription(v7);
                    Com_Error(
                        ERR_FATAL,
                        ".\\r_model_lighting.cpp (%i) lightImage->texture.volmap->UnlockBox( 0 ) failed: %s\n",
                        949,
                        v3);
                } while (alwaysfails);
            }
        } while (alwaysfails);
        modelLightGlob.lockedBox.pBits = 0;
        if (useAltUpdate)
        {
            do
            {
                if (r_logFile && r_logFile->current.integer)
                    RB_LogPrint("dx.device->UpdateTexture( lightImage->texture.volmap, modelLightGlob.lightImages[0]->texture.volmap )\n");
                v6 = dx.device->UpdateTexture(
                    lightImage->texture.volmap,
                    modelLightGlob.lightImages[0]->texture.volmap);
                if (v6 < 0)
                {
                    do
                    {
                        ++g_disableRendering;
                        v4 = R_ErrorDescription(v6);
                        Com_Error(
                            ERR_FATAL,
                            ".\\r_model_lighting.cpp (%i) dx.device->UpdateTexture( lightImage->texture.volmap, modelLightGlob.lightIma"
                            "ges[0]->texture.volmap ) failed: %s\n",
                            953,
                            v4);
                    } while (alwaysfails);
                }
            } while (alwaysfails);
        }
    }
}