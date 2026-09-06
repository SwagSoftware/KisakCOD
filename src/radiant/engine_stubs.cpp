// engine_stubs.cpp
// Definitions for engine symbols the Radiant build links but cannot compile (unsafe
// headers: mss.h, snd_local.h, client.h, scr_parser.h, game_public.h), plus the editor
// globals whose home file is not ported yet.  A stub graduates by being deleted here.

#include "stdafx.h"   // windows.h, d3d9/dx headers, cstdio, cstring, etc.
#include <cstdarg>
#include <csetjmp>    // asset-drop recovery guard (see Com_Error)
#include <intrin.h>   // __debugbreak()

// Engine headers safe for KISAK_RADIANT
#include <gfx_d3d/r_init.h>       // r_globals_t, GfxViewInfo, etc.
#include <gfx_d3d/r_image.h>      // ImgGlobals
#include <gfx_d3d/r_material.h>   // MaterialGlobals
#include <gfx_d3d/rb_state.h>     // gfxCmdBufInput (cinematic code-image seed)
#include <gfx_d3d/r_bsp.h>        // GfxWorld
#include <universal/com_memory.h>  // fileData_s
#include <universal/com_math.h>    // mat4x4 + typed MatrixInverse44(const mat4x4&, mat4x4&)
#include <qcommon/qcommon.h>       // Com_*, errorParm_t, CM_*, DObj_s, dvar_t (via q_shared.h)
#include <qcommon/threads.h>       // ThreadContext_t, WinThreadLock
#include <qcommon/com_bsp.h>       // LumpType, ComSaveLumpBehavior
#include <database/database.h>     // XAssetType, XAssetHeader, XZoneInfo
#include <DynEntity/DynEntity_client.h> // DynEntityDrawType, DynEntityCollType
#include <EffectsCore/fx_system.h> // FxEffectDef, FxSystem, FxCmd, FX_*
#include <gfx_d3d/r_dpvs.h>       // DpvsDynamicCellCmd
#include <gfx_d3d/r_reflection_probe.h> // DiskGfxReflectionProbe
#include <gfx_d3d/rb_stats.h>     // GfxPrimStatsTarget
#include <gfx_d3d/r_workercmds_common.h>
#include <qcommon/graph.h>         // DevGraph
#include <stringed/stringed_hooks.h> // msgLocErrType_t, SEH_*
#include <cgame/cg_pose.h>
#include <cgame/cg_ents.h>
#include <xanim/dobj_utils.h>

// Forward declarations for types from unsafe headers
struct LoadedSound;
struct unz_file_info_s;
struct unz_global_info_s;

// snd_stopsounds_arg_t forward (from sound/snd_public.h which pulls mss.h)
enum snd_stopsounds_arg_t : __int32;

// Globals from database/db_registry.cpp / win32/win_main.cpp / qcommon/threads.cpp
// (those files are not in the Radiant build set).
r_globals_t rg{};
ImgGlobals imageGlobals{};
MaterialGlobals materialGlobals{};
GfxWorld s_world{};
fileData_s *com_fileDataHashTable[1024] = {};
HWND g_splashWnd = nullptr;
uint32_t s_affinityMaskForCpu[4] = {};
uint32_t s_affinityMaskForProcess = 0;
uint32_t s_cpuCount = 1;
volatile uint32_t g_mainThreadBlocked = 0;

// Renderer hooks normally supplied by the SP client/cgame targets.
namespace
{
    constexpr long RADIANT_SKEL_MEMORY_SIZE = 0x200000;
    __declspec(align(16)) unsigned char s_radiantSkelMemory[RADIANT_SKEL_MEMORY_SIZE];
    volatile long s_radiantSkelMemoryPos = 0;
    uint32_t s_radiantSkelFrame = UINT32_MAX;
}

DObjAnimMat *__cdecl CG_DObjCalcPose(const cpose_t *pose, const DObj_s *obj, int *partBits)
{
    iassert(pose);
    iassert(obj);

    const uint32_t frame = rg.frontEndFrameCount;
    if (s_radiantSkelFrame != frame)
    {
        s_radiantSkelFrame = frame;
        InterlockedExchange(&s_radiantSkelMemoryPos, 0);
    }

    DObjAnimMat *boneMatrix;
    if (DObjSkelExists(obj, frame))
    {
        boneMatrix = I_dmaGetDObjSkel(obj);
        if (DObjSkelAreBonesUpToDate(obj, partBits))
            return boneMatrix;
    }
    else
    {
        const long size = (DObjGetAllocSkelSize(obj) + 15) & ~15;
        const long offset = InterlockedExchangeAdd(&s_radiantSkelMemoryPos, size);
        if (offset < 0 || offset + size > RADIANT_SKEL_MEMORY_SIZE)
        {
            Com_PrintWarning(CON_CHANNEL_CLIENT, "Radiant skeleton memory exhausted\n");
            return nullptr;
        }

        boneMatrix = reinterpret_cast<DObjAnimMat *>(&s_radiantSkelMemory[offset]);
        DObjCreateSkel(const_cast<DObj_s *>(obj), reinterpret_cast<char *>(boneMatrix), frame);
    }

    DObjCompleteHierarchyBits(obj, partBits);
    DObjCalcSkel(obj, partBits);
    return boneMatrix;
}

void __cdecl CG_GetPoseOrigin(const cpose_t *pose, float *origin)
{
    iassert(pose);
    Vec3Copy(pose->origin, origin);
}

void __cdecl CG_GetPoseAngles(const cpose_t *pose, float *angles)
{
    iassert(pose);
    Vec3Copy(pose->angles, angles);
}

void __cdecl CG_UsedDObjCalcPose(cpose_t *pose)
{
    iassert(pose);
    InterlockedCompareExchange(reinterpret_cast<volatile long *>(&pose->cullIn), 1, 0);
}

void __cdecl CG_CullIn(cpose_t *pose)
{
    iassert(pose);
    pose->cullIn = 2;
}

void __cdecl CG_PredictiveSkinCEntity(GfxSceneEntity *sceneEnt)
{
    iassert(sceneEnt);
    cpose_t *pose = sceneEnt->info.pose;
    if (pose->cullIn == 1)
    {
        pose->cullIn = 0;
        R_UpdateXModelBoundsDelayed(sceneEnt);
    }
    else if (pose->cullIn == 2)
    {
        pose->cullIn = 0;
        R_SkinGfxEntityDelayed(sceneEnt);
    }
}

void __cdecl CL_FlushDebugClientData() {}
void __cdecl CL_UpdateDebugClientData() {}
void __cdecl CG_CalculateFPS() {}
void __cdecl CL_UpdateSound() {}

// ─────────────────────────────────────────────────────────────────────────────
// Dvars the compiled engine subset dereferences but whose registrar file is not in
// the Radiant target.  GROUP A self-heals (R_RegisterDvars is compiled in and runs at
// R_Init); GROUP B is guarded at every deref; GROUP C is dereferenced UNCONDITIONALLY
// on render/asset paths and MUST be registered by Radiant_RegisterGroupCDvars below.
// ─────────────────────────────────────────────────────────────────────────────
// GROUP A — R_RegisterDvars (gfx_d3d/r_dvars.cpp).
const dvar_t *vid_xpos                = nullptr;
const dvar_t *vid_ypos                = nullptr;
const dvar_t *r_fullscreen            = nullptr;
const dvar_t *sv_cheats               = nullptr;
const dvar_t *com_statmon             = nullptr;
// GROUP B — null-safe at all deref sites.
const dvar_t *com_logfile             = nullptr;   // Com_InitDvars (common.cpp)
// GROUP C — registered below; home registrar in parentheses.
const dvar_t *sys_smp_allowed         = nullptr;   // Com_InitDvars  | render path, unconditional
const dvar_t *com_sv_running          = nullptr;   // Com_InitDvars
const dvar_t *com_wideScreen          = nullptr;   // Com_InitDvars  | r_init.cpp Dvar_SetBool
const dvar_t *useFastFile             = nullptr;   // Com_InitDvars
const dvar_t *fx_marks                = nullptr;   // FX_RegisterDvars
const dvar_t *fx_marks_smodels        = nullptr;   // FX_RegisterDvars
const dvar_t *fx_marks_ents           = nullptr;   // FX_RegisterDvars
const dvar_t *snd_errorOnMissing      = nullptr;   // SND_Init
const dvar_t *snd_touchStreamFilesOnLoad = nullptr;// SND_Init
const dvar_t *sys_SSE                 = nullptr;   // Com_PlayerProfile_RegisterDvars | r_model_skin.cpp:63

