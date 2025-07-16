#pragma once

#include <universal/assertive.h>

#include <math.h>

#define EQUAL_EPSILON 0.001f
#define WEIGHT_EPSILON EQUAL_EPSILON

#define CLAMP(x, low, high) ((x) < (low) ? (low) : ((x) > (high) ? (high) : (x)))

struct cplane_s // sizeof=0x14
{                                       // ...
    float normal[3];                    // ...
    float dist;
    unsigned __int8 type;
    unsigned __int8 signbits;
    unsigned __int8 pad[2];
};

union PackedUnitVec // sizeof=0x4
{                                       // ...
    operator unsigned int()
    {
        return packed;
    }
    operator int()
    {
        return packed;
    }
    unsigned int packed;
    unsigned __int8 array[4];
};

using vec2 = float[2];
using vec3 = float[3];
using vec4 = float[4];

// TODO change if we ever actually use classes
#define vec2r float*
#define vec3r float*
#define vec4r float*

// note - row major order
using mat3x3 = float[3][3];
using mat4x3 = float[4][3];
using mat4x4 = float[4][4];

struct GfxMatrix; // sizeof=0x40

// TODO fun fact: if we initialize these to -0.0 instead the compiler can remove the float addition without -ffast-math or some equivalent
constexpr vec2 vec2_origin = { 0.0, 0.0 };
constexpr vec3 vec3_origin = { 0.0, 0.0, 0.0 };
constexpr vec4 vec4_origin = { 0.0, 0.0, 0.0, 0.0 };

using uint4 = unsigned int[4];

// NOTE: yes, these really seem to be different matrix types specific to the FX system for some reason..
union float4 {
    vec4 v;
    uint4 u;
    PackedUnitVec unitVec[4];
};

union float4x3 {
    union {
        float4 x;
        float4 y;
        float4 z;
    };

    mat4x3 mat;
};

union float4x4 {
    struct {
        float4 x;
        float4 y;
        float4 z;
        float4 w;
    };

    mat4x4 mat;
};

constexpr float4 g_zero = {
    .v = {
        0.0, 0.0, 0.0, 0.0
    }
};

constexpr float4 g_unit = {
    .v = {
        0.0, 0.0, 0.0, 1.0
    }
};

constexpr float4 g_2xunit = {
    .v = {
        0.0, 0.0, 0.0, 1.0
    }
};

constexpr float4 g_swizzleXYZA = {
    .u = {
        0x00010203,
        0x04050607,
        0x08090A0B,
        0x10111213
    }
};

constexpr float4 g_swizzleYZXW = {
    .u = {
        0x04050607,
        0x08090A0B,
        0x00010203,
        0x0C0D0E0F
    }
};

constexpr float4 g_keepXYW = {
    .u = {
        0xFFFFFFFF,
        0xFFFFFFFF,
        0,
        0xFFFFFFFF
    }
};

constexpr float4 g_keepXYZ = {
    .u = {
        0xFFFFFFFF,
        0xFFFFFFFF,
        0xFFFFFFFF,
        0
    }
};


struct vector3
{
    float4 x;
    float4 y;
    float4 z;
};
struct vector4 : vector3
{
    float4 w;
};


bool __cdecl Vec4IsNormalized(const vec4r v);

void __cdecl TRACK_com_math();

// == RANDOM == 
void __cdecl Rand_Init(int seed);

float __cdecl random();
float __cdecl crandom();

float __cdecl flrand(float min, float max);
int __cdecl irand(int min, int max);

unsigned int __cdecl RandWithSeed(int* seed);
void __cdecl GaussianRandom(float* f0, float* f1);
void __cdecl PointInCircleFromUniformDeviates(float radiusDeviate, float yawDeviate, float* point);

// == SCALAR FUNCTIONS ==
float __cdecl Q_rint(float in);
float __cdecl Q_acos(float c);
float __cdecl Q_rsqrt(float number);

float __cdecl DiffTrack(float tgt, float cur, float rate, float deltaTime);
float __cdecl DiffTrackAngle(float tgt, float cur, float rate, float deltaTime);

