#include "xmodel.h"
#include <universal/com_math.h>
#include <universal/com_memory.h>
#include <universal/com_files.h>
#include <qcommon/qcommon.h>
#include <qcommon/com_pack.h>
#include <universal/aabbtree.h>
#include <gfx_d3d/r_dvars.h>
#include "xanim.h"
#include <physics/phys_local.h>

XModelDefault g_default;
Material *g_materials[1];

void __cdecl TRACK_xmodel()
{
    track_static_alloc_internal(&g_default, sizeof(g_default), "g_default", 11);
}

XModelSurfs *__cdecl XModelSurfsFindData(const char *name)
{
    return (XModelSurfs*)Hunk_FindDataForFile(3, name);
}

void __cdecl XModelReadSurface_BuildCollisionTree(
    XSurface *surface,
    unsigned int vertListIndex,
    void *(__cdecl *Alloc)(int))
{
    unsigned __int8 *v3; // eax
    int v4; // [esp+60h] [ebp-138h]
    int v5; // [esp+64h] [ebp-134h]
    int v6; // [esp+68h] [ebp-130h]
    int v7; // [esp+6Ch] [ebp-12Ch]
    int v8; // [esp+70h] [ebp-128h]
    int v9; // [esp+74h] [ebp-124h]
    int v10; // [esp+78h] [ebp-120h]
    int v11; // [esp+7Ch] [ebp-11Ch]
    short v12; // [esp+80h] [ebp-118h]
    int v13; // [esp+84h] [ebp-114h]
    short v14; // [esp+88h] [ebp-110h]
    int v15; // [esp+8Ch] [ebp-10Ch]
    float *v16; // [esp+90h] [ebp-108h]
    float *v17; // [esp+94h] [ebp-104h]
    float *v18; // [esp+98h] [ebp-100h]
    float *v19; // [esp+9Ch] [ebp-FCh]
    GenericAabbTree *builtNode; // [esp+A0h] [ebp-F8h]
    unsigned int leafIndex; // [esp+A4h] [ebp-F4h]
    float nodeMins[3]; // [esp+A8h] [ebp-F0h] BYREF
    XSurfaceCollisionNode *outNode; // [esp+B4h] [ebp-E4h]
    unsigned int leafEnd; // [esp+B8h] [ebp-E0h]
    float nodeMaxs[3]; // [esp+BCh] [ebp-DCh] BYREF
    unsigned int allocSize; // [esp+C8h] [ebp-D0h]
    unsigned int alignedAddr; // [esp+CCh] [ebp-CCh]
    unsigned __int8 *alloced; // [esp+D0h] [ebp-C8h]
    float combinedVolume; // [esp+D4h] [ebp-C4h]
    float thisVolume; // [esp+D8h] [ebp-C0h]
    float tmp[3]; // [esp+DCh] [ebp-BCh] BYREF
    float prevVolume; // [esp+E8h] [ebp-B0h]
    bool merge; // [esp+EFh] [ebp-A9h]
    float triMins[3]; // [esp+F0h] [ebp-A8h] BYREF
    float triMaxs[3]; // [esp+FCh] [ebp-9Ch] BYREF
    XSurfaceCollisionTree *tree; // [esp+108h] [ebp-90h]
    bool generateLeafsPass; // [esp+10Fh] [ebp-89h]
    unsigned int nodeIndex; // [esp+110h] [ebp-88h]
    unsigned int triEndIndex; // [esp+114h] [ebp-84h]
    float prevMins[3]; // [esp+118h] [ebp-80h] BYREF
    unsigned int leafCount; // [esp+124h] [ebp-74h]
    GenericAabbTreeOptions options; // [esp+128h] [ebp-70h] BYREF
    unsigned int nodeCount; // [esp+150h] [ebp-48h]
    float globalMaxs[3]; // [esp+154h] [ebp-44h] BYREF
    unsigned int triIndex; // [esp+160h] [ebp-38h]
    float prevMaxs[3]; // [esp+164h] [ebp-34h] BYREF
    bool lastMergeable; // [esp+173h] [ebp-25h]
    float globalMins[3]; // [esp+174h] [ebp-24h] BYREF
    float globalDelta[3]; // [esp+180h] [ebp-18h] BYREF
    unsigned int triBeginIndex; // [esp+18Ch] [ebp-Ch]
    XRigidVertList *vertList; // [esp+190h] [ebp-8h]
    unsigned int allocedLeafCount; // [esp+194h] [ebp-4h]

    iassert(!surface->deformed);
    iassert(vertListIndex >= 0 && vertListIndex < surface->vertListCount);
    vertList = &surface->vertList[vertListIndex];
    tree = (XSurfaceCollisionTree*)Alloc(40);
    vertList->collisionTree = tree;
    iassert(surface->triCount > 0);
    memset(&options, 0, 12);
    options.mins = 0;
    options.maxs = 0;
    options.maintainValidBounds = 1;
    options.treeNodePool = (GenericAabbTree*)malloc(0x20000u);
    options.treeNodeLimit = 0x2000;
    options.minItemsPerLeaf = 1;
    options.maxItemsPerLeaf = 16;
    ClearBounds(globalMins, globalMaxs);
    tree->leafs = 0;
    generateLeafsPass = 0;
    allocedLeafCount = 0;
    triBeginIndex = vertList->triOffset;
    triEndIndex = triBeginIndex + vertList->triCount;
    while (1)
    {
        leafCount = 0;
        ClearBounds(prevMins, prevMaxs);
        lastMergeable = 0;
        for (triIndex = triBeginIndex; triIndex != triEndIndex; ++triIndex)
        {
            ClearBounds(triMins, triMaxs);
            AddPointToBounds(surface->verts0[surface->triIndices[3 * triIndex]].xyz, triMins, triMaxs);
            AddPointToBounds(surface->verts0[surface->triIndices[3 * triIndex + 1]].xyz, triMins, triMaxs);
            AddPointToBounds(surface->verts0[surface->triIndices[3 * triIndex + 2]].xyz, triMins, triMaxs);
            ExpandBounds(triMins, triMaxs, globalMins, globalMaxs);
            merge = 0;
            if (lastMergeable)
            {
                Vec3Sub(prevMaxs, prevMins, tmp);
                prevVolume = tmp[0] * tmp[1] * tmp[2];
                Vec3Sub(triMaxs, triMins, tmp);
                thisVolume = tmp[0] * tmp[1] * tmp[2];
                ExpandBounds(triMins, triMaxs, prevMins, prevMaxs);
                Vec3Sub(prevMaxs, prevMins, tmp);
                combinedVolume = tmp[0] * tmp[1] * tmp[2];
                if (combinedVolume <= prevVolume + thisVolume)
                    merge = 1;
            }
            if (merge)
            {
                if (generateLeafsPass)
                {
                    if (!leafCount)
                        MyAssertHandler(".\\r_xsurface_load_obj.cpp", 284, 0, "%s", "leafCount > 0");
                    if (leafCount - 1 >= allocedLeafCount)
                        MyAssertHandler(".\\r_xsurface_load_obj.cpp", 285, 0, "%s", "(leafCount - 1) < allocedLeafCount");
                    v19 = options.mins[leafCount - 1];
                    v19[0] = prevMins[0];
                    v19[1] = prevMins[1];
                    v19[2] = prevMins[2];

                    v18 = options.maxs[leafCount - 1];
                    v18[0] = prevMaxs[0];
                    v18[1] = prevMaxs[1];
                    v18[2] = prevMaxs[2];

                    if (tree->leafs[leafCount - 1].triangleBeginIndex >= 0x8000u)
                        MyAssertHandler(
                            ".\\r_xsurface_load_obj.cpp",
                            288,
                            0,
                            "%s\n\t(tree->leafs[leafCount - 1].triangleBeginIndex) = %i",
                            "(tree->leafs[leafCount - 1].triangleBeginIndex < 0x8000)",
                            tree->leafs[leafCount - 1].triangleBeginIndex);
                    tree->leafs[leafCount - 1].triangleBeginIndex += 0x8000;
                }
                lastMergeable = 0;
            }
            else
            {
                if (generateLeafsPass)
                {
                    if (leafCount >= allocedLeafCount)
                        MyAssertHandler(".\\r_xsurface_load_obj.cpp", 297, 0, "%s", "leafCount < allocedLeafCount");
                    if (triIndex >= 0x8000)
                        MyAssertHandler(
                            ".\\r_xsurface_load_obj.cpp",
                            298,
                            0,
                            "%s",
                            "triIndex < XSURFACE_COLLISION_LEAF_TWO_TRIANGLES");
                    tree->leafs[leafCount].triangleBeginIndex = triIndex;
                    if (tree->leafs[leafCount].triangleBeginIndex != triIndex)
                        MyAssertHandler(
                            ".\\r_xsurface_load_obj.cpp",
                            300,
                            0,
                            "%s\n\t(triIndex) = %i",
                            "(tree->leafs[leafCount].triangleBeginIndex == triIndex)",
                            triIndex);
                    v17 = options.mins[leafCount];
                    v17[0] = triMins[0];
                    v17[1] = triMins[1];
                    v17[2] = triMins[2];

                    v16 = options.maxs[leafCount];
                    v16[0] = triMaxs[0];
                    v16[1] = triMaxs[1];
                    v16[2] = triMaxs[2];
                }
                ++leafCount;
                lastMergeable = 1;

                prevMins[0] = triMins[0];
                prevMins[1] = triMins[1];
                prevMins[2] = triMins[2];

                prevMaxs[0] = triMaxs[0];
                prevMaxs[1] = triMaxs[1];
                prevMaxs[2] = triMaxs[2];
            }
        }
        if (generateLeafsPass)
            break;
        generateLeafsPass = 1;
        tree->leafs = (XSurfaceCollisionLeaf*)Alloc(2 * leafCount);
        tree->leafCount = leafCount;
        options.mins = (float(*)[3])malloc(12 * leafCount);
        options.maxs = (float(*)[3])malloc(12 * leafCount);
        options.items = tree->leafs;
        options.itemCount = leafCount;
        options.itemSize = 2;
        allocedLeafCount = leafCount;
    }
    if (leafCount != allocedLeafCount)
        MyAssertHandler(".\\r_xsurface_load_obj.cpp", 313, 0, "%s", "leafCount == allocedLeafCount");
    tree->trans[0] = -globalMins[0];
    tree->trans[1] = -globalMins[1];
    tree->trans[2] = -globalMins[2];
    Vec3Sub(globalMaxs, globalMins, globalDelta);
    tree->scale[0] = 65535.0 / globalDelta[0];
    tree->scale[1] = 65535.0 / globalDelta[1];
    tree->scale[2] = 65535.0 / globalDelta[2];
    nodeCount = BuildAabbTree(&options);
    tree->nodeCount = nodeCount;
    allocSize = 16 * nodeCount + 15;
    v3 = (unsigned char*)Alloc(allocSize);
    alloced = v3;
    alignedAddr = (uintptr_t)(v3 + 15) & 0xFFFFFFF0;
    tree->nodes = (XSurfaceCollisionNode*)alignedAddr;
    if (((uintptr_t)tree->nodes & 0xF) != 0)
        MyAssertHandler(".\\r_xsurface_load_obj.cpp", 352, 0, "%s", "!(reinterpret_cast< uint32_t >( tree->nodes ) & 0x0F)");
    for (nodeIndex = 0; nodeIndex != nodeCount; ++nodeIndex)
    {
        outNode = &tree->nodes[nodeIndex];
        builtNode = &options.treeNodePool[nodeIndex];
        leafEnd = builtNode->itemCount + builtNode->firstItem;
        ClearBounds(nodeMins, nodeMaxs);
        for (leafIndex = builtNode->firstItem; leafIndex != leafEnd; ++leafIndex)
            ExpandBounds(options.mins[leafIndex], options.maxs[leafIndex], nodeMins, nodeMaxs);

        v15 = (tree->scale[0] * (tree->trans[0] + nodeMins[0]) - 0.5);
        if (v15 >= 0)
        {
            if (v15 <= 0xFFFF)
                outNode->aabb.mins[0] = (unsigned short)v15;
            else
                outNode->aabb.mins[0] = 0xFFFF;
        }
        else
        {
            outNode->aabb.mins[0] = 0;
        }

        v13 = (tree->scale[1] * (tree->trans[1] + nodeMins[1]) - 0.5);
        if (v13 >= 0)
        {
            if (v13 <= 0xFFFF)
                outNode->aabb.mins[1] = (unsigned short)v13;
            else
                outNode->aabb.mins[1] = 0xFFFF;
        }
        else
        {
            outNode->aabb.mins[1] = 0;
        }

        v11 = (tree->scale[2] * (tree->trans[2] + nodeMins[2]) - 0.5);
        if (v11 >= 0)
        {
            if (v11 <= 0xFFFF)
                outNode->aabb.mins[2] = (unsigned short)v11;
            else
                outNode->aabb.mins[2] = 0xFFFF;
        }
        else
        {
            outNode->aabb.mins[2] = 0;
        }

        v9 = (tree->scale[0] * (tree->trans[0] + nodeMaxs[0]) + 0.5);
        if (v9 >= 0)
        {
            if (v9 <= 0xFFFF)
                outNode->aabb.maxs[0] = (unsigned short)v9;
            else
                outNode->aabb.maxs[0] = 0xFFFF;
        }
        else
        {
            outNode->aabb.maxs[0] = 0;
        }
        
        v7 = (tree->scale[1] * (tree->trans[1] + nodeMaxs[1]) + 0.5);
        if (v7 >= 0)
        {
            if (v7 <= 0xFFFF)
                outNode->aabb.maxs[1] = (unsigned short)v7;
            else
                outNode->aabb.maxs[1] = 0xFFFF;
        }
        else
        {
            outNode->aabb.maxs[1] = 0;
        }

        v5 = (tree->scale[2] * (tree->trans[2] + nodeMaxs[2]) + 0.5);
        if (v5 >= 0)
        {
            if (v5 <= 0xFFFF)
                outNode->aabb.maxs[2] = (unsigned short)v5;
            else
                outNode->aabb.maxs[2] = 0xFFFF;
        }
        else
        {
            outNode->aabb.maxs[2] = 0;
        }

        if (builtNode->childCount)
        {
            outNode->childBeginIndex = builtNode->firstChild;
            iassert(outNode->childBeginIndex == builtNode->firstChild);
            outNode->childCount = builtNode->childCount;
            iassert(outNode->childCount == builtNode->childCount);
        }
        else
        {
            iassert(builtNode->itemCount);
            outNode->childBeginIndex = builtNode->firstItem;
            iassert(outNode->childBeginIndex == builtNode->firstItem);
            outNode->childCount = builtNode->itemCount + 0x8000;
            iassert(outNode->childCount == builtNode->itemCount + 0x8000);
        }
    }
    free(options.mins);
    free(options.maxs);
    free(options.treeNodePool);
}

