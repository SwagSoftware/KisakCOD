#include "r_dpvs.h"
#include "r_dvars.h"
#include <cgame/cg_local.h>

void __cdecl R_AddAabbTreeSurfacesInFrustum_r(const GfxAabbTree *tree, const DpvsClipPlaneSet *clipSet)
{
    int v2; // [esp+10h] [ebp-D0h]
    int v3; // [esp+14h] [ebp-CCh]
    const DpvsPlane *v4; // [esp+18h] [ebp-C8h]
    unsigned int m; // [esp+1Ch] [ebp-C4h]
    GfxSurface *v6; // [esp+24h] [ebp-BCh]
    int v7; // [esp+28h] [ebp-B8h]
    int v8; // [esp+2Ch] [ebp-B4h]
    const DpvsPlane *v9; // [esp+30h] [ebp-B0h]
    unsigned int j; // [esp+34h] [ebp-ACh]
    GfxStaticModelInst *v11; // [esp+38h] [ebp-A8h]
    unsigned int surfaceCountNoDecal; // [esp+3Ch] [ebp-A4h]
    unsigned __int16 *v13; // [esp+40h] [ebp-A0h]
    int startSurfIndexNoDecal; // [esp+44h] [ebp-9Ch]
    unsigned __int16 *smodelIndexes; // [esp+48h] [ebp-98h]
    unsigned int v16; // [esp+4Ch] [ebp-94h]
    unsigned int k; // [esp+50h] [ebp-90h]
    unsigned int i; // [esp+54h] [ebp-8Ch]
    unsigned int surfaceCount; // [esp+58h] [ebp-88h]
    unsigned __int16 *v20; // [esp+5Ch] [ebp-84h]
    unsigned int startSurfIndex; // [esp+60h] [ebp-80h]
    unsigned __int16 *indices; // [esp+64h] [ebp-7Ch]
    unsigned int smodelIndexCount; // [esp+68h] [ebp-78h]
    const DpvsPlane *plane; // [esp+6Ch] [ebp-74h]
    unsigned int planeCount; // [esp+70h] [ebp-70h]
    const GfxAabbTree *children; // [esp+74h] [ebp-6Ch]
    unsigned int smodelIndexIter; // [esp+78h] [ebp-68h]
    DpvsClipPlaneSet clipSetChild; // [esp+80h] [ebp-60h] BYREF
    unsigned int childIndex; // [esp+CCh] [ebp-14h]
    unsigned int surfNodeIndex; // [esp+D0h] [ebp-10h]
    unsigned int planeIndex; // [esp+D4h] [ebp-Ch]
    unsigned int childCount; // [esp+D8h] [ebp-8h]
    unsigned int smodelIndex; // [esp+DCh] [ebp-4h]

    clipSetChild.count = 0;
    planeCount = clipSet->count;
    for (planeIndex = 0; planeIndex < planeCount; ++planeIndex)
    {
        plane = clipSet->planes[planeIndex];
        if (*(float *)((char *)tree->mins + plane->side[0]) * plane->coeffs[0]
            + plane->coeffs[3]
            + *(float *)((char *)tree->mins + plane->side[1]) * plane->coeffs[1]
            + *(float *)((char *)tree->mins + plane->side[2]) * plane->coeffs[2] <= 0.0)
            return;
        if (*(float *)((char *)tree->maxs - plane->side[0]) * plane->coeffs[0]
            + plane->coeffs[3]
            + *(float *)((char *)&tree->maxs[2] - plane->side[1]) * plane->coeffs[1]
            + *(float *)((char *)&tree->startSurfIndex - plane->side[2]) * plane->coeffs[2] < 0.0)
            clipSetChild.planes[clipSetChild.count++] = plane;
    }
    if (clipSetChild.count)
    {
        if (tree->childCount)
        {
            children = (const GfxAabbTree *)((char *)tree + tree->childrenOffset);
            childCount = tree->childCount;
            for (childIndex = 0; childIndex < childCount; ++childIndex)
                R_AddAabbTreeSurfacesInFrustum_r(&children[childIndex], &clipSetChild);
        }
        else
        {
            if (r_showAabbTrees->current.integer)
                R_AddDebugBox(&frontEndDataOut->debugGlobals, tree->mins, tree->maxs, colorOrange);
            if (rg.drawSModels)
            {
                v16 = tree->smodelIndexCount;
                if (tree->smodelIndexCount)
                {
                    smodelIndexes = tree->smodelIndexes;
                    for (i = 0; i < v16; ++i)
                    {
                        v7 = smodelIndexes[i];
                        //if (!*(_BYTE *)(*(_DWORD *)(*((_DWORD *)NtCurrentTeb()->ThreadLocalStoragePointer + _tls_index) + 24) + v7))
                        if (!g_smodelVisData[v7])
                        {
                            for (j = 0; j < clipSetChild.count; ++j)
                            {
                                v9 = clipSetChild.planes[j];
                                v11 = &rgp.world->dpvs.smodelInsts[v7];
                                if (*(float *)((char *)v11->mins + v9->side[0]) * v9->coeffs[0]
                                    + v9->coeffs[3]
                                    + *(float *)((char *)v11->mins + v9->side[1]) * v9->coeffs[1]
                                    + *(float *)((char *)v11->mins + v9->side[2]) * v9->coeffs[2] <= 0.0)
                                {
                                    v8 = 1;
                                    goto LABEL_42;
                                }
                            }
                            v8 = 0;
                        LABEL_42:
                            if (!v8)
                            {
                                //*(_BYTE *)(*(_DWORD *)(*((_DWORD *)NtCurrentTeb()->ThreadLocalStoragePointer + _tls_index) + 24) + v7) = 1;
                                g_smodelVisData[v7] = 1;
                            }
                        }
                    }
                }
            }
            if (rg.drawWorld)
            {
                if (r_drawDecals->current.enabled)
                {
                    startSurfIndexNoDecal = tree->startSurfIndex;
                    surfaceCountNoDecal = tree->surfaceCount;
                }
                else
                {
                    startSurfIndexNoDecal = tree->startSurfIndexNoDecal;
                    surfaceCountNoDecal = tree->surfaceCountNoDecal;
                }
                if (surfaceCountNoDecal)
                {
                    v13 = &rgp.world->dpvs.sortedSurfIndex[startSurfIndexNoDecal];
                    for (k = 0; k < surfaceCountNoDecal; ++k)
                    {
                        v2 = v13[k];
                        //if (!*(_BYTE *)(*(_DWORD *)(*((_DWORD *)NtCurrentTeb()->ThreadLocalStoragePointer + _tls_index) + 28) + v2))
                        if (!g_surfaceVisData[v2])
                        {
                            v6 = &rgp.world->dpvs.surfaces[v2];
                            for (m = 0; m < clipSetChild.count; ++m)
                            {
                                v4 = clipSetChild.planes[m];
                                if (*(float *)((char *)v6->bounds[0] + v4->side[0]) * v4->coeffs[0]
                                    + v4->coeffs[3]
                                    + *(float *)((char *)v6->bounds[0] + v4->side[1]) * v4->coeffs[1]
                                    + *(float *)((char *)v6->bounds[0] + v4->side[2]) * v4->coeffs[2] <= 0.0)
                                {
                                    v3 = 1;
                                    goto LABEL_59;
                                }
                            }
                            v3 = 0;
                        LABEL_59:
                            if (!v3)
                            {
                                if ((r_showAabbTrees->current.integer & 2) != 0)
                                    R_AddDebugBox(&frontEndDataOut->debugGlobals, v6->bounds[0], v6->bounds[1], colorGreen);
                                //*(_BYTE *)(*(_DWORD *)(*((_DWORD *)NtCurrentTeb()->ThreadLocalStoragePointer + _tls_index) + 28) + v2) = 1;
                                g_surfaceVisData[v2] = 1;
                            }
                        }
                    }
                }
            }
        }
    }
    else
    {
        if (r_showAabbTrees->current.integer)
            R_AddDebugBox(&frontEndDataOut->debugGlobals, tree->mins, tree->maxs, colorYellow);
        if (rg.drawSModels)
        {
            smodelIndexCount = tree->smodelIndexCount;
            if (tree->smodelIndexCount)
            {
                indices = tree->smodelIndexes;
                for (smodelIndexIter = 0; smodelIndexIter < smodelIndexCount; ++smodelIndexIter)
                {
                    smodelIndex = indices[smodelIndexIter];
                    //*(_BYTE *)(*(_DWORD *)(*((_DWORD *)NtCurrentTeb()->ThreadLocalStoragePointer + _tls_index) + 24) + smodelIndex) = 1;
                    g_smodelVisData[smodelIndex] = 1;
                }
            }
        }
        if (rg.drawWorld)
        {
            if (r_drawDecals->current.enabled)
            {
                startSurfIndex = tree->startSurfIndex;
                surfaceCount = tree->surfaceCount;
            }
            else
            {
                startSurfIndex = tree->startSurfIndexNoDecal;
                surfaceCount = tree->surfaceCountNoDecal;
            }
            if (surfaceCount)
            {
                v20 = &rgp.world->dpvs.sortedSurfIndex[startSurfIndex];
                for (surfNodeIndex = 0; surfNodeIndex < surfaceCount; ++surfNodeIndex)
                {
                    g_surfaceVisData[v20[surfNodeIndex]] = 1;
                }
            }
        }
    }
}

