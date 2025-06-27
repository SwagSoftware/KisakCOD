#include "xanim.h"
#include "xanim_calc.h"

#include "dobj.h"
#include <cgame_mp/cg_local_mp.h>

#include <qcommon/qcommon.h>
#include <database/database.h>
#include <qcommon/threads.h>
#include <universal/profile.h>
#include <script/scr_vm.h>
#include <universal/com_files.h>


static int g_info_usage;
static int g_info_high_usage;
int g_notifyListSize;

static unsigned int g_endNotetrackName;

static bool g_anim_developer;

static XAnimNotify_s g_notifyList[0x80];
static XAnimInfo g_xAnimInfo[0x1000];

int __cdecl XAnimGetTreeHighMemUsage()
{
    return g_info_high_usage << 6;
}

int __cdecl XAnimGetTreeMemUsage()
{
    return g_info_usage << 6;
}

void __cdecl TRACK_xanim()
{
    //track_static_alloc_internal(g_xAnimInfo, 0x40000, "g_xAnimInfo", 11);
    //track_static_alloc_internal(g_notifyList, 1536, "g_notifyList", 11);
}

int __cdecl XAnimGetTreeMaxMemUsage()
{
    return 0x40000;
}

void __cdecl XAnimInit()
{
    int i; // [esp+0h] [ebp-4h]

    for (i = 0; i < 4096; ++i)
    {
        g_xAnimInfo[i].prev = (i + 4095) % 4096;
        g_xAnimInfo[i].next = (i + 1) % 4096;
    }
    g_xAnimInfo[0].state.currentAnimTime = 0.0;
    g_xAnimInfo[0].state.oldTime = 0.0;
    *(DWORD*)&g_xAnimInfo[0].state.cycleCount = 0;
    g_xAnimInfo[0].state.goalTime = 0.0;
    g_xAnimInfo[0].state.goalWeight = 0.0;
    g_xAnimInfo[0].state.weight = 0.0;
    g_xAnimInfo[0].state.rate = 0.0;
    *(DWORD*)&g_xAnimInfo[0].state.instantWeightChange = 0;
    g_endNotetrackName = SL_GetString_("end", 0, 3);
    g_anim_developer = 1;
    g_info_usage = 1;
    g_info_high_usage = 1;
}

void __cdecl XAnimShutdown()
{
    if (g_endNotetrackName)
    {
        XAnimCheckTreeLeak();
        SL_RemoveRefToString(g_endNotetrackName);
        g_endNotetrackName = 0;
    }
}

XAnimParts* __cdecl XAnimFindData_LoadObj(const char* name)
{
    return (XAnimParts*)Hunk_FindDataForFile(6, name);
}

XAnimParts* __cdecl XAnimFindData_FastFile(const char* name)
{
    return DB_FindXAssetHeader(ASSET_TYPE_XANIMPARTS, name).parts;
}

void __cdecl XAnimCreate(XAnim_s* anims, unsigned int animIndex, const char* name)
{
    char v4; // [esp+3h] [ebp-31h]
    char* v5; // [esp+8h] [ebp-2Ch]
    const char* v6; // [esp+Ch] [ebp-28h]
    char* debugName; // [esp+28h] [ebp-Ch]
    XAnimParts* parts; // [esp+30h] [ebp-4h]

    if (useFastFile->current.enabled)
        parts = XAnimFindData_FastFile(name);
    else
        parts = XAnimFindData_LoadObj(name);

    if (useFastFile->current.enabled || parts)
    {
        iassert(parts);
        anims->entries[animIndex].numAnims = 0;
        anims->entries[animIndex].parts = parts;
        if (anims->debugAnimNames)
        {
            iassert(!anims->debugAnimNames[animIndex]);
            debugName = (char*)Hunk_AllocDebugMem(strlen(name) + 1, "XAnimCreate");
            v6 = name;
            v5 = debugName;
            do
            {
                v4 = *v6;
                *v5++ = *v6++;
            } while (v4);
            anims->debugAnimNames[animIndex] = debugName;
        }
    }
    else
    {
        Com_Error(ERR_DROP, "Cannot find xanim %s", name);
    }
}

XAnimParts *__cdecl XAnimClone(XAnimParts *fromParts, void *(__cdecl *Alloc)(int))
{
    XAnimParts *toParts; // [esp+8h] [ebp-18h]
    int size; // [esp+Ch] [ebp-14h]
    XAnimNotifyInfo *notify; // [esp+10h] [ebp-10h]
    int i; // [esp+14h] [ebp-Ch]
    __int16 notifyInfoIndex; // [esp+18h] [ebp-8h]
    unsigned __int16 *boneNames; // [esp+1Ch] [ebp-4h]

    toParts = (XAnimParts*)Alloc(88);
    qmemcpy(toParts, fromParts, sizeof(XAnimParts));
    boneNames = toParts->names;
    size = toParts->boneCount[9];
    for (i = 0; i < size; ++i)
        SL_AddRefToString(boneNames[i]);
    notifyInfoIndex = 0;
    notify = toParts->notify;
    while (notifyInfoIndex < toParts->notifyCount)
    {
        SL_AddRefToString(notify->name);
        ++notifyInfoIndex;
        ++notify;
    }
    return toParts;
}

XAnimParts *__cdecl XAnimPrecache(const char *name, void *(__cdecl *Alloc)(int))
{
    XAnimParts *result; // eax
    XAnimParts *Data_FastFile; // eax
    XAnimParts *defaultParts; // [esp+Ch] [ebp-8h]
    XAnimParts *parts; // [esp+10h] [ebp-4h]

    if (useFastFile->current.enabled)
        result = XAnimFindData_FastFile(name);
    else
        result = XAnimFindData_LoadObj(name);
    if (!result)
    {
        parts = XAnimLoadFile((char*)name, Alloc);
        if (!parts)
        {
            Com_PrintWarning(19, "WARNING: Couldn't find xanim '%s', using default xanim '%s' instead\n", name, "void");
            if (useFastFile->current.enabled)
                Data_FastFile = XAnimFindData_FastFile("void");
            else
                Data_FastFile = XAnimFindData_LoadObj("void");
            defaultParts = Data_FastFile;
            if (!Data_FastFile)
            {
                defaultParts = XAnimLoadFile((char*)"void", Alloc);
                if (!defaultParts)
                {
                    Com_Error(ERR_DROP, "Cannot find xanim %s", "void");
                    return 0;
                }
                Hunk_SetDataForFile(6, "void", defaultParts, Alloc);
            }
            parts = XAnimClone(defaultParts, Alloc);
            parts->isDefault = 1;
        }
        parts->name = Hunk_SetDataForFile(6, name, parts, Alloc);
        return parts;
    }
    return result;
}

void __cdecl XAnimBlend(
    XAnim_s* anims,
    unsigned int animIndex,
    const char* name,
    unsigned int children,
    unsigned int num,
    unsigned int flags)
{
    char v6; // [esp+3h] [ebp-31h]
    char* v7; // [esp+8h] [ebp-2Ch]
    const char* v8; // [esp+Ch] [ebp-28h]
    int parentIndex; // [esp+24h] [ebp-10h]
    char* debugName; // [esp+28h] [ebp-Ch]
    XAnimEntry* anim; // [esp+2Ch] [ebp-8h]
    unsigned int i; // [esp+30h] [ebp-4h]

    iassert(num > 0);
    anim = &anims->entries[animIndex];
    anim->numAnims = num;
    iassert(num == anim->numAnims);
    anims->entries[animIndex].animParent.flags = flags;
    iassert(flags == anim->animParent.flags);
    anims->entries[animIndex].animParent.children = children;
    iassert(children == anim->animParent.children);
    for (i = 0; i < num; ++i)
    {
        anims->entries[i + anims->entries[animIndex].animParent.children].parent = animIndex;
        iassert(animIndex == anims->entries[anim->animParent.children + i].parent);
    }
    if (IsNodeAdditive(anim))
    {
        for (parentIndex = anims->entries[animIndex].parent; parentIndex; parentIndex = anims->entries[parentIndex].parent)
        {
            if (IsNodeAdditive(&anims->entries[parentIndex]))
                Com_Error(ERR_DROP, "Do not nest additives");
        }
    }
    if (anims->debugAnimNames)
    {
        iassert(!anims->debugAnimNames[animIndex]);
        debugName = (char*)Hunk_AllocDebugMem(strlen(name) + 1, "XAnimBlend");
        v8 = name;
        v7 = debugName;
        do
        {
            v6 = *v8;
            *v7++ = *v8++;
        } while (v6);
        anims->debugAnimNames[animIndex] = debugName;
    }
}

bool __cdecl IsNodeAdditive(const XAnimEntry* node)
{
    iassert(node);

    return !IsLeafNode(node) && (node->animParent.flags & 0x10) != 0;
}

bool __cdecl IsLeafNode(const XAnimEntry* anim)
{
    return anim->numAnims == 0;
}

XAnim_s* __cdecl XAnimCreateAnims(const char* debugName, unsigned int size, void* (__cdecl* Alloc)(int))
{
    char v4; // [esp+3h] [ebp-29h]
    char* v5; // [esp+8h] [ebp-24h]
    const char* v6; // [esp+Ch] [ebp-20h]
    char* newDebugName; // [esp+20h] [ebp-Ch]
    XAnim_s* anims; // [esp+28h] [ebp-4h]

    iassert(debugName);
    iassert(Alloc);

    anims = (XAnim_s*)Alloc(8 * size + 12);
    anims->size = size;

    if (g_anim_developer)
    {
        newDebugName = (char*)Hunk_AllocDebugMem(strlen(debugName) + 1, "XAnimCreateAnims");
        v6 = debugName;
        v5 = newDebugName;
        do
        {
            v4 = *v6;
            *v5++ = *v6++;
        } while (v4);
        anims->debugName = newDebugName;
        anims->debugAnimNames = (const char**)Hunk_AllocDebugMem(4 * size, "XAnimCreateAnims");
    }

    if (Hunk_DataOnHunk((unsigned char*)anims))
        Hunk_AddData(2, anims, Alloc);

    return anims;
}

void __cdecl XAnimFree(XAnimParts *parts)
{
    int size; // [esp+0h] [ebp-14h]
    XAnimNotifyInfo *notify; // [esp+4h] [ebp-10h]
    int i; // [esp+8h] [ebp-Ch]
    __int16 notifyInfoIndex; // [esp+Ch] [ebp-8h]
    unsigned __int16 *boneNames; // [esp+10h] [ebp-4h]

    boneNames = parts->names;
    size = parts->boneCount[9];
    for (i = 0; i < size; ++i)
        SL_RemoveRefToString(boneNames[i]);
    notifyInfoIndex = 0;
    notify = parts->notify;
    while (notifyInfoIndex < parts->notifyCount)
    {
        SL_RemoveRefToString(notify->name);
        ++notifyInfoIndex;
        ++notify;
    }
}

void __cdecl XAnimFreeList(XAnim_s* anims)
{
    unsigned int i; // [esp+0h] [ebp-4h]

    if (anims->debugName)
    {
        Hunk_FreeDebugMem((void*)anims->debugName);
        anims->debugName = 0;
    }
    if (anims->debugAnimNames)
    {
        for (i = 0; i < anims->size; ++i)
        {
            if (anims->debugAnimNames[i])
            {
                Hunk_FreeDebugMem((void*)anims->debugAnimNames[i]);
                anims->debugAnimNames[i] = 0;
            }
        }
        Hunk_FreeDebugMem(anims->debugAnimNames);
        anims->debugAnimNames = 0;
    }
}

XAnimTree_s* __cdecl XAnimCreateTree(XAnim_s* anims, void* (__cdecl* Alloc)(int))
{
    XAnimTree_s* tree; // [esp+0h] [ebp-Ch]

    iassert(anims);
    iassert(anims->size);

    tree = (XAnimTree_s*)Alloc(sizeof(XAnimTree_s));
    memset(tree, 0, sizeof(XAnimTree_s));
    tree->anims = anims;
    iassert(!tree->info_usage);
    return tree;
}

void __cdecl XAnimFreeTree(XAnimTree_s* tree, void(__cdecl* Free)(void*, int))
{
    iassert(tree);
    iassert(tree->anims);
    iassert(tree->anims->size);

    XAnimClearTree(tree);
    iassert(!tree->info_usage);
    if (Free)
    {
        Free(tree, sizeof(XAnimTree_s));
    }
}

bool g_disableLeakCheck;
void XAnimCheckTreeLeak()
{
    if (!g_disableLeakCheck)
    {
        iassert(g_info_usage == 1);
    }
}

XAnim_s* __cdecl XAnimGetAnims(const XAnimTree_s* tree)
{
    iassert(tree);

    return tree->anims;
}

void XAnimResetAnimMap(const DObj_s *obj, unsigned int infoIndex)
{
    XModelNameMap modelMap[256]; // [sp+50h] [-410h] BYREF

    iassert(obj->numModels < 256); // lwss add

    PROF_SCOPED("XAnimSetModel");
    XAnimInitModelMap(obj->models, obj->numModels, modelMap);
    XAnimResetAnimMap_r(modelMap, infoIndex);
}

void __cdecl XAnimInitModelMap(XModel* const* models, unsigned int numModels, XModelNameMap* modelMap)
{
    unsigned int boneIndex; // [esp+0h] [ebp-20h]
    unsigned __int16 boneName; // [esp+4h] [ebp-1Ch]
    unsigned int hash; // [esp+8h] [ebp-18h]
    XModel* model; // [esp+Ch] [ebp-14h]
    unsigned int boneCount; // [esp+10h] [ebp-10h]
    unsigned int localBoneIndex; // [esp+14h] [ebp-Ch]
    unsigned int i; // [esp+18h] [ebp-8h]
    unsigned const __int16* boneNames; // [esp+1Ch] [ebp-4h]

    memset((unsigned __int8*)modelMap, 0, 1024);
    boneIndex = 0;

    for (i = 0; i < numModels; ++i)
    {
        model = models[i];
        boneNames = model->boneNames;
        boneCount = model->numBones;

        iassert(boneCount < DOBJ_MAX_PARTS);

        for (localBoneIndex = 0; localBoneIndex < boneCount; ++localBoneIndex)
        {
            boneName = boneNames[localBoneIndex];
            iassert(boneName);

            for (hash = (unsigned __int8)boneName; modelMap[hash].name; hash = (unsigned __int8)(hash + 1));

            modelMap[hash].index = boneIndex;
            modelMap[hash].name = boneName;
            ++boneIndex;
        }
    }
}

void __cdecl XAnimResetAnimMap_r(XModelNameMap* modelMap, unsigned int infoIndex)
{
    XAnimInfo* info; // [esp+0h] [ebp-8h]
    unsigned int childInfoIndex; // [esp+4h] [ebp-4h]

    iassert(infoIndex && (infoIndex < 4096));
    info = &g_xAnimInfo[infoIndex];
    iassert(info->inuse);
    if (info->animToModel)
    {
        iassert(!info->children);
        XAnimResetAnimMapLeaf(modelMap, infoIndex);
    }
    else
    {
        for (childInfoIndex = info->children; childInfoIndex; childInfoIndex = g_xAnimInfo[childInfoIndex].next)
        {
            iassert(childInfoIndex && (childInfoIndex < 4096));
            iassert(g_xAnimInfo[childInfoIndex].inuse);
            XAnimResetAnimMap_r(modelMap, childInfoIndex);
        }
    }
}

void __cdecl XAnimResetAnimMapLeaf(const XModelNameMap* modelMap, unsigned int infoIndex)
{
    char* animToModel2; // [esp+4h] [ebp-8h]
    unsigned int animToModel; // [esp+8h] [ebp-4h]

    iassert((infoIndex && (infoIndex < 4096)));
    iassert(g_xAnimInfo[infoIndex].inuse);
    animToModel = g_xAnimInfo[infoIndex].animToModel;
    iassert(animToModel);
    animToModel2 = SL_ConvertToString(animToModel);
    g_xAnimInfo[infoIndex].animToModel = XAnimGetAnimMap(g_xAnimInfo[infoIndex].parts, modelMap);
    SL_RemoveRefToStringOfSize(animToModel, (unsigned __int8)animToModel2[16] + 17);
}

HashEntry_unnamed_type_u __cdecl XAnimGetAnimMap(const XAnimParts* parts, const XModelNameMap* modelMap)
{
    unsigned int boneIndex; // [esp+0h] [ebp-BCh]
    unsigned int hash; // [esp+8h] [ebp-B4h]
    unsigned int partIndex; // [esp+Ch] [ebp-B0h]
    unsigned int boneCount; // [esp+10h] [ebp-ACh]
    XAnimToXModel animToModel; // [esp+14h] [ebp-A8h] BYREF
    unsigned __int16* partNames; // [esp+B4h] [ebp-8h]
    unsigned int partName; // [esp+B8h] [ebp-4h]

    if (!parts)
        MyAssertHandler(".\\xanim\\xanim.cpp", 575, 0, "%s", "parts");
    memset(&animToModel, 0, 16);
    boneCount = parts->boneCount[9];
    partNames = parts->names;
    for (partIndex = 0; partIndex < boneCount; ++partIndex)
    {
        partName = partNames[partIndex];
        for (hash = (unsigned __int8)partName; ; hash = (unsigned __int8)(hash + 1))
        {
            if (!modelMap[hash].name)
            {
                animToModel.boneIndex[partIndex] = 127;
                goto LABEL_4;
            }
            if (partName == modelMap[hash].name)
                break;
        }
        boneIndex = modelMap[hash].index;
        animToModel.boneIndex[partIndex] = boneIndex;
        //bitarray<128>::setBit(&animToModel.partBits, boneIndex);
        animToModel.partBits.setBit(boneIndex);
    LABEL_4:
        ;
    }
    animToModel.boneCount = boneCount;
    if ((unsigned __int8)boneCount != boneCount)
        MyAssertHandler(".\\xanim\\xanim.cpp", 606, 0, "%s", "animToModel.boneCount == boneCount");
    return SL_GetStringOfSize((char*)&animToModel, 0, boneCount + 17, 11);
}

double __cdecl XAnimGetLength(const XAnim_s* anims, unsigned int animIndex)
{
    XAnimParts* parts; // [esp+Ch] [ebp-4h]

    if (!anims)
        MyAssertHandler(".\\xanim\\xanim.cpp", 2395, 0, "%s", "anims");
    if (animIndex >= anims->size)
        MyAssertHandler(".\\xanim\\xanim.cpp", 2396, 0, "animIndex < anims->size\n\t%i, %i", animIndex, anims->size);
    if ((const XAnim_s*)((char*)anims + 8 * animIndex) == (const XAnim_s*)-12)
        MyAssertHandler(".\\xanim\\xanim.cpp", 2399, 0, "%s", "entry");
    if (!IsLeafNode(&anims->entries[animIndex]))
        MyAssertHandler(".\\xanim\\xanim.cpp", 2400, 0, "%s", "IsLeafNode( entry )");
    parts = anims->entries[animIndex].parts;
    if (!parts)
        MyAssertHandler(".\\xanim\\xanim.cpp", 2403, 0, "%s", "parts");
    return (float)((double)parts->numframes / parts->framerate);
}

int __cdecl XAnimGetLengthMsec(const XAnim_s* anims, unsigned int anim)
{
    return (int)(XAnimGetLength(anims, anim) * 1000.0);
}

double __cdecl XAnimGetTime(const XAnimTree_s* tree, unsigned int animIndex)
{
    unsigned int infoIndex; // [esp+4h] [ebp-4h]

    if (!tree)
        MyAssertHandler(".\\xanim\\xanim.cpp", 2481, 0, "%s", "tree");
    if (!tree->anims)
        MyAssertHandler(".\\xanim\\xanim.cpp", 2482, 0, "%s", "tree->anims");
    if (animIndex >= tree->anims->size)
        MyAssertHandler(
            ".\\xanim\\xanim.cpp",
            2483,
            0,
            "animIndex < tree->anims->size\n\t%i, %i",
            animIndex,
            tree->anims->size);
    infoIndex = XAnimGetInfoIndex(tree, animIndex);
    if (infoIndex >= 0x1000)
        MyAssertHandler(".\\xanim\\xanim.cpp", 2486, 0, "%s\n\t(infoIndex) = %i", "(infoIndex < 4096)", infoIndex);
    if (infoIndex)
        return g_xAnimInfo[infoIndex].state.currentAnimTime;
    else
        return 0.0f;
}

