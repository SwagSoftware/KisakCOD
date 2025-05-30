#pragma once

#include <d3d9.h>
#include "r_material.h" // GfxVertex
#include "r_init.h"
#include "r_rendercmds.h"

enum MaterialVertexDeclType : __int32
{                                       // ...
    VERTDECL_GENERIC = 0x0,
    VERTDECL_PACKED = 0x1,
    VERTDECL_WORLD = 0x2,
    VERTDECL_WORLD_T1N0 = 0x3,
    VERTDECL_WORLD_T1N1 = 0x4,
    VERTDECL_WORLD_T2N0 = 0x5,
    VERTDECL_WORLD_T2N1 = 0x6,
    VERTDECL_WORLD_T2N2 = 0x7,
    VERTDECL_WORLD_T3N0 = 0x8,
    VERTDECL_WORLD_T3N1 = 0x9,
    VERTDECL_WORLD_T3N2 = 0xA,
    VERTDECL_WORLD_T4N0 = 0xB,
    VERTDECL_WORLD_T4N1 = 0xC,
    VERTDECL_WORLD_T4N2 = 0xD,
    VERTDECL_POS_TEX = 0xE,
    VERTDECL_STATICMODELCACHE = 0xF,
    VERTDECL_COUNT = 0x10,
};

 enum $C15A5588E915B6036BD955C44CEC191A : __int32
 {                                       // ...
     THREAD_CONTEXT_MAIN         = 0x0,
     THREAD_CONTEXT_BACKEND      = 0x1,
     THREAD_CONTEXT_WORKER0      = 0x2,
     THREAD_CONTEXT_WORKER1      = 0x3,
     THREAD_CONTEXT_TRACE_COUNT  = 0x4,
     THREAD_CONTEXT_TRACE_LAST   = 0x3,
     THREAD_CONTEXT_CINEMATIC    = 0x4,
     THREAD_CONTEXT_TITLE_SERVER = 0x5,
     THREAD_CONTEXT_DATABASE     = 0x6,
     THREAD_CONTEXT_COUNT        = 0x7,
 };

struct GfxCmdSetMaterialColor // sizeof=0x14
{
    GfxCmdHeader header;
    float color[4];
};

struct GfxCmdDrawLines // sizeof=0x28
{
    GfxCmdHeader header;
    __int16 lineCount;
    unsigned __int8 width;
    unsigned __int8 dimensions;
    GfxPointVertex verts[2];
};

struct GfxCmdBlendSavedScreenFlashed // sizeof=0x1C
{
    GfxCmdHeader header;
    float intensityWhiteout;
    float intensityScreengrab;
    float s0;
    float t0;
    float ds;
    float dt;
};

struct GfxCmdBlendSavedScreenBlurred // sizeof=0x1C
{
    GfxCmdHeader header;
    int fadeMsec;
    float s0;
    float t0;
    float ds;
    float dt;
    int screenTimerId;
};

struct GfxCmdDrawQuadPic // sizeof=0x2C
{
    GfxCmdHeader header;
    const Material *material;
    float verts[4][2];
    GfxColor color;
};

struct GfxCmdStretchPicRotateST // sizeof=0x34
{
    GfxCmdHeader header;
    const Material *material;
    float x;
    float y;
    float w;
    float h;
    float centerS;
    float centerT;
    float radiusST;
    float scaleFinalS;
    float scaleFinalT;
    GfxColor color;
    float rotation;
};

struct GfxCmdStretchPicRotateXY // sizeof=0x30
{
    GfxCmdHeader header;
    const Material *material;
    float x;
    float y;
    float w;
    float h;
    float s0;
    float t0;
    float s1;
    float t1;
    GfxColor color;
    float rotation;
};

