#ifndef KISAK_SP
#error This file is for SinglePlayer only
#endif

#include "cg_pose.h"

// local variable allocation has failed, the output may be wrong!
void __fastcall PitchToQuat(double pitch, float *quat, float *a3)
{
    double v4; // fp31
    long double v5; // fp2
    long double v6; // fp2
    long double v7; // fp2

    v4 = (float)((float)pitch * (float)0.0087266462);
    *a3 = 0.0;
    a3[2] = 0.0;
    *(double *)&v5 = v4;
    v6 = sin(v5);
    a3[1] = *(double *)&v6;
    *(double *)&v6 = v4;
    v7 = cos(v6);
    a3[3] = *(double *)&v7;
}

// local variable allocation has failed, the output may be wrong!
void __fastcall RollToQuat(double roll, float *quat, float *a3)
{
    double v4; // fp31
    long double v5; // fp2
    long double v6; // fp2
    long double v7; // fp2

    v4 = (float)((float)roll * (float)0.0087266462);
    a3[1] = 0.0;
    a3[2] = 0.0;
    *(double *)&v5 = v4;
    v6 = sin(v5);
    *a3 = *(double *)&v6;
    *(double *)&v6 = v4;
    v7 = cos(v6);
    a3[3] = *(double *)&v7;
}

void __fastcall LocalMatrixTransformVectorQuatTrans(const float *in, const DObjAnimMat *mat, float *out)
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

void __fastcall NormalizeQuatTrans(DObjAnimMat *mat)
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

void __fastcall CG_mg42_DoControllers(const cpose_t *pose, const DObj_s *obj, int *partBits, long double a4)
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
        v15 = (float)(pose->turret.$51809EA76892896F64281DFB626CE797::angles.pitch - pose->actor.roll);
        pitch = pose->actor.pitch;
        v16 = v15;
        v19[0] = roll;
    }
    DObjSetControlTagAngles(obj, partBits, pose->turret.tag_aim, &v16);
    DObjSetControlTagAngles(obj, partBits, pose->turret.tag_aim_animated, &v16);
    DObjSetControlTagAngles(obj, partBits, pose->turret.tag_flash, v19);
}

