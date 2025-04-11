#pragma once

#include <qcommon/qcommon.h>

#include "r_debug.h"
#include "r_font.h"
#include "r_gfx.h"
#include "r_material.h"

enum GfxRenderCommand : __int32
{                                       // ...
    RC_END_OF_LIST = 0x0,
    RC_SET_MATERIAL_COLOR = 0x1,
    RC_SAVE_SCREEN = 0x2,
    RC_SAVE_SCREEN_SECTION = 0x3,
    RC_CLEAR_SCREEN = 0x4,
    RC_SET_VIEWPORT = 0x5,
    RC_FIRST_NONCRITICAL = 0x6,
    RC_STRETCH_PIC = 0x6,
    RC_STRETCH_PIC_FLIP_ST = 0x7,
    RC_STRETCH_PIC_ROTATE_XY = 0x8,
    RC_STRETCH_PIC_ROTATE_ST = 0x9,
    RC_STRETCH_RAW = 0xA,
    RC_DRAW_QUAD_PIC = 0xB,
    RC_DRAW_FULL_SCREEN_COLORED_QUAD = 0xC,
    RC_DRAW_TEXT_2D = 0xD,
    RC_DRAW_TEXT_3D = 0xE,
    RC_BLEND_SAVED_SCREEN_BLURRED = 0xF,
    RC_BLEND_SAVED_SCREEN_FLASHED = 0x10,
    RC_DRAW_POINTS = 0x11,
    RC_DRAW_LINES = 0x12,
    RC_DRAW_TRIANGLES = 0x13,
    RC_DRAW_PROFILE = 0x14,
    RC_PROJECTION_SET = 0x15,
    RC_COUNT = 0x16,
};

enum GfxRenderTargetId : __int32
{                                       // ...
    R_RENDERTARGET_SAVED_SCREEN = 0x0,
    R_RENDERTARGET_FRAME_BUFFER = 0x1,
    R_RENDERTARGET_SCENE = 0x2,
    R_RENDERTARGET_RESOLVED_POST_SUN = 0x3,
    R_RENDERTARGET_RESOLVED_SCENE = 0x4,
    R_RENDERTARGET_FLOAT_Z = 0x5,
    R_RENDERTARGET_DYNAMICSHADOWS = 0x6,
    R_RENDERTARGET_PINGPONG_0 = 0x7,
    R_RENDERTARGET_PINGPONG_1 = 0x8,
    R_RENDERTARGET_SHADOWCOOKIE = 0x9,
    R_RENDERTARGET_SHADOWCOOKIE_BLUR = 0xA,
    R_RENDERTARGET_POST_EFFECT_0 = 0xB,
    R_RENDERTARGET_POST_EFFECT_1 = 0xC,
    R_RENDERTARGET_SHADOWMAP_SUN = 0xD,
    R_RENDERTARGET_SHADOWMAP_SPOT = 0xE,
    R_RENDERTARGET_COUNT = 0xF,
    R_RENDERTARGET_NONE = 0x10,
};

enum ShadowType : __int32
{                                       // ...
    SHADOW_NONE = 0x0,
    SHADOW_COOKIE = 0x1,
    SHADOW_MAP = 0x2,
};

enum GfxProjectionTypes : __int32
{                                       // ...
    GFX_PROJECTION_2D = 0x0,
    GFX_PROJECTION_3D = 0x1,
};