struct __declspec(align(16)) GfxCmdBufSourceState // sizeof=0xF00
{                                       // ...
    GfxCodeMatrices matrices;
    GfxCmdBufInput input;               // ...
    GfxViewParms viewParms;             // ...
    GfxMatrix shadowLookupMatrix;
    unsigned __int16 constVersions[90];
    unsigned __int16 matrixVersions[8];
    float eyeOffset[4];                 // ...
    unsigned int shadowableLightForShadowLookupMatrix;
    const GfxScaledPlacement *objectPlacement;
    const GfxViewParms *viewParms3D;    // ...
    unsigned int depthHackFlags;
    GfxScaledPlacement skinnedPlacement;
    int cameraView;
    GfxViewMode viewMode;               // ...
    GfxSceneDef sceneDef;               // ...
    GfxViewport sceneViewport;          // ...
    float materialTime;
    GfxViewportBehavior viewportBehavior; // ...
    int renderTargetWidth;              // ...
    int renderTargetHeight;             // ...
    bool viewportIsDirty;               // ...
    // padding byte
    // padding byte
    // padding byte
    unsigned int shadowableLightIndex;
    // padding byte
    // padding byte
    // padding byte
    // padding byte
    // padding byte
    // padding byte
    // padding byte
    // padding byte
};

struct GfxCmdBufPrimState_stream // sizeof=0xC
{                                       // ...
    unsigned int stride;                // ...
    IDirect3DVertexBuffer9 *vb;         // ...
    unsigned int offset;                // ...
};
struct GfxCmdBufPrimState // sizeof=0x28
{                                       // ...
    IDirect3DDevice9 *device;           // ...
    IDirect3DIndexBuffer9 *indexBuffer; // ...
    MaterialVertexDeclType vertDeclType; // ...
    GfxCmdBufPrimState_stream streams[2]; // ...
    IDirect3DVertexDeclaration9 *vertexDecl; // ...
};
struct GfxCmdBufState // sizeof=0xA10
{                                       // ...
    unsigned __int8 refSamplerState[16];
    unsigned int samplerState[16];
    const GfxTexture *samplerTexture[16];
    GfxCmdBufPrimState prim;            // ...
    const Material *material;           // ...
    MaterialTechniqueType techType;     // ...
    const MaterialTechnique *technique; // ...
    const MaterialPass *pass;
    unsigned int passIndex;
    GfxDepthRangeType depthRangeType;
    float depthRangeNear;
    float depthRangeFar;
    unsigned __int64 vertexShaderConstState[32]; // ...
    unsigned __int64 pixelShaderConstState[256]; // ...
    unsigned __int8 alphaRef;           // ...
    // padding byte
    // padding byte
    // padding byte
    unsigned int refStateBits[2];
    unsigned int activeStateBits[2];    // ...
    const MaterialPixelShader *pixelShader; // ...
    const MaterialVertexShader *vertexShader; // ...
    GfxViewport viewport;
    GfxRenderTargetId renderTargetId;   // ...
    const Material *origMaterial;       // ...
    MaterialTechniqueType origTechType; // ...
};

struct GfxCmdBuf // sizeof=0x4
{                                       // ...
    IDirect3DDevice9 *device;
};

struct GfxCmdBufContext // sizeof=0x8
{                                       // ...
    GfxCmdBufSourceState *source;       // ...
    GfxCmdBufState *state;              // ...
};

struct GfxCmdDrawPoints // sizeof=0x18
{
    GfxCmdHeader header;
    __int16 pointCount;
    unsigned __int8 size;
    unsigned __int8 dimensions;
    GfxPointVertex verts[1];
};

struct GfxPrimStats // sizeof=0x18
{                                       // ...
    int primCount;                      // ...
    int triCount;                       // ...
    int staticIndexCount;               // ...
    int staticVertexCount;              // ...
    int dynamicIndexCount;              // ...
    int dynamicVertexCount;             // ...
};

struct GfxDrawPrimArgs // sizeof=0xC
{                                       // ...
    int vertexCount;                    // ...
    int triCount;                       // ...
    int baseIndex;                      // ...
};

struct GfxViewStats // sizeof=0x134
{                                       // ...
    GfxPrimStats primStats[10];         // ...
    int drawSurfCount;
    int drawPrimHistogram[16];          // ...
};

