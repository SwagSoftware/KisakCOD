#include "r_dpvs.h"


void __usercall R_AddCellSceneEntSurfacesInFrustumCmd(unsigned int a1@<ebp>, GfxWorldDpvsPlanes *data)
{
    bool v2; // zf
    int v3; // eax
    int v4; // eax
    int v5; // [esp-1Ch] [ebp-E4h]
    unsigned int v6; // [esp-14h] [ebp-DCh]
    signed int v7; // [esp-10h] [ebp-D8h]
    int v8; // [esp-Ch] [ebp-D4h]
    int v9; // [esp-4h] [ebp-CCh]
    const GfxBrushModel *bmodel; // [esp+4h] [ebp-C4h]
    int v11; // [esp+8h] [ebp-C0h]
    int v12; // [esp+10h] [ebp-B8h]
    unsigned int v13; // [esp+18h] [ebp-B0h]
    signed int v14; // [esp+1Ch] [ebp-ACh]
    float v15; // [esp+20h] [ebp-A8h]
    GfxSceneEntity *v16; // [esp+24h] [ebp-A4h]
    int v17; // [esp+28h] [ebp-A0h]
    GfxSceneEntity *sceneEnt; // [esp+30h] [ebp-98h]
    signed int v19; // [esp+34h] [ebp-94h]
    float v20; // [esp+38h] [ebp-90h]
    int v21; // [esp+44h] [ebp-84h]
    int v22; // [esp+44h] [ebp-84h]
    GfxEntCellRefInfo *info; // [esp+4Ch] [ebp-7Ch]
    GfxEntCellRefInfo *infoa; // [esp+4Ch] [ebp-7Ch]
    unsigned int sceneEntIndex; // [esp+50h] [ebp-78h]
    unsigned int sceneEntIndexa; // [esp+50h] [ebp-78h]
    unsigned int bit; // [esp+54h] [ebp-74h]
    unsigned int bita; // [esp+54h] [ebp-74h]
    unsigned int entnum; // [esp+58h] [ebp-70h]
    unsigned int entnuma; // [esp+58h] [ebp-70h]
    signed int indexLow; // [esp+5Ch] [ebp-6Ch]
    unsigned int bits; // [esp+60h] [ebp-68h]
    unsigned int wordIndex; // [esp+64h] [ebp-64h]
    unsigned int wordIndexa; // [esp+64h] [ebp-64h]
    int innerPlaneCount; // [esp+68h] [ebp-60h]
    int innerPlaneCounta; // [esp+68h] [ebp-60h]
    const DpvsPlane *innerPlanes[2]; // [esp+6Ch] [ebp-5Ch] BYREF
    __int16 offset; // [esp+74h] [ebp-54h]
    __int16 offset_2; // [esp+76h] [ebp-52h]
    DpvsEntityCmd dpvsEntity; // [esp+78h] [ebp-50h]
    int v41; // [esp+88h] [ebp-40h]
    int nodes_low; // [esp+8Ch] [ebp-3Ch]
    const DpvsPlane *cellCount; // [esp+90h] [ebp-38h]
    int frustumPlaneCount; // [esp+94h] [ebp-34h]
    int planeCount; // [esp+98h] [ebp-30h]
    const DpvsPlane *planes; // [esp+9Ch] [ebp-2Ch]
    unsigned int cellIndex; // [esp+A0h] [ebp-28h]
    unsigned int viewIndex; // [esp+A4h] [ebp-24h]
    unsigned __int16 *sceneDObjIndex; // [esp+A8h] [ebp-20h]
    unsigned __int16 *sceneXModelIndex; // [esp+ACh] [ebp-1Ch]
    GfxEntCellRefInfo *entInfo; // [esp+B0h] [ebp-18h]
    unsigned int wordCount; // [esp+B4h] [ebp-14h]
    GfxWorldDpvsPlanes *worldDpvs; // [esp+B8h] [ebp-10h]
    unsigned int localClientNum; // [esp+BCh] [ebp-Ch]
    GfxSceneDpvs *sceneDpvs; // [esp+C0h] [ebp-8h]
    GfxSceneDpvs *retaddr; // [esp+C8h] [ebp+0h]

    localClientNum = a1;
    sceneDpvs = retaddr;
    worldDpvs = data;
    wordCount = (unsigned int)&scene.dpvs;
    entInfo = (GfxEntCellRefInfo *)scene.dpvs.localClientNum;
    sceneXModelIndex = (unsigned __int16 *)&rgp.world->dpvsPlanes;
    sceneDObjIndex = (unsigned __int16 *)(gfxCfg.entCount >> 5);
    if (scene.dpvs.localClientNum >= gfxCfg.maxClientViews)
        MyAssertHandler(
            ".\\r_dpvs_sceneent.cpp",
            136,
            0,
            "localClientNum doesn't index gfxCfg.maxClientViews\n\t%i not in [0, %i)",
            entInfo,
            gfxCfg.maxClientViews);
    viewIndex = (unsigned int)scene.dpvs.entInfo[(_DWORD)entInfo];
    cellIndex = (unsigned int)scene.dpvs.sceneXModelIndex;
    planes = (const DpvsPlane *)scene.dpvs.sceneDObjIndex;
    planeCount = HIWORD(worldDpvs->nodes);
    frustumPlaneCount = (int)worldDpvs->planes;
    cellCount = (const DpvsPlane *)worldDpvs->cellCount;
    nodes_low = LOBYTE(worldDpvs->nodes);
    v41 = BYTE1(worldDpvs->nodes);
    if ((unsigned int)frustumPlaneCount >= *(_DWORD *)sceneXModelIndex)
        MyAssertHandler(
            ".\\r_dpvs_sceneent.cpp",
            148,
            0,
            "cellIndex doesn't index worldDpvs->cellCount\n\t%i not in [0, %i)",
            frustumPlaneCount,
            *(_DWORD *)sceneXModelIndex);
    dpvsEntity.sceneEnt = *(GfxSceneEntity **)(wordCount + 4 * planeCount + 4);
    innerPlanes[1] = cellCount;
    offset = nodes_low;
    offset_2 = frustumPlaneCount;
    innerPlaneCount = (_DWORD)sceneDObjIndex * (_DWORD)entInfo;
    if ((unsigned int)((_DWORD)sceneDObjIndex * (_DWORD)entInfo) >= 0x80)
        MyAssertHandler(
            ".\\r_dpvs_sceneent.cpp",
            157,
            0,
            "offset doesn't index MAX_TOTAL_ENT_COUNT >> 5\n\t%i not in [0, %i)",
            innerPlaneCount,
            128);
    innerPlaneCounta = innerPlaneCount + (frustumPlaneCount << 7);
    wordIndex = *((_DWORD *)sceneXModelIndex + 3) + 4 * innerPlaneCounta;
    if (v41 > nodes_low)
        MyAssertHandler(".\\r_dpvs_sceneent.cpp", 181, 0, "%s", "frustumPlaneCount <= planeCount");
    bits = (unsigned int)&cellCount[v41];
    indexLow = nodes_low - v41;
    for (entnum = 0; entnum < (unsigned int)sceneDObjIndex; ++entnum)
    {
        bit = *(_DWORD *)(wordIndex + 4 * entnum);
        while (1)
        {
            v2 = !_BitScanReverse((unsigned int *)&v3, bit);
            if (v2)
                v3 = `CountLeadingZeros'::`2': : notFound;
            sceneEntIndex = v3 ^ 0x1F;
            if ((v3 ^ 0x1Fu) >= 0x20)
                break;
            info = (GfxEntCellRefInfo *)(sceneEntIndex + 32 * entnum);
            if (((0x80000000 >> sceneEntIndex) & bit) == 0)
                MyAssertHandler(".\\r_dpvs_sceneent.cpp", 198, 0, "%s", "bits & bit");
            bit &= ~(0x80000000 >> sceneEntIndex);
            if (!*((_BYTE *)&info->radius + (unsigned int)dpvsEntity.sceneEnt))
            {
                v21 = *((unsigned __int16 *)planes->coeffs + (_DWORD)info);
                if (v21 == 0xFFFF)
                {
                    v22 = *(unsigned __int16 *)(cellIndex + 2 * (_DWORD)info);
                    if (v22 != 0xFFFF)
                    {
                        v20 = *(float *)(viewIndex + 4 * (_DWORD)info);
                        v19 = 0;
                        sceneEnt = (GfxSceneEntity *)bits;
                        while (v19 < indexLow)
                        {
                            if (Vec3Dot(sceneEnt->lightingOrigin, scene.sceneModel[v22].placement.base.origin)
                                + sceneEnt->placement.base.quat[0]
                                + v20 <= 0.0)
                            {
                                v17 = 1;
                                goto LABEL_26;
                            }
                            ++v19;
                            sceneEnt = (GfxSceneEntity *)((char *)sceneEnt + 20);
                        }
                        v17 = 0;
                    LABEL_26:
                        if (!v17)
                            *((_BYTE *)&info->radius + (unsigned int)dpvsEntity.sceneEnt) = 1;
                    }
                }
                else
                {
                    v16 = &scene.sceneDObj[v21];
                    v15 = *(float *)(viewIndex + 4 * (_DWORD)info);
                    v14 = 0;
                    v13 = bits;
                    while (v14 < indexLow)
                    {
                        if (Vec3Dot((const float *)v13, v16->placement.base.origin) + *(float *)(v13 + 12) + v15 <= 0.0)
                        {
                            v12 = 1;
                            goto LABEL_34;
                        }
                        ++v14;
                        v13 += 20;
                    }
                    v12 = 0;
                LABEL_34:
                    if (!v12)
                    {
                        v11 = 0;
                        bmodel = (const GfxBrushModel *)cellCount;
                        while (v11 < nodes_low)
                        {
                            if (*(float *)((char *)v16->cull.mins + LOBYTE(bmodel->writable.maxs[1])) * bmodel->writable.mins[0]
                                + bmodel->writable.maxs[0]
                                + *(float *)((char *)v16->cull.mins + BYTE1(bmodel->writable.maxs[1])) * bmodel->writable.mins[1]
                                + *(float *)((char *)v16->cull.mins + BYTE2(bmodel->writable.maxs[1])) * bmodel->writable.mins[2] <= 0.0)
                            {
                                v9 = 1;
                                goto LABEL_41;
                            }
                            ++v11;
                            bmodel = (const GfxBrushModel *)((char *)bmodel + 20);
                        }
                        v9 = 0;
                    LABEL_41:
                        if (!v9)
                        {
                            innerPlanes[0] = (const DpvsPlane *)&scene.sceneDObj[v21];
                            if (v16->cull.state < 2)
                                R_AddWorkerCmd(7, (unsigned __int8 *)innerPlanes);
                            else
                                R_AddEntitySurfacesInFrustumCmd((unsigned __int16 *)innerPlanes);
                        }
                    }
                }
            }
        }
    }
    wordIndexa = *((_DWORD *)sceneXModelIndex + 3) + 4 * innerPlaneCounta + (*(_DWORD *)sceneXModelIndex << 9);
    for (entnuma = 0; entnuma < (unsigned int)sceneDObjIndex; ++entnuma)
    {
        bita = *(_DWORD *)(wordIndexa + 4 * entnuma);
        while (1)
        {
            v2 = !_BitScanReverse((unsigned int *)&v4, bita);
            if (v2)
                v4 = `CountLeadingZeros'::`2': : notFound;
            sceneEntIndexa = v4 ^ 0x1F;
            if ((v4 ^ 0x1Fu) >= 0x20)
                break;
            infoa = (GfxEntCellRefInfo *)(sceneEntIndexa + 32 * entnuma);
            if (((0x80000000 >> sceneEntIndexa) & bita) == 0)
                MyAssertHandler(".\\r_dpvs_sceneent.cpp", 280, 0, "%s", "bits & bit");
            bita &= ~(0x80000000 >> sceneEntIndexa);
            if (!*((_BYTE *)&infoa->radius + (unsigned int)dpvsEntity.sceneEnt))
            {
                v7 = 0;
                v6 = bits;
                while (v7 < indexLow)
                {
                    v8 = *(_DWORD *)(viewIndex + 4 * (_DWORD)infoa);
                    if (*(float *)(v8 + *(unsigned __int8 *)(v6 + 16)) * *(float *)v6
                        + *(float *)(v6 + 12)
                        + *(float *)(v8 + *(unsigned __int8 *)(v6 + 17)) * *(float *)(v6 + 4)
                        + *(float *)(v8 + *(unsigned __int8 *)(v6 + 18)) * *(float *)(v6 + 8) <= 0.0)
                    {
                        v5 = 1;
                        goto LABEL_61;
                    }
                    ++v7;
                    v6 += 20;
                }
                v5 = 0;
            LABEL_61:
                if (!v5)
                    *((_BYTE *)&infoa->radius + (unsigned int)dpvsEntity.sceneEnt) = 1;
            }
        }
    }
}