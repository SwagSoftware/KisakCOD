#include "dobj.h"
#include "xmodel.h"
#include <qcommon/qcommon.h>
#include <script/scr_memorytree.h>
#include <physics/phys_local.h>
#include "dobj_utils.h"
#include <universal/profile.h>

struct SavedDObjModel // sizeof=0x2
{                                       // ...
    unsigned __int16 boneName;
};

struct SavedDObj // sizeof=0x60
{                                       // ...
    SavedDObjModel dobjModels[32];
    XModel **models;                    // ...
    unsigned int ignoreCollision;       // ...
    unsigned __int16 numModels;         // ...
    unsigned __int16 entnum;            // ...
    XAnimTree_s *tree;                  // ...
    unsigned int hidePartBits[4];       // ...
};

unsigned int g_empty;

void __cdecl DObjInit()
{
    int duplicatePartBits[5]; // [esp+0h] [ebp-14h] BYREF

    memset(duplicatePartBits, 0, sizeof(duplicatePartBits));
    g_empty = SL_GetStringOfSize((char *)duplicatePartBits, 0, 0x11u, 12).prev;
}

void __cdecl DObjShutdown()
{
    if (g_empty)
    {
        SL_RemoveRefToStringOfSize(g_empty, 0x11u);
        g_empty = 0;
    }
}

void __cdecl DObjDumpInfo(const DObj_s *obj)
{
    char *BoneName; // eax
    char *v2; // eax
    int v3; // [esp-8h] [ebp-28h]
    char *v4; // [esp-4h] [ebp-24h]
    int j; // [esp+0h] [ebp-20h]
    int numBones; // [esp+4h] [ebp-1Ch]
    const unsigned __int8 *pos; // [esp+8h] [ebp-18h]
    int boneIndex; // [esp+Ch] [ebp-14h]
    int numModels; // [esp+10h] [ebp-10h]
    XModel *model; // [esp+14h] [ebp-Ch]
    int i; // [esp+18h] [ebp-8h]
    XModel **models; // [esp+1Ch] [ebp-4h]

    if (obj)
    {
        Com_Printf(19, "\nModels:\n");
        numModels = obj->numModels;
        boneIndex = 0;
        models = obj->models;
        for (j = 0; j < numModels; ++j)
        {
            model = models[j];
            Com_Printf(19, "%d: '%s'\n", boneIndex, model->name);
            boneIndex += XModelNumBones(model);
        }
        Com_Printf(19, "\nBones:\n");
        numBones = obj->numBones;
        for (i = 0; i < numBones; ++i)
        {
            BoneName = DObjGetBoneName(obj, i);
            Com_Printf(19, "Bone %d: '%s'\n", i, BoneName);
        }
        if (obj->duplicateParts)
        {
            Com_Printf(19, "\nPart duplicates:\n");
            for (pos = (const unsigned __int8 *)(SL_ConvertToString(obj->duplicateParts) + 16); *pos; pos += 2)
            {
                v4 = DObjGetBoneName(obj, pos[1] - 1);
                v3 = pos[1] - 1;
                v2 = DObjGetBoneName(obj, *pos - 1);
                Com_Printf(19, "%d ('%s') -> %d ('%s')\n", *pos - 1, v2, v3, v4);
            }
        }
        else
        {
            Com_Printf(19, "\nNo part duplicates.\n");
        }
        Com_Printf(19, "\n");
    }
    else
    {
        Com_Printf(19, "No Dobj\n");
    }
}

bool __cdecl DObjIgnoreCollision(const DObj_s *obj, char modelIndex)
{
    return (obj->ignoreCollision & (1 << modelIndex)) != 0;
}

void __cdecl DObjGetHierarchyBits(const DObj_s *obj, int boneIndex, int *partBits)
{
    int j; // [esp+4Ch] [ebp-B8h]
    const unsigned __int8 *pos; // [esp+50h] [ebp-B4h]
    int newBoneIndex; // [esp+54h] [ebp-B0h]
    int newBoneIndexa = 0; // [esp+54h] [ebp-B0h]
    const unsigned __int8 *modelParents; // [esp+58h] [ebp-ACh]
    const unsigned __int8 *duplicateParts; // [esp+5Ch] [ebp-A8h]
    unsigned int bit; // [esp+60h] [ebp-A4h]
    int numModels; // [esp+64h] [ebp-A0h]
    XModel *subModel; // [esp+68h] [ebp-9Ch]
    int startIndex[33]; // [esp+6Ch] [ebp-98h]
    int localBoneIndex; // [esp+F0h] [ebp-14h]
    unsigned __int8 *parentList; // [esp+F4h] [ebp-10h]
    const int *duplicatePartBits; // [esp+F8h] [ebp-Ch]
    XModel **models; // [esp+FCh] [ebp-8h]
    int highBoneIndex; // [esp+100h] [ebp-4h]

    Profile_Begin(329);
    iassert(obj);
    bcassert(boneIndex, obj->numBones);
    partBits[0] = 0;
    partBits[1] = 0;
    partBits[2] = 0;
    partBits[3] = 0;
    numModels = obj->numModels;
    iassert(numModels > 0);
    iassert(obj->duplicateParts);
    duplicatePartBits = (const int *)SL_ConvertToString(obj->duplicateParts);
    duplicateParts = (const unsigned __int8 *)(duplicatePartBits + 4);
    newBoneIndex = 0;
    subModel = 0;
    models = obj->models;
    modelParents = (const unsigned __int8 *)&models[numModels];
    for (j = 0; j < numModels; ++j)
    {
        startIndex[j] = newBoneIndex;
        subModel = models[j];
        newBoneIndex = startIndex[j] + subModel->numBones;
        if (newBoneIndex > boneIndex)
            break;
    }
    if (j != numModels)
    {
        partBits[0] = 0x80000000;
        for (parentList = subModel->parentList; ; boneIndex -= parentList[newBoneIndexa])
        {
            localBoneIndex = boneIndex - startIndex[j];
            while (1)
            {
                iassert(localBoneIndex >= 0);
                bit = 0x80000000 >> (boneIndex & 0x1F);
                highBoneIndex = boneIndex >> 5;
                partBits[boneIndex >> 5] |= bit;
                if ((bit & duplicatePartBits[highBoneIndex]) != 0)
                {
                    for (pos = duplicateParts; ; pos += 2)
                    {
                        iassert(*pos);
                        if (boneIndex == pos[0] - 1)
                            break;
                    }
                    boneIndex = pos[1] - 1;
                    goto LABEL_30;
                }
                newBoneIndexa = localBoneIndex - subModel->numRootBones;
                if (newBoneIndexa >= 0)
                    break;
                boneIndex = modelParents[j];
                if (boneIndex == 255)
                    goto END;
                do
                {
                LABEL_30:
                    j--;
                    iassert(j >= 0);
                    localBoneIndex = boneIndex - startIndex[j];
                } while (localBoneIndex < 0);

                subModel = models[j];
                parentList = subModel->parentList;
            }
        }
    }
END:
    Profile_EndInternal(0);
}

