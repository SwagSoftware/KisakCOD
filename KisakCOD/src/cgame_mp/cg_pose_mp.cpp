#include "cg_local_mp.h"
#include "cg_public_mp.h"


void __cdecl BG_Player_DoControllers(const CEntPlayerInfo *player, const DObj_s *obj, int *partBits)
{
    clientControllers_t *control; // [esp+0h] [ebp-8h]
    int i; // [esp+4h] [ebp-4h]

    control = player->control;
    for (i = 0; i < 6; ++i)
        DObjSetControlTagAngles(obj, partBits, player->tag[i], control->angles[i]);
    DObjSetLocalTag(obj, partBits, 0, control->tag_origin_offset, control->tag_origin_angles);
}

void  CG_VehPoseControllers(float a1@<ebp>, const cpose_t *pose, const DObj_s *obj, int *partBits)
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
    int v16; // [esp+3Ch] [ebp-194h]
    float v17; // [esp+40h] [ebp-190h]
    float v18; // [esp+44h] [ebp-18Ch]
    float v19; // [esp+48h] [ebp-188h]
    float v20; // [esp+4Ch] [ebp-184h]
    float v21; // [esp+50h] [ebp-180h]
    float v22; // [esp+54h] [ebp-17Ch]
    float v23; // [esp+58h] [ebp-178h]
    float4 wheelPos; // [esp+5Ch] [ebp-174h]
    float4 trans; // [esp+6Ch] [ebp-164h]
    int j; // [esp+7Ch] [ebp-154h]
    const DObjAnimMat *mtx; // [esp+80h] [ebp-150h]
    unsigned int boneIndex; // [esp+84h] [ebp-14Ch]
    int i; // [esp+88h] [ebp-148h]
    const DObjAnimMat *boneMtxList; // [esp+8Ch] [ebp-144h]
    unsigned int boneCount; // [esp+90h] [ebp-140h]
    XModel *model; // [esp+94h] [ebp-13Ch]
    vector4 invAxis; // [esp+98h] [ebp-138h]
    float4 axisW; // [esp+D8h] [ebp-F8h]
    float v35; // [esp+E8h] [ebp-E8h]
    float4 axisZ; // [esp+ECh] [ebp-E4h]
    float v37; // [esp+FCh] [ebp-D4h]
    vector4 axis; // [esp+100h] [ebp-D0h]
    float *v39; // [esp+140h] [ebp-90h]
    float v40[3]; // [esp+144h] [ebp-8Ch] BYREF
    float tempAxis[4][3]; // [esp+150h] [ebp-80h] BYREF
    int yaw; // [esp+180h] [ebp-50h]
    int suspTravel; // [esp+184h] [ebp-4Ch]
    int roll; // [esp+188h] [ebp-48h]
    int pitch; // [esp+18Ch] [ebp-44h]
    float v46; // [esp+190h] [ebp-40h] BYREF
    float v47; // [esp+194h] [ebp-3Ch]
    float v48; // [esp+198h] [ebp-38h]
    float steerAngles[3]; // [esp+19Ch] [ebp-34h] BYREF
    float bodyAngles[3]; // [esp+1A8h] [ebp-28h] BYREF
    float barrelAngles[7]; // [esp+1B4h] [ebp-1Ch] BYREF
    float retaddr; // [esp+1D0h] [ebp+0h]

    barrelAngles[4] = a1;
    barrelAngles[5] = retaddr;
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
    v46 = 0.0;
    v47 = 0.0;
    v48 = 0.0;
    pitch = pose->vehicle.pitch;
    steerAngles[0] = (double)pitch * 0.0054931640625;
    roll = pose->vehicle.roll;
    steerAngles[2] = (double)roll * 0.0054931640625;
    suspTravel = pose->vehicle.barrelPitch;
    bodyAngles[0] = (double)suspTravel * 0.0054931640625;
    if (pose->eType == 12)
        bodyAngles[2] = pose->turret.barrelPitch;
    yaw = pose->vehicle.yaw;
    barrelAngles[1] = (double)yaw * 0.0054931640625;
    LODWORD(tempAxis[3][2]) = pose->vehicle.steerYaw;
    v47 = (double)SLODWORD(tempAxis[3][2]) * 0.0054931640625;
    DObjSetLocalTag(obj, partBits, pose->vehicle.tag_body, vec3_origin, steerAngles);
    DObjSetLocalTag(obj, partBits, pose->vehicle.tag_turret, vec3_origin, barrelAngles);
    DObjSetLocalTag(obj, partBits, pose->vehicle.tag_barrel, vec3_origin, bodyAngles);
    tempAxis[3][1] = pose->vehicle.time;
    AnglesToAxis(pose->angles, (float (*)[3])v40);
    v39 = tempAxis[2];
    axis.w.u[3] = (unsigned int)pose->origin;
    tempAxis[2][0] = pose->origin[0];
    tempAxis[2][1] = pose->origin[1];
    tempAxis[2][2] = pose->origin[2];
    axis.w.u[2] = (unsigned int)v40;
    axisZ.v[2] = v40[0];
    axisZ.v[3] = v40[1];
    v37 = v40[2];
    axis.x.v[0] = 0.0;
    axis.x.v[1] = tempAxis[0][0];
    axis.x.v[2] = tempAxis[0][1];
    axis.x.v[3] = tempAxis[0][2];
    axis.y.v[0] = 0.0;
    axis.y.v[1] = tempAxis[1][0];
    axis.y.v[2] = tempAxis[1][1];
    axis.y.v[3] = tempAxis[1][2];
    axis.z.v[0] = 0.0;
    axis.z.v[1] = tempAxis[2][0];
    axis.z.v[2] = tempAxis[2][1];
    axis.z.v[3] = tempAxis[2][2];
    axis.w.v[0] = 1.0;
    axisZ.u[1] = (unsigned int)tempAxis[1];
    axisW.v[2] = tempAxis[1][0];
    axisW.v[3] = tempAxis[1][1];
    v35 = tempAxis[1][2];
    axisZ.v[0] = 0.0;
    axisW.u[1] = (unsigned int)tempAxis[2];
    invAxis.w.v[1] = tempAxis[2][0];
    invAxis.w.v[2] = tempAxis[2][1];
    invAxis.w.v[3] = tempAxis[2][2];
    axisW.v[0] = 0.0;
    boneMtxList = (const DObjAnimMat *)LODWORD(v40[0]);
    boneCount = LODWORD(tempAxis[0][0]);
    model = (XModel *)LODWORD(tempAxis[1][0]);
    invAxis.x.v[0] = 1.0;
    invAxis.x.v[1] = v40[1];
    invAxis.x.v[2] = tempAxis[0][1];
    invAxis.x.v[3] = tempAxis[1][1];
    invAxis.y.v[0] = 1.0;
    invAxis.y.v[1] = v40[2];
    invAxis.y.v[2] = tempAxis[0][2];
    invAxis.y.v[3] = tempAxis[1][2];
    invAxis.z.v[0] = 1.0;
    i = (int)DObjGetModel(obj, 0);
    boneIndex = XModelNumBones((const XModel *)i);
    mtx = XModelGetBasePose((const XModel *)i);
    for (j = 0; j < 4; ++j)
    {
        trans.u[3] = pose->vehicle.wheelBoneIndex[j];
        if (trans.u[3] < 0xFE && DObjSetRotTransIndex(obj, partBits, trans.u[3]))
        {
            trans.u[2] = (unsigned int)&mtx[trans.u[3]];
            trans.u[1] = trans.u[2] + 16;
            wheelPos.v[1] = *(float *)(trans.u[2] + 16);
            wheelPos.v[2] = *(float *)(trans.u[2] + 20);
            wheelPos.v[3] = *(float *)(trans.u[2] + 24);
            trans.v[0] = 0.0;
            v17 = wheelPos.v[1] * axisZ.v[2] + wheelPos.v[2] * axis.x.v[1] + wheelPos.v[3] * axis.y.v[1] + axis.z.v[1];
            v18 = wheelPos.v[1] * axisZ.v[3] + wheelPos.v[2] * axis.x.v[2] + wheelPos.v[3] * axis.y.v[2] + axis.z.v[2];
            v19 = wheelPos.v[1] * v37 + wheelPos.v[2] * axis.x.v[3] + wheelPos.v[3] * axis.y.v[3] + axis.z.v[3];
            v20 = wheelPos.v[1] * axis.x.v[0] + wheelPos.v[2] * axis.y.v[0] + wheelPos.v[3] * axis.z.v[0] + axis.w.v[0];
            v21 = v17;
            v22 = v18;
            v23 = v19;
            wheelPos.v[0] = v20;
            v16 = pose->vehicle.wheelFraction[j];
            v15 = (double)v16 * 0.00001525902189314365;
            v14 = v15 * (tempAxis[3][1] + 40.0);
            dist = 40.0 - tempAxis[3][1];
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
            v21 = v21 - invAxis.w.v[1];
            v22 = v22 - invAxis.w.v[2];
            v23 = v23 - invAxis.w.v[3];
            v5 = v21 * *(float *)&boneMtxList + v22 * invAxis.x.v[1] + v23 * invAxis.y.v[1];
            v6 = v21 * *(float *)&boneCount + v22 * invAxis.x.v[2] + v23 * invAxis.y.v[2];
            v7 = v21 * *(float *)&model + v22 * invAxis.x.v[3] + v23 * invAxis.y.v[3];
            v8 = v21 * invAxis.x.v[0] + v22 * invAxis.y.v[0] + v23 * invAxis.z.v[0];
            v21 = v5 - wheelPos.v[1];
            v22 = v6 - wheelPos.v[2];
            v23 = v7 - wheelPos.v[3];
            wheelPos.v[0] = v8 - trans.v[0];
            v4[0] = v21;
            v4[1] = v22;
            v4[2] = v23;
            v4[3] = wheelPos.v[0];
            if (v47 == 0.0 || (unsigned int)j > 1)
                DObjSetLocalTagInternal(obj, v4, 0, trans.u[3]);
            else
                DObjSetLocalTagInternal(obj, v4, &v46, trans.u[3]);
        }
    }
}

