#ifndef KISAK_MP
#error This File is MultiPlayer Only
#endif

#include "cg_local_mp.h"
#include "cg_public_mp.h"
#include <xanim/dobj_utils.h>
#include <ragdoll/ragdoll.h>
#include <gfx_d3d/r_scene.h>
#include <universal/profile.h>


// KISAKTODO: this function is real bad
void CG_VehPoseControllers(const cpose_t *pose, const DObj_s *obj, int32_t *partBits)
{
    float v4[7]; // [esp-Ch] [ebp-1DCh] BYREF
    float v5; // [esp+10h] [ebp-1C0h]
    float v6; // [esp+14h] [ebp-1BCh]
    float v7; // [esp+18h] [ebp-1B8h]
    float v8; // [esp+1Ch] [ebp-1B4h]
    float v9; // [esp+20h] [ebp-1B0h]
    float v10; // [esp+24h] [ebp-1ACh]
    float v11; // [esp+28h] [ebp-1A8h]
    float v12; // [esp+2Ch] [ebp-1A4h]
    float dist; // [esp+30h] [ebp-1A0h]
    float v14; // [esp+34h] [ebp-19Ch]
    float v15; // [esp+38h] [ebp-198h]
    int32_t v16; // [esp+3Ch] [ebp-194h]
    float v17; // [esp+40h] [ebp-190h]
    float v18; // [esp+44h] [ebp-18Ch]
    float v19; // [esp+48h] [ebp-188h]
    float v20; // [esp+4Ch] [ebp-184h]
    float v21; // [esp+50h] [ebp-180h]
    float v22; // [esp+54h] [ebp-17Ch]
    float v23; // [esp+58h] [ebp-178h]
    float4 wheelPos; // [esp+5Ch] [ebp-174h]
    float4 trans; // [esp+6Ch] [ebp-164h]
    int32_t j; // [esp+7Ch] [ebp-154h]
    const DObjAnimMat *mtx; // [esp+80h] [ebp-150h]
    uint32_t boneIndex; // [esp+84h] [ebp-14Ch]
    const XModel *i; // [esp+88h] [ebp-148h]
    const DObjAnimMat *boneMtxList; // [esp+8Ch] [ebp-144h]
    uint32_t boneCount; // [esp+90h] [ebp-140h]
    XModel *model; // [esp+94h] [ebp-13Ch]
    float invAxis[16]; // [esp+98h] [ebp-138h]
    float4 axisW; // [esp+D8h] [ebp-F8h]
    float v35; // [esp+E8h] [ebp-E8h]
    float4 axisZ; // [esp+ECh] [ebp-E4h]
    float v37; // [esp+FCh] [ebp-D4h]
    float axis[16]; // [esp+100h] [ebp-D0h]
    float *p_tempAxis_24; // [esp+140h] [ebp-90h]
    float v40[9]; // [esp+144h] [ebp-8Ch] BYREF
    float tempAxis_24; // [esp+168h] [ebp-68h] BYREF
    float tempAxis_28; // [esp+16Ch] [ebp-64h]
    float tempAxis_32; // [esp+170h] [ebp-60h]
    float tempAxis_40; // [esp+178h] [ebp-58h]
    int32_t tempAxis_44; // [esp+17Ch] [ebp-54h]
    int32_t yaw; // [esp+180h] [ebp-50h]
    int32_t suspTravel; // [esp+184h] [ebp-4Ch]
    int32_t roll; // [esp+188h] [ebp-48h]
    int32_t pitch; // [esp+18Ch] [ebp-44h]
    float v50; // [esp+190h] [ebp-40h] BYREF
    float v51; // [esp+194h] [ebp-3Ch]
    float v52; // [esp+198h] [ebp-38h]
    float steerAngles[3]; // [esp+19Ch] [ebp-34h] BYREF
    float bodyAngles[3]; // [esp+1A8h] [ebp-28h] BYREF
    float barrelAngles[7]; // [esp+1B4h] [ebp-1Ch] BYREF
    float retaddr; // [esp+1D0h] [ebp+0h]

    //barrelAngles[4] = a1;
    //barrelAngles[5] = retaddr;
    if (!obj)
        MyAssertHandler(".\\cgame_mp\\cg_pose_mp.cpp", 70, 0, "%s", "obj");
    barrelAngles[0] = 0.0;
    barrelAngles[1] = 0.0;
    barrelAngles[2] = 0.0;
    bodyAngles[0] = 0.0;
    bodyAngles[1] = 0.0;
    bodyAngles[2] = 0.0;
    steerAngles[0] = 0.0;
    steerAngles[1] = 0.0;
    steerAngles[2] = 0.0;
    v50 = 0.0;
    v51 = 0.0;
    v52 = 0.0;
    pitch = pose->vehicle.pitch;
    steerAngles[0] = pitch * 0.0054931640625;
    roll = pose->vehicle.roll;
    steerAngles[2] = roll * 0.0054931640625;
    suspTravel = pose->vehicle.barrelPitch;
    bodyAngles[0] = suspTravel * 0.0054931640625;
    if (pose->eType == ET_HELICOPTER)
        bodyAngles[2] = pose->turret.barrelPitch;
    yaw = pose->vehicle.yaw;
    barrelAngles[1] = yaw * 0.0054931640625;
    tempAxis_44 = pose->vehicle.steerYaw;
    v51 = tempAxis_44 * 0.0054931640625;
    DObjSetLocalTag((DObj_s*)obj, partBits, pose->vehicle.tag_body, vec3_origin, steerAngles);
    DObjSetLocalTag((DObj_s*)obj, partBits, pose->vehicle.tag_turret, vec3_origin, barrelAngles);
    DObjSetLocalTag((DObj_s*)obj, partBits, pose->vehicle.tag_barrel, vec3_origin, bodyAngles);
    tempAxis_40 = pose->vehicle.time;
    AnglesToAxis(pose->angles, (float(*)[3])v40);
    p_tempAxis_24 = &tempAxis_24;
    //LODWORD(axis[15]) = pose->origin;
    tempAxis_24 = pose->origin[0];
    tempAxis_28 = pose->origin[1];
    tempAxis_32 = pose->origin[2];
    //LODWORD(axis[14]) = v40;
    axisZ.v[2] = v40[0];
    axisZ.v[3] = v40[1];
    v37 = v40[2];
    axis[0] = 0.0;
    axis[1] = v40[3];
    axis[2] = v40[4];
    axis[3] = v40[5];
    axis[4] = 0.0;
    axis[5] = v40[6];
    axis[6] = v40[7];
    axis[7] = v40[8];
    axis[8] = 0.0;
    axis[9] = tempAxis_24;
    axis[10] = tempAxis_28;
    axis[11] = tempAxis_32;
    axis[12] = 1.0;
    axisZ.u[1] = (uint32_t)&v40[6];
    axisZ.u[1] = v40[5];
    axisW.v[2] = v40[6];
    axisW.v[3] = v40[7];
    v35 = v40[8];
    axisZ.v[0] = 0.0;
    axisW.u[1] = (uint32_t) & tempAxis_24;
    invAxis[13] = tempAxis_24;
    invAxis[14] = tempAxis_28;
    invAxis[15] = tempAxis_32;
    axisW.v[0] = 0.0;
    boneMtxList = (const DObjAnimMat *)LODWORD(v40[0]);
    boneCount = LODWORD(v40[3]);
    model = (XModel*)LODWORD(v40[6]);
    invAxis[0] = 1.0;
    invAxis[1] = v40[1];
    invAxis[2] = v40[4];
    invAxis[3] = v40[7];
    invAxis[4] = 1.0;
    invAxis[5] = v40[2];
    invAxis[6] = v40[5];
    invAxis[7] = v40[8];
    invAxis[8] = 1.0;
    i = DObjGetModel(obj, 0);
    boneIndex = XModelNumBones(i);
    mtx = XModelGetBasePose(i);
    for (j = 0; j < 4; ++j)
    {
        trans.u[3] = pose->vehicle.wheelBoneIndex[j];
        if (trans.u[3] < 0xFE && DObjSetRotTransIndex((DObj_s*)obj, partBits, trans.u[3]))
        {
            trans.u[2] = (uint32_t)&mtx[trans.u[3]];
            trans.u[1] = trans.u[2] + 16;
            wheelPos.v[1] = *(float*)(trans.u[2] + 16);
            wheelPos.v[2] = *(float*)(trans.u[2] + 20);
            wheelPos.v[3] = *(float*)(trans.u[2] + 24);
            trans.v[0] = 0.0;
            v17 = wheelPos.v[1] * axisZ.v[2] + wheelPos.v[2] * axis[1] + wheelPos.v[3] * axis[5] + axis[9];
            v18 = wheelPos.v[1] * axisZ.v[3] + wheelPos.v[2] * axis[2] + wheelPos.v[3] * axis[6] + axis[10];
            v19 = wheelPos.v[1] * v37 + wheelPos.v[2] * axis[3] + wheelPos.v[3] * axis[7] + axis[11];
            v20 = wheelPos.v[1] * axis[0] + wheelPos.v[2] * axis[4] + wheelPos.v[3] * axis[8] + axis[12];
            v21 = v17;
            v22 = v18;
            v23 = v19;
            wheelPos.v[0] = v20;
            v16 = pose->vehicle.wheelFraction[j];
            v15 = v16 * 0.00001525902189314365;
            v14 = v15 * (tempAxis_40 + 40.0);
            dist = 40.0 - tempAxis_40;
            v12 = v14 - dist;
            if (v12 < 0.0)
                v11 = dist;
            else
                v11 = v14;
            v10 = v11;
            v21 = 40.0 * axisW.v[2] + v21;
            v22 = 40.0 * axisW.v[3] + v22;
            v23 = 40.0 * v35 + v23;
            v9 = -v11;
            v21 = v9 * axisW.v[2] + v21;
            v22 = v9 * axisW.v[3] + v22;
            v23 = v9 * v35 + v23;
            v21 = v21 - invAxis[13];
            v22 = v22 - invAxis[14];
            v23 = v23 - invAxis[15];
            v5 = v21 * *(float*)&boneMtxList + v22 * invAxis[1] + v23 * invAxis[5];
            v6 = v21 * *(float*)&boneCount + v22 * invAxis[2] + v23 * invAxis[6];
            v7 = v21 * *(float*)&model + v22 * invAxis[3] + v23 * invAxis[7];
            v8 = v21 * invAxis[0] + v22 * invAxis[4] + v23 * invAxis[8];
            v21 = v5 - wheelPos.v[1];
            v22 = v6 - wheelPos.v[2];
            v23 = v7 - wheelPos.v[3];
            wheelPos.v[0] = v8 - trans.v[0];
            v4[0] = v21;
            v4[1] = v22;
            v4[2] = v23;
            v4[3] = wheelPos.v[0];
            if (v51 == 0.0 || j > 1)
                DObjSetLocalTagInternal(obj, v4, 0, trans.u[3]);
            else
                DObjSetLocalTagInternal(obj, v4, &v50, trans.u[3]);
        }
    }
}

