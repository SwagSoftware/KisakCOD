#include "scr_animtree.h"

#include <qcommon/mem_track.h>
#include <universal/q_parse.h>

#include "scr_main.h"
#include "scr_parser.h"
#include "scr_variable.h"

scrAnimPub_t scrAnimPub;
scrAnimGlob_t scrAnimGlob;

//    int marker_scr_animtree  83020a00     scr_animtree.obj
//    struct scrAnimPub_t scrAnimPub 83020c18     scr_animtree.obj

void __cdecl TRACK_scr_animtree()
{
    track_static_alloc_internal(&scrAnimGlob, 524, "scrAnimGlob", 7);
    track_static_alloc_internal(&scrAnimPub, 1052, "scrAnimPub", 7);
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

    if (!names)
        MyAssertHandler(".\\script\\scr_animtree.cpp", 149, 0, "%s", "names");
    animId = FindVariable(names, animName);
    if (animId)
    {
        value = GetVariableValueAddress(animId);
        *(VariableValueInternal_u *)pos = *value;
        value->u.intValue = (int)pos;
    }
    else
    {
        NewVariable = GetNewVariable(names, animName);
        *(unsigned int *)pos = 0;
        tempValue.type = 7;
        tempValue.u.intValue = (int)pos;
        SetVariableValue(NewVariable, &tempValue);
    }
}

int __cdecl Scr_GetAnimsIndex(const XAnim_s *anims)
{
    int i; // [esp+4h] [ebp-4h]

    for (i = scrAnimPub.xanim_num[1]; i && (const XAnim_s *)scrAnimPub.xanim_num[i - 128] != anims; --i)
        ;
    return i;
}

XAnim_s *__cdecl Scr_GetAnims(unsigned int index)
{
    if (!index || index > scrAnimPub.xanim_num[1])
        MyAssertHandler(
            ".\\script\\scr_animtree.cpp",
            408,
            0,
            "index not in [1, scrAnimPub.xanim_num[SCR_XANIM_SERVER]]\n\t%i not in [%i, %i]",
            index,
            1,
            scrAnimPub.xanim_num[1]);
    return (XAnim_s *)scrAnimPub.xanim_num[index - 128];
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

    if (!scrAnimPub.animtree_loading)
        MyAssertHandler(".\\script\\scr_animtree.cpp", 550, 0, "%s", "scrAnimPub.animtree_loading");
    if (!Scr_IsIdentifier(filename))
        MyAssertHandler(".\\script\\scr_animtree.cpp", 551, 0, "%s", "Scr_IsIdentifier( filename )");
    name = Scr_CreateCanonicalFilename(filename).prev;
    id = FindVariable(scrAnimPub.animtrees, name);
    if (id)
    {
        fileId = FindObject(id).u.stringValue;
        *index = 0;
        for (i = 1; i <= scrAnimPub.xanim_num[user]; ++i)
        {
            if (scrAnimGlob.using_xanim_lookup[user][i] == id)
            {
                *index = i;
                break;
            }
        }
        if (!*index)
            MyAssertHandler(".\\script\\scr_animtree.cpp", 582, 0, "%s", "*index");
    }
    else
    {
        ida = GetNewVariable(scrAnimPub.animtrees, name);
        fileId = GetObject(ida).u.stringValue;
        if (++scrAnimPub.xanim_num[user] >= 0x80)
            MyAssertHandler(".\\script\\scr_animtree.cpp", 562, 0, "%s", "scrAnimPub.xanim_num[user] < MAX_XANIMTREE_NUM");
        scrAnimGlob.using_xanim_lookup[user][scrAnimPub.xanim_num[user]] = ida;
        *index = scrAnimPub.xanim_num[user];
    }
    if (FindVariable(fileId, 1u))
        MyAssertHandler(".\\script\\scr_animtree.cpp", 585, 0, "%s", "!FindVariable( fileId, ANIMTREE_XANIM )");
    Variable = GetVariable(fileId, 0);
    names = GetArray(Variable).u.stringValue;
    SL_RemoveRefToString(name);
    return names;
}

void __cdecl Scr_PrecacheAnimationTree(unsigned int parentNode)
{
    VariableValueInternal_u Object; // eax
    char *v2; // eax
    unsigned int node; // [esp+0h] [ebp-8h]
    unsigned int name; // [esp+4h] [ebp-4h]

    for (node = FindFirstSibling(parentNode); node; node = FindNextSibling(node))
    {
        name = GetVariableName(node);
        if (name < 0x10000)
        {
            if (GetValueType(node) == 1)
            {
                Object = FindObject(node);
                Scr_PrecacheAnimationTree(Object.u.stringValue);
            }
            else
            {
                v2 = SL_ConvertToString(name);
                XAnimPrecache(v2, (void *(__cdecl *)(int))Hunk_AllocXAnimPrecache);
            }
        }
    }
}

