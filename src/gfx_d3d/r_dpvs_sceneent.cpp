#include "r_dpvs.h"
#include "r_workercmds.h"

static int __cdecl R_CullSphereDpvs(const float *origin, float radius, const DpvsPlane *planes, int planeCount)
{
    int planeIndex; // [esp+8h] [ebp-4h]

    for (planeIndex = 0; planeIndex < planeCount; ++planeIndex)
    {
        if ((float)((float)((float)((float)((float)(planes->coeffs[0] * *origin) + (float)(planes->coeffs[1] * origin[1]))
            + (float)(planes->coeffs[2] * origin[2]))
            + planes->coeffs[3])
            + radius) <= 0.0)
            return 1;
        ++planes;
    }
    return 0;
}

// Blops augmented 
void R_AddCellSceneEntSurfacesInFrustumCmd(GfxWorldDpvsPlanes *data)
{
    bool v2; // zf
    DWORD v3; // eax
    DWORD v4; // eax
    int v5; // [esp+18h] [ebp-C0h]
    const float *mins; // [esp+28h] [ebp-B0h]
    int skipWorkerCmd; // [esp+30h] [ebp-A8h]
    const DpvsPlane *bmodel; // [esp+38h] [ebp-A0h]
    GfxSceneEntity *sceneEnt; // [esp+40h] [ebp-98h]
    unsigned int sceneEntIndex; // [esp+4Ch] [ebp-8Ch]
    unsigned int entnum; // [esp+54h] [ebp-84h]
    unsigned int indexLow; // [esp+58h] [ebp-80h]
    unsigned int bits; // [esp+5Ch] [ebp-7Ch]
    unsigned int wordIndex; // [esp+60h] [ebp-78h]
    int innerPlaneCount; // [esp+64h] [ebp-74h]
    const DpvsPlane *innerPlanes; // [esp+68h] [ebp-70h]
    unsigned int *entCellBits; // [esp+6Ch] [ebp-6Ch]
    const DpvsPlane *planes; // [esp+70h] [ebp-68h]
    unsigned int offset; // [esp+78h] [ebp-60h]
    DpvsEntityCmd dpvsEntity; // [esp+7Ch] [ebp-5Ch] BYREF
    int frustumPlaneCount; // [esp+98h] [ebp-40h]
    int planeCount; // [esp+9Ch] [ebp-3Ch]
    const DpvsPlane *planesEA; // [esp+A0h] [ebp-38h]
    unsigned int cellIndex; // [esp+A4h] [ebp-34h]
    unsigned int viewIndex; // [esp+A8h] [ebp-30h]
    unsigned __int16 *sceneDObjIndex; // [esp+ACh] [ebp-2Ch]
    unsigned __int16 *sceneXModelIndex; // [esp+B0h] [ebp-28h]
    GfxEntCellRefInfo *entInfo; // [esp+B4h] [ebp-24h]
    unsigned int wordCount; // [esp+B8h] [ebp-20h]
    GfxWorldDpvsPlanes *worldDpvsPlanes; // [esp+BCh] [ebp-1Ch]
    unsigned int localClientNum; // [esp+C0h] [ebp-18h]
    GfxSceneDpvs *sceneDpvs; // [esp+C4h] [ebp-14h]
    DpvsDynamicCellCmd *dpvsCell; // [esp+C8h] [ebp-10h]

    dpvsCell = (DpvsDynamicCellCmd*)data;
    sceneDpvs = &scene.dpvs;
    localClientNum = scene.dpvs.localClientNum;
    worldDpvsPlanes = &rgp.world->dpvsPlanes;
    wordCount = gfxCfg.entCount >> 5;

    bcassert(localClientNum, gfxCfg.maxClientViews);

    //entInfo = (GfxEntCellRefInfo *)scene.dynSModelVisBitsCamera[localClientNum - 4];
    entInfo = scene.dpvs.entInfo[localClientNum];
    sceneXModelIndex = scene.dpvs.sceneXModelIndex;
    sceneDObjIndex = scene.dpvs.sceneDObjIndex;
    viewIndex = dpvsCell->viewIndex;
    cellIndex = dpvsCell->cellIndex;
    planesEA = dpvsCell->planes;
    planeCount = dpvsCell->planeCount;
    frustumPlaneCount = dpvsCell->frustumPlaneCount;

    bcassert(cellIndex, worldDpvsPlanes->cellCount);

    dpvsEntity.entVisData = sceneDpvs->entVisData[viewIndex];
    dpvsEntity.planes = planesEA;
    dpvsEntity.planeCount = planeCount;
    dpvsEntity.cellIndex = cellIndex;

    offset = wordCount * localClientNum;

    bcassert(offset, MAX_TOTAL_ENT_COUNT >> 5);

    //offseta = offset + (cellIndex << 8);
    offset += (cellIndex << 7);
    planes = planesEA;
    entCellBits = &worldDpvsPlanes->sceneEntCellBits[offset];

    iassert(frustumPlaneCount <= planeCount);

    innerPlanes = &planesEA[frustumPlaneCount];
    innerPlaneCount = planeCount - frustumPlaneCount;
    for (wordIndex = 0; wordIndex < wordCount; ++wordIndex)
    {
        bits = entCellBits[wordIndex];
        while (1)
        {
            v2 = !_BitScanReverse(&v3, bits);
            if (v2)
                v3 = 63;

            indexLow = v3 ^ 31;
            if ((v3 ^ 31) >= 32)
                break;

            entnum = indexLow + 32 * wordIndex;
            unsigned int bit = (0x80000000 >> indexLow);
            iassert(bits & bit);

            bits &= ~(bit);
            if (!dpvsEntity.entVisData[entnum])
            {
                sceneEntIndex = sceneDObjIndex[entnum];
                if (sceneEntIndex == 0xFFFF)
                {
                    sceneEntIndex = sceneXModelIndex[entnum];
                    if (sceneEntIndex != 0xFFFF
                        && !R_CullSphereDpvs(
                            scene.sceneModel[sceneEntIndex].placement.base.origin,
                            entInfo[entnum].radius,
                            innerPlanes,
                            innerPlaneCount))
                    {
                        dpvsEntity.entVisData[entnum] = 1;
                    }
                }
                else
                {
                    sceneEnt = &scene.sceneDObj[sceneEntIndex];
                    if (!R_CullSphereDpvs(sceneEnt->placement.base.origin, entInfo[entnum].radius, innerPlanes, innerPlaneCount))
                    {
                        int itr = 0;
                        if (sceneEnt->cull.state < 2)
                            goto LABEL_36;

                        bmodel = planes;
                        while (itr < planeCount)
                        {
                            if (R_DpvsPlaneMaxSignedDistToBox(bmodel, sceneEnt->cull.mins) <= 0.0)
                            {
                                skipWorkerCmd = 1;
                                goto LABEL_35;
                            }
                            ++itr;
                            ++bmodel;
                        }
                        skipWorkerCmd = 0;
                    LABEL_35:
                        if (!skipWorkerCmd)
                        {
                        LABEL_36:
                            dpvsEntity.sceneEnt = &scene.sceneDObj[sceneEntIndex];
                            if (sceneEnt->cull.state < 2)
                                R_AddWorkerCmd(WRKCMD_DPVS_ENTITY, (unsigned __int8 *)&dpvsEntity);
                            else
                                R_AddEntitySurfacesInFrustumCmd((unsigned __int16 *)&dpvsEntity);
                        }
                    }
                }
            }
        }
    }


    //entCellBits = &worldDpvsPlanes->sceneEntCellBits[256 * worldDpvsPlanes->cellCount + offset];
    entCellBits = &worldDpvsPlanes->sceneEntCellBits[128 * worldDpvsPlanes->cellCount + offset];
    for (wordIndex = 0; wordIndex < wordCount; ++wordIndex)
    {
        bits = entCellBits[wordIndex];
        while (1)
        {
            v2 = !_BitScanReverse(&v4, bits);
            if (v2)
                v4 = 63;
            indexLow = v4 ^ 31;
            if ((v4 ^ 31) >= 32)
                break;
            entnum = indexLow + 32 * wordIndex;

            unsigned int bit = (0x80000000 >> indexLow);
            iassert(bits & bit);

            bits &= ~(bit);
            if (!dpvsEntity.entVisData[entnum])
            {
                mins = entInfo[entnum].bmodel->writable.mins;
                int itr = 0;
                const DpvsPlane *plane = innerPlanes;
                while (itr < innerPlaneCount)
                {
                    if (R_DpvsPlaneMaxSignedDistToBox(plane, mins) <= 0.0)
                    {
                        v5 = 1;
                        goto LABEL_56;
                    }
                    ++itr;
                    ++plane;
                }
                v5 = 0;
            LABEL_56:
                if (!v5)
                    dpvsEntity.entVisData[entnum] = 1;
            }
        }
    }
}