float __cdecl LinearTrack(float tgt, float cur, float rate, float deltaTime);
float __cdecl LinearTrackAngle(float tgt, float cur, float rate, float deltaTime);

float __cdecl GraphGetValueFromFraction(int knotCount, const float (*knots)[2], float fraction);

float __cdecl AngleDelta(float a1, float a2);
void __cdecl AnglesSubtract(float* v1, float* v2, float* v3);

float __cdecl RotationToYaw(const float* rot);
float AngleNormalize360(float angle);
float AngleNormalize180(float angle);
float AngleSubtract(float a1, float a2);

float __cdecl ColorNormalize(const float *in, float *out);

float __cdecl PitchForYawOnNormal(float fYaw, const float* normal);

// == PACKING ==
unsigned __int8 __cdecl DirToByte(const float *dir);
void __cdecl ByteToDir(unsigned int b, float *dir);

// == VECTOR FUNCTIONS ==
float __cdecl Vec2Distance(const vec2r v1, const vec2r v2);
float __cdecl Vec2DistanceSq(const vec2r p1, const vec2r p2);
float __cdecl Vec2Normalize(vec2r v);
float __cdecl Vec2NormalizeTo(const vec2r v, vec2r out);
float __cdecl Vec2Length(const vec2r v);
void __cdecl YawVectors2D(float yaw, vec2r forward, vec2r right);

void __cdecl Vec3Add(const vec3r a, const vec3r b, vec3r sum);
void __cdecl Vec3Sub(const vec3r a, const vec3r b, vec3r diff);
void __cdecl Vec3Mul(const vec3r a, const vec3r b, vec3r product);

void __cdecl Vec3Negate(const vec3r from, vec3r to);

void __cdecl Vec3Avg(const vec3r a, const vec3r b, vec3r result);

float __cdecl Vec3Dot(const vec3r a, const vec3r b);
void __cdecl Vec3Cross(const vec3r v0, const vec3r v1, vec3r cross);
float __cdecl Vec3LengthSq(const vec3r v);
void __cdecl Vec3Scale(const vec3r v, float scale, vec3r result);
// :)
inline float __cdecl I_fres(float val)
{
    iassert(val != 0.0);
    return (float)(1.0 / val);
}

void __cdecl Vec3ScaleMad(float scale0, const vec3r dir0, float scale1, const vec3r dir1, vec3r result);
float __cdecl Vec3Normalize(float* v);
bool __cdecl Vec3IsNormalized(const vec3r v);

float __cdecl Vec3Length(const vec3r v);
void __cdecl Vec3Copy(const vec3r from, vec3r to);

void __cdecl Vec3ProjectionCoords(const float *dir, int *xCoord, int *yCoord);
float __cdecl Vec3NormalizeTo(const vec3r v, vec3r out);
void __cdecl Vec3Mad(const vec3r start, float scale, const vec3r dir, vec3r result);
void __cdecl Vec3Accum(const float *subTotal, const float *weight, const float *added, float *total);
void __cdecl Vec3Rotate(const vec3r in, const mat3x3& matrix, vec3r out);
void __cdecl Vec3RotateTranspose(const vec3r in, const mat3x3& matrix, vec3r out);

void __cdecl Vec3AddScalar(const float* a, float s, float* sum);

void __cdecl Vec3Basis_RightHanded(const float *forward, float *left, float *up);

void __cdecl Vec3Lerp(const float* start, const float* end, float fraction, float* endpos);

int __cdecl VecNCompareCustomEpsilon(const float* v0, const float* v1, float epsilon, int coordCount);
void __cdecl RotatePointAroundVector(float* dst, const float* dir, const float* point, float degrees);

void __cdecl YawVectors(float yaw, float* forward, float* right);

void __cdecl AngleVectors(const float* angles, float* forward, float* right, float* up);
void __cdecl AnglesToAxis(const float* angles, float axis[3][3]);
void __cdecl AxisToQuat(const float (*mat)[3], float* out);

float __cdecl PointToBoxDistSq(const float* pt, const float* mins, const float* maxs);

