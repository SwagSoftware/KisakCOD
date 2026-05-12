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

void __cdecl CG_mg42_DoControllers(const cpose_t *pose, const DObj_s *obj, int *partBits)
{
    bool playerUsing; // r10
    float *turretViewAngles; // r30
    double v9; // fp30
    double v11; // fp0
    double v12; // fp31
    long double v13; // fp2
    double roll; // fp0
    double v15; // fp13
    float aimAngles[3]; // [sp+50h] [-70h] BYREF
    //float pitch; // [sp+54h] [-6Ch]
    //float v18; // [sp+58h] [-68h]
    float flashAngles[3]; // [sp+60h] [-60h] BYREF

    iassert(obj);

    playerUsing = pose->turret.playerUsing;
    aimAngles[0] = 0.0;
    aimAngles[1] = 0.0; 
    aimAngles[2] = 0.0;
    flashAngles[0] = 0.0;
    flashAngles[1] = 0.0;
    flashAngles[2] = 0.0;
    if (playerUsing)
    {
        iassert(pose->turret.viewAngles);
        turretViewAngles = (float*)pose->turret.viewAngles;
        v9 = ((*turretViewAngles - pose->angles[0]) * 0.0027777778);
        v11 = pose->angles[1];
        aimAngles[0] = (float)((float)v9 - floor((((*turretViewAngles - pose->angles[0]) * 0.0027777778f) + 0.5f))) * (float)360.0;
        v12 = (float)((float)(turretViewAngles[1] - (float)v11) * (float)0.0027777778);
        v13 = floor((((turretViewAngles[1] - (float)v11) * (float)0.0027777778) + (float)0.5));
        flashAngles[0] = 0.0;
        aimAngles[1] = (float)((float)v12 - (float)*(double *)&v13) * (float)360.0;
    }
    else
    {
        aimAngles[0] = (pose->turret.angles.pitch - pose->turret.barrelPitch);
        aimAngles[1] = pose->turret.angles.yaw;
        flashAngles[0] = pose->turret.barrelPitch;
    }
    DObjSetControlTagAngles((DObj_s*)obj, partBits, pose->turret.tag_aim, aimAngles);
    DObjSetControlTagAngles((DObj_s*)obj, partBits, pose->turret.tag_aim_animated, aimAngles);
    DObjSetControlTagAngles((DObj_s*)obj, partBits, pose->turret.tag_flash, flashAngles);
}

void CG_Vehicle_DoControllers(const cpose_t *pose, const DObj_s *obj, int *partBits)
{
    const float SHORT2ANGLE = 0.0054931641f;

    float bodyAngles[3];   // tag_body:   pitch (X), 0, roll (Z)
    float turretAngles[3]; // tag_turret: 0, yaw (Y), 0
    float barrelAngles[3]; // tag_barrel: barrelPitch (X), 0, 0
    float steerAngles[3];  // applied to front 2 wheels: 0, steerYaw (Y), 0

    float axis[3][3];
    float wheelOffset[3];

    iassert(obj);

    bodyAngles[0]   = (float)pose->vehicle.pitch       * SHORT2ANGLE;
    bodyAngles[1]   = 0.0f;
    bodyAngles[2]   = (float)pose->vehicle.roll        * SHORT2ANGLE;
    turretAngles[0] = 0.0f;
    turretAngles[1] = (float)pose->vehicle.yaw         * SHORT2ANGLE;
    turretAngles[2] = 0.0f;
    barrelAngles[0] = (float)pose->vehicle.barrelPitch * SHORT2ANGLE;
    barrelAngles[1] = 0.0f;
    barrelAngles[2] = 0.0f;
    steerAngles[0]  = 0.0f;
    steerAngles[1]  = (float)pose->vehicle.steerYaw    * SHORT2ANGLE;
    steerAngles[2]  = 0.0f;

    DObjSetLocalTag((DObj_s *)obj, partBits, pose->vehicle.tag_body,   vec3_origin, bodyAngles);
    DObjSetLocalTag((DObj_s *)obj, partBits, pose->vehicle.tag_turret, vec3_origin, turretAngles);
    DObjSetLocalTag((DObj_s *)obj, partBits, pose->vehicle.tag_barrel, vec3_origin, barrelAngles);

    // Per-wheel suspension/steering. The shared time value lives at pose+0x38
    // and is reachable via either the actor or vehicle union arm.
    const float height = pose->actor.height;

    AnglesToAxis(pose->angles, axis);

    const XModel *model = DObjGetModel(obj, 0);
    XModelNumBones(model);
    const DObjAnimMat *basePose = XModelGetBasePose(model);

    for (int i = 0; i < 6; ++i)
    {
        unsigned int boneIndex = pose->vehicle.wheelBoneIndex[i];
        if (boneIndex < 0xFEu && DObjSetRotTransIndex((DObj_s *)obj, partBits, boneIndex))
        {
            const float *baseTrans = basePose[boneIndex].trans;
            const float lx = baseTrans[0];
            const float ly = baseTrans[1];
            const float lz = baseTrans[2];

            // Vehicle-local -> world.
            float wx = lx * axis[0][0] + ly * axis[1][0] + lz * axis[2][0] + pose->origin[0];
            float wy = lx * axis[0][1] + ly * axis[1][1] + lz * axis[2][1] + pose->origin[1];
            float wz = lx * axis[0][2] + ly * axis[1][2] + lz * axis[2][2] + pose->origin[2];

            // Clamped suspension travel: prefer the spring-scaled drop, never
            // pierce 40 units below the rest height.
            const float fraction = (float)pose->vehicle.wheelFraction[i] * 0.000015259022f;
            const float drop     = (height + 40.0f) * fraction;
            const float floorDist = 40.0f - height;
            const float susp = (drop - floorDist >= 0.0f) ? drop : floorDist;

            // Raise +40 along vehicle-up, then lower by suspTravel.
            const float zStep = 40.0f - susp;
            wx += zStep * axis[2][0];
            wy += zStep * axis[2][1];
            wz += zStep * axis[2][2];

            // World -> vehicle-local using axis transpose (inverse of orthonormal).
            const float dx = wx - pose->origin[0];
            const float dy = wy - pose->origin[1];
            const float dz = wz - pose->origin[2];

            wheelOffset[0] = (dx * axis[0][0] + dy * axis[0][1] + dz * axis[0][2]) - lx;
            wheelOffset[1] = (dx * axis[1][0] + dy * axis[1][1] + dz * axis[1][2]) - ly;
            wheelOffset[2] = (dx * axis[2][0] + dy * axis[2][1] + dz * axis[2][2]) - lz;

            const float *wheelAngles = NULL;
            if (steerAngles[1] != 0.0f && (unsigned int)i <= 1u)
                wheelAngles = steerAngles;
            DObjSetLocalTagInternal(obj, wheelOffset, wheelAngles, boneIndex);
        }
    }
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
                    LocalMatrixTransformVectorQuatTrans(mat->trans, &animMat, origin);
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
    int setPartBits[4];

    PROF_SCOPED("CG_DoControllers");

    DObjGetSetBones(obj, setPartBits);
    switch (pose->eType)
    {
    case ET_MG42:
        CG_mg42_DoControllers(pose, obj, partBits);
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