struct GfxFrameStats // sizeof=0x274
{                                       // ...
    GfxViewStats viewStats[2];          // ...
    int gfxEntCount;
    int geoIndexCount;                  // ...
    int fxIndexCount;                   // ...
};

struct GfxDrawSurfListArgs // sizeof=0x10
{                                       // ...
    GfxCmdBufContext context;           // ...
    unsigned int firstDrawSurfIndex;    // ...
    const GfxDrawSurfListInfo* info;    // ...
};

struct r_backEndGlobals_t // sizeof=0x280
{                                       // ...
    int glowCount;                      // ...
    GfxImage *glowImage;                // ...
    Font_s *debugFont;                  // ...
    GfxFrameStats frameStatsMax;        // ...
};

struct __declspec(align(8)) materialCommands_t // sizeof=0x22A960
{                                       // ...
    GfxVertex verts[5450];              // ...
    unsigned __int16 indices[1048576];  // ...
    MaterialVertexDeclType vertDeclType;
    unsigned int vertexSize;
    int indexCount;                     // ...
    int vertexCount;                    // ...
    int firstVertex;                    // ...
    int lastVertex;                     // ...
    bool finishedFilling;               // ...
    // padding byte
    // padding byte
    // padding byte
    // padding byte
    // padding byte
    // padding byte
    // padding byte
};




void __cdecl TRACK_rb_backend();
void __cdecl RB_CopyBackendStats();
void __cdecl RB_SetIdentity();
void __cdecl R_SetVertex2d(GfxVertex *vert, float x, float y, float s, float t, unsigned int color);
void __cdecl R_SetVertex4dWithNormal(
    GfxVertex *vert,
    float x,
    float y,
    float z,
    float w,
    float nx,
    float ny,
    float nz,
    float s,
    float t,
    const unsigned __int8 *color);
void __cdecl RB_DrawStretchPic(
    const Material *material,
    float x,
    float y,
    float w,
    float h,
    float s0,
    float t0,
    float s1,
    float t1,
    unsigned int color,
    GfxPrimStatsTarget statsTarget);
void __cdecl RB_CheckTessOverflow(int vertexCount, int indexCount);
void __cdecl RB_DrawStretchPicFlipST(
    const Material *material,
    float x,
    float y,
    float w,
    float h,
    float s0,
    float t0,
    float s1,
    float t1,
    unsigned int color,
    GfxPrimStatsTarget statsTarget);
void __cdecl RB_DrawFullScreenColoredQuad(
    const Material *material,
    float s0,
    float t0,
    float s1,
    float t1,
    unsigned int color);
void __cdecl RB_FullScreenColoredFilter(const Material *material, unsigned int color);
void __cdecl RB_FullScreenFilter(const Material *material);
void __cdecl RB_SplitScreenFilter(const Material *material, const GfxViewInfo *viewInfo);
void __cdecl RB_SplitScreenTexCoords(float x, float y, float w, float h, float *s0, float *t0, float *s1, float *t1);
void __cdecl R_Resolve(GfxCmdBufContext context, GfxImage *image);
void __cdecl RB_StretchPicCmd(GfxRenderCommandExecState *execState);
void __cdecl RB_StretchPicCmdFlipST(GfxRenderCommandExecState *execState);
void __cdecl RB_StretchPicRotateXYCmd(GfxRenderCommandExecState *execState);
void __cdecl RB_StretchPicRotateSTCmd(GfxRenderCommandExecState *execState);
void __cdecl RB_DrawQuadPicCmd(GfxRenderCommandExecState *execState);
void __cdecl RB_DrawFullScreenColoredQuadCmd(GfxRenderCommandExecState *execState);
void __cdecl RB_StretchRawCmd(GfxRenderCommandExecState *execState);
void __cdecl RB_StretchRaw(int x, int y, int w, int h, int cols, int rows, const unsigned __int8 *data);
void __cdecl R_DrawSurfs(GfxCmdBufContext context, GfxCmdBufState *prepassState, const GfxDrawSurfListInfo *info);
unsigned int __cdecl R_RenderDrawSurfListMaterial(const GfxDrawSurfListArgs *listArgs, GfxCmdBufContext prepassContext);
void __cdecl R_TessEnd(GfxCmdBufContext context, GfxCmdBufContext prepassContext);
void __cdecl RB_ClearScreenCmd(GfxRenderCommandExecState *execState);
void __cdecl RB_SetGammaRamp(const GfxGammaRamp *gammaTable);
void __cdecl RB_SaveScreenCmd(GfxRenderCommandExecState *execState);
void __cdecl RB_SaveScreenSectionCmd(GfxRenderCommandExecState *execState);
void __cdecl R_ResolveSection(GfxCmdBufContext context, GfxImage *image);
void __cdecl RB_BlendSavedScreenBlurredCmd(GfxRenderCommandExecState *execState);
void __cdecl R_SetCodeImageTexture(GfxCmdBufSourceState *source, unsigned int codeTexture, const GfxImage *image);
void __cdecl RB_BlendSavedScreenFlashedCmd(GfxRenderCommandExecState *execState);
void __cdecl RB_DrawPointsCmd(GfxRenderCommandExecState *execState);
void __cdecl RB_DrawPoints2D(const GfxCmdDrawPoints *cmd);
void __cdecl R_SetVertex4d(
    GfxVertex *vert,
    float x,
    float y,
    float z,
    float w,
    float s,
    float t,
    const unsigned __int8 *color);