// local variable allocation has failed, the output may be wrong!
void __fastcall CG_Vehicle_DoControllers(const cpose_t *pose, const DObj_s *obj, int *partBits)
{
    __int64 v3; // r28
    __int128 v7; // r9 OVERLAPPED
    __int16 pitch; // r10
    unsigned int tag_body; // r5
    __int64 v10; // r11
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
    DWORD1(v7) = v44;
    LODWORD(v7) = pose->vehicle.barrelPitch;
    DWORD2(v7) = pose->vehicle.yaw;
    pitch = pose->vehicle.pitch;
    LODWORD(v3) = pose->vehicle.steerYaw;
    WORD1(v7) = pose->vehicle.roll;
    tag_body = pose->vehicle.tag_body;
    v45 = v7;
    v46 = *(_QWORD *)((char *)&v7 + 4);
    v38 = v3;
    LODWORD(v10) = pitch;
    HIDWORD(v10) = SWORD1(v7);
    v42[0] = 0.0;
    v42[2] = 0.0;
    v43[1] = 0.0;
    v43[2] = 0.0;
    v36 = v10;
    v37 = *(_QWORD *)((char *)&v7 - 4);
    v44[1] = 0.0;
    v39 = 0.0;
    v41 = 0.0;
    v44[2] = (float)*(__int64 *)((char *)&v7 - 4) * (float)0.0054931641;
    v44[0] = (float)v10 * (float)0.0054931641;
    v43[0] = (float)(__int64)v7 * (float)0.0054931641;
    v42[1] = (float)*(__int64 *)((char *)&v7 + 4) * (float)0.0054931641;
    v40 = (float)v3 * (float)0.0054931641;
    DObjSetLocalTag(obj, partBits, tag_body, vec3_origin, v44);
    DObjSetLocalTag(obj, partBits, pose->vehicle.tag_turret, vec3_origin, v42);
    DObjSetLocalTag(obj, partBits, pose->vehicle.tag_barrel, vec3_origin, v43);
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
    __asm { lvx128    v13, r0, r11 }
    _R11 = v47;
    _R7 = v51;
    __asm
    {
        vmr       v10, v0
        lvrx      v8, r27, r10
        vmr       v9, v0
        lvlx      v7, r0, r9
    }
    __asm { lvrx      v6, r27, r8 }
    __asm { lvx128    v127, r0, r11 }
    _R11 = &v50;
    __asm
    {
        vsldoi128 v126, v127, v13, 0xC
        vrlimi128 v127, v0, 1, 3
        lvlx      v5, r0, r7
        lvx128    v11, r0, r11
    }
    __asm
    {
        vsldoi128 v125, v13, v11, 8
        vrlimi128 v0, v11, 0xE, 1
    }
    _R11 = &g_one;
    __asm
    {
        vor       v11, v7, v8
        vor       v8, v5, v6
        vrlimi128 v126, v10, 1, 3
        vrlimi128 v125, v9, 1, 3
        vmr128    v123, v0
        lvx128    v13, r0, r11
    }
    __asm { vmrghw128 v0, v126, v13 }
    _R28 = &g_keepXYZ;
    __asm
    {
        lvx128    v12, r0, r28
        vand128   v124, v11, v12
        vmrglw128 v11, v127, v125
        vand128   v122, v8, v12
        vmrglw128 v12, v126, v13
        vmrghw128 v13, v127, v125
        vmrghw128 v119, v11, v12
        vmrghw128 v121, v13, v0
        vmrglw128 v120, v13, v0
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
                vmr128    v10, v123
            }
            _R11 = (int)BasePose[v28].trans;
            HIDWORD(_R10) = &v37;
            v38 = _R10;
            __asm
            {
                lvlx      v12, r0, r11
                lvrx      v13, r27, r11
            }
            _R11 = v52;
            __asm { vor       v13, v12, v13 }
            *(float *)&v37 = 40.0;
            __asm
            {
                vand      v0, v13, v0
                vspltw    v13, v0, 2
                vspltw    v12, v0, 1
                vspltw    v11, v0, 0
                vmaddfp128 v10, v125, v13, v10
                vmaddfp128 v10, v12, v126, v10
                vmaddfp128 v10, v11, v127, v10
            }
            _FP12 = (float)((float)((float)((float)((float)height + (float)40.0) * (float)_R10) * (float)0.000015259022)
                - (float)((float)40.0 - (float)height));
            LODWORD(_R10) = &v36;
            __asm { fsel      f0, f12, f13, f0 }
            *(float *)&v36 = -_FP0;
            __asm
            {
                lvlx      v13, r0, r9
                vspltw    v13, v13, 0
                lvlx      v12, r0, r10
                vspltw    v12, v12, 0
                vmaddfp128 v10, v13, v124, v10
                vmaddfp128 v10, v12, v124, v10
                vsubfp128 v13, v10, v122
                vspltw    v12, v13, 2
                vspltw    v11, v13, 1
                vspltw    v13, v13, 0
                vmulfp128 v12, v12, v119
                vmaddfp128 v12, v11, v120, v12
                vmaddfp128 v12, v13, v121, v12
                vsubfp    v0, v12, v0
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

void __fastcall CG_Actor_DoControllers(const cpose_t *pose, const DObj_s *obj, int *partBits)
{
    DObjAnimMat *RotTransArray; // r28
    float *v7; // r3
    int proneType; // r8
    float *v9; // r3
    float v10[4]; // [sp+50h] [-60h] BYREF
    float v11[4]; // [sp+60h] [-50h] BYREF
    float v12[16]; // [sp+70h] [-40h] BYREF

    if (!obj)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\cgame\\cg_pose.cpp", 169, 0, "%s", "obj");
    if (pose->actor.proneType)
    {
        RotTransArray = DObjGetRotTransArray(obj);
        if (RotTransArray)
        {
            v7 = (float *)DObjSetRotTransIndex(obj, partBits, 0);
            if (v7)
            {
                proneType = pose->actor.proneType;
                if (proneType == 2)
                {
                    PitchToQuat(pose->actor.pitch, v7, v11);
                    RollToQuat(pose->actor.roll, v9, v12);
                    QuatMultiply(v12, v11, RotTransArray->quat);
                }
                else
                {
                    if (proneType != 1)
                        MyAssertHandler(
                            "c:\\trees\\cod3\\cod3src\\src\\cgame\\cg_pose.cpp",
                            194,
                            0,
                            "%s\n\t(pose->actor.proneType) = %i",
                            "(pose->actor.proneType == CENT_ACTOR_PRONE_NORMAL)",
                            proneType);
                    PitchToQuat(pose->actor.pitch, v7, RotTransArray->quat);
                }
                v10[2] = pose->actor.height;
                v10[0] = 0.0;
                v10[1] = 0.0;
                DObjSetTrans(RotTransArray, v10);
            }
        }
    }
}

void __fastcall CG_DoBaseOriginController(const cpose_t *pose, const DObj_s *obj, int *setPartBits)
{
    unsigned int RootBoneCount; // r31
    int v7; // r29
    int v8; // r31
    unsigned int v9; // r9
    unsigned int v10; // r11
    int *v11; // r10
    DObjAnimMat *RotTransArray; // r30
    int LocalClientNum; // r8
    double v14; // fp30
    double v15; // fp29
    double v16; // fp28
    unsigned int v17; // r11
    unsigned int v18; // r10
    double v19; // fp12
    double v20; // fp13
    double v21; // fp0
    double v22; // fp7
    double v23; // fp0
    double v24; // fp12
    double v25; // fp11
    double v26; // fp8
    double v27; // fp6
    double v28; // fp5
    double v29; // fp4
    float v30; // [sp+50h] [-D0h] BYREF
    float v31; // [sp+54h] [-CCh]
    float v32; // [sp+58h] [-C8h]
    float v33; // [sp+60h] [-C0h] BYREF
    float v34; // [sp+64h] [-BCh]
    float v35; // [sp+68h] [-B8h]
    float v36; // [sp+6Ch] [-B4h]
    int v37; // [sp+70h] [-B0h]
    int v38; // [sp+74h] [-ACh]
    int v39; // [sp+78h] [-A8h]
    unsigned int v40[5]; // [sp+7Ch] [-A4h] BYREF
    DObjAnimMat v41[2]; // [sp+90h] [-90h] BYREF

    RootBoneCount = DObjGetRootBoneCount(obj);
    if (!RootBoneCount)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\cgame\\cg_pose.cpp", 225, 0, "%s", "rootBoneCount");
    v7 = RootBoneCount - 1;
    v9 = (int)(RootBoneCount - 1) >> 5;
    v8 = 0;
    v10 = 0;
    if (v9)
    {
        v11 = setPartBits;
        while (*v11 == -1)
        {
            ++v10;
            ++v11;
            if (v10 >= v9)
                goto LABEL_7;
        }
    }
    else
    {
    LABEL_7:
        if (((0xFFFFFFFF >> ((v7 & 0x1F) + 1)) | setPartBits[v9]) == 0xFFFFFFFF)
            return;
    }
    RotTransArray = DObjGetRotTransArray(obj);
    if (RotTransArray)
    {
        AnglesToQuat(pose->angles, &v33);
        v37 = 0;
        v38 = 0;
        v39 = 0;
        memset(&v40[1], 0, 12);
        v40[0] = 0x80000000;
        LocalClientNum = R_GetLocalClientNum();
        if (LocalClientNum)
            MyAssertHandler(
                "c:\\trees\\cod3\\cod3src\\src\\cgame\\cg_local.h",
                910,
                0,
                "%s\n\t(localClientNum) = %i",
                "(localClientNum == 0)",
                LocalClientNum);
        v14 = cgArray[0].refdef.viewOffset[0];
        v15 = cgArray[0].refdef.viewOffset[1];
        v16 = cgArray[0].refdef.viewOffset[2];
        if (v7 >= 0)
        {
            v17 = v40[0];
            do
            {
                v18 = 4 * (v8 >> 5);
                if ((setPartBits[v18 / 4] & v17) == 0)
                {
                    if (DObjSetRotTransIndex(obj, &v40[v18 / 0xFFFFFFFC], v8))
                    {
                        RotTransArray->quat[0] = v33;
                        RotTransArray->quat[1] = v34;
                        RotTransArray->quat[2] = v35;
                        RotTransArray->quat[3] = v36;
                        v19 = pose->origin[0];
                        v20 = pose->origin[1];
                        v21 = pose->origin[2];
                    }
                    else
                    {
                        v41[0].quat[0] = v33;
                        v41[0].quat[1] = v34;
                        v41[0].quat[2] = v35;
                        v41[0].quat[3] = v36;
                        DObjSetTrans(v41, pose->origin);
                        if ((float)((float)(v41[0].quat[0] * v41[0].quat[0])
                            + (float)((float)(v41[0].quat[1] * v41[0].quat[1])
                                + (float)((float)(v41[0].quat[3] * v41[0].quat[3])
                                    + (float)(v41[0].quat[2] * v41[0].quat[2])))) == 0.0)
                        {
                            v41[0].quat[3] = 1.0;
                            v41[0].transWeight = 2.0;
                        }
                        else
                        {
                            v41[0].transWeight = (float)2.0
                                / (float)((float)(v41[0].quat[0] * v41[0].quat[0])
                                    + (float)((float)(v41[0].quat[1] * v41[0].quat[1])
                                        + (float)((float)(v41[0].quat[3] * v41[0].quat[3])
                                            + (float)(v41[0].quat[2] * v41[0].quat[2]))));
                        }
                        v22 = v34;
                        v23 = RotTransArray->quat[2];
                        v24 = RotTransArray->quat[1];
                        v25 = RotTransArray->quat[3];
                        v26 = v35;
                        v27 = (float)((float)(RotTransArray->quat[0] * v35)
                            + (float)((float)(RotTransArray->quat[1] * v36) - (float)(RotTransArray->quat[2] * v33)));
                        v28 = (float)-(float)((float)(RotTransArray->quat[0] * v34)
                            - (float)((float)(RotTransArray->quat[2] * v36) + (float)(RotTransArray->quat[1] * v33)));
                        v29 = (float)((float)(RotTransArray->quat[3] * v36) - (float)(RotTransArray->quat[0] * v33));
                        RotTransArray->quat[0] = -(float)((float)(RotTransArray->quat[1] * v35)
                            - (float)((float)(RotTransArray->quat[3] * v33)
                                + (float)((float)(RotTransArray->quat[0] * v36)
                                    + (float)(RotTransArray->quat[2] * v34))));
                        RotTransArray->quat[1] = (float)((float)v25 * (float)v22) + (float)v27;
                        RotTransArray->quat[2] = (float)((float)v25 * (float)v26) + (float)v28;
                        RotTransArray->quat[3] = -(float)((float)((float)v23 * (float)v26)
                            - (float)-(float)((float)((float)v24 * (float)v22) - (float)v29));
                        LocalMatrixTransformVectorQuatTrans(RotTransArray->trans, v41, &v30);
                        v21 = v32;
                        v20 = v31;
                        v19 = v30;
                    }
                    v30 = (float)v19 - (float)v14;
                    v31 = (float)v20 - (float)v15;
                    v32 = (float)v21 - (float)v16;
                    DObjSetTrans(RotTransArray, &v30);
                    v17 = v40[0];
                }
                ++v8;
                v17 = (v17 >> 1) | (v17 << 31);
                ++RotTransArray;
                v40[0] = v17;
            } while (v8 <= v7);
        }
    }
}

void __fastcall CG_DoControllers(const cpose_t *pose, const DObj_s *obj, int *partBits)
{
    long double v6; // fp2
    int v7[12]; // [sp+50h] [-30h] BYREF

    //Profile_Begin(327);
    DObjGetSetBones(obj, v7);
    switch (pose->eType)
    {
    case 0xAu:
        CG_mg42_DoControllers(pose, obj, partBits, v6);
        break;
    case 0xBu:
    case 0xDu:
        CG_Vehicle_DoControllers(pose, obj, partBits);
        break;
    case 0xEu:
    case 0x10u:
        CG_Actor_DoControllers(pose, obj, partBits);
        break;
    default:
        break;
    }
    CG_DoBaseOriginController(pose, obj, v7);
    if (pose->isRagdoll && pose->ragdollHandle)
        Ragdoll_DoControllers(pose, obj, partBits);
    //Profile_EndInternal(0);
}

DObjAnimMat *__fastcall CG_DObjCalcPose(const cpose_t *pose, const DObj_s *obj, int *partBits)
{
    DObjAnimMat *v7; // [sp+50h] [-40h] BYREF

    if (!obj)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\cgame\\cg_pose.cpp", 333, 0, "%s", "obj");
    if (!pose)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\cgame\\cg_pose.cpp", 334, 0, "%s", "pose");
    if (!CL_DObjCreateSkelForBones(obj, partBits, &v7))
    {
        DObjCompleteHierarchyBits(obj, partBits);
        CG_DoControllers(pose, obj, partBits);
        DObjCalcSkel(obj, partBits);
    }
    return v7;
}

