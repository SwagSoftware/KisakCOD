#include "ragdoll.h"
#include <xanim/dobj.h>
#include <physics/phys_local.h>
#include <qcommon/threads.h>
#include <cgame_mp/cg_local_mp.h>
#include <xanim/dobj_utils.h>


//Line 53622:  0006 : 03066908       int ragdollTime          85816908     ragdoll_update.obj

float quatZRot[4] =
{
    0.0f, 0.0f, 1.0f, 0.0f
};

StateEnt stateEntries[6] =
{
    // EnterFunc, ExitFunc, UpdateFunc
    {Ragdoll_EnterDead, Ragdoll_ExitDead, NULL},
    {NULL, Ragdoll_ExitDObjWait, Ragdoll_UpdateDObjWait},
    {NULL, NULL, Ragdoll_UpdateVelocityCapture},
    {Ragdoll_EnterTunnelTest, NULL, NULL},
    {Ragdoll_EnterRunning, NULL, Ragdoll_UpdateRunning},
    {Ragdoll_EnterIdle, Ragdoll_ExitIdle, NULL}
};

int ragdollTime;

char __cdecl Ragdoll_ValidateBodyObj(RagdollBody *body)
{
    RagdollDef *def; // [esp+0h] [ebp-18h]
    DObj_s *obj; // [esp+4h] [ebp-14h]
    BoneDef *boneDef; // [esp+8h] [ebp-10h]
    unsigned __int8 boneIdx; // [esp+Fh] [ebp-9h] BYREF
    int i; // [esp+10h] [ebp-8h]
    BaseLerpBoneDef *lerpBoneDef; // [esp+14h] [ebp-4h]

    if (!body)
        MyAssertHandler(".\\ragdoll\\ragdoll_update.cpp", 367, 0, "%s", "body");
    def = Ragdoll_BodyDef(body);
    if (!def)
        MyAssertHandler(".\\ragdoll\\ragdoll_update.cpp", 370, 0, "%s", "def");
    obj = Ragdoll_BodyDObj(body);
    if (!obj)
        return 0;
    boneDef = def->boneDefs;
    i = 0;
    while (i < body->numBones)
    {
        boneIdx = 0;
        if (!DObjGetBoneIndex(obj, boneDef->animBoneNames[0], &boneIdx) || boneIdx == 255)
            return 0;
        boneIdx = 0;
        if (boneDef->animBoneNames[1] && !DObjGetBoneIndex(obj, boneDef->animBoneNames[1], &boneIdx) || boneIdx == 255)
            return 0;
        ++i;
        ++boneDef;
    }
    lerpBoneDef = def->baseLerpBoneDefs;
    i = 0;
    while (i < body->numLerpBones)
    {
        boneIdx = 0;
        if (!DObjGetBoneIndex(obj, lerpBoneDef->animBoneName, &boneIdx) || boneIdx == 255)
            return 0;
        ++i;
        ++lerpBoneDef;
    }
    return 1;
}

void __cdecl Ragdoll_SnapshotBaseLerpOffsets(RagdollBody *body)
{
    float v1; // [esp+20h] [ebp-118h]
    float v2; // [esp+24h] [ebp-114h]
    float v3; // [esp+28h] [ebp-110h]
    float v4; // [esp+2Ch] [ebp-10Ch]
    float result[3]; // [esp+30h] [ebp-108h] BYREF
    float v6; // [esp+3Ch] [ebp-FCh]
    float v7; // [esp+40h] [ebp-F8h]
    float v8; // [esp+44h] [ebp-F4h]
    float v9; // [esp+48h] [ebp-F0h]
    float v10; // [esp+4Ch] [ebp-ECh]
    RagdollDef *def; // [esp+50h] [ebp-E8h]
    BoneOrientation *boneOrientation; // [esp+54h] [ebp-E4h]
    DObj_s *obj; // [esp+58h] [ebp-E0h]
    DObjAnimMat parentAnimMat; // [esp+5Ch] [ebp-DCh] BYREF
    DObjAnimMat boneAnimMat; // [esp+7Ch] [ebp-BCh] BYREF
    float boneMat[4][3]; // [esp+9Ch] [ebp-9Ch] BYREF
    int parentIndex; // [esp+CCh] [ebp-6Ch]
    int i; // [esp+D0h] [ebp-68h]
    float invParentMat[4][3]; // [esp+D4h] [ebp-64h] BYREF
    LerpBone *bone; // [esp+104h] [ebp-34h]
    float relMat[4][3]; // [esp+108h] [ebp-30h] BYREF

    if (!body)
        MyAssertHandler(".\\ragdoll\\ragdoll_update.cpp", 658, 0, "%s", "body");
    def = Ragdoll_BodyDef(body);
    if (!def)
        MyAssertHandler(".\\ragdoll\\ragdoll_update.cpp", 661, 0, "%s", "def");
    obj = Ragdoll_BodyDObj(body);
    if (!obj)
        MyAssertHandler(".\\ragdoll\\ragdoll_update.cpp", 664, 0, "%s", "obj");
    bone = body->lerpBones;
    boneOrientation = body->lerpBoneOffsets;
    i = 0;
    while (i < def->numBaseLerpBones)
    {
        parentIndex = bone->parentBone;
        if (parentIndex >= body->numBones)
            MyAssertHandler(
                ".\\ragdoll\\ragdoll_update.cpp",
                672,
                0,
                "parentIndex doesn't index body->numBones\n\t%i not in [0, %i)",
                parentIndex,
                body->numBones);
        Ragdoll_GetDObjBaseBoneMatrix(obj, bone->animBone, &boneAnimMat);
        Ragdoll_GetDObjBaseBoneMatrix(obj, body->bones[parentIndex].animBones[0], &parentAnimMat);
        if ((LODWORD(parentAnimMat.quat[0]) & 0x7F800000) == 0x7F800000
            || (LODWORD(parentAnimMat.quat[1]) & 0x7F800000) == 0x7F800000
            || (LODWORD(parentAnimMat.quat[2]) & 0x7F800000) == 0x7F800000
            || (LODWORD(parentAnimMat.quat[3]) & 0x7F800000) == 0x7F800000)
        {
            MyAssertHandler(
                "c:\\trees\\cod3\\src\\ragdoll\\../xanim/xanim_public.h",
                536,
                0,
                "%s",
                "!IS_NAN((mat->quat)[0]) && !IS_NAN((mat->quat)[1]) && !IS_NAN((mat->quat)[2]) && !IS_NAN((mat->quat)[3])");
        }
        if ((LODWORD(parentAnimMat.transWeight) & 0x7F800000) == 0x7F800000)
            MyAssertHandler(
                "c:\\trees\\cod3\\src\\ragdoll\\../xanim/xanim_public.h",
                537,
                0,
                "%s",
                "!IS_NAN(mat->transWeight)");
        Vec3Scale(parentAnimMat.quat, parentAnimMat.transWeight, result);
        v9 = result[0] * parentAnimMat.quat[0];
        v2 = result[0] * parentAnimMat.quat[1];
        v7 = result[0] * parentAnimMat.quat[2];
        v10 = result[0] * parentAnimMat.quat[3];
        v1 = result[1] * parentAnimMat.quat[1];
        v8 = result[1] * parentAnimMat.quat[2];
        v6 = result[1] * parentAnimMat.quat[3];
        v3 = result[2] * parentAnimMat.quat[2];
        v4 = result[2] * parentAnimMat.quat[3];
        invParentMat[0][0] = 1.0 - (v1 + v3);
        invParentMat[0][1] = v2 - v4;
        invParentMat[0][2] = v7 + v6;
        invParentMat[1][0] = v2 + v4;
        invParentMat[1][1] = 1.0 - (v9 + v3);
        invParentMat[1][2] = v8 - v10;
        invParentMat[2][0] = v7 - v6;
        invParentMat[2][1] = v8 + v10;
        invParentMat[2][2] = 1.0 - (v9 + v1);
        invParentMat[3][0] = -(parentAnimMat.trans[0] * invParentMat[0][0]
            + parentAnimMat.trans[1] * invParentMat[1][0]
            + parentAnimMat.trans[2] * invParentMat[2][0]);
        invParentMat[3][1] = -(parentAnimMat.trans[0] * invParentMat[0][1]
            + parentAnimMat.trans[1] * invParentMat[1][1]
            + parentAnimMat.trans[2] * invParentMat[2][1]);
        invParentMat[3][2] = -(parentAnimMat.trans[0] * invParentMat[0][2]
            + parentAnimMat.trans[1] * invParentMat[1][2]
            + parentAnimMat.trans[2] * invParentMat[2][2]);
        Ragdoll_AnimMatToMat43(&boneAnimMat, boneMat);
        MatrixMultiply43(boneMat, invParentMat, relMat);
        AxisToQuat(relMat, boneOrientation->orientation);
        boneOrientation->origin[0] = relMat[3][0];
        boneOrientation->origin[1] = relMat[3][1];
        boneOrientation->origin[2] = relMat[3][2];
        ++i;
        ++bone;
        ++boneOrientation;
    }
}

void __cdecl Ragdoll_GetDObjBaseBoneMatrix(DObj_s *obj, unsigned __int8 boneIndex, DObjAnimMat *outMat)
{
    if (!obj)
        MyAssertHandler(".\\ragdoll\\ragdoll_update.cpp", 86, 0, "%s", "obj");
    DObjGetBasePoseMatrix(obj, boneIndex, outMat);
}

void __cdecl Ragdoll_AnimMatToMat43(const DObjAnimMat *mat, float (*out)[3])
{
    float v2; // [esp+24h] [ebp-30h]
    float v3; // [esp+28h] [ebp-2Ch]
    float v4; // [esp+2Ch] [ebp-28h]
    float v5; // [esp+30h] [ebp-24h]
    float result[3]; // [esp+34h] [ebp-20h] BYREF
    float v7; // [esp+40h] [ebp-14h]
    float v8; // [esp+44h] [ebp-10h]
    float v9; // [esp+48h] [ebp-Ch]
    float v10; // [esp+4Ch] [ebp-8h]
    float v11; // [esp+50h] [ebp-4h]

    if ((COERCE_UNSIGNED_INT(mat->quat[0]) & 0x7F800000) == 0x7F800000
        || (COERCE_UNSIGNED_INT(mat->quat[1]) & 0x7F800000) == 0x7F800000
        || (COERCE_UNSIGNED_INT(mat->quat[2]) & 0x7F800000) == 0x7F800000
        || (COERCE_UNSIGNED_INT(mat->quat[3]) & 0x7F800000) == 0x7F800000)
    {
        MyAssertHandler(
            "c:\\trees\\cod3\\src\\ragdoll\\../xanim/xanim_public.h",
            432,
            0,
            "%s",
            "!IS_NAN((mat->quat)[0]) && !IS_NAN((mat->quat)[1]) && !IS_NAN((mat->quat)[2]) && !IS_NAN((mat->quat)[3])");
    }
    if ((COERCE_UNSIGNED_INT(mat->transWeight) & 0x7F800000) == 0x7F800000)
        MyAssertHandler("c:\\trees\\cod3\\src\\ragdoll\\../xanim/xanim_public.h", 433, 0, "%s", "!IS_NAN(mat->transWeight)");
    Vec3Scale(mat->quat, mat->transWeight, result);
    v10 = result[0] * mat->quat[0];
    v3 = result[0] * mat->quat[1];
    v8 = result[0] * mat->quat[2];
    v11 = result[0] * mat->quat[3];
    v2 = result[1] * mat->quat[1];
    v9 = result[1] * mat->quat[2];
    v7 = result[1] * mat->quat[3];
    v4 = result[2] * mat->quat[2];
    v5 = result[2] * mat->quat[3];
    (*out)[0] = 1.0 - (v2 + v4);
    (*out)[1] = v3 + v5;
    (*out)[2] = v8 - v7;
    (*out)[3] = v3 - v5;
    (*out)[4] = 1.0 - (v10 + v4);
    (*out)[5] = v9 + v11;
    (*out)[6] = v8 + v7;
    (*out)[7] = v9 - v11;
    (*out)[8] = 1.0 - (v10 + v2);
    (*out)[9] = mat->trans[0];
    (*out)[10] = mat->trans[1];
    (*out)[11] = mat->trans[2];
}

