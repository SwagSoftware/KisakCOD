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

    unsigned int lod = 0;
    unsigned int totalBones = 0;

    unsigned int totalSurfaceCount = 0;
    unsigned int numSkinnedVerts = 0;

    while (lod < DObjGetNumModels(obj))
    {
        const XModel* model = DObjGetModel(obj, lod++);
        iassert(model);

        int bones = XModelNumBones(model);
        totalBones += bones;

        int cullLod = sceneEnt->cull.lods[lod];
        if (cullLod < 0) continue;

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

        unsigned int partbits[4] = { 0,0,0,0 };
        DObjGetHidePartBits(obj, partbits);

        for (unsigned int i = 0; i < surfaceCount; i++)
        {
            XSurface* surface = &surfaces[i];

            if (surface->partBits[3] & partbits[3]
                | surface->partBits[2] & partbits[2]
                | surface->partBits[1] & partbits[1]
                | surface->partBits[0] & partbits[0])
            {
                *(unsigned int*)surfPos = -3;
                surfPos += 4;
            }
            else
            {
                skinCmd.surfacePartBits[3] |= partbits[3];
                skinCmd.surfacePartBits[2] |= partbits[2];
                skinCmd.surfacePartBits[1] |= partbits[1];
                skinCmd.surfacePartBits[0] |= partbits[0];

                int surfBufSize = R_PreSkinXSurface(obj, surface, &targBoneIndexHigh, &numSkinnedVerts, (float*)surfPos);
                
                GfxModelSkinnedSurface* skinnedSurface = (GfxModelSkinnedSurface*)surfPos;
                skinnedSurface->xsurf = surface;
                skinnedSurface->info = targBoneIndexHigh;

                iassert(surfBufSize);
                surfPos += surfBufSize;
            }
        }
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
            unsigned int totalSurfaceIndex = INT_MAX;
            if (gfxBuf.fastSkin)
            {
                GfxSkinCacheEntry* skinCacheEntry = CG_GetSkinCacheEntry(sceneEnt->info.pose);
                if (skinCacheEntry->ageCount < 3u
                    && gfxBuf.skinnedCacheNormalsFrameCount - skinCacheEntry->frameCount == 1
                    && skinCacheEntry->numSkinnedVerts == numSkinnedVerts)
                {
                    totalSurfaceIndex = skinCacheEntry->skinnedCachedOffset;
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
            unsigned int* surfPos2 = (unsigned int*)surfsBuffer;
            for (unsigned int offset = 0; offset < totalSurfaceCount; ++offset)
            {
                if (*(int*)surfPos2 == -2)
                {
                    surfPos2 += 14;
                }
                else if (*(int*)surfPos2 == -3)
                {
                    ++surfPos2;
                }
                else
                {
                    int tempSkinPos = 32 * *(int*)surfPos2;
                    surfPos2[5] = tempSkinPos + totalSurfaceIndex;
                    *(int*)surfPos2 = tempSkinPos + skinnedCachedOffset;
                    surfPos2 += 6;
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
            unsigned int* surfPos2 = (unsigned int*)surfsBuffer;
            for (unsigned int offset = 0; offset < totalSurfaceCount; ++offset)
            {
                if (*(int*)surfPos2 == -2)
                {
                    surfPos2 += 14;
                }
                else if (*(int*)surfPos2 == -3)
                {
                    ++surfPos2;
                }
                else
                {
                    surfPos2[5] = (unsigned int)&frontEndDataOut->tempSkinBuf[32 * *(int*)surfPos2 + firstSurf];
                    *(int*)surfPos2 = -1;

                    surfPos2 += 6;
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
    float *surfPos)
{
    float v7[2]; // [esp+10h] [ebp-164h] BYREF
    //float origin[3]; // [esp+1Ch] [ebp-158h] BYREF
    float v9; // [esp+28h] [ebp-14Ch]
    float quat[6]; // [esp+2Ch] [ebp-148h] BYREF
    DObjSkelMat skelMat; // [esp+44h] [ebp-130h] BYREF
    float v12; // [esp+84h] [ebp-F0h]
    float v13; // [esp+88h] [ebp-ECh]
    float v14; // [esp+8Ch] [ebp-E8h]
    float v15; // [esp+90h] [ebp-E4h]
    float v16; // [esp+94h] [ebp-E0h]
    float v17; // [esp+98h] [ebp-DCh]
    float v18; // [esp+9Ch] [ebp-D8h]
    float v19; // [esp+A0h] [ebp-D4h]
    float v20; // [esp+A4h] [ebp-D0h] BYREF
    float v21; // [esp+A8h] [ebp-CCh]
    float v22; // [esp+ACh] [ebp-C8h]
    float v23; // [esp+B0h] [ebp-C4h]
    float v24; // [esp+B4h] [ebp-C0h]
    float v25; // [esp+B8h] [ebp-BCh]
    float v26; // [esp+BCh] [ebp-B8h]
    float v27; // [esp+C0h] [ebp-B4h]
    DObjAnimMat *v28; // [esp+C4h] [ebp-B0h]
    float v29; // [esp+C8h] [ebp-ACh]
    float v30; // [esp+CCh] [ebp-A8h]
    const DObjAnimMat *bone; // [esp+D0h] [ebp-A4h]
    DObjSkelMat invBaseMat; // [esp+D4h] [ebp-A0h] BYREF
    float v33; // [esp+114h] [ebp-60h]
    float v34; // [esp+118h] [ebp-5Ch]
    float v35; // [esp+11Ch] [ebp-58h]
    float v36; // [esp+120h] [ebp-54h]
    float v37; // [esp+124h] [ebp-50h]
    float v38; // [esp+128h] [ebp-4Ch]
    float v39; // [esp+12Ch] [ebp-48h]
    float v40; // [esp+130h] [ebp-44h] BYREF
    float v41; // [esp+134h] [ebp-40h]
    float v42; // [esp+138h] [ebp-3Ch]
    float transWeight; // [esp+13Ch] [ebp-38h]
    float v44; // [esp+140h] [ebp-34h]
    float v45; // [esp+144h] [ebp-30h]
    float v46; // [esp+148h] [ebp-2Ch]
    float v47; // [esp+14Ch] [ebp-28h]
    const DObjAnimMat *mat; // [esp+150h] [ebp-24h]
    int v49; // [esp+154h] [ebp-20h]
    DObjAnimMat *RotTransArray; // [esp+158h] [ebp-1Ch]
    //const DObjAnimMat *baseBone; // [esp+15Ch] [ebp-18h]
    unsigned int boneOffset; // [esp+160h] [ebp-14h]
    const DObjAnimMat *boneMatrix; // [esp+164h] [ebp-10h]
    GfxModelRigidSurface *rigidSurf; // [esp+168h] [ebp-Ch]
    void *v55; // [esp+16Ch] [ebp-8h]
    void *retaddr; // [esp+174h] [ebp+0h]

    //rigidSurf = a1;
    //v55 = retaddr;
    if (!obj)
        MyAssertHandler(".\\r_dobj_skin.cpp", 65, 0, "%s", "obj");
    if (!surf)
        MyAssertHandler(".\\r_dobj_skin.cpp", 66, 0, "%s", "surf");
    if (!surfaceInfo)
        MyAssertHandler(".\\r_dobj_skin.cpp", 67, 0, "%s", "surfaceInfo");
    if (!surf->deformed
        && (HIBYTE(boneMatrix) = useFastFile->current.enabled, boneOffset = HIBYTE(boneMatrix) != 0, HIBYTE(boneMatrix))
        && surf->vertListCount == 1)
    {
        surf->vertList = surf->vertList;
        //baseBone = (const DObjAnimMat *)surfPos;

        //if (baseBone != (const DObjAnimMat *)surfPos)
        //    MyAssertHandler(
        //        ".\\r_dobj_skin.cpp",
        //        82,
        //        0,
        //        "%s",
        //        "&rigidSurf->surf == reinterpret_cast< GfxModelSkinnedSurface * >( surfPos )");
        //baseBone[1].trans[1] = 1.0;
        //*surfPos = NAN;

        ((GfxModelSkinnedSurface*)surfPos)->skinnedCachedOffset = -2;
        surfPos[13] = 1.0f;

        RotTransArray = &DObjGetRotTransArray(obj)[surfaceInfo->boneIndex];
        v49 = surf->vertList->boneOffset >> 6;
        mat = &surfaceInfo->baseMat[v49];

        ConvertQuatToInverseSkelMat(mat, &invBaseMat);

#pragma region CONVERT_QUAT_TO_INVERSE
#if 0
        //iassert(!IS_NAN((mat->quat)[0]) && !IS_NAN((mat->quat)[1]) && !IS_NAN((mat->quat)[2]) && !IS_NAN((mat->quat)[3]));
        //v47 = mat->quat[0];
        //if ((LODWORD(v47) & 0x7F800000) == 0x7F800000
        //    || (v46 = mat->quat[1], (LODWORD(v46) & 0x7F800000) == 0x7F800000)
        //    || (v45 = mat->quat[2], (LODWORD(v45) & 0x7F800000) == 0x7F800000)
        //    || (v44 = mat->quat[3], (LODWORD(v44) & 0x7F800000) == 0x7F800000))
        //{
        //    MyAssertHandler(
        //        "c:\\trees\\cod3\\src\\renderer\\../xanim/xanim_public.h",
        //        581,
        //        0,
        //        "%s",
        //        "!IS_NAN((mat->quat)[0]) && !IS_NAN((mat->quat)[1]) && !IS_NAN((mat->quat)[2]) && !IS_NAN((mat->quat)[3])");
        //}
        transWeight = mat->transWeight;
        //iassert(!IS_NAN(mat->transWeight));
        //if ((LODWORD(transWeight) & 0x7F800000) == 0x7F800000)
        //    MyAssertHandler(
        //        "c:\\trees\\cod3\\src\\renderer\\../xanim/xanim_public.h",
        //        582,
        //        0,
        //        "%s",
        //        "!IS_NAN(mat->transWeight)");
        Vec3Scale(mat->quat, mat->transWeight, &v40);
        v39 = v40 * mat->quat[0];
        v38 = v40 * mat->quat[1];
        v37 = v40 * mat->quat[2];
        v36 = v40 * mat->quat[3];
        v35 = v41 * mat->quat[1];
        v34 = v41 * mat->quat[2];
        v33 = v41 * mat->quat[3];
        invBaseMat.origin[3] = v42 * mat->quat[2];
        invBaseMat.origin[2] = v42 * mat->quat[3];
        v29 = 1.0 - (v35 + invBaseMat.origin[3]);
        v30 = v38 - invBaseMat.origin[2];
        *(float *)&bone = v37 + v33;
        invBaseMat.axis[0][0] = 0.0;
        invBaseMat.axis[0][1] = v38 + invBaseMat.origin[2];
        invBaseMat.axis[0][2] = 1.0 - (v39 + invBaseMat.origin[3]);
        invBaseMat.axis[0][3] = v34 - v36;
        invBaseMat.axis[1][0] = 0.0;
        invBaseMat.axis[1][1] = v37 - v33;
        invBaseMat.axis[1][2] = v34 + v36;
        invBaseMat.axis[1][3] = 1.0 - (v39 + v35);
        invBaseMat.axis[2][0] = 0.0;
        invBaseMat.axis[2][1] = -(mat->trans[0] * v29
            + mat->trans[1] * invBaseMat.axis[0][1]
            + mat->trans[2] * invBaseMat.axis[1][1]);
        invBaseMat.axis[2][2] = -(mat->trans[0] * v30
            + mat->trans[1] * invBaseMat.axis[0][2]
            + mat->trans[2] * invBaseMat.axis[1][2]);
        invBaseMat.axis[2][3] = -(mat->trans[0] * *(float *)&bone
            + mat->trans[1] * invBaseMat.axis[0][3]
            + mat->trans[2] * invBaseMat.axis[1][3]);
        invBaseMat.origin[0] = 1.0;

#endif
#pragma endregion
        v28 = &RotTransArray[v49];
        LocalConvertQuatToSkelMat(v28, &skelMat);
#pragma region CONVERT_QUAT
#if 0
        v27 = v28->quat[0];
        if ((LODWORD(v27) & 0x7F800000) == 0x7F800000
            || (v26 = v28->quat[1], (LODWORD(v26) & 0x7F800000) == 0x7F800000)
            || (v25 = v28->quat[2], (LODWORD(v25) & 0x7F800000) == 0x7F800000)
            || (v24 = v28->quat[3], (LODWORD(v24) & 0x7F800000) == 0x7F800000))
        {
            MyAssertHandler(
                "c:\\trees\\cod3\\src\\renderer\\../xanim/xanim_public.h",
                473,
                0,
                "%s",
                "!IS_NAN((mat->quat)[0]) && !IS_NAN((mat->quat)[1]) && !IS_NAN((mat->quat)[2]) && !IS_NAN((mat->quat)[3])");
        }
        v23 = v28->transWeight;
        if ((LODWORD(v23) & 0x7F800000) == 0x7F800000)
            MyAssertHandler(
                "c:\\trees\\cod3\\src\\renderer\\../xanim/xanim_public.h",
                474,
                0,
                "%s",
                "!IS_NAN(mat->transWeight)");
        Vec3Scale(v28->quat, v28->transWeight, &v20);
        v19 = v20 * v28->quat[0];
        v18 = v20 * v28->quat[1];
        v17 = v20 * v28->quat[2];
        v16 = v20 * v28->quat[3];
        v15 = v21 * v28->quat[1];
        v14 = v21 * v28->quat[2];
        v13 = v21 * v28->quat[3];
        v12 = v22 * v28->quat[2];
        skelMat.origin[3] = v22 * v28->quat[3];
        quat[3] = 1.0 - (v15 + v12);
        quat[4] = v18 + skelMat.origin[3];
        quat[5] = v17 - v13;
        skelMat.axis[0][0] = 0.0;
        skelMat.axis[0][1] = v18 - skelMat.origin[3];
        skelMat.axis[0][2] = 1.0 - (v19 + v12);
        skelMat.axis[0][3] = v14 + v16;
        skelMat.axis[1][0] = 0.0;
        skelMat.axis[1][1] = v17 + v13;
        skelMat.axis[1][2] = v14 - v16;
        skelMat.axis[1][3] = 1.0 - (v19 + v15);
        skelMat.axis[2][0] = 0.0;
        LODWORD(quat[2]) = &skelMat.axis[2][1];
        LODWORD(quat[1]) = v28->trans;
        skelMat.axis[2][1] = v28->trans[0];
        skelMat.axis[2][2] = v28->trans[1];
        skelMat.axis[2][3] = v28->trans[2];
        skelMat.origin[0] = 1.0;
#endif
#pragma endregion

        float origin[4];
        const float *cursedFloats = (const float *)((char *)surfaceInfo->baseMat->quat + ((surf->vertList->boneOffset >> 1) & 0x7FFFFFE0));
        LocalQuatMultiplyInverse(cursedFloats, v28->quat, origin);

#pragma region QUAT_MULTIPLY_INVERSE
#if 0
        origin[1] = -mat->quat[0] * v28->quat[3]
            + mat->quat[3] * v28->quat[0]
            - mat->quat[2] * v28->quat[1]
            + mat->quat[1] * v28->quat[2];
        origin[2] = -mat->quat[1] * v28->quat[3]
            + mat->quat[2] * v28->quat[0]
            + mat->quat[3] * v28->quat[1]
            - mat->quat[0] * v28->quat[2];
        v9 = -mat->quat[2] * v28->quat[3]
            - mat->quat[1] * v28->quat[0]
            + mat->quat[0] * v28->quat[1]
            + mat->quat[3] * v28->quat[2];
        quat[0] = mat->quat[3] * v28->quat[3]
            + mat->quat[0] * v28->quat[0]
            + mat->quat[1] * v28->quat[1]
            + mat->quat[2] * v28->quat[2];
#endif
#pragma endregion

        Vec4Normalize(origin);

        surfPos[5] = origin[0];
        surfPos[6] = origin[1]; 
        surfPos[7] = origin[2]; 
        surfPos[8] = origin[3]; 

        //LODWORD(origin[0]) = &baseBone->trans[2];
        //baseBone->trans[2] = origin[1];
        //*(float *)(LODWORD(origin[0]) + 4) = origin[2];
        //*(float *)(LODWORD(origin[0]) + 8) = v9;
        //*(float *)(LODWORD(origin[0]) + 12) = quat[0];
        //LocalTransformVector(&invBaseMat.axis[2][1], (const float4 *)&quat[3], v7);

        R_TransformSkelMat(invBaseMat.origin, &skelMat, origin);

        Vec3Add(origin, scene.def.viewOffset, &surfPos[9]);
        return 56;
    }
    else
    {
        *(_DWORD *)surfPos = *numSkinnedVerts;
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