static void __cdecl XSurfaceTransferGetTexCoordRange(const XVertexInfo_s *v, int vertCount, float *texCoordAv)
{
    int vertIndex; // [esp+1Ch] [ebp-8h]
    char texCoordUnitRange[2]; // [esp+20h] [ebp-4h]

    texCoordAv[0] = 0.0f;
    texCoordAv[1] = 0.0f;
    texCoordUnitRange[0] = 1;
    texCoordUnitRange[1] = 1;
    for (vertIndex = 0; vertIndex < vertCount; ++vertIndex)
    {
        *texCoordAv = *texCoordAv + v->texCoordX;
        texCoordAv[1] = texCoordAv[1] + v->texCoordY;
        if (v->texCoordX < 0.0f || v->texCoordX > 1.0f)
            texCoordUnitRange[0] = 0;
        if (v->texCoordY < 0.0f || v->texCoordY > 1.0f)
            texCoordUnitRange[1] = 0;
    }
    texCoordAv[0] = (1.0f / (float)vertCount) * texCoordAv[0];
    texCoordAv[1] = (1.0f / (float)vertCount) * texCoordAv[1];

    if (texCoordUnitRange[0])
    {
        texCoordAv[0] = 0.0f;
    }
    else
    {
        texCoordAv[0] = floor((float)(texCoordAv[0] + 0.5f));
    }

    if (texCoordUnitRange[1])
    {
        texCoordAv[1] = 0.0f;
    }
    else
    {
        texCoordAv[1] = floor((float)(texCoordAv[1] + 0.5f));
    }
}

