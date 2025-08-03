#include "scr_animtree.h"

#include <qcommon/mem_track.h>
#include <universal/q_parse.h>

#include "scr_main.h"
#include "scr_parser.h"
#include "scr_variable.h"
#include "scr_vm.h"

enum
{
    SCR_XANIM_CLIENT = 0,
    SCR_XANIM_SERVER = 1,
    SCR_XANIM_COUNT
};
enum
{
    ANIMTREE_NAMES,
    ANIMTREE_XANIM
};

// LWSS: make these flags here
#define ANIMFLAG_LOOPSYNC       1
#define ANIMFLAG_NONLOOPSYNC    2
#define ANIMFLAG_COMPLETE       8
#define ANIMFLAG_ADDITIVE       16

scrAnimGlob_t scrAnimGlob;
scrAnimPub_t scrAnimPub;


void __cdecl TRACK_scr_animtree()
{
    track_static_alloc_internal(&scrAnimGlob, sizeof(scrAnimGlob_t), "scrAnimGlob", 7);
    track_static_alloc_internal(&scrAnimPub, sizeof(scrAnimPub_t), "scrAnimPub", 7);
}

void __cdecl SetAnimCheck(int bAnimCheck)
{
    scrAnimGlob.bAnimCheck = bAnimCheck;
}

void __cdecl Scr_EmitAnimation(char *pos, unsigned int animName, unsigned int sourcePos)
{
    if (scrAnimPub.animTreeNames)
        Scr_EmitAnimationInternal(pos, animName, scrAnimPub.animTreeNames);
    else
        CompileError(sourcePos, "#using_animtree was not specified");
}

void __cdecl Scr_EmitAnimationInternal(char *pos, unsigned int animName, unsigned int names)
{
    unsigned int NewVariable; // eax
    VariableValueInternal_u *value; // [esp+0h] [ebp-10h]
    unsigned int animId; // [esp+4h] [ebp-Ch]
    VariableValue tempValue; // [esp+8h] [ebp-8h] BYREF

    iassert(names);
    animId = FindVariable(names, animName);

    if (animId)
    {
        value = GetVariableValueAddress(animId);
        *(const char **)pos = value->u.codePosValue;
        value->u.codePosValue = pos;
    }
    else
    {
        NewVariable = GetNewVariable(names, animName);
        *(unsigned char**)pos = NULL;
        tempValue.type = VAR_CODEPOS;
        tempValue.u.codePosValue = pos;
        SetVariableValue(NewVariable, &tempValue);
    }
}

int __cdecl Scr_GetAnimsIndex(const XAnim_s *anims)
{
    int i; // [esp+4h] [ebp-4h]

    //for (i = scrAnimPub.xanim_num[1]; i && (const XAnim_s *)scrAnimPub.xanim_num[i - 128] != anims; --i);
    for (i = scrAnimPub.xanim_num[1]; i && scrAnimPub.xanim_lookup[1][i].anims != anims; --i);

    return i;
}

XAnim_s *__cdecl Scr_GetAnims(unsigned int index)
{
    iassert(index && index <= scrAnimPub.xanim_num[SCR_XANIM_SERVER]);
    //return (XAnim_s *)scrAnimPub.xanim_num[index - 128];
    return scrAnimPub.xanim_lookup[1][index].anims;
}

void __cdecl Scr_UsingTree(const char *filename, unsigned int sourcePos)
{
    if (Scr_IsIdentifier(filename))
        scrAnimPub.animTreeNames = Scr_UsingTreeInternal(filename, &scrAnimPub.animTreeIndex, 1);
    else
        CompileError(sourcePos, "bad anim tree name");
}

