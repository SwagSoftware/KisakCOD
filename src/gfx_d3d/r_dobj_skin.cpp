#include "r_dobj_skin.h"

#include <xanim/dobj.h>
#include <xanim/xmodel.h>
#include <xanim/dobj_utils.h>
#include <universal/profile.h>
#include "r_model.h"
#include "r_dvars.h"
#include "r_workercmds.h"
#include "r_buffers.h"
#include "r_model_pose.h"
#include "r_dpvs.h"

static void __cdecl R_FlagXModelAsSkinned(GfxSceneEntity *sceneEnt, unsigned int surfaceCount)
{
    iassert(sceneEnt->cull.state == CULL_STATE_SKINNED_PENDING);
    sceneEnt->cull.state = surfaceCount + 4;
}

static GfxSkinCacheEntry *__cdecl CG_GetSkinCacheEntry(cpose_t *pose)
{
    return &pose->skinCacheEntry;
}

static int __cdecl R_AllocSkinnedCachedVerts(int vertCount)
{
    LONG offset; // [esp+4h] [ebp-8h]

    iassert(vertCount > 0);
    if (!gfxBuf.skinnedCacheLockAddr)
        return -1;

    offset = InterlockedExchangeAdd(&frontEndDataOut->skinnedCacheVb->used, 32 * vertCount);
    if ((unsigned int)(32 * vertCount + offset) <= 0x480000)
        return offset;
    R_WarnOncePerFrame(R_WARN_MAX_SKINNED_CACHE_VERTICES);
    return -1;
}

int  R_PreSkinXSurface(
    const DObj_s *obj,
    XSurface *surf,
    const GfxModelSurfaceInfo *surfaceInfo,
    unsigned int *numSkinnedVerts,
    GfxModelSkinnedSurface *surfPos_)
{
    float origin[4]; // [esp+20h] [ebp-154h] BYREF
    DObjAnimMat *mat_1; // [esp+C4h] [ebp-B0h]
    const DObjAnimMat *mat; // [esp+150h] [ebp-24h]
    int offset; // [esp+154h] [ebp-20h]
    DObjAnimMat *RotTransArray; // [esp+158h] [ebp-1Ch]
    GfxModelRigidSurface *rigidSurf; // [esp+15Ch] [ebp-18h]

    GfxModelSkinnedSurface *surfPos = surfPos_;

    iassert(obj);
    iassert(surf);
    iassert(surfaceInfo);

    if (!surf->deformed && useFastFile->current.enabled && surf->vertListCount == 1)
    {
        surf->vertList = surf->vertList;
        rigidSurf = (GfxModelRigidSurface *)surfPos;
        surfPos->skinnedCachedOffset = -2;
        iassert(&rigidSurf->surf == reinterpret_cast<GfxModelSkinnedSurface *>(surfPos));
        rigidSurf->placement.scale = 1.0;
        RotTransArray = &DObjGetRotTransArray(obj)[surfaceInfo->boneIndex];
        offset = surf->vertList->boneOffset >> 6;
        mat = &surfaceInfo->baseMat[offset];

        DObjSkelMat invBaseMat;
        DObjSkelMat skelMat;
        ConvertQuatToInverseSkelMat(mat, &invBaseMat);

        mat_1 = &RotTransArray[offset];

        ConvertQuatToSkelMat(mat_1, &skelMat);
        QuatMultiplyInverse(mat->quat, mat_1->quat, origin);

        Vec4Normalize(origin);

        rigidSurf->placement.base.quat[0] = origin[0];
        rigidSurf->placement.base.quat[1] = origin[1];
        rigidSurf->placement.base.quat[2] = origin[2];
        rigidSurf->placement.base.quat[3] = origin[3];

        float tmp[3];
        R_TransformSkelMat(invBaseMat.origin, &skelMat, tmp);
        Vec3Add(tmp, scene.def.viewOffset, rigidSurf->placement.base.origin);
        return 56;
    }
    else
    {
        surfPos->skinnedCachedOffset = *numSkinnedVerts;
        *numSkinnedVerts += surf->vertCount;
        return 24;
    }
}