char __cdecl Ragdoll_CreateBodyPhysics(RagdollBody *body)
{
    if (!body)
        MyAssertHandler(".\\ragdoll\\ragdoll_update.cpp", 740, 0, "%s", "body");
    if (body->state <= BS_VELOCITY_CAPTURE)
        MyAssertHandler(".\\ragdoll\\ragdoll_update.cpp", 741, 0, "%s", "body->state > RAGDOLL_DOBJ_VALID_STATE");
    if (!body)
        MyAssertHandler("c:\\trees\\cod3\\src\\ragdoll\\ragdoll.h", 285, 0, "%s", "body");
    if (body->references <= 0)
        MyAssertHandler(".\\ragdoll\\ragdoll_update.cpp", 742, 0, "%s", "Ragdoll_BodyInUse( body )");
    if (Ragdoll_CreatePhysObjs(body))
    {
        if (Ragdoll_CreatePhysJoints(body))
        {
            return 1;
        }
        else
        {
            Ragdoll_DestroyPhysObjs(body);
            Ragdoll_DestroyPhysJoints(body);
            return 0;
        }
    }
    else
    {
        Ragdoll_DestroyPhysObjs(body);
        return 0;
    }
}

char __cdecl Ragdoll_CreatePhysJoints(RagdollBody *body)
{
    RagdollDef *def; // [esp+0h] [ebp-8h]
    int i; // [esp+4h] [ebp-4h]

    if (!body)
        MyAssertHandler(".\\ragdoll\\ragdoll_update.cpp", 580, 0, "%s", "body");
    def = Ragdoll_BodyDef(body);
    if (!def)
        MyAssertHandler(".\\ragdoll\\ragdoll_update.cpp", 583, 0, "%s", "def");
    for (i = 0; i < body->numJoints; ++i)
    {
        if (!Ragdoll_CreatePhysJoint(body, &def->jointDefs[i], &body->joints[i]))
            return 0;
    }
    return 1;
}

char __cdecl Ragdoll_CreatePhysJoint(RagdollBody *body, JointDef *jointDef, Joint *joint)
{
    JointType type; // [esp+10h] [ebp-98h]
    Bone *parentBone; // [esp+18h] [ebp-90h]
    float limitAxes[3][3]; // [esp+1Ch] [ebp-8Ch] BYREF
    float tAxis[3][3]; // [esp+40h] [ebp-68h] BYREF
    float anchor[3]; // [esp+64h] [ebp-44h] BYREF
    float fric[3]; // [esp+70h] [ebp-38h] BYREF
    int i; // [esp+7Ch] [ebp-2Ch]
    float axis[3][3]; // [esp+80h] [ebp-28h] BYREF
    Bone *bone; // [esp+A4h] [ebp-4h]

    if (!body)
        MyAssertHandler(".\\ragdoll\\ragdoll_update.cpp", 524, 0, "%s", "body");
    if (!jointDef)
        MyAssertHandler(".\\ragdoll\\ragdoll_update.cpp", 525, 0, "%s", "jointDef");
    if (!joint)
        MyAssertHandler(".\\ragdoll\\ragdoll_update.cpp", 526, 0, "%s", "joint");
    if ((unsigned int)jointDef->bone >= body->numBones)
        MyAssertHandler(
            ".\\ragdoll\\ragdoll_update.cpp",
            527,
            0,
            "jointDef->bone doesn't index body->numBones\n\t%i not in [0, %i)",
            jointDef->bone,
            body->numBones);
    bone = &body->bones[jointDef->bone];
    if (bone->parentBone == -1)
        parentBone = 0;
    else
        parentBone = &body->bones[bone->parentBone];
    if (!bone->rigidBody)
        return 0;
    Phys_ObjGetPosition((dxBody *)bone->rigidBody, anchor, tAxis);
    AxisTranspose(tAxis, axis);
    if (!jointDef->numLimitAxes)
    {
        jointDef->limitAxes[0][0] = 0.0;
        jointDef->limitAxes[0][1] = 0.0;
        jointDef->limitAxes[0][2] = 1.0;
        jointDef->axisFriction[0] = 0.0;
        jointDef->minAngles[0] = -1.5707964;
        jointDef->maxAngles[0] = 1.5707964;
        ++jointDef->numLimitAxes;
    }
    for (i = 0; i < jointDef->numLimitAxes; ++i)
        Vec3Rotate(jointDef->limitAxes[i], axis, limitAxes[i]);
    Vec3Scale(jointDef->axisFriction, 15.0, fric);
    type = jointDef->type;
    if (type == RAGDOLL_JOINT_HINGE)
    {
        joint->joint = Phys_CreateHinge(
            PHYS_WORLD_RAGDOLL,
            bone->rigidBody,
            parentBone->rigidBody,
            anchor,
            limitAxes[0],
            0.0,
            fric[0],
            jointDef->minAngles[0],
            jointDef->maxAngles[0]);
        joint->joint2 = 0;
    }
    else if (type == RAGDOLL_JOINT_SWIVEL)
    {
        joint->joint = Phys_CreateBallAndSocket(
            PHYS_WORLD_RAGDOLL,
            bone->rigidBody,
            parentBone->rigidBody,
            anchor);
        joint->joint2 = Phys_CreateAngularMotor(
            PHYS_WORLD_RAGDOLL,
            bone->rigidBody,
            parentBone->rigidBody,
            jointDef->numLimitAxes,
            limitAxes,
            vec3_origin,
            fric,
            jointDef->minAngles,
            jointDef->maxAngles);
    }
    return 1;
}

char __cdecl Ragdoll_CreatePhysObjs(RagdollBody *body)
{
    RagdollDef *def; // [esp+0h] [ebp-8h]
    int i; // [esp+4h] [ebp-4h]

    if (!body)
        MyAssertHandler(".\\ragdoll\\ragdoll_update.cpp", 598, 0, "%s", "body");
    def = Ragdoll_BodyDef(body);
    if (!def)
        MyAssertHandler(".\\ragdoll\\ragdoll_update.cpp", 601, 0, "%s", "def");
    for (i = 0; i < body->numBones; ++i)
    {
        if (!Ragdoll_CreatePhysObj(body, &def->boneDefs[i], &body->bones[i]))
            return 0;
    }
    return 1;
}

char __cdecl Ragdoll_CreatePhysObj(RagdollBody *body, BoneDef *boneDef, Bone *bone)
{
    float v4; // [esp+Ch] [ebp-E0h]
    float halfHeight; // [esp+10h] [ebp-DCh]
    PhysicsGeomType geomType; // [esp+14h] [ebp-D8h]
    float v7; // [esp+18h] [ebp-D4h]
    float v8; // [esp+24h] [ebp-C8h]
    float diff[3]; // [esp+2Ch] [ebp-C0h] BYREF
    float qRot[4]; // [esp+3Ch] [ebp-B0h] BYREF
    RagdollDef *def; // [esp+4Ch] [ebp-A0h]
    DObj_s *obj; // [esp+50h] [ebp-9Ch]
    float b1Origin[3]; // [esp+54h] [ebp-98h] BYREF
    float b0Origin[3]; // [esp+60h] [ebp-8Ch] BYREF
    float mins[3]; // [esp+6Ch] [ebp-80h] BYREF
    float b0Quat[4]; // [esp+78h] [ebp-74h] BYREF
    float invPoseAxis[3][3]; // [esp+88h] [ebp-64h] BYREF
    float maxs[3]; // [esp+ACh] [ebp-40h] BYREF
    const cpose_t *pose; // [esp+B8h] [ebp-34h]
    PhysPreset preset; // [esp+BCh] [ebp-30h] BYREF
    int boneIdx; // [esp+E8h] [ebp-4h]

    if (!body)
        MyAssertHandler(".\\ragdoll\\ragdoll_update.cpp", 415, 0, "%s", "body");
    if (!boneDef)
        MyAssertHandler(".\\ragdoll\\ragdoll_update.cpp", 416, 0, "%s", "boneDef");
    if (!bone)
        MyAssertHandler(".\\ragdoll\\ragdoll_update.cpp", 417, 0, "%s", "bone");
    def = Ragdoll_BodyDef(body);
    if (!def)
        MyAssertHandler(".\\ragdoll\\ragdoll_update.cpp", 420, 0, "%s", "def");
    obj = Ragdoll_BodyDObj(body);
    if (!obj)
        return 0;
    if (!def->bound)
        MyAssertHandler(".\\ragdoll\\ragdoll_update.cpp", 427, 0, "%s", "def->bound");
    if (!boneDef->animBoneNames[0])
        MyAssertHandler(".\\ragdoll\\ragdoll_update.cpp", 428, 0, "%s", "boneDef->animBoneNames[0]");
    bone->animBones[0] = 0;
    bone->animBones[1] = 0;
    if (!DObjGetBoneIndex(obj, boneDef->animBoneNames[0], bone->animBones) || bone->animBones[0] == 255)
        return 0;
    if (boneDef->animBoneNames[1])
    {
        if (!DObjGetBoneIndex(obj, boneDef->animBoneNames[1], &bone->animBones[1]) || bone->animBones[0] == 255)
            return 0;
    }
    else
    {
        bone->animBones[1] = 0;
    }
    pose = (const cpose_t *)Ragdoll_BodyPose(body);
    if (!pose)
        MyAssertHandler(".\\ragdoll\\ragdoll_update.cpp", 447, 0, "%s", "pose");
    Ragdoll_PoseInvAxis(pose, invPoseAxis);
    Ragdoll_GetDObjBaseBoneOriginQuat(
        body->localClientNum,
        obj,
        pose->origin,
        invPoseAxis,
        bone->animBones[0],
        b0Origin,
        qRot);
    if (boneDef->mirror)
    {
        Ragdoll_QuatMul(qRot, quatZRot, b0Quat);
    }
    else
    {
        b0Quat[0] = qRot[0];
        b0Quat[1] = qRot[1];
        b0Quat[2] = qRot[2];
        b0Quat[3] = qRot[3];
    }
    if (bone->animBones[1])
    {
        Ragdoll_GetDObjBaseBoneOrigin(body->localClientNum, obj, pose->origin, invPoseAxis, bone->animBones[1], b1Origin);
    }
    else
    {
        b1Origin[0] = b0Origin[0];
        b1Origin[1] = b0Origin[1];
        b1Origin[2] = b0Origin[2];
    }
    boneIdx = bone - body->bones;
    preset.bounce = 0.0;
    preset.bulletForceScale = 1.0;
    preset.explosiveForceScale = 1.0;
    preset.friction = boneDef->friction;
    preset.mass = boneDef->mass;
    preset.name = "ragdoll_bone";
    preset.type = 0;
    preset.sndAliasPrefix = 0;
    bone->parentBone = boneDef->parentBone;
    bone->rigidBody = Phys_ObjCreate(PHYS_WORLD_RAGDOLL, b0Origin, b0Quat, (float *)vec3_origin, &preset);
    if (bone->rigidBody)
    {
        Vec3Sub(b1Origin, b0Origin, diff);
        bone->length = Vec3Length(diff);
        v8 = bone->length * 0.5;
        bone->center[0] = v8;
        bone->center[1] = 0.0;
        bone->center[2] = 0.0;
        geomType = boneDef->geomType;
        switch (geomType)
        {
        case PHYS_GEOM_BOX:
            v7 = -boneDef->radius;
            mins[0] = 0.0;
            mins[1] = v7;
            mins[2] = v7;
            maxs[0] = -(float)0.0;
            maxs[1] = -v7;
            maxs[2] = maxs[1];
            maxs[0] = bone->length;
            Phys_ObjAddGeomBox(PHYS_WORLD_RAGDOLL, (dxBody *)bone->rigidBody, mins, maxs);
            break;
        case PHYS_GEOM_CYLINDER:
            halfHeight = bone->length * 0.5;
            Phys_ObjAddGeomCylinderDirection(
                PHYS_WORLD_RAGDOLL,
                bone->rigidBody,
                0,
                boneDef->radius,
                halfHeight,
                bone->center);
            break;
        case PHYS_GEOM_CAPSULE:
            v4 = bone->length * 0.5;
            Phys_ObjAddGeomCapsule(PHYS_WORLD_RAGDOLL, (dxBody *)bone->rigidBody, 0, boneDef->radius, v4, bone->center);
            break;
        default:
            Com_PrintWarning(14, "Ragdoll: Unknown bone geom type %d\n", boneDef->geomType);
            break;
        }
        return 1;
    }
    else
    {
        Com_PrintWarning(14, "Ragdoll: Failed to create rigid body\n");
        return 0;
    }
}