unsigned int __cdecl Scr_UsingTreeInternal(const char *filename, unsigned int *index, int user)
{
    unsigned int Variable; // eax
    unsigned int name; // [esp+0h] [ebp-18h]
    unsigned int names; // [esp+8h] [ebp-10h]
    unsigned int i; // [esp+Ch] [ebp-Ch]
    unsigned int fileId; // [esp+10h] [ebp-8h]
    unsigned int id; // [esp+14h] [ebp-4h]
    unsigned int ida; // [esp+14h] [ebp-4h]

    iassert(scrAnimPub.animtree_loading);
    iassert(Scr_IsIdentifier(filename));

    name = Scr_CreateCanonicalFilename(filename).prev;
    id = FindVariable(scrAnimPub.animtrees, name);
    if (id)
    {
        fileId = FindObject(id);
        *index = 0;
        for (i = 1; i <= scrAnimPub.xanim_num[user]; ++i)
        {
            if (scrAnimGlob.using_xanim_lookup[user][i] == id)
            {
                *index = i;
                break;
            }
        }
        iassert(*index);
    }
    else
    {
        ida = GetNewVariable(scrAnimPub.animtrees, name);
        fileId = GetObject(ida);

        scrAnimPub.xanim_num[user]++;

        iassert(scrAnimPub.xanim_num[user] < MAX_XANIMTREE_NUM);

        scrAnimGlob.using_xanim_lookup[user][scrAnimPub.xanim_num[user]] = ida;
        *index = scrAnimPub.xanim_num[user];
    }
    
    iassert(!FindVariable(fileId, ANIMTREE_XANIM));

    Variable = GetVariable(fileId, 0);
    names = GetArray(Variable);
    SL_RemoveRefToString(name);
    return names;
}

void __cdecl Scr_PrecacheAnimationTree(unsigned int parentNode)
{
    VariableValueInternal_u Object; // eax
    unsigned int node; // [esp+0h] [ebp-8h]
    unsigned int name; // [esp+4h] [ebp-4h]

    for (node = FindFirstSibling(parentNode); node; node = FindNextSibling(node))
    {
        name = GetVariableName(node);
        if (name < SL_MAX_STRING_INDEX)
        {
            if (GetValueType(node) == 1)
            {
                Object = FindObject(node);
                Scr_PrecacheAnimationTree(Object.u.stringValue);
            }
            else
            {
                XAnimPrecache(SL_ConvertToString(name), (void *(__cdecl *)(int))Hunk_AllocXAnimPrecache);
            }
        }
    }
}

void __cdecl Scr_LoadAnimTreeAtIndex(unsigned int index, void *(__cdecl *Alloc)(int), int user)
{
    char *v4; // eax
    const char *v5; // eax
    char *v6; // eax
    unsigned int Variable; // eax
    unsigned int animtree_node; // [esp-8h] [ebp-30h]
    XAnim_s *animtree; // [esp+0h] [ebp-28h]
    unsigned int size; // [esp+8h] [ebp-20h]
    unsigned int size2;
    HashEntry_unnamed_type_u name; // [esp+Ch] [ebp-1Ch]
    unsigned int filenameId; // [esp+10h] [ebp-18h]
    unsigned int names; // [esp+14h] [ebp-14h]
    unsigned int namesa; // [esp+14h] [ebp-14h]
    unsigned int fileId; // [esp+18h] [ebp-10h]
    VariableValue tempValue; // [esp+1Ch] [ebp-Ch] BYREF
    unsigned int id; // [esp+24h] [ebp-4h]

    id = scrAnimGlob.using_xanim_lookup[user][index];
    iassert(scrAnimPub.animtree_loading);

    Hunk_CheckTempMemoryClear();
    Hunk_CheckTempMemoryHighClear();

    iassert(GetVariableName(id) < SL_MAX_STRING_INDEX);

    filenameId = (unsigned __int16)GetVariableName(id);
    fileId = FindObject(id);
    iassert(fileId);
    if (!FindVariable(fileId, 1u))
    {
        names = FindVariable(fileId, 0);
        if (names)
        {
            namesa = FindObject(names);

            iassert(namesa);
            iassert(!scrAnimPub.animtree_node);

            scrAnimPub.animtree_node = Scr_AllocArray();
            animtree_node = scrAnimPub.animtree_node;
            if (!Scr_LoadAnimTreeInternal(SL_ConvertToString(filenameId), animtree_node, namesa))
            {
                Com_Error(ERR_DROP, va("unknown anim tree '%s'", SL_ConvertToString(filenameId)));
            }
            Hunk_CheckTempMemoryClear();
            Hunk_CheckTempMemoryHighClear();
            size = Scr_GetAnimTreeSize(scrAnimPub.animtree_node);
            iassert(size);
            animtree = XAnimCreateAnims(SL_ConvertToString(filenameId), size, Alloc);
            name.prev = SL_GetString_("root", 0, 4);
            ConnectScriptToAnim(namesa, 0, filenameId, name.prev, index);
            SL_RemoveRefToString(name.prev);

            if (!useFastFile->current.enabled)
                Scr_PrecacheAnimationTree(scrAnimPub.animtree_node);

            size2 = Scr_CreateAnimationTree(scrAnimPub.animtree_node, namesa, animtree, 1, "root", 0, filenameId, index, 0);

            iassert(size == size2);

            Scr_CheckAnimsDefined(namesa, filenameId);
            RemoveVariable(fileId, 0);
            RemoveRefToObject(scrAnimPub.animtree_node);
            scrAnimPub.animtree_node = 0;
            tempValue.type = VAR_CODEPOS;
            tempValue.u.intValue = (int)animtree;
            Variable = GetVariable(fileId, 1);
            SetVariableValue(Variable, &tempValue);
            XAnimSetupSyncNodes(animtree);

            scrAnimPub.xanim_lookup[user][index].anims = animtree;
        }
        else
        {
            scrAnimPub.xanim_lookup[user][index].anims = 0;
        }
    }
}