int  R_SkinSceneDObjModels(
    GfxSceneEntity *sceneEnt,
    const DObj_s *obj,
    DObjAnimMat *boneMatrix)
{
    iassert(!sceneEnt->cull.skinnedSurfs.firstSurf);
    iassert(obj);
    iassert(boneMatrix);
    iassert(!useFastFile->current.enabled || !DObjBad(obj));

    Profile_Begin(89);

    unsigned char surfsBuffer[131072];
    GfxModelSkinnedSurface *surfPos = (GfxModelSkinnedSurface *)surfsBuffer;
    GfxModelSurfaceInfo targBoneIndexHigh;

    SkinXModelCmd skinCmd;
    memset(&skinCmd, 0, sizeof(skinCmd));

    uint32 surfPartBits[7] = {};

    unsigned int NumModels = DObjGetNumModels(obj);
    if (!NumModels)
    {
        return 0;
    }
    unsigned int lod = 0;
    unsigned int boneIndex = 0;

    unsigned int totalSurfaceCount = 0;
    unsigned int numSkinnedVerts = 0;

    unsigned int boneCount = 0;
    int cullLod;
    unsigned int surfaceCount;
    unsigned int boneIndex_div32;
    unsigned int boneIndex_mod32;
    unsigned int boneIndex_rem32;

    unsigned int partbits[4];
    unsigned int partBitsCheck[4] = {};
    GfxModelRigidSurface *rigidSurf;

    while (lod < NumModels)
    {
        const XModel *model = DObjGetModel(obj, lod);
        iassert(model);

        boneCount = XModelNumBones(model);
        cullLod = sceneEnt->cull.lods[lod];

        if (cullLod >= 0)
        {
            targBoneIndexHigh.boneIndex = boneIndex;
            targBoneIndexHigh.baseMat = XModelGetBasePose(model);
            targBoneIndexHigh.boneCount = boneCount;
            targBoneIndexHigh.gfxEntIndex = sceneEnt->gfxEntIndex;
            targBoneIndexHigh.lightingHandle = 0;

            XSurface *surfaces = NULL;
            surfaceCount = XModelGetSurfaces(model, &surfaces, cullLod);

            iassert(surfaces);
            iassert(surfaceCount);

            totalSurfaceCount += surfaceCount;

            boneIndex_div32 = boneIndex >> 5;
            boneIndex_mod32 = boneIndex & 0x1F;
            boneIndex_rem32 = 32 - boneIndex_mod32;

            DObjGetHidePartBits(obj, partbits);

            for (unsigned int i = 0; i < surfaceCount; i++)
            {
                XSurface *surface = &surfaces[i];

                surfPartBits[3] = surface->partBits[0];
                surfPartBits[4] = surface->partBits[1];
                surfPartBits[5] = surface->partBits[2];
                surfPartBits[6] = surface->partBits[3];

                if (!boneIndex_mod32)
                {
                    partBitsCheck[0] = surfPartBits[3 - boneIndex_div32];
                    partBitsCheck[1] = surfPartBits[4 - boneIndex_div32];
                    partBitsCheck[2] = surfPartBits[5 - boneIndex_div32];
                    partBitsCheck[3] = surfPartBits[6 - boneIndex_div32];
                }
                else
                {
                    partBitsCheck[0] = surfPartBits[3 - boneIndex_div32] >> boneIndex_mod32;
                    partBitsCheck[1] = (surfPartBits[4 - boneIndex_div32] >> boneIndex_mod32) | (surfPartBits[3 - boneIndex_div32] << boneIndex_rem32);
                    partBitsCheck[2] = (surfPartBits[5 - boneIndex_div32] >> boneIndex_mod32) | (surfPartBits[4 - boneIndex_div32] << boneIndex_rem32);
                    partBitsCheck[3] = (surfPartBits[6 - boneIndex_div32] >> boneIndex_mod32) | (surfPartBits[5 - boneIndex_div32] << boneIndex_rem32);
                }

                if (partBitsCheck[3] & partbits[3]
                    | partBitsCheck[2] & partbits[2]
                    | partBitsCheck[1] & partbits[1]
                    | partBitsCheck[0] & partbits[0])
                {
                    surfPos->skinnedCachedOffset = -3;
                    surfPos = (GfxModelSkinnedSurface *)((char *)surfPos + 4);
                }
                else
                {
                    skinCmd.surfacePartBits[0] |= partBitsCheck[0];
                    skinCmd.surfacePartBits[1] |= partBitsCheck[1];
                    skinCmd.surfacePartBits[2] |= partBitsCheck[2];
                    skinCmd.surfacePartBits[3] |= partBitsCheck[3];

                    int surfBufSize = R_PreSkinXSurface(obj, surface, &targBoneIndexHigh, &numSkinnedVerts, surfPos);

                    GfxModelSkinnedSurface *skinnedSurface = (GfxModelSkinnedSurface *)surfPos;
                    skinnedSurface->xsurf = surface;
                    skinnedSurface->info = targBoneIndexHigh;

                    iassert(surfBufSize);
                    surfPos = (GfxModelSkinnedSurface *)((char *)surfPos + surfBufSize);
                }
            }
        }

        lod++;
        boneIndex += boneCount;
    }

    iassert(DObjSkelAreBonesUpToDate(obj, skinCmd.surfacePartBits));

    if (numSkinnedVerts)
    {
        if (gfxBuf.skinCache)
        {
            int skinnedCachedOffset = R_AllocSkinnedCachedVerts(numSkinnedVerts);
            if (skinnedCachedOffset < 0)
            {
                Profile_EndInternal(0);
                return 0;
            }
            unsigned int oldSkinnedCachedOffset = 0x80000001;
            if (gfxBuf.fastSkin)
            {
                GfxSkinCacheEntry *skinCacheEntry = CG_GetSkinCacheEntry(sceneEnt->info.pose);
                if (skinCacheEntry->ageCount < 3u
                    && gfxBuf.skinnedCacheNormalsFrameCount - skinCacheEntry->frameCount == 1
                    && skinCacheEntry->numSkinnedVerts == numSkinnedVerts)
                {
                    oldSkinnedCachedOffset = skinCacheEntry->skinnedCachedOffset;
                    ++skinCacheEntry->ageCount;
                }
                else
                {
                    skinCacheEntry->ageCount = 0;
                }
                skinCacheEntry->frameCount = gfxBuf.skinnedCacheNormalsFrameCount;
                skinCacheEntry->numSkinnedVerts = numSkinnedVerts;
                iassert(skinCacheEntry->numSkinnedVerts == numSkinnedVerts);
                skinCacheEntry->skinnedCachedOffset = skinnedCachedOffset;
            }
            GfxModelSkinnedSurface *surfPos2 = (GfxModelSkinnedSurface *)surfsBuffer;
            for (unsigned int offset = 0; offset < totalSurfaceCount; ++offset)
            {
                rigidSurf = (GfxModelRigidSurface *)surfPos2;
                if (surfPos2->skinnedCachedOffset == -2)
                {
                    static_assert(sizeof(GfxModelRigidSurface) == 56);
                    surfPos2 = (GfxModelSkinnedSurface *)((char *)surfPos2 + sizeof(GfxModelRigidSurface));
                }
                else if (surfPos2->skinnedCachedOffset == -3)
                {
                    // idk
                    surfPos2 = (GfxModelSkinnedSurface *)((char *)surfPos2 + 4);
                }
                else
                {
                    ++surfPos2;
                    int size = 32 * rigidSurf->surf.skinnedCachedOffset;
                    rigidSurf->surf.oldSkinnedCachedOffset = size + oldSkinnedCachedOffset;
                    rigidSurf->surf.skinnedCachedOffset = size + skinnedCachedOffset;
                }
            }
            iassert((byte *)surfPos == (byte *)surfPos2);
        }
        else
        {
            static_assert(sizeof(GfxPackedVertex) == 32);
            unsigned int vertsSize = sizeof(GfxPackedVertex) * numSkinnedVerts;
            iassert(frontEndDataOut->tempSkinBuf);
            unsigned long firstSurf = InterlockedExchangeAdd(&frontEndDataOut->tempSkinPos, vertsSize);
            if ((firstSurf + vertsSize) > 0x480000)
            {
                R_WarnOncePerFrame(R_WARN_TEMP_SKIN_BUF_SIZE);
                Profile_EndInternal(0);
                return 0;
            }
            Z_VirtualCommit(&frontEndDataOut->tempSkinBuf[firstSurf], vertsSize);
            GfxModelSkinnedSurface *surfPos2 = (GfxModelSkinnedSurface *)surfsBuffer;
            for (unsigned int offset = 0; offset < totalSurfaceCount; ++offset)
            {
                GfxModelRigidSurface *rigidSurf2_ = (GfxModelRigidSurface *)surfPos2;
                if (surfPos2->skinnedCachedOffset == -2)
                {
                    static_assert(sizeof(GfxModelRigidSurface) == 56);
                    surfPos2 = (GfxModelSkinnedSurface *)((char *)surfPos2 + sizeof(GfxModelRigidSurface));
                }
                else if (surfPos2->skinnedCachedOffset == -3)
                {
                    // idk
                    surfPos2 = (GfxModelSkinnedSurface *)((char *)surfPos2 + 4);
                }
                else
                {
                    ++surfPos2;
                    rigidSurf2_->surf.oldSkinnedCachedOffset = (int)&frontEndDataOut->tempSkinBuf[sizeof(GfxPackedVertex) * surfPos2->skinnedCachedOffset + firstSurf];
                    rigidSurf2_->surf.skinnedCachedOffset = -1;
                }
            }
            iassert((byte *)surfPos == (byte *)surfPos2);
        }
    }

    unsigned int totalSurfSize = ((char *)surfPos - (char *)surfsBuffer);
    unsigned int startSurfPos = InterlockedExchangeAdd(&frontEndDataOut->surfPos, totalSurfSize);

    if (startSurfPos + totalSurfSize > 0x20000)
    {
        R_WarnOncePerFrame(R_WARN_MAX_SCENE_SURFS_SIZE);
        Profile_EndInternal(0);
        return 0;
    }

    iassert(!(totalSurfSize & 3));

    sceneEnt->cull.skinnedSurfs.firstSurf = &frontEndDataOut->surfsBuffer[startSurfPos];
    memcpy(&frontEndDataOut->surfsBuffer[startSurfPos], surfsBuffer, totalSurfSize);

    Profile_EndInternal(0);

    iassert(totalSurfaceCount);

    if (r_xdebug->current.integer)
        R_XModelDebug(obj, skinCmd.surfacePartBits);

    if (numSkinnedVerts)
    {
        skinCmd.modelSurfs = &frontEndDataOut->surfsBuffer[startSurfPos];
        skinCmd.surfCount = totalSurfaceCount;
        skinCmd.mat = boneMatrix;
        R_AddWorkerCmd(16, (unsigned char *)&skinCmd);
    }

    return totalSurfaceCount;
}