bool __cdecl DObjSkelIsBoneUpToDate(DObj_s *obj, int boneIndex)
{
    iassert(obj);

    return obj->skel.partBits.skel.testBit(boneIndex);
}

void __cdecl DObjSetTree(DObj_s *obj, XAnimTree_s *tree)
{
    obj->tree = tree;
    if (tree)
    {
        if (tree->children)
            XAnimResetAnimMap(obj, tree->children);
    }
}

void __cdecl DObjCreate(DObjModel_s *dobjModels, unsigned int numModels, XAnimTree_s *tree, DObj_s *obj, __int16 entnum)
{
    Profile_Begin(326);
    iassert(dobjModels);
    iassert(numModels > 0);
    iassert((unsigned)numModels <= DOBJ_MAX_SUBMODELS);
    iassert(obj);

    memset((unsigned __int8 *)&obj->skel, 0, sizeof(obj->skel));
    obj->duplicatePartsSize = 0;
    obj->duplicateParts = 0;
    obj->ignoreCollision = 0;
    obj->locked = 0;
    obj->entnum = entnum;
    obj->hidePartBits[0] = 0;
    obj->hidePartBits[1] = 0;
    obj->hidePartBits[2] = 0;
    obj->hidePartBits[3] = 0;
    DObjCreateDuplicateParts(obj, dobjModels, numModels);
    DObjComputeBounds(obj);
    DObjSetTree(obj, tree);

    Profile_EndInternal(0);
}

void __cdecl DObjCreateDuplicateParts(DObj_s *obj, DObjModel_s *dobjModels, unsigned int numModels)
{
    int numBones; // [esp+30h] [ebp-5ACh]
    unsigned __int8 modelParents[32]; // [esp+34h] [ebp-5A8h] BYREF
    int boneIndex; // [esp+58h] [ebp-584h]
    DObjModel_s *dobjModel; // [esp+5Ch] [ebp-580h]
    unsigned __int8 *duplicateParts; // [esp+60h] [ebp-57Ch]
    bool bRootMeld; // [esp+67h] [ebp-575h]
    int boneCount; // [esp+68h] [ebp-574h]
    XModel *model; // [esp+6Ch] [ebp-570h]
    unsigned int currNumModels; // [esp+70h] [ebp-56Ch]
    unsigned int name; // [esp+74h] [ebp-568h]
    int len; // [esp+78h] [ebp-564h]
    unsigned int size; // [esp+7Ch] [ebp-560h]
    unsigned __int8 parentIndex; // [esp+83h] [ebp-559h] BYREF
    int localBoneIndex; // [esp+84h] [ebp-558h]
    int index; // [esp+88h] [ebp-554h]
    int duplicatePartBits[273]; // [esp+8Ch] [ebp-550h] BYREF
    int matOffset[32]; // [esp+4D4h] [ebp-108h]
    XModel *models[32]; // [esp+554h] [ebp-88h] BYREF
    int modelIndex; // [esp+5D4h] [ebp-8h]
    unsigned const __int16 *boneNames; // [esp+5D8h] [ebp-4h]

    Profile_Begin(325);
    duplicateParts = (unsigned __int8 *)&duplicatePartBits[4];
    memset(duplicatePartBits, 0, 16);
    len = 0;
    boneCount = 0;
    dobjModel = dobjModels;
    currNumModels = 0;
    while (currNumModels < numModels)
    {
        boneIndex = boneCount;
        model = dobjModel->model;
        iassert(model);
        boneCount += XModelNumBones(model);
        if (boneCount > 128)
        {
            iassert(currNumModels);
            DObjDumpCreationInfo(dobjModels, numModels);
            Com_Error(ERR_DROP, "dobj for xmodel %s has more than %d bones (see console for details)", models[0]->name, 128);
        }
        models[currNumModels] = model;
        modelParents[currNumModels] = -1;
        matOffset[currNumModels] = boneIndex;
        if (dobjModel->ignoreCollision)
            obj->ignoreCollision |= 1 << currNumModels;
        if (currNumModels)
        {
            name = dobjModel->boneName;
            if (name && *SL_ConvertToString(name))
            {
                for (modelIndex = currNumModels - 1; modelIndex >= 0; --modelIndex)
                {
                    if (XModelGetBoneIndex(models[modelIndex], name, matOffset[modelIndex], &modelParents[currNumModels]))
                        goto LABEL_2;
                }
                iassert(currNumModels);
                Com_PrintWarning(19, "WARNING: Part '%s' not found in model '%s' or any of its descendants\n", SL_ConvertToString(name), models[0]->name);
                iassert(modelParents[currNumModels] == NO_BONEINDEX);
            }
            else
            {
                numBones = model->numBones;
                boneNames = model->boneNames;
                bRootMeld = 0;
                for (localBoneIndex = 0; localBoneIndex < numBones; ++localBoneIndex)
                {
                    name = boneNames[localBoneIndex];
                    for (modelIndex = currNumModels - 1; modelIndex >= 0; --modelIndex)
                    {
                        if (XModelGetBoneIndex(models[modelIndex], name, matOffset[modelIndex], &parentIndex))
                        {
                            iassert(parentIndex != 255);
                            iassert(parentIndex != 254);
                            iassert(parentIndex != boneIndex + localBoneIndex);
                            if (!localBoneIndex)
                                bRootMeld = 1;
                            iassert(boneIndex + localBoneIndex + 1 < 256);
                            iassert(parentIndex + 1 < 256);
                            iassert(parentIndex < boneIndex + localBoneIndex);
                            index = localBoneIndex + boneIndex;
                            duplicateParts[len] = localBoneIndex + boneIndex + 1;
                            duplicatePartBits[index >> 5] |= 0x80000000 >> (index & 0x1F);
                            iassert(duplicateParts[len]);
                            duplicateParts[++len] = parentIndex + 1;
                            iassert(duplicateParts[len]);
                            ++len;
                            break;
                        }
                    }
                }
                if (!bRootMeld)
                {
                    iassert(currNumModels);
                    Com_PrintWarning(
                        19,
                        "WARNING: Attempting to meld model, but root part '%s' of model '%s' not found in model '%s' or any of its descendants\n",
                        SL_ConvertToString(*boneNames),
                        model->name,
                        models[0]->name);
                }
            }
        }
    LABEL_2:
        ++currNumModels;
        ++dobjModel;
    }
    iassert(numModels == (byte)numModels);
    obj->numModels = numModels;
    iassert(boneCount == (byte)boneCount);
    obj->numBones = boneCount;
    iassert(numModels > 0);
    obj->models = (XModel **)MT_Alloc(5 * numModels, 13);
    memcpy((unsigned __int8 *)obj->models, (unsigned __int8 *)models, 4 * numModels);
    memcpy((unsigned __int8 *)&obj->models[numModels], modelParents, numModels);
    iassert(g_empty);
    iassert(!obj->duplicateParts);

    if (len)
    {
        duplicateParts[len] = 0;
        size = ++len + 16;
        obj->duplicatePartsSize = len + 16;
        iassert(obj->duplicatePartsSize == size);
        obj->duplicateParts = SL_GetStringOfSize((char *)duplicatePartBits, 0, obj->duplicatePartsSize, 12).prev;
    }
    else
    {
        obj->duplicatePartsSize = 17;
        obj->duplicateParts = g_empty;
    }
    Profile_EndInternal(0);
}