enum MaterialTechniqueType : __int32
{                                       // ...
    TECHNIQUE_DEPTH_PREPASS = 0x0,
    TECHNIQUE_BUILD_FLOAT_Z = 0x1,
    TECHNIQUE_BUILD_SHADOWMAP_DEPTH = 0x2,
    TECHNIQUE_BUILD_SHADOWMAP_COLOR = 0x3,
    TECHNIQUE_UNLIT = 0x4,
    TECHNIQUE_EMISSIVE = 0x5,
    TECHNIQUE_EMISSIVE_SHADOW = 0x6,
    TECHNIQUE_LIT_BEGIN = 0x7,
    TECHNIQUE_LIT = 0x7,
    TECHNIQUE_LIT_SUN = 0x8,
    TECHNIQUE_LIT_SUN_SHADOW = 0x9,
    TECHNIQUE_LIT_SPOT = 0xA,
    TECHNIQUE_LIT_SPOT_SHADOW = 0xB,
    TECHNIQUE_LIT_OMNI = 0xC,
    TECHNIQUE_LIT_OMNI_SHADOW = 0xD,
    TECHNIQUE_LIT_INSTANCED = 0xE,
    TECHNIQUE_LIT_INSTANCED_SUN = 0xF,
    TECHNIQUE_LIT_INSTANCED_SUN_SHADOW = 0x10,
    TECHNIQUE_LIT_INSTANCED_SPOT = 0x11,
    TECHNIQUE_LIT_INSTANCED_SPOT_SHADOW = 0x12,
    TECHNIQUE_LIT_INSTANCED_OMNI = 0x13,
    TECHNIQUE_LIT_INSTANCED_OMNI_SHADOW = 0x14,
    TECHNIQUE_LIT_END = 0x15,
    TECHNIQUE_LIGHT_SPOT = 0x15,
    TECHNIQUE_LIGHT_OMNI = 0x16,
    TECHNIQUE_LIGHT_SPOT_SHADOW = 0x17,
    TECHNIQUE_FAKELIGHT_NORMAL = 0x18,
    TECHNIQUE_FAKELIGHT_VIEW = 0x19,
    TECHNIQUE_SUNLIGHT_PREVIEW = 0x1A,
    TECHNIQUE_CASE_TEXTURE = 0x1B,
    TECHNIQUE_WIREFRAME_SOLID = 0x1C,
    TECHNIQUE_WIREFRAME_SHADED = 0x1D,
    TECHNIQUE_SHADOWCOOKIE_CASTER = 0x1E,
    TECHNIQUE_SHADOWCOOKIE_RECEIVER = 0x1F,
    TECHNIQUE_DEBUG_BUMPMAP = 0x20,
    TECHNIQUE_DEBUG_BUMPMAP_INSTANCED = 0x21,
    TECHNIQUE_COUNT = 0x22,
    TECHNIQUE_TOTAL_COUNT = 0x23,
    TECHNIQUE_NONE = 0x24,
};

enum FullscreenType : __int32
{                                       // ...
    FULLSCREEN_DISPLAY = 0x0,
    FULLSCREEN_MIXED = 0x1,
    FULLSCREEN_SCENE = 0x2,
};

struct GfxRenderTargetSurface // sizeof=0x8
{                                       // ...
    IDirect3DSurface9 *color;           // ...
    IDirect3DSurface9 *depthStencil;    // ...
};

struct GfxRenderTarget // sizeof=0x14
{                                       // ...
    GfxImage *image;                    // ...
    GfxRenderTargetSurface surface;     // ...
    unsigned int width;                 // ...
    unsigned int height;                // ...
};

struct StateBitsTable // sizeof=0x8
{                                       // ...
    int stateBits;                      // ...
    const char *name;                   // ...
};

struct GfxCmdHeader // sizeof=0x4
{                                       // ...
    unsigned __int16 id;
    unsigned __int16 byteCount;
};

struct GfxCmdArray // sizeof=0x10
{                                       // ...
    unsigned __int8 *cmds;              // ...
    int usedTotal;
    int usedCritical;
    GfxCmdHeader *lastCmd;
};

struct GfxRenderCommandExecState // sizeof=0x4
{                                       // ...
    const void *cmd;                    // ...
};
struct __declspec(align(2)) GfxCmdDrawText2D // sizeof=0x54
{
    GfxCmdHeader header;
    float x;
    float y;
    float rotation;
    Font_s *font;
    float xScale;
    float yScale;
    GfxColor color;
    int maxChars;
    int renderFlags;
    int cursorPos;
    char cursorLetter;
    // padding byte
    // padding byte
    // padding byte
    GfxColor glowForceColor;
    int fxBirthTime;
    int fxLetterTime;
    int fxDecayStartTime;
    int fxDecayDuration;
    const Material *fxMaterial;
    const Material *fxMaterialGlow;
    float padding;
    char text[3];
    // padding byte
};

struct FxCodeMeshData // sizeof=0x10
{                                       // ...
    unsigned int triCount;
    unsigned __int16 *indices;
    unsigned __int16 argOffset;
    unsigned __int16 argCount;
    unsigned int pad;
};