void __cdecl CG_DoControllers(const cpose_t *pose, const DObj_s *obj, int *partBits)
{
    int setPartBits[4]; // [esp+34h] [ebp-10h] BYREF
    int savedregs; // [esp+44h] [ebp+0h] BYREF

    //Profile_Begin(323);
    DObjGetSetBones(obj, setPartBits);
    switch (pose->eType)
    {
    case 1u:
        CG_Player_DoControllers(pose, obj, partBits);
        break;
    case 0xBu:
        CG_mg42_DoControllers(pose, obj, partBits);
        break;
    case 0xCu:
    case 0xEu:
        CG_VehPoseControllers(COERCE_FLOAT(&savedregs), pose, obj, partBits);
        break;
    default:
        break;
    }
    CG_DoBaseOriginController(pose, obj, setPartBits);
    if (pose->isRagdoll && (pose->ragdollHandle || pose->killcamRagdollHandle))
        Ragdoll_DoControllers(pose, obj, partBits);
    //Profile_EndInternal(0);
}

void __cdecl CG_Player_DoControllers(const cpose_t *pose, const DObj_s *obj, int *partBits)
{
    if (pose->fx.triggerTime)
        BG_Player_DoControllers(&pose->player, obj, partBits);
}

void __cdecl CG_mg42_DoControllers(const cpose_t *pose, const DObj_s *obj, int *partBits)
{
    float angles[3]; // [esp+10h] [ebp-10h] BYREF
    const float *viewAngles; // [esp+1Ch] [ebp-4h]

    if (pose->turret.playerUsing)
    {
        viewAngles = pose->turret.viewAngles;
        angles[0] = AngleDelta(*viewAngles, pose->angles[0]);
        angles[1] = AngleDelta(viewAngles[1], pose->angles[1]);
    }
    else
    {
        angles[0] = pose->turret.$9D88A49AD898204B3D6E378457DD8419::angles.pitch;
        angles[1] = pose->turret.$9D88A49AD898204B3D6E378457DD8419::angles.yaw;
    }
    angles[2] = 0.0;
    DObjSetControlTagAngles(obj, partBits, pose->turret.tag_aim, angles);
    DObjSetControlTagAngles(obj, partBits, pose->turret.tag_aim_animated, angles);
    angles[0] = pose->turret.barrelPitch;
    angles[1] = 0.0;
    DObjSetControlTagAngles(obj, partBits, pose->turret.tag_flash, angles);
}

