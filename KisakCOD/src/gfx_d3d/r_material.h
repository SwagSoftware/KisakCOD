#pragma once

#include <d3d9.h>

#include "r_gfx.h"

enum MaterialTextureSource : __int32
{                                       // ...
    TEXTURE_SRC_CODE_BLACK = 0x0, // ...
    TEXTURE_SRC_CODE_WHITE = 0x1, // ...
    TEXTURE_SRC_CODE_IDENTITY_NORMAL_MAP = 0x2, // ...
    TEXTURE_SRC_CODE_MODEL_LIGHTING = 0x3, // ...
    TEXTURE_SRC_CODE_LIGHTMAP_PRIMARY = 0x4, // ...
    TEXTURE_SRC_CODE_LIGHTMAP_SECONDARY = 0x5, // ...
    TEXTURE_SRC_CODE_SHADOWCOOKIE = 0x6, // ...
    TEXTURE_SRC_CODE_SHADOWMAP_SUN = 0x7, // ...
    TEXTURE_SRC_CODE_SHADOWMAP_SPOT = 0x8, // ...
    TEXTURE_SRC_CODE_FEEDBACK = 0x9, // ...
    TEXTURE_SRC_CODE_RESOLVED_POST_SUN = 0xA, // ...
    TEXTURE_SRC_CODE_RESOLVED_SCENE = 0xB, // ...
    TEXTURE_SRC_CODE_POST_EFFECT_0 = 0xC, // ...
    TEXTURE_SRC_CODE_POST_EFFECT_1 = 0xD, // ...
    TEXTURE_SRC_CODE_SKY = 0xE, // ...
    TEXTURE_SRC_CODE_LIGHT_ATTENUATION = 0xF, // ...
    TEXTURE_SRC_CODE_DYNAMIC_SHADOWS = 0x10, // ...
    TEXTURE_SRC_CODE_OUTDOOR = 0x11, // ...
    TEXTURE_SRC_CODE_FLOATZ = 0x12, // ...
    TEXTURE_SRC_CODE_PROCESSED_FLOATZ = 0x13, // ...
    TEXTURE_SRC_CODE_RAW_FLOATZ = 0x14, // ...
    TEXTURE_SRC_CODE_CASE_TEXTURE = 0x15,
    TEXTURE_SRC_CODE_CINEMATIC_Y = 0x16, // ...
    TEXTURE_SRC_CODE_CINEMATIC_CR = 0x17, // ...
    TEXTURE_SRC_CODE_CINEMATIC_CB = 0x18, // ...
    TEXTURE_SRC_CODE_CINEMATIC_A = 0x19, // ...
    TEXTURE_SRC_CODE_REFLECTION_PROBE = 0x1A, // ...
    TEXTURE_SRC_CODE_COUNT = 0x1B,
};

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

enum ShaderParamType : __int32
{                                       // ...
    SHADER_PARAM_FLOAT4 = 0x0,
    SHADER_PARAM_SAMPLER_2D = 0x1,
    SHADER_PARAM_SAMPLER_3D = 0x2,
    SHADER_PARAM_SAMPLER_CUBE = 0x3,
};

enum MaterialShaderType : __int32
{                                       // ...
    MTL_VERTEX_SHADER = 0x0,
    MTL_PIXEL_SHADER = 0x1,
};

enum MtlParseSuccess : __int32
{                                       // ...
    MTL_PARSE_SUCCESS = 0x0,
    MTL_PARSE_NO_MATCH = 0x1,
    MTL_PARSE_ERROR = 0x2,
};

enum MaterialWorldVertexFormat : __int32
{                                       // ...
    MTL_WORLDVERT_TEX_1_NRM_1 = 0x0,    // ...
    MTL_WORLDVERT_TEX_2_NRM_1 = 0x1,    // ...
    MTL_WORLDVERT_TEX_2_NRM_2 = 0x2,
    MTL_WORLDVERT_TEX_3_NRM_1 = 0x3,    // ...
    MTL_WORLDVERT_TEX_3_NRM_2 = 0x4,
    MTL_WORLDVERT_TEX_3_NRM_3 = 0x5,
    MTL_WORLDVERT_TEX_4_NRM_1 = 0x6,    // ...
    MTL_WORLDVERT_TEX_4_NRM_2 = 0x7,
    MTL_WORLDVERT_TEX_4_NRM_3 = 0x8,
    MTL_WORLDVERT_TEX_5_NRM_1 = 0x9,    // ...
    MTL_WORLDVERT_TEX_5_NRM_2 = 0xA,
    MTL_WORLDVERT_TEX_5_NRM_3 = 0xB,
};