void __cdecl DObjDumpCreationInfo(DObjModel_s *dobjModels, unsigned int numModels)
{
    const char *Name; // eax
    char *v3; // eax
    unsigned int j; // [esp+0h] [ebp-14h]
    int numBones; // [esp+4h] [ebp-10h]
    unsigned int boneIndex; // [esp+8h] [ebp-Ch]
    XModel *model; // [esp+Ch] [ebp-8h]
    int i; // [esp+10h] [ebp-4h]

    boneIndex = 0;
    for (j = 0; j < numModels; ++j)
    {
        model = dobjModels[j].model;
        Name = XModelGetName(model);
        Com_Printf(19, "Model '%s':\n", Name);
        numBones = XModelNumBones(model);
        for (i = 0; i < numBones; ++i)
        {
            v3 = SL_ConvertToString(model->boneNames[i]);
            Com_Printf(19, "Bone %d: '%s'\n", boneIndex++, v3);
        }
    }
}

void __cdecl DObjComputeBounds(DObj_s *obj)
{
    int numModels; // [esp+0h] [ebp-10h]
    float radius; // [esp+4h] [ebp-Ch]
    XModel **models; // [esp+8h] [ebp-8h]
    int modelIndex; // [esp+Ch] [ebp-4h]

    if (!obj)
        MyAssertHandler(".\\xanim\\dobj.cpp", 487, 0, "%s", "obj");
    numModels = obj->numModels;
    radius = 0.0;
    models = obj->models;
    for (modelIndex = 0; modelIndex < numModels; ++modelIndex)
    {
        if (!models[modelIndex])
            MyAssertHandler(".\\xanim\\dobj.cpp", 497, 0, "%s", "models[modelIndex]");
        radius = XModelGetRadius(models[modelIndex]) + radius;
    }
    obj->radius = radius;
}

void __cdecl DObjFree(DObj_s *obj)
{
    XModel **models; // [esp+34h] [ebp-4h]

    Profile_Begin(327);
    iassert(obj);
    models = obj->models;
    if (models)
    {
        MT_Free((byte*)models, 5 * obj->numModels);
        obj->models = 0;
    }
    obj->numModels = 0; // LWSS: blops backport
    if (obj->tree)
    {
        iassert(obj->tree->anims);
        obj->tree = 0;
    }
    iassert(g_empty);
    if (obj->duplicateParts)
    {
        if (obj->duplicateParts != g_empty)
            SL_RemoveRefToStringOfSize(obj->duplicateParts, obj->duplicatePartsSize);
        obj->duplicatePartsSize = 0;
        obj->duplicateParts = 0;
    }
    Profile_EndInternal(0);
}

void __cdecl DObjGetCreateParms(
    const DObj_s *obj,
    DObjModel_s *dobjModels,
    unsigned __int16 *numModels,
    XAnimTree_s **tree,
    unsigned __int16 *entnum)
{
    const unsigned __int8 *modelParents; // [esp+0h] [ebp-A8h]
    DObjModel_s *dobjModel; // [esp+4h] [ebp-A4h]
    int boneIndex; // [esp+8h] [ebp-A0h]
    XModel *model; // [esp+Ch] [ebp-9Ch]
    int startBoneIndex; // [esp+10h] [ebp-98h]
    int parentModelIndex; // [esp+14h] [ebp-94h]
    int matOffset[33]; // [esp+18h] [ebp-90h]
    XModel **models; // [esp+9Ch] [ebp-Ch]
    int modelIndex; // [esp+A0h] [ebp-8h]
    unsigned const __int16 *boneNames; // [esp+A4h] [ebp-4h]

    if (!obj)
        MyAssertHandler(".\\xanim\\dobj.cpp", 621, 0, "%s", "obj");
    if (!obj->numModels || obj->numModels > 0x20u)
        MyAssertHandler(".\\xanim\\dobj.cpp", 622, 0, "%s", "obj->numModels > 0 && obj->numModels <= DOBJ_MAX_SUBMODELS");
    if (!dobjModels)
        MyAssertHandler(".\\xanim\\dobj.cpp", 623, 0, "%s", "dobjModels");
    if (!numModels)
        MyAssertHandler(".\\xanim\\dobj.cpp", 624, 0, "%s", "numModels");
    if (!tree)
        MyAssertHandler(".\\xanim\\dobj.cpp", 625, 0, "%s", "tree");
    *numModels = obj->numModels;
    *tree = obj->tree;
    *entnum = obj->entnum;
    startBoneIndex = 0;
    models = obj->models;
    modelParents = (const unsigned __int8 *)&models[obj->numModels];
    modelIndex = 0;
    dobjModel = dobjModels;
    while (modelIndex < obj->numModels)
    {
        model = models[modelIndex];
        matOffset[modelIndex] = startBoneIndex;
        startBoneIndex += XModelNumBones(model);
        dobjModel->model = models[modelIndex];
        dobjModel->boneName = 0;
        dobjModel->ignoreCollision = (obj->ignoreCollision & (1 << modelIndex)) != 0;
        if (modelParents[modelIndex] != 255)
        {
            for (parentModelIndex = modelIndex - 1; parentModelIndex >= 0; --parentModelIndex)
            {
                if (modelParents[modelIndex] >= matOffset[parentModelIndex])
                {
                    boneIndex = modelParents[modelIndex] - matOffset[parentModelIndex];
                    if (boneIndex >= XModelNumBones(models[parentModelIndex]))
                        MyAssertHandler(
                            ".\\xanim\\dobj.cpp",
                            653,
                            0,
                            "%s",
                            "boneIndex < XModelNumBones( models[parentModelIndex] )");
                    boneNames = models[parentModelIndex]->boneNames;
                    dobjModel->boneName = boneNames[boneIndex];
                    break;
                }
            }
        }
        ++modelIndex;
        ++dobjModel;
    }
}

