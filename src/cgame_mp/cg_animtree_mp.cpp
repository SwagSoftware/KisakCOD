#ifndef KISAK_MP
#error This File is MultiPlayer Only
#endif

#include "cg_local_mp.h"
#include "cg_public_mp.h"
#include <aim_assist/aim_assist.h>
#include <script/scr_main.h>
#include <script/scr_memorytree.h>

void __cdecl CGScr_LoadAnimTrees()
{
    Scr_BeginLoadAnimTrees(0);
}

void __cdecl CG_FreeClientDObjInfo(int32_t localClientNum)
{
    int32_t i; // [esp+0h] [ebp-4h]

    for (i = 0; i < 64; ++i)
        CG_SafeDObjFree(localClientNum, i);
}

void __cdecl CG_SetDObjInfo(int32_t localClientNum, int32_t iEntNum, int32_t iEntType, XModel *pXModel)
{
    cg_s *cgameGlob;

    cgameGlob = CG_GetLocalClientGlobals(localClientNum);

    cgameGlob->iEntityLastType[iEntNum] = iEntType;
    cgameGlob->pEntityLastXModel[iEntNum] = pXModel;
}

bool __cdecl CG_CheckDObjInfoMatches(int32_t localClientNum, int32_t iEntNum, int32_t iEntType, XModel *pXModel)
{
    const cg_s *cgameGlob;

    cgameGlob = CG_GetLocalClientGlobals(localClientNum);

    return cgameGlob->iEntityLastType[iEntNum] == iEntType && cgameGlob->pEntityLastXModel[iEntNum] == pXModel;
}

void __cdecl CG_SafeDObjFree(int32_t localClientNum, int32_t entIndex)
{
    centity_s *cent; // [esp+0h] [ebp-4h]

    Com_SafeClientDObjFree(entIndex, localClientNum);
    CG_SetDObjInfo(localClientNum, entIndex, 0, 0);
    cent = CG_GetEntity(localClientNum, entIndex);
    if (cent->tree)
    {
        XAnimFreeTree(cent->tree, (void(__cdecl *)(void *, int))MT_Free);
        cent->tree = 0;
    }
}

void __cdecl CG_FreeEntityDObjInfo(int32_t localClientNum)
{
    int32_t i; // [esp+0h] [ebp-4h]

    for (i = 64; i < 1024; ++i)
        CG_SafeDObjFree(localClientNum, i);
}