enum MaterialUpdateFrequency : __int32
{                                       // ...
    MTL_UPDATE_PER_PRIM = 0x0,
    MTL_UPDATE_PER_OBJECT = 0x1,        // ...
    MTL_UPDATE_RARELY = 0x2,        // ...
    MTL_UPDATE_CUSTOM = 0x3,        // ...
};

struct MaterialWaterDef // sizeof=0x20
{
    int textureWidth;
    float horizontalWorldLength;
    float verticalWorldLength;
    float amplitude;
    float windSpeed;
    float windDirection[2];
    struct water_t *map;
};

union MaterialTextureDefRaw_u // sizeof=0x4
{                                       // ...
    unsigned int imageNameOffset;
    unsigned int waterDefOffset;
};
struct MaterialTextureDefRaw // sizeof=0xC
{
    unsigned int nameOffset;
    unsigned __int8 samplerState;
    unsigned __int8 semantic;
    // padding byte
    // padding byte
    MaterialTextureDefRaw_u u;
};

struct MaterialConstantDefRaw // sizeof=0x14
{
    unsigned int nameOffset;
    float literal[4];
};

struct LayeredTechniqueSetName // sizeof=0x10
{                                       // ...
    const char *inputName;              // ...
    const char *namePrefixRegister;
    const char *namePrefixGenerate;
    const char *nameChunk;
};

struct __declspec(align(4)) ShaderUniformDef // sizeof=0x10
{                                       // ...
    ShaderParamType type;
    const char *name;
    unsigned __int16 index;
    unsigned __int16 resourceDest;
    bool isTransposed;
    bool isAssigned;
    // padding byte
    // padding byte
};

struct __declspec(align(2)) ShaderVaryingDef // sizeof=0x8
{                                       // ...
    const char *name;
    unsigned __int8 streamDest;
    unsigned __int8 resourceDest;
    bool isAssigned;
    // padding byte
};

struct ShaderParameterSet // sizeof=0x38C
{                                       // ...
    ShaderUniformDef uniformInputs[32];
    ShaderVaryingDef varyingInputs[32]; // ...
    ShaderVaryingDef outputs[16];       // ...
    unsigned int uniformInputCount;
    unsigned int varyingInputCount;     // ...
    unsigned int outputCount;           // ...
};

struct MaterialTypeInfo // sizeof=0xC
{                                       // ...
    const char *prefix;                 // ...
    const char *techniqueSetPrefix;     // ...
    unsigned int prefixLen;             // ...
};

struct MaterialStreamRouting // sizeof=0x2
{                                       // ...
    unsigned __int8 source;             // ...
    unsigned __int8 dest;               // ...
};

struct MtlStateMapBitName // sizeof=0x8
{                                       // ...
    const char *name;
    int bits;
};

struct MtlStateMapBitGroup // sizeof=0x10
{                                       // ...
    const char *name;                   // ...
    const MtlStateMapBitName *bitNames; // ...
    int stateBitsMask[2];               // ...
};

