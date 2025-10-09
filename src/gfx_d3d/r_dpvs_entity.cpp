#include "r_dpvs.h"
#include "r_model_pose.h"
#include "r_dobj_skin.h"
#include <cgame/cg_local.h>



void __cdecl R_AddEntitySurfacesInFrustumCmd(unsigned __int16 *data)
{
    int v1; // [esp+4h] [ebp-28h]
    const DpvsPlane *v2; // [esp+Ch] [ebp-20h]
    int v3; // [esp+10h] [ebp-1Ch]
    DObjAnimMat *boneMatrix; // [esp+14h] [ebp-18h]
    const DObj_s *obj; // [esp+1Ch] [ebp-10h] BYREF
    GfxSceneEntity *localSceneEnt; // [esp+20h] [ebp-Ch] BYREF
    const DpvsPlane *planes; // [esp+24h] [ebp-8h]
    GfxSceneEntity *sceneEnt; // [esp+28h] [ebp-4h]

    sceneEnt = *(GfxSceneEntity **)data;
    boneMatrix = R_UpdateSceneEntBounds(sceneEnt, &localSceneEnt, &obj, 1);
    if (boneMatrix)
    {
        iassert( localSceneEnt );
        planes = (const DpvsPlane *)*((unsigned int *)data + 1);
        v3 = 0;
        v2 = planes;
        while (v3 < data[4])
        {
            if (*(float *)((char *)localSceneEnt->cull.mins + v2->side[0]) * v2->coeffs[0]
                + v2->coeffs[3]
                + *(float *)((char *)localSceneEnt->cull.mins + v2->side[1]) * v2->coeffs[1]
                + *(float *)((char *)localSceneEnt->cull.mins + v2->side[2]) * v2->coeffs[2] <= 0.0)
            {
                v1 = 1;
                goto LABEL_13;
            }
            ++v3;
            ++v2;
        }
        v1 = 0;
    LABEL_13:
        if (!v1
            && R_BoundsInCell(
                (mnode_t *)rgp.world->dpvsPlanes.nodes,
                data[5],
                localSceneEnt->cull.mins,
                localSceneEnt->cull.maxs))
        {
            CG_CullIn(localSceneEnt->info.pose);
            R_SkinSceneDObj(sceneEnt, localSceneEnt, obj, boneMatrix, 0);
            iassert( localSceneEnt->entnum != gfxCfg.entnumNone );
            *(_BYTE *)(localSceneEnt->entnum + *((unsigned int *)data + 3)) = 1;
        }
        else
        {
            CG_UsedDObjCalcPose(localSceneEnt->info.pose);
        }
    }
    else if (localSceneEnt)
    {
        CG_UsedDObjCalcPose(localSceneEnt->info.pose);
    }
}

bool __cdecl R_BoundsInCell(mnode_t *node, int findCellIndex, const float *mins, const float *maxs)
{
    return R_BoundsInCell_r(node, findCellIndex, mins, maxs);
}

bool __cdecl R_BoundsInCell_r(mnode_t *node, int findCellIndex, const float *mins, const float *maxs)
{
    float localmaxs[3]; // [esp+0h] [ebp-58h]
    float dist; // [esp+Ch] [ebp-4Ch]
    float localmins[3]; // [esp+10h] [ebp-48h] BYREF
    unsigned int type; // [esp+1Ch] [ebp-3Ch]
    int side; // [esp+20h] [ebp-38h]
    cplane_s *plane; // [esp+24h] [ebp-34h]
    int cellIndex; // [esp+28h] [ebp-30h]
    mnode_t *leftNode; // [esp+30h] [ebp-28h]
    float mins2[3]; // [esp+34h] [ebp-24h] BYREF
    int cellCount; // [esp+40h] [ebp-18h]
    float maxs2[3]; // [esp+44h] [ebp-14h] BYREF
    mnode_t *rightNode; // [esp+50h] [ebp-8h]
    int planeIndex; // [esp+54h] [ebp-4h]

    cellCount = rgp.world->dpvsPlanes.cellCount + 1;
    mins2[0] = *mins;
    mins2[1] = mins[1];
    mins2[2] = mins[2];
    maxs2[0] = *maxs;
    maxs2[1] = maxs[1];
    maxs2[2] = maxs[2];
    while (1)
    {
        cellIndex = node->cellIndex;
        planeIndex = cellIndex - cellCount;
        if (cellIndex - cellCount < 0)
            break;
        plane = &rgp.world->dpvsPlanes.planes[planeIndex];
        side = BoxOnPlaneSide(mins2, maxs2, plane);
        if (side == 3)
        {
            type = plane->type;
            rightNode = (mnode_t *)((char *)node + 2 * node->rightChildOffset);
            if (type >= 3)
            {
                leftNode = node + 1;
                if (R_BoundsInCell_r(node + 1, findCellIndex, mins2, maxs2))
                    return 1;
            }
            else
            {
                dist = plane->dist;
                localmins[0] = mins2[0];
                localmins[1] = mins2[1];
                localmins[2] = mins2[2];
                localmins[type] = dist;
                localmaxs[0] = maxs2[0];
                localmaxs[1] = maxs2[1];
                localmaxs[2] = maxs2[2];
                localmaxs[type] = dist;
                if (BoxOnPlaneSide(localmins, maxs2, plane) != 1)
                    MyAssertHandler(
                        ".\\r_dpvs_entity.cpp",
                        128,
                        0,
                        "%s",
                        "BoxOnPlaneSide( localmins, maxs2, plane ) == BOXSIDE_FRONT");
                if (maxs2[type] > (double)dist)
                {
                    leftNode = node + 1;
                    if (R_BoundsInCell_r(node + 1, findCellIndex, localmins, maxs2))
                        return 1;
                }
                maxs2[0] = localmaxs[0];
                maxs2[1] = localmaxs[1];
                maxs2[2] = localmaxs[2];
            }
            node = rightNode;
        }
        else
        {
            iassert( (side == BOXSIDE_FRONT) || (side == BOXSIDE_BACK) );
            node = (mnode_t *)((char *)node + ((side - 1) * (node->rightChildOffset - 2)) * 2 + 4);
        }
    }
    return cellIndex && cellIndex - 1 == findCellIndex;
}

