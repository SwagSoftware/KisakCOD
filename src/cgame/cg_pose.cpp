#ifndef KISAK_SP
#error This file is for SinglePlayer only
#endif

#include "cg_pose.h"
#include "cg_ents.h"
#include <xanim/dobj_utils.h>
#include <ragdoll/ragdoll.h>
#include <client/cl_pose.h>
#include "cg_main.h"
#include <gfx_d3d/r_scene.h>
#include <universal/profile.h>

void __cdecl PitchToQuat(float pitch, float *quat)
{
    pitch = DEG2RAD(pitch);

    quat[0] = 0.0;
    quat[1] = sin(pitch);
    quat[2] = 0.0;
    quat[3] = cos(pitch);
}

void __cdecl RollToQuat(float  roll, float *quat)
{
    roll = DEG2RAD(roll);

    quat[0] = sin(roll);
    quat[1] = 0.0;
    quat[2] = 0.0;
    quat[3] = cos(roll);
}

void __cdecl LocalMatrixTransformVectorQuatTrans(const float *in, const DObjAnimMat *mat, float *out)
{
    double v6; // fp8
    double v7; // fp7
    double v8; // fp6
    double v9; // fp5
    double v10; // fp13
    double v11; // fp13
    double v12; // fp0
    float v13[20]; // [sp+50h] [-50h] BYREF

    LocalConvertQuatToMat(mat, (float (*)[3])v13);
    v6 = v13[1];
    v7 = v13[4];
    v8 = v13[2];
    v9 = v13[5];
    v10 = v13[7];
    *out = (float)((float)(v13[3] * in[1]) + (float)((float)(*in * v13[0]) + (float)(v13[6] * in[2]))) + mat->trans[0];
    v12 = (float)((float)((float)((float)v7 * in[1]) + (float)((float)(*in * (float)v6) + (float)((float)v10 * in[2])))
        + mat->trans[1]);
    v11 = v13[8];
    out[1] = v12;
    out[2] = (float)((float)((float)v9 * in[1]) + (float)((float)(*in * (float)v8) + (float)((float)v11 * in[2])))
        + mat->trans[2];
}

void __cdecl NormalizeQuatTrans(DObjAnimMat *mat)
{
    if ((float)((float)(mat->quat[3] * mat->quat[3])
        + (float)((float)(mat->quat[2] * mat->quat[2])
            + (float)((float)(mat->quat[0] * mat->quat[0]) + (float)(mat->quat[1] * mat->quat[1])))) == 0.0)
    {
        mat->quat[3] = 1.0;
        mat->transWeight = 2.0;
    }
    else
    {
        mat->transWeight = (float)2.0
            / (float)((float)(mat->quat[3] * mat->quat[3])
                + (float)((float)(mat->quat[2] * mat->quat[2])
                    + (float)((float)(mat->quat[0] * mat->quat[0])
                        + (float)(mat->quat[1] * mat->quat[1]))));
    }
}

void __cdecl CG_mg42_DoControllers(const cpose_t *pose, const DObj_s *obj, int *partBits, long double a4)
{
    bool playerUsing; // r10
    float *proneType; // r30
    double v9; // fp30
    long double v10; // fp2
    double v11; // fp0
    double v12; // fp31
    long double v13; // fp2
    double roll; // fp0
    double v15; // fp13
    float v16; // [sp+50h] [-70h] BYREF
    float pitch; // [sp+54h] [-6Ch]
    float v18; // [sp+58h] [-68h]
    float v19[12]; // [sp+60h] [-60h] BYREF

    if (!obj)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\cgame\\cg_pose.cpp", 32, 0, "%s", "obj");
    playerUsing = pose->turret.playerUsing;
    v16 = 0.0;
    pitch = 0.0;
    v18 = 0.0;
    v19[0] = 0.0;
    v19[1] = 0.0;
    v19[2] = 0.0;
    if (playerUsing)
    {
        if (!pose->actor.proneType)
            MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\cgame\\cg_pose.cpp", 39, 0, "%s", "pose->turret.viewAngles");
        proneType = (float *)pose->actor.proneType;
        v9 = (float)((float)(*proneType - pose->angles[0]) * (float)0.0027777778);
        *(double *)&a4 = (float)((float)((float)(*proneType - pose->angles[0]) * (float)0.0027777778) + (float)0.5);
        v10 = floor(a4);
        v11 = pose->angles[1];
        v16 = (float)((float)v9 - (float)*(double *)&v10) * (float)360.0;
        v12 = (float)((float)(proneType[1] - (float)v11) * (float)0.0027777778);
        *(double *)&v10 = (float)((float)((float)(proneType[1] - (float)v11) * (float)0.0027777778) + (float)0.5);
        v13 = floor(v10);
        v19[0] = 0.0;
        pitch = (float)((float)v12 - (float)*(double *)&v13) * (float)360.0;
    }
    else
    {
        roll = pose->actor.roll;
        //v15 = (float)(pose->turret.$51809EA76892896F64281DFB626CE797::angles.pitch - pose->actor.roll);
        v15 = (float)(pose->turret.angles.pitch - pose->actor.roll);
        pitch = pose->actor.pitch;
        v16 = v15;
        v19[0] = roll;
    }
    DObjSetControlTagAngles((DObj_s*)obj, partBits, pose->turret.tag_aim, &v16);
    DObjSetControlTagAngles((DObj_s*)obj, partBits, pose->turret.tag_aim_animated, &v16);
    DObjSetControlTagAngles((DObj_s*)obj, partBits, pose->turret.tag_flash, v19);
}

