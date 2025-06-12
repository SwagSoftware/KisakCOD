#include "r_model_pose.h"
#include <xanim/dobj_utils.h>
#include "r_dobj_skin.h"
#include <cgame_mp/cg_local_mp.h>
#include <universal/profile.h>

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
    float v40; // [esp+A4h] [ebp-2B0h]
    float v41; // [esp+A8h] [ebp-2ACh]
    float v42; // [esp+ACh] [ebp-2A8h]
    float v43; // [esp+B0h] [ebp-2A4h]
    float v44; // [esp+B4h] [ebp-2A0h]
    float v45; // [esp+B8h] [ebp-29Ch]
    float v46; // [esp+BCh] [ebp-298h]
    float v47; // [esp+C0h] [ebp-294h]
    float v48; // [esp+C4h] [ebp-290h]
    float v49[3]; // [esp+C8h] [ebp-28Ch] BYREF
    float transWeight; // [esp+D4h] [ebp-280h]
    float v51; // [esp+D8h] [ebp-27Ch]
    float v52; // [esp+DCh] [ebp-278h]
    float v53; // [esp+E0h] [ebp-274h]
    float v54; // [esp+E4h] [ebp-270h]
    DObjAnimMat *bone; // [esp+E8h] [ebp-26Ch]
    int boneIndex; // [esp+ECh] [ebp-268h]
    unsigned int animPartBit; // [esp+F0h] [ebp-264h]
    int boneCount; // [esp+F4h] [ebp-260h]
    XBoneInfo *boneInfoArray[128]; // [esp+F8h] [ebp-25Ch] BYREF
    unsigned int *v60; // [esp+2FCh] [ebp-58h]
    float boneInfoArray_508; // [esp+300h] [ebp-54h]
    float bounds_4; // [esp+304h] [ebp-50h]
    float bounds_8; // [esp+308h] [ebp-4Ch]
    float4 bounds; // [esp+30Ch] [ebp-48h] BYREF
    float partBits; // [esp+31Ch] [ebp-38h]
    DObjAnimMat *partBits_4; // [esp+320h] [ebp-34h]
    int partBits_8; // [esp+324h] [ebp-30h]
    int partBits_12[4]; // [esp+328h] [ebp-2Ch] BYREF
    const DObj_s *obj; // [esp+338h] [ebp-1Ch]
    GfxSceneEntity *localSceneEnt; // [esp+33Ch] [ebp-18h]
    unsigned int state; // [esp+340h] [ebp-14h]
    //_UNKNOWN *cuck; // [esp+348h] [ebp-Ch]
    GfxSceneEntity *sceneEnta; // [esp+34Ch] [ebp-8h]
    const DObj_s **pObja; // [esp+354h] [ebp+0h]

    //cuck = a1;
    //sceneEnta = pObja;
    if (InterlockedCompareExchange(&sceneEnt->cull.state, 1, 0))
    {
        *pLocalSceneEnt = 0;
        if (waitForCullState)
        {
            do
            {
                state = sceneEnt->cull.state;
                if (!state)
                    MyAssertHandler(
                        ".\\r_model_pose.cpp",
                        258,
                        0,
                        "%s\n\t(state) = %i",
                        "(state >= CULL_STATE_BOUNDED_PENDING)",
                        0);
            } while (state == 1);
            if (state == 4)
            {
                return 0;
            }
            else
            {
                localSceneEnt = sceneEnt;
                *pLocalSceneEnt = sceneEnt;
                obj = localSceneEnt->obj;
                *pObj = obj;
                if (!obj)
                    MyAssertHandler(".\\r_model_pose.cpp", 269, 0, "%s", "obj");
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
        if (!localSceneEnt->obj)
            MyAssertHandler(".\\r_model_pose.cpp", 282, 0, "%s", "localSceneEnt->obj");
        obj = localSceneEnt->obj;
        *pObj = obj;
        if (!obj)
            MyAssertHandler(".\\r_model_pose.cpp", 285, 0, "%s", "obj");
        DObjGetSurfaceData(
            obj,
            localSceneEnt->placement.base.origin,
            localSceneEnt->placement.scale,
            localSceneEnt->cull.lods);
        if (useFastFile->current.enabled || !DObjBad(obj))
        {
            partBits_8 = DObjGetSurfaces(obj, partBits_12, localSceneEnt->cull.lods);
            if (partBits_8 && (partBits_4 = R_DObjCalcPose(localSceneEnt, obj, partBits_12)) != 0)
            {
                if (!DObjSkelAreBonesUpToDate(obj, partBits_12))
                    MyAssertHandler(".\\r_model_pose.cpp", 315, 0, "%s", "DObjSkelAreBonesUpToDate( obj, partBits )");
                boneInfoArray_508 = 131072.0;
                bounds_4 = 131072.0;
                bounds_8 = 131072.0;
                bounds.v[0] = 0.0;
                v60 = &bounds.u[1];
                bounds.v[1] = -131072.0;
                bounds.v[2] = -131072.0;
                bounds.v[3] = -131072.0;
                partBits = 0.0;
                DObjGetBoneInfo(obj, boneInfoArray);
                boneCount = DObjNumBones(obj);
                animPartBit = 0x80000000;
                boneIndex = 0;
                while (boneIndex < boneCount)
                {
                    if ((animPartBit & partBits_12[boneIndex >> 5]) != 0)
                    {
                        bone = &partBits_4[boneIndex];
                        v54 = bone->quat[0];
                        if ((LODWORD(v54) & 0x7F800000) == 0x7F800000
                            || (v53 = bone->quat[1], (LODWORD(v53) & 0x7F800000) == 0x7F800000)
                            || (v52 = bone->quat[2], (LODWORD(v52) & 0x7F800000) == 0x7F800000)
                            || (v51 = bone->quat[3], (LODWORD(v51) & 0x7F800000) == 0x7F800000))
                        {
                            MyAssertHandler(
                                "c:\\trees\\cod3\\src\\renderer\\../xanim/xanim_public.h",
                                473,
                                0,
                                "%s",
                                "!IS_NAN((mat->quat)[0]) && !IS_NAN((mat->quat)[1]) && !IS_NAN((mat->quat)[2]) && !IS_NAN((mat->quat)[3])");
                        }
                        transWeight = bone->transWeight;
                        if ((LODWORD(transWeight) & 0x7F800000) == 0x7F800000)
                            MyAssertHandler(
                                "c:\\trees\\cod3\\src\\renderer\\../xanim/xanim_public.h",
                                474,
                                0,
                                "%s",
                                "!IS_NAN(mat->transWeight)");
                        Vec3Scale(bone->quat, bone->transWeight, v49);
                        v48 = v49[0] * bone->quat[0];
                        v47 = v49[0] * bone->quat[1];
                        v46 = v49[0] * bone->quat[2];
                        v45 = v49[0] * bone->quat[3];
                        v44 = v49[1] * bone->quat[1];
                        v43 = v49[1] * bone->quat[2];
                        v42 = v49[1] * bone->quat[3];
                        v41 = v49[2] * bone->quat[2];
                        v40 = v49[2] * bone->quat[3];
                        boneInfo = 1.0 - (v44 + v41);
                        v37 = v47 + v40;
                        v38 = v46 - v42;
                        boneAxis.axis[0][1] = v47 - v40;
                        boneAxis.axis[0][2] = 1.0 - (v48 + v41);
                        boneAxis.axis[0][3] = v43 + v45;
                        boneAxis.axis[1][1] = v46 + v42;
                        boneAxis.axis[1][2] = v43 - v45;
                        boneAxis.axis[1][3] = 1.0 - (v48 + v44);
                        boneAxis.axis[2][1] = bone->trans[0];
                        boneAxis.axis[2][2] = bone->trans[1];
                        boneAxis.axis[2][3] = bone->trans[2];
                        boneAxis.origin[0] = 1.0;
                        Vec3Add(&boneAxis.axis[2][1], scene.def.viewOffset, &boneAxis.axis[2][1]);
                        v35 = boneInfoArray[boneIndex];
                        v8 = boneInfo >= 0.0 ? 0 : 0xC;
                        v26 = *(v35->bounds[0] + v8) * boneInfo + boneAxis.axis[2][1];
                        v17 = *(v35->bounds[1] - v8) * boneInfo + boneAxis.axis[2][1];
                        v9 = boneAxis.axis[0][1] >= 0.0 ? 0 : 0xC;
                        v27 = *(&v35->bounds[0][1] + v9) * boneAxis.axis[0][1] + v26;
                        v18 = *(&v35->bounds[1][1] - v9) * boneAxis.axis[0][1] + v17;
                        v10 = boneAxis.axis[1][1] >= 0.0 ? 0 : 0xC;
                        v28 = *(&v35->bounds[0][2] + v10) * boneAxis.axis[1][1] + v27;
                        v19 = *(&v35->bounds[1][2] - v10) * boneAxis.axis[1][1] + v18;
                        if (v28 < boneInfoArray_508)
                            boneInfoArray_508 = v28;
                        if (v19 > bounds.v[1])
                            bounds.v[1] = v19;
                        v11 = v37 >= 0.0 ? 0 : 0xC;
                        v29 = *(v35->bounds[0] + v11) * v37 + boneAxis.axis[2][2];
                        v20 = *(v35->bounds[1] - v11) * v37 + boneAxis.axis[2][2];
                        v12 = boneAxis.axis[0][2] >= 0.0 ? 0 : 0xC;
                        v30 = *(&v35->bounds[0][1] + v12) * boneAxis.axis[0][2] + v29;
                        v21 = *(&v35->bounds[1][1] - v12) * boneAxis.axis[0][2] + v20;
                        v13 = boneAxis.axis[1][2] >= 0.0 ? 0 : 0xC;
                        v31 = *(&v35->bounds[0][2] + v13) * boneAxis.axis[1][2] + v30;
                        v22 = *(&v35->bounds[1][2] - v13) * boneAxis.axis[1][2] + v21;
                        if (v31 < bounds_4)
                            bounds_4 = v31;
                        if (v22 > bounds.v[2])
                            bounds.v[2] = v22;
                        v14 = v38 >= 0.0 ? 0 : 0xC;
                        v32 = *(v35->bounds[0] + v14) * v38 + boneAxis.axis[2][3];
                        v23 = *(v35->bounds[1] - v14) * v38 + boneAxis.axis[2][3];
                        v15 = boneAxis.axis[0][3] >= 0.0 ? 0 : 0xC;
                        v33 = *(&v35->bounds[0][1] + v15) * boneAxis.axis[0][3] + v32;
                        v24 = *(&v35->bounds[1][1] - v15) * boneAxis.axis[0][3] + v23;
                        v16 = boneAxis.axis[1][3] >= 0.0 ? 0 : 0xC;
                        v34 = *(&v35->bounds[0][2] + v16) * boneAxis.axis[1][3] + v33;
                        v25 = *(&v35->bounds[1][2] - v16) * boneAxis.axis[1][3] + v24;
                        if (v34 < bounds_8)
                            bounds_8 = v34;
                        if (v25 > bounds.v[3])
                            bounds.v[3] = v25;
                    }
                    ++boneIndex;
                    animPartBit = (animPartBit << 31) | (animPartBit >> 1);
                }
                mins = localSceneEnt->cull.mins;
                localSceneEnt->cull.mins[0] = boneInfoArray_508;
                mins[1] = bounds_4;
                mins[2] = bounds_8;
                maxs = localSceneEnt->cull.maxs;
                localSceneEnt->cull.maxs[0] = bounds.v[1];
                maxs[1] = bounds.v[2];
                maxs[2] = bounds.v[3];
                if (localSceneEnt->cull.state != 1)
                    MyAssertHandler(
                        ".\\r_model_pose.cpp",
                        339,
                        0,
                        "%s\n\t(localSceneEnt->cull.state) = %i",
                        "(localSceneEnt->cull.state == CULL_STATE_BOUNDED_PENDING)",
                        localSceneEnt->cull.state);
                localSceneEnt->cull.state = 2;
                return partBits_4;
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
    DObjAnimMat *boneMatrix; // [esp+30h] [ebp-14h]
    int completePartBits[4]; // [esp+34h] [ebp-10h] BYREF

    if (!sceneEnt)
        MyAssertHandler(".\\r_model_pose.cpp", 35, 0, "%s", "sceneEnt");
    if (!obj)
        MyAssertHandler(".\\r_model_pose.cpp", 36, 0, "%s", "obj");
    completePartBits[0] = *partBits;
    completePartBits[1] = partBits[1];
    completePartBits[2] = partBits[2];
    completePartBits[3] = partBits[3];
    DObjLock((DObj_s*)obj);
    Profile_Begin(318);
    boneMatrix = CG_DObjCalcPose(sceneEnt->info.pose, obj, completePartBits);
    Profile_EndInternal(0);
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

    if (!data)
        MyAssertHandler(".\\r_model_pose.cpp", 365, 0, "%s", "data");
    pSceneEnt = data;
    sceneEnt = *data;
    if (R_UpdateSceneEntBounds(sceneEnt, &localSceneEnt, &obj, 0))
    {
        if (!localSceneEnt)
            MyAssertHandler(".\\r_model_pose.cpp", 372, 0, "%s", "localSceneEnt");
    }
}