char __cdecl Ragdoll_GetDObjBaseBoneOrigin(
    int localClientNum,
    DObj_s *obj,
    const float *offset,
    const mat3x3 &axis,
    unsigned __int8 boneIndex,
    float *origin)
{
    DObjAnimMat mat; // [esp+0h] [ebp-20h] BYREF

    if (!obj)
        MyAssertHandler(".\\ragdoll\\ragdoll_update.cpp", 96, 0, "%s", "obj");
    Ragdoll_GetDObjBaseBoneMatrix(obj, boneIndex, &mat);
    Vec3Rotate(mat.trans, axis, origin);
    Vec3Add(offset, origin, origin);
    return 1;
}

char __cdecl Ragdoll_GetDObjBaseBoneOriginQuat(
    int localClientNum,
    DObj_s *obj,
    const float *offset,
    const mat3x3 &axis,
    unsigned __int8 boneIndex,
    float *origin,
    float *quat)
{
    DObjAnimMat mat; // [esp+0h] [ebp-30h] BYREF
    float orientation[4]; // [esp+20h] [ebp-10h] BYREF

    if (!obj)
        MyAssertHandler(".\\ragdoll\\ragdoll_update.cpp", 112, 0, "%s", "obj");
    Ragdoll_GetDObjBaseBoneMatrix(obj, boneIndex, &mat);
    Ragdoll_Mat33ToQuat(axis, orientation);
    Ragdoll_QuatMul(orientation, mat.quat, quat);
    Vec3Rotate(mat.trans, axis, origin);
    Vec3Add(offset, origin, origin);
    return 1;
}

void __cdecl Ragdoll_PoseInvAxis(const cpose_t *pose, mat3x3 &invAxis)
{
    float axis[3][3]; // [esp+0h] [ebp-24h] BYREF

    if (!pose)
        MyAssertHandler(".\\ragdoll\\ragdoll_update.cpp", 130, 0, "%s", "pose");
    AnglesToAxis(pose->angles, axis);
    AxisTranspose(axis, invAxis);
}

void __cdecl Ragdoll_DestroyPhysJoints(RagdollBody *body)
{
    int i; // [esp+0h] [ebp-4h]

    if (!body)
        MyAssertHandler(".\\ragdoll\\ragdoll_update.cpp", 615, 0, "%s", "body");
    for (i = 0; i < body->numJoints; ++i)
    {
        if (body->joints[i].joint)
            Phys_JointDestroy(PHYS_WORLD_RAGDOLL, (dxJointHinge *)body->joints[i].joint);
        if (body->joints[i].joint2)
            Phys_JointDestroy(PHYS_WORLD_RAGDOLL, (dxJointHinge *)body->joints[i].joint2);
    }
    memset((unsigned __int8 *)body->joints, 0, sizeof(body->joints));
}

void __cdecl Ragdoll_DestroyPhysObjs(RagdollBody *body)
{
    int i; // [esp+0h] [ebp-4h]

    if (!body)
        MyAssertHandler(".\\ragdoll\\ragdoll_update.cpp", 633, 0, "%s", "body");
    for (i = 0; i < body->numBones; ++i)
    {
        if (body->bones[i].rigidBody)
            Phys_ObjDestroy(PHYS_WORLD_RAGDOLL, (dxBody *)body->bones[i].rigidBody);
        body->bones[i].rigidBody = 0;
    }
}

void __cdecl Ragdoll_RemoveBodyPhysics(RagdollBody *body)
{
    if (!body)
        MyAssertHandler(".\\ragdoll\\ragdoll_update.cpp", 763, 0, "%s", "body");
    Ragdoll_DestroyPhysJoints(body);
    Ragdoll_DestroyPhysObjs(body);
}

bool __cdecl Ragdoll_ValidatePrecalcBoneDef(RagdollDef *def, BoneDef *bone)
{
    float v3; // [esp+0h] [ebp-8h]

    if (!def)
        MyAssertHandler(".\\ragdoll\\ragdoll_update.cpp", 815, 0, "%s", "def");
    if (!bone)
        MyAssertHandler(".\\ragdoll\\ragdoll_update.cpp", 816, 0, "%s", "bone");
    v3 = fabs(bone->mass);
    return v3 >= 0.000001;
}

void __cdecl Ragdoll_ClosestPointsTwoSegs(float (*s0)[3], float (*s1)[3], float *t0, float *t1, float *direction)
{
    double v5; // [esp+0h] [ebp-70h]
    float v6; // [esp+8h] [ebp-68h]
    double v7; // [esp+Ch] [ebp-64h]
    float v8; // [esp+14h] [ebp-5Ch]
    float sc; // [esp+18h] [ebp-58h]
    float tc; // [esp+1Ch] [ebp-54h]
    float D; // [esp+20h] [ebp-50h]
    int c; // [esp+24h] [ebp-4Ch]
    float tN; // [esp+28h] [ebp-48h]
    float sD; // [esp+2Ch] [ebp-44h]
    float uwDot; // [esp+30h] [ebp-40h]
    float sN; // [esp+34h] [ebp-3Ch]
    float uvDot; // [esp+38h] [ebp-38h]
    float vwDot; // [esp+3Ch] [ebp-34h]
    float tD; // [esp+40h] [ebp-30h]
    float u[3]; // [esp+44h] [ebp-2Ch] BYREF
    float uuDot; // [esp+50h] [ebp-20h]
    float vvDot; // [esp+54h] [ebp-1Ch]
    float v[3]; // [esp+58h] [ebp-18h] BYREF
    float w[3]; // [esp+64h] [ebp-Ch] BYREF

    if (!t0)
        MyAssertHandler(".\\ragdoll\\ragdoll_update.cpp", 847, 0, "%s", "t0");
    if (!t1)
        MyAssertHandler(".\\ragdoll\\ragdoll_update.cpp", 848, 0, "%s", "t1");
    Vec3Sub(&(*s0)[3], (const float *)s0, u);
    Vec3Sub(&(*s1)[3], (const float *)s1, v);
    Vec3Sub((const float *)s0, (const float *)s1, w);
    uuDot = Vec3Dot(u, u);
    uvDot = Vec3Dot(u, v);
    vvDot = Vec3Dot(v, v);
    uwDot = Vec3Dot(u, w);
    vwDot = Vec3Dot(v, w);
    D = uuDot * vvDot - uvDot * uvDot;
    tD = D;
    sD = D;
    if (D >= 0.000001)
    {
        sN = uvDot * vwDot - vvDot * uwDot;
        tN = uuDot * vwDot - uvDot * uwDot;
        if (sN >= 0.0)
        {
            if (D < (double)sN)
            {
                sN = uuDot * vvDot - uvDot * uvDot;
                tN = vwDot + uvDot;
                tD = vvDot;
            }
        }
        else
        {
            sN = 0.0;
            tN = vwDot;
            tD = vvDot;
        }
    }
    else
    {
        sN = 0.0;
        sD = 1.0;
        tN = vwDot;
        tD = vvDot;
    }
    if (tN >= 0.0)
    {
        if (tD < (double)tN)
        {
            tN = tD;
            if (uvDot - uwDot >= 0.0)
            {
                if (uuDot >= uvDot - uwDot)
                {
                    sN = uvDot - uwDot;
                    sD = uuDot;
                }
                else
                {
                    sN = sD;
                }
            }
            else
            {
                sN = 0.0;
            }
        }
    }
    else
    {
        tN = 0.0;
        if (-uwDot >= 0.0)
        {
            if (uuDot >= -uwDot)
            {
                sN = -uwDot;
                sD = uuDot;
            }
            else
            {
                sN = sD;
            }
        }
        else
        {
            sN = 0.0;
        }
    }
    v8 = fabs(sN);
    if (v8 >= 0.00009999999747378752)
        v7 = sN / sD;
    else
        v7 = 0.0;
    sc = v7;
    v6 = fabs(tN);
    if (v6 >= 0.00009999999747378752)
        v5 = tN / tD;
    else
        v5 = 0.0;
    tc = v5;
    *t0 = sc;
    *t1 = tc;
    for (c = 0; c < 3; ++c)
        direction[c] = sc * u[c] + w[c] - tc * v[c];
}

