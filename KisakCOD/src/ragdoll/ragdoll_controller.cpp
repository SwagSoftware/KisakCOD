#include "ragdoll.h"

#include <client_mp/client_mp.h>

#include <gfx_d3d/r_scene.h>

#include <xanim/dobj.h>
#include <xanim/dobj_utils.h>

RagdollBody *__cdecl Ragdoll_HandleBody(int ragdollHandle)
{
    if ((unsigned int)(ragdollHandle - 1) >= 0x20)
        MyAssertHandler(
            ".\\ragdoll\\ragdoll_controller.cpp",
            31,
            0,
            "ragdollHandle - 1 doesn't index RAGDOLL_MAX\n\t%i not in [0, %i)",
            ragdollHandle - 1,
            32);
    return &ragdollBodies[ragdollHandle - 1];
}

BoneOrientation *__cdecl Ragdoll_BodyBoneOrientations(RagdollBody *body)
{
    if (!body)
        MyAssertHandler(".\\ragdoll\\ragdoll_controller.cpp", 42, 0, "%s", "body");
    if (body->curOrientationBuffer >= 2u)
        MyAssertHandler(
            ".\\ragdoll\\ragdoll_controller.cpp",
            43,
            0,
            "body->curOrientationBuffer doesn't index 2\n\t%i not in [0, %i)",
            body->curOrientationBuffer,
            2);
    return body->boneOrientations[body->curOrientationBuffer];
}

BoneOrientation *__cdecl Ragdoll_BodyPrevBoneOrientations(RagdollBody *body)
{
    if (!body)
        MyAssertHandler(".\\ragdoll\\ragdoll_controller.cpp", 50, 0, "%s", "body");
    if (body->curOrientationBuffer >= 2u)
        MyAssertHandler(
            ".\\ragdoll\\ragdoll_controller.cpp",
            51,
            0,
            "body->curOrientationBuffer doesn't index 2\n\t%i not in [0, %i)",
            body->curOrientationBuffer,
            2);
    return body->boneOrientations[body->curOrientationBuffer ^ 1];
}

void __cdecl Ragdoll_DoControllers(const cpose_t *pose, DObj_s *obj, int *partBits)
{
    RagdollBody *v3; // eax
    float v4; // [esp+0h] [ebp-48h]
    float v5; // [esp+8h] [ebp-40h]
    unsigned int LocalClientNum; // [esp+14h] [ebp-34h]
    BoneOrientation *boneOrientation; // [esp+1Ch] [ebp-2Ch]
    float viewOffset[3]; // [esp+20h] [ebp-28h] BYREF
    RagdollBody *body; // [esp+2Ch] [ebp-1Ch]
    DObjAnimMat *boneMat; // [esp+30h] [ebp-18h]
    DObjAnimMat *skel; // [esp+34h] [ebp-14h]
    LerpBone *lerpBone; // [esp+38h] [ebp-10h]
    int boneIdx; // [esp+3Ch] [ebp-Ch]
    int i; // [esp+40h] [ebp-8h]
    Bone *bone; // [esp+44h] [ebp-4h]

    if (!pose)
        MyAssertHandler(".\\ragdoll\\ragdoll_controller.cpp", 69, 0, "%s", "pose");
    if (!obj)
        MyAssertHandler(".\\ragdoll\\ragdoll_controller.cpp", 70, 0, "%s", "obj");
    if (pose->isRagdoll && (pose->ragdollHandle || pose->killcamRagdollHandle))
    {
        if (pose->killcamRagdollHandle)
            v3 = Ragdoll_HandleBody(pose->killcamRagdollHandle);
        else
            v3 = Ragdoll_HandleBody(pose->ragdollHandle);
        body = v3;
        if (!v3)
            MyAssertHandler("c:\\trees\\cod3\\src\\ragdoll\\ragdoll.h", 271, 0, "%s", "body");
        if (body->state >= BS_TUNNEL_TEST)
        {
            skel = DObjGetRotTransArray(obj);
            if (skel)
            {
                LocalClientNum = R_GetLocalClientNum();
                if (LocalClientNum)
                    MyAssertHandler(
                        "c:\\trees\\cod3\\src\\ragdoll\\../cgame_mp/cg_local_mp.h",
                        1071,
                        0,
                        "%s\n\t(localClientNum) = %i",
                        "(localClientNum == 0)",
                        LocalClientNum);
                viewOffset[0] = cgArray[0].refdef.viewOffset[0];
                viewOffset[1] = cgArray[0].refdef.viewOffset[1];
                viewOffset[2] = cgArray[0].refdef.viewOffset[2];
                bone = body->bones;
                boneOrientation = Ragdoll_BodyBoneOrientations(body);
                i = 0;
                while (i < body->numBones)
                {
                    boneIdx = bone->animBones[0];
                    if (DObjSetSkelRotTransIndex(obj, partBits, boneIdx))
                    {
                        boneMat = &skel[boneIdx];
                        Vec3Sub(boneOrientation->origin, viewOffset, boneMat->trans);
                        boneMat->quat[0] = boneOrientation->orientation[0];
                        boneMat->quat[1] = boneOrientation->orientation[1];
                        boneMat->quat[2] = boneOrientation->orientation[2];
                        boneMat->quat[3] = boneOrientation->orientation[3];
                        v5 = Vec4LengthSq(boneMat->quat);
                        if (v5 == 0.0)
                        {
                            boneMat->quat[3] = 1.0;
                            boneMat->transWeight = 2.0;
                        }
                        else
                        {
                            boneMat->transWeight = 2.0 / v5;
                        }
                    }
                    ++i;
                    ++bone;
                    ++boneOrientation;
                }
                lerpBone = body->lerpBones;
                i = 0;
                while (i < body->numLerpBones)
                {
                    boneIdx = lerpBone->animBone;
                    if (DObjSetSkelRotTransIndex(obj, partBits, boneIdx))
                    {
                        boneMat = &skel[boneIdx];
                        Vec3Sub(boneOrientation->origin, viewOffset, boneMat->trans);
                        boneMat->quat[0] = boneOrientation->orientation[0];
                        boneMat->quat[1] = boneOrientation->orientation[1];
                        boneMat->quat[2] = boneOrientation->orientation[2];
                        boneMat->quat[3] = boneOrientation->orientation[3];
                        v4 = Vec4LengthSq(boneMat->quat);
                        if (v4 == 0.0)
                        {
                            boneMat->quat[3] = 1.0;
                            boneMat->transWeight = 2.0;
                        }
                        else
                        {
                            boneMat->transWeight = 2.0 / v4;
                        }
                    }
                    ++i;
                    ++lerpBone;
                    ++boneOrientation;
                }
            }
        }
    }
}

