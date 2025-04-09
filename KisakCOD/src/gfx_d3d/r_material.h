#pragma once

#include <d3d9.h>

#include <xanim/xasset.h>

#include "r_gfx.h"

enum surfaceType_t : __int32
{                                       // ...
    SF_TRIANGLES = 0x0,
    SF_TRIANGLES_PRETESS = 0x1,
    SF_BEGIN_STATICMODEL = 0x2,
    SF_STATICMODEL_RIGID = 0x2,
    SF_STATICMODEL_PRETESS = 0x3,
    SF_STATICMODEL_CACHED = 0x4,
    SF_STATICMODEL_SKINNED = 0x5,
    SF_END_STATICMODEL = 0x6,
    SF_BMODEL = 0x6,
    SF_BEGIN_XMODEL = 0x7,
    SF_XMODEL_RIGID = 0x7,
    SF_XMODEL_RIGID_SKINNED = 0x8,
    SF_XMODEL_SKINNED = 0x9,
    SF_END_XMODEL = 0xA,
    SF_BEGIN_FX = 0xA,
    SF_CODE_MESH = 0xA,
    SF_MARK_MESH = 0xB,
    SF_PARTICLE_CLOUD = 0xC,
    SF_END_FX = 0xD,
    SF_NUM_SURFACE_TYPES = 0xD,
    SF_FORCE_32_BITS = -0x1,
};

struct DObjSkelMat // sizeof=0x40
{                                       // ...
    float axis[3][4];
    float origin[4];
};

struct MaterialStreamRouting // sizeof=0x2
{                                       // ...
    unsigned __int8 source;             // ...
    unsigned __int8 dest;               // ...
};

struct MaterialVertexStreamRouting // sizeof=0x60
{                                       // ...
    MaterialStreamRouting data[16];
    IDirect3DVertexDeclaration9 *decl[16];
};

struct MaterialVertexDeclaration // sizeof=0x64
{                                       // ...
    unsigned __int8 streamCount;
    bool hasOptionalSource;
    bool isLoaded;
    // padding byte
    MaterialVertexStreamRouting routing;
};
struct MaterialVertexShaderProgram // sizeof=0xC
{                                       // ...
    IDirect3DVertexShader9 *vs;
    GfxVertexShaderLoadDef loadDef;
};
struct MaterialVertexShader // sizeof=0x10
{                                       // ...
    const char *name;
    MaterialVertexShaderProgram prog;
};

struct MaterialPixelShaderProgram // sizeof=0xC
{                                       // ...
    IDirect3DPixelShader9 *ps;
    GfxPixelShaderLoadDef loadDef;
};
struct MaterialPixelShader // sizeof=0x10
{                                       // ...
    const char *name;
    MaterialPixelShaderProgram prog;
};
struct MaterialArgumentCodeConst // sizeof=0x4
{                                       // ...
    unsigned __int16 index;
    unsigned __int8 firstRow;
    unsigned __int8 rowCount;
};
union MaterialArgumentDef // sizeof=0x4
{                                       // ...
    const float *literalConst;
    MaterialArgumentCodeConst codeConst;
    unsigned int codeSampler;
    unsigned int nameHash;
};
struct MaterialShaderArgument // sizeof=0x8
{                                       // ...
    unsigned __int16 type;
    unsigned __int16 dest;
    MaterialArgumentDef u;
};
struct MaterialPass // sizeof=0x14
{                                       // ...
    MaterialVertexDeclaration *vertexDecl; // ...
    MaterialVertexShader *vertexShader;
    MaterialPixelShader *pixelShader;
    unsigned __int8 perPrimArgCount;
    unsigned __int8 perObjArgCount;
    unsigned __int8 stableArgCount;
    unsigned __int8 customSamplerFlags;
    MaterialShaderArgument *args;
};

struct MaterialTechnique // sizeof=0x1C
{
    const char *name;
    unsigned __int16 flags;
    unsigned __int16 passCount;
    MaterialPass passArray[1];
};
struct WaterWritable // sizeof=0x4
{                                       // ...
    float floatTime;
};

struct complex_s // sizeof=0x8
{                                       // ...
    float real;                         // ...
    float imag;                         // ...
};
struct water_t // sizeof=0x44
{                                       // ...
    WaterWritable writable;
    complex_s *H0;
    float *wTerm;
    int M;                              // ...
    int N;                              // ...
    float Lx;                           // ...
    float Lz;                           // ...
    float gravity;                      // ...
    float windvel;                      // ...
    float winddir[2];                   // ...
    float amplitude;                    // ...
    float codeConstant[4];
    GfxImage *image;                    // ...
};

union MaterialTextureDefInfo // sizeof=0x4
{                                       // ...
    GfxImage *image;
    water_t *water;
};

struct MaterialTextureDef // sizeof=0xC
{
    unsigned int nameHash;
    char nameStart;
    char nameEnd;
    unsigned __int8 samplerState;
    unsigned __int8 semantic;
    MaterialTextureDefInfo u;
};

struct MaterialConstantDef // sizeof=0x20
{
    unsigned int nameHash;
    char name[12];
    float literal[4];
};