unsigned int __cdecl XAnimGetInfoIndex(const XAnimTree_s* tree, unsigned int animIndex)
{
    if (!tree)
        MyAssertHandler(".\\xanim\\xanim.cpp", 2466, 0, "%s", "tree");
    if (tree->children)
        return XAnimGetInfoIndex_r(tree, animIndex, tree->children);
    else
        return 0;
}

unsigned int __cdecl XAnimGetInfoIndex_r(const XAnimTree_s* tree, unsigned int animIndex, unsigned int infoIndex)
{
    XAnimInfo* info; // [esp+0h] [ebp-10h]
    unsigned int prevAnimIndex; // [esp+4h] [ebp-Ch]
    unsigned int nextAnimIndex; // [esp+8h] [ebp-8h]
    unsigned int resultInfoIndex; // [esp+Ch] [ebp-4h]
    unsigned int infoIndexa; // [esp+20h] [ebp+10h]

    if (!infoIndex || infoIndex >= 0x1000)
        MyAssertHandler(
            ".\\xanim\\xanim.cpp",
            2433,
            0,
            "%s\n\t(infoIndex) = %i",
            "(infoIndex && (infoIndex < 4096))",
            infoIndex);
    info = &g_xAnimInfo[infoIndex];
    if (!info->inuse)
        MyAssertHandler(".\\xanim\\xanim.cpp", 2436, 0, "%s", "info->inuse");
    if (info->animIndex == animIndex)
        return infoIndex;
    prevAnimIndex = 0;
    for (infoIndexa = info->children; infoIndexa; infoIndexa = g_xAnimInfo[infoIndexa].next)
    {
        nextAnimIndex = g_xAnimInfo[infoIndexa].animIndex;
        if (nextAnimIndex != prevAnimIndex)
        {
            resultInfoIndex = XAnimGetInfoIndex_r(tree, animIndex, infoIndexa);
            if (resultInfoIndex)
                return resultInfoIndex;
            prevAnimIndex = nextAnimIndex;
        }
    }
    return 0;
}

double __cdecl XAnimGetWeight(const XAnimTree_s* tree, unsigned int animIndex)
{
    unsigned int infoIndex; // [esp+4h] [ebp-4h]

    if (!tree)
        MyAssertHandler(".\\xanim\\xanim.cpp", 2500, 0, "%s", "tree");
    if (!tree->anims)
        MyAssertHandler(".\\xanim\\xanim.cpp", 2501, 0, "%s", "tree->anims");
    if (animIndex >= tree->anims->size)
        MyAssertHandler(
            ".\\xanim\\xanim.cpp",
            2502,
            0,
            "animIndex < tree->anims->size\n\t%i, %i",
            animIndex,
            tree->anims->size);
    infoIndex = XAnimGetInfoIndex(tree, animIndex);
    if (infoIndex >= 0x1000)
        MyAssertHandler(".\\xanim\\xanim.cpp", 2505, 0, "%s\n\t(infoIndex) = %i", "(infoIndex < 4096)", infoIndex);
    if (infoIndex)
        return g_xAnimInfo[infoIndex].state.weight;
    else
        return (float)0.0;
}

bool __cdecl XAnimHasFinished(const XAnimTree_s* tree, unsigned int animIndex)
{
    XAnimState* state; // [esp+4h] [ebp-8h]
    unsigned int infoIndex; // [esp+8h] [ebp-4h]

    if (!tree)
        MyAssertHandler(".\\xanim\\xanim.cpp", 2520, 0, "%s", "tree");
    if (!tree->anims)
        MyAssertHandler(".\\xanim\\xanim.cpp", 2521, 0, "%s", "tree->anims");
    if (animIndex >= tree->anims->size)
        MyAssertHandler(
            ".\\xanim\\xanim.cpp",
            2522,
            0,
            "animIndex < tree->anims->size\n\t%i, %i",
            animIndex,
            tree->anims->size);
    infoIndex = XAnimGetInfoIndex(tree, animIndex);
    if (!infoIndex)
        return 1;
    if (infoIndex >= 0x1000)
        MyAssertHandler(
            ".\\xanim\\xanim.cpp",
            2528,
            0,
            "%s\n\t(infoIndex) = %i",
            "(infoIndex && (infoIndex < 4096))",
            infoIndex);
    state = &g_xAnimInfo[infoIndex].state;
    return state->oldTime > (double)state->currentAnimTime
        || state->currentAnimTime == 1.0
        || state->cycleCount > state->oldCycleCount;
}

int __cdecl XAnimGetNumChildren(const XAnim_s* anims, unsigned int animIndex)
{
    if (!anims)
        MyAssertHandler(".\\xanim\\xanim.cpp", 2541, 0, "%s", "anims");
    if (animIndex >= anims->size)
        MyAssertHandler(".\\xanim\\xanim.cpp", 2542, 0, "animIndex < anims->size\n\t%i, %i", animIndex, anims->size);
    return anims->entries[animIndex].numAnims;
}

unsigned int __cdecl XAnimGetChildAt(const XAnim_s* anims, unsigned int animIndex, unsigned int childIndex)
{
    if (!anims)
        MyAssertHandler(".\\xanim\\xanim.cpp", 2555, 0, "%s", "anims");
    if (animIndex >= anims->size)
        MyAssertHandler(".\\xanim\\xanim.cpp", 2556, 0, "animIndex < anims->size\n\t%i, %i", animIndex, anims->size);
    if (childIndex >= anims->entries[animIndex].numAnims)
        MyAssertHandler(
            ".\\xanim\\xanim.cpp",
            2557,
            0,
            "childIndex < anims->entries[animIndex].numAnims\n\t%i, %i",
            childIndex,
            anims->entries[animIndex].numAnims);
    return childIndex + anims->entries[animIndex].animParent.children;
}

const char* __cdecl XAnimGetAnimName(const XAnim_s* anims, unsigned int animIndex)
{
    iassert(anims);
    iassert(animIndex < anims->size);

    if (IsLeafNode(&anims->entries[animIndex]))
        return anims->entries[animIndex].parts->name;
    else
        return "";
}

char* __cdecl XAnimGetAnimDebugName(const XAnim_s* anims, unsigned int animIndex)
{
    bool isDefault; // [esp+Fh] [ebp-15h]
    XAnimParts* parts; // [esp+10h] [ebp-14h]
    const char* debugName; // [esp+18h] [ebp-Ch]
    const XAnimEntry* anim; // [esp+1Ch] [ebp-8h]

    if (!anims)
        MyAssertHandler(".\\xanim\\xanim.cpp", 2602, 0, "%s", "anims");
    if (animIndex >= anims->size)
        MyAssertHandler(".\\xanim\\xanim.cpp", 2603, 0, "animIndex < anims->size\n\t%i, %i", animIndex, anims->size);
    anim = &anims->entries[animIndex];
    if (anims->debugAnimNames)
    {
        if (!anims->debugAnimNames[animIndex])
            MyAssertHandler(".\\xanim\\xanim.cpp", 2608, 0, "%s", "anims->debugAnimNames[animIndex]");
        debugName = anims->debugAnimNames[animIndex];
        if (IsLeafNode(anim)
            && ((parts = anims->entries[animIndex].parts, !useFastFile->current.enabled)
                ? (isDefault = parts->isDefault)
                : (isDefault = DB_IsXAssetDefault(ASSET_TYPE_XANIMPARTS, parts->name)),
                isDefault))
        {
            return va("^3%s (missing)", debugName);
        }
        else
        {
            return (char*)debugName;
        }
    }
    else if (IsLeafNode(anim))
    {
        return (char*)anims->entries[animIndex].parts->name;
    }
    else
    {
        return va("%i", animIndex);
    }
}

const char* __cdecl XAnimGetAnimTreeDebugName(const XAnim_s* anims)
{
    if (!anims)
        MyAssertHandler(".\\xanim\\xanim.cpp", 2622, 0, "%s", "anims");
    return anims->debugName;
}

unsigned int __cdecl XAnimGetAnimTreeSize(const XAnim_s* anims)
{
    if (!anims)
        MyAssertHandler(".\\xanim\\xanim.cpp", 2634, 0, "%s", "anims");
    return anims->size;
}

void __cdecl XAnimInitInfo(XAnimInfo* info)
{
    info->state.currentAnimTime = 0.0;
    info->state.oldTime = 0.0;
    *(_DWORD*)&info->state.cycleCount = 0;
    info->state.goalTime = 0.0;
    info->state.goalWeight = 0.0;
    info->state.weight = 0.0;
    info->state.rate = 0.0;
    *(_DWORD*)&info->state.instantWeightChange = 0;
    info->notifyName = 0;
    info->notifyIndex = -1;
    info->notifyChild = 0;
    info->notifyType = 0;
}

void __cdecl XAnimUpdateOldTime(
    DObj_s* obj,
    unsigned int infoIndex,
    XAnimState* syncState,
    float dtime,
    bool parentHasWeight,
    bool* childHasTimeForParent)
{
    bool v6; // [esp+10h] [ebp-20h]
    XAnimState* state; // [esp+14h] [ebp-1Ch]
    unsigned int nextInfoIndex; // [esp+18h] [ebp-18h]
    XAnimInfo* info; // [esp+1Ch] [ebp-14h]
    XAnimTree_s* tree; // [esp+20h] [ebp-10h]
    bool childHasTime; // [esp+27h] [ebp-9h] BYREF
    unsigned int childInfoIndex; // [esp+28h] [ebp-8h]
    XAnimParts* parts; // [esp+2Ch] [ebp-4h]

    tree = obj->tree;
    if (!obj->tree)
        MyAssertHandler(".\\xanim\\xanim.cpp", 1667, 0, "%s", "tree");
    if (!tree->anims)
        MyAssertHandler(".\\xanim\\xanim.cpp", 1668, 0, "%s", "tree->anims");
    if (dtime < 0.0)
        MyAssertHandler(".\\xanim\\xanim.cpp", 1669, 0, "%s\n\t(dtime) = %g", "(dtime >= 0)", dtime);
    if (!infoIndex || infoIndex >= 0x1000)
        MyAssertHandler(
            ".\\xanim\\xanim.cpp",
            1671,
            0,
            "%s\n\t(infoIndex) = %i",
            "(infoIndex && (infoIndex < 4096))",
            infoIndex);
    info = &g_xAnimInfo[infoIndex];
    if (!info->inuse)
        MyAssertHandler(".\\xanim\\xanim.cpp", 1674, 0, "%s", "info->inuse");
    state = &info->state;
    if (parentHasWeight && dtime < (double)info->state.goalTime)
    {
        info->state.weight = (info->state.goalWeight - info->state.weight) * dtime / info->state.goalTime
            + info->state.weight;
        if (info->state.weight < 0.00000100000011116208)
            info->state.weight = info->state.goalWeight * EQUAL_EPSILON;
        info->state.goalTime = info->state.goalTime - dtime;
    }
    else
    {
        info->state.weight = info->state.goalWeight;
        info->state.goalTime = 0.0;
    }
    v6 = parentHasWeight && info->state.weight != 0.0;
    info->state.instantWeightChange = 0;
    if (!info->inuse)
        MyAssertHandler(".\\xanim\\xanim.cpp", 1695, 0, "%s", "info->inuse");
    if (info->animToModel)
    {
        parts = info->parts;
        if (!parts)
            MyAssertHandler(".\\xanim\\xanim.cpp", 1701, 0, "%s", "parts");
        childHasTime = parts->frequency != 0.0;
    }
    else
    {
        childHasTime = 0;
        if ((info->animParent.flags & 4) != 0)
            syncState = &info->state;
        for (childInfoIndex = info->children; childInfoIndex; childInfoIndex = nextInfoIndex)
        {
            nextInfoIndex = g_xAnimInfo[childInfoIndex].next;
            XAnimUpdateOldTime(obj, childInfoIndex, syncState, dtime, v6, &childHasTime);
        }
    }
    if (v6 && childHasTime)
    {
        *childHasTimeForParent = 1;
    }
    else if (info->animToModel || (info->animParent.flags & 4) == 0)
    {
        if (syncState->currentAnimTime != state->currentAnimTime || info->state.cycleCount != syncState->cycleCount)
        {
            state->currentAnimTime = syncState->currentAnimTime;
            info->state.cycleCount = syncState->cycleCount;
            info->notifyIndex = -1;
        }
    }
    else
    {
        XAnimInitTime(tree, infoIndex, 0.0);
    }
    info->state.oldTime = info->state.currentAnimTime;
    info->state.oldCycleCount = info->state.cycleCount;
}

unsigned int __cdecl XAnimInitTime(XAnimTree_s* tree, unsigned int infoIndex, float goalTime)
{
    unsigned int toInfoIndex; // [esp+10h] [ebp-4h]

    if (!infoIndex || infoIndex >= 0x1000)
        MyAssertHandler(
            ".\\xanim\\xanim.cpp",
            1632,
            0,
            "%s\n\t(infoIndex) = %i",
            "(infoIndex && (infoIndex < 4096))",
            infoIndex);
    if (g_xAnimInfo[infoIndex].state.currentAnimTime == 0.0 && !g_xAnimInfo[infoIndex].state.cycleCount)
        return infoIndex;
    if (g_xAnimInfo[infoIndex].state.cycleCount != g_xAnimInfo[infoIndex].state.oldCycleCount)
        return infoIndex;
    if (goalTime == 0.0 || g_xAnimInfo[infoIndex].state.weight == 0.0)
    {
        XAnimResetTime(infoIndex);
        return infoIndex;
    }
    else
    {
        toInfoIndex = XAnimCloneInitTime(tree, infoIndex, g_xAnimInfo[infoIndex].parent);
        XAnimClearTreeGoalWeightsInternal(tree, infoIndex, goalTime, 1);
        return toInfoIndex;
    }
}

void __cdecl XAnimResetTime(unsigned int infoIndex)
{
    unsigned int childInfoIndex; // [esp+0h] [ebp-4h]

    XAnimResetTimeInternal(infoIndex);
    for (childInfoIndex = g_xAnimInfo[infoIndex].children; childInfoIndex; childInfoIndex = g_xAnimInfo[childInfoIndex].next)
        XAnimResetTime(childInfoIndex);
}

void __cdecl XAnimResetTimeInternal(unsigned int infoIndex)
{
    XAnimState* state; // [esp+0h] [ebp-8h]

    if (!infoIndex || infoIndex >= 0x1000)
        MyAssertHandler(
            ".\\xanim\\xanim.cpp",
            1570,
            0,
            "%s\n\t(infoIndex) = %i",
            "(infoIndex && (infoIndex < 4096))",
            infoIndex);
    state = &g_xAnimInfo[infoIndex].state;
    state->currentAnimTime = 0.0;
    state->cycleCount = 0;
    state->oldTime = 0.0;
    state->oldCycleCount = 0;
    g_xAnimInfo[infoIndex].notifyIndex = -1;
}

unsigned int __cdecl XAnimCloneInitTime(XAnimTree_s* tree, unsigned int infoIndex, unsigned int parentIndex)
{
    XAnimInfo* toInfo; // [esp+0h] [ebp-14h]
    unsigned int toInfoIndex; // [esp+4h] [ebp-10h]
    XAnimInfo* fromInfo; // [esp+8h] [ebp-Ch]
    unsigned int animToModel; // [esp+Ch] [ebp-8h]
    unsigned int childInfoIndex; // [esp+10h] [ebp-4h]

    if (!infoIndex || infoIndex >= 0x1000)
        MyAssertHandler(
            ".\\xanim\\xanim.cpp",
            1603,
            0,
            "%s\n\t(infoIndex) = %i",
            "(infoIndex && (infoIndex < 4096))",
            infoIndex);
    fromInfo = &g_xAnimInfo[infoIndex];
    animToModel = fromInfo->animToModel;
    if (fromInfo->animToModel)
        SL_AddRefToString(animToModel);
    toInfoIndex = XAnimAllocInfoWithParent(tree, animToModel, fromInfo->animIndex, parentIndex, 0);
    toInfo = &g_xAnimInfo[toInfoIndex];
    XAnimCloneAnimInfo(fromInfo, toInfo);
    XAnimResetTimeInternal(toInfoIndex);
    toInfo->state.weight = 0.0;
    toInfo->state.instantWeightChange = 1;
    for (childInfoIndex = g_xAnimInfo[infoIndex].children; childInfoIndex; childInfoIndex = g_xAnimInfo[childInfoIndex].next)
        XAnimCloneInitTime(tree, childInfoIndex, toInfoIndex);
    return toInfoIndex;
}

void __cdecl DObjInitServerTime(DObj_s* obj, float dtime)
{
    XAnimState syncState; // [esp+58h] [ebp-28h] BYREF
    XAnimTree_s* tree; // [esp+78h] [ebp-8h]
    bool childHasTime; // [esp+7Fh] [ebp-1h] BYREF

    PROF_SCOPED("DObjInitServerTime");
    iassert(obj);

    tree = obj->tree;
    if (tree && tree->children)
    {
        syncState.currentAnimTime = 0.0;
        syncState.cycleCount = 0;
        XAnimUpdateOldTime(obj, tree->children, &syncState, dtime, 1, &childHasTime);
    }
}

void __cdecl DObjUpdateClientInfo(DObj_s* obj, float dtime, bool notify)
{
    XAnimState syncState; // [esp+58h] [ebp-28h] BYREF
    XAnimTree_s* tree; // [esp+78h] [ebp-8h]
    bool childHasTime; // [esp+7Fh] [ebp-1h] BYREF

    PROF_SCOPED("DObjUpdateClientInfo");

    iassert(obj);
    iassert(dtime >= 0);
    iassert(Sys_IsMainThread());

    g_notifyListSize = 0;
    tree = obj->tree;

    if (tree && tree->children)
    {
        syncState.currentAnimTime = 0.0;
        syncState.cycleCount = 0;
        XAnimUpdateOldTime(obj, tree->children, &syncState, dtime, 1, &childHasTime);
        if (tree->children)
            XAnimUpdateTimeAndNotetrack(obj, tree->children, dtime, notify);
    }
}

