#pragma once

#include <universal/assertive.h>

#include <math.h>

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

// TODO fun fact: if we initialize these to -0.0 instead the compiler can remove the float addition without -ffast-math or some equivalent
const vec2 vec2_origin = { 0.0, 0.0 };
const vec3 vec3_origin = { 0.0, 0.0, 0.0 };
const vec4 vec4_origin = { 0.0, 0.0, 0.0, 0.0 };

bool __cdecl Vec4HomogenousClipBothZ(vec4r pt0, vec4r pt1);
bool __cdecl Vec4HomogenousClipZW(vec4r pt0, vec4r pt1, vec4r coeffZW);
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
float __cdecl AngleNormalize360(float angle);

float __cdecl ColorNormalize(float* in, float* out);

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

void __cdecl Vec3Avg(const vec3r a, const vec3r b, vec3r sum);

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
void __cdecl Vec3Rotate(const vec3r in, const mat3x3& matrix, vec3r out);
void __cdecl Vec3RotateTranspose(const vec3r in, const mat3x3& matrix, vec3r out);

void __cdecl Vec3Basis_RightHanded(const float *forward, float *left, float *up);

void __cdecl Vec3Lerp(const float* start, const float* end, float fraction, float* endpos);

int __cdecl VecNCompareCustomEpsilon(const float* v0, const float* v1, float epsilon, int coordCount);
void __cdecl RotatePointAroundVector(float* dst, const float* dir, const float* point, float degrees);

void __cdecl YawVectors(float yaw, float* forward, float* right);

void __cdecl AngleVectors(const float* angles, float* forward, float* right, float* up);
void __cdecl AnglesToAxis(const float* angles, float (*axis)[3]);
void __cdecl AxisToQuat(const float (*mat)[3], float* out);

float __cdecl PointToBoxDistSq(const float* pt, const float* mins, const float* maxs);

float __cdecl Vec4Dot(const float* a, const float* b);
float __cdecl Vec4Normalize(float* v);
void __cdecl Vec4Mul(const float* a, const float* b, float* product);
float __cdecl Vec4LengthSq(const float* v);
void __cdecl Vec4Scale(const float* v, float scale, float* result);

void __cdecl Vec4Lerp(const float* from, const float* to, float frac, float* result);

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
void __cdecl MatrixIdentity33(mat3x3& out);
void __cdecl MatrixIdentity44(mat4x4& out);

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

void __cdecl MatrixTransposeTransformVector(const vec3r in1, const mat3x3& in2, vec3r out);
void __cdecl MatrixTransposeTransformVector43(const vec3r in1, const mat4x3&, vec3r out);

void __cdecl MatrixTransformVectorQuatTrans(const vec3r in, const struct DObjAnimMat* mat, vec3r out);

void __cdecl MatrixForViewer(float (*mtx)[4], const float *origin, const float (*axis)[3]);
void __cdecl InfinitePerspectiveMatrix(float (*mtx)[4], float tanHalfFovX, float tanHalfFovY, float zNear);


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
void __cdecl ProjectPointOnPlane(const float *p, __int64 normal);
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