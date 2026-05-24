#include "r_model_pose.h"
#include <xanim/dobj_utils.h>
#include "r_dobj_skin.h"
#include <universal/profile.h>
#include "r_dpvs.h"

#ifdef KISAK_MP
#include <cgame_mp/cg_local_mp.h>
#elif KISAK_SP
#include <cgame/cg_pose.h>
#endif

// LWSS: this function basically determines the visibility (dormancy) of Entities in the worldspace. Bodies will disappear in the edges of your FOV if you fk it up. Mounted machine guns as well. Edit with care I reverted this file lol
DObjAnimMat *R_UpdateSceneEntBounds(
    GfxSceneEntity *sceneEnt,
    GfxSceneEntity **pLocalSceneEnt,
    const DObj_s **pObj,
    int waitForCullState)
{
    float *maxs; // [esp+18h] [ebp-33Ch]
    float *mins; // [esp+1Ch] [ebp-338h]
    int v8; // [esp+40h] [ebp-314h]
    int v9; // [esp+40h] [ebp-314h]
    int v10; // [esp+40h] [ebp-314h]
    int v11; // [esp+40h] [ebp-314h]
    int v12; // [esp+40h] [ebp-314h]
    int v13; // [esp+40h] [ebp-314h]
    int v14; // [esp+40h] [ebp-314h]
    int v15; // [esp+40h] [ebp-314h]
    int v16; // [esp+40h] [ebp-314h]
    float v17; // [esp+44h] [ebp-310h]
    float v18; // [esp+44h] [ebp-310h]
    float v19; // [esp+44h] [ebp-310h]
    float v20; // [esp+44h] [ebp-310h]
    float v21; // [esp+44h] [ebp-310h]
    float v22; // [esp+44h] [ebp-310h]
    float v23; // [esp+44h] [ebp-310h]
    float v24; // [esp+44h] [ebp-310h]
    float v25; // [esp+44h] [ebp-310h]
    float v26; // [esp+48h] [ebp-30Ch]
    float v27; // [esp+48h] [ebp-30Ch]
    float v28; // [esp+48h] [ebp-30Ch]
    float v29; // [esp+48h] [ebp-30Ch]
    float v30; // [esp+48h] [ebp-30Ch]
    float v31; // [esp+48h] [ebp-30Ch]
    float v32; // [esp+48h] [ebp-30Ch]
    float v33; // [esp+48h] [ebp-30Ch]
    float v34; // [esp+48h] [ebp-30Ch]
    XBoneInfo *v35; // [esp+4Ch] [ebp-308h]
    float boneInfo; // [esp+58h] [ebp-2FCh]
    float v37; // [esp+5Ch] [ebp-2F8h]
    float v38; // [esp+60h] [ebp-2F4h]
    DObjSkelMat boneAxis; // [esp+64h] [ebp-2F0h] BYREF
    float zw; // [esp+A4h] [ebp-2B0h]
    float zz; // [esp+A8h] [ebp-2ACh]
    float yw; // [esp+ACh] [ebp-2A8h]
    float yz; // [esp+B0h] [ebp-2A4h]
    float yy; // [esp+B4h] [ebp-2A0h]
    float xw; // [esp+B8h] [ebp-29Ch]
    float xz; // [esp+BCh] [ebp-298h]
    float xy; // [esp+C0h] [ebp-294h]
    float xx; // [esp+C4h] [ebp-290h]
    float v49[3]; // [esp+C8h] [ebp-28Ch] BYREF
    float transWeight; // [esp+D4h] [ebp-280h]
    float v51; // [esp+D8h] [ebp-27Ch]
    float v52; // [esp+DCh] [ebp-278h]
    float v53; // [esp+E0h] [ebp-274h]
    float v54; // [esp+E4h] [ebp-270h]
    DObjAnimMat *mat; // [esp+E8h] [ebp-26Ch]
    int boneIndex; // [esp+ECh] [ebp-268h]
    uint32_t animPartBit; // [esp+F0h] [ebp-264h]
    int boneCount; // [esp+F4h] [ebp-260h]
    XBoneInfo *boneInfoArray[128]; // [esp+F8h] [ebp-25Ch] BYREF
    float4 minWorld; // [esp+300h] [ebp-54h]
    float4 maxWorld; // [esp+310h] [ebp-44h] BYREF
    DObjAnimMat *boneMatrix; // [esp+320h] [ebp-34h]
    int surfCount; // [esp+324h] [ebp-30h]
    int partBits[4]; // [esp+328h] [ebp-2Ch] BYREF
    const DObj_s *obj; // [esp+338h] [ebp-1Ch]
    GfxSceneEntity *localSceneEnt; // [esp+33Ch] [ebp-18h]
    uint32_t state; // [esp+340h] [ebp-14h]

    if (InterlockedCompareExchange((volatile LONG *)&sceneEnt->cull, 1, 0))
    {
        *pLocalSceneEnt = 0;
        if (waitForCullState)
        {
            do
            {
                state = sceneEnt->cull.state;
                iassert(state >= CULL_STATE_BOUNDED_PENDING);
            } while (state == CULL_STATE_BOUNDED_PENDING);
            if (state == CULL_STATE_DONE)
            {
                return 0;
            }
            else
            {
                localSceneEnt = sceneEnt;
                *pLocalSceneEnt = sceneEnt;
                obj = localSceneEnt->obj;
                *pObj = obj;
                iassert(obj);
                return I_dmaGetDObjSkel(obj);
            }
        }
        else
        {
            return 0;
        }
    }
    else
    {
        localSceneEnt = sceneEnt;
        *pLocalSceneEnt = sceneEnt;
        iassert(localSceneEnt->obj);
        obj = localSceneEnt->obj;
        *pObj = obj;
        iassert(obj);
        DObjGetSurfaceData(
            obj,
            localSceneEnt->placement.base.origin,
            localSceneEnt->placement.scale,
            localSceneEnt->cull.lods);
        if (useFastFile->current.enabled || !DObjBad(obj))
        {
            surfCount = DObjGetSurfaces(obj, partBits, localSceneEnt->cull.lods);
            if (surfCount && (boneMatrix = R_DObjCalcPose(localSceneEnt, obj, partBits)) != 0)
            {
                iassert(DObjSkelAreBonesUpToDate(obj, partBits));

                minWorld.v[0] = 131072.0;
                minWorld.v[1] = 131072.0;
                minWorld.v[2] = 131072.0;
                minWorld.v[3] = 0.0;

                maxWorld.v[0] = -131072.0;
                maxWorld.v[1] = -131072.0;
                maxWorld.v[2] = -131072.0;
                maxWorld.v[3] = 0.0;

                DObjGetBoneInfo(obj, boneInfoArray);
                boneCount = DObjNumBones(obj);
                animPartBit = 0x80000000;
                boneIndex = 0;

                while (boneIndex < boneCount)
                {
                    if ((animPartBit & partBits[boneIndex >> 5]) != 0)
                    {
                        mat = &boneMatrix[boneIndex];

                        iassert(!IS_NAN((mat->quat)[0]) && !IS_NAN((mat->quat)[1]) && !IS_NAN((mat->quat)[2]) && !IS_NAN((mat->quat)[3]));
                        iassert(!IS_NAN(mat->transWeight));

                        Vec3Scale(mat->quat, mat->transWeight, v49);
                        xx = v49[0] * mat->quat[0];
                        xy = v49[0] * mat->quat[1];
                        xz = v49[0] * mat->quat[2];
                        xw = v49[0] * mat->quat[3];
                        yy = v49[1] * mat->quat[1];
                        yz = v49[1] * mat->quat[2];
                        yw = v49[1] * mat->quat[3];
                        zz = v49[2] * mat->quat[2];
                        zw = v49[2] * mat->quat[3];
                        boneInfo = 1.0 - (yy + zz);
                        v37 = xy + zw;
                        v38 = xz - yw;
                        boneAxis.axis[0][1] = xy - zw;
                        boneAxis.axis[0][2] = 1.0 - (xx + zz);
                        boneAxis.axis[0][3] = yz + xw;
                        boneAxis.axis[1][1] = xz + yw;
                        boneAxis.axis[1][2] = yz - xw;
                        boneAxis.axis[1][3] = 1.0 - (xx + yy);
                        boneAxis.axis[2][1] = mat->trans[0];
                        boneAxis.axis[2][2] = mat->trans[1];
                        boneAxis.axis[2][3] = mat->trans[2];
                        boneAxis.origin[0] = 1.0;

                        Vec3Add(
                            &boneAxis.axis[2][1],
                            scene.def.viewOffset,
                            &boneAxis.axis[2][1]);
                        v35 = boneInfoArray[boneIndex];
                        v8 = boneInfo >= 0.0 ? 0 : 12;
                        v26 = *(float *)((char *)v35->bounds[0] + v8) * boneInfo + boneAxis.axis[2][1];
                        v17 = *(float *)((char *)v35->bounds[1] - v8) * boneInfo + boneAxis.axis[2][1];
                        v9 = boneAxis.axis[0][1] >= 0.0 ? 0 : 12;
                        v27 = *(float *)((char *)&v35->bounds[0][1] + v9) * boneAxis.axis[0][1] + v26;
                        v18 = *(float *)((char *)&v35->bounds[1][1] - v9) * boneAxis.axis[0][1] + v17;
                        v10 = boneAxis.axis[1][1] >= 0.0 ? 0 : 12;
                        v28 = *(float *)((char *)&v35->bounds[0][2] + v10) * boneAxis.axis[1][1] + v27;
                        v19 = *(float *)((char *)&v35->bounds[1][2] - v10) * boneAxis.axis[1][1] + v18;
                        if (v28 < (double)minWorld.v[0])
                            minWorld.v[0] = v28;
                        if (v19 > (double)maxWorld.v[0])
                            maxWorld.v[0] = v19;
                        v11 = v37 >= 0.0 ? 0 : 12;
                        v29 = *(float *)((char *)v35->bounds[0] + v11) * v37 + boneAxis.axis[2][2];
                        v20 = *(float *)((char *)v35->bounds[1] - v11) * v37 + boneAxis.axis[2][2];
                        v12 = boneAxis.axis[0][2] >= 0.0 ? 0 : 12;
                        v30 = *(float *)((char *)&v35->bounds[0][1] + v12) * boneAxis.axis[0][2] + v29;
                        v21 = *(float *)((char *)&v35->bounds[1][1] - v12) * boneAxis.axis[0][2] + v20;
                        v13 = boneAxis.axis[1][2] >= 0.0 ? 0 : 12;
                        v31 = *(float *)((char *)&v35->bounds[0][2] + v13) * boneAxis.axis[1][2] + v30;
                        v22 = *(float *)((char *)&v35->bounds[1][2] - v13) * boneAxis.axis[1][2] + v21;
                        if (v31 < (double)minWorld.v[1])
                            minWorld.v[1] = v31;
                        if (v22 > (double)maxWorld.v[1])
                            maxWorld.v[1] = v22;
                        v14 = v38 >= 0.0 ? 0 : 12;
                        v32 = *(float *)((char *)v35->bounds[0] + v14) * v38 + boneAxis.axis[2][3];
                        v23 = *(float *)((char *)v35->bounds[1] - v14) * v38 + boneAxis.axis[2][3];
                        v15 = boneAxis.axis[0][3] >= 0.0 ? 0 : 0xC;
                        v33 = *(float *)((char *)&v35->bounds[0][1] + v15) * boneAxis.axis[0][3] + v32;
                        v24 = *(float *)((char *)&v35->bounds[1][1] - v15) * boneAxis.axis[0][3] + v23;
                        v16 = boneAxis.axis[1][3] >= 0.0 ? 0 : 0xC;
                        v34 = *(float *)((char *)&v35->bounds[0][2] + v16) * boneAxis.axis[1][3] + v33;
                        v25 = *(float *)((char *)&v35->bounds[1][2] - v16) * boneAxis.axis[1][3] + v24;
                        if (v34 < (double)minWorld.v[2])
                            minWorld.v[2] = v34;
                        if (v25 > (double)maxWorld.v[2])
                            maxWorld.v[2] = v25;
                    }
                    ++boneIndex;
                    animPartBit = (animPartBit << 31) | (animPartBit >> 1);
                }

                localSceneEnt->cull.mins[0] = minWorld.v[0];
                localSceneEnt->cull.mins[1] = minWorld.v[1];
                localSceneEnt->cull.mins[2] = minWorld.v[2];

                localSceneEnt->cull.maxs[0] = maxWorld.v[0];
                localSceneEnt->cull.maxs[1] = maxWorld.v[1];
                localSceneEnt->cull.maxs[2] = maxWorld.v[2];

                iassert(localSceneEnt->cull.state == CULL_STATE_BOUNDED_PENDING);

                localSceneEnt->cull.state = CULL_STATE_BOUNDED;
                return boneMatrix;
            }
            else
            {
                R_SetNoDraw(sceneEnt);
                return 0;
            }
        }
        else
        {
            R_SetNoDraw(sceneEnt);
            return 0;
        }
    }
}