static void __cdecl XSurfaceTransfer_Position_GfxPackedVertex_(GfxPackedVertex *out, const XVertexInfo_s *v)
{
    out->xyz[0] = v->offset[0];
    out->xyz[1] = v->offset[1];
    out->xyz[2] = v->offset[2];
}

static void __cdecl XSurfaceTransfer_BinormalSign_GfxPackedVertex_(GfxPackedVertex *out, const XVertexInfo_s *v)
{
    float binormal[3]; // [esp+8h] [ebp-Ch] BYREF

    Vec3Cross(v->normal, v->tangent, binormal);
    if (Vec3Dot(binormal, v->binormal) < 0.0f)
        out->binormalSign = -1.0f;
    else
        out->binormalSign = 1.0f;
}

static void __cdecl XSurfaceTransfer_NormalTangent_GfxPackedVertex_(GfxPackedVertex *out, const XVertexInfo_s *v)
{
    out->normal = Vec3PackUnitVec(v->normal);
    out->tangent = Vec3PackUnitVec(v->tangent);
}

static void __cdecl XSurfaceTransfer_Texcoord_GfxPackedVertex_(
    GfxPackedVertex *out,
    const XVertexInfo_s *v,
    const float *texCoordAv)
{
    float texCoord[2]; // [esp+4h] [ebp-8h] BYREF

    texCoord[0] = v->texCoordX - *texCoordAv;
    texCoord[1] = v->texCoordY - texCoordAv[1];
    out->texCoord = Vec2PackTexCoords(texCoord);
}

static void __cdecl XSurfaceTransfer_Color_GfxPackedVertex_(GfxPackedVertex *out, const XVertexInfo_s *v)
{
    out->color.packed = *(_DWORD *)v->color;
}

void __cdecl XSurfaceTransfer(
    const XVertexBuffer *surfVerts,
    GfxPackedVertex *verts0,
    GfxPackedVertex *verts1,
    int vertCount)
{
    float texCoordAv[2];
    const XVertexInfo_s *v;

    iassert(vertCount);

    v = &surfVerts->v;

    XSurfaceTransferGetTexCoordRange(&surfVerts->v, vertCount, texCoordAv);

    for (int vertIndex = 0; vertIndex < vertCount; ++vertIndex)
    {
        XSurfaceTransfer_Position_GfxPackedVertex_(&verts0[vertIndex], v);
        XSurfaceTransfer_BinormalSign_GfxPackedVertex_(&verts0[vertIndex], v);
        XSurfaceTransfer_NormalTangent_GfxPackedVertex_(&verts1[vertIndex], v);
        XSurfaceTransfer_Texcoord_GfxPackedVertex_(&verts1[vertIndex], v, texCoordAv);
        XSurfaceTransfer_Color_GfxPackedVertex_(&verts1[vertIndex], v);

        v = (const XVertexInfo_s *)((char *)v + 4 * v->numWeights + 64);
    }
}

static void ReadBlend(XSurface *surface, int *partBits, XBlendLoadInfo *blend, unsigned char **pos)
{
    short boner = Buf_Read<short>(pos);
    partBits[boner >> 5] |= 0x80000000 >> (boner & 0x1F);

    blend->boneOffset = (boner << 6);
    blend->boneWeight = Buf_Read<unsigned short>(pos);
}