void __cdecl Vec4Set(float *v, float x, float y, float z, float w);
bool __cdecl Vec4Compare(const float *a, const float *b);
float __cdecl Vec4Dot(const float* a, const float* b);
float __cdecl Vec4Normalize(float* v);
void __cdecl Vec4Mul(const float* a, const float* b, float* product);
float __cdecl Vec4LengthSq(const float* v);
void __cdecl Vec4Scale(const float* v, float scale, float* result);
void __cdecl Vec4Mad(const float *start, float scale, const float *dir, float *result);
void __cdecl Vec4Add(const float *a, const float *b, float *sum);
void __cdecl Vec4Sub(const float *a, const float *b, float *diff);
void __cdecl Vec4MadMad(
    const float *start,
    float scale0,
    const float *dir0,
    float scale1,
    const float *dir1,
    float *result);

void __cdecl Vec4Lerp(const float* from, const float* to, float frac, float* result);
float __cdecl Vec4Length(const float *v);

float __cdecl vectoyaw(const float *vec);
float __cdecl vectosignedyaw(const float *vec);
float __cdecl vectopitch(const float *vec);
float __cdecl vectosignedpitch(const float *vec);
void __cdecl vectoangles(const float *vec, float *angles);

void __cdecl PerpendicularVector(const float *src, float *dst);

void __cdecl VectorAngleMultiply(float* vec, float angle);

void __cdecl AxisClear(mat3x3& axis);
void __cdecl AxisTranspose(const mat3x3& in, mat3x3& out);
void __cdecl AxisTransformVec3(const mat3x3& axis, const float* vec, float* out);

void __cdecl YawToAxis(float yaw, mat3x3& axis);
void __cdecl AxisToAngles(const mat3x3& axis, vec3r angles);

// == MATRICES ==
void __cdecl OrthographicMatrix(mat4x4 &mtx, float width, float height, float depth);

void __cdecl MatrixIdentity33(mat3x3 &out);
void __cdecl MatrixIdentity44(mat4x4 &out);

void __cdecl MatrixSet44(mat4x4& out, const vec3r origin, const mat3x3& axis, float scale);

void __cdecl MatrixMultiply(const mat3x3& in1, const mat3x3& in2, mat3x3& out);
void __cdecl MatrixMultiply43(const mat4x3& in1, const mat4x3& int2, mat4x3& out);
void __cdecl MatrixMultiply44(const mat4x4& in1, const mat4x4& int2, mat4x4& out);

void __cdecl MatrixTranspose(const mat3x3& in, mat3x3& out);
void __cdecl MatrixTranspose44(const mat4x4& in, mat4x4& out);

void __cdecl MatrixInverseOrthogonal43(const mat4x3& in, mat4x3& out);
void __cdecl MatrixInverse44(const mat4x4& mat, mat4x4& dst);

void __cdecl MatrixTransformVector(const vec3r in1, const mat3x3& in2, vec3r out);
void __cdecl MatrixTransformVector43(const vec3r in1, const mat4x3& in2, vec3r out);
void __cdecl MatrixTransformVector44(const vec4r vec, const mat4x4& mat, vec4r out);

void __cdecl InvMatrixTransformVectorQuatTrans(const float *in, const struct DObjAnimMat *mat, float *out);

void __cdecl MatrixTransposeTransformVector(const vec3r in1, const mat3x3& in2, vec3r out);
void __cdecl MatrixTransposeTransformVector43(const vec3r in1, const mat4x3&, vec3r out);

void __cdecl MatrixTransformVectorQuatTrans(const vec3r in, const struct DObjAnimMat* mat, vec3r out);

void __cdecl MatrixForViewer(mat4x4 &mtx, const vec3r origin, const mat3x3 &axis);
void __cdecl InfinitePerspectiveMatrix(float (*mtx)[4], float tanHalfFovX, float tanHalfFovY, float zNear);

void __cdecl FinitePerspectiveMatrix(float (*mtx)[4], float tanHalfFovX, float tanHalfFovY, float zNear, float zFar);

float LerpAngle(float from, float to, float frac);

void __cdecl ClearBounds(float *mins, float *maxs);