void __cdecl R_SkinGfxEntityCmd(GfxSceneEntity **data)
{
    DObjAnimMat *boneMatrix; // [esp+0h] [ebp-14h]
    const DObj_s *obj; // [esp+4h] [ebp-10h] BYREF
    GfxSceneEntity *localSceneEnt; // [esp+8h] [ebp-Ch] BYREF
    GfxSceneEntity *sceneEnt; // [esp+Ch] [ebp-8h]
    GfxSceneEntity **pSceneEnt; // [esp+10h] [ebp-4h]

    if (!data)
        MyAssertHandler(".\\r_dobj_skin.cpp", 608, 0, "%s", "data");
    pSceneEnt = data;
    sceneEnt = *data;
    boneMatrix = R_UpdateSceneEntBounds(sceneEnt, &localSceneEnt, &obj, 0);
    if (boneMatrix)
    {
        if (!localSceneEnt)
            MyAssertHandler(".\\r_dobj_skin.cpp", 615, 0, "%s", "localSceneEnt");
        R_SkinSceneDObj(sceneEnt, localSceneEnt, obj, boneMatrix, 0);
    }
}

void __cdecl R_SkinSceneDObj(
    GfxSceneEntity *sceneEnt,
    GfxSceneEntity *localSceneEnt,
    const DObj_s *obj,
    DObjAnimMat *boneMatrix,
    int waitForCullState)
{
    volatile unsigned int state; // [esp+0h] [ebp-8h]
    int surfaceCount; // [esp+4h] [ebp-4h]

    iassert(localSceneEnt);
    iassert(boneMatrix);
    iassert(obj);

    if (localSceneEnt->cull.state < 4)
    {
        if (InterlockedCompareExchange((volatile unsigned int *)&sceneEnt->cull, 3, 2) == 2)
        {
            surfaceCount = R_SkinSceneDObjModels(localSceneEnt, obj, boneMatrix);
            R_FlagXModelAsSkinned(localSceneEnt, surfaceCount);
        }
        else if (waitForCullState)
        {
            do
            {
                state = sceneEnt->cull.state;
                iassert(state >= CULL_STATE_SKINNED_PENDING);
            } while (state == CULL_STATE_SKINNED_PENDING);

            iassert(state >= CULL_STATE_DONE);
        }
    }
}

int __cdecl DObjBad(const DObj_s *obj)
{
    int j; // [esp+0h] [ebp-4h]

    for (j = obj->numModels - 1; j >= 0; --j)
    {
        if (XModelBad(obj->models[j]))
            return 1;
    }
    return 0;
}