// Values match the originals EXCEPT the two that must reflect "editor = windowed,
// synchronous, loose-asset tool": sys_smp_allowed FALSE (R_IssueRenderCommands must not
// hand off to a backend thread the editor never spawns) and useFastFile FALSE (loose
// assets).  Idempotent, so a later real Com_InitDvars would not double-register.
void Radiant_RegisterGroupCDvars()
{
    if (!sys_smp_allowed)
        sys_smp_allowed = Dvar_RegisterBool("sys_smp_allowed", 0, DVAR_INIT, "Allow multi-threading");
    if (!useFastFile)
        useFastFile = Dvar_RegisterBool("useFastFile", 0, DVAR_INIT,
            "Enables loading data from fast files. Only tools can run without fast files.");
    if (!com_sv_running)
        com_sv_running = Dvar_RegisterBool("sv_running", 0, DVAR_ROM, "Server is running");
    if (!com_wideScreen)
        com_wideScreen = Dvar_RegisterBool("wideScreen", 1, DVAR_ROM,
            "True if the game video is running in 16x9 aspect, false if 4x3.");
    if (!fx_marks)
        fx_marks = Dvar_RegisterBool("fx_marks", 1, DVAR_ARCHIVE, "Enable bullet impact marks");
    if (!fx_marks_smodels)
        fx_marks_smodels = Dvar_RegisterBool("fx_marks_smodels", 1, DVAR_ARCHIVE, "Enable bullet marks on static models");
    if (!fx_marks_ents)
        fx_marks_ents = Dvar_RegisterBool("fx_marks_ents", 1, DVAR_ARCHIVE, "Enable bullet marks on entities");
    if (!snd_errorOnMissing)
        snd_errorOnMissing = Dvar_RegisterBool("snd_errorOnMissing", 0, DVAR_NOFLAG, "Error on missing sound alias");
    if (!snd_touchStreamFilesOnLoad)
        snd_touchStreamFilesOnLoad = Dvar_RegisterBool("snd_touchStreamFilesOnLoad", 0, DVAR_NOFLAG, "Touch stream files on load");
    if (!sys_SSE)
        sys_SSE = Dvar_RegisterBool("sys_SSE",
            IsProcessorFeaturePresent(PF_XMMI_INSTRUCTIONS_AVAILABLE) != 0, DVAR_ROM,
            "Operating system allows Streaming SIMD Extensions");
}

// ─────────────────────────────────────────────────────────────────────────────
// Com_* — logging / error
// ─────────────────────────────────────────────────────────────────────────────
void QDECL Com_Printf(int channel, const char *fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    vprintf(fmt, args);
    va_end(args);
}

// Editor asset-drop recovery frame.  The engine's Com_Error(ERR_DROP) longjmps to a
// per-thread abort frame the Radiant build never installs, so ERR_DROP would be fatal.
// Editor asset loads (model skin, brush-face material realize) bracket themselves with
// this guard; while it is armed an ERR_DROP unwinds here and the asset is skipped.
// ERR_FATAL stays fatal even inside the guard.
int     g_radiantAssetLoadGuard = 0;     // >0 while inside an editor asset-load bracket
jmp_buf g_radiantAssetLoadJmp;           // armed by Cam_SkinModelGuarded / below

// Runs brush.cpp's load-time face-material realize under that frame; on an ERR_DROP the
// brush's remaining faces stay degenerate (wireframe fallback) instead of killing the
// editor.  Lives in this TU because MSVC forbids setjmp in a function with C++ unwinding.
struct brush_t;                                         // full def in qe3.h (included below)
extern void Brush_RealizeFaceMaterials(brush_t *def);   // brush.cpp

void Brush_RealizeFaceMaterialsGuarded(brush_t *def)
{
    if (!def)
        return;
    if (setjmp(g_radiantAssetLoadJmp) != 0)
    {
        g_radiantAssetLoadGuard = 0;   // unwound from an ERR_DROP
        return;
    }
    ++g_radiantAssetLoadGuard;
    Brush_RealizeFaceMaterials(def);
    --g_radiantAssetLoadGuard;
}

// Break into an attached debugger before every fatal ExitProcess so the Call Stack window
// still shows who raised it; no-op when no debugger is attached.
#define RADIANT_BREAK_BEFORE_FATAL_EXIT()  do { if (IsDebuggerPresent()) __debugbreak(); } while (0)

void QDECL Com_Error(errorParm_t code, const char *fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    char msg[1024];
    _vsnprintf(msg, sizeof(msg), fmt ? fmt : "", args);
    msg[sizeof(msg) - 1] = 0;
    va_end(args);

    // stderr is invisible in the windowed editor: also record every Com_Error (code +
    // guard depth) to %TEMP%\radiant_firstlight.log — that is what identifies the asset
    // or path that raised it when the editor "just closes".
    {
        char tmp[MAX_PATH], logPath[MAX_PATH];
        GetTempPathA(sizeof(tmp), tmp);
        _snprintf(logPath, sizeof(logPath), "%sradiant_firstlight.log", tmp);
        FILE *lf = fopen(logPath, "a");
        if (lf)
        {
            fprintf(lf, "COM_ERROR(code=%d, guard=%d): %s\n", (int)code, g_radiantAssetLoadGuard, msg);
            fclose(lf);
        }
    }

    if (g_radiantAssetLoadGuard > 0 && code == ERR_DROP)
    {
        fprintf(stderr, "ASSET-DROP (recovered, skipped): %s\n", msg);
        g_radiantAssetLoadGuard = 0;     // clear before unwinding (frame is leaving)
        longjmp(g_radiantAssetLoadJmp, 1);
    }

    fprintf(stderr, "COM_ERROR(%d): %s\n", (int)code, msg);
    RADIANT_BREAK_BEFORE_FATAL_EXIT();
    ExitProcess(1);
}

void __cdecl Com_PrintError(int channel, const char *fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    char msg[1024];
    _vsnprintf(msg, sizeof(msg), fmt ? fmt : "", args);
    msg[sizeof(msg) - 1] = 0;
    va_end(args);

    fprintf(stderr, "ERROR[%d]: %s", channel, msg);

    // Also to the firstlight log: this is where Image_Register_LoadObj's "failed to load
    // image" lands — a white texture-browser thumbnail whose image IS logged here is a
    // load/parse bug; one that is not logged is simply a missing asset.
    {
        char tmp[MAX_PATH], logPath[MAX_PATH];
        GetTempPathA(sizeof(tmp), tmp);
        _snprintf(logPath, sizeof(logPath), "%sradiant_firstlight.log", tmp);
        FILE *lf = fopen(logPath, "a");
        if (lf) { fprintf(lf, "ERROR[%d]: %s", channel, msg); fclose(lf); }
    }
}

void __cdecl Com_DPrintf(int channel, const char *fmt, ...)
{
    (void)channel; (void)fmt;   // debug channel not hooked up in the editor
}

void __cdecl Com_PrintWarning(int channel, const char *fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    fprintf(stderr, "WARNING: ");
    vfprintf(stderr, fmt, args);
    va_end(args);
}

// Com_PrintMessage comes from cmdlib.cpp (varargs form); the engine-ABI form
// (int channel, const char *msg, int error) is unused in this build.

void __cdecl Com_ErrorAbort() { RADIANT_BREAK_BEFORE_FATAL_EXIT(); ExitProcess(1); }
int  __cdecl Com_SafeMode()   { return 0; }
bool __cdecl Com_LogFileOpen() { return false; }
void __cdecl Com_StartupVariable(const char *match) { (void)match; }
void __cdecl Com_SyncThreads() {}
DObj_s *__cdecl Com_GetClientDObj(uint32_t handle, int localClientNum) { (void)handle; (void)localClientNum; return nullptr; }

// ─────────────────────────────────────────────────────────────────────────────
// Sys_* — threading / renderer / process stubs
// ─────────────────────────────────────────────────────────────────────────────
void __cdecl Sys_Error(const char *fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    fprintf(stderr, "SYS_ERROR: ");
    vfprintf(stderr, fmt, args);
    va_end(args);
    fputc('\n', stderr);
    RADIANT_BREAK_BEFORE_FATAL_EXIT();
    ExitProcess(1);
}