float __cdecl RadiusFromBounds(const float *mins, const float *maxs);
float __cdecl RadiusFromBounds2D(const float *mins, const float *maxs);

float __cdecl RadiusFromBoundsSq(const float *mins, const float *maxs);
float __cdecl RadiusFromBounds2DSq(const float *mins, const float *maxs);

void __cdecl ExtendBounds(float *mins, float *maxs, const float *offset);
void __cdecl ExpandBoundsToWidth(float *mins, float *maxs);
void __cdecl ShrinkBoundsToHeight(float *mins, float *maxs);
void __cdecl ClearBounds2D(float *mins, float *maxs);
void __cdecl AddPointToBounds(const float *v, float *mins, float *maxs);
void __cdecl AddPointToBounds2D(const float *v, float *mins, float *maxs);
bool __cdecl PointInBounds(const float *v, const float *mins, const float *maxs);
bool __cdecl BoundsOverlap(const float *mins0, const float *maxs0, const float *mins1, const float *maxs1);
void __cdecl ExpandBounds(const float *addedmins, const float *addedmaxs, float *mins, float *maxs);

int __cdecl ProjectedWindingContainsCoplanarPoint(
    const float (*verts)[3],
    int vertCount,
    int x,
    int y,
    const float *point);

// == PLANES ==

int __cdecl IntersectPlanes(const float** plane, float* xyz);
void __cdecl SnapPointToIntersectingPlanes(const float** planes, float* xyz, float snapGrid, float snapEpsilon);
int __cdecl PlaneFromPoints(float *plane, const float *v0, const float *v1, const float *v2);
void __cdecl ProjectPointOnPlane(const float * const f1, const float * const normal, float * const result);
void __cdecl SetPlaneSignbits(cplane_s *out);
void __cdecl NearestPitchAndYawOnPlane(const float *angles, const float *normal, float *result);

// == BOXES ==
bool __cdecl BoxDistSqrdExceeds(const float* absmin, const float* absmax, const float* org, float fogOpaqueDistSqrd);

bool __cdecl CullBoxFromCone(
    const float *coneOrg,
    const float *coneDir,
    float cosHalfFov,
    const float *boxCenter,
    const float *boxHalfSize);
bool __cdecl CullBoxFromSphere(const float *sphereOrg, float radius, const float *boxCenter, const float *boxHalfSize);
bool __cdecl CullBoxFromConicSectionOfSphere(
    const float *coneOrg,
    const float *coneDir,
    float cosHalfFov,
    float radius,
    const float *boxCenter,
    const float *boxHalfSize);
bool __cdecl CullSphereFromCone(
    const float *coneOrg,
    const float *coneDir,
    float cosHalfFov,
    const float *sphereCenter,
    float radius);
void __cdecl AxisCopy(const mat3x3 &in, mat3x3& out);

// == QUATERNIONS ==
void __cdecl AnglesToQuat(const float* angles, float* quat);
void __cdecl UnitQuatToAngles(const float* quat, float* angles);

void __cdecl QuatToAxis(const float* quat, mat3x3& axis);
void __cdecl UnitQuatToAxis(const float* quat, mat3x3& axis);
void __cdecl UnitQuatToForward(const float* quat, float* forward);
void __cdecl QuatSlerp(const float* from, const float* to, float frac, float* result);
void __cdecl QuatMultiply(const float* in1, const float* in2, float* out);
void __cdecl QuatLerp(const float* qa, const float* qb, float frac, float* out);

// == MISC ==
char __cdecl ClampChar(int i);

void __cdecl ClosestApproachOfTwoLines(
    const float* p1,
    const float* dir1,
    const float* p2,
    const float* dir2,
    float* s,
    float* t);

// KISAK ADDITION: pray that the optimizer doesn't shit the bed
__forceinline static float COERCE_FLOAT(unsigned val) {
    union {
        unsigned v;
        float f;
    } lol = { val };
    return lol.f;
}

__forceinline static unsigned int COERCE_UNSIGNED_INT(float val) {
    union {
        float f;
        unsigned v;
    } lol = { val };
    return lol.v;
}

