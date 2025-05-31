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

    if (!localSceneEnt)
        MyAssertHandler(".\\r_dobj_skin.cpp", 552, 0, "%s", "localSceneEnt");
    if (!boneMatrix)
        MyAssertHandler(".\\r_dobj_skin.cpp", 553, 0, "%s", "boneMatrix");
    if (!obj)
        MyAssertHandler(".\\r_dobj_skin.cpp", 554, 0, "%s", "obj");
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
                if (state < 3)
                    MyAssertHandler(
                        ".\\r_dobj_skin.cpp",
                        578,
                        0,
                        "%s\n\t(state) = %i",
                        "(state >= CULL_STATE_SKINNED_PENDING)",
                        state);
            } while (state == 3);
            if (state < 4)
                MyAssertHandler(".\\r_dobj_skin.cpp", 581, 0, "%s\n\t(state) = %i", "(state >= CULL_STATE)", state);
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
    float v39[3]; // [esp+130h] [ebp-44h] BYREF
    float transWeight; // [esp+13Ch] [ebp-38h]
    float v41; // [esp+140h] [ebp-34h]
    float v42; // [esp+144h] [ebp-30h]
    float v43; // [esp+148h] [ebp-2Ch]
    float v44; // [esp+14Ch] [ebp-28h]
    const DObjAnimMat* mat; // [esp+150h] [ebp-24h]
    int v46; // [esp+154h] [ebp-20h]
    DObjAnimMat* RotTransArray; // [esp+158h] [ebp-1Ch]
    GfxModelRigidSurface* rigidSurf; // [esp+15Ch] [ebp-18h]
    unsigned int v49; // [esp+160h] [ebp-14h]
    bool enabled; // [esp+167h] [ebp-Dh]

    GfxModelSkinnedSurface* surfPos = (GfxModelSkinnedSurface*)surfPos_;

    if (!obj)
        MyAssertHandler(".\\r_dobj_skin.cpp", 65, 0, "%s", "obj");
    if (!surf)
        MyAssertHandler(".\\r_dobj_skin.cpp", 66, 0, "%s", "surf");
    if (!surfaceInfo)
        MyAssertHandler(".\\r_dobj_skin.cpp", 67, 0, "%s", "surfaceInfo");
    if (!surf->deformed && (enabled = useFastFile->current.enabled, (v49 = enabled) != 0) && surf->vertListCount == 1)
    {
        surf->vertList = surf->vertList;
        rigidSurf = (GfxModelRigidSurface*)surfPos;
        surfPos->skinnedCachedOffset = -2;
        if (rigidSurf != (GfxModelRigidSurface*)surfPos)
            MyAssertHandler(
                ".\\r_dobj_skin.cpp",
                82,
                0,
                "%s",
                "&rigidSurf->surf == reinterpret_cast< GfxModelSkinnedSurface * >( surfPos )");
        rigidSurf->placement.scale = 1.0;
        RotTransArray = &DObjGetRotTransArray(obj)[surfaceInfo->boneIndex];
        v46 = surf->vertList->boneOffset >> 6;
        mat = &surfaceInfo->baseMat[v46];

#if 1
        v44 = mat->quat[0];
        if ((LODWORD(v44) & 0x7F800000) == 0x7F800000
            || (v43 = mat->quat[1], (LODWORD(v43) & 0x7F800000) == 0x7F800000)
            || (v42 = mat->quat[2], (LODWORD(v42) & 0x7F800000) == 0x7F800000)
            || (v41 = mat->quat[3], (LODWORD(v41) & 0x7F800000) == 0x7F800000))
        {
            MyAssertHandler(
                "c:\\trees\\cod3\\src\\renderer\\../xanim/xanim_public.h",
                581,
                0,
                "%s",
                "!IS_NAN((mat->quat)[0]) && !IS_NAN((mat->quat)[1]) && !IS_NAN((mat->quat)[2]) && !IS_NAN((mat->quat)[3])");
        }
        transWeight = mat->transWeight;
        if ((LODWORD(transWeight) & 0x7F800000) == 0x7F800000)
            MyAssertHandler(
                "c:\\trees\\cod3\\src\\renderer\\../xanim/xanim_public.h",
                582,
                0,
                "%s",
                "!IS_NAN(mat->transWeight)");
        Vec3Scale(mat->quat, mat->transWeight, v39);
        v38 = v39[0] * mat->quat[0];
        v37 = v39[0] * mat->quat[1];
        v36 = v39[0] * mat->quat[2];
        v35 = v39[0] * mat->quat[3];
        v34 = v39[1] * mat->quat[1];
        v33 = v39[1] * mat->quat[2];
        v32 = v39[1] * mat->quat[3];
        v31 = v39[2] * mat->quat[2];
        v30 = v39[2] * mat->quat[3];
        v29.axis[0][0] = 1.0 - (v34 + v31);
        v29.axis[0][1] = v37 - v30;
        v29.axis[0][2] = v36 + v32;
        v29.axis[0][3] = 0.0;
        v29.axis[1][0] = v37 + v30;
        v29.axis[1][1] = 1.0 - (v38 + v31);
        v29.axis[1][2] = v33 - v35;
        v29.axis[1][3] = 0.0;
        v29.axis[2][0] = v36 - v32;
        v29.axis[2][1] = v33 + v35;
        v29.axis[2][2] = 1.0 - (v38 + v34);
        v29.axis[2][3] = 0.0;
        v29.origin[0] = -(mat->trans[0] * v29.axis[0][0] + mat->trans[1] * v29.axis[1][0] + mat->trans[2] * v29.axis[2][0]);
        v29.origin[1] = -(mat->trans[0] * v29.axis[0][1] + mat->trans[1] * v29.axis[1][1] + mat->trans[2] * v29.axis[2][1]);
        v29.origin[2] = -(mat->trans[0] * v29.axis[0][2] + mat->trans[1] * v29.axis[1][2] + mat->trans[2] * v29.axis[2][2]);
        v29.origin[3] = 1.0;

        // TODO(mrsteyk): BROKEN!
#if 0
        {
            DObjSkelMat test;
            ConvertQuatToInverseSkelMat(mat, &test);
            iassert(!memcmp(&test, &v29, sizeof(test)));
        }
#endif
#else
        // NOTE(mrsteyk): I was testing shit
        ConvertQuatToInverseSkelMat(mat, &v29);
#endif

        mat_1 = &RotTransArray[v46];

#if 0
        float v27;
        float v26, v25, v24;
        float v23;
        float v22[3];

        float v13, v14, v15, v16, v17, v18, v19, v20, v21;

        v27 = mat_1->quat[0];
        if ((LODWORD(v27) & 0x7F800000) == 0x7F800000
            || (v26 = mat_1->quat[1], (LODWORD(v26) & 0x7F800000) == 0x7F800000)
            || (v25 = mat_1->quat[2], (LODWORD(v25) & 0x7F800000) == 0x7F800000)
            || (v24 = mat_1->quat[3], (LODWORD(v24) & 0x7F800000) == 0x7F800000))
        {
            MyAssertHandler(
                "c:\\trees\\cod3\\src\\renderer\\../xanim/xanim_public.h",
                473,
                0,
                "%s",
                "!IS_NAN((mat->quat)[0]) && !IS_NAN((mat->quat)[1]) && !IS_NAN((mat->quat)[2]) && !IS_NAN((mat->quat)[3])");
        }
        v23 = mat_1->transWeight;
        if ((LODWORD(v23) & 0x7F800000) == 0x7F800000)
            MyAssertHandler(
                "c:\\trees\\cod3\\src\\renderer\\../xanim/xanim_public.h",
                474,
                0,
                "%s",
                "!IS_NAN(mat->transWeight)");
        Vec3Scale(mat_1->quat, mat_1->transWeight, v22);
        v21 = v22[0] * mat_1->quat[0];
        v20 = v22[0] * mat_1->quat[1];
        v19 = v22[0] * mat_1->quat[2];
        v18 = v22[0] * mat_1->quat[3];
        v17 = v22[1] * mat_1->quat[1];
        v16 = v22[1] * mat_1->quat[2];
        v15 = v22[1] * mat_1->quat[3];
        v14 = v22[2] * mat_1->quat[2];
        v13 = v22[2] * mat_1->quat[3];
        quat_12.axis[0][0] = 1.0 - (v17 + v14);
        quat_12.axis[0][1] = v20 + v13;
        quat_12.axis[0][2] = v19 - v15;
        quat_12.axis[0][3] = 0.0;
        quat_12.axis[1][0] = v20 - v13;
        quat_12.axis[1][1] = 1.0 - (v21 + v14);
        quat_12.axis[1][2] = v16 + v18;
        quat_12.axis[1][3] = 0.0;
        quat_12.axis[2][0] = v19 + v15;
        quat_12.axis[2][1] = v16 - v18;
        quat_12.axis[2][2] = 1.0 - (v21 + v17);
        quat_12.axis[2][3] = 0.0;
        quat_12.origin[0] = mat_1->trans[0];
        quat_12.origin[1] = mat_1->trans[1];
        quat_12.origin[2] = mat_1->trans[2];
        quat_12.origin[3] = 1.0;

        {
            DObjSkelMat test;
            LocalConvertQuatToSkelMat(mat_1, &test);
            iassert(!memcmp(&test, &quat_12, sizeof(test)));
        }
#else
        LocalConvertQuatToSkelMat(mat_1, &quat_12);
#endif

#if 0
        v9[0] = -mat->quat[0] * mat_1->quat[3]
            + mat->quat[3] * mat_1->quat[0]
            - mat->quat[2] * mat_1->quat[1]
            + mat->quat[1] * mat_1->quat[2];
        v9[1] = -mat->quat[1] * mat_1->quat[3]
            + mat->quat[2] * mat_1->quat[0]
            + mat->quat[3] * mat_1->quat[1]
            - mat->quat[0] * mat_1->quat[2];
        v9[2] = -mat->quat[2] * mat_1->quat[3]
            - mat->quat[1] * mat_1->quat[0]
            + mat->quat[0] * mat_1->quat[1]
            + mat->quat[3] * mat_1->quat[2];
        v9[3] = mat->quat[3] * mat_1->quat[3]
            + mat->quat[0] * mat_1->quat[0]
            + mat->quat[1] * mat_1->quat[1]
            + mat->quat[2] * mat_1->quat[2];

        {
            float test[4];
            LocalQuatMultiplyInverse(mat->quat, mat_1->quat, test);
            iassert(!memcmp(test, v9, sizeof(test)));
        }
#else
        LocalQuatMultiplyInverse(mat->quat, mat_1->quat, v9);
#endif
        
        Vec4Normalize(v9);

        rigidSurf->placement.base.quat[0] = v9[0];
        rigidSurf->placement.base.quat[1] = v9[1];
        rigidSurf->placement.base.quat[2] = v9[2];
        rigidSurf->placement.base.quat[3] = v9[3];
        LocalTransformVector(v29.origin, (const float4*)&quat_12.axis, v7);
        Vec3Add(v7, scene.def.viewOffset, rigidSurf->placement.base.origin);
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

