#include "r_outdoor.h"
#include <universal/assertive.h>
#include "r_image.h"

OutdoorGlob outdoorGlob;

void __cdecl Outdoor_ApplyBoundingBox(const float *outdoorMin, const float *outdoorMax)
{
    int dimIter; // [esp+4h] [ebp-4h]

    outdoorGlob.bbox[0][0] = *outdoorMin;
    outdoorGlob.bbox[0][1] = outdoorMin[1];
    outdoorGlob.bbox[0][2] = outdoorMin[2];
    outdoorGlob.bbox[1][0] = *outdoorMax;
    outdoorGlob.bbox[1][1] = outdoorMax[1];
    outdoorGlob.bbox[1][2] = outdoorMax[2];
    for (dimIter = 0; dimIter != 3; ++dimIter)
    {
        if (outdoorGlob.bbox[0][dimIter] == 131072.0)
        {
            if (outdoorGlob.scale[dimIter - 3] != -131072.0)
                MyAssertHandler(".\\r_outdoor.cpp", 90, 1, "%s", "outdoorGlob.bbox[SUPREMUM][dimIter] == MIN_WORLD_COORD");
            outdoorGlob.bbox[0][dimIter] = 0.0;
            outdoorGlob.scale[dimIter - 3] = 0.0;
        }
        if (outdoorGlob.bbox[0][dimIter] > outdoorGlob.scale[dimIter - 3])
            MyAssertHandler(
                ".\\r_outdoor.cpp",
                94,
                1,
                "%s",
                "outdoorGlob.bbox[SUPREMUM][dimIter] >= outdoorGlob.bbox[INFIMUM][dimIter]");
        if (outdoorGlob.scale[dimIter - 3] - outdoorGlob.bbox[0][dimIter] < 1.0)
        {
            outdoorGlob.bbox[0][dimIter] = outdoorGlob.bbox[0][dimIter] - 0.5;
            outdoorGlob.scale[dimIter - 3] = outdoorGlob.scale[dimIter - 3] + 0.5;
        }
        if (outdoorGlob.bbox[0][dimIter] >= outdoorGlob.scale[dimIter - 3])
            MyAssertHandler(
                ".\\r_outdoor.cpp",
                100,
                1,
                "%s",
                "outdoorGlob.bbox[SUPREMUM][dimIter] > outdoorGlob.bbox[INFIMUM][dimIter]");
    }
}

int Outdoor_UpdateTransforms()
{
    int result; // eax
    int dimension; // [esp+4h] [ebp-4h]

    for (dimension = 0; dimension != 3; ++dimension)
    {
        outdoorGlob.scale[dimension] = (outdoorMapSize[dimension] - 1)
            / (outdoorGlob.scale[dimension - 3] - outdoorGlob.bbox[0][dimension]);
        outdoorGlob.invScale[dimension] = 1.0 / outdoorGlob.scale[dimension];
        outdoorGlob.add[dimension] = -outdoorGlob.bbox[0][dimension] * outdoorGlob.scale[dimension];
        result = dimension + 1;
    }
    return result;
}

void __cdecl R_RegisterOutdoorImage(GfxWorld *world, const float *outdoorMin, const float *outdoorMax)
{
    if (!world)
        MyAssertHandler(".\\r_outdoor.cpp", 192, 0, "%s", "world");
    Outdoor_ApplyBoundingBox(outdoorMin, outdoorMax);
    Outdoor_UpdateTransforms();
    Outdoor_SetRendererOutdoorLookupMatrix(world);
    world->outdoorImage = Image_Register("$outdoor", 1u, 0);
    if (!world->outdoorImage)
        MyAssertHandler(".\\r_outdoor.cpp", 199, 0, "%s", "world->outdoorImage");
}

void __cdecl Outdoor_SetRendererOutdoorLookupMatrix(GfxWorld *world)
{
    float outdoorScale[3]; // [esp+0h] [ebp-1Ch]
    float outdoorTranslate[3]; // [esp+Ch] [ebp-10h]
    int dimIter; // [esp+18h] [ebp-4h]

    for (dimIter = 0; dimIter != 3; ++dimIter)
    {
        outdoorScale[dimIter] = 1.0 / (outdoorGlob.scale[dimIter - 3] - outdoorGlob.bbox[0][dimIter]);
        outdoorTranslate[dimIter] = -outdoorGlob.bbox[0][dimIter] * outdoorScale[dimIter];
    }
    MatrixIdentity44(world->outdoorLookupMatrix);
    world->outdoorLookupMatrix[0][0] = outdoorScale[0];
    world->outdoorLookupMatrix[1][1] = outdoorScale[1];
    world->outdoorLookupMatrix[2][2] = outdoorScale[2];
    world->outdoorLookupMatrix[3][0] = outdoorTranslate[0];
    world->outdoorLookupMatrix[3][1] = outdoorTranslate[1];
    world->outdoorLookupMatrix[3][2] = outdoorTranslate[2];
}