void __cdecl Scr_LoadAnimTreeAtIndex(unsigned int index, void *(__cdecl *Alloc)(int), int user)
{
    char *v3; // eax
    char *v4; // eax
    const char *v5; // eax
    char *v6; // eax
    unsigned int Variable; // eax
    unsigned int animtree_node; // [esp-8h] [ebp-30h]
    XAnim_s *animtree; // [esp+0h] [ebp-28h]
    unsigned int size; // [esp+8h] [ebp-20h]
    HashEntry_unnamed_type_u name; // [esp+Ch] [ebp-1Ch]
    unsigned int filenameId; // [esp+10h] [ebp-18h]
    unsigned int names; // [esp+14h] [ebp-14h]
    unsigned int namesa; // [esp+14h] [ebp-14h]
    unsigned int fileId; // [esp+18h] [ebp-10h]
    VariableValue tempValue; // [esp+1Ch] [ebp-Ch] BYREF
    unsigned int id; // [esp+24h] [ebp-4h]

    id = scrAnimGlob.using_xanim_lookup[user][index];
    if (!scrAnimPub.animtree_loading)
        MyAssertHandler(".\\script\\scr_animtree.cpp", 652, 0, "%s", "scrAnimPub.animtree_loading");
    Hunk_CheckTempMemoryClear();
    Hunk_CheckTempMemoryHighClear();
    if (GetVariableName(id) >= 0x10000)
        MyAssertHandler(".\\script\\scr_animtree.cpp", 656, 0, "%s", "GetVariableName( id ) < SL_MAX_STRING_INDEX");
    filenameId = (unsigned __int16)GetVariableName(id);
    fileId = FindObject(id).u.stringValue;
    if (!fileId)
        MyAssertHandler(".\\script\\scr_animtree.cpp", 661, 0, "%s", "fileId");
    if (!FindVariable(fileId, 1u))
    {
        names = FindVariable(fileId, 0);
        if (names)
        {
            namesa = FindObject(names).u.stringValue;
            if (!namesa)
                MyAssertHandler(".\\script\\scr_animtree.cpp", 676, 0, "%s", "names");
            if (scrAnimPub.animtree_node)
                MyAssertHandler(".\\script\\scr_animtree.cpp", 678, 0, "%s", "!scrAnimPub.animtree_node");
            scrAnimPub.animtree_node = Scr_AllocArray();
            animtree_node = scrAnimPub.animtree_node;
            v3 = SL_ConvertToString(filenameId);
            if (!Scr_LoadAnimTreeInternal(v3, animtree_node, namesa))
            {
                v4 = SL_ConvertToString(filenameId);
                v5 = va("unknown anim tree '%s'", v4);
                Com_Error(ERR_DROP, v5);
            }
            Hunk_CheckTempMemoryClear();
            Hunk_CheckTempMemoryHighClear();
            size = Scr_GetAnimTreeSize(scrAnimPub.animtree_node);
            if (!size)
                MyAssertHandler(".\\script\\scr_animtree.cpp", 688, 0, "%s", "size");
            v6 = SL_ConvertToString(filenameId);
            animtree = XAnimCreateAnims(v6, size, Alloc);
            name.prev = SL_GetString_("root", 0, 4);
            ConnectScriptToAnim(namesa, 0, filenameId, name.prev, index);
            SL_RemoveRefToString(name.prev);
            if (!useFastFile->current.enabled)
                Scr_PrecacheAnimationTree(scrAnimPub.animtree_node);
            if (size != Scr_CreateAnimationTree(
                scrAnimPub.animtree_node,
                namesa,
                animtree,
                1u,
                "root",
                0,
                filenameId,
                index,
                0))
                MyAssertHandler(".\\script\\scr_animtree.cpp", 702, 0, "%s", "size == size2");
            Scr_CheckAnimsDefined(namesa, filenameId);
            RemoveVariable(fileId, 0);
            RemoveRefToObject(scrAnimPub.animtree_node);
            scrAnimPub.animtree_node = 0;
            tempValue.type = 7;
            tempValue.u.intValue = (int)animtree;
            Variable = GetVariable(fileId, 1u);
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
        if (GetVariableName(node) < 0x10000)
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
    char *v5; // eax
    char *v6; // [esp-4h] [ebp-1Ch]
    scr_anim_s anim; // [esp+4h] [ebp-14h]
    char *codePos; // [esp+8h] [ebp-10h]
    unsigned int animId; // [esp+Ch] [ebp-Ch]
    VariableValueInternal_u *value; // [esp+10h] [ebp-8h]
    const char *nextCodePos; // [esp+14h] [ebp-4h]

    animId = FindVariable(names, name);
    if (animId)
    {
        value = GetVariableValueAddress(animId);
        if (!value->u.intValue)
        {
            v6 = SL_ConvertToString(filename);
            v5 = SL_ConvertToString(name);
            Com_Error(ERR_DROP, "duplicate animation %s in animtrees/%s.atr", v5, v6);
        }
        anim.index = index;
        anim.tree = treeIndex;
        for (codePos = (char *)value->u.intValue; codePos; codePos = (char *)nextCodePos)
        {
            nextCodePos = *(const char **)codePos;
            *(scr_anim_s *)codePos = anim;
        }
        value->u.intValue = 0;
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
    char *v9; // eax
    char *v10; // eax
    VariableUnion v12; // [esp+0h] [ebp-20h]
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
        if (GetVariableName(nodeRef) < 0x10000)
            ++size;
    }
    if (parentIndex != (unsigned __int16)parentIndex)
        MyAssertHandler(".\\script\\scr_animtree.cpp", 435, 0, "%s", "parentIndex == (unsigned short)parentIndex");
    if (childIndex != (unsigned __int16)childIndex)
        MyAssertHandler(".\\script\\scr_animtree.cpp", 436, 0, "%s", "childIndex == (unsigned short)childIndex");
    if (size != (unsigned __int16)size)
        MyAssertHandler(".\\script\\scr_animtree.cpp", 437, 0, "%s", "size == (unsigned short)size");
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
            if (name < 0x10000)
            {
                if (GetValueType(nodeRefa) != 1)
                {
                    if (processAdditive)
                        continue;
                    if (parentIndexa != (unsigned __int16)parentIndexa)
                        MyAssertHandler(".\\script\\scr_animtree.cpp", 476, 0, "%s", "parentIndex == (unsigned short)parentIndex");
                    scrVarPub.checksum *= 31;
                    scrVarPub.checksum += parentIndexa;
                    v10 = SL_ConvertToString(name);
                    XAnimCreate(anims, parentIndexa, v10);
                    goto LABEL_28;
                }
                node = FindObject(nodeRefa).u.stringValue;
                flagsId = FindArrayVariable(node, 0);
                if (flagsId)
                    v12.intValue = (int)GetVariableValueAddress(flagsId)->u;
                else
                    LOWORD(v12.floatValue) = 0;
                if (((LOBYTE(v12.floatValue) & 0x10) != 0) == processAdditive)
                {
                    v9 = SL_ConvertToString(name);
                    childIndexa = Scr_CreateAnimationTree(
                        node,
                        names,
                        anims,
                        childIndexa,
                        v9,
                        parentIndexa,
                        filename,
                        treeIndex,
                        LOWORD(v12.floatValue));
                LABEL_28:
                    ConnectScriptToAnim(names, parentIndexa++, filename, (unsigned __int16)name, treeIndex);
                    continue;
                }
            }
        }
    }
    return childIndexa;
}

