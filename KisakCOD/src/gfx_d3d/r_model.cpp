#include "r_model.h"
#include <qcommon/mem_track.h>
#include <universal/com_memory.h>
#include "r_buffers.h"
#include <universal/profile.h>
#include <database/database.h>

#include <algorithm>
#include "r_staticmodel.h"
#include <DynEntity/DynEntity_client.h>
#include "r_dvars.h"
#include <xanim/dobj_utils.h>
#include "r_dobj_skin.h"
#include "r_utils.h"
#include <cgame/cg_local.h>
#include "r_model_pose.h"

const int boxVerts[24][3] =
{
  { 0, 0, 0 },
  { 1, 0, 0 },
  { 0, 0, 0 },
  { 0, 1, 0 },
  { 1, 1, 0 },
  { 1, 0, 0 },
  { 1, 1, 0 },
  { 0, 1, 0 },
  { 0, 0, 1 },
  { 1, 0, 1 },
  { 0, 0, 1 },
  { 0, 1, 1 },
  { 1, 1, 1 },
  { 1, 0, 1 },
  { 1, 1, 1 },
  { 0, 1, 1 },
  { 0, 0, 0 },
  { 0, 0, 1 },
  { 1, 0, 0 },
  { 1, 0, 1 },
  { 0, 1, 0 },
  { 0, 1, 1 },
  { 1, 1, 0 },
  { 1, 1, 1 }
}; // idb

void __cdecl TRACK_r_model()
{
    track_static_alloc_internal((void *)boxVerts, 288, "boxVerts", 18);
    track_static_alloc_internal(&gfxBuf, 2359456, "gfxBuf", 18);
}

void __cdecl R_UnlockSkinnedCache()
{
    IDirect3DVertexBuffer9 *vb; // [esp+30h] [ebp-4h]

    if (gfxBuf.skinnedCacheLockAddr)
    {
        gfxBuf.skinnedCacheLockAddr = 0;
        Profile_Begin(160);
        vb = frontEndDataOut->skinnedCacheVb->buffer;
        if (!vb)
            MyAssertHandler(".\\r_model.cpp", 372, 0, "%s", "vb");
        R_UnlockVertexBuffer(vb);
        Profile_EndInternal(0);
    }
}

static bool __cdecl R_ModelSort(XModel *model1, XModel *model2)
{
    int MemUsage; // esi

    MemUsage = XModelGetMemUsage(model1);
    return MemUsage < XModelGetMemUsage(model2);
}

