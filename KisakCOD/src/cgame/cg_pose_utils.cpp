#include "cg_local.h"
#include "cg_public.h"

void __cdecl CG_UsedDObjCalcPose(const cpose_t *pose)
{
    if (!pose)
        MyAssertHandler(".\\cgame\\cg_pose_utils.cpp", 13, 0, "%s", "pose");
    InterlockedCompareExchange(&pose->cullIn, 1, 0);
}

void __cdecl CG_CullIn(const cpose_t *pose)
{
    if (!pose)
        MyAssertHandler(".\\cgame\\cg_pose_utils.cpp", 21, 0, "%s", "pose");
    pose->cullIn = 2;
}