void __cdecl CG_DoBaseOriginController(const cpose_t *pose, const DObj_s *obj, int *setPartBits)
{
    float *trans; // [esp+8h] [ebp-104h]
    float v4; // [esp+20h] [ebp-ECh]
    float v5; // [esp+24h] [ebp-E8h]
    float v6; // [esp+28h] [ebp-E4h]
    float v7; // [esp+2Ch] [ebp-E0h]
    float result; // [esp+30h] [ebp-DCh] BYREF
    float v9; // [esp+34h] [ebp-D8h]
    float v10; // [esp+38h] [ebp-D4h]
    float v11; // [esp+3Ch] [ebp-D0h]
    float v12; // [esp+40h] [ebp-CCh]
    float v13; // [esp+44h] [ebp-C8h]
    float v14; // [esp+48h] [ebp-C4h]
    float v15; // [esp+4Ch] [ebp-C0h]
    float v16; // [esp+50h] [ebp-BCh]
    float v17; // [esp+54h] [ebp-B8h]
    float v18; // [esp+58h] [ebp-B4h]
    float v19; // [esp+5Ch] [ebp-B0h]
    float v20; // [esp+60h] [ebp-ACh]
    float v21; // [esp+64h] [ebp-A8h]
    float v22; // [esp+68h] [ebp-A4h]
    float v23; // [esp+6Ch] [ebp-A0h]
    float v24; // [esp+70h] [ebp-9Ch]
    float v25; // [esp+74h] [ebp-98h]
    float v26; // [esp+78h] [ebp-94h]
    float v27; // [esp+7Ch] [ebp-90h]
    float v28; // [esp+80h] [ebp-8Ch]
    float *v29; // [esp+84h] [ebp-88h]
    float *v30; // [esp+88h] [ebp-84h]
    unsigned int LocalClientNum; // [esp+8Ch] [ebp-80h]
    unsigned int rootBoneMask; // [esp+90h] [ebp-7Ch]
    float baseQuat[4]; // [esp+94h] [ebp-78h] BYREF
    float viewOffset[3]; // [esp+A4h] [ebp-68h] BYREF
    float origin[3]; // [esp+B0h] [ebp-5Ch] BYREF
    int partIndex; // [esp+BCh] [ebp-50h]
    DObjAnimMat animMat; // [esp+C0h] [ebp-4Ch] BYREF
    int rootBoneCount; // [esp+E0h] [ebp-2Ch]
    unsigned int maxHighIndex; // [esp+E4h] [ebp-28h]
    DObjAnimMat *mat; // [esp+E8h] [ebp-24h]
    unsigned int highIndex; // [esp+ECh] [ebp-20h]
    int partBits[7]; // [esp+F0h] [ebp-1Ch] BYREF

    rootBoneCount = DObjGetRootBoneCount(obj);
    if (!rootBoneCount)
        MyAssertHandler(".\\cgame_mp\\cg_pose_mp.cpp", 198, 0, "%s", "rootBoneCount");
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
        memset(partBits, 0, 12);
        partBits[3] = 0x80000000;
        memset(&partBits[4], 0, 12);
        LocalClientNum = R_GetLocalClientNum();
        if (LocalClientNum)
            MyAssertHandler(
                "c:\\trees\\cod3\\src\\cgame_mp\\cg_local_mp.h",
                1071,
                0,
                "%s\n\t(localClientNum) = %i",
                "(localClientNum == 0)",
                LocalClientNum);
        v30 = MEMORY[0x9D8748];
        viewOffset[0] = MEMORY[0x9D8748][0];
        viewOffset[1] = MEMORY[0x9D8748][1];
        viewOffset[2] = MEMORY[0x9D8748][2];
        partIndex = 0;
        while (partIndex <= rootBoneCount)
        {
            highIndex = partIndex >> 5;
            if ((setPartBits[partIndex >> 5] & partBits[3]) == 0)
            {
                if (DObjSetRotTransIndex(obj, &partBits[3 - highIndex], partIndex))
                {
                    mat->quat[0] = baseQuat[0];
                    mat->quat[1] = baseQuat[1];
                    mat->quat[2] = baseQuat[2];
                    mat->quat[3] = baseQuat[3];
                    v29 = pose->origin;
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
                    v28 = Vec4LengthSq(animMat.quat);
                    if (v28 == 0.0)
                    {
                        animMat.quat[3] = 1.0;
                        animMat.transWeight = 2.0;
                    }
                    else
                    {
                        animMat.transWeight = 2.0 / v28;
                    }
                    v25 = mat->quat[0] * baseQuat[3]
                        + mat->quat[3] * baseQuat[0]
                        + mat->quat[2] * baseQuat[1]
                        - mat->quat[1] * baseQuat[2];
                    v26 = mat->quat[1] * baseQuat[3]
                        - mat->quat[2] * baseQuat[0]
                        + mat->quat[3] * baseQuat[1]
                        + mat->quat[0] * baseQuat[2];
                    v27 = mat->quat[2] * baseQuat[3]
                        + mat->quat[1] * baseQuat[0]
                        - mat->quat[0] * baseQuat[1]
                        + mat->quat[3] * baseQuat[2];
                    mat->quat[3] = mat->quat[3] * baseQuat[3]
                        - mat->quat[0] * baseQuat[0]
                        - mat->quat[1] * baseQuat[1]
                        - mat->quat[2] * baseQuat[2];
                    mat->quat[0] = v25;
                    mat->quat[1] = v26;
                    mat->quat[2] = v27;
                    trans = mat->trans;
                    if ((LODWORD(animMat.quat[0]) & 0x7F800000) == 0x7F800000
                        || (LODWORD(animMat.quat[1]) & 0x7F800000) == 0x7F800000
                        || (LODWORD(animMat.quat[2]) & 0x7F800000) == 0x7F800000
                        || (LODWORD(animMat.quat[3]) & 0x7F800000) == 0x7F800000)
                    {
                        MyAssertHandler(
                            "c:\\trees\\cod3\\src\\bgame\\../xanim/xanim_public.h",
                            432,
                            0,
                            "%s",
                            "!IS_NAN((mat->quat)[0]) && !IS_NAN((mat->quat)[1]) && !IS_NAN((mat->quat)[2]) && !IS_NAN((mat->quat)[3])");
                    }
                    if ((LODWORD(animMat.transWeight) & 0x7F800000) == 0x7F800000)
                        MyAssertHandler(
                            "c:\\trees\\cod3\\src\\bgame\\../xanim/xanim_public.h",
                            433,
                            0,
                            "%s",
                            "!IS_NAN(mat->transWeight)");
                    Vec3Scale(animMat.quat, animMat.transWeight, &result);
                    v14 = result * animMat.quat[0];
                    v5 = result * animMat.quat[1];
                    v12 = result * animMat.quat[2];
                    v15 = result * animMat.quat[3];
                    v4 = v9 * animMat.quat[1];
                    v13 = v9 * animMat.quat[2];
                    v11 = v9 * animMat.quat[3];
                    v6 = v10 * animMat.quat[2];
                    v7 = v10 * animMat.quat[3];
                    v16 = 1.0 - (v4 + v6);
                    v17 = v5 + v7;
                    v18 = v12 - v11;
                    v19 = v5 - v7;
                    v20 = 1.0 - (v14 + v6);
                    v21 = v13 + v15;
                    v22 = v12 + v11;
                    v23 = v13 - v15;
                    v24 = 1.0 - (v14 + v4);
                    origin[0] = *trans * v16 + trans[1] * v19 + trans[2] * v22 + animMat.trans[0];
                    origin[1] = *trans * v17 + trans[1] * v20 + trans[2] * v23 + animMat.trans[1];
                    origin[2] = *trans * v18 + trans[1] * v21 + trans[2] * v24 + animMat.trans[2];
                }
                Vec3Sub(origin, viewOffset, origin);
                DObjSetTrans(mat, origin);
            }
            ++partIndex;
            partBits[3] = (partBits[3] << 31) | ((unsigned int)partBits[3] >> 1);
            ++mat;
        }
    }
}

double __cdecl Vec4LengthSq(const float *v)
{
    return (float)(*v * *v + v[1] * v[1] + v[2] * v[2] + v[3] * v[3]);
}

DObjAnimMat *__cdecl CG_DObjCalcPose(const cpose_t *pose, const DObj_s *obj, int *partBits)
{
    DObjAnimMat *boneMatrix; // [esp+0h] [ebp-4h] BYREF

    if (!obj)
        MyAssertHandler(".\\cgame_mp\\cg_pose_mp.cpp", 307, 0, "%s", "obj");
    if (!pose)
        MyAssertHandler(".\\cgame_mp\\cg_pose_mp.cpp", 308, 0, "%s", "pose");
    if (!CL_DObjCreateSkelForBones(obj, partBits, &boneMatrix))
    {
        DObjCompleteHierarchyBits(obj, partBits);
        CG_DoControllers(pose, obj, partBits);
        DObjCalcSkel(obj, partBits);
    }
    return boneMatrix;
}