void __cdecl XAnimUpdateTimeAndNotetrack(const DObj_s* obj, unsigned int infoIndex, float dtime, bool bNotify)
{
    unsigned int nextInfoIndex; // [esp+Ch] [ebp-Ch]
    XAnimInfo* info; // [esp+10h] [ebp-8h]
    XAnimTree_s* tree; // [esp+14h] [ebp-4h]
    unsigned int infoIndexa; // [esp+24h] [ebp+Ch]
    float dtimea; // [esp+28h] [ebp+10h]
    float dtimeb; // [esp+28h] [ebp+10h]

    tree = obj->tree;
    if (!obj->tree)
        MyAssertHandler(".\\xanim\\xanim.cpp", 1363, 0, "%s", "tree");
    if (dtime < 0.0)
        MyAssertHandler(".\\xanim\\xanim.cpp", 1365, 0, "%s\n\t(dtime) = %g", "(dtime >= 0)", dtime);
    if (!infoIndex || infoIndex >= 0x1000)
        MyAssertHandler(
            ".\\xanim\\xanim.cpp",
            1366,
            0,
            "%s\n\t(infoIndex) = %i",
            "(infoIndex && (infoIndex < 4096))",
            infoIndex);
    info = &g_xAnimInfo[infoIndex];
    if (!info->inuse)
        MyAssertHandler(".\\xanim\\xanim.cpp", 1369, 0, "%s", "info->inuse");
    if (info->state.weight == 0.0)
    {
        XAnimCheckFreeInfo(obj->tree, infoIndex, 0);
        return;
    }
    if (info->state.goalWeight == 0.0)
        bNotify = 0;
    if (info->animToModel)
    {
        XAnimUpdateTimeAndNotetrackLeaf(obj, info->parts, infoIndex, dtime, bNotify);
        return;
    }
    if ((info->animParent.flags & 3) != 0)
    {
        dtimea = XAnimGetAverageRateFrequency(tree, infoIndex) * info->state.rate * dtime;
        if (dtimea == 0.0)
        {
        LABEL_18:
            XAnimCheckFreeInfo(obj->tree, infoIndex, 1);
            return;
        }
        XAnimUpdateTimeAndNotetrackSyncSubTree(obj, infoIndex, dtimea, bNotify);
    }
    else
    {
        if ((info->animParent.flags & 4) != 0)
            MyAssertHandler(".\\xanim\\xanim.cpp", 1401, 0, "%s", "!(info->animParent.flags & XANIM_SYNC_ROOT)");
        if (obj->entnum && info->notifyName)
            MyAssertHandler(".\\xanim\\xanim.cpp", 1402, 0, "%s", "!(obj->entnum && info->notifyName)");
        dtimeb = dtime * info->state.rate;
        if (dtimeb == 0.0)
            goto LABEL_18;
        for (infoIndexa = info->children; infoIndexa; infoIndexa = nextInfoIndex)
        {
            nextInfoIndex = g_xAnimInfo[infoIndexa].next;
            XAnimUpdateTimeAndNotetrack(obj, infoIndexa, dtimeb, bNotify);
        }
    }
}

void __cdecl XAnimCheckFreeInfo(XAnimTree_s* tree, unsigned int infoIndex, int hasWeight)
{
    unsigned int nextInfoIndex; // [esp+4h] [ebp-Ch]
    XAnimInfo* info; // [esp+8h] [ebp-8h]
    unsigned int childInfoIndex; // [esp+Ch] [ebp-4h]

    if (!infoIndex || infoIndex >= 0x1000)
        MyAssertHandler(
            ".\\xanim\\xanim.cpp",
            821,
            0,
            "%s\n\t(infoIndex) = %i",
            "(infoIndex && (infoIndex < 4096))",
            infoIndex);
    info = &g_xAnimInfo[infoIndex];
    if (info->state.weight == 0.0)
        hasWeight = 0;
    for (childInfoIndex = info->children; childInfoIndex; childInfoIndex = nextInfoIndex)
    {
        nextInfoIndex = g_xAnimInfo[childInfoIndex].next;
        XAnimCheckFreeInfo(tree, childInfoIndex, hasWeight);
    }
    if (!info->children && !hasWeight && g_xAnimInfo[infoIndex].state.goalWeight == 0.0)
        XAnimFreeInfo(tree, infoIndex);
}

void __cdecl XAnimFreeInfo(XAnimTree_s* tree, unsigned int infoIndex)
{
    XAnimInfo* info; // [esp+0h] [ebp-14h]
    unsigned int next; // [esp+4h] [ebp-10h]
    char* animToModel; // [esp+8h] [ebp-Ch]
    unsigned int prev; // [esp+Ch] [ebp-8h]

    InterlockedIncrement(&tree->modifyRefCount);
    if (tree->calcRefCount)
        MyAssertHandler(".\\xanim\\xanim.cpp", 732, 0, "%s", "!tree->calcRefCount");
    if (!tree)
        MyAssertHandler(".\\xanim\\xanim.cpp", 735, 0, "%s", "tree");
    if (!infoIndex || infoIndex >= 0x1000)
        MyAssertHandler(
            ".\\xanim\\xanim.cpp",
            737,
            0,
            "%s\n\t(infoIndex) = %i",
            "(infoIndex && (infoIndex < 4096))",
            infoIndex);
    info = &g_xAnimInfo[infoIndex];
    if (info->tree != tree)
        MyAssertHandler(".\\xanim\\xanim.cpp", 741, 0, "%s", "info->tree == tree");
    if (!info->inuse)
        MyAssertHandler(".\\xanim\\xanim.cpp", 742, 0, "%s", "info->inuse");
    info->inuse = 0;
    if (info->animToModel)
    {
        if (info->children)
            MyAssertHandler(".\\xanim\\xanim.cpp", 748, 0, "%s", "!info->children");
        if (!info->parts)
            MyAssertHandler(".\\xanim\\xanim.cpp", 751, 0, "%s", "parts");
        if (!info->animToModel)
            MyAssertHandler(".\\xanim\\xanim.cpp", 753, 0, "%s", "info->animToModel");
        animToModel = SL_ConvertToString(info->animToModel);
        SL_RemoveRefToStringOfSize(info->animToModel, (unsigned __int8)animToModel[16] + 17);
        info->animToModel = 0;
    }
    else
    {
        while (info->children)
            XAnimFreeInfo(tree, info->children);
    }
    prev = info->prev;
    next = info->next;
    if (info->prev)
    {
        g_xAnimInfo[prev].next = next;
    }
    else if (info->parent)
    {
        g_xAnimInfo[info->parent].children = info->next;
    }
    else
    {
        tree->children = info->next;
    }
    if (next)
        g_xAnimInfo[next].prev = prev;
    XAnimClearServerNotify(info);
    info->prev = 0;
    info->next = g_xAnimInfo[0].next;
    g_xAnimInfo[g_xAnimInfo[0].next].prev = infoIndex;
    g_xAnimInfo[0].next = infoIndex;
    if (!g_info_usage)
        MyAssertHandler(".\\xanim\\xanim.cpp", 796, 0, "%s", "g_info_usage");
    --g_info_usage;
    if (!tree->info_usage)
        MyAssertHandler(".\\xanim\\xanim.cpp", 799, 0, "%s", "tree->info_usage");
    --tree->info_usage;
    if (tree->calcRefCount)
        MyAssertHandler(".\\xanim\\xanim.cpp", 808, 0, "%s", "!tree->calcRefCount");
    InterlockedDecrement(&tree->modifyRefCount);
}

void __cdecl XAnimClearServerNotify(XAnimInfo* info)
{
    if (info->notifyName)
    {
        SL_RemoveRefToString(info->notifyName);
        info->notifyName = 0;
    }
    info->notifyIndex = -1;
}

double __cdecl XAnimGetAverageRateFrequency(const XAnimTree_s *tree, unsigned int infoIndex)
{
    const XAnimInfo *info; // [esp+14h] [ebp-18h]
    const XAnimInfo *infoa; // [esp+14h] [ebp-18h]
    float totalDtime; // [esp+18h] [ebp-14h]
    float totalWeight; // [esp+1Ch] [ebp-10h]
    float weight; // [esp+20h] [ebp-Ch]
    float frequency; // [esp+24h] [ebp-8h]
    const XAnimParts *parts; // [esp+28h] [ebp-4h]
    unsigned int infoIndexa; // [esp+38h] [ebp+Ch]

    if (!tree)
        MyAssertHandler(".\\xanim\\xanim.cpp", 853, 0, "%s", "tree");
    if (!tree->anims)
        MyAssertHandler(".\\xanim\\xanim.cpp", 854, 0, "%s", "tree->anims");
    info = &g_xAnimInfo[infoIndex];
    if (!info->inuse)
        MyAssertHandler(".\\xanim\\xanim.cpp", 858, 0, "%s", "info->inuse");
    if (info->animToModel)
    {
        parts = info->parts;
        if (!parts)
            MyAssertHandler(".\\xanim\\xanim.cpp", 864, 0, "%s", "parts");
        return parts->frequency;
    }
    else
    {
        totalWeight = 0.0;
        totalDtime = 0.0;
        for (infoIndexa = info->children; infoIndexa; infoIndexa = infoa->next)
        {
            if (infoIndexa >= 0x1000)
                MyAssertHandler(
                    ".\\xanim\\xanim.cpp",
                    873,
                    0,
                    "%s\n\t(infoIndex) = %i",
                    "(infoIndex && (infoIndex < 4096))",
                    infoIndexa);
            infoa = &g_xAnimInfo[infoIndexa];
            weight = infoa->state.weight;
            if (weight < 0.0)
                MyAssertHandler(".\\xanim\\xanim.cpp", 876, 0, "%s\n\t(weight) = %g", "(weight >= 0.0f)", weight);
            if (weight != 0.0)
            {
                frequency = XAnimGetAverageRateFrequency(tree, infoIndexa);
                if (frequency != 0.0)
                {
                    totalWeight = totalWeight + weight;
                    totalDtime = frequency * weight * infoa->state.rate + totalDtime;
                }
            }
        }
        if (totalWeight == 0.0)
            return 0.0;
        else
            return (totalDtime / totalWeight);
    }
}

void __cdecl XAnimUpdateTimeAndNotetrackLeaf(
    const DObj_s* obj,
    const XAnimParts* parts,
    unsigned int infoIndex,
    float dtime,
    bool bNotify)
{
    const char* v5; // eax
    const char* v6; // eax
    BOOL v7; // [esp+10h] [ebp-30h]
    BOOL v8; // [esp+20h] [ebp-20h]
    float v9; // [esp+28h] [ebp-18h]
    float v10; // [esp+2Ch] [ebp-14h]
    XAnimState* state; // [esp+30h] [ebp-10h]
    XAnimInfo* info; // [esp+34h] [ebp-Ch]
    float time; // [esp+38h] [ebp-8h]
    __int16 cycleCount; // [esp+3Ch] [ebp-4h]
    float dtimea; // [esp+54h] [ebp+14h]

    if (!parts)
        MyAssertHandler(".\\xanim\\xanim.cpp", 1231, 0, "%s", "parts");
    info = &g_xAnimInfo[infoIndex];
    state = &info->state;
    if (info->state.oldTime < 0.0)
        MyAssertHandler(".\\xanim\\xanim.cpp", 1236, 0, "%s", "state->oldTime >= 0");
    dtimea = g_xAnimInfo[infoIndex].state.rate * parts->frequency * dtime;
    if (dtimea != 0.0)
    {
        time = g_xAnimInfo[infoIndex].state.oldTime + dtimea;
        cycleCount = g_xAnimInfo[infoIndex].state.cycleCount;
        if (time < 0.0)
            MyAssertHandler(".\\xanim\\xanim.cpp", 1245, 0, "%s\n\t(time) = %g", "(time >= 0)", time);
        if (time >= 1.0)
        {
            if (parts->bLoop)
            {
                do
                {
                    time = time - 1.0;
                    ++cycleCount;
                } while (time >= 1.0);
                if (time < 0.0)
                    MyAssertHandler(".\\xanim\\xanim.cpp", 1262, 0, "%s", "time >= 0");
            }
            else
            {
                v10 = g_xAnimInfo[infoIndex].state.oldTime - 0.9999998807907104;
                if (v10 < 0.0)
                    v9 = 0.99999988f;
                else
                    v9 = 1.0;
                time = v9;
            }
        }
        if (parts->bLoop)
            v8 = time < 1.0;
        else
            v8 = time <= 1.0;
        if (!v8)
        {
            v5 = va("time: %f, parts->bLoop: %d", time, parts->bLoop);
            MyAssertHandler(".\\xanim\\xanim.cpp", 1266, 0, "%s\n\t%s", "parts->bLoop ? (time < 1.f) : (time <= 1.f)", v5);
        }
        if (state->currentAnimTime - time <= (double)(cycleCount - g_xAnimInfo[infoIndex].state.cycleCount))
        {
            if (bNotify)
                XAnimProcessServerNotify(obj, info, time);
            state->currentAnimTime = time;
            g_xAnimInfo[infoIndex].state.cycleCount = cycleCount;
            info->notifyIndex = -1;
            if (bNotify)
                XAnimProcessClientNotify(info, dtimea);
            if (state->currentAnimTime < 0.0)
                MyAssertHandler(
                    ".\\xanim\\xanim.cpp",
                    1282,
                    0,
                    "%s\n\t(state->currentAnimTime) = %g",
                    "(state->currentAnimTime >= 0)",
                    state->currentAnimTime);
            if (parts->bLoop)
                v7 = state->currentAnimTime < 1.0;
            else
                v7 = state->currentAnimTime <= 1.0;
            if (!v7)
            {
                v6 = va("time: %f, parts->bLoop: %d", state->currentAnimTime, parts->bLoop);
                MyAssertHandler(
                    ".\\xanim\\xanim.cpp",
                    1283,
                    0,
                    "%s\n\t%s",
                    "parts->bLoop ? (state->currentAnimTime < 1.f) : (state->currentAnimTime <= 1.f)",
                    v6);
            }
        }
    }
}

void __cdecl XAnimProcessClientNotify(XAnimInfo* info, float dtime)
{
    float frac; // [esp+4h] [ebp-1Ch]
    float fraca; // [esp+4h] [ebp-1Ch]
    float fracb; // [esp+4h] [ebp-1Ch]
    float fracc; // [esp+4h] [ebp-1Ch]
    float fracd; // [esp+4h] [ebp-1Ch]
    float frace; // [esp+4h] [ebp-1Ch]
    float fracf; // [esp+4h] [ebp-1Ch]
    const XAnimState* state; // [esp+Ch] [ebp-14h]
    XAnimNotifyInfo* notifyInfo; // [esp+10h] [ebp-10h]
    XAnimNotifyInfo* notifyInfoa; // [esp+10h] [ebp-10h]
    XAnimNotifyInfo* notifyInfob; // [esp+10h] [ebp-10h]
    unsigned __int16 notifyType; // [esp+14h] [ebp-Ch]
    unsigned __int16 notifyIndex; // [esp+18h] [ebp-8h]
    unsigned __int16 notifyIndexa; // [esp+18h] [ebp-8h]
    XAnimParts* parts; // [esp+1Ch] [ebp-4h]

    state = &info->state;
    if (dtime == 0.0)
        MyAssertHandler(".\\xanim\\xanim.cpp", 1062, 0, "%s", "dtime");
    notifyType = info->notifyType;
    if (notifyType)
    {
        if (info->state.goalWeight == 0.0)
            MyAssertHandler(".\\xanim\\xanim.cpp", 1068, 0, "%s", "state->goalWeight");
        if (info->state.oldTime == 1.0)
        {
            frac = XAnimGetNotifyFracLeaf(state, state, 1.0, dtime);
            XAnimAddClientNotify(g_endNotetrackName, frac, notifyType);
        }
        else if (info->animToModel)
        {
            parts = info->parts;
            if (!parts)
                MyAssertHandler(".\\xanim\\xanim.cpp", 1084, 0, "%s", "parts");
            notifyIndex = XAnimGetNextNotifyIndex(parts, info->state.oldTime);
            if (notifyIndex >= (int)parts->notifyCount)
                MyAssertHandler(".\\xanim\\xanim.cpp", 1087, 0, "%s", "notifyIndex < parts->notifyCount");
            notifyInfo = &parts->notify[notifyIndex];
            if (info->state.oldTime <= (double)info->state.currentAnimTime)
            {
                if (state->currentAnimTime == 1.0)
                {
                    if (parts->bLoop)
                        MyAssertHandler(".\\xanim\\xanim.cpp", 1134, 0, "%s", "!parts->bLoop");
                    if (notifyInfo->time >= (double)info->state.oldTime)
                    {
                        do
                        {
                            if (notifyInfo->time < (double)info->state.oldTime)
                                MyAssertHandler(".\\xanim\\xanim.cpp", 1141, 0, "%s", "state->oldTime <= notifyInfo->time");
                            frace = XAnimGetNotifyFracLeaf(state, state, notifyInfo->time, dtime);
                            XAnimAddClientNotify(notifyInfo->name, frace, notifyType);
                            ++notifyInfo;
                            ++notifyIndex;
                        } while (notifyIndex < (int)parts->notifyCount);
                    }
                }
                else if (notifyInfo->time < (double)state->currentAnimTime && notifyInfo->time >= (double)info->state.oldTime)
                {
                    do
                    {
                        if (notifyInfo->time < (double)info->state.oldTime)
                            MyAssertHandler(".\\xanim\\xanim.cpp", 1158, 0, "%s", "state->oldTime <= notifyInfo->time");
                        fracf = XAnimGetNotifyFracLeaf(state, state, notifyInfo->time, dtime);
                        XAnimAddClientNotify(notifyInfo->name, fracf, notifyType);
                        ++notifyInfo;
                        ++notifyIndex;
                    } while (notifyIndex < (int)parts->notifyCount && notifyInfo->time < (double)state->currentAnimTime);
                }
            }
            else if (notifyInfo->time >= (double)state->currentAnimTime)
            {
                if (notifyInfo->time >= (double)info->state.oldTime)
                {
                    do
                    {
                        if (notifyInfo->time < (double)info->state.oldTime)
                            MyAssertHandler(".\\xanim\\xanim.cpp", 1110, 0, "%s", "state->oldTime <= notifyInfo->time");
                        fracc = XAnimGetNotifyFracLeaf(state, state, notifyInfo->time, dtime);
                        XAnimAddClientNotify(notifyInfo->name, fracc, notifyType);
                        ++notifyInfo;
                        ++notifyIndex;
                    } while (notifyIndex < (int)parts->notifyCount);
                    notifyIndexa = 0;
                    for (notifyInfoa = parts->notify; (notifyInfoa->time < (double)state->currentAnimTime); notifyInfoa = notifyInfob + 1)
                    {
                        fracd = XAnimGetNotifyFracLeaf(state, state, notifyInfoa->time, dtime);
                        XAnimAddClientNotify(notifyInfoa->name, fracd, notifyType);
                        notifyInfob = notifyInfoa + 1;
                        if (++notifyIndexa >= (int)parts->notifyCount)
                            MyAssertHandler(".\\xanim\\xanim.cpp", 1125, 0, "%s", "notifyIndex < parts->notifyCount");
                    }
                }
            }
            else
            {
                do
                {
                    fracb = XAnimGetNotifyFracLeaf(state, state, notifyInfo->time, dtime);
                    XAnimAddClientNotify(notifyInfo->name, fracb, notifyType);
                    ++notifyInfo;
                    ++notifyIndex;
                } while (notifyIndex < (int)parts->notifyCount && notifyInfo->time < (double)state->currentAnimTime);
            }
        }
        else if (info->state.oldTime > (double)info->state.currentAnimTime || state->currentAnimTime == 1.0)
        {
            fraca = XAnimGetNotifyFracLeaf(state, state, 1.0, dtime);
            XAnimAddClientNotify(g_endNotetrackName, fraca, notifyType);
        }
    }
}

unsigned __int16 __cdecl XAnimGetNextNotifyIndex(const XAnimParts* parts, float time)
{
    XAnimNotifyInfo* notifyInfo; // [esp+8h] [ebp-14h]
    XAnimNotifyInfo* bestNotifyInfo; // [esp+Ch] [ebp-10h]
    float bestTime; // [esp+10h] [ebp-Ch]
    float testTime; // [esp+14h] [ebp-8h]
    int notifyInfoIndex; // [esp+18h] [ebp-4h]

    if (!parts)
        MyAssertHandler(".\\xanim\\xanim.cpp", 901, 0, "%s", "parts");
    if (time < 0.0)
        MyAssertHandler(".\\xanim\\xanim.cpp", 903, 0, "%s\n\t(time) = %g", "(time >= 0)", time);
    if (time >= 1.0)
        MyAssertHandler(".\\xanim\\xanim.cpp", 904, 0, "%s\n\t(time) = %g", "(time < 1.f)", time);
    bestNotifyInfo = 0;
    bestTime = 2.0;
    notifyInfo = parts->notify;
    if (!notifyInfo)
        MyAssertHandler(".\\xanim\\xanim.cpp", 910, 0, "%s", "notifyInfo");
    for (notifyInfoIndex = 0; notifyInfoIndex < parts->notifyCount; ++notifyInfoIndex)
    {
        testTime = notifyInfo->time;
        if (testTime < 0.0)
            MyAssertHandler(".\\xanim\\xanim.cpp", 914, 0, "%s", "testTime >= 0");
        if (time <= (double)testTime && bestTime > (double)testTime)
        {
            bestTime = testTime;
            bestNotifyInfo = notifyInfo;
        }
        ++notifyInfo;
    }
    if (!bestNotifyInfo)
        MyAssertHandler(".\\xanim\\xanim.cpp", 924, 0, "%s", "bestNotifyInfo");
    if (bestNotifyInfo != parts->notify && bestNotifyInfo[-1].time >= (double)bestNotifyInfo->time)
        MyAssertHandler(
            ".\\xanim\\xanim.cpp",
            925,
            0,
            "%s",
            "bestNotifyInfo == parts->notify || bestNotifyInfo->time > (bestNotifyInfo - 1)->time");
    return bestNotifyInfo - parts->notify;
}