struct $4ABF24606230B73E4E420CE33A1F14B1 // sizeof=0xC
{                                       // ...
    bool isRegisteringOverrides;        // ...
    bool isDirty;                       // ...
    // padding byte
    // padding byte
    unsigned int remapMask;             // ...
    unsigned int remapValue;            // ...
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
    MaterialVertexDeclaration *vertexDecl; // ... // 0
    MaterialVertexShader *vertexShader; // 4
    MaterialPixelShader *pixelShader; // 8
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

struct __declspec(align(4)) ShaderIndexRange // sizeof=0xC
{                                       // ...
    unsigned int first;                 // ...
    unsigned int count;                 // ...
    bool isImplicit;                    // ...
    // padding byte
    // padding byte
    // padding byte
};

union ShaderArgumentSource_u // sizeof=0x4
{                                       // ...
    const float *literalConst;
    unsigned __int16 codeIndex;
    const char *name;
};
struct ShaderArgumentSource // sizeof=0x14
{                                       // ...
    ShaderIndexRange indexRange;
    unsigned __int16 type;              // ...
    // padding byte
    // padding byte
    ShaderArgumentSource_u u; // ...
};
struct ShaderArgumentDest // sizeof=0x10
{                                       // ...
    ShaderIndexRange indexRange;        // ...
    const char *paramName;              // ...
};

struct CodeSamplerSource // sizeof=0x14
{                                       // ...
    const char *name;
    MaterialTextureSource source;
    const CodeSamplerSource *subtable;
    int arrayCount;
    int arrayStride;
};

struct CodeConstantSource // sizeof=0x14
{                                       // ...
    const char *name;
    unsigned __int8 source;
    // padding byte
    // padding byte
    // padding byte
    const CodeConstantSource *subtable;
    int arrayCount;
    int arrayStride;
};

struct GfxAssembledShaderTextFile // sizeof=0x108
{                                       // ...
    unsigned int srcLine;
    unsigned int destLine;
    char fileName[256];
};

struct GfxAssembledShaderText // sizeof=0x8418
{                                       // ...
    char *string;
    unsigned int used;
    unsigned int total;
    unsigned int currentDestLine;
    bool overflowed;
    // padding byte
    // padding byte
    // padding byte
    unsigned int fileCount;
    GfxAssembledShaderTextFile files[128];
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

struct GfxCachedShaderText // sizeof=0xC
{                                       // ...
    const char *name;                   // ...
    const char *text;                   // ...
    int textSize;                       // ...
};
struct MaterialString // sizeof=0x8
{                                       // ...
    const char *string;                 // ...
    unsigned int hash;                  // ...
};
struct MaterialStateMapRule // sizeof=0x20
{                                       // ...
    unsigned int stateBitsMask[2];
    unsigned int stateBitsValue[2];
    unsigned int stateBitsSet[2];
    unsigned int stateBitsClear[2];
};
struct MaterialStateMapRuleSet // sizeof=0x24
{
    int ruleCount;
    MaterialStateMapRule rules[1];
};
struct MaterialStateMap // sizeof=0x2C
{
    const char *name;
    MaterialStateMapRuleSet *ruleSet[10];
};
struct MaterialInfoRaw // sizeof=0x28
{                                       // ...
    unsigned int nameOffset;
    unsigned int refImageNameOffset;
    unsigned __int8 gameFlags;
    unsigned __int8 sortKey;
    unsigned __int8 textureAtlasRowCount;
    unsigned __int8 textureAtlasColumnCount;
    float maxDeformMove;
    unsigned __int8 deformFlags;
    unsigned __int8 usage;
    unsigned __int16 toolFlags;
    unsigned int locale;
    unsigned __int16 autoTexScaleWidth;
    unsigned __int16 autoTexScaleHeight;
    float tessSize;
    int surfaceFlags;
    int contents;
};
struct MaterialRaw // sizeof=0x40
{
    MaterialInfoRaw info;
    unsigned int refStateBits[2];
    unsigned __int16 textureCount;
    unsigned __int16 constantCount;
    unsigned int techSetNameOffset;
    unsigned int textureTableOffset;
    unsigned int constantTableOffset;
};
struct $8E67C8D28114E56A26FBAF05ACADB66A // sizeof=0x11028
{                                       // ...
    unsigned int cachedShaderCount;     // ...
    GfxCachedShaderText *cachedShaderText; // ...
    unsigned int vertexDeclCount;       // ...
    MaterialVertexDeclaration vertexDeclHashTable[32]; // ...
    unsigned int literalCount;          // ...
    float literalTable[16][4];          // ...
    unsigned int stringCount;           // ...
    MaterialString stringHashTable[64]; // ...
    unsigned int vertexShaderCount;     // ...
    MaterialVertexShader *vertexShaderHashTable[2][2048]; // ...
    unsigned int pixelShaderCount;      // ...
    MaterialPixelShader *pixelShaderHashTable[2][2048]; // ...
    unsigned int stateMapCount;         // ...
    MaterialStateMap *stateMapHashTable[32]; // ...
    unsigned int techniqueCount;        // ...
    MaterialTechnique *techniqueHashTable[2][4096]; // ...
    const MaterialRaw *sortMtlRaw;      // ...
};


const char* __cdecl Material_GetName(Material* handle);
const Material* __cdecl Material_FromHandle(Material* handle);

void __cdecl Material_ReloadAll();
void __cdecl Material_ReleaseAll();

void __cdecl Material_Init();
void __cdecl Material_Shutdown();
void __cdecl Material_LoadBuiltIn(const BuiltInMaterialTable* mtlTable, int mtlTableCount);
void __cdecl Material_ForEachTechniqueSet_FastFile(void(__cdecl* callback)(MaterialTechniqueSet*));
bool __cdecl IsValidMaterialHandle(Material* const handle);

//void __cdecl R_GetMaterialList(XAssetHeader header, char *data);
//void __cdecl Material_CollateTechniqueSets(XAssetHeader header, XAssetHeader *userData);
//void __cdecl Material_ReleaseTechniqueSet(XAssetHeader header, void* crap);

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

void __cdecl Material_GetHashIndex(const char *name, unsigned __int16 *hashIndex, bool *exists);

void __cdecl R_MaterialList_f();
int __cdecl R_GetMaterialMemory(Material *material);
bool __cdecl R_MaterialCompare(const MaterialMemory *material0, const MaterialMemory *material1);

void __cdecl Material_PreventOverrideTechniqueGeneration();

void __cdecl Material_UpdatePicmipAll();
void __cdecl R_Cmd_ReloadMaterialTextures();

struct __declspec(align(4)) GfxMtlFeatureMap // sizeof=0x10
{                                       // ...
    const char *name;
    unsigned int mask;
    unsigned int value;
    bool valueRequired;
    // padding byte
    // padding byte
    // padding byte
};

struct GfxShaderConstantBlock // sizeof=0x64
{                                       // ...
    unsigned int count;                 // ...
    unsigned __int16 dest[16];          // ...
    const float *value[16];             // ...
};

extern bool g_generateOverrideTechniques;
extern MaterialGlobals materialGlobals;
extern $4ABF24606230B73E4E420CE33A1F14B1 mtlOverrideGlob;

// r_material_load_obj
bool __cdecl Material_CachedShaderTextLess(const GfxCachedShaderText *cached0, const GfxCachedShaderText *cached1);
Material *__cdecl R_GetBspMaterial(unsigned int materialIndex);
void __cdecl Material_FreeAll();
void __cdecl Material_PreLoadAllShaderText();
Material *__cdecl Material_Load(char *assetName, int imageTrack);
MaterialTechniqueSet *__cdecl Material_FindTechniqueSet_LoadObj(
    const char *name,
    MtlTechSetNotFoundBehavior notFoundBehavior);

void __cdecl Material_GetInfo(Material *handle, MaterialInfo *matInfo);

Material *__cdecl Material_Duplicate(Material *mtlCopy, char *name);

void __cdecl Material_Sort();

char __cdecl Material_SetPassShaderArguments_DX(
    const char **text,
    const char *shaderName,
    MaterialShaderType shaderType,
    unsigned int *program,
    unsigned __int16 *techFlags,
    ShaderParameterSet *paramSet,
    unsigned int argLimit,
    unsigned int *argCount,
    MaterialShaderArgument *args);

const char *__cdecl Material_RegisterString(char *string);
const char *__cdecl Material_NameForStreamDest(unsigned __int8 dest);
MaterialTechniqueSet *__cdecl Material_RegisterTechniqueSet(const char *name);
void __cdecl Material_SetMaterialDrawRegion(Material *material);
char __cdecl Material_Validate(const Material *material);
void __cdecl Material_SetStateBits(Material *material, unsigned int (*stateBitsTable)[2], unsigned int stateBitsCount);
bool __cdecl Material_GenerateShaderString_r(
    GfxAssembledShaderText *prog,
    char *shaderName,
    const char *file,
    unsigned int fileSize,
    bool isInLibDir);

// r_material_override
void __cdecl Material_OverrideTechniqueSets();
void __cdecl Material_OriginalRemapTechniqueSet(MaterialTechniqueSet *techSet);
void __cdecl Material_DirtyTechniqueSetOverrides();
void __cdecl Material_ClearShaderUploadList();
bool __cdecl Material_WouldTechniqueSetBeOverridden(const MaterialTechniqueSet *techSet);