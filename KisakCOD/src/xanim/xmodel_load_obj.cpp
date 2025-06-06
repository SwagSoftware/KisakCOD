#include "xmodel.h"
#include <universal/com_math.h>
#include <universal/com_memory.h>
#include <universal/com_files.h>
#include <qcommon/qcommon.h>
#include <qcommon/com_pack.h>
#include <universal/aabbtree.h>
#include <gfx_d3d/r_dvars.h>
#include "xanim.h"

int __cdecl XModelGetStaticBounds(const XModel *model, mat3x3& axis, float *mins, float *maxs)
{
    float v5; // [esp+0h] [ebp-34h]
    float v6; // [esp+4h] [ebp-30h]
    float v7; // [esp+8h] [ebp-2Ch]
    int j; // [esp+Ch] [ebp-28h]
    int k; // [esp+10h] [ebp-24h]
    const XModelCollSurf_s *csurf; // [esp+14h] [ebp-20h]
    float rotated[3]; // [esp+18h] [ebp-1Ch] BYREF
    int i; // [esp+24h] [ebp-10h]
    float corner[3]; // [esp+28h] [ebp-Ch] BYREF

    if (model->numCollSurfs)
    {
        
        mins[0] = FLT_MAX;
        mins[1] = FLT_MAX;
        mins[2] = FLT_MAX;

        maxs[1] = -FLT_MAX;
        maxs[2] = -FLT_MAX;
        maxs[0] = -FLT_MAX;

        for (i = 0; i < model->numCollSurfs; ++i)
        {
            csurf = &model->collSurfs[i];
            for (k = 0; k < 8; ++k)
            {
                if ((k & 1) != 0)
                    v7 = csurf->mins[0];
                else
                    v7 = csurf->maxs[0];
                corner[0] = v7;

                if ((k & 2) != 0)
                    v6 = csurf->mins[1];
                else
                    v6 = csurf->maxs[1];
                corner[1] = v6;

                if ((k & 4) != 0)
                    v5 = csurf->mins[2];
                else
                    v5 = csurf->maxs[2];
                corner[2] = v5;

                MatrixTransformVector(corner, axis, rotated);
                for (j = 0; j < 3; ++j)
                {
                    if (rotated[j] < (double)mins[j])
                        mins[j] = rotated[j];
                    if (rotated[j] > (double)maxs[j])
                        maxs[j] = rotated[j];
                }
            }
        }
        return 1;
    }
    else
    {
        iassert(!model->contents);
        return 0;
    }
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

    if (surface->deformed)
        MyAssertHandler(".\\r_xsurface_load_obj.cpp", 222, 0, "%s", "!surface->deformed");
    if (vertListIndex >= surface->vertListCount)
        MyAssertHandler(
            ".\\r_xsurface_load_obj.cpp",
            223,
            0,
            "%s\n\t(vertListIndex) = %i",
            "(vertListIndex >= 0 && vertListIndex < surface->vertListCount)",
            vertListIndex);
    vertList = &surface->vertList[vertListIndex];
    tree = (XSurfaceCollisionTree*)Alloc(40);
    vertList->collisionTree = tree;
    if (!surface->triCount)
        MyAssertHandler(".\\r_xsurface_load_obj.cpp", 228, 0, "%s", "surface->triCount > 0");
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

void __cdecl XSurfaceTransfer(
    const XVertexBuffer *surfVerts,
    GfxPackedVertex *verts0,
    GfxPackedVertex *verts1,
    int vertCount)
{
    float v4; // [esp+8h] [ebp-54h]
    float v5; // [esp+Ch] [ebp-50h]
    float v6; // [esp+10h] [ebp-4Ch]
    GfxPackedVertex *v7; // [esp+14h] [ebp-48h]
    const float *offset; // [esp+18h] [ebp-44h]
    float v9; // [esp+1Ch] [ebp-40h]
    float v10; // [esp+20h] [ebp-3Ch]
    float v11; // [esp+24h] [ebp-38h]
    PackedUnitVec v12; // [esp+28h] [ebp-34h]
    PackedTexCoords v13; // [esp+2Ch] [ebp-30h]
    PackedUnitVec v14; // [esp+30h] [ebp-2Ch]
    float texCoordAv; // [esp+34h] [ebp-28h]
    float texCoordAva; // [esp+34h] [ebp-28h]
    float texCoordAvb; // [esp+34h] [ebp-28h]
    float texCoordAv_4; // [esp+38h] [ebp-24h]
    float texCoordAv_4a; // [esp+38h] [ebp-24h]
    float texCoordAv_4b; // [esp+38h] [ebp-24h]
    float binormal[3]; // [esp+3Ch] [ebp-20h] BYREF
    float texCoord[2]; // [esp+48h] [ebp-14h] BYREF
    bool texCoordUnitRange[4]; // [esp+50h] [ebp-Ch]
    int vertIndex; // [esp+54h] [ebp-8h]
    const XVertexInfo_s *v; // [esp+58h] [ebp-4h]

    if (!vertCount)
        MyAssertHandler(".\\r_xsurface.cpp", 116, 0, "%s", "vertCount");
    texCoordAv = 0.0;
    texCoordAv_4 = 0.0;
    texCoordUnitRange[0] = 1;
    texCoordUnitRange[1] = 1;
    v = &surfVerts->v;
    for (vertIndex = 0; vertIndex < vertCount; ++vertIndex)
    {
        texCoordAv = texCoordAv + v->texCoordX;
        texCoordAv_4 = texCoordAv_4 + v->texCoordY;
        if (v->texCoordX < 0.0 || v->texCoordX > 1.0)
            texCoordUnitRange[0] = 0;
        if (v->texCoordY < 0.0 || v->texCoordY > 1.0)
            texCoordUnitRange[1] = 0;
    }
    v11 = 1.0 / vertCount;
    texCoordAva = v11 * texCoordAv;
    texCoordAv_4a = v11 * texCoordAv_4;
    if (texCoordUnitRange[0])
    {
        texCoordAvb = 0.0;
    }
    else
    {
        v10 = texCoordAva + 0.5;
        v6 = floor(v10);
        texCoordAvb = v6;
    }
    if (texCoordUnitRange[1])
    {
        texCoordAv_4b = 0.0;
    }
    else
    {
        v9 = texCoordAv_4a + 0.5;
        v5 = floor(v9);
        texCoordAv_4b = v5;
    }
    v = &surfVerts->v;
    for (vertIndex = 0; vertIndex < vertCount; ++vertIndex)
    {
        v7 = &verts0[vertIndex];
        offset = v->offset;
        v7->xyz[0] = offset[0];
        v7->xyz[1] = offset[1];
        v7->xyz[2] = offset[2];
        Vec3Cross(v->normal, v->tangent, binormal);
        if (Vec3Dot(binormal, v->binormal) < 0.0)
            v4 = -1.0;
        else
            v4 = 1.0;
        verts0[vertIndex].binormalSign = v4;
        v14.packed = Vec3PackUnitVec(v->normal).packed;
        verts1[vertIndex].normal = v14;
        verts1[vertIndex].color.packed = *v->color;
        texCoord[0] = v->texCoordX - texCoordAvb;
        texCoord[1] = v->texCoordY - texCoordAv_4b;
        v13.packed = Vec2PackTexCoords(texCoord).packed;
        verts1[vertIndex].texCoord = v13;
        v12.packed = Vec3PackUnitVec(v->tangent).packed;
        verts1[vertIndex].tangent = v12;
        v = (const XVertexInfo_s *)((char *)v + 4 * v->numWeights + 64);
    }
}

void __cdecl ReadBlend(XSurface *surface, int *partBits, XBlendLoadInfo *blend, unsigned __int16 **pos)
{
    unsigned __int16 v4; // [esp+0h] [ebp-10h]
    __int16 v5; // [esp+4h] [ebp-Ch]

    v5 = *(*pos)++;
    partBits[v5 >> 5] |= 0x80000000 >> (v5 & 0x1F);
    blend->boneOffset = v5 << 6;
    if (v5 << 6 != blend->boneOffset)
        MyAssertHandler(".\\r_xsurface_load_obj.cpp", 67, 0, "%s", "boneOffset == blend->boneOffset");
    v4 = *(*pos)++;
    blend->boneWeight = v4;
}

void __cdecl XModelReadSurface(XModel *model, unsigned __int16 **pos, void *(__cdecl *Alloc)(int), XSurface *surface)
{
    int vertCount; // edx
    unsigned __int16 *v5; // [esp+Ch] [ebp-734h]
    unsigned __int8 v6; // [esp+10h] [ebp-730h]
    XRigidVertList *v7; // [esp+14h] [ebp-72Ch]
    unsigned __int16 v8; // [esp+18h] [ebp-728h]
    float v9; // [esp+1Ch] [ebp-724h]
    float v10; // [esp+20h] [ebp-720h]
    float v11; // [esp+24h] [ebp-71Ch]
    float v12; // [esp+28h] [ebp-718h]
    float v13; // [esp+2Ch] [ebp-714h]
    float v14; // [esp+30h] [ebp-710h]
    __int16 v15; // [esp+34h] [ebp-70Ch]
    float v16; // [esp+38h] [ebp-708h]
    float v17; // [esp+3Ch] [ebp-704h]
    float v18; // [esp+40h] [ebp-700h]
    float v19; // [esp+44h] [ebp-6FCh]
    float v20; // [esp+48h] [ebp-6F8h]
    float v21; // [esp+4Ch] [ebp-6F4h]
    float v22; // [esp+50h] [ebp-6F0h]
    float v23; // [esp+54h] [ebp-6ECh]
    float v24; // [esp+64h] [ebp-6DCh]
    float v25; // [esp+68h] [ebp-6D8h]
    float v26; // [esp+6Ch] [ebp-6D4h]
    __int16 v27; // [esp+70h] [ebp-6D0h]
    __int16 v28; // [esp+74h] [ebp-6CCh]
    unsigned __int16 v29; // [esp+78h] [ebp-6C8h]
    unsigned __int16 v30; // [esp+7Ch] [ebp-6C4h]
    unsigned __int16 v31; // [esp+80h] [ebp-6C0h]
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
    XVertexInfo2 *verts2; // [esp+DCh] [ebp-664h]
    XVertexInfo3 *verts3; // [esp+E0h] [ebp-660h]
    int startTriIndex; // [esp+E4h] [ebp-65Ch]
    unsigned int vertListIter; // [esp+E8h] [ebp-658h]
    int localBoneIndex; // [esp+ECh] [ebp-654h]
    XVertexInfo1 *verts1; // [esp+F0h] [ebp-650h]
    int vertListCount; // [esp+F4h] [ebp-64Ch]
    XRigidVertList rigidVertListArray[129]; // [esp+F8h] [ebp-648h] BYREF
    XVertexInfo0 *verts0; // [esp+708h] [ebp-38h]
    GfxPackedVertex *surfaceVerts1; // [esp+70Ch] [ebp-34h]
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
    surface->tileMode = *(_BYTE *)*pos;
    *pos = (unsigned __int16 *)((char *)*pos + 1);
    v31 = *++*pos;
    ++*pos;
    surface->vertCount = v31;
    v30 = *(*pos)++;
    surface->triCount = v30;
    if (!surface->triCount)
        MyAssertHandler(
            ".\\r_xsurface_load_obj.cpp",
            455,
            0,
            "%s\n\t(surface->triCount) = %i",
            "(surface->triCount > 0)",
            surface->triCount);
    vertListCount = 0;
    rigidVertCount = 0;
    while (1)
    {
        if (vertListCount >= 0x81)
            MyAssertHandler(".\\r_xsurface_load_obj.cpp", 462, 0, "%s", "vertListCount < ARRAY_COUNT( rigidVertListArray )");
        rigidVertList = &rigidVertListArray[vertListCount];
        v29 = *(*pos)++;
        rigidVertList->vertCount = v29;
        if (!rigidVertList->vertCount)
            break;
        v28 = *(*pos)++;
        localBoneIndex = v28;
        rigidVertList->boneOffset = v28 << 6;
        rigidVertCount += rigidVertList->vertCount;
        ++vertListCount;
    }
    vertCount = surface->vertCount;
    deformed = rigidVertCount != vertCount;
    if (rigidVertCount != vertCount)
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
        v27 = *(*pos)++;
        numblends = v27;
    }
    size = (surface->vertCount << 6) + 4 * numblends;
    surfVerts = (XVertexBuffer*)Hunk_AllocateTempMemory(size, "XModelReadSurface");
    verts = &surfVerts->v;
    for (j = 0; j < surface->vertCount; ++j)
    {
        v26 = *(float *)*pos;
        *pos += 2;
        verts->normal[0] = v26;
        v25 = *(float *)*pos;
        *pos += 2;
        verts->normal[1] = v25;
        v24 = *(float *)*pos;
        *pos += 2;
        verts->normal[2] = v24;
        if (r_modelVertColor->current.enabled)
            Byte4CopyBgraToVertexColor((const unsigned __int8 *)*pos, verts->color);
        else
            *(_DWORD *)verts->color = -1;
        *pos += 2;
        v23 = *(float *)*pos;
        *pos += 2;
        verts->texCoordX = v23;
        v22 = *(float *)*pos;
        *pos += 2;
        verts->texCoordY = v22;
        v21 = *(float *)*pos;
        *pos += 2;
        verts->binormal[0] = v21;
        v20 = *(float *)*pos;
        *pos += 2;
        verts->binormal[1] = v20;
        v19 = *(float *)*pos;
        *pos += 2;
        verts->binormal[2] = v19;
        v18 = *(float *)*pos;
        *pos += 2;
        verts->tangent[0] = v18;
        v17 = *(float *)*pos;
        *pos += 2;
        verts->tangent[1] = v17;
        v16 = *(float *)*pos;
        *pos += 2;
        verts->tangent[2] = v16;
        check[0] = Vec3Dot(verts->normal, verts->tangent);
        check[1] = Vec3Dot(verts->tangent, verts->binormal);
        check[2] = Vec3Dot(verts->binormal, verts->normal);
        if (!VecNCompareCustomEpsilon(check, vec3_origin, 0.0020000001, 3))
            MyAssertHandler(
                ".\\r_xsurface_load_obj.cpp",
                533,
                0,
                "%s",
                "VecNCompareCustomEpsilon( check, vec3_origin, EQUAL_EPSILON * 2, 3 )");
        if (vertListCount == 1)
        {
            if (deformed)
                MyAssertHandler(".\\r_xsurface_load_obj.cpp", 571, 0, "%s", "!deformed");
            verts->numWeights = 0;
            verts->boneOffset = rigidVertListArray[0].boneOffset;
            v11 = *(float *)*pos;
            *pos += 2;
            verts->offset[0] = v11;
            v10 = *(float *)*pos;
            *pos += 2;
            verts->offset[1] = v10;
            v9 = *(float *)*pos;
            *pos += 2;
            verts->offset[2] = v9;
            ++verts;
        }
        else
        {
            numWeights = *(_BYTE *)*pos;
            *pos = (unsigned __int16 *)((char *)*pos + 1);
            verts->numWeights = numWeights;
            if (numWeights >= 4u)
                MyAssertHandler(".\\r_xsurface_load_obj.cpp", 543, 0, "%s\n\t(numWeights) = %i", "(numWeights < 4)", numWeights);
            ++weightCount[numWeights];
            v15 = *(*pos)++;
            blendBoneIndex = v15;
            surface->partBits[v15 >> 5] |= 0x80000000 >> (v15 & 0x1F);
            blendBoneOffset = blendBoneIndex << 6;
            verts->boneOffset = (_WORD)blendBoneIndex << 6;
            if (blendBoneOffset != verts->boneOffset)
                MyAssertHandler(".\\r_xsurface_load_obj.cpp", 550, 0, "%s", "blendBoneOffset == verts->boneOffset");
            v14 = *(float *)*pos;
            *pos += 2;
            verts->offset[0] = v14;
            v13 = *(float *)*pos;
            *pos += 2;
            verts->offset[1] = v13;
            v12 = *(float *)*pos;
            *pos += 2;
            verts->offset[2] = v12;
            ++verts;
            if (numWeights)
            {
                if (!deformed)
                    MyAssertHandler(".\\r_xsurface_load_obj.cpp", 560, 0, "%s", "deformed");
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
    if (!surface->triIndices)
        MyAssertHandler(".\\r_xsurface_load_obj.cpp", 599, 0, "%s", "surface->triIndices");
    for (vertIndex = 0; vertIndex < 3 * surface->triCount; ++vertIndex)
    {
        v8 = *(*pos)++;
        surface->triIndices[vertIndex] = v8;
        if (surface->triIndices[vertIndex] >= surface->vertCount)
            MyAssertHandler(".\\r_xsurface_load_obj.cpp", 604, 0, "%s", "surface->triIndices[vertIndex] < surface->vertCount");
    }
    triIndex = 0;
    endIndex = 0;
    for (j = 0; j < vertListCount; ++j)
    {
        startTriIndex = triIndex;
        rigidVertListArray[j].triOffset = triIndex;
        if (rigidVertListArray[j].triOffset != startTriIndex)
            MyAssertHandler(".\\r_xsurface_load_obj.cpp", 613, 0, "%s", "rigidVertListArray[j].triOffset == startTriIndex");
        endIndex += rigidVertListArray[j].vertCount;
        while (triIndex < surface->triCount && surface->triIndices[3 * triIndex] < endIndex)
            ++triIndex;
        rigidVertListArray[j].triCount = triIndex - startTriIndex;
        if (rigidVertListArray[j].triCount != triIndex - startTriIndex)
            MyAssertHandler(
                ".\\r_xsurface_load_obj.cpp",
                618,
                0,
                "%s",
                "rigidVertListArray[j].triCount == triIndex - startTriIndex");
    }
    if (allocCount != surface->triCount)
    {
        if (allocCount != surface->triCount + 1)
            MyAssertHandler(".\\r_xsurface_load_obj.cpp", 623, 1, "%s", "allocCount == surface->triCount + 1");
        surface->triIndices[vertIndex] = surface->triIndices[vertIndex - 1];
        surface->triIndices[vertIndex + 1] = surface->triIndices[vertIndex - 1];
        surface->triIndices[vertIndex + 2] = surface->triIndices[vertIndex - 1];
        ++surface->triCount;
    }
    if (vertListCount)
        v7 = (XRigidVertList*)Alloc(12 * vertListCount);
    else
        v7 = 0;
    surface->vertList = v7;
    surface->vertListCount = vertListCount;
    memcpy(surface->vertList, rigidVertListArray, 12 * vertListCount);
    vertexBytes = 32 * surface->vertCount;
    surface->verts0 = (GfxPackedVertex*)Alloc(vertexBytes);
    model->memUsage += vertexBytes;
    surfaceVerts1 = surface->verts0;
    XSurfaceTransfer(surfVerts, surface->verts0, surfaceVerts1, surface->vertCount);
    if (deformed)
    {
        if (XModelNumBones(model) <= 1)
            MyAssertHandler(".\\r_xsurface_load_obj.cpp", 659, 0, "%s", "XModelNumBones( model ) > 1");
        verts0 = 0;
        verts1 = 0;
        verts2 = 0;
        verts3 = 0;
        if (weightCount[0])
        {
            size = 2 * weightCount[0];
            verts0 = (XVertexInfo0*)Alloc(2 * weightCount[0]);
        }
        if (weightCount[1])
        {
            size = 6 * weightCount[1];
            verts1 = (XVertexInfo1*)Alloc(6 * weightCount[1]);
        }
        if (weightCount[2])
        {
            size = 10 * weightCount[2];
            verts2 = (XVertexInfo2*)Alloc(10 * weightCount[2]);
        }
        if (weightCount[3])
        {
            size = 14 * weightCount[3];
            verts3 = (XVertexInfo3*)Alloc(14 * weightCount[3]);
        }
        for (i = 0; i < 4; ++i)
        {
            surface->vertInfo.vertCount[i] = weightCount[i];
            if (surface->vertInfo.vertCount[i] != weightCount[i])
                MyAssertHandler(".\\r_xsurface_load_obj.cpp", 693, 0, "%s", "surface->vertInfo.vertCount[i] == weightCount[i]");
        }
        verts = &surfVerts->v;
        vert0Out = verts0;
        vert1Out = verts1;
        vert2Out = verts2;
        vert3Out = verts3;
        for (j = 0; j < surface->vertCount; ++j)
        {
            v6 = verts->numWeights;
            if (v6)
            {
                if (v6 == 1)
                {
                    if (!vert1Out)
                        MyAssertHandler(".\\r_xsurface_load_obj.cpp", 714, 0, "%s", "vert1Out");
                    vertOut = &vert1Out->vert0;
                    blendOut = vert1Out->blend;
                    ++vert1Out;
                }
                else if (v6 == 2)
                {
                    if (!vert2Out)
                        MyAssertHandler(".\\r_xsurface_load_obj.cpp", 721, 0, "%s", "vert2Out");
                    vertOut = &vert2Out->vert0;
                    blendOut = vert2Out->blend;
                    ++vert2Out;
                }
                else
                {
                    if (verts->numWeights != 3)
                        MyAssertHandler(
                            ".\\r_xsurface_load_obj.cpp",
                            728,
                            0,
                            "%s\n\t(verts->numWeights) = %i",
                            "(verts->numWeights == 3)",
                            verts->numWeights);
                    if (!vert3Out)
                        MyAssertHandler(".\\r_xsurface_load_obj.cpp", 729, 0, "%s", "vert3Out");
                    vertOut = &vert3Out->vert0;
                    blendOut = vert3Out->blend;
                    ++vert3Out;
                }
            }
            else
            {
                if (!vert0Out)
                    MyAssertHandler(".\\r_xsurface_load_obj.cpp", 707, 0, "%s", "vert0Out");
                vertOut = vert0Out;
                blendOut = 0;
                ++vert0Out;
            }
            vertOut->boneOffset = verts->boneOffset;
            numWeights = verts->numWeights;
            ++verts;
            if (numWeights)
            {
                if (!blendOut)
                    MyAssertHandler(".\\r_xsurface_load_obj.cpp", 743, 0, "%s", "blendOut");
                for (i = 0; i < numWeights; ++i)
                {
                    blendOut->boneOffset = LOWORD(verts->normal[0]);
                    blendOut->boneWeight = HIWORD(verts->normal[0]);
                    ++blendOut;
                    verts = (XVertexInfo_s *)((char *)verts + 4);
                }
            }
        }
        if (vert0Out != &verts0[surface->vertInfo.vertCount[0]])
            MyAssertHandler(".\\r_xsurface_load_obj.cpp", 755, 0, "%s", "vert0Out == verts0 + surface->vertInfo.vertCount[0]");
        if (vert1Out != &verts1[surface->vertInfo.vertCount[1]])
            MyAssertHandler(".\\r_xsurface_load_obj.cpp", 756, 0, "%s", "vert1Out == verts1 + surface->vertInfo.vertCount[1]");
        if (vert2Out != &verts2[surface->vertInfo.vertCount[2]])
            MyAssertHandler(".\\r_xsurface_load_obj.cpp", 757, 0, "%s", "vert2Out == verts2 + surface->vertInfo.vertCount[2]");
        if (vert3Out != &verts3[surface->vertInfo.vertCount[3]])
            MyAssertHandler(".\\r_xsurface_load_obj.cpp", 758, 0, "%s", "vert3Out == verts3 + surface->vertInfo.vertCount[3]");
        size = 2
            * (7 * surface->vertInfo.vertCount[3]
                + 5 * surface->vertInfo.vertCount[2]
                + 3 * surface->vertInfo.vertCount[1]
                + surface->vertInfo.vertCount[0]);
        if (size)
            v5 = (unsigned short*)Alloc(size);
        else
            v5 = 0;
        vertsBlendOut = v5;
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
        if ((char *)vertsBlendOut - (char *)surface->vertInfo.vertsBlend != size)
            MyAssertHandler(
                ".\\r_xsurface_load_obj.cpp",
                845,
                0,
                "%s",
                "(byte *)vertsBlendOut - (byte *)(surface->vertInfo.vertsBlend) == size");
    }
    if (surface->deformed != (surface->vertListCount == 0))
        MyAssertHandler(".\\r_xsurface_load_obj.cpp", 848, 0, "%s", "surface->deformed == (surface->vertListCount == 0)");
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
    unsigned __int16 **pos,
    void *(__cdecl *AllocMesh)(int))
{
    int j; // [esp+4h] [ebp-18h]
    int surfIndex; // [esp+8h] [ebp-14h]
    int baseVertIndex; // [esp+Ch] [ebp-10h]
    XSurface *xsurf; // [esp+10h] [ebp-Ch]
    int baseTriIndex; // [esp+14h] [ebp-8h]
    XSurface *surfs; // [esp+18h] [ebp-4h]

    surfs = modelSurfs->surfs;
    if (!modelSurfs->surfs)
        MyAssertHandler(".\\xanim\\xmodel_load_obj.cpp", 720, 0, "%s", "surfs");
    if (!modelPartBits)
        MyAssertHandler(".\\xanim\\xmodel_load_obj.cpp", 721, 0, "%s", "modelPartBits");
    if (surfCount <= 0)
        MyAssertHandler(".\\xanim\\xmodel_load_obj.cpp", 722, 0, "%s", "surfCount > 0");
    if (!pos)
        MyAssertHandler(".\\xanim\\xmodel_load_obj.cpp", 723, 0, "%s", "pos");
    if (!*pos)
        MyAssertHandler(".\\xanim\\xmodel_load_obj.cpp", 724, 0, "%s", "*pos");
    baseTriIndex = 0;
    baseVertIndex = 0;
    for (surfIndex = 0; surfIndex < surfCount; ++surfIndex)
    {
        xsurf = &surfs[surfIndex];
        XModelReadSurface(model, pos, AllocMesh, xsurf);
        for (j = 0; j < 4; ++j)
            modelPartBits[j] |= xsurf->partBits[j];
        if (baseTriIndex != baseTriIndex)
            MyAssertHandler(
                "c:\\trees\\cod3\\src\\qcommon\\../universal/assertive.h",
                281,
                0,
                "i == static_cast< Type >( i )\n\t%i, %i",
                baseTriIndex,
                baseTriIndex);
        xsurf->baseTriIndex = baseTriIndex;
        if (baseVertIndex != baseVertIndex)
            MyAssertHandler(
                "c:\\trees\\cod3\\src\\qcommon\\../universal/assertive.h",
                281,
                0,
                "i == static_cast< Type >( i )\n\t%i, %i",
                baseVertIndex,
                baseVertIndex);
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
    __int16 v6; // [esp+0h] [ebp-6Ch]
    __int16 v7; // [esp+4h] [ebp-68h]
    const unsigned __int8 *pos; // [esp+8h] [ebp-64h] BYREF
    char filename[68]; // [esp+Ch] [ebp-60h] BYREF
    unsigned __int8 *buf; // [esp+54h] [ebp-18h] BYREF
    __int16 version; // [esp+58h] [ebp-14h]
    XModelSurfs *modelSurfs; // [esp+5Ch] [ebp-10h]
    int size; // [esp+60h] [ebp-Ch]
    int fileSize; // [esp+64h] [ebp-8h]
    __int16 numsurfs; // [esp+68h] [ebp-4h]

    if (Com_sprintf(filename, 0x40u, "xmodelsurfs/%s", name) >= 0)
    {
        fileSize = FS_ReadFile(filename, (void**)&buf);
        if (fileSize >= 0)
        {
            if (fileSize)
            {
                if (!buf)
                    MyAssertHandler(".\\xanim\\xmodel_load_obj.cpp", 847, 0, "%s", "buf");
                pos = buf;
                v7 = *(_WORD *)buf;
                pos = buf + 2;
                version = v7;
                if (v7 == 25)
                {
                    v6 = *(_WORD *)pos;
                    pos += 2;
                    numsurfs = v6;
                    if (v6 == modelNumsurfs)
                    {
                        size = 56 * modelNumsurfs + 20;
                        modelSurfs = (XModelSurfs*)Alloc(size);
                        model->memUsage += size;
                        modelSurfs->surfs = modelSurfs[1].surfs;
                        XModelReadSurfaces(model, name, modelSurfs, modelSurfs->partBits, modelNumsurfs, (unsigned short**)&pos, Alloc);
                        FS_FreeFile((char*)buf);
                        if (!modelSurfs)
                            MyAssertHandler(".\\xanim\\xmodel_load_obj.cpp", 876, 0, "%s", "modelSurfs");
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
            else
            {
                Com_PrintError(19, "ERROR: xmodelsurf '%s' has 0 length\n", name);
                FS_FreeFile((char*)buf);
                return 0;
            }
        }
        else
        {
            if (buf)
                MyAssertHandler(".\\xanim\\xmodel_load_obj.cpp", 835, 0, "%s", "!buf");
            Com_PrintError(19, "ERROR: xmodelsurf '%s' not found\n", name);
            return 0;
        }
    }
    else
    {
        Com_PrintError(19, "ERROR: filename '%s' too long\n", filename);
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