__forceinline static int COERCE_INT(float val) {
    union {
        float f;
        int v;
    } lol = { val };
    return lol.v;
}

#define IS_NAN(x) (isnan(x))

// LWSS: There appear to be a lot more functions on XBox.
//__Eg_fltMin@@YAXXZ       8278c5f0 f   com_math_anglevectors.obj
//__Eg_negativeZero@@YAXXZ 8278c610 f   com_math_anglevectors.obj
//__Eg_inc@@YAXXZ          8278c630 f   com_math_anglevectors.obj
//__Eg_swizzleXYZW@@YAXXZ  8278c650 f   com_math_anglevectors.obj
//__Eg_swizzleYXZW@@YAXXZ  8278c670 f   com_math_anglevectors.obj
//__Eg_swizzleXZYW@@YAXXZ  8278c690 f   com_math_anglevectors.obj
//__Eg_swizzleYXWZ@@YAXXZ  8278c6b0 f   com_math_anglevectors.obj
//__Eg_swizzleXAZC@@YAXXZ  8278c6d0 f   com_math_anglevectors.obj
//__Eg_swizzleYBWD@@YAXXZ  8278c6f0 f   com_math_anglevectors.obj
//__Eg_swizzleXYAB@@YAXXZ  8278c710 f   com_math_anglevectors.obj
//__Eg_swizzleZWCD@@YAXXZ  8278c730 f   com_math_anglevectors.obj
//__Eg_swizzleXYZA@@YAXXZ  8278c750 f   com_math_anglevectors.obj
//__Eg_swizzleYZXW@@YAXXZ  8278c770 f   com_math_anglevectors.obj
//__Eg_swizzleZXYW@@YAXXZ  8278c790 f   com_math_anglevectors.obj
//__Eg_swizzleWABW@@YAXXZ  8278c7b0 f   com_math_anglevectors.obj
//__Eg_swizzleZWAW@@YAXXZ  8278c7d0 f   com_math_anglevectors.obj
//__Eg_swizzleYZWA@@YAXXZ  8278c7f0 f   com_math_anglevectors.obj
//__Eg_swizzleXXXX@@YAXXZ  8278c810 f   com_math_anglevectors.obj
//__Eg_swizzleYYYY@@YAXXZ  8278c830 f   com_math_anglevectors.obj
//__Eg_swizzleZZZZ@@YAXXZ  8278c850 f   com_math_anglevectors.obj
//__Eg_swizzleWWWW@@YAXXZ  8278c870 f   com_math_anglevectors.obj
//__Eg_selectX@@YAXXZ      8278c890 f   com_math_anglevectors.obj
//__Eg_selectY@@YAXXZ      8278c8b0 f   com_math_anglevectors.obj
//__Eg_selectZ@@YAXXZ      8278c8d0 f   com_math_anglevectors.obj
//__Eg_selectW@@YAXXZ      8278c8f0 f   com_math_anglevectors.obj
//__Eg_keepYZW@@YAXXZ      8278c910 f   com_math_anglevectors.obj
//__Eg_keepXZW@@YAXXZ      8278c930 f   com_math_anglevectors.obj
//__Eg_keepXYW@@YAXXZ      8278c950 f   com_math_anglevectors.obj
//__Eg_keepXYZ@@YAXXZ      8278c970 f   com_math_anglevectors.obj
//__Eg_keepXY@@YAXXZ       8278c990 f   com_math_anglevectors.obj
//__Eg_keepZW@@YAXXZ       8278c9b0 f   com_math_anglevectors.obj
//__Eg_keepX@@YAXXZ        8278c9d0 f   com_math_anglevectors.obj
//__Eg_keepZ@@YAXXZ        8278c9f0 f   com_math_anglevectors.obj
//__Eg_packedMaskQuat@@YAXXZ 8278ca10 f   com_math_anglevectors.obj
//__Eg_unpackSignBitQuat@@YAXXZ 8278ca30 f   com_math_anglevectors.obj
//__Eg_unpackAndQuat@@YAXXZ 8278ca50 f   com_math_anglevectors.obj
//__Eg_packMantissa@@YAXXZ 8278ca70 f   com_math_anglevectors.obj
//__Eg_packedMaskSimpleQuat@@YAXXZ 8278ca90 f   com_math_anglevectors.obj
//__Eg_unpackSignBitSimpleQuat@@YAXXZ 8278cab0 f   com_math_anglevectors.obj
//__Eg_unpackAndSimpleQuat@@YAXXZ 8278cad0 f   com_math_anglevectors.obj
//__Eg_unpackQuatRotateMask@@YAXXZ 8278caf0 f   com_math_anglevectors.obj
//__Eg_unpackQuatSignBit@@YAXXZ 8278cb10 f   com_math_anglevectors.obj
//__Eg_unpackSimpleQuatRotateMask@@YAXXZ 8278cb30 f   com_math_anglevectors.obj
//__Eg_unpackQuatRotatePerm@@YAXXZ 8278cb50 f   com_math_anglevectors.obj
//__Eg_unpackQuatRotateAdd@@YAXXZ 8278cb70 f   com_math_anglevectors.obj
//__Eg_unpackSimpleQuatRotatePerm@@YAXXZ 8278cb90 f   com_math_anglevectors.obj
//__Eg_unpackSimpleQuatRotateAdd@@YAXXZ 8278cbb0 f   com_math_anglevectors.obj
//__Eg_unpackShiftQuat@@YAXXZ 8278cbd0 f   com_math_anglevectors.obj
//__Eg_unpackSignShiftQuat@@YAXXZ 8278cbf0 f   com_math_anglevectors.obj
//__Eg_unpackShiftSimpleQuat@@YAXXZ 8278cc10 f   com_math_anglevectors.obj