#if 0
void __cdecl CG_Vehicle_DoControllers(const cpose_t *pose, const DObj_s *obj, int *partBits)
{
    __int64 v3; // r28
    __int128 boneMatrix; // r9 OVERLAPPED
    __int16 pitch; // r10
    unsigned int tag_body; // r5
    __int64 offset; // r11
    double height; // fp29
    const XModel *Model; // r30
    const DObjAnimMat *BasePose; // r24
    int v25; // r25
    CEntFx *v26; // r30
    unsigned __int8 *wheelBoneIndex; // r23
    unsigned int v28; // r31
    const float *v35; // r5
    __int64 v36; // [sp+50h] [-1D0h] BYREF
    __int64 v37; // [sp+58h] [-1C8h] BYREF
    __int64 v38; // [sp+60h] [-1C0h]
    float v39; // [sp+68h] [-1B8h] BYREF
    float v40; // [sp+6Ch] [-1B4h]
    float v41; // [sp+70h] [-1B0h]
    float v42[4]; // [sp+78h] [-1A8h] BYREF
    float v43[4]; // [sp+88h] [-198h] BYREF
    float v44[4]; // [sp+98h] [-188h] BYREF
    __int64 v45; // [sp+A8h] [-178h]
    __int64 v46; // [sp+B0h] [-170h]
    float v47[3]; // [sp+C0h] [-160h] BYREF
    char v48; // [sp+D0h] [-150h] BYREF
    _BYTE v49[8]; // [sp+D8h] [-148h] BYREF
    char v50; // [sp+E0h] [-140h] BYREF
    float v51[3]; // [sp+E4h] [-13Ch] BYREF
    float v52[40]; // [sp+F0h] [-130h] BYREF

    if (!obj)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\cgame\\cg_pose.cpp", 82, 0, "%s", "obj");
    DWORD1(boneMatrix) = v44;
    LODWORD(boneMatrix) = pose->vehicle.barrelPitch;
    DWORD2(boneMatrix) = pose->vehicle.yaw;
    pitch = pose->vehicle.pitch;
    LODWORD(v3) = pose->vehicle.steerYaw;
    WORD1(boneMatrix) = pose->vehicle.roll;
    tag_body = pose->vehicle.tag_body;
    v45 = boneMatrix;
    v46 = *(_QWORD *)((char *)&boneMatrix + 4);
    v38 = v3;
    LODWORD(offset) = pitch;
    HIDWORD(offset) = SWORD1(boneMatrix);
    v42[0] = 0.0;
    v42[2] = 0.0;
    v43[1] = 0.0;
    v43[2] = 0.0;
    v36 = offset;
    v37 = *(_QWORD *)((char *)&boneMatrix - 4);
    v44[1] = 0.0;
    v39 = 0.0;
    v41 = 0.0;
    v44[2] = (float)*(__int64 *)((char *)&boneMatrix - 4) * (float)0.0054931641;
    v44[0] = (float)offset * (float)0.0054931641;
    v43[0] = (float)(__int64)boneMatrix * (float)0.0054931641;
    v42[1] = (float)*(__int64 *)((char *)&boneMatrix + 4) * (float)0.0054931641;
    v40 = (float)v3 * (float)0.0054931641;
    DObjSetLocalTag((DObj_s*)obj, partBits, tag_body, vec3_origin, v44);
    DObjSetLocalTag((DObj_s*)obj, partBits, pose->vehicle.tag_turret, vec3_origin, v42);
    DObjSetLocalTag((DObj_s*)obj, partBits, pose->vehicle.tag_barrel, vec3_origin, v43);
    height = pose->actor.height;
    AnglesToAxis(pose->angles, (float (*)[3])v47);
    v51[0] = pose->origin[0];
    __asm { vspltisw  v0, 0 }
    v51[1] = pose->origin[1];
    _R11 = &v48;
    v51[2] = pose->origin[2];
    __asm { vupkd3d128 v0, v0, 1 }
    _R10 = v49;
    _R27 = 16;
    _R9 = v49;
    _R8 = v51;
    __asm { lvx128    trans, r0, r11 }
    _R11 = v47;
    _R7 = v51;
    __asm
    {
        vmr       offset, v0
        lvrx      v8, r27, r10
        vmr       v9, v0
        lvlx      boneMatrix, r0, r9
    }
    __asm { lvrx      v6, r27, r8 }
    __asm { lvx128    v127, r0, r11 }
    _R11 = &v50;
    __asm
    {
        vsldoi128 v126, v127, trans, 0xC
        vrlimi128 v127, v0, 1, 3
        lvlx      v5, r0, r7
        lvx128    angles, r0, r11
    }
    __asm
    {
        vsldoi128 v125, trans, angles, 8
        vrlimi128 v0, angles, 0xE, 1
    }
    _R11 = &g_one;
    __asm
    {
        vor       angles, boneMatrix, v8
        vor       v8, v5, v6
        vrlimi128 v126, offset, 1, 3
        vrlimi128 v125, v9, 1, 3
        vmr128    v123, v0
        lvx128    trans, r0, r11
    }
    __asm { vmrghw128 v0, v126, trans }
    _R28 = &g_keepXYZ;
    __asm
    {
        lvx128    rollQuat, r0, r28
        vand128   v124, angles, rollQuat
        vmrglw128 angles, v127, v125
        vand128   v122, v8, rollQuat
        vmrglw128 rollQuat, v126, trans
        vmrghw128 trans, v127, v125
        vmrghw128 v119, angles, rollQuat
        vmrghw128 v121, trans, v0
        vmrglw128 v120, trans, v0
    }
    Model = DObjGetModel(obj, 0);
    XModelNumBones(Model);
    BasePose = XModelGetBasePose(Model);
    v25 = 0;
    v26 = &pose->fx + 2;
    wheelBoneIndex = pose->vehicle.wheelBoneIndex;
    do
    {
        v28 = wheelBoneIndex[v25];
        if (v28 < 0xFE && DObjSetRotTransIndex(obj, partBits, wheelBoneIndex[v25]))
        {
            LODWORD(_R10) = HIWORD(v26->triggerTime);
            __asm
            {
                lvx128    v0, r0, r28
                vmr128    offset, v123
            }
            _R11 = (int)BasePose[v28].trans;
            HIDWORD(_R10) = &v37;
            v38 = _R10;
            __asm
            {
                lvlx      rollQuat, r0, r11
                lvrx      trans, r27, r11
            }
            _R11 = v52;
            __asm { vor       trans, rollQuat, trans }
            *(float *)&v37 = 40.0;
            __asm
            {
                vand      v0, trans, v0
                vspltw    trans, v0, 2
                vspltw    rollQuat, v0, 1
                vspltw    angles, v0, 0
                vmaddfp128 offset, v125, trans, offset
                vmaddfp128 offset, rollQuat, v126, offset
                vmaddfp128 offset, angles, v127, offset
            }
            _FP12 = (float)((float)((float)((float)((float)height + (float)40.0) * (float)_R10) * (float)0.000015259022)
                - (float)((float)40.0 - (float)height));
            LODWORD(_R10) = &v36;
            __asm { fsel      f0, f12, f13, f0 }
            *(float *)&v36 = -_FP0;
            __asm
            {
                lvlx      trans, r0, r9
                vspltw    trans, trans, 0
                lvlx      rollQuat, r0, r10
                vspltw    rollQuat, rollQuat, 0
                vmaddfp128 offset, trans, v124, offset
                vmaddfp128 offset, rollQuat, v124, offset
                vsubfp128 trans, offset, v122
                vspltw    rollQuat, trans, 2
                vspltw    angles, trans, 1
                vspltw    trans, trans, 0
                vmulfp128 rollQuat, rollQuat, v119
                vmaddfp128 rollQuat, angles, v120, rollQuat
                vmaddfp128 rollQuat, trans, v121, rollQuat
                vsubfp    v0, rollQuat, v0
                stvx128   v0, r0, r11
            }
            if (v40 == 0.0 || (unsigned int)v25 > 1)
                v35 = 0;
            else
                v35 = &v39;
            DObjSetLocalTagInternal(obj, v52, v35, v28);
        }
        ++v25;
        v26 = (CEntFx *)((char *)v26 + 2);
    } while (v25 < 6);
}
#endif