void __cdecl R_ModelList_f()
{
    const char *Name; // eax
    const char *v1; // [esp+Ch] [ebp-212Ch]
    const char *fmt; // [esp+10h] [ebp-2128h]
    int inData; // [esp+108h] [ebp-2030h] BYREF
    XModel *v4[2050]; // [esp+10Ch] [ebp-202Ch] BYREF
    XModel *model; // [esp+2114h] [ebp-24h]
    int v6; // [esp+2118h] [ebp-20h]
    int v7; // [esp+211Ch] [ebp-1Ch]
    int XModelUsageCount; // [esp+2120h] [ebp-18h]
    int MemoryUsage; // [esp+2124h] [ebp-14h]
    int MemUsage; // [esp+2128h] [ebp-10h]
    int i; // [esp+212Ch] [ebp-Ch]
    float v12; // [esp+2130h] [ebp-8h]
    int modelCount; // [esp+2134h] [ebp-4h] BYREF

    v7 = 0;
    v6 = 0;
    inData = 0;
    DB_EnumXAssets(ASSET_TYPE_XMODEL, (void(__cdecl *)(XAssetHeader, void *))R_GetModelList, &inData, 1);
    //std::_Sort<XModel **, int, bool(__cdecl *)(XModel *&, XModel *&)>(v4, &v4[inData], (4 * inData) >> 2, R_ModelSort);
    std::sort(&v4[0], &v4[inData], R_ModelSort);
    Com_Printf(8, "---------------------------\n");
    Com_Printf(8, "SM# is the number of static model instances\n");
    Com_Printf(8, "instKB is static model instance usage\n");
    Com_Printf(8, "DE# is the number of dyn entity instances\n");
    Com_Printf(8, "   SM#  instKB   DE#   geoKB  name\n");
    for (i = 0; i < inData; ++i)
    {
        model = v4[i];
        MemUsage = XModelGetMemUsage(model);
        v7 += MemUsage;
        MemoryUsage = R_StaticModelGetMemoryUsage(model, &modelCount);
        if (MemoryUsage)
        {
            v6 += MemoryUsage;
            v12 = (double)MemoryUsage / 1024.0;
            Com_Printf(8, "  %4i  ", modelCount);
            if (v12 >= 10.0)
                fmt = "%6.0f";
            else
                fmt = "%6.1f";
            Com_Printf(8, fmt, v12);
        }
        else
        {
            Com_Printf(8, "              ");
        }
        XModelUsageCount = DynEnt_GetXModelUsageCount(model);
        if (XModelUsageCount)
            Com_Printf(8, "  %4i  ", XModelUsageCount);
        else
            Com_Printf(8, "        ");
        v12 = (double)MemUsage / 1024.0;
        if (v12 >= 10.0)
            v1 = "%6.0f";
        else
            v1 = "%6.1f";
        Com_Printf(8, v1, v12);
        Name = XModelGetName(model);
        Com_Printf(8, "  %s\n", Name);
    }
    Com_Printf(8, "---------------------------\n");
    Com_Printf(8, "current inst total  %4.1f MB\n", (double)v6 / 1048576.0);
    Com_Printf(8, "current geo total   %4.1f MB\n", (double)v7 / 1048576.0);
    Com_Printf(8, "current total       %4.1f MB\n", (double)(v7 + v6) / 1048576.0);
    Com_Printf(8, "Related commands: meminfo, imagelist, gfx_world, gfx_model, cg_drawfps, com_statmon, tempmeminfo\n");
}

void __cdecl R_GetModelList(XAssetHeader header, XAssetHeader *data)
{
    if (data->xmodelPieces >= (XModelPieces *)0x800)
        MyAssertHandler(".\\r_model.cpp", 96, 0, "%s", "modelList->count < ARRAY_COUNT( modelList->sorted )");
    data[(int)data->xmodelPieces++ + 1] = header;
}

XModel *__cdecl R_RegisterModel(const char *name)
{
    return XModelPrecache(
        (char*)name,
        (void *(__cdecl *)(int))Hunk_AllocXModelPrecache,
        (void *(__cdecl *)(int))Hunk_AllocXModelPrecacheColl);
}

void __cdecl R_XModelDebug(const DObj_s *obj, int *partBits)
{
    if ((r_xdebug->current.integer & 1) != 0)
        R_XModelDebugBoxes(obj, partBits);
    if ((r_xdebug->current.integer & 2) != 0)
        R_XModelDebugAxes(obj, partBits);
}