double __cdecl XAnimGetNotifyFracLeaf(const XAnimState* state, const XAnimState* nextState, float time, float dtime)
{
    if (dtime == 0.0)
        MyAssertHandler(".\\xanim\\xanim.cpp", 940, 0, "%s", "dtime");
    if (nextState->oldTime == 1.0)
        return 1.0;
    if (nextState->oldTime <= (double)nextState->currentAnimTime)
    {
        if ((time < (double)nextState->currentAnimTime || nextState->currentAnimTime == 1.0)
            && time >= (double)nextState->oldTime)
        {
            return (float)(((double)(nextState->oldCycleCount - state->oldCycleCount) + time - state->oldTime) / dtime);
        }
        else
        {
            return 1.0;
        }
    }
    else if (time >= (double)nextState->currentAnimTime)
    {
        if (time < (double)nextState->oldTime)
            return 1.0;
        else
            return (float)(((double)(nextState->oldCycleCount - state->oldCycleCount) + time - state->oldTime) / dtime);
    }
    else
    {
        return (float)(((double)(nextState->oldCycleCount + 1 - state->oldCycleCount) + time - state->oldTime) / dtime);
    }
}

void __cdecl XAnimAddClientNotify(unsigned int notetrackName, float frac, unsigned int notifyType)
{
    XAnimNotify_s* notify; // [esp+0h] [ebp-8h]
    XAnimNotify_s* notifya; // [esp+0h] [ebp-8h]
    int i; // [esp+4h] [ebp-4h]

    if (g_notifyListSize >= 128)
        MyAssertHandler(".\\xanim\\xanim.cpp", 1032, 0, "%s", "g_notifyListSize < MAX_NOTIFYLIST");
    if (!Sys_IsMainThread())
        MyAssertHandler(".\\xanim\\xanim.cpp", 1033, 0, "%s", "Sys_IsMainThread()");
    for (i = g_notifyListSize - 1; i >= 0; --i)
    {
        notify = &g_notifyList[i];
        if (notify->timeFrac <= (double)frac)
            break;
        notify[1].name = notify->name;
        notify[1].type = notify->type;
        notify[1].timeFrac = notify->timeFrac;
    }
    notifya = &g_notifyList[i + 1];
    notifya->name = SL_ConvertToString(notetrackName);
    notifya->timeFrac = frac;
    notifya->type = notifyType;
    ++g_notifyListSize;
}

void __cdecl XAnimUpdateTimeAndNotetrackSyncSubTree(
    const DObj_s* obj,
    unsigned int infoIndex,
    float dtime,
    bool bNotify)
{
    const char* v4; // eax
    const char* v5; // eax
    BOOL v6; // [esp+1Ch] [ebp-34h]
    BOOL v7; // [esp+2Ch] [ebp-24h]
    float v8; // [esp+34h] [ebp-1Ch]
    float v9; // [esp+38h] [ebp-18h]
    XAnimState* state; // [esp+3Ch] [ebp-14h]
    unsigned int nextInfoIndex; // [esp+40h] [ebp-10h]
    XAnimInfo* info; // [esp+44h] [ebp-Ch]
    float time; // [esp+48h] [ebp-8h]
    __int16 cycleCount; // [esp+4Ch] [ebp-4h]
    unsigned int infoIndexa; // [esp+5Ch] [ebp+Ch]

    info = &g_xAnimInfo[infoIndex];
    state = &info->state;
    if ((info->animParent.flags & 4) == 0)
        MyAssertHandler(".\\xanim\\xanim.cpp", 1298, 0, "%s", "info->animParent.flags & XANIM_SYNC_ROOT");
    if (g_xAnimInfo[infoIndex].state.oldTime < 0.0 || g_xAnimInfo[infoIndex].state.oldTime > 1.0)
        MyAssertHandler(
            ".\\xanim\\xanim.cpp",
            1300,
            0,
            "state->oldTime not in [0.0f, 1.0f]\n\t%g not in [%g, %g]",
            g_xAnimInfo[infoIndex].state.oldTime,
            0.0,
            1.0);
    time = g_xAnimInfo[infoIndex].state.oldTime + dtime;
    cycleCount = g_xAnimInfo[infoIndex].state.oldCycleCount;
    if (time < 0.0)
        MyAssertHandler(".\\xanim\\xanim.cpp", 1305, 0, "%s\n\t(time) = %g", "(time >= 0)", time);
    if (time >= 1.0)
    {
        if ((info->animParent.flags & 2) != 0)
        {
            if ((info->animParent.flags & 1) != 0)
                MyAssertHandler(".\\xanim\\xanim.cpp", 1311, 0, "%s", "!(info->animParent.flags & XANIM_LOOP_SYNC_TIME)");
            v9 = g_xAnimInfo[infoIndex].state.oldTime - 0.9999998807907104;
            if (v9 < 0.0)
                v8 = 0.99999988f;
            else
                v8 = 1.0;
            time = v8;
        }
        else
        {
            if ((info->animParent.flags & 1) == 0)
                MyAssertHandler(".\\xanim\\xanim.cpp", 1316, 0, "%s", "info->animParent.flags & XANIM_LOOP_SYNC_TIME");
            do
            {
                time = time - 1.0;
                ++cycleCount;
            } while (time >= 1.0);
            if (time < 0.0)
                MyAssertHandler(".\\xanim\\xanim.cpp", 1324, 0, "%s", "time >= 0");
        }
    }
    if ((info->animParent.flags & 1) != 0)
        v7 = time < 1.0;
    else
        v7 = time <= 1.0;
    if (!v7)
    {
        v4 = va("time: %f, info->animParent.flags & XANIM_LOOP_SYNC_TIME: %d", time, info->animParent.flags & 1);
        MyAssertHandler(
            ".\\xanim\\xanim.cpp",
            1328,
            0,
            "%s\n\t%s",
            "info->animParent.flags & XANIM_LOOP_SYNC_TIME ? (time < 1.f) : (time <= 1.f)",
            v4);
    }
    if (state->currentAnimTime - time <= (double)(cycleCount - g_xAnimInfo[infoIndex].state.cycleCount))
    {
        if (bNotify)
            XAnimProcessServerNotify(obj, info, time);
        state->currentAnimTime = time;
        g_xAnimInfo[infoIndex].state.cycleCount = cycleCount;
        info->notifyIndex = -1;
        if (bNotify)
            XAnimProcessClientNotify(info, dtime);
        if (state->currentAnimTime < 0.0)
            MyAssertHandler(
                ".\\xanim\\xanim.cpp",
                1344,
                0,
                "%s\n\t(state->currentAnimTime) = %g",
                "(state->currentAnimTime >= 0)",
                state->currentAnimTime);
        if ((info->animParent.flags & 1) != 0)
            v6 = state->currentAnimTime < 1.0;
        else
            v6 = state->currentAnimTime <= 1.0;
        if (!v6)
        {
            v5 = va(
                "time: %f, info->animParent.flags & XANIM_LOOP_SYNC_TIME: %d",
                state->currentAnimTime,
                info->animParent.flags & 1);
            MyAssertHandler(
                ".\\xanim\\xanim.cpp",
                1345,
                0,
                "%s\n\t%s",
                "info->animParent.flags & XANIM_LOOP_SYNC_TIME ? (state->currentAnimTime < 1.f) : (state->currentAnimTime <= 1.f)",
                v5);
        }
        for (infoIndexa = info->children; infoIndexa; infoIndexa = nextInfoIndex)
        {
            nextInfoIndex = g_xAnimInfo[infoIndexa].next;
            XAnimUpdateInfoSync(obj, infoIndexa, bNotify, state, dtime);
        }
    }
}

void __cdecl XAnimUpdateInfoSync(
    const DObj_s* obj,
    unsigned int infoIndex,
    bool bNotify,
    XAnimState* syncState,
    float dtime)
{
    XAnimState* state; // [esp+4h] [ebp-Ch]
    unsigned int nextInfoIndex; // [esp+8h] [ebp-8h]
    XAnimInfo* info; // [esp+Ch] [ebp-4h]
    unsigned int infoIndexa; // [esp+1Ch] [ebp+Ch]

    if (dtime <= 0.0)
        MyAssertHandler(".\\xanim\\xanim.cpp", 1172, 0, "%s", "dtime > 0");
    if (!infoIndex || infoIndex >= 0x1000)
        MyAssertHandler(
            ".\\xanim\\xanim.cpp",
            1173,
            0,
            "%s\n\t(infoIndex) = %i",
            "(infoIndex && (infoIndex < 4096))",
            infoIndex);
    info = &g_xAnimInfo[infoIndex];
    state = &info->state;
    if (info->state.weight == 0.0)
    {
        XAnimCheckFreeInfo(obj->tree, infoIndex, 0);
    }
    else
    {
        if (g_xAnimInfo[infoIndex].state.goalWeight == 0.0)
            bNotify = 0;
        if (!info->inuse)
            MyAssertHandler(".\\xanim\\xanim.cpp", 1187, 0, "%s", "info->inuse");
        if (syncState->oldTime != g_xAnimInfo[infoIndex].state.oldTime
            || g_xAnimInfo[infoIndex].state.oldCycleCount != syncState->oldCycleCount)
        {
            state->currentAnimTime = syncState->oldTime;
            g_xAnimInfo[infoIndex].state.cycleCount = syncState->oldCycleCount;
            g_xAnimInfo[infoIndex].state.oldTime = syncState->oldTime;
            g_xAnimInfo[infoIndex].state.oldCycleCount = syncState->oldCycleCount;
            info->notifyIndex = -1;
        }
        if (bNotify)
            XAnimProcessServerNotify(obj, info, syncState->currentAnimTime);
        state->currentAnimTime = syncState->currentAnimTime;
        g_xAnimInfo[infoIndex].state.cycleCount = syncState->cycleCount;
        info->notifyIndex = -1;
        if (bNotify)
            XAnimProcessClientNotify(info, dtime);
        for (infoIndexa = info->children; infoIndexa; infoIndexa = nextInfoIndex)
        {
            nextInfoIndex = g_xAnimInfo[infoIndexa].next;
            XAnimUpdateInfoSync(obj, infoIndexa, bNotify, syncState, dtime);
        }
    }
}

void __cdecl XAnimProcessServerNotify(const DObj_s* obj, XAnimInfo* info, float time)
{
    XAnimNotifyInfo* notifyInfo; // [esp+0h] [ebp-10h]
    XAnimNotifyInfo* notifyInfoa; // [esp+0h] [ebp-10h]
    XAnimTree_s* tree; // [esp+4h] [ebp-Ch]
    int notifyIndex; // [esp+8h] [ebp-8h]
    int notifyIndexa; // [esp+8h] [ebp-8h]
    XAnimParts* parts; // [esp+Ch] [ebp-4h]

    if (info->state.goalWeight == 0.0)
        MyAssertHandler(".\\xanim\\xanim.cpp", 1944, 0, "%s", "info->state.goalWeight");
    if (time > 1.0)
        MyAssertHandler(".\\xanim\\xanim.cpp", 1945, 0, "%s", "time <= 1.f");
    if (obj->entnum && info->notifyName)
    {
        if (info->state.currentAnimTime == 1.0)
        {
            Scr_AddConstString(g_endNotetrackName);
            Scr_NotifyNum(obj->entnum - 1, 0, info->notifyName, 1u);
        }
        else
        {
            tree = obj->tree;
            if (!obj->tree)
                MyAssertHandler(".\\xanim\\xanim.cpp", 1960, 0, "%s", "tree");
            parts = XAnimGetParts(tree, info);
            if (parts)
            {
                if (info->notifyIndex >= 0 || (XAnimUpdateServerNotifyIndex(info, parts), info->notifyIndex >= 0))
                {
                    if (info->state.currentAnimTime >= 1.0)
                        MyAssertHandler(".\\xanim\\xanim.cpp", 1987, 0, "%s", "info->state.currentAnimTime < 1.f");
                    if (!parts->notifyCount)
                        MyAssertHandler(".\\xanim\\xanim.cpp", 1988, 0, "%s", "parts->notifyCount > 0");
                    if (parts->notify->time > 1.0)
                        MyAssertHandler(".\\xanim\\xanim.cpp", 1989, 0, "%s", "parts->notify[0].time <= 1.f");
                    notifyIndex = info->notifyIndex;
                    if (notifyIndex < 0)
                        MyAssertHandler(".\\xanim\\xanim.cpp", 1992, 0, "%s", "notifyIndex >= 0");
                    if (notifyIndex >= parts->notifyCount)
                        MyAssertHandler(".\\xanim\\xanim.cpp", 1993, 0, "%s", "notifyIndex < parts->notifyCount");
                    notifyInfo = &parts->notify[notifyIndex];
                    if (info->state.currentAnimTime <= (double)time)
                    {
                        if (time == 1.0)
                        {
                            if (parts->bLoop)
                                MyAssertHandler(".\\xanim\\xanim.cpp", 2042, 0, "%s", "!parts->bLoop");
                            if (notifyInfo->time >= (double)info->state.currentAnimTime)
                            {
                                if (notifyIndex >= parts->notifyCount)
                                    MyAssertHandler(".\\xanim\\xanim.cpp", 2047, 0, "%s", "notifyIndex < parts->notifyCount");
                                do
                                {
                                    if (notifyInfo->time < (double)info->state.currentAnimTime)
                                        MyAssertHandler(
                                            ".\\xanim\\xanim.cpp",
                                            2051,
                                            0,
                                            "%s",
                                            "info->state.currentAnimTime <= notifyInfo->time");
                                    NotifyServerNotetrack(obj, info->notifyName, notifyInfo->name);
                                    ++notifyInfo;
                                    ++notifyIndex;
                                } while (notifyIndex < parts->notifyCount);
                            }
                        }
                        else if (notifyInfo->time < (double)time && notifyInfo->time >= (double)info->state.currentAnimTime)
                        {
                            if (notifyIndex >= parts->notifyCount)
                                MyAssertHandler(".\\xanim\\xanim.cpp", 2066, 0, "%s", "notifyIndex < parts->notifyCount");
                            do
                            {
                                if (notifyInfo->time < (double)info->state.currentAnimTime)
                                    MyAssertHandler(
                                        ".\\xanim\\xanim.cpp",
                                        2070,
                                        0,
                                        "%s",
                                        "info->state.currentAnimTime <= notifyInfo->time");
                                NotifyServerNotetrack(obj, info->notifyName, notifyInfo->name);
                                ++notifyInfo;
                                ++notifyIndex;
                            } while (notifyIndex < parts->notifyCount && notifyInfo->time < (double)time);
                        }
                    }
                    else if (notifyInfo->time >= (double)time)
                    {
                        if (notifyInfo->time >= (double)info->state.currentAnimTime)
                        {
                            if (notifyIndex >= parts->notifyCount)
                                MyAssertHandler(".\\xanim\\xanim.cpp", 2015, 0, "%s", "notifyIndex < parts->notifyCount");
                            do
                            {
                                if (notifyInfo->time < (double)info->state.currentAnimTime)
                                    MyAssertHandler(
                                        ".\\xanim\\xanim.cpp",
                                        2019,
                                        0,
                                        "%s",
                                        "info->state.currentAnimTime <= notifyInfo->time");
                                NotifyServerNotetrack(obj, info->notifyName, notifyInfo->name);
                                ++notifyInfo;
                                ++notifyIndex;
                            } while (notifyIndex < parts->notifyCount);
                            notifyIndexa = 0;
                            for (notifyInfoa = parts->notify; notifyInfoa->time < (double)time; ++notifyInfoa)
                            {
                                if (notifyIndexa >= parts->notifyCount)
                                    MyAssertHandler(".\\xanim\\xanim.cpp", 2030, 0, "%s", "notifyIndex < parts->notifyCount");
                                NotifyServerNotetrack(obj, info->notifyName, notifyInfoa->name);
                                ++notifyIndexa;
                            }
                        }
                    }
                    else
                    {
                        if (notifyIndex >= parts->notifyCount)
                            MyAssertHandler(".\\xanim\\xanim.cpp", 2000, 0, "%s", "notifyIndex < parts->notifyCount");
                        do
                        {
                            NotifyServerNotetrack(obj, info->notifyName, notifyInfo->name);
                            ++notifyInfo;
                            ++notifyIndex;
                        } while (notifyIndex < parts->notifyCount && notifyInfo->time < (double)time);
                    }
                }
                else if (info->state.currentAnimTime > (double)time || time == 1.0)
                {
                    Scr_AddConstString(g_endNotetrackName);
                    Scr_NotifyNum(obj->entnum - 1, 0, info->notifyName, 1u);
                }
            }
            else if (info->state.currentAnimTime > (double)time || time == 1.0)
            {
                Scr_AddConstString(g_endNotetrackName);
                Scr_NotifyNum(obj->entnum - 1, 0, info->notifyName, 1u);
            }
        }
    }
}

XAnimParts* __cdecl XAnimGetParts(const XAnimTree_s* tree, XAnimInfo* info)
{
    XAnimEntry* anim; // [esp+0h] [ebp-8h]
    XAnimParts* parts; // [esp+4h] [ebp-4h]
    XAnimParts* partsa; // [esp+4h] [ebp-4h]

    if (info->animToModel)
    {
        parts = info->parts;
        if (!parts)
            MyAssertHandler(".\\xanim\\xanim.cpp", 971, 0, "%s", "parts");
        return parts;
    }
    else if (info->notifyChild)
    {
        anim = &tree->anims->entries[info->notifyChild];
        if (!IsLeafNode(anim))
            MyAssertHandler(".\\xanim\\xanim.cpp", 979, 0, "%s", "IsLeafNode( anim )");
        partsa = anim->parts;
        if (!partsa)
            MyAssertHandler(".\\xanim\\xanim.cpp", 982, 0, "%s", "parts");
        return partsa;
    }
    else
    {
        return 0;
    }
}

void __cdecl NotifyServerNotetrack(const DObj_s* obj, unsigned int notifyName, unsigned int notetrackName)
{
    Scr_AddConstString(notetrackName);
    Scr_NotifyNum(obj->entnum - 1, 0, notifyName, 1u);
}

int __cdecl DObjUpdateServerInfo(DObj_s* obj, float dtime, int bNotify)
{
    float fracDtime; // [esp+54h] [ebp-Ch]
    XAnimTree_s* tree; // [esp+58h] [ebp-8h]
    float frac; // [esp+5Ch] [ebp-4h]

    PROF_SCOPED("DObjUpdateServerInfo");

    iassert(dtime >= 0);

    tree = obj->tree;
    if (obj->tree && tree->children)
    {
        if (bNotify)
        {
            frac = XAnimFindServerNoteTrack(obj, tree->children, dtime);

            iassert(frac >= 0);

            if (frac == 1.0 || (fracDtime = dtime * frac + EQUAL_EPSILON, dtime < (double)fracDtime))
            {
                XAnimUpdateTimeAndNotetrack(obj, tree->children, dtime, 1);
                return 0;
            }
            else
            {
                XAnimUpdateTimeAndNotetrack(obj, tree->children, fracDtime, 1);
                return 1;
            }
        }
        else
        {
            XAnimUpdateTimeAndNotetrack(obj, tree->children, dtime, 0);
            return 0;
        }
    }
    else
    {
        return 0;
    }
}