void __cdecl Sys_OutOfMemErrorInternal(const char *file, int line)
{
    fprintf(stderr, "OUT_OF_MEMORY: %s:%d\n", file ? file : "?", line);
    RADIANT_BREAK_BEFORE_FATAL_EXIT();
    ExitProcess(1);
}

void __cdecl Sys_NormalExit()           { ExitProcess(0); }
void __cdecl Sys_FrontEndSleep()        {}
void __cdecl Sys_WaitForMainThread()    {}
void __cdecl Sys_WaitForWorkerCmd()     {}
void __cdecl Sys_SetWorkerCmdEvent()    {}
void __cdecl Sys_ResetWorkerCmdEvent()  {}
void __cdecl Sys_NotifyRenderer()       {}
void __cdecl Sys_RenderCompleted()      {}
void __cdecl Sys_StartRenderer()        {}
void __cdecl Sys_StopRenderer()         {}
void __cdecl Sys_ReleaseThreadOwnership() {}
void __cdecl Sys_WakeRenderer(void *data) { (void)data; }
// Per-thread Sys_SetValue/Sys_GetValue slots.  The engine backs these with
// g_threadValues[ctx][4] installed by Sys_InitMainThread; the editor runs the renderer
// synchronously on the main thread, so a thread_local 4-slot array is equivalent.
// Com_InitThreadData(THREAD_CONTEXT_MAIN) fills slot 1 = &va_info[0] (va() derefs it
// unguarded), 2 = &g_com_error[0], 3 = &g_traceThreadInfo[0].
static thread_local void *g_radiantThreadValues[4];
void __cdecl Sys_SetValue(int idx, void *data) { g_radiantThreadValues[idx] = data; }
void *__cdecl Sys_GetValue(int idx)     { return g_radiantThreadValues[idx]; }
void *__cdecl Sys_RendererSleep()       { return nullptr; }

bool __cdecl Sys_IsMainThread()         { return true; }
bool __cdecl Sys_IsRenderThread()       { return false; }
bool __cdecl Sys_IsDatabaseThread()     { return false; }
bool __cdecl Sys_FinishRenderer()       { return true; }
int  __cdecl Sys_IsMainThreadReady()    { return 1; }
int  __cdecl Sys_IsRendererReady()      { return 1; }
int  __cdecl Sys_RendererReady()        { return 1; }
int  __cdecl Sys_WaitBackendEvent()     { return 0; }
uint32_t __cdecl Sys_GetCpuCount()      { return 1; }

char __cdecl Sys_SpawnRenderThread(void(__cdecl *function)(uint32_t)) { (void)function; return 1; }
bool __cdecl Sys_SpawnWorkerThread(void(__cdecl *function)(uint32_t), uint32_t idx) { (void)function; (void)idx; return true; }

void __cdecl Sys_SuspendThread(ThreadContext_t ctx) { (void)ctx; }
void __cdecl Sys_ResumeThread(ThreadContext_t ctx)  { (void)ctx; }

// ─────────────────────────────────────────────────────────────────────────────
// Win32 helpers
// ─────────────────────────────────────────────────────────────────────────────
void __cdecl Win_SetThreadLock(WinThreadLock lock) { (void)lock; }
char *__cdecl Win_LocalizeRef(const char *ref)     { return const_cast<char *>(ref ? ref : ""); }

// ─────────────────────────────────────────────────────────────────────────────
// Geometry helpers
// ─────────────────────────────────────────────────────────────────────────────
int __cdecl BoxOnPlaneSide(const float *emins, const float *emaxs, const cplane_s *p)
{
    (void)emins; (void)emaxs; (void)p;
    return 3; // both sides
}

void __cdecl _copyDWord(uint32_t *dest, uint32_t constant, uint32_t count)
{
    for (uint32_t i = 0; i < count; ++i)
        dest[i] = constant;
}

// ─────────────────────────────────────────────────────────────────────────────
// CM (collision map) stubs
// ─────────────────────────────────────────────────────────────────────────────
void __cdecl CM_BoxTrace(trace_t *results, const float *start, const float *end,
    const float *mins, const float *maxs, uint32_t model, int contentmask)
{
    (void)results; (void)start; (void)end; (void)mins; (void)maxs; (void)model; (void)contentmask;
    if (results) memset(results, 0, sizeof(*results));
}

void __cdecl CM_CalcTraceExtents(TraceExtents *te) { if (te) memset(te, 0, sizeof(*te)); }

int  __cdecl CM_TraceBox(const TraceExtents *te, float *p1, float *p2, float dist)
{ (void)te; (void)p1; (void)p2; (void)dist; return 0; }

int  __cdecl CM_BoxSightTrace(int model, const float *start, const float *end,
    const float *mins, const float *maxs, uint32_t mask, int skipmask)
{ (void)model; (void)start; (void)end; (void)mins; (void)maxs; (void)mask; (void)skipmask; return 0; }

int  __cdecl CM_GetPlaneCount() { return 0; }
cplane_s *__cdecl CM_GetPlanes() { return nullptr; }
uint8_t *__cdecl CM_Hunk_Alloc(uint32_t size, const char *name, int type)
{ (void)size; (void)name; (void)type; return nullptr; }

// ─────────────────────────────────────────────────────────────────────────────
// DB (asset database) stubs
// ─────────────────────────────────────────────────────────────────────────────
void __cdecl DB_BeginRecoverLostDevice()  {}
void __cdecl DB_EndRecoverLostDevice()    {}
void __cdecl DB_ResetZoneSize(int rebuild) { (void)rebuild; }
void __cdecl DB_ShutdownXAssets()          {}
void __cdecl DB_SyncXAssets()              {}
void __cdecl DB_LoadedExternalData(int idx){ (void)idx; }
void __cdecl DB_LoadXAssets(XZoneInfo *zoneInfo, uint32_t zoneCount, int sync)
{ (void)zoneInfo; (void)zoneCount; (void)sync; }

bool __cdecl DB_IsMinimumFastFileLoaded()           { return true; }
bool __cdecl DB_IsXAssetDefault(XAssetType t, const char *name) { (void)t; (void)name; return true; }
int  __cdecl DB_GetAllXAssetOfType(XAssetType t, XAssetHeader *out, int count) { (void)t; (void)out; (void)count; return 0; }
int  __cdecl DB_GetAllXAssetOfType_FastFile(XAssetType t, XAssetHeader *out, int count) { (void)t; (void)out; (void)count; return 0; }

XAssetHeader __cdecl DB_FindXAssetHeader(XAssetType t, const char *name)
{ (void)t; (void)name; XAssetHeader h{}; return h; }

void __cdecl DB_EnumXAssets(XAssetType t, void(__cdecl *callback)(XAssetHeader, void*), void *data, bool overrides)
{ (void)t; (void)callback; (void)data; (void)overrides; }

void __cdecl DB_GetIndexBufferAndBase(uint8_t pool, void *ibOut, void **baseOut, int *offsetOut)
{ (void)pool; (void)ibOut; if (baseOut) *baseOut = nullptr; if (offsetOut) *offsetOut = 0; }

void __cdecl DB_GetVertexBufferAndOffset(uint8_t zoneHandle, uint8_t *verts, void **vbOut, int *offsetOut)
{ (void)zoneHandle; (void)verts; if (vbOut) *vbOut = nullptr; if (offsetOut) *offsetOut = 0; }

// ─────────────────────────────────────────────────────────────────────────────
// FX stubs
// ─────────────────────────────────────────────────────────────────────────────
const FxEffectDef *__cdecl FX_Register(const char *name) { (void)name; return nullptr; }
FxSystem *__cdecl FX_GetSystem(int clientIndex) { (void)clientIndex; return nullptr; }

void __cdecl FX_BeginUpdate(int32_t localClientNum) { (void)localClientNum; }
void __cdecl FX_FillUpdateCmd(int32_t localClientNum, FxCmd *cmd) { (void)localClientNum; (void)cmd; }
void __cdecl FX_GenerateVerts(FxGenerateVertsCmd *cmd) { (void)cmd; }
void __cdecl FX_RunPhysics(int32_t localClientNum) { (void)localClientNum; }
void __cdecl FX_SetNextUpdateCamera(int32_t localClientNum, const refdef_s *refdef, float zfar)
{ (void)localClientNum; (void)refdef; (void)zfar; }