//__int128 _mask__NegFloat_ = 0x80000000800000008000000080000000;

struct DObjAnimMat;
struct DObjSkelMat;

void __cdecl ConvertQuatToSkelMat(const DObjAnimMat *const mat, DObjSkelMat *skelMat);
void __cdecl ConvertQuatToInverseSkelMat(const DObjAnimMat *const mat, DObjSkelMat *skelMat);
void __cdecl QuatMultiplyInverse(const float *in1, const float *in2, float *out);
void __cdecl QuatMultiplyReverseInverse(const float *in1, const float *in2, float *out);
void __cdecl QuatMultiplyReverseEquals(const float *in, float *inout);
void __cdecl ConvertQuatToMat(const struct DObjAnimMat *mat, float (*axis)[3]);
void __cdecl ConvertQuatToInverseMat(const DObjAnimMat *mat, float (*axis)[3]);
void __cdecl QuatMultiplyEquals(const float *in, float *inout);
void __cdecl MatrixTransformVectorQuatTransEquals(const struct DObjAnimMat *in, float *inout);
void __cdecl R_TransformSkelMat(const float *origin, const DObjSkelMat *mat, float *out);

/*
===========================================================================
Copyright (C) 1999-2005 Id Software, Inc.

This file is part of Quake III Arena source code.

Quake III Arena source code is free software; you can redistribute it
and/or modify it under the terms of the GNU General Public License as
published by the Free Software Foundation; either version 2 of the License,
or (at your option) any later version.

Quake III Arena source code is distributed in the hope that it will be
useful, but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with Foobar; if not, write to the Free Software
Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
===========================================================================
*/
inline float __cdecl Q_rsqrt(float number)
{
    iassert(number);
    iassert(!isnan(number));

    union standards_compliant_fp_bit_hack {
        int i;
        float f;
    };

    standards_compliant_fp_bit_hack v;
    float x2, y;
    const float threehalfs = 1.5F;

    x2 = number * 0.5F;
    y = number;
    v.f = y;						// evil floating point bit level hacking
    v.i = 0x5f3759df - (v.i >> 1);               // what the fuck?
    y = v.f;
    y = y * (threehalfs - (x2 * y * y));   // 1st iteration
    //	y  = y * ( threehalfs - ( x2 * y * y ) );   // 2nd iteration, this can be removed

    // TODO: use rsqrtss instead since it's not 1990 anymore
    return y;
}

#define I_rsqrt Q_rsqrt