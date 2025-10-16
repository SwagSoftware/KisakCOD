#pragma once

#include <xanim/xanim.h>

#define BSP_VERSION 22

struct DiskPrimaryLight_Version16 // sizeof=0x60
{
    unsigned __int16 falloffStart;
    unsigned __int8 falloffSizeLessOne;
    unsigned __int8 type;
    float color[3];
    float dir[3];
    float origin[3];
    float radius;
    float cosHalfFovOuter;
    float cosHalfFovInner;
    int exponent;
    char defName[40];
};
struct DiskPrimaryLight // sizeof=0x80
{
    unsigned __int8 type;
    unsigned __int8 canUseShadowMap;
    unsigned __int8 unused[2];
    float color[3];
    float dir[3];
    float origin[3];
    float radius;
    float cosHalfFovOuter;
    float cosHalfFovInner;
    int exponent;
    float rotationLimit;
    float translationLimit;
    char defName[64];
};

char __cdecl Com_CanPrimaryLightAffectPoint(const ComPrimaryLight *light, const float *point);
unsigned int Com_FindClosestPrimaryLight(const float *origin);
double __cdecl CosOfSumOfArcCos(float cos0, float cos1);
void __cdecl Com_UnloadWorld();


enum LumpType : __int32
{                                       // ...
    LUMP_MATERIALS = 0x0,
    LUMP_LIGHTBYTES = 0x1,
    LUMP_LIGHTGRIDENTRIES = 0x2,
    LUMP_LIGHTGRIDCOLORS = 0x3,
    LUMP_PLANES = 0x4,
    LUMP_BRUSHSIDES = 0x5,
    LUMP_BRUSHSIDEEDGECOUNTS = 0x6,
    LUMP_BRUSHEDGES = 0x7,
    LUMP_BRUSHES = 0x8,
    LUMP_TRIANGLES = 0x9,
    LUMP_DRAWVERTS = 0xA,
    LUMP_DRAWINDICES = 0xB,
    LUMP_CULLGROUPS = 0xC,
    LUMP_CULLGROUPINDICES = 0xD,
    LUMP_OBSOLETE_1 = 0xE,
    LUMP_OBSOLETE_2 = 0xF,
    LUMP_OBSOLETE_3 = 0x10,
    LUMP_OBSOLETE_4 = 0x11,
    LUMP_OBSOLETE_5 = 0x12,
    LUMP_PORTALVERTS = 0x13,
    LUMP_OBSOLETE_6 = 0x14,
    LUMP_OBSOLETE_7 = 0x15,
    LUMP_OBSOLETE_8 = 0x16,
    LUMP_OBSOLETE_9 = 0x17,
    LUMP_AABBTREES = 0x18,
    LUMP_CELLS = 0x19,
    LUMP_PORTALS = 0x1A,
    LUMP_NODES = 0x1B,
    LUMP_LEAFS = 0x1C,
    LUMP_LEAFBRUSHES = 0x1D,
    LUMP_LEAFSURFACES = 0x1E,
    LUMP_COLLISIONVERTS = 0x1F,
    LUMP_COLLISIONTRIS = 0x20,
    LUMP_COLLISIONEDGEWALKABLE = 0x21,
    LUMP_COLLISIONBORDERS = 0x22,
    LUMP_COLLISIONPARTITIONS = 0x23,
    LUMP_COLLISIONAABBS = 0x24,
    LUMP_MODELS = 0x25,
    LUMP_VISIBILITY = 0x26,
    LUMP_ENTITIES = 0x27,
    LUMP_PATHCONNECTIONS = 0x28,
    LUMP_REFLECTION_PROBES = 0x29,
    LUMP_VERTEX_LAYER_DATA = 0x2A,
    LUMP_PRIMARY_LIGHTS = 0x2B,
    LUMP_LIGHTGRIDHEADER = 0x2C,
    LUMP_LIGHTGRIDROWS = 0x2D,
    LUMP_OBSOLETE_10 = 0x2E,
    LUMP_UNLAYERED_TRIANGLES = 0x2F,
    LUMP_UNLAYERED_DRAWVERTS = 0x30,
    LUMP_UNLAYERED_DRAWINDICES = 0x31,
    LUMP_UNLAYERED_CULLGROUPS = 0x32,
    LUMP_UNLAYERED_AABBTREES = 0x33,
    LUMP_LIGHTREGIONS = 0x34,
    LUMP_LIGHTREGION_HULLS = 0x35,
    LUMP_LIGHTREGION_AXES = 0x36,
};

enum ComSaveLumpBehavior : __int32
{                                       // ...
    COM_SAVE_LUMP_AND_CLOSE = 0x0,
    COM_SAVE_LUMP_AND_REOPEN = 0x1,
};

struct BspChunk // sizeof=0x8
{                                       // ...
    LumpType type;                      // ...
    unsigned int length;                // ...
};
struct BspHeader // sizeof=0x32C
{                                       // ...
    unsigned int ident;                 // ...
    unsigned int version;               // ...
    unsigned int chunkCount;            // ...
    BspChunk chunks[100];               // ...
};
struct BspGlob // sizeof=0x54
{                                       // ...
    char name[64];                      // ...
    BspHeader *header;                  // ...
    unsigned int fileSize;              // ...
    unsigned int checksum;              // ...
    LumpType loadedLumpType;
    const void *loadedLumpData;         // ...
};

//#define $0368CFE3C958026DEB0A011CBC6EA813 BspGlob // sizeof=0x54

// com_bsp_load_obj
char *__cdecl Com_GetBspLump(LumpType type, unsigned int elemSize, unsigned int *count);
void __cdecl Com_LoadBsp(char *filename);
void __cdecl Com_UnloadBsp();
bool __cdecl Com_IsBspLoaded();
unsigned int __cdecl Com_GetBspLumpCountForVersion(int version);
void __cdecl Com_GetBspFilename(char *filename, unsigned int size, const char *mapname);
void __cdecl Com_CleanupBsp();
bool __cdecl Com_BspHasLump(LumpType type);
bool __cdecl Com_BspError();
char *__cdecl Com_ValidateBspLumpData(
    LumpType type,
    unsigned int offset,
    unsigned int length,
    unsigned int elemSize,
    unsigned int *count);
unsigned int __cdecl Com_GetBspVersion();

int __cdecl Com_BlockChecksumKey32(const unsigned __int8 *data, unsigned int length, unsigned int initialCrc);
char *__cdecl Com_EntityString(int *numEntityChars);

void __cdecl Com_LoadWorld(char *name);
void __cdecl Com_LoadWorld_FastFile(const char *name);
void __cdecl Com_ShutdownWorld();
void __cdecl Com_SaveLump(LumpType type, const void *newLump, unsigned int size, ComSaveLumpBehavior behavior);

extern ComWorld comWorld;

inline unsigned int Com_GetPrimaryLightCount()
{
    return comWorld.primaryLightCount;
}