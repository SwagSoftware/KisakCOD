#pragma once

struct cplane_s // sizeof=0x14
{                                       // ...
    float normal[3];                    // ...
    float dist;
    unsigned __int8 type;
    unsigned __int8 signbits;
    unsigned __int8 pad[2];
};

double __cdecl AngleDelta(float a1, float a2);
void __cdecl TRACK_com_math();
double __cdecl random();
double __cdecl crandom();
void __cdecl GaussianRandom(float *f0, float *f1);
unsigned int __cdecl RandWithSeed(int *seed);
void __cdecl PointInCircleFromUniformDeviates(float radiusDeviate, float yawDeviate, float *point);
double __cdecl LinearTrack(float tgt, float cur, float rate, float deltaTime);
double __cdecl LinearTrackAngle(float tgt, float cur, float rate, float deltaTime);
double __cdecl DiffTrack(float tgt, float cur, float rate, float deltaTime);
double __cdecl DiffTrackAngle(float tgt, float cur, float rate, float deltaTime);
double __cdecl GraphGetValueFromFraction(int knotCount, const float (*knots)[2], float fraction);
double __cdecl Q_acos(float c);
char __cdecl ClampChar(int i);
unsigned __int8 __cdecl DirToByte(const float *dir);
void __cdecl ByteToDir(unsigned int b, float *dir);
int __cdecl VecNCompareCustomEpsilon(const float *v0, const float *v1, float epsilon, int coordCount);
double __cdecl Vec2Distance(const float *v1, const float *v2);
double __cdecl Vec2DistanceSq(const float *p1, const float *p2);
void __cdecl Vec3ProjectionCoords(const float *dir, int *xCoord, int *yCoord);
double __cdecl Vec2Normalize(float *v);
double __cdecl Vec3NormalizeTo(const float *v, float *out);
double __cdecl Vec2NormalizeTo(const float *v, float *out);
void __cdecl Vec3Rotate(const float *in, const float (*matrix)[3], float *out);
void __cdecl Vec3RotateTranspose(const float *in, const float (*matrix)[3], float *out);
void __cdecl RotatePointAroundVector(float *dst, const float *dir, const float *point, float degrees);
void __cdecl Vec3Basis_RightHanded(const float *forward, float *left, float *up);
double __cdecl vectoyaw(const float *vec);
double __cdecl vectosignedyaw(const float *vec);
double __cdecl vectopitch(const float *vec);
double __cdecl vectosignedpitch(const float *vec);
void __cdecl vectoangles(const float *vec, float *angles);
void __cdecl UnitQuatToAngles(const float *quat, float *angles);
void __cdecl YawVectors(float yaw, float *forward, float *right);
void __cdecl YawVectors2D(float yaw, float *forward, float *right);
void __cdecl PerpendicularVector(const float *src, float *dst);
double __cdecl PointToBoxDistSq(const float *pt, const float *mins, const float *maxs);
void __cdecl ClosestApproachOfTwoLines(
    const float *p1,
    const float *dir1,
    const float *p2,
    const float *dir2,
    float *s,
    float *t);