void Outdoor_TempHunkFreePic()
{
    if (!outdoorGlob.pic)
        MyAssertHandler(".\\r_outdoor.cpp", 51, 0, "%s", "outdoorGlob.pic");
    Hunk_FreeTempMemory((char *)outdoorGlob.pic);
}

unsigned __int8 *Outdoor_ComputeTexels()
{
    unsigned __int8 *result; // eax
    int v1; // [esp+24h] [ebp-1Ch]
    float zWorld; // [esp+28h] [ebp-18h]
    float yWorld; // [esp+2Ch] [ebp-14h]
    unsigned __int8 *texelPtr; // [esp+30h] [ebp-10h]
    int x; // [esp+34h] [ebp-Ch]
    int y; // [esp+38h] [ebp-8h]
    float xWorld; // [esp+3Ch] [ebp-4h]

    if (!outdoorGlob.pic)
        MyAssertHandler(".\\r_outdoor.cpp", 173, 0, "%s", "outdoorGlob.pic");
    result = outdoorGlob.pic;
    texelPtr = outdoorGlob.pic;
    for (y = 0; y != outdoorMapSize[1]; ++y)
    {
        yWorld = outdoorGlob.invScale[1] * ((double)y + 0.5 - outdoorGlob.add[1]);
        for (x = 0; ; ++x)
        {
            result = (unsigned __int8 *)x;
            if (x == outdoorMapSize[0])
                break;
            xWorld = outdoorGlob.invScale[0] * ((double)x + 0.5 - outdoorGlob.add[0]);
            zWorld = Outdoor_TraceHeightInWorld(xWorld, yWorld);
            v1 = Outdoor_TransformToTextureClamped(2, zWorld);
            *texelPtr = v1;
            if (v1 != *texelPtr)
                MyAssertHandler(".\\r_outdoor.cpp", 163, 0, "%s", "zTexture == *outByte");
            ++texelPtr;
        }
    }
    return result;
}

double __cdecl Outdoor_TraceHeightInWorld(float worldX, float worldY)
{
    float traceEndHeight; // [esp+4h] [ebp-4Ch]
    float traceStartHeight; // [esp+8h] [ebp-48h]
    trace_t results; // [esp+Ch] [ebp-44h] BYREF
    float traceStart[3]; // [esp+38h] [ebp-18h] BYREF
    float traceEnd[3]; // [esp+44h] [ebp-Ch] BYREF

    traceStartHeight = outdoorGlob.bbox[1][2] + 1.0;
    traceEndHeight = outdoorGlob.bbox[0][2] - 1.0;
    traceStart[0] = worldX;
    traceStart[1] = worldY;
    traceStart[2] = traceStartHeight;
    traceEnd[0] = worldX;
    traceEnd[1] = worldY;
    traceEnd[2] = traceEndHeight;
    memset((unsigned __int8 *)&results, 0, sizeof(results));
    results.fraction = 1.0;
    CM_BoxTrace(&results, traceStart, traceEnd, vec3_origin, vec3_origin, 0, 8193);
    return (float)((traceEndHeight - traceStartHeight) * results.fraction + traceStartHeight);
}

int __cdecl Outdoor_TransformToTextureClamped(int dimension, float inWorld)
{
    int v4; // [esp+4h] [ebp-18h]
    int unclamped; // [esp+14h] [ebp-8h]
    float transformed; // [esp+18h] [ebp-4h]

    transformed = inWorld * outdoorGlob.scale[dimension] + outdoorGlob.add[dimension];
    unclamped = (int)(transformed - 0.4999999990686774);
    v4 = outdoorMapSize[dimension] - 1;
    if (v4 <= 0)
        MyAssertHandler("c:\\trees\\cod3\\src\\universal\\com_math.h", 522, 0, "%s", "min < max");
    if (unclamped < 0)
        return 0;
    if (unclamped <= v4)
        return (int)(transformed - 0.4999999990686774);
    return v4;
}