void __cdecl Scr_CheckAnimsDefined(unsigned int names, unsigned int filename)
{
    char *v2; // eax
    char *v3; // [esp-4h] [ebp-14h]
    unsigned int name; // [esp+0h] [ebp-10h]
    char *msg; // [esp+4h] [ebp-Ch]
    unsigned int animId; // [esp+8h] [ebp-8h]
    VariableValueInternal_u *value; // [esp+Ch] [ebp-4h]

    for (animId = FindFirstSibling(names); animId; animId = FindNextSibling(animId))
    {
        name = GetVariableName(animId);
        if (name >= 0x10000)
            MyAssertHandler(".\\script\\scr_animtree.cpp", 502, 0, "%s", "name < SL_MAX_STRING_INDEX");
        value = GetVariableValueAddress(animId);
        if (value->u.intValue)
        {
            v3 = SL_ConvertToString(filename);
            v2 = SL_ConvertToString(name);
            msg = va("animation '%s' not defined in anim tree '%s'", v2, v3);
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

    if (strlen(filename) + 15 >= 0x40)
        MyAssertHandler(
            ".\\script\\scr_animtree.cpp",
            613,
            0,
            "%s",
            "strlen( filename ) + sizeof( \"animtrees/.atr\" ) < MAX_QPATH");
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
    bool v9; // [esp+8h] [ebp-28h]
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

    tempValue.type = 6;
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
                v9 = !bComplete && !FindVariable(names, animName) && !scrAnimGlob.bAnimCheck;
                bIgnore = v9;
                flags = 0;
                tokena = Com_ParseOnLine(&scrAnimGlob.pos);
                if (tokena->token[0])
                {
                    if (Scr_IsIdentifier(tokena->token))
                        AnimTreeCompileError("FIXME: aliases not yet implemented");
                    if (tokena->token[0] != 58 || tokena->token[1])
                        AnimTreeCompileError("bad token");
                    flags = GetAnimTreeParseProperties();
                    token = Com_Parse(&scrAnimGlob.pos);
                    if (token->token[0] != 123 || token->token[1])
                        AnimTreeCompileError("properties cannot be applied to primitive animations");
                    break;
                }
            }
            if (token->token[0] != 123)
                break;
            if (token->token[1])
                AnimTreeCompileError("bad token");
            if (Com_ParseOnLine(&scrAnimGlob.pos)->token[0])
                AnimTreeCompileError("token not allowed after '{'");
            if (!currentAnim)
                AnimTreeCompileError("no animation specified for this block");
            currentAnimArray = GetArray(currentAnim).u.stringValue;
            if (bComplete || (flags & 8) != 0 && !bIgnore)
                v5 = AnimTreeParseInternal(currentAnimArray, names, !bIgnore, (flags & 1) != 0, 1);
            else
                v5 = AnimTreeParseInternal(currentAnimArray, names, !bIgnore, (flags & 1) != 0, 0);
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
        if (token->token[0] == 125)
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

const char *propertyNames[4] = { "loopsync", "nonloopsync", "complete", "additive" }; // idb

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
        for (i = 0; i < 4 && I_stricmp(token->token, propertyNames[i]); ++i)
            ;
        switch (i)
        {
        case 0u:
            flags |= 1u;
            break;
        case 1u:
            flags |= 2u;
            break;
        case 2u:
            flags |= 8u;
            break;
        case 3u:
            flags |= 0x10u;
            break;
        default:
            AnimTreeCompileError("unknown anim property");
            break;
        }
    }
    return flags;
}

