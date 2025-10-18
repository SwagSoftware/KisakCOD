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

    int skelMemPos = InterlockedExchangeAdd(&skel_glob->skelMemPos, size);

    char *result = skel_glob->skelMemoryStart + skelMemPos;

    if (!result || (size + skelMemPos > CL_SKEL_MEMORY_SIZE))
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
    int timeStamp; // r31
    unsigned int AllocSkelSize; // r3
    DObjAnimMat *buf; // r3

    iassert(obj);

    timeStamp = CL_GetSkelTimeStamp();

    if (DObjSkelExists(obj, timeStamp))
    {
        *pMatOut = I_dmaGetDObjSkel(obj);
        return DObjSkelAreBonesUpToDate(obj, partBits);
    }
    else
    {
        AllocSkelSize = DObjGetAllocSkelSize(obj);
        buf = (DObjAnimMat *)CL_AllocSkelMemory(AllocSkelSize);
        if (buf)
        {
            *pMatOut = buf;
            DObjCreateSkel((DObj_s*)obj, (char *)buf, timeStamp);
            return 0;
        }
        else
        {
            *pMatOut = 0;
            if (warnCount_0 != timeStamp)
            {
                warnCount_0 = timeStamp;
                Com_PrintWarning(14, "WARNING: CL_SKEL_MEMORY_SIZE exceeded - not calculating skeleton\n");
            }
            return 1;
        }
    }
}