void __cdecl Ragdoll_GenerateAllSelfCollisionContacts()
{
    float diff[3]; // [esp+14h] [ebp-88h] BYREF
    float t0; // [esp+20h] [ebp-7Ch] BYREF
    RagdollDef *def; // [esp+24h] [ebp-78h]
    float t1; // [esp+28h] [ebp-74h] BYREF
    float s1[2][3]; // [esp+2Ch] [ebp-70h] BYREF
    RagdollBody *body; // [esp+44h] [ebp-58h]
    SelfPairDef *selfPair; // [esp+48h] [ebp-54h]
    float pi0[3]; // [esp+4Ch] [ebp-50h] BYREF
    float pi1[3]; // [esp+58h] [ebp-44h] BYREF
    float segDistance; // [esp+64h] [ebp-38h]
    int ragdollIdx; // [esp+68h] [ebp-34h]
    float radius0; // [esp+6Ch] [ebp-30h]
    float s0[2][3]; // [esp+70h] [ebp-2Ch] BYREF
    float direction[3]; // [esp+88h] [ebp-14h] BYREF
    float radius1; // [esp+94h] [ebp-8h]
    int pairIdx; // [esp+98h] [ebp-4h]

    for (ragdollIdx = 0; ragdollIdx < 32; ++ragdollIdx)
    {
        body = &ragdollBodies[ragdollIdx];
        if (Ragdoll_BodyHasPhysics(body))
        {
            def = Ragdoll_BodyDef(body);
            for (pairIdx = 0; pairIdx < def->numSelfPairs; ++pairIdx)
            {
                selfPair = &def->selfPairDefs[pairIdx];
                Ragdoll_GenBoneCapsuleSegments(body, selfPair->bones, s0, s1);
                Ragdoll_ClosestPointsTwoSegs(s0, s1, &t0, &t1, direction);
                Vec3Lerp(s0[0], s0[1], t0, pi0);
                Vec3Lerp(s1[0], s1[1], t1, pi1);
                Vec3Sub(pi1, pi0, diff);
                segDistance = Vec3Length(diff);
                radius0 = def->boneDefs[selfPair->bones[0]].radius * ragdoll_self_collision_scale->current.value;
                radius1 = def->boneDefs[selfPair->bones[1]].radius * ragdoll_self_collision_scale->current.value;
                if (segDistance <= radius0 + radius1)
                    Ragdoll_AddSelfContact(body, selfPair->bones, radius0, radius1, pi0, pi1);
            }
        }
    }
}

void __cdecl Ragdoll_GenBoneCapsuleSegments(RagdollBody *body, unsigned __int8 *bones, float (*s0)[3], float (*s1)[3])
{
    if ((unsigned int)*bones >= body->numBones)
        MyAssertHandler(
            ".\\ragdoll\\ragdoll_update.cpp",
            957,
            0,
            "bones[0] doesn't index body->numBones\n\t%i not in [0, %i)",
            *bones,
            body->numBones);
    if ((unsigned int)bones[1] >= body->numBones)
        MyAssertHandler(
            ".\\ragdoll\\ragdoll_update.cpp",
            958,
            0,
            "bones[1] doesn't index body->numBones\n\t%i not in [0, %i)",
            bones[1],
            body->numBones);
    Ragdoll_GenBoneCapsuleSegment(&body->bones[*bones], s0);
    Ragdoll_GenBoneCapsuleSegment(&body->bones[bones[1]], s1);
}

void __cdecl Ragdoll_GenBoneCapsuleSegment(Bone *bone, float (*seg)[3])
{
    float boneMtx[3][3]; // [esp+Ch] [ebp-24h] BYREF

    Phys_ObjGetPosition((dxBody *)bone->rigidBody, (float *)seg, boneMtx);
    Vec3Mad((const float *)seg, bone->length, boneMtx[0], &(*seg)[3]);
}

void __cdecl Ragdoll_AddSelfContact(
    RagdollBody *body,
    unsigned __int8 *bones,
    float radius0,
    float radius1,
    float *point0,
    float *point1)
{
    float scale; // [esp+Ch] [ebp-40h]
    RagdollDef *def; // [esp+10h] [ebp-3Ch]
    float contactDir[3]; // [esp+14h] [ebp-38h] BYREF
    float dist; // [esp+20h] [ebp-2Ch]
    float halfCorrection; // [esp+24h] [ebp-28h]
    PhysContact contact; // [esp+28h] [ebp-24h] BYREF

    def = Ragdoll_BodyDef(body);
    Vec3Sub(point1, point0, contactDir);
    if (Vec3LengthSq(contactDir) >= 0.002000000094994903)
    {
        dist = Vec3NormalizeTo(contactDir, contact.normal);
        halfCorrection = (radius0 + radius1 - dist) * 0.5;
        if (halfCorrection < -0.000001)
            MyAssertHandler(".\\ragdoll\\ragdoll_update.cpp", 984, 0, "%s", "halfCorrection >= -ZERO_EPSILON");
        scale = radius0 - halfCorrection;
        Vec3Mad(point0, scale, contact.normal, contact.pos);
        contact.depth = halfCorrection;
        contact.bounce = 0.0;
        contact.friction = def->boneDefs[*bones].friction;
        Phys_AddCollisionContact(
            PHYS_WORLD_RAGDOLL,
            &contact,
            (dxBody *)body->bones[bones[1]].rigidBody,
            (dxBody *)body->bones[*bones].rigidBody);
    }
}

void __cdecl Ragdoll_ExplosionEvent(
    int localClientNum,
    bool isCylinder,
    const float *origin,
    float innerRadius,
    float outerRadius,
    const float *impulse,
    float inScale)
{
    double v7; // st7
    double v8; // st7
    double v9; // st7
    float v10; // [esp+8h] [ebp-80h]
    float v12; // [esp+28h] [ebp-60h]
    float v13; // [esp+2Ch] [ebp-5Ch]
    float distSqra; // [esp+30h] [ebp-58h]
    float distSqr; // [esp+30h] [ebp-58h]
    float delta[3]; // [esp+34h] [ebp-54h] BYREF
    float torsoPos[3]; // [esp+40h] [ebp-48h] BYREF
    float boneScale; // [esp+4Ch] [ebp-3Ch]
    RagdollBody *body; // [esp+50h] [ebp-38h]
    float hitForce[3]; // [esp+54h] [ebp-34h] BYREF
    float invRange; // [esp+60h] [ebp-28h]
    float centerOfMass[3]; // [esp+64h] [ebp-24h] BYREF
    int i; // [esp+70h] [ebp-18h]
    int boneIdx; // [esp+74h] [ebp-14h]
    float innerRadiusSqr; // [esp+78h] [ebp-10h]
    float scale; // [esp+7Ch] [ebp-Ch]
    Bone *bone; // [esp+80h] [ebp-8h]
    float outerRadiusSqr; // [esp+84h] [ebp-4h]

    if (!origin)
        MyAssertHandler(".\\ragdoll\\ragdoll_update.cpp", 1074, 0, "%s", "origin");
    if (innerRadius < 0.0)
        MyAssertHandler(".\\ragdoll\\ragdoll_update.cpp", 1075, 0, "%s", "innerRadius >= 0.0f");
    if (innerRadius > (double)outerRadius)
        MyAssertHandler(".\\ragdoll\\ragdoll_update.cpp", 1076, 0, "%s", "outerRadius >= innerRadius");
    if (localClientNum == RETURN_ZERO32() && outerRadius != 0.0)
    {
        outerRadiusSqr = outerRadius * outerRadius;
        innerRadiusSqr = innerRadius * innerRadius;
        invRange = 0.0;
        if (innerRadiusSqr < (double)outerRadiusSqr)
            invRange = 1.0 / (outerRadiusSqr - innerRadiusSqr);
        for (i = 0; i < 32; ++i)
        {
            body = &ragdollBodies[i];
            if (!body)
                MyAssertHandler("c:\\trees\\cod3\\src\\ragdoll\\ragdoll.h", 285, 0, "%s", "body");
            if (body->references > 0 && body->state >= BS_RUNNING)
            {
                Ragdoll_GetTorsoPosition(body, torsoPos);
                if (isCylinder)
                {
                    v12 = *origin - torsoPos[0];
                    v13 = origin[1] - torsoPos[1];
                    delta[0] = v12;
                    delta[1] = v13;
                    delta[2] = 0.0;
                }
                else
                {
                    Vec3Sub(torsoPos, origin, delta);
                }
                distSqra = Vec3LengthSq(delta);
                if (outerRadiusSqr > (double)distSqra)
                {
                    if (!body)
                        MyAssertHandler("c:\\trees\\cod3\\src\\ragdoll\\ragdoll.h", 299, 0, "%s", "body");
                    if (body->state != BS_IDLE || Ragdoll_BodyNewState(body, BS_RUNNING))
                    {
                        body->stateMsec = 0;
                        boneScale = 1.0 / (double)body->numBones;
                        for (boneIdx = 0; boneIdx < body->numBones; ++boneIdx)
                        {
                            scale = inScale;
                            bone = &body->bones[boneIdx];
                            if (body->bones[boneIdx].rigidBody)
                            {
                                Phys_ObjGetCenterOfMass((dxBody *)bone->rigidBody, centerOfMass);
                                v7 = flrand(-1.0, 1.0);
                                centerOfMass[0] = v7 + centerOfMass[0];
                                v8 = flrand(-1.0, 1.0);
                                centerOfMass[1] = v8 + centerOfMass[1];
                                v9 = flrand(-1.0, 1.0);
                                centerOfMass[2] = v9 + centerOfMass[2];
                                Vec3Sub(centerOfMass, origin, delta);
                                distSqr = Vec3LengthSq(delta);
                                if (outerRadiusSqr > (double)distSqr)
                                {
                                    if (innerRadiusSqr < (double)distSqr)
                                    {
                                        if (innerRadiusSqr >= (double)outerRadiusSqr)
                                            MyAssertHandler(
                                                ".\\ragdoll\\ragdoll_update.cpp",
                                                1142,
                                                1,
                                                "%s",
                                                "outerRadiusSqr > innerRadiusSqr");
                                        scale = (outerRadiusSqr - distSqr) * invRange * scale;
                                    }
                                    if (*impulse == 0.0 && impulse[1] == 0.0 && impulse[2] == 0.0)
                                    {
                                        hitForce[0] = delta[0];
                                        hitForce[1] = delta[1];
                                        hitForce[2] = delta[2];
                                        if (isCylinder)
                                            hitForce[2] = 0.0;
                                        Vec3Normalize(hitForce);
                                    }
                                    else
                                    {
                                        hitForce[0] = *impulse;
                                        hitForce[1] = impulse[1];
                                        hitForce[2] = impulse[2];
                                    }
                                    hitForce[2] = hitForce[2] + ragdoll_explode_upbias->current.value;
                                    Vec3Normalize(hitForce);
                                    v10 = scale * ragdoll_explode_force->current.value * boneScale;
                                    Vec3Scale(hitForce, v10, hitForce);
                                    Phys_ObjAddForce(PHYS_WORLD_RAGDOLL, (dxBody *)bone->rigidBody, centerOfMass, hitForce);
                                }
                            }
                        }
                    }
                }
            }
        }
    }
}