scr_animtree_t *__cdecl Scr_FindAnimTree(const char *filename)
{
    VariableValue v2; // [esp+0h] [ebp-20h]
    unsigned int xanimId; // [esp+Ch] [ebp-14h]
    HashEntry_unnamed_type_u filenameId; // [esp+10h] [ebp-10h]
    unsigned int fileId; // [esp+14h] [ebp-Ch]
    unsigned int fileIda; // [esp+14h] [ebp-Ch]

    if (!scrAnimPub.animtree_loading)
        MyAssertHandler(".\\script\\scr_animtree.cpp", 728, 0, "%s", "scrAnimPub.animtree_loading");
    filenameId.prev = Scr_CreateCanonicalFilename(filename).prev;
    fileId = FindVariable(scrAnimPub.animtrees, filenameId.prev);
    SL_RemoveRefToString(filenameId.prev);
    if (!fileId)
        return 0;
    if (GetVariableName(fileId) >= 0x10000)
        MyAssertHandler(".\\script\\scr_animtree.cpp", 739, 0, "%s", "GetVariableName( fileId ) < SL_MAX_STRING_INDEX");
    GetVariableName(fileId);
    fileIda = FindObject(fileId).u.stringValue;
    if (!fileIda)
        MyAssertHandler(".\\script\\scr_animtree.cpp", 744, 0, "%s", "fileId");
    xanimId = FindVariable(fileIda, 1u);
    if (!xanimId)
        return 0;
    v2 = Scr_EvalVariable(xanimId);
    if (v2.type != 7)
        MyAssertHandler(".\\script\\scr_animtree.cpp", 751, 0, "%s", "tempValue.type == VAR_CODEPOS");
    return (scr_animtree_t*)v2.u.intValue;
}

void __cdecl Scr_FindAnim(const char *filename, const char *animName, scr_anim_s *anim, int user)
{
    unsigned int v4; // eax
    unsigned int name; // [esp+0h] [ebp-8h]
    unsigned int index; // [esp+4h] [ebp-4h] BYREF

    name = SL_GetLowercaseString_(animName, 0, 4);
    v4 = Scr_UsingTreeInternal(filename, &index, user);
    Scr_EmitAnimationInternal((char *)anim, name, v4);
    SL_RemoveRefToString(name);
}