void __cdecl DObjArchive(DObj_s *obj)
{
    DObjModel_s *model; // [esp+8h] [ebp-170h]
    SavedDObj savedObj; // [esp+10h] [ebp-168h] BYREF
    unsigned int modelIndex; // [esp+74h] [ebp-104h]
    DObjModel_s dobjModels[32]; // [esp+78h] [ebp-100h] BYREF

    DObjGetCreateParms(obj, dobjModels, &savedObj.numModels, &savedObj.tree, &savedObj.entnum);
    savedObj.ignoreCollision = 0;
    savedObj.models = obj->models;
    savedObj.hidePartBits[0] = obj->hidePartBits[0];
    savedObj.hidePartBits[1] = obj->hidePartBits[1];
    savedObj.hidePartBits[2] = obj->hidePartBits[2];
    savedObj.hidePartBits[3] = obj->hidePartBits[3];
    for (modelIndex = 0; modelIndex < savedObj.numModels; ++modelIndex)
    {
        model = &dobjModels[modelIndex];
        savedObj.dobjModels[modelIndex].boneName = model->boneName;
        if (model->ignoreCollision)
            savedObj.ignoreCollision |= 1 << modelIndex;
    }

    iassert(obj->models);
    obj->models = NULL;
    DObjFree(obj);

    static_assert((sizeof(DObj_s) - sizeof(obj->models)) == 96);
    memcpy(obj, &savedObj, sizeof(DObj_s) - sizeof(obj->models));
}

void __cdecl DObjUnarchive(DObj_s *obj)
{
    DObjModel_s *model; // [esp+8h] [ebp-170h]
    SavedDObj savedObj; // [esp+10h] [ebp-168h] BYREF
    unsigned int modelIndex; // [esp+74h] [ebp-104h]
    DObjModel_s dobjModels[32]; // [esp+78h] [ebp-100h] BYREF

    memcpy(&savedObj, obj, sizeof(savedObj));
    for (modelIndex = 0; modelIndex < savedObj.numModels; ++modelIndex)
    {
        model = &dobjModels[modelIndex];
        model->boneName = savedObj.dobjModels[modelIndex].boneName;
        model->model = savedObj.models[modelIndex];
        model->ignoreCollision = (savedObj.ignoreCollision & (1 << modelIndex)) != 0;
    }
    MT_Free((_BYTE *)savedObj.models, 5 * savedObj.numModels);
    DObjCreate(dobjModels, savedObj.numModels, savedObj.tree, obj, savedObj.entnum);
    DObjSetHidePartBits(obj, savedObj.hidePartBits);
}

void __cdecl DObjSkelClear(const DObj_s *obj)
{
    memset((unsigned __int8 *)&obj->skel, 0, sizeof(obj->skel));
}

void __cdecl DObjGetBounds(const DObj_s *obj, float *mins, float *maxs)
{
    iassert(obj);

    mins[0] = -obj->radius;
    mins[1] = -obj->radius;
    mins[2] = -obj->radius;

    maxs[0] = obj->radius;
    maxs[1] = obj->radius;
    maxs[2] = obj->radius;
}

void __cdecl DObjPhysicsGetBounds(const DObj_s *obj, float *mins, float *maxs)
{
    iassert(obj);
    iassert(obj->models[0]);
    XModelGetBounds(obj->models[0], mins, maxs);
}

void __cdecl DObjPhysicsSetCollisionFromXModel(const DObj_s *obj, PhysWorld worldIndex, dxBody *physId)
{
    Phys_ObjSetCollisionFromXModel(*(const XModel **)obj->models, worldIndex, physId);
}

double __cdecl DObjGetRadius(const DObj_s *obj)
{
    iassert(obj);
    return obj->radius;
}

PhysPreset *__cdecl DObjGetPhysPreset(const DObj_s *obj)
{
    iassert(obj);
    iassert(obj->numModels > 0);
    iassert(obj->models[0]);

    return obj->models[0]->physPreset;
}

const char *__cdecl DObjGetName(const DObj_s *obj)
{
    iassert(obj);
    iassert(obj->numModels > 0);
    iassert(obj->models[0]);

    return obj->models[0]->name;
}

char *__cdecl DObjGetBoneName(const DObj_s *obj, int boneIndex)
{
    int j; // [esp+0h] [ebp-20h]
    int numBones; // [esp+4h] [ebp-1Ch]
    int baseBoneIndex; // [esp+8h] [ebp-18h]
    int numModels; // [esp+Ch] [ebp-14h]
    XModel *model; // [esp+10h] [ebp-10h]
    int index; // [esp+14h] [ebp-Ch]
    XModel **models; // [esp+18h] [ebp-8h]
    unsigned const __int16 *boneNames; // [esp+1Ch] [ebp-4h]

    iassert(obj);

    numModels = obj->numModels;
    baseBoneIndex = 0;
    models = obj->models;
    for (j = 0; j < numModels; ++j)
    {
        model = models[j];
        boneNames = model->boneNames;
        numBones = model->numBones;
        index = boneIndex - baseBoneIndex;
        iassert(index >= 0);

        if (index < numBones)
            return SL_ConvertToString(boneNames[index]);

        baseBoneIndex += numBones;
    }
    return 0;
}

char *__cdecl DObjGetModelParentBoneName(const DObj_s *obj, int modelIndex)
{
    iassert(obj);
    iassert(modelIndex < obj->numModels);

    return DObjGetBoneName(obj, *((unsigned __int8 *)&obj->models[obj->numModels] + modelIndex));
}

XAnimTree_s *__cdecl DObjGetTree(const DObj_s *obj)
{
    iassert(obj);
    return obj->tree;
}