void __cdecl XModelReadSurface(XModel *model, unsigned char **pos, void *(__cdecl *Alloc)(int), XSurface *surface)
{
    int vertCount; // edx
    __int16 v32; // [esp+84h] [ebp-6BCh]
    float check[3]; // [esp+88h] [ebp-6B8h] BYREF
    int j; // [esp+94h] [ebp-6ACh]
    unsigned __int8 numWeights; // [esp+9Bh] [ebp-6A5h]
    int weightCount[4]; // [esp+9Ch] [ebp-6A4h] BYREF
    XVertexInfo0 *vert0Out; // [esp+ACh] [ebp-694h]
    XVertexInfo2 *vert2Out; // [esp+B0h] [ebp-690h]
    int boneIndex; // [esp+B4h] [ebp-68Ch]
    int numblends; // [esp+B8h] [ebp-688h]
    XBlendLoadInfo *blendOut; // [esp+BCh] [ebp-684h]
    int allocCount; // [esp+C0h] [ebp-680h]
    int blendBoneOffset; // [esp+C4h] [ebp-67Ch]
    XRigidVertList *rigidVertList; // [esp+C8h] [ebp-678h]
    int rigidVertCount; // [esp+CCh] [ebp-674h]
    int vertexBytes; // [esp+D0h] [ebp-670h]
    int size; // [esp+D4h] [ebp-66Ch]
    bool deformed; // [esp+DBh] [ebp-665h]
    XVertexInfo0 *verts0; // [esp+708h] [ebp-38h]
    XVertexInfo1 *verts1; // [esp+F0h] [ebp-650h]
    XVertexInfo2 *verts2; // [esp+DCh] [ebp-664h]
    XVertexInfo3 *verts3; // [esp+E0h] [ebp-660h]
    int startTriIndex; // [esp+E4h] [ebp-65Ch]
    unsigned int vertListIter; // [esp+E8h] [ebp-658h]
    int localBoneIndex; // [esp+ECh] [ebp-654h]
    int vertListCount; // [esp+F4h] [ebp-64Ch]
    XRigidVertList rigidVertListArray[129]; // [esp+F8h] [ebp-648h] BYREF
    XVertexInfo_s *verts; // [esp+710h] [ebp-30h]
    int blendBoneIndex; // [esp+714h] [ebp-2Ch]
    XVertexInfo1 *vert1Out; // [esp+718h] [ebp-28h]
    int endIndex; // [esp+71Ch] [ebp-24h]
    int i; // [esp+720h] [ebp-20h]
    unsigned __int16 *vertsBlendOut; // [esp+724h] [ebp-1Ch]
    int triIndex; // [esp+728h] [ebp-18h]
    int vertIndex; // [esp+72Ch] [ebp-14h]
    XVertexInfo0 *vertOut; // [esp+730h] [ebp-10h]
    int sizeInBytes; // [esp+734h] [ebp-Ch]
    XVertexBuffer *surfVerts; // [esp+738h] [ebp-8h]
    XVertexInfo3 *vert3Out; // [esp+73Ch] [ebp-4h]

    memset(weightCount, 0, sizeof(weightCount));
    
    surface->tileMode = Buf_Read<unsigned char>(pos);

    v32 = Buf_Read<unsigned short>(pos); // unused? what is this

    surface->vertCount = Buf_Read<unsigned short>(pos);

    surface->triCount = Buf_Read<unsigned short>(pos);

    iassert(surface->triCount > 0);

    vertListCount = 0;
    rigidVertCount = 0;

    while (1)
    {
        iassert(vertListCount < ARRAY_COUNT(rigidVertListArray));

        rigidVertList = &rigidVertListArray[vertListCount];
        rigidVertList->vertCount = Buf_Read<unsigned short>(pos);

        if (!rigidVertList->vertCount)
            break;

        localBoneIndex = Buf_Read<unsigned short>(pos);
        rigidVertList->boneOffset = localBoneIndex << 6;
        rigidVertCount += rigidVertList->vertCount;
        ++vertListCount;
    }

    vertCount = surface->vertCount;
    deformed = (rigidVertCount != vertCount);

    if (deformed)
        vertListCount = 0;

    surface->deformed = deformed;

    if (vertListCount == 1)
    {
        numblends = 0;
        boneIndex = rigidVertListArray[0].boneOffset >> 6;
        surface->partBits[rigidVertListArray[0].boneOffset >> 11] |= 0x80000000 >> (boneIndex & 0x1F);
    }
    else
    {
        numblends = Buf_Read<unsigned short>(pos);
    }

    size = (surface->vertCount << 6) + 4 * numblends;
    surfVerts = (XVertexBuffer*)Hunk_AllocateTempMemory(size, "XModelReadSurface");
    verts = &surfVerts->v;
    for (j = 0; j < surface->vertCount; ++j)
    {
        verts->normal[0] = Buf_Read<float>(pos);
        verts->normal[1] = Buf_Read<float>(pos);
        verts->normal[2] = Buf_Read<float>(pos);

        if (r_modelVertColor->current.enabled)
            Byte4CopyBgraToVertexColor(*pos, verts->color);
        else
            *(_DWORD *)verts->color = -1;

        *pos += 4;

        verts->texCoordX = Buf_Read<float>(pos);
        verts->texCoordY= Buf_Read<float>(pos);

        verts->binormal[0] = Buf_Read<float>(pos);
        verts->binormal[1] = Buf_Read<float>(pos);
        verts->binormal[2] = Buf_Read<float>(pos);

        verts->tangent[0] = Buf_Read<float>(pos);
        verts->tangent[1] = Buf_Read<float>(pos);
        verts->tangent[2] = Buf_Read<float>(pos);

        check[0] = Vec3Dot(verts->normal, verts->tangent);
        check[1] = Vec3Dot(verts->tangent, verts->binormal);
        check[2] = Vec3Dot(verts->binormal, verts->normal);

        iassert(VecNCompareCustomEpsilon(check, vec3_origin, EQUAL_EPSILON * 2, 3));

        if (vertListCount == 1)
        {
            iassert(!deformed);

            verts->numWeights = 0;
            verts->boneOffset = rigidVertListArray[0].boneOffset;

            verts->offset[0] = Buf_Read<float>(pos);
            verts->offset[1] = Buf_Read<float>(pos);
            verts->offset[2] = Buf_Read<float>(pos);

            ++verts;
        }
        else
        {
            numWeights = Buf_Read<unsigned char>(pos);
            verts->numWeights = numWeights;

            iassert(numWeights < 4);

            ++weightCount[numWeights];

            blendBoneIndex = Buf_Read<unsigned short>(pos);

            surface->partBits[blendBoneIndex >> 5] |= 0x80000000 >> (blendBoneIndex & 0x1F);
            blendBoneOffset = blendBoneIndex << 6;
            verts->boneOffset = (_WORD)blendBoneIndex << 6;
            iassert(blendBoneOffset == verts->boneOffset);

            verts->offset[0] = Buf_Read<float>(pos);
            verts->offset[1] = Buf_Read<float>(pos);
            verts->offset[2] = Buf_Read<float>(pos);

            ++verts;

            if (numWeights)
            {
                iassert(deformed);

                for (i = 0; i < numWeights; ++i)
                {
                    ReadBlend(surface, surface->partBits, (XBlendLoadInfo *)verts, pos);
                    verts = (XVertexInfo_s *)((char *)verts + 4);
                }
            }
        }
    }
    allocCount = (surface->triCount + 1) & 0xFFFFFFFE;
    sizeInBytes = 6 * (surface->triCount + 1);
    surface->triIndices = (unsigned short*)Alloc(sizeInBytes);

    iassert(surface->triIndices);

    for (vertIndex = 0; vertIndex < 3 * surface->triCount; ++vertIndex)
    {
        surface->triIndices[vertIndex] = Buf_Read<unsigned short>(pos);

        iassert(surface->triIndices[vertIndex] < surface->vertCount);
    }

    triIndex = 0;
    endIndex = 0;
    for (j = 0; j < vertListCount; ++j)
    {
        startTriIndex = triIndex;
        rigidVertListArray[j].triOffset = triIndex;

        iassert(rigidVertListArray[j].triOffset == startTriIndex);

        endIndex += rigidVertListArray[j].vertCount;
        while (triIndex < surface->triCount && surface->triIndices[3 * triIndex] < endIndex)
            ++triIndex;
        rigidVertListArray[j].triCount = triIndex - startTriIndex;

        iassert(rigidVertListArray[j].triCount == triIndex - startTriIndex);
    }

    if (allocCount != surface->triCount)
    {
        iassert(allocCount == surface->triCount + 1);

        surface->triIndices[vertIndex] = surface->triIndices[vertIndex - 1];
        surface->triIndices[vertIndex + 1] = surface->triIndices[vertIndex - 1];
        surface->triIndices[vertIndex + 2] = surface->triIndices[vertIndex - 1];
        ++surface->triCount;
    }

    if (vertListCount)
        surface->vertList = (XRigidVertList *)Alloc(sizeof(XRigidVertList) * vertListCount);
    else
        surface->vertList = 0;

    surface->vertListCount = vertListCount;
    memcpy(surface->vertList, rigidVertListArray, sizeof(XRigidVertList) * vertListCount);
    vertexBytes = sizeof(GfxPackedVertex) * surface->vertCount;
    surface->verts0 = (GfxPackedVertex *)Alloc(vertexBytes);
    memset(surface->verts0, 0, vertexBytes); // Add from blops
    model->memUsage += vertexBytes;
    XSurfaceTransfer(surfVerts, surface->verts0, surface->verts0, surface->vertCount);
    if (deformed)
    {
        iassert(XModelNumBones(model) > 1);
        verts0 = 0;
        verts1 = 0;
        verts2 = 0;
        verts3 = 0;
        if (weightCount[0])
        {
            size = 2 * weightCount[0];
            verts0 = (XVertexInfo0 *)Alloc(2 * weightCount[0]);
        }
        if (weightCount[1])
        {
            size = 6 * weightCount[1];
            verts1 = (XVertexInfo1 *)Alloc(6 * weightCount[1]);
        }
        if (weightCount[2])
        {
            size = 10 * weightCount[2];
            verts2 = (XVertexInfo2 *)Alloc(10 * weightCount[2]);
        }
        if (weightCount[3])
        {
            size = 14 * weightCount[3];
            verts3 = (XVertexInfo3 *)Alloc(14 * weightCount[3]);
        }

        for (i = 0; i < 4; ++i)
        {
            surface->vertInfo.vertCount[i] = weightCount[i];
            iassert(surface->vertInfo.vertCount[i] == weightCount[i]);
        }

        verts = &surfVerts->v;
        vert0Out = verts0;
        vert1Out = verts1;
        vert2Out = verts2;
        vert3Out = verts3;

        for (j = 0; j < surface->vertCount; ++j)
        {
            if (verts->numWeights)
            {
                if (verts->numWeights == 1)
                {
                    iassert(vert1Out);
                    vertOut = &vert1Out->vert0;
                    blendOut = vert1Out->blend;
                    ++vert1Out;
                }
                else if (verts->numWeights == 2)
                {
                    iassert(vert2Out);
                    vertOut = &vert2Out->vert0;
                    blendOut = vert2Out->blend;
                    ++vert2Out;
                }
                else
                {
                    iassert(verts->numWeights == 3);
                    iassert(vert3Out);
                    vertOut = &vert3Out->vert0;
                    blendOut = vert3Out->blend;
                    ++vert3Out;
                }
            }
            else
            {
                iassert(vert0Out);
                vertOut = vert0Out;
                blendOut = 0;
                ++vert0Out;
            }
            vertOut->boneOffset = verts->boneOffset;
            numWeights = verts->numWeights;
            ++verts;
            if (numWeights)
            {
                iassert(blendOut);
                for (i = 0; i < numWeights; ++i)
                {
                    blendOut->boneOffset = LOWORD(verts->normal[0]);
                    blendOut->boneWeight = HIWORD(verts->normal[0]);
                    ++blendOut;
                    verts = (XVertexInfo_s*)((char*)verts + 4);
                }
            }
        }

        iassert(vert0Out == verts0 + surface->vertInfo.vertCount[0]);
        iassert(vert1Out == verts1 + surface->vertInfo.vertCount[1]);
        iassert(vert2Out == verts2 + surface->vertInfo.vertCount[2]);
        iassert(vert3Out == verts3 + surface->vertInfo.vertCount[3]);

        size = 2
            * (7 * surface->vertInfo.vertCount[3]
                + 5 * surface->vertInfo.vertCount[2]
                + 3 * surface->vertInfo.vertCount[1]
                + surface->vertInfo.vertCount[0]);

        if (size)
            vertsBlendOut = (unsigned short*)Alloc(size);
        else
            vertsBlendOut = 0;

        model->memUsage += size;
        surface->vertInfo.vertsBlend = vertsBlendOut;
        if (surface->vertInfo.vertCount[0])
        {
            vert0Out = verts0;
            j = 0;
            while (j < surface->vertInfo.vertCount[0])
            {
                *vertsBlendOut++ = vert0Out->boneOffset;
                ++j;
                ++vert0Out;
            }
        }
        if (surface->vertInfo.vertCount[1])
        {
            vert1Out = verts1;
            j = 0;
            while (j < surface->vertInfo.vertCount[1])
            {
                *vertsBlendOut++ = vert1Out->vert0.boneOffset;
                *vertsBlendOut++ = vert1Out->blend[0].boneOffset;
                *vertsBlendOut++ = vert1Out->blend[0].boneWeight;
                ++j;
                ++vert1Out;
            }
        }
        if (surface->vertInfo.vertCount[2])
        {
            vert2Out = verts2;
            j = 0;
            while (j < surface->vertInfo.vertCount[2])
            {
                *vertsBlendOut++ = vert2Out->vert0.boneOffset;
                *vertsBlendOut++ = vert2Out->blend[0].boneOffset;
                *vertsBlendOut++ = vert2Out->blend[0].boneWeight;
                *vertsBlendOut++ = vert2Out->blend[1].boneOffset;
                *vertsBlendOut++ = vert2Out->blend[1].boneWeight;
                ++j;
                ++vert2Out;
            }
        }
        if (surface->vertInfo.vertCount[3])
        {
            vert3Out = verts3;
            j = 0;
            while (j < surface->vertInfo.vertCount[3])
            {
                *vertsBlendOut++ = vert3Out->vert0.boneOffset;
                *vertsBlendOut++ = vert3Out->blend[0].boneOffset;
                *vertsBlendOut++ = vert3Out->blend[0].boneWeight;
                *vertsBlendOut++ = vert3Out->blend[1].boneOffset;
                *vertsBlendOut++ = vert3Out->blend[1].boneWeight;
                *vertsBlendOut++ = vert3Out->blend[2].boneOffset;
                *vertsBlendOut++ = vert3Out->blend[2].boneWeight;
                ++j;
                ++vert3Out;
            }
        }

        iassert((byte *)vertsBlendOut - (byte *)(surface->vertInfo.vertsBlend) == size);
    }
    iassert(surface->deformed == (surface->vertListCount == 0));

    for (vertListIter = 0; vertListIter != surface->vertListCount; ++vertListIter)
        XModelReadSurface_BuildCollisionTree(surface, vertListIter, Alloc);

    Hunk_FreeTempMemory((char*)surfVerts);
}