DObjAnimMat *__cdecl R_DObjCalcPose(const GfxSceneEntity *sceneEnt, const DObj_s *obj, int *partBits)
{
    DObjAnimMat *boneMatrix;
    int completePartBits[4];

    iassert(sceneEnt);
    iassert(obj);

    completePartBits[0] = partBits[0];
    completePartBits[1] = partBits[1];
    completePartBits[2] = partBits[2];
    completePartBits[3] = partBits[3];

    DObjLock((DObj_s*)obj);
    {
        PROF_SCOPED("R_DObjCalcPose");
        boneMatrix = CG_DObjCalcPose(sceneEnt->info.pose, obj, completePartBits);
    }
    DObjUnlock((DObj_s *)obj);

    return boneMatrix;
}

void __cdecl R_SetNoDraw(GfxSceneEntity *sceneEnt)
{
    if (sceneEnt->cull.state != 1)
        MyAssertHandler(
            ".\\r_model_pose.cpp",
            68,
            0,
            "%s\n\t(sceneEnt->cull.state) = %i",
            "(sceneEnt->cull.state == CULL_STATE_BOUNDED_PENDING)",
            sceneEnt->cull.state);
    sceneEnt->cull.state = 4;
}

void __cdecl R_UpdateGfxEntityBoundsCmd(GfxSceneEntity **data)
{
    const DObj_s *obj; // [esp+0h] [ebp-10h] BYREF
    GfxSceneEntity *localSceneEnt; // [esp+4h] [ebp-Ch] BYREF
    GfxSceneEntity *sceneEnt; // [esp+8h] [ebp-8h]
    GfxSceneEntity **pSceneEnt; // [esp+Ch] [ebp-4h]

    iassert( data );
    pSceneEnt = data;
    sceneEnt = *data;
    if (R_UpdateSceneEntBounds(sceneEnt, &localSceneEnt, &obj, 0))
    {
        iassert( localSceneEnt );
    }
}

