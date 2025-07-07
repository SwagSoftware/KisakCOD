#pragma once
#include <xanim/dobj.h>

#ifndef KISAK_SP
#error This file is for SinglePlayer only
#endif

char *__cdecl CL_AllocSkelMemory(unsigned int size);
int __cdecl CL_GetSkelTimeStamp();
int __cdecl CL_DObjCreateSkelForBones(const DObj_s *obj, int *partBits, DObjAnimMat **pMatOut);