void __cdecl FX_BeginGeneratingMarkVertsForEntModels(int32_t lc, uint32_t *indexCount)
{ (void)lc; (void)indexCount; }
void __cdecl FX_EndGeneratingMarkVertsForEntModels(int32_t lc) { (void)lc; }

void __cdecl FX_GenerateMarkVertsForEntXModel(int32_t lc, int32_t markGroup, uint32_t *indexCount,
    uint16_t entityHandle, uint8_t unused, const GfxScaledPlacement *placement)
{ (void)lc; (void)markGroup; (void)indexCount; (void)entityHandle; (void)unused; (void)placement; }

void __cdecl FX_GenerateMarkVertsForEntBrush(int32_t lc, int32_t entId, uint32_t *indexCount,
    uint8_t reflectionProbeIndex, const GfxPlacement *placement)
{ (void)lc; (void)entId; (void)indexCount; (void)reflectionProbeIndex; (void)placement; }

void __cdecl FX_GenerateMarkVertsForEntDObj(int32_t lc, int32_t markGroup, uint32_t *indexCount,
    uint16_t entityHandle, uint8_t unused, const DObj_s *dobj, const cpose_t *pose)
{ (void)lc; (void)markGroup; (void)indexCount; (void)entityHandle; (void)unused; (void)dobj; (void)pose; }

void __cdecl FX_GenerateMarkVertsForStaticModels(int32_t lc, int32_t markGroup, const uint8_t *visData)
{ (void)lc; (void)markGroup; (void)visData; }

void __cdecl FX_GenerateMarkVertsForWorld(int32_t lc) { (void)lc; }

// ─────────────────────────────────────────────────────────────────────────────
// DynEnt stubs
// ─────────────────────────────────────────────────────────────────────────────
void __cdecl DynEntCl_ProcessEntities(int localClientNum)    { (void)localClientNum; }
void __cdecl DynEntPieces_AddDrawSurfs()                     {}
DynEntityClient *__cdecl DynEnt_GetClientEntity(uint16_t idx, DynEntityDrawType type)
{ (void)idx; (void)type; return nullptr; }
const DynEntityDef *__cdecl DynEnt_GetEntityDef(uint16_t idx, DynEntityDrawType type)
{ (void)idx; (void)type; return nullptr; }
DynEntityPose *__cdecl DynEnt_GetClientPose(uint16_t idx, DynEntityDrawType type)
{ (void)idx; (void)type; return nullptr; }
uint16_t __cdecl DynEnt_GetEntityCount(DynEntityCollType type) { (void)type; return 0; }
int __cdecl DynEnt_GetXModelUsageCount(const XModel *model)   { (void)model; return 0; }
DynEntityPose *__cdecl DynEnt_GetClientModelPoseList()         { return nullptr; }

// ─────────────────────────────────────────────────────────────────────────────
// SND stubs
// ─────────────────────────────────────────────────────────────────────────────
bool __cdecl SND_IsMultiChannel()                              { return false; }
void __cdecl SND_StopSounds(snd_stopsounds_arg_t which)        { (void)which; }
LoadedSound *__cdecl SND_LoadSoundFile(const char *name)       { (void)name; return nullptr; }
int  __cdecl SND_GetSoundFileSize(uint32_t *outSize)           { if (outSize) *outSize = 0; return 0; }

// ─────────────────────────────────────────────────────────────────────────────
// Scr stubs
// ─────────────────────────────────────────────────────────────────────────────
void __cdecl Scr_AddArray()                              {}
void __cdecl Scr_AddConstString(uint32_t s)              { (void)s; }
void __cdecl Scr_AddFloat(float f)                       { (void)f; }
void __cdecl Scr_MonitorCommand(const char *cmd)         { (void)cmd; }
void __cdecl Scr_NotifyNum(uint32_t id, uint32_t type, uint32_t name, uint32_t count)
{ (void)id; (void)type; (void)name; (void)count; }

// ─────────────────────────────────────────────────────────────────────────────
// SEH (localization) stubs
// ─────────────────────────────────────────────────────────────────────────────
void __cdecl SEH_Init_StringEd()       {}
void __cdecl SEH_Shutdown_StringEd()   {}
void __cdecl SEH_InitLanguage()        {}
void __cdecl SEH_UpdateLanguageInfo()  {}

char *__cdecl SEH_LocalizeTextMessage(const char *text, const char *msgType, msgLocErrType_t errType)
{ (void)msgType; (void)errType; return const_cast<char *>(text ? text : ""); }

char *__cdecl SEH_SafeTranslateString(char *s)             { return s ? s : const_cast<char *>(""); }
const char *__cdecl SEH_GetLanguageName(uint32_t lang)     { (void)lang; return "english"; }
int  __cdecl SEH_GetLanguageIndexForName(const char *name, int *idx)
{ (void)name; if (idx) *idx = 0; return 0; }
int  __cdecl SEH_GetCurrentLanguage()                       { return 0; }
int  __cdecl SEH_PrintStrlen(const char *s)                 { return s ? (int)strlen(s) : 0; }
uint32_t __cdecl SEH_DecodeLetter(uint32_t c, uint32_t font, int *outLen, int *outGlyph)
{ (void)c; (void)font; if (outLen) *outLen = 1; if (outGlyph) *outGlyph = 0; return c; }
uint32_t __cdecl SEH_ReadCharFromString(const char **text, int *isTrailingPunct)
{
    if (!text || !*text || !**text) { if (isTrailingPunct) *isTrailingPunct = 0; return 0; }
    if (isTrailingPunct) *isTrailingPunct = 0;
    return (uint8_t)*(*text)++;
}

// ─────────────────────────────────────────────────────────────────────────────
// R_Cinematic — the IDB editor build nulls Init (R_Cinematic_Init_NULL 0x536060, a pure
// retn), so no cinematic thread exists and activeImageFrame stays 0.
// ─────────────────────────────────────────────────────────────────────────────
void __cdecl R_Cinematic_Init()             {}
void __cdecl R_Cinematic_Shutdown()         {}
// 0x5360a0 — the editor variant is JUST the activeImageFrame==-1 branch of
// R_Cinematic_UpdateRendererImages (black/gray/gray/black), no thread sync.  Runs every
// frame via RB_BeginFrame, so the cinematic code images are non-NULL before the texture
// browser draws a cinematic-material thumbnail (otherwise R_TextureFromCodeError fatals
// with "Tried to use 'cinematicY' when it isn't valid").
void __cdecl R_Cinematic_UpdateFrame()
{
    gfxCmdBufInput.codeImages[TEXTURE_SRC_CODE_CINEMATIC_Y]  = rgp.blackImage;
    gfxCmdBufInput.codeImages[TEXTURE_SRC_CODE_CINEMATIC_CR] = rgp.grayImage;
    gfxCmdBufInput.codeImages[TEXTURE_SRC_CODE_CINEMATIC_CB] = rgp.grayImage;
    gfxCmdBufInput.codeImages[TEXTURE_SRC_CODE_CINEMATIC_A]  = rgp.blackImage;
}
void __cdecl R_Cinematic_BeginLostDevice()  {}
void __cdecl R_Cinematic_EndLostDevice()    {}
bool __cdecl R_Cinematic_IsStarted()        { return false; }
bool __cdecl R_Cinematic_IsUnderrun()       { return false; }

// ─────────────────────────────────────────────────────────────────────────────
// RB profile / DevGui stubs
// ─────────────────────────────────────────────────────────────────────────────
void __cdecl RB_DrawProfile()       {}
void __cdecl RB_DrawProfileScript() {}
void __cdecl RB_ProfileInit()       {}
void __cdecl DevGui_AddGraph(const char *path, DevGraph *graph) { (void)path; (void)graph; }

// ─────────────────────────────────────────────────────────────────────────────
// Net / server stubs
// ─────────────────────────────────────────────────────────────────────────────
void __cdecl NET_Sleep(int msec) { Sleep(msec > 0 ? (DWORD)msec : 0); }
int  __cdecl SV_GameCommand()    { return 0; }
void __cdecl FS_PureServerSetLoadedIwds(char *loaded, char *referenced) { (void)loaded; (void)referenced; }

