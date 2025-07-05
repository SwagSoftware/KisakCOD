#pragma once
#include <cstdint>

struct adjacencyWinding_t // sizeof=0x34
{                                       // ...
    int32_t numsides;                       // ...
    int32_t sides[12];
};
static_assert(sizeof(adjacencyWinding_t) == 0x34);

struct SimplePlaneIntersection // sizeof=0x18
{                                       // ...
    float xyz[3];                       // ...
    int32_t planeIndex[3];
};
static_assert(sizeof(SimplePlaneIntersection) == 0x18);

adjacencyWinding_t *__cdecl BuildBrushdAdjacencyWindingForSide(
    float *sideNormal,
    int32_t basePlaneIndex,
    const SimplePlaneIntersection *InPts,
    int32_t InPtCount,
    adjacencyWinding_t *optionalOutWinding,
    int32_t optionalOutWindingCount);
void __cdecl ReverseAdjacencyWinding(adjacencyWinding_t *w);
double __cdecl RepresentativeTriangleFromWinding(
    const float (*xyz)[3],
    int32_t pointCount,
    const float *normal,
    int32_t *i0,
    int32_t *i1,
    int32_t *i2);
int32_t __cdecl GetPointListAllowDupes(
    int32_t planeIndex,
    const SimplePlaneIntersection *pts,
    int32_t ptCount,
    const SimplePlaneIntersection **xyz,
    int32_t xyzLimit);
bool __cdecl IsPtFormedByThisPlane(int32_t plane, const SimplePlaneIntersection *pt);
char __cdecl PlaneInCommonExcluding(
    const SimplePlaneIntersection *pt1,
    const SimplePlaneIntersection *pt2,
    int32_t excludePlane,
    int32_t *result);
int32_t __cdecl SecondPlane(const SimplePlaneIntersection *point, int32_t plane);
int32_t __cdecl ThirdPlane(const SimplePlaneIntersection *point, int32_t plane1, int32_t plane2);
const SimplePlaneIntersection *__cdecl RemoveNextPointFormedByThisPlane(
    int32_t planeIndex,
    const SimplePlaneIntersection **begin,
    const SimplePlaneIntersection **end);
const SimplePlaneIntersection **__cdecl NextPointFormedByThisPlane(
    int32_t planeIndex,
    const SimplePlaneIntersection **begin,
    const SimplePlaneIntersection **end);
double __cdecl CyclePerimiter(const SimplePlaneIntersection **pts, int32_t ptsCount);
char __cdecl TestConvexWithoutNearPoints(const SimplePlaneIntersection **pts, uint32_t ptCount);
char __cdecl IsConvex(const float (*pts)[3], uint32_t ptCount);
bool __cdecl CycleLess(
    bool isConvex1,
    bool isConvex2,
    float perimiter1,
    float perimiter2,
    int32_t nodeCount1,
    int32_t nodeCount2);
int32_t __cdecl ReduceToACycle(int32_t basePlane, const SimplePlaneIntersection **pts, int32_t ptsCount);
char __cdecl IntAlreadyInList(const int32_t *list, int32_t listCount, int32_t value);
char __cdecl FindCycleBFS(
    int32_t basePlane,
    const SimplePlaneIntersection **pts,
    int32_t ptsCount,
    const SimplePlaneIntersection *start,
    const SimplePlaneIntersection *end,
    int32_t connectingPlane,
    const SimplePlaneIntersection **resultCycle,
    int32_t *resultCycleCount);
int32_t __cdecl RemovePtsWithPlanesThatOccurLessThanTwice(const SimplePlaneIntersection **pts, int32_t ptsCount);
int32_t __cdecl NumberOfOccurancesOfPlane(int32_t planeIndex, const SimplePlaneIntersection **pts, int32_t ptCount);
int32_t __cdecl GetPtsFormedByPlane(
    int32_t planeIndex,
    const SimplePlaneIntersection **pts,
    int32_t ptCount,
    const SimplePlaneIntersection **result,
    int32_t maxResults);
int32_t __cdecl ChooseEdgeToRemove(
    int32_t basePlane,
    int32_t connectingPlane,
    const SimplePlaneIntersection **pts,
    int32_t ptsCount,
    const SimplePlaneIntersection **edges);
int32_t __cdecl PartitionEdges(
    int32_t basePlane,
    int32_t connectingPlane,
    const SimplePlaneIntersection **pts,
    int32_t ptsCount,
    const SimplePlaneIntersection **edges,
    int32_t edgeCount,
    int32_t *partition);
int32_t __cdecl Remove(const SimplePlaneIntersection **pts, int32_t ptsCount, const SimplePlaneIntersection *removePoint);
int32_t __cdecl NumberOfUniquePoints(const SimplePlaneIntersection **pts, int32_t ptsCount);