void __cdecl XModelReadSurfaces(
    XModel *model,
    const char *name,
    XModelSurfs *modelSurfs,
    int *modelPartBits,
    int surfCount,
    unsigned char **pos,
    void *(__cdecl *AllocMesh)(int))
{
    int j; // [esp+4h] [ebp-18h]
    int surfIndex; // [esp+8h] [ebp-14h]
    XSurface *surfs; // [esp+18h] [ebp-4h]

    int baseTriIndex; // [esp+14h] [ebp-8h]
    int baseVertIndex; // [esp+Ch] [ebp-10h]

    surfs = modelSurfs->surfs;

    iassert(surfs);
    iassert(modelPartBits);
    iassert(surfCount > 0);
    iassert(pos);
    iassert(*pos);

    baseTriIndex = 0;
    baseVertIndex = 0;
    for (surfIndex = 0; surfIndex < surfCount; ++surfIndex)
    {
        XSurface *xsurf = &surfs[surfIndex];

        XModelReadSurface(model, pos, AllocMesh, xsurf);

        for (j = 0; j < 4; ++j)
            modelPartBits[j] |= xsurf->partBits[j];

        xsurf->baseTriIndex = baseTriIndex;
        xsurf->baseVertIndex = baseVertIndex;
        baseTriIndex += xsurf->triCount;
        baseVertIndex += xsurf->vertCount;
    }
}

XModelSurfs *__cdecl R_XModelSurfsLoadFile(
    XModel *model,
    const char *name,
    void *(__cdecl *Alloc)(int),
    __int16 modelNumsurfs,
    const char *modelName)
{
    unsigned __int8 *pos; // [esp+8h] [ebp-64h] BYREF
    char filename[68]; // [esp+Ch] [ebp-60h] BYREF
    unsigned __int8 *buf = NULL; // [esp+54h] [ebp-18h] BYREF
    XModelSurfs *modelSurfs; // [esp+5Ch] [ebp-10h]
    int size; // [esp+60h] [ebp-Ch]
    int fileSize; // [esp+64h] [ebp-8h]
    __int16 numsurfs; // [esp+68h] [ebp-4h]

    if (Com_sprintf(filename, 0x40u, "xmodelsurfs/%s", name) < 0)
    {
        Com_PrintError(19, "ERROR: filename '%s' too long\n", filename);
        return 0;
    }

    fileSize = FS_ReadFile(filename, (void**)&buf);

    if (fileSize < 0)
    {
        iassert(!buf);
        Com_PrintError(19, "ERROR: xmodelsurf '%s' not found\n", name);
        return 0;
    }

    if (!fileSize)
    {
        Com_PrintError(19, "ERROR: xmodelsurf '%s' has 0 length\n", name);
        FS_FreeFile((char *)buf);
        return 0;
    }

    iassert(buf);

    pos = buf;
    short version = Buf_Read<short>(&pos);

    if (version == 25)
    {
        numsurfs = Buf_Read<short>(&pos);

        if (numsurfs == modelNumsurfs)
        {
            size = sizeof(XSurface) * modelNumsurfs + sizeof(XModelSurfs);
            modelSurfs = (XModelSurfs*)Alloc(size);
            model->memUsage += size;
            modelSurfs->surfs = (XSurface*)&modelSurfs[1];
            XModelReadSurfaces(model, name, modelSurfs, modelSurfs->partBits, modelNumsurfs, &pos, Alloc);
            FS_FreeFile((char*)buf);

            iassert(modelSurfs);
            return modelSurfs;
        }
        else
        {
            FS_FreeFile((char*)buf);
            Com_PrintError(
                19,
                "ERROR: File conflict (between non-iwd and iwd file) on xmodelsurfs '%s' for xmodel '%s'.\n"
                "Rename the export file to fix.\n",
                name,
                modelName);
            return 0;
        }
    }
    else
    {
        FS_FreeFile((char*)buf);
        Com_PrintError(19, "ERROR: xmodelsurfs '%s' out of date (version %d, expecting %d).\n", name, version, 25);
        return 0;
    }
}

void __cdecl XModelSurfsSetData(const char *name, XModelSurfs *modelSurfs, void *(__cdecl *Alloc)(int))
{
    Hunk_SetDataForFile(3, name, modelSurfs, Alloc);
}

int __cdecl XModelSurfsPrecache(
    XModel *model,
    const char *name,
    void *(__cdecl *Alloc)(int),
    __int16 modelNumsurfs,
    const char *modelName,
    XModelSurfs *outModelSurfs)
{
    XModelSurfs *modelSurfs; // [esp+0h] [ebp-4h]
    XModelSurfs *modelSurfsa; // [esp+0h] [ebp-4h]

    modelSurfs = XModelSurfsFindData(name);
    if (modelSurfs)
    {
        *outModelSurfs = *modelSurfs;
        return 1;
    }
    else
    {
        modelSurfsa = R_XModelSurfsLoadFile(model, name, Alloc, modelNumsurfs, modelName);
        if (modelSurfsa)
        {
            XModelSurfsSetData(name, modelSurfsa, Alloc);
            *outModelSurfs = *modelSurfsa;
            return 1;
        }
        else
        {
            Com_PrintError(19, "ERROR: Cannot find 'xmodelsurfs '%s'.\n", name);
            return 0;
        }
    }
}

PhysPreset *__cdecl XModel_PhysPresetPrecache(const char *name, void *(__cdecl *Alloc)(int))
{
    iassert(name);

    return PhysPresetPrecache(name, Alloc);
}

