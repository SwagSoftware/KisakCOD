#pragma once
#include <xanim/dobj.h>

#ifndef KISAK_SP
#error This file is for SinglePlayer only
#endif

char *__fastcall CL_AllocSkelMemory(unsigned int size);
int __fastcall CL_GetSkelTimeStamp();
int __fastcall CL_DObjCreateSkelForBones(const DObj_s *obj, int *partBits, DObjAnimMat **pMatOut);