// ─────────────────────────────────────────────────────────────────────────────
// Physics stub
// ─────────────────────────────────────────────────────────────────────────────
void __cdecl Phys_ObjSetCollisionFromXModel(const XModel *model, PhysWorld world, dxBody *body)
{ (void)model; (void)world; (void)body; }

// ─────────────────────────────────────────────────────────────────────────────
// FX worker-thread update stubs (r_workercmds_common.cpp)
// ─────────────────────────────────────────────────────────────────────────────
void __cdecl FX_FillGenerateVertsCmd(int32_t localClientNum, FxGenerateVertsCmd *cmd) { (void)localClientNum; (void)cmd; }
void __cdecl FX_UpdateSpotLight(FxCmd *cmd)        { (void)cmd; }
void __cdecl FX_UpdateNonDependent(FxCmd *cmd)     { (void)cmd; }
void __cdecl FX_UpdateRemaining(FxCmd *cmd)        { (void)cmd; }
void __cdecl FX_EndUpdate(int32_t localClientNum)  { (void)localClientNum; }
void __cdecl FX_AddNonSpriteDrawSurfs(FxCmd *cmd)  { (void)cmd; }

// ─────────────────────────────────────────────────────────────────────────────
// Sys FX-event stubs (r_workercmds_common.cpp)
// ─────────────────────────────────────────────────────────────────────────────
void __cdecl Sys_SetUpdateSpotLightEffectEvent()           {}
void __cdecl Sys_ResetUpdateSpotLightEffectEvent()         {}
void __cdecl Sys_WaitUpdateNonDependentEffectsCompleted()  {}
void __cdecl Sys_SetUpdateNonDependentEffectsEvent()       {}
void __cdecl Sys_ResetUpdateNonDependentEffectsEvent()     {}

// ─────────────────────────────────────────────────────────────────────────────
// ProfLoad stubs (load profiling, used in com_files/r_material/etc.)
// ─────────────────────────────────────────────────────────────────────────────
void __cdecl ProfLoad_Begin(const char *label)                              { (void)label; }
void __cdecl ProfLoad_End()                                                  {}
void __cdecl ProfLoad_BeginTrackedValue(MapProfileTrackedValue type)        { (void)type; }
void __cdecl ProfLoad_EndTrackedValue(MapProfileTrackedValue type)          { (void)type; }

// ─────────────────────────────────────────────────────────────────────────────
// CM collision-display stub (rb_showcollision.cpp)
// ─────────────────────────────────────────────────────────────────────────────
void __cdecl CM_ShowBrushCollision(int contentMask, cplane_s *frustumPlanes,
    int frustumPlaneCount, void(__cdecl *drawPoly)(int, float (*)[3], const float *))
{ (void)contentMask; (void)frustumPlanes; (void)frustumPlaneCount; (void)drawPoly; }

// ═════════════════════════════════════════════════════════════════════════════
// Radiant-side definitions from here down.
// ═════════════════════════════════════════════════════════════════════════════
#include "qe3.h"           // entity_s, brush_t, eclass_t, models_t
#include "prefs.h"         // g_PrefsDlg (prefData_t* — the real settings object)
#include "mainfrm.h"       // CMainFrame

// 0x49cea0 — the editor's Assert.  Routed to iassert so the port's assert handling is
// uniform; the binary pops a message box and lets you continue.
void Assert(const char *file, int line, int type, const char *fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    fprintf(stderr, "ASSERT [type=%d] %s:%d - ", type, file ? file : "?", line);
    vfprintf(stderr, fmt, args);
    va_end(args);
    fputc('\n', stderr);
    fflush(stderr);            // flush before the break, else the message is lost
    iassert(0);
}

// 0x49a9e0 — Radiant FatalError (common.cpp).
void FatalError(int code, const char *fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    fprintf(stderr, "FATAL ERROR(%d): ", code);
    vfprintf(stderr, fmt, args);
    va_end(args);
    fputc('\n', stderr);
    RADIANT_BREAK_BEFORE_FATAL_EXIT();
    ExitProcess(1);
}

// Renderer enable/disable refcount pair (no-ops here).
void DisableRendering_pp() {}
void DisableRendering_mm() {}

// MFC CString COW internals, referenced only by the g_bBuildList=1 eclass path (dead at
// runtime, g_bBuildList defaults 0).  Linker satisfaction only.
void cstr(void **, void *)                  {}
void str_set(void **, void *, int)          {}
void str_append(void **, void *, int)       {}
void __strcpy(void **, int)                 {}
void OleException(long)                     {}
void *lpBuffer = nullptr;                   // CString accumulation buffer (build-list path)

// `zero` (0x6D58F0) is the binary's empty-CString rep used as "" throughout the editor.
// MUST point at a real empty string, not nullptr: ported code casts it to const char* and
// feeds it to atof/strcmp/strncmp (first live hit was select.cpp's modelscale atof).
static char s_zeroEmptyString[4] = "";
void *zero = s_zeroEmptyString;

// ─── editor globals whose home .cpp is not ported yet ─────────────────────────
// grid_sizes 0x6dde5c — indexed by g_qeglobals.d_gridsize (0..10).
float grid_sizes[11] = { 0.5f, 1.0f, 2.0f, 4.0f, 8.0f, 16.0f, 32.0f, 64.0f, 128.0f, 256.0f, 512.0f };
char g_activeLayer_string[256] = "000_Global";   // 0x73bf78 (layers.cpp will own this)
int  g_nUpdateBits = 0;                          // 0x25d5a74 — pending window redraw bits (-1 = W_ALL)

// Brush list sentinels: selected_brushes 0x23f1864, active_brushes 0x23f189c,
// filtered_brushes 0x23f182c.  Embedded selbrush_t nodes (declared extern in qe3.h).
selbrush_t selected_brushes{};
selbrush_t active_brushes{};
selbrush_t filtered_brushes{};

// Clipper cluster (g_Clip1/2/3, g_bClipMode, g_bRogueClipMode, g_bSwitch, g_pMovingClip,
// g_brFrontSplits/BackSplits) — owned by xywnd.cpp's clipper code; the split-list
// sentinels self-point in Map_Free's bootstrap and in SetClipMode.
CClipPoint g_Clip1{};
CClipPoint g_Clip2{};
CClipPoint g_Clip3{};
int        g_bClipMode      = 0;
char       g_bRogueClipMode = 0;
char       g_bSwitch        = 0;
// 0x25d5b06 — XY crosshair toggle (Shift+X).  Consumers ported: CXYWnd::OnPaint ->
// Ed_DrawCrosshair (0x4655d0) and the XY_MouseMoved idle branch (0x46824b).
int        g_bCrossHairs    = 0;
float     *g_pMovingClip    = nullptr;
selbrush_t g_brFrontSplits{};
selbrush_t g_brBackSplits{};

int g_nPatchClickedView = 0;                // 0x73b108 — which view a patch was clicked in
int g_bPatchBendMode    = 0;                // 0x25d5b04
int g_qeglobals_redispersePatchVerts = 0;   // 0x25d5a6b — "redisperse control points" mode

// 0x181f51c — MaterialDef realize-state word (materialdef.cpp clears it on success;
// brush.cpp seeds/tests it for the special-material check).
int dword_181F51C = 0;

CMainFrame *g_pParentWnd = nullptr;         // 0x25d5a70

// prefab-edit stack (IDB byte_25EB240 @ 0x25eb240). KISAK BUG FIX: the binary's
// storage is 16 levels (runs to exactly g_qeglobals @0x25F39C0) and the full-check
// is == 16; the port's previous 8*2168 char array overflowed on deep nesting.
prefabLevel_t g_prefabStack[16] = {};

// texWndGlob.materialNameRemap (0x25e7a00) — head of the material-name remap list
// ({char* key; char* value; node* next}).  Populated by the unported material remap
// system; null here, so sub_45AD50 just strips to basename.
struct TexWndGlob_t { void *materialNameRemap; } texWndGlob;   // IDB texWndGlob (assert strings name .materialNameRemap)