void __cdecl Ragdoll_GetTorsoPosition(RagdollBody *body, float *center)
{
    BoneOrientation *boneOrientation; // [esp+0h] [ebp-4h]

    if (!body || body->references <= 0)
        MyAssertHandler(".\\ragdoll\\ragdoll_update.cpp", 1049, 0, "%s", "body && Ragdoll_BodyInUse( body )");
    boneOrientation = Ragdoll_BodyBoneOrientations(body);
    *center = boneOrientation->origin[0];
    center[1] = boneOrientation->origin[1];
    center[2] = boneOrientation->origin[2];
}

bool __cdecl Ragdoll_EnterTunnelTest(RagdollBody *body, BodyState_t curState, BodyState_t newState)
{
    BoneOrientation *v2; // eax
    BoneOrientation *v3; // eax
    BoneOrientation *boneOrientation; // [esp+0h] [ebp-Ch]
    char tunnelPassed; // [esp+7h] [ebp-5h]
    const cpose_t *pose; // [esp+8h] [ebp-4h]

    if (Ragdoll_CreateBodyPhysics(body))
    {
        v2 = Ragdoll_BodyBoneOrientations(body);
        Ragdoll_SetCurrentPoseFromSnapshot(body, v2);
        Ragdoll_EstimateInitialVelocities(body);
        tunnelPassed = Ragdoll_TunnelTest(body);
        pose = Ragdoll_BodyPose(body);
        if (!pose)
            MyAssertHandler(".\\ragdoll\\ragdoll_update.cpp", 1442, 0, "%s", "pose");
        boneOrientation = Ragdoll_BodyBoneOrientations(body);
        Vec3Sub(pose->origin, boneOrientation->origin, body->poseOffset);
        v3 = Ragdoll_BodyBoneOrientations(body);
        Ragdoll_SnapshotBaseLerpBones(body, v3);
        Ragdoll_UpdateBodyContactCentroids(body);
        if (tunnelPassed)
        {
            Ragdoll_BodyNewState(body, BS_RUNNING);
        }
        else
        {
            Ragdoll_PrintTunnelFail(body);
            Ragdoll_BodyNewState(body, BS_DEAD);
        }
        return 1;
    }
    else
    {
        Ragdoll_BodyNewState(body, BS_DEAD);
        return 0;
    }
}

void __cdecl Ragdoll_SnapshotBaseLerpBones(RagdollBody *body, BoneOrientation *snapshot)
{
    BoneOrientation *v2; // eax
    float v3; // [esp+8h] [ebp-1A8h]
    float v4; // [esp+Ch] [ebp-1A4h]
    float v5; // [esp+10h] [ebp-1A0h]
    float v6; // [esp+34h] [ebp-17Ch]
    float v7; // [esp+38h] [ebp-178h]
    float v8; // [esp+3Ch] [ebp-174h]
    float v9; // [esp+40h] [ebp-170h]
    float result[3]; // [esp+44h] [ebp-16Ch] BYREF
    float v11; // [esp+50h] [ebp-160h]
    float v12; // [esp+54h] [ebp-15Ch]
    float v13; // [esp+58h] [ebp-158h]
    float v14; // [esp+5Ch] [ebp-154h]
    float v15; // [esp+60h] [ebp-150h]
    float v16; // [esp+64h] [ebp-14Ch]
    RagdollDef *def; // [esp+68h] [ebp-148h]
    int parentBoneNum; // [esp+6Ch] [ebp-144h]
    DObj_s *obj; // [esp+70h] [ebp-140h]
    BoneOrientation *baseOffsetOrientation; // [esp+74h] [ebp-13Ch]
    DObjAnimMat *parentAnimMat; // [esp+78h] [ebp-138h]
    float bodyOffset[3]; // [esp+7Ch] [ebp-134h] BYREF
    float lerp; // [esp+88h] [ebp-128h]
    float boneMat[4][3]; // [esp+8Ch] [ebp-124h] BYREF
    DObjAnimMat *boneAnimMat; // [esp+BCh] [ebp-F4h]
    BoneOrientation *parentOrientation; // [esp+C0h] [ebp-F0h]
    unsigned __int8 parentBoneIdx; // [esp+C7h] [ebp-E9h]
    int goalMsec; // [esp+C8h] [ebp-E8h]
    float invAxis[3][3]; // [esp+CCh] [ebp-E4h] BYREF
    const cpose_t *pose; // [esp+F0h] [ebp-C0h]
    LerpBone *lerpBone; // [esp+F4h] [ebp-BCh]
    unsigned __int8 boneIdx; // [esp+FBh] [ebp-B5h]
    int i; // [esp+FCh] [ebp-B4h]
    float currentOffset[3]; // [esp+100h] [ebp-B0h] BYREF
    float newLocalRot[4]; // [esp+10Ch] [ebp-A4h] BYREF
    float axis[3][3]; // [esp+11Ch] [ebp-94h] BYREF
    float currentLocalRot[16]; // [esp+140h] [ebp-70h] BYREF
    float relMat[4][3]; // [esp+180h] [ebp-30h] BYREF
    BoneOrientation *snapshota; // [esp+1BCh] [ebp+Ch]

    def = Ragdoll_BodyDef(body);
    if (!def)
        MyAssertHandler(".\\ragdoll\\ragdoll_update.cpp", 171, 0, "%s", "def");
    obj = Ragdoll_BodyDObj(body);
    if (obj)
    {
        pose = Ragdoll_BodyPose(body);
        if (pose)
        {
            lerpBone = body->lerpBones;
            baseOffsetOrientation = body->lerpBoneOffsets;
            snapshota = &snapshot[body->numBones];
            i = 0;
            while (i < def->numBaseLerpBones)
            {
                goalMsec = def->baseLerpBoneDefs[i].lerpTime;
                if (goalMsec <= 0)
                {
                    lerp = 1.0;
                }
                else
                {
                    lerp = body->stateMsec / goalMsec;
                    v5 = lerp - 1.0;
                    if (v5 < 0.0)
                        v16 = lerp;
                    else
                        v16 = 1.0;
                    v4 = 0.0 - lerp;
                    if (v4 < 0.0)
                        v3 = v16;
                    else
                        v3 = 0.0;
                    lerp = v3;
                }
                boneIdx = lerpBone->animBone;
                parentBoneNum = lerpBone->parentBone;
                if (parentBoneNum >= body->numBones)
                    MyAssertHandler(
                        ".\\ragdoll\\ragdoll_update.cpp",
                        202,
                        0,
                        "parentBoneNum doesn't index body->numBones\n\t%i not in [0, %i)",
                        parentBoneNum,
                        body->numBones);
                v2 = Ragdoll_BodyBoneOrientations(body);
                parentOrientation = &v2[parentBoneNum];
                parentBoneIdx = body->bones[parentBoneNum].animBones[0];
                boneAnimMat = Ragdoll_GetDObjLocalBoneMatrix(pose, obj, boneIdx);
                parentAnimMat = Ragdoll_GetDObjLocalBoneMatrix(pose, obj, parentBoneIdx);
                if (!boneAnimMat || !parentAnimMat || boneAnimMat == parentAnimMat)
                    break;
                if ((COERCE_UNSIGNED_INT(parentAnimMat->quat[0]) & 0x7F800000) == 0x7F800000
                    || (COERCE_UNSIGNED_INT(parentAnimMat->quat[1]) & 0x7F800000) == 0x7F800000
                    || (COERCE_UNSIGNED_INT(parentAnimMat->quat[2]) & 0x7F800000) == 0x7F800000
                    || (COERCE_UNSIGNED_INT(parentAnimMat->quat[3]) & 0x7F800000) == 0x7F800000)
                {
                    MyAssertHandler(
                        "c:\\trees\\cod3\\src\\ragdoll\\../xanim/xanim_public.h",
                        536,
                        0,
                        "%s",
                        "!IS_NAN((mat->quat)[0]) && !IS_NAN((mat->quat)[1]) && !IS_NAN((mat->quat)[2]) && !IS_NAN((mat->quat)[3])");
                }
                if ((COERCE_UNSIGNED_INT(parentAnimMat->transWeight) & 0x7F800000) == 0x7F800000)
                    MyAssertHandler(
                        "c:\\trees\\cod3\\src\\ragdoll\\../xanim/xanim_public.h",
                        537,
                        0,
                        "%s",
                        "!IS_NAN(mat->transWeight)");
                Vec3Scale(parentAnimMat->quat, parentAnimMat->transWeight, result);
                v14 = result[0] * parentAnimMat->quat[0];
                v7 = result[0] * parentAnimMat->quat[1];
                v12 = result[0] * parentAnimMat->quat[2];
                v15 = result[0] * parentAnimMat->quat[3];
                v6 = result[1] * parentAnimMat->quat[1];
                v13 = result[1] * parentAnimMat->quat[2];
                v11 = result[1] * parentAnimMat->quat[3];
                v8 = result[2] * parentAnimMat->quat[2];
                v9 = result[2] * parentAnimMat->quat[3];
                currentLocalRot[4] = 1.0 - (v6 + v8);
                currentLocalRot[5] = v7 - v9;
                currentLocalRot[6] = v12 + v11;
                currentLocalRot[7] = v7 + v9;
                currentLocalRot[8] = 1.0 - (v14 + v8);
                currentLocalRot[9] = v13 - v15;
                currentLocalRot[10] = v12 - v11;
                currentLocalRot[11] = v13 + v15;
                currentLocalRot[12] = 1.0 - (v14 + v6);
                currentLocalRot[13] = -(parentAnimMat->trans[0] * currentLocalRot[4]
                    + parentAnimMat->trans[1] * currentLocalRot[7]
                    + parentAnimMat->trans[2] * currentLocalRot[10]);
                currentLocalRot[14] = -(parentAnimMat->trans[0] * currentLocalRot[5]
                    + parentAnimMat->trans[1] * currentLocalRot[8]
                    + parentAnimMat->trans[2] * currentLocalRot[11]);
                currentLocalRot[15] = -(parentAnimMat->trans[0] * currentLocalRot[6]
                    + parentAnimMat->trans[1] * currentLocalRot[9]
                    + parentAnimMat->trans[2] * currentLocalRot[12]);
                Ragdoll_AnimMatToMat43(boneAnimMat, boneMat);
                MatrixMultiply43(boneMat, *(const mat4x3*)&currentLocalRot[4], relMat);
                AxisToQuat(relMat, currentLocalRot);
                currentOffset[0] = relMat[3][0];
                currentOffset[1] = relMat[3][1];
                currentOffset[2] = relMat[3][2];
                if (Vec3Length(currentOffset) <= 400.0)
                {
                    Ragdoll_QuatLerp(currentLocalRot, baseOffsetOrientation->orientation, lerp, newLocalRot);
                    Vec3Lerp(currentOffset, baseOffsetOrientation->origin, lerp, bodyOffset);
                }
                else
                {
                    newLocalRot[0] = baseOffsetOrientation->orientation[0];
                    newLocalRot[1] = baseOffsetOrientation->orientation[1];
                    newLocalRot[2] = baseOffsetOrientation->orientation[2];
                    newLocalRot[3] = baseOffsetOrientation->orientation[3];
                    bodyOffset[0] = baseOffsetOrientation->origin[0];
                    bodyOffset[1] = baseOffsetOrientation->origin[1];
                    bodyOffset[2] = baseOffsetOrientation->origin[2];
                }
                Ragdoll_QuatMul(parentOrientation->orientation, newLocalRot, snapshota->orientation);
                QuatToAxis(parentOrientation->orientation, axis);
                AxisTranspose(axis, invAxis);
                Vec3Rotate(bodyOffset, invAxis, snapshota->origin);
                Vec3Add(parentOrientation->origin, snapshota->origin, snapshota->origin);
                ++i;
                ++lerpBone;
                ++baseOffsetOrientation;
                ++snapshota;
            }
        }
    }
}

