#include "r_dpvs.h"
#include "r_dvars.h"
#include <DynEntity/DynEntity_client.h>


void __cdecl R_AddCellDynModelSurfacesInFrustumCmd(const DpvsPlane **data)
{
    unsigned int planeCount; // [esp+0h] [ebp-28h]
    const DpvsPlane *planes; // [esp+4h] [ebp-24h]
    unsigned int dynEntClientWordCount; // [esp+8h] [ebp-20h]
    unsigned __int8 *dynEntVisData; // [esp+10h] [ebp-18h]
    unsigned int *dynEntCellBits; // [esp+14h] [ebp-14h]
    DynEntityPose *dynModelList; // [esp+18h] [ebp-10h]
    GfxWorldDpvsDynamic *worldDpvsDyn; // [esp+24h] [ebp-4h]

    if (r_drawDynEnts->current.enabled)
    {
        worldDpvsDyn = &rgp.world->dpvsDyn;
        if ((unsigned int)data[1] >= rgp.world->dpvsPlanes.cellCount)
            MyAssertHandler(
                ".\\r_dpvs_dynmodel.cpp",
                118,
                0,
                "dpvsCell->cellIndex doesn't index rgp.world->dpvsPlanes.cellCount\n\t%i not in [0, %i)",
                data[1],
                rgp.world->dpvsPlanes.cellCount);
        dynEntClientWordCount = worldDpvsDyn->dynEntClientWordCount[0];
        planeCount = *((unsigned __int8 *)data + 8);
        dynEntCellBits = &worldDpvsDyn->dynEntCellBits[0][worldDpvsDyn->dynEntClientWordCount[0] * (unsigned int)data[1]];
        dynEntVisData = worldDpvsDyn->dynEntVisData[0][*((unsigned __int16 *)data + 5)];
        planes = *data;
        dynModelList = DynEnt_GetClientModelPoseList();
        R_CullDynModelInCell(dynEntCellBits, dynEntClientWordCount, dynModelList, planes, planeCount, dynEntVisData);
    }
}

void __cdecl R_CullDynModelInCell(
    const unsigned int *dynEntCellBits,
    unsigned int dynEntClientWordCount,
    DynEntityPose *dynModelList,
    const DpvsPlane *planes,
    int planeCount,
    unsigned __int8 *dynEntVisData)
{
    int v7; // eax
    int v8; // [esp+4h] [ebp-28h]
    float radius; // [esp+8h] [ebp-24h]
    const DpvsPlane *a; // [esp+Ch] [ebp-20h]
    int v11; // [esp+10h] [ebp-1Ch]
    unsigned int dynEntIndex; // [esp+1Ch] [ebp-10h]
    unsigned int bits; // [esp+20h] [ebp-Ch]
    unsigned int indexLow; // [esp+24h] [ebp-8h]
    unsigned int wordIndex; // [esp+28h] [ebp-4h]

    for (wordIndex = 0; wordIndex < dynEntClientWordCount; ++wordIndex)
    {
        bits = dynEntCellBits[wordIndex];
        while (1)
        {
            if (!_BitScanReverse((unsigned int *)&v7, bits))
                v7 = `CountLeadingZeros'::`2': : notFound;
            indexLow = v7 ^ 0x1F;
            if ((v7 ^ 0x1Fu) >= 0x20)
                break;
            dynEntIndex = indexLow + 32 * wordIndex;
            if (((0x80000000 >> indexLow) & bits) == 0)
                MyAssertHandler(".\\r_dpvs_dynmodel.cpp", 54, 0, "%s", "bits & bit");
            bits &= ~(0x80000000 >> indexLow);
            if (!dynEntVisData[dynEntIndex])
            {
                radius = dynModelList[dynEntIndex].radius;
                v11 = 0;
                a = planes;
                while (v11 < planeCount)
                {
                    if (Vec3Dot(a->coeffs, dynModelList[dynEntIndex].pose.origin) + a->coeffs[3] + radius <= 0.0)
                    {
                        v8 = 1;
                        goto LABEL_16;
                    }
                    ++v11;
                    ++a;
                }
                v8 = 0;
            LABEL_16:
                if (!v8)
                    dynEntVisData[dynEntIndex] = 1;
            }
        }
    }
}