// filter_usage_array 0x739f80 / filter_locale_array 0x73a780 — the usage/locale filter
// name tables (filter_material_t {char *name; int index} x 256), filled from index 1 by
// TexFilter_LoadMenuFile (qe3.cpp).  Entry [0] is the static {"all", 0} the IDB ships in
// .data and CTexWnd_Shutdown's free loop skips it, so its literal is never freed.
struct RadiantFilterEntry { char *name; int index; };
static_assert(sizeof(RadiantFilterEntry) == 8, "filter_material_t must be 8 bytes (IDB)");
RadiantFilterEntry filter_usage_array[256]  = { { (char *)"all", 0 } };
RadiantFilterEntry filter_locale_array[256] = { { (char *)"all", 0 } };

// 0x402ac0 — Byte4PackPixelColor packs BGRA (array[2]=R, [1]=G, [0]=B, [3]=A) with the
// usual 2^-30 round epsilon, identical to the engine's Byte4PackVertexColor; delegate.
extern void __cdecl Byte4PackVertexColor(const float *from, uint8_t *to);  // qcommon/com_pack.cpp
char Byte4PackPixelColor(float *from, GfxColor *out)
{
    if (out)
    {
        if (from)
            Byte4PackVertexColor(from, out->array);
        else
            out->packed = 0xFFFFFFFFu;   // null `from` keeps the white fallback
    }
    return 1;
}

// 0x471420 — grid-snap every face planept to the nearest half unit on the bFull
// Brush_BuildWindings edit path (brush create/resize, csg).  Skipped when prefs
// m_bNoClamp.  The loader uses bFull=0 and never reaches it.
unsigned int Brush_SnapPlanepts(brush_t *b)
{
    if ( g_PrefsDlg->m_bNoClamp )
        return 0;
    for ( int f = 0; f < b->faceCount; ++f )
    {
        float *p = &b->faces[f].planepts[0][0];
        for ( int i = 0; i < 9; ++i )
            p[i] = (float)( floor( 2.0 * (double)p[i] + 0.5 ) * 0.5 );
    }
    return (unsigned int)b->faceCount;
}

// ─── math helpers ────────────────────────────────────────────────────────────
// 0x40a5e0 — in-place normalize, zero-length safe.  RETURNS the pre-normalize LENGTH
// (Lines_AddLinkTo uses it to space connection arrowheads); most callers ignore it.
float Vec3Normalize_R(float *normal)
{
    float len = (float)sqrt((double)(normal[0]*normal[0]
                                   + normal[1]*normal[1]
                                   + normal[2]*normal[2]));
    float div = len;
    if ( -len >= 0.0f )   // len <= 0 (zero-length guard, matches IDA)
        div = 1.0f;
    float inv = 1.0f / div;
    normal[0] = normal[0] * inv;
    normal[1] = inv * normal[1];
    normal[2] = inv * normal[2];
    return len;
}

// Raw-pointer overload only Brush_ApplyTextureProjection needs; routes to the typed
// com_math.cpp inverse (it was a FATAL stub, so brush.cpp bound to the crash instead).
void MatrixInverse44(const void *src, void *dst)
{
    MatrixInverse44( *(const mat4x4 *)src, *(mat4x4 *)dst );
}

extern void Vec3Cross( const float *a, const float *b, float *out );   // com_math 0x40a4d0

// 0x4a45d0 (com_math.cpp:690) — index of the vec3 component with the largest
// magnitude.  Canonical port is linearmapping.cpp Vec3_MajorAxis (this file
// carried a duplicate copy).
extern int Vec3_MajorAxis( const float *dir );

// 0x4769a0 — solve the 2D texture-coordinate gradient for a face from two texture planes.
// a1 = out face normal, a2 = {Sx,Sy,Sz,Sd, Tx,Ty,Tz,Td}, a3 = out gradient.  Reached from
// Brush_ApplyTextureProjection (Drag_FaceAlign: align texture between two faces).
void sub_4769A0( int a1, int a2, int a3 )
{
    float *normalOut = (float *)(intptr_t)a1;
    float *p         = (float *)(intptr_t)a2;   // S plane p[0..3], T plane p[4..7]
    float *grad      = (float *)(intptr_t)a3;

    Vec3Cross( p, p + 4, normalOut );
    Vec3Normalize_R( normalOut );
    int v4 = Vec3_MajorAxis( normalOut );
    int v5 = ( v4 + 1 ) % 3;
    int v6 = ( v4 + 2 ) % 3;
    float v8 = p[v6 + 4] * p[v5] - p[v5 + 4] * p[v6];
    grad[v5] = ( p[7] * p[v6] - p[v6 + 4] * p[3] ) / v8;
    grad[v6] = ( p[v5 + 4] * p[3] - p[7] * p[v5] ) / v8;
    grad[v4] = 0.0f;
}

// ─── brush `contents` / `toolFlags` line: parse (0x42f710) + write (0x42f6a0) ─────
// A brush's optional `contents <names>;` / `toolFlags <names>;` line.  Normal solid
// brushes carry no such line (value 0 -> nothing parsed or written).  The name<->bit
// tables are the binary's (contents 0x6de570, toolflags 0x6de590), verbatim.
#include <universal/q_parse.h>   // Com_Parse / Com_ParseOnLine / Com_UngetToken
struct RadiantFlagName { const char *name; int bits; };
static const RadiantFlagName s_contentsTable[] = {
    { "weaponClip",   0x8002080 },
    { "nonColliding", 0x8000004 },
    { "detail",       0x8000000 },
    { nullptr,        0 }
};
static const RadiantFlagName s_toolflagsTable[] = {
    { "splitGeo", 0x100 },
    { nullptr,    0 }
};
int *contents_table  = (int *)s_contentsTable;
int *toolflags_table = (int *)s_toolflagsTable;

// KISAK: on a malformed line the binary walks off the end of the name table (it prints
// "missing token"/"not a known token" and keeps scanning, then strcmps against the
// table's NULL terminator) — we stop instead.  Identical on well-formed input.
static int Map_ParseFlagsLine( const char **text, const char *keyword,
                               const RadiantFlagName *table )
{
    parseInfo_t *tok = Com_Parse( text );          // prologue + ParseExt(text,1)
    if ( strcmp( tok->token, keyword ) )
    {
        Com_UngetToken();
        return 0;
    }
    int bits = 0;
    for ( ;; )
    {
        parseInfo_t *f = Com_ParseOnLine( text );  // prologue + ParseExt(text,0)
        if ( !f->token[0] )
        {
            Com_PrintMessage( 0, "missing token for '%s'\n", keyword );
            break;
        }
        if ( f->token[0] == ';' )
            return bits;
        int i = 0;
        while ( table[i].name && strcmp( f->token, table[i].name ) )
            ++i;
        if ( table[i].name )
            bits |= table[i].bits;
        else
            Com_PrintMessage( 0, "'%s' is not a known token for '%s'\n", f->token, keyword );
    }
    return bits;
}

int  sub_42FB80( const char **text ) { return Map_ParseFlagsLine( text, "contents",  s_contentsTable  ); }
int  sub_42FBA0( const char **text ) { return Map_ParseFlagsLine( text, "toolFlags", s_toolflagsTable ); }

// 0x45ad50 — material-name normaliser: strip to basename, then apply the
// materialNameRemap table (case-insensitive).  Only the version==0 legacy .map parse
// path calls it (Patch_ParseMesh / Face_ParseSurfDef gate on version == 0).
char *sub_45AD50(char *name)
{
    char *base = name;
    for ( char *p = name; *p; ++p )
        if ( *p == '/' || *p == '\\' )
            base = p + 1;                       // basename = after the last slash
    const char **node = (const char **)texWndGlob.materialNameRemap;
    while ( node )
    {
        if ( !_stricmp( base, node[0] ) )
            return (char *)node[1];             // remapped value
        node = (const char **)node[2];          // ->next
    }
    return base;
}

// 0x4aacc0 — FP-noise cleaner (NOT an angle normaliser): operates on the raw IEEE bit
// pattern.  If the low 12 mantissa bits are within `threshold` of a 4096-boundary, snap
// to it; otherwise leave the value alone.  __usercall — the float arrives on the stack,
// the threshold in ecx.  Sole caller Face_ParseSurfDef (0x472ec0) passes threshold=4 to
// scrub two parsed texdef floats in the legacy (version<4) .map format.
float sub_4AACC0( float f, int threshold )
{
    unsigned int bits = *(unsigned int *)&f;
    int low = (int)( bits & 0xFFF );
    if ( low <= threshold )
        bits -= (unsigned int)low;                  // snap down (clear low 12 mantissa bits)
    else if ( 4096 - low <= threshold )
        bits += (unsigned int)( 4096 - low );        // snap up (carry into bit 12)
    return *(float *)&bits;
}