void __cdecl CG_DoControllers(const cpose_t *pose, const DObj_s *obj, int32_t *partBits)
{
    int32_t setPartBits[4]; // [esp+34h] [ebp-10h] BYREF

    PROF_SCOPED("CG_DoControllers");

    DObjGetSetBones(obj, setPartBits);
    switch (pose->eType)
    {
    case ET_PLAYER:
        CG_Player_DoControllers(pose, obj, partBits);
        break;
    case ET_MG42:
        CG_mg42_DoControllers(pose, obj, partBits);
        break;
    case ET_HELICOPTER:
    case ET_VEHICLE:
        CG_VehPoseControllers(pose, obj, partBits);
        break;
    default:
        break;
    }
    CG_DoBaseOriginController(pose, obj, setPartBits);
    if (pose->isRagdoll && (pose->ragdollHandle || pose->killcamRagdollHandle))
        Ragdoll_DoControllers(pose, (DObj_s*)obj, partBits);
}

void __cdecl CG_Player_DoControllers(const cpose_t *pose, const DObj_s *obj, int32_t *partBits)
{
    if (pose->fx.triggerTime)
        BG_Player_DoControllers(&pose->player, obj, partBits);
}

void __cdecl CG_mg42_DoControllers(const cpose_t *pose, const DObj_s *obj, int32_t *partBits)
{
    float angles[3]; // [esp+10h] [ebp-10h] BYREF
    const float *viewAngles; // [esp+1Ch] [ebp-4h]

    if (pose->turret.playerUsing)
    {
        viewAngles = pose->turret.viewAngles;
        angles[0] = AngleDelta(viewAngles[0], pose->angles[0]);
        angles[1] = AngleDelta(viewAngles[1], pose->angles[1]);
    }
    else
    {
        //angles[0] = pose->turret.$9D88A49AD898204B3D6E378457DD8419::angles.pitch;
        angles[0] = pose->turret.angles.pitch;
        //angles[1] = pose->turret.$9D88A49AD898204B3D6E378457DD8419::angles.yaw;
        angles[1] = pose->turret.angles.yaw;
    }
    angles[2] = 0.0;
    DObjSetControlTagAngles((DObj_s*)obj, partBits, pose->turret.tag_aim, angles);
    DObjSetControlTagAngles((DObj_s *)obj, partBits, pose->turret.tag_aim_animated, angles);
    angles[0] = pose->turret.barrelPitch;
    angles[1] = 0.0;
    DObjSetControlTagAngles((DObj_s *)obj, partBits, pose->turret.tag_flash, angles);
}