int __cdecl Scr_GetAnimTreeSize(unsigned int parentNode)
{
    VariableValueInternal_u Object; // eax
    unsigned int node; // [esp+0h] [ebp-Ch]
    int size; // [esp+8h] [ebp-4h]

    size = 0;
    for (node = FindFirstSibling(parentNode); node; node = FindNextSibling(node))
    {
        if (GetVariableName(node) < SL_MAX_STRING_INDEX)
        {
            if (GetValueType(node) == 1)
            {
                Object = FindObject(node);
                size += Scr_GetAnimTreeSize(Object.u.stringValue);
            }
            else
            {
                ++size;
            }
        }
    }
    if (size)
        ++size;
    return size;
}

void __cdecl ConnectScriptToAnim(
    unsigned int names,
    unsigned __int16 index,
    unsigned int filename,
    unsigned int name,
    unsigned __int16 treeIndex)
{
    scr_anim_s anim; // [esp+4h] [ebp-14h]
    const char *codePos; // [esp+8h] [ebp-10h]
    unsigned int animId; // [esp+Ch] [ebp-Ch]
    VariableValueInternal_u *value; // [esp+10h] [ebp-8h]
    const char *nextCodePos; // [esp+14h] [ebp-4h]

    animId = FindVariable(names, name);
    if (animId)
    {
        value = GetVariableValueAddress(animId);
        if (!value->u.codePosValue)
        {
            Com_Error(ERR_DROP, "duplicate animation %s in animtrees/%s.atr", SL_ConvertToString(name), SL_ConvertToString(filename));
        }
        anim.index = index;
        anim.tree = treeIndex;
        for (codePos = (char *)value->u.codePosValue; codePos; codePos = nextCodePos)
        {
            nextCodePos = *(const char **)codePos;
            *(const char**)codePos = anim.linkPointer;
        }
        value->u.codePosValue = NULL;
    }
}