DObjAnimMat *__cdecl Ragdoll_GetDObjLocalBoneMatrix(const cpose_t *pose, DObj_s *obj, unsigned __int8 boneIndex)
{
    DObjAnimMat *mat; // [esp+34h] [ebp-4h]

    if (!obj)
        MyAssertHandler(".\\ragdoll\\ragdoll_update.cpp", 43, 0, "%s", "obj");
    //Profile_Begin(398);
    CG_DObjCalcBone(pose, obj, boneIndex);
    //Profile_EndInternal(0);
    mat = DObjGetRotTransArray(obj);
    if (mat)
        return &mat[boneIndex];
    else
        return 0;
}

void __cdecl Ragdoll_SetCurrentPoseFromSnapshot(RagdollBody *body, BoneOrientation *snapshot)
{
    const float *qa; // [esp+4h] [ebp-24h]
    RagdollDef *def; // [esp+8h] [ebp-20h]
    float rotatedOrientation[4]; // [esp+Ch] [ebp-1Ch] BYREF
    BoneDef *boneDef; // [esp+1Ch] [ebp-Ch]
    int i; // [esp+20h] [ebp-8h]
    Bone *bone; // [esp+24h] [ebp-4h]

    if (!body)
        MyAssertHandler(".\\ragdoll\\ragdoll_update.cpp", 337, 0, "%s", "body");
    def = Ragdoll_BodyDef(body);
    if (!def)
        MyAssertHandler(".\\ragdoll\\ragdoll_update.cpp", 340, 0, "%s", "def");
    bone = body->bones;
    boneDef = def->boneDefs;
    i = 0;
    while (i < body->numBones)
    {
        qa = snapshot->orientation;
        if (boneDef->mirror)
        {
            Ragdoll_QuatMul(qa, quatZRot, rotatedOrientation);
        }
        else
        {
            rotatedOrientation[0] = *qa;
            rotatedOrientation[1] = snapshot->orientation[1];
            rotatedOrientation[2] = snapshot->orientation[2];
            rotatedOrientation[3] = snapshot->orientation[3];
        }
        if (bone->rigidBody)
            Phys_ObjSetOrientation(PHYS_WORLD_RAGDOLL, (dxBody *)bone->rigidBody, snapshot->origin, rotatedOrientation);
        ++i;
        ++bone;
        ++boneDef;
        ++snapshot;
    }
}

void __cdecl Ragdoll_UpdateBodyContactCentroids(RagdollBody *body)
{
    float com[3]; // [esp+0h] [ebp-14h] BYREF
    int i; // [esp+Ch] [ebp-8h]
    Bone *bone; // [esp+10h] [ebp-4h]

    Ragdoll_BodyCenterOfMass(body, com);
    bone = body->bones;
    i = 0;
    while (i < body->numBones)
    {
        if (bone->rigidBody)
            Phys_ObjSetContactCentroid((dxBody *)bone->rigidBody, com);
        ++i;
        ++bone;
    }
}

void __cdecl Ragdoll_BodyCenterOfMass(RagdollBody *body, float *com)
{
    float v2; // [esp+Ch] [ebp-24h]
    float massSum; // [esp+14h] [ebp-1Ch]
    float boneCom[3]; // [esp+18h] [ebp-18h] BYREF
    BoneDef *boneDef; // [esp+24h] [ebp-Ch]
    int i; // [esp+28h] [ebp-8h]
    Bone *bone; // [esp+2Ch] [ebp-4h]

    if (!body)
        MyAssertHandler(".\\ragdoll\\ragdoll_update.cpp", 698, 0, "%s", "body");
    bone = body->bones;
    boneDef = Ragdoll_BodyDef(body)->boneDefs;
    massSum = 0.0;
    i = 0;
    while (i < body->numBones)
    {
        massSum = massSum + boneDef->mass;
        if (bone->rigidBody)
        {
            Phys_ObjGetCenterOfMass((dxBody *)bone->rigidBody, boneCom);
            Vec3Mad(com, boneDef->mass, boneCom, com);
        }
        ++i;
        ++bone;
        ++boneDef;
    }
    if (massSum <= 0.000001)
        MyAssertHandler(".\\ragdoll\\ragdoll_update.cpp", 716, 0, "%s", "massSum > ZERO_EPSILON");
    v2 = 1.0 / massSum;
    Vec3Scale(com, v2, com);
}

void __cdecl Ragdoll_EstimateInitialVelocities(RagdollBody *body)
{
    float scale; // [esp+8h] [ebp-78h]
    float angleOffset[3]; // [esp+18h] [ebp-68h] BYREF
    float posOffset[3]; // [esp+24h] [ebp-5Ch] BYREF
    BoneOrientation *prevOrientation; // [esp+30h] [ebp-50h]
    float timeScale; // [esp+34h] [ebp-4Ch]
    float prevRot[4]; // [esp+38h] [ebp-48h] BYREF
    float curRot[4]; // [esp+48h] [ebp-38h] BYREF
    BoneOrientation *curOrientation; // [esp+58h] [ebp-28h]
    float rotOffset[4]; // [esp+5Ch] [ebp-24h] BYREF
    int i; // [esp+6Ch] [ebp-14h]
    float angleOffsetWorld[3]; // [esp+70h] [ebp-10h] BYREF
    Bone *bone; // [esp+7Ch] [ebp-4h]

    if (!body)
        MyAssertHandler(".\\ragdoll\\ragdoll_update.cpp", 1233, 0, "%s", "body");
    if (body->velCaptureMsec)
    {
        timeScale = 1000.0 / (double)body->velCaptureMsec;
        curOrientation = Ragdoll_BodyBoneOrientations(body);
        prevOrientation = Ragdoll_BodyPrevBoneOrientations(body);
        bone = body->bones;
        i = 0;
        while (i < body->numBones)
        {
            Vec3Sub(curOrientation->origin, prevOrientation->origin, posOffset);
            if (Vec4Dot(curOrientation->orientation, prevOrientation->orientation) >= 0.0)
            {
                prevRot[0] = prevOrientation->orientation[0];
                prevRot[1] = prevOrientation->orientation[1];
                prevRot[2] = prevOrientation->orientation[2];
                prevRot[3] = prevOrientation->orientation[3];
            }
            else
            {
                prevRot[0] = -prevOrientation->orientation[0];
                prevRot[1] = -prevOrientation->orientation[1];
                prevRot[2] = -prevOrientation->orientation[2];
                prevRot[3] = -prevOrientation->orientation[3];
            }
            curRot[0] = curOrientation->orientation[0];
            curRot[1] = curOrientation->orientation[1];
            curRot[2] = curOrientation->orientation[2];
            curRot[3] = curOrientation->orientation[3];
            Ragdoll_QuatNormalize(curRot);
            Ragdoll_QuatNormalize(prevRot);
            Ragdoll_QuatMulInvSecond(curRot, prevRot, rotOffset);
            Ragdoll_QuatNormalize(rotOffset);
            Ragdoll_QuatToAxisAngle(rotOffset, angleOffset);
            Ragdoll_QuatPointRotate(angleOffset, curRot, angleOffsetWorld);
            scale = timeScale * ragdoll_rotvel_scale->current.value;
            Vec3Scale(angleOffsetWorld, scale, angleOffset);
            Vec3Scale(posOffset, timeScale, posOffset);
            Phys_ObjSetAngularVelocityRaw((dxBody *)bone->rigidBody, angleOffset);
            Phys_ObjSetVelocity((dxBody *)bone->rigidBody, posOffset);
            ++i;
            ++curOrientation;
            ++prevOrientation;
            ++bone;
        }
    }
}

char __cdecl Ragdoll_TunnelTest(RagdollBody *body)
{
    float scale; // [esp+Ch] [ebp-100h]
    const float *qa; // [esp+14h] [ebp-F8h]
    RagdollDef *def; // [esp+18h] [ebp-F4h]
    float start[3]; // [esp+20h] [ebp-ECh] BYREF
    float end[3]; // [esp+2Ch] [ebp-E0h] BYREF
    float boneMat[3][3]; // [esp+38h] [ebp-D4h] BYREF
    trace_t trace; // [esp+5Ch] [ebp-B0h] BYREF
    BoneDef *boneDef; // [esp+88h] [ebp-84h]
    int childIndices[6]; // [esp+8Ch] [ebp-80h] BYREF
    Bone *childBone; // [esp+A4h] [ebp-68h]
    BoneOrientation *boneOrientations; // [esp+A8h] [ebp-64h]
    int child; // [esp+ACh] [ebp-60h]
    BoneOrientation *curOrientation; // [esp+B0h] [ebp-5Ch]
    int numChildren; // [esp+B4h] [ebp-58h]
    int i; // [esp+B8h] [ebp-54h]
    int childIdx; // [esp+BCh] [ebp-50h]
    trace_t revTrace; // [esp+C0h] [ebp-4Ch] BYREF
    Bone *bone; // [esp+ECh] [ebp-20h]
    float orientation[4]; // [esp+F0h] [ebp-1Ch] BYREF
    float center[3]; // [esp+100h] [ebp-Ch] BYREF

    if (!body)
        MyAssertHandler(".\\ragdoll\\ragdoll_update.cpp", 1346, 0, "%s", "body");
    def = Ragdoll_BodyDef(body);
    if (!def)
        MyAssertHandler(".\\ragdoll\\ragdoll_update.cpp", 1349, 0, "%s", "def");
    bone = body->bones;
    boneDef = def->boneDefs;
    curOrientation = Ragdoll_BodyBoneOrientations(body);
    boneOrientations = curOrientation;
    i = 0;
    while (i < body->numBones)
    {
        if (bone->rigidBody)
        {
            qa = curOrientation->orientation;
            if (boneDef->mirror)
            {
                Ragdoll_QuatMul(qa, quatZRot, orientation);
            }
            else
            {
                orientation[0] = *qa;
                orientation[1] = curOrientation->orientation[1];
                orientation[2] = curOrientation->orientation[2];
                orientation[3] = curOrientation->orientation[3];
            }
            QuatToAxis(orientation, boneMat);
            start[0] = curOrientation->origin[0];
            start[1] = curOrientation->origin[1];
            start[2] = curOrientation->origin[2];
            numChildren = Ragdoll_FindBoneChildren(body, i, childIndices);
            if (numChildren > 1)
            {
                scale = bone->length * 0.5;
                Vec3Mad(start, scale, boneMat[0], center);
                for (child = 0; child < numChildren; ++child)
                {
                    childIdx = childIndices[child];
                    if ((unsigned int)childIdx >= body->numBones)
                        MyAssertHandler(
                            ".\\ragdoll\\ragdoll_update.cpp",
                            1377,
                            0,
                            "childIdx doesn't index body->numBones\n\t%i not in [0, %i)",
                            childIdx,
                            body->numBones);
                    childBone = &body->bones[childIdx];
                    if (!Ragdoll_BoneTrace(&trace, &revTrace, center, boneOrientations[childIdx].origin))
                        return 0;
                }
            }
            Vec3Mad(curOrientation->origin, bone->length, boneMat[0], end);
            if (!Ragdoll_BoneTrace(&trace, &revTrace, start, end))
                return 0;
        }
        ++i;
        ++bone;
        ++boneDef;
        ++curOrientation;
    }
    return 1;
}

