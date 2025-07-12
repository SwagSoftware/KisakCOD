#include "cg_local.h"
#include "cg_public.h"

#include <bgame/bg_local.h>

void __cdecl CG_UsedDObjCalcPose(cpose_t *pose)
{
    iassert(pose);
    InterlockedCompareExchange((volatile unsigned int*)&pose->cullIn, 1, 0);
}

void __cdecl CG_CullIn(cpose_t *pose)
{
    iassert(pose);
    pose->cullIn = 2;
}