struct __declspec(align(4)) MaterialInfo // sizeof=0x18
{                                       // ...
    const char *name;                   // ...
    unsigned __int8 gameFlags;
    unsigned __int8 sortKey;
    unsigned __int8 textureAtlasRowCount; // ...
    unsigned __int8 textureAtlasColumnCount; // ...
    GfxDrawSurf drawSurf;
    unsigned int surfaceTypeBits;
    unsigned __int16 hashIndex;
    // padding byte
    // padding byte
};

struct MaterialTechniqueSet // sizeof=0x94
{                                       // ...
    const char *name;
    unsigned __int8 worldVertFormat;
    bool hasBeenUploaded;
    unsigned __int8 unused[1];
    // padding byte
    MaterialTechniqueSet *remappedTechniqueSet;
    MaterialTechnique *techniques[34];
};

struct Material // sizeof=0x50
{                                       // ...
    MaterialInfo info;
    unsigned __int8 stateBitsEntry[34];
    unsigned __int8 textureCount;
    unsigned __int8 constantCount;
    unsigned __int8 stateBitsCount;
    unsigned __int8 stateFlags;
    unsigned __int8 cameraRegion;
    // padding byte
    MaterialTechniqueSet *techniqueSet;
    MaterialTextureDef *textureTable;
    MaterialConstantDef *constantTable;
    GfxStateBits *stateBitsTable;
};

struct MaterialMemory // sizeof=0x8
{                                       // ...
    Material* material;
    int memory;
};

struct stream_source_info_t // sizeof=0x3
{                                       // ...
    unsigned __int8 Stream;
    unsigned __int8 Offset;
    unsigned __int8 Type;
};
struct stream_dest_info_t // sizeof=0x2
{                                       // ...
    unsigned __int8 Usage;
    unsigned __int8 UsageIndex;
};

enum MtlTechSetNotFoundBehavior : __int32
{                                       // ...
    MTL_TECHSET_NOT_FOUND_RETURN_NULL = 0x0,
    MTL_TECHSET_NOT_FOUND_RETURN_DEFAULT = 0x1,
};

struct BuiltInMaterialTable // sizeof=0x8
{                                       // ...
    const char *name;
    Material **material;
};

struct MaterialGlobals
{
    int techniqueSetCount;
    MaterialTechniqueSet *techniqueSetHashTable[1024]; // ...
};


const char* __cdecl Material_GetName(Material* handle);
const Material* __cdecl Material_FromHandle(Material* handle);

void __cdecl Material_Init();
void __cdecl Material_LoadBuiltIn(const BuiltInMaterialTable* mtlTable, int mtlTableCount);
void __cdecl Material_ForEachTechniqueSet_FastFile(void(__cdecl* callback)(MaterialTechniqueSet*));
void __cdecl Material_CollateTechniqueSets(XAssetHeader header, XAssetHeader* userData);
bool __cdecl IsValidMaterialHandle(Material* const handle);

void __cdecl TRACK_r_material();

unsigned __int8 *__cdecl Material_Alloc(unsigned int size);
void __cdecl Load_CreateMaterialPixelShader(GfxPixelShaderLoadDef *loadDef, MaterialPixelShader *mtlShader);
void __cdecl Load_CreateMaterialVertexShader(GfxVertexShaderLoadDef *loadDef, MaterialVertexShader *mtlShader);
void __cdecl AssertValidVertexDeclOffsets(const stream_source_info_t *streamTable);
void __cdecl Load_BuildVertexDecl(MaterialVertexDeclaration **mtlVertDecl);
IDirect3DVertexDeclaration9 *__cdecl Material_BuildVertexDecl(
    const MaterialStreamRouting *routingData,
    int streamCount,
    const stream_source_info_t *sourceTable);
MaterialTechniqueSet *__cdecl Material_FindTechniqueSet(const char *name, MtlTechSetNotFoundBehavior notFoundBehavior);
MaterialTechniqueSet *__cdecl Material_FindTechniqueSet_FastFile(
    const char *name,
    MtlTechSetNotFoundBehavior notFoundBehavior);
void __cdecl Material_DirtySort();
bool __cdecl Material_IsDefault(const Material *material);
Material *__cdecl Material_Register_FastFile(const char *name);
Material *__cdecl Material_Register(char *name, int imageTrack);
Material *__cdecl Material_RegisterHandle(const char *name, int imageTrack);

void __cdecl R_MaterialList_f();
void __cdecl R_GetMaterialList(XAssetHeader header, char *data);
int __cdecl R_GetMaterialMemory(Material *material);
bool __cdecl R_MaterialCompare(const MaterialMemory *material0, const MaterialMemory *material1);

void __cdecl Material_PreventOverrideTechniqueGeneration();

void __cdecl Material_UpdatePicmipAll();
void __cdecl R_Cmd_ReloadMaterialTextures();

extern MaterialGlobals materialGlobals;

// r_material_load_obj
MaterialTechniqueSet *__cdecl Material_FindTechniqueSet_LoadObj(
    const char *name,
    MtlTechSetNotFoundBehavior notFoundBehavior);

void __cdecl Material_GetInfo(Material *handle, MaterialInfo *matInfo);

Material *__cdecl Material_Duplicate(Material *mtlCopy, char *name);