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

const GfxSkinCacheEntry *__cdecl CG_GetSkinCacheEntry(const cpose_t *pose)
{
    return &pose->skinCacheEntry;
}

int  R_SkinSceneDObjModels(
    GfxSceneEntity *sceneEnt,
    const DObj_s *obj,
    DObjAnimMat *boneMatrix)
{
    unsigned int v4; // edx
    unsigned __int8 *v6; // [esp-Ch] [ebp-F0Ch]
    LONG v7; // [esp-8h] [ebp-F08h]
    LONG firstSurf; // [esp+0h] [ebp-F00h]
    LONG v9; // [esp+8h] [ebp-EF8h]
    int tempSkinPos; // [esp+Ch] [ebp-EF4h]
    GfxModelRigidSurface *size; // [esp+14h] [ebp-EECh]
    GfxModelRigidSurface *sizea; // [esp+14h] [ebp-EECh]
    unsigned int offset; // [esp+18h] [ebp-EE8h]
    unsigned int offseta; // [esp+18h] [ebp-EE8h]
    GfxModelRigidSurface *rigidSurf; // [esp+1Ch] [ebp-EE4h]
    GfxModelRigidSurface *rigidSurfa; // [esp+1Ch] [ebp-EE4h]
    GfxModelSkinnedSurface *skinnedSurf; // [esp+20h] [ebp-EE0h]
    unsigned int totalSurfaceIndex; // [esp+24h] [ebp-EDCh]
    XSurface *surfPos2; // [esp+28h] [ebp-ED8h]
    GfxSkinCacheEntry *skinCacheEntry; // [esp+2Ch] [ebp-ED4h]
    int oldSkinnedCachedOffset; // [esp+30h] [ebp-ED0h]
    unsigned int oldSkinnedCachedOffseta; // [esp+30h] [ebp-ED0h]
    unsigned int skinnedCachedOffset; // [esp+34h] [ebp-ECCh]
    unsigned int surf; // [esp+38h] [ebp-EC8h]
    int surfBufSize; // [esp+3Ch] [ebp-EC4h]
    XSurface *partBits_8; // [esp+48h] [ebp-EB8h]
    unsigned int partBits_12; // [esp+4Ch] [ebp-EB4h]
    int v28; // [esp+50h] [ebp-EB0h]
    XSurface *xsurf[4]; // [esp+54h] [ebp-EACh] BYREF
    int hidePartBits_4; // [esp+64h] [ebp-E9Ch]
    int hidePartBits_8; // [esp+68h] [ebp-E98h]
    unsigned int hidePartBits_12; // [esp+6Ch] [ebp-E94h]
    int invTargBoneIndexLow; // [esp+70h] [ebp-E90h]
    int targBoneIndexLow; // [esp+74h] [ebp-E8Ch] BYREF
    GfxModelSurfaceInfo targBoneIndexHigh; // [esp+78h] [ebp-E88h] BYREF
    XModel *surfaceInfo; // [esp+84h] [ebp-E7Ch]
    const XModel *v37; // [esp+88h] [ebp-E78h]
    int v38; // [esp+8Ch] [ebp-E74h]
    int lod; // [esp+90h] [ebp-E70h]
    XModel *model[2]; // [esp+94h] [ebp-E6Ch] BYREF
    unsigned int modelIndex; // [esp+9Ch] [ebp-E64h] BYREF
    SkinXModelCmd skinCmd; // [esp+A0h] [ebp-E60h] BYREF
    int v43; // [esp+BCh] [ebp-E44h]
    int surfPartBits[7]; // [esp+C0h] [ebp-E40h]
    unsigned int totalSurfaceCount; // [esp+DCh] [ebp-E24h] BYREF
    unsigned int modelCount; // [esp+E0h] [ebp-E20h]
    unsigned int boneIndex; // [esp+E4h] [ebp-E1Ch] BYREF

    if (sceneEnt->cull.skinnedSurfs.firstSurf)
        MyAssertHandler(".\\r_dobj_skin.cpp", 163, 0, "%s", "!sceneEnt->cull.skinnedSurfs.firstSurf");
    if (!obj)
        MyAssertHandler(".\\r_dobj_skin.cpp", 164, 0, "%s", "obj");
    if (!boneMatrix)
        MyAssertHandler(".\\r_dobj_skin.cpp", 165, 0, "%s", "boneMatrix");
    if (!useFastFile->current.enabled && DObjBad(obj))
        MyAssertHandler(".\\r_dobj_skin.cpp", 168, 0, "%s", "!IsObjFileLoad() || !DObjBad( obj )");
    Profile_Begin(89);
    modelCount = (unsigned int)&boneIndex;
    totalSurfaceCount = 0;
    surfPartBits[6] = 0;
    surfPartBits[5] = DObjGetNumModels(obj);
    surfPartBits[4] = 0;
    skinCmd.surfacePartBits[3] = 0;
    v43 = 0;
    modelIndex = 0;
    memset(&skinCmd, 0, 12);
    lod = 0;
    while ((unsigned int)lod < surfPartBits[5])
    {
        v37 = DObjGetModel(obj, lod);
        if (!v37)
            MyAssertHandler(".\\r_dobj_skin.cpp", 198, 0, "%s", "model");
        v38 = XModelNumBones(v37);
        surfaceInfo = (XModel *)sceneEnt->cull.lods[lod];
        if ((int)surfaceInfo >= 0)
        {
            targBoneIndexHigh.boneIndex = surfPartBits[6];
            targBoneIndexHigh.baseMat = XModelGetBasePose(v37);
            targBoneIndexHigh.boneCount = v38;
            targBoneIndexHigh.gfxEntIndex = sceneEnt->gfxEntIndex;
            targBoneIndexHigh.lightingHandle = 0;
            invTargBoneIndexLow = XModelGetSurfaces(v37, (XSurface **)&targBoneIndexLow, (int)surfaceInfo);
            if (!targBoneIndexLow)
                MyAssertHandler(".\\r_dobj_skin.cpp", 213, 0, "%s", "surfaces");
            if (!invTargBoneIndexLow)
                MyAssertHandler(".\\r_dobj_skin.cpp", 214, 0, "%s", "surfaceCount");
            surfPartBits[4] += invTargBoneIndexLow;
            hidePartBits_12 = (unsigned int)surfPartBits[6] >> 5;
            hidePartBits_8 = surfPartBits[6] & 0x1F;
            hidePartBits_4 = 32 - hidePartBits_8;
            DObjGetHidePartBits(obj, (unsigned int *)xsurf);
            v28 = targBoneIndexLow;
            for (partBits_12 = 0; partBits_12 < invTargBoneIndexLow; ++partBits_12)
            {
                partBits_8 = (XSurface *)(v28 + 56 * partBits_12);
                surfPartBits[0] = partBits_8->partBits[0];
                surfPartBits[1] = partBits_8->partBits[1];
                surfPartBits[2] = partBits_8->partBits[2];
                surfPartBits[3] = partBits_8->partBits[3];
                if (hidePartBits_8)
                {
                    skinnedCachedOffset = (unsigned int)skinCmd.surfacePartBits[3 - hidePartBits_12 + 3] >> hidePartBits_8;
                    surf = ((unsigned int)skinCmd.surfacePartBits[4 - hidePartBits_12 + 3] >> hidePartBits_8)
                        | (skinCmd.surfacePartBits[3 - hidePartBits_12 + 3] << hidePartBits_4);
                    surfBufSize = ((unsigned int)skinCmd.surfacePartBits[5 - hidePartBits_12 + 3] >> hidePartBits_8)
                        | (skinCmd.surfacePartBits[4 - hidePartBits_12 + 3] << hidePartBits_4);
                    v4 = ((unsigned int)skinCmd.surfacePartBits[6 - hidePartBits_12 + 3] >> hidePartBits_8)
                        | (skinCmd.surfacePartBits[5 - hidePartBits_12 + 3] << hidePartBits_4);
                }
                else
                {
                    skinnedCachedOffset = skinCmd.surfacePartBits[3 - hidePartBits_12 + 3];
                    surf = skinCmd.surfacePartBits[4 - hidePartBits_12 + 3];
                    surfBufSize = skinCmd.surfacePartBits[5 - hidePartBits_12 + 3];
                    v4 = skinCmd.surfacePartBits[6 - hidePartBits_12 + 3];
                }
                if (v4 & (unsigned int)xsurf[3]
                    | surfBufSize & (int)xsurf[2]
                    | surf & (unsigned int)xsurf[1]
                    | skinnedCachedOffset & (unsigned int)xsurf[0])
                {
                    *(_DWORD *)modelCount = -3;
                    modelCount += 4;
                }
                else
                {
                    modelIndex |= skinnedCachedOffset;
                    skinCmd.modelSurfs = (void *)((int)skinCmd.modelSurfs | surf);
                    skinCmd.mat = (const DObjAnimMat *)((int)skinCmd.mat | surfBufSize);
                    skinCmd.surfacePartBits[0] |= v4;
                    oldSkinnedCachedOffset = R_PreSkinXSurface(
                        obj,
                        partBits_8,
                        &targBoneIndexHigh,
                        &totalSurfaceCount,
                        (float *)modelCount);
                    skinCacheEntry = (GfxSkinCacheEntry *)modelCount;
                    *(_DWORD *)(modelCount + 4) = targBoneIndexLow + 56 * partBits_12;
                    *(GfxModelSurfaceInfo *)&skinCacheEntry->numSkinnedVerts = targBoneIndexHigh;
                    if (!oldSkinnedCachedOffset)
                        MyAssertHandler(".\\r_dobj_skin.cpp", 283, 0, "%s", "surfBufSize");
                    modelCount += oldSkinnedCachedOffset;
                }
            }
        }
        ++lod;
        surfPartBits[6] += v38;
    }
    if (!DObjSkelAreBonesUpToDate(obj, (int *)&modelIndex))
        MyAssertHandler(".\\r_dobj_skin.cpp", 288, 0, "%s", "DObjSkelAreBonesUpToDate( obj, skinCmd.surfacePartBits )");
    if (totalSurfaceCount)
    {
        if (gfxBuf.skinCache)
        {
            surfPos2 = (XSurface *)R_AllocSkinnedCachedVerts(totalSurfaceCount);
            if ((int)surfPos2 < 0)
            {
                Profile_EndInternal(0);
                return 0;
            }
            totalSurfaceIndex = -2147483647;
            if (gfxBuf.fastSkin)
            {
                skinnedSurf = (GfxModelSkinnedSurface *)CG_GetSkinCacheEntry(sceneEnt->info.pose);
                if (HIWORD(skinnedSurf->info.baseMat) < 3u
                    && gfxBuf.skinnedCacheNormalsFrameCount - skinnedSurf->skinnedCachedOffset == 1
                    && LOWORD(skinnedSurf->info.baseMat) == totalSurfaceCount)
                {
                    totalSurfaceIndex = (unsigned int)skinnedSurf->xsurf;
                    ++HIWORD(skinnedSurf->info.baseMat);
                }
                else
                {
                    HIWORD(skinnedSurf->info.baseMat) = 0;
                }
                skinnedSurf->skinnedCachedOffset = gfxBuf.skinnedCacheNormalsFrameCount;
                LOWORD(skinnedSurf->info.baseMat) = totalSurfaceCount;
                if (LOWORD(skinnedSurf->info.baseMat) != totalSurfaceCount)
                    MyAssertHandler(".\\r_dobj_skin.cpp", 323, 0, "%s", "skinCacheEntry->numSkinnedVerts == numSkinnedVerts");
                skinnedSurf->xsurf = surfPos2;
            }
            rigidSurf = (GfxModelRigidSurface *)&boneIndex;
            for (offset = 0; offset < surfPartBits[4]; ++offset)
            {
                size = rigidSurf;
                if (rigidSurf->surf.skinnedCachedOffset == -2)
                {
                    ++rigidSurf;
                }
                else if (rigidSurf->surf.skinnedCachedOffset == -3)
                {
                    rigidSurf = (GfxModelRigidSurface *)((char *)rigidSurf + 4);
                }
                else
                {
                    rigidSurf = (GfxModelRigidSurface *)((char *)rigidSurf + 24);
                    tempSkinPos = 32 * size->surf.skinnedCachedOffset;
                    size->surf.oldSkinnedCachedOffset = tempSkinPos + totalSurfaceIndex;
                    size->surf.skinnedCachedOffset = (int)surfPos2 + tempSkinPos;
                }
            }
            if ((GfxModelRigidSurface *)modelCount != rigidSurf)
                MyAssertHandler(".\\r_dobj_skin.cpp", 354, 0, "%s", "surfPos == (byte *)surfPos2");
        }
        else
        {
            v9 = 32 * totalSurfaceCount;
            if (!frontEndDataOut->tempSkinBuf)
                MyAssertHandler(".\\r_dobj_skin.cpp", 361, 0, "%s", "frontEndDataOut->tempSkinBuf");
            firstSurf = InterlockedExchangeAdd(&frontEndDataOut->tempSkinPos, v9);
            if ((unsigned int)(v9 + firstSurf) > 0x480000)
            {
                R_WarnOncePerFrame(R_WARN_TEMP_SKIN_BUF_SIZE);
                Profile_EndInternal(0);
                return 0;
            }
            Z_VirtualCommit(&frontEndDataOut->tempSkinBuf[firstSurf], v9);
            rigidSurfa = (GfxModelRigidSurface *)&boneIndex;
            for (offseta = 0; offseta < surfPartBits[4]; ++offseta)
            {
                sizea = rigidSurfa;
                if (rigidSurfa->surf.skinnedCachedOffset == -2)
                {
                    ++rigidSurfa;
                }
                else if (rigidSurfa->surf.skinnedCachedOffset == -3)
                {
                    rigidSurfa = (GfxModelRigidSurface *)((char *)rigidSurfa + 4);
                }
                else
                {
                    rigidSurfa = (GfxModelRigidSurface *)((char *)rigidSurfa + 24);
                    sizea->surf.oldSkinnedCachedOffset = (int)&frontEndDataOut->tempSkinBuf[32 * sizea->surf.skinnedCachedOffset
                        + firstSurf];
                    sizea->surf.skinnedCachedOffset = -1;
                }
            }
            if ((GfxModelRigidSurface *)modelCount != rigidSurfa)
                MyAssertHandler(".\\r_dobj_skin.cpp", 399, 0, "%s", "surfPos == (byte *)surfPos2");
        }
    }
    oldSkinnedCachedOffseta = modelCount - (_DWORD)&boneIndex;
    v7 = InterlockedExchangeAdd(&frontEndDataOut->surfPos, modelCount - (_DWORD)&boneIndex);
    if (oldSkinnedCachedOffseta + v7 <= 0x20000)
    {
        if ((v7 & 3) != 0)
            MyAssertHandler(".\\r_dobj_skin.cpp", 464, 0, "%s", "!(startSurfPos & 3)");
        v6 = &frontEndDataOut->surfsBuffer[v7];
        sceneEnt->cull.skinnedSurfs.firstSurf = &frontEndDataOut->surfsBuffer[v7];
        memcpy(v6, (unsigned __int8 *)&boneIndex, oldSkinnedCachedOffseta);
        Profile_EndInternal(0);
        if (!surfPartBits[4])
            MyAssertHandler(".\\r_dobj_skin.cpp", 490, 0, "%s", "totalSurfaceCount");
        if (r_xdebug->current.integer)
            R_XModelDebug(obj, (int *)&modelIndex);
        if (totalSurfaceCount)
        {
            model[0] = (XModel *)v6;
            LOWORD(skinCmd.surfacePartBits[1]) = surfPartBits[4];
            model[1] = (XModel *)boneMatrix;
            R_AddWorkerCmd(16, (unsigned __int8 *)model);
        }
        return surfPartBits[4];
    }
    else
    {
        R_WarnOncePerFrame(R_WARN_MAX_SCENE_SURFS_SIZE);
        Profile_EndInternal(0);
        return 0;
    }
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
    v55 = retaddr;
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

        surfPos[12] = 1.0f;
        surfPos[0] = NAN;

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