void __cdecl XModelLoadCollData(
    unsigned __int8 **pos,
    XModel *model,
    void *(__cdecl *AllocColl)(int),
    const char *name)
{
    iassert(!model->contents);

    model->numCollSurfs = Buf_Read<int>(pos);

    if (model->numCollSurfs)
    {
        model->collSurfs = (XModelCollSurf_s *)AllocColl(44 * model->numCollSurfs);
        for (int i = 0; i < model->numCollSurfs; ++i)
        {
            XModelCollSurf_s *surf = &model->collSurfs[i];

            int numCollTris = Buf_Read<int>(pos);

            iassert(numCollTris);

            surf->numCollTris = numCollTris;
            surf->collTris = (XModelCollTri_s *)AllocColl(sizeof(XModelCollTri_s) * numCollTris);

            for (int j = 0; j < numCollTris; ++j)
            {
                float plane[4];

                plane[0] = Buf_Read<float>(pos);
                plane[1] = Buf_Read<float>(pos);
                plane[2] = Buf_Read<float>(pos);
                plane[3] = Buf_Read<float>(pos);

                iassert(I_fabs(Vec3Length(plane) - 1.0f) < 0.01f); // I_I_fabs?

                XModelCollTri_s *tri = &surf->collTris[j];

                tri->svec[0] = Buf_Read<float>(pos);
                tri->svec[1] = Buf_Read<float>(pos);
                tri->svec[2] = Buf_Read<float>(pos);
                tri->svec[3] = Buf_Read<float>(pos);

                tri->tvec[0] = Buf_Read<float>(pos);
                tri->tvec[1] = Buf_Read<float>(pos);
                tri->tvec[2] = Buf_Read<float>(pos);
                tri->tvec[3] = Buf_Read<float>(pos);

                tri->plane[0] = plane[0];
                tri->plane[1] = plane[1];
                tri->plane[2] = plane[2];
                tri->plane[3] = plane[3];
            }

            surf->mins[0] = Buf_Read<float>(pos) - EQUAL_EPSILON;
            surf->mins[1] = Buf_Read<float>(pos) - EQUAL_EPSILON;
            surf->mins[2] = Buf_Read<float>(pos) - EQUAL_EPSILON;

            surf->maxs[0] = Buf_Read<float>(pos) + EQUAL_EPSILON;
            surf->maxs[1] = Buf_Read<float>(pos) + EQUAL_EPSILON;
            surf->maxs[2] = Buf_Read<float>(pos) + EQUAL_EPSILON;

            surf->boneIdx = Buf_Read<int>(pos);

            surf->contents = Buf_Read<int>(pos) & 0xDFFFFFFB;

            iassert(!surf->contents || (surf->boneIdx >= 0));

            surf->surfFlags = Buf_Read<int>(pos);
            model->contents |= surf->contents;
        }
    }
    else if (model->collSurfs)
    {
        MyAssertHandler(".\\xanim\\xmodel_load_obj.cpp", 323, 0, "%s", "!model->collSurfs");
    }
}

char __cdecl XModelLoadConfigFile(const char *name, unsigned __int8 **pos, XModelConfig *config)
{
    short version = Buf_Read<short>(pos);

    if (version != 25)
    {
        Com_PrintError(19, "ERROR: xmodel '%s' out of date (version %d, expecting %d).\n", name, version, 25);
        return 0;
    }

    config->flags = Buf_Read<unsigned char>(pos); // DWORD in blops

    config->mins[0] = Buf_Read<float>(pos);
    config->mins[1] = Buf_Read<float>(pos);
    config->mins[2] = Buf_Read<float>(pos);

    config->maxs[0] = Buf_Read<float>(pos);
    config->maxs[1] = Buf_Read<float>(pos);
    config->maxs[2] = Buf_Read<float>(pos);

    char *str = (char *)*pos;
    char *out = config->physicsPresetFilename;

    char c;
    do
    {
        c = *str;
        *out++ = *str++;
    } while (c);
    *pos += strlen((const char *)*pos) + 1;

    for (int i = 0; i < 4; i++)
    {
        config->entries[i].dist = Buf_Read<float>(pos);

        char *name = (char*)*pos;
        char *entryfilename = config->entries[i].filename;

        do
        {
            c = *name;
            *entryfilename++ = *name++;
        } while (c);
        *pos += strlen((const char *)*pos) + 1;
    }

    config->collLod = Buf_Read<int>(pos);

    return 1;
}

bool __cdecl XModelAllowLoadMesh()
{
#ifdef KISAK_MP
    return com_dedicated->current.integer == 0;
#elif KISAK_SP
    return true;
#endif
}

static XModelPartsLoad *__cdecl XModelPartsFindData(const char *name)
{
    return (XModelPartsLoad *)Hunk_FindDataForFile(4, name);
}

void __cdecl XModelPartsSetData(const char *name, XModelPartsLoad *modelParts, void *(__cdecl *Alloc)(int))
{
    Hunk_SetDataForFile(4, name, modelParts, Alloc);
}

XModelPartsLoad *__cdecl XModelPartsLoadFile(XModel *model, const char *name, void *(__cdecl *Alloc)(int));

XModelPartsLoad *__cdecl XModelPartsPrecache(XModel *model, const char *name, void *(__cdecl *Alloc)(int))
{
    XModelPartsLoad *modelParts; // [esp+0h] [ebp-4h]

    modelParts = XModelPartsFindData(name);
    if (modelParts)
        return modelParts;

    modelParts = XModelPartsLoadFile(model, name, Alloc);
    if (modelParts)
    {
        XModelPartsSetData(name, modelParts, Alloc);
        return modelParts;
    }
    else
    {
        Com_PrintError(19, "ERROR: Cannot find xmodelparts '%s'.\n", name);
        return 0;
    }
}

void __cdecl XModelCopyXModelParts(const XModelPartsLoad *modelParts, XModel *model)
{
    model->numBones = modelParts->numBones;
    model->numRootBones = modelParts->numRootBones;
    model->boneNames = modelParts->boneNames;
    model->parentList = modelParts->parentList;
    model->quats = modelParts->quats;
    model->trans = modelParts->trans;
    model->partClassification = modelParts->partClassification;
    model->baseMat = modelParts->baseMat;
}

