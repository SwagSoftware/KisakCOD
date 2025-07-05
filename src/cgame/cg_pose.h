#pragma once

#ifndef KISAK_SP
#error This file is for SinglePlayer only
#endif

#include <xanim/dobj.h>
#include <bgame/bg_local.h>

void __fastcall QuatMultiply(float *in1, float *in2, float *out);
void __fastcall PitchToQuat(double pitch, float *quat, float *a3);
void __fastcall RollToQuat(double roll, float *quat, float *a3);
void __fastcall LocalMatrixTransformVectorQuatTrans(const float *in, const DObjAnimMat *mat, float *out);
void __fastcall NormalizeQuatTrans(DObjAnimMat *mat);
void __fastcall CG_mg42_DoControllers(const cpose_t *pose, const DObj_s *obj, int *partBits, long double a4);
void __fastcall CG_Vehicle_DoControllers(const cpose_t *pose, const DObj_s *obj, int *partBits);
void __fastcall CG_Actor_DoControllers(const cpose_t *pose, const DObj_s *obj, int *partBits);
void __fastcall CG_DoBaseOriginController(const cpose_t *pose, const DObj_s *obj, int *setPartBits);
void __fastcall CG_DoControllers(const cpose_t *pose, const DObj_s *obj, int *partBits);
DObjAnimMat *__fastcall CG_DObjCalcPose(const cpose_t *pose, const DObj_s *obj, int *partBits);
