#include "r_dpvs.h"
#include "r_workercmds.h"

// KISAKTODO: cleanup this 
void R_AddCellSceneEntSurfacesInFrustumCmd(GfxWorldDpvsPlanes *data)
{
    bool v2; // zf
    DWORD v3; // eax
    DWORD v4; // eax
    int v5; // [esp-1Ch] [ebp-E4h]
    DpvsPlane *pln; // [esp-14h] [ebp-DCh]
    signed int i; // [esp-10h] [ebp-D8h]
    GfxEntCellRefInfo v8; // [esp-Ch] [ebp-D4h]
    int skipWorkerCmd; // [esp-4h] [ebp-CCh]
    DpvsPlane *plane__; // [esp+4h] [ebp-C4h]
    int v11; // [esp+8h] [ebp-C0h]
    int v12; // [esp+10h] [ebp-B8h]
    DpvsPlane *v13; // [esp+18h] [ebp-B0h]
    signed int itr; // [esp+1Ch] [ebp-ACh]
    float radius_; // [esp+20h] [ebp-A8h]
    GfxSceneEntity *gfxSceneEnt; // [esp+24h] [ebp-A4h]
    int v17; // [esp+28h] [ebp-A0h]
    DpvsPlane *coeffs; // [esp+30h] [ebp-98h]
    signed int v19; // [esp+34h] [ebp-94h]
    float radius; // [esp+38h] [ebp-90h]
    int v21; // [esp+44h] [ebp-84h]
    int v22; // [esp+44h] [ebp-84h]
    int offset_; // [esp+4Ch] [ebp-7Ch]
    int infoa; // [esp+4Ch] [ebp-7Ch]
    unsigned int sceneEntIndex; // [esp+50h] [ebp-78h]
    DWORD sceneEntIndexa; // [esp+50h] [ebp-78h]
    unsigned int bits; // [esp+54h] [ebp-74h]
    unsigned int bita; // [esp+54h] [ebp-74h]
    unsigned int entnum; // [esp+58h] [ebp-70h]
    unsigned int entnuma; // [esp+58h] [ebp-70h]
    signed int indexLow; // [esp+5Ch] [ebp-6Ch]
    const DpvsPlane *plane; // [esp+60h] [ebp-68h]
    unsigned int *wordIndex; // [esp+64h] [ebp-64h]
    unsigned int *wordIndexa; // [esp+64h] [ebp-64h]
    int innerPlaneCount; // [esp+68h] [ebp-60h]
    int innerPlaneCounta; // [esp+68h] [ebp-60h]
    void *data_[4]; // [esp+6Ch] [ebp-5Ch] BYREF
    int frustumPlaneCount; // [esp+88h] [ebp-40h]
    int planeCount_; // [esp+8Ch] [ebp-3Ch]
    DpvsPlane *planes; // [esp+90h] [ebp-38h]
    unsigned int cellIndex_; // [esp+94h] [ebp-34h]
    int planeCount; // [esp+98h] [ebp-30h]
    unsigned __int16 *dobjIndex; // [esp+9Ch] [ebp-2Ch]
    unsigned __int16 *xmodelIndex; // [esp+A0h] [ebp-28h]
    GfxEntCellRefInfo *entInfo; // [esp+A4h] [ebp-24h]
    unsigned int entCountIndex; // [esp+A8h] [ebp-20h]
    GfxWorldDpvsPlanes *dpvsPlanes; // [esp+ACh] [ebp-1Ch]
    unsigned int localClientNum_; // [esp+B0h] [ebp-18h]
    GfxSceneDpvs *dpvs; // [esp+B4h] [ebp-14h]
    DpvsDynamicCellCmd *worldDpvs; // [esp+B8h] [ebp-10h]
    unsigned int localClientNum; // [esp+BCh] [ebp-Ch]
    GfxSceneDpvs *sceneDpvs; // [esp+C0h] [ebp-8h]
    GfxSceneDpvs *retaddr; // [esp+C8h] [ebp+0h]

    //localClientNum = a1;
    //sceneDpvs = retaddr;
    worldDpvs = (DpvsDynamicCellCmd*)data;
    dpvs = &scene.dpvs;
    localClientNum_ = scene.dpvs.localClientNum;
    dpvsPlanes = &rgp.world->dpvsPlanes;
    entCountIndex = gfxCfg.entCount >> 5;
    if (scene.dpvs.localClientNum >= gfxCfg.maxClientViews)
        MyAssertHandler(
            ".\\r_dpvs_sceneent.cpp",
            136,
            0,
            "localClientNum doesn't index gfxCfg.maxClientViews\n\t%i not in [0, %i)",
            localClientNum_,
            gfxCfg.maxClientViews);
    entInfo = scene.dpvs.entInfo[localClientNum_];
    xmodelIndex = scene.dpvs.sceneXModelIndex;
    dobjIndex = scene.dpvs.sceneDObjIndex;
    planeCount = worldDpvs->viewIndex;
    cellIndex_ = worldDpvs->cellIndex;
    planes = (DpvsPlane *)worldDpvs->planes;
    planeCount_ = worldDpvs->planeCount;
    frustumPlaneCount = worldDpvs->frustumPlaneCount;
    if (cellIndex_ >= dpvsPlanes->cellCount)
        MyAssertHandler(
            ".\\r_dpvs_sceneent.cpp",
            148,
            0,
            "cellIndex doesn't index worldDpvs->cellCount\n\t%i not in [0, %i)",
            cellIndex_,
            dpvsPlanes->cellCount);
    data_[3] = dpvs->entVisData[planeCount];
    data_[1] = planes;
    LOWORD(data_[2]) = planeCount_;
    HIWORD(data_[2]) = cellIndex_;
    innerPlaneCount = entCountIndex * localClientNum_;
    if (entCountIndex * localClientNum_ >= 0x80)
        MyAssertHandler(
            ".\\r_dpvs_sceneent.cpp",
            157,
            0,
            "offset doesn't index MAX_TOTAL_ENT_COUNT >> 5\n\t%i not in [0, %i)",
            innerPlaneCount,
            128);
    innerPlaneCounta = innerPlaneCount + (cellIndex_ << 7);
    wordIndex = &dpvsPlanes->sceneEntCellBits[innerPlaneCounta];
    //iassert( frustumPlaneCount <= planeCount );
    plane = &planes[frustumPlaneCount];
    indexLow = planeCount_ - frustumPlaneCount;
    for (entnum = 0; entnum < entCountIndex; ++entnum)
    {
        bits = wordIndex[entnum];
        while (1)
        {
            v2 = !_BitScanReverse(&v3, bits);
            if (v2)
                v3 = 63;
            sceneEntIndex = v3 ^ 0x1F;
            if ((v3 ^ 0x1Fu) >= 0x20)
                break;
            offset_ = sceneEntIndex + 32 * entnum;
            unsigned int bit = (0x80000000 >> sceneEntIndex);
            iassert( bits & bit );
            bits &= ~bit;
            if (!*((_BYTE *)data_[3] + offset_))
            {
                v21 = dobjIndex[offset_];
                if (v21 == 0xFFFF)
                {
                    v22 = xmodelIndex[offset_];
                    if (v22 != 0xFFFF)
                    {
                        radius = entInfo[offset_].radius;
                        v19 = 0;
                        coeffs = (DpvsPlane *)plane;
                        while (v19 < indexLow)
                        {
                            if (Vec3Dot(coeffs->coeffs, scene.sceneModel[v22].placement.base.origin)
                                + coeffs->coeffs[3]
                                + radius <= 0.0)
                            {
                                v17 = 1;
                                goto LABEL_26;
                            }
                            ++v19;
                            ++coeffs;
                        }
                        v17 = 0;
                    LABEL_26:
                        if (!v17)
                            *((_BYTE *)data_[3] + offset_) = 1;
                    }
                }
                else
                {
                    gfxSceneEnt = &scene.sceneDObj[v21];
                    radius_ = entInfo[offset_].radius;
                    itr = 0;
                    v13 = (DpvsPlane *)plane;
                    while (itr < indexLow)
                    {
                        if (Vec3Dot(v13->coeffs, gfxSceneEnt->placement.base.origin)
                            + v13->coeffs[3]
                            + radius_ <= 0.0)
                        {
                            v12 = 1;
                            goto LABEL_34;
                        }
                        ++itr;
                        ++v13;
                    }
                    v12 = 0;
                LABEL_34:
                    if (!v12)
                    {
                        v11 = 0;
                        plane__ = planes;
                        while (v11 < planeCount_)
                        {
                            if (*(float *)((char *)gfxSceneEnt->cull.mins + plane__->side[0]) * plane__->coeffs[0]
                                + plane__->coeffs[3]
                                + *(float *)((char *)gfxSceneEnt->cull.mins + plane__->side[1]) * plane__->coeffs[1]
                                + *(float *)((char *)gfxSceneEnt->cull.mins + plane__->side[2]) * plane__->coeffs[2] <= 0.0)
                            {
                                skipWorkerCmd = 1;
                                goto LABEL_41;
                            }
                            ++v11;
                            ++plane__;
                        }
                        skipWorkerCmd = 0;
                    LABEL_41:
                        if (!skipWorkerCmd)
                        {
                            data_[0] = &scene.sceneDObj[v21];
                            if (gfxSceneEnt->cull.state < 2)
                                R_AddWorkerCmd(7, (unsigned __int8 *)data_);
                            else
                                R_AddEntitySurfacesInFrustumCmd((unsigned __int16 *)data_);
                        }
                    }
                }
            }
        }
    }
    wordIndexa = &dpvsPlanes->sceneEntCellBits[128 * dpvsPlanes->cellCount + innerPlaneCounta];
    for (entnuma = 0; entnuma < entCountIndex; ++entnuma)
    {
        bita = wordIndexa[entnuma];
        while (1)
        {
            v2 = !_BitScanReverse(&v4, bita);
            if (v2)
                v4 = 63;
            sceneEntIndexa = v4 ^ 0x1F;
            if ((v4 ^ 0x1F) >= 0x20)
                break;
            infoa = sceneEntIndexa + 32 * entnuma;
            unsigned int bit = (0x80000000 >> sceneEntIndexa);
            iassert( bita & bit ); // should be `bits` instead of `bita`
            bita &= ~bit;
            if (!*((_BYTE *)data_[3] + infoa))
            {
                i = 0;
                pln = (DpvsPlane *)plane;
                while (i < indexLow)
                {
                    v8.radius = entInfo[infoa].radius;
                    if (*(float *)((char *)v8.bmodel->writable.mins + pln->side[0]) * pln->coeffs[0]
                        + pln->coeffs[3]
                        + *(float *)((char *)v8.bmodel->writable.mins + pln->side[1]) * pln->coeffs[1]
                        + *(float *)((char *)v8.bmodel->writable.mins + pln->side[2]) * pln->coeffs[2] <= 0.0)
                    {
                        v5 = 1;
                        goto LABEL_61;
                    }
                    ++i;
                    ++pln;
                }
                v5 = 0;
            LABEL_61:
                if (!v5)
                    *((_BYTE *)data_[3] + infoa) = 1;
            }
        }
    }
}