double __cdecl XAnimFindServerNoteTrack(const DObj_s* obj, unsigned int infoIndex, float dtime)
{
    float v4; // [esp+8h] [ebp-1Ch]
    float v5; // [esp+Ch] [ebp-18h]
    XAnimInfo* info; // [esp+14h] [ebp-10h]
    XAnimTree_s* tree; // [esp+18h] [ebp-Ch]
    float minFrac; // [esp+1Ch] [ebp-8h]
    float testFrac; // [esp+20h] [ebp-4h]
    unsigned int infoIndexa; // [esp+30h] [ebp+Ch]
    float dtimea; // [esp+34h] [ebp+10h]
    float dtimeb; // [esp+34h] [ebp+10h]

    tree = obj->tree;
    if (!obj->tree)
        MyAssertHandler(".\\xanim\\xanim.cpp", 1887, 0, "%s", "tree");
    if (dtime < 0.0)
        MyAssertHandler(".\\xanim\\xanim.cpp", 1889, 0, "%s", "dtime >= 0");
    if (!tree->anims)
        MyAssertHandler(".\\xanim\\xanim.cpp", 1890, 0, "%s", "tree->anims");
    if (!infoIndex || infoIndex >= 0x1000)
        MyAssertHandler(
            ".\\xanim\\xanim.cpp",
            1892,
            0,
            "%s\n\t(infoIndex) = %i",
            "(infoIndex && (infoIndex < 4096))",
            infoIndex);
    info = &g_xAnimInfo[infoIndex];
    if (info->state.weight == 0.0 || g_xAnimInfo[infoIndex].state.goalWeight == 0.0)
        return 1.0;
    if (!info->inuse)
        MyAssertHandler(".\\xanim\\xanim.cpp", 1900, 0, "%s", "info->inuse");
    if (info->animToModel)
        return XAnimFindServerNoteTrackLeafNode(obj, info, dtime);
    if ((info->animParent.flags & 3) != 0)
    {
        dtimea = XAnimGetAverageRateFrequency(tree, infoIndex) * g_xAnimInfo[infoIndex].state.rate * dtime;
        if (dtimea == 0.0)
        {
            return 1.0;
        }
        else
        {
            if (g_xAnimInfo[infoIndex].state.oldTime < 0.0)
                MyAssertHandler(
                    ".\\xanim\\xanim.cpp",
                    1913,
                    0,
                    "%s\n\t(state->oldTime) = %g",
                    "(state->oldTime >= 0)",
                    g_xAnimInfo[infoIndex].state.oldTime);
            return XAnimFindServerNoteTrackSyncSubTree(obj, info, dtimea);
        }
    }
    else
    {
        if (obj->entnum && info->notifyName)
            MyAssertHandler(".\\xanim\\xanim.cpp", 1919, 0, "%s", "!(obj->entnum && info->notifyName)");
        dtimeb = dtime * g_xAnimInfo[infoIndex].state.rate;
        if (dtimeb == 0.0)
        {
            return 1.0;
        }
        else
        {
            minFrac = 1.0;
            for (infoIndexa = info->children; infoIndexa; infoIndexa = g_xAnimInfo[infoIndexa].next)
            {
                testFrac = XAnimFindServerNoteTrack(obj, infoIndexa, dtimeb);
                v5 = testFrac - minFrac;
                if (v5 < 0.0)
                    v4 = testFrac;
                else
                    v4 = minFrac;
                minFrac = v4;
            }
            return minFrac;
        }
    }
}

double __cdecl XAnimFindServerNoteTrackLeafNode(const DObj_s* obj, XAnimInfo* info, float dtime)
{
    float v4; // [esp+8h] [ebp-38h]
    float v5; // [esp+Ch] [ebp-34h]
    XAnimState* state; // [esp+10h] [ebp-30h]
    float time; // [esp+14h] [ebp-2Ch]
    XAnimState nextState; // [esp+18h] [ebp-28h] BYREF
    __int16 cycleCount; // [esp+38h] [ebp-8h]
    const XAnimParts* parts; // [esp+3Ch] [ebp-4h]
    float dtimea; // [esp+50h] [ebp+10h]

    if (!info->inuse)
        MyAssertHandler(".\\xanim\\xanim.cpp", 1784, 0, "%s", "info->inuse");
    parts = info->parts;
    if (!parts)
        MyAssertHandler(".\\xanim\\xanim.cpp", 1788, 0, "%s", "parts");
    state = &info->state;
    dtimea = info->state.rate * parts->frequency * dtime;
    if (dtimea == 0.0)
        return 1.0;
    time = info->state.oldTime + dtimea;
    cycleCount = info->state.oldCycleCount;
    if (time < 0.0)
        MyAssertHandler(".\\xanim\\xanim.cpp", 1799, 0, "%s", "time >= 0");
    if (parts->bLoop)
    {
        while (time >= 1.0)
        {
            time = time - 1.0;
            ++cycleCount;
        }
    }
    else if (time >= 1.0)
    {
        v5 = info->state.oldTime - 0.9999998807907104;
        if (v5 < 0.0)
            v4 = 0.99999988f;
        else
            v4 = 1.0;
        time = v4;
    }
    if (info->state.currentAnimTime - time > (double)(cycleCount - info->state.cycleCount))
        return 1.0;
    nextState.oldTime = state->currentAnimTime;
    nextState.oldCycleCount = info->state.cycleCount;
    nextState.currentAnimTime = time;
    nextState.cycleCount = cycleCount;
    return XAnimGetNextServerNotifyFrac(obj, info, state, &nextState, dtimea);
}

double __cdecl XAnimGetNextServerNotifyFrac(
    const DObj_s* obj,
    XAnimInfo* info,
    const XAnimState* syncState,
    const XAnimState* nextSyncState,
    float dtime)
{
    XAnimTree_s* tree; // [esp+8h] [ebp-8h]
    const XAnimParts* parts; // [esp+Ch] [ebp-4h]

    if (info->state.goalWeight == 0.0)
        MyAssertHandler(".\\xanim\\xanim.cpp", 999, 0, "%s", "info->state.goalWeight");
    if (dtime == 0.0)
        MyAssertHandler(".\\xanim\\xanim.cpp", 1000, 0, "%s", "dtime");
    if (!obj->entnum || !info->notifyName)
        return 1.0;
    if (!info->inuse)
        MyAssertHandler(".\\xanim\\xanim.cpp", 1006, 0, "%s", "info->inuse");
    tree = obj->tree;
    if (!obj->tree)
        MyAssertHandler(".\\xanim\\xanim.cpp", 1010, 0, "%s", "tree");
    parts = XAnimGetParts(tree, info);
    if (parts && (info->notifyIndex >= 0 || (XAnimUpdateServerNotifyIndex(info, parts), info->notifyIndex >= 0)))
        return XAnimGetNotifyFracLeaf(syncState, nextSyncState, parts->notify[info->notifyIndex].time, dtime);
    else
        return XAnimGetNotifyFracLeaf(syncState, nextSyncState, 1.0, dtime);
}

double __cdecl XAnimFindServerNoteTrackSyncSubTree(const DObj_s* obj, XAnimInfo* info, float dtime)
{
    float v4; // [esp+8h] [ebp-34h]
    float v5; // [esp+Ch] [ebp-30h]
    XAnimState* state; // [esp+10h] [ebp-2Ch]
    float time; // [esp+14h] [ebp-28h]
    XAnimState nextState; // [esp+18h] [ebp-24h] BYREF
    __int16 cycleCount; // [esp+38h] [ebp-4h]

    state = &info->state;
    time = info->state.oldTime + dtime;
    cycleCount = info->state.oldCycleCount;
    if (time < 0.0)
        MyAssertHandler(".\\xanim\\xanim.cpp", 1840, 0, "%s", "time >= 0");
    if (!info->inuse)
        MyAssertHandler(".\\xanim\\xanim.cpp", 1843, 0, "%s", "info->inuse");
    if ((info->animParent.flags & 2) != 0)
    {
        if ((info->animParent.flags & 1) != 0)
            MyAssertHandler(".\\xanim\\xanim.cpp", 1848, 0, "%s", "!(info->animParent.flags & XANIM_LOOP_SYNC_TIME)");
        if (time >= 1.0)
        {
            v5 = info->state.oldTime - 0.9999998807907104;
            if (v5 < 0.0)
                v4 = 0.99999988f;
            else
                v4 = 1.0;
            time = v4;
        }
    }
    else
    {
        if ((info->animParent.flags & 1) == 0)
            MyAssertHandler(".\\xanim\\xanim.cpp", 1854, 0, "%s", "info->animParent.flags & XANIM_LOOP_SYNC_TIME");
        while (time >= 1.0)
        {
            time = time - 1.0;
            ++cycleCount;
        }
        if (time < 0.0)
            MyAssertHandler(".\\xanim\\xanim.cpp", 1862, 0, "%s", "time >= 0");
    }
    if (info->state.currentAnimTime - time > (double)(cycleCount - info->state.cycleCount))
        return 1.0;
    nextState.oldTime = state->currentAnimTime;
    nextState.oldCycleCount = info->state.cycleCount;
    nextState.currentAnimTime = time;
    nextState.cycleCount = cycleCount;
    return XAnimGetServerNotifyFracSyncTotal(obj, info, state, &nextState, dtime);
}

double __cdecl XAnimGetServerNotifyFracSyncTotal(
    const DObj_s* obj,
    XAnimInfo* info,
    const XAnimState* syncState,
    const XAnimState* nextSyncState,
    float dtime)
{
    float minFrac; // [esp+4h] [ebp-Ch]
    unsigned int infoIndex; // [esp+8h] [ebp-8h]
    float testFrac; // [esp+Ch] [ebp-4h]
    XAnimInfo* infoa; // [esp+1Ch] [ebp+Ch]

    minFrac = XAnimGetNextServerNotifyFrac(obj, info, syncState, nextSyncState, dtime);
    for (infoIndex = info->children; infoIndex; infoIndex = g_xAnimInfo[infoIndex].next)
    {
        if (infoIndex >= 0x1000)
            MyAssertHandler(
                ".\\xanim\\xanim.cpp",
                1761,
                0,
                "%s\n\t(infoIndex) = %i",
                "(infoIndex && (infoIndex < 4096))",
                infoIndex);
        infoa = &g_xAnimInfo[infoIndex];
        if (infoa->state.weight != 0.0 && infoa->state.goalWeight != 0.0)
        {
            testFrac = XAnimGetServerNotifyFracSyncTotal(obj, infoa, syncState, nextSyncState, dtime);
            if (minFrac > (double)testFrac)
                minFrac = testFrac;
        }
    }
    return minFrac;
}

int __cdecl DObjGetClientNotifyList(XAnimNotify_s** notifyList)
{
    if (!Sys_IsMainThread())
        MyAssertHandler(".\\xanim\\xanim.cpp", 2804, 0, "%s", "Sys_IsMainThread()");
    *notifyList = g_notifyList;
    return g_notifyListSize;
}

void __cdecl DObjDisplayAnimToBuffer(const DObj_s* obj, const char* header, char* buffer, int bufferSize)
{
    XAnimTree_s* tree; // [esp+0h] [ebp-8h]
    int bufferPos; // [esp+4h] [ebp-4h] BYREF

    if (!header)
        MyAssertHandler(".\\xanim\\xanim.cpp", 2816, 0, "%s", "header");
    if (!buffer)
        MyAssertHandler(".\\xanim\\xanim.cpp", 2817, 0, "%s", "buffer");
    tree = obj->tree;
    if (obj->tree && tree->children)
    {
        bufferPos = 0;
        Com_sprintfPos(buffer, bufferSize, &bufferPos, "%s", header);
        XAnimDisplay(tree, tree->children, 0, buffer, bufferSize, &bufferPos);
        Com_sprintfPos(buffer, bufferSize, &bufferPos, "\n");
    }
    else
    {
        Com_sprintf(buffer, bufferSize, "%sNO TREE\n", header);
    }
}

void __cdecl XAnimDisplay(
    const XAnimTree_s *tree,
    unsigned int infoIndex,
    int depth,
    char *buffer,
    int bufferSize,
    int *bufferPos)
{
    char *v6; // [esp+28h] [ebp-30h]
    char *v7; // [esp+28h] [ebp-30h]
    char *v8; // [esp+28h] [ebp-30h]
    double v9; // [esp+2Ch] [ebp-2Ch]
    XAnimInfo *info; // [esp+38h] [ebp-20h]
    float delta; // [esp+3Ch] [ebp-1Ch]
    char *debugName; // [esp+40h] [ebp-18h]
    unsigned int animIndex; // [esp+44h] [ebp-14h]
    int i; // [esp+48h] [ebp-10h]
    const XAnimParts *parts; // [esp+4Ch] [ebp-Ch]
    const char *color; // [esp+50h] [ebp-8h]
    float realtimedelta; // [esp+54h] [ebp-4h]
    unsigned int infoIndexa; // [esp+64h] [ebp+Ch]

    if (!tree)
        MyAssertHandler(".\\xanim\\xanim.cpp", 2159, 0, "%s", "tree");
    if (!tree->anims)
        MyAssertHandler(".\\xanim\\xanim.cpp", 2160, 0, "%s", "tree->anims");
    if (!buffer)
        MyAssertHandler(".\\xanim\\xanim.cpp", 2161, 0, "%s", "buffer");
    if (!bufferPos)
        MyAssertHandler(".\\xanim\\xanim.cpp", 2162, 0, "%s", "bufferPos");
    if (bufferSize <= *bufferPos)
        MyAssertHandler(".\\xanim\\xanim.cpp", 2163, 0, "%s", "bufferSize > *bufferPos");
    if (!infoIndex || infoIndex >= 0x1000)
        MyAssertHandler(
            ".\\xanim\\xanim.cpp",
            2165,
            0,
            "%s\n\t(infoIndex) = %i",
            "(infoIndex && (infoIndex < 4096))",
            infoIndex);
    info = &g_xAnimInfo[infoIndex];
    if (info->state.weight < 0.0)
        MyAssertHandler(".\\xanim\\xanim.cpp", 2169, 0, "%s", "state->weight >= 0");
    for (i = 0; i < depth; ++i)
        Com_sprintfPos(buffer, bufferSize, bufferPos, " ");
    if (!info->inuse)
        MyAssertHandler(".\\xanim\\xanim.cpp", 2175, 0, "%s", "info->inuse");
    animIndex = info->animIndex;
    if (animIndex >= tree->anims->size)
        MyAssertHandler(".\\xanim\\xanim.cpp", 2178, 0, "%s", "animIndex < tree->anims->size");
    debugName = XAnimGetAnimDebugName(tree->anims, animIndex);
    if (g_xAnimInfo[infoIndex].state.weight == 0.0)
    {
        color = "^0";
    }
    else if (g_xAnimInfo[infoIndex].state.goalWeight <= g_xAnimInfo[infoIndex].state.weight)
    {
        if (g_xAnimInfo[infoIndex].state.goalWeight >= g_xAnimInfo[infoIndex].state.weight)
            color = "";
        else
            color = "^1";
    }
    else
    {
        color = "^4";
    }
    if (info->animToModel)
    {
        parts = info->parts;
        if (!parts)
            MyAssertHandler(".\\xanim\\xanim.cpp", 2194, 0, "%s", "parts");
        delta = g_xAnimInfo[infoIndex].state.currentAnimTime - g_xAnimInfo[infoIndex].state.oldTime;
        if (delta < 0.0)
            delta = delta + 1.0;
        if (parts->frequency == 0.0)
            v9 = 0.0;
        else
            v9 = delta / parts->frequency;
        realtimedelta = v9;
        if (info->notifyName)
        {
            v6 = SL_ConvertToString(info->notifyName);
            Com_sprintfPos(
                buffer,
                bufferSize,
                bufferPos,
                "%s%s: (weight) %.2f -> %.2f, (time) %.2f -> %.2f, (realtimedelta) %.2f, '%s'\n",
                color,
                debugName,
                g_xAnimInfo[infoIndex].state.weight,
                g_xAnimInfo[infoIndex].state.goalWeight,
                g_xAnimInfo[infoIndex].state.oldTime,
                g_xAnimInfo[infoIndex].state.currentAnimTime,
                realtimedelta,
                v6);
        }
        else
        {
            Com_sprintfPos(
                buffer,
                bufferSize,
                bufferPos,
                "%s%s: (weight) %.2f -> %.2f, (time) %.2f -> %.2f, (realtimedelta) %.2f\n",
                color,
                debugName,
                g_xAnimInfo[infoIndex].state.weight,
                g_xAnimInfo[infoIndex].state.goalWeight,
                g_xAnimInfo[infoIndex].state.oldTime,
                g_xAnimInfo[infoIndex].state.currentAnimTime,
                realtimedelta);
        }
    }
    else
    {
        if (info->notifyName)
        {
            if (XAnimHasTime(tree->anims, animIndex))
            {
                v7 = SL_ConvertToString(info->notifyName);
                Com_sprintfPos(
                    buffer,
                    bufferSize,
                    bufferPos,
                    "%s%s: (weight) %.2f -> %.2f, (time) %.2f -> %.2f, '%s'\n",
                    color,
                    debugName,
                    g_xAnimInfo[infoIndex].state.weight,
                    g_xAnimInfo[infoIndex].state.goalWeight,
                    g_xAnimInfo[infoIndex].state.oldTime,
                    g_xAnimInfo[infoIndex].state.currentAnimTime,
                    v7);
            }
            else
            {
                v8 = SL_ConvertToString(info->notifyName);
                Com_sprintfPos(
                    buffer,
                    bufferSize,
                    bufferPos,
                    "%s%s: (weight) %.2f -> %.2f, '%s'\n",
                    color,
                    debugName,
                    g_xAnimInfo[infoIndex].state.weight,
                    g_xAnimInfo[infoIndex].state.goalWeight,
                    v8);
            }
        }
        else if (XAnimHasTime(tree->anims, animIndex))
        {
            Com_sprintfPos(
                buffer,
                bufferSize,
                bufferPos,
                "%s%s: (weight) %.2f -> %.2f, (time) %.2f -> %.2f\n",
                color,
                debugName,
                g_xAnimInfo[infoIndex].state.weight,
                g_xAnimInfo[infoIndex].state.goalWeight,
                g_xAnimInfo[infoIndex].state.oldTime,
                g_xAnimInfo[infoIndex].state.currentAnimTime);
        }
        else
        {
            Com_sprintfPos(
                buffer,
                bufferSize,
                bufferPos,
                "%s%s: (weight) %.2f -> %.2f\n",
                color,
                debugName,
                g_xAnimInfo[infoIndex].state.weight,
                g_xAnimInfo[infoIndex].state.goalWeight);
        }
        for (infoIndexa = info->children; infoIndexa; infoIndexa = g_xAnimInfo[infoIndexa].next)
            XAnimDisplay(tree, infoIndexa, depth + 1, buffer, bufferSize, bufferPos);
    }
}

void __cdecl DObjDisplayAnim(const DObj_s* obj, const char* header)
{
    char buffer[2052]; // [esp+0h] [ebp-808h] BYREF

    iassert(header);
    DObjDisplayAnimToBuffer(obj, header, buffer, 2048);
    Com_Printf(19, buffer);
}