XModel *__cdecl XModelLoadFile(char *name, void *(__cdecl *Alloc)(int), void *(__cdecl *AllocColl)(int))
{
    double v4; // st7
    int *partBits; // edx
    Material *v6; // eax
    PhysPreset *v7; // eax
    PhysGeomList *PhysicsCollMap; // eax
    float v9; // [esp+2Ch] [ebp-1648h]
    float v10; // [esp+4Ch] [ebp-1628h]
    float v11; // [esp+50h] [ebp-1624h]
    float v12; // [esp+54h] [ebp-1620h]
    float v13; // [esp+58h] [ebp-161Ch]
    float v14; // [esp+5Ch] [ebp-1618h]
    float v15; // [esp+60h] [ebp-1614h]
    unsigned __int16 v16; // [esp+64h] [ebp-1610h]
    unsigned __int8 *pos; // [esp+68h] [ebp-160Ch] BYREF
    int j; // [esp+6Ch] [ebp-1608h]
    int numBones; // [esp+70h] [ebp-1604h]
    char dest[68]; // [esp+74h] [ebp-1600h] BYREF
    float diff[12]; // [esp+B8h] [ebp-15BCh] BYREF
    int surfIndex; // [esp+E8h] [ebp-158Ch]
    XModel *model; // [esp+ECh] [ebp-1588h]
    float *a; // [esp+F0h] [ebp-1584h]
    void *buf = NULL; // [esp+F4h] [ebp-1580h] BYREF
    XModelLodInfo *modelLodInfo; // [esp+F8h] [ebp-157Ch]
    float *sum; // [esp+FCh] [ebp-1578h]
    const char *v29; // [esp+104h] [ebp-1570h]
    int filelen; // [esp+108h] [ebp-156Ch]
    char v31[256]; // [esp+10Ch] [ebp-1568h] BYREF
    float *b; // [esp+20Ch] [ebp-1468h]
    XModelSurfs outModelSurfs; // [esp+210h] [ebp-1464h] BYREF
    XBoneInfo *boneInfos; // [esp+224h] [ebp-1450h]
    int i; // [esp+228h] [ebp-144Ch]
    int numsurfs; // [esp+230h] [ebp-1444h]
    XModelConfig config; // [esp+234h] [ebp-1440h] BYREF
    XModelPartsLoad *modelParts; // [esp+166Ch] [ebp-8h]
    const char *v40; // [esp+1670h] [ebp-4h]
    unsigned __int8 *v36;

    if (Com_IsLegacyXModelName(name))
    {
        Com_PrintError(19, "ERROR: Remove xmodel prefix from model name '%s'\n", name);
        return 0;
    }

    if (Com_sprintf(dest, 0x40u, "xmodel/%s", name) < 0)
    {
        Com_PrintError(19, "ERROR: filename '%s' too long\n", dest);
        return 0;
    }

    filelen = FS_ReadFile(dest, &buf);

    if (filelen < 0)
    {
        iassert(!buf);
        Com_PrintError(19, "ERROR: xmodel '%s' not found\n", name);
        return 0;
    }

    if (!filelen)
    {
        Com_PrintError(19, "ERROR: xmodel '%s' has 0 length\n", name);
        FS_FreeFile((char *)buf);
        return 0;
    }

    pos = (unsigned __int8 *)buf;
    if (!XModelLoadConfigFile(name, &pos, &config))
        goto LABEL_28;

    model = (XModel *)Alloc(sizeof(XModel));
    model->memUsage = sizeof(XModel);
    XModelLoadCollData(&pos, model, AllocColl, name);

    model->numLods = 0;
    v36 = pos;
    numsurfs = 0;
    for (i = 0; i < 4; ++i)
    {
        modelLodInfo = &model->lodInfo[i];
        if (config.entries[i].filename[0])
        {
            iassert(i == model->numLods);
            ++model->numLods;

            modelLodInfo->numsurfs = Buf_Read<unsigned short>(&pos);

            numsurfs += modelLodInfo->numsurfs;
            for (j = 0; j < modelLodInfo->numsurfs; ++j)
            {
                v40 = (const char *)pos;
                pos += strlen((const char *)pos) + 1;
            }
        }
        modelLodInfo->dist = (config.entries[i].dist == 0.0f) ? 1000000.0f : config.entries[i].dist;
    }

    iassert(model->numLods);

    modelParts = XModelPartsPrecache(model, (const char *)&config, Alloc);
    if (modelParts)
    {
        XModelCopyXModelParts(modelParts, model);
        numBones = model->numBones;
        boneInfos = (XBoneInfo *)Alloc(sizeof(XBoneInfo) * numBones);
        model->memUsage += (sizeof(XBoneInfo) * numBones);
        for (i = 0; i < numBones; ++i)
        {
            a = boneInfos[i].bounds[0];
            a[0] = Buf_Read<float>(&pos);
            a[1] = Buf_Read<float>(&pos);
            a[2] = Buf_Read<float>(&pos);

            b = boneInfos[i].bounds[1];
            b[0] = Buf_Read<float>(&pos);
            b[1] = Buf_Read<float>(&pos);
            b[2] = Buf_Read<float>(&pos);

            sum = boneInfos[i].offset;
            Vec3Avg(a, b, sum);
            Vec3Sub(b, sum, diff);

            boneInfos[i].radiusSquared = Vec3LengthSq(diff);
        }
        model->boneInfo = boneInfos;
        model->lodRampType = 0;
        if (XModelAllowLoadMesh())
        {
            pos = v36;
            iassert(config.entries[0].filename[0]);
            model->numsurfs = numsurfs;
            model->surfs = (XSurface *)Alloc(sizeof(XSurface) * numsurfs);
            model->materialHandles = (Material **)Alloc(4 * numsurfs);
            surfIndex = 0;
            for (i = 0; i < 4; ++i)
            {
                modelLodInfo = &model->lodInfo[i];
                if (config.entries[i].filename[0])
                {
                    pos += 2;
                    if (!XModelSurfsPrecache(model, config.entries[i].filename, Alloc, modelLodInfo->numsurfs, name, &outModelSurfs))
                        goto LABEL_28;
                    partBits = modelLodInfo->partBits;
                    modelLodInfo->partBits[0] = outModelSurfs.partBits[0];
                    partBits[1] = outModelSurfs.partBits[1];
                    partBits[2] = outModelSurfs.partBits[2];
                    partBits[3] = outModelSurfs.partBits[3];
                    modelLodInfo->surfIndex = surfIndex;
                    iassert(i == (unsigned __int8)i);
                    modelLodInfo->lod = i;
                    modelLodInfo->smcIndexPlusOne = 0;

                    for (j = 0; j < modelLodInfo->numsurfs; ++j)
                    {
                        v40 = (const char *)pos;
                        pos += strlen((const char *)pos) + 1;
                        if (!strcmp(v40, "$default"))
                            v40 = "$default3d";
                        v29 = "mc/";
                        Com_sprintf(v31, 0x100u, "%s%s", "mc/", v40);
                        v6 = Material_RegisterHandle(v31, 8);
                        model->materialHandles[surfIndex] = v6;
                        if (outModelSurfs.surfs[j].deformed)
                            model->lodRampType = 1;
                        qmemcpy(&model->surfs[surfIndex++], &outModelSurfs.surfs[j], sizeof(model->surfs[surfIndex++]));
                    }
                }
            }

            iassert(surfIndex == numsurfs);

            diff[3] = 1.0;
            diff[4] = 0.0;
            diff[5] = 0.0;
            diff[6] = 0.0;
            diff[7] = 1.0;
            diff[8] = 0.0;
            diff[9] = 0.0;
            diff[10] = 0.0;
            diff[11] = 1.0;

            R_GetXModelBounds(model, (const float (*)[3]) & diff[3], model->mins, model->maxs);
        }

        FS_FreeFile((char *)buf);
        iassert(config.maxs[0] >= 0.0f);
        model->radius = config.maxs[0];
        model->collLod = config.collLod;
        iassert(model->collLod < model->numLods);
        model->flags = config.flags;
        if (config.physicsPresetFilename[0])
        {
            v7 = XModel_PhysPresetPrecache(config.physicsPresetFilename, Alloc);
            model->physPreset = v7;
        }
        PhysicsCollMap = XModel_LoadPhysicsCollMap(name, Alloc);
        model->physGeoms = PhysicsCollMap;
        return model;
    }
    else
    {
    LABEL_28:
        FS_FreeFile((char *)buf);
        return 0;
    }
}

void __cdecl XModelCalcBasePose(XModelPartsLoad *modelParts)
{
    float result[3]; // [esp+44h] [ebp-74h] BYREF
    float len; // [esp+88h] [ebp-30h]
    int numBones; // [esp+90h] [ebp-28h]
    float *trans; // [esp+94h] [ebp-24h]
    __int16 *quats; // [esp+98h] [ebp-20h]
    unsigned __int8 *parentList; // [esp+9Ch] [ebp-1Ch]
    int i; // [esp+A0h] [ebp-18h]
    float tempQuat[4]; // [esp+A4h] [ebp-14h] BYREF
    DObjAnimMat *quatTrans; // [esp+B4h] [ebp-4h]

    parentList = modelParts->parentList;
    numBones = modelParts->numBones;
    quats = modelParts->quats;
    trans = modelParts->trans;
    quatTrans = modelParts->baseMat;

    for (i = modelParts->numRootBones; i; --i)
    {
        quatTrans->quat[0] = 0.0f;
        quatTrans->quat[1] = 0.0f;
        quatTrans->quat[2] = 0.0f;
        quatTrans->quat[3] = 1.0f;

        quatTrans->trans[0] = 0.0f;
        quatTrans->trans[1] = 0.0f;
        quatTrans->trans[2] = 0.0f;

        quatTrans->transWeight = 2.0f;
        ++quatTrans;
    }

    i = numBones - modelParts->numRootBones;

    while (i)
    {
        tempQuat[0] = quats[0] * 0.00003051850944757462;
        tempQuat[1] = quats[1] * 0.00003051850944757462;
        tempQuat[2] = quats[2] * 0.00003051850944757462;
        tempQuat[3] = quats[3] * 0.00003051850944757462;
        QuatMultiply(tempQuat, quatTrans[-*parentList].quat, quatTrans->quat);
        len = Vec4LengthSq(quatTrans->quat);
        if (len == 0.0f)
        {
            quatTrans->quat[3] = 1.0f;
            quatTrans->transWeight = 2.0f;
        }
        else
        {
            quatTrans->transWeight = 2.0f / len;
        }
        MatrixTransformVectorQuatTrans(trans, &quatTrans[-*parentList], quatTrans->trans);
        --i;
        quats += 4;
        trans += 3;
        ++quatTrans;
        ++parentList;
    }
}

