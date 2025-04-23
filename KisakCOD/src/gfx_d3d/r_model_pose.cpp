#include "r_model_pose.h"
#include <xanim/dobj_utils.h>
#include "r_dobj_skin.h"
#include <cgame_mp/cg_local_mp.h>


DObjAnimMat *__fastcall R_UpdateSceneEntBounds(
    GfxSceneEntity *sceneEnt,
    GfxSceneEntity **pLocalSceneEnt,
    const DObj_s **pObj,
    int waitForCullState)
{
    int v6; // [esp+18h] [ebp-33Ch]
    int v7; // [esp+1Ch] [ebp-338h]
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
    int v35; // [esp+4Ch] [ebp-308h]
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
    float v49; // [esp+C8h] [ebp-28Ch] BYREF
    float v50; // [esp+CCh] [ebp-288h]
    float v51; // [esp+D0h] [ebp-284h]
    float v52; // [esp+D4h] [ebp-280h]
    float v53; // [esp+D8h] [ebp-27Ch]
    float v54; // [esp+DCh] [ebp-278h]
    float v55; // [esp+E0h] [ebp-274h]
    float v56; // [esp+E4h] [ebp-270h]
    const float *v57; // [esp+E8h] [ebp-26Ch]
    int v58; // [esp+ECh] [ebp-268h]
    unsigned int v59; // [esp+F0h] [ebp-264h]
    DObjAnimMat *bone; // [esp+F4h] [ebp-260h]
    int boneIndex[130]; // [esp+F8h] [ebp-25Ch] BYREF
    float boneInfoArray_508; // [esp+300h] [ebp-54h]
    float v63; // [esp+304h] [ebp-50h]
    float v64; // [esp+308h] [ebp-4Ch]
    float4 bounds[2]; // [esp+30Ch] [ebp-48h] BYREF
    int partBits[4]; // [esp+334h] [ebp-20h]
    GfxSceneEntity *localSceneEnt; // [esp+348h] [ebp-Ch]
    unsigned int state; // [esp+34Ch] [ebp-8h]
    unsigned int retaddr; // [esp+354h] [ebp+0h]

    //localSceneEnt = a1;
    state = retaddr;
    if (InterlockedCompareExchange(&sceneEnt->cull.state, 1, 0))
    {
        *pLocalSceneEnt = 0;
        if (waitForCullState)
        {
            do
            {
                partBits[3] = sceneEnt->cull.state;
                if (!partBits[3])
                    MyAssertHandler(
                        ".\\r_model_pose.cpp",
                        258,
                        0,
                        "%s\n\t(state) = %i",
                        "(state >= CULL_STATE_BOUNDED_PENDING)",
                        0);
            } while (partBits[3] == 1);
            if (partBits[3] == 4)
            {
                return 0;
            }
            else
            {
                partBits[2] = (int)sceneEnt;
                *pLocalSceneEnt = sceneEnt;
                partBits[1] = *(unsigned int *)(partBits[2] + 112);
                *pObj = (const DObj_s *)partBits[1];
                if (!partBits[1])
                    MyAssertHandler(".\\r_model_pose.cpp", 269, 0, "%s", "obj");
                return I_dmaGetDObjSkel((const DObj_s *)partBits[1]);
            }
        }
        else
        {
            return 0;
        }
    }
    else
    {
        partBits[2] = (int)sceneEnt;
        *pLocalSceneEnt = sceneEnt;
        if (!*(unsigned int *)(partBits[2] + 112))
            MyAssertHandler(".\\r_model_pose.cpp", 282, 0, "%s", "localSceneEnt->obj");
        partBits[1] = *(unsigned int *)(partBits[2] + 112);
        *pObj = (const DObj_s *)partBits[1];
        if (!partBits[1])
            MyAssertHandler(".\\r_model_pose.cpp", 285, 0, "%s", "obj");
        DObjGetSurfaceData(
            (const DObj_s *)partBits[1],
            (const float *)(partBits[2] + 28),
            *(float *)(partBits[2] + 40),
            (char *)(partBits[2] + 72));
        if (useFastFile->current.enabled || !DObjBad((const DObj_s *)partBits[1]))
        {
            bounds[1].u[2] = DObjGetSurfaces(
                (const DObj_s *)partBits[1],
                (int *)&bounds[1].u[3],
                (const char *)(partBits[2] + 72));
            if (bounds[1].u[2]
                && (bounds[1].u[1] = (unsigned int)R_DObjCalcPose(
                    (const GfxSceneEntity *)partBits[2],
                    (const DObj_s *)partBits[1],
                    (int *)&bounds[1].u[3])) != 0)
            {
                if (!DObjSkelAreBonesUpToDate((const DObj_s *)partBits[1], (int *)&bounds[1].u[3]))
                    MyAssertHandler(".\\r_model_pose.cpp", 315, 0, "%s", "DObjSkelAreBonesUpToDate( obj, partBits )");
                boneInfoArray_508 = 131072.0;
                v63 = 131072.0;
                v64 = 131072.0;
                bounds[0].v[0] = 0.0;
                boneIndex[129] = (int)&bounds[0].u[1];
                bounds[0].v[1] = -131072.0;
                bounds[0].v[2] = -131072.0;
                bounds[0].v[3] = -131072.0;
                bounds[1].v[0] = 0.0;
                DObjGetBoneInfo((const DObj_s *)partBits[1], (XBoneInfo **)boneIndex);
                bone = (DObjAnimMat *)DObjNumBones((const DObj_s *)partBits[1]);
                v59 = 0x80000000;
                v58 = 0;
                while (v58 < (int)bone)
                {
                    if ((v59 & bounds[1].u[(v58 >> 5) + 3]) != 0)
                    {
                        v57 = (const float *)(bounds[1].u[1] + 32 * v58);
                        v56 = *v57;
                        if ((LODWORD(v56) & 0x7F800000) == 0x7F800000
                            || (v55 = v57[1], (LODWORD(v55) & 0x7F800000) == 0x7F800000)
                            || (v54 = v57[2], (LODWORD(v54) & 0x7F800000) == 0x7F800000)
                            || (v53 = v57[3], (LODWORD(v53) & 0x7F800000) == 0x7F800000))
                        {
                            MyAssertHandler(
                                "c:\\trees\\cod3\\src\\renderer\\../xanim/xanim_public.h",
                                473,
                                0,
                                "%s",
                                "!IS_NAN((mat->quat)[0]) && !IS_NAN((mat->quat)[1]) && !IS_NAN((mat->quat)[2]) && !IS_NAN((mat->quat)[3])");
                        }
                        v52 = v57[7];
                        if ((LODWORD(v52) & 0x7F800000) == 0x7F800000)
                            MyAssertHandler(
                                "c:\\trees\\cod3\\src\\renderer\\../xanim/xanim_public.h",
                                474,
                                0,
                                "%s",
                                "!IS_NAN(mat->transWeight)");
                        Vec3Scale(v57, v57[7], &v49);
                        v48 = v49 * *v57;
                        v47 = v49 * v57[1];
                        v46 = v49 * v57[2];
                        v45 = v49 * v57[3];
                        v44 = v50 * v57[1];
                        v43 = v50 * v57[2];
                        v42 = v50 * v57[3];
                        v41 = v51 * v57[2];
                        v40 = v51 * v57[3];
                        boneInfo = 1.0 - (v44 + v41);
                        v37 = v47 + v40;
                        v38 = v46 - v42;
                        boneAxis.axis[0][1] = v47 - v40;
                        boneAxis.axis[0][2] = 1.0 - (v48 + v41);
                        boneAxis.axis[0][3] = v43 + v45;
                        boneAxis.axis[1][1] = v46 + v42;
                        boneAxis.axis[1][2] = v43 - v45;
                        boneAxis.axis[1][3] = 1.0 - (v48 + v44);
                        boneAxis.axis[2][1] = v57[4];
                        boneAxis.axis[2][2] = v57[5];
                        boneAxis.axis[2][3] = v57[6];
                        boneAxis.origin[0] = 1.0;
                        Vec3Add(&boneAxis.axis[2][1], scene.def.viewOffset, &boneAxis.axis[2][1]);
                        v35 = boneIndex[v58];
                        v8 = boneInfo >= 0.0 ? 0 : 0xC;
                        v26 = *(float *)(v8 + v35) * boneInfo + boneAxis.axis[2][1];
                        v17 = *(float *)(v35 + 12 - v8) * boneInfo + boneAxis.axis[2][1];
                        v9 = boneAxis.axis[0][1] >= 0.0 ? 0 : 0xC;
                        v27 = *(float *)(v9 + v35 + 4) * boneAxis.axis[0][1] + v26;
                        v18 = *(float *)(v35 + 16 - v9) * boneAxis.axis[0][1] + v17;
                        v10 = boneAxis.axis[1][1] >= 0.0 ? 0 : 0xC;
                        v28 = *(float *)(v10 + v35 + 8) * boneAxis.axis[1][1] + v27;
                        v19 = *(float *)(v35 + 20 - v10) * boneAxis.axis[1][1] + v18;
                        if (v28 < (double)boneInfoArray_508)
                            boneInfoArray_508 = v28;
                        if (v19 > (double)bounds[0].v[1])
                            bounds[0].v[1] = v19;
                        v11 = v37 >= 0.0 ? 0 : 0xC;
                        v29 = *(float *)(v11 + v35) * v37 + boneAxis.axis[2][2];
                        v20 = *(float *)(v35 + 12 - v11) * v37 + boneAxis.axis[2][2];
                        v12 = boneAxis.axis[0][2] >= 0.0 ? 0 : 0xC;
                        v30 = *(float *)(v12 + v35 + 4) * boneAxis.axis[0][2] + v29;
                        v21 = *(float *)(v35 + 16 - v12) * boneAxis.axis[0][2] + v20;
                        v13 = boneAxis.axis[1][2] >= 0.0 ? 0 : 0xC;
                        v31 = *(float *)(v13 + v35 + 8) * boneAxis.axis[1][2] + v30;
                        v22 = *(float *)(v35 + 20 - v13) * boneAxis.axis[1][2] + v21;
                        if (v31 < (double)v63)
                            v63 = v31;
                        if (v22 > (double)bounds[0].v[2])
                            bounds[0].v[2] = v22;
                        v14 = v38 >= 0.0 ? 0 : 0xC;
                        v32 = *(float *)(v14 + v35) * v38 + boneAxis.axis[2][3];
                        v23 = *(float *)(v35 + 12 - v14) * v38 + boneAxis.axis[2][3];
                        v15 = boneAxis.axis[0][3] >= 0.0 ? 0 : 0xC;
                        v33 = *(float *)(v15 + v35 + 4) * boneAxis.axis[0][3] + v32;
                        v24 = *(float *)(v35 + 16 - v15) * boneAxis.axis[0][3] + v23;
                        v16 = boneAxis.axis[1][3] >= 0.0 ? 0 : 0xC;
                        v34 = *(float *)(v16 + v35 + 8) * boneAxis.axis[1][3] + v33;
                        v25 = *(float *)(v35 + 20 - v16) * boneAxis.axis[1][3] + v24;
                        if (v34 < (double)v64)
                            v64 = v34;
                        if (v25 > (double)bounds[0].v[3])
                            bounds[0].v[3] = v25;
                    }
                    ++v58;
                    v59 = (v59 << 31) | (v59 >> 1);
                }
                v7 = partBits[2] + 48;
                *(float *)(partBits[2] + 48) = boneInfoArray_508;
                *(float *)(v7 + 4) = v63;
                *(float *)(v7 + 8) = v64;
                v6 = partBits[2] + 60;
                *(float *)(partBits[2] + 60) = bounds[0].v[1];
                *(float *)(v6 + 4) = bounds[0].v[2];
                *(float *)(v6 + 8) = bounds[0].v[3];
                if (*(unsigned int *)(partBits[2] + 44) != 1)
                    MyAssertHandler(
                        ".\\r_model_pose.cpp",
                        339,
                        0,
                        "%s\n\t(localSceneEnt->cull.state) = %i",
                        "(localSceneEnt->cull.state == CULL_STATE_BOUNDED_PENDING)",
                        *(unsigned int *)(partBits[2] + 44));
                *(unsigned int *)(partBits[2] + 44) = 2;
                return (DObjAnimMat *)bounds[1].u[1];
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
    //Profile_Begin(318);
    boneMatrix = CG_DObjCalcPose(sceneEnt->info.pose, obj, completePartBits);
    //Profile_EndInternal(0);
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

