#include "r_outdoor.h"
#include <universal/assertive.h>
#include "r_image.h"

const int outdoorMapSize[3] = { 0x200, 0x200, 0x100 };

struct OutdoorGlob // sizeof=0x40
{                                       // ...
    float bbox[2][3];                   // ...
    float scale[3];                     // ...
    float invScale[3];                  // ...
    float add[3];                       // ...
    unsigned __int8 *pic;               // ...
};

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