void __cdecl DObjTraceline(DObj_s *obj, float *start, float *end, unsigned __int8 *priorityMap, DObjTrace_s *trace)
{
    double v5; // st7
    const char *v6; // eax
    double scale; // [esp+18h] [ebp-398h]
    float v8; // [esp+20h] [ebp-390h]
    float v9; // [esp+24h] [ebp-38Ch]
    float v10; // [esp+28h] [ebp-388h]
    float v11; // [esp+2Ch] [ebp-384h]
    float v12; // [esp+30h] [ebp-380h]
    float v13; // [esp+34h] [ebp-37Ch]
    unsigned __int16 v14; // [esp+3Ah] [ebp-376h]
    float v15; // [esp+6Ch] [ebp-344h]
    float v16; // [esp+70h] [ebp-340h]
    float v17; // [esp+74h] [ebp-33Ch]
    float v18; // [esp+78h] [ebp-338h]
    float v19[3]; // [esp+7Ch] [ebp-334h] BYREF
    float v20; // [esp+88h] [ebp-328h]
    float v21; // [esp+8Ch] [ebp-324h]
    float v22; // [esp+90h] [ebp-320h]
    float v23; // [esp+94h] [ebp-31Ch]
    float v24; // [esp+98h] [ebp-318h]
    float v25; // [esp+C0h] [ebp-2F0h]
    float v26; // [esp+C4h] [ebp-2ECh]
    float v27; // [esp+C8h] [ebp-2E8h]
    float v28; // [esp+CCh] [ebp-2E4h]
    float v29; // [esp+D0h] [ebp-2E0h]
    float v30; // [esp+D4h] [ebp-2DCh]
    float v31; // [esp+D8h] [ebp-2D8h]
    float v32; // [esp+DCh] [ebp-2D4h]
    float v33; // [esp+E0h] [ebp-2D0h]
    float v34; // [esp+E4h] [ebp-2CCh]
    float v35; // [esp+E8h] [ebp-2C8h]
    float *v36; // [esp+ECh] [ebp-2C4h]
    float transWeight; // [esp+F0h] [ebp-2C0h]
    float v38; // [esp+F4h] [ebp-2BCh]
    float v39; // [esp+F8h] [ebp-2B8h]
    float v40; // [esp+FCh] [ebp-2B4h]
    float v41; // [esp+100h] [ebp-2B0h]
    float v42; // [esp+104h] [ebp-2ACh]
    float v43; // [esp+108h] [ebp-2A8h]
    float v44; // [esp+10Ch] [ebp-2A4h]
    float v45; // [esp+110h] [ebp-2A0h]
    float result[3]; // [esp+114h] [ebp-29Ch] BYREF
    float v47; // [esp+120h] [ebp-290h]
    float v48; // [esp+124h] [ebp-28Ch]
    float v49; // [esp+128h] [ebp-288h]
    float v50; // [esp+12Ch] [ebp-284h]
    float v51; // [esp+130h] [ebp-280h]
    float v52; // [esp+134h] [ebp-27Ch]
    float v53; // [esp+138h] [ebp-278h]
    float v54; // [esp+13Ch] [ebp-274h]
    float v55; // [esp+140h] [ebp-270h]
    float v56; // [esp+144h] [ebp-26Ch]
    float v57; // [esp+148h] [ebp-268h]
    float v58; // [esp+14Ch] [ebp-264h]
    float v59; // [esp+150h] [ebp-260h]
    float v60; // [esp+154h] [ebp-25Ch]
    float v61; // [esp+158h] [ebp-258h]
    float v62; // [esp+15Ch] [ebp-254h]
    float v63; // [esp+160h] [ebp-250h]
    float v64; // [esp+164h] [ebp-24Ch]
    float v65; // [esp+168h] [ebp-248h]
    float v66; // [esp+16Ch] [ebp-244h]
    float v67; // [esp+170h] [ebp-240h]
    float v68; // [esp+174h] [ebp-23Ch]
    float v69; // [esp+178h] [ebp-238h]
    float v70; // [esp+17Ch] [ebp-234h]
    float v71; // [esp+180h] [ebp-230h]
    float v72; // [esp+184h] [ebp-22Ch]
    float v73; // [esp+188h] [ebp-228h]
    float *normal; // [esp+18Ch] [ebp-224h]
    DObjAnimMat *boneMatrix; // [esp+1A4h] [ebp-20Ch]
    unsigned int j; // [esp+1A8h] [ebp-208h]
    const unsigned __int8 *pos; // [esp+1ACh] [ebp-204h]
    const unsigned __int8 *modelParents; // [esp+1B0h] [ebp-200h]
    float invL2; // [esp+1B4h] [ebp-1FCh]
    int t; // [esp+1B8h] [ebp-1F8h]
    bool bEndSolid; // [esp+1BFh] [ebp-1F1h]
    float delta[3]; // [esp+1C0h] [ebp-1F0h] BYREF
    DObjAnimMat *hitBoneMatrix; // [esp+1CCh] [ebp-1E4h]
    float sphereFraction; // [esp+1D0h] [ebp-1E0h]
    float localStart[3]; // [esp+1D4h] [ebp-1DCh] BYREF
    float enterFrac; // [esp+1E0h] [ebp-1D0h]
    float startOffset[3]; // [esp+1E4h] [ebp-1CCh] BYREF
    float dist1; // [esp+1F0h] [ebp-1C0h]
    int hitT; // [esp+1F4h] [ebp-1BCh]
    unsigned __int16 classificationArray[128]; // [esp+1F8h] [ebp-1B8h]
    XModel *model; // [esp+2FCh] [ebp-B4h]
    float dist; // [esp+300h] [ebp-B0h]
    unsigned int numModels; // [esp+304h] [ebp-ACh]
    float diff2; // [esp+308h] [ebp-A8h]
    float deltaLengthSq; // [esp+30Ch] [ebp-A4h]
    float sign; // [esp+310h] [ebp-A0h]
    unsigned int size; // [esp+314h] [ebp-9Ch]
    float offset[3]; // [esp+318h] [ebp-98h] BYREF
    unsigned int globalBoneIndex; // [esp+324h] [ebp-8Ch]
    float d2; // [esp+328h] [ebp-88h]
    float cappedSphereFraction; // [esp+32Ch] [ebp-84h]
    float localEnd[3]; // [esp+330h] [ebp-80h] BYREF
    float *bounds; // [esp+33Ch] [ebp-74h]
    bool bStartSolid; // [esp+343h] [ebp-6Dh]
    unsigned int localBoneIndex; // [esp+344h] [ebp-6Ch]
    unsigned __int8 parentIndex; // [esp+34Bh] [ebp-65h]
    float solidHitFrac; // [esp+34Ch] [ebp-64h]
    float dist2; // [esp+350h] [ebp-60h]
    DSkel *skel; // [esp+354h] [ebp-5Ch]
    unsigned __int16 *names; // [esp+358h] [ebp-58h]
    float leaveFrac; // [esp+35Ch] [ebp-54h]
    int traceHitT; // [esp+360h] [ebp-50h]
    unsigned int lowestPriority; // [esp+364h] [ebp-4Ch]
    XBoneInfo *boneInfo; // [esp+368h] [ebp-48h]
    int ignoreCollision; // [esp+36Ch] [ebp-44h]
    float axis[3][3]; // [esp+370h] [ebp-40h] BYREF
    XModel **models; // [esp+394h] [ebp-1Ch]
    unsigned __int16 classification; // [esp+398h] [ebp-18h]
    float center[3]; // [esp+39Ch] [ebp-14h] BYREF
    float hitSign; // [esp+3A8h] [ebp-8h]
    unsigned int currentPriority; // [esp+3ACh] [ebp-4h]

    Profile_Begin(311);
    trace->surfaceflags = 0;
    trace->modelIndex = 0;
    trace->partName = 0;
    trace->partGroup = 0;
    normal = trace->normal;
    trace->normal[0] = 0.0;
    normal[1] = 0.0;
    normal[2] = 0.0;
    Vec3Sub(end, start, delta);
    deltaLengthSq = Vec3LengthSq(delta);
    if (deltaLengthSq == 0.0 || (boneMatrix = DObjGetRotTransArray(obj)) == 0)
    {
        Profile_EndInternal(0);
        return;
    }
    invL2 = 1.0 / deltaLengthSq;
    lowestPriority = 2;
    skel = &obj->skel;
    iassert(skel);
    iassert(obj->duplicateParts);
    pos = (const unsigned char*)(SL_ConvertToString(obj->duplicateParts) + 16);
    iassert(!IS_NAN(start[0]) && !IS_NAN(start[1]) && !IS_NAN(start[2]));
    iassert(!IS_NAN(end[0]) && !IS_NAN(end[1]) && !IS_NAN(end[2]));
    globalBoneIndex = 0;
    hitT = -1;
    traceHitT = -1;
    hitSign = 0.0;
    hitBoneMatrix = 0;
    solidHitFrac = trace->fraction;
    models = obj->models;
    modelParents =  (const unsigned char*)&models[obj->numModels];
    numModels = obj->numModels;
    j = 0;
LABEL_17:
    if (j < numModels)
    {
        model = models[j];
        names = model->boneNames;
        size = model->numBones;
        ignoreCollision = obj->ignoreCollision & (1 << j);
        localBoneIndex = 0;
        while (1)
        {
            if (localBoneIndex >= size)
            {
                ++j;
                goto LABEL_17;
            }
            classification = model->partClassification[localBoneIndex];
            currentPriority = priorityMap[classification];
            if (globalBoneIndex == *pos - 1)
            {
                pos += 2;
                if (currentPriority == 1)
                {
                    classification = classificationArray[*(pos - 1) - 1];
                    currentPriority = priorityMap[classification];
                }
            }
            else if (currentPriority == 1)
            {
                if (localBoneIndex >= model->numRootBones)
                {
                    classification = classificationArray[globalBoneIndex - model->parentList[localBoneIndex - model->numRootBones]];
                }
                else
                {
                    parentIndex = modelParents[j];
                    if (parentIndex == 255)
                        v14 = 0;
                    else
                        v14 = classificationArray[parentIndex];
                    classification = v14;
                }
                currentPriority = priorityMap[classification];
            }
            classificationArray[globalBoneIndex] = classification;
            if (!ignoreCollision)
            {
                boneInfo = &model->boneInfo[localBoneIndex];
                if (LODWORD(boneInfo->radiusSquared))
                {
                    if ((obj->hidePartBits[globalBoneIndex >> 5] & (0x80000000 >> (globalBoneIndex & 0x1F))) == 0)
                    {
                        iassert(skel->partBits.skel[globalBoneIndex >> 5] & (HIGH_BIT >> (globalBoneIndex & 31)));
                        if (lowestPriority <= currentPriority)
                        {
                            iassert(!IS_NAN(boneMatrix->quat[0]) && !IS_NAN(boneMatrix->quat[1]) && !IS_NAN(boneMatrix->quat[2]) && !IS_NAN(boneMatrix->quat[3]));
                            iassert(!IS_NAN(boneMatrix->trans[0]) && !IS_NAN(boneMatrix->trans[1]) && !IS_NAN(boneMatrix->trans[2]) && !IS_NAN(boneMatrix->trans[3]));
                            iassert(!IS_NAN(boneMatrix->transWeight));

                            MatrixTransformVectorQuatTrans(boneInfo->offset, boneMatrix, center);

                            Vec3Sub(start, center, startOffset);
                            v5 = Vec3Dot(startOffset, delta);
                            sphereFraction = -v5 * invL2;
                            v13 = sphereFraction - 1.0;
                            v35 = v13 < 0.0 ? sphereFraction : 1.0;
                            v12 = 0.0 - sphereFraction;
                            v11 = v12 < 0.0 ? v35 : 0.0;
                            cappedSphereFraction = v11;
                            Vec3Mad(startOffset, v11, delta, offset);
                            d2 = Vec3LengthSq(offset);
                            diff2 = boneInfo->radiusSquared - d2;
                            if (diff2 > 0.0)
                            {
                                if (lowestPriority != currentPriority
                                    || (float)(sphereFraction - sqrtf(diff2 * invL2)) < trace->fraction)
                                {
                                    InvMatrixTransformVectorQuatTrans(start, boneMatrix, localStart);
                                    InvMatrixTransformVectorQuatTrans(end, boneMatrix, localEnd);
                                    iassert(!IS_NAN(localStart[0]) && !IS_NAN(localStart[1]) && !IS_NAN(localStart[2]));
                                    iassert(!IS_NAN(localEnd[0]) && !IS_NAN(localEnd[1]) && !IS_NAN(localEnd[2]));
                                    enterFrac = 0.0;
                                    if (lowestPriority == currentPriority)
                                        leaveFrac = trace->fraction;
                                    else
                                        leaveFrac = solidHitFrac;
                                    bStartSolid = 1;
                                    bEndSolid = 1;
                                    sign = -1.0;
                                    for (bounds = (float*)boneInfo; ; bounds += 3)
                                    {
                                        iassert(!IS_NAN(bounds[0]) && !IS_NAN(bounds[1]) && !IS_NAN(bounds[2]));
                                        for (t = 0; t < 3; ++t)
                                        {
                                            dist1 = (localStart[t] - bounds[t]) * sign;
                                            dist2 = (localEnd[t] - bounds[t]) * sign;
                                            if (dist1 <= 0.0)
                                            {
                                                if (dist2 > 0.0)
                                                {
                                                    bEndSolid = 0;
                                                    dist = dist1 - dist2;
                                                    iassert(dist < 0.0f);
                                                    if (dist1 > leaveFrac * dist)
                                                    {
                                                        leaveFrac = dist1 / dist;
                                                        if (leaveFrac <= enterFrac)
                                                            goto LABEL_19;
                                                    }
                                                }
                                            }
                                            else
                                            {
                                                if (dist2 > 0.0)
                                                    goto LABEL_19;
                                                bStartSolid = 0;
                                                dist = dist1 - dist2;
                                                iassert(dist > 0.0f);
                                                if (dist1 > enterFrac * dist)
                                                {
                                                    enterFrac = dist1 / dist;
                                                    if (leaveFrac <= enterFrac)
                                                        goto LABEL_19;
                                                    hitSign = sign;
                                                    hitT = t;
                                                }
                                            }
                                        }
                                        if (sign == 1.0f)
                                            break;
                                        sign = 1.0f;
                                    }
                                    if (bStartSolid)
                                    {
                                        if (bEndSolid)
                                        {
                                            v8 = start[1] * delta[1] + *start * delta[0];
                                            if (v8 < 0.0)
                                            {
                                                trace->fraction = 0.0;
                                                trace->modelIndex = j;
                                                iassert(trace->modelIndex == j);
                                                trace->partName = names[localBoneIndex];
                                                trace->partGroup = classification;
                                                Vec2NormalizeTo(start, trace->normal);
                                                iassert(Vec3IsNormalized(trace->normal));
                                                Profile_EndInternal(0);
                                                return;
                                            }
                                        }
                                    }
                                    else
                                    {
                                        if (lowestPriority == currentPriority)
                                        {
                                            if (trace->fraction <= enterFrac)
                                                goto LABEL_19;
                                        }
                                        else
                                        {
                                            lowestPriority = currentPriority;
                                        }
                                        trace->fraction = enterFrac;
                                        iassert(trace->fraction >= 0.0f && trace->fraction <= 1.0f);
                                        trace->modelIndex = j;
                                        iassert(trace->modelIndex == j);
                                        trace->partName = names[localBoneIndex];
                                        trace->partGroup = classification;
                                        iassert(hitT >= 0);
                                        iassert(hitT < 3);
                                        traceHitT = hitT;
                                        hitBoneMatrix = boneMatrix;
                                    }
                                }
                            }
                        }
                    }
                }
            }
        LABEL_19:
            ++localBoneIndex;
            ++boneMatrix;
            ++globalBoneIndex;
        }
    }
    if (hitBoneMatrix)
    {
        iassert(traceHitT >= 0);
        iassert(traceHitT < 3);
        ConvertQuatToMat(hitBoneMatrix, axis);
        Vec3Scale(axis[traceHitT], hitSign, trace->normal);
    }
    Profile_EndInternal(0);
}