XModelPartsLoad *__cdecl XModelPartsLoadFile(XModel *model, const char *name, void *(__cdecl *Alloc)(int))
{
    unsigned __int16 prev; // ax
    unsigned __int8 *pos; // [esp+30h] [ebp-88h] BYREF
    int numBones; // [esp+34h] [ebp-84h]
    char filename[64]; // [esp+38h] [ebp-80h] BYREF
    int numRootBones; // [esp+7Ch] [ebp-3Ch]
    __int16 numChildBones; // [esp+80h] [ebp-38h]
    unsigned __int8 *buf = NULL; // [esp+84h] [ebp-34h] BYREF
    float *trans; // [esp+88h] [ebp-30h]
    __int16 version; // [esp+8Ch] [ebp-2Ch]
    int len; // [esp+90h] [ebp-28h]
    int size; // [esp+94h] [ebp-24h]
    __int16 *quats; // [esp+98h] [ebp-20h]
    int fileSize; // [esp+9Ch] [ebp-1Ch]
    unsigned __int8 *parentList; // [esp+A0h] [ebp-18h]
    int index; // [esp+A4h] [ebp-14h]
    int i; // [esp+A8h] [ebp-10h]
    XModelPartsLoad *modelParts; // [esp+ACh] [ebp-Ch]
    bool useBones; // [esp+B3h] [ebp-5h]
    unsigned __int16 *boneNames; // [esp+B4h] [ebp-4h]

    if (Com_sprintf(filename, 0x40u, "xmodelparts/%s", name) < 0)
    {
        Com_PrintError(19, "ERROR: filename '%s' too long\n", filename);
        return 0;
    }

    fileSize = FS_ReadFile(filename, (void **)&buf);

    if (fileSize < 0)
    {
        iassert(!buf);
        Com_PrintError(19, "ERROR: xmodelparts '%s' not found\n", name);
        return 0;
    }

    if (!fileSize)
    {
        Com_PrintError(19, "ERROR: xmodelparts '%s' has 0 length\n", name);
        FS_FreeFile((char *)buf);
        return 0;
    }

    iassert(buf);
    pos = buf;

    version = Buf_Read<unsigned short>(&pos);

    if (version != 25)
    {
        FS_FreeFile((char *)buf);
        Com_PrintError(19, "ERROR: xmodelparts '%s' out of date (version %d, expecting %d).\n", name, version, 25);
        return 0;
    }

    numChildBones = Buf_Read<unsigned short>(&pos);
    numRootBones = Buf_Read<unsigned short>(&pos);
    numBones = numRootBones + numChildBones;
    size = 2 * numBones;
    boneNames = (unsigned __int16 *)Alloc(2 * numBones);
    model->memUsage += size;

    if (numBones < 128)
    {
        size = numChildBones;
        if (numChildBones)
            parentList = (unsigned __int8 *)Alloc(size);
        else
            parentList = 0;

        model->memUsage += size;
        size = sizeof(XModelPartsLoad);
        modelParts = (XModelPartsLoad *)Alloc(size);
        model->memUsage += size;
        modelParts->parentList = parentList;
        modelParts->boneNames = boneNames;
        size = sizeof(DObjAnimMat) * numBones;
        modelParts->baseMat = (DObjAnimMat *)Alloc(size);
        model->memUsage += size;
        if (numChildBones)
        {
            size = 8 * numChildBones;
            modelParts->quats = (__int16 *)Alloc(size);
            model->memUsage += size;

            size = 16 * numChildBones;
            modelParts->trans = (float *)Alloc(size);
            model->memUsage += size;
        }
        else
        {
            modelParts->quats = 0;
            modelParts->trans = 0;
        }
        size = numBones;
        modelParts->partClassification = (unsigned __int8 *)Alloc(numBones);
        model->memUsage += size;
        modelParts->numBones = numBones;
        modelParts->numRootBones = numRootBones;
        quats = modelParts->quats;
        trans = modelParts->trans;
        i = numRootBones;
        while (i < numBones)
        {
            index = *pos++;
            iassert(index < i);
            *parentList = i - index;

            trans[0] = Buf_Read<float>(&pos);
            trans[1] = Buf_Read<float>(&pos);
            trans[2] = Buf_Read<float>(&pos);

            ConsumeQuatNoSwap(&pos, quats);
            ++i;
            quats += 4;
            trans += 3;
            ++parentList;
        }
        for (i = 0; i < numBones; ++i)
        {
            len = strlen((const char *)pos) + 1;
            prev = SL_GetStringOfSize((char *)pos, 0, len, 10).prev;
            boneNames[i] = prev;
            pos += len;
        }
        memcpy(modelParts->partClassification, (unsigned __int8 *)pos, numBones);
        pos += numBones;
        useBones = *pos++ != 0;
        FS_FreeFile((char *)buf);
        XModelCalcBasePose(modelParts);
        if (!useBones)
            memset((unsigned __int8 *)modelParts->trans, 0, 16 * numChildBones);
        return modelParts;
    }
    else
    {
        FS_FreeFile((char *)buf);
        Com_PrintError(19, "ERROR: xmodel '%s' has more than %d bones\n", name, 127);
        return 0;
    }
}

XModel *__cdecl XModelLoad(char *name, void *(__cdecl *Alloc)(int), void *(__cdecl *AllocColl)(int))
{
    XModel *model; // [esp+0h] [ebp-4h]

    model = XModelLoadFile(name, Alloc, AllocColl);
    if (model)
        return model;
    else
        return 0;
}

static XModelPartsLoad *__cdecl XModelCreateDefaultParts()
{
    g_default.modelParts.parentList = g_default.parentList;
    g_default.modelParts.boneNames = (unsigned __int16 *)&g_default;
    g_default.modelParts.quats = 0;
    g_default.modelParts.trans = 0;
    g_default.modelParts.numBones = 1;
    g_default.modelParts.numRootBones = 1;
    g_default.modelParts.partClassification = g_default.partClassification;
    g_default.partClassification[0] = 0;
    g_default.boneNames[0] = 0;
    return &g_default.modelParts;
}

static void __cdecl XModelMakeDefault(XModel *model)
{
    const XModelPartsLoad *DefaultParts; // eax

    model->bad = 1;
    DefaultParts = XModelCreateDefaultParts();
    XModelCopyXModelParts(DefaultParts, model);
    memset((unsigned __int8 *)model->lodInfo, 0, sizeof(model->lodInfo));
    model->numLods = 1;
    model->collLod = 0;
    model->name = "DEFAULT";
    model->surfs = 0;
    model->materialHandles = g_materials;
    g_materials[0] = Material_RegisterHandle("mc/$default", 8);
    g_default.boneInfo.bounds[0][0] = -16.0;
    g_default.boneInfo.bounds[0][1] = -16.0;
    g_default.boneInfo.bounds[0][2] = -16.0;
    g_default.boneInfo.bounds[1][0] = 16.0;
    g_default.boneInfo.bounds[1][1] = 16.0;
    g_default.boneInfo.bounds[1][2] = 16.0;
    model->boneInfo = &g_default.boneInfo;
}

static XModel *__cdecl XModelCreateDefault(void *(__cdecl *Alloc)(int))
{
    XModel *model; // [esp+0h] [ebp-4h]

    model = (XModel *)Alloc(332);
    XModelMakeDefault(model);
    return model;
}

static XModel *__cdecl XModelDefaultModel(const char *name, void *(__cdecl *Alloc)(int))
{
    XModel *model; // [esp+0h] [ebp-4h]

    model = XModelCreateDefault(Alloc);
    Hunk_SetDataForFile(5, name, model, Alloc);
    return model;
}

XModel *__cdecl XModelPrecache_LoadObj(char *name, void *(__cdecl *Alloc)(int), void *(__cdecl *AllocColl)(int))
{
    XModel *model; // [esp+0h] [ebp-4h]
    XModel *modela; // [esp+0h] [ebp-4h]

    model = (XModel *)Hunk_FindDataForFile(5, name);
    if (model)
        return model;
    ProfLoad_Begin("Load xmodel");
    modela = XModelLoad(name, Alloc, AllocColl);
    ProfLoad_End();
    if (modela)
    {
        modela->name = Hunk_SetDataForFile(5, name, modela, Alloc);
        return modela;
    }
    else
    {
        Com_PrintError(19, "ERROR: Cannot find xmodel '%s'.\n", name);
        return XModelDefaultModel(name, Alloc);
    }
}