void __cdecl MatrixIdentity33(float (*out)[3]);
void __cdecl MatrixIdentity44(float (*out)[4]);
void __cdecl MatrixSet44(float (*out)[4], const float *origin, const float (*axis)[3], float scale);
void __cdecl MatrixMultiply(const float (*in1)[3], const float (*in2)[3], float (*out)[3]);
void __cdecl MatrixMultiply43(const float (*in1)[3], const float (*in2)[3], float (*out)[3]);
void __cdecl MatrixMultiply44(const float (*in1)[4], const float (*in2)[4], float (*out)[4]);
void __cdecl MatrixTranspose(const float (*in)[3], float (*out)[3]);
void __cdecl MatrixTranspose44(const float *in, float *out);
void __cdecl MatrixInverseOrthogonal43(const float (*in)[3], float (*out)[3]);
void __cdecl MatrixInverse44(const float *mat, float *dst);
void __cdecl MatrixTransformVector44(const float *vec, const float (*mat)[4], float *out);
void __cdecl MatrixTransposeTransformVector(const float *in1, const float (*in2)[3], float *out);
void __cdecl MatrixTransformVector43(const float *in1, const float (*in2)[3], float *out);
void __cdecl MatrixTransposeTransformVector43(float *in1, const float (*in2)[3], float *out);
void __cdecl VectorAngleMultiply(float *vec, float angle);
void __cdecl QuatToAxis(const float *quat, float (*axis)[3]);
void __cdecl UnitQuatToAxis(const float *quat, float (*axis)[3]);
void __cdecl UnitQuatToForward(const float *quat, float *forward);
void __cdecl QuatSlerp(const float *from, const float *to, float frac, float *result);
void __cdecl QuatMultiply(const float *in1, const float *in2, float *out);
double __cdecl RotationToYaw(const float *rot);
void __cdecl AnglesSubtract(float *v1, float *v2, float *v3);
double __cdecl AngleNormalize360(float angle);
double __cdecl RadiusFromBounds(const float *mins, const float *maxs);
double __cdecl RadiusFromBounds2D(const float *mins, const float *maxs);
double __cdecl RadiusFromBoundsSq(const float *mins, const float *maxs);
double __cdecl RadiusFromBounds2DSq(const float *mins, const float *maxs);
void __cdecl ExtendBounds(float *mins, float *maxs, const float *offset);
void __cdecl ExpandBoundsToWidth(float *mins, float *maxs);
void __cdecl ShrinkBoundsToHeight(float *mins, float *maxs);
void __cdecl ClearBounds2D(float *mins, float *maxs);
void __cdecl AddPointToBounds(const float *v, float *mins, float *maxs);
void __cdecl AddPointToBounds2D(const float *v, float *mins, float *maxs);
bool __cdecl PointInBounds(const float *v, const float *mins, const float *maxs);
bool __cdecl BoundsOverlap(const float *mins0, const float *maxs0, const float *mins1, const float *maxs1);
void __cdecl ExpandBounds(const float *addedmins, const float *addedmaxs, float *mins, float *maxs);
void __cdecl AxisClear(float (*axis)[3]);
void __cdecl AxisTranspose(const float (*in)[3], float (*out)[3]);
void __cdecl AxisTransformVec3(const float (*axes)[3], const float *vec, float *out);
void __cdecl YawToAxis(float yaw, float (*axis)[3]);
void __cdecl AxisToAngles(const float (*axis)[3], float *angles);
int __cdecl IntersectPlanes(const float **plane, float *xyz);
void __cdecl SnapPointToIntersectingPlanes(const float **planes, float *xyz, float snapGrid, float snapEpsilon);
int __cdecl ProjectedWindingContainsCoplanarPoint(
    const float (*verts)[3],
    int vertCount,
    int x,
    int y,
    const float *point);
int __cdecl PlaneFromPoints(float *plane, const float *v0, const float *v1, const float *v2);
void __cdecl ProjectPointOnPlane(const float *p, __int64 normal);
void __cdecl SetPlaneSignbits(cplane_s *out);
bool __cdecl BoxDistSqrdExceeds(const float *absmin, const float *absmax, const float *org, float fogOpaqueDistSqrd);
double __cdecl Q_rint(float in);
double __cdecl ColorNormalize(float *in, float *out);
double __cdecl PitchForYawOnNormal(float fYaw, const float *normal);
void __cdecl NearestPitchAndYawOnPlane(const float *angles, const float *normal, float *result);
void __cdecl Rand_Init(int seed);
double __cdecl flrand(float min, float max);
int __cdecl irand(int min, int max);
void __cdecl MatrixTransformVectorQuatTrans(const float *in, const struct DObjAnimMat *mat, float *out);
void __cdecl AxisToQuat(const float (*mat)[3], float *out);
void __cdecl QuatLerp(const float *qa, const float *qb, float frac, float *out);
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
void __cdecl AxisCopy(const float (*in)[3], float (*out)[3]);
double __cdecl log(double X);



//=============================================

void __cdecl AngleVectors(const float *angles, float *forward, float *right, float *up);
void __cdecl AnglesToAxis(const float *angles, float (*axis)[3]);
double __cdecl Vec4Normalize(float *v);
void __cdecl AnglesToQuat(const float *angles, float *quat);

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