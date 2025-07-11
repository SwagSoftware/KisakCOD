#ifndef KISAK_SP
#error This file is for SinglePlayer only
#endif

#include "cl_pose.h"

#include <universal/assertive.h>
#include <gfx_d3d/r_scene.h>
#include "client.h"
#include <xanim/dobj_utils.h>


char *__cdecl CL_AllocSkelMemory(unsigned int size)
{
    iassert(size);

    // Align size to `SKEL_MEM_ALIGNMENT`
    size = (size + (SKEL_MEM_ALIGNMENT - 1)) & ~(SKEL_MEM_ALIGNMENT - 1);

    iassert(size <= CL_SKEL_MEMORY_SIZE - SKEL_MEM_ALIGNMENT);

    clientActive_t *skel_glob = &clients[R_GetLocalClientNum()];

    iassert(skel_glob->skelMemoryStart);

    // Atomic allocation using InterlockedCompareExchange()
    // (Adapted from PowerPC assembly...)
    LONG oldPos, newPos;
    do
    {
        oldPos = skel_glob->skelMemPos;
        newPos = oldPos + size;

        if ((unsigned int)newPos > CL_SKEL_MEMORY_SIZE)
        {
            // Out of Memory
            return NULL;
        }
    } while (InterlockedCompareExchange((volatile uintptr_t*)&skel_glob->skelMemPos, newPos, oldPos) != oldPos);

    char *result = skel_glob->skelMemoryStart + oldPos;

    if (!result || (unsigned int)result > CL_SKEL_MEMORY_SIZE)
    {
        iassert(0);
        return NULL;
    }

    return result;
}

int __cdecl CL_GetSkelTimeStamp()
{
    return clients[R_GetLocalClientNum()].skelTimeStamp;
}

int warnCount_0 = 0;
int __cdecl CL_DObjCreateSkelForBones(const DObj_s *obj, int *partBits, DObjAnimMat **pMatOut)
{
    int skelTimeStamp; // r31
    unsigned int AllocSkelSize; // r3
    DObjAnimMat *v9; // r3
    int v10; // r10

    if (!obj)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\client\\cl_pose.cpp", 75, 0, "%s", "obj");
    skelTimeStamp = clients[R_GetLocalClientNum()].skelTimeStamp;
    if (DObjSkelExists(obj, skelTimeStamp))
    {
        *pMatOut = I_dmaGetDObjSkel(obj);
        return DObjSkelAreBonesUpToDate(obj, partBits);
    }
    else
    {
        AllocSkelSize = DObjGetAllocSkelSize(obj);
        v9 = (DObjAnimMat *)CL_AllocSkelMemory(AllocSkelSize);
        if (v9)
        {
            *pMatOut = v9;
            DObjCreateSkel((DObj_s*)obj, (char *)v9, skelTimeStamp);
            return 0;
        }
        else
        {
            v10 = warnCount_0;
            *pMatOut = 0;
            if (v10 != skelTimeStamp)
            {
                warnCount_0 = skelTimeStamp;
                Com_PrintWarning(14, "WARNING: CL_SKEL_MEMORY_SIZE exceeded - not calculating skeleton\n");
            }
            return 1;
        }
    }
}