void CG_Vehicle_DoControllers(const cpose_t *pose, const DObj_s *obj, int *partBits)
{
    int v5; // xmm0_4
    float v6[3]; // [esp-Ch] [ebp-2A8h] BYREF
    float angles[3]; // [esp+0h] [ebp-29Ch]
    int ChildBones; // [esp+Ch] [ebp-290h]
    unsigned __int8 v9[4]; // [esp+10h] [ebp-28Ch] BYREF
    float v10; // [esp+14h] [ebp-288h]
    float v12; // [esp+1Ch] [ebp-280h]
    float trans[3]; // [esp+20h] [ebp-27Ch]
    float v14; // [esp+24h] [ebp-278h]
    float v15[3]; // [esp+28h] [ebp-274h] BYREF
    int k; // [esp+38h] [ebp-264h]
    int n; // [esp+40h] [ebp-25Ch]
    int m; // [esp+44h] [ebp-258h]
    DObjAnimMat *skel; // [esp+48h] [ebp-254h]
    unsigned __int8 v22[4]; // [esp+4Ch] [ebp-250h] BYREF
    int j; // [esp+50h] [ebp-24Ch] BYREF
    int numChildren; // [esp+54h] [ebp-248h]
    unsigned __int8 children[4]; // [esp+58h] [ebp-244h]
    float offset[4]; // [esp+5Ch] [ebp-240h]
    float v27; // [esp+6Ch] [ebp-230h]
    float v28; // [esp+70h] [ebp-22Ch]
    int v29; // [esp+74h] [ebp-228h]
    int v30; // [esp+78h] [ebp-224h]
    int v31; // [esp+7Ch] [ebp-220h]
    float v32; // [esp+80h] [ebp-21Ch]
    float dist; // [esp+84h] [ebp-218h]
    float v34; // [esp+88h] [ebp-214h]
    float v35; // [esp+8Ch] [ebp-210h]
    float v36; // [esp+90h] [ebp-20Ch]
    float v37; // [esp+94h] [ebp-208h]
    float v38; // [esp+98h] [ebp-204h]
    float v39; // [esp+9Ch] [ebp-200h]
    float v40; // [esp+A0h] [ebp-1FCh]
    float4 partPos; // [esp+A4h] [ebp-1F8h]
    //float4 trans; // [esp+B4h] [ebp-1E8h]
    centity_s *Entity; // [esp+C4h] [ebp-1D8h]
    const DObjAnimMat *mtx; // [esp+C8h] [ebp-1D4h]
    unsigned int boneIndex; // [esp+CCh] [ebp-1D0h]
    centity_s *cent; // [esp+D0h] [ebp-1CCh]
    const DObjAnimMat *boneMtxList; // [esp+D4h] [ebp-1C8h]
    const DObjAnimMat *remote_boneMtxList; // [esp+D8h] [ebp-1C4h]
    unsigned int boneCount; // [esp+DCh] [ebp-1C0h]
    XModel *model; // [esp+E0h] [ebp-1BCh]
    vector4 invAxis; // [esp+E4h] [ebp-1B8h]
    float4 axisW; // [esp+124h] [ebp-178h]
    float v53; // [esp+134h] [ebp-168h]
    float4 axisZ; // [esp+138h] [ebp-164h]
    float v55; // [esp+148h] [ebp-154h]
    vector4 axis; // [esp+14Ch] [ebp-150h]
    float *v57; // [esp+18Ch] [ebp-110h]
    float v58[3]; // [esp+190h] [ebp-10Ch] BYREF
    float tempAxis[4][3]; // [esp+19Ch] [ebp-100h] BYREF
    float suspTravel; // [esp+1CCh] [ebp-D0h]
    float minigunAngles[3]; // [esp+1D0h] [ebp-CCh] BYREF
    float barrelOffset[5]; // [esp+1DCh] [ebp-C0h] BYREF
    float gunnerTurretAngles[4][3]; // [esp+1F0h] [ebp-ACh] BYREF
    float gunnerBarrelAngles[4][3]; // [esp+224h] [ebp-78h] BYREF
    int i; // [esp+254h] [ebp-48h]
    float steerYaw; // [esp+258h] [ebp-44h]
    float steerAnglesPitch[3]; // [esp+25Ch] [ebp-40h] BYREF
    float steerAnglesYaw[3]; // [esp+268h] [ebp-34h] BYREF
    float bodyAngles[3]; // [esp+274h] [ebp-28h] BYREF
    float barrelAngles[3]; // [esp+280h] [ebp-1Ch] BYREF
    float turretAngles[4]; // [esp+28Ch] [ebp-10h]
    float retaddr; // [esp+29Ch] [ebp+0h]

    //turretAngles[1] = a1;
    //turretAngles[2] = retaddr;
    iassert(obj);

    memset(barrelAngles, 0, sizeof(barrelAngles));
    memset(bodyAngles, 0, sizeof(bodyAngles));
    memset(steerAnglesYaw, 0, sizeof(steerAnglesYaw));
    memset(steerAnglesPitch, 0, sizeof(steerAnglesPitch));

    //gunnerBarrelAngles[3][2] = 0.0f;

    //i = *(_DWORD *)&FLOAT_0_0;
    steerYaw = 0.0f;
    steerAnglesYaw[0] = (float)pose->vehicle.pitch * 0.0054931641;
    steerAnglesYaw[2] = (float)pose->vehicle.roll * 0.0054931641;
    bodyAngles[0] = (float)pose->vehicle.barrelPitch * 0.0054931641;
    barrelAngles[1] = (float)pose->vehicle.yaw * 0.0054931641;
    //gunnerBarrelAngles[3][0] = 0.0f;
    //gunnerBarrelAngles[3][1] = (float)pose->vehicle.steerYaw * 0.0054931641;
    //gunnerBarrelAngles[3][2] = (float)pose->vehicle.steerPitch * 0.0054931641;
    //
    //for (gunnerBarrelAngles[3][0] = 0.0; SLODWORD(gunnerBarrelAngles[3][0]) < 4; ++LODWORD(gunnerBarrelAngles[3][0]))
    //{
    //    LODWORD(gunnerTurretAngles[3][0]) = &gunnerTurretAngles[LODWORD(gunnerBarrelAngles[3][0]) + 3][1];
    //    *(_DWORD *)LODWORD(gunnerTurretAngles[3][0]) = *(_DWORD *)&FLOAT_0_0;
    //    *(_DWORD *)(LODWORD(gunnerTurretAngles[3][0]) + 4) = *(_DWORD *)&FLOAT_0_0;
    //    *(_DWORD *)(LODWORD(gunnerTurretAngles[3][0]) + 8) = *(_DWORD *)&FLOAT_0_0;
    //
    //    LODWORD(barrelOffset[1]) = &barrelOffset[3 * LODWORD(gunnerBarrelAngles[3][0]) + 2];
    //    *(_DWORD *)LODWORD(barrelOffset[1]) = *(_DWORD *)&FLOAT_0_0;
    //    *(_DWORD *)(LODWORD(barrelOffset[1]) + 4) = *(_DWORD *)&FLOAT_0_0;
    //    *(_DWORD *)(LODWORD(barrelOffset[1]) + 8) = *(_DWORD *)&FLOAT_0_0;
    //
    //    barrelOffset
    //    gunnerTurretAngles[LODWORD(gunnerBarrelAngles[3][0]) + 3][1] = (float)pose->vehicle.gunnerPitch[LODWORD(gunnerBarrelAngles[3][0])]
    //        * 0.0054931641;
    //    barrelOffset[3 * LODWORD(gunnerBarrelAngles[3][0]) + 3] = (float)pose->vehicle.gunnerYaw[LODWORD(gunnerBarrelAngles[3][0])]
    //        * 0.0054931641;
    //}

    DObjSetLocalTag((DObj_s*)obj, partBits, pose->vehicle.tag_body, vec3_origin, steerAnglesYaw);
    DObjSetLocalTag((DObj_s*)obj, partBits, pose->vehicle.tag_turret, vec3_origin, barrelAngles);
    DObjSetLocalTag((DObj_s*)obj, partBits, pose->vehicle.tag_barrel, vec3_origin, bodyAngles);

    //if (pose->vehicle.barrelRecoil > 0.0)
    //{
    //    barrelOffset[0] = pose->vehicle.barrelRecoil;
    //    minigunAngles[0] = barrelOffset[0] * recoilVec[0];
    //    minigunAngles[1] = barrelOffset[0] * *(float *)&dword_E03414;
    //    minigunAngles[2] = barrelOffset[0] * *(float *)&dword_E03418;
    //    DObjSetLocalTag(obj, partBits, pose->vehicle.tag_barrel_recoil, minigunAngles, vec3_origin);
    //}
    //for (gunnerBarrelAngles[3][0] = 0.0; SLODWORD(gunnerBarrelAngles[3][0]) < 4; ++LODWORD(gunnerBarrelAngles[3][0]))
    //{
    //    DObjSetLocalTag(
    //        obj,
    //        partBits,
    //        pose->vehicle.tag_gunner_turret[LODWORD(gunnerBarrelAngles[3][0])],
    //        vec3_origin,
    //        &barrelOffset[3 * LODWORD(gunnerBarrelAngles[3][0]) + 2]);
    //    DObjSetLocalTag(
    //        obj,
    //        partBits,
    //        pose->vehicle.tag_gunner_barrel[LODWORD(gunnerBarrelAngles[3][0])],
    //        vec3_origin,
    //        &gunnerTurretAngles[LODWORD(gunnerBarrelAngles[3][0]) + 3][1]);
    //}
    //if (pose->vehicle.tag_minigun_spin != 254)
    //{
    //    tempAxis[3][1] = *(float *)&FLOAT_0_0;
    //    tempAxis[3][2] = *(float *)&FLOAT_0_0;
    //    suspTravel = (float)pose->vehicle.minigun_rotation * 0.0054931641;
    //    DObjSetLocalTag(obj, partBits, pose->vehicle.tag_minigun_spin, vec3_origin, &tempAxis[3][1]);
    //}
    //tempAxis[3][0] = pose->vehicle.time;

    AnglesToAxis(pose->angles, tempAxis);
    //v51[2] = tempAxis[3];
    //v51[1] = pose->origin;
    tempAxis[3][0] = pose->origin[0];
    tempAxis[3][1] = pose->origin[1];
    tempAxis[3][2] = pose->origin[2];
    //v51[0] = tempAxis;
    axis.x.v[0] = tempAxis[0][0];
    axis.x.v[1] = tempAxis[0][1];
    axis.x.v[2] = tempAxis[0][2];
    axis.x.v[3] = 0.0f;

    axis.y.v[0] = tempAxis[1][0];
    axis.y.v[1] = tempAxis[1][1];
    axis.y.v[2] = tempAxis[1][2];
    axis.y.v[3] = 0.0f;

    axis.z.v[0] = tempAxis[2][0];
    axis.z.v[1] = tempAxis[2][1];
    axis.z.v[2] = tempAxis[2][2];
    axis.z.v[3] = 0.0f;

    axis.w.v[0] = tempAxis[3][0];
    axis.w.v[1] = tempAxis[3][1];
    axis.w.v[2] = tempAxis[3][2];
    axis.w.v[3] = 1.0f;

    //v49 = tempAxis[2];
    //v47 = tempAxis[3];

    axisZ.v[0] = tempAxis[2][0];
    axisZ.v[1] = tempAxis[2][1];
    axisZ.v[2] = tempAxis[2][2];
    axisZ.v[3] = 0.0f;

    axisW.v[0] = tempAxis[3][0];
    axisW.v[1] = tempAxis[3][1];
    axisW.v[2] = tempAxis[3][2];
    axisW.v[3] = 0.0f;

    invAxis.x.v[0] = tempAxis[0][0];
    invAxis.x.v[1] = tempAxis[1][0];
    invAxis.x.v[2] = tempAxis[2][0];
    invAxis.x.v[3] = 1.0f;

    invAxis.y.v[0] = tempAxis[0][1];
    invAxis.y.v[1] = tempAxis[1][1];
    invAxis.y.v[2] = tempAxis[2][1];
    invAxis.y.v[3] = 1.0f;

    invAxis.z.v[0] = tempAxis[0][2];
    invAxis.z.v[1] = tempAxis[1][2];
    invAxis.z.v[2] = tempAxis[2][2];
    invAxis.z.v[3] = 1.0f;

    model = DObjGetModel(obj, 0);
    boneCount = XModelNumBones(model);
    remote_boneMtxList = XModelGetBasePose(model);
    boneMtxList = remote_boneMtxList;

    //cent = CG_GetEntity(pose->localClientNum, DObjGetEntNum(obj) - 1);

    //if (cent->nitrousVeh)
    {
        if (DObjGetRotTransArray(obj))
        {
            for (k = 0; k < 6; ++k)
            {
                boneIndex = pose->vehicle.wheelBoneIndex[k];
                if (boneIndex < 0xFE && DObjSetRotTransIndex((DObj_s*)obj, partBits, boneIndex))
                {
                    //trans[0] = 0.0f;
                    //trans[1] = 0.0f;
                    //trans[2] = pose->vehicle.wheelHeight[k];
                    //
                    //angles[0] = 0.0f;
                    //angles[1] = pose->vehicle.nitrousWheelYaw[k];
                    //angles[2] = 0.0f;
                    //
                    //DObjSetLocalTagInternal(obj, trans, angles, boneIndex);
                    //
                    //angles[0] = pose->vehicle.nitrousWheelRotation[k];
                    //angles[1] = 0.0f;
                    //angles[2] = 0.0f;


                    // KISAKTODO: very scuffed logic

                    trans[0] = 0.0f;
                    trans[1] = 0.0f;
                    trans[2] = pose->actor.height;

                    angles[0] = 0.0f;
                    //angles[1] = pose->vehicle.nitrousWheelYaw[k];
                    angles[1] = 0.0f;
                    angles[2] = 0.0f;

                    DObjSetLocalTagInternal(obj, trans, angles, boneIndex);

                    //ChildBones = DObjGetChildBones(obj, boneIndex, v9, 4);
                    //for (int bone = 0; bone < ChildBones; bone++)
                    //{
                    //    if (DObjSetRotTransIndex((DObj_s*)obj, partBits, v9[bone]))
                    //    {
                    //        DObjSetLocalTagInternal(obj, vec3_origin, angles, v9[bone]);
                    //    }
                    //}

                    DObjSetRotTransIndex((DObj_s *)obj, partBits, boneIndex);
                }
            }
            //for (angles[1] = 0.0; SLODWORD(angles[1]) < 4; ++LODWORD(angles[1]))
            //{
            //    boneIndex = pose->vehicle.tag_extra_tank_wheels[LODWORD(angles[1])];
            //    if (boneIndex < 0xFE)
            //    {
            //        angles[0] = pose->vehicle.nitrousWheelRotation[LODWORD(angles[1])] * pose->vehicle.extra_wheel_rot_scale;
            //        v6[0] = angles[0];
            //        v6[1] = *(float *)&FLOAT_0_0;
            //        v6[2] = *(float *)&FLOAT_0_0;
            //        if (DObjSetRotTransIndex(obj, partBits, boneIndex))
            //            DObjSetLocalTagInternal(obj, vec3_origin, v6, boneIndex);
            //    }
            //}
        }
    }
    //else
    //{
    //    for (gunnerBarrelAngles[3][0] = 0.0; SLODWORD(gunnerBarrelAngles[3][0]) < 6; ++LODWORD(gunnerBarrelAngles[3][0]))
    //    {
    //        trans.u[3] = pose->vehicle.wheelBoneIndex[LODWORD(gunnerBarrelAngles[3][0])];
    //        if (trans.u[3] < 0xFE && DObjSetRotTransIndex(obj, partBits, trans.u[3]))
    //        {
    //            trans.u[2] = (unsigned int)&mtx[trans.u[3]];
    //            trans.u[1] = trans.u[2] + 16;
    //            *(_QWORD *)&partPos.unitVec[1].packed = *(_QWORD *)(trans.u[2] + 16);
    //            partPos.u[3] = *(_DWORD *)(trans.u[2] + 24);
    //            trans.u[0] = *(_DWORD *)&FLOAT_0_0;
    //            v34 = (float)((float)((float)(partPos.v[1] * axisZ.v[2]) + (float)(partPos.v[2] * axis.x.v[1]))
    //                + (float)(partPos.v[3] * axis.y.v[1]))
    //                + axis.z.v[1];
    //            v35 = (float)((float)((float)(partPos.v[1] * axisZ.v[3]) + (float)(partPos.v[2] * axis.x.v[2]))
    //                + (float)(partPos.v[3] * axis.y.v[2]))
    //                + axis.z.v[2];
    //            v36 = (float)((float)((float)(partPos.v[1] * v55) + (float)(partPos.v[2] * axis.x.v[3]))
    //                + (float)(partPos.v[3] * axis.y.v[3]))
    //                + axis.z.v[3];
    //            v37 = (float)((float)((float)(partPos.v[1] * axis.x.v[0]) + (float)(partPos.v[2] * axis.y.v[0]))
    //                + (float)(partPos.v[3] * axis.z.v[0]))
    //                + axis.w.v[0];
    //            v38 = v34;
    //            v39 = v35;
    //            v40 = v36;
    //            partPos.v[0] = v37;
    //            dist = (float)(tempAxis[3][0] + 40.0) * pose->vehicle.wheelHeight[LODWORD(gunnerBarrelAngles[3][0])];
    //            v32 = 40.0 - tempAxis[3][0];
    //            v5 = (float)(dist - (float)(40.0 - tempAxis[3][0])) < 0.0 ? LODWORD(v32) : LODWORD(dist);
    //            v31 = v5;
    //            v30 = v5;
    //            v38 = (float)(40.0 * axisW.v[2]) + v38;
    //            v39 = (float)(40.0 * axisW.v[3]) + v39;
    //            v40 = (float)(40.0 * v53) + v40;
    //            v29 = v5 ^ _mask__NegFloat_;
    //            v38 = (float)(COERCE_FLOAT(v5 ^ _mask__NegFloat_) * axisW.v[2]) + v38;
    //            v39 = (float)(COERCE_FLOAT(v5 ^ _mask__NegFloat_) * axisW.v[3]) + v39;
    //            v40 = (float)(COERCE_FLOAT(v5 ^ _mask__NegFloat_) * v53) + v40;
    //            v38 = v38 - invAxis.w.v[1];
    //            v39 = v39 - invAxis.w.v[2];
    //            v40 = v40 - invAxis.w.v[3];
    //            offset[2] = (float)((float)(v38 * *(float *)&remote_boneMtxList) + (float)(v39 * invAxis.x.v[1]))
    //                + (float)(v40 * invAxis.y.v[1]);
    //            offset[3] = (float)((float)(v38 * *(float *)&boneCount) + (float)(v39 * invAxis.x.v[2]))
    //                + (float)(v40 * invAxis.y.v[2]);
    //            v27 = (float)((float)(v38 * *(float *)&model) + (float)(v39 * invAxis.x.v[3])) + (float)(v40 * invAxis.y.v[3]);
    //            v28 = (float)((float)(v38 * invAxis.x.v[0]) + (float)(v39 * invAxis.y.v[0])) + (float)(v40 * invAxis.z.v[0]);
    //            v38 = offset[2] - partPos.v[1];
    //            v39 = offset[3] - partPos.v[2];
    //            v40 = v27 - partPos.v[3];
    //            partPos.v[0] = v28 - trans.v[0];
    //            *(float *)&j = offset[2] - partPos.v[1];
    //            *(float *)&numChildren = offset[3] - partPos.v[2];
    //            *(float *)children = v27 - partPos.v[3];
    //            offset[0] = v28 - trans.v[0];
    //            steerAnglesPitch[1] = LODWORD(gunnerBarrelAngles[3][0]) > 1 ? *(float *)&FLOAT_0_0 : gunnerBarrelAngles[3][1];
    //            DObjSetLocalTagInternal(obj, (const float *)&j, steerAnglesPitch, trans.u[3]);
    //            if (gunnerBarrelAngles[3][2] != 0.0)
    //            {
    //                skel = (DObjAnimMat *)DObjGetChildBones(obj, trans.unitVec[3].array[0], v22, 4);
    //                for (m = 0; m < (int)skel; ++m)
    //                {
    //                    if (DObjSetRotTransIndex(obj, partBits, v22[m]))
    //                        DObjSetLocalTagInternal(obj, vec3_origin, &gunnerBarrelAngles[3][2], v22[m]);
    //                }
    //            }
    //        }
    //    }
    //    gunnerBarrelAngles[3][2] = gunnerBarrelAngles[3][2] * pose->vehicle.extra_wheel_rot_scale;
    //    for (n = 0; n < 4; ++n)
    //    {
    //        trans.u[3] = pose->vehicle.tag_extra_tank_wheels[n];
    //        if (trans.u[3] < 254 && DObjSetRotTransIndex(obj, partBits, trans.u[3]))
    //            DObjSetLocalTagInternal(obj, vec3_origin, &gunnerBarrelAngles[3][2], trans.u[3]);
    //    }
    //}
}