void __cdecl R_CopyClipPlane(const DpvsPlane *in, DpvsPlane *out);

void __cdecl R_AddCellStaticSurfacesInFrustum(DpvsStaticCellCmd *dpvsCell)
{
    DpvsPlane clipPlanePool[16]; // [esp+CCh] [ebp-1A0h] BYREF
    const GfxAabbTree *tree; // [esp+210h] [ebp-5Ch]
    DpvsClipPlaneSet clipSet; // [esp+214h] [ebp-58h] BYREF
    DpvsPlanes planes; // [esp+260h] [ebp-Ch]
    unsigned int planeIndex; // [esp+268h] [ebp-4h]

    tree = dpvsCell->cell->aabbTree;
    if (tree)
    {
        planes.count = dpvsCell->planeCount;
        if (planes.count > 16)
            MyAssertHandler(
                ".\\r_dpvs_static.cpp",
                663,
                0,
                "%s\n\t(planes.count) = %i",
                "(planes.count <= (10 + 4 + 2))",
                planes.count);
        iassert( planes.count );
        planes.planes = dpvsCell->planes;
        for (planeIndex = 0; planeIndex < planes.count; ++planeIndex)
        {
            R_CopyClipPlane(&planes.planes[planeIndex], &clipPlanePool[planeIndex]);
            clipSet.planes[planeIndex] = &clipPlanePool[planeIndex];
        }
        clipSet.count = planes.count;
        R_AddAabbTreeSurfacesInFrustum_r(tree, &clipSet);
    }
}