void __cdecl CG_DoBaseOriginController(const cpose_t *pose, const DObj_s *obj, int32_t *setPartBits)
{
    float *trans; // [esp+8h] [ebp-104h]
    float result[3]; // [esp+30h] [ebp-DCh] BYREF
    uint32_t rootBoneMask; // [esp+90h] [ebp-7Ch]
    float baseQuat[4]; // [esp+94h] [ebp-78h] BYREF
    float viewOffset[3]; // [esp+A4h] [ebp-68h] BYREF
    float origin[3]; // [esp+B0h] [ebp-5Ch] BYREF
    int32_t partIndex; // [esp+BCh] [ebp-50h]
    DObjAnimMat animMat; // [esp+C0h] [ebp-4Ch] BYREF
    int32_t rootBoneCount; // [esp+E0h] [ebp-2Ch]
    uint32_t maxHighIndex; // [esp+E4h] [ebp-28h]
    DObjAnimMat *mat; // [esp+E8h] [ebp-24h]
    uint32_t highIndex; // [esp+ECh] [ebp-20h]
    int32_t partBits[7];
    cg_s *cgameGlob;

    rootBoneCount = DObjGetRootBoneCount(obj);
    iassert(rootBoneCount);

    maxHighIndex = --rootBoneCount >> 5;
    for (highIndex = 0; highIndex < maxHighIndex; ++highIndex)
    {
        if (setPartBits[highIndex] != -1)
            goto notSet;
    }

    rootBoneMask = 0xFFFFFFFF >> ((rootBoneCount & 0x1F) + 1);
    if ((rootBoneMask | setPartBits[maxHighIndex]) == 0xFFFFFFFF)
        return;
notSet:
    mat = DObjGetRotTransArray(obj);
    if (mat)
    {
        AnglesToQuat(pose->angles, baseQuat);
        memset(partBits, 0, sizeof(partBits));
        partBits[3] = 0x80000000;
        cgameGlob = CG_GetLocalClientGlobals(R_GetLocalClientNum());
        viewOffset[0] = cgameGlob->refdef.viewOffset[0];
        viewOffset[1] = cgameGlob->refdef.viewOffset[1];
        viewOffset[2] = cgameGlob->refdef.viewOffset[2];
        partIndex = 0;
        while (partIndex <= rootBoneCount)
        {
            highIndex = partIndex >> 5;
            if ((setPartBits[partIndex >> 5] & partBits[3]) == 0)
            {
                if (DObjSetRotTransIndex((DObj_s*)obj, &partBits[3 - highIndex], partIndex))
                {
                    mat->quat[0] = baseQuat[0];
                    mat->quat[1] = baseQuat[1];
                    mat->quat[2] = baseQuat[2];
                    mat->quat[3] = baseQuat[3];

                    origin[0] = pose->origin[0];
                    origin[1] = pose->origin[1];
                    origin[2] = pose->origin[2];
                }
                else
                {
                    animMat.quat[0] = baseQuat[0];
                    animMat.quat[1] = baseQuat[1];
                    animMat.quat[2] = baseQuat[2];
                    animMat.quat[3] = baseQuat[3];
                    DObjSetTrans(&animMat, pose->origin);
                    float len = Vec4LengthSq(animMat.quat);
                    if (len == 0.0f)
                    {
                        animMat.quat[3] = 1.0f;
                        animMat.transWeight = 2.0f;
                    }
                    else
                    {
                        animMat.transWeight = 2.0f / len;
                    }
                    QuatMultiplyEquals(baseQuat, mat->quat);
                    MatrixTransformVectorQuatTrans(mat->trans, &animMat, origin);
                }
                Vec3Sub(origin, viewOffset, origin);
                DObjSetTrans(mat, origin);
            }
            ++partIndex;
            partBits[3] = (partBits[3] << 31) | ((uint32_t)partBits[3] >> 1);
            ++mat;
        }
    }
}

DObjAnimMat *__cdecl CG_DObjCalcPose(const cpose_t *pose, const DObj_s *obj, int32_t *partBits)
{
    DObjAnimMat *boneMatrix; // [esp+0h] [ebp-4h] BYREF

    iassert(obj);
    iassert(pose);

    if (!CL_DObjCreateSkelForBones(obj, partBits, &boneMatrix))
    {
        DObjCompleteHierarchyBits(obj, partBits);
        CG_DoControllers(pose, obj, partBits);
        DObjCalcSkel(obj, partBits);
    }

    return boneMatrix;
}