void __cdecl XAnimCalcDelta(DObj_s* obj, unsigned int animIndex, float* rot, float* trans, bool bUseGoalWeight)
{
    XAnimSimpleRotPos rotPos; // [esp+3Ch] [ebp-24h] BYREF
    XAnimTree_s* tree; // [esp+54h] [ebp-Ch]
    unsigned int infoIndex; // [esp+58h] [ebp-8h]
    XAnimDeltaInfo deltaInfo; // [esp+5Ch] [ebp-4h]

    PROF_SCOPED("XAnimCalcDelta");

    tree = obj->tree;

    iassert(tree);
    iassert(tree->anims);
    iassert(animIndex < tree->anims->size);

    infoIndex = XAnimGetInfoIndex(tree, animIndex);
    if (infoIndex)
    {
        deltaInfo.bClear = 1;
        deltaInfo.bNormQuat = 0;
        deltaInfo.bAbs = 0;
        deltaInfo.bUseGoalWeight = bUseGoalWeight;
        XAnimCalcDeltaTree(obj, infoIndex, 1.0f, deltaInfo, &rotPos);
        if (rotPos.rot[0] == 0.0f || rotPos.rot[1] == 0.0f)
        {
            rot[0] = 0.0f;
            rot[1] = 1.0f;
        }
        else
        {
            rot[0] = rotPos.rot[0];
            rot[1] = rotPos.rot[1];
        }
        trans[0] = rotPos.pos[0];
        trans[1] = rotPos.pos[1];
        trans[2] = rotPos.pos[2];
    }
    else
    {
        rot[0] = 0.0f;
        rot[1] = 1.0f;
        trans[0] = 0.0f;
        trans[1] = 0.0f;
        trans[2] = 0.0f;
    }
}

void __cdecl XAnimCalcDeltaTree(
    const DObj_s* obj,
    unsigned int infoIndex,
    float weightScale,
    XAnimDeltaInfo deltaInfo,
    XAnimSimpleRotPos* rotPos)
{
    float v5; // [esp+14h] [ebp-94h]
    float v6; // [esp+18h] [ebp-90h]
    float v7; // [esp+1Ch] [ebp-8Ch]
    float v8; // [esp+20h] [ebp-88h]
    XAnimSimpleRotPos* p_newRotPos; // [esp+24h] [ebp-84h]
    float v10; // [esp+28h] [ebp-80h]
    float goalWeight; // [esp+2Ch] [ebp-7Ch]
    float v12; // [esp+34h] [ebp-74h]
    float v13; // [esp+4Ch] [ebp-5Ch]
    unsigned int infoIndex1; // [esp+68h] [ebp-40h]
    float r; // [esp+70h] [ebp-38h]
    float ra; // [esp+70h] [ebp-38h]
    XAnimInfo* info; // [esp+74h] [ebp-34h]
    XAnimInfo* infoa; // [esp+74h] [ebp-34h]
    XAnimInfo* infob; // [esp+74h] [ebp-34h]
    XAnimInfo* infoc; // [esp+74h] [ebp-34h]
    XAnimSimpleRotPos newRotPos; // [esp+78h] [ebp-30h] BYREF
    XAnimDeltaInfo childDeltaInfo; // [esp+90h] [ebp-18h]
    unsigned int infoIndex2; // [esp+94h] [ebp-14h]
    float weight; // [esp+98h] [ebp-10h]
    float firstWeight; // [esp+9Ch] [ebp-Ch]
    XAnimSimpleRotPos* rotPos2; // [esp+A0h] [ebp-8h]
    const XAnimParts* parts; // [esp+A4h] [ebp-4h]

    if (!infoIndex || infoIndex >= 0x1000)
        MyAssertHandler(
            ".\\xanim\\xanim.cpp",
            2262,
            0,
            "%s\n\t(infoIndex) = %i",
            "(infoIndex && (infoIndex < 4096))",
            infoIndex);
    info = &g_xAnimInfo[infoIndex];
    if (!info->inuse)
        MyAssertHandler(".\\xanim\\xanim.cpp", 2265, 0, "%s", "info->inuse");
    if (info->animToModel)
    {
        if (deltaInfo.bClear)
        {
            rotPos->rot[0] = 0.0;
            rotPos->rot[1] = 0.0;
            rotPos->posWeight = 0.0;
            rotPos->pos[0] = 0.0;
            rotPos->pos[1] = 0.0;
            rotPos->pos[2] = 0.0;
        }
        parts = info->parts;
        if (!parts)
            MyAssertHandler(".\\xanim\\xanim.cpp", 2278, 0, "%s", "parts");
        if (parts->bDelta)
        {
            if (deltaInfo.bAbs)
                XAnimCalcAbsDeltaParts(parts, weightScale, info->state.currentAnimTime, rotPos);
            else
                XAnimCalcRelDeltaParts(parts, weightScale, info->state.oldTime, info->state.currentAnimTime, rotPos, 1);
        }
    }
    else
    {
        for (infoIndex1 = info->children; ; infoIndex1 = infoa->next)
        {
            if (!infoIndex1)
            {
                if (deltaInfo.bClear)
                {
                    rotPos->rot[0] = 0.0;
                    rotPos->rot[1] = 0.0;
                    rotPos->posWeight = 0.0;
                    rotPos->pos[0] = 0.0;
                    rotPos->pos[1] = 0.0;
                    rotPos->pos[2] = 0.0;
                }
                return;
            }
            if (infoIndex1 >= 0x1000)
                MyAssertHandler(
                    ".\\xanim\\xanim.cpp",
                    2294,
                    0,
                    "%s\n\t(infoIndex1) = %i",
                    "(infoIndex1 && (infoIndex1 < 4096))",
                    infoIndex1);
            infoa = &g_xAnimInfo[infoIndex1];
            if (deltaInfo.bUseGoalWeight)
                goalWeight = g_xAnimInfo[infoIndex1].state.goalWeight;
            else
                goalWeight = g_xAnimInfo[infoIndex1].state.weight;
            firstWeight = goalWeight;
            if (goalWeight < 0.0)
                MyAssertHandler(
                    ".\\xanim\\xanim.cpp",
                    2298,
                    0,
                    "%s\n\t(firstWeight) = %g",
                    "(firstWeight >= 0.0f)",
                    firstWeight);
            if (firstWeight != 0.0)
                break;
        }
        for (infoIndex2 = infoa->next; ; infoIndex2 = infob->next)
        {
            if (!infoIndex2)
            {
                XAnimCalcDeltaTree(obj, infoIndex1, weightScale, deltaInfo, rotPos);
                return;
            }
            if (infoIndex2 >= 0x1000)
                MyAssertHandler(
                    ".\\xanim\\xanim.cpp",
                    2304,
                    0,
                    "%s\n\t(infoIndex2) = %i",
                    "(infoIndex2 && (infoIndex2 < 4096))",
                    infoIndex2);
            infob = &g_xAnimInfo[infoIndex2];
            if (deltaInfo.bUseGoalWeight)
                v10 = g_xAnimInfo[infoIndex2].state.goalWeight;
            else
                v10 = g_xAnimInfo[infoIndex2].state.weight;
            weight = v10;
            if (v10 < 0.0)
                MyAssertHandler(".\\xanim\\xanim.cpp", 2308, 0, "%s", "weight >= 0");
            if (weight != 0.0)
                break;
        }
        if (deltaInfo.bClear)
            p_newRotPos = rotPos;
        else
            p_newRotPos = &newRotPos;
        rotPos2 = p_newRotPos;
        *(_WORD*)&childDeltaInfo.bAbs = *(_WORD*)&deltaInfo.bAbs;
        childDeltaInfo.bClear = 1;
        childDeltaInfo.bNormQuat = 1;
        XAnimCalcDeltaTree(obj, infoIndex1, firstWeight, childDeltaInfo, p_newRotPos);
        childDeltaInfo.bClear = 0;
        XAnimCalcDeltaTree(obj, infoIndex2, weight, childDeltaInfo, rotPos2);
        for (infoIndex2 = infob->next; infoIndex2; infoIndex2 = infoc->next)
        {
            if (infoIndex2 >= 0x1000)
                MyAssertHandler(
                    ".\\xanim\\xanim.cpp",
                    2324,
                    0,
                    "%s\n\t(infoIndex2) = %i",
                    "(infoIndex2 && (infoIndex2 < 4096))",
                    infoIndex2);
            infoc = &g_xAnimInfo[infoIndex2];
            if (deltaInfo.bUseGoalWeight)
                v8 = g_xAnimInfo[infoIndex2].state.goalWeight;
            else
                v8 = g_xAnimInfo[infoIndex2].state.weight;
            weight = v8;
            if (v8 < 0.0)
                MyAssertHandler(".\\xanim\\xanim.cpp", 2328, 0, "%s", "weight >= 0");
            if (weight != 0.0)
                XAnimCalcDeltaTree(obj, infoIndex2, weight, childDeltaInfo, rotPos2);
        }
        if (deltaInfo.bNormQuat)
        {
            if (deltaInfo.bClear)
            {
                r = rotPos->rot[1] * rotPos->rot[1] + rotPos->rot[0] * rotPos->rot[0];
                if (r != 0.0)
                {
                    v13 = I_rsqrt(r) * weightScale;
                    rotPos->rot[0] = v13 * rotPos->rot[0];
                    rotPos->rot[1] = v13 * rotPos->rot[1];
                }
                if (rotPos->posWeight != 0.0)
                {
                    v6 = weightScale / rotPos->posWeight;
                    Vec3Scale(rotPos->pos, v6, rotPos->pos);
                    rotPos->posWeight = weightScale;
                }
            }
            else
            {
                ra = rotPos2->rot[1] * rotPos2->rot[1] + rotPos2->rot[0] * rotPos2->rot[0];
                if (ra != 0.0)
                {
                    v12 = I_rsqrt(ra) * weightScale;
                    rotPos->rot[0] = v12 * rotPos2->rot[0] + rotPos->rot[0];
                    rotPos->rot[1] = v12 * rotPos2->rot[1] + rotPos->rot[1];
                }
                if (rotPos2->posWeight != 0.0)
                {
                    v5 = weightScale / rotPos2->posWeight;
                    Vec3Mad(rotPos->pos, v5, rotPos2->pos, rotPos->pos);
                    rotPos->posWeight = rotPos->posWeight + weightScale;
                }
            }
        }
        else
        {
            if (!deltaInfo.bClear)
                MyAssertHandler(".\\xanim\\xanim.cpp", 2337, 0, "%s", "deltaInfo.bClear");
            if (rotPos->posWeight != 0.0)
            {
                v7 = 1.0 / rotPos->posWeight;
                Vec3Scale(rotPos->pos, v7, rotPos->pos);
            }
        }
    }
}

void __cdecl XAnimCalcRelDeltaParts(
    const XAnimParts* parts,
    float weightScale,
    float time1,
    float time2,
    XAnimSimpleRotPos* rotPos,
    int quatIndex)
{
    unsigned __int16* v6; // [esp+40h] [ebp-ACh]
    unsigned __int16* bigTrans; // [esp+44h] [ebp-A8h]
    unsigned __int8* v8; // [esp+48h] [ebp-A4h]
    unsigned __int8* pSmallTrans; // [esp+4Ch] [ebp-A0h]
    float sizeVec_4; // [esp+54h] [ebp-98h]
    float sizeVec_8; // [esp+58h] [ebp-94h]
    float4 toVec;
    float4 fromVec;
    float pos[3]; // [esp+90h] [ebp-5Ch] BYREF
    float rotWeightScale; // [esp+9Ch] [ebp-50h]
    const XAnimDeltaPart* part; // [esp+A0h] [ebp-4Ch]
    const XAnimPartTrans* trans; // [esp+A4h] [ebp-48h]
    float4 vec1; // [esp+A8h] [ebp-44h] BYREF
    float Q[2][2]; // [esp+B8h] [ebp-34h] BYREF
    float4 vec2; // [esp+C8h] [ebp-24h] BYREF
    float4 vec; // [esp+D8h] [ebp-14h]
    float4 delta;

    XAnim_CalcDeltaForTime(parts, time1, Q[0], &vec1);
    XAnim_CalcDeltaForTime(parts, time2, Q[1], &vec2);
    if (parts->bLoop && time1 > (double)time2)
    {
        part = parts->deltaPart;
        trans = part->trans;
        if (trans)
        {
            if (trans->size)
            {
                if (trans->smallTrans)
                {
                    pSmallTrans = *trans->u.frames.frames._1;
                    fromVec.v[0] = (float)pSmallTrans[0];
                    fromVec.v[1] = (float)pSmallTrans[1];
                    fromVec.v[2] = (float)pSmallTrans[2];
                    fromVec.v[3] = 0.0f;
                    v8 = &pSmallTrans[3 * trans->size];
                    toVec.v[0] = (float)v8[0];
                    toVec.v[1] = (float)v8[1];
                    toVec.v[2] = (float)v8[2];
                }
                else
                {
                    bigTrans = *trans->u.frames.frames._2;
                    fromVec.v[0] = (float)bigTrans[0];
                    fromVec.v[1] = (float)bigTrans[1];
                    fromVec.v[2] = (float)bigTrans[2];
                    fromVec.u[3] = 0.0f;
                    v6 = &bigTrans[3 * trans->size];
                    toVec.v[0] = (float)v6[0];
                    toVec.v[1] = (float)v6[1];
                    toVec.v[2] = (float)v6[2];
                }
                sizeVec_4 = trans->u.frames.size[1];
                sizeVec_8 = trans->u.frames.size[2];

                delta.v[0] = toVec.v[0] - fromVec.v[0];
                delta.v[1] = toVec.v[1] - fromVec.v[1];
                delta.v[2] = toVec.v[2] - fromVec.v[2];
                delta.v[3] = (float)0.0f - fromVec.v[3];

                vec2.v[0] = trans->u.frames.size[0] * delta.v[0] + vec2.v[0];
                vec2.v[0] = sizeVec_4 * delta.v[1] + vec.v[0];
                vec2.v[1] = sizeVec_8 * delta.v[2] + vec.v[1];
                vec2.v[2] = 0.0f * delta.v[3] + vec.v[2];
            }
        }
    }
    rotWeightScale = weightScale * 9.313794180343393e-10;
    rotPos->rot[0] = (Q[1][0] * Q[0][1] - Q[1][1] * Q[0][0]) * rotWeightScale + rotPos->rot[0];
    rotPos->rot[1] = (Q[1][0] * Q[0][0] + Q[1][1] * Q[0][1]) * rotWeightScale + rotPos->rot[1];

    vec.v[0] = vec2.v[0] - vec1.v[0];
    vec.v[1] = vec2.v[1] - vec1.v[1];
    vec.v[2] = vec2.v[2] - vec1.v[2];
    vec.v[3] = vec2.v[3] - vec1.v[3];

    vec.v[0] = weightScale * vec.v[0];
    vec.v[1] = weightScale * vec.v[1];
    vec.v[2] = weightScale * vec.v[2];
    vec.v[3] = weightScale * vec.v[3];

    pos[0] = vec.v[0];
    pos[1] = vec.v[1];
    pos[2] = vec.v[2];

    TransformToQuatRefFrame(Q[quatIndex], pos);
    rotPos->posWeight = rotPos->posWeight + weightScale;
    Vec3Add(rotPos->pos, pos, rotPos->pos);
}

void __cdecl TransformToQuatRefFrame(const float* rot, float* trans)
{
    float r; // [esp+0h] [ebp-10h]
    float ra; // [esp+0h] [ebp-10h]
    float zz; // [esp+4h] [ebp-Ch]
    float zza; // [esp+4h] [ebp-Ch]
    float zw; // [esp+8h] [ebp-8h]
    float temp; // [esp+Ch] [ebp-4h]

    zz = rot[0] * rot[0];
    r = rot[1] * rot[1] + zz;
    if (r != 0.0f)
    {
        ra = 2.0f / r;
        zza = zz * ra;
        zw = rot[0] * rot[1] * ra;
        temp = (1.0f - zza) * trans[0] + zw * trans[1];
        trans[1] = trans[1] - (zw * trans[0] + zza * trans[1]);
        trans[0] = temp;
    }
}

void __cdecl XAnimCalcAbsDeltaParts(const XAnimParts* parts, float weightScale, float time, XAnimSimpleRotPos* rotPos)
{
    float v4; // [esp+Ch] [ebp-2Ch]
    float pos[3]; // [esp+10h] [ebp-28h] BYREF
    float Q[2]; // [esp+1Ch] [ebp-1Ch] BYREF
    float4 vec; // [esp+24h] [ebp-14h] BYREF

    XAnim_CalcDeltaForTime(parts, time, Q, &vec);
    v4 = weightScale * 0.00003051850944757462;
    rotPos->rot[0] = v4 * Q[0] + rotPos->rot[0];
    rotPos->rot[1] = v4 * Q[1] + rotPos->rot[1];
    vec.v[0] = weightScale * vec.v[0];
    vec.v[1] = weightScale * vec.v[1];
    vec.v[2] = weightScale * vec.v[2];
    vec.v[3] = weightScale * vec.v[3];
    pos[0] = vec.v[0];
    pos[1] = vec.v[1];
    pos[2] = vec.v[2];
    rotPos->posWeight = rotPos->posWeight + weightScale;
    Vec3Add(rotPos->pos, pos, rotPos->pos);
}

void __cdecl XAnimCalcAbsDelta(DObj_s* obj, unsigned int animIndex, float* rot, float* trans)
{
    XAnimSimpleRotPos rotPos; // [esp+3Ch] [ebp-24h] BYREF
    XAnimTree_s* tree; // [esp+54h] [ebp-Ch]
    unsigned int infoIndex; // [esp+58h] [ebp-8h]
    XAnimDeltaInfo deltaInfo; // [esp+5Ch] [ebp-4h]

    PROF_SCOPED("XAnimCalcAbsDelta");

    tree = obj->tree;

    iassert(tree);
    iassert(tree->anims);
    iassert(animIndex < tree->anims->size);

    infoIndex = XAnimGetInfoIndex(tree, animIndex);
    if (infoIndex)
    {
        deltaInfo.bClear = 1;
        deltaInfo.bNormQuat = 0;
        deltaInfo.bAbs = 1;
        deltaInfo.bUseGoalWeight = 1;
        XAnimCalcDeltaTree(obj, infoIndex, 1.0, deltaInfo, &rotPos);
        if (rotPos.rot[0] == 0.0 && rotPos.rot[1] == 0.0)
        {
            rot[0] = 0.0;
            rot[1] = 1.0;
        }
        else
        {
            rot[0] = rotPos.rot[0];
            rot[1] = rotPos.rot[1];
        }
        trans[0] = rotPos.pos[0];
        trans[1] = rotPos.pos[1];
        trans[2] = rotPos.pos[2];
    }
    else
    {
        rot[0] = 0.0;
        rot[1] = 1.0;
        trans[0] = 0.0;
        trans[1] = 0.0;
        trans[2] = 0.0;
    }
}

void __cdecl XAnimGetRelDelta(
    const XAnim_s* anims,
    unsigned int animIndex,
    float* rot,
    float* trans,
    float time1,
    float time2)
{
    XAnimSimpleRotPos rotPos; // [esp+44h] [ebp-20h] BYREF
    const XAnimEntry* anim; // [esp+5Ch] [ebp-8h]
    const XAnimParts* parts; // [esp+60h] [ebp-4h]

    PROF_SCOPED("XAnimGetRelDelta");

    anim = &anims->entries[animIndex];
    if (!IsLeafNode(anim))
        goto LABEL_10;

    parts = anim->parts;
    iassert(parts);
    if (parts->bDelta)
    {
        rotPos.rot[0] = 0.0;
        rotPos.rot[1] = 0.0;
        rotPos.posWeight = 0.0;
        rotPos.pos[0] = 0.0;
        rotPos.pos[1] = 0.0;
        rotPos.pos[2] = 0.0;
        XAnimCalcRelDeltaParts(parts, 1.0, time1, time2, &rotPos, 0);
        if (rotPos.rot[0] == 0.0 && rotPos.rot[1] == 0.0)
        {
            rot[0] = 0.0;
            rot[1] = 1.0;
        }
        else
        {
            rot[0] = rotPos.rot[0];
            rot[1] = rotPos.rot[1];
        }
        trans[0] = rotPos.pos[0];
        trans[1] = rotPos.pos[1];
        trans[2] = rotPos.pos[2];
    }
    else
    {
    LABEL_10:
        rot[0] = 0.0;
        rot[1] = 1.0;
        trans[0] = 0.0;
        trans[1] = 0.0;
        trans[2] = 0.0;
    }
}

