#pragma once
#include <ode/collision.h>
#include "phys_local.h"

struct SeparatingAxisInfo // sizeof=0x14
{                                       // ...
    float bestDepth;                    // ...
    unsigned int bestAxis;              // ...
    float bestContactNormal[3];         // ...
};


// phys_coll_cylinderbrush
void __cdecl Phys_CollideCylinderWithBrush(const cbrush_t *brush, const objInfo *info, Results *results);
void __cdecl Phys_CollideCylinderWithFace(
    const float *polyPlane,
    const Poly *poly,
    const objInfo *info,
    int surfaceFlags,
    Results *results);
void __cdecl Phys_PushPolyOutOfCylinderEndcapPlane(
    Results *results,
    const Poly *poly,
    const float *polyNormal,
    const objInfo *info,
    int surfaceFlags);
unsigned int __cdecl Phys_ClipPolygonAgainstCylinderRadius(
    const Poly *poly,
    const objInfo *info,
    float (*result)[3],
    unsigned int maxVerts);
unsigned int __cdecl Phys_ClipLineSegmentAgainstCylinderRadius(
    const float *pt1,
    const float *pt2,
    const objInfo *info,
    float (*result)[3]);
char __cdecl Phys_CylinderFaceTestSeparatingAxes(
    const float *polyPlane,
    const Poly *poly,
    const objInfo *info,
    SeparatingAxisInfo *axisInfo);
char __cdecl Phys_CylinderFaceTestAxis(
    const float *polyNormal,
    const Poly *poly,
    const objInfo *info,
    const float *axis,
    float depthEpsilon,
    SeparatingAxisInfo *axisInfo,
    unsigned int axisNumber,
    bool testForRejectionOnly);
char __cdecl Phys_TestCircleToEdgeAxis(
    const float *polyNormal,
    const Poly *poly,
    const float *pt1,
    const float *edge,
    const objInfo *info,
    const float *circleCenter,
    SeparatingAxisInfo *axisInfo,
    unsigned int axisNumber);
void __cdecl Phys_ClipCylinderEdgeToPoly(
    SeparatingAxisInfo *axisInfo,
    const float *polyPlane,
    const Poly *poly,
    const objInfo *info,
    int surfaceFlags,
    Results *results);
void __cdecl Phys_AddContactForPlane(Results *results, float *pt, const float *plane, float *normal, int surfaceFlags);
void __cdecl Phys_PushEdgeAwayFromCylinderCircle(
    const float *ptOnEdge,
    const float *pt2OnEdge,
    const float *edge,
    float *contactNormal,
    const objInfo *info,
    int surfaceFlags,
    Results *results);
unsigned int __cdecl Phys_IntersectionOfCircleWithPlane(
    const float *plane,
    const float *circleCenter,
    const float *circleAxis,
    float circleRadius,
    float *pt1,
    float *pt2);
void __cdecl Phys_ClipCylinderEndcapToPoly(
    const float *polyPlane,
    const Poly *poly,
    const objInfo *info,
    int surfaceFlags,
    Results *results);
void __cdecl Phys_PushLinesAway(
    const float *p1,
    const float *dir1,
    const float *p2,
    const float *dir2,
    float *contactNormal,
    float depth,
    Results *results,
    int surfaceFlags);
double __cdecl Phys_DistanceOfCylinderFromPlane(const float *plane, const objInfo *info);
void __cdecl Phys_CollideCylinderWithTriangleList(
    const unsigned __int16 *a_indices,
    const float (*verts)[3],
    unsigned int triCount,
    const objInfo *info,
    int surfaceFlags,
    Results *results);



// phys_coll_capsulebrush
struct Capsule // sizeof=0x40
{                                       // ...
    float p0[3];
    float p1[3];
    float center[3];
    float axis[3];
    float radius;
    float sqRadius;
    float halfLength;
    float halfHeight;
};
struct LocalContactData // sizeof=0x24
{                                       // ...
    float pos[3];
    float normal[3];                    // ...
    float depth;
    int surfFlags;                      // ...
    int inUse;                          // ...
};
struct AxisTestResults // sizeof=0x1C
{                                       // ...
    float bestDepth;
    float bestCenter;                   // ...
    int bestAxis;
    float bestRt;                       // ...
    float normal[3];                    // ...
};
void __cdecl Phys_CollideCapsuleWithBrush(const cbrush_t *brush, const objInfo *info, Results *results);
void __cdecl Phys_CapsuleBuildContactsForTri(
    Results *results,
    const float *plane,
    const Capsule *capsule,
    const float *p0,
    const float *p1,
    const float *p2,
    int surfaceFlags);
bool __cdecl Phys_CapsuleSeparatingAxisTest(
    AxisTestResults *axisResults,
    const float *plane,
    const Capsule *capsule,
    const float *tri0,
    const float *tri1,
    const float *tri2);
char __cdecl Phys_TestAxis(
    AxisTestResults *axisResults,
    const Capsule *capsule,
    const float *p0,
    const float *p1,
    const float *p2,
    const float *inAxis,
    int axisNum,
    bool noFlip);
void __cdecl Phys_CapsuleBuildContactsForTriEndEdges(
    Results *results,
    const float *plane,
    const Capsule *capsule,
    const float *p0,
    const float *p1,
    const float *p2,
    int surfaceFlags);
bool __cdecl Phys_CapsuleSeparatingAxisTestEndEdges(
    AxisTestResults *axisResults,
    const float *plane,
    const Capsule *capsule,
    const float *tri0,
    const float *tri1,
    const float *tri2);
void __cdecl Phys_CapsuleBuildContactsForTriMiddleEdge(
    Results *results,
    const float *plane,
    const Capsule *capsule,
    const float *p0,
    const float *p1,
    const float *p2,
    int surfaceFlags);
bool __cdecl Phys_CapsuleSeparatingAxisTestMiddleEdge(
    AxisTestResults *axisResults,
    const float *plane,
    const Capsule *capsule,
    const float *tri0,
    const float *tri1,
    const float *tri2);
bool __cdecl Phys_TestCapsulePlane(const float *plane, const Capsule *capsule);
void __cdecl Phys_CollideCapsuleWithTriangleList(
    const unsigned __int16 *a_indices,
    const float (*verts)[3],
    unsigned int triCount,
    const objInfo *info,
    int surfaceFlags,
    Results *results);
