#ifndef KISAK_SP
#error This file is for SinglePlayer only
#endif


char *__cdecl CL_AllocSkelMemory(unsigned int size)
{
    unsigned int v2; // r28
    char v7; // cr34
    int v8; // r30
    char *v9; // r31

    if (!size)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\client\\cl_pose.cpp", 30, 0, "%s", "size");
    v2 = (size + 15) & 0xFFFFFFF0;
    if (v2 > 0x7FFF0)
        MyAssertHandler(
            "c:\\trees\\cod3\\cod3src\\src\\client\\cl_pose.cpp",
            33,
            0,
            "%s",
            "size <= CL_SKEL_MEMORY_SIZE - SKEL_MEM_ALIGNMENT");
    if (!clients[R_GetLocalClientNum()].skelMemoryStart)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\client\\cl_pose.cpp", 35, 0, "%s", "skel_glob->skelMemoryStart");
    _R11 = &clients[R_GetLocalClientNum()].skelMemPos;
    do
    {
        __asm
        {
            mfmsr     r7
            mtmsree   r13
            lwarx     r9, 0, r11
        }
        _R8 = v2 + _R9;
        __asm
        {
            stwcx.r8, 0, r11
            mtmsree   r7
        }
    } while (!v7);
    v8 = _R9;
    v9 = &clients[R_GetLocalClientNum()].skelMemoryStart[_R9];
    if (v8 + v2 > 0x7FFF0)
        return 0;
    if (!v9)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\client\\cl_pose.cpp", 46, 0, "%s", "result");
    return v9;
}

int __cdecl CL_GetSkelTimeStamp()
{
    return clients[R_GetLocalClientNum()].skelTimeStamp;
}

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
            DObjCreateSkel(obj, (char *)v9, skelTimeStamp);
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