void __cdecl R_XModelDebugBoxes(const DObj_s *obj, int *partBits)
{
    DObjAnimMat *boneMatrix; // [esp+0h] [ebp-250h]
    unsigned int boxEdge; // [esp+4h] [ebp-24Ch]
    XBoneInfo *boneInfoArray[128]; // [esp+8h] [ebp-248h] BYREF
    int boneIndex; // [esp+208h] [ebp-48h]
    int boneCount; // [esp+20Ch] [ebp-44h]
    float start[3]; // [esp+210h] [ebp-40h] BYREF
    float end[3]; // [esp+21Ch] [ebp-34h] BYREF
    const int (*tempBoxVerts)[3]; // [esp+228h] [ebp-28h]
    float org[3]; // [esp+22Ch] [ebp-24h] BYREF
    XBoneInfo *boneInfo; // [esp+238h] [ebp-18h]
    float color[4]; // [esp+23Ch] [ebp-14h] BYREF
    unsigned int animPartBit; // [esp+24Ch] [ebp-4h]

    if (!obj)
        MyAssertHandler(".\\r_model.cpp", 215, 0, "%s", "obj");
    boneMatrix = DObjGetRotTransArray(obj);
    if (boneMatrix)
    {
        boneCount = DObjNumBones(obj);
        if (boneCount > 128)
            MyAssertHandler(".\\r_model.cpp", 221, 0, "%s", "boneCount <= DOBJ_MAX_PARTS");
        DObjGetBoneInfo(obj, boneInfoArray);
        color[0] = 1.0;
        color[1] = 1.0;
        color[2] = 1.0;
        color[3] = 0.0;
        animPartBit = 0x80000000;
        boneIndex = 0;
        while (boneIndex < boneCount)
        {
            if ((animPartBit & partBits[boneIndex >> 5]) != 0)
            {
                boneInfo = boneInfoArray[boneIndex];
                tempBoxVerts = boxVerts;
                for (boxEdge = 0; boxEdge < 0xC; ++boxEdge)
                {
                    org[0] = boneInfo->bounds[(*tempBoxVerts)[0]][0];
                    org[1] = boneInfo->bounds[(*tempBoxVerts)[1]][1];
                    org[2] = boneInfo->bounds[(*tempBoxVerts)[2]][2];
                    MatrixTransformVectorQuatTrans(org, &boneMatrix[boneIndex], start);
                    Vec3Add(start, scene.def.viewOffset, start);
                    org[0] = boneInfo->bounds[(*++tempBoxVerts)[0]][0];
                    org[1] = boneInfo->bounds[(*tempBoxVerts)[1]][1];
                    org[2] = boneInfo->bounds[(*tempBoxVerts)[2]][2];
                    MatrixTransformVectorQuatTrans(org, &boneMatrix[boneIndex], end);
                    Vec3Add(end, scene.def.viewOffset, end);
                    ++tempBoxVerts;
                    R_AddDebugLine(&frontEndDataOut->debugGlobals, start, end, color);
                }
            }
            ++boneIndex;
            animPartBit = (animPartBit << 31) | (animPartBit >> 1);
        }
    }
}

void __cdecl R_XModelDebugAxes(const DObj_s *obj, int *partBits)
{
    DObjAnimMat *boneMatrix; // [esp+0h] [ebp-60h]
    int boneIndex; // [esp+4h] [ebp-5Ch]
    float translation[3][3]; // [esp+8h] [ebp-58h] BYREF
    int boneCount; // [esp+2Ch] [ebp-34h]
    float start[3]; // [esp+30h] [ebp-30h] BYREF
    float end[3]; // [esp+3Ch] [ebp-24h] BYREF
    int axis; // [esp+48h] [ebp-18h]
    float color[4]; // [esp+4Ch] [ebp-14h] BYREF
    unsigned int animPartBit; // [esp+5Ch] [ebp-4h]

    if (!obj)
        MyAssertHandler(".\\r_model.cpp", 273, 0, "%s", "obj");
    boneMatrix = DObjGetRotTransArray(obj);
    if (boneMatrix)
    {
        translation[0][0] = 6.0;
        translation[0][1] = 0.0;
        translation[0][2] = 0.0;
        translation[1][0] = 0.0;
        translation[1][1] = 6.0;
        translation[1][2] = 0.0;
        translation[2][0] = 0.0;
        translation[2][1] = 0.0;
        translation[2][2] = 6.0;
        boneCount = DObjNumBones(obj);
        animPartBit = 0x80000000;
        for (boneIndex = 0; boneIndex < boneCount; ++boneIndex)
        {
            if ((animPartBit & partBits[boneIndex >> 5]) != 0)
            {
                for (axis = 0; axis < 3; ++axis)
                {
                    color[0] = 0.0;
                    color[1] = 0.0;
                    color[2] = 0.0;
                    color[3] = 0.0;
                    color[axis] = 1.0;
                    MatrixTransformVectorQuatTrans(vec3_origin, &boneMatrix[boneIndex], start);
                    Vec3Add(start, scene.def.viewOffset, start);
                    MatrixTransformVectorQuatTrans(translation[axis], &boneMatrix[boneIndex], end);
                    Vec3Add(end, scene.def.viewOffset, end);
                    R_AddDebugLine(&frontEndDataOut->debugGlobals, start, end, color);
                }
            }
            animPartBit = (animPartBit << 31) | (animPartBit >> 1);
        }
    }
}