void __cdecl RB_DrawPoints3D(const GfxCmdDrawPoints *cmd);
void __cdecl RB_DrawLines2D(int count, int width, const GfxPointVertex *verts);
void __cdecl R_SetVertex3d(GfxVertex *vert, float x, float y, float z, float s, float t, const unsigned __int8 *color);
void __cdecl RB_DrawLines3D(int count, int width, const GfxPointVertex *verts, bool depthTest);
void __cdecl RB_DrawLinesCmd(GfxRenderCommandExecState *execState);
void __cdecl RB_DrawTrianglesCmd(GfxRenderCommandExecState *execState);
void __cdecl RB_DrawTriangles_Internal(
    const Material *material,
    MaterialTechniqueType techType,
    __int16 indexCount,
    const unsigned __int16 *indices,
    __int16 vertexCount,
    const float (*xyzw)[4],
    const float (*normal)[3],
    const GfxColor *color,
    const float (*st)[2]);
void __cdecl RB_DrawProfileCmd(GfxRenderCommandExecState *execState);
void __cdecl RB_SetMaterialColorCmd(GfxRenderCommandExecState *execState);
void __cdecl R_SetCodeConstantFromVec4(GfxCmdBufSourceState *source, unsigned int constant, float *value);
void __cdecl R_DirtyCodeConstant(GfxCmdBufSourceState *source, unsigned int constant);
void __cdecl RB_SetViewportCmd(GfxRenderCommandExecState *execState);
void __cdecl RB_LookupColor(unsigned __int8 c, GfxColor *color);
void __cdecl RB_DrawText(const char *text, Font_s *font, float x, float y, GfxColor color);
void __cdecl DrawText2D(
    const char *text,
    float x,
    float y,
    Font_s *font,
    float xScale,
    float yScale,
    float sinAngle,
    float cosAngle,
    GfxColor color,
    int maxLength,
    __int16 renderFlags,
    int cursorPos,
    char cursorLetter,
    float padding,
    GfxColor glowForcedColor,
    int fxBirthTime,
    int fxLetterTime,
    int fxDecayStartTime,
    int fxDecayDuration,
    const Material *fxMaterial,
    const Material *fxMaterialGlow);
void __cdecl RB_DrawStretchPicRotate(
    const Material *material,
    float x,
    float y,
    float w,
    float h,
    float s0,
    float t0,
    float s1,
    float t1,
    float sinAngle,
    float cosAngle,
    unsigned int color,
    GfxPrimStatsTarget statsTarget);
double __cdecl RB_DrawHudIcon(
    const char *text,
    float x,
    float y,
    float sinAngle,
    float cosAngle,
    Font_s *font,
    float xScale,
    float yScale,
    unsigned int color);