struct GfxParticleCloud // sizeof=0x40
{                                       // ...
    GfxScaledPlacement placement;
    float endpos[3];
    GfxColor color;
    float radius[2];
    unsigned int pad[2];
};
union PackedLightingCoords // sizeof=0x4
{                                       // ...
    unsigned int packed;
    unsigned __int8 array[4];
};
struct GfxSModelCachedVertex // sizeof=0x20
{                                       // ...
    float xyz[3];
    GfxColor color;
    PackedTexCoords texCoord;
    PackedUnitVec normal;
    PackedUnitVec tangent;
    PackedLightingCoords baseLighting;
};
struct GfxModelLightingPatch // sizeof=0x28
{                                       // ...
    unsigned __int16 modelLightingIndex;
    unsigned __int8 primaryLightWeight;
    unsigned __int8 colorsCount;
    unsigned __int8 groundLighting[4];
    unsigned __int16 colorsWeight[8];
    unsigned __int16 colorsIndex[8];
};
struct GfxBackEndPrimitiveData // sizeof=0x4
{                                       // ...
    int hasSunDirChanged;
};
struct FxMarkMeshData // sizeof=0x10
{                                       // ...
    unsigned int triCount;
    unsigned __int16 *indices;
    unsigned __int16 modelIndex;
    unsigned __int8 modelTypeAndSurf;
    unsigned __int8 pad0;
    unsigned int pad1;
};

struct GfxDrawSurfListInfo // sizeof=0x28
{                                       // ...
    const GfxDrawSurf *drawSurfs;
    unsigned int drawSurfCount;
    MaterialTechniqueType baseTechType; // ...
    const struct GfxViewInfo *viewInfo;
    float viewOrigin[4];
    const GfxLight *light;
    int cameraView;
};
struct PointLightPartition // sizeof=0x68
{                                       // ...
    GfxLight light;
    GfxDrawSurfListInfo info;
};
struct __declspec(align(16)) ShadowCookie // sizeof=0xC0
{                                       // ...
    GfxMatrix shadowLookupMatrix;
    float boxMin[3];
    float boxMax[3];
    GfxViewParms *shadowViewParms;
    float fade;
    unsigned int sceneEntIndex;
    GfxDrawSurfListInfo casterInfo;
    GfxDrawSurfListInfo receiverInfo;
    // padding byte
    // padding byte
    // padding byte
    // padding byte
    // padding byte
    // padding byte
    // padding byte
    // padding byte
    // padding byte
    // padding byte
    // padding byte
    // padding byte
};
struct __declspec(align(16)) ShadowCookieList // sizeof=0x1210
{                                       // ...
    ShadowCookie cookies[24];
    unsigned int cookieCount;
    // padding byte
    // padding byte
    // padding byte
    // padding byte
    // padding byte
    // padding byte
    // padding byte
    // padding byte
    // padding byte
    // padding byte
    // padding byte
    // padding byte
};
struct ShadowCookieCmd // sizeof=0x10
{                                       // ...
    const GfxViewParms *viewParmsDpvs;
    const GfxViewParms *viewParmsDraw;
    ShadowCookieList *shadowCookieList;
    int localClientNum;
};
struct GfxSunShadowProjection // sizeof=0x60
{                                       // ...
    float viewMatrix[4][4];
    float switchPartition[4];
    float shadowmapScale[4];
};
struct GfxSunShadowBoundingPoly // sizeof=0x78
{                                       // ...
    float snapDelta[2];
    int pointCount;
    float points[9][2];
    int pointIsNear[9];
};
struct __declspec(align(16)) GfxSunShadowPartition // sizeof=0x200
{                                       // ...
    GfxViewParms shadowViewParms;
    int partitionIndex;
    GfxViewport viewport;
    GfxDrawSurfListInfo info;
    GfxSunShadowBoundingPoly boundingPoly;
    // padding byte
    // padding byte
    // padding byte
    // padding byte
    // padding byte
    // padding byte
    // padding byte
    // padding byte
    // padding byte
    // padding byte
    // padding byte
    // padding byte
};
struct GfxSunShadow // sizeof=0x4A0
{                                       // ...
    GfxMatrix lookupMatrix;
    GfxSunShadowProjection sunProj;
    GfxSunShadowPartition partition[2];
};
struct __declspec(align(16)) GfxSpotShadow // sizeof=0x1F0
{                                       // ...
    GfxViewParms shadowViewParms;
    GfxMatrix lookupMatrix;
    unsigned __int8 shadowableLightIndex;
    unsigned __int8 pad[3];
    const GfxLight* light;
    float fade;
    GfxDrawSurfListInfo info;
    GfxViewport viewport;
    GfxImage* image;
    GfxRenderTargetId renderTargetId;
    float pixelAdjust[4];
    int clearScreen;
    GfxMeshData* clearMesh;
    // padding byte
    // padding byte
    // padding byte
    // padding byte
    // padding byte
    // padding byte
    // padding byte
    // padding byte
    // padding byte
    // padding byte
    // padding byte
    // padding byte
};