void __cdecl XAnimGetAbsDelta(const XAnim_s* anims, unsigned int animIndex, float* rot, float* trans, float time)
{
    XAnimSimpleRotPos rotPos; // [esp+3Ch] [ebp-20h] BYREF
    const XAnimEntry* anim; // [esp+54h] [ebp-8h]
    const XAnimParts* parts; // [esp+58h] [ebp-4h]

    PROF_SCOPED("XAnimGetAbsDelta");

    anim = &anims->entries[animIndex];

    if (!IsLeafNode(anim))
        goto LABEL_10;

    parts = anim->parts;
    iassert(parts);
    if (parts->bDelta)
    {
        rotPos.rot[0] = 0.0;
        rotPos.rot[1] = 0.0;
        rotPos.posWeight = 0.0;
        rotPos.pos[0] = 0.0;
        rotPos.pos[1] = 0.0;
        rotPos.pos[2] = 0.0;
        XAnimCalcAbsDeltaParts(parts, 1.0, time, &rotPos);
        if (rotPos.rot[0] == 0.0 && rotPos.rot[1] == 0.0)
        {
            rot[0] = 0.0;
            rot[1] = 1.0;
        }
        else
        {
            rot[0] = rotPos.rot[0];
            rot[1] = rotPos.rot[1];
        }
        trans[0] = rotPos.pos[0];
        trans[1] = rotPos.pos[1];
        trans[2] = rotPos.pos[2];
    }
    else
    {
    LABEL_10:
        rot[0] = 0.0;
        rot[1] = 1.0;
        trans[0] = 0.0;
        trans[1] = 0.0;
        trans[2] = 0.0;
    }
}

unsigned int __cdecl XAnimAllocInfoWithParent(
    XAnimTree_s* tree,
    unsigned __int16 animToModel,
    unsigned int animIndex,
    unsigned int parentInfoIndex,
    int after)
{
    XAnimInfo* childInfo; // [esp+0h] [ebp-18h]
    XAnimInfo* childInfoa; // [esp+0h] [ebp-18h]
    XAnimInfo* info; // [esp+4h] [ebp-14h]
    unsigned int next; // [esp+8h] [ebp-10h]
    unsigned int nexta; // [esp+8h] [ebp-10h]
    unsigned int infoIndex; // [esp+Ch] [ebp-Ch]
    unsigned int prev; // [esp+10h] [ebp-8h]
    XAnimEntry* anim; // [esp+14h] [ebp-4h]

    iassert(tree);
    iassert(tree->anims);
    infoIndex = g_xAnimInfo[0].next;
    if (g_xAnimInfo[0].next)
    {
        if (!++g_info_usage)
            MyAssertHandler(".\\xanim\\xanim.cpp", 3084, 0, "%s", "g_info_usage");
        if (g_info_usage > g_info_high_usage)
            g_info_high_usage = g_info_usage;
        if (!++tree->info_usage)
            MyAssertHandler(".\\xanim\\xanim.cpp", 3090, 0, "%s", "tree->info_usage");
        if (!infoIndex || infoIndex >= 0x1000)
            MyAssertHandler(
                ".\\xanim\\xanim.cpp",
                3093,
                0,
                "%s\n\t(infoIndex) = %i",
                "(infoIndex && (infoIndex < 4096))",
                infoIndex);
        g_xAnimInfo[0].next = g_xAnimInfo[infoIndex].next;
        next = g_xAnimInfo[0].next;
        if (g_xAnimInfo[0].next >= 0x1000u)
            MyAssertHandler(".\\xanim\\xanim.cpp", 3096, 0, "%s\n\t(next) = %i", "(next < 4096)", g_xAnimInfo[0].next);
        g_xAnimInfo[next].prev = 0;
        info = &g_xAnimInfo[infoIndex];
        if (animIndex >= tree->anims->size)
            MyAssertHandler(
                ".\\xanim\\xanim.cpp",
                3104,
                0,
                "animIndex < tree->anims->size\n\t%i, %i",
                animIndex,
                tree->anims->size);
        prev = 0;
        if (after)
        {
            for (nexta = g_xAnimInfo[parentInfoIndex].children; nexta; nexta = childInfoa->next)
            {
                childInfoa = &g_xAnimInfo[nexta];
                if (!childInfoa->inuse)
                    MyAssertHandler(".\\xanim\\xanim.cpp", 3127, 0, "%s", "childInfo->inuse");
                if (childInfoa->animIndex > animIndex)
                    break;
                prev = nexta;
            }
        }
        else
        {
            for (nexta = g_xAnimInfo[parentInfoIndex].children; nexta; nexta = childInfo->next)
            {
                childInfo = &g_xAnimInfo[nexta];
                if (!childInfo->inuse)
                    MyAssertHandler(".\\xanim\\xanim.cpp", 3114, 0, "%s", "childInfo->inuse");
                if (childInfo->animIndex >= animIndex)
                    break;
                prev = nexta;
            }
        }
        if (animIndex >= tree->anims->size)
            MyAssertHandler(".\\xanim\\xanim.cpp", 3135, 0, "%s", "animIndex < tree->anims->size");
        info->prev = prev;
        info->next = nexta;
        info->animIndex = animIndex;
        anim = &tree->anims->entries[animIndex];
        info->children = 0;
        info->parent = parentInfoIndex;
        info->animToModel = animToModel;
        info->parts = anim->parts;
        if (info->animParent.children != anim->animParent.children)
            MyAssertHandler(".\\xanim\\xanim.cpp", 3146, 0, "%s", "info->animParent.children == anim->animParent.children");
        if (info->animParent.flags != anim->animParent.flags)
            MyAssertHandler(".\\xanim\\xanim.cpp", 3147, 0, "%s", "info->animParent.flags == anim->animParent.flags");
        if (animIndex >= tree->anims->size)
            MyAssertHandler(".\\xanim\\xanim.cpp", 3149, 0, "%s", "animIndex < tree->anims->size");
        if (info->inuse)
            MyAssertHandler(".\\xanim\\xanim.cpp", 3152, 0, "%s", "!info->inuse");
        info->inuse = 1;
        info->tree = tree;
        if (nexta)
            g_xAnimInfo[nexta].prev = infoIndex;
        if (prev)
        {
            g_xAnimInfo[prev].next = infoIndex;
        }
        else if (parentInfoIndex)
        {
            g_xAnimInfo[parentInfoIndex].children = infoIndex;
        }
        else
        {
            tree->children = infoIndex;
        }
        return infoIndex;
    }
    else
    {
        Com_Error(ERR_DROP, "exceeded maximum number of anim info");
        return 0;
    }
}

unsigned int XAnimAllocInfoIndex(DObj_s *obj, unsigned int animIndex, int after)
{
    unsigned __int16 prev; // [esp-Ch] [ebp-420h]
    XModelNameMap modelMap[256]; // [esp-8h] [ebp-41Ch] BYREF
    unsigned int parentInfoIndex; // [esp+3F8h] [ebp-1Ch]
    unsigned int parentAnimIndex; // [esp+3FCh] [ebp-18h]
    const XAnimEntry *animEntry; // [esp+400h] [ebp-14h]
    XAnimTree_s *tree; // [esp+404h] [ebp-10h]

    tree = obj->tree;
    iassert(tree);
    if (animIndex)
    {
        animEntry = &tree->anims->entries[animIndex];
        parentAnimIndex = animEntry->parent;
        parentInfoIndex = XAnimEnsureGoalWeightParent(obj, parentAnimIndex);
        if (IsLeafNode(animEntry))
        {
            PROF_SCOPED("XAnimSetModel");
            XAnimInitModelMap(obj->models, obj->numModels, modelMap);
            prev = XAnimGetAnimMap(animEntry->parts, modelMap).prev;
        }
        else
        {
            prev = 0;
        }
    }
    else
    {
        parentInfoIndex = 0;
        prev = 0;
    }
    return XAnimAllocInfoWithParent(tree, prev, animIndex, parentInfoIndex, after);
}

unsigned int __cdecl XAnimEnsureGoalWeightParent(DObj_s* obj, unsigned int animIndex)
{
    XAnimInfo* infoa; // [esp+0h] [ebp-14h]
    XAnimInfo* info; // [esp+0h] [ebp-14h]
    XAnimTree_s* tree; // [esp+4h] [ebp-10h]
    unsigned int parentInfoIndex; // [esp+8h] [ebp-Ch]
    unsigned int infoIndex; // [esp+Ch] [ebp-8h]
    unsigned int infoIndexa; // [esp+Ch] [ebp-8h]

    tree = obj->tree;
    if (!obj->tree)
        MyAssertHandler(".\\xanim\\xanim.cpp", 3192, 0, "%s", "tree");
    if (animIndex)
    {
        parentInfoIndex = XAnimEnsureGoalWeightParent(obj, tree->anims->entries[animIndex].parent);
        for (infoIndexa = g_xAnimInfo[parentInfoIndex].children; infoIndexa; infoIndexa = info->next)
        {
            info = &g_xAnimInfo[infoIndexa];
            if (!info->inuse)
                MyAssertHandler(".\\xanim\\xanim.cpp", 3218, 0, "%s", "info->inuse");
            if (info->animIndex == animIndex)
                return infoIndexa;
        }
        infoIndex = XAnimAllocInfoWithParent(tree, 0, animIndex, parentInfoIndex, 0);
        XAnimInitInfo(&g_xAnimInfo[infoIndex]);
    }
    else
    {
        if (tree->children)
            return tree->children;
        infoIndex = XAnimAllocInfoWithParent(tree, 0, 0, 0, 0);
        infoa = &g_xAnimInfo[infoIndex];
        XAnimInitInfo(infoa);
        infoa->state.goalWeight = 1.0;
        infoa->state.weight = 1.0;
        infoa->state.goalTime = 0.0;
        infoa->state.rate = 1.0;
    }
    return infoIndex;
}

void __cdecl XAnimClearGoalWeightInternal(
    XAnimTree_s* tree,
    unsigned int infoIndex,
    float blendTime,
    int forceBlendTime)
{
    float v4; // [esp+0h] [ebp-14h]
    float v5; // [esp+4h] [ebp-10h]
    float goalTime; // [esp+8h] [ebp-Ch]
    XAnimInfo* info; // [esp+10h] [ebp-4h]

    if (!tree)
        MyAssertHandler(".\\xanim\\xanim.cpp", 3286, 0, "%s", "tree");
    if (!tree->anims)
        MyAssertHandler(".\\xanim\\xanim.cpp", 3287, 0, "%s", "tree->anims");
    if (!infoIndex || infoIndex >= 0x1000)
        MyAssertHandler(
            ".\\xanim\\xanim.cpp",
            3288,
            0,
            "%s\n\t(infoIndex) = %i",
            "(infoIndex && (infoIndex < 4096))",
            infoIndex);
    info = &g_xAnimInfo[infoIndex];
    if (!info->inuse)
        MyAssertHandler(".\\xanim\\xanim.cpp", 3291, 0, "%s", "info->inuse");
    if (info->state.goalWeight == 0.0 && !forceBlendTime)
    {
        goalTime = info->state.goalTime;
        v5 = blendTime - goalTime;
        if (v5 < 0.0)
            v4 = blendTime;
        else
            v4 = goalTime;
        info->state.goalTime = v4;
    }
    else
    {
        info->state.goalTime = blendTime;
    }
    info->state.goalWeight = 0.0;
    if (blendTime == 0.0)
    {
        info->state.weight = 0.0;
        info->state.instantWeightChange = 1;
    }
    XAnimClearServerNotify(info);
}

void __cdecl XAnimClearTreeGoalWeightsInternal(
    XAnimTree_s* tree,
    unsigned int infoIndex,
    float blendTime,
    int forceBlendTime)
{
    BOOL v4; // [esp+8h] [ebp-8h]
    unsigned int animIndex; // [esp+Ch] [ebp-4h]
    unsigned int infoIndexa; // [esp+1Ch] [ebp+Ch]

    XAnimClearGoalWeightInternal(tree, infoIndex, blendTime, forceBlendTime);
    animIndex = 0;
    for (infoIndexa = g_xAnimInfo[infoIndex].children; infoIndexa; infoIndexa = g_xAnimInfo[infoIndexa].next)
    {
        v4 = forceBlendTime && animIndex != g_xAnimInfo[infoIndexa].animIndex;
        XAnimClearTreeGoalWeightsInternal(tree, infoIndexa, blendTime, v4);
        animIndex = g_xAnimInfo[infoIndexa].animIndex;
    }
}

void __cdecl XAnimClearTreeGoalWeights(XAnimTree_s* tree, unsigned int animIndex, float blendTime)
{
    unsigned int infoIndex; // [esp+8h] [ebp-4h]

    infoIndex = XAnimGetInfoIndex(tree, animIndex);
    if (infoIndex)
        XAnimClearTreeGoalWeightsInternal(tree, infoIndex, blendTime, 1);
}

void __cdecl XAnimClearTreeGoalWeightsStrict(XAnimTree_s* tree, unsigned int animIndex, float blendTime)
{
    int numAnims; // [esp+4h] [ebp-Ch]
    const XAnimEntry* anim; // [esp+8h] [ebp-8h]
    int i; // [esp+Ch] [ebp-4h]

    if (!tree)
        MyAssertHandler(".\\xanim\\xanim.cpp", 3355, 0, "%s", "tree");
    if (!tree->anims)
        MyAssertHandler(".\\xanim\\xanim.cpp", 3356, 0, "%s", "tree->anims");
    if (animIndex >= tree->anims->size)
        MyAssertHandler(".\\xanim\\xanim.cpp", 3357, 0, "%s", "animIndex < tree->anims->size");
    anim = &tree->anims->entries[animIndex];
    numAnims = anim->numAnims;
    for (i = 0; i < numAnims; ++i)
        XAnimClearTreeGoalWeights(tree, i + anim->animParent.children, blendTime);
}

void __cdecl XAnimClearGoalWeightKnobInternal(
    XAnimTree_s* tree,
    unsigned int infoIndex,
    float goalWeight,
    float goalTime)
{
    float v4; // [esp+8h] [ebp-38h]
    float v5; // [esp+Ch] [ebp-34h]
    float v6; // [esp+10h] [ebp-30h]
    float v7; // [esp+14h] [ebp-2Ch]
    float v8; // [esp+18h] [ebp-28h]
    unsigned __int16 children; // [esp+1Eh] [ebp-22h]
    float blendTime; // [esp+20h] [ebp-20h]
    float weight; // [esp+30h] [ebp-10h]
    float largestWeightDiff; // [esp+34h] [ebp-Ch]
    unsigned int childInfoIndex; // [esp+3Ch] [ebp-4h]
    unsigned int childInfoIndexa; // [esp+3Ch] [ebp-4h]

    if (!tree)
        MyAssertHandler(".\\xanim\\xanim.cpp", 3377, 0, "%s", "tree");
    if (!tree->anims)
        MyAssertHandler(".\\xanim\\xanim.cpp", 3378, 0, "%s", "tree->anims");
    if (!infoIndex || infoIndex >= 0x1000)
        MyAssertHandler(
            ".\\xanim\\xanim.cpp",
            3379,
            0,
            "%s\n\t(infoIndex) = %i",
            "(infoIndex && (infoIndex < 4096))",
            infoIndex);
    largestWeightDiff = 0.0;
    if (g_xAnimInfo[infoIndex].parent)
        children = g_xAnimInfo[g_xAnimInfo[infoIndex].parent].children;
    else
        children = tree->children;
    for (childInfoIndex = children; childInfoIndex; childInfoIndex = g_xAnimInfo[childInfoIndex].next)
    {
        if (childInfoIndex >= 0x1000)
            MyAssertHandler(
                ".\\xanim\\xanim.cpp",
                3388,
                0,
                "%s\n\t(childInfoIndex) = %i",
                "(childInfoIndex && (childInfoIndex < 4096))",
                childInfoIndex);
        weight = g_xAnimInfo[childInfoIndex].state.weight;
        if (childInfoIndex == infoIndex)
        {
            v8 = goalWeight - weight;
            v7 = fabs(v8);
            v6 = v7;
        }
        else
        {
            v6 = weight;
        }
        v5 = largestWeightDiff - v6;
        if (v5 < 0.0)
            v4 = v6;
        else
            v4 = largestWeightDiff;
        largestWeightDiff = v4;
    }
    for (childInfoIndexa = children; childInfoIndexa; childInfoIndexa = g_xAnimInfo[childInfoIndexa].next)
    {
        if (childInfoIndexa != infoIndex)
        {
            blendTime = largestWeightDiff * goalTime;
            XAnimClearGoalWeightInternal(tree, childInfoIndexa, blendTime, 0);
        }
    }
}

int __cdecl XAnimSetCompleteGoalWeightNode(
    XAnimTree_s* tree,
    unsigned int infoIndex,
    float goalWeight,
    float goalTime,
    float rate,
    unsigned int notifyName,
    unsigned int notifyType)
{
    unsigned int parentInfoIndex; // [esp+14h] [ebp-8h]
    int error; // [esp+18h] [ebp-4h]

    error = XAnimSetGoalWeightNode(tree, infoIndex, goalWeight, goalTime, rate, notifyName, notifyType);
    parentInfoIndex = infoIndex;
    while (1)
    {
        parentInfoIndex = g_xAnimInfo[parentInfoIndex].parent;
        if (!parentInfoIndex)
            break;
        iassert(parentInfoIndex && (parentInfoIndex < 4096));
        if (g_xAnimInfo[parentInfoIndex].state.goalWeight == 0.0)
            XAnimSetGoalWeightNode(tree, parentInfoIndex, 1.0, goalTime, 1.0, 0, 0);
    }
    return error;
}

int __cdecl XAnimSetGoalWeightKnobAll(
    DObj_s* obj,
    unsigned int animIndex,
    unsigned int rootIndex,
    float goalWeight,
    float goalTime,
    float rate,
    unsigned int notifyName,
    int bRestart)
{
    XAnimTree_s* tree; // [esp+60h] [ebp-Ch]
    int error; // [esp+64h] [ebp-8h]
    unsigned int infoIndex; // [esp+68h] [ebp-4h]

    iassert(animIndex != rootIndex);
    iassert(obj);
    error = XAnimSetGoalWeightKnob(obj, animIndex, goalWeight, goalTime, rate, notifyName, bRestart);
    PROF_SCOPED("XAnimSetGoalWeight");
    tree = obj->tree;
    infoIndex = XAnimGetInfoIndex(obj->tree, animIndex);
    iassert(infoIndex);

    while (1)
    {
        infoIndex = g_xAnimInfo[infoIndex].parent;
        if (!infoIndex)
        {
            return 1;
        }
        if (g_xAnimInfo[infoIndex].animIndex == rootIndex)
            break;
        if (bRestart)
            infoIndex = XAnimRestart(tree, infoIndex, goalTime);
        XAnimClearGoalWeightKnobInternal(tree, infoIndex, 1.0, goalTime);
        XAnimSetGoalWeightNode(tree, infoIndex, 1.0, goalTime, 1.0, 0, 0);
    }

    return error;
}

int __cdecl XAnimSetGoalWeightKnob(
    DObj_s* obj,
    unsigned int animIndex,
    float goalWeight,
    float goalTime,
    float rate,
    unsigned int notifyName,
    int bRestart)
{
    XAnimTree_s* tree; // [esp+44h] [ebp-Ch]
    unsigned int infoIndex; // [esp+48h] [ebp-8h]
    int error; // [esp+4Ch] [ebp-4h]

    PROF_SCOPED("XAnimSetGoalWeight");

    iassert(obj);

    if (goalWeight < EQUAL_EPSILON)
        goalWeight = 0.0f;

    tree = obj->tree;
    infoIndex = XAnimGetInfoIndex(obj->tree, animIndex);
    if (infoIndex)
    {
        if (bRestart)
            infoIndex = XAnimRestart(tree, infoIndex, goalTime);
    }
    else
    {
        infoIndex = XAnimAllocInfoIndex(obj, animIndex, 0);
        XAnimInitInfo(&g_xAnimInfo[infoIndex]);
    }
    XAnimClearGoalWeightKnobInternal(tree, infoIndex, goalWeight, goalTime);
    error = XAnimSetGoalWeightNode(tree, infoIndex, goalWeight, goalTime, rate, notifyName, 0);
    return error;
}