// 0x4aa220 — rotate a vec3 by sequential X/Y/Z Euler angles (degrees).  Component pairs
// (a,b) per axis: X->(1,2) Y->(2,0) Z->(0,1) (the index table baked into the binary; the
// decompiler renders the X pair as a stack-adjacency read of the preceding local).
// dbl_6F42B0 is the float-rounded PI widened to double — exactly what the x87 code loads.
static const double SUB4AA220_PI = 3.141592741012573;   // dbl_6F42B0

void sub_4AA220( const float *in, const float *anglesDeg, float *out )   // 0x4aa220
{
    static const int pair[3][2] = { { 1, 2 }, { 2, 0 }, { 0, 1 } };

    float v[3] = { in[0], in[1], in[2] };
    for ( int i = 0; i < 3; ++i )
    {
        if ( anglesDeg[i] != 0.0f )
        {
            double rad = (double)anglesDeg[i] * SUB4AA220_PI / 180.0;
            float  c   = (float)cos( rad );
            float  s   = (float)sin( rad );
            int    a   = pair[i][0];
            int    b   = pair[i][1];
            float  va  = v[a];
            float  vb  = v[b];
            v[a] = va * c - vb * s;
            v[b] = va * s + vb * c;
        }
    }
    out[0] = v[0];
    out[1] = v[1];
    out[2] = v[2];
}

// vector->angle helpers: vectoangles 0x4a5020, vectosignedpitch 0x4a4f70, AxisToAngles
// 0x4a8a00.  The originals are inline x87; the math is the standard Quake form.  Reached
// on the prefab-enter camera reframe, not on the .map round-trip path.
static const double RAD_PI = 3.1415927410125732;   // dbl_6F42B0 (float PI widened)

void vectoangles( float *ang, int vecAddr )         // 0x4a5020
{
    const float *vec = (const float *)(intptr_t)vecAddr;
    float yaw, pitch;
    if ( vec[1] == 0.0f && vec[0] == 0.0f )
    {
        // Straight up/down: yaw 0, pitch +90 (down) / 270 (= -90, up).
        yaw   = 0.0f;
        pitch = ( -vec[2] < 0.0f ) ? 270.0f : 90.0f;   // -z<0 means z>0, looking up
    }
    else
    {
        yaw = (float)( atan2( (double)vec[1], (double)vec[0] ) * 180.0 / RAD_PI );
        if ( yaw < 0.0f )
            yaw += 360.0f;
        double fwd = sqrt( (double)vec[0] * vec[0] + (double)vec[1] * vec[1] );
        pitch = (float)( atan2( (double)vec[2], fwd ) * -180.0 / RAD_PI );
        if ( pitch < 0.0f )
            pitch += 360.0f;
    }
    ang[0] = pitch;
    ang[1] = yaw;
    ang[2] = 0.0f;
}

// 0x4a4f70 — SIGNED pitch (no +360 wrap); used only by AxisToAngles to recover roll.
static float vectosignedpitch( const float *vec )
{
    if ( vec[1] == 0.0f && vec[0] == 0.0f )
        return ( -vec[2] < 0.0f ) ? -90.0f : 90.0f;   // flt_6F4410 / flt_6F4414
    double fwd = sqrt( (double)vec[0] * vec[0] + (double)vec[1] * vec[1] );
    return (float)( atan2( (double)vec[2], fwd ) * -180.0 / RAD_PI );
}

void AxisToAngles( float *angles, float (*axis)[3] )   // 0x4a8a00
{
    // pitch/yaw from the forward vector; roll by rotating the right vector back through
    // -yaw then -pitch and reading the residual signed pitch (its sign vs the rolled-right
    // Y picks the 0 / +-180 quadrant).  axis layout: [0]=fwd, [1]=right, [2]=up.
    vectoangles( angles, (int)(intptr_t)&axis[0][0] );

    float rx = axis[1][0], ry = axis[1][1], rz = axis[1][2];

    double cyaw = -DEG2RAD( (double)angles[1] );
    float cy = (float)cos( cyaw ), sy = (float)sin( cyaw );
    float t  = rx * cy - ry * sy;
    ry       = ry * cy + sy * rx;

    double cpit = -DEG2RAD( (double)angles[0] );
    float cp = (float)cos( cpit ), sp = (float)sin( cpit );
    float rolled[3];
    rolled[0] = t * cp + rz * sp;
    rolled[1] = ry;                        // unused by signed-pitch
    rolled[2] = cp * rz - t * sp;
    float roll = vectosignedpitch( rolled );

    if ( ry >= 0.0f )
        angles[2] = -roll;
    else if ( roll >= 0.0f )
        angles[2] = roll - 180.0f;
    else
        angles[2] = roll + 180.0f;
}

// 0x4a59c0 (com_math.cpp:1399) — the radiant binary's copy of the engine's
// ClosestApproachOfTwoLines (universal/com_math.cpp, which carries the 1399
// assert).  Callers use the engine fn directly; the duplicate copy is gone.

// 0x4abeb0 AnglesToAxis / 0x4a49a0 Vec3RotateTranspose / 0x4ba7d0 OrientationConcatenate.
// The editor declares these with its own signatures (distinct overloads from the kisak
// com_math versions, which differ by const/return); the orientation pair is in draw.cpp.
// Vec3RotateTranspose's top-level-const pointer params are load-bearing: MSVC mangles
// them distinctly, and entity.cpp's call resolves against exactly this declaration.
extern void OrientationPosToWorldPos( float *out, const float *pos, const orientation_t *orient );
extern void OrientationDirToWorldDir( float *out, const orientation_t *orient, const float *dir );

void OrientationConcatenate( const orientation_t *orFirst, const orientation_t *orSecond,
                             orientation_t *out )
{
    OrientationDirToWorldDir( out->axis[0], orSecond, orFirst->axis[0] );
    OrientationDirToWorldDir( out->axis[1], orSecond, orFirst->axis[1] );
    OrientationDirToWorldDir( out->axis[2], orSecond, orFirst->axis[2] );
    OrientationPosToWorldPos( out->origin, orFirst->origin, orSecond );
}

float *AnglesToAxis( float *angles, float (*axis)[3] )
{
    double yaw = DEG2RAD( angles[1] ), cy = cos( yaw ), sy = sin( yaw );
    double pit = DEG2RAD( angles[0] ), cp = cos( pit ), sp = sin( pit );
    axis[0][0] = (float)( cp * cy );
    axis[0][1] = (float)( cp * sy );
    axis[0][2] = (float)( -sp );
    double rol = DEG2RAD( angles[2] ), cr = cos( rol ), sr = sin( rol );
    axis[1][0] = (float)( cy * ( sr * sp ) - cr * sy );
    axis[1][1] = (float)( ( sr * sp ) * sy + cr * cy );
    axis[1][2] = (float)( sr * cp );
    axis[2][0] = (float)( cy * ( cr * sp ) + sy * sr );
    axis[2][1] = (float)( sy * ( cr * sp ) - cy * sr );
    axis[2][2] = (float)( cr * cp );
    return (float *)axis;
}

void Vec3RotateTranspose( float * const in, float ( *matrix )[3], float * const out )
{
    const float *m = &matrix[0][0];
    out[0] = m[0] * in[0] + m[3] * in[1] + m[6] * in[2];
    out[1] = m[1] * in[0] + m[4] * in[1] + m[7] * in[2];
    out[2] = m[2] * in[0] + m[5] * in[1] + m[8] * in[2];
}