struct GfxBackEndData;

struct __declspec(align(8)) GfxCmdBufInput // sizeof=0x430
{                                       // ...
    float consts[58][4];
    const GfxImage* codeImages[27];     // ...
    unsigned __int8 codeImageSamplerStates[27]; // ...
    // padding byte
    const GfxBackEndData* data;         // ...
    // padding byte
    // padding byte
    // padding byte
    // padding byte
};

const struct GfxViewInfo // sizeof=0x67B0
{                                       // ...
    GfxViewParms viewParms;
    GfxSceneDef sceneDef;
    GfxViewport sceneViewport;
    GfxViewport displayViewport;
    GfxViewport scissorViewport;
    ShadowType dynamicShadowType;
    // padding byte
    // padding byte
    // padding byte
    // padding byte
    // padding byte
    // padding byte
    // padding byte
    // padding byte
    ShadowCookieList shadowCookieList;
    int localClientNum;
    int isRenderingFullScreen;
    bool needsFloatZ;
    // padding byte
    // padding byte
    // padding byte
    GfxLight shadowableLights[255];
    unsigned int shadowableLightCount;
    PointLightPartition pointLightPartitions[4];
    GfxMeshData pointLightMeshData[4];
    int pointLightCount;
    unsigned int emissiveSpotLightIndex;
    GfxLight emissiveSpotLight;
    int emissiveSpotDrawSurfCount;
    GfxDrawSurf *emissiveSpotDrawSurfs;
    unsigned int emissiveSpotLightCount;
    float blurRadius;
    float frustumPlanes[4][4];
    GfxDepthOfField dof;
    GfxFilm film;
    GfxGlow glow;
    const void *cmds;
    // padding byte
    // padding byte
    // padding byte
    // padding byte
    GfxSunShadow sunShadow;
    unsigned int spotShadowCount;
    // padding byte
    // padding byte
    // padding byte
    // padding byte
    // padding byte
    // padding byte
    // padding byte
    // padding byte
    // padding byte
    // padding byte
    // padding byte
    // padding byte
    GfxSpotShadow spotShadows[4];
    GfxQuadMeshData *fullSceneViewMesh;
    GfxDrawSurfListInfo litInfo;
    GfxDrawSurfListInfo decalInfo;
    GfxDrawSurfListInfo emissiveInfo;
    // padding byte
    // padding byte
    // padding byte
    // padding byte
    GfxCmdBufInput input;
};
const struct __declspec(align(16)) GfxBackEndData // sizeof=0x11E780
{                                       // ...
    unsigned __int8 surfsBuffer[131072];
    FxCodeMeshData codeMeshes[2048];
    unsigned int primDrawSurfsBuf[65536]; // ...
    GfxViewParms viewParms[28];
    unsigned __int8 primaryLightTechType[13][256];
    float codeMeshArgs[256][4];
    GfxParticleCloud clouds[256];
    GfxDrawSurf drawSurfs[32768];
    GfxMeshData codeMesh;
    GfxSModelCachedVertex smcPatchVerts[8192];
    unsigned __int16 smcPatchList[256];
    unsigned int smcPatchCount;
    unsigned int smcPatchVertsUsed;
    GfxModelLightingPatch modelLightingPatchList[4096];
    volatile int modelLightingPatchCount;
    GfxBackEndPrimitiveData prim;
    unsigned int shadowableLightHasShadowMap[8];
    unsigned int frameCount;
    int drawSurfCount;
    volatile long surfPos;
    volatile long gfxEntCount;
    GfxEntity gfxEnts[128];
    volatile long cloudCount;
    volatile long codeMeshCount;
    volatile long codeMeshArgsCount;
    volatile long markMeshCount;
    FxMarkMeshData markMeshes[1536];
    GfxMeshData markMesh;
    GfxVertexBufferState *skinnedCacheVb;
    IDirect3DQuery9 *endFence;
    unsigned __int8 *tempSkinBuf;
    volatile long tempSkinPos;
    IDirect3DIndexBuffer9 *preTessIb;
    int viewParmCount;
    GfxFog fogSettings;
    GfxCmdArray *commands;              // ...
    unsigned int viewInfoIndex;
    unsigned int viewInfoCount;
    GfxViewInfo *viewInfo;
    const void *cmds;
    GfxLight sunLight;
    int hasApproxSunDirChanged;
    volatile unsigned int primDrawSurfPos;
    unsigned int *staticModelLit;
    DebugGlobals debugGlobals;
    unsigned int drawType;
    // padding byte
    // padding byte
    // padding byte
    // padding byte
    // padding byte
    // padding byte
    // padding byte
    // padding byte
    // padding byte
    // padding byte
    // padding byte
    // padding byte
};