int __cdecl Ragdoll_FindBoneChildren(RagdollBody *body, int boneIdx, int *childIndices)
{
    int numChildren; // [esp+0h] [ebp-8h]
    int i; // [esp+4h] [ebp-4h]

    if (!body)
        MyAssertHandler(".\\ragdoll\\ragdoll_update.cpp", 1277, 0, "%s", "body");
    numChildren = 0;
    for (i = 0; i < body->numBones; ++i)
    {
        if (body->bones[i].parentBone == boneIdx)
        {
            if (numChildren == 6)
                return 6;
            childIndices[numChildren++] = i;
        }
    }
    return numChildren;
}

char __cdecl Ragdoll_BoneTrace(trace_t *trace, trace_t *revTrace, float *start, float *end)
{
    CM_BoxTrace(trace, start, end, vec3_origin, vec3_origin, 0, 0x2806C91);
    if (trace->startsolid)
    {
        CM_BoxTrace(revTrace, end, start, vec3_origin, vec3_origin, 0, 0x2806C91);
        if (revTrace->startsolid)
            return 0;
    }
    else if (trace->fraction != 1.0)
    {
        CM_BoxTrace(revTrace, end, start, vec3_origin, vec3_origin, 0, 0x2806C91);
        if (revTrace->fraction != 1.0)
            return 0;
    }
    return 1;
}

void __cdecl Ragdoll_PrintTunnelFail(RagdollBody *body)
{
    DObj_s *obj; // [esp+18h] [ebp-8h]
    const cpose_t *pose; // [esp+1Ch] [ebp-4h]

    if (ragdoll_dump_anims->current.enabled)
    {
        pose = Ragdoll_BodyPose(body);
        obj = Ragdoll_BodyDObj(body);
        if (!pose)
            MyAssertHandler(".\\ragdoll\\ragdoll_update.cpp", 1413, 0, "%s", "pose");
        if (!obj)
            MyAssertHandler(".\\ragdoll\\ragdoll_update.cpp", 1414, 0, "%s", "obj");
        Com_Printf(
            20,
            "Ragdoll initial state in solid, using regular anim. Pos (%0.f %0.f %0.f)\n",
            pose->origin[0],
            pose->origin[1],
            pose->origin[2]);
        DObjDisplayAnim(obj, "Ragdoll anim tree: ");
    }
}

void __cdecl Ragdoll_UpdateVelocityCapture(RagdollBody *body)
{
    BoneOrientation *snapshot; // [esp+0h] [ebp-4h]
    BoneOrientation *snapshota; // [esp+0h] [ebp-4h]

    if (!body)
        MyAssertHandler(".\\ragdoll\\ragdoll_update.cpp", 1469, 0, "%s", "body");
    if (body->stateFrames >= 1)
    {
        body->velCaptureMsec = body->stateMsec;
        body->curOrientationBuffer ^= 1u;
        snapshota = Ragdoll_BodyBoneOrientations(body);
        Ragdoll_SnapshotAnimOrientations(body, snapshota);
        Ragdoll_BodyNewState(body, BodyState_t::BS_TUNNEL_TEST);
    }
    else
    {
        body->curOrientationBuffer ^= 1u;
        snapshot = Ragdoll_BodyBoneOrientations(body);
        Ragdoll_SnapshotAnimOrientations(body, snapshot);
    }
}

void __cdecl Ragdoll_SnapshotAnimOrientations(RagdollBody *body, BoneOrientation *snapshot)
{
    RagdollDef *def; // [esp+0h] [ebp-18h]
    DObj_s *obj; // [esp+4h] [ebp-14h]
    BoneDef *boneDef; // [esp+8h] [ebp-10h]
    const cpose_t *pose; // [esp+Ch] [ebp-Ch]
    int i; // [esp+10h] [ebp-8h]
    Bone *bone; // [esp+14h] [ebp-4h]

    if (!body)
        MyAssertHandler(".\\ragdoll\\ragdoll_update.cpp", 305, 0, "%s", "body");
    def = Ragdoll_BodyDef(body);
    obj = Ragdoll_BodyDObj(body);
    pose = Ragdoll_BodyPose(body);
    if (def && obj && pose)
    {
        bone = body->bones;
        boneDef = def->boneDefs;
        i = 0;
        while (i < body->numBones)
        {
            Ragdoll_GetDObjWorldBoneOriginQuat(
                body->localClientNum,
                pose,
                obj,
                bone->animBones[0],
                snapshot->origin,
                snapshot->orientation);
            ++i;
            ++bone;
            ++boneDef;
            ++snapshot;
        }
    }
    else
    {
        Ragdoll_BodyNewState(body, BS_DEAD);
    }
}

char __cdecl Ragdoll_GetDObjWorldBoneOriginQuat(
    int localClientNum,
    const cpose_t *pose,
    DObj_s *obj,
    unsigned __int8 boneIndex,
    float *origin,
    float *quat)
{
    DObjAnimMat *mat; // [esp+4h] [ebp-4h]

    if (!obj)
        MyAssertHandler(".\\ragdoll\\ragdoll_update.cpp", 69, 0, "%s", "obj");
    if (!pose)
        MyAssertHandler(".\\ragdoll\\ragdoll_update.cpp", 70, 0, "%s", "pose");
    mat = Ragdoll_GetDObjLocalBoneMatrix(pose, obj, boneIndex);
    if (!mat)
        return 0;
    *quat = mat->quat[0];
    quat[1] = mat->quat[1];
    quat[2] = mat->quat[2];
    quat[3] = mat->quat[3];
    if (localClientNum)
        MyAssertHandler(
            "c:\\trees\\cod3\\src\\ragdoll\\../cgame_mp/cg_local_mp.h",
            1071,
            0,
            "%s\n\t(localClientNum) = %i",
            "(localClientNum == 0)",
            localClientNum);
    Vec3Add(mat->trans, cgArray[0].refdef.viewOffset, origin);
    return 1;
}

bool __cdecl Ragdoll_EnterDead(RagdollBody *body, BodyState_t curState, BodyState_t newState)
{
    int references; // [esp+0h] [ebp-4h]

    if (!body)
        MyAssertHandler(".\\ragdoll\\ragdoll_update.cpp", 1553, 0, "%s", "body");
    Ragdoll_RemoveBodyPhysics(body);
    references = body->references;
    memset((unsigned __int8 *)body, 0, sizeof(RagdollBody));
    body->references = references;
    return 1;
}

bool __cdecl Ragdoll_ExitDead(RagdollBody *body, BodyState_t curState, BodyState_t newState)
{
    RagdollDef *def; // [esp+0h] [ebp-4h]

    if (!body)
        MyAssertHandler(".\\ragdoll\\ragdoll_update.cpp", 1570, 0, "%s", "body");
    def = Ragdoll_BodyDef(body);
    if (!def)
        MyAssertHandler(".\\ragdoll\\ragdoll_update.cpp", 1573, 0, "%s", "def");
    body->numBones = def->numBones;
    body->numLerpBones = def->numBaseLerpBones;
    body->numJoints = def->numJoints;
    return 1;
}

bool __cdecl Ragdoll_ExitDObjWait(RagdollBody *body, BodyState_t prevState, BodyState_t curState)
{
    RagdollDef *def; // [esp+0h] [ebp-18h]
    DObj_s *obj; // [esp+4h] [ebp-14h]
    BoneDef *boneDef; // [esp+8h] [ebp-10h]
    LerpBone *lerpBone; // [esp+Ch] [ebp-Ch]
    int i; // [esp+10h] [ebp-8h]
    int ia; // [esp+10h] [ebp-8h]
    Bone *bone; // [esp+14h] [ebp-4h]

    if (!body)
        MyAssertHandler(".\\ragdoll\\ragdoll_update.cpp", 1592, 0, "%s", "body");
    if (curState == BS_DEAD)
        return 1;
    def = Ragdoll_BodyDef(body);
    if (!def)
        MyAssertHandler(".\\ragdoll\\ragdoll_update.cpp", 1598, 0, "%s", "def");
    obj = Ragdoll_BodyDObj(body);
    if (!obj)
        MyAssertHandler(".\\ragdoll\\ragdoll_update.cpp", 1601, 0, "%s", "obj");
    bone = body->bones;
    boneDef = def->boneDefs;
    for (i = 0; i < body->numBones; ++i)
    {
        bone->parentBone = boneDef->parentBone;
        bone->animBones[1] = 0;
        bone->animBones[0] = 0;
        if (!DObjGetBoneIndex(obj, boneDef->animBoneNames[0], bone->animBones) || bone->animBones[0] == 255)
            return 0;
        if (boneDef->animBoneNames[1] == -1)
        {
            bone->animBones[1] = 0;
        }
        else if (!DObjGetBoneIndex(obj, boneDef->animBoneNames[1], &bone->animBones[1]) || bone->animBones[0] == 255)
        {
            return 0;
        }
        ++bone;
        ++boneDef;
    }
    lerpBone = body->lerpBones;
    for (ia = 0; ia < body->numLerpBones; ++ia)
    {
        lerpBone->animBone = 0;
        if (!DObjGetBoneIndex(obj, def->baseLerpBoneDefs[ia].animBoneName, &lerpBone->animBone)
            || lerpBone->animBone == 255)
        {
            return 0;
        }
        lerpBone->parentBone = def->baseLerpBoneDefs[ia].parentBoneIndex;
        ++lerpBone;
    }
    Ragdoll_SnapshotBaseLerpOffsets(body);
    return 1;
}

bool __cdecl Ragdoll_ExitIdle(RagdollBody *body, BodyState_t curState, BodyState_t newState)
{
    BoneOrientation *v4; // eax

    if (!body)
        MyAssertHandler(".\\ragdoll\\ragdoll_update.cpp", 1642, 0, "%s", "body");
    if ((unsigned int)newState <= BS_DOBJ_WAIT)
        return 1;
    if (!Ragdoll_ValidateBodyObj(body))
        return 0;
    if (!body)
        MyAssertHandler("c:\\trees\\cod3\\src\\ragdoll\\ragdoll.h", 271, 0, "%s", "body");
    if (body->state < BS_TUNNEL_TEST)
        return 0;
    if (Ragdoll_CountPhysicsBodies() >= ragdoll_max_simulating->current.integer)
        return 0;
    if (!Ragdoll_CreateBodyPhysics(body))
        return 0;
    v4 = Ragdoll_BodyBoneOrientations(body);
    Ragdoll_SetCurrentPoseFromSnapshot(body, v4);
    return 1;
}

