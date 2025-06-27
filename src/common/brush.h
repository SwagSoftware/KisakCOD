#pragma once

struct adjacencyWinding_t // sizeof=0x34
{                                       // ...
    int numsides;                       // ...
    int sides[12];
};

struct SimplePlaneIntersection // sizeof=0x18
{                                       // ...
    float xyz[3];                       // ...
    int planeIndex[3];
};

adjacencyWinding_t *__cdecl BuildBrushdAdjacencyWindingForSide(
    float *sideNormal,
    int basePlaneIndex,
    const SimplePlaneIntersection *InPts,
    int InPtCount,
    adjacencyWinding_t *optionalOutWinding,
    int optionalOutWindingCount);
void __cdecl ReverseAdjacencyWinding(adjacencyWinding_t *w);
double __cdecl RepresentativeTriangleFromWinding(
    const float (*xyz)[3],
    int pointCount,
    const float *normal,
    int *i0,
    int *i1,
    int *i2);
int __cdecl GetPointListAllowDupes(
    int planeIndex,
    const SimplePlaneIntersection *pts,
    int ptCount,
    const SimplePlaneIntersection **xyz,
    int xyzLimit);
bool __cdecl IsPtFormedByThisPlane(int plane, const SimplePlaneIntersection *pt);
char __cdecl PlaneInCommonExcluding(
    const SimplePlaneIntersection *pt1,
    const SimplePlaneIntersection *pt2,
    int excludePlane,
    int *result);
int __cdecl SecondPlane(const SimplePlaneIntersection *point, int plane);
int __cdecl ThirdPlane(const SimplePlaneIntersection *point, int plane1, int plane2);
const SimplePlaneIntersection *__cdecl RemoveNextPointFormedByThisPlane(
    int planeIndex,
    const SimplePlaneIntersection **begin,
    const SimplePlaneIntersection **end);
const SimplePlaneIntersection **__cdecl NextPointFormedByThisPlane(
    int planeIndex,
    const SimplePlaneIntersection **begin,
    const SimplePlaneIntersection **end);
double __cdecl CyclePerimiter(const SimplePlaneIntersection **pts, int ptsCount);
char __cdecl TestConvexWithoutNearPoints(const SimplePlaneIntersection **pts, unsigned int ptCount);
char __cdecl IsConvex(const float (*pts)[3], unsigned int ptCount);
bool __cdecl CycleLess(
    bool isConvex1,
    bool isConvex2,
    float perimiter1,
    float perimiter2,
    int nodeCount1,
    int nodeCount2);
int __cdecl ReduceToACycle(int basePlane, const SimplePlaneIntersection **pts, int ptsCount);
char __cdecl IntAlreadyInList(const int *list, int listCount, int value);
char __cdecl FindCycleBFS(
    int basePlane,
    const SimplePlaneIntersection **pts,
    int ptsCount,
    const SimplePlaneIntersection *start,
    const SimplePlaneIntersection *end,
    int connectingPlane,
    const SimplePlaneIntersection **resultCycle,
    int *resultCycleCount);
int __cdecl RemovePtsWithPlanesThatOccurLessThanTwice(const SimplePlaneIntersection **pts, int ptsCount);
int __cdecl NumberOfOccurancesOfPlane(int planeIndex, const SimplePlaneIntersection **pts, int ptCount);
int __cdecl GetPtsFormedByPlane(
    int planeIndex,
    const SimplePlaneIntersection **pts,
    int ptCount,
    const SimplePlaneIntersection **result,
    int maxResults);
int __cdecl ChooseEdgeToRemove(
    int basePlane,
    int connectingPlane,
    const SimplePlaneIntersection **pts,
    int ptsCount,
    const SimplePlaneIntersection **edges);
int __cdecl PartitionEdges(
    int basePlane,
    int connectingPlane,
    const SimplePlaneIntersection **pts,
    int ptsCount,
    const SimplePlaneIntersection **edges,
    int edgeCount,
    int *partition);
int __cdecl Remove(const SimplePlaneIntersection **pts, int ptsCount, const SimplePlaneIntersection *removePoint);
int __cdecl NumberOfUniquePoints(const SimplePlaneIntersection **pts, int ptsCount);