int __cdecl R_SkinXModel(
    XModelDrawInfo *modelInfo,
    const XModel *model,
    const DObj_s *obj,
    const GfxPlacement *placement,
    float scale,
    __int16 gfxEntIndex)
{
    unsigned int v8; // [esp+20h] [ebp-E64h]
    unsigned int startSurfPos; // [esp+2Ch] [ebp-E58h]
    XSurface* xsurf; // [esp+38h] [ebp-E4Ch]
    int surfaceIndex; // [esp+40h] [ebp-E44h]
    unsigned __int16* surfPos; // [esp+44h] [ebp-E40h]
    unsigned __int8 surfBuf[3580]; // [esp+48h] [ebp-E3Ch] BYREF
    unsigned int hidePartBits[4]; // [esp+E4Ch] [ebp-38h] BYREF
    //XSurface* surfaces; // [esp+E5Ch] [ebp-28h]
    XSurface* surfaces; // [esp+E60h] [ebp-24h] BYREF
    XModelLodRampType lodRampType; // [esp+E64h] [ebp-20h]
    XModelLodRampType dist; // [esp+E68h] [ebp-1Ch]
    float AdjustedLodDist; // [esp+E6Ch] [ebp-18h]
    float v21; // [esp+E70h] [ebp-14h]
    XModelDrawInfo* modelInfoa; // [esp+E78h] [ebp-Ch]
    //const XModel* modela; // [esp+E7Ch] [ebp-8h]
    //const GfxPlacement* obja; // [esp+E84h] [ebp+0h]

    //modelInfoa = a1;
    //modela = (const XModel*)obja;
    if (!model)
        MyAssertHandler(".\\r_model.cpp", 410, 0, "%s", "model");
    if (!useFastFile->current.enabled && XModelBad(model))
        return 0;
    if (scale == 0.0)
        MyAssertHandler("c:\\trees\\cod3\\src\\universal\\com_math.h", 107, 0, "%s", "val");
    v21 = 1.0 / scale;
    AdjustedLodDist = R_GetBaseLodDist(placement->origin) * v21;
    dist = XModelGetLodRampType(model);
    AdjustedLodDist = R_GetAdjustedLodDist(AdjustedLodDist, dist);
    lodRampType = XModelGetLodForDist(model, AdjustedLodDist);
    if (lodRampType < XMODEL_LOD_RAMP_RIGID)
        return 0;
    Profile_Begin(89);
    int surfaceCount = XModelGetSurfaces(model, &surfaces, lodRampType);
    if (!surfaceCount)
        MyAssertHandler(".\\r_model.cpp", 430, 0, "%s", "surfaceCount");
    if (obj)
        DObjGetHidePartBits(obj, hidePartBits);
    surfPos = (unsigned __int16*)surfBuf;
    for (surfaceIndex = 0; surfaceIndex < surfaceCount; ++surfaceIndex)
    {
        xsurf = surfaces + surfaceIndex;
        if (obj
            && xsurf->partBits[3] & hidePartBits[3]
            | xsurf->partBits[2] & hidePartBits[2]
            | xsurf->partBits[1] & hidePartBits[1]
            | xsurf->partBits[0] & hidePartBits[0])
        {
            *(_DWORD*)surfPos = -3;
            surfPos += 2;
        }
        else
        {
            if (!xsurf->deformed && useFastFile->current.enabled)
                startSurfPos = -2;
            else
                startSurfPos = -1;
            *(_DWORD*)surfPos = startSurfPos;
            // @Correctness
            *((_DWORD*)surfPos + 1) = (_DWORD)xsurf;
            surfPos[7] = gfxEntIndex;
            surfPos[8] = 0;
            qmemcpy(surfPos + 12, placement, 0x1Cu);
            *((float*)surfPos + 13) = scale;
            surfPos += 28;
        }
    }
    v8 = InterlockedExchangeAdd(&frontEndDataOut->surfPos, (char*)surfPos - (char*)surfBuf);
    if ((char*)surfPos - (char*)surfBuf + v8 <= 0x20000)
    {
        if ((v8 & 3) != 0)
            MyAssertHandler(".\\r_model.cpp", 480, 0, "%s", "!(startSurfPos & 3)");
        modelInfo->surfId = v8 >> 2;
        memcpy(&frontEndDataOut->surfsBuffer[v8], surfBuf, (char*)surfPos - (char*)surfBuf);
        modelInfo->lod = lodRampType;
        Profile_EndInternal(0);
        return 1;
    }
    else
    {
        R_WarnOncePerFrame(R_WARN_MAX_SCENE_SURFS_SIZE);
        Profile_EndInternal(0);
        return 0;
    }
}