int __cdecl Scr_CreateAnimationTree(
    unsigned int parentNode,
    unsigned int names,
    XAnim_s *anims,
    unsigned int childIndex,
    const char *parentName,
    unsigned int parentIndex,
    unsigned int filename,
    int treeIndex,
    unsigned __int16 flags)
{
    int varFlags; // [esp+0h] [ebp-20h]
    unsigned int node; // [esp+4h] [ebp-1Ch]
    unsigned int nodeRef; // [esp+8h] [ebp-18h]
    unsigned int nodeRefa; // [esp+8h] [ebp-18h]
    unsigned int flagsId; // [esp+Ch] [ebp-14h]
    unsigned int name; // [esp+10h] [ebp-10h]
    unsigned int size; // [esp+14h] [ebp-Ch]
    unsigned int processAdditive; // [esp+18h] [ebp-8h]
    int childIndexa; // [esp+34h] [ebp+14h]
    unsigned int parentIndexa; // [esp+3Ch] [ebp+1Ch]

    size = 0;
    for (nodeRef = FindFirstSibling(parentNode); nodeRef; nodeRef = FindNextSibling(nodeRef))
    {
        if (GetVariableName(nodeRef) < SL_MAX_STRING_INDEX)
            ++size;
    }

    iassert(parentIndex == (unsigned short)parentIndex);
    iassert(childIndex == (unsigned short)childIndex);
    iassert(size == (unsigned short)size);

    scrVarPub.checksum *= 31;
    scrVarPub.checksum += parentIndex;

    scrVarPub.checksum *= 31;
    scrVarPub.checksum += childIndex;

    scrVarPub.checksum *= 31;
    scrVarPub.checksum += size;

    scrVarPub.checksum *= 31;
    scrVarPub.checksum += flags;

    XAnimBlend(anims, parentIndex, parentName, childIndex, size, flags);

    parentIndexa = childIndex;
    childIndexa = size + childIndex;
    processAdditive = 0;
LABEL_13:
    if (processAdditive <= 1)
    {
        for (nodeRefa = FindFirstSibling(parentNode); ; nodeRefa = FindNextSibling(nodeRefa))
        {
            if (!nodeRefa)
            {
                ++processAdditive;
                goto LABEL_13;
            }
            name = GetVariableName(nodeRefa);
            if (name < SL_MAX_STRING_INDEX)
            {
                if (GetValueType(nodeRefa) != 1)
                {
                    if (processAdditive)
                        continue;

                    iassert(parentIndex == (unsigned short)parentIndex);
                    scrVarPub.checksum *= 31;
                    scrVarPub.checksum += parentIndexa;
                    XAnimCreate(anims, parentIndexa, SL_ConvertToString(name));
                    goto LABEL_28;
                }
                node = FindObject(nodeRefa);
                flagsId = FindArrayVariable(node, 0);
                if (flagsId)
                    varFlags = GetVariableValueAddress(flagsId)->u.intValue;
                else
                    LOWORD(varFlags) = 0;
                if (((varFlags & 0x10) != 0) == processAdditive) 
                {
                    childIndexa = Scr_CreateAnimationTree(
                        node,
                        names,
                        anims,
                        childIndexa,
                        SL_ConvertToString(name),
                        parentIndexa,
                        filename,
                        treeIndex,
                        varFlags);
                LABEL_28:
                    ConnectScriptToAnim(names, parentIndexa++, filename, name, treeIndex);
                    continue;
                }
            }
        }
    }
    return childIndexa;
}

void __cdecl Scr_CheckAnimsDefined(unsigned int names, unsigned int filename)
{
    unsigned int name; // [esp+0h] [ebp-10h]
    char *msg; // [esp+4h] [ebp-Ch]
    unsigned int animId; // [esp+8h] [ebp-8h]
    VariableValueInternal_u *value; // [esp+Ch] [ebp-4h]

    for (animId = FindFirstSibling(names); animId; animId = FindNextSibling(animId))
    {
        name = GetVariableName(animId);

        iassert(name < SL_MAX_STRING_INDEX);

        value = GetVariableValueAddress(animId);
        if (value->u.intValue)
        {
            msg = va("animation '%s' not defined in anim tree '%s'", SL_ConvertToString(name), SL_ConvertToString(filename));
            if (Scr_IsInOpcodeMemory(value->u.codePosValue))
                CompileError2((char *)value->u.intValue, "%s", msg);
            else
                Com_Error(ERR_DROP, "%s", msg);
        }
    }
}

bool __cdecl Scr_LoadAnimTreeInternal(const char *filename, unsigned int parentNode, unsigned int names)
{
    char extFilename[64]; // [esp+10h] [ebp-50h] BYREF
    char *sourceBuffer; // [esp+54h] [ebp-Ch]
    const char *oldFilename; // [esp+58h] [ebp-8h]
    const char *oldSourceBuf; // [esp+5Ch] [ebp-4h]

    iassert(strlen(filename) + sizeof("animtrees/.atr") < MAX_QPATH);

    sprintf(extFilename, "animtrees/%s.atr", filename);
    oldSourceBuf = scrParserPub.sourceBuf;
    sourceBuffer = Scr_AddSourceBuffer(0, extFilename, 0, 1);

    if (!sourceBuffer)
        return 0;

    oldFilename = scrParserPub.scriptfilename;
    scrParserPub.scriptfilename = extFilename;
    Scr_AnimTreeParse(sourceBuffer, parentNode, names);
    scrParserPub.scriptfilename = oldFilename;
    scrParserPub.sourceBuf = oldSourceBuf;
    Hunk_ClearTempMemoryHigh();
    return GetArraySize(parentNode) != 0;
}

