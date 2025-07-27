#ifndef KISAK_SP 
#error This file is for SinglePlayer only 
#endif

#include "xanim_readwrite.h"
#include <script/scr_readwrite.h>

void __cdecl XAnimArchiveAnimState(XAnimState *state, MemoryFile *memFile)
{
    MemFile_ArchiveData(memFile, 32, state);
}

void __cdecl XAnimLoadAnimInfo(XAnimInfo *info, MemoryFile *memFile)
{
    _WORD v4[4]; // [sp+50h] [-20h] BYREF

    MemFile_ReadData(memFile, 2, (unsigned char*)v4);
    info->notifyIndex = v4[0];
    MemFile_ReadData(memFile, 2, (unsigned char *)v4);
    info->notifyChild = v4[0];
    MemFile_ReadData(memFile, 2, (unsigned char *)v4);
    info->notifyType = v4[0];
    info->notifyName = Scr_ReadOptionalString(memFile);
    MemFile_ArchiveData(memFile, 32, &info->state);
}

void __cdecl XAnimSaveAnimInfo(XAnimInfo *info, MemoryFile *memFile)
{
    const char *v4; // r3
    _WORD v5[4]; // [sp+50h] [-20h] BYREF

    v5[0] = info->notifyIndex;
    MemFile_WriteData(memFile, 2, v5);
    v5[0] = info->notifyChild;
    MemFile_WriteData(memFile, 2, v5);
    v5[0] = info->notifyType;
    MemFile_WriteData(memFile, 2, v5);
    if (info->notifyName)
    {
        HIBYTE(v5[0]) = 1;
        MemFile_WriteData(memFile, 1, v5);
        v4 = SL_ConvertToString(info->notifyName);
        MemFile_WriteCString(memFile, v4);
    }
    else
    {
        HIBYTE(v5[0]) = 0;
        MemFile_WriteData(memFile, 1, v5);
    }
    MemFile_ArchiveData(memFile, 32, &info->state);
}

void __cdecl XAnimLoadAnimTree(DObj_s *obj, MemoryFile *memFile)
{
    unsigned int i; // r4
    XAnimInfo *v5; // r3
    _WORD v6[24]; // [sp+50h] [-30h] BYREF

    iassert(obj);

    if (obj->tree)
    {
        if (obj->tree->children)
            MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\xanim\\xanim_readwrite.cpp", 65, 0, "%s", "!tree->children");
        MemFile_ReadData(memFile, 2, (unsigned char *)v6);
        for (i = v6[0]; v6[0] != 0xFFFF; i = v6[0])
        {
            v5 = XAnimAllocInfo(obj, i, 1);
            XAnimLoadAnimInfo(v5, memFile);
            MemFile_ReadData(memFile, 2, (unsigned char *)v6);
        }
    }
}

void __cdecl XAnimSaveAnimTree_r(const XAnimTree_s *tree, MemoryFile *memFile, int infoIndex)
{
    XAnimInfo *info; // r28
    unsigned int children; // r31
    XAnimInfo *v8; // r30
    unsigned __int16 animIndex; // [sp+50h] [-40h] BYREF

    iassert(tree);
    info = GetAnimInfo(infoIndex);
    iassert(info->inuse);
    animIndex = info->animIndex;
    MemFile_WriteData(memFile, 2, &animIndex);
    XAnimSaveAnimInfo(info, memFile);
    children = info->children;
    if (info->children)
    {
        do
        {
            v8 = GetAnimInfo(children);
            XAnimSaveAnimTree_r(tree, memFile, children);
            children = v8->next;
        } while (v8->next);
    }
}

void __cdecl XAnimSaveAnimTree(const DObj_s *obj, MemoryFile *memFile)
{
    XAnimTree_s *tree; // r3
    __int16 v5[4]; // [sp+50h] [-20h] BYREF

    iassert(obj);

    tree = obj->tree;

    if (obj->tree)
    {
        if (tree->children)
            XAnimSaveAnimTree_r(tree, memFile, tree->children);
        v5[0] = -1;
        MemFile_WriteData(memFile, 2, v5);
    }
}