void __cdecl TRACK_r_rendercmds();
void __cdecl R_InitRenderCommands();
void __cdecl R_InitRenderBuffers();
void __cdecl R_InitDynamicMesh(
    GfxMeshData *mesh,
    unsigned int indexCount,
    unsigned int vertCount,
    unsigned int vertSize);
void __cdecl R_InitRenderThread();
void __cdecl R_SyncRenderThread();
GfxCmdArray *R_ClearCmdList();
void __cdecl R_ReleaseThreadOwnership();
void __cdecl R_IssueRenderCommands(unsigned int type);
void R_PerformanceCounters();
bool R_UpdateSkinCacheUsage();
char __cdecl R_HandOffToBackend(char type);
void __cdecl R_ToggleSmpFrameCmd(char type);
void __cdecl R_AbortRenderCommands();
void __cdecl R_BeginClientCmdList2D();
void __cdecl R_ClearClientCmdList2D();
void __cdecl R_BeginSharedCmdList();
void __cdecl R_AddCmdEndOfList();
GfxCmdHeader *__cdecl R_GetCommandBuffer(GfxRenderCommand renderCmd, int bytes);
DebugGlobals *R_ToggleSmpFrame();
GfxViewParms *__cdecl R_AllocViewParms();
void __cdecl R_AddCmdDrawStretchPic(
    float x,
    float y,
    float w,
    float h,
    float s0,
    float t0,
    float s1,
    float t1,
    const float *color,
    Material *material);
bool __cdecl Material_HasAnyFogableTechnique(const Material *material);
const MaterialTechnique *__cdecl Material_GetTechnique(const Material *material, MaterialTechniqueType techType);
MaterialTechniqueSet *__cdecl Material_GetTechniqueSet(const Material *material);
void __cdecl R_AddCmdDrawStretchPicFlipST(
    float x,
    float y,
    float w,
    float h,
    float s0,
    float t0,
    float s1,
    float t1,
    const float *color,
    Material *material);
void __cdecl R_AddCmdDrawStretchPicRotateXY(
    float x,
    float y,
    float w,
    float h,
    float s0,
    float t0,
    float s1,
    float t1,
    float angle,
    const float *color,
    Material *material);
void __cdecl R_AddCmdDrawStretchPicRotateST(
    float x,
    float y,
    float w,
    float h,
    float centerS,
    float centerT,
    float radiusST,
    float scaleFinalS,
    float scaleFinalT,
    float angle,
    const float *color,
    Material *material);
void __cdecl R_AddCmdDrawTextWithCursor(
    char *text,
    int maxChars,
    Font_s *font,
    float x,
    float y,
    float xScale,
    float yScale,
    float rotation,
    const float *color,
    int style,
    int cursorPos,
    char cursor);
GfxCmdDrawText2D *__cdecl AddBaseDrawTextCmd(
    char *text,
    int maxChars,
    Font_s *font,
    float x,
    float y,
    float xScale,
    float yScale,
    float rotation,
    const float *color,
    int style,
    int cursorPos,
    char cursor);
void __cdecl R_AddCmdDrawText(
    char *text,
    int maxChars,
    Font_s *font,
    float x,
    float y,
    float xScale,
    float yScale,
    float rotation,
    const float *color,
    int style);
void __cdecl R_AddCmdDrawTextSubtitle(
    char *text,
    int maxChars,
    Font_s *font,
    float x,
    float y,
    float xScale,
    float yScale,
    float rotation,
    const float *color,
    int style,
    const float *glowColor,
    bool cinematic);