void __cdecl R_SkinSceneEnt(GfxSceneEntity *sceneEnt)
{
    DObjAnimMat *boneMatrix; // [esp+0h] [ebp-8h]
    const DObj_s *obj; // [esp+4h] [ebp-4h]

    obj = sceneEnt->obj;
    boneMatrix = DObjGetRotTransArray(obj);
    if (boneMatrix)
    {
        CG_CullIn(sceneEnt->info.pose);
        R_SkinSceneDObj(sceneEnt, sceneEnt, obj, boneMatrix, 1);
    }
}

int __cdecl R_SkinAndBoundSceneEnt(GfxSceneEntity *sceneEnt)
{
    DObjAnimMat *boneMatrix; // [esp+0h] [ebp-Ch]
    const DObj_s *obj; // [esp+4h] [ebp-8h] BYREF
    GfxSceneEntity *localSceneEnt; // [esp+8h] [ebp-4h] BYREF
    int savedregs; // [esp+Ch] [ebp+0h] BYREF

    boneMatrix = R_UpdateSceneEntBounds(sceneEnt, &localSceneEnt, &obj, 1);
    if (boneMatrix)
    {
        if (!localSceneEnt)
            MyAssertHandler(".\\r_model.cpp", 524, 0, "%s", "localSceneEnt");
        CG_CullIn(localSceneEnt->info.pose);
        R_SkinSceneDObj(sceneEnt, localSceneEnt, obj, boneMatrix, 0);
        return 1;
    }
    else
    {
        if (localSceneEnt)
            CG_UsedDObjCalcPose(localSceneEnt->info.pose);
        return 0;
    }
}

void __cdecl R_LockSkinnedCache()
{
    IDirect3DVertexBuffer9 *vb; // [esp+30h] [ebp-4h]

    if (gfxBuf.skinnedCacheLockAddr)
        MyAssertHandler(".\\r_model.cpp", 337, 0, "%s", "!gfxBuf.skinnedCacheLockAddr");
    if (!dx.deviceLost)
    {
        vb = frontEndDataOut->skinnedCacheVb->buffer;
        if (!vb)
            MyAssertHandler(".\\r_model.cpp", 342, 0, "%s", "vb");
        Profile_Begin(159);
        gfxBuf.skinnedCacheLockAddr = (unsigned char*)R_LockVertexBuffer(vb, 0, 0, 0x2000);
        if (((unsigned int)gfxBuf.skinnedCacheLockAddr & 0xF) != 0)
        {
            R_UnlockVertexBuffer(vb);
            gfxBuf.skinnedCacheLockAddr = 0;
        }
        Profile_EndInternal(0);
        gfxBuf.oldSkinnedCacheNormalsAddr = gfxBuf.skinnedCacheNormals[gfxBuf.skinnedCacheNormalsFrameCount & 1];
        ++gfxBuf.skinnedCacheNormalsFrameCount;
        gfxBuf.skinnedCacheNormalsAddr = gfxBuf.skinnedCacheNormals[gfxBuf.skinnedCacheNormalsFrameCount & 1];
    }
}