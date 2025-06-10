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

void __cdecl R_FlagXModelAsSkinned(GfxSceneEntity *sceneEnt, unsigned int surfaceCount)
{
    if (sceneEnt->cull.state != 3)
        MyAssertHandler(
            ".\\r_dobj_skin.cpp",
            520,
            0,
            "%s\n\t(sceneEnt->cull.state) = %i",
            "(sceneEnt->cull.state == CULL_STATE_SKINNED_PENDING)",
            sceneEnt->cull.state);
    sceneEnt->cull.state = surfaceCount + 4;
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

GfxSkinCacheEntry *__cdecl CG_GetSkinCacheEntry(cpose_t *pose)
{
    return &pose->skinCacheEntry;
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
    unsigned char *surfPos = surfsBuffer;

    SkinXModelCmd skinCmd;
    memset(&skinCmd, 0, sizeof(skinCmd));

    uint32 surfPartBits[7] = {};

    unsigned int NumModels = DObjGetNumModels(obj);
    if (!NumModels)
    {
        return 0;
    }
    unsigned int lod = 0;
    unsigned int totalBones = 0;

    unsigned int totalSurfaceCount = 0;
    unsigned int numSkinnedVerts = 0;

    while (lod < NumModels)
    {
        const XModel* model = DObjGetModel(obj, lod);
        iassert(model);

        int bones = XModelNumBones(model);

        int cullLod = sceneEnt->cull.lods[lod];
        
        if (cullLod >= 0)
        {

            GfxModelSurfaceInfo targBoneIndexHigh;
            targBoneIndexHigh.boneIndex = totalBones;
            targBoneIndexHigh.baseMat = XModelGetBasePose(model);
            targBoneIndexHigh.boneCount = bones;
            targBoneIndexHigh.gfxEntIndex = sceneEnt->gfxEntIndex;
            targBoneIndexHigh.lightingHandle = 0;

            XSurface* surfaces = nullptr;
            int surfaceCount = XModelGetSurfaces(model, &surfaces, cullLod);
            iassert(surfaces);
            iassert(surfaceCount);

            totalSurfaceCount += surfaceCount;

            unsigned int boneIndex_div32 = totalBones >> 5;
            unsigned int boneIndex_mod32 = totalBones & 0x1F;
            unsigned int boneIndex_rem32 = 32 - boneIndex_mod32;

            unsigned int partbits[4] = { 0,0,0,0 };
            DObjGetHidePartBits(obj, partbits);

            for (unsigned int i = 0; i < surfaceCount; i++)
            {
                XSurface* surface = &surfaces[i];

                surfPartBits[3] = surface->partBits[0];
                surfPartBits[4] = surface->partBits[1];
                surfPartBits[5] = surface->partBits[2];
                surfPartBits[6] = surface->partBits[3];

                unsigned int partBitsCheck[4] = {};
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
                    partBitsCheck[1] = (surfPartBits[4 - boneIndex_div32] >> boneIndex_mod32)
                        | (surfPartBits[3 - boneIndex_div32] << boneIndex_rem32);
                    partBitsCheck[2] = (surfPartBits[5 - boneIndex_div32] >> boneIndex_mod32)
                        | (surfPartBits[4 - boneIndex_div32] << boneIndex_rem32);
                    partBitsCheck[3] = (surfPartBits[6 - boneIndex_div32] >> boneIndex_mod32)
                        | (surfPartBits[5 - boneIndex_div32] << boneIndex_rem32);
                }

                if (partBitsCheck[3] & partbits[3]
                    | partBitsCheck[2] & partbits[2]
                    | partBitsCheck[1] & partbits[1]
                    | partBitsCheck[0] & partbits[0])
                {
                    *(int*)surfPos = -3;
                    surfPos += 4;
                }
                else
                {
                    skinCmd.surfacePartBits[0] |= partBitsCheck[0];
                    skinCmd.surfacePartBits[1] |= partBitsCheck[1];
                    skinCmd.surfacePartBits[2] |= partBitsCheck[2];
                    skinCmd.surfacePartBits[3] |= partBitsCheck[3];

                    int surfBufSize = R_PreSkinXSurface(obj, surface, &targBoneIndexHigh, &numSkinnedVerts, (float*)surfPos);

                    GfxModelSkinnedSurface* skinnedSurface = (GfxModelSkinnedSurface*)surfPos;
                    skinnedSurface->xsurf = surface;
                    skinnedSurface->info = targBoneIndexHigh;

                    iassert(surfBufSize);
                    surfPos += surfBufSize;
                }
            }
        }

        lod++;
        totalBones += bones;
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
                GfxSkinCacheEntry* skinCacheEntry = CG_GetSkinCacheEntry(sceneEnt->info.pose);
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
            GfxModelSkinnedSurface* surfPos2 = (GfxModelSkinnedSurface*)surfsBuffer;
            for (unsigned int offset = 0; offset < totalSurfaceCount; ++offset)
            {
                //GfxModelSkinnedSurface* entry = (GfxModelSkinnedSurface*)surfPos2;
                if (surfPos2->skinnedCachedOffset == -2)
                {
                    // NOTE(mrsteyk): Skip bigger struct. (GfxModelRigidSurface?)
                    surfPos2 = (GfxModelSkinnedSurface*)((char*)surfPos2 + 56);
                }
                else if (surfPos2->skinnedCachedOffset == -3)
                {
                    // idk
                    surfPos2 = (GfxModelSkinnedSurface*)((char*)surfPos2 + 4);
                }
                else
                {
                    // GfxModelSkinnedSurface!!!
                    int tempSkinPos = 32 * surfPos2->skinnedCachedOffset;
                    surfPos2->oldSkinnedCachedOffset = tempSkinPos + oldSkinnedCachedOffset;
                    surfPos2->skinnedCachedOffset = tempSkinPos + skinnedCachedOffset;
                    surfPos2++;
                }
            }
            iassert(surfPos == (byte*)surfPos2);
        }
        else
        {
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
            GfxModelSkinnedSurface* surfPos2 = (GfxModelSkinnedSurface*)surfsBuffer;
            for (unsigned int offset = 0; offset < totalSurfaceCount; ++offset)
            {
                if (surfPos2->skinnedCachedOffset == -2)
                {
                    // NOTE(mrsteyk): Skip bigger struct. (GfxModelRigidSurface?)
                    surfPos2 = (GfxModelSkinnedSurface*)((char*)surfPos2 + 56);
                }
                else if (surfPos2->skinnedCachedOffset == -3)
                {
                    // idk
                    surfPos2 = (GfxModelSkinnedSurface*)((char*)surfPos2 + 4);
                }
                else
                {
                    // GfxModelSkinnedSurface!!!
                    surfPos2->oldSkinnedCachedOffset = (int)&frontEndDataOut->tempSkinBuf[32 * surfPos2->skinnedCachedOffset + firstSurf];
                    surfPos2->skinnedCachedOffset = -1;
                    ++surfPos2;
                }
            }
            iassert(surfPos == (byte*)surfPos2);
        }
    }

    unsigned int totalSurfSize = surfPos - surfsBuffer;
    unsigned int startSurfPos = InterlockedExchangeAdd(&frontEndDataOut->surfPos, totalSurfSize);
    if (startSurfPos + totalSurfSize > 131072)
    {
        R_WarnOncePerFrame(R_WARN_MAX_SCENE_SURFS_SIZE);
        Profile_EndInternal(0);
        return 0;
    }

    iassert(!(totalSurfSize & 3));

    unsigned char* modelSurfs = &frontEndDataOut->surfsBuffer[startSurfPos];
    sceneEnt->cull.skinnedSurfs.firstSurf = modelSurfs;
    memcpy(modelSurfs, surfsBuffer, totalSurfSize);

    Profile_EndInternal(0);

    iassert(totalSurfaceCount);
    
    if (r_xdebug->current.integer)
        R_XModelDebug(obj, skinCmd.surfacePartBits);

    if (numSkinnedVerts)
    {
        skinCmd.modelSurfs = modelSurfs;
        skinCmd.surfCount = totalSurfaceCount;
        skinCmd.mat = boneMatrix;
        R_AddWorkerCmd(16, (unsigned char*)&skinCmd);
    }

    return totalSurfaceCount;
}