void __cdecl DObjTracelinePartBits(DObj_s *obj, int *partBits)
{
    unsigned int j; // [esp+30h] [ebp-24h]
    XModel *model; // [esp+34h] [ebp-20h]
    unsigned int numModels; // [esp+38h] [ebp-1Ch]
    unsigned int size; // [esp+3Ch] [ebp-18h]
    unsigned int globalBoneIndex; // [esp+40h] [ebp-14h]
    unsigned int localBoneIndex; // [esp+44h] [ebp-10h]
    XModel **models; // [esp+50h] [ebp-4h]

    Profile_Begin(311);
    partBits[0] = 0;
    partBits[1] = 0;
    partBits[2] = 0;
    partBits[3] = 0;
    globalBoneIndex = 0;
    models = obj->models;
    numModels = obj->numModels;
    for (j = 0; j < numModels; ++j)
    {
        model = models[j];
        size = model->numBones;
        if ((obj->ignoreCollision & (1 << j)) != 0)
        {
            globalBoneIndex += size;
        }
        else
        {
            for (localBoneIndex = 0; localBoneIndex < size; ++localBoneIndex)
            {
                if (LODWORD(model->boneInfo[localBoneIndex].radiusSquared))
                {
                    if ((obj->hidePartBits[globalBoneIndex >> 5] & (0x80000000 >> (globalBoneIndex & 0x1F))) == 0)
                        partBits[globalBoneIndex >> 5] |= 0x80000000 >> (globalBoneIndex & 0x1F);
                }
                ++globalBoneIndex;
            }
        }
    }
    DObjCompleteHierarchyBits(obj, partBits);
    Profile_EndInternal(0);
}