void __cdecl CG_Actor_DoControllers(const cpose_t *pose, const DObj_s *obj, int *partBits)
{
    DObjAnimMat *mat; // r28
    int proneType; // r8
    float offset[4]; // [sp+50h] [-60h] BYREF
    float pitchQuat[4]; // [sp+60h] [-50h] BYREF
    float rollQuat[4]; // [sp+70h] [-40h] BYREF

    iassert(obj);

    if (pose->actor.proneType)
    {
        mat = DObjGetRotTransArray(obj);
        if (mat)
        {
            if (DObjSetRotTransIndex((DObj_s *)obj, partBits, 0))
            {
                proneType = pose->actor.proneType;
                if (proneType == 2)
                {
                    PitchToQuat(pose->actor.pitch, pitchQuat);
                    RollToQuat(pose->actor.roll, rollQuat);
                    QuatMultiply(rollQuat, pitchQuat, mat->quat);
                }
                else
                {
                    iassert(pose->actor.proneType == CENT_ACTOR_PRONE_NORMAL);
                    PitchToQuat(pose->actor.pitch, mat->quat);
                }

                offset[0] = 0.0;
                offset[1] = 0.0;
                offset[2] = pose->actor.height;

                DObjSetTrans(mat, offset);
            }
        }
    }
}