void __cdecl XAnimClearTree(XAnimTree_s* tree)
{
    iassert(tree);
    if (tree->children)
    {
        XAnimFreeInfo(tree, tree->children);
        iassert(!tree->children);
    }
    iassert(!tree->info_usage);
}

int __cdecl XAnimSetGoalWeightNode(
    XAnimTree_s* tree,
    unsigned int infoIndex,
    float goalWeight,
    float goalTime,
    float rate,
    unsigned int notifyName,
    unsigned int notifyType)
{
    double v7; // st7
    float v9; // [esp+0h] [ebp-28h]
    float v10; // [esp+4h] [ebp-24h]
    float v11; // [esp+8h] [ebp-20h]
    float v12; // [esp+Ch] [ebp-1Ch]
    float v13; // [esp+10h] [ebp-18h]
    float v14; // [esp+14h] [ebp-14h]
    XAnimInfo* info; // [esp+1Ch] [ebp-Ch]
    unsigned int animIndex; // [esp+20h] [ebp-8h]
    float weightDiff; // [esp+24h] [ebp-4h]
    float goalTimea; // [esp+3Ch] [ebp+14h]

    iassert(tree);
    iassert(tree->anims);
    iassert(!goalWeight || goalWeight >= WEIGHT_EPSILON);
    iassert(goalTime >= 0);
    iassert(rate >= 0);
    iassert((!notifyName && !notifyType) || goalWeight);
    iassert(infoIndex && (infoIndex < 4096));

    info = &g_xAnimInfo[infoIndex];
    XAnimClearServerNotify(info);
    iassert(info->inuse);
    animIndex = info->animIndex;
    iassert(animIndex < tree->anims->size);
    if (!animIndex)
    {
        goalWeight = 1.0;
        goalTime = 0.0;
        rate = 1.0;
    }
    if (goalTime == 0.0)
    {
        info->state.weight = goalWeight;
        info->state.goalTime = 0.0;
        info->state.instantWeightChange = 1;
    }
    else
    {
        if (info->state.weight == 0.0)
            info->state.weight = goalWeight * EQUAL_EPSILON;
        weightDiff = info->state.weight - goalWeight;
        if (weightDiff == 0.0 || (info->state.goalWeight - goalWeight) * weightDiff > 0.0)
        {
            info->state.goalTime = goalTime;
        }
        else
        {
            v14 = info->state.goalTime;
            v12 = goalTime - v14;
            if (v12 < 0.0)
                v11 = goalTime;
            else
                v11 = v14;
            info->state.goalTime = v11;
        }
        if (goalWeight != 0.0)
        {
            if (info->state.weight > (double)goalWeight)
                v7 = (info->state.weight - goalWeight) / info->state.weight * goalTime;
            else
                v7 = (goalWeight - info->state.weight) / goalWeight * goalTime;
            goalTimea = v7;
            v13 = info->state.goalTime;
            v10 = v13 - goalTimea;
            if (v10 < 0.0)
                v9 = v7;
            else
                v9 = v13;
            info->state.goalTime = v9;
        }
    }
    info->state.goalWeight = goalWeight;
    info->state.rate = rate;
    info->notifyName = notifyName;
    if (notifyName)
        SL_AddRefToString(notifyName);
    info->notifyType = notifyType;
    iassert(info->notifyIndex == -1);
    if (notifyName && !info->animToModel && (info->animParent.flags & 3) != 0)
    {
        iassert(goalWeight);
        info->notifyChild = XAnimGetDescendantWithGreatestWeight(tree, infoIndex);
        if (!info->notifyChild)
            return 2;
    }
    else
    {
        info->notifyChild = 0;
    }
    return 0;
}

unsigned int __cdecl XAnimGetDescendantWithGreatestWeight(const XAnimTree_s* tree, unsigned int infoIndex)
{
    float testWeight; // [esp+0h] [ebp-14h]
    unsigned int result; // [esp+4h] [ebp-10h]
    XAnimInfo* info; // [esp+8h] [ebp-Ch]
    unsigned int test; // [esp+Ch] [ebp-8h]
    float bestWeight; // [esp+10h] [ebp-4h]
    unsigned int infoIndexa; // [esp+20h] [ebp+Ch]

    info = &g_xAnimInfo[infoIndex];
    iassert(info->inuse);
    if (info->animToModel)
    {
        iassert(info->state.goalWeight);
        return info->animIndex;
    }
    else
    {
        bestWeight = 0.0;
        result = 0;
        for (infoIndexa = info->children; infoIndexa; infoIndexa = g_xAnimInfo[infoIndexa].next)
        {
            testWeight = g_xAnimInfo[infoIndexa].state.goalWeight;
            if (bestWeight < (double)testWeight)
            {
                test = XAnimGetDescendantWithGreatestWeight(tree, infoIndexa);
                if (test)
                {
                    bestWeight = testWeight;
                    result = test;
                }
            }
        }
        return result;
    }
}

void __cdecl XAnimSetupSyncNodes(XAnim_s* anims)
{
    XAnimSetupSyncNodes_r(anims, 0);
}

void __cdecl XAnimSetupSyncNodes_r(XAnim_s* anims, unsigned int animIndex)
{
    int flag; // [esp+0h] [ebp-14h]
    int numAnims; // [esp+4h] [ebp-10h]
    int i; // [esp+10h] [ebp-4h]
    int ia; // [esp+10h] [ebp-4h]

    if (!IsLeafNode(&anims->entries[animIndex]))
    {
        numAnims = anims->entries[animIndex].numAnims;
        flag = anims->entries[animIndex].animParent.flags & 3;
        if (flag)
        {
            if (flag == 3)
                Com_Error(ERR_DROP, "animation cannot be sync looping and sync nonlooping");
            anims->entries[animIndex].animParent.flags |= 4u;
            for (i = 0; i < numAnims; ++i)
                XAnimFillInSyncNodes_r(anims, i + anims->entries[animIndex].animParent.children, flag == 1);
        }
        else
        {
            for (ia = 0; ia < numAnims; ++ia)
                XAnimSetupSyncNodes_r(anims, ia + anims->entries[animIndex].animParent.children);
        }
    }
}

void __cdecl XAnimFillInSyncNodes_r(XAnim_s* anims, unsigned int animIndex, bool bLoop)
{
    XAnimParts* Data_FastFile; // eax
    char* AnimDebugName; // eax
    const char* debugName; // [esp-4h] [ebp-24h]
    bool IsXAssetDefault; // [esp+7h] [ebp-19h]
    XAnimParts* parts; // [esp+8h] [ebp-18h]
    int numAnims; // [esp+10h] [ebp-10h]
    XAnimEntry* anim; // [esp+14h] [ebp-Ch]
    int i; // [esp+18h] [ebp-8h]
    int count; // [esp+1Ch] [ebp-4h]

    anim = &anims->entries[animIndex];
    if (IsLeafNode(anim))
    {
        if (anims->entries[animIndex].parts->bLoop != bLoop)
        {
            parts = anims->entries[animIndex].parts;
            if (useFastFile->current.enabled)
                IsXAssetDefault = DB_IsXAssetDefault(ASSET_TYPE_XANIMPARTS, parts->name);
            else
                IsXAssetDefault = parts->isDefault;
            if (IsXAssetDefault)
            {
                if (!useFastFile->current.enabled)
                    XAnimPrecache("void_loop", (void*(*)(int))Hunk_AllocXAnimPrecache);

                if (useFastFile->current.enabled)
                    Data_FastFile = XAnimFindData_FastFile("void_loop");
                else
                    Data_FastFile = XAnimFindData_LoadObj("void_loop");

                anims->entries[animIndex].parts = Data_FastFile;
                if (!anims->entries[animIndex].parts)
                    Com_Error(ERR_DROP, "Cannot find xanim/%s. This is a default xanim file that you should have.", "void_loop");
            }
            else if (bLoop)
            {
                debugName = anims->debugName;
                AnimDebugName = XAnimGetAnimDebugName(anims, animIndex);
                Com_Error(ERR_DROP, "animation %s in %s cannot be sync looping and nonlooping", AnimDebugName, debugName);
            }
            else
            {
                Com_Error(ERR_DROP, "animation %s in %s cannot be sync nonlooping and looping", XAnimGetAnimDebugName(anims, animIndex), anims->debugName);
            }
        }
    }
    else
    {
        if ((anims->entries[animIndex].animParent.flags & 3) != 0)
        {
            count = 0;
            do
            {
                ++count;
                anim = &anims->entries[anim->animParent.children];
            } while (!IsLeafNode(anim));
            Com_Error(ERR_DROP, "duplicate specification of animation sync in %s %d nodes above %s", anims->debugName, count, XAnimGetAnimDebugName(anims, animIndex));
        }
        anim->animParent.flags |= 2 - bLoop;
        numAnims = anim->numAnims;
        for (i = 0; i < numAnims; ++i)
            XAnimFillInSyncNodes_r(anims, i + anim->animParent.children, bLoop);
    }
}

bool __cdecl XAnimHasTime(const XAnim_s* anims, unsigned int animIndex)
{
    return IsLeafNode(&anims->entries[animIndex]) || (anims->entries[animIndex].animParent.flags & 3) != 0;
}

BOOL __cdecl XAnimIsPrimitive(XAnim_s* anims, unsigned int animIndex)
{
    return anims->entries[animIndex].numAnims == 0;
}

void __cdecl XAnimSetTime(XAnimTree_s *tree, unsigned int animIndex, float time)
{
    XAnimState *state; // [esp+18h] [ebp-10h]
    unsigned int infoIndex; // [esp+20h] [ebp-8h]
    const XAnimEntry *anim; // [esp+24h] [ebp-4h]

    iassert(tree);
    iassert(tree->anims);
    iassert(animIndex < tree->anims->size);

    infoIndex = XAnimGetInfoIndex(tree, animIndex);

    if (infoIndex)
    {
        anim = &tree->anims->entries[animIndex];

        iassert(IsLeafNode(anim));
        iassert(time >= 0.0f && time <= 1.0f);
        iassert(anim->parts->bLoop ? (time < 1.0f) : (time <= 1.0f));
        iassert(infoIndex && (infoIndex < 4096));

        state = &g_xAnimInfo[infoIndex].state;
        state->currentAnimTime = time;
        state->cycleCount = 0;
        state->oldTime = time;
        state->oldCycleCount = 0;
        g_xAnimInfo[infoIndex].notifyIndex = -1;
    }
}

void __cdecl XAnimUpdateServerNotifyIndex(XAnimInfo* info, const XAnimParts* parts)
{
    iassert(parts);
    iassert(info->notifyName);
    iassert(info->notifyIndex == -1);

    if (info->state.currentAnimTime != 1.0f)
        info->notifyIndex = XAnimGetNextNotifyIndex(parts, info->state.currentAnimTime);
}

unsigned int __cdecl XAnimRestart(XAnimTree_s* tree, unsigned int infoIndex, float goalTime)
{
    unsigned int parentInfoIndex; // [esp+8h] [ebp-10h]
    XAnimInfo* parentInfo; // [esp+Ch] [ebp-Ch]
    unsigned int parentAnimIndex; // [esp+10h] [ebp-8h]
    const XAnimEntry* anim; // [esp+14h] [ebp-4h]

    iassert(tree);
    iassert(infoIndex && (infoIndex < 4096));

    for (parentInfoIndex = infoIndex; parentInfoIndex; parentInfoIndex = parentInfo->parent)
    {
        iassert(parentInfoIndex && (parentInfoIndex < 4096));
        parentInfo = &g_xAnimInfo[parentInfoIndex];
        iassert(parentInfo->inuse);
        parentAnimIndex = parentInfo->animIndex;
        iassert(parentAnimIndex < tree->anims->size);
        anim = &tree->anims->entries[parentAnimIndex];
        if (!IsLeafNode(anim) && (anim->animParent.flags & 4) != 0)
        {
            XAnimInitTime(tree, parentInfoIndex, goalTime);
            return XAnimGetInfoIndex(tree, g_xAnimInfo[infoIndex].animIndex);
        }
    }

    iassert(g_xAnimInfo[infoIndex].inuse);

    if (g_xAnimInfo[infoIndex].animToModel)
        return XAnimInitTime(tree, infoIndex, goalTime);
    else
        return infoIndex;
}

int __cdecl XAnimSetGoalWeight(
    DObj_s* obj,
    unsigned int animIndex,
    float goalWeight,
    float goalTime,
    float rate,
    unsigned int notifyName,
    unsigned int notifyType,
    int bRestart)
{
    XAnimTree_s* tree; // [esp+60h] [ebp-Ch]
    int error; // [esp+64h] [ebp-8h]
    unsigned int infoIndex; // [esp+68h] [ebp-4h]

    PROF_SCOPED("XAnimSetGoalWeight");
    iassert(obj);

    if (goalWeight < EQUAL_EPSILON)
        goalWeight = 0.0;

    tree = obj->tree;
    infoIndex = XAnimGetInfoIndex(obj->tree, animIndex);
    if (infoIndex)
    {
        if (bRestart)
            infoIndex = XAnimRestart(tree, infoIndex, goalTime);
    }
    else
    {
        if (goalWeight == 0.0)
        {
            return 0;
        }
        infoIndex = XAnimAllocInfoIndex(obj, animIndex, 0);
        XAnimInitInfo(&g_xAnimInfo[infoIndex]);
    }
    error = XAnimSetGoalWeightNode(tree, infoIndex, goalWeight, goalTime, rate, notifyName, notifyType);
    return error;
}

void __cdecl XAnimSetAnimRate(XAnimTree_s* tree, unsigned int animIndex, float rate)
{
    unsigned int infoIndex; // [esp+0h] [ebp-4h]

    infoIndex = XAnimGetInfoIndex(tree, animIndex);
    if (infoIndex)
    {
        iassert(tree);
        iassert(tree->anims);
        iassert(animIndex < tree->anims->size);
        iassert(rate >= 0.0f);
        iassert(infoIndex && (infoIndex < 4096));

        g_xAnimInfo[infoIndex].state.rate = rate;
    }
}

bool __cdecl XAnimIsLooped(const XAnim_s* anims, unsigned int animIndex)
{
    iassert(anims);

    if (IsLeafNode(&anims->entries[animIndex]))
        return anims->entries[animIndex].parts->bLoop;
    else
        return (anims->entries[animIndex].animParent.flags & 1) != 0;
}

char __cdecl XAnimNotetrackExists(const XAnim_s* anims, unsigned int animIndex, unsigned int name)
{
    const XAnimNotifyInfo* notify; // [esp+0h] [ebp-10h]
    int notifyIndex; // [esp+4h] [ebp-Ch]
    XAnimParts* parts; // [esp+Ch] [ebp-4h]

    parts = anims->entries[animIndex].parts;
    iassert(parts);
    notify = parts->notify;

    if (!notify)
        return 0;

    for (notifyIndex = 0; notifyIndex < parts->notifyCount; ++notifyIndex)
    {
        if (notify->name == name)
            return 1;
        ++notify;
    }

    return 0;
}

void __cdecl XAnimAddNotetrackTimesToScriptArray(const XAnim_s* anims, unsigned int animIndex, unsigned int name)
{
    const XAnimNotifyInfo* notify; // [esp+4h] [ebp-10h]
    int notifyIndex; // [esp+8h] [ebp-Ch]
    XAnimParts* parts; // [esp+10h] [ebp-4h]

    parts = anims->entries[animIndex].parts;
    iassert(parts);
    notify = parts->notify;

    if (notify)
    {
        for (notifyIndex = 0; notifyIndex < parts->notifyCount; ++notifyIndex)
        {
            if (notify->name == name)
            {
                Scr_AddFloat(notify->time);
                Scr_AddArray();
            }
            ++notify;
        }
    }
}

int __cdecl XAnimSetCompleteGoalWeight(
    DObj_s* obj,
    unsigned int animIndex,
    float goalWeight,
    float goalTime,
    float rate,
    unsigned int notifyName,
    unsigned int notifyType,
    int bRestart)
{
    XAnimTree_s* tree; // [esp+44h] [ebp-Ch]
    unsigned int infoIndex; // [esp+48h] [ebp-8h]
    int error; // [esp+4Ch] [ebp-4h]

    PROF_SCOPED("XAnimSetGoalWeight");

    iassert(obj);

    if (goalWeight < EQUAL_EPSILON)
        goalWeight = 0.0;

    tree = obj->tree;
    infoIndex = XAnimGetInfoIndex(obj->tree, animIndex);
    if (infoIndex)
    {
        if (bRestart)
            infoIndex = XAnimRestart(tree, infoIndex, goalTime);
    }
    else
    {
        infoIndex = XAnimAllocInfoIndex(obj, animIndex, 0);
        XAnimInitInfo(&g_xAnimInfo[infoIndex]);
    }
    error = XAnimSetCompleteGoalWeightNode(tree, infoIndex, goalWeight, goalTime, rate, notifyName, notifyType);
    return error;
}

void __cdecl XAnimCloneAnimInfo(const XAnimInfo* from, XAnimInfo* to)
{
    iassert(to->animIndex == from->animIndex);
    qmemcpy(&to->state, &from->state, sizeof(to->state));
    to->notifyChild = from->notifyChild;
    to->notifyIndex = from->notifyIndex;
    to->notifyName = from->notifyName;
    to->notifyType = from->notifyType;

    if (to->notifyName)
        SL_AddRefToString(to->notifyName);
}

void __cdecl XAnimCloneAnimTree(const XAnimTree_s* from, XAnimTree_s* to)
{
    PROF_SCOPED("XAnimCloneAnimTree");

    iassert(from);
    iassert(from->anims);
    iassert(from->anims->size);
    iassert(to);
    iassert(to->anims == from->anims);

    if (to->children)
        XAnimFreeInfo(to, to->children);

    iassert(!to->children);

    if (from->children)
        XAnimCloneAnimTree_r(from, to, from->children, 0);
}

void __cdecl XAnimCloneAnimTree_r(
    const XAnimTree_s* from,
    XAnimTree_s* to,
    unsigned int fromInfoIndex,
    unsigned int toInfoParentIndex)
{
    unsigned int toInfoIndex; // [esp+4h] [ebp-10h]
    unsigned int fromChildInfoIndex; // [esp+8h] [ebp-Ch]
    XAnimInfo* fromInfo; // [esp+Ch] [ebp-8h]
    unsigned int animToModel; // [esp+10h] [ebp-4h]

    iassert(from);
    iassert(from->anims);
    iassert(from->anims->size);
    iassert(to);
    iassert(to->anims == from->anims);
    iassert(fromInfoIndex && (fromInfoIndex < 4096));

    fromInfo = &g_xAnimInfo[fromInfoIndex];
    iassert(fromInfo->inuse);

    animToModel = fromInfo->animToModel;

    if (animToModel)
        SL_AddRefToString(animToModel);

    toInfoIndex = XAnimAllocInfoWithParent(to, animToModel, fromInfo->animIndex, toInfoParentIndex, 1);
    XAnimCloneAnimInfo(fromInfo, &g_xAnimInfo[toInfoIndex]);

    for (fromChildInfoIndex = fromInfo->children;
        fromChildInfoIndex;
        fromChildInfoIndex = g_xAnimInfo[fromChildInfoIndex].next)
    {
        XAnimCloneAnimTree_r(from, to, fromChildInfoIndex, toInfoIndex);
    }
}

XAnimInfo* __cdecl GetAnimInfo(int infoIndex)
{
    iassert(infoIndex > 0 && infoIndex < 4096);

    return &g_xAnimInfo[infoIndex];
}