void __cdecl DObjGeomTraceline(
    DObj_s *obj,
    float *localStart,
    float *const localEnd,
    int contentmask,
    DObjTrace_s *results)
{
    unsigned int boneIndex; // [esp+Ch] [ebp-48h]
    XModel* model; // [esp+10h] [ebp-44h]
    int partIndex; // [esp+14h] [ebp-40h]
    unsigned int numModels; // [esp+18h] [ebp-3Ch]
    trace_t trace; // [esp+1Ch] [ebp-38h] BYREF
    DObjAnimMat* boneMtxList; // [esp+48h] [ebp-Ch]
    unsigned int i; // [esp+4Ch] [ebp-8h]
    XModel** models; // [esp+50h] [ebp-4h]

    results->modelIndex = 0;
    results->partName = 0;
    results->partGroup = 0;
    trace.fraction = results->fraction;
    if (trace.fraction < 0.0 || trace.fraction > 1.0)
        MyAssertHandler(
            ".\\xanim\\dobj.cpp",
            1431,
            1,
            "%s\n\t(trace.fraction) = %g",
            "(trace.fraction >= 0.0f && trace.fraction <= 1.0f)",
            trace.fraction);
    trace.surfaceFlags = 0;
    trace.normal[0] = 0.0;
    trace.normal[1] = 0.0;
    trace.normal[2] = 0.0;
    boneMtxList = DObjGetRotTransArray(obj);
    if (boneMtxList)
    {
        models = obj->models;
        boneIndex = 0;
        numModels = obj->numModels;
        i = 0;
        while (i < numModels)
        {
            model = models[i];
            partIndex = XModelTraceLineAnimated(obj, i, boneIndex, &trace, boneMtxList, localStart, localEnd, contentmask);
            if (partIndex >= 0)
            {
                results->modelIndex = i;
                if (results->modelIndex != i)
                    MyAssertHandler(".\\xanim\\dobj.cpp", 1450, 1, "%s", "results->modelIndex == i");
                results->partName = model->boneNames[partIndex];
            }
            ++i;
            boneIndex += model->numBones;
            boneMtxList += model->numBones;
        }
    }
    results->fraction = trace.fraction;
    if (results->fraction < 0.0 || results->fraction > 1.0)
        MyAssertHandler(
            ".\\xanim\\dobj.cpp",
            1456,
            1,
            "%s\n\t(results->fraction) = %g",
            "(results->fraction >= 0.0f && results->fraction <= 1.0f)",
            results->fraction);
    results->surfaceflags = trace.surfaceFlags;
    results->normal[0] = trace.normal[0];
    results->normal[1] = trace.normal[1];
    results->normal[2] = trace.normal[2];
}