void __cdecl RB_DrawCursor(
    const Material *material,
    unsigned __int8 cursor,
    float x,
    float y,
    float sinAngle,
    float cosAngle,
    Font_s *font,
    float xScale,
    float yScale,
    unsigned int color);
void __cdecl RotateXY(
    float cosAngle,
    float sinAngle,
    float pivotX,
    float pivotY,
    float x,
    float y,
    float *outX,
    float *outY);
double __cdecl GetMonospaceWidth(Font_s *font, char renderFlags);
void __cdecl GlowColor(GfxColor *result, GfxColor baseColor, GfxColor forcedGlowColor, char renderFlags);
char __cdecl SetupPulseFXVars(
    const char *text,
    int maxLength,
    char renderFlags,
    int fxBirthTime,
    int fxLetterTime,
    int fxDecayStartTime,
    int fxDecayDuration,
    bool *resultDrawRandChar,
    int *resultRandSeed,
    int *resultMaxLength,
    bool *resultDecaying,
    int *resultdecayTimeElapsed);
void __cdecl GetDecayingLetterInfo(
    unsigned int letter,
    Font_s *font,
    int *randSeed,
    int decayTimeElapsed,
    int fxBirthTime,
    int fxDecayDuration,
    unsigned __int8 alpha,
    bool *resultSkipDrawing,
    unsigned __int8 *resultAlpha,
    unsigned int *resultLetter,
    bool *resultDrawExtraFxChar);
void __cdecl DrawTextFxExtraCharacter(
    const Material *material,
    int charIndex,
    float x,
    float y,
    float w,
    float h,
    float sinAngle,
    float cosAngle,
    unsigned int color);
unsigned __int8 __cdecl ModulateByteColors(unsigned __int8 colorA, unsigned __int8 colorB);
void __cdecl RB_DrawTextInSpace(
    const char *text,
    Font_s *font,
    const float *org,
    const float *xPixelStep,
    const float *yPixelStep,
    unsigned int color);
void __cdecl RB_DrawCharInSpace(
    const Material *material,
    float *xyz,
    const float *dx,
    const float *dy,
    const Glyph *glyph,
    unsigned int color);
void __cdecl RB_DrawText2DCmd(GfxRenderCommandExecState *execState);
void __cdecl RB_DrawText3DCmd(GfxRenderCommandExecState *execState);
void __cdecl RB_ProjectionSetCmd(GfxRenderCommandExecState *execState);
void __cdecl RB_ResetStatTracking();
void __cdecl RB_BeginFrame(const GfxBackEndData *data);
void __cdecl RB_EndFrame(char drawType);
GfxIndexBufferState *RB_SwapBuffers();
void RB_UpdateBackEndDvarOptions();
void __cdecl RB_ExecuteRenderCommandsLoop(const void *cmds);
void __cdecl RB_Draw3D();
void __cdecl RB_CallExecuteRenderCommands();
// positive sp value has been detected, the output may be wrong!
void __cdecl  RB_RenderThread(unsigned int threadContext);
void __cdecl RB_RenderCommandFrame(const GfxBackEndData *data);
void __cdecl RB_InitBackendGlobalStructs();
void __cdecl RB_SetBspImages();
void __cdecl RB_InitCodeImages();
void __cdecl RB_BindDefaultImages();
void __cdecl RB_RegisterBackendAssets();

void RB_AbandonGpuFence();
void __cdecl R_InsertGpuFence();
bool __cdecl R_GpuFenceTimeout();
void __cdecl R_FinishGpuFence();
void __cdecl R_AcquireGpuFenceLock();
void __cdecl R_ReleaseGpuFenceLock();


// rb_imagetouch
void __cdecl RB_TouchAllImages();

extern r_backEndGlobals_t backEnd;
extern materialCommands_t tess;
extern GfxBackEndData *backEndData;
//extern GfxRenderTarget gfxRenderTargets[15];
extern GfxRenderTarget gfxRenderTargets[17];// LWSS: changed to 17 to please ASAN. May not be correct.