void __cdecl Scr_AnimTreeParse(const char *pos, unsigned int parentNode, unsigned int names)
{
    Com_BeginParseSession("Scr_AnimTreeParse");
    scrAnimGlob.pos = pos;
    scrAnimGlob.start = pos;
    if (!AnimTreeParseInternal(parentNode, names, 1, 0, 0))
        AnimTreeCompileError("bad token");
    Com_EndParseSession();
}

void __cdecl AnimTreeCompileError(const char *msg)
{
    const char *pos; // [esp+0h] [ebp-4h]

    pos = Com_GetLastTokenPos();
    Com_EndParseSession();
    CompileError(pos - scrAnimGlob.start, "%s", msg);
}

bool __cdecl AnimTreeParseInternal(
    unsigned int parentNode,
    unsigned int names,
    bool bIncludeParent,
    bool bLoop,
    bool bComplete)
{
    bool v5; // al
    unsigned int ArrayVariable; // eax
    unsigned int prev; // eax
    unsigned int currentAnim; // [esp+Ch] [ebp-24h]
    bool bResult; // [esp+13h] [ebp-1Dh]
    unsigned int animName; // [esp+14h] [ebp-1Ch]
    unsigned int animNamea; // [esp+14h] [ebp-1Ch]
    parseInfo_t *token; // [esp+18h] [ebp-18h]
    parseInfo_t *tokena; // [esp+18h] [ebp-18h]
    unsigned int currentAnimArray; // [esp+1Ch] [ebp-14h]
    int flags; // [esp+20h] [ebp-10h]
    bool bIgnore; // [esp+27h] [ebp-9h]
    VariableValue tempValue; // [esp+28h] [ebp-8h] BYREF

    tempValue.type = VAR_INTEGER;
    animName = 0;
    currentAnim = 0;
    flags = 0;
    bIgnore = 0;
    while (1)
    {
        while (1)
        {
            while (1)
            {
                token = Com_Parse(&scrAnimGlob.pos);
                if (!scrAnimGlob.pos)
                {
                    bResult = 1;
                    goto end;
                }

                if (!Scr_IsIdentifier(token->token))
                    break;

                if (bIgnore)
                    RemoveVariable(parentNode, animName);

                animName = SL_GetLowercaseString_(token->token, 2u, 4);
                if (FindVariable(parentNode, animName))
                    AnimTreeCompileError("duplicate animation");

                currentAnim = GetVariable(parentNode, animName);
                bIgnore = (!bComplete && !FindVariable(names, animName) && !scrAnimGlob.bAnimCheck);
                flags = 0;
                tokena = Com_ParseOnLine(&scrAnimGlob.pos);

                if (tokena->token[0])
                {
                    if (Scr_IsIdentifier(tokena->token))
                        AnimTreeCompileError("FIXME: aliases not yet implemented");

                    if (tokena->token[0] != ':' || tokena->token[1])
                        AnimTreeCompileError("bad token");

                    flags = GetAnimTreeParseProperties();
                    token = Com_Parse(&scrAnimGlob.pos);

                    if (token->token[0] != '{' || token->token[1])
                        AnimTreeCompileError("properties cannot be applied to primitive animations");

                    break;
                }
            }
            if (token->token[0] != '{')
                break;

            if (token->token[1])
                AnimTreeCompileError("bad token");

            if (Com_ParseOnLine(&scrAnimGlob.pos)->token[0])
                AnimTreeCompileError("token not allowed after '{'");

            if (!currentAnim)
                AnimTreeCompileError("no animation specified for this block");

            currentAnimArray = GetArray(currentAnim);
            if (bComplete || (flags & ANIMFLAG_COMPLETE) != 0 && !bIgnore)
                v5 = AnimTreeParseInternal(currentAnimArray, names, !bIgnore, flags & ANIMFLAG_LOOPSYNC, true);
            else
                v5 = AnimTreeParseInternal(currentAnimArray, names, !bIgnore, flags & ANIMFLAG_LOOPSYNC, false);

            if (v5)
                AnimTreeCompileError("unexpected end of file");

            if (GetArraySize(currentAnimArray))
            {
                tempValue.u.intValue = flags;
                ArrayVariable = GetArrayVariable(currentAnimArray, 0);
                SetVariableValue(ArrayVariable, &tempValue);
            }
            else
            {
                if (!bIgnore)
                    MyAssertHandler(".\\script\\scr_animtree.cpp", 265, 0, "%s", "bIgnore");
                RemoveVariable(parentNode, animName);
            }

            currentAnim = 0;
            bIgnore = 0;
        }

        if (token->token[0] == '}')
            break;

        AnimTreeCompileError("bad token");
    }
    if (token->token[1])
        AnimTreeCompileError("bad token");

    if (Com_ParseOnLine(&scrAnimGlob.pos)->token[0])
        AnimTreeCompileError("token not allowed after '}'");

    bResult = 0;
end:
    if (bIgnore)
        RemoveVariable(parentNode, animName);

    if (bIncludeParent && !GetArraySize(parentNode))
    {
        if (bLoop)
            prev = SL_GetString_("void_loop", 0, 4);
        else
            prev = SL_GetString_("void", 0, 4);
        animNamea = prev;
        GetVariable(parentNode, prev);
        SL_RemoveRefToString(animNamea);
    }
    return bResult;
}