void __cdecl DObjGeomTracelinePartBits(DObj_s *obj, int contentmask, int *partBits)
{
    unsigned int boneIndex; // [esp+0h] [ebp-14h]
    XModel *model; // [esp+4h] [ebp-10h]
    unsigned int numModels; // [esp+8h] [ebp-Ch]
    unsigned int i; // [esp+Ch] [ebp-8h]
    XModel **models; // [esp+10h] [ebp-4h]

    *partBits = 0;
    partBits[1] = 0;
    partBits[2] = 0;
    partBits[3] = 0;
    models = obj->models;
    boneIndex = 0;
    numModels = obj->numModels;
    for (i = 0; i < numModels; ++i)
    {
        model = models[i];
        XModelTraceLineAnimatedPartBits(obj, i, boneIndex, contentmask, partBits);
        boneIndex += model->numBones;
    }
    DObjCompleteHierarchyBits(obj, partBits);
}

int __cdecl DObjHasContents(DObj_s *obj, int contentmask)
{
    int i; // [esp+0h] [ebp-8h]
    XModel **models; // [esp+4h] [ebp-4h]

    models = obj->models;
    for (i = 0; i < obj->numModels; ++i)
    {
        if ((contentmask & XModelGetContents(models[i])) != 0)
            return 1;
    }
    return 0;
}

int __cdecl DObjGetContents(const DObj_s *obj)
{
    int contents; // [esp+0h] [ebp-8h]
    int i; // [esp+4h] [ebp-4h]

    contents = 0;
    for (i = 0; i < obj->numModels; ++i)
        contents |= XModelGetContents(obj->models[i]);
    return contents;
}

int __cdecl DObjSetLocalBoneIndex(DObj_s *obj, int *partBits, int boneIndex, const float *trans, const float *angles)
{
    if (!DObjSetRotTransIndex(obj, partBits, boneIndex))
        return 0;
    DObjSetLocalTagInternal(obj, trans, angles, boneIndex);
    return 1;
}

int __cdecl DObjGetBoneIndex(const DObj_s *obj, unsigned int name, unsigned __int8 *index)
{
    int j; // [esp+0h] [ebp-18h]
    int ja; // [esp+0h] [ebp-18h]
    unsigned int boneIndex; // [esp+4h] [ebp-14h]
    int numModels; // [esp+8h] [ebp-10h]
    XModel *model; // [esp+Ch] [ebp-Ch]
    XModel *modela; // [esp+Ch] [ebp-Ch]
    unsigned int localBoneIndex; // [esp+10h] [ebp-8h]
    XModel **models; // [esp+14h] [ebp-4h]

    iassert(obj);
    iassert(name);
    iassert(SL_IsLowercaseString(name));

    localBoneIndex = *index;
    if (localBoneIndex == 255)
        return 0;

    numModels = obj->numModels;
    models = obj->models;
    if (localBoneIndex < obj->numBones)
    {
        for (j = 0; j < numModels; ++j)
        {
            model = models[j];
            if (localBoneIndex < model->numBones)
            {
                if (name != model->boneNames[localBoneIndex])
                    break;
                return 1;
            }
            localBoneIndex -= model->numBones;
        }
    }
    boneIndex = 0;
    for (ja = 0; ja < numModels; ++ja)
    {
        modela = models[ja];
        if (XModelGetBoneIndex(modela, name, boneIndex, index))
            return 1;
        boneIndex += modela->numBones;
    }
    *index = -1;
    return 0;
}

int __cdecl DObjGetModelBoneIndex(const DObj_s *obj, const char *modelName, unsigned int name, unsigned __int8 *index)
{
    char *v4; // eax
    int j; // [esp+0h] [ebp-18h]
    int ja; // [esp+0h] [ebp-18h]
    unsigned int boneIndex; // [esp+4h] [ebp-14h]
    int numModels; // [esp+8h] [ebp-10h]
    XModel *model; // [esp+Ch] [ebp-Ch]
    XModel *modela; // [esp+Ch] [ebp-Ch]
    unsigned int localBoneIndex; // [esp+10h] [ebp-8h]
    XModel **models; // [esp+14h] [ebp-4h]

    if (!obj)
        MyAssertHandler(".\\xanim\\dobj.cpp", 1698, 0, "%s", "obj");
    if (!name)
        MyAssertHandler(".\\xanim\\dobj.cpp", 1699, 0, "%s", "name");
    if (!SL_IsLowercaseString(name))
    {
        v4 = SL_ConvertToString(name);
        MyAssertHandler(
            ".\\xanim\\dobj.cpp",
            1701,
            0,
            "%s\n\t(SL_ConvertToString( name )) = %s",
            "(SL_IsLowercaseString( name ))",
            v4);
    }
    localBoneIndex = *index;
    if (localBoneIndex == 255)
        return 0;
    numModels = obj->numModels;
    models = obj->models;
    if (localBoneIndex < obj->numBones)
    {
        for (j = 0; j < numModels; ++j)
        {
            model = models[j];
            if (localBoneIndex < model->numBones)
            {
                if (name == model->boneNames[localBoneIndex] && !I_stricmp(modelName, model->name))
                    return 1;
                break;
            }
            localBoneIndex -= model->numBones;
        }
    }
    boneIndex = 0;
    for (ja = 0; ja < numModels; ++ja)
    {
        modela = models[ja];
        if (!I_stricmp(modelName, modela->name) && XModelGetBoneIndex(modela, name, boneIndex, index))
            return 1;
        boneIndex += modela->numBones;
    }
    *index = -1;
    return 0;
}

// LWSS: for ragdolls
void __cdecl DObjGetBasePoseMatrix(const DObj_s *obj, unsigned __int8 boneIndex, DObjAnimMat *outMat)
{
    DObjAnimMat mat[128]; // [esp+8h] [ebp-1010h] BYREF
    int partBits[4]; // [esp+1008h] [ebp-10h] BYREF

    iassert(obj);
    iassert(outMat);
    iassert(boneIndex < obj->numBones); 

    if (boneIndex >= (int)(*obj->models)->numBones)
    {
        DObjGetHierarchyBits(obj, boneIndex, partBits);
        DObjCalcBaseSkel(obj, mat, partBits);
        memcpy(outMat, &mat[boneIndex], sizeof(DObjAnimMat));
    }
    else
    {
        memcpy(outMat, &XModelGetBasePose(*(const XModel **)obj->models)[boneIndex], sizeof(DObjAnimMat));
    }
}

void __cdecl DObjSetHidePartBits(DObj_s *obj, const unsigned int *partBits)
{
    obj->hidePartBits[0] = *partBits;
    obj->hidePartBits[1] = partBits[1];
    obj->hidePartBits[2] = partBits[2];
    obj->hidePartBits[3] = partBits[3];
}