int __cdecl R_AllocSkinnedCachedVerts(int vertCount)
{
    LONG offset; // [esp+4h] [ebp-8h]

    if (vertCount <= 0)
        MyAssertHandler(".\\r_dobj_skin.cpp", 27, 0, "%s", "vertCount > 0");
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
    float *surfPos_)
{
    float v7[3]; // [esp+10h] [ebp-164h] BYREF
    float v9[4]; // [esp+20h] [ebp-154h] BYREF
    DObjSkelMat quat_12; // [esp+38h] [ebp-13Ch] BYREF
    
    DObjAnimMat* mat_1; // [esp+C4h] [ebp-B0h]
    DObjSkelMat v29; // [esp+C8h] [ebp-ACh] BYREF
    float v30; // [esp+10Ch] [ebp-68h]
    float v31; // [esp+110h] [ebp-64h]
    float v32; // [esp+114h] [ebp-60h]
    float v33; // [esp+118h] [ebp-5Ch]
    float v34; // [esp+11Ch] [ebp-58h]
    float v35; // [esp+120h] [ebp-54h]
    float v36; // [esp+124h] [ebp-50h]
    float v37; // [esp+128h] [ebp-4Ch]
    float v38; // [esp+12Ch] [ebp-48h]
    float scaledQuat[3]; // [esp+130h] [ebp-44h] BYREF
    //float transWeight; // [esp+13Ch] [ebp-38h]
    //float v41; // [esp+140h] [ebp-34h]
    //float v42; // [esp+144h] [ebp-30h]
    //float v43; // [esp+148h] [ebp-2Ch]
    //float v44; // [esp+14Ch] [ebp-28h]
    const DObjAnimMat* mat; // [esp+150h] [ebp-24h]
    int v46; // [esp+154h] [ebp-20h]
    DObjAnimMat* RotTransArray; // [esp+158h] [ebp-1Ch]
    GfxModelRigidSurface* rigidSurf; // [esp+15Ch] [ebp-18h]
    unsigned int v49; // [esp+160h] [ebp-14h]
    //bool enabled; // [esp+167h] [ebp-Dh]
    DObjSkelMat skelMat;
    DObjSkelMat invBaseMat;
    float origin[3];
    float quat[4];

    GfxModelSkinnedSurface* surfPos = (GfxModelSkinnedSurface*)surfPos_;

    iassert(obj);
    iassert(surf);
    iassert(surfaceInfo);

    if (!surf->deformed && useFastFile->current.enabled && surf->vertListCount == 1)
    {
        surf->vertList = surf->vertList;
        rigidSurf = (GfxModelRigidSurface*)surfPos;
        surfPos->skinnedCachedOffset = -2;
        iassert(&rigidSurf->surf == reinterpret_cast<GfxModelSkinnedSurface *>(surfPos));
        rigidSurf->placement.scale = 1.0;
        RotTransArray = &DObjGetRotTransArray(obj)[surfaceInfo->boneIndex];
        //RotTransArray += (surf->vertList->boneOffset / 64);
        mat = &surfaceInfo->baseMat[surf->vertList->boneOffset / 64];

        iassert(!IS_NAN(mat->quat[0]) && !IS_NAN(mat->quat[1]) && !IS_NAN(mat->quat[2]) && !IS_NAN(mat->quat[3]));
        iassert(!IS_NAN(mat->transWeight));

        Vec3Scale(mat->quat, mat->transWeight, scaledQuat);

        ConvertQuatToInverseSkelMat(mat, &invBaseMat);
        ConvertQuatToSkelMat(RotTransArray, &skelMat);

        LocalQuatMultiplyInverse(mat->quat, RotTransArray->quat, quat);
        Vec4Normalize(quat);

        rigidSurf->placement.base.quat[0] = quat[0];
        rigidSurf->placement.base.quat[1] = quat[1];
        rigidSurf->placement.base.quat[2] = quat[2];
        rigidSurf->placement.base.quat[3] = quat[3];

        R_TransformSkelMat(invBaseMat.origin, &skelMat, origin);

        rigidSurf->placement.base.origin[0] = origin[0];
        rigidSurf->placement.base.origin[1] = origin[1];
        rigidSurf->placement.base.origin[2] = origin[2];

        Vec3Add(origin, scene.def.viewOffset, rigidSurf->placement.base.origin);

        return 56;
    }
    else
    {
        surfPos->skinnedCachedOffset = *numSkinnedVerts;
        *numSkinnedVerts += surf->vertCount;
        return 24;
    }
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