void __cdecl CG_DoBaseOriginController(const cpose_t *pose, const DObj_s *obj, int *setPartBits)
{
    unsigned int rootBoneCount; // r31
    DObjAnimMat *mat; // r30
    int LocalClientNum; // r8
    float baseQuat[4];
    int partBits[8];
    DObjAnimMat animMat;
    unsigned int highIndex;
    int partIndex;
    float origin[3];
    float viewOffset[3];

    rootBoneCount = DObjGetRootBoneCount(obj);
    iassert(rootBoneCount);

    unsigned int maxHighIndex = --rootBoneCount >> 5;
    for (highIndex = 0; highIndex < maxHighIndex; ++highIndex)
    {
        if (setPartBits[highIndex] != -1)
            goto notSet;
    }

    if (((0xFFFFFFFF >> ((rootBoneCount & 0x1F) + 1)) | setPartBits[maxHighIndex]) == 0xFFFFFFFF)
        return;

notSet:
    mat = DObjGetRotTransArray(obj);
    if (mat)
    {
        AnglesToQuat(pose->angles, baseQuat);
        memset(partBits, 0, sizeof(partBits));
        partBits[3] = 0x80000000;
        cg_s *cgameGlob = CG_GetLocalClientGlobals(R_GetLocalClientNum());
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
            partBits[3] = (partBits[3] << 31) | ((unsigned int)partBits[3] >> 1);
            ++mat;
        }
    }
}

void __cdecl CG_DoControllers(const cpose_t *pose, const DObj_s *obj, int *partBits)
{
    long double v6; // fp2
    int setPartBits[4];

    PROF_SCOPED("CG_DoControllers");

    DObjGetSetBones(obj, setPartBits);
    switch (pose->eType)
    {
    case ET_MG42:
        CG_mg42_DoControllers(pose, obj, partBits, v6);
        break;
    case ET_VEHICLE:
    case ET_VEHICLE_CORPSE:
        CG_Vehicle_DoControllers(pose, obj, partBits);
        break;
    case ET_ACTOR:
    case ET_ACTOR_CORPSE:
        CG_Actor_DoControllers(pose, obj, partBits);
        break;
    default:
        break;
    }
    CG_DoBaseOriginController(pose, obj, setPartBits);
    if (pose->isRagdoll && pose->ragdollHandle)
        Ragdoll_DoControllers(pose, (DObj_s*)obj, partBits);
}

DObjAnimMat *__cdecl CG_DObjCalcPose(const cpose_t *pose, const DObj_s *obj, int *partBits)
{
    DObjAnimMat *boneMatrix; // [sp+50h] [-40h] BYREF

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