int __cdecl GetAnimTreeParseProperties()
{
    parseInfo_t *token; // [esp+4h] [ebp-Ch]
    unsigned int i; // [esp+8h] [ebp-8h]
    int flags; // [esp+Ch] [ebp-4h]

    flags = 0;
    while (1)
    {
        token = Com_ParseOnLine(&scrAnimGlob.pos);
        if (!token->token[0])
            break;

        static const char *propertyNames[4] = { "loopsync", "nonloopsync", "complete", "additive" }; // idb
        for (i = 0; i < 4 && I_stricmp(token->token, propertyNames[i]); ++i);

        switch (i)
        {
        case 0u:
            flags |= ANIMFLAG_LOOPSYNC;
            break;
        case 1u:
            flags |= ANIMFLAG_NONLOOPSYNC;
            break;
        case 2u:
            flags |= ANIMFLAG_COMPLETE;
            break;
        case 3u:
            flags |= ANIMFLAG_ADDITIVE;
            break;
        default:
            AnimTreeCompileError("unknown anim property");
            break;
        }
    }
    return flags;
}

scr_animtree_t __cdecl Scr_FindAnimTree(const char *filename)
{
    VariableValue tempValue; // [esp+0h] [ebp-20h]
    unsigned int xanimId; // [esp+Ch] [ebp-14h]
    HashEntry_unnamed_type_u filenameId; // [esp+10h] [ebp-10h]
    unsigned int fileId; // [esp+14h] [ebp-Ch]
    unsigned int fileIda; // [esp+14h] [ebp-Ch]
    scr_animtree_t tree;

    iassert(scrAnimPub.animtree_loading);

    filenameId.prev = Scr_CreateCanonicalFilename(filename).prev;
    fileId = FindVariable(scrAnimPub.animtrees, filenameId.prev);

    SL_RemoveRefToString(filenameId.prev);

    if (!fileId)
    {
        tree.anims = NULL;
        return tree;
    }

    iassert(GetVariableName(fileId) < SL_MAX_STRING_INDEX);

    GetVariableName(fileId);
    fileIda = FindObject(fileId);
    iassert(fileIda);
    xanimId = FindVariable(fileIda, ANIMTREE_XANIM);

    if (!xanimId)
    {
        tree.anims = NULL;
        return tree;
    }

    tempValue = Scr_EvalVariable(xanimId);
    iassert(tempValue.type == VAR_CODEPOS);
    tree.anims = (XAnim_s *)tempValue.u.codePosValue;
    return tree;
}

void __cdecl Scr_FindAnim(const char *filename, const char *animName, scr_anim_s *anim, int user)
{
    unsigned int name; // [esp+0h] [ebp-8h]
    unsigned int index; // [esp+4h] [ebp-4h] BYREF

    name = SL_GetLowercaseString_(animName, 0, 4);
    Scr_EmitAnimationInternal((char *)anim, name, Scr_UsingTreeInternal(filename, &index, user));
    SL_RemoveRefToString(name);
}