char __cdecl SetDrawText2DGlowParms(GfxCmdDrawText2D *cmd, const float *color, const float *glowColor);
void __cdecl R_AddCmdDrawTextWithEffects(
    char *text,
    int maxChars,
    Font_s *font,
    float x,
    float y,
    float xScale,
    float yScale,
    float rotation,
    const float *color,
    int style,
    const float *glowColor,
    Material *fxMaterial,
    Material *fxMaterialGlow,
    int fxBirthTime,
    int fxLetterTime,
    int fxDecayStartTime,
    int fxDecayDuration);
char __cdecl SetDrawText2DPulseFXParms(
    GfxCmdDrawText2D *cmd,
    Material *fxMaterial,
    Material *fxMaterialGlow,
    int fxBirthTime,
    int fxLetterTime,
    int fxDecayStartTime,
    int fxDecayDuration);
void __cdecl R_AddCmdDrawConsoleText(
    char *textPool,
    int poolSize,
    int firstChar,
    int charCount,
    Font_s *font,
    float x,
    float y,
    float xScale,
    float yScale,
    const float *color,
    int style);
GfxCmdDrawText2D *__cdecl AddBaseDrawConsoleTextCmd(
    char *textPool,
    int poolSize,
    int firstChar,
    int charCount,
    Font_s *font,
    float x,
    float y,
    float xScale,
    float yScale,
    const float *color,
    int style);
void __cdecl CopyPoolTextToCmd(char *textPool, int poolSize, int firstChar, int charCount, GfxCmdDrawText2D *cmd);
void __cdecl R_AddCmdDrawConsoleTextSubtitle(
    char *textPool,
    int poolSize,
    int firstChar,
    int charCount,
    Font_s *font,
    float x,
    float y,
    float xScale,
    float yScale,
    const float *color,
    int style,
    const float *glowColor);
void __cdecl R_AddCmdDrawConsoleTextPulseFX(
    char *textPool,
    int poolSize,
    int firstChar,
    int charCount,
    Font_s *font,
    float x,
    float y,
    float xScale,
    float yScale,
    const float *color,
    int style,
    const float *glowColor,
    int fxBirthTime,
    int fxLetterTime,
    int fxDecayStartTime,
    int fxDecayDuration,
    Material *fxMaterial,
    Material *fxMaterialGlow);
void __cdecl R_AddCmdDrawQuadPic(const float (*verts)[2], const float *color, Material *material);
void __cdecl R_BeginFrame();
const dvar_s *R_UpdateFrontEndDvarOptions();
void __cdecl R_SetInputCodeConstantFromVec4(GfxCmdBufInput *input, unsigned int constant, float *value);
void __cdecl R_SetInputCodeImageTexture(GfxCmdBufInput *input, unsigned int codeTexture, const GfxImage *image);
bool __cdecl R_LightTweaksModified();
void R_SetTestLods();
bool __cdecl R_AreAnyImageOverridesActive();
void R_SetOutdoorFeatherConst();
void __cdecl R_SetInputCodeConstant(GfxCmdBufInput *input, unsigned int constant, float x, float y, float z, float w);
void R_EnvMapOverrideConstants();
void __cdecl R_EndFrame();
void __cdecl R_AddCmdClearScreen(int whichToClear, const float *color, float depth, unsigned __int8 stencil);
void __cdecl R_AddCmdSaveScreen(unsigned int screenTimerId);
void __cdecl R_AddCmdSaveScreenSection(
    float viewX,
    float viewY,
    float viewWidth,
    float viewHeight,
    unsigned int screenTimerId);
void __cdecl R_AddCmdBlendSavedScreenShockBlurred(
    int fadeMsec,
    float viewX,
    float viewY,
    float viewWidth,
    float viewHeight,
    unsigned int screenTimerId);
void __cdecl R_AddCmdBlendSavedScreenShockFlashed(
    float intensityWhiteout,
    float intensityScreengrab,
    float viewX,
    float viewY,
    float viewWidth,
    float viewHeight);
void __cdecl R_AddCmdDrawProfile();
void __cdecl R_AddCmdProjectionSet2D();
void __cdecl R_AddCmdProjectionSet(GfxProjectionTypes projection);
void __cdecl R_BeginRemoteScreenUpdate();
void __cdecl R_EndRemoteScreenUpdate();
void __cdecl R_PushRemoteScreenUpdate(int remoteScreenUpdateNesting);
int __cdecl R_PopRemoteScreenUpdate();
bool __cdecl R_IsInRemoteScreenUpdate();

void __cdecl R_InitTempSkinBuf();

extern GfxBackEndData *frontEndDataOut;