#include "client_mp.h"


char *__cdecl CL_AllocSkelMemory(unsigned int size)
{
    volatile int *Addend; // [esp+0h] [ebp-Ch]
    char *result; // [esp+4h] [ebp-8h]
    int skelMemPos; // [esp+8h] [ebp-4h]
    unsigned int sizea; // [esp+14h] [ebp+8h]

    if (!size)
        MyAssertHandler(".\\client_mp\\cl_pose_mp.cpp", 30, 0, "%s", "size");
    sizea = (size + 15) & 0xFFFFFFF0;
    if (sizea > 0x3FFF0)
        MyAssertHandler(".\\client_mp\\cl_pose_mp.cpp", 33, 0, "%s", "size <= CL_SKEL_MEMORY_SIZE - SKEL_MEM_ALIGNMENT");
    if (!clients[R_GetLocalClientNum()].skelMemoryStart)
        MyAssertHandler(".\\client_mp\\cl_pose_mp.cpp", 35, 0, "%s", "skel_glob->skelMemoryStart");
    Addend = &clients[R_GetLocalClientNum()].skelMemPos;
    skelMemPos = InterlockedExchangeAdd(Addend, sizea);
    result = &clients[R_GetLocalClientNum()].skelMemoryStart[skelMemPos];
    if (sizea + skelMemPos > 0x3FFF0)
        return 0;
    if (!result)
        MyAssertHandler(".\\client_mp\\cl_pose_mp.cpp", 46, 0, "%s", "result");
    return result;
}

int __cdecl CL_GetSkelTimeStamp()
{
    return clients[R_GetLocalClientNum()].skelTimeStamp;
}

int __cdecl CL_DObjCreateSkelForBones(const DObj_s *obj, int *partBits, DObjAnimMat **pMatOut)
{
    char *buf; // [esp+0h] [ebp-Ch]
    unsigned int len; // [esp+4h] [ebp-8h]
    int timeStamp; // [esp+8h] [ebp-4h]

    if (!obj)
        MyAssertHandler(".\\client_mp\\cl_pose_mp.cpp", 75, 0, "%s", "obj");
    timeStamp = CL_GetSkelTimeStamp();
    if (DObjSkelExists(obj, timeStamp))
    {
        *pMatOut = I_dmaGetDObjSkel(obj);
        return DObjSkelAreBonesUpToDate(obj, partBits);
    }
    else
    {
        len = DObjGetAllocSkelSize(obj);
        buf = CL_AllocSkelMemory(len);
        if (buf)
        {
            *pMatOut = (DObjAnimMat *)buf;
            DObjCreateSkel(obj, buf, timeStamp);
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