bool __cdecl Ragdoll_EnterIdle(RagdollBody *body, BodyState_t curState, BodyState_t newState)
{
    BoneOrientation *v1; // eax

    if (!body)
        MyAssertHandler(".\\ragdoll\\ragdoll_update.cpp", 1664, 0, "%s", "body");
    if (Ragdoll_BodyHasPhysics(body))
    {
        v1 = Ragdoll_BodyBoneOrientations(body);
        Ragdoll_SnapshotBonePositions(body, v1);
    }
    Ragdoll_RemoveBodyPhysics(body);
    return 1;
}

void __cdecl Ragdoll_SnapshotBonePositions(RagdollBody *body, BoneOrientation *boneSnapshot)
{
    float *dest; // [esp+0h] [ebp-2Ch]
    RagdollDef *def; // [esp+8h] [ebp-24h]
    BoneOrientation *snapshot; // [esp+Ch] [ebp-20h]
    float boneRot[4]; // [esp+10h] [ebp-1Ch] BYREF
    BoneDef *boneDef; // [esp+20h] [ebp-Ch]
    int i; // [esp+24h] [ebp-8h]
    Bone *bone; // [esp+28h] [ebp-4h]

    if (!body || !boneSnapshot)
        MyAssertHandler(".\\ragdoll\\ragdoll_update.cpp", 253, 0, "%s", "body && boneSnapshot");
    if (Ragdoll_BodyHasPhysics(body))
    {
        def = Ragdoll_BodyDef(body);
        if (!def)
            MyAssertHandler(".\\ragdoll\\ragdoll_update.cpp", 259, 0, "%s", "def");
        bone = body->bones;
        boneDef = def->boneDefs;
        snapshot = boneSnapshot;
        i = 0;
        while (i < body->numBones)
        {
            if (!bone->rigidBody)
                MyAssertHandler(".\\ragdoll\\ragdoll_update.cpp", 267, 0, "%s", "bone->rigidBody");
            Phys_ObjGetInterpolatedState(PHYS_WORLD_RAGDOLL, (dxBody *)bone->rigidBody, snapshot->origin, boneRot);
            dest = snapshot->orientation;
            if (boneDef->mirror)
            {
                Ragdoll_QuatMul(boneRot, quatZRot, dest);
            }
            else
            {
                *dest = boneRot[0];
                snapshot->orientation[1] = boneRot[1];
                snapshot->orientation[2] = boneRot[2];
                snapshot->orientation[3] = boneRot[3];
            }
            ++i;
            ++bone;
            ++snapshot;
            ++boneDef;
        }
        Ragdoll_SnapshotBaseLerpBones(body, boneSnapshot);
    }
}

bool __cdecl Ragdoll_EnterRunning(RagdollBody *body, BodyState_t curState, BodyState_t newState)
{
    if (!body)
        MyAssertHandler(".\\ragdoll\\ragdoll_update.cpp", 1677, 0, "%s", "body");
    if (Ragdoll_CountPhysicsBodies() < ragdoll_max_simulating->current.integer)
        return 1;
    if (curState == BS_IDLE)
        Ragdoll_BodyNewState(body, BodyState_t::BS_IDLE);
    else
        Ragdoll_BodyNewState(body, BodyState_t::BS_DEAD);
    return 0;
}

void __cdecl Ragdoll_UpdateDObjWait(RagdollBody *body)
{
    if (!body)
        MyAssertHandler(".\\ragdoll\\ragdoll_update.cpp", 1695, 0, "%s", "body");
    if (body->stateFrames <= 3)
    {
        if (Ragdoll_ValidateBodyObj(body))
            Ragdoll_BodyNewState(body, BodyState_t::BS_VELOCITY_CAPTURE);
    }
    else
    {
        if (body->obj)
            Com_PrintWarning(20, "Ragdoll activation timed out waiting for dobj 0x%x\n", body->dobj);
        else
            Com_PrintWarning(20, "Ragdoll activation timed out waiting for dobj 0x%x\n", body->obj);
        Ragdoll_BodyNewState(body, BodyState_t::BS_DEAD);
    }
}

void __cdecl Ragdoll_UpdateRunning(RagdollBody *body)
{
    if (!body)
        MyAssertHandler(".\\ragdoll\\ragdoll_update.cpp", 1770, 0, "%s", "body");
    if (Ragdoll_CheckIdle(body))
    {
        Ragdoll_BodyNewState(body, BodyState_t::BS_IDLE);
    }
    else
    {
        Ragdoll_FilterBonePositions(body);
        Ragdoll_UpdateFriction(body);
        Ragdoll_UpdateBodyContactCentroids(body);
    }
}

void __cdecl Ragdoll_UpdateFriction(RagdollBody *body)
{
    JointType type; // [esp+10h] [ebp-28h]
    float frictionForce; // [esp+14h] [ebp-24h]
    RagdollDef *def; // [esp+18h] [ebp-20h]
    float frictionVec[3]; // [esp+1Ch] [ebp-1Ch] BYREF
    Joint *joint; // [esp+28h] [ebp-10h]
    float lerpScale; // [esp+2Ch] [ebp-Ch]
    int i; // [esp+30h] [ebp-8h]
    JointDef *jointDef; // [esp+34h] [ebp-4h]

    if (!body)
        MyAssertHandler(".\\ragdoll\\ragdoll_update.cpp", 1499, 0, "%s", "body");
    def = Ragdoll_BodyDef(body);
    if (!def)
        MyAssertHandler(".\\ragdoll\\ragdoll_update.cpp", 1502, 0, "%s", "def");
    jointDef = def->jointDefs;
    joint = body->joints;
    lerpScale = 1.0 - (double)body->stateMsec / (double)ragdoll_jointlerp_time->current.integer;
    lerpScale = lerpScale * lerpScale;
    lerpScale = lerpScale * 0.8999999761581421 + 0.1000000014901161;
    i = 0;
    while (i < def->numJoints)
    {
        type = jointDef->type;
        if (type == RAGDOLL_JOINT_HINGE)
        {
            if (joint->joint)
            {
                frictionForce = jointDef->axisFriction[0] * lerpScale;
                Phys_SetHingeParams(
                    PHYS_WORLD_RAGDOLL,
                    (dxJointHinge *)joint->joint,
                    0.0,
                    frictionForce,
                    jointDef->minAngles[0],
                    jointDef->maxAngles[0]);
            }
        }
        else if (type == RAGDOLL_JOINT_SWIVEL)
        {
            Vec3Scale(jointDef->axisFriction, lerpScale, frictionVec);
            if (joint->joint2)
                Phys_SetAngularMotorParams(
                    PHYS_WORLD_RAGDOLL,
                    (dxJointAMotor *)joint->joint2,
                    vec3_origin,
                    frictionVec,
                    jointDef->minAngles,
                    jointDef->maxAngles);
        }
        ++i;
        ++jointDef;
        ++joint;
    }
}

char __cdecl Ragdoll_CheckIdle(RagdollBody *body)
{
    int i; // [esp+0h] [ebp-4h]

    if (body->stateMsec > ragdoll_max_life->current.integer)
        return 1;
    for (i = 0; i < body->numBones; ++i)
    {
        if (!Phys_ObjIsAsleep((dxBody *)body->bones[i].rigidBody))
            return 0;
    }
    return 1;
}

void __cdecl Ragdoll_FilterBonePositions(RagdollBody *body)
{
    BoneOrientation *v1; // eax

    v1 = Ragdoll_BodyBoneOrientations(body);
    Ragdoll_SnapshotBonePositions(body, v1);
}

char __cdecl Ragdoll_BodyNewState(RagdollBody *body, BodyState_t state)
{
    BodyState_t prevState; // [esp+4h] [ebp-4h]

    if (!body)
        MyAssertHandler(".\\ragdoll\\ragdoll_update.cpp", 1826, 0, "%s", "body");
    if ((unsigned int)state >= RAGDOLL_NUM_STATES)
        MyAssertHandler(
            ".\\ragdoll\\ragdoll_update.cpp",
            1827,
            0,
            "state doesn't index RAGDOLL_NUM_STATES\n\t%i not in [0, %i)",
            state,
            6);
    prevState = body->state;
    if ((unsigned int)prevState >= RAGDOLL_NUM_STATES)
        MyAssertHandler(
            ".\\ragdoll\\ragdoll_update.cpp",
            1830,
            0,
            "prevState doesn't index RAGDOLL_NUM_STATES\n\t%i not in [0, %i)",
            prevState,
            6);
    if (prevState == state)
        return 1;
    if (stateEntries[prevState].exitFunc && !stateEntries[prevState].exitFunc(body, prevState, state))
        return 0;
    body->state = state;
    if (!stateEntries[state].enterFunc || stateEntries[state].enterFunc(body, prevState, state))
    {
        body->stateFrames = 0;
        body->stateMsec = 0;
        return 1;
    }
    else
    {
        if (body->state == state)
            body->state = prevState;
        return 0;
    }
}

void __cdecl Ragdoll_BodyUpdate(int msec, RagdollBody *body)
{
    StateEnt *entry; // [esp+0h] [ebp-8h]
    BodyState_t prevState; // [esp+4h] [ebp-4h]

    if (!body)
        MyAssertHandler(".\\ragdoll\\ragdoll_update.cpp", 1870, 0, "%s", "body");
    if (body->state >= (unsigned int)RAGDOLL_NUM_STATES)
        MyAssertHandler(
            ".\\ragdoll\\ragdoll_update.cpp",
            1871,
            0,
            "body->state doesn't index RAGDOLL_NUM_STATES\n\t%i not in [0, %i)",
            body->state,
            6);
    do
    {
        prevState = body->state;
        entry = &stateEntries[prevState];
        if (entry->updateFunc)
            entry->updateFunc(body);
    } while (prevState != body->state);
    ++body->stateFrames;
    body->stateMsec += msec;
}

void __cdecl Ragdoll_Update(int msec)
{
    RagdollBody *body; // [esp+30h] [ebp-8h]
    int i; // [esp+34h] [ebp-4h]

    if (!Sys_IsMainThread())
        MyAssertHandler(".\\ragdoll\\ragdoll_update.cpp", 1898, 0, "%s", "Sys_IsMainThread()");
    if (ragdollInited)
    {
        //Profile_Begin(400);
        if (ragdoll_enable->current.enabled && cg_paused && !cg_paused->current.integer)
        {
            ragdollTime += msec;
            for (i = 0; i < 32; ++i)
            {
                body = &ragdollBodies[i];
                if (!body)
                    MyAssertHandler("c:\\trees\\cod3\\src\\ragdoll\\ragdoll.h", 285, 0, "%s", "body");
                if (body->references > 0)
                    Ragdoll_BodyUpdate(msec, body);
            }
            Phys_SetCollisionCallback(PHYS_WORLD_RAGDOLL, Ragdoll_GenerateAllSelfCollisionContacts);
            Phys_RunToTime(0, PHYS_WORLD_RAGDOLL, ragdollTime);
        }
        //Profile_EndInternal(0);
    }
}