void __cdecl R_AddCullGroupSurfacesInFrustum(int cullGroupIndex, const DpvsPlane *planes, int planeCount)
{
    int v3; // [esp+4h] [ebp-20h]
    int i; // [esp+Ch] [ebp-18h]
    const unsigned __int16 *indices; // [esp+18h] [ebp-Ch]
    GfxCullGroup *group; // [esp+1Ch] [ebp-8h]
    int count; // [esp+20h] [ebp-4h]

    group = &rgp.world->dpvs.cullGroups[cullGroupIndex];
    for (i = 0; i < planeCount; ++i)
    {
        if (*(float *)((char *)group->mins + planes->side[0]) * planes->coeffs[0]
            + planes->coeffs[3]
            + *(float *)((char *)group->mins + planes->side[1]) * planes->coeffs[1]
            + *(float *)((char *)group->mins + planes->side[2]) * planes->coeffs[2] <= 0.0)
        {
            v3 = 1;
            goto LABEL_7;
        }
        ++planes;
    }
    v3 = 0;
LABEL_7:
    if (!v3)
    {
        if ((r_showPortals->current.integer & 1) != 0)
            R_AddDebugBox(&frontEndDataOut->debugGlobals, group->mins, group->maxs, colorLtYellow);
        if (group->surfaceCount)
        {
            indices = &rgp.world->dpvs.sortedSurfIndex[group->startSurfIndex];
            for (count = 0; count < group->surfaceCount; ++count)
            {
                //*(_BYTE *)(*(_DWORD *)(*((_DWORD *)NtCurrentTeb()->ThreadLocalStoragePointer + _tls_index) + 28) + indices[count]) = 1;
                g_surfaceVisData[indices[count]] = 1;
            }
        }
    }
}

void __cdecl R_AddCellCullGroupsInFrustum(DpvsStaticCellCmd *dpvsCell)
{
    const GfxCell *cell; // [esp+0h] [ebp-10h]
    int *cullGroup; // [esp+8h] [ebp-8h]
    int count; // [esp+Ch] [ebp-4h]

    cell = dpvsCell->cell;
    count = cell->cullGroupCount;
    cullGroup = cell->cullGroups;
    while (count)
    {
        R_AddCullGroupSurfacesInFrustum(*cullGroup, dpvsCell->planes, dpvsCell->planeCount);
        --count;
        ++cullGroup;
    }
}

void __cdecl R_AddCellStaticSurfacesInFrustumCmd(DpvsStaticCellCmd *data)
{
    unsigned int viewIndex; // [esp+4h] [ebp-4h]

    viewIndex = data->viewIndex;
    g_smodelVisData = rgp.world->dpvs.smodelVisData[viewIndex];
    g_surfaceVisData = rgp.world->dpvs.surfaceVisData[viewIndex];
    R_AddCellStaticSurfacesInFrustum(data);
    if (rg.drawWorld)
        R_AddCellCullGroupsInFrustum(data);
}