// ─── .map writer helpers (mapparsing.cpp) ────────────────────────────────────
// 0x42fb40 — write `layer "<name>"` for a non-default layer.
void MapLoad_ParseBrush_Layer(int (**writer)(int, const char *, ...), int layerStr)
{
    // KEEP_VERBOSE: this IS the mapparsing.cpp:190 assert carrier (foreign common/ TU).
    if ( !layerStr )
        Assert( "C:\\trees\\cod3-pc\\cod3-modtools\\cod3src\\common\\mapparsing.cpp",
                190, 0, "%s", "layerName" );
    (**writer)( (int)(intptr_t)writer, "layer \"%s\"\n", layerStr );
}
// 0x42f6a0 — write `  <keyword> name name ... ;` when value is non-zero, nothing
// otherwise (so solid brushes emit no contents/toolFlags line).
void MapLoad_ParseBrush_Content(int keyword, int (**writer)(int, const char *, ...),
                                int value, void *tablePtr)
{
    if ( !value )
        return;
    const RadiantFlagName *table = (const RadiantFlagName *)tablePtr;
    (**writer)( (int)(intptr_t)writer, "  %s", keyword );
    for ( int i = 0; table[i].name; ++i )
    {
        if ( ( value & table[i].bits ) == table[i].bits )
        {
            (**writer)( (int)(intptr_t)writer, " %s", (int)(intptr_t)table[i].name );
            value &= ~table[i].bits;
        }
    }
    (**writer)( (int)(intptr_t)writer, ";\n" );
}

// Radiant Error() — wraps Com_Error (used by brush parse code).
void Error(const char *fmt, ...)
{
    char buf[2048];
    va_list ap;
    va_start(ap, fmt);
    vsnprintf(buf, sizeof(buf), fmt, ap);
    va_end(ap);
    Com_Error(ERR_FATAL, "Radiant Error: %s", buf);
}

// j__atol — IDA import thunk for atol.
long j__atol(const char *s) { return atol(s); }

// Trampolines for two brush.cpp entry points whose IDB __usercall shape passes
// face/brush as ints: sub_476740 -> Brush_SetFaceTexdefSize, sub_4767E0 ->
// Brush_SetFaceTexdef (find/replace "Live" path + select.cpp Brush_SetTextureMapping).
void Brush_SetFaceTexdefSize(const float *size2, face_t *f, brush_t *b);           // brush.cpp 0x476740
brush_t *Brush_SetFaceTexdef( const texdef_sub_t *texDef, face_t *f, brush_t *b );  // brush.cpp 0x4767e0
void sub_476740(int texdef, face_t *face, brush_t *brush)
{
    Brush_SetFaceTexdefSize((const float *)(intptr_t)texdef, face, brush);
}
void sub_4767E0( const texdef_sub_t *texDef, int facePtr, int brushDef )
{
    Brush_SetFaceTexdef( texDef, (face_t *)(intptr_t)facePtr, (brush_t *)(intptr_t)brushDef );
}


extern void Com_BeginParseSession(const char *);  // q_parse.cpp

// KISAK: 0x40a480 VA is MFC CString::Format (first arg is the CString to format into).
// Ported as a 4-slot rotating buffer keyed by that arg — every caller only reads the
// result as a const char*.
char *VA( int slot, const char *fmt, ... )
{
    static char s_vaBufs[4][4096];
    int idx = slot & 3;
    va_list ap;
    va_start( ap, fmt );
    vsnprintf( s_vaBufs[idx], sizeof(s_vaBufs[idx]), fmt, ap );
    va_end( ap );
    return s_vaBufs[idx];
}

// 0x42c1e0 MainFrm_EntList is NOT a window populate: like MainFrm_BrushList (ported in
// mainfrm.cpp) it is a pure Assert-based consistency walk over the entity list, and it
// recurses into each prefab.  UNPORTED on purpose: several of its invariants
// (ent->modelInst == 0 for a fixed-size entity, ent->version != ent->def->version) are
// about bookkeeping this port deliberately does differently, so porting it verbatim
// would fire asserts on healthy maps.  See CLEANUP_TRACKER (unit 11) before wiring it.
void MainFrm_EntList( entity_s *, const char * ) {}

// ═════════════════════════════════════════════════════════════════════════════
//  PERFORCE COMMAND LAYER  (0x437640 / 0x4377e0 / 0x437850 / 0x4379c0)
//
//  CANNOT BE PORTED — the binary does NOT shell out to p4.exe.  P4_RunCommand
//  (0x437640) drives the statically-linked PERFORCE C++ API directly:
//      ClientApi client;              // sub_4DED90 ctor / sub_4DE910 dtor
//      Error e;
//      client.Init(&e);               // sub_4DE950   -> CheckError("Init")  0x437560
//      char buf[1024]; strcpy(buf, path);
//      char *argv[1] = { buf };
//      client.SetArgv(1, argv);       // sub_4E8230
//      client.Run(cmd, ui);           // sub_4DE980   (ui = the ClientUser subclass)
//      bool failed = g_p4Failed;      // byte_25D5A69 — set by the ClientUser callbacks
//      g_p4Failed = 0;
//      client.Final(&e);              // sub_4DE990   -> CheckError("Final") 0x437560
//      return !failed;
//  and the three wrappers are each one line of it with a different ClientUser + verb:
//      P4_OpenForEdit (0x4377e0)  StdClientUser    verb "edit"
//      P4_AddFile     (0x437850)  StdClientUser    verb "add"
//      FileExists     (0x4379c0)  SilentClientUser verb "fstat"   <- a PERFORCE query,
//                                 NOT a filesystem probe (it answers "is this path known
//                                 to the depot?").
//  p4api (clientapi.h / p4api.lib) is a third-party dependency that is not in the
//  KisakCOD tree, so the ClientApi half is unportable here.  What IS reproduced is the
//  binary's own Init-FAILURE path (0x4376b9): CheckError("Init") true -> clear
//  g_qeglobals.toggle_unk05 for the session and return false.  That is exactly what the
//  real function does on a machine with no p4 connection, and it keeps every caller
//  (Map_SaveFileToPerforce 0x48cc70, Eclass_RealizeModel) on a faithful branch.
//  toggle_unk05 is zero-initialised and nothing in this port sets it, so these are
//  unreachable in practice; wiring real p4 support means adding p4api and filling in
//  P4_RunCommand below — nothing else changes.
// ═════════════════════════════════════════════════════════════════════════════
// 0x437640 — the shared entry point.  `ui` is the ClientUser subclass vtable the real
// implementation would pass to ClientApi::Run; kept in the signature so the three
// wrappers stay 1:1 with the binary.
static bool P4_RunCommand( const char * /*uiKind*/, const char * /*path*/, const char * /*verb*/ )
{
    g_qeglobals.toggle_unk05 = false;   // 0x437772 — the Init/Final failure path
    return false;                       // 0x4376df
}

bool P4_OpenForEdit( const char *path )      // 0x4377e0
{
    return P4_RunCommand( "StdClientUser", path, "edit" );
}
bool P4_AddFile( const char *path )          // 0x437850
{
    return P4_RunCommand( "StdClientUser", path, "add" );
}
int  FileExists( const char *path )          // 0x4379c0 — `p4 fstat`, see the banner
{
    return P4_RunCommand( "SilentClientUser", path, "fstat" ) ? 1 : 0;
}

// Historical port names kept as forwarders so the existing call sites (map.cpp) and the
// IDB addresses stay greppable.
void sub_437850( const char *path ) { P4_AddFile( path ); }
int  sub_4377E0( const char *path ) { return P4_OpenForEdit( path ) ? 1 : 0; }

// unknown_libname_291 — the CRT bounds-check abort (IDA _VEC_validate equivalent),
// reached from g_SelectedFaces.GetAt indexing.
void unknown_libname_291()
{
    Com_Error(ERR_FATAL, "RADIANT: g_SelectedFaces bounds check failed (unknown_libname_291)");
}

// 0x447860 — 1 iff no faces are selected and EVERY selected brush is a patch (selection
// non-empty).  Loop shape is the binary's.
int OnlyPatchesSelected()
{
    if ( g_SelectedFaces.GetSize() > 0 )
        return 0;
    selbrush_t *b = selected_brushes.next;
    if ( b == &selected_brushes )
        return 0;
    for ( ; b->patch; b = b->next )
        if ( b->next == &selected_brushes )
            return 1;     // walked the whole list, all patches
    return 0;
}

// 0x447890 — 1 iff any selected brush is a patch.
int AnyPatchesSelected()
{
    for ( selbrush_t *b = selected_brushes.next; b != &selected_brushes; b = b->next )
        if ( b->patch )
            return 1;
